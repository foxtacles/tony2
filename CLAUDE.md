# Tony & Friends - New Adventures Decompilation

Decompilation of Tony & Friends - New Adventures (1999, "Tony 2", `TONY2.EXE`). Modeled after the
[LEGO Island](https://github.com/isledecomp/isle) and LEGO Racers decompilations; uses the same
tooling (reccmp) and conventions.

One target:
- **TONY2** — `TONY2.EXE` (game code spans 0x401000..~0x430000, ~13 game/lib TUs plus CRT + static MFC)

## Original binary facts

- **Built with Visual C++ 5.0 (cl 11.00.7022)** - but NOT exactly RTM or SP3: both produce
  identical code that mismatches the original in a systematic "local-slot direction /
  stack-arg pre-caching" family (~15 parked functions document it). Likely SP1 or SP2.
  Confirmed — confirmed empirically:
  functions that stuck at 84%/44% under VC6 cl 12.00.8168 (allocator-margin diffs: push
  placement, register seeding) match 100% under VC5. Corroborated by forensics: PE optional
  header says linker 5.10 (= VC5 SP3 link 5.10.7303), the Rich header's cvtres 5.00.1668 entry
  is VS97 SP3's, and the game C++ objects carry no `@comp.id` (pre-VC6 compilers don't emit
  them). We build with the portable
  [archaic-msvc/msvc500sp3](https://github.com/archaic-msvc/msvc500sp3) package
  (`~/Projects/MSVC500-SP3`, with Wine wrappers added; `msvcp50.dll` must sit next to
  `link.exe` for `msdis100.dll`). VC5 RTM and SP3 produced identical code on everything tested
  so far.
- The Rich header's "Microsoft Visual C/C++ 12.00.8168 [C]" (what DIE reports) belongs to a
  **GSM voice codec static library** (13 C TUs, `gsmvoice.c`, plus 2 MASM 6.13.7299 objects and
  LINK 5.12.8034 lib-tooling entries) that was built elsewhere with VC6 RTM and linked in.
  Those TUs will need cl 12.00.8168 (`MSVC_DIR=~/Projects/MSVC600-8168 ./build.sh`) when we
  reach them.
- Game code is **C++** (C++ EH frames, `new` with EH states, `__thiscall` members). Free/static
  helper functions are frequently `__fastcall` (two register args ecx/edx); we write explicit
  `__fastcall`.
- **Statically linked CRT (LIBCMT, /MT)** — entry `WinMainCRTStartup` at 0x4301b0 calls `_mtinit`.
  With VC5 SP3's LIBCMT the entry point compares at ~83% (exact CRT lib vintage still under
  investigation; game code is unaffected).
- **Statically linked MFC 4.21** (RTTI shows `CWnd`, `CFile`, `CPtrList`, `CMapPtrToPtr`,
  `CMemFile`, exception classes, `AFX_MODULE_STATE`...). No `CWinApp` — MFC is used as a
  utility library; WinMain (0x410920) is custom.
- **No /Gy**: functions inside a TU are separated by compiler-emitted 0x90 NOP padding and the
  binary shows no COMDAT folding, so the original was compiled without function-level linking
  (VC5/VC6 `/O2` implies `/Gy`, so the build spells out the /O2 expansion minus /Gy). Folding/ICF
  concerns from the racers playbook mostly do not apply here.
- **Float pool rendering**: VC5 does not emit `__real@...` PDB symbols for float literals, so a
  pooled literal renders as `<OFFSET1>` on the recomp side of reccmp diffs. Where the original
  plausibly used a named constant (e.g. `g_unk0x44c4a0` = 0.0f), declare a TU-local
  `static const TonyFloat` with a `// GLOBAL:` annotation — never combined with a floats-CSV
  row for the same address.
- Imports: DDRAW, DINPUT, DSOUND, WINMM, ole32, SHLWAPI, COMCTL32 + the usual KERNEL32/USER32/
  GDI32 (+ WINSPOOL/comdlg32 pulled in by MFC).
- Game object system: a 105-case type switch at 0x410ec0 installs per-type init functions;
  init functions install handler callbacks (function pointers, not vtables).

## Building

On macOS (Apple Silicon), use the Wine + MSVC 5.0 SP3 wrapper script:

```bash
./build.sh          # configure + build into build/, prints BUILD OK / BUILD FAILED
MSVC_DIR=~/Projects/MSVC600-8168 BUILD_DIR=build-vc6 ./build.sh   # VC6 RTM (GSM lib TUs)
```

On Windows: run VCVARS32.BAT from a Visual C++ 5.0 SP3 installation, then the usual
`cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo` + double `cmake --build .`
(NMake bug workaround). TONY2.EXE links the static CRT (`/MT`).

Portable toolchains: https://github.com/archaic-msvc/msvc500sp3 (game code) and
https://github.com/isledecomp/MSVC600-8168 (GSM codec TUs).

## reccmp

```bash
pip install -r tools/requirements.txt

# Compare (run from build/ directory); --nolib excludes CRT/MFC library functions,
# which are vintage-bound and not fixable from source (the game links a slightly
# different VC5 CRT build, and MFC's operator new from nafxcw.lib)
reccmp-reccmp --target TONY2 --nolib --print-rec-addr
reccmp-reccmp --target TONY2 --verbose 0x401000 --print-rec-addr

# Compare global variable data values
reccmp-datacmp --target TONY2 --verbose --print-rec-addr

# Progress SVG
reccmp-reccmp --target TONY2 --nolib -S TONY2PROGRESS.SVG

# Lint annotations (pass source dir to avoid scanning gitignored files)
reccmp-decomplint --module TONY2 --warnfail TONY2
```

`reccmp-user.yml` (gitignored) points to the original binary (`tony2bin/TONY2.EXE`) for local
comparison.

## Annotations

Functions in a compilation unit must be ordered by address (ascending).

```cpp
// FUNCTION: TONY2 0x00401000    — complete, compared by reccmp
// STUB: TONY2 0x00402180        — incomplete, skipped by reccmp
// LIBRARY: TONY2 0x00430390     — CRT/MFC/3rd-party (annotate in library_msvc.h once created)
// SYNTHETIC: TONY2 0x0040xxxx   — compiler-generated (scalar deleting destructors)
// GLOBAL: TONY2 0x0045c7f4      — global variable
// VTABLE: TONY2 0x0044xxxx      — virtual function table (MFC-derived classes)
// SIZE 0x1d298                   — struct/class size assertion
```

**The colon is required** for every annotation except `// SIZE`. Addresses are 8 hex digits,
lowercase, zero-padded.

## Conventions

Follows the LEGO Racers conventions (NCC-enforceable, see `tools/ncc/`):

- Functions: `FUN_XXXXXXXX` (8 hex digits, lowercase) until semantics are proven.
- Globals: `g_unk0xXXXXXXXX`; members: `m_unk0xXX` (by offset); parameters: `p_...`.
- Unknown classes: `RandomName0xSIZE` (random PascalCase + hex size). If the size is not yet
  proven, use the random name without the suffix and add the suffix on first proof.
- Member offset comments (`// 0xNN`) are required; gap members use self-documenting subtraction:
  `undefined m_unk0x20[0x78 - 0x20]; // 0x20`.
- Types: `TonyS8`/`TonyU8`/`TonyS16`/`TonyU16`/`TonyS32`/`TonyU32`/`TonyFloat`/`TonyChar`/
  `TonyBool` from `util/types.h` for game code. Keep original types at API boundaries
  (Win32, DirectX, MFC, CRT). Unproven types: `undefined`/`undefined2`/`undefined4`.
- Bit tests: `if (flags & 0x200)` — no `!= 0` / `== 0`.
- `NULL` for pointers, `TRUE`/`FALSE` for `TonyBool`/`TonyBool32`.
- Win32 API: prefer un-suffixed names (`CreateWindowEx`, not `CreateWindowExA`).
- STUB every unknown callee (with the `STUB(0xADDRESS)` macro body from `decomp.h`), ordered by
  address ascending per file.
- clang-format the files you touch (`cmake --build build --target clang-format`).

## Decompiling a New Function

1. Read the disassembly; note called functions, globals, and the byte budget to the next
   function.
2. Check calling conventions at call sites: `mov ecx, X` only ⇒ `__thiscall` member or
   single-arg `__fastcall`; `ecx` + `edx` ⇒ `__fastcall`; `ecx` + pushes ⇒ `__thiscall`
   member with stack args.
3. STUB every unknown callee at its real address.
4. Write clean C++, not IDA pseudocode.
5. Build (`./build.sh`), then `reccmp-reccmp --target TONY2 --verbose 0xADDRESS --print-rec-addr`
   from `build/`. Iterate to 100%.
6. Verify global data with `reccmp-datacmp` when globals with initial values are involved.
7. Lint: `reccmp-decomplint --module TONY2 --warnfail TONY2`.

## Decompilation Principles

- **Every type must be corroborated by matched code.** A type is proven only when a
  `// FUNCTION:` using it reaches 100%. Until then, `undefined`/`undefined4`.
- **`// FUNCTION:` means 100% match.** Any diff ⇒ the code is wrong; investigate the root cause.
- **Every annotation has a real address** — no placeholders.
- **No raw pointer arithmetic as a substitute for types.** Find the real struct so casts are
  legitimate C++.
- **Read the original binary directly** (Python/pefile/capstone) when in doubt; disassembler
  dumps stop at the first `ret` — a function can continue past it.
- Direct member access chains (`p_object->m_unk0x08->m_unk0x78`) usually beat cached locals —
  add locals only when the original demonstrably homes a value.
- Float literals compile to immediate stores (`mov [x], 0x3f800000`) or pooled memory operands;
  named float constants load const-first. See the LEGO Racers float rules before adding any
  float CSV entries.

## MSVC Codegen Patterns

The LEGO Racers CLAUDE.md catalogue of MSVC 6.0 codegen patterns (register allocation levers,
loop shapes, branch layout, return type inference, SEH emission) largely applies to VC5 as
well — consult `~/Projects/racers/CLAUDE.md` when a function is stuck below 100%. VC5-vs-VC6
differences observed: VC5 keeps callee-saved pushes in the prologue where VC6 sinks them and
duplicates epilogues; VC5 seeds the register allocator differently on zero/offset temps.
TONY2-specific patterns observed so far:

- `__fastcall` init functions pass `this`-like first args through ecx untouched to base-init
  calls (`FUN_00416890`).
- Frame records are 26-byte structs (13 `TonyS16`s); indexing emits `lea` *13 then scale-2
  addressing.
- Instance data blocks: 0x1c-byte descriptor head copied via `rep movsd` (7 dwords), runtime
  state lives directly behind it.

## Project Structure

```
TONY2/               # TONY2.EXE source
  include/           # Headers
  src/               # Source files
util/                # decomp.h, compat.h, types.h
cmake/               # reccmp CMake integration
tools/               # requirements.txt, ncc naming checker, iwyu mapping
3rdparty/dx6/        # DirectX 6 SDK (headers + import libs)
tony2bin/            # Original TONY2.EXE for reccmp comparison (gitignored)
```
