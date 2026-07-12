#include "videomanager.h"

#include "backgroundrenderer.h"
#include "camera.h"
#include "engine.h"
#include "gamemanager.h"
#include "gameobject.h"
#include "hitbox.h"
#include "objectmanager.h"
#include "registrystore.h"
#include "soundmanager.h"

#include <stdlib.h>
#include <string.h>

// GLOBAL: TONY2 0x0044c588
static const TonyFloat g_livesHudX = 568.0f;

// GLOBAL: TONY2 0x0044c58c
static const TonyFloat g_livesHudY = 8.0f;

// GLOBAL: TONY2 0x0044c590
static const TonyFloat g_cerealHudX = 8.0f;

// GLOBAL: TONY2 0x0044c594
static const TonyFloat g_cerealHudY = 8.0f;

// GLOBAL: TONY2 0x0044c598
static const TonyFloat g_healthFlashX = 288.0f;

// GLOBAL: TONY2 0x0044c59c
static const TonyFloat g_healthFlashY = 330.0f;

// GLOBAL: TONY2 0x0045c570
TonyU16 g_blueBits;

// GLOBAL: TONY2 0x0045c578
DDSURFACEDESC g_surfaceDesc;

// GLOBAL: TONY2 0x0045c5e8
TonyU16 g_blueTable[0x100];

// GLOBAL: TONY2 0x0045c7e8
TonyU16 g_blueShift;

// GLOBAL: TONY2 0x0045c7ec
TonyU16 g_redBits;

// GLOBAL: TONY2 0x0045c7f0
TonyU16 g_greenBits;

// GLOBAL: TONY2 0x0045c7f4
VideoManager* g_videoManager;

// GLOBAL: TONY2 0x0045c7f8
TonyU16 g_redTable[0x100];

// GLOBAL: TONY2 0x0045c9f8
TonyU16 g_greenTable[0x100];

// GLOBAL: TONY2 0x0045cbf8
TonyU16 g_redShift;

// GLOBAL: TONY2 0x0045cbfc
TonyU16 g_greenShift;

// GLOBAL: TONY2 0x0045cc00
TonyS32 g_videoInitFlag;

// FUNCTION: TONY2 0x00406410
TonyS32 __fastcall HighestBit(TonyU32 p_mask)
{
	TonyS32 i;

	for (i = 0x1f; i >= 0; i--) {
		if (p_mask & (1 << i)) {
			return i;
		}
	}

	return 0;
}

// FUNCTION: TONY2 0x00406430
TonyS32 __fastcall LowestBit(TonyU32 p_mask)
{
	TonyS32 i;

	for (i = 0; i < 0x20; i++) {
		if (p_mask & (1 << i)) {
			return i;
		}
	}

	return 0;
}

// FUNCTION: TONY2 0x00406450
VideoManager::VideoManager()
{
	TonyS32 i;

	m_nodeCount = 0x100;

	for (i = 0; i < (TonyS32) sizeOfArray(m_nodePool); i++) {
		m_nodePool[i] = new DrawNode();
	}

	for (i = 0; i < (TonyS32) sizeOfArray(m_layers); i++) {
		m_layers[i] = new DrawNode();
	}

	for (i = 0; i < (TonyS32) sizeOfArray(m_sprites); i++) {
		m_sprites[i] = NULL;
		m_spriteSlots[i].m_key = -1;
	}

	m_spriteCount = 0;

	for (i = 0; i < (TonyS32) sizeOfArray(m_frameSets); i++) {
		m_frameSets[i] = NULL;
		m_frameSetSlots[i].m_key = -1;
	}

	m_frameSetCount = 0;
	m_reserved2 = 0xff;
	m_reserved0 = 0xff;
	m_pixelCount = 0;
	m_reserved1 = 0;
}

// FUNCTION: TONY2 0x00406570
void VideoManager::Destroy()
{
	TonyS32 i;

	ShutdownVideo(1);

	for (i = 0; i < (TonyS32) sizeOfArray(m_nodePool); i++) {
		DrawNode* object = m_nodePool[i];
		if (object) {
			NoOpHandler(object);
			delete object;
		}
	}

	for (i = 0; i < (TonyS32) sizeOfArray(m_layers); i++) {
		DrawNode* object = m_layers[i];
		if (object) {
			NoOpHandler(object);
			delete object;
		}
	}
}

// Fully implemented, kept as STUB because it compares at 98.21%: the single differing
// instruction is the strength-reduced loop bound (&g_blueTable[0x100]), which reccmp
// renders as the next .bss symbol - g_blueShift in the original layout vs a different
// neighbor in ours. Byte-equivalent; re-annotate when data placement matches.
// STUB: TONY2 0x004065d0
void VideoManager::BuildColorTables()
{
	TonyS32 i;

	for (i = 0; i < 0x100; i++) {
		g_redTable[i] = (i >> (8 - g_redBits)) << g_redShift;
		g_greenTable[i] = (i >> (8 - g_greenBits)) << g_greenShift;
		g_blueTable[i] = (i >> (8 - g_blueBits)) << g_blueShift;
	}
}

// Fully implemented, kept as STUB because it compares at 80%: every call and structure
// matches, but the allocator homes the title argument in ebp (reusing the metrics register)
// and mirrors ecx/edx in the COM call sequences, and the timeGetTime store is interleaved
// into the ShowWindow argument pushes. Same allocator-margin family as RefreshHitBoxes.
// Re-annotate as FUNCTION when a matching form is found.
// STUB: TONY2 0x004066a0
HWND VideoManager::CreateGameWindow(
	HINSTANCE p_hInstance,
	char* p_className,
	char* p_title,
	WNDPROC p_wndProc,
	TonyS32 p_nShowCmd
)
{
	WNDCLASSA wc;
	DDSURFACEDESC desc;
	DDSCAPS caps;
	char buf[0x100];

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = p_wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = p_hInstance;
	wc.hIcon = LoadIconA(p_hInstance, (LPCSTR) 0x6a);
	wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
	wc.lpszMenuName = p_className;
	wc.lpszClassName = p_className;
	wc.hbrBackground = NULL;
	RegisterClassA(&wc);

	m_hWnd = CreateWindowExA(
		WS_EX_TOPMOST,
		p_className,
		p_title,
		WS_POPUP,
		0,
		0,
		GetSystemMetrics(SM_CXFIXEDFRAME) * 2 + 0x280,
		GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION) + 0x190,
		NULL,
		NULL,
		p_hInstance,
		NULL
	);

	if (!m_hWnd) {
		return NULL;
	}

	m_lastFlip = timeGetTime();
	ShowWindow(m_hWnd, p_nShowCmd);
	UpdateWindow(m_hWnd);

	g_videoInitFlag = 0;

	if (DirectDrawCreate(NULL, (LPDIRECTDRAW*) this, NULL)) {
		return NULL;
	}

	if (m_ddraw->QueryInterface(IID_IDirectDraw2, (LPVOID*) &m_ddraw2)) {
		return NULL;
	}

	if (m_ddraw2->SetCooperativeLevel(m_hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN)) {
		return NULL;
	}

	m_displayHeight = 0x190;
	if (m_ddraw2->SetDisplayMode(0x280, 0x190, 0x10, 0, 0)) {
		m_displayHeight = 0x1e0;
		if (m_ddraw2->SetDisplayMode(0x280, 0x1e0, 0x10, 0, 0)) {
			WideToAnsi(LangGetString(0x3e), buf, 0x100);
			MessageBoxA(m_hWnd, buf, NULL, MB_ICONHAND);
			return NULL;
		}
	}

	desc.dwSize = 0x6c;
	desc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	desc.dwBackBufferCount = 1;

	if (m_ddraw2->CreateSurface(&desc, &m_frontSurface, NULL)) {
		return NULL;
	}

	caps.dwCaps = DDSCAPS_BACKBUFFER;
	if (m_frontSurface->GetAttachedSurface(&caps, &m_backSurface)) {
		return NULL;
	}

	memset(&g_surfaceDesc, 0, 0x6c);
	g_surfaceDesc.dwSize = 0x6c;

	if (m_backSurface->GetSurfaceDesc(&g_surfaceDesc)) {
		return NULL;
	}

	g_redShift = LowestBit(g_surfaceDesc.ddpfPixelFormat.dwRBitMask);
	g_redBits = HighestBit(g_surfaceDesc.ddpfPixelFormat.dwRBitMask) + (1 - g_redShift);
	g_greenShift = LowestBit(g_surfaceDesc.ddpfPixelFormat.dwGBitMask);
	g_greenBits = HighestBit(g_surfaceDesc.ddpfPixelFormat.dwGBitMask) + (1 - g_greenShift);
	g_blueShift = LowestBit(g_surfaceDesc.ddpfPixelFormat.dwBBitMask);
	g_blueBits = HighestBit(g_surfaceDesc.ddpfPixelFormat.dwBBitMask) + (1 - g_blueShift);

	memset(&g_surfaceDesc, 0, 0x6c);
	g_surfaceDesc.dwSize = 0x6c;

	g_videoManager->LockBackSurface();
	g_videoManager->UnlockBackSurface();
	ClearScreens();
	BuildColorTables();

	return m_hWnd;
}

// FUNCTION: TONY2 0x00406a10
void VideoManager::ShutdownVideo(TonyS32 p_force)
{
	FreeAllFrameSets(p_force);
	FreeAllSprites(p_force);
	ResetDrawLists();

	if (m_frontSurface) {
		m_frontSurface->Release();
		m_frontSurface = NULL;
	}

	if (m_ddraw2) {
		m_ddraw2->FlipToGDISurface();
		m_ddraw2->Release();
		m_ddraw2 = NULL;
	}

	if (m_ddraw) {
		m_ddraw->Release();
		m_ddraw = NULL;
	}

	m_hWnd = NULL;
}

// FUNCTION: TONY2 0x00406a70
TonyU8* VideoManager::LockBackSurface()
{
	if (m_backSurface->Lock(NULL, &g_surfaceDesc, DDLOCK_WAIT, NULL) == DDERR_SURFACELOST) {
		m_frontSurface->Restore();
		return NULL;
	}

	if (m_displayHeight == 0x190) {
		return (TonyU8*) g_surfaceDesc.lpSurface;
	}

	return (TonyU8*) g_surfaceDesc.lpSurface + g_surfaceDesc.lPitch * 40;
}

// FUNCTION: TONY2 0x00406ad0
void VideoManager::UnlockBackSurface()
{
	if (m_backSurface->Unlock(NULL) == DDERR_SURFACELOST) {
		m_frontSurface->Restore();
	}
}

// FUNCTION: TONY2 0x00406b00
TonyU8* VideoManager::LockFrontSurface()
{
	if (m_frontSurface->Lock(NULL, &g_surfaceDesc, DDLOCK_WAIT, NULL) == DDERR_SURFACELOST) {
		m_frontSurface->Restore();
		return NULL;
	}

	if (m_displayHeight == 0x190) {
		return (TonyU8*) g_surfaceDesc.lpSurface;
	}

	return (TonyU8*) g_surfaceDesc.lpSurface + g_surfaceDesc.lPitch * 40;
}

// FUNCTION: TONY2 0x00406b60
void VideoManager::UnlockFrontSurface()
{
	if (m_frontSurface->Unlock(NULL) == DDERR_SURFACELOST) {
		m_frontSurface->Restore();
	}
}

// FUNCTION: TONY2 0x00406b90
TonyS32 VideoManager::FlipIfDue(TonyS32 p_interval)
{
	DWORD now;

	if (m_hWnd) {
		now = timeGetTime();

		if ((TonyS32) ((now - m_lastFlip) / 14) >= p_interval) {
			m_lastFlip = now;
			m_frontSurface->Flip(NULL, DDFLIP_WAIT);
			return 1;
		}
	}

	return 0;
}

// Fully implemented, kept as STUB because it compares at 66%: the three LUT lookups and
// OR combine match, but implementing PumpFrame later in this TU flipped VC5's
// evaluation order of the commutative OR chain (byte 1 before byte 2); no source
// spelling, statement split, or TU relocation restores byte-2-first. Expected to
// converge as the TU approaches the original's full content. Same whole-TU
// canonicalization family as the Camera::GetViewOffset effective match.
// STUB: TONY2 0x00406bf0
TonyU16 __fastcall PackRgb565(RgbColor p_color)
{
	return g_blueTable[p_color.m_b] | g_redTable[p_color.m_r] | g_greenTable[p_color.m_g];
}

// FUNCTION: TONY2 0x00406c30
void VideoManager::QueueSprite(TonyS32 p_sprite, TonyS32 p_x, TonyS32 p_y, TonyS32 p_layer, TonyS32 p_noRecord)
{
	if (p_sprite == -1) {
		return;
	}

	DrawNode* entry = TakeDrawNode();
	entry->m_x = p_x;
	entry->m_y = p_y;
	entry->m_pixels = m_sprites[p_sprite];
	entry->m_rows = m_spriteRows[p_sprite];
	LinkDrawNode(entry, p_layer);

	if (p_noRecord == 0) {
		g_backgroundRenderer->RecordDraw(p_sprite, p_x, p_y, p_layer);
	}
}

// FUNCTION: TONY2 0x00406ca0
void VideoManager::ResetDrawLists()
{
	TonyS32 i;

	for (i = 0; i < (TonyS32) sizeOfArray(m_layers); i++) {
		m_layers[i]->m_next = NULL;
	}

	m_nodeCount = 0x100;
}

// FUNCTION: TONY2 0x00406cd0
DrawNode* VideoManager::TakeDrawNode()
{
	m_nodeCount--;
	return m_nodePool[m_nodeCount];
}

// FUNCTION: TONY2 0x00406cf0
void VideoManager::LinkDrawNode(DrawNode* p_entry, TonyS32 p_layer)
{
	p_entry->m_next = m_layers[p_layer]->m_next;
	p_entry->m_prev = m_layers[p_layer];

	if (m_layers[p_layer]->m_next != NULL) {
		m_layers[p_layer]->m_next->m_prev = p_entry;
	}

	m_layers[p_layer]->m_next = p_entry;
}

// FUNCTION: TONY2 0x00406d30
void VideoManager::FreeAllSprites(TonyS32 p_force)
{
	TonyS32 i;

	for (i = 0; i < (TonyS32) sizeOfArray(m_sprites); i++) {
		FreeSprite(i, p_force);
	}
}

// FUNCTION: TONY2 0x00406d60
void VideoManager::FreeAllFrameSets(TonyS32 p_force)
{
	TonyS32 i;

	for (i = 0; i < (TonyS32) sizeOfArray(m_frameSets); i++) {
		FreeFrameSet(i, p_force);
	}

	m_frameSetCount = 0;
}

// FUNCTION: TONY2 0x00406d90
void VideoManager::FreeFrameSet(TonyS32 p_frameSet, TonyS32 p_force)
{
	AnimFrame* rec;
	TonyS32 k;

	if (p_frameSet == -1) {
		return;
	}

	if (m_frameSets[p_frameSet] == NULL) {
		return;
	}

	if (m_frameSetSlots[p_frameSet].m_refCount != 0 && p_force != 1) {
		return;
	}

	for (k = 0; m_frameSets[p_frameSet][k].m_duration != -1; k++) {
		rec = &m_frameSets[p_frameSet][k];
		free(rec->m_parts);

		if (rec->m_attachments) {
			free(rec->m_attachments);
		}

		if (rec->m_hitBoxes) {
			free(rec->m_hitBoxes);
		}

		if (rec->m_touchBoxes) {
			free(rec->m_touchBoxes);
		}
	}

	free(m_frameSets[p_frameSet]);
	m_frameSets[p_frameSet] = NULL;
	m_frameSetSlots[p_frameSet].m_key = -1;
}

// FUNCTION: TONY2 0x00407900
TonyS32 VideoManager::PumpFrame(TonyS32 p_interval)
{
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT) {
			return 1;
		}
	}

	while (g_videoManager->FlipIfDue(p_interval) == 0) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				return 1;
			}
		}
	}

	return 0;
}

// FUNCTION: TONY2 0x00407eb0
TonyS32 VideoManager::IsSpriteLoaded(TonyS32 p_key, TonyS32 p_variant)
{
	return FindSprite(p_key, p_variant) != -1;
}

// FUNCTION: TONY2 0x00407ed0
TonyBool32 VideoManager::IsFrameSetLoaded(TonyS32 p_frameSet, TonyS32 p_variant)
{
	return FindFrameSet(p_frameSet, p_variant) != -1;
}

// FUNCTION: TONY2 0x00407ef0
TonyS32 VideoManager::FindSprite(TonyS32 p_key, TonyS32 p_variant)
{
	TonyS32 i;

	for (i = 0; i < (TonyS32) sizeOfArray(m_spriteSlots); i++) {
		if (m_spriteSlots[i].m_key == p_key && m_spriteSlots[i].m_variant == p_variant) {
			return i;
		}
	}

	return -1;
}

// FUNCTION: TONY2 0x00407f20
TonyS32 VideoManager::GetSprite(TonyS32 p_key, TonyS32 p_variant)
{
	if (g_videoManager->IsSpriteLoaded(p_key, p_variant)) {
		return g_videoManager->FindSprite(p_key, p_variant);
	}

	return g_videoManager->LoadSprite(p_key, p_variant);
}

// FUNCTION: TONY2 0x00407f60
TonyS32 VideoManager::FindFrameSet(TonyS32 p_frameSet, TonyS32 p_variant)
{
	TonyS32 i;

	for (i = 0; i < (TonyS32) sizeOfArray(m_frameSetSlots); i++) {
		if (m_frameSetSlots[i].m_key == p_frameSet && m_frameSetSlots[i].m_variant == p_variant) {
			return i;
		}
	}

	return -1;
}

// FUNCTION: TONY2 0x00407f90
TonyS32 VideoManager::GetFrameSet(TonyS32 p_frameSet, TonyS32 p_variant)
{
	if (IsFrameSetLoaded(p_frameSet, p_variant)) {
		return FindFrameSet(p_frameSet, p_variant);
	}

	return LoadFrameSet(p_frameSet, p_variant);
}

// FUNCTION: TONY2 0x00407fd0
void VideoManager::ClearScreens()
{
	TonyU8* surface = g_videoManager->LockBackSurface();
	if (surface) {
		memset(surface, 0, m_displayHeight * 0x500);
	}

	g_videoManager->UnlockBackSurface();
	g_videoManager->PumpFrame(0);

	surface = g_videoManager->LockBackSurface();
	if (surface) {
		memset(surface, 0, m_displayHeight * 0x500);
	}

	g_videoManager->UnlockBackSurface();
}

// FUNCTION: TONY2 0x00408060
void VideoManager::FreeSprite(TonyS32 p_sprite, TonyS32 p_force)
{
	if (m_sprites[p_sprite]) {
		if (m_spriteSlots[p_sprite].m_refCount == 0 || p_force == 1) {
			free(m_sprites[p_sprite]);
			m_sprites[p_sprite] = NULL;
			free(m_spriteRows[p_sprite]);
			m_spriteRows[p_sprite] = NULL;
			m_spriteSlots[p_sprite].m_key = -1;
			m_spriteCount--;
		}
	}
}

// FUNCTION: TONY2 0x004080e0
TonyS32 VideoManager::AllocSpriteSlot()
{
	TonyS32 i;

	for (i = 0; i < 0xbb8; i++) {
		if (m_sprites[i] == 0) {
			m_spriteCount++;
			return i;
		}
	}

	return -1;
}

// FUNCTION: TONY2 0x00408110
TonyS32 VideoManager::AllocFrameSetSlot()
{
	TonyS32 i;

	for (i = 0; i < (TonyS32) sizeOfArray(m_frameSets); i++) {
		if (m_frameSets[i] == NULL) {
			m_frameSetCount++;
			return i;
		}
	}

	return -1;
}

// FUNCTION: TONY2 0x00408140
void VideoManager::FlushPixelQueue()
{
	TonyS32 i;

	for (i = 0; i < m_pixelCount; i++) {
		m_canvas[m_pixelQueue[i].m_y * 640 + m_pixelQueue[i].m_x] = m_pixelQueue[i].m_color;
	}

	m_pixelCount = 0;
}

// FUNCTION: TONY2 0x00408190
void VideoManager::CopyFrontToBack()
{
	TonyU8* src = LockFrontSurface();
	TonyU8* dst = LockBackSurface();

	memcpy(dst, src, 0x7d000);
	UnlockBackSurface();
	UnlockFrontSurface();
}

// FUNCTION: TONY2 0x004081c0
void VideoManager::AddRefSprite(TonyS32 p_sprite)
{
	m_spriteSlots[p_sprite].m_refCount++;
}

// FUNCTION: TONY2 0x004081e0
void VideoManager::ReleaseSprite(TonyS32 p_sprite)
{
	m_spriteSlots[p_sprite].m_refCount--;
}

// FUNCTION: TONY2 0x00408200
void VideoManager::AddRefFrameSet(TonyS32 p_frameSet)
{
	TonyS32 j;
	TonyS32 k;

	m_frameSetSlots[p_frameSet].m_refCount++;

	for (k = 0; m_frameSets[p_frameSet][k].m_duration != -1; k++) {
		for (j = 0; j < m_frameSets[p_frameSet][k].m_partCount; j++) {
			AddRefSprite(m_frameSets[p_frameSet][k].m_parts[j].m_sprite);
		}
	}
}

// FUNCTION: TONY2 0x00408280
void VideoManager::ReleaseFrameSet(TonyS32 p_frameSet)
{
	TonyS32 j;
	TonyS32 k;

	for (k = 0; m_frameSets[p_frameSet][k].m_duration != -1; k++) {
		for (j = 0; j < m_frameSets[p_frameSet][k].m_partCount; j++) {
			ReleaseSprite(m_frameSets[p_frameSet][k].m_parts[j].m_sprite);
		}
	}

	m_frameSetSlots[p_frameSet].m_refCount--;
}

// FUNCTION: TONY2 0x00408300
void VideoManager::AddRefAllSprites()
{
	TonyS32 i;

	for (i = 0; i < 0xbb8; i++) {
		if (m_spriteSlots[i].m_key != -1) {
			AddRefSprite(i);
		}
	}
}

// FUNCTION: TONY2 0x00408330
void VideoManager::ReleaseAllSprites()
{
	TonyS32 i;

	for (i = 0; i < 0xbb8; i++) {
		if (m_spriteSlots[i].m_key != -1) {
			ReleaseSprite(i);
		}
	}
}

// FUNCTION: TONY2 0x00408360
void VideoManager::AddRefAllFrameSets()
{
	TonyS32 i;

	for (i = 0; i < 0x200; i++) {
		if (m_frameSetSlots[i].m_key != -1) {
			AddRefFrameSet(i);
		}
	}
}

// FUNCTION: TONY2 0x00408390
void VideoManager::ReleaseAllFrameSets()
{
	TonyS32 i;

	for (i = 0; i < 0x200; i++) {
		if (m_frameSetSlots[i].m_key != -1) {
			ReleaseFrameSet(i);
		}
	}
}

// FUNCTION: TONY2 0x004083c0
void VideoManager::AddRefEverything()
{
	AddRefAllSprites();
	AddRefAllFrameSets();
}

// FUNCTION: TONY2 0x004083e0
void VideoManager::ReleaseEverything()
{
	ReleaseAllFrameSets();
	ReleaseAllSprites();
}

// FUNCTION: TONY2 0x00408400
DrawNode::DrawNode()
{
	m_prev = NULL;
	m_next = NULL;
	m_pixels = NULL;
}

// FUNCTION: TONY2 0x00408410
void __fastcall BindGroupTemplate(GameObject* p_object, GroupTemplate* p_template)
{
	*(GroupTemplate::Head*) p_object->m_head = p_template->m_head;
	p_object->m_state = (GameObject::State*) ((GroupTemplate::Head*) p_object->m_head + 1);
	p_object->m_ext = &p_template->m_ext;
	p_object->m_state->m_template = (ObjectTemplate*) p_template;
}

// FUNCTION: TONY2 0x00408440
void __fastcall GroupInit(GameObject* p_object, GroupTemplate* p_template)
{
	BindGroupTemplate(p_object, p_template);
	p_object->m_tickFn = GroupTick;
	p_object->m_drawFn = NULL;
	p_object->m_destroyFn = NULL;
	GroupReset(p_object);
}

// FUNCTION: TONY2 0x00408460
void __fastcall GroupReset(GameObject* p_object)
{
	TonyS32 i;

	ResetMotion(p_object);
	p_object->m_state->m_sprite = 0;

	for (i = 0; i < p_object->m_state->m_sprite; i++) {
		p_object->m_state->m_boundsMinX =
			(p_object->m_state->m_boundsMinX < ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMinX)
				? p_object->m_state->m_boundsMinX
				: ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMinX;
		p_object->m_state->m_boundsMinY =
			(p_object->m_state->m_boundsMinY < ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMinY)
				? p_object->m_state->m_boundsMinY
				: ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMinY;
		p_object->m_state->m_boundsMaxX =
			(p_object->m_state->m_boundsMaxX > ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMaxX)
				? p_object->m_state->m_boundsMaxX
				: ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMaxX;
		p_object->m_state->m_boundsMaxY =
			(p_object->m_state->m_boundsMaxY > ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMaxY)
				? p_object->m_state->m_boundsMaxY
				: ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMaxY;
	}
}

// FUNCTION: TONY2 0x004084e0
TonyS32 __fastcall GroupTick(GameObject* p_object)
{
	TonyS32 i;

	MoveTick(p_object);
	p_object->Decelerate(0, 0, -1234.0f, -1234.0f);

	for (i = 0; i < p_object->m_state->m_sprite; i++) {
		p_object->m_state->m_boundsMinX =
			(p_object->m_state->m_boundsMinX < ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMinX)
				? p_object->m_state->m_boundsMinX
				: ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMinX;
		p_object->m_state->m_boundsMinY =
			(p_object->m_state->m_boundsMinY < ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMinY)
				? p_object->m_state->m_boundsMinY
				: ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMinY;
		p_object->m_state->m_boundsMaxX =
			(p_object->m_state->m_boundsMaxX > ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMaxX)
				? p_object->m_state->m_boundsMaxX
				: ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMaxX;
		p_object->m_state->m_boundsMaxY =
			(p_object->m_state->m_boundsMaxY > ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMaxY)
				? p_object->m_state->m_boundsMaxY
				: ((GameObject::State**) &p_object->m_state->m_frameSet)[i]->m_boundsMaxY;
	}

	return 0;
}

// FUNCTION: TONY2 0x00408580
void __fastcall GroupAddChild(GameObject* p_group, GameObject* p_child)
{
	((GameObject**) &p_group->m_state->m_frameSet)[p_group->m_state->m_sprite] = p_child;
	p_child->m_state->m_parent = p_group;
	p_group->m_state->m_sprite++;
}

// FUNCTION: TONY2 0x004085a0
void __fastcall BindCounterTemplate(GameObject* p_object, CounterTemplate* p_template)
{
	*(CounterTemplate::Head*) p_object->m_head = p_template->m_head;
	p_object->m_state = (GameObject::State*) ((CounterTemplate::Head*) p_object->m_head + 1);
	p_object->m_ext = &p_template->m_ext;
	p_object->m_state->m_template = (ObjectTemplate*) p_template;
}

// FUNCTION: TONY2 0x004085d0
void __fastcall CollectibleInit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	p_object->m_tickFn = CollectibleTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = NULL;
	ResetObjectAnimation(p_object);
}

// FUNCTION: TONY2 0x00408600
void __fastcall CollectibleReinit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	CollectibleRegister(p_object);
}

// FUNCTION: TONY2 0x00408620
void __fastcall CollectibleRegister(GameObject* p_object)
{
	p_object->ResolveFrameSet();
	g_camera->m_totals[((CounterTemplate::Head*) p_object->m_head)->m_kind] +=
		((CounterTemplate::Head*) p_object->m_head)->m_value;
}

// FUNCTION: TONY2 0x00408640
TonyS32 __fastcall CollectibleTick(GameObject* p_object)
{
	SpriteTick(p_object);
	CollideWithHitList(
		p_object,
		(HitResult*) g_objectManager->m_playerHits,
		g_objectManager->m_playerHitCount,
		CollectibleTouch
	);
	return 0;
}

// FUNCTION: TONY2 0x00408670
void __fastcall CollectibleTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_hitIndex
)
{
	if (p_other->m_state->m_patrolStep) {
		((void(__fastcall*)(GameObject*, GameObject*, TonyS32))
			 p_other->m_state->m_patrolStep)(p_other, p_object, ((CounterTemplate::Head*) p_object->m_head)->m_kind);
	}
}

// FUNCTION: TONY2 0x00408690
void __fastcall HopperInit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	p_object->m_tickFn = HopperTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = NULL;
	HopperReset(p_object);
}

// FUNCTION: TONY2 0x004086c0
void __fastcall HopperReset(GameObject* p_object)
{
	EnemyReset(p_object);
	SetHopperState(p_object, 0);
	p_object->m_state->m_pendingState = p_object->m_ext->m_hurtSetR;
}

// FUNCTION: TONY2 0x004086f0
void __fastcall HopperReinit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	HopperResolveSets(p_object);
}

// FUNCTION: TONY2 0x00408710
void __fastcall HopperResolveSets(GameObject* p_object)
{
	p_object->m_ext->m_jumpSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_walkSetL, 2);
	p_object->m_ext->m_walkSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_walkSetL, 0);
	p_object->m_ext->m_jumpSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_idleSetR, 2);
	p_object->m_ext->m_idleSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_idleSetR, 0);
	p_object->m_ext->m_fallSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_fallSetR, 2);
	p_object->m_ext->m_fallSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_fallSetR, 0);
}

// FUNCTION: TONY2 0x004087c0
TonyS32 __fastcall HopperTick(GameObject* p_object)
{
	switch (p_object->m_state->m_behavior) {
	case 5:
		EnemyDeathTick(p_object);
		return 0;
	case 3:
		HopperWalk(p_object);
		return 0;
	case 0:
		HopperIdle(p_object);
		break;
	}

	return 0;
}

// FUNCTION: TONY2 0x004087f0
void __fastcall HopperWalk(GameObject* p_object)
{
	EnemyBaseTick(p_object);

	if (p_object->m_head->m_facing == 4) {
		p_object->m_state->m_velX = -p_object->m_ext->m_walkSpeed;
	}

	if (p_object->m_head->m_facing == 8) {
		p_object->m_state->m_velX = p_object->m_ext->m_walkSpeed;
	}

	p_object->ApplyGravity(0, 0, -1234.0f, -1234.0f);
	CollideWithGroundLines(p_object, HopperHitWorld, 0, 0);
	CollideWithMapBoxes(p_object, HopperHitFrame);
}

// FUNCTION: TONY2 0x00408860
void __fastcall HopperIdle(GameObject* p_object)
{
	EnemyBaseTick(p_object);
	p_object->m_state->m_patrolStep--;

	if (p_object->m_state->m_patrolStep <= 0) {
		SetHopperState(p_object, 3);
	}
}

// Fully implemented, kept as STUB because it compares at 98%: the only diff is the
// pushed 0.0f argument taken from the zeroed mode register by the original where
// cl 11.00.7022 pushes an immediate. Zero-register seeding family (see BouncerHitWorld).
// STUB: TONY2 0x004088a0
TonyFloat __fastcall HopperHitFrame(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value)
{
	switch (p_kind) {
	case 8:
		HopperTurnAround(p_object);
		return p_value;
	case 4:
		HopperTurnAround(p_object);
		return p_value;
	case 2:
		if (p_object->m_state->m_behavior != 3) {
			break;
		}

		SetHopperState(p_object, 0);
		MoveObject(p_object, 0, 0.0f, -p_value);
		p_object->m_state->m_pendingState--;

		if (p_object->m_state->m_pendingState <= 0) {
			HopperTurnAround(p_object);
			p_object->m_state->m_pendingState = p_object->m_ext->m_hurtSetR;
		}

		return p_value;
	}

	return 0.0f;
}

// FUNCTION: TONY2 0x00408940
void __fastcall HopperTurnAround(GameObject* p_object)
{
	if (p_object->m_head->m_facing == 4) {
		p_object->m_head->m_facing = 8;
		SetHopperState(p_object, 0);
	}
	else if (p_object->m_head->m_facing == 8) {
		p_object->m_head->m_facing = 4;
		SetHopperState(p_object, 0);
	}

	p_object->m_state->m_pendingState = p_object->m_ext->m_hurtSetR;
}

// FUNCTION: TONY2 0x00408980
void __fastcall SetHopperState(GameObject* p_object, TonyS32 p_state)
{
	SetEnemyState(p_object, p_state);

	switch (p_state) {
	case 3:
		p_object->m_state->m_velY = p_object->m_ext->m_jumpSpeed * -1.25;
		PlayObjectSound(p_object, 7, -1, -1);

		if (p_object->m_head->m_facing == 4) {
			SetFrameSet(p_object, p_object->m_ext->m_jumpSetL);
		}

		if (p_object->m_head->m_facing == 8) {
			SetFrameSet(p_object, p_object->m_ext->m_idleSetR);
		}
		break;
	case 0:
		p_object->m_state->m_patrolStep = p_object->m_ext->m_hurtSetL;
		p_object->SetVelocity(0, 0);

		if (p_object->m_head->m_facing == 4) {
			SetFrameSet(p_object, p_object->m_ext->m_fallSetL);
		}

		if (p_object->m_head->m_facing == 8) {
			SetFrameSet(p_object, p_object->m_ext->m_fallSetR);
		}
		break;
	}
}

// Fully implemented, kept as STUB because it compares at 93%: the only diff is the
// pushed 0.0f argument taken from the zeroed mode register by the original where
// cl 11.00.7022 pushes an immediate. Zero-register seeding family (see BouncerHitWorld).
// STUB: TONY2 0x00408a30
TonyFloat __fastcall HopperHitWorld(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value)
{
	if (p_object->m_state->m_behavior == 3) {
		SetHopperState(p_object, 0);
		MoveObject(p_object, 0, 0.0f, -p_value);
		p_object->m_state->m_pendingState--;

		if (p_object->m_state->m_pendingState <= 0) {
			HopperTurnAround(p_object);
			p_object->m_state->m_pendingState = p_object->m_ext->m_hurtSetR;
		}
	}

	return p_value;
}

// FUNCTION: TONY2 0x00408aa0
GameObject::GameObject()
{
	m_head = (ObjectTemplate::Head*) malloc(0x1f4);
	Clear();
}

// FUNCTION: TONY2 0x00408ac0
void GameObject::FreeDataBlock()
{
	free(m_head);
}

// FUNCTION: TONY2 0x00408ad0
void GameObject::Clear()
{
	m_ext = NULL;
	m_state = NULL;
	m_prev = NULL;
	m_next = NULL;
	m_destroyFn = NULL;
	m_suspendCount = 0;
}

// FUNCTION: TONY2 0x00408af0
void GameObject::Suspend()
{
	m_suspendCount++;
}

// FUNCTION: TONY2 0x00408b00
void GameObject::Resume()
{
	if (m_suspendCount > 0) {
		m_suspendCount--;
	}
}

// The boss family stores per-phase function pointers in the state slots the other
// families use for data (0xac/0xb0).
// FUNCTION: TONY2 0x00408b10
void __fastcall SessionInit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	p_object->m_tickFn = SessionTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = SessionDestroy;
	*(void(__fastcall**)(GameObject*, GameObject*, TonyS32)) & p_object->m_state->m_patrolStep = SessionPickup;
	p_object->m_state->m_touchFn = SessionTouch;
	p_object->m_state->m_hitWorldFn = SessionHitWorld;
	p_object->m_state->m_hitFrameFn = SessionHitFrame;
	SessionCreateHud(p_object);
}

// FUNCTION: TONY2 0x00408b70
void __fastcall SessionReinit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	SessionResolveSets(p_object);
}

// FUNCTION: TONY2 0x00408b90
void __fastcall SessionResolveSets(GameObject* p_object)
{
	p_object->m_ext->m_smacksIdleL = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksIdleR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksIdleL);
	p_object->m_ext->m_smacksIdleR = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksIdleR, 0);
	p_object->m_ext->m_idleSetR = p_object->m_ext->m_smacksIdleR;
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksIdleR);
	p_object->m_ext->m_smacksWalkL = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksWalkR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksWalkL);
	p_object->m_ext->m_smacksWalkR = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksWalkR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksWalkR);
	p_object->m_ext->m_smacksJumpL = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksJumpR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksJumpL);
	p_object->m_ext->m_smacksJumpR = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksJumpR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksJumpR);
	p_object->m_ext->m_smacksFallL = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksFallR, 2);
	p_object->m_ext->m_fallSetL = p_object->m_ext->m_smacksFallL;
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksFallL);
	p_object->m_ext->m_smacksFallR = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksFallR, 0);
	p_object->m_ext->m_fallSetR = p_object->m_ext->m_smacksFallR;
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksFallR);
	p_object->m_ext->m_smacksHurtL = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksHurtR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksHurtL);
	p_object->m_ext->m_smacksHurtR = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksHurtR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksHurtR);
	p_object->m_ext->m_smacksDuckL = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksDuckR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksDuckL);
	p_object->m_ext->m_smacksDuckR = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksDuckR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksDuckR);
	p_object->m_ext->m_smacksRiseL = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksRiseR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksRiseL);
	p_object->m_ext->m_smacksRiseR = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksRiseR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksRiseR);
	p_object->m_ext->m_smacksPose = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksPose, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksPose);
	p_object->m_ext->m_smacksSpecialL = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksSpecialR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksSpecialL);
	p_object->m_ext->m_smacksSpecialR = g_videoManager->GetFrameSet(p_object->m_ext->m_smacksSpecialR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_smacksSpecialR);
	p_object->m_ext->m_smacksPortrait = g_videoManager->GetSprite(p_object->m_ext->m_smacksPortrait, 0);
	g_videoManager->AddRefSprite(p_object->m_ext->m_smacksPortrait);
	p_object->m_ext->m_tonyIdleL = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyIdleR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyIdleL);
	p_object->m_ext->m_tonyIdleR = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyIdleR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyIdleR);
	p_object->m_ext->m_tonyWalkL = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyWalkR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyWalkL);
	p_object->m_ext->m_tonyWalkR = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyWalkR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyWalkR);
	p_object->m_ext->m_tonyJumpL = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyJumpR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyJumpL);
	p_object->m_ext->m_tonyJumpR = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyJumpR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyJumpR);
	p_object->m_ext->m_tonyFallL = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyFallR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyFallL);
	p_object->m_ext->m_tonyFallR = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyFallR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyFallR);
	p_object->m_ext->m_tonyHurtL = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyHurtR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyHurtL);
	p_object->m_ext->m_tonyHurtR = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyHurtR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyHurtR);
	p_object->m_ext->m_tonyDuckL = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyDuckR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyDuckL);
	p_object->m_ext->m_tonyDuckR = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyDuckR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyDuckR);
	p_object->m_ext->m_tonyRiseL = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyRiseR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyRiseL);
	p_object->m_ext->m_tonyRiseR = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyRiseR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyRiseR);
	p_object->m_ext->m_tonyPose = g_videoManager->GetFrameSet(p_object->m_ext->m_tonyPose, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonyPose);
	p_object->m_ext->m_tonySpecialL = g_videoManager->GetFrameSet(p_object->m_ext->m_tonySpecialR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonySpecialL);
	p_object->m_ext->m_tonySpecialR = g_videoManager->GetFrameSet(p_object->m_ext->m_tonySpecialR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_tonySpecialR);
	p_object->m_ext->m_tonyPortrait = g_videoManager->GetSprite(p_object->m_ext->m_tonyPortrait, 0);
	g_videoManager->AddRefSprite(p_object->m_ext->m_tonyPortrait);
	p_object->m_ext->m_cocoIdleL = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoIdleR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoIdleL);
	p_object->m_ext->m_cocoIdleR = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoIdleR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoIdleR);
	p_object->m_ext->m_cocoWalkL = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoWalkR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoWalkL);
	p_object->m_ext->m_cocoWalkR = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoWalkR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoWalkR);
	p_object->m_ext->m_cocoJumpL = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoJumpR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoJumpL);
	p_object->m_ext->m_cocoJumpR = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoJumpR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoJumpR);
	p_object->m_ext->m_cocoFallL = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoFallR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoFallL);
	p_object->m_ext->m_cocoFallR = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoFallR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoFallR);
	p_object->m_ext->m_cocoHurtL = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoHurtR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoHurtL);
	p_object->m_ext->m_cocoHurtR = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoHurtR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoHurtR);
	p_object->m_ext->m_cocoDuckL = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoDuckR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoDuckL);
	p_object->m_ext->m_cocoDuckR = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoDuckR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoDuckR);
	p_object->m_ext->m_cocoRiseL = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoRiseR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoRiseL);
	p_object->m_ext->m_cocoRiseR = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoRiseR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoRiseR);
	p_object->m_ext->m_cocoPose = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoPose, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoPose);
	p_object->m_ext->m_cocoSpecialL = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoSpecialR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoSpecialL);
	p_object->m_ext->m_cocoSpecialR = g_videoManager->GetFrameSet(p_object->m_ext->m_cocoSpecialR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_cocoSpecialR);
	p_object->m_ext->m_cocoPortrait = g_videoManager->GetSprite(p_object->m_ext->m_cocoPortrait, 0);
	g_videoManager->AddRefSprite(p_object->m_ext->m_cocoPortrait);
	p_object->m_ext->m_trioIdleL = g_videoManager->GetFrameSet(p_object->m_ext->m_trioIdleR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioIdleL);
	p_object->m_ext->m_trioIdleR = g_videoManager->GetFrameSet(p_object->m_ext->m_trioIdleR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioIdleR);
	p_object->m_ext->m_trioWalkL = g_videoManager->GetFrameSet(p_object->m_ext->m_trioWalkR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioWalkL);
	p_object->m_ext->m_trioWalkR = g_videoManager->GetFrameSet(p_object->m_ext->m_trioWalkR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioWalkR);
	p_object->m_ext->m_trioJumpL = g_videoManager->GetFrameSet(p_object->m_ext->m_trioJumpR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioJumpL);
	p_object->m_ext->m_trioJumpR = g_videoManager->GetFrameSet(p_object->m_ext->m_trioJumpR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioJumpR);
	p_object->m_ext->m_trioFallL = g_videoManager->GetFrameSet(p_object->m_ext->m_trioFallR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioFallL);
	p_object->m_ext->m_trioFallR = g_videoManager->GetFrameSet(p_object->m_ext->m_trioFallR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioFallR);
	p_object->m_ext->m_trioHurtL = g_videoManager->GetFrameSet(p_object->m_ext->m_trioHurtR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioHurtL);
	p_object->m_ext->m_trioHurtR = g_videoManager->GetFrameSet(p_object->m_ext->m_trioHurtR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioHurtR);
	p_object->m_ext->m_trioDuckL = g_videoManager->GetFrameSet(p_object->m_ext->m_trioDuckR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioDuckL);
	p_object->m_ext->m_trioDuckR = g_videoManager->GetFrameSet(p_object->m_ext->m_trioDuckR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioDuckR);
	p_object->m_ext->m_trioRiseL = g_videoManager->GetFrameSet(p_object->m_ext->m_trioRiseR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioRiseL);
	p_object->m_ext->m_trioRiseR = g_videoManager->GetFrameSet(p_object->m_ext->m_trioRiseR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioRiseR);
	p_object->m_ext->m_trioPose = g_videoManager->GetFrameSet(p_object->m_ext->m_trioPose, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioPose);
	p_object->m_ext->m_trioSpecialL = g_videoManager->GetFrameSet(p_object->m_ext->m_trioSpecialR, 2);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioSpecialL);
	p_object->m_ext->m_trioSpecialR = g_videoManager->GetFrameSet(p_object->m_ext->m_trioSpecialR, 0);
	g_videoManager->AddRefFrameSet(p_object->m_ext->m_trioSpecialR);
	p_object->m_ext->m_trioPortrait = g_videoManager->GetSprite(p_object->m_ext->m_trioPortrait, 0);
	g_videoManager->AddRefSprite(p_object->m_ext->m_trioPortrait);
}

// FUNCTION: TONY2 0x00409a50
void __fastcall SessionDestroy(GameObject* p_object)
{
	OverlayData* block;

	g_videoManager->FreeFrameSet(p_object->m_state->m_hudCerealSet, 1);
	g_videoManager->FreeFrameSet(p_object->m_state->m_hudExtraSet, 1);
	g_videoManager->ReleaseFrameSet(p_object->m_state->m_meterSet3);
	g_videoManager->ReleaseFrameSet(p_object->m_state->m_meterSet2);
	g_videoManager->ReleaseFrameSet(p_object->m_state->m_meterSet1);
	g_objectManager->FreeObject(p_object->m_state->m_livesText);
	g_objectManager->FreeObject(p_object->m_state->m_portrait);
	g_objectManager->FreeObject(p_object->m_state->m_livesGroup);
	g_objectManager->FreeObject(p_object->m_state->m_cerealText);
	g_objectManager->FreeObject(p_object->m_state->m_cerealIcon);
	g_objectManager->FreeObject(p_object->m_state->m_cerealGroup);
	g_objectManager->FreeObject(p_object->m_state->m_popupGroup);
	g_objectManager->FreeObject(p_object->m_state->m_popupIcon);
	g_objectManager->FreeObject((GameObject*) p_object->m_state->m_nutrientGauge);
	g_objectManager->FreeObject(p_object->m_state->m_healthBar);
	g_objectManager->FreeObject(p_object->m_state->m_keyGauge);
	g_objectManager->FreeObject(p_object->m_state->m_bonusTimer);
	free(p_object->m_state->m_livesData);
	free(p_object->m_state->m_portraitData);
	free(p_object->m_state->m_livesGroupData);
	free(p_object->m_state->m_cerealData);
	free(p_object->m_state->m_cerealIconData);
	free(p_object->m_state->m_cerealGroupData);
	free(p_object->m_state->m_popupData);
	free(p_object->m_state->m_popupGroupData);
	free(p_object->m_state->m_nutrientData);
	free(p_object->m_state->m_healthBarData);
	free(p_object->m_state->m_keyData);
	free(p_object->m_state->m_timerData);

	block = (OverlayData*) p_object->m_state->m_savedCamera;

	if (block != NULL) {
		block->FreePayload();
		delete block;
		p_object->m_state->m_savedCamera = NULL;
	}
}

// FUNCTION: TONY2 0x00409cd0
void __fastcall SessionCreateHud(GameObject* p_object)
{
	PlayerActivate(p_object);
	p_object->m_state->m_health = 3;
	p_object->m_state->m_lives = 3;
	p_object->m_state->m_cereals = 0;
	p_object->m_state->m_livesData = (OverlayData*) malloc(0x1f4);
	p_object->m_state->m_livesData->m_type = 5;
	p_object->m_state->m_livesData->m_x = 40.0f;
	p_object->m_state->m_livesData->m_y = 0;
	p_object->m_state->m_livesData->m_flags = 0;
	p_object->m_state->m_livesData->m_facing = 0;
	p_object->m_state->m_livesData->m_layer = 0xff;
	p_object->m_state->m_livesData->m_arg3 = 1;
	p_object->m_state->m_livesData->m_arg4 = 5;
	p_object->m_state->m_livesData->m_arg0 = 1;
	p_object->m_state->m_livesData->m_flags |= 0x200;
	p_object->m_state->m_livesData->SpawnOnce();
	p_object->m_state->m_livesText = g_objectManager->AllocObject();
	InitObjectFromData(p_object->m_state->m_livesText, p_object->m_state->m_livesData);
	g_objectManager->InsertObject(p_object->m_state->m_livesText, 8);
	FormatObjectText(p_object->m_state->m_livesText, 5, p_object->m_state->m_lives);
	ObjectSetFlags(p_object->m_state->m_livesText, 0x80, 0);
	p_object->m_state->m_portraitData = (OverlayData*) malloc(0x1f4);
	p_object->m_state->m_portraitData->m_type = 8;
	p_object->m_state->m_portraitData->m_x = 0;
	p_object->m_state->m_portraitData->m_y = 0;
	p_object->m_state->m_portraitData->m_flags = 0;
	p_object->m_state->m_portraitData->m_facing = 0;
	p_object->m_state->m_portraitData->m_layer = 0xff;
	p_object->m_state->m_portraitData->m_flags = 0;
	p_object->m_state->m_portraitData->m_arg3 = 0x98;
	p_object->m_state->m_portraitData->m_arg0 = 1;
	p_object->m_state->m_portraitData->m_flags |= 0x200;
	p_object->m_state->m_portraitData->SpawnOnce();
	p_object->m_state->m_portrait = g_objectManager->AllocObject();
	InitObjectFromData(p_object->m_state->m_portrait, p_object->m_state->m_portraitData);
	g_objectManager->InsertObject(p_object->m_state->m_portrait, 0xe);
	ObjectSetFlags(p_object->m_state->m_portrait, 0x80, 0);
	p_object->m_state->m_livesGroupData = (OverlayData*) malloc(0x1f4);
	p_object->m_state->m_livesGroupData->m_type = 7;
	p_object->m_state->m_livesGroupData->m_x = g_livesHudX;
	p_object->m_state->m_livesGroupData->m_y = g_livesHudY;
	p_object->m_state->m_livesGroupData->m_flags = 0;
	p_object->m_state->m_livesGroupData->m_reserved0 = 8;
	p_object->m_state->m_livesGroupData->m_facing = 0;
	p_object->m_state->m_livesGroupData->m_layer = 1;
	p_object->m_state->m_livesGroupData->m_arg0 = 0x3f000000;
	p_object->m_state->m_livesGroupData->m_arg1 = 0x3f000000;
	p_object->m_state->m_livesGroup = g_objectManager->AllocObject();
	InitObjectFromData(p_object->m_state->m_livesGroup, p_object->m_state->m_livesGroupData);
	g_objectManager->InsertObject(p_object->m_state->m_livesGroup, 0xe);
	GroupAddChild(p_object->m_state->m_livesGroup, p_object->m_state->m_livesText);
	GroupAddChild(p_object->m_state->m_livesGroup, p_object->m_state->m_portrait);
	ObjectSetFlags(p_object->m_state->m_livesGroup, 0x80, 0);
	p_object->m_state->m_cerealData = (OverlayData*) malloc(0x1f4);
	p_object->m_state->m_cerealData->m_type = 5;
	p_object->m_state->m_cerealData->m_x = 40.0f;
	p_object->m_state->m_cerealData->m_y = 0;
	p_object->m_state->m_cerealData->m_flags = 0;
	p_object->m_state->m_cerealData->m_facing = 0;
	p_object->m_state->m_cerealData->m_layer = 0xff;
	p_object->m_state->m_cerealData->m_arg3 = 1;
	p_object->m_state->m_cerealData->m_arg4 = 5;
	p_object->m_state->m_cerealData->m_arg0 = 1;
	p_object->m_state->m_cerealData->m_flags |= 0x200;
	p_object->m_state->m_cerealData->SpawnOnce();
	p_object->m_state->m_cerealText = g_objectManager->AllocObject();
	InitObjectFromData(p_object->m_state->m_cerealText, p_object->m_state->m_cerealData);
	g_objectManager->InsertObject(p_object->m_state->m_cerealText, 8);
	FormatObjectText(p_object->m_state->m_cerealText, 5, p_object->m_state->m_cereals);
	ObjectSetFlags(p_object->m_state->m_cerealText, 0x80, 0);
	p_object->m_state->m_cerealIconData = (OverlayData*) malloc(0x1f4);
	p_object->m_state->m_cerealIconData->m_type = 9;
	p_object->m_state->m_cerealIconData->m_x = 12.0f;
	p_object->m_state->m_cerealIconData->m_y = 24.0f;
	p_object->m_state->m_cerealIconData->m_flags = 0;
	p_object->m_state->m_cerealIconData->m_facing = 0;
	p_object->m_state->m_cerealIconData->m_layer = 0xff;
	p_object->m_state->m_cerealIconData->m_flags = 0;
	p_object->m_state->m_cerealIconData->m_arg3 = 0x66;
	p_object->m_state->m_cerealIconData->m_arg0 = 1;
	p_object->m_state->m_cerealIconData->m_flags |= 0x200;
	p_object->m_state->m_cerealIconData->SpawnOnce();
	p_object->m_state->m_cerealIcon = g_objectManager->AllocObject();
	InitObjectFromData(p_object->m_state->m_cerealIcon, p_object->m_state->m_cerealIconData);
	g_objectManager->InsertObject(p_object->m_state->m_cerealIcon, 0xe);
	ObjectSetFlags(p_object->m_state->m_cerealIcon, 0x80, 0);
	p_object->m_state->m_cerealGroupData = (OverlayData*) malloc(0x1f4);
	p_object->m_state->m_cerealGroupData->m_type = 7;
	p_object->m_state->m_cerealGroupData->m_x = g_cerealHudX;
	p_object->m_state->m_cerealGroupData->m_y = g_cerealHudY;
	p_object->m_state->m_cerealGroupData->m_flags = 0;
	p_object->m_state->m_cerealGroupData->m_reserved0 = 8;
	p_object->m_state->m_cerealGroupData->m_facing = 0;
	p_object->m_state->m_cerealGroupData->m_layer = 1;
	p_object->m_state->m_cerealGroupData->m_arg0 = 0x3f000000;
	p_object->m_state->m_cerealGroupData->m_arg1 = 0x3f000000;
	p_object->m_state->m_cerealGroup = g_objectManager->AllocObject();
	InitObjectFromData(p_object->m_state->m_cerealGroup, p_object->m_state->m_cerealGroupData);
	g_objectManager->InsertObject(p_object->m_state->m_cerealGroup, 0xe);
	GroupAddChild(p_object->m_state->m_cerealGroup, p_object->m_state->m_cerealText);
	GroupAddChild(p_object->m_state->m_cerealGroup, p_object->m_state->m_cerealIcon);
	ObjectSetFlags(p_object->m_state->m_cerealGroup, 0x80, 0);
	p_object->m_state->m_popupData = (OverlayData*) malloc(0x1f4);
	p_object->m_state->m_popupData->m_type = 8;
	p_object->m_state->m_popupData->m_x = 0;
	p_object->m_state->m_popupData->m_y = 0;
	p_object->m_state->m_popupData->m_flags = 0;
	p_object->m_state->m_popupData->m_facing = 0;
	p_object->m_state->m_popupData->m_layer = 0xff;
	p_object->m_state->m_popupData->m_flags = 0;
	p_object->m_state->m_popupData->m_arg3 = 0x97;
	p_object->m_state->m_popupData->m_arg0 = 1;
	p_object->m_state->m_popupData->m_flags |= 0x200;
	p_object->m_state->m_popupData->SpawnOnce();
	p_object->m_state->m_popupIcon = g_objectManager->AllocObject();
	InitObjectFromData(p_object->m_state->m_popupIcon, p_object->m_state->m_popupData);
	g_objectManager->InsertObject(p_object->m_state->m_popupIcon, 0xe);
	ObjectSetFlags(p_object->m_state->m_popupIcon, 0x80, 0);
	p_object->m_state->m_popupGroupData = (OverlayData*) malloc(0x1f4);
	p_object->m_state->m_popupGroupData->m_type = 7;
	p_object->m_state->m_popupGroupData->m_x = g_healthFlashX;
	p_object->m_state->m_popupGroupData->m_y = g_healthFlashY;
	p_object->m_state->m_popupGroupData->m_flags = 0;
	p_object->m_state->m_popupGroupData->m_reserved0 = 8;
	p_object->m_state->m_popupGroupData->m_facing = 0;
	p_object->m_state->m_popupGroupData->m_layer = 1;
	p_object->m_state->m_popupGroupData->m_arg0 = 0x3f000000;
	p_object->m_state->m_popupGroupData->m_arg1 = 0x3f000000;
	p_object->m_state->m_popupGroup = g_objectManager->AllocObject();
	InitObjectFromData(p_object->m_state->m_popupGroup, p_object->m_state->m_popupGroupData);
	g_objectManager->InsertObject(p_object->m_state->m_popupGroup, 0xe);
	GroupAddChild(p_object->m_state->m_popupGroup, p_object->m_state->m_popupIcon);
	ObjectSetFlags(p_object->m_state->m_popupGroup, 0x80, 0);
	p_object->m_state->m_nutrientData = (OverlayData*) malloc(0x1f4);
	p_object->m_state->m_nutrientData->m_type = 0xe;
	p_object->m_state->m_nutrientData->m_x = 8.0f;
	p_object->m_state->m_nutrientData->m_y = 368.0f;
	p_object->m_state->m_nutrientData->m_flags = 0;
	p_object->m_state->m_nutrientData->m_facing = 0;
	p_object->m_state->m_nutrientData->m_layer = 0xff;
	p_object->m_state->m_nutrientData->m_flags = 0;
	p_object->m_state->m_nutrientData->m_arg0 = 0;
	p_object->m_state->m_nutrientData->m_arg1 = 1;
	p_object->m_state->m_nutrientData->m_flags |= 0x200;
	p_object->m_state->m_nutrientData->SpawnOnce();
	p_object->m_state->m_nutrientGauge = g_objectManager->AllocObject();
	InitObjectFromData((GameObject*) p_object->m_state->m_nutrientGauge, p_object->m_state->m_nutrientData);
	g_objectManager->InsertObject((GameObject*) p_object->m_state->m_nutrientGauge, 0xe);
	SetSegmentSprites((GameObject*) p_object->m_state->m_nutrientGauge, 0, 0x5f8, -1);
	SetSegmentSprites((GameObject*) p_object->m_state->m_nutrientGauge, 1, 0x608, -1);
	SetSegmentSprites((GameObject*) p_object->m_state->m_nutrientGauge, 2, 0x618, -1);
	ObjectSetFlags((GameObject*) p_object->m_state->m_nutrientGauge, 0x80, 0);
	p_object->m_state->m_keyData = (OverlayData*) malloc(0x1f4);
	p_object->m_state->m_keyData->m_type = 0xe;
	p_object->m_state->m_keyData->m_x = 480.0f;
	p_object->m_state->m_keyData->m_y = 368.0f;
	p_object->m_state->m_keyData->m_flags = 0;
	p_object->m_state->m_keyData->m_facing = 0;
	p_object->m_state->m_keyData->m_layer = 0xff;
	p_object->m_state->m_keyData->m_flags = 0;
	p_object->m_state->m_keyData->m_arg0 = 0;
	p_object->m_state->m_keyData->m_arg1 = 1;
	p_object->m_state->m_keyData->m_flags |= 0x200;
	p_object->m_state->m_keyData->SpawnOnce();
	p_object->m_state->m_keyGauge = g_objectManager->AllocObject();
	InitObjectFromData(p_object->m_state->m_keyGauge, p_object->m_state->m_keyData);
	g_objectManager->InsertObject(p_object->m_state->m_keyGauge, 0xe);
	SetSegmentSprites(p_object->m_state->m_keyGauge, 0, 0xaaa, -1);
	SetSegmentSprites(p_object->m_state->m_keyGauge, 1, 0xaa8, -1);
	SetSegmentSprites(p_object->m_state->m_keyGauge, 2, 0x833, -1);
	ObjectSetFlags(p_object->m_state->m_keyGauge, 0x80, 0);
	p_object->m_state->m_meterSet3 = g_videoManager->GetFrameSet(0xaf, 0);
	g_videoManager->AddRefFrameSet(p_object->m_state->m_meterSet3);
	p_object->m_state->m_meterSet2 = g_videoManager->GetFrameSet(0xae, 0);
	g_videoManager->AddRefFrameSet(p_object->m_state->m_meterSet2);
	p_object->m_state->m_meterSet1 = g_videoManager->GetFrameSet(0xab, 0);
	g_videoManager->AddRefFrameSet(p_object->m_state->m_meterSet1);
	p_object->m_state->m_healthBarData = (OverlayData*) malloc(0x1f4);
	p_object->m_state->m_healthBarData->m_type = 9;
	p_object->m_state->m_healthBarData->m_x = 320.0f;
	p_object->m_state->m_healthBarData->m_y = 40.0f;
	p_object->m_state->m_healthBarData->m_flags = 0;
	p_object->m_state->m_healthBarData->m_facing = 0;
	p_object->m_state->m_healthBarData->m_layer = 0xff;
	p_object->m_state->m_healthBarData->m_flags = 0;
	p_object->m_state->m_healthBarData->m_arg3 = 0xaf;
	p_object->m_state->m_healthBarData->m_arg0 = 1;
	p_object->m_state->m_healthBarData->SpawnOnce();
	p_object->m_state->m_healthBar = g_objectManager->AllocObject();
	InitObjectFromData(p_object->m_state->m_healthBar, p_object->m_state->m_healthBarData);
	g_objectManager->InsertObject(p_object->m_state->m_healthBar, 0xe);
	ObjectSetFlags(p_object->m_state->m_healthBar, 0x80, 0);
	p_object->m_state->m_timerData = (OverlayData*) malloc(0x1f4);
	p_object->m_state->m_timerData->m_type = 5;
	p_object->m_state->m_timerData->m_x = 300.0f;
	p_object->m_state->m_timerData->m_y = 48.0f;
	p_object->m_state->m_timerData->m_flags = 0;
	p_object->m_state->m_timerData->m_facing = 0;
	p_object->m_state->m_timerData->m_layer = 0xff;
	p_object->m_state->m_timerData->m_arg3 = 1;
	p_object->m_state->m_timerData->m_arg4 = 5;
	p_object->m_state->m_timerData->m_arg0 = 1;
	p_object->m_state->m_timerData->m_flags |= 0xa00;
	p_object->m_state->m_timerData->SpawnOnce();
	p_object->m_state->m_bonusTimer = g_objectManager->AllocObject();
	InitObjectFromData(p_object->m_state->m_bonusTimer, p_object->m_state->m_timerData);
	g_objectManager->InsertObject(p_object->m_state->m_bonusTimer, 8);
	ObjectSetFlags(p_object->m_state->m_bonusTimer, 0x80, 0);
	p_object->m_state->m_hudCerealSet = -1;
	p_object->m_state->m_hudExtraSet = -1;
	PlayerSetForm(p_object, ((CounterTemplate::Head*) p_object->m_head)->m_value, 1);
	StoreRespawnForm(p_object);
	p_object->m_state->m_livesShowTicks = 0x46;
	p_object->m_state->m_cerealShowTicks = 0x46;
	p_object->m_state->m_popupShowTicks = 0x46;
	PlayerReset(p_object, 1);
}

// Fully implemented, kept as STUB because it compares at 90%: the ceiling logic (the
// case-2-into-default fallthrough), all three splash spawns, the shield timers and the
// music handoff match; the residue is the state pointer cached in edi by the original
// versus edx here (register-seeding family, see TickAll) and two swapped stores in
// the spawn blocks. Re-annotate when the vintage is found.
// STUB: TONY2 0x0040aad0
TonyS32 __fastcall SessionTick(GameObject* p_object)
{
	OverlayData* block;
	GameObject* object;

	SnapshotPosition(p_object);

	if (g_camera->m_waterY != -1.0 && p_object->m_state->m_moveState != 7 && p_object->m_state->m_moveState != 6 &&
		p_object->m_state->m_moveState != 13) {
		switch (((CounterTemplate::Head*) p_object->m_head)->m_value) {
		case 2:
			if (p_object->m_state->m_worldY > g_camera->m_waterY && p_object->m_state->m_moveState != 0xb) {
				MoveObject(p_object, 0, 0.0f, g_camera->m_waterY - p_object->m_state->m_worldY);
				SetPlayerState(p_object, 0xb, 0);
				block = (OverlayData*) malloc(0x1f4);
				block->m_type = 0x1a;
				block->m_x = p_object->m_state->m_worldX;
				block->m_facing = 0;
				block->m_y = g_camera->m_waterY;
				block->m_layer = 0xfe;
				block->m_flags = 0;
				block->m_arg3 = 0xb4;
				block->m_arg0 = 0;
				block->SpawnOnce();
				object = g_objectManager->AllocObject();
				InitObjectFromData(object, block);
				g_objectManager->InsertObject(object, 8);
			}
		default:
			if (p_object->m_state->m_worldY > g_camera->m_waterY) {
				TakeDamage(p_object, 1, 0, 6);
			}
			break;
		case 1:
		case 3:
			if (p_object->m_state->m_worldY > g_camera->m_waterY && p_object->m_state->m_moveState != 0xa) {
				SetPlayerState(p_object, 0xa, 0);
				block = (OverlayData*) malloc(0x1f4);
				block->m_type = 0x1a;
				block->m_x = p_object->m_state->m_worldX;
				block->m_facing = 0;
				block->m_y = g_camera->m_waterY;
				block->m_layer = 0xfe;
				block->m_flags = 0;
				block->m_arg3 = 0xb4;
				block->m_arg0 = 0;
				block->SpawnOnce();
				object = g_objectManager->AllocObject();
				InitObjectFromData(object, block);
				g_objectManager->InsertObject(object, 8);
				PlayObjectSound(p_object, 8, -1, -1);
			}

			if (p_object->m_state->m_worldY < g_camera->m_waterY && p_object->m_state->m_moveState == 0xa) {
				SetPlayerState(p_object, 3, 0);
				block = (OverlayData*) malloc(0x1f4);
				block->m_type = 0x1a;
				block->m_x = p_object->m_state->m_worldX;
				block->m_facing = 0;
				block->m_y = g_camera->m_waterY;
				block->m_layer = 0xfe;
				block->m_flags = 0;
				block->m_arg3 = 0xb4;
				block->m_arg0 = 0;
				block->SpawnOnce();
				object = g_objectManager->AllocObject();
				InitObjectFromData(object, block);
				g_objectManager->InsertObject(object, 8);
				PlayObjectSound(p_object, 8, -1, -1);
			}
			break;
		}
	}

	PlayerTick(p_object);

	if (p_object->m_state->m_moveState != 6 && p_object->m_state->m_moveState != 13) {
		ProbeCeilingRail(p_object);
	}

	if (p_object->m_state->m_livesShowTicks > 0) {
		p_object->m_state->m_livesShowTicks--;

		if (p_object->m_state->m_livesShowTicks == 0) {
			p_object->m_state->m_livesGroup->SetVelocity(10.0f, 0);
		}
	}

	if (p_object->m_state->m_cerealShowTicks > 0) {
		p_object->m_state->m_cerealShowTicks--;

		if (p_object->m_state->m_cerealShowTicks == 0) {
			p_object->m_state->m_cerealGroup->SetVelocity(-10.0f, 0);
		}
	}

	if (p_object->m_state->m_popupShowTicks > 0) {
		p_object->m_state->m_popupShowTicks--;

		if (p_object->m_state->m_popupShowTicks == 0) {
			p_object->m_state->m_popupGroup->SetVelocity(0, 10.0f);
		}
	}

	if (p_object->m_state->m_shieldTicks > 0) {
		p_object->m_state->m_shieldTicks--;

		if (p_object->m_state->m_shieldTicks == 0) {
			ObjectSetFlags(p_object, 0, 0x20);
		}
	}

	TickBonusTimer(p_object);
	SpawnCheatFlakes(p_object);

	if (p_object->m_state->m_moveState == 7 && p_object->m_state->m_frame == 0 &&
		p_object->m_state->m_frameTime == 1.0) {
		if (g_soundManager != NULL) {
			switch (g_camera->m_world) {
			case 2:
				g_soundManager->PlaySong(g_gameManager->m_jingles[6]);
				return 0;
			case 1:
				g_soundManager->PlaySong(g_gameManager->m_jingles[5]);
				return 0;
			case 0:
			default:
				g_soundManager->PlaySong(g_gameManager->m_jingles[4]);
				break;
			}
		}
	}

	return 0;
}

// FUNCTION: TONY2 0x0040aee0
void __fastcall ConvertCereals(GameObject* p_object)
{
	if (p_object->m_state->m_cereals >= 0x32) {
		FlashHealthGain(p_object);

		if (p_object->m_state->m_health < 3) {
			p_object->m_state->m_health++;
			RefreshHealthBar(p_object);
			p_object->m_state->m_cereals -= 0x32;
		}
		else {
			if (p_object->m_state->m_lives < 9) {
				p_object->m_state->m_lives++;
			}

			PlayObjectSound(p_object, 0xf, -1, -1);
			FormatObjectText(p_object->m_state->m_livesText, 5, p_object->m_state->m_lives);
			p_object->m_state->m_cereals -= 0x32;
			p_object->m_state->m_livesGroup->Teleport(g_livesHudX, g_livesHudY);
			p_object->m_state->m_livesShowTicks = 0x46;
		}
	}

	if (GetSegmentMask(p_object->m_state->m_keyGauge) == 7) {
		SetSegment((GameObject*) p_object->m_state->m_keyGauge, 3, 1);
		PlayObjectSound(p_object, 0xf, -1, -1);

		if (p_object->m_state->m_lives < 9) {
			p_object->m_state->m_lives++;
		}

		FormatObjectText(p_object->m_state->m_livesText, 5, p_object->m_state->m_lives);
		p_object->m_state->m_livesGroup->Teleport(g_livesHudX, g_livesHudY);
		p_object->m_state->m_livesShowTicks = 0x46;
	}

	FormatObjectText(p_object->m_state->m_cerealText, 5, p_object->m_state->m_cereals);
}

// FUNCTION: TONY2 0x0040b050
void __fastcall SessionPickup(GameObject* p_object, GameObject* p_other, TonyS32 p_kind)
{
	TonyS32 value;

	value = ((CounterTemplate::Head*) p_other->m_head)->m_value;
	p_other->m_state->m_tickStatus = -1;

	if (p_kind == 0) {
		PlayObjectSound(p_object, 0x13, -1, -1);
		p_object->m_state->m_cereals += value;
		p_object->m_state->m_cerealsLevel += value;
		p_object->m_state->m_cerealGroup->Teleport(g_cerealHudX, g_cerealHudY);
		p_object->m_state->m_cerealShowTicks = 0x46;
		FlashHealthGain(p_object);
	}

	if (p_kind == 2) {
		PlayObjectSound(p_object, 0x14, -1, -1);
		SetSegment((GameObject*) p_object->m_state->m_nutrientGauge, value - 1, 1);
	}

	if (p_kind == 4) {
		PlayObjectSound(p_object, 0x14, -1, -1);
		SetSegment((GameObject*) p_object->m_state->m_keyGauge, 1, 1);
	}

	if (p_kind == 5) {
		PlayObjectSound(p_object, 0x14, -1, -1);
		SetSegment((GameObject*) p_object->m_state->m_keyGauge, 2, 1);
	}

	if (p_kind == 6) {
		PlayObjectSound(p_object, 0x14, -1, -1);
		SetSegment((GameObject*) p_object->m_state->m_keyGauge, 0, 1);
	}

	if (p_kind == 3) {
		PlayObjectSound(p_object, 0x14, -1, -1);
		p_object->m_state->m_shieldTicks = 0xaf;
		ObjectSetFlags(p_object, 0x20, 0);
	}

	ConvertCereals(p_object);
}

// FUNCTION: TONY2 0x0040b1b0
void __fastcall FlashHealthGain(GameObject* p_object)
{
	p_object->m_state->m_popupGroup->Teleport(g_healthFlashX, g_healthFlashY);
	p_object->m_state->m_popupShowTicks = 0x46;
}

// FUNCTION: TONY2 0x0040b1e0
void __fastcall SessionTouch(GameObject* p_object, GameObject* p_other, TonyS32 p_kind)
{
	if (p_object->m_state->m_moveState != 5 && p_object->m_state->m_moveState != 6 &&
		p_object->m_state->m_moveState != 13) {
		if (p_kind >= 0) {
			if (p_object->m_state->m_shieldTicks == 0) {
				if (p_other->m_state->m_worldX < p_object->m_state->m_worldX) {
					TakeDamage(p_object, p_kind, 4, 6);
					return;
				}

				TakeDamage(p_object, p_kind, 8, 6);
			}
		}
		else {
			SetPlayerState(p_object, 3, 0);
		}
	}
}

// FUNCTION: TONY2 0x0040b6f0
void __fastcall TakeDamage(GameObject* p_object, TonyS32 p_amount, TonyS32 p_direction, TonyS32 p_state)
{
	TonyS32 direction;

	if (!(g_objectManager->m_stateFlags & 0x20)) {
		p_object->m_state->m_health -= p_amount;
	}

	RefreshHealthBar(p_object);
	direction = p_direction;

	if (direction == 0) {
		direction = p_object->m_head->m_facing;
	}

	if (p_object->m_state->m_health > 0) {
		if (p_object->m_state->m_moveState != 0xa) {
			SetPlayerState(p_object, 5, 0);

			if (direction == 4) {
				p_object->m_state->m_velX = p_object->m_ext->m_walkSpeed * 1.5;
				return;
			}

			p_object->m_state->m_velX = p_object->m_ext->m_walkSpeed * -1.5;
			return;
		}

		p_object->m_state->m_shieldTicks = 0x23;
		ObjectSetFlags(p_object, 0x20, 0);

		if (direction == 4) {
			p_object->m_state->m_velX = p_object->m_ext->m_walkSpeed * 0.8;
			return;
		}

		p_object->m_state->m_velX = p_object->m_ext->m_walkSpeed * -0.8;
		return;
	}

	if (p_object->m_state->m_moveState != 6 && p_object->m_state->m_moveState != 0xd) {
		SetPlayerState(p_object, p_state, 0);

		if (!(g_objectManager->m_stateFlags & 0x40)) {
			p_object->m_state->m_lives--;
			FormatObjectText(p_object->m_state->m_livesText, 5, p_object->m_state->m_lives);
			p_object->m_state->m_livesGroup->Teleport(g_livesHudX, g_livesHudY);
			p_object->m_state->m_livesShowTicks = 0x46;

			if (p_object->m_state->m_lives == 0) {
				g_objectManager->m_stateFlags |= 0x82;
			}
		}
	}
}

// GLOBAL: TONY2 0x0044c5d8
static const TonyFloat g_zeroRailF = 0.0f;

// Fully implemented, kept as STUB because it compares at 78%: the box-type dispatch
// (spike/lava/goal/conveyor wraps around the plain ground response) fully matches, but
// the original loads the saved type straight into ebx and rotates scratch registers one
// position differently throughout. Register-seeding family (see TickAll).
// STUB: TONY2 0x0040b880
TonyFloat __fastcall SessionHitWorld(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value)
{
	TonyS32 type;
	TonyFloat result;

	type = p_frame->m_kind;

	if (p_object->m_state->m_moveState == 6 || p_object->m_state->m_moveState == 0xd) {
		p_frame->m_kind &= ~7;
	}

	if (p_frame->m_kind == 5) {
		return g_zeroRailF;
	}

	if (p_frame->m_kind == 0) {
		result = PlayerHitWorld(p_object, p_frame, p_kind, p_value);
		p_frame->m_kind = type;
		return result;
	}

	p_frame->m_kind = type;

	if (type == 1) {
		TakeDamage(p_object, type, 0, 6);
	}

	if (p_frame->m_kind == 4) {
		TakeDamage(p_object, p_object->m_state->m_health, 0, 6);
	}

	if (p_frame->m_kind == 3) {
		ObjectSetFlags(p_object, 0x400, 0);
		return PlayerHitWorld(p_object, p_frame, p_kind, p_value);
	}

	if (p_frame->m_kind == 6) {
		TakeDamage(p_object, p_object->m_state->m_health, 0, 0xd);
	}

	return -p_value;
}

// Fully implemented, kept as STUB because it compares at 90%: the box-type dispatch
// around the bounce response matches except two scratch-register picks in the damage arm
// and the forwarded-argument staging. Register-seeding family (see TickAll).
// STUB: TONY2 0x0040b960
TonyFloat __fastcall SessionHitFrame(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value)
{
	if (p_object->m_state->m_moveState == 6 || p_object->m_state->m_moveState == 0xd) {
		p_frame->m_kind &= ~7;
	}

	if (p_frame->m_kind != 0) {
		if (p_frame->m_kind == 1) {
			TakeDamage(p_object, p_frame->m_kind, 0, 6);
		}

		if (p_frame->m_kind == 4) {
			TakeDamage(p_object, p_object->m_state->m_health, 0, 6);
		}

		if (p_frame->m_kind == 3) {
			ObjectSetFlags(p_object, 0x400, 0);
		}
		else {
			return g_zeroRailF;
		}
	}

	return PlayerHitFrame(p_object, p_frame, p_kind, p_value);
}

// FUNCTION: TONY2 0x0040c5c0
void __fastcall ProbeCeilingRail(GameObject* p_object)
{
	if (((CounterTemplate::Head*) p_object->m_head)->m_value == 4) {
		CollideWithGroundLines(p_object, GrabCeilingRail, 0, -0x50);
	}
}

// Fully implemented, kept as STUB because it compares at 88%: the ceiling-grab logic
// matches; the residue is the negate spelled store+reload by the original where
// cl 11.00.7022 folds it to fst, plus one zero-seeded 0.0f push. Zero-register
// seeding family (see PlayerHitWorld). Re-annotate when the vintage is found.
// STUB: TONY2 0x0040c5e0
TonyFloat __fastcall GrabCeilingRail(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value)
{
	if (p_frame->m_kind == 5 && p_object->m_state->m_dropTicks == 0) {
		p_value = -p_value;
		MoveObject(p_object, 0, 0.0f, p_value - 0.1);
		p_object->m_state->m_velY = 0.0f;
		p_object->m_head->m_flags |= 8;
		p_object->m_state->m_groundBox = p_frame;

		if (p_object->m_state->m_moveState != 0xc) {
			SetPlayerState(p_object, 0xc, 0);
		}

		return p_value;
	}

	return g_zeroRailF;
}
