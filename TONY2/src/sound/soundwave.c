// Wave/driver dispatch translation unit of the third-party sound library. Split from
// soundlib.c because the original library's TUs declare these functions with int
// parameters (masked & 0xff at use) while the mixer TU calls them through byte-typed
// prototypes. Compiled with VC6 RTM at /Od like the rest of the band.

#include "decomp.h"
#include "types.h"

struct WaveChannel;
struct SongDesc;
void __cdecl WaveChannelRelease(struct WaveChannel* p_channel);
void __fastcall DSndEnvRelease(TonyU8 p_channel);
void __cdecl WaveMixBlock(TonyS32 p_step, TonyS32 p_frames, TonyS32 p_dest);
TonyS32 __cdecl WaveRateToStep(TonyS32 p_rate);
TonyS32 __fastcall WaveDriverPosition(TonyU8 p_channel);
TonyS32 __fastcall DSndChannelPosition(TonyU8 p_channel);
void __fastcall DSndChannelSetStep(TonyU8 p_channel, TonyS32 p_step);
void __fastcall DSndChannelPrepare(TonyU8 p_channel, struct SongDesc* p_desc, TonyU8 p_flag);
void __fastcall DSndChannelSetVolumes(TonyU8 p_channel, TonyS32 p_a, TonyU16 p_b, TonyU16 p_c, TonyU16 p_d);
void __fastcall WaveCornerGains(TonyFloat* p_front, TonyFloat* p_back, TonyFloat* p_left, TonyFloat* p_right,
	TonyS32 p_level, TonyS32 p_balance, TonyS32 p_pan, TonyS32 p_boost);
void WaveEnvAdvance(void);
void WaveMixRender(void);
void WaveMixAccumulate(void);
void WaveConvertDual16(void);
void WaveConvertSingle16(void);
void WaveConvertSingle8Mono(void);
void WaveConvertDual8Mono(void);
void WaveConvertSingle8Stereo(void);
void WaveConvertDual8Stereo(void);
void WaveEchoProcess(void);
void WaveStopLevelBleed(void);
void __fastcall DSndMixBlock(TonyS32 p_frames, TonyS16* p_dest);
TonyU8 __fastcall DSndChannelBusy(TonyU8 p_channel);
void __fastcall DSndChannelStart(TonyU8 p_channel);
void __fastcall DSndChannelStop(TonyU8 p_channel);
void __fastcall DSndChannelKill(TonyU8 p_channel);
void __fastcall DSndChannelSetAdsr(TonyU8 p_channel, TonyU16* p_desc);
void __fastcall DSndChannelSetRelease(TonyU8 p_channel, TonyS32 p_b);
void __fastcall DSndChannelGateOff(TonyU8 p_channel);

// Play descriptor handed from the dispatch TU (see soundlib.c).
// SIZE 0x1c
typedef struct SongDesc {
	TonyS32 m_rate;       // 0x00
	TonyS32 m_data;       // 0x04
	TonyS32 m_startPos;   // 0x08
	TonyS32 m_endPos;     // 0x0c
	TonyS32 m_loopStart;  // 0x10
	TonyS32 m_loopLength; // 0x14
	TonyU8 m_flags;       // 0x18
} SongDesc;

// Wave-driver channel state. The gap blocks are only touched by the hand-written
// mixer kernels: +0x10 the one-shot tail length, +0x1c the resample cursor
// (remaining samples, 20-bit fraction, float fraction), +0x30 the level ramp
// (block start level, per-ms step, next level), +0x46 the envelope stage block
// (stage byte, gate byte, stage time remaining, parked last output pair).
// SIZE 0x54
typedef struct WaveChannel {
	TonyU32 m_startPtr;                    // 0x00
	TonyU32 m_length;                      // 0x04
	TonyU32 m_loopPtr;                     // 0x08
	TonyU32 m_loopLength;                  // 0x0c
	undefined m_tailLength[0x14 - 0x10];   // 0x10
	TonyU16 m_step;                        // 0x14
	TonyU8 m_active;                       // 0x16
	undefined m_mixFlags[0x18 - 0x17];     // 0x17
	TonyU32 m_readPtr;                     // 0x18
	undefined m_resampleCursor[0x28 - 0x1c]; // 0x1c
	TonyFloat m_gainLeft;                  // 0x28
	TonyFloat m_gainRight;                 // 0x2c
	undefined m_levelRamp[0x3c - 0x30];    // 0x30
	TonyU16 m_attackTime;                  // 0x3c
	TonyU16 m_decayTime;                   // 0x3e
	TonyFloat m_sustainLevel;              // 0x40
	TonyU16 m_releaseTime;                 // 0x44
	undefined m_envState[0x54 - 0x46];     // 0x46
} WaveChannel;

// Pitch scale tables in 12.20 fixed point: entry i of the up table is 2^(i/12), entry i
// of the down table is 2^(-i/12). WaveNoteToStep walks up or down by the priority delta.
// GLOBAL: TONY2 0x00456270
TonyU32 g_wavePitchUpTable[0x80] = {
	0x00100000, 0x0010f390, 0x0011f59b, 0x001306fe, 0x001428a3, 0x00155b81, 0x0016a09f, 0x0017f911,
	0x001965ff, 0x001ae8a0, 0x001c823f, 0x001e3438, 0x00200001, 0x0021e720, 0x0023eb36, 0x00260dfd,
	0x00285147, 0x002ab703, 0x002d413e, 0x002ff223, 0x0032cbff, 0x0035d141, 0x0039047e, 0x003c6871,
	0x00400002, 0x0043ce40, 0x0047d66d, 0x004c1bfa, 0x0050a28e, 0x00556e07, 0x005a827c, 0x005fe446,
	0x006597fe, 0x006ba282, 0x007208fc, 0x0078d0e3, 0x00800004, 0x00879c81, 0x008facdb, 0x009837f5,
	0x00a1451d, 0x00aadc0e, 0x00b504f9, 0x00bfc88d, 0x00cb2ffc, 0x00d74504, 0x00e411f8, 0x00f1a1c7,
	0x01000008, 0x010f3902, 0x011f59b6, 0x01306fea, 0x01428a3a, 0x0155b81c, 0x016a09f2, 0x017f911a,
	0x01965ff8, 0x01ae8a08, 0x01c823f0, 0x01e3438e, 0x02000010, 0x021e7204, 0x023eb36c, 0x0260dfd4,
	0x02851474, 0x02ab7038, 0x02d413e4, 0x02ff2234, 0x032cbff0, 0x035d1410, 0x039047e0, 0x03c6871c,
	0x04000020, 0x043ce408, 0x047d66d8, 0x04c1bfa8, 0x050a28e8, 0x0556e070, 0x05a827c8, 0x05fe4468,
	0x06597fe0, 0x06ba2820, 0x07208fc0, 0x078d0e38, 0x08000040, 0x0879c810, 0x08facdb0, 0x09837f50,
	0x0a1451d0, 0x0aadc0e0, 0x0b504f90, 0x0bfc88d0, 0x0cb2ffc0, 0x0d745040, 0x0e411f80, 0x0f1a1c70,
	0x10000080, 0x10f39020, 0x11f59b60, 0x1306fea0, 0x1428a3a0, 0x155b81c0, 0x16a09f20, 0x17f911a0,
	0x1965ff80, 0x1ae8a080, 0x1c823f00, 0x1e3438e0, 0x20000100, 0x21e72040, 0x23eb36c0, 0x260dfd40,
	0x28514740, 0x2ab70380, 0x2d413e40, 0x2ff22340, 0x32cbff00, 0x35d14100, 0x39047e00, 0x3c6871c0,
	0x40000200, 0x43ce4080, 0x47d66d80, 0x4c1bfa80, 0x50a28e80, 0x556e0700, 0x5a827c80, 0x5fe44680

};

// GLOBAL: TONY2 0x00456470
TonyU32 g_wavePitchDownTable[0x80] = {
	0x00100000, 0x000f1a1c, 0x000e411f, 0x000d7450, 0x000cb2ff, 0x000bfc88, 0x000b504f, 0x000aadc0,
	0x000a1451, 0x0009837f, 0x0008facd, 0x000879c8, 0x00080000, 0x00078d0e, 0x0007208f, 0x0006ba28,
	0x00065980, 0x0005fe44, 0x0005a827, 0x000556e0, 0x00050a29, 0x0004c1bf, 0x00047d67, 0x00043ce4,
	0x00040000, 0x0003c687, 0x00039048, 0x00035d14, 0x00032cc0, 0x0002ff22, 0x0002d414, 0x0002ab70,
	0x00028514, 0x000260e0, 0x00023eb3, 0x00021e72, 0x00020000, 0x0001e343, 0x0001c824, 0x0001ae8a,
	0x00019660, 0x00017f91, 0x00016a0a, 0x000155b8, 0x0001428a, 0x00013070, 0x00011f5a, 0x00010f39,
	0x00010000, 0x0000f1a2, 0x0000e412, 0x0000d745, 0x0000cb30, 0x0000bfc9, 0x0000b505, 0x0000aadc,
	0x0000a145, 0x00009838, 0x00008fad, 0x0000879c, 0x00008000, 0x000078d1, 0x00007209, 0x00006ba2,
	0x00006598, 0x00005fe4, 0x00005a82, 0x0000556e, 0x000050a3, 0x00004c1c, 0x000047d6, 0x000043ce,
	0x00004000, 0x00003c68, 0x00003904, 0x000035d1, 0x000032cc, 0x00002ff2, 0x00002d41, 0x00002ab7,
	0x00002851, 0x0000260e, 0x000023eb, 0x000021e7, 0x00002000, 0x00001e34, 0x00001c82, 0x00001ae9,
	0x00001966, 0x000017f9, 0x000016a1, 0x0000155c, 0x00001429, 0x00001307, 0x000011f6, 0x000010f4,
	0x00001000, 0x00000f1a, 0x00000e41, 0x00000d74, 0x00000cb3, 0x00000bfd, 0x00000b50, 0x00000aae,
	0x00000a14, 0x00000983, 0x000008fb, 0x0000087a, 0x00000800, 0x0000078d, 0x00000721, 0x000006ba,
	0x00000659, 0x000005fe, 0x000005a8, 0x00000557, 0x0000050a, 0x000004c2, 0x0000047d, 0x0000043d,
	0x00000400, 0x000003c7, 0x00000390, 0x0000035d, 0x0000032d, 0x000002ff, 0x000002d4, 0x000002ab

};

// Fine pitch scale tables in 12.20 fixed point with 0x300 steps per octave: entry i of
// the up table is 2^(i/768), entry i of the down table is 2^(-i/768). One extra entry
// caps each table so the pair readers can fetch entry i + 1.
// GLOBAL: TONY2 0x00456670
TonyU32 g_waveFinePitchUpTable[0x301] = {
	0x00100000, 0x001003b3, 0x00100766, 0x00100b1b, 0x00100ed0, 0x00101287, 0x0010163e, 0x001019f6,
	0x00101dae, 0x00102168, 0x00102523, 0x001028de, 0x00102c9a, 0x00103057, 0x00103415, 0x001037d4,
	0x00103b94, 0x00103f55, 0x00104316, 0x001046d8, 0x00104a9b, 0x00104e5f, 0x00105224, 0x001055ea,
	0x001059b1, 0x00105d78, 0x00106141, 0x0010650a, 0x001068d4, 0x00106c9f, 0x0010706b, 0x00107438,
	0x00107805, 0x00107bd4, 0x00107fa3, 0x00108373, 0x00108745, 0x00108b17, 0x00108eea, 0x001092be,
	0x00109692, 0x00109a68, 0x00109e3e, 0x0010a216, 0x0010a5ee, 0x0010a9c7, 0x0010ada1, 0x0010b17c,
	0x0010b558, 0x0010b935, 0x0010bd12, 0x0010c0f1, 0x0010c4d0, 0x0010c8b1, 0x0010cc92, 0x0010d074,
	0x0010d457, 0x0010d83b, 0x0010dc20, 0x0010e005, 0x0010e3ec, 0x0010e7d3, 0x0010ebbc, 0x0010efa5,
	0x0010f38f, 0x0010f77a, 0x0010fb66, 0x0010ff53, 0x00110341, 0x00110730, 0x00110b1f, 0x00110f10,
	0x00111301, 0x001116f4, 0x00111ae7, 0x00111edb, 0x001122d0, 0x001126c6, 0x00112abd, 0x00112eb5,
	0x001132ae, 0x001136a8, 0x00113aa2, 0x00113e9e, 0x0011429a, 0x00114698, 0x00114a96, 0x00114e95,
	0x00115295, 0x00115697, 0x00115a99, 0x00115e9b, 0x0011629f, 0x001166a4, 0x00116aaa, 0x00116eb0,
	0x001172b8, 0x001176c1, 0x00117aca, 0x00117ed4, 0x001182e0, 0x001186ec, 0x00118af9, 0x00118f07,
	0x00119316, 0x00119726, 0x00119b37, 0x00119f49, 0x0011a35c, 0x0011a76f, 0x0011ab84, 0x0011af9a,
	0x0011b3b0, 0x0011b7c8, 0x0011bbe0, 0x0011bffa, 0x0011c414, 0x0011c82f, 0x0011cc4b, 0x0011d069,
	0x0011d487, 0x0011d8a6, 0x0011dcc6, 0x0011e0e7, 0x0011e509, 0x0011e92c, 0x0011ed50, 0x0011f175,
	0x0011f59a, 0x0011f9c1, 0x0011fde9, 0x00120211, 0x0012063b, 0x00120a66, 0x00120e91, 0x001212be,
	0x001216eb, 0x00121b1a, 0x00121f49, 0x00122379, 0x001227ab, 0x00122bdd, 0x00123010, 0x00123445,
	0x0012387a, 0x00123cb0, 0x001240e7, 0x0012451f, 0x00124959, 0x00124d93, 0x001251ce, 0x0012560a,
	0x00125a47, 0x00125e85, 0x001262c4, 0x00126704, 0x00126b45, 0x00126f87, 0x001273ca, 0x0012780e,
	0x00127c53, 0x00128098, 0x001284df, 0x00128927, 0x00128d70, 0x001291ba, 0x00129605, 0x00129a51,
	0x00129e9e, 0x0012a2eb, 0x0012a73a, 0x0012ab8a, 0x0012afdb, 0x0012b42d, 0x0012b87f, 0x0012bcd3,
	0x0012c128, 0x0012c57e, 0x0012c9d4, 0x0012ce2c, 0x0012d285, 0x0012d6df, 0x0012db3a, 0x0012df95,
	0x0012e3f2, 0x0012e850, 0x0012ecaf, 0x0012f10f, 0x0012f570, 0x0012f9d2, 0x0012fe35, 0x00130298,
	0x001306fd, 0x00130b63, 0x00130fca, 0x00131432, 0x0013189b, 0x00131d05, 0x00132170, 0x001325dc,
	0x00132a49, 0x00132eb8, 0x00133327, 0x00133797, 0x00133c08, 0x0013407a, 0x001344ed, 0x00134962,
	0x00134dd7, 0x0013524d, 0x001356c5, 0x00135b3d, 0x00135fb6, 0x00136431, 0x001368ac, 0x00136d29,
	0x001371a7, 0x00137625, 0x00137aa5, 0x00137f25, 0x001383a7, 0x0013882a, 0x00138cae, 0x00139133,
	0x001395b8, 0x00139a3f, 0x00139ec7, 0x0013a350, 0x0013a7db, 0x0013ac66, 0x0013b0f2, 0x0013b57f,
	0x0013ba0d, 0x0013be9d, 0x0013c32d, 0x0013c7bf, 0x0013cc51, 0x0013d0e5, 0x0013d579, 0x0013da0f,
	0x0013dea6, 0x0013e33d, 0x0013e7d6, 0x0013ec70, 0x0013f10b, 0x0013f5a7, 0x0013fa44, 0x0013fee2,
	0x00140382, 0x00140822, 0x00140cc3, 0x00141166, 0x00141609, 0x00141aae, 0x00141f54, 0x001423fa,
	0x001428a2, 0x00142d4b, 0x001431f5, 0x001436a0, 0x00143b4c, 0x00143ff9, 0x001444a8, 0x00144957,
	0x00144e08, 0x001452b9, 0x0014576c, 0x00145c1f, 0x001460d4, 0x0014658a, 0x00146a41, 0x00146ef9,
	0x001473b2, 0x0014786d, 0x00147d28, 0x001481e4, 0x001486a2, 0x00148b60, 0x00149020, 0x001494e1,
	0x001499a3, 0x00149e66, 0x0014a32a, 0x0014a7ef, 0x0014acb6, 0x0014b17d, 0x0014b646, 0x0014bb0f,
	0x0014bfda, 0x0014c4a6, 0x0014c973, 0x0014ce41, 0x0014d310, 0x0014d7e0, 0x0014dcb2, 0x0014e184,
	0x0014e658, 0x0014eb2d, 0x0014f002, 0x0014f4d9, 0x0014f9b2, 0x0014fe8b, 0x00150365, 0x00150841,
	0x00150d1d, 0x001511fb, 0x001516da, 0x00151bba, 0x0015209b, 0x0015257d, 0x00152a61, 0x00152f45,
	0x0015342b, 0x00153912, 0x00153df9, 0x001542e2, 0x001547cd, 0x00154cb8, 0x001551a4, 0x00155692,
	0x00155b80, 0x00156070, 0x00156561, 0x00156a53, 0x00156f47, 0x0015743b, 0x00157931, 0x00157e27,
	0x0015831f, 0x00158818, 0x00158d12, 0x0015920e, 0x0015970a, 0x00159c08, 0x0015a106, 0x0015a606,
	0x0015ab07, 0x0015b00a, 0x0015b50d, 0x0015ba11, 0x0015bf17, 0x0015c41e, 0x0015c926, 0x0015ce2f,
	0x0015d33a, 0x0015d845, 0x0015dd52, 0x0015e260, 0x0015e76f, 0x0015ec7f, 0x0015f190, 0x0015f6a3,
	0x0015fbb6, 0x001600cb, 0x001605e1, 0x00160af8, 0x00161011, 0x0016152a, 0x00161a45, 0x00161f61,
	0x0016247e, 0x0016299c, 0x00162ebc, 0x001633dd, 0x001638fe, 0x00163e21, 0x00164346, 0x0016486b,
	0x00164d92, 0x001652b9, 0x001657e2, 0x00165d0d, 0x00166238, 0x00166764, 0x00166c92, 0x001671c1,
	0x001676f1, 0x00167c23, 0x00168155, 0x00168689, 0x00168bbe, 0x001690f4, 0x0016962b, 0x00169b64,
	0x0016a09e, 0x0016a5d9, 0x0016ab15, 0x0016b052, 0x0016b591, 0x0016bad1, 0x0016c012, 0x0016c554,
	0x0016ca98, 0x0016cfdc, 0x0016d522, 0x0016da69, 0x0016dfb2, 0x0016e4fb, 0x0016ea46, 0x0016ef92,
	0x0016f4df, 0x0016fa2e, 0x0016ff7d, 0x001704ce, 0x00170a20, 0x00170f74, 0x001714c8, 0x00171a1e,
	0x00171f75, 0x001724ce, 0x00172a27, 0x00172f82, 0x001734de, 0x00173a3b, 0x00173f9a, 0x001744f9,
	0x00174a5a, 0x00174fbd, 0x00175520, 0x00175a85, 0x00175feb, 0x00176552, 0x00176abb, 0x00177024,
	0x0017758f, 0x00177afc, 0x00178069, 0x001785d8, 0x00178b48, 0x001790b9, 0x0017962c, 0x00179b9f,
	0x0017a114, 0x0017a68b, 0x0017ac02, 0x0017b17b, 0x0017b6f5, 0x0017bc70, 0x0017c1ed, 0x0017c76b,
	0x0017ccea, 0x0017d26a, 0x0017d7ec, 0x0017dd6f, 0x0017e2f3, 0x0017e879, 0x0017edff, 0x0017f387,
	0x0017f911, 0x0017fe9b, 0x00180427, 0x001809b4, 0x00180f43, 0x001814d3, 0x00181a64, 0x00181ff6,
	0x0018258a, 0x00182b1e, 0x001830b5, 0x0018364c, 0x00183be5, 0x0018417f, 0x0018471a, 0x00184cb7,
	0x00185255, 0x001857f4, 0x00185d95, 0x00186336, 0x001868da, 0x00186e7e, 0x00187424, 0x001879cb,
	0x00187f73, 0x0018851d, 0x00188ac8, 0x00189074, 0x00189622, 0x00189bd1, 0x0018a181, 0x0018a732,
	0x0018ace5, 0x0018b299, 0x0018b84f, 0x0018be06, 0x0018c3be, 0x0018c977, 0x0018cf32, 0x0018d4ee,
	0x0018daab, 0x0018e06a, 0x0018e62a, 0x0018ebec, 0x0018f1ae, 0x0018f773, 0x0018fd38, 0x001902ff,
	0x001908c7, 0x00190e90, 0x0019145b, 0x00191a27, 0x00191ff4, 0x001925c3, 0x00192b93, 0x00193165,
	0x00193738, 0x00193d0c, 0x001942e1, 0x001948b8, 0x00194e90, 0x0019546a, 0x00195a45, 0x00196021,
	0x001965ff, 0x00196bde, 0x001971be, 0x001977a0, 0x00197d83, 0x00198367, 0x0019894d, 0x00198f34,
	0x0019951c, 0x00199b06, 0x0019a0f1, 0x0019a6de, 0x0019accc, 0x0019b2bb, 0x0019b8ac, 0x0019be9e,
	0x0019c492, 0x0019ca87, 0x0019d07d, 0x0019d675, 0x0019dc6e, 0x0019e268, 0x0019e864, 0x0019ee61,
	0x0019f45f, 0x0019fa5f, 0x001a0060, 0x001a0663, 0x001a0c67, 0x001a126c, 0x001a1873, 0x001a1e7c,
	0x001a2485, 0x001a2a90, 0x001a309d, 0x001a36aa, 0x001a3cba, 0x001a42ca, 0x001a48dc, 0x001a4ef0,
	0x001a5504, 0x001a5b1b, 0x001a6132, 0x001a674b, 0x001a6d66, 0x001a7381, 0x001a799f, 0x001a7fbd,
	0x001a85dd, 0x001a8bff, 0x001a9222, 0x001a9846, 0x001a9e6c, 0x001aa493, 0x001aaabc, 0x001ab0e6,
	0x001ab711, 0x001abd3e, 0x001ac36c, 0x001ac99c, 0x001acfcd, 0x001ad600, 0x001adc34, 0x001ae269,
	0x001ae8a0, 0x001aeed9, 0x001af512, 0x001afb4e, 0x001b018a, 0x001b07c8, 0x001b0e08, 0x001b1449,
	0x001b1a8b, 0x001b20cf, 0x001b2714, 0x001b2d5b, 0x001b33a3, 0x001b39ed, 0x001b4038, 0x001b4685,
	0x001b4cd3, 0x001b5322, 0x001b5973, 0x001b5fc6, 0x001b661a, 0x001b6c6f, 0x001b72c6, 0x001b791e,
	0x001b7f78, 0x001b85d3, 0x001b8c30, 0x001b928e, 0x001b98ed, 0x001b9f4e, 0x001ba5b1, 0x001bac15,
	0x001bb27a, 0x001bb8e1, 0x001bbf4a, 0x001bc5b4, 0x001bcc1f, 0x001bd28c, 0x001bd8fb, 0x001bdf6a,
	0x001be5dc, 0x001bec4f, 0x001bf2c3, 0x001bf939, 0x001bffb0, 0x001c0629, 0x001c0ca4, 0x001c131f,
	0x001c199d, 0x001c201c, 0x001c269c, 0x001c2d1e, 0x001c33a1, 0x001c3a26, 0x001c40ad, 0x001c4734,
	0x001c4dbe, 0x001c5449, 0x001c5ad5, 0x001c6163, 0x001c67f2, 0x001c6e83, 0x001c7516, 0x001c7baa,
	0x001c823f, 0x001c88d6, 0x001c8f6f, 0x001c9609, 0x001c9ca4, 0x001ca341, 0x001ca9e0, 0x001cb080,
	0x001cb722, 0x001cbdc5, 0x001cc46a, 0x001ccb10, 0x001cd1b8, 0x001cd861, 0x001cdf0c, 0x001ce5b9,
	0x001cec67, 0x001cf316, 0x001cf9c7, 0x001d007a, 0x001d072e, 0x001d0de4, 0x001d149b, 0x001d1b54,
	0x001d220f, 0x001d28cb, 0x001d2f88, 0x001d3647, 0x001d3d08, 0x001d43ca, 0x001d4a8e, 0x001d5153,
	0x001d581a, 0x001d5ee3, 0x001d65ad, 0x001d6c78, 0x001d7345, 0x001d7a14, 0x001d80e4, 0x001d87b6,
	0x001d8e8a, 0x001d955f, 0x001d9c35, 0x001da30e, 0x001da9e7, 0x001db0c3, 0x001db7a0, 0x001dbe7e,
	0x001dc55e, 0x001dcc40, 0x001dd323, 0x001dda08, 0x001de0ef, 0x001de7d7, 0x001deec0, 0x001df5ac,
	0x001dfc99, 0x001e0387, 0x001e0a77, 0x001e1169, 0x001e185c, 0x001e1f51, 0x001e2647, 0x001e2d40,
	0x001e3439, 0x001e3b35, 0x001e4232, 0x001e4930, 0x001e5030, 0x001e5732, 0x001e5e35, 0x001e653a,
	0x001e6c41, 0x001e7349, 0x001e7a53, 0x001e815f, 0x001e886c, 0x001e8f7b, 0x001e968b, 0x001e9d9d,
	0x001ea4b1, 0x001eabc6, 0x001eb2dd, 0x001eb9f6, 0x001ec110, 0x001ec82c, 0x001ecf49, 0x001ed668,
	0x001edd89, 0x001ee4ac, 0x001eebd0, 0x001ef2f6, 0x001efa1d, 0x001f0146, 0x001f0871, 0x001f0f9d,
	0x001f16cb, 0x001f1dfb, 0x001f252c, 0x001f2c5f, 0x001f3394, 0x001f3acb, 0x001f4203, 0x001f493c,
	0x001f5078, 0x001f57b5, 0x001f5ef3, 0x001f6634, 0x001f6d76, 0x001f74ba, 0x001f7bff, 0x001f8346,
	0x001f8a8f, 0x001f91d9, 0x001f9925, 0x001fa073, 0x001fa7c3, 0x001faf14, 0x001fb667, 0x001fbdbc,
	0x001fc512, 0x001fcc6a, 0x001fd3c3, 0x001fdb1f, 0x001fe27c, 0x001fe9db, 0x001ff13b, 0x001ff89d,
	0x00200000
};

// GLOBAL: TONY2 0x00457274
TonyU32 g_waveFinePitchDownTable[0x301] = {
	0x00100000, 0x000ffc4e, 0x000ff89d, 0x000ff4ed, 0x000ff13d, 0x000fed8f, 0x000fe9e1, 0x000fe634,
	0x000fe288, 0x000fdedd, 0x000fdb33, 0x000fd789, 0x000fd3e1, 0x000fd039, 0x000fcc92, 0x000fc8ec,
	0x000fc547, 0x000fc1a2, 0x000fbdff, 0x000fba5c, 0x000fb6ba, 0x000fb319, 0x000faf79, 0x000fabda,
	0x000fa83b, 0x000fa49e, 0x000fa101, 0x000f9d65, 0x000f99c9, 0x000f962f, 0x000f9296, 0x000f8efd,
	0x000f8b65, 0x000f87ce, 0x000f8438, 0x000f80a2, 0x000f7d0e, 0x000f797a, 0x000f75e7, 0x000f7255,
	0x000f6ec4, 0x000f6b34, 0x000f67a4, 0x000f6415, 0x000f6087, 0x000f5cfa, 0x000f596e, 0x000f55e2,
	0x000f5258, 0x000f4ece, 0x000f4b45, 0x000f47bd, 0x000f4435, 0x000f40af, 0x000f3d29, 0x000f39a4,
	0x000f3620, 0x000f329d, 0x000f2f1a, 0x000f2b98, 0x000f2818, 0x000f2497, 0x000f2118, 0x000f1d9a,
	0x000f1a1c, 0x000f169f, 0x000f1323, 0x000f0fa8, 0x000f0c2d, 0x000f08b4, 0x000f053b, 0x000f01c3,
	0x000efe4c, 0x000efad5, 0x000ef760, 0x000ef3eb, 0x000ef077, 0x000eed03, 0x000ee991, 0x000ee61f,
	0x000ee2af, 0x000edf3e, 0x000edbcf, 0x000ed861, 0x000ed4f3, 0x000ed186, 0x000ece1a, 0x000ecaaf,
	0x000ec744, 0x000ec3da, 0x000ec072, 0x000ebd09, 0x000eb9a2, 0x000eb63b, 0x000eb2d6, 0x000eaf71,
	0x000eac0c, 0x000ea8a9, 0x000ea546, 0x000ea1e4, 0x000e9e83, 0x000e9b23, 0x000e97c3, 0x000e9465,
	0x000e9107, 0x000e8da9, 0x000e8a4d, 0x000e86f1, 0x000e8397, 0x000e803c, 0x000e7ce3, 0x000e798b,
	0x000e7633, 0x000e72dc, 0x000e6f86, 0x000e6c30, 0x000e68db, 0x000e6587, 0x000e6234, 0x000e5ee2,
	0x000e5b90, 0x000e583f, 0x000e54ef, 0x000e51a0, 0x000e4e52, 0x000e4b04, 0x000e47b7, 0x000e446a,
	0x000e411f, 0x000e3dd4, 0x000e3a8a, 0x000e3741, 0x000e33f9, 0x000e30b1, 0x000e2d6a, 0x000e2a24,
	0x000e26de, 0x000e239a, 0x000e2056, 0x000e1d12, 0x000e19d0, 0x000e168e, 0x000e134d, 0x000e100d,
	0x000e0cce, 0x000e098f, 0x000e0651, 0x000e0314, 0x000dffd8, 0x000dfc9c, 0x000df961, 0x000df627,
	0x000df2ed, 0x000defb5, 0x000dec7d, 0x000de945, 0x000de60f, 0x000de2d9, 0x000ddfa4, 0x000ddc70,
	0x000dd93d, 0x000dd60a, 0x000dd2d8, 0x000dcfa7, 0x000dcc76, 0x000dc946, 0x000dc617, 0x000dc2e9,
	0x000dbfbb, 0x000dbc8e, 0x000db962, 0x000db637, 0x000db30c, 0x000dafe2, 0x000dacb9, 0x000da991,
	0x000da669, 0x000da342, 0x000da01b, 0x000d9cf6, 0x000d99d1, 0x000d96ad, 0x000d938a, 0x000d9067,
	0x000d8d45, 0x000d8a24, 0x000d8703, 0x000d83e4, 0x000d80c4, 0x000d7da6, 0x000d7a89, 0x000d776c,
	0x000d744f, 0x000d7134, 0x000d6e19, 0x000d6aff, 0x000d67e6, 0x000d64cd, 0x000d61b6, 0x000d5e9e,
	0x000d5b88, 0x000d5872, 0x000d555d, 0x000d5249, 0x000d4f35, 0x000d4c22, 0x000d4910, 0x000d45ff,
	0x000d42ee, 0x000d3fde, 0x000d3ccf, 0x000d39c0, 0x000d36b2, 0x000d33a5, 0x000d3098, 0x000d2d8d,
	0x000d2a82, 0x000d2777, 0x000d246d, 0x000d2164, 0x000d1e5c, 0x000d1b55, 0x000d184e, 0x000d1547,
	0x000d1242, 0x000d0f3d, 0x000d0c39, 0x000d0936, 0x000d0633, 0x000d0331, 0x000d0030, 0x000cfd2f,
	0x000cfa2f, 0x000cf730, 0x000cf431, 0x000cf133, 0x000cee36, 0x000ceb3a, 0x000ce83e, 0x000ce543,
	0x000ce248, 0x000cdf4f, 0x000cdc55, 0x000cd95d, 0x000cd665, 0x000cd36e, 0x000cd078, 0x000ccd82,
	0x000cca8e, 0x000cc799, 0x000cc4a6, 0x000cc1b3, 0x000cbec1, 0x000cbbcf, 0x000cb8de, 0x000cb5ee,
	0x000cb2ff, 0x000cb010, 0x000cad22, 0x000caa34, 0x000ca748, 0x000ca45b, 0x000ca170, 0x000c9e85,
	0x000c9b9b, 0x000c98b2, 0x000c95c9, 0x000c92e1, 0x000c8ffa, 0x000c8d13, 0x000c8a2d, 0x000c8747,
	0x000c8463, 0x000c817f, 0x000c7e9b, 0x000c7bb9, 0x000c78d7, 0x000c75f5, 0x000c7315, 0x000c7034,
	0x000c6d55, 0x000c6a76, 0x000c6798, 0x000c64bb, 0x000c61de, 0x000c5f02, 0x000c5c27, 0x000c594c,
	0x000c5672, 0x000c5399, 0x000c50c0, 0x000c4de8, 0x000c4b10, 0x000c4839, 0x000c4563, 0x000c428e,
	0x000c3fb9, 0x000c3ce5, 0x000c3a11, 0x000c373e, 0x000c346c, 0x000c319b, 0x000c2eca, 0x000c2bf9,
	0x000c292a, 0x000c265b, 0x000c238c, 0x000c20bf, 0x000c1df2, 0x000c1b25, 0x000c185a, 0x000c158f,
	0x000c12c4, 0x000c0ffa, 0x000c0d31, 0x000c0a69, 0x000c07a1, 0x000c04da, 0x000c0213, 0x000bff4d,
	0x000bfc88, 0x000bf9c3, 0x000bf6ff, 0x000bf43c, 0x000bf179, 0x000beeb7, 0x000bebf5, 0x000be935,
	0x000be674, 0x000be3b5, 0x000be0f6, 0x000bde38, 0x000bdb7a, 0x000bd8bd, 0x000bd600, 0x000bd345,
	0x000bd08a, 0x000bcdcf, 0x000bcb15, 0x000bc85c, 0x000bc5a3, 0x000bc2eb, 0x000bc034, 0x000bbd7d,
	0x000bbac7, 0x000bb812, 0x000bb55d, 0x000bb2a8, 0x000baff5, 0x000bad42, 0x000baa8f, 0x000ba7de,
	0x000ba52d, 0x000ba27c, 0x000b9fcc, 0x000b9d1d, 0x000b9a6e, 0x000b97c0, 0x000b9513, 0x000b9266,
	0x000b8fba, 0x000b8d0f, 0x000b8a64, 0x000b87b9, 0x000b8510, 0x000b8267, 0x000b7fbe, 0x000b7d16,
	0x000b7a6f, 0x000b77c8, 0x000b7522, 0x000b727d, 0x000b6fd8, 0x000b6d34, 0x000b6a90, 0x000b67ee,
	0x000b654b, 0x000b62a9, 0x000b6008, 0x000b5d68, 0x000b5ac8, 0x000b5829, 0x000b558a, 0x000b52ec,
	0x000b504e, 0x000b4db1, 0x000b4b15, 0x000b4879, 0x000b45de, 0x000b4344, 0x000b40aa, 0x000b3e11,
	0x000b3b78, 0x000b38e0, 0x000b3648, 0x000b33b2, 0x000b311b, 0x000b2e86, 0x000b2bf1, 0x000b295c,
	0x000b26c8, 0x000b2435, 0x000b21a2, 0x000b1f10, 0x000b1c7f, 0x000b19ee, 0x000b175d, 0x000b14ce,
	0x000b123e, 0x000b0fb0, 0x000b0d22, 0x000b0a95, 0x000b0808, 0x000b057c, 0x000b02f0, 0x000b0065,
	0x000afddb, 0x000afb51, 0x000af8c7, 0x000af63f, 0x000af3b7, 0x000af12f, 0x000aeea8, 0x000aec22,
	0x000ae99c, 0x000ae717, 0x000ae492, 0x000ae20e, 0x000adf8b, 0x000add08, 0x000ada86, 0x000ad804,
	0x000ad583, 0x000ad303, 0x000ad083, 0x000ace03, 0x000acb84, 0x000ac906, 0x000ac689, 0x000ac40b,
	0x000ac18f, 0x000abf13, 0x000abc98, 0x000aba1d, 0x000ab7a3, 0x000ab529, 0x000ab2b0, 0x000ab038,
	0x000aadc0, 0x000aab48, 0x000aa8d1, 0x000aa65b, 0x000aa3e6, 0x000aa171, 0x000a9efc, 0x000a9c88,
	0x000a9a15, 0x000a97a2, 0x000a9530, 0x000a92be, 0x000a904d, 0x000a8ddc, 0x000a8b6c, 0x000a88fd,
	0x000a868e, 0x000a8420, 0x000a81b2, 0x000a7f45, 0x000a7cd8, 0x000a7a6c, 0x000a7801, 0x000a7596,
	0x000a732b, 0x000a70c1, 0x000a6e58, 0x000a6bf0, 0x000a6987, 0x000a6720, 0x000a64b9, 0x000a6252,
	0x000a5fec, 0x000a5d87, 0x000a5b22, 0x000a58be, 0x000a565a, 0x000a53f7, 0x000a5194, 0x000a4f32,
	0x000a4cd1, 0x000a4a70, 0x000a480f, 0x000a45b0, 0x000a4350, 0x000a40f2, 0x000a3e93, 0x000a3c36,
	0x000a39d9, 0x000a377c, 0x000a3520, 0x000a32c4, 0x000a3069, 0x000a2e0f, 0x000a2bb5, 0x000a295c,
	0x000a2703, 0x000a24ab, 0x000a2253, 0x000a1ffc, 0x000a1da5, 0x000a1b4f, 0x000a18fa, 0x000a16a5,
	0x000a1450, 0x000a11fd, 0x000a0fa9, 0x000a0d56, 0x000a0b04, 0x000a08b2, 0x000a0661, 0x000a0410,
	0x000a01c0, 0x0009ff71, 0x0009fd21, 0x0009fad3, 0x0009f885, 0x0009f637, 0x0009f3ea, 0x0009f19e,
	0x0009ef52, 0x0009ed07, 0x0009eabc, 0x0009e872, 0x0009e628, 0x0009e3df, 0x0009e196, 0x0009df4e,
	0x0009dd06, 0x0009dabf, 0x0009d878, 0x0009d632, 0x0009d3ed, 0x0009d1a8, 0x0009cf63, 0x0009cd1f,
	0x0009cadc, 0x0009c899, 0x0009c656, 0x0009c414, 0x0009c1d3, 0x0009bf92, 0x0009bd52, 0x0009bb12,
	0x0009b8d3, 0x0009b694, 0x0009b456, 0x0009b218, 0x0009afdb, 0x0009ad9e, 0x0009ab62, 0x0009a926,
	0x0009a6eb, 0x0009a4b0, 0x0009a276, 0x0009a03d, 0x00099e03, 0x00099bcb, 0x00099993, 0x0009975b,
	0x00099524, 0x000992ee, 0x000990b8, 0x00098e82, 0x00098c4d, 0x00098a19, 0x000987e5, 0x000985b1,
	0x0009837e, 0x0009814c, 0x00097f1a, 0x00097ce8, 0x00097ab7, 0x00097887, 0x00097657, 0x00097428,
	0x000971f9, 0x00096fca, 0x00096d9c, 0x00096b6f, 0x00096942, 0x00096716, 0x000964ea, 0x000962be,
	0x00096093, 0x00095e69, 0x00095c3f, 0x00095a16, 0x000957ed, 0x000955c5, 0x0009539d, 0x00095175,
	0x00094f4e, 0x00094d28, 0x00094b02, 0x000948dc, 0x000946b8, 0x00094493, 0x0009426f, 0x0009404c,
	0x00093e29, 0x00093c06, 0x000939e4, 0x000937c3, 0x000935a2, 0x00093381, 0x00093161, 0x00092f42,
	0x00092d23, 0x00092b04, 0x000928e6, 0x000926c9, 0x000924ac, 0x0009228f, 0x00092073, 0x00091e58,
	0x00091c3c, 0x00091a22, 0x00091808, 0x000915ee, 0x000913d5, 0x000911bc, 0x00090fa4, 0x00090d8c,
	0x00090b75, 0x0009095e, 0x00090748, 0x00090532, 0x0009031d, 0x00090108, 0x0008fef4, 0x0008fce0,
	0x0008facd, 0x0008f8ba, 0x0008f6a7, 0x0008f495, 0x0008f284, 0x0008f073, 0x0008ee62, 0x0008ec52,
	0x0008ea43, 0x0008e834, 0x0008e625, 0x0008e417, 0x0008e209, 0x0008dffc, 0x0008ddf0, 0x0008dbe3,
	0x0008d9d8, 0x0008d7cc, 0x0008d5c1, 0x0008d3b7, 0x0008d1ad, 0x0008cfa4, 0x0008cd9b, 0x0008cb93,
	0x0008c98b, 0x0008c783, 0x0008c57c, 0x0008c375, 0x0008c16f, 0x0008bf6a, 0x0008bd64, 0x0008bb60,
	0x0008b95c, 0x0008b758, 0x0008b554, 0x0008b352, 0x0008b14f, 0x0008af4d, 0x0008ad4c, 0x0008ab4b,
	0x0008a94a, 0x0008a74a, 0x0008a54b, 0x0008a34b, 0x0008a14d, 0x00089f4e, 0x00089d51, 0x00089b53,
	0x00089957, 0x0008975a, 0x0008955e, 0x00089363, 0x00089168, 0x00088f6d, 0x00088d73, 0x00088b79,
	0x00088980, 0x00088787, 0x0008858f, 0x00088397, 0x000881a0, 0x00087fa9, 0x00087db3, 0x00087bbd,
	0x000879c7, 0x000877d2, 0x000875dd, 0x000873e9, 0x000871f5, 0x00087002, 0x00086e0f, 0x00086c1d,
	0x00086a2b, 0x00086839, 0x00086648, 0x00086458, 0x00086268, 0x00086078, 0x00085e89, 0x00085c9a,
	0x00085aac, 0x000858be, 0x000856d0, 0x000854e3, 0x000852f7, 0x0008510a, 0x00084f1f, 0x00084d33,
	0x00084b49, 0x0008495e, 0x00084774, 0x0008458b, 0x000843a2, 0x000841b9, 0x00083fd1, 0x00083de9,
	0x00083c02, 0x00083a1b, 0x00083835, 0x0008364f, 0x00083469, 0x00083284, 0x000830a0, 0x00082ebc,
	0x00082cd8, 0x00082af5, 0x00082912, 0x0008272f, 0x0008254d, 0x0008236c, 0x0008218b, 0x00081faa,
	0x00081dc9, 0x00081bea, 0x00081a0a, 0x0008182b, 0x0008164d, 0x0008146e, 0x00081291, 0x000810b4,
	0x00080ed7, 0x00080cfa, 0x00080b1e, 0x00080943, 0x00080768, 0x0008058d, 0x000803b3, 0x000801d9,
	0x00080000
};

// Level-to-gain curve sampled by WaveLevelPanToGains at level >> 16 with linear interpolation
// on the low 16 bits.
// GLOBAL: TONY2 0x00457e78
TonyFloat g_waveLevelCurve[0x80] = {
	0.0f, 3.05185e-05f, 0.000152593f, 0.000396741f, 0.000701926f, 0.00112918f, 0.001648f, 0.00222785f,
	0.00292978f, 0.00372326f, 0.00460829f, 0.00558489f, 0.00665304f, 0.00784326f, 0.00912503f,
	0.0104984f, 0.0119633f, 0.0135502f, 0.0151982f, 0.0169988f, 0.0188604f, 0.0208441f, 0.0229194f,
	0.0251167f, 0.0274056f, 0.0298166f, 0.0323191f, 0.0349437f, 0.0376598f, 0.0404675f, 0.0434278f,
	0.0464797f, 0.0496231f, 0.0528886f, 0.0562761f, 0.0597858f, 0.0633869f, 0.0671102f, 0.0709555f,
	0.0749229f, 0.0789819f, 0.0831629f, 0.087466f, 0.0919218f, 0.096469f, 0.101138f, 0.10593f,
	0.110843f, 0.115879f, 0.121036f, 0.126347f, 0.131748f, 0.137303f, 0.142979f, 0.148778f, 0.154729f,
	0.160772f, 0.166997f, 0.173315f, 0.179785f, 0.186407f, 0.193121f, 0.200018f, 0.207007f, 0.214179f,
	0.221473f, 0.228919f, 0.236488f, 0.244209f, 0.252083f, 0.260079f, 0.268258f, 0.276559f, 0.285012f,
	0.293649f, 0.302408f, 0.311319f, 0.320383f, 0.3296f, 0.339f, 0.348521f, 0.358226f, 0.368084f,
	0.378094f, 0.388287f, 0.398633f, 0.409131f, 0.419813f, 0.430647f, 0.441664f, 0.452864f, 0.464217f,
	0.475753f, 0.487442f, 0.499313f, 0.511399f, 0.523606f, 0.536027f, 0.548631f, 0.561419f, 0.574389f,
	0.587542f, 0.600879f, 0.614399f, 0.628132f, 0.642018f, 0.656148f, 0.670431f, 0.684927f, 0.699637f,
	0.71453f, 0.729637f, 0.744926f, 0.76043f, 0.776147f, 0.792077f, 0.808191f, 0.824549f, 0.84109f,
	0.857845f, 0.874844f, 0.892056f, 0.909452f, 0.927122f, 0.945006f, 0.963073f, 0.981414f, 1.0f

};

// -3 dB gain applied to both channels when the pan argument selects the phase-inverted
// center mode. Also read as the interpolation guard entry past the level curve.
// GLOBAL: TONY2 0x00458078
TonyFloat g_waveCenterGain = 0.7079f;

// Pan-to-gain curve sampled by WaveLevelPanToGains at pan >> 22 with linear interpolation on
// the low 22 bits.
// GLOBAL: TONY2 0x0045807c
TonyFloat g_wavePanCurve[3] = {
	0.0f, 0.7079f, 1.0f

};

// Reciprocal of the 22-bit pan fraction range (~1 / 0x400000).
// GLOBAL: TONY2 0x00458088
TonyFloat g_wavePanFracScale = 2.38e-07f;

// Second copy of the level-to-gain curve, sampled by the four-corner gain helper
// WaveCornerGains.
// GLOBAL: TONY2 0x004580bc
TonyFloat g_waveLevelCurve2[0x80] = {
	0.0f, 3.05185e-05f, 0.000152593f, 0.000396741f, 0.000701926f, 0.00112918f, 0.001648f, 0.00222785f,
	0.00292978f, 0.00372326f, 0.00460829f, 0.00558489f, 0.00665304f, 0.00784326f, 0.00912503f,
	0.0104984f, 0.0119633f, 0.0135502f, 0.0151982f, 0.0169988f, 0.0188604f, 0.0208441f, 0.0229194f,
	0.0251167f, 0.0274056f, 0.0298166f, 0.0323191f, 0.0349437f, 0.0376598f, 0.0404675f, 0.0434278f,
	0.0464797f, 0.0496231f, 0.0528886f, 0.0562761f, 0.0597858f, 0.0633869f, 0.0671102f, 0.0709555f,
	0.0749229f, 0.0789819f, 0.0831629f, 0.087466f, 0.0919218f, 0.096469f, 0.101138f, 0.10593f,
	0.110843f, 0.115879f, 0.121036f, 0.126347f, 0.131748f, 0.137303f, 0.142979f, 0.148778f, 0.154729f,
	0.160772f, 0.166997f, 0.173315f, 0.179785f, 0.186407f, 0.193121f, 0.200018f, 0.207007f, 0.214179f,
	0.221473f, 0.228919f, 0.236488f, 0.244209f, 0.252083f, 0.260079f, 0.268258f, 0.276559f, 0.285012f,
	0.293649f, 0.302408f, 0.311319f, 0.320383f, 0.3296f, 0.339f, 0.348521f, 0.358226f, 0.368084f,
	0.378094f, 0.388287f, 0.398633f, 0.409131f, 0.419813f, 0.430647f, 0.441664f, 0.452864f, 0.464217f,
	0.475753f, 0.487442f, 0.499313f, 0.511399f, 0.523606f, 0.536027f, 0.548631f, 0.561419f, 0.574389f,
	0.587542f, 0.600879f, 0.614399f, 0.628132f, 0.642018f, 0.656148f, 0.670431f, 0.684927f, 0.699637f,
	0.71453f, 0.729637f, 0.744926f, 0.76043f, 0.776147f, 0.792077f, 0.808191f, 0.824549f, 0.84109f,
	0.857845f, 0.874844f, 0.892056f, 0.909452f, 0.927122f, 0.945006f, 0.963073f, 0.981414f, 1.0f

};

// Second copy of the pan-to-gain curve, sampled by WaveCornerGains.
// GLOBAL: TONY2 0x004582bc
TonyFloat g_wavePanCurve2[3] = {
	0.0f, 0.7079f, 1.0f

};

// 16-bit gain scale for the hand-scheduled float-to-word conversion in WaveDriverSetGains.
// GLOBAL: TONY2 0x004582c8
TonyFloat g_waveGainWordScale = 32767.0f;

// Fixed-point fraction scale (1/4096) used by the hand-written mixer kernel.
// GLOBAL: TONY2 0x004580a0
TonyFloat g_waveStepFracScale = 0.000244140625f;

// Float-to-integer bias (1.5 * 2^23) used by the hand-written output converter: adding
// it leaves the sample in the mantissa so the integer subtract of the constant's own
// bit pattern recovers the value.
// GLOBAL: TONY2 0x004580b4
TonyFloat g_waveFloatToIntBias = 12582912.0f;

// Decay factor applied to the parked stop levels each pump so the residual offset of
// stopped channels fades out.
// GLOBAL: TONY2 0x004580b8
TonyFloat g_waveStopLevelDecay = 0.98f;

// Echo tap block addressed off this symbol by the hand-written echo processor: tap
// count at +0x00, per-tap gain pairs at +0x28, qword-strided delay offsets at +0xa8.
// GLOBAL: TONY2 0x004e62b0
TonyS32 g_waveEchoTaps;

// Echo delay line base (annotated in soundlib.c), write index and index mask.
extern void* g_mixBusPrimary;

// GLOBAL: TONY2 0x004e62b8
TonyU32 g_waveEchoIndex;

// GLOBAL: TONY2 0x004e62bc
TonyU32 g_waveEchoMask;

// Echo feed gain pair, wet return pair and dry pair.
// GLOBAL: TONY2 0x004e62c0
TonyFloat g_waveEchoFeedL;

// GLOBAL: TONY2 0x004e62c4
TonyFloat g_waveEchoFeedR;

// GLOBAL: TONY2 0x004e62c8
TonyFloat g_waveEchoWetL;

// GLOBAL: TONY2 0x004e62cc
TonyFloat g_waveEchoWetR;

// GLOBAL: TONY2 0x004e62d0
TonyFloat g_waveEchoDryL;

// GLOBAL: TONY2 0x004e62d4
TonyFloat g_waveEchoDryR;

// Wave-mixer configuration block loaded by WaveOpen.
// GLOBAL: TONY2 0x004e6458
TonyFloat g_waveStopLevelL;

// GLOBAL: TONY2 0x004e645c
TonyFloat g_waveStopLevelR;

// GLOBAL: TONY2 0x004e6460
TonyS32 g_waveChannelCount;

// GLOBAL: TONY2 0x004e6464
TonyS32 g_waveOutputRate;

// GLOBAL: TONY2 0x004e6468
TonyS32 g_waveFormatCode;

// GLOBAL: TONY2 0x004e646c
TonyFloat g_waveBlockMillis;

// GLOBAL: TONY2 0x004e6470
TonyS32 g_waveEchoBus;

// GLOBAL: TONY2 0x004e6474
TonyS32 g_waveDryBus;

// Mix-bus level pair parked by WaveMixAccumulate before accumulation so the channel's own
// contribution can be derived as new total minus old total.
// GLOBAL: TONY2 0x004e6478
TonyFloat g_waveOldTotalL;

// GLOBAL: TONY2 0x004e647c
TonyFloat g_waveOldTotalR;

// GLOBAL: TONY2 0x004e6480
TonyU8 g_waveInterpEnable;

// GLOBAL: TONY2 0x004e6481
TonyU8 g_waveEchoEnable;

// Active sound driver: 0 = wave, 1 = DirectSound.
// GLOBAL: TONY2 0x004e6b60
TonyU8 g_waveDriverMode;

// The wave-driver channels.
// GLOBAL: TONY2 0x004f0600
WaveChannel g_waveChannels[0x40];

// Empty master-level driver hook in this build of the library.
// FUNCTION: TONY2 0x0042410e
void __fastcall WaveMasterLevelHook(TonyS32 p_level)
{
}

// Empty wave service hook in this build of the library.
// FUNCTION: TONY2 0x00424119
void WaveServiceHook(void)
{
}

// Returns the driver play position of a device voice, in 16-bit samples.
// FUNCTION: TONY2 0x0042411e
TonyS32 __fastcall WaveGetPlayPosition(TonyS32 p_voice)
{
	return WaveDriverPosition(p_voice);
}

// Post-refill cache hook; nothing to do on this platform.
// FUNCTION: TONY2 0x00424131
void __fastcall WaveCacheFlushHook(TonyS16* p_samples, TonyU32 p_count)
{
}

// Copies the first four samples past the end of a voice ring so the resampler can
// read through the wrap.
// FUNCTION: TONY2 0x00424141
void __fastcall WaveRingWrapGuard(TonyS16* p_buffer, TonyS32 p_length)
{
	TonyS16* guard;

	guard = p_buffer;
	guard[p_length] = guard[0];
	guard[p_length + 1] = guard[1];
	guard[p_length + 2] = guard[2];
	guard[p_length + 3] = guard[3];
}

// Hand-written note-to-step scale: offsets the note byte against the base note packed
// in the rate dword's top byte (default 22050 Hz at note 0x40), scales the 24-bit rate
// fraction by the matching semitone pitch-table ratio and converts the result through
// WaveRateToStep.
// FUNCTION: TONY2 0x004241c0
__declspec(naked) TonyU16 __cdecl WaveNoteToStep(TonyU8 p_priority, TonyS32 p_level)
{
	__asm {
		push edx
		movzx eax, byte ptr [esp + 8]
		mov edx, dword ptr [esp + 0xc]
		test edx, 0xffffffff
		jne has_level
		mov edx, 0x40005622
	has_level:
		push edx
		shr edx, 0x18
		cmp al, dl
		jae scale_up
		sub dl, al
		and edx, 0xff
		lea edx, [edx*4 + g_wavePitchDownTable]
		jmp scaled
	scale_up:
		sub al, dl
		and eax, 0xff
		lea edx, [eax*4 + g_wavePitchUpTable]
	scaled:
		mov eax, dword ptr [esp]
		and eax, 0xffffff
		mul dword ptr [edx]
		shr eax, 0x14
		shl edx, 0xc
		or eax, edx
		push eax
		call WaveRateToStep
		add esp, 8
		pop edx
		ret
	}
}

// Hand-written semitone-up rate scale: value * 2^(1/12) in 12.20 fixed point. The
// original multiplier drops one bit of the pitch-table ratio (0x10d390 vs 0x10f390).
// FUNCTION: TONY2 0x0042421b
__declspec(naked) TonyS32 __cdecl WaveRateSemitoneUp(TonyU16 p_value)
{
	__asm {
		push edx
		movzx eax, word ptr [esp + 8]
		mov edx, 0x10d390
		mul edx
		shr eax, 0x14
		shl edx, 0xc
		or eax, edx
		pop edx
		ret
	}
}

// Hand-written semitone-down rate scale: value * 2^(-1/12) in 12.20 fixed point.
// FUNCTION: TONY2 0x00424232
__declspec(naked) TonyS32 __cdecl WaveRateSemitoneDown(TonyU16 p_value)
{
	__asm {
		push edx
		movzx eax, word ptr [esp + 8]
		mov edx, 0xf1a1c
		mul edx
		shr eax, 0x14
		shl edx, 0xc
		or eax, edx
		pop edx
		ret
	}
}

// Hand-written pitch-up rate scale: value * 2^(steps/12) in 12.20 fixed point.
// FUNCTION: TONY2 0x00424249
__declspec(naked) TonyS32 __cdecl WaveRatePitchUp(TonyU16 p_value, TonyS32 p_steps)
{
	__asm {
		push edx
		mov edx, dword ptr [esp + 0xc]
		movzx eax, word ptr [esp + 8]
		mov edx, dword ptr [edx*4 + g_wavePitchUpTable]
		mul edx
		shr eax, 0x14
		shl edx, 0xc
		or eax, edx
		pop edx
		ret
	}
}

// Hand-written pitch-down rate scale: value * 2^(-steps/12) in 12.20 fixed point.
// FUNCTION: TONY2 0x00424266
__declspec(naked) TonyS32 __cdecl WaveRatePitchDown(TonyU16 p_value, TonyS32 p_steps)
{
	__asm {
		push edx
		mov edx, dword ptr [esp + 0xc]
		movzx eax, word ptr [esp + 8]
		mov edx, dword ptr [edx*4 + g_wavePitchDownTable]
		mul edx
		shr eax, 0x14
		shl edx, 0xc
		or eax, edx
		pop edx
		ret
	}
}

// Hand-written fine pitch-up pair: splits the pitch offset into octave (quotient by
// 0x300) and fraction, scales the value by the fine table entries at the fraction and
// the one above it, and shifts both results up by the octave count.
// FUNCTION: TONY2 0x00424283
__declspec(naked) void __cdecl WaveRateFinePitchUpPair(TonyU16 p_pitch, TonyS32* p_low, TonyS32* p_high, TonyU16 p_value)
{
	__asm {
		push eax
		push edx
		push esi
		push edi
		push ebp
		push ecx
		push ebx
		movzx eax, word ptr [esp + 0x20]
		mov edx, dword ptr [esp + 0x24]
		mov ebx, dword ptr [esp + 0x28]
		movzx ecx, word ptr [esp + 0x2c]
		mov edi, edx
		mov ebp, ecx
		mov edx, 0
		mov esi, 0x300
		div esi
		mov ecx, eax
		lea esi, [edx*4 + g_waveFinePitchUpTable]
		mov eax, ebp
		mul dword ptr [esi]
		shr eax, 0x14
		shl edx, 0xc
		or eax, edx
		shl eax, cl
		mov dword ptr [edi], eax
		mov eax, ebp
		mul dword ptr [esi + 4]
		shr eax, 0x14
		shl edx, 0xc
		or eax, edx
		shl eax, cl
		mov dword ptr [ebx], eax
		pop ebx
		pop ecx
		pop ebp
		pop edi
		pop esi
		pop edx
		pop eax
		ret
	}
}

// Hand-written fine pitch-down pair: same split as WaveRateFinePitchUpPair against the descending
// fine table, shifting both results down by the octave count.
// FUNCTION: TONY2 0x004242de
__declspec(naked) void __cdecl WaveRateFinePitchDownPair(TonyU16 p_pitch, TonyS32* p_low, TonyS32* p_high, TonyU16 p_value)
{
	__asm {
		push eax
		push edx
		push esi
		push edi
		push ebp
		push ecx
		push ebx
		movzx eax, word ptr [esp + 0x20]
		mov edx, dword ptr [esp + 0x24]
		mov ebx, dword ptr [esp + 0x28]
		movzx ecx, word ptr [esp + 0x2c]
		mov edi, edx
		mov ebp, ecx
		mov edx, 0
		mov esi, 0x300
		div esi
		mov ecx, eax
		lea esi, [edx*4 + g_waveFinePitchDownTable]
		mov eax, ebp
		mul dword ptr [esi]
		shr eax, 0x14
		shl edx, 0xc
		or eax, edx
		shr eax, cl
		mov dword ptr [edi], eax
		mov eax, ebp
		mul dword ptr [esi + 4]
		shr eax, 0x14
		shl edx, 0xc
		or eax, edx
		shr eax, cl
		mov dword ptr [ebx], eax
		pop ebx
		pop ecx
		pop ebp
		pop edi
		pop esi
		pop edx
		pop eax
		ret
	}
}

// Hand-written level/pan to per-channel float gain pair. The level curve fraction is
// loaded with fld instead of fild in the original, so the interpolation term is a
// denormal that contributes nothing. A pan argument of 0x800000 selects the
// phase-inverted center mode used by the surround voice.
// FUNCTION: TONY2 0x00424339
__declspec(naked) void __cdecl WaveLevelPanToGains(TonyFloat* p_left, TonyFloat* p_right, TonyS32 p_level, TonyS32 p_pan)
{
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		mov eax, dword ptr [esp + 0x1c]
		mov edx, dword ptr [esp + 0x20]
		mov ebx, dword ptr [esp + 0x24]
		mov ecx, dword ptr [esp + 0x28]
		cmp ebx, 0x7f0000
		jb level_in_range
		mov ebx, 0x7f0000
	level_in_range:
		mov edi, ebx
		shr edi, 0x10
		lea esi, [edi*4 + g_waveLevelCurve]
		fld dword ptr [esi]
		and ebx, 0xffff
		push ebx
		mov ebx, 0x10000
		push ebx
		fld dword ptr [esp + 4]
		fidiv dword ptr [esp]
		add esp, 8
		fld dword ptr [esi + 4]
		fsub st, st(2)
		fmulp st(1), st
		faddp st(1), st
		cmp ecx, 0x800000
		jne pan_normal
		fmul dword ptr [g_waveCenterGain]
		fst dword ptr [eax]
		fldz
		fsubrp st(1), st
		fstp dword ptr [edx]
		jmp done
	pan_normal:
		mov edi, ecx
		mov esi, ecx
		and edi, 0x3fffff
		shr esi, 0x16
		push edi
		fld dword ptr [esi*4 + g_wavePanCurve]
		fld dword ptr [esi*4 + g_wavePanCurve + 4]
		fsub st, st(1)
		fild dword ptr [esp]
		add esp, 4
		fmul dword ptr [g_wavePanFracScale]
		fmulp st(1), st
		faddp st(1), st
		fld st(1)
		fmulp st(1), st
		fstp dword ptr [eax]
		mov edi, 0x7f0000
		sub edi, ecx
		mov esi, edi
		and edi, 0x3fffff
		shr esi, 0x16
		push edi
		fld dword ptr [esi*4 + g_wavePanCurve]
		fld dword ptr [esi*4 + g_wavePanCurve + 4]
		fsub st, st(1)
		fild dword ptr [esp]
		add esp, 4
		fmul dword ptr [g_wavePanFracScale]
		fmulp st(1), st
		faddp st(1), st
		fmulp st(1), st
		fstp dword ptr [edx]
	done:
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}

// Hand-written per-block wave pump (all registers preserved): walks the 0x54-byte
// channels, runs the envelope walker on active ones and copies (first user of a bus)
// or accumulates (later users) each into its mix bus, zero-fills untouched buses,
// post-processes them and converts to the negotiated output format.
// FUNCTION: TONY2 0x00424420
__declspec(naked) void __cdecl WaveMixBlock(TonyS32 p_step, TonyS32 p_frames, TonyS32 p_dest)
{
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		push ebp
		push edi
		push esi
		pushfd
		push es
		mov eax, dword ptr [esp + 0x28]
		mov edx, dword ptr [esp + 0x2c]
		mov ebx, dword ptr [esp + 0x30]
		mov cx, ds
		mov es, cx
		mov ecx, edx
		imul ecx, ecx, 0x3e8
		push ecx
		fild dword ptr [esp]
		fidiv dword ptr [g_waveOutputRate]
		add esp, 4
		fstp dword ptr [g_waveBlockMillis]
		mov ebp, 0
		mov ebx, edx
		mov ecx, dword ptr [g_waveChannelCount]
	next_channel:
		cmp byte ptr [eax + 0x16], 1
		jne advance
		call WaveEnvAdvance
		test byte ptr [eax + 0x17], 2
		jne bus_two
		mov edx, dword ptr [g_waveEchoBus]
		test ebp, 1
		jne accumulate
		or ebp, 1
		call WaveMixRender
		jmp advance
	bus_two:
		mov edx, dword ptr [g_waveDryBus]
		test ebp, 2
		jne accumulate
		or ebp, 2
		call WaveMixRender
		jmp advance
	accumulate:
		call WaveMixAccumulate
	advance:
		add eax, 0x54
		dec ecx
		jne next_channel
		test byte ptr [g_waveEchoEnable], 1
		je check_two
		test ebp, 1
		jne check_two
		mov edi, dword ptr [g_waveEchoBus]
		mov ecx, dword ptr [esp + 0x2c]
		mov eax, 0
		shl ecx, 1
		cld
		rep stosd
	check_two:
		test ebp, 2
		jne filter
		mov edi, dword ptr [g_waveDryBus]
		mov ecx, dword ptr [esp + 0x2c]
		mov eax, 0
		shl ecx, 1
		cld
		rep stosd
	filter:
		test byte ptr [g_waveEchoEnable], 1
		je filter_two
		mov eax, dword ptr [g_waveEchoBus]
		mov edx, dword ptr [esp + 0x2c]
		call WaveStopLevelBleed
	filter_two:
		mov eax, dword ptr [g_waveDryBus]
		mov edx, dword ptr [esp + 0x2c]
		call WaveStopLevelBleed
		test byte ptr [g_waveEchoEnable], 1
		je pick_format
		mov eax, dword ptr [g_waveEchoBus]
		mov edx, dword ptr [esp + 0x2c]
		call WaveEchoProcess
	pick_format:
		mov edx, dword ptr [esp + 0x30]
		mov ebx, dword ptr [esp + 0x2c]
		mov al, byte ptr [g_waveFormatCode]
		and al, 0xfd
		cmp al, 8
		je format_eight
		cmp al, 9
		je format_nine
		test byte ptr [g_waveEchoEnable], 1
		je sixteen_mono
		mov eax, dword ptr [g_waveEchoBus]
		mov ecx, dword ptr [g_waveDryBus]
		call WaveConvertDual16
		jmp done
	sixteen_mono:
		mov eax, dword ptr [g_waveDryBus]
		call WaveConvertSingle16
		jmp done
	format_nine:
		test byte ptr [g_waveEchoEnable], 1
		je nine_mono
		mov eax, dword ptr [g_waveEchoBus]
		mov ecx, dword ptr [g_waveDryBus]
		call WaveConvertDual8Stereo
		jmp done
	nine_mono:
		mov eax, dword ptr [g_waveDryBus]
		call WaveConvertSingle8Stereo
		jmp done
	format_eight:
		test byte ptr [g_waveEchoEnable], 1
		je eight_mono
		mov eax, dword ptr [g_waveEchoBus]
		mov ecx, dword ptr [g_waveDryBus]
		call WaveConvertDual8Mono
		jmp done
	eight_mono:
		mov eax, dword ptr [g_waveDryBus]
		call WaveConvertSingle8Mono
	done:
		pop es
		popfd
		pop esi
		pop edi
		pop ebp
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}

// Hand-written fade-in setup (channel in eax): arms the envelope gate and stage
// bytes, clears the level state and derives the per-millisecond ramp step from the
// output rate.
// FUNCTION: TONY2 0x004245c0
__declspec(naked) void WaveEnvStart(void)
{
	__asm {
		push ebx
		mov byte ptr [eax + 0x47], 1
		mov byte ptr [eax + 0x46], 0
		mov dword ptr [eax + 0x30], 0
		mov dword ptr [eax + 0x38], 0
		test word ptr [eax + 0x3c], 0xffff
		jne has_time
		fld1
		jmp store
	has_time:
		fild word ptr [eax + 0x3c]
	store:
		fst dword ptr [eax + 0x48]
		mov ebx, 0x3e8
		push ebx
		fild dword ptr [g_waveOutputRate]
		fmulp st(1), st
		fild dword ptr [esp]
		fdivrp st(1), st
		add esp, 4
		fstp dword ptr [eax + 0x34]
		pop ebx
		ret
	}
}

// Hand-written per-tick envelope walker (channel in eax, elapsed ticks in ebx):
// drives attack -> decay -> sustain -> release with per-millisecond ramp steps.
// FUNCTION: TONY2 0x00424604
__declspec(naked) void WaveEnvAdvance(void)
{
	__asm {
		push eax
		push ebx
		mov ebx, eax
		cmp byte ptr [ebx + 0x47], 0
		jne no_restart
		cmp byte ptr [ebx + 0x46], 3
		je no_restart
		mov byte ptr [ebx + 0x46], 3
		fldz
		fsub dword ptr [ebx + 0x38]
		test word ptr [ebx + 0x44], 0xffff
		jne has_out_time
		fld1
		jmp store_out
	has_out_time:
		fild word ptr [ebx + 0x44]
	store_out:
		fst dword ptr [ebx + 0x48]
		mov eax, 0x3e8
		push eax
		fimul dword ptr [g_waveOutputRate]
		fidiv dword ptr [esp]
		add esp, 4
		fdivp st(1), st
		fstp dword ptr [ebx + 0x34]
		jmp advance
	no_restart:
		cmp byte ptr [ebx + 0x46], 2
		je advance
		fld dword ptr [ebx + 0x48]
		fsub dword ptr [g_waveBlockMillis]
		ftst
		wait
		fnstsw ax
		fstp dword ptr [ebx + 0x48]
		sahf
		ja advance
		cmp byte ptr [ebx + 0x46], 0
		jne stage_hold
		mov byte ptr [ebx + 0x46], 1
		fld1
		fst dword ptr [ebx + 0x38]
		fsubr dword ptr [ebx + 0x40]
		test word ptr [ebx + 0x3e], 0xffff
		jne has_hold_time
		fld1
		jmp store_hold
	has_hold_time:
		fild word ptr [ebx + 0x3e]
	store_hold:
		fst dword ptr [ebx + 0x48]
		mov eax, 0x3e8
		push eax
		fimul dword ptr [g_waveOutputRate]
		fidiv dword ptr [esp]
		add esp, 4
		fdivp st(1), st
		fstp dword ptr [ebx + 0x34]
		jmp advance
	stage_hold:
		cmp byte ptr [ebx + 0x46], 1
		jne stage_end
		mov byte ptr [ebx + 0x46], 2
		mov eax, dword ptr [ebx + 0x40]
		mov dword ptr [ebx + 0x30], eax
		mov dword ptr [ebx + 0x38], eax
		mov dword ptr [ebx + 0x34], 0
		jmp advance
	stage_end:
		mov byte ptr [ebx + 0x16], 0
	advance:
		mov eax, dword ptr [ebx + 0x38]
		mov dword ptr [ebx + 0x30], eax
		cmp byte ptr [ebx + 0x46], 0
		jne ramp_hold
		fld dword ptr [ebx + 0x34]
		fimul dword ptr [esp]
		fadd dword ptr [ebx + 0x30]
		fst dword ptr [ebx + 0x38]
		fld1
		fcompp
		wait
		fnstsw ax
		sahf
		jae walker_done
		fld dword ptr [ebx + 0x30]
		fld1
		fst dword ptr [ebx + 0x38]
		fsubrp st(1), st
		fidiv dword ptr [esp]
		fstp dword ptr [ebx + 0x34]
		jmp walker_done
	ramp_hold:
		cmp byte ptr [ebx + 0x46], 1
		jne ramp_release
		fld dword ptr [ebx + 0x34]
		fimul dword ptr [esp]
		fadd dword ptr [ebx + 0x30]
		fst dword ptr [ebx + 0x38]
		test dword ptr [ebx + 0x34], 0xffffffff
		jns going_up
		fld dword ptr [ebx + 0x40]
		fcompp
		wait
		fnstsw ax
		sahf
		jbe walker_done
	clamp_hold:
		fld dword ptr [ebx + 0x30]
		fld dword ptr [ebx + 0x40]
		fst dword ptr [ebx + 0x38]
		fsubrp st(1), st
		fidiv dword ptr [esp]
		fstp dword ptr [ebx + 0x34]
		jmp walker_done
	going_up:
		fld dword ptr [ebx + 0x40]
		fcompp
		wait
		fnstsw ax
		sahf
		jae walker_done
		jmp clamp_hold
	ramp_release:
		cmp byte ptr [ebx + 0x46], 3
		jne walker_done
		fld dword ptr [ebx + 0x34]
		fimul dword ptr [esp]
		fadd dword ptr [ebx + 0x30]
		fst dword ptr [ebx + 0x38]
		fldz
		fcompp
		wait
		fnstsw ax
		sahf
		jbe walker_done
		fld dword ptr [ebx + 0x30]
		fldz
		fst dword ptr [ebx + 0x38]
		fsubrp st(1), st
		fidiv dword ptr [esp]
		fstp dword ptr [ebx + 0x34]
	walker_done:
		pop ebx
		pop eax
		ret
	}
}

// Hand-written release stub in the original wave driver: forces the sustain stage and
// drops the gate so the walker enters release, without touching any register but eax.
// FUNCTION: TONY2 0x0042476c
__declspec(naked) void __cdecl WaveChannelRelease(struct WaveChannel* p_channel)
{
	__asm {
		push eax
		mov eax, [esp + 8]
		mov byte ptr [eax + 0x46], 2
		mov byte ptr [eax + 0x47], 0
		pop eax
		ret
	}
}

// Hand-written echo-bypass setter, sibling of WaveChannelRelease: flips the channel's bus
// routing bit so it mixes into the dry bus that skips the echo processor.
// FUNCTION: TONY2 0x0042477b
__declspec(naked) void __cdecl WaveChannelEchoOff(struct WaveChannel* p_channel)
{
	__asm {
		push eax
		mov eax, [esp + 8]
		or byte ptr [eax + 0x17], 2
		pop eax
		ret
	}
}

// Hand-written wave-mixer setup: loads the configuration globals from the seven
// arguments and clears every channel's active byte.
// FUNCTION: TONY2 0x00424786
__declspec(naked) void __cdecl WaveOpen(
	struct WaveChannel* p_channels,
	TonyS32 p_b,
	TonyS32 p_c,
	TonyS32 p_d,
	TonyU8 p_e,
	TonyS32 p_f,
	TonyS32 p_g
)
{
	__asm {
		push ebp
		mov ebp, esp
		push eax
		push edx
		push esi
		push ecx
		push ebx
		mov eax, [ebp + 8]
		mov edx, [ebp + 0xc]
		mov ebx, [ebp + 0x10]
		mov ecx, [ebp + 0x14]
		mov dword ptr [g_waveOutputRate], ebx
		mov dword ptr [g_waveFormatCode], ecx
		mov bl, byte ptr [ebp + 0x18]
		mov byte ptr [g_waveInterpEnable], bl
		mov ebx, [ebp + 0x1c]
		mov dword ptr [g_waveEchoBus], ebx
		mov ebx, [ebp + 0x20]
		mov dword ptr [g_waveDryBus], ebx
		mov dword ptr [g_waveChannelCount], edx
		mov edx, 0x40
	clear_loop:
		mov byte ptr [eax + 0x16], 0
		add eax, 0x54
		dec edx
		jne clear_loop
		mov dword ptr [g_waveStopLevelL], edx
		mov dword ptr [g_waveStopLevelR], edx
		pop ebx
		pop ecx
		pop esi
		pop edx
		pop eax
		pop ebp
		ret
	}
}

// Empty software-mixer shutdown hook.
// FUNCTION: TONY2 0x004247e9
__declspec(naked) void WaveClose(void)
{
	__asm {
		ret
	}
}

// Hand-written channel start prep: derives the interpolation/bus-routing flags, arms
// the loop point and window, clears the accumulators and marks the channel active.
// FUNCTION: TONY2 0x004247ea
__declspec(naked) void __cdecl WaveChannelStart(struct WaveChannel* p_channel, TonyU8 p_flags)
{
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		mov eax, [esp + 0x14]
		mov dl, [esp + 0x18]
		and dl, byte ptr [g_waveInterpEnable]
		and dl, 1
		test byte ptr [g_waveEchoEnable], 1
		jne no_stereo
		or dl, 2
	no_stereo:
		mov [eax + 0x17], dl
		mov ebx, [eax]
		mov [eax + 0x18], ebx
		mov ebx, [eax + 0xc]
		cmp ebx, 0
		je no_loop
		mov ecx, [eax + 8]
		sub ecx, [eax]
		shr ecx, 1
		add ebx, ecx
		mov [eax + 0x1c], ebx
		mov ecx, [eax + 4]
		sub ecx, ebx
		cmp ecx, 0xa
		jae have_room
		xor ecx, ecx
	have_room:
		mov [eax + 0x10], ecx
		jmp joined
	no_loop:
		mov dword ptr [eax + 0x10], 0
		mov ebx, [eax + 4]
		mov [eax + 0x1c], ebx
	joined:
		mov dword ptr [eax + 0x20], 0
		mov dword ptr [eax + 0x24], 0
		mov dword ptr [eax + 0x4c], 0
		mov dword ptr [eax + 0x50], 0
		mov byte ptr [eax + 0x16], 1
		call WaveEnvStart
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}

// Hand-written channel stop: folds the channel's level accumulators into the global
// pair and clears the active byte.
// FUNCTION: TONY2 0x0042486f
__declspec(naked) void __cdecl WaveChannelStop(struct WaveChannel* p_channel)
{
	__asm {
		push eax
		mov eax, [esp + 8]
		mov byte ptr [eax + 0x16], 0
		fld dword ptr [eax + 0x4c]
		fld dword ptr [eax + 0x50]
		fxch st(1)
		fadd dword ptr [g_waveStopLevelL]
		fxch st(1)
		fadd dword ptr [g_waveStopLevelR]
		fxch st(1)
		fstp dword ptr [g_waveStopLevelL]
		fstp dword ptr [g_waveStopLevelR]
		pop eax
		ret
	}
}

// Hand-written gate-off: clears the hold byte so the loop finishes and the envelope
// walker enters its release stage.
// FUNCTION: TONY2 0x0042489e
__declspec(naked) void __cdecl WaveChannelGateOff(struct WaveChannel* p_channel)
{
	__asm {
		push eax
		mov eax, [esp + 8]
		mov byte ptr [eax + 0x47], 0
		pop eax
		ret
	}
}

// Hand-written 3.13 step derivation: (rate << 13) / output rate, rounded, capped at 2x.
// FUNCTION: TONY2 0x004248a9
__declspec(naked) TonyS32 __cdecl WaveRateToStep(TonyS32 p_rate)
{
	__asm {
		push edx
		mov eax, [esp + 8]
		mov edx, eax
		shl eax, 0xd
		shr edx, 0x13
		div dword ptr [g_waveOutputRate]
		inc eax
		shr eax, 1
		cmp eax, 0x4000
		jbe no_cap
		mov eax, 0x4000
	no_cap:
		pop edx
		ret
	}
}

// Hand-written wave mixer render kernel (channel in eax, frame count in ebx, float
// stereo output pairs in edx). Resamples the channel's 16-bit source through the 1.12
// fixed-point step at +0x14 in three bands: stereo with linear interpolation
// (step < 1.0), stereo skipping (step >= 1.0) and mono, applying the per-side gains at
// +0x28/+0x2c and the envelope level ramp at +0x24/+0x30/+0x34. Handles loop wrap
// (+0x08/+0x0c) and one-shot end (+0x10), zero-fills the remainder when the channel
// dies, and parks the last rendered pair at +0x4c/+0x50 for the stop accumulator.
// FUNCTION: TONY2 0x004248cd
__declspec(naked) void WaveMixRender(void)
{
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp
		sub esp, 0x14
		mov ecx, ebx
		mov esi, [eax + 0x18]
		mov edi, [eax + 0x1c]
		mov bp, word ptr [eax + 0x14]
		test byte ptr [eax + 0x17], 1
		je mono
		cmp bp, 0x1000
		jae st_fast
		mov ebx, [eax + 0x20]
		shl ebp, 0x14
		fld dword ptr [eax + 0x34]
		fild word ptr [eax + 0x14]
		fmul dword ptr [g_waveStepFracScale]
		fst dword ptr [esp + 8]
		fdivp st(1), st
		fstp dword ptr [esp + 0xc]
		fld dword ptr [eax + 0x30]
		fld dword ptr [eax + 0x24]
		fild word ptr [esi]
		fmul st, st(2)
		fld st(0)
		fmul dword ptr [eax + 0x2c]
		fxch st(1)
		fmul dword ptr [eax + 0x28]
	st_loop:
		fild word ptr [esi + 2]
		fmul st, st(4)
		fxch st(4)
		fadd dword ptr [esp + 0xc]
		fxch st(4)
		fld st(0)
		fmul dword ptr [eax + 0x2c]
		fxch st(1)
		fmul dword ptr [eax + 0x28]
		fld st(1)
		fstp dword ptr [esp + 4]
		fst dword ptr [esp]
		fsub st, st(2)
		fld st(3)
		fsubp st(2), st
	st_interp:
		fld st(0)
		fmul st, st(5)
		fld st(2)
		fmul st, st(6)
		fxch st(1)
		fadd st, st(4)
		fxch st(1)
		fadd st, st(5)
		fxch st(6)
		fadd dword ptr [esp + 8]
		fxch st(6)
		fstp dword ptr [edx + 4]
		fstp dword ptr [edx]
		add ebx, ebp
		dec ecx
		je st_count_done
		lea edx, [edx + 8]
		jae st_interp
		fcompp
		fcompp
		fld1
		fsubp st(1), st
		fld dword ptr [esp + 4]
		fld dword ptr [esp]
		add esi, 2
		dec edi
		jne st_loop
		cmp byte ptr [eax + 0x47], 0
		je st_endchk
	st_wrap:
		mov esi, [eax + 8]
		mov edi, [eax + 0xc]
		cmp edi, 0
		jne st_loop
	st_dead:
		mov byte ptr [eax + 0x16], 0
		fcompp
		fcompp
	zero_fill:
		mov ebp, 0
	zero_loop:
		mov [edx], ebp
		mov [edx + 4], ebp
		add edx, 8
		dec ecx
		ja zero_loop
		jmp pop_exit
	st_endchk:
		mov edi, [eax + 0x10]
		cmp edi, 0
		je st_wrap
		cmp edi, -1
		je st_dead
		mov dword ptr [eax + 0x10], 0xffffffff
		jmp st_loop
	st_count_done:
		lea edx, [edx + 8]
		fcompp
		fcompp
		jae st_store
		fld1
		fsubp st(1), st
		add esi, 2
		dec edi
		jne st_store
		mov edi, [eax + 0xc]
		cmp edi, 0
		jne st_cd_wrapchk
	st_cd_dead:
		mov byte ptr [eax + 0x16], 0
		fcompp
		jmp tail_copy
	st_cd_wrapchk:
		cmp byte ptr [eax + 0x47], 0
		je st_cd_endchk
	st_cd_wrap:
		mov edi, [eax + 0xc]
		mov esi, [eax + 8]
		jmp st_store
	st_cd_endchk:
		mov edi, [eax + 0x10]
		cmp edi, 0
		je st_cd_wrap
		jl st_cd_dead
		mov dword ptr [eax + 0x10], 0xffffffff
	st_store:
		fstp dword ptr [eax + 0x24]
		mov [eax + 0x18], esi
		mov [eax + 0x1c], edi
		mov [eax + 0x20], ebx
		fstp dword ptr [eax + 0x30]
	tail_copy:
		mov ebx, [edx - 8]
		mov ecx, [edx - 4]
		mov [eax + 0x4c], ebx
		mov [eax + 0x50], ecx
	pop_exit:
		add esp, 0x14
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	st_fast:
		fld1
		mov ebx, ebp
		and ebx, 0xfff
		mov [esp + 0x10], ebx
		fild dword ptr [esp + 0x10]
		fmul dword ptr [g_waveStepFracScale]
		fstp dword ptr [esp + 0x10]
		and ebp, 0xffff
		rol ebp, 0x14
		mov ebx, ebp
		and ebp, 0xfff00000
		and ebx, 0xfffff
		mov [esp], ebx
		shl ebx, 1
		mov [esp + 4], ebx
		mov ebx, [eax + 0x20]
		fld dword ptr [eax + 0x24]
		fld dword ptr [eax + 0x30]
		fld dword ptr [eax + 0x34]
	stf_loop:
		fild word ptr [esi]
		fild word ptr [esi + 2]
		fsub st, st(1)
		fmul st, st(4)
		faddp st(1), st
		fmul st, st(2)
		fxch st(2)
		fadd st, st(1)
		fxch st(2)
		fld st(0)
		fmul dword ptr [eax + 0x2c]
		fxch st(1)
		fmul dword ptr [eax + 0x28]
		fxch st(1)
		fstp dword ptr [edx + 4]
		fstp dword ptr [edx]
		add edx, 8
		add ebx, ebp
		jae stf_carry
		fxch st(2)
		fadd dword ptr [esp + 0x10]
		fsub st, st(3)
		fxch st(2)
		sbb edi, [esp]
		jg stf_next
		cmp byte ptr [eax + 0x47], 0
		je stf_endchk
	stf_wrapchk:
		test dword ptr [eax + 0xc], 0xffffffff
		je stf_dead
		mov esi, [eax + 8]
		sub esi, edi
		sub esi, edi
		add edi, [eax + 0xc]
		dec ecx
		jne stf_loop
		jmp stf_done
	stf_endchk:
		cmp dword ptr [eax + 0x10], 0
		je stf_wrapchk
		cmp dword ptr [eax + 0x10], -1
		je stf_dead
		add edi, [eax + 0x10]
		mov dword ptr [eax + 0x10], 0xffffffff
	stf_next:
		add esi, [esp + 4]
		add esi, 2
		dec ecx
		jne stf_loop
		jmp stf_done
	stf_dead:
		mov byte ptr [eax + 0x16], 0
		fcompp
		fcompp
		jmp zero_fill
	stf_carry:
		fxch st(2)
		fadd dword ptr [esp + 0x10]
		fxch st(2)
		sub edi, [esp]
		jg stf_carry_next
		cmp byte ptr [eax + 0x47], 0
		je stf_endchk
		test dword ptr [eax + 0xc], 0xffffffff
		je stf_dead
		mov esi, [eax + 8]
		sub esi, edi
		sub esi, edi
		add edi, [eax + 0xc]
		dec ecx
		jne stf_loop
		jmp stf_done
	stf_carry_next:
		add esi, [esp + 4]
		dec ecx
		jne stf_loop
	stf_done:
		fcomp st(1)
		fstp dword ptr [eax + 0x30]
		fstp dword ptr [eax + 0x24]
		fcomp st(1)
		mov [eax + 0x18], esi
		mov [eax + 0x1c], edi
		mov [eax + 0x20], ebx
		jmp tail_copy
	mono:
		and ebp, 0xffff
		rol ebp, 0x14
		mov ebx, ebp
		and ebp, 0xfff00000
		and ebx, 0xfffff
		mov [esp], ebx
		shl ebx, 1
		mov [esp + 4], ebx
		mov ebx, [eax + 0x20]
		fld dword ptr [eax + 0x30]
		fld dword ptr [eax + 0x34]
	m_loop:
		fild word ptr [esi]
		fmul st, st(2)
		fxch st(2)
		fadd st, st(1)
		fxch st(2)
		fld st(0)
		fmul dword ptr [eax + 0x2c]
		fxch st(1)
		fmul dword ptr [eax + 0x28]
		fxch st(1)
		fstp dword ptr [edx + 4]
		fstp dword ptr [edx]
		add edx, 8
		add ebx, ebp
		jae m_carry
		sbb edi, [esp]
		jg m_next
		cmp byte ptr [eax + 0x47], 0
		je m_endchk
	m_wrapchk:
		test dword ptr [eax + 0xc], 0xffffffff
		je m_dead
		mov esi, [eax + 8]
		sub esi, edi
		sub esi, edi
		add edi, [eax + 0xc]
		dec ecx
		jne m_loop
		jmp m_done
	m_endchk:
		cmp dword ptr [eax + 0x10], 0
		je m_wrapchk
		cmp dword ptr [eax + 0x10], -1
		je m_dead
		add edi, [eax + 0x10]
		mov dword ptr [eax + 0x10], 0xffffffff
	m_next:
		add esi, [esp + 4]
		add esi, 2
		dec ecx
		jne m_loop
		jmp m_done
	m_dead:
		mov byte ptr [eax + 0x16], 0
		fcompp
		jmp zero_fill
	m_carry:
		sub edi, [esp]
		jg m_carry_next
		cmp byte ptr [eax + 0x47], 0
		je m_endchk
		test dword ptr [eax + 0xc], 0xffffffff
		je m_dead
		mov esi, [eax + 8]
		sub esi, edi
		sub esi, edi
		add edi, [eax + 0xc]
		dec ecx
		jne m_loop
		jmp m_done
	m_carry_next:
		add esi, [esp + 4]
		dec ecx
		jne m_loop
	m_done:
		fcomp st(1)
		fstp dword ptr [eax + 0x30]
		mov [eax + 0x18], esi
		mov [eax + 0x1c], edi
		mov [eax + 0x20], ebx
		mov dword ptr [eax + 0x24], 0
		jmp tail_copy
	}
}

// Hand-written wave mixer accumulate kernel: the mixing twin of WaveMixRender (same
// register convention and band structure) that adds the channel into an already
// rendered float bus instead of overwriting it. Parks the bus's previous last pair in
// g_waveOldTotalL/g_waveOldTotalR on entry and derives the channel's own contribution for
// +0x4c/+0x50 as new total minus old total; dead channels simply stop accumulating
// instead of zero-filling. A copy of the render kernel's end-check band survives
// unreachable at the original 0x00424d72.
// FUNCTION: TONY2 0x00424c3e
__declspec(naked) void WaveMixAccumulate(void)
{
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp
		sub esp, 0x14
		mov edi, [edx + ebx * 8 - 8]
		mov esi, [edx + ebx * 8 - 4]
		mov dword ptr [g_waveOldTotalL], edi
		mov dword ptr [g_waveOldTotalR], esi
		mov ecx, ebx
		mov esi, [eax + 0x18]
		mov edi, [eax + 0x1c]
		mov bp, word ptr [eax + 0x14]
		test byte ptr [eax + 0x17], 1
		je mono
		cmp bp, 0x1000
		jae st_fast
		mov ebx, [eax + 0x20]
		shl ebp, 0x14
		fld dword ptr [eax + 0x34]
		fild word ptr [eax + 0x14]
		fmul dword ptr [g_waveStepFracScale]
		fst dword ptr [esp + 8]
		fdivp st(1), st
		fstp dword ptr [esp + 0xc]
		fld dword ptr [eax + 0x30]
		fld dword ptr [eax + 0x24]
		fild word ptr [esi]
		fmul st, st(2)
		fld st(0)
		fmul dword ptr [eax + 0x2c]
		fxch st(1)
		fmul dword ptr [eax + 0x28]
	st_loop:
		fild word ptr [esi + 2]
		fmul st, st(4)
		fxch st(4)
		fadd dword ptr [esp + 0xc]
		fxch st(4)
		fld st(0)
		fmul dword ptr [eax + 0x2c]
		fxch st(1)
		fmul dword ptr [eax + 0x28]
		fld st(1)
		fstp dword ptr [esp + 4]
		fst dword ptr [esp]
		fsub st, st(2)
		fld st(3)
		fsubp st(2), st
	st_interp:
		fld st(0)
		fmul st, st(5)
		fld st(2)
		fmul st, st(6)
		fxch st(1)
		fadd st, st(4)
		fxch st(1)
		fadd st, st(5)
		fxch st(6)
		fadd dword ptr [esp + 8]
		fxch st(6)
		fadd dword ptr [edx + 4]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [edx + 4]
		fstp dword ptr [edx]
		add ebx, ebp
		dec ecx
		je st_count_done
		lea edx, [edx + 8]
		jae st_interp
		fcompp
		fcompp
		fld1
		fsubp st(1), st
		fld dword ptr [esp + 4]
		fld dword ptr [esp]
		add esi, 2
		dec edi
		jne st_loop
		cmp byte ptr [eax + 0x47], 0
		je st_endchk
	st_wrap:
		mov esi, [eax + 8]
		mov edi, [eax + 0xc]
		cmp edi, 0
		jne st_loop
	st_dead:
		mov byte ptr [eax + 0x16], 0
		fcompp
		fcompp
		jmp tail_delta
	st_endchk:
		mov edi, [eax + 0x10]
		cmp edi, 0
		je st_wrap
		cmp edi, -1
		je st_dead
		mov dword ptr [eax + 0x10], 0xffffffff
		jmp st_loop
	st_count_done:
		lea edx, [edx + 8]
		fcompp
		fcompp
		jae st_store
		fld1
		fsubp st(1), st
		add esi, 2
		dec edi
		jne st_store
		mov esi, [eax + 8]
		mov edi, [eax + 0xc]
		cmp edi, 0
		jne st_store
	st_cd_dead:
		mov byte ptr [eax + 0x16], 0
		fcompp
		jmp tail_delta
	st_cd_deadband:
		cmp byte ptr [eax + 0x47], 0
		je st_cd_endchk
	st_cd_wrap:
		mov edi, [eax + 0xc]
		mov esi, [eax + 8]
		jmp st_store
	st_cd_endchk:
		mov edi, [eax + 0x10]
		cmp edi, 0
		je st_cd_wrap
		jl st_cd_dead
		mov dword ptr [eax + 0x10], 0xffffffff
	st_store:
		fstp dword ptr [eax + 0x24]
		mov [eax + 0x18], esi
		mov [eax + 0x1c], edi
		mov [eax + 0x20], ebx
		fstp dword ptr [eax + 0x30]
	tail_delta:
		fld dword ptr [edx - 8]
		fld dword ptr [edx - 4]
		fxch st(1)
		fsub dword ptr [g_waveOldTotalL]
		fxch st(1)
		fsub dword ptr [g_waveOldTotalR]
		fxch st(1)
		fstp dword ptr [eax + 0x4c]
		fstp dword ptr [eax + 0x50]
	pop_exit:
		add esp, 0x14
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	st_fast:
		fld1
		mov ebx, ebp
		and ebx, 0xfff
		mov [esp + 0x10], ebx
		fild dword ptr [esp + 0x10]
		fmul dword ptr [g_waveStepFracScale]
		fstp dword ptr [esp + 0x10]
		and ebp, 0xffff
		rol ebp, 0x14
		mov ebx, ebp
		and ebp, 0xfff00000
		and ebx, 0xfffff
		mov [esp], ebx
		shl ebx, 1
		mov [esp + 4], ebx
		mov ebx, [eax + 0x20]
		fld dword ptr [eax + 0x24]
		fld dword ptr [eax + 0x30]
		fld dword ptr [eax + 0x34]
	stf_loop:
		fild word ptr [esi]
		fild word ptr [esi + 2]
		fsub st, st(1)
		fmul st, st(4)
		faddp st(1), st
		fmul st, st(2)
		fxch st(2)
		fadd st, st(1)
		fxch st(2)
		fld st(0)
		fmul dword ptr [eax + 0x2c]
		fxch st(1)
		fmul dword ptr [eax + 0x28]
		fxch st(1)
		fadd dword ptr [edx + 4]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [edx + 4]
		fstp dword ptr [edx]
		add edx, 8
		add ebx, ebp
		jae stf_carry
		fxch st(2)
		fadd dword ptr [esp + 0x10]
		fsub st, st(3)
		fxch st(2)
		sbb edi, [esp]
		jg stf_next
		cmp byte ptr [eax + 0x47], 0
		je stf_endchk
	stf_wrapchk:
		test dword ptr [eax + 0xc], 0xffffffff
		je stf_dead
		mov esi, [eax + 8]
		sub esi, edi
		sub esi, edi
		add edi, [eax + 0xc]
		dec ecx
		jne stf_loop
		jmp stf_done
	stf_dead:
		mov byte ptr [eax + 0x16], 0
		fcompp
		fcompp
		jmp pop_exit
	stf_endchk:
		cmp dword ptr [eax + 0x10], 0
		je stf_wrapchk
		cmp dword ptr [eax + 0x10], -1
		je stf_dead
		add edi, [eax + 0x10]
		mov dword ptr [eax + 0x10], 0xffffffff
	stf_next:
		add esi, [esp + 4]
		add esi, 2
		dec ecx
		jne stf_loop
		jmp stf_done
	stf_carry:
		fxch st(2)
		fadd dword ptr [esp + 0x10]
		fxch st(2)
		sub edi, [esp]
		jg stf_carry_next
		cmp byte ptr [eax + 0x47], 0
		je stf_endchk
		test dword ptr [eax + 0xc], 0xffffffff
		je stf_dead
		mov esi, [eax + 8]
		sub esi, edi
		sub esi, edi
		add edi, [eax + 0xc]
		dec ecx
		jne stf_loop
		jmp stf_done
	stf_carry_next:
		add esi, [esp + 4]
		dec ecx
		jne stf_loop
	stf_done:
		fcomp st(1)
		fstp dword ptr [eax + 0x30]
		fstp dword ptr [eax + 0x24]
		fcomp st(1)
		mov [eax + 0x18], esi
		mov [eax + 0x1c], edi
		mov [eax + 0x20], ebx
		jmp tail_delta
	mono:
		and ebp, 0xffff
		rol ebp, 0x14
		mov ebx, ebp
		and ebp, 0xfff00000
		and ebx, 0xfffff
		mov [esp], ebx
		shl ebx, 1
		mov [esp + 4], ebx
		mov ebx, [eax + 0x20]
		fld dword ptr [eax + 0x30]
		fld dword ptr [eax + 0x34]
	m_loop:
		fild word ptr [esi]
		fmul st, st(2)
		fxch st(2)
		fadd st, st(1)
		fxch st(2)
		fld st(0)
		fmul dword ptr [eax + 0x2c]
		fxch st(1)
		fmul dword ptr [eax + 0x28]
		fxch st(1)
		fadd dword ptr [edx + 4]
		fxch st(1)
		fadd dword ptr [edx]
		fxch st(1)
		fstp dword ptr [edx + 4]
		fstp dword ptr [edx]
		add edx, 8
		add ebx, ebp
		jae m_carry
		sbb edi, [esp]
		jg m_next
		cmp byte ptr [eax + 0x47], 0
		je m_endchk
	m_wrapchk:
		test dword ptr [eax + 0xc], 0xffffffff
		je m_dead
		mov esi, [eax + 8]
		sub esi, edi
		sub esi, edi
		add edi, [eax + 0xc]
		dec ecx
		jne m_loop
		jmp m_done
	m_endchk:
		cmp dword ptr [eax + 0x10], 0
		je m_wrapchk
		cmp dword ptr [eax + 0x10], -1
		je m_dead
		add edi, [eax + 0x10]
		mov dword ptr [eax + 0x10], 0xffffffff
	m_next:
		add esi, [esp + 4]
		add esi, 2
		dec ecx
		jne m_loop
		jmp m_done
	m_dead:
		mov byte ptr [eax + 0x16], 0
		fcompp
		jmp tail_delta
	m_carry:
		sub edi, [esp]
		jg m_carry_next
		cmp byte ptr [eax + 0x47], 0
		je m_endchk
		test dword ptr [eax + 0xc], 0xffffffff
		je m_dead
		mov esi, [eax + 8]
		sub esi, edi
		sub esi, edi
		add edi, [eax + 0xc]
		dec ecx
		jne m_loop
		jmp m_done
	m_carry_next:
		add esi, [esp + 4]
		dec ecx
		jne m_loop
	m_done:
		fcomp st(1)
		fstp dword ptr [eax + 0x30]
		mov [eax + 0x18], esi
		mov [eax + 0x1c], edi
		mov [eax + 0x20], ebx
		mov dword ptr [eax + 0x24], 0
		jmp tail_delta
	}
}

// Hand-written output converter (float bus pair in eax and ecx, frame count in ebx,
// interleaved 16-bit output in edx): sums the two buses, converts through the
// g_waveFloatToIntBias mantissa bias, saturates to +-0x7fff and stores the word pair with the
// channel order selected by bit 1 of g_waveFormatCode.
// FUNCTION: TONY2 0x00424fe7
__declspec(naked) void WaveConvertDual16(void)
{
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		sub esp, 8
		fld dword ptr [g_waveFloatToIntBias]
		mov edi, 0xfffffffc
		mov esi, 0xfffffffe
		test byte ptr [g_waveFormatCode], 2
		je conv_loop
		xchg edi, esi
	conv_loop:
		fld dword ptr [eax]
		fld dword ptr [eax + 4]
		fadd dword ptr [ecx + 4]
		fxch st(1)
		fadd dword ptr [ecx]
		fxch st(1)
		fadd st, st(2)
		add eax, 8
		add ecx, 8
		add edx, 4
		fstp dword ptr [esp]
		fadd st, st(1)
		mov ebp, [esp]
		sub ebp, dword ptr [g_waveFloatToIntBias]
		cmp ebp, 0x7fff
		jle clamp_lo_l
		mov ebp, 0x7fff
		jmp store_l
	clamp_lo_l:
		cmp ebp, 0xffff8000
		jge store_l
		mov ebp, 0xffff8000
	store_l:
		fstp dword ptr [esp + 4]
		mov word ptr [edi + edx], bp
		mov ebp, [esp + 4]
		sub ebp, dword ptr [g_waveFloatToIntBias]
		cmp ebp, 0x7fff
		jle clamp_lo_r
		mov ebp, 0x7fff
		jmp store_r
	clamp_lo_r:
		cmp ebp, 0xffff8000
		jge store_r
		mov ebp, 0xffff8000
	store_r:
		dec ebx
		mov word ptr [esi + edx], bp
		jne conv_loop
		fcomp st(1)
		add esp, 8
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}

// Hand-written single-bus output converter (float bus in eax, frame count in ebx,
// interleaved 16-bit output in edx): fistp variant of WaveConvertDual16 that saturates from
// the 64-bit integer result's high dword instead of the mantissa-bias trick.
// FUNCTION: TONY2 0x0042508f
__declspec(naked) void WaveConvertSingle16(void)
{
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		sub esp, 0x10
		mov esi, 0xfffffffc
		mov edi, 0xfffffffe
		test byte ptr [g_waveFormatCode], 2
		je cv_loop
		xchg edi, esi
	cv_loop:
		fld dword ptr [eax]
		fld dword ptr [eax + 4]
		fxch st(0)
		fistp qword ptr [esp]
		add eax, 8
		add edx, 4
		fistp qword ptr [esp + 8]
		cmp dword ptr [esp + 4], -1
		mov ebp, [esp]
		jl clamp_lo_l
		jne chk_hi_l
		cmp ebp, 0xffff8000
		jge chk_hi_l
	clamp_lo_l:
		mov ebp, 0xffff8000
		jmp store_l
	chk_hi_l:
		cmp dword ptr [esp + 4], 0
		jg clamp_hi_l
		cmp ebp, 0x7fff
		jle store_l
	clamp_hi_l:
		mov ebp, 0x7fff
	store_l:
		mov word ptr [esi + edx], bp
		cmp dword ptr [esp + 0xc], -1
		mov ebp, [esp + 8]
		jl clamp_lo_r
		jne chk_hi_r
		cmp ebp, 0xffff8000
		jge chk_hi_r
	clamp_lo_r:
		mov ebp, 0xffff8000
		jmp store_r
	chk_hi_r:
		cmp dword ptr [esp + 0xc], 0
		jg clamp_hi_r
		cmp ebp, 0x7fff
		jle store_r
	clamp_hi_r:
		mov ebp, 0x7fff
	store_r:
		mov word ptr [edi + edx], bp
		dec ebx
		jne cv_loop
		add esp, 0x10
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}

// Hand-written single-bus 8-bit output converter (float bus in eax, frame count in
// ebx, mono 8-bit output in edx): sums each stereo pair, saturates to 16 bits and
// stores the high byte biased to unsigned.
// FUNCTION: TONY2 0x00425135
__declspec(naked) void WaveConvertSingle8Mono(void)
{
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp
		sub esp, 4
		mov esi, 0x7fff
		mov edi, 0xffff8000
	c8_loop:
		fld dword ptr [eax]
		fadd dword ptr [eax + 4]
		fistp dword ptr [esp]
		add eax, 8
		inc edx
		mov ecx, [esp]
		cmp ecx, esi
		jle chk_lo
		mov ecx, esi
		jmp c8_store
	chk_lo:
		cmp ecx, edi
		jge c8_store
		mov ecx, edi
	c8_store:
		add ch, 0x80
		mov byte ptr [edx - 1], ch
		dec ebx
		jne c8_loop
		add esp, 4
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}

// Hand-written two-bus 8-bit output converter (float buses in eax and ecx, frame
// count in ebx, mono 8-bit output in edx): WaveConvertSingle8Mono with a second bus mixed in.
// FUNCTION: TONY2 0x0042517a
__declspec(naked) void WaveConvertDual8Mono(void)
{
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp
		sub esp, 4
		mov esi, ecx
	c8_loop:
		fld dword ptr [eax]
		fadd dword ptr [eax + 4]
		add eax, 8
		fadd dword ptr [esi]
		inc edx
		fadd dword ptr [esi + 4]
		add esi, 8
		fistp dword ptr [esp]
		mov ecx, [esp]
		cmp ecx, 0x7fff
		jle chk_lo
		mov ecx, 0x7fff
		jmp c8_store
	chk_lo:
		cmp ecx, 0xffff8000
		jge c8_store
		mov ecx, 0xffff8000
	c8_store:
		add ch, 0x80
		mov byte ptr [edx - 1], ch
		dec ebx
		jne c8_loop
		add esp, 4
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}

// Hand-written single-bus stereo 8-bit output converter (float bus in eax, frame
// count in ebx, interleaved 8-bit output in edx): stores the saturated high bytes with
// the channel order selected by bit 1 of g_waveFormatCode.
// FUNCTION: TONY2 0x004251cd
__declspec(naked) void WaveConvertSingle8Stereo(void)
{
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp
		sub esp, 8
		mov edi, 0xfffffffe
		mov esi, 0xffffffff
		test byte ptr [g_waveFormatCode], 2
		je s8_loop
		xchg edi, esi
	s8_loop:
		fld dword ptr [eax]
		fld dword ptr [eax + 4]
		fistp dword ptr [esp + 4]
		add eax, 8
		add edx, 2
		fistp dword ptr [esp]
		mov ecx, [esp]
		cmp ecx, 0x7fff
		jle s8_chklo_l
		mov ecx, 0x7fff
		jmp s8_store_l
	s8_chklo_l:
		cmp ecx, 0xffff8000
		jge s8_store_l
		mov ecx, 0xffff8000
	s8_store_l:
		mov byte ptr [edi + edx], ch
		mov ecx, [esp + 4]
		cmp ecx, 0x7fff
		jle s8_chklo_r
		mov ecx, 0x7fff
		jmp s8_store_r
	s8_chklo_r:
		cmp ecx, 0xffff8000
		jge s8_store_r
		mov ecx, 0xffff8000
	s8_store_r:
		mov byte ptr [esi + edx], ch
		dec ebx
		jne s8_loop
		add esp, 8
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}

// Hand-written two-bus stereo 8-bit output converter (float buses in eax and ecx,
// frame count in ebx, interleaved 8-bit output in edx).
// FUNCTION: TONY2 0x00425251
__declspec(naked) void WaveConvertDual8Stereo(void)
{
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp
		sub esp, 8
		mov esi, ecx
		mov edi, edx
		mov ebp, ebx
		mov edx, 0xffffffff
		mov ebx, 0xfffffffe
		test byte ptr [g_waveFormatCode], 2
		je s8x_loop
		xchg edx, ebx
	s8x_loop:
		fld dword ptr [eax]
		fld dword ptr [eax + 4]
		fadd dword ptr [esi + 4]
		fxch st(1)
		fadd dword ptr [esi]
		fxch st(1)
		fistp dword ptr [esp + 4]
		add eax, 8
		add esi, 8
		add edi, 2
		fistp dword ptr [esp]
		mov ecx, [esp + 4]
		cmp ecx, 0x7fff
		jle x8_chklo_l
		mov ecx, 0x7fff
		jmp x8_store_l
	x8_chklo_l:
		cmp ecx, 0xffff8000
		jge x8_store_l
		mov ecx, 0xffff8000
	x8_store_l:
		mov byte ptr [edx + edi], ch
		mov ecx, [esp]
		cmp ecx, 0x7fff
		jle x8_chklo_r
		mov ecx, 0x7fff
		jmp x8_store_r
	x8_chklo_r:
		cmp ecx, 0xffff8000
		jge x8_store_r
		mov ecx, 0xffff8000
	x8_store_r:
		mov byte ptr [ebx + edi], ch
		dec ebp
		jne s8x_loop
		add esp, 8
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}

// Hand-written echo processor (float bus in eax, frame count in edx): per frame, sums
// the taps of g_waveEchoTaps from the ring delay line, feeds the input into the line
// through the g_waveEchoFeedL pair (rounded to keep the line integral), and remixes the
// bus as dry (g_waveEchoDryL pair) plus wet return (g_waveEchoWetL pair).
// FUNCTION: TONY2 0x004252e7
__declspec(naked) void WaveEchoProcess(void)
{
	__asm {
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		push ebp
		mov esi, eax
		mov ecx, edx
		mov eax, dword ptr [g_waveEchoIndex]
		mov ebp, dword ptr [g_waveEchoMask]
		mov edx, dword ptr [g_mixBusPrimary]
	frame_loop:
		push eax
		push ecx
		fldz
		fld st(0)
		mov edi, offset g_waveEchoTaps
		mov ecx, dword ptr [edi]
	tap_loop:
		add eax, dword ptr [edi + 0xa8]
		and eax, ebp
		fld dword ptr [edx + eax * 8 + 4]
		fld dword ptr [edx + eax * 8]
		fmul dword ptr [edi + 0x28]
		fxch st(1)
		fmul dword ptr [edi + 0x2c]
		fxch st(1)
		faddp st(2), st
		faddp st(2), st
		add edi, 8
		dec ecx
		jne tap_loop
		pop ecx
		pop eax
		fld dword ptr [esi + 4]
		fld dword ptr [esi]
		fld st(3)
		fld st(3)
		fmul dword ptr [g_waveEchoFeedL]
		fxch st(1)
		fmul dword ptr [g_waveEchoFeedR]
		fxch st(1)
		fadd st, st(2)
		fxch st(1)
		fadd st, st(3)
		fxch st(1)
		frndint
		fstp dword ptr [edx + eax * 8]
		frndint
		fstp dword ptr [edx + eax * 8 + 4]
		fmul dword ptr [g_waveEchoWetL]
		fxch st(1)
		fmul dword ptr [g_waveEchoWetR]
		fxch st(3)
		fmul dword ptr [g_waveEchoDryR]
		fxch st(2)
		fmul dword ptr [g_waveEchoDryL]
		fxch st(3)
		faddp st(2), st
		faddp st(2), st
		fstp dword ptr [esi + 4]
		fstp dword ptr [esi]
		dec eax
		and eax, ebp
		add esi, 8
		dec ecx
		jne frame_loop
		mov dword ptr [g_waveEchoIndex], eax
		pop ebp
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}

// Hand-written stop-level bleed (float bus in eax, frame count in edx): while the
// parked stop levels are nonzero, decays them through g_waveStopLevelDecay and adds the
// offset across the bus so stopped channels release without a click.
// FUNCTION: TONY2 0x004253a4
__declspec(naked) void WaveStopLevelBleed(void)
{
	__asm {
		push eax
		push ecx
		push edx
		mov ecx, dword ptr [g_waveStopLevelL]
		or ecx, dword ptr [g_waveStopLevelR]
		je bleed_done
		fld dword ptr [g_waveStopLevelL]
		fld dword ptr [g_waveStopLevelR]
		fxch st(1)
		fmul dword ptr [g_waveStopLevelDecay]
		fxch st(1)
		fmul dword ptr [g_waveStopLevelDecay]
	add_loop:
		fld dword ptr [eax]
		fld dword ptr [eax + 4]
		fxch st(1)
		fadd st, st(3)
		fxch st(1)
		fadd st, st(2)
		fxch st(1)
		fstp dword ptr [eax]
		fstp dword ptr [eax + 4]
		add eax, 8
		dec edx
		jne add_loop
		frndint
		fstp dword ptr [g_waveStopLevelR]
		frndint
		fstp dword ptr [g_waveStopLevelL]
	bleed_done:
		pop edx
		pop ecx
		pop eax
		ret
	}
}

// Hand-written timestamp read.
// FUNCTION: TONY2 0x004253ff
__declspec(naked) void WaveReadTimestamp(void)
{
	__asm {
		rdtsc
		ret
	}
}

// Hand-written MMX capability probe.
// FUNCTION: TONY2 0x00425402
__declspec(naked) TonyU8 WaveHasMmx(void)
{
	__asm {
		push edx
		push ecx
		push ebx
		mov al, 1
		cpuid
		test edx, 0x800000
		setne al
		pop ebx
		pop ecx
		pop edx
		ret
	}
}

// Runs the active driver's output mix for one block.
// FUNCTION: TONY2 0x00426c00
void __fastcall WaveDriverMix(TonyS32 p_step, TonyS32 p_frames, TonyS32 p_dest)
{
	switch (g_waveDriverMode) {
	case 0:
		WaveMixBlock(p_step, p_frames, p_dest);
		break;
	case 1:
		DSndMixBlock(p_frames, p_dest);
		break;
	}
}

// Channel-busy query core.
// FUNCTION: TONY2 0x00426c49
TonyU8 __fastcall WaveDriverBusy(TonyS32 p_channel)
{
	TonyU8 result;

	switch (g_waveDriverMode) {
	case 0:
		result = g_waveChannels[p_channel & 0xff].m_active != 0;
		break;
	case 1:
		result = DSndChannelBusy(p_channel);
		break;
	}

	return result;
}

// Loads a channel's envelope descriptor (attack/decay words, Q12 sustain fraction,
// release word).
// FUNCTION: TONY2 0x00426c99
void __fastcall WaveDriverSetAdsr(TonyS32 p_channel, TonyU16* p_desc)
{
	switch (g_waveDriverMode) {
	case 0:
		g_waveChannels[p_channel & 0xff].m_attackTime = p_desc[0];
		g_waveChannels[p_channel & 0xff].m_decayTime = p_desc[1];
		g_waveChannels[p_channel & 0xff].m_sustainLevel = p_desc[2] / 4096.0;
		g_waveChannels[p_channel & 0xff].m_releaseTime = p_desc[3];
		break;
	case 1:
		DSndChannelSetAdsr(p_channel, p_desc);
		break;
	}
}

// Sets the channel's envelope release time word (0x11 = quick stop).
// FUNCTION: TONY2 0x00426d42
void __fastcall WaveDriverSetRelease(TonyS32 p_channel, TonyS32 p_release)
{
	switch (g_waveDriverMode) {
	case 0:
		g_waveChannels[p_channel & 0xff].m_releaseTime = (TonyU16) p_release;
		break;
	case 1:
		DSndChannelSetRelease(p_channel, p_release);
		break;
	}
}

// Arms a channel's fade-in/start command.
// Arms a mixer channel from the sample descriptor for the active driver.
// Arms a wave channel from a play descriptor, switched by the active driver: in the
// software mixer, optionally resets the fade ramp (level 1.0, 0x28 ms) and loads the
// play cursor, loop point and remaining length from the descriptor.
// FUNCTION: TONY2 0x00426d8c
void __fastcall WaveDriverPrepare(TonyU8 p_channel, struct SongDesc* p_desc, TonyU8 p_flag)
{
	switch (g_waveDriverMode) {
	case 0:
		if (p_flag) {
			g_waveChannels[p_channel].m_attackTime = 0;
			g_waveChannels[p_channel].m_decayTime = 0;
			g_waveChannels[p_channel].m_sustainLevel = 1.0f;
			g_waveChannels[p_channel].m_releaseTime = 0x28;
		}

		g_waveChannels[p_channel].m_startPtr = p_desc->m_data + p_desc->m_startPos * 2;
		g_waveChannels[p_channel].m_loopPtr = p_desc->m_data + p_desc->m_loopStart * 2;
		g_waveChannels[p_channel].m_length = p_desc->m_endPos - p_desc->m_startPos;
		g_waveChannels[p_channel].m_loopLength = p_desc->m_loopLength;
		break;
	case 1:
		DSndChannelPrepare(p_channel, p_desc, p_flag);
		break;
	}
}

// Sets a channel's pitch step (12-bit fixed point), clamped to 2.0 in the software
// mixer, switched by the active driver.
// FUNCTION: TONY2 0x00426ea4
void __fastcall WaveDriverSetStep(TonyU8 p_channel, TonyS32 p_step)
{
	switch (g_waveDriverMode) {
	case 0:
		if ((p_step & 0xffff) > 0x2000) {
			g_waveChannels[p_channel].m_step = 0x2000;
		}
		else {
			g_waveChannels[p_channel].m_step = p_step;
		}
		break;
	case 1:
		DSndChannelSetStep(p_channel, p_step);
		break;
	}
}

// FUNCTION: TONY2 0x00426f15
void __fastcall WaveDriverStart(TonyS32 p_channel, TonyU8 p_value)
{
	switch (g_waveDriverMode) {
	case 0:
		WaveChannelStart(&g_waveChannels[p_channel & 0xff], p_value);
		break;
	case 1:
		DSndChannelStart(p_channel);
		break;
	}
}

// Stops a channel, folding its level into the stop accumulators.
// FUNCTION: TONY2 0x00426f64
void __fastcall WaveDriverStop(TonyS32 p_channel)
{
	switch (g_waveDriverMode) {
	case 0:
		WaveChannelStop(&g_waveChannels[p_channel & 0xff]);
		break;
	case 1:
		DSndChannelStop(p_channel);
		break;
	}
}


// Gates a channel's envelope off for the active driver.
// FUNCTION: TONY2 0x00426fac
void __fastcall WaveDriverGateOff(TonyS32 p_channel)
{
	switch (g_waveDriverMode) {
	case 0:
		WaveChannelGateOff(&g_waveChannels[p_channel & 0xff]);
		break;
	case 1:
		DSndChannelGateOff(p_channel);
		break;
	}
}

// Releases a channel's driver resources.
// FUNCTION: TONY2 0x00426ff4
void __fastcall WaveDriverRelease(TonyS32 p_channel)
{
	switch (g_waveDriverMode) {
	case 0:
		WaveChannelRelease(&g_waveChannels[p_channel & 0xff]);
		break;
	case 1:
		DSndEnvRelease(p_channel);
		break;
	}
}

// Applies a four-corner (pan/balance) gain update to a wave channel, switched by the
// active driver: the software mixer keeps a stereo pair, the DirectSound path expands
// level/balance/pan/boost to four word gains with a hand-scheduled FPU block.
// FUNCTION: TONY2 0x0042703c
void __fastcall WaveDriverSetGains(TonyU8 p_channel, TonyS32 p_a, TonyS32 p_b, TonyS32 p_c, TonyS32 p_d)
{
	TonyS32 back;
	TonyFloat tail;
	TonyFloat aux;
	TonyU16 extra;
	TonyU16 other;
	TonyU16 fade;
	TonyFloat fore;
	TonyFloat left;

	switch (g_waveDriverMode) {
	case 0:
		WaveLevelPanToGains(&g_waveChannels[p_channel].m_gainLeft, &g_waveChannels[p_channel].m_gainRight, p_a, p_b);
		break;
	case 1:
		WaveCornerGains(&fore, &aux, &left, &tail, p_a, p_b, p_c, p_d);
		__asm {
			fld dword ptr [aux]
			fld dword ptr [fore]
			fld dword ptr [left]
			fld dword ptr [tail]
			fmul dword ptr [g_waveGainWordScale]
			fxch st(1)
			fmul dword ptr [g_waveGainWordScale]
			fxch st(2)
			fmul dword ptr [g_waveGainWordScale]
			fxch st(3)
			fmul dword ptr [g_waveGainWordScale]
			fxch st(1)
			fistp word ptr [fade]
			fistp word ptr [back]
			fistp word ptr [other]
			fistp word ptr [extra]
		}
		DSndChannelSetVolumes(p_channel, back, extra, other, fade);
		break;
	}
}

// Expands level/balance/pan/boost (16.16 and 10.22 fixed point) into the four corner
// gains through the level and pan curves.
// FUNCTION: TONY2 0x00427118
void __fastcall WaveCornerGains(TonyFloat* p_front, TonyFloat* p_back, TonyFloat* p_left, TonyFloat* p_right,
	TonyS32 p_level, TonyS32 p_balance, TonyS32 p_pan, TonyS32 p_boost)
{
	TonyS32 pos;
	TonyFloat blend;
	TonyFloat vol;
	TonyFloat curve;

	if (p_level > 0x7f0000) {
		p_level = 0x7f0000;
	}

	pos = p_level >> 0x10;
	blend = (p_level & 0xffff) * 1.52587890625e-05f;
	vol = (1.0f - blend) * g_waveLevelCurve2[pos] + blend * g_waveLevelCurve2[pos + 1];

	pos = p_pan >> 0x16;
	blend = (p_pan & 0x3fffff) * 2.384185791015625e-07f;
	curve = (1.0f - blend) * g_wavePanCurve2[pos] + blend * g_wavePanCurve2[pos + 1];
	*p_left = vol * curve * 0.7079f;

	p_pan = 0x7f0000 - p_pan;
	pos = p_pan >> 0x16;
	blend = (p_pan & 0x3fffff) * 2.384185791015625e-07f;
	curve = (1.0f - blend) * g_wavePanCurve2[pos] + blend * g_wavePanCurve2[pos + 1];
	vol = vol * curve;

	if (p_balance == 0x800000) {
		*p_front = vol * 0.7079f;
		*p_back = vol * -0.7079f;
	}
	else {
		pos = p_balance >> 0x16;
		blend = (p_balance & 0x3fffff) * 2.384185791015625e-07f;
		curve = (1.0f - blend) * g_wavePanCurve2[pos] + blend * g_wavePanCurve2[pos + 1];
		*p_back = vol * curve;

		p_balance = 0x7f0000 - p_balance;
		pos = p_balance >> 0x16;
		blend = (p_balance & 0x3fffff) * 2.384185791015625e-07f;
		curve = (1.0f - blend) * g_wavePanCurve2[pos] + blend * g_wavePanCurve2[pos + 1];
		*p_front = vol * curve;
	}

	if (p_boost > 0x7f0000) {
		p_boost = 0x7f0000;
	}

	pos = p_boost >> 0x10;
	blend = (p_boost & 0xffff) * 1.52587890625e-05f;
	*p_right = (vol = (1.0f - blend) * g_waveLevelCurve2[pos] + blend * g_waveLevelCurve2[pos + 1]) * 0.7079f;
}

// Clears a channel's busy state.
// FUNCTION: TONY2 0x00427369
void __fastcall WaveDriverKill(TonyS32 p_channel)
{
	switch (g_waveDriverMode) {
	case 0:
		g_waveChannels[p_channel & 0xff].m_active = 0;
		break;
	case 1:
		DSndChannelKill(p_channel);
		break;
	}
}

// Returns the current play position of a wave channel in samples, switched by the
// active driver.
// FUNCTION: TONY2 0x004273a9
TonyS32 __fastcall WaveDriverPosition(TonyU8 p_channel)
{
	switch (g_waveDriverMode) {
	case 0:
		return (g_waveChannels[p_channel].m_readPtr - g_waveChannels[p_channel].m_startPtr) >> 1;
	case 1:
		return DSndChannelPosition(p_channel);
	}

	return 0;
}


