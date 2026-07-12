// clang-format off
// gamefile.h pulls in afx.h, which must precede any windows.h inclusion.
#include "gamefile.h"
// clang-format on

#include "camera.h"

#include "dialogbuilder.h"
#include "engine.h"
#include "gameobject.h"
#include "hitbox.h"
#include "inputmanager.h"
#include "movieifaces.h"
#include "objectmanager.h"
#include "registrystore.h"
#include "soundmanager.h"
#include "videomanager.h"

// CLSID_AMovie
// GLOBAL: TONY2 0x0044c8e8
static const GUID g_clsidAMovie = {0x49c47ce5, 0x9ba4, 0x11d0, {0x82, 0x12, 0x00, 0xc0, 0x4f, 0xc3, 0x2c, 0x45}};

// IID_IAMovie
// GLOBAL: TONY2 0x0044c868
static const GUID g_iidIAMovie = {0xbebe595c, 0x9a6f, 0x11d0, {0x8f, 0xde, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d}};

// ActiveMovie video renderer id (installed into the graph with the DirectDraw
// target, fetched back for the IMovieFinder QI in RunMovieGraph (0x412500))
// GLOBAL: TONY2 0x0044c988
static const GUID g_clsidVideoRenderer = {0xa35ff56a, 0x9fda, 0x11d0, {0x8f, 0xdf, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d}};

// ActiveMovie audio renderer id
// GLOBAL: TONY2 0x0044c998
static const GUID g_clsidAudioRenderer = {0xa35ff56b, 0x9fda, 0x11d0, {0x8f, 0xdf, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d}};

// IID_IMovieFinder (QI'd off the video renderer)
// GLOBAL: TONY2 0x0044c928
static const GUID g_iidIMovieFinder = {0xf4104fce, 0x9a70, 0x11d0, {0x8f, 0xde, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d}};

// FUNCTION: TONY2 0x00410e00
Camera::Camera()
{
	m_player = NULL;
	SetPosition(0, 0);
	m_mapData = NULL;
	m_scrollLock = 0;
	m_round = -1;
	ClearTemplates();
	m_waterY = -1.0f;
}

// FUNCTION: TONY2 0x004111b0
void Camera::Update()
{
	m_prevX = m_x;
	m_prevY = m_y;
	m_prevIntX = m_intX;
	m_prevIntY = m_intY;

	if (m_player != NULL) {
		FollowPlayer();
	}

	m_intX = (TonyS32) m_x;
	m_intY = (TonyS32) m_y;
	SpawnEnteredTiles();
}

// Follow easing rate: the camera closes 10% of the remaining distance per
// frame (16px minimum step) in FollowPlayer (0x411200)
// GLOBAL: TONY2 0x0044c6c0
static const TonyFloat g_followRate = 0.1f;

// Spacing between segment display sprites in SegmentDisplayDraw (0x412710)
// GLOBAL: TONY2 0x0044c718
static const TonyFloat g_segmentSpacing = 4.0f;

// Horizontal/vertical follow anchor as a fraction of the 640x400 view
// (GetFollowTarget, 0x411440)
// GLOBAL: TONY2 0x0044c6b8
static const TonyFloat g_followFracX = 0.33f;

// GLOBAL: TONY2 0x0044c6bc
static const TonyFloat g_followFracY = 0.33f;

// GLOBAL: TONY2 0x0044c6d8
static const TonyFloat g_screenWidthF = 640.0f;

// GLOBAL: TONY2 0x0044c6dc
static const TonyFloat g_screenWidthNegF = -640.0f;

// GLOBAL: TONY2 0x0044c6e8
static const TonyFloat g_screenHeightF = 400.0f;

// GLOBAL: TONY2 0x0044c6ec
static const TonyFloat g_screenHeightNegF = -400.0f;

// Fully implemented, kept as STUB because it compares at 78%: the accumulate,
// follow-target, four approach clamps and both bound clamps all match, but the
// 16.0-minimum-step ternaries emit an extra test+branch per clamp where the
// original folds to one fcompp test, and the bound ternaries branch-load where
// the original fstp/flds. fcompp-ternary variant family; retest with the
// original compiler vintage.
// STUB: TONY2 0x00411200
void Camera::FollowPlayer()
{
	TonyFloat x;
	TonyFloat y;

	if (m_scrollLock != 0) {
		return;
	}

	m_scrollX += m_player->m_state->m_worldX - m_player->m_state->m_prevX;
	m_scrollY += m_player->m_state->m_worldY - m_player->m_state->m_prevY;
	GetFollowTarget(&x, &y);
	x += m_followOffsetX;
	y += m_followOffsetY;

	if (x > m_scrollX) {
		TonyFloat step = (x - m_scrollX) * g_followRate;
		m_scrollX += (16.0 > step) ? 16.0 : step;
	}

	if (x < m_scrollX) {
		TonyFloat step = (m_scrollX - x) * g_followRate;
		m_scrollX -= (16.0 > step) ? 16.0 : step;
	}

	if (y > m_scrollY) {
		TonyFloat step = (y - m_scrollY) * g_followRate;
		m_scrollY += (16.0 > step) ? 16.0 : step;
	}

	if (y < m_scrollY) {
		TonyFloat step = (m_scrollY - y) * g_followRate;
		m_scrollY -= (16.0 > step) ? 16.0 : step;
	}

	x = (m_scrollX <= 1.0) ? 1.0f : m_scrollX;
	m_x = x;
	y = (m_scrollY <= 1.0) ? 1.0f : m_scrollY;
	m_y = y;

	TonyFloat maxX = (TonyFloat) (m_mapWidth - 0x280) - 1.0;
	m_x = (maxX < x) ? maxX : x;
	TonyFloat maxY = (TonyFloat) (m_mapHeight - 0x190) - 1.0;
	m_y = (maxY < y) ? maxY : y;
}

// FUNCTION: TONY2 0x00411420
void Camera::AttachPlayer(GameObject* p_object)
{
	m_player = p_object;
	SetFollowOffset(0, 0);
}

// FUNCTION: TONY2 0x00411440
void Camera::GetFollowTarget(TonyFloat* p_x, TonyFloat* p_y)
{
	if (((OverlayData*) m_player->m_head)->m_facing & 8) {
		*p_x = m_player->m_state->m_worldX - g_followFracX * g_screenWidthF;
	}
	else if (((OverlayData*) m_player->m_head)->m_facing & 4) {
		*p_x = m_player->m_state->m_worldX - g_followFracX * g_screenWidthNegF - g_screenWidthF;
	}
	else {
		*p_x = m_player->m_state->m_worldX - 320.0;
	}

	if (((OverlayData*) m_player->m_head)->m_facing & 2) {
		*p_y = m_player->m_state->m_worldY - g_followFracY * g_screenHeightF;
	}
	else if (((OverlayData*) m_player->m_head)->m_facing & 1) {
		*p_y = m_player->m_state->m_worldY - g_followFracY * g_screenHeightNegF - g_screenHeightF;
	}
	else {
		*p_y = m_player->m_state->m_worldY - 280.0;
	}
}

// FUNCTION: TONY2 0x00411500
void Camera::SpawnEnteredTiles()
{
	TonyS32 dx = (m_intX >> 6) - (m_prevIntX >> 6);
	TonyS32 dy = (m_intY >> 6) - (m_prevIntY >> 6);
	TonyS32 line;

	if (dx == 1) {
		for (line = -0x3f; line < 0x1d0; line += 0x40) {
			SpawnTileAt(m_intX + 0x280, min(max(line + m_intY, 0), g_camera->m_mapHeight - 1));
		}
	}

	if (dx == -1) {
		for (line = -0x3f; line < 0x1d0; line += 0x40) {
			SpawnTileAt(m_intX, min(max(line + m_intY, 0), g_camera->m_mapHeight - 1));
		}
	}

	if (dy == 1) {
		for (line = -0x3f; line < 0x2c0; line += 0x40) {
			SpawnTileAt(min(max(line + m_intX, 0), g_camera->m_mapWidth - 1), m_intY + 0x190);
		}
	}

	if (dy == -1) {
		for (line = -0x3f; line < 0x2c0; line += 0x40) {
			SpawnTileAt(min(max(line + m_intX, 0), g_camera->m_mapWidth - 1), m_intY);
		}
	}
}

// FUNCTION: TONY2 0x00411650
void Camera::SetPosition(TonyFloat p_x, TonyFloat p_y)
{
	p_x = (p_x > 0.0) ? p_x : 0.0;
	TonyFloat maxX = (TonyFloat) (m_mapWidth - 0x280);
	p_x = (p_x < maxX) ? p_x : maxX;
	p_y = (p_y > 0.0) ? p_y : 0.0;
	TonyFloat maxY = (TonyFloat) (m_mapHeight - 0x190);
	p_y = (p_y < maxY) ? p_y : maxY;

	m_scrollX = p_x;
	m_scrollY = p_y;
	m_x = p_x;
	m_y = p_y;
	m_prevX = p_x;
	m_prevY = p_y;
	m_intX = (TonyU32) p_x;
	m_intY = (TonyU32) p_y;
	m_prevIntX = m_intX;
	m_prevIntY = m_intY;
}

// Fully implemented, kept as STUB because it compares at 42%: the tile spawn cursor
// matches but SP3 lowers max(p_x, 0) on the stack argument to setle/dec/and where
// the original emits the zero-register cmp/sbb/and (the same macro reaches sbb form
// in SpawnEnteredTiles (0x411500) where the operand is a computed register). Max-lowering variant
// of the boolean-materialization family; retest with the original compiler vintage.
// STUB: TONY2 0x00411730
void Camera::SpawnTileAt(TonyS32 p_x, TonyS32 p_y)
{
	TonyS32 x = min(max(p_x, 0), (TonyS32) (m_mapWidth - 1));
	TonyS32 y = min(max(p_y, 0), (TonyS32) (m_mapHeight - 1));
	TonyS32 tile = (y >> 6) * m_tileCols + (x >> 6);

	for (TonyS32 count = 0; count < ((MapTile*) m_mapData)[tile].m_spawnCount; count++) {
		OverlayData* block = ((MapTile*) m_mapData)[tile].m_spawnList[count];

		if (!(block->m_flags & 1)) {
			block->m_flags |= 1;
			GameObject* object = g_objectManager->AllocObject();
			InitObjectFromData(object, block);
			g_objectManager->InsertObject(object, block->m_reserved0);
		}
	}
}

// Fully implemented, was 100% before the cluster-C/D identifier renames; the
// commutative m_tileCols*m_tileRows load order flipped (whole-TU canonicalization,
// same family as PackRgb565/SpawnOnceAll). Source spelling and local-token probes
// do not flip it back; expected to converge with future TU content changes.
// Re-annotate as FUNCTION when it does. 95.92%.
// STUB: TONY2 0x004117e0
void Camera::LoadMap(char* p_path, TonyS32 p_round)
{
	GameFile file(p_path, 0x8000);
	TonyS32 count;

	if (m_mapData != NULL) {
		free(m_mapData);
	}

	m_waterY = -1.0f;
	ClearTemplates();
	m_round = p_round;

	for (count = 0; count < 7; count++) {
		m_totals[count] = 0;
	}

	file.Read(&m_mapWidth, 0x28);
	TonyS32 size = file.GetLength() - file.GetPosition();
	m_mapData = (char*) malloc(size);
	file.Read(m_mapData, size);

	for (count = 0; count < m_tileCols * m_tileRows; count++) {
		*(TonyS32*) &m_mapData[count * 0x18 + 0xc] += (TonyS32) m_mapData;
		*(TonyS32*) &m_mapData[count * 0x18 + 0x10] += (TonyS32) m_mapData;
		*(TonyS32*) &m_mapData[count * 0x18 + 0x14] += (TonyS32) m_mapData;
	}

	TonyS32* cursor = (TonyS32*) (m_listPoolOffset + m_mapData);

	count = 0;
	while (count < m_listPoolCount) {
		*cursor += (TonyS32) m_mapData;
		count++;
		cursor++;
	}
}

// Bistable whole-TU load-order canonicalization (same family as PackRgb565);
// currently on the matching side - re-park if a later TU edit flips it.
// FUNCTION: TONY2 0x00411920
void Camera::SpawnOnceAll()
{
	char* cursor = &m_mapData[m_objectsOffset];

	for (TonyS32 count = 0; count < m_objectCount; count++) {
		OverlayData* block = (OverlayData*) (cursor + 2);
		TonyS32 length = *(TonyU16*) cursor;

		if (block->m_type == 8) {
			block->m_flags |= 0x2000;
		}

		block->SpawnOnce();
		cursor += length + 2;
	}
}

// Fully implemented, kept as STUB because it compares at 93%: SP3 loads the record
// offset (0x58) before the base pointer (0x5c) regardless of addend spelling, where
// the original loads base-first (SpawnOnceAll (0x411920) reaches the original order in its
// ebp-homed context). Scheduler-chirality family; retest with the original vintage.
// STUB: TONY2 0x00411970
OverlayData* Camera::FindMapObject(TonyS32 p_type)
{
	char* cursor = &m_mapData[m_objectsOffset];

	for (TonyS32 count = 0; count < m_objectCount; count++) {
		OverlayData* block = (OverlayData*) (cursor + 2);
		TonyS32 length = *(TonyU16*) cursor;

		if (block->m_type == p_type) {
			return block;
		}

		cursor += length + 2;
	}

	return NULL;
}

// Fully implemented, kept as STUB because it compares at 97%: the 64px sweep and
// the block spawn cursor match, but the original shares one epilogue between the
// spawn-loop exit and its empty-guard skip where SP3 duplicates it. Epilogue-merge
// layout margin; retest with the original compiler vintage.
// STUB: TONY2 0x004119b0
void Camera::SpawnVisible()
{
	TonyS32 x;
	TonyS32 y;
	TonyS32 i;

	for (y = (TonyS32) (m_y - 64.0f); (TonyFloat) y < m_y - (-464.0f); y += 0x40) {
		for (x = (TonyS32) (m_x - 64.0f); (TonyFloat) x < m_x - (-704.0f); x += 0x40) {
			SpawnTileAt(x, y);
		}
	}

	for (i = 0; i < m_templateCount; i++) {
		OverlayData* block = (OverlayData*) m_templates[i];

		if (!(block->m_flags & 1)) {
			block->m_flags |= 1;
			GameObject* object = g_objectManager->AllocObject();
			InitObjectFromData(object, block);
			g_objectManager->InsertObject(object, block->m_reserved0);
		}
	}
}

// Wall list for the 64px tile at p_x/p_y: *p_count receives the entry count
// (file field 0x04), *p_list the HitBox* array (relocated field 0x10); twin of
// GetFloorsAt (0x411b20). See the MapTile note about the swapped declared types.
// Fully implemented, kept as STUB for the same imul reg,[mem] canonicalization
// margin as its twin; retest with the original compiler vintage.
// FUNCTION: TONY2 0x00411ad0
void Camera::GetWallsAt(TonyS32 p_x, TonyS32 p_y, TonyS32** p_count, TonyS32* p_list)
{
	TonyS32 tile = m_tileCols * (p_y / 0x40) + p_x / 0x40;

	*p_count = ((MapTile*) m_mapData)[tile].m_wallCount;
	*p_list = ((MapTile*) m_mapData)[tile].m_walls;
}

// Floor list for the 64px tile at p_x/p_y: *p_count receives the entry count
// (file field 0x00), *p_list the HitBox* array (relocated field 0x0c). See the
// MapTile note about the swapped declared types.
// FUNCTION: TONY2 0x00411b20
void Camera::GetFloorsAt(TonyS32 p_x, TonyS32 p_y, TonyS32** p_count, TonyS32* p_list)
{
	TonyS32 tile = m_tileCols * (p_y / 0x40) + p_x / 0x40;

	*p_count = ((MapTile*) m_mapData)[tile].m_floorCount;
	*p_list = ((MapTile*) m_mapData)[tile].m_floors;
}

// FUNCTION: TONY2 0x00411b70
void Camera::SetFollowOffset(TonyFloat p_x, TonyFloat p_y)
{
	m_followOffsetX = p_x;
	m_followOffsetY = p_y;
}

// FUNCTION: TONY2 0x00411b90
void Camera::LockScroll()
{
	m_scrollLock++;
}

// FUNCTION: TONY2 0x00411ba0
void Camera::UnlockScroll()
{
	if (m_scrollLock > 0) {
		m_scrollLock--;
	}
}

// FUNCTION: TONY2 0x00411bb0
void Camera::AddRespawnTemplate(ObjectTemplate* p_template)
{
	m_templates[m_templateCount] = p_template;
	m_templateCount++;
}

// FUNCTION: TONY2 0x00411be0
void Camera::ClearTemplates()
{
	m_templateCount = 0;
}

// FUNCTION: TONY2 0x00411bf0
void Camera::GetViewOffset(TonyFloat* p_xOffset, TonyFloat* p_yOffset)
{
	if (g_objectManager->m_smoothPass == 0) {
		*p_xOffset = m_x;
		*p_yOffset = m_y;
	}
	else {
		*p_xOffset = (m_prevX + m_x) * 0.5;
		*p_yOffset = (m_prevY + m_y) * 0.5;
	}
}

// FUNCTION: TONY2 0x00411c40
DialogBuilder::DialogBuilder()
{
	Init();
}

// FUNCTION: TONY2 0x00411c50
DialogBuilder::~DialogBuilder()
{
	Teardown();
}

// FUNCTION: TONY2 0x00411c60
void DialogBuilder::Init()
{
	m_itemCount = 0;
	m_backdropSlot = -1;
	m_backdropSprite = -1;
	m_hiddenCount = 0;
	m_hiddenSpecs = 0;
	m_selection = 0;
}

// FUNCTION: TONY2 0x00411c90
void DialogBuilder::Teardown()
{
	TonyS32 i;

	ClearBackdrop();

	for (i = 0; i < m_itemCount; i++) {
		if (m_specs[i].m_kind & 4) {
			g_objectManager->FreeObject(m_objects[0x1f - i]);
			free(m_overlays[0x1f - i]);
		}

		g_objectManager->FreeObject(m_objects[i]);
		free(m_overlays[i]);
	}

	m_itemCount = 0;
}

// FUNCTION: TONY2 0x00411d30
void DialogBuilder::Build(DialogItem* p_specs, TonyS32 p_count)
{
	Teardown();
	m_itemCount = p_count;
	m_specs = p_specs;

	for (p_count = 0; p_count < m_itemCount; p_count++) {
		m_overlays[p_count] = (OverlayData*) malloc(0x1f4);
		m_overlays[p_count]->m_type = 5;
		m_overlays[p_count]->m_x = (TonyFloat) m_specs[p_count].m_x;
		m_overlays[p_count]->m_y = (TonyFloat) m_specs[p_count].m_y;
		m_overlays[p_count]->m_flags = 0;
		m_overlays[p_count]->m_facing = 0;
		m_overlays[p_count]->m_layer = 0xff;
		m_overlays[p_count]->m_arg3 = m_specs[p_count].m_flags;
		m_overlays[p_count]->m_arg4 = m_specs[p_count].m_stringId;
		m_overlays[p_count]->m_arg0 = 1;
		m_overlays[p_count]->SpawnOnce();
		m_objects[p_count] = g_objectManager->AllocObject();
		InitObjectFromData(m_objects[p_count], m_overlays[p_count]);
		g_objectManager->InsertObject(m_objects[p_count], 8);

		if (m_specs[p_count].m_kind & 2) {
			m_objects[p_count]->m_state->m_frameSet |= 1;
		}

		if (m_specs[p_count].m_kind & 8) {
			m_objects[p_count]->m_state->m_frameSet |= 2;
		}

		if (m_specs[p_count].m_callback) {
			m_specs[p_count].m_callback(m_objects[p_count], m_specs[p_count].m_tag);
		}

		if (m_specs[p_count].m_kind & 4) {
			m_overlays[0x1f - p_count] = (OverlayData*) malloc(0x1f4);
			m_overlays[0x1f - p_count]->m_type = 8;
			m_overlays[0x1f - p_count]->m_x = (TonyFloat) (m_specs[p_count].m_x - 8);
			m_overlays[0x1f - p_count]->m_y = (TonyFloat) m_specs[p_count].m_y;
			m_overlays[0x1f - p_count]->m_flags = 0;
			m_overlays[0x1f - p_count]->m_facing = 0;
			m_overlays[0x1f - p_count]->m_layer = 0xff;
			m_overlays[0x1f - p_count]->m_flags = 0;
			m_overlays[0x1f - p_count]->m_arg3 = m_specs[p_count].m_tag;
			m_overlays[0x1f - p_count]->m_arg0 = 1;
			m_overlays[0x1f - p_count]->SpawnOnce();
			m_objects[0x1f - p_count] = g_objectManager->AllocObject();
			InitObjectFromData(m_objects[0x1f - p_count], m_overlays[0x1f - p_count]);
			g_objectManager->InsertObject(m_objects[0x1f - p_count], 8);
			SetSpriteAnchor(m_objects[0x1f - p_count], 6);
		}
	}
}

// Fully implemented, kept as STUB because it compares at 92%: rotation guard, arrow
// navigation, both key exits and the sound/volume epilogue all match, but the original
// sinks the confirm-exit's result assignment into a deferred block between the loop's
// bottom test and the common exit, where the recompile keeps it inline and duplicates
// the epilogue. Deferred-break-block scheduling margin; re-annotate when solved.
// STUB: TONY2 0x00411f30
TonyS32 DialogBuilder::Run()
{
	TonyS32 result;
	TonyS32 prevMask;
	TonyU16 keys;

	result = -1;
	SetItemHighlight(m_selection, 1);
	prevMask = g_inputManager->SetEdgeMask(0xffff);

	if (!(m_specs[m_selection].m_kind & 1)) {
		SelectNext(0);
	}

	while (g_videoManager->PumpFrame(2) == 0) {
		g_inputManager->Poll();

		if (g_inputManager->m_buttons & 2) {
			SelectNext(1);
		}

		if (g_inputManager->m_buttons & 1) {
			SelectPrev(1);
		}

		keys = g_inputManager->m_buttons;

		if (keys & 0x10) {
			result = m_selection;
			break;
		}

		if (keys & 0x80) {
			break;
		}

		TickAndDrawObjects();
	}

	g_soundManager->PlaySample(0xd, 0x40, -1);
	g_inputManager->SetEdgeMask(prevMask);
	return result;
}

// FUNCTION: TONY2 0x00412020
void DialogBuilder::SetBackdrop(TonyS32 p_sprite)
{
	ClearBackdrop();
	m_backdropSprite = p_sprite;
	m_backdropOverlay = (OverlayData*) malloc(sizeof(OverlayData));
	m_backdropOverlay->m_type = 8;
	m_backdropOverlay->m_x = 0.0f;
	m_backdropOverlay->m_y = 0.0f;
	m_backdropOverlay->m_flags = 0;
	m_backdropOverlay->m_facing = 0;
	m_backdropOverlay->m_layer = 0xff;
	m_backdropOverlay->m_flags = 0;
	m_backdropOverlay->m_arg3 = p_sprite;
	m_backdropOverlay->m_arg0 = 1;
	m_backdropOverlay->SpawnOnce();
	m_backdropObject = g_objectManager->AllocObject();
	InitObjectFromData(m_backdropObject, m_backdropOverlay);
	g_objectManager->InsertObject(m_backdropObject, 0xe);
	m_backdropSlot = m_backdropObject->m_ext->m_idleSetR;
}

// FUNCTION: TONY2 0x00412100
void DialogBuilder::ClearBackdrop()
{
	if (m_backdropSlot != -1) {
		g_objectManager->FreeObject(m_backdropObject);
		free(m_backdropOverlay);
		g_videoManager->FreeSprite(m_backdropSlot, 0);
		m_backdropSlot = -1;
	}
}

// FUNCTION: TONY2 0x00412150
void DialogBuilder::SetItemHighlight(TonyS32 p_index, TonyS32 p_on)
{
	if (p_on == 1) {
		m_objects[p_index]->m_head->m_flags |= 0x40;
	}
	else {
		m_objects[p_index]->m_head->m_flags &= 0xffffffbf;
	}
}

// FUNCTION: TONY2 0x00412180
void DialogBuilder::SelectNext(TonyS32 p_playSound)
{
	TonyS32 index;

	if (p_playSound == 1) {
		g_soundManager->PlaySample(0xe, 0x40, -1);
	}

	index = m_selection;

	do {
		index = (index + 1) % m_itemCount;
	} while (!(m_specs[index].m_kind & 1));

	SetSelection(index);
}

// FUNCTION: TONY2 0x004121d0
void DialogBuilder::SelectPrev(TonyS32 p_playSound)
{
	TonyS32 index;

	if (p_playSound == 1) {
		g_soundManager->PlaySample(0xe, 0x40, -1);
	}

	index = m_selection;

	do {
		index = (index + m_itemCount - 1) % m_itemCount;
	} while (!(m_specs[index].m_kind & 1));

	SetSelection(index);
}

// FUNCTION: TONY2 0x00412230
TonyS32 DialogBuilder::GetItemTag(TonyS32 p_index)
{
	return m_specs[p_index].m_tag;
}

// FUNCTION: TONY2 0x00412250
void DialogBuilder::Hide()
{
	m_hiddenCount = m_itemCount;
	m_hiddenSpecs = m_specs;
	Teardown();
}

// FUNCTION: TONY2 0x00412270
void DialogBuilder::Show()
{
	Build(m_hiddenSpecs, m_hiddenCount);
	SetBackdrop(m_backdropSprite);
}

// FUNCTION: TONY2 0x004122a0
void DialogBuilder::RefreshItem(TonyS32 p_index)
{
	GameObject* object = m_objects[p_index];

	SetObjectText(object, m_specs[p_index].m_stringId);

	if (m_specs[p_index].m_kind & 2) {
		object->m_state->m_frameSet |= 1;
	}

	if (m_specs[p_index].m_callback) {
		m_specs[p_index].m_callback(object, m_specs[p_index].m_tag);
	}
}

// FUNCTION: TONY2 0x00412300
void DialogBuilder::SetSelection(TonyS32 p_index)
{
	SetItemHighlight(m_selection, 0);
	m_selection = p_index;
	SetItemHighlight(p_index, 1);
}

// FUNCTION: TONY2 0x00412330
void TickAndDrawObjects()
{
	g_objectManager->TickAll();
	g_objectManager->DrawAll();
}

// FUNCTION: TONY2 0x00412350
void DialogBuilder::Present()
{
	TickAndDrawObjects();
	g_videoManager->PumpFrame(2);
}

// Type 0xc init: plain animated sprite with the standard motion tick; unused
// by the shipped maps (no type-0xc records and no runtime spawns found).
// FUNCTION: TONY2 0x00412370
void __fastcall PropInit(GameObject* p_object, PlayerTemplate* p_template)
{
	BindPlayerTemplate(p_object, p_template);
	p_object->m_tickFn = WaterTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = NULL;
	ResetObjectAnimation(p_object);
}

// Fully implemented, kept as STUB because it compares at 71%: the speed-cap min
// ternary hits the fcompp-min fingerprint (c) - the original emits
// fld B/fld st(1)/fcompp with B a struct member, a shape SP3 only reaches for
// locals/args (it emits fcom [mem] here); the arg-forward movs also mirror
// (round-robin phase). Retest with the original compiler vintage.
// STUB: TONY2 0x004123a0
void GameObject::ApplyGravity(TonyFloat p_x, TonyFloat p_y, TonyFloat p_dx, TonyFloat p_dy)
{
	if (!(m_head->m_flags & 4)) {
		TonyFloat speed = g_objectManager->m_gravity + m_state->m_velY;

		m_state->m_velY = (speed >= m_ext->m_jumpSpeed) ? m_ext->m_jumpSpeed : speed;
	}

	Decelerate(p_x, p_y, p_dx, p_dy);

	if (((PlayerTemplate::Head*) m_head)->m_mapNode == 0) {
		m_head->m_facing &= ~3;
	}
}

// FUNCTION: TONY2 0x00412410
TonyS32 __fastcall CreateMovieGraph(char* p_file, LPDIRECTDRAW p_dd, IMovieGraph** p_graph)
{
	IMovieGraph* graph;
	WCHAR path[0x104];
	TonyS32 result;

	*p_graph = NULL;

	if ((result = CoCreateInstance(g_clsidAMovie, NULL, CLSCTX_INPROC_SERVER, g_iidIAMovie, (LPVOID*) &graph)) >= 0 &&
		(result = graph->Setup(0, 0, 0)) >= 0 && (result = graph->AddRenderer(p_dd, g_clsidVideoRenderer, 0, 0)) >= 0 &&
		(result = graph->AddRenderer(NULL, g_clsidAudioRenderer, 1, 0)) >= 0) {
		MultiByteToWideChar(CP_ACP, 0, p_file, -1, (LPWSTR) path, 0x104);

		if ((result = graph->RenderFile(path, 0)) >= 0) {
			*p_graph = graph;
			graph->AddRef();
		}
	}

	if (graph) {
		graph->Release();
	}

	return result;
}

// FUNCTION: TONY2 0x00412500
TonyS32 __fastcall RunMovieGraph(
	LPDIRECTDRAW p_dd,
	LPDIRECTDRAWSURFACE p_surface,
	IMovieGraph* p_graph,
	TonyS32 (*p_callback)()
)
{
	IUnknown* renderer;
	IMovieFinder* finder;
	IMovieSurface* movieSurface;
	LPDIRECTDRAWSURFACE source;
	RECT rect;
	TonyS32 result;

	renderer = NULL;
	finder = NULL;
	source = NULL;
	movieSurface = NULL;

	if ((result = p_graph->GetFilter(g_clsidVideoRenderer, &renderer)) >= 0 &&
		(result = renderer->QueryInterface(g_iidIMovieFinder, (void**) &finder)) >= 0 &&
		(result = finder->GetSurfaceProvider(0, 0, 0, &movieSurface)) >= 0 &&
		(result = movieSurface->GetSurface(&source, &rect)) >= 0 && (result = p_graph->Run(1)) >= 0) {
		while (movieSurface->Poll(0, 0, 0, 0) == 0) {
			if (p_callback && p_callback() == 1) {
				break;
			}

			p_surface->Blt(NULL, source, &rect, DDBLT_WAIT, NULL);
		}
	}

	if (renderer) {
		renderer->Release();
	}

	if (finder) {
		finder->Release();
	}

	if (movieSurface) {
		movieSurface->Release();
	}

	if (source) {
		source->Release();
	}

	return result;
}

// FUNCTION: TONY2 0x00412630
TonyS32 __fastcall PlayMovie(LPDIRECTDRAW p_dd, LPDIRECTDRAWSURFACE p_surface, char* p_file, TonyS32 (*p_callback)())
{
	IMovieGraph* graph;
	TonyS32 result;

	CoInitialize(NULL);

	if (CreateMovieGraph(p_file, p_dd, &graph) >= 0) {
		TonyS32 r = RunMovieGraph(p_dd, p_surface, graph, p_callback);
		graph->Release();
		result = r >= 0 ? 0 : 2;
	}
	else {
		result = 1;
	}

	CoUninitialize();
	return result;
}

// Type 0xe init: strip of up to four on/off sprite pairs laid out horizontally
// or vertically (world-map progress and key displays); toggled through
// SetSegment (0x412910) / GetSegmentMask (0x412950).
// FUNCTION: TONY2 0x004126b0
void __fastcall SegmentDisplayInit(GameObject* p_object, PlayerTemplate* p_template)
{
	BindPlayerTemplate(p_object, p_template);
	p_object->m_tickFn = SegmentDisplayTick;
	p_object->m_drawFn = SegmentDisplayDraw;
	p_object->m_destroyFn = SegmentDisplayDestroy;
	SegmentDisplaySetup(p_object);
}

// FUNCTION: TONY2 0x004126e0
void __fastcall SegmentDisplayReinit(GameObject* p_object, PlayerTemplate* p_template)
{
	BindPlayerTemplate(p_object, p_template);
	NoOpHandler(p_object);
}

// FUNCTION: TONY2 0x00412700
TonyS32 __fastcall SegmentDisplayTick(GameObject* p_object)
{
	SceneryTick(p_object);
	return 0;
}

// Fully implemented, kept as STUB because it compares at 84%: byte-equivalent logic
// but SP3 homes the object pointer in edi where the original uses ebx, renaming
// every dependent operand. Register-seeding variant family; retest with the
// original compiler vintage.
// STUB: TONY2 0x00412710
void __fastcall SegmentDisplayDraw(GameObject* p_object)
{
	TonyFloat x;
	TonyFloat y;
	TonyS32 slot;

	if (p_object->m_ext->m_screenSpace != 0) {
		x = p_object->m_state->m_worldX;
		y = p_object->m_state->m_worldY;
	}
	else {
		x = p_object->m_state->m_worldX - g_camera->m_x;
		y = p_object->m_state->m_worldY - g_camera->m_y;
	}

	for (slot = 0; slot < 4; slot++) {
		TonyS32 sprite;

		if (p_object->m_state->m_frameSet & (1 << slot)) {
			sprite = (&p_object->m_state->m_prevFrameSet)[slot];
		}
		else {
			sprite = (&p_object->m_state->m_prevFrameSet)[slot + 4];
		}

		g_videoManager->QueueSprite(sprite, (TonyS32) x, (TonyS32) y, ((OverlayData*) p_object->m_head)->m_layer, 0);

		if ((&p_object->m_state->m_prevFrameSet)[slot] != -1) {
			TonyU16* dims = g_videoManager->m_sprites[(&p_object->m_state->m_prevFrameSet)[slot]];

			if (((OverlayData*) p_object->m_head)->m_arg0 == 0) {
				x = dims[0] + x + g_segmentSpacing;
			}
			else {
				y = dims[1] + y + g_segmentSpacing;
			}
		}
	}
}

// FUNCTION: TONY2 0x00412820
void __fastcall SegmentDisplaySetup(GameObject* p_object)
{
	InitMotion(p_object);
	p_object->m_state->m_frameSet = 0;

	for (TonyS32 slot = 0; slot < 4; slot++) {
		(&p_object->m_state->m_prevFrameSet)[slot] = -1;
		(&p_object->m_state->m_prevFrameSet)[slot + 4] = -1;
	}
}

// FUNCTION: TONY2 0x00412860
void __fastcall SetSegmentSprites(GameObject* p_object, TonyS32 p_slot, TonyS32 p_onSprite, TonyS32 p_offSprite)
{
	if (p_onSprite != -1) {
		(&p_object->m_state->m_prevFrameSet)[p_slot] = g_videoManager->GetSprite(p_onSprite, 0);

		if (((OverlayData*) p_object->m_head)->m_flags & 0x200) {
			g_videoManager->AddRefSprite((&p_object->m_state->m_prevFrameSet)[p_slot]);
		}
	}
	else {
		(&p_object->m_state->m_prevFrameSet)[p_slot] = -1;
	}

	if (p_offSprite != -1) {
		(&p_object->m_state->m_prevFrameSet)[p_slot + 4] = g_videoManager->GetSprite(p_offSprite, 0);

		if (((OverlayData*) p_object->m_head)->m_flags & 0x200) {
			g_videoManager->AddRefSprite((&p_object->m_state->m_prevFrameSet)[p_slot + 4]);
		}
	}
	else {
		(&p_object->m_state->m_prevFrameSet)[p_slot + 4] = -1;
	}
}

// FUNCTION: TONY2 0x00412910
void __fastcall SetSegment(GameObject* p_object, TonyS32 p_index, TonyS32 p_on)
{
	if (p_on == 1) {
		p_object->m_state->m_frameSet |= 1 << p_index;
	}
	else {
		p_object->m_state->m_frameSet &= ~(1 << p_index);
	}
}

// FUNCTION: TONY2 0x00412950
TonyS32 __fastcall GetSegmentMask(GameObject* p_object)
{
	return p_object->m_state->m_frameSet;
}

// FUNCTION: TONY2 0x00412960
void __fastcall SegmentDisplayDestroy(GameObject* p_object)
{
	TonyS32 i;

	if (p_object->m_head->m_flags & 0x200) {
		for (i = 0; i < 4; i++) {
			if ((&p_object->m_state->m_prevFrameSet)[i] != -1) {
				g_videoManager->AddRefSprite((&p_object->m_state->m_prevFrameSet)[i]);
			}
		}

		for (i = 0; i < 4; i++) {
			if ((&p_object->m_state->m_prevFrameSet)[i + 4] != -1) {
				g_videoManager->AddRefSprite((&p_object->m_state->m_prevFrameSet)[i + 4]);
			}
		}
	}
}

// Type 0x1a init: one-shot water-splash effect - SplashTick (0x4129f0) kills
// the object (layer -1) after the last frame. Spawned by the water-line
// crossing code (videomanager.cpp 0x40aad0).
// FUNCTION: TONY2 0x004129c0
void __fastcall SplashInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = SplashTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = SplashDestroy;
	SplashSetup(p_object);
}

// FUNCTION: TONY2 0x004129f0
TonyS32 __fastcall SplashTick(GameObject* p_object)
{
	SpriteTick(p_object);

	if (p_object->m_state->m_frame == p_object->m_state->m_typeFlags - 1 &&
		p_object->m_state->m_frameTime <= p_object->m_state->m_animSpeed) {
		p_object->m_state->m_tickStatus = -1;
	}

	return 0;
}

// FUNCTION: TONY2 0x00412a30
void __fastcall SplashSetup(GameObject* p_object)
{
	p_object->ResetAnimation();
	p_object->m_state->m_typeFlags = GetFrameCount(p_object);
}

// FUNCTION: TONY2 0x00412a50
void __fastcall SplashDestroy(GameObject* p_object)
{
	ObjectFreeTemplate(p_object);
}

// Type 0xb init: moving platform; carries riders through the 16-slot array at
// State+0xa0 (CollectRiders 0x412b80 / SnapRiders 0x412ba0 here, plus
// ReleaseRiders/PushRiders and the camera path script in engine.cpp).
// FUNCTION: TONY2 0x00412a60
void __fastcall PlatformInit(GameObject* p_object, PlayerTemplate* p_template)
{
	BindPlayerTemplate(p_object, p_template);
	p_object->m_tickFn = PlatformTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = StopCameraScript;
	PlatformSetup(p_object);
}

// FUNCTION: TONY2 0x00412a90
void __fastcall PlatformReinit(GameObject* p_object, PlayerTemplate* p_template)
{
	BindPlayerTemplate(p_object, p_template);
	PlatformPreload(p_object);
}

// FUNCTION: TONY2 0x00412ab0
void __fastcall PlatformPreload(GameObject* p_object)
{
	p_object->ResolveFrameSet();
}

// FUNCTION: TONY2 0x00412ac0
TonyS32 __fastcall PlatformTick(GameObject* p_object)
{
	SnapshotPosition(p_object);
	WaterTick(p_object);
	p_object->m_state->m_behavior = 0;
	CollectRiders(p_object);
	ReleaseRiders(p_object);
	BobTick(p_object);
	RunCameraScript(p_object);
	PushRiders(p_object);
	return 0;
}

// FUNCTION: TONY2 0x00412b10
void __fastcall PlatformSetup(GameObject* p_object)
{
	TonyS32 i;

	ResetObjectAnimation(p_object);

	for (i = 0; i < 0x10; i++) {
		((GameObject**) &p_object->m_state->m_cooldown)[i] = NULL;
	}

	p_object->m_state->m_savedSpawn = 0;
	p_object->m_state->m_surfSound = 0;
	p_object->m_state->m_typeFlags = 1;
	ObjectSetFlags(p_object, 0x4000, 2);
	p_object->m_state->m_behavior = 0;
}

// FUNCTION: TONY2 0x00412b80
void __fastcall CollectRiders(GameObject* p_object)
{
	SnapRiders(p_object, (HitResult*) g_objectManager->m_playerHits, g_objectManager->m_playerHitCount);
}

// Rider-carry sweep: for each hitbox of the platform's current frame, snap every
// buffered contact (HitResult) whose ±8px vertical probe intersects the anchored
// box onto the fence line, mark it active and register it in the 16-slot carry
// array (State+0xa0) with its keep-bit (State+0x9c).
// Fully implemented, kept as STUB because it compares at 35%: all five calls
// (both rect fetchers, the intersect test, the anchor read and the snap) align
// positionally and every gate matches, but the box/probe locals and loop
// walkers land phase-shifted across the whole body (this band's usual register
// round-robin / slot direction). Refine against the diff or retest with the
// original compiler vintage.
// STUB: TONY2 0x00412ba0
void __fastcall SnapRiders(GameObject* p_object, HitResult* p_buffer, TonyS32 p_count)
{
	HitBox worldBox;
	HitBox anchorBox;
	HitBox probe;
	TonyFloat prevX;
	TonyFloat prevY;
	TonyS32 box;
	TonyS32 i;
	TonyS32 slot;
	TonyS32 found;

	for (box = 0;
		 box < g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame].m_hitBoxCount;
		 box++) {
		BuildWorldHitBox(
			p_object,
			&g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame].m_hitBoxes[box],
			&worldBox
		);
		BuildPrevHitBox(
			p_object,
			&g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame].m_hitBoxes[box],
			&anchorBox
		);

		for (i = 0; i < p_count; i++) {
			GameObject* other = p_buffer[i].m_object;

			probe.m_left = other->m_state->m_worldX - 8.0f;
			probe.m_right = other->m_state->m_worldX - -8.0f;
			probe.m_top = probe.m_bottom = (TonyFloat) p_buffer[i].m_bottom + other->m_state->m_worldY;

			if (HitBoxesOverlap(&worldBox, &probe) == 1) {
				if (other->m_head->m_flags & 4) {
					for (slot = 0; slot < 0x10; slot++) {
						if (((GameObject**) &p_object->m_state->m_cooldown)[slot] == other) {
							p_object->m_state->m_behavior |= 1 << slot;
							break;
						}
					}
				}
				else {
					VideoManager::FrameHitBox* fence;

					GetPrevPosition(other, &prevX, &prevY);
					fence = &g_videoManager->m_frameSets[other->m_state->m_frameSet][other->m_state->m_prevFrame]
								 .m_hitBoxes[p_buffer[i].m_index];

					if ((TonyFloat) fence->m_bottom + prevY <= anchorBox.m_top && prevX - 8.0f < anchorBox.m_right &&
						prevX - -8.0f >= anchorBox.m_left && fence->m_kind != -1) {
						MoveObject(
							other,
							0,
							0,
							((TonyFloat) g_videoManager
								 ->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame]
								 .m_hitBoxes[box]
								 .m_top +
							 p_object->m_state->m_worldY) -
								((TonyFloat) fence->m_bottom + other->m_state->m_worldY)
						);
						other->m_state->m_velY = 0;
						other->m_head->m_flags |= 4;
						found = 0;

						for (slot = 0; slot < 0x10; slot++) {
							if (((GameObject**) &p_object->m_state->m_cooldown)[slot] == other) {
								p_object->m_state->m_behavior |= 1 << slot;
								other->m_head->m_flags &= ~0x100;
								found = 1;
								break;
							}
						}

						if (!found) {
							other->m_head->m_flags |= 0x100;

							for (slot = 0; slot < 0x10; slot++) {
								if (((GameObject**) &p_object->m_state->m_cooldown)[slot] == NULL) {
									p_object->m_state->m_behavior |= 1 << slot;
									((GameObject**) &p_object->m_state->m_cooldown)[slot] = other;
									break;
								}
							}
						}

						if (p_object->m_state->m_savedSpawn == 0) {
							p_object->m_state->m_savedSpawn = 1;
						}
					}
				}
			}
		}
	}
}
