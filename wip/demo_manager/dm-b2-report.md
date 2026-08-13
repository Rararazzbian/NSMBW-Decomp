# Batch 2/6 report: the goal-pole demo sequence

All 6 functions are byte-exact, verified against the full standard (address
extraction, size-vs-symbol-map check, text+symbol-name comparison, raw
`.rodata`/`.sdata2` byte comparison, and a negative control). File:
`wip/demo_manager/dm-b2.cpp`.

| Addr | Function | Bytes | Result |
|---|---|---:|---|
| 0x8005B8A0 | `executeGoalDemo_Pole` | 1100 | **byte-exact** |
| 0x8005BCF0 | `executeGoalDemo_PoleDown` | 108 | **byte-exact** |
| 0x8005BD60 | `executeGoalDemo_JumpCheck` | 208 | **byte-exact** |
| 0x8005BE30 | `executeGoalDemo_Jump` | 216 | **byte-exact** |
| 0x8005BF10 | `executeGoalDemo_Land` | 180 | **byte-exact** |
| 0x8005BFD0 | `executeGoalDemo_KimeWait` | 192 | **byte-exact** |

Verification performed, per function, beyond the harness's text diff:
- Object's emitted function sizes checked against `elf info`: 0x6C, 0xD0,
  0xD8, 0xB4, 0xC0, 0x44C — matches the brief's table exactly (108, 208,
  216, 180, 192, 1100 bytes).
- `executeGoalDemo_Pole`'s two local-array initializers and five float
  constants were read directly out of `original/wiimj2d.dol` as raw bytes
  (not inferred from the canonicalised text diff, which cannot see a wrong
  constant): both 4-int arrays are `{-1,-1,-1,-1}` (0x802EEEA0, 0x802EEEB0),
  and the five `.sdata2` floats at 0x8042BCF8..0x8042BD08 are `0.0f, 6.0f,
  2.0f, 4.0f, 0.7f` in that order. Confirmed a second time by dumping my own
  compiled object's `.rodata`/`.sdata2` bytes and diffing them against the
  same DOL bytes -- exact match, not just "same pattern of references."
- Negative control: hand-corrupted one instruction line in the extracted
  draft body and confirmed `want == got` correctly turns `False`; also
  cross-compared `Land`'s target against `KimeWait`'s draft (definitely
  different functions) and confirmed no false-positive match.
- Compiled the file against the **real** `include/` tree (no scratch
  override) to confirm the only failures are the two known gaps below, and
  nothing else regresses.

## Two blocking cross-file gaps (found, not resolved -- flagging per the brief)

Both are real methods the target binary calls that are simply **not declared**
in this repo's already-existing (not-frozen, but out of this batch's scope)
headers. I could not fix these without editing `include/`, which the hard
rules forbid, so I verified the rest of the file by compiling against a
**scratch-only** copy of the two headers (`wip/demo_manager/dm-b2.cpp` itself
is untouched -- it includes the real headers only) and confirmed each gap is
exactly a missing declaration, nothing structural:

1. **`SndSceneMgr::startGoal(bool)`** (symbol `startGoal__11SndSceneMgrFb`),
   called from `executeGoalDemo_Jump`. Not declared in
   `include/game/snd/snd_scene_manager.hpp` (which currently only has
   `moveMissFin`/`FUN_8019d5b0`/`fn_8019be60`/`fn_8019bd90` plus `mPad1`,
   `m_10`, `m_14`, `sInstance`).
2. **`dScStage_c::ReplayEnd()`** (symbol `ReplayEnd__10dScStage_cFv`), called
   from `executeGoalDemo_Pole`. Not declared in
   `include/game/bases/d_s_stage.hpp`.

Whoever owns those headers next just needs to add the two declarations
(`void startGoal(bool);` on `SndSceneMgr`, `static void ReplayEnd();` on
`dScStage_c`); this batch's logic around both call sites is already proven
byte-exact against them.

## One header/field contradiction (flagged, worked around locally)

`daPyDemoMng_c::m_18` is declared `u32` in the frozen
`d_a_player_demo_manager.hpp`, but `executeGoalDemo_Jump`'s target
disassembly does a **signed** `cmpwi`/`bge` comparison of `m_18` against 0,
which is only meaningful (and only emits any code at all -- an unsigned
`>=0` is a compile-time tautology MWCC just optimizes away, which is
literally what happened on the first attempt) if the field is treated as
signed at that use site. Did not touch the frozen header; worked around with
a local `(int)m_18` cast at the one comparison that needs it. Flagging per
"if a finding contradicts the header, stop and report it" -- this doesn't
block anything, just recording it in case the field turns out to be `int`
rather than `u32` when another batch/consumer looks at it.

## Unnamed `dAcPy_c` fields used (raw casts, all offsets proven by this TU's own bytes)

Six small `*_ref()` helper functions in `dm-b2.cpp` wrap raw offset casts
into `dAcPy_c`, since these fields are not yet named in the frozen player
headers and this batch cannot edit `include/`:
- `+0x430` (int) -- per-player "pole slot" index, read in `executeGoalDemo_Pole`.
- `+0x434` (int) -- written in `executeGoalDemo_Jump` with the remaining
  jump-queue count.
- `+0x38c` (u8) -- a ground/contact-type byte, read right after a
  `checkGround()` call in `executeGoalDemo_Pole`; confirmed NOT the same as
  `dActor_c::mLayer` (that's the already-named field at `+0x38f`, used
  directly as `ctrlPl->mLayer`).
- `+0x438` (float) -- written in `executeGoalDemo_Pole`, just past `mPos`
  (`+0xac`).
- `+0x1090` (int) -- read in `executeGoalDemo_Pole`, compared to 0 and 3.
- `+0x1030` (float) -- read in `executeGoalDemo_Pole`, multiplied into the
  height accumulator by the 0.7f constant.

## Levers that actually mattered (for whoever reads this next)

- **`daPyMng_c::checkPlayer(i)`**, not a hand-written
  `mActPlayerInfo & (1<<i)`: the inline accessor's implicit `u8` truncation
  is what produces the `clrlwi` the target has before the `lbz`/`slw`.
  Matches the coordinator's batch-4 relay; confirmed independently here on
  all four loop-shaped functions before that relay arrived.
- **Hoist the loop-body `dAcPy_c *` pointer out of the `for`**: fixed an
  i/ctrlPl register-order swap in every one of this batch's loop functions,
  including both loops inside `executeGoalDemo_Pole`. Also matches the
  coordinator's relay; found independently first.
- **"break" vs "return" is not interchangeable even when both compile**:
  `executeGoalDemo_JumpCheck` and `executeGoalDemo_Land` looked like
  `break`-out-of-loop-then-fall-into-shared-tail-code (like `KimeWait`,
  which really is that shape), but their early exit actually skips the
  tail entirely and jumps straight to the epilogue -- i.e. it's a `return`,
  not a `break`. Reading the raw branch target address (not just "does it
  branch") is what caught this; a size-only diff (52 vs 52 instructions)
  can still hide a wrong branch target.
- **A variable whose address is passed to a callee cannot live in a
  register across that call.** `executeGoalDemo_Pole`'s height accumulator
  is the *same* stack slot as `checkGround()`'s `float *` output parameter
  (its address escapes into that call), so the target keeps it fully
  memory-resident (`lfs`/`stfs` to a fixed stack offset every time) instead
  of promoting it to a saved FPR. Writing it as two separate locals
  (`groundY` + `heightAccum`) let the compiler promote the accumulator to
  `f29`, which cost an extra callee-saved-FPR prologue/epilogue pair (8
  instructions) the target doesn't have. Using one variable for both roles
  fixed it immediately.
- **Constants must be read as bytes, not guessed and shape-matched.** The
  text diff cannot see a wrong float literal (it canonicalises pool
  references to `SYM0`, `SYM1`, ... by first appearance). Two placeholder
  guesses (`64.0f`/`32.0f`/`16.0f`/`-1.0f`) happened to produce the same
  *pattern* of five distinct symbol references as the real values
  (`0.0f`/`6.0f`/`2.0f`/`4.0f`/`0.7f`) until a literal got accidentally
  reused (`32.0f` twice) and collapsed two pool slots into one, which is
  what actually surfaced the problem. Read `original/wiimj2d.dol` directly
  once real byte-level uncertainty showed up, rather than trial-and-error
  guessing values.
- **A pure two-register FPR rotation** (`f0`/`f1` swapped, same
  instructions/order otherwise) in the per-player height-cap comparison
  closed by introducing a named local (`float accum = heightAccum;`) held
  *before* computing the other operand (`cap`), i.e. forcing load order via
  an extra named binding -- consistent with the brief's stated lever, found
  by direct sweep of a few spellings (addition order, comparison polarity,
  named locals) rather than a single guess.
