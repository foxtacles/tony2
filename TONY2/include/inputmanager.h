#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "decomp.h"
#include "types.h"

#include <dinput.h>
#include <stdio.h>
#include <windows.h>

// Input system (DirectInput keyboard + optional joystick). Created in WinMain (0x00410920) with new(0x154) and
// published through g_inputManager. SIZE 0x154
class InputManager {
public:
	InputManager();

	void Shutdown();
	TonyBool32 Init(HINSTANCE p_hInstance, HWND p_hWnd);
	void ReleaseDevices();
	void Poll();
	TonyS32 InitJoystick();
	TonyS32 InitKeyboard();
	TonyU16 SetEdgeMask(TonyU16 p_mask);

	LPDIRECTINPUT m_directInput;     // 0x00
	LPDIRECTINPUTDEVICE m_keyboard;  // 0x04
	LPDIRECTINPUTDEVICE2 m_joystick; // 0x08
	TonyU8 m_keyState[0x10c - 0x0c]; // 0x0c
	TonyU32 m_bindings[0x10];        // 0x10c (scancode << 16 | button mask, zero-terminated)
	TonyU16 m_buttons;               // 0x14c
	TonyU16 m_prevButtons;           // 0x14e
	TonyU16 m_edgeMask;              // 0x150
};

DECOMP_SIZE_ASSERT(InputManager, 0x154)

BOOL __stdcall JoystickEnumCallback(LPCDIDEVICEINSTANCEA p_instance, LPDIRECTINPUTDEVICE p_device);

extern InputManager* g_inputManager;
extern "C" HWND g_hWnd;
extern TonyS32 g_appActive;
extern TonyS32 g_archiveStreamCount;
extern TonyS32* g_archiveEntries;
extern HLOCAL g_archiveStreams;
extern void* g_archiveDir;
extern TonyS32 g_archiveKeepOpen;
extern TonyS32 g_archiveLastOffset;
extern FILE* g_archiveFile;
extern TonyS32 g_archiveEntryCount;

#endif // INPUTMANAGER_H
