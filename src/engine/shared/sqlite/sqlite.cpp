#include "sqlite.h"

#include <base/system.h>

#include <sqlite3.h>
#include <lua/lua.hpp>
#include <engine/external/luabridge/LuaBridge.h>

void th_func(void *pUser)
{
	CSQLite *pSelf = (CSQLite *)pUser;

	while(pSelf->m_IsRunning)
		pSelf->Loop();
}

bool CSQLite::SResult::CheckError(int Result)
{
	if(Result != SQLITE_OK)
		m_Error = sqlite3_errmsg(m_pDB);
	return (Result != SQLITE_OK);
}

bool CSQLite::SResult::Step()
{
	return (sqlite3_step(m_pStmt) == SQLITE_ROW);
}

int CSQLite::SResult::GetInt(int Idx)
{
	return sqlite3_column_int(m_pStmt, Idx);
}

float CSQLite::SResult::GetFloat(int Idx)
{
	return (float)sqlite3_column_double(m_pStmt, Idx);
}

bool CSQLite::SResult::Prepare(const char *pQuery)
{
	if(m_pStmt)
		sqlite3_finalize(m_pStmt);
	m_pStmt = NULL;

	int Result = sqlite3_prepare_v2(
		m_pDB,
		pQuery,
		-1,
		&m_pStmt,
		NULL);

	return CheckError(Result);
}

int CSQLite::SResult::Execute()
{
	Step();
	return sqlite3_changes(m_pDB);
}

CSQLite::CSQLite()
{
	m_pDB = 0x0;
}

CSQLite::~CSQLite()
{
	Disconnect();
}

bool CSQLite::Connect(const char *pFile)
{
	return (sqlite3_open(pFile, &m_pDB) == SQLITE_OK);
}

void CSQLite::Disconnect()
{
	if(m_pDB)
	{
		sqlite3_close(m_pDB);

		dbg_msg("sqlite", "closing connection");

		m_IsRunning = false;
		thread_wait(m_pThread);
	}
}

void CSQLite::Run()
{
	if(!m_pDB)
		return;

	m_IsRunning = true;
	m_pThread = thread_init(th_func, this, "SQLite worker");
}

void CSQLite::Loop()
{
	if(m_Requests.empty())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
		return;
	}

	SRequest Req = m_Requests.front();
	m_Requests.pop();

	// Get
	SResult Res;
	Res.m_pDB = m_pDB;
	Res.m_pStmt = 0x0;

	luabridge::LuaRef Func = luabridge::getGlobal(Req.m_pState, Req.m_CallbackName.c_str());
	if(Func.isValid() && Func.isFunction())
	{
		luabridge::LuaResult LuaRes = Func(Res);
		if(LuaRes.hasFailed())
			dbg_msg("lua_error", "error: %s", LuaRes.errorMessage().c_str());
	}

	if(Res.m_pStmt)
		sqlite3_finalize(Res.m_pStmt);
}

void CSQLite::Execute(lua_State *L)
{
	SRequest Req;
	Req.m_pState = L;
	Req.m_CallbackName = luaL_checkstring(L, 2);

	m_Requests.push(Req);
}
