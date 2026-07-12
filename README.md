# Tony & Friends: New Adventures Decompilation

This is a functionally complete decompilation of **Tony & Friends: New Adventures: Invasion in
Kellogg's Land** (1999, `TONY2.EXE`). It aims to be as accurate as possible, matching the
recompiled instructions to the original machine code as much as possible. The goal is to provide a
workable codebase that can be modified, improved, and ported to other platforms later on.

This project is modeled after the [LEGO Island decompilation](https://github.com/isledecomp/isle),
and uses the same tooling ([reccmp](https://github.com/isledecomp/reccmp)) and conventions.

> **Note:** This repository is for decompilation only and its code is true to the original release.
> It will not compile for targets other than 32-bit Windows.

## Status

<a href="https://foxtacles.github.io/tony2/TONY2PROGRESS.HTML"><img src="https://foxtacles.github.io/tony2/TONY2PROGRESS.SVG" width="50%"></a>

Every function of the game and its bundled sound library is decompiled and, to the best of our
knowledge, functionally identical to the original. Each implemented function matches the original
byte-for-byte except for a small number of documented near-misses that are stuck on a
compiler-vintage nuance (see below). The statically linked C runtime and MFC are vintage-bound
library code and are excluded from the accuracy measure via `--nolib`.

## Building

This project uses the [CMake](https://cmake.org/) build system. The original game was built with
two compilers, and the most accurate results require both:

- **Microsoft Visual C++ 5.0 SP3** (cl 11.00.7022, link 5.10.7303) for the game code. A portable
  copy is available at [archaic-msvc/msvc500sp3](https://github.com/archaic-msvc/msvc500sp3).
- **Microsoft Visual C++ 6.0 RTM** (cl 12.00.8168) for the four GSM voice codec / sound library
  translation units, which were compiled with VC6 and linked in (this is the toolchain that
  detection tools report from the Rich header). A portable copy is available at
  [archaic-msvc/msvc600](https://github.com/archaic-msvc/msvc600).

Since we're trying to match the output of this code to the original executable as closely as
possible, all contributions are graded with the output of these compilers.

#### Prerequisites

- The two portable MSVC toolchains linked above.
- [CMake](https://cmake.org/).
- [Python 3](https://www.python.org/) for the reccmp verification tooling.

#### Compiling (Windows)

1. Open a Command Prompt (`cmd`).
1. The stripped VC5 package omits `SHLWAPI`; copy `SHLWAPI.H` and `SHLWAPI.LIB` from the VC6
   package's `VC98\Include` and `VC98\Lib` into the VC5 package's `include` and `lib` directories.
1. Run the VC5 `bin\VCVARS32.BAT` to populate the path and other environment variables.
1. Make a `build` folder and `cd` into it.
1. Configure, pointing `TONY2_VC6_DIR` at the VC6 toolchain so the sound library TUs are built with
   the correct vintage:
```
cmake <path-to-source> -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DTONY2_VC6_DIR=<path-to-msvc600>
```
1. Build with `nmake` or `cmake --build .`. (NMake must sometimes be run twice; `nmake && nmake`.)
1. When this is done, there should be a recompiled `TONY2.EXE` in the build folder.

On macOS (Apple Silicon) the repository ships a self-contained `build.sh` wrapper that drives the
same toolchains under Wine. See `CLAUDE.md` for details.

### Verification

To verify your build against the original binary, install the [reccmp](https://github.com/isledecomp/reccmp)
tooling:

```
pip install -r tools/requirements.txt
```

Create `reccmp-user.yml` in the project root pointing to the original binary:
```yaml
targets:
  TONY2:
    path: path/to/TONY2.EXE
```

Then, from the build directory, run:
```
reccmp-reccmp --target TONY2 --nolib -S TONY2PROGRESS.SVG
```

## Project Structure

- `TONY2/` - Decompilation of `TONY2.EXE`
- `util/` - Utility headers for decompilation
- `cmake/` - CMake modules
- `tools/` - Python tools and requirements
- `3rdparty/` - DirectX 6 SDK (headers + import libs)
- `assets/` - Progress report icon

## Target Binary

| Binary | Size | SHA-256 |
|--------|------|---------|
| TONY2.EXE | 372,736 bytes | `97d48902c6f3db81a5b8e2fa53171f91cf58bce3c1c9fd8fcbe176285f1bd1c2` |

The binary is dated January 18, 1999. A copy of the original game disc is preserved at the
[Internet Archive](https://archive.org/details/tony-2_friends); CI extracts `TONY2.EXE` from that
image with `tools/extract_cdrom_file.py`.

## Contributing

Contributions are welcome. Please follow the conventions established in the codebase:
- Use reccmp annotations (`FUNCTION:`, `STUB:`, `GLOBAL:`, ...) for all decompiled code
- Functions in a compilation unit must be ordered by their address in ascending order
- Follow the clang-format configuration
- Use NCC naming conventions (`FUN_XXXXXXXX` for unknown functions, `g_unk0xXXXXXXXX` for unknown
  globals)
- Keep pull requests small and focused

See `CONTRIBUTING.md` for more.

## Disclaimer

This project is intended for education, preservation, and interoperability research. It is not
affiliated with the original developers or publishers.
