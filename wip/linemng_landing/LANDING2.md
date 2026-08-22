# `d_line_mng.cpp` landing readiness — round 2

Working copy: `wip/linemng_landing/` (forked from `wip/fix_bigtwo/`, Aug 22).
Experiments preserved under `wip/linemng_landing/experiments/<id>/`.
No build was run. No shared header, `slices/*.json` or `syms.txt` was edited.

## VERDICT: **NO-GO** — one blocker remains (`smc_UNIT_SIZE_X`).

Two of the three items in the brief are CLOSED. The third is not, and it is a
hard blocker: the object is 4 bytes short in `.sdata2` and every pooled constant
in the unit sits 4 bytes low.

---

## 0. Baseline reproduced

```
python wip/line_mng_shared/tally.py wip/linemng_landing/d_line_mng.cpp wip/linemng_landing/shadow_include
(7 paired by CONTENT -- unnamed target vs mangled draft name)
matched 181/182 functions   7531/7631 words = 98.7% BY BYTES
  100w   LEN OK   line_cross_chk2__10dLineMng_cFfRC7mVec2_c7mVec2_c7mVec2_cRf
```

Confirms the survey. `line_cross_chk2` untouched (another agent owns it).

---

## 1. FUNCTION ORDER — **CLOSED, 14 violations -> 0**

`wip/linemng_landing/order_check.py <draft_disasm.txt>` (parameterised rewrite of
the survey's script).

| state | violations | tally |
|---|---:|---|
| baseline | **14** | 181/182 |
| + inline `~dLineMng_c() {}` | 9 | 181/182 |
| + `dummy()` in `dLineMng_c` | 1 | 181/182 |
| + `refreshState` moved in `s_StateStateMgr.hpp` | **0** | 181/182 |

### Block 1 — the six destructors (6 of the 14)

MWCC emits the base-destructor cascade **base-first** when the CONSTRUCTOR is
what first pulls the vtables in, and **derived-first** when a DESTRUCTOR is.
Proven from the landed, byte-exact `source/dol/bases/d_actor_state.cpp`, which
contains one class of each kind in a single TU:

* `dActorState_c` — ctor defined in the .cpp -> `__ct__`, then the cascade
  `sFState_c, sFStateFct_c, sStateMgr_c, sFStateMgr_c` (base-first), then `__dt__`.
* `dActorMultiState_c` — ctor inline in the class, **not emitted**; dtor in the
  .cpp -> `__dt__`, then `sFStateStateMgr_c, sStateStateMgr_c, sFStateMgr_c,
  sStateMgr_c, sFStateFct_c, sFState_c` (derived-first).

Retail `d_line_mng` has `__ct__` first AND the derived-first cascade, which needs
both triggers. **Giving `dLineMng_c` an inline, in-class destructor does exactly
that**: the class becomes destructible, the implicit chain is instantiated in
destruction order, and `__dt__10dLineMng_cFv` itself is weak, unreferenced and
never emitted (verified: absent from the object).

Measured emission order after the fix (weak symbols retail resolves elsewhere
shown in brackets — they are not placed):

```
__ct__10dLineMng_cFv, [__ct__7mVec2_cFv, ~EGG::Vector2f, ~mVec2_c],
__dt__77sFStateStateMgr_c, __dt__91sStateStateMgr_c, __dt__49sFStateMgr_c,
__dt__79sStateMgr_c, [~sStateMethodUsr_FI_c], __dt__26sFStateFct_c,
__dt__23sFState_c, [...], init__10dLineMng_cFRC7mVec2_cfiUc
```
which is retail's placed order exactly (`0x800C0DC0, ED0, F60, FF0, 1060, 10C0, 1100, 1140`).

Rejected variants, all measured:
* out-of-line `~dLineMng_c()` after the ctor — cascade unchanged (base-first), dtor emitted after it;
* out-of-line `~dLineMng_c()` **before** the ctor — cascade becomes derived-first but `__ct__` is pushed behind it (wrong);
* declared-only destructor — no effect at all.

### Block 2 — the 13 framework accessors (8 of the 14)

MWCC emits a vtable-only inline member **at end of TU, in class DECLARATION
order**; a member the TU actually CALLS is emitted immediately after the calling
function. Confirmed three ways (E5/E6/E7 in `experiments/`): moving the *definition*
out of the class does **not** move the emission point, only the declaration does.

Retail needs `isSubState` (slot 12) emitted 3rd and `changeToSubState` (slot 10)
emitted 12th — declaration order and slot order must DISAGREE, which the current
header comment correctly calls impossible for a single class. Two independent
levers together resolve it:

1. **A `dummy()` member in `dLineMng_c`** — the same device
   `d_actor_state.hpp` already uses for both its classes. Its body is never
   emitted, but the calls it makes queue those members ahead of the
   declaration-order leftovers:

   ```cpp
   /// @cond
   void dummy() {
       mStateMgr.initializeState();
       mStateMgr.finalizeState();
       mStateMgr.isSubState();
       mStateMgr.returnState();
       mStateMgr.getOldStateID();
   }
   /// @endcond
   ```
   -> emits `initializeState, finalizeState, isSubState, returnState,
   getOldStateID`, then the `sFStateFct_c`/`sFState_c` block
   (`build, dispose, initialize, execute, finalize`), which is retail's order
   through `0x800C7250..0x800C7430`. 14 -> 1 violation.

2. **One slot-neutral declaration move in `include/game/sLib/s_StateStateMgr.hpp`**
   for the last violation: `virtual void refreshState()` must be declared
   **before** `changeToSubState`. `refreshState` overrides a pure virtual of
   `sStateMgrIf_c`, so it takes its slot from the base and moving it cannot
   change any slot; the four NEW virtuals keep their order
   (`changeToSubState=10, returnState=11, isSubState=12, getMainStateID=13`,
   confirmed against `finalizeState`'s own `lwz r12,0x38` / `0x34`). 1 -> 0.

**Blast radius of the shared-header move, MEASURED, not argued.** Every landed
source whose object contains a `sStateStateMgr_c<...>` instantiation (21 files,
found by `grep -rl "sStateStateMgr_c<" bin/compiled/`) was recompiled with and
without the change and the objects byte-compared (`wip/linemng_landing/hdr_ab/`):

* **20 of 21 byte-identical**, including `d_actor_state.cpp`, `d_a_en_noko.cpp`
  and `d_a_en_snake_block.cpp`.
* `d_enemy.o` differs in **`.rela.data` entry ORDER only** — 223 relocations,
  identical multiset, all section contents byte-identical, `dtk elf disasm` and
  `dtk elf info` produce identical output. Link-neutral.

Proposed diff (the only change to the shared header):

```
     virtual void finalizeState() { ... }
 
-    virtual void changeToSubState(const sStateIDIf_c &newState) {
-        currentMgr = &subMgr;
-        currentMgr->changeState(newState);
-    }
+    virtual void refreshState() { currentMgr->refreshState(); }
+
+    virtual void changeToSubState(const sStateIDIf_c &newState) {
+        currentMgr = &subMgr;
+        currentMgr->changeState(newState);
+    }
 
     virtual void returnState() { ... }
 
     virtual const sStateIDIf_c *getOldStateID() const { ... }
 
-    virtual void refreshState() { currentMgr->refreshState(); }
-
     virtual bool isSubState() const { return currentMgr == &subMgr; }
```

Per AGENT_CONTEXT rule 2 this must still be verified ALONE with
`progress.py --verify-bin` before it lands with anything else.

### `.data` / `.bss` static ORDER

Checked as instructed. The draft's `.data` opens with the four `STATE_DEFINE`
tables at `0x0/0x84/0x104/0x180` (`0x84+0x80+0x7c+0x74 = 0x1F8`), matching retail's
`@55792/@55889/@55993/@56054` at `0x80316CA0..0x80316E98` exactly, and the six
class vtables follow at `0x1F8` in retail's order (`77, 91, 49, 79, 26, 23`).
`.data` total `0xa98` = retail's `0x80316CA0..0x80317738`. No reordering needed.

---

## 2. `smc_UNIT_SIZE_X` — **NOT CLOSED. This is the blocker.**

### What retail has

```
smc_UNIT_SIZE_X__10dLineMng_c = .sdata2:0x8042CB18; // type:object size:0x4 align:4 data:float
python tools/auto_decomp/pool.py 0x8042CB18  ->  41800000 -> f32 16.0
```
Global (no `scope:` tag), 4 bytes, value **16.0f**, and it is the FIRST object of
this unit's `.sdata2`, with the literal pool running from `0x8042CB1C`.

### It MUST be defined in this TU — proven from the section's padding fingerprint

The draft's `.sdata2` (no definition) is `0xBC`, align 8, laid out
`+0x0 f4, [4 bytes of padding], +0x8 d8, +0x10 f4, [pad], +0x18 d8 ...`.
Retail's block from `0x8042CB18` is
`+0x0 smc(f4), +0x4 @54951(f4), +0x8 @54956(d8), +0x10 @55011(f4), [pad], +0x18 d8 ...`
— identical from `+0x8` onward, with **`smc` filling the hole at `+0x0`**.

An 8-aligned contribution can never place its first float at `…CB1C` and its
first double at `…CB20`. So retail's `d_line_mng.o` contains a 4-byte object
ahead of the pool, and the only symbol there is `smc_UNIT_SIZE_X`. Corroborated
independently by two of the gate tools:

* `check_sections.py` -> `.sdata2  claim 0xc0  object 0xbc  UNDER 0x4 -- something is missing`
* the unresolved-symbol sweep -> `smc_UNIT_SIZE_X__10dLineMng_c ... retail places it INSIDE our own claim`

The "define it in a neighbouring TU and pin it" idea is therefore **refuted**,
not merely untried.

### A/B across all four using functions (8 statements, 16 references)

`init`, `start_line_move`, `check_term`, `fn_800C31C0` — every reference is
`(f32)(int)(v / smc_UNIT_SIZE_X) * smc_UNIT_SIZE_X`.

| variant (`experiments/`) | `.sdata2` layout | tally | init 70w | start_line_move 94w | check_term 73w | fn_800C31C0 549w |
|---|---|---|---|---|---|---|
| **no definition** (current) | pool at +0, **hole at +4** — WRONG | **181/182** | exact | exact | exact | exact |
| `s2` definition at TOP | smc at +0 but pool grows 2 entries — WRONG | 177/182 | 71w (+1) | 94w diff | 78w (+5) | 549w diff |
| `s1` definition at END | pool at +0, smc at +0xBC — WRONG | 181/182 | exact | exact | exact | exact |
| `s3` `const volatile` member | smc at +0 | 177/182 | 73w (+3) | 97w (+3) | 72w (-1) | 552w (+3) |
| `s5` TOP + `#pragma opt_strength_reduction off` | smc at +0 | 177/182 | 71w | 94w diff | 78w (+5) | 549w diff |
| `s6` non-`const` member | **lands in `.sdata`** — WRONG SECTION | 178/182 | 71w (+1) | exact | 69w (-4) | 549w diff |
| `s7` TOP + volatile-laundered read | **smc at +0, pool exact — RETAIL'S LAYOUT** | 177/182 | 70w, 15 reg diffs | 94w, ~6 reg diffs | 69w (-4) | 549w diff |
| **`s8` = s7 + `unit` hoisted above check_term's loop** | **RETAIL'S LAYOUT** | **178/182** | 70w, 15 reg diffs | 94w, reg diffs | **exact** | 549w diff |

Root cause of every regression, read off the disassembly rather than guessed:
once the initialiser is visible, `-O4` strength-reduces `v / 16.0f` to
`v * 0.0625f` and needs two new pool entries. Retail emits
`lfs f3, smc_UNIT_SIZE_X@sda21(r0)` and a real `fdivs`, i.e. it treats the symbol
as opaque.

### Where s8 got to, and what is left

`s7`/`s8` define it at the top (retail's exact `.sdata2` layout) and launder each
read through `#define UNIT_X (*(const volatile f32 *)&dLineMng_c::smc_UNIT_SIZE_X)`
into one plain local per function. That restores the `fdivs` and the single load.

* `check_term` closes **byte-exact** once `f32 unit = UNIT_X;` is hoisted ABOVE
  the loop — retail keeps it in the callee-saved `f30` across the `bl
  getLineUnitNo`, with a `0x60` frame saving `f30`+`f31`. That is direct evidence
  the original really does hold the value in a local there.
* `init`, `start_line_move`, `fn_800C31C0` are **length-exact** with a pure FP
  register rotation (target `f5/f4/f3/f6`, draft `f4/f3/f6/f5`): the local's
  def-point ranks it above the bare leaves. This is AGENT_CONTEXT's
  "A def-point is not free" case, levers 11-13 territory, and it is the obvious
  next round of work.

**Do not land the `volatile` form as-is** — it is a hack and it is 3 functions
short. It is recorded because it is the first form that reproduces retail's
`.sdata2` byte layout at all, and because it localises what remains to a
register-allocation problem in three functions rather than an unsolved
structural one.

---

## 3. BOUNDS — validated, unchanged from `LANDING.md`

**Bases read from `slices/wiimj2d.json` -> `meta.sections`, stated per section so
the arithmetic is checkable:**

| section | base subtracted | claim (offsets) | VA range | size |
|---|---|---|---|---|
| `.text` | `0x80006780` | `0xba640-0xc2220` | `0x800C0DC0-0x800C89A0` | `0x7be0` |
| `.ctors` | `0x802edce0` | `0x118-0x11c` | `0x802EDDF8-0x802EDDFC` | `0x4` |
| `.rodata` | `0x802edfe0` | `0x3308-0x3338` | `0x802F12E8-0x802F1318` | `0x30` |
| `.data` | `0x802fe6a0` | `0x18600-0x19098` | `0x80316CA0-0x80317738` | `0xa98` |
| `.sbss` | `0x80429ea0` | `0x3d0-0x3d1` | `0x8042A270-0x8042A271` | `0x1` |
| `.bss` | `0x80351980` | `0x7780-0x7de0` | `0x80359100-0x80359760` | `0x660` |
| `.sdata2` | `0x8042b360` | `0x17b8-0x1878` | `0x8042CB18-0x8042CBD8` | `0xc0` |

Ready to paste:

```json
{
  ".text":   "0xba640-0xc2220",
  ".ctors":  "0x118-0x11c",
  ".rodata": "0x3308-0x3338",
  ".data":   "0x18600-0x19098",
  ".sbss":   "0x3d0-0x3d1",
  ".bss":    "0x7780-0x7de0",
  ".sdata2": "0x17b8-0x1878"
}
```

Note `.sdata2` starts at `smc_UNIT_SIZE_X`. That is correct **only once the
symbol is defined in the object** — which is exactly blocker 2.

### Symbol-boundary check (`check_bounds.py`, run with VA ranges)

`check_bounds.py` compares slice offsets against dtk symbol addresses directly,
which works for RELs but silently reports "no symbols in range" for the DOL. Run
with VA ranges it validates properly:

```
.text    first 0x800c0dc0 __ct__10dLineMng_cFv (0x108)  last 0x800c8910 isSameName__25sFStateID_c<10dLineMng_c>CFPCc (0x88)
.rodata  first 0x802f12e8 @LOCAL@is_unit_circle2x2...@d_unit  last 0x802f12f8 @LOCAL@is_unit_circle4x4...@d_unit (0x20) -> ends exactly 0x802f1318
.data    first 0x80316ca0 @55792 (0x84)  last 0x80317704 __vt__25sFStateID_c<10dLineMng_c> (0x34) -> ends exactly 0x80317738
.sbss    first/last 0x8042a270 lbl_8042A270 (0x1) -> ends exactly 0x8042a271 (0x7 alignment gap to d_main's g_InitialTime, benign)
.bss     first 0x80359100 @49614  last 0x80359740 lbl_80359740 (0x20) -> ends exactly 0x80359760
.sdata2  first 0x8042cb18 smc_UNIT_SIZE_X__10dLineMng_c (0x4)  last 0x8042cbd0 @56552 (0x4)
BOUNDS PLAUSIBLE
```
Every range begins on a real symbol boundary; four end exactly where the last
symbol ends. `.text` ends `0x8` past its last instruction (padding to
`d_lytbase`), `.sdata2` `0x4` past (padding to the next unit's 8-aligned start).

### Overlap-and-adjacency, computed in offset space against every existing slice

Zero overlaps. **Six of the seven sections abut `dol/bases/d_lytbase.cpp`'s start
with gap `0x0`** (`.text .ctors .rodata .data .bss .sdata2`); `.sbss` abuts
`dol/bases/d_main.cpp` with a `0x7` alignment gap. A six-way exact adjacency is
not a coincidence.

---

## 4. Gate output

| check | result |
|---|---|
| `check_bounds.py` (VA form) | **BOUNDS PLAUSIBLE** |
| overlap/adjacency (offsets) | **0 overlaps**, 6/7 sections exactly adjacent |
| `verify_anon.py` | **180/181 byte-identical modulo symbol names**; no `FUNCTION ORDER IS WRONG`; only `line_cross_chk2` differs (27 instructions). `__sinit` is not in this dump — it lives in `bin/dtkspl/obj/auto_sinit__d_line_mng_cp_text.o` |
| `check_sections.py` | `.ctors .rodata .data .sbss .bss` **ok**; `.text` over `0x288` (unreferenced weak symbols, expected); **`.sdata2` UNDER 0x4 -- FAIL** |
| vtable slot check | **VTABLE CLEAN** — all 7 vtables (`77sFStateStateMgr_c`, `91sStateStateMgr_c`, `49sFStateMgr_c`, `79sStateMgr_c`, `26sFStateFct_c`, `23sFState_c`, `25sFStateID_c`) match retail slot-for-slot BY SYMBOL NAME |
| `poolcheck.py` | **235 constants compared by value, 0 mismatched**, 181/182 functions covered. 2 unresolved, both `.bss` loads of `s_dDir` (no static image — the tool's own explained class) |

`check_vtable.py` itself cannot be used here: it expects numeric slot values, and
the DOL split object carries resolved symbol NAMES. The direct name-for-name
comparison above is the correct equivalent and is stricter.

---

## 5. Unresolved symbols and the weak-placement trap

None of the five gate tools sees this, so it was done separately by parsing the
object's symbol table against `bin/dtk/wiimj2d_symbols.txt`, `syms.txt` and all
146 already-landed DOL objects.

**Undefined symbols referenced: 28. One problem:**
`smc_UNIT_SIZE_X__10dLineMng_c` — retail places it inside our own claim, so it
must be defined by us (blocker 2). The other 27 all resolve outside the claim.

**Symbols defined: 163 global, 65 weak.** Cross-checked against every landed DOL
object:

* 19 weak duplicates (`__dt__13sStateMgrIf_cFv`, `__vt__10sStateIf_c`,
  `isNormalID__13sStateIDChk_c`, the `sStateMethod_c` accessors, `__dt__7mVec2_cFv`,
  `__dt__Q23EGG8Vector2fFv`, …) are **already emitted by 6-43 landed, verified
  objects** (`d_actor_state.o`, `d_a_player_base.o`, `d_CourseSelectGuide.o` …).
  The tree is green with them. Not a risk.
* **`__ct__7mVec2_cFv` is emitted by ZERO landed DOL objects.** `d_line_mng`
  would be the first, and retail resolves it to `0x8007F800` in an un-landed TU.
  This is the exact defect AGENT_CONTEXT records for the previous landing attempt
  (`.text` pushed out by `0x10`). The precedent for the fix is already in the
  tree: `__ct__7mVec3_cFv=0x80015E30` is pinned in `syms.txt`.

  **Proposed, for the integrator:**
  * `syms.txt`: `__ct__7mVec2_cFv=0x8007F800`
  * `slices/wiimj2d.json` `deadstrip`: `"__ct__7mVec2_cFv"`
* `fn_800C1EE0__FP10dLineMng_c…` and `setArrElem_800C15B0__FP7mVec2_c…` are
  emitted `global` and have no retail name (retail's are the stripped
  `fn_800C1EE0` / `fn_800C15B0` inside our claim, both correctly ordered).
  Harmless, but consider making them `static` like the other three file-scope
  helpers so they stay out of the link.

---

## 6. What remains, in order

1. **`smc_UNIT_SIZE_X` (BLOCKER).** Find the source form that defines it first in
   `.sdata2` without the reciprocal fold, or close the three register rotations
   left in `experiments/s8`. Everything else is ready.
2. `__ct__7mVec2_cFv` `syms.txt` pin + `deadstrip` entry (mechanical, precedented).
3. Verify the one-line `s_StateStateMgr.hpp` move ALONE with `--verify-bin`.
4. `line_cross_chk2` (100w) stays open; it does not block landing.
