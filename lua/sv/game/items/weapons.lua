SNAP_WEAPON_HAMMER = 0
SNAP_WEAPON_GUN = 1

AddItem(2, TYPE_WEAPON, "Hammer", "Standart weapon", {
	["DamageType"] = DAMAGE_TYPE_MELEE,
	["ReloadTime"] = 10,
	["AttackType"] = ATTACK_TYPE_MELEE,
	["Damage"] = 3,
	["SnapWeapon"] = SNAP_WEAPON_HAMMER
})

AddItem(3, TYPE_WEAPON, "Wand", "Magic weapon", {
	["DamageType"] = DAMAGE_TYPE_MAGIC,
	["ReloadTime"] = 15,
	["AttackType"] = ATTACK_TYPE_SPAWN_PROJECTILE,
	["Projectile"] = "basic_projectile",
	["ManaCost"] = 3,
	["Damage"] = 5,
	["SnapWeapon"] = SNAP_WEAPON_GUN
})

AddItem(4, TYPE_WEAPON, "Wooden bow", "Range weapon", {
	["DamageType"] = DAMAGE_TYPE_RANGE,
	["ReloadTime"] = 15,
	["AttackType"] = ATTACK_TYPE_SPAWN_PROJECTILE,
	["Projectile"] = "basic_projectile",
	["Damage"] = 3,
	["SnapWeapon"] = SNAP_WEAPON_GUN
})
