EntityManager.Register("basic_slowdown", {
	Pos = nil,
	Dir = nil,
	SnapID = 0,
	SnapIDs = nil,
	SnapIDs2 = nil,
	DamageType = -1,
	OwnerID = -1,
	MarkedToRemove = false,
	Damage = 0,
	StartedTick = 0,
	RemoveTick = 0,
	Segments = 10,
	Size = 1,

	OnInit = function(self, Pos, Dir, OwnerID)
		self.Pos = copy_vector(Pos)
		self.Dir = copy_vector(Dir)
		self.SnapID = Game.Server:SnapNewID()
		self.OwnerID = OwnerID
		self.StartedTick = Game.Server.Tick
		self.RemoveTick = Game.Server.Tick + 350
		self.SnapIDs = {}
		for i = 1, self.Segments do
			self.SnapIDs[i] = Game.Server:SnapNewID()
		end
		self.SnapIDs2 = {}
		for i = 1, self.Segments * 2 do
			self.SnapIDs2[i] = Game.Server:SnapNewID()
		end
	end,

	OnTick = function(self)
		if Game.Server.Tick < self.StartedTick + 128 then
			self.Size = (Game.Server.Tick - self.StartedTick)
		end
		if Game.Server.Tick > self.RemoveTick then
			self.Size = (self.RemoveTick - Game.Server.Tick + 128)
		end
		if self.Size == 0 then
			self.MarkedToRemove = true 
			return
		end

		-- Check for dummies
		local Ent = World.GetClosestEntity(self.Pos, "dummy_simple", self.Size)

		if Ent then
			Ent.Core.Vel = Ent.Core.Vel / vec2(2, 1.1)
			return
		end

		-- Check for characters
		Ent = World.GetFirstCharacterInRadius(self.Pos, self.Size)

		if Ent then
			Ent.Core.Vel = Ent.Core.Vel / vec2(2, 1.1)
			return
		end
	end,

	OnSnap = function(self, ClientID)
		local Step = (math.pi * 2) / self.Segments
        for i = 1, self.Segments do
            local a = Step * i
			local a2 = Step * (i + 1)

            local x = math.cos(a) * self.Size
            local y = math.sin(a) * self.Size

			local x2 = math.cos(a2) * self.Size
            local y2 = math.sin(a2) * self.Size
			

		    local Item = Game.Server:SnapNewItemLaser(self.SnapIDs[i])
            Item.ToX = math.floor(self.Pos.x) + math.floor(x)
            Item.ToY = math.floor(self.Pos.y) + math.floor(y)
            Item.FromX = math.floor(self.Pos.x) + math.floor(x2)
            Item.FromY = math.floor(self.Pos.y) + math.floor(y2)
            Item.StartTick = Game.Server.Tick
            Item.Owner = self.OwnerID
            Item.Type = -1
            Item.SwitchNumber = -1
            Item.Subtype = -1
            Item.Flags = 0
        end

		for i = 1, self.Segments * 2 do
			local angle = Step * i / 2
    
			local distance = math.sqrt(math.random()) * self.Size
			
			local x = distance * math.cos(angle)
			local y = distance * math.sin(angle)

			local Item = Game.Server:SnapNewItemProjectile(self.SnapIDs2[i])

			Item.x = math.floor(self.Pos.x) + math.floor(x)
			Item.y = math.floor(self.Pos.y) + math.floor(y)
			Item.VelX = 0
			Item.VelY = 0
			Item.Type = 0
			Item.StartTick = Game.Server.Tick
		end
	end
})
