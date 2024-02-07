#include "multiworld_manager.h"

#include <base/system.h>

#include <lua/lua.hpp>
#include <engine/shared/map.h>
#include <engine/shared/lua/lua_state.h>
#include <engine/external/luabridge/LuaBridge.h>

#include <game/server/gamecontext.h>
#include <game/server/player.h>

void CMultiWorldManager::Init(CGameContext *pGameServer)
{
	m_pGameServer = pGameServer;

	for(int &Val : m_aNextMapChunks)
		Val = 0;
}

bool CMultiWorldManager::LoadWorld(const char *pMapPath)
{
	dbg_msg("multi_worlds", "loading world: %s", pMapPath);

	SWorld World;
	World.m_pMap = new CMap();
	World.m_pMap->m_pKernel = m_pGameServer->Kernel();
	World.m_pCollision = new CCollision();
	World.m_pGameWorld = new CGameWorld();
	World.m_pLayers = new CLayers();

	char aPath[IO_MAX_PATH_LENGTH];
	str_format(aPath, sizeof(aPath), "maps/%s.map", pMapPath);

	// Load map
	if(!((CMap *)World.m_pMap)->Load(aPath))
		return false;

	// Setup some stuff
	World.m_MapSha256 = World.m_pMap->Sha256();
	World.m_MapCrc = World.m_pMap->Crc();
	World.m_MapName = pMapPath;

	// Load map into memory
	m_pGameServer->Storage()->ReadFile(aPath, IStorage::TYPE_ALL, (void **)&World.m_pMapData, &World.m_MapSize);

	// Init systems
	World.m_pLayers->InitNoKernel(World.m_pMap);
	World.m_pCollision->Init(World.m_pLayers);
	World.m_pGameWorld->SetGameServer(m_pGameServer);

	m_vWorlds.push_back(World);
	int WorldID = m_vWorlds.size() - 1;

	m_pGameServer->CreateAllEntities(WorldID, true);
	LUA_FIRE_EVENT("OnWorldLoaded", WorldID)

	dbg_msg("multi_worlds", "successfully loaded world %d: %s", WorldID, pMapPath);

	return true;
}

void CMultiWorldManager::ChangeWorld(int ClientID, int WorldID)
{
	m_pGameServer->Server()->SetClientWorld(ClientID, WorldID);

	SendMap(ClientID, WorldID);

	CPlayer *pPly = m_pGameServer->m_apPlayers[ClientID];
	if(pPly)
		pPly->KillCharacter();
}

void CMultiWorldManager::SendMap(int ClientID, int WorldID)
{
	if(WorldID < 0 || WorldID >= m_vWorlds.size())
	{
		dbg_msg("multi_worlds", "tried to send map with invalid world id: %d cid: %d", WorldID, ClientID);
		return;
	}
	SWorld &World = m_vWorlds[WorldID];

	// Map details
	{
		CMsgPacker Msg(NETMSG_MAP_DETAILS, true);
		Msg.AddString(World.m_MapName.c_str(), 0);
		Msg.AddRaw(World.m_MapSha256.data, sizeof(World.m_MapSha256.data));
		Msg.AddInt(World.m_MapCrc);
		Msg.AddInt(World.m_MapSize);
		Msg.AddString("", 0); // HTTPS map download URL

		m_pGameServer->Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
	}

	// Map change
	{
		CMsgPacker Msg(NETMSG_MAP_CHANGE, true);
		Msg.AddString(World.m_MapName.c_str(), 0);
		Msg.AddInt(World.m_MapCrc);
		Msg.AddInt(World.m_MapSize);

		m_pGameServer->Server()->SendMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_FLUSH, ClientID);
	}

	m_aNextMapChunks[ClientID] = 0;
}

void CMultiWorldManager::SendMapData(int ClientID, int Chunk)
{
	SWorld &World = m_vWorlds[m_pGameServer->Server()->GetClientWorld(ClientID)];

	unsigned int ChunkSize = 1024 - 128;
	unsigned int Offset = Chunk * ChunkSize;
	bool Last = false;

	if(Chunk < 0 || Offset > World.m_MapSize)
		return;

	if(Offset + ChunkSize >= World.m_MapSize)
	{
		ChunkSize = World.m_MapSize - Offset;
		Last = true;
	}

	CMsgPacker Msg(NETMSG_MAP_DATA, true);
	Msg.AddInt(Last);
	Msg.AddInt(World.m_MapCrc);
	Msg.AddInt(Chunk);
	Msg.AddInt(ChunkSize);
	Msg.AddRaw(&World.m_pMapData[Offset], ChunkSize);

	m_pGameServer->Server()->SendMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_FLUSH, ClientID);
}

CCollision *CMultiWorldManager::Collision(int WorldID)
{
	return m_vWorlds[WorldID].m_pCollision;
}

CGameWorld *CMultiWorldManager::GameWorld(int WorldID)
{
	return m_vWorlds[WorldID].m_pGameWorld;
}

CLayers *CMultiWorldManager::Layers(int WorldID)
{
	return m_vWorlds[WorldID].m_pLayers;
}

IMap *CMultiWorldManager::Map(int WorldID)
{
	return m_vWorlds[WorldID].m_pMap;
}

void CMultiWorldManager::OnTick()
{
	for(SWorld &World : m_vWorlds)
		World.m_pGameWorld->Tick();
}

void CMultiWorldManager::OnSnap(int ClientID)
{
	for(SWorld &World : m_vWorlds)
		World.m_pGameWorld->Snap(ClientID);
}

void CMultiWorldManager::OnShutdown()
{
	for(auto &World : m_vWorlds)
	{
		World.m_pMap->Shutdown();
		delete (CMap *)World.m_pMap;
		delete World.m_pCollision;
		delete World.m_pLayers;
		delete World.m_pGameWorld;

		free(World.m_pMapData);
	}
}
