local _SpellsData = {}


local function LoadSpells()
	_SpellsData = table.clear(_SpellsData)

	_G["AddSpell"] = function(Data)
		_SpellsData[Data.Cmd] = Data
	end

	include("sv/game/spells/spells.lua")

	_G["AddSpell"] = nil
end


local function ConCastSpell(Result)
	local CID = Result.ClientID

	-- List avaible spells, if arguments is not provided
	if Result.NumArguments < 1 then
		Server.SendChat(CID, "List of avaible spells:")

		for k, _ in pairs(_SpellsData) do
			Server.SendChat(CID, "- %s", k)
		end

		return
	end

	local SpellID = Result:GetString(0)
	local Spell = _SpellsData[SpellID]

	-- Check for valid spell
	if not Spell then
		Server.SendChat(CID, "Invalid spell ID")
		return
	end

	-- Check for valid character
	local Chr = Game.Players(CID).Character
	if not Chr then
		return
	end

	local Dir = normalize(vec2(Chr.Input.TargetX, Chr.Input.TargetY))
	local ManaCost = Spell.ManaCost

	if ManaCost and not RemoveMana(CID, ManaCost) then
		Server.SendChat(CID, "Not enough mana")
		return
	end

	World.Spawn(Spell.Projectile, Chr.Pos, Dir, CID)
end


Hook.Add("OnConsoleInit", "SpellsCmds", function()
	LoadSpells()

	Console.RegisterChat("cast", "?s[spell_name]", "Cast a spell", ConCastSpell)
	Console.RegisterRcon("reload_spells", "", "Reload spells from lua", LoadSpells)
end)
