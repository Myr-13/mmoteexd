#include "lua_binding.h"

#include "lua_state.h"

#include <lua/lua.hpp>
#include <engine/shared/lua/lua_manager.h>
#include <base/system.h>
#include <filesystem>

int lua_include(lua_State *L)
{
	if(lua_gettop(L) < 1)
		return luaL_argerror(L, 1, "expected 1 argument");

	CLuaFile *pLF = SLuaState::ms_pLuaManager->GetLuaFile(L);
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

int lua_list_dir(lua_State *L)
{
	// Check arg
	if(lua_gettop(L) != 1)
		return luaL_argerror(L, 1, "expected a string, got nil");

	// Get dir name
	const char *pDirName = luaL_checklstring(L, 1, 0);
	if(!pDirName)
		return 0;

	// Check for directory
	if(!std::filesystem::is_directory(pDirName))
		return luaL_error(L, "'%s' is not directory", pDirName);

	// Outer table
	lua_newtable(L);
	int OuterIdx = lua_gettop(L);

	int InnerIdx = 1;
	for(const auto &entry : std::filesystem::directory_iterator(pDirName))
	{
		bool IsFile = entry.is_regular_file();
		std::string Name = entry.path().filename().u8string();

		lua_newtable(L);
		int InnerTable = lua_gettop(L);

		// Push type
		lua_pushstring(L, IsFile ? "file" : "dir");
		lua_rawseti(L, InnerTable, 1);

		// Push name
		lua_pushstring(L, Name.c_str());
		lua_rawseti(L, InnerTable, 2);

		// Push table to outer table
		lua_rawseti(L, OuterIdx, InnerIdx);
		InnerIdx++;
	}

	return 1;
}

struct SPart
{
	bool m_IsCmd;
	std::string m_Cmd;
	std::string m_Data;
};

int lua_parse_rich_text(lua_State *L)
{
	if(lua_gettop(L) < 1)
		return luaL_argerror(L, 1, "expected 1 argument");
	std::string Str = luaL_checkstring(L, 1);

	std::vector<SPart> Result;

	size_t Pos = 0;
	while(Pos < Str.size())
	{
		if(Str[Pos] == '<')
		{
			size_t EndPos = Str.find('>', Pos);

			if(EndPos != std::string::npos)
			{
				std::string Tag = Str.substr(Pos + 1, EndPos - Pos - 1);

				if(Tag.empty())
				{
					// Invalid tag
					Pos++;
					continue;
				}

				// Opening tag
				size_t ColonPos = Tag.find(':');
				std::string Cmd;
				std::string Data;

				if(ColonPos != std::string::npos)
				{
					Cmd = Tag.substr(0, ColonPos);
					Data = Tag.substr(ColonPos + 1);
				}
				else
					Cmd = Tag;

				Result.push_back({true, Cmd, Data});
				Pos = EndPos + 1;
			}
			else // Invalid tag
				Pos++;
		} else {
			// Text
			size_t EndPos = Str.find('<', Pos);

			if(EndPos == std::string::npos)
				EndPos = Str.size();

			Result.push_back({false, "", Str.substr(Pos, EndPos - Pos)});
			Pos = EndPos;
		}
	}

	// Result table
	lua_newtable(L);
	int OuterIdx = lua_gettop(L);

	int InnerIdx = 1;
	for(SPart &Part : Result)
	{
		lua_newtable(L);
		int InnerTable = lua_gettop(L);

		lua_pushboolean(L, Part.m_IsCmd);
		lua_rawseti(L, InnerTable, 1);

		lua_pushstring(L, Part.m_Cmd.c_str());
		lua_rawseti(L, InnerTable, 2);

		lua_pushstring(L, Part.m_Data.c_str());
		lua_rawseti(L, InnerTable, 3);

		// Push table to outer table
		lua_rawseti(L, OuterIdx, InnerIdx);
		InnerIdx++;
	}

	return 1;
}
