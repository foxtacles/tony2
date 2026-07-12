#ifndef HITBOX_H
#define HITBOX_H

#include "decomp.h"
#include "types.h"

// Axis-aligned float rectangle (left/top/right/bottom) plus one more field
// (never touched by the intersection test; proven by the stack frame of CullIfOffscreen).
// SIZE 0x14
struct HitBox {
	TonyFloat m_left;   // 0x00
	TonyFloat m_top;    // 0x04
	TonyFloat m_right;  // 0x08
	TonyFloat m_bottom; // 0x0c
	TonyS32 m_kind;     // 0x10
};

DECOMP_SIZE_ASSERT(HitBox, 0x14)

#endif // HITBOX_H
