local _Mobs = {}


function AddMob(ID, EntType, Data)
	_Mobs[ID] = {
		["EntType"] = EntType,
		["Data"] = Data
	}
end


include("sv/game/mobs/mobs.lua")

-- Reset AddMob function
_G["AddMob"] = nil


local function InitMobs(WorldID)
	local Layers = Game.Collision(WorldID).Layers
	local Switch = Layers.Switch
	local Width = Game.Collision(WorldID).Width

	for y = 0, Game.Collision(WorldID).Height - 1 do
		for x = 0, Width - 1 do
			local Tile = Layers:GetTile(Switch, y * Width + x)
			local ID = Tile.Type
			local RealPos = vec2(x * 32 + 16, y * 32 + 16)

			if ID == TILE_SWITCH_MOB then
				local Mob = _Mobs[Tile.Number]
				World.Spawn(Mob.EntType, RealPos, WorldID, Mob.Data)
			end
		end
	end
end


Hook.Add("OnWorldLoaded", "SpawnMobs", InitMobs)
