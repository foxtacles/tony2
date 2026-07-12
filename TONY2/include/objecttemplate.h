#ifndef OBJECTTEMPLATE_H
#define OBJECTTEMPLATE_H

#include "decomp.h"
#include "types.h"

// Template/descriptor for a game object. The first 0x1c bytes (Head) are copied into the
// object's instance data block by BindTemplate; the extension (Ext) is referenced in place.
struct ObjectTemplate {
	// SIZE 0x1c
	struct Head {
		TonyS32 m_type;         // 0x00
		TonyFloat m_x;          // 0x04
		TonyFloat m_y;          // 0x08
		TonyU32 m_flags;        // 0x0c
		undefined m_pad0[0x04]; // 0x10
		TonyS32 m_facing;       // 0x14 direction bits: 1 up, 2 down, 4 left, 8 right
		TonyS32 m_layer;        // 0x18
	};

	// The 0x0c..0x5c slots are the ACTIVE movement/animation parameter bank, named after
	// the player family (its main consumer). Other families reuse the slots: the enemy
	// families keep their walk sets in m_idleSetR/m_walkSetR, death sets in
	// m_walkSetL/m_jumpSetR, touch damage in m_idleSetL and patrol data in
	// m_jumpSetL/m_hurtSetR/m_hurtSetL; the camera-script family reads
	// m_walkSpeed/m_jumpSpeed as its per-tick step. The four per-form banks at
	// 0x60..0x17c (18 params each) are copied into the active bank by PlayerSetForm
	// (0x0040b250) and resolved to frame-set slots by SessionResolveSets (0x00408b90).
	struct Ext {
		TonyS32 m_screenSpace;    // 0x00 draw in screen space (no camera offset)
		TonyFloat m_dragX;        // 0x04
		TonyFloat m_dragY;        // 0x08
		TonyS32 m_idleSetR;       // 0x0c sprite/scenery families: primary frame set or sprite id
		TonyFloat m_walkSpeed;    // 0x10
		TonyFloat m_jumpSpeed;    // 0x14 max vertical speed; enemy bounce/launch impulse
		TonyFloat m_walkAccel;    // 0x18
		TonyFloat m_jumpAccel;    // 0x1c
		TonyS32 m_idleSetL;       // 0x20 enemy families: touch damage value
		TonyS32 m_walkSetR;       // 0x24 enemy families: walk set left
		TonyS32 m_walkSetL;       // 0x28 enemy families: death set right
		TonyS32 m_jumpSetR;       // 0x2c enemy families: death set left
		TonyS32 m_jumpSetL;       // 0x30 mover: patrol range; hopper/dispenser: extra set
		TonyS32 m_fallSetR;       // 0x34
		TonyS32 m_fallSetL;       // 0x38
		TonyS32 m_hurtSetR;       // 0x3c hopper: hop count
		TonyS32 m_hurtSetL;       // 0x40 hopper: rest ticks; boss: extra anim pair
		TonyS32 m_duckSetR;       // 0x44
		TonyS32 m_duckSetL;       // 0x48
		TonyS32 m_riseSetR;       // 0x4c
		TonyS32 m_riseSetL;       // 0x50
		TonyS32 m_poseSet;        // 0x54 goal/celebrate anim (no facing variant)
		TonyS32 m_specialSetR;    // 0x58 swim (Smacks/trio), water surf (Tony), rail hang (Coco)
		TonyS32 m_specialSetL;    // 0x5c
		TonyS32 m_smacksIdleR;    // 0x60 boss: final-boss flag (1 = death completes the level)
		TonyS32 m_smacksIdleL;    // 0x64
		TonyS32 m_smacksWalkR;    // 0x68
		TonyS32 m_smacksWalkL;    // 0x6c
		TonyS32 m_smacksJumpR;    // 0x70
		TonyS32 m_smacksJumpL;    // 0x74
		TonyS32 m_smacksFallR;    // 0x78
		TonyS32 m_smacksFallL;    // 0x7c
		TonyS32 m_smacksPortrait; // 0x80
		TonyS32 m_smacksHurtR;    // 0x84
		TonyS32 m_smacksHurtL;    // 0x88
		TonyS32 m_smacksDuckR;    // 0x8c
		TonyS32 m_smacksDuckL;    // 0x90
		TonyS32 m_smacksRiseR;    // 0x94
		TonyS32 m_smacksRiseL;    // 0x98
		TonyS32 m_smacksPose;     // 0x9c
		TonyS32 m_smacksSpecialR; // 0xa0
		TonyS32 m_smacksSpecialL; // 0xa4
		TonyS32 m_tonyIdleR;      // 0xa8
		TonyS32 m_tonyIdleL;      // 0xac
		TonyS32 m_tonyWalkR;      // 0xb0
		TonyS32 m_tonyWalkL;      // 0xb4
		TonyS32 m_tonyJumpR;      // 0xb8
		TonyS32 m_tonyJumpL;      // 0xbc
		TonyS32 m_tonyFallR;      // 0xc0
		TonyS32 m_tonyFallL;      // 0xc4
		TonyS32 m_tonyPortrait;   // 0xc8
		TonyS32 m_tonyHurtR;      // 0xcc
		TonyS32 m_tonyHurtL;      // 0xd0
		TonyS32 m_tonyDuckR;      // 0xd4
		TonyS32 m_tonyDuckL;      // 0xd8
		TonyS32 m_tonyRiseR;      // 0xdc
		TonyS32 m_tonyRiseL;      // 0xe0
		TonyS32 m_tonyPose;       // 0xe4
		TonyS32 m_tonySpecialR;   // 0xe8
		TonyS32 m_tonySpecialL;   // 0xec
		TonyS32 m_cocoIdleR;      // 0xf0
		TonyS32 m_cocoIdleL;      // 0xf4
		TonyS32 m_cocoWalkR;      // 0xf8
		TonyS32 m_cocoWalkL;      // 0xfc
		TonyS32 m_cocoJumpR;      // 0x100
		TonyS32 m_cocoJumpL;      // 0x104
		TonyS32 m_cocoFallR;      // 0x108
		TonyS32 m_cocoFallL;      // 0x10c
		TonyS32 m_cocoPortrait;   // 0x110
		TonyS32 m_cocoHurtR;      // 0x114
		TonyS32 m_cocoHurtL;      // 0x118
		TonyS32 m_cocoDuckR;      // 0x11c
		TonyS32 m_cocoDuckL;      // 0x120
		TonyS32 m_cocoRiseR;      // 0x124
		TonyS32 m_cocoRiseL;      // 0x128
		TonyS32 m_cocoPose;       // 0x12c
		TonyS32 m_cocoSpecialR;   // 0x130
		TonyS32 m_cocoSpecialL;   // 0x134
		TonyS32 m_trioIdleR;      // 0x138
		TonyS32 m_trioIdleL;      // 0x13c
		TonyS32 m_trioWalkR;      // 0x140
		TonyS32 m_trioWalkL;      // 0x144
		TonyS32 m_trioJumpR;      // 0x148
		TonyS32 m_trioJumpL;      // 0x14c
		TonyS32 m_trioFallR;      // 0x150
		TonyS32 m_trioFallL;      // 0x154
		TonyS32 m_trioPortrait;   // 0x158
		TonyS32 m_trioHurtR;      // 0x15c
		TonyS32 m_trioHurtL;      // 0x160
		TonyS32 m_trioDuckR;      // 0x164
		TonyS32 m_trioDuckL;      // 0x168
		TonyS32 m_trioRiseR;      // 0x16c
		TonyS32 m_trioRiseL;      // 0x170
		TonyS32 m_trioPose;       // 0x174
		TonyS32 m_trioSpecialR;   // 0x178
		TonyS32 m_trioSpecialL;   // 0x17c
		TonyS32 m_trioVariant;    // 0x180 Snap/Crackle/Pop sibling: 0/1/2
	};

	// Alternate Ext view used by the ambient/decor families: the slots that hold
	// movement floats for the player types carry sprite ids and generic parameters here
	// (banner: sprite + horizon rows; door: required nutrient; speech: voice line and
	// mouth frames; checkpoint: world/level/bonus/music/backdrop ids).
	struct SpriteExt {
		TonyS32 m_screenSpace; // 0x00
		TonyS32 m_dragX;       // 0x04
		TonyS32 m_dragY;       // 0x08
		TonyS32 m_sprite;      // 0x0c
		TonyS32 m_param0;      // 0x10
		TonyS32 m_param1;      // 0x14
		TonyS32 m_param2;      // 0x18
		TonyS32 m_param3;      // 0x1c
		TonyS32 m_param4;      // 0x20
	};

	Head m_head; // 0x00
	Ext m_ext;   // 0x1c
};

DECOMP_SIZE_ASSERT(ObjectTemplate::Head, 0x1c)

// Player-family template: same layout as ObjectTemplate but the head carries one extra
// dword and is 0x20 bytes; BindPlayerTemplate (0x00402b60) copies it and places the state
// block at +0x20.
struct PlayerTemplate {
	// SIZE 0x20
	struct Head {
		TonyS32 m_type;         // 0x00
		TonyFloat m_x;          // 0x04
		TonyFloat m_y;          // 0x08
		TonyU32 m_flags;        // 0x0c
		undefined m_pad1[0x04]; // 0x10
		TonyS32 m_facing;       // 0x14
		TonyS32 m_layer;        // 0x18
		TonyS32 m_mapNode;      // 0x1c world-map marker: selected node; player: must be 0 to jump/duck
	};

	Head m_head;               // 0x00
	ObjectTemplate::Ext m_ext; // 0x20
};

DECOMP_SIZE_ASSERT(PlayerTemplate::Head, 0x20)

// Group/container template: the head is 0x18 bytes; BindGroupTemplate copies it and places
// the state block at +0x18.
struct GroupTemplate {
	// SIZE 0x18
	struct Head {
		TonyS32 m_type;         // 0x00
		TonyFloat m_x;          // 0x04
		TonyFloat m_y;          // 0x08
		TonyU32 m_flags;        // 0x0c
		undefined m_pad2[0x04]; // 0x10
		TonyS32 m_facing;       // 0x14
	};

	Head m_head;               // 0x00
	ObjectTemplate::Ext m_ext; // 0x18
};

DECOMP_SIZE_ASSERT(GroupTemplate::Head, 0x18)

// Counter/collectible template: the head carries two extra dwords (score bucket and
// amount) and is 0x24 bytes; BindCounterTemplate copies it and places the state at +0x24.
// The full in-level player (type 0x64) and the enemy families also use this shape.
struct CounterTemplate {
	// SIZE 0x24
	struct Head {
		TonyS32 m_type;         // 0x00
		TonyFloat m_x;          // 0x04
		TonyFloat m_y;          // 0x08
		TonyU32 m_flags;        // 0x0c
		undefined m_pad3[0x04]; // 0x10
		TonyS32 m_facing;       // 0x14
		TonyS32 m_layer;        // 0x18
		TonyS32 m_kind;         // 0x1c collect kind/score bucket (0 cereal, 2 nutrient, 3 shield, 4-6 keys)
		TonyS32 m_value;        // 0x20 item value / enemy hit points / player: current form
	};

	Head m_head;               // 0x00
	ObjectTemplate::Ext m_ext; // 0x24
};

DECOMP_SIZE_ASSERT(CounterTemplate::Head, 0x24)

// Overlay template: the smallest head (0x14 bytes); BindOverlayTemplate copies it and
// places the state block at +0x14.
struct OverlayTemplate {
	// SIZE 0x14
	struct Head {
		TonyS32 m_type;         // 0x00
		TonyFloat m_x;          // 0x04
		TonyFloat m_y;          // 0x08
		TonyU32 m_flags;        // 0x0c
		undefined m_pad4[0x04]; // 0x10
	};

	Head m_head;               // 0x00
	ObjectTemplate::Ext m_ext; // 0x14
};

DECOMP_SIZE_ASSERT(OverlayTemplate::Head, 0x14)

#endif // OBJECTTEMPLATE_H
