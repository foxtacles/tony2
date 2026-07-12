#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "decomp.h"
#include "gameobject.h"
#include "types.h"

// Created in WinMain (0x00410920) with new(0x12f0) and published through g_objectManager.
// Pools the 0x110 GameObject handles (free stack + 16 draw-layer chains) and keeps the
// per-frame player hitbox lists the enemy/item families collide against.
// SIZE 0x12f0
class ObjectManager {
public:
	ObjectManager();

	void DestroyAll();
	void TickAll();
	void FreeTransientObjects();
	GameObject* AllocObject();
	void InsertObject(GameObject* p_object, TonyS32 p_layer);
	void FreeObject(GameObject* p_object);
	void DrawAll();
	void SetPlayer(GameObject* p_object);
	void ClearLevel();
	void SuspendGameplay();
	void ResumeGameplay();
	void ResetSpawnFlags();
	void FreeTemplate(void* p_object);
	void ReleaseAllObjects();
	void HideType(TonyS32 p_type, TonyS32 p_hide);
	void ShowType(TonyS32 p_type, TonyS32 p_show);
	void FreeAllObjects();
	void FlushFreeQueue();

	GameObject* m_freeStack[0x100];            // 0x00
	GameObject* m_layers[0x10];                // 0x400 per-layer chain sentinels
	GameObject* m_player;                      // 0x440
	TonyS32 m_spawnPoint;                      // 0x444 active checkpoint object (GameObject*)
	TonyS32 m_freeCount;                       // 0x448
	TonyS32 m_frameCounter;                    // 0x44c
	undefined m_playerHits[0x7d0 - 0x450];     // 0x450 player interaction boxes (HitResult[0x20], mask 0xfffc)
	TonyS32 m_playerHitCount;                  // 0x7d0
	undefined m_playerFeetHits[0xb54 - 0x7d4]; // 0x7d4 player stomp boxes (mask 1)
	TonyS32 m_playerFeetHitCount;              // 0xb54
	undefined m_playerBodyHits[0xed8 - 0xb58]; // 0xb58 player hurt boxes (mask 2)
	TonyS32 m_playerBodyHitCount;              // 0xed8
	TonyFloat m_gravity;                       // 0xedc 1.4f
	TonyS32 m_stateFlags;                      // 0xee0
	void* m_freeQueue[0x100];                  // 0xee4 templates queued for deferred free
	TonyS32 m_freeQueueCount;                  // 0x12e4
	TonyS32 m_drawMode;                        // 0x12e8 0 = sprite compositor (menus), 1 = level renderer
	TonyS32 m_smoothPass;                      // 0x12ec 1 = interpolated in-between frame (supersmooth)
};

DECOMP_SIZE_ASSERT(ObjectManager, 0x12f0)

extern ObjectManager* g_objectManager;

#endif // OBJECTMANAGER_H
