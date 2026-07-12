#ifndef DIALOGBUILDER_H
#define DIALOGBUILDER_H

#include "decomp.h"
#include "gameobject.h"
#include "types.h"

// Stack-allocated dialog/menu screen builder. Build (0x411d30) spawns one
// type-5 text object per DialogItem spec (m_objects[i] plus its malloc'd 0x1f4
// OverlayData in m_overlays[i]); specs with kind bit 2 also get a right-aligned
// type-8 value label, stored mirrored from the top of both arrays (index
// 0x1f - i). Run (0x411f30) polls input, moves the highlight across selectable
// items and returns the confirmed item index (-1 on cancel). Hide/Show
// (0x412250/0x412270) tear down and rebuild from the saved specs while a
// sub-screen runs; SetBackdrop (0x412020) spawns a type-8 full-screen sprite at
// layer 0xe and remembers its sprite cache slot in m_backdropSlot.
// SIZE 0x124
class DialogBuilder {
public:
	// Per-widget spec passed to Build (0x411d30). m_kind bits: 1 = selectable,
	// 2 = centered-title text style, 4 = add a right-aligned value label
	// (sprite id taken from m_tag), 8 = second text style bit. m_stringId is
	// the localized text line; m_tag doubles as action tag (e.g. 0x4711 =
	// back/exit), value sprite id and callback argument; m_callback runs after
	// the item object is (re)built. m_flags feeds the text object's style
	// field (OverlayData+0x28).
	// SIZE 0x1c
	struct DialogItem {
		TonyS32 m_kind;                                     // 0x00
		TonyS32 m_x;                                        // 0x04
		TonyS32 m_y;                                        // 0x08
		TonyS32 m_flags;                                    // 0x0c
		TonyS32 m_stringId;                                 // 0x10
		TonyS32 m_tag;                                      // 0x14
		void(__fastcall* m_callback)(GameObject*, TonyS32); // 0x18
	};

	DialogBuilder();
	~DialogBuilder();

	void Init();
	void Teardown();
	void Build(DialogItem* p_specs, TonyS32 p_count);
	TonyS32 Run();
	void SetBackdrop(TonyS32 p_sprite);
	void ClearBackdrop();
	void SetItemHighlight(TonyS32 p_index, TonyS32 p_on);
	void SelectNext(TonyS32 p_playSound);
	void SelectPrev(TonyS32 p_playSound);
	TonyS32 GetItemTag(TonyS32 p_index);
	void Hide();
	void Show();
	void RefreshItem(TonyS32 p_index);
	void SetSelection(TonyS32 p_index);
	void Present();

	GameObject* m_objects[0x20];    // 0x00
	OverlayData* m_overlays[0x20];  // 0x80
	GameObject* m_backdropObject;   // 0x100
	OverlayData* m_backdropOverlay; // 0x104
	DialogItem* m_specs;            // 0x108
	TonyS32 m_itemCount;            // 0x10c
	TonyS32 m_backdropSlot;         // 0x110
	TonyS32 m_selection;            // 0x114
	TonyS32 m_backdropSprite;       // 0x118
	DialogItem* m_hiddenSpecs;      // 0x11c
	TonyS32 m_hiddenCount;          // 0x120
};

DECOMP_SIZE_ASSERT(DialogBuilder, 0x124)

#endif // DIALOGBUILDER_H
