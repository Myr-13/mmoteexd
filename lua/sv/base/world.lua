EntityManager = {}
EntityManager._Entities = {}


EntityManager.Register = function(Type, Data)
	EntityManager._Entities[Type] = Data
end


World = {}
World._Entities = {}


World.Spawn = function(Type, ...)
	-- Check for registred type
	if EntityManager._Entities[Type] == nil then
		print_err("tried to spawn invalid entity type:", Type)
		return
	end

	-- Create entity list with specific entity type
	if World._Entities[Type] == nil then
		World._Entities[Type] = {}
	end

	local Ent = table.copy(EntityManager._Entities[Type])

	if Ent.OnInit then
		Ent:OnInit(...)
	end

	table.insert(World._Entities[Type], Ent)
end


World.OnTick = function()
	for k1, v in pairs(World._Entities) do
		for k2, Ent in pairs(v) do
			if Ent.OnTick then
				Ent:OnTick()
			end

			if Ent.MarkedToRemove then
				Game.Server:SnapFreeID(Ent.SnapID)
				table.remove(World._Entities[k1], k2)
			end
		end
	end
end


World.OnSnap = function(ClientID)
	for _, v in pairs(World._Entities) do
		for _, Ent in pairs(v) do
			if Ent.OnSnap then
				Ent:OnSnap(ClientID)
			end
		end
	end
end


World.GetClosestEntity = function(Pos, Type, MaxDist)
	if World._Entities[Type] == nil then
		return
	end

	local MinDist = MaxDist
	local ClosestEnt = nil

	for _, Ent in pairs(World._Entities[Type]) do
		local Dist = distance(Ent.Pos, Pos)

		if Dist < MinDist then
			if string.starts_with(Type, "dummy_") then
				if Ent.Alive then
					MinDist = Dist
					ClosestEnt = Ent
				end
			else
				MinDist = Dist
				ClosestEnt = Ent
			end
		end
	end

	return ClosestEnt
end


World.GetClosestEntityFilter = function(Pos, Type, MaxDist, FilterFunc)
	if World._Entities[Type] == nil then
		return
	end

	local MinDist = MaxDist
	local ClosestEnt = nil

	for _, Ent in pairs(World._Entities[Type]) do
		local Dist = distance(Ent.Pos, Pos)

		if Dist < MinDist and FilterFunc(Ent) then
			if string.starts_with(Type, "dummy_") then
				if Ent.Alive then
					MinDist = Dist
					ClosestEnt = Ent
				end
			else
				MinDist = Dist
				ClosestEnt = Ent
			end
		end
	end

	return ClosestEnt
end


World.GetEntitiesInRadius = function(Pos, Type, Dist)
	if World._Entities[Type] == nil then
		return {}
	end

	local Ents = {}

	for _, Ent in pairs(World._Entities[Type]) do
		if distance(Ent.Pos, Pos) < Dist then
			if string.starts_with(Type, "dummy_") then
				if Ent.Alive then
					table.insert(Ents, Ent)
				end
			else
				table.insert(Ents, Ent)
			end
		end
	end

	return Ents
end


World.GetCharactersInRadius = function(Pos, Radius, NotThis)
	local Chrs = {}

	if not NotThis then
		NotThis = -1
	end

	for i = 0, 23 do
		local Ply = Game.Players(i)

		if i ~= NotThis and Ply then
			local Chr = Ply.Character

			if Chr and distance(Chr.Pos, Pos) <= Radius then
				table.insert(Chrs, Chr)
			end
		end
	end

	return Chrs
end


World.GetFirstCharacterInRadius = function(Pos, Radius, NotThis)
	local FinalChr = nil

	if not NotThis then
		NotThis = -1
	end

	for i = 0, 23 do
		local Ply = Game.Players(i)

		if i ~= NotThis and Ply and not FinalChr then
			local Chr = Ply.Character

			if Chr and distance(Chr.Pos, Pos) <= Radius then
				FinalChr = Chr
			end
		end
	end

	return FinalChr
end


World.GetClosestCharacter = function(Pos, MaxDist)
	local MinDist = MaxDist
	local ClosestChr = nil

	for i = 0, 23 do
		local Ply = Game.Players(i)

		if Ply and Ply.Character then
			local Chr = Ply.Character

			if Chr then
				local Dist = distance(Chr.Pos, Pos)

				if Dist < MinDist then
					MinDist = Dist
					ClosestChr = Chr
				end
			end
		end
	end

	return ClosestChr
end


Hook.Add("OnTick", "TickWorld", World.OnTick)
Hook.Add("OnSnap", "SnapWorld", World.OnSnap)
Hook.Add("OnConsoleInit", "EntitiesCommands", function()
	Console.RegisterRcon("reload_entities", "", "Reload all entities from lua", function(Result)
		EntityManager._Entities = table.clear(EntityManager._Entities)

		include("sv/entities/dummies/dummy_base.lua")
		include("sv/entities/dummies/slime.lua")
		include("sv/entities/weapons/basic_projectile.lua")
		include("sv/entities/weapons/spell_wall.lua")
		include("sv/entities/weapons/spell_slowdown.lua")
		include("sv/entities/weapons/spell_healing_rain.lua")
		include("sv/entities/pickup.lua")
		include("sv/entities/work_item.lua")
	end)
end)
