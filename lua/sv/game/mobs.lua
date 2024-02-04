local __Mobs = {}


function AddMob(ID, EntType, Data)
	__Mobs[ID] = {
		["EntType"] = EntType,
		["Data"] = Data
	}
end


include("sv/game/mobs/mobs.lua")

-- Reset AddMob function
_G["AddMob"] = nil


local function InitMobs()
	local Layers = Game.Collision.Layers
	local Switch = Layers.Switch
	local Width = Game.Collision.Width

	for y = 0, Game.Collision.Height - 1 do
		for x = 0, Width - 1 do
			local Tile = Layers:GetTile(Switch, y * Width + x)
			local ID = Tile.Type
			local RealPos = vec2(x * 32 + 16, y * 32 + 16)

			if ID == TILE_SWITCH_MOB then
				local Mob = __Mobs[Tile.Number]
				World.Spawn(Mob.EntType, RealPos, Mob.Data)
			end
		end
	end
end


Hook.Add("OnInit", "SpawnMobs", InitMobs)
