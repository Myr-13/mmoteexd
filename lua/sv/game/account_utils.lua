function GiveMoney(CID, Count)
	local AccData = Player.GetData(CID, "AccData")
	AccData.Money = AccData.Money + Count
	Player.SetData(CID, "AccData", AccData)
end


function GiveItem(CID, ID, Count)
	local Inv = Player.GetData(CID, "Inventory")

	-- By default count is 1
	if not Count then
		Count = 1
	end

	if not Inv[ID] then
		Inv[ID] = {
			ID = ID,
			Count = Count
		}
	else
		Inv[ID].Count = Inv[ID].Count + Count
	end

	Player.SetData(CID, "Inventory", Inv)
end


function RemoveItem(CID, ID, Count)
	local Inv = Player.GetData(CID, "Inventory")

	-- By default count is 1
	if not Count then
		Count = 1
	end

	if not Inv[ID] then
		return
	end

	Inv[ID].Count = Inv[ID].Count - Count

	if Inv[ID].Count <= 0 then
		Inv[ID] = nil
	end

	Player.SetData(CID, "Inventory", Inv)
end


function GetExpForLevelUp(Level)
	local Exp = 250

	if Level > 100 then Exp = 700 end
	if Level > 200 then Exp = 1000 end
	if Level > 300 then Exp = 1500 end
	if Level > 400 then Exp = 2500 end
	if Level > 500 then Exp = 4000 end
	if Level > 600 then Exp = 6000 end
	if Level > 700 then Exp = 8000 end
	if Level > 1000 then Exp = 12000 end
	if Level > 1100 then Exp = 13000 end
	if Level > 1200 then Exp = 14000 end

	return Level * Exp
end


function GiveExp(CID, Count)
	local AccData = Player.GetData(CID, "AccData")

	AccData.Exp = AccData.Exp + Count

	while AccData.Exp >= GetExpForLevelUp(AccData.Level) do
		AccData.Exp = AccData.Exp - GetExpForLevelUp(AccData.Level)
		AccData.Level = AccData.Level + 1
		AccData.UpgradePoints = AccData.UpgradePoints + 2

		Server.SendChat(CID, "Level Up!")
		Server.SendChat(CID, AccData.Level - 1, " -> ", AccData.Level)
		Server.SendChat(CID, "+2 Upgrade Points")
	end

	Player.SetData(CID, "AccData", AccData)
end


function RemoveMana(CID, Count)
	local Mana = Player.GetData(CID, "Mana")

	if Mana - Count < 0 then
		return false
	end

	Mana = Mana - Count

	Player.SetData(CID, "Mana", Mana)

	return true
end


function GetDamage(CID, DamageType, Damage)
	local Stats = Player.GetData(CID, "Stats")

	if DamageType == DAMAGE_TYPE_MELEE then
		Damage = Damage + (Damage / 100 * Statistic.MeleeDamage(Stats.Str))
	elseif DamageType == DAMAGE_TYPE_RANGE then
		Damage = Damage + (Damage / 100 * Statistic.RangeDamage(Stats.Dex))
	elseif DamageType == DAMAGE_TYPE_MAGIC then
		Damage = Damage + (Damage / 100 * Statistic.MagicDamage(Stats.Int))
	elseif DamageType == DAMAGE_TYPE_BARD then
		-- TODO: Add formula for this shit
	end

	return math.floor(Damage)
end


function GetReloadTime(CID, ReloadTime)
	local Stats = Player.GetData(CID, "Stats")
	return math.max(math.floor(ReloadTime - (ReloadTime / 100 * Statistic.AttackSpeed(Stats.Dex))), 0)
end
