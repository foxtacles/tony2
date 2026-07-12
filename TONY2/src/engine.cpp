// clang-format off
// textlabel.h pulls in afx.h, which must precede any windows.h inclusion.
#include "textlabel.h"
// clang-format on

#include "engine.h"

#include "backgroundrenderer.h"
#include "camera.h"
#include "gameobject.h"
#include "hitbox.h"
#include "objectmanager.h"
#include "registrystore.h"
#include "soundmanager.h"
#include "videomanager.h"

#include <math.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// Camera shake offset pattern, indexed by State::m_savedSpawn slot (bob phase 1-5).
// GLOBAL: TONY2 0x0044c71c
static const TonyFloat g_bobOffsets[6] = {0.0f, 3.0f, 2.0f, 0.0f, -3.0f, -2.0f};

// GLOBAL: TONY2 0x0044c740
static const TonyFloat g_cullMarginNear = 192.0f;
// GLOBAL: TONY2 0x0044c744
static const TonyFloat g_cullScreenW = -640.0f;
// GLOBAL: TONY2 0x0044c748
static const TonyFloat g_cullOneF = 1.0f;
// GLOBAL: TONY2 0x0044c74c
static const TonyFloat g_cullMarginFar = -192.0f;
// GLOBAL: TONY2 0x0044c750
static const TonyFloat g_cullScreenH = -400.0f;

// GLOBAL: TONY2 0x0044c768
static const TonyFloat g_zeroSpeed = 0.0f;

// Per-language glyph key lists (banks 0-2) and the bank pointer table.
// GLOBAL: TONY2 0x00455648
TonyS32 g_fontBank0Keys[62] = {3260, 1916, 1917, 1918, 1919, 1920, 1921, 1922, 1923, 1924, 1925, 1926, 1927,
							   1928, 1929, 1930, 1931, 1932, 1933, 1934, 1935, 1936, 1937, 1938, 1939, 1940,
							   1941, 1942, 1943, 1944, 1945, 1946, 1947, 1948, 1949, 1950, 1951, 1952, 1953,
							   1954, 1955, 1956, 1957, 1958, 1959, 1960, 1961, 1962, 1963, 1964, 1965, 1966,
							   1967, 1968, 1969, 1970, 1971, 1915, 1976, 3252, 3251, 3250};

// GLOBAL: TONY2 0x00455740
TonyS32 g_fontBank1Keys[62] = {3259, 2237, 2238, 2239, 2240, 2241, 2242, 2243, 2244, 2245, 2246, 2247, 2248,
							   2249, 2250, 2251, 2252, 2253, 2254, 2255, 2256, 2257, 2258, 2259, 2260, 2261,
							   2262, 2263, 2264, 2265, 2266, 2267, 2268, 2269, 2270, 2271, 2272, 2273, 2274,
							   2275, 2276, 2277, 2278, 2279, 2280, 2281, 2282, 2283, 2284, 2285, 2286, 2287,
							   2288, 2289, 2290, 2291, 2292, 2293, 2236, 3249, 3248, 3247};

// GLOBAL: TONY2 0x00455838
TonyS32 g_fontBank2Keys[62] = {3261, 2177, 2178, 2179, 2180, 2181, 2182, 2183, 2184, 2185, 2186, 2187, 2188,
							   2189, 2190, 2191, 2192, 2193, 2194, 2195, 2196, 2197, 2198, 2199, 2200, 2201,
							   2202, 2203, 2204, 2205, 2206, 2207, 2208, 2209, 2210, 2211, 2212, 2213, 2214,
							   2215, 2216, 2217, 2218, 2219, 2220, 2221, 2222, 2223, 2224, 2225, 2226, 2227,
							   2228, 2229, 2230, 2231, 2232, 2233, 2176, 3255, 3254, 3253};

// GLOBAL: TONY2 0x00455930
TonyS32* g_fontGlyphIds[3] = {g_fontBank1Keys, g_fontBank0Keys, g_fontBank2Keys};

// GLOBAL: TONY2 0x0045cf48
RegistryStore* g_settings;

// GLOBAL: TONY2 0x0045cf4c
SoundManager* g_soundManager;

// GLOBAL: TONY2 0x0045cf54
BackgroundRenderer* g_backgroundRenderer;

// Deduped 4-dword draw record (see AddLandPiece).
// SIZE 0x10
struct LandPiece {
	TonyS32 m_x;      // 0x00
	TonyS32 m_y;      // 0x04
	TonyS32 m_sprite; // 0x08
	TonyS32 m_kind;   // 0x0c
};

DECOMP_SIZE_ASSERT(LandPiece, 0x10)

// Queued scanline span: packed x/sprite pair, run pointer and half-length.
// SIZE 0x0c
struct RowSegment {
	TonyS16 m_x;      // 0x00
	TonyS16 m_sprite; // 0x02
	TonyS32 m_data;   // 0x04
	TonyS16 m_length; // 0x08
};

DECOMP_SIZE_ASSERT(RowSegment, 0x0c)

// Deferred-draw/sound command queues drained by the render pass.
// GLOBAL: TONY2 0x0045cf50
TonyS32 g_landPieceCount;

// Per-layer copy-job chain heads (= &g_backgroundRenderer->m_jobChains).
// GLOBAL: TONY2 0x0045cf5c
TonyS32* g_composeChains;

// GLOBAL: TONY2 0x0045cf68
RowSegment g_segmentQueue[5000];

// Surface pointer of the last composited frame.
// GLOBAL: TONY2 0x0046b9c8
TonyU8* g_landscapeSurface;

// GLOBAL: TONY2 0x0046b9d0
LandPiece g_landPieces[10000];

// One-shot latch for the compositor globals below.
// GLOBAL: TONY2 0x00492ae0
TonyU8 g_composeLatch;

// GLOBAL: TONY2 0x00492ae8
RowSegment g_spanQueue[5000];

// Compositor output cursor (destination pixel bytes).
// GLOBAL: TONY2 0x004a1548
TonyU8* g_composeRow;

// GLOBAL: TONY2 0x004a154c
TonyS32 g_segmentCount;

// GLOBAL: TONY2 0x004b74e8
TonyS32 g_blankRow[0x1770];

// GLOBAL: TONY2 0x004c3070
TonyS32 g_spanCount;

// Compositor copy-job cursor into the BackgroundRenderer arena.
// GLOBAL: TONY2 0x004c3074
BackgroundRenderer::RowCopy* g_composeJobCursor;

// GLOBAL: TONY2 0x004d1ad8
TonyS32 g_rendererDestroyed;

// Glyph sprite handles, 0x100 per language bank.
// GLOBAL: TONY2 0x004d1ae0
TonyS32 g_fontSprites[0x300];

// Sound-mixer tick handler installed by ServiceStartTimer, invoked by the ServiceTimerProc trampoline.
// GLOBAL: TONY2 0x004d5ae0
void (*g_serviceTick)();

// GLOBAL: TONY2 0x004d5ae8
TonyU16 g_charsetCodes[0x100];

// GLOBAL: TONY2 0x004d5ce8
TonyS32 g_charsetCount;

// GLOBAL: TONY2 0x004d5cf0
TonyU16** g_langStrings[16];

// GLOBAL: TONY2 0x004d5d30
TonyS32 g_langStringCounts[16];

// GLOBAL: TONY2 0x004d5d70
TonyS32 g_language;

// GLOBAL: TONY2 0x004d5d78
TonyU8 g_charsetReverse[0x10000];

// Multimedia timer id returned by timeSetEvent in ServiceStartTimer.
// GLOBAL: TONY2 0x004ed1d0
TonyU32 g_serviceTimerId;

// FUNCTION: TONY2 0x00412f00
void __fastcall ReleaseRiders(GameObject* p_object)
{
	TonyS32 i;

	for (i = 0; i < 0x10; i++) {
		GameObject* slot = ((GameObject**) &p_object->m_state->m_cooldown)[i];

		if (slot != NULL && !(p_object->m_state->m_behavior & (1 << i))) {
			slot->m_head->m_flags &= ~4;
			((GameObject**) &p_object->m_state->m_cooldown)[i] = NULL;
		}
	}
}

// FUNCTION: TONY2 0x00412f50
void __fastcall PushRiders(GameObject* p_object)
{
	TonyFloat dx = p_object->m_state->m_pushX;
	TonyFloat dy = p_object->m_state->m_pushY;
	TonyS32 i;

	for (i = 0; i < 0x10; i++) {
		if (((GameObject**) &p_object->m_state->m_cooldown)[i] != NULL) {
			MoveObject(((GameObject**) &p_object->m_state->m_cooldown)[i], 1, dx, dy);
		}
	}
}

// FUNCTION: TONY2 0x00412fa0
void __fastcall BobTick(GameObject* p_object)
{
	if (p_object->m_state->m_savedSpawn > 0) {
		MoveObject(p_object, 1, 0, g_bobOffsets[p_object->m_state->m_savedSpawn]);
		p_object->m_state->m_savedSpawn++;

		if (p_object->m_state->m_savedSpawn == 6) {
			p_object->m_state->m_savedSpawn = 0;
		}
	}
}

// Camera path-script interpreter: comma-separated commands of a letter plus a
// three-digit repeat count ("r120,u045,w000,..."), stored behind the template
// ext at +0x20. d/l/r/u step the camera through MoveObject, p pauses and w
// waits for state flag 2.
// Fully implemented, kept as STUB because it compares at 71%: both jump tables,
// the command parse and all six execute cases match structurally, but the
// fetch/cursor block schedules with mirrored registers and the shared
// counter-decrement tail merges from a different case order. Register-phase /
// tail-merge family; refine against the diff or retest with the original
// compiler vintage.
// STUB: TONY2 0x00412ff0
void __fastcall RunCameraScript(GameObject* p_object)
{
	char buffer[4];

	if (p_object->m_state->m_typeFlags & 1) {
	fetch:
		char* cmd = (char*) p_object->m_ext + p_object->m_state->m_surfSound + 0x20;
		char* comma = strchr(cmd, ',');

		if (comma == NULL) {
			strchr(cmd, 0);
			p_object->m_state->m_surfSound = 0;
		}
		else {
			p_object->m_state->m_surfSound = (TonyS32) (comma - (char*) p_object->m_ext) - 0x1f;
		}

		*(TonyS8*) &p_object->m_state->m_savedSong = *cmd;

		switch (*cmd) {
		case 'd':
		case 'l':
		case 'p':
		case 'r':
		case 'u':
			strncpy(buffer, cmd + 1, 3);
			buffer[3] = 0;
			*(TonyS32*) ((TonyU8*) &p_object->m_state->m_savedSong + 1) = atoi(buffer);
			break;
		case 'w':
			if (p_object->m_state->m_typeFlags & 2) {
				goto fetch;
			}
			break;
		default:
			goto execute;
		}

		p_object->m_state->m_typeFlags &= ~1;
	}

execute:
	switch (*(TonyS8*) &p_object->m_state->m_savedSong) {
	case 'r':
		MoveObject(p_object, 1, p_object->m_ext->m_walkSpeed, 0);
		(*(TonyS32*) ((TonyU8*) &p_object->m_state->m_savedSong + 1))--;

		if (*(TonyS32*) ((TonyU8*) &p_object->m_state->m_savedSong + 1) == 0) {
			p_object->m_state->m_typeFlags |= 1;
		}
		break;
	case 'l':
		MoveObject(p_object, 1, -p_object->m_ext->m_walkSpeed, 0);
		(*(TonyS32*) ((TonyU8*) &p_object->m_state->m_savedSong + 1))--;

		if (*(TonyS32*) ((TonyU8*) &p_object->m_state->m_savedSong + 1) == 0) {
			p_object->m_state->m_typeFlags |= 1;
		}
		break;
	case 'd':
		MoveObject(p_object, 1, 0, p_object->m_ext->m_jumpSpeed);
		(*(TonyS32*) ((TonyU8*) &p_object->m_state->m_savedSong + 1))--;

		if (*(TonyS32*) ((TonyU8*) &p_object->m_state->m_savedSong + 1) == 0) {
			p_object->m_state->m_typeFlags |= 1;
		}
		break;
	case 'u':
		MoveObject(p_object, 1, 0, -p_object->m_ext->m_jumpSpeed);
		(*(TonyS32*) ((TonyU8*) &p_object->m_state->m_savedSong + 1))--;

		if (*(TonyS32*) ((TonyU8*) &p_object->m_state->m_savedSong + 1) == 0) {
			p_object->m_state->m_typeFlags |= 1;
		}
		break;
	case 'p':
		(*(TonyS32*) ((TonyU8*) &p_object->m_state->m_savedSong + 1))--;

		if (*(TonyS32*) ((TonyU8*) &p_object->m_state->m_savedSong + 1) == 0) {
			p_object->m_state->m_typeFlags |= 1;
		}
		break;
	case 'w':
		if (p_object->m_state->m_behavior != 0) {
			p_object->m_state->m_typeFlags |= 3;
		}
		break;
	}
}

// FUNCTION: TONY2 0x004131d0
void __fastcall StopCameraScript(GameObject* p_object)
{
	p_object->m_state->m_behavior = 0;
	ReleaseRiders(p_object);
	ObjectSetMoveFlags(p_object, 0, 1);
}

// FUNCTION: TONY2 0x00413200
void __fastcall BindOverlayTemplate(GameObject* p_object, ObjectTemplate* p_template)
{
	*(OverlayTemplate::Head*) p_object->m_head = ((OverlayTemplate*) p_template)->m_head;
	p_object->m_state = (GameObject::State*) ((OverlayTemplate::Head*) p_object->m_head + 1);
	p_object->m_ext = &((OverlayTemplate*) p_template)->m_ext;
	p_object->m_state->m_template = p_template;
}

// FUNCTION: TONY2 0x00413230
void __fastcall ScreenTileInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindOverlayTemplate(p_object, p_template);
	p_object->m_tickFn = NullObjectHandler;
	p_object->m_drawFn = ScreenTileDraw;
	p_object->m_destroyFn = NULL;
}

// FUNCTION: TONY2 0x00413250
void __fastcall ScreenTileDraw(GameObject* p_object)
{
	TonyS32 sprite = g_videoManager->FindSprite(0x16, 0);
	TonyU16* header = g_videoManager->m_sprites[sprite];
	TonyS32 width = header[0];
	TonyS32 height = header[1];

	for (TonyS32 y = (TonyU32) g_objectManager->m_frameCounter % (height / 12) * 12 - height; y < height + 0x190;
		 y += height) {
		for (TonyS32 x = -((TonyS32) ((TonyU32) g_objectManager->m_frameCounter % (width / 12)) * 12);
			 x < width + 0x280;
			 x += width) {
			g_videoManager->QueueSprite(sprite, x, y, 0xff, 0);
		}
	}
}

// FUNCTION: TONY2 0x00413330
RegistryStore::RegistryStore(char* p_subKey)
{
	strcpy(m_subKey, "Software\\");
	strcat(m_subKey, p_subKey);
}

// FUNCTION: TONY2 0x00413390
TonyS32 RegistryStore::ReadInt(char* p_name, TonyS32 p_default)
{
	DWORD type = 4;
	DWORD size = 4;

	TonyS32 result = SHGetValueA(HKEY_LOCAL_MACHINE, m_subKey, p_name, &type, &p_name, &size);
	if (result == 2) {
		return p_default;
	}

	if (result) {
		ShowErrorMessage(result);
	}

	return (TonyS32) p_name;
}

// FUNCTION: TONY2 0x004133f0
void RegistryStore::WriteInt(char* p_name, TonyS32 p_value)
{
	SHSetValueA(HKEY_LOCAL_MACHINE, m_subKey, p_name, REG_DWORD, &p_value, 4);
}

// FUNCTION: TONY2 0x00413410
char* RegistryStore::ReadString(char* p_name, char* p_default, char* p_buffer, TonyS32 p_size)
{
	DWORD type = 1;
	LONG result = SHGetValue(HKEY_LOCAL_MACHINE, m_subKey, p_name, &type, p_buffer, (DWORD*) &p_size);

	if (result == 2) {
		strcpy(p_buffer, p_default);
		return p_buffer;
	}

	if (result != 0) {
		ShowErrorMessage(result);
	}

	return p_buffer;
}

// FUNCTION: TONY2 0x00413480
void RegistryStore::WriteString(char* p_name, char* p_value)
{
	SHSetValue(HKEY_LOCAL_MACHINE, m_subKey, p_name, 1, p_value, strlen(p_value));
}

// FUNCTION: TONY2 0x004134b0
void __fastcall CarrierInit(GameObject* p_object, GroupTemplate* p_template)
{
	BindGroupTemplate(p_object, p_template);
	p_object->m_tickFn = MoveTick;
	p_object->m_drawFn = NULL;
	p_object->m_destroyFn = NULL;
	p_object->m_state->m_boundsMinX = 0;
	p_object->m_state->m_boundsMinY = 0;
	p_object->m_state->m_boundsMaxX = 0;
	p_object->m_state->m_boundsMaxY = 0;
	ResetMotion(p_object);
}

// FUNCTION: TONY2 0x004134f0
TonyS32 __fastcall MoveTick(GameObject* p_object)
{
	if (!(p_object->m_head->m_flags & 0x4000)) {
		SnapshotPosition(p_object);
	}

	p_object->m_head->m_x += p_object->m_state->m_velX;
	p_object->m_head->m_y += p_object->m_state->m_velY;
	p_object->m_head->m_x += p_object->m_state->m_pushX;
	p_object->m_head->m_y += p_object->m_state->m_pushY;
	ClearPush(p_object);
	UpdateWorldPosition(p_object);

	if (p_object->m_head->m_flags & 0x2) {
		CullIfOffscreen(p_object);
	}

	return 0;
}

// FUNCTION: TONY2 0x00413560
void __fastcall ResetMotion(GameObject* p_object)
{
	p_object->m_state->m_velX = 0.0f;
	p_object->m_state->m_velY = 0.0f;
	p_object->m_state->m_prevX = p_object->m_head->m_x;
	p_object->m_state->m_prevY = p_object->m_head->m_y;
	p_object->m_state->m_worldX = p_object->m_head->m_x;
	p_object->m_state->m_worldY = p_object->m_head->m_y;
	p_object->m_state->m_prevLocalX = p_object->m_head->m_x;
	p_object->m_state->m_prevLocalY = p_object->m_head->m_y;
	p_object->m_state->m_moveViaParent = 1;
	p_object->m_state->m_parent = 0;
	ClearPush(p_object);
}

// FUNCTION: TONY2 0x004135d0
void __fastcall MoveObject(GameObject* p_object, TonyS32 p_mode, TonyFloat p_dx, TonyFloat p_dy)
{
	if (p_object->m_state->m_moveViaParent == 1 && p_object->m_state->m_parent != NULL) {
		MoveObject(p_object->m_state->m_parent, p_mode, p_dx, p_dy);
		UpdateWorldPosition(p_object);
		return;
	}

	if (p_mode == 0) {
		p_object->m_head->m_x = p_dx + p_object->m_head->m_x;
		p_object->m_head->m_y = p_dy + p_object->m_head->m_y;
	}
	else {
		p_object->m_state->m_pushX = p_dx + p_object->m_state->m_pushX;
		p_object->m_state->m_pushY = p_dy + p_object->m_state->m_pushY;
	}

	UpdateWorldPosition(p_object);
}

// FUNCTION: TONY2 0x00413650
void GameObject::Translate(TonyFloat p_dx, TonyFloat p_dy)
{
	m_head->m_x += p_dx;
	m_head->m_y += p_dy;
	UpdateWorldPosition(this);
}

// FUNCTION: TONY2 0x00413670
void __fastcall UpdateWorldPosition(GameObject* p_object)
{
	if (p_object->m_state->m_parent != NULL) {
		p_object->m_state->m_worldX = p_object->m_head->m_x + p_object->m_state->m_parent->m_state->m_worldX;
		p_object->m_state->m_worldY = p_object->m_state->m_parent->m_state->m_worldY + p_object->m_head->m_y;
	}
	else {
		p_object->m_state->m_worldX = p_object->m_head->m_x;
		p_object->m_state->m_worldY = p_object->m_head->m_y;
	}

	p_object->m_state->m_prevXInt = (TonyS32) p_object->m_state->m_prevX;
	p_object->m_state->m_prevYInt = (TonyS32) p_object->m_state->m_prevY;
	p_object->m_state->m_localXInt = (TonyS32) p_object->m_head->m_x;
	p_object->m_state->m_localYInt = (TonyS32) p_object->m_head->m_y;
	p_object->m_state->m_worldXInt = (TonyS32) p_object->m_state->m_worldX;
	p_object->m_state->m_worldYInt = (TonyS32) p_object->m_state->m_worldY;
}

// FUNCTION: TONY2 0x00413710
TonyBool32 __fastcall HitBoxesOverlap(HitBox* p_a, HitBox* p_b)
{
	if (p_a->m_right >= p_b->m_left && p_a->m_left <= p_b->m_right && p_a->m_bottom >= p_b->m_top &&
		p_a->m_top <= p_b->m_bottom) {
		return TRUE;
	}

	return FALSE;
}

// FUNCTION: TONY2 0x00413750
void __fastcall CullIfOffscreen(GameObject* p_object)
{
	HitBox a;
	HitBox b;

	a.m_left = (TonyFloat) p_object->m_state->m_boundsMinX + p_object->m_state->m_worldX;
	a.m_top = (TonyFloat) p_object->m_state->m_boundsMinY + p_object->m_state->m_worldY;
	a.m_right = (TonyFloat) p_object->m_state->m_boundsMaxX + p_object->m_state->m_worldX;
	a.m_bottom = (TonyFloat) p_object->m_state->m_boundsMaxY + p_object->m_state->m_worldY;

	b.m_left = g_camera->m_x - g_cullMarginNear;
	b.m_top = g_camera->m_y - g_cullMarginNear;
	b.m_right = g_camera->m_x - g_cullScreenW - g_cullOneF - g_cullMarginFar;
	b.m_bottom = g_camera->m_y - g_cullScreenH - g_cullOneF - g_cullMarginFar;

	if (!HitBoxesOverlap(&a, &b)) {
		if (!(p_object->m_head->m_flags & 0x10)) {
			p_object->m_state->m_tickStatus = -2;
		}
		else {
			p_object->m_state->m_tickStatus = -3;
		}
	}
}

// FUNCTION: TONY2 0x00413810
void GameObject::Decelerate(TonyFloat p_x, TonyFloat p_y, TonyFloat p_dx, TonyFloat p_dy)
{
	if (p_dx == -1234.0) {
		p_dx = m_ext->m_dragX;
	}

	if (p_dy == -1234.0) {
		p_dy = m_ext->m_dragY;
	}

	if (p_x == 0.0) {
		if (m_state->m_velX > g_zeroSpeed) {
			TonyFloat speed = m_state->m_velX - p_dx;
			m_state->m_velX = (g_zeroSpeed <= speed) ? speed : g_zeroSpeed;
		}
		else {
			TonyFloat speed = p_dx + m_state->m_velX;
			m_state->m_velX = (g_zeroSpeed < speed) ? g_zeroSpeed : speed;
		}
	}

	if (p_y == 0.0) {
		if (m_state->m_velY > g_zeroSpeed) {
			TonyFloat speed = m_state->m_velY - p_dy;
			m_state->m_velY = (g_zeroSpeed <= speed) ? speed : g_zeroSpeed;
		}
		else {
			TonyFloat speed = p_dy + m_state->m_velY;
			m_state->m_velY = (g_zeroSpeed < speed) ? g_zeroSpeed : speed;
		}
	}

	if (m_state->m_velX == 0.0) {
		if (((OverlayData*) m_head)->m_facing == 4) {
			((OverlayData*) m_head)->m_x = (TonyFloat) floor(((OverlayData*) m_head)->m_x);
		}
		else {
			((OverlayData*) m_head)->m_x = (TonyFloat) ceil(((OverlayData*) m_head)->m_x);
		}
	}

	if (m_state->m_velY == 0.0) {
		if (((OverlayData*) m_head)->m_facing == 1) {
			((OverlayData*) m_head)->m_y = (TonyFloat) floor(((OverlayData*) m_head)->m_y);
		}
		else {
			((OverlayData*) m_head)->m_y = (TonyFloat) ceil(((OverlayData*) m_head)->m_y);
		}
	}
}

// FUNCTION: TONY2 0x004139a0
void GameObject::SetVelocity(TonyFloat p_x, TonyFloat p_y)
{
	m_state->m_velX = p_x;
	m_state->m_velY = p_y;
}

// FUNCTION: TONY2 0x004139c0
void GameObject::Teleport(TonyFloat p_x, TonyFloat p_y)
{
	((OverlayData*) m_head)->m_x = p_x;
	((OverlayData*) m_head)->m_y = p_y;
	UpdateWorldPosition(this);
	SnapshotPosition(this);
}

// FUNCTION: TONY2 0x004139f0
void __fastcall ObjectSetFlags(GameObject* p_object, TonyS32 p_set, TonyS32 p_clear)
{
	((OverlayData*) p_object->m_head)->m_flags |= p_set;
	((OverlayData*) p_object->m_head)->m_flags &= ~p_clear;
}

// FUNCTION: TONY2 0x00413a10
void __fastcall ObjectSetMoveFlags(GameObject* p_object, TonyS32 p_set, TonyS32 p_clear)
{
	p_object->m_state->m_template->m_head.m_flags |= p_set;
	p_object->m_state->m_template->m_head.m_flags &= ~p_clear;
}

// Fully implemented, kept as STUB because it compares at 61%: the -1 pan computation
// (320/0.2/-64 pool constants), range gate, cap, branchless setle clamp and the call all
// match, but the recompile caches the stack pan argument in esi before the prologue and
// spills the sound argument to edi, where the original keeps the pan in eax and re-reads
// the stack slot. Identical entry anomaly to JoystickEnumCallback (0x405430) (see inputmanager.cpp) - the
// same stack-arg pre-prologue caching family. Re-annotate as FUNCTION when solved.
// STUB: TONY2 0x00413a40
TonyS32 __fastcall PlayObjectSound(GameObject* p_object, TonyS32 p_sound, TonyS32 p_pan, TonyS32 p_mode)
{
	TonyS32 pan = p_pan;

	if (pan == -1) {
		pan = (TonyS32) ((p_object->m_state->m_worldX - g_camera->m_x - 320.0f) * 0.2f - -64.0f);
	}

	if (pan < -0x20 || pan > 0x9f) {
		return 0;
	}

	if (pan >= 0x7f) {
		pan = 0x7f;
	}

	return g_soundManager->PlaySample(p_sound, pan <= 0 ? 0 : pan, p_pan);
}

// FUNCTION: TONY2 0x00413ab0
void __fastcall SnapshotPosition(GameObject* p_object)
{
	p_object->m_state->m_prev2LocalX = p_object->m_state->m_prevLocalX;
	p_object->m_state->m_prev2LocalY = p_object->m_state->m_prevLocalY;
	p_object->m_state->m_prev2X = p_object->m_state->m_prevX;
	p_object->m_state->m_prev2Y = p_object->m_state->m_prevY;
	p_object->m_state->m_prevLocalX = p_object->m_head->m_x;
	p_object->m_state->m_prevLocalY = p_object->m_head->m_y;
	p_object->m_state->m_prevX = p_object->m_state->m_worldX;
	p_object->m_state->m_prevY = p_object->m_state->m_worldY;
}

// FUNCTION: TONY2 0x00413b00
void __fastcall GetPrevPosition(GameObject* p_object, TonyFloat* p_outX, TonyFloat* p_outY)
{
	if (p_object->m_head->m_flags & 0x4000) {
		*p_outX = p_object->m_state->m_prevX;
		*p_outY = p_object->m_state->m_prevY;
	}
	else {
		*p_outX = p_object->m_state->m_prevX;
		*p_outY = p_object->m_state->m_prevY;
	}
}

// FUNCTION: TONY2 0x00413b30
void __fastcall ObjectFreeTemplate(GameObject* p_object)
{
	g_objectManager->FreeTemplate(p_object->m_state->m_template);
}

// FUNCTION: TONY2 0x00413b50
void __fastcall ClearPush(GameObject* p_object)
{
	p_object->m_state->m_pushX = 0.0f;
	p_object->m_state->m_pushY = 0.0f;
}

// FUNCTION: TONY2 0x00413b60
void __fastcall QueueObjectSprite(
	GameObject* p_object,
	TonyS32 p_sprite,
	TonyFloat p_x,
	TonyFloat p_y,
	TonyS32 p_layer,
	TonyS32 p_noRecord
)
{
	if (p_object->m_ext->m_screenSpace) {
		g_videoManager->QueueSprite(p_sprite, (TonyS32) floor(p_x), (TonyS32) floor(p_y), p_layer, p_noRecord);
	}
	else {
		TonyFloat xOffset;
		TonyFloat yOffset;

		g_camera->GetViewOffset(&xOffset, &yOffset);
		g_videoManager->QueueSprite(
			p_sprite,
			(TonyS32) floor(p_x - xOffset),
			(TonyS32) floor(p_y - yOffset),
			p_layer,
			p_noRecord
		);
	}
}

// FUNCTION: TONY2 0x00413c10
void __fastcall GetDrawPosition(GameObject* p_object, TonyFloat* p_xOffset, TonyFloat* p_yOffset)
{
	if (g_objectManager->m_smoothPass == 0) {
		*p_xOffset = p_object->m_state->m_worldX;
		*p_yOffset = p_object->m_state->m_worldY;
	}
	else {
		*p_xOffset = (p_object->m_state->m_prevX + p_object->m_state->m_worldX) * 0.5;
		*p_yOffset = (p_object->m_state->m_worldY + p_object->m_state->m_prevY) * 0.5;
	}
}

// FUNCTION: TONY2 0x00413c60
void __fastcall BannerInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = NullObjectHandler;
	p_object->m_drawFn = BannerDraw;
	p_object->m_destroyFn = NULL;
	InitMotion(p_object);
	SetObjectSprite(p_object, p_object->m_ext->m_idleSetR, 5);
}

// FUNCTION: TONY2 0x00413ca0
void __fastcall BannerDraw(GameObject* p_object)
{
	if (g_objectManager->m_drawMode != 0) {
		return;
	}

	TonyS32 width = *g_videoManager->m_sprites[p_object->m_state->m_sprite];
	TonyFloat depth = (TonyFloat) (g_camera->m_mapHeight - 0x190);
	TonyFloat row = (depth - g_camera->m_y) / depth *
						(((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param2 -
						 ((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param1) +
					((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param1;
	TonyFloat x = -((TonyS32) ((TonyFloat) g_camera->m_intX / p_object->m_ext->m_walkSpeed) % width);
	TonyFloat limit = (TonyFloat) (width + 0x280);

	if (x < limit) {
		TonyS32 y = (TonyS32) row;

		do {
			g_videoManager->QueueSprite(
				p_object->m_state->m_sprite,
				(TonyS32) x,
				y,
				((OverlayData*) p_object->m_head)->m_layer,
				0
			);
			x += (TonyFloat) width;
		} while (x < limit);
	}
}

// FUNCTION: TONY2 0x00413dc0
SoundManager::SoundManager(TonyS32 p_music, TonyS32 p_sfx)
{
	m_bankProj = NULL;
	m_registeredCount = 0;
	m_sampleRate = 0x5622;
	m_channels = 0x18;
	m_stereo = 1;
	m_musicOn = 0;
	m_sfxOn = 0;
	m_currentSong = -1;

	for (TonyS32 count = 0; count < 0x20; count++) {
		m_songs[count].m_data = NULL;
		m_songs[count].m_handle = -1;
	}

	SetEnabled(p_music, p_sfx);
}

// FUNCTION: TONY2 0x00413e30
void SoundManager::Shutdown()
{
	if (m_musicOn == 1 || m_sfxOn == 1) {
		SndStopAll();

		if (m_currentSong != -1 && m_songs[m_currentSong].m_handle != -1) {
			SampleStop(m_songs[m_currentSong].m_handle);
		}

		while (!SndIsIdle()) {
		}

		SndClose();
		SeqClose();
	}

	FreeSongs();
	FreeBanks();
}

// FUNCTION: TONY2 0x00413e90
void SoundManager::PlaySong(TonyS32 p_track)
{
	if (m_musicOn == 1 && g_settings->ReadInt("Music", 1) == 1 && p_track != -1) {
		StopSong();
		m_songs[p_track].m_handle =
			StreamPlayEntry(m_songs[p_track].m_bank, m_songs[p_track].m_entry, m_songs[p_track].m_data, 0);
		SampleFadeVolume(0x64, 0, m_songs[p_track].m_handle, 0);
	}

	m_currentSong = p_track;
}

// FUNCTION: TONY2 0x00413f00
TonyS32 SoundManager::StopSong()
{
	TonyS32 track = m_currentSong;

	FadeOutSong();
	m_currentSong = -1;
	return track;
}

// FUNCTION: TONY2 0x00413f20
TonyS32 SoundManager::FadeOutSong()
{
	if (m_musicOn == 1 && m_currentSong != -1) {
		SampleFadeVolume(0, 0x3e8, m_songs[m_currentSong].m_handle, 1);
		m_songs[m_currentSong].m_handle = -1;
	}

	return m_currentSong;
}

// FUNCTION: TONY2 0x00414050
void SoundManager::FreeBanks()
{
	if (m_bankProj) {
		free(m_bankProj);
		free(m_bankPool);
		free(m_bankSamp);
		free(m_bankSdir);
		m_bankProj = NULL;
	}
}

// FUNCTION: TONY2 0x00414090
TonyS32 SoundManager::LoadSong(char* p_name, TonyU16 p_bank, TonyU16 p_entry)
{
	TonyS32 slot;
	TonyS32 i;

	slot = -1;

	for (i = 0; i < (TonyS32) sizeOfArray(m_songs); i++) {
		if (m_songs[i].m_data == NULL) {
			slot = i;
			break;
		}
	}

	if (slot == -1) {
		return -1;
	}

	ReadFileBlob(p_name, &m_songs[slot].m_data);
	m_songs[slot].m_bank = p_bank;
	m_songs[slot].m_entry = p_entry;
	return slot;
}

// FUNCTION: TONY2 0x004140f0
void SoundManager::FreeSongs()
{
	TonyS32 i;

	for (i = 0; i < (TonyS32) sizeOfArray(m_songs); i++) {
		if (m_songs[i].m_data) {
			free(m_songs[i].m_data);
			m_songs[i].m_data = NULL;
		}
	}
}

// FUNCTION: TONY2 0x00414120
void SoundManager::RegisterSongBank(TonyU16 p_track)
{
	BankRegister(m_bankProj, p_track, m_bankSamp, m_bankSdir, m_bankPool);
	m_registered[m_registeredCount] = p_track;
	m_registeredCount++;
}

// FUNCTION: TONY2 0x00414160
TonyS32 SoundManager::PlaySample(TonyS32 p_sound, TonyS32 p_pan, TonyS32 p_level)
{
	if (m_sfxOn == 1) {
		if (p_level == -1) {
			p_level = 0xff;
		}

		return SamplePlay(p_sound, p_level, p_pan);
	}

	return 0;
}

// FUNCTION: TONY2 0x00414190
void SoundManager::SetEnabled(TonyS32 p_music, TonyS32 p_sfx)
{
	TonyS32 old = m_musicOn;
	TonyS32 result;

	if (p_music == 0) {
		FadeOutSong();
	}

	if ((m_musicOn == 1 || m_sfxOn == 1) && p_music == 0 && p_sfx == 0) {
		SndClose();
		SeqClose();
		m_musicOn = 0;
		m_sfxOn = 0;
	}
	else if (m_musicOn == 0 && m_sfxOn == 0 && (p_music == 1 || p_sfx == 1)) {
		m_musicOn = p_music;
		m_sfxOn = p_sfx;
		result = SndOpenDSound(m_sampleRate, m_channels, 0x10, 7, m_stereo, 0, 0x70010000, g_hWnd);
		ReregisterSongBanks();

		if (result == 0) {
			TrackFadeTo(0x7f, 0, 0xff);
		}
		else {
			m_musicOn = 0;
			m_sfxOn = 0;
		}
	}
	else {
		m_musicOn = p_music;
		m_sfxOn = p_sfx;
	}

	if (m_musicOn == 1 && old == 0) {
		PlaySong(m_currentSong);
	}
}

// FUNCTION: TONY2 0x00414260
void SoundManager::ReregisterSongBanks()
{
	TonyS32 i;

	for (i = 0; i < m_registeredCount; i++) {
		BankRegister(m_bankProj, m_registered[i], m_bankSamp, m_bankSdir, m_bankPool);
	}
}

// FUNCTION: TONY2 0x004142a0
void SoundManager::StopHandle(void* p_handle)
{
	HandleRelease(p_handle);
}

// FUNCTION: TONY2 0x004142b0
TonyS32 SoundManager::SuspendSong()
{
	TonyS32 track = m_currentSong;

	if (track != -1) {
		SampleFadeVolume(0, 0x3e8, m_songs[track].m_handle, 2);
	}

	m_currentSong = -1;
	return track;
}

// FUNCTION: TONY2 0x004142f0
void SoundManager::ResumeSong(TonyS32 p_track)
{
	if (m_musicOn == 1 && p_track != -1) {
		SamplePause(m_songs[p_track].m_handle);
		SampleFadeVolume(0x64, 0x3e8, m_songs[p_track].m_handle, 0);
	}

	m_currentSong = p_track;
}

// FUNCTION: TONY2 0x00414330
TonyS32 SoundManager::SetSongVolume(TonyU8 p_volume)
{
	SampleFadeVolume(p_volume, 0x3e8, m_songs[m_currentSong].m_handle, 0);
	return 0x64;
}

// FUNCTION: TONY2 0x00414360
void SoundManager::LoadSpeechBank(char* p_name)
{
	ReadFileBlob(p_name, &m_speechBank);
	SpeechStartup();
	SpeechSubmitBlock((TonyS32*) m_speechBank, 0);
}

// FUNCTION: TONY2 0x00414390
void SoundManager::PlaySpeech(TonyS32 p_block)
{
	m_speechHandle = SpeechPlayBlock(p_block, 0, 0xff, 0x7f, 0x40, 0, 0);
}

// FUNCTION: TONY2 0x004143c0
TonyS32 SoundManager::IsSpeechPlaying()
{
	return SpeechIsPlaying(m_speechHandle);
}

// FUNCTION: TONY2 0x004143e0
void SoundManager::UnloadSpeechBank()
{
	SpeechStop(m_speechHandle);

	do {
	} while (SpeechIsPlaying(m_speechHandle) == 1);

	SpeechShutdown();
	free(m_speechBank);
}

// FUNCTION: TONY2 0x00414420
void SoundManager::StopSpeech()
{
	SpeechStop(m_speechHandle);
}

// FUNCTION: TONY2 0x00414430
BackgroundRenderer::BackgroundRenderer()
{
	TonyS32 i;

	m_bufferCount = 0;
	m_buffers = NULL;

	for (i = 0; i < (TonyS32) sizeOfArray(g_blankRow); i++) {
		g_blankRow[i] = 0x80008;
	}

	ResetArenas();
	m_jobCursor = m_jobArena;
	m_trackCount = 1;

	for (i = 0; i < (TonyS32) sizeOfArray(m_jobChains); i++) {
		m_jobChains[i] = 0;
	}

	for (i = 0; i < (TonyS32) sizeOfArray(m_tracks); i++) {
		m_tracks[i].m_rows = NULL;
		m_tracks[i].m_arena = NULL;
	}
}

// FUNCTION: TONY2 0x004144a0
void BackgroundRenderer::Destroy()
{
	FreeTracks();
	g_rendererDestroyed = 1;
}

// FUNCTION: TONY2 0x004144b0
void BackgroundRenderer::FreeTracks()
{
	TonyS32 i;

	for (i = 0; i < m_bufferCount; i++) {
		if (m_buffers[i].m_id) {
			delete m_buffers[i].m_buffer;
			m_buffers[i].m_buffer = NULL;
			m_buffers[i].m_id = NULL;
		}
	}

	if (m_bufferCount) {
		delete m_buffers;
		m_buffers = NULL;
		m_bufferCount = 0;
	}

	if (m_tracks[0].m_rows) {
		delete m_tracks[0].m_rows;
		m_tracks[0].m_rows = NULL;
	}

	for (i = 1; i < m_trackCount; i++) {
		if (m_tracks[i].m_rows) {
			if (m_tracks[i].m_arena) {
				delete m_tracks[i].m_arena;
				m_tracks[i].m_arena = NULL;
			}

			delete m_tracks[i].m_rows;
			m_tracks[i].m_rows = NULL;
		}
	}

	m_trackCount = 1;
}

// FUNCTION: TONY2 0x00414580
void BackgroundRenderer::ResetArenas()
{
	TonyS32 i;

	m_spanCursor = m_spanArena;
	memset(m_rowStamps, 0, 0x1400);
	m_trackCursor = m_trackArena;
	m_drawCursor = (DrawRecord*) m_drawArena;

	for (i = 0; i < (TonyS32) sizeOfArray(m_drawChains); i++) {
		m_drawChains[i] = NULL;
	}
}

// FUNCTION: TONY2 0x004145d0
void BackgroundRenderer::RebuildRowRing()
{
	TonyS32 i;

	m_rowHead = 0;

	for (i = 0; i < (TonyS32) sizeOfArray(m_rowRing); i++) {
		m_rowRing[i].m_spans = m_tracks[0].m_rows[i + m_scrollY].m_block;
		m_rowRing[i].m_phase = 0;
	}
}

// Fully implemented, kept as STUB because it compares at 65%: both scroll loops,
// the wrap logic and the rebuild paths match, but the x/y locals and loop scratch
// registers come out phase-shifted (eax/edx roles, one extra spill slot).
// Register round-robin / slot-direction family; retest with the original
// compiler vintage.
// STUB: TONY2 0x00414610
void BackgroundRenderer::ScrollTo(TonyS32 p_x, TonyS32 p_y)
{
	TonyS32 x = p_x + 1;
	TonyS32 y = p_y + 1;
	TonyS32 delta = y - m_scrollY;
	TonyS32 i;

	if (delta < 0) {
		if (delta < -0x190) {
			m_scrollY = y;
			RebuildRowRing();
		}
		else {
			for (i = -delta; i > 0; i--) {
				m_rowHead--;

				if (m_rowHead < 0) {
					m_rowHead += 0x190;
				}

				m_scrollY--;
				m_rowRing[m_rowHead].m_phase = 0;
				m_rowRing[m_rowHead].m_spans = m_tracks[0].m_rows[m_scrollY].m_block;
			}
		}
	}

	if (delta > 0) {
		if (delta > 0x190) {
			m_scrollY = y;
			RebuildRowRing();
		}
		else {
			for (; delta > 0; delta--) {
				m_rowRing[m_rowHead].m_phase = 0;
				m_rowRing[m_rowHead].m_spans = m_tracks[0].m_rows[m_scrollY + 0x190].m_block;
				m_rowHead++;

				if (m_rowHead >= 0x190) {
					m_rowHead -= 0x190;
				}

				m_scrollY++;
			}
		}
	}

	m_scrollY = y;
	m_scrollX = x;
}

// Fully implemented, kept as STUB because it compares at 38%: the layer walk,
// job-chain memcpy (word-align prologue) and DrawRecord draw drain all match
// semantically, but the original builds an ebp frame with memory-homed loop
// state, seeds dst/src straight into edi/esi for the string ops, and
// materializes the align mask via xor/mov al. Several allocator/seeding
// fingerprints stacked; retest with the original compiler vintage.
// STUB: TONY2 0x00414710
void BackgroundRenderer::FlushRowJobs()
{
	TonyS32 i;

	for (i = 0; i < 0x100; i++) {
		RowCopy* job = (RowCopy*) m_jobChains[i];

		while (job != NULL) {
			TonyU16* dst = job->m_dest;
			TonyU16* src = job->m_source;
			TonyS32 count = job->m_count;

			job = job->m_next;

			if ((TonyU32) dst & 3) {
				*dst = *src;
				dst++;
				src++;
				count--;
			}

			memcpy(dst, src, count * 2);
		}

		m_jobChains[i] = 0;

		for (DrawRecord* draw = m_drawChains[i]; draw != NULL; draw = draw->m_next) {
			BlitSprite(
				g_videoManager->m_sprites[draw->m_sprite],
				g_videoManager->m_spriteRows[draw->m_sprite],
				draw->m_x,
				draw->m_y,
				0,
				(TonyU8*) g_backgroundRenderer->m_canvas
			);
		}
	}

	m_jobCursor = (undefined*) &m_jobArena;
}

// Fully implemented, kept as STUB because it compares at 83%: the height gate,
// blank-row fallback, run walk and clamped tail emits all match, but the track
// record loads fold into indexed addressing where the original homes the lea,
// and the clamp arm picks eax vs edx (register phase). Retest with the original
// compiler vintage.
// STUB: TONY2 0x004147d0
BackgroundRenderer::RowSpan* __fastcall EmitSpriteRowSpans(
	TonyS32 p_sprite,
	TonyS32 p_row,
	BackgroundRenderer::RowSpan* p_cursor
)
{
	TonyS32 height = g_videoManager->m_sprites[p_sprite][1];

	if (p_row >= 0 && p_row < height) {
		BackgroundRenderer::RleRow* channel = (BackgroundRenderer::RleRow*) g_videoManager->m_spriteRows[p_sprite];
		BackgroundRenderer::RleRow* track = &channel[p_row];
		TonyU8* data = (TonyU8*) track->m_block;
		TonyS32 x = track->m_x;

		if (track->m_row == p_row) {
			TonyU32 skip;

			data += 4;

			if (x > 0) {
				p_cursor = EmitSpan(p_cursor, &g_blankRow, (TonyS16) x, 0);
			}

			do {
				TonyS32 length = *(TonyS32*) data;
				TonyS32 half = length / 2;
				TonyS32 next;

				p_cursor = EmitSpan(p_cursor, data + 4, (TonyS16) half, 1);
				data += length + 4;
				x += half;
				skip = *(TonyU32*) data;
				next = x + (TonyS32) (skip >> 1);

				if ((TonyU32) next >= 0x280) {
					if (0x280 - x > 0) {
						p_cursor = EmitSpan(p_cursor, &g_blankRow, (TonyS16) (0x280 - x), 0);
					}
				}
				else {
					p_cursor = EmitSpan(p_cursor, &g_blankRow, (TonyS16) (skip >> 1), 0);
				}

				data += 4;
				x = next;
			} while (!(skip & 0x80000000));

			return p_cursor;
		}
	}

	return EmitSpan(p_cursor, &g_blankRow, 0x280, 0);
}

// Fully implemented, kept as STUB because it compares at 71%: three stores and
// the cursor advance match, but the original reuses ax for both word loads where
// SP3 picks dx for the second (register-reuse phase, dominates this tiny body).
// Retest with the original compiler vintage.
// STUB: TONY2 0x004148b0
BackgroundRenderer::RowSpan* __fastcall EmitSpan(
	BackgroundRenderer::RowSpan* p_slot,
	void* p_data,
	TonyS16 p_length,
	TonyS16 p_kind
)
{
	p_slot->m_length = p_length;
	p_slot->m_data = p_data;
	p_slot->m_kind = p_kind;
	return p_slot + 1;
}

// Fully implemented, kept as STUB because it compares at 71%: the arena/table
// allocation, both fill modes (static 0x320-row wrap vs parallax double-bank)
// and the ParallaxTrack tail all match, but the height/arena locals come out in
// mirrored registers (bp/ax, edi/ebp) with shifted spill slots, including the
// original reusing the dead p_flag arg slot for the blank-row cursor. Register
// round-robin / slot-direction family; retest with the original compiler vintage.
// STUB: TONY2 0x004148d0
void BackgroundRenderer::AddParallaxTrack(
	TonyS32 p_sprite,
	TonyS32 p_x,
	TonyFloat p_scale,
	TonyFloat p_speed,
	TonyS32 p_flag
)
{
	TonyS32 height = g_videoManager->m_sprites[p_sprite][1];
	ParallaxTrack* drum = &m_tracks[m_trackCount];
	RowSpan* blankA;
	RowSpan* blankB;
	RowSpan* cursor;
	TonyS32 i;

	blankA = (RowSpan*) new TonyU8[0x13880];
	drum->m_arena = (TonyU16*) blankA;
	blankB = EmitSpan(blankA, &g_blankRow, 0x280, (TonyS16) p_flag);
	cursor = EmitSpan(blankB, &g_blankRow, 0x280, (TonyS16) p_flag);
	drum->m_rows = (RleRow*) new TonyU8[0x2580];

	if (p_x == 0) {
		for (i = 0; i < 0x320; i++) {
			TonyS32 row = i % 0x190;

			if (row >= 0 && row < height) {
				drum->m_rows[i].m_block = cursor;
				cursor = EmitSpriteRowSpans(p_sprite, row, cursor);
				drum->m_rows[i].m_x = (TonyS32) cursor;
				cursor = EmitSpriteRowSpans(p_sprite, row, cursor);
			}
			else {
				drum->m_rows[i].m_block = blankA;
				drum->m_rows[i].m_x = (TonyS32) blankB;
			}

			drum->m_rows[i].m_row = 0x280;
		}
	}
	else {
		for (i = 0; i < 0x190; i++) {
			drum->m_rows[i].m_block = blankA;
			drum->m_rows[i].m_x = (TonyS32) blankB;
		}

		for (i = 0; i < height; i++) {
			drum->m_rows[i + 0x190].m_block = cursor;
			cursor = EmitSpriteRowSpans(p_sprite, i, cursor);
			drum->m_rows[i + 0x190].m_x = (TonyS32) cursor;
			cursor = EmitSpriteRowSpans(p_sprite, i, cursor);
		}

		for (i = height; i < 0x190; i++) {
			drum->m_rows[i + 0x190].m_block = blankA;
			drum->m_rows[i + 0x190].m_x = (TonyS32) blankB;
		}
	}

	drum->m_scale = p_scale;
	drum->m_height = 0x190;
	drum->m_rowCount = 0x320;
	drum->m_width = 0x280;
	drum->m_speed = p_speed;
	drum->m_x = p_x;
	m_trackCount++;
}

// Recursive span compositor: syncs the foreground walker to p_base, then merges
// the layer run lists front-to-back, chaining RowCopy copy jobs per kind and
// recursing into the next walker across transparent runs. The original is
// hand-written assembly (internal subroutine with flag-based returns), kept as
// faithful inline asm like the blitter family.
// FUNCTION: TONY2 0x00414ab0
void __fastcall ComposeSpans(BackgroundRenderer::RowWalker* p_walkers, TonyS32 p_base, TonyS32 p_limit, TonyU8* p_row)
{
	TonyU8 latch = g_composeLatch;

	if (!(latch & 1)) {
		latch |= 1;
		g_composeRow = p_row;
		g_composeLatch = latch;
	}

	if (!(latch & 2)) {
		latch |= 2;
		g_composeLatch = latch;
		g_composeJobCursor = (BackgroundRenderer::RowCopy*) g_backgroundRenderer->m_jobCursor;
	}

	if (!(latch & 4)) {
		latch |= 4;
		g_composeLatch = latch;
		g_composeChains = g_backgroundRenderer->m_jobChains;
	}

	g_composeRow = p_row;
	g_composeJobCursor = (BackgroundRenderer::RowCopy*) g_backgroundRenderer->m_jobCursor;
	g_composeChains = g_backgroundRenderer->m_jobChains;

	__asm {
		push ebp
		push p_base
		push p_limit
		mov eax, p_walkers
		mov ebx, [eax + 4]
		mov ecx, [eax]
		mov edx, ebx
		add dx, word ptr [ecx]
		cmp edx, [esp + 4]
		jg retreat
	advance:
		add ecx, 8
		mov ebx, edx
		add dx, word ptr [ecx]
		cmp edx, [esp + 4]
		jle advance
		mov [eax + 4], ebx
	retreat:
		cmp ebx, [esp + 4]
		jle synced
	back:
		sub ecx, 8
		mov dx, word ptr [ecx]
		sub ebx, edx
		cmp ebx, [esp + 4]
		jg back
		mov [eax + 4], ebx
	synced:
		mov [eax + 4], ebx
		mov [eax], ecx
		call walker
		jmp merged
	walker:
		mov ebx, [eax + 4]
		mov ecx, [eax]
		mov edx, ebx
		add dx, word ptr [ecx]
		cmp edx, [esp + 8]
		jg entered
	seek:
		add ecx, 8
		mov ebx, edx
		add dx, word ptr [ecx]
		cmp edx, [esp + 8]
		jle seek
		mov [eax + 4], ebx
	entered:
		sub edx, ebx
		mov edi, [ecx + 4]
		cmp ebx, [esp + 8]
		jge aligned
		add edi, [esp + 8]
		add edi, [esp + 8]
		sub edx, [esp + 8]
		sub edi, ebx
		sub edi, ebx
		add edx, ebx
		mov ebx, [esp + 8]
	aligned:
		mov esi, ebx
		add esi, edx
		cmp esi, [esp + 4]
		jg tail
	runs:
		cmp word ptr [ecx + 2], 0
		jne emit
		push ecx
		push ebx
		push esi
		add eax, 8
		call walker
		sub eax, 8
		pop ebx
		pop ecx
		pop ecx
		ja next
	emit:
		mov ebp, g_composeJobCursor
		mov [ebp + 4], edi
		mov esi, g_composeRow
		mov [ebp], esi
		xor esi, esi
		mov [ebp + 0xc], edx
		mov si, word ptr [ecx + 2]
		shl esi, 2
		add esi, g_composeChains
		mov edi, [esi]
		mov [ebp + 8], edi
		mov [esi], ebp
		add g_composeJobCursor, 0x10
		add ebx, edx
		shl edx, 1
		add g_composeRow, edx
	next:
		add ecx, 8
		mov dx, word ptr [ecx]
		mov edi, [ecx + 4]
		mov esi, ebx
		add esi, edx
		cmp esi, [esp + 4]
		jle runs
		mov [eax + 4], ebx
	tail:
		mov [eax], ecx
		cmp ebx, [esp + 4]
		jge done
		mov edx, [esp + 4]
		sub edx, ebx
		cmp word ptr [ecx + 2], 0
		jne emit2
		push ecx
		push ebx
		add ebx, edx
		push ebx
		add eax, 8
		call walker
		sub eax, 8
		pop ebx
		pop ecx
		pop ecx
		ja done
	emit2:
		mov ebp, g_composeJobCursor
		mov [ebp + 4], edi
		mov esi, g_composeRow
		mov [ebp], esi
		xor esi, esi
		mov [ebp + 0xc], edx
		mov si, word ptr [ecx + 2]
		shl esi, 2
		add esi, g_composeChains
		mov edi, [esi]
		mov [ebp + 8], edi
		mov [esi], ebp
		add g_composeJobCursor, 0x10
		shl edx, 1
		add g_composeRow, edx
	done:
		ret
	merged:
		pop eax
		pop eax
		pop ebp
	}

	g_backgroundRenderer->m_jobCursor = (undefined*) g_composeJobCursor;
}

// Fully implemented, kept as STUB because it compares at 87%: the table build,
// row fill and ParallaxTrack tail all match, but the count-0x18f store computes via
// add (mutating the CSE register) where the original emits lea into a fresh
// register, shifting the tail registers. Lea/add allocator margin; retest with
// the original compiler vintage.
// STUB: TONY2 0x00414cc0
void BackgroundRenderer::BuildMainTrack()
{
	TonyS32 i;

	m_tracks[0].m_rows = (RleRow*) new TonyU8[m_bufferCount * 12];

	for (i = 0; i < m_bufferCount; i++) {
		m_tracks[0].m_rows[i].m_block = m_buffers[i].m_buffer;
		m_tracks[0].m_rows[i].m_x = (TonyS32) ((RowSpan*) m_buffers[i].m_buffer + m_buffers[i].m_id - 1);
		m_tracks[0].m_rows[i].m_row = g_camera->m_mapWidth;
	}

	m_tracks[0].m_rowCount = m_bufferCount;
	m_tracks[0].m_height = m_bufferCount - 0x18f;
	m_tracks[0].m_width = g_camera->m_mapWidth;
	m_tracks[0].m_scale = 1.0f;
	m_tracks[0].m_speed = 1.0f;
}

// Fully implemented, kept as STUB because it compares at 62%: the parallax
// offsets, ring walk, walker staging and both calls match, but base/ring/rowOff
// land in mirrored registers (ebp/eax, ebx/ebp) with two spill slots shifted
// (the original reuses the dead arg slot differently). Register round-robin /
// slot-direction family; retest with the original compiler vintage.
// STUB: TONY2 0x00414d70
void BackgroundRenderer::RenderLandscape(TonyU8* p_surface)
{
	TonyU8* base = p_surface;
	TonyS32 rowOff1;
	TonyS32 rowOff2;
	TonyS32 x1;
	TonyS32 x2;
	TonyS32 ring;
	RowWalker walkers[3];
	TonyS32 i;

	m_canvas = p_surface;
	rowOff1 = (TonyS32) (m_tracks[1].m_speed * m_scrollY + m_tracks[1].m_x);
	rowOff2 = (TonyS32) (m_tracks[2].m_speed * m_scrollY + m_tracks[2].m_x);
	x1 = m_scrollX - (TonyS32) (m_tracks[1].m_scale * m_scrollX) % 0x280;
	x2 = m_scrollX - (TonyS32) (m_tracks[2].m_scale * m_scrollX) % 0x280;
	ring = m_rowHead;

	for (i = 0; i < 0x190; i++) {
		walkers[0] = m_rowRing[ring];
		walkers[1].m_spans = m_tracks[1].m_rows[rowOff1].m_block;
		walkers[1].m_phase = x1;

		if (m_trackCount >= 3) {
			walkers[2].m_spans = m_tracks[2].m_rows[rowOff2].m_block;
			walkers[2].m_phase = x2;
		}

		ComposeSpans(walkers, m_scrollX, m_scrollX + 0x280, p_surface);
		m_rowRing[ring] = walkers[0];
		p_surface += 0x500;
		ring++;

		if (ring >= 0x190) {
			ring -= 0x190;
		}

		rowOff1++;
		rowOff2++;
	}

	FlushRowJobs();
	g_landscapeSurface = base;
}

// FUNCTION: TONY2 0x00414f10
void __stdcall PushSpan(RowSegment p_rec)
{
	g_spanQueue[g_spanCount++] = p_rec;
}

// Fully implemented, kept as STUB because it compares at 24%: the queue snapshot
// (chkstk frame + memcpy), the four clip cases and the by-value re-pushes all
// match semantically (PushSpan's record model is proven at 100%), but the
// original's frame is 0x10 smaller (piece temps fold into the by-value arg area)
// and the cur/staging slots shift accordingly, which cascades through every
// stack reference. Local-slot/arg-staging family; retest with the original
// compiler vintage.
// STUB: TONY2 0x00414f40
void __stdcall InsertSpanClipped(RowSegment p_rec)
{
	RowSegment queue[5000];
	TonyS32 n = g_spanCount;
	RowSegment* span;

	if (n > 0) {
		TonyS32 dwords = g_spanCount * 3;

		memcpy(queue, g_spanQueue, dwords * 4);
	}

	g_spanCount = 0;

	for (span = queue; n > 0; n--) {
		RowSegment cur = *span;

		if (cur.m_x < p_rec.m_x) {
			if (cur.m_x + (TonyU16) cur.m_length > p_rec.m_x) {
				if (cur.m_x + (TonyU16) cur.m_length <= p_rec.m_x + (TonyU16) p_rec.m_length) {
					cur.m_length = p_rec.m_x - cur.m_x;
					PushSpan(cur);
				}
				else {
					RowSegment piece = *span;

					piece.m_x = p_rec.m_x + (TonyU16) p_rec.m_length;
					piece.m_length = cur.m_x - piece.m_x + cur.m_length;
					piece.m_data = cur.m_data + ((TonyU16) cur.m_length - (TonyU16) piece.m_length) * 2;
					PushSpan(piece);

					cur.m_length = p_rec.m_x - cur.m_x;
					PushSpan(cur);
				}
			}
			else {
				PushSpan(*span);
			}
		}
		else if (cur.m_x < p_rec.m_x + (TonyU16) p_rec.m_length) {
			if (cur.m_x + (TonyU16) cur.m_length > p_rec.m_x + (TonyU16) p_rec.m_length) {
				TonyS16 x = cur.m_x;
				TonyS16 length = cur.m_length;

				cur.m_x = p_rec.m_x + (TonyU16) p_rec.m_length;
				cur.m_length = x + length - cur.m_x;
				cur.m_data = cur.m_data + ((TonyU16) length - (TonyU16) cur.m_length) * 2;
				PushSpan(cur);
			}
		}
		else {
			PushSpan(*span);
		}

		span++;
	}

	PushSpan(p_rec);
}

// FUNCTION: TONY2 0x00415150
void __fastcall PushSegment(TonyS16 p_x, TonyS16 p_sprite, TonyS16 p_length, TonyS32 p_data)
{
	RowSegment rec;

	rec.m_x = p_x;
	rec.m_sprite = p_sprite;
	rec.m_data = p_data;
	rec.m_length = p_length;
	g_segmentQueue[g_segmentCount++] = rec;
}

// Fully implemented, kept as STUB because it compares at 28%: the height gate,
// track row check and the span-emit loop all match semantically (PushSegment's
// record layout is proven at 100%), but every scratch register lands one
// round-robin phase off (height in edx vs ecx, sprite/base reloads swapped,
// ebp pushed early). Register-phase family; retest with the original vintage.
// STUB: TONY2 0x004151a0
void __fastcall EmitSpriteRowSegments(
	TonyS32 p_pos,
	TonyS32 p_kind,
	TonyS32 p_xBase,
	TonyS32 p_y,
	TonyS32 p_layer,
	TonyS32 p_sprite
)
{
	TonyS32 height = g_videoManager->m_sprites[p_sprite][1];
	TonyS32 row = p_pos - p_y;

	if (row < 0 || row >= height) {
		return;
	}

	BackgroundRenderer::RleRow* track = &((BackgroundRenderer::RleRow*) g_videoManager->m_spriteRows[p_sprite])[row];
	TonyU8* data = (TonyU8*) track->m_block;
	TonyS32 x = track->m_x;
	TonyU32 skip;

	if (track->m_row != row) {
		return;
	}

	data += 4;
	x += p_xBase;

	do {
		TonyS32 length = *(TonyS32*) data;

		PushSegment((TonyS16) x, (TonyS16) p_sprite, (TonyS16) (length / 2), (TonyS32) (data + 4));
		skip = *(TonyU32*) (data + length + 4);
		data += length + 8;
		x += length / 2 + (skip >> 1);
	} while (!(skip & 0x80000000));
}

// FUNCTION: TONY2 0x00415230
void __fastcall AddLandPiece(TonyS32 p_a, TonyS32 p_b, TonyS32 p_c, TonyS32 p_d)
{
	LandPiece rec;
	TonyS32 i;

	rec.m_x = p_b;
	rec.m_kind = p_a;
	rec.m_y = p_c;
	rec.m_sprite = p_d;

	for (i = 0; i < g_landPieceCount; i++) {
		if (memcmp(&g_landPieces[i], &rec, 0x10) == 0) {
			return;
		}
	}

	g_landPieces[g_landPieceCount++] = rec;
}

// FUNCTION: TONY2 0x004152b0
void BackgroundRenderer::RecordDraw(TonyS32 p_sprite, TonyS32 p_x, TonyS32 p_y, TonyS32 p_layer)
{
	DrawRecord* record = m_drawCursor++;
	record->m_sprite = p_sprite;
	record->m_x = p_x;
	record->m_y = p_y;
	record->m_next = m_drawChains[p_layer];
	m_drawChains[p_layer] = record;
}

// Heapsort of the span queue by x position (1-based sift-down).
// Fully implemented, kept as STUB because it compares at 38%: both heap phases,
// the strength-reduced descending walkers and the record swaps match, but the
// temp copies and walker pointers come out in mirrored registers with shifted
// spill slots throughout. Register round-robin / slot-direction family; retest
// with the original compiler vintage.
// STUB: TONY2 0x004152f0
void __fastcall SortSpansByX(RowSegment* p_list, TonyS32 p_count)
{
	RowSegment temp;
	TonyS32 hole;
	TonyS32 child;
	TonyS32 n = p_count;
	TonyS32 i;

	p_list--;

	for (i = p_count / 2 + 1; i > 1;) {
		i--;
		temp = p_list[i];
		hole = i;

		for (child = i * 2; child <= n; child *= 2) {
			if (child < n && p_list[child].m_x < p_list[child + 1].m_x) {
				child++;
			}

			if (temp.m_x >= p_list[child].m_x) {
				break;
			}

			p_list[hole] = p_list[child];
			hole = child;
		}

		p_list[hole] = temp;
	}

	for (; n > 1;) {
		temp = p_list[1];
		p_list[1] = p_list[n];
		p_list[n] = temp;
		n--;
		temp = p_list[1];
		hole = 1;

		for (child = 2; child <= n; child *= 2) {
			if (child < n && p_list[child].m_x < p_list[child + 1].m_x) {
				child++;
			}

			if (temp.m_x >= p_list[child].m_x) {
				break;
			}

			p_list[hole] = p_list[child];
			hole = child;
		}

		p_list[hole] = temp;
	}
}

// Fully implemented, kept as STUB because it compares at 21%: the whole pipeline
// (type-8 gather, span emit, normalize, layer bubble sort, window clip, heapsort)
// aligns call-for-call, but the segment-count/alloc/seam tail diverges
// structurally (the run-list allocation folds into the seam loop differently)
// and every register/slot in this TU region lands phase-shifted. Refine the
// seam section against the diff and retest with the original compiler vintage.
// STUB: TONY2 0x004154b0
void BackgroundRenderer::BuildLandscape()
{
	TonyS32 total = 0;
	TonyS32 row;
	TonyS32 i;
	TonyS32 n;

	m_bufferCount = g_camera->m_mapHeight;
	m_buffers = (BufferSlot*) new TonyU8[g_camera->m_mapHeight * 8];

	for (row = g_camera->m_mapHeight - 1; row >= 0; row--) {
		TonyS32 tile = row / 0x40 * g_camera->m_tileCols;
		TonyS32 count;
		RowSpan* list;
		TonyS32 written;
		TonyS32 start;
		TonyS16 x;

		g_landPieceCount = 0;

		for (; tile < row / 0x40 * g_camera->m_tileCols + g_camera->m_tileCols; tile++) {
			for (i = 0; i < ((MapTile*) g_camera->m_mapData)[tile].m_spawnCount; i++) {
				OverlayData* block = ((MapTile*) g_camera->m_mapData)[tile].m_spawnList[i];

				if (block->m_type == 8) {
					AddLandPiece(block->m_arg3, (TonyS32) block->m_x, (TonyS32) block->m_y, block->m_layer);
				}
			}
		}

		g_segmentCount = 0;

		for (i = 0; i < g_landPieceCount; i++) {
			EmitSpriteRowSegments(
				row,
				0,
				g_landPieces[i].m_x,
				g_landPieces[i].m_y,
				g_landPieces[i].m_sprite,
				g_landPieces[i].m_kind
			);
		}

		count = g_segmentCount;

		for (i = 0; i < count; i++) {
			if (g_segmentQueue[i].m_x < 0) {
				g_segmentQueue[i].m_data += -g_segmentQueue[i].m_x * 2;
				g_segmentQueue[i].m_length += g_segmentQueue[i].m_x;
				g_segmentQueue[i].m_x = 0;
			}
		}

		for (n = count; n > 0; n--) {
			for (i = 1; i < count; i++) {
				if (g_segmentQueue[i - 1].m_sprite > g_segmentQueue[i].m_sprite) {
					RowSegment swap = g_segmentQueue[i];

					g_segmentQueue[i] = g_segmentQueue[i - 1];
					g_segmentQueue[i - 1] = swap;
				}
			}
		}

		g_spanCount = 0;

		for (i = 0; i < count; i++) {
			InsertSpanClipped(g_segmentQueue[i]);
		}

		SortSpansByX(g_spanQueue, g_spanCount);
		n = 1;

		for (i = 1; i < g_spanCount; i++) {
			n++;

			if (g_spanQueue[i - 1].m_x + (TonyU16) g_spanQueue[i - 1].m_length < g_spanQueue[i].m_x) {
				n++;
			}
		}

		total += n;
		m_buffers[row].m_id = n;

		if (n) {
			m_buffers[row].m_buffer = new TonyU8[n * 8 + 0x18];
		}

		start = 0;

		if (g_spanQueue[0].m_x + (TonyU16) g_spanQueue[0].m_length <= 0) {
			start = 0;

			do {
				start++;
			} while (g_spanQueue[start].m_x + (TonyU16) g_spanQueue[start].m_length <= 0);
		}

		list = (RowSpan*) m_buffers[row].m_buffer;
		written = 0;
		x = g_spanQueue[start].m_x;

		if (x > 0) {
			list[0].m_length = x;
			list[0].m_kind = 0;
			list[0].m_data = &g_blankRow;
			written = 1;
		}

		if (g_spanQueue[start].m_x < 0) {
			*(TonyS32*) &list[written] = *(TonyS32*) &g_spanQueue[start];
			list[written].m_data = (void*) g_spanQueue[start].m_data;
			list[written].m_length = g_spanQueue[start].m_x + g_spanQueue[start].m_length;
			*(TonyS32*) &list[written].m_data += -g_spanQueue[start].m_x * 2;
			written++;

			if (g_spanQueue[start].m_x + (TonyU16) g_spanQueue[start].m_length < g_spanQueue[start + 1].m_x) {
				list[written].m_length =
					g_spanQueue[start + 1].m_x - g_spanQueue[start].m_x - (TonyU16) g_spanQueue[start].m_length;
				list[written].m_kind = 0;
				list[written].m_data = &g_blankRow;
				written++;
			}
		}

		for (i = start; i < g_spanCount - 1; i++) {
			*(TonyS32*) &list[written] = *(TonyS32*) &g_spanQueue[i];
			list[written].m_data = (void*) g_spanQueue[i].m_data;
			list[written].m_length = g_spanQueue[i].m_length;
			written++;

			if ((TonyU16) g_spanQueue[i].m_length + g_spanQueue[i].m_x < g_spanQueue[i + 1].m_x) {
				list[written].m_length =
					g_spanQueue[i + 1].m_x - g_spanQueue[i].m_x - (TonyU16) g_spanQueue[i].m_length;
				list[written].m_kind = 0;
				list[written].m_data = &g_blankRow;
				written++;
			}
		}

		*(TonyS32*) &list[written] = *(TonyS32*) &g_spanQueue[g_spanCount - 1];
		list[written].m_data = (void*) g_spanQueue[g_spanCount - 1].m_data;
		list[written].m_length = g_spanQueue[g_spanCount - 1].m_length;
		list[written + 1].m_length = 0x2ee0;
		list[written + 1].m_kind = 0;
		list[written + 1].m_data = &g_blankRow;
	}

	BuildMainTrack();
	m_scrollX = 0;
	m_scrollY = 0;
	RebuildRowRing();
}

// FUNCTION: TONY2 0x004159d0
void __fastcall SceneryInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = SceneryTick;
	p_object->m_drawFn = SceneryDraw;
	p_object->m_destroyFn = SceneryDestroy;
	InitMotion(p_object);
	SetObjectSprite(p_object, p_object->m_ext->m_idleSetR, 5);
}

// FUNCTION: TONY2 0x00415a10
void __fastcall SceneryResolveSprite(GameObject* p_object)
{
	p_object->m_ext->m_idleSetR = g_videoManager->GetSprite(p_object->m_ext->m_idleSetR, 0);

	if (((OverlayData*) p_object->m_head)->m_flags & 0x200) {
		g_videoManager->AddRefSprite(p_object->m_ext->m_idleSetR);
	}
}

// FUNCTION: TONY2 0x00415a50
void __fastcall SceneryDestroy(GameObject* p_object)
{
	if (((OverlayData*) p_object->m_head)->m_flags & 0x200) {
		g_videoManager->ReleaseSprite(p_object->m_ext->m_idleSetR);
	}
}

// FUNCTION: TONY2 0x00415a70
TonyS32 __fastcall SceneryTick(GameObject* p_object)
{
	MoveTick(p_object);
	return 0;
}

// FUNCTION: TONY2 0x00415a80
void __fastcall SceneryDraw(GameObject* p_object)
{
	TonyFloat x;
	TonyFloat y;

	TonyS32 flag = ((TonyU32) ((OverlayData*) p_object->m_head)->m_flags >> 0xd) & 1;
	GetDrawPosition(p_object, &x, &y);
	QueueObjectSprite(
		p_object,
		p_object->m_state->m_sprite,
		(TonyFloat) p_object->m_state->m_boundsMinX + x,
		(TonyFloat) p_object->m_state->m_boundsMinY + y,
		((OverlayData*) p_object->m_head)->m_layer,
		flag
	);
}

// FUNCTION: TONY2 0x00415ae0
void __fastcall InitMotion(GameObject* p_object)
{
	ResetMotion(p_object);
}

// FUNCTION: TONY2 0x00415af0
void __fastcall SetObjectSprite(GameObject* p_object, TonyS32 p_sprite, TonyS32 p_anchor)
{
	p_object->m_state->m_sprite = p_sprite;
	SetSpriteAnchor(p_object, p_anchor);
}

// FUNCTION: TONY2 0x00415b10
void __fastcall SetSpriteAnchor(GameObject* p_object, TonyS32 p_anchor)
{
	if (p_object->m_state->m_sprite == -1) {
		return;
	}

	TonyU16* sprite = g_videoManager->m_sprites[p_object->m_state->m_sprite];

	switch (p_anchor) {
	case 1:
		p_object->m_state->m_boundsMinX = -(sprite[0] >> 1);
		p_object->m_state->m_boundsMinY = -sprite[1];
		break;
	case 0:
		p_object->m_state->m_boundsMinX = -(sprite[0] >> 1);
		p_object->m_state->m_boundsMinY = 0;
		break;
	case 5:
		p_object->m_state->m_boundsMinX = 0;
		p_object->m_state->m_boundsMinY = 0;
		break;
	case 6:
		p_object->m_state->m_boundsMinX = -sprite[0];
		p_object->m_state->m_boundsMinY = 0;
		break;
	case 3:
		p_object->m_state->m_boundsMinX = -sprite[0];
		p_object->m_state->m_boundsMinY = -(sprite[1] >> 1);
		break;
	default:
		p_object->m_state->m_boundsMinX = -(sprite[0] >> 1);
		p_object->m_state->m_boundsMinY = -(sprite[1] >> 1);
		break;
	}

	p_object->m_state->m_boundsMaxX = p_object->m_state->m_boundsMinX + sprite[0];
	p_object->m_state->m_boundsMaxY = p_object->m_state->m_boundsMinY + sprite[1];
}

// FUNCTION: TONY2 0x00415bf0
void __fastcall CheckpointInit(GameObject* p_object, GroupTemplate* p_template)
{
	BindGroupTemplate(p_object, p_template);
	p_object->m_tickFn = NullObjectHandler;
	p_object->m_drawFn = NULL;
	p_object->m_destroyFn = CheckpointDestroy;
	CheckpointRegister(p_object);
}

// FUNCTION: TONY2 0x00415c20
void __fastcall CheckpointReinit(GameObject* p_object, GroupTemplate* p_template)
{
	BindGroupTemplate(p_object, p_template);
	RespawnAtCheckpoint(p_object);
}

// FUNCTION: TONY2 0x00415c40
void __fastcall RespawnAtCheckpoint(GameObject* p_object)
{
	TonyFloat x;
	TonyFloat y;

	g_objectManager->m_player->Teleport(((OverlayData*) p_object->m_head)->m_x, ((OverlayData*) p_object->m_head)->m_y);
	g_camera->GetFollowTarget(&x, &y);
	g_camera->SetPosition(x, y);
	g_camera->m_world = ((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_sprite;
	g_camera->m_levelNum = ((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0;
	g_camera->m_bonusLevel = ((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param1;
	g_camera->m_musicTrack = ((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param2;
	g_camera->m_backdrop = ((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param3;
	ObjectSetMoveFlags(p_object, 0, 2);
}

// FUNCTION: TONY2 0x00415cf0
void __fastcall CheckpointRegister(GameObject* p_object)
{
	ResetMotion(p_object);
	ObjectSetFlags(p_object, 0x80, 0);
	g_objectManager->m_spawnPoint = (TonyS32) p_object;
}

// FUNCTION: TONY2 0x00415d20
void __fastcall CheckpointDestroy(GameObject* p_object)
{
	g_objectManager->m_spawnPoint = 0;
	ObjectSetMoveFlags(p_object, 0, 1);
}

// FUNCTION: TONY2 0x00415d40
void __fastcall TextInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = TextTick;
	p_object->m_drawFn = TextDraw;
	p_object->m_destroyFn = TextDestroy;
	TextCreateLabel(p_object);
	TextMeasure(p_object);
}

// FUNCTION: TONY2 0x00415d70
void __fastcall TextReinit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	TextLoadFont(p_object);
}

// FUNCTION: TONY2 0x00415d90
void __fastcall TextDestroy(GameObject* p_object)
{
	TonyS32 i;

	for (i = 1; i < LangGetCharsetSize(); i++) {
		g_videoManager->ReleaseSprite(g_fontSprites[(p_object->m_ext->m_idleSetR << 8) + i]);
	}

	delete (TextLabel*) p_object->m_state->m_prevFrameSet;
}

// FUNCTION: TONY2 0x00415df0
TonyS32 __fastcall TextTick(GameObject* p_object)
{
	TextMeasure(p_object);
	SceneryTick(p_object);
	return 0;
}

// FUNCTION: TONY2 0x00415e10
void __fastcall TextLoadFont(GameObject* p_object)
{
	TonyS32 i;

	for (i = 1; i < LangGetCharsetSize(); i++) {
		g_fontSprites[(p_object->m_ext->m_idleSetR << 8) + i] =
			g_videoManager->GetSprite(g_fontGlyphIds[p_object->m_ext->m_idleSetR][i - 1], 0);
		g_videoManager->AddRefSprite(g_fontSprites[(p_object->m_ext->m_idleSetR << 8) + i]);
	}
}

// FUNCTION: TONY2 0x00415e80
void __fastcall TextCreateLabel(GameObject* p_object)
{
	InitMotion(p_object);

	if (((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0 != -1) {
		p_object->m_state->m_prevFrameSet =
			(TonyS32) new TextLabel(((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0);
	}
	else {
		p_object->m_state->m_prevFrameSet = (TonyS32) new TextLabel();
	}

	p_object->m_state->m_frameSet = 0;
}

// Fully implemented, kept as STUB because it compares at 98%: everything matches
// except the dy/dx glyph-metric locals land at [esp+0x20]/[esp+0x1c] in the original
// and swapped under SP3 (declaration order, split declaration, and function-scope
// hoisting all fail to flip it). Local-slot direction family; retest with the
// original compiler vintage.
// STUB: TONY2 0x00415f30
void __fastcall TextDraw(GameObject* p_object)
{
	TonyS32 x = 0;
	TonyS32 i;

	if (p_object->m_state->m_frameSet & 1) {
		x = -((p_object->m_state->m_boundsMaxX - p_object->m_state->m_boundsMinX) / 2);
	}

	if (p_object->m_state->m_frameSet & 2) {
		x += p_object->m_state->m_boundsMinX - p_object->m_state->m_boundsMaxX;
	}

	for (i = 0; i < ((TextLabel*) p_object->m_state->m_prevFrameSet)->m_text.GetLength(); i++) {
		TonyS32 sprite = g_fontSprites
			[(p_object->m_ext->m_idleSetR << 8) + ((TextLabel*) p_object->m_state->m_prevFrameSet)->m_text[i]];
		TonyS16* header = (TonyS16*) g_videoManager->m_sprites[sprite];
		TonyS32 dy = header[2];
		TonyS32 dx = header[3];

		if (p_object->m_ext->m_screenSpace) {
			g_videoManager->QueueSprite(
				sprite,
				(TonyS32) (floor(p_object->m_state->m_worldX) + x - dx),
				(TonyS32) (floor(p_object->m_state->m_worldY) - dy),
				p_object->m_head->m_layer,
				0
			);
		}
		else {
			g_videoManager->QueueSprite(
				sprite,
				(TonyS32) (floor(p_object->m_state->m_worldX - g_camera->m_x) + x - dx),
				(TonyS32) (floor(p_object->m_state->m_worldY - g_camera->m_y) - dy),
				p_object->m_head->m_layer,
				0
			);
		}

		x += g_videoManager->m_sprites[sprite][0] + 2;
	}
}

// FUNCTION: TONY2 0x00416090
void __fastcall TextMeasure(GameObject* p_object)
{
	TonyS32 i;

	p_object->m_state->m_boundsMinX = 0;
	p_object->m_state->m_boundsMinY = 0;
	p_object->m_state->m_boundsMaxX = 0;
	p_object->m_state->m_boundsMaxY = 0;

	for (i = 0; i < ((TextLabel*) p_object->m_state->m_prevFrameSet)->m_text.GetLength(); i++) {
		TonyS32 sprite = g_fontSprites
			[(p_object->m_ext->m_idleSetR << 8) + ((TextLabel*) p_object->m_state->m_prevFrameSet)->m_text[i]];

		p_object->m_state->m_boundsMaxX += g_videoManager->m_sprites[sprite][0] + 2;
		p_object->m_state->m_boundsMaxY = max(p_object->m_state->m_boundsMaxY, g_videoManager->m_sprites[sprite][1]);
	}
}

// FUNCTION: TONY2 0x00416140
void FormatObjectText(GameObject* p_object, TonyS32 p_kind, ...)
{
	va_list args;

	va_start(args, p_kind);
	((TextLabel*) p_object->m_state->m_prevFrameSet)->FormatString(p_kind, (TonyS32) args);
	TextMeasure(p_object);
}

// FUNCTION: TONY2 0x00416170
void __fastcall DispenserInit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	p_object->m_tickFn = DispenserTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = (void(__fastcall*)(GameObject*)) NoOpHandler;
	DispenserReset(p_object);
}

// FUNCTION: TONY2 0x004161a0
void __fastcall DispenserReset(GameObject* p_object)
{
	EnemyReset(p_object);
	SetDispenserMode(p_object, 6);
	*(TonyS32*) &p_object->m_state->m_touchFn = GetFrameCount(p_object) / 2;
}

// FUNCTION: TONY2 0x004161d0
void __fastcall DispenserReinit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	DispenserResolveSets(p_object);
}

// FUNCTION: TONY2 0x004161f0
void __fastcall DispenserResolveSets(GameObject* p_object)
{
	EnemyResolveBaseSets(p_object);
	p_object->m_ext->m_fallSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_jumpSetL, 2);
	p_object->m_ext->m_jumpSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_jumpSetL, 0);
	g_videoManager->GetFrameSet(0x52, 0);
	g_videoManager->GetFrameSet(0x52, 2);
}

// FUNCTION: TONY2 0x00416250
TonyS32 __fastcall DispenserTick(GameObject* p_object)
{
	switch (p_object->m_state->m_behavior) {
	case 6:
		DispenserSpawn(p_object);
		break;
	case 5:
		EnemyDeathTick(p_object);
		break;
	case 0:
		DispenserIdle(p_object);
		break;
	}

	return 0;
}

// FUNCTION: TONY2 0x00416280
void __fastcall DispenserSpawn(GameObject* p_object)
{
	EnemyBaseTick(p_object);

	if (p_object->m_state->m_frame == *(TonyS32*) &p_object->m_state->m_touchFn &&
		p_object->m_state->m_frameTime == 1.0) {
		OverlayData* block = (OverlayData*) malloc(0x1f4);

		block->m_type = 0x17;
		block->m_x = p_object->m_state->m_worldX;
		block->m_y = p_object->m_state->m_worldY - 60.0f;
		block->m_facing = p_object->m_head->m_facing;
		block->m_layer = 0xfa;
		block->m_arg5 = 0x52;
		block->m_arg12 = 0x52;
		block->m_flags = 2;
		*(TonyFloat*) &block->m_arg3 = 1.0f;
		*(TonyFloat*) &block->m_arg4 = 1.0f;
		block->m_arg6 = 6.0f;
		block->m_arg7 = 14.0f;
		block->m_arg8 = 3.0f;
		block->m_arg9 = 3.0f;
		block->m_arg2 = 0;
		block->m_facing = p_object->m_head->m_facing;
		block->SpawnOnce();

		GameObject* object = g_objectManager->AllocObject();
		InitObjectFromData(object, block);
		g_objectManager->InsertObject(object, 8);
	}

	if (IsAnimationDone(p_object) && p_object->m_state->m_behavior != 5) {
		SetDispenserMode(p_object, 0);
	}
}

// FUNCTION: TONY2 0x00416390
void __fastcall DispenserIdle(GameObject* p_object)
{
	EnemyBaseTick(p_object);

	if (IsAnimationDone(p_object) && p_object->m_state->m_behavior != 5 && g_bouncerCount < 2) {
		SetDispenserMode(p_object, 6);
	}
}

// FUNCTION: TONY2 0x004163d0
void __fastcall NoOpHandler(void*)
{
}

// FUNCTION: TONY2 0x004163e0
void __fastcall SetDispenserMode(GameObject* p_object, TonyS32 p_mode)
{
	SetEnemyState(p_object, p_mode);

	if (p_mode) {
		if (p_mode == 6) {
			if (p_object->m_head->m_facing == 4) {
				SetFrameSet(p_object, p_object->m_ext->m_fallSetR);
			}

			if (p_object->m_head->m_facing == 8) {
				SetFrameSet(p_object, p_object->m_ext->m_jumpSetL);
			}
		}
	}
	else {
		if (p_object->m_head->m_facing == 4) {
			SetFrameSet(p_object, p_object->m_ext->m_walkSetR);
		}

		if (p_object->m_head->m_facing == 8) {
			SetFrameSet(p_object, p_object->m_ext->m_idleSetR);
		}
	}
}

// FUNCTION: TONY2 0x00416450
void __fastcall BackdropInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = NullObjectHandler;
	p_object->m_drawFn = BackdropDraw;
	p_object->m_destroyFn = NULL;
	InitMotion(p_object);
	SetObjectSprite(p_object, p_object->m_ext->m_idleSetR, 5);
}

// FUNCTION: TONY2 0x00416490
void __fastcall SceneryReinit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	SceneryResolveSprite(p_object);
}

// FUNCTION: TONY2 0x004164b0
void __fastcall BackdropDraw(GameObject* p_object)
{
	TonyU16* header = g_videoManager->m_sprites[p_object->m_state->m_sprite];
	TonyS32 width = header[0];
	TonyS32 height = header[1];

	for (TonyS32 y = -((TonyS32) ((TonyFloat) g_camera->m_intY / p_object->m_ext->m_jumpSpeed) % height);
		 y < height + 0x190;
		 y += height) {
		for (TonyS32 x = -((TonyS32) ((TonyFloat) g_camera->m_intX / p_object->m_ext->m_walkSpeed) % width);
			 x < width + 0x280;
			 x += width) {
			g_videoManager->QueueSprite(p_object->m_state->m_sprite, x, y, p_object->m_head->m_layer, 0);
		}
	}
}

void CALLBACK ServiceTimerProc(UINT p_timerId, UINT p_msg, DWORD p_user, DWORD p_dw1, DWORD p_dw2);

// Sound-library memory/timer services (called by the mixer init/shutdown at 0x42bb29/0x42bc2d).

// FUNCTION: TONY2 0x00416590
void* __stdcall ServiceAlloc(TonyU32 p_size, TonyU32 p_unused)
{
	return malloc(p_size);
}

// FUNCTION: TONY2 0x004165a0
void __stdcall ServiceFree(void* p_memory)
{
	free(p_memory);
}

// FUNCTION: TONY2 0x004165b0
TonyBool __fastcall ServiceStartTimer(void (*p_tick)())
{
	MMRESULT timer = timeSetEvent(0x10, 0, (LPTIMECALLBACK) ServiceTimerProc, 0, TIME_PERIODIC);

	g_serviceTimerId = timer;
	g_serviceTick = p_tick;

	if (timer != 0) {
		return TRUE;
	}

	return FALSE;
}

// The sound library passes its tick callback to the stop call as well; the argument is
// unused here.
// FUNCTION: TONY2 0x004165e0
TonyBool __fastcall ServiceStopTimer(void (*p_tick)())
{
	MMRESULT result = timeKillEvent(g_serviceTimerId);

	g_serviceTick = NULL;

	if (result == TIMERR_NOERROR) {
		return TRUE;
	}

	return FALSE;
}

// Multimedia timer trampoline handed to timeSetEvent by ServiceStartTimer.
// FUNCTION: TONY2 0x00416600
void CALLBACK ServiceTimerProc(UINT p_timerId, UINT p_msg, DWORD p_user, DWORD p_dw1, DWORD p_dw2)
{
	g_serviceTick();
}

// FUNCTION: TONY2 0x00416610
void __fastcall MoverInit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	p_object->m_tickFn = MoverTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = NULL;
	MoverReset(p_object);
}

// FUNCTION: TONY2 0x00416640
void __fastcall MoverReset(GameObject* p_object)
{
	EnemyReset(p_object);
	SetMoverMode(p_object, 1);

	if (p_object->m_head->m_facing & 4) {
		SetFrameSet(p_object, p_object->m_ext->m_walkSetR);
	}
	else if (p_object->m_head->m_facing & 8) {
		SetFrameSet(p_object, p_object->m_ext->m_idleSetR);
	}

	p_object->m_state->m_pendingState = 2;
	p_object->m_state->m_patrolStep = p_object->m_ext->m_jumpSetL;

	if (p_object->m_head->m_facing & 3) {
		p_object->m_head->m_facing &= ~0xc;
	}
}

// FUNCTION: TONY2 0x004166b0
TonyS32 __fastcall MoverTick(GameObject* p_object)
{
	switch (p_object->m_state->m_behavior) {
	case 5:
		EnemyDeathTick(p_object);
		break;
	case 1:
		MoverAdvance(p_object);
		break;
	}

	return 0;
}

// FUNCTION: TONY2 0x004166e0
void __fastcall MoverAdvance(GameObject* p_object)
{
	if (p_object->m_state->m_frame == 0) {
		PlayObjectSound(p_object, 4, -1, -1);
	}

	if (p_object->m_head->m_facing & 4) {
		p_object->m_state->m_velX = -p_object->m_ext->m_walkSpeed;
	}

	if (p_object->m_head->m_facing & 8) {
		p_object->m_state->m_velX = p_object->m_ext->m_walkSpeed;
	}

	if (p_object->m_head->m_facing & 1) {
		p_object->m_state->m_velY = -p_object->m_ext->m_walkSpeed;
	}

	if (p_object->m_head->m_facing & 2) {
		p_object->m_state->m_velY = p_object->m_ext->m_walkSpeed;
	}

	p_object->m_state->m_patrolStep = (TonyS32) (p_object->m_state->m_patrolStep - p_object->m_ext->m_walkSpeed);

	if (p_object->m_state->m_patrolStep <= 0) {
		MoverReverse(p_object, 0);
	}

	EnemyBaseTick(p_object);
}

// Fully implemented, kept as STUB because it compares at 82%: all four mirror
// branches match, but the original zeroes edx (the dead mode register) and pushes
// it for the 0.0f dx/dy arguments where SP3 pushes immediate 0. Zero-register
// seeding family (fingerprint b); retest with the original compiler vintage.
// STUB: TONY2 0x00416790
void __fastcall MoverReverse(GameObject* p_object, TonyFloat p_amount)
{
	if (p_object->m_head->m_facing & 4) {
		MoveObject(p_object, 0, p_amount, 0);
		p_object->m_head->m_facing = 8;
		SetFrameSet(p_object, p_object->m_ext->m_idleSetR);
	}
	else if (p_object->m_head->m_facing & 8) {
		MoveObject(p_object, 0, -p_amount, 0);
		p_object->m_head->m_facing = 4;
		SetFrameSet(p_object, p_object->m_ext->m_walkSetR);
	}

	if (p_object->m_head->m_facing & 1) {
		MoveObject(p_object, 0, 0, p_amount);
		p_object->m_head->m_facing = 2;
	}
	else if (p_object->m_head->m_facing & 2) {
		MoveObject(p_object, 0, 0, -p_amount);
		p_object->m_head->m_facing = 1;
	}

	p_object->m_state->m_patrolStep = p_object->m_ext->m_jumpSetL;
}

// FUNCTION: TONY2 0x00416850
void __fastcall SetMoverMode(GameObject* p_object, TonyS32 p_mode)
{
	SetEnemyState(p_object, p_mode);

	switch (p_mode) {
	case 1:
		if (p_object->m_head->m_facing == 4) {
			SetFrameSet(p_object, p_object->m_ext->m_walkSetR);
		}

		if (p_object->m_head->m_facing == 8) {
			SetFrameSet(p_object, p_object->m_ext->m_idleSetR);
		}
		break;
	}
}

// FUNCTION: TONY2 0x00416890
void __fastcall BindTemplate(GameObject* p_object, ObjectTemplate* p_template)
{
	*p_object->m_head = p_template->m_head;
	p_object->m_state = (GameObject::State*) (p_object->m_head + 1);
	p_object->m_ext = &p_template->m_ext;
	p_object->m_state->m_template = p_template;
}

// FUNCTION: TONY2 0x004168c0
void __fastcall WaterInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = WaterTick;
	p_object->m_drawFn = WaterDraw;
	p_object->m_destroyFn = WaterDetach;
	WaterReset(p_object);
}

// FUNCTION: TONY2 0x004168f0
void __fastcall WaterReinit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	WaterAttach(p_object);
}

// FUNCTION: TONY2 0x00416910
void __fastcall WaterReset(GameObject* p_object)
{
	p_object->ResetAnimation();
	ObjectSetFlags(p_object, 0, 2);
}

// FUNCTION: TONY2 0x00416930
void __fastcall WaterAttach(GameObject* p_object)
{
	p_object->ResolveFrameSet();
	g_camera->AddRespawnTemplate(p_object->m_state->m_template);
	g_camera->m_waterY = ((OverlayData*) p_object->m_head)->m_y;
}

// FUNCTION: TONY2 0x00416960
TonyS32 __fastcall WaterTick(GameObject* p_object)
{
	SpriteTick(p_object);
	return 0;
}

// Fully implemented, kept as STUB because it compares at 86%: every guard, the
// tile loop and the glyph draw match, but camX and the width/x pair land in
// swapped stack slots (local-slot direction family) and the loop-head x store
// keeps a spill/reload pair SP3 folds to a single fst (fst-fold family).
// Retest with the original compiler vintage.
// STUB: TONY2 0x00416970
void __fastcall WaterDraw(GameObject* p_object)
{
	TonyFloat camX;
	TonyFloat camY;
	TonyFloat objX;
	TonyFloat objY;
	TonyS32 j;

	g_camera->GetViewOffset(&camX, &camY);
	GetDrawPosition(p_object, &objX, &objY);

	if ((TonyFloat) p_object->m_state->m_boundsMaxY + objY < camY) {
		return;
	}

	if ((TonyFloat) p_object->m_state->m_boundsMinY + objY > camY - (-400.0f) - 1.0f) {
		return;
	}

	TonyS32 width = p_object->m_state->m_boundsMaxX - p_object->m_state->m_boundsMinX;
	TonyFloat fwidth = (TonyFloat) width;
	TonyFloat x;

	for (x = (TonyFloat) (floor(camX / fwidth) * width); x < camX + fwidth - (-640.0f); x += fwidth) {
		for (j = 0;
			 j < g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame].m_partCount;
			 j++) {
			g_videoManager->QueueSprite(
				g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame]
					.m_parts[j]
					.m_sprite,
				(TonyS32) (x - camX +
						   g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame]
							   .m_parts[j]
							   .m_dx),
				(TonyS32) (floor(objY - camY) +
						   g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame]
							   .m_parts[j]
							   .m_dy),
				p_object->m_head->m_layer,
				0
			);
		}
	}
}

// FUNCTION: TONY2 0x00416b50
void __fastcall WaterDetach(GameObject* p_object)
{
	ObjectSetMoveFlags(p_object, 0, 1);
	g_camera->AddRespawnTemplate(p_object->m_state->m_template);
}

// FUNCTION: TONY2 0x00416bf0
void __fastcall TrimTrailingBackslash(char* p_path)
{
	if (p_path[strlen(p_path) - 1] == '\\') {
		p_path[strlen(p_path) - 1] = 0;
	}
}

// FUNCTION: TONY2 0x004170d0
void __fastcall OpenGameArchive(char* p_file)
{
	ArchiveMount(p_file, 0x20);
}

// FUNCTION: TONY2 0x004170e0
void CloseGameArchive()
{
	ArchiveUnmount();
}

// FUNCTION: TONY2 0x00417220
void LangFreeStrings()
{
	TonyS32 i;
	TonyS32 j;

	for (i = 0; i < (TonyS32) sizeOfArray(g_langStrings); i++) {
		if (g_langStrings[i]) {
			for (j = 0; j < g_langStringCounts[i]; j++) {
				delete g_langStrings[i][j];
			}

			delete g_langStrings[i];
		}
	}
}

// FUNCTION: TONY2 0x00417400
TonyS32 LangGetCharsetSize()
{
	return g_charsetCount;
}

// FUNCTION: TONY2 0x00417410
void __fastcall WideToGlyphs(TonyU16* p_src, char* p_dest)
{
	TonyU32 i;

	for (i = 0; i < wcslen((wchar_t*) p_src); i++) {
		p_dest[i] = g_charsetReverse[p_src[i]];
	}

	p_dest[wcslen((wchar_t*) p_src)] = 0;
}

// FUNCTION: TONY2 0x00417590
TonyU16* __fastcall LangGetString(TonyS32 p_string)
{
	LangLoadStrings(g_language);
	return g_langStrings[g_language][p_string];
}

// FUNCTION: TONY2 0x004175b0
void __fastcall WideToAnsi(TonyU16* p_string, char* p_dest, TonyS32 p_size)
{
	char defaultChar = ' ';

	WideCharToMultiByte(0, 0, (LPCWSTR) p_string, -1, p_dest, p_size, &defaultChar, (LPBOOL) &p_size);
}
