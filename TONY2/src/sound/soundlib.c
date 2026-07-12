// Third-party sound library ("DirectSound, V2.1 (Dec 10 1998)" banner region) with the
// GSM voice codec (gsmvoice.c per its assert strings). Compiled with VC6 RTM
// (cl 12.00.8168) at /Od — see the TONY2_VC6_DIR CMake option.

#include "decomp.h"
#include "types.h"

#include <stdlib.h>
#include <windows.h>

// CRT assertion failure handler (LIBCMT, non-NDEBUG vintage in the original library).
void __cdecl _assert(void* p_expression, void* p_file, unsigned p_line);
char* __cdecl strcpy(char* p_dest, const char* p_source);
void* __cdecl memset(void* p_dest, TonyS32 p_value, TonyU32 p_size);

// DSOUND.dll imports, reached through linker-generated jmp thunks (the TU declares
// them without dllimport).
TonyS32 __stdcall DirectSoundCreate(struct RawGuid* p_guid, struct IDSDevice** p_out, void* p_outer);
TonyS32 __stdcall DirectSoundEnumerateA(
	TonyS32(__stdcall* p_callback)(struct RawGuid*, char*, char*, void*), void* p_context);

TonyS32 __stdcall DSndEnumCallback(struct RawGuid* p_guid, char* p_name, char* p_module, void* p_context);
void __fastcall SndDebugError(char* p_message, TonyS32 p_code);

// Game-side timer/allocation services (defined in engine.cpp with C linkage). The
// library's own prototypes pass the tick callback to both timer calls; the game-side
// implementations ignore the stop argument.
void* __stdcall ServiceAlloc(TonyU32 p_size, TonyU32 p_unused);
void __stdcall ServiceFree(void* p_memory);
TonyU8 __fastcall ServiceStartTimer(void (*p_tick)(void));
TonyU8 __fastcall ServiceStopTimer(void (*p_tick)(void));

TonyS32 __fastcall SamplePlayDefault(TonyS32 p_sound, TonyU8 p_b, TonyU8 p_pan);
TonyS32 __fastcall ChainRelease(void* p_a);
void __fastcall SeqStopVoices(TonyU8 p_a);
void __fastcall TrackFadeCommand(TonyU8 p_level, TonyU16 p_rate, TonyU8 p_command, TonyU8 p_d, TonyS32 p_e);
void __fastcall VoiceTeardown(TonyS32 p_voice);
TonyS32 __fastcall VoiceHandleOwner(TonyS32 p_handle);
TonyS32 __fastcall HandleReleaseOne(TonyS32 p_handle);
void __fastcall ChanArm(TonyS32 p_channel, struct SongDesc* p_desc, TonyU8 p_flag);
void __fastcall WaveDriverSetStep(TonyU8 p_channel, TonyS32 p_step);
void WaveClose(void);
void PoolRebuildFree(void);
void __fastcall WaveMasterLevelHook(TonyS32 p_a);
void WaveServiceHook(void);
void PendingRebuildFree(void);
void PendingClearCounter(void);
void MixerServiceTick(void);
void ChannelRetireCommands(void);
void __fastcall PendingUnlink(struct PendingCmd* p_node);
void __fastcall PendingActivate(struct PendingCmd* p_node);
void __fastcall ChannelProgramChange(struct SampleChannel* p_channel, TonyU8 p_program, TonyU8 p_route);
TonyS32 __fastcall ChannelMintHandle(TonyS32 p_slot);
void __fastcall BuildProgramMap(TonyU8* p_map, struct SampleRoute* p_records);
void __fastcall RouteLevel(TonyU8 p_value, TonyU8 p_channel);
void __fastcall RouteHold(TonyU8 p_value, TonyU8 p_channel);
struct PendingCmd* PendingTake(void);
void __fastcall PendingRelease(struct PendingCmd* p_node);
void __fastcall CmdApplyPlay(struct PlayCmd* p_cmd, TonyS32* p_out, TonyU8 p_direct);
void __fastcall ChannelRewindClock(TonyU32 p_pos);
TonyS32 __fastcall HandleValidate(TonyS32 p_handle);
TonyU8 ChannelPumpEvents(void);
void ChannelAdvanceCursors(void);
TonyU8 ChannelPumpScheduled(void);
void ChannelPumpBends(void);
void ChannelPumpWheel(void);
TonyU8 ChannelPumpTracks(void);
void ChannelDeriveStep(void);
void ChannelPumpTempo(void);
void ChannelAdvanceClock(void);
void __cdecl RingCopy(void* p_dest, void* p_source, TonyU32 p_bytes);
void MixerInitFpu(void);
TonyU32 __stdcall MixerThreadProc(void* p_param);
void ChannelPumpAll(void);
void StreamPump(void);
void DevVoiceRefreshGuards(void);
void DevVoicePumpAll(void);
void __fastcall WaveRingWrapGuard(TonyS16* p_buffer, TonyS32 p_length);
void __fastcall WaveDriverMix(void* p_channels, TonyU32 p_frames, void* p_out);
void DSndClose(void);
void __fastcall ChanStop(TonyS32 p_channel, TonyU8 p_flag);
void __fastcall VoiceCaptureFade(struct SeqVoice* p_slot, TonyS32 p_level);
TonyU32 __fastcall TickToWhole(TonyU32 p_value);
void SeqPumpVoices(void);
void ChannelResetAll(void);
void PendingResetWord(void);
void WaveOutputMix(void);
TonyS32 __fastcall WaveGetPlayPosition(TonyS32 p_voice);
void __fastcall WaveCacheFlushHook(TonyS16* p_samples, TonyU32 p_count);
void DevVoiceResetAll(void);
void SeqResetService(void);
void __fastcall BankUnregisterSequences(TonyU16* p_ids);
void __fastcall BankUnregisterWaves(TonyU16* p_ids);
void __fastcall BankUnregisterInstruments(TonyU16* p_ids);
void __fastcall BankUnregisterKits(TonyU16* p_ids);
void __fastcall BankUnregisterSongs(TonyU16* p_ids);
struct BankChunk* __fastcall BankFindChunk(TonyU16 p_id, TonyS32* p_bank);
struct BankChunk* __fastcall BankFindSongChunk(TonyU16 p_id, TonyS32* p_bank);
struct BankChunk* __fastcall BankFindTaggedChunk(TonyU16 p_id, TonyS32* p_bank);
struct BankChunk* __fastcall BankFindSequenceChunk(TonyU16 p_id, TonyS32* p_bank);
TonyS32 __fastcall SampleDataBase(TonyS32 p_data);
TonyS32 __fastcall BankFindSampleEntry(TonyU16 p_id, TonyS32 p_data, void* p_dir, TonyS32* p_addr);
struct BankChunk* __fastcall BankWalkChunks(TonyU16 p_id, struct BankChunk* p_chunk);
extern TonyU32 g_dsndBlockFrames;

// Active wave driver selector defined in soundwave.c.
extern TonyU8 g_waveDriverMode;

// Stream list heads defined in soundstream.c.
extern struct StreamSource* g_streamList;
extern struct StreamListener* g_streamListeners;
void __fastcall WaveDriverPrepare(TonyU8 p_channel, struct SongDesc* p_desc, TonyU8 p_flag);
void __fastcall SeqTickVoice(struct SeqVoice* p_slot);
void __fastcall VoiceRestartFade(struct SeqVoice* p_slot);
TonyS32 __fastcall VoiceBlendLevel(struct SeqVoice* p_slot);
TonyS16 __fastcall SndSineQ12(TonyU16 p_phase);
void __fastcall ChannelResume(TonyS32 p_handle);
void __fastcall ChannelSetPitchSlide(TonyS32 p_handle, TonyS32 p_b, TonyS32 p_c);
void __fastcall ChanArmFadeIn(TonyS32 p_channel);
void __fastcall ChanClearAck(TonyS32 p_channel);
void __fastcall ChanDriverHook(TonyS32 p_channel);
void __fastcall ChanSetPitchStep(TonyS32 p_channel, TonyS32 p_step);
TonyS32 __cdecl WaveRateSemitoneUp(TonyU16 p_value);
TonyS32 __cdecl WaveRateToStep(TonyS32 p_rate);
void __fastcall ChanLoadEnvelope(TonyS32 p_channel, TonyU16* p_desc);
TonyS32 __cdecl WaveRateSemitoneDown(TonyU16 p_value);
TonyS32 __cdecl WaveRatePitchUp(TonyU16 p_value, TonyU32 p_range);
TonyS32 __cdecl WaveRatePitchDown(TonyU16 p_value, TonyU32 p_range);
void __cdecl WaveRateFinePitchUpPair(TonyU16 p_pitch, TonyS32* p_low, TonyS32* p_high, TonyU16 p_value);
void __cdecl WaveRateFinePitchDownPair(TonyU16 p_pitch, TonyS32* p_low, TonyS32* p_high, TonyU16 p_value);
TonyU8 __fastcall SeqIsPlayable(TonyU8 p_a, TonyU8 p_b);
TonyU16 __fastcall ModEvalRouteBlock(struct SeqRecord* p_record, TonyU8* p_routes);
void __fastcall SeqSetController(TonyU8 p_ctrl, TonyU8 p_channel, TonyU8 p_category, TonyU8 p_value);
void __fastcall SeqSetProgram(TonyU8 p_channel, TonyU8 p_category, TonyU8 p_value);
void __fastcall SeqSetPlayable(TonyU8 p_channel, TonyU8 p_category, TonyU8 p_value);
TonyS32 __fastcall VoiceRetrigger(TonyU8 p_priority, TonyU8 p_sound, TonyU8 p_category);
TonyU16 __cdecl WaveNoteToStep(TonyU8 p_priority, TonyS32 p_b);
TonyS32 __fastcall VoiceMintHandle(struct SeqVoice* p_slot);
TonyS32 VoiceNextHandle(void);
struct PoolNode* __fastcall VoiceFindRecord(TonyS32 p_handle);
void __fastcall VoiceUnhook(struct SeqVoice* p_slot);
void __fastcall VoiceResetSlot(struct SeqVoice* p_slot);
TonyS32 __fastcall HandleStopInstances(TonyS32 p_handle);
void __fastcall ChanRelease(TonyS32 p_channel);
void __fastcall ChanFree(TonyS32 p_slot);
void __fastcall ChanReset(TonyS32 p_slot);
void __fastcall WaveDriverSetAdsr(TonyU8 p_channel, TonyU16* p_desc);
void __fastcall WaveDriverSetRelease(TonyU8 p_channel, TonyS32 p_b);
void __fastcall WaveDriverStart(TonyU8 p_channel, TonyU8 p_value);
void __fastcall WaveDriverStop(TonyU8 p_channel);
void __fastcall WaveDriverGateOff(TonyU8 p_channel);
void __fastcall WaveDriverRelease(TonyU8 p_channel);
void __fastcall WaveDriverSetGains(TonyU8 p_channel, TonyS32 p_a, TonyS32 p_b, TonyS32 p_c, TonyS32 p_d);
void __fastcall WaveDriverKill(TonyU8 p_channel);
void __fastcall ChannelSpliceFree(struct SampleChannel* p_channel);
void __fastcall ChannelFreeQueued(struct SampleChannel* p_channel);
void __fastcall ChannelStop(TonyS32 p_handle);
void __fastcall ChannelFadeVolume(TonyU8 p_volume, TonyU16 p_b, TonyS32 p_handle, TonyU8 p_d);
void ChannelStopAll(void);
TonyS32 __fastcall HandleToChannel(TonyS32 p_handle);
void __fastcall ChannelPause(TonyS32 p_handle);
void StreamReleaseAll(void);
void __fastcall BankRegisterResources(void* p_section, void* p_b);
void __fastcall BankRegisterSongs(void* p_section, void* p_b);
void __fastcall BankRegisterTagged(void* p_section, void* p_b);
void __fastcall BankRegisterSamples(void* p_section, void* p_b, void* p_c);
void __fastcall TriggerRegisterTable(void* p_section);
void __fastcall BankRegisterSequences(void* p_section, void* p_b);
TonyS32 __fastcall StreamEntryStart(TonyU16 p_a, TonyU16 p_b, void* p_c, struct StreamStart* p_d, TonyU8 p_e);
TonyS32 __fastcall StreamPlayEntry(TonyU16 p_a, TonyU16 p_b, void* p_c, struct StreamStart* p_d);
TonyS32 __fastcall SeqOpen(TonyS32 p_a);
TonyU8 __fastcall MixNegotiateFormat(TonyU32 p_rate, TonyU8 p_stereo, TonyU8 p_depth, TonyS32 p_flags,
	TonyU16 p_channels, TonyU8 p_f, TonyU32 p_g, TonyU32 p_h);
TonyU8 __fastcall WaveOpenChannels(void* p_channels, TonyU32 p_count, TonyU32 p_rate, TonyU8 p_mode, TonyU8 p_e,
	TonyS32 p_samples, TonyU32 p_g, TonyU32 p_h);
void __cdecl WaveOpen(void* p_channels, TonyU32 p_count, TonyU32 p_rate, TonyU8 p_mode, TonyU8 p_e,
	void* p_bus, void* p_bus2);
TonyU8 __fastcall DSndOpen(TonyU32 p_count, TonyU32 p_rate, TonyU32 p_samples, TonyU32 p_d,
	TonyU32 p_format);

// Wave channel array defined in soundwave.c (WaveChannel[0x40]).
extern undefined g_waveChannels[];

TonyS32 __fastcall SndOpenOutput(
	void* p_hWnd,
	TonyU32* p_out,
	TonyU16 p_channels,
	TonyU8 p_d,
	TonyU32 p_e,
	TonyU32 p_f
);
TonyS32 __fastcall SndOpenOutputOnDevice(TonyU32* p_out, TonyU16 p_channels, TonyU32 p_c, TonyU32 p_d);
TonyU8 DSndReleaseBuffer(void);
TonyU8 DSndTeardown(void);
void MixerLockInit(void);
void MixerLockClose(void);
void MixerLock(void);
void MixerUnlock(void);
TonyU8 __fastcall ChanIsActive(TonyS32 p_channel);
TonyU8 SndHasQueuedWork(void);
void WaveTeardownHook(void);
TonyS32 __fastcall DevVoiceSpawn(
	TonyU8 p_a,
	void* p_buffer,
	TonyS32 p_size,
	TonyU32 p_duration,
	TonyU8 p_e,
	TonyU8 p_f,
	TonyU8 p_g,
	TonyU8 p_h,
	TonyU8 (__fastcall* p_callback)(TonyS32, TonyS32, TonyS32, TonyS32, TonyS32),
	TonyS32 p_channel
);
void __fastcall DevVoiceRelease(TonyS32 p_handle);
void __fastcall ChanSetLevels(TonyS32 p_handle, TonyS32 p_a, TonyS32 p_b, TonyS32 p_c, TonyS32 p_d);
TonyU8 __fastcall WaveDriverBusy(TonyU8 p_channel);
void WaveShutdown(void);
TonyS32 __fastcall DevVoiceBufferSize(TonyS32 p_samples);
TonyU32 __fastcall SpeechSwapHeader(TonyS32 p_a);
TonyS32 SpeechLock(void);
void __fastcall SpeechUnlock(TonyS32 p_a);
TonyS32 __fastcall SpeechMintHandle(TonyS32 p_channel);
TonyS32 __fastcall SpeechHandleToChannel(TonyS32 p_handle);
void SpeechSuspend(void);
TonyU8 __fastcall SpeechProgressCallback(TonyS32 p_a, TonyS32 p_b, TonyS32 p_c, TonyS32 p_d, TonyS32 p_channel);
void SpeechResume(void);
void __fastcall SpeechRingCopy(TonyS32* p_dest, TonyS32 p_source, TonyS32 p_bytes);
void __fastcall SpeechFill(void* p_dest, TonyU8 p_value, TonyS32 p_bytes);
TonyU32 __fastcall SpeechSwapDword(TonyU32 p_value);
TonyS32 __fastcall GsmDecodeStreamFrame(struct SpeechChannel* p_voice, TonyS32* p_out);
void __fastcall GsmZeroFill(char* p_dest, TonyS32 p_count);
TonyU32 __fastcall GsmSwapDword(TonyU32 p_value);
void __fastcall SpeechDriveChannel(struct SpeechChannel* p_voice);
TonyS32 __fastcall SpeechConsumeWindow(struct SpeechChannel* p_voice, TonyS32 p_bytes);
void __fastcall SpeechResetDecoder(struct SpeechChannel* p_voice);
void __fastcall SpeechFlushDecoder(struct SpeechChannel* p_voice);
void __fastcall SpeechStopChannel(TonyS32 p_channel);
void SpeechServiceTick(void);
struct SpeechChannel;
void __fastcall SpeechPrepareChannel(struct SpeechChannel* p_channel, TonyS32 p_data, void* p_buffer, TonyS32 p_size);
void SpeechServiceUnhook(void);
void SpeechWaitIdle(void);
void SpeechFreeBuffer(void);

// Variable-length sound-bank block: size/terminator head, id/kind words, then
// section offsets relative to the block start + 8.
typedef struct SoundBank {
	TonyS32 m_size;  // 0x00
	TonyU16 m_id;  // 0x04
	TonyU16 m_kind;  // 0x06
	TonyS32 m_sequencesOfs;  // 0x08
	TonyS32 m_samplesOfs;  // 0x0c
	TonyS32 m_resourcesOfs;  // 0x10
	TonyS32 m_songsOfs;  // 0x14
	TonyS32 m_taggedOfs;  // 0x18
	TonyS32 m_triggersOfs;  // 0x1c
	TonyS32 m_kitDirOfs;  // 0x20
	TonyS32 m_streamsOfs;  // 0x24
} SoundBank;

// Stream entry record in a bank's third section (id plus playback parameters).
// SIZE 0x84
// Per-route setup row of a stream entry: program, level, pan, reverb and chorus.
// SIZE 0x8
typedef struct StreamEntry {
	TonyU8 m_program;       // 0x00
	TonyU8 m_level;       // 0x01
	TonyU8 m_pan;       // 0x02
	TonyU8 m_reverb;       // 0x03
	TonyU8 m_chorus;       // 0x04
	undefined m_pad0[3]; // 0x05
} StreamEntry;

typedef struct StreamConfig {
	TonyU16 m_id;                  // 0x00
	undefined m_pad0[2];             // 0x02
	struct StreamEntry m_routes[0x10]; // 0x04
} StreamConfig;

// The mixer output DirectSound buffer, called through its raw vtable (this TU does
// not include dsound.h; only the Release and Stop slots are used).
typedef struct IDSBuffer {
	struct IDSBufferVtbl* m_vtbl; // 0x00
} IDSBuffer;

typedef struct IDSBufferVtbl {
	undefined4 m_iunknownSlots[2];                                                                        // 0x00
	TonyU32(__stdcall* m_release)(struct IDSBuffer*);                                               // 0x08
	TonyS32(__stdcall* m_getCaps)(struct IDSBuffer*, TonyU32*);                                     // 0x0c
	TonyS32(__stdcall* m_getCurrentPosition)(struct IDSBuffer*, TonyU32*, TonyU32*);                // 0x10
	undefined4 m_getFormatSlots[4];                                                                        // 0x14
	TonyS32(__stdcall* m_getStatus)(struct IDSBuffer*, TonyU32*);                                   // 0x24
	undefined4 m_initialize;                                                                           // 0x28
	TonyS32(__stdcall* m_lock)(
		struct IDSBuffer*, TonyU32, TonyU32, void**, TonyU32*, void**, TonyU32*, TonyU32);          // 0x2c
	TonyS32(__stdcall* m_play)(struct IDSBuffer*, TonyU32, TonyU32, TonyU32);                       // 0x30
	undefined4 m_setCurrentPosition;                                                                           // 0x34
	TonyS32(__stdcall* m_setFormat)(struct IDSBuffer*, struct OutputFormat*);                        // 0x38
	TonyS32(__stdcall* m_setVolume)(struct IDSBuffer*, TonyS32);                                    // 0x3c
	undefined4 m_setPanSlots[2];                                                                        // 0x40
	TonyS32(__stdcall* m_stop)(struct IDSBuffer*);                                                  // 0x48
	TonyS32(__stdcall* m_unlock)(struct IDSBuffer*, void*, TonyU32, void*, TonyU32);                // 0x4c
	TonyS32(__stdcall* m_restore)(struct IDSBuffer*);                                               // 0x50
} IDSBufferVtbl;

// Output wave format (WAVEFORMATEX-shaped, 0x12 data bytes) negotiated by
// DSndNegotiateFormat.
typedef struct OutputFormat {
	TonyU16 m_formatTag; // 0x00
	TonyU16 m_channels; // 0x02
	TonyU32 m_samplesPerSec; // 0x04
	TonyU32 m_avgBytesPerSec; // 0x08
	TonyU16 m_blockAlign; // 0x0c
	TonyU16 m_bitsPerSample; // 0x0e
	TonyU16 m_extraSize; // 0x10
} OutputFormat;

// Device GUID as four raw dwords (this TU does not include the COM headers).
typedef struct RawGuid {
	TonyU32 m_data1; // 0x00
	TonyU32 m_data2; // 0x04
	TonyU32 m_data3; // 0x08
	TonyU32 m_data4; // 0x0c
} RawGuid;

// Enumerated DirectSound device record: GUID, NULL-GUID (primary device) flag, and
// the device description string.
typedef struct DeviceRecord {
	RawGuid m_guid;   // 0x00
	TonyU8 m_isDefault;       // 0x10
	TonyChar m_name[0x103]; // 0x11
} DeviceRecord;

// Device capabilities block (dwSize/dwFlags head; the rest is not read).
typedef struct DeviceCaps {
	TonyU32 m_size;             // 0x00
	TonyU32 m_flags;             // 0x04
	undefined m_reserved[0x60 - 8]; // 0x08
} DeviceCaps;

// Sound buffer description passed to the createSoundBuffer slot.
typedef struct BufferDesc {
	TonyU32 m_size; // 0x00
	TonyU32 m_flags; // 0x04
	TonyU32 m_bufferBytes; // 0x08
	TonyU32 m_reserved; // 0x0c
	void* m_format;   // 0x10
} BufferDesc;

// The DirectSound device object, called through its raw vtable (only the Release,
// CreateSoundBuffer, GetCaps, and SetCooperativeLevel slots are used).
typedef struct IDSDevice {
	struct IDSDeviceVtbl* m_vtbl; // 0x00
} IDSDevice;

typedef struct IDSDeviceVtbl {
	undefined4 m_iunknownSlots[2];                                                              // 0x00
	TonyU32(__stdcall* m_release)(struct IDSDevice*);                                     // 0x08
	TonyS32(__stdcall* m_createSoundBuffer)(
		struct IDSDevice*, struct BufferDesc*, struct IDSBuffer**, void*);            // 0x0c
	TonyS32(__stdcall* m_getCaps)(struct IDSDevice*, struct DeviceCaps*);              // 0x10
	undefined4 m_duplicateSoundBuffer;                                                                 // 0x14
	TonyS32(__stdcall* m_setCooperativeLevel)(struct IDSDevice*, void*, TonyU32);         // 0x18
} IDSDeviceVtbl;

// Opened-device description: negotiated output format plus the device name and
// driver banner strings.
typedef struct DeviceInfo {
	TonyU32 m_rate;          // 0x00
	TonyU8 m_stereo;           // 0x04
	TonyU8 m_depth;           // 0x05
	TonyChar m_deviceName[0x100];  // 0x06
	TonyChar m_driverName[0x100]; // 0x106
} DeviceInfo;

// Device (DirectSound) voice slot.
// SIZE 0x18
typedef struct DeviceVoice {
	TonyU8 m_active;                                                                       // 0x00
	undefined m_pad0[3];                                                                 // 0x01
	TonyU8(__fastcall* m_refill)(TonyS16*, TonyU32, TonyS16*, TonyU32, TonyS32);           // 0x04
	TonyS16* m_buffer;                                                                     // 0x08
	TonyU32 m_size;                                                                      // 0x0c
	TonyU32 m_playPos;                                                                      // 0x10
	TonyS32 m_user;                                                                      // 0x14
} DeviceVoice;

// Sound-effect instance slot.
// SIZE 0x17c

// Sample directory entry: resource id, data offset and the sample descriptor.
// SIZE 0x18
typedef struct SampleDirEntry {
	TonyU16 m_id;              // 0x00
	undefined m_pad0[2];         // 0x02
	TonyS32 m_dataOfs;              // 0x04
	undefined m_desc[0x18 - 0x08]; // 0x08
} SampleDirEntry;

// Bank chunk header: byte size of the chunk and its resource id.
// SIZE 0x8
typedef struct BankChunk {
	TonyS32 m_size;      // 0x00
	TonyU16 m_id;      // 0x04
	undefined m_pad0[2]; // 0x06
} BankChunk;

// Track fade state: level, target, step, span, pending handle and mode.
// SIZE 0x28
typedef struct TrackFade {
	TonyS32 m_level;               // 0x00
	TonyS32 m_target;               // 0x04
	TonyS32 m_step;               // 0x08
	TonyS32 m_time;               // 0x0c
	TonyS32 m_handle;               // 0x10
	TonyU8 m_mode;                 // 0x14
	TonyU8 m_command;                 // 0x15
	undefined m_pad0[2];           // 0x16
	TonyS32 m_groupLevel;                // 0x18
	TonyS32 m_groupTarget;                // 0x1c
	TonyS32 m_groupStep;                // 0x20
	TonyS32 m_groupTime;                // 0x24
} TrackFade;

// Modulation route entry: controller, target and 8.8 scale.
// SIZE 0x4
typedef struct ModRoute {
	TonyU8 m_ctrl;  // 0x00
	TonyU8 m_op;  // 0x01
	TonyS16 m_scale; // 0x02
} ModRoute;

// Modulation route block: four routes and their count.
// SIZE 0x12
typedef struct ModRouteBlock {
	ModRoute m_routes[4]; // 0x00
	TonyU8 m_count;           // 0x10
	undefined m_pad0;        // 0x11
} ModRouteBlock;

// Retrigger timer pair on a voice: phase, period and reload.
// SIZE 0xc
typedef struct RetrigTimer {
	TonyS32 m_phase;      // 0x00
	TonyS32 m_period;      // 0x04
	TonyU16 m_value;      // 0x08
	undefined m_pad0[2]; // 0x0a
} RetrigTimer;

typedef struct SeqVoice {
	struct SortedEvent* m_eventList;     // 0x00
	struct SortedEvent* m_eventCursor;     // 0x04
	struct SortedEvent* m_savedList;     // 0x08
	struct SortedEvent* m_savedCursor;     // 0x0c
	TonyS32 m_chainNext;                   // 0x10
	TonyS32 m_chainPrev;                   // 0x14
	struct PoolNode* m_record;        // 0x18
	struct SortedEvent* m_releaseList;     // 0x1c
	struct SortedEvent* m_releaseCursor;     // 0x20
	TonyS32 m_flags;                   // 0x24
	TonyU32 m_age;                   // 0x28
	TonyU16 m_ageStep;                   // 0x2c
	TonyU8 m_priority;                    // 0x2e
	TonyU8 m_volumeTrack;                    // 0x2f
	TonyU32 m_level;                   // 0x30
	TonyU32 m_levelBase;                   // 0x34
	TonyU32 m_pan;                   // 0x38
	TonyS32 m_panStep;                   // 0x3c
	TonyU32 m_gateTime;                   // 0x40
	TonyS32 m_tremoloPhase;                   // 0x44
	TonyS32 m_autoPanPhase;                   // 0x48
	TonyU8 m_tremoloReload;                    // 0x4c
	TonyU8 m_autoPanReload;                    // 0x4d
	TonyU8 m_sound;                    // 0x4e
	TonyU8 m_category;                    // 0x4f
	TonyU8 m_class;                    // 0x50
	undefined m_pad0;                 // 0x51
	TonyU16 m_baseNote;                   // 0x52
	TonyU16 m_note;                   // 0x54
	TonyU8 m_startLevel;                    // 0x56
	TonyU8 m_startPan;                    // 0x57
	TonyU8 m_startSound;                    // 0x58
	TonyU8 m_startCategory;                    // 0x59
	TonyU8 m_startVolumeTrack;                    // 0x5a
	undefined m_pad1;                 // 0x5b
	TonyS32 m_sampleData;                   // 0x5c
	TonyS32 m_sampleRate;                   // 0x60
	TonyS32 m_handle;                   // 0x64
	TonyU16 m_listId;                   // 0x68
	undefined m_pad2[2];              // 0x6a
	TonyU16 m_loopCount;                   // 0x6c
	TonyU8 m_bendRangeDown;                    // 0x6e
	TonyU8 m_bendRangeUp;                    // 0x6f
	TonyU32 m_pitch;                   // 0x70
	TonyU32 m_vibPhase;                   // 0x74
	TonyU32 m_vibPeriod;                   // 0x78
	TonyS32 m_vibValue;                   // 0x7c
	TonyU8 m_vibDepth;                    // 0x80
	TonyU8 m_vibSkew;                    // 0x81
	undefined m_pad3[2];              // 0x82
	TonyS32 m_tremoloStep;                   // 0x84
	TonyS32 m_autoPanStep;                   // 0x88
	TonyS32 m_levelFadeStep;                   // 0x8c
	TonyU32 m_levelFadeTarget;                   // 0x90
	TonyU32 m_glidePos;                   // 0x94
	TonyU32 m_glideSpan;                   // 0x98
	TonyU32 m_prevPitch;                   // 0x9c
	TonyU8 m_muteMode;                    // 0xa0
	undefined m_pad4[3];              // 0xa1
	TonyU32 m_panSweepTime;                   // 0xa4
	TonyU32 m_panSweepTarget;                   // 0xa8
	TonyU8 m_tremoloCount;                    // 0xac
	TonyU8 m_autoPanCount;                    // 0xad
	undefined m_pad5[2];              // 0xae
	TonyU32 m_levelFadeTime;                   // 0xb0
	TonyU16 m_pitchMod;                   // 0xb4
	TonyU16 m_groupKey;                   // 0xb6
	undefined m_pad6;                 // 0xb8
	TonyU8 m_reserved;                    // 0xb9
	TonyU8 m_track;                    // 0xba
	TonyU8 m_startTrack;                    // 0xbb
	TonyS8 m_noteSlew;                    // 0xbc
	TonyU8 m_prevNote;                    // 0xbd
	TonyU16 m_levelWord;                   // 0xbe
	struct ModRouteBlock m_modBlocks[9];    // 0xc0
	undefined m_pad7[2];             // 0x162
	struct RetrigTimer m_timers[2];  // 0x164
} SeqVoice;

// Song note record: due time, controller bytes, note id and loop target.
// SIZE 0xc
typedef struct NoteEvent {
	TonyU32 m_time;      // 0x00
	TonyU8 m_program;       // 0x04
	TonyU8 m_level;       // 0x05
	undefined m_pad0[2]; // 0x06
	TonyU16 m_note;      // 0x08
	TonyU8 m_noteOffset;       // 0x0a
	TonyU8 m_velocityOffset;       // 0x0b
} NoteEvent;

// Song track state: note stream base, cursor, clock fraction and next-due time.
// SIZE 0x10
typedef struct TrackCursor {
	struct NoteEvent* m_stream; // 0x00
	struct NoteEvent* m_cursor; // 0x04
	TonyU32 m_frac;              // 0x08
	TonyU32 m_pos;              // 0x0c
} TrackCursor;

// Note descriptor in a song: pitch-bend/modulation stream offsets, sample events
// follow.
// SIZE 0xc
typedef struct NoteDesc {
	TonyS32 m_size; // 0x00
	TonyS32 m_bendOfs; // 0x04
	TonyS32 m_wheelOfs; // 0x08
} NoteDesc;

// Sample directory record: resource id and two route bytes.
// SIZE 0x8
typedef struct SampleRoute {
	TonyU16 m_listId;      // 0x00
	TonyU8 m_priority;       // 0x02
	TonyU8 m_polyphony;       // 0x03
	TonyU8 m_route;       // 0x04
	TonyU8 m_program;       // 0x05
	undefined m_pad0[2]; // 0x06
} SampleRoute;

// Queued play command: handles, level/pan/pitch targets and the field-select flags.
// SIZE 0x28
typedef struct PlayCmd {
	TonyS32 m_fadeHandle;      // 0x00
	TonyU16 m_fadeSpan;      // 0x04
	undefined m_pad0[2]; // 0x06
	TonyS32 m_resumeHandle;      // 0x08
	TonyU16 m_resumeSpan;      // 0x0c
	undefined m_pad1[2]; // 0x0e
	TonyS32 m_songData;      // 0x10
	TonyU16 m_bankId;      // 0x14
	TonyU16 m_entryId;      // 0x16
	TonyU8 m_volume;       // 0x18
	undefined m_pad2[3]; // 0x19
	TonyS32 m_trackMaskLo;      // 0x1c
	TonyS32 m_trackMaskHi;      // 0x20
	TonyU16 m_rateScale;      // 0x24
	TonyU8 m_flags;       // 0x26
	undefined m_pad3;    // 0x27
} PlayCmd;

// Stream start parameters handed to the wave starter.
// SIZE 0x1c
typedef struct StreamStart {
	TonyS32 m_flags;      // 0x00
	TonyS32 m_trackMaskLo;      // 0x04
	TonyS32 m_trackMaskHi;      // 0x08
	TonyU16 m_rateScale;      // 0x0c
	TonyU16 m_fadeSpan;      // 0x0e
	TonyU8 m_fadeLevel;       // 0x10
	undefined m_pad0;    // 0x11
	TonyU8 m_remapCount;       // 0x12
	undefined m_pad1;    // 0x13
	TonyU8* m_remapPairs;      // 0x14
	TonyU8 m_fadeTrackCount;       // 0x18
	undefined m_pad2[3]; // 0x19
	TonyU8* m_fadeTracks;      // 0x1c
} StreamStart;

// Fraction/position clock pair shared by voice cursors and pending nodes.
// SIZE 0x8
typedef struct ClockPair {
	TonyS32 m_frac; // 0x00
	TonyS32 m_pos; // 0x04
} ClockPair;

// Sample event record in a note stream: due time, duration and two event bytes.
// SIZE 0x8
typedef struct SampleEvent {
	TonyU32 m_time; // 0x00
	TonyU16 m_duration; // 0x04
	TonyU8 m_note;  // 0x06
	TonyU8 m_velocity;  // 0x07
} SampleEvent;

// Per-voice resample cursor of a sample channel.
// SIZE 0x24
typedef struct VoiceCursor {
	struct ClockPair m_clock; // 0x00
	struct SampleEvent* m_event; // 0x08
	TonyU8* m_bendStream;             // 0x0c
	TonyU8* m_wheelStream;      // 0x10
	TonyU16 m_bendValue;      // 0x14
	TonyU16 m_wheelValue;      // 0x16
	TonyU32 m_bendTime;      // 0x18
	TonyU32 m_wheelTime;      // 0x1c
	TonyU8 m_route;       // 0x20
	TonyU8 m_velocityOffset;       // 0x21
	TonyU8 m_noteOffset;       // 0x22
	TonyU8 m_track;       // 0x23
} VoiceCursor;

// Sample (wave) channel state, eight slots.
// SIZE 0xef8
typedef struct SampleChannel {
	TonyS32 m_handle;                      // 0x00
	struct SampleRoute* m_instDir;          // 0x04
	TonyU8 m_instMap[0x80];                 // 0x08
	struct SampleRoute* m_kitDir;          // 0x88
	TonyU8 m_kitMap[0x80];                 // 0x8c
	struct SongBankHeader* m_song;          // 0x10c
	TonyU32 m_trackMask[2];                  // 0x110
	TonyS32 m_stepFrac;                     // 0x118
	TonyS32 m_stepWhole;                     // 0x11c
	TonyS32 m_stepLead;                     // 0x120
	TonyU32 m_tickRate;                     // 0x124
	struct TrackCursor m_tracks[0x40]; // 0x128
	TonyU8 m_voiceTracks[0x40];                // 0x528
	struct VoiceCursor m_cursors[0x40];  // 0x568
	TonyU32* m_tempoStream;                    // 0xe68
	TonyU32* m_tempoCursor;                    // 0xe6c
	TonyU32 m_clockFrac;                     // 0xe70
	TonyU32 m_clockPos;                     // 0xe74
	struct PendingCmd* m_noteOffChain;     // 0xe78
	struct PendingCmd* m_retireChain;     // 0xe7c
	TonyS32 m_routePrograms[0x10];               // 0xe80
	TonyU8 m_running;                      // 0xec0
	TonyU8 m_free;                      // 0xec1
	TonyU16 m_rateScale;                     // 0xec2
	TonyU8 m_fadeTrack;                      // 0xec4
	TonyU8 m_noLoop;                      // 0xec5
	TonyU16 m_loopCount;                     // 0xec6
	undefined m_deferredCmd[0xee0 - 0xec8];    // 0xec8
	TonyU8 m_streamVolume;                      // 0xee0
	undefined m_pad0[3];                // 0xee1
	TonyS32 m_streamMaskLo;                     // 0xee4
	TonyS32 m_streamMaskHi;                     // 0xee8
	TonyU16 m_streamRate;                     // 0xeec
	TonyU8 m_streamFlags;                      // 0xeee
	undefined m_pad1;                   // 0xeef
	TonyS32 m_deferredOut;                     // 0xef0
	TonyU8 m_deferredArmed;                      // 0xef4
	undefined m_pad2[3];                // 0xef5
} SampleChannel;

// Sorted sequencer event entry: value and time key.
// SIZE 0x8
typedef struct SortedEvent {
	TonyS32 m_value; // 0x00
	TonyU16 m_key; // 0x04
} SortedEvent;

// Sequencer record; the tempo-slot byte and the modulation-route defaults are mapped.
typedef struct SeqRecord {
	undefined m_gap0[0x4e];           // 0x00
	TonyU8 m_sound;                    // 0x4e
	TonyU8 m_category;                    // 0x4f
	undefined m_gap1[0xc0 - 0x50];    // 0x50
	TonyU8 m_levelRouteCtrl;                    // 0xc0
	TonyU8 m_levelRouteOp;                    // 0xc1
	TonyU16 m_levelRouteScale;                   // 0xc2
	undefined m_gap2[0xd0 - 0xc4];    // 0xc4
	TonyU8 m_levelRouteCount;                    // 0xd0
	undefined m_gap3;                 // 0xd1
	TonyU8 m_volumeRouteCtrl;                    // 0xd2
	TonyU8 m_volumeRouteOp;                    // 0xd3
	TonyU16 m_volumeRouteScale;                   // 0xd4
	undefined m_gap4[0xe2 - 0xd6];    // 0xd6
	TonyU8 m_volumeRouteCount;                    // 0xe2
	undefined m_gap5;                 // 0xe3
	TonyU8 m_panRouteCtrl;                    // 0xe4
	TonyU8 m_panRouteOp;                    // 0xe5
	TonyU16 m_panRouteScale;                   // 0xe6
	undefined m_gap6[0xf4 - 0xe8];    // 0xe8
	TonyU8 m_panRouteCount;                    // 0xf4
	undefined m_gap7;                 // 0xf5
	TonyU8 m_rateRouteCtrl;                    // 0xf6
	TonyU8 m_rateRouteOp;                    // 0xf7
	TonyU16 m_rateRouteScale;                   // 0xf8
	undefined m_gap8[0x106 - 0xfa];   // 0xfa
	TonyU8 m_rateRouteCount;                   // 0x106
	undefined m_gap9;                // 0x107
	TonyU8 m_effectsRouteCtrl;                   // 0x108
	TonyU8 m_effectsRouteOp;                   // 0x109
	TonyU16 m_effectsRouteScale;                  // 0x10a
	undefined m_gap10[0x118 - 0x10c]; // 0x10c
	TonyU8 m_effectsRouteCount;                   // 0x118
	undefined m_gap11;                // 0x119
	TonyU8 m_vibDepthRouteCtrl;                   // 0x11a
	TonyU8 m_vibDepthRouteOp;                   // 0x11b
	TonyU16 m_vibDepthRouteScale;                  // 0x11c
	undefined m_gap12[0x12a - 0x11e]; // 0x11e
	TonyU8 m_vibDepthRouteCount;                   // 0x12a
	undefined m_gap13;                // 0x12b
	TonyU8 m_vibRateRouteCtrl;                   // 0x12c
	TonyU8 m_vibRateRouteOp;                   // 0x12d
	TonyU16 m_vibRateRouteScale;                  // 0x12e
	undefined m_gap14[0x13c - 0x130]; // 0x130
	TonyU8 m_vibRateRouteCount;                   // 0x13c
	undefined m_gap15;                // 0x13d
	TonyU8 m_wheelRouteCtrl;                   // 0x13e
	TonyU8 m_wheelRouteOp;                   // 0x13f
	TonyU16 m_wheelRouteScale;                  // 0x140
	undefined m_gap16[0x14e - 0x142]; // 0x142
	TonyU8 m_wheelRouteCount;                   // 0x14e
	undefined m_gap17;                // 0x14f
	TonyU8 m_bendRouteCtrl;                   // 0x150
	TonyU8 m_bendRouteOp;                   // 0x151
	TonyU16 m_bendRouteScale;                  // 0x152
	undefined m_gap18[0x160 - 0x154]; // 0x154
	TonyU8 m_bendRouteCount;                   // 0x160
	undefined m_gap19[0x16c - 0x161]; // 0x161
	TonyS16 m_timerValueA;                  // 0x16c
	undefined m_gap20[0x178 - 0x16e]; // 0x16e
	TonyS16 m_timerValueB;                  // 0x178
} SeqRecord;

// Doubly-linked allocation node in the mixer's event pool.
// SIZE 0x10
typedef struct PoolNode {
	struct PoolNode* m_next; // 0x00
	struct PoolNode* m_prev; // 0x04
	TonyS32 m_handle;            // 0x08
	TonyS32 m_owner;            // 0x0c
} PoolNode;

// Pending-command node; the link head overlays PoolNode, followed by the payload.
// SIZE 0x18
typedef struct PendingCmd {
	struct PendingCmd* m_next; // 0x00
	struct PendingCmd* m_prev; // 0x04
	TonyS32 m_handle;                 // 0x08
	TonyS32 m_offTime;                 // 0x0c
	struct ClockPair m_clock;      // 0x10
} PendingCmd;

// GSM voice channel state; only the busy flag is mapped so far.
// SIZE 0x41e8
typedef struct SpeechChannel {
	TonyS16 m_history[0xa0];                // 0x00
	TonyS32 m_filterTaps[9];                  // 0x140
	TonyS16 m_deemph;                     // 0x164
	TonyU16 m_lastLag;                     // 0x166
	TonyS16 m_larBanks[2][8];               // 0x168
	TonyS16 m_larBankIndex;                     // 0x188
	TonyS16 m_voicedRun;                     // 0x18a
	TonyS16 m_silenceRun;                     // 0x18c
	undefined m_pad0[2];                // 0x18e
	TonyS32 m_source;                     // 0x190
	void* m_window;                       // 0x194
	TonyS32 m_windowSize;                     // 0x198
	undefined m_stageRing[0x419c - 0x19c];   // 0x19c
	TonyU16 m_frameCount;                    // 0x419c
	TonyU16 m_headerLow;                    // 0x419e
	TonyU32 m_sourceLeft;                    // 0x41a0
	TonyS32 m_drainLeft;                    // 0x41a4
	TonyS32 m_bitCursor;                    // 0x41a8
	TonyS32 m_stageReadPos;                    // 0x41ac
	TonyU32 m_stageWritePos;                    // 0x41b0
	TonyU32 m_windowReadPos;                    // 0x41b4
	TonyU32 m_windowWritePos;                    // 0x41b8
	TonyU8 m_stageTick;                     // 0x41bc
	TonyS8 m_decodeState;                     // 0x41bd
	TonyS8 m_decodeBudget;                     // 0x41be
	TonyU8 m_stagePending;                     // 0x41bf
	TonyU8 m_state;                     // 0x41c0
	TonyU8 m_level;                     // 0x41c1
	TonyU8 m_balance;                     // 0x41c2
	TonyU8 m_pan;                     // 0x41c3
	TonyU8 m_priority;                     // 0x41c4
	undefined m_pad1[0x41c8 - 0x41c5]; // 0x41c5
	TonyU32 m_rate;                    // 0x41c8
	TonyS32 m_devVoice;                    // 0x41cc
	void* m_mixBuffer;                      // 0x41d0
	TonyS32 m_mixSize;                    // 0x41d4
	TonyS32 m_consumed;                    // 0x41d8
	TonyFloat m_duration;                  // 0x41dc
	TonyU8 m_stopTicks;                     // 0x41e0
	TonyU8 m_boost;                     // 0x41e1
	undefined m_pad2[0x41e4 - 0x41e2]; // 0x41e2
	TonyS32 m_handle;                    // 0x41e4
} SpeechChannel;

// Query-in-progress flag raised by SndIsIdle around the busy sweep.
// GLOBAL: TONY2 0x004e622c
TonyU8 g_sndIdleQueryBusy;

// Multiplicative PRNG seed.
// GLOBAL: TONY2 0x00455958
TonyU32 g_sndRandomSeed = 1;

// Quarter-wave Q12 sine ROM.
// GLOBAL: TONY2 0x0045595c
TonyS16 g_sndSineRom[0x400] = {
	0, 6, 12, 18, 25, 31, 37, 43, 50, 56, 62, 69,
	75, 81, 87, 94, 100, 106, 113, 119, 125, 131, 138, 144,
	150, 157, 163, 169, 175, 182, 188, 194, 200, 207, 213, 219,
	226, 232, 238, 244, 251, 257, 263, 269, 276, 282, 288, 295,
	301, 307, 313, 320, 326, 332, 338, 345, 351, 357, 363, 370,
	376, 382, 388, 395, 401, 407, 413, 420, 426, 432, 438, 445,
	451, 457, 463, 470, 476, 482, 488, 495, 501, 507, 513, 520,
	526, 532, 538, 545, 551, 557, 563, 569, 576, 582, 588, 594,
	601, 607, 613, 619, 625, 632, 638, 644, 650, 656, 663, 669,
	675, 681, 687, 694, 700, 706, 712, 718, 725, 731, 737, 743,
	749, 755, 762, 768, 774, 780, 786, 792, 799, 805, 811, 817,
	823, 829, 836, 842, 848, 854, 860, 866, 872, 879, 885, 891,
	897, 903, 909, 915, 921, 928, 934, 940, 946, 952, 958, 964,
	970, 976, 983, 989, 995, 1001, 1007, 1013, 1019, 1025, 1031, 1037,
	1043, 1050, 1056, 1062, 1068, 1074, 1080, 1086, 1092, 1098, 1104, 1110,
	1116, 1122, 1128, 1134, 1140, 1146, 1152, 1158, 1164, 1170, 1176, 1182,
	1189, 1195, 1201, 1207, 1213, 1219, 1225, 1231, 1237, 1243, 1248, 1254,
	1260, 1266, 1272, 1278, 1284, 1290, 1296, 1302, 1308, 1314, 1320, 1326,
	1332, 1338, 1344, 1350, 1356, 1362, 1368, 1373, 1379, 1385, 1391, 1397,
	1403, 1409, 1415, 1421, 1427, 1433, 1438, 1444, 1450, 1456, 1462, 1468,
	1474, 1479, 1485, 1491, 1497, 1503, 1509, 1515, 1520, 1526, 1532, 1538,
	1544, 1550, 1555, 1561, 1567, 1573, 1579, 1584, 1590, 1596, 1602, 1608,
	1613, 1619, 1625, 1631, 1636, 1642, 1648, 1654, 1659, 1665, 1671, 1677,
	1682, 1688, 1694, 1699, 1705, 1711, 1717, 1722, 1728, 1734, 1739, 1745,
	1751, 1756, 1762, 1768, 1773, 1779, 1785, 1790, 1796, 1802, 1807, 1813,
	1819, 1824, 1830, 1835, 1841, 1847, 1852, 1858, 1864, 1869, 1875, 1880,
	1886, 1891, 1897, 1903, 1908, 1914, 1919, 1925, 1930, 1936, 1941, 1947,
	1952, 1958, 1964, 1969, 1975, 1980, 1986, 1991, 1997, 2002, 2007, 2013,
	2018, 2024, 2029, 2035, 2040, 2046, 2051, 2057, 2062, 2067, 2073, 2078,
	2084, 2089, 2094, 2100, 2105, 2111, 2116, 2121, 2127, 2132, 2138, 2143,
	2148, 2154, 2159, 2164, 2170, 2175, 2180, 2186, 2191, 2196, 2201, 2207,
	2212, 2217, 2223, 2228, 2233, 2238, 2244, 2249, 2254, 2259, 2265, 2270,
	2275, 2280, 2286, 2291, 2296, 2301, 2306, 2312, 2317, 2322, 2327, 2332,
	2337, 2343, 2348, 2353, 2358, 2363, 2368, 2373, 2379, 2384, 2389, 2394,
	2399, 2404, 2409, 2414, 2419, 2424, 2429, 2434, 2439, 2445, 2450, 2455,
	2460, 2465, 2470, 2475, 2480, 2485, 2490, 2495, 2500, 2505, 2510, 2515,
	2519, 2524, 2529, 2534, 2539, 2544, 2549, 2554, 2559, 2564, 2569, 2574,
	2578, 2583, 2588, 2593, 2598, 2603, 2608, 2613, 2617, 2622, 2627, 2632,
	2637, 2641, 2646, 2651, 2656, 2661, 2665, 2670, 2675, 2680, 2684, 2689,
	2694, 2699, 2703, 2708, 2713, 2717, 2722, 2727, 2732, 2736, 2741, 2746,
	2750, 2755, 2760, 2764, 2769, 2773, 2778, 2783, 2787, 2792, 2796, 2801,
	2806, 2810, 2815, 2819, 2824, 2828, 2833, 2837, 2842, 2847, 2851, 2856,
	2860, 2865, 2869, 2874, 2878, 2882, 2887, 2891, 2896, 2900, 2905, 2909,
	2914, 2918, 2922, 2927, 2931, 2936, 2940, 2944, 2949, 2953, 2957, 2962,
	2966, 2970, 2975, 2979, 2983, 2988, 2992, 2996, 3000, 3005, 3009, 3013,
	3018, 3022, 3026, 3030, 3034, 3039, 3043, 3047, 3051, 3055, 3060, 3064,
	3068, 3072, 3076, 3080, 3085, 3089, 3093, 3097, 3101, 3105, 3109, 3113,
	3117, 3121, 3126, 3130, 3134, 3138, 3142, 3146, 3150, 3154, 3158, 3162,
	3166, 3170, 3174, 3178, 3182, 3186, 3190, 3193, 3197, 3201, 3205, 3209,
	3213, 3217, 3221, 3225, 3229, 3232, 3236, 3240, 3244, 3248, 3252, 3255,
	3259, 3263, 3267, 3271, 3274, 3278, 3282, 3286, 3289, 3293, 3297, 3301,
	3304, 3308, 3312, 3315, 3319, 3323, 3326, 3330, 3334, 3337, 3341, 3345,
	3348, 3352, 3356, 3359, 3363, 3366, 3370, 3373, 3377, 3381, 3384, 3388,
	3391, 3395, 3398, 3402, 3405, 3409, 3412, 3416, 3419, 3423, 3426, 3429,
	3433, 3436, 3440, 3443, 3447, 3450, 3453, 3457, 3460, 3463, 3467, 3470,
	3473, 3477, 3480, 3483, 3487, 3490, 3493, 3497, 3500, 3503, 3506, 3510,
	3513, 3516, 3519, 3522, 3526, 3529, 3532, 3535, 3538, 3541, 3545, 3548,
	3551, 3554, 3557, 3560, 3563, 3566, 3570, 3573, 3576, 3579, 3582, 3585,
	3588, 3591, 3594, 3597, 3600, 3603, 3606, 3609, 3612, 3615, 3618, 3621,
	3624, 3627, 3629, 3632, 3635, 3638, 3641, 3644, 3647, 3650, 3652, 3655,
	3658, 3661, 3664, 3667, 3669, 3672, 3675, 3678, 3680, 3683, 3686, 3689,
	3691, 3694, 3697, 3700, 3702, 3705, 3708, 3710, 3713, 3716, 3718, 3721,
	3723, 3726, 3729, 3731, 3734, 3736, 3739, 3742, 3744, 3747, 3749, 3752,
	3754, 3757, 3759, 3762, 3764, 3767, 3769, 3772, 3774, 3776, 3779, 3781,
	3784, 3786, 3789, 3791, 3793, 3796, 3798, 3800, 3803, 3805, 3807, 3810,
	3812, 3814, 3816, 3819, 3821, 3823, 3826, 3828, 3830, 3832, 3834, 3837,
	3839, 3841, 3843, 3845, 3848, 3850, 3852, 3854, 3856, 3858, 3860, 3862,
	3864, 3867, 3869, 3871, 3873, 3875, 3877, 3879, 3881, 3883, 3885, 3887,
	3889, 3891, 3893, 3895, 3897, 3899, 3900, 3902, 3904, 3906, 3908, 3910,
	3912, 3914, 3915, 3917, 3919, 3921, 3923, 3925, 3926, 3928, 3930, 3932,
	3933, 3935, 3937, 3939, 3940, 3942, 3944, 3945, 3947, 3949, 3950, 3952,
	3954, 3955, 3957, 3959, 3960, 3962, 3963, 3965, 3967, 3968, 3970, 3971,
	3973, 3974, 3976, 3977, 3979, 3980, 3982, 3983, 3985, 3986, 3988, 3989,
	3990, 3992, 3993, 3995, 3996, 3997, 3999, 4000, 4001, 4003, 4004, 4005,
	4007, 4008, 4009, 4011, 4012, 4013, 4014, 4016, 4017, 4018, 4019, 4020,
	4022, 4023, 4024, 4025, 4026, 4027, 4029, 4030, 4031, 4032, 4033, 4034,
	4035, 4036, 4037, 4038, 4039, 4040, 4041, 4042, 4043, 4044, 4045, 4046,
	4047, 4048, 4049, 4050, 4051, 4052, 4053, 4054, 4055, 4056, 4057, 4057,
	4058, 4059, 4060, 4061, 4062, 4062, 4063, 4064, 4065, 4065, 4066, 4067,
	4068, 4068, 4069, 4070, 4071, 4071, 4072, 4073, 4073, 4074, 4075, 4075,
	4076, 4076, 4077, 4078, 4078, 4079, 4079, 4080, 4080, 4081, 4081, 4082,
	4082, 4083, 4083, 4084, 4084, 4085, 4085, 4086, 4086, 4087, 4087, 4087,
	4088, 4088, 4089, 4089, 4089, 4090, 4090, 4090, 4091, 4091, 4091, 4091,
	4092, 4092, 4092, 4092, 4093, 4093, 4093, 4093, 4094, 4094, 4094, 4094,
	4094, 4094, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4095,
	4095, 4095, 4095, 4095
};

// Tempo-scaled tick rates per slot (8 = default).
// GLOBAL: TONY2 0x004e61f0
TonyU32 g_seqSlotTickRates[9];

// Controller rows for the default bank and the per-category cube.
// GLOBAL: TONY2 0x004fcc00
TonyU8 g_seqControllers[0x10][0x86];

// GLOBAL: TONY2 0x00508380
TonyU8 g_seqControllersByCategory[8][0x10][0x86];

// Program bytes for the per-category banks and the default bank.
// GLOBAL: TONY2 0x004fed80
TonyU8 g_seqPrograms[0x8c][0x10];

// GLOBAL: TONY2 0x0050c680
TonyU8 g_seqProgramsDefault[0x10];

// Playable flags for the default bank and the per-category banks.
// GLOBAL: TONY2 0x004ff640
TonyU8 g_seqPlayableFlags[0x40];

// Second sorted event entry: value, time key and tag.
// SIZE 0x8
typedef struct TaggedEvent {
	TonyS32 m_value; // 0x00
	TonyU16 m_key; // 0x04
	TonyU16 m_tag; // 0x06
} TaggedEvent;

// Note-pool bucket: entry count and pool start index.
// SIZE 0x4
typedef struct NoteBucket {
	TonyU16 m_count; // 0x00
	TonyU16 m_start; // 0x02
} NoteBucket;

// Note pool cursor, entries and the 64-key buckets.
// GLOBAL: TONY2 0x00506340
TonyS32 g_notePoolCursor;

// Guard entry below the bucketed event table (same folded-base pattern as
// g_regGuard).
// GLOBAL: TONY2 0x00506358
SortedEvent g_notePoolGuard = {0};

// GLOBAL: TONY2 0x00506360
SortedEvent g_notePool[0x400] = {0};

// GLOBAL: TONY2 0x0050c8e0
NoteBucket g_noteBuckets[0x200];

// Trigger entry: key, rate word and the note/velocity byte set.
// SIZE 0xc
typedef struct TriggerEntry {
	TonyU16 m_id;      // 0x00
	TonyU16 m_listId;      // 0x02
	TonyU8 m_polyphony;       // 0x04
	TonyU8 m_priority;       // 0x05
	TonyU8 m_level;       // 0x06
	TonyU8 m_pan;       // 0x07
	TonyU8 m_note;       // 0x08
	TonyU8 m_track;       // 0x09
	undefined m_pad0[2]; // 0x0a
} TriggerEntry;

// Trigger table blob: entry count followed by the trigger records.
// SIZE 0x4
typedef struct TriggerTable {
	TonyU16 m_count;             // 0x00
	undefined m_pad0[2];        // 0x02
	struct TriggerEntry m_entries[1]; // 0x04
} TriggerTable;

// Drum kit cell: target key, note offset, pan and fine tune for one kit note.
// SIZE 0x8
typedef struct DrumKitNote {
	TonyU16 m_listId;      // 0x00
	TonyS8 m_noteOffset;       // 0x02
	TonyU8 m_pan;       // 0x03
	TonyS16 m_priorityOffset;      // 0x04
	undefined m_pad0[2]; // 0x06
} DrumKitNote;

// Layer region: key, velocity window, note offset, velocity scale, fine tune and pan
// for one region of a layered note.
// SIZE 0xc
typedef struct LayerRegion {
	TonyU16 m_listId;      // 0x00
	TonyU8 m_velMin;       // 0x02
	TonyU8 m_velMax;       // 0x03
	TonyS8 m_noteOffset;       // 0x04
	TonyU8 m_velScale;       // 0x05
	TonyS16 m_priorityOffset;      // 0x06
	TonyU8 m_pan;       // 0x08
	undefined m_pad0[3]; // 0x09
} LayerRegion;

// Trigger table and its count.
// GLOBAL: TONY2 0x004fab84
TonyS32 g_triggerCount;

// GLOBAL: TONY2 0x004faba0
TriggerEntry g_triggerTable[0x100];

// Wide sorted event entry: two values and the time key.
// SIZE 0xc
typedef struct WideEvent {
	TonyS32 m_value; // 0x00
	TonyS32 m_data; // 0x04
	TonyU16 m_key; // 0x08
} WideEvent;

// Fourth (wide) sorted event table and its count.
// GLOBAL: TONY2 0x004fb7a0
TonyS32 g_songRegCount;

// GLOBAL: TONY2 0x004fbfc0
WideEvent g_songRegTable[0x100];

// Third sorted event table and its count.
// GLOBAL: TONY2 0x004fee00
TonyS32 g_regAltCount;

// Guard entry below the retrigger table (same folded-base pattern as
// g_regGuard).
// GLOBAL: TONY2 0x004fee18
SortedEvent g_regAltGuard = {0};

// GLOBAL: TONY2 0x004fee20
SortedEvent g_regAltTable[0x100] = {0};

// Second sorted event table and its count.
// GLOBAL: TONY2 0x004fb7a4
TonyS32 g_regTaggedCount;

// Guard entry below the pending-command table (same folded-base pattern as
// g_regGuard).
// GLOBAL: TONY2 0x004fb7b8
TaggedEvent g_regTaggedGuard = {0};

// GLOBAL: TONY2 0x004fb7c0
TaggedEvent g_regTaggedTable[0x100] = {0};

// Sorted event table and its count.
// GLOBAL: TONY2 0x004ff700
TonyS32 g_regCount;

// Guard entry below the sorted event table: the removal shift writes entry j - 1
// through a folded table base. Both are zero-initialized so they land adjacently
// in .data (uninitialized globals become linker-ordered COMMON symbols).
// GLOBAL: TONY2 0x004ff718
SortedEvent g_regGuard = {0};

// GLOBAL: TONY2 0x004ff720
SortedEvent g_regTable[0x100] = {0};

// GLOBAL: TONY2 0x004ff680
TonyU8 g_seqProgramsAlt[0x8c][0x10];

// Registered sound-bank block pointers, indexed by g_bankCount.
// GLOBAL: TONY2 0x004f1b40
struct SoundBank* g_bankBlocks[16];

// GLOBAL: TONY2 0x004f1b80
TonyS16 g_bankCount;

// The device voice slots.
// GLOBAL: TONY2 0x004ef720
DeviceVoice g_devVoices[0x40];

// Song-bank header: section offsets (track table, per-note stream table, track route
// map, tempo stream) plus the tick rate and the loop-restart clock. Wave descriptors
// in the wide event table overlay the first four slots as rate / packed end|flags /
// loop start / loop length (see SongLookup).
typedef struct SongBankHeader {
	TonyS32 m_trackTableOfs;      // 0x00
	TonyU32 m_noteTableOfs;      // 0x04
	TonyS32 m_routeMapOfs;      // 0x08
	TonyS32 m_tempoStreamOfs;      // 0x0c
	TonyU32 m_tickRate;      // 0x10
	TonyU32 m_loopClock;      // 0x14
} SongBankHeader;

// Unpacked song descriptor filled by the wide-table lookup.
typedef struct SongDesc {
	TonyS32 m_rate; // 0x00
	TonyS32 m_data; // 0x04
	TonyS32 m_startPos; // 0x08
	TonyS32 m_endPos; // 0x0c
	TonyS32 m_loopStart; // 0x10
	TonyS32 m_loopLength; // 0x14
	TonyU8 m_flags;  // 0x18
} SongDesc;

// Per-table lookup scratch: result pointers and key templates.
// GLOBAL: TONY2 0x004e5db8
TaggedEvent* g_regTaggedResult;

// GLOBAL: TONY2 0x004e5dc0
SortedEvent g_regKeyTemplate;

// GLOBAL: TONY2 0x004e5dc8
TaggedEvent g_regTaggedKeyTemplate;

// GLOBAL: TONY2 0x004e6218
SortedEvent* g_regAltResult;

// GLOBAL: TONY2 0x004e6220
TriggerEntry g_triggerKeyTemplate;

// GLOBAL: TONY2 0x004e6230
SortedEvent* g_regResult;

// GLOBAL: TONY2 0x004e6238
SortedEvent g_regAltKeyTemplate;

// Wide-table lookup scratch: key template, header pointer and result entry.
// GLOBAL: TONY2 0x004e61d8
WideEvent g_songKeyTemplate;

// GLOBAL: TONY2 0x004e6240
SongBankHeader* g_songHeaderResult;

// GLOBAL: TONY2 0x004e6244
WideEvent* g_songEntryResult;

// Note-pool lookup scratch: result pointer, key template and bucket start.
// GLOBAL: TONY2 0x004e5d78
SortedEvent* g_notePoolResult;

// GLOBAL: TONY2 0x004e5d80
SortedEvent g_notePoolKeyTemplate;

// GLOBAL: TONY2 0x004e5db0
TonyS32 g_notePoolBucketStart;

// Running voice-handle counter (skips -1).
// GLOBAL: TONY2 0x004e5db4
TonyS32 g_voiceHandleCounter;

// Bucket index scratch for the note-pool lookup.
// GLOBAL: TONY2 0x004e6214
TonyS32 g_notePoolBucketIndex;

// Head of the sorted instance-record list searched by VoiceFindRecord.
// GLOBAL: TONY2 0x004e5d88
PoolNode* g_voiceRecordList;

// Head of the free instance-record list.
// Song descriptor scratch for the song-start event.
// GLOBAL: TONY2 0x004e5d90
SongDesc g_songDescScratch;

// GLOBAL: TONY2 0x004e5dac
PoolNode* g_voiceRecordFree;

// The mixer event pool backing store.
// GLOBAL: TONY2 0x004e5dd8
PoolNode g_voicePool[0x40];

// The pending-command pool backing store.
// GLOBAL: TONY2 0x004f9380
PendingCmd g_pendingPool[0x100];

// GLOBAL: TONY2 0x004e624c
TonyS32 g_pendingCount;

// Per-track master volumes seeded to 0x7f on channel start.
// GLOBAL: TONY2 0x004fcbc0
TonyU8 g_trackVolumes[0x40];

// Multimedia timer id, timer period, thread-mode flag and thread handle of the
// mixer service.
// GLOBAL: TONY2 0x004e6288
TonyU32 g_mixerTimerId;

// GLOBAL: TONY2 0x004e628c
TonyU32 g_mixerTimerPeriod;

// GLOBAL: TONY2 0x004e6290
TonyU8 g_mixerThreadMode;

// GLOBAL: TONY2 0x004e6294
HANDLE g_mixerThread;

// Locked-ring segment pointers and lengths from IDirectSoundBuffer::Lock.
// GLOBAL: TONY2 0x004e6250
void* g_ringSegment1;

// GLOBAL: TONY2 0x004e6254
void* g_ringSegment2;

// GLOBAL: TONY2 0x004e6258
TonyU32 g_ringSegment1Size;

// GLOBAL: TONY2 0x004e625c
TonyU32 g_ringSegment2Size;

// Ring write cursor, lead distance and ring size.
// GLOBAL: TONY2 0x004e6274
TonyU32 g_ringWriteCursor;

// GLOBAL: TONY2 0x004e62a4
TonyU32 g_ringLead;

// GLOBAL: TONY2 0x004f1b28
TonyS32 g_ringSize;

// Head of the secondary-buffer capabilities block, memset as a 0x14-byte DSBCAPS by
// DSndNegotiateFormat; the dwBufferBytes member is g_ringSize.
// GLOBAL: TONY2 0x004f1b20
TonyU32 g_dsndBufferCaps;

// The negotiated output format; m_blockAlign is the output frame quantum.
// GLOBAL: TONY2 0x004f05e0
OutputFormat g_outputFormat;

// Saved FPU control word of the mixer service.
// GLOBAL: TONY2 0x004e6268
TonyU32 g_mixerFpuControl;

// First-tick flag of the timer-driven service.
// GLOBAL: TONY2 0x004e6291
TonyU8 g_mixerFirstTick;

// Mixer service thread handle and id.
// GLOBAL: TONY2 0x004e6298
HANDLE g_mixerServiceThread;

// GLOBAL: TONY2 0x004e629c
TonyU32 g_mixerServiceThreadId;

// Sequencer service arm flag.
// GLOBAL: TONY2 0x004ff620
TonyU8 g_seqServiceArmed;

// Master track group enable.
// GLOBAL: TONY2 0x0050632c
TonyU8 g_trackGroupEnable;

// Crossfade position step per level query.
// GLOBAL: TONY2 0x004e5dd0
TonyU32 g_fadePositionStep;

// Mixer-active flag set by the init entry and cleared by the shutdown twins.
// GLOBAL: TONY2 0x004e6248
TonyU8 g_mixerActive;

// The sound-effect instance slots.
// Per-track transpose bytes written by the track-transpose event.
// GLOBAL: TONY2 0x004fff20
TonyU8 g_trackTranspose[0x20];

// GLOBAL: TONY2 0x004fff40
SeqVoice g_seqVoices[0x40];

// The eight sample channels.
// Free-list head for the sample channels' pending chains.
// GLOBAL: TONY2 0x004f1b84
PoolNode* g_pendingFree;

// GLOBAL: TONY2 0x004f1ba0
SampleChannel g_sampleChannels[8];

// Mixer semaphore for the reentrant lock pair MixerLock/MixerUnlock.
// GLOBAL: TONY2 0x004e6264
HANDLE g_mixerSemaphore;

// The DirectSound device and its primary and output buffers.
// GLOBAL: TONY2 0x004f1b00
IDSBuffer* g_dsndPrimaryBuffer;

// GLOBAL: TONY2 0x004f1b04
IDSDevice* g_dsndDevice;

// GLOBAL: TONY2 0x004f1b0c
IDSBuffer* g_dsndOutputBuffer;

// Enumerated DirectSound devices, their count, and the opened device index.
// GLOBAL: TONY2 0x004efd40
DeviceRecord g_dsndDevices[8];

// GLOBAL: TONY2 0x004f1b08
TonyU16 g_dsndDeviceCount;

// GLOBAL: TONY2 0x004f1b34
TonyU16 g_dsndOpenedDevice;

// Set when GetVersionEx reports a non-Win9x platform.
// GLOBAL: TONY2 0x004e6260
TonyU8 g_platformIsNt;

// Mixer staging buffer freed on shutdown.
// GLOBAL: TONY2 0x004e6278
void* g_mixerStaging;

// Bytes per output frame (depth/4 in stereo, depth/8 in mono).
// GLOBAL: TONY2 0x004e6270
TonyS32 g_bytesPerFrame;

// GLOBAL: TONY2 0x004e627c
TonyS32 g_mixerBlockBytes;

// Driver banner published on a successful open.
// GLOBAL: TONY2 0x00456248
TonyChar g_driverBanner[] = "DirectSound, V2.1 (Dec 10 1998)";

// GLOBAL: TONY2 0x0050c6c0
DeviceInfo g_deviceInfo;

// Set when the output device is lost; skips the buffer release on shutdown.
// GLOBAL: TONY2 0x004e6280
TonyU8 g_deviceLost;

// GLOBAL: TONY2 0x004e62a8
TonyU8 g_outputPaused;

// Reentrancy depth of the mixer lock.
// GLOBAL: TONY2 0x004e62ac
TonyS32 g_mixerLockDepth;

// Primary mixing buffer, released on shutdown.
// GLOBAL: TONY2 0x004e62b4
void* g_mixBusPrimary;

// Mixer scratch buffer freed on shutdown.
// GLOBAL: TONY2 0x004e6b5c
void* g_mixerScratch;

// GLOBAL: TONY2 0x004e6b68
TonyS32 g_speechChannelCount;

// Voice channel bank (a single channel in this build).
// GLOBAL: TONY2 0x004e6b70
SpeechChannel g_speechChannels[1];

// Semaphore guarding the voice mixer (CreateSemaphoreA(NULL, 1, 1, NULL)).
// GLOBAL: TONY2 0x004ead58
HANDLE g_speechSemaphore;

// Reentrancy depth of the voice-mixer lock.
// GLOBAL: TONY2 0x004ead60
TonyS32 g_speechLockDepth;

// GLOBAL: TONY2 0x004ead5c
TonyU8 g_speechBufferOwned;

// Performance-counter snapshots around the voice pump and their delta.
// GLOBAL: TONY2 0x0050d0e8
LARGE_INTEGER g_speechPumpStart;

// GLOBAL: TONY2 0x0050d0f0
LARGE_INTEGER g_speechPumpEnd;

// GLOBAL: TONY2 0x0050d100
TonyS32 g_speechPumpDelta;

// Voice-mixer-running flag set by SpeechStartup and cleared by SpeechShutdown.
// GLOBAL: TONY2 0x004ead5d
TonyU8 g_speechRunning;

// Master output mode byte.
// GLOBAL: TONY2 0x004e626c
TonyU8 g_outputMode;

// GLOBAL: TONY2 0x004fab88
TonyU8 g_channelBusyCount;

// The sample channel being pumped: state pointer, index and resolved program.
// GLOBAL: TONY2 0x004fab80
SampleChannel* g_activeChannel;

// GLOBAL: TONY2 0x004f9364
TonyS32 g_activeChannelIndex;

// GLOBAL: TONY2 0x004f9360
TonyU8 g_activeProgram;

// Playing-channel count used by the idle query sweep.
// The track fade table.
// GLOBAL: TONY2 0x00505e40
TrackFade g_trackFades[0x20];

// GLOBAL: TONY2 0x00508360
TonyU8 g_playingChannelCount;

// GLOBAL: TONY2 0x0050c8c8
TonyS32 g_speechSaved1;

// GLOBAL: TONY2 0x0050c8cc
TonyS32 g_speechSaved2;

// GLOBAL: TONY2 0x0050c8d0
TonyU8 g_speechSavedFlag;

// Secondary voice buffer, released on shutdown when g_speechBufferOwned is set.
// GLOBAL: TONY2 0x0050d0e0
void* g_speechSecondaryBuffer;

// GLOBAL: TONY2 0x0050d0e4
TonyS32* g_speechStagingPtr;

// GLOBAL: TONY2 0x0050d0f8
TonyS32 g_speechStagingLen;

// The voice mixing buffer ("voice_buffer" per the gsmvoice.c:721 assert).
// GLOBAL: TONY2 0x0050d0fc
void* g_speechMixBuffer;

// FUNCTION: TONY2 0x004175f7
void __fastcall SndFree(void* p_memory)
{
	free(p_memory);
}

// Multiplicative congruential random step.
// FUNCTION: TONY2 0x0041760e
TonyU32 SndRandom(void)
{
	g_sndRandomSeed = g_sndRandomSeed * 0xa8351d63;
	return g_sndRandomSeed >> 6;
}

// Q12 sine from the quarter-wave ROM.
// FUNCTION: TONY2 0x0041762b
TonyS16 __fastcall SndSineQ12(TonyU16 p_phase)
{
	p_phase &= 0xfff;

	if (p_phase < 0x400) {
		return g_sndSineRom[p_phase];
	}

	if (p_phase < 0x800) {
		return g_sndSineRom[0x3ff - (p_phase & 0x3ff)];
	}

	if (p_phase < 0xc00) {
		return -g_sndSineRom[p_phase & 0x3ff];
	}

	return -g_sndSineRom[0x3ff - (p_phase & 0x3ff)];
}

// Stores a slot's tick rate scaled to the sequencer base (0x600 ticks per 0xf0 beat).
// FUNCTION: TONY2 0x004176df
void __fastcall SeqSetSlotTickRate(TonyU32 p_rate, TonyU8 p_slot)
{
	if (p_slot == 0xff) {
		p_slot = 8;
	}

	g_seqSlotTickRates[p_slot] = p_rate * 0x600 / 0xf0;
}

// Returns the record's tempo-slot tick rate (8 = default).
// FUNCTION: TONY2 0x00417724
TonyS32 __fastcall SeqGetTempoTickRate(SeqRecord* p_record)
{
	return g_seqSlotTickRates[p_record->m_category == 0xff ? 8 : p_record->m_category];
}

// Stores a controller value (7 bits) into the channel's row.
// FUNCTION: TONY2 0x0041775f
void __fastcall SeqSetController(TonyU8 p_ctrl, TonyU8 p_channel, TonyU8 p_category, TonyU8 p_value)
{
	if (p_category != 0xff) {
		g_seqControllersByCategory[p_category][p_channel][p_ctrl] = p_value & 0x7f;
	}
	else {
		g_seqControllers[p_channel][p_ctrl] = p_value & 0x7f;
	}
}

// Stores a channel's program byte.
// FUNCTION: TONY2 0x004177e7
void __fastcall SeqSetProgram(TonyU8 p_channel, TonyU8 p_category, TonyU8 p_value)
{
	if (p_category != 0xff) {
		g_seqPrograms[p_category][p_channel] = p_value;
	}
	else {
		g_seqProgramsDefault[p_channel] = p_value;
	}
}

// Returns a channel's program byte.
// FUNCTION: TONY2 0x0041783b
TonyU8 __fastcall SeqGetProgram(TonyU8 p_channel, TonyU8 p_category)
{
	if (p_category != 0xff) {
		return g_seqPrograms[p_category][p_channel];
	}

	return g_seqProgramsDefault[p_channel];
}

// Stores a channel's playable flag.
// FUNCTION: TONY2 0x00417886
void __fastcall SeqSetPlayable(TonyU8 p_channel, TonyU8 p_category, TonyU8 p_value)
{
	if (p_category != 0xff) {
		g_seqProgramsAlt[p_category][p_channel] = p_value;
	}
	else {
		g_seqPlayableFlags[p_channel] = p_value;
	}
}

// Checks whether a sound id in a bank is playable: the per-category table when a
// category is given, the default table otherwise.
// FUNCTION: TONY2 0x004178da
TonyU8 __fastcall SeqIsPlayable(TonyU8 p_sound, TonyU8 p_category)
{
	if (p_category != 0xff) {
		return g_seqProgramsAlt[p_category][p_sound];
	}

	return g_seqPlayableFlags[p_sound];
}

// Resets a channel's controller row and reapplies the driver defaults (volume 0x7f,
// pans centered, pedals/effects off, program 0xff, bend 0).
// FUNCTION: TONY2 0x00417925
void __fastcall SeqResetChannel(TonyU8 p_channel, TonyU8 p_category)
{
	TonyU32 i;

	if (p_category != 0xff) {
		for (i = 0; i < 0x86; i++) {
			g_seqControllersByCategory[p_category][p_channel][i] = 0;
		}
	}
	else {
		for (i = 0; i < 0x86; i++) {
			g_seqControllers[p_channel][i] = 0;
		}
	}

	SeqSetController(7, p_channel, p_category, 0x7f);
	SeqSetController(0xa, p_channel, p_category, 0x40);
	SeqSetController(0x80, p_channel, p_category, 0x40);
	SeqSetController(0x81, p_channel, p_category, 0);
	SeqSetController(0x40, p_channel, p_category, 0);
	SeqSetController(0x41, p_channel, p_category, 0);
	SeqSetController(0x5b, p_channel, p_category, 0);
	SeqSetController(0x83, p_channel, p_category, 0);
	SeqSetController(0x84, p_channel, p_category, 0x40);
	SeqSetController(0x85, p_channel, p_category, 0);
	SeqSetProgram(p_channel, p_category, 0xff);
	SeqSetPlayable(p_channel, p_category, 0);
}

// Returns a controller's effective 14-bit value: MSB/LSB pairs for 0..0x3f, the
// paired bytes for 0x80/0x81, switch thresholds for the pedals, zero for the data
// entry range, and the scaled single byte otherwise.
// FUNCTION: TONY2 0x00417a80
TonyU16 __fastcall SeqGetController14(TonyU8 p_ctrl, TonyU8 p_channel, TonyU8 p_category)
{
	if (p_category != 0xff) {
		if (p_ctrl < 0x40) {
			return (g_seqControllersByCategory[p_category][p_channel][p_ctrl & 0x1f] << 7) |
				   g_seqControllersByCategory[p_category][p_channel][(p_ctrl & 0x1f) + 0x20];
		}

		if (p_ctrl == 0x80 || p_ctrl == 0x81) {
			return (g_seqControllersByCategory[p_category][p_channel][p_ctrl & 0xfe] << 7) |
				   g_seqControllersByCategory[p_category][p_channel][(p_ctrl & 0xfe) + 1];
		}

		if (p_ctrl < 0x46) {
			return g_seqControllersByCategory[p_category][p_channel][p_ctrl] < 0x40 ? 0 : 0x3fff;
		}

		if (p_ctrl >= 0x60 && p_ctrl < 0x66) {
			return 0;
		}

		return g_seqControllersByCategory[p_category][p_channel][p_ctrl] << 7;
	}

	if (p_ctrl < 0x40) {
		return (g_seqControllers[p_channel][p_ctrl & 0x1f] << 7) |
			   g_seqControllers[p_channel][(p_ctrl & 0x1f) + 0x20];
	}

	if (p_ctrl == 0x80 || p_ctrl == 0x81) {
		return (g_seqControllers[p_channel][p_ctrl & 0xfe] << 7) |
			   g_seqControllers[p_channel][(p_ctrl & 0xfe) + 1];
	}

	if (p_ctrl < 0x46) {
		return g_seqControllers[p_channel][p_ctrl] < 0x40 ? 0 : 0x3fff;
	}

	if (p_ctrl >= 0x60 && p_ctrl < 0x66) {
		return 0;
	}

	return g_seqControllers[p_channel][p_ctrl] << 7;
}

// Loads the record's default modulation routes: volume, pan, effect and bend targets
// with unity scale, then the pedal/switch routes.
// FUNCTION: TONY2 0x00417de1
void __fastcall ModLoadDefaults(SeqRecord* p_record)
{
	p_record->m_levelRouteCtrl = 7;
	p_record->m_levelRouteOp = 0;
	p_record->m_levelRouteScale = 0x100;
	p_record->m_levelRouteCount = 1;
	p_record->m_volumeRouteCtrl = 0xa;
	p_record->m_volumeRouteOp = 0;
	p_record->m_volumeRouteScale = 0x100;
	p_record->m_volumeRouteCount = 1;
	p_record->m_panRouteCtrl = 0x83;
	p_record->m_panRouteOp = 0;
	p_record->m_panRouteScale = 0x100;
	p_record->m_panRouteCount = 1;
	p_record->m_rateRouteCtrl = 0x80;
	p_record->m_rateRouteOp = 0;
	p_record->m_rateRouteScale = 0x100;
	p_record->m_rateRouteCount = 1;
	p_record->m_vibDepthRouteCtrl = 1;
	p_record->m_vibDepthRouteOp = 0;
	p_record->m_vibDepthRouteScale = 0x100;
	p_record->m_vibDepthRouteCount = 1;
	p_record->m_vibRateRouteCtrl = 0x40;
	p_record->m_vibRateRouteOp = 0;
	p_record->m_vibRateRouteScale = 0x100;
	p_record->m_vibRateRouteCount = 1;
	p_record->m_wheelRouteCtrl = 0x41;
	p_record->m_wheelRouteOp = 0;
	p_record->m_wheelRouteScale = 0x100;
	p_record->m_wheelRouteCount = 1;
	p_record->m_bendRouteCtrl = 0x5b;
	p_record->m_bendRouteOp = 0;
	p_record->m_bendRouteScale = 0x100;
	p_record->m_bendRouteCount = 1;
	p_record->m_effectsRouteCtrl = 0x84;
	p_record->m_effectsRouteOp = 0;
	p_record->m_effectsRouteScale = 0x100;
	p_record->m_effectsRouteCount = 1;
}

// Evaluates the record's modulation routes from their block base.
// FUNCTION: TONY2 0x00417f66
TonyU16 __fastcall ModEvalRoutes(SeqRecord* p_record)
{
	return ModEvalRouteBlock(p_record, &p_record->m_levelRouteCtrl);
}

// Modulation-route evaluator: runs the block's routes in order, sourcing each from
// the LFO words or the controller state, scaling by the signed Q8 route scale, and
// folding into the running total as replace/add/multiply - center-relative 13-bit
// math for the bipolar targets, unsigned 14-bit for the rest.
// FUNCTION: TONY2 0x00417f82
TonyU16 __fastcall ModEvalRouteBlock(SeqRecord* p_record, TonyU8* p_routes)
{
	TonyS32 result;
	TonyS32 value;
	TonyS32 acc;
	TonyU32 i;
	TonyU8 target;

	result = 0;

	for (i = 0; i < p_routes[0x10]; i++) {
		target = p_routes[i * 4];

		if (target == 0x80 || target == 1 || target == 0xa || target == 0xa0 || target == 0xa1 ||
			target == 0x83 || target == 0x84) {
			acc = result - 0x2000;

			switch (target) {
			case 0xa0:
				value = p_record->m_timerValueA << 1;
				break;
			case 0xa1:
				value = p_record->m_timerValueB << 1;
				break;
			default:
				value = SeqGetController14(target, p_record->m_sound, p_record->m_category) - 0x2000;
			}

			value = value * *(TonyS16*) (p_routes + i * 4 + 2) >> 8;
			value = value < -0x2000 ? (value = -0x2000) : (value > 0x1fff ? 0x1fff : value);

			switch (p_routes[i * 4 + 1]) {
			case 0:
				acc = value;
				break;
			case 1:
				acc += value;
				acc = acc < -0x2000 ? -0x2000 : (acc > 0x1fff ? 0x1fff : acc);
				break;
			case 2:
				acc = acc * value >> 0xd;
				acc = acc < -0x2000 ? -0x2000 : (acc > 0x1fff ? 0x1fff : acc);
				break;
			}

			result = acc + 0x2000;
		}
		else {
			value = SeqGetController14(target, p_record->m_sound, p_record->m_category) *
						*(TonyS16*) (p_routes + i * 4 + 2) >>
					8;

			if (value > 0x3fff) {
				value = 0x3fff;
			}

			switch (p_routes[i * 4 + 1]) {
			case 0:
				result = value;
				break;
			case 1:
				result += value;
				result = (TonyU32) result > 0x3fff ? 0x3fff : result;
				break;
			case 2:
				result = (TonyU32) (result * value) >> 0xe;
				result = (TonyU32) result > 0x3fff ? 0x3fff : result;
				break;
			}
		}
	}

	return result;
}

// Evaluates the route group at +0xd2.
// FUNCTION: TONY2 0x00418275
TonyU16 __fastcall ModEvalVolumeGroup(SeqRecord* p_record)
{
	return ModEvalRouteBlock(p_record, &p_record->m_volumeRouteCtrl);
}

// Evaluates the route group at +0xe4.
// FUNCTION: TONY2 0x00418291
TonyU16 __fastcall ModEvalPanGroup(SeqRecord* p_record)
{
	return ModEvalRouteBlock(p_record, &p_record->m_panRouteCtrl);
}

// Evaluates the route group at +0xf6.
// FUNCTION: TONY2 0x004182ad
TonyU16 __fastcall ModEvalRateGroup(SeqRecord* p_record)
{
	return ModEvalRouteBlock(p_record, &p_record->m_rateRouteCtrl);
}

// Evaluates the route group at +0x108.
// FUNCTION: TONY2 0x004182c9
TonyU16 __fastcall ModEvalEffectsGroup(SeqRecord* p_record)
{
	return ModEvalRouteBlock(p_record, &p_record->m_effectsRouteCtrl);
}

// Evaluates the route group at +0x11a.
// FUNCTION: TONY2 0x004182e5
TonyU16 __fastcall ModEvalVibDepthGroup(SeqRecord* p_record)
{
	return ModEvalRouteBlock(p_record, &p_record->m_vibDepthRouteCtrl);
}

// Evaluates the route group at +0x12c.
// FUNCTION: TONY2 0x00418301
TonyU16 __fastcall ModEvalVibRateGroup(SeqRecord* p_record)
{
	return ModEvalRouteBlock(p_record, &p_record->m_vibRateRouteCtrl);
}

// Evaluates the route group at +0x13e.
// FUNCTION: TONY2 0x0041831d
TonyU16 __fastcall ModEvalWheelGroup(SeqRecord* p_record)
{
	return ModEvalRouteBlock(p_record, &p_record->m_wheelRouteCtrl);
}

// Evaluates the route group at +0x150.
// FUNCTION: TONY2 0x00418339
TonyU16 __fastcall ModEvalBendGroup(SeqRecord* p_record)
{
	return ModEvalRouteBlock(p_record, &p_record->m_bendRouteCtrl);
}

// Inserts a value into the sorted event table under the mixer lock; rejects
// duplicate keys and overflow. Returns TRUE on insert.
// FUNCTION: TONY2 0x00418355
TonyU8 __fastcall RegInsert(TonyU16 p_key, TonyS32 p_value)
{
	TonyS32 i;
	TonyS32 j;

	MixerLock();

	for (i = 0; i < g_regCount && g_regTable[i].m_key < p_key; i++) {
	}

	if (i < g_regCount) {
		if (g_regTable[i].m_key != p_key) {
			if (g_regCount < 0x100) {
				for (j = g_regCount - 1; j >= i; j--) {
					g_regTable[j + 1] = g_regTable[j];
				}

				g_regCount++;
			}
			else {
				MixerUnlock();
				return 0;
			}
		}
		else {
			MixerUnlock();
			return 0;
		}
	}
	else {
		if (g_regCount < 0x100) {
			g_regCount++;
		}
		else {
			MixerUnlock();
			return 0;
		}
	}

	g_regTable[i].m_key = p_key;
	g_regTable[i].m_value = p_value;
	MixerUnlock();
	return 1;
}

// Removes an event by key under the mixer lock; returns TRUE when found. Parked as a
// FUNCTION: TONY2 0x0041848a
TonyU8 __fastcall RegRemove(TonyU16 p_key)
{
	TonyS32 i;
	TonyS32 j;

	MixerLock();

	for (i = 0; i < g_regCount && g_regTable[i].m_key != p_key; i++) {
	}

	if (i != g_regCount) {
		for (j = i + 1; j < g_regCount; j++) {
			g_regTable[j - 1] = g_regTable[j];
		}

		g_regCount--;
		MixerUnlock();
		return 1;
	}

	MixerUnlock();
	return 0;
}

// Inserts a tagged value into the second sorted event table under the mixer lock;
// rejects duplicate keys and overflow.
// FUNCTION: TONY2 0x00418542
TonyU8 __fastcall RegInsertTagged(TonyU16 p_key, TonyS32 p_value, TonyU16 p_tag)
{
	TonyS32 i;
	TonyS32 j;

	MixerLock();

	for (i = 0; i < g_regTaggedCount && g_regTaggedTable[i].m_key < p_key; i++) {
	}

	if (i < g_regTaggedCount) {
		if (g_regTaggedTable[i].m_key != p_key) {
			if (g_regTaggedCount < 0x100) {
				for (j = g_regTaggedCount - 1; j >= i; j--) {
					g_regTaggedTable[j + 1] = g_regTaggedTable[j];
				}

				g_regTaggedCount++;
			}
			else {
				MixerUnlock();
				return 0;
			}
		}
		else {
			MixerUnlock();
			return 0;
		}
	}
	else {
		if (g_regTaggedCount < 0x100) {
			g_regTaggedCount++;
		}
		else {
			MixerUnlock();
			return 0;
		}
	}

	g_regTaggedTable[i].m_key = p_key;
	g_regTaggedTable[i].m_value = p_value;
	g_regTaggedTable[i].m_tag = p_tag;
	MixerUnlock();
	return 1;
}

// Removes an event by key from the second table under the mixer lock. Parked as the
// same reccmp symbolization artifact as RegRemove: the entry j - 1 store folds to
// the table base minus 8 (bytes identical).
// FUNCTION: TONY2 0x00418688
TonyU8 __fastcall RegRemoveTagged(TonyU16 p_key)
{
	TonyS32 i;
	TonyS32 j;

	MixerLock();

	for (i = 0; i < g_regTaggedCount && g_regTaggedTable[i].m_key != p_key; i++) {
	}

	if (i != g_regTaggedCount) {
		for (j = i + 1; j < g_regTaggedCount; j++) {
			g_regTaggedTable[j - 1] = g_regTaggedTable[j];
		}

		g_regTaggedCount--;
		MixerUnlock();
		return 1;
	}

	MixerUnlock();
	return 0;
}

// Inserts a value into the third sorted event table under the mixer lock; rejects
// duplicate keys and overflow.
// FUNCTION: TONY2 0x00418740
TonyU8 __fastcall RegInsertAlt(TonyU16 p_key, TonyS32 p_value)
{
	TonyS32 i;
	TonyS32 j;

	MixerLock();

	for (i = 0; i < g_regAltCount && g_regAltTable[i].m_key < p_key; i++) {
	}

	if (i < g_regAltCount) {
		if (g_regAltTable[i].m_key != p_key) {
			if (g_regAltCount < 0x100) {
				for (j = g_regAltCount - 1; j >= i; j--) {
					g_regAltTable[j + 1] = g_regAltTable[j];
				}

				g_regAltCount++;
			}
			else {
				MixerUnlock();
				return 0;
			}
		}
		else {
			MixerUnlock();
			return 0;
		}
	}
	else {
		if (g_regAltCount < 0x100) {
			g_regAltCount++;
		}
		else {
			MixerUnlock();
			return 0;
		}
	}

	g_regAltTable[i].m_key = p_key;
	g_regAltTable[i].m_value = p_value;
	MixerUnlock();
	return 1;
}

// Removes an event by key from the third table under the mixer lock. Parked as the
// same reccmp symbolization artifact as RegRemove (bytes identical).
// FUNCTION: TONY2 0x00418875
TonyU8 __fastcall RegRemoveAlt(TonyU16 p_key)
{
	TonyS32 i;
	TonyS32 j;

	MixerLock();

	for (i = 0; i < g_regAltCount && g_regAltTable[i].m_key != p_key; i++) {
	}

	if (i != g_regAltCount) {
		for (j = i + 1; j < g_regAltCount; j++) {
			g_regAltTable[j - 1] = g_regAltTable[j];
		}

		g_regAltCount--;
		MixerUnlock();
		return 1;
	}

	MixerUnlock();
	return 0;
}

// Inserts a value pair into the wide sorted event table under the mixer lock;
// rejects duplicate keys and overflow.
// FUNCTION: TONY2 0x0041892d
TonyU8 __fastcall SongRegInsert(TonyU16 p_key, TonyS32 p_value, TonyS32 p_value2)
{
	TonyS32 i;
	TonyS32 j;

	MixerLock();

	for (i = 0; i < g_songRegCount && g_songRegTable[i].m_key < p_key; i++) {
	}

	if (i < g_songRegCount) {
		if (g_songRegTable[i].m_key != p_key) {
			if (g_songRegCount < 0x100) {
				for (j = g_songRegCount - 1; j >= i; j--) {
					g_songRegTable[j + 1] = g_songRegTable[j];
				}

				g_songRegCount++;
			}
			else {
				MixerUnlock();
				return 0;
			}
		}
		else {
			MixerUnlock();
			return 0;
		}
	}
	else {
		if (g_songRegCount < 0x100) {
			g_songRegCount++;
		}
		else {
			MixerUnlock();
			return 0;
		}
	}

	g_songRegTable[i].m_key = p_key;
	g_songRegTable[i].m_value = p_value;
	g_songRegTable[i].m_data = p_value2;
	MixerUnlock();
	return 1;
}

// Removes an event by key from the wide table under the mixer lock. Parked as the
// same reccmp symbolization artifact as RegRemove (bytes identical).
// FUNCTION: TONY2 0x00418a83
TonyU8 __fastcall SongRegRemove(TonyU16 p_key)
{
	TonyS32 i;
	TonyS32 j;

	MixerLock();

	for (i = 0; i < g_songRegCount && g_songRegTable[i].m_key != p_key; i++) {
	}

	if (i != g_songRegCount) {
		for (j = i + 1; j < g_songRegCount; j++) {
			g_songRegTable[j - 1] = g_songRegTable[j];
		}

		g_songRegCount--;
		MixerUnlock();
		return 1;
	}

	MixerUnlock();
	return 0;
}

// Inserts or overwrites a trigger under the mixer lock: sorted by key, duplicates
// update in place, new entries armed with lifetime 0x1f.
// FUNCTION: TONY2 0x00418b45
TonyU8 __fastcall TriggerInsert(
	TonyU16 p_key,
	TonyU16 p_rate,
	TonyU8 p_priority,
	TonyU8 p_level,
	TonyU8 p_pan,
	TonyU8 p_note,
	TonyU8 p_track
)
{
	TonyS32 i;
	TonyS32 j;

	MixerLock();

	for (i = 0; i < g_triggerCount && g_triggerTable[i].m_id < p_key; i++) {
	}

	if (i < g_triggerCount) {
		if (g_triggerTable[i].m_id != p_key) {
			if (g_triggerCount < 0x100) {
				for (j = g_triggerCount - 1; j >= i; j--) {
					g_triggerTable[j + 1] = g_triggerTable[j];
				}

				g_triggerCount++;
			}
			else {
				MixerUnlock();
				return 0;
			}
		}
	}
	else {
		if (g_triggerCount < 0x100) {
			g_triggerCount++;
		}
		else {
			MixerUnlock();
			return 0;
		}
	}

	g_triggerTable[i].m_id = p_key;
	g_triggerTable[i].m_listId = p_rate;
	g_triggerTable[i].m_pan = p_priority;
	g_triggerTable[i].m_level = p_level;
	g_triggerTable[i].m_priority = p_pan;
	g_triggerTable[i].m_polyphony = p_note;
	g_triggerTable[i].m_note = p_track;
	g_triggerTable[i].m_track = 0x1f;
	MixerUnlock();
	return 1;
}

// Removes a trigger by key under the mixer lock; returns TRUE when found.
// FUNCTION: TONY2 0x00418ce2
TonyU8 __fastcall TriggerRemove(TonyU16 p_key)
{
	TonyS32 i;
	TonyS32 j;

	MixerLock();

	for (i = 0; i < g_triggerCount && g_triggerTable[i].m_id != p_key; i++) {
	}

	if (i != g_triggerCount) {
		for (j = i + 1; j < g_triggerCount; j++) {
			g_triggerTable[j - 1] = g_triggerTable[j];
		}

		g_triggerCount--;
		MixerUnlock();
		return 1;
	}

	MixerUnlock();
	return 0;
}

// Inserts a value into the bucketed note pool under the mixer lock: 64-key buckets
// index a shared sorted pool; later bucket starts are bumped past the insertion.
// FUNCTION: TONY2 0x00418da4
TonyU8 __fastcall NotePoolInsert(TonyU16 p_key, TonyS32 p_value)
{
	TonyS32 bucket;
	TonyS32 k;
	TonyS32 start;
	TonyS32 pos;

	MixerLock();
	bucket = p_key >> 6;

	if (g_noteBuckets[bucket].m_count == 0) {
		g_noteBuckets[bucket].m_start = g_notePoolCursor;
		start = (TonyU16) g_notePoolCursor;
		pos = start;
	}
	else {
		start = g_noteBuckets[bucket].m_start;

		for (k = 0; k < g_noteBuckets[bucket].m_count && g_notePool[start + k].m_key < p_key;
			 k++) {
		}

		if (k < g_noteBuckets[bucket].m_count) {
			pos = start + k;

			if (g_notePool[pos].m_key == p_key) {
				MixerUnlock();
				return 0;
			}
		}
		else {
			pos = start + k;
		}
	}

	if (g_notePoolCursor < 0x400) {
		for (k = 0; k < 0x200; k++) {
			if (g_noteBuckets[k].m_start > start) {
				g_noteBuckets[k].m_start++;
			}
		}

		for (k = g_notePoolCursor - 1; k >= pos; k--) {
			g_notePool[k + 1] = g_notePool[k];
		}

		g_notePool[pos].m_key = p_key;
		g_notePool[pos].m_value = p_value;
		g_noteBuckets[bucket].m_count++;
		g_notePoolCursor++;
		MixerUnlock();
		return 1;
	}

	MixerUnlock();
	return 0;
}

// Removes a key from the bucketed note pool under the mixer lock; later bucket
// starts are bumped back. Always returns 0. Parked as the shared reccmp
// symbolization artifact: the entry k - 1 store folds to the pool base minus 8
// inside the unannotated gap (bytes identical).
// FUNCTION: TONY2 0x00418f8f
TonyU8 __fastcall NotePoolRemove(TonyU16 p_key)
{
	TonyS32 bucket;
	TonyS32 k;
	TonyS32 start;

	MixerLock();
	bucket = p_key >> 6;

	if (g_noteBuckets[bucket].m_count != 0) {
		start = g_noteBuckets[bucket].m_start;

		for (k = 0; k < g_noteBuckets[bucket].m_count && g_notePool[start + k].m_key != p_key;
			 k++) {
		}

		if (k < g_noteBuckets[bucket].m_count) {
			for (k = start + k + 1; k < g_notePoolCursor; k++) {
				g_notePool[k - 1] = g_notePool[k];
			}

			for (k = 0; k < 0x200; k++) {
				if (g_noteBuckets[k].m_start > start) {
					g_noteBuckets[k].m_start--;
				}
			}

			g_noteBuckets[bucket].m_count--;
			g_notePoolCursor--;
		}
	}

	MixerUnlock();
	return 0;
}

// One-based binary search over a sorted array; returns the matching entry or NULL.
// FUNCTION: TONY2 0x004190ed
void* __fastcall SndBSearch(
	void* p_key,
	void* p_base,
	TonyS32 p_count,
	TonyS32 p_size,
	TonyS32(__cdecl* p_compare)(void*, void*)
)
{
	void* node;
	TonyS32 s1;
	TonyS32 s2;
	TonyS32 s3;
	TonyS32 s4;

	if (p_count != 0) {
		s3 = 1;
		s1 = p_count;

		do {
			s4 = (s3 + s1) >> 1;
			node = (char*) p_base + p_size * (s4 - 1);
			s2 = p_compare(p_key, node);

			if (s2 == 0) {
				return node;
			}

			if (s2 < 0) {
				s1 = s4 - 1;
			}
			else {
				s3 = s4 + 1;
			}
		} while (s3 <= s1);
	}

	return NULL;
}

// Key comparator for the sorted event entries.
// FUNCTION: TONY2 0x00419171
TonyS32 __cdecl RegCompareEntries(SortedEvent* p_a, SortedEvent* p_b)
{
	return p_a->m_key - p_b->m_key;
}

// Looks a key up in the bucketed note pool through the binary search; the scratch
// state lives in globals. Returns the entry's value or 0.
// FUNCTION: TONY2 0x0041918c
TonyS32 __fastcall NotePoolLookup(TonyU16 p_key)
{
	g_notePoolBucketIndex = p_key >> 6;

	if (g_noteBuckets[g_notePoolBucketIndex].m_count != 0) {
		g_notePoolBucketStart = g_noteBuckets[g_notePoolBucketIndex].m_start;
		g_notePoolKeyTemplate.m_key = p_key;
		g_notePoolResult = (SortedEvent*) SndBSearch(
			&g_notePoolKeyTemplate,
			&g_notePool[g_notePoolBucketStart],
			g_noteBuckets[g_notePoolBucketIndex].m_count,
			8,
			(TonyS32(__cdecl*)(void*, void*)) RegCompareEntries
		);

		if (g_notePoolResult != NULL) {
			return g_notePoolResult->m_value;
		}
	}

	return 0;
}

// Key comparator for the wide sorted event entries.
// FUNCTION: TONY2 0x00419222
TonyS32 __cdecl SongRegCompare(WideEvent* p_a, WideEvent* p_b)
{
	return p_a->m_key - p_b->m_key;
}

// Looks a song id up in the wide event table and unpacks its header into the
// descriptor. The wide entries hold wave descriptors, which overlay the song-header
// slots as rate / packed end|flags / loop start / loop length. Returns 0 on success
// or -1.
// FUNCTION: TONY2 0x0041923d
TonyS32 __fastcall SongLookup(TonyU16 p_key, SongDesc* p_out)
{
	g_songKeyTemplate.m_key = p_key;
	g_songEntryResult = (WideEvent*) SndBSearch(
		&g_songKeyTemplate,
		g_songRegTable,
		g_songRegCount,
		0xc,
		(TonyS32(__cdecl*)(void*, void*)) SongRegCompare
	);

	if (g_songEntryResult != NULL) {
		g_songHeaderResult = (SongBankHeader*) g_songEntryResult->m_value;
		p_out->m_rate = g_songHeaderResult->m_trackTableOfs;
		p_out->m_data = g_songEntryResult->m_data;
		p_out->m_startPos = 0;
		p_out->m_loopStart = g_songHeaderResult->m_routeMapOfs;
		p_out->m_endPos = g_songHeaderResult->m_noteTableOfs & 0xffffff;
		p_out->m_loopLength = g_songHeaderResult->m_tempoStreamOfs;
		p_out->m_flags = (TonyU8) (g_songHeaderResult->m_noteTableOfs >> 0x18);
		return 0;
	}

	return -1;
}

// Key comparator for the sorted event entries (second copy in the original).
// FUNCTION: TONY2 0x004192ff
TonyS32 __cdecl RegCompareEntriesB(SortedEvent* p_a, SortedEvent* p_b)
{
	return p_a->m_key - p_b->m_key;
}

// Looks a key up in the third event table; returns the value or 0.
// FUNCTION: TONY2 0x0041931a
TonyS32 __fastcall RegLookupAlt(TonyU16 p_key)
{
	g_regKeyTemplate.m_key = p_key;
	g_regAltResult = (SortedEvent*) SndBSearch(
		&g_regKeyTemplate,
		g_regAltTable,
		g_regAltCount,
		8,
		(TonyS32(__cdecl*)(void*, void*)) RegCompareEntriesB
	);

	if (g_regAltResult != NULL) {
		return g_regAltResult->m_value;
	}

	return 0;
}

// Looks a key up in the primary event table; returns the value or 0.
// FUNCTION: TONY2 0x00419367
TonyS32 __fastcall RegLookup(TonyU16 p_key)
{
	g_regAltKeyTemplate.m_key = p_key;
	g_regResult = (SortedEvent*) SndBSearch(
		&g_regAltKeyTemplate,
		g_regTable,
		g_regCount,
		8,
		(TonyS32(__cdecl*)(void*, void*)) RegCompareEntriesB
	);

	if (g_regResult != NULL) {
		return g_regResult->m_value;
	}

	return 0;
}

// Key comparator for the tagged event entries.
// FUNCTION: TONY2 0x004193b4
TonyS32 __cdecl RegCompareTagged(TaggedEvent* p_a, TaggedEvent* p_b)
{
	return p_a->m_key - p_b->m_key;
}

// Looks a key up in the tagged event table; writes the tag through and returns the
// value, or 0.
// FUNCTION: TONY2 0x004193cf
TonyS32 __fastcall RegLookupTagged(TonyU16 p_key, TonyU16* p_outTag)
{
	g_regTaggedKeyTemplate.m_key = p_key;
	g_regTaggedResult = (TaggedEvent*) SndBSearch(
		&g_regTaggedKeyTemplate,
		g_regTaggedTable,
		g_regTaggedCount,
		8,
		(TonyS32(__cdecl*)(void*, void*)) RegCompareTagged
	);

	if (g_regTaggedResult != NULL) {
		*p_outTag = g_regTaggedResult->m_tag;
		return g_regTaggedResult->m_value;
	}

	return 0;
}

// Key comparator for the trigger entries.
// FUNCTION: TONY2 0x00419430
TonyS32 __cdecl TriggerCompare(TriggerEntry* p_a, TriggerEntry* p_b)
{
	return p_a->m_id - p_b->m_id;
}

// Finds a trigger entry by key, or NULL.
// FUNCTION: TONY2 0x00419449
TriggerEntry* __fastcall TriggerFind(TonyU16 p_key)
{
	g_triggerKeyTemplate.m_id = p_key;
	return (TriggerEntry*) SndBSearch(
		&g_triggerKeyTemplate,
		g_triggerTable,
		g_triggerCount,
		0xc,
		(TonyS32(__cdecl*)(void*, void*)) TriggerCompare
	);
}

// Unhooks an SFX instance from its handle chain; when it is the last one, its record
// leaves the sorted list for the free-record list.
// FUNCTION: TONY2 0x0041947c
void __fastcall VoiceUnhook(struct SeqVoice* p_slot)
{
	if (p_slot->m_handle != -1) {
		if (p_slot->m_chainPrev != -1) {
			g_seqVoices[p_slot->m_chainPrev & 0xff].m_chainNext = p_slot->m_chainNext;

			if (p_slot->m_chainNext != -1) {
				g_seqVoices[p_slot->m_chainNext & 0xff].m_chainPrev = p_slot->m_chainPrev;
			}
		}
		else if (p_slot->m_chainNext != -1) {
			p_slot->m_record->m_owner = p_slot->m_chainNext;
			g_seqVoices[p_slot->m_chainNext & 0xff].m_chainPrev = -1;
			g_seqVoices[p_slot->m_chainNext & 0xff].m_record = p_slot->m_record;
		}
		else {
			if (p_slot->m_record->m_prev != NULL) {
				p_slot->m_record->m_prev->m_next = p_slot->m_record->m_next;
			}
			else {
				g_voiceRecordList = p_slot->m_record->m_next;
			}

			if (p_slot->m_record->m_next != NULL) {
				p_slot->m_record->m_next->m_prev = p_slot->m_record->m_prev;
			}

			p_slot->m_record->m_next = g_voiceRecordFree;

			if (g_voiceRecordFree != NULL) {
				g_voiceRecordFree->m_prev = p_slot->m_record;
			}

			p_slot->m_record->m_prev = NULL;
			g_voiceRecordFree = p_slot->m_record;
		}
	}
}

// Mints a playing-voice handle for a prepared slot: takes a unique id, links a
// record from the free list into the sorted instance list and points the slot at it.
// FUNCTION: TONY2 0x004195c8
TonyS32 __fastcall VoiceMintHandle(SeqVoice* p_slot)
{
	TonyS32 result;
	PoolNode* cursor;
	PoolNode* prev;
	PoolNode* alloc;

	result = VoiceNextHandle();
	prev = NULL;

	for (cursor = g_voiceRecordList; cursor != NULL; cursor = cursor->m_next) {
		if ((TonyU32) cursor->m_handle > (TonyU32) result) {
			break;
		}

		if (cursor->m_handle == result) {
			result = VoiceNextHandle();
		}

		prev = cursor;
	}

	alloc = g_voiceRecordFree;

	if (alloc == NULL) {
		return -1;
	}

	g_voiceRecordFree = g_voiceRecordFree->m_next;

	if (g_voiceRecordFree != NULL) {
		g_voiceRecordFree->m_prev = NULL;
	}

	if (prev == NULL) {
		g_voiceRecordList = alloc;
	}
	else {
		prev->m_next = alloc;
	}

	alloc->m_prev = prev;
	alloc->m_next = cursor;

	if (cursor != NULL) {
		cursor->m_prev = alloc;
	}

	alloc->m_handle = result;
	alloc->m_owner = p_slot->m_handle;
	p_slot->m_record = alloc;
	return result;
}

// Returns the next voice handle, skipping -1.
// FUNCTION: TONY2 0x004196b4
TonyS32 VoiceNextHandle(void)
{
	TonyS32 handle;

	do {
		handle = g_voiceHandleCounter;
		g_voiceHandleCounter++;
	} while (handle == -1);

	return handle;
}

// Maps an instance handle to its owning id, or -1.
// FUNCTION: TONY2 0x004196dc
TonyS32 __fastcall VoiceHandleOwner(TonyS32 p_handle)
{
	PoolNode* node;

	if (p_handle != -1) {
		node = VoiceFindRecord(p_handle);

		if (node != NULL) {
			return node->m_owner;
		}
	}

	return -1;
}

// Finds the instance record for p_handle in the sorted list, or NULL.
// FUNCTION: TONY2 0x0041970b
PoolNode* __fastcall VoiceFindRecord(TonyS32 p_handle)
{
	PoolNode* cursor;

	for (cursor = g_voiceRecordList; cursor != NULL; cursor = cursor->m_next) {
		if (cursor->m_handle == p_handle) {
			return cursor;
		}

		if ((TonyU32) cursor->m_handle > (TonyU32) p_handle) {
			break;
		}
	}

	return NULL;
}

// Claims a mixer slot for a note voice: within the per-class polyphony budget it
// steals the oldest lowest-program voice of the note's rate group or class, and
// otherwise picks the free slot with the lowest program and age. Returns -1 when the
// requesting program is outranked.
// FUNCTION: TONY2 0x00419751
TonyS32 __fastcall VoiceClaimSlot(TonyU8 p_program, TonyU8 p_note, TonyU16 p_rate, TonyU8 p_flag)
{
	TonyS32 goal;
	TonyS32 victim;
	TonyS32 count;
	TonyS32 i;
	TonyS32 same;
	TonyS32 worst;

	if (!g_sndIdleQueryBusy) {
		goal = p_flag ? g_channelBusyCount : g_speechSavedFlag;

		if (p_note < g_playingChannelCount || goal < g_playingChannelCount) {
			same = 0;
			count = 0;
			victim = -1;
			worst = -1;

			for (i = 0; i < g_playingChannelCount; i++) {
				if (!g_seqVoices[i].m_reserved && g_seqVoices[i].m_eventList != NULL) {
					if (g_seqVoices[i].m_class == p_flag) {
						count++;

						if (!(g_seqVoices[i].m_flags & 1)) {
							if (worst != -1) {
								if (g_seqVoices[i].m_priority == g_seqVoices[worst].m_priority) {
									if (g_seqVoices[i].m_age < g_seqVoices[worst].m_age) {
										worst = i;
									}
								}
								else if (g_seqVoices[i].m_priority < g_seqVoices[worst].m_priority) {
									worst = i;
								}
							}
							else {
								worst = i;
							}
						}
					}

					if (g_seqVoices[i].m_groupKey == p_rate) {
						same++;

						if (!(g_seqVoices[i].m_flags & 1)) {
							if (victim != -1) {
								if (g_seqVoices[i].m_priority == g_seqVoices[victim].m_priority) {
									if (g_seqVoices[i].m_age < g_seqVoices[victim].m_age) {
										victim = i;
									}
								}
								else if (g_seqVoices[i].m_priority < g_seqVoices[victim].m_priority) {
									victim = i;
								}
							}
							else {
								victim = i;
							}
						}
					}
				}
			}

			if (same >= p_note) {
				if (victim != -1 && p_program >= g_seqVoices[victim].m_priority) {
					return victim;
				}

				return -1;
			}

			if (count >= goal) {
				if (worst != -1 && p_program >= g_seqVoices[worst].m_priority) {
					return worst;
				}

				return -1;
			}
		}

		victim = -1;

		for (i = 0; i < g_playingChannelCount; i++) {
			if (!(g_seqVoices[i].m_flags & 1) && !g_seqVoices[i].m_reserved) {
				if (victim != -1) {
					if (g_seqVoices[i].m_priority < g_seqVoices[victim].m_priority) {
						victim = i;
					}
					else if (g_seqVoices[victim].m_priority == g_seqVoices[i].m_priority &&
							 g_seqVoices[victim].m_age > g_seqVoices[i].m_age) {
						victim = i;
					}
				}
				else {
					victim = i;
				}
			}
		}

		if (victim != -1 && g_seqVoices[victim].m_priority <= p_program) {
			return victim;
		}
	}

	return -1;
}

// Reserves a mixer slot outside the note system: claims a slot at the given program
// with wildcard note and rate, marks it reserved and detaches it from the event pool.
// FUNCTION: TONY2 0x00419b60
TonyS32 __fastcall VoiceReserveSlot(TonyU8 p_program)
{
	TonyS32 result;

	result = VoiceClaimSlot(p_program, 0xff, 0xffff, 1);

	if (result != -1) {
		g_seqVoices[result].m_reserved = 1;
		VoiceUnhook(&g_seqVoices[result]);
		g_seqVoices[result].m_handle = -1;

		if (ChanIsActive(result)) {
			ChanRelease(result);
		}

		g_seqVoices[result].m_eventList = NULL;
		g_seqVoices[result].m_priority = 0;
	}

	return result;
}

// Tears down the mixer channel behind an SFX voice and clears its retrigger flag.
// FUNCTION: TONY2 0x00419bfe
void __fastcall VoiceTeardown(TonyS32 p_voice)
{
	if (p_voice != -1) {
		if (ChanIsActive(p_voice)) {
			ChanRelease(p_voice);
		}

		g_seqVoices[p_voice].m_reserved = 0;
	}
}

// Blended level for a crossfading slot: while the fade position runs, mixes the
// current and previous levels by the fade fraction and advances the position.
// FUNCTION: TONY2 0x00419c38
TonyS32 __fastcall VoiceBlendLevel(SeqVoice* p_slot)
{
	TonyS32 result;
	TonyU32 frac;

	if (p_slot->m_flags & 0x800) {
		if (p_slot->m_glidePos < p_slot->m_glideSpan) {
			frac = (p_slot->m_glidePos << 8) / (p_slot->m_glideSpan >> 8);
			result = frac * (p_slot->m_pitch >> 16) + (0x10000 - frac) * (p_slot->m_prevPitch >> 16);
			p_slot->m_glidePos += g_fadePositionStep;
		}
		else {
			result = p_slot->m_pitch;
		}
	}
	else {
		result = p_slot->m_pitch;
	}

	return result;
}

// Returns a playing voice's rate word after validating its slot, or 0.
// FUNCTION: TONY2 0x00419ce8
TonyU16 __fastcall VoiceGetRate(TonyS32 p_handle)
{
	TonyS32 index;

	p_handle = VoiceHandleOwner(p_handle);

	if (p_handle != -1) {
		index = p_handle & 0xff;

		if (g_seqVoices[index].m_handle == p_handle) {
			if (!(g_seqVoices[index].m_flags & 2)) {
				return g_seqVoices[index].m_levelWord;
			}
		}
	}

	return 0;
}

// Starts a bank sound on a free slot when the id is playable; returns the voice
// handle or -1.
// FUNCTION: TONY2 0x00419d50
TonyS32 __fastcall VoiceStartSound(TonyU8 p_priority, TonyU8 p_sound, TonyU8 p_category)
{
	TonyS32 handle;

	if (p_sound != 0xff) {
		if (SeqIsPlayable(p_sound, p_category)) {
			handle = VoiceRetrigger(p_priority & 0x7f, p_sound, p_category);

			if (handle != -1) {
				return handle;
			}
		}
	}

	return -1;
}

// Retriggers every playing slot of the sound: captures the crossfade level, applies
// the new priority, chains the retriggered slots by owner id under one handle and
// republishes the priority as the channel program.
// FUNCTION: TONY2 0x00419dae
TonyS32 __fastcall VoiceRetrigger(TonyU8 p_priority, TonyU8 p_sound, TonyU8 p_category)
{
	TonyS32 result;
	TonyU32 i;
	TonyS32 owner;

	result = -1;

	for (i = 0; i < g_playingChannelCount; i++) {
		if (g_seqVoices[i].m_sound == p_sound && g_seqVoices[i].m_category == p_category &&
			(g_seqVoices[i].m_flags & 0x10) &&
			(!(g_seqVoices[i].m_flags & 8) || (g_seqVoices[i].m_flags & 0x4000000)) &&
			ChanIsActive(i)) {
			g_seqVoices[i].m_prevPitch = VoiceBlendLevel(&g_seqVoices[i]);
			g_seqVoices[i].m_prevNote = (TonyU8) g_seqVoices[i].m_note;
			g_seqVoices[i].m_note = p_priority;
			g_seqVoices[i].m_glidePos = 0;
			g_seqVoices[i].m_pitch = WaveNoteToStep((TonyU8) g_seqVoices[i].m_note, g_seqVoices[i].m_sampleRate)
										 << 0x10;
			VoiceUnhook(&g_seqVoices[i]);

			if (result == -1) {
				g_seqVoices[i].m_chainNext = -1;
				g_seqVoices[i].m_chainPrev = -1;
				result = VoiceMintHandle(&g_seqVoices[i]);
				owner = g_seqVoices[i].m_handle;
			}
			else {
				g_seqVoices[owner & 0xff].m_chainNext = g_seqVoices[i].m_handle;
				g_seqVoices[i].m_chainPrev = owner;
				owner = g_seqVoices[i].m_handle;
			}
		}
	}

	if (result != -1) {
		SeqSetProgram(p_sound, g_seqVoices[i].m_category, p_priority);
	}

	return result;
}

// Starts a note voice from the packed note key {pool key, program, note}: looks up the
// note's event list in the pool, claims a slot through the start core and fills in the
// slot's playback state. Returns the composed voice handle, minting it into the
// instance list when requested, or -1.
// FUNCTION: TONY2 0x0041a014
TonyS32 __fastcall NoteStart(TonyU32 p_packed, TonyU16 p_rate, TonyU8 p_flags, TonyU8 p_level,
	TonyU8 p_pan, TonyU8 p_category, TonyU8 p_track, TonyU16 p_index, TonyU16 p_volumeTrack, TonyU8 p_mint,
	TonyU8 p_startTrack)
{
	TonyS32 result;
	TonyU8 note;
	TonyU8 program;
	SortedEvent* entry;
	TonyU16 id;

	program = (p_packed >> 8) & 0xff;
	note = p_packed & 0xff;
	id = (TonyU16) (p_packed >> 0x10);
	entry = (SortedEvent*) NotePoolLookup(id);

	if (entry != NULL) {
		result = VoiceClaimSlot(program, note, p_rate, (p_flags & 0x80) != 0);

		if (result != -1) {
			VoiceUnhook(&g_seqVoices[result]);
			g_seqVoices[result].m_flags = (g_seqVoices[result].m_flags & 0x10) | 2;

			if (ChanIsActive(result)) {
				g_seqVoices[result].m_flags |= 1;
			}

			g_seqVoices[result].m_gateTime = 0;

			if (p_flags & 0x80) {
				g_seqVoices[result].m_class = 1;
				p_flags &= 0x7f;
				SeqResetChannel((TonyU8) result, 0xff);
				g_seqVoices[result].m_startSound = (TonyU8) result;
				g_seqVoices[result].m_startCategory = 0xff;
			}
			else {
				g_seqVoices[result].m_class = 0;
				g_seqVoices[result].m_startSound = p_category;
				g_seqVoices[result].m_startCategory = p_track;
			}

			g_seqVoices[result].m_priority = program;
			g_seqVoices[result].m_listId = id;
			g_seqVoices[result].m_groupKey = p_rate;
			g_seqVoices[result].m_age = 0xea600000;
			g_seqVoices[result].m_ageStep = 0x10;
			g_seqVoices[result].m_eventList = entry;
			g_seqVoices[result].m_eventCursor = entry + p_index;
			g_seqVoices[result].m_baseNote = p_flags;
			g_seqVoices[result].m_note = p_flags;
			g_seqVoices[result].m_noteSlew = 0;
			g_seqVoices[result].m_startLevel = p_level;
			g_seqVoices[result].m_startPan = p_pan;
			g_seqVoices[result].m_startVolumeTrack = p_volumeTrack;
			g_seqVoices[result].m_savedList = NULL;
			g_seqVoices[result].m_chainNext = -1;
			g_seqVoices[result].m_chainPrev = -1;
			g_seqVoices[result].m_startTrack = p_startTrack;
			g_seqVoices[result].m_handle = (id << 0x10) | (p_flags << 8) | result;

			if (p_mint) {
				return VoiceMintHandle(&g_seqVoices[result]);
			}

			return g_seqVoices[result].m_handle;
		}
	}

	return -1;
}

// Starts every layer region matching the note and velocity: walks the region table of
// the note's key, retriggers duplicates, scales velocity, pan and pitch per region and
// chains the started voices under the first voice's handle.
// FUNCTION: TONY2 0x0041a344
TonyS32 __fastcall NoteStartLayers(TonyU32 p_packed, TonyU16 p_rate, TonyU8 p_velocity, TonyU8 p_scale,
	TonyU8 p_transpose, TonyU8 p_pan, TonyU8 p_category, TonyU16 p_track, TonyU16 p_volumeTrack,
	TonyU8 p_startTrack)
{
	TonyS32 pan;
	TonyS32 again;
	TonyU8 loud;
	TonyS32 freq;
	TonyS32 fused;
	TonyS32 link;
	TonyU32 forged;
	TonyS32 i;
	TonyS32 waist;
	LayerRegion* areas;
	TonyU16 gotten;
	TonyS32 minted;

	fused = -1;
	areas = (LayerRegion*) RegLookupTagged((TonyU16) (p_packed >> 0x10), &gotten);

	if (areas != NULL) {
		for (i = 0; i < gotten; i++) {
			if (areas[i].m_listId != 0xffff && areas[i].m_velMin <= (p_velocity & 0x7f) &&
				areas[i].m_velMax >= (p_velocity & 0x7f)) {
				waist = (p_velocity & 0x7f) + areas[i].m_noteOffset;
				waist = waist > 0x7f ? 0x7f : (waist < 0 ? 0 : waist);
				again = VoiceStartSound((TonyU8) waist, p_pan, p_category);

				if (again != -1) {
					return again;
				}

				if (!(areas[i].m_pan & 0x80)) {
					pan = areas[i].m_pan - 0x40;
					pan += p_transpose;
					pan = pan < 0 ? 0 : (pan > 0x7f ? 0x7f : pan);
				}
				else {
					pan = 0x80;
				}

				loud = p_scale * areas[i].m_velScale / 0x7f;
				freq = (p_packed >> 8) & 0xff;
				freq += areas[i].m_priorityOffset;
				freq = freq > 0xff ? 0xff : (freq < 0 ? 0 : freq);
				forged = p_packed & 0xff;
				forged |= freq << 8;
				forged |= areas[i].m_listId << 0x10;

				switch (areas[i].m_listId & 0xc000) {
				case 0:
					if (fused == -1) {
						minted = NoteStart(forged, p_rate, waist | (p_velocity & 0x80), loud, pan, p_pan,
							p_category, p_track, p_volumeTrack, 0, p_startTrack);

						if (minted != -1) {
							fused = VoiceMintHandle(&g_seqVoices[minted & 0xff]);
						}
					}
					else {
						link = NoteStart(forged, p_rate, waist | (p_velocity & 0x80), loud, pan, p_pan,
							p_category, p_track, p_volumeTrack, 0, p_startTrack);

						if (link != -1) {
							g_seqVoices[minted & 0xff].m_chainNext = link;
							g_seqVoices[link & 0xff].m_chainPrev = minted;
							minted = link;
						}
					}
					break;
				}
			}
		}
	}

	return fused;
}

// Starts the note by its key class: melodic notes go straight to the dispatcher, drum
// keys resolve their per-note kit cell first and layered keys fan out through the
// region walker. The pitch byte is offset and clamped before dispatch.
// FUNCTION: TONY2 0x0041a694
TonyS32 __fastcall NoteStartByClass(TonyU32 p_packed, TonyU8 p_flags, TonyU8 p_level, TonyU8 p_pan,
	TonyU8 p_category, TonyU8 p_track, TonyU16 p_rate, TonyU16 p_transpose, TonyU8 p_volumeTrack,
	TonyS16 p_noteOffset)
{
	TonyS32 pan;
	DrumKitNote* kit;
	TonyS32 freq;
	TonyS32 dupl;
	TonyS32 cnote;
	TonyU16 tag;
	TonyU8 cell;

	tag = (TonyU16) (p_packed >> 0x10);
	freq = ((p_packed >> 8) & 0xff) + p_noteOffset;
	freq = freq < 0 ? 0 : (freq > 0xff ? 0xff : freq);
	p_packed = (p_packed & 0xffff00ff) | (freq << 8);

	switch (tag & 0xc000) {
	case 0:
		dupl = VoiceStartSound(p_flags, p_category, p_track);

		if (dupl != -1) {
			return dupl;
		}

		return NoteStart(p_packed, (TonyU16) (p_packed >> 0x10), p_flags, p_level, p_pan, p_category,
			p_track, p_rate, p_transpose, 1, p_volumeTrack);
	case 0x4000:
		kit = (DrumKitNote*) RegLookup(tag);

		if (kit != NULL) {
			cell = p_flags & 0x7f;

			if (kit[cell].m_listId != 0xffff) {
				if (!(kit[cell].m_pan & 0x80)) {
					pan = kit[p_flags].m_pan - 0x40;
					pan += p_pan;

					if (pan < 0) {
						p_pan = 0;
					}
					else if (pan > 0x7f) {
						p_pan = 0x7f;
					}
					else {
						p_pan = (TonyU8) pan;
					}
				}
				else {
					p_pan = 0x80;
				}

				cnote = (p_flags & 0x7f) + kit[cell].m_noteOffset;
				cnote = cnote > 0x7f ? 0x7f : (cnote < 0 ? 0 : cnote);
				freq += kit[cell].m_priorityOffset;
				freq = freq > 0xff ? 0xff : (freq < 0 ? 0 : freq);
				p_packed &= 0xff;
				p_packed |= freq << 8;
				p_packed |= kit[cell].m_listId << 0x10;

				if (!(kit[cell].m_listId & 0xc000)) {
					dupl = VoiceStartSound((TonyU8) cnote, p_category, p_track);

					if (dupl != -1) {
						return dupl;
					}

					return NoteStart(p_packed, tag, cnote | (p_flags & 0x80), p_level, p_pan, p_category,
						p_track, p_rate, p_transpose, 1, p_volumeTrack);
				}

				return NoteStartLayers(p_packed, tag, cnote | (p_flags & 0x80), p_level, p_pan, p_category,
					p_track, p_rate, p_transpose, p_volumeTrack);
			}
		}
		break;
	case 0x8000:
		return NoteStartLayers(p_packed, (TonyU16) (p_packed >> 0x10), p_flags, p_level, p_pan, p_category,
			p_track, p_rate, p_transpose, p_volumeTrack);
	}

	return -1;
}

// Plays a sample by its trigger-table entry, defaulting level and pan from the entry.
// FUNCTION: TONY2 0x0041a9ee
TonyS32 __fastcall SamplePlayDefault(TonyS32 p_sound, TonyU8 p_level, TonyU8 p_pan)
{
	TonyS32 result;
	TriggerEntry* entry;

	result = -1;
	entry = TriggerFind(p_sound);

	if (entry != NULL) {
		if (p_level == 0xff) {
			p_level = entry->m_level;
		}

		if (p_pan == 0xff) {
			p_pan = entry->m_pan;
		}

		result = NoteStartByClass(
			(entry->m_listId << 0x10) | (entry->m_priority << 8) | entry->m_polyphony,
			entry->m_note | 0x80,
			p_level,
			p_pan,
			0xff,
			0xff,
			0,
			0xff,
			entry->m_track,
			0
		);
	}

	return result;
}

// Plays a sample; returns its handle or -1.
// FUNCTION: TONY2 0x0041aaa9
TonyS32 __fastcall SamplePlay(TonyS32 p_sound, TonyU8 p_level, TonyU8 p_pan)
{
	TonyS32 result;

	result = -1;

	if (g_mixerActive) {
		MixerLock();
		result = SamplePlayDefault(p_sound, p_level, p_pan);
		MixerUnlock();
	}

	return result;
}
// Validates a play handle: returns it unchanged while any voice under it is still
// alive, -1 otherwise.
// FUNCTION: TONY2 0x0041aaec
TonyS32 __fastcall HandleValidate(TonyS32 p_handle)
{
	if (VoiceHandleOwner(p_handle) != -1) {
		return p_handle;
	}

	return -1;
}


// Sets the level controller on every voice chained under the handle, routing to the
// chain category of note voices or the sample category otherwise.
// FUNCTION: TONY2 0x0041ac55
TonyS32 __fastcall ChainSetLevel(TonyS32 p_handle, TonyU8 p_value)
{
	TonyS32 result;
	TonyS32 slot;

	result = -1;
	p_handle = VoiceHandleOwner(p_handle);

	while (p_handle != -1) {
		slot = p_handle & 0xff;

		if (g_seqVoices[slot].m_handle == p_handle) {
			if (g_seqVoices[slot].m_flags & 2) {
				SeqSetController(0xa, (TonyU8) slot, g_seqVoices[slot].m_startCategory, p_value);
			}
			else {
				SeqSetController(0xa, (TonyU8) slot, g_seqVoices[slot].m_category, p_value);
			}

			p_handle = g_seqVoices[slot].m_chainNext;
			result = 0;
		}
		else {
			return result;
		}
	}

	return result;
}

// Sets a chained voice level, guarded by the mixer lock.
// FUNCTION: TONY2 0x0041ad1c
TonyS32 __fastcall HandleSetLevel(TonyS32 p_handle, TonyU8 p_value)
{
	TonyS32 result;

	result = -1;

	if (g_mixerActive) {
		MixerLock();
		result = ChainSetLevel(p_handle, p_value);
		MixerUnlock();
	}

	return result;
}

// Sets the pan controller on every voice chained under the handle.
// FUNCTION: TONY2 0x0041ad59
TonyS32 __fastcall ChainSetPan(TonyS32 p_handle, TonyU8 p_value)
{
	TonyS32 result;
	TonyS32 slot;

	result = -1;
	p_handle = VoiceHandleOwner(p_handle);

	while (p_handle != -1) {
		slot = p_handle & 0xff;

		if (g_seqVoices[slot].m_handle == p_handle) {
			if (g_seqVoices[slot].m_flags & 2) {
				SeqSetController(0x83, (TonyU8) slot, g_seqVoices[slot].m_startCategory, p_value);
			}
			else {
				SeqSetController(0x83, (TonyU8) slot, g_seqVoices[slot].m_category, p_value);
			}

			p_handle = g_seqVoices[slot].m_chainNext;
			result = 0;
		}
		else {
			return result;
		}
	}

	return result;
}

// Sets a chained voice pan, guarded by the mixer lock.
// FUNCTION: TONY2 0x0041ae20
TonyS32 __fastcall HandleSetPan(TonyS32 p_handle, TonyU8 p_value)
{
	TonyS32 result;

	result = -1;

	if (g_mixerActive) {
		MixerLock();
		result = ChainSetPan(p_handle, p_value);
		MixerUnlock();
	}

	return result;
}

// Sets the rate controller pair on every voice chained under the handle.
// FUNCTION: TONY2 0x0041ae5d
TonyS32 __fastcall ChainSetRate(TonyS32 p_handle, TonyU16 p_value)
{
	TonyS32 result;
	TonyS32 slot;

	result = -1;
	p_handle = VoiceHandleOwner(p_handle);

	while (p_handle != -1) {
		slot = p_handle & 0xff;

		if (g_seqVoices[slot].m_handle == p_handle) {
			if (g_seqVoices[slot].m_flags & 2) {
				SeqSetController(0x84, (TonyU8) slot, g_seqVoices[slot].m_startCategory, p_value >> 7);
				SeqSetController(0x85, (TonyU8) slot, g_seqVoices[slot].m_startCategory, p_value & 0x7f);
			}
			else {
				SeqSetController(0x84, (TonyU8) slot, g_seqVoices[slot].m_category, p_value >> 7);
				SeqSetController(0x85, (TonyU8) slot, g_seqVoices[slot].m_category, p_value & 0x7f);
			}

			p_handle = g_seqVoices[slot].m_chainNext;
			result = 0;
		}
		else {
			return result;
		}
	}

	return result;
}

// Sets a chained voice rate, guarded by the mixer lock.
// FUNCTION: TONY2 0x0041af88
TonyS32 __fastcall HandleSetRate(TonyS32 p_handle, TonyU16 p_value)
{
	TonyS32 result;

	result = -1;

	if (g_mixerActive) {
		MixerLock();
		result = ChainSetRate(p_handle, p_value);
		MixerUnlock();
	}

	return result;
}

// Sets the pitch-bend controller pair on every voice chained under the handle.
// FUNCTION: TONY2 0x0041afc5
TonyS32 __fastcall ChainSetBend(TonyS32 p_handle, TonyU16 p_value)
{
	TonyS32 result;
	TonyS32 slot;

	result = -1;
	p_handle = VoiceHandleOwner(p_handle);

	while (p_handle != -1) {
		slot = p_handle & 0xff;

		if (g_seqVoices[slot].m_handle == p_handle) {
			if (g_seqVoices[slot].m_flags & 2) {
				SeqSetController(0x80, (TonyU8) slot, g_seqVoices[slot].m_startCategory, p_value >> 7);
				SeqSetController(0x81, (TonyU8) slot, g_seqVoices[slot].m_startCategory, p_value & 0x7f);
			}
			else {
				SeqSetController(0x80, (TonyU8) slot, g_seqVoices[slot].m_category, p_value >> 7);
				SeqSetController(0x81, (TonyU8) slot, g_seqVoices[slot].m_category, p_value & 0x7f);
			}

			p_handle = g_seqVoices[slot].m_chainNext;
			result = 0;
		}
		else {
			return result;
		}
	}

	return result;
}

// Sets a chained voice pitch bend, guarded by the mixer lock.
// FUNCTION: TONY2 0x0041b0f0
TonyS32 __fastcall HandleSetBend(TonyS32 p_handle, TonyU16 p_value)
{
	TonyS32 result;

	result = -1;

	if (g_mixerActive) {
		MixerLock();
		result = ChainSetBend(p_handle, p_value);
		MixerUnlock();
	}

	return result;
}

// Sets the modulation controller pair on every voice chained under the handle,
// guarded by the mixer lock.
// FUNCTION: TONY2 0x0041b12d
TonyS32 __fastcall HandleSetWheel(TonyS32 p_handle, TonyU8 p_value)
{
	TonyS32 result;
	TonyS32 slot;

	result = -1;

	if (g_mixerActive) {
		MixerLock();
		p_handle = VoiceHandleOwner(p_handle);

		while (p_handle != -1) {
			slot = p_handle & 0xff;

			if (g_seqVoices[slot].m_handle == p_handle) {
				if (g_seqVoices[slot].m_flags & 2) {
					SeqSetController(1, (TonyU8) slot, g_seqVoices[slot].m_startCategory, p_value);
					SeqSetController(0x21, (TonyU8) slot, g_seqVoices[slot].m_startCategory, 0);
				}
				else {
					SeqSetController(1, (TonyU8) slot, g_seqVoices[slot].m_category, p_value);
					SeqSetController(0x21, (TonyU8) slot, g_seqVoices[slot].m_category, 0);
				}

				p_handle = g_seqVoices[slot].m_chainNext;
				result = 0;
			}
			else {
				MixerUnlock();
				return result;
			}
		}

		MixerUnlock();
	}

	return result;
}

// Sets the volume controller on every voice chained under the handle.
// FUNCTION: TONY2 0x0041b24f
TonyS32 __fastcall ChainSetVolume(TonyS32 p_handle, TonyU8 p_value)
{
	TonyS32 result;
	TonyS32 slot;

	result = -1;
	p_handle = VoiceHandleOwner(p_handle);

	while (p_handle != -1) {
		slot = p_handle & 0xff;

		if (g_seqVoices[slot].m_handle == p_handle) {
			if (g_seqVoices[slot].m_flags & 2) {
				SeqSetController(7, (TonyU8) slot, g_seqVoices[slot].m_startCategory, p_value);
			}
			else {
				SeqSetController(7, (TonyU8) slot, g_seqVoices[slot].m_category, p_value);
			}

			p_handle = g_seqVoices[slot].m_chainNext;
			result = 0;
		}
		else {
			return result;
		}
	}

	return result;
}

// Sets a chained voice volume, guarded by the mixer lock.
// FUNCTION: TONY2 0x0041b316
TonyS32 __fastcall HandleSetVolume(TonyS32 p_handle, TonyU8 p_value)
{
	TonyS32 result;

	result = -1;

	if (g_mixerActive) {
		MixerLock();
		result = ChainSetVolume(p_handle, p_value);
		MixerUnlock();
	}

	return result;
}

// Sets the effects controller on every voice chained under the handle, guarded by the
// mixer lock.
// FUNCTION: TONY2 0x0041b353
TonyS32 __fastcall HandleSetEffects(TonyS32 p_handle, TonyU8 p_value)
{
	TonyS32 result;
	TonyS32 slot;

	result = -1;

	if (g_mixerActive) {
		MixerLock();
		p_handle = VoiceHandleOwner(p_handle);

		while (p_handle != -1) {
			slot = p_handle & 0xff;

			if (g_seqVoices[slot].m_handle == p_handle) {
				if (g_seqVoices[slot].m_flags & 2) {
					SeqSetController(0x5b, (TonyU8) slot, g_seqVoices[slot].m_startCategory, p_value);
				}
				else {
					SeqSetController(0x5b, (TonyU8) slot, g_seqVoices[slot].m_category, p_value);
				}

				p_handle = g_seqVoices[slot].m_chainNext;
				result = 0;
			}
			else {
				MixerUnlock();
				return result;
			}
		}

		MixerUnlock();
	}

	return result;
}

// Marks every voice chained under the handle for release.
// FUNCTION: TONY2 0x0041b439
TonyS32 __fastcall ChainRelease(TonyS32 p_handle)
{
	TonyS32 result;
	TonyS32 slot;

	result = -1;

	if (g_mixerActive) {
		p_handle = VoiceHandleOwner(p_handle);

		while (p_handle != -1) {
			slot = p_handle & 0xff;

			if (g_seqVoices[slot].m_handle == p_handle) {
				g_seqVoices[slot].m_flags |= 8;
				result = 0;
			}

			p_handle = g_seqVoices[slot].m_chainNext;
		}
	}

	return result;
}

// Releases the voices chained under the handle, guarded by the mixer lock.
// FUNCTION: TONY2 0x0041b4c8
TonyS32 __fastcall HandleRelease(TonyS32 p_handle)
{
	TonyS32 result = -1;

	if (g_mixerActive) {
		MixerLock();
		result = ChainRelease(p_handle);
		MixerUnlock();
	}

	return result;
}

// Widens a tick duration to the fine scale.
// FUNCTION: TONY2 0x0041b4ff
void __fastcall TickWiden(TonyU32* p_value)
{
	*p_value = *p_value << 8;
}

// Rescales a tick duration by the record's tempo: ticks * 32000 / tempo, in eighths.
// FUNCTION: TONY2 0x0041b517
void __fastcall TickRescaleTempo(TonyU32* p_value, SeqRecord* p_record)
{
	*p_value = *p_value * 0x7d00 / (TonyU32) SeqGetTempoTickRate(p_record) << 3;
}

// Applies a duration event to the voice: latches the sustain and release-arm flags per
// the event's option bits, then derives the gate time literally, randomized or tempo
// scaled. Returns whether a gate time is armed.
// FUNCTION: TONY2 0x0041b54c
TonyU8 __fastcall CmdDuration(SeqVoice* p_slot, TonyU32* p_event)
{
	if ((p_event[0] >> 8) & 0xff & 1) {
		if (!(p_slot->m_flags & 0x30)) {
			return 0;
		}

		p_slot->m_flags |= 4;
	}
	else {
		p_slot->m_flags &= ~4;
	}

	if ((p_event[0] >> 0x18) & 0xff & 1) {
		if (!(p_slot->m_flags & 0x20) && !ChanIsActive(p_slot->m_handle & 0xff)) {
			return 0;
		}

		p_slot->m_flags |= 0x80000;
	}
	else {
		p_slot->m_flags &= 0xfff7ffff;
	}

	if ((p_event[0] >> 0x10) & 0xff & 1) {
		p_slot->m_gateTime = (TonyS32) (SndRandom() & 0xffff) % (TonyS32) ((p_event[1] >> 0x10) & 0xffff);
	}
	else {
		p_slot->m_gateTime = (p_event[1] >> 0x10) & 0xffff;
	}

	if (p_slot->m_gateTime != 0xffff) {
		if ((p_event[1] >> 8) & 0xff & 1) {
			TickWiden(&p_slot->m_gateTime);
		}
		else {
			TickRescaleTempo(&p_slot->m_gateTime, (SeqRecord*) p_slot);
		}
	}

	return p_slot->m_gateTime != 0;
}

// Arms the fine-duration flag on the event before applying it.
// FUNCTION: TONY2 0x0041b6a4
TonyU8 __fastcall CmdDurationFine(SeqVoice* p_slot, TonyU32* p_event)
{
	((TonyU8*) p_event)[5] = 1;
	return CmdDuration(p_slot, p_event);
}

// Detaches the voice from its event list and parks its handle.
// FUNCTION: TONY2 0x0041b6c6
TonyU8 __fastcall CmdDetach(SeqVoice* p_slot, TonyU32* p_event)
{
	p_slot->m_eventList = NULL;
	p_slot->m_priority = 0;
	VoiceUnhook(p_slot);
	p_slot->m_handle = -1;
	return 1;
}

// Ends or loops the voice's event list: without a loop request or snapshot the voice
// is detached, otherwise the cursor pair rewinds to the loop snapshot.
// FUNCTION: TONY2 0x0041b6fa
TonyU8 __fastcall CmdEndOrLoop(SeqVoice* p_slot, TonyU32* p_event)
{
	if (!((p_event[0] >> 8) & 0xff) || p_slot->m_savedList == NULL) {
		return CmdDetach(p_slot, p_event);
	}

	p_slot->m_eventList = p_slot->m_savedList;
	p_slot->m_eventCursor = p_slot->m_savedCursor;
	return 0;
}

// Jumps the voice's event cursor to another note's list when the voice priority
// reaches the event's threshold.
// FUNCTION: TONY2 0x0041b74b
TonyU8 __fastcall CmdJumpOnPriority(SeqVoice* p_slot, TonyU32* p_event)
{
	SortedEvent* entry;

	if (p_slot->m_note >= (TonyS32) ((p_event[0] >> 8) & 0xff)) {
		entry = (SortedEvent*) NotePoolLookup((TonyU16) (p_event[0] >> 0x10));

		if (entry != NULL) {
			p_slot->m_eventList = entry;
			p_slot->m_eventCursor = entry + *(TonyU16*) (p_event + 1);
		}
	}

	return 0;
}

// Jumps the voice's event cursor to another note's list when the current level
// reaches the event's threshold.
// FUNCTION: TONY2 0x0041b7aa
TonyU8 __fastcall CmdJumpOnLevel(SeqVoice* p_slot, TonyU32* p_event)
{
	SortedEvent* entry;

	if (p_slot->m_level >> 0x10 >= ((p_event[0] >> 8) & 0xff)) {
		entry = (SortedEvent*) NotePoolLookup((TonyU16) (p_event[0] >> 0x10));

		if (entry != NULL) {
			p_slot->m_eventList = entry;
			p_slot->m_eventCursor = entry + *(TonyU16*) (p_event + 1);
		}
	}

	return 0;
}

// Jumps the voice's event cursor to another note's list when the sounding level
// reaches the event's threshold.
// FUNCTION: TONY2 0x0041b809
TonyU8 __fastcall CmdJumpOnSoundLevel(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU8 level;
	SortedEvent* entry;

	if (p_slot->m_sound != 0xff) {
		level = ModEvalVibDepthGroup((SeqRecord*) p_slot) >> 7;

		if (level >= (TonyS32) (p_event[0] >> 8 & 0xff)) {
			entry = (SortedEvent*) NotePoolLookup((TonyU16) (p_event[0] >> 0x10));

			if (entry != NULL) {
				p_slot->m_eventList = entry;
				p_slot->m_eventCursor = entry + *(TonyU16*) (p_event + 1);
			}
		}
	}

	return 0;
}

// Jumps the voice's event cursor to another note's list with the event's probability.
// FUNCTION: TONY2 0x0041b88c
TonyU8 __fastcall CmdJumpRandom(SeqVoice* p_slot, TonyU32* p_event)
{
	SortedEvent* entry;

	if (p_slot->m_sound != 0xff) {
		if (((TonyU16) SndRandom() & 0xff) >= (TonyS32) (p_event[0] >> 8 & 0xff)) {
			entry = (SortedEvent*) NotePoolLookup((TonyU16) (p_event[0] >> 0x10));

			if (entry != NULL) {
				p_slot->m_eventList = entry;
				p_slot->m_eventCursor = entry + *(TonyU16*) (p_event + 1);
			}
		}
	}

	return 0;
}

// Calls into another note's event list, snapshotting the cursor pair for the return.
// FUNCTION: TONY2 0x0041b902
TonyU8 __fastcall CmdCallList(SeqVoice* p_slot, TonyU32* p_event)
{
	SortedEvent* entry;

	entry = (SortedEvent*) NotePoolLookup((TonyU16) (p_event[0] >> 0x10));

	if (entry != NULL) {
		p_slot->m_savedList = p_slot->m_eventList;
		p_slot->m_savedCursor = p_slot->m_eventCursor;
		p_slot->m_eventList = entry;
		p_slot->m_eventCursor = entry + *(TonyU16*) (p_event + 1);
		return 0;
	}

	return CmdDetach(p_slot, p_event);
}

// Attaches the voice's release event list to another note's list.
// FUNCTION: TONY2 0x0041b96b
TonyU8 __fastcall CmdAttachRelease(SeqVoice* p_slot, TonyU32* p_event)
{
	SortedEvent* entry;

	entry = (SortedEvent*) NotePoolLookup((TonyU16) (p_event[0] >> 0x10));

	if (entry != NULL) {
		p_slot->m_releaseList = entry;
		p_slot->m_releaseCursor = entry + *(TonyU16*) (p_event + 1);
	}

	return 0;
}

// Clears the voice's release event list.
// FUNCTION: TONY2 0x0041b9b1
TonyU8 __fastcall CmdClearRelease(SeqVoice* p_slot, TonyU32* p_event)
{
	p_slot->m_releaseCursor = NULL;
	p_slot->m_releaseList = NULL;
	return 0;
}

// Counts down the loop counter and rewinds the cursor within the current list while
// iterations remain: a fresh counter is seeded literally or randomized, 0xffff loops
// forever, and an armed release breaks a breakable loop out early.
// FUNCTION: TONY2 0x0041b9d7
TonyU8 __fastcall CmdLoop(SeqVoice* p_slot, TonyU32* p_event)
{
	if (!p_slot->m_loopCount) {
		if ((p_event[0] >> 0x10) & 0xff & 1) {
			p_slot->m_loopCount = (TonyS32) (SndRandom() & 0xffff) % (TonyS32) ((p_event[1] >> 0x10) & 0xffff);
		}
		else {
			p_slot->m_loopCount = p_event[1] >> 0x10;
		}

		if (p_slot->m_loopCount == 0xffff) {
			goto taken;
		}

		p_slot->m_loopCount++;
	}

	p_slot->m_loopCount--;

	if (p_slot->m_loopCount) {
	taken:
		if ((p_event[0] >> 8) & 0xff & 1 && !(p_slot->m_flags & 0x4000000) && (p_slot->m_flags & 8)) {
			p_slot->m_loopCount = 0;
			return 0;
		}

		p_slot->m_eventCursor = p_slot->m_eventList + *(TonyU16*) (p_event + 1);
	}

	return 0;
}

// Spawns a child note from the event: repacks the note key, offsets the voice's flag
// note, inherits level, pan, rate and category from the voice and records the child's
// handle with a backlink to this voice.
// FUNCTION: TONY2 0x0041bad8
TonyU8 __fastcall CmdSpawnChild(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyS32 note;
	TonyU32 packed;

	packed = ((p_event[0] >> 0x10 & 0xffff) << 0x10) | ((p_event[1] >> 0x10 & 0xff) << 8) |
			 (p_event[1] >> 8 & 0xff);
	note = p_slot->m_baseNote + (TonyS8) (p_event[0] >> 8);
	note = note < 0 ? 0 : (note > 0x7f ? 0x7f : note);

	if (p_slot->m_class) {
		note |= 0x80;
	}

	p_slot->m_chainNext = NoteStart(packed, p_slot->m_groupKey, note, p_slot->m_level >> 0x10,
		p_slot->m_pan >> 0x10, p_slot->m_sound, p_slot->m_category, *(TonyU16*) (p_event + 1),
		p_slot->m_volumeTrack, 0, p_slot->m_track);
	g_seqVoices[p_slot->m_chainNext & 0xff].m_chainPrev = p_slot->m_handle;
	return 0;
}

// Releases every voice whose handle matches the event's note key and flag note.
// FUNCTION: TONY2 0x0041bbf7
TonyU8 __fastcall CmdReleaseMatching(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU32 key;
	TonyU32 i;

	key = p_slot->m_baseNote + ((p_event[0] >> 8 & 0xff) << 8);
	key |= (p_event[0] >> 0x10 & 0xffff) << 0x10;

	for (i = 0; i < g_playingChannelCount; i++) {
		if (g_seqVoices[i].m_handle == (TonyS32) (key | i)) {
			HandleReleaseOne(key | i);
		}
	}

	return 0;
}

// Releases the single voice matching the handle.
// FUNCTION: TONY2 0x0041bc83
TonyS32 __fastcall HandleReleaseOne(TonyS32 p_handle)
{
	TonyS32 slot;

	if (g_mixerActive && p_handle != -1) {
		slot = p_handle & 0xff;

		if (g_seqVoices[slot].m_handle == p_handle) {
			g_seqVoices[slot].m_flags |= 8;
			return 0;
		}
	}

	return -1;
}

// Offsets the voice's age position by the event's signed delta, saturating to the
// 16.16 range.
// FUNCTION: TONY2 0x0041bce9
TonyU8 __fastcall CmdAgeOffset(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyS32 pos;
	TonyS16 slew;

	slew = (TonyS16) (p_event[0] >> 0x10);
	pos = (p_slot->m_age >> 0x10) + slew;

	if (pos < 0) {
		p_slot->m_age = 0;
	}
	else if (pos > 0xffff) {
		p_slot->m_age = 0xffff0000;
	}
	else {
		p_slot->m_age = pos << 0x10;
	}

	return 0;
}

// Sets the voice's age position absolutely.
// FUNCTION: TONY2 0x0041bd4c
TonyU8 __fastcall CmdAgeSet(SeqVoice* p_slot, TonyU32* p_event)
{
	p_slot->m_age = (p_event[0] >> 0x10 & 0xffff) << 0x10;
	return 0;
}

// Derives the age step from the position and the event's span.
// FUNCTION: TONY2 0x0041bd75
TonyU8 __fastcall CmdAgeSlope(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU32 span;

	span = p_event[1];

	if (span != 0) {
		p_slot->m_ageStep = (p_slot->m_age >> 8) / span;
	}
	else {
		p_slot->m_ageStep = 0;
	}

	return 0;
}

// Offsets the voice's program note by the event's signed delta, saturating to a byte.
// FUNCTION: TONY2 0x0041bdb6
TonyU8 __fastcall CmdNoteOffset(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyS16 note;
	TonyS16 slew;

	slew = (TonyS16) (p_event[0] >> 0x10);
	note = p_slot->m_priority + slew;

	if (note < 0) {
		p_slot->m_priority = 0;
	}
	else if (note > 0xff) {
		p_slot->m_priority = 0xff;
	}
	else {
		p_slot->m_priority = (TonyU8) note;
	}

	return 0;
}

// Sets the voice's program note absolutely.
// FUNCTION: TONY2 0x0041be15
TonyU8 __fastcall CmdNoteSet(SeqVoice* p_slot, TonyU32* p_event)
{
	p_slot->m_priority = p_event[0] >> 8;
	return 0;
}

// Stores the event's transpose byte for the addressed track.
// FUNCTION: TONY2 0x0041be35
TonyU8 __fastcall CmdTrackTranspose(SeqVoice* p_slot, TonyU32* p_event)
{
	g_trackTranspose[p_event[0] >> 8 & 0xff] = p_event[0] >> 0x10;
	return 0;
}

// Stores the event's modulation depth pair on the voice.
// FUNCTION: TONY2 0x0041be62
TonyU8 __fastcall CmdModDepth(SeqVoice* p_slot, TonyU32* p_event)
{
	p_slot->m_bendRangeDown = p_event[0] >> 0x10;
	p_slot->m_bendRangeUp = p_event[0] >> 8;
	return 0;
}

// Rescales the crossfade level and reprocesses the event as a plain duration event.
// FUNCTION: TONY2 0x0041be90
TonyU8 __fastcall CmdRescaleFade(SeqVoice* p_slot, TonyU32* p_event, TonyS32 p_level)
{
	p_slot->m_pitch = p_slot->m_pitch << 0x10;
	p_event[0] = 4;
	return CmdDuration(p_slot, p_event);
}

// Derives the per-tick pitch-slew step: the gap to the next semitone up or down,
// scaled by the voice's signed slew percentage.
// FUNCTION: TONY2 0x0041bec5
TonyS32 __fastcall VoiceSlewStep(SeqVoice* p_slot)
{
	TonyS32 result;

	if (p_slot->m_noteSlew) {
		if (p_slot->m_noteSlew > 0) {
			result = ((TonyU16) WaveRateSemitoneUp(p_slot->m_pitch) - (TonyS32) p_slot->m_pitch) *
					 p_slot->m_noteSlew / 0x64;
		}
		else {
			result = ((TonyS32) p_slot->m_pitch - (TonyU16) WaveRateSemitoneDown(p_slot->m_pitch)) *
					 p_slot->m_noteSlew / 0x64;
		}
	}
	else {
		result = 0;
	}

	return result;
}

// Applies a priority-change event: republishes the priority, rebases the crossfade
// level from the priority scale plus the slew step, and reprocesses the event as a
// duration event carrying the previous level.
// FUNCTION: TONY2 0x0041bf66
TonyU8 __fastcall CmdPriority(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU32 level;

	level = p_slot->m_pitch;
	p_slot->m_note = (TonyU8) (p_event[0] >> 8);
	p_slot->m_pitch = WaveNoteToStep((TonyU8) p_slot->m_note, p_slot->m_sampleRate);
	p_slot->m_noteSlew = p_event[0] >> 0x10;
	p_slot->m_pitch += VoiceSlewStep(p_slot);
	return CmdRescaleFade(p_slot, p_event, level);
}

// Applies a relative priority-change event: offsets the priority from itself or from
// the flag note, then rebases the level like the absolute variant.
// FUNCTION: TONY2 0x0041bfea
TonyU8 __fastcall CmdPriorityRel(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU32 level;

	level = p_slot->m_pitch;

	if (!(p_event[0] >> 0x18 & 0xff)) {
		p_slot->m_note += (TonyU8) (p_event[0] >> 8);
	}
	else {
		p_slot->m_note = p_slot->m_baseNote + (p_event[0] >> 8 & 0xff);
	}

	p_slot->m_pitch = WaveNoteToStep((TonyU8) p_slot->m_note, p_slot->m_sampleRate);
	p_slot->m_noteSlew = p_event[0] >> 0x10;
	p_slot->m_pitch += VoiceSlewStep(p_slot);
	return CmdRescaleFade(p_slot, p_event, level);
}

// Applies a priority-change event based from the saved pre-retrigger priority.
// FUNCTION: TONY2 0x0041c0ab
TonyU8 __fastcall CmdPrioritySaved(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU32 level;

	level = p_slot->m_pitch;
	p_slot->m_note = p_slot->m_prevNote + (p_event[0] >> 8 & 0xff);
	p_slot->m_pitch = WaveNoteToStep((TonyU8) p_slot->m_note, p_slot->m_sampleRate);
	p_slot->m_noteSlew = p_event[0] >> 0x10;
	p_slot->m_pitch += VoiceSlewStep(p_slot);
	return CmdRescaleFade(p_slot, p_event, level);
}

// Applies a song-start event: resolves the song descriptor, derives the start offset
// literally or from the remaining level headroom, arms the mixer channel, publishes
// the level base and rate and rebases the crossfade unless the voice is muted.
// FUNCTION: TONY2 0x0041c13d
TonyU8 __fastcall CmdSongStart(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyS32 slot;
	TonyU32 prior;

	if (!SongLookup(p_event[0] >> 8, &g_songDescScratch)) {
		if (!(p_event[0] >> 0x18 & 0xff)) {
			g_songDescScratch.m_startPos = p_event[1];
		}
		else {
			g_songDescScratch.m_startPos = (0x7f - (p_slot->m_level >> 0x10)) * p_event[1] / 0x7f;
		}

		if ((TonyU32) g_songDescScratch.m_startPos >= (TonyU32) g_songDescScratch.m_endPos) {
			g_songDescScratch.m_startPos = g_songDescScratch.m_endPos - 1;
		}

		slot = p_slot->m_handle & 0xff;

		if (!(p_slot->m_flags & 0x200)) {
			ChanArm(slot, &g_songDescScratch, 1);
		}
		else {
			ChanArm(slot, &g_songDescScratch, 0);
		}

		VoiceCaptureFade(p_slot, g_songDescScratch.m_rate);
		p_slot->m_sampleRate = g_songDescScratch.m_rate;
		p_slot->m_sampleData = g_songDescScratch.m_data;
		p_slot->m_flags |= 0x20;

		if (!(p_slot->m_flags & 0x100)) {
			prior = p_slot->m_pitch;
			p_slot->m_pitch = WaveNoteToStep((TonyU8) p_slot->m_note, p_slot->m_sampleRate);
			p_slot->m_pitch += VoiceSlewStep(p_slot);
			return CmdRescaleFade(p_slot, p_event, prior);
		}
	}

	return 0;
}

// Captures the crossfade level base for the voice's saved priority at the level base.
// FUNCTION: TONY2 0x0041c29f
void __fastcall VoiceCaptureFade(SeqVoice* p_slot, TonyS32 p_level)
{
	p_slot->m_prevPitch = WaveNoteToStep(p_slot->m_prevNote, p_level) << 0x10;
}

// Stops and releases the voice's mixer channel.
// FUNCTION: TONY2 0x0041c2d6
TonyU8 __fastcall CmdStopChannel(SeqVoice* p_slot, TonyU32* p_event)
{
	ChanStop(p_slot->m_handle & 0xff, 1);
	return 0;
}

// Marks the voice's channel as detached.
// FUNCTION: TONY2 0x0041c2fb
TonyU8 __fastcall CmdDetachChannel(SeqVoice* p_slot, TonyU32* p_event)
{
	p_slot->m_flags |= 0x80;
	return 0;
}

// Applies a vibrato-setup event: latches the waveform flag, derives the period
// literally or tempo scaled, and arms depth, phase offset and target with a signed
// depth selecting the inverted phase start.
// FUNCTION: TONY2 0x0041c31c
TonyU8 __fastcall CmdVibrato(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU32 period;
	TonyS8 swing;

	if (p_event[0] >> 0x18 & 0xff & 3) {
		p_slot->m_flags |= 0x4000;
	}
	else {
		p_slot->m_flags &= ~0x4000;
	}

	period = p_event[1] >> 0x10 & 0xffff;

	if (p_event[1] >> 8 & 0xff & 1) {
		TickWiden(&period);
	}
	else {
		TickRescaleTempo(&period, (SeqRecord*) p_slot);
	}

	if (period != 0) {
		p_slot->m_flags |= 0x2000;
		p_slot->m_vibPeriod = period;
		swing = (TonyS8) (p_event[0] >> 8);
		p_slot->m_vibSkew = p_event[0] >> 0x10;

		if (swing < 0) {
			p_slot->m_vibDepth = -swing;
			p_slot->m_vibPhase = p_slot->m_vibPeriod >> 1;
		}
		else {
			p_slot->m_vibDepth = swing;
			p_slot->m_vibPhase = 0;
		}
	}
	else {
		p_slot->m_flags &= ~0x2000;
	}

	return 0;
}

// Arms one of the voice's periodic timers with a fine-scaled period, clearing its
// phase when it was already running.
// FUNCTION: TONY2 0x0041c427
TonyU8 __fastcall CmdTimer(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU32 period;
	TonyU8 which;

	which = p_event[0] >> 8;
	period = p_event[0] >> 0x10 & 0xffff;
	TickWiden(&period);

	if (p_slot->m_timers[which].m_period != 0) {
		p_slot->m_timers[which].m_phase = 0;
	}

	p_slot->m_timers[which].m_period = period;
	return 0;
}

// Locks the voice's rate to the event's raw rate.
// FUNCTION: TONY2 0x0041c4a7
TonyU8 __fastcall CmdRateLock(SeqVoice* p_slot, TonyU32* p_event)
{
	p_slot->m_flags |= 0x100;
	p_slot->m_pitch = (TonyU16) WaveRateToStep(p_event[0] >> 8 & 0xffff) << 0x10;
	return 0;
}

// Applies an envelope-attach event: resolves the envelope record, snapshots its four
// stage words and hands them to the mixer channel.
// FUNCTION: TONY2 0x0041c4ed
TonyU8 __fastcall CmdEnvelope(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU16* entry;
	TonyU16 desc[4];

	entry = (TonyU16*) RegLookupAlt(p_event[0] >> 8);

	if (entry != NULL) {
		desc[0] = entry[0];
		desc[1] = entry[1];
		desc[2] = entry[2];
		desc[3] = entry[3];
		ChanLoadEnvelope(p_slot->m_handle & 0xff, desc);
		p_slot->m_flags |= 0x200;
	}

	return 0;
}

// Applies a tremolo/auto-pan setup event for the addressed unit: clears its phase,
// stores depth twice, publishes the converted rate and reprocesses the event as an
// empty duration event.
// FUNCTION: TONY2 0x0041c563
TonyU8 __fastcall CmdTremolo(SeqVoice* p_slot, TonyU32* p_event, TonyS32 p_which)
{
	TonyS32 rate;

	(&p_slot->m_tremoloPhase)[p_which] = 0;
	(&p_slot->m_tremoloReload)[p_which] = p_event[0] >> 8;
	(&p_slot->m_tremoloCount)[p_which] = (&p_slot->m_tremoloReload)[p_which];
	rate = p_event[0] >> 0x10 & 0xffff;
	rate = (TonyU16) WaveRateToStep(rate);
	(&p_slot->m_tremoloStep)[p_which] = rate << 0x10;
	p_event[0] = 0;
	return CmdDuration(p_slot, p_event);
}

// Applies a pan-sweep event: arms the sweep with a fine-scaled span, sets the start
// pan and signed target, and derives the per-tick step.
// FUNCTION: TONY2 0x0041c5f4
TonyU8 __fastcall CmdPanSweep(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyS32 goal;
	TonyU32 span;

	p_slot->m_flags |= 0x8000;
	p_slot->m_panSweepTime = p_event[0] >> 0x10 & 0xffff;
	span = p_slot->m_panSweepTime;
	TickWiden(&p_slot->m_panSweepTime);
	goal = *(TonyS8*) (p_event + 1);
	p_slot->m_pan = (p_event[0] >> 8 & 0xff) << 0x10;
	p_slot->m_panSweepTarget = (goal << 0x10) + p_slot->m_pan;

	if (p_slot->m_panSweepTime != 0) {
		p_slot->m_panStep = (goal << 0x10) / span;
	}
	else {
		p_slot->m_panStep = goal << 0x10;
	}

	return 0;
}

// Applies a priority-relative pan event: scales the gap between the voice priority
// and the event base by the signed factor, rebases it and clamps to the pan range.
// FUNCTION: TONY2 0x0041c6a7
TonyU8 __fastcall CmdPanFromPriority(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyS32 scale;
	TonyS32 offset;

	offset = (p_slot->m_note << 0x10) - ((p_event[0] >> 0x10 & 0xff) << 0x10);
	scale = (TonyS8) (p_event[0] >> 8);
	offset = offset * scale / 0x80;
	offset += (p_event[0] >> 0x18 & 0xff) << 0x10;

	if (offset < 0) {
		offset = 0;
	}
	else if (offset > 0x7f0000) {
		offset = 0x7f0000;
	}

	p_slot->m_pan = offset;
	return 0;
}

// Maps a 16.16 level through the addressed curve table with linear interpolation.
// FUNCTION: TONY2 0x0041c73c
TonyS32 __fastcall LevelCurve(TonyU32 p_level, TonyU8 p_curve)
{
	TonyU8* curve;
	TonyS32 rise;
	TonyU32 spot;
	TonyU32 chunk;

	if (p_curve) {
		curve = (TonyU8*) RegLookupAlt(p_curve);

		if (curve != NULL) {
			spot = p_level >> 0x10;
			chunk = p_level & 0xffff;

			if (spot < 0x7f) {
				rise = (curve[spot + 1] - curve[spot]) * chunk;
				p_level = (curve[spot] << 0x10) + rise;
			}
			else {
				p_level = curve[spot] << 0x10;
			}
		}
	}

	return p_level;
}

// Applies a level-set event: scales the current or base level, rebases it, clamps to
// the level range and maps it through the addressed curve. The curve id's high bits
// are lost to the byte shift, as in the original.
// FUNCTION: TONY2 0x0041c7cc
TonyU8 __fastcall CmdLevelSet(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU8 shape;

	if (!(p_event[1] >> 8 & 0xff)) {
		p_slot->m_level = p_slot->m_level * (p_event[0] >> 8 & 0xff) >> 7;
	}
	else {
		p_slot->m_level = p_slot->m_levelBase * (p_event[0] >> 8 & 0xff) >> 7;
	}

	p_slot->m_level += (p_event[0] >> 0x10 & 0xff) << 0x10;

	if (p_slot->m_level > 0x7f0000) {
		p_slot->m_level = 0x7f0000;
	}

	shape = p_event[0] >> 0x18;
	shape |= *(TonyU8*) (p_event + 1) << 8;
	p_slot->m_level = LevelCurve(p_slot->m_level, shape);
	return 0;
}

// Applies a level-fade event: derives the fade span literally or tempo scaled,
// computes the curved target level like the level-set event and arms the per-tick
// level step toward it.
// FUNCTION: TONY2 0x0041c899
TonyU8 __fastcall CmdLevelFade(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU8 tint;
	TonyU32 settle;
	TonyS32 gotten;

	p_slot->m_levelFadeTime = p_event[1] >> 0x10 & 0xffff;

	if (p_event[1] >> 8 & 0xff & 1) {
		TickWiden(&p_slot->m_levelFadeTime);
	}
	else {
		TickRescaleTempo(&p_slot->m_levelFadeTime, (SeqRecord*) p_slot);
	}

	gotten = TickToWhole(p_slot->m_levelFadeTime);

	if (gotten == 0) {
		gotten = 1;
	}

	settle = p_slot->m_level * (p_event[0] >> 8 & 0xff) >> 7;
	settle += (p_event[0] >> 0x10 & 0xff) << 0x10;

	if (settle > 0x7f0000) {
		settle = 0x7f0000;
	}

	tint = p_event[0] >> 0x18;
	tint |= *(TonyU8*) (p_event + 1) << 8;
	settle = LevelCurve(settle, tint);
	p_slot->m_levelFadeTarget = settle;
	p_slot->m_levelFadeStep = (TonyS32) (settle - p_slot->m_level) / gotten;
	p_slot->m_flags |= 0x10000;
	return 0;
}

// Converts a fine tick duration to whole ticks.
// FUNCTION: TONY2 0x0041c9bd
TonyU32 __fastcall TickToWhole(TonyU32 p_value)
{
	return p_value >> 8;
}

// Applies a randomized priority-change event: derives the priority window literally
// or relative to the current priority, rolls a priority inside it and a slew when
// requested, rewrites the event as an absolute priority change and processes it.
// FUNCTION: TONY2 0x0041c9ce
TonyU8 __fastcall CmdPriorityRandom(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU8 fall;
	TonyU8 ebb;
	TonyU8 seep;
	TonyU8 fade;
	TonyS16 crest;
	TonyS16 foam;

	if (!(p_event[1] >> 8 & 0xff)) {
		ebb = p_event[0] >> 8;
		fade = p_event[0] >> 0x18;

		if (ebb > fade) {
			seep = ebb;
			ebb = fade;
			fade = seep;
		}
	}
	else {
		crest = p_slot->m_note - (p_event[0] >> 8 & 0xff);
		foam = p_slot->m_note + (p_event[0] >> 0x18 & 0xff);
		ebb = crest < 0 ? 0 : (crest > 0x7f ? 0x7f : *(TonyS32*) &crest & 0xff);
		fade = foam < 0 ? 0 : (foam > 0x7f ? 0x7f : *(TonyS32*) &foam & 0xff);
	}

	if (*(TonyU8*) (p_event + 1)) {
		fall = (TonyU16) SndRandom() % 0xc9 - 0x64;
	}
	else {
		fall = p_event[0] >> 0x10;
	}

	p_event[0] = (fall << 0x10 | 0x19) |
				 ((ebb + (TonyU16) SndRandom() % (fade - ebb + 1)) << 8);
	p_event[1] = 0;
	CmdPriority(p_slot, p_event);
	return 0;
}

// Marks the voice sustained.
// FUNCTION: TONY2 0x0041cb75
TonyU8 __fastcall CmdSustain(SeqVoice* p_slot, TonyU32* p_event)
{
	p_slot->m_flags |= 0x20000;
	return 0;
}

// Marks the voice release-armed.
// FUNCTION: TONY2 0x0041cb99
TonyU8 __fastcall CmdReleaseArm(SeqVoice* p_slot, TonyU32* p_event)
{
	p_slot->m_flags |= 0x40000;
	return 0;
}

// Adds a modulation route from the event: remaps the extended controller ids, stores
// the target and converts the percentage scale to 8.8.
// FUNCTION: TONY2 0x0041cbbd
TonyU8 __fastcall ModAddRoute(ModRouteBlock* p_routes, TonyU32* p_event)
{
	TonyU8 cc;
	TonyU8 filled;

	if (p_routes->m_count < 4) {
		filled = p_routes->m_count;
		p_routes->m_count += 1;
		cc = p_event[0] >> 8;

		switch (cc) {
		case 0x80:
			cc = 0x80;
			break;
		case 0x81:
			cc = 0x82;
			break;
		case 0x82:
			cc = 0xa0;
			break;
		case 0x83:
			cc = 0xa1;
			break;
		case 0x84:
			cc = 0x83;
			break;
		case 0x85:
			cc = 0x84;
			break;
		}

		p_routes->m_routes[filled].m_ctrl = cc;
		p_routes->m_routes[filled].m_op = *(TonyU8*) (p_event + 1);
		p_routes->m_routes[filled].m_scale = ((TonyS16) (p_event[0] >> 0x10) << 8) / 0x64;
	}
}

// Adds a volume modulation route, resetting its block on first use.
// FUNCTION: TONY2 0x0041ccaf
TonyU8 __fastcall ModAddVolumeRoute(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU8 fresh;

	fresh = *(TonyU8*) (p_event + 1);

	if (!(p_slot->m_flags & 0x100000) || !fresh) {
		p_slot->m_flags |= 0x100000;
		p_slot->m_modBlocks[0].m_count = 0;
	}

	ModAddRoute(&p_slot->m_modBlocks[0], p_event);
	return 0;
}

// Adds a pan modulation route, resetting its block on first use.
// FUNCTION: TONY2 0x0041cd12
TonyU8 __fastcall ModAddPanRoute(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU8 fresh;

	fresh = *(TonyU8*) (p_event + 1);

	if (!(p_slot->m_flags & 0x200000) || !fresh) {
		p_slot->m_flags |= 0x200000;
		p_slot->m_modBlocks[1].m_count = 0;
	}

	ModAddRoute(&p_slot->m_modBlocks[1], p_event);
	return 0;
}

// Adds a rate modulation route, resetting its block on first use.
// FUNCTION: TONY2 0x0041cd75
TonyU8 __fastcall ModAddRateRoute(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU8 fresh;

	fresh = *(TonyU8*) (p_event + 1);

	if (!(p_slot->m_flags & 0x400000) || !fresh) {
		p_slot->m_flags |= 0x400000;
		p_slot->m_modBlocks[3].m_count = 0;
	}

	ModAddRoute(&p_slot->m_modBlocks[3], p_event);
	return 0;
}

// Adds a effects modulation route, resetting its block on first use.
// FUNCTION: TONY2 0x0041cdd8
TonyU8 __fastcall ModAddEffectsRoute(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU8 fresh;

	fresh = *(TonyU8*) (p_event + 1);

	if (!(p_slot->m_flags & 0x800000) || !fresh) {
		p_slot->m_flags |= 0x800000;
		p_slot->m_modBlocks[5].m_count = 0;
	}

	ModAddRoute(&p_slot->m_modBlocks[5], p_event);
	return 0;
}

// Adds a vibrato depth modulation route, resetting its block on first use.
// FUNCTION: TONY2 0x0041ce3b
TonyU8 __fastcall ModAddVibDepthRoute(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU8 fresh;

	fresh = *(TonyU8*) (p_event + 1);

	if (!(p_slot->m_flags & 0x2000000) || !fresh) {
		p_slot->m_flags |= 0x2000000;
		p_slot->m_modBlocks[6].m_count = 0;
	}

	ModAddRoute(&p_slot->m_modBlocks[6], p_event);
	return 0;
}

// Adds a vibrato rate modulation route, resetting its block on first use.
// FUNCTION: TONY2 0x0041ce9e
TonyU8 __fastcall ModAddVibRateRoute(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU8 fresh;

	fresh = *(TonyU8*) (p_event + 1);

	if (!(p_slot->m_flags & 0x2000000) || !fresh) {
		p_slot->m_flags |= 0x2000000;
		p_slot->m_modBlocks[7].m_count = 0;
	}

	ModAddRoute(&p_slot->m_modBlocks[7], p_event);
	return 0;
}

// Adds a modulation modulation route, resetting its block on first use.
// FUNCTION: TONY2 0x0041cf01
TonyU8 __fastcall ModAddWheelRoute(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU8 fresh;

	fresh = *(TonyU8*) (p_event + 1);

	if (!(p_slot->m_flags & 0x1000000) || !fresh) {
		p_slot->m_flags |= 0x1000000;
		p_slot->m_modBlocks[8].m_count = 0;
	}

	ModAddRoute(&p_slot->m_modBlocks[8], p_event);
	return 0;
}

// Restarts the crossfade for a retriggered voice: seeds the ramp from the captured
// level unless muted, rebases the target from the saved priority and republishes the
// sound as playing.
// FUNCTION: TONY2 0x0041cf64
void __fastcall VoiceRestartFade(SeqVoice* p_slot)
{
	if (p_slot->m_muteMode != 1) {
		p_slot->m_glidePos = p_slot->m_glideSpan;
	}
	else {
		p_slot->m_glidePos = 0;
	}

	p_slot->m_prevPitch = WaveNoteToStep(p_slot->m_prevNote, p_slot->m_sampleRate) << 0x10;

	if (p_slot->m_sound != 0xff) {
		SeqSetPlayable(p_slot->m_sound, p_slot->m_category, 1);
	}
}

// Applies a crossfade-mode event: stores the mute mode and the fade span, then stops
// or restarts the crossfade ramp per the requested direction.
// FUNCTION: TONY2 0x0041cfec
TonyU8 __fastcall CmdFadeMode(SeqVoice* p_slot, TonyU32* p_event)
{
	TonyU32 span;

	p_slot->m_muteMode = p_event[0] >> 0x10;
	span = p_event[1] >> 0x10 & 0xffff;

	if (p_event[1] >> 8 & 0xff & 1) {
		TickWiden(&span);
	}
	else {
		TickRescaleTempo(&span, (SeqRecord*) p_slot);
	}

	p_slot->m_glideSpan = span;

	switch ((TonyU8) (p_event[0] >> 8)) {
	case 0:
		p_slot->m_flags &= ~0x800;
		p_slot->m_flags |= 0x1000;

		if (p_slot->m_sound != 0xff) {
			SeqSetPlayable(p_slot->m_sound, p_slot->m_category, 0);
		}
		break;
	case 1:
		if (!(p_slot->m_flags & 0x800)) {
			VoiceRestartFade(p_slot);
		}

		p_slot->m_flags = 0x1800;
		break;
	}

	return 0;
}

// Runs one sequencer frame: derives the tick step from the tick length and tempo
// divider, then pumps the voices.
// FUNCTION: TONY2 0x0041d0d4
void SeqRunFrame(void)
{
	if (g_dsndBlockFrames != 0) {
		g_fadePositionStep = (TonyS32) g_dsndBlockFrames * 0x7d00 / g_speechSaved2 << 3;
		SeqPumpVoices();
	}
}

// Pumps every sequencer voice for one tick.
// The per-tick sequencer service: for every active song slot runs the event pump,
// steps the two LFOs and the vibrato oscillator, ticks the two controller fades and
// the tempo/volume/echo ramps, evaluates glide, dispatches the deferred flag
// services, then recomputes the slot's pitch step and the three send levels and
// hands them to the voice router; finally ticks the 0x20 track-group fade pairs.
// FUNCTION: TONY2 0x0041d101
void SeqPumpVoices(void)
{
	TonyS32 fall;
	TonyS32 ebb;
	TonyU32 rise;
	TonyU8 swell;
	TonyS32 seep;
	TonyS32 mire;
	TonyU32 lull;
	TonyS32 fade;
	TonyS32 crest;
	TonyU32 surge;
	TonyS32 mist;
	TonyS32 gain;
	TonyU32 foam;
	TonyS32 drift;
	TonyS32 tide;
	TonyS32 brine;

	for (mire = 0; mire < g_playingChannelCount; mire++) {
		if (g_seqVoices[mire].m_eventList) {
			SeqTickVoice(&g_seqVoices[mire]);
		}

		if (!g_seqVoices[mire].m_reserved) {
			for (fade = 0; fade < 2; fade++) {
				if (g_seqVoices[mire].m_timers[fade].m_period) {
					g_seqVoices[mire].m_timers[fade].m_phase += g_fadePositionStep;
					g_seqVoices[mire].m_timers[fade].m_value = SndSineQ12(
						((TonyU32) g_seqVoices[mire].m_timers[fade].m_phase %
							g_seqVoices[mire].m_timers[fade].m_period
							<< 4) /
						((TonyU32) g_seqVoices[mire].m_timers[fade].m_period >> 8));
				}
			}

			if (g_seqVoices[mire].m_flags & 0x2000) {
				g_seqVoices[mire].m_vibPhase += g_fadePositionStep;
				g_seqVoices[mire].m_vibValue = (TonyS16) SndSineQ12(
					(g_seqVoices[mire].m_vibPhase % g_seqVoices[mire].m_vibPeriod << 4) /
					(g_seqVoices[mire].m_vibPeriod >> 8));
			}

			for (fade = 0; fade < 2; fade++) {
				if ((&g_seqVoices[mire].m_tremoloReload)[fade]) {
					(&g_seqVoices[mire].m_tremoloCount)[fade] -= 1;

					if ((&g_seqVoices[mire].m_tremoloCount)[fade] == 0) {
						(&g_seqVoices[mire].m_tremoloCount)[fade] = (&g_seqVoices[mire].m_tremoloReload)[fade];
						(&g_seqVoices[mire].m_tremoloPhase)[fade] = 0;
					}
					else {
						(&g_seqVoices[mire].m_tremoloPhase)[fade] += (&g_seqVoices[mire].m_tremoloStep)[fade];
					}
				}
			}

			if (g_seqVoices[mire].m_sound != 0xff) {
				if (ModEvalVibRateGroup((SeqRecord*) &g_seqVoices[mire]) <= 0x1f80) {
					g_seqVoices[mire].m_flags &= 0xfbffffff;
				}
				else {
					g_seqVoices[mire].m_flags |= 0x4000000;
				}
			}
			else {
				g_seqVoices[mire].m_flags &= 0xfbffffff;
			}

			if (!(g_seqVoices[mire].m_flags & 0x1000) && !(g_seqVoices[mire].m_flags & 0x10) &&
				g_seqVoices[mire].m_sound != 0xff) {
				if (ModEvalWheelGroup((SeqRecord*) &g_seqVoices[mire]) <= 0x1f80) {
					g_seqVoices[mire].m_flags &= 0xfffff7ff;
					SeqSetPlayable(g_seqVoices[mire].m_sound, g_seqVoices[mire].m_category, 0);
				}
				else {
					if (!(g_seqVoices[mire].m_flags & 0x800)) {
						VoiceRestartFade(&g_seqVoices[mire]);
					}

					g_seqVoices[mire].m_flags |= 0x800;
				}
			}

			if (g_seqVoices[mire].m_flags & 0x20) {
				g_seqVoices[mire].m_flags &= 0xffffffdf;
				g_seqVoices[mire].m_flags |= 0x10;
				ChanArmFadeIn(mire);
			}

			if ((g_seqVoices[mire].m_flags & 0x80) && !(g_seqVoices[mire].m_flags & 0x4000000)) {
				g_seqVoices[mire].m_flags &= 0xffffff6f;
				ChanClearAck(mire);
			}

			if (g_seqVoices[mire].m_flags & 0x40000) {
				g_seqVoices[mire].m_flags &= 0xfffbffff;
				ChanDriverHook(mire);
			}

			if (g_seqVoices[mire].m_age != 0) {
				g_seqVoices[mire].m_age -= g_seqVoices[mire].m_ageStep * g_fadePositionStep;

				if (g_seqVoices[mire].m_age < 0) {
					g_seqVoices[mire].m_age = 0;
				}
			}

			if (g_seqVoices[mire].m_flags & 0x8000) {
				g_seqVoices[mire].m_pan =
					g_seqVoices[mire].m_panSweepTarget -
					(g_seqVoices[mire].m_panSweepTime >> 8) * g_seqVoices[mire].m_panStep;

				if ((TonyS32) g_seqVoices[mire].m_pan < 0) {
					g_seqVoices[mire].m_pan = 0;
				}

				if (g_seqVoices[mire].m_pan > 0x7f0000) {
					g_seqVoices[mire].m_pan = 0x7f0000;
				}

				g_seqVoices[mire].m_panSweepTime -= g_fadePositionStep;

				if (g_seqVoices[mire].m_panSweepTime <= 0) {
					g_seqVoices[mire].m_flags &= 0xffff7fff;
					g_seqVoices[mire].m_pan = g_seqVoices[mire].m_panSweepTarget;
				}
			}

			foam = VoiceBlendLevel(&g_seqVoices[mire]);

			if (g_seqVoices[mire].m_flags & 0x20010) {
				if (g_seqVoices[mire].m_sound == 0xff) {
					goto vibrato;
				}

				gain = ModEvalRateGroup((SeqRecord*) &g_seqVoices[mire]);
				g_seqVoices[mire].m_pitchMod = (TonyU16) gain;
			}
			else {
				gain = g_seqVoices[mire].m_pitchMod;
			}

			if (gain != 0x2000) {
				gain -= 0x2000;

				if (gain < 0) {
					gain = -gain * g_seqVoices[mire].m_bendRangeDown;
					surge = gain & 0x7f;
					gain >>= 7;
					WaveRateFinePitchDownPair((TonyU16) gain, (TonyS32*) &rise, (TonyS32*) &lull, foam >> 0x10);
					foam = rise - ((rise - lull) * surge >> 7);
				}
				else {
					gain = gain * g_seqVoices[mire].m_bendRangeUp;
					surge = gain & 0x7f;
					gain >>= 7;
					WaveRateFinePitchUpPair((TonyU16) gain, (TonyS32*) &rise, (TonyS32*) &lull, foam >> 0x10);
					foam = rise + ((lull - rise) * surge >> 7);
				}

				if ((TonyS32) foam < 0) {
					foam = 0;
				}
				else if (foam > 0xffff) {
					foam = 0xffff0000;
				}
				else {
					foam <<= 0x10;
				}
			}

vibrato:
			if (g_seqVoices[mire].m_flags & 0x2000) {
				if (g_seqVoices[mire].m_vibValue < 0) {
					drift = WaveRatePitchDown((TonyU16) (foam >> 0x10), g_seqVoices[mire].m_vibDepth) & 0xffff;

					if (g_seqVoices[mire].m_vibSkew) {
						drift -= (drift - (WaveRateSemitoneDown((TonyU16) drift) & 0xffff)) *
								 g_seqVoices[mire].m_vibSkew / 100;
					}

					brine = g_seqVoices[mire].m_vibValue * ((TonyS32) (foam >> 0x10) - drift) >> 0xc;
				}
				else {
					drift = WaveRatePitchUp((TonyU16) (foam >> 0x10), g_seqVoices[mire].m_vibDepth) & 0xffff;

					if (g_seqVoices[mire].m_vibSkew) {
						drift += ((WaveRateSemitoneUp((TonyU16) drift) & 0xffff) - drift) *
								 g_seqVoices[mire].m_vibSkew / 100;
					}

					brine = g_seqVoices[mire].m_vibValue * (drift - (TonyS32) (foam >> 0x10)) >> 0xc;
				}

				if ((g_seqVoices[mire].m_flags & 0x4000) && g_seqVoices[mire].m_sound != 0xff) {
					ebb = ModEvalVibDepthGroup((SeqRecord*) &g_seqVoices[mire]) >> 7;
				}
				else {
					ebb = 0x40;
				}

				tide = (foam >> 0x10) + (brine >> 6) * ebb;

				if (tide > 0x7fff) {
					foam = 0x7fff0000;
				}
				else if (tide >= 0) {
					foam += (brine << 0xa) * ebb;
				}
				else {
					foam = 0;
				}
			}

			foam += g_seqVoices[mire].m_tremoloPhase + g_seqVoices[mire].m_autoPanPhase;

			if (g_seqVoices[mire].m_sound != 0xff) {
				foam = (foam >> 0x10) * ModEvalEffectsGroup((SeqRecord*) &g_seqVoices[mire]) >> 0xd;
			}
			else {
				foam = foam >> 0x10;
			}

			ChanSetPitchStep(mire, foam);

			if (g_seqVoices[mire].m_flags & 0x10000) {
				g_seqVoices[mire].m_level =
					g_seqVoices[mire].m_levelFadeTarget -
					(g_seqVoices[mire].m_levelFadeTime >> 8) * g_seqVoices[mire].m_levelFadeStep;

				if ((TonyS32) g_seqVoices[mire].m_level < 0) {
					g_seqVoices[mire].m_level = 0;
				}

				if ((TonyS32) g_seqVoices[mire].m_level > 0x7f0000) {
					g_seqVoices[mire].m_level = 0x7f0000;
				}

				g_seqVoices[mire].m_levelFadeTime -= g_fadePositionStep;

				if ((TonyS32) g_seqVoices[mire].m_levelFadeTime <= 0) {
					g_seqVoices[mire].m_flags &= 0xfffeffff;
					g_seqVoices[mire].m_level = g_seqVoices[mire].m_levelFadeTarget;
				}
			}

			fall = g_seqVoices[mire].m_pan;
			swell = g_seqVoices[mire].m_sound;
			mist = g_seqVoices[mire].m_level;
			mist = (mist >> 7) * (g_trackFades[g_seqVoices[mire].m_track].m_groupLevel >> 0x10);
			mist = (mist >> 7) * (g_trackFades[g_seqVoices[mire].m_track].m_level >> 0x10);
			mist = (mist >> 7) *
				   (g_trackFades[(g_seqVoices[mire].m_class != 0) + 0x15].m_level >> 0x10);

			if (g_seqVoices[mire].m_volumeTrack != 0xff) {
				mist = (mist >> 7) * g_trackVolumes[g_seqVoices[mire].m_volumeTrack];
			}

			if (swell != 0xff) {
				mist = (mist >> 7) * (ModEvalRoutes((SeqRecord*) &g_seqVoices[mire]) >> 7);

				if (fall != 0x800000) {
					fall += ((ModEvalVolumeGroup((SeqRecord*) &g_seqVoices[mire]) >> 7) - 0x40) << 0x10;
					fall = fall < 0 ? 0 : (fall > 0x7f0000 ? 0x7f0000 : fall);
				}

				crest = ModEvalPanGroup((SeqRecord*) &g_seqVoices[mire]) << 9;

				if (crest > 0x7f0000) {
					crest = 0x7f0000;
				}
			}
			else {
				crest = 0;
			}

			if (g_seqServiceArmed) {
				fall = 0x400000;
				crest = 0;
			}

			if (swell != 0xff) {
				seep = (ModEvalBendGroup((SeqRecord*) &g_seqVoices[mire]) >> 7) * mist >> 7;

				if (seep > 0x7f0000) {
					seep = 0x7f0000;
				}
			}
			else {
				seep = 0;
			}

			g_seqVoices[mire].m_levelWord = (TonyU16) (mist >> 8);
			ChanSetLevels(mire, mist, fall, crest, seep);
		}
	}

	for (mire = 0; mire < 0x20; mire++) {
		if (g_trackFades[mire].m_time != 0) {
			g_trackFades[mire].m_level =
				g_trackFades[mire].m_target -
				(g_trackFades[mire].m_time >> 8) * g_trackFades[mire].m_step;

			if (g_trackFades[mire].m_level < 0) {
				g_trackFades[mire].m_level = 0;
			}

			g_trackFades[mire].m_time -= g_fadePositionStep;

			if (g_trackFades[mire].m_time <= 0) {
				g_trackFades[mire].m_level = g_trackFades[mire].m_target;
				g_trackFades[mire].m_time = 0;

				switch (g_trackFades[mire].m_command) {
				case 1:
					ChannelStop(g_trackFades[mire].m_handle);
					break;
				case 2:
					ChannelResume(g_trackFades[mire].m_handle);
					break;
				case 3:
					ChannelSetPitchSlide(g_trackFades[mire].m_handle, 0, 0);
					break;
				}
			}
		}

		if (g_trackFades[mire].m_groupTime != 0) {
			g_trackFades[mire].m_groupLevel =
				g_trackFades[mire].m_groupTarget -
				(g_trackFades[mire].m_groupTime >> 8) * g_trackFades[mire].m_groupStep;

			if (g_trackFades[mire].m_groupLevel < 0) {
				g_trackFades[mire].m_groupLevel = 0;
			}

			g_trackFades[mire].m_groupTime -= g_fadePositionStep;

			if (g_trackFades[mire].m_groupTime <= 0) {
				g_trackFades[mire].m_groupLevel = g_trackFades[mire].m_groupTarget;
				g_trackFades[mire].m_groupTime = 0;
			}
		}
	}
}

// Advances one voice's playback state for a tick.
// The event pair the tick driver publishes for the command handlers.
// GLOBAL: TONY2 0x004e61e8
TonyU32 g_seqTickEventLo;

// GLOBAL: TONY2 0x004e61ec
TonyU32 g_seqTickEventHi;

// The sequencer tick driver: handles pending restart/reset requests, counts the
// slot's wait down by the tick step, then pumps events from the stream, dispatching
// each command to its handler until one of them stalls the slot.
// FUNCTION: TONY2 0x0041e1cc
void __fastcall SeqTickVoice(SeqVoice* p_slot)
{
	TonyU8 ok;

	if (p_slot->m_flags & 3) {
		if (p_slot->m_flags & 1) {
			p_slot->m_flags &= 0xfffffffe;
			ChanRelease(p_slot->m_handle & 0xff);
		}

		p_slot->m_gateTime -= g_fadePositionStep;

		if ((TonyS32) p_slot->m_gateTime <= 0) {
			p_slot->m_gateTime = 0;

			if (!(p_slot->m_flags & 0x40)) {
				ModLoadDefaults((SeqRecord*) p_slot);
				p_slot->m_pan = (TonyU32) p_slot->m_startPan << 0x10;
				p_slot->m_level = (TonyU32) p_slot->m_startLevel << 0x10;
				p_slot->m_levelBase = p_slot->m_level;
				p_slot->m_sound = p_slot->m_startSound;
				p_slot->m_category = p_slot->m_startCategory;
				p_slot->m_volumeTrack = p_slot->m_startVolumeTrack;

				if (SeqGetProgram(p_slot->m_sound, p_slot->m_category) != 0xff) {
					p_slot->m_prevNote = SeqGetProgram(p_slot->m_sound, p_slot->m_category);
				}
				else {
					p_slot->m_prevNote = p_slot->m_baseNote;
				}

				SeqSetProgram(p_slot->m_sound, p_slot->m_category, p_slot->m_baseNote);
				p_slot->m_track = p_slot->m_startTrack;
				p_slot->m_glidePos = 0;
				p_slot->m_glideSpan = 0x1f4;
				p_slot->m_muteMode = 0;
				p_slot->m_bendRangeDown = 2;
				p_slot->m_bendRangeUp = 2;
				p_slot->m_loopCount = 0;
				p_slot->m_tremoloReload = 0;
				p_slot->m_autoPanReload = 0;
				p_slot->m_tremoloPhase = 0;
				p_slot->m_autoPanPhase = 0;
				p_slot->m_timers[0].m_period = 0;
				p_slot->m_timers[0].m_value = 0;
				p_slot->m_timers[1].m_period = 0;
				p_slot->m_timers[1].m_value = 0;
				p_slot->m_releaseList = 0;
				p_slot->m_releaseCursor = 0;
				p_slot->m_sampleRate = 0x4000ac44;
				p_slot->m_pitchMod = 0x2000;
				p_slot->m_levelWord = 0;
				p_slot->m_flags &= 8;
				goto pump;
			}

			p_slot->m_priority = 0;
			p_slot->m_eventList = NULL;
			p_slot->m_flags = 0;
		}

		return;
	}

pump:
	{
		if (p_slot->m_releaseCursor != 0 && !(p_slot->m_flags & 0x4000000) && (p_slot->m_flags & 8)) {
			p_slot->m_eventCursor = p_slot->m_releaseCursor;
			p_slot->m_eventList = p_slot->m_releaseList;
			p_slot->m_gateTime = 0;
			p_slot->m_releaseCursor = NULL;
			p_slot->m_releaseList = NULL;
		}

		if (p_slot->m_gateTime == 0) {
			do {
				g_seqTickEventLo = p_slot->m_eventCursor->m_value;
				g_seqTickEventHi = *(TonyU32*) &p_slot->m_eventCursor->m_key;
				p_slot->m_eventCursor++;

				switch (g_seqTickEventLo & 0x7f) {
			case 0x0:
				ok = CmdDetach(p_slot, &g_seqTickEventLo);
				break;
			case 0x1:
				ok = CmdEndOrLoop(p_slot, &g_seqTickEventLo);
				break;
			case 0x2:
				ok = CmdJumpOnPriority(p_slot, &g_seqTickEventLo);
				break;
			case 0x3:
				ok = CmdJumpOnLevel(p_slot, &g_seqTickEventLo);
				break;
			case 0x4:
				ok = CmdDuration(p_slot, &g_seqTickEventLo);
				break;
			case 0x5:
				ok = CmdLoop(p_slot, &g_seqTickEventLo);
				break;
			case 0x6:
				ok = CmdCallList(p_slot, &g_seqTickEventLo);
				break;
			case 0x7:
				ok = CmdDurationFine(p_slot, &g_seqTickEventLo);
				break;
			case 0x8:
				ok = CmdSpawnChild(p_slot, &g_seqTickEventLo);
				break;
			case 0x9:
				ok = CmdReleaseMatching(p_slot, &g_seqTickEventLo);
				break;
			case 0xa:
				ok = CmdJumpOnSoundLevel(p_slot, &g_seqTickEventLo);
				break;
			case 0xb:
				ok = CmdPanFromPriority(p_slot, &g_seqTickEventLo);
				break;
			case 0xc:
				ok = CmdEnvelope(p_slot, &g_seqTickEventLo);
				break;
			case 0xd:
				ok = CmdLevelSet(p_slot, &g_seqTickEventLo);
				break;
			case 0xe:
				ok = CmdPanSweep(p_slot, &g_seqTickEventLo);
				break;
			case 0xf:
				ok = CmdLevelFade(p_slot, &g_seqTickEventLo);
				break;
			case 0x10:
				ok = CmdSongStart(p_slot, &g_seqTickEventLo);
				break;
			case 0x11:
				ok = CmdStopChannel(p_slot, &g_seqTickEventLo);
				break;
			case 0x12:
				ok = CmdDetachChannel(p_slot, &g_seqTickEventLo);
				break;
			case 0x13:
				ok = CmdJumpRandom(p_slot, &g_seqTickEventLo);
				break;
			case 0x17:
				ok = CmdPriorityRandom(p_slot, &g_seqTickEventLo);
				break;
			case 0x18:
				ok = CmdPriorityRel(p_slot, &g_seqTickEventLo);
				break;
			case 0x19:
				ok = CmdPriority(p_slot, &g_seqTickEventLo);
				break;
			case 0x1a:
				ok = CmdPrioritySaved(p_slot, &g_seqTickEventLo);
				break;
			case 0x1b:
				ok = CmdFadeMode(p_slot, &g_seqTickEventLo);
				break;
			case 0x1c:
				ok = CmdVibrato(p_slot, &g_seqTickEventLo);
				break;
			case 0x1d:
				ok = CmdTremolo(p_slot, &g_seqTickEventLo, 0);
				break;
			case 0x1e:
				ok = CmdTremolo(p_slot, &g_seqTickEventLo, 1);
				break;
			case 0x1f:
				ok = CmdRateLock(p_slot, &g_seqTickEventLo);
				break;
			case 0x28:
				ok = CmdAttachRelease(p_slot, &g_seqTickEventLo);
				break;
			case 0x29:
				ok = CmdClearRelease(p_slot, &g_seqTickEventLo);
				break;
			case 0x30:
				ok = CmdAgeOffset(p_slot, &g_seqTickEventLo);
				break;
			case 0x31:
				ok = CmdAgeSet(p_slot, &g_seqTickEventLo);
				break;
			case 0x32:
				ok = CmdTrackTranspose(p_slot, &g_seqTickEventLo);
				break;
			case 0x33:
				ok = CmdModDepth(p_slot, &g_seqTickEventLo);
				break;
			case 0x34:
				ok = CmdReleaseArm(p_slot, &g_seqTickEventLo);
				break;
			case 0x35:
				ok = CmdSustain(p_slot, &g_seqTickEventLo);
				break;
			case 0x36:
				ok = CmdNoteSet(p_slot, &g_seqTickEventLo);
				break;
			case 0x37:
				ok = CmdNoteOffset(p_slot, &g_seqTickEventLo);
				break;
			case 0x38:
				ok = CmdAgeSlope(p_slot, &g_seqTickEventLo);
				break;
			case 0x40:
				ok = ModAddVolumeRoute(p_slot, &g_seqTickEventLo);
				break;
			case 0x41:
				ok = ModAddPanRoute(p_slot, &g_seqTickEventLo);
				break;
			case 0x42:
				ok = ModAddRateRoute(p_slot, &g_seqTickEventLo);
				break;
			case 0x43:
				ok = ModAddEffectsRoute(p_slot, &g_seqTickEventLo);
				break;
			case 0x44:
				ok = ModAddVibDepthRoute(p_slot, &g_seqTickEventLo);
				break;
			case 0x45:
				ok = ModAddVibRateRoute(p_slot, &g_seqTickEventLo);
				break;
			case 0x46:
				ok = ModAddWheelRoute(p_slot, &g_seqTickEventLo);
				break;
			case 0x50:
				ok = CmdTimer(p_slot, &g_seqTickEventLo);
				break;
				default:
					ok = 0;
					break;
				}
			} while (ok == 0);
		}
		else {
			if ((p_slot->m_flags & 4) && !(p_slot->m_flags & 0x4000000) && (p_slot->m_flags & 8)) {
				p_slot->m_flags &= 0xfffffffb;
				p_slot->m_gateTime = 0;
			}

			if (p_slot->m_flags & 0x80000) {
				if (ChanIsActive(p_slot->m_handle & 0xff) == 0) {
					p_slot->m_flags &= 0xfff7ffff;
					p_slot->m_gateTime = 0;
				}
			}

			if (p_slot->m_gateTime != 0xffff) {
				p_slot->m_gateTime -= g_fadePositionStep;

				if ((TonyS32) p_slot->m_gateTime < 0) {
					p_slot->m_gateTime = 0;
				}
			}
		}
	}
}

// Stop-everything entry.
// FUNCTION: TONY2 0x0041ea95
void SndStopAll(void)
{
	if (g_mixerActive) {
		MixerLock();
		ChannelStopAll();
		StreamReleaseAll();
		SeqStopVoices(0);
		MixerUnlock();
	}
}

// Resets an SFX instance slot after its voice is unhooked.
// FUNCTION: TONY2 0x0041eac0
void __fastcall VoiceResetSlot(struct SeqVoice* p_slot)
{
	VoiceUnhook(p_slot);
	p_slot->m_handle = -1;
	p_slot->m_eventList = 0;
	p_slot->m_flags &= ~3;
	p_slot->m_priority = 0;
	p_slot->m_age = 0;
}

// Releases every instance chained under p_handle; returns 0 when any was found.
// FUNCTION: TONY2 0x0041eb05
TonyS32 __fastcall HandleStopInstances(TonyS32 p_handle)
{
	TonyS32 result;
	TonyS32 slot;

	result = -1;

	if (g_mixerActive) {
		p_handle = VoiceHandleOwner(p_handle);

		while (p_handle != -1) {
			slot = p_handle & 0xff;

			if (g_seqVoices[slot].m_handle == p_handle) {
				if (g_seqVoices[slot].m_eventList != 0) {
					VoiceResetSlot(&g_seqVoices[slot]);
				}

				ChanFree(slot);
				result = 0;
			}

			p_handle = g_seqVoices[slot].m_chainNext;
		}
	}

	return result;
}

// Stops every sequenced voice, optionally keeping looping ones, and frees the
// mixer channels.
// FUNCTION: TONY2 0x0041eba1
void __fastcall SeqStopVoices(TonyU8 p_keep)
{
	TonyS32 i;

	for (i = 0; i < g_playingChannelCount; i++) {
		if (g_seqVoices[i].m_eventList != NULL) {
			if (!p_keep || (p_keep != 0 && !g_seqVoices[i].m_class)) {
				VoiceResetSlot(&g_seqVoices[i]);
				g_seqVoices[i].m_eventList = NULL;
				g_seqVoices[i].m_flags &= ~3;
				g_seqVoices[i].m_priority = 0;
				g_seqVoices[i].m_age = 0;
			}
			else {
				goto next;
			}
		}

		ChanFree(i);
	next:;
	}
}

// Applies a track-level fade command: the realtime commands fade every track of the
// matching way toward the level over the span, other commands fade one track and
// record the pending handle and flag.
// FUNCTION: TONY2 0x0041ece7
void __fastcall TrackFadeCommand(TonyU8 p_level, TonyU16 p_rate, TonyU8 p_command, TonyU8 p_track,
	TonyS32 p_handle)
{
	TonyU32 fall;
	TonyU8 ebb;
	TonyU32 seep;

	if (!p_rate) {
		p_rate += 1;
	}

	fall = p_rate;
	TickWiden(&fall);

	switch (p_command) {
	case 0xff:
		for (seep = 0; seep < 0x20; seep++) {
			if (!g_trackFades[seep].m_mode || g_trackFades[seep].m_mode == 1) {
				g_trackFades[seep].m_target = p_level << 0x10;
				g_trackFades[seep].m_time = fall;
				g_trackFades[seep].m_step =
					(g_trackFades[seep].m_target - g_trackFades[seep].m_level) / p_rate;
				g_trackFades[seep].m_handle = -1;
			}
		}
		break;
	case 0xfc:
		for (seep = 0; seep < 0x20; seep++) {
			if (g_trackFades[seep].m_mode == 2 || g_trackFades[seep].m_mode == 3) {
				g_trackFades[seep].m_target = p_level << 0x10;
				g_trackFades[seep].m_time = fall;
				g_trackFades[seep].m_step =
					(g_trackFades[seep].m_target - g_trackFades[seep].m_level) / p_rate;
				g_trackFades[seep].m_handle = -1;
			}
		}
		break;
	case 0xfa:
		ebb = 2;
		goto sweep;
	case 0xfb:
		ebb = 3;
		goto sweep;
	case 0xfd:
		ebb = 0;
		goto sweep;
	case 0xfe:
		ebb = 1;
	sweep:
		for (seep = 0; seep < 0x20; seep++) {
			if (g_trackFades[seep].m_mode == ebb) {
				g_trackFades[seep].m_target = p_level << 0x10;
				g_trackFades[seep].m_time = fall;
				g_trackFades[seep].m_step =
					(g_trackFades[seep].m_target - g_trackFades[seep].m_level) / p_rate;
				g_trackFades[seep].m_handle = -1;
			}
		}
		break;
	default:
		g_trackFades[p_command].m_target = p_level << 0x10;
		g_trackFades[p_command].m_time = fall;
		g_trackFades[p_command].m_step =
			(g_trackFades[p_command].m_target - g_trackFades[p_command].m_level) / p_rate;
		g_trackFades[p_command].m_command = p_track;
		g_trackFades[p_command].m_handle = p_handle;
	}
}
// FUNCTION: TONY2 0x0041f02a
void __fastcall TrackFadeTo(TonyU8 p_level, TonyS32 p_span, TonyU8 p_track)
{
	if (g_mixerActive) {
		MixerLock();
		TrackFadeCommand(p_level, p_span, p_track, 0, 0);
		MixerUnlock();
	}
}

// Reports whether a track is actively fading out.
// FUNCTION: TONY2 0x0041f0c7
TonyU8 __fastcall TrackIsFading(TonyU8 p_track)
{
	if (g_trackFades[p_track].m_mode != 4 && g_trackFades[p_track].m_time != 0 &&
		g_trackFades[p_track].m_step < 0) {
		return 1;
	}

	return 0;
}

// Sets a track's mode byte.
// FUNCTION: TONY2 0x0041f3a8
void __fastcall TrackSetMode(TonyU8 p_track, TonyU8 p_mode)
{
	if (g_mixerActive) {
		g_trackFades[p_track].m_mode = p_mode;
	}
}

// Returns TRUE when no queued sound and no busy channel remains.
// FUNCTION: TONY2 0x0041f3d8
TonyU8 SndIsIdle(void)
{
	TonyU8 busy = 0;

	if (g_mixerActive) {
		g_sndIdleQueryBusy = 1;

		if (SndHasQueuedWork() == 0) {
			TonyS32 i;

			for (i = 0; i < g_playingChannelCount; i++) {
				busy |= ChanIsActive(i);
			}
		}
		else {
			busy = 1;
		}

		g_sndIdleQueryBusy = 0;
	}

	return busy == 0;
}

// Stores the sequencer timing configuration.
// Resets the whole mixer state: output rate, default tempo, every mixer slot, the
// track fades and the event pool.
// FUNCTION: TONY2 0x0041f451
void __fastcall SeqConfigureTiming(TonyS32 p_rate)
{
	TonyS32 i;

	g_speechSaved2 = p_rate;
	g_speechSaved1 = 0x2800;
	SeqSetSlotTickRate(0x78, 0xff);
	g_seqServiceArmed = 0;

	for (i = 0; i < 0x40; i++) {
		g_seqVoices[i].m_handle = -1;
		g_seqVoices[i].m_eventList = NULL;
		g_seqVoices[i].m_flags = 0;
		g_seqVoices[i].m_age = 0;
		g_seqVoices[i].m_priority = 0;
		g_seqVoices[i].m_loopCount = 0;
		g_seqVoices[i].m_sound = 0xff;
		g_seqVoices[i].m_level = 0;
		g_seqVoices[i].m_pan = 0x3f0000;
		g_seqVoices[i].m_tremoloPhase = 0;
		g_seqVoices[i].m_autoPanPhase = 0;
		g_seqVoices[i].m_tremoloReload = 0;
		g_seqVoices[i].m_autoPanReload = 0;
		g_seqVoices[i].m_reserved = 0;
		g_seqVoices[i].m_track = 0x17;
		g_seqVoices[i].m_timers[0].m_period = 0;
		g_seqVoices[i].m_timers[0].m_value = 0;
		g_seqVoices[i].m_timers[1].m_period = 0;
		g_seqVoices[i].m_timers[1].m_value = 0;
		g_seqVoices[i].m_glidePos = 0x1f4;
		g_seqVoices[i].m_muteMode = 0;
	}

	for (i = 0; i < 0x20; i++) {
		g_trackFades[i].m_level = 0;
		g_trackFades[i].m_target = 0;
		g_trackFades[i].m_time = 0;
		g_trackFades[i].m_mode = 4;
		g_trackFades[i].m_groupLevel = 0x7f0000;
	}

	g_trackGroupEnable = 1;

	for (i = 0; i < 8; i++) {
		g_trackFades[i + 0x17].m_mode = 0;
	}

	g_trackFades[0x15].m_level = 0x7f0000;
	g_trackFades[0x16].m_level = 0x7f0000;
	PoolRebuildFree();
}

// Rebuilds the mixer event pool free list.
// FUNCTION: TONY2 0x0041f6cd
void PoolRebuildFree(void)
{
	TonyS32 i;
	PoolNode* prev;

	g_voiceHandleCounter = 0;
	g_voiceRecordList = NULL;
	g_voiceRecordFree = g_voicePool;
	prev = NULL;

	for (i = 0; i < 0x40; i++) {
		g_voicePool[i].m_prev = prev;

		if (prev != NULL) {
			prev->m_next = &g_voicePool[i];
		}

		prev = &g_voicePool[i];
	}

	prev->m_next = NULL;
}

// Resets the sequencer service state: note pool cursors, retrigger count and the
// 64-key note buckets.
// FUNCTION: TONY2 0x0041f753
void SeqResetService(void)
{
	TonyS32 i;

	g_songRegCount = 0;
	g_regAltCount = 0;
	g_regCount = 0;
	g_regTaggedCount = 0;
	g_notePoolCursor = 0;

	for (i = 0; i < 0x200; i++) {
		g_noteBuckets[i].m_count = 0;
		g_noteBuckets[i].m_start = 0;
	}
}

// Returns the opened-device description, or NULL while the library is closed.
// FUNCTION: TONY2 0x0041f7c4
DeviceInfo* SndGetDeviceInfo(void)
{
	if (g_mixerActive) {
		return &g_deviceInfo;
	}

	return NULL;
}

// Forwards the master level to the wave driver hook.
// FUNCTION: TONY2 0x0041f7dd
void __fastcall SndSetMasterLevel(TonyS32 p_level)
{
	if (g_mixerActive) {
		WaveMasterLevelHook(p_level);
	}
}

// Runs the wave service hook under the mixer lock.
// FUNCTION: TONY2 0x0041f7fb
void SndRunWaveService(void)
{
	if (g_mixerActive) {
		MixerLock();
		WaveServiceHook();
		MixerUnlock();
	}
}

// Opens the sequencer: clears the note pools, track transposes and service state,
// stores the timing configuration, resets the mixer tables and unpauses.
// FUNCTION: TONY2 0x0041f81a
TonyS32 __fastcall SeqOpen(TonyS32 p_rate)
{
	TonyS32 result;
	TonyS32 i;

	result = 0;
	PendingResetWord();

	for (i = 0; i < 0x10; i++) {
		g_trackTranspose[i] = 0;
	}

	SeqResetService();
	ChannelResetAll();
	g_sndIdleQueryBusy = 0;
	SeqConfigureTiming(p_rate);
	DevVoiceResetAll();
	WaveOutputMix();
	g_mixerActive = 1;
	return result;
}

// Mixer init (DirectSound path): clamps the channel count, stores the config bytes and
// opens the output device; p_a carries the sample rate in and the device handle
// out.
// FUNCTION: TONY2 0x0041f884
TonyS32 __fastcall SndOpenDSound(
	TonyU32 p_rate,
	TonyU8 p_channels,
	TonyU8 p_depth,
	TonyU8 p_classes,
	TonyU8 p_stereo,
	TonyU32 p_mode,
	TonyU32 p_config,
	void* p_hWnd
)
{
	TonyS32 result;

	result = 0;
	g_mixerActive = 0;

	if (p_channels <= 0x40) {
		g_playingChannelCount = p_channels;
	}
	else {
		g_playingChannelCount = 0x40;
	}

	g_speechSavedFlag = p_depth;
	g_channelBusyCount = p_classes;
	result = SndOpenOutput(p_hWnd, &p_rate, g_playingChannelCount, p_stereo, p_mode & 0xffff, p_config);

	if (result == 0) {
		result = SeqOpen(p_rate);
	}

	return result;
}

// Mixer init twin without the window/format extras (waveOut path).
// FUNCTION: TONY2 0x0041f911
TonyS32 __fastcall SndOpenWaveOut(
	TonyU32 p_rate,
	TonyU8 p_channels,
	TonyU8 p_depth,
	TonyU8 p_classes,
	TonyU32 p_mode,
	TonyU32 p_config
)
{
	TonyS32 result;

	result = 0;
	g_mixerActive = 0;

	if (p_channels <= 0x40) {
		g_playingChannelCount = p_channels;
	}
	else {
		g_playingChannelCount = 0x40;
	}

	g_speechSavedFlag = p_depth;
	g_channelBusyCount = p_classes;
	result = SndOpenOutputOnDevice(&p_rate, g_playingChannelCount, p_mode & 0xffff, p_config);

	if (result == 0) {
		result = SeqOpen(p_rate);
	}

	return result;
}

// FUNCTION: TONY2 0x0041f996
void SndClose(void)
{
	if (g_mixerActive) {
		MixerLockInit();

		if (g_mixBusPrimary) {
			SndFree(g_mixBusPrimary);
		}

		WaveTeardownHook();
		g_mixerActive = 0;
	}
}

// FUNCTION: TONY2 0x0041f9cb
void SeqClose(void)
{
	if (g_mixerActive) {
		MixerLockClose();

		if (g_mixBusPrimary) {
			SndFree(g_mixBusPrimary);
		}

		WaveTeardownHook();
		g_mixerActive = 0;
	}
}

// Claims a free sample channel and wires it for a stream entry: directories and
// program maps, key masks / rate / initial levels and route wiring from the start
// pack, the song clock and tracks, controller resets and the per-route setup rows.
// Returns the minted play handle or -1.
// FUNCTION: TONY2 0x0041fa30
TonyS32 __fastcall ChannelOpenStream(
	void* p_entries, void* p_names, StreamConfig* p_record, SongBankHeader* p_song, StreamStart* p_pack)
{
	TonyS32 scan;
	SampleChannel* husk;
	TonyU32 i;
	TonyU8 list;
	TonyS32 strip;
	TonyS32* chans;
	TonyU8* code;
	SampleRoute* found;
	TonyS32 chunk;
	TonyU8 pos;

	for (i = 0; i < 8; i++) {
		if (g_sampleChannels[i].m_free) {
			break;
		}
	}

	if (i != 8) {
		husk = &g_sampleChannels[i];
	husk->m_deferredArmed = 0;
	husk->m_instDir = (SampleRoute*) p_entries;
	husk->m_kitDir = (SampleRoute*) p_names;
	husk->m_song = p_song;
	BuildProgramMap(husk->m_instMap, husk->m_instDir);
	BuildProgramMap(husk->m_kitMap, husk->m_kitDir);
	husk->m_fadeTrack = i + 0x17;

	for (strip = 0; strip < 0x40; strip++) {
		husk->m_voiceTracks[strip] = husk->m_fadeTrack;
	}

	if (p_pack == NULL) {
		husk->m_trackMask[0] = -1;
		husk->m_trackMask[1] = -1;
		husk->m_rateScale = 0x100;
	}
	else {
		if (p_pack->m_flags & 1) {
			husk->m_trackMask[0] = p_pack->m_trackMaskLo;
			husk->m_trackMask[1] = p_pack->m_trackMaskHi;
		}
		else {
			husk->m_trackMask[0] = -1;
			husk->m_trackMask[1] = -1;
		}

		if (p_pack->m_flags & 2) {
			husk->m_rateScale = p_pack->m_rateScale;
		}
		else {
			husk->m_rateScale = 0x100;
		}

		if (p_pack->m_flags & 8) {
			for (strip = 0; strip < p_pack->m_remapCount; strip++) {
				husk->m_voiceTracks[p_pack->m_remapPairs[strip * 2]] = p_pack->m_remapPairs[strip * 2 + 1];
				TrackSetMode(p_pack->m_remapPairs[strip * 2 + 1], 0);
			}
		}

		if (p_pack->m_flags & 4) {
			TrackFadeCommand(p_pack->m_fadeLevel, p_pack->m_fadeSpan, husk->m_fadeTrack, 0, 0);

			for (strip = 0; strip < p_pack->m_fadeTrackCount; strip++) {
				TrackFadeCommand(p_pack->m_fadeLevel, p_pack->m_fadeSpan, p_pack->m_fadeTracks[strip], 0, 0);
			}
		}
	}

	scan = (TonyS32) p_song;
	husk->m_tickRate = ((SongBankHeader*) scan)->m_tickRate;
	SeqSetSlotTickRate(husk->m_tickRate, i);
	husk->m_tempoStream = (TonyU32*) (((SongBankHeader*) scan)->m_tempoStreamOfs + scan);
	husk->m_tempoCursor = husk->m_tempoStream;
	husk->m_clockFrac = 0;
	husk->m_clockPos = 0;
	husk->m_noLoop = 0;
	chans = (TonyS32*) (((SongBankHeader*) scan)->m_trackTableOfs + scan);

	for (strip = 0; strip < 0x40; strip++) {
		g_trackVolumes[strip] = 0x7f;
		husk->m_tracks[strip].m_frac = 0;
		husk->m_tracks[strip].m_pos = 0;
		husk->m_cursors[strip].m_event = NULL;
		husk->m_cursors[strip].m_bendStream = NULL;
		husk->m_cursors[strip].m_wheelStream = NULL;

		if (chans[strip] != 0) {
			husk->m_tracks[strip].m_stream = (NoteEvent*) (chans[strip] + scan);
			husk->m_tracks[strip].m_cursor = husk->m_tracks[strip].m_stream;
		}
		else {
			husk->m_tracks[strip].m_stream = NULL;
			husk->m_tracks[strip].m_cursor = NULL;
		}
	}

	husk->m_noteOffChain = NULL;
	husk->m_retireChain = NULL;

	for (strip = 0; strip < 0x10; strip++) {
		SeqResetChannel(strip, i);
	}

	for (strip = 0; strip < 0x10; strip++) {
		husk->m_routePrograms[strip] = -1;
	}

	found = husk->m_instDir;
	code = husk->m_instMap;

	for (chunk = 0; chunk < 2; chunk++) {
		for (strip = 0; strip < 0x80; strip++) {
			pos = code[strip];

			if (pos != 0xff) {
				list = found[pos].m_route;

				if (list != 0xff) {
					husk->m_routePrograms[list] =
						found[pos].m_listId << 0x10 | found[pos].m_priority << 8 | found[pos].m_polyphony;
				}
			}
		}

		found = husk->m_kitDir;
		code = husk->m_kitMap;
	}

	if (p_record != NULL) {
		for (strip = 0; strip < 0x10; strip++) {
			ChannelProgramChange(husk, p_record->m_routes[strip].m_program, strip);
			SeqSetController(7, strip, i, p_record->m_routes[strip].m_level);
			SeqSetController(0xa, strip, i, p_record->m_routes[strip].m_pan);
			SeqSetController(0x5b, strip, i, p_record->m_routes[strip].m_reverb);
			SeqSetController(0x5d, strip, i, p_record->m_routes[strip].m_chorus);
		}
	}

	husk->m_loopCount = 0;

	if (p_pack != NULL) {
		if (!(p_pack->m_flags & 0x10)) {
			husk->m_running = 1;
		}
	}
	else {
		husk->m_running = 1;
	}

	i = ChannelMintHandle(i);
	husk->m_free = 0;
	return i;
	}

	return -1;
}

// Mints a fresh play handle for a sample channel, skipping values still used by
// any busy channel.
// FUNCTION: TONY2 0x0042006f
TonyS32 __fastcall ChannelMintHandle(TonyS32 p_slot)
{
	TonyS32 result;
	TonyU32 i;

	do {
		result = g_pendingCount;
		g_pendingCount = g_pendingCount + 1;
		g_pendingCount = g_pendingCount & 0x7fffffff;

		for (i = 0; i < 8; i++) {
			if (g_sampleChannels[i].m_free == 0 && g_sampleChannels[i].m_handle == result) {
				result = -1;
				break;
			}
		}
	} while (result == -1);

	g_sampleChannels[p_slot].m_handle = result;
	return result;
}

// Applies a program change for a route of the channel: resolves the program (or,
// on route 9, the kit) through the channel map and packs the directory record into
// the route's play handle.
// FUNCTION: TONY2 0x0042010c
void __fastcall ChannelProgramChange(struct SampleChannel* p_channel, TonyU8 p_program, TonyU8 p_route)
{
	if (p_route != 9) {
		p_program = p_channel->m_instMap[p_program];

		if (p_program != 0xff) {
			p_channel->m_routePrograms[p_route] = p_channel->m_instDir[p_program].m_listId << 0x10 |
											 p_channel->m_instDir[p_program].m_priority << 8 |
											 p_channel->m_instDir[p_program].m_polyphony;
		}
	}
	else {
		p_program = p_channel->m_kitMap[p_program];

		if (p_program != 0xff) {
			p_channel->m_routePrograms[p_route] = p_channel->m_kitDir[p_program].m_listId << 0x10 |
											 p_channel->m_kitDir[p_program].m_priority << 8 |
											 p_channel->m_kitDir[p_program].m_polyphony;
		}
	}
}

// Builds a program map from a sample directory: clears all 0x80 slots, then keys
// each record's program byte back to its directory index.
// FUNCTION: TONY2 0x0042023c
void __fastcall BuildProgramMap(TonyU8* p_map, SampleRoute* p_records)
{
	TonyU8 i;

	for (i = 0; i < 0x80; i++) {
		p_map[i] = 0xff;
	}

	for (i = 0; p_records->m_program != 0xff; i++, p_records = p_records + 1) {
		p_map[p_records->m_program] = i;
	}
}

// Returns the loop counter of the sample channel behind a play handle, 0 when idle
// or wave-routed.
// FUNCTION: TONY2 0x004202b8
TonyU16 __fastcall ChannelGetLoopCount(TonyS32 p_handle)
{
	if (g_mixerActive) {
		p_handle = HandleToChannel(p_handle);

		if (p_handle != -1) {
			if (!(p_handle & 0x80000000)) {
				return g_sampleChannels[p_handle].m_loopCount;
			}
		}
	}

	return 0;
}

// Maps a sample/stream handle to its channel slot (sign bit preserved), or -1.
// FUNCTION: TONY2 0x00420301
TonyS32 __fastcall HandleToChannel(TonyS32 p_handle)
{
	TonyU32 i;

	for (i = 0; i < 8; i++) {
		if (g_sampleChannels[i].m_free == 0 && g_sampleChannels[i].m_handle == (p_handle & 0x7fffffff)) {
			return i | p_handle & 0x80000000;
		}
	}

	return -1;
}

// FUNCTION: TONY2 0x0042036a
TonyU8 __fastcall ChannelIsBusy(TonyS32 p_handle)
{
	if (g_mixerActive) {
		return HandleToChannel(p_handle) != -1;
	}

	return 0;
}

// Resumes the sample or stream paused under p_handle.
// FUNCTION: TONY2 0x00420396
void __fastcall ChannelResume(TonyS32 p_handle)
{
	p_handle = HandleToChannel(p_handle);

	if (p_handle != -1) {
		if (!(p_handle & 0x80000000)) {
			if (g_sampleChannels[p_handle].m_running && g_sampleChannels[p_handle].m_free == 0) {
				g_sampleChannels[p_handle].m_running = 0;
				ChannelFreeQueued(&g_sampleChannels[p_handle]);
				ChannelSpliceFree(&g_sampleChannels[p_handle]);
			}
		}
		else {
			p_handle &= 0x7fffffff;

			if (g_sampleChannels[p_handle].m_running && g_sampleChannels[p_handle].m_free == 0) {
				g_sampleChannels[p_handle].m_streamFlags |= 8;
			}
		}
	}
}

// Splices both of the channel's pending chains onto the shared free list.
// FUNCTION: TONY2 0x0042047b
void __fastcall ChannelSpliceFree(struct SampleChannel* p_channel)
{
	PendingCmd* cursor;

	cursor = p_channel->m_noteOffChain;

	if (cursor != NULL) {
		for (; cursor->m_next != NULL; cursor = cursor->m_next) {
		}

		if (g_pendingFree != NULL) {
			cursor->m_next = (PendingCmd*) g_pendingFree;
			g_pendingFree->m_prev = (PoolNode*) cursor;
		}

		g_pendingFree = (PoolNode*) p_channel->m_noteOffChain;
		p_channel->m_noteOffChain = NULL;
	}

	cursor = p_channel->m_retireChain;

	if (cursor != NULL) {
		for (; cursor->m_next != NULL; cursor = cursor->m_next) {
		}

		if (g_pendingFree != NULL) {
			cursor->m_next = (PendingCmd*) g_pendingFree;
			g_pendingFree->m_prev = (PoolNode*) cursor;
		}

		g_pendingFree = (PoolNode*) p_channel->m_retireChain;
		p_channel->m_retireChain = NULL;
	}
}

// Releases every queued node payload on both pending chains.
// FUNCTION: TONY2 0x0042054a
void __fastcall ChannelFreeQueued(struct SampleChannel* p_channel)
{
	PendingCmd* cursor;

	for (cursor = p_channel->m_noteOffChain; cursor != NULL; cursor = cursor->m_next) {
		HandleStopInstances(cursor->m_handle);
	}

	for (cursor = p_channel->m_retireChain; cursor != NULL; cursor = cursor->m_next) {
		HandleStopInstances(cursor->m_handle);
	}
}

// Resume entry (locked wrapper around ChannelResume).
// FUNCTION: TONY2 0x004205a9
void __fastcall SampleResume(TonyS32 p_handle)
{
	if (g_mixerActive) {
		MixerLock();
		ChannelResume(p_handle);
		MixerUnlock();
	}
}

// Stops the sample or stream playing under p_handle.
// FUNCTION: TONY2 0x004205d1
void __fastcall ChannelStop(TonyS32 p_handle)
{
	p_handle = HandleToChannel(p_handle);

	if (p_handle != -1) {
		if (!(p_handle & 0x80000000)) {
			if (g_sampleChannels[p_handle].m_running && g_sampleChannels[p_handle].m_free == 0) {
				g_sampleChannels[p_handle].m_running = 0;
				ChannelFreeQueued(&g_sampleChannels[p_handle]);
				ChannelSpliceFree(&g_sampleChannels[p_handle]);
			}

			g_sampleChannels[p_handle].m_free = 1;
		}
		else {
			p_handle &= 0x7fffffff;

			if (g_sampleChannels[p_handle].m_running && g_sampleChannels[p_handle].m_free == 0) {
				g_sampleChannels[p_handle].m_deferredOut = 0;
			}
		}
	}
}

// FUNCTION: TONY2 0x004206ba
void __fastcall SampleStop(TonyS32 p_handle)
{
	if (g_mixerActive) {
		MixerLock();
		ChannelStop(p_handle);
		MixerUnlock();
	}
}

// Stops all eight sample channels.
// FUNCTION: TONY2 0x004206e2
void ChannelStopAll(void)
{
	TonyU32 i;

	for (i = 0; i < 8; i++) {
		ChannelStop(g_sampleChannels[i].m_handle);
	}
}

// Sets the rate scale of the sample channel behind a handle, or defers it on a
// pending handle.
// FUNCTION: TONY2 0x00420774
void __fastcall ChannelSetRateScale(TonyS32 p_handle, TonyU16 p_rate)
{
	p_handle = HandleToChannel(p_handle);

	if (p_handle != -1) {
		if (!(p_handle & 0x80000000)) {
			g_sampleChannels[p_handle].m_rateScale = p_rate;
		}
		else {
			p_handle = p_handle & 0x7fffffff;
			g_sampleChannels[p_handle].m_streamFlags |= 0x20;
			g_sampleChannels[p_handle].m_streamRate = p_rate;
		}
	}
}

// Locked wrapper around the rate-scale setter.
// FUNCTION: TONY2 0x004207f8
void __fastcall SampleSetRate(TonyS32 p_handle, TonyU16 p_rate)
{
	if (g_mixerActive) {
		MixerLock();
		ChannelSetRateScale(p_handle, p_rate);
		MixerUnlock();
	}
}

// Pauses the sample or stream playing under p_handle.
// FUNCTION: TONY2 0x00420828
void __fastcall ChannelPause(TonyS32 p_handle)
{
	p_handle = HandleToChannel(p_handle);

	if (p_handle != -1) {
		if (!(p_handle & 0x80000000)) {
			g_sampleChannels[p_handle].m_running = 1;
		}
		else {
			g_sampleChannels[p_handle & 0x7fffffff].m_streamFlags &= ~8;
		}
	}
}

// FUNCTION: TONY2 0x0042088e
void __fastcall SamplePause(TonyS32 p_handle)
{
	if (g_mixerActive) {
		MixerLock();
		ChannelPause(p_handle);
		MixerUnlock();
	}
}

// Sets the pitch-slide pair of the sample channel behind a handle, or defers it on
// a pending handle.
// FUNCTION: TONY2 0x004208b6
void __fastcall ChannelSetPitchSlide(TonyS32 p_handle, TonyS32 p_step, TonyS32 p_span)
{
	p_handle = HandleToChannel(p_handle);

	if (p_handle != -1) {
		if (!(p_handle & 0x80000000)) {
			g_sampleChannels[p_handle].m_trackMask[0] = p_step;
			g_sampleChannels[p_handle].m_trackMask[1] = p_span;
		}
		else {
			p_handle = p_handle & 0x7fffffff;
			g_sampleChannels[p_handle].m_streamFlags |= 0x10;
			g_sampleChannels[p_handle].m_streamMaskLo = p_step;
			g_sampleChannels[p_handle].m_streamMaskHi = p_span;
		}
	}
}

// Locked wrapper around the pitch-slide setter.
// FUNCTION: TONY2 0x00420960
void __fastcall SampleSetPitchSlide(TonyS32 p_handle, TonyS32 p_step, TonyS32 p_span)
{
	if (g_mixerActive) {
		MixerLock();
		ChannelSetPitchSlide(p_handle, p_step, p_span);
		MixerUnlock();
	}
}

// Applies a volume fade to a playing sample: resolves the handle, fades its category
// track and every other track it spans, or in direct mode stores the volume and fade
// flags on the sample state.
// FUNCTION: TONY2 0x00420996
void __fastcall ChannelFadeVolume(TonyU8 p_volume, TonyU16 p_span, TonyS32 p_handle, TonyU8 p_mode)
{
	TonyS32 asked;
	TonyU32 i;

	asked = p_handle;
	p_handle = HandleToChannel(p_handle);

	if (p_handle != -1) {
		if (!(p_handle & 0x80000000)) {
			TrackFadeCommand(p_volume, p_span, g_sampleChannels[p_handle].m_fadeTrack, p_mode, asked);

			for (i = 0; i < 0x40; i++) {
				if (g_sampleChannels[p_handle].m_voiceTracks[i] != g_sampleChannels[p_handle].m_fadeTrack) {
					TrackFadeCommand(p_volume, p_span, g_sampleChannels[p_handle].m_voiceTracks[i], 0, 0);
				}
			}
		}
		else {
			p_handle &= 0x7fffffff;

			switch (p_mode & 0xf) {
			case 0:
				g_sampleChannels[p_handle].m_streamVolume = p_volume;
				break;
			case 1:
				g_sampleChannels[p_handle].m_deferredOut = 0;
				break;
			case 2:
				g_sampleChannels[p_handle].m_streamFlags |= 8;
				g_sampleChannels[p_handle].m_streamVolume = p_volume;
				break;
			case 3:
				g_sampleChannels[p_handle].m_streamFlags |= 0x80;
				g_sampleChannels[p_handle].m_streamVolume = p_volume;
				break;
			}
		}
	}
}
// FUNCTION: TONY2 0x00420b31
void __fastcall SampleFadeVolume(TonyU8 p_volume, TonyU16 p_span, TonyS32 p_handle, TonyU8 p_mode)
{
	if (g_mixerActive) {
		MixerLock();
		ChannelFadeVolume(p_volume, p_span, p_handle, p_mode);
		MixerUnlock();
	}
}

// Clears the sequencer voice records.
// Applies a queued play command: defers it onto the channel when requested, else
// routes level/pan/pitch fades (immediate or deferred variants) and stream restarts,
// writing the resulting handle back through p_out.
// FUNCTION: TONY2 0x00420bf9
void __fastcall CmdApplyPlay(PlayCmd* p_cmd, TonyS32* p_out, TonyU8 p_direct)
{
	TonyS32 slot;
	StreamStart block;

	slot = HandleToChannel(p_cmd->m_fadeHandle);

	if (slot != -1) {
		if (p_cmd->m_flags & 4) {
			*(PlayCmd*) &g_sampleChannels[slot].m_deferredCmd = *p_cmd;
			g_sampleChannels[slot].m_deferredArmed = 1;
			g_sampleChannels[slot].m_deferredOut = (TonyS32) p_out;
			g_sampleChannels[slot].m_streamFlags &= ~4;
			*p_out = p_cmd->m_fadeHandle | 0x80000000;
			return;
		}

		if (p_direct) {
			if (p_cmd->m_flags & 1) {
				ChannelFadeVolume(0, p_cmd->m_fadeSpan, p_cmd->m_fadeHandle, 2);
			}
			else if (p_cmd->m_flags & 0x40) {
				ChannelFadeVolume(0, p_cmd->m_fadeSpan, p_cmd->m_fadeHandle, 3);
			}
			else {
				ChannelFadeVolume(0, p_cmd->m_fadeSpan, p_cmd->m_fadeHandle, 1);
			}
		}
		else {
			if (p_cmd->m_flags & 1) {
				SampleFadeVolume(0, p_cmd->m_fadeSpan, p_cmd->m_fadeHandle, 2);
			}
			else if (p_cmd->m_flags & 0x40) {
				SampleFadeVolume(0, p_cmd->m_fadeSpan, p_cmd->m_fadeHandle, 3);
			}
			else {
				SampleFadeVolume(0, p_cmd->m_fadeSpan, p_cmd->m_fadeHandle, 1);
			}
		}

		if (p_out != NULL) {
			if (p_cmd->m_flags & 2) {
				if (HandleToChannel(p_cmd->m_resumeHandle) != -1) {
					if (p_direct) {
						ChannelPause(p_cmd->m_resumeHandle);
						ChannelFadeVolume(p_cmd->m_volume, p_cmd->m_resumeSpan, p_cmd->m_resumeHandle, 0);

						if (p_cmd->m_flags & 0x10) {
							ChannelSetPitchSlide(p_cmd->m_resumeHandle, p_cmd->m_trackMaskLo, p_cmd->m_trackMaskHi);
						}

						if (p_cmd->m_flags & 0x20) {
							ChannelSetRateScale(p_cmd->m_resumeHandle, p_cmd->m_rateScale);
						}
					}
					else {
						SamplePause(p_cmd->m_resumeHandle);
						SampleFadeVolume(p_cmd->m_volume, p_cmd->m_resumeSpan, p_cmd->m_resumeHandle, 0);

						if (p_cmd->m_flags & 0x10) {
							SampleSetPitchSlide(p_cmd->m_resumeHandle, p_cmd->m_trackMaskLo, p_cmd->m_trackMaskHi);
						}

						if (p_cmd->m_flags & 0x20) {
							SampleSetRate(p_cmd->m_resumeHandle, p_cmd->m_rateScale);
						}
					}

					*p_out = p_cmd->m_resumeHandle;
				}
				else {
					*p_out = -1;
				}
			}
			else {
				block.m_flags = 4;

				if (p_cmd->m_flags & 8) {
					block.m_flags |= 0x10;
				}

				if (p_cmd->m_flags & 0x20) {
					block.m_flags |= 2;
					block.m_rateScale = p_cmd->m_rateScale;
				}

				if (p_cmd->m_flags & 0x10) {
					block.m_flags |= 1;
					block.m_trackMaskLo = p_cmd->m_trackMaskLo;
					block.m_trackMaskHi = p_cmd->m_trackMaskHi;
				}

				block.m_fadeSpan = p_cmd->m_resumeSpan;
				block.m_fadeLevel = p_cmd->m_volume;
				block.m_fadeTrackCount = 0;

				if (p_direct) {
					*p_out = StreamEntryStart(p_cmd->m_bankId, p_cmd->m_entryId, (void*) p_cmd->m_songData, &block, 1);

					if (*p_out != -1) {
						if (p_cmd->m_flags & 0x80) {
							ChannelSetPitchSlide(*p_out, 0, 0);
						}
					}
				}
				else {
					*p_out = StreamPlayEntry(p_cmd->m_bankId, p_cmd->m_entryId, (void*) p_cmd->m_songData, &block);

					if (*p_out != -1) {
						if (p_cmd->m_flags & 0x80) {
							SampleSetPitchSlide(*p_out, 0, 0);
						}
					}
				}
			}
		}
	}
}

// Pumps the eight sample channels for one block: publishes the active channel,
// runs the per-block pipeline and retires channels whose stages all report idle.
// FUNCTION: TONY2 0x00421035
void ChannelPumpAll(void)
{
	TonyU32 i;
	TonyU8 chunk;
	TonyU8 ramp;
	TonyU8 res;

	if (g_dsndBlockFrames != 0) {
		for (i = 0; i < 8; i++) {
			if (g_sampleChannels[i].m_running) {
				g_activeChannel = &g_sampleChannels[i];
				g_activeChannelIndex = i;
				g_activeProgram = TrackIsFading(g_sampleChannels[i].m_fadeTrack);
				ChannelPumpTempo();
				ChannelDeriveStep();
				ChannelAdvanceClock();
				chunk = ChannelPumpTracks();
				ramp = ChannelPumpEvents();
				ChannelPumpBends();
				ChannelPumpWheel();
				ChannelAdvanceCursors();
				res = ChannelPumpScheduled();
				ChannelRetireCommands();

				if (!chunk && !ramp && !res) {
					g_sampleChannels[i].m_running = 0;
					g_sampleChannels[i].m_free = 1;
				}
			}
		}
	}
}

// Retires finished pending commands of the active sample channel.
// FUNCTION: TONY2 0x0042113c
void ChannelRetireCommands(void)
{
	PendingCmd* cur;
	PendingCmd* next;

	for (cur = g_activeChannel->m_retireChain; cur != NULL; cur = next) {
		next = cur->m_next;

		if (HandleValidate(cur->m_handle) == -1) {
			PendingUnlink(cur);
		}
	}
}

// Unlinks a pending command from the active channel's chain and returns it to the
// pending pool free list.
// FUNCTION: TONY2 0x00421184
void __fastcall PendingUnlink(PendingCmd* p_node)
{
	if (p_node->m_next != NULL) {
		p_node->m_next->m_prev = p_node->m_prev;
	}

	if (p_node->m_prev != NULL) {
		p_node->m_prev->m_next = p_node->m_next;
	}
	else {
		g_activeChannel->m_retireChain = p_node->m_next;
	}

	p_node->m_next = (PendingCmd*) g_pendingFree;

	if (g_pendingFree != NULL) {
		g_pendingFree->m_prev = (PoolNode*) p_node;
	}

	p_node->m_prev = NULL;
	g_pendingFree = (PoolNode*) p_node;
}

// Walks every voice's sample event stream: program changes, hold pedal, raw
// controllers and note-ons (with key filtering, transpose and clamping, allocation
// of a pending node and the actual voice start). Returns TRUE while any stream is
// live.
// FUNCTION: TONY2 0x004211ff
TonyU8 ChannelPumpEvents(void)
{
	PendingCmd* scan;
	TonyU8 list;
	TonyU8 res;
	TonyS32 done;
	TonyS32 i;
	TonyS32 names;
	TonyS32 entry;
	TonyS32 pos;

	list = 0;

	for (i = 0; i < 0x40; i++) {
		if (g_activeChannel->m_cursors[i].m_event != NULL) {
			list = 1;
		}

		while (g_activeChannel->m_cursors[i].m_event != NULL &&
			   g_activeChannel->m_cursors[i].m_event->m_time <=
				   g_activeChannel->m_cursors[i].m_clock.m_pos + g_activeChannel->m_stepLead) {
				entry = g_activeChannel->m_cursors[i].m_event->m_note;
				pos = g_activeChannel->m_cursors[i].m_event->m_velocity;
				res = g_activeChannel->m_cursors[i].m_route;

				if (entry == 0xff && pos == 0xff) {
					g_activeChannel->m_cursors[i].m_event = NULL;
					goto rescan;
				}

				if ((entry & 0x80) == 0x80 && pos == 0) {
					ChannelProgramChange(g_activeChannel, entry & 0x7f, res);
				}
				else if ((entry & 0x80) == 0x80 && pos == 1) {
					RouteHold(entry & 0x7f, res);
				}
				else if ((entry & 0x80) == 0x80 && (pos & 0x80) == 0x80) {
					if ((pos & 0x7f) == 0x68) {
						if (g_activeChannel->m_deferredArmed) {
							CmdApplyPlay((PlayCmd*) g_activeChannel->m_deferredCmd, (TonyS32*) g_activeChannel->m_deferredOut, 1);
							g_activeChannel->m_deferredArmed = 0;
						}
					}
					else {
						SeqSetController(pos & 0x7f, res, g_activeChannelIndex, entry & 0x7f);
					}
				}
				else {
					done = g_activeChannel->m_cursors[i].m_track;

					if (g_activeChannel->m_trackMask[done / 0x20] & 1 << (done & 0x1f)) {
						names = g_activeChannel->m_routePrograms[res];

						if (names != -1) {
							entry = entry + (TonyS8) g_activeChannel->m_cursors[i].m_noteOffset;
							entry = entry > 0x7f ? 0x7f : (entry < 0 ? 0 : entry);
							pos = pos + (TonyS8) g_activeChannel->m_cursors[i].m_velocityOffset;
							pos = pos > 0x7f ? 0x7f : (pos < 0 ? 0 : pos);
							scan = PendingTake();

							if (scan != NULL) {
								names = NoteStartByClass(names, entry, pos, 0x40, res, g_activeChannelIndex, 0,
									(TonyU16) (TonyU8) done, g_activeChannel->m_voiceTracks[done],
									g_activeProgram ? -1 : 0);

								if (names != -1) {
									scan->m_handle = names;
									scan->m_offTime = g_activeChannel->m_cursors[i].m_event->m_time +
													  g_activeChannel->m_cursors[i].m_event->m_duration;
									scan->m_clock = g_activeChannel->m_cursors[i].m_clock;
								}
								else {
									PendingRelease(scan);
								}
							}
						}
					}
				}

				g_activeChannel->m_cursors[i].m_event = g_activeChannel->m_cursors[i].m_event + 1;
		rescan:;
		}
	}

	return list;
}

// Takes a pending node from the shared free list and pushes it onto the active
// channel's note-off chain.
// FUNCTION: TONY2 0x004215dd
PendingCmd* PendingTake(void)
{
	PendingCmd* node;

	node = (PendingCmd*) g_pendingFree;

	if (node != NULL) {
		g_pendingFree = (PoolNode*) node->m_next;

		if (g_pendingFree != NULL) {
			g_pendingFree->m_prev = NULL;
		}

		node->m_prev = NULL;
		node->m_next = g_activeChannel->m_noteOffChain;

		if (node->m_next != NULL) {
			g_activeChannel->m_noteOffChain->m_prev = node;
		}

		g_activeChannel->m_noteOffChain = node;
	}

	return node;
}

// Unlinks a node from the active channel's note-off chain and returns it to the
// shared free list.
// FUNCTION: TONY2 0x00421657
void __fastcall PendingRelease(PendingCmd* p_node)
{
	if (p_node->m_next != NULL) {
		p_node->m_next->m_prev = p_node->m_prev;
	}

	if (p_node->m_prev != NULL) {
		p_node->m_prev->m_next = p_node->m_next;
	}
	else {
		g_activeChannel->m_noteOffChain = p_node->m_next;
	}

	p_node->m_next = (PendingCmd*) g_pendingFree;

	if (g_pendingFree != NULL) {
		g_pendingFree->m_prev = (PoolNode*) p_node;
	}

	p_node->m_prev = NULL;
	g_pendingFree = (PoolNode*) p_node;
}

// Routes a hold command through the controller dispatcher (controller 0x82).
// FUNCTION: TONY2 0x004216d2
void __fastcall RouteHold(TonyU8 p_value, TonyU8 p_channel)
{
	SeqSetController(0x82, p_channel, g_activeChannelIndex, p_value);
}

// Advances every voice cursor of the active sample channel by the channel step
// (16.16 fixed point split into fraction and sample position).
// FUNCTION: TONY2 0x004216f7
void ChannelAdvanceCursors(void)
{
	TonyS32 i;
	TonyS32 acc;

	for (i = 0; i < 0x40; i++) {
		acc = g_activeChannel->m_cursors[i].m_clock.m_frac + g_activeChannel->m_stepFrac;
		g_activeChannel->m_cursors[i].m_clock.m_frac = acc & 0xffff;
		acc = acc >> 0x10;
		g_activeChannel->m_cursors[i].m_clock.m_pos =
			g_activeChannel->m_cursors[i].m_clock.m_pos + (acc + g_activeChannel->m_stepWhole);
	}
}

// Advances the active channel's scheduled-note chain: notes whose position reaches
// their due time are started and moved to the active chain, the rest track the
// channel step. Returns TRUE while any note is scheduled.
// FUNCTION: TONY2 0x004217a0
TonyU8 ChannelPumpScheduled(void)
{
	PendingCmd* scan;
	PendingCmd* list;
	TonyS32 res;
	TonyS32 chunk;

	if (g_activeChannel->m_noteOffChain == NULL) {
		return 0;
	}

	for (scan = g_activeChannel->m_noteOffChain; scan != NULL; scan = list) {
		list = scan->m_next;
		chunk = scan->m_clock.m_pos + g_activeChannel->m_stepLead;

		if (chunk >= scan->m_offTime) {
			ChainRelease((void*) scan->m_handle);
			PendingActivate(scan);
		}
		else {
			res = scan->m_clock.m_frac + g_activeChannel->m_stepFrac;
			scan->m_clock.m_frac = res & 0xffff;
			res = res >> 0x10;
			scan->m_clock.m_pos = scan->m_clock.m_pos + (res + g_activeChannel->m_stepWhole);
		}
	}

	return 1;
}

// Moves a scheduled note that just started from the scheduled chain to the head of
// the active chain.
// FUNCTION: TONY2 0x0042186c
void __fastcall PendingActivate(PendingCmd* p_node)
{
	if (p_node->m_next != NULL) {
		p_node->m_next->m_prev = p_node->m_prev;
	}

	if (p_node->m_prev != NULL) {
		p_node->m_prev->m_next = p_node->m_next;
	}
	else {
		g_activeChannel->m_noteOffChain = p_node->m_next;
	}

	p_node->m_next = g_activeChannel->m_retireChain;

	if (p_node->m_next != NULL) {
		g_activeChannel->m_retireChain->m_prev = p_node;
	}

	p_node->m_prev = NULL;
	g_activeChannel->m_retireChain = p_node;
}

// Walks every voice's pitch-bend event stream: consumes due delta/value pairs
// (1- or 2-byte forms), accumulates the bend and routes coarse/fine through the
// controller dispatcher.
// FUNCTION: TONY2 0x004218f9
void ChannelPumpBends(void)
{
	TonyS32 i;
	TonyU8 entry;
	TonyU8 pos;
	TonyU8 scan;
	TonyU16 res;
	TonyS16 names;
	TonyS32 chunk;
	TonyU32 list;

	for (i = 0; i < 0x40; i++) {
		if (g_activeChannel->m_cursors[i].m_bendStream != NULL) {
			entry = 0;

			do {
				pos = *g_activeChannel->m_cursors[i].m_bendStream;
				scan = g_activeChannel->m_cursors[i].m_bendStream[1];

				if (pos == 0x80 && scan == 0) {
					g_activeChannel->m_cursors[i].m_bendStream = NULL;
					entry = 1;
				}
				else {
					if (pos & 0x80) {
						res = (pos & 0x7f) << 8 | scan;
						chunk = 2;
					}
					else {
						res = pos;
						chunk = 1;
					}

					list = g_activeChannel->m_cursors[i].m_bendTime + res;

					if (g_activeChannel->m_cursors[i].m_clock.m_pos + g_activeChannel->m_stepLead >= list) {
						g_activeChannel->m_cursors[i].m_bendStream += chunk;
						g_activeChannel->m_cursors[i].m_bendTime = list;
						pos = *g_activeChannel->m_cursors[i].m_bendStream;
						scan = g_activeChannel->m_cursors[i].m_bendStream[1];

						if (pos & 0x80) {
							names = (pos & 0x7f) << 8 | scan;
							names <<= 1;
							names >>= 1;
							g_activeChannel->m_cursors[i].m_bendStream += 2;
						}
						else {
							names = (TonyS8) pos;
							names <<= 9;
							names >>= 9;
							g_activeChannel->m_cursors[i].m_bendStream += 1;
						}

						g_activeChannel->m_cursors[i].m_bendValue += names;
						SeqSetController(0x80, g_activeChannel->m_cursors[i].m_route, g_activeChannelIndex,
							g_activeChannel->m_cursors[i].m_bendValue >> 7);
						SeqSetController(0x81, g_activeChannel->m_cursors[i].m_route, g_activeChannelIndex,
							g_activeChannel->m_cursors[i].m_bendValue & 0x7f);
					}
					else {
						entry = 1;
					}
				}
			} while (!entry);
		}
	}
}

// Walks every voice's modulation event stream: consumes due delta/value pairs and
// routes the accumulated wheel through controllers 1/0x21.
// FUNCTION: TONY2 0x00421c1c
void ChannelPumpWheel(void)
{
	TonyS32 i;
	TonyU8 entry;
	TonyU8 pos;
	TonyU8 scan;
	TonyS16 res;
	TonyS32 chunk;
	TonyU32 list;

	for (i = 0; i < 0x40; i++) {
		if (g_activeChannel->m_cursors[i].m_wheelStream != NULL) {
			entry = 0;

			do {
				pos = *g_activeChannel->m_cursors[i].m_wheelStream;
				scan = g_activeChannel->m_cursors[i].m_wheelStream[1];

				if (pos == 0x80 && scan == 0) {
					g_activeChannel->m_cursors[i].m_wheelStream = NULL;
					entry = 1;
				}
				else {
					if (pos & 0x80) {
						res = (pos & 0x7f) << 8 | scan;
						chunk = 2;
					}
					else {
						res = pos;
						chunk = 1;
					}

					list = g_activeChannel->m_cursors[i].m_wheelTime + res;

					if (g_activeChannel->m_cursors[i].m_clock.m_pos + g_activeChannel->m_stepLead >= list) {
						g_activeChannel->m_cursors[i].m_wheelStream += chunk;
						g_activeChannel->m_cursors[i].m_wheelTime = list;
						pos = *g_activeChannel->m_cursors[i].m_wheelStream;
						scan = g_activeChannel->m_cursors[i].m_wheelStream[1];

						if (pos & 0x80) {
							res = (pos & 0x7f) << 8 | scan;
							res <<= 1;
							res >>= 1;
							g_activeChannel->m_cursors[i].m_wheelStream += 2;
						}
						else {
							res = (TonyS8) pos;
							g_activeChannel->m_cursors[i].m_wheelStream += 1;
						}

						g_activeChannel->m_cursors[i].m_wheelValue += res;
						SeqSetController(1, g_activeChannel->m_cursors[i].m_route, g_activeChannelIndex,
							g_activeChannel->m_cursors[i].m_wheelValue >> 7);
						SeqSetController(0x21, g_activeChannel->m_cursors[i].m_route, g_activeChannelIndex,
							g_activeChannel->m_cursors[i].m_wheelValue & 0x7f);
					}
					else {
						entry = 1;
					}
				}
			} while (!entry);
		}
	}
}

// Pumps every song track of the active channel: starts due notes (wiring the voice
// cursors, streams and controllers), handles loop and end markers and advances each
// track clock. Returns TRUE while any track is live.
// FUNCTION: TONY2 0x00421f23
TonyU8 ChannelPumpTracks(void)
{
	NoteDesc* scan;
	TonyU32 list;
	TonyS32* res;
	TonyS32 i;
	NoteEvent* chunk;
	TonyU8 found;
	TonyU8 code;

	found = 1;
	code = 0;

	for (i = 0; i < 0x40; i++) {
		if (g_activeChannel->m_tracks[i].m_cursor != NULL) {
			code = 1;

			while (g_activeChannel->m_tracks[i].m_cursor->m_time <=
				   g_activeChannel->m_tracks[i].m_pos + g_activeChannel->m_stepLead) {
				chunk = g_activeChannel->m_tracks[i].m_cursor;

				switch (chunk->m_note) {
				case 0xffff:
					g_activeChannel->m_tracks[i].m_cursor = NULL;
					goto next;
				case 0xfffe:
						if (g_activeChannel->m_noLoop) {
						g_activeChannel->m_tracks[i].m_cursor = NULL;
						goto next;
					}

					list = *(TonyU16*) &chunk->m_noteOffset;
					g_activeChannel->m_tracks[i].m_cursor = g_activeChannel->m_tracks[i].m_stream + list;
					g_activeChannel->m_tracks[i].m_pos = g_activeChannel->m_song->m_loopClock;
					g_activeChannel->m_tracks[i].m_frac = 0;

					if (found) {
						ChannelRewindClock(g_activeChannel->m_song->m_loopClock);
						g_activeChannel->m_loopCount += 1;
						found = 0;
					}

					goto wrap;
				}

				res = (TonyS32*) (g_activeChannel->m_song->m_noteTableOfs + (TonyS32) g_activeChannel->m_song);
				scan = (NoteDesc*) (res[chunk->m_note] + (TonyS32) g_activeChannel->m_song);
				g_activeChannel->m_cursors[i].m_clock.m_pos = 0;
				g_activeChannel->m_cursors[i].m_clock.m_frac = 0;
				g_activeChannel->m_cursors[i].m_event = (SampleEvent*) &scan[1];

				if (scan->m_bendOfs != 0) {
					g_activeChannel->m_cursors[i].m_bendStream =
						(TonyU8*) (scan->m_bendOfs + (TonyS32) g_activeChannel->m_song);
				}
				else {
					g_activeChannel->m_cursors[i].m_bendStream = NULL;
				}

				g_activeChannel->m_cursors[i].m_bendTime = 0;
				g_activeChannel->m_cursors[i].m_bendValue = 0x2000;

				if (scan->m_wheelOfs != 0) {
					g_activeChannel->m_cursors[i].m_wheelStream =
						(TonyU8*) (scan->m_wheelOfs + (TonyS32) g_activeChannel->m_song);
				}
				else {
					g_activeChannel->m_cursors[i].m_wheelStream = NULL;
				}

				g_activeChannel->m_cursors[i].m_wheelTime = 0;
				g_activeChannel->m_cursors[i].m_wheelValue = 0;
				g_activeChannel->m_cursors[i].m_route =
					((TonyU8*) (g_activeChannel->m_song->m_routeMapOfs + (TonyS32) g_activeChannel->m_song))[i];
				g_activeChannel->m_cursors[i].m_velocityOffset = chunk->m_velocityOffset;
				g_activeChannel->m_cursors[i].m_noteOffset = chunk->m_noteOffset;
				g_activeChannel->m_cursors[i].m_track = i;

				if (chunk->m_program != 0xff) {
					ChannelProgramChange(g_activeChannel, chunk->m_program, g_activeChannel->m_cursors[i].m_route);
				}

				if (chunk->m_level != 0xff) {
					RouteLevel(chunk->m_level, g_activeChannel->m_cursors[i].m_route);
				}

				g_activeChannel->m_tracks[i].m_cursor = g_activeChannel->m_tracks[i].m_cursor + 1;
			wrap:;
			}

			list = g_activeChannel->m_tracks[i].m_frac + g_activeChannel->m_stepFrac;
			g_activeChannel->m_tracks[i].m_frac = list & 0xffff;
			list = list >> 0x10;
			g_activeChannel->m_tracks[i].m_pos =
				g_activeChannel->m_tracks[i].m_pos + (list + g_activeChannel->m_stepWhole);
		}

	next:
		list = list;
	}

	return code;
}

// Routes a level command through the controller dispatcher (controller 7).
// FUNCTION: TONY2 0x004223dd
void __fastcall RouteLevel(TonyU8 p_value, TonyU8 p_channel)
{
	SeqSetController(7, p_channel, g_activeChannelIndex, p_value);
}

// Recomputes the active channel's step from its tempo, the block size and the fine
// tune, then derives the block lead (half the whole-sample step).
// FUNCTION: TONY2 0x00422402
void ChannelDeriveStep(void)
{
	TonyU32 step;

	step = g_activeChannel->m_tickRate * 0x600 / 0xf0 * ((TonyS32) (g_dsndBlockFrames << 0x10) / g_speechSaved2);
	step = step * g_activeChannel->m_rateScale >> 8;
	g_activeChannel->m_stepFrac = step & 0xffff;
	g_activeChannel->m_stepWhole = step >> 0x10;
	g_activeChannel->m_stepLead = g_activeChannel->m_stepWhole >> 1;
}

// Consumes due tempo events of the active channel and applies them to the channel
// clock.
// FUNCTION: TONY2 0x00422495
void ChannelPumpTempo(void)
{
	for (; *g_activeChannel->m_tempoCursor != -1; g_activeChannel->m_tempoCursor += 2) {
		if (*g_activeChannel->m_tempoCursor > g_activeChannel->m_clockPos + g_activeChannel->m_stepLead) {
			break;
		}

		SeqSetSlotTickRate(g_activeChannel->m_tickRate = g_activeChannel->m_tempoCursor[1], g_activeChannelIndex);
	}
}

// Advances the active channel's clock by the channel step.
// FUNCTION: TONY2 0x00422524
void ChannelAdvanceClock(void)
{
	TonyU32 acc;

	acc = g_activeChannel->m_clockFrac + g_activeChannel->m_stepFrac;
	g_activeChannel->m_clockFrac = acc & 0xffff;
	acc = acc >> 0x10;
	g_activeChannel->m_clockPos = g_activeChannel->m_clockPos + (acc + g_activeChannel->m_stepWhole);
}

// Rewinds the active channel's clock to p_pos and replays tempo state.
// FUNCTION: TONY2 0x0042258b
void __fastcall ChannelRewindClock(TonyU32 p_pos)
{
	g_activeChannel->m_tempoCursor = g_activeChannel->m_tempoStream;
	g_activeChannel->m_clockPos = p_pos;
	g_activeChannel->m_clockFrac = 0;
	ChannelPumpTempo();
	ChannelDeriveStep();
}

// Resets the eight sample channels and rebuilds the pending-command pool.
// FUNCTION: TONY2 0x004225d5
void ChannelResetAll(void)
{
	TonyU32 i;

	for (i = 0; i < 8; i++) {
		g_sampleChannels[i].m_running = 0;
		g_sampleChannels[i].m_free = 1;
	}

	PendingRebuildFree();
	PendingClearCounter();
}

// Rebuilds the pending-command pool free list.
// FUNCTION: TONY2 0x00422621
void PendingRebuildFree(void)
{
	PendingCmd* cursor;
	TonyS32 i;

	cursor = NULL;
	g_pendingFree = (PoolNode*) g_pendingPool;

	for (i = 0; i < 0x100; i++) {
		g_pendingPool[i].m_prev = cursor;

		if (cursor != NULL) {
			cursor->m_next = &g_pendingPool[i];
		}

		cursor = &g_pendingPool[i];
	}

	cursor->m_next = NULL;
}

// Clears the pending-command counter.
// FUNCTION: TONY2 0x00422696
void PendingClearCounter(void)
{
	g_pendingCount = 0;
}

// Resets the pending-command word.
// FUNCTION: TONY2 0x004226b0
void PendingResetWord(void)
{
	g_bankCount = 0;
}

// Registers every listed resource from a loaded bank: finds each id's chunk and
// points the resource table at its payload.
// FUNCTION: TONY2 0x004226be
void __fastcall BankRegisterResources(TonyU16* p_ids, void* p_bank)
{
	BankChunk* found;

	while (*p_ids != 0xffff) {
		found = BankFindChunk(*p_ids, p_bank);

		if (found != NULL) {
			RegInsertAlt(*p_ids, (TonyS32) found + 8);
		}

		p_ids++;
	}
}

// Finds a resource chunk in a loaded bank by id.
// FUNCTION: TONY2 0x00422711
BankChunk* __fastcall BankFindChunk(TonyU16 p_id, TonyS32* p_bank)
{
	return BankWalkChunks(p_id, (BankChunk*) ((TonyS32) p_bank + p_bank[1]));
}

// Walks a chunk list for the id, stopping at the -1 terminator.
// FUNCTION: TONY2 0x00422732
BankChunk* __fastcall BankWalkChunks(TonyU16 p_id, BankChunk* p_chunk)
{
	while (p_chunk->m_size != -1) {
		if (p_chunk->m_id == p_id) {
			return p_chunk;
		}

		p_chunk = (BankChunk*) ((TonyS32) p_chunk + p_chunk->m_size);
	}

	return NULL;
}
// Registers every listed song from a loaded bank: finds each id's chunk in the song
// section and points the song table at its payload.
// FUNCTION: TONY2 0x00422773
void __fastcall BankRegisterSongs(TonyU16* p_ids, void* p_bank)
{
	BankChunk* found;

	while (*p_ids != 0xffff) {
		found = BankFindSongChunk(*p_ids, p_bank);

		if (found != NULL) {
			RegInsert(*p_ids, (TonyS32) found + 8);
		}

		p_ids++;
	}
}

// Finds a song chunk in a loaded bank by id.
// FUNCTION: TONY2 0x004227c6
BankChunk* __fastcall BankFindSongChunk(TonyU16 p_id, TonyS32* p_bank)
{
	return BankWalkChunks(p_id, (BankChunk*) ((TonyS32) p_bank + p_bank[2]));
}
// Registers every listed tagged resource from a loaded bank: finds each id's chunk
// in the tagged section and stores its payload with the chunk tag.
// FUNCTION: TONY2 0x004227e7
void __fastcall BankRegisterTagged(TonyU16* p_ids, void* p_bank)
{
	BankChunk* found;

	while (*p_ids != 0xffff) {
		found = BankFindTaggedChunk(*p_ids, p_bank);

		if (found != NULL) {
			RegInsertTagged(*p_ids, (TonyS32) found + 0xc, *(TonyU16*) ((TonyS32) found + 8));
		}

		p_ids++;
	}
}

// Finds a tagged resource chunk in a loaded bank by id.
// FUNCTION: TONY2 0x00422842
BankChunk* __fastcall BankFindTaggedChunk(TonyU16 p_id, TonyS32* p_bank)
{
	return BankWalkChunks(p_id, (BankChunk*) ((TonyS32) p_bank + p_bank[3]));
}
// Registers every listed sample from a loaded sample bank: resolves each id's
// directory entry and registers the descriptor with the absolute data address.
// FUNCTION: TONY2 0x00422863
void __fastcall BankRegisterSamples(TonyU16* p_ids, TonyS32 p_data, void* p_dir)
{
	TonyS32 start;
	TonyS32 gotten;

	p_data = SampleDataBase(p_data);

	while (*p_ids != 0xffff) {
		gotten = BankFindSampleEntry(*p_ids, p_data, p_dir, &start);

		if (gotten != 0) {
			SongRegInsert(*p_ids, gotten, start);
		}

		p_ids++;
	}
}

// Finds a sample directory entry by id: stores the absolute data address and returns
// the descriptor behind the entry header.
// FUNCTION: TONY2 0x004228cc
TonyS32 __fastcall BankFindSampleEntry(TonyU16 p_id, TonyS32 p_data, void* p_dir, TonyS32* p_addr)
{
	SampleDirEntry* cursor;

	cursor = (SampleDirEntry*) p_dir;

	while (cursor->m_id != 0xffff) {
		if (cursor->m_id == p_id) {
			*p_addr = p_data + cursor->m_dataOfs;
			return (TonyS32) cursor + 8;
		}

		cursor++;
	}

	return 0;
}
// Registers every trigger record from a loaded trigger table.
// FUNCTION: TONY2 0x0042292c
void __fastcall TriggerRegisterTable(TriggerTable* p_table)
{
	TonyS32 i;

	for (i = 0; i < p_table->m_count; i++) {
		TriggerInsert(p_table->m_entries[i].m_id, p_table->m_entries[i].m_listId,
			p_table->m_entries[i].m_pan, p_table->m_entries[i].m_level,
			p_table->m_entries[i].m_priority, p_table->m_entries[i].m_polyphony,
			p_table->m_entries[i].m_note);
	}
}
// Unregisters every trigger record from a trigger table.
// FUNCTION: TONY2 0x004229c1
void __fastcall TriggerUnregisterTable(TriggerTable* p_table)
{
	TonyS32 i;

	for (i = 0; i < p_table->m_count; i++) {
		TriggerRemove(p_table->m_entries[i].m_id);
	}
}

// Walks the bank chain for the block with p_id, registers it and initializes its
// sections; returns TRUE when found.
// FUNCTION: TONY2 0x00422a02
TonyU8 __fastcall BankRegister(SoundBank* p_data, TonyU16 p_id, void* p_a, void* p_b, void* p_c)
{
	SoundBank* block;

	if (g_mixerActive) {
		block = p_data;

		while (block->m_size != -1) {
			if (block->m_id == p_id) {
				g_bankBlocks[g_bankCount] = block;
				BankRegisterSequences((char*) block + block->m_sequencesOfs + 8, p_c);
				BankRegisterSamples((char*) block + block->m_samplesOfs + 8, p_a, p_b);
				BankRegisterResources((char*) block + block->m_resourcesOfs + 8, p_c);
				BankRegisterSongs((char*) block + block->m_songsOfs + 8, p_c);
				BankRegisterTagged((char*) block + block->m_taggedOfs + 8, p_c);

				if (block->m_kind == 1) {
					TriggerRegisterTable((char*) block + block->m_triggersOfs + 8);
				}

				g_bankCount++;
				return 1;
			}

			block = (SoundBank*) ((char*) block + block->m_size);
		}
	}

	return 0;
}

// Registers every listed sequence from a loaded bank: finds each id's chunk in the
// first section and points the sequence table at its payload.
// FUNCTION: TONY2 0x00422b15
void __fastcall BankRegisterSequences(TonyU16* p_ids, void* p_bank)
{
	BankChunk* found;

	while (*p_ids != 0xffff) {
		found = BankFindSequenceChunk(*p_ids, p_bank);

		if (found != NULL) {
			NotePoolInsert(*p_ids, (TonyS32) found + 8);
		}

		p_ids++;
	}
}

// Finds a sequence chunk in a loaded bank by id.
// FUNCTION: TONY2 0x00422b68
BankChunk* __fastcall BankFindSequenceChunk(TonyU16 p_id, TonyS32* p_bank)
{
	return BankWalkChunks(p_id, (BankChunk*) ((TonyS32) p_bank + p_bank[0]));
}
// Pops the most recently registered bank block and unregisters everything its
// sections had installed.
// FUNCTION: TONY2 0x00422b88
TonyU8 __fastcall BankPopLast(void)
{
	SoundBank* block;

	if (g_mixerActive) {
		if (g_bankCount) {
			g_bankCount--;
			block = g_bankBlocks[g_bankCount];
			BankUnregisterSequences((char*) block + block->m_sequencesOfs + 8);
			BankUnregisterSongs((char*) block + block->m_samplesOfs + 8);
			BankUnregisterWaves((char*) block + block->m_resourcesOfs + 8);
			BankUnregisterInstruments((char*) block + block->m_songsOfs + 8);
			BankUnregisterKits((char*) block + block->m_taggedOfs + 8);

			if (block->m_kind == 1) {
				TriggerUnregisterTable((char*) block + block->m_triggersOfs + 8);
			}

			return 1;
		}
	}

	return 0;
}

// Unregisters every sequence listed in a bank section.
// FUNCTION: TONY2 0x00422c51
void __fastcall BankUnregisterSequences(TonyU16* p_ids)
{
	while (*p_ids != 0xffff) {
		NotePoolRemove(*p_ids);
		p_ids++;
	}
}

// Unregisters every wave listed in a bank section.
// FUNCTION: TONY2 0x00422c82
void __fastcall BankUnregisterWaves(TonyU16* p_ids)
{
	while (*p_ids != 0xffff) {
		RegRemoveAlt(*p_ids);
		p_ids++;
	}
}

// Unregisters every instrument listed in a bank section.
// FUNCTION: TONY2 0x00422cb3
void __fastcall BankUnregisterInstruments(TonyU16* p_ids)
{
	while (*p_ids != 0xffff) {
		RegRemove(*p_ids);
		p_ids++;
	}
}

// Unregisters every kit listed in a bank section.
// FUNCTION: TONY2 0x00422ce4
void __fastcall BankUnregisterKits(TonyU16* p_ids)
{
	while (*p_ids != 0xffff) {
		RegRemoveTagged(*p_ids);
		p_ids++;
	}
}

// Unregisters every song listed in a bank section.
// FUNCTION: TONY2 0x00422d15
void __fastcall BankUnregisterSongs(TonyU16* p_ids)
{
	while (*p_ids != 0xffff) {
		SongRegRemove(*p_ids);
		p_ids++;
	}
}

// Unused stub handler; ignores its arguments and reports failure.
// FUNCTION: TONY2 0x00422d46
TonyU8 __fastcall BankStubHandler(TonyS32 p_a, TonyU16 p_b, TonyS32 p_c, TonyS32 p_d)
{
	return 0;
}

// Finds p_b's stream entry in bank p_a and starts it; p_e skips
// the mixer lock. Returns the start result or -1.
// FUNCTION: TONY2 0x00422d5b
TonyS32 __fastcall StreamEntryStart(
	TonyU16 p_bank, TonyU16 p_entry, void* p_data, StreamStart* p_start, TonyU8 p_noLock)
{
	TonyS32 i;
	SoundBank* entry;
	StreamConfig* scan;
	void* list;
	void* chunk;
	TonyS32 res;

	if (g_mixerActive) {
		for (i = 0; i < g_bankCount; i++) {
			if (g_bankBlocks[i]->m_id == p_bank) {
				if (g_bankBlocks[i]->m_kind == 0) {
					entry = g_bankBlocks[i];
					list = (char*) entry + entry->m_triggersOfs + 8;
					chunk = (char*) entry + entry->m_kitDirOfs + 8;

					for (scan = (StreamConfig*) ((char*) entry + entry->m_streamsOfs + 8);
						 scan->m_id != 0xffff; scan++) {
						if (scan->m_id == p_entry) {
							if (p_noLock) {
								res = ChannelOpenStream(list, chunk, scan, p_data, p_start);
							}
							else {
								MixerLock();
								res = ChannelOpenStream(list, chunk, scan, p_data, p_start);
								MixerUnlock();
							}

							return res;
						}
					}
				}

				return -1;
			}
		}
	}

	return -1;
}

// FUNCTION: TONY2 0x00422ea8
TonyS32 __fastcall StreamPlayEntry(TonyU16 p_bank, TonyU16 p_entry, void* p_data, StreamStart* p_start)
{
	return StreamEntryStart(p_bank, p_entry, p_data, p_start, 0);
}

// Prepares the output ring for a write: restores a lost buffer, (re)starts looping
// playback, and computes how many bytes may be mixed ahead of the play cursor;
// returns the staging buffer and the aligned, clamped byte budget.
// FUNCTION: TONY2 0x00422ed0
TonyU8 __fastcall RingPrepareWrite(void** p_base, TonyU32* p_avail)
{
	TonyU32 fall;
	TonyU32 seep;
	TonyS32 ebb;
	TonyU32 fade;

	g_dsndOutputBuffer->m_vtbl->m_getStatus(g_dsndOutputBuffer, &seep);

	if (seep & 2) {
		if (g_dsndOutputBuffer->m_vtbl->m_restore(g_dsndOutputBuffer)) {
			return 0;
		}
	}

	g_dsndOutputBuffer->m_vtbl->m_getStatus(g_dsndOutputBuffer, &seep);

	if (!(seep & 1)) {
		if (g_outputMode) {
			return 0;
		}

		g_dsndOutputBuffer->m_vtbl->m_play(g_dsndOutputBuffer, 0, 0, 1);
		g_ringWriteCursor = 0;
	}
	else {
		if (g_outputMode) {
			g_dsndOutputBuffer->m_vtbl->m_stop(g_dsndOutputBuffer);
			return 0;
		}
	}

	if (g_dsndOutputBuffer->m_vtbl->m_getCurrentPosition(g_dsndOutputBuffer, &fade, &fall) == 0) {
		ebb = (g_ringWriteCursor < fade ? g_ringWriteCursor + g_ringSize : g_ringWriteCursor);
		ebb = fade + g_ringLead - ebb;

		if (ebb > 0) {
			ebb = ((ebb / g_outputFormat.m_blockAlign + 0xf) & 0xfffffff0) * g_outputFormat.m_blockAlign;

			if (ebb > g_ringSize / 2) {
				ebb = g_ringSize / 2;
			}

			*p_base = g_mixerStaging;
			*p_avail = ebb;
			return 1;
		}
	}

	return 0;
}

// Commits p_bytes from the staging buffer into the locked output ring, splitting
// across the ring wrap; returns TRUE on success.
// FUNCTION: TONY2 0x0042304d
TonyU8 __fastcall RingCommit(TonyU32 p_bytes)
{
	if (p_bytes != 0) {
		if (g_dsndOutputBuffer->m_vtbl->m_lock(g_dsndOutputBuffer, g_ringWriteCursor, p_bytes, &g_ringSegment1, &g_ringSegment1Size,
				&g_ringSegment2, &g_ringSegment2Size, 0) == 0) {
			g_ringWriteCursor = (g_ringWriteCursor + p_bytes) % g_ringSize;

			if (g_ringSegment1Size != 0) {
				if (p_bytes >= g_ringSegment1Size) {
					RingCopy(g_ringSegment1, g_mixerStaging, g_ringSegment1Size);
					p_bytes -= g_ringSegment1Size;
				}
				else {
					RingCopy(g_ringSegment1, g_mixerStaging, p_bytes);
					p_bytes = 0;
				}
			}

			if (p_bytes != 0) {
				if (p_bytes >= g_ringSegment2Size) {
					RingCopy(g_ringSegment2, (char*) g_mixerStaging + g_ringSegment1Size, g_ringSegment2Size);
				}
				else {
					RingCopy(g_ringSegment2, (char*) g_mixerStaging + g_ringSegment1Size, p_bytes);
				}
			}

			if (g_dsndOutputBuffer->m_vtbl->m_unlock(
					g_dsndOutputBuffer, g_ringSegment1, g_ringSegment1Size, g_ringSegment2, g_ringSegment2Size) == 0) {
				return 1;
			}
		}
	}

	return 0;
}

// Hand-written dword copy into the locked ring.
// FUNCTION: TONY2 0x0042319a
void __cdecl RingCopy(void* p_dest, void* p_source, TonyU32 p_bytes)
{
	__asm {
		push edi
		push esi
		mov ecx, dword ptr [p_bytes]
		mov edi, dword ptr [p_dest]
		shr ecx, 2
		mov esi, dword ptr [p_source]
		cld
		repne movsd
		pop esi
		pop edi
	}
}

// One mixer service tick: pumps the streams, then renders as many whole blocks as
// fit into the output ring (events, sample channels, song pump, wave mix, voice
// refills) and commits them.
// FUNCTION: TONY2 0x004231b8
void MixerServiceTick(void)
{
	void* scan;
	TonyU32 list;
	TonyU32 res;
	TonyU32 chunk;

	if (g_outputPaused) {
		if (g_mixerActive) {
			StreamPump();

			if (RingPrepareWrite(&scan, &res)) {
				list = res / g_bytesPerFrame + g_dsndBlockFrames - 1;
				chunk = 0;

				while (list >= g_dsndBlockFrames) {
					DevVoiceRefreshGuards();
					ChannelPumpAll();
					SeqRunFrame();
					WaveDriverMix(g_waveChannels, g_dsndBlockFrames, (char*) scan + chunk * g_bytesPerFrame);
					chunk += g_dsndBlockFrames;
					g_mixerBlockBytes += g_dsndBlockFrames;
					list -= g_dsndBlockFrames;
					DevVoicePumpAll();
				}

				RingCommit(chunk * g_bytesPerFrame);
			}
		}
	}
}

// Timer callback of the timer-driven mixer service: skips overlapping ticks, arms
// the FPU on the first tick and runs the mixer tick under the mixer semaphore.
// FUNCTION: TONY2 0x00423297
void __stdcall MixerTimerProc(TonyU32 p_id, TonyU32 p_msg, TonyU32 p_user, TonyU32 p_dw1, TonyU32 p_dw2)
{
	if (WaitForSingleObject(g_mixerThread, 0) == 0) {
		WaitForSingleObject(g_mixerSemaphore, -1);

		if (g_mixerFirstTick) {
			MixerInitFpu();
			g_mixerFirstTick = 0;
		}

		MixerServiceTick();
		ReleaseSemaphore(g_mixerSemaphore, 1, 0);
		ReleaseSemaphore(g_mixerThread, 1, 0);
	}
}

// Reinitializes the FPU for the mixer service and drops the precision control to
// single (hand-written).
// FUNCTION: TONY2 0x004232fd
void MixerInitFpu(void)
{
	__asm {
		finit
		wait
		fstcw word ptr [g_mixerFpuControl]
		wait
		mov eax, dword ptr [g_mixerFpuControl]
		and eax, 0xfffffcff
		mov dword ptr [g_mixerFpuControl], eax
		fldcw word ptr [g_mixerFpuControl]
		wait
	}
}

// Starts the timer-driven mixer service: semaphore, timer resolution and the
// periodic timer event.
// FUNCTION: TONY2 0x0042332a
void __fastcall MixerStartTimer(TonyS32 p_period, TonyS32 p_b, TonyS32 p_c)
{
	g_outputMode = 0;
	g_mixerThread = CreateSemaphore(0, 1, 1, 0);
	g_mixerTimerPeriod = p_period;
	g_mixerFirstTick = 1;
	timeBeginPeriod(g_mixerTimerPeriod);
	g_mixerTimerId = timeSetEvent(g_mixerTimerPeriod, 0, (LPTIMECALLBACK) MixerTimerProc, 0, 1);
}

// Stops the timer-driven mixer service: kills the timer event, restores the timer
// resolution and closes the service thread handle.
// FUNCTION: TONY2 0x0042338f
void MixerStopTimer(void)
{
	timeKillEvent(g_mixerTimerId);
	timeEndPeriod(g_mixerTimerPeriod);
	CloseHandle(g_mixerThread);
}

// Mixer service thread: waits out each period on the performance counter with 5 ms
// naps, then runs the mixer tick under the mixer semaphore.
// FUNCTION: TONY2 0x004233ba
TonyU32 __stdcall MixerThreadProc(void* p_param)
{
	TonyS32 ticks;
	LARGE_INTEGER hz;
	LARGE_INTEGER from;
	LARGE_INTEGER next;

	SetThreadPriority(g_mixerServiceThread, 0xf);
	MixerInitFpu();
	QueryPerformanceFrequency(&hz);
	ticks = (TonyS32) ((TonyFloat) g_mixerTimerPeriod / 1000.0f * hz.LowPart);
	QueryPerformanceCounter(&from);

	while (1) {
		do {
			Sleep(5);
			QueryPerformanceCounter(&next);
		} while (next.LowPart - from.LowPart < (TonyU32) ticks);

		QueryPerformanceCounter(&from);
		WaitForSingleObject(g_mixerSemaphore, -1);
		MixerServiceTick();
		ReleaseSemaphore(g_mixerSemaphore, 1, 0);
	}

	return 0;
}

// Starts the thread-driven mixer service.
// FUNCTION: TONY2 0x00423476
void __fastcall MixerStartThread(TonyS32 p_period, TonyS32 p_b, TonyS32 p_c)
{
	g_outputMode = 0;
	g_mixerTimerPeriod = p_period;
	g_mixerServiceThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) MixerThreadProc, 0, 0, &g_mixerServiceThreadId);
}

// Stops the thread-driven mixer service.
// FUNCTION: TONY2 0x004234b4
void MixerStopThread(void)
{
	CloseHandle(g_mixerServiceThread);
}

// Starts the mixer service, thread- or timer-driven.
// FUNCTION: TONY2 0x004234c5
void __fastcall MixerStartService(TonyS32 p_period, TonyS32 p_b, TonyS32 p_c, TonyU8 p_thread)
{
	g_mixerThreadMode = p_thread;

	if (g_mixerThreadMode) {
		MixerStartThread(p_period, p_b, p_c);
	}
	else {
		MixerStartTimer(p_period, p_b, p_c);
	}
}

// Stops the mixer service, thread- or timer-driven.
// FUNCTION: TONY2 0x0042350b
void MixerStopService(void)
{
	if (g_mixerThreadMode) {
		MixerStopThread();
	}
	else {
		MixerStopTimer();
	}
}

// Output latency table in milliseconds, indexed by the configuration nibble
// (p_f >> 0x1c) in DSndNegotiateFormat.
// GLOBAL: TONY2 0x0045615c
static TonyU8 g_latencyTable[8] = { 50, 55, 60, 65, 70, 75, 80, 85 };

// GLOBAL: TONY2 0x00456164
static TonyChar g_msgNoSecondaryCaps[] = "Could not get caps of secondary buffer";

// GLOBAL: TONY2 0x0045618c
static TonyChar g_msgNoSecondaryBuffer[] = "Could not create secondary buffer";

// GLOBAL: TONY2 0x004561b0
static TonyChar g_msgNoPrimaryFormat[] = "Could not set format of primary buffer";

// Negotiates the output format: sets the primary buffer format, creates the secondary
// mixing buffer (or aliases the primary in write-primary mode), sizes the staging ring
// from the configured latency, and starts the mixer service.
// FUNCTION: TONY2 0x00423527
TonyU8 __fastcall DSndNegotiateFormat(TonyU32* p_out, TonyU8* p_stereo, TonyU8* p_bits, TonyS32* p_flags, char* p_name,
	TonyU32 p_config)
{
	BufferDesc prim;
	TonyS32 res;
	TonyU32 turns;
	TonyU32 cycle;
	TonyU8 check;

	check = 1;
	g_outputFormat.m_formatTag = 1;
	g_outputFormat.m_channels = (*p_stereo != 0) + 1;
	g_outputFormat.m_samplesPerSec = *p_out;
	g_outputFormat.m_blockAlign = g_outputFormat.m_channels * (*p_bits >> 3);
	g_outputFormat.m_avgBytesPerSec = *p_out * g_outputFormat.m_blockAlign;
	g_outputFormat.m_bitsPerSample = *p_bits;
	g_outputFormat.m_extraSize = 0;

	res = g_dsndPrimaryBuffer->m_vtbl->m_setFormat(g_dsndPrimaryBuffer, &g_outputFormat);
	if (res == 0) {
		if (!(p_config & 0x20000)) {
			g_deviceLost = 0;
			memset(&prim, 0, sizeof(prim));
			prim.m_size = sizeof(prim);
			prim.m_flags = 0x14000;
			prim.m_bufferBytes = (*p_out * g_outputFormat.m_blockAlign * 2 + 0xf) & 0xfffffff0;
			prim.m_format = &g_outputFormat;

			if (g_platformIsNt) {
				prim.m_bufferBytes <<= 1;
			}

			res = g_dsndDevice->m_vtbl->m_createSoundBuffer(g_dsndDevice, &prim, &g_dsndOutputBuffer, NULL);
			if (res == 0) {
			resume:
				memset(&g_dsndBufferCaps, 0, 0x14);
				g_dsndBufferCaps = 0x14;
				res = g_dsndOutputBuffer->m_vtbl->m_getCaps(g_dsndOutputBuffer, &g_dsndBufferCaps);

				if (res == 0) {
					g_mixerStaging = malloc(g_ringSize);

					if (g_mixerStaging != NULL) {
						if (p_config & 0x10000) {
							turns = g_latencyTable[p_config >> 0x1c];
							cycle = turns >> 1;

							if (cycle > 0x28) {
								cycle = 0x28;
							}
						}
						else {
							turns = 0x154;
							cycle = 0x28;
						}

						memset(g_mixerStaging, 0, g_ringSize);
						g_ringWriteCursor = 0;
						g_ringLead = g_outputFormat.m_blockAlign * (TonyS32) (*p_out / 1000.0f * turns);

						if (g_platformIsNt) {
							g_ringLead <<= 1;
						}

						g_ringLead = (g_ringLead + 0xf) & 0xfffffff0;

						if (g_ringLead > g_ringSize) {
							g_ringLead = g_ringSize;
						}

						if (!g_deviceLost) {
							g_dsndOutputBuffer->m_vtbl->m_setVolume(g_dsndOutputBuffer, 0);
						}

						g_dsndPrimaryBuffer->m_vtbl->m_setVolume(g_dsndPrimaryBuffer, 0);
						g_mixerSemaphore = CreateSemaphore(NULL, 1, 1, NULL);
						*p_flags = (TonyU32) g_ringSize / g_outputFormat.m_blockAlign;
						MixerStartService(cycle, *p_flags, *p_out, (p_config & 0x10000) != 0);
						strcpy(p_name, g_dsndDevices[g_dsndOpenedDevice].m_name);
						return 0;
					}
					else {
						check = 3;
					}
				}
				else {
					SndDebugError(g_msgNoSecondaryCaps, res);
				}
			}
			else {
				SndDebugError(g_msgNoSecondaryBuffer, res);
			}
		}
		else {
			g_deviceLost = 1;
			g_dsndOutputBuffer = g_dsndPrimaryBuffer;
			goto resume;
		}
	}
	else {
		SndDebugError(g_msgNoPrimaryFormat, res);
	}

	return check;
}

// Device-open failure messages for the SndDebugError error reports.
// GLOBAL: TONY2 0x004561d8
static TonyChar g_msgNoPrimaryBuffer[] = "Could not create primary sound buffer";

// GLOBAL: TONY2 0x00456200
static TonyChar g_msgNoDSoundCaps[] = "Could not get DirectSound capabilities";

// GLOBAL: TONY2 0x00456228
static TonyChar g_msgNoCoopLevel[] = "Could not set cooperative level";

// Debug error report, compiled out in the release library.
// FUNCTION: TONY2 0x004238a0
void __fastcall SndDebugError(char* p_message, TonyS32 p_code)
{
}

// Opens the DirectSound device: enumerates the installed devices, then tries each
// one (default device first) until the primary buffer and output format negotiation
// of DSndNegotiateFormat succeed.
// FUNCTION: TONY2 0x004238b0
TonyU8 __fastcall DSndOpenDevice(void* p_hWnd, TonyU32* p_out, TonyU8* p_stereo, TonyU8* p_depth, TonyS32* p_flags,
	char* p_name, TonyU32 p_config)
{
	TonyS32 i;
	OSVERSIONINFO info;
	TonyU8 fail;
	DeviceCaps caps;
	BufferDesc prim;
	TonyS32 stat;

	*p_name = 0;
	info.dwOSVersionInfoSize = sizeof(info);
	GetVersionEx(&info);
	g_platformIsNt = info.dwPlatformId != 1;
	g_mixerStaging = NULL;
	fail = 1;
	g_dsndDeviceCount = 0;

	if (DirectSoundEnumerateA(DSndEnumCallback, NULL) == 0) {
		for (i = 0; i < g_dsndDeviceCount; i++) {
			if (DirectSoundCreate(g_dsndDevices[i].m_isDefault ? NULL : &g_dsndDevices[i].m_guid, &g_dsndDevice,
					NULL) == 0) {
				if (p_config & 0x20000) {
					stat = g_dsndDevice->m_vtbl->m_setCooperativeLevel(g_dsndDevice, p_hWnd, 4);
				}
				else {
					stat = g_dsndDevice->m_vtbl->m_setCooperativeLevel(g_dsndDevice, p_hWnd, 3);
				}

				if (stat == 0) {
					memset(&caps, 0, sizeof(caps));
					caps.m_size = sizeof(caps);
					stat = g_dsndDevice->m_vtbl->m_getCaps(g_dsndDevice, &caps);

					if (stat == 0) {
						*p_stereo = (caps.m_flags & 2) != 0;
						*p_depth = (caps.m_flags & 8) ? 0x10 : 8;

						memset(&prim, 0, sizeof(prim));
						prim.m_size = sizeof(prim);
						prim.m_flags = 1;
						prim.m_bufferBytes = 0;
						prim.m_format = NULL;
						stat = g_dsndDevice->m_vtbl->m_createSoundBuffer(g_dsndDevice, &prim, &g_dsndPrimaryBuffer, NULL);

						if (stat == 0) {
							g_dsndOpenedDevice = (TonyU16) i;
							fail = DSndNegotiateFormat(p_out, p_stereo, p_depth, p_flags, p_name, p_config);
							if (fail == 0) {
								return fail;
							}
						}
						else {
							SndDebugError(g_msgNoPrimaryBuffer, stat);
						}
					}
					else {
						SndDebugError(g_msgNoDSoundCaps, stat);
					}
				}
				else {
					SndDebugError(g_msgNoCoopLevel, stat);
				}

				g_dsndDevice->m_vtbl->m_release(g_dsndDevice);
			}
		}
	}

	g_dsndDevice = NULL;
	return fail;
}

// DirectSoundEnumerate callback: records each device GUID and description string,
// stopping once eight devices have been seen.
// FUNCTION: TONY2 0x00423b3f
TonyS32 __stdcall DSndEnumCallback(struct RawGuid* p_guid, char* p_name, char* p_module, void* p_context)
{
	if (p_guid != NULL) {
		g_dsndDevices[g_dsndDeviceCount].m_guid = *p_guid;
		g_dsndDevices[g_dsndDeviceCount].m_isDefault = 0;
	}
	else {
		g_dsndDevices[g_dsndDeviceCount].m_isDefault = 1;
	}

	strcpy(g_dsndDevices[g_dsndDeviceCount].m_name, p_name);
	g_dsndDeviceCount += 1;
	return g_dsndDeviceCount < 8;
}

// Stops the mixer output buffer and, unless the device was lost, releases it, all
// under the mixer semaphore; returns TRUE when the buffer is gone.
// FUNCTION: TONY2 0x00423bee
TonyU8 DSndReleaseBuffer(void)
{
	TonyU8 done;

	done = 0;

	if (g_dsndOutputBuffer->m_vtbl->m_stop(g_dsndOutputBuffer) == 0) {
		WaitForSingleObject(g_mixerSemaphore, -1);
		MixerStopService();

		if (!g_deviceLost) {
			if (g_dsndOutputBuffer->m_vtbl->m_release(g_dsndOutputBuffer) == 0) {
				done = 1;
			}
		}
		else {
			done = 1;
		}

		ReleaseSemaphore(g_mixerSemaphore, 1, 0);
	}

	return done;
}

// Full DirectSound teardown: stops the output buffer, releases the buffers and the
// device under the mixer semaphore, then closes the semaphore and frees the staging
// buffer. Returns TRUE when everything was already gone or fully released.
// FUNCTION: TONY2 0x00423c61
TonyU8 DSndTeardown(void)
{
	TonyU8 done;

	done = 0;

	if (g_dsndDevice) {
		if (g_dsndOutputBuffer->m_vtbl->m_stop(g_dsndOutputBuffer) == 0) {
			WaitForSingleObject(g_mixerSemaphore, -1);
			MixerStopService();

			if (!g_deviceLost) {
				g_dsndOutputBuffer->m_vtbl->m_release(g_dsndOutputBuffer);
			}

			if (g_dsndPrimaryBuffer->m_vtbl->m_release(g_dsndPrimaryBuffer) == 0) {
				if (g_dsndDevice->m_vtbl->m_release(g_dsndDevice)) {
					done = 1;
				}
			}

			ReleaseSemaphore(g_mixerSemaphore, 1, 0);
		}
	}
	else {
		done = 1;
	}

	CloseHandle(g_mixerSemaphore);

	if (g_mixerStaging) {
		free(g_mixerStaging);
	}

	return done;
}

// Opens the sound output: negotiates a 16-bit stereo format with the DirectSound
// device probe, then brings up the mixer with the result.
// FUNCTION: TONY2 0x00423d2b
TonyS32 __fastcall SndOpenOutput(
	void* p_hWnd, TonyU32* p_out, TonyU16 p_channels, TonyU8 p_classes, TonyU32 p_mode, TonyU32 p_config)
{
	TonyS32 flags;
	TonyU8 stereo;
	TonyU8 bits;
	TonyU8 code;

	bits = 0x10;
	stereo = 1;
	code = DSndOpenDevice(p_hWnd, p_out, &stereo, &bits, &flags, g_deviceInfo.m_deviceName, p_config);

	if (code == 0) {
		code = MixNegotiateFormat(*p_out, stereo, bits, flags, p_channels, p_classes, p_mode, p_config);
	}

	return code;
}

// Negotiates the mixer format: seeds the DMA block size from the sample rate, opens
// the wave channels in 16- or 8-bit mode and on success publishes the format and the
// driver banner.
// FUNCTION: TONY2 0x00423da6
TonyU8 __fastcall MixNegotiateFormat(TonyU32 p_rate, TonyU8 p_stereo, TonyU8 p_depth, TonyS32 p_flags,
	TonyU16 p_channels, TonyU8 p_classes, TonyU32 p_config, TonyU32 p_h)
{
	TonyU8 result;

	g_dsndBlockFrames = (p_rate / 0x32) & 0xfffffff0;

	if (p_depth == 0x10) {
		result = WaveOpenChannels(
			g_waveChannels, p_channels, p_rate, 0x11, p_classes, p_flags, p_config, p_h);
	}
	else {
		result = WaveOpenChannels(g_waveChannels, p_channels, p_rate, 8, p_classes, p_flags, p_config, p_h);
	}

	if (result == 0) {
		g_deviceInfo.m_rate = p_rate;
		g_deviceInfo.m_depth = p_depth;
		g_deviceInfo.m_stereo = p_stereo;
		strcpy(g_deviceInfo.m_driverName, g_driverBanner);
		g_bytesPerFrame = p_stereo ? p_depth >> 2 : p_depth >> 3;
		g_mixerBlockBytes = 0;
		g_outputPaused = 1;
	}

	return result;
}

// Opens the sound output on an already-created DirectSound device: negotiates the
// format via the alternate probe, then brings up the mixer.
// FUNCTION: TONY2 0x00423eb5
TonyS32 __fastcall SndOpenOutputOnDevice(TonyU32* p_out, TonyU16 p_channels, TonyU32 p_mode, TonyU32 p_config)
{
	TonyS32 flags;
	TonyU8 stereo;
	TonyU8 bits;
	TonyU8 code;

	bits = 0x10;
	stereo = 1;
	code = DSndNegotiateFormat(p_out, &stereo, &bits, &flags, g_deviceInfo.m_deviceName, p_config);

	if (code == 0) {
		code = MixNegotiateFormat(*p_out, stereo, bits, flags, p_channels, 1, p_mode, p_config);
	}

	return code;
}

// FUNCTION: TONY2 0x00423f2a
void MixerLockInit(void)
{
	g_outputPaused = 0;
	DSndTeardown();
	WaveShutdown();
}

// FUNCTION: TONY2 0x00423f40
void MixerLockClose(void)
{
	DSndReleaseBuffer();
	WaveShutdown();
}

// Reentrant mixer lock.
// FUNCTION: TONY2 0x00423f4f
void MixerLock(void)
{
	if (g_mixerLockDepth == 0) {
		WaitForSingleObject(g_mixerSemaphore, -1);
	}

	g_mixerLockDepth++;
}

// FUNCTION: TONY2 0x00423f7a
void MixerUnlock(void)
{
	g_mixerLockDepth--;

	if (g_mixerLockDepth == 0) {
		ReleaseSemaphore(g_mixerSemaphore, 1, 0);
	}
}

// FUNCTION: TONY2 0x00423fa6
TonyU8 __fastcall ChanIsActive(TonyS32 p_channel)
{
	return WaveDriverBusy(p_channel);
}

// Queued-work query; trivially FALSE in this build of the library.
// FUNCTION: TONY2 0x00423fb9
TonyU8 SndHasQueuedWork(void)
{
	return 0;
}

// Stops the mixer channel behind a voice.
// Arms the mixer channel behind the voice from the song descriptor.
// FUNCTION: TONY2 0x00423fc0
void __fastcall ChanArm(TonyS32 p_channel, SongDesc* p_desc, TonyU8 p_flag)
{
	WaveDriverPrepare((TonyU8) p_channel, p_desc, p_flag);
}

// Stops the mixer channel, optionally releasing it.
// FUNCTION: TONY2 0x00423fe1
void __fastcall ChanStop(TonyS32 p_channel, TonyU8 p_flag)
{
	WaveDriverSetRelease((TonyU8) p_channel, 0x11);

	if (p_flag) {
		WaveDriverRelease((TonyU8) p_channel);
	}
}

// FUNCTION: TONY2 0x00424012
void __fastcall ChanRelease(TonyS32 p_channel)
{
	WaveDriverStop(p_channel);
}

// Loads a channel's envelope descriptor.
// FUNCTION: TONY2 0x00424025
void __fastcall ChanLoadEnvelope(TonyS32 p_channel, TonyU16* p_desc)
{
	WaveDriverSetAdsr(p_channel, p_desc);
}

// Arms a channel's fade-in.
// FUNCTION: TONY2 0x00424040
void __fastcall ChanArmFadeIn(TonyS32 p_channel)
{
	WaveDriverStart(p_channel, 1);
}

// Clears a channel's command acknowledge.
// FUNCTION: TONY2 0x00424055
void __fastcall ChanClearAck(TonyS32 p_channel)
{
	WaveDriverGateOff(p_channel);
}

// Empty driver hook in this build of the library.
// FUNCTION: TONY2 0x00424068
void __fastcall ChanDriverHook(TonyS32 p_channel)
{
}

// Sets a channel's pitch step.
// FUNCTION: TONY2 0x00424073
void __fastcall ChanSetPitchStep(TonyS32 p_channel, TonyS32 p_step)
{
	WaveDriverSetStep(p_channel, p_step);
}

// FUNCTION: TONY2 0x0042408e
void __fastcall ChanSetLevels(TonyS32 p_handle, TonyS32 p_a, TonyS32 p_b, TonyS32 p_c, TonyS32 p_d)
{
	WaveDriverSetGains(p_handle, p_a, p_b, p_c, p_d);
}

// Frees a busy mixer channel (mode 0x11 teardown).
// FUNCTION: TONY2 0x004240b7
void __fastcall ChanFree(TonyS32 p_slot)
{
	if (WaveDriverBusy(p_slot)) {
		WaveDriverSetRelease(p_slot, 0x11);
		WaveDriverRelease(p_slot);
	}
}

// FUNCTION: TONY2 0x004240e8
void __fastcall ChanReset(TonyS32 p_slot)
{
	WaveDriverKill(p_slot);
}

// Returns the sample data base unchanged in this build of the library.
// FUNCTION: TONY2 0x0042419d
TonyS32 __fastcall SampleDataBase(TonyS32 p_data)
{
	return p_data;
}

// Stores the master output mode byte.
// FUNCTION: TONY2 0x004241ab
void __fastcall SndSetOutputMode(TonyU8 p_mode)
{
	g_outputMode = p_mode;
}

// Wave-driver output mix for one block.
// Empty teardown hook in this build of the library.
// Resets the service callback list head.
// Clears the stream list heads.
// FUNCTION: TONY2 0x00426771
void WaveOutputMix(void)
{
	g_streamList = NULL;
	g_streamListeners = NULL;
}

// FUNCTION: TONY2 0x0042678a
void WaveTeardownHook(void)
{
}

// Resets the mixer channel table.
// Marks every device voice slot idle.
// FUNCTION: TONY2 0x00426790
void DevVoiceResetAll(void)
{
	TonyS32 i;

	for (i = 0; i < g_playingChannelCount; i++) {
		g_devVoices[i].m_active = 0;
	}
}

// Advances every active device voice: polls the driver play position and feeds the
// consumed span (split in two on ring wrap) to the slot's refill callback.
// FUNCTION: TONY2 0x004267c6
void DevVoicePumpAll(void)
{
	TonyS32 i;
	TonyS32 mark;

	for (i = 0; i < g_playingChannelCount; i++) {
		if (g_devVoices[i].m_active == 1) {
			mark = WaveGetPlayPosition(i);

			if (g_devVoices[i].m_playPos != mark) {
				if (g_devVoices[i].m_playPos < mark) {
					if (g_devVoices[i].m_refill(g_devVoices[i].m_buffer + g_devVoices[i].m_playPos,
							mark - g_devVoices[i].m_playPos, NULL, 0, g_devVoices[i].m_user)) {
						WaveCacheFlushHook(g_devVoices[i].m_buffer + g_devVoices[i].m_playPos,
							mark - g_devVoices[i].m_playPos);
					}
				}
				else {
					if (g_devVoices[i].m_refill(g_devVoices[i].m_buffer + g_devVoices[i].m_playPos,
							g_devVoices[i].m_size - g_devVoices[i].m_playPos, g_devVoices[i].m_buffer,
							mark, g_devVoices[i].m_user)) {
						WaveCacheFlushHook(g_devVoices[i].m_buffer + g_devVoices[i].m_playPos,
							g_devVoices[i].m_size - g_devVoices[i].m_playPos);
						WaveCacheFlushHook(g_devVoices[i].m_buffer, mark);
					}
				}
			}

			g_devVoices[i].m_playPos = mark;
		}
	}
}

// Refreshes the wrap guards of every active device voice.
// FUNCTION: TONY2 0x0042698b
void DevVoiceRefreshGuards(void)
{
	TonyS32 i;

	for (i = 0; i < g_playingChannelCount; i++) {
		if (g_devVoices[i].m_active == 1) {
			WaveRingWrapGuard(g_devVoices[i].m_buffer, g_devVoices[i].m_size);
		}
	}
}

// Spawns a device voice for a prepared channel; returns its handle or -1.
// FUNCTION: TONY2 0x004269e4
TonyS32 __fastcall DevVoiceSpawn(
	TonyU8 p_a,
	void* p_buffer,
	TonyS32 p_size,
	TonyU32 p_rate,
	TonyU8 p_e,
	TonyU8 p_f,
	TonyU8 p_g,
	TonyU8 p_h,
	TonyU8 (__fastcall* p_callback)(TonyS16*, TonyU32, TonyS16*, TonyU32, TonyS32),
	TonyS32 p_user)
{
	TonyS32 slot;
	TonyFloat step;
	SongDesc block;

	if (g_mixerActive) {
		MixerLock();
		slot = VoiceReserveSlot(p_a);

		if (slot != -1) {
			g_devVoices[slot].m_buffer = p_buffer;
			g_devVoices[slot].m_size = p_size;
			g_devVoices[slot].m_refill = p_callback;
			g_devVoices[slot].m_active = 1;
			g_devVoices[slot].m_playPos = 0;
			g_devVoices[slot].m_user = p_user;

			block.m_rate = p_rate | 0x400000;
			block.m_data = (TonyS32) p_buffer;
			block.m_startPos = 0;
			block.m_endPos = p_size;
			block.m_loopStart = 0;
			block.m_loopLength = p_size;
			block.m_flags = 0;
			ChanArm(slot, &block, 1);

			ChanSetPitchStep(slot, (TonyS32) ((step = (TonyFloat) p_rate / g_speechSaved2) * 4096.0f));
			ChanSetLevels(slot, p_e << 0x10, p_f << 0x10, p_g << 0x10, p_h << 0x10);
			ChanArmFadeIn(slot);
		}

		MixerUnlock();
		return slot;
	}

	return -1;
}

// Voice buffer byte size for a sample count (16-bit mono plus the block header).
// FUNCTION: TONY2 0x00426b32
TonyS32 __fastcall DevVoiceBufferSize(TonyS32 p_samples)
{
	return p_samples * 2 + 8;
}

// Sets the four 8.16 mixer levels for a playing handle.
// FUNCTION: TONY2 0x00426b44
void __fastcall DevVoiceSetLevels(TonyS32 p_handle, TonyS32 p_a, TonyS32 p_b, TonyS32 p_c, TonyS32 p_d)
{
	if (g_mixerActive && p_handle != -1) {
		MixerLock();
		ChanSetLevels(p_handle, (p_a & 0xff) << 0x10, (p_b & 0xff) << 0x10, (p_c & 0xff) << 0x10, (p_d & 0xff) << 0x10);
		MixerUnlock();
	}
}

// Releases a device voice when it is still marked live.
// FUNCTION: TONY2 0x00426bab
void __fastcall DevVoiceRelease(TonyS32 p_handle)
{
	if (g_mixerActive) {
		MixerLock();

		if (g_devVoices[p_handle].m_active == 1) {
			g_devVoices[p_handle].m_active = 0;
			VoiceTeardown(p_handle);
		}

		MixerUnlock();
	}
}

// Opens the wave mixer channels: forces the DirectSound driver, then either sets up
// the software mixer with a freshly allocated bus pair or hands off to the
// DirectSound ring setup.
// FUNCTION: TONY2 0x00427400
TonyU8 __fastcall WaveOpenChannels(void* p_channels, TonyU32 p_count, TonyU32 p_rate, TonyU8 p_mode, TonyU8 p_depth,
	TonyS32 p_samples, TonyU32 p_config, TonyU32 p_h)
{
	TonyU32 size;
	TonyU8 code;

	g_waveDriverMode = 1;
	code = 3;

	switch (g_waveDriverMode) {
	case 0:
		size = p_samples * 4 * 2;
		g_mixerScratch = malloc(size);

		if (g_mixerScratch != NULL) {
			WaveOpen(p_channels, p_count, p_rate, p_mode, p_depth, g_mixerScratch,
				(char*) g_mixerScratch + size / 2);
			code = 0;
		}
		break;
	case 1:
		g_mixerScratch = NULL;
		code = DSndOpen(p_count, p_rate, p_samples, p_config, p_h);
		break;
	}

	return code;
}

// Shuts the wave mixer down, switched by the active driver, and frees the mixer
// scratch buffer.
// FUNCTION: TONY2 0x004274b9
void WaveShutdown(void)
{
	switch (g_waveDriverMode) {
	case 0:
		WaveClose();
		break;
	case 1:
		DSndClose();
		break;
	}

	if (g_mixerScratch) {
		free(g_mixerScratch);
	}
}

// Starts playing voice block p_index on a free channel; returns the new voice handle
// or -1.
// FUNCTION: TONY2 0x0042b160
TonyS32 __fastcall SpeechPlayBlock(
	TonyU32 p_index,
	TonyU32 p_duration,
	TonyU8 p_priority,
	TonyU8 p_level,
	TonyU8 p_pan,
	TonyU8 p_boost,
	TonyU8 p_rateScale
)
{
	// Local names chosen so the /Od frame slots land as in the original (the allocator
	// orders slots by a name hash; "voice"/"i"/"mask" give -4/-8/-0xc).
	SpeechChannel* voice;
	TonyS32 i;
	TonyS32 mask;

	if (g_speechRunning) {
		if (p_index < (TonyU32) g_speechStagingLen) {
			for (i = 0; i < 1; i++) {
				if (g_speechChannels[i].m_state == 0) {
					break;
				}
			}

			if (i != 1) {
				voice = &g_speechChannels[i];

				if (p_duration == 0) {
					p_duration =
						((SpeechSwapHeader(((TonyS32*) g_speechSecondaryBuffer)[p_index]) >> 0x18 & 0xff) ? 0x1f40 : 0) + 0x1f40;
				}

				voice->m_priority = p_priority;
				voice->m_rate = p_duration;
				voice->m_level = p_level;
				voice->m_balance = p_pan;
				voice->m_pan = p_boost;
				voice->m_boost = p_rateScale;
				voice->m_mixSize = (p_duration * 8 / 0x1e + 0x9f) / 0xa0 * 0xa0;
				voice->m_mixBuffer = g_speechMixBuffer;

				if (1) {
					mask = SpeechLock();
					SpeechPrepareChannel(
						voice,
						(SpeechSwapHeader(((TonyS32*) g_speechSecondaryBuffer)[p_index]) & 0xffffff) + (TonyS32) g_speechStagingPtr,
						voice->m_mixBuffer,
						voice->m_mixSize
					);
					voice->m_devVoice = -1;
					voice->m_state = 1;
					SpeechUnlock(mask);
					return SpeechMintHandle(i);
				}
			}
		}
	}

	return -1;
}

// Byte-swaps a big-endian block header dword (the gsmvoice.c twin of SpeechSwapDword).
// FUNCTION: TONY2 0x0042b308
TonyU32 __fastcall SpeechSwapHeader(TonyS32 p_value)
{
	TonyU32 result;

	result = (TonyU32) p_value >> 0x18;
	result |= (TonyU32) p_value >> 8 & 0xff00;
	result |= p_value << 8 & 0xff0000;
	result |= p_value << 0x18;
	return result;
}

// Reentrant voice-mixer lock; the result feeds the matching release call.
// FUNCTION: TONY2 0x0042b356
TonyS32 SpeechLock(void)
{
	if (g_speechLockDepth == 0) {
		WaitForSingleObject(g_speechSemaphore, -1);
	}

	g_speechLockDepth++;
	return 0;
}

// FUNCTION: TONY2 0x0042b383
void __fastcall SpeechUnlock(TonyS32 p_token)
{
	g_speechLockDepth--;

	if (g_speechLockDepth == 0) {
		ReleaseSemaphore(g_speechSemaphore, 1, 0);
	}
}

// Mints a voice handle for the channel that no other busy channel is using.
// FUNCTION: TONY2 0x0042b3b5
TonyS32 __fastcall SpeechMintHandle(TonyS32 p_channel)
{
	TonyU32 i;

	while (1) {
		for (i = 0; i < 1; i++) {
			if (g_speechChannels[i].m_state && i != (TonyU32) p_channel &&
				g_speechChannels[i].m_handle == g_speechChannelCount) {
				break;
			}
		}

		if (i == 1) {
			break;
		}

		g_speechChannelCount++;

		if (g_speechChannelCount == -1) {
			g_speechChannelCount = 0;
		}
	}

	g_speechChannels[p_channel].m_handle = g_speechChannelCount;
	g_speechChannelCount++;

	if (g_speechChannelCount == -1) {
		g_speechChannelCount = 0;
	}

	return g_speechChannels[p_channel].m_handle;
}

// Stops the voice playing under p_handle; returns TRUE when one was found.
// FUNCTION: TONY2 0x0042b48f
TonyU8 __fastcall SpeechStop(TonyS32 p_handle)
{
	if (p_handle != -1 && g_speechRunning) {
		p_handle = SpeechHandleToChannel(p_handle);

		if (p_handle != -1) {
			SpeechStopChannel(p_handle);
			return 1;
		}
	}

	return 0;
}

// Maps a voice handle to its channel index, or -1.
// FUNCTION: TONY2 0x0042b4ca
TonyS32 __fastcall SpeechHandleToChannel(TonyS32 p_handle)
{
	TonyU32 i;

	for (i = 0; i < 1; i++) {
		if (g_speechChannels[i].m_state && g_speechChannels[i].m_handle == p_handle) {
			return i;
		}
	}

	return -1;
}

// Stops a voice channel: queued voices are dropped, playing ones are torn down.
// FUNCTION: TONY2 0x0042b522
void __fastcall SpeechStopChannel(TonyS32 p_channel)
{
	TonyS32 mask;

	switch (g_speechChannels[p_channel].m_state) {
	case 1:
		g_speechChannels[p_channel].m_state = 0;
		break;
	case 2:
		mask = SpeechLock();
		SpeechFlushDecoder(&g_speechChannels[p_channel]);
		g_speechChannels[p_channel].m_state = 3;
		g_speechChannels[p_channel].m_stopTicks = 4;
		SpeechUnlock(mask);
		break;
	}
}

// FUNCTION: TONY2 0x0042b5a5
TonyU8 __fastcall SpeechIsPlaying(TonyS32 p_handle)
{
	if (g_speechRunning) {
		p_handle = SpeechHandleToChannel(p_handle);

		if (p_handle == -1) {
			return 0;
		}

		return g_speechChannels[p_handle].m_state != 0;
	}

	return 0;
}

// Voice-mixer timer tick installed via ServiceStartTimer: advances every channel's
// queued -> playing -> draining state machine.
// FUNCTION: TONY2 0x0042b6a0
void SpeechServiceTick(void)
{
	TonyS32 i;

	if (g_speechRunning) {
		SpeechSuspend();
		QueryPerformanceCounter(&g_speechPumpEnd);

		for (i = 0; i < 1; i++) {
			if (g_speechChannels[i].m_state) {
				SpeechDriveChannel(&g_speechChannels[i]);

				switch (g_speechChannels[i].m_state) {
				case 1:
					if (g_speechChannels[i].m_decodeState == 2) {
						g_speechChannels[i].m_duration =
							(TonyFloat) g_speechChannels[i].m_frameCount * 160.0f / g_speechChannels[i].m_rate;
						g_speechChannels[i].m_consumed = 0;
						g_speechChannels[i].m_state = 2;
						SpeechResetDecoder(&g_speechChannels[i]);
						g_speechChannels[i].m_devVoice = DevVoiceSpawn(
							g_speechChannels[i].m_priority,
							g_speechChannels[i].m_mixBuffer,
							g_speechChannels[i].m_mixSize,
							g_speechChannels[i].m_rate,
							g_speechChannels[i].m_level,
							g_speechChannels[i].m_balance,
							g_speechChannels[i].m_pan,
							g_speechChannels[i].m_boost,
							SpeechProgressCallback,
							i
						);

						if (g_speechChannels[i].m_devVoice == -1) {
							SpeechFlushDecoder(&g_speechChannels[i]);
							g_speechChannels[i].m_state = 0;
						}
					}
					break;
				case 2:
					if (g_speechChannels[i].m_decodeState == 4) {
						g_speechChannels[i].m_state = 3;
						g_speechChannels[i].m_stopTicks = 4;
					}
					break;
				case 3:
					if (g_speechChannels[i].m_stopTicks == 0) {
						g_speechChannels[i].m_state = 0;
					}
					else {
						if (g_speechChannels[i].m_stopTicks == 2) {
							DevVoiceRelease(g_speechChannels[i].m_devVoice);
							g_speechChannels[i].m_devVoice = -1;
						}

						g_speechChannels[i].m_stopTicks--;
					}
					break;
				}
			}
		}

		QueryPerformanceCounter(&g_speechPumpStart);
		g_speechPumpDelta = g_speechPumpStart.LowPart - g_speechPumpEnd.LowPart;
		SpeechResume();
	}
}

// Device-voice progress callback handed to DevVoiceSpawn.
// FUNCTION: TONY2 0x0042b996
TonyU8 __fastcall SpeechProgressCallback(TonyS32 p_a, TonyS32 p_b, TonyS32 p_c, TonyS32 p_d, TonyS32 p_channel)
{
	if (g_speechRunning) {
		SpeechConsumeWindow(&g_speechChannels[p_channel], p_b + p_d);
		g_speechChannels[p_channel].m_consumed += p_b + p_d;
	}

	return 0;
}

// FUNCTION: TONY2 0x0042b9f5
void SpeechSuspend(void)
{
	WaitForSingleObject(g_speechSemaphore, -1);
}

// FUNCTION: TONY2 0x0042ba08
void SpeechResume(void)
{
	ReleaseSemaphore(g_speechSemaphore, 1, 0);
}

// Hands a voice block to the mixer; returns TRUE when accepted.
// FUNCTION: TONY2 0x0042bae4
TonyU8 __fastcall SpeechSubmitBlock(TonyS32* p_data, TonyS32 p_channel)
{
	// The original frame reserves an untouched slot above the homed parameters.
	TonyS32 unused;

	if (g_speechRunning) {
		g_speechStagingPtr = p_data;
		g_speechSecondaryBuffer = p_data + 1;
		g_speechStagingLen = SpeechSwapHeader(*p_data);
		return 1;
	}

	return 0;
}

// The gsmvoice.c assert strings (named so both sides symbolize the operands).
// GLOBAL: TONY2 0x004586e0
static TonyChar g_gsmAssertFile[] = "gsmvoice.c";

// GLOBAL: TONY2 0x004586ec
static TonyChar g_gsmAssertExpr[] = "voice_buffer!=NULL";

// Voice mixer startup: resets the voice slots, allocates the mixing buffer and
// registers the service tick.
// FUNCTION: TONY2 0x0042bb29
void SpeechStartup(void)
{
	TonyS32 i;

	g_speechSecondaryBuffer = 0;

	for (i = 0; i < 1; i++) {
		SpeechPrepareChannel(&g_speechChannels[i], 0, 0, 0);
		g_speechChannels[i].m_state = 0;
	}

	SpeechServiceUnhook();
	g_speechMixBuffer = ServiceAlloc(DevVoiceBufferSize(0x10e0), 0);

	if (g_speechMixBuffer == NULL) {
		_assert(g_gsmAssertExpr, g_gsmAssertFile, 721);
	}

	ServiceStartTimer(SpeechServiceTick);
	g_speechRunning = 1;
	g_speechChannelCount = 0;
}

// FUNCTION: TONY2 0x0042bbd6
void SpeechServiceUnhook(void)
{
	g_speechSemaphore = CreateSemaphoreA(0, 1, 1, 0);
}

// Spin until no voice channel is busy.
// FUNCTION: TONY2 0x0042bbee
void SpeechWaitIdle(void)
{
	TonyS32 i;

	do {
		for (i = 0; i < 1; i++) {
			if (g_speechChannels[i].m_state) {
				break;
			}
		}
	} while (i != 1);
}

// Voice mixer shutdown.
// FUNCTION: TONY2 0x0042bc2d
void SpeechShutdown(void)
{
	TonyS32 i;

	// The original frame reserves a second, untouched slot below the loop counter; a
	// block-scoped local (allocated deeper than function-scope ones at /Od) reproduces it.
	{
		TonyS32 unused;

		for (i = 0; i < 1; i++) {
			SpeechStopChannel(i);
		}
	}

	SpeechWaitIdle();
	g_speechRunning = 0;
	ServiceStopTimer(SpeechServiceTick);

	if (g_speechSecondaryBuffer != 0 && g_speechBufferOwned) {
		ServiceFree(g_speechSecondaryBuffer);
	}

	ServiceFree(g_speechMixBuffer);
	SpeechFreeBuffer();
}

// FUNCTION: TONY2 0x0042bca0
void SpeechFreeBuffer(void)
{
	CloseHandle(g_speechSemaphore);
}

// Byte copy into the staging ring.
// FUNCTION: TONY2 0x0042bcc0
void __fastcall SpeechRingCopy(TonyS32* p_dest, TonyS32 p_source, TonyS32 p_bytes)
{
	char* srcp;
	char* dstp;

	srcp = (char*) p_source;
	dstp = (char*) p_dest;

	for (; p_bytes != 0; p_bytes--) {
		*dstp = *srcp;
		dstp++;
		srcp++;
	}
}

// Byte fill over the staging structures.
// FUNCTION: TONY2 0x0042bd0d
void __fastcall SpeechFill(void* p_dest, TonyU8 p_value, TonyS32 p_bytes)
{
	char* dst;

	dst = (char*) p_dest;

	for (; p_bytes != 0; p_bytes--) {
		*dst = p_value;
		dst++;
	}
}

// Prepares a voice channel for a new block (GSM frame size 0x28, state from p_data).
// FUNCTION: TONY2 0x0042bd49
void __fastcall SpeechPrepareChannel(struct SpeechChannel* p_channel, TonyS32 p_data, void* p_buffer, TonyS32 p_size)
{
	SpeechFill(p_channel, 0, 0x190);
	p_channel->m_lastLag = 0x28;
	p_channel->m_source = p_data;
	p_channel->m_window = p_buffer;
	p_channel->m_windowSize = p_size;
	p_channel->m_drainLeft = 0;

	if (p_data == 0) {
		p_channel->m_decodeState = 0;
	}
	else {
		p_channel->m_decodeState = 1;
	}

	p_channel->m_stageTick = 0;
	p_channel->m_headerLow = 0;
	p_channel->m_sourceLeft = 0;
	p_channel->m_frameCount = 0;
	p_channel->m_bitCursor = 0;
	p_channel->m_stageWritePos = 0;
	p_channel->m_stageReadPos = 0;
	p_channel->m_windowWritePos = 0;
	p_channel->m_windowReadPos = 0;
	p_channel->m_decodeBudget = 2;
	p_channel->m_stagePending = 0;
}

// Per-tick channel driver: latches freshly staged data, parses the block header,
// stages the next chunk, decodes frames into the output window and pads silence at
// the end of the stream.
// FUNCTION: TONY2 0x0042be47
void __fastcall SpeechDriveChannel(struct SpeechChannel* p_voice)
{
	TonyS32 budget;
	TonyS32 chunk;

	budget = p_voice->m_decodeBudget;

	if (p_voice->m_stagePending) {
		p_voice->m_stagePending = 0;

		if (p_voice->m_stageWritePos == 0) {
			p_voice->m_frameCount = (TonyU16) (SpeechSwapDword(*(TonyS32*) &p_voice->m_stageRing[4]) >> 0x10 & 0xffff);
			p_voice->m_headerLow = (TonyU16) (SpeechSwapDword(*(TonyS32*) &p_voice->m_stageRing[4]) & 0xffff);
			p_voice->m_sourceLeft = SpeechSwapDword(*(TonyS32*) &p_voice->m_stageRing[8]) & 0xffffff;
			p_voice->m_bitCursor = 0x60;
		}

		p_voice->m_stageWritePos += 0x400;

		if (p_voice->m_sourceLeft > 0x100) {
			p_voice->m_sourceLeft -= 0x100;
		}
		else {
			p_voice->m_sourceLeft = 0;
		}

		p_voice->m_stageTick = 0;
	}

	if (p_voice->m_decodeState == 1 || p_voice->m_decodeState == 2 || p_voice->m_decodeState == 3) {
		if (p_voice->m_stageWritePos - p_voice->m_stageReadPos <= 0xc00) {
			if (p_voice->m_stageWritePos == 0) {
				chunk = 0x400;
			}
			else {
				chunk = p_voice->m_sourceLeft > 0x100 ? 0x400 : p_voice->m_sourceLeft << 2;
			}

			if (chunk > 0) {
				SpeechRingCopy((TonyS32*) p_voice->m_stageRing + (p_voice->m_stageWritePos % 0x1000 >> 2),
					p_voice->m_source + p_voice->m_stageWritePos, chunk);
				p_voice->m_stagePending = 1;
			}
		}
	}

	while (p_voice->m_windowWritePos - p_voice->m_windowReadPos < (TonyU32) (p_voice->m_windowSize - 0xa0) &&
		   (p_voice->m_decodeState == 1 || p_voice->m_frameCount > 0) &&
		   (p_voice->m_stageWritePos > 0 && p_voice->m_sourceLeft <= 0 ||
			   p_voice->m_stageWritePos - p_voice->m_stageReadPos > 0x24) &&
		   budget > 0) {
		GsmDecodeStreamFrame(p_voice, (TonyS32*) p_voice->m_window + (p_voice->m_windowWritePos % p_voice->m_windowSize >> 1));
		p_voice->m_stageReadPos = (TonyU32) p_voice->m_bitCursor >> 3 & ~3;
		p_voice->m_windowWritePos += 0xa0;
		p_voice->m_frameCount--;

		if (p_voice->m_frameCount == 0) {
			p_voice->m_drainLeft = p_voice->m_windowSize;
		}

		budget--;
	}

	if (p_voice->m_decodeState == 3 || p_voice->m_decodeState == 4) {
		if (p_voice->m_frameCount <= 0) {
			while (p_voice->m_windowWritePos - p_voice->m_windowReadPos < (TonyU32) (p_voice->m_windowSize - 0xa0) &&
				   budget > 0) {
				SpeechFill((TonyS32*) p_voice->m_window + (p_voice->m_windowWritePos % p_voice->m_windowSize >> 1), 0,
					0x140);
				p_voice->m_windowWritePos += 0xa0;
				budget--;
			}
		}
	}

	switch (p_voice->m_decodeState) {
	case 1:
		if (p_voice->m_windowWritePos - p_voice->m_windowReadPos >= (TonyU32) (p_voice->m_windowSize - 0x140)) {
			p_voice->m_decodeState = 2;
		}
		break;
	}
}

// Consumes up to p_bytes from the channel's decode window; returns the step taken.
// Byte-swaps a big-endian stream dword.
// FUNCTION: TONY2 0x0042c243
TonyU32 __fastcall SpeechSwapDword(TonyU32 p_value)
{
	TonyU32 result;

	result = p_value >> 0x18;
	result |= p_value >> 8 & 0xff00;
	result |= p_value << 8 & 0xff0000;
	result |= p_value << 0x18;
	return result;
}

// FUNCTION: TONY2 0x0042c291
TonyS32 __fastcall SpeechConsumeWindow(struct SpeechChannel* p_voice, TonyS32 p_bytes)
{
	TonyS32 step;

	if (p_voice->m_decodeState == 3) {
		if (p_voice->m_drainLeft > 0) {
			p_voice->m_drainLeft -= p_bytes;

			if (p_voice->m_drainLeft <= 0) {
				p_voice->m_decodeState = 4;
				p_voice->m_drainLeft = 0;
			}
		}

		if (p_voice->m_windowWritePos - p_voice->m_windowReadPos > (TonyU32) p_bytes) {
			step = p_bytes;
		}
		else {
			step = p_voice->m_windowWritePos - p_voice->m_windowReadPos;
		}

		p_voice->m_windowReadPos += step;
		return step;
	}

	return 0;
}

// FUNCTION: TONY2 0x0042c348
void __fastcall SpeechResetDecoder(struct SpeechChannel* p_voice)
{
	if (p_voice->m_decodeState == 2) {
		p_voice->m_decodeState = 3;
	}
}

// FUNCTION: TONY2 0x0042c36c
void __fastcall SpeechFlushDecoder(struct SpeechChannel* p_voice)
{
	p_voice->m_decodeState = 4;
}

void __fastcall GsmRpeDequant(TonyS16 p_max, TonyS16 p_grid, TonyS16* p_xmc, TonyS16* p_ep);
void __fastcall GsmLongTermSynth(struct SpeechChannel* p_voice, TonyS16 p_lag, TonyS16 p_gain, TonyS16* p_erp);
void __fastcall GsmShortTermSynth(
	struct SpeechChannel* p_voice, TonyS16* p_rrp, TonyS32 p_count, TonyS16* p_in, TonyS16* p_out);

// GSM LAR decode and interpolation: dequantizes the eight log-area ratios into
// the current bank, then runs the short-term filter over the four interpolation
// regions (13/14/13/120 samples) blending the previous and current banks, and
// flips the bank index.
// FUNCTION: TONY2 0x0042c390
void __fastcall GsmDecodeLar(SpeechChannel* p_voice, TonyS16* p_larc, TonyS16* p_in, TonyS16* p_out)
{
	TonyS16* fall;
	TonyS16 ebb[8];
	TonyS32 seep;
	TonyS16* fade;

	fall = p_voice->m_larBanks[p_voice->m_larBankIndex];
	fade = p_voice->m_larBanks[p_voice->m_larBankIndex ^ 1];
	fall[0] = (TonyS16) ((p_larc[0] - 0x20 << 0xa) * 0x3333 + 0x4000 >> 0xf << 1);
	fall[1] = (TonyS16) ((p_larc[1] - 0x20 << 0xa) * 0x3333 + 0x4000 >> 0xf << 1);
	fall[2] = (TonyS16) (((p_larc[2] - 0x10 << 0xa) - 0x1000) * 0x3333 + 0x4000 >> 0xf << 1);
	fall[3] = (TonyS16) (((p_larc[3] - 0x10 << 0xa) + 0x1400) * 0x3333 + 0x4000 >> 0xf << 1);
	fall[4] = (TonyS16) (((p_larc[4] - 8 << 0xa) - 0xbc) * 0x4b17 + 0x4000 >> 0xf << 1);
	fall[5] = (TonyS16) (((p_larc[5] - 8 << 0xa) + 0xe00) * 0x4444 + 0x4000 >> 0xf << 1);
	fall[6] = (TonyS16) (((p_larc[6] - 4 << 0xa) + 0x2aa) * 0x7ade + 0x4000 >> 0xf << 1);
	fall[7] = (TonyS16) (((p_larc[7] - 4 << 0xa) + 0x8f0) * 0x740c + 0x4000 >> 0xf << 1);

	for (seep = 0; seep < 8; seep++) {
		ebb[seep] = (TonyS16) ((fade[seep] >> 2) + (fall[seep] >> 2) + (fade[seep] >> 1));
	}

	GsmShortTermSynth(p_voice, ebb, 0xd, p_in, p_out);

	for (seep = 0; seep < 8; seep++) {
		ebb[seep] = (TonyS16) ((fade[seep] >> 1) + (fall[seep] >> 1));
	}

	GsmShortTermSynth(p_voice, ebb, 0xe, p_in + 0xd, p_out + 0xd);

	for (seep = 0; seep < 8; seep++) {
		ebb[seep] = (TonyS16) ((fade[seep] >> 2) + (fall[seep] >> 2) + (fall[seep] >> 1));
	}

	GsmShortTermSynth(p_voice, ebb, 0xd, p_in + 0x1b, p_out + 0x1b);

	for (seep = 0; seep < 8; seep++) {
		ebb[seep] = fall[seep];
	}

	GsmShortTermSynth(p_voice, ebb, 0x78, p_in + 0x28, p_out + 0x28);
	p_voice->m_larBankIndex ^= 1;
}

// GSM short-term synthesis: converts the interpolated log-area ratios to
// reflection coefficients, then runs the 8-stage lattice filter over the block
// through the voice's state taps.
// FUNCTION: TONY2 0x0042c693
void __fastcall GsmShortTermSynth(SpeechChannel* p_voice, TonyS16* p_rrp, TonyS32 p_count, TonyS16* p_in,
	TonyS16* p_out)
{
	TonyS32 fall;
	TonyS32* ebb;
	TonyS32 seep;
	TonyS32 fade;

	ebb = p_voice->m_filterTaps;

	for (fade = 0; fade < 8; fade++) {
		seep = p_rrp[fade] < 0 ? -p_rrp[fade] : p_rrp[fade];
		seep = seep < 0x2b33 ? seep << 1 : (seep < 0x4e66 ? seep + 0x2b33 : (seep >> 2) + 0x6600);
		seep = seep > 0x7fff ? 0x7fff : seep;
		p_rrp[fade] = (TonyS16) (p_rrp[fade] < 0 ? -seep : seep);
	}

	for (fall = 0; fall < p_count; fall++) {
		seep = p_in[fall];

		for (fade = 7; fade >= 0; fade--) {
			seep -= p_rrp[fade] * ebb[fade] + 0x4000 >> 0xf;
			ebb[fade + 1] = ebb[fade] + (p_rrp[fade] * seep + 0x4000 >> 0xf);
		}

		ebb[0] = seep;
		p_out[fall] = (TonyS16) seep;
	}
}

// GSM frame assembly: for each of the four subframes dequantizes the RPE grid and
// adds the long-term prediction, then runs the short-term synthesis over the frame
// and applies the de-emphasis filter with 13-bit output truncation.
// FUNCTION: TONY2 0x0042c842
void __fastcall GsmAssembleFrame(SpeechChannel* p_voice, TonyS16* p_larc, TonyS16* p_lag, TonyS16* p_gain,
	TonyS16* p_grid, TonyS16* p_max, TonyS16* p_xmc, TonyS16* p_out)
{
	TonyS16 fall[0x28];
	TonyS32 ebb;
	TonyS32 seep;
	TonyS32 fade;

	for (fade = 0; fade <= 3; fade++, p_max++, p_gain++, p_lag++, p_grid++, p_xmc += 0xd) {
		GsmRpeDequant(*p_max, *p_grid, p_xmc, fall);
		GsmLongTermSynth(p_voice, *p_lag, *p_gain, fall);
	}

	GsmDecodeLar(p_voice, p_larc, p_voice->m_history, p_out);
	ebb = p_voice->m_deemph;

	for (fade = 0; fade < 0xa0; fade++) {
		seep = ebb * 0x6e14 + 0x4000 >> 0xf;
		ebb = p_out[fade] + seep;
		seep = ebb << 1;
		p_out[fade] = (TonyS16) (seep < -0x8000 ? -0x8000 : (seep > 0x7fff ? 0x7fff : seep & 0xfffffff8));
	}

	p_voice->m_deemph = (TonyS16) ebb;
}

// GSM long-term prediction gain quantizer levels (QLB).
// GLOBAL: TONY2 0x00458700
TonyS16 g_gsmQlbLevels[4] = {3277, 11469, 21299, 32767};

// GSM long-term synthesis: validates the decoded lag against the last one, shifts
// the reconstructed history down a subframe and adds the scaled prediction to the
// residual.
// FUNCTION: TONY2 0x0042c988
void __fastcall GsmLongTermSynth(SpeechChannel* p_voice, TonyS16 p_lag, TonyS16 p_gain, TonyS16* p_erp)
{
	TonyS16 fall;
	TonyS32 ebb;
	TonyS16 seep;
	TonyS32 fade;

	fade = p_lag >= 0x28 && p_lag <= 0x78 ? p_lag : (TonyS16) p_voice->m_lastLag;
	p_voice->m_lastLag = (TonyU16) fade;
	fall = g_gsmQlbLevels[p_gain];

	for (ebb = 0; ebb < 0x78; ebb++) {
		p_voice->m_history[ebb] = p_voice->m_history[ebb + 0x28];
	}

	for (ebb = 0; ebb < 0x28; ebb++) {
		seep = (TonyS16) (fall * p_voice->m_history[0x78 - fade + ebb] + 0x4000 >> 0xf);
		p_voice->m_history[ebb + 0x78] = (TonyS16) (p_erp[ebb] + seep);
	}
}

// GSM RPE inverse block-adaptive quantization: decodes the block maximum into
// exponent and mantissa, rescales the thirteen samples and spreads them onto the
// selected grid positions of the cleared subframe.
// FUNCTION: TONY2 0x0042ca75
void __fastcall GsmRpeDequant(TonyS16 p_max, TonyS16 p_grid, TonyS16* p_xmc, TonyS16* p_ep)
{
	TonyS32 fall;
	TonyS32 ebb;
	TonyS16 seep;
	TonyS16 fade;
	TonyS32 crest;
	TonyS16 gain;
	TonyS16 foam;

	switch (p_max) {
	case 0:
		gain = -4;
		foam = 7;
		break;
	case 1:
		gain = -3;
		foam = 7;
		break;
	case 2:
		gain = -2;
		foam = 3;
		break;
	case 3:
		gain = -2;
		foam = 7;
		break;
	case 4:
		gain = -1;
		foam = 1;
		break;
	case 5:
		gain = -1;
		foam = 3;
		break;
	case 6:
		gain = -1;
		foam = 5;
		break;
	case 7:
		gain = -1;
		foam = 7;
		break;
	default:
		gain = (TonyS16) (p_max - 8 >> 3);
		foam = (TonyS16) (p_max & 7);
		break;
	}

	ebb = (foam << 0xb) + 0x47ff;
	fade = (TonyS16) (6 - gain);
	seep = (TonyS16) (1 << fade - 1);

	for (crest = 0; crest < 0xd; crest++) {
		fall = (p_xmc[crest] << 0xd) - 0x7000;
		fall = fall * ebb + 0x4000 >> 0xf;
		fall = fall + seep >> fade;
		p_xmc[crest] = (TonyS16) fall;
	}

	for (crest = 0; crest <= 0x27; crest++) {
		p_ep[crest] = 0;
	}

	for (crest = 0; crest <= 0xc; crest++) {
		p_ep[p_grid + crest * 3] = p_xmc[crest];
	}
}

// GSM frame unpack: splits the 33-byte frame into the eight LAR codes and the four
// subframes' lag/gain/grid/maximum/sample fields, then hands them to the frame
// assembly.
// FUNCTION: TONY2 0x0042cc3b
TonyS32 __fastcall GsmUnpackFrame(SpeechChannel* p_voice, TonyU8* p_frame, TonyS16* p_out)
{
	TonyS16 fall[4];
	TonyS16 ebb[0x34];
	TonyS32 seep;
	TonyS16 fade[8];
	TonyS16 crest[4];
	TonyS16 gain[4];
	TonyS16 foam[4];

	fade[0] = (TonyS16) ((p_frame[0] & 0xf) << 2 | (p_frame[1] >> 6 & 3));
	fade[1] = (TonyS16) (p_frame[1] & 0x3f);
	fade[2] = (TonyS16) (p_frame[2] >> 3 & 0x1f);
	fade[3] = (TonyS16) ((p_frame[2] & 7) << 2 | (p_frame[3] >> 6 & 3));
	fade[4] = (TonyS16) (p_frame[3] >> 2 & 0xf);
	fade[5] = (TonyS16) ((p_frame[3] & 3) << 2 | (p_frame[4] >> 6 & 3));
	fade[6] = (TonyS16) (p_frame[4] >> 3 & 7);
	fade[7] = (TonyS16) (p_frame[4] & 7);

	for (seep = 0; seep < 4; seep++) {
		foam[seep] = (TonyS16) (p_frame[seep * 7 + 5] >> 1 & 0x7f);
		fall[seep] = (TonyS16) ((p_frame[seep * 7 + 5] & 1) << 1 | (p_frame[seep * 7 + 6] >> 7 & 1));
		crest[seep] = (TonyS16) (p_frame[seep * 7 + 6] >> 5 & 3);
		gain[seep] = (TonyS16) ((p_frame[seep * 7 + 6] & 0x1f) << 1 | (p_frame[seep * 7 + 7] >> 7 & 1));
		ebb[seep * 0xd] = (TonyS16) (p_frame[seep * 7 + 7] >> 4 & 7);
		ebb[seep * 0xd + 1] = (TonyS16) (p_frame[seep * 7 + 7] >> 1 & 7);
		ebb[seep * 0xd + 2] = (TonyS16) ((p_frame[seep * 7 + 7] & 1) << 2 | (p_frame[seep * 7 + 8] >> 6 & 3));
		ebb[seep * 0xd + 3] = (TonyS16) (p_frame[seep * 7 + 8] >> 3 & 7);
		ebb[seep * 0xd + 4] = (TonyS16) (p_frame[seep * 7 + 8] & 7);
		ebb[seep * 0xd + 5] = (TonyS16) (p_frame[seep * 7 + 9] >> 5 & 7);
		ebb[seep * 0xd + 6] = (TonyS16) (p_frame[seep * 7 + 9] >> 2 & 7);
		ebb[seep * 0xd + 7] = (TonyS16) ((p_frame[seep * 7 + 9] & 3) << 1 | (p_frame[seep * 7 + 0xa] >> 7 & 1));
		ebb[seep * 0xd + 8] = (TonyS16) (p_frame[seep * 7 + 0xa] >> 4 & 7);
		ebb[seep * 0xd + 9] = (TonyS16) (p_frame[seep * 7 + 0xa] >> 1 & 7);
		ebb[seep * 0xd + 0xa] = (TonyS16) ((p_frame[seep * 7 + 0xa] & 1) << 2 | (p_frame[seep * 7 + 0xb] >> 6 & 3));
		ebb[seep * 0xd + 0xb] = (TonyS16) (p_frame[seep * 7 + 0xb] >> 3 & 7);
		ebb[seep * 0xd + 0xc] = (TonyS16) (p_frame[seep * 7 + 0xb] & 7);
	}

	GsmAssembleFrame(p_voice, fade, foam, fall, crest, gain, ebb, p_out);
	return 0;
}

// GSM stream frame decoder, bit-reader variant: pulls the frame fields directly
// from the byteswapped dword ring through an inline bit cache, handling the
// voiced/silence run header, then assembles the frame like GsmUnpackFrame.
// FUNCTION: TONY2 0x0042d053
TonyS32 __fastcall GsmDecodeStreamFrame(SpeechChannel* p_voice, TonyS32* p_out)
{
	TonyS32 fall;
	TonyS16 ebb[4];
	TonyS32 rise;
	TonyU32 swell;
	TonyS32 seep;
	TonyS32 mire;
	TonyS16 lull[0x34];
	TonyS16 fade[8];
	TonyS16 crest[4];
	TonyS16 surge[4];
	TonyU32* mist;
	TonyS16 gain[4];

	mist = (TonyU32*) p_voice->m_stageRing;
	rise = 0x3ff;
	seep = p_voice->m_bitCursor;
	swell = GsmSwapDword(mist[seep >> 5 & rise]) >> (seep & 0x1f);
	fall = 0x20 - (seep & 0x1f);

	if (p_voice->m_voicedRun == 0 && p_voice->m_silenceRun == 0) {
		if (fall > 1) {
			mire = (swell & 0x1);
			swell >>= 1;
			fall -= 1;
		}
		else {
			mire = (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 1) {
				mire |= (swell & ((1 << (1 - fall)) - 1)) << fall;
			}

			swell >>= 1 - fall;
			fall += 0x1f;
		}

		if (mire != 0) {
			if (fall > 4) {
				p_voice->m_silenceRun = (TonyS16) (swell & 0xf);
				swell >>= 4;
				fall -= 4;
			}
			else {
				p_voice->m_silenceRun = (TonyS16) (swell & ((1 << fall) - 1));
				seep += 0x20;
				swell = GsmSwapDword(mist[seep >> 5 & rise]);

				if (fall != 4) {
					p_voice->m_silenceRun |= (swell & ((1 << (4 - fall)) - 1)) << fall;
				}

				swell >>= 4 - fall;
				fall += 0x1c;
			}

			p_voice->m_silenceRun += 1;
		}
		else {
			if (fall > 7) {
				p_voice->m_voicedRun = (TonyS16) (swell & 0x7f);
				swell >>= 7;
				fall -= 7;
			}
			else {
				p_voice->m_voicedRun = (TonyS16) (swell & ((1 << fall) - 1));
				seep += 0x20;
				swell = GsmSwapDword(mist[seep >> 5 & rise]);

				if (fall != 7) {
					p_voice->m_voicedRun |= (swell & ((1 << (7 - fall)) - 1)) << fall;
				}

				swell >>= 7 - fall;
				fall += 0x19;
			}

			p_voice->m_voicedRun += 1;
		}
	}

	if (p_voice->m_silenceRun != 0) {
		GsmZeroFill((char*) p_out, 0x140);
		p_voice->m_silenceRun -= 1;
		goto done;
	}

	if (fall > 6) {
		fade[0] = (TonyS16) (swell & 0x3f);
		swell >>= 6;
		fall -= 6;
	}
	else {
		fade[0] = (TonyS16) (swell & ((1 << fall) - 1));
		seep += 0x20;
		swell = GsmSwapDword(mist[seep >> 5 & rise]);

		if (fall != 6) {
			fade[0] |= (swell & ((1 << (6 - fall)) - 1)) << fall;
		}

		swell >>= 6 - fall;
		fall += 0x1a;
	}

	if (fall > 6) {
		fade[1] = (TonyS16) (swell & 0x3f);
		swell >>= 6;
		fall -= 6;
	}
	else {
		fade[1] = (TonyS16) (swell & ((1 << fall) - 1));
		seep += 0x20;
		swell = GsmSwapDword(mist[seep >> 5 & rise]);

		if (fall != 6) {
			fade[1] |= (swell & ((1 << (6 - fall)) - 1)) << fall;
		}

		swell >>= 6 - fall;
		fall += 0x1a;
	}

	if (fall > 5) {
		fade[2] = (TonyS16) (swell & 0x1f);
		swell >>= 5;
		fall -= 5;
	}
	else {
		fade[2] = (TonyS16) (swell & ((1 << fall) - 1));
		seep += 0x20;
		swell = GsmSwapDword(mist[seep >> 5 & rise]);

		if (fall != 5) {
			fade[2] |= (swell & ((1 << (5 - fall)) - 1)) << fall;
		}

		swell >>= 5 - fall;
		fall += 0x1b;
	}

	if (fall > 5) {
		fade[3] = (TonyS16) (swell & 0x1f);
		swell >>= 5;
		fall -= 5;
	}
	else {
		fade[3] = (TonyS16) (swell & ((1 << fall) - 1));
		seep += 0x20;
		swell = GsmSwapDword(mist[seep >> 5 & rise]);

		if (fall != 5) {
			fade[3] |= (swell & ((1 << (5 - fall)) - 1)) << fall;
		}

		swell >>= 5 - fall;
		fall += 0x1b;
	}

	if (fall > 4) {
		fade[4] = (TonyS16) (swell & 0xf);
		swell >>= 4;
		fall -= 4;
	}
	else {
		fade[4] = (TonyS16) (swell & ((1 << fall) - 1));
		seep += 0x20;
		swell = GsmSwapDword(mist[seep >> 5 & rise]);

		if (fall != 4) {
			fade[4] |= (swell & ((1 << (4 - fall)) - 1)) << fall;
		}

		swell >>= 4 - fall;
		fall += 0x1c;
	}

	if (fall > 4) {
		fade[5] = (TonyS16) (swell & 0xf);
		swell >>= 4;
		fall -= 4;
	}
	else {
		fade[5] = (TonyS16) (swell & ((1 << fall) - 1));
		seep += 0x20;
		swell = GsmSwapDword(mist[seep >> 5 & rise]);

		if (fall != 4) {
			fade[5] |= (swell & ((1 << (4 - fall)) - 1)) << fall;
		}

		swell >>= 4 - fall;
		fall += 0x1c;
	}

	if (fall > 3) {
		fade[6] = (TonyS16) (swell & 0x7);
		swell >>= 3;
		fall -= 3;
	}
	else {
		fade[6] = (TonyS16) (swell & ((1 << fall) - 1));
		seep += 0x20;
		swell = GsmSwapDword(mist[seep >> 5 & rise]);

		if (fall != 3) {
			fade[6] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
		}

		swell >>= 3 - fall;
		fall += 0x1d;
	}

	if (fall > 3) {
		fade[7] = (TonyS16) (swell & 0x7);
		swell >>= 3;
		fall -= 3;
	}
	else {
		fade[7] = (TonyS16) (swell & ((1 << fall) - 1));
		seep += 0x20;
		swell = GsmSwapDword(mist[seep >> 5 & rise]);

		if (fall != 3) {
			fade[7] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
		}

		swell >>= 3 - fall;
		fall += 0x1d;
	}

	for (mire = 0; mire < 4; mire++) {
		if (fall > 7) {
			gain[mire] = (TonyS16) (swell & 0x7f);
			swell >>= 7;
			fall -= 7;
		}
		else {
			gain[mire] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 7) {
				gain[mire] |= (swell & ((1 << (7 - fall)) - 1)) << fall;
			}

			swell >>= 7 - fall;
			fall += 0x19;
		}

		if (fall > 2) {
			ebb[mire] = (TonyS16) (swell & 0x3);
			swell >>= 2;
			fall -= 2;
		}
		else {
			ebb[mire] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 2) {
				ebb[mire] |= (swell & ((1 << (2 - fall)) - 1)) << fall;
			}

			swell >>= 2 - fall;
			fall += 0x1e;
		}

		if (fall > 2) {
			crest[mire] = (TonyS16) (swell & 0x3);
			swell >>= 2;
			fall -= 2;
		}
		else {
			crest[mire] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 2) {
				crest[mire] |= (swell & ((1 << (2 - fall)) - 1)) << fall;
			}

			swell >>= 2 - fall;
			fall += 0x1e;
		}

		if (fall > 6) {
			surge[mire] = (TonyS16) (swell & 0x3f);
			swell >>= 6;
			fall -= 6;
		}
		else {
			surge[mire] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 6) {
				surge[mire] |= (swell & ((1 << (6 - fall)) - 1)) << fall;
			}

			swell >>= 6 - fall;
			fall += 0x1a;
		}

		if (fall > 3) {
			lull[mire * 0xd] = (TonyS16) (swell & 0x7);
			swell >>= 3;
			fall -= 3;
		}
		else {
			lull[mire * 0xd] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 3) {
				lull[mire * 0xd] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
			}

			swell >>= 3 - fall;
			fall += 0x1d;
		}

		if (fall > 3) {
			lull[mire * 0xd + 1] = (TonyS16) (swell & 0x7);
			swell >>= 3;
			fall -= 3;
		}
		else {
			lull[mire * 0xd + 1] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 3) {
				lull[mire * 0xd + 1] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
			}

			swell >>= 3 - fall;
			fall += 0x1d;
		}

		if (fall > 3) {
			lull[mire * 0xd + 2] = (TonyS16) (swell & 0x7);
			swell >>= 3;
			fall -= 3;
		}
		else {
			lull[mire * 0xd + 2] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 3) {
				lull[mire * 0xd + 2] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
			}

			swell >>= 3 - fall;
			fall += 0x1d;
		}

		if (fall > 3) {
			lull[mire * 0xd + 3] = (TonyS16) (swell & 0x7);
			swell >>= 3;
			fall -= 3;
		}
		else {
			lull[mire * 0xd + 3] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 3) {
				lull[mire * 0xd + 3] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
			}

			swell >>= 3 - fall;
			fall += 0x1d;
		}

		if (fall > 3) {
			lull[mire * 0xd + 4] = (TonyS16) (swell & 0x7);
			swell >>= 3;
			fall -= 3;
		}
		else {
			lull[mire * 0xd + 4] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 3) {
				lull[mire * 0xd + 4] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
			}

			swell >>= 3 - fall;
			fall += 0x1d;
		}

		if (fall > 3) {
			lull[mire * 0xd + 5] = (TonyS16) (swell & 0x7);
			swell >>= 3;
			fall -= 3;
		}
		else {
			lull[mire * 0xd + 5] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 3) {
				lull[mire * 0xd + 5] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
			}

			swell >>= 3 - fall;
			fall += 0x1d;
		}

		if (fall > 3) {
			lull[mire * 0xd + 6] = (TonyS16) (swell & 0x7);
			swell >>= 3;
			fall -= 3;
		}
		else {
			lull[mire * 0xd + 6] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 3) {
				lull[mire * 0xd + 6] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
			}

			swell >>= 3 - fall;
			fall += 0x1d;
		}

		if (fall > 3) {
			lull[mire * 0xd + 7] = (TonyS16) (swell & 0x7);
			swell >>= 3;
			fall -= 3;
		}
		else {
			lull[mire * 0xd + 7] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 3) {
				lull[mire * 0xd + 7] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
			}

			swell >>= 3 - fall;
			fall += 0x1d;
		}

		if (fall > 3) {
			lull[mire * 0xd + 8] = (TonyS16) (swell & 0x7);
			swell >>= 3;
			fall -= 3;
		}
		else {
			lull[mire * 0xd + 8] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 3) {
				lull[mire * 0xd + 8] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
			}

			swell >>= 3 - fall;
			fall += 0x1d;
		}

		if (fall > 3) {
			lull[mire * 0xd + 9] = (TonyS16) (swell & 0x7);
			swell >>= 3;
			fall -= 3;
		}
		else {
			lull[mire * 0xd + 9] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 3) {
				lull[mire * 0xd + 9] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
			}

			swell >>= 3 - fall;
			fall += 0x1d;
		}

		if (fall > 3) {
			lull[mire * 0xd + 10] = (TonyS16) (swell & 0x7);
			swell >>= 3;
			fall -= 3;
		}
		else {
			lull[mire * 0xd + 10] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 3) {
				lull[mire * 0xd + 10] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
			}

			swell >>= 3 - fall;
			fall += 0x1d;
		}

		if (fall > 3) {
			lull[mire * 0xd + 11] = (TonyS16) (swell & 0x7);
			swell >>= 3;
			fall -= 3;
		}
		else {
			lull[mire * 0xd + 11] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 3) {
				lull[mire * 0xd + 11] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
			}

			swell >>= 3 - fall;
			fall += 0x1d;
		}

		if (fall > 3) {
			lull[mire * 0xd + 12] = (TonyS16) (swell & 0x7);
			swell >>= 3;
			fall -= 3;
		}
		else {
			lull[mire * 0xd + 12] = (TonyS16) (swell & ((1 << fall) - 1));
			seep += 0x20;
			swell = GsmSwapDword(mist[seep >> 5 & rise]);

			if (fall != 3) {
				lull[mire * 0xd + 12] |= (swell & ((1 << (3 - fall)) - 1)) << fall;
			}

			swell >>= 3 - fall;
			fall += 0x1d;
		}
	}

	p_voice->m_voicedRun -= 1;
	GsmAssembleFrame(p_voice, fade, gain, ebb, crest, surge, lull, p_out);

done:
	p_voice->m_bitCursor = (seep & 0xffffffe0) + (0x20 - fall);
	return 0;
}

// Zero-fills a decoder buffer.
// FUNCTION: TONY2 0x0042e793
void __fastcall GsmZeroFill(char* p_dest, TonyS32 p_count)
{
	char* fall;

	for (fall = p_dest; p_count != 0; p_count--) {
		*fall++ = 0;
	}
}

// Byte-swaps a big-endian stream dword (voice-bank sibling of SpeechSwapDword).
// FUNCTION: TONY2 0x0042e7cb
TonyU32 __fastcall GsmSwapDword(TonyU32 p_value)
{
	TonyU32 result;

	result = p_value >> 0x18;
	result |= p_value >> 8 & 0xff00;
	result |= p_value << 8 & 0xff0000;
	result |= p_value << 0x18;
	return result;
}
