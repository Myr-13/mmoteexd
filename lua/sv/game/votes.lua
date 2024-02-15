local _Votes = {}
local _VoteMenus = {}

local MENU_NONE = 0
local MENU_ACCOUNT = 1
local MENU_SERVER_INFO = 2
local MENU_INVENTORY = 3
local MENU_ITEM = 4
local MENU_UPGRADES = 5
local MENU_WORKS = 6
local MENU_EQUIPMENT = 7

for i = 0, 63 do
	_Votes[i] = {}
	_VoteMenus[i] = MENU_NONE
end


local function AddVote(CID, Data, Desc)
	table.insert(_Votes[CID], {Data, Desc})

	local Msg = CMsgPacker(NETMSGTYPE_SV_VOTEOPTIONADD, false)
	Msg:AddString(Desc)
	Game.Server:SendMsg(Msg, MSGFLAG_VITAL, CID)
end


local function AddVoteFormat(CID, Data, Desc, ...)
	AddVote(CID, Data, string.format(Desc, ...))
end


local function AddVoteMenuFormat(CID, Menu, Desc)
	AddVote(CID, "menu" .. tostring(Menu), Desc)
end


local function AddVoteLabel(CID, Desc, ...)
	local Args = {...}

	if #Args == 0 then
		AddVote(CID, "null", Desc)
	else
		AddVote(CID, "null", string.format(Desc, ...))
	end
end


local function AddBack(CID, Menu)
	AddVote(CID, "null", "")
	AddVote(CID, "menu" .. Menu, "◄ Back ◄")
end


local function ClearVotes(CID)
	_Votes[CID] = table.clear(_Votes[CID])

	local Msg = CMsgPacker(NETMSGTYPE_SV_VOTECLEAROPTIONS, false)
	Game.Server:SendMsg(Msg, MSGFLAG_VITAL, CID)
end


local function RebuildMenu(CID)
	ClearVotes(CID)

	local AccData = Player.GetData(CID, "AccData")
	local Inv = Player.GetData(CID, "Inventory")
	local Stats = Player.GetData(CID, "Stats")
	local Works = Player.GetData(CID, "Works")
	local Equip = Player.GetData(CID, "Equip")

	if _VoteMenus[CID] == MENU_ACCOUNT then
		AddVoteLabel(CID, "┌─ Profile info")
		AddVoteLabel(CID, "│ ☪ Account: %s", AccData.Name)
		AddVoteLabel(CID, "│ ღ Level: %d", AccData.Level)
		AddVoteLabel(CID, "│ ღ EXP: %d", AccData.Exp)
		AddVoteLabel(CID, "│ ღ Money: %.1f", AccData.Money)
		AddVoteLabel(CID, "│ ღ Ruby: %d", AccData.Ruby)
		AddVoteLabel(CID, "├─ Server")
		AddVoteMenuFormat(CID, MENU_SERVER_INFO, "│ ☞ Info")
		AddVoteLabel(CID, "├─ Account menu")
		AddVoteMenuFormat(CID, MENU_UPGRADES, "│ ☞ Stats")
		AddVoteMenuFormat(CID, MENU_WORKS, "│ ☞ Works")
		AddVoteMenuFormat(CID, MENU_INVENTORY, "│ ☞ Inventory")
		AddVoteMenuFormat(CID, MENU_EQUIPMENT, "│ ☞ Equipment")
		AddVoteLabel(CID, "└──────────────────")
	elseif _VoteMenus[CID] == MENU_SERVER_INFO then
		AddBack(CID, MENU_ACCOUNT)
	elseif _VoteMenus[CID] == MENU_INVENTORY then
		for k, v in pairs(Inv) do
			AddVoteFormat(CID, "inv_select_item" .. tostring(k), "☞ %s x%d", GetItemName(k), v.Count)
		end

		AddBack(CID, MENU_ACCOUNT)
	elseif _VoteMenus[CID] == MENU_UPGRADES then
		AddVoteLabel(CID, "┌─ Stats")
		AddVoteLabel(CID, "│ ღ Upgrade points: %d", AccData.UpgradePoints)
		AddVoteLabel(CID, "├─ Upgrading")
		AddVoteFormat(CID, "stats_upgrade0", "│ ☞ Strength - %d", Stats.Str)
		AddVoteLabel(CID, "│ ├── Melee damage: +%.2f%% (+%.2f%%)", Statistic.MeleeDamage(Stats.Str), Statistic.MeleeDamage(1))
		AddVoteLabel(CID, "│ ├── Health: +%d (+%d)", Statistic.Health(Stats.Str), Statistic.Health(1))
		AddVoteLabel(CID, "│ └── Health regen: +%.2f (+%.2f)", Statistic.HealthRegen(Stats.Str), Statistic.HealthRegen(1))
		AddVoteFormat(CID, "stats_upgrade1", "│ ☞ Agility - %d", Stats.Dex)
		AddVoteLabel(CID, "│ ├── Range damage: +%.2f%% (+%.2f%%)", Statistic.RangeDamage(Stats.Dex), Statistic.RangeDamage(1))
		AddVoteLabel(CID, "│ ├── Crit chance: +%.2f%% (+%.2f%%)", Statistic.CritChance(Stats.Dex), Statistic.CritChance(1))
		AddVoteLabel(CID, "│ └── Attack speed: +%.2f%% (+%.2f%%)", Statistic.AttackSpeed(Stats.Dex), Statistic.AttackSpeed(1))
		AddVoteFormat(CID, "stats_upgrade2", "│ ☞ Intelligence - %d", Stats.Int)
		AddVoteLabel(CID, "│ ├── Magic damage: +%.2f%% (+%.2f%%)", Statistic.MagicDamage(Stats.Int), Statistic.MagicDamage(1))
		AddVoteLabel(CID, "│ ├── Mana: +%d (+%d)", Statistic.Mana(Stats.Int), Statistic.Mana(1))
		AddVoteLabel(CID, "│ └── Mana regen: +%.2f (+%.2f)", Statistic.ManaRegen(Stats.Int), Statistic.ManaRegen(1))
		AddVoteLabel(CID, "└──────────────────")

		AddBack(CID, MENU_ACCOUNT)
	elseif _VoteMenus[CID] == MENU_ITEM then
		local ItemID = Player.GetData(CID, "SelectedItem")

		AddVoteLabel(CID, "Call reason = Count")
		AddVoteLabel(CID, "┌─ Item menu")
		AddVoteLabel(CID, "│ Item: %s", GetItemName(ItemID))
		AddVoteLabel(CID, "│ Count: %d", Inv[ItemID].Count)
		AddVoteLabel(CID, "├─ Actions")

		local Type = GetItemType(ItemID)

		if Type == TYPE_USABLE then
			AddVote(CID, "inv_use_item" .. tostring(ItemID), "│ ☞ Use")
		end

		AddVote(CID, "inv_drop_item" .. tostring(ItemID), "│ ☞ Drop")
		AddVoteLabel(CID, "└──────────────────")

		AddBack(CID, MENU_INVENTORY)
	elseif _VoteMenus[CID] == MENU_WORKS then
		local FarmerLevel = GetWorkLevel(WORK_FARMER, Works.Farmer)
		local MinerLevel = GetWorkLevel(WORK_MINER, Works.Miner)
		local ForagerLevel = GetWorkLevel(WORK_FORAGER, Works.Forager)
		local FisherLevel = GetWorkLevel(WORK_FISHER, Works.Fisher)
		local LoaderLevel = GetWorkLevel(WORK_LOADER, Works.Loader)

		AddVoteLabel(CID, "┌─ Works")
		AddVoteLabel(CID, "│ ღ Farmer - %d lvl (%d/%d)", FarmerLevel, Works.Farmer, GetWorkExp(WORK_FARMER, FarmerLevel + 1))
		AddVoteLabel(CID, "│ └── Farming luck: +%d ☘", GetWorkLuck(WORK_FARMER, FarmerLevel))
		AddVoteLabel(CID, "│ ღ Miner - %d lvl (%d/%d)", MinerLevel, Works.Miner, GetWorkExp(WORK_MINER, MinerLevel + 1))
		AddVoteLabel(CID, "│ └── Mining luck: +%d ☘", GetWorkLuck(WORK_MINER, MinerLevel))
		AddVoteLabel(CID, "│ ღ Forager - %d lvl (%d/%d)", ForagerLevel, Works.Forager, GetWorkExp(WORK_FORAGER, ForagerLevel + 1))
		AddVoteLabel(CID, "│ └── Foraging luck: +%d ☘", GetWorkLuck(WORK_FORAGER, ForagerLevel))
		AddVoteLabel(CID, "│ ღ Fisher - %d lvl (%d/%d)", FisherLevel, Works.Fisher, GetWorkExp(WORK_FISHER, FisherLevel + 1))
		AddVoteLabel(CID, "│ └── Fishing luck: +%d ☘", GetWorkLuck(WORK_FISHER, FisherLevel))
		AddVoteLabel(CID, "│ ღ Loader - %d lvl (%d/%d)", LoaderLevel, Works.Loader, GetWorkExp(WORK_LOADER, LoaderLevel + 1))
		AddVoteLabel(CID, "│ └── Loading luck: +%d ☘", GetWorkLuck(WORK_LOADER, LoaderLevel))
		AddVoteLabel(CID, "└──────────────────")

		AddBack(CID, MENU_ACCOUNT)
	elseif _VoteMenus[CID] == MENU_EQUIPMENT then
		local SelectedEquipSlot = Player.GetData(CID, "SelectedEquipSlot")

		AddVoteLabel(CID, "┌─ Armor")
		AddVoteFormat(
			CID,
			"select_equip_slot" .. tostring(SLOT_ARMOR_BODY),
			"│ • Body: %s", (#Equip[SLOT_ARMOR_BODY] ~= 0) and GetItemName(Equip[SLOT_ARMOR_BODY][1]) or "None")
		AddVoteFormat(
			CID,
			"select_equip_slot" .. tostring(SLOT_ARMOR_FEET),
			"│ • Feet: %s", (#Equip[SLOT_ARMOR_FEET] ~= 0) and GetItemName(Equip[SLOT_ARMOR_FEET][1]) or "None")
		AddVoteLabel(CID, "├─ Weapons")

		for k, ItemID in pairs(Equip[SLOT_WEAPONS]) do
			AddVoteLabel(CID, "│ • %d - %s", k, GetItemName(ItemID))
		end

		if SelectedEquipSlot then
			AddVoteLabel(CID, "├─ Items")

			for ItemID, _ in pairs(Inv) do
				local ItemData = GetItemData(ItemID)

				if ItemData and ItemData["EquipSlot"] == SelectedEquipSlot then
					AddVoteFormat(CID, "select_equip" .. tostring(ItemID), "│ • %s", GetItemName(ItemID))
				end
			end
		end

		AddVoteLabel(CID, "└──────────────────")

		AddBack(CID, MENU_ACCOUNT)
	elseif _VoteMenus[CID] == MENU_NONE then
		-- Just empty
	else
		AddVoteLabel(CID, "Woah, how you got here? O.o")
		AddBack(CID, MENU_ACCOUNT)
	end
end


local function HandleCmd(CID, Cmd)
	-- Change menu
	if string.starts_with(Cmd, "menu") then
		_VoteMenus[CID] = tonumber(string.ex_match(Cmd, "%d+")[1])
		RebuildMenu(CID)

		return
	end

	-- Select item in inventory
	if string.starts_with(Cmd, "inv_select_item") then
		Player.SetData(CID, "SelectedItem", tonumber(string.ex_match(Cmd, "%d+")[1]))
		_VoteMenus[CID] = MENU_ITEM
		RebuildMenu(CID)

		return
	end

	-- Use item
	if string.starts_with(Cmd, "inv_use_item") then
		local ItemID = tonumber(string.ex_match(Cmd, "%d+")[1])
		local Func = GetItemFunc(ItemID)

		if Func then
			if Func(CID) then
				RemoveItem(CID, ItemID, 1)
			end
		end

		_VoteMenus[CID] = MENU_INVENTORY
		RebuildMenu(CID)

		return
	end

	-- Stats upgrade
	if string.starts_with(Cmd, "stats_upgrade") then
		local StatID = tonumber(string.ex_match(Cmd, "%d+")[1])
		UpgradeStat(CID, StatID)
		RebuildMenu(CID)

		return
	end

	-- Select equipment slot
	if string.starts_with(Cmd, "select_equip_slot") then
		Player.SetData(CID, "SelectedEquipSlot", tonumber(string.ex_match(Cmd, "%d+")[1]))

		RebuildMenu(CID)

		return
	end

	-- Select equipment
	if string.starts_with(Cmd, "select_equip") then
		EquipItem(CID, tonumber(string.ex_match(Cmd, "%d+")[1]))

		RebuildMenu(CID)

		return
	end
end


local function OnVote(CID, Msg)
	local Data = nil
	local Type = Msg:GetString(2)
	local Desc = Msg:GetString(2)
	local Reason = Msg:GetString(2)

	-- Only option callvotes
	if Type ~= "option" then
		return
	end

	-- Search for vote data
	for _, v in pairs(_Votes[CID]) do
		if v[2] == Desc then
			Data = v[1]
		end
	end

	-- Check for correct data
	if Data == nil then
		print_err("Failed to get data for vote '", Desc, "'")
		return
	end

	-- Check for placeholder vote
	if Data == "null" then
		return
	end

	-- Do stuff
	HandleCmd(CID, Data)
end


Hook.Add("OnMessage", "VotesMenu", function(MsgID, Unpacker, CID)
	if MsgID == NETMSGTYPE_CL_CALLVOTE then
		OnVote(CID, Unpacker)

		return true
	end
end)

Hook.Add("OnPlayerLoggedIn", "LoggedVotes", function(CID)
	_VoteMenus[CID] = MENU_ACCOUNT
	RebuildMenu(CID)
end)
