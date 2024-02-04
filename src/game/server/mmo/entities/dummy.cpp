#include "dummy.h"

#include <lua/lua.hpp>
#include <engine/external/luabridge/LuaBridge.h>
#include <engine/shared/lua/lua_state.h>

#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/entities/character.h>
#include <game/server/entities/laser.h>
#include <game/server/entities/projectile.h>

CDummy::CDummy(CGameWorld *pWorld, vec2 Pos, const char *pLuaFile) :
	CLuaEntity(pWorld, Pos, nullptr, "dummy", CGameWorld::ENTTYPE_DUMMY)
{
	GameWorld()->m_Core.m_vDummies.push_back(&m_Core);
	m_Core.Init(&GameWorld()->m_Core, Collision());

	m_SpawnPos = Pos;
	m_DefaultEmote = EMOTE_NORMAL;
	m_Level = 0;
	m_MaxHealth = 10;
	m_MaxArmor = 10;
	m_Damage = 0;
	m_AttackTick = 0;

	str_copy(m_aName, "[NULL BOT]");
	str_copy(m_aClan, "");

	LoadLua(pLuaFile);

	Spawn();
}

void CDummy::RegisterAPI()
{
	luabridge::getGlobalNamespace(m_Lua.LuaState())
		.deriveClass<CDummy, CEntity>("CDummy")
		        .addProperty("Input", &CDummy::m_Input)
		.endClass();
}

void CDummy::Spawn()
{
	if(m_Alive)
		Die(-1);

	m_Health = m_MaxHealth;
	m_Armor = m_MaxArmor;
	m_Alive = true;
	m_SpawnTick = 0;
	m_EmoteType = EMOTE_NORMAL;
	m_EmoteStop = 0;

	m_Core.m_Pos = m_SpawnPos;
	m_Pos = m_SpawnPos;
	m_Core.m_Vel = vec2(0, 0);

	GameServer()->CreatePlayerSpawn(m_Pos);
}

void CDummy::Die(int Killer)
{
	m_Alive = false;
	m_SpawnTick = Server()->Tick() + Server()->TickSpeed();

	GameServer()->CreateDeath(m_Pos, 0);
	GameServer()->CreateSound(m_Pos, SOUND_PLAYER_DIE);
}

void CDummy::TakeDamage(vec2 Force, int Damage, int From, int Weapon)
{
	if(!m_Alive)
		return;

	//if(From >= 0)
	//	Damage += MMOCore()->GetPlusDamage(From);

	if(Damage)
	{
		// Emote
		m_EmoteType = EMOTE_PAIN;
		m_EmoteStop = Server()->Tick() + 500 * Server()->TickSpeed() / 1000;

		if(m_Armor)
		{
			if(Damage <= m_Armor)
			{
				m_Armor -= Damage;
				Damage = 0;
			}
			else
			{
				Damage -= m_Armor;
				m_Armor = 0;
			}
		}

		m_Health -= Damage;

		if(From >= 0 && GameServer()->m_apPlayers[From])
		{
			GameServer()->CreateSound(m_Pos, SOUND_HIT);

			//int Steal = (100 - Server()->GetItemCount(From, ACCESSORY_ADD_STEAL_HP) > 30) ? 100 - Server()->GetItemCount(From, ACCESSORY_ADD_STEAL_HP) > 30 : 30;
			//pFrom->m_Health += Steal;
		}
	}

	if(m_Health <= 0)
		Die(From);

	vec2 Temp = m_Core.m_Vel + Force;
	m_Core.m_Vel = Temp;
}

void CDummy::FireWeapon()
{
	vec2 Direction = normalize(vec2(m_Input.m_TargetX, m_Input.m_TargetY));

	bool FullAuto = false;
	if(m_Core.m_ActiveWeapon == WEAPON_GRENADE || m_Core.m_ActiveWeapon == WEAPON_SHOTGUN || m_Core.m_ActiveWeapon == WEAPON_LASER)
		FullAuto = true;

	bool WillFire = (m_Input.m_Fire & 1) && (!(m_PrevInput.m_Fire & 1) || FullAuto);
	if (!WillFire)
		return;

	vec2 ProjStartPos = m_Pos + Direction * 28.f * 0.75f;

	int Weapon = m_Core.m_ActiveWeapon;
	if (Weapon == WEAPON_GUN ||
		Weapon == WEAPON_SHOTGUN ||
		Weapon == WEAPON_GRENADE)
	{
		// Get lifetime
		int LifeTime;
		if (Weapon == WEAPON_GUN)
			LifeTime = Server()->TickSpeed() * GameServer()->Tuning()->m_GunLifetime;
		else if (Weapon == WEAPON_SHOTGUN)
			LifeTime = Server()->TickSpeed() * GameServer()->Tuning()->m_ShotgunLifetime;
		else
			LifeTime = Server()->TickSpeed() * GameServer()->Tuning()->m_GrenadeLifetime;

		const float Fov = 60.f;
		int Spray = 1 + ((Weapon == WEAPON_SHOTGUN) ? 5 : 0);

		// Create projectile
		new CProjectile(
			GameWorld(),
			Weapon, // Type
			-1, // Owner
			ProjStartPos, // Pos
			Direction, // Dir
			LifeTime, // Span
			false, // Freeze
			(Weapon == WEAPON_GRENADE), // Explosive
			-1, // SoundImpact
			ProjStartPos
		);

		// Spread
		const float fovRad = (Fov / 2.f) * pi / 180.f;
		const float aps = fovRad / Spray * 2;
		const float a = angle(Direction);

		for (int i = 1; i <= Spray; i++) {
			float m = (i % 2 == 0) ? -1 : 1;
			float a1 = a + aps * (i / 2) * m;
			vec2 dir = direction(a1);

			new CProjectile(
				GameWorld(),
				Weapon, // Type
				-1, // Owner
				ProjStartPos, // Pos
				dir, // Dir
				LifeTime, // Span
				false, // Freeze
				(Weapon == WEAPON_GRENADE), // Explosive
				-1, // SoundImpact
				ProjStartPos
			);
		}

		// Make a sound :O
		if (Weapon == WEAPON_GUN)
			GameServer()->CreateSound(m_Pos, SOUND_GUN_FIRE);
		else if (Weapon == WEAPON_SHOTGUN)
			GameServer()->CreateSound(m_Pos, SOUND_SHOTGUN_FIRE);
		else
			GameServer()->CreateSound(m_Pos, SOUND_GRENADE_FIRE);
	}

	switch(m_Core.m_ActiveWeapon)
	{
	case WEAPON_HAMMER:
	{
		// reset objects Hit
		GameServer()->CreateSound(m_Pos, SOUND_HAMMER_FIRE);

		if(m_Core.m_HammerHitDisabled)
			break;

		CEntity *apEnts[MAX_CLIENTS];
		int Hits = 0;
		int Num = GameServer()->m_World.FindEntities(ProjStartPos, GetProximityRadius() * 0.5f, apEnts,
			MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

		for(int i = 0; i < Num; ++i)
		{
			auto *pTarget = static_cast<CCharacter *>(apEnts[i]);

			if(!pTarget->IsAlive())
				continue;

			// set his velocity to fast upward (for now)
			if(length(pTarget->m_Pos - ProjStartPos) > 0.0f)
				GameServer()->CreateHammerHit(pTarget->m_Pos - normalize(pTarget->m_Pos - ProjStartPos) * GetProximityRadius() * 0.5f);
			else
				GameServer()->CreateHammerHit(ProjStartPos);

			vec2 Dir;
			if(length(pTarget->m_Pos - m_Pos) > 0.0f)
				Dir = normalize(pTarget->m_Pos - m_Pos);
			else
				Dir = vec2(0.f, -1.f);

			float Strength = GameServer()->Tuning()->m_HammerStrength;

			vec2 Temp = pTarget->GetCore().m_Vel + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f;
			Temp = ClampVel(pTarget->m_MoveRestrictions, Temp);
			Temp -= pTarget->GetCore().m_Vel;
			pTarget->TakeDamage((vec2(0.f, -1.0f) + Temp) * Strength, 3 + m_Damage,
				-1, m_Core.m_ActiveWeapon);

			Hits++;
		}

//		Num = GameServer()->m_World.FindEntities(ProjStartPos, GetProximityRadius(), apEnts,
//			MAX_CLIENTS, CGameWorld::ENTTYPE_DUMMY);
//
//		for(int i = 0; i < Num; ++i)
//		{
//			auto *pTarget = static_cast<CDummy *>(apEnts[i]);
//
//			if(!pTarget->IsAlive())
//				continue;
//
//			// set his velocity to fast upward (for now)
//			if(length(pTarget->m_Pos - ProjStartPos) > 0.0f)
//				GameServer()->CreateHammerHit(pTarget->m_Pos - normalize(pTarget->m_Pos - ProjStartPos) * GetProximityRadius() * 0.5f);
//			else
//				GameServer()->CreateHammerHit(ProjStartPos);
//
//			vec2 Dir;
//			if(length(pTarget->m_Pos - m_Pos) > 0.0f)
//				Dir = normalize(pTarget->m_Pos - m_Pos);
//			else
//				Dir = vec2(0.f, -1.f);
//
//			float Strength = GameServer()->Tuning()->m_HammerStrength;
//
//			vec2 Temp = pTarget->Core()->m_Vel + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f;
//			//Temp = ClampVel(pTarget->m_MoveRestrictions, Temp);
//			Temp -= pTarget->Core()->m_Vel;
//			pTarget->TakeDamage((vec2(0.f, -1.0f) + Temp) * Strength, 3 + m_Damage,
//				-1, m_Core.m_ActiveWeapon);
//
//			Hits++;
//		}

		// if we Hit anything, we have to wait for to reload
		if(Hits)
		{
			float FireDelay = GameServer()->Tuning()->m_HammerHitFireDelay;
			m_ReloadTimer = FireDelay * Server()->TickSpeed() / 1000;
		}
	}
	break;

	case WEAPON_LASER:
	{
		float LaserReach = GameServer()->Tuning()->m_LaserReach;

		new CLaser(GameWorld(), m_Pos, Direction, LaserReach, -1, WEAPON_LASER);
		GameServer()->CreateSound(m_Pos, SOUND_LASER_FIRE);
	}
	break;
	}

	m_AttackTick = Server()->Tick();
}

void CDummy::Destroy()
{
	for(int i = 0; i < GameWorld()->m_Core.m_vDummies.size(); i++)
		if(GameWorld()->m_Core.m_vDummies[i] == &m_Core)
			GameWorld()->m_Core.m_vDummies.erase(GameWorld()->m_Core.m_vDummies.begin() + i);

	delete this;
}

void CDummy::EntTick()
{
	m_Core.m_Alive = m_Alive;

	if(Server()->Tick() > m_SpawnTick && !m_Alive)
		Spawn();

	// Don't calc phys if dummy is dead
	if(!m_Alive)
		return;

	// Reset input
	m_Input.m_Fire = 0;
	m_Input.m_Jump = 0;
	m_Input.m_Direction = 0;

	// Update lua
	luabridge::setGlobal(m_Lua.LuaState(), this, "ENT");
}

void CDummy::PostEntTick()
{
	// Physics
	m_Core.m_Input = m_Input;
	m_Core.m_ActiveWeapon = m_Input.m_WantedWeapon;
	m_Core.Tick(true);
	m_Core.Move();
	m_Core.Quantize();
	m_Pos = m_Core.m_Pos;

	FireWeapon();

	m_PrevInput = m_Input;
}

void CDummy::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	int SelfID = GameServer()->GetNextBotSnapID(SnappingClient);
	if(SelfID == -1)
		return;

	// Snap player
	CNetObj_ClientInfo *pClientInfo = Server()->SnapNewItem<CNetObj_ClientInfo>(SelfID);
	if(!pClientInfo)
		return;

	StrToInts(&pClientInfo->m_Name0, 4, m_aName);
	StrToInts(&pClientInfo->m_Clan0, 3, m_aClan);
	StrToInts(&pClientInfo->m_Skin0, 6, m_TeeInfo.m_aSkinName);
	pClientInfo->m_Country = 0;
	pClientInfo->m_UseCustomColor = m_TeeInfo.m_UseCustomColor;
	pClientInfo->m_ColorBody = m_TeeInfo.m_ColorBody;
	pClientInfo->m_ColorFeet = m_TeeInfo.m_ColorFeet;

	CNetObj_PlayerInfo *pPlayerInfo = Server()->SnapNewItem<CNetObj_PlayerInfo>(SelfID);
	if(!pPlayerInfo)
		return;

	pPlayerInfo->m_Latency = 0;
	pPlayerInfo->m_Score = 0;
	pPlayerInfo->m_Local = 0;
	pPlayerInfo->m_ClientID = SelfID;
	pPlayerInfo->m_Team = 10;

	// Don't snap character if dummy is dead
	if (!m_Alive)
		return;

	// Snap character
	CNetObj_Character *pCharacter = Server()->SnapNewItem<CNetObj_Character>(SelfID);
	if(!pCharacter)
		return;

	m_Core.Write(pCharacter);

	pCharacter->m_Tick = Server()->Tick();
	pCharacter->m_Emote = (m_EmoteStop > Server()->Tick()) ? m_EmoteType : m_DefaultEmote;
	pCharacter->m_HookedPlayer = -1;
	pCharacter->m_AttackTick = m_AttackTick;
	pCharacter->m_Direction = m_Input.m_Direction;
	pCharacter->m_Weapon = m_Input.m_WantedWeapon;
	pCharacter->m_AmmoCount = 0;
	pCharacter->m_Health = 0;
	pCharacter->m_Armor = 0;
	pCharacter->m_PlayerFlags = PLAYERFLAG_PLAYING;

	// Snap ddnet character (just for pets kek)
	CNetObj_DDNetCharacter *pDDNetCharacter = Server()->SnapNewItem<CNetObj_DDNetCharacter>(SelfID);
	if(!pDDNetCharacter)
		return;

	pDDNetCharacter->m_Flags = 0;
	pDDNetCharacter->m_FreezeEnd = 0;
	pDDNetCharacter->m_Jumps = m_Core.m_Jumps;
	pDDNetCharacter->m_TeleCheckpoint = -1;
	pDDNetCharacter->m_StrongWeakID = 0;

	// Display Information
	pDDNetCharacter->m_JumpedTotal = m_Core.m_JumpedTotal;
	pDDNetCharacter->m_NinjaActivationTick = m_Core.m_Ninja.m_ActivationTick;
	pDDNetCharacter->m_FreezeStart = m_Core.m_FreezeStart;
	pDDNetCharacter->m_TargetX = m_Core.m_Input.m_TargetX;
	pDDNetCharacter->m_TargetY = m_Core.m_Input.m_TargetY;
}
