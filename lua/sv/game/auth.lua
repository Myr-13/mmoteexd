local function ChatRegister(Result)
	local CID = Result.ClientID
	local Login = Result:GetString(0)
	local Password = Result:GetString(1)
	local Password2 = Result:GetString(2)

	if Password ~= Password2 then
		Server.SendChat(CID, "Passwords not matching")
		return
	end

	DB.Execute(function(Stmt)
		-- Check for account with this login in db
		if DB.Prepare(Stmt, string.format(DB.SQL.CheckForAccount, Login)) then
			return
		end
		Stmt:Step()
		local Count = Stmt:GetInt(0)

		if Count ~= 0 then
			Server.SendChat(CID, "This name already taken")
			return
		end

		-- Create new account
		local PasswordHash = hash(Password)
		local ClientName = Game.Server:ClientName(CID)
		if DB.Prepare(Stmt, string.format(DB.SQL.NewAccount, Login, PasswordHash, ClientName, ClientName)) then
			return
		end
		Stmt:Execute()

		-- Get user id
		if DB.Prepare(Stmt, string.format(DB.SQL.GetAccount, Login)) then
			return
		end
		Stmt:Step()

		local UserID = Stmt:GetInt(0)

		-- Create new stats
		if DB.Prepare(Stmt, string.format(DB.SQL.NewStats, UserID)) then
			return
		end
		Stmt:Step()

		-- Create new works
		if DB.Prepare(Stmt, string.format(DB.SQL.NewWorks, UserID)) then
			return
		end
		Stmt:Step()

		-- Message
		Server.SendChat(CID, "You successfully registred")
	end)
end


local function ChatLogin(Result)
	local CID = Result.ClientID
	local Login = Result:GetString(0)
	local Password = Result:GetString(1)

	if Player.GetData(CID, "LoggedIn") then
		Server.SendChat(CID, "You already logged in")
		return
	end

	local PasswordHash = hash(Password)

	DB.Execute(function(Stmt)
		-- Check for account with this name and password
		if DB.Prepare(Stmt, string.format(DB.SQL.CheckForAccountWithPassword, Login, PasswordHash)) then
			return
		end
		Stmt:Step()
		local Count = Stmt:GetInt(0)

		if Count == 0 then
			Server.SendChat(CID, "Wrong login/password")
			return
		end

		-- Load basic account info
		if DB.Prepare(Stmt, string.format(DB.SQL.GetAccount, Login)) then
			return
		end
		Stmt:Step()

		-- Check for ban on account
		if Stmt:GetInt(11) ~= 0 then
			Server.SendChat(CID, "This account is banned")
			return
		end

		local AccID = Stmt:GetInt(0)

		-- Set account basic data
		Player.SetData(CID, "AccData", {
			Name = Login,
			ID = AccID,
			Level = Stmt:GetInt(3),
			Exp = Stmt:GetInt(4),
			Money = Stmt:GetFloat(5),
			Ruby = Stmt:GetInt(6),
			ClanID = Stmt:GetInt(7),
			UpgradePoints = Stmt:GetInt(8)
		})

		-- Load inventory
		if DB.Prepare(Stmt, string.format(DB.SQL.GetInventory, AccID)) then
			return
		end
		
		local Inventory = {}
		while Stmt:Step() do
			local Item = {
				ID = Stmt:GetInt(1),
				Count = Stmt:GetInt(2)
			}

			Inventory[Item.ID] = Item
		end

		Player.SetData(CID, "Inventory", Inventory)

		-- Load equipment
		if DB.Prepare(Stmt, string.format(DB.SQL.GetEquipment, AccID)) then
			return
		end

		local Equip = {}
		while Stmt:Step() do
			local Slot = Stmt:GetInt(1)

			if not Equip[Slot] then
				Equip[Slot] = {}
			end

			table.insert(Equip[Slot], Stmt:GetInt(2))
		end

		if not Equip[1] then
			Equip[1] = {
				2  -- 2 = Hammer
			}
		end

		Player.SetData(CID, "Equip", Equip)

		-- Load stats
		if DB.Prepare(Stmt, string.format(DB.SQL.GetStats, AccID)) then
			return
		end
		Stmt:Step()

		Player.SetData(CID, "Stats", {
			Str = Stmt:GetInt(1),
			Dex = Stmt:GetInt(2),
			Int = Stmt:GetInt(3)
		})

		-- Load works
		if DB.Prepare(Stmt, string.format(DB.SQL.GetWorks, AccID)) then
			return
		end
		Stmt:Step()

		Player.SetData(CID, "Works", {
			Farmer = Stmt:GetInt(1),
			Miner = Stmt:GetInt(2),
			Forager = Stmt:GetInt(3),
			Fisher = Stmt:GetInt(4),
			Loader = Stmt:GetInt(5)
		})

		-- Message and mark as logged in
		Player.SetData(CID, "LoggedIn", true)
		Player.SetData(CID, "SelectedWeapon", 1)
		Server.SendChat(CID, "You successfully logged in")
		Game.Players(CID):SetTeam(0)

		Hook.Call("OnPlayerLoggedIn", CID)
	end)
end


local function SaveAccount(CID)
	if not Player.GetData(CID, "LoggedIn") then
		return
	end

	local AccData = Player.GetData(CID, "AccData")
	local Inv = Player.GetData(CID, "Inventory")
	local Equip = Player.GetData(CID, "Equip")
	local Stats = Player.GetData(CID, "Stats")
	local Works = Player.GetData(CID, "Works")

	DB.Execute(function(Stmt)
		-- Update account data
		if DB.Prepare(Stmt, string.format(DB.SQL.UpdateAccount, AccData.Level, AccData.Exp, AccData.Money, AccData.Ruby, AccData.ClanID, AccData.UpgradePoints, AccData.ID)) then
			return
		end
		Stmt:Step()

		-- Clear old inventory data
		if DB.Prepare(Stmt, string.format(DB.SQL.ClearInventory, AccData.ID)) then
			return
		end
		Stmt:Step()

		-- Save inventory
		for k, v in pairs(Inv) do
			if DB.Prepare(Stmt, string.format(DB.SQL.AddItem, AccData.ID, k, v.Count, "")) then
				return
			end
			Stmt:Step()
		end

		-- Clear old equip data
		if DB.Prepare(Stmt, string.format(DB.SQL.ClearEquipment, AccData.ID)) then
			return
		end
		Stmt:Step()

		-- Save equip
		for Slot, v in pairs(Equip) do
			for _, Item in pairs(v) do
				if DB.Prepare(Stmt, string.format(DB.SQL.AddEquip, AccData.ID, Slot, Item)) then
					return
				end
				Stmt:Step()
			end
		end

		-- Update stats data
		if DB.Prepare(Stmt, string.format(DB.SQL.UpdateStats, Stats.Str, Stats.Dex, Stats.Int, AccData.ID)) then
			return
		end
		Stmt:Step()

		-- Update works data
		if DB.Prepare(Stmt, string.format(DB.SQL.UpdateWorks, Works.Farmer, Works.Miner, Works.Forager, Works.Fisher, Works.Loader, AccData.ID)) then
			return
		end
		Stmt:Step()
	end)
end


local function RegisterCommands()
	Console.RegisterChat("register", "s[login] s[password] s[password2]", "", ChatRegister)
	Console.RegisterChat("login", "s[login] s[password]", "", ChatLogin)
end


local function AuthMessage(MsgID, Unpacker, CID)
	if MsgID == NETMSGTYPE_CL_SETTEAM then
		if not Player.GetData(CID, "LoggedIn") then
			Server.SendChat(CID, "Login first!")
			Server.SendChat(CID, "/wtf")
		end

		return true
	end
end


Hook.Add("OnConsoleInit", "AuthCommands", RegisterCommands)
Hook.Add("OnMessage", "AuthMessage", AuthMessage)
Hook.Add("OnPlayerLeft", "ClearPlayerData", SaveAccount)
