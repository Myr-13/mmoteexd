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
	Fov = 160,
	SegmentsHalf = 4,
	SnapIDs = nil,
	WorldID = 0,

	OnInit = function(self, Pos, WorldID, Dir, OwnerID, DamageType, Damage)
        self.Pos = copy_vector(Pos)
        self.Dir = copy_vector(Dir) * vec2(10, 10)
        self.SnapID = Game.Server:SnapNewID()
        self.OwnerID = OwnerID
        self.DamageType = DamageType
        self.Damage = Damage
        self.StartedTick = Game.Server.Tick
        self.EndTick = Game.Server.Tick + 75
        self.WorldID = WorldID

		for i = -self.SegmentsHalf, self.SegmentsHalf do
			self.SnapIDs[i] = Game.Server:SnapNewID()
		end
	end,

	OnTick = function(self)
		self.Pos = self.Pos + self.Dir

		if Game.Server.Tick > self.EndTick then
			self.MarkedToRemove = true
			return
		end

		-- Check for walls
		if Game.Collision(self.WorldID):IntersectLine(self.Pos, self.Pos + self.Dir * vec2(6, 6), nil, nil) ~= TILE_AIR then
			self.MarkedToRemove = true
			return
		end

		-- Check for dummies
		local Ent = World.GetClosestEntity(self.Pos, "dummy_simple", 72)

		if Ent then
			if Game.Server.Tick % 5 == 0 then
				Ent:Damage(vec2(0, 0), self.Damage, self.OwnerID)
			end
			Ent.Core.Pos = self.Pos
			return
		end

		-- Check for characters
		Ent = World.GetFirstCharacterInRadius(self.Pos, 72, self.OwnerID)
		if Ent then
			if Game.Server.Tick % 5 == 0 then
				Ent:TakeDamage(vec2(0, 0), self.Damage, self.OwnerID)
			end
			Ent.Core.Pos = self.Pos
			return
		end
	end,

	OnSnap = function(self, ClientID)
        if self:NetworkClipped(ClientID) then
            return
        end

		self.Fov = self.Fov * math.pi / 180
		Step = self.Fov / self.SegmentsHalf
		for i = -self.SegmentsHalf, self.SegmentsHalf do
			angle = Step * i

			x = cos(angle)
			y = sin(angle)

			local Item = Game.Server:SnapNewItemLaser(self.SnapIDs[i])

			Item.ToX = math.floor(self.Pos.x) + x
			Item.ToY = math.floor(self.Pos.y) + y
			Item.FromX = math.floor(self.Pos.x)
			Item.FromY = math.floor(self.Pos.y)
			Item.StartTick = Game.Server.Tick
			Item.Owner = self.OwnerID
			Item.Type = -1
			Item.SwitchNumber = -1
			Item.Subtype = -1
			Item.Flags = 0
		  end
	end
})

