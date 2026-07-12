#ifndef ENGINE_H
#define ENGINE_H

#include "types.h"

#include <ddraw.h>
#include <windows.h>

struct GameObject;
struct OverlayData;
struct ObjectTemplate;

// Free engine functions without an identified owning class yet.

extern TonyS32 g_language;
extern TonyS32 g_charsetCount;
extern TonyU16 g_charsetCodes[0x100];
extern TonyU8 g_charsetReverse[0x10000];
extern TonyU16** g_langStrings[16];
extern TonyS32 g_langStringCounts[16];
// C linkage: referenced from both STRICT (afx) and non-STRICT TUs, and the sound
// library is a C library - keep the mangling handle-type-independent.
extern "C" HWND g_hWnd;
extern char g_cheatBuffer[0x20];

void __fastcall ShowErrorMessage(TonyS32 p_error);
TonyS32 __fastcall HighestBit(TonyU32 p_mask);
TonyS32 __fastcall LowestBit(TonyU32 p_mask);

// Game-side timer/allocation services called by the sound library (C linkage). The
// library prototypes the timer stop with the tick callback; the implementation ignores
// it.
extern "C"
{
	void* __stdcall ServiceAlloc(TonyU32 p_size, TonyU32 p_unused);
	void __stdcall ServiceFree(void* p_memory);
	TonyBool __fastcall ServiceStartTimer(void (*p_tick)());
	TonyBool __fastcall ServiceStopTimer(void (*p_tick)());
}

// Third-party sound library entry points (C library, VC6-built - see soundlib.c).
extern "C"
{
	void __fastcall SndFree(void* p_memory);
	TonyS32 __fastcall SamplePlay(TonyS32 p_sound, TonyU8 p_level, TonyU8 p_pan);
	TonyS32 __fastcall HandleRelease(void* p_handle);
	void SndStopAll();
	void __fastcall TrackFadeTo(TonyU8 p_level, TonyS32 p_span, TonyU8 p_track);
	TonyU8 SndIsIdle();
	TonyS32 __fastcall SndOpenDSound(
		TonyS32 p_rate,
		TonyU8 p_channels,
		TonyU8 p_depth,
		TonyU8 p_classes,
		TonyU8 p_stereo,
		TonyU32 p_mode,
		TonyU32 p_config,
		HWND p_hWnd
	);
	void SndClose();
	void SeqClose();
	void __fastcall SampleStop(TonyS32 p_handle);
	void __fastcall SamplePause(TonyS32 p_handle);
	void __fastcall SampleFadeVolume(TonyU8 p_volume, TonyS32 p_span, TonyS32 p_handle, TonyU8 p_mode);
	TonyU8 __fastcall BankRegister(void* p_proj, TonyU16 p_id, void* p_samp, void* p_sdir, void* p_pool);
	TonyS32 __fastcall StreamPlayEntry(TonyU16 p_bank, TonyU16 p_entry, void* p_data, TonyS32 p_start);
	TonyS32 __fastcall SpeechPlayBlock(
		TonyU32 p_block,
		TonyU32 p_start,
		TonyU8 p_priority,
		TonyU8 p_level,
		TonyU8 p_pan,
		TonyU8 p_boost,
		TonyU8 p_rateScale
	);
	TonyU8 __fastcall SpeechStop(TonyS32 p_handle);
	TonyU8 __fastcall SpeechIsPlaying(TonyS32 p_handle);
	TonyU8 __fastcall SpeechSubmitBlock(TonyS32* p_data, TonyS32 p_channel);
	void SpeechStartup();
	void SpeechShutdown();
}

TonyS32 __fastcall ArchiveMount(char* p_file, TonyS32 p_count);
void ArchiveUnmount();
void __fastcall TrimTrailingBackslash(char* p_path);
void __fastcall OpenGameArchive(char* p_file);
void CloseGameArchive();
void __fastcall LangLoadCharset(char* p_file, TonyS32 p_language);
void LangFreeStrings();
void __fastcall LangLoadStrings(TonyS32 p_language);
TonyU16* __fastcall LangGetString(TonyS32 p_string);
void __fastcall WideToAnsi(TonyU16* p_string, char* p_dest, TonyS32 p_size);
TonyS32 __fastcall PlayMovie(
	LPDIRECTDRAW p_ddraw,
	LPDIRECTDRAWSURFACE p_surface,
	char* p_file,
	TonyS32 (*p_callback)()
);

extern TonyS32 g_blankRow[0x1770];

void __fastcall AssignObjectType(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall InitObjectFromData(GameObject* p_object, OverlayData* p_block);
void __fastcall BlitSpriteFast(TonyU16* p_sprite, TonyS32 p_x, TonyS32 p_y, TonyS32 p_arg, TonyU8* p_surface);
void __fastcall BlitSpriteClipY(
	TonyU16* p_data,
	void* p_rows,
	TonyS32 p_x,
	TonyS32 p_y,
	TonyS32 p_arg,
	TonyU8* p_surface
);
void __fastcall BlitSpriteClipXY(
	TonyU16* p_data,
	void* p_rows,
	TonyS32 p_x,
	TonyS32 p_y,
	TonyS32 p_arg,
	TonyU8* p_surface
);
void __fastcall BlitSprite(TonyU16* p_data, void* p_rows, TonyS32 p_x, TonyS32 p_y, TonyS32 p_arg, TonyU8* p_surface);
void __fastcall LightAllNutrients(GameObject* p_object);
void __fastcall SpawnCheatFlakes(GameObject* p_object);
void __fastcall SetSegment(GameObject* p_object, TonyS32 p_index, TonyS32 p_on);
void __fastcall ObjectSetFlags(GameObject* p_object, TonyS32 p_set, TonyS32 p_clear);
void __fastcall ObjectSetMoveFlags(GameObject* p_object, TonyS32 p_set, TonyS32 p_clear);
void __fastcall ObjectFreeTemplate(GameObject* p_object);
TonyS32 __fastcall PlayObjectSound(GameObject* p_object, TonyS32 p_sound, TonyS32 p_pan, TonyS32 p_mode);
void __fastcall SetSpriteAnchor(GameObject* p_object, TonyS32 p_anchor);
void __fastcall SetObjectText(GameObject* p_object, TonyS32 p_sprite);
void* __fastcall ArchiveOpen(char* p_name);
TonyS32 __fastcall ArchiveClose(void* p_handle);
TonyU32 __fastcall ArchiveRead(void* p_handle, void* p_buffer, TonyS32 p_count);
TonyS32 __fastcall ArchiveSeek(void* p_handle, TonyS32 p_offset, TonyS32 p_origin);
TonyS32 __fastcall ArchiveTell(void* p_handle);
void __fastcall LangSetLanguage(TonyS32 p_language);
TonyS32 LangGetCharsetSize();
void __fastcall WideToGlyphs(TonyU16* p_src, char* p_dest);
TonyS32 __fastcall ReadFileBlob(char* p_name, void** p_dest);
void TickAndDrawObjects();

#endif // ENGINE_H
