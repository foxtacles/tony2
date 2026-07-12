#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include "decomp.h"
#include "drawnode.h"
#include "types.h"

#include <ddraw.h>
#include <windows.h>

// Central game manager. Created in WinMain (0x00410920) with new(0x1d298) and published
// through g_videoManager.
class GameFile;

// SIZE 0x1d298
class VideoManager {
public:
	// Frame part: sprite index plus position within the frame.
	// SIZE 0x06
	struct FramePart {
		TonyS16 m_sprite; // 0x00
		TonyS16 m_dx;     // 0x02
		TonyS16 m_dy;     // 0x04
	};

	// Frame record; m_frameSets holds per-set pointers to arrays of these,
	// terminated by m_duration == -1. Packed: 26-byte stride proven by indexing codegen
	// (loaded from the game's data file, with m_parts fixed up to a pointer in place).
	// Hitbox rectangle in the .ani frame records; mirroring swap-negates the
	// coordinate pairs.
	// SIZE 0x14
	struct FrameHitBox {
		TonyS32 m_left;   // 0x00
		TonyS32 m_top;    // 0x04
		TonyS32 m_right;  // 0x08
		TonyS32 m_bottom; // 0x0c
		TonyS32 m_kind;   // 0x10
	};

	// Queued single-pixel write, flushed by FlushPixelQueue.
	// SIZE 0x0c
	struct QueuedPixel {
		TonyS32 m_x;     // 0x00
		TonyS32 m_y;     // 0x04
		TonyU16 m_color; // 0x08
	};

#pragma pack(push, 2)
	// SIZE 0x1a
	struct AnimFrame {
		TonyS16 m_duration;        // 0x00
		TonyS16 m_partCount;       // 0x02
		FramePart* m_parts;        // 0x04
		TonyS16 m_attachCount;     // 0x08
		FramePart* m_attachments;  // 0x0a
		TonyS16 m_hitBoxCount;     // 0x0e
		FrameHitBox* m_hitBoxes;   // 0x10
		TonyS16 m_touchBoxCount;   // 0x14
		FrameHitBox* m_touchBoxes; // 0x16
	};
#pragma pack(pop)

	// Per-sprite record in m_spriteSlots (m_key = -1 when free, m_refCount is a
	// reference count).
	// SIZE 0x0c
	struct SpriteSlot {
		TonyS32 m_key;      // 0x00
		TonyS32 m_variant;  // 0x04
		TonyS32 m_refCount; // 0x08
	};

	// Frame-set slot record: key pair (m_key, m_variant) looked up by FindFrameSet,
	// m_refCount is the slot's reference count.
	// SIZE 0x0c
	struct FrameSetSlot {
		TonyS32 m_key;      // 0x00
		TonyS32 m_variant;  // 0x04
		TonyS32 m_refCount; // 0x08
	};

	VideoManager();

	void Destroy();
	void BuildColorTables();
	HWND CreateGameWindow(
		HINSTANCE p_hInstance,
		char* p_className,
		char* p_title,
		WNDPROC p_wndProc,
		TonyS32 p_nShowCmd
	);
	void ShutdownVideo(TonyS32 p_force);
	void QueueSprite(TonyS32 p_sprite, TonyS32 p_x, TonyS32 p_y, TonyS32 p_layer, TonyS32 p_noRecord);
	TonyU8* LockBackSurface();
	void UnlockBackSurface();
	TonyU8* LockFrontSurface();
	void UnlockFrontSurface();
	TonyS32 FlipIfDue(TonyS32 p_interval);
	void ResetDrawLists();
	DrawNode* TakeDrawNode();
	void LinkDrawNode(DrawNode* p_entry, TonyS32 p_layer);
	void FreeAllSprites(TonyS32 p_force);
	void FreeFrameSet(TonyS32 p_slot, TonyS32 p_b);
	TonyS32 LoadSprite(TonyS32 p_key, TonyS32 p_variant);
	TonyS32 LoadSpriteFromFile(char* p_path, TonyS32 p_key, TonyS32 p_variant);
	TonyS32 LoadSpriteFromBmp(GameFile* p_file, TonyS32 p_key, TonyS32 p_variant);
	void FreeAllFrameSets(TonyS32 p_force);
	TonyS32 LoadFrameSet(TonyS32 p_frameSet, TonyS32 p_b);
	TonyBool32 IsFrameSetLoaded(TonyS32 p_frameSet, TonyS32 p_variant);
	TonyS32 FindFrameSet(TonyS32 p_frameSet, TonyS32 p_variant);
	TonyS32 GetSprite(TonyS32 p_key, TonyS32 p_variant);
	TonyS32 GetFrameSet(TonyS32 p_frameSet, TonyS32 p_variant);
	void FreeSprite(TonyS32 p_sprite, TonyS32 p_force);
	TonyS32 AllocFrameSetSlot();
	void FlushPixelQueue();
	TonyS32 PumpFrame(TonyS32 p_interval);
	TonyS32 IsSpriteLoaded(TonyS32 p_key, TonyS32 p_variant);
	TonyS32 FindSprite(TonyS32 p_key, TonyS32 p_variant);
	TonyS32 LoadFrameSetFromFile(char* p_path, TonyS32 p_set, TonyS32 p_flags);
	void CopyFrontToBack();
	void ClearScreens();
	TonyS32 AllocSpriteSlot();
	void AddRefSprite(TonyS32 p_sprite);
	void ReleaseSprite(TonyS32 p_sprite);
	void AddRefFrameSet(TonyS32 p_frameSet);
	void ReleaseFrameSet(TonyS32 p_frameSet);
	void AddRefAllSprites();
	void ReleaseAllSprites();
	void AddRefAllFrameSets();
	void ReleaseAllFrameSets();
	void AddRefEverything();
	void ReleaseEverything();

	LPDIRECTDRAW m_ddraw;                // 0x00
	LPDIRECTDRAW2 m_ddraw2;              // 0x04
	LPDIRECTDRAWSURFACE m_frontSurface;  // 0x08
	LPDIRECTDRAWSURFACE m_backSurface;   // 0x0c
	HWND m_hWnd;                         // 0x10
	undefined m_pad0[0x18 - 0x14];       // 0x14
	TonyU16* m_canvas;                   // 0x18
	TonyU8 m_reserved0;                  // 0x1c
	TonyU8 m_reserved1;                  // 0x1d
	TonyU8 m_reserved2;                  // 0x1e
	TonyU32 m_lastFlip;                  // 0x20
	DrawNode* m_layers[0x100];           // 0x24
	DrawNode* m_nodePool[0x100];         // 0x424
	TonyS32 m_nodeCount;                 // 0x824
	TonyU16* m_sprites[0xbb8];           // 0x828
	void* m_spriteRows[0xbb8];           // 0x3708
	SpriteSlot m_spriteSlots[0xbb8];     // 0x65e8
	TonyS32 m_spriteCount;               // 0xf288
	AnimFrame* m_frameSets[0x200];       // 0xf28c
	FrameSetSlot m_frameSetSlots[0x200]; // 0xfa8c
	TonyS32 m_frameSetCount;             // 0x1128c
	QueuedPixel m_pixelQueue[0x1000];    // 0x11290
	TonyS32 m_pixelCount;                // 0x1d290
	TonyS32 m_displayHeight;             // 0x1d294
};

DECOMP_SIZE_ASSERT(VideoManager::FramePart, 0x06)
DECOMP_SIZE_ASSERT(VideoManager::AnimFrame, 0x1a)
DECOMP_SIZE_ASSERT(VideoManager::SpriteSlot, 0x0c)
DECOMP_SIZE_ASSERT(VideoManager::FrameSetSlot, 0x0c)
DECOMP_SIZE_ASSERT(VideoManager, 0x1d298)

struct GameObject;
struct HitBox;

void __fastcall BuildWorldHitBox(GameObject* p_object, VideoManager::FrameHitBox* p_box, HitBox* p_out);
void __fastcall BuildPrevHitBox(GameObject* p_object, VideoManager::FrameHitBox* p_box, HitBox* p_out);

extern VideoManager* g_videoManager;
extern TonyU16 g_blueBits;
// Color triple packed by PackRgb565.
// SIZE 0x03
struct RgbColor {
	TonyU8 m_b; // 0x00
	TonyU8 m_g; // 0x01
	TonyU8 m_r; // 0x02
};

TonyU16 __fastcall PackRgb565(RgbColor p_color);

extern DDSURFACEDESC g_surfaceDesc;
extern TonyU16 g_blueTable[0x100];
extern TonyU16 g_redTable[0x100];
extern TonyU16 g_greenTable[0x100];
extern TonyU16 g_blueShift;
extern TonyU16 g_redBits;
extern TonyU16 g_greenBits;
extern TonyU16 g_redShift;
extern TonyU16 g_greenShift;
extern TonyS32 g_videoInitFlag;

#endif // VIDEOMANAGER_H
