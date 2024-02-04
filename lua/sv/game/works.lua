local _WorksData = {}


function AddWork(ID, Name)
	_WorksData[ID] = Name
	_G["JOB_" .. string.upper(Name)] = ID
end


-- Works
AddWork(1, "Farmer")
AddWork(2, "Miner")
AddWork(3, "Forager")
AddWork(4, "Fisher")
AddWork(5, "Loader")


function GetWorkName(ID)
	return _WorksData[ID]
end


local function InitJobs()
	local Layers = Game.Collision.Layers
	local Switch = Layers.Switch
	local Width = Game.Collision.Width

	for y = 0, Game.Collision.Height - 1 do
		for x = 0, Width - 1 do
			local Tile = Layers:GetTile(Switch, y * Width + x)
			local SwitchID = Tile.Type
			local RealPos = vec2(x * 32 + 16, y * 32 + 16)
			local ID = Game.Collision:GetTile(RealPos.x, RealPos.y)

			if IsSolid(RealPos.x, RealPos.y + 32) then
				RealPos.y = RealPos.y + 16
			elseif IsSolid(RealPos.x + 32, RealPos.y) then
				RealPos.x = RealPos.x + 16
			elseif IsSolid(RealPos.x - 32, RealPos.y) then
				RealPos.x = RealPos.x - 16
			else
				RealPos.y = RealPos.y - 16
			end

			if ID == TILE_JOB_FARM then
				World.Spawn("work_item", RealPos, JOB_FARMER)
			elseif ID == TILE_JOB_MINE then
				World.Spawn("work_item", RealPos, JOB_MINER)
			elseif ID == TILE_JOB_FORAGER then
				World.Spawn("work_item", RealPos, JOB_FORAGER)
			elseif ID == TILE_JOB_FISHER or SwitchID == TILE_JOB_FISHER then
				World.Spawn("work_item", RealPos, JOB_FISHER)
			elseif ID == TILE_JOB_LOADER then
				World.Spawn("work_item", RealPos, JOB_LOADER)
			end
		end
	end
end


Hook.Add("OnInit", "SpawnJobs", InitJobs)
