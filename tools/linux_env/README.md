# Linux build environment

The project README covers obtaining the original binaries and the CodeWarrior
compilers. This directory covers the two host tools that are left as an
exercise on Linux, and documents one non-obvious failure mode.

## Quick start

```bash
# after placing the original binaries in original/ and CodeWarrior in compilers/
sudo ./tools/linux_env/setup.sh
./configure.py && ninja
./progress.py --verify-bin
```

Build prerequisites for `setup.sh`: `cmake`, `g++`, `lld`, `libclang-dev`,
`git`, `curl`, plus `ninja` and `pyyaml` for the project itself.

## Why wibo needs a patch

`mwcceppc.exe` works out which bytes are Shift-JIS lead bytes by probing every
value from `0x80` to `0xFF` through `MultiByteToWideChar(932, 0, &b, 1, buf, 1)`.
On Windows a lone lead byte is a truncated sequence, so the call fails with
`ERROR_NO_UNICODE_TRANSLATION`, and the compiler records that byte as a lead
byte.

Upstream wibo ignores the code page argument entirely and widens each input
byte to a wide character, so the probe always succeeds and no byte is ever
classified as a lead byte. `-enc SJIS` then silently degrades to ASCII.

The visible symptom is narrow: a Shift-JIS character whose trail byte is `0x5C`
— ソ, ЎЅ, ‾ and friends — has that byte consumed as the start of an escape
sequence, dropping one byte from the string literal. In this project that
affects the `D2D_HEAP_NAME` constant in `include/constants/sjis_constants.h`,
which is enough to make `wiimj2d.dol` miscompare while all four `.rel` files
still match.

`wibo-cp932.patch` routes the known multi-byte code pages (932, 936, 949, 950,
1361, 65001) through iconv in both `MultiByteToWideChar` and
`WideCharToMultiByte`, keeping the existing behaviour for everything else. A
truncated sequence reports `ERROR_NO_UNICODE_TRANSLATION` regardless of
`MB_ERR_INVALID_CHARS`, which is what the lead-byte probe depends on.

If you use WINE instead of wibo, none of this applies.

## Why the wibo build overrides linker settings

`setup.sh` passes `-DWIBO_ENABLE_LTO=OFF` and `-fuse-ld=lld`. wibo links itself
with `-Wl,--image-base=0x70000000`, which GNU ld rejects for ELF targets, so
lld is required; and lld cannot read the GCC LTO objects that wibo's default
`AUTO` LTO setting produces, which surfaces as a wall of bogus
"undefined symbol" errors (including `main`).
