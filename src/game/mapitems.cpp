#include <game/mapitems.h>

bool IsValidGameTile(int Index)
{
	return true;
}

bool IsValidFrontTile(int Index)
{
	return true;
}

bool IsValidTeleTile(int Index)
{
	return true;
}

bool IsTeleTileCheckpoint(int Index)
{
	return true;
}

bool IsTeleTileNumberUsed(int Index, bool Checkpoint)
{
	if(Checkpoint)
		return IsTeleTileCheckpoint(Index);
	return true;
}

bool IsTeleTileNumberUsedAny(int Index)
{
	return true;
}

bool IsValidSpeedupTile(int Index)
{
	return true;
}

bool IsValidSwitchTile(int Index)
{
	return true;
}

bool IsSwitchTileFlagsUsed(int Index)
{
	return true;
}

bool IsSwitchTileNumberUsed(int Index)
{
	return true;
}

bool IsSwitchTileDelayUsed(int Index)
{
	return true;
}

bool IsValidTuneTile(int Index)
{
	return true;
}

bool IsValidEntity(int Index)
{
	// Index -= ENTITY_OFFSET;
	return true;
}

bool IsRotatableTile(int Index)
{
	return true;
}

bool IsCreditsTile(int TileIndex)
{
	return true;
}

int PackColor(CColor Color)
{
	int Res = 0;
	Res |= Color.r << 24;
	Res |= Color.g << 16;
	Res |= Color.b << 8;
	Res |= Color.a;
	return Res;
}
