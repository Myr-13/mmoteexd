TYPE_ITEM = 0
TYPE_USABLE = 1
TYPE_WEAPON = 2
TYPE_EQUIP = 3

SLOT_WEAPONS = 1
SLOT_ARMOR_BODY = 2
SLOT_ARMOR_FEET = 3

DAMAGE_TYPE_MELEE = 0
DAMAGE_TYPE_RANGE = 1
DAMAGE_TYPE_MAGIC = 2
DAMAGE_TYPE_BARD = 3

ATTACK_TYPE_MELEE = 0
ATTACK_TYPE_SPAWN_PROJECTILE = 1

SNAP_WEAPON_HAMMER = 0
SNAP_WEAPON_GUN = 1

local _Items = {}


function AddItem(ID, StrID, Type, Name, Desc, Data)
	local Item = {
		StrID = StrID,
		Type = Type,
		Name = Name,
		Desc = Desc,
		Data = Data
	}

	_Items[ID] = Item
	_G["ITEM_" .. string.upper(string.gsub(Name, " ", "_"))] = ID
end


-- Add all items
include("sh/game/items/base.lua")
include("sh/game/items/weapons.lua")
include("sh/game/items/armor.lua")

Hook.Add("OnConsoleInit", "ItemsCommands", function()
	-- This code will never reached on client
	Console.RegisterRcon("reload_items", "", "Reload all items from lua", function(Result)
		_Items = table.clear(_Items)

		include("sh/game/items/base.lua")
		include("sh/game/items/weapons.lua")
		include("sh/game/items/armor.lua")
	end)
end)


function GetItemName(ID)
	return _Items[ID] and _Items[ID].Name or string.format("[ERROR ITEM %d]", (not ID) and -1 or ID)
end


function GetItemType(ID)
	return _Items[ID] and _Items[ID].Type or TYPE_ITEM
end


function GetItemData(ID)
	return _Items[ID] and _Items[ID].Data or nil
end


function GetItemStrID(ID)
	return _Items[ID] and _Items[ID].StrID or "error_item"
end
