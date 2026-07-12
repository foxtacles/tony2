#include "inputmanager.h"

#include "engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma function(strcmp)

// Archive handle slot (16 bytes each in the g_archiveStreams table).
struct ArchiveStream {
	TonyS32 m_open;     // 0x00
	TonyS32 m_position; // 0x04
	TonyS32 m_size;     // 0x08
	TonyS32* m_entry;   // 0x0c
};

TonyS32 __cdecl ArchiveCompareEntries(const void* p_a, const void* p_b);

// GLOBAL: TONY2 0x0045c53c
InputManager* g_inputManager;

// GLOBAL: TONY2 0x0045c540
TonyS32 g_archiveStreamCount;

// GLOBAL: TONY2 0x0045c544
TonyS32* g_archiveEntries;

// GLOBAL: TONY2 0x0045c548
HLOCAL g_archiveStreams;

// GLOBAL: TONY2 0x0045c54c
void* g_archiveDir;

// GLOBAL: TONY2 0x0045c558
TonyS32 g_archiveKeepOpen;

// GLOBAL: TONY2 0x0045c55c
TonyS32 g_archiveLastOffset;

// GLOBAL: TONY2 0x0045c560
FILE* g_archiveFile;

// GLOBAL: TONY2 0x0045c568
TonyS32 g_archiveEntryCount;

// FUNCTION: TONY2 0x004051e0
InputManager::InputManager()
{
}

// FUNCTION: TONY2 0x004051f0
void InputManager::Shutdown()
{
	ReleaseDevices();
}

// Fully implemented, kept as STUB because it compares at 86%: the original hoists the sound
// library call's argument pushes above the config-table stores and keeps a callee-saved zero
// (edi) alive for the last argument, the two word stores, and the result comparison; cl
// 11.00.7022 reuses the memset zero eagerly from this source (chained assignment and statement
// order variants tested). Re-annotate as FUNCTION when a matching form is found.
// STUB: TONY2 0x00405200
TonyBool32 InputManager::Init(HINSTANCE p_hInstance, HWND)
{
	memset(&m_bindings[0], 0, 0x40);
	m_bindings[0] = (DIK_UP << 16) | 0x0001;
	m_bindings[1] = (DIK_DOWN << 16) | 0x0002;
	m_bindings[2] = (DIK_LEFT << 16) | 0x0004;
	m_bindings[3] = (DIK_RIGHT << 16) | 0x0008;
	m_bindings[4] = (DIK_ESCAPE << 16) | 0x0080;
	m_bindings[5] = (DIK_SPACE << 16) | 0x0010;
	m_bindings[6] = (DIK_RETURN << 16) | 0x0010;
	m_prevButtons = m_edgeMask = 0;

	if (DirectInputCreateA((HINSTANCE) p_hInstance, 0x500, (LPDIRECTINPUT*) this, NULL) < 0) {
		return 0;
	}

	if (!InitKeyboard()) {
		return 0;
	}

	return InitJoystick() != 0;
}

// FUNCTION: TONY2 0x004052b0
void InputManager::ReleaseDevices()
{
	if (m_keyboard) {
		m_keyboard->Unacquire();
		m_keyboard->Release();
	}

	if (m_joystick) {
		m_joystick->Unacquire();
		m_joystick->Release();
	}

	if (m_directInput) {
		m_directInput->Release();
	}
}

// Fully implemented, kept as STUB because it compares at 69%: every call, flag test and
// the edge-detection tail match, but the original keeps the dead GetDeviceState result
// in a stack slot (frame 0x54 vs 0x50; even an array store is eliminated here) and walks
// the key map with a per-iteration cursor copy that no pointer/cast spelling reproduces.
// Same allocator-margin family as RefreshHitBoxes. Re-annotate as FUNCTION when a matching
// form is found.
// STUB: TONY2 0x004052f0
void InputManager::Poll()
{
	DIJOYSTATE js;
	TonyU32* entry;
	TonyS32 result;
	TonyU16 current;
	TonyU16 mask;

	m_buttons = 0;

	if (g_appActive == 1) {
		m_keyboard->GetDeviceState(sizeof(m_keyState), m_keyState);

		if (m_bindings[0]) {
			entry = (TonyU32*) &m_bindings[0];

			do {
				if (m_keyState[*entry >> 16] & 0x80) {
					m_buttons |= (TonyU16) *entry;
				}

				entry++;
			} while (*entry);
		}

		if (m_joystick) {
			m_joystick->Poll();

			result = m_joystick->GetDeviceState(sizeof(DIJOYSTATE), &js);

			if (js.lX < -500) {
				m_buttons |= 4;
			}
			else if (js.lX > 500) {
				m_buttons |= 8;
			}

			if (js.lY < -500) {
				m_buttons |= 1;
			}
			else if (js.lY > 500) {
				m_buttons |= 2;
			}

			if (js.rgbButtons[0] & 0x80) {
				m_buttons |= 0x10;
			}

			if (js.rgbButtons[1] & 0x80) {
				m_buttons |= 0x20;
			}

			if (js.rgbButtons[2] & 0x80) {
				m_buttons |= 0x40;
			}

			if (js.rgbButtons[9] & 0x80) {
				m_buttons |= 0x80;
			}
		}
	}

	current = m_buttons;
	mask = m_edgeMask & m_prevButtons;
	m_prevButtons = current;
	m_buttons = current & ~mask;
}

// Fully implemented, kept as STUB because it compares at 94%: all eight DirectInput
// calls, the dead-arg-slot device pointer, both DIPROPRANGE fills and every fail path
// match, but the recompile loads the context argument into eax before the prologue and
// copies it to esi (the original loads it into esi directly after the push), and the
// two range-fill blocks load m_joystick after the field stores instead of before. Same
// scheduling-margin family as RefreshHitBoxes. Re-annotate as FUNCTION when a matching
// form is found.
// STUB: TONY2 0x00405430
BOOL __stdcall JoystickEnumCallback(LPCDIDEVICEINSTANCEA p_instance, LPDIRECTINPUTDEVICE p_device)
{
	DIPROPRANGE range;
	InputManager* self = (InputManager*) p_device;

	if (self->m_directInput->CreateDevice(p_instance->guidInstance, &p_device, NULL) < 0) {
		return DIENUM_CONTINUE;
	}

	if (p_device->QueryInterface(IID_IDirectInputDevice2, (LPVOID*) &self->m_joystick) < 0) {
		return DIENUM_CONTINUE;
	}

	p_device->Release();

	if (self->m_joystick->SetDataFormat(&c_dfDIJoystick) < 0) {
		self->m_joystick->Release();
		return DIENUM_CONTINUE;
	}

	if (self->m_joystick->SetCooperativeLevel(g_hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) < 0) {
		self->m_joystick->Release();
		return DIENUM_CONTINUE;
	}

	range.diph.dwSize = sizeof(DIPROPRANGE);
	range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	range.diph.dwObj = DIJOFS_X;
	range.diph.dwHow = DIPH_BYOFFSET;
	range.lMin = -1000;
	range.lMax = 1000;

	if (self->m_joystick->SetProperty(DIPROP_RANGE, &range.diph) < 0) {
		self->m_joystick->Release();
		return DIENUM_CONTINUE;
	}

	range.diph.dwSize = sizeof(DIPROPRANGE);
	range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	range.diph.dwObj = DIJOFS_Y;
	range.diph.dwHow = DIPH_BYOFFSET;
	range.lMin = -1000;
	range.lMax = 1000;

	if (self->m_joystick->SetProperty(DIPROP_RANGE, &range.diph) < 0) {
		self->m_joystick->Release();
		return DIENUM_CONTINUE;
	}

	if (self->m_joystick->Acquire() < 0) {
		self->m_joystick->Release();
		return DIENUM_CONTINUE;
	}

	return DIENUM_STOP;
}

// FUNCTION: TONY2 0x004055b0
TonyS32 InputManager::InitJoystick()
{
	m_joystick = NULL;
	m_directInput
		->EnumDevices(DIDEVTYPE_JOYSTICK, (LPDIENUMDEVICESCALLBACKA) JoystickEnumCallback, this, DIEDFL_ATTACHEDONLY);
	return 1;
}

// FUNCTION: TONY2 0x004055d0
TonyS32 InputManager::InitKeyboard()
{
	if (m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL) < 0) {
		return 0;
	}

	if (m_keyboard->SetDataFormat(&c_dfDIKeyboard) < 0) {
		return 0;
	}

	if (m_keyboard->SetCooperativeLevel(g_hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) < 0) {
		return 0;
	}

	return m_keyboard->Acquire() >= 0;
}

// FUNCTION: TONY2 0x00405630
TonyU16 InputManager::SetEdgeMask(TonyU16 p_mask)
{
	TonyU16 old = m_edgeMask;
	m_edgeMask = p_mask;
	return old;
}

// bsearch comparator for the archive directory: entries are (data offset, name offset)
// pairs whose names live in the directory blob.
// FUNCTION: TONY2 0x00405930
TonyS32 __cdecl ArchiveCompareEntries(const void* p_a, const void* p_b)
{
	return strcmp((char*) g_archiveDir + ((TonyS32*) p_a)[1], (char*) g_archiveDir + ((TonyS32*) p_b)[1]);
}

// FUNCTION: TONY2 0x00405960
TonyS32 __fastcall ArchiveMount(char* p_file, TonyS32 p_count)
{
	g_archiveKeepOpen = 0;
	ArchiveUnmount();

	g_archiveStreams = LocalAlloc(LMEM_ZEROINIT, p_count << 4);
	if (!g_archiveStreams) {
		return 0;
	}

	g_archiveStreamCount = p_count;

	g_archiveFile = fopen(p_file, "rb");
	if (!g_archiveFile) {
		return 0;
	}

	TonyS32* header = (TonyS32*) LocalAlloc(LMEM_ZEROINIT, 8);
	if (fread(header, 4, 2, g_archiveFile) <= 0) {
		LocalFree(header);
		return 0;
	}

	fseek(g_archiveFile, 0, 0);

	g_archiveDir = malloc(header[1] - 1);
	if (fread(g_archiveDir, 1, header[1] - 1, g_archiveFile) <= 0) {
		free(g_archiveDir);
		return 0;
	}

	LocalFree(header);
	g_archiveEntryCount = *(TonyS32*) g_archiveDir;
	g_archiveEntries = (TonyS32*) g_archiveDir + 1;
	g_archiveLastOffset = g_archiveEntries[g_archiveEntryCount * 2 - 2];
	return 1;
}

// FUNCTION: TONY2 0x00405a70
void ArchiveUnmount()
{
	if (g_archiveKeepOpen == 0) {
		if (g_archiveFile) {
			fclose(g_archiveFile);
			g_archiveFile = NULL;
		}

		if (g_archiveDir) {
			free(g_archiveDir);
			g_archiveDir = NULL;
		}

		if (g_archiveStreams) {
			LocalFree(g_archiveStreams);
			g_archiveStreams = NULL;
		}

		g_archiveStreamCount = 0;
		g_archiveEntryCount = 0;
		g_archiveEntries = NULL;
	}
}

// Fully implemented, kept as STUB because it compares at 81%: the guards, bsearch key,
// directory lookup, free-slot scan and handle fill all match; the recompile hoists the
// handle-table base load above the zero-trip guard where the original loads it in the
// loop preheader, and merges the scan-exhausted return with the shared NULL epilogue
// where the original duplicates it. Allocator/scheduler margin. Re-annotate when solved.
// STUB: TONY2 0x00405ae0
void* __fastcall ArchiveOpen(char* p_name)
{
	TonyS32 key[2];
	TonyS32* entry;
	TonyU32 i;

	if (g_archiveEntries == NULL || p_name == NULL || *p_name == 0) {
		return NULL;
	}

	key[1] = p_name - (char*) g_archiveDir;
	entry = (TonyS32*) bsearch(key, g_archiveEntries, g_archiveEntryCount, 8, ArchiveCompareEntries);

	if (entry == NULL) {
		return NULL;
	}

	for (i = 0; i < (TonyU32) g_archiveStreamCount; i++) {
		if (((ArchiveStream*) g_archiveStreams)[i].m_open == 0) {
			break;
		}
	}

	if (i >= (TonyU32) g_archiveStreamCount) {
		return NULL;
	}

	((ArchiveStream*) g_archiveStreams)[i].m_open = 1;
	((ArchiveStream*) g_archiveStreams)[i].m_position = entry[0];
	((ArchiveStream*) g_archiveStreams)[i].m_size = entry[2] - entry[0];
	((ArchiveStream*) g_archiveStreams)[i].m_entry = entry;
	return &((ArchiveStream*) g_archiveStreams)[i];
}

// FUNCTION: TONY2 0x00405ba0
TonyS32 __fastcall ArchiveClose(void* p_handle)
{
	if (p_handle != NULL && ((ArchiveStream*) p_handle)->m_open == 1) {
		((ArchiveStream*) p_handle)->m_open = 0;
		return 1;
	}

	return 0;
}

// FUNCTION: TONY2 0x00405bc0
TonyU32 __fastcall ArchiveRead(void* p_handle, void* p_buffer, TonyS32 p_count)
{
	TonyU32 read;

	if (p_handle != NULL && ((ArchiveStream*) p_handle)->m_open == 1 && p_count >= 0) {
		if (((ArchiveStream*) p_handle)->m_position + p_count > ((ArchiveStream*) p_handle)->m_entry[2]) {
			p_count = ((ArchiveStream*) p_handle)->m_entry[2] - ((ArchiveStream*) p_handle)->m_position;
		}

		fseek(g_archiveFile, ((ArchiveStream*) p_handle)->m_position, 0);
		read = fread(p_buffer, 1, p_count, g_archiveFile);
		((ArchiveStream*) p_handle)->m_position += p_count;
		return read;
	}

	return 0;
}

// Fully implemented, kept as STUB because it compares at 39% on a single register-pair
// swap that cascades through every branch offset: the original dispatches the origin from
// eax and keeps the directory entry in esi, while cl 11.00.7022 assigns them the other
// way round regardless of declaration order or spelling. All three seek modes and their
// bounds checks match. Zero/register-seeding family (see TickAll). Re-annotate when
// the vintage is found.
// STUB: TONY2 0x00405c30
TonyS32 __fastcall ArchiveSeek(void* p_handle, TonyS32 p_offset, TonyS32 p_origin)
{
	TonyS32* entry;
	TonyS32 pos;

	if (p_handle == NULL || ((ArchiveStream*) p_handle)->m_open != 1) {
		return 0;
	}

	entry = ((ArchiveStream*) p_handle)->m_entry;

	switch (p_origin) {
	case 2:
		if (p_offset < 0 || p_offset >= ((ArchiveStream*) p_handle)->m_size) {
			return 0;
		}

		((ArchiveStream*) p_handle)->m_position = entry[2] - p_offset;
		return 1;
	case 1:
		pos = ((ArchiveStream*) p_handle)->m_position + p_offset;

		if (pos < entry[0]) {
			return 0;
		}

		if (pos >= entry[2]) {
			return 0;
		}

		((ArchiveStream*) p_handle)->m_position = pos;
		return 1;
	case 0:
		if (p_offset < 0 || p_offset >= ((ArchiveStream*) p_handle)->m_size) {
			return 0;
		}

		((ArchiveStream*) p_handle)->m_position = p_offset + entry[0];
		return 1;
	}

	return 0;
}

// FUNCTION: TONY2 0x00405ca0
TonyS32 __fastcall ArchiveTell(void* p_handle)
{
	if (p_handle != NULL && ((ArchiveStream*) p_handle)->m_open == 1) {
		return ((ArchiveStream*) p_handle)->m_position - ((ArchiveStream*) p_handle)->m_entry[0];
	}

	return -1;
}
