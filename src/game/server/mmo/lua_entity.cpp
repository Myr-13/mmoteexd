#include "lua_entity.h"

#include <lua/lua.hpp>
#include <engine/external/luabridge/LuaBridge.h>
#include <engine/shared/lua/lua_state.h>

#include <engine/server.h>
#include <game/collision.h>
#include <game/server/gamecontext.h>

CLuaEntity::CLuaEntity(CGameWorld *pWorld, vec2 Pos, const char *pLuaFile, const char *pLuaEntType, int EntType) :
	CEntity(pWorld, EntType, Pos)
{
	m_LuaEntType = pLuaEntType;
	m_Self = pLuaFile;
	m_InitCalled = false;

	GameWorld()->InsertEntity(this);
	GameWorld()->InsertLuaEntity(this);

	if(pLuaFile)
		LoadLua(pLuaFile);
}

void CLuaEntity::LoadLua(const char *pFile)
{
	m_Lua.Create();

	RegisterBaseAPI();
	RegisterAPI();

	if(!m_Lua.OpenFile(pFile))
		dbg_msg("lua_ent", "%s: failed to load '%s'", m_LuaEntType.c_str(), pFile);
}

void CLuaEntity::RegisterBaseAPI()
{
	luabridge::getGlobalNamespace(m_Lua.LuaState())
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

		.beginClass<CNetObj_DDNetLaser>("CNetObj_DDNetLaser")
			.addProperty("ToX", &CNetObj_DDNetLaser::m_ToX)
			.addProperty("ToY", &CNetObj_DDNetLaser::m_ToY)
			.addProperty("FromX", &CNetObj_DDNetLaser::m_FromX)
			.addProperty("FromY", &CNetObj_DDNetLaser::m_FromY)
			.addProperty("StartTick", &CNetObj_DDNetLaser::m_StartTick)
			.addProperty("Owner", &CNetObj_DDNetLaser::m_Owner)
			.addProperty("Type", &CNetObj_DDNetLaser::m_Type)
			.addProperty("SwitchNumber", &CNetObj_DDNetLaser::m_SwitchNumber)
			.addProperty("Subtype", &CNetObj_DDNetLaser::m_Subtype)
			.addProperty("Flags", &CNetObj_DDNetLaser::m_Flags)
		.endClass()

		.beginClass<CNetObj_Pickup>("CNetObj_Pickup")
		        .addProperty("x", &CNetObj_Pickup::m_X)
		        .addProperty("y", &CNetObj_Pickup::m_Y)
		        .addProperty("Type", &CNetObj_Pickup::m_Type)
		        .addProperty("Subtype", &CNetObj_Pickup::m_Subtype)
		.endClass()

		.beginClass<CNetObj_Projectile>("CNetObj_Projectile")
		        .addProperty("x", &CNetObj_Projectile::m_X)
		        .addProperty("y", &CNetObj_Projectile::m_Y)
		        .addProperty("VelX", &CNetObj_Projectile::m_VelX)
		        .addProperty("VelY", &CNetObj_Projectile::m_VelY)
			.addProperty("Type", &CNetObj_Projectile::m_Type)
			.addProperty("StartTick", &CNetObj_Projectile::m_StartTick)
		.endClass()

		.beginClass<CEntity>("CEntity")
			.addFunction("NetworkClipped", [](CEntity *pSelf, int ClientID) { return pSelf->NetworkClipped(ClientID); })

			.addProperty("Pos", &CEntity::m_Pos)
			.addProperty("ID", &CEntity::GetID)
		.endClass()

		.deriveClass<CLuaEntity, CEntity>("CLuaEntity")
		.endClass()

		.beginClass<IServer>("IServer")
			.addProperty("Tick", &IServer::Tick)

			.addFunction("SnapNewID", &IServer::SnapNewID)
			.addFunction("SnapFreeID", &IServer::SnapFreeID)
			.addFunction("SnapNewItemLaser", [](IServer *pSelf, int ID) { return pSelf->SnapNewItem<CNetObj_DDNetLaser>(ID); })
			.addFunction("SnapNewItemPickup", [](IServer *pSelf, int ID) { return pSelf->SnapNewItem<CNetObj_Pickup>(ID); })
			.addFunction("SnapNewItemProjectile", [](IServer *pSelf, int ID) { return pSelf->SnapNewItem<CNetObj_Projectile>(ID); })
		.endClass()

		.beginClass<CCollision>("CCollision")
		        .addFunction("GetTile", &CCollision::GetTile)
		.endClass()

		.beginNamespace("sys")
			.addFunction("dbg_msg", [&](const char *pSys, const char *pText) { dbg_msg(pSys, pText); })
		.endNamespace()

		.beginNamespace("Game")
			.addProperty("Server", &SLuaState::ms_pServer)
			.addProperty("Collision", &SLuaState::ms_pCollision)
		.endNamespace();

	luaL_dostring(m_Lua.LuaState(), "function print(...) for _, v in pairs({...}) do sys.dbg_msg(\"lua_print\", tostring(v)) end end");
}

void CLuaEntity::Tick()
{
	EntTick();

	if(m_Self)
		luabridge::setGlobal(m_Lua.LuaState(), this, "ENT");

	if(!m_InitCalled)
	{
		m_InitCalled = true;

		luabridge::LuaRef Func = luabridge::getGlobal(m_Lua.LuaState(), "OnInit");

		if(Func.isValid() && Func.isFunction())
		{
			luabridge::LuaResult Res = Func();
			if(Res.hasFailed())
				dbg_msg("lua_error", "%s", Res.errorMessage().c_str());
		}
	}

	// Call lua
	{
		luabridge::LuaRef Func = luabridge::getGlobal(m_Lua.LuaState(), "OnTick");

		if(Func.isValid() && Func.isFunction())
		{
			luabridge::LuaResult Res = Func();
			if(Res.hasFailed())
				dbg_msg("lua_error", "%s", Res.errorMessage().c_str());
		}
	}

	PostEntTick();
}

void CLuaEntity::Snap(int ClientID)
{
	luabridge::LuaRef Func = luabridge::getGlobal(m_Lua.LuaState(), "OnSnap");

	if(Func.isValid() && Func.isFunction())
	{
		luabridge::LuaResult Res = Func(ClientID);
		if(Res.hasFailed())
			dbg_msg("lua_error", "%s", Res.errorMessage().c_str());
	}
}
