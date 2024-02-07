local function HandleTiles(CID, Ply, WorldID)
	local Chr = Ply.Character
	if not Chr then
		return
	end

	local Tile = Game.Collision(WorldID):GetTile(Chr.Pos.x, Chr.Pos.y)

	if Tile == TILE_PLUS_5 and Game.Server.Tick % 50 == 0 then
		GiveExp(CID, 5)

		Player.SendBroadcast(CID, "+5 exp from chair")
	end

	if Tile == TILE_PLUS_10 and Game.Server.Tick % 50 == 0 then
		GiveExp(CID, 10)

		Player.SendBroadcast(CID, "+10 exp from chair")
	end

	if Tile == TILE_WATER then
		Chr.Core.Vel.y = Chr.Core.Vel.y - 0.75

		if not Player.GetData(CID, "InWater") then
			Game.GameServer:CreateDeath(Chr.Pos, CID)
		end

		Player.SetData(CID, "InWater", true)
	else
		if Player.GetData(CID, "InWater") then
			Game.GameServer:CreateDeath(Chr.Pos, CID)
		end

		Player.SetData(CID, "InWater", false)
	end
end


local function HandleHUD(CID, Ply, WorldID)
	if Game.Server.Tick % 25 ~= 0 then
		return
	end

	if not Player.GetData(CID, "LoggedIn") then
		return
	end

	local Chr = Ply.Character
	if not Chr then
		return
	end

	local AccData = Player.GetData(CID, "AccData")
	local Equip = Player.GetData(CID, "Equip")

	local Lvl = AccData.Level
	local Exp = AccData.Exp
	local Health = math.floor(Chr.Health)
	local MaxHealth = Chr.MaxHealth
	local NeedExp = GetExpForLevelUp(Lvl)
	local Mana = math.floor(Player.GetData(CID, "Mana"))
	local MaxMana = Player.GetData(CID, "MaxMana")
	local Broadcast = Player.GetData(CID, "Broadcast")

	local Text = "Lv: %d | Exp: %d/%d\n"
	Text = Text .. "───────────────\n"
	Text = Text .. "[%s] %d%% XP\n"
	Text = Text .. "[%s] %d/%d HP\n"
	Text = Text .. "[%s] %d/%d MP\n"
	Text = Text .. "───────────────\n"

	if Broadcast and Broadcast.EndTime >= Game.Server.Tick then
		Text = Text .. Broadcast.Text
		Text = Text .. "\n───────────────\n"
	end

	-- Show active weapon
	local SelectedWeapon = Player.GetData(CID, "SelectedWeapon")
	for k, ItemID in pairs(Equip[SLOT_WEAPONS]) do
		if k == SelectedWeapon then
			Text = Text .. "► "
		end

		Text = Text .. GetItemName(ItemID) .. "\n"
	end

	Text = Text .. "                                                                                                                                                                                   "

	Server.SendBroadcast(CID, Text,
		Lvl, Exp, NeedExp,  -- Main stat
		Utils.GetProgressBar(10, "═", "  ", Exp, NeedExp), math.floor(Exp / NeedExp * 100),  -- Exp
		Utils.GetProgressBar(10, "═", "  ", Health, MaxHealth), Health, MaxHealth,  -- Health
		Utils.GetProgressBar(10, "═", "  ", Mana, MaxMana), Mana, MaxMana  -- Mana
	)

	Ply.Score = Lvl
end


local function HandleRegen(CID, Ply, WorldID)
	if Game.Server.Tick % 50 ~= 0 then
		return
	end

	if not Player.GetData(CID, "LoggedIn") then
		return
	end

	local Chr = Ply.Character
	if not Chr then
		return
	end

	local Stats = Player.GetData(CID, "Stats")

	-- Mana
	local Mana = Player.GetData(CID, "Mana")
	local MaxMana = Player.GetData(CID, "MaxMana")
	local ManaRegen = 0.1 + Statistic.ManaRegen(Stats.Int)

	if Mana < MaxMana then
		Mana = math.clamp(Mana + ManaRegen, 0, MaxMana)
	end

	Player.SetData(CID, "Mana", Mana)

	-- Health
	local HealthRegen = Statistic.HealthRegen(Stats.Str)

	if Chr.Health < Chr.MaxHealth then
		Chr.Health = math.clamp(Chr.Health + HealthRegen, 0, Chr.MaxHealth)
	end
end


local function PlayersTick()
	for i = 0, MAX_CLIENTS - 1 do
		local Ply = Game.Players(i)

		if Ply then
			local WorldID = Ply.WorldID

			HandleTiles(i, Ply, WorldID)
			HandleHUD(i, Ply, WorldID)
			HandleRegen(i, Ply, WorldID)
		end
	end
end


local function HandleWeaponSwitch(CID)
	local Ply = Game.Players(CID)
	local Chr = Ply.Character
	if not Chr then
		return
	end

	if not Player.GetData(CID, "LoggedIn") then
		return
	end

	local Next = CountInput(Chr.LatestPrevInput.NextWeapon, Chr.LatestInput.NextWeapon).Presses
	local Prev = CountInput(Chr.LatestPrevInput.PrevWeapon, Chr.LatestInput.PrevWeapon).Presses
	local WantedWeapon = Player.GetData(CID, "SelectedWeapon") - 1
	local NumWeapons = #(Player.GetData(CID, "Equip")[SLOT_WEAPONS])

	while Next > 0 do
		WantedWeapon = (WantedWeapon + 1) % NumWeapons

		Next = Next - 1
	end

	while Prev > 0 do
		WantedWeapon = ((WantedWeapon - 1) < 0) and (NumWeapons - 1) or (WantedWeapon - 1)

		Prev = Prev - 1
	end

	Player.SetData(CID, "SelectedWeapon", WantedWeapon + 1)

	Chr.Core.ActiveWeapon = 0
end


local function PlayerWeapons(CID)
	local SelectedWeapon = Player.GetData(CID, "SelectedWeapon")
	local ItemID = Player.GetData(CID, "Equip")[SLOT_WEAPONS][SelectedWeapon]

	FireWeapon(CID, ItemID)

	-- Don't call C++ character fire weapon logic
	return true
end


function UpdatePlayerStats(CID)
	local Ply = Game.Players(CID)
	local Stats = Player.GetData(CID, "Stats")

	-- Update max health
	local OldMaxHealth = Ply.MaxHealth
	Ply.MaxHealth = 10 + Statistic.Health(Stats.Str)

	-- Update max mana
	Player.SetData(CID, "MaxMana", 25 + Statistic.Mana(Stats.Int))

	-- Check for player
	local Chr = Ply.Character
	if not Chr then
		return
	end

	-- Update health
	if math.ceil(Chr.Health) == math.ceil(OldMaxHealth) then
		Chr.Health = Ply.MaxHealth
	end
end


local function InitPlayerVars(CID)
	Player.SetData(CID, "Mana", 0)

	UpdatePlayerStats(CID)
end


Hook.Add("OnTick", "PlayersTick", PlayersTick)
Hook.Add("OnCharacterFireWeapon", "PlayersCustomWeapons", PlayerWeapons)
Hook.Add("OnCharacterInput", "ChangeWeapons", HandleWeaponSwitch)
Hook.Add("OnPlayerLoggedIn", "InitPlayer", InitPlayerVars)
