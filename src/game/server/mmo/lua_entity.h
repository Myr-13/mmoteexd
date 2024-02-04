#ifndef GAME_SERVER_MMO_LUA_ENTITY_H
#define GAME_SERVER_MMO_LUA_ENTITY_H

#include <engine/shared/lua/lua_file.h>

#include <game/server/entity.h>

class CLuaEntity : public CEntity
{
	friend class CGameWorld;

	std::string m_LuaEntType;
	bool m_Self;
	bool m_InitCalled;

	void RegisterBaseAPI();

protected:
	CLuaFile m_Lua;

	virtual void RegisterAPI() {};
	virtual void EntTick() {};
	virtual void PostEntTick() {};

	void LoadLua(const char *pFile);

public:
	CLuaEntity(CGameWorld *pWorld, vec2 Pos, const char *pLuaFile, const char *pLuaEntType, int EntType = CGameWorld::ENTTYPE_LUA_ENTITY);

	virtual void Tick() override;
	virtual void Snap(int ClientID) override;
};

#endif // GAME_SERVER_MMO_LUA_ENTITY_H
