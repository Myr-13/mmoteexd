#ifndef ENGINE_SHARED_LUA_LUA_STATE_H
#define ENGINE_SHARED_LUA_LUA_STATE_H

struct SLuaState
{
	static class CGameClient *ms_pGameClient;
	static class CGameContext *ms_pGameServer;
	static class IConsole *ms_pConsole;
	static class CLuaManager *ms_pLuaManager;
	static class IServer *ms_pServer;
	static class CCollision *ms_pCollision;
	static class CGameWorld *ms_pWorld;
};

extern SLuaState g_LuaState;

#endif // ENGINE_SHARED_LUA_LUA_STATE_H
