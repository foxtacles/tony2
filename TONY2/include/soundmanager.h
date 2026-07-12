#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include "decomp.h"
#include "types.h"

// Sound system: GSM voice-library banks, music tracks and speech buffers. Created by
// GameManager::InitSound with new(0x1d4) and published through g_soundManager.
// SIZE 0x1d4
class SoundManager {
public:
	SoundManager(TonyS32 p_music, TonyS32 p_sfx);

	// Per-slot record.
	// SIZE 0x0c
	struct SoundSlot {
		TonyU16 m_bank;   // 0x00
		TonyU16 m_entry;  // 0x02
		void* m_data;     // 0x04
		TonyS32 m_handle; // 0x08
	};

	void Shutdown();
	void PlaySong(TonyS32 p_track);
	TonyS32 StopSong();
	TonyS32 FadeOutSong();
	TonyS32 LoadBanks(char* p_name);
	void FreeBanks();
	TonyS32 LoadSong(char* p_name, TonyU16 p_bank, TonyU16 p_entry);
	void FreeSongs();
	void RegisterSongBank(TonyU16 p_track);
	TonyS32 PlaySample(TonyS32 p_sound, TonyS32 p_pan, TonyS32 p_level);
	void SetEnabled(TonyS32 p_music, TonyS32 p_sfx);
	void ReregisterSongBanks();
	void StopHandle(void* p_handle);
	TonyS32 SuspendSong();
	void ResumeSong(TonyS32 p_track);
	TonyS32 SetSongVolume(TonyU8 p_volume);
	void LoadSpeechBank(char* p_name);
	void PlaySpeech(TonyS32 p_block);
	TonyS32 IsSpeechPlaying();
	void UnloadSpeechBank();
	void StopSpeech();

	TonyS32 m_musicOn;          // 0x00
	TonyS32 m_sfxOn;            // 0x04
	void* m_bankPool;           // 0x08
	void* m_bankProj;           // 0x0c
	void* m_bankSamp;           // 0x10
	void* m_bankSdir;           // 0x14
	SoundSlot m_songs[0x20];    // 0x18
	TonyS32 m_sampleRate;       // 0x198
	TonyS32 m_channels;         // 0x19c
	TonyS32 m_stereo;           // 0x1a0
	TonyU16 m_registered[0x10]; // 0x1a4
	TonyS32 m_registeredCount;  // 0x1c4
	TonyS32 m_currentSong;      // 0x1c8
	void* m_speechBank;         // 0x1cc
	TonyS32 m_speechHandle;     // 0x1d0
};

DECOMP_SIZE_ASSERT(SoundManager, 0x1d4)

extern SoundManager* g_soundManager;

#endif // SOUNDMANAGER_H
