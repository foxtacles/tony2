#!/usr/bin/env bash
#
# Recompile the Tony & Friends - New Adventures decomp on macOS (Apple Silicon) via Wine 11 +
# Rosetta 2 + MSVC 5.0 SP3 (the game's original compiler, cl 11.00.7022 + link 5.10.7303).
#
# Self-contained: ensures a wineserver is running, loads the MSVC environment, configures the
# build tree if needed, then builds the target. Prints "BUILD OK" on success or "BUILD FAILED" on
# any error, and exits non-zero on failure.
#
# Paths are overridable via env: TONY2_DIR, MSVC_DIR, BUILD_DIR, JOBS (and VC6_DIR for the MSVC 6.0
# RTM toolchain used to compile the GSM voice codec / sound library TUs). Point MSVC_DIR at a
# portable archaic-msvc/msvc500sp3 checkout and VC6_DIR at archaic-msvc/msvc600.

set -eo pipefail

TONY2_DIR="${TONY2_DIR:-/Users/foxtacles/Projects/TONY2}"
MSVC_DIR="${MSVC_DIR:-/Users/foxtacles/Projects/MSVC500-SP3}"
BUILD_DIR="${BUILD_DIR:-$TONY2_DIR/build}"
JOBS="${JOBS:-12}"

export WINEPREFIX="${WINEPREFIX:-$HOME/.wine}"
export WINEDEBUG="${WINEDEBUG:--all}"
export MVK_CONFIG_LOG_LEVEL="${MVK_CONFIG_LOG_LEVEL:-0}"   # silence MoltenVK Vulkan log spam

fail() { echo "BUILD FAILED: $*" >&2; exit 1; }
trap 'echo "BUILD FAILED (line $LINENO)" >&2' ERR

# 1. Sanity checks.
[ -f "$MSVC_DIR/activate_x86" ] || fail "activate_x86 not found in $MSVC_DIR"
[ -x "$TONY2_DIR/.venv/bin/python" ] || fail "repo venv python not found at $TONY2_DIR/.venv/bin/python"
command -v wine  >/dev/null 2>&1 || fail "wine not installed (brew install --cask wine-stable)"
command -v cmake >/dev/null 2>&1 || fail "cmake not installed (brew install cmake)"

# No need to pre-start wineserver: the first cl.exe auto-starts one, and the overlapping parallel
# cl/link clients keep that single server alive for the whole build (it exits a few seconds after).
# (wineserver -p persistence does not stick on this Wine 11 build, so an always-on server isn't
# worth the trouble for the ~1-2s it would save.)

# 2. Load the MSVC environment (INCLUDE/LIB/WINEPATH + cl/link wrappers on PATH).
#    activate_x86 requires bash, which is what this script runs under.
source "$MSVC_DIR/activate_x86" >/dev/null

# 3. Configure. The VC6 RTM toolchain (sound-library TU vintage) is passed along when
#    present; see the TONY2_VC6_DIR option in CMakeLists.txt.
VC6_DIR="${VC6_DIR:-$HOME/Projects/MSVC600-8168}"
VC6_ARG=()
[ -f "$VC6_DIR/activate_x86" ] && VC6_ARG=(-DTONY2_VC6_DIR="$VC6_DIR")

echo "[build] configuring -> $BUILD_DIR"
cmake -S "$TONY2_DIR" -B "$BUILD_DIR" -G "Unix Makefiles" \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DCMAKE_C_COMPILER=cl \
  -DCMAKE_CXX_COMPILER=cl \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DPython3_EXECUTABLE="$TONY2_DIR/.venv/bin/python" \
  "${VC6_ARG[@]}"

# 4. Build the target.
echo "[build] compiling with -j$JOBS"
cmake --build "$BUILD_DIR" -j"$JOBS"

# 5. Verify artifacts.
[ -f "$BUILD_DIR/TONY2.EXE" ] || fail "TONY2.EXE was not produced"

# Strip the PE debug directory so the recomp header matches the original's
# release layout (reccmp's assert fixup keys on it; the PDB path comes from
# reccmp-build.yml, not the PE).
"$TONY2_DIR/.venv/bin/python" - "$BUILD_DIR/TONY2.EXE" <<'PYEOF'
import sys, pefile
pe = pefile.PE(sys.argv[1])
d = pe.OPTIONAL_HEADER.DATA_DIRECTORY[6]
if d.VirtualAddress:
    d.VirtualAddress = 0
    d.Size = 0
    pe.write(sys.argv[1] + ".tmp")
    pe.close()
    import os
    os.replace(sys.argv[1] + ".tmp", sys.argv[1])
PYEOF

echo "BUILD OK"
