// RLE sprite blitters. Kept in a minimal TU: BlitSprite's codegen is sensitive to
// whole-TU content (VC5 re-canonicalizes on unrelated header changes), so this file
// must not include gameobject.h or other frequently-edited headers.
#include "decomp.h"
#include "types.h"

// TU-local view of BackgroundRenderer::RleRow. Kept self-contained on purpose:
// BlitSprite's codegen re-canonicalizes on unrelated header edits, so this file
// must not include shared game headers.
// SIZE 0x0c
struct SpriteRow {
	void* m_block; // 0x00
	TonyS32 m_x;   // 0x04
	TonyS32 m_row; // 0x08
};

#include <ddraw.h>

extern DDSURFACEDESC g_surfaceDesc;

void __fastcall BlitSpriteFast(TonyU16* p_sprite, TonyS32 p_x, TonyS32 p_y, TonyS32 p_arg, TonyU8* p_surface);
void __fastcall BlitSpriteClipY(
	TonyU16* p_sprite,
	void* p_rows,
	TonyS32 p_x,
	TonyS32 p_y,
	TonyS32 p_arg,
	TonyU8* p_surface
);
void __fastcall BlitSpriteClipXY(
	TonyU16* p_sprite,
	void* p_rows,
	TonyS32 p_x,
	TonyS32 p_y,
	TonyS32 p_arg,
	TonyU8* p_surface
);

// FUNCTION: TONY2 0x004074e0
void __fastcall BlitSprite(TonyU16* p_sprite, void* p_rows, TonyS32 p_x, TonyS32 p_y, TonyS32 p_arg, TonyU8* p_surface)
{
	if ((p_x < 0 || p_x >= 0x280) && (p_x + p_sprite[0] <= 0 || p_x + p_sprite[0] > 0x280)) {
		return;
	}

	if ((p_y < 0 || p_y >= 0x190) && (p_y + p_sprite[1] <= 0 || p_y + p_sprite[1] > 0x190)) {
		return;
	}

	if (p_x >= 0 && p_x + p_sprite[0] <= 0x280 && p_y >= 0 && p_y + p_sprite[1] <= 0x190) {
		BlitSpriteFast(p_sprite, p_x, p_y, p_arg, p_surface);
	}
	else if (p_x >= 0 && p_x + p_sprite[0] <= 0x280) {
		BlitSpriteClipY(p_sprite, p_rows, p_x, p_y, p_arg, p_surface);
	}
	else {
		BlitSpriteClipXY(p_sprite, p_rows, p_x, p_y, p_arg, p_surface);
	}
}

// FUNCTION: TONY2 0x004075c0
void __fastcall BlitSpriteFast(TonyU16* p_sprite, TonyS32 p_x, TonyS32 p_y, TonyS32 p_arg, TonyU8* p_surface)
{
	TonyU8* data;
	TonyS32 x = p_x;

	data = (TonyU8*) p_sprite + 0x10;

	__asm {
		push es
		mov ax, ds
		mov es, ax
		mov eax, g_surfaceDesc.lPitch
		mov edx, p_y
		imul edx
		add eax, p_surface
		add eax, x
		add eax, x
		mov edi, eax
		mov esi, data
	row:
		cmp dword ptr [esi], -1
		je done
		lodsd
		and eax, 0x7fffffff
		add edi, eax
		lodsd
		mov ecx, esi
		and ecx, 3
		cmp ecx, eax
		jl split
		mov ecx, eax
		rep movsb
		jmp next
	split:
		sub eax, ecx
		rep movsb
		mov ecx, eax
		shr ecx, 2
		rep movsd
		mov ecx, eax
		and ecx, 3
		rep movsb
	next:
		jmp row
	done:
		pop es
	}
}

// Fully implemented (C + faithful inline asm), kept as STUB because it compares at 93%:
// the row clipping, RLE copy loop and segment setup match, but the recompile homes the
// data pointer at [ebp-4] and the end pointer at [ebp-8] where the original swaps them
// (no declaration order changes it - first-use allocation runs the other way in the
// original build), plus one lea operand-order flip. Same allocator-direction family as
// JoystickEnumCallback (0x405430). Re-annotate as FUNCTION when solved.
// STUB: TONY2 0x00407630
void __fastcall BlitSpriteClipY(
	TonyU16* p_sprite,
	void* p_rows,
	TonyS32 p_x,
	TonyS32 p_y,
	TonyS32 p_arg,
	TonyU8* p_surface
)
{
	SpriteRow* rows = (SpriteRow*) p_rows;
	TonyU8* end;
	TonyU8* data;
	TonyS32 y0 = p_y;
	TonyS32 count;

	if (p_y < 0) {
		data = (TonyU8*) rows[-p_y].m_block;

		if (!data) {
			return;
		}

		p_x += rows[-p_y].m_x;
		p_y += rows[-p_y].m_row;
	}
	else {
		data = (TonyU8*) rows[0].m_block;
		p_x += rows[0].m_x;
		p_y += rows[0].m_row;
	}

	if (p_y >= 0x190) {
		return;
	}

	count = p_sprite[1];

	if (count + y0 >= 0x190) {
		count = 0x190 - y0;
	}

	end = (TonyU8*) rows[count].m_block;

	__asm {
		push es
		mov ax, ds
		mov es, ax
		mov eax, g_surfaceDesc.lPitch
		mov edx, p_y
		imul edx
		add eax, p_surface
		add eax, p_x
		add eax, p_x
		mov edi, eax
		mov esi, data
		add esi, 4
		jmp first
	row:
		cmp esi, end
		je done
		cmp dword ptr [esi], -1
		je done
		lodsd
		and eax, 0x7fffffff
		add edi, eax
	first:
		lodsd
		mov ecx, edi
		and ecx, 3
		cmp ecx, eax
		jl split
		mov ecx, eax
		rep movsb
		jmp next
	split:
		sub eax, ecx
		rep movsb
		mov ecx, eax
		shr ecx, 2
		rep movsd
		mov ecx, eax
		and ecx, 3
		rep movsb
	next:
		jmp row
	done:
		pop es
	}
}

// Fully implemented (C + faithful inline asm), kept as STUB because it compares
// at 47%: the branchless clamps (setg/dec/and), run pre-scan, both clipped copy
// blocks and the row loop all match semantically, but the whole C head schedules
// differently - the original computes each clamp sequentially with the rows
// pointer spilled at [ebp-0x18] where SP3 interleaves the statements, picks
// mirrored registers and spills at [ebp-0x20]. Same allocator-direction family
// as BlitSpriteClipY. Re-annotate as FUNCTION when solved.
// STUB: TONY2 0x00407710
void __fastcall BlitSpriteClipXY(
	TonyU16* p_sprite,
	void* p_rows,
	TonyS32 p_x,
	TonyS32 p_y,
	TonyS32 p_arg,
	TonyU8* p_surface
)
{
	TonyS32 x;
	TonyS32 y;
	TonyS32 end;
	TonyU8* data;
	TonyS32 yLast;
	SpriteRow* rows = (SpriteRow*) p_rows;
	TonyS32 trim;
	TonyS32 clip;

	clip = -((p_x > 0) ? 0 : p_x) * 2;
	end = min(0x27f - p_x, p_sprite[0] - 1) * 2 + 2;
	y = -((p_y > 0) ? 0 : p_y);
	yLast = min(0x18f - p_y, p_sprite[1] - 1);

	if (y > yLast) {
		return;
	}

	do {
		data = (TonyU8*) rows[y].m_block;

		if (!data) {
			return;
		}

		x = rows[y].m_x * 2;
		y = rows[y].m_row;

		if (y <= yLast) {
			TonyS32 len = *(TonyS32*) (data + 4);

			if (x + len < clip) {
				do {
					data += len + 8;
					x += len;
					len = *(TonyS32*) data;
					x += len & 0x7fffffff;

					if (len & 0x80000000) {
						x = end + 1;
						break;
					}

					len = *(TonyS32*) (data + 4);
				} while (x + len < clip);
			}

			if (x < end) {
				trim = (clip - x < 0) ? 0 : clip - x;

				__asm {
					push es
					mov ax, ds
					mov es, ax
					mov ebx, 0
					mov eax, g_surfaceDesc.lPitch
					mov edx, p_y
					add edx, y
					imul edx
					add eax, p_surface
					add eax, p_x
					add eax, p_x
					add eax, x
					mov edi, eax
					mov esi, data
					add esi, 4
					lodsd
					mov edx, trim
					add edi, edx
					add esi, edx
					sub eax, edx
					mov edx, x
					add edx, eax
					cmp edx, end
					jle first
					sub edx, end
					sub eax, edx
					mov ebx, 1
				first:
					add x, eax
					mov ecx, edi
					and ecx, 3
					cmp ecx, eax
					jl split
					mov ecx, eax
					rep movsb
					jmp next
				split:
					sub eax, ecx
					rep movsb
					mov ecx, eax
					shr ecx, 2
					rep movsd
					mov ecx, eax
					and ecx, 3
					rep movsb
				next:
					cmp ebx, 0
					jne done
				row:
					lodsd
					bt eax, 0x1f
					jb done
					and eax, 0x7fffffff
					add edi, eax
					add x, eax
					mov edx, x
					cmp edx, end
					jge done
					lodsd
					mov edx, x
					add edx, eax
					cmp edx, end
					jle first2
					sub edx, end
					sub eax, edx
					mov ebx, 1
				first2:
					add x, eax
					mov ecx, edi
					and ecx, 3
					cmp ecx, eax
					jl split2
					mov ecx, eax
					rep movsb
					jmp next2
				split2:
					sub eax, ecx
					rep movsb
					mov ecx, eax
					shr ecx, 2
					rep movsd
					mov ecx, eax
					and ecx, 3
					rep movsb
				next2:
					cmp ebx, 0
					je row
				done:
					pop es
				}
			}
		}

		y++;
	} while (y <= yLast);
}
