// Language, text and file subsystem: the one TU that uses the statically linked
// MFC 4.21 (CFile, CString).

// clang-format off
// gamefile.h pulls in afx.h, which must precede any windows.h inclusion.
#include "gamefile.h"
// clang-format on

#include "engine.h"
#include "gameobject.h"
#include "soundmanager.h"
#include "textlabel.h"
#include "videomanager.h"

#include <afx.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// GameFile::m_backend values (TU-local: keeping these out of gamefile.h leaves the
// header token stream unchanged for the bistable-canonicalization TUs, see camera.cpp
// SpawnOnceAll).
enum GameFileBackend {
	c_backendNone = 0,
	c_backendFile = 1,
	c_backendMemory = 2,
	c_backendArchive = 4,
	c_backendAuto = 5
};

// FUNCTION: TONY2 0x00406e60
TonyS32 VideoManager::LoadSprite(TonyS32 p_key, TonyS32 p_variant)
{
	CString path;

	path.Format("graphics\\gb\\%d.bmp", p_key);
	return LoadSpriteFromFile((char*) (LPCTSTR) path, p_key, p_variant);
}

// FUNCTION: TONY2 0x00406ee0
TonyS32 VideoManager::LoadSpriteFromFile(char* p_path, TonyS32 p_key, TonyS32 p_variant)
{
	GameFile file(p_path, CFile::typeBinary);

	return LoadSpriteFromBmp(&file, p_key, p_variant);
}

// TU-local view of BackgroundRenderer::RleRow for the sprite row table.
// SIZE 0x0c
struct SpriteRow {
	void* m_block; // 0x00
	TonyS32 m_x;   // 0x04
	TonyS32 m_row; // 0x08
};

// BMP loader: reads a 24-bit bottom-up BMP through the unified file object,
// optionally mirrors it (flag 2), and RLE-encodes it into the sprite stream
// format ({surface-delta skip, byte length, RGB565 words} runs per row, -1
// terminated) with a RleRow row table, using the top-left pixel as the
// transparent key and the anchor words repurposed from the PelsPerMeter fields.
// Fully implemented, kept as STUB because the encoder's scratch-slot dance
// compares low under SP3 (this band's register-phase/slot margins); the header,
// mirror, allocation and tail sections align. Refine against the diff or retest
// with the original compiler vintage.
// STUB: TONY2 0x00406f60
TonyS32 VideoManager::LoadSpriteFromBmp(GameFile* p_file, TonyS32 p_key, TonyS32 p_flags)
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	TonyS32 stride;
	TonyU8* pixels;
	TonyU16* sprite;
	SpriteRow* rows;
	TonyU16* cursor;
	TonyU32* block;
	TonyU8* key;
	TonyS32 x;
	TonyS32 row;
	TonyS32 prevX;
	TonyS32 prevRow;
	TonyS32 lastRow;
	TonyS32 done;
	TonyS32 wrapped;
	TonyS32 i;
	TonyS32 slot;

	p_file->Read(&fileHeader, 0xe);
	p_file->Read(&infoHeader, 0x28);
	stride = infoHeader.biWidth * 3 + (4 - infoHeader.biWidth * 3 % 4) % 4;
	p_file->Seek(fileHeader.bfOffBits, 0);
	pixels = (TonyU8*) malloc(stride * infoHeader.biHeight);
	p_file->Read(pixels, stride * infoHeader.biHeight);

	if (p_flags & 2) {
		for (row = 0; row < infoHeader.biHeight; row++) {
			TonyU8* left = pixels + row * stride;
			TonyU8* right = pixels + infoHeader.biWidth * 3 + row * stride - 3;

			for (i = 0; i < infoHeader.biWidth / 2; i++) {
				TonyU16 pair = *(TonyU16*) left;
				TonyU8 high = left[2];

				*(TonyU16*) left = *(TonyU16*) right;
				left[2] = right[2];
				*(TonyU16*) right = pair;
				right[2] = high;
				left += 3;
				right -= 3;
			}
		}
	}

	sprite = (TonyU16*) malloc(infoHeader.biWidth * infoHeader.biHeight / 2 * 10 + 8);
	sprite[0] = (TonyU16) infoHeader.biWidth;
	sprite[1] = (TonyU16) infoHeader.biHeight;
	sprite[2] = (TonyU16) infoHeader.biXPelsPerMeter;
	sprite[3] = (TonyU16) infoHeader.biYPelsPerMeter;
	rows = (SpriteRow*) malloc((infoHeader.biHeight + 1) * 12);
	key = pixels;
	block = (TonyU32*) (sprite + 4);
	cursor = sprite + 8;
	x = 0;
	row = 0;
	prevX = 0;
	prevRow = 0;
	lastRow = -1;
	done = 0;

	do {
		block[0] = 0xffffffff;
		block[1] = 2;

		while (!done && !memcmp(pixels + (sprite[1] - row - 1) * stride + x * 3, key, 3)) {
			x++;

			if (x == infoHeader.biWidth) {
				x = 0;
				row++;

				if (row == infoHeader.biHeight) {
					done = 1;
				}
			}
		}

		if (done) {
			break;
		}

		block[0] = (row - prevRow) * g_surfaceDesc.lPitch + (x - prevX) * 2;

		if (row != prevRow) {
			block[0] |= 0x80000000;
		}

		for (i = lastRow + 1; i <= row; i++) {
			rows[i].m_block = block;
			rows[i].m_x = x;
			rows[i].m_row = row;
		}

		lastRow = row;
		prevX = x;
		prevRow = row;
		wrapped = 0;

		while (!done && !wrapped && memcmp(pixels + (sprite[1] - row - 1) * stride + x * 3, key, 3)) {
			TonyU8* pixel = pixels + (sprite[1] - row - 1) * stride + x * 3;

			*cursor = PackRgb565(*(RgbColor*) pixel);
			cursor++;
			x++;

			if (x == infoHeader.biWidth) {
				block[1] = (row - prevRow) * g_surfaceDesc.lPitch + (x - prevX) * 2;
				prevX = x;
				prevRow = row;
				x = 0;
				row++;
				wrapped = 1;

				if (row == infoHeader.biHeight) {
					done = 1;
				}
			}
		}

		if (!wrapped) {
			block[1] = (row - prevRow) * g_surfaceDesc.lPitch + (x - prevX) * 2;
			prevX = x;
			prevRow = row;
		}

		if (wrapped && !done) {
			continue;
		}

		if (!done) {
			block = (TonyU32*) cursor;
			cursor = (TonyU16*) (block + 2);
		}
	} while (!done);

	for (i = lastRow + 1; i <= sprite[1]; i++) {
		rows[i].m_block = NULL;
		rows[i].m_x = 0;
		rows[i].m_row = 0;
	}

	rows[0].m_x = (TonyS32) ((TonyU8*) cursor - (TonyU8*) rows - 8);
	sprite = (TonyU16*) realloc(sprite, (TonyU8*) cursor - (TonyU8*) sprite);
	slot = AllocSpriteSlot();
	m_sprites[slot] = sprite;
	m_spriteRows[slot] = rows;
	m_spriteSlots[slot].m_key = p_key;
	m_spriteSlots[slot].m_variant = p_flags;
	m_spriteSlots[slot].m_refCount = 0;
	free(pixels);
	return slot;
}

// FUNCTION: TONY2 0x004079f0
TonyS32 VideoManager::LoadFrameSet(TonyS32 p_set, TonyS32 p_flags)
{
	CString path;

	// STRING: TONY2 0x004550b0
	path.Format("graphics\\ga\\%d.ani", p_set);
	return LoadFrameSetFromFile((char*) (LPCTSTR) path, p_set, p_flags);
}

// Fully implemented, kept as STUB because it compares at 71%: every read, allocation,
// remap and mirror block matches, but the allocator homes `this` in ebx / slot in edi /
// flags in ebp where the recompile picks ebp/ebx/edi - the same this-in-ebx seeding as
// TickAll (see objectmanager.cpp). Re-annotate as FUNCTION when solved.
// STUB: TONY2 0x00407a70
TonyS32 VideoManager::LoadFrameSetFromFile(char* p_path, TonyS32 p_set, TonyS32 p_flags)
{
	CString unused;
	GameFile file(p_path, CFile::typeBinary);
	TonyS16 header[3];
	AnimFrame* rec;
	TonyS32 slot;
	TonyS32 frame;
	TonyS32 part;
	TonyS32 mirrorX;
	TonyS32 mirrorY;
	TonyS32 a;
	TonyS32 b;
	TonyS32 i;

	slot = AllocFrameSetSlot();
	file.Read(header, 6);
	m_frameSets[slot] = (AnimFrame*) malloc((header[2] + 1) * 0x1a);
	m_frameSetSlots[slot].m_key = p_set;
	m_frameSetSlots[slot].m_variant = p_flags;
	m_frameSetSlots[slot].m_refCount = 0;

	for (frame = 0; frame < header[2]; frame++) {
		rec = &m_frameSets[slot][frame];
		file.Read(rec, 4);
		rec->m_parts = (FramePart*) malloc(rec->m_partCount * 6);

		if (rec->m_partCount > 0) {
			mirrorX = p_flags & 2;
			mirrorY = p_flags & 1;

			for (part = 0; part < rec->m_partCount; part++) {
				file.Read(&rec->m_parts[part], 6);

				if (IsSpriteLoaded(rec->m_parts[part].m_sprite, p_flags)) {
					rec->m_parts[part].m_sprite = (TonyS16) FindSprite(rec->m_parts[part].m_sprite, p_flags);
				}
				else {
					rec->m_parts[part].m_sprite = (TonyS16) LoadSprite(rec->m_parts[part].m_sprite, p_flags);
				}

				if (mirrorX) {
					rec->m_parts[part].m_dx = -(m_sprites[rec->m_parts[part].m_sprite][0] + rec->m_parts[part].m_dx);
				}

				if (mirrorY) {
					rec->m_parts[part].m_dy = -(m_sprites[rec->m_parts[part].m_sprite][1] + rec->m_parts[part].m_dy);
				}
			}
		}

		file.Read(&rec->m_attachCount, 2);

		if (rec->m_attachCount > 0) {
			rec->m_attachments = (FramePart*) malloc(rec->m_attachCount * 6);

			for (i = 0; i < rec->m_attachCount; i++) {
				file.Read(&rec->m_attachments[i], 6);
				rec->m_attachments[i].m_sprite = (TonyS16) FindFrameSet(rec->m_attachments[i].m_sprite, p_flags);
			}
		}
		else {
			rec->m_attachments = NULL;
		}

		file.Read(&rec->m_hitBoxCount, 2);

		if (rec->m_hitBoxCount > 0) {
			rec->m_hitBoxes = (FrameHitBox*) malloc(rec->m_hitBoxCount * 0x14);

			mirrorY = p_flags & 1;
			mirrorX = p_flags & 2;

			for (i = 0; i < rec->m_hitBoxCount; i++) {
				file.Read(&rec->m_hitBoxes[i], 0x14);

				if (mirrorX) {
					a = rec->m_hitBoxes[i].m_right;
					b = rec->m_hitBoxes[i].m_left;
					rec->m_hitBoxes[i].m_left = -a;
					rec->m_hitBoxes[i].m_right = -b;
				}

				if (mirrorY) {
					a = rec->m_hitBoxes[i].m_bottom;
					b = rec->m_hitBoxes[i].m_top;
					rec->m_hitBoxes[i].m_top = -a;
					rec->m_hitBoxes[i].m_bottom = -b;
				}
			}
		}
		else {
			rec->m_hitBoxes = NULL;
		}

		file.Read(&rec->m_touchBoxCount, 2);

		if (rec->m_touchBoxCount > 0) {
			rec->m_touchBoxes = (FrameHitBox*) malloc(rec->m_touchBoxCount * 0x14);

			mirrorY = p_flags & 1;
			mirrorX = p_flags & 2;

			for (i = 0; i < rec->m_touchBoxCount; i++) {
				file.Read(&rec->m_touchBoxes[i], 0x14);

				if (mirrorX) {
					a = rec->m_touchBoxes[i].m_right;
					b = rec->m_touchBoxes[i].m_left;
					rec->m_touchBoxes[i].m_left = -a;
					rec->m_touchBoxes[i].m_right = -b;
				}

				if (mirrorY) {
					a = rec->m_touchBoxes[i].m_bottom;
					b = rec->m_touchBoxes[i].m_top;
					rec->m_touchBoxes[i].m_top = -a;
					rec->m_touchBoxes[i].m_bottom = -b;
				}
			}
		}
		else {
			rec->m_touchBoxes = NULL;
		}
	}

	m_frameSets[slot][frame].m_duration = -1;
	return slot;
}

// Fully implemented, kept as STUB because it compares at 92%: the round-number switch,
// both CString::Format calls and the sprite rebuilds match, but the original re-reads the
// zero default from its stack slot in the fallback arm where cl 11.00.7022 folds the
// stored zero into xor. Constant-propagation variance (fst-fold family). Re-annotate
// when the vintage is found.
// STUB: TONY2 0x0040c320
void __fastcall CounterRefreshSprites(GameObject* p_object)
{
	CString text;
	TonyS32 def;
	TonyS32 spriteA;
	TonyS32 spriteB;

	def = 0;
	g_videoManager->FreeFrameSet(p_object->m_state->m_hudCerealSet, 1);
	g_videoManager->FreeFrameSet(p_object->m_state->m_hudExtraSet, 1);

	switch (((CounterTemplate::Head*) p_object->m_head)->m_value) {
	case 1:
		spriteA = 0x67;
		spriteB = 0x7a;
		break;
	case 2:
		spriteA = 0x66;
		spriteB = 0x79;
		break;
	case 3:
		spriteA = 0x64;
		spriteB = 0x77;
		break;
	case 4:
		spriteA = 0x65;
		spriteB = 0x78;
		break;
	default:
		spriteA = def;
		spriteB = def;
		break;
	}

	text.Format("graphics\\ga\\%d.ani", spriteA);
	p_object->m_state->m_hudCerealSet = g_videoManager->LoadFrameSetFromFile((char*) (LPCTSTR) text, 0x67, 0);
	g_videoManager->AddRefFrameSet(p_object->m_state->m_hudCerealSet);
	text.Format("graphics\\ga\\%d.ani", spriteB);
	p_object->m_state->m_hudExtraSet = g_videoManager->LoadFrameSetFromFile((char*) (LPCTSTR) text, 0x7a, 0);
	g_videoManager->AddRefFrameSet(p_object->m_state->m_hudExtraSet);
	SetFrameSet(p_object->m_state->m_cerealIcon, p_object->m_state->m_hudCerealSet);
}

// FUNCTION: TONY2 0x00413f70
TonyS32 SoundManager::LoadBanks(char* p_name)
{
	CString path;

	path.Format("%s.proj", p_name);
	ReadFileBlob((char*) (LPCTSTR) path, &m_bankProj);
	path.Format("%s.pool", p_name);
	ReadFileBlob((char*) (LPCTSTR) path, &m_bankPool);
	path.Format("%s.samp", p_name);
	ReadFileBlob((char*) (LPCTSTR) path, &m_bankSamp);
	path.Format("%s.sdir", p_name);
	ReadFileBlob((char*) (LPCTSTR) path, &m_bankSdir);
	return 1;
}

// FUNCTION: TONY2 0x00416130
void __fastcall SetObjectText(GameObject* p_object, TonyS32 p_sprite)
{
	((TextLabel*) p_object->m_state->m_prevFrameSet)->SetString(p_sprite);
}

// FUNCTION: TONY2 0x00416b70
TonyS32 __fastcall ReadFileBlob(char* p_name, void** p_dest)
{
	CFile file(p_name, CFile::modeRead);

	*p_dest = malloc(file.GetLength());
	file.Read(*p_dest, file.GetLength());
	return 1;
}

// FUNCTION: TONY2 0x00416c20
GameFile::GameFile()
{
	SetBackend(c_backendAuto);
}

// FUNCTION: TONY2 0x00416c90
GameFile::GameFile(const char* p_name, UINT p_flags)
{
	SetBackend(c_backendAuto);
	Open(p_name, p_flags, NULL);
}

// FUNCTION: TONY2 0x00416cf0
GameFile::~GameFile()
{
	Close();

	if (m_backend > 0 && m_backend <= 2 && m_innerFile) {
		delete m_innerFile;
	}
}

// FUNCTION: TONY2 0x00416d60
BOOL GameFile::Open(LPCTSTR p_fileName, UINT p_openFlags, CFileException* p_error)
{
	if (m_backend == c_backendAuto) {
		SetBackend(ProbeFileBackend((char*) p_fileName));
	}

	switch (m_backend) {
	case c_backendFile:
		return m_innerFile->Open(p_fileName, p_openFlags, p_error);
	case c_backendArchive:
		m_archiveStream = ArchiveOpen((char*) p_fileName);
		return m_archiveStream != NULL;
	default:
		return 0;
	}
}

// FUNCTION: TONY2 0x00416dc0
void GameFile::SetBackend(TonyS32 p_type)
{
	m_backend = p_type;
	m_innerFile = NULL;

	switch (p_type) {
	case c_backendFile:
		m_innerFile = new CStdioFile();
		break;
	case c_backendMemory:
		m_innerFile = new CMemFile(0x400);
		break;
	}
}

// Fully implemented, kept as STUB because it compares at 68%: the BOM/CR handling, the
// dead-arg-slot scratch char, snapshots and loop shape all match, but the allocator homes
// `this` in ebp and the max snapshot in ebx where the original swaps them. Same
// register-tie family as JoystickEnumCallback (0x405430). Re-annotate as FUNCTION when solved.
// STUB: TONY2 0x00416e50
void GameFile::ReadLine(TonyU16* p_buffer, TonyU32 p_max)
{
	TonyU16* cursor = p_buffer;
	TonyU32 count = 0;
	TonyU32 max = p_max;
	TonyU16 c;

	while (1) {
		if (Read(&c, 2) < 2) {
			break;
		}

		if (c == 0xfeff) {
			continue;
		}

		if (c == 0xd) {
			Read(&c, 2);
			break;
		}

		*cursor = c;
		count++;
		cursor++;

		if (count >= max) {
			break;
		}
	}

	p_buffer[count] = 0;
}

// FUNCTION: TONY2 0x00416eb0
GameFileSize GameFile::GetPosition() const
{
	switch (m_backend) {
	case c_backendFile:
	case c_backendMemory:
		return m_innerFile->GetPosition();
	case c_backendArchive:
		return ArchiveTell(m_archiveStream);
	default:
		return 0;
	}
}

// FUNCTION: TONY2 0x00416ee0
#ifdef COMPAT_MODE
ULONGLONG GameFile::Seek(LONGLONG p_offset, UINT p_from)
#else
LONG GameFile::Seek(LONG p_offset, UINT p_from)
#endif
{
	switch (m_backend) {
	case c_backendFile:
	case c_backendMemory:
		return m_innerFile->Seek(p_offset, p_from);
	case c_backendArchive:
		switch (p_from) {
		case CFile::end:
			ArchiveSeek(m_archiveStream, p_offset, 2);
			break;
		case CFile::current:
			ArchiveSeek(m_archiveStream, p_offset, 1);
			return GetPosition();
		case CFile::begin:
			ArchiveSeek(m_archiveStream, p_offset, 0);
			break;
		default:
			return -1;
		}

		return GetPosition();
	default:
		return -1;
	}
}

// FUNCTION: TONY2 0x00416f60
void GameFile::SetLength(GameFileSize p_newLen)
{
	if (m_backend > 0 && m_backend <= 2) {
		m_innerFile->SetLength(p_newLen);
	}
}

// FUNCTION: TONY2 0x00416f80
UINT GameFile::Read(void* p_buffer, UINT p_count)
{
	switch (m_backend) {
	case c_backendFile:
	case c_backendMemory:
		return m_innerFile->Read(p_buffer, p_count);
	case c_backendArchive:
		return ArchiveRead(m_archiveStream, p_buffer, p_count);
	default:
		return 0;
	}
}

// FUNCTION: TONY2 0x00416fc0
void GameFile::Write(const void* p_buffer, UINT p_count)
{
	if (m_backend > 0 && m_backend <= 2) {
		m_innerFile->Write(p_buffer, p_count);
	}
}

// FUNCTION: TONY2 0x00416ff0
void GameFile::Abort()
{
	switch (m_backend) {
	case c_backendFile:
	case c_backendMemory:
		m_innerFile->Abort();
		break;
	case c_backendArchive:
		ArchiveClose(m_archiveStream);
		break;
	}
}

// FUNCTION: TONY2 0x00417020
void GameFile::Flush()
{
	if (m_backend > 0 && m_backend <= 2) {
		m_innerFile->Flush();
	}
}

// FUNCTION: TONY2 0x00417040
void GameFile::Close()
{
	switch (m_backend) {
	case c_backendFile:
	case c_backendMemory:
		m_innerFile->Close();
		break;
	case c_backendArchive:
		ArchiveClose(m_archiveStream);
		break;
	}
}

// FUNCTION: TONY2 0x00417080
CFile* GameFile::Duplicate() const
{
	AfxThrowNotSupportedException();
	return NULL;
}

// FUNCTION: TONY2 0x00417090
TonyS32 __stdcall ProbeFileBackend(char* p_name)
{
	void* archive = ArchiveOpen(p_name);

	if (archive) {
		ArchiveClose(archive);
		return c_backendArchive;
	}

	return PathFileExists(p_name) != 0;
}

// FUNCTION: TONY2 0x00417120
void GameFile::SetFilePath(LPCTSTR p_newName)
{
	m_strFileName = p_newName;
}

// FUNCTION: TONY2 0x00417130
void __fastcall LangLoadCharset(char* p_file, TonyS32 p_language)
{
	GameFile file(p_file, CFile::typeBinary);
	TonyS32 i;

	file.Seek(2, CFile::begin);
	g_charsetCount = (file.GetLength() - 6) >> 1;
	g_charsetCodes[0] = 0;
	file.Read(&g_charsetCodes[1], g_charsetCount * 2);
	g_charsetCount++;
	memset(g_charsetReverse, 0, 0x10000);

	for (i = 0; i < g_charsetCount; i++) {
		g_charsetReverse[g_charsetCodes[i]] = (TonyU8) i;
	}

	LangSetLanguage(p_language);
	memset(g_langStringCounts, 0, sizeof(g_langStringCounts));
	memset(g_langStrings, 0, sizeof(g_langStrings));
}

// FUNCTION: TONY2 0x00417280
void __fastcall LangSetLanguage(TonyS32 p_language)
{
	g_language = p_language;
}

// Fully implemented, kept as STUB because it compares at 76%: the CString::Format path,
// binary open, count parse and per-line allocation all match, but the recompile homes
// p_language in edi (original: ebp) and saves edi in the prologue where the original
// sinks the push around the copy loop - a push-sinking the rest of the codebase never
// shows, further evidence the original cl is a slightly different VC5 build. Same
// register-tie family as JoystickEnumCallback (0x405430). Re-annotate as FUNCTION when solved.
// STUB: TONY2 0x00417290
void __fastcall LangLoadStrings(TonyS32 p_language)
{
	CString path;
	GameFile file;
	TonyU16 line[0x100];
	TonyS32 count;
	TonyS32 i;

	if (g_langStrings[p_language] == NULL) {
		path.Format("text\\language%d.txt", p_language);
		file.Open(path, CFile::typeBinary, NULL);
		file.Seek(2, CFile::begin);
		file.ReadLine(line, 0x100);
		swscanf((wchar_t*) line, L"%d", &count);
		g_langStrings[p_language] = new TonyU16*[count];
		g_langStringCounts[p_language] = count;

		for (i = 0; i < count; i++) {
			file.ReadLine(line, 0x100);
			g_langStrings[p_language][i] = (TonyU16*) new TonyU8[wcslen((wchar_t*) line) * 2 + 2];
			wcscpy((wchar_t*) g_langStrings[p_language][i], (wchar_t*) line);
		}
	}
}

// FUNCTION: TONY2 0x00417460
TextLabel::TextLabel()
{
}

// FUNCTION: TONY2 0x00417470
TextLabel::TextLabel(TonyS32 p_string)
{
	SetString(p_string);
}

// FUNCTION: TONY2 0x004174c0
void TextLabel::FormatString(TonyS32 p_string, TonyS32 p_arg)
{
	LangLoadStrings(g_language);

	TonyU16* format = g_langStrings[g_language][p_string];
	TonyU16* buffer = new TonyU16[0x100];

	swprintf((wchar_t*) buffer, (wchar_t*) format, p_arg);
	SetText(buffer);
	delete[] buffer;
}

// FUNCTION: TONY2 0x00417520
void TextLabel::SetText(TonyU16* p_text)
{
	WideToGlyphs(p_text, m_text.GetBufferSetLength(wcslen((wchar_t*) p_text) + 1));
	m_text.ReleaseBuffer(-1);
}

// FUNCTION: TONY2 0x00417560
void TextLabel::SetString(TonyS32 p_string)
{
	LangLoadStrings(g_language);
	SetText(g_langStrings[g_language][p_string]);
}
