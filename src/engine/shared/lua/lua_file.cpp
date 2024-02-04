#include "lua_file.h"

#include <base/system.h>

#include <lua/lua.hpp>
#include <engine/external/luabridge/LuaBridge.h>

#include "lua_binding.h"

CLuaFile::CLuaFile()
{
	m_pState = 0x0;
}

void CLuaFile::Create()
{
	if(m_pState)
		Close();

	m_pState = luaL_newstate();
	if(!m_pState)
	{
		dbg_msg("lua", "failed to allocate new lua_State, out of memory?");
		return;
	}

	luaL_openlibs(m_pState);
	lua_atpanic(m_pState, ErrorFunc);
	luabridge::registerMainThread(m_pState);
}

void CLuaFile::Close()
{
	if(!m_pState)
		return;

	lua_close(m_pState);
	m_pState = 0x0;
}

bool CLuaFile::OpenFile(const char *pFile)
{
	if(luaL_dofile(m_pState, pFile) != LUA_OK)
	{
		PrintError();
		return false;
	}

	return true;
}

void CLuaFile::PrintError()
{
	CLuaFile::ErrorFunc(m_pState);
}

int CLuaFile::ErrorFunc(lua_State *L)
{
	if(lua_isstring(L, -1))
	{
		dbg_msg("lua_error", "%s", lua_tostring(L, -1));
		lua_pop(L, 1);
	}

	return 0;
}
