# Assembly report — `d_a_player_manager.cpp` (`daPyMng_c`)

Merged all eight batch drafts into `wip/player_manager/assembled.cpp`, 65
authored functions plus the two foreign weak-inline bodies, in **target
address order**. I did not author or fix anything — every near-miss reported
below is exactly what the owning batch delivered and characterized; my job
was placement, deduplication, and reverting two isolation-only workarounds
per the brief's rule 4. Full per-batch source recovery, the merge itself, and
three independent verification passes are all described below.

## 0. Where the batch source actually came from

Six of the eight batches' full deliverables were **not** present in
`wip/player_manager/` when I started — only prose/snippets survived in the
`BATCH*.md` files, and the batches' own scratch directories (`.../scratchpad/bN/`)
live in per-session temp storage, not the repo. This session's own scratchpad
happened to still hold every batch's `draft.cpp` (all 8 sub-agents shared one
session), and `wip/player_manager/scratch/{b2,b3,b4}/` held three more copies
directly in-repo. I cross-checked every recovered draft's content against its
`BATCHN.md` narrative (return types, instruction counts, named findings) before
using it — nothing here is reconstructed from memory or guessed. **This is
worth flagging to the lead as a process gap**: if this session's scratchpad
had been cleared, B1's `initStage`, and all of B5/B6/B8, would have had no
recoverable source at all.

## 1. Address-order placement (rule 1)

Built the canonical order from `bin/dtk/wiimj2d_symbols.txt` directly (67
functions, `0x8005E9A0`–`0x80061304`, `__sinit` excluded) — reproduced
independently here and cross-checked against `BOUNDS.md`'s own derivation;
they agree exactly. Every function was placed individually by address, not
concatenated by batch. Data objects were interleaved the same way: pool
literals were **not** hand-positioned at all — I let each function reference
its own literals in place and relied on rule 1's own mechanism (MWCC pools
literals in first-reference order once the whole TU is compiled together).
Verification (§4.4) confirms this worked: every `.sdata2` object in the
compiled draft lands at the exact relative offset the target has, **in the
target's exact order**, from nothing more than placing the functions
correctly.

## 2. Overload pairing (rule 2)

`addNum`/`decNum` each have two overloads at `0x8005FDB0`–`0x8005FED0`:
`addNum(int)` → `decNum(int)` → `addNum()` → `decNum()`, in that address
order. B5's own source already defined them in exactly this order with
explicit per-function parameter counts in its status table, so no reordering
or re-pairing was needed — I verified this against the symbol map myself
before trusting it. `bin/dtk/wiimj2d_symbols.txt` confirms: `addNum__9daPyMng_cFi`
(1 param) at `0x8005FDB0`, `decNum__9daPyMng_cFi` (1 param) at `0x8005FE30`,
`addNum__9daPyMng_cFv` (0 params) at `0x8005FEB0`, `decNum__9daPyMng_cFv`
(0 params) at `0x8005FED0`. Compiled-object symbol dump (`dtk elf info`) shows
**no duplicate `.text` symbol names anywhere** in the assembled object — the
specific bug this rule exists to prevent did not recur.

## 3. The two foreign functions (rule 3)

`getCourseIn__10dScStage_cFv` and `getFileP__5dCd_cFi` are **not written** in
`assembled.cpp`. Confirmed in the compiled, disassembled object:
- `getCourseIn` is flushed automatically (called with `bl` from `initStage`)
  and is **byte-exact** against the target (trivial 8-byte body).
- `getFileP` is **never flushed at all** — it is still fully inlined at all
  three call sites (`getPlayerSetPos`, `getPlayerCreateAction`,
  `createCourseInit`) even in the complete, whole-TU compile. The target only
  calls it out-of-line from `createCourseInit`, and `createCourseInit` is
  still 7 instructions short of the target (345 vs. 352, unchanged by
  assembly — see §5). This is exactly the open question B2 and the brief both
  flagged: the `bl` should appear once `createCourseInit` reaches its true
  size, and it hasn't yet. Not something this assembly step can fix.

## 4. Rule 4 — reverted the two pointer-arithmetic workarounds

B3's `update()` and B7's `setHipAttackQuake()` both computed
`u8 *base = reinterpret_cast<u8*>(m_playerID)` and indexed through it to
reach `mRest`/`m_quakeTimer`/`m_quakeEffectFlag` (and, in `setHipAttackQuake`,
an unnamed 3-int `.bss` table). Both batches flagged this as an isolation
workaround, not original source shape. I reverted both to plain array syntax
(`mRest[j]`, `m_quakeTimer[i]`, `m_quakeEffectFlag[i]`) and, for the unnamed
table, declared it as an ordinary **function-local static** (`static int
seTable[3];`) instead of computing it as `m_playerID + 0xea0`.

**This is now measured, not just asserted per the brief's own probe.** The
compiled object's `.bss` layout (`dtk elf info` on the assembled draft, all
offsets relative to the section start = `m_playerID`):

| Object | Compiled offset | Target offset (addr − `0x80355110`) | Match |
|---|---|---|---|
| `m_playerID`…`m_quakeEffectFlag` (12 arrays) | `0x0`–`0xC0` | `0x0`–`0xC0` | exact |
| `mDemoManager` | `0xD0` | `0xD0` | exact |
| `mMultiManager` | `0x174` | `0x174` | exact |
| `mAttention` | `0x1E0` | `0x1E0` | exact |
| `mEffectMng` | `0x244` | `0x244` | exact |
| **`seTable[3]` (local static, unnamed in target)** | **`0xEA0`** | **`0xEA0`** (`0x80355FB0`) | **exact** |

The local static landed at the *exact* target address purely from being an
ordinary `static int[3]` declared after the class's own out-of-line storage
definitions — no offset arithmetic needed. This is strong, direct evidence
for the brief's rule-0 claim, not just a repeat of it. `.sbss` and `.sdata2`
show the identical pattern (§4.4).

Both functions still show a residual diff after the revert (`update()`: ~14
canonicalised lines; `setHipAttackQuake`: 6, down from 7 non-canonicalised
mismatches in B7's isolated draft) — see §5 for what's left and why it isn't
mine to fix.

## 5. Data ownership (rule 5) — no duplicates found

Checked every named object BATCHES.md assigns an owner to appears **exactly
once** in `assembled.cpp`: `scBaseID[2]`, `scRestMax`/`scCoinMax`/`scScoreMax`,
`scModelTypeDt[4]`, `scOfsX[4]`/`scOfsY[4]`, `lbl_80429FD0`, `seTable[3]`, and
all twelve `.bss` arrays plus the four managers — one grep hit each,
confirmed by direct search of the assembled file. No two batches had defined
the same object under different names on this unit.

**Deduplicated two helper accessors that two batches independently invented**
for the same foreign fields, exactly as B2 predicted would be needed:
- `dScStage_c` `+0x120e`/`+0x1211` — B1's `getPlayerCreateAction` and B2's
  `createCourseInit` each wrote their own raw-offset accessor for the same
  two bytes. Unified onto one definition (`stageField_0x120e`/`_0x1211`),
  used by both call sites.
- `dInfo_c` `+0xaf4` — read by B1's `initStage` course-in gate *and* B2's
  `createCourseInit` mid-scroll override. Same unification
  (`infoField_0xaf4`).

## 6. `scRestMax`/`scCoinMax`/`scScoreMax` (rule 6)

Kept exactly as B6 proved: plain, non-`const`, anonymous-namespace `int`s.
Confirmed in the compiled object at `.sdata` offsets `0x0`/`0x4`/`0x8` (target
`0x80427C00`/`04`/`08` relative to the same base) — present and correctly
loaded via `@sda21`, not folded away.

## 7. Objects nobody's batch owned (rule 7)

- **`seTable[3]`** (the three ints at `.bss:0x80355FB0`): added as a
  function-local static inside `setHipAttackQuake`, per §4 above. Landed at
  the exact target offset.
- **`lbl_80429FD0`**: B7 already named and defined this (`static s8`, file
  scope). Placed immediately after the pasted `.sbss` block from
  `statics_defs.inc`. Confirmed in the compiled object at `.sbss` offset
  `0x50` — target's `0x80429FD0` is exactly `0x50` past `0x80429F80`. Exact.

## 8. Header state at assembly time — the lead had already applied several fixes

`include/game/bases/d_a_player_manager.hpp` already carries `bool`/`int`
corrections for `changeItemKinopioPlrNo`, `addRest`, `create`, `setYoshi`,
`addNum()`/`decNum()`, **and `decRest` (now `int`, not `bool`)** — all of
these match what the batches proved. The **only** place this actually
required a change from what a batch delivered: B6's `decRest` body was
written against the header as `bool daPyMng_c::decRest(int)` (matching the
header *at the time B6 ran*); the header has since been fixed to `int`, so I
changed the definition to `int daPyMng_c::decRest(int plrNo)` to match. This
single change turned `decRest` from B6's reported "proven header defect, not
fixable" into a **confirmed byte-exact match** in the assembled compile — the
first concrete example of the count going up for a reason beyond mere
assembly (§9).

## 9. Verification, three ways

Wrote `wip/player_manager/scratch/assembly/verify_pm.py` (adapted from
`tools/unit_verify.py`, pointed at `wip/player_manager/target_text.txt`
directly since this unit has no `tools/auto_decomp/work/<unit>/` directory
prepared) and used it for all three checks below, plus `dtk elf info` for the
data-object cross-checks in §4/§7.

**A blocking compile problem, found and reported, not fixed:** `update()`
needs `PauseManager_c` (does not exist anywhere in the tree — confirmed by
grep, matching B3's finding exactly), `dScStage_c::getGameDisplay()`
(undeclared), `dGameDisplay_c::setPlayNum/setCoinNum/setScore/setCollect`
(undeclared — the real header only has `c_PLAYNUM_DIGIT`), and
`dStageTimer_c::mStopped` (the real header ends 4 bytes short of where this
field needs to be). B6's `startMissBGM` also needs `SndSceneMgr::startMiss()`,
which is likewise undeclared in the real header (B6 already flagged this in
its own isolated shadow copy). **None of these five were fixed** — per the
brief, they belong to other classes' shared headers, which this unit may not
edit. For verification only, I shadow-copied five headers into
`wip/player_manager/scratch/assembly/verify_override/` (never into
`include/`) with exactly the missing declarations added, comments marking
them shadow-only, and compiled `assembled.cpp` against real headers everywhere
else plus that override directory prepended to the include search path. This
mirrors exactly what B3/B6/B7 already did individually; I did not invent
the technique or the specific declarations (B3's copies for four of the five;
B6's report for the fifth). **The real repo is not fully compilable today
without someone creating/extending these five headers** — reporting this
plainly since it blocks a real link, not just my own check.

One more compile-time fix needed before anything would build at all: `decRest`
per §8.

### 9.1 Byte equality per function

Ran `verify_pm.py` against `assembled.cpp` compiled standalone with the
override above. Also re-ran against a **byte-identical copy renamed to
`d_a_player_manager.cpp`**, because the first run showed a diff on `addScore`
that turned out to be purely the anonymous-namespace mangling embedding my
scratch file's own name (`@unnamed@assembled_cpp@` vs. the target's
`@unnamed@d_a_player_manager_cpp@`) — exactly the artifact B6 already
documented for the same reason. Under the correctly-named copy, `addScore`
is a clean byte-exact match, confirming it was never a real difference. All
figures below are from the correctly-named run.

**43 of 65 functions matched by direct address lookup, plus one more (`fn_80060DB0`)
that only matches when compared by name** — `fn_80060DB0` is a file-scope
static, so CFront mangles it (`fn_80060DB0__Fv`) in my compile while the
*target's* copy carries no linker name at all (dtk's placeholder), which is
exactly the situation B7 already solved by bypassing the automated by-address
lookup and comparing instruction lists directly by name. Doing the same here:
**byte-identical, 78/78 instructions, zero differences** — confirmed, not
re-guessed.

**44 of 65 confirmed byte-exact**, up from the 41 the eight batches
self-reported as MATCHING in isolation (`4+1+5+9+10+7+4+2`, counting B7's
`fn_80060DB0` in its own tally). **The task brief states a baseline of "31 of
58" — that does not match this unit's own numbers anywhere I can find them.**
The batch reports' own per-batch tallies sum to 41/65, `BOUNDS.md` and my own
independent symbol-map extraction both confirm 65 authored + 2 foreign = 67
functions, not 58. I'm flagging this as a contradiction with the task
instructions rather than silently substituting my own count for theirs — it
reads like boilerplate carried over from a different unit's prompt.

**Zero regressions.** Every function a batch reported MATCHING is still an
exact match here (checked by direct comparison of the two lists, not
sampling) — the only two changes from the batches' own tallies are strictly
positive: `decRest` newly matches (§8), and `fn_80060DB0` needed the by-name
bypass to see what was already true.

**One thing worth double-checking became newly visible in the whole-TU compile
and is worth calling out even though it's a naming artifact, not a functional
regression:** several functions that reference `m_playerID` or a `.bss` array
by name — `initGame`, `initStage`, `getNumInGame`, `incCoin`,
`checkCorrectCreateInfo`, `setHipAttackQuake` — show their `lis`/`addi` base
pair against `SYM0`/`...bss.0` in canonicalised form, versus the target's
named `m_playerID__9daPyMng_c@ha`/`@l`. Traced to the raw disassembly
directly (not inferred): dtk names a `.bss` relocation from its **own
object's symbol table**, and a single unlinked `.o` has no external
references to `m_playerID` to justify keeping the name — it degrades to the
per-fragment placeholder `...bss.0`. B1 predicted precisely this and said it
would resolve once linked against the rest of the project, which is
consistent with what's observed: assembling the TU did **not** make it
disappear, because *assembly is not linking*. This is not something further
work on this `.cpp` can fix — it needs an actual project link (out of scope,
`ninja` is forbidden here) to verify. Listed as its own category below rather
than folded into "still not matching" so it isn't mistaken for a code defect.

**Near-misses, unchanged from what the owning batch already characterized —
not re-diagnosed, not touched:**

| Function | What's left (batch's own words, unmodified) |
|---|---|
| `setDefaultParam` | scheduling-driven register/frame difference (41 target vs. 35 draft instructions) |
| `getPlayerSetPos` | missing `frsp` after `fneg`, semantically inert, 1-line byte diff |
| `createCourseInit` | `action∈{0,1}` range-fold (2 instr) + the `getFileP` inlining coupling above + one bool-materialisation idiom |
| `fn_8005f570` | 6 lines, pure register choice |
| `decideCtrlPlrNo` | 26 vs. 25 instr, CSE/register-allocation only |
| `getYoshi` | 2 of 39 lines, `r4` vs. `r12` register choice (the untyped-vtable-cast tradeoff B4 documented) |
| `getCoinAll` | 20/20 instr, pure register permutation |
| `addRest` | 74/74 instr, uniform +1 register shift in the clamp section |
| `startMissBGM` | 2/24 lines, `r4` vs. `r12` in a vtable-slot chase |
| `checkLastAlivePlayer` | 34/34 instr, one boolean-materialisation idiom differs |
| `deleteCullingYoshi` | 86/86 instr, all register-number swaps |
| `initYoshiPriority` | 45/46 instr, one `cmpw` operand-order artifact |
| `setYoshiPriority` | 38/38 instr (count now matches, was short by 2 in isolation), register swaps + the same `cmpw` artifact |
| `isCreateBalloon` | 18/18 instr, block-order/branch-polarity swap, semantically identical |
| `checkCorrectCreateInfo` | 105 target / 103 draft — the `m_playerID` naming artifact above plus real reordering in the clamp loop, both already present in B8's isolated report |
| `incCoin` | 130 target / 126 draft — base-pointer folding gap and a branch-structure difference B6 already flagged as unresolved |
| `update()` | ~14 canonicalised lines — matches B3's own "register-allocation/materialization only" characterization; **cannot be independently re-verified against real headers today**, see the blocking-header note above |
| `setHipAttackQuake` | 6 of 104 lines: the `m_playerID` naming artifact (2), the `.sbss` `lbl_80429FD0` pool-numbering (2, same class as `fn_8005f4d0` below), and **one real, reportable header gap**: `bl startSystemSe__11SndAudioMgrFUlUl` (target) vs. `bl startSystemSe__11SndAudioMgrFUiUl` (draft) — the real header only declares the `(u32, u32)` overload; the `(u32, u64)` one the target actually calls is entirely undeclared. B6 tried adding it and reverted because it makes 7 existing call sites elsewhere ambiguous. Not fixed here either — reporting per the brief. |
| `fn_8005f4d0` | 39/39 instr, 1 line: `SYM0`/`scBaseID` naming — the target's copy of this pool has no linker symbol at all (stripped), ours (unlinked, unstripped) keeps the real local name. Same root cause class as the `m_playerID` naming artifact, not a code defect. |
| `getFileP` | never emitted — coupled to `createCourseInit`'s size, see §3 |

None of these were touched. Reporting them, per the brief, is the deliverable
here — not fixing them.

### 9.2 Per-function size against the symbol map

Cross-checked every one of the 65 authored functions' compiled `.text` size
(read from `dtk elf info` on the assembled object) against
`bin/dtk/wiimj2d_symbols.txt`. **7 mismatches, and they are exactly the same
7 functions already known to be near-misses or the still-inlined `getFileP`**
— `setDefaultParam`, `getPlayerSetPos`, `createCourseInit`, `decideCtrlPlrNo`,
`incCoin`, `checkCorrectCreateInfo`, `getFileP`. All other 60 functions are
size-exact. This check is independent of the canonicalised-text comparator in
§9.1 (it reads raw section layout, not instruction text) and found nothing
the other view didn't already know about — a useful negative result, not a
null one.

### 9.3 Emitted symbol order against target address order

`verify_pm.py` walks the compiled disassembly in file order and compares it
against the target's address order. **Matches exactly**, all 65 placed
functions plus `getCourseIn`.

**Deliberately broke it to prove the check works, per the task's own
instruction**, on a throwaway copy (never the deliverable): swapped
`setCourseInStarBGM__9daPyMng_cFv` and `startStarBGM__9daPyMng_cFv` (both
B6's, adjacent in the file) and re-ran. The check fired immediately:

```
EMISSION ORDER: !! MISMATCH vs target address order
   at position 46: draft has startStarBGM__9daPyMng_cFv, expected setCourseInStarBGM__9daPyMng_cFv
   at position 47: draft has setCourseInStarBGM__9daPyMng_cFv, expected startStarBGM__9daPyMng_cFv
```

Then reverted the swap and re-confirmed the clean pass. `assembled.cpp` was
never actually modified for this test — it ran against a copy in
`wip/player_manager/scratch/assembly/`.

### 9.4 Data-object bytes (the blind spot §9.1's comparator has)

Read every named object's actual offset out of the compiled object
(`dtk elf info`) rather than trusting the instruction-text comparator, per
the brief's explicit warning that it cannot see a wrong constant.

- **`.bss`**: all 12 arrays, the 4 managers, and the local-static `seTable`
  land at their exact target-relative offsets (table in §4). Section total is
  `0xEAC` vs. the target's `0xEB0` — the missing 4 bytes are the trailing
  alignment pad to the *next* TU's `.bss` start, which only exists once
  actually linked; not a defect.
- **`.sbss`**: all 18 scalars, ending with `lbl_80429FD0` at offset `0x50`
  (target `0x80429FD0 − 0x80429F80 = 0x50`) — exact. Section total `0x51` vs.
  target's `0x58`; same trailing-pad-only-when-linked situation.
- **`.sdata`**: `scRestMax`/`scCoinMax`/`scScoreMax` at `0x0`/`0x4`/`0x8` —
  exact.
- **`.sdata2`**: all eight objects this unit references — the `getPlayerSetPos`
  `0.0f` and bias-double, `createCourseInit`'s `504.0f`/`0.1f`/`12.0f`/`24.0f`
  and staff-credits bias-double, `fn_8005f4d0`'s `scBaseID` pair, and
  `fn_80060DB0`'s shared `0.5f` and `3800.0f` — land **in the target's exact
  order and at the target's exact relative offsets**, `0x0` through `0x38`
  (section total `0x38`, exact, no padding needed — this is the one section
  where the target's own bound ends exactly on our last object). This is the
  strongest direct confirmation that rule 1 (source order controls pool
  order) worked: nothing here was hand-placed, it fell out of simply putting
  the functions in address order.
- **`.data`**: `"Wm_mr_vshipattack"` at offset `0x0` (`0x12` bytes) and
  `"Wm_mr_vshipattack_ind"` at offset `0x14` (2-byte alignment gap between
  them) — exact, matching B7's own byte-level check.
- **`.rodata`** — see the contradiction below; not a clean confirmation.

## 10. A `.rodata` contradiction the brief did not have — reporting, not reconciling

`SHARED-BRIEF.md`'s section table states our `.rodata` claim is **"one object
only"** (`scModelTypeDt`, `0x802EF608`–`18`, `0x10` bytes). But B2's own data
table already flagged that `createCourseInit`'s staff-credits-branch locals
`scOfsX[4]`/`scOfsY[4]` are **also** emitted as `.rodata` by the code that
uses them, at `0x802EF5D8`–`0x5E7` per `target_rodata.txt` — an address
range entirely *before* the claimed bound, confirmed byte-for-byte by B2
against the raw target bytes. My own compiled object reproduces this
independently: `scOfsX`/`scOfsY` land in `.rodata` ahead of `scModelTypeDt`
in the same section (offsets `0x0`, `0x10`, `0x30` respectively in the
unlinked object). Both facts are independently confirmed and they disagree
with the brief's "one object only" framing. Not reconciling this — the
`.rodata` bound stated in the brief may need revisiting by the lead
(possibly `0x802EF5D8`–`0x802EF618`, `0x40` bytes, three objects, not one),
but that determination needs the STATICS/BOUNDS documents' own derivation
method, not an assembly-time guess.

## 11. Summary

- `wip/player_manager/assembled.cpp` — 65 authored functions in address
  order, plus the class's out-of-line static storage
  (from `statics_defs.inc`, unmodified/unreordered) and `lbl_80429FD0`.
  Compiles clean against the real project headers plus the five-header
  verification-only override described in §9 (that override is **not**
  needed for the other ~63 functions — only `update()` and `startMissBGM`
  touch the missing declarations).
- 44 of 65 functions confirmed byte-exact (67.7%), zero regressions from the
  batches' own isolated tallies, one function (`decRest`) newly resolved by
  a header fix the lead had already applied.
- All three verification methods run and reported: byte equality (with the
  by-name bypass needed for `fn_80060DB0`, matching B7's own precedent),
  per-function size against the map (7 mismatches, all already-known), and
  emission order (clean pass, deliberately broken and restored to prove the
  check fires).
- `.bss`/`.sbss`/`.sdata`/`.sdata2`/`.data` all confirmed byte-for-byte at
  the correct relative offsets by reading the compiled object directly, not
  by trusting the instruction-text comparator.
- Five real header gaps block full standalone recompilation of `update()`/
  `startMissBGM` today (`PauseManager_c` entirely missing;
  `dScStage_c::getGameDisplay()`, `dGameDisplay_c`'s four display methods,
  `dStageTimer_c::mStopped`, `SndSceneMgr::startMiss()` all undeclared) —
  none fixed, all reported, shadow copies for verification only live under
  `wip/player_manager/scratch/assembly/verify_override/`.
- One real, unresolved header defect found (not by me — B6's, reconfirmed
  here): `SndAudioMgr::startSystemSe`'s `(u32,u64)` overload is entirely
  undeclared, and the target genuinely calls it from both `decRest` and
  `setHipAttackQuake`.
- One `.rodata`-bound contradiction against `SHARED-BRIEF.md`, independently
  reconfirmed against the compiled object (§10).
- Two accessor helpers deduplicated across batch boundaries as B2 predicted
  would be needed (§5).
