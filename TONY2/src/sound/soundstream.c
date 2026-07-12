// Spatialized-stream translation unit of the third-party sound library. Split from
// soundlib.c because this TU declares the SFX start entry with a 16-bit sound id
// (the call site passes the word member with no widening) while the mixer TU defines
// it with an int parameter. Compiled with VC6 RTM at /Od like the rest of the band.

#include "decomp.h"
#include "types.h"

// 3D position triple carried by the spatialized stream records.
// SIZE 0xc
typedef struct StreamVec3 {
	TonyFloat m_x; // 0x00
	TonyFloat m_y; // 0x04
	TonyFloat m_z; // 0x08
} StreamVec3;

// Listener pan basis: three rotation rows and a translation triple.
// SIZE 0x30
typedef struct StreamBasis {
	StreamVec3 m_row0;  // 0x00
	StreamVec3 m_row1;  // 0x0c
	StreamVec3 m_row2;  // 0x18
	StreamVec3 m_trans; // 0x24
} StreamBasis;

// Game-side play services (soundlib TU).
TonyS32 __fastcall SamplePlayDefault(TonyU16 p_sound, TonyU8 p_b, TonyU8 p_pan);
TonyS32 __fastcall HandleValidate(TonyS32 p_handle);
TonyS32 __fastcall ChainRelease(void* p_a);
void StreamGroupsReset(void);
void __fastcall StreamGroupAddMember(struct StreamSource* p_stream, TonyFloat p_level);
TonyU8 __fastcall StreamGroupRequest(struct StreamSource* p_stream, TonyFloat p_level, TonyFloat p_b, TonyFloat p_c,
	TonyFloat p_d, TonyFloat p_e);

// Spatialized stream record on the g_streamList list (default record g_streamDefaultSource).
typedef struct StreamSource {
	struct StreamSource* m_next; // 0x00
	struct StreamSource* m_prev; // 0x04
	TonyS32 m_flags;          // 0x08
	StreamVec3 m_position; // 0x0c
	StreamVec3 m_velocity; // 0x18
	TonyFloat m_range;        // 0x24
	TonyFloat m_volume;       // 0x28
	TonyFloat m_minVolume;    // 0x2c
	TonyFloat m_curve;        // 0x30
	TonyS32 m_handle;         // 0x34
	TonyU32 m_group;          // 0x38
	TonyU16 m_sound;          // 0x3c
	TonyU16 m_denyCount;      // 0x3e
	TonyFloat m_fadeLevel;    // 0x40
} StreamSource;

// Listener record on the g_streamListeners list: shares the stream record's head layout
// and carries the pan basis and doppler/gain parameters behind it.
typedef struct StreamListener {
	struct StreamListener* m_next;     // 0x00
	undefined m_prev[8 - 4];          // 0x04
	TonyS32 m_flags;                  // 0x08
	StreamVec3 m_position;         // 0x0c
	StreamVec3 m_velocity;         // 0x18
	undefined m_reserved[0x48 - 0x24]; // 0x24
	StreamBasis m_panBasis;       // 0x48
	TonyFloat m_frontRange;           // 0x78
	TonyFloat m_backRange;            // 0x7c
	TonyFloat m_soundSpeed;           // 0x80
	TonyFloat m_gain;                 // 0x84
} StreamListener;

void MixerLock(void);
void MixerUnlock(void);
// This TU's original prototype takes the sound id as a word.
TonyS32 __fastcall SamplePlay(TonyU16 p_sound, TonyU8 p_b, TonyU8 p_pan);
TonyS32 __fastcall ChainRelease(void* p_a);
TonyFloat __cdecl StreamSqrt(TonyFloat p_value);
void __fastcall StreamTransformPoint(StreamBasis* p_basis, StreamVec3* p_pos, StreamVec3* p_out);
TonyFloat __fastcall StreamVecNormalize(StreamVec3* p_vec);
void __fastcall ChainSetLevel(TonyS32 p_handle, TonyU8 p_value);
void __fastcall ChainSetPan(TonyS32 p_handle, TonyU8 p_value);
void __fastcall ChainSetRate(TonyS32 p_handle, TonyU16 p_value);
void __fastcall ChainSetVolume(TonyS32 p_handle, TonyU8 p_value);
TonyU8 __fastcall StreamClampLevel(TonyU8 p_value);
TonyU16 __fastcall StreamClampRate(TonyU32 p_value);

TonyU8 __fastcall StreamRelease(StreamSource* p_node);
void __fastcall StreamUnlink(StreamSource* p_node);
TonyS32 __fastcall StreamOpen(
	StreamSource* p_node,
	StreamVec3* p_pos,
	StreamVec3* p_vec,
	TonyFloat p_range,
	TonyFloat p_curve,
	TonyS32 p_flags,
	TonyU16 p_rate,
	TonyU32 p_handle,
	TonyU8 p_volume,
	TonyU8 p_volume2
);
void __fastcall StreamSpatialize(
	StreamSource* p_node,
	TonyFloat* p_out0,
	TonyFloat* p_out4,
	TonyFloat* p_out1,
	TonyFloat* p_out2,
	TonyFloat* p_out3
);
void __fastcall StreamApplyParams(
	StreamSource* p_node,
	TonyFloat p_out0,
	TonyFloat p_out1,
	TonyFloat p_out2,
	TonyFloat p_out3,
	TonyFloat p_out4
);

// Head of the mixer's pending allocation list, drained by StreamReleaseAll.
// GLOBAL: TONY2 0x004efd24
StreamSource* g_streamList;

// Head of the spatializer's listener list walked by StreamSpatialize.
// GLOBAL: TONY2 0x004efd20
StreamListener* g_streamListeners;

// Default stream record used when the caller passes no node.
// GLOBAL: TONY2 0x004e6790
StreamSource g_streamDefaultSource;

// Count of active stream mix groups and the two service flags.
// GLOBAL: TONY2 0x004e6788
TonyU8 g_streamGroupCount;

// GLOBAL: TONY2 0x004e6b58
TonyU8 g_streamMemberCount;

// GLOBAL: TONY2 0x004e6b59
TonyU8 g_streamRequestCount;

// Stream mix group: group key, pending request list and the level-sorted live
// member list.
// SIZE 0xc
typedef struct StreamGroup {
	TonyU32 m_key;                    // 0x00
	struct StreamRequest* m_requests; // 0x04
	struct StreamMember* m_members;   // 0x08
} StreamGroup;

// Stream mix-group member node: next, level and the stream.
// SIZE 0xc
typedef struct StreamMember {
	struct StreamMember* m_next; // 0x00
	TonyFloat m_level;           // 0x04
	struct StreamSource* m_stream;  // 0x08
} StreamMember;

// Positional stream request: next, level, the spatialized parameter set and the
// stream.
// SIZE 0x1c
typedef struct StreamRequest {
	struct StreamRequest* m_next; // 0x00
	TonyFloat m_level;            // 0x04
	TonyFloat m_panX;             // 0x08
	TonyFloat m_panY;             // 0x0c
	TonyFloat m_balance;          // 0x10
	TonyFloat m_doppler;          // 0x14
	struct StreamSource* m_stream;   // 0x18
} StreamRequest;

// The stream mix groups and the member node pool.
// GLOBAL: TONY2 0x004e6488
StreamGroup g_streamGroups[0x20];

// GLOBAL: TONY2 0x004e6608
StreamMember g_streamMembers[0x20];

// The positional request pool.
// GLOBAL: TONY2 0x004e67d8
StreamRequest g_streamRequests[0x20];

// Mixer-active flag (annotated in soundlib.c).
extern TonyU8 g_mixerActive;

// Locked update of a stream record's position pair and volume; clamps the secondary
// volume to the new primary.
// FUNCTION: TONY2 0x00425420
TonyU8 __fastcall StreamSetPosition(StreamSource* p_node, StreamVec3* p_pos, StreamVec3* p_vec, TonyS32 p_volume)
{
	if (g_mixerActive) {
		MixerLock();
		p_node->m_position = *p_pos;
		p_node->m_velocity = *p_vec;
		p_node->m_volume = (TonyFloat) (p_volume & 0xff) / 127.0f;

		if (p_node->m_minVolume > p_node->m_volume) {
			p_node->m_minVolume = p_node->m_volume;
		}

		MixerUnlock();
		return 1;
	}

	return 0;
}

// Reports whether the stream record is queued (its open flag armed).
// FUNCTION: TONY2 0x004254b8
TonyU8 __fastcall StreamIsQueued(StreamSource* p_node)
{
	if (g_mixerActive) {
		return (p_node->m_flags & 0x10000) != 0;
	}

	return 0;
}

// Spatialized stream start: forwards to the open core with the sound word doubled up
// as the mix-group key (high bit marking the auto-derived key).
// FUNCTION: TONY2 0x004254e3
TonyS32 __fastcall StreamPlay(
	StreamSource* p_node,
	StreamVec3* p_pos,
	StreamVec3* p_vec,
	TonyFloat p_range,
	TonyFloat p_curve,
	TonyS32 p_flags,
	TonyU16 p_rate,
	TonyU8 p_volume,
	TonyU8 p_volume2
)
{
	if (g_mixerActive) {
		return StreamOpen(
			p_node,
			p_pos,
			p_vec,
			p_range,
			p_curve,
			p_flags,
			p_rate,
			p_rate | 0x80000000,
			p_volume,
			p_volume2
		);
	}

	return -1;
}

// Stream-open core: fills the (default or caller) record; a caller record is parked
// on the pending list for later start while the default record is spatialized and its
// SFX voice started immediately.
// FUNCTION: TONY2 0x0042553b
TonyS32 __fastcall StreamOpen(
	StreamSource* p_node,
	StreamVec3* p_pos,
	StreamVec3* p_vec,
	TonyFloat p_range,
	TonyFloat p_curve,
	TonyS32 p_flags,
	TonyU16 p_rate,
	TonyU32 p_d,
	TonyU8 p_volume,
	TonyU8 p_volume2
)
{
	TonyFloat s0;
	TonyFloat s1;
	TonyFloat s2;
	TonyFloat s3;
	StreamSource* cur;
	TonyFloat sfx;

	MixerLock();
	cur = p_node == NULL ? &g_streamDefaultSource : p_node;
	cur->m_flags = p_flags;
	cur->m_position = *p_pos;
	cur->m_velocity = *p_vec;
	cur->m_range = p_range;
	cur->m_sound = p_rate;
	cur->m_volume = (TonyFloat) p_volume / 127.0f;
	cur->m_minVolume = (TonyFloat) p_volume2 / 127.0f;
	cur->m_curve = p_curve;
	cur->m_group = p_d;

	if (p_node == NULL) {
		StreamSpatialize(cur, &s0, &sfx, &s1, &s2, &s3);

		if (s0 == 0.0f) {
			MixerUnlock();
			return -1;
		}

		cur->m_handle = SamplePlay(cur->m_sound, 0x7f, 0x40);

		if (cur->m_handle == -1) {
			MixerUnlock();
			return -1;
		}

		StreamApplyParams(cur, s0, s1, s2, s3, sfx);
		MixerUnlock();
		return cur->m_handle;
	}

	cur->m_next = g_streamList;

	if (g_streamList != NULL) {
		g_streamList->m_prev = cur;
	}

	cur->m_prev = NULL;
	g_streamList = cur;
	cur->m_handle = -1;
	cur->m_denyCount = 0;
	cur->m_flags |= 0x30000;
	MixerUnlock();
	return -1;
}

// Spatializes a stream record against every listener: accumulates the quadratic
// distance attenuation into the level, derives the doppler factor from the relative
// speed, transforms the position into the listener pan basis and normalizes it for
// the direction outputs.
// FUNCTION: TONY2 0x004256f8
void __fastcall StreamSpatialize(
	StreamSource* p_node,
	TonyFloat* p_out0,
	TonyFloat* p_out4,
	TonyFloat* p_out1,
	TonyFloat* p_out2,
	TonyFloat* p_out3
)
{
	StreamVec3 v10;  // pan-space position
	TonyFloat v11;      // simulation step
	TonyFloat v12;      // distance
	TonyFloat v13;      // attenuation
	TonyFloat v14;      // dy
	TonyFloat v15;      // distance squared
	TonyFloat v16;      // relative velocity y
	TonyFloat v17;      // predicted distance
	TonyFloat v18;      // relative speed
	StreamListener* v19; // listener cursor
	TonyFloat v20;      // dz
	TonyFloat v21;      // dx
	TonyFloat v22;      // relative velocity z
	TonyFloat v23;      // relative velocity x

	v11 = 1.0f / 60.0f;
	*p_out0 = 0.0f;
	*p_out4 = 1.0f;

	for (v19 = g_streamListeners; v19 != NULL; v19 = v19->m_next) {
		v21 = p_node->m_position.m_x - v19->m_position.m_x;
		v14 = p_node->m_position.m_y - v19->m_position.m_y;
		v20 = p_node->m_position.m_z - v19->m_position.m_z;
		v15 = v21 * v21 + v14 * v14 + v20 * v20;
		v12 = StreamSqrt(v15);

		if (p_node->m_range >= v12) {
			v13 = v12 / p_node->m_range;
			*p_out0 += ((p_node->m_volume - p_node->m_minVolume) *
							(1.0f - ((1.0f - p_node->m_curve) * v13 + v13 * p_node->m_curve * v13)) +
						p_node->m_minVolume) *
					   v19->m_gain;

			if (!(p_node->m_flags & 0x80000)) {
				if ((p_node->m_flags & 8) || (v19->m_flags & 1)) {
					v23 = v19->m_velocity.m_x - p_node->m_velocity.m_x;
					v16 = v19->m_velocity.m_y - p_node->m_velocity.m_y;
					v22 = v19->m_velocity.m_z - p_node->m_velocity.m_z;

					if ((v18 = StreamSqrt(v23 * v23 + v16 * v16 + v22 * v22)) > 0.0f) {
						v21 = v11 * p_node->m_velocity.m_x + p_node->m_position.m_x -
							  (v11 * v19->m_velocity.m_x + v19->m_position.m_x);
						v14 = v11 * p_node->m_velocity.m_y + p_node->m_position.m_y -
							  (v11 * v19->m_velocity.m_y + v19->m_position.m_y);
						v20 = v11 * p_node->m_velocity.m_z + p_node->m_position.m_z -
							  (v11 * v19->m_velocity.m_z + v19->m_position.m_z);

						if ((v17 = StreamSqrt(v21 * v21 + v14 * v14 + v20 * v20)) < v12) {
							*p_out4 = v19->m_soundSpeed / (v19->m_soundSpeed - v18);
						}
						else {
							*p_out4 = v19->m_soundSpeed / (v18 + v19->m_soundSpeed);
						}
					}
				}

				StreamTransformPoint(&v19->m_panBasis, &p_node->m_position, &v10);

				if (v10.m_z >= 0.0f) {
					*p_out3 = v19->m_frontRange > v10.m_z ? v10.m_z / v19->m_frontRange : 1.0;
				}
				else {
					*p_out3 = -v19->m_backRange < v10.m_z ? v10.m_z / v19->m_backRange : -1.0;
				}

				if (v10.m_x != 0.0 || v10.m_y != 0.0 || v10.m_z != 0.0f) {
					StreamVecNormalize(&v10);
				}

				*p_out1 = v10.m_x;
				*p_out2 = v10.m_y;
			}
		}
	}
}

// Applies the spatialized parameter set to the stream's playing voice: level
// (optionally shaped by the record's own gain), the pan pair from the direction and
// the doppler-scaled rate.
// FUNCTION: TONY2 0x00425a42
void __fastcall StreamApplyParams(
	StreamSource* p_node,
	TonyFloat p_out0,
	TonyFloat p_out1,
	TonyFloat p_out2,
	TonyFloat p_out3,
	TonyFloat p_out4
)
{
	TonyS32 handle;

	handle = p_node->m_handle;

	if (p_node->m_flags & 0x100000) {
		ChainSetVolume(handle, StreamClampLevel((TonyS32) (p_out0 * p_node->m_fadeLevel * 127.0f)));
	}
	else {
		ChainSetVolume(handle, StreamClampLevel((TonyS32) (p_out0 * 127.0f)));
	}

	ChainSetLevel(handle, StreamClampLevel((TonyS32) ((1.0f + p_out1) * 64.0f)));
	ChainSetPan(handle, StreamClampLevel((TonyS32) ((1.0f - p_out3) * 64.0f)));
	ChainSetRate(handle, StreamClampRate((TonyS32) (p_out4 * 8192.0f)));
}

// Saturates a level to the 0x7f driver maximum.
// FUNCTION: TONY2 0x00425b18
TonyU8 __fastcall StreamClampLevel(TonyU8 p_value)
{
	if (p_value > 0x7f) {
		return 0x7f;
	}

	return p_value;
}

// Saturates a rate to the 0x3fff driver maximum.
// FUNCTION: TONY2 0x00425b37
TonyU16 __fastcall StreamClampRate(TonyU32 p_value)
{
	if (p_value > 0x3fff) {
		return 0x3fff;
	}

	return (TonyU16) p_value;
}

// Public spatialized stream start: forwards to the open core when the mixer is up.
// FUNCTION: TONY2 0x00425b55
TonyS32 __fastcall StreamPlayGrouped(
	StreamSource* p_node,
	StreamVec3* p_pos,
	StreamVec3* p_vec,
	TonyFloat p_range,
	TonyFloat p_curve,
	TonyS32 p_flags,
	TonyU16 p_rate,
	TonyU16 p_rate2,
	TonyU8 p_volume,
	TonyU8 p_volume2
)
{
	if (g_mixerActive) {
		return StreamOpen(
			p_node,
			p_pos,
			p_vec,
			p_range,
			p_curve,
			p_flags,
			p_rate,
			p_rate2,
			p_volume,
			p_volume2
		);
	}

	return -1;
}

// Locked node release; returns TRUE when the mixer was up.
// FUNCTION: TONY2 0x00425ba8
TonyU8 __fastcall StreamRelease(StreamSource* p_node)
{
	if (g_mixerActive) {
		MixerLock();
		StreamUnlink(p_node);
		MixerUnlock();
		return 1;
	}

	return 0;
}

// Unlinks the node from the pending list, strips its high flag word and releases the
// voice handle when one is attached.
// FUNCTION: TONY2 0x00425bd6
void __fastcall StreamUnlink(StreamSource* p_node)
{
	if (p_node->m_next != NULL) {
		p_node->m_next->m_prev = p_node->m_prev;
	}

	if (p_node->m_prev != NULL) {
		p_node->m_prev->m_next = p_node->m_next;
	}
	else {
		g_streamList = p_node->m_next;
	}

	p_node->m_flags &= 0xffff;

	if (p_node->m_handle != -1) {
		ChainRelease((void*) p_node->m_handle);
	}
}

// Returns the queued node's voice handle, or -1.
// FUNCTION: TONY2 0x00425c40
TonyS32 __fastcall StreamGetHandle(StreamSource* p_node)
{
	TonyS32 result;

	result = -1;

	if (g_mixerActive) {
		MixerLock();

		if (p_node->m_flags & 0x10000) {
			result = p_node->m_handle;
		}

		MixerUnlock();
	}

	return result;
}

// Drains the pending list, releasing every node.
// FUNCTION: TONY2 0x00425c85
void StreamReleaseAll(void)
{
	StreamSource* cursor;
	StreamSource* next;

	for (cursor = g_streamList; cursor != NULL; cursor = next) {
		next = cursor->m_next;
		StreamRelease(cursor);
	}
}

void StreamGroupArbitrate(void);

// Pumps the active streams: computes levels, starts pending streams (3D or flat),
// validates running handles with requeue-on-loss, aggregates mix groups, kills or
// updates each stream and advances fade-ins.
// FUNCTION: TONY2 0x00425fa1
void StreamPump(void)
{
	StreamSource* fade;
	StreamSource* gain;
	TonyFloat crest;
	TonyFloat foam;
	TonyFloat fall;
	TonyFloat ebb;
	TonyFloat seep;

	StreamGroupsReset();

	for (fade = g_streamList; fade != NULL; fade = gain) {
		gain = fade->m_next;

		if (fade->m_flags & 0x40000) {
			StreamUnlink(fade);
			continue;
		}

		if (fade->m_flags & 0x20001) {
			StreamSpatialize(fade, &crest, &foam, &fall, &ebb, &seep);
		}

		if (!(fade->m_flags & 0x80000)) {
			if (fade->m_flags & 0x20000) {
				if (crest == 0.0f && fade->m_flags & 4) {
					fade->m_flags = fade->m_flags | 0x80000;
					fade->m_flags = fade->m_flags & 0xfffdffff;
				}
				else if (fade->m_flags & 1) {
					if (StreamGroupRequest(fade, crest, fall, ebb, seep, foam)) {
						continue;
					}
				}
				else {
					fade->m_handle = SamplePlayDefault(fade->m_sound, 0x7f, 0x40);

					if (fade->m_handle == -1) {
						if (fade->m_flags & 2) {
							continue;
						}

						fade->m_flags = fade->m_flags | 0x40000;
						fade->m_flags = fade->m_flags & 0xfffdffff;
					}
				}
			}
			else {
				fade->m_handle = HandleValidate(fade->m_handle);

				if (fade->m_handle == -1) {
					if (fade->m_flags & 2) {
						fade->m_flags = fade->m_flags | 0x20000;
					}
					else {
						fade->m_flags = fade->m_flags | 0x40000;
					}
				}
			}

			if (fade->m_handle != -1) {
				if (fade->m_flags & 1) {
					StreamGroupAddMember(fade, crest);
				}

				if (crest == 0.0f && fade->m_flags & 4) {
					fade->m_flags = fade->m_flags | 0x80000;
					ChainRelease((void*) fade->m_handle);
					fade->m_handle = -1;
				}
				else {
					StreamApplyParams(fade, crest, fall, ebb, seep, foam);
				}
			}

			if (fade->m_flags & 0x100000) {
				fade->m_fadeLevel = fade->m_fadeLevel + 0.3f;

				if (!(fade->m_fadeLevel < 1.0f)) {
					fade->m_flags = fade->m_flags & 0xffefffff;
				}
			}
		}
		else {
			if (crest != 0.0f) {
				fade->m_flags = fade->m_flags & 0xfff7ffff;
				fade->m_flags = fade->m_flags | 0x20000;
			}
		}
	}

	StreamGroupArbitrate();
}

// Resets the stream mix-group table and the service flags.
// FUNCTION: TONY2 0x00426266
void StreamGroupsReset(void)
{
	g_streamGroupCount = 0;
	g_streamRequestCount = 0;
	g_streamMemberCount = 0;
}

// Adds a live grouped stream into its mix group, creating the group on first use
// and keeping the member list sorted by level.
// FUNCTION: TONY2 0x00426280
void __fastcall StreamGroupAddMember(StreamSource* p_stream, TonyFloat p_level)
{
	StreamMember* spot;
	TonyS32 i;
	StreamMember* prev;

	for (i = 0; i < g_streamGroupCount; i++) {
		if (p_stream->m_group == g_streamGroups[i].m_key) {
			break;
		}
	}

	if (i == g_streamGroupCount) {
		g_streamGroups[i].m_requests = 0;
		g_streamGroups[i].m_members = NULL;
		g_streamGroups[i].m_key = p_stream->m_group;
		g_streamGroupCount += 1;
	}

	prev = NULL;

	for (spot = g_streamGroups[i].m_members; spot != NULL; spot = spot->m_next) {
		if (spot->m_level > p_level) {
			break;
		}

		prev = spot;
	}

	if (prev == NULL) {
		g_streamGroups[i].m_members = &g_streamMembers[g_streamMemberCount];
	}
	else {
		prev->m_next = &g_streamMembers[g_streamMemberCount];
	}

	g_streamMembers[g_streamMemberCount].m_next = spot;
	g_streamMembers[g_streamMemberCount].m_stream = p_stream;
	g_streamMembers[g_streamMemberCount].m_level = p_level;
	g_streamMemberCount += 1;
}

// Queues a positional stream start: finds or creates its mix group and inserts a
// request node sorted by level (descending). Returns TRUE when queued.
// FUNCTION: TONY2 0x004263d7
TonyU8 __fastcall StreamGroupRequest(StreamSource* p_stream, TonyFloat p_level, TonyFloat p_b, TonyFloat p_c, TonyFloat p_d,
	TonyFloat p_e)
{
	StreamRequest* spot;
	TonyS32 i;

	for (i = 0; i < g_streamGroupCount; i++) {
		if (p_stream->m_group == g_streamGroups[i].m_key) {
			break;
		}
	}

	if (i == g_streamGroupCount) {
		if (g_streamGroupCount == 0x20) {
			return 0;
		}

		g_streamGroups[i].m_requests = NULL;
		g_streamGroups[i].m_members = NULL;
		g_streamGroups[i].m_key = p_stream->m_group;
		g_streamGroupCount += 1;
	}

	if (g_streamRequestCount == 0x20) {
		return 0;
	}

	spot = g_streamGroups[i].m_requests;

	if (spot != NULL) {
		for (; spot->m_next != NULL; spot = spot->m_next) {
			if (spot->m_level < p_level) {
				break;
			}
		}

		g_streamRequests[g_streamRequestCount].m_next = spot->m_next;
		spot->m_next = &g_streamRequests[g_streamRequestCount];
	}
	else {
		g_streamRequests[g_streamRequestCount].m_next = g_streamGroups[i].m_requests;
		g_streamGroups[i].m_requests = &g_streamRequests[g_streamRequestCount];
	}

	g_streamRequests[g_streamRequestCount].m_stream = p_stream;
	g_streamRequests[g_streamRequestCount].m_doppler = p_e;
	g_streamRequests[g_streamRequestCount].m_panX = p_b;
	g_streamRequests[g_streamRequestCount].m_panY = p_c;
	g_streamRequests[g_streamRequestCount].m_balance = p_d;
	g_streamRequests[g_streamRequestCount].m_level = p_level;
	g_streamRequestCount += 1;
	return 1;
}

// Arbitrates the positional requests of every mix group: a request only starts when
// it outweighs the quietest live member by a hysteresis margin (with a 0x14-tick
// denial counter for borderline cases), evicting that member on success.
// FUNCTION: TONY2 0x004265b6
void StreamGroupArbitrate(void)
{
	StreamSource* strm;
	StreamRequest* cur;
	TonyFloat gap;
	TonyS32 i;

	for (i = 0; i < g_streamGroupCount; i++) {
		for (cur = g_streamGroups[i].m_requests; cur != NULL; cur = cur->m_next) {
			if (g_streamGroups[i].m_members != NULL) {
				gap = cur->m_level - g_streamGroups[i].m_members->m_level;

				if (gap <= 0.08f) {
					continue;
				}

				if (gap <= 0.15f) {
					cur->m_stream->m_denyCount += 1;

					if (cur->m_stream->m_denyCount < 0x14) {
						continue;
					}
				}
				else {
					cur->m_stream->m_denyCount = 0;
				}
			}

			strm = cur->m_stream;
			strm->m_handle = SamplePlayDefault(strm->m_sound, 0x7f, 0x40);

			if (strm->m_handle == -1) {
				if (!(strm->m_flags & 2)) {
					strm->m_flags = strm->m_flags | 0x40000;
					strm->m_flags = strm->m_flags & 0xfffdffff;
				}
			}
			else {
				strm->m_flags = strm->m_flags | 0x100000;
				strm->m_fadeLevel = 0;
				StreamApplyParams(strm, cur->m_level, cur->m_panX, cur->m_panY, cur->m_balance, cur->m_doppler);
				strm->m_flags = strm->m_flags & 0xfffdffff;

				if (g_streamGroups[i].m_members != NULL) {
					g_streamGroups[i].m_members = g_streamGroups[i].m_members->m_next;
				}
			}
		}
	}
}

// Transforms a point into a listener pan basis (rotate plus translate).
// FUNCTION: TONY2 0x00427500
void __fastcall StreamTransformPoint(StreamBasis* p_basis, StreamVec3* p_pos, StreamVec3* p_out)
{
	p_out->m_x = p_basis->m_row0.m_x * p_pos->m_x + p_basis->m_row0.m_y * p_pos->m_y +
				 p_basis->m_row0.m_z * p_pos->m_z + p_basis->m_trans.m_x;
	p_out->m_y = p_basis->m_row1.m_x * p_pos->m_x + p_basis->m_row1.m_y * p_pos->m_y +
				 p_basis->m_row1.m_z * p_pos->m_z + p_basis->m_trans.m_y;
	p_out->m_z = p_basis->m_row2.m_x * p_pos->m_x + p_basis->m_row2.m_y * p_pos->m_y +
				 p_basis->m_row2.m_z * p_pos->m_z + p_basis->m_trans.m_z;
}

// Normalizes a 3-float vector in place; returns its previous length.
// FUNCTION: TONY2 0x004275a9
TonyFloat __fastcall StreamVecNormalize(StreamVec3* p_vec)
{
	TonyFloat len;

	len = StreamSqrt(p_vec->m_x * p_vec->m_x + p_vec->m_y * p_vec->m_y + p_vec->m_z * p_vec->m_z);
	p_vec->m_x /= len;
	p_vec->m_y /= len;
	p_vec->m_z /= len;
	return len;
}

// Cross product.
// FUNCTION: TONY2 0x00427619
void __fastcall StreamVecCross(StreamVec3* p_out, StreamVec3* p_a, StreamVec3* p_b)
{
	p_out->m_x = p_a->m_y * p_b->m_z - p_a->m_z * p_b->m_y;
	p_out->m_y = p_a->m_z * p_b->m_x - p_a->m_x * p_b->m_z;
	p_out->m_z = p_a->m_x * p_b->m_y - p_a->m_y * p_b->m_x;
}

// Inverts a rigid pan basis: adjugate over determinant for the rotation, then the
// negated translation through the new rows.
// FUNCTION: TONY2 0x00427686
void __fastcall StreamBasisInvert(StreamBasis* p_out, StreamBasis* p_in)
{
	TonyFloat c0;
	TonyFloat c1;
	TonyFloat c2;
	TonyFloat inv;

	c0 = p_in->m_row1.m_y * p_in->m_row2.m_z - p_in->m_row2.m_y * p_in->m_row1.m_z;
	c1 = -(p_in->m_row1.m_x * p_in->m_row2.m_z - p_in->m_row2.m_x * p_in->m_row1.m_z);
	c2 = p_in->m_row1.m_x * p_in->m_row2.m_y - p_in->m_row2.m_x * p_in->m_row1.m_y;
	p_out->m_row0.m_x =
		(inv = 1.0f / (c0 * p_in->m_row0.m_x + c1 * p_in->m_row0.m_y + c2 * p_in->m_row0.m_z)) * c0;
	p_out->m_row1.m_x = inv * c1;
	p_out->m_row2.m_x = inv * c2;
	p_out->m_row0.m_y = -inv * (p_in->m_row0.m_y * p_in->m_row2.m_z - p_in->m_row2.m_y * p_in->m_row0.m_z);
	p_out->m_row1.m_y = (p_in->m_row0.m_x * p_in->m_row2.m_z - p_in->m_row2.m_x * p_in->m_row0.m_z) * inv;
	p_out->m_row2.m_y = -inv * (p_in->m_row0.m_x * p_in->m_row2.m_y - p_in->m_row2.m_x * p_in->m_row0.m_y);
	p_out->m_row0.m_z = (p_in->m_row0.m_y * p_in->m_row1.m_z - p_in->m_row1.m_y * p_in->m_row0.m_z) * inv;
	p_out->m_row1.m_z = -inv * (p_in->m_row0.m_x * p_in->m_row1.m_z - p_in->m_row1.m_x * p_in->m_row0.m_z);
	p_out->m_row2.m_z = (p_in->m_row0.m_x * p_in->m_row1.m_y - p_in->m_row1.m_x * p_in->m_row0.m_y) * inv;
	p_out->m_trans.m_x = -p_in->m_trans.m_x * p_out->m_row0.m_x - p_in->m_trans.m_y * p_out->m_row0.m_y -
						 p_in->m_trans.m_z * p_out->m_row0.m_z;
	p_out->m_trans.m_y = -p_in->m_trans.m_x * p_out->m_row1.m_x - p_in->m_trans.m_y * p_out->m_row1.m_y -
						 p_in->m_trans.m_z * p_out->m_row1.m_z;
	p_out->m_trans.m_z = -p_in->m_trans.m_x * p_out->m_row2.m_x - p_in->m_trans.m_y * p_out->m_row2.m_y -
						 p_in->m_trans.m_z * p_out->m_row2.m_z;
}

// Square root helper used by the spatializer; the original computes it with inline
// assembly instead of the CRT sqrt.
// FUNCTION: TONY2 0x0042789f
TonyFloat __cdecl StreamSqrt(TonyFloat p_value)
{
	TonyFloat result;

	__asm {
		fld p_value
		fsqrt
		fstp result
	}

	return result;
}

// Byte fill used by the driver band.
// FUNCTION: TONY2 0x004278c0
void __fastcall StreamFillBytes(char* p_dest, TonyU8 p_value, TonyU32 p_count)
{
	char* cursor;

	for (cursor = p_dest; p_count > 0; p_count--) {
		*cursor = p_value;
		cursor++;
	}
}
