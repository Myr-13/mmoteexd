#include <engine/shared/lua/lua_binding.h>
#include <engine/shared/lua/lua_file.h>
#include <engine/shared/lua/lua_state.h>

#include <lua/lua.hpp>
#include <engine/external/luabridge/LuaBridge.h>

#include <base/system.h>
#include <base/vmath.h>
#include <engine/console.h>
#include <game/client/gameclient.h>

void register_client_api(lua_State *L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<IGraphics::CQuadItem>("QuadItem")
		.addConstructor <void (*) (float, float, float, float)> ()
		.addProperty("x", &IGraphics::CQuadItem::m_X)
		.addProperty("y", &IGraphics::CQuadItem::m_Y)
		.addProperty("w", &IGraphics::CQuadItem::m_Width)
		.addProperty("h", &IGraphics::CQuadItem::m_Height)
		.endClass()

		.beginClass<IGraphics::CLineItem>("LineItem")
		.addConstructor <void (*) (float, float, float, float)> ()
		.addProperty("x1", &IGraphics::CLineItem::m_X0)
		.addProperty("y1", &IGraphics::CLineItem::m_Y0)
		.addProperty("x2", &IGraphics::CLineItem::m_X1)
		.addProperty("y2", &IGraphics::CLineItem::m_Y1)
		.endClass()

		.beginClass<IGraphics>("IGraphics")
		.addFunction("QuadsBegin", &IGraphics::QuadsBegin)
		.addFunction("QuadsEnd", &IGraphics::QuadsEnd)
		.addFunction("LinesBegin", &IGraphics::LinesBegin)
		.addFunction("LinesEnd", &IGraphics::LinesEnd)
		.addFunction("QuadsDraw", &IGraphics::QuadsDrawLua)
		.addFunction("LinesDraw", &IGraphics::LinesDrawLua)
		.endClass()

		.beginNamespace("Game")
		.addProperty("GameClient", &SLuaState::ms_pGameClient, false)
		.addProperty("Graphics", &SLuaState::ms_pGameClient->m_pGraphics, false)
		.endNamespace();
}

void register_shared_api(lua_State *L)
{
	luabridge::getGlobalNamespace(L)
		.beginClass<vector2_base<float>>("vec2")
		.addConstructor<void (*)(float, float)>()
		.addFunction("__add", &vector2_base<float>::lua_operator_add)
		.addFunction("__sub", &vector2_base<float>::lua_operator_sub)
		.addFunction("__mul", &vector2_base<float>::lua_operator_mul)
		.addFunction("__div", &vector2_base<float>::lua_operator_div)
		.addFunction("__eq", &vector2_base<float>::operator==)
		.addFunction("__tostring", &vector2_base<float>::tostring)
		.addProperty("x", &vector2_base<float>::x)
		.addProperty("y", &vector2_base<float>::y)
		.endClass()

		.beginClass<IConsole::IResult>("IResult")
		.addProperty("NumArguments", &IConsole::IResult::NumArguments)
		.addProperty("ClientID", &IConsole::IResult::m_ClientID, false)

		.addFunction("GetInteger", &IConsole::IResult::GetInteger)
		.addFunction("GetFloat", &IConsole::IResult::GetFloat)
		.addFunction("GetString", &IConsole::IResult::GetString)
		.addFunction("GetVictim", &IConsole::IResult::GetVictim)
		.endClass()

		//.deriveClass<CConsole::CResult, IConsole::IResult>("CResult")
		//.endClass()

		.beginClass<IConsole>("IConsole")
		.addFunction("Register", &IConsole::LuaRegister)
		.endClass()

		.beginNamespace("Engine")
		.addProperty("Console", &SLuaState::ms_pConsole, false)
		.endNamespace()

		.beginNamespace("sys")
		.addFunction("dbg_msg", [&](const char *pSys, const char *pText) { dbg_msg(pSys, pText); })
		.endNamespace();

	lua_register(L, "include", lua_include);
	lua_register(L, "hash", lua_hash);
	luaL_dostring(L, "function print(...) for _, v in pairs({...}) do sys.dbg_msg(\"lua_print\", tostring(v)) end end");
}

void CLuaFile::RegisterAPI(bool Server)
{
	register_shared_api(m_pState);
	register_client_api(m_pState);
}
