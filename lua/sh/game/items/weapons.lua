-- Weapons min ID = 20000

AddItem(20000, "hammer", TYPE_WEAPON, "Hammer", "Standart weapon", {
	["DamageType"] = DAMAGE_TYPE_MELEE,
	["ReloadTime"] = 10,
	["AttackType"] = ATTACK_TYPE_MELEE,
	["Damage"] = 3,
	["SnapWeapon"] = SNAP_WEAPON_HAMMER
})

AddItem(20001, "wand", TYPE_WEAPON, "Wand", "Magic weapon", {
	["DamageType"] = DAMAGE_TYPE_MAGIC,
	["ReloadTime"] = 15,
	["AttackType"] = ATTACK_TYPE_SPAWN_PROJECTILE,
	["Projectile"] = "basic_projectile",
	["ManaCost"] = 3,
	["Damage"] = 5,
	["SnapWeapon"] = SNAP_WEAPON_GUN
})

AddItem(20002, "wooden_bow", TYPE_WEAPON, "Wooden bow", "Range weapon", {
	["DamageType"] = DAMAGE_TYPE_RANGE,
	["ReloadTime"] = 15,
	["AttackType"] = ATTACK_TYPE_SPAWN_PROJECTILE,
	["Projectile"] = "basic_projectile",
	["Damage"] = 3,
	["SnapWeapon"] = SNAP_WEAPON_GUN
})
