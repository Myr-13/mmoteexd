#include <game/server/gamecontext.h>

#include <lua/lua.hpp>
#include <engine/external/luabridge/LuaBridge.h>
#include <engine/server/server.h>
#include <engine/shared/lua/lua_state.h>

#define LUA_CHUNK_SIZE (1024 - 128)

void CGameContext::SendLuaFiles(int ClientID)
{
	CMsgPacker Msg(NETMSG_NUMBER_OF_LUA_FILES);
	Msg.AddInt(m_vClientFiles.size());
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);

	CServer::CClient &Client = ((CServer *)Server())->m_aClients[ClientID];
	Client.m_CurrentFileID = 0;

	SendLuaFileInfo(ClientID);
}

void CGameContext::SendLuaFileInfo(int ClientID)
{
	CServer::CClient &Client = ((CServer *)Server())->m_aClients[ClientID];
	Client.m_NextChunk = 0;

	const char *pFileName = m_vClientFiles[Client.m_CurrentFileID].c_str();
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "lua/%s", pFileName);

	IOHANDLE File = io_open(aBuf, IOFLAG_READ);
	if(!File)
	{
		dbg_msg("lua", "failed to open '%s' for start sending", aBuf);
		Server()->Kick(ClientID, "Failed to start transfer of file, ask admin in discord");
		return;
	}

	size_t Len = io_length(File);
	int ChunksNum = ceil(Len / LUA_CHUNK_SIZE);

	io_close(File);

	CMsgPacker Msg(NETMSG_LUA_FILE_INFO);
	Msg.AddString(pFileName);
	Msg.AddInt(ChunksNum);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_FLUSH, ClientID);
}

void CGameContext::SendLuaFileChunk(int ClientID, int Chunk)
{
	int Begin = Chunk * LUA_CHUNK_SIZE;

	CServer::CClient &Client = ((CServer *)Server())->m_aClients[ClientID];
	Client.m_NextChunk = 0;

	const char *pFileName = m_vClientFiles[Client.m_CurrentFileID].c_str();
	char aFileName[256];
	str_format(aFileName, sizeof(aFileName), "lua/%s", pFileName);

	IOHANDLE File = io_open(aFileName, IOFLAG_READ);
	if(!File)
	{
		dbg_msg("lua", "failed to open CS file for sending chunk");
		Server()->Kick(ClientID, "Failed to send file chunk, ask admin in discord");
		return;
	}

	char aBuf[LUA_CHUNK_SIZE];
	io_skip(File, Begin);
	size_t DataLen = io_read(File, aBuf, sizeof(aBuf));

	io_close(File);

	bool IsLastChunk = (DataLen != LUA_CHUNK_SIZE);
	int Flags = MSGFLAG_VITAL;
	if(IsLastChunk)
		Flags |= MSGFLAG_FLUSH;

	CMsgPacker Msg(NETMSG_LUA_FILE_CHUNK);
	Msg.AddInt(IsLastChunk);
	Msg.AddInt(DataLen);
	Msg.AddRaw(aBuf, DataLen);
	Server()->SendMsg(&Msg, Flags, ClientID);

	if(IsLastChunk && Client.m_CurrentFileID < m_vClientFiles.size() - 1)
	{
		// End of file, start next

		Client.m_CurrentFileID++;
		SendLuaFileInfo(ClientID);
	}
}

bool CGameContext::OnMMOMessage(int MsgID, CUnpacker *pUnpacker, int ClientID)
{
	switch(MsgID)
	{
	case NETMSG_LUA_FILE_CHUNK_REQUEST:
	{
		SendLuaFileChunk(ClientID, pUnpacker->GetInt());
	} return true;
	}

	for(auto &LF : SLuaState::ms_pLuaManager->GetLuaFiles())
	{
		luabridge::LuaRef Func = luabridge::getGlobal(LF.LuaState(), "OnMessage");
		if(Func.isValid() && Func.isFunction())
		{
			luabridge::LuaResult Res = Func(MsgID, pUnpacker, ClientID);
			if(Res.hasFailed())
				dbg_msg("lua_error", "error: %s", Res.errorMessage().c_str());

			if(Res.size() != 0)
			{
				luabridge::LuaRef Result = Res[0];
				if(Result.isValid() && Result.isBool() && Result)
					return true;
			}
		}
	}

	return false;
}

void CGameContext::OnMMOTick()
{
	LUA_FIRE_EVENT("OnTick")

	m_DB.Loop();
}

CPlayer *CGameContext::LuaGetPlayer(int ClientID)
{
	return SLuaState::ms_pGameServer->m_apPlayers[ClientID];
}

void CGameContext::ClearBotSnapIDs()
{
	for(int &i : m_aBotSnapIDs)
		i = 24;
}

int CGameContext::GetNextBotSnapID(int ClientID)
{
	int Prev = m_aBotSnapIDs[ClientID]++;
	return ((Prev >= MAX_CLIENTS) ? -1 : Prev);
}

CCollision *CGameContext::Collision(int WorldID)
{
	return m_MultiWorldManager.Collision(WorldID);
}

CGameWorld *CGameContext::GameWorld(int WorldID)
{
	return m_MultiWorldManager.GameWorld(WorldID);
}
