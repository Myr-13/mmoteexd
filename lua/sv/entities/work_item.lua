EntityManager.Register("work_item", {
	Pos = nil,
	SnapID = 0,
	WorkID = 0,
	BreakProgress = 0,
	SpawnTick = 0,
	Alive = false,

	Damage = function(self, CID, Dmg)
		self.BreakProgress = self.BreakProgress + Dmg

		Game.GameServer:CreateSound(self.Pos, SOUND_HOOK_LOOP)

		if self.BreakProgress >= 1 then
			-- Sound & respawn time
			Game.GameServer:CreateSound(self.Pos, SOUND_NINJA_HIT)
			self.SpawnTick = Game.Server.Tick + 200

			-- Exp
			GiveWorkExp(CID, self.WorkID, 1)

			-- Give item
			local PickupType = GetWorkPickupType(self.WorkID)
			local Items = GetWorkItems(self.WorkID)

			if PickupType == WORK_PICKUP_TYPE_RANDOM_ITEM then
				GiveItem(CID, Items[math.random(1, #Items)], 1)
			else
				GiveItem(CID, Items[math.random(1, #Items)], 1)
			end
		end

		Player.SendBroadcast(CID, "[%s]\n%s work", Utils.GetProgressBar(10, "═", "  ", self.BreakProgress, 100), GetWorkName(self.WorkID))
	end,

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
		Item.Type = (self.WorkID == WORK_FARMER or self.WorkID == WORK_MINER) and 1 or 0
		Item.Subtype = 0
	end
})
