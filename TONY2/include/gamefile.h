#ifndef GAMEFILE_H
#define GAMEFILE_H

#include "decomp.h"
#include "types.h"

#include <afx.h>

// CFile's file position/length virtuals widened from DWORD to ULONGLONG after MFC 4.21; transparent
// DWORD on the decomp build.
#ifdef COMPAT_MODE
typedef ULONGLONG GameFileSize;
#else
typedef DWORD GameFileSize;
#endif

// Unified game file. m_backend selects the backend: 1 reads a plain file through an
// inner CStdioFile, 2 a memory file (CMemFile), 4 a RESULT.FF archive entry (handle in
// m_archiveStream); 5 defers the choice to Open, which probes the archive and the file system
// (ProbeFileBackend). MFC CFile-derived; LockRange/UnlockRange and Serialize/Dump are empty
// inline overrides the linker folds (/OPT:ICF).
// VTABLE: TONY2 0x0044c7a0
// SIZE 0x1c
class GameFile : public CFile {
public:
	GameFile();
	GameFile(const char* p_name, UINT p_flags);
	~GameFile() override;

	void Serialize(CArchive& p_archive) override {}
	void AssertValid() const override {}
	void Dump(CDumpContext& p_context) const override {}
	GameFileSize GetPosition() const override;
	void SetFilePath(LPCTSTR p_newName) override;
	BOOL Open(LPCTSTR p_fileName, UINT p_openFlags, CFileException* p_error = NULL) override;
	CFile* Duplicate() const override;
#ifdef COMPAT_MODE
	ULONGLONG Seek(LONGLONG p_offset, UINT p_from) override;
#else
	LONG Seek(LONG p_offset, UINT p_from) override;
#endif
	void SetLength(GameFileSize p_newLen) override;
	UINT Read(void* p_buffer, UINT p_count) override;
	void Write(const void* p_buffer, UINT p_count) override;
	void LockRange(GameFileSize p_pos, GameFileSize p_count) override { AfxThrowNotSupportedException(); }
	void UnlockRange(GameFileSize p_pos, GameFileSize p_count) override { AfxThrowNotSupportedException(); }
	void Abort() override;
	void Flush() override;
	void Close() override;

	void SetBackend(TonyS32 p_type);
	void ReadLine(TonyU16* p_buffer, TonyU32 p_max);

	CFile* m_innerFile;    // 0x10
	void* m_archiveStream; // 0x14
	TonyS32 m_backend;     // 0x18
};

DECOMP_SIZE_ASSERT(GameFile, 0x1c)

TonyS32 __stdcall ProbeFileBackend(char* p_name);

#endif // GAMEFILE_H
