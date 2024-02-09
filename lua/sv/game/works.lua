local _WorksData = {}


function AddWork(Data)
	local ID = Data.ID
	_WorksData[ID] = Data
	_G["WORK_" .. string.upper(Data.Name)] = ID
end


WORK_PICKUP_TYPE_RANDOM_ITEM = 0
WORK_PICKUP_TYPE_LEVEL_ITEM = 1

include("sv/game/works/works.lua")


function GetWorkName(ID)
	return _WorksData[ID].Name
end


function GetWorkExp(ID, Lvl)
	if not Lvl then
		Lvl = 1
	end

	return _WorksData[ID].Exp * Lvl
end


function GetWorkLuck(ID, Lvl)
	if not Lvl then
		Lvl = 1
	end

	return _WorksData[ID].Luck * Lvl
end


function GetWorkLevel(ID, Exp)
	return math.floor(Exp / GetWorkExp(ID))
end


function GetWorkPickupType(ID)
	return _WorksData[ID].PickupType
end


function GetWorkItems(ID)
	return _WorksData[ID].Items
end


function GiveWorkExp(CID, ID, Exp)
	local Works = Player.GetData(CID, "Works")

	if ID == WORK_FARMER then
		Works.Farmer = Works.Farmer + Exp
	elseif ID == WORK_MINER then
		Works.Miner = Works.Miner + Exp
	elseif ID == WORK_FORAGER then
		Works.Forager = Works.Forager + Exp
	elseif ID == WORK_FISHER then
		Works.Fisher = Works.Fisher + Exp
	else
		Works.Loader = Works.Loader + Exp
	end

	Player.SetData(CID, "Works", Works)
end


local function InitJobs(WorldID)
	local Collision = Game.Collision(WorldID)
	local Layers = Collision.Layers
	local Switch = Layers.Switch
	local Width = Collision.Width

	for y = 0, Collision.Height - 1 do
		for x = 0, Width - 1 do
			local Tile = Layers:GetTile(Switch, y * Width + x)
			local SwitchID = Tile.Type
			local RealPos = vec2(x * 32 + 16, y * 32 + 16)
			local ID = Collision:GetTile(RealPos.x, RealPos.y)

			if IsSolid(WorldID, RealPos.x, RealPos.y + 32) then
				RealPos.y = RealPos.y + 16
			elseif IsSolid(WorldID, RealPos.x + 32, RealPos.y) then
				RealPos.x = RealPos.x + 16
			elseif IsSolid(WorldID, RealPos.x - 32, RealPos.y) then
				RealPos.x = RealPos.x - 16
			else
				RealPos.y = RealPos.y - 16
			end

			if ID == TILE_JOB_FARM then
				World.Spawn("work_item", RealPos, WorldID, WORK_FARMER)
			elseif ID == TILE_JOB_MINE then
				World.Spawn("work_item", RealPos, WorldID, WORK_MINER)
			elseif ID == TILE_JOB_FORAGER then
				World.Spawn("work_item", RealPos, WorldID, WORK_FORAGER)
			elseif ID == TILE_JOB_FISHER or SwitchID == TILE_JOB_FISHER then
				World.Spawn("work_item", RealPos, WorldID, WORK_FISHER)
			elseif ID == TILE_JOB_LOADER then
				World.Spawn("work_item", RealPos, WorldID, WORK_LOADER)
			end
		end
	end
end


Hook.Add("OnWorldLoaded", "SpawnJobs", InitJobs)
