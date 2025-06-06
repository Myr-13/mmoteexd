/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/demo.h>
#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/serverbrowser.h>
#include <engine/shared/config.h>
#include <engine/storage.h>

#include <game/client/gameclient.h>
#include <game/client/render.h>

#include <game/layers.h>
#include <game/mapitems.h>
#include <game/mapitems_ex.h>

#include <game/client/components/camera.h>
#include <game/client/components/mapimages.h>
#include <game/localization.h>

#include "maplayers.h"

#include <chrono>

using namespace std::chrono_literals;

CMapLayers::CMapLayers(int t, bool OnlineOnly)
{
	m_Type = t;
	m_pLayers = 0;
	m_CurrentLocalTick = 0;
	m_LastLocalTick = 0;
	m_EnvelopeUpdate = false;
	m_OnlineOnly = OnlineOnly;
}

void CMapLayers::OnInit()
{
	m_pLayers = Layers();
	m_pImages = &m_pClient->m_MapImages;
}

CCamera *CMapLayers::GetCurCamera()
{
	return &m_pClient->m_Camera;
}

void CMapLayers::EnvelopeUpdate()
{
	if(Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = DemoPlayer()->BaseInfo();
		m_CurrentLocalTick = pInfo->m_CurrentTick;
		m_LastLocalTick = pInfo->m_CurrentTick;
		m_EnvelopeUpdate = true;
	}
}

void CMapLayers::EnvelopeEval(int TimeOffsetMillis, int Env, ColorRGBA &Channels, void *pUser)
{
	CMapLayers *pThis = (CMapLayers *)pUser;
	Channels = ColorRGBA();

	int EnvStart, EnvNum;
	pThis->m_pLayers->Map()->GetType(MAPITEMTYPE_ENVELOPE, &EnvStart, &EnvNum);

	if(Env < 0 || Env >= EnvNum)
		return;

	const CMapItemEnvelope *pItem = (CMapItemEnvelope *)pThis->m_pLayers->Map()->GetItem(EnvStart + Env);

	CMapBasedEnvelopePointAccess EnvelopePoints(pThis->m_pLayers->Map());
	EnvelopePoints.SetPointsRange(pItem->m_StartPoint, pItem->m_NumPoints);
	if(EnvelopePoints.NumPoints() == 0)
		return;

	const auto TickToNanoSeconds = std::chrono::nanoseconds(1s) / (int64_t)pThis->Client()->GameTickSpeed();

	static std::chrono::nanoseconds s_Time{0};
	static auto s_LastLocalTime = time_get_nanoseconds();
	if(pThis->Client()->State() == IClient::STATE_DEMOPLAYBACK)
	{
		const IDemoPlayer::CInfo *pInfo = pThis->DemoPlayer()->BaseInfo();

		if(!pInfo->m_Paused || pThis->m_EnvelopeUpdate)
		{
			if(pThis->m_CurrentLocalTick != pInfo->m_CurrentTick)
			{
				pThis->m_LastLocalTick = pThis->m_CurrentLocalTick;
				pThis->m_CurrentLocalTick = pInfo->m_CurrentTick;
			}
			if(pItem->m_Version < 2 || pItem->m_Synchronized)
			{
				if(pThis->m_pClient->m_Snap.m_pGameInfoObj)
				{
					// get the lerp of the current tick and prev
					int MinTick = pThis->Client()->PrevGameTick(g_Config.m_ClDummy) - pThis->m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick;
					int CurTick = pThis->Client()->GameTick(g_Config.m_ClDummy) - pThis->m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick;
					s_Time = std::chrono::nanoseconds((int64_t)(mix<double>(
											    0,
											    (CurTick - MinTick),
											    (double)pThis->Client()->IntraGameTick(g_Config.m_ClDummy)) *
										    TickToNanoSeconds.count())) +
						 MinTick * TickToNanoSeconds;
				}
			}
			else
			{
				int MinTick = pThis->m_LastLocalTick;
				s_Time = std::chrono::nanoseconds((int64_t)(mix<double>(0,
										    pThis->m_CurrentLocalTick - MinTick,
										    (double)pThis->Client()->IntraGameTick(g_Config.m_ClDummy)) *
									    TickToNanoSeconds.count())) +
					 MinTick * TickToNanoSeconds;
			}
		}
		CRenderTools::RenderEvalEnvelope(&EnvelopePoints, 4, s_Time + (int64_t)TimeOffsetMillis * std::chrono::nanoseconds(1ms), Channels);
	}
	else
	{
		if(pThis->m_OnlineOnly && (pItem->m_Version < 2 || pItem->m_Synchronized))
		{
			if(pThis->m_pClient->m_Snap.m_pGameInfoObj) // && !(pThis->m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED))
			{
				// get the lerp of the current tick and prev
				int MinTick = pThis->Client()->PrevGameTick(g_Config.m_ClDummy) - pThis->m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick;
				int CurTick = pThis->Client()->GameTick(g_Config.m_ClDummy) - pThis->m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick;
				s_Time = std::chrono::nanoseconds((int64_t)(mix<double>(
										    0,
										    (CurTick - MinTick),
										    (double)pThis->Client()->IntraGameTick(g_Config.m_ClDummy)) *
									    TickToNanoSeconds.count())) +
					 MinTick * TickToNanoSeconds;
			}
		}
		else
		{
			auto CurTime = time_get_nanoseconds();
			s_Time += CurTime - s_LastLocalTime;
			s_LastLocalTime = CurTime;
		}
		CRenderTools::RenderEvalEnvelope(&EnvelopePoints, 4, s_Time + std::chrono::nanoseconds(std::chrono::milliseconds(TimeOffsetMillis)), Channels);
	}
}

void FillTmpTile(SGraphicTile *pTmpTile, SGraphicTileTexureCoords *pTmpTex, unsigned char Flags, unsigned char Index, int x, int y, const ivec2 &Offset, int Scale, CMapItemGroup *pGroup)
{
	if(pTmpTex)
	{
		unsigned char x0 = 0;
		unsigned char y0 = 0;
		unsigned char x1 = x0 + 1;
		unsigned char y1 = y0;
		unsigned char x2 = x0 + 1;
		unsigned char y2 = y0 + 1;
		unsigned char x3 = x0;
		unsigned char y3 = y0 + 1;

		if(Flags & TILEFLAG_XFLIP)
		{
			x0 = x2;
			x1 = x3;
			x2 = x3;
			x3 = x0;
		}

		if(Flags & TILEFLAG_YFLIP)
		{
			y0 = y3;
			y2 = y1;
			y3 = y1;
			y1 = y0;
		}

		if(Flags & TILEFLAG_ROTATE)
		{
			unsigned char Tmp = x0;
			x0 = x3;
			x3 = x2;
			x2 = x1;
			x1 = Tmp;
			Tmp = y0;
			y0 = y3;
			y3 = y2;
			y2 = y1;
			y1 = Tmp;
		}

		pTmpTex->m_TexCoordTopLeft.x = x0;
		pTmpTex->m_TexCoordTopLeft.y = y0;
		pTmpTex->m_TexCoordBottomLeft.x = x3;
		pTmpTex->m_TexCoordBottomLeft.y = y3;
		pTmpTex->m_TexCoordTopRight.x = x1;
		pTmpTex->m_TexCoordTopRight.y = y1;
		pTmpTex->m_TexCoordBottomRight.x = x2;
		pTmpTex->m_TexCoordBottomRight.y = y2;

		pTmpTex->m_TexCoordTopLeft.z = Index;
		pTmpTex->m_TexCoordBottomLeft.z = Index;
		pTmpTex->m_TexCoordTopRight.z = Index;
		pTmpTex->m_TexCoordBottomRight.z = Index;

		bool HasRotation = (Flags & TILEFLAG_ROTATE) != 0;
		pTmpTex->m_TexCoordTopLeft.w = HasRotation;
		pTmpTex->m_TexCoordBottomLeft.w = HasRotation;
		pTmpTex->m_TexCoordTopRight.w = HasRotation;
		pTmpTex->m_TexCoordBottomRight.w = HasRotation;
	}

	pTmpTile->m_TopLeft.x = x * Scale + Offset.x;
	pTmpTile->m_TopLeft.y = y * Scale + Offset.y;
	pTmpTile->m_BottomLeft.x = x * Scale + Offset.x;
	pTmpTile->m_BottomLeft.y = y * Scale + Scale + Offset.y;
	pTmpTile->m_TopRight.x = x * Scale + Scale + Offset.x;
	pTmpTile->m_TopRight.y = y * Scale + Offset.y;
	pTmpTile->m_BottomRight.x = x * Scale + Scale + Offset.x;
	pTmpTile->m_BottomRight.y = y * Scale + Scale + Offset.y;
}

void FillTmpTileSpeedup(SGraphicTile *pTmpTile, SGraphicTileTexureCoords *pTmpTex, unsigned char Flags, unsigned char Index, int x, int y, const ivec2 &Offset, int Scale, CMapItemGroup *pGroup, short AngleRotate)
{
	int Angle = AngleRotate % 360;
	FillTmpTile(pTmpTile, pTmpTex, Angle >= 270 ? ROTATION_270 : (Angle >= 180 ? ROTATION_180 : (Angle >= 90 ? ROTATION_90 : 0)), AngleRotate % 90, x, y, Offset, Scale, pGroup);
}

bool CMapLayers::STileLayerVisuals::Init(unsigned int Width, unsigned int Height)
{
	m_Width = Width;
	m_Height = Height;
	if(Width == 0 || Height == 0)
		return false;
	if constexpr(sizeof(unsigned int) >= sizeof(ptrdiff_t))
		if(Width >= std::numeric_limits<std::ptrdiff_t>::max() || Height >= std::numeric_limits<std::ptrdiff_t>::max())
			return false;

	m_pTilesOfLayer = new CMapLayers::STileLayerVisuals::STileVisual[Height * Width];

	m_vBorderTop.resize(Width);
	m_vBorderBottom.resize(Width);

	m_vBorderLeft.resize(Height);
	m_vBorderRight.resize(Height);
	return true;
}

CMapLayers::STileLayerVisuals::~STileLayerVisuals()
{
	delete[] m_pTilesOfLayer;

	m_pTilesOfLayer = NULL;
}

bool AddTile(std::vector<SGraphicTile> &vTmpTiles, std::vector<SGraphicTileTexureCoords> &vTmpTileTexCoords, unsigned char Index, unsigned char Flags, int x, int y, CMapItemGroup *pGroup, bool DoTextureCoords, bool FillSpeedup = false, int AngleRotate = -1, const ivec2 &Offset = ivec2{0, 0}, int Scale = 32)
{
	if(Index)
	{
		vTmpTiles.emplace_back();
		SGraphicTile &Tile = vTmpTiles.back();
		SGraphicTileTexureCoords *pTileTex = NULL;
		if(DoTextureCoords)
		{
			vTmpTileTexCoords.emplace_back();
			SGraphicTileTexureCoords &TileTex = vTmpTileTexCoords.back();
			pTileTex = &TileTex;
		}
		if(FillSpeedup)
			FillTmpTileSpeedup(&Tile, pTileTex, Flags, 0, x, y, Offset, Scale, pGroup, AngleRotate);
		else
			FillTmpTile(&Tile, pTileTex, Flags, Index, x, y, Offset, Scale, pGroup);

		return true;
	}
	return false;
}

struct STmpQuadVertexTextured
{
	float m_X, m_Y, m_CenterX, m_CenterY;
	unsigned char m_R, m_G, m_B, m_A;
	float m_U, m_V;
};

struct STmpQuadVertex
{
	float m_X, m_Y, m_CenterX, m_CenterY;
	unsigned char m_R, m_G, m_B, m_A;
};

struct STmpQuad
{
	STmpQuadVertex m_aVertices[4];
};

struct STmpQuadTextured
{
	STmpQuadVertexTextured m_aVertices[4];
};

void mem_copy_special(void *pDest, void *pSource, size_t Size, size_t Count, size_t Steps)
{
	size_t CurStep = 0;
	for(size_t i = 0; i < Count; ++i)
	{
		mem_copy(((char *)pDest) + CurStep + i * Size, ((char *)pSource) + i * Size, Size);
		CurStep += Steps;
	}
}

CMapLayers::~CMapLayers()
{
	//clear everything and destroy all buffers
	if(!m_vpTileLayerVisuals.empty())
	{
		int s = m_vpTileLayerVisuals.size();
		for(int i = 0; i < s; ++i)
		{
			delete m_vpTileLayerVisuals[i];
		}
	}
	if(!m_vpQuadLayerVisuals.empty())
	{
		int s = m_vpQuadLayerVisuals.size();
		for(int i = 0; i < s; ++i)
		{
			delete m_vpQuadLayerVisuals[i];
		}
	}
}

void CMapLayers::OnMapLoad()
{
	if(!Graphics()->IsTileBufferingEnabled() && !Graphics()->IsQuadBufferingEnabled())
		return;

	const char *pConnectCaption = GameClient()->DemoPlayer()->IsPlaying() ? Localize("Preparing demo playback") : Localize("Connected");
	const char *pLoadMapContent = Localize("Uploading map data to GPU");

	auto CurTime = time_get_nanoseconds();
	auto &&RenderLoading = [&]() {
		if(CanRenderMenuBackground())
			GameClient()->m_Menus.RenderLoading(pConnectCaption, pLoadMapContent, 0, false);
		else if(time_get_nanoseconds() - CurTime > 500ms)
			GameClient()->m_Menus.RenderLoading(pConnectCaption, pLoadMapContent, 0, false, false);
	};

	//clear everything and destroy all buffers
	if(!m_vpTileLayerVisuals.empty())
	{
		int s = m_vpTileLayerVisuals.size();
		for(int i = 0; i < s; ++i)
		{
			Graphics()->DeleteBufferContainer(m_vpTileLayerVisuals[i]->m_BufferContainerIndex, true);
			delete m_vpTileLayerVisuals[i];
		}
		m_vpTileLayerVisuals.clear();
	}
	if(!m_vpQuadLayerVisuals.empty())
	{
		int s = m_vpQuadLayerVisuals.size();
		for(int i = 0; i < s; ++i)
		{
			Graphics()->DeleteBufferContainer(m_vpQuadLayerVisuals[i]->m_BufferContainerIndex, true);
			delete m_vpQuadLayerVisuals[i];
		}
		m_vpQuadLayerVisuals.clear();

		RenderLoading();
	}

	bool PassedGameLayer = false;
	//prepare all visuals for all tile layers
	std::vector<SGraphicTile> vtmpTiles;
	std::vector<SGraphicTileTexureCoords> vtmpTileTexCoords;
	std::vector<SGraphicTile> vtmpBorderTopTiles;
	std::vector<SGraphicTileTexureCoords> vtmpBorderTopTilesTexCoords;
	std::vector<SGraphicTile> vtmpBorderLeftTiles;
	std::vector<SGraphicTileTexureCoords> vtmpBorderLeftTilesTexCoords;
	std::vector<SGraphicTile> vtmpBorderRightTiles;
	std::vector<SGraphicTileTexureCoords> vtmpBorderRightTilesTexCoords;
	std::vector<SGraphicTile> vtmpBorderBottomTiles;
	std::vector<SGraphicTileTexureCoords> vtmpBorderBottomTilesTexCoords;
	std::vector<SGraphicTile> vtmpBorderCorners;
	std::vector<SGraphicTileTexureCoords> vtmpBorderCornersTexCoords;

	std::vector<STmpQuad> vtmpQuads;
	std::vector<STmpQuadTextured> vtmpQuadsTextured;

	for(int g = 0; g < m_pLayers->NumGroups(); g++)
	{
		CMapItemGroup *pGroup = m_pLayers->GetGroup(g);
		if(!pGroup)
		{
			dbg_msg("maplayers", "error group was null, group number = %d, total groups = %d", g, m_pLayers->NumGroups());
			dbg_msg("maplayers", "this is here to prevent a crash but the source of this is unknown, please report this for it to get fixed");
			dbg_msg("maplayers", "we need mapname and crc and the map that caused this if possible, and anymore info you think is relevant");
			continue;
		}

		for(int l = 0; l < pGroup->m_NumLayers; l++)
		{
			CMapItemLayer *pLayer = m_pLayers->GetLayer(pGroup->m_StartLayer + l);
			bool IsFrontLayer = false;
			bool IsSwitchLayer = false;
			bool IsTeleLayer = false;
			bool IsSpeedupLayer = false;
			bool IsTuneLayer = false;
			bool IsGameLayer = false;
			bool IsEntityLayer = false;

			if(pLayer == (CMapItemLayer *)m_pLayers->GameLayer())
			{
				IsGameLayer = true;
				IsEntityLayer = true;
				PassedGameLayer = true;
			}

			if(pLayer == (CMapItemLayer *)m_pLayers->FrontLayer())
				IsEntityLayer = IsFrontLayer = true;

			if(pLayer == (CMapItemLayer *)m_pLayers->SwitchLayer())
				IsEntityLayer = IsSwitchLayer = true;

			if(pLayer == (CMapItemLayer *)m_pLayers->TeleLayer())
				IsEntityLayer = IsTeleLayer = true;

			if(pLayer == (CMapItemLayer *)m_pLayers->SpeedupLayer())
				IsEntityLayer = IsSpeedupLayer = true;

			if(pLayer == (CMapItemLayer *)m_pLayers->TuneLayer())
				IsEntityLayer = IsTuneLayer = true;

			if(m_Type <= TYPE_BACKGROUND_FORCE)
			{
				if(PassedGameLayer)
					return;
			}
			else if(m_Type == TYPE_FOREGROUND)
			{
				if(!PassedGameLayer)
					continue;
			}

			if(pLayer->m_Type == LAYERTYPE_TILES && Graphics()->IsTileBufferingEnabled())
			{
				bool DoTextureCoords = false;
				CMapItemLayerTilemap *pTMap = (CMapItemLayerTilemap *)pLayer;
				if(pTMap->m_Image == -1)
				{
					if(IsEntityLayer)
						DoTextureCoords = true;
				}
				else
					DoTextureCoords = true;

				int DataIndex = 0;
				unsigned int TileSize = 0;
				int OverlayCount = 0;
				if(IsFrontLayer)
				{
					DataIndex = pTMap->m_Front;
					TileSize = sizeof(CTile);
				}
				else if(IsSwitchLayer)
				{
					DataIndex = pTMap->m_Switch;
					TileSize = sizeof(CSwitchTile);
					OverlayCount = 2;
				}
				else if(IsTeleLayer)
				{
					DataIndex = pTMap->m_Tele;
					TileSize = sizeof(CTeleTile);
					OverlayCount = 1;
				}
				else if(IsSpeedupLayer)
				{
					DataIndex = pTMap->m_Speedup;
					TileSize = sizeof(CSpeedupTile);
					OverlayCount = 2;
				}
				else if(IsTuneLayer)
				{
					DataIndex = pTMap->m_Tune;
					TileSize = sizeof(CTuneTile);
				}
				else
				{
					DataIndex = pTMap->m_Data;
					TileSize = sizeof(CTile);
				}
				unsigned int Size = m_pLayers->Map()->GetDataSize(DataIndex);
				void *pTiles = m_pLayers->Map()->GetData(DataIndex);

				if(Size >= pTMap->m_Width * pTMap->m_Height * TileSize)
				{
					int CurOverlay = 0;
					while(CurOverlay < OverlayCount + 1)
					{
						// We can later just count the tile layers to get the idx in the vector
						m_vpTileLayerVisuals.push_back(new STileLayerVisuals());
						STileLayerVisuals &Visuals = *m_vpTileLayerVisuals.back();
						if(!Visuals.Init(pTMap->m_Width, pTMap->m_Height))
						{
							++CurOverlay;
							continue;
						}
						Visuals.m_IsTextured = DoTextureCoords;

						vtmpTiles.clear();
						vtmpTileTexCoords.clear();

						vtmpBorderTopTiles.clear();
						vtmpBorderLeftTiles.clear();
						vtmpBorderRightTiles.clear();
						vtmpBorderBottomTiles.clear();
						vtmpBorderCorners.clear();
						vtmpBorderTopTilesTexCoords.clear();
						vtmpBorderLeftTilesTexCoords.clear();
						vtmpBorderRightTilesTexCoords.clear();
						vtmpBorderBottomTilesTexCoords.clear();
						vtmpBorderCornersTexCoords.clear();

						if(!DoTextureCoords)
						{
							vtmpTiles.reserve((size_t)pTMap->m_Width * pTMap->m_Height);
							vtmpBorderTopTiles.reserve((size_t)pTMap->m_Width);
							vtmpBorderBottomTiles.reserve((size_t)pTMap->m_Width);
							vtmpBorderLeftTiles.reserve((size_t)pTMap->m_Height);
							vtmpBorderRightTiles.reserve((size_t)pTMap->m_Height);
							vtmpBorderCorners.reserve((size_t)4);
						}
						else
						{
							vtmpTileTexCoords.reserve((size_t)pTMap->m_Width * pTMap->m_Height);
							vtmpBorderTopTilesTexCoords.reserve((size_t)pTMap->m_Width);
							vtmpBorderBottomTilesTexCoords.reserve((size_t)pTMap->m_Width);
							vtmpBorderLeftTilesTexCoords.reserve((size_t)pTMap->m_Height);
							vtmpBorderRightTilesTexCoords.reserve((size_t)pTMap->m_Height);
							vtmpBorderCornersTexCoords.reserve((size_t)4);
						}

						int x = 0;
						int y = 0;
						for(y = 0; y < pTMap->m_Height; ++y)
						{
							for(x = 0; x < pTMap->m_Width; ++x)
							{
								unsigned char Index = 0;
								unsigned char Flags = 0;
								int AngleRotate = -1;
								if(IsEntityLayer)
								{
									if(IsGameLayer)
									{
										Index = ((CTile *)pTiles)[y * pTMap->m_Width + x].m_Index;
										Flags = ((CTile *)pTiles)[y * pTMap->m_Width + x].m_Flags;
									}
									if(IsFrontLayer)
									{
										Index = ((CTile *)pTiles)[y * pTMap->m_Width + x].m_Index;
										Flags = ((CTile *)pTiles)[y * pTMap->m_Width + x].m_Flags;
									}
									if(IsSwitchLayer)
									{
										Flags = 0;
										Index = ((CSwitchTile *)pTiles)[y * pTMap->m_Width + x].m_Type;
										if(CurOverlay == 0)
										{
											Flags = ((CSwitchTile *)pTiles)[y * pTMap->m_Width + x].m_Flags;
										}
										else if(CurOverlay == 1)
											Index = ((CSwitchTile *)pTiles)[y * pTMap->m_Width + x].m_Number;
										else if(CurOverlay == 2)
											Index = ((CSwitchTile *)pTiles)[y * pTMap->m_Width + x].m_Delay;
									}
									if(IsTeleLayer)
									{
										Index = ((CTeleTile *)pTiles)[y * pTMap->m_Width + x].m_Type;
										Flags = 0;
										if(CurOverlay == 1)
										{
											if(IsTeleTileNumberUsedAny(Index))
												Index = ((CTeleTile *)pTiles)[y * pTMap->m_Width + x].m_Number;
											else
												Index = 0;
										}
									}
									if(IsSpeedupLayer)
									{
										Index = ((CSpeedupTile *)pTiles)[y * pTMap->m_Width + x].m_Type;
										Flags = 0;
										AngleRotate = ((CSpeedupTile *)pTiles)[y * pTMap->m_Width + x].m_Angle;
										if(((CSpeedupTile *)pTiles)[y * pTMap->m_Width + x].m_Force == 0)
											Index = 0;
										else if(CurOverlay == 1)
											Index = ((CSpeedupTile *)pTiles)[y * pTMap->m_Width + x].m_Force;
										else if(CurOverlay == 2)
											Index = ((CSpeedupTile *)pTiles)[y * pTMap->m_Width + x].m_MaxSpeed;
									}
									if(IsTuneLayer)
									{
										Index = ((CTuneTile *)pTiles)[y * pTMap->m_Width + x].m_Type;
										Flags = 0;
									}
								}
								else
								{
									Index = ((CTile *)pTiles)[y * pTMap->m_Width + x].m_Index;
									Flags = ((CTile *)pTiles)[y * pTMap->m_Width + x].m_Flags;
								}

								//the amount of tiles handled before this tile
								int TilesHandledCount = vtmpTiles.size();
								Visuals.m_pTilesOfLayer[y * pTMap->m_Width + x].SetIndexBufferByteOffset((offset_ptr32)(TilesHandledCount));

								bool AddAsSpeedup = false;
								if(IsSpeedupLayer && CurOverlay == 0)
									AddAsSpeedup = true;

								if(AddTile(vtmpTiles, vtmpTileTexCoords, Index, Flags, x, y, pGroup, DoTextureCoords, AddAsSpeedup, AngleRotate))
									Visuals.m_pTilesOfLayer[y * pTMap->m_Width + x].Draw(true);

								//do the border tiles
								if(x == 0)
								{
									if(y == 0)
									{
										Visuals.m_BorderTopLeft.SetIndexBufferByteOffset((offset_ptr32)(vtmpBorderCorners.size()));
										if(AddTile(vtmpBorderCorners, vtmpBorderCornersTexCoords, Index, Flags, 0, 0, pGroup, DoTextureCoords, AddAsSpeedup, AngleRotate, ivec2{-32, -32}))
											Visuals.m_BorderTopLeft.Draw(true);
									}
									else if(y == pTMap->m_Height - 1)
									{
										Visuals.m_BorderBottomLeft.SetIndexBufferByteOffset((offset_ptr32)(vtmpBorderCorners.size()));
										if(AddTile(vtmpBorderCorners, vtmpBorderCornersTexCoords, Index, Flags, 0, 0, pGroup, DoTextureCoords, AddAsSpeedup, AngleRotate, ivec2{-32, 0}))
											Visuals.m_BorderBottomLeft.Draw(true);
									}
									Visuals.m_vBorderLeft[y].SetIndexBufferByteOffset((offset_ptr32)(vtmpBorderLeftTiles.size()));
									if(AddTile(vtmpBorderLeftTiles, vtmpBorderLeftTilesTexCoords, Index, Flags, 0, y, pGroup, DoTextureCoords, AddAsSpeedup, AngleRotate, ivec2{-32, 0}))
										Visuals.m_vBorderLeft[y].Draw(true);
								}
								else if(x == pTMap->m_Width - 1)
								{
									if(y == 0)
									{
										Visuals.m_BorderTopRight.SetIndexBufferByteOffset((offset_ptr32)(vtmpBorderCorners.size()));
										if(AddTile(vtmpBorderCorners, vtmpBorderCornersTexCoords, Index, Flags, 0, 0, pGroup, DoTextureCoords, AddAsSpeedup, AngleRotate, ivec2{0, -32}))
											Visuals.m_BorderTopRight.Draw(true);
									}
									else if(y == pTMap->m_Height - 1)
									{
										Visuals.m_BorderBottomRight.SetIndexBufferByteOffset((offset_ptr32)(vtmpBorderCorners.size()));
										if(AddTile(vtmpBorderCorners, vtmpBorderCornersTexCoords, Index, Flags, 0, 0, pGroup, DoTextureCoords, AddAsSpeedup, AngleRotate, ivec2{0, 0}))
											Visuals.m_BorderBottomRight.Draw(true);
									}
									Visuals.m_vBorderRight[y].SetIndexBufferByteOffset((offset_ptr32)(vtmpBorderRightTiles.size()));
									if(AddTile(vtmpBorderRightTiles, vtmpBorderRightTilesTexCoords, Index, Flags, 0, y, pGroup, DoTextureCoords, AddAsSpeedup, AngleRotate, ivec2{0, 0}))
										Visuals.m_vBorderRight[y].Draw(true);
								}
								if(y == 0)
								{
									Visuals.m_vBorderTop[x].SetIndexBufferByteOffset((offset_ptr32)(vtmpBorderTopTiles.size()));
									if(AddTile(vtmpBorderTopTiles, vtmpBorderTopTilesTexCoords, Index, Flags, x, 0, pGroup, DoTextureCoords, AddAsSpeedup, AngleRotate, ivec2{0, -32}))
										Visuals.m_vBorderTop[x].Draw(true);
								}
								else if(y == pTMap->m_Height - 1)
								{
									Visuals.m_vBorderBottom[x].SetIndexBufferByteOffset((offset_ptr32)(vtmpBorderBottomTiles.size()));
									if(AddTile(vtmpBorderBottomTiles, vtmpBorderBottomTilesTexCoords, Index, Flags, x, 0, pGroup, DoTextureCoords, AddAsSpeedup, AngleRotate, ivec2{0, 0}))
										Visuals.m_vBorderBottom[x].Draw(true);
								}
							}
						}

						//append one kill tile to the gamelayer
						if(IsGameLayer)
						{
							Visuals.m_BorderKillTile.SetIndexBufferByteOffset((offset_ptr32)(vtmpTiles.size()));
							if(AddTile(vtmpTiles, vtmpTileTexCoords, TILE_DEATH, 0, 0, 0, pGroup, DoTextureCoords))
								Visuals.m_BorderKillTile.Draw(true);
						}

						//add the border corners, then the borders and fix their byte offsets
						int TilesHandledCount = vtmpTiles.size();
						Visuals.m_BorderTopLeft.AddIndexBufferByteOffset(TilesHandledCount);
						Visuals.m_BorderTopRight.AddIndexBufferByteOffset(TilesHandledCount);
						Visuals.m_BorderBottomLeft.AddIndexBufferByteOffset(TilesHandledCount);
						Visuals.m_BorderBottomRight.AddIndexBufferByteOffset(TilesHandledCount);
						//add the Corners to the tiles
						vtmpTiles.insert(vtmpTiles.end(), vtmpBorderCorners.begin(), vtmpBorderCorners.end());
						vtmpTileTexCoords.insert(vtmpTileTexCoords.end(), vtmpBorderCornersTexCoords.begin(), vtmpBorderCornersTexCoords.end());

						//now the borders
						TilesHandledCount = vtmpTiles.size();
						if(pTMap->m_Width > 0)
						{
							for(int i = 0; i < pTMap->m_Width; ++i)
							{
								Visuals.m_vBorderTop[i].AddIndexBufferByteOffset(TilesHandledCount);
							}
						}
						vtmpTiles.insert(vtmpTiles.end(), vtmpBorderTopTiles.begin(), vtmpBorderTopTiles.end());
						vtmpTileTexCoords.insert(vtmpTileTexCoords.end(), vtmpBorderTopTilesTexCoords.begin(), vtmpBorderTopTilesTexCoords.end());

						TilesHandledCount = vtmpTiles.size();
						if(pTMap->m_Width > 0)
						{
							for(int i = 0; i < pTMap->m_Width; ++i)
							{
								Visuals.m_vBorderBottom[i].AddIndexBufferByteOffset(TilesHandledCount);
							}
						}
						vtmpTiles.insert(vtmpTiles.end(), vtmpBorderBottomTiles.begin(), vtmpBorderBottomTiles.end());
						vtmpTileTexCoords.insert(vtmpTileTexCoords.end(), vtmpBorderBottomTilesTexCoords.begin(), vtmpBorderBottomTilesTexCoords.end());

						TilesHandledCount = vtmpTiles.size();
						if(pTMap->m_Height > 0)
						{
							for(int i = 0; i < pTMap->m_Height; ++i)
							{
								Visuals.m_vBorderLeft[i].AddIndexBufferByteOffset(TilesHandledCount);
							}
						}
						vtmpTiles.insert(vtmpTiles.end(), vtmpBorderLeftTiles.begin(), vtmpBorderLeftTiles.end());
						vtmpTileTexCoords.insert(vtmpTileTexCoords.end(), vtmpBorderLeftTilesTexCoords.begin(), vtmpBorderLeftTilesTexCoords.end());

						TilesHandledCount = vtmpTiles.size();
						if(pTMap->m_Height > 0)
						{
							for(int i = 0; i < pTMap->m_Height; ++i)
							{
								Visuals.m_vBorderRight[i].AddIndexBufferByteOffset(TilesHandledCount);
							}
						}
						vtmpTiles.insert(vtmpTiles.end(), vtmpBorderRightTiles.begin(), vtmpBorderRightTiles.end());
						vtmpTileTexCoords.insert(vtmpTileTexCoords.end(), vtmpBorderRightTilesTexCoords.begin(), vtmpBorderRightTilesTexCoords.end());

						//setup params
						float *pTmpTiles = vtmpTiles.empty() ? NULL : (float *)vtmpTiles.data();
						unsigned char *pTmpTileTexCoords = vtmpTileTexCoords.empty() ? NULL : (unsigned char *)vtmpTileTexCoords.data();

						Visuals.m_BufferContainerIndex = -1;
						size_t UploadDataSize = vtmpTileTexCoords.size() * sizeof(SGraphicTileTexureCoords) + vtmpTiles.size() * sizeof(SGraphicTile);
						if(UploadDataSize > 0)
						{
							char *pUploadData = (char *)malloc(sizeof(char) * UploadDataSize);

							mem_copy_special(pUploadData, pTmpTiles, sizeof(vec2), vtmpTiles.size() * 4, (DoTextureCoords ? sizeof(ubvec4) : 0));
							if(DoTextureCoords)
							{
								mem_copy_special(pUploadData + sizeof(vec2), pTmpTileTexCoords, sizeof(ubvec4), vtmpTiles.size() * 4, sizeof(vec2));
							}

							// first create the buffer object
							int BufferObjectIndex = Graphics()->CreateBufferObject(UploadDataSize, pUploadData, 0, true);

							// then create the buffer container
							SBufferContainerInfo ContainerInfo;
							ContainerInfo.m_Stride = (DoTextureCoords ? (sizeof(float) * 2 + sizeof(ubvec4)) : 0);
							ContainerInfo.m_VertBufferBindingIndex = BufferObjectIndex;
							ContainerInfo.m_vAttributes.emplace_back();
							SBufferContainerInfo::SAttribute *pAttr = &ContainerInfo.m_vAttributes.back();
							pAttr->m_DataTypeCount = 2;
							pAttr->m_Type = GRAPHICS_TYPE_FLOAT;
							pAttr->m_Normalized = false;
							pAttr->m_pOffset = 0;
							pAttr->m_FuncType = 0;
							if(DoTextureCoords)
							{
								ContainerInfo.m_vAttributes.emplace_back();
								pAttr = &ContainerInfo.m_vAttributes.back();
								pAttr->m_DataTypeCount = 4;
								pAttr->m_Type = GRAPHICS_TYPE_UNSIGNED_BYTE;
								pAttr->m_Normalized = false;
								pAttr->m_pOffset = (void *)(sizeof(vec2));
								pAttr->m_FuncType = 1;
							}

							Visuals.m_BufferContainerIndex = Graphics()->CreateBufferContainer(&ContainerInfo);
							// and finally inform the backend how many indices are required
							Graphics()->IndicesNumRequiredNotify(vtmpTiles.size() * 6);

							RenderLoading();
						}

						++CurOverlay;
					}
				}
			}
			else if(pLayer->m_Type == LAYERTYPE_QUADS && Graphics()->IsQuadBufferingEnabled())
			{
				CMapItemLayerQuads *pQLayer = (CMapItemLayerQuads *)pLayer;

				m_vpQuadLayerVisuals.push_back(new SQuadLayerVisuals());
				SQuadLayerVisuals *pQLayerVisuals = m_vpQuadLayerVisuals.back();

				bool Textured = (pQLayer->m_Image != -1);

				vtmpQuads.clear();
				vtmpQuadsTextured.clear();

				if(Textured)
					vtmpQuadsTextured.resize(pQLayer->m_NumQuads);
				else
					vtmpQuads.resize(pQLayer->m_NumQuads);

				CQuad *pQuads = (CQuad *)m_pLayers->Map()->GetDataSwapped(pQLayer->m_Data);
				for(int i = 0; i < pQLayer->m_NumQuads; ++i)
				{
					CQuad *pQuad = &pQuads[i];
					for(int j = 0; j < 4; ++j)
					{
						int QuadIDX = j;
						if(j == 2)
							QuadIDX = 3;
						else if(j == 3)
							QuadIDX = 2;
						if(!Textured)
						{
							// ignore the conversion for the position coordinates
							vtmpQuads[i].m_aVertices[j].m_X = (pQuad->m_aPoints[QuadIDX].x);
							vtmpQuads[i].m_aVertices[j].m_Y = (pQuad->m_aPoints[QuadIDX].y);
							vtmpQuads[i].m_aVertices[j].m_CenterX = (pQuad->m_aPoints[4].x);
							vtmpQuads[i].m_aVertices[j].m_CenterY = (pQuad->m_aPoints[4].y);
							vtmpQuads[i].m_aVertices[j].m_R = (unsigned char)pQuad->m_aColors[QuadIDX].r;
							vtmpQuads[i].m_aVertices[j].m_G = (unsigned char)pQuad->m_aColors[QuadIDX].g;
							vtmpQuads[i].m_aVertices[j].m_B = (unsigned char)pQuad->m_aColors[QuadIDX].b;
							vtmpQuads[i].m_aVertices[j].m_A = (unsigned char)pQuad->m_aColors[QuadIDX].a;
						}
						else
						{
							// ignore the conversion for the position coordinates
							vtmpQuadsTextured[i].m_aVertices[j].m_X = (pQuad->m_aPoints[QuadIDX].x);
							vtmpQuadsTextured[i].m_aVertices[j].m_Y = (pQuad->m_aPoints[QuadIDX].y);
							vtmpQuadsTextured[i].m_aVertices[j].m_CenterX = (pQuad->m_aPoints[4].x);
							vtmpQuadsTextured[i].m_aVertices[j].m_CenterY = (pQuad->m_aPoints[4].y);
							vtmpQuadsTextured[i].m_aVertices[j].m_U = fx2f(pQuad->m_aTexcoords[QuadIDX].x);
							vtmpQuadsTextured[i].m_aVertices[j].m_V = fx2f(pQuad->m_aTexcoords[QuadIDX].y);
							vtmpQuadsTextured[i].m_aVertices[j].m_R = (unsigned char)pQuad->m_aColors[QuadIDX].r;
							vtmpQuadsTextured[i].m_aVertices[j].m_G = (unsigned char)pQuad->m_aColors[QuadIDX].g;
							vtmpQuadsTextured[i].m_aVertices[j].m_B = (unsigned char)pQuad->m_aColors[QuadIDX].b;
							vtmpQuadsTextured[i].m_aVertices[j].m_A = (unsigned char)pQuad->m_aColors[QuadIDX].a;
						}
					}
				}

				size_t UploadDataSize = 0;
				if(Textured)
					UploadDataSize = vtmpQuadsTextured.size() * sizeof(STmpQuadTextured);
				else
					UploadDataSize = vtmpQuads.size() * sizeof(STmpQuad);

				if(UploadDataSize > 0)
				{
					void *pUploadData = NULL;
					if(Textured)
						pUploadData = vtmpQuadsTextured.data();
					else
						pUploadData = vtmpQuads.data();
					// create the buffer object
					int BufferObjectIndex = Graphics()->CreateBufferObject(UploadDataSize, pUploadData, 0);
					// then create the buffer container
					SBufferContainerInfo ContainerInfo;
					ContainerInfo.m_Stride = (Textured ? (sizeof(STmpQuadTextured) / 4) : (sizeof(STmpQuad) / 4));
					ContainerInfo.m_VertBufferBindingIndex = BufferObjectIndex;
					ContainerInfo.m_vAttributes.emplace_back();
					SBufferContainerInfo::SAttribute *pAttr = &ContainerInfo.m_vAttributes.back();
					pAttr->m_DataTypeCount = 4;
					pAttr->m_Type = GRAPHICS_TYPE_FLOAT;
					pAttr->m_Normalized = false;
					pAttr->m_pOffset = 0;
					pAttr->m_FuncType = 0;
					ContainerInfo.m_vAttributes.emplace_back();
					pAttr = &ContainerInfo.m_vAttributes.back();
					pAttr->m_DataTypeCount = 4;
					pAttr->m_Type = GRAPHICS_TYPE_UNSIGNED_BYTE;
					pAttr->m_Normalized = true;
					pAttr->m_pOffset = (void *)(sizeof(float) * 4);
					pAttr->m_FuncType = 0;
					if(Textured)
					{
						ContainerInfo.m_vAttributes.emplace_back();
						pAttr = &ContainerInfo.m_vAttributes.back();
						pAttr->m_DataTypeCount = 2;
						pAttr->m_Type = GRAPHICS_TYPE_FLOAT;
						pAttr->m_Normalized = false;
						pAttr->m_pOffset = (void *)(sizeof(float) * 4 + sizeof(unsigned char) * 4);
						pAttr->m_FuncType = 0;
					}

					pQLayerVisuals->m_BufferContainerIndex = Graphics()->CreateBufferContainer(&ContainerInfo);
					// and finally inform the backend how many indices are required
					Graphics()->IndicesNumRequiredNotify(pQLayer->m_NumQuads * 6);

					RenderLoading();
				}
			}
		}
	}
}

void CMapLayers::RenderTileLayer(int LayerIndex, ColorRGBA &Color, CMapItemLayerTilemap *pTileLayer, CMapItemGroup *pGroup)
{
	STileLayerVisuals &Visuals = *m_vpTileLayerVisuals[LayerIndex];
	if(Visuals.m_BufferContainerIndex == -1)
		return; //no visuals were created

	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);

	ColorRGBA Channels(1.f, 1.f, 1.f, 1.f);
	if(pTileLayer->m_ColorEnv >= 0)
	{
		EnvelopeEval(pTileLayer->m_ColorEnvOffset, pTileLayer->m_ColorEnv, Channels, this);
	}

	int BorderX0, BorderY0, BorderX1, BorderY1;
	bool DrawBorder = false;

	int Y0 = BorderY0 = std::floor(ScreenY0 / 32);
	int X0 = BorderX0 = std::floor(ScreenX0 / 32);
	int Y1 = BorderY1 = std::ceil(ScreenY1 / 32);
	int X1 = BorderX1 = std::ceil(ScreenX1 / 32);

	if(X0 < 0)
	{
		X0 = 0;
		DrawBorder = true;
	}
	if(Y0 < 0)
	{
		Y0 = 0;
		DrawBorder = true;
	}
	if(X1 > pTileLayer->m_Width)
	{
		X1 = pTileLayer->m_Width;
		DrawBorder = true;
	}
	if(Y1 > pTileLayer->m_Height)
	{
		Y1 = pTileLayer->m_Height;
		DrawBorder = true;
	}

	bool DrawLayer = true;
	if(X1 <= 0)
		DrawLayer = false;
	if(Y1 <= 0)
		DrawLayer = false;
	if(X0 >= pTileLayer->m_Width)
		DrawLayer = false;
	if(Y0 >= pTileLayer->m_Height)
		DrawLayer = false;

	if(DrawLayer)
	{
		//create the indice buffers we want to draw -- reuse them
		static std::vector<char *> s_vpIndexOffsets;
		static std::vector<unsigned int> s_vDrawCounts;

		s_vpIndexOffsets.clear();
		s_vDrawCounts.clear();

		unsigned long long Reserve = absolute(Y1 - Y0) + 1;
		s_vpIndexOffsets.reserve(Reserve);
		s_vDrawCounts.reserve(Reserve);

		for(int y = Y0; y < Y1; ++y)
		{
			if(X0 > X1)
				continue;
			int XR = X1 - 1;

			dbg_assert(Visuals.m_pTilesOfLayer[y * pTileLayer->m_Width + XR].IndexBufferByteOffset() >= Visuals.m_pTilesOfLayer[y * pTileLayer->m_Width + X0].IndexBufferByteOffset(), "Tile count wrong.");

			unsigned int NumVertices = ((Visuals.m_pTilesOfLayer[y * pTileLayer->m_Width + XR].IndexBufferByteOffset() - Visuals.m_pTilesOfLayer[y * pTileLayer->m_Width + X0].IndexBufferByteOffset()) / sizeof(unsigned int)) + (Visuals.m_pTilesOfLayer[y * pTileLayer->m_Width + XR].DoDraw() ? 6lu : 0lu);

			if(NumVertices)
			{
				s_vpIndexOffsets.push_back((offset_ptr_size)Visuals.m_pTilesOfLayer[y * pTileLayer->m_Width + X0].IndexBufferByteOffset());
				s_vDrawCounts.push_back(NumVertices);
			}
		}

		Color.x *= Channels.r;
		Color.y *= Channels.g;
		Color.z *= Channels.b;
		Color.w *= Channels.a;

		int DrawCount = s_vpIndexOffsets.size();
		if(DrawCount != 0)
		{
			Graphics()->RenderTileLayer(Visuals.m_BufferContainerIndex, Color, s_vpIndexOffsets.data(), s_vDrawCounts.data(), DrawCount);
		}
	}

	if(DrawBorder)
		RenderTileBorder(LayerIndex, Color, pTileLayer, pGroup, BorderX0, BorderY0, BorderX1, BorderY1);
}

void CMapLayers::RenderTileBorder(int LayerIndex, const ColorRGBA &Color, CMapItemLayerTilemap *pTileLayer, CMapItemGroup *pGroup, int BorderX0, int BorderY0, int BorderX1, int BorderY1)
{
	STileLayerVisuals &Visuals = *m_vpTileLayerVisuals[LayerIndex];

	int Y0 = BorderY0;
	int X0 = BorderX0;
	int Y1 = BorderY1;
	int X1 = BorderX1;

	if(X0 < 0)
		X0 = 0;
	if(Y0 < 0)
		Y0 = 0;
	if(X1 > pTileLayer->m_Width)
		X1 = pTileLayer->m_Width;
	if(Y1 > pTileLayer->m_Height)
		Y1 = pTileLayer->m_Height;

	// corners
	if(BorderX0 < 0)
	{
		// Draw corners on left side
		if(BorderY0 < 0)
		{
			if(Visuals.m_BorderTopLeft.DoDraw())
			{
				vec2 Offset;
				Offset.x = 0;
				Offset.y = 0;
				vec2 Scale;
				Scale.x = absolute(BorderX0);
				Scale.y = absolute(BorderY0);

				Graphics()->RenderBorderTiles(Visuals.m_BufferContainerIndex, Color, (offset_ptr_size)Visuals.m_BorderTopLeft.IndexBufferByteOffset(), Offset, Scale, 1);
			}
		}
		if(BorderY1 > pTileLayer->m_Height)
		{
			if(Visuals.m_BorderBottomLeft.DoDraw())
			{
				vec2 Offset;
				Offset.x = 0;
				Offset.y = pTileLayer->m_Height * 32.0f;
				vec2 Scale;
				Scale.x = absolute(BorderX0);
				Scale.y = BorderY1 - pTileLayer->m_Height;

				Graphics()->RenderBorderTiles(Visuals.m_BufferContainerIndex, Color, (offset_ptr_size)Visuals.m_BorderBottomLeft.IndexBufferByteOffset(), Offset, Scale, 1);
			}
		}
	}
	if(BorderX1 > pTileLayer->m_Width)
	{
		// Draw corners on right side
		if(BorderY0 < 0)
		{
			if(Visuals.m_BorderTopRight.DoDraw())
			{
				vec2 Offset;
				Offset.x = pTileLayer->m_Width * 32.0f;
				Offset.y = 0;
				vec2 Scale;
				Scale.x = BorderX1 - pTileLayer->m_Width;
				Scale.y = absolute(BorderY0);

				Graphics()->RenderBorderTiles(Visuals.m_BufferContainerIndex, Color, (offset_ptr_size)Visuals.m_BorderTopRight.IndexBufferByteOffset(), Offset, Scale, 1);
			}
		}
		if(BorderY1 > pTileLayer->m_Height)
		{
			if(Visuals.m_BorderBottomRight.DoDraw())
			{
				vec2 Offset;
				Offset.x = pTileLayer->m_Width * 32.0f;
				Offset.y = pTileLayer->m_Height * 32.0f;
				vec2 Scale;
				Scale.x = BorderX1 - pTileLayer->m_Width;
				Scale.y = BorderY1 - pTileLayer->m_Height;

				Graphics()->RenderBorderTiles(Visuals.m_BufferContainerIndex, Color, (offset_ptr_size)Visuals.m_BorderBottomRight.IndexBufferByteOffset(), Offset, Scale, 1);
			}
		}
	}

	if(BorderX1 > pTileLayer->m_Width)
	{
		// Draw right border
		if(Y0 < pTileLayer->m_Height && Y1 > 0)
		{
			int YB = Y1 - 1;
			unsigned int DrawNum = ((Visuals.m_vBorderRight[YB].IndexBufferByteOffset() - Visuals.m_vBorderRight[Y0].IndexBufferByteOffset()) / (sizeof(unsigned int) * 6)) + (Visuals.m_vBorderRight[YB].DoDraw() ? 1lu : 0lu);
			offset_ptr_size pOffset = (offset_ptr_size)Visuals.m_vBorderRight[Y0].IndexBufferByteOffset();
			vec2 Offset;
			Offset.x = 32.f * pTileLayer->m_Width;
			Offset.y = 0.f;
			vec2 Scale;
			Scale.x = BorderX1 - pTileLayer->m_Width;
			Scale.y = 1.f;
			Graphics()->RenderBorderTiles(Visuals.m_BufferContainerIndex, Color, pOffset, Offset, Scale, DrawNum);
		}
	}
	if(BorderX0 < 0)
	{
		// Draw left border
		if(Y0 < pTileLayer->m_Height && Y1 > 0)
		{
			int YB = Y1 - 1;
			unsigned int DrawNum = ((Visuals.m_vBorderLeft[YB].IndexBufferByteOffset() - Visuals.m_vBorderLeft[Y0].IndexBufferByteOffset()) / (sizeof(unsigned int) * 6)) + (Visuals.m_vBorderLeft[YB].DoDraw() ? 1lu : 0lu);
			offset_ptr_size pOffset = (offset_ptr_size)Visuals.m_vBorderLeft[Y0].IndexBufferByteOffset();
			vec2 Offset;
			Offset.x = 0;
			Offset.y = 0;
			vec2 Scale;
			Scale.x = absolute(BorderX0);
			Scale.y = 1;
			Graphics()->RenderBorderTiles(Visuals.m_BufferContainerIndex, Color, pOffset, Offset, Scale, DrawNum);
		}
	}
	if(BorderY0 < 0)
	{
		// Draw top border
		if(X0 < pTileLayer->m_Width && X1 > 0)
		{
			int XR = X1 - 1;
			unsigned int DrawNum = ((Visuals.m_vBorderTop[XR].IndexBufferByteOffset() - Visuals.m_vBorderTop[X0].IndexBufferByteOffset()) / (sizeof(unsigned int) * 6)) + (Visuals.m_vBorderTop[XR].DoDraw() ? 1lu : 0lu);
			offset_ptr_size pOffset = (offset_ptr_size)Visuals.m_vBorderTop[X0].IndexBufferByteOffset();
			vec2 Offset;
			Offset.x = 0.f;
			Offset.y = 0;
			vec2 Scale;
			Scale.x = 1;
			Scale.y = absolute(BorderY0);
			Graphics()->RenderBorderTiles(Visuals.m_BufferContainerIndex, Color, pOffset, Offset, Scale, DrawNum);
		}
	}
	if(BorderY1 > pTileLayer->m_Height)
	{
		// Draw bottom border
		if(X0 < pTileLayer->m_Width && X1 > 0)
		{
			int XR = X1 - 1;
			unsigned int DrawNum = ((Visuals.m_vBorderBottom[XR].IndexBufferByteOffset() - Visuals.m_vBorderBottom[X0].IndexBufferByteOffset()) / (sizeof(unsigned int) * 6)) + (Visuals.m_vBorderBottom[XR].DoDraw() ? 1lu : 0lu);
			offset_ptr_size pOffset = (offset_ptr_size)Visuals.m_vBorderBottom[X0].IndexBufferByteOffset();
			vec2 Offset;
			Offset.x = 0.f;
			Offset.y = 32.f * pTileLayer->m_Height;
			vec2 Scale;
			Scale.x = 1;
			Scale.y = BorderY1 - pTileLayer->m_Height;
			Graphics()->RenderBorderTiles(Visuals.m_BufferContainerIndex, Color, pOffset, Offset, Scale, DrawNum);
		}
	}
}

void CMapLayers::RenderKillTileBorder(int LayerIndex, const ColorRGBA &Color, CMapItemLayerTilemap *pTileLayer, CMapItemGroup *pGroup)
{
	STileLayerVisuals &Visuals = *m_vpTileLayerVisuals[LayerIndex];
	if(Visuals.m_BufferContainerIndex == -1)
		return; //no visuals were created

	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);

	bool DrawBorder = false;

	int BorderY0 = std::floor(ScreenY0 / 32);
	int BorderX0 = std::floor(ScreenX0 / 32);
	int BorderY1 = std::ceil(ScreenY1 / 32);
	int BorderX1 = std::ceil(ScreenX1 / 32);

	if(BorderX0 < -201)
		DrawBorder = true;
	if(BorderY0 < -201)
		DrawBorder = true;
	if(BorderX1 > pTileLayer->m_Width + 201)
		DrawBorder = true;
	if(BorderY1 > pTileLayer->m_Height + 201)
		DrawBorder = true;

	if(!DrawBorder)
		return;
	if(!Visuals.m_BorderKillTile.DoDraw())
		return;

	if(BorderX0 < -300)
		BorderX0 = -300;
	if(BorderY0 < -300)
		BorderY0 = -300;
	if(BorderX1 >= pTileLayer->m_Width + 300)
		BorderX1 = pTileLayer->m_Width + 299;
	if(BorderY1 >= pTileLayer->m_Height + 300)
		BorderY1 = pTileLayer->m_Height + 299;

	if(BorderX1 < -300)
		BorderX1 = -300;
	if(BorderY1 < -300)
		BorderY1 = -300;
	if(BorderX0 >= pTileLayer->m_Width + 300)
		BorderX0 = pTileLayer->m_Width + 299;
	if(BorderY0 >= pTileLayer->m_Height + 300)
		BorderY0 = pTileLayer->m_Height + 299;

	// Draw left kill tile border
	if(BorderX0 < -201)
	{
		unsigned int DrawNum = 1;
		offset_ptr_size pOffset = (offset_ptr_size)Visuals.m_BorderKillTile.IndexBufferByteOffset();
		vec2 Offset;
		Offset.x = 32.f * BorderX0;
		Offset.y = 32.f * BorderY0;
		vec2 Scale;
		Scale.x = -201 - BorderX0;
		Scale.y = BorderY1 - BorderY0;
		Graphics()->RenderBorderTiles(Visuals.m_BufferContainerIndex, Color, pOffset, Offset, Scale, DrawNum);
	}
	// Draw top kill tile border
	if(BorderY0 < -201)
	{
		unsigned int DrawNum = 1;
		offset_ptr_size pOffset = (offset_ptr_size)Visuals.m_BorderKillTile.IndexBufferByteOffset();
		vec2 Offset;
		Offset.x = maximum(BorderX0, -201) * 32.0f;
		Offset.y = 32.f * BorderY0;
		vec2 Scale;
		Scale.x = minimum(BorderX1, pTileLayer->m_Width + 201) - maximum(BorderX0, -201);
		Scale.y = -201 - BorderY0;
		Graphics()->RenderBorderTiles(Visuals.m_BufferContainerIndex, Color, pOffset, Offset, Scale, DrawNum);
	}
	// Draw right kill tile border
	if(BorderX1 > pTileLayer->m_Width + 201)
	{
		unsigned int DrawNum = 1;
		offset_ptr_size pOffset = (offset_ptr_size)Visuals.m_BorderKillTile.IndexBufferByteOffset();
		vec2 Offset;
		Offset.x = 32.0f * (pTileLayer->m_Width + 201);
		Offset.y = 32.0f * BorderY0;
		vec2 Scale;
		Scale.x = BorderX1 - (pTileLayer->m_Width + 201);
		Scale.y = BorderY1 - BorderY0;
		Graphics()->RenderBorderTiles(Visuals.m_BufferContainerIndex, Color, pOffset, Offset, Scale, DrawNum);
	}
	// Draw bottom kill tile border
	if(BorderY1 > pTileLayer->m_Height + 201)
	{
		unsigned int DrawNum = 1;
		offset_ptr_size pOffset = (offset_ptr_size)Visuals.m_BorderKillTile.IndexBufferByteOffset();
		vec2 Offset;
		Offset.x = maximum(BorderX0, -201) * 32.0f;
		Offset.y = 32.0f * (pTileLayer->m_Height + 201);
		vec2 Scale;
		Scale.x = minimum(BorderX1, pTileLayer->m_Width + 201) - maximum(BorderX0, -201);
		Scale.y = BorderY1 - (pTileLayer->m_Height + 201);
		Graphics()->RenderBorderTiles(Visuals.m_BufferContainerIndex, Color, pOffset, Offset, Scale, DrawNum);
	}
}

void CMapLayers::RenderQuadLayer(int LayerIndex, CMapItemLayerQuads *pQuadLayer, CMapItemGroup *pGroup, bool Force)
{
	SQuadLayerVisuals &Visuals = *m_vpQuadLayerVisuals[LayerIndex];
	if(Visuals.m_BufferContainerIndex == -1)
		return; //no visuals were created

	if(!Force && (!g_Config.m_ClShowQuads || g_Config.m_ClOverlayEntities == 100))
		return;

	CQuad *pQuads = (CQuad *)m_pLayers->Map()->GetDataSwapped(pQuadLayer->m_Data);

	static std::vector<SQuadRenderInfo> s_vQuadRenderInfo;

	s_vQuadRenderInfo.resize(pQuadLayer->m_NumQuads);
	size_t QuadsRenderCount = 0;
	size_t CurQuadOffset = 0;
	for(int i = 0; i < pQuadLayer->m_NumQuads; ++i)
	{
		CQuad *pQuad = &pQuads[i];

		ColorRGBA Color(1.f, 1.f, 1.f, 1.f);
		if(pQuad->m_ColorEnv >= 0)
		{
			EnvelopeEval(pQuad->m_ColorEnvOffset, pQuad->m_ColorEnv, Color, this);
		}

		float OffsetX = 0;
		float OffsetY = 0;
		float Rot = 0;

		if(pQuad->m_PosEnv >= 0)
		{
			ColorRGBA Channels;
			EnvelopeEval(pQuad->m_PosEnvOffset, pQuad->m_PosEnv, Channels, this);
			OffsetX = Channels.r;
			OffsetY = Channels.g;
			Rot = Channels.b / 180.0f * pi;
		}

		const bool IsFullyTransparent = Color.a <= 0;
		bool NeedsFlush = QuadsRenderCount == gs_GraphicsMaxQuadsRenderCount || IsFullyTransparent;

		if(NeedsFlush)
		{
			// render quads of the current offset directly(cancel batching)
			Graphics()->RenderQuadLayer(Visuals.m_BufferContainerIndex, s_vQuadRenderInfo.data(), QuadsRenderCount, CurQuadOffset);
			QuadsRenderCount = 0;
			CurQuadOffset = i;
			if(IsFullyTransparent)
			{
				// since this quad is ignored, the offset is the next quad
				++CurQuadOffset;
			}
		}

		if(!IsFullyTransparent)
		{
			SQuadRenderInfo &QInfo = s_vQuadRenderInfo[QuadsRenderCount++];
			QInfo.m_Color = Color;
			QInfo.m_Offsets.x = OffsetX;
			QInfo.m_Offsets.y = OffsetY;
			QInfo.m_Rotation = Rot;
		}
	}
	Graphics()->RenderQuadLayer(Visuals.m_BufferContainerIndex, s_vQuadRenderInfo.data(), QuadsRenderCount, CurQuadOffset);
}

void CMapLayers::LayersOfGroupCount(CMapItemGroup *pGroup, int &TileLayerCount, int &QuadLayerCount, bool &PassedGameLayer)
{
	int TileLayerCounter = 0;
	int QuadLayerCounter = 0;
	for(int l = 0; l < pGroup->m_NumLayers; l++)
	{
		CMapItemLayer *pLayer = m_pLayers->GetLayer(pGroup->m_StartLayer + l);
		bool IsFrontLayer = false;
		bool IsSwitchLayer = false;
		bool IsTeleLayer = false;
		bool IsSpeedupLayer = false;
		bool IsTuneLayer = false;

		if(pLayer == (CMapItemLayer *)m_pLayers->GameLayer())
		{
			PassedGameLayer = true;
		}

		if(pLayer == (CMapItemLayer *)m_pLayers->FrontLayer())
			IsFrontLayer = true;

		if(pLayer == (CMapItemLayer *)m_pLayers->SwitchLayer())
			IsSwitchLayer = true;

		if(pLayer == (CMapItemLayer *)m_pLayers->TeleLayer())
			IsTeleLayer = true;

		if(pLayer == (CMapItemLayer *)m_pLayers->SpeedupLayer())
			IsSpeedupLayer = true;

		if(pLayer == (CMapItemLayer *)m_pLayers->TuneLayer())
			IsTuneLayer = true;

		if(m_Type <= TYPE_BACKGROUND_FORCE)
		{
			if(PassedGameLayer)
				break;
		}
		else if(m_Type == TYPE_FOREGROUND)
		{
			if(!PassedGameLayer)
				continue;
		}

		if(pLayer->m_Type == LAYERTYPE_TILES)
		{
			CMapItemLayerTilemap *pTMap = (CMapItemLayerTilemap *)pLayer;
			int DataIndex = 0;
			unsigned int TileSize = 0;
			int TileLayerAndOverlayCount = 0;
			if(IsFrontLayer)
			{
				DataIndex = pTMap->m_Front;
				TileSize = sizeof(CTile);
				TileLayerAndOverlayCount = 1;
			}
			else if(IsSwitchLayer)
			{
				DataIndex = pTMap->m_Switch;
				TileSize = sizeof(CSwitchTile);
				TileLayerAndOverlayCount = 3;
			}
			else if(IsTeleLayer)
			{
				DataIndex = pTMap->m_Tele;
				TileSize = sizeof(CTeleTile);
				TileLayerAndOverlayCount = 2;
			}
			else if(IsSpeedupLayer)
			{
				DataIndex = pTMap->m_Speedup;
				TileSize = sizeof(CSpeedupTile);
				TileLayerAndOverlayCount = 3;
			}
			else if(IsTuneLayer)
			{
				DataIndex = pTMap->m_Tune;
				TileSize = sizeof(CTuneTile);
				TileLayerAndOverlayCount = 1;
			}
			else
			{
				DataIndex = pTMap->m_Data;
				TileSize = sizeof(CTile);
				TileLayerAndOverlayCount = 1;
			}

			unsigned int Size = m_pLayers->Map()->GetDataSize(DataIndex);
			if(Size >= pTMap->m_Width * pTMap->m_Height * TileSize)
			{
				TileLayerCounter += TileLayerAndOverlayCount;
			}
		}
		else if(pLayer->m_Type == LAYERTYPE_QUADS)
		{
			++QuadLayerCounter;
		}
	}

	TileLayerCount += TileLayerCounter;
	QuadLayerCount += QuadLayerCounter;
}

void CMapLayers::OnRender()
{
	if(m_OnlineOnly && Client()->State() != IClient::STATE_ONLINE && Client()->State() != IClient::STATE_DEMOPLAYBACK)
		return;

	CUIRect Screen;
	Graphics()->GetScreen(&Screen.x, &Screen.y, &Screen.w, &Screen.h);

	vec2 Center = GetCurCamera()->m_Center;

	bool PassedGameLayer = false;
	int TileLayerCounter = 0;
	int QuadLayerCounter = 0;

	for(int g = 0; g < m_pLayers->NumGroups(); g++)
	{
		CMapItemGroup *pGroup = m_pLayers->GetGroup(g);

		if(!pGroup)
		{
			dbg_msg("maplayers", "error group was null, group number = %d, total groups = %d", g, m_pLayers->NumGroups());
			dbg_msg("maplayers", "this is here to prevent a crash but the source of this is unknown, please report this for it to get fixed");
			dbg_msg("maplayers", "we need mapname and crc and the map that caused this if possible, and anymore info you think is relevant");
			continue;
		}

		if((!g_Config.m_GfxNoclip || m_Type == TYPE_FULL_DESIGN) && pGroup->m_Version >= 2 && pGroup->m_UseClipping)
		{
			// set clipping
			float aPoints[4];
			RenderTools()->MapScreenToGroup(Center.x, Center.y, m_pLayers->GameGroup(), GetCurCamera()->m_Zoom);
			Graphics()->GetScreen(&aPoints[0], &aPoints[1], &aPoints[2], &aPoints[3]);
			float x0 = (pGroup->m_ClipX - aPoints[0]) / (aPoints[2] - aPoints[0]);
			float y0 = (pGroup->m_ClipY - aPoints[1]) / (aPoints[3] - aPoints[1]);
			float x1 = ((pGroup->m_ClipX + pGroup->m_ClipW) - aPoints[0]) / (aPoints[2] - aPoints[0]);
			float y1 = ((pGroup->m_ClipY + pGroup->m_ClipH) - aPoints[1]) / (aPoints[3] - aPoints[1]);

			if(x1 < 0.0f || x0 > 1.0f || y1 < 0.0f || y0 > 1.0f)
			{
				//check tile layer count of this group
				LayersOfGroupCount(pGroup, TileLayerCounter, QuadLayerCounter, PassedGameLayer);
				continue;
			}

			Graphics()->ClipEnable((int)(x0 * Graphics()->ScreenWidth()), (int)(y0 * Graphics()->ScreenHeight()),
				(int)((x1 - x0) * Graphics()->ScreenWidth()), (int)((y1 - y0) * Graphics()->ScreenHeight()));
		}

		RenderTools()->MapScreenToGroup(Center.x, Center.y, pGroup, GetCurCamera()->m_Zoom);

		for(int l = 0; l < pGroup->m_NumLayers; l++)
		{
			CMapItemLayer *pLayer = m_pLayers->GetLayer(pGroup->m_StartLayer + l);
			bool Render = false;
			bool IsGameLayer = false;
			bool IsFrontLayer = false;
			bool IsSwitchLayer = false;
			bool IsTeleLayer = false;
			bool IsSpeedupLayer = false;
			bool IsTuneLayer = false;
			bool IsEntityLayer = false;

			if(pLayer == (CMapItemLayer *)m_pLayers->GameLayer())
			{
				IsEntityLayer = IsGameLayer = true;
				PassedGameLayer = true;
			}

			if(pLayer == (CMapItemLayer *)m_pLayers->FrontLayer())
				IsEntityLayer = IsFrontLayer = true;

			if(pLayer == (CMapItemLayer *)m_pLayers->SwitchLayer())
				IsEntityLayer = IsSwitchLayer = true;

			if(pLayer == (CMapItemLayer *)m_pLayers->TeleLayer())
				IsEntityLayer = IsTeleLayer = true;

			if(pLayer == (CMapItemLayer *)m_pLayers->SpeedupLayer())
				IsEntityLayer = IsSpeedupLayer = true;

			if(pLayer == (CMapItemLayer *)m_pLayers->TuneLayer())
				IsEntityLayer = IsTuneLayer = true;

			if(m_Type == -1)
				Render = true;
			else if(m_Type <= TYPE_BACKGROUND_FORCE)
			{
				if(PassedGameLayer)
					return;
				Render = true;

				if(m_Type == TYPE_BACKGROUND_FORCE)
				{
					if(pLayer->m_Type == LAYERTYPE_TILES && !g_Config.m_ClBackgroundShowTilesLayers)
						continue;
				}
			}
			else if(m_Type == TYPE_FOREGROUND)
			{
				if(PassedGameLayer && !IsGameLayer)
					Render = true;
			}
			else if(m_Type == TYPE_FULL_DESIGN)
			{
				if(!IsGameLayer)
					Render = true;
			}

			if(Render && pLayer->m_Type == LAYERTYPE_TILES && Input()->ModifierIsPressed() && Input()->ShiftIsPressed() && Input()->KeyPress(KEY_KP_0))
			{
				CMapItemLayerTilemap *pTMap = (CMapItemLayerTilemap *)pLayer;
				CTile *pTiles = (CTile *)m_pLayers->Map()->GetData(pTMap->m_Data);
				CServerInfo CurrentServerInfo;
				Client()->GetServerInfo(&CurrentServerInfo);
				char aFilename[IO_MAX_PATH_LENGTH];
				str_format(aFilename, sizeof(aFilename), "dumps/tilelayer_dump_%s-%d-%d-%dx%d.txt", CurrentServerInfo.m_aMap, g, l, pTMap->m_Width, pTMap->m_Height);
				IOHANDLE File = Storage()->OpenFile(aFilename, IOFLAG_WRITE, IStorage::TYPE_SAVE);
				if(File)
				{
					for(int y = 0; y < pTMap->m_Height; y++)
					{
						for(int x = 0; x < pTMap->m_Width; x++)
							io_write(File, &(pTiles[y * pTMap->m_Width + x].m_Index), sizeof(pTiles[y * pTMap->m_Width + x].m_Index));
						io_write_newline(File);
					}
					io_close(File);
				}
			}

			if((Render || IsGameLayer) && pLayer->m_Type == LAYERTYPE_TILES)
			{
				CMapItemLayerTilemap *pTMap = (CMapItemLayerTilemap *)pLayer;
				int DataIndex = 0;
				unsigned int TileSize = 0;
				int TileLayerAndOverlayCount = 0;
				if(IsFrontLayer)
				{
					DataIndex = pTMap->m_Front;
					TileSize = sizeof(CTile);
					TileLayerAndOverlayCount = 1;
				}
				else if(IsSwitchLayer)
				{
					DataIndex = pTMap->m_Switch;
					TileSize = sizeof(CSwitchTile);
					TileLayerAndOverlayCount = 3;
				}
				else if(IsTeleLayer)
				{
					DataIndex = pTMap->m_Tele;
					TileSize = sizeof(CTeleTile);
					TileLayerAndOverlayCount = 2;
				}
				else if(IsSpeedupLayer)
				{
					DataIndex = pTMap->m_Speedup;
					TileSize = sizeof(CSpeedupTile);
					TileLayerAndOverlayCount = 3;
				}
				else if(IsTuneLayer)
				{
					DataIndex = pTMap->m_Tune;
					TileSize = sizeof(CTuneTile);
					TileLayerAndOverlayCount = 1;
				}
				else
				{
					DataIndex = pTMap->m_Data;
					TileSize = sizeof(CTile);
					TileLayerAndOverlayCount = 1;
				}

				unsigned int Size = m_pLayers->Map()->GetDataSize(DataIndex);
				if(Size >= pTMap->m_Width * pTMap->m_Height * TileSize)
				{
					TileLayerCounter += TileLayerAndOverlayCount;
				}
			}
			else if(Render && pLayer->m_Type == LAYERTYPE_QUADS)
			{
				++QuadLayerCounter;
			}

			// skip rendering if detail layers if not wanted, or is entity layer and we are a background map
			if((pLayer->m_Flags & LAYERFLAG_DETAIL && (!g_Config.m_GfxHighDetail && !(m_Type == TYPE_FULL_DESIGN)) && !IsGameLayer) || (m_Type == TYPE_BACKGROUND_FORCE && IsEntityLayer) || (m_Type == TYPE_FULL_DESIGN && IsEntityLayer))
				continue;

			int EntityOverlayVal = g_Config.m_ClOverlayEntities;
			if(m_Type == TYPE_FULL_DESIGN)
				EntityOverlayVal = 0;

			if((Render && EntityOverlayVal < 100 && !IsGameLayer && !IsFrontLayer && !IsSwitchLayer && !IsTeleLayer && !IsSpeedupLayer && !IsTuneLayer) || (EntityOverlayVal && IsGameLayer) || (m_Type == TYPE_BACKGROUND_FORCE))
			{
				if(pLayer->m_Type == LAYERTYPE_TILES)
				{
					CMapItemLayerTilemap *pTMap = (CMapItemLayerTilemap *)pLayer;
					if(pTMap->m_Image < 0 || pTMap->m_Image >= m_pImages->Num())
					{
						if(!IsGameLayer)
							Graphics()->TextureClear();
						else
							Graphics()->TextureSet(m_pImages->GetEntities(MAP_IMAGE_ENTITY_LAYER_TYPE_ALL_EXCEPT_SWITCH));
					}
					else
						Graphics()->TextureSet(m_pImages->Get(pTMap->m_Image));

					CTile *pTiles = (CTile *)m_pLayers->Map()->GetData(pTMap->m_Data);
					unsigned int Size = m_pLayers->Map()->GetDataSize(pTMap->m_Data);

					if(Size >= (size_t)pTMap->m_Width * pTMap->m_Height * sizeof(CTile))
					{
						ColorRGBA Color = ColorRGBA(pTMap->m_Color.r / 255.0f, pTMap->m_Color.g / 255.0f, pTMap->m_Color.b / 255.0f, pTMap->m_Color.a / 255.0f);
						if(IsGameLayer && EntityOverlayVal)
							Color = ColorRGBA(pTMap->m_Color.r / 255.0f, pTMap->m_Color.g / 255.0f, pTMap->m_Color.b / 255.0f, pTMap->m_Color.a / 255.0f * EntityOverlayVal / 100.0f);
						else if(!IsGameLayer && EntityOverlayVal && !(m_Type == TYPE_BACKGROUND_FORCE))
							Color = ColorRGBA(pTMap->m_Color.r / 255.0f, pTMap->m_Color.g / 255.0f, pTMap->m_Color.b / 255.0f, pTMap->m_Color.a / 255.0f * (100 - EntityOverlayVal) / 100.0f);
						if(!Graphics()->IsTileBufferingEnabled())
						{
							Graphics()->BlendNone();
							RenderTools()->RenderTilemap(pTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND | LAYERRENDERFLAG_OPAQUE,
								EnvelopeEval, this, pTMap->m_ColorEnv, pTMap->m_ColorEnvOffset);

							Graphics()->BlendNormal();

							// draw kill tiles outside the entity clipping rectangle
							if(IsGameLayer)
							{
								// slow blinking to hint that it's not a part of the map
								double Seconds = time_get() / (double)time_freq();
								ColorRGBA ColorHint = ColorRGBA(1.0f, 1.0f, 1.0f, 0.3 + 0.7 * (1 + std::sin(2 * (double)pi * Seconds / 3)) / 2);

								RenderTools()->RenderTileRectangle(-201, -201, pTMap->m_Width + 402, pTMap->m_Height + 402,
									0, TILE_DEATH, // display air inside, death outside
									32.0f, Color.v4() * ColorHint.v4(), TILERENDERFLAG_EXTEND | LAYERRENDERFLAG_TRANSPARENT,
									EnvelopeEval, this, pTMap->m_ColorEnv, pTMap->m_ColorEnvOffset);
							}

							RenderTools()->RenderTilemap(pTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND | LAYERRENDERFLAG_TRANSPARENT,
								EnvelopeEval, this, pTMap->m_ColorEnv, pTMap->m_ColorEnvOffset);
						}
						else
						{
							Graphics()->BlendNormal();
							// draw kill tiles outside the entity clipping rectangle
							if(IsGameLayer)
							{
								// slow blinking to hint that it's not a part of the map
								double Seconds = time_get() / (double)time_freq();
								ColorRGBA ColorHint = ColorRGBA(1.0f, 1.0f, 1.0f, 0.3 + 0.7 * (1.0 + std::sin(2 * (double)pi * Seconds / 3)) / 2);

								ColorRGBA ColorKill(Color.x * ColorHint.x, Color.y * ColorHint.y, Color.z * ColorHint.z, Color.w * ColorHint.w);
								RenderKillTileBorder(TileLayerCounter - 1, ColorKill, pTMap, pGroup);
							}
							RenderTileLayer(TileLayerCounter - 1, Color, pTMap, pGroup);
						}
					}
				}
				else if(pLayer->m_Type == LAYERTYPE_QUADS)
				{
					CMapItemLayerQuads *pQLayer = (CMapItemLayerQuads *)pLayer;
					if(pQLayer->m_Image < 0 || pQLayer->m_Image >= m_pImages->Num())
						Graphics()->TextureClear();
					else
						Graphics()->TextureSet(m_pImages->Get(pQLayer->m_Image));

					CQuad *pQuads = (CQuad *)m_pLayers->Map()->GetDataSwapped(pQLayer->m_Data);
					if(m_Type == TYPE_BACKGROUND_FORCE || m_Type == TYPE_FULL_DESIGN)
					{
						if(g_Config.m_ClShowQuads || m_Type == TYPE_FULL_DESIGN)
						{
							if(!Graphics()->IsQuadBufferingEnabled())
							{
								//Graphics()->BlendNone();
								//RenderTools()->ForceRenderQuads(pQuads, pQLayer->m_NumQuads, LAYERRENDERFLAG_OPAQUE, EnvelopeEval, this, 1.f);
								Graphics()->BlendNormal();
								RenderTools()->ForceRenderQuads(pQuads, pQLayer->m_NumQuads, LAYERRENDERFLAG_TRANSPARENT, EnvelopeEval, this, 1.f);
							}
							else
							{
								RenderQuadLayer(QuadLayerCounter - 1, pQLayer, pGroup, true);
							}
						}
					}
					else
					{
						if(!Graphics()->IsQuadBufferingEnabled())
						{
							//Graphics()->BlendNone();
							//RenderTools()->RenderQuads(pQuads, pQLayer->m_NumQuads, LAYERRENDERFLAG_OPAQUE, EnvelopeEval, this);
							Graphics()->BlendNormal();
							RenderTools()->RenderQuads(pQuads, pQLayer->m_NumQuads, LAYERRENDERFLAG_TRANSPARENT, EnvelopeEval, this);
						}
						else
						{
							RenderQuadLayer(QuadLayerCounter - 1, pQLayer, pGroup, false);
						}
					}
				}
			}
			else if(Render && EntityOverlayVal && IsFrontLayer)
			{
				CMapItemLayerTilemap *pTMap = (CMapItemLayerTilemap *)pLayer;
				Graphics()->TextureSet(m_pImages->GetEntities(MAP_IMAGE_ENTITY_LAYER_TYPE_ALL_EXCEPT_SWITCH));

				CTile *pFrontTiles = (CTile *)m_pLayers->Map()->GetData(pTMap->m_Front);
				unsigned int Size = m_pLayers->Map()->GetDataSize(pTMap->m_Front);

				if(Size >= (size_t)pTMap->m_Width * pTMap->m_Height * sizeof(CTile))
				{
					ColorRGBA Color = ColorRGBA(pTMap->m_Color.r / 255.0f, pTMap->m_Color.g / 255.0f, pTMap->m_Color.b / 255.0f, pTMap->m_Color.a / 255.0f * EntityOverlayVal / 100.0f);
					if(!Graphics()->IsTileBufferingEnabled())
					{
						Graphics()->BlendNone();
						RenderTools()->RenderTilemap(pFrontTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND | LAYERRENDERFLAG_OPAQUE,
							EnvelopeEval, this, pTMap->m_ColorEnv, pTMap->m_ColorEnvOffset);
						Graphics()->BlendNormal();
						RenderTools()->RenderTilemap(pFrontTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND | LAYERRENDERFLAG_TRANSPARENT,
							EnvelopeEval, this, pTMap->m_ColorEnv, pTMap->m_ColorEnvOffset);
					}
					else
					{
						Graphics()->BlendNormal();
						RenderTileLayer(TileLayerCounter - 1, Color, pTMap, pGroup);
					}
				}
			}
			else if(Render && EntityOverlayVal && IsSwitchLayer)
			{
				CMapItemLayerTilemap *pTMap = (CMapItemLayerTilemap *)pLayer;
				Graphics()->TextureSet(m_pImages->GetEntities(MAP_IMAGE_ENTITY_LAYER_TYPE_SWITCH));

				CSwitchTile *pSwitchTiles = (CSwitchTile *)m_pLayers->Map()->GetData(pTMap->m_Switch);
				unsigned int Size = m_pLayers->Map()->GetDataSize(pTMap->m_Switch);

				if(Size >= (size_t)pTMap->m_Width * pTMap->m_Height * sizeof(CSwitchTile))
				{
					ColorRGBA Color = ColorRGBA(pTMap->m_Color.r / 255.0f, pTMap->m_Color.g / 255.0f, pTMap->m_Color.b / 255.0f, pTMap->m_Color.a / 255.0f * EntityOverlayVal / 100.0f);
					if(!Graphics()->IsTileBufferingEnabled())
					{
						Graphics()->BlendNone();
						RenderTools()->RenderSwitchmap(pSwitchTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND | LAYERRENDERFLAG_OPAQUE);
						Graphics()->BlendNormal();
						RenderTools()->RenderSwitchmap(pSwitchTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND | LAYERRENDERFLAG_TRANSPARENT);
						RenderTools()->RenderSwitchOverlay(pSwitchTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, EntityOverlayVal / 100.0f);
					}
					else
					{
						Graphics()->BlendNormal();
						RenderTileLayer(TileLayerCounter - 3, Color, pTMap, pGroup);
						if(g_Config.m_ClTextEntities)
						{
							Graphics()->TextureSet(m_pImages->GetOverlayBottom());
							RenderTileLayer(TileLayerCounter - 2, Color, pTMap, pGroup);
							Graphics()->TextureSet(m_pImages->GetOverlayTop());
							RenderTileLayer(TileLayerCounter - 1, Color, pTMap, pGroup);
						}
					}
				}
			}
			else if(Render && EntityOverlayVal && IsTeleLayer)
			{
				CMapItemLayerTilemap *pTMap = (CMapItemLayerTilemap *)pLayer;
				Graphics()->TextureSet(m_pImages->GetEntities(MAP_IMAGE_ENTITY_LAYER_TYPE_ALL_EXCEPT_SWITCH));

				CTeleTile *pTeleTiles = (CTeleTile *)m_pLayers->Map()->GetData(pTMap->m_Tele);
				unsigned int Size = m_pLayers->Map()->GetDataSize(pTMap->m_Tele);

				if(Size >= (size_t)pTMap->m_Width * pTMap->m_Height * sizeof(CTeleTile))
				{
					ColorRGBA Color = ColorRGBA(pTMap->m_Color.r / 255.0f, pTMap->m_Color.g / 255.0f, pTMap->m_Color.b / 255.0f, pTMap->m_Color.a / 255.0f * EntityOverlayVal / 100.0f);
					if(!Graphics()->IsTileBufferingEnabled())
					{
						Graphics()->BlendNone();
						RenderTools()->RenderTelemap(pTeleTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND | LAYERRENDERFLAG_OPAQUE);
						Graphics()->BlendNormal();
						RenderTools()->RenderTelemap(pTeleTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND | LAYERRENDERFLAG_TRANSPARENT);
						RenderTools()->RenderTeleOverlay(pTeleTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, EntityOverlayVal / 100.0f);
					}
					else
					{
						Graphics()->BlendNormal();
						RenderTileLayer(TileLayerCounter - 2, Color, pTMap, pGroup);
						if(g_Config.m_ClTextEntities)
						{
							Graphics()->TextureSet(m_pImages->GetOverlayCenter());
							RenderTileLayer(TileLayerCounter - 1, Color, pTMap, pGroup);
						}
					}
				}
			}
			else if(Render && EntityOverlayVal && IsSpeedupLayer)
			{
				CMapItemLayerTilemap *pTMap = (CMapItemLayerTilemap *)pLayer;
				Graphics()->TextureSet(m_pImages->GetEntities(MAP_IMAGE_ENTITY_LAYER_TYPE_ALL_EXCEPT_SWITCH));

				CSpeedupTile *pSpeedupTiles = (CSpeedupTile *)m_pLayers->Map()->GetData(pTMap->m_Speedup);
				unsigned int Size = m_pLayers->Map()->GetDataSize(pTMap->m_Speedup);

				if(Size >= (size_t)pTMap->m_Width * pTMap->m_Height * sizeof(CSpeedupTile))
				{
					ColorRGBA Color = ColorRGBA(pTMap->m_Color.r / 255.0f, pTMap->m_Color.g / 255.0f, pTMap->m_Color.b / 255.0f, pTMap->m_Color.a / 255.0f * EntityOverlayVal / 100.0f);
					if(!Graphics()->IsTileBufferingEnabled())
					{
						Graphics()->BlendNone();
						RenderTools()->RenderSpeedupmap(pSpeedupTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND | LAYERRENDERFLAG_OPAQUE);
						Graphics()->BlendNormal();
						RenderTools()->RenderSpeedupmap(pSpeedupTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND | LAYERRENDERFLAG_TRANSPARENT);
						RenderTools()->RenderSpeedupOverlay(pSpeedupTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, EntityOverlayVal / 100.0f);
					}
					else
					{
						Graphics()->BlendNormal();

						// draw arrow -- clamp to the edge of the arrow image
						Graphics()->WrapClamp();
						Graphics()->TextureSet(m_pImages->GetSpeedupArrow());
						RenderTileLayer(TileLayerCounter - 3, Color, pTMap, pGroup);
						Graphics()->WrapNormal();

						if(g_Config.m_ClTextEntities)
						{
							Graphics()->TextureSet(m_pImages->GetOverlayBottom());
							RenderTileLayer(TileLayerCounter - 2, Color, pTMap, pGroup);
							Graphics()->TextureSet(m_pImages->GetOverlayTop());
							RenderTileLayer(TileLayerCounter - 1, Color, pTMap, pGroup);
						}
					}
				}
			}
			else if(Render && EntityOverlayVal && IsTuneLayer)
			{
				CMapItemLayerTilemap *pTMap = (CMapItemLayerTilemap *)pLayer;
				Graphics()->TextureSet(m_pImages->GetEntities(MAP_IMAGE_ENTITY_LAYER_TYPE_ALL_EXCEPT_SWITCH));

				CTuneTile *pTuneTiles = (CTuneTile *)m_pLayers->Map()->GetData(pTMap->m_Tune);
				unsigned int Size = m_pLayers->Map()->GetDataSize(pTMap->m_Tune);

				if(Size >= (size_t)pTMap->m_Width * pTMap->m_Height * sizeof(CTuneTile))
				{
					ColorRGBA Color = ColorRGBA(pTMap->m_Color.r / 255.0f, pTMap->m_Color.g / 255.0f, pTMap->m_Color.b / 255.0f, pTMap->m_Color.a / 255.0f * EntityOverlayVal / 100.0f);
					if(!Graphics()->IsTileBufferingEnabled())
					{
						Graphics()->BlendNone();
						RenderTools()->RenderTunemap(pTuneTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND | LAYERRENDERFLAG_OPAQUE);
						Graphics()->BlendNormal();
						RenderTools()->RenderTunemap(pTuneTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, Color, TILERENDERFLAG_EXTEND | LAYERRENDERFLAG_TRANSPARENT);
						//RenderTools()->RenderTuneOverlay(pTuneTiles, pTMap->m_Width, pTMap->m_Height, 32.0f, EntityOverlayVal/100.0f);
					}
					else
					{
						Graphics()->BlendNormal();
						RenderTileLayer(TileLayerCounter - 1, Color, pTMap, pGroup);
					}
				}
			}
		}
		if(!g_Config.m_GfxNoclip || m_Type == TYPE_FULL_DESIGN)
			Graphics()->ClipDisable();
	}

	if(!g_Config.m_GfxNoclip || m_Type == TYPE_FULL_DESIGN)
		Graphics()->ClipDisable();

	// reset the screen like it was before
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);
}
