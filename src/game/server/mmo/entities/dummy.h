#ifndef GAME_SERVER_MMO_ENTITIES_DUMMY_H
#define GAME_SERVER_MMO_ENTITIES_DUMMY_H

#include <game/gamecore.h>
#include <game/server/teeinfo.h>
#include <game/server/mmo/lua_entity.h>

class CDummy : public CLuaEntity
{
	CCharacterCore m_Core;
	CTeeInfo m_TeeInfo;

	vec2 m_SpawnPos;
	bool m_Alive;
	int m_SpawnTick;
	int m_ReloadTimer;
	int m_AttackTick;

	char m_aName[MAX_NAME_LENGTH];
	char m_aClan[MAX_CLAN_LENGTH];

	int m_DefaultEmote;
	int m_EmoteType;
	int m_EmoteStop;

	void RegisterAPI() override;

public:
	CDummy(CGameWorld *pWorld, vec2 Pos, const char *pLuaFile);

	CNetObj_PlayerInput m_Input;
	CNetObj_PlayerInput m_PrevInput;

	void Spawn();
	void Die(int Killer);
	void TakeDamage(vec2 Force, int Damage, int From, int Weapon);
	void FireWeapon();

	virtual void Destroy() override;
	virtual void EntTick() override;
	virtual void PostEntTick() override;
	virtual void Snap(int SnappingClient) override;

	// Setters
	void SetName(const char *pName) { str_copy(m_aName, pName); }
	void SetClan(const char *pClan) { str_copy(m_aClan, pClan); }
	void SetTeeInfo(CTeeInfo Info) { m_TeeInfo = Info; }

	// Getters
	bool IsAlive() const { return m_Alive; }
	CCharacterCore *Core() { return &m_Core; }

	// Stats
	int m_Level;
	int m_MaxHealth;
	int m_MaxArmor;
	int m_Damage;

	int m_Health;
	int m_Armor;
};

#endif // GAME_SERVER_MMO_ENTITIES_DUMMY_H
