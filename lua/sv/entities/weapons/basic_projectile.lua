EntityManager.Register("basic_projectile", {
	Pos = nil,
	Dir = nil,
	SnapID = 0,
	DamageType = -1,
	OwnerID = -1,
	MarkedToRemove = false,
	Damage = 0,
	WorldID = 0,

	OnInit = function(self, Pos, WorldID, Dir, OwnerID, DamageType, Damage)
		self.Pos = copy_vector(Pos)
		self.Dir = copy_vector(Dir) * vec2(17, 17)
		self.SnapID = Game.Server:SnapNewID()
		self.OwnerID = OwnerID
		self.DamageType = DamageType
		self.Damage = Damage
		self.WorldID = WorldID
	end,

	OnTick = function(self)
		self.Pos = self.Pos + self.Dir

		-- Check for walls
		if Game.Collision(self.WorldID):IntersectLine(self.Pos, self.Pos + self.Dir, nil, nil) ~= TILE_AIR then
			self.MarkedToRemove = true
			return
		end

		-- Check for dummies
		local Ent = World.GetClosestEntity(self.Pos, "dummy_simple", 28)

		if Ent then
			Game.GameServer:CreateSound(self.Pos, SOUND_HIT)
			Ent:Damage(vec2(0, 0), self.Damage, self.OwnerID)
			self.MarkedToRemove = true
			return
		end

		-- Check for characters
		Ent = World.GetFirstCharacterInRadius(self.Pos, 28, self.OwnerID)

		if Ent then
			Game.GameServer:CreateSound(self.Pos, SOUND_HIT)
			Ent:TakeDamage(vec2(0, 0), self.Damage, self.OwnerID)
			self.MarkedToRemove = true
			return
		end
	end,

	OnSnap = function(self, ClientID)
		if self:NetworkClipped(ClientID) then
			return
		end

		local Item = Game.Server:SnapNewItemLaser(self.SnapID)

		Item.ToX = math.floor(self.Pos.x)
		Item.ToY = math.floor(self.Pos.y)
		Item.FromX = math.floor(self.Pos.x - self.Dir.x * 2)
		Item.FromY = math.floor(self.Pos.y - self.Dir.y * 2)
		Item.StartTick = Game.Server.Tick
		Item.Owner = self.OwnerID
		Item.Type = -1
		Item.SwitchNumber = -1
		Item.Subtype = -1
		Item.Flags = 0
	end
})
