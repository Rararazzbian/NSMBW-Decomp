# Work order — round 38

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## `dIceEfMaker_c` LANDED — 6/6, third in a row

    ACCEPTED -- all five binaries are byte-identical to the original.
    Total: Decompiled 740136/6500368 code bytes (11.386%)

Three units, three rounds, all 100%. 11.353% → 11.363% → 11.373% → 11.386%.

Your round survived the machine going to sleep mid-run: the process was killed
with exit code -1, but you had already written your results, so the work was
complete and I only had to find it. Your layout table for `dIceEfMaker_c` and
`dIceEfScale_c` was right on every member.

### One thing that cost a rejected landing, so you should know it

Your `.cpp` was correct. The landing still failed, at **link** time:

    undefined: 'dEffActorMng_c::createIceFragEff(mVec3_c&, unsigned long, signed char)'

You had declared that function in the shadow header — correctly, it is real and
the unit calls it — and that satisfied the *compiler*. But the function itself
has not been decompiled yet, so the **linker** had no address to bind the call
to. The fix was to register its address in `syms.txt`:

    createIceFragEff__14dEffActorMng_cFR7mVec3_cUlSc=0x80092720

**So a landing manifest needs three things, not two.** New headers, shared
header changes — and **every external symbol your unit calls that is not yet
decompiled, with its address**. You can find the address with
`search_symbols`. Include that list in your manifest from now on; I cannot
land without it and it is faster for you to produce than for me to chase.

---

## Round 38 — two small self-contained classes

Both are complete contiguous TUs. I verified each with `search_symbols`: every
symbol of the class falls in one uninterrupted address run, and the function
immediately after each run belongs to a different class.

Work in **`scratch/gemini_shake/`**. Do both. They are independent, so if one
fights you, bank the other.

### Unit A — `dPosShake_c`, 0x800D81A0, 304 B, 3 functions

     28 B  init__11dPosShake_cFffffff
    244 B  move__11dPosShake_cFv
     16 B  startShake__11dPosShake_cFf

Target already prepared for you:
`tools/auto_decomp/work/dol_bases_d_pos_shake/target.txt`
(range `0x800D81A0-0x800D82D0`; boundary checked — preceded by
`sFStateID_c<dPosGoAndComeExeFrm_c>` templates, followed by `dPropelParts_c`).

### Unit B — `dRotShake_c`, 0x800DF950, 300 B, 2 functions

     40 B  init__11dRotShake_cFssssssss
    252 B  move__11dRotShake_cFv

Target already prepared:
`tools/auto_decomp/work/dol_bases_d_rot_shake/target.txt`
(range `0x800DF950-0x800DFA78`; boundary checked — followed by `dsChrLib`).

---

## What I already extracted from the binary, so you do not re-derive it

**Both classes are entirely self-contained.** Across all five functions there is
**not a single `bl` call**, and every function is a leaf with **no stack frame**
— no `stwu r1, -0xN(r1)`, no callee-saved GPR or FPR spills. So there are no
external dependencies to declare, no `syms.txt` entries needed, and no
frame-shape puzzles. This is pure layout-and-expression work.

### `dPosShake_c` — all six members are `f32`

| Offset | Type | Evidence |
|---|---|---|
| 0x0  | f32 | `stfs f4, 0x0(r3)` in init; `lfs f1, 0x0(r3)` / `stfs f1, 0x0(r3)` in move |
| 0x4  | f32 | `stfs f5, 0x4(r3)`; also the only member `startShake` touches |
| 0x8  | f32 | `stfs f1, 0x8(r3)`; `lfs f0, 0x8(r3)` in move |
| 0xc  | f32 | `stfs f2, 0xc(r3)`; read twice in move |
| 0x10 | f32 | `stfs f6, 0x10(r3)`; `lfs f2, 0x10(r3)` in move |
| 0x14 | f32 | `stfs f3, 0x14(r3)`; `lfs f0, 0x14(r3)` in move |

Highest offset touched is 0x14, so the class is at least 0x18 bytes.

**The `init` argument mapping is scrambled, and this is the interesting part:**

    f1 -> 0x8     f2 -> 0xc     f3 -> 0x14
    f4 -> 0x0     f5 -> 0x4     f6 -> 0x10

The parameters are *not* stored in declaration order. Read that as evidence
about what the members mean: the first three arguments go to the second half of
the object and the last three to the first half. A plausible reading is that
`init` takes (something, something, something, current, target, something) and
the declaration order of the members groups them differently — work out which
grouping makes `move` read naturally, because `move` is the function that has to
match and its expression shapes will tell you the semantics.

**`move` control flow:** no loop, no early return. A single straight-line chain
of forward conditional branches — an if / else-if decision tree — that computes
a new value for the member at 0x4 and then for the member at 0x0, then falls
through to one store-and-return at the end. The comparisons are all `fcmpo`
against members, several using the `cror` idiom:

    fcmpo cr0, f3, f2 / bge .L_800D81FC
    fcmpo cr0, f3, f2 / cror eq, gt, eq / bne .L_800D8214
    fcmpo cr0, f3, f2 / cror eq, lt, eq / bne .L_800D8214
    fcmpo cr0, f3, f0 / cror eq, gt, eq / bne .L_800D822C
    fcmpo cr0, f1, f0 / bge .L_800D8288

`cror eq, gt, eq` after `fcmpo` is `>=` and `cror eq, lt, eq` is `<=`. A plain
`bge` with no `cror` is a `<` written the other way round. Getting these the
right way round is most of this function.

One pooled float constant, referenced twice in `move`:

    lfs f2, "@46223_8042CDE8"@sda21(r0)
    lfs f0, "@46223_8042CDE8"@sda21(r0)

Same symbol both times, so it is one literal used twice — likely `0.0f` or a
small clamp value. Deduce it from the comparisons it takes part in.

### `dRotShake_c` — eight `s16` members and one `int` flag

| Offset | Type | Evidence |
|---|---|---|
| 0x0  | s16 | `sth r9, 0x0(r3)` in init; `lha r6, 0x0(r3)` / `sth r6, 0x0(r3)` in move |
| 0x2  | s16 | `sth r8, 0x2(r3)`; `lha r5, 0x2(r3)` / `sth r5, 0x2(r3)` in move |
| 0x4  | s16 | `sth r5, 0x4(r3)`; `lha r0, 0x4(r3)` in move |
| 0x6  | s16 | `sth r6, 0x6(r3)`; `lha r4, 0x6(r3)` in move |
| 0x8  | s16 | `sth r7, 0x8(r3)`; `lha r4, 0x8(r3)` in move |
| 0xa  | s16 | `sth r10, 0xa(r3)`; `lha r8, 0xa(r3)` in move |
| 0xc  | s16 | `sth r0, 0xc(r3)`; `lha r9, 0xc(r3)` in move |
| 0xe  | s16 | `sth r4, 0xe(r3)` — **init only, never read by move** |
| 0x10 | int | `stw r7, 0x10(r3)` in move, where r7 is only ever `li r7,0` or `li r7,1` |

Highest offset touched is 0x10 as a word, so the class is at least 0x14 bytes.

**`init` argument mapping, also scrambled** (`this`=r3, args are r4..r10 then one
on the caller's stack):

    r4 (arg1) -> 0xe        r5 (arg2) -> 0x4       r6 (arg3) -> 0x6
    r7 (arg4) -> 0x8        r8 (arg5) -> 0x2       r9 (arg6) -> 0x0
    r10 (arg7) -> 0xa       arg8 -> 0xc   (loaded with `lha r0, 0xa(r1)`)

The eighth argument arrives on the stack because only r4..r10 are available for
integer parameters after `this` — that is normal, not a clue about its type.

**`move` control flow:** forward branches only, no loop, single exit
(`mr r3, r5` then `blr` — so it **returns a value**, and that value is the
member at 0x2, which is also written back). Structure: first a modular
increment/decrement of the value at 0x0 against bounds, then a bounds check on
the value at 0x2 that sets the 0/1 flag at 0x10.

    extsh. r0, r6 / bge .L_800DF9BC
    extsh. r0, r6 / ble .L_800DF9CC        (after add r6,r6,r4)
    extsh. r0, r6 / bge .L_800DF9CC        (after subf r6,r4,r6)
    cmpw r0, r4 / ble .L_800DF9E4
    cmpw r0, r4 / bge .L_800DF9F4          (after neg r4,r4)
    cmpwi r5, 0x0 / bge .L_800DFA44

Note `move` **does not return void** — check the retail signature. If the
declaration in your header says `void`, `mr r3, r5` will never appear.

---

## Order of work

1. **`dRotShake_c::init`** — 40 bytes, and the mapping above is already solved.
   Bank it first.
2. **`dPosShake_c::init`** (28 B) and **`startShake`** (16 B) — also nearly free.
3. **`dRotShake_c::move`** and **`dPosShake_c::move`** — the two real ones.

Two standing rules:

- **Function definition order is part of the object.** Define them in address
  order within each file, not grouped logically.
- **Compile or it did not happen.** Every function you report gets a real object
  and a real `fndiff.py` line.

**Include style is a hard rule:** `#include <game/bases/d_pos_shake.hpp>`, never
`#include "d_pos_shake.hpp"`. Put your headers under
`scratch/gemini_shake/shadow/game/bases/` so the angle-bracket form resolves in
scratch and after landing both.

---

## Completeness

Both units are in scope. If a task's details seem incomplete, that is not
permission to skip it — make the most reasonable interpretation, do it, and
state your assumption. A task is only "not done" if you ran out of budget, and
then name it in a `NOT REACHED` list at the end.

## Constraints

Work only in `scratch/gemini_shake/`. `prepare.py` writes to
`tools/auto_decomp/work/`, which is its own output directory and fine; do not
edit anything else under `tools/**`. Do not touch any other `scratch/`
directory, `wip/**`, `source/**`, `include/**`, `slices/`, `syms.txt`,
`configure.py`, `QWEN_*`, `CODEX_HANDOFF.md`, or `HANDOFF.md`.
**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`** — I land.

---

## Reporting

- **Lead with two counts: `DIFFS 0` out of 3 for `dPosShake_c`, and out of 2 for
  `dRotShake_c`.**
- The final layout table for each class, with any member you renamed or retyped
  from my table above and why.
- Per-function results with `fndiff.py` lines.
- **GAINED and LOST by name.**
- A landing manifest per unit: slice `memoryRanges`, the new header, any shared
  header change classified as additive or modifying, and **any external symbol
  with its address** (I expect none for these two — say so explicitly).
- `NOT REACHED`, if anything.
