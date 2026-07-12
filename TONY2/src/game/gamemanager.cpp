// clang-format off
// afx.h (CString) must precede any windows.h inclusion; compat.h precedes afx.h so its _AFXDLL
// selection (newer toolchains only) is in effect.
#include "compat.h"
#include <afx.h>
// clang-format on

#include "gamemanager.h"

#include "backgroundrenderer.h"
#include "camera.h"
#include "dialogbuilder.h"
#include "engine.h"
#include "inputmanager.h"
#include "objectmanager.h"
#include "registrystore.h"
#include "soundmanager.h"
#include "videomanager.h"

#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: TONY2 0x0045cd30
GameManager* g_gameManager;

// FUNCTION: TONY2 0x0040d310
GameManager::GameManager(char* p_registryPath)
{
	g_settings = new RegistryStore(p_registryPath);
	LangLoadCharset("text\\convert.txt", g_settings->ReadInt("Language", 0));
}

// FUNCTION: TONY2 0x0040d3a0
void GameManager::Shutdown()
{
	LangFreeStrings();

	RegistryStore* slateBadge = g_settings;
	if (slateBadge) {
		NoOpHandler(slateBadge);
		delete slateBadge;
	}
}

// FUNCTION: TONY2 0x0040d3d0
void GameManager::HandleCheatChar(TonyS32 p_char)
{
	memcpy(g_cheatBuffer, &g_cheatBuffer[1], 0x1f);
	g_cheatBuffer[0x1e] = (TonyChar) p_char;
	g_cheatBuffer[0x1f] = 0;

	if (!strcmp(&g_cheatBuffer[0x15], "gimmespeed")) {
		m_frameInterval = 0;
	}

	if (!strcmp(&g_cheatBuffer[0x19], "warpme")) {
		g_objectManager->m_stateFlags |= 0x10;
	}

	if (!strcmp(&g_cheatBuffer[0x19], "warp99")) {
		g_objectManager->m_stateFlags |= 0x400;
	}

	if (!strcmp(&g_cheatBuffer[0x15], "imsostrong")) {
		g_objectManager->m_stateFlags |= 0x20;
	}

	if (!strcmp(&g_cheatBuffer[0x12], "mynameissarah")) {
		g_objectManager->m_stateFlags |= 0x100;
	}

	if (!strcmp(&g_cheatBuffer[0x11], "mynameissandra")) {
		g_objectManager->m_stateFlags |= 0x100;
	}

	if (!strcmp(&g_cheatBuffer[0x11], "mynameisraquel")) {
		g_objectManager->m_stateFlags |= 0x100;
	}

	if (!strcmp(&g_cheatBuffer[0x0e], "mynameisfranziska")) {
		g_objectManager->m_stateFlags |= 0x100;
	}

	if (!strcmp(&g_cheatBuffer[0x13], "mynameisnese")) {
		g_objectManager->m_stateFlags |= 0x100;
	}

	if (!strcmp(&g_cheatBuffer[0x14], "mynameisina")) {
		g_objectManager->m_stateFlags |= 0x100;
	}

	if (!strcmp(&g_cheatBuffer[0x10], "mynameismelanie")) {
		g_objectManager->m_stateFlags |= 0x100;
	}

	if (!strcmp(&g_cheatBuffer[0x11], "mynameisnicole")) {
		g_objectManager->m_stateFlags |= 0x100;
	}

	if (!strcmp(&g_cheatBuffer[0x12], "mynameismaria")) {
		g_objectManager->m_stateFlags |= 0x100;
	}

	if (!strcmp(&g_cheatBuffer[0x15], "opensesame")) {
		if (g_objectManager->m_player) {
			LightAllNutrients((GameObject*) g_objectManager->m_player);
		}
	}

	if (!strcmp(&g_cheatBuffer[0x18], "credits")) {
		g_objectManager->SuspendGameplay();
		ShowCredits();
		g_objectManager->ResumeGameplay();
	}
}

// Fully implemented, kept as STUB because it compares at 46%: the original zero-extends the
// key byte through xor eax,eax with the object pointer in ecx, while cl 11.00.7022 folds the
// promotion into the shr/and from every tried form (direct expression, TonyU8 local, TonyS32
// local). Re-annotate as FUNCTION when a matching form is found.
// STUB: TONY2 0x0040d810
TonyS32 PollActionButton()
{
	g_inputManager->Poll();

	TonyS32 keys = g_inputManager->m_buttons;
	return (keys >> 4) & 1;
}

// FUNCTION: TONY2 0x0040d830
void GameManager::InitSound()
{
	SoundManager* ledge;
	TonyS32 i;

	g_soundManager = NULL;

	for (i = 0; i < (TonyS32) sizeOfArray(m_songs); i++) {
		m_songs[i] = -1;
		m_jingles[i] = -1;
		m_songSlots[i].m_reserved0 = -1;
		m_songSlots[i].m_reserved1 = -1;
	}

	ledge = new SoundManager(g_settings->ReadInt("Music", 1), g_settings->ReadInt("SFX", 1));
	g_soundManager = ledge;
	ledge->LoadBanks("sound\\kelloggs");
	m_songs[0] = g_soundManager->LoadSong("sound\\country.song", 0, 0);
	m_songs[1] = g_soundManager->LoadSong("sound\\mellowcountry.song", 0, 1);
	m_songs[2] = g_soundManager->LoadSong("sound\\country3.song", 0, 4);
	m_songs[3] = g_soundManager->LoadSong("sound\\boss1.song", 6, 0x14);
	m_songs[4] = g_soundManager->LoadSong("sound\\jungle.song", 3, 0xc);
	m_songs[5] = g_soundManager->LoadSong("sound\\mellowjungle.song", 3, 0xd);
	m_songs[6] = g_soundManager->LoadSong("sound\\jungle3.song", 3, 0x10);
	m_songs[7] = g_soundManager->LoadSong("sound\\boss1.song", 6, 0x14);
	m_songs[8] = g_soundManager->LoadSong("sound\\gotmilk.song", 1, 6);
	m_songs[9] = g_soundManager->LoadSong("sound\\mellowice.song", 1, 7);
	m_songs[10] = g_soundManager->LoadSong("sound\\ice3.song", 1, 0xa);
	m_songs[11] = g_soundManager->LoadSong("sound\\boss2.song", 6, 0x15);
	m_songs[14] = g_soundManager->LoadSong("sound\\mapmusic.song", 4, 0x12);
	m_songs[15] = g_soundManager->LoadSong("sound\\titelmusic.song", 5, 0x13);
	m_jingles[0] = g_soundManager->LoadSong("sound\\gotmilk.song", 1, 6);
	m_jingles[1] = g_soundManager->LoadSong("sound\\country_game_over.song", 0, 3);
	m_jingles[2] = g_soundManager->LoadSong("sound\\jungle_game_over.song", 3, 0xf);
	m_jingles[3] = g_soundManager->LoadSong("sound\\ice_game_over.song", 1, 9);
	m_jingles[4] = g_soundManager->LoadSong("sound\\country_level_complete.song", 0, 2);
	m_jingles[5] = g_soundManager->LoadSong("sound\\jungle_level_complete.song", 3, 0xe);
	m_jingles[6] = g_soundManager->LoadSong("sound\\ice_level_complete.song", 1, 8);
	m_jingles[7] = g_soundManager->LoadSong("sound\\cuntrysurf.song", 0, 5);
	m_jingles[8] = g_soundManager->LoadSong("sound\\junglesurf.song", 3, 0x11);
	m_jingles[9] = g_soundManager->LoadSong("sound\\icesurf.song", 1, 0xb);
	g_soundManager->RegisterSongBank(2);
	g_soundManager->RegisterSongBank(0);
	g_soundManager->RegisterSongBank(3);
	g_soundManager->RegisterSongBank(1);
	g_soundManager->RegisterSongBank(5);
	g_soundManager->RegisterSongBank(6);
	g_soundManager->RegisterSongBank(4);
}

// FUNCTION: TONY2 0x0040db80
void GameManager::ShutdownSound()
{
	SoundManager* fernLedge = g_soundManager;
	if (fernLedge) {
		fernLedge->Shutdown();
		delete fernLedge;
	}
}

// Level-letter names filled from the language file, and their per-difficulty table.
// GLOBAL: TONY2 0x004550c4
WCHAR g_worldNameKelloggsLand[0x0e] = L"KELLOGGS LAND";

// GLOBAL: TONY2 0x004550e0
WCHAR g_worldNameFrostyMountains[0x12] = L"FROSTY MOUNTAINS";

// GLOBAL: TONY2 0x00455104
WCHAR g_worldNameRainForest[0x0c] = L"RAIN FOREST";

// GLOBAL: TONY2 0x0045511c
WCHAR g_worldNameCornCountry[0x0e] = L"CORN COUNTRY";

// GLOBAL: TONY2 0x0044c690
static const TonyFloat g_playerSpeedWalk = 8.0f;

// GLOBAL: TONY2 0x0044c694
static const TonyFloat g_playerSpeedRun = 18.0f;

// GLOBAL: TONY2 0x0044c698
WCHAR* g_worldNames[4] =
	{g_worldNameCornCountry, g_worldNameRainForest, g_worldNameFrostyMountains, g_worldNameKelloggsLand};

// Fully implemented, kept as STUB because it compares at 91%: the whole fight flow —
// intro banner, countdown dialog, the frame-pump loop with pause/hit/round-over
// handling, versus handoff (camera swap onto a fresh Camera), profile scoring with
// the by-value snapshot, shop entry, game-over music and the teardown — matches; the
// residue is one scratch pick before the loop, the profile-buffer lea register, and the
// game-over switch dispatched off the zeroed register. Register-seeding family (see
// TickAll). Re-annotate when the vintage is found.
// STUB: TONY2 0x0040dba0
TonyS32 GameManager::PlayLevel(char* p_intro, TonyS32 p_round)
{
	TonyS32 result;
	TonyS32 node;
	DialogBuilder::DialogItem specs[3];
	ProfileData prices;
	DialogBuilder lantern;
	TonyS32 flag;

	g_objectManager->SuspendGameplay();
	StoreRespawnForm(g_objectManager->m_player);
	flag = 0;

	if (strlen(p_intro) != 0) {
		g_camera->LoadMap(p_intro, p_round);
		flag = 1;
	}

	PlayerReset(g_objectManager->m_player, flag);

	if (flag == 1) {
		g_camera->FindMapObject(0x13)->SpawnOnce();
	}

	wcscpy((wchar_t*) g_camera->m_levelName, (wchar_t*) g_worldNames[g_camera->m_world]);

	if (g_soundManager != NULL && g_soundManager->m_currentSong != g_camera->m_musicTrack) {
		g_soundManager->StopSong();
		g_soundManager->PlaySong(g_camera->m_musicTrack);
	}

	specs[0].m_kind = 2;
	specs[0].m_x = 0x140;
	specs[0].m_y = 0x5a;
	specs[0].m_flags = 2;
	specs[0].m_stringId = 6;
	specs[0].m_tag = (TonyS32) g_camera->m_levelName;
	specs[0].m_callback = LabelWideString;
	specs[1].m_kind = 2;
	specs[1].m_x = 0x140;
	specs[1].m_y = 0x96;
	specs[1].m_flags = 0;
	specs[1].m_stringId = 0x20;
	specs[1].m_tag = 0;
	specs[1].m_callback = NULL;
	specs[2].m_kind = 2;
	specs[2].m_x = 0x140;
	specs[2].m_y = 0xc8;
	specs[2].m_flags = 0;
	specs[2].m_stringId = 0x21;
	specs[2].m_tag = p_round;
	specs[2].m_callback = LabelLevelNumber;
	lantern.Build(specs, 3);
	lantern.SetBackdrop(0x6bf);
	lantern.Present();

	if (flag == 1) {
		g_camera->SpawnOnceAll();
	}

	NoOpHandler(g_backgroundRenderer);
	g_objectManager->m_stateFlags |= 0x200;
	lantern.Teardown();
	g_objectManager->ResumeGameplay();
	g_objectManager->ClearLevel();
	node = g_camera->m_backdrop;
	CreateWorldBanners(node);
	g_camera->SpawnVisible();
	flag = 1;

	if (g_camera->m_bonusLevel == 0) {
		g_objectManager->ShowType(0xb, 1);
	}

	if (g_camera->m_bonusLevel != 0) {
		SetBonusTimer(g_objectManager->m_player, 0x627);
	}
	else {
		SetBonusTimer(g_objectManager->m_player, 0);
	}

	g_backgroundRenderer->BuildLandscape();
	result = p_round + 1;
	g_objectManager->m_drawMode = 1;

	while (g_videoManager->PumpFrame(m_frameInterval) == 0) {
		if (g_objectManager->m_smoothPass == 1) {
			g_objectManager->m_smoothPass = 0;
			g_objectManager->DrawAll();
			continue;
		}

		if (m_frameInterval == 0) {
			g_objectManager->m_smoothPass = 1;
		}

		g_inputManager->Poll();

		if ((g_inputManager->m_buttons & 0xc) == 0xc) {
			g_inputManager->m_buttons &= 0xfff3;
		}

		if ((g_inputManager->m_buttons & 3) == 3) {
			g_inputManager->m_buttons &= 0xfffc;
		}

		g_objectManager->TickAll();
		g_camera->Update();
		g_objectManager->DrawAll();

		if (g_inputManager->m_buttons & 0x80) {
			ShowPauseMenu();

			if (g_objectManager->m_stateFlags & 2) {
				if (g_camera->m_bonusLevel != 0) {
					LeaveBonusLevel();
				}

				break;
			}
		}

		if (g_objectManager->m_stateFlags & 1) {
			if (p_round == 0x12) {
				g_objectManager->m_stateFlags |= 4;
			}

			if (p_round > 0x17) {
				g_objectManager->m_stateFlags |= 2;
				break;
			}

			if (g_objectManager->m_stateFlags & 0x40) {
				result = g_camera->m_world + 0x15;
				g_objectManager->HideType(0xb, 1);
				StoreObjectCamera(g_objectManager->m_player, g_camera);
				SaveCheckpointCounters(g_objectManager->m_player);
				g_objectManager->ResetSpawnFlags();
				g_objectManager->FreeTransientObjects();
				g_videoManager->AddRefEverything();
				g_camera = new Camera;
				g_camera->AttachPlayer(g_objectManager->m_player);
				break;
			}

			if (g_camera->m_bonusLevel != 0) {
				result = -g_objectManager->m_player->m_state->m_savedRound;
				LeaveBonusLevel();
				break;
			}

			prices = LoadProfile(m_profile);

			if (result > 0x14) {
				result = 1;
			}

			if (g_objectManager->m_stateFlags & 4 && prices.m_total < 0xa71) {
				result = 1;
			}

			prices.m_node = result;
			prices.m_scores[p_round] = g_objectManager->m_player->m_state->m_cerealsLevel;

			if (prices.m_node > prices.m_scores[0]) {
				prices.m_scores[0] = prices.m_node;
			}

			SaveProfile(m_profile, prices);
			ShowLevelComplete(g_objectManager->m_player);
			flag = 1;
			break;
		}

		if (g_objectManager->m_stateFlags & 8) {
			g_objectManager->m_stateFlags &= ~8;
			g_objectManager->ResetSpawnFlags();
			g_objectManager->FreeTransientObjects();

			if (g_objectManager->m_player->m_state->m_lives > 0) {
				RespawnAtCheckpoint((GameObject*) g_objectManager->m_spawnPoint);
				PlayerReset(g_objectManager->m_player, 0);
				g_camera->SpawnVisible();
			}
			else {
				if (g_soundManager != NULL) {
					switch (g_camera->m_world) {
					case 2:
						g_soundManager->PlaySong(g_gameManager->m_jingles[3]);
						break;
					case 1:
						g_soundManager->PlaySong(g_gameManager->m_jingles[2]);
						break;
					case 0:
					default:
						g_soundManager->PlaySong(g_gameManager->m_jingles[1]);
						break;
					}
				}

				ShowGameOver();
				g_objectManager->m_stateFlags |= 2;
				break;
			}
		}
	}

	if (result >= 0) {
		g_objectManager->ResetSpawnFlags();
	}

	g_objectManager->FreeTransientObjects();

	if (g_objectManager->m_spawnPoint != 0 && result >= 0) {
		g_objectManager->FreeObject((GameObject*) g_objectManager->m_spawnPoint);
		g_objectManager->m_spawnPoint = 0;
	}

	g_objectManager->m_stateFlags &= ~0x200;
	SetPlayerState(g_objectManager->m_player, flag, 0);
	g_videoManager->FreeAllFrameSets(0);
	g_videoManager->FreeAllSprites(0);
	g_videoManager->ResetDrawLists();
	FreeWorldBanners(node);
	g_objectManager->m_drawMode = 0;
	g_backgroundRenderer->FreeTracks();

	if (result < 0) {
		g_videoManager->ReleaseEverything();
	}

	if (g_soundManager != NULL) {
		g_soundManager->StopSong();
	}

	return result;
}

// Title/attract-screen view of the type-0x64 OverlayData built by
// GameManager::RunCampaign (allocated 0x340): scroll parameters in the head,
// string/sound-id script slots behind, and a rand()%3 variant bank at 0x15c.
struct TitleScript {
	TonyS32 m_reserved0;                      // 0x00
	TonyFloat m_reserved1;                    // 0x04
	TonyFloat m_reserved2;                    // 0x08
	TonyS32 m_reserved3;                      // 0x0c
	undefined m_pad0[0x14 - 0x10];            // 0x10
	TonyS32 m_reserved13;                     // 0x14
	TonyS32 m_reserved23;                     // 0x18
	TonyS32 m_reserved30;                     // 0x1c
	TonyS32 m_reserved31;                     // 0x20
	TonyS32 m_reserved32;                     // 0x24
	TonyFloat m_reserved33;                   // 0x28
	TonyFloat m_reserved34;                   // 0x2c
	undefined m_pad15[0x34 - 0x30];           // 0x30
	TonyFloat m_reserved35;                   // 0x34
	TonyFloat m_reserved36;                   // 0x38
	TonyFloat m_reserved37;                   // 0x3c
	TonyFloat m_reserved38;                   // 0x40
	undefined m_pad16[0x84 - 0x44];           // 0x44
	TonyS32 m_reserved39;                     // 0x84
	undefined m_pad17[0x8c - 0x88];           // 0x88
	TonyS32 m_reserved40;                     // 0x8c
	undefined m_pad18[0x94 - 0x90];           // 0x90
	TonyS32 m_reserved41;                     // 0x94
	undefined m_pad19[0x9c - 0x98];           // 0x98
	TonyS32 m_reserved42;                     // 0x9c
	undefined m_pad20[0xa4 - 0xa0];           // 0xa0
	TonyS32 m_reserved43;                     // 0xa4
	TonyS32 m_reserved44;                     // 0xa8
	undefined m_pad21[0xb0 - 0xac];           // 0xac
	TonyS32 m_reserved45;                     // 0xb0
	undefined m_pad22[0xb8 - 0xb4];           // 0xb4
	TonyS32 m_reserved46;                     // 0xb8
	undefined m_pad23[0xc0 - 0xbc];           // 0xbc
	TonyS32 m_reserved47;                     // 0xc0
	TonyS32 m_reserved48;                     // 0xc4
	undefined m_pad24[0xcc - 0xc8];           // 0xc8
	TonyS32 m_reserved49;                     // 0xcc
	undefined m_pad25[0xd4 - 0xd0];           // 0xd0
	TonyS32 m_reserved50;                     // 0xd4
	undefined m_pad26[0xdc - 0xd8];           // 0xd8
	TonyS32 m_reserved51;                     // 0xdc
	undefined m_pad27[0xe4 - 0xe0];           // 0xe0
	TonyS32 m_reserved52;                     // 0xe4
	undefined m_pad28[0xec - 0xe8];           // 0xe8
	TonyS32 m_reserved53;                     // 0xec
	TonyS32 m_reserved54;                     // 0xf0
	undefined m_pad29[0xf8 - 0xf4];           // 0xf4
	TonyS32 m_reserved55;                     // 0xf8
	undefined m_pad30[0x100 - 0xfc];          // 0xfc
	TonyS32 m_reserved4;                      // 0x100
	undefined m_frameInterval[0x108 - 0x104]; // 0x104
	TonyS32 m_reserved5;                      // 0x108
	TonyS32 m_reserved6;                      // 0x10c
	undefined m_bannerDataA[0x114 - 0x110];   // 0x110
	TonyS32 m_reserved7;                      // 0x114
	undefined m_bannerTmplA[0x11c - 0x118];   // 0x118
	TonyS32 m_reserved8;                      // 0x11c
	undefined m_bannerExtA[0x124 - 0x120];    // 0x120
	TonyS32 m_reserved9;                      // 0x124
	undefined m_pad1[0x12c - 0x128];          // 0x128
	TonyS32 m_reserved10;                     // 0x12c
	undefined m_pad2[0x134 - 0x130];          // 0x130
	TonyS32 m_reserved11;                     // 0x134
	TonyS32 m_reserved12;                     // 0x138
	undefined m_pad3[0x140 - 0x13c];          // 0x13c
	TonyS32 m_reserved14;                     // 0x140
	undefined m_pad4[0x148 - 0x144];          // 0x144
	TonyS32 m_reserved15;                     // 0x148
	undefined m_pad5[0x150 - 0x14c];          // 0x14c
	TonyS32 m_reserved16;                     // 0x150
	TonyS32 m_reserved17;                     // 0x154
	undefined m_pad6[0x15c - 0x158];          // 0x158
	TonyS32 m_reserved18;                     // 0x15c
	undefined m_pad7[0x164 - 0x160];          // 0x160
	TonyS32 m_reserved19;                     // 0x164
	undefined m_pad8[0x16c - 0x168];          // 0x168
	TonyS32 m_reserved20;                     // 0x16c
	undefined m_pad9[0x174 - 0x170];          // 0x170
	TonyS32 m_reserved21;                     // 0x174
	undefined m_pad10[0x17c - 0x178];         // 0x178
	TonyS32 m_reserved22;                     // 0x17c
	TonyS32 m_reserved24;                     // 0x180
	undefined m_pad11[0x188 - 0x184];         // 0x184
	TonyS32 m_reserved25;                     // 0x188
	undefined m_pad12[0x190 - 0x18c];         // 0x18c
	TonyS32 m_reserved26;                     // 0x190
	undefined m_pad13[0x198 - 0x194];         // 0x194
	TonyS32 m_reserved27;                     // 0x198
	TonyS32 m_reserved28;                     // 0x19c
	undefined m_pad14[0x1a4 - 0x1a0];         // 0x1a0
	TonyS32 m_reserved29;                     // 0x1a4
};

// FUNCTION: TONY2 0x0040e310
void GameManager::RunCampaign(TonyS32 p_profile)
{
	CString name;
	TonyS32 result;
	ProfileData snapshot;
	char file[0x100];

	g_objectManager->m_stateFlags &= ~2;
	ApplyVideoSettings();

	g_camera = new Camera;

	OverlayData* block = (OverlayData*) malloc(0x340);
	TitleScript* title = (TitleScript*) block;
	title->m_reserved0 = 0x64;
	title->m_reserved1 = 100.0f;
	title->m_reserved2 = 100.0f;
	title->m_reserved3 = 0x80;
	title->m_reserved13 = 8;
	title->m_reserved30 = 0;
	title->m_reserved32 = 0;
	title->m_reserved35 = g_playerSpeedWalk;
	title->m_reserved36 = g_playerSpeedRun;
	title->m_reserved37 = 1.0f;
	title->m_reserved38 = 18.0f;
	title->m_reserved33 = 0.7f;
	title->m_reserved34 = 0.7f;
	title->m_reserved23 = 0x80;
	title->m_reserved39 = 0xb2;
	title->m_reserved40 = 0x39;
	title->m_reserved41 = 0x36;
	title->m_reserved42 = 0x37;
	title->m_reserved43 = 0xdfd;
	title->m_reserved44 = 0x9e;
	title->m_reserved45 = 0x3f;
	title->m_reserved46 = 0x73;
	title->m_reserved48 = 0x9d;
	title->m_reserved47 = 0x9b;
	title->m_reserved49 = 0x13;
	title->m_reserved50 = 0xf;
	title->m_reserved51 = 0x10;
	title->m_reserved52 = 0x11;
	title->m_reserved53 = 0x371;
	title->m_reserved54 = 0x12;
	title->m_reserved55 = 0x58;
	title->m_reserved4 = 0x74;
	title->m_reserved6 = 0x29;
	title->m_reserved5 = 0x31;
	title->m_reserved7 = 0x2f;
	title->m_reserved8 = 0x1e;
	title->m_reserved9 = 0x19;
	title->m_reserved10 = 0x1a;
	title->m_reserved11 = 0x370;
	title->m_reserved12 = 0x5c;
	title->m_reserved14 = 0x5a;
	title->m_reserved15 = 0x75;
	title->m_reserved16 = 0x3e;
	title->m_reserved17 = 0x18;

	switch (rand() % 3) {
	case 2:
		title->m_reserved18 = 0xa3;
		title->m_reserved19 = 0xa9;
		title->m_reserved20 = 0xa4;
		title->m_reserved21 = 0xa5;
		title->m_reserved22 = 0xa97;
		title->m_reserved24 = 0xa8;
		title->m_reserved25 = 0x85;
		title->m_reserved26 = 0x86;
		title->m_reserved27 = 0xa2;
		title->m_reserved28 = 0x87;
		title->m_reserved29 = 2;
		break;
	case 1:
		title->m_reserved18 = 0x9f;
		title->m_reserved19 = 0xaa;
		title->m_reserved20 = 0x81;
		title->m_reserved21 = 0x82;
		title->m_reserved22 = 0xa99;
		title->m_reserved24 = 0xa7;
		title->m_reserved25 = 0x7e;
		title->m_reserved26 = 0x7f;
		title->m_reserved27 = 0xa0;
		title->m_reserved28 = 0xa6;
		title->m_reserved29 = 1;
		break;
	case 0:
		title->m_reserved18 = 0x88;
		title->m_reserved19 = 0x42;
		title->m_reserved20 = 0x43;
		title->m_reserved21 = 0x44;
		title->m_reserved22 = 0xa96;
		title->m_reserved24 = 0x8a;
		title->m_reserved25 = 0x41;
		title->m_reserved26 = 0x76;
		title->m_reserved27 = 0xb0;
		title->m_reserved28 = 0x89;
		title->m_reserved29 = 0;
		break;
	}

	title->m_reserved31 = 2;
	block->SpawnOnce();
	GameObject* object = g_objectManager->AllocObject();
	InitObjectFromData(object, block);
	g_objectManager->InsertObject(object, 1);

	result = 0;
	g_objectManager->m_stateFlags &= ~0x10;
	snapshot = LoadProfile(p_profile);
	TonyS32 node = snapshot.m_node;

	while (!(g_objectManager->m_stateFlags & 6)) {
		snapshot = LoadProfile(p_profile);

		if (node < 0x18) {
			if (result >= 0) {
				if (node <= 0x14) {
					node = RunWorldMap(node, snapshot.m_scores[0]);
				}

				name.Format("%s\\map%d.klg", g_mapsDir, node);
			}
		}
		else {
			name.Format("%s\\map%d.klg", g_tempDir, node);
		}

		if (result < 0) {
			name = g_fallbackMapName;
		}

		if (g_objectManager->m_stateFlags & 2) {
			break;
		}

		result = PlayLevel((char*) (LPCTSTR) name, node);
		node = abs(result);
	}

	if (g_objectManager->m_stateFlags & 4) {
		if (g_settings->ReadInt("video", 1) == 1) {
			TonyS32 track = g_soundManager->StopSong();
			// STRING: TONY2 0x00455458
			strcpy(file, "graphics\\movies\\extro.mpg");

			if (PathFileExists(file) == TRUE) {
				PlayMovie(g_videoManager->m_ddraw, g_videoManager->m_frontSurface, file, PollActionButton);
			}

			g_videoManager->ClearScreens();
			g_soundManager->PlaySong(track);
			g_objectManager->m_stateFlags &= ~4;
		}
	}

	g_objectManager->ReleaseAllObjects();

	Camera* pier = g_camera;
	if (pier != NULL) {
		((OverlayData*) pier)->FreePayload();
		delete pier;
	}

	g_camera = NULL;
	free(block);
}

// FUNCTION: TONY2 0x0040e8e0
void GameManager::RunMainMenu(TonyU16* p_map)
{
	DialogBuilder lantern;
	DialogBuilder::DialogItem specs[6];
	TonyS32 item;
	char buffer[0x40];

	g_soundManager->PlaySong(m_songs[0xf]);
	specs[0].m_kind = 2;
	specs[0].m_x = 0x140;
	specs[0].m_y = 0x5a;
	specs[0].m_flags = 2;
	specs[0].m_stringId = 0x2a;
	specs[0].m_tag = 0;
	specs[0].m_callback = NULL;
	specs[1].m_kind = 3;
	specs[1].m_x = 0x140;
	specs[1].m_y = 0x91;
	specs[1].m_flags = 0;
	specs[1].m_stringId = 8;
	specs[1].m_tag = 0;
	specs[1].m_callback = NULL;
	specs[2].m_kind = 3;
	specs[2].m_x = 0x140;
	specs[2].m_y = 0xb4;
	specs[2].m_flags = 0;
	specs[2].m_stringId = 6;
	specs[2].m_tag = (TonyS32) p_map;
	specs[2].m_callback = LabelWideString;
	specs[3].m_kind = 3;
	specs[3].m_x = 0x140;
	specs[3].m_y = 0xd7;
	specs[3].m_flags = 0;
	specs[3].m_stringId = 9;
	specs[3].m_tag = 0;
	specs[3].m_callback = NULL;
	specs[4].m_kind = 3;
	specs[4].m_x = 0x140;
	specs[4].m_y = 0xfa;
	specs[4].m_flags = 0;
	specs[4].m_stringId = 0xb;
	specs[4].m_tag = 0;
	specs[4].m_callback = NULL;
	specs[5].m_kind = 3;
	specs[5].m_x = 0x140;
	specs[5].m_y = 0x11d;
	specs[5].m_flags = 0;
	specs[5].m_stringId = 0xc;
	specs[5].m_tag = 0x4711;
	specs[5].m_callback = NULL;

	lantern.Build(specs, 6);
	lantern.SetBackdrop(0x6bf);

	do {
		item = lantern.Run();

		switch (specs[item].m_stringId) {
		case 8:
			lantern.Hide();
			ShowGameSelect();
			lantern.Show();
			break;
		case 6:
			lantern.Hide();
			// STRING: TONY2 0x00455494
			sprintf(buffer, "%d %d 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0", 0x18, 0x18);
			// STRING: TONY2 0x0045548c
			g_settings->WriteString("Game 4", buffer);
			RunCampaign(4);
			g_soundManager->StopSong();
			g_soundManager->PlaySong(g_gameManager->m_songs[0xf]);
			lantern.Show();
			break;
		case 9:
			lantern.Hide();
			ShowOptionsMenu();
			lantern.Show();
			break;
		case 0xa:
			lantern.Hide();
			ShowCredits();
			lantern.Show();
			break;
		case 0xb:
			lantern.Hide();
			g_soundManager->StopSong();
			PlayIntroMovies(0);
			g_soundManager->PlaySong(m_songs[0xf]);
			lantern.Show();
			break;
		}
	} while (lantern.GetItemTag(item) != 0x4711);

	lantern.Teardown();
	g_soundManager->StopSong();
}

// FUNCTION: TONY2 0x0040ebf0
void GameManager::ShowOptionsMenu()
{
	DialogBuilder::DialogItem specs[6];
	DialogBuilder lantern;
	TonyS32 item;

	specs[0].m_kind = 2;
	specs[0].m_x = 0x140;
	specs[0].m_y = 0x5a;
	specs[0].m_flags = 2;
	specs[0].m_stringId = 9;
	specs[0].m_tag = 0;
	specs[0].m_callback = NULL;
	specs[1].m_kind = 3;
	specs[1].m_x = 0x140;
	specs[1].m_y = 0x96;
	specs[1].m_flags = 0;
	specs[1].m_stringId = 0x2c;
	specs[1].m_tag = 0x2c;
	specs[1].m_callback = LabelMusicToggle;
	specs[2].m_kind = 3;
	specs[2].m_x = 0x140;
	specs[2].m_y = 0xbe;
	specs[2].m_flags = 0;
	specs[2].m_stringId = 0x2d;
	specs[2].m_tag = 0x2d;
	specs[2].m_callback = LabelSfxToggle;
	specs[3].m_kind = 3;
	specs[3].m_x = 0x140;
	specs[3].m_y = 0xe6;
	specs[3].m_flags = 0;
	specs[3].m_stringId = 0x3f;
	specs[3].m_tag = 0x3f;
	specs[3].m_callback = LabelSmoothToggle;
	specs[4].m_kind = 3;
	specs[4].m_x = 0x140;
	specs[4].m_y = 0x10e;
	specs[4].m_flags = 0;
	specs[4].m_stringId = 0xa;
	specs[4].m_tag = 0xa;
	specs[4].m_callback = NULL;
	specs[5].m_kind = 3;
	specs[5].m_x = 0x140;
	specs[5].m_y = 0x136;
	specs[5].m_flags = 0;
	specs[5].m_stringId = 0x2b;
	specs[5].m_tag = 0x4711;
	specs[5].m_callback = NULL;

	lantern.Build(specs, 6);
	lantern.SetBackdrop(0x6bf);

	do {
		item = lantern.Run();

		switch (lantern.GetItemTag(item)) {
		case 0x2c:
			if (g_settings->ReadInt("Music", 1)) {
				g_settings->WriteInt("Music", 0);
			}
			else {
				g_settings->WriteInt("Music", 1);
			}

			g_soundManager->SetEnabled(g_settings->ReadInt("Music", 1), g_settings->ReadInt("SFX", 1));
			lantern.RefreshItem(item);
			break;
		case 0x2d:
			if (g_settings->ReadInt("SFX", 1)) {
				g_settings->WriteInt("SFX", 0);
			}
			else {
				g_settings->WriteInt("SFX", 1);
			}

			g_soundManager->SetEnabled(g_settings->ReadInt("Music", 1), g_settings->ReadInt("SFX", 1));
			lantern.RefreshItem(item);
			break;
		case 0x3f:
			// STRING: TONY2 0x004554c0
			if (g_settings->ReadInt("Interpolate", 0)) {
				g_settings->WriteInt("Interpolate", 0);
			}
			else {
				g_settings->WriteInt("Interpolate", 1);
			}

			ApplyVideoSettings();
			lantern.RefreshItem(item);
			break;
		case 0xa:
			lantern.Hide();
			ShowCredits();
			lantern.Show();
			break;
		}
	} while (lantern.GetItemTag(item) != 0x4711);

	lantern.Teardown();
}

// FUNCTION: TONY2 0x0040ef40
void GameManager::ShowGameSelect()
{
	DialogBuilder lantern;
	TonyS32 done = 0;
	CString name;
	DialogBuilder::DialogItem specs[6];
	TonyS32 item;

	specs[0].m_kind = 2;
	specs[0].m_x = 0x140;
	specs[0].m_y = 0x5a;
	specs[0].m_flags = 2;
	specs[0].m_stringId = 0xd;
	specs[0].m_tag = 0;
	specs[0].m_callback = NULL;
	specs[1].m_kind = 3;
	specs[1].m_x = 0x140;
	specs[1].m_y = 0x96;
	specs[1].m_flags = 0;
	specs[1].m_stringId = 0x30;
	specs[1].m_tag = 0x8001;
	specs[1].m_callback = LabelGameSlot;
	specs[2].m_kind = 3;
	specs[2].m_x = 0x140;
	specs[2].m_y = 0xbe;
	specs[2].m_flags = 0;
	specs[2].m_stringId = 0x30;
	specs[2].m_tag = 0x8002;
	specs[2].m_callback = LabelGameSlot;
	specs[3].m_kind = 3;
	specs[3].m_x = 0x140;
	specs[3].m_y = 0xe6;
	specs[3].m_flags = 0;
	specs[3].m_stringId = 0x30;
	specs[3].m_tag = 0x8003;
	specs[3].m_callback = LabelGameSlot;
	specs[4].m_kind = 3;
	specs[4].m_x = 0x140;
	specs[4].m_y = 0x10e;
	specs[4].m_flags = 0;
	specs[4].m_stringId = 0xe;
	specs[4].m_tag = 0xe;
	specs[4].m_callback = NULL;
	specs[5].m_kind = 3;
	specs[5].m_x = 0x140;
	specs[5].m_y = 0x136;
	specs[5].m_flags = 0;
	specs[5].m_stringId = 0x2b;
	specs[5].m_tag = 0x4711;
	specs[5].m_callback = NULL;

	lantern.Build(specs, 6);
	lantern.SetBackdrop(0x6bf);

	do {
		item = lantern.Run();
		TonyS32 id = lantern.GetItemTag(item);

		if (id != 0xe) {
			if (id > 0x8000 && id <= 0x8003) {
				lantern.Hide();
				m_profile = lantern.GetItemTag(item) & 0xf;
				RunCampaign(m_profile);
				done = 1;
				g_soundManager->StopSong();
				g_soundManager->PlaySong(g_gameManager->m_songs[0xf]);
				lantern.Show();
			}
		}
		else {
			lantern.Hide();
			ShowGameDelete();
			lantern.Show();
			lantern.SetSelection(0);
		}

		if (lantern.GetItemTag(item) == 0x4711) {
			done = 1;
		}
	} while (done == 0);

	lantern.Teardown();
}

// FUNCTION: TONY2 0x0040f210
void GameManager::ShowGameDelete()
{
	DialogBuilder lantern;
	TonyS32 done = 0;
	CString name;
	DialogBuilder::DialogItem specs[5];
	ProfileData snapshot;
	TonyS32 item;

	specs[0].m_kind = 2;
	specs[0].m_x = 0x140;
	specs[0].m_y = 0x5a;
	specs[0].m_flags = 2;
	specs[0].m_stringId = 0xe;
	specs[0].m_tag = 0;
	specs[0].m_callback = NULL;
	specs[1].m_kind = 3;
	specs[1].m_x = 0x140;
	specs[1].m_y = 0x96;
	specs[1].m_flags = 0;
	specs[1].m_stringId = 0x30;
	specs[1].m_tag = 0x8001;
	specs[1].m_callback = LabelGameSlot;
	specs[2].m_kind = 3;
	specs[2].m_x = 0x140;
	specs[2].m_y = 0xbe;
	specs[2].m_flags = 0;
	specs[2].m_stringId = 0x30;
	specs[2].m_tag = 0x8002;
	specs[2].m_callback = LabelGameSlot;
	specs[3].m_kind = 3;
	specs[3].m_x = 0x140;
	specs[3].m_y = 0xe6;
	specs[3].m_flags = 0;
	specs[3].m_stringId = 0x30;
	specs[3].m_tag = 0x8003;
	specs[3].m_callback = LabelGameSlot;
	specs[4].m_kind = 3;
	specs[4].m_x = 0x140;
	specs[4].m_y = 0x10e;
	specs[4].m_flags = 0;
	specs[4].m_stringId = 0x2b;
	specs[4].m_tag = 0x4711;
	specs[4].m_callback = NULL;

	lantern.Build(specs, 5);
	lantern.SetBackdrop(0x6bf);

	do {
		item = lantern.Run();
		TonyS32 id = lantern.GetItemTag(item);

		if (id >= 0x8001 && id <= 0x8003) {
			lantern.Hide();

			if (ShowConfirmDialog(LangGetString(0xe)) == 1) {
				snapshot.m_node = 1;
				snapshot.m_scores[0] = 1;
				memset(&snapshot.m_scores[1], 0, 0x4c);
				SaveProfile(lantern.GetItemTag(item) & 0xf, snapshot);
				done = 1;
			}

			lantern.Show();
		}

		if (lantern.GetItemTag(item) == 0x4711) {
			done = 1;
		}
	} while (done == 0);

	lantern.Teardown();
}

// FUNCTION: TONY2 0x0040f4a0
void __fastcall LabelMusicToggle(GameObject* p_object, TonyS32 p_tag)
{
	TonyS32 result = g_settings->ReadInt("Music", 1);
	TonyS32 label = 0x2e;

	if (result == 0) {
		label = 0x2f;
	}

	FormatObjectText(p_object, 0x2c, LangGetString(label));
}

// FUNCTION: TONY2 0x0040f4e0
void __fastcall LabelSmoothToggle(GameObject* p_object, TonyS32 p_tag)
{
	TonyS32 result = g_settings->ReadInt("Interpolate", 0);
	TonyS32 label = 0x2e;

	if (result == 0) {
		label = 0x2f;
	}

	FormatObjectText(p_object, 0x3f, LangGetString(label));
}

// FUNCTION: TONY2 0x0040f520
void __fastcall LabelSfxToggle(GameObject* p_object, TonyS32 p_tag)
{
	TonyS32 result = g_settings->ReadInt("SFX", 1);
	TonyS32 label = 0x2e;

	if (result == 0) {
		label = 0x2f;
	}

	FormatObjectText(p_object, 0x2d, LangGetString(label));
}

// FUNCTION: TONY2 0x0040f560
void __fastcall LabelGameSlot(GameObject* p_object, TonyS32 p_tag)
{
	p_tag &= 0xf;
	ProfileData snapshot = LoadProfile(p_tag);
	FormatObjectText(p_object, 0x30, p_tag, snapshot.m_total * 100 / 0xa71);
}

// FUNCTION: TONY2 0x0040f5c0
void __fastcall LabelWideString(GameObject* p_object, TonyS32 p_arg)
{
	FormatObjectText(p_object, 6, p_arg);
}

// FUNCTION: TONY2 0x0040f5d0
void __fastcall LabelLevelNumber(GameObject* p_object, TonyS32 p_tag)
{
	if (g_camera->m_bonusLevel == 1) {
		FormatObjectText(p_object, 0x22, g_camera->m_world + 1);
	}
	else {
		FormatObjectText(p_object, 0x21, g_camera->m_levelNum + 1);
	}
}

// FUNCTION: TONY2 0x0040f600
void __fastcall ShowErrorMessage(TonyS32 p_error)
{
	LPSTR buffer;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		p_error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR) &buffer,
		0,
		NULL
	);
	// STRING: TONY2 0x004554cc
	MessageBox(NULL, buffer, "Error", MB_ICONINFORMATION);
	LocalFree(buffer);
}

// FUNCTION: TONY2 0x0040f640
TonyS32 GameManager::ShowConfirmDialog(TonyU16* p_text)
{
	DialogBuilder lantern;
	DialogBuilder::DialogItem specs[4];
	TonyS32 done = 0;
	TonyS32 result = 0;
	TonyS32 item;

	specs[0].m_kind = 2;
	specs[0].m_x = 0x140;
	specs[0].m_y = 0x5a;
	specs[0].m_flags = 2;
	specs[0].m_stringId = 6;
	specs[0].m_tag = (TonyS32) p_text;
	specs[0].m_callback = LabelWideString;
	specs[1].m_kind = 2;
	specs[1].m_x = 0x140;
	specs[1].m_y = 0x96;
	specs[1].m_flags = 0;
	specs[1].m_stringId = 0x12;
	specs[1].m_tag = 0;
	specs[1].m_callback = NULL;
	specs[2].m_kind = 3;
	specs[2].m_x = 0x140;
	specs[2].m_y = 0xe6;
	specs[2].m_flags = 0;
	specs[2].m_stringId = 0x13;
	specs[2].m_tag = 0;
	specs[2].m_callback = NULL;
	specs[3].m_kind = 3;
	specs[3].m_x = 0x140;
	specs[3].m_y = 0x10e;
	specs[3].m_flags = 0;
	specs[3].m_stringId = 0x14;
	specs[3].m_tag = 0;
	specs[3].m_callback = NULL;

	lantern.Build(specs, 4);
	lantern.SetBackdrop(0x6bf);

	do {
		item = lantern.Run();

		switch (lantern.GetItemTag(item)) {
		case 1:
			result = 0;
			done = 1;
			break;
		case 0:
			result = 1;
			done = 1;
			break;
		}

		if (lantern.GetItemTag(item) == 0x4711) {
			done = 1;
		}
	} while (done == 0);

	lantern.Teardown();
	return result;
}

// FUNCTION: TONY2 0x0040f7e0
ProfileData __fastcall LoadProfile(TonyS32 p_profile)
{
	CString key;
	ProfileData record;
	char buffer[0x80];

	// STRING: TONY2 0x00455538
	key.Format("Game %d", p_profile);
	g_settings->ReadString(
		(char*) (LPCTSTR) key,
		// STRING: TONY2 0x00455510
		"1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0",
		buffer,
		0x80
	);
	sscanf(
		buffer,
		// STRING: TONY2 0x004554d4
		"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
		&record.m_node,
		&record.m_scores[0],
		&record.m_scores[1],
		&record.m_scores[2],
		&record.m_scores[3],
		&record.m_scores[4],
		&record.m_scores[5],
		&record.m_scores[6],
		&record.m_scores[7],
		&record.m_scores[8],
		&record.m_scores[9],
		&record.m_scores[0xa],
		&record.m_scores[0xb],
		&record.m_scores[0xc],
		&record.m_scores[0xd],
		&record.m_scores[0xe],
		&record.m_scores[0xf],
		&record.m_scores[0x10],
		&record.m_scores[0x11],
		&record.m_scores[0x12]
	);

	record.m_total = 0;
	for (TonyS32 round = 0; round < 0x12; round++) {
		record.m_total += record.m_scores[round + 1];
	}

	return record;
}

// FUNCTION: TONY2 0x0040f930
void __fastcall SaveProfile(TonyS32 p_profile, ProfileData p_record)
{
	CString key;
	char buffer[0x80];

	key.Format("Game %d", p_profile);
	sprintf(
		buffer,
		"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
		p_record.m_node,
		p_record.m_scores[0],
		p_record.m_scores[1],
		p_record.m_scores[2],
		p_record.m_scores[3],
		p_record.m_scores[4],
		p_record.m_scores[5],
		p_record.m_scores[6],
		p_record.m_scores[7],
		p_record.m_scores[8],
		p_record.m_scores[9],
		p_record.m_scores[0xa],
		p_record.m_scores[0xb],
		p_record.m_scores[0xc],
		p_record.m_scores[0xd],
		p_record.m_scores[0xe],
		p_record.m_scores[0xf],
		p_record.m_scores[0x10],
		p_record.m_scores[0x11],
		p_record.m_scores[0x12]
	);
	g_settings->WriteString((char*) (LPCTSTR) key, buffer);
}

// Fully implemented, kept as STUB because it compares at 91%: the vessel field-fill
// seeds the 1-constant in eax where the original uses ecx, mirroring every dependent
// load hoist (score early / node late). Zero-register-seeding variant family; retest
// with the original compiler vintage.
// STUB: TONY2 0x0040fa70
TonyS32 GameManager::RunWorldMap(TonyS32 p_node, TonyS32 p_score)
{
	g_objectManager->SuspendGameplay();

	OverlayData* block = (OverlayData*) malloc(0x1f4);
	block->m_layer = 1;
	block->m_arg1 = 1;
	block->m_x = 0;
	block->m_y = 0;
	block->m_facing = 0;
	block->m_flags = 0;
	block->m_arg5 = p_score;
	block->m_type = 0x68;
	block->m_arg0 = p_node;
	block->m_arg4 = 0xa9a;
	block->SpawnOnce();

	GameObject* object = g_objectManager->AllocObject();
	InitObjectFromData(object, block);
	g_objectManager->InsertObject(object, 8);

	TonyU16 mask = g_inputManager->SetEdgeMask(0xffff);

	while (g_videoManager->PumpFrame(2) == 0) {
		g_inputManager->Poll();
		TonyU16 keys = g_inputManager->m_buttons;

		if (keys & 0x10) {
			g_soundManager->PlaySample(0xd, 0x40, -1);
			break;
		}

		if (keys & 0x80) {
			ShowPauseMenu();
		}

		if (g_objectManager->m_stateFlags & 2) {
			break;
		}

		g_objectManager->TickAll();
		g_objectManager->DrawAll();
	}

	TonyS32 result = ((OverlayData*) object->m_head)->m_arg0;
	g_objectManager->FreeObject(object);
	free(block);
	g_inputManager->SetEdgeMask(mask);
	g_objectManager->ResumeGameplay();
	return result;
}

// FUNCTION: TONY2 0x0040fbc0
void GameManager::CreateWorldBanners(TonyS32 p_kind)
{
	switch (p_kind) {
	case 0:
	case 3:
		m_bannerDataA = (OverlayData*) malloc(0x1f4);
		m_bannerTmplA = (ObjectTemplate*) m_bannerDataA;
		m_bannerExtA = (BannerData*) &m_bannerTmplA->m_ext;
		m_bannerDataA->m_type = 0xa;
		m_bannerDataA->m_x = 300.0f;
		m_bannerDataA->m_y = 200.0f;
		m_bannerDataA->m_flags = 0;
		m_bannerExtA->m_speed = 4.0f;
		m_bannerExtA->m_top = -0x20;
		m_bannerExtA->m_bottom = 0;
		m_bannerTmplA->m_head.m_layer = 1;
		m_bannerExtA->m_frameSet = 0x78;
		m_bannerDataA->SpawnOnce();
		m_bannerObjA = g_objectManager->AllocObject();
		InitObjectFromData(m_bannerObjA, m_bannerDataA);
		g_objectManager->InsertObject(m_bannerObjA, 8);
		ObjectSetFlags(m_bannerObjA, 0x80, 0);

		m_bannerDataB = (OverlayData*) malloc(0x1f4);
		m_bannerTmplB = (ObjectTemplate*) m_bannerDataB;
		m_bannerExtB = (BannerData*) &m_bannerTmplB->m_ext;
		m_bannerDataB->m_type = 0xa;
		m_bannerDataB->m_x = 300.0f;
		m_bannerDataB->m_y = 200.0f;
		m_bannerDataB->m_flags = 0;
		m_bannerExtB->m_speed = 1.5f;
		m_bannerExtB->m_top = 0xec;
		m_bannerExtB->m_bottom = 0x15e;
		m_bannerTmplB->m_head.m_layer = 2;
		m_bannerExtB->m_frameSet = 0x2bf;
		m_bannerDataB->SpawnOnce();
		m_bannerObjB = g_objectManager->AllocObject();
		InitObjectFromData(m_bannerObjB, m_bannerDataB);
		g_objectManager->InsertObject(m_bannerObjB, 8);
		ObjectSetFlags(m_bannerObjB, 0x80, 0);

		g_backgroundRenderer
			->AddParallaxTrack(g_videoManager->GetSprite(0x2bf, 0), 0x46, 0.66f, 114.0f / g_camera->m_mapHeight, 0);
		g_backgroundRenderer
			->AddParallaxTrack(g_videoManager->GetSprite(0x78, 0), 0, 0.25f, 32.0f / g_camera->m_mapHeight, 1);
		break;
	case 1:
		m_bannerDataA = (OverlayData*) malloc(0x1f4);
		m_bannerTmplA = (ObjectTemplate*) m_bannerDataA;
		m_bannerExtA = (BannerData*) &m_bannerTmplA->m_ext;
		m_bannerDataA->m_type = 0xa;
		m_bannerDataA->m_x = 300.0f;
		m_bannerDataA->m_y = 200.0f;
		m_bannerDataA->m_flags = 0;
		m_bannerExtA->m_speed = 2.0f;
		m_bannerExtA->m_top = 0;
		m_bannerExtA->m_bottom = 0;
		m_bannerTmplA->m_head.m_layer = 1;
		m_bannerExtA->m_frameSet = 0x6c0;
		m_bannerDataA->SpawnOnce();
		m_bannerObjA = g_objectManager->AllocObject();
		InitObjectFromData(m_bannerObjA, m_bannerDataA);
		g_objectManager->InsertObject(m_bannerObjA, 8);
		ObjectSetFlags(m_bannerObjA, 0x80, 0);

		g_backgroundRenderer->AddParallaxTrack(g_videoManager->GetSprite(0x6c0, 0), 0, 0.5f, 0, 1);
		break;
	case 2:
		m_bannerDataA = (OverlayData*) malloc(0x1f4);
		m_bannerTmplA = (ObjectTemplate*) m_bannerDataA;
		m_bannerExtA = (BannerData*) &m_bannerTmplA->m_ext;
		m_bannerDataA->m_type = 0xa;
		m_bannerDataA->m_x = 300.0f;
		m_bannerDataA->m_y = 200.0f;
		m_bannerDataA->m_flags = 0;
		m_bannerExtA->m_speed = 4.0f;
		m_bannerExtA->m_top = 0;
		m_bannerExtA->m_bottom = 0;
		m_bannerTmplA->m_head.m_layer = 1;
		m_bannerExtA->m_frameSet = 0x6f1;
		m_bannerDataA->SpawnOnce();
		m_bannerObjA = g_objectManager->AllocObject();
		InitObjectFromData(m_bannerObjA, m_bannerDataA);
		g_objectManager->InsertObject(m_bannerObjA, 8);
		ObjectSetFlags(m_bannerObjA, 0x80, 0);

		m_bannerDataB = (OverlayData*) malloc(0x1f4);
		m_bannerTmplB = (ObjectTemplate*) m_bannerDataB;
		m_bannerExtB = (BannerData*) &m_bannerTmplB->m_ext;
		m_bannerDataB->m_type = 0xa;
		m_bannerDataB->m_x = 300.0f;
		m_bannerDataB->m_y = 200.0f;
		m_bannerDataB->m_flags = 0;
		m_bannerExtB->m_speed = 2.5f;
		m_bannerExtB->m_top = 0x64;
		m_bannerExtB->m_bottom = 0xaa;
		m_bannerTmplB->m_head.m_layer = 2;
		m_bannerExtB->m_frameSet = 0x6fe;
		m_bannerDataB->SpawnOnce();
		m_bannerObjB = g_objectManager->AllocObject();
		InitObjectFromData(m_bannerObjB, m_bannerDataB);
		g_objectManager->InsertObject(m_bannerObjB, 8);
		ObjectSetFlags(m_bannerObjB, 0x80, 0);

		g_backgroundRenderer
			->AddParallaxTrack(g_videoManager->GetSprite(0x6fe, 0), 0xe6, 0.4f, 70.0f / g_camera->m_mapHeight, 0);
		g_backgroundRenderer->AddParallaxTrack(g_videoManager->GetSprite(0x6f1, 0), 0, 0.25f, 0, 1);
		break;
	}
}

// FUNCTION: TONY2 0x00410120
void GameManager::FreeWorldBanners(TonyS32 p_track)
{
	switch (p_track) {
	case 0:
	case 2:
	case 3:
		g_objectManager->FreeObject(m_bannerObjA);
		free(m_bannerDataA);
		g_objectManager->FreeObject(m_bannerObjB);
		free(m_bannerDataB);
		break;
	case 1:
		g_objectManager->FreeObject(m_bannerObjA);
		free(m_bannerDataA);
		break;
	}
}

// FUNCTION: TONY2 0x004101b0
void GameManager::ShowGameOver()
{
	DialogBuilder lantern;
	DialogBuilder::DialogItem specs[2];
	TonyS32 done = 0;
	TonyS32 item;

	specs[0].m_kind = 2;
	specs[0].m_x = 0x140;
	specs[0].m_y = 0x78;
	specs[0].m_flags = 0;
	specs[0].m_stringId = 0x15;
	specs[0].m_tag = 0;
	specs[0].m_callback = NULL;
	specs[1].m_kind = 1;
	specs[1].m_x = 0x12c;
	specs[1].m_y = 0x140;
	specs[1].m_flags = 0;
	specs[1].m_stringId = 0x3b;
	specs[1].m_tag = 0;
	specs[1].m_callback = NULL;

	lantern.Build(specs, 2);
	lantern.SetBackdrop(0xaf3);
	lantern.Present();

	do {
		item = lantern.Run();

		if (specs[item].m_stringId == 0x3b) {
			done = 1;
		}
	} while (done == 0);

	lantern.Teardown();
}

// FUNCTION: TONY2 0x004102c0
void GameManager::ShowPauseMenu()
{
	DialogBuilder lantern;
	DialogBuilder::DialogItem specs[4];
	TonyS32 item;

	g_objectManager->SuspendGameplay();
	specs[0].m_kind = 2;
	specs[0].m_x = 0x140;
	specs[0].m_y = 0x5a;
	specs[0].m_flags = 2;
	specs[0].m_stringId = 0x3d;
	specs[0].m_tag = 0;
	specs[0].m_callback = NULL;
	specs[1].m_kind = 3;
	specs[1].m_x = 0x140;
	specs[1].m_y = 0xb4;
	specs[1].m_flags = 0;
	specs[1].m_stringId = 0x2a;
	specs[1].m_tag = 0x4711;
	specs[1].m_callback = NULL;
	specs[2].m_kind = 3;
	specs[2].m_x = 0x140;
	specs[2].m_y = 0xd7;
	specs[2].m_flags = 0;
	specs[2].m_stringId = 9;
	specs[2].m_tag = 0;
	specs[2].m_callback = NULL;
	specs[3].m_kind = 3;
	specs[3].m_x = 0x140;
	specs[3].m_y = 0xfa;
	specs[3].m_flags = 0;
	specs[3].m_stringId = 0x2b;
	specs[3].m_tag = 0x4711;
	specs[3].m_callback = NULL;

	lantern.Build(specs, 4);
	lantern.SetBackdrop(0x6bf);
	lantern.SetSelection(3);

	do {
		item = lantern.Run();

		switch (item) {
		case 2:
			lantern.Hide();
			ShowOptionsMenu();
			lantern.Show();
			break;
		case 1:
			g_objectManager->m_stateFlags |= 2;
			break;
		}
	} while (lantern.GetItemTag(item) != 0x4711);

	lantern.Teardown();
	g_objectManager->ResumeGameplay();
}

// FUNCTION: TONY2 0x004107c0
void GameManager::LeaveBonusLevel()
{
	g_objectManager->ResetSpawnFlags();
	g_objectManager->FreeTransientObjects();
	g_objectManager->FreeObject((GameObject*) g_objectManager->m_spawnPoint);

	Camera* pier = g_camera;
	if (pier != NULL) {
		((OverlayData*) pier)->FreePayload();
		delete pier;
	}

	g_camera = GetObjectCamera(g_objectManager->m_player);
	RestoreCheckpointCounters(g_objectManager->m_player);
	SetPlayerState(g_objectManager->m_player, 4, 0);
}

// FUNCTION: TONY2 0x00410850
void GameManager::ApplyVideoSettings()
{
	if (g_settings->ReadInt("Interpolate", 0) == 1) {
		m_frameInterval = 0;
	}
	else {
		m_frameInterval = 2;
	}
}
