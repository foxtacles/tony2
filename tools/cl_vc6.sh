#!/usr/bin/env bash
#
# Compile a translation unit with the VC6 RTM toolchain (cl 12.00.8168) under Wine.
# The sound library linked into TONY2.EXE was built with VC6 while the game code is
# VC5; CMake invokes this wrapper for the VC6-vintage TUs.
#
# Usage: cl_vc6.sh <MSVC600_DIR> <cl args...>

set -eo pipefail

MSVC600_DIR="$1"
shift

[ -f "$MSVC600_DIR/activate_x86" ] || { echo "cl_vc6.sh: activate_x86 not found in $MSVC600_DIR" >&2; exit 1; }

source "$MSVC600_DIR/activate_x86" >/dev/null

exec cl "$@"
