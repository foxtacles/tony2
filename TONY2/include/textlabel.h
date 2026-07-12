#ifndef TEXTLABEL_H
#define TEXTLABEL_H

#include "decomp.h"
#include "types.h"

#include <afx.h>

// Text/label object referenced from GameObject::State (label slot)
// (size not yet proven). The destructor is implicit: animobject's TU emits the
// compiler-synthesized out-of-line copy (a tail-jump to CString::~CString).

// SYNTHETIC: TONY2 0x00404f70
// TextLabel::~TextLabel

class TextLabel {
public:
	TextLabel();
	TextLabel(TonyS32 p_string);

	void FormatString(TonyS32 p_string, TonyS32 p_arg);
	void SetText(TonyU16* p_text);
	void SetString(TonyS32 p_string);

	CString m_text; // 0x00
};

#endif // TEXTLABEL_H
