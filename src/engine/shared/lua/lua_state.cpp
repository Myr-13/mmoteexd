#include "lua_state.h"

SLuaState g_LuaState;

class CGameClient *SLuaState::ms_pGameClient = 0x0;
class CGameContext *SLuaState::ms_pGameServer = 0x0;
class IConsole *SLuaState::ms_pConsole = 0x0;
class CLuaManager *SLuaState::ms_pLuaManager = 0x0;
class IServer *SLuaState::ms_pServer = 0x0;
class CCollision *SLuaState::ms_pCollision = 0x0;
class CGameWorld *SLuaState::ms_pWorld = 0x0;
