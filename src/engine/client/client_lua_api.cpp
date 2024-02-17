#include <engine/shared/lua/lua_binding.h>
#include <engine/shared/lua/lua_file.h>
#include <engine/shared/lua/lua_state.h>

#include <lua/lua.hpp>
#include <engine/external/luabridge/LuaBridge.h>
#include <engine/client/imgui/imgui.h>

#include <base/system.h>
#include <base/vmath.h>
#include <engine/console.h>
#include <game/client/gameclient.h>

void register_imgui(lua_State *L)
{
	luabridge::getGlobalNamespace(L)
		.beginNamespace("ImGui")
		.addFunction("Begin", [](const char *pName, int Flags) { ImGui::Begin(pName, 0x0, Flags); })
		.addFunction("End", &ImGui::End)
		.addFunction("_LabelText", [](const char *pLabel, const char *pText) { ImGui::LabelText(pLabel, "%s", pText); })
		.addFunction("_BulletText", [](const char *pText) { ImGui::BulletText("%s", pText); })
		.addFunction("_Button", [](const char *pText, vec2 Size) { return ImGui::Button(pText, ImVec2(Size.x, Size.y)); })
		.addFunction("SameLine", []() { ImGui::SameLine(); })
		.endNamespace();
}

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

		.beginClass<CUnpacker>("CUnpacker")
		.addFunction("GetInt", &CUnpacker::GetInt)
		.addFunction("GetString", [](CUnpacker *pSelf) { return pSelf->GetString(); })
		.endClass()

		.beginClass<CPacker>("CPacker")
		.addFunction("Reset", &CPacker::Reset)
		.addFunction("AddInt", &CPacker::AddInt)
		.addFunction("AddString", [](CPacker *pSelf, const char *pStr) { pSelf->AddString(pStr); })
		.endClass()

		.deriveClass<CMsgPacker, CPacker>("CMsgPacker")
		.addConstructor<void (*)(int, bool)>()
		.endClass()

		.beginClass<IGraphics::CTextureHandle>("CTextureHandle")
		.endClass()

		.beginClass<CImageInfo>("CImageInfo")
		.addConstructor<void (*)()>()

		.addProperty("Width", &CImageInfo::m_Width, false)
		.addProperty("Height", &CImageInfo::m_Height, false)
		.endClass()

		.beginClass<IGraphics>("IGraphics")
		.addProperty("ScreenAspect", &IGraphics::ScreenAspect)

		.addFunction("QuadsBegin", &IGraphics::QuadsBegin)
		.addFunction("QuadsEnd", &IGraphics::QuadsEnd)
		.addFunction("LinesBegin", &IGraphics::LinesBegin)
		.addFunction("LinesEnd", &IGraphics::LinesEnd)
		.addFunction("QuadsDraw", &IGraphics::QuadsDrawLua)
		.addFunction("LinesDraw", &IGraphics::LinesDrawLua)
		.addFunction("DrawRectExt", &IGraphics::DrawRectExt)

		.addFunction("SetColor", [](IGraphics *pSelf, float r, float g, float b, float a) { pSelf->SetColor(r, g, b, a); })
		.addFunction("TextureClear", &IGraphics::TextureClear)
		.addFunction("TextureSet", &IGraphics::TextureSet)
		.addFunction("MapScreen", &IGraphics::MapScreen)
		.addFunction("LoadTexture", [](IGraphics *pSelf, const char *pPath) { return pSelf->LoadTexture(pPath, IStorage::TYPE_ALL); })
		.addFunction("UnloadTexture", &IGraphics::UnloadTexture)

		.addFunction("LoadPNG", [](IGraphics *pSelf, CImageInfo *pImg, const char *pPath) { return pSelf->LoadPNG(pImg, pPath, IStorage::TYPE_ALL); })
		.addFunction("FreePNG", &IGraphics::FreePNG)
		.addFunction("LoadSpriteTextureImpl", &IGraphics::LoadSpriteTextureImpl)
		.endClass()

		.beginClass<ITextRender>("ITextRender")
		.addFunction("Text", &ITextRender::Text)
		.addFunction("TextWidth", [](ITextRender *pSelf, int Size, const char *pText) { return pSelf->TextWidth(Size, pText); })
		.addFunction("TextColor", [](ITextRender *pSelf, float r, float g, float b, float a) { pSelf->TextColor(r, g, b, a); })
		.endClass()

		.beginClass<IInput::CEvent>("CEvent")
		.addProperty("Key", &IInput::CEvent::m_Key, false)
		.addProperty("Flags", &IInput::CEvent::m_Flags, false)
		.endClass()

		.beginClass<IInput>("IInput")
		.addFunction("MouseModeAbsolute", &IInput::MouseModeAbsolute)
		.addFunction("MouseModeRelative", &IInput::MouseModeRelative)
		.endClass()

		.beginClass<IClient>("IClient")
		.addProperty("LocalTime", &IClient::LocalTime)

		.addFunction("SendMsg", [](IClient *pSelf, CMsgPacker *pMsg, int Flags) { pSelf->SendMsg(0, pMsg, Flags); })
		.endClass()

		.beginNamespace("Game")
		.addProperty("Client", &SLuaState::ms_pGameClient->m_pClient, false)
		.addProperty("GameClient", &SLuaState::ms_pGameClient, false)
		.addProperty("Graphics", &SLuaState::ms_pGameClient->m_pGraphics, false)
		.addProperty("TextRender", &SLuaState::ms_pGameClient->m_pTextRender, false)
		.addProperty("Input", &SLuaState::ms_pGameClient->m_pInput, false)
		.endNamespace();

	lua_register(L, "parse_rich_text", lua_parse_rich_text);
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
	lua_register(L, "list_dir", lua_list_dir);
	luaL_dostring(L, "function print(...) for _, v in pairs({...}) do sys.dbg_msg(\"lua_print\", tostring(v)) end end");
}

void CLuaFile::RegisterAPI(bool Server)
{
	register_shared_api(m_pState);
	register_imgui(m_pState);
	register_client_api(m_pState);
}
