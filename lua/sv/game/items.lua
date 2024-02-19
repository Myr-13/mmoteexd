-- Other utility functions
function EquipItem(CID, ID)
	local ItemData = GetItemData(ID)
	if not ItemData then
		return
	end

	local Equip = Player.GetData(CID, "Equip")
	local Slot = ItemData["EquipSlot"]
	local Equipment = Equip[Slot]

	if #Equipment >= ItemData["EquipCount"] then
		Equipment = table.clear(Equipment)
	else
		table.insert(Equipment, ID)
	end

	Equip[Slot] = Equipment
	Player.SetData(CID, "Equip", Equip)
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
			Ent:Damage(CID, 10)

			Chr.ReloadTimer = 50

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
		World.Spawn(Projectile, Chr.Pos, Game.Players(CID).WorldID, Dir, CID, DamageType, Damage)
	end
end
