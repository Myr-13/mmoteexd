EntityManager.Register("work_item", {
	Pos = nil,
	SnapID = 0,
	WorkID = 0,
	BreakProgress = 0,
	SpawnTick = 0,
	Alive = false,

	OnInit = function(self, Pos, WorkID)
		self.Pos = copy_vector(Pos)
		self.SnapID = Game.Server:SnapNewID()
		self.WorkID = WorkID
	end,

	OnTick = function(self)
		local OldAlive = self.Alive
		self.Alive = (Game.Server.Tick > self.SpawnTick)

		if not OldAlive and self.Alive then
			self.BreakProgress = 0
		end
	end,

	OnSnap = function(self, ClientID)
		if Game.GameServer:NetworkClipped(ClientID, self.Pos) then
			return
		end

		if not self.Alive then
			return
		end
		
		local Item = Game.Server:SnapNewItemPickup(self.SnapID)

		Item.x = math.floor(self.Pos.x)
		Item.y = math.floor(self.Pos.y)
		Item.Type = (self.WorkID == JOB_FARMER or self.WorkID == JOB_MINER) and 1 or 0
		Item.Subtype = 0
	end
})
