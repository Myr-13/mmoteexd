/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_MAPITEMS_H
#define GAME_MAPITEMS_H

#include <base/vmath.h>

// layer types
enum
{
	LAYERTYPE_INVALID = 0,
	LAYERTYPE_GAME, // unused
	LAYERTYPE_TILES,
	LAYERTYPE_QUADS,
	LAYERTYPE_FRONT,
	LAYERTYPE_TELE,
	LAYERTYPE_SPEEDUP,
	LAYERTYPE_SWITCH,
	LAYERTYPE_TUNE,
	LAYERTYPE_SOUNDS_DEPRECATED, // deprecated! do not use this, this is just for compatibility reasons
	LAYERTYPE_SOUNDS,

	MAPITEMTYPE_VERSION = 0,
	MAPITEMTYPE_INFO,
	MAPITEMTYPE_IMAGE,
	MAPITEMTYPE_ENVELOPE,
	MAPITEMTYPE_GROUP,
	MAPITEMTYPE_LAYER,
	MAPITEMTYPE_ENVPOINTS,
	MAPITEMTYPE_SOUND,
	// High map item type numbers suggest that they use the alternate
	// format with UUIDs. See src/engine/shared/datafile.cpp for some of
	// the implementation.

	CURVETYPE_STEP = 0,
	CURVETYPE_LINEAR,
	CURVETYPE_SLOW,
	CURVETYPE_FAST,
	CURVETYPE_SMOOTH,
	CURVETYPE_BEZIER,
	NUM_CURVETYPES,

	// game layer tiles
	// TODO define which Layer uses which tiles (needed for mapeditor)
	ENTITY_NULL = 0,
	ENTITY_SPAWN,
	ENTITY_SPAWN_RED,
	ENTITY_SPAWN_BLUE,
	ENTITY_FLAGSTAND_RED,
	ENTITY_FLAGSTAND_BLUE,

	// End Of Lower Tiles
	NUM_ENTITIES,

	// Start From Top Left
	// Tile Controllers
	TILE_AIR = 0,
	TILE_SOLID,
	TILE_DEATH,
	TILE_NOHOOK,

	TILE_STOP = 60,
	TILE_STOPS,
	TILE_STOPA,

	TILE_ENTITIES_OFF_1 = 190,
	TILE_ENTITIES_OFF_2,
	//End of higher tiles

	// Layers
	LAYER_GAME = 0,
	LAYER_FRONT,
	LAYER_TELE,
	LAYER_SPEEDUP,
	LAYER_SWITCH,
	LAYER_TUNE,
	NUM_LAYERS,
	// Flags
	TILEFLAG_XFLIP = 1,
	TILEFLAG_YFLIP = 2,
	TILEFLAG_OPAQUE = 4,
	TILEFLAG_ROTATE = 8,
	// Rotation
	ROTATION_0 = 0,
	ROTATION_90 = TILEFLAG_ROTATE,
	ROTATION_180 = (TILEFLAG_XFLIP | TILEFLAG_YFLIP),
	ROTATION_270 = (TILEFLAG_XFLIP | TILEFLAG_YFLIP | TILEFLAG_ROTATE),

	LAYERFLAG_DETAIL = 1,
	TILESLAYERFLAG_GAME = 1,
	TILESLAYERFLAG_TELE = 2,
	TILESLAYERFLAG_SPEEDUP = 4,
	TILESLAYERFLAG_FRONT = 8,
	TILESLAYERFLAG_SWITCH = 16,
	TILESLAYERFLAG_TUNE = 32,

	ENTITY_OFFSET = 255 - 16 * 4,
};

static constexpr size_t MAX_MAPIMAGES = 64;
static constexpr size_t MAX_MAPSOUNDS = 64;

typedef ivec2 CPoint; // 22.10 fixed point
typedef ivec4 CColor;

struct CQuad
{
	CPoint m_aPoints[5];
	CColor m_aColors[4];
	CPoint m_aTexcoords[4];

	int m_PosEnv;
	int m_PosEnvOffset;

	int m_ColorEnv;
	int m_ColorEnvOffset;
};

class CTile
{
public:
	unsigned char m_Index;
	unsigned char m_Flags;
	unsigned char m_Skip;
	unsigned char m_Reserved;
};

struct CMapItemInfo
{
	int m_Version;
	int m_Author;
	int m_MapVersion;
	int m_Credits;
	int m_License;
};

struct CMapItemInfoSettings : CMapItemInfo
{
	int m_Settings;
};

struct CMapItemImage_v1
{
	enum
	{
		CURRENT_VERSION = 1,
	};

	int m_Version;
	int m_Width;
	int m_Height;
	int m_External;
	int m_ImageName;
	int m_ImageData;
};

struct CMapItemImage_v2 : public CMapItemImage_v1
{
	enum
	{
		CURRENT_VERSION = 2,
	};

	int m_Format; // Default before this version is CImageInfo::FORMAT_RGBA
};

typedef CMapItemImage_v1 CMapItemImage;

struct CMapItemGroup_v1
{
	int m_Version;
	int m_OffsetX;
	int m_OffsetY;
	int m_ParallaxX;
	int m_ParallaxY;

	int m_StartLayer;
	int m_NumLayers;
};

struct CMapItemGroup : public CMapItemGroup_v1
{
	enum
	{
		CURRENT_VERSION = 3
	};

	int m_UseClipping;
	int m_ClipX;
	int m_ClipY;
	int m_ClipW;
	int m_ClipH;

	int m_aName[3];
};

struct CMapItemLayer
{
	int m_Version;
	int m_Type;
	int m_Flags;
};

struct CMapItemLayerTilemap
{
	enum
	{
		CURRENT_VERSION = 3,
		TILE_SKIP_MIN_VERSION = 4, // supported for loading but not saving
	};

	CMapItemLayer m_Layer;
	int m_Version;

	int m_Width;
	int m_Height;
	int m_Flags;

	CColor m_Color;
	int m_ColorEnv;
	int m_ColorEnvOffset;

	int m_Image;
	int m_Data;

	int m_aName[3];

	// DDRace

	int m_Tele;
	int m_Speedup;
	int m_Front;
	int m_Switch;
	int m_Tune;
};

struct CMapItemLayerQuads
{
	CMapItemLayer m_Layer;
	int m_Version;

	int m_NumQuads;
	int m_Data;
	int m_Image;

	int m_aName[3];
};

struct CMapItemVersion
{
	enum
	{
		CURRENT_VERSION = 1
	};

	int m_Version;
};

// Represents basic information about envelope points.
// In upstream Teeworlds, this is only used if all CMapItemEnvelope are version 1 or 2.
struct CEnvPoint
{
	enum
	{
		MAX_CHANNELS = 4,
	};

	int m_Time; // in ms
	int m_Curvetype; // CURVETYPE_* constants, any unknown value behaves like CURVETYPE_LINEAR
	int m_aValues[MAX_CHANNELS]; // 1-4 depending on envelope (22.10 fixed point)

	bool operator<(const CEnvPoint &Other) const { return m_Time < Other.m_Time; }
};

// Represents additional envelope point information for CURVETYPE_BEZIER.
// In DDNet, these are stored separately in an UUID-based map item.
// In upstream Teeworlds, CEnvPointBezier_upstream is used instead.
struct CEnvPointBezier
{
	// DeltaX in ms and DeltaY as 22.10 fxp
	int m_aInTangentDeltaX[CEnvPoint::MAX_CHANNELS];
	int m_aInTangentDeltaY[CEnvPoint::MAX_CHANNELS];
	int m_aOutTangentDeltaX[CEnvPoint::MAX_CHANNELS];
	int m_aOutTangentDeltaY[CEnvPoint::MAX_CHANNELS];
};

// Written to maps on upstream Teeworlds for envelope points including bezier information instead of the basic
// CEnvPoint items, if at least one CMapItemEnvelope with version 3 or higher exists in the map.
struct CEnvPointBezier_upstream : public CEnvPoint
{
	CEnvPointBezier m_Bezier;
};

// Used to represent all envelope point information at runtime in editor.
// (Can eventually be different than CEnvPointBezier_upstream)
struct CEnvPoint_runtime : public CEnvPoint
{
	CEnvPointBezier m_Bezier;
};

struct CMapItemEnvelope_v1
{
	enum
	{
		CURRENT_VERSION = 1,
	};

	int m_Version;
	int m_Channels;
	int m_StartPoint;
	int m_NumPoints;
	int m_aName[8];
};

struct CMapItemEnvelope_v2 : public CMapItemEnvelope_v1
{
	enum
	{
		CURRENT_VERSION = 2,
	};

	int m_Synchronized;
};

// Only written to maps in upstream Teeworlds.
// If at least one of these exists in a map, the envelope points
// are represented by CEnvPointBezier_upstream instead of CEnvPoint.
struct CMapItemEnvelope_v3 : public CMapItemEnvelope_v2
{
	enum
	{
		CURRENT_VERSION = 3,
	};
};

typedef CMapItemEnvelope_v2 CMapItemEnvelope;

struct CSoundShape
{
	enum
	{
		SHAPE_RECTANGLE = 0,
		SHAPE_CIRCLE,
		NUM_SHAPES,
	};

	struct CRectangle
	{
		int m_Width, m_Height; // fxp 22.10
	};

	struct CCircle
	{
		int m_Radius;
	};

	int m_Type;

	union
	{
		CRectangle m_Rectangle;
		CCircle m_Circle;
	};
};

struct CSoundSource
{
	CPoint m_Position;
	int m_Loop;
	int m_Pan; // 0 - no panning, 1 - panning
	int m_TimeDelay; // in s
	int m_Falloff; // [0,255] // 0 - No falloff, 255 - full

	int m_PosEnv;
	int m_PosEnvOffset;
	int m_SoundEnv;
	int m_SoundEnvOffset;

	CSoundShape m_Shape;
};

struct CMapItemLayerSounds
{
	enum
	{
		CURRENT_VERSION = 2
	};

	CMapItemLayer m_Layer;
	int m_Version;

	int m_NumSources;
	int m_Data;
	int m_Sound;

	int m_aName[3];
};

struct CMapItemSound
{
	int m_Version;

	int m_External;

	int m_SoundName;
	int m_SoundData;
	// Deprecated. Do not read this value, it could be wrong.
	// Use GetDataSize instead, which returns the de facto size.
	// Value must still be written for compatibility.
	int m_SoundDataSize;
};

// DDRace

class CTeleTile
{
public:
	unsigned char m_Number;
	unsigned char m_Type;
};

class CSpeedupTile
{
public:
	unsigned char m_Force;
	unsigned char m_MaxSpeed;
	unsigned char m_Type;
	short m_Angle;
};

class CSwitchTile
{
public:
	unsigned char m_Number;
	unsigned char m_Type;
	unsigned char m_Flags;
	unsigned char m_Delay;
};

class CDoorTile
{
public:
	unsigned char m_Index;
	unsigned char m_Flags;
	int m_Number;
};

class CTuneTile
{
public:
	unsigned char m_Number;
	unsigned char m_Type;
};

bool IsValidGameTile(int Index);
bool IsValidFrontTile(int Index);
bool IsValidTeleTile(int Index);
bool IsTeleTileCheckpoint(int Index); // Assumes that Index is a valid tele tile index
bool IsTeleTileNumberUsed(int Index, bool Checkpoint); // Assumes that Index is a valid tele tile index
bool IsTeleTileNumberUsedAny(int Index); // Does not check for checkpoint only
bool IsValidSpeedupTile(int Index);
bool IsValidSwitchTile(int Index);
bool IsSwitchTileFlagsUsed(int Index); // Assumes that Index is a valid switch tile index
bool IsSwitchTileNumberUsed(int Index); // Assumes that Index is a valid switch tile index
bool IsSwitchTileDelayUsed(int Index); // Assumes that Index is a valid switch tile index
bool IsValidTuneTile(int Index);
bool IsValidEntity(int Index);
bool IsRotatableTile(int Index);
bool IsCreditsTile(int TileIndex);
int PackColor(CColor Color);

#endif
