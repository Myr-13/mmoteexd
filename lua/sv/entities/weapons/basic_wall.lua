EntityManager.Register("basic_wall", {
	Pos = nil,
	Dir = nil,
	SnapID = 0,
	DamageType = -1,
	OwnerID = -1,
	MarkedToRemove = false,
	Damage = 0,
	StartedTick = 0,
	EndTick = 0,

	OnInit = function(self, Pos, Dir, OwnerID, DamageType, Damage)
		self.Pos = copy_vector(Pos)
		self.Dir = copy_vector(Dir) * vec2(10, 10)
		self.SnapID = Game.Server:SnapNewID()
		self.OwnerID = OwnerID
		self.DamageType = DamageType
		self.Damage = Damage
		self.StartedTick = Game.Server.Tick
		self.EndTick = Game.Server.Tick + 75
	end,

	OnTick = function(self)
		self.Pos = self.Pos + self.Dir

		if Game.Server.Tick > self.EndTick then
			self.MarkedToRemove = true
			return
		end

		-- Check for walls
		if Game.Collision:IntersectLine(self.Pos, self.Pos + self.Dir * vec2(6, 6), nil, nil) ~= TILE_AIR then
			self.MarkedToRemove = true
			return
		end

		-- Check for dummies
		local Ent = World.GetClosestEntity(self.Pos, "dummy_simple", 72)

		if Ent then
			if Game.Server.Tick % 10 == 0 then
				Ent:Damage(vec2(0, 0), self.Damage, self.OwnerID)
			end
			Ent.Core.Pos = self.Pos - self.Dir
			return
		end

		-- Check for characters
		Ent = World.GetFirstCharacterInRadius(self.Pos, 72, self.OwnerID)
		if Ent then
			if Game.Server.Tick % 10 == 0 then
				Ent:TakeDamage(vec2(0, 0), self.Damage, self.OwnerID)
			end
			Ent.Core.Pos = self.Pos - self.Dir
			return
		end
	end,

	OnSnap = function(self, ClientID)
		local Item = Game.Server:SnapNewItemLaser(self.SnapID)

		Item.ToX = math.floor(self.Pos.x - self.Dir.y * 5)
		Item.ToY = math.floor(self.Pos.y + self.Dir.x * 5)
		Item.FromX = math.floor(self.Pos.x + self.Dir.y * 5)
		Item.FromY = math.floor(self.Pos.y - self.Dir.x * 5)
		Item.StartTick = Game.Server.Tick
		Item.Owner = self.OwnerID
		Item.Type = -1
		Item.SwitchNumber = -1
		Item.Subtype = -1
		Item.Flags = 0
	end
})

