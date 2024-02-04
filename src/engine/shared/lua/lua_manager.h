#ifndef ENGINE_SHARED_LUA_LUA_MANAGER_H
#define ENGINE_SHARED_LUA_LUA_MANAGER_H

#include "lua_file.h"

#include <vector>

#define LUA_FIRE_EVENT(Name, ...) \
	for(auto &LF : SLuaState::ms_pLuaManager->GetLuaFiles()) \
	{ \
		luabridge::LuaRef Func = luabridge::getGlobal(LF.LuaState(), Name); \
                if(Func.isValid() && Func.isFunction()) \
                { \
                	luabridge::LuaResult Res = Func(__VA_ARGS__); \
                        if(Res.hasFailed()) \
				dbg_msg("lua_error", "error: %s", Res.errorMessage().c_str()); \
		} \
	}

class CLuaManager
{
	std::vector<CLuaFile> m_vLuaFiles;

public:
	~CLuaManager();

	bool Open(const char *pFolder, bool IsServer);
	void CloseAll();
	void ReloadAll();

	CLuaFile *GetLuaFile(lua_State *L);
	std::vector<CLuaFile> &GetLuaFiles() { return m_vLuaFiles; }
};

#endif // ENGINE_SHARED_LUA_LUA_MANAGER_H
