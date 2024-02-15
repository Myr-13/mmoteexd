Statistic = {}

-------------------------------------------------------------------------------------------------------------
--                                                    MELEE                                                --
-------------------------------------------------------------------------------------------------------------
Statistic.MeleeDamage = function(Lvl)
	return Lvl
end


Statistic.Health = function(Lvl)
	return Lvl * 2
end


Statistic.HealthRegen = function(Lvl)
	return Lvl * 0.1
end

-------------------------------------------------------------------------------------------------------------
--                                                    RANGE                                                --
-------------------------------------------------------------------------------------------------------------
Statistic.RangeDamage = function(Lvl)
	return Lvl
end


Statistic.CritChance = function(Lvl)
	return Lvl * 0.15
end


Statistic.AttackSpeed = function(Lvl)
	return Lvl * 0.5
end

-------------------------------------------------------------------------------------------------------------
--                                                    MAGIC                                                --
-------------------------------------------------------------------------------------------------------------
Statistic.MagicDamage = function(Lvl)
	return Lvl
end


Statistic.Mana = function(Lvl)
	return Lvl * 3
end


Statistic.ManaRegen = function(Lvl)
	return Lvl * 0.1
end
