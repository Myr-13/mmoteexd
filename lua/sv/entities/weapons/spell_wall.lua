EntityManager.Register("spell_healing_rain", {
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
	SnapIDs2 = nil,

	OnInit = function(self, Pos, Dir, OwnerID, DamageType, Damage)
		self.Pos = copy_vector(Pos)
		self.Dir = copy_vector(Dir) * vec2(10, 10)
		self.SnapID = Game.Server:SnapNewID()
		self.OwnerID = OwnerID
		self.DamageType = DamageType
		self.Damage = Damage
		self.StartedTick = Game.Server.Tick
		self.EndTick = Game.Server.Tick + 75
		self.SnapIDs = {}
		for i = 2, self.SegmentsHalf - 1 do
			self.SnapIDs[i] = Game.Server:SnapNewID()
		end
		self.SnapIDs2 = {}
		for i = 1, 16 do
			self.SnapIDs2[i] = Game.Server:SnapNewID()
		end
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

		-- Check for walls
		if Game.Collision:IntersectLine(self.Pos, self.Pos + vec2(0, 12), nil, nil) ~= TILE_AIR then
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
			Ent.Core.Vel = vec2(0, 0)
			return
		end

		-- Check for characters
		Ent = World.GetFirstCharacterInRadius(self.Pos, 72, self.OwnerID)
		if Ent then
			if Game.Server.Tick % 10 == 0 then
				Ent:TakeDamage(vec2(0, 0), self.Damage, self.OwnerID)
			end
			Ent.Core.Pos = self.Pos - self.Dir
			Ent.Core.Vel = vec2(0, 0)
			return
		end
	end,

	OnSnap = function(self, ClientID)
		if self:NetworkClipped(ClientID) then
			return
		end
		
		for i = 2, self.SegmentsHalf - 1 do
			angle = i * math.pi / (self.SegmentsHalf - 1)
			angle2 = (i + 1) * math.pi / (self.SegmentsHalf - 1)
			angle3 = (i - 1) * math.pi / (self.SegmentsHalf - 1)
		  
			x = math.cos(angle) * 8
			y = math.sin(angle) * 8

			x2 = math.cos(angle2) * 8
            y2 = math.sin(angle2) * 8

			x3 = math.cos(angle3) * 8
			y3 = math.sin(angle3) * 8

			rotatedX = -self.Dir.x * x - self.Dir.y * y
        	rotatedY = self.Dir.x * y - self.Dir.y * x

			rotatedX2 = -self.Dir.x * x2 - self.Dir.y * y2
        	rotatedY2 = self.Dir.x * y2 - self.Dir.y * x2

			rotatedX3 = -self.Dir.x * x3 - self.Dir.y * y3
			rotatedY3 = self.Dir.x * y3 - self.Dir.y * x3

			local Item = Game.Server:SnapNewItemLaser(self.SnapIDs[i])
			Item.ToX = math.floor(self.Pos.x) + math.floor(rotatedX)
			Item.ToY = math.floor(self.Pos.y) + math.floor(rotatedY)
			Item.FromX = math.floor(self.Pos.x) + math.floor(rotatedX2)
			Item.FromY = math.floor(self.Pos.y) + math.floor(rotatedY2)
			Item.StartTick = Game.Server.Tick
			Item.Owner = self.OwnerID
			Item.Type = -1
			Item.SwitchNumber = -1
			Item.Subtype = -1
			Item.Flags = 0
		end
		for i = 1, 6 do
			local t = math.random()
			local u = math.random()

			if t + u <= 1 then
				local x = (1 - t) * rotatedX3 + t * rotatedX2 + u * (1 - t) * rotatedX2 + u * t * rotatedX
       			local y = (1 - t) * rotatedY3 + t * rotatedY2 + u * (1 - t) * rotatedY2 + u * t * rotatedY

				local Item = Game.Server:SnapNewItemProjectile(self.SnapIDs2[i])
				Item.x = math.floor(self.Pos.x) + math.floor(x)
				Item.y = math.floor(self.Pos.y) + math.floor(y)
				Item.VelX = 0
				Item.VelY = 0
				Item.Type = 0
				Item.StartTick = Game.Server.Tick
			else
				i = i - 1
			end
		end
	end
})

