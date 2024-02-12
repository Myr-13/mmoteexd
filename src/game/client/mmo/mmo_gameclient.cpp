#include <game/client/gameclient.h>

#include <filesystem>

#define LUA_CHUNK_SIZE (1024 - 128)

bool CGameClient::OnMMOMessage(int MsgID, CUnpacker *pUnpacker)
{
	switch(MsgID)
	{
	case NETMSG_NUMBER_OF_LUA_FILES:
	{
		m_FilesToDownload = pUnpacker->GetInt();
		m_CurrentFile = 0x0;
	} return true;

	case NETMSG_LUA_FILE_INFO:
	{
		const char *pFileName = pUnpacker->GetString();
		m_NumChunks = pUnpacker->GetInt();
		m_NextChunk = 0;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "lua_cli/%s", pFileName);

		std::string Dirs = aBuf;
		size_t Pos = Dirs.rfind('/');
		if(Pos != std::string::npos)
			std::filesystem::create_directories(Dirs.erase(Pos));

		m_CurrentFile = io_open(aBuf, IOFLAG_WRITE);

		RequestFileChunk();
	} return true;

	case NETMSG_LUA_FILE_CHUNK:
	{
		bool IsLast = pUnpacker->GetInt();
		int DataLen = pUnpacker->GetInt();
		const unsigned char *pData = pUnpacker->GetRaw(DataLen);

		if(m_CurrentFile)
			io_write(m_CurrentFile, pData, DataLen);

		if(IsLast)
		{
			m_FilesToDownload--;

			if(m_CurrentFile)
				io_close(m_CurrentFile);
			m_CurrentFile = 0x0;

			dbg_msg("lua", "files left: %d", m_FilesToDownload);

			if(m_FilesToDownload == 0)
			{
				dbg_msg("lua", "starting lua");
				RunLua();
			}
		}
		else
		{
			m_NextChunk++;
			RequestFileChunk();
		}
	} return true;
	}

	return false;
}

void CGameClient::RequestFileChunk()
{
	CMsgPacker Msg(NETMSG_LUA_FILE_CHUNK_REQUEST);
	Msg.AddInt(m_NextChunk);
	Client()->SendMsgActive(&Msg, MSGFLAG_VITAL | MSGFLAG_FLUSH);
}

void CGameClient::RunLua()
{
	m_LuaManager.Open("lua_cli", false);
}
