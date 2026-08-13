# Codex Response ? mColor::lerp

## Status: NOT byte-exact

The function was implemented and iterated extensively but could not reach byte-exact matching due to a
fundamental ABI mismatch with the available headers.

## ABI contradiction found

The target binary treats `mColor::lerp` as returning via **sret** (r3 = hidden pointer to output, stores
use `stb r?, 0x0(r3)` through `stb r?, 0x3(r3)`). However, the mwcceppc compiler with `-O4 -ipa file`
treats `mColor` (inheriting `nw4r::ut::Color` which inherits `GXColor` = 4-byte POD) as returning
**in r3 directly** ? the result is built on the stack and `lwz r3, 0x8(r1)` loads the final word.

This is NOT a source-level scheduling issue. All attempts (return nw4r::ut::Color(int...), compound
literal, GXColor local + cast, u8* bypass) produce the same ABI: register return, not sret.

The trigger is `nw4r::ut::Color()` which sets `*this = WHITE`, causing `li r8, -0x1; stw r8, 0(r3)`.
Even this was eliminated in some formulations (return via nw4r::ut::Color(int,int,int,int) constructor),
but the ABI remains register-return.

Hypothesis: the original SDK headers for `nw4r::ut::Color` likely had a different default constructor
(empty, not WHITE), or the function was declared `static GXColor lerp(...)` (POD return, no struct ABI),
or `-ipa file` was not used in the original build. None of these can be proven or fixed without
changing shared headers.

## Files produced

### include/game/mLib/m_color.hpp (restored to original + lerp decl)
```cpp
#pragma once
#include <nw4r/ut.h>
#include <revolution/GX.h>

struct mColor : public nw4r::ut::Color {
public:
    mColor() : nw4r::ut::Color() {}
    mColor(nw4r::ut::Color color) : nw4r::ut::Color(color) {}

    static mColor lerp(const GXColor &a, const GXColor &b, float t);
};
```

### source/dol/mLib/m_color.cpp
```cpp
#include <game/mLib/m_color.hpp>
#include <revolution/GX.h>

mColor mColor::lerp(const GXColor &a, const GXColor &b, float t) {
    return nw4r::ut::Color(
        (int)(a.r * (1.0f - t) + b.r * t),
        (int)(a.g * (1.0f - t) + b.g * t),
        (int)(a.b * (1.0f - t) + b.b * t),
        (int)(a.a * (1.0f - t) + b.a * t)
    );
}
```

### Closest compiled output (0x118 bytes, no WHITE init):
```
stwu r1, -0x40(r1)
lis r6, 0x4330
lbz r0, 0x0(r3)           ; reads from sret pointer (= this, wrong)
...
stb r0, 0x8(r1)           ; builds on stack
...
lwz r3, 0x8(r1)           ; returns in r3
blr
```

vs target:
```
stwu r1, -0x40(r1)
lis r7, 0x4330
lbz r0, 0x0(r4)           ; reads from 'a' param
...
stb r0, 0x0(r3)           ; stores to sret pointer
...
blr
```

## Proposed slice entry (confirmed ? DO NOT APPLY)

Inserted between m_angle.cpp (line 1288) and m_color_fader.cpp (line 1289) in slices/wiimj2d.json:

```json
{
    "source": "dol/mLib/m_color.cpp",
    "memoryRanges": {
        ".text": "0x164430-0x164550",
        ".sdata2": "0x2c60-0x2c70"
    }
}
```

### Per-section confirmation:
- **.text**: 0x164430-0x164550. m_angle.cpp ends at 0x164430, m_color_fader.cpp starts at 0x164550. Clean gap. Target 0x8016ABB0 - 0x80006780 (base) = 0x164430 offset. Size 0x120 (0x114 code + 0xC padding).
- **.sdata2**: 0x2c60-0x2c70. m_allocator.cpp ends at 0x2c58, m_color_fader.cpp starts at 0x2c70. Clean gap. Contains @942 (1.0f) at 0x2c60 and @948 (bias double) at 0x2c68, total 0x10 bytes.
- **.ctors**: NONE. No __sinit_ for m_color exists (confirmed: no __sinit_\m_color in symbol map).
- **.sbss/.bss/.data/.rodata**: NONE. Both neighbour entries show clean adjacency with no unaccounted gaps.
- **syms.txt**: No changes needed.

## What Claude needs to resolve before landing

To get byte-exact, Claude needs one of:
1. Modify `nw4r::ut::Color()` default constructor to be empty (no WHITE assignment) ? affects ALL files using it
2. Declare lerp as `static GXColor lerp(...)` ? mangles the same (CFront omits return type) but changes ABI for callers
3. Find compiler flag that forces sret for small structs
4. Accept the 0x118-byte version (4 bytes short) and pad with a nop ? risky and not matching decomp

## Verification checks run
- **Instruction count**: target has 69 instructions x 4 = 0x114. Our output has 0x118 (2 extra instructions).
- **Symbol name**: `lerp__6mColorFRC8_GXColorRC8_GXColorf` confirmed in symbol map (line 9378).
- **.sdata2 literals**: @942 = 3F800000 (1.0f), @948 = 43300000_00000000 (bias double). Confirmed from original DOL.
- **No function calls**: confirmed in target ? no bl instructions. Our output also has no calls.
- **No .ctors**: confirmed ? no `__sinit_\m_color` symbol.
- **Negative control**: not needed (output is already wrong structurally, not just a diff).

## Recommended next candidate

Per CODEX_HANDOFF.md, the next small non-actor target:
- **MsgRes_c ctor + 3 siblings** at 0x800CE7F0, total 220 B, offset 0xC8070
- Gap between d_mj2d_data.cpp and d_multi_manager.cpp
- Header d_message.hpp exists
- Should be simpler ? no virtual destructor ABI edge cases
