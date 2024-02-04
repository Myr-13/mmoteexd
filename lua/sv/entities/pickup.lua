EntityManager.Register("pickup", {
	Pos = nil,
	SnapID = 0,
	TargetID = -1,
	Speed = 10,

	OnInit = function(self, Pos, CID)
		self.Pos = copy_vector(Pos)
		self.SnapID = Game.Server:SnapNewID()
		self.TargetID = CID
	end,

	OnTick = function(self)
		local Ply = Game.Players(self.TargetID)
		if not Ply then
			self.MarkedToRemove = true
			return
		end

		local Chr = Ply.Character
		if not Chr then
			self.MarkedToRemove = true
			return
		end

		local Dir = normalize(Chr.Pos - self.Pos)
		self.Pos = self.Pos + Dir * vec2(self.Speed, self.Speed)

		if distance(self.Pos, Chr.Pos) <= 26 then
			self.MarkedToRemove = true

			Game.GameServer:CreateSound(self.Pos, SOUND_PICKUP_HEALTH)
			Game.GameServer:CreateDeath(self.Pos, self.TargetID)
		end
	end,

	OnSnap = function(self, ClientID)
		if Game.GameServer:NetworkClipped(ClientID, self.Pos) then
			return
		end

		local Item = Game.Server:SnapNewItemProjectile(self.SnapID)

		Item.x = math.floor(self.Pos.x)
		Item.y = math.floor(self.Pos.y)
		Item.VelX = 0
		Item.VelY = 0
		Item.Type = 0
		Item.StartTick = Game.Server.Tick
	end
})
