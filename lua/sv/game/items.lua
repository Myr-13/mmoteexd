TYPE_ITEM = 0
TYPE_USABLE = 1
TYPE_WEAPON = 2
TYPE_ARMOR = 3
TYPE_ACCESSORY = 4

SLOT_WEAPONS = 1

DAMAGE_TYPE_MELEE = 0
DAMAGE_TYPE_RANGE = 1
DAMAGE_TYPE_MAGIC = 2
DAMAGE_TYPE_BARD = 3

ATTACK_TYPE_MELEE = 0
ATTACK_TYPE_SPAWN_PROJECTILE = 1

local _Items = {}


function AddItem(ID, Type, Name, Desc, Data)
	local Item = {
		Type = Type,
		Name = Name,
		Desc = Desc,
		Data = Data
	}

	_Items[ID] = Item
end


-- Add all items
include("sv/game/items/base.lua")
include("sv/game/items/weapons.lua")

Hook.Add("OnConsoleInit", "ItemsCommands", function()
	Console.RegisterRcon("reload_items", "", "Reload all items from lua", function(Result)
		_Items = table.clear(_Items)

		include("sv/game/items/base.lua")
		include("sv/game/items/weapons.lua")
	end)

	Console.RegisterRcon("reload_entities", "", "Reload all entities from lua", function(Result)
		include("sv/entities/weapons/basic_projectile.lua")
		include("sv/entities/weapons/basic_wall.lua")
		include("sv/entities/weapons/basic_slowdown.lua")
	end)
end)

-- Other utility functions
function GetItemName(ID)
	return _Items[ID] and _Items[ID].Name or "[ERROR ITEM]"
end

function GetItemType(ID)
	return _Items[ID] and _Items[ID].Type or TYPE_ITEM
end

function GetItemData(ID)
	return _Items[ID] and _Items[ID].Data or nil
end

function FireWeapon(CID, ID)
	local ItemData = GetItemData(ID)

	-- Check for item data
	if not ItemData then
		Server.SendChat(CID, "Error with fire weapon")
		return
	end

	-- Load some data
	local AttackType = ItemData["AttackType"]
	local ReloadTime = GetReloadTime(CID, ItemData["ReloadTime"])
	local DamageType = ItemData["DamageType"]
	local ManaCost = ItemData["ManaCost"]
	local Projectile = ItemData["Projectile"]
	local Damage = GetDamage(CID, DamageType, ItemData["Damage"])

	-- Set animation and reload time
	local Chr = Game.Players(CID).Character
	local Dir = normalize(vec2(Chr.Input.TargetX, Chr.Input.TargetY))

	-- Check for mana
	if ManaCost and not RemoveMana(CID, ManaCost) then
		Server.SendChat(CID, "Not enough mana")
		return
	end

	local FirePoint = Chr.Pos + Dir * vec2(28, 28) * vec2(0.75, 0.75)

	-- Hammer
	if ID == 2 then  -- 2 - hammer item
		local Ent = World.GetClosestEntityFilter(FirePoint, "work_item", 42, function(Ent) return Ent.Alive end)

		if Ent then
			Ent.BreakProgress = Ent.BreakProgress + 10

			if Ent.BreakProgress >= 100 then
				Game.GameServer:CreateSound(FirePoint, SOUND_NINJA_HIT)

				Ent.SpawnTick = Game.Server.Tick + 200
			end

			Chr.ReloadTimer = 50
			Game.GameServer:CreateSound(FirePoint, SOUND_HOOK_LOOP)

			return
		end
	end

	Chr.ReloadTimer = ReloadTime
	Chr.AttackTick = Game.Server.Tick

	-- Melee attack type
	if AttackType == ATTACK_TYPE_MELEE then
		local Ents = World.GetEntitiesInRadius(FirePoint, "dummy_simple", 42)

		-- Get dummies
		for _, Ent in pairs(Ents) do
			if Ent.Alive then
				local Force = normalize(Dir + vec2(0, -1.1)) * vec2(10, 10)
				Force = vec2(0, -1) + Force

				Ent:Damage(Force, Damage, CID)

				Game.GameServer:CreateHammerHit(FirePoint)
			end
		end

		-- Get characters
		local Chrs = World.GetCharactersInRadius(FirePoint, 42, CID)

		for _, Ent in pairs(Chrs) do
			local Force = normalize(Dir + vec2(0, -1.1)) * vec2(10, 10)
			Force = vec2(0, -1) + Force

			Ent:TakeDamage(Force, Damage, CID)

			Game.GameServer:CreateHammerHit(FirePoint)
		end
	elseif AttackType == ATTACK_TYPE_SPAWN_PROJECTILE then
		World.Spawn(Projectile, Chr.Pos, Dir, CID, DamageType, Damage)
	end
end
