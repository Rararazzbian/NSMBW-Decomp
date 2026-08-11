#!/usr/bin/env bash
# Sets up a Linux build environment for the NSMBW decompilation.
#
# This installs the two host tools the project needs on Linux and that the
# README leaves to the reader: a wibo build that can run mwcceppc.exe, and
# objdiff-cli for inspecting per-function diffs.
#
# The original game binaries are *not* handled here. Follow steps 1-4 of the
# README to place wiimj2d.dol and the four .rel files into original/ first.

set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
WORK_DIR="${WORK_DIR:-$(mktemp -d)}"
PREFIX="${PREFIX:-/usr/local/bin}"

# Pinned so the patch below applies cleanly. Bump both together.
WIBO_COMMIT="e8f4795ca29e4eb3fdd57e39d3a7490c8eef185b"
OBJDIFF_VERSION="2.7.1"

echo "==> Working directory: $WORK_DIR"

echo "==> Building wibo"
git clone https://github.com/decompals/wibo.git "$WORK_DIR/wibo"
git -C "$WORK_DIR/wibo" checkout --quiet "$WIBO_COMMIT"

# mwcceppc detects DBCS lead bytes by probing every byte value 0x80-0xFF
# through MultiByteToWideChar. Upstream wibo ignores the code page argument
# and never reports a truncated sequence, so no byte is ever classified as a
# lead byte and `-enc SJIS` silently degrades to ASCII. That corrupts every
# Shift-JIS string literal containing a 0x5C trail byte, which is enough to
# break the wiimj2d.dol match. See tools/linux_env/README.md.
git -C "$WORK_DIR/wibo" apply "$REPO_DIR/tools/linux_env/wibo-cp932.patch"

# LTO is disabled because GCC's LTO objects are opaque to lld, and lld is
# required for the -Wl,--image-base link option that GNU ld rejects.
cmake -S "$WORK_DIR/wibo" -B "$WORK_DIR/wibo/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DWIBO_ENABLE_FIXTURE_TESTS=OFF \
    -DWIBO_ENABLE_LTO=OFF \
    -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld"
cmake --build "$WORK_DIR/wibo/build" -j"$(nproc)"
install -m755 "$WORK_DIR/wibo/build/wibo" "$PREFIX/wibo"

echo "==> Installing objdiff-cli $OBJDIFF_VERSION"
curl -sSL -o "$WORK_DIR/objdiff-cli" \
    "https://github.com/encounter/objdiff/releases/download/v$OBJDIFF_VERSION/objdiff-cli-linux-x86_64"
install -m755 "$WORK_DIR/objdiff-cli" "$PREFIX/objdiff-cli"

echo "==> Verifying"
wibo "$REPO_DIR/compilers/Wii/1.1/mwcceppc.exe" -version | head -2
objdiff-cli --version

echo
echo "Done. Next: ./configure.py && ninja && ./progress.py --verify-bin"
