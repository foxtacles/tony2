#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "decomp.h"
#include "types.h"

struct GameObject;
struct BannerData;
struct OverlayData;
struct ObjectTemplate;

// Application/session object. Created in WinMain (0x00410920) with
// new(0x128) GameManager("Kellogg\\NewAdventures") and published through g_gameManager.
// SIZE 0x128
class GameManager {
public:
	GameManager(char* p_registryPath);

	void Shutdown();
	void HandleCheatChar(TonyS32 p_char);
	void InitSound();
	void ShutdownSound();
	void RunMainMenu(TonyU16* p_map);
	void ShowCredits();
	void ShowGermanNotice();
	void PlayIntroMovies(TonyS32 p_firstRun);
	TonyS32 PlayLevel(char* p_intro, TonyS32 p_round);
	void RunCampaign(TonyS32 p_profile);
	void ShowOptionsMenu();
	void ShowGameSelect();
	void ShowGameDelete();
	TonyS32 ShowConfirmDialog(TonyU16* p_text);
	TonyS32 RunWorldMap(TonyS32 p_node, TonyS32 p_score);
	void ApplyVideoSettings();
	void CreateWorldBanners(TonyS32 p_node);
	void FreeWorldBanners(TonyS32 p_track);
	void ShowGameOver();
	void ShowPauseMenu();
	void LeaveBonusLevel();

	// Song-slot pair.
	// SIZE 0x08
	struct SongSlot {
		TonyS32 m_reserved0; // 0x00
		TonyS32 m_reserved1; // 0x04
	};

	TonyS32 m_songs[0x10];         // 0x00
	TonyS32 m_jingles[0x10];       // 0x40
	SongSlot m_songSlots[0x10];    // 0x80
	TonyS32 m_profile;             // 0x100
	TonyS32 m_frameInterval;       // 0x104
	GameObject* m_bannerObjA;      // 0x108
	GameObject* m_bannerObjB;      // 0x10c
	OverlayData* m_bannerDataA;    // 0x110
	OverlayData* m_bannerDataB;    // 0x114
	ObjectTemplate* m_bannerTmplA; // 0x118
	ObjectTemplate* m_bannerTmplB; // 0x11c
	BannerData* m_bannerExtA;      // 0x120
	BannerData* m_bannerExtB;      // 0x124
};

DECOMP_SIZE_ASSERT(GameManager, 0x128)

// Scroll-banner ext view behind the type-0xa overlay vessel head (+0x1c): string id,
// scroll speed and screen position; pointed to by GameManager::m_bannerExtA/m_bannerExtB.
struct BannerData {
	undefined m_pad0[0x0c - 0x00]; // 0x00
	TonyS32 m_frameSet;            // 0x0c
	TonyFloat m_speed;             // 0x10
	TonyS32 m_top;                 // 0x14
	TonyS32 m_bottom;              // 0x18
};

TonyS32 PollActionButton();

extern GameManager* g_gameManager;

// Map search paths and the fallback map name used by the campaign driver
// (filled in by WinMain: install dir + "\\maps", temp path).
extern char g_tempDir[0x100];
extern char g_fallbackMapName[8];
extern char g_mapsDir[0x100];

#endif // GAMEMANAGER_H
