#ifndef BACKGROUNDRENDERER_H
#define BACKGROUNDRENDERER_H

#include "decomp.h"
#include "types.h"

// Video/render system. Created in WinMain (0x00410920) with new(0x125fb0) and published
// through g_backgroundRenderer.
// SIZE 0x125fb0
class BackgroundRenderer {
public:
	// Recorded row: RLE data pointer, x offset (pixels) and target row. Consumed by
	// the blitters (BlitSpriteClipY/BlitSpriteClipXY) through DrawNode::m_rows.
	// SIZE 0x0c
	struct RleRow {
		void* m_block; // 0x00
		TonyS32 m_x;   // 0x04
		TonyS32 m_row; // 0x08
	};

	// Arena run record: half-length in pixels, kind (0 = blank, 1 = image data)
	// and the source pointer. Built by EmitSpan/EmitSpriteRowSpans.
	// SIZE 0x08
	struct RowSpan {
		TonyS16 m_length; // 0x00
		TonyS16 m_kind;   // 0x02
		void* m_data;     // 0x04
	};

	// Per-scanline draw pair built by RebuildRowRing.
	// SIZE 0x08
	struct RowWalker {
		void* m_spans;   // 0x00
		TonyS32 m_phase; // 0x04
	};

	// Named buffer pair in m_buffers.
	// SIZE 0x08
	struct BufferSlot {
		TonyS32 m_id;   // 0x00
		void* m_buffer; // 0x04
	};

	// Track record (element [0]'s first field is the recorder arena).
	// SIZE 0x20
	struct ParallaxTrack {
		RleRow* m_rows;     // 0x00
		TonyU16* m_arena;   // 0x04
		TonyS32 m_height;   // 0x08
		TonyS32 m_rowCount; // 0x0c
		TonyS32 m_width;    // 0x10
		TonyFloat m_scale;  // 0x14
		TonyFloat m_speed;  // 0x18
		TonyS32 m_x;        // 0x1c
	};

	// Deferred row-copy job, chained per layer.
	// SIZE 0x10
	struct RowCopy {
		TonyU16* m_dest;   // 0x00
		TonyU16* m_source; // 0x04
		RowCopy* m_next;   // 0x08
		TonyS32 m_count;   // 0x0c
	};

	// Recorded draw call, chained per layer.
	// SIZE 0x10
	struct DrawRecord {
		TonyS32 m_sprite;   // 0x00
		TonyS32 m_x;        // 0x04
		TonyS32 m_y;        // 0x08
		DrawRecord* m_next; // 0x0c
	};

	BackgroundRenderer();

	void Destroy();
	void ResetArenas();
	void RebuildRowRing();
	void ScrollTo(TonyS32 p_x, TonyS32 p_y);
	void FlushRowJobs();
	void BuildMainTrack();
	void RenderLandscape(TonyU8* p_surface);
	void FreeTracks();
	void BuildLandscape();
	void AddParallaxTrack(TonyS32 p_sprite, TonyS32 p_x, TonyFloat p_scale, TonyFloat p_speed, TonyS32 p_flag);
	void RecordDraw(TonyS32 p_sprite, TonyS32 p_x, TonyS32 p_y, TonyS32 p_layer);

	TonyS32 m_scrollX;                          // 0x00
	TonyS32 m_scrollY;                          // 0x04
	void* m_canvas;                             // 0x08
	TonyS32 m_bufferCount;                      // 0x0c
	BufferSlot* m_buffers;                      // 0x10
	TonyS32 m_rowHead;                          // 0x14
	RowWalker m_rowRing[0x190];                 // 0x18
	undefined m_spanArena[0x4b18 - 0xc98];      // 0xc98
	void* m_spanCursor;                         // 0x4b18
	TonyS32 m_rowStamps[0x280];                 // 0x4b1c
	undefined m_trackArena[0x5f1c - 0x551c];    // 0x551c
	void* m_trackCursor;                        // 0x5f1c
	undefined m_pad0[0x5e3a4 - 0x5f20];         // 0x5f20
	undefined m_jobArena[0x1218a4 - 0x5e3a4];   // 0x5e3a4
	undefined* m_jobCursor;                     // 0x1218a4
	TonyS32 m_jobChains[0x100];                 // 0x1218a8
	undefined m_drawArena[0x125b28 - 0x121ca8]; // 0x121ca8
	DrawRecord* m_drawCursor;                   // 0x125b28
	DrawRecord* m_drawChains[0x100];            // 0x125b2c
	ParallaxTrack m_tracks[4];                  // 0x125f2c
	TonyS32 m_trackCount;                       // 0x125fac
};

DECOMP_SIZE_ASSERT(BackgroundRenderer, 0x125fb0)

void __fastcall SortSpansByX(struct RowSegment* p_list, TonyS32 p_count);
void __fastcall ComposeSpans(BackgroundRenderer::RowWalker* p_walkers, TonyS32 p_base, TonyS32 p_limit, TonyU8* p_row);
BackgroundRenderer::RowSpan* __fastcall EmitSpriteRowSpans(
	TonyS32 p_sprite,
	TonyS32 p_row,
	BackgroundRenderer::RowSpan* p_cursor
);
BackgroundRenderer::RowSpan* __fastcall EmitSpan(
	BackgroundRenderer::RowSpan* p_slot,
	void* p_data,
	TonyS16 p_length,
	TonyS16 p_kind
);

extern BackgroundRenderer* g_backgroundRenderer;

#endif // BACKGROUNDRENDERER_H
