#ifndef GAME_SERVER_MMO_MULTIWORLD_MANAGER_H
#define GAME_SERVER_MMO_MULTIWORLD_MANAGER_H

#include <base/hash.h>

#include <game/collision.h>
#include <game/layers.h>
#include <game/server/gameworld.h>

#include <list>

class CMultiWorldManager
{
	class CGameContext *m_pGameServer;

	struct SWorld
	{
		class IEngineMap *m_pMap;
		CCollision *m_pCollision;
		CGameWorld *m_pGameWorld;
		CLayers *m_pLayers;

		// Used for map downloading
		unsigned char *m_pMapData;
		unsigned int m_MapSize;
		std::string m_MapName;
		SHA256_DIGEST m_MapSha256;
		unsigned m_MapCrc;
	};

	std::vector<SWorld> m_vWorlds;

	// Map sending
	int m_aNextMapChunks[MAX_CLIENTS];

public:
	void Init(CGameContext *pGameServer);
	bool LoadWorld(const char *pMapPath);
	void ChangeWorld(int ClientID, int WorldID);

	void SendMap(int ClientID, int WorldID);
	void SendMapData(int ClientID, int Chunk);

	CCollision *Collision(int WorldID);
	CGameWorld *GameWorld(int WorldID);
	CLayers *Layers(int WorldID);
	IMap *Map(int WorldID);

	void OnTick();
	void OnSnap(int ClientID);
	void OnShutdown();
};

#endif // GAME_SERVER_MMO_MULTIWORLD_MANAGER_H
