-- Add ./libs/ to package.path
do
	local Cwd = package.path
	local _, End

	_, End = string.find(Cwd, ";", 1)
	Cwd = string.sub(Cwd, End + 1, #Cwd)
	_, End = string.find(Cwd, ";", 1)
	Cwd = string.sub(Cwd, 1, End - 6)
	Cwd = Cwd .. "libs\\?.lua"

	package.path = package.path .. Cwd .. ";"
end

-- Base library
include("sh/base/hook.lua")
include("sv/base/console.lua")
include("sv/base/database.lua")
include("sv/base/network.lua")
include("sv/base/sql_queries.lua")
include("sv/base/utils.lua")
include("sv/base/world.lua")

-- Gameplay stuff
include("sv/game/commands/admin.lua")
include("sv/game/commands/general.lua")
include("sv/game/enums/sounds.lua")
include("sv/game/enums/tiles.lua")

include("sv/game/account_utils.lua")
include("sv/game/auth.lua")
include("sv/game/items.lua")
include("sv/game/mmo_client_handler.lua")
include("sv/game/mobs.lua")
include("sv/game/players.lua")
include("sv/game/spells.lua")
include("sv/game/stats.lua")
include("sv/game/votes.lua")
include("sv/game/works.lua")
include("sv/game/worlds.lua")

-- Entities
include("sv/entities/dummies/dummy_base.lua")
include("sv/entities/dummies/slime.lua")
include("sv/entities/weapons/basic_projectile.lua")
include("sv/entities/weapons/spell_wall.lua")
include("sv/entities/weapons/spell_slowdown.lua")
include("sv/entities/weapons/spell_healing_rain.lua")
include("sv/entities/pickup.lua")
include("sv/entities/work_item.lua")

DB.CreateTables()


function RegisterHook(Name)
	_G[Name] = function(...)
		local a = Hook.Call(Name, ...)

		if a then
			return a
		end
	end
end


RegisterHook("OnConsoleInit")
RegisterHook("OnMessage")
RegisterHook("OnPlayerLeft")
RegisterHook("OnPostPlayerLeft")
RegisterHook("OnPlayerJoin")
RegisterHook("OnTick")
RegisterHook("OnSnap")
RegisterHook("OnCharacterFireWeapon")
RegisterHook("OnCharacterInput")
RegisterHook("OnInit")
RegisterHook("OnWorldLoaded")

Hook.Add("OnPostPlayerLeft", "ClearPlayerData", function(CID)
	Player.ClearData(CID)
end)
