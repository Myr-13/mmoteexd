__Ent = CreateDummyEntity()
__Ent.OnLogicInit = function(self)
	self.ChangeDirTick = 0
	self.Direction = 0
end


__Ent.OnLogicTick = function(self)
	local Tick = Game.Server.Tick

	if Tick > self.ChangeDirTick then
		self.ChangeDirTick = Tick + 50 + math.random(0, 100)

		self.Direction = (math.random(0, 1) == 0) and -1 or 1
	end

	self.Input.Direction = self.Direction
	self.Input.Jump = ((Game.Server.Tick % 15 == 0) and (math.random(0, 4) == 0)) and 1 or 0
	self.Input.TargetX = self.Input.Direction * 100

	-- Try to find player
	local Chr = World.GetClosestCharacter(self.Pos, 320)

	if Chr then
		local LocalPos = Chr.Pos - self.Pos

		if LocalPos.x < 0 then
			self.Input.Direction = -1
		else
			self.Input.Direction = 1
		end

		self.Input.TargetX = math.floor(LocalPos.x)
		self.Input.TargetY = math.floor(LocalPos.y)

		local Dist = distance(self.Pos, Chr.Pos)
		if Dist < 63 then
			self.Input.Fire = (Game.Server.Tick % 2 == 0) and 1 or 0
		end
	end
end


EntityManager.Register("dummy_simple", __Ent)
