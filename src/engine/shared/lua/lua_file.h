#ifndef ENGINE_SHARED_LUA_LUA_FILE_H
#define ENGINE_SHARED_LUA_LUA_FILE_H

struct lua_State;

class CLuaFile
{
	friend class CLuaManager;

	lua_State *m_pState;

public:
	CLuaFile();

	void Create();
	void Close();

	bool OpenFile(const char *pFile);

	void PrintError();
	static int ErrorFunc(lua_State *L);

	void RegisterAPI(bool Server);

	lua_State *LuaState() { return m_pState; }
};

#endif // ENGINE_SHARED_LUA_LUA_FILE_H
