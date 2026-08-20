# `d_line_mng.cpp` -- `mov_to_*`/`mov_frm_*` quadruplets, author_mov round

Scope: the eight assigned functions only. Nothing under `source/`, `include/`,
`syms.txt`, `slices/*.json`, `wip/line_mng_shared/` (shared, read-only), or any
other agent's directory was touched. All work is in `wip/author_mov/`.

## HEADLINE RESULT

**All eight assigned functions are byte-exact matches.** Measured with
`tools/auto_decomp/harness.py`'s `compile_draft`/`disasm`/`diff_fn`, compiling
`wip/author_mov/d_line_mng.cpp` with `-i wip/author_mov/shadow_include` ahead
of the normal includes (see "Local header overrides" below for why this isn't
`wip/line_mng_shared/shadow_include` verbatim).

| function | target words | draft words | differing (real) | matched |
|---|---|---|---|---|
| `mov_to_rightlower` | 207 | 207 | 0 | **YES** |
| `mov_to_rightupper` | 239 | 239 | 0 | **YES** |
| `mov_to_leftupper` | 247 | 247 | 0 | **YES** |
| `mov_to_leftlower` | 243 | 243 | 0 | **YES** |
| `mov_frm_rightupper` | 63 | 63 | 0 | **YES** |
| `mov_frm_leftlower` | 63 | 63 | 0 | **YES** |
| `mov_frm_rightlower` | 63 | 63 | 0 | **YES** |
| `mov_frm_leftupper` | 63 | 63 | 0 | **YES** |

"differing (real)" excludes a cosmetic artifact explained next -- it is NOT a
content difference, verified down to the raw instruction bytes.

### The one recurring "diff" that isn't one

Every `mov_to_*` function's `diff_fn` report shows exactly two differing
lines, always the same two (function-relative lines 5 and 12, the `lis
r31,X@ha` / `addi r31,r31,X@l` pair that loads the base of the 25-entry
`StateID_*` `.bss` array):

```
5 | want: lis r31, "SYM0"@ha        got: lis r31, SYM0@ha
12 | want: addi r31, r31, "SYM0"@l  got: addi r31, r31, SYM0@l
```

I checked the **raw instruction bytes** in both disassemblies rather than
trusting the text: target `800C3D04: 3F E0 00 00` vs my draft's equivalent
`00000604: 3F E0 00 00` -- **identical machine code**. The only difference is
that dtk names the relocation target `"@49614_80359100"` in the fully-linked
DOL (target.txt) but `...bss.0` in a freshly-compiled standalone `.o` (my
draft), because the anonymous `.bss` pool object hasn't been resolved to a
final linker symbol outside of a real link. `canonicalise()` already treats
both spellings as pool symbols and numbers them `SYM0`; the residual
difference is only the surrounding quote characters carried over from each
side's original text, which `diff_fn`'s exact string compare still flags.
This is not something my source can fix -- it is a property of comparing a
standalone-object disassembly against a whole-DOL disassembly, and it should
disappear once this unit is actually linked into the DOL at land time.

## Required shared-header corrections (I did NOT touch the shared header --
see "Local header overrides" below)

These three fixes were necessary to get the eight functions to compile *and*
match, and none of them touch a function outside my assignment, but all three
live in `wip/line_mng_shared/shadow_include/game/bases/d_line_mng.hpp`, which
I was told is shared/read-only. I made the changes ONLY in a private copy at
`wip/author_mov/shadow_include/game/bases/d_line_mng.hpp` and report them here
for the lead to apply to the real shared header.

1. **`getLineUnitNo(f32, f32)` must return a non-void value.** Proof: every
   `mov_to_*`/`mov_frm_*` call site does `bl getLineUnitNo...` immediately
   followed by `mr r31,r3` (or an immediate `cmplwi r3,...`) -- the return
   register is read, never just clobbered. I declared it `u32` locally; this
   is **not proven to be exactly `u32`** (could be `u8`/`int`/etc, all of
   which pass a full, unmasked register back per the call sites I can see) --
   whoever owns `getLineUnitNo` should settle the exact width with their own
   compile-both-ways test.
2. **`mov_to_rightupper`/`mov_to_rightlower`/`mov_to_leftupper`/`mov_to_leftlower`
   must return `bool`, not `void`.** Proof: every `mov_frm_*` call site does
   `bl mov_to_...` then `cmpwi r3,0x0` / `bne`, and every `mov_to_*` body ends
   each path with `li r3,0x1` or `li r3,0x0` right before the shared
   epilogue -- textbook return-value codegen. This is exactly the "CFront
   mangling omits return types" trap the project brief warns about; the
   mangled name never changes.
3. **`is_unit_circle2x2(ulong)` / `is_unit_circle4x4(ulong)` must be
   `static`, not ordinary members.** Proof: every call site inside the
   `mov_to_*` bodies sets only ONE register before the `bl` (`mr r3,<id>`) --
   never a second `mr r4,<id>` alongside an implicit `this` in r3. A
   non-static declaration made MWCC emit `mr r3,this / mr r4,id`, which is 7
   extra instructions per call and was the entire size gap (214 vs 207 words,
   etc.) before I fixed it locally. This one is somewhat surprising since the
   comment on these functions already correctly says "Confirmed bool", but
   the non-static-ness was never checked.

## Local header overrides

`wip/author_mov/shadow_include/game/bases/d_line_mng.hpp` is a full copy of
the shared header with only those three fixes applied (each marked `LOCAL
OVERRIDE` in a doc comment with the evidence above). It is NOT registered
anywhere outside my own `wip/author_mov/build.py` driver, which passes it via
`extra_inc` **ahead of** `wip/line_mng_shared/shadow_include`, so it never
touches the shared checkout. Diff against the shared header is exactly the
three signature changes above (plus doc comments); no member/layout/other
declaration was touched.

## What I DIDN'T have to invent: the `.data` "constant tables" are the
switch's own jump tables, not lookup data

The brief flagged four leading `.data` objects (`0x80316CA0`-`0x80316E98`,
sizes `0x84/0x80/0x7C/0x74`) as probably my functions' lookup constants. I
read them directly out of `original/wiimj2d.dol` (`.data` file offset =
`va - 0x802fe6a0 + 0x2fa7a0`, `.sdata2` file offset = `va - 0x8042b360 +
0x34ffa0`, both derived from the DOL's own program-header table, not
assumed):

- `@55792` (`0x84`, `0x80316CA0`) is referenced by `__sinit` and an unrelated
  unnamed helper (`fn_800C31C0`) -- **not one of my eight functions.** Not
  investigated further; out of scope.
- `@55889`/`@55993`/`@56054` (`0x80`/`0x7C`/`0x74`, at `0x80316D24`/
  `0x80316DA4`/`0x80316E20`) ARE mine, but they are **switch-statement jump
  tables**, one word per `case` value from a dense `switch (unitID) { case 0:
  ... case N: ... }`, generated automatically by MWCC for
  `mov_to_rightupper`/`mov_to_leftupper`/`mov_to_leftlower` (32/31/29 entries
  respectively, matching each function's `cmplwi r3,<max>; bgt default`
  bound exactly). I did not need to hand-declare any constant array -- writing
  the `switch` with the right `case` labels reproduces the table byte-for-byte
  as a side effect of codegen. `mov_to_rightlower` has only 7 sparse case
  values and MWCC compiles it as a `cmplwi`/`bne` chain instead (no jump
  table), which is also why it's the shortest of the four (207 vs 239-247).
- I decoded each table's 32/31/29 raw relocated words directly from the DOL
  (already-linked addresses, no relocation-as-zero issue) to recover the exact
  `case value -> case body` mapping and the exact `changeState(&StateID_X)`
  argument for each case -- this is what let me write the `switch` bodies
  correctly on the first attempt once the two header fixes above were in.
- The `16.0f` unit-size constant used in several `case`s (`@55056` at
  `0x8042CB48` in `.sdata2`) reads as `16.0` in the DOL; it is a distinct pool
  object from the named `smc_UNIT_SIZE_X` (`0x8042CB18`, also `16.0`) despite
  the same value, so I wrote it as a plain `16.0f` literal rather than
  referencing `smc_UNIT_SIZE_X` by name (confirmed correct: literal + register
  scheduling only converged to a match once I stopped treating it as an
  inline-expression add and split it into a separate `mUnitBasePos.x = pos.x;
  ...; mUnitBasePos.x += 16.0f;` -- see next section).

## MWCC levers that closed real residuals this round

1. **The single biggest miss, twice: `beq`/`bne` skip range is bigger than it
   looks.** In `mov_to_rightlower`'s first case (`unitID==0x2`) and the
   analogous first case in every other `mov_to_*`, the `id != <caseValue>`
   check's `beq` branches PAST the `changeState()` call entirely, not just
   past the `change_dir()` call. I initially wrote `if (id != X && reverse)
   change_dir(); changeState(...);` (changeState always runs); the target
   skips **both** when `id == X`:
   ```cpp
   if (id != X) {
       if (reverse) change_dir();
       mStateMgr.changeState(StateID_Y);
   }
   return true;
   ```
   Caught by reading the label target address arithmetically (`.L_800C4130`
   sits right after the `changeState` block, not before it) rather than
   trusting the visual proximity of the `beq` to the `change_dir` call.
2. **`mVec2_c tmp.field = base + K;` (a `float` field assigned from a fused
   add expression) schedules registers differently than the target**, even
   though the instruction count and shape are otherwise identical. Splitting
   into three statements --
   ```cpp
   tmp.x = pos.x;
   tmp.y = pos.y;
   tmp.x += 16.0f;
   ```
   -- closed every `mov_frm_*` residual outright (each was 63/63 already,
   only register-numbering differed) and also closed the two `mUnitBasePos.y
   = pos.y + 16.0f`-style cases inside `mov_to_rightupper`. Tried first:
   `mVec2_c tmp(pos.x + K, pos.y)` (constructor form) and `tmp.set(...)` --
   both produced byte-identical (wrong) output to the naive field-assignment
   form, so the fix is specifically the **compound-assignment (`+=`)
   reassociation barrier** from `AGENT_CONTEXT.md`'s lever list, not a
   call-shape change. Swapping the write-order of `tmp.x`/`tmp.y` alone (with
   the fused add still in place) also did NOT help -- only decomposing the add
   into a separate `+=` statement did.
3. **`is_unit_circle2x2`/`is_unit_circle4x4` bool argument is `reverse` for
   the two `right*` functions and `!reverse` for the two `left*` functions,
   consistently.** Confirmed via `cntlzw r0,r30 / srwi r5,r0,5` (a `!bool`
   idiom) appearing before every `calc_rotate_to_circle_*` call in
   `mov_to_leftupper`/`mov_to_leftlower`, and a plain `mr r5,r30` in
   `mov_to_rightupper`/`mov_to_rightlower`.
4. **`change_dir()` polarity around a circle case is not simply tied to
   REV-vs-PREV.** For `right*` functions: REV cases are flipped (`if
   (!reverse) change_dir();`), PREV cases are normal (`if (reverse)
   change_dir();`). For `left*` functions this INVERTS: PREV is flipped, REV
   is normal. And it is not perfectly regular even within one function --
   `mov_to_leftupper`'s `case 6` (`Height`, a plain non-circle case) is
   **flipped** (`if (!reverse) change_dir();`) while its siblings `case 2`
   and `case 5` (same shape, same `id!=X` guard) are normal. Caught only by
   reading each case's `beq`/`bne` byte directly per
   `AGENT_CONTEXT.md`'s rule 5 ("branch polarity: settle it from the target's
   beq/bne bytes, never from guessed semantics") -- a pattern inferred from
   3 of 4 cases would have gotten `mov_to_leftupper` wrong.
5. **Exact 16-bit angle immediates were extracted mechanically, not
   estimated**, because `lis rX,1 / {addi|subi} r0,rX,<imm> / clrlwi
   r4,r0,16` is a "load a value near 0x8000/0xC000 that doesn't fit a signed
   16-bit immediate" idiom and it is easy to mis-arithmetic by hand (I did,
   once, on the first pass -- read `0xc000` where the real value was `0x8000`
   for `mov_to_rightlower`'s two PREV cases; caught by rebuilding the
   computation with a script instead of mental math). Final values, per
   function/case: `mov_to_rightlower` REV=`0x3fff`, PREV=`0x8000`;
   `mov_to_rightupper` REV=`0x7fff`, PREV=`0xC000`; `mov_to_leftupper`
   REV=`0xBFFF`, PREV=`0x0`; `mov_to_leftlower` REV=`0xFFFF`, PREV=`0x4000`.

## Offset-perturbing?

**NO.** Nothing outside the eight assigned function bodies was written. The
`.data` jump tables and the `.sdata2` `16.0f` pool entry are emitted as a
side effect of the function bodies themselves (already accounted for in the
measured `.data`/`.sdata2` bounds from `wip/agent_line_mng_bounds/BOUNDS.md`),
not new declarations of mine.

## Files

- `wip/author_mov/d_line_mng.cpp` -- full draft (state framework + all eight
  bodies), compiles clean, all eight match.
- `wip/author_mov/shadow_include/game/bases/d_line_mng.hpp` -- private header
  copy with the three `LOCAL OVERRIDE`-tagged fixes above. Diff against
  `wip/line_mng_shared/shadow_include/game/bases/d_line_mng.hpp` is exactly
  those three signatures (plus doc comments).
- `wip/author_mov/build.py` -- my compile/disasm/diff driver
  (`compile_draft`/`disasm`/`diff_fn` from `tools/auto_decomp/harness.py`,
  nothing hand-rolled for the compiler invocation itself).

## The eight function bodies (self-contained, for merge)

```cpp
bool dLineMng_c::mov_to_rightlower(ulong id, const mVec2_c &pos, bool reverse) {
    u32 unitID = getLineUnitNo(pos.x, pos.y);
    if (unitID == 0x2) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 0x2) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Right45);
        }
        return true;
    }
    if (unitID == 0xb) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Right30Left);
        return true;
    }
    if (unitID == 0xf) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Right60Up);
        return true;
    }
    if (unitID == 0x11) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0x3fff, reverse);
        } else {
            mAngle = 0x3fff;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2Rightup);
        return true;
    }
    if (unitID == 0x14) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0x8000, reverse);
        } else {
            mAngle = 0x8000;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2LeftDown);
        return true;
    }
    if (unitID == 0x17) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0x3fff, reverse);
        } else {
            mAngle = 0x3fff;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4Rightup);
        return true;
    }
    if (unitID == 0x1b) {
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0x8000, reverse);
        } else {
            mAngle = 0x8000;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4LeftDown);
        return true;
    }
    return false;
}

bool dLineMng_c::mov_to_rightupper(ulong id, const mVec2_c &pos, bool reverse) {
    u32 unitID = getLineUnitNo(pos.x, pos.y);
    switch (unitID) {
    case 1:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 1) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Left45);
        }
        return true;
    case 4:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_CornerSideLine);
        return true;
    case 5:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 5) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Side);
        }
        return true;
    case 9:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Left30Left);
        return true;
    case 13:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Left60Down);
        return true;
    case 18:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0x7fff, reverse);
        } else {
            mAngle = 0x7fff;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2Leftup);
        return true;
    case 19:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0xc000, reverse);
        } else {
            mAngle = 0xc000;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2RightDown);
        return true;
    case 25:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        mUnitBasePos.y += 16.0f;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0x7fff, reverse);
        } else {
            mAngle = 0x7fff;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4LeftUp);
        return true;
    case 31:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        mUnitBasePos.y += 16.0f;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0xc000, reverse);
        } else {
            mAngle = 0xc000;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4RightDown);
        return true;
    }
    return false;
}

bool dLineMng_c::mov_to_leftupper(ulong id, const mVec2_c &pos, bool reverse) {
    u32 unitID = getLineUnitNo(pos.x, pos.y);
    switch (unitID) {
    case 2:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 2) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Right45);
        }
        return true;
    case 5:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 5) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Side);
        }
        return true;
    case 6:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 6) {
            if (!reverse) change_dir();
            mStateMgr.changeState(StateID_Height);
        }
        return true;
    case 10:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Right30Right);
        return true;
    case 14:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Right60Down);
        return true;
    case 17:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0x0, !reverse);
        } else {
            mAngle = 0x0;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2Rightup);
        return true;
    case 20:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0xbfff, !reverse);
        } else {
            mAngle = 0xbfff;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2LeftDown);
        return true;
    case 26:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        mUnitBasePos.x -= 16.0f;
        mUnitBasePos.y += 16.0f;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0x0, !reverse);
        } else {
            mAngle = 0x0;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4Rightup);
        return true;
    case 30:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        mUnitBasePos.x -= 16.0f;
        mUnitBasePos.y += 16.0f;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0xbfff, !reverse);
        } else {
            mAngle = 0xbfff;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4LeftDown);
        return true;
    }
    return false;
}

bool dLineMng_c::mov_to_leftlower(ulong id, const mVec2_c &pos, bool reverse) {
    u32 unitID = getLineUnitNo(pos.x, pos.y);
    switch (unitID) {
    case 1:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 1) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Left45);
        }
        return true;
    case 4:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_CornerHeightLine);
        return true;
    case 6:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (id != 6) {
            if (reverse) change_dir();
            mStateMgr.changeState(StateID_Height);
        }
        return true;
    case 8:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Left30Right);
        return true;
    case 12:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Left60Up);
        return true;
    case 18:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0x4000, !reverse);
        } else {
            mAngle = 0x4000;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2Leftup);
        return true;
    case 19:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        if (is_unit_circle2x2(id) || is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0xffff, !reverse);
        } else {
            mAngle = 0xffff;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle2x2RightDown);
        return true;
    case 22:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        mUnitBasePos.x -= 16.0f;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_prev(0x4000, !reverse);
        } else {
            mAngle = 0x4000;
        }
        if (!reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4LeftUp);
        return true;
    case 28:
        mUnitBasePos.x = pos.x;
        mUnitBasePos.y = pos.y;
        mUnitBasePos.x -= 16.0f;
        if (is_unit_circle4x4(id)) {
            calc_rotate_to_circle_rev(0xffff, !reverse);
        } else {
            mAngle = 0xffff;
        }
        if (reverse) change_dir();
        mStateMgr.changeState(StateID_Circle4x4RightDown);
        return true;
    }
    return false;
}

void dLineMng_c::mov_frm_rightupper(const mVec2_c &pos, bool reverse) {
    u32 id = getLineUnitNo(pos.x, pos.y);
    mVec2_c tmp;
    tmp.x = pos.x;
    tmp.y = pos.y;
    tmp.x += 16.0f;
    if (mov_to_rightlower(id, tmp, reverse)) return;
    tmp.y = pos.y;
    tmp.y += 16.0f;
    if (mov_to_rightupper(id, tmp, reverse)) return;
    tmp.x = pos.x;
    if (mov_to_leftupper(id, tmp, !reverse)) return;
    mStateMgr.changeState(StateID_FallDown);
}

void dLineMng_c::mov_frm_leftlower(const mVec2_c &pos, bool reverse) {
    u32 id = getLineUnitNo(pos.x, pos.y);
    mVec2_c tmp;
    tmp.x = pos.x;
    tmp.y = pos.y;
    tmp.x -= 16.0f;
    if (mov_to_leftupper(id, tmp, reverse)) return;
    tmp.y = pos.y;
    tmp.y -= 16.0f;
    if (mov_to_leftlower(id, tmp, reverse)) return;
    tmp.x = pos.x;
    if (mov_to_rightlower(id, tmp, !reverse)) return;
    mStateMgr.changeState(StateID_FallDown);
}

void dLineMng_c::mov_frm_rightlower(const mVec2_c &pos, bool reverse) {
    u32 id = getLineUnitNo(pos.x, pos.y);
    mVec2_c tmp;
    tmp.x = pos.x;
    tmp.y = pos.y;
    tmp.x += 16.0f;
    if (mov_to_rightupper(id, tmp, reverse)) return;
    tmp.y = pos.y;
    tmp.y -= 16.0f;
    if (mov_to_rightlower(id, tmp, reverse)) return;
    tmp.x = pos.x;
    if (mov_to_leftlower(id, tmp, !reverse)) return;
    mStateMgr.changeState(StateID_FallDown);
}

void dLineMng_c::mov_frm_leftupper(const mVec2_c &pos, bool reverse) {
    u32 id = getLineUnitNo(pos.x, pos.y);
    mVec2_c tmp;
    tmp.x = pos.x;
    tmp.y = pos.y;
    tmp.x -= 16.0f;
    if (mov_to_leftlower(id, tmp, reverse)) return;
    tmp.y = pos.y;
    tmp.y += 16.0f;
    if (mov_to_leftupper(id, tmp, reverse)) return;
    tmp.x = pos.x;
    if (mov_to_rightupper(id, tmp, !reverse)) return;
    mStateMgr.changeState(StateID_FallDown);
}
```
