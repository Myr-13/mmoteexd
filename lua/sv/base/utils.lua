Player = {}
Server = {}
Utils = {}

Player._Player_Data = {}


function print_ex(Sys, ...)
	local Args = {...}
	local Text = ""

	for _, v in pairs(Args) do
		if type(v) == "table" then
			Text = Text .. "{"

			for k2, v2 in pairs(v) do
				Text = Text .. string.format("\"%s\" = \"%s\", ", tostring(k2), tostring(v2))
			end

			Text = Text .. "} "
		else
			Text = Text .. tostring(v) .. " "
		end
	end

	sys.dbg_msg(Sys, Text)
end


function print(...)
	print_ex("lua_print", ...)
end


function print_err(...)
	print_ex("mmo_err", ...)
end


function copy_vector(Vec)
	return vec2(Vec.x, Vec.y)
end


function copy_input(Out, In)
	Out.Direction = In.Direction
	Out.TargetX = In.TargetX
	Out.TargetY = In.TargetY
	Out.Jump = In.Jump
	Out.Fire = In.Fire
	Out.Hook = In.Hook
	Out.PlayerFlags = In.PlayerFlags
	Out.WantedWeapon = In.WantedWeapon
	Out.NextWeapon = In.NextWeapon
	Out.PrevWeapon = In.PrevWeapon
end


Player.SetData = function(CID, Key, Data)
	if Player._Player_Data[CID] == nil then
		Player._Player_Data[CID] = {}
	end
	
	Player._Player_Data[CID][Key] = Data
end


Player.GetData = function(CID, Key)
	if Player._Player_Data[CID] == nil then
		return nil
	end
	
	return Player._Player_Data[CID][Key]
end


Player.ClearData = function(CID)
	Player._Player_Data[CID] = nil
end


Player.SendBroadcastTime = function(CID, Time, Format, ...)
	Player.SetData(CID, "Broadcast", {
		["Text"] = string.format(Format, ...),
		["EndTime"] = Game.Server.Tick + Time
	})
end


Player.SendBroadcast = function(CID, Format, ...)
	Player.SendBroadcastTime(CID, 50, Format, ...)
end


Server.SendChat = function(CID, Format, ...)
	Game.GameServer:SendChatTarget(CID, string.format(Format, ...), 3)
end


Server.SendBroadcast = function(CID, Format, ...)
	local Args = {...}
	local Text = Format

	if #Args ~= 0 then
		Text = string.format(Format, ...)
	end

	Game.GameServer:SendBroadcast(Text, CID, true)
end


Utils.GetProgressBar = function(StrSize, Filler, Empty, Num, MaxNum)
	local Progress = Num / MaxNum
	local CharNum = StrSize * Progress
	local Text = ""

	for i = 0, StrSize - 1 do
		Text = Text .. ((i + 1 <= CharNum) and Filler or Empty)
	end

	return Text
end


table.clear = function(Table)
	for k in pairs(Table) do
		Table[k] = nil
	end

	return Table
end


table.copy = function(t)
	local t2 = {}

	for k, v in pairs(t) do
		t2[k] = v
	end

	return t2
end


string.ex_match = function(Str, Pattern)
	a = {}

	for i in string.gmatch(Str, Pattern) do
		a[#a + 1] = i
	end

	return a
end


string.starts_with = function(Str, Prefix)
	return string.sub(Str, 1, #Prefix) == Prefix
end


math.clamp = function(Value, Min, Max)
	if Value > Max then
		Value = Max
	end

	if Value < Min then
		Value = Min
	end

	return Value
end
