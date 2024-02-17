#ifndef ENGINE_SHARED_LUA_LUA_BINDING_H
#define ENGINE_SHARED_LUA_LUA_BINDING_H

#include <lua/lua.hpp>
#include <engine/external/luabridge/LuaBridge.h>

#include <base/vmath.h>

int lua_include(lua_State *L);
int lua_hash(lua_State *L);
int lua_list_dir(lua_State *L);
int lua_parse_rich_text(lua_State *L);

#endif // ENGINE_SHARED_LUA_LUA_BINDING_H
