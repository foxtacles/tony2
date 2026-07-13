// clang-format off
// textlabel.h pulls in afx.h, which must precede any windows.h inclusion.
#include "textlabel.h"
// clang-format on

#include "gameobject.h"

#include "camera.h"
#include "engine.h"
#include "gamemanager.h"
#include "hitbox.h"
#include "inputmanager.h"
#include "objectmanager.h"
#include "soundmanager.h"
#include "videomanager.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void __fastcall MergeUniqueBoxes(TonyS32* p_list, TonyS32* p_count, TonyS32* p_values, TonyS32 p_valueCount);
void __fastcall BuildWorldHitBox(GameObject* p_object, VideoManager::FrameHitBox* p_box, HitBox* p_out);
void __fastcall BuildPrevHitBox(GameObject* p_object, VideoManager::FrameHitBox* p_box, HitBox* p_out);

// GLOBAL: TONY2 0x0044c4a0
static const TonyFloat g_zeroF = 0.0f;

// GLOBAL: TONY2 0x0045c538
TonyS32 g_bouncerCount;

// GLOBAL: TONY2 0x0045c520
TonyFloat g_moveDxLeft;

// GLOBAL: TONY2 0x0045c524
TonyFloat g_moveDxRight;

// GLOBAL: TONY2 0x0045c528
TonyFloat g_moveDyTop;

// GLOBAL: TONY2 0x0045c52c
TonyFloat g_moveDyBottom;

// GLOBAL: TONY2 0x0045c530
HitBox* g_prevHitBox;

// GLOBAL: TONY2 0x0045c534
HitBox* g_obstacleBox;

// FUNCTION: TONY2 0x00401000
void __fastcall SpriteInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = SpriteTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = SpriteDestroy;
	p_object->ResetAnimation();
}

// FUNCTION: TONY2 0x00401030
void __fastcall ResolveTemplateFrameSet(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->ResolveFrameSet();
}

// FUNCTION: TONY2 0x00401050
void GameObject::ResolveFrameSet()
{
	m_ext->m_idleSetR = g_videoManager->GetFrameSet(m_ext->m_idleSetR, 0);

	if (m_head->m_flags & 0x200) {
		g_videoManager->AddRefFrameSet(m_ext->m_idleSetR);
	}
}

// FUNCTION: TONY2 0x00401090
TonyS32 __fastcall SpriteTick(GameObject* p_object)
{
	p_object->AdvanceAnimation();
	SceneryTick(p_object);
	return 0;
}

// FUNCTION: TONY2 0x004010b0
void __fastcall SpriteDraw(GameObject* p_object)
{
	TonyFloat xOffset;
	TonyFloat yOffset;
	TonyS32 i;

	GetDrawPosition(p_object, &xOffset, &yOffset);

	if (p_object->m_state->m_frameSet == -1) {
		return;
	}

	for (i = 0; i < g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame].m_partCount;
		 i++) {
		QueueObjectSprite(
			p_object,
			g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame].m_parts[i].m_sprite,
			(TonyFloat) g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame]
					.m_parts[i]
					.m_dx +
				xOffset,
			(TonyFloat) g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame]
					.m_parts[i]
					.m_dy +
				yOffset,
			p_object->m_head->m_layer,
			0
		);
	}
}

// FUNCTION: TONY2 0x00401180
void GameObject::ResetAnimation()
{
	InitMotion(this);
	m_state->m_nextFrameSet = -1;
	m_state->m_frameLatch = 1;
	SetFrameSet(this, m_ext->m_idleSetR);
	m_state->m_prevFrameSet = m_state->m_frameSet;
	m_state->m_prevFrame = m_state->m_frame;
}

// FUNCTION: TONY2 0x004011d0
void __fastcall SetFrameSet(GameObject* p_object, TonyS32 p_frameSet)
{
	if (p_frameSet == -1) {
		p_object->m_state->m_frameSet = p_frameSet;
		p_object->m_state->m_frame = 0;
		p_object->m_state->m_frameTime = 1000.0f;
		p_object->m_state->m_animSpeed = 0.0f;
		return;
	}

	if (p_object->m_state->m_frameLatch == 1) {
		p_object->m_state->m_prevFrameSet = p_object->m_state->m_frameSet;
		p_object->m_state->m_prevFrame = p_object->m_state->m_frame;
		p_object->m_state->m_frameLatch = 0;
	}

	p_object->m_state->m_frameSet = p_frameSet;
	p_object->m_state->m_frame = 0;
	p_object->m_state->m_frameTime = (TonyFloat) g_videoManager->m_frameSets[p_frameSet]->m_duration;
	p_object->m_state->m_animSpeed = 1.0f;
	RefreshHitBoxes(p_object);
}

// FUNCTION: TONY2 0x00401280
void __fastcall QueueFrameSet(GameObject* p_object, TonyS32 p_frameSet)
{
	p_object->m_state->m_nextFrameSet = p_frameSet;
}

// FUNCTION: TONY2 0x00401290
void GameObject::AdvanceAnimation()
{
	m_state->m_frameTime -= m_state->m_animSpeed;

	if (m_state->m_frameLatch == 1) {
		m_state->m_prevFrameSet = m_state->m_frameSet;
		m_state->m_prevFrame = m_state->m_frame;
	}

	if (m_state->m_frameTime <= g_zeroF) {
		m_state->m_frame++;

		if (g_videoManager->m_frameSets[m_state->m_frameSet][m_state->m_frame].m_duration == -1) {
			if (m_state->m_nextFrameSet != -1) {
				m_state->m_frameSet = m_state->m_nextFrameSet;
				m_state->m_nextFrameSet = -1;
			}

			m_state->m_frame = 0;
		}

		m_state->m_frameTime =
			(TonyFloat) g_videoManager->m_frameSets[m_state->m_frameSet][m_state->m_frame].m_duration;
		RefreshHitBoxes(this);
	}

	m_state->m_frameLatch = 1;
}

// Fully implemented, kept as STUB because it compares at 94.68%: the only real diff is the
// materialization order of the CSE'd sprite-table offset (the original fuses the first entry
// load into full [mgr + sprite*4 + 0x828] addressing and spins the lea off afterwards; cl
// 11.00.7022 hoists the lea before the first use), plus the resulting jle encodings. Source
// forms tested: duplicated full expressions, TonyS32/TonyS16 sprite local, entry pointer
// local with re-fetch. Re-annotate as FUNCTION when a matching form is found.
// FUNCTION: TONY2 0x00401390
void __fastcall CollectHitBoxes(GameObject* p_object, HitResult* p_out, TonyS32* p_count, TonyS32 p_mask)
{
	CollectFrameHitBoxes(p_object, p_object->m_state->m_frameSet, p_object->m_state->m_frame, p_out, p_count, p_mask);
}

// FUNCTION: TONY2 0x004013c0
void __fastcall CollectFrameHitBoxes(
	GameObject* p_object,
	TonyS32 p_frameSet,
	TonyS32 p_frame,
	HitResult* p_out,
	TonyS32* p_count,
	TonyS32 p_mask
)
{
	TonyS32 type;
	TonyS32 i;

	for (i = 0; i < g_videoManager->m_frameSets[p_frameSet][p_frame].m_hitBoxCount; i++) {
		type = g_videoManager->m_frameSets[p_frameSet][p_frame].m_hitBoxes[i].m_kind;

		if (!type) {
			type = 0xf000;
		}

		if (p_mask & type) {
			p_out[*p_count].m_object = p_object;
			p_out[*p_count].m_left = g_videoManager->m_frameSets[p_frameSet][p_frame].m_hitBoxes[i].m_left;
			p_out[*p_count].m_top = g_videoManager->m_frameSets[p_frameSet][p_frame].m_hitBoxes[i].m_top;
			p_out[*p_count].m_right = g_videoManager->m_frameSets[p_frameSet][p_frame].m_hitBoxes[i].m_right;
			p_out[*p_count].m_bottom = g_videoManager->m_frameSets[p_frameSet][p_frame].m_hitBoxes[i].m_bottom;
			p_out[*p_count].m_kind = g_videoManager->m_frameSets[p_frameSet][p_frame].m_hitBoxes[i].m_kind;
			p_out[*p_count].m_index = i;
			(*p_count)++;
		}
	}
}

// FUNCTION: TONY2 0x00401530
void __fastcall CollideWithHitList(
	GameObject* p_object,
	HitResult* p_list,
	TonyS32 p_count,
	void(__fastcall* p_callback)(GameObject*, GameObject*, HitBox*, HitBox*, TonyS32)
)
{
	HitBox other;
	HitBox own;
	TonyS32 i;
	TonyS32 j;

	for (i = 0; i < p_count; i++) {
		BuildWorldHitBox(p_list[i].m_object, (VideoManager::FrameHitBox*) &p_list[i], &other);

		for (j = 0;
			 j < g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame].m_hitBoxCount;
			 j++) {
			BuildWorldHitBox(
				p_object,
				&g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame].m_hitBoxes[j],
				&own
			);

			if (HitBoxesOverlap(&own, &other) == 1) {
				p_callback(p_object, p_list[i].m_object, &own, &other, 0);
			}
		}
	}
}

// Swept collision pass: fetches the current and previous hitbox rect lists,
// builds both world-space boxes per hitbox, gathers map candidates from all
// twelve swept-corner tiles (second MapTile list) and resolves each through
// ResolveBoxContact, rebuilding the current box after every hit.
// Fully implemented, kept as STUB because it compares at 44%: the frame size,
// both fetches, all twelve lookup/merge pairs and the resolve loop align, but
// the corner visit order and the box/scratch slot assignment need diff-driven
// refinement on top of this band's register-phase margins. Retest with the
// original compiler vintage.
// STUB: TONY2 0x00401610
void __fastcall CollideWithMapBoxes(GameObject* p_object, HitResponseFn p_callback)
{
	HitBox curBox;
	HitBox prevBox;
	TonyS32* cornerListA;
	TonyS32 cornerCountA;
	TonyS32* cornerListB;
	TonyS32 cornerCountB;
	TonyS32 list[0x1d];
	HitResult cur[5];
	HitResult prev[4];
	TonyS32 count1;
	TonyS32 count2;
	TonyS32 n;
	TonyS32 k;
	TonyS32 j;

	count1 = 0;
	count2 = 0;
	CollectHitBoxes(p_object, cur, &count1, 0xfffc);
	CollectFrameHitBoxes(
		p_object,
		p_object->m_state->m_prevFrameSet,
		p_object->m_state->m_prevFrame,
		prev,
		&count2,
		0xfffc
	);

	for (k = 0; k < count1; k++) {
		n = 0;
		prevBox.m_left = (TonyFloat) prev[k].m_left + p_object->m_state->m_prevX;
		prevBox.m_top = (TonyFloat) prev[k].m_top + p_object->m_state->m_prevY;
		prevBox.m_right = (TonyFloat) prev[k].m_right + p_object->m_state->m_prevX;
		prevBox.m_bottom = (TonyFloat) prev[k].m_bottom + p_object->m_state->m_prevY;
		curBox.m_left = (TonyFloat) cur[k].m_left + p_object->m_state->m_worldX;
		curBox.m_top = (TonyFloat) cur[k].m_top + p_object->m_state->m_worldY;
		curBox.m_right = (TonyFloat) cur[k].m_right + p_object->m_state->m_worldX;
		curBox.m_bottom = (TonyFloat) cur[k].m_bottom + p_object->m_state->m_worldY;

		g_camera->GetWallsAt((TonyS32) curBox.m_left, (TonyS32) curBox.m_top, &cornerListA, &cornerCountA);
		g_camera->GetWallsAt((TonyS32) curBox.m_right, (TonyS32) curBox.m_top, &cornerListB, &cornerCountB);
		MergeUniqueBoxes(list, &n, cornerListA, cornerCountA);
		MergeUniqueBoxes(list, &n, cornerListB, cornerCountB);
		g_camera->GetWallsAt((TonyS32) curBox.m_left, (TonyS32) curBox.m_bottom, &cornerListA, &cornerCountA);
		g_camera->GetWallsAt((TonyS32) curBox.m_right, (TonyS32) curBox.m_bottom, &cornerListB, &cornerCountB);
		MergeUniqueBoxes(list, &n, cornerListA, cornerCountA);
		MergeUniqueBoxes(list, &n, cornerListB, cornerCountB);

		g_camera->GetWallsAt((TonyS32) prevBox.m_left, (TonyS32) prevBox.m_top, &cornerListA, &cornerCountA);
		g_camera->GetWallsAt((TonyS32) prevBox.m_right, (TonyS32) prevBox.m_top, &cornerListB, &cornerCountB);
		MergeUniqueBoxes(list, &n, cornerListA, cornerCountA);
		MergeUniqueBoxes(list, &n, cornerListB, cornerCountB);
		g_camera->GetWallsAt((TonyS32) prevBox.m_left, (TonyS32) prevBox.m_bottom, &cornerListA, &cornerCountA);
		g_camera->GetWallsAt((TonyS32) prevBox.m_right, (TonyS32) prevBox.m_bottom, &cornerListB, &cornerCountB);
		MergeUniqueBoxes(list, &n, cornerListA, cornerCountA);
		MergeUniqueBoxes(list, &n, cornerListB, cornerCountB);

		g_camera->GetWallsAt((TonyS32) curBox.m_left, (TonyS32) prevBox.m_top, &cornerListA, &cornerCountA);
		g_camera->GetWallsAt((TonyS32) curBox.m_right, (TonyS32) prevBox.m_top, &cornerListB, &cornerCountB);
		MergeUniqueBoxes(list, &n, cornerListA, cornerCountA);
		MergeUniqueBoxes(list, &n, cornerListB, cornerCountB);
		g_camera->GetWallsAt((TonyS32) curBox.m_left, (TonyS32) prevBox.m_bottom, &cornerListA, &cornerCountA);
		g_camera->GetWallsAt((TonyS32) curBox.m_right, (TonyS32) prevBox.m_bottom, &cornerListB, &cornerCountB);
		MergeUniqueBoxes(list, &n, cornerListA, cornerCountA);
		MergeUniqueBoxes(list, &n, cornerListB, cornerCountB);

		for (j = 0; j < n; j++) {
			if (ResolveBoxContact(p_object, &prevBox, &curBox, (HitBox*) list[j], p_callback) == 1) {
				curBox.m_left = (TonyFloat) cur[k].m_left + p_object->m_state->m_worldX;
				curBox.m_top = (TonyFloat) cur[k].m_top + p_object->m_state->m_worldY;
				curBox.m_right = (TonyFloat) cur[k].m_right + p_object->m_state->m_worldX;
				curBox.m_bottom = (TonyFloat) cur[k].m_bottom + p_object->m_state->m_worldY;
			}
		}
	}
}

// Fully implemented, kept as STUB at ~88%: the guards, ftol pair and slope compare all
// match mathematically, but the recompile assigns the first-computed local the higher
// stack slot where the original build fills [esp+0xc] first (declaration order tested
// both ways), flipping the fild order and the fcompp polarity downstream. Note the
// sibling IsSideContactTR/IsSideContactBL get the original slots from the same shape and
// reached 100% via comparison-polarity spelling; the allocator's slot choice here is
// the vintage anomaly. Same local-slot-direction family as BlitSpriteClipY and
// IsSideContactTL. Re-annotate when solved.
// STUB: TONY2 0x00401b90
TonyS32 IsSideContactBR()
{
	TonyS32 dx;
	TonyS32 dy;

	dx = (TonyS32) (g_prevHitBox->m_left - g_obstacleBox->m_right);
	dy = (TonyS32) (g_prevHitBox->m_top - g_obstacleBox->m_bottom);

	if (dx >= 0) {
		if (dy < 0) {
			return 1;
		}

		if (-((TonyFloat) dy * g_moveDxLeft) < -((TonyFloat) dx * g_moveDyTop)) {
			return 1;
		}
	}

	return 0;
}

// FUNCTION: TONY2 0x00401c20
TonyS32 IsSideContactTR()
{
	TonyS32 dx;
	TonyS32 dy;

	dx = (TonyS32) (g_prevHitBox->m_left - g_obstacleBox->m_right);
	dy = (TonyS32) (g_obstacleBox->m_top - g_prevHitBox->m_bottom);

	if (dx >= 0) {
		if (dy < 0) {
			return 1;
		}

		if ((TonyFloat) dx * g_moveDyBottom > -((TonyFloat) dy * g_moveDxLeft)) {
			return 1;
		}
	}

	return 0;
}

// FUNCTION: TONY2 0x00401ca0
TonyS32 IsSideContactBL()
{
	TonyS32 dy;
	TonyS32 dx;

	dx = (TonyS32) (g_obstacleBox->m_left - g_prevHitBox->m_right);
	dy = (TonyS32) (g_prevHitBox->m_top - g_obstacleBox->m_bottom);

	if (dx >= 0) {
		if (dy < 0) {
			return 1;
		}

		if (-((TonyFloat) dx * g_moveDyTop) > (TonyFloat) dy * g_moveDxRight) {
			return 1;
		}
	}

	return 0;
}

// Fully implemented, kept as STUB at ~87%: the guards, ftol pair and slope compare all
// match mathematically, but the recompile assigns the first-computed local the higher
// stack slot where the original build fills [esp+0xc] first, flipping the fild order and
// the fcompp polarity. Same local-slot-direction family as BlitSpriteClipY. Re-annotate
// when solved.
// STUB: TONY2 0x00401d20
TonyS32 IsSideContactTL()
{
	TonyS32 dy;
	TonyS32 dx;

	dx = (TonyS32) (g_obstacleBox->m_left - g_prevHitBox->m_right);
	dy = (TonyS32) (g_obstacleBox->m_top - g_prevHitBox->m_bottom);

	if (dx >= 0) {
		if (dy < 0) {
			return 1;
		}

		if ((TonyFloat) dy * g_moveDxRight < (TonyFloat) dx * g_moveDyBottom) {
			return 1;
		}
	}

	return 0;
}

// Contact classifier/resolver: p_frame is the previous own box, p_otherFrame the
// current own box and p_own the obstacle. Publishes the frame pair and per-edge
// motion deltas, classifies the contact side (with the corner tie-breakers),
// bounces the velocity, applies the separation through MoveObject and fires
// the response callback with the side code.
// Fully implemented, kept as STUB because it compares at 74%: every gate,
// tie-break, bounce constant and both calls match, but the original homes
// p_object in ebp and spills p_frame where SP3 does the opposite, shifting the
// register file through the body, and the zeroed side register seeds the dx/dy
// stores. First-use allocation / zero-seed family; retest with the original
// compiler vintage.
// STUB: TONY2 0x00401da0
TonyS32 __fastcall ResolveBoxContact(
	GameObject* p_object,
	HitBox* p_frame,
	HitBox* p_otherFrame,
	HitBox* p_own,
	HitResponseFn p_callback
)
{
	TonyFloat dx;
	TonyFloat dy;
	TonyS32 side = 0;

	if (HitBoxesOverlap(p_otherFrame, p_own) == 1) {
		g_obstacleBox = p_own;
		g_prevHitBox = p_frame;
		dy = 0;
		g_moveDxLeft = p_otherFrame->m_left - p_frame->m_left;
		dx = 0;
		g_moveDxRight = p_otherFrame->m_right - p_frame->m_right;
		g_moveDyTop = p_otherFrame->m_top - p_frame->m_top;
		g_moveDyBottom = p_otherFrame->m_bottom - p_frame->m_bottom;

		if (p_otherFrame->m_right >= p_own->m_left && p_otherFrame->m_right <= p_own->m_right &&
			g_moveDxRight >= g_zeroF) {
			if (p_otherFrame->m_bottom >= p_own->m_top && p_otherFrame->m_bottom <= p_own->m_bottom &&
				g_moveDyBottom >= g_zeroF) {
				if (IsSideContactBL() == 0) {
					goto bottom;
				}
			}
			else if (p_otherFrame->m_top <= p_own->m_bottom && g_moveDyTop <= g_zeroF) {
				if (IsSideContactTL() == 0) {
					goto top;
				}
			}

			side = 8;
			p_object->m_state->m_velX = p_object->m_state->m_velX * -0.6f;
			dx = (p_otherFrame->m_right - p_own->m_left) * -1.5f;
		}
		else if (
			p_otherFrame->m_left >= p_own->m_left && p_otherFrame->m_left <= p_own->m_right && g_moveDxLeft <= g_zeroF
		) {
			if (p_otherFrame->m_bottom >= p_own->m_top && p_otherFrame->m_bottom <= p_own->m_bottom &&
				g_moveDyBottom >= g_zeroF) {
				if (IsSideContactBR() == 0) {
					goto bottom;
				}
			}
			else if (p_otherFrame->m_top <= p_own->m_bottom && g_moveDyTop <= g_zeroF) {
				if (IsSideContactTR() == 0) {
					goto top;
				}
			}

			side = 4;
			p_object->m_state->m_velX = p_object->m_state->m_velX * -0.4f;
			dx = (p_otherFrame->m_left - p_own->m_right) * -1.5f;
		}
		else if (
			p_otherFrame->m_bottom >= p_own->m_top && p_otherFrame->m_bottom <= p_own->m_bottom &&
			g_moveDyBottom >= g_zeroF
		) {
		bottom:
			side = 1;
			p_object->m_state->m_velY = p_object->m_state->m_velY * -0.6f;
			dy = (p_otherFrame->m_bottom - p_own->m_top) * -1.5f - 4.1f;
		}
		else if (p_otherFrame->m_top <= p_own->m_bottom && g_moveDyTop <= g_zeroF) {
		top:
			side = 2;
			p_object->m_state->m_velY = p_object->m_state->m_velY * -0.4f;
			dy = (p_otherFrame->m_top - p_own->m_bottom) * -1.5f;
		}

		MoveObject(p_object, 0, dx, dy);

		if (p_callback) {
			p_callback(p_object, p_own, side, 0);
		}

		return 1;
	}

	return 0;
}

// FUNCTION: TONY2 0x00402080
void __fastcall MergeUniqueBoxes(TonyS32* p_list, TonyS32* p_count, TonyS32* p_values, TonyS32 p_valueCount)
{
	TonyS32 found;
	TonyS32 i;
	TonyS32 j;

	for (i = 0; i < p_valueCount; i++) {
		found = 0;

		for (j = 0; j < *p_count; j++) {
			if (p_values[i] == p_list[j]) {
				found = 1;
				break;
			}
		}

		if (!found) {
			p_list[*p_count] = p_values[i];
			(*p_count)++;
		}
	}
}

// FUNCTION: TONY2 0x004020f0
void __fastcall BuildWorldHitBox(GameObject* p_object, VideoManager::FrameHitBox* p_box, HitBox* p_out)
{
	p_out->m_left = p_box->m_left + p_object->m_state->m_worldX;
	p_out->m_top = p_box->m_top + p_object->m_state->m_worldY;
	p_out->m_right = p_box->m_right + p_object->m_state->m_worldX;
	p_out->m_bottom = p_box->m_bottom + p_object->m_state->m_worldY;
	p_out->m_kind = p_box->m_kind;
}

// FUNCTION: TONY2 0x00402130
void __fastcall BuildPrevHitBox(GameObject* p_object, VideoManager::FrameHitBox* p_box, HitBox* p_out)
{
	TonyFloat x;
	TonyFloat y;

	GetPrevPosition(p_object, &x, &y);
	p_out->m_left = p_box->m_left + x;
	p_out->m_top = p_box->m_top + y;
	p_out->m_right = p_box->m_right + x;
	p_out->m_bottom = p_box->m_bottom + y;
	p_out->m_kind = p_box->m_kind;
}

// FUNCTION: TONY2 0x00402180
void __fastcall RefreshHitBoxes(GameObject* p_object)
{
	TonyS32 i;
	TonyS32 x;
	TonyS32 y;

	p_object->m_state->m_boundsMinX = 1000;
	p_object->m_state->m_boundsMaxX = -1000;
	p_object->m_state->m_boundsMinY = 1000;
	p_object->m_state->m_boundsMaxY = -1000;

	for (i = 0; i < g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame].m_partCount;
		 i++) {
		x = g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame].m_parts[i].m_dx;
		y = g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame].m_parts[i].m_dy;

		p_object->m_state->m_boundsMinX = p_object->m_state->m_boundsMinX < x ? p_object->m_state->m_boundsMinX : x;
		p_object->m_state->m_boundsMinY = p_object->m_state->m_boundsMinY < y ? p_object->m_state->m_boundsMinY : y;

		TonyS16 sprite =
			g_videoManager->m_frameSets[p_object->m_state->m_frameSet][p_object->m_state->m_frame].m_parts[i].m_sprite;
		TonyU16* entry = g_videoManager->m_sprites[sprite];

		TonyS32 x2 = x + entry[0] - 1;
		p_object->m_state->m_boundsMaxX = p_object->m_state->m_boundsMaxX > x2 ? p_object->m_state->m_boundsMaxX : x2;

		entry = g_videoManager->m_sprites[sprite];
		TonyS32 y2 = y + entry[0] - 1;
		p_object->m_state->m_boundsMaxY = p_object->m_state->m_boundsMaxY > y2 ? p_object->m_state->m_boundsMaxY : y2;
	}
}

// Fully implemented, kept as STUB because it compares at 93%: all four corner
// lookups, the dedupe merges and the sweep call match, but two of the eight corner
// out-locals land in swapped stack slots. Tested exhaustively: declaration order,
// full renames (VC5 /O2 slot order is name-independent, unlike VC6 /Od), and all
// four local VC5 builds (RTM/SP1/SP2/SP3) produce identical output - the slot
// order comes from compiler-internal allocation state; needs the original build.
// STUB: TONY2 0x004022c0
void __fastcall CollideWithGroundLines(GameObject* p_object, HitResponseFn p_callback, TonyS32 p_dx, TonyS32 p_dy)
{
	TonyS32 list[0x20];
	TonyS32 count;
	TonyS32* listA;
	TonyS32 countA;
	TonyS32* listB;
	TonyS32 countB;
	TonyS32* listC;
	TonyS32 countC;
	TonyS32* listD;
	TonyS32 countD;

	g_camera->GetFloorsAt(p_object->m_state->m_prevXInt + p_dx, p_object->m_state->m_prevYInt + p_dy, &listA, &countA);
	g_camera->GetFloorsAt(p_object->m_state->m_prevXInt + p_dx, p_object->m_state->m_worldYInt + p_dy, &listB, &countB);
	g_camera->GetFloorsAt(p_object->m_state->m_worldXInt + p_dx, p_object->m_state->m_prevYInt + p_dy, &listC, &countC);
	g_camera
		->GetFloorsAt(p_object->m_state->m_worldXInt + p_dx, p_object->m_state->m_worldYInt + p_dy, &listD, &countD);

	count = 0;
	MergeUniqueBoxes(list, &count, listA, countA);
	MergeUniqueBoxes(list, &count, listB, countB);
	MergeUniqueBoxes(list, &count, listC, countC);
	MergeUniqueBoxes(list, &count, listD, countD);

	TestGroundLines(p_object, count, (HitBox**) list, p_callback, (TonyFloat) p_dx, (TonyFloat) p_dy);
}

// FUNCTION: TONY2 0x00402410
void __fastcall GetGroundSlope(HitBox* p_frame, TonyFloat* p_slope, TonyS32* p_direction)
{
	*p_slope = (p_frame->m_bottom - p_frame->m_top) / fabs(p_frame->m_right - p_frame->m_left);

	if (p_frame->m_bottom < p_frame->m_top) {
		*p_direction = 8;
	}
	else {
		*p_direction = 4;
	}
}

// Fully implemented, kept as STUB because it compares at 81%: the register allocation,
// range gates, slope and both edge lines match (the __fastcall callback with the frame in
// edx resolved the earlier base-register swap), but the original schedules the two line
// evaluations as one interleaved FPU burst around the threshold compare (half of lineA
// hoisted above the fcompp) that separate statements do not reproduce. FPU-scheduler
// margin. Re-annotate when solved.
// STUB: TONY2 0x00402450
void __fastcall TestGroundLines(
	GameObject* p_object,
	TonyS32 p_count,
	HitBox** p_list,
	HitResponseFn p_callback,
	TonyFloat p_dx,
	TonyFloat p_dy
)
{
	TonyFloat x;
	TonyFloat slope;
	TonyFloat lineA;
	TonyFloat lineB;
	TonyS32 i;

	for (i = 0; i < p_count; i++) {
		x = p_dx + p_object->m_state->m_worldX;

		if ((x >= p_list[i]->m_left && x <= p_list[i]->m_right) ||
			(p_object->m_state->m_prevX + p_dx >= p_list[i]->m_left &&
			 p_object->m_state->m_prevX + p_dx <= p_list[i]->m_right)) {
			slope = (p_list[i]->m_bottom - p_list[i]->m_top) / (p_list[i]->m_right - p_list[i]->m_left);
			lineB = (p_object->m_state->m_prevX + p_dx - p_list[i]->m_left) * slope + p_list[i]->m_top;
			lineA = (x - p_list[i]->m_left) * slope + p_list[i]->m_top;

			if (lineB > p_object->m_state->m_prevY + p_dy - 1.0) {
				if (p_object->m_state->m_worldY + p_dy > lineA) {
					p_callback(p_object, p_list[i], 0, p_object->m_state->m_worldY + p_dy - lineA);
				}
			}
		}
	}
}

// FUNCTION: TONY2 0x00402540
TonyS32 __fastcall IsAnimationDone(GameObject* p_object)
{
	GameObject::State* state = p_object->m_state;

	if (g_videoManager->m_frameSets[state->m_frameSet][state->m_frame + 1].m_duration == -1 &&
		state->m_frameTime <= state->m_animSpeed) {
		return 1;
	}

	return 0;
}

// FUNCTION: TONY2 0x00402590
void __fastcall SpriteDestroy(GameObject* p_object)
{
	if (p_object->m_head->m_flags & 0x200) {
		g_videoManager->ReleaseFrameSet(p_object->m_ext->m_idleSetR);
	}
}

// FUNCTION: TONY2 0x004025b0
TonyS32 __fastcall GetFrameCount(GameObject* p_object)
{
	TonyS32 count = 0;
	VideoManager::AnimFrame* rec = g_videoManager->m_frameSets[p_object->m_state->m_frameSet];

	while (rec->m_duration != -1) {
		rec++;
		count++;
	}

	return count;
}

// FUNCTION: TONY2 0x004025e0
void __fastcall BouncerInit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	p_object->m_tickFn = BouncerTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = BouncerDestroy;
	BouncerReset(p_object);
}

// FUNCTION: TONY2 0x00402610
void __fastcall BouncerReset(GameObject* p_object)
{
	EnemyReset(p_object);
	SetEnemyState(p_object, 1);
	BouncerApplyFacing(p_object);
	g_bouncerCount++;
	((OverlayData*) p_object->m_head)->m_arg1 = 1;
	p_object->m_ext->m_idleSetL = 1;
}

// FUNCTION: TONY2 0x00402650
void __fastcall EnemyReinit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	EnemyResolveSets(p_object);
}

// FUNCTION: TONY2 0x00402670
void __fastcall EnemyResolveSets(GameObject* p_object)
{
	EnemyResolveBaseSets(p_object);
}

// FUNCTION: TONY2 0x00402680
void __fastcall BouncerDestroy(GameObject* p_object)
{
	g_bouncerCount--;
	ObjectFreeTemplate(p_object);
}

// FUNCTION: TONY2 0x00402690
TonyS32 __fastcall BouncerTick(GameObject* p_object)
{
	switch (p_object->m_state->m_behavior) {
	case 1:
		BouncerWalk(p_object);
		break;
	case 5:
		EnemyDeathTick(p_object);
		break;
	}

	return 0;
}

// FUNCTION: TONY2 0x004026c0
void __fastcall BouncerWalk(GameObject* p_object)
{
	EnemyBaseTick(p_object);

	if (p_object->m_head->m_facing == 4) {
		p_object->m_state->m_velX = -p_object->m_ext->m_walkSpeed;
	}

	if (p_object->m_head->m_facing == 8) {
		p_object->m_state->m_velX = p_object->m_ext->m_walkSpeed;
	}

	p_object->ApplyGravity(0, 0, -1234.0f, -1234.0f);
	CollideWithGroundLines(p_object, BouncerHitWorld, 0, 0);
	CollideWithMapBoxes(p_object, BouncerHitFrame);
}

// FUNCTION: TONY2 0x00402730
TonyFloat __fastcall BouncerHitFrame(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value)
{
	if (p_kind == 4 || p_kind == 8) {
		BouncerTurnAround(p_object);
	}

	return 0.0f;
}

// FUNCTION: TONY2 0x00402750
void __fastcall BouncerTurnAround(GameObject* p_object)
{
	if (p_object->m_head->m_facing == 4) {
		p_object->m_head->m_facing = 8;
	}
	else if (p_object->m_head->m_facing == 8) {
		p_object->m_head->m_facing = 4;
	}

	PlayObjectSound(p_object, 0x1b, -1, -1);
	BouncerApplyFacing(p_object);
}

// Fully implemented, kept as STUB because it compares at 78%: everything matches except
// the original zeroes edx once at entry and reuses it for both the mode argument and the
// pushed 0.0f, where the recompile pushes an immediate zero and zeroes edx at the call.
// Zero-register seeding margin (see TickAll). Re-annotate when solved.
// STUB: TONY2 0x00402790
TonyFloat __fastcall BouncerHitWorld(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value)
{
	p_object->m_state->m_velY = -p_object->m_ext->m_jumpSpeed;
	p_value = -p_value;
	MoveObject(p_object, 0, 0.0f, p_value);
	PlayObjectSound(p_object, 0x1b, -1, -1);
	return p_value;
}

// FUNCTION: TONY2 0x004027e0
void __fastcall EnemyDeathTick(GameObject* p_object)
{
	EnemyBaseTick(p_object);
	p_object->ApplyGravity(0, 0, -1234.0f, -1234.0f);
}

// FUNCTION: TONY2 0x00402800
void __fastcall BouncerApplyFacing(GameObject* p_object)
{
	TonyS32 kind = p_object->m_head->m_facing;

	if (kind == 4) {
		SetFrameSet(p_object, p_object->m_ext->m_walkSetR);
		return;
	}

	if (kind == 8) {
		SetFrameSet(p_object, p_object->m_ext->m_idleSetR);
	}
}

// FUNCTION: TONY2 0x00402830
void __fastcall StaticEnemyInit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	p_object->m_tickFn = EnemyBaseTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = NULL;
	EnemyReset(p_object);
}

// FUNCTION: TONY2 0x00402860
void __fastcall StaticEnemyReinit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	EnemyResolveBaseSets(p_object);
}

// FUNCTION: TONY2 0x00402880
TonyS32 __fastcall EnemyBaseTick(GameObject* p_object)
{
	WaterTick(p_object);

	if (p_object->m_state->m_moveState > 0) {
		p_object->m_state->m_moveState--;
		ObjectSetFlags(p_object, 0x20, 0);

		if (p_object->m_state->m_moveState == 0) {
			ObjectSetFlags(p_object, 0, 0x20);
		}
	}

	if (!(p_object->m_head->m_flags & 0x10)) {
		CollideWithHitList(
			p_object,
			(HitResult*) g_objectManager->m_playerFeetHits,
			g_objectManager->m_playerFeetHitCount,
			EnemyTouch
		);
	}

	if (p_object->m_state->m_cooldown > 0) {
		p_object->m_state->m_cooldown--;
		return 0;
	}

	CollideWithHitList(
		p_object,
		(HitResult*) g_objectManager->m_playerBodyHits,
		g_objectManager->m_playerBodyHitCount,
		EnemyTouch
	);
	return 0;
}

// FUNCTION: TONY2 0x00402930
void __fastcall EnemyReset(GameObject* p_object)
{
	ResetObjectAnimation(p_object);
	p_object->m_state->m_cooldown = 0;
	p_object->m_state->m_moveState = 0;
	p_object->m_state->m_typeFlags = 0;
	SetEnemyState(p_object, 0);
}

// FUNCTION: TONY2 0x00402960
void __fastcall EnemyResolveBaseSets(GameObject* p_object)
{
	p_object->m_ext->m_jumpSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_walkSetL, 2);
	p_object->m_ext->m_walkSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_walkSetL, 0);
	p_object->m_ext->m_walkSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_idleSetR, 2);
	p_object->m_ext->m_idleSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_idleSetR, 0);
}

// Fully implemented, kept as STUB because it compares at 80%: the stomp gate (box type
// bit 1, the swept player-bottom/enemy-top comparison through the dead fifth-arg slot),
// victim callback, health decrement, death transition and invincibility all match, but
// the original caches the own-frame argument in ebp at entry where the recompile
// re-reads the slot, and the two float locals take swapped slots. Same allocator-
// direction family as JoystickEnumCallback (0x405430). Re-annotate when solved.
// STUB: TONY2 0x004029d0
void __fastcall EnemyTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_extra
)
{
	TonyFloat a;
	TonyFloat b;

	if (p_otherFrame->m_kind & 1) {
		GetPrevPosition(p_other, (TonyFloat*) &p_extra, &a);
		GetPrevPosition(p_object, (TonyFloat*) &p_extra, &b);

		if (p_otherFrame->m_bottom - p_other->m_state->m_worldY + a < p_own->m_top - p_object->m_state->m_worldY + b &&
			p_otherFrame->m_bottom >= p_own->m_top && p_object->m_state->m_moveState == 0) {
			if (p_other->m_state->m_touchFn) {
				p_other->m_state->m_touchFn(p_other, p_object, -1);
			}

			p_object->m_state->m_cooldown = 2;
			((OverlayData*) p_object->m_head)->m_arg1--;
			PlayObjectSound(p_object, 0x17, -1, -1);

			if (((OverlayData*) p_object->m_head)->m_arg1 <= 0) {
				SetEnemyState(p_object, 5);
			}

			if ((p_object->m_state->m_typeFlags & 1) > 0) {
				p_object->m_state->m_moveState = 0x46;
			}
		}
	}
	else {
		if (p_object->m_state->m_behavior != 5 && p_other->m_state->m_touchFn) {
			p_other->m_state->m_touchFn(p_other, p_object, p_object->m_ext->m_idleSetL);
		}
	}
}

// FUNCTION: TONY2 0x00402af0
void __fastcall SetEnemyState(GameObject* p_object, TonyS32 p_state)
{
	if (p_state == 5) {
		p_object->m_state->m_velX = 0.0f;
		p_object->m_state->m_velY = p_object->m_ext->m_jumpSpeed * -1.5;
		p_object->m_head->m_flags |= 0x10;

		if (p_object->m_head->m_facing == 4) {
			SetFrameSet(p_object, p_object->m_ext->m_jumpSetR);
		}

		if (p_object->m_head->m_facing == 8) {
			SetFrameSet(p_object, p_object->m_ext->m_walkSetL);
		}
	}

	p_object->m_state->m_behavior = p_state;
}

// FUNCTION: TONY2 0x00402b60
void __fastcall BindPlayerTemplate(GameObject* p_object, PlayerTemplate* p_template)
{
	*(PlayerTemplate::Head*) p_object->m_head = p_template->m_head;
	p_object->m_state = (GameObject::State*) ((PlayerTemplate::Head*) p_object->m_head + 1);
	p_object->m_ext = &p_template->m_ext;
	p_object->m_state->m_template = (ObjectTemplate*) p_template;
}

// FUNCTION: TONY2 0x00402b90
void __fastcall PlayerInit(GameObject* p_object, PlayerTemplate* p_template)
{
	BindPlayerTemplate(p_object, p_template);
	p_object->m_tickFn = PlayerTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = NULL;
	p_object->m_state->m_patrolStep = 0;
	p_object->m_state->m_touchFn = NULL;
	p_object->m_state->m_hitWorldFn = PlayerHitWorld;
	p_object->m_state->m_hitFrameFn = PlayerHitFrame;
	PlayerActivate(p_object);
}

// FUNCTION: TONY2 0x00402be0
void __fastcall PlayerReinit(GameObject* p_object, PlayerTemplate* p_template)
{
	BindPlayerTemplate(p_object, p_template);
	PlayerResolveSets(p_object);
}

// FUNCTION: TONY2 0x00402c00
void __fastcall PlayerResolveSets(GameObject* p_object)
{
	p_object->m_ext->m_idleSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_idleSetR, 2);
	p_object->m_ext->m_idleSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_idleSetR, 0);
	p_object->m_ext->m_walkSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_walkSetR, 2);
	p_object->m_ext->m_walkSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_walkSetR, 0);
	p_object->m_ext->m_jumpSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_jumpSetR, 2);
	p_object->m_ext->m_jumpSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_jumpSetR, 0);
	p_object->m_ext->m_fallSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_fallSetR, 2);
	p_object->m_ext->m_fallSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_fallSetR, 0);
	p_object->m_ext->m_hurtSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_hurtSetR, 2);
	p_object->m_ext->m_hurtSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_hurtSetR, 0);
	p_object->m_ext->m_duckSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_duckSetR, 2);
	p_object->m_ext->m_duckSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_duckSetR, 0);
	p_object->m_ext->m_poseSet = g_videoManager->GetFrameSet(p_object->m_ext->m_poseSet, 0);
}

// FUNCTION: TONY2 0x00402d60
TonyS32 __fastcall PlayerTick(GameObject* p_object)
{
	TonyS32 saved;

	p_object->m_state->m_animSpeed = fabs(p_object->m_state->m_velX) / fabs(p_object->m_ext->m_walkSpeed);
	saved = g_inputManager->m_buttons;

	if (p_object->m_head->m_flags & 0x1000) {
		g_inputManager->m_buttons = 0;
	}

	if (p_object->m_head->m_flags & 0x100) {
		ObjectSetFlags(p_object, 0, 0x100);

		if (p_object->m_state->m_moveState >= 3 && p_object->m_state->m_moveState <= 4) {
			SetPlayerState(p_object, 1, 0);
		}
	}

	switch (p_object->m_state->m_moveState) {
	case 4:
		PlayerFall(p_object);
		break;
	case 1:
		PlayerIdle(p_object);
		break;
	case 2:
		PlayerWalk(p_object);
		break;
	case 3:
		PlayerJump(p_object);
		break;
	case 5:
		PlayerBounce(p_object);
		break;
	case 6:
		PlayerDie(p_object);
		break;
	case 7:
		PlayerCelebrate(p_object);
		break;
	case 8:
		PlayerDuck(p_object);
		break;
	case 10:
		PlayerSwim(p_object);
		break;
	case 11:
		PlayerSurf(p_object);
		break;
	case 12:
		PlayerHang(p_object);
		break;
	case 13:
		PlayerDrown(p_object);
		break;
	}

	WaterTick(p_object);

	if (p_object->m_state->m_dropTicks > 0) {
		p_object->m_state->m_dropTicks--;
	}

	ObjectSetFlags(p_object, 0, 0x408);

	if (p_object->m_state->m_moveState != 13) {
		CollideWithGroundLines(p_object, p_object->m_state->m_hitWorldFn, 0, 0);
	}

	CollectHitBoxes(p_object, (HitResult*) g_objectManager->m_playerHits, &g_objectManager->m_playerHitCount, 0xfffc);
	CollectHitBoxes(
		p_object,
		(HitResult*) g_objectManager->m_playerFeetHits,
		&g_objectManager->m_playerFeetHitCount,
		1
	);

	if (!(p_object->m_head->m_flags & 0x1000)) {
		CollectHitBoxes(
			p_object,
			(HitResult*) g_objectManager->m_playerBodyHits,
			&g_objectManager->m_playerBodyHitCount,
			2
		);
	}

	if (p_object->m_state->m_moveState != 13) {
		CollideWithMapBoxes(p_object, p_object->m_state->m_hitFrameFn);
	}

	if (p_object->m_state->m_moveState > 0) {
		if (p_object->m_state->m_moveState > 2) {
			if (p_object->m_state->m_moveState == 12 && p_object->m_state->m_velY > 6.0) {
				SetPlayerState(p_object, 4, 0);
			}
		}
		else if (p_object->m_state->m_velY > 1.5) {
			SetPlayerState(p_object, 4, 0);
		}
	}

	switch (p_object->m_state->m_moveState) {
	case 1:
	case 10:
		if (p_object->m_state->m_pendingState != 0) {
			SetPlayerState(p_object, p_object->m_state->m_pendingState, 0);
			p_object->m_state->m_pendingState = 0;
		}
	}

	if (p_object->m_head->m_flags & 0x1000) {
		g_inputManager->m_buttons = (TonyU16) saved;
	}

	return 0;
}

// FUNCTION: TONY2 0x00402fe0
void __fastcall PlayerActivate(GameObject* p_object)
{
	ResetObjectAnimation(p_object);
	g_objectManager->SetPlayer(p_object);
	g_camera->AttachPlayer(p_object);
	PlayerClearCounters(p_object);
	SetPlayerState(p_object, 4, 0);
	p_object->m_state->m_pendingState = 0;
	p_object->m_head->m_flags |= 0x80;
	p_object->m_state->m_typeFlags = 0;
	p_object->m_state->m_behavior = 0;
	p_object->m_state->m_jingleActive = 0;
	p_object->m_state->m_savedCamera = NULL;
	p_object->m_state->m_dropTicks = 0;
	ObjectSetFlags(p_object, 0x4000, 0);
	SnapshotPosition(p_object);
}

// Fully implemented, kept as STUB because it compares at 79%: all four response cases,
// their velocity clears and the state-4 rest/move transition match; the only diff is that
// the original zeroes edx first and pushes the zeroed register as the 0.0f argument in
// every arm, where the recompile pushes an immediate zero and zeroes edx afterwards.
// Zero-register seeding margin (see TickAll, BouncerHitWorld). Re-annotate when solved.
// STUB: TONY2 0x00403070
TonyFloat __fastcall PlayerHitFrame(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value)
{
	switch (p_kind) {
	case 8:
		MoveObject(p_object, 0, -p_value, 0.0f);
		p_object->m_state->m_velX = 0.0f;
		return p_value;
	case 4:
		MoveObject(p_object, 0, -p_value, 0.0f);
		p_object->m_state->m_velX = 0.0f;
		return p_value;
	case 1:
		if (p_object->m_state->m_velY < 0.0) {
			MoveObject(p_object, 0, 0.0f, -p_value);
			p_object->m_state->m_velY = 0.0f;
			return p_value;
		}
		break;
	case 2:
		if (p_object->m_state->m_velY > 0.0) {
			MoveObject(p_object, 0, 0.0f, -p_value);
			p_object->m_state->m_velY = 0.0f;

			if (p_object->m_state->m_moveState == 4) {
				SetPlayerState(p_object, (p_object->m_state->m_velX == 0.0) ? 1 : 2, 0);
			}

			return p_value;
		}
		break;
	}

	return 0.0;
}

// FUNCTION: TONY2 0x004031b0
void __fastcall PlayerClearCounters(GameObject* p_object)
{
	p_object->m_state->m_cereals = 0;
	p_object->m_state->m_lives = 0;
	p_object->m_state->m_reserved1 = 0;
}

// FUNCTION: TONY2 0x004031d0
TonyS32 __fastcall SetPlayerState(GameObject* p_object, TonyS32 p_state, TonyS32 p_soft)
{
	TonyS32 track;

	if (p_soft == 1) {
		if (p_object->m_state->m_moveState == 6 || p_object->m_state->m_moveState == 7 ||
			p_object->m_state->m_moveState == 13) {
			return 0;
		}
	}

	track = 0;

	if (p_object->m_state->m_moveState == 4 || p_object->m_state->m_moveState == 8) {
		g_camera->SetFollowOffset(0, 0);
	}
	else if (p_object->m_state->m_moveState == 0xb) {
		p_object->m_head->m_layer = 0x80;
		g_soundManager->StopHandle((void*) p_object->m_state->m_surfSound);
	}

	switch (p_state) {
	case 1:
		if (p_object->m_state->m_jingleActive == 1) {
			if (g_soundManager != NULL) {
				g_soundManager->StopSong();
				g_soundManager->ResumeSong(p_object->m_state->m_savedSong);
			}

			p_object->m_state->m_jingleActive = 0;
		}

		if (p_object->m_state->m_moveState == 4) {
			if (p_object->m_head->m_facing & 8) {
				SetFrameSet(p_object, p_object->m_ext->m_fallSetR);
				QueueFrameSet(p_object, p_object->m_ext->m_idleSetR);
			}
			else {
				SetFrameSet(p_object, p_object->m_ext->m_fallSetL);
				QueueFrameSet(p_object, p_object->m_ext->m_idleSetL);
			}
		}
		else if (p_object->m_head->m_facing & 8) {
			SetFrameSet(p_object, p_object->m_ext->m_idleSetR);
		}
		else {
			SetFrameSet(p_object, p_object->m_ext->m_idleSetL);
		}
		break;
	case 4:
		if (p_object->m_head->m_facing & 8) {
			SetFrameSet(p_object, p_object->m_ext->m_fallSetR);
		}
		else {
			SetFrameSet(p_object, p_object->m_ext->m_fallSetL);
		}
		break;
	case 2:
		if (p_object->m_state->m_jingleActive == 1) {
			if (g_soundManager != NULL) {
				g_soundManager->StopSong();
				g_soundManager->ResumeSong(p_object->m_state->m_savedSong);
			}

			p_object->m_state->m_jingleActive = 0;
		}

		if (p_object->m_head->m_facing & 8) {
			SetFrameSet(p_object, p_object->m_ext->m_walkSetR);
		}
		else {
			SetFrameSet(p_object, p_object->m_ext->m_walkSetL);
		}
		break;
	case 3:
		p_object->m_state->m_velY = -p_object->m_ext->m_jumpAccel;
		p_object->m_state->m_behavior = 5;
		g_camera->SetFollowOffset(0, 100.0f);
		SetFrameSet(
			p_object,
			(p_object->m_head->m_facing & 8) ? p_object->m_ext->m_jumpSetR : p_object->m_ext->m_jumpSetL
		);
		PlayObjectSound(p_object, 6, -1, -1);
		break;
	case 5:
		p_object->m_state->m_velY = p_object->m_ext->m_jumpSpeed * -1.25;
		SetFrameSet(
			p_object,
			(p_object->m_head->m_facing & 8) ? p_object->m_ext->m_jumpSetR : p_object->m_ext->m_jumpSetL
		);
		PlayObjectSound(p_object, 2, -1, -1);
		break;
	case 6:
		p_object->m_state->m_velY = p_object->m_ext->m_jumpSpeed * -0.5;

		if (p_object->m_head->m_facing & 8) {
			SetFrameSet(p_object, p_object->m_ext->m_hurtSetR);
		}
		else {
			SetFrameSet(p_object, p_object->m_ext->m_hurtSetL);
		}

		PlayObjectSound(p_object, 0x16, -1, -1);
		break;
	case 7:
		p_object->m_state->m_velX = 0.0f;
		p_object->m_state->m_velY = 0.0f;
		SetFrameSet(p_object, p_object->m_ext->m_poseSet);
		break;
	case 8:
		p_object->m_state->m_cooldown = 0;

		if (p_object->m_head->m_facing & 8) {
			SetFrameSet(p_object, p_object->m_ext->m_duckSetR);
		}
		else {
			SetFrameSet(p_object, p_object->m_ext->m_duckSetL);
		}
		break;
	case 10:
		p_object->m_state->m_velY = 0.0f;

		if (p_object->m_head->m_facing & 8) {
			SetFrameSet(p_object, p_object->m_ext->m_specialSetR);
		}
		else {
			SetFrameSet(p_object, p_object->m_ext->m_specialSetL);
		}
		break;
	case 11:
		p_object->m_state->m_velY = 0.0f;
		p_object->m_head->m_layer = 0xc0;
		PlayObjectSound(p_object, 8, -1, -1);
		p_object->m_state->m_surfSound = PlayObjectSound(p_object, 0x11, 0x40, -1);

		if (g_soundManager != NULL) {
			switch (g_camera->m_world) {
			case 0:
				track = g_gameManager->m_jingles[7];
				break;
			case 1:
				track = g_gameManager->m_jingles[8];
				break;
			case 2:
				track = g_gameManager->m_jingles[9];
				break;
			default:
				track = g_gameManager->m_jingles[7];
				break;
			}

			if (g_soundManager->m_currentSong != track) {
				p_object->m_state->m_savedSong = g_soundManager->SuspendSong();
				g_soundManager->PlaySong(track);
				p_object->m_state->m_jingleActive = 1;
			}
		}

		if (p_object->m_head->m_facing & 8) {
			SetFrameSet(p_object, p_object->m_ext->m_specialSetR);
		}
		else {
			SetFrameSet(p_object, p_object->m_ext->m_specialSetL);
		}
		break;
	case 12:
		if (p_object->m_head->m_facing & 8) {
			SetFrameSet(p_object, p_object->m_ext->m_specialSetR);
		}
		else {
			SetFrameSet(p_object, p_object->m_ext->m_specialSetL);
		}
		break;
	case 13:
		ObjectSetFlags(p_object, 0x1000, 0);
		g_camera->LockScroll();
		PlayObjectSound(p_object, 0x16, -1, -1);
		break;
	}

	p_object->m_state->m_moveState = p_state;
	return 1;
}

// Fully implemented, kept as STUB because it compares at 95%: both speed clamps, the
// direction flag updates, the frame-set switches and the mover call match, but for the
// right-clamp min ternary the original loads the limit and compares with
// fld/fld st(1)/fcompp where cl 11.00.7022 emits the shorter fcom [mem] (every ternary
// spelling and /G3-/G6, /Op tested). Same emission also appears in the original
// ApplyGravity — a third compiler-variant fingerprint besides slot direction and
// zero-register seeding. Re-annotate when the vintage is found.
// STUB: TONY2 0x00403640
void __fastcall PlayerFall(GameObject* p_object)
{
	TonyFloat speed;

	speed = 0.0f;
	p_object->m_state->m_animSpeed = 0.0f;

	if (g_inputManager->m_buttons & 4) {
		speed = p_object->m_ext->m_walkAccel;
		p_object->m_state->m_velX = (p_object->m_state->m_velX - speed <= -p_object->m_ext->m_walkSpeed)
										? -p_object->m_ext->m_walkSpeed
										: (p_object->m_state->m_velX - speed);
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~8) | 4;
	}

	if (g_inputManager->m_buttons & 8) {
		speed = p_object->m_ext->m_walkAccel;
		p_object->m_state->m_velX = (p_object->m_ext->m_walkSpeed > speed + p_object->m_state->m_velX)
										? (speed + p_object->m_state->m_velX)
										: p_object->m_ext->m_walkSpeed;
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~4) | 8;
	}

	if (p_object->m_head->m_facing & 4) {
		if (p_object->m_state->m_frameSet != p_object->m_ext->m_fallSetL) {
			SetFrameSet(p_object, p_object->m_ext->m_fallSetL);
		}
	}

	if (p_object->m_head->m_facing & 8) {
		if (p_object->m_state->m_frameSet != p_object->m_ext->m_fallSetR) {
			SetFrameSet(p_object, p_object->m_ext->m_fallSetR);
		}
	}

	p_object->ApplyGravity(speed, 0, -1234.0f, -1234.0f);
}

// FUNCTION: TONY2 0x00403750
void __fastcall PlayerIdle(GameObject* p_object)
{
	TonyFloat slope;
	TonyS32 direction;

	slope = 0.0f;
	p_object->m_state->m_animSpeed = 1.0f;

	if (p_object->m_head->m_flags & 8) {
		GetGroundSlope(p_object->m_state->m_groundBox, &slope, &direction);
	}

	if (g_inputManager->m_buttons & 4) {
		if (slope > -1.0 || direction == 8) {
			p_object->m_head->m_facing = 4;
			SetPlayerState(p_object, 2, 0);
		}
	}

	if (g_inputManager->m_buttons & 8) {
		if (slope > -1.0 || direction == 4) {
			p_object->m_head->m_facing = 8;
			SetPlayerState(p_object, 2, 0);
		}
	}

	if (((PlayerTemplate::Head*) p_object->m_head)->m_mapNode == 0) {
		if (g_inputManager->m_buttons & 1) {
			SetPlayerState(p_object, 3, 0);
		}

		if (g_inputManager->m_buttons & 2) {
			SetPlayerState(p_object, 8, 0);
		}
	}

	p_object->ApplyGravity(0, 0, -1234.0f, -1234.0f);
}

// Fully implemented, kept as STUB because it compares at ~76%: the walk-cycle sound,
// slope dispatch (goto layout matching the binary CFG), clamps, frame-set switches and
// the duplicated mover calls match; the residue is the slot-direction family (locals land
// in different stack slots under cl 11.00.7022) and the recompile caching the constant 8
// in ebx. See JoystickEnumCallback (0x405430). Re-annotate when the vintage is found.
// STUB: TONY2 0x00403870
void __fastcall PlayerWalk(GameObject* p_object)
{
	TonyFloat target;
	TonyFloat step;
	TonyFloat slope;
	TonyS32 steps;
	double f;
	TonyS32 direction;

	slope = 0.0f;
	steps = 0;

	if (p_object->m_state->m_frame == 0) {
		PlayObjectSound(p_object, 3, -1, -1);
	}

	if (p_object->m_head->m_flags & 8) {
		GetGroundSlope(p_object->m_state->m_groundBox, &slope, &direction);

		if (fabs(slope) > 0.01) {
			if ((g_inputManager->m_buttons & 4 && direction == 8) ||
				(g_inputManager->m_buttons & 8 && direction == 4)) {
				target = p_object->m_ext->m_walkSpeed;
				step = p_object->m_ext->m_walkAccel;

				if (g_inputManager->m_buttons & 4) {
					p_object->m_state->m_velX =
						(p_object->m_state->m_velX - step <= -target) ? -target : (p_object->m_state->m_velX - step);
					p_object->m_head->m_facing = (p_object->m_head->m_facing & ~8) | 4;
					steps = (TonyS32) step;
				}

				if (g_inputManager->m_buttons & 8) {
					p_object->m_state->m_velX =
						(step + p_object->m_state->m_velX < target) ? (step + p_object->m_state->m_velX) : target;
					p_object->m_head->m_facing = (p_object->m_head->m_facing & ~4) | 8;
					steps = (TonyS32) step;
				}

				p_object->m_state->m_velY += fabs(slope) * fabs(p_object->m_state->m_velX);
				goto framesets;
			}

			if (slope > -1.0) {
				f = fabs(slope * 0.1);
				step = p_object->m_ext->m_walkAccel - f * p_object->m_ext->m_walkAccel;
				target = p_object->m_ext->m_walkSpeed - p_object->m_ext->m_walkSpeed * f;
				goto clamps;
			}

			goto pun;
		}
	}

	target = p_object->m_ext->m_walkSpeed;
	step = p_object->m_ext->m_walkAccel;
	goto clamps;

pun:
	step = *(TonyFloat*) &direction;

clamps:
	if (g_inputManager->m_buttons & 4) {
		p_object->m_state->m_velX =
			(p_object->m_state->m_velX - step <= -target) ? -target : (p_object->m_state->m_velX - step);
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~8) | 4;
		steps = (TonyS32) step;
	}

	if (g_inputManager->m_buttons & 8) {
		p_object->m_state->m_velX =
			(step + p_object->m_state->m_velX < target) ? (step + p_object->m_state->m_velX) : target;
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~4) | 8;
		steps = (TonyS32) step;
	}

framesets:
	if (p_object->m_head->m_facing & 4) {
		if (p_object->m_state->m_frameSet != p_object->m_ext->m_walkSetL) {
			SetFrameSet(p_object, p_object->m_ext->m_walkSetL);
		}
	}

	if (p_object->m_head->m_facing & 8) {
		if (p_object->m_state->m_frameSet != p_object->m_ext->m_walkSetR) {
			SetFrameSet(p_object, p_object->m_ext->m_walkSetR);
		}
	}

	if (((PlayerTemplate::Head*) p_object->m_head)->m_mapNode == 0) {
		if (g_inputManager->m_buttons & 1) {
			SetPlayerState(p_object, 3, 0);
		}
	}

	if (p_object->m_head->m_flags & 0x400) {
		p_object->ApplyGravity((TonyFloat) steps, 0, p_object->m_ext->m_dragX * 0.2, p_object->m_ext->m_dragY * 0.2);
	}
	else {
		p_object->ApplyGravity((TonyFloat) steps, 0, p_object->m_ext->m_dragX, p_object->m_ext->m_dragY);
	}

	if (p_object->m_state->m_velX == 0.0) {
		SetPlayerState(p_object, 1, 0);
	}
}

// Fully implemented, kept as STUB because it compares at 96%: everything matches except
// the single min-clamp, where the original emits fld/fld st(1)/fcompp against the speed
// cap and cl 11.00.7022 emits fcom [mem]. fcompp-min compiler-variant fingerprint (see
// PlayerFall). Re-annotate when the vintage is found.
// STUB: TONY2 0x00403ba0
void __fastcall PlayerJump(GameObject* p_object)
{
	TonyFloat limit;
	TonyFloat dx;
	TonyFloat dy;

	dx = 0.0f;
	dy = 0.0f;
	p_object->m_state->m_animSpeed = 1.0f;

	if (g_inputManager->m_buttons & 4) {
		p_object->m_state->m_velX =
			(p_object->m_state->m_velX - p_object->m_ext->m_walkAccel <= -p_object->m_ext->m_walkSpeed)
				? -p_object->m_ext->m_walkSpeed
				: (p_object->m_state->m_velX - p_object->m_ext->m_walkAccel);
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~8) | 4;
		dx = p_object->m_ext->m_walkAccel;
	}

	if (g_inputManager->m_buttons & 8) {
		p_object->m_state->m_velX =
			(p_object->m_ext->m_walkAccel + p_object->m_state->m_velX >= p_object->m_ext->m_walkSpeed)
				? p_object->m_ext->m_walkSpeed
				: (p_object->m_ext->m_walkAccel + p_object->m_state->m_velX);
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~4) | 8;
		dx = p_object->m_ext->m_walkAccel;
	}

	if (g_inputManager->m_buttons & 1) {
		if (p_object->m_state->m_behavior != 0) {
			limit = -p_object->m_ext->m_jumpSpeed;

			if (p_object->m_state->m_velY > limit) {
				p_object->m_state->m_velY = (p_object->m_state->m_velY - p_object->m_ext->m_jumpAccel > limit)
												? (p_object->m_state->m_velY - p_object->m_ext->m_jumpAccel)
												: limit;
				dy = p_object->m_ext->m_jumpAccel;
			}
		}
	}

	if (p_object->m_state->m_behavior > 0) {
		p_object->m_state->m_behavior--;
	}

	p_object->ApplyGravity(dx, dy, -1234.0f, -1234.0f);

	if (p_object->m_state->m_velY >= 0.0) {
		SetPlayerState(p_object, 4, 0);
	}
}

// FUNCTION: TONY2 0x00403d10
void __fastcall PlayerBounce(GameObject* p_object)
{
	p_object->m_state->m_animSpeed = 1.0f;
	p_object->ApplyGravity(0, 0, -1234.0f, -1234.0f);

	if (p_object->m_state->m_velY >= 0.0) {
		SetPlayerState(p_object, 4, 0);
	}
}

// Fully implemented, kept as STUB because it compares at 84%: the ground snap, flag set,
// frame capture and rest/move transition match; the diffs are the negate spelled
// store+reload by the original where cl 11.00.7022 folds it to fst, and the pushed-zero
// argument taken from the zeroed mode register. Zero-register seeding family (see
// BouncerHitWorld). Re-annotate when solved.
// STUB: TONY2 0x00403d60
TonyFloat __fastcall PlayerHitWorld(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value)
{
	p_value = -p_value;
	MoveObject(p_object, 0, 0.0f, p_value - 0.1);
	p_object->m_state->m_velY = 0.0f;
	p_object->m_head->m_flags |= 8;
	p_object->m_state->m_groundBox = p_frame;

	if (p_object->m_state->m_moveState == 4) {
		SetPlayerState(p_object, (p_object->m_state->m_velX != 0.0) ? 2 : 1, 0);
	}

	return p_value;
}

// FUNCTION: TONY2 0x00403de0
void __fastcall PlayerDie(GameObject* p_object)
{
	p_object->m_state->m_animSpeed = 1.0f;

	if (IsAnimationDone(p_object)) {
		if (g_objectManager->m_stateFlags & 0x40) {
			g_objectManager->m_stateFlags |= 1;
			g_objectManager->m_stateFlags &= ~0x40;
		}
		else {
			g_objectManager->m_stateFlags |= 8;
		}
	}

	p_object->ApplyGravity(0, 0, -1234.0f, -1234.0f);
}

// FUNCTION: TONY2 0x00403e40
void __fastcall PlayerDrown(GameObject* p_object)
{
	p_object->m_state->m_animSpeed = 1.0f;

	if (p_object->m_state->m_worldY > g_camera->m_y - p_object->m_ext->m_jumpSpeed * -70.0f - -400.0f) {
		if (g_objectManager->m_stateFlags & 0x40) {
			g_objectManager->m_stateFlags |= 1;
			g_objectManager->m_stateFlags &= ~0x40;
			p_object->ApplyGravity(0, 0, -1234.0f, -1234.0f);
			return;
		}

		g_objectManager->m_stateFlags |= 8;
	}

	p_object->ApplyGravity(0, 0, -1234.0f, -1234.0f);
}

// FUNCTION: TONY2 0x00403ed0
void __fastcall PlayerCelebrate(GameObject* p_object)
{
	p_object->m_state->m_animSpeed = 1.0f;

	if (IsAnimationDone(p_object)) {
		g_objectManager->m_stateFlags |= 1;
		g_objectManager->m_stateFlags &= ~0x40;
	}
}

// FUNCTION: TONY2 0x00403f10
void __fastcall PlayerDuck(GameObject* p_object)
{
	p_object->m_state->m_animSpeed = 1.0f;

	if (!(g_inputManager->m_buttons & 2)) {
		SetPlayerState(p_object, 1, 0);

		if (p_object->m_head->m_facing & 8) {
			SetFrameSet(p_object, p_object->m_ext->m_riseSetR);
			QueueFrameSet(p_object, p_object->m_ext->m_idleSetR);
		}

		if (p_object->m_head->m_facing & 4) {
			SetFrameSet(p_object, p_object->m_ext->m_riseSetL);
			QueueFrameSet(p_object, p_object->m_ext->m_idleSetL);
		}
	}
	else {
		p_object->m_state->m_cooldown++;

		if (p_object->m_state->m_cooldown == 0x23) {
			g_camera->SetFollowOffset(0, 100.0f);
		}
	}
}

// GLOBAL: TONY2 0x0044c530
static const double g_swimFactor = 0.3;

// FUNCTION: TONY2 0x00403fc0
void __fastcall PlayerSwim(GameObject* p_object)
{
	TonyFloat maxSpeed;
	TonyFloat dyStep;
	TonyFloat dxStep;

	dxStep = 0.0f;
	dyStep = 0.0f;
	maxSpeed = p_object->m_ext->m_walkSpeed * 0.7;
	p_object->m_state->m_animSpeed =
		((fabs(p_object->m_state->m_velX) <= fabs(p_object->m_state->m_velY)) ? fabs(p_object->m_state->m_velY)
																			  : fabs(p_object->m_state->m_velX)) /
		fabs(maxSpeed);

	if (g_inputManager->m_buttons & 4) {
		dxStep = p_object->m_ext->m_walkAccel * 0.2;
		p_object->m_state->m_velX =
			(p_object->m_state->m_velX - dxStep <= -maxSpeed) ? -maxSpeed : (p_object->m_state->m_velX - dxStep);
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~8) | 4;
	}

	if (g_inputManager->m_buttons & 8) {
		dxStep = p_object->m_ext->m_walkAccel * 0.2;
		p_object->m_state->m_velX =
			(dxStep + p_object->m_state->m_velX < maxSpeed) ? (dxStep + p_object->m_state->m_velX) : maxSpeed;
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~4) | 8;
	}

	if (g_inputManager->m_buttons & 1) {
		dyStep = p_object->m_ext->m_jumpAccel * 0.2;
		p_object->m_state->m_velY =
			(p_object->m_state->m_velY - dyStep <= -maxSpeed) ? -maxSpeed : (p_object->m_state->m_velY - dyStep);
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~2) | 1;
	}

	if (g_inputManager->m_buttons & 2) {
		dyStep = p_object->m_ext->m_jumpAccel * 0.2;
		p_object->m_state->m_velY =
			(dyStep + p_object->m_state->m_velY < maxSpeed) ? (dyStep + p_object->m_state->m_velY) : maxSpeed;
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~1) | 2;
	}

	if (!(p_object->m_head->m_flags & 4)) {
		p_object->m_state->m_velY = (p_object->m_state->m_velY + g_objectManager->m_gravity * g_swimFactor >= maxSpeed)
										? maxSpeed
										: (p_object->m_state->m_velY + g_objectManager->m_gravity * g_swimFactor);
	}

	p_object
		->Decelerate(dxStep, dyStep, p_object->m_ext->m_dragX * g_swimFactor, p_object->m_ext->m_dragY * g_swimFactor);

	if (p_object->m_head->m_facing & 4) {
		if (p_object->m_state->m_frameSet != p_object->m_ext->m_specialSetL) {
			SetFrameSet(p_object, p_object->m_ext->m_specialSetL);
		}
	}

	if (p_object->m_head->m_facing & 8) {
		if (p_object->m_state->m_frameSet != p_object->m_ext->m_specialSetR) {
			SetFrameSet(p_object, p_object->m_ext->m_specialSetR);
		}
	}
}

// FUNCTION: TONY2 0x00404210
void __fastcall PlayerSurf(GameObject* p_object)
{
	TonyFloat step;
	TonyFloat target;

	step = 0.0f;
	target = p_object->m_ext->m_walkSpeed;
	p_object->m_state->m_animSpeed = 1.0f;

	if (g_inputManager->m_buttons & 4) {
		step = p_object->m_ext->m_walkAccel;
		p_object->m_state->m_velX =
			(p_object->m_state->m_velX - step <= -target) ? -target : (p_object->m_state->m_velX - step);
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~8) | 4;
	}

	if (g_inputManager->m_buttons & 8) {
		step = p_object->m_ext->m_walkAccel;
		p_object->m_state->m_velX =
			(step + p_object->m_state->m_velX < target) ? (step + p_object->m_state->m_velX) : target;
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~4) | 8;
	}

	p_object->Decelerate(step, 0, 0, 0);

	if (p_object->m_head->m_facing & 4) {
		if (p_object->m_state->m_frameSet != p_object->m_ext->m_specialSetL) {
			SetFrameSet(p_object, p_object->m_ext->m_specialSetL);
		}
	}

	if (p_object->m_head->m_facing & 8) {
		if (p_object->m_state->m_frameSet != p_object->m_ext->m_specialSetR) {
			SetFrameSet(p_object, p_object->m_ext->m_specialSetR);
		}
	}

	if (g_inputManager->m_buttons & 1) {
		SetPlayerState(p_object, 3, 0);
	}
}

// FUNCTION: TONY2 0x00404340
Camera* __fastcall GetObjectCamera(GameObject* p_object)
{
	Camera* camera;

	p_object->Teleport(p_object->m_state->m_savedX, p_object->m_state->m_savedY);
	g_objectManager->m_spawnPoint = p_object->m_state->m_savedSpawn;
	camera = p_object->m_state->m_savedCamera;
	p_object->m_state->m_savedCamera = NULL;
	return camera;
}

// FUNCTION: TONY2 0x00404390
void __fastcall StoreObjectCamera(GameObject* p_object, Camera* p_camera)
{
	p_object->m_state->m_savedRound = p_camera->m_round;
	p_object->m_state->m_savedCamera = p_camera;
	p_object->m_state->m_savedX = p_object->m_head->m_x;
	p_object->m_state->m_savedY = p_object->m_head->m_y;
	p_object->m_state->m_savedSpawn = g_objectManager->m_spawnPoint;
	g_objectManager->m_spawnPoint = 0;
}

// Fully implemented, kept as STUB because it compares at 79%: the sound trigger, the
// slope dispatch (goto layout proven against the binary CFG), both clamp blocks, the
// dismount tail and the duplicated mover calls all match; the residue is the
// slot-direction family (slope/steps and f/direction land in swapped stack slots under
// cl 11.00.7022 regardless of declaration order) plus one zero-seeded 0.0f push.
// See JoystickEnumCallback (0x405430). Re-annotate when the vintage is found.
// STUB: TONY2 0x004043f0
void __fastcall PlayerHang(GameObject* p_object)
{
	TonyFloat target;
	TonyFloat step;
	TonyS32 steps;
	TonyFloat slope;
	TonyS32 direction;
	double f;

	slope = 0.0f;
	steps = 0;

	if ((p_object->m_state->m_frame == 0 && p_object->m_state->m_prevFrame != 0) ||
		(p_object->m_state->m_frame == 8 && p_object->m_state->m_prevFrame != 8)) {
		PlayObjectSound(p_object, 0x15, -1, -1);
	}

	if (p_object->m_head->m_flags & 8) {
		GetGroundSlope(p_object->m_state->m_groundBox, &slope, &direction);

		if (fabs(slope) > 0.01) {
			if ((g_inputManager->m_buttons & 4 && direction == 8) ||
				(g_inputManager->m_buttons & 8 && direction == 4)) {
				target = p_object->m_ext->m_walkSpeed;
				step = p_object->m_ext->m_walkAccel;

				if (g_inputManager->m_buttons & 4) {
					p_object->m_state->m_velX =
						(p_object->m_state->m_velX - step <= -target) ? -target : (p_object->m_state->m_velX - step);
					p_object->m_head->m_facing = (p_object->m_head->m_facing & ~8) | 4;
					steps = (TonyS32) step;
				}

				if (g_inputManager->m_buttons & 8) {
					p_object->m_state->m_velX =
						(step + p_object->m_state->m_velX < target) ? (step + p_object->m_state->m_velX) : target;
					p_object->m_head->m_facing = (p_object->m_head->m_facing & ~4) | 8;
					steps = (TonyS32) step;
				}

				p_object->m_state->m_velY += fabs(slope) * fabs(p_object->m_state->m_velX);
				goto framesets;
			}

			if (slope > -1.0) {
				f = fabs(slope * 0.1);
				step = p_object->m_ext->m_walkAccel - f * p_object->m_ext->m_walkAccel;
				target = p_object->m_ext->m_walkSpeed - p_object->m_ext->m_walkSpeed * f;
				goto clamps;
			}

			goto pun;
		}
	}

	target = p_object->m_ext->m_walkSpeed;
	step = p_object->m_ext->m_walkAccel;
	goto clamps;

pun:
	step = *(TonyFloat*) &direction;

clamps:
	if (g_inputManager->m_buttons & 4) {
		p_object->m_state->m_velX =
			(p_object->m_state->m_velX - step <= -target) ? -target : (p_object->m_state->m_velX - step);
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~8) | 4;
		steps = (TonyS32) step;
	}

	if (g_inputManager->m_buttons & 8) {
		p_object->m_state->m_velX =
			(step + p_object->m_state->m_velX < target) ? (step + p_object->m_state->m_velX) : target;
		p_object->m_head->m_facing = (p_object->m_head->m_facing & ~4) | 8;
		steps = (TonyS32) step;
	}

framesets:
	if (p_object->m_head->m_facing & 4) {
		if (p_object->m_state->m_frameSet != p_object->m_ext->m_specialSetL) {
			SetFrameSet(p_object, p_object->m_ext->m_specialSetL);
		}
	}

	if (p_object->m_head->m_facing & 8) {
		if (p_object->m_state->m_frameSet != p_object->m_ext->m_specialSetR) {
			SetFrameSet(p_object, p_object->m_ext->m_specialSetR);
		}
	}

	if (((PlayerTemplate::Head*) p_object->m_head)->m_mapNode == 0) {
		if (g_inputManager->m_buttons & 1) {
			SetPlayerState(p_object, 3, 0);
		}

		if (g_inputManager->m_buttons & 2) {
			MoveObject(p_object, 0, 0.0f, 5.0f);
			p_object->m_state->m_dropTicks = 3;
			SetPlayerState(p_object, 4, 0);
		}
	}

	if (p_object->m_head->m_flags & 0x400) {
		p_object->ApplyGravity((TonyFloat) steps, 0, p_object->m_ext->m_dragX * 0.2, p_object->m_ext->m_dragY * 0.2);
	}
	else {
		p_object->ApplyGravity((TonyFloat) steps, 0, p_object->m_ext->m_dragX, p_object->m_ext->m_dragY);
	}
}

// FUNCTION: TONY2 0x00404750
void __fastcall PlayerRequestState(GameObject* p_object, TonyS32 p_state)
{
	p_object->m_state->m_pendingState = p_state;
}

// GLOBAL: TONY2 0x0044c544
static const TonyFloat g_zeroResponse = 0.0f;

// FUNCTION: TONY2 0x00404760
void __fastcall DropItemInit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	p_object->m_tickFn = DropItemTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = NULL;
	DropItemReset(p_object);
}

// FUNCTION: TONY2 0x00404790
void __fastcall DropItemReset(GameObject* p_object)
{
	ResetObjectAnimation(p_object);
	p_object->m_state->m_typeFlags = 0;
}

// GLOBAL: TONY2 0x0044c538
static const TonyFloat g_dropGravityScale = 0.5f;

// FUNCTION: TONY2 0x004047b0
TonyS32 __fastcall DropItemTick(GameObject* p_object)
{
	CollectibleTick(p_object);

	if (p_object->m_state->m_typeFlags == 0) {
		p_object->m_state->m_velY = (p_object->m_state->m_velY + g_objectManager->m_gravity * g_dropGravityScale < 8.0f)
										? (p_object->m_state->m_velY + g_objectManager->m_gravity * g_dropGravityScale)
										: 8.0f;
	}

	p_object->m_state->m_velX = p_object->m_state->m_velX * 0.9f;
	CollideWithMapBoxes(p_object, DropItemHitFrame);
	CollideWithGroundLines(p_object, DropItemHitWorld, 0, 0);
	return 0;
}

// FUNCTION: TONY2 0x00404820
TonyFloat __fastcall DropItemHitFrame(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value)
{
	if (p_object->m_head->m_facing == 4) {
		p_object->m_head->m_facing = 8;
		return g_zeroResponse;
	}

	if (p_object->m_head->m_facing == 8) {
		p_object->m_head->m_facing = 4;
	}

	return g_zeroResponse;
}

// FUNCTION: TONY2 0x00404850
TonyFloat __fastcall DropItemHitWorld(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value)
{
	if (p_object->m_state->m_velY > 2.0) {
		p_object->m_state->m_velY = -p_object->m_state->m_velY * 0.5;
		return g_zeroResponse;
	}

	TonyFloat result = g_zeroResponse;
	p_object->m_state->m_velY = 0.0f;
	p_object->m_state->m_typeFlags = 1;
	return result;
}

// FUNCTION: TONY2 0x004048a0
void __fastcall SpringInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = SpringTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = NULL;
	ResetObjectAnimation(p_object);
}

// FUNCTION: TONY2 0x004048d0
void __fastcall ResetObjectAnimation(GameObject* p_object)
{
	p_object->ResetAnimation();
}

// FUNCTION: TONY2 0x004048e0
TonyS32 __fastcall SpringTick(GameObject* p_object)
{
	SpriteTick(p_object);
	CollideWithHitList(
		p_object,
		(HitResult*) g_objectManager->m_playerFeetHits,
		g_objectManager->m_playerFeetHitCount,
		SpringTouch
	);
	return 0;
}

// Fully implemented, kept as STUB because it compares at 79%: everything matches
// except the two frame pointers load into swapped registers (original edx/ecx,
// SP3 ecx/edx) which shifts every use. Register round-robin phase family (same
// as GameFile::Seek); retest with the original compiler vintage.
// STUB: TONY2 0x00404910
void __fastcall SpringTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_extra
)
{
	TonyFloat anchorX;
	TonyFloat anchorY;

	GetPrevPosition(p_other, &anchorX, &anchorY);

	if (p_otherFrame->m_bottom - p_other->m_state->m_worldY + anchorY < p_own->m_top &&
		p_otherFrame->m_bottom >= p_own->m_top && SetPlayerState(p_other, 3, 1) == 1) {
		PlayObjectSound(p_object, 0x10, -1, -1);
		p_other->m_state->m_velY = -(p_object->m_ext->m_walkSpeed * p_other->m_ext->m_jumpAccel);
	}
}

// FUNCTION: TONY2 0x004049a0
void __fastcall CritterInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = CritterTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = NULL;
	CritterReset(p_object);
}

// FUNCTION: TONY2 0x004049d0
void __fastcall CritterReinit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	CritterResolveSets(p_object);
}

// FUNCTION: TONY2 0x004049f0
void __fastcall CritterResolveSets(GameObject* p_object)
{
	p_object->ResolveFrameSet();
	*(TonyS32*) &p_object->m_ext->m_walkSpeed =
		g_videoManager->GetFrameSet(*(TonyS32*) &p_object->m_ext->m_walkSpeed, 0);
}

// FUNCTION: TONY2 0x00404a20
TonyS32 __fastcall CritterTick(GameObject* p_object)
{
	if (p_object->m_state->m_typeFlags & 1) {
		p_object->Translate((TonyFloat) (rand() % 7 - 3), (TonyFloat) (rand() % 7 - 3));
		SpriteTick(p_object);
		return 0;
	}

	TonyFloat dx = g_objectManager->m_player->m_state->m_worldX - p_object->m_state->m_worldX;
	TonyFloat dy = g_objectManager->m_player->m_state->m_worldY - p_object->m_state->m_worldY;

	if (dy * dy + dx * dx < 40000.0f) {
		p_object->m_state->m_typeFlags |= 1;
		SetFrameSet(p_object, *(TonyS32*) &p_object->m_ext->m_walkSpeed);
		p_object->m_state->m_velX = (TonyFloat) (rand() % 7 - 3);
		p_object->m_state->m_velY = (TonyFloat) - (rand() % 3);
	}

	SpriteTick(p_object);
	return 0;
}

// FUNCTION: TONY2 0x00404b20
void __fastcall CritterReset(GameObject* p_object)
{
	p_object->ResetAnimation();
	p_object->m_state->m_typeFlags = 0;
}

// FUNCTION: TONY2 0x00404b40
void __fastcall CharSelectInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = CharSelectTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = NULL;
	p_object->ResetAnimation();
}

// FUNCTION: TONY2 0x00404b70
TonyS32 __fastcall CharSelectTick(GameObject* p_object)
{
	SpriteTick(p_object);
	CollideWithHitList(
		p_object,
		(HitResult*) g_objectManager->m_playerHits,
		g_objectManager->m_playerHitCount,
		CharSelectTouch
	);
	return 0;
}

// FUNCTION: TONY2 0x00404ba0
void __fastcall CharSelectTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_extra
)
{
	if (g_inputManager->m_buttons & 2) {
		PlayObjectSound(p_object, 0x19, -1, -1);
		ShowCharacterSelect(g_objectManager->m_player);
	}
}

// FUNCTION: TONY2 0x00404bd0
void __fastcall SpeechInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = SpeechTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = SpeechDestroy;
	SpeechReset(p_object);
}

// FUNCTION: TONY2 0x00404c00
void __fastcall SpeechReinit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	SpeechResolveSets(p_object);
}

// FUNCTION: TONY2 0x00404c20
void __fastcall SpeechResolveSets(GameObject* p_object)
{
	if (g_language >= 0 && g_language <= 0) {
		p_object->m_ext->m_idleSetR = 0x63;
		((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param1 = g_videoManager->GetFrameSet(0x5f, 0);
		((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param2 = g_videoManager->GetFrameSet(0x5d, 0);
		((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param3 = g_videoManager->GetFrameSet(0x62, 0);
	}

	if (g_language >= 1 && g_language <= 4) {
		p_object->m_ext->m_idleSetR = 0xb5;
		((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param1 = g_videoManager->GetFrameSet(0xb6, 0);
		((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param2 = g_videoManager->GetFrameSet(0xb8, 0);
		((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param3 = g_videoManager->GetFrameSet(0xb7, 0);
	}

	if (g_language >= 5 && g_language <= 9) {
		p_object->m_ext->m_idleSetR = 0x17;
		((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param1 = g_videoManager->GetFrameSet(0x15, 0);
		((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param2 = g_videoManager->GetFrameSet(0x14, 0);
		((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param3 = g_videoManager->GetFrameSet(0x16, 0);
	}

	p_object->ResolveFrameSet();
}

// FUNCTION: TONY2 0x00404d40
void __fastcall SpeechReset(GameObject* p_object)
{
	TextLabel banner;
	char buf[0x100];
	TonyS32 i;
	TonyS32 sprite;

	g_soundManager->SetSongVolume(0x32);
	p_object->m_state->m_meterSet1 = g_objectManager->m_drawMode;
	g_objectManager->m_drawMode = 0;
	p_object->ResetAnimation();
	p_object->m_state->m_typeFlags = (TonyS32) malloc(0x1f4);
	((OverlayData*) p_object->m_state->m_typeFlags)->m_type = 8;
	((OverlayData*) p_object->m_state->m_typeFlags)->m_x = -147.0f;
	((OverlayData*) p_object->m_state->m_typeFlags)->m_y = -188.0f;
	((OverlayData*) p_object->m_state->m_typeFlags)->m_flags = 0;
	((OverlayData*) p_object->m_state->m_typeFlags)->m_facing = 0;
	((OverlayData*) p_object->m_state->m_typeFlags)->m_layer = 0xfa;
	((OverlayData*) p_object->m_state->m_typeFlags)->m_flags = 0;
	((OverlayData*) p_object->m_state->m_typeFlags)->m_arg3 = 0xaab;
	((OverlayData*) p_object->m_state->m_typeFlags)->m_arg0 = 1;
	((OverlayData*) p_object->m_state->m_typeFlags)->SpawnOnce();
	p_object->m_state->m_behavior = (TonyS32) g_objectManager->AllocObject();
	InitObjectFromData((GameObject*) p_object->m_state->m_behavior, (OverlayData*) p_object->m_state->m_typeFlags);
	g_objectManager->InsertObject((GameObject*) p_object->m_state->m_behavior, 0xe);
	((GameObject*) p_object->m_state->m_behavior)->m_state->m_parent = p_object;
	SnapshotPosition((GameObject*) p_object->m_state->m_behavior);
	UpdateWorldPosition((GameObject*) p_object->m_state->m_behavior);
	p_object->m_state->m_mouthStep = 0;

	for (i = 0; i < 9; i++) {
		(&p_object->m_state->m_pendingState)[i] = p_object->m_ext->m_idleSetR;
	}

	sprite = ((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param2;
	p_object->m_state->m_pendingState = sprite;
	SetFrameSet(p_object, sprite);
	g_camera->LockScroll();
	g_videoManager->CopyFrontToBack();
	// STRING: TONY2 0x00455080
	sprintf(buf, "sound\\language%d.mdir", g_language);
	g_soundManager->LoadSpeechBank(buf);
	g_soundManager->PlaySpeech(((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0 + 1);
}

// FUNCTION: TONY2 0x00404f80
TonyS32 __fastcall SpeechTick(GameObject* p_object)
{
	SpriteTick(p_object);

	if (IsAnimationDone(p_object)) {
		p_object->m_state->m_mouthStep++;

		if (p_object->m_state->m_mouthStep == 9) {
			p_object->m_state->m_mouthStep = 1;
		}

		QueueFrameSet(p_object, (&p_object->m_state->m_pendingState)[p_object->m_state->m_mouthStep]);
	}

	if (g_soundManager->IsSpeechPlaying() == 0) {
		g_soundManager->UnloadSpeechBank();
		p_object->m_state->m_tickStatus = -1;
	}

	if (g_inputManager->m_buttons & 0x70) {
		g_soundManager->StopSpeech();
	}

	return 0;
}

// FUNCTION: TONY2 0x00405020
void __fastcall SpeechDestroy(GameObject* p_object)
{
	g_objectManager->m_drawMode = p_object->m_state->m_meterSet1;
	g_soundManager->SetSongVolume(0x64);
	g_objectManager->FreeObject((GameObject*) p_object->m_state->m_behavior);
	free((void*) p_object->m_state->m_typeFlags);
	g_objectManager->FreeTemplate(p_object->m_state->m_template);
	g_objectManager->ResumeGameplay();
	g_camera->UnlockScroll();
}

// FUNCTION: TONY2 0x004050a0
void __fastcall SpeechTriggerInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = SpeechTriggerTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = NULL;
	SpeechTriggerReset(p_object);
}

// FUNCTION: TONY2 0x004050d0
void __fastcall SpeechTriggerReset(GameObject* p_object)
{
	p_object->ResetAnimation();
	p_object->m_state->m_typeFlags = 0x46;
}

// FUNCTION: TONY2 0x004050f0
TonyS32 __fastcall SpeechTriggerTick(GameObject* p_object)
{
	SpriteTick(p_object);
	p_object->m_state->m_typeFlags++;
	CollideWithHitList(
		p_object,
		(HitResult*) g_objectManager->m_playerHits,
		g_objectManager->m_playerHitCount,
		SpeechTriggerTouch
	);
	return 0;
}

// FUNCTION: TONY2 0x00405130
void __fastcall SpeechTriggerTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_extra
)
{
	OverlayData* block;
	GameObject* object;

	if (p_object->m_state->m_typeFlags >= 0x46) {
		g_objectManager->SuspendGameplay();
		p_object->m_state->m_typeFlags = 0;
		block = (OverlayData*) malloc(0x1f4);
		block->m_type = 0x67;
		block->m_x = 320.0f;
		block->m_y = 300.0f;
		block->m_facing = 0;
		block->m_layer = 0xfb;
		block->m_flags = 0;
		block->m_arg4 = ((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0;
		block->m_arg0 = 1;
		block->SpawnOnce();
		object = g_objectManager->AllocObject();
		InitObjectFromData(object, block);
		g_objectManager->InsertObject(object, 8);
	}
}

// FUNCTION: TONY2 0x00405650
void __fastcall DoorInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = DoorTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = NULL;
	DoorReset(p_object);
}

// FUNCTION: TONY2 0x00405680
void __fastcall DoorReset(GameObject* p_object)
{
	p_object->ResetAnimation();
	p_object->m_state->m_animSpeed = 0.0f;
}

// FUNCTION: TONY2 0x004056a0
TonyS32 __fastcall DoorTick(GameObject* p_object)
{
	SpriteTick(p_object);
	CollideWithHitList(
		p_object,
		(HitResult*) g_objectManager->m_playerHits,
		g_objectManager->m_playerHitCount,
		DoorTouch
	);

	if (IsAnimationDone(p_object)) {
		p_object->m_state->m_animSpeed = 0.0f;

		if (((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0 == 0x63) {
			((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0 = 0;
			g_objectManager->m_stateFlags = 0x41;
		}
	}

	return 0;
}

// FUNCTION: TONY2 0x00405700
void __fastcall DoorTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_extra
)
{
	TonyFloat x;
	TonyFloat y;
	TonyS32 count;
	HitBox frame;
	HitResult hits[4];
	GameObject::State* otherState;

	count = 0;
	otherState = p_other->m_state;

	if (((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0 == 0x63) {
		if (p_object->m_state->m_animSpeed == 0.0) {
			PlayObjectSound(p_object, 9, -1, -1);
			PlayObjectSound(p_object, 0x1a, -1, -1);
		}

		p_object->m_state->m_animSpeed = 1.0f;
		ObjectSetFlags(g_objectManager->m_player, 0x1000, 0);
	}

	if (((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0 > 0 &&
		((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0 < 0xa) {
		if ((1 << (((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0 - 1)) &
			GetSegmentMask((GameObject*) otherState->m_nutrientGauge)) {
			if (p_object->m_state->m_animSpeed == 0.0 && p_object->m_state->m_frame == 0) {
				PlayObjectSound(p_object, 9, -1, -1);
			}

			p_object->m_state->m_animSpeed = 1.0f;
		}
		else {
			GetPrevPosition(p_other, &x, &y);
			CollectFrameHitBoxes(
				p_other,
				p_other->m_state->m_prevFrameSet,
				p_other->m_state->m_prevFrame,
				hits,
				&count,
				0xfffc
			);
			frame.m_left = hits[0].m_left + x;
			frame.m_top = hits[0].m_top + y;
			frame.m_right = hits[0].m_right + x;
			frame.m_bottom = hits[0].m_bottom + y;
			ResolveBoxContact(p_other, &frame, p_otherFrame, p_own, 0);
		}
	}
}

// FUNCTION: TONY2 0x00405890
void __fastcall GoalInit(GameObject* p_object, ObjectTemplate* p_template)
{
	BindTemplate(p_object, p_template);
	p_object->m_tickFn = GoalTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = NULL;
	p_object->ResetAnimation();
}

// FUNCTION: TONY2 0x004058c0
TonyS32 __fastcall GoalTick(GameObject* p_object)
{
	SpriteTick(p_object);
	CollideWithHitList(
		p_object,
		(HitResult*) g_objectManager->m_playerHits,
		g_objectManager->m_playerHitCount,
		GoalTouch
	);
	return 0;
}

// FUNCTION: TONY2 0x004058f0
void __fastcall GoalTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_extra
)
{
	if (g_objectManager->m_player->m_state->m_moveState != 7) {
		PlayerRequestState(g_objectManager->m_player, 7);
		ObjectSetFlags(g_objectManager->m_player, 0x1000, 0);
	}
}

// FUNCTION: TONY2 0x00405cc0
void __fastcall BossInit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	p_object->m_tickFn = BossTick;
	p_object->m_drawFn = SpriteDraw;
	p_object->m_destroyFn = BossDestroy;
	BossReset(p_object);
}

// FUNCTION: TONY2 0x00405cf0
void __fastcall BossReset(GameObject* p_object)
{
	EnemyReset(p_object);
	BossRestartScript(p_object);
	p_object->m_state->m_typeFlags |= 1;
	ObjectSetFlags(p_object, 0, 2);
	p_object->m_state->m_cereals = 0;
	p_object->m_state->m_touchFn = NULL;

	if (p_object->m_head->m_facing & 4) {
		SetFrameSet(p_object, p_object->m_ext->m_walkSetR);
		return;
	}

	if (p_object->m_head->m_facing & 8) {
		SetFrameSet(p_object, p_object->m_ext->m_idleSetR);
	}
}

// FUNCTION: TONY2 0x00405d60
void __fastcall BossReinit(GameObject* p_object, CounterTemplate* p_template)
{
	BindCounterTemplate(p_object, p_template);
	BossResolveSets(p_object);
}

// FUNCTION: TONY2 0x00405d80
void __fastcall BossResolveSets(GameObject* p_object)
{
	EnemyResolveBaseSets(p_object);
	p_object->m_ext->m_hurtSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_fallSetL, 2);
	p_object->m_ext->m_fallSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_fallSetL, 0);
	p_object->m_ext->m_fallSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_jumpSetL, 2);
	p_object->m_ext->m_jumpSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_jumpSetL, 0);
	p_object->m_ext->m_duckSetR = g_videoManager->GetFrameSet(p_object->m_ext->m_hurtSetL, 2);
	p_object->m_ext->m_hurtSetL = g_videoManager->GetFrameSet(p_object->m_ext->m_hurtSetL, 0);
}

// FUNCTION: TONY2 0x00405e30
void __fastcall BossDestroy(GameObject* p_object)
{
	if (p_object->m_state->m_cereals != 0) {
		free((void*) p_object->m_state->m_cereals);
	}

	if (p_object->m_state->m_behavior != 5) {
		ObjectSetMoveFlags(p_object, 0, 1);
	}
}

// Fully implemented, kept as STUB because it compares at 81%: the pending-state hook,
// state dispatch and boss handoff match, but cases 0 and 2 both call BossStand and
// cl 11.00.7022 cross-jumps the two identical case bodies into one where the original
// keeps both copies. Tail-merge variance family. Re-annotate when the vintage is found.
// STUB: TONY2 0x00405e70
TonyS32 __fastcall BossTick(GameObject* p_object)
{
	if (p_object->m_state->m_pendingState == 1) {
		BossRunScript(p_object);
	}

	switch (p_object->m_state->m_behavior) {
	case 1:
		BossWalk(p_object);
		break;
	case 5:
		BossDie(p_object);
		break;
	case 2:
		BossStand(p_object);
		break;
	case 0:
		BossStand(p_object);
		break;
	case 7:
		BossCharge(p_object);
		break;
	}

	if (p_object->m_state->m_behavior == 5 && p_object->m_ext->m_smacksIdleR == 1) {
		PlayerRequestState(g_objectManager->m_player, 7);
	}

	return 0;
}

// FUNCTION: TONY2 0x00405f20
void __fastcall BossWalk(GameObject* p_object)
{
	if (p_object->m_state->m_frame == 0) {
		PlayObjectSound(p_object, 5, -1, -1);
	}

	if (p_object->m_head->m_facing & 4) {
		p_object->m_state->m_velX = -p_object->m_ext->m_walkSpeed;
	}

	if (p_object->m_head->m_facing & 8) {
		p_object->m_state->m_velX = p_object->m_ext->m_walkSpeed;
	}

	if (IsAnimationDone(p_object)) {
		p_object->m_state->m_pendingState = 1;
	}

	EnemyBaseTick(p_object);
}

// FUNCTION: TONY2 0x00405f90
void __fastcall BossDie(GameObject* p_object)
{
	p_object->m_state->m_velX = 0.0f;

	if (p_object->m_state->m_cereals == 0 && p_object->m_ext->m_smacksIdleR == 0) {
		p_object->m_state->m_cereals = (TonyS32) malloc(0x1f4);
		((OverlayData*) p_object->m_state->m_cereals)->m_type = 0x19;
		((OverlayData*) p_object->m_state->m_cereals)->m_x = p_object->m_state->m_worldX;
		((OverlayData*) p_object->m_state->m_cereals)->m_y = p_object->m_state->m_worldY;
		((OverlayData*) p_object->m_state->m_cereals)->m_flags = 0;
		((OverlayData*) p_object->m_state->m_cereals)->m_facing = 0;
		((OverlayData*) p_object->m_state->m_cereals)->m_layer = p_object->m_head->m_layer + 1;
		((OverlayData*) p_object->m_state->m_cereals)->m_flags = 0;
		((OverlayData*) p_object->m_state->m_cereals)->m_arg0 = 2;
		((OverlayData*) p_object->m_state->m_cereals)->m_arg1 = 3;
		((OverlayData*) p_object->m_state->m_cereals)->m_arg5 = 0x57;
		((OverlayData*) p_object->m_state->m_cereals)->m_arg2 = 0;
		((OverlayData*) p_object->m_state->m_cereals)->SpawnOnce();
		*(GameObject**) &p_object->m_state->m_touchFn = g_objectManager->AllocObject();
		InitObjectFromData(*(GameObject**) &p_object->m_state->m_touchFn, (OverlayData*) p_object->m_state->m_cereals);
		g_objectManager->InsertObject(*(GameObject**) &p_object->m_state->m_touchFn, 8);
		(*(GameObject**) &p_object->m_state->m_touchFn)->m_state->m_velY = -16.0f;
		(*(GameObject**) &p_object->m_state->m_touchFn)->m_state->m_velX = 0.0f;
		p_object->m_state->m_velY = 0.0f;
	}

	if (IsAnimationDone(p_object) && p_object->m_state->m_frameSet == p_object->m_ext->m_jumpSetR) {
		SetFrameSet(p_object, p_object->m_ext->m_duckSetR);
	}

	if (IsAnimationDone(p_object) && p_object->m_state->m_frameSet == p_object->m_ext->m_walkSetL) {
		SetFrameSet(p_object, p_object->m_ext->m_hurtSetL);
	}

	EnemyBaseTick(p_object);
}

// FUNCTION: TONY2 0x00406140
void __fastcall SetBossState(GameObject* p_object, TonyS32 p_state)
{
	switch (p_state) {
	case 1:
		if (p_object->m_head->m_facing == 4) {
			SetFrameSet(p_object, p_object->m_ext->m_walkSetR);
		}

		if (p_object->m_head->m_facing == 8) {
			SetFrameSet(p_object, p_object->m_ext->m_idleSetR);
		}
		break;
	case 0:
		if (p_object->m_head->m_facing == 4) {
			SetFrameSet(p_object, p_object->m_ext->m_fallSetR);
		}

		if (p_object->m_head->m_facing == 8) {
			SetFrameSet(p_object, p_object->m_ext->m_jumpSetL);
		}
		break;
	case 2:
	case 7:
		if (p_object->m_head->m_facing == 4) {
			SetFrameSet(p_object, p_object->m_ext->m_hurtSetR);
		}

		if (p_object->m_head->m_facing == 8) {
			SetFrameSet(p_object, p_object->m_ext->m_fallSetL);
		}
		break;
	}

	p_object->m_state->m_behavior = p_state;
}

// Patrol script interpreter: two-character commands (direction suffixed L/R) read from
// the template behind the sprite table; "<-" rewinds.
// Fully implemented, kept as STUB because it compares at 72%: the 16-bit command fetch,
// the nine-case dispatch tree and every body match, but the scratch register chosen for
// the head-pointer loads is rotated by one position through all case bodies (the original
// starts the rotation at edx in the rewind arm, cl 11.00.7022 at eax) and the early
// return merges with the shared tail. Register-seeding family (see TickAll).
// Re-annotate when the vintage is found.
// STUB: TONY2 0x00406220
void __fastcall BossRunScript(GameObject* p_object)
{
	TonyU16 cmd;
	TonyS16 lo;
	TonyS16 hi;

	lo = ((TonyS8*) p_object->m_ext)[p_object->m_state->m_patrolStep * 2 + 0x49];
	hi = ((TonyS8*) p_object->m_ext)[p_object->m_state->m_patrolStep * 2 + 0x48];
	cmd = (hi << 8) + lo;
	p_object->m_state->m_patrolStep++;

	switch (cmd) {
	case 0x3c2d:
		BossRestartScript(p_object);
		p_object->m_state->m_pendingState = 0;
		return;
	case 0x414c:
		p_object->m_head->m_facing = 4;
		SetBossState(p_object, 2);
		break;
	case 0x4152:
		p_object->m_head->m_facing = 8;
		SetBossState(p_object, 2);
		break;
	case 0x504c:
		p_object->m_head->m_facing = 4;
		SetBossState(p_object, 0);
		break;
	case 0x5052:
		p_object->m_head->m_facing = 8;
		SetBossState(p_object, 0);
		break;
	case 0x514c:
		p_object->m_head->m_facing = 4;
		SetBossState(p_object, 7);
		break;
	case 0x5752:
		p_object->m_head->m_facing = 8;
		SetBossState(p_object, 1);
		break;
	case 0x574c:
		p_object->m_head->m_facing = 4;
		SetBossState(p_object, 1);
		break;
	case 0x5152:
		p_object->m_head->m_facing = 8;
		SetBossState(p_object, 7);
		break;
	}

	p_object->m_state->m_pendingState = 0;
}

// FUNCTION: TONY2 0x00406340
void __fastcall BossRestartScript(GameObject* p_object)
{
	p_object->m_state->m_patrolStep = 0;
	BossRunScript(p_object);
}

// FUNCTION: TONY2 0x00406360
void __fastcall BossStand(GameObject* p_object)
{
	p_object->m_state->m_velX = 0.0f;
	EnemyBaseTick(p_object);

	if (IsAnimationDone(p_object)) {
		p_object->m_state->m_pendingState = 1;
	}
}

// FUNCTION: TONY2 0x00406390
void __fastcall BossCharge(GameObject* p_object)
{
	if (p_object->m_state->m_frame == 0) {
		PlayObjectSound(p_object, 5, -1, -1);
	}

	if (p_object->m_head->m_facing & 4) {
		p_object->m_state->m_velX = p_object->m_ext->m_walkSpeed * -2.0f;
	}

	if (p_object->m_head->m_facing & 8) {
		p_object->m_state->m_velX = p_object->m_ext->m_walkSpeed * 2.0f;
	}

	if (IsAnimationDone(p_object)) {
		p_object->m_state->m_pendingState = 1;
	}

	EnemyBaseTick(p_object);
}
