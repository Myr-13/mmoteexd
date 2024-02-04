#include "lua_binding.h"

#include "lua_state.h"

#include <lua/lua.hpp>

#include <game/server/gamecontext.h>

int lua_add_cs_file(lua_State *L)
{
	if(lua_gettop(L) < 1)
		return luaL_argerror(L, 1, "expected 1 argument: string");
	if(!lua_isstring(L, -1))
		return luaL_argerror(L, 1, "expected string");

	const char *pFile = luaL_checkstring(L, -1);

	SLuaState::ms_pGameServer->m_vClientFiles.emplace_back(pFile);

	return 0;
}

int lua_include(lua_State *L)
{
	if(lua_gettop(L) < 1)
		return luaL_argerror(L, 1, "expected 1 argument");

	CLuaFile *pLF = SLuaState::ms_pGameServer->m_LuaManager.GetLuaFile(L);
	if(!pLF)
		return luaL_error(L, "this should be never reached...");

	const char *pFile = luaL_checkstring(L, 1);
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "lua/%s", pFile);
	pLF->OpenFile(aBuf);

	return 0;
}

int lua_hash(lua_State *L)
{
	if(lua_gettop(L) < 1)
		return luaL_argerror(L, 1, "expected 1 argument");

	uint32_t Res = str_quickhash(luaL_checkstring(L, 1));
	lua_pushinteger(L, Res);

	return 1;
}
