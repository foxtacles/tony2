#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "decomp.h"
#include "objecttemplate.h"
#include "types.h"

// Object type ids dispatched by AssignObjectType (0x00410ec0). Family names follow the
// handler prefixes (BackdropInit, PlayerInit, SessionInit, ...).
enum ObjectType {
	e_backdrop = 1,         // tiled parallax image layer
	e_playerBase = 2,       // player without HUD (intro/menu levels)
	e_screenTile = 3,       // scrolling menu backdrop pattern
	e_collectible = 4,      // static pickup (cereal/nutrient/key/shield)
	e_text = 5,             // glyph-run text label
	e_container = 6,        // non-drawing move container (verify)
	e_group = 7,            // sliding widget group (HUD)
	e_scenery = 8,          // static image
	e_sprite = 9,           // animated sprite
	e_banner = 0xa,         // horizon-scaled tiled strip
	e_cameraScript = 0xb,   // scripted camera/rider mover ("r120,u045,...")
	e_prop = 0xc,           // animated prop, player-shaped template (verify)
	e_critter = 0xd,        // ambient critter, startles when the player nears
	e_gauge = 0xe,          // segment gauge (nutrients/keys HUD)
	e_staticEnemy = 0xf,    // stationary hazard with stomp/touch response
	e_mover = 0x10,         // 4-direction patrolling enemy
	e_goal = 0x11,          // level exit, freezes the player into the goal pose
	e_hopper = 0x12,        // leaping enemy
	e_checkpoint = 0x13,    // spawn point + level config (world/music/backdrop)
	e_spring = 0x14,        // trampoline
	e_door = 0x15,          // nutrient-locked door / bonus exit
	e_water = 0x16,         // water surface line
	e_bouncer = 0x17,       // dispenser-spawned bouncing enemy
	e_dispenser = 0x18,     // enemy spawner
	e_dropItem = 0x19,      // falling collectible with physics
	e_splash = 0x1a,        // one-shot water splash effect
	e_session = 0x64,       // in-level player with HUD/lives/water logic
	e_charSelect = 0x65,    // character-select trigger post
	e_speechTrigger = 0x66, // spawns the speech dialog on touch
	e_speech = 0x67,        // GSM speech dialog with animated mouth
	e_worldMap = 0x68,      // world-map marker/cursor
	e_boss = 0x69           // script-driven boss, drops a nutrient on death
};

// Spawn record for dynamically created objects and the dialog/text overlays:
// ObjectTemplate::Head-shaped front, family-dependent template fields behind (the head
// size varies per family, so the 0x1c+ slots are named by offset; e.g. for the
// CounterTemplate-shaped enemies m_arg0/m_arg1 are kind/value and the Ext
// starts at m_arg2, while for plain ObjectTemplate types the Ext starts at
// m_arg0).
// SIZE 0x1f4
struct OverlayData {
	void FreePayload();
	void SpawnOnce();

	TonyS32 m_type;                 // 0x00
	TonyFloat m_x;                  // 0x04
	TonyFloat m_y;                  // 0x08
	TonyS32 m_flags;                // 0x0c
	TonyS32 m_reserved0;            // 0x10
	TonyS32 m_facing;               // 0x14
	TonyS32 m_layer;                // 0x18
	TonyS32 m_arg0;                 // 0x1c
	TonyS32 m_arg1;                 // 0x20
	TonyS32 m_arg2;                 // 0x24
	TonyS32 m_arg3;                 // 0x28
	TonyS32 m_arg4;                 // 0x2c
	TonyS32 m_arg5;                 // 0x30
	TonyFloat m_arg6;               // 0x34
	TonyFloat m_arg7;               // 0x38
	TonyFloat m_arg8;               // 0x3c
	TonyFloat m_arg9;               // 0x40
	undefined m_pad0[0x4c - 0x44];  // 0x44
	TonyS32 m_arg12;                // 0x4c
	undefined m_pad1[0x5c - 0x50];  // 0x50
	void* m_payload;                // 0x5c
	undefined m_pad2[0x1f4 - 0x60]; // 0x60
};

DECOMP_SIZE_ASSERT(OverlayData, 0x1f4)

class Camera;
struct HitBox;
struct GameObject;

// Save/profile snapshot copied out by LoadProfile.
// SIZE 0x60
struct ProfileData {
	TonyS32 m_node;         // 0x00
	TonyS32 m_scores[0x16]; // 0x04
	TonyS32 m_total;        // 0x5c
};

// Per-difficulty profile snapshots, read and written as by-value ProfileData records
// (the hidden return slot is pushed behind the __fastcall register args).
ProfileData __fastcall LoadProfile(TonyS32 p_profile);
void __fastcall SaveProfile(TonyS32 p_profile, ProfileData p_record);

// Per-type collision-response callback, installed into State::m_hitFrameFn/m_hitWorldFn
// and dispatched by CollideWithGroundLines (0x004022c0) / CollideWithMapBoxes
// (0x00401610) with the touched frame in the second register.
typedef TonyFloat(__fastcall* HitResponseFn)(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value);

// Collision query hit, collected by CollectFrameHitBoxes (0x004013c0): the object-local
// hitbox rect, its kind bits and the owning object.
// SIZE 0x1c
struct HitResult {
	TonyS32 m_left;       // 0x00
	TonyS32 m_top;        // 0x04
	TonyS32 m_right;      // 0x08
	TonyS32 m_bottom;     // 0x0c
	TonyS32 m_kind;       // 0x10
	GameObject* m_object; // 0x14
	TonyS32 m_index;      // 0x18
};

// Game object. Instances are set up through a per-type switch (0x00410ec0) that installs
// an init function at m_initFn; the init functions install the handler callbacks at
// m_tickFn/m_drawFn/m_destroyFn. ObjectManager pools these (free stack m_freeStack,
// per-layer chains m_layers) and links them through m_next/m_prev.
// SIZE 0x2c
struct GameObject {
	// Runtime state, lives behind the copied Head in the instance data block (+0x1c for
	// plain ObjectTemplate types; +0x14/0x18/0x20/0x24 for the other head shapes). The
	// slots from m_typeFlags on are per-family scratch; the names follow the player
	// family and the per-family overloads are noted per member.
	struct State {
		TonyFloat m_worldX;            // 0x00 parent-relative world anchor
		TonyFloat m_worldY;            // 0x04
		TonyS32 m_worldXInt;           // 0x08
		TonyS32 m_worldYInt;           // 0x0c
		GameObject* m_parent;          // 0x10
		undefined m_pad3[0x18 - 0x14]; // 0x14
		TonyS32 m_tickStatus;          // 0x18 -1/-3 despawn, -2 despawn + allow respawn
		ObjectTemplate* m_template;    // 0x1c
		TonyS32 m_localXInt;           // 0x20
		TonyS32 m_localYInt;           // 0x24
		TonyFloat m_velX;              // 0x28
		TonyFloat m_velY;              // 0x2c
		TonyFloat m_prevLocalX;        // 0x30
		TonyFloat m_prevLocalY;        // 0x34
		TonyFloat m_prevX;             // 0x38 previous-frame world position
		TonyFloat m_prevY;             // 0x3c
		TonyFloat m_prev2LocalX;       // 0x40 two frames back
		TonyFloat m_prev2LocalY;       // 0x44
		TonyFloat m_prev2X;            // 0x48
		TonyFloat m_prev2Y;            // 0x4c
		TonyS32 m_prevXInt;            // 0x50
		TonyS32 m_prevYInt;            // 0x54
		TonyS32 m_boundsMinX;          // 0x58 frame extent, object-local
		TonyS32 m_boundsMinY;          // 0x5c
		TonyS32 m_boundsMaxX;          // 0x60
		TonyS32 m_boundsMaxY;          // 0x64
		TonyS32 m_moveViaParent;       // 0x68 1 = MoveObject forwards to m_parent
		TonyFloat m_pushX;             // 0x6c pending carry/push delta
		TonyFloat m_pushY;             // 0x70
		TonyS32 m_sprite;              // 0x74 scenery/backdrop sprite; group: child count
		TonyS32 m_frameSet;            // 0x78 text: align flags; group: child list base
		TonyS32 m_prevFrameSet;        // 0x7c text: TextLabel*
		TonyS32 m_frame;               // 0x80
		TonyS32 m_prevFrame;           // 0x84
		TonyFloat m_frameTime;         // 0x88 ticks left on the current frame
		TonyFloat m_animSpeed;         // 0x8c 0 = paused
		TonyS32 m_nextFrameSet;        // 0x90 queued follow-up set (-1 none)
		TonyS32 m_frameLatch;          // 0x94 1 = latch prev set/frame this tick
		TonyS32 m_typeFlags; // 0x98 enemy: bit0 stomp-hardened; item: settled; trigger: retrigger timer; speech:
							 // OverlayData*
		TonyS32 m_behavior;  // 0x9c enemy behavior state; player: jump-boost ticks; camera script: rider mask; speech:
							 // bubble object
		TonyS32 m_cooldown;  // 0xa0 enemy hit grace; player: duck-look timer; camera script: rider slots base
		TonyS32 m_moveState; // 0xa4 player move state (1 idle 2 walk 3 jump 4 fall 5 bounce 6 die 7 pose 8 duck 10 swim
							 // 11 surf 12 hang 13 drown); enemy: flash-invuln ticks
		TonyS32
			m_pendingState; // 0xa8 player: requested move state; enemy: advance-patrol flag; speech: mouth ring base
		TonyS32
			m_patrolStep; // 0xac boss: script cursor; mover: travel left; hopper: rest ticks; session: pickup callback
		void(__fastcall*
				 m_touchFn)(GameObject*, GameObject*, TonyS32); // 0xb0 touched-by handler; dispenser: half frame count
		TonyS32 m_cereals;                                      // 0xb4 rolls over at 50; boss: bubble OverlayData*
		TonyS32 m_lives;                                        // 0xb8
		TonyS32 m_reserved1;                                    // 0xbc zeroed with the counters, never read
		HitBox* m_groundBox;                                    // 0xc0 current ground line under the object
		HitResponseFn m_hitFrameFn;                             // 0xc4 swept map-box response
		HitResponseFn m_hitWorldFn;                             // 0xc8 ground-line response
		TonyS32 m_mouthStep;                                    // 0xcc speech: mouth frame ring cursor
		TonyS32 m_savedRound;                                   // 0xd0 saved Camera::m_round
		Camera* m_savedCamera;                                  // 0xd4 session destroy: leftover OverlayData*
		TonyFloat m_savedX;                                     // 0xd8
		TonyFloat m_savedY;                                     // 0xdc
		TonyS32 m_savedSpawn;             // 0xe0 saved ObjectManager::m_spawnPoint; camera script: shake phase
		TonyS32 m_surfSound;              // 0xe4 looped surf-spray handle; camera script: command cursor
		TonyS32 m_savedSong;              // 0xe8 song to resume after the surf jingle; camera script: packed command
		TonyS32 m_jingleActive;           // 0xec
		TonyS32 m_dropTicks;              // 0xf0 rail regrab lockout after dropping through
		TonyS32 m_health;                 // 0xf4 nutrition meter 0-3
		TonyS32 m_meterSet3;              // 0xf8 health-bar frame sets by health value
		TonyS32 m_meterSet2;              // 0xfc
		TonyS32 m_meterSet1;              // 0x100 speech: saved ObjectManager::m_drawMode
		OverlayData* m_livesData;         // 0x104 HUD widget pairs (spawn record + object)
		GameObject* m_livesText;          // 0x108
		OverlayData* m_portraitData;      // 0x10c
		GameObject* m_portrait;           // 0x110
		OverlayData* m_livesGroupData;    // 0x114
		GameObject* m_livesGroup;         // 0x118
		OverlayData* m_cerealData;        // 0x11c
		GameObject* m_cerealText;         // 0x120
		OverlayData* m_cerealIconData;    // 0x124
		GameObject* m_cerealIcon;         // 0x128
		OverlayData* m_cerealGroupData;   // 0x12c
		GameObject* m_cerealGroup;        // 0x130
		OverlayData* m_popupData;         // 0x134
		GameObject* m_popupIcon;          // 0x138
		OverlayData* m_popupGroupData;    // 0x13c
		GameObject* m_popupGroup;         // 0x140
		OverlayData* m_nutrientData;      // 0x144
		void* m_nutrientGauge;            // 0x148
		OverlayData* m_healthBarData;     // 0x14c
		GameObject* m_healthBar;          // 0x150
		OverlayData* m_keyData;           // 0x154
		GameObject* m_keyGauge;           // 0x158
		OverlayData* m_timerData;         // 0x15c
		GameObject* m_bonusTimer;         // 0x160
		TonyS32 m_livesShowTicks;         // 0x164 HUD slide-away timers
		TonyS32 m_cerealShowTicks;        // 0x168
		TonyS32 m_popupShowTicks;         // 0x16c
		TonyS32 m_hudCerealSet;           // 0x170 level-specific cereal icon frame set
		TonyS32 m_hudExtraSet;            // 0x174 second level-loaded HUD frame set
		TonyS32 m_shieldTicks;            // 0x178 collect-kind-3 invulnerability
		TonyS32 m_cerealsLevel;           // 0x17c cereals collected this level
		TonyS32 m_bonusTicks;             // 0x180 bonus-level countdown
		TonyS32 m_respawnForm;            // 0x184
		TonyS32 m_checkpointKeys;         // 0x188 checkpoint snapshot
		TonyS32 m_checkpointNutrients;    // 0x18c
		TonyS32 m_checkpointCerealsLevel; // 0x190
		TonyS32 m_checkpointCereals;      // 0x194
	};

	GameObject();

	void ApplyGravity(TonyFloat p_dx, TonyFloat p_dy, TonyFloat p_x, TonyFloat p_y);
	void Decelerate(TonyFloat p_dx, TonyFloat p_dy, TonyFloat p_x, TonyFloat p_y);
	void SetVelocity(TonyFloat p_x, TonyFloat p_y);
	void Teleport(TonyFloat p_x, TonyFloat p_y);
	void Translate(TonyFloat p_dx, TonyFloat p_dy);
	void ResolveFrameSet();
	void ResetAnimation();
	void AdvanceAnimation();
	void FreeDataBlock();
	void Clear();
	void Suspend();
	void Resume();

	ObjectTemplate::Head* m_head;                               // 0x00
	ObjectTemplate::Ext* m_ext;                                 // 0x04
	State* m_state;                                             // 0x08
	void(__fastcall* m_initFn)(GameObject*, ObjectTemplate*);   // 0x0c
	void(__fastcall* m_reinitFn)(GameObject*, ObjectTemplate*); // 0x10
	TonyS32(__fastcall* m_tickFn)(GameObject*);                 // 0x14
	void(__fastcall* m_drawFn)(GameObject*);                    // 0x18
	void(__fastcall* m_destroyFn)(GameObject*);                 // 0x1c
	GameObject* m_next;                                         // 0x20
	GameObject* m_prev;                                         // 0x24
	TonyS32 m_suspendCount;                                     // 0x28
};

DECOMP_SIZE_ASSERT(GameObject, 0x2c)

struct HitBox;

void __fastcall CollideWithHitList(
	GameObject* p_object,
	HitResult* p_list,
	TonyS32 p_count,
	void(__fastcall* p_callback)(GameObject*, GameObject*, HitBox*, HitBox*, TonyS32)
);
void __fastcall CollideWithMapBoxes(GameObject* p_object, HitResponseFn p_callback);
TonyFloat __fastcall BouncerHitFrame(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value);
TonyFloat __fastcall BouncerHitWorld(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value);
TonyFloat __fastcall PlayerHitFrame(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value);
TonyFloat __fastcall PlayerHitWorld(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value);
void __fastcall BouncerTurnAround(GameObject* p_object);
TonyS32 __fastcall EnemyBaseTick(GameObject* p_object);
void __fastcall CollideWithGroundLines(GameObject* p_object, HitResponseFn p_callback, TonyS32 p_dx, TonyS32 p_dy);
void __fastcall SpringTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_extra
);
TonyS32 __fastcall SpringTick(GameObject* p_object);
void __fastcall EnemyTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_extra
);
void __fastcall GetPrevPosition(GameObject* p_object, TonyFloat* p_b, TonyFloat* p_out);
void __fastcall MoveObject(GameObject* p_object, TonyS32 p_mode, TonyFloat p_dx, TonyFloat p_dy);
TonyS32 __fastcall WaterTick(GameObject* p_object);
void __fastcall GetGroundSlope(HitBox* p_frame, TonyFloat* p_slope, TonyS32* p_direction);
void __fastcall TestGroundLines(
	GameObject* p_object,
	TonyS32 p_count,
	HitBox** p_list,
	HitResponseFn p_callback,
	TonyFloat p_dx,
	TonyFloat p_dy
);
TonyS32 __fastcall IsAnimationDone(GameObject* p_object);
TonyS32 __fastcall GetFrameCount(GameObject* p_object);

extern TonyS32 g_bouncerCount;
void __fastcall BouncerInit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall BouncerReset(GameObject* p_object);
void __fastcall EnemyReinit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall EnemyResolveSets(GameObject* p_object);
void __fastcall BouncerDestroy(GameObject* p_object);
TonyS32 __fastcall BouncerTick(GameObject* p_object);
void __fastcall BouncerWalk(GameObject* p_object);
void __fastcall EnemyDeathTick(GameObject* p_object);
void __fastcall BouncerApplyFacing(GameObject* p_object);
void __fastcall StaticEnemyInit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall StaticEnemyReinit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall BindPlayerTemplate(GameObject* p_object, PlayerTemplate* p_template);
void __fastcall PlayerInit(GameObject* p_object, PlayerTemplate* p_template);
void __fastcall PlayerResolveSets(GameObject* p_object);
TonyS32 __fastcall PlayerTick(GameObject* p_object);
void __fastcall PlayerActivate(GameObject* p_object);
void __fastcall PlayerClearCounters(GameObject* p_object);
TonyS32 __fastcall SetPlayerState(GameObject* p_object, TonyS32 p_state, TonyS32 p_soft);
void __fastcall PlayerFall(GameObject* p_object);
void __fastcall PlayerIdle(GameObject* p_object);
void __fastcall PlayerWalk(GameObject* p_object);
void __fastcall PlayerJump(GameObject* p_object);
void __fastcall PlayerBounce(GameObject* p_object);
void __fastcall PlayerDie(GameObject* p_object);
void __fastcall PlayerDrown(GameObject* p_object);
void __fastcall PlayerCelebrate(GameObject* p_object);
void __fastcall PlayerDuck(GameObject* p_object);
void __fastcall PlayerSwim(GameObject* p_object);
void __fastcall PlayerSurf(GameObject* p_object);
Camera* __fastcall GetObjectCamera(GameObject* p_object);
void __fastcall StoreObjectCamera(GameObject* p_object, Camera* p_camera);
void __fastcall PlayerRequestState(GameObject* p_object, TonyS32 p_state);
void __fastcall DropItemInit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall DropItemReset(GameObject* p_object);
TonyS32 __fastcall DropItemTick(GameObject* p_object);
TonyFloat __fastcall DropItemHitFrame(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value);
TonyFloat __fastcall DropItemHitWorld(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value);
void __fastcall SpringInit(GameObject* p_object, ObjectTemplate* p_template);
TonyS32 __fastcall CollectibleTick(GameObject* p_object);
void __fastcall CritterInit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall CritterReinit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall CritterResolveSets(GameObject* p_object);
TonyS32 __fastcall CritterTick(GameObject* p_object);
void __fastcall CritterReset(GameObject* p_object);
void __fastcall CharSelectInit(GameObject* p_object, ObjectTemplate* p_template);
TonyS32 __fastcall CharSelectTick(GameObject* p_object);
void __fastcall CharSelectTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_extra
);
void __fastcall ShowCharacterSelect(GameObject* p_object);
void __fastcall SpeechReinit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall SpeechResolveSets(GameObject* p_object);
void __fastcall SpeechReset(GameObject* p_object);
TonyS32 __fastcall SpeechTick(GameObject* p_object);
void __fastcall SpeechDestroy(GameObject* p_object);
void __fastcall SpeechTriggerInit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall SpeechTriggerReset(GameObject* p_object);
TonyS32 __fastcall SpeechTriggerTick(GameObject* p_object);
void __fastcall SpeechTriggerTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_extra
);
void __fastcall DoorInit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall DoorReset(GameObject* p_object);
TonyS32 __fastcall DoorTick(GameObject* p_object);
void __fastcall DoorTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_extra
);
TonyS32 __fastcall GetSegmentMask(GameObject* p_object);
void __fastcall GoalInit(GameObject* p_object, ObjectTemplate* p_template);
TonyS32 __fastcall GoalTick(GameObject* p_object);
void __fastcall GoalTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_extra
);
void __fastcall BossInit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall BossReset(GameObject* p_object);
void __fastcall BossReinit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall BossResolveSets(GameObject* p_object);
void __fastcall BossDestroy(GameObject* p_object);
TonyS32 __fastcall BossTick(GameObject* p_object);
void __fastcall BossWalk(GameObject* p_object);
void __fastcall BossDie(GameObject* p_object);
void __fastcall BossRunScript(GameObject* p_object);
void __fastcall BindGroupTemplate(GameObject* p_object, GroupTemplate* p_template);
void __fastcall CarrierInit(GameObject* p_object, GroupTemplate* p_template);
void __fastcall GroupInit(GameObject* p_object, GroupTemplate* p_template);
void __fastcall GroupReset(GameObject* p_object);
void __fastcall GroupAddChild(GameObject* p_group, GameObject* p_child);
void __fastcall CollectibleInit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall CollectibleReinit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall CollectibleRegister(GameObject* p_object);
void __fastcall HopperReset(GameObject* p_object);
void __fastcall HopperReinit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall HopperResolveSets(GameObject* p_object);
void __fastcall HopperWalk(GameObject* p_object);
void __fastcall HopperIdle(GameObject* p_object);
TonyFloat __fastcall HopperHitFrame(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value);
void __fastcall HopperTurnAround(GameObject* p_object);
void __fastcall SetHopperState(GameObject* p_object, TonyS32 p_state);
TonyFloat __fastcall HopperHitWorld(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value);
void __fastcall SessionInit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall SessionReinit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall SessionResolveSets(GameObject* p_object);
void __fastcall SessionDestroy(GameObject* p_object);
void __fastcall SessionCreateHud(GameObject* p_object);
TonyS32 __fastcall SessionTick(GameObject* p_object);
void __fastcall SessionPickup(GameObject* p_object, GameObject* p_other, TonyS32 p_kind);
void __fastcall SessionTouch(GameObject* p_object, GameObject* p_other, TonyS32 p_kind);
TonyFloat __fastcall SessionHitWorld(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value);
TonyFloat __fastcall SessionHitFrame(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value);
void __fastcall ConvertCereals(GameObject* p_object);
void __fastcall FlashHealthGain(GameObject* p_object);
void __fastcall PlayerSetForm(GameObject* p_object, TonyS32 p_form, TonyS32 p_force);
void __fastcall RefreshHealthBar(GameObject* p_object);
void __fastcall TakeDamage(GameObject* p_object, TonyS32 p_amount, TonyS32 p_direction, TonyS32 p_state);
void __fastcall LabelGameSlot(GameObject* p_object, TonyS32 p_b);
void __fastcall LabelMusicToggle(GameObject* p_object, TonyS32 p_b);
void __fastcall LabelSmoothToggle(GameObject* p_object, TonyS32 p_b);
void __fastcall LabelSfxToggle(GameObject* p_object, TonyS32 p_b);
void __fastcall LabelWideString(GameObject* p_object, TonyS32 p_b);
void __fastcall LabelLevelNumber(GameObject* p_object, TonyS32 p_b);
void __fastcall RespawnAtCheckpoint(GameObject* p_object);
void __fastcall ShowLevelComplete(GameObject* p_object);
void __fastcall TallyPercent(GameObject* p_object, TonyS32 p_value);
void __fastcall TallyNumber(GameObject* p_object, TonyS32 p_value);
void __fastcall TallyOutOf(GameObject* p_object, TonyS32 p_value);
void __fastcall CounterRefreshSprites(GameObject* p_object);
void __fastcall SetBonusTimer(GameObject* p_object, TonyS32 p_on);
void __fastcall TickBonusTimer(GameObject* p_object);
void __fastcall SetObjectSprite(GameObject* p_object, TonyS32 p_variant, TonyS32 p_c);
void __fastcall WorldMapInit(GameObject* p_object, PlayerTemplate* p_template);
void __fastcall WorldMapReinit(GameObject* p_object, PlayerTemplate* p_template);
void __fastcall WorldMapStartMusic(GameObject* p_object);
void __fastcall WorldMapPlace(GameObject* p_object);
TonyS32 __fastcall WorldMapTick(GameObject* p_object);
void __fastcall WorldMapDraw(GameObject* p_object);
void __fastcall WorldMapDestroy(GameObject* p_object);
void __fastcall ProbeCeilingRail(GameObject* p_object);
TonyFloat __fastcall GrabCeilingRail(GameObject* p_object, HitBox* p_frame, TonyS32 p_kind, TonyFloat p_value);
void __fastcall SaveCheckpointCounters(GameObject* p_object);
void __fastcall RestoreCheckpointCounters(GameObject* p_object);
void __fastcall SetSegmentSprites(GameObject* p_object, TonyS32 p_slot, TonyS32 p_sound, TonyS32 p_d);
void __fastcall PlayerReset(GameObject* p_object, TonyS32 p_fresh);
void __fastcall StoreRespawnForm(GameObject* p_object);
void FormatObjectText(GameObject* p_object, TonyS32 p_kind, ...);
TonyS32 __fastcall GroupTick(GameObject* p_object);
void __fastcall SetBossState(GameObject* p_object, TonyS32 p_state);
void __fastcall BossRestartScript(GameObject* p_object);
void __fastcall BossStand(GameObject* p_object);
void __fastcall BossCharge(GameObject* p_object);
void __fastcall CollectibleTouch(
	GameObject* p_object,
	GameObject* p_other,
	HitBox* p_own,
	HitBox* p_otherFrame,
	TonyS32 p_e
);
void __fastcall PlayerHang(GameObject* p_object);
void __fastcall EnemyReset(GameObject* p_object);
void __fastcall EnemyResolveBaseSets(GameObject* p_object);
void __fastcall SetEnemyState(GameObject* p_object, TonyS32 p_state);
void __fastcall ResetObjectAnimation(GameObject* p_object);
void __fastcall BindCounterTemplate(GameObject* p_object, CounterTemplate* p_template);
TonyS32 IsSideContactBR();
TonyS32 IsSideContactTR();
TonyS32 IsSideContactBL();
TonyS32 IsSideContactTL();
TonyS32 __fastcall ResolveBoxContact(
	GameObject* p_object,
	HitBox* p_frame,
	HitBox* p_otherFrame,
	HitBox* p_own,
	HitResponseFn p_callback
);
void __fastcall CollectHitBoxes(GameObject* p_object, HitResult* p_out, TonyS32* p_count, TonyS32 p_mask);
void __fastcall CollectFrameHitBoxes(
	GameObject* p_object,
	TonyS32 p_frameSet,
	TonyS32 p_frame,
	HitResult* p_out,
	TonyS32* p_count,
	TonyS32 p_mask
);
void __fastcall SpriteInit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall ResolveTemplateFrameSet(GameObject* p_object, ObjectTemplate* p_template);
TonyS32 __fastcall SpriteTick(GameObject* p_object);
void __fastcall SpriteDraw(GameObject* p_object);
void __fastcall SetFrameSet(GameObject* p_object, TonyS32 p_frameSet);
void __fastcall QueueFrameSet(GameObject* p_object, TonyS32 p_frameSet);
void __fastcall RefreshHitBoxes(GameObject* p_object);
void __fastcall SpriteDestroy(GameObject* p_object);

struct HitBox;

TonyS32 __fastcall MoveTick(GameObject* p_object);
void __fastcall ResetMotion(GameObject* p_object);
void __fastcall UpdateWorldPosition(GameObject* p_object);
TonyBool32 __fastcall HitBoxesOverlap(HitBox* p_a, HitBox* p_b);
void __fastcall CullIfOffscreen(GameObject* p_object);
void __fastcall SnapshotPosition(GameObject* p_object);
void __fastcall ClearPush(GameObject* p_object);
void __fastcall QueueObjectSprite(
	GameObject* p_object,
	TonyS32 p_sprite,
	TonyFloat p_x,
	TonyFloat p_y,
	TonyS32 p_layer,
	TonyS32 p_e
);
void __fastcall GetDrawPosition(GameObject* p_object, TonyFloat* p_xOffset, TonyFloat* p_yOffset);
TonyS32 __fastcall SceneryTick(GameObject* p_object);
void __fastcall BannerInit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall PlayerReinit(GameObject* p_object, PlayerTemplate* p_template);
void __fastcall SpeechInit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall HopperInit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall PropInit(GameObject* p_object, PlayerTemplate* p_template);
void __fastcall SegmentDisplayInit(GameObject* p_object, PlayerTemplate* p_template);
void __fastcall SegmentDisplayReinit(GameObject* p_object, PlayerTemplate* p_template);
void __fastcall SplashInit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall PlatformInit(GameObject* p_object, PlayerTemplate* p_template);
void __fastcall PlatformReinit(GameObject* p_object, PlayerTemplate* p_template);
void __fastcall ScreenTileInit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall SceneryInit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall TextInit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall TextReinit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall DispenserInit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall DispenserReinit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall BackdropInit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall BackdropDraw(GameObject* p_object);
void __fastcall TextDestroy(GameObject* p_object);
TonyS32 __fastcall TextTick(GameObject* p_object);
void __fastcall TextLoadFont(GameObject* p_object);
void __fastcall TextCreateLabel(GameObject* p_object);
void __fastcall TextDraw(GameObject* p_object);
void __fastcall TextMeasure(GameObject* p_object);
void __fastcall DispenserSpawn(GameObject* p_object);
void __fastcall DispenserIdle(GameObject* p_object);
void __fastcall MoverAdvance(GameObject* p_object);
void __fastcall MoverReverse(GameObject* p_object, TonyFloat p_amount);
void __fastcall SetMoverMode(GameObject* p_object, TonyS32 p_mode);
void __fastcall SetDispenserMode(GameObject* p_object, TonyS32 p_mode);
void __fastcall BindOverlayTemplate(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall ScreenTileDraw(GameObject* p_object);
TonyS32 __fastcall SegmentDisplayTick(GameObject* p_object);
void __fastcall SegmentDisplayDraw(GameObject* p_object);
void __fastcall SegmentDisplayDestroy(GameObject* p_object);
void __fastcall SegmentDisplaySetup(GameObject* p_object);
TonyS32 __fastcall SplashTick(GameObject* p_object);
void __fastcall SplashSetup(GameObject* p_object);
void __fastcall SplashDestroy(GameObject* p_object);
void __fastcall PlatformPreload(GameObject* p_object);
TonyS32 __fastcall PlatformTick(GameObject* p_object);
void __fastcall PlatformSetup(GameObject* p_object);
void __fastcall StopCameraScript(GameObject* p_object);
TonyS32 __fastcall HopperTick(GameObject* p_object);
TonyS32 __fastcall DispenserTick(GameObject* p_object);
void __fastcall DispenserReset(GameObject* p_object);
void __fastcall DispenserResolveSets(GameObject* p_object);
TonyS32 __fastcall MoverTick(GameObject* p_object);
void __fastcall MoverReset(GameObject* p_object);
void __fastcall WaterDraw(GameObject* p_object);
void __fastcall WaterDetach(GameObject* p_object);
void __fastcall WaterReset(GameObject* p_object);
void __fastcall WaterAttach(GameObject* p_object);
void __fastcall CollectRiders(GameObject* p_object);
void __fastcall SnapRiders(GameObject* p_object, HitResult* p_buffer, TonyS32 p_count);
void __fastcall ReleaseRiders(GameObject* p_object);
void __fastcall PushRiders(GameObject* p_object);
void __fastcall BobTick(GameObject* p_object);
void __fastcall RunCameraScript(GameObject* p_object);
void __fastcall SceneryReinit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall MoverInit(GameObject* p_object, CounterTemplate* p_template);
void __fastcall WaterInit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall WaterReinit(GameObject* p_object, ObjectTemplate* p_template);
void __fastcall BannerDraw(GameObject* p_object);
void __fastcall InitMotion(GameObject* p_object);
void __fastcall CheckpointInit(GameObject* p_object, GroupTemplate* p_template);
void __fastcall CheckpointReinit(GameObject* p_object, GroupTemplate* p_template);
void __fastcall CheckpointRegister(GameObject* p_object);
void __fastcall CheckpointDestroy(GameObject* p_object);
TonyS32 __fastcall NullObjectHandler(GameObject* p_object);
void __fastcall SceneryResolveSprite(GameObject* p_object);
void __fastcall SceneryDestroy(GameObject* p_object);
void __fastcall SceneryDraw(GameObject* p_object);
void __fastcall InitMotion(GameObject* p_object);
void __fastcall BindTemplate(GameObject* p_object, ObjectTemplate* p_template);

#endif // GAMEOBJECT_H
