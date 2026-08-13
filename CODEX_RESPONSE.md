# Codex Response ? mColor::lerp (Round 2)

## Status: NOT byte-exact, root cause identified

The ABI claim from round 1 was wrong ? sret works fine. The blocker is 2 instructions:
`li r8, -0x1; stw r8, 0x0(r3)` ? `nw4r::ut::Color()` WHITE init on the return object.

## What Claude ruled out (confirmed)
- Empty `nw4r::ut::Color()` ? breaks all verified TUs
- `static GXColor lerp(...)` ? gives register return, wrong ABI
- `return nw4r::ut::Color(r,g,b,a)` ? spills locals, 0x160

## What I found

A struct inheriting from `GXColor` (POD) with empty default ctor + user-declared destructor
produces **0x114 with correct sret, no WHITE init**:

```cpp
struct mColor : public GXColor {
    mColor() {}
    mColor(nw4r::ut::Color color) { *(u32 *)this = *(const u32 *)&color; }
    ~mColor() {}
    static mColor lerp(const GXColor &a, const GXColor &b, float t);
};
```

The lerp function compiles to exactly 0x114 bytes, correct sret (stb r0,0(r3) through stb r0,3(r3)),
no WHITE init. Not quite byte-exact beyond that (f8/f9 register swap, different scheduling) but
structurally correct.

## Contradiction: this is a shared-header change

Changing `mColor` to inherit from `GXColor` instead of `nw4r::ut::Color` affects every TU that
includes `m_color.hpp`. Files that depend on `mColor()` being WHITE (like `m_fader_base.hpp`'s
`mFaderColor()` member init, and `d_wipe_*.cpp` files passing `mColor(color)` through
`mFaderBase_c`) would change semantics. Claude warned one such change already failed verification.

The original `mColor` likely did NOT inherit from `nw4r::ut::Color` ? our reconstruction is wrong.
But fixing it requires re-verifying all five binaries. Claude should decide whether to take that risk.

## Next
Moving to `MsgRes_c` (0x800CE7F0, 220 B) as scoped.
