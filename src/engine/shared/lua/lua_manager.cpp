#include "lua_manager.h"

#include <base/system.h>

CLuaManager::~CLuaManager()
{
	CloseAll();
}

void CLuaManager::CloseAll()
{
	for(auto &LuaFile : m_vLuaFiles)
		LuaFile.Close();
	m_vLuaFiles.clear();
}

bool CLuaManager::Open(const char *pFolder, bool IsServer)
{
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "%s/%s", pFolder, IsServer ? "sv_init.lua" : "cl_init.lua");

	m_vLuaFiles.emplace_back();
	CLuaFile &File = m_vLuaFiles.back();

	File.Create();
	File.RegisterAPI(IsServer);

	if(!File.OpenFile(aBuf))
		return false;
	return true;
}

CLuaFile *CLuaManager::GetLuaFile(lua_State *L)
{
	for(CLuaFile &LuaFile : m_vLuaFiles)
		if(LuaFile.m_pState == L)
			return &LuaFile;
	return 0x0;
}

void CLuaManager::ReloadAll()
{
	CloseAll();
	Open("lua", true);
}
