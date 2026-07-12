// DirectSound driver translation unit of the third-party sound library (the 0x429xxx
// band). Split out like soundwave.c: its functions declare int channel parameters
// masked & 0xff while the dispatch TU calls them through byte-typed prototypes.
// Compiled with VC6 RTM at /Od like the rest of the band.

#include "decomp.h"
#include "types.h"

// DirectSound channel state.
// SIZE 0x70
typedef struct DSndChannel {
	TonyU8 m_busy;                     // 0x00
	TonyU8 m_envStage;                 // 0x01
	TonyU8 m_gate;                     // 0x02
	TonyU8 m_volReset;                 // 0x03
	TonyS32 m_source;                  // 0x04
	TonyU32 m_position;                // 0x08
	TonyS32 m_posFrac;                 // 0x0c
	TonyU32 m_loopStart;               // 0x10
	TonyU32 m_endPos;                  // 0x14
	TonyU32 m_loopLength;              // 0x18
	TonyS32 m_step;                    // 0x1c
	TonyS16 m_vol0;                    // 0x20
	TonyS16 m_vol1;                    // 0x22
	TonyS16 m_vol2;                    // 0x24
	TonyS16 m_vol3;                    // 0x26
	TonyS32 m_prevVol0;                // 0x28
	TonyS32 m_prevVol1;                // 0x2c
	TonyS32 m_prevVol2;                // 0x30
	TonyS32 m_prevVol3;                // 0x34
	TonyU16 m_attackTime;              // 0x38
	TonyU16 m_decayTime;               // 0x3a
	TonyS32 m_sustainLevel;            // 0x3c
	TonyU16 m_releaseTime;             // 0x40
	undefined m_pad0[0x44 - 0x42];     // 0x42
	TonyS32 m_envLevel;                // 0x44
	TonyS32 m_releaseLevel;            // 0x48
	TonyU32 m_envAccum;                // 0x4c
	TonyFloat m_resampleState[4];         // 0x50
	undefined m_resampleTail[0x70 - 0x60]; // 0x60
} DSndChannel;

// The DirectSound channels.
// GLOBAL: TONY2 0x004ed9e0
DSndChannel g_dsndChannels[0x40];

// Resampler state head; the float mixer cores save these four dwords back to the
// channel, the fixed-point cores the full eight (ResampleState).
// SIZE 0x10
typedef struct ResampleStateHead {
	TonyS32 m_tail0; // 0x00
	TonyS32 m_tail1; // 0x04
	TonyS32 m_tail2; // 0x08
	TonyS32 m_tail3; // 0x0c
} ResampleStateHead;

// SIZE 0x20
typedef struct ResampleState {
	TonyS32 m_tail0; // 0x00
	TonyS32 m_tail1; // 0x04
	TonyS32 m_tail2; // 0x08
	TonyS32 m_tail3; // 0x0c
	TonyS32 m_tail4; // 0x10
	TonyS32 m_tail5; // 0x14
	TonyS32 m_tail6; // 0x18
	TonyS32 m_tail7; // 0x1c
} ResampleState;

// Per-channel mix job handed to the inner mixer cores: four volume tracks with
// per-quarter interpolation points and shifted steps (the float cores reinterpret
// the first two groups as floats), the source window and resample cursor, and the
// resampler state the cores save back. Built on the pump's stack frame.
// SIZE 0xd0
typedef struct MixJob {
	TonyS32 m_vol0[4];       // 0x00
	TonyS32 m_vol1[4];       // 0x10
	TonyS32 m_vol2[4];       // 0x20
	TonyS32 m_vol3[4];       // 0x30
	TonyS32 m_volStep0[4];   // 0x40
	TonyS32 m_volStep1[4];   // 0x50
	TonyS32 m_volStep2[4];   // 0x60
	TonyS32 m_volStep3[4];   // 0x70
	void* m_source;          // 0x80
	char* m_dryRowA;         // 0x84
	char* m_dryRowB;         // 0x88
	char* m_wetRow;          // 0x8c
	char* m_delayRow;        // 0x90
	TonyU32 m_windowEnd;     // 0x94
	TonyU32 m_windowStart;   // 0x98
	TonyU32 m_stepInt;       // 0x9c
	TonyU32 m_stepFrac;      // 0xa0
	TonyU32 m_position;      // 0xa4
	TonyS32 m_posFrac;       // 0xa8
	TonyS32 m_frames;        // 0xac
	ResampleState m_state;  // 0xb0
} MixJob;

void __fastcall StreamFillBytes(char* p_dest, TonyU8 p_value, TonyU32 p_count);
void __cdecl MixCoreMmx(struct MixJob* p_job);
void __cdecl MixPreambleFixed(TonyS32 p_frames);
void __cdecl MixPreambleFloat(TonyS32 p_frames);
void MixCarryDecayFixed(void);
void MixCarryDecayFloat(void);
void __cdecl MixCoreFloatStereoInterp(struct MixJob* p_job);
void __cdecl MixCoreFloatStereoNearest(struct MixJob* p_job);
void __cdecl MixCoreFloatMonoNearest(struct MixJob* p_job);
void __cdecl MixCoreFloatMonoInterp(struct MixJob* p_job);
TonyU8 WaveHasMmx(void);
void* __cdecl malloc(unsigned int p_size);
void __cdecl free(void* p_memory);
TonyS32 __cdecl DSndFixedLerp(TonyS32 p_frac, TonyS32 p_from, TonyS32 p_to);
void __fastcall DSndCarryAdd(TonyS32* p_accum, TonyS16 p_delta);

// Output rate global of the wave TU (annotated in soundwave.c).
extern TonyS32 g_waveOutputRate;

// pmaddwd pair-sum coefficients of the fixed-point mixer core.
// GLOBAL: TONY2 0x004582cc
TonyS16 g_mixPairSumCoeffs[4] = {1, 1, 1, 1};

// Q15 interpolation coefficient ROM expanded into g_mixInterpTableFloat at open.
// GLOBAL: TONY2 0x004582d4
TonyS16 g_mixInterpRomQ15[0x200] = {
	3129, 26285, 3398, -33, 2873, 26262, 3679, -40, 2628, 26217, 3971, -48,
	2394, 26150, 4276, -56, 2173, 26061, 4592, -65, 1963, 25950, 4920, -74,
	1764, 25817, 5260, -84, 1576, 25663, 5611, -95, 1399, 25487, 5974, -106,
	1233, 25291, 6347, -118, 1077, 25075, 6732, -130, 932, 24838, 7127, -143,
	796, 24583, 7532, -156, 671, 24309, 7947, -170, 554, 24016, 8371, -184,
	446, 23706, 8804, -198, 347, 23379, 9246, -212, 257, 23036, 9696, -226,
	174, 22678, 10153, -240, 99, 22304, 10618, -254, 31, 21917, 11088, -268,
	-30, 21517, 11564, -280, -84, 21104, 12045, -293, -132, 20679, 12531, -304,
	-173, 20244, 13020, -314, -210, 19799, 13512, -323, -241, 19345, 14006, -330,
	-267, 18882, 14501, -336, -289, 18413, 14997, -340, -306, 17937, 15493, -341,
	-320, 17456, 15988, -340, -330, 16970, 16480, -337, -337, 16480, 16970, -330,
	-340, 15988, 17456, -320, -341, 15493, 17937, -306, -340, 14997, 18413, -289,
	-336, 14501, 18882, -267, -330, 14006, 19345, -241, -323, 13512, 19799, -210,
	-314, 13020, 20244, -173, -304, 12531, 20679, -132, -293, 12045, 21104, -84,
	-280, 11564, 21517, -30, -268, 11088, 21917, 31, -254, 10618, 22304, 99,
	-240, 10153, 22678, 174, -226, 9696, 23036, 257, -212, 9246, 23379, 347,
	-198, 8804, 23706, 446, -184, 8371, 24016, 554, -170, 7947, 24309, 671,
	-156, 7532, 24583, 796, -143, 7127, 24838, 932, -130, 6732, 25075, 1077,
	-118, 6347, 25291, 1233, -106, 5974, 25487, 1399, -95, 5611, 25663, 1576,
	-84, 5260, 25817, 1764, -74, 4920, 25950, 1963, -65, 4592, 26061, 2173,
	-56, 4276, 26150, 2394, -48, 3971, 26217, 2628, -40, 3679, 26262, 2873,
	-33, 3398, 26285, 3129, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

// Output format code table indexed by the low format bits.
// GLOBAL: TONY2 0x004586d4
TonyU8 g_dsndFormatCodes[4] = {0, 1, 2, 3};

// Carry decay factor of the float resampler service.
// GLOBAL: TONY2 0x004586dc
TonyFloat g_dsndCarryDecay = 0.97f;

// Saturation threshold compared against the float bit pattern with the sign masked.
// GLOBAL: TONY2 0x004586d8
TonyFloat g_dsndSatLimit = 32767.0f;

// The expanded interpolation coefficients used by the resampler kernel.
// GLOBAL: TONY2 0x004ed1e0
TonyFloat g_mixInterpTableFloat[0x200];

// Output driver state: window handle/channel count, the mix bus pointers carved from
// the g_dsndBusBlock allocation, the ring block pointers and the driver bytes.
// GLOBAL: TONY2 0x004ef5e0
TonyU32 g_dsndChannelCount;

// GLOBAL: TONY2 0x004ef5e4
char* g_dsndDryRowA;

// GLOBAL: TONY2 0x004ef5e8
char* g_dsndDryRowB;

// GLOBAL: TONY2 0x004ef5ec
char* g_dsndWetRows[2];

// GLOBAL: TONY2 0x004ef5f4
char* g_dsndDelayRows[0x40];

// GLOBAL: TONY2 0x004ef6f4
char* g_dsndBusBlock;

// GLOBAL: TONY2 0x004ef6f8
TonyU8 g_dsndWetFlip;

// GLOBAL: TONY2 0x004ef6f9
TonyU8 g_dsndDelayCursor;

// GLOBAL: TONY2 0x004ef6fa
TonyU8 g_dsndDelayLength;

// GLOBAL: TONY2 0x004ef6fb
TonyU8 g_dsndFloatMode;

// GLOBAL: TONY2 0x004ef6fc
TonyU8 g_dsndMixMode;

// 16.16 envelope step added per pump tick.
// GLOBAL: TONY2 0x004ef700
TonyU32 g_dsndEnvStep;

// Resampler carry state cleared at open.
// GLOBAL: TONY2 0x004ef710
TonyS32 g_dsndCarryDryA;

// GLOBAL: TONY2 0x004ef714
TonyS32 g_dsndCarryDryB;

// GLOBAL: TONY2 0x004ef718
TonyS32 g_dsndCarryWet;

// GLOBAL: TONY2 0x004ef71c
TonyS32 g_dsndCarryDelay;

// Block length in frames for the output rings.
// GLOBAL: TONY2 0x004fab8c
TonyU32 g_dsndBlockFrames;

// Opens the mixer output: sizes the ring set, picks the frame stride from the format
// and MMX probe, allocates the bus block, carves the bus and ring pointers, clears the
// channel bank and expands the Q15 interpolation ROM. Returns 0 or error 3.
// Parked at 46%: every statement shape matches but the /Od register rotor runs two
// slots ahead from the format-table statement onward (orig eax/cl vs edx/al); the
// join-state spelling that reseeds it has not been found (matrix-tested: nested
// assignment, condition polarity, operand order, else placement).
// STUB: TONY2 0x004278fc
TonyU8 __fastcall DSndOpen(TonyU32 p_channels, TonyS32 p_rate, TonyU32 p_samples, TonyU32 p_depth, TonyU32 p_format)
{
	TonyU32 frames;
	TonyU32 stride;
	TonyU32 i;

	p_samples = g_dsndBlockFrames;

	if (p_depth != 0) {
		frames = (p_rate * 1000 + p_depth - 1) / p_depth;
		g_dsndDelayLength = (frames + p_samples - 1) / p_samples;
		g_dsndDelayLength = 0x40;
	}
	else {
		g_dsndDelayLength = 1;
	}

	g_dsndMixMode = g_dsndFormatCodes[p_format & 3];

	if (g_dsndMixMode == 0) {
		g_dsndFloatMode = WaveHasMmx() == 0;
	}
	else {
		g_dsndFloatMode = 1;
	}

	stride = g_dsndFloatMode != 0 ? 4 : 2;
	g_dsndBusBlock = malloc(stride * p_samples * (g_dsndDelayLength + 4));

	if (g_dsndBusBlock == NULL) {
		return 3;
	}

	g_waveOutputRate = p_rate;
	g_dsndDryRowA = g_dsndBusBlock;
	g_dsndDryRowB = g_dsndBusBlock + p_samples * stride;
	g_dsndWetRows[0] = g_dsndBusBlock + p_samples * stride * 2;
	g_dsndWetRows[1] = g_dsndBusBlock + p_samples * stride * 3;

	for (i = 0; i < g_dsndDelayLength; i++) {
		g_dsndDelayRows[i] = g_dsndBusBlock + (i + 4) * (p_samples * stride);
		StreamFillBytes(g_dsndDelayRows[i], 0, p_samples * stride);
	}

	g_dsndDelayCursor = 0;
	g_dsndChannelCount = p_channels;
	g_dsndWetFlip = 0;
	StreamFillBytes(g_dsndWetRows[0], 0, p_samples * stride);
	StreamFillBytes(g_dsndWetRows[1], 0, p_samples * stride);

	for (i = 0; i < g_dsndChannelCount; i++) {
		g_dsndChannels[i].m_busy = 0;
	}

	if (g_dsndFloatMode == 0) {
		g_dsndCarryDryA = 0;
		g_dsndCarryDryB = 0;
		g_dsndCarryWet = 0;
		g_dsndCarryDelay = 0;
	}
	else {
		g_dsndCarryDryA = 0;
		g_dsndCarryDryB = 0;
		g_dsndCarryWet = 0;
		g_dsndCarryDelay = 0;

		for (i = 0; i < 0x200; i++) {
			g_mixInterpTableFloat[i] = (TonyFloat) g_mixInterpRomQ15[i] * 3.0517578125e-05f;
		}
	}

	return 0;
}

// Releases the output bus block.
// FUNCTION: TONY2 0x00427b96
void DSndClose(void)
{
	free(g_dsndBusBlock);
}

// Interleaves the two output buses into 16-bit frames.
// FUNCTION: TONY2 0x00427ba9
void __fastcall DSndInterleaveOutput(TonyS16* p_dest, TonyU32 p_count)
{
	TonyS16* a;
	TonyS16* b;

	a = (TonyS16*) g_dsndDryRowB;
	b = (TonyS16*) g_dsndDryRowA;

	for (; p_count > 0; p_count--) {
		*p_dest = *a;
		p_dest++;
		a++;
		*p_dest = *b;
		p_dest++;
		b++;
	}
}

// Saturating output converter: streams both buses into interleaved 16-bit frames,
// fast-testing the masked float bit pattern against 32767.0f before each fistp; the
// hand-written core keeps the original's quirk of reading the first bus's sign when
// the second bus clips.
// FUNCTION: TONY2 0x00427c19
void __cdecl DSndSaturateOutput(TonyS16* p_dest, TonyU32 p_count)
{
	__asm {
		push esi
		push edi
		mov edi, dword ptr [g_dsndDryRowB]
		mov esi, dword ptr [g_dsndDryRowA]
		mov edx, p_dest
		mov ecx, p_count
	conv_loop:
		mov ebx, [edi]
		mov eax, [esi]
		add edi, 4
		add esi, 4
		and ebx, 0x7fffffff
		and eax, 0x7fffffff
		cmp ebx, dword ptr [g_dsndSatLimit]
		jg clamp_b
		fld dword ptr [edi - 4]
		add edx, 2
		fistp word ptr [edx - 2]
	chk_a:
		cmp eax, dword ptr [g_dsndSatLimit]
		jg clamp_a
		fld dword ptr [esi - 4]
		add edx, 2
		fistp word ptr [edx - 2]
	next:
		dec ecx
		jne conv_loop
		jmp conv_done
	clamp_a:
		test dword ptr [esi - 4], 0xffffffff
		jns pos_a
		mov word ptr [edx], 0x8000
		add edx, 2
		jmp next
	pos_a:
		mov word ptr [edx], 0x7fff
		add edx, 2
		jmp next
	clamp_b:
		test dword ptr [esi - 4], 0xffffffff
		jns pos_b
		mov word ptr [edx], 0x8000
		add edx, 2
		jmp chk_a
	pos_b:
		mov word ptr [edx], 0x7fff
		add edx, 2
		jmp chk_a
	conv_done:
		pop edi
		pop esi
	}
}

// Resets a DirectSound channel's stage and accumulators.
// FUNCTION: TONY2 0x00427cb0
void __fastcall DSndEnvReset(DSndChannel* p_channel)
{
	p_channel->m_envStage = 0;
	p_channel->m_envAccum = 0;
	p_channel->m_envLevel = 0;
}

// DS envelope pump: advances the channel's 16.16 accumulator through
// attack/decay/sustain/release, deriving the level via the fixed-point lerp; returns
// FALSE once the release has run out.
// FUNCTION: TONY2 0x00427cd6
TonyU8 __fastcall DSndEnvAdvance(TonyU8 p_channel)
{
	TonyU8 result;
	DSndChannel* channel;

	channel = &g_dsndChannels[p_channel];
	result = 1;

	switch (channel->m_envStage) {
	case 0:
		channel->m_envAccum += g_dsndEnvStep;

		if (channel->m_gate == 0) {
			goto release_start;
		}

		if (channel->m_envAccum >> 16 < channel->m_attackTime) {
			channel->m_envLevel = channel->m_envAccum / channel->m_attackTime;
		}
		else {
			channel->m_envStage = 1;
			channel->m_envAccum -= channel->m_attackTime << 16;
			goto decay_scan;
		}
		break;
	case 1:
		channel->m_envAccum += g_dsndEnvStep;

		if (channel->m_gate == 0) {
			goto release_start;
		}

	decay_scan:
		if (channel->m_envAccum >> 16 < channel->m_decayTime) {
			channel->m_envLevel = DSndFixedLerp(channel->m_envAccum / channel->m_decayTime, 0x10000, channel->m_sustainLevel);
		}
		else {
			channel->m_envStage = 2;
			channel->m_envLevel = channel->m_sustainLevel;
			goto sustain_check;
		}
		break;
	case 2:
	sustain_check:
		if (channel->m_gate == 0) {
		release_start:
			channel->m_envStage = 3;
			channel->m_envAccum = 0;
			channel->m_releaseLevel = channel->m_envLevel;
			goto release_scan;
		}
		break;
	case 3:
		channel->m_envAccum += g_dsndEnvStep;

	release_scan:
		if (channel->m_envAccum >> 16 < channel->m_releaseTime) {
			channel->m_envLevel = DSndFixedLerp(channel->m_envAccum / channel->m_releaseTime, channel->m_releaseLevel, 0);
		}
		else {
			channel->m_envLevel = 0;
			result = 0;
		}
	}

	return result;
}

// 16.16 fixed-point interpolation from p_from to p_to; the hand-written core keeps
// the full 64-bit product.
// FUNCTION: TONY2 0x00427ebe
TonyS32 __cdecl DSndFixedLerp(TonyS32 p_frac, TonyS32 p_from, TonyS32 p_to)
{
	TonyS32 result;

	__asm {
		mov eax, p_to
		sub eax, p_from
		imul dword ptr [p_frac]
		shr eax, 0x10
		shl edx, 0x10
		or eax, edx
		add eax, p_from
		mov result, eax
	}

	return result;
}

// Kicks a DirectSound channel into its release stage from the current level.
// FUNCTION: TONY2 0x00427ee6
void __fastcall DSndEnvRelease(TonyU8 p_channel)
{
	DSndChannel* channel;

	channel = &g_dsndChannels[p_channel];
	channel->m_envAccum = 0;
	channel->m_envStage = 3;
	channel->m_releaseLevel = channel->m_envLevel;
}

// DirectSound output mix for one block: derives the envelope tick step, renders and
// converts every busy channel into the ring set.
// The DirectSound pump: recomputes the envelope step, services the driver, then for
// every active channel interpolates the four volume tracks across the block (fixed
// point in the integer path, normalized floats in the FPU path), builds the mix job
// and runs the matching mixer core, saving the resample cursor and state back.
// FUNCTION: TONY2 0x00427f23
void __fastcall DSndMixBlock(TonyS32 p_frames, TonyS16* p_dest)
{
	MixJob seep;
	TonyU32 crest;
	TonyFloat surge;
	TonyU32 mist;
	TonyS32 gain;
	TonyS32 rise;
	TonyS32 swell;
	TonyS32 fade;
	TonyS32 wash;
	TonyU8 brine;
	TonyS32 ebb;
	TonyS32 mire;
	TonyS32 lull;
	TonyS32 foam;
	TonyS32 drift;
	TonyS32 tide;
	TonyS32 fall;

	g_dsndEnvStep = (TonyU32) (p_frames << 0x10) / g_waveOutputRate * 1000;

	if (!g_dsndFloatMode) {
		MixPreambleFixed(p_frames);
	}
	else {
		MixPreambleFloat(p_frames);
	}

	for (crest = 0; crest < g_dsndChannelCount; crest++) {
		if (g_dsndChannels[crest].m_busy) {
			brine = 1;
			seep.m_frames = p_frames;
			seep.m_dryRowA = g_dsndDryRowA;
			seep.m_dryRowB = g_dsndDryRowB;
			seep.m_wetRow = g_dsndWetRows[g_dsndWetFlip];
			seep.m_delayRow = g_dsndDelayRows[g_dsndDelayCursor];
			brine = DSndEnvAdvance((TonyU8) crest);

			gain = g_dsndChannels[crest].m_vol0 * g_dsndChannels[crest].m_envLevel;
			swell = g_dsndChannels[crest].m_vol1 * g_dsndChannels[crest].m_envLevel;
			rise = g_dsndChannels[crest].m_vol2 * g_dsndChannels[crest].m_envLevel;
			lull = g_dsndChannels[crest].m_vol3 * g_dsndChannels[crest].m_envLevel;

			if (g_dsndChannels[crest].m_volReset) {
				g_dsndChannels[crest].m_prevVol0 = gain;
				g_dsndChannels[crest].m_prevVol1 = swell;
				g_dsndChannels[crest].m_prevVol2 = rise;
				g_dsndChannels[crest].m_prevVol3 = lull;
			}

			if (!g_dsndFloatMode) {
				fade = g_dsndChannels[crest].m_prevVol0;
				mire = (gain - fade) / p_frames;
				tide = g_dsndChannels[crest].m_prevVol1;
				foam = (swell - tide) / p_frames;
				wash = g_dsndChannels[crest].m_prevVol2;
				drift = (rise - wash) / p_frames;
				ebb = g_dsndChannels[crest].m_prevVol3;
				fall = (lull - ebb) / p_frames;

				for (mist = 0; mist < 4; mist++) {
					seep.m_vol0[mist] = fade;
					seep.m_vol1[mist] = tide;
					seep.m_vol2[mist] = wash;
					seep.m_vol3[mist] = ebb;
					seep.m_volStep0[mist] = mire << 2;
					seep.m_volStep1[mist] = foam << 2;
					seep.m_volStep2[mist] = drift << 2;
					seep.m_volStep3[mist] = fall << 2;
					fade += mire;
					tide += foam;
					wash += drift;
					ebb += fall;
				}
			}
			else {
				surge = 4.0f / ((TonyU32) p_frames * 2147483648.0f);
				fade = g_dsndChannels[crest].m_prevVol0;
				*(TonyFloat*) &seep.m_vol1[0] = (gain - fade) * surge;
				tide = g_dsndChannels[crest].m_prevVol1;
				*(TonyFloat*) &seep.m_vol1[1] = (swell - tide) * surge;
				wash = g_dsndChannels[crest].m_prevVol2;
				*(TonyFloat*) &seep.m_vol1[2] = (rise - wash) * surge;
				ebb = g_dsndChannels[crest].m_prevVol3;
				*(TonyFloat*) &seep.m_vol1[3] = (lull - ebb) * surge;
				*(TonyFloat*) &seep.m_vol0[0] = fade * 4.656612873077393e-10f;
				*(TonyFloat*) &seep.m_vol0[1] = tide * 4.656612873077393e-10f;
				*(TonyFloat*) &seep.m_vol0[2] = wash * 4.656612873077393e-10f;
				*(TonyFloat*) &seep.m_vol0[3] = ebb * 4.656612873077393e-10f;
			}

			g_dsndChannels[crest].m_prevVol0 = gain;
			g_dsndChannels[crest].m_prevVol1 = swell;
			g_dsndChannels[crest].m_prevVol2 = rise;
			g_dsndChannels[crest].m_prevVol3 = lull;

			if (g_dsndChannels[crest].m_loopLength == 0 ||
				(!g_dsndChannels[crest].m_gate &&
					g_dsndChannels[crest].m_endPos >
						g_dsndChannels[crest].m_loopStart + g_dsndChannels[crest].m_loopLength)) {
				seep.m_windowEnd = g_dsndChannels[crest].m_endPos + 8;
				seep.m_windowStart = g_dsndChannels[crest].m_endPos;

				if (g_dsndChannels[crest].m_position + p_frames >= g_dsndChannels[crest].m_endPos) {
					brine = 0;
				}
			}
			else {
				seep.m_windowEnd = g_dsndChannels[crest].m_loopStart + g_dsndChannels[crest].m_loopLength;
				seep.m_windowStart = g_dsndChannels[crest].m_loopStart;
			}

			seep.m_source = g_dsndChannels[crest].m_source;
			seep.m_stepInt = (TonyU32) g_dsndChannels[crest].m_step >> 0x10;
			seep.m_stepFrac = g_dsndChannels[crest].m_step << 0x10;
			seep.m_position = g_dsndChannels[crest].m_position;
			seep.m_posFrac = g_dsndChannels[crest].m_posFrac;

			if (!g_dsndFloatMode) {
				MixCoreMmx(&seep);
				*(ResampleState*) g_dsndChannels[crest].m_resampleState = seep.m_state;
			}
			else {
				switch (g_dsndMixMode) {
				case 0:
					MixCoreFloatStereoInterp(&seep);
					break;
				case 1:
					MixCoreFloatMonoInterp(&seep);
					seep.m_state.m_tail2 = seep.m_state.m_tail3 = 0;
					break;
				case 2:
					MixCoreFloatStereoNearest(&seep);
					break;
				case 3:
					MixCoreFloatMonoNearest(&seep);
					seep.m_state.m_tail2 = seep.m_state.m_tail3 = 0;
					break;
				}

				*(ResampleStateHead*) g_dsndChannels[crest].m_resampleState = *(ResampleStateHead*) &seep.m_state;
			}

			g_dsndChannels[crest].m_position = seep.m_position;
			g_dsndChannels[crest].m_posFrac = seep.m_posFrac;
			g_dsndChannels[crest].m_volReset = 0;
			g_dsndChannels[crest].m_busy = brine;
		}
	}

	if (!g_dsndFloatMode) {
		MixCarryDecayFixed();
		DSndInterleaveOutput(p_dest, p_frames);
	}
	else {
		MixCarryDecayFloat();
		DSndSaturateOutput(p_dest, p_frames);
	}

	g_dsndWetFlip ^= 1;
	g_dsndDelayCursor = (g_dsndDelayCursor + 1) % g_dsndDelayLength;
}

// Fixed-point MMX mixer core: cubic-interpolates four source samples per output
// frame through the Q15 ROM (wrapping the resample cursor at the window end),
// advances the four volume tracks, scales and saturate-mixes into the two dry and
// two wet rows, saving the last packed samples as the resampler state. Hand-written
// in the original.
// FUNCTION: TONY2 0x00428759
void __cdecl MixCoreMmx(MixJob* p_job)
{
	__asm {
		push esi
		push edi
		push ebp
		mov esi, dword ptr [p_job]
		mov ebx, dword ptr [esi + 0xa8]
		mov edi, dword ptr [esi + 0xa4]
		mov ebp, dword ptr [esi + 0xac]
		shr ebp, 2
	top:
		mov eax, dword ptr [esi + 0x80]
		mov ecx, ebx
		movq mm0, qword ptr [eax + edi*2]
		shr ecx, 0x1a
		add ebx, dword ptr [esi + 0xa0]
		pmaddwd mm0, qword ptr [ecx*8 + g_mixInterpRomQ15]
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		jns wrap0
	cont0:
		psrad mm0, 0xf
		mov ecx, ebx
		movq mm1, qword ptr [eax + edi*2]
		shr ecx, 0x1a
		add ebx, dword ptr [esi + 0xa0]
		pmaddwd mm1, qword ptr [ecx*8 + g_mixInterpRomQ15]
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		jns wrap1
	cont1:
		psrad mm1, 0xf
		packssdw mm0, mm1
		pmaddwd mm0, qword ptr [g_mixPairSumCoeffs]
		mov ecx, ebx
		movq mm2, qword ptr [eax + edi*2]
		shr ecx, 0x1a
		add ebx, dword ptr [esi + 0xa0]
		pmaddwd mm2, qword ptr [ecx*8 + g_mixInterpRomQ15]
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		jns wrap2
	cont2:
		psrad mm2, 0xf
		mov ecx, ebx
		movq mm3, qword ptr [eax + edi*2]
		shr ecx, 0x1a
		add ebx, dword ptr [esi + 0xa0]
		pmaddwd mm3, qword ptr [ecx*8 + g_mixInterpRomQ15]
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		jns wrap3
	cont3:
		psrad mm3, 0xf
		packssdw mm2, mm3
		pmaddwd mm2, qword ptr [g_mixPairSumCoeffs]
		movq mm4, qword ptr [esi]
		movq mm5, qword ptr [esi + 4]
		movq mm6, qword ptr [esi + 0x10]
		movq mm7, qword ptr [esi + 0x14]
		packssdw mm0, mm2
		movq mm1, mm4
		movq mm3, mm5
		psrad mm1, 0x10
		psrad mm3, 0x10
		packssdw mm1, mm3
		movq mm2, mm6
		movq mm3, mm7
		psrad mm2, 0x10
		psrad mm3, 0x10
		packssdw mm2, mm3
		paddd mm4, qword ptr [esi + 0x40]
		paddd mm5, qword ptr [esi + 0x44]
		paddd mm6, qword ptr [esi + 0x50]
		paddd mm7, qword ptr [esi + 0x54]
		movq qword ptr [esi], mm4
		movq qword ptr [esi + 4], mm5
		movq qword ptr [esi + 0x10], mm6
		movq qword ptr [esi + 0x14], mm7
		movq mm4, mm0
		movq mm5, mm0
		pmullw mm4, mm1
		pmulhw mm1, mm0
		pmullw mm5, mm2
		pmulhw mm2, mm0
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x88]
		movq mm3, mm4
		punpcklwd mm4, mm1
		punpckhwd mm3, mm1
		psrad mm4, 0xf
		psrad mm3, 0xf
		packssdw mm4, mm3
		add eax, 8
		add edx, 8
		movq mm6, mm5
		punpcklwd mm5, mm2
		punpckhwd mm6, mm2
		psrad mm5, 0xf
		psrad mm6, 0xf
		packssdw mm5, mm6
		movq qword ptr [esi + 0xb0], mm4
		movq qword ptr [esi + 0xb8], mm5
		paddsw mm4, qword ptr [eax - 8]
		paddsw mm5, qword ptr [edx - 8]
		movq qword ptr [eax - 8], mm4
		movq qword ptr [edx - 8], mm5
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x88], edx
		movq mm4, qword ptr [esi + 0x20]
		movq mm5, qword ptr [esi + 0x24]
		movq mm6, qword ptr [esi + 0x30]
		movq mm7, qword ptr [esi + 0x34]
		movq mm1, mm4
		movq mm3, mm5
		psrad mm1, 0x10
		psrad mm3, 0x10
		packssdw mm1, mm3
		movq mm2, mm6
		movq mm3, mm7
		psrad mm2, 0x10
		psrad mm3, 0x10
		packssdw mm2, mm3
		paddd mm4, qword ptr [esi + 0x60]
		paddd mm5, qword ptr [esi + 0x64]
		paddd mm6, qword ptr [esi + 0x70]
		paddd mm7, qword ptr [esi + 0x74]
		movq qword ptr [esi + 0x20], mm4
		movq qword ptr [esi + 0x24], mm5
		movq qword ptr [esi + 0x30], mm6
		movq qword ptr [esi + 0x34], mm7
		movq mm4, mm0
		movq mm5, mm0
		pmullw mm4, mm1
		pmulhw mm1, mm0
		pmullw mm5, mm2
		pmulhw mm2, mm0
		mov eax, dword ptr [esi + 0x8c]
		mov edx, dword ptr [esi + 0x90]
		movq mm3, mm4
		punpcklwd mm4, mm1
		punpckhwd mm3, mm1
		psrad mm4, 0xf
		psrad mm3, 0xf
		packssdw mm4, mm3
		add eax, 8
		add edx, 8
		movq mm6, mm5
		punpcklwd mm5, mm2
		punpckhwd mm6, mm2
		psrad mm5, 0xf
		psrad mm6, 0xf
		packssdw mm5, mm6
		movq qword ptr [esi + 0xc0], mm4
		movq qword ptr [esi + 0xc8], mm5
		paddsw mm4, qword ptr [eax - 8]
		paddsw mm5, qword ptr [edx - 8]
		movq qword ptr [eax - 8], mm4
		movq qword ptr [edx - 8], mm5
		mov dword ptr [esi + 0x8c], eax
		mov dword ptr [esi + 0x90], edx
		dec ebp
		jne top
		mov dword ptr [esi + 0xa8], ebx
		mov dword ptr [esi + 0xa4], edi
		jmp done
	wrap0:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp cont0
	wrap1:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp cont1
	wrap2:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp cont2
	wrap3:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp cont3
	done:
		emms
		pop ebp
		pop edi
		pop esi
	}
}

// Fixed-point block preamble: clamps the four carries to 16 bits, splats each into
// a four-lane word pack, then an MMX loop seeds the dry rows from the opposite wet
// ring row (saturating sub/add) and refills both wet rows with the carry packs.
// FUNCTION: TONY2 0x00428a34
void __cdecl MixPreambleFixed(TonyS32 p_frames)
{
	TonyS16 fall[4];
	TonyFloat* ebb;
	TonyS16 seep[4];
	TonyFloat* rise;
	TonyS16 swell[4];
	TonyS16 mire[4];
	TonyFloat* lull;

	rise = (TonyFloat*) g_dsndDelayRows[g_dsndDelayCursor];
	ebb = (TonyFloat*) g_dsndWetRows[g_dsndWetFlip];
	lull = (TonyFloat*) g_dsndWetRows[g_dsndWetFlip ^ 1];

	mire[0] = (TonyS16) (g_dsndCarryDryA > 0x7fff ? 0x7fff : (g_dsndCarryDryA < -0x8000 ? -0x8000 : g_dsndCarryDryA));
	mire[1] = mire[0];
	mire[2] = mire[1];
	mire[3] = mire[2];
	fall[0] = (TonyS16) (g_dsndCarryDryB > 0x7fff ? 0x7fff : (g_dsndCarryDryB < -0x8000 ? -0x8000 : g_dsndCarryDryB));
	fall[1] = fall[0];
	fall[2] = fall[1];
	fall[3] = fall[2];
	swell[0] = (TonyS16) (g_dsndCarryWet > 0x7fff ? 0x7fff : (g_dsndCarryWet < -0x8000 ? -0x8000 : g_dsndCarryWet));
	swell[1] = swell[0];
	swell[2] = swell[1];
	swell[3] = swell[2];
	seep[0] = (TonyS16) (g_dsndCarryDelay > 0x7fff ? 0x7fff : (g_dsndCarryDelay < -0x8000 ? -0x8000 : g_dsndCarryDelay));
	seep[1] = seep[0];
	seep[2] = seep[1];
	seep[3] = seep[2];

	__asm {
		push esi
		push edi
		mov ecx, dword ptr [p_frames]
		shr ecx, 2
		mov eax, offset g_dsndChannelCount
		movq mm4, qword ptr [mire]
		movq mm5, qword ptr [fall]
		movq mm6, qword ptr [swell]
		movq mm7, qword ptr [seep]
		mov ebx, dword ptr [eax + 4]
		mov edx, dword ptr [eax + 8]
		mov esi, dword ptr [lull]
		mov edi, dword ptr [ebb]
		mov eax, dword ptr [rise]
	pump:
		movq mm0, qword ptr [esi]
		movq mm1, mm4
		movq mm2, mm5
		psubsw mm1, mm0
		add esi, 8
		paddsw mm2, mm0
		add ebx, 8
		movq qword ptr [ebx - 8], mm1
		add edx, 8
		movq qword ptr [edx - 8], mm2
		add edi, 8
		movq qword ptr [edi - 8], mm6
		add eax, 8
		movq qword ptr [eax - 8], mm7
		loop pump
		emms
		pop edi
		pop esi
	}
}

// Float-mode block preamble: seeds the dry output rows from the opposite wet ring
// row minus/plus the carries, and refills both wet rows with the carry values.
// FUNCTION: TONY2 0x00428c3c
void __cdecl MixPreambleFloat(TonyS32 p_frames)
{
	TonyFloat fall;
	TonyFloat* ebb;
	TonyFloat rise;
	TonyFloat swell;
	TonyFloat* seep;
	TonyU32 mire;
	TonyFloat* lull;
	TonyFloat fade;
	TonyFloat* crest;
	TonyFloat* surge;
	TonyFloat mist;

	surge = (TonyFloat*) g_dsndDryRowA;
	seep = (TonyFloat*) g_dsndDryRowB;
	lull = (TonyFloat*) g_dsndDelayRows[g_dsndDelayCursor];
	ebb = (TonyFloat*) g_dsndWetRows[g_dsndWetFlip];
	crest = (TonyFloat*) g_dsndWetRows[g_dsndWetFlip ^ 1];
	mist = *(TonyFloat*) &g_dsndCarryDryA;
	fall = *(TonyFloat*) &g_dsndCarryDryB;
	rise = *(TonyFloat*) &g_dsndCarryWet;
	fade = *(TonyFloat*) &g_dsndCarryDelay;

	for (mire = 0; mire < p_frames; mire++) {
		*surge++ = mist - (swell = *crest);
		crest++;
		*seep++ = fall + swell;
		*ebb++ = rise;
		*lull++ = fade;
	}
}

// Fixed-point resampler carry decay, hand-written in the original: scales the four
// carry accumulators by 0xf850/0x10000 through a widening multiply.
// FUNCTION: TONY2 0x00428d29
void MixCarryDecayFixed(void)
{
	__asm {
		mov ebx, 0xf850
		mov ecx, offset g_dsndCarryDryA
		mov eax, dword ptr [ecx]
		imul ebx
		shr eax, 0x10
		shl edx, 0x10
		or eax, edx
		mov dword ptr [ecx], eax
		mov eax, dword ptr [ecx + 4]
		imul ebx
		shr eax, 0x10
		shl edx, 0x10
		or eax, edx
		mov dword ptr [ecx + 4], eax
		mov eax, dword ptr [ecx + 8]
		imul ebx
		shr eax, 0x10
		shl edx, 0x10
		or eax, edx
		mov dword ptr [ecx + 8], eax
		mov eax, dword ptr [ecx + 0xc]
		imul ebx
		shr eax, 0x10
		shl edx, 0x10
		or eax, edx
		mov dword ptr [ecx + 0xc], eax
	}
}

// Float resampler carry decay: scales the four carries by 0.97 and rounds them back
// to integral floats, hand-scheduled on the FPU stack.
// FUNCTION: TONY2 0x00428d7c
void MixCarryDecayFloat(void)
{
	__asm {
		fld dword ptr [g_dsndCarryDecay]
		fld dword ptr [g_dsndCarryDryA]
		fld dword ptr [g_dsndCarryDryB]
		fld dword ptr [g_dsndCarryWet]
		fld dword ptr [g_dsndCarryDelay]
		fmul st, st(4)
		frndint
		fstp dword ptr [g_dsndCarryDelay]
		fmul st, st(3)
		frndint
		fstp dword ptr [g_dsndCarryWet]
		fmul st, st(2)
		frndint
		fstp dword ptr [g_dsndCarryDryB]
		fmulp st(1), st
		frndint
		fstp dword ptr [g_dsndCarryDryA]
	}
}

// DirectSound channel busy query.
// FUNCTION: TONY2 0x00428dcd
TonyU8 __fastcall DSndChannelBusy(TonyS32 p_channel)
{
	return g_dsndChannels[p_channel & 0xff].m_busy;
}

// Prepares a DirectSound channel from a playback descriptor; p_reset also rearms the
// resampler state.
// FUNCTION: TONY2 0x00428de9
void __fastcall DSndChannelPrepare(TonyS32 p_channel, TonyS32* p_desc, TonyS32 p_reset)
{
	if (p_reset & 0xff) {
		g_dsndChannels[p_channel & 0xff].m_attackTime = 0;
		g_dsndChannels[p_channel & 0xff].m_decayTime = 0;
		g_dsndChannels[p_channel & 0xff].m_sustainLevel = 0x10000;
		g_dsndChannels[p_channel & 0xff].m_releaseTime = 0x28;
	}

	g_dsndChannels[p_channel & 0xff].m_source = p_desc[1];
	g_dsndChannels[p_channel & 0xff].m_position = p_desc[2];
	g_dsndChannels[p_channel & 0xff].m_posFrac = 0;
	g_dsndChannels[p_channel & 0xff].m_loopStart = p_desc[4];
	g_dsndChannels[p_channel & 0xff].m_endPos = p_desc[3];
	g_dsndChannels[p_channel & 0xff].m_loopLength = p_desc[5];
}

// Sets a DirectSound channel's 12.4 step from a 16-bit rate, capped at 4x.
// FUNCTION: TONY2 0x00428ee6
void __fastcall DSndChannelSetStep(TonyS32 p_channel, TonyS32 p_rate)
{
	if ((p_rate & 0xffff) > 0x4000) {
		g_dsndChannels[p_channel & 0xff].m_step = 0x40000;
	}
	else {
		g_dsndChannels[p_channel & 0xff].m_step = (p_rate & 0xffff) << 4;
	}
}

// Starts a DirectSound channel: gates the envelope on, resets its stage and
// accumulators, marks it busy and clears the resampler carry block.
// FUNCTION: TONY2 0x00428f3a
void __fastcall DSndChannelStart(TonyS32 p_channel)
{
	g_dsndChannels[p_channel & 0xff].m_gate = 1;
	DSndEnvReset(&g_dsndChannels[p_channel & 0xff]);
	g_dsndChannels[p_channel & 0xff].m_volReset = 1;
	g_dsndChannels[p_channel & 0xff].m_busy = 1;
	StreamFillBytes((char*) g_dsndChannels[p_channel & 0xff].m_resampleState, 0, 0x20);
}

// Stops a DirectSound channel, folding its resampler carries into the output state:
// saturating word residuals in the integer path, plain float sums in the float path.
// FUNCTION: TONY2 0x00428faf
void __fastcall DSndChannelStop(TonyS32 p_channel)
{
	if (g_dsndFloatMode == 0) {
		DSndCarryAdd(&g_dsndCarryDryA, ((TonyU16*) g_dsndChannels[p_channel & 0xff].m_resampleState)[3]);
		DSndCarryAdd(&g_dsndCarryDryB, ((TonyU16*) g_dsndChannels[p_channel & 0xff].m_resampleState)[7]);
		DSndCarryAdd(&g_dsndCarryWet, ((TonyU16*) g_dsndChannels[p_channel & 0xff].m_resampleState)[11]);
		DSndCarryAdd(&g_dsndCarryDelay, ((TonyU16*) g_dsndChannels[p_channel & 0xff].m_resampleState)[15]);
	}
	else {
		*(TonyFloat*) &g_dsndCarryDryA += g_dsndChannels[p_channel & 0xff].m_resampleState[0];
		*(TonyFloat*) &g_dsndCarryDryB += g_dsndChannels[p_channel & 0xff].m_resampleState[1];
		*(TonyFloat*) &g_dsndCarryWet += g_dsndChannels[p_channel & 0xff].m_resampleState[2];
		*(TonyFloat*) &g_dsndCarryDelay += g_dsndChannels[p_channel & 0xff].m_resampleState[3];
	}

	g_dsndChannels[p_channel & 0xff].m_busy = 0;
}

// Adds a signed word residual into an accumulator with saturation at both rails.
// FUNCTION: TONY2 0x004290c3
void __fastcall DSndCarryAdd(TonyS32* p_accum, TonyS16 p_delta)
{
	if (p_delta < 0) {
		if (*p_accum < 0) {
			*p_accum += p_delta;

			if (*p_accum > 0) {
				*p_accum = 0x80000000;
			}
		}
		else {
			*p_accum += p_delta;
		}
	}
	else {
		if (*p_accum > 0) {
			*p_accum += p_delta;

			if (*p_accum < 0) {
				*p_accum = 0x7fffffff;
			}
		}
		else {
			*p_accum += p_delta;
		}
	}
}

// Clears a DirectSound channel's busy flag.
// FUNCTION: TONY2 0x00429154
void __fastcall DSndChannelKill(TonyS32 p_channel)
{
	g_dsndChannels[p_channel & 0xff].m_busy = 0;
}

// Stores the four mixer levels for a DirectSound channel.
// FUNCTION: TONY2 0x00429171
void __fastcall DSndChannelSetVolumes(TonyS32 p_channel, TonyU16 p_a, TonyU16 p_b, TonyU16 p_c, TonyU16 p_d)
{
	g_dsndChannels[p_channel & 0xff].m_vol0 = p_a;
	g_dsndChannels[p_channel & 0xff].m_vol1 = p_b;
	g_dsndChannels[p_channel & 0xff].m_vol2 = p_c;
	g_dsndChannels[p_channel & 0xff].m_vol3 = p_d;
}

// Installs a word-packed envelope descriptor (attack/decay words, Q12 sustain
// fraction, release word).
// FUNCTION: TONY2 0x004291de
void __fastcall DSndChannelSetAdsr(TonyS32 p_channel, TonyU16* p_state)
{
	g_dsndChannels[p_channel & 0xff].m_attackTime = p_state[0];
	g_dsndChannels[p_channel & 0xff].m_decayTime = p_state[1];
	g_dsndChannels[p_channel & 0xff].m_sustainLevel = p_state[2] << 4;
	g_dsndChannels[p_channel & 0xff].m_releaseTime = p_state[3];
}

// Sets a DirectSound channel's envelope release time word (0x11 = quick stop).
// FUNCTION: TONY2 0x00429256
void __fastcall DSndChannelSetRelease(TonyS32 p_channel, TonyU16 p_command)
{
	g_dsndChannels[p_channel & 0xff].m_releaseTime = p_command;
}

// FUNCTION: TONY2 0x0042927d
void __fastcall DSndChannelGateOff(TonyS32 p_channel)
{
	g_dsndChannels[p_channel & 0xff].m_gate = 0;
}

// FUNCTION: TONY2 0x0042929a
TonyS32 __fastcall DSndChannelPosition(TonyS32 p_channel)
{
	return g_dsndChannels[p_channel & 0xff].m_position;
}

// Float mixer core, stereo interpolated variant. Hand-written in the original.
// FUNCTION: TONY2 0x004292c0
void __cdecl MixCoreFloatStereoInterp(MixJob* p_job)
{
	__asm {
		push esi
		push edi
		push ebp
		mov esi, dword ptr [p_job]
		mov ebx, dword ptr [esi + 0xa8]
		mov edi, dword ptr [esi + 0xa4]
		mov ebp, dword ptr [esi + 0xac]
		fld dword ptr [esi + 0xc]
		fld dword ptr [esi + 8]
		fld dword ptr [esi + 4]
		fld dword ptr [esi]
		shr ebp, 2
	lbl_4292ec:
		mov eax, dword ptr [esi + 0x80]
		mov ecx, ebx
		fild word ptr [eax + edi*2]
		fild word ptr [eax + edi*2 + 2]
		fild word ptr [eax + edi*2 + 4]
		shr ecx, 0x1a
		fild word ptr [eax + edi*2 + 6]
		shl ecx, 4
		add ebx, dword ptr [esi + 0xa0]
		fmul dword ptr [ecx + g_mixInterpTableFloat + 0xc]
		fxch st(1)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 8]
		fxch st(2)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 4]
		fxch st(3)
		fmul dword ptr [ecx + g_mixInterpTableFloat]
		fxch st(2)
		faddp st(1), st
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		faddp st(1), st
		jns lbl_42964d
	lbl_429347:
		faddp st(1), st
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x8c]
		fld st(0)
		fld st(0)
		fld st(0)
		fmul st, st(4)
		fxch st(3)
		fmul st, st(5)
		fxch st(2)
		fmul st, st(6)
		fxch st(1)
		fmul st, st(7)
		fxch st(3)
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x8c], edx
		mov eax, dword ptr [esi + 0x88]
		mov edx, dword ptr [esi + 0x90]
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x88], eax
		mov dword ptr [esi + 0x90], edx
		mov eax, dword ptr [esi + 0x80]
		mov ecx, ebx
		fild word ptr [eax + edi*2]
		fild word ptr [eax + edi*2 + 2]
		fild word ptr [eax + edi*2 + 4]
		shr ecx, 0x1a
		fild word ptr [eax + edi*2 + 6]
		shl ecx, 4
		add ebx, dword ptr [esi + 0xa0]
		fmul dword ptr [ecx + g_mixInterpTableFloat + 0xc]
		fxch st(1)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 8]
		fxch st(2)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 4]
		fxch st(3)
		fmul dword ptr [ecx + g_mixInterpTableFloat]
		fxch st(2)
		faddp st(1), st
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		faddp st(1), st
		jns lbl_42965a
	lbl_42940e:
		faddp st(1), st
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x8c]
		fld st(0)
		fld st(0)
		fld st(0)
		fmul st, st(4)
		fxch st(3)
		fmul st, st(5)
		fxch st(2)
		fmul st, st(6)
		fxch st(1)
		fmul st, st(7)
		fxch st(3)
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x8c], edx
		mov eax, dword ptr [esi + 0x88]
		mov edx, dword ptr [esi + 0x90]
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x88], eax
		mov dword ptr [esi + 0x90], edx
		mov eax, dword ptr [esi + 0x80]
		mov ecx, ebx
		fild word ptr [eax + edi*2]
		fild word ptr [eax + edi*2 + 2]
		fild word ptr [eax + edi*2 + 4]
		shr ecx, 0x1a
		fild word ptr [eax + edi*2 + 6]
		shl ecx, 4
		add ebx, dword ptr [esi + 0xa0]
		fmul dword ptr [ecx + g_mixInterpTableFloat + 0xc]
		fxch st(1)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 8]
		fxch st(2)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 4]
		fxch st(3)
		fmul dword ptr [ecx + g_mixInterpTableFloat]
		fxch st(2)
		faddp st(1), st
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		faddp st(1), st
		jns lbl_429667
	lbl_4294d5:
		faddp st(1), st
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x8c]
		fld st(0)
		fld st(0)
		fld st(0)
		fmul st, st(4)
		fxch st(3)
		fmul st, st(5)
		fxch st(2)
		fmul st, st(6)
		fxch st(1)
		fmul st, st(7)
		fxch st(3)
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x8c], edx
		mov eax, dword ptr [esi + 0x88]
		mov edx, dword ptr [esi + 0x90]
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x88], eax
		mov dword ptr [esi + 0x90], edx
		mov eax, dword ptr [esi + 0x80]
		mov ecx, ebx
		fild word ptr [eax + edi*2]
		fild word ptr [eax + edi*2 + 2]
		fild word ptr [eax + edi*2 + 4]
		shr ecx, 0x1a
		fild word ptr [eax + edi*2 + 6]
		shl ecx, 4
		add ebx, dword ptr [esi + 0xa0]
		fmul dword ptr [ecx + g_mixInterpTableFloat + 0xc]
		fxch st(1)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 8]
		fxch st(2)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 4]
		fxch st(3)
		fmul dword ptr [ecx + g_mixInterpTableFloat]
		fxch st(2)
		faddp st(1), st
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		faddp st(1), st
		jns lbl_429674
	lbl_42959c:
		faddp st(1), st
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x8c]
		fld st(0)
		fld st(0)
		fld st(0)
		fmul st, st(4)
		fxch st(3)
		fmul st, st(5)
		fxch st(2)
		fmul st, st(6)
		fxch st(1)
		fmul st, st(7)
		fxch st(3)
		fst dword ptr [esi + 0xb0]
		fadd dword ptr [eax]
		fxch st(1)
		fst dword ptr [esi + 0xb8]
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x8c], edx
		mov eax, dword ptr [esi + 0x88]
		mov edx, dword ptr [esi + 0x90]
		fst dword ptr [esi + 0xb4]
		fadd dword ptr [eax]
		fxch st(1)
		fst dword ptr [esi + 0xbc]
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x88], eax
		mov dword ptr [esi + 0x90], edx
		fadd dword ptr [esi + 0x10]
		fxch st(1)
		fadd dword ptr [esi + 0x14]
		fxch st(2)
		fadd dword ptr [esi + 0x18]
		fxch st(3)
		fadd dword ptr [esi + 0x1c]
		fxch st(3)
		fxch st(2)
		fxch st(1)
		dec ebp
		jne lbl_4292ec
		mov dword ptr [esi + 0xa8], ebx
		mov dword ptr [esi + 0xa4], edi
		jmp lbl_429681
	lbl_42964d:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp lbl_429347
	lbl_42965a:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp lbl_42940e
	lbl_429667:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp lbl_4294d5
	lbl_429674:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp lbl_42959c
	lbl_429681:
		fcompp
		fcompp
		pop ebp
		pop edi
		pop esi
	}
}

// Float mixer core, stereo nearest-sample variant. Hand-written in the original.
// FUNCTION: TONY2 0x0042968d
void __cdecl MixCoreFloatStereoNearest(MixJob* p_job)
{
	__asm {
		push esi
		push edi
		push ebp
		mov esi, dword ptr [p_job]
		mov ebx, dword ptr [esi + 0xa8]
		mov edi, dword ptr [esi + 0xa4]
		mov ebp, dword ptr [esi + 0xac]
		fld dword ptr [esi + 0xc]
		fld dword ptr [esi + 8]
		fld dword ptr [esi + 4]
		fld dword ptr [esi]
		shr ebp, 2
	lbl_4296b9:
		mov eax, dword ptr [esi + 0x80]
		fild word ptr [eax + edi*2 + 2]
		add ebx, dword ptr [esi + 0xa0]
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		jns lbl_429936
	lbl_4296dd:
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x8c]
		fld st(0)
		fld st(0)
		fld st(0)
		fmul st, st(4)
		fxch st(3)
		fmul st, st(5)
		fxch st(2)
		fmul st, st(6)
		fxch st(1)
		fmul st, st(7)
		fxch st(3)
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x8c], edx
		mov eax, dword ptr [esi + 0x88]
		mov edx, dword ptr [esi + 0x90]
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x88], eax
		mov dword ptr [esi + 0x90], edx
		mov eax, dword ptr [esi + 0x80]
		fild word ptr [eax + edi*2 + 2]
		add ebx, dword ptr [esi + 0xa0]
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		jns lbl_429943
	lbl_42976b:
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x8c]
		fld st(0)
		fld st(0)
		fld st(0)
		fmul st, st(4)
		fxch st(3)
		fmul st, st(5)
		fxch st(2)
		fmul st, st(6)
		fxch st(1)
		fmul st, st(7)
		fxch st(3)
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x8c], edx
		mov eax, dword ptr [esi + 0x88]
		mov edx, dword ptr [esi + 0x90]
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x88], eax
		mov dword ptr [esi + 0x90], edx
		mov eax, dword ptr [esi + 0x80]
		fild word ptr [eax + edi*2 + 2]
		add ebx, dword ptr [esi + 0xa0]
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		jns lbl_429950
	lbl_4297f9:
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x8c]
		fld st(0)
		fld st(0)
		fld st(0)
		fmul st, st(4)
		fxch st(3)
		fmul st, st(5)
		fxch st(2)
		fmul st, st(6)
		fxch st(1)
		fmul st, st(7)
		fxch st(3)
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x8c], edx
		mov eax, dword ptr [esi + 0x88]
		mov edx, dword ptr [esi + 0x90]
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x88], eax
		mov dword ptr [esi + 0x90], edx
		mov eax, dword ptr [esi + 0x80]
		fild word ptr [eax + edi*2 + 2]
		add ebx, dword ptr [esi + 0xa0]
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		jns lbl_42995d
	lbl_429887:
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x8c]
		fld st(0)
		fld st(0)
		fld st(0)
		fmul st, st(4)
		fxch st(3)
		fmul st, st(5)
		fxch st(2)
		fmul st, st(6)
		fxch st(1)
		fmul st, st(7)
		fxch st(3)
		fst dword ptr [esi + 0xb0]
		fadd dword ptr [eax]
		fxch st(1)
		fst dword ptr [esi + 0xb8]
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x8c], edx
		mov eax, dword ptr [esi + 0x88]
		mov edx, dword ptr [esi + 0x90]
		fst dword ptr [esi + 0xb4]
		fadd dword ptr [eax]
		fxch st(1)
		fst dword ptr [esi + 0xbc]
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x88], eax
		mov dword ptr [esi + 0x90], edx
		fadd dword ptr [esi + 0x10]
		fxch st(1)
		fadd dword ptr [esi + 0x14]
		fxch st(2)
		fadd dword ptr [esi + 0x18]
		fxch st(3)
		fadd dword ptr [esi + 0x1c]
		fxch st(3)
		fxch st(2)
		fxch st(1)
		dec ebp
		jne lbl_4296b9
		mov dword ptr [esi + 0xa8], ebx
		mov dword ptr [esi + 0xa4], edi
		jmp lbl_42996a
	lbl_429936:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp lbl_4296dd
	lbl_429943:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp lbl_42976b
	lbl_429950:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp lbl_4297f9
	lbl_42995d:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp lbl_429887
	lbl_42996a:
		fcompp
		fcompp
		pop ebp
		pop edi
		pop esi
	}
}

// Float mixer core, mono nearest-sample variant: pulls the high word of each source
// pair, scales by the two volume floats and accumulates into the dry rows, stepping
// the volumes per quarter and saving the last two scaled samples as state. Hand-
// written in the original.
// FUNCTION: TONY2 0x00429976
void __cdecl MixCoreFloatMonoNearest(MixJob* p_job)
{
	__asm {
		push esi
		push edi
		push ebp
		mov esi, dword ptr [p_job]
		mov ebx, dword ptr [esi + 0xa8]
		mov edi, dword ptr [esi + 0xa4]
		mov ebp, dword ptr [esi + 0xac]
		fld dword ptr [esi + 4]
		fld dword ptr [esi]
		shr ebp, 2
	top:
		mov eax, dword ptr [esi + 0x80]
		fild word ptr [eax + edi*2 + 2]
		add ebx, dword ptr [esi + 0xa0]
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		jns wrap0
	cont0:
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x88]
		fld st(0)
		fmul st, st(2)
		fxch st(1)
		fmul st, st(3)
		fxch st(1)
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x88], edx
		mov eax, dword ptr [esi + 0x80]
		fild word ptr [eax + edi*2 + 2]
		add ebx, dword ptr [esi + 0xa0]
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		jns wrap1
	cont1:
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x88]
		fld st(0)
		fmul st, st(2)
		fxch st(1)
		fmul st, st(3)
		fxch st(1)
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x88], edx
		mov eax, dword ptr [esi + 0x80]
		fild word ptr [eax + edi*2 + 2]
		add ebx, dword ptr [esi + 0xa0]
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		jns wrap2
	cont2:
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x88]
		fld st(0)
		fmul st, st(2)
		fxch st(1)
		fmul st, st(3)
		fxch st(1)
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x88], edx
		mov eax, dword ptr [esi + 0x80]
		fild word ptr [eax + edi*2 + 2]
		add ebx, dword ptr [esi + 0xa0]
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		jns wrap3
	cont3:
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x88]
		fld st(0)
		fmul st, st(2)
		fxch st(1)
		fmul st, st(3)
		fxch st(1)
		fst dword ptr [esi + 0xb0]
		fadd dword ptr [eax]
		fxch st(1)
		fst dword ptr [esi + 0xb4]
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x88], edx
		fadd dword ptr [esi + 0x10]
		fxch st(1)
		fadd dword ptr [esi + 0x14]
		fxch st(1)
		dec ebp
		jne top
		mov dword ptr [esi + 0xa8], ebx
		mov dword ptr [esi + 0xa4], edi
		jmp done
	wrap0:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp cont0
	wrap1:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp cont1
	wrap2:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp cont2
	wrap3:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp cont3
	done:
		fcompp
		pop ebp
		pop edi
		pop esi
	}
}

// Float mixer core, mono interpolated variant: cubic-interpolates each source
// sample through the expanded float coefficient table, scales by the two volume
// floats and accumulates into the dry rows. Hand-written in the original.
// FUNCTION: TONY2 0x00429b65
void __cdecl MixCoreFloatMonoInterp(MixJob* p_job)
{
	__asm {
		push esi
		push edi
		push ebp
		mov esi, dword ptr [p_job]
		mov ebx, dword ptr [esi + 0xa8]
		mov edi, dword ptr [esi + 0xa4]
		mov ebp, dword ptr [esi + 0xac]
		fld dword ptr [esi + 4]
		fld dword ptr [esi]
		shr ebp, 2
	lbl_429b8b:
		mov eax, dword ptr [esi + 0x80]
		mov ecx, ebx
		fild word ptr [eax + edi*2]
		fild word ptr [eax + edi*2 + 2]
		fild word ptr [eax + edi*2 + 4]
		shr ecx, 0x1a
		fild word ptr [eax + edi*2 + 6]
		shl ecx, 4
		add ebx, dword ptr [esi + 0xa0]
		fmul dword ptr [ecx + g_mixInterpTableFloat + 0xc]
		fxch st(1)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 8]
		fxch st(2)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 4]
		fxch st(3)
		fmul dword ptr [ecx + g_mixInterpTableFloat]
		fxch st(2)
		faddp st(1), st
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		faddp st(1), st
		jns lbl_429dfa
	lbl_429be6:
		faddp st(1), st
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x88]
		fld st(0)
		fmul st, st(2)
		fxch st(1)
		fmul st, st(3)
		fxch st(1)
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x88], edx
		mov eax, dword ptr [esi + 0x80]
		mov ecx, ebx
		fild word ptr [eax + edi*2]
		fild word ptr [eax + edi*2 + 2]
		fild word ptr [eax + edi*2 + 4]
		shr ecx, 0x1a
		fild word ptr [eax + edi*2 + 6]
		shl ecx, 4
		add ebx, dword ptr [esi + 0xa0]
		fmul dword ptr [ecx + g_mixInterpTableFloat + 0xc]
		fxch st(1)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 8]
		fxch st(2)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 4]
		fxch st(3)
		fmul dword ptr [ecx + g_mixInterpTableFloat]
		fxch st(2)
		faddp st(1), st
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		faddp st(1), st
		jns lbl_429e07
	lbl_429c77:
		faddp st(1), st
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x88]
		fld st(0)
		fmul st, st(2)
		fxch st(1)
		fmul st, st(3)
		fxch st(1)
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x88], edx
		mov eax, dword ptr [esi + 0x80]
		mov ecx, ebx
		fild word ptr [eax + edi*2]
		fild word ptr [eax + edi*2 + 2]
		fild word ptr [eax + edi*2 + 4]
		shr ecx, 0x1a
		fild word ptr [eax + edi*2 + 6]
		shl ecx, 4
		add ebx, dword ptr [esi + 0xa0]
		fmul dword ptr [ecx + g_mixInterpTableFloat + 0xc]
		fxch st(1)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 8]
		fxch st(2)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 4]
		fxch st(3)
		fmul dword ptr [ecx + g_mixInterpTableFloat]
		fxch st(2)
		faddp st(1), st
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		faddp st(1), st
		jns lbl_429e14
	lbl_429d08:
		faddp st(1), st
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x88]
		fld st(0)
		fmul st, st(2)
		fxch st(1)
		fmul st, st(3)
		fxch st(1)
		fadd dword ptr [eax]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x88], edx
		mov eax, dword ptr [esi + 0x80]
		mov ecx, ebx
		fild word ptr [eax + edi*2]
		fild word ptr [eax + edi*2 + 2]
		fild word ptr [eax + edi*2 + 4]
		shr ecx, 0x1a
		fild word ptr [eax + edi*2 + 6]
		shl ecx, 4
		add ebx, dword ptr [esi + 0xa0]
		fmul dword ptr [ecx + g_mixInterpTableFloat + 0xc]
		fxch st(1)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 8]
		fxch st(2)
		fmul dword ptr [ecx + g_mixInterpTableFloat + 4]
		fxch st(3)
		fmul dword ptr [ecx + g_mixInterpTableFloat]
		fxch st(2)
		faddp st(1), st
		adc edi, dword ptr [esi + 0x9c]
		mov ecx, edi
		sub ecx, dword ptr [esi + 0x94]
		faddp st(1), st
		jns lbl_429e21
	lbl_429d99:
		faddp st(1), st
		mov eax, dword ptr [esi + 0x84]
		mov edx, dword ptr [esi + 0x88]
		fld st(0)
		fmul st, st(2)
		fxch st(1)
		fmul st, st(3)
		fxch st(1)
		fst dword ptr [esi + 0xb0]
		fadd dword ptr [eax]
		fxch st(1)
		fst dword ptr [esi + 0xb4]
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [edx]
		add eax, 4
		add edx, 4
		mov dword ptr [esi + 0x84], eax
		mov dword ptr [esi + 0x88], edx
		fadd dword ptr [esi + 0x10]
		fxch st(1)
		fadd dword ptr [esi + 0x14]
		fxch st(1)
		dec ebp
		jne lbl_429b8b
		mov dword ptr [esi + 0xa8], ebx
		mov dword ptr [esi + 0xa4], edi
		jmp lbl_429e2e
	lbl_429dfa:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp lbl_429be6
	lbl_429e07:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp lbl_429c77
	lbl_429e14:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp lbl_429d08
	lbl_429e21:
		mov edi, ecx
		add edi, dword ptr [esi + 0x98]
		jmp lbl_429d99
	lbl_429e2e:
		fcompp
		pop ebp
		pop edi
		pop esi
	}
}
