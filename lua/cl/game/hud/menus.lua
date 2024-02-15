local Menus = {
	IsStatsOpen = false
}


local function RenderStats()
	if not Account.Stats then
		return
	end

	ImGui.Begin("Stats", 0)
	ImGuiEx.Text("Upgrade points - %d", Account.UpgradePoints)

	-- Str
	if ImGuiEx.ButtonEx(vec2(75, 20), "Upgrade##0") then
		Network.SendMsg("upgrade@stats", { ID = 0 })
		print("A")
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


Hook.Add("RenderLevel39", "MenusRender", function()
	if Menus.IsStatsOpen then
		RenderStats()
	end
end)


Hook.Add("OnInput", "Menus", function(Event)
	if Event.Flags == 1 and Event.Key == KEY_I then  -- LuaJIT dosen't support binop, only with module :(
		Menus.IsStatsOpen = not Menus.IsStatsOpen

		if Menus.IsStatsOpen then
			Game.Input:MouseModeAbsolute()
		else
			Game.Input:MouseModeRelative()
		end
	end
end)
