-- Game layer
TILE_AIR = 0
TILE_HOOK = 1
TILE_NOHOOK = 3
TILE_PVP_OFF = 142
TILE_PVP_ON = 143
TILE_CRAFT = 144
TILE_PLUS_5 = 145
TILE_PLUS_10 = 146
TILE_SHOP = 160
TILE_CLAN_CHAIR = 161
TILE_PLUS_2 = 162
TILE_WATER = 176
TILE_JAIL = 177
TILE_JOB_FARM = 128
TILE_JOB_MINE = 129
TILE_JOB_FORAGER = 130
TILE_JOB_FISHER = 131
TILE_JOB_LOADER = 132

-- Switch layer
TILE_SWITCH_CLAN_SPAWN = 148
TILE_SWITCH_CLAN = 149
TILE_SWITCH_MOB = 150
TILE_SWITCH_JOB = 164  -- UNUSED
TILE_SWITCH_WORLD = 165


function IsSolid(WorldID, x, y)
	local ID = Game.Collision(WorldID):GetTile(x, y)
	return (ID == TILE_HOOK or ID == TILE_NOHOOK)
end


function GetSwitchTile(WorldID, x, y)
	-- TODO: Add case, when switch tile is not exists

	local Width = Game.Collision(WorldID).Width
	local Height = Game.Collision(WorldID).Height

	x = math.clamp(math.floor(x / 32), 0, Width - 1)
	y = math.clamp(math.floor(y / 32), 0, Height - 1)

	local Layers = Game.Collision(WorldID).Layers
	local Switch = Layers.Switch
	
	return Layers:GetTile(Switch, y * Width + x)
end
