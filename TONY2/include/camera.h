#ifndef CAMERA_H
#define CAMERA_H

#include "decomp.h"
#include "types.h"

#include <windows.h>

struct GameObject;
struct OverlayData;
struct ObjectTemplate;

// Camera/view/session object, published through g_camera. Owns the loaded map:
// LoadMap reads the 0x28-byte MAP*.KLG header into m_mapWidth..m_objectsOffset
// and keeps the remainder in m_mapData (MapTile array at offset 0, tile-list
// pointer pool at m_listPoolOffset, u16-length-prefixed object records at
// m_objectsOffset). Scrolling: FollowPlayer (0x411200) accumulates m_player's
// movement into m_scrollX/Y and eases toward GetFollowTarget (0x411440) +
// m_followOffsetX/Y, clamped to the 640x400 view inside m_mapWidth/m_mapHeight;
// m_intX/m_intY and their previous values drive the 64px tile spawn sweeps;
// GetViewOffset returns the midpoint with m_prevX/m_prevY in supersmooth mode.
// m_waterY is the registered water-line Y (-1 = none) used by the swim/surf and
// splash logic; m_templates collects templates to (re)spawn in SpawnVisible
// (0x4119b0).
// SIZE 0x1a0
// 64px map tile record (0x18 stride, tile array at m_mapData offset 0): three
// counts followed by three list pointers relocated in place by LoadMap.
// m_floors entries are wide flat/sloped ground segments (stand/walk lines),
// m_walls entries are tall solid boxes (both HitBox*); m_spawnList entries
// point into the map's object record list. NOTE (proven against MAP*.KLG and
// the LoadMap fixup loop): the file stores counts at 0x00/0x04/0x08 and
// pointers at 0x0c/0x10/0x14, so the declared types of the (m_floorCount,
// m_floors) and (m_wallCount, m_walls) pairs are swapped relative to their
// values. The declarations are kept as-is because the parked getters
// (0x411ad0/0x411b20) and their swept callers in gameobject.cpp compile from
// this shape; fixing the types means swapping the out-args at those call
// sites, which must be reccmp-verified against the local-slot diffs noted on
// CollideWithGroundLines/CollideWithMapBoxes.
// SIZE 0x18
struct MapTile {
	TonyS32* m_floorCount;     // 0x00
	TonyS32* m_wallCount;      // 0x04
	TonyS32 m_spawnCount;      // 0x08
	TonyS32 m_floors;          // 0x0c
	TonyS32 m_walls;           // 0x10
	OverlayData** m_spawnList; // 0x14
};

class Camera {
public:
	Camera();

	void AttachPlayer(GameObject* p_object);
	void LoadMap(char* p_path, TonyS32 p_round);
	void SpawnOnceAll();
	OverlayData* FindMapObject(TonyS32 p_type);
	void SpawnVisible();
	void Update();
	void GetWallsAt(TonyS32 p_x, TonyS32 p_y, TonyS32** p_count, TonyS32* p_list);
	void GetFloorsAt(TonyS32 p_x, TonyS32 p_y, TonyS32** p_count, TonyS32* p_list);
	void LockScroll();
	void UnlockScroll();
	void SetFollowOffset(TonyFloat p_x, TonyFloat p_y);
	void GetViewOffset(TonyFloat* p_xOffset, TonyFloat* p_yOffset);
	void SetPosition(TonyFloat p_x, TonyFloat p_y);
	void ClearTemplates();
	void AddRespawnTemplate(ObjectTemplate* p_template);
	void FollowPlayer();
	void SpawnEnteredTiles();
	void GetFollowTarget(TonyFloat* p_x, TonyFloat* p_y);
	void SpawnTileAt(TonyS32 p_x, TonyS32 p_y);

	TonyFloat m_x;                     // 0x00
	TonyFloat m_y;                     // 0x04
	TonyFloat m_prevX;                 // 0x08
	TonyFloat m_prevY;                 // 0x0c
	TonyU32 m_intX;                    // 0x10
	TonyU32 m_intY;                    // 0x14
	TonyU32 m_prevIntX;                // 0x18
	TonyU32 m_prevIntY;                // 0x1c
	TonyFloat m_scrollX;               // 0x20
	TonyFloat m_scrollY;               // 0x24
	TonyFloat m_followOffsetX;         // 0x28
	TonyFloat m_followOffsetY;         // 0x2c
	GameObject* m_player;              // 0x30
	TonyU32 m_mapWidth;                // 0x34
	TonyU32 m_mapHeight;               // 0x38
	TonyS32 m_tileCols;                // 0x3c
	TonyS32 m_tileRows;                // 0x40
	TonyS32 m_listPoolOffset;          // 0x44
	TonyS32 m_listPoolCount;           // 0x48
	undefined m_pad0[0x54 - 0x4c];     // 0x4c
	TonyS32 m_objectCount;             // 0x54
	TonyS32 m_objectsOffset;           // 0x58
	char* m_mapData;                   // 0x5c
	TonyS32 m_scrollLock;              // 0x60
	TonyS32 m_round;                   // 0x64
	TonyS32 m_levelNum;                // 0x68
	TonyS32 m_world;                   // 0x6c
	TonyS32 m_bonusLevel;              // 0x70
	TonyS32 m_musicTrack;              // 0x74
	TonyS32 m_backdrop;                // 0x78
	TonyS32 m_totals[7];               // 0x7c
	ObjectTemplate* m_templates[0x20]; // 0x98
	TonyS32 m_templateCount;           // 0x118
	TonyFloat m_waterY;                // 0x11c
	WCHAR m_levelName[0x40];           // 0x120
};

DECOMP_SIZE_ASSERT(Camera, 0x1a0)

extern Camera* g_camera;

#endif // CAMERA_H
