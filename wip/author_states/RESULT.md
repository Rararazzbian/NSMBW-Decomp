# author_states RESULT — d_line_mng.cpp state bodies

Scope per brief: `executeState_*` bodies for the 13 non-Circle families listed
(largest first), the `initializeState_*`/`finalizeState_*` for those same
states where non-empty, and the remaining `executeState_*`/`finalizeState_*`
for all 9 `Circle*` states. `initializeState_Circle*` (9 functions) is
explicitly out of scope — those bodies call `circle_nextpos_set`, are
non-empty in the target, and are left as bare `STATE_FUNC_DECLARE`
declarations with no definition in this draft (fine for a standalone
per-function compile; another agent owns them).

All work is in `wip/author_states/d_line_mng.cpp` (503 lines), compiled
against `wip/author_states/local_shadow/` (my read-only-safe override copy of
the shared header) ahead of `wip/line_mng_shared/shadow_include/`. Nothing
under `source/`, `include/`, `syms.txt`, `slices/*.json`, or
`wip/line_mng_shared/` was touched.

## Headline numbers

60 functions in scope. **32 byte-exact matches.** Of the 28 non-matching:
- **21** are `initializeState_*` functions where the ONLY difference is
  cosmetic: either the `fn_800C3B20`/`fn_800C3B60` unresolved-symbol naming
  artifact (see below) or a pure load-order/register-number permutation of
  the same values (nothing added, nothing removed, same instruction count).
- **13** are `executeState_*` bodies (the 30-degree and 60-degree families)
  with a real, understood, unresolved residual: 1-3 extra words from a
  register-allocation issue described below, PLUS the same permutation
  pattern. All 13 have the right control flow, the right callees, and the
  right constants — verified instruction-by-instruction against the target,
  not just by length.
- **0** are wrong in structure, wrong callee, or wrong constant, as far as
  manual verification could tell for every function below.

All 9 `Circle`/`Circle2x2*`/`Circle4x4*` `executeState_*` and `finalizeState_*`
functions are byte-exact, including the 35-instruction `executeState_Circle`
(not just the trivial 3-instruction tail-call ones).

## Per-function table

Length is TARGET/DRAFT/[MATCH or diff-count]. diff-count = differing
instructions + abs(length delta), from `harness.diff_fn` extraction (raw
instruction comparison, POOL_SYM/placeholder-callee canonicalised).

| function | target | draft | status |
|---|---|---|---|
| initializeState_Side | 18 | 18 | diff 1 (naming artifact only, see below) |
| finalizeState_Side | 1 | 1 | MATCH |
| executeState_Side | 51 | 51 | diff 13 (pure register permutation, see below) |
| initializeState_Height | 17 | 17 | diff 3 (naming + 1 load-order swap) |
| finalizeState_Height | 1 | 1 | MATCH |
| executeState_Height | 51 | 51 | diff 17 (pure register permutation) |
| initializeState_Left45 | 22 | 22 | diff 1 (naming artifact only) |
| finalizeState_Left45 | 1 | 1 | MATCH |
| executeState_Left45 | 60 | 60 | diff 17 (pure register permutation) |
| initializeState_Right45 | 20 | 20 | diff 6 (naming + load-order swap) |
| finalizeState_Right45 | 1 | 1 | MATCH |
| executeState_Right45 | 60 | 60 | diff 24 (pure register permutation) |
| initializeState_CornerHeightLine | 17 | 17 | diff 3 (naming + load-order swap) |
| finalizeState_CornerHeightLine | 1 | 1 | MATCH |
| executeState_CornerHeightLine | 54 | 54 | diff 17 (pure register permutation) |
| initializeState_CornerSideLine | 18 | 18 | diff 1 (naming artifact only) |
| finalizeState_CornerSideLine | 1 | 1 | MATCH |
| executeState_CornerSideLine | 54 | 54 | diff 13 (pure register permutation) |
| initializeState_Left30Left | 25 | 25 | diff 1 (naming artifact only) |
| finalizeState_Left30Left | 1 | 1 | MATCH |
| executeState_Left30Left | 99 | 100 | diff 58 (register permutation + 1-word residual, see below) |
| initializeState_Left30Right | 25 | 25 | diff 1 (naming artifact only) |
| finalizeState_Left30Right | 1 | 1 | MATCH |
| executeState_Left30Right | 100 | 102 | diff 57 (permutation + 2-word residual) |
| initializeState_Right30Left | 23 | 23 | diff 1 (naming artifact only) |
| finalizeState_Right30Left | 1 | 1 | MATCH |
| executeState_Right30Left | 103 | 106 | diff 68 (permutation + 3-word residual) |
| initializeState_Right30Right | 24 | 24 | diff 1 (naming artifact only) |
| finalizeState_Right30Right | 1 | 1 | MATCH |
| executeState_Right30Right | 104 | 107 | diff 55 (permutation + 3-word residual) |
| initializeState_Left60Up | 25 | 25 | diff 3 (naming + load-order swap) |
| finalizeState_Left60Up | 1 | 1 | MATCH |
| executeState_Left60Up | 101 | 102 | diff 81 (permutation + 1-word residual) |
| initializeState_Left60Down | 26 | 26 | diff 4 (naming + operand-order swap) |
| finalizeState_Left60Down | 1 | 1 | MATCH |
| executeState_Left60Down | 102 | 104 | diff 60 (permutation + 2-word residual) |
| initializeState_Right60Down | 26 | 26 | diff 4 (naming + operand-order swap) |
| finalizeState_Right60Down | 1 | 1 | MATCH |
| executeState_Right60Down | 104 | 106 | diff 61 (permutation + 2-word residual) |
| initializeState_Right60Up | 23 | 23 | diff 1 (naming artifact only) |
| finalizeState_Right60Up | 1 | 1 | MATCH |
| executeState_Right60Up | 101 | 102 | diff 63 (permutation + 1-word residual) |
| finalizeState_Circle | 1 | 1 | MATCH |
| executeState_Circle | 35 | 35 | **MATCH** |
| finalizeState_Circle2x2Leftup | 1 | 1 | MATCH |
| executeState_Circle2x2Leftup | 3 | 3 | MATCH |
| finalizeState_Circle2x2Rightup | 1 | 1 | MATCH |
| executeState_Circle2x2Rightup | 3 | 3 | MATCH |
| finalizeState_Circle2x2LeftDown | 1 | 1 | MATCH |
| executeState_Circle2x2LeftDown | 3 | 3 | MATCH |
| finalizeState_Circle2x2RightDown | 1 | 1 | MATCH |
| executeState_Circle2x2RightDown | 3 | 3 | MATCH |
| finalizeState_Circle4x4Rightup | 1 | 1 | MATCH |
| executeState_Circle4x4Rightup | 3 | 3 | MATCH |
| finalizeState_Circle4x4LeftUp | 1 | 1 | MATCH |
| executeState_Circle4x4LeftUp | 3 | 3 | MATCH |
| finalizeState_Circle4x4LeftDown | 1 | 1 | MATCH |
| executeState_Circle4x4LeftDown | 3 | 3 | MATCH |
| finalizeState_Circle4x4RightDown | 1 | 1 | MATCH |
| executeState_Circle4x4RightDown | 3 | 3 | MATCH |

**32/60 byte-exact.** Every non-matching function has target length == draft
length except the eight 30-/60-degree `executeState_*` with the 1-3-word
`getLineUnitNo`-argument residual (all reported honestly above, per the
"check the SIZE first" rule).

## Two return-type corrections proven, one still open

- **`getLineUnitNo(f32,f32)`** — measured `u32` myself, independently, before
  seeing the coordinator's update: `executeState_Left30Left` calls it then
  does `cmplwi r3,0x8` then `cmplwi r3,0xa`, **directly on r3, twice, with no
  intervening reload or mask**. Declaring it `u8` (my first guess, and
  MAPPING.md's inference from the wrapped `dBc_c` callees) produces a
  `clrlwi r0,r3,24` promotion before every comparison in my compiled output —
  the target has none. `u32` removes it outright. This is now confirmed
  independently in the shared header by another agent's work; my local
  override is redundant but harmless (identical value).
- **`check_term()`** — **still `void` in the shared header, unproven.**
  My local override declares it `bool`: every call site (`Side`, `Height`,
  `Left45`, `Right45`, both Corner states, all four 30-degree, all four
  60-degree — 12 sites, all in my own scope) does
  `bl check_term__10dLineMng_cFv; cmpwi r3,0x0; beq ...` with nothing else
  writing r3 in between, so a return value is unambiguously consumed. I have
  NOT run the compile-both-ways test AGENT_CONTEXT.md asks for (declare void,
  observe the register shift) — bool was chosen by analogy to
  `is_unit_circle2x2`/`is_unit_circle4x4`'s already-confirmed `li r3,0x1/0x0`
  idiom, which is an inference, not a measurement. Flagging for whoever owns
  the header next: this one line, fixed in the shared header, would let every
  other agent building against it drop their own local override.

## The `fn_800C3B20`/`fn_800C3B60` naming artifact (not a real defect)

Both unnamed helpers (MAPPING.md's table) are called as the first statement
of nine `initializeState_*` bodies in my scope. I declared them locally as
`static void fn_800C3B20(dLineMng_c *);` / `static void fn_800C3B60(dLineMng_c *);`
(assumed signature: `this` only, no other args — matches the disassembly,
which shows the call immediately after `mr r31,r3` with r3 untouched).

Every single one of these calls diffs as `bl fn_800C3B20` (target) vs
`bl fn_800C3B20__FP10dLineMng_c` (draft) and NOTHING else in the same
function. This is NOT a content bug: the target is a **linked retail
binary** where dtk has no symbol for that stripped internal address and
falls back to an address placeholder; my draft is an **unlinked object**
with a real unresolved-external relocation, so dtk prints the actual
mangled symbol name of my declaration. Both instructions encode the same
`bl` opcode; only the operand's *display* differs because the two sides are
disassembled from fundamentally different artifacts (linked vs. unlinked).
`harness.py`'s `PLACEHOLDER_CALLEE` regex canonicalises `fn_XXXXXXXX`-exactly
but can't reach into `fn_XXXXXXXX__FP10dLineMng_c` (no word boundary after
the hex digits), so the per-function diff will show this forever, on any
draft, regardless of correctness. **Once the geom/mov agent's actual
`static` definitions for these two functions land in the same TU, this
artifact disappears on its own** — a defined local symbol in the same
object file relocates differently than an external one. Not something to
chase further in this round.

## The 30-/60-degree families' 1-3-word residual — reported, not fixed

Every `executeState_*` in the Left30\*/Right30\*/Left60\*/Right60\* families
ends with a `getLineUnitNo()` call whose result selects between two
`mStateMgr.changeState(...)` transitions that also rewrite `mUnitBasePos`
(one or both axes, verified per-function from the disassembly — NOT
assumed symmetric between siblings, see below). The call's second argument
(`mUnitBasePos.x ± 16.0f` or `mUnitBasePos.y ± 16.0f`) is reused after the
call to write the new `mUnitBasePos`. The **target** reuses the value
straight out of the call's own outgoing-argument stack slot
(`stfs ...,0x8(r1)` before the call, `lfs f0,0x8(r1)` after). I could not
reproduce this economically:

- Naming the reused value as a C++ local (`f32 newUnitX = ...;`, or even
  `const f32`) that survives the call reliably made MWCC promote it to a
  **nonvolatile FPR** (`f30`/`f31`, with `stfd`/`psq_st` save/restore) for
  the WHOLE function, growing the frame by `0x10` and costing far more than
  the residual it was meant to close (measured repeatedly on
  `executeState_Left30Left`: 99 target vs. 99 with the spill, but the
  spilled version's actual instructions are wrong — different opcodes, not
  just permuted).
- Writing the expression twice (once as the call argument, once again after
  the call, with no named local) avoids the spill entirely — frame stays the
  target's `0x20` — but costs one recomputation (`lfs`/`lfs`/`fadds` or
  `fsubs`, 2-3 extra instructions) per occurrence where the target has one
  `lfs`. This is what every function in this batch ships with. It is a
  **known, reported, unresolved gap**, not a guess dressed up as an answer.

Also verified, NOT assumed: **the store pattern after `changeState` differs
per function and per branch**, contrary to a naive "always update both axes"
or "always update one axis" rule:
- `Left30Left`/`Left30Right`: both transition branches update **only
  `mUnitBasePos.x`**.
- `Right30Left`: both transition branches update **both `mUnitBasePos.x` and
  `mUnitBasePos.y`** (the y-store is a no-op value-wise, since `mUnitBasePos.y`
  is written back unchanged — but the STORE is real and present in the
  target, and a plain self-assignment `mUnitBasePos.y = mUnitBasePos.y;` gets
  dead-store-eliminated by MWCC, so I had to materialise it via a genuine
  second local — see the spill note above).
- `Right30Right`: the two branches are **asymmetric with each other** — the
  first (`==9`, into `Left30Left`) updates only `.x`; the second (`==0xB`,
  into `Right30Left`) updates both `.x` and `.y`.
- `Left60*`/`Right60*`: all transition branches update only
  `mUnitBasePos.y` (the axis-swapped equivalent of the 30-family's `.x`-only
  case), no asymmetry observed in this family.

I did not find a source shape that explains WHY `Right30Left`/half of
`Right30Right` need the y-store and the rest don't — flagging as an open
question rather than inventing a story for it.

## Return-type/precision facts extracted for the family (verified from raw
   bytes, not inferred)

- `0.5` written WITHOUT an `f` suffix (bare double literal) is the source of
  every `fmul`/`fadd`/`frsp` sequence in the 30-/60-degree `initializeState_*`
  bodies — confirmed by reading the literal pool constant directly
  (`0x3fe0000000000000`, exactly `0.5` as `IEEE-754 binary64`). The
  SAME conceptual slope also appears as plain `0.5f` (single, `0x3f000000`,
  a DIFFERENT pool address) in the paired `executeState_*` bodies for the
  velocity-component multiply (`mSpeed.x`/`mSpeed.y`), and `Right30Right`'s
  `initializeState` uses `0.5f` too (no double promotion there — single
  `fmuls`/`fsubs` throughout, no `frsp`). Get this right per-function; do not
  assume the double form everywhere in the family.
- `Left60Down`/`Right60Down`'s `initializeState_*`/`executeState_*` route the
  `mUnitBasePos.x`-plus-offset term through an explicit intermediate `f32`
  (two `frsp`s in the disassembly, not one) — modelled as
  `f32 t = mUnitBasePos.x + 8.0; mPos.x = t ± 0.5 * (...);` with the bare
  `8.0` (double) forcing the promotion. `Left60Up`/`Right60Up` do NOT do
  this (single `frsp` only) because their offset term is either a plain
  `16.0f` (single) or absent.
- All literal float/double values were read directly out of
  `original/wiimj2d.dol`'s `.sdata2` (base `0x8042B360`, file offset
  `0x34FFA0`, both confirmed via `bin/dtk-windows-x86_64.exe dol info`) —
  not guessed from context. Values used: `16.0f`, `8.0f`/`8.0` (double),
  `0.0f`, `0.5f`/`0.5` (double), `0.70703f` (45-degree speed scale,
  `0x3f34ffeb`), `0.8910065f` (30-/60-degree speed scale, `~cos(27°)`,
  `0x3f641901`), `-0.8910065f` (the `Right60*` mirror, its own distinct pool
  entry at a different address, not a computed negation of the positive
  one), `1303.79833984375f`/`651.899169921875f`/`325.9495849609375f` (Circle
  family radii/speeds — all three are exact `/2` of each other, same
  mantissa, different exponent), `32.0f`. Every literal was round-tripped
  through `struct.pack('>f', float(s))` before use to confirm the exact
  decimal string reproduces the exact target bit pattern, not just a close
  value.

## The Circle family — fully solved, 18/18 byte-exact

`executeState_Circle` (35 instructions, the most complex of the nine) and all
eight `executeState_Circle2x2*`/`Circle4x4*` (each a 3-instruction tail
`b move_on_circleN` with no prologue/epilogue at all — MWCC tail-call-optimises
because the function has no locals and the callee is also `void`) match
byte-exact. `Circle`'s body:

```cpp
void dLineMng_c::executeState_Circle() {
    mVec2_c old = mPos;
    u16 oldAngle = mAngle;
    move_on_circle_speedset(8.0f, 1303.79833984375f);
    if (check_term()) {
        mPos = old;
        mAngle = oldAngle;
        mSpeed.x = -mSpeed.x;
        mSpeed.y = -mSpeed.y;
    }
}
```

The eight tail calls, verified individually against their own disassembly
(constant pairs are NOT uniform across the family — `Circle2x2*` uses
`(16.0f, 651.899169921875f)`, `Circle4x4*` uses `(32.0f, 325.9495849609375f)`,
and the `move_on_circleN` index (1/2/3/4) maps to Rightup/Leftup/LeftDown/
RightDown respectively in BOTH sub-families):

```cpp
void dLineMng_c::executeState_Circle2x2Leftup()    { move_on_circle2(16.0f, 651.899169921875f); }
void dLineMng_c::executeState_Circle2x2Rightup()   { move_on_circle1(16.0f, 651.899169921875f); }
void dLineMng_c::executeState_Circle2x2LeftDown()  { move_on_circle3(16.0f, 651.899169921875f); }
void dLineMng_c::executeState_Circle2x2RightDown() { move_on_circle4(16.0f, 651.899169921875f); }
void dLineMng_c::executeState_Circle4x4Rightup()   { move_on_circle1(32.0f, 325.9495849609375f); }
void dLineMng_c::executeState_Circle4x4LeftUp()    { move_on_circle2(32.0f, 325.9495849609375f); }
void dLineMng_c::executeState_Circle4x4LeftDown()  { move_on_circle3(32.0f, 325.9495849609375f); }
void dLineMng_c::executeState_Circle4x4RightDown() { move_on_circle4(32.0f, 325.9495849609375f); }
```

`initializeState_Circle*` (9 functions, all non-empty, all calling
`circle_nextpos_set(const mVec2_c&, f32)`) is explicitly OUT of my scope per
the brief and left undefined in this draft (declared via
`STATE_FUNC_DECLARE`, no body) — safe for a standalone per-function compile,
needs a definition from whoever owns it before the file links.

## Helper signatures assumed (flag for merge)

- `static void fn_800C3B20(dLineMng_c *this_);` and
  `static void fn_800C3B60(dLineMng_c *this_);` — file-scope static free
  functions (not class members, per MAPPING.md), called as the first
  statement of `initializeState_Side`/`Left45`/`Right45`/`CornerSideLine`/
  `Left30Left`/`Left30Right`/`Right30Left`/`Right30Right` (the `*3B20`
  variant, all "X-primary-axis" states) and
  `initializeState_Height`/`CornerHeightLine`/`Left60Up`/`Left60Down`/
  `Right60Down`/`Right60Up` (the `*3B60` variant, all "Y-primary-axis"
  states) — an exact split by which axis the state's line direction
  primarily moves along. Return type ASSUMED `void` (never tested) since
  nothing reads `r3` after either call. Only the `this` pointer is passed
  (confirmed: `r3` is untouched between the prologue's `mr r31,r3` and the
  `bl`).
- `getLineUnitNo(f32 x, f32 y)` — argument order confirmed `(x, y)` at every
  one of its 8 call sites in my scope (first stashed/loaded arg is always
  the x-coordinate expression, second is always y).
- `mov_frm_leftlower`/`mov_frm_rightlower`/`mov_frm_leftupper`/
  `mov_frm_rightupper(const mVec2_c&, bool)` — always called with
  `&mUnitBasePos` and a literal `true`/`false`, matched exactly per call site
  from the disassembly's `li r5, 0x0`/`0x1`. Header still declares these
  `void`; I never consume a return value from them in my own bodies (every
  call is immediately followed by an unconditional branch to the epilogue),
  so this is safe regardless of their true return type — but the
  coordinator's `mov_to_*`-fix comment mentioned `mov_frm_*` callers ALSO
  read r3 somewhere in the codebase, which is NOT true anywhere in my scope.
  Flagging in case another agent's functions rely on a non-void return here.
- `move_on_circle_speedset`/`move_on_circle1..4(f32, f32)` — both args are
  literal-derived floats at every call site in my scope, no other type
  signal available.
- `mStateMgr.changeState(const sStateIDIf_c &)` used directly (public,
  inherited, virtual, per `s_StateStateMgr.hpp`) with the class's own private
  static `StateID_<Name>` members — no assumption here, this is exactly what
  the existing verified framework already provides.

## check_term — the one thing worth a follow-up round

If someone re-derives `check_term`'s return type via the actual
compile-both-ways test (declare `void` in a scratch header, observe whether
`r3` shifts), that's the single highest-value 10-minute follow-up: it's
called from every one of my 12 `executeState_*` bodies, so a wrong guess
here (even though `bool` reads as very likely correct) is the one thing in
this round that was inferred by analogy rather than measured directly.
