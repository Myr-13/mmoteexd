local function AdmGiveItem(Result)
	local TargetID = Result:GetVictim(0)
	local ItemID = Result:GetInteger(1)
	local Count = 1

	if not Player.GetData(TargetID, "LoggedIn") then
		return
	end

	if Result.NumArguments > 2 then
		Count = Result:GetInteger(2)
	end

	GiveItem(TargetID, ItemID, Count)
end


local function AdmGiveMoney(Result)
	local TargetID = Result:GetVictim(0)
	local Count = Result:GetFloat(1)

	if not Player.GetData(TargetID, "LoggedIn") then
		return
	end

	GiveMoney(TargetID, Count)
end


local function AdmCreateEntity(Result)
	local Ply = Game.Players(Result.ClientID)
	local Chr = Ply.Character
	if not Chr then
		return
	end

	World.Spawn(Result:GetString(0), Chr.Pos, Ply.WorldID)
end


local function AdmGotoWorld(Result)
	Game.GameServer.MultiWorldManager:ChangeWorld(Result:GetVictim(0), Result:GetInteger(1))
end


Hook.Add("OnConsoleInit", "AdminCommands", function()
	Console.RegisterRcon("create_dummy", "s[dummy_ai]", "Create dummy with specific AI", CreateDummy)
	Console.RegisterRcon("give_item", "v[id] i[item_id] ?i[count]", "Give item", AdmGiveItem)
	Console.RegisterRcon("give_money", "v[id] f[count]", "Give item", AdmGiveMoney)
	Console.RegisterRcon("create_entity", "s[entity]", "Create entity with specific logic", AdmCreateEntity)
	Console.RegisterRcon("goto_world", "v[id] i[world_id]", "Teleport player v to world i", AdmGotoWorld)
end)
