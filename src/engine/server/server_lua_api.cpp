#include <engine/shared/lua/lua_binding.h>
#include <engine/shared/lua/lua_file.h>
#include <engine/shared/lua/lua_state.h>

#include <base/system.h>

#include <lua/lua.hpp>
#include <engine/external/luabridge/LuaBridge.h>
#include <engine/shared/sqlite/sqlite.h>
#include <engine/shared/console.h>
#include <engine/map.h>

#include <game/server/gamecontext.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/entity.h>
#include <game/mapitems.h>

void register_network_api(lua_State *L)
{
	luabridge::getGlobalNamespace(L)
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

		.beginClass<CNetObj_ClientInfo>("CNetObj_ClientInfo")
		.addProperty("Name0", &CNetObj_ClientInfo::m_Name0)
		.addProperty("Name1", &CNetObj_ClientInfo::m_Name1)
		.addProperty("Name2", &CNetObj_ClientInfo::m_Name2)
		.addProperty("Name3", &CNetObj_ClientInfo::m_Name3)
		.addProperty("Clan0", &CNetObj_ClientInfo::m_Clan0)
		.addProperty("Clan1", &CNetObj_ClientInfo::m_Clan1)
		.addProperty("Clan2", &CNetObj_ClientInfo::m_Clan2)
		.addProperty("Country", &CNetObj_ClientInfo::m_Country)
		.addProperty("Skin0", &CNetObj_ClientInfo::m_Skin0)
		.addProperty("Skin1", &CNetObj_ClientInfo::m_Skin1)
		.addProperty("Skin2", &CNetObj_ClientInfo::m_Skin2)
		.addProperty("Skin3", &CNetObj_ClientInfo::m_Skin3)
		.addProperty("Skin4", &CNetObj_ClientInfo::m_Skin4)
		.addProperty("Skin5", &CNetObj_ClientInfo::m_Skin5)
		.addProperty("UseCustomColor", &CNetObj_ClientInfo::m_UseCustomColor)
		.addProperty("ColorBody", &CNetObj_ClientInfo::m_ColorBody)
		.addProperty("ColorFeet", &CNetObj_ClientInfo::m_ColorFeet)

		.addFunction("SetName", [](CNetObj_ClientInfo *pSelf, int Num, const char *pStr) { StrToInts(&pSelf->m_Name0, Num, pStr); })
		.addFunction("SetClan", [](CNetObj_ClientInfo *pSelf, int Num, const char *pStr) { StrToInts(&pSelf->m_Clan0, Num, pStr); })
		.addFunction("SetSkin", [](CNetObj_ClientInfo *pSelf, int Num, const char *pStr) { StrToInts(&pSelf->m_Skin0, Num, pStr); })
		.endClass()

		.beginClass<CNetObj_PlayerInfo>("CNetObj_PlayerInfo")
		.addProperty("Local", &CNetObj_PlayerInfo::m_Local)
		.addProperty("ClientID", &CNetObj_PlayerInfo::m_ClientID)
		.addProperty("Team", &CNetObj_PlayerInfo::m_Team)
		.addProperty("Score", &CNetObj_PlayerInfo::m_Score)
		.addProperty("Latency", &CNetObj_PlayerInfo::m_Latency)
		.endClass()

		.beginClass<CNetObj_CharacterCore>("CNetObj_CharacterCore")
		.addProperty("Tick", &CNetObj_CharacterCore::m_Tick)
		.addProperty("X", &CNetObj_CharacterCore::m_X)
		.addProperty("Y", &CNetObj_CharacterCore::m_Y)
		.addProperty("VelX", &CNetObj_CharacterCore::m_VelX)
		.addProperty("VelY", &CNetObj_CharacterCore::m_VelY)
		.addProperty("Angle", &CNetObj_CharacterCore::m_Angle)
		.addProperty("Direction", &CNetObj_CharacterCore::m_Direction)
		.addProperty("Jumped", &CNetObj_CharacterCore::m_Jumped)
		.addProperty("HookedPlayer", &CNetObj_CharacterCore::m_HookedPlayer)
		.addProperty("HookState", &CNetObj_CharacterCore::m_HookState)
		.addProperty("HookTick", &CNetObj_CharacterCore::m_HookTick)
		.addProperty("HookX", &CNetObj_CharacterCore::m_HookX)
		.addProperty("HookY", &CNetObj_CharacterCore::m_HookY)
		.addProperty("HookDx", &CNetObj_CharacterCore::m_HookDx)
		.addProperty("HookDy", &CNetObj_CharacterCore::m_HookDy)
		.endClass()

		.deriveClass<CNetObj_Character, CNetObj_CharacterCore>("CNetObj_Character")
		.addProperty("PlayerFlags", &CNetObj_Character::m_PlayerFlags)
		.addProperty("Health", &CNetObj_Character::m_Health)
		.addProperty("Armor", &CNetObj_Character::m_Armor)
		.addProperty("AmmoCount", &CNetObj_Character::m_AmmoCount)
		.addProperty("Weapon", &CNetObj_Character::m_Weapon)
		.addProperty("Emote", &CNetObj_Character::m_Emote)
		.addProperty("AttackTick", &CNetObj_Character::m_AttackTick)
		.endClass()

		.beginClass<CNetObj_PlayerInput>("CNetObj_PlayerInput")
		.addConstructor<void (*)()>()

		.addProperty("Direction", &CNetObj_PlayerInput::m_Direction)
		.addProperty("TargetX", &CNetObj_PlayerInput::m_TargetX)
		.addProperty("TargetY", &CNetObj_PlayerInput::m_TargetY)
		.addProperty("Jump", &CNetObj_PlayerInput::m_Jump)
		.addProperty("Fire", &CNetObj_PlayerInput::m_Fire)
		.addProperty("Hook", &CNetObj_PlayerInput::m_Hook)
		.addProperty("PlayerFlags", &CNetObj_PlayerInput::m_PlayerFlags)
		.addProperty("WantedWeapon", &CNetObj_PlayerInput::m_WantedWeapon)
		.addProperty("NextWeapon", &CNetObj_PlayerInput::m_NextWeapon)
		.addProperty("PrevWeapon", &CNetObj_PlayerInput::m_PrevWeapon)
		.endClass()
	;
}

void register_server_api(lua_State *L)
{
	lua_register(L, "AddCSFile", lua_add_cs_file);

	luabridge::getGlobalNamespace(L)
		.beginClass<CSQLite>("CSQLite")
		.addFunction("Execute", &CSQLite::Execute)
		.endClass()

		.beginClass<CSQLite::SResult>("SResult")
		.addProperty("ErrorMsg", &CSQLite::SResult::m_Error)

		.addFunction("Prepare", &CSQLite::SResult::Prepare)
		.addFunction("Step", &CSQLite::SResult::Step)
		.addFunction("Execute", &CSQLite::SResult::Execute)

		.addFunction("GetInt", &CSQLite::SResult::GetInt)
		.addFunction("GetFloat", &CSQLite::SResult::GetFloat)
		.endClass()

		.beginClass<CUnpacker>("CUnpacker")
		.addFunction("GetInt", &CUnpacker::GetInt)
		.addFunction("GetString", &CUnpacker::GetString)
		.endClass()

		.beginClass<CPacker>("CPacker")
		.addFunction("Reset", &CPacker::Reset)
		.addFunction("AddInt", &CPacker::AddInt)
		.addFunction("AddString", [](CPacker *pSelf, const char *pStr) { pSelf->AddString(pStr); })
		.endClass()

		.deriveClass<CMsgPacker, CPacker>("CMsgPacker")
		.addConstructor<void (*)(int, bool)>()
		.endClass()

		.beginClass<CEntity>("CEntity")
		.addProperty("Pos", &CEntity::m_Pos)
		.endClass()

		.deriveClass<CCharacter, CEntity>("CCharacter")
		.addProperty("AttackTick", &CCharacter::m_AttackTick)
		.addProperty("ReloadTimer", &CCharacter::m_ReloadTimer)
		.addProperty("Core", &CCharacter::m_Core)
		.addProperty("Health", &CCharacter::m_Health)
		.addProperty("MaxHealth", &CCharacter::GetMaxHealth)

		.addProperty("Input", &CCharacter::m_Input)
		.addProperty("LatestInput", &CCharacter::m_LatestInput)
		.addProperty("LatestPrevInput", &CCharacter::m_LatestPrevInput)

		.addFunction("IncreaseHealth", &CCharacter::IncreaseHealth)
		.addFunction("TakeDamage", [](CCharacter *pSelf, vec2 Force, int Dmg, int From) { pSelf->TakeDamage(Force, Dmg, From, WEAPON_WORLD); })
		.endClass()

		.beginClass<CCharacterCore>("CCharacterCore")
		.addConstructor<void (*)()>()

		.addProperty("Pos", &CCharacterCore::m_Pos)
		.addProperty("Vel", &CCharacterCore::m_Vel)
		.addProperty("Alive", &CCharacterCore::m_Alive)
		.addProperty("Input", &CCharacterCore::m_Input)
		.addProperty("ActiveWeapon", &CCharacterCore::m_ActiveWeapon)

		.addFunction("Write", &CCharacterCore::Write)
		.addFunction("Init", &CCharacterCore::Init)
		.addFunction("Tick", &CCharacterCore::Tick)
		.addFunction("Move", &CCharacterCore::Move)
		.addFunction("Quantize", &CCharacterCore::Quantize)
		.endClass()

		.beginClass<CPlayer>("CPlayer")
		.addProperty("Character", [](CPlayer *pSelf) { return pSelf->GetCharacter(); })
		.addProperty("Score", &CPlayer::m_Score)
		.addProperty("MaxHealth", &CPlayer::m_MaxHealth)
		.addProperty("WorldID", [](CPlayer *pSelf) { return pSelf->Server()->GetClientWorld(pSelf->GetCID()); })

		.addFunction("SetTeam", &CPlayer::SetTeam)
		.endClass()

		.beginClass<CMultiWorldManager>("CMultiWorldManager")
		.addFunction("LoadWorld", &CMultiWorldManager::LoadWorld)
		.addFunction("ChangeWorld", &CMultiWorldManager::ChangeWorld)
		.endClass()

		.beginClass<IGameServer>("IGameServer")
		.endClass()

		.deriveClass<CGameContext, IGameServer>("CGameContext")
		.addProperty("MultiWorldManager", &CGameContext::m_MultiWorldManager)

		.addFunction("SendChatTarget", &CGameContext::SendChatTarget)
		.addFunction("GetNextBotSnapID", &CGameContext::GetNextBotSnapID)
		.addFunction("SendBroadcast", &CGameContext::SendBroadcast)

		.addFunction("CreateSound", [](CGameContext *pSelf, vec2 Pos, int Sound) { pSelf->CreateSound(Pos, Sound); })
		.addFunction("CreateDeath", [](CGameContext *pSelf, vec2 Pos, int ClientID) { pSelf->CreateDeath(Pos, ClientID); })
		.addFunction("CreatePlayerSpawn", [](CGameContext *pSelf, vec2 Pos) { pSelf->CreatePlayerSpawn(Pos); })
		.addFunction("CreateHammerHit", [](CGameContext *pSelf, vec2 Pos) { pSelf->CreateHammerHit(Pos); })
		.addFunction("CreateDamageInd", [](CGameContext *pSelf, vec2 Pos, float AngleMod, int Amount) { pSelf->CreateDamageInd(Pos, AngleMod, Amount); })

		.addFunction("NetworkClipped", [](CGameContext *pSelf, int ClientID, vec2 CheckPos) {
			float dx = pSelf->m_apPlayers[ClientID]->m_ViewPos.x - CheckPos.x;
			if(absolute(dx) > pSelf->m_apPlayers[ClientID]->m_ShowDistance.x)
				return true;

			float dy = pSelf->m_apPlayers[ClientID]->m_ViewPos.y - CheckPos.y;
			return absolute(dy) > pSelf->m_apPlayers[ClientID]->m_ShowDistance.y;
		})
		.endClass()

		.beginClass<IServer>("IServer")
		.addProperty("Tick", &IServer::Tick)

		.addFunction("ClientName", &IServer::ClientName)
		.addFunction("SendMsg", &IServer::SendMsg)

		.addFunction("SnapNewID", &IServer::SnapNewID)
		.addFunction("SnapFreeID", &IServer::SnapFreeID)
		.addFunction("SnapNewItemLaser", [](IServer *pSelf, int ID) { return pSelf->SnapNewItem<CNetObj_DDNetLaser>(ID); })
		.addFunction("SnapNewItemPickup", [](IServer *pSelf, int ID) { return pSelf->SnapNewItem<CNetObj_Pickup>(ID); })
		.addFunction("SnapNewItemProjectile", [](IServer *pSelf, int ID) { return pSelf->SnapNewItem<CNetObj_Projectile>(ID); })
		.addFunction("SnapNewItemClientInfo", [](IServer *pSelf, int ID) { return pSelf->SnapNewItem<CNetObj_ClientInfo>(ID); })
		.addFunction("SnapNewItemPlayerInfo", [](IServer *pSelf, int ID) { return pSelf->SnapNewItem<CNetObj_PlayerInfo>(ID); })
		.addFunction("SnapNewItemCharacter", [](IServer *pSelf, int ID) { return pSelf->SnapNewItem<CNetObj_Character>(ID); })
		.endClass()

		.beginClass<CSwitchTile>("CSwitchTile")
		.addProperty("Number", &CSwitchTile::m_Number)
		.addProperty("Type", &CSwitchTile::m_Type)
		.addProperty("Flags", &CSwitchTile::m_Flags)
		.addProperty("Delay", &CSwitchTile::m_Delay)
		.endClass()

		.beginClass<CMapItemLayerTilemap>("CMapItemLayerTilemap")
		.endClass()

		.beginClass<CLayers>("CLayers")
		.addProperty("Switch", &CLayers::SwitchLayer)

		.addFunction("GetTile", [](CLayers *pSelf, CMapItemLayerTilemap *pLayer, int Index) { return ((CSwitchTile *)pSelf->Map()->GetData(pLayer->m_Switch))[Index]; })
		.endClass()

		.beginClass<CCollision>("CCollision")
		.addProperty("Layers", [](CCollision *pSelf) { return pSelf->Layers(); })
		.addProperty("Width", &CCollision::GetWidth)
		.addProperty("Height", &CCollision::GetHeight)

		.addFunction("GetTile", &CCollision::GetTile)
		.addFunction("GetFTile", &CCollision::GetFTile)

		.addFunction("MoveBox", [](CCollision *pSelf, vec2 *pPos, vec2 *pVel, vec2 Size, vec2 Elasticity) { pSelf->MoveBox(pPos, pVel, Size, Elasticity); })
		.addFunction("IntersectLine", &CCollision::IntersectLine)
		.endClass()

		.beginClass<CWorldCore>("CWorldCore")
		.addFunction("AddDummy", &CWorldCore::AddDummy)
		.endClass()

		.beginClass<CGameWorld>("CGameWorld")
		.addProperty("Core", &CGameWorld::m_Core)
		.endClass()

		.beginClass<CInputCount>("CInputCount")
		.addProperty("Presses", &CInputCount::m_Presses)
		.addProperty("Releases", &CInputCount::m_Releases)
		.endClass()

		.beginNamespace("Engine")
		.addProperty("Database", &SLuaState::ms_pGameServer->m_DB, false)
		.endNamespace()

		.beginNamespace("Game")
		.addFunction("Players", CGameContext::LuaGetPlayer)
		.addFunction("Collision", [](int WorldID) { return SLuaState::ms_pGameServer->Collision(WorldID); })
		.addFunction("World", [](int WorldID) { return SLuaState::ms_pGameServer->GameWorld(WorldID); })

		.addProperty("GameServer", &SLuaState::ms_pGameServer, false)
		.addProperty("Server", &SLuaState::ms_pServer, false)
		.endNamespace()

		.addFunction("distance", [](vec2 Pos1, vec2 Pos2) { return distance(Pos1, Pos2); })
		.addFunction("normalize", [](vec2 Vec) { return normalize(Vec); })
		.addFunction("CountInput", CountInput);
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

		.deriveClass<CConsole::CResult, IConsole::IResult>("CResult")
		.endClass()

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
	register_network_api(m_pState);
	register_server_api(m_pState);
	register_shared_api(m_pState);
}
