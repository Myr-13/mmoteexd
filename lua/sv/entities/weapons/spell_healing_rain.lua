EntityManager.Register("spell_healing_rain", {
	Pos = nil,
	SnapID = 0,
	DamageType = -1,
	OwnerID = -1,
	MarkedToRemove = false,
	Heal = 0,
	StartedTick = 0,
	EndTick = 0,
	SnapIDs = nil,
	FallY = nil,
	FallAcceleration = 25,

	OnInit = function(self, Pos, Dir, OwnerID, DamageType, Damage)
		self.Pos = copy_vector(Pos)
		self.SnapID = Game.Server:SnapNewID()
		self.OwnerID = OwnerID
		self.DamageType = DamageType
		self.Heal = Damage
		self.StartedTick = Game.Server.Tick
		self.EndTick = Game.Server.Tick + 300
		self.SnapIDs = {}
		for i = 1, 10 do
			self.SnapIDs[i] = Game.Server:SnapNewID()
		end

		self.FallY = {}
		for i = 1, 10 do
		    self.FallY[i] = 0
		end
	end,

	OnTick = function(self)
		if Game.Server.Tick > self.EndTick + 100 then
			self.MarkedToRemove = true
			return
		end

		if Game.Server.Tick < self.EndTick - 200 then
			self.StartedTick = Game.Server.Tick - self.EndTick + 300
		end
		if Game.Server.Tick > self.EndTick then
			self.StartedTick = Game.Server.Tick - self.EndTick - 100
		end
		
		for i = 1, 10 do
			self.FallY[i] = self.FallAcceleration * i
			self.FallAcceleration = 25
		end

		-- Check for characters
		Ent = World.GetFirstCharacterInRadius(self.Pos, self.StartedTick)
		if Ent then
			if Game.Server.Tick % 50 == 0 then
				Ent.Health = math.clamp(Ent.Health + self.Heal, 0, Ent.MaxHealth)
			end
			return
		end
	end,

	OnSnap = function(self, ClientID)
		for i = 1, 10 do
			local t = math.random()

			x = (1 - t) * self.Pos.x - self.StartedTick + t * self.Pos.x + (self.StartedTick * 2) * t
			y = (1 - t) * self.Pos.y - 100 + t * self.Pos.y - 100

			local Item = Game.Server:SnapNewItemLaser(self.SnapIDs[i])
			Item.ToX = math.floor(x)
			Item.ToY = math.floor(y - 20) + self.FallY[i]
			Item.FromX = math.floor(x)
			Item.FromY = math.floor(y - 40) + self.FallY[i]
			Item.StartTick = Game.Server.Tick
			Item.Owner = self.OwnerID
			Item.Type = -1
			Item.SwitchNumber = -1
			Item.Subtype = -1
			Item.Flags = 0
		end
	end
})

