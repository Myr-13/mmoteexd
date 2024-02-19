local MENU_NONE = 0
local MENU_STATS = 1
local MENU_INVENTORY = 2

local MenuOpened = MENU_NONE


local function RenderStats()
	if not Account.Stats then
		return
	end

	ImGui.Begin("Stats", 0)
	ImGuiEx.Text("Upgrade points - %d", Account.UpgradePoints)

	-- Str
	if ImGuiEx.ButtonEx(vec2(75, 20), "Upgrade##0") then
		Network.SendMsg("upgrade@stats", { ID = 0 })
	end

	ImGui.SameLine()
	ImGuiEx.Text("Strength - %d", Account.Stats.Str)
	ImGuiEx.BulletText("Melee damage: +%.2f%% (+%.2f%%)", Statistic.MeleeDamage(Account.Stats.Str), Statistic.MeleeDamage(1))
	ImGuiEx.BulletText("Health: +%d (+%d)", Statistic.Health(Account.Stats.Str), Statistic.Health(1))
	ImGuiEx.BulletText("Health regen: +%.2f (+%.2f)", Statistic.HealthRegen(Account.Stats.Str), Statistic.HealthRegen(1))

	-- Dex
	if ImGuiEx.ButtonEx(vec2(75, 20), "Upgrade##1") then
		Network.SendMsg("upgrade@stats", { ID = 1 })
	end

	ImGui.SameLine()
	ImGuiEx.Text("Agility - %d", Account.Stats.Dex)
	ImGuiEx.BulletText("Range damage: +%.2f%% (+%.2f%%)", Statistic.RangeDamage(Account.Stats.Dex), Statistic.RangeDamage(1))
	ImGuiEx.BulletText("Crit chance: +%.2f%% (+%.2f%%)", Statistic.CritChance(Account.Stats.Dex), Statistic.CritChance(1))
	ImGuiEx.BulletText("Attack speed: +%.2f%% (+%.2f%%)", Statistic.AttackSpeed(Account.Stats.Dex), Statistic.AttackSpeed(1))

	-- Int
	if ImGuiEx.ButtonEx(vec2(75, 20), "Upgrade##2") then
		Network.SendMsg("upgrade@stats", { ID = 2 })
	end

	ImGui.SameLine()
	ImGuiEx.Text("Intelligence - %d", Account.Stats.Int)
	ImGuiEx.BulletText("Magic damage: +%.2f%% (+%.2f%%)", Statistic.MagicDamage(Account.Stats.Int), Statistic.MagicDamage(1))
	ImGuiEx.BulletText("Mana: +%d (+%d)", Statistic.Mana(Account.Stats.Int), Statistic.Mana(1))
	ImGuiEx.BulletText("Mana regen: +%.2f (+%.2f)", Statistic.ManaRegen(Account.Stats.Int), Statistic.ManaRegen(1))

	ImGui.End()
end


local function RenderInventory()
	ImGui.Begin("Inventory", 0)

	for k, v in pairs(Account.Inventory) do
		ImGuiEx.Text("%s x%d", GetItemName(k), v["Count"])
	end

	ImGui.End()
end


local function OpenMenu(MenuID)
	if MenuOpened == MenuID then
		MenuOpened = MENU_NONE
	else
		MenuOpened = MenuID
	end

	if MenuOpened == MENU_NONE then
		Game.Input:MouseModeRelative()
	else
		Game.Input:MouseModeAbsolute()
	end
end


Hook.Add("RenderLevel39", "MenusRender", function()
	if MenuOpened == MENU_STATS then
		RenderStats()
	elseif MenuOpened == MENU_INVENTORY then
		RenderInventory()
	end
end)


Hook.Add("OnInput", "Menus", function(Event)
	if Event.Flags == 1 then  -- LuaJIT dosen't support binop, only with module :(
		local Key = Event.Key

		if Key == KEY_O then
			OpenMenu(MENU_STATS)
		elseif Event.Key == KEY_I then
			OpenMenu(MENU_INVENTORY)
		end

		if Key == KEY_ESCAPE then
			OpenMenu(MENU_NONE)
		end
	end
end)
