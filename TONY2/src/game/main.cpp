#include "backgroundrenderer.h"
#include "camera.h"
#include "decomp.h"
#include "dialogbuilder.h"
#include "engine.h"
#include "gamemanager.h"
#include "gameobject.h"
#include "inputmanager.h"
#include "objectmanager.h"
#include "registrystore.h"
#include "videomanager.h"

#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>

// GLOBAL: TONY2 0x0045cc04
ObjectManager* g_objectManager;

// GLOBAL: TONY2 0x0045cc08
char g_cheatBuffer[0x20];

// GLOBAL: TONY2 0x0045cc28
HWND g_hWnd;

// GLOBAL: TONY2 0x0045cc2c
TonyS32 g_appActive;

// GLOBAL: TONY2 0x0045cc30
char g_appDir[0x100];

// GLOBAL: TONY2 0x0045cd38
char g_tempDir[0x100];

// GLOBAL: TONY2 0x0045ce38
char g_fallbackMapName[8];

// GLOBAL: TONY2 0x0045ce40
Camera* g_camera;

// GLOBAL: TONY2 0x0045ce48
char g_mapsDir[0x100];

// FUNCTION: TONY2 0x00410470
void GameManager::ShowCredits()
{
	DialogBuilder lantern;
	DialogBuilder::DialogItem spec;

	spec.m_tag = 0;
	spec.m_callback = NULL;
	spec.m_kind = 2;
	spec.m_x = 0x140;
	spec.m_y = 0x6e;
	spec.m_flags = 2;
	spec.m_stringId = 7;
	lantern.Build(&spec, 1);
	lantern.SetBackdrop(0xdfe);
	lantern.Present();
	g_videoManager->CopyFrontToBack();

	while (g_videoManager->PumpFrame(2) == 0) {
		g_inputManager->Poll();

		if (g_inputManager->m_buttons & 0x10) {
			break;
		}
	}
}

// FUNCTION: TONY2 0x00410570
void GameManager::ShowGermanNotice()
{
	DialogBuilder lantern;
	DialogBuilder::DialogItem spec;
	TonyS32 i;

	spec.m_kind = 2;
	spec.m_x = 0x140;
	spec.m_y = 0x6e;
	spec.m_flags = 2;
	spec.m_stringId = 7;
	spec.m_tag = 0;
	spec.m_callback = NULL;
	lantern.Build(&spec, 1);

	if (g_language == 5) {
		lantern.SetBackdrop(0xdd1);
		lantern.Present();
		g_videoManager->CopyFrontToBack();

		if (g_videoManager->PumpFrame(2) == 0) {
			i = 0;

			while (1) {
				if (i >= 0x15e) {
					break;
				}

				i++;
				g_inputManager->Poll();

				if (g_inputManager->m_buttons & 0x10) {
					break;
				}

				if (g_videoManager->PumpFrame(2)) {
					break;
				}
			}
		}
	}
}

// FUNCTION: TONY2 0x00410680
void GameManager::PlayIntroMovies(TonyS32 p_firstRun)
{
	char buf[0x100];

	if (g_settings->ReadInt("video", 1) == 1) {
		if (g_language >= 0 && g_language <= 0) {
			strcpy(buf, "graphics\\movies\\intro1.mpg");
		}

		if (g_language >= 1 && g_language <= 4) {
			strcpy(buf, "graphics\\movies\\intro3.mpg");
		}

		if (g_language >= 5 && g_language <= 9) {
			strcpy(buf, "graphics\\movies\\intro2.mpg");
		}

		if (PathFileExistsA("graphics\\movies\\logos.mpg") == TRUE && g_language != 5 && p_firstRun == 1) {
			PlayMovie(
				g_videoManager->m_ddraw,
				g_videoManager->m_frontSurface,
				"graphics\\movies\\logos.mpg",
				PollActionButton
			);
		}

		if (PathFileExistsA(buf) == TRUE) {
			PlayMovie(g_videoManager->m_ddraw, g_videoManager->m_frontSurface, buf, PollActionButton);
		}
	}

	g_videoManager->ClearScreens();
	ShowGermanNotice();
}

// FUNCTION: TONY2 0x00410890
LRESULT CALLBACK GameWindowProc(HWND p_hWnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam)
{
	switch (p_msg) {
	case WM_CLOSE:
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ACTIVATEAPP:
		g_appActive = p_wParam;
		break;
	case WM_SETCURSOR:
		SetCursor(NULL);
		return 1;
	case WM_CHAR:
		if ((p_wParam >= 'a' && p_wParam <= 'z') || (p_wParam >= '0' && p_wParam <= '9')) {
			g_gameManager->HandleCheatChar(p_wParam);
		}
		break;
	}

	return DefWindowProcA(p_hWnd, p_msg, p_wParam, p_lParam);
}

// FUNCTION: TONY2 0x00410920
int WINAPI WinMain(HINSTANCE p_hInstance, HINSTANCE, LPSTR p_lpCmdLine, int p_nShowCmd)
{
	char buf[0x100];
	char buf2[0x100];
	TonyU16 wbuf[0x100];

	strcpy(g_appDir, GetCommandLineA());
	PathRemoveArgsA(g_appDir);
	PathUnquoteSpacesA(g_appDir);
	PathRemoveFileSpecA(g_appDir);
	TrimTrailingBackslash(g_appDir);
	srand(1234);
	SetCurrentDirectoryA(g_appDir);

	g_gameManager = new GameManager("Kellogg\\NewAdventures");

	strcpy(g_mapsDir, g_appDir);
	strcat(g_mapsDir, "\\maps");
	GetTempPathA(0x100, g_tempDir);
	TrimTrailingBackslash(g_tempDir);
	OpenGameArchive("result.ff");

	g_videoManager = new VideoManager();
	g_inputManager = new InputManager();
	g_objectManager = new ObjectManager();

	g_hWnd =
		g_videoManager
			->CreateGameWindow(p_hInstance, "Tony 2", "Tony & Friends - New Adventures", GameWindowProc, p_nShowCmd);

	if (!g_hWnd) {
		g_videoManager->ShutdownVideo(0);
		return 0;
	}

	SetCursor(NULL);

	if (!g_inputManager->Init(p_hInstance, g_hWnd)) {
		g_inputManager->ReleaseDevices();
		return 0;
	}

	g_backgroundRenderer = new BackgroundRenderer();

	if (strlen(p_lpCmdLine) == 0) {
		g_settings->WriteInt("video", 1);
	}

	if (strcmp(p_lpCmdLine, "novideo") == 0) {
		g_settings->WriteInt("video", 0);
	}

	g_gameManager->PlayIntroMovies(1);
	g_gameManager->InitSound();

	if (PathFileExistsA(p_lpCmdLine)) {
		sprintf(buf, "%s\\map%d.klg", g_tempDir, 0x18);
		CopyFileA(p_lpCmdLine, buf, 0);
		SetFileAttributesA(buf, 0x80);
		GetFileTitleA(p_lpCmdLine, buf, 0xff);
		PathRemoveExtensionA(buf);
		MultiByteToWideChar(0, 1, buf, -1, (LPWSTR) wbuf, 0x100);
		_wcsupr((wchar_t*) wbuf);
	}
	else {
		sprintf(buf, "%s\\intro.klg", g_mapsDir);
		sprintf(buf2, "%s\\map%d.klg", g_tempDir, 0x18);
		CopyFileA(buf, buf2, 0);
		SetFileAttributesA(buf2, 0x80);
		wcscpy((wchar_t*) wbuf, (wchar_t*) LangGetString(0x31));
	}

	g_gameManager->RunMainMenu(wbuf);
	g_gameManager->ShutdownSound();

	BackgroundRenderer* renderer = g_backgroundRenderer;
	if (renderer) {
		renderer->Destroy();
		delete renderer;
	}

	g_backgroundRenderer = NULL;

	InputManager* input = g_inputManager;
	if (input) {
		input->Shutdown();
		delete input;
	}

	VideoManager* video = g_videoManager;
	if (video) {
		video->Destroy();
		delete video;
	}

	ObjectManager* objects = g_objectManager;
	if (objects) {
		objects->DestroyAll();
		delete objects;
	}

	CloseGameArchive();

	GameManager* game = g_gameManager;
	if (game) {
		game->Shutdown();
		delete game;
	}

	return 0;
}

// FUNCTION: TONY2 0x00410e40
void OverlayData::FreePayload()
{
	if (m_payload != NULL) {
		free(m_payload);
	}
}

// FUNCTION: TONY2 0x00410e60
void __fastcall InitObjectFromData(GameObject* p_object, OverlayData* p_block)
{
	AssignObjectType(p_object, (ObjectTemplate*) p_block);
	p_object->m_initFn(p_object, (ObjectTemplate*) p_block);
}

// FUNCTION: TONY2 0x00410e80
void OverlayData::SpawnOnce()
{
	GameObject* object = g_objectManager->AllocObject();

	AssignObjectType(object, (ObjectTemplate*) this);

	if (object->m_reinitFn) {
		object->m_reinitFn(object, (ObjectTemplate*) this);
	}

	g_objectManager->FreeObject(object);
}

// FUNCTION: TONY2 0x00410ec0
void __fastcall AssignObjectType(GameObject* p_object, ObjectTemplate* p_template)
{
	switch (p_template->m_head.m_type) {
	case e_backdrop:
		p_object->m_initFn = BackdropInit;
		p_object->m_reinitFn = SceneryReinit;
		break;
	case e_playerBase:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) PlayerInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) PlayerReinit;
		break;
	case e_screenTile:
		p_object->m_initFn = ScreenTileInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) NoOpHandler;
		break;
	case e_collectible:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) CollectibleInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) CollectibleReinit;
		break;
	case e_text:
		p_object->m_initFn = TextInit;
		p_object->m_reinitFn = TextReinit;
		break;
	case e_container:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) CarrierInit;
		p_object->m_reinitFn = NULL;
		break;
	case e_group:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) GroupInit;
		p_object->m_reinitFn = NULL;
		break;
	case e_scenery:
		p_object->m_initFn = SceneryInit;
		p_object->m_reinitFn = SceneryReinit;
		break;
	case e_sprite:
		p_object->m_initFn = SpriteInit;
		p_object->m_reinitFn = ResolveTemplateFrameSet;
		break;
	case e_banner:
		p_object->m_initFn = BannerInit;
		p_object->m_reinitFn = SceneryReinit;
		break;
	case e_cameraScript:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) PlatformInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) PlatformReinit;
		break;
	case e_prop:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) PropInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) NoOpHandler;
		break;
	case e_critter:
		p_object->m_initFn = CritterInit;
		p_object->m_reinitFn = CritterReinit;
		break;
	case e_gauge:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) SegmentDisplayInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) SegmentDisplayReinit;
		break;
	case e_staticEnemy:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) StaticEnemyInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) StaticEnemyReinit;
		break;
	case e_mover:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) MoverInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) EnemyReinit;
		break;
	case e_goal:
		p_object->m_initFn = GoalInit;
		p_object->m_reinitFn = ResolveTemplateFrameSet;
		break;
	case e_hopper:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) HopperInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) HopperReinit;
		break;
	case e_checkpoint:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) CheckpointInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) CheckpointReinit;
		break;
	case e_spring:
		p_object->m_initFn = SpringInit;
		p_object->m_reinitFn = ResolveTemplateFrameSet;
		break;
	case e_door:
		p_object->m_initFn = DoorInit;
		p_object->m_reinitFn = ResolveTemplateFrameSet;
		break;
	case e_water:
		p_object->m_initFn = WaterInit;
		p_object->m_reinitFn = WaterReinit;
		break;
	case e_bouncer:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) BouncerInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) EnemyReinit;
		break;
	case e_dispenser:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) DispenserInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) DispenserReinit;
		break;
	case e_dropItem:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) DropItemInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) CollectibleReinit;
		break;
	case e_splash:
		p_object->m_initFn = SplashInit;
		p_object->m_reinitFn = ResolveTemplateFrameSet;
		break;
	case e_session:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) SessionInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) SessionReinit;
		break;
	case e_charSelect:
		p_object->m_initFn = CharSelectInit;
		p_object->m_reinitFn = ResolveTemplateFrameSet;
		break;
	case e_speechTrigger:
		p_object->m_initFn = SpeechTriggerInit;
		p_object->m_reinitFn = ResolveTemplateFrameSet;
		break;
	case e_speech:
		p_object->m_initFn = SpeechInit;
		p_object->m_reinitFn = SpeechReinit;
		break;
	case e_worldMap:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) WorldMapInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) WorldMapReinit;
		break;
	case e_boss:
		p_object->m_initFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) BossInit;
		p_object->m_reinitFn = (void(__fastcall*)(GameObject*, ObjectTemplate*)) BossReinit;
		break;
	}
}
