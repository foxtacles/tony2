#ifndef DRAWNODE_H
#define DRAWNODE_H

#include "decomp.h"
#include "types.h"

// Draw-list node: 256 layer list heads live at VideoManager::m_layers and the
// free-node stack at m_nodePool (count in m_nodeCount). Filled by QueueSprite and
// linked per layer by LinkDrawNode.
// SIZE 0x1c
class DrawNode {
public:
	DrawNode();

	TonyS32 m_x;        // 0x00
	TonyS32 m_y;        // 0x04
	TonyS32 m_reserved; // 0x08
	DrawNode* m_next;   // 0x0c
	DrawNode* m_prev;   // 0x10
	TonyU16* m_pixels;  // 0x14
	void* m_rows;       // 0x18
};

DECOMP_SIZE_ASSERT(DrawNode, 0x1c)

#endif // DRAWNODE_H
