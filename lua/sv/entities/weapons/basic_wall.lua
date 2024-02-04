EntityManager.Register("basic_wall", {
	Pos = nil,
	Dir = nil,
	SnapID = 0,
	DamageType = -1,
	OwnerID = -1,
	MarkedToRemove = false,
	Damage = 0,

	OnInit = function(self, Pos, Dir, OwnerID, DamageType, Damage)
		self.Pos = copy_vector(Pos)
		self.Dir = copy_vector(Dir) * vec2(3, 3)
		self.SnapID = Game.Server:SnapNewID()
		self.OwnerID = OwnerID
		self.DamageType = DamageType
		self.Damage = Damage
	end,

	OnTick = function(self)
		self.Pos = self.Pos + self.Dir

		-- Check for characters
		local Chars = World.GetCharactersInRadius(self.Pos, 36, self.OwnerID)

		for _, v in pairs(Chars) do
			local WallEnt = World.GetClosestEntity(v.Pos, 36, self.OwnerID)
			print(WallEnt)
		end
	end,

	OnSnap = function(self, ClientID)
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

