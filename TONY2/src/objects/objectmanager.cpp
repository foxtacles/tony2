#include "objectmanager.h"

#include "backgroundrenderer.h"
#include "camera.h"
#include "dialogbuilder.h"
#include "engine.h"
#include "gamemanager.h"
#include "inputmanager.h"
#include "soundmanager.h"
#include "videomanager.h"

#include <stdlib.h>

// GLOBAL: TONY2 0x0044c580
static const TonyFloat g_playerSpeedWalk = 8.0f;

// GLOBAL: TONY2 0x0044c584
static const TonyFloat g_playerSpeedRun = 18.0f;

// FUNCTION: TONY2 0x0040b250
void __fastcall PlayerSetForm(GameObject* p_object, TonyS32 p_form, TonyS32 p_force)
{
	if (p_form == ((CounterTemplate::Head*) p_object->m_head)->m_value && p_force != 1) {
		return;
	}

	((CounterTemplate::Head*) p_object->m_head)->m_value = p_form;

	switch (p_form) {
	case 1:
		p_object->m_ext->m_walkSpeed = g_playerSpeedWalk;
		p_object->m_ext->m_jumpSpeed = g_playerSpeedRun;
		p_object->m_ext->m_idleSetL = p_object->m_ext->m_smacksIdleL;
		p_object->m_ext->m_idleSetR = p_object->m_ext->m_smacksIdleR;
		p_object->m_ext->m_walkSetL = p_object->m_ext->m_smacksWalkL;
		p_object->m_ext->m_walkSetR = p_object->m_ext->m_smacksWalkR;
		p_object->m_ext->m_jumpSetL = p_object->m_ext->m_smacksJumpL;
		p_object->m_ext->m_jumpSetR = p_object->m_ext->m_smacksJumpR;
		p_object->m_ext->m_fallSetL = p_object->m_ext->m_smacksFallL;
		p_object->m_ext->m_fallSetR = p_object->m_ext->m_smacksFallR;
		p_object->m_ext->m_hurtSetL = p_object->m_ext->m_smacksHurtL;
		p_object->m_ext->m_hurtSetR = p_object->m_ext->m_smacksHurtR;
		p_object->m_ext->m_duckSetL = p_object->m_ext->m_smacksDuckL;
		p_object->m_ext->m_duckSetR = p_object->m_ext->m_smacksDuckR;
		p_object->m_ext->m_riseSetL = p_object->m_ext->m_smacksRiseL;
		p_object->m_ext->m_riseSetR = p_object->m_ext->m_smacksRiseR;
		p_object->m_ext->m_poseSet = p_object->m_ext->m_smacksPose;
		p_object->m_ext->m_specialSetL = p_object->m_ext->m_smacksSpecialL;
		p_object->m_ext->m_specialSetR = p_object->m_ext->m_smacksSpecialR;
		SetObjectSprite(p_object->m_state->m_portrait, p_object->m_ext->m_smacksPortrait, 5);
		break;
	case 2:
		p_object->m_ext->m_walkSpeed = g_playerSpeedWalk * 1.25;
		p_object->m_ext->m_jumpSpeed = g_playerSpeedRun * 1.25;
		p_object->m_ext->m_idleSetL = p_object->m_ext->m_tonyIdleL;
		p_object->m_ext->m_idleSetR = p_object->m_ext->m_tonyIdleR;
		p_object->m_ext->m_walkSetL = p_object->m_ext->m_tonyWalkL;
		p_object->m_ext->m_walkSetR = p_object->m_ext->m_tonyWalkR;
		p_object->m_ext->m_jumpSetL = p_object->m_ext->m_tonyJumpL;
		p_object->m_ext->m_jumpSetR = p_object->m_ext->m_tonyJumpR;
		p_object->m_ext->m_fallSetL = p_object->m_ext->m_tonyFallL;
		p_object->m_ext->m_fallSetR = p_object->m_ext->m_tonyFallR;
		p_object->m_ext->m_hurtSetL = p_object->m_ext->m_tonyHurtL;
		p_object->m_ext->m_hurtSetR = p_object->m_ext->m_tonyHurtR;
		p_object->m_ext->m_duckSetL = p_object->m_ext->m_tonyDuckL;
		p_object->m_ext->m_duckSetR = p_object->m_ext->m_tonyDuckR;
		p_object->m_ext->m_riseSetL = p_object->m_ext->m_tonyRiseL;
		p_object->m_ext->m_riseSetR = p_object->m_ext->m_tonyRiseR;
		p_object->m_ext->m_poseSet = p_object->m_ext->m_tonyPose;
		p_object->m_ext->m_specialSetL = p_object->m_ext->m_tonySpecialL;
		p_object->m_ext->m_specialSetR = p_object->m_ext->m_tonySpecialR;
		SetObjectSprite(p_object->m_state->m_portrait, p_object->m_ext->m_tonyPortrait, 5);
		break;
	case 4:
		p_object->m_ext->m_walkSpeed = g_playerSpeedWalk;
		p_object->m_ext->m_jumpSpeed = g_playerSpeedRun;
		p_object->m_ext->m_idleSetL = p_object->m_ext->m_cocoIdleL;
		p_object->m_ext->m_idleSetR = p_object->m_ext->m_cocoIdleR;
		p_object->m_ext->m_walkSetL = p_object->m_ext->m_cocoWalkL;
		p_object->m_ext->m_walkSetR = p_object->m_ext->m_cocoWalkR;
		p_object->m_ext->m_jumpSetL = p_object->m_ext->m_cocoJumpL;
		p_object->m_ext->m_jumpSetR = p_object->m_ext->m_cocoJumpR;
		p_object->m_ext->m_fallSetL = p_object->m_ext->m_cocoFallL;
		p_object->m_ext->m_fallSetR = p_object->m_ext->m_cocoFallR;
		p_object->m_ext->m_hurtSetL = p_object->m_ext->m_cocoHurtL;
		p_object->m_ext->m_hurtSetR = p_object->m_ext->m_cocoHurtR;
		p_object->m_ext->m_duckSetL = p_object->m_ext->m_cocoDuckL;
		p_object->m_ext->m_duckSetR = p_object->m_ext->m_cocoDuckR;
		p_object->m_ext->m_riseSetL = p_object->m_ext->m_cocoRiseL;
		p_object->m_ext->m_riseSetR = p_object->m_ext->m_cocoRiseR;
		p_object->m_ext->m_poseSet = p_object->m_ext->m_cocoPose;
		p_object->m_ext->m_specialSetL = p_object->m_ext->m_cocoSpecialL;
		p_object->m_ext->m_specialSetR = p_object->m_ext->m_cocoSpecialR;
		SetObjectSprite(p_object->m_state->m_portrait, p_object->m_ext->m_cocoPortrait, 5);
		break;
	case 3:
		p_object->m_ext->m_walkSpeed = g_playerSpeedWalk;
		p_object->m_ext->m_jumpSpeed = g_playerSpeedRun;
		p_object->m_ext->m_idleSetL = p_object->m_ext->m_trioIdleL;
		p_object->m_ext->m_idleSetR = p_object->m_ext->m_trioIdleR;
		p_object->m_ext->m_walkSetL = p_object->m_ext->m_trioWalkL;
		p_object->m_ext->m_walkSetR = p_object->m_ext->m_trioWalkR;
		p_object->m_ext->m_jumpSetL = p_object->m_ext->m_trioJumpL;
		p_object->m_ext->m_jumpSetR = p_object->m_ext->m_trioJumpR;
		p_object->m_ext->m_fallSetL = p_object->m_ext->m_trioFallL;
		p_object->m_ext->m_fallSetR = p_object->m_ext->m_trioFallR;
		p_object->m_ext->m_hurtSetL = p_object->m_ext->m_trioHurtL;
		p_object->m_ext->m_hurtSetR = p_object->m_ext->m_trioHurtR;
		p_object->m_ext->m_duckSetL = p_object->m_ext->m_trioDuckL;
		p_object->m_ext->m_duckSetR = p_object->m_ext->m_trioDuckR;
		p_object->m_ext->m_riseSetL = p_object->m_ext->m_trioRiseL;
		p_object->m_ext->m_riseSetR = p_object->m_ext->m_trioRiseR;
		p_object->m_ext->m_poseSet = p_object->m_ext->m_trioPose;
		p_object->m_ext->m_specialSetL = p_object->m_ext->m_trioSpecialL;
		p_object->m_ext->m_specialSetR = p_object->m_ext->m_trioSpecialR;
		SetObjectSprite(p_object->m_state->m_portrait, p_object->m_ext->m_trioPortrait, 5);
		break;
	}

	SetPlayerState(p_object, 4, 0);
	CounterRefreshSprites(p_object);
}

// FUNCTION: TONY2 0x0040b680
void __fastcall RefreshHealthBar(GameObject* p_object)
{
	switch (p_object->m_state->m_health) {
	case 0:
		SetFrameSet(p_object->m_state->m_healthBar, -1);
		break;
	case 1:
		SetFrameSet(p_object->m_state->m_healthBar, p_object->m_state->m_meterSet1);
		break;
	case 2:
		SetFrameSet(p_object->m_state->m_healthBar, p_object->m_state->m_meterSet2);
		break;
	case 3:
		SetFrameSet(p_object->m_state->m_healthBar, p_object->m_state->m_meterSet3);
		break;
	}
}

// FUNCTION: TONY2 0x0040b9f0
void __fastcall ShowCharacterSelect(GameObject* p_object)
{
	DialogBuilder::DialogItem a[4];
	DialogBuilder::DialogItem b[4];
	DialogBuilder::DialogItem* specs;
	DialogBuilder lantern;
	TonyS32 count;
	TonyS32 done;

	done = 0;
	g_objectManager->SuspendGameplay();
	a[0].m_kind = 2;
	a[0].m_x = 0x140;
	a[0].m_y = 0x5a;
	a[0].m_flags = 2;
	a[0].m_stringId = 0x32;
	a[0].m_tag = 0;
	a[0].m_callback = NULL;
	a[1].m_kind = 5;
	a[1].m_x = 0xfa;
	a[1].m_y = 0x96;
	a[1].m_flags = 0;
	a[1].m_stringId = 0x33;
	a[1].m_tag = 0x371;
	a[1].m_callback = NULL;
	a[2].m_kind = 5;
	a[2].m_x = 0xfa;
	a[2].m_y = 0xc8;
	a[2].m_flags = 0;
	a[2].m_stringId = 0x36;
	a[2].m_tag = 0xa96;
	a[2].m_callback = NULL;
	a[3].m_kind = 5;
	a[3].m_x = 0xfa;
	a[3].m_y = 0xfa;
	a[3].m_flags = 0;
	a[3].m_stringId = 0x34;
	a[3].m_tag = 0x370;
	a[3].m_callback = NULL;
	b[0].m_kind = 2;
	b[0].m_x = 0x140;
	b[0].m_y = 0x5a;
	b[0].m_flags = 2;
	b[0].m_stringId = 0x32;
	b[0].m_tag = 0;
	b[0].m_callback = NULL;
	b[1].m_kind = 5;
	b[1].m_x = 0xfa;
	b[1].m_y = 0x96;
	b[1].m_flags = 0;
	b[1].m_stringId = 0x33;
	b[1].m_tag = 0x371;
	b[1].m_callback = NULL;
	b[2].m_kind = 5;
	b[2].m_x = 0xfa;
	b[2].m_y = 0xc8;
	b[2].m_flags = 0;
	b[2].m_stringId = 0x35;
	b[2].m_tag = 0xdfd;
	b[2].m_callback = NULL;
	b[3].m_kind = 5;
	b[3].m_x = 0xfa;
	b[3].m_y = 0xfa;
	b[3].m_flags = 0;
	b[3].m_stringId = 0x34;
	b[3].m_tag = 0x370;
	b[3].m_callback = NULL;
	count = (g_camera->m_backdrop != 0) + 3;

	if (g_language >= 0 && g_language <= 4) {
		specs = a;

		switch (p_object->m_ext->m_trioVariant) {
		case 1:
			a[2].m_stringId = 0x37;
			a[2].m_tag = 0xa99;
			break;
		case 2:
			a[2].m_stringId = 0x38;
			a[2].m_tag = 0xa97;
			break;
		}
	}

	if (g_language >= 5 && g_language <= 9) {
		specs = b;
	}

	lantern.Build(specs, count);
	lantern.SetBackdrop(0x6bf);

	do {
		switch (specs[lantern.Run()].m_stringId) {
		case 0x33:
			PlayerSetForm(p_object, 2, 0);
			done = 1;
			break;
		case 0x35:
			PlayerSetForm(p_object, 1, 0);
			done = 1;
			break;
		case 0x34:
			PlayerSetForm(p_object, 4, 0);
			done = 1;
			break;
		case 0x36:
		case 0x37:
		case 0x38:
			PlayerSetForm(p_object, 3, 0);
			done = 1;
			break;
		}
	} while (done == 0);

	lantern.Teardown();
	g_objectManager->ResumeGameplay();
}

// FUNCTION: TONY2 0x0040bd20
void __fastcall PlayerReset(GameObject* p_object, TonyS32 p_fresh)
{
	p_object->m_state->m_health = 3;
	PlayerRequestState(p_object, 0);
	SetPlayerState(p_object, 1, 0);
	g_camera->m_scrollLock = 0;
	ObjectSetFlags(p_object, 0, 0x1008);
	p_object->m_state->m_groundBox = NULL;
	PlayerSetForm(p_object, p_object->m_state->m_respawnForm, 0);

	if (g_objectManager->m_spawnPoint != 0 && p_fresh == 1) {
		g_objectManager->FreeObject((GameObject*) g_objectManager->m_spawnPoint);
	}

	p_object->m_state->m_shieldTicks = 0;
	RefreshHealthBar(p_object);
	CounterRefreshSprites(p_object);

	if (p_fresh == 1) {
		SetSegment((GameObject*) p_object->m_state->m_keyGauge, 0, 0);
		SetSegment((GameObject*) p_object->m_state->m_keyGauge, 1, 0);
		SetSegment((GameObject*) p_object->m_state->m_keyGauge, 2, 0);
		SetSegment((GameObject*) p_object->m_state->m_keyGauge, 3, 0);
		SetSegment((GameObject*) p_object->m_state->m_nutrientGauge, 0, 0);
		SetSegment((GameObject*) p_object->m_state->m_nutrientGauge, 1, 0);
		SetSegment((GameObject*) p_object->m_state->m_nutrientGauge, 2, 0);
		p_object->m_state->m_cereals = 0;
		p_object->m_state->m_cerealsLevel = 0;
	}
}

// FUNCTION: TONY2 0x0040be50
void __fastcall ShowLevelComplete(GameObject* p_object)
{
	DialogBuilder::DialogItem specs[13];
	ProfileData prices;
	DialogBuilder lantern;
	TonyS32 done;

	done = 0;
	g_objectManager->SuspendGameplay();
	prices = LoadProfile(g_gameManager->m_profile);
	specs[0].m_kind = 2;
	specs[0].m_x = 0x140;
	specs[0].m_y = 0x5a;
	specs[0].m_flags = 2;
	specs[0].m_stringId = 0x23;
	specs[0].m_tag = 0;
	specs[0].m_callback = NULL;
	specs[1].m_kind = 8;
	specs[1].m_x = 0x12c;
	specs[1].m_y = 0x8c;
	specs[1].m_flags = 0;
	specs[1].m_stringId = 0x3c;
	specs[1].m_tag = 0;
	specs[1].m_callback = NULL;
	specs[2].m_kind = 0;
	specs[2].m_x = 0x14a;
	specs[2].m_y = 0x8c;
	specs[2].m_flags = 0;
	specs[2].m_stringId = 5;
	specs[2].m_tag = p_object->m_state->m_lives;
	specs[2].m_callback = TallyNumber;
	specs[3].m_kind = 8;
	specs[3].m_x = 0x12c;
	specs[3].m_y = 0xb4;
	specs[3].m_flags = 0;
	specs[3].m_stringId = 0x24;
	specs[3].m_tag = 0;
	specs[3].m_callback = NULL;
	specs[4].m_kind = 0;
	specs[4].m_x = 0x14a;
	specs[4].m_y = 0xb4;
	specs[4].m_flags = 0;
	specs[4].m_stringId = 0x25;
	specs[4].m_tag = p_object->m_state->m_cerealsLevel;
	specs[4].m_callback = TallyOutOf;
	specs[5].m_kind = 8;
	specs[5].m_x = 0x12c;
	specs[5].m_y = 0xdc;
	specs[5].m_flags = 0;
	specs[5].m_stringId = 0x26;
	specs[5].m_tag = 0;
	specs[5].m_callback = NULL;
	specs[6].m_kind = 4;
	specs[6].m_x = 0x154;
	specs[6].m_y = 0xe2;
	specs[6].m_flags = 0;
	specs[6].m_stringId = 7;
	specs[6].m_tag = 0xaaa;
	specs[6].m_callback = NULL;
	specs[7].m_kind = 4;
	specs[7].m_x = 0x172;
	specs[7].m_y = 0xe2;
	specs[7].m_flags = 0;
	specs[7].m_stringId = 7;
	specs[7].m_tag = 0xaa8;
	specs[7].m_callback = NULL;
	specs[8].m_kind = 4;
	specs[8].m_x = 0x190;
	specs[8].m_y = 0xe2;
	specs[8].m_flags = 0;
	specs[8].m_stringId = 7;
	specs[8].m_tag = 0x833;
	specs[8].m_callback = NULL;
	specs[9].m_kind = 8;
	specs[9].m_x = 0x12c;
	specs[9].m_y = 0x104;
	specs[9].m_flags = 0;
	specs[9].m_stringId = 0x28;
	specs[9].m_tag = 0;
	specs[9].m_callback = NULL;
	specs[10].m_kind = 0;
	specs[10].m_x = 0x14a;
	specs[10].m_y = 0x104;
	specs[10].m_flags = 0;
	specs[10].m_stringId = 0x29;
	specs[10].m_tag = prices.m_total * 100 / 0xa71;
	specs[10].m_callback = TallyPercent;
	specs[11].m_kind = 4;
	specs[11].m_x = 0x212;
	specs[11].m_y = 0x10c;
	specs[11].m_flags = 0;
	specs[11].m_stringId = 7;
	specs[11].m_tag = 0x66e;
	specs[11].m_callback = NULL;
	specs[12].m_kind = 1;
	specs[12].m_x = 0x12c;
	specs[12].m_y = 0x140;
	specs[12].m_flags = 0;
	specs[12].m_stringId = 0x3b;
	specs[12].m_tag = 0;
	specs[12].m_callback = NULL;

	if (g_language >= 1 && g_language <= 4) {
		specs[11].m_tag = 0xe5e;
	}

	if (g_language >= 5 && g_language <= 9) {
		specs[11].m_tag = 0x154;
	}

	if (!(GetSegmentMask(p_object->m_state->m_keyGauge) & 1)) {
		specs[6].m_kind &= ~4;
	}

	if (!(GetSegmentMask(p_object->m_state->m_keyGauge) & 2)) {
		specs[7].m_kind &= ~4;
	}

	if (!(GetSegmentMask(p_object->m_state->m_keyGauge) & 4)) {
		specs[8].m_kind &= ~4;
	}

	lantern.Build(specs, 0xd);
	lantern.SetBackdrop(0x6bf);

	do {
		if (specs[lantern.Run()].m_stringId == 0x3b) {
			done = 1;
		}
	} while (done == 0);

	lantern.Teardown();
	g_objectManager->ResumeGameplay();
}

// FUNCTION: TONY2 0x0040c2e0
void __fastcall TallyPercent(GameObject* p_object, TonyS32 p_value)
{
	FormatObjectText(p_object, 0x29, p_value);
}

// FUNCTION: TONY2 0x0040c2f0
void __fastcall TallyNumber(GameObject* p_object, TonyS32 p_value)
{
	FormatObjectText(p_object, 5, p_value);
}

// FUNCTION: TONY2 0x0040c300
void __fastcall TallyOutOf(GameObject* p_object, TonyS32 p_value)
{
	FormatObjectText(p_object, 0x25, p_value, g_camera->m_totals[0]);
}

// FUNCTION: TONY2 0x0040c4a0
void __fastcall SetBonusTimer(GameObject* p_object, TonyS32 p_on)
{
	if (p_on > 0) {
		if (p_object->m_state->m_bonusTimer->m_head->m_flags & 0x800) {
			ObjectSetFlags(p_object->m_state->m_bonusTimer, 0, 0x800);
			p_object->m_state->m_bonusTicks = p_on;
		}
	}

	if (p_on == 0) {
		ObjectSetFlags(p_object->m_state->m_bonusTimer, 0x800, 0);
		p_object->m_state->m_bonusTicks = 0;
	}
}

// FUNCTION: TONY2 0x0040c500
void __fastcall TickBonusTimer(GameObject* p_object)
{
	TonyS32 seg;

	if (!(p_object->m_state->m_bonusTimer->m_head->m_flags & 0x800)) {
		seg = p_object->m_state->m_bonusTicks / 0x23;

		if (seg < 10) {
			ObjectSetFlags(p_object->m_state->m_bonusTimer, 0x40, 0);
		}
		else {
			ObjectSetFlags(p_object->m_state->m_bonusTimer, 0, 0x40);
		}

		FormatObjectText(p_object->m_state->m_bonusTimer, 5, seg);
		p_object->m_state->m_bonusTicks--;

		if (p_object->m_state->m_bonusTicks == 0) {
			ObjectSetFlags(p_object->m_state->m_bonusTimer, 0x800, 0);
			g_objectManager->m_stateFlags |= 1;
			g_objectManager->m_stateFlags &= ~0x40;
		}
	}
}

// FUNCTION: TONY2 0x0040c670
void __fastcall StoreRespawnForm(GameObject* p_object)
{
	p_object->m_state->m_respawnForm = ((CounterTemplate::Head*) p_object->m_head)->m_value;
}

// FUNCTION: TONY2 0x0040c680
void __fastcall SaveCheckpointCounters(GameObject* p_object)
{
	p_object->m_state->m_checkpointKeys = GetSegmentMask(p_object->m_state->m_keyGauge);
	p_object->m_state->m_checkpointNutrients = GetSegmentMask((GameObject*) p_object->m_state->m_nutrientGauge);
	p_object->m_state->m_checkpointCerealsLevel = p_object->m_state->m_cerealsLevel;
	p_object->m_state->m_checkpointCereals = p_object->m_state->m_cereals;
}

// FUNCTION: TONY2 0x0040c6e0
void __fastcall RestoreCheckpointCounters(GameObject* p_object)
{
	TonyS32 i;

	for (i = 0; i < 4; i++) {
		if (p_object->m_state->m_checkpointKeys & (1 << i)) {
			SetSegment((GameObject*) p_object->m_state->m_keyGauge, i, 1);
		}
		else {
			SetSegment((GameObject*) p_object->m_state->m_keyGauge, i, 0);
		}
	}

	for (i = 0; i < 3; i++) {
		if (p_object->m_state->m_checkpointNutrients & (1 << i)) {
			SetSegment((GameObject*) p_object->m_state->m_nutrientGauge, i, 1);
		}
		else {
			SetSegment((GameObject*) p_object->m_state->m_nutrientGauge, i, 0);
		}
	}

	p_object->m_state->m_cerealsLevel = p_object->m_state->m_checkpointCerealsLevel;
	p_object->m_state->m_cereals = p_object->m_state->m_checkpointCereals;
}

// FUNCTION: TONY2 0x0040c770
void __fastcall LightAllNutrients(GameObject* p_object)
{
	TonyS32 i;

	for (i = 0; i < 3; i++) {
		SetSegment((GameObject*) p_object->m_state->m_nutrientGauge, i, 1);
	}
}

// World-map node positions; entry 0 is the off-map slot (float -320.0f bits, read
// as the flake spawn offset by SpawnCheatFlakes).
// GLOBAL: TONY2 0x0044c5e8
static const TonyS32 g_worldMapNodes[8][2] = {
	{(TonyS32) 0xc3a00000, 0},
	{177, 325},
	{137, 269},
	{207, 214},
	{268, 275},
	{373, 222},
	{403, 308},
	{524, 297},
};

// Fully implemented, kept as STUB because it compares at 95.5%: all 48 flake objects,
// the random drift, callback install and flag clear match. The original stores m_flags
// twice (kept faithfully); cl 11.00.7022 schedules the first m_flags store into the
// m_x fsub/fstp latency gap and swaps the m_y/m_facing stores, where the
// original fills m_y's load-use gap instead (reordering the statements makes SP3
// dead-store-eliminate the duplicate). Store-scheduling margin; re-annotate when solved.
// STUB: TONY2 0x0040c7a0
void __fastcall SpawnCheatFlakes(GameObject* p_object)
{
	OverlayData* block;
	GameObject* object;
	TonyS32 i;

	if (g_objectManager->m_stateFlags & 0x100) {
		PlayObjectSound(p_object, 0x18, -1, -1);

		for (i = 0; i < 0x30; i++) {
			block = (OverlayData*) malloc(0x1f4);
			block->m_type = 0x19;
			block->m_x = g_camera->m_x - *(const TonyFloat*) &g_worldMapNodes[0][0];
			block->m_flags = 0;
			block->m_y = g_camera->m_y;
			block->m_facing = 0;
			block->m_flags = 0;
			block->m_layer = p_object->m_head->m_layer + 1;
			block->m_arg0 = 0;
			block->m_arg1 = 1;
			block->m_arg5 = 0xab;
			block->m_arg2 = 0;
			block->SpawnOnce();
			object = g_objectManager->AllocObject();
			InitObjectFromData(object, block);
			g_objectManager->InsertObject(object, 8);
			ObjectSetFlags(object, 2, 0);
			object->m_state->m_velY = (TonyFloat) - (rand() % 0x18);
			object->m_state->m_velX = (TonyFloat) (rand() % 0x30 - 0x18);
			object->m_destroyFn = ObjectFreeTemplate;
		}

		g_objectManager->m_stateFlags &= ~0x100;
	}
}

// FUNCTION: TONY2 0x0040c8e0
void __fastcall WorldMapInit(GameObject* p_object, PlayerTemplate* p_template)
{
	BindPlayerTemplate(p_object, p_template);
	p_object->m_tickFn = WorldMapTick;
	p_object->m_drawFn = WorldMapDraw;
	p_object->m_destroyFn = WorldMapDestroy;
	WorldMapPlace(p_object);
	SetObjectSprite(p_object, p_object->m_ext->m_idleSetR, 5);
}

// FUNCTION: TONY2 0x0040c920
void __fastcall WorldMapReinit(GameObject* p_object, PlayerTemplate* p_template)
{
	BindPlayerTemplate(p_object, p_template);
	WorldMapStartMusic(p_object);
}

// FUNCTION: TONY2 0x0040c940
void __fastcall WorldMapStartMusic(GameObject* p_object)
{
	g_soundManager->PlaySong(g_gameManager->m_songs[0xe]);
	SceneryResolveSprite(p_object);
}

// FUNCTION: TONY2 0x0040c960
void __fastcall WorldMapPlace(GameObject* p_object)
{
	InitMotion(p_object);
}

// Fully implemented, kept as STUB because it compares at 90%: the pause/cheat sprite
// swaps, the wrap-around cursor with its %-count arithmetic and both key checks match;
// the residue is a scratch-register pick on the decrement, the original reading the key
// word through ax, and the lea operand order in the wrap expression — all compiler
// canonicalization (register-seeding family, see TickAll). Re-annotate when the
// vintage is found.
// STUB: TONY2 0x0040c970
TonyS32 __fastcall WorldMapTick(GameObject* p_object)
{
	TonyS32 count;

	SceneryTick(p_object);

	if (g_objectManager->m_stateFlags & 0x10) {
		((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0 = 0x12;
	}

	if (g_objectManager->m_stateFlags & 0x400) {
		((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0 = 0x14;
	}

	((PlayerTemplate::Head*) p_object->m_head)->m_mapNode--;

	if (g_inputManager->m_buttons & 6) {
		count = ((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0;

		if (count > 1) {
			((PlayerTemplate::Head*) p_object->m_head)->m_mapNode =
				(count + ((PlayerTemplate::Head*) p_object->m_head)->m_mapNode - 1) % count;
			PlayObjectSound(p_object, 0xe, -1, 0x40);
		}
	}

	if (g_inputManager->m_buttons & 9) {
		count = ((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0;

		if (count > 1) {
			((PlayerTemplate::Head*) p_object->m_head)->m_mapNode =
				(((PlayerTemplate::Head*) p_object->m_head)->m_mapNode + 1) % count;
			PlayObjectSound(p_object, 0xe, -1, 0x40);
		}
	}

	((PlayerTemplate::Head*) p_object->m_head)->m_mapNode++;
	return 0;
}

// Fully implemented, kept as STUB because it compares at 95%: the marker, node stars and
// the five-dot path interpolation all match, but the strength-reduced walk over the node
// table anchors at the x column in the original and the y column here — every access
// resolves to the same slot at a +/-4 displacement. Induction-anchor margin. Re-annotate
// when solved.
// STUB: TONY2 0x0040ca40
void __fastcall WorldMapDraw(GameObject* p_object)
{
	TonyS32 sprA;
	TonyS32 sprB;
	TonyS32 i;
	TonyS32 j;

	SceneryDraw(p_object);

	if (g_objectManager->m_frameCounter & 4) {
		g_videoManager->QueueSprite(
			g_objectManager->m_player->m_state->m_portrait->m_state->m_sprite,
			g_worldMapNodes[((PlayerTemplate::Head*) p_object->m_head)->m_mapNode][0] - 0xa,
			g_worldMapNodes[((PlayerTemplate::Head*) p_object->m_head)->m_mapNode][1] - 0x14,
			p_object->m_head->m_layer + 0x30,
			0
		);
	}

	sprA = g_videoManager->GetSprite(0xaa7, 0);

	if (((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0 > 1) {
		g_videoManager->QueueSprite(
			sprA,
			g_worldMapNodes[1][0] + 4,
			g_worldMapNodes[1][1] - 0x1d,
			p_object->m_head->m_layer + 0x20,
			0
		);
	}

	sprB = g_videoManager->GetSprite(0xaa9, 0);

	for (i = 1; i < ((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0; i++) {
		for (j = 0; j < 5; j++) {
			g_videoManager->QueueSprite(
				sprB,
				(TonyS32) (TonyFloat) ((g_worldMapNodes[i + 1][0] - g_worldMapNodes[i][0]) * j / 5 +
									   g_worldMapNodes[i][0]),
				(TonyS32) (TonyFloat) ((g_worldMapNodes[i + 1][1] - g_worldMapNodes[i][1]) * j / 5 +
									   g_worldMapNodes[i][1]),
				p_object->m_head->m_layer + 0x10,
				0
			);
		}

		if (i < ((ObjectTemplate::SpriteExt*) p_object->m_ext)->m_param0 - 1) {
			g_videoManager->QueueSprite(
				sprA,
				g_worldMapNodes[i + 1][0] + 4,
				g_worldMapNodes[i + 1][1] - 0x1d,
				p_object->m_head->m_layer + 0x20,
				0
			);
		}
	}
}

// FUNCTION: TONY2 0x0040cc00
void __fastcall WorldMapDestroy(GameObject* p_object)
{
	g_videoManager->FreeSprite(p_object->m_ext->m_idleSetR, 0);
}

// FUNCTION: TONY2 0x0040cc20
ObjectManager::ObjectManager()
{
	TonyS32 i;

	m_freeCount = 0x100;

	for (i = 0; i < (TonyS32) sizeOfArray(m_layers); i++) {
		m_layers[i] = new GameObject();
	}

	for (i = 0; i < (TonyS32) sizeOfArray(m_freeStack); i++) {
		m_freeStack[i] = new GameObject();
	}

	m_frameCounter = 0;
	m_freeQueueCount = 0;
	m_stateFlags = 0;
	m_drawMode = 0;
	m_spawnPoint = 0;
	m_smoothPass = 0;
	m_gravity = 1.4f;
}

// FUNCTION: TONY2 0x0040cd10
void ObjectManager::DestroyAll()
{
	TonyS32 i;

	FreeAllObjects();
	FlushFreeQueue();

	for (i = 0; i < (TonyS32) sizeOfArray(m_freeStack); i++) {
		GameObject* object = m_freeStack[i];
		if (object) {
			object->FreeDataBlock();
			delete object;
		}
	}

	for (i = 0; i < (TonyS32) sizeOfArray(m_layers); i++) {
		GameObject* object = m_layers[i];
		if (object) {
			object->FreeDataBlock();
			delete object;
		}
	}
}

// Fully implemented, kept as STUB because it compares at 39%: all blocks, calls, the
// status switch (with the case -2 fallthrough) and both loops match, but the allocator
// homes `this` in ebx and keeps the dead object counter in memory, where the recompile
// gives the counter ebp and spills `this` — every downstream register shifts, and the
// flag clear degrades to a byte-and through a register. Same allocator-margin family as
// RefreshHitBoxes/CreateGameWindow. Re-annotate as FUNCTION when a matching form is found.
// With /O2 the forwarding body compiles to a tail jump.
// FUNCTION: TONY2 0x0040cd80
void ObjectManager::ReleaseAllObjects()
{
	FreeAllObjects();
}

// STUB: TONY2 0x0040cd90
void ObjectManager::TickAll()
{
	TonyS32 n;
	GameObject* object;
	GameObject* current;
	TonyS32 i;

	n = 0;
	m_playerHitCount = 0;
	m_playerFeetHitCount = 0;
	m_playerBodyHitCount = 0;

	for (i = 0; i < (TonyS32) sizeOfArray(m_layers); i++) {
		object = m_layers[i]->m_next;

		while (object) {
			n++;
			current = object;
			object = object->m_next;

			if (current->m_suspendCount == 0) {
				current->m_state->m_tickStatus = 0;
				current->m_tickFn(current);

				switch (current->m_state->m_tickStatus) {
				case -2:
					current->m_state->m_template->m_head.m_flags &= 0xfffffffe;
				case -3:
				case -1:
					FreeObject(current);
					break;
				}
			}
		}
	}

	FlushFreeQueue();
	m_frameCounter++;
}

// FUNCTION: TONY2 0x0040ce50
void ObjectManager::FreeObject(GameObject* p_object)
{
	if (p_object->m_destroyFn) {
		p_object->m_destroyFn(p_object);
	}

	m_freeCount++;
	m_freeStack[m_freeCount - 1] = p_object;

	if (p_object->m_prev) {
		p_object->m_prev->m_next = p_object->m_next;
	}

	if (p_object->m_next) {
		p_object->m_next->m_prev = p_object->m_prev;
	}

	p_object->Clear();
}

// FUNCTION: TONY2 0x0040cea0
void ObjectManager::FreeAllObjects()
{
	TonyS32 i;

	for (i = 0; i < (TonyS32) sizeOfArray(m_layers); i++) {
		GameObject* object = m_layers[i]->m_next;

		while (object) {
			GameObject* current = object;
			object = object->m_next;
			FreeObject(current);
		}
	}
}

// FUNCTION: TONY2 0x0040cee0
void ObjectManager::FreeTransientObjects()
{
	GameObject* object;
	GameObject* current;
	TonyS32 i;

	for (i = 0; i < (TonyS32) sizeOfArray(m_layers); i++) {
		object = m_layers[i]->m_next;

		while (object) {
			current = object;
			object = object->m_next;

			if (!(current->m_head->m_flags & 0x80)) {
				FreeObject(current);
			}
		}
	}
}

// FUNCTION: TONY2 0x0040cf30
GameObject* ObjectManager::AllocObject()
{
	m_freeCount--;
	return m_freeStack[m_freeCount];
}

// FUNCTION: TONY2 0x0040cf50
void ObjectManager::InsertObject(GameObject* p_object, TonyS32 p_layer)
{
	p_object->m_next = m_layers[p_layer]->m_next;
	p_object->m_prev = m_layers[p_layer];

	if (m_layers[p_layer]->m_next) {
		m_layers[p_layer]->m_next->m_prev = p_object;
	}

	m_layers[p_layer]->m_next = p_object;
}

// FUNCTION: TONY2 0x0040cf90
void ObjectManager::DrawAll()
{
	TonyU8* surface;
	GameObject* object;
	TonyU32 flags;
	TonyFloat x;
	TonyFloat y;
	TonyS32 run;
	TonyS32 i;

	g_videoManager->ResetDrawLists();
	g_backgroundRenderer->ResetArenas();

	for (i = 0; i < (TonyS32) sizeOfArray(m_layers); i++) {
		for (object = m_layers[i]->m_next; object; object = object->m_next) {
			if (object->m_suspendCount == 0) {
				run = 1;
				flags = object->m_head->m_flags;

				if ((flags & 0x20) && (g_objectManager->m_frameCounter & 1)) {
					run = 0;
				}

				if ((flags & 0x40) && (g_objectManager->m_frameCounter & 4)) {
					run = 0;
				}

				if (flags & 0x800) {
					run = 0;
				}

				if (object->m_drawFn && run == 1) {
					object->m_drawFn(object);
				}
			}
		}
	}

	surface = g_videoManager->LockBackSurface();

	if (surface) {
		switch (m_drawMode) {
		case 0: {
			DrawNode* node;

			for (i = 0; i < (TonyS32) sizeOfArray(g_videoManager->m_layers); i++) {
				for (node = g_videoManager->m_layers[i]->m_next; node; node = node->m_next) {
					BlitSprite(node->m_pixels, node->m_rows, node->m_x, node->m_y, node->m_reserved, surface);
				}
			}

			break;
		}
		case 1:
			g_camera->GetViewOffset(&x, &y);
			g_backgroundRenderer->ScrollTo((TonyS32) x, (TonyS32) y);
			g_backgroundRenderer->RenderLandscape(surface);
			break;
		}

		g_videoManager->FlushPixelQueue();
	}

	g_videoManager->UnlockBackSurface();
}

// FUNCTION: TONY2 0x0040d0f0
void ObjectManager::SetPlayer(GameObject* p_object)
{
	m_player = p_object;
}

// FUNCTION: TONY2 0x0040d100
void ObjectManager::ClearLevel()
{
	FreeTransientObjects();
	m_stateFlags &= 0x50;
}

// FUNCTION: TONY2 0x0040d120
void ObjectManager::SuspendGameplay()
{
	GameObject* object;
	GameObject* current;
	TonyS32 i;

	for (i = 0; i < (TonyS32) sizeOfArray(m_layers); i++) {
		object = m_layers[i]->m_next;

		while (object) {
			current = object;
			object = object->m_next;
			current->Suspend();
		}
	}
}

// FUNCTION: TONY2 0x0040d150
void ObjectManager::ResumeGameplay()
{
	GameObject* object;
	GameObject* current;
	TonyS32 i;

	for (i = 0; i < (TonyS32) sizeOfArray(m_layers); i++) {
		object = m_layers[i]->m_next;

		while (object) {
			current = object;
			object = object->m_next;
			current->Resume();
		}
	}
}

// FUNCTION: TONY2 0x0040d180
void ObjectManager::ResetSpawnFlags()
{
	GameObject* object;
	GameObject* current;
	TonyS32 i;

	for (i = 0; i < (TonyS32) sizeOfArray(m_layers); i++) {
		object = m_layers[i]->m_next;

		while (object) {
			current = object;
			object = object->m_next;

			if (current->m_head->m_flags & 2) {
				ObjectSetMoveFlags(current, 0, 1);
			}
		}
	}
}

// FUNCTION: TONY2 0x0040d1c0
void ObjectManager::FreeTemplate(void* p_object)
{
	m_freeQueue[m_freeQueueCount] = p_object;
	m_freeQueueCount++;
}

// FUNCTION: TONY2 0x0040d1f0
void ObjectManager::FlushFreeQueue()
{
	TonyS32 i;

	for (i = 0; i < m_freeQueueCount; i++) {
		free(m_freeQueue[i]);
	}

	m_freeQueueCount = 0;
}

// FUNCTION: TONY2 0x0040d230
void ObjectManager::HideType(TonyS32 p_type, TonyS32 p_hide)
{
	TonyS32 i;
	GameObject* object;
	GameObject* next;

	for (i = 0; i < 0x10; i++) {
		next = m_layers[i]->m_next;

		while (next != NULL) {
			object = next;
			next = object->m_next;

			if (object->m_head->m_type == p_type) {
				object->Suspend();

				if (p_hide == 1) {
					ObjectSetFlags(object, 0x80, 0);
				}
			}
		}
	}
}

// FUNCTION: TONY2 0x0040d2a0
void ObjectManager::ShowType(TonyS32 p_type, TonyS32 p_show)
{
	TonyS32 i;
	GameObject* object;
	GameObject* next;

	for (i = 0; i < 0x10; i++) {
		next = m_layers[i]->m_next;

		while (next != NULL) {
			object = next;
			next = object->m_next;

			if (object->m_head->m_type == p_type) {
				object->Resume();

				if (p_show == 1) {
					ObjectSetFlags(object, 0, 0x80);
				}
			}
		}
	}
}
