function CreateDummyEntity()
	return {
		Core = nil,
		Pos = vec2(0, 0),
		SpawnPos = vec2(0, 0),
		SpawnTick = 0,
		SnapID = 0,
		Alive = false,
		Input = nil,
		OldInput = nil,
		Health = 10,
		Data = nil,
		AttackTick = 0,
		ReloadTime = 0,
		WorldID = 0,
	
		Damage = function(self, Force, Dmg, CID)
			self.Health = self.Health - Dmg
			self.Core.Vel = self.Core.Vel + Force
	
			if self.Health <= 0 then
				self:OnDeath(CID)
			end
		end,
	
		OnDeath = function(self, CID)
			self.Alive = false
			self.SpawnTick = Game.Server.Tick + 50
	
			if CID >= 0 then
				Game.GameServer:CreateSound(self.Pos, SOUND_PLAYER_DIE)
				Game.GameServer:CreateDeath(self.Pos, CID)

				World.Spawn("pickup", self.Pos, self.WorldID, CID)
				GiveExp(CID, self.Data.Level * 10)
			end
		end,
	
		OnSpawn = function(self)
			self.Alive = true
			self.Core.Pos = copy_vector(self.SpawnPos)
			self.Pos = copy_vector(self.SpawnPos)
			self.Core.Vel = vec2(0, 0)
			self.Health = self.Data.Health
	
			Game.GameServer:CreateSound(self.Pos, SOUND_PLAYER_SPAWN)
			Game.GameServer:CreatePlayerSpawn(self.Pos)
		end,

		OnFireWeapon = function(self)
			if self.OldInput.Fire == self.Input.Fire or self.Input.Fire == 0 then
				return
			end

			-- Reload
			if self.ReloadTime > 0 then
				self.ReloadTime = self.ReloadTime - 1
				return
			end

			-- Set vars
			self.AttackTick = Game.Server.Tick
			self.ReloadTime = 4

			-- Handle hammer hit
			local Dir = normalize(vec2(self.Input.TargetX, self.Input.TargetY))
			local FirePoint = self.Pos + Dir * vec2(28, 28) * vec2(0.75, 0.75)
			local Chrs = World.GetCharactersInRadius(FirePoint, 42, -1)

			for _, Ent in pairs(Chrs) do
				local Force = normalize(Dir + vec2(0, -1.1)) * vec2(10, 10)
				Force = vec2(0, -1) + Force

				Ent:TakeDamage(Force, 3, -1)

				Game.GameServer:CreateHammerHit(FirePoint)
			end
		end,
	
		OnInit = function(self, Pos, WorldID, Data)
			self.Core = CCharacterCore()
			self.Input = CNetObj_PlayerInput()
			self.OldInput = CNetObj_PlayerInput()
			self.Core:Init(Game.World(WorldID).Core, Game.Collision(WorldID), nil, nil)
			self.Core.Pos = copy_vector(Pos)
			self.Pos = copy_vector(Pos)
			self.SpawnPos = copy_vector(Pos)
			self.SnapID = Game.Server:SnapNewID()
			self.Data = Data
			self.WorldID = WorldID
	
			Game.World(WorldID).Core:AddDummy(self.Core)

			if self.OnLogicInit then
				self:OnLogicInit()
			end
		end,
	
		OnTick = function(self)
			self.Core.Alive = self.Alive
	
			-- Spawn
			if Game.Server.Tick >= self.SpawnTick and not self.Alive then
				self:OnSpawn()
			end
	
			-- Don't calc physic if ded
			if not self.Alive then
				return
			end
	
			-- Reset input
			self.Input.Fire = 0
			self.Input.Hook = 0
			self.Input.Direction = 0
			self.Input.Jump = 0
	
			-- Call logic update
			if self.OnLogicTick then
				self:OnLogicTick()
			end
	
			-- Update physic
			self.Core.Input = self.Input
			self.Core:Tick(true, true)
			self.Core:Move()
			self.Core:Quantize()
			self.Pos = copy_vector(self.Core.Pos)

			-- Handle tiles
			if Game.Collision(self.WorldID):GetTile(self.Pos.x, self.Pos.y) == TILE_PVP_ON then
				self:OnDeath(-1)
			end

			self:OnFireWeapon()

			copy_input(self.OldInput, self.Input)
		end,
	
		OnSnap = function(self, ClientID)
			if self:NetworkClipped(ClientID) then
				return
			end

			local SelfID = Game.GameServer:GetNextBotSnapID(ClientID)
			if SelfID == -1 then
				return
			end
	
			-- Snap client
			local ClientInfo = Game.Server:SnapNewItemClientInfo(SelfID)
			ClientInfo:SetName(4, self.Data.Name)  -- 4 ints * 4 bytes per int = 16 bytes. last byte is null-termination(\0 at the end)
			ClientInfo:SetClan(3, "Mob Lv " .. tostring(self.Data.Level))
			ClientInfo:SetSkin(6, self.Data.Skin)
			ClientInfo.Country = 0
			ClientInfo.UseCustomColor = self.Data.UseCustomColor or 0
			ClientInfo.ColorBody = self.Data.ColorBody or 0
			ClientInfo.ColorFeet = self.Data.ColorFeet or 0
	
			-- Snap player
			local PlayerInfo = Game.Server:SnapNewItemPlayerInfo(SelfID)
			PlayerInfo.Latency = 0
			PlayerInfo.Score = 0
			PlayerInfo.Local = 0
			PlayerInfo.ClientID = SelfID
			PlayerInfo.Team = 10
	
			if not self.Alive then
				return
			end
	
			-- Snap character
			local Chr = Game.Server:SnapNewItemCharacter(SelfID)
			self.Core:Write(Chr)
	
			Chr.Tick = Game.Server.Tick
			Chr.Emote = 0
			Chr.HookedPlayer = -1
			Chr.Direction = self.Input.Direction
			Chr.Weapon = 0
			Chr.AmmoCount = 0
			Chr.Health = self.Health
			Chr.Armor = 0
			Chr.PlayerFlags = 1  -- PLAYERFLAG_PLAYING = 1
			Chr.AttackTick = self.AttackTick
		end
	}
end
