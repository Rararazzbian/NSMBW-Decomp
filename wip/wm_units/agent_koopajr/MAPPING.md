# WM_KOOPAJR (`daWmKoopaJr_c`) — function inventory

## ROUND 3 — UNIT PARKED, BLOCKED ON LINK (read first)

Coordinator independently verified round 2's order fix and `procNone` fix
(`verify_anon` reports 15/21 byte-identical, no order violation) and asked me
to author the `runMain` cases with confirmed pool consumers in `0x90`-`0xd8`
(cases 5-13, plus case 3 which was already fully read), re-diffing
`create()`/`execute()` after each and STOPPING once that range closes if the
`0x8be8`-`0x8c08` gap still hasn't moved — a bounded round, explicitly not
meant to be ground through past that point.

**Authored this round, each read directly off the disassembly (see the
individual `case N:` comment blocks in the source for the full citation —
line/offset numbers, header matches, landed precedents):** cases 3, 5, 7, 8,
9, 10, 12. Combined with round 2's case 4, that's 8 of `runMain`'s 16 cases
now non-stub (0, 1, 2, 4, 5, 7, 8, 9, 10, 12, 14, 15 — 12 of 16, the stub list
below is smaller than it looks since 0/1/2/14/15 predate this notebook
section).

**Re-diffed `create()`/`execute()` after this batch — ZERO movement, third
independent confirmation:** PTMF table `+0x50` vs target `+0x70` (unchanged),
CalcShadow constants `+0x6c`/`+0x68` vs target `+0x8c`/`+0x88` (unchanged).
Identical to round 2's post-order-fix numbers before any case-4-and-later
work. Order gate re-checked: still **ORDER OK** (15/19). `.ctors`: still 1
entry, matches target.

**Conclusion, per the coordinator's own bound: PARK THIS UNIT.** The
`0x90`-`0xd8` pool range is now substantially closed by authored cases (new
pool entries this round match retail exactly: 9.0f/28.0f at 0xb0/0xb4 (case
3's `checkFrame` thresholds), 34.0f/46.0f at 0xc0/0xc4 (case 8's), -1.0f at
0xbc (case 7's `setDirection`), 2.0f at 0xc8 (case 10's `mSpeedF` term)) and
the `create()`/`execute()` gap has still not moved by a single byte across
three independent measurements (after the order fix alone, after case 4
alone, and after this whole batch). Two cases were deliberately left as
stubs rather than guessed: **case 6** and **case 11** add no new pool value
beyond what cases 3/7/9/10 already cover (verified by scanning their full
target disassembly for `(r31)` offsets before deciding not to author them —
not a coverage gap, a genuine no-op for the pool-completion goal). **Case 13**
was read but NOT authored: its first instruction (`lfs f2, 0x280(r3)`) reads
a field that is not a clean `mAnimChrs[i]` index (`0x280 - 0x1e8 = 0x98`,
`0x98 / 0x38` does not divide evenly), so it would require guessing an
unverified sub-field layout inside `m3d::anmChr_c`/`fanm_c`/`banm_c` — not
attempted, consistent with the project's "don't guess" rule.

Per the coordinator's own framing: this is a **planning fact, not a defect**.
The remaining `create()`/`execute()` gap (`0x8be8`-`0x8c08` in the retail
REL, a 9-word/0x24-byte block with no consumer anywhere in this unit's own
disassembly, exhaustively re-confirmed) is not reachable from a single-file
`build.py` compile. This unit should be re-tested against the real multi-object
link once more of `d_basesNP` is landed, not reworked further from here.

---

## ROUND 2 FINDINGS (superseded in priority by ROUND 3 above, kept for the
detailed reasoning behind the order fix / procNone fix / gap investigation)


Picked up mid-edit after a session-limit kill. Rebuilt first per instructions:
`build.py` compiled clean (`draft.o`/`draft.txt` were current), and the draft
on disk already had cases 1 and 2 of `runMain` authored beyond what the
notebook below described (0, 14, 15 only) — the predecessor made progress
after the last MAPPING.md save but before being killed.

**1. FUNCTION DEFINITION ORDER WAS WRONG — fixed, verified, real.** The `.cpp`
defined functions in an unrelated order (`ctor,dtor,doDelete,draw,calcModel,
resetScaleAndProc,resetState,create,procMain,changeAnim,runMain,execute,
processCutsceneCommand,lookupAction,startAction,createModel`) against the
target's true address order (`create,execute,draw,doDelete,createModel,
calcModel,resetState,resetScaleAndProc,procNone,startAction,procMain,
processCutsceneCommand,lookupAction,runMain,changeAnim`). `check_fn_order.py`
can't see this (it only checks files using `fn_2_*` symbol names, and this
unit's functions have real names), so I reproduced `verify_anon.py`'s
ascending-pairing algorithm directly against the two target dumps + `draft.txt`
and it reported **FUNCTION ORDER IS WRONG** outright, with 8 functions "defined
too late". This is exactly the `d_a_wm_smallcloud.cpp` failure mode
`verify_anon.py`'s own docstring warns about: a unit can tally well
per-function and still fail to link because every `bl`/pool-relative
displacement past the inversion point is wrong. **Fixed** by physically
reordering every out-of-line member function definition in the `.cpp` to match
the target's address order (see the block comment now at the top of that
section in the source). Re-ran the same check after: **ORDER OK**, all 15
non-trivial functions in the range now pair up in strictly ascending order.

**2. `procNone` was in-class inline; the target has it out-of-line — fixed,
verified.** The target's `fn_2_16D7E0` is marked `global` in the dump, not
`weak` — a trivial in-class-defined member (`void procNone() {}`) compiles
with vague/weak linkage, so `global` proves the original source defined it
out-of-line as its own `daWmKoopaJr_c::procNone() {}` at its own position in
the file (between `resetScaleAndProc` and `startAction`). Before this fix the
compiler was placing the in-class inline body wherever it was first
ODR-referenced (adjacent to `execute()`, wherever `execute()` happened to be
defined) — completely wrong position. After the fix `fn_2_16D7E0` pairs
**MATCH** in the order-check, in the right slot.

**3. Re-measured create()/execute() pool displacement after the order fix —
gap COLLAPSED to one clean number, but does not close from cases 1-13.**
Before this round (per the stale table below): PTMF table `+0x50` vs target
`+0x70` (gap 8), CalcShadow constants `+0x48`/`+0x3c` vs target `+0x88`/`+0x8c`
(gaps 0x40/0x50 — inconsistent, scattered). **After only the order fix** (no
new case authored yet): PTMF `+0x50` vs `+0x70` (gap **0x20**), CalcShadow
`+0x6c`/`+0x68` vs `+0x8c`/`+0x88` (gap **0x20** both). All three collapsed to
the exact same clean 0x20-byte (8-word) gap — a real signal that fixing order
removed noise from the measurement, leaving one genuine missing chunk.

I then tried to find what fills it. Full-pool dump (`original/d_basesNP.rel`,
file offset `0x1C6600+addr`) shows a **9-word (0x24-byte) block of retail data
at `0x8be8`-`0x8c08`, between `sc_jumpParams`'s end (`0x8be4`) and `create()`'s
`250.0f` (`0x8c0c`)**, that is the root cause. I searched EXHAUSTIVELY for a
consumer of this specific range: every `(r31)` (the pool-base register)
displacement referenced anywhere across all 664 lines / all 16 cases of the
target's own `fn_2_16D940` (`runMain`) disassembly, every function in
`createModel`, and every other function in the unit, against all three target
dumps covering this unit's full address range (including `__sinit`,
`fn_2_16E490`). **None reference offsets 0x48-0x68 (pool-relative).** The
offsets runMain's cases DO use are `0x18, 0x88, 0x8c, 0x90, 0xa8, 0xac, 0xb0,
0xb4, 0xb8, 0xbc, 0xc0, 0xc4, 0xc8, 0xcc, 0xd0, 0xd8` — none in the gap.

As an empirical test (per the task's own prescribed method — author a case,
re-diff, check whether the displacement moved), I authored **case 4**
(`0x16DCDC`-`0x16DDA8`, see below) and rebuilt: **the create()/execute() pool
displacement did not move at all** (still `+0x50`/`+0x6c`/`+0x68` vs
`+0x70`/`+0x8c`/`+0x88`), exactly as the exhaustive scan predicted, because
case 4 introduces zero new distinct rodata literals (its one constant,
`sc_jumpParams[1].unk24` = 200.0f, is already present in the pool).

**Conclusion, clearly labelled as inference, not fact:** the 9-word gap
blocking create()/execute()'s last few lines is very likely **not resolvable
by authoring more of this unit's own code** — I cannot find a consumer for it
anywhere in daWmKoopaJr_c. It is possibly a sibling class's pooled constant
that the real multi-object link interleaves adjacent to ours (a single-file
`build.py` compile cannot reproduce that), consistent with `verify_anon.py`'s
own documented warning that this class of problem is invisible until the
actual link. **This is different from the OTHER unclaimed pool range
(0x90-0xd8), which DOES have confirmed consumers in runMain's still-unwritten
cases 5-13** — that range should keep closing as more cases are authored;
the 0x48-0x68 gap should not be expected to.

**4. Case 4 of `runMain` authored** (0x16DCDC-0x16DDA8, 34 target instructions).
Every constant read directly off the disassembly, none guessed: `fn_80103520`
called with the string `"koopaJr_all_root"` (read from `.data` at
`lbl_2_data_45EB0`, same string case 2 already uses), `calcSpeed()`/`posMove()`
(inherited, already-used pattern), `adjustHeightBase(startPos, targetPos,
directionType)` matching `include/game/bases/d_wm_demo_actor.hpp:53` exactly,
`mVec3_c::distTo` (`include/game/mLib/m_vec.hpp:214`) matching the
`PSVECSquareDistance`+`sqrt` instruction pair exactly, and the trailing
`this+0x60 -> +0x68` dispatch confirmed as `setCutEnd()` (same pattern as case
15 / `processCutsceneCommand`'s case `0x45`). One new fact: `sc_jumpParams[1].
unk24` is read here via `lfs` (not as a raw word) and its bit pattern
(`0x43480000`) is a clean, round-trippable 200.0f — the first evidence that
`unk24` is genuinely a float field (kept as `u32` in the struct and read via
reinterpret-cast at the use site, since index 0's `unk24` — `0x003c0000` — is
still a denormal that does not round-trip through a decimal literal, so
widening the struct's declared field type is not safe). Not diff-verified
against target byte-for-byte (runMain as a whole is still far from complete,
so a raw per-function diff count isn't meaningful yet) — logic correctness is
"read directly off the disassembly", not machine-verified.

**Cases 3, 5-13 remain unauthored stubs** — case 3 was read in full during
this investigation (own comment block above it in the source lists everything
found: `checkFrame` threshold comparisons at pool `0xb0`/`0xb4`, an
int-to-double magic-constant conversion feeding `DegreeToAngleCoefficient`/
`rotDirectionY`, a write to `this+0x10c`) but not authored — it's meaningfully
more complex than case 4 and was not finished this round for time, not
because anything about it looks unrecoverable.

**5. Verified gates:** `.ctors` — 1 entry (`fn_2_16E490` /
`g_profile_WM_KOOPAJR`), matches target, no double-init. Function order —
OK (see above). Full per-function tally after all changes: 15/19 in the
mapped table below are unchanged from before this round (this round's fixes
address the *order* gate and the *procNone* placement/type, not per-function
byte counts, since the previous per-function diffs were already correct logic
modulo symbol names before the reorder — the reorder mainly fixes whether the
unit LINKS, which the per-function tally cannot see at all). `createModel`
regressed in raw diff count (83/92 lines vs an earlier claimed "86/92") purely
because the pool shift changed its register allocation cascade — its content
gap (the unmodelled `GetResNode` mask-clear block) is unchanged and was
already a known, explicitly-flagged gap, not a new defect. Left untouched
per instructions ("re-test only after the pool fills").

---

`daWmKoopaJr_c : public dWmDemoActor_c`, `sizeof == 0x360` (confirmed twice:
`fn_2_16D290`'s `li r3, 0x360`, and independently by the destructor's member
teardown reaching exactly `+0x184` before falling into the base dtor chain).

Unit bounds: `.text` `0x16d290`-`0x16e540` (20 real functions, confirmed by
listing every `.fn` in that exact address range across both target dumps —
`gap_*`/`pad_*` entries excluded). This count matches the task's "20" headline
exactly.

Tools used: `wip/wm_units/agent_koopajr/build.py` (compile+disasm) and
`difftool.py` (raw per-function instruction diff against the target dump,
address-suffix differences only — **not** the project's real land-time
diff/link, see caveat at the bottom).

## Status legend

- **MATCH** — 0 differing lines via `difftool.py`.
- **MATCH\*** — every differing line is a symbol-NAME-only difference for a
  reference to this unit's own not-yet-separately-compiled code (this class's
  own vtable, or a `bl` to another of this unit's own member functions). The
  target dump names these `lbl_2_data_XXXXX`/`fn_2_XXXXXXX` because the unit
  hasn't been split into its own object yet; my draft names them by their real
  C++ symbol. Expected to resolve automatically once this file lands and gets
  compiled as its own object — **not independently verified against the
  project's true link-time diff**.
- **BLOCKED (rodata pool)** — logic matches, but some `lis/lfs` immediate or
  displacement differs because the shared anonymous `.rodata` pool
  (`lbl_2_rodata_8BA0`..`0x8c90`) also holds constants that belong to
  `fn_2_16D940`/`fn_2_16E3A0` (proven: several pool slots, e.g.
  `0x8bc4/0x8bd4/0x8bd8/0x8be0/0x8bec`, never appear in any authored
  function's disassembly), so those functions must be authored first for this
  unit's own constants to land at the retail offsets.
- **NOT ATTEMPTED** — not authored at all.

## The six named functions (task priority)

| target | role | size | draft symbol | status |
|---|---|---|---|---|
| `fn_2_16D580` | `doDelete` | 0x8 | `doDelete__13daWmKoopaJr_cFv` | **MATCH** (0/2 lines) |
| `fn_2_16D530` | `draw` | 0x4C | `draw__13daWmKoopaJr_cFv` | **MATCH** (0/19 lines) |
| `fn_2_16D340` | destructor | 0xAC | `__dt__13daWmKoopaJr_cFv` | **MATCH** (0/43 lines) |
| `fn_2_16D870` | `processCutsceneCommand` | 0xB0 | `processCutsceneCommand__13daWmKoopaJr_cFib` | **MATCH\*** (3/44 lines, all `bl startAction` vs `bl fn_2_16D7F0`) |
| `fn_2_16D3F0` | `create` | 0x64 | `create__13daWmKoopaJr_cFv` | 3 lines MATCH\* (own-symbol calls) + **2 lines BLOCKED (rodata pool)** — the `lbl_2_rodata_8C0C` (250.0f, `mClipSphere` radius) `lis/lfs` pair |
| `fn_2_16D460` | `execute` | 0xD0 | `execute__13daWmKoopaJr_cFv` | 1 line MATCH\* (own-symbol call) + **5 lines BLOCKED (rodata pool)**, but the pool position **measurably moved** after authoring `changeAnim()`/`runMain()` case 0 (the PTMF table displacement went from `+0x8` to `+0x50`, wanted `+0x70`; the two `CalcShadow` constant displacements went from `+0x20`/`+0x24` to `+0x48`/`+0x3c`, wanted `+0x88`/`+0x8c`) — confirms the shared-pool mechanism directly, not just by inference. Still short by roughly 8 floats' worth; more of `runMain`'s unauthored cases (1-13) are needed to close the rest. |

**4 of 6 fully closed** (doDelete, draw, destructor byte-identical modulo
nothing; processCutsceneCommand byte-identical modulo own-symbol naming that
resolves at land time). **create and execute have fully correct, verified
logic** — every instruction, register, and branch matches — **but cannot reach
byte-identity until `fn_2_16D940`/`fn_2_16E3A0` are authored**, because their
constants share one pool with those two unwritten functions. This is not a
logic defect; see the class-declaration comment in the `.cpp` and the
"rodata pool" note above for the exact proof (specific pool slots with no
referencing function found anywhere in this unit's authored code).

## Supporting non-virtual members (needed as callees, not independently required)

| target | size | draft symbol | status |
|---|---|---|---|
| `fn_2_16D290` | 0x30 | `daWmKoopaJr_c_classInit__Fv` (ACTOR_PROFILE-generated) | **MATCH\*** (1/12 lines, own ctor symbol) |
| `fn_2_16D2C0` | 0x74 | `__ct__13daWmKoopaJr_cFv` (constructor) | **MATCH\*** (2/29 lines, own vtable symbol) |
| `fn_2_16D700` | 0xB0 | `calcModel__13daWmKoopaJr_cFv` | **MATCH** (0/44 lines) |
| `fn_2_16D7B0` | 0xC | `resetState__13daWmKoopaJr_cFv` | **MATCH\*** (1/3 lines, own-symbol tail call) |
| `fn_2_16D7C0` | 0x20 | `resetScaleAndProc__13daWmKoopaJr_cFv` | 2/8 lines — **BLOCKED (rodata pool)**, the 0.01f constant |
| `fn_2_16D7E0` | 0x4 | `procNone__13daWmKoopaJr_cFv` | **MATCH** (0/1 lines; empty body, the PTMF "idle" handler) |
| `fn_2_16D7F0` | 0x34 | `startAction__13daWmKoopaJr_cFi` | **MATCH\*** (1/13 lines, own-symbol call) |
| `fn_2_16D920` | 0x18 | `lookupAction__13daWmKoopaJr_cFi` | logic/shape matches; **NOT verified** — the 4-entry lookup table `lbl_2_rodata_8C38` content is unrecovered, so the table itself (currently `{0,0,0,0}`) is a guess. 2/6 lines differ (table symbol). |
| `fn_2_16D830` | 0x3C | `procMain__13daWmKoopaJr_cFv` | **MATCH\*** (2/15 lines, own-symbol calls to `runMain`/`resetScaleAndProc`) |
| `fn_2_16D590` | 0x170 | `createModel__13daWmKoopaJr_cFv` | **NOT verified / best-effort.** All strings recovered directly from the REL's `.data` (see below) and the call shape/argument order matches the disassembly read instruction-by-instruction, but register allocation differs (86/92 lines) — most likely driven by the same missing rodata-pool constants forcing a different local/register schedule. Not re-attempted a second time; parked. |

## Not attempted (out of scope per task instructions)

| target | size | note |
|---|---|---|
| `fn_2_16D940` | 0xA60 | **STALE — see "ROUND 3" at the top.** As of round 3: 12 of 16 cases authored (0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 12, 14, 15), 4 still bare `break;` stubs (6, 11, 13 deliberately not guessed/not needed for pool coverage; case 13 specifically blocked on an unverified sub-field). `runMain__13daWmKoopaJr_cFv` diffs against `fn_2_16D940` (expected — the function is still not fully authored, and a raw line-diff count isn't meaningful until it is). Unit is PARKED per the coordinator's bound: three independent re-diffs of `execute`/`create` (after the order fix, after case 4, after this whole round-3 batch) show ZERO pool-displacement movement. |
| `fn_2_16E3A0` | 0xE4 | **AUTHORED this round as `changeAnim(int animIdx, float blendFrame, float rate, float startFrame)`.** 57/57 lines match in size; the only 4 differing lines are this unit's own `sc_animNames`/`sc_playModes` symbol names (own-symbol naming, see the MATCH\* convention above) — logic is a confirmed match. |
| `fn_2_16E490` | 0x84 | `__sinit_d_a_wm_koopajr_cpp` — the file's static initializer. Constructs a global object at `lbl_2_data_45DE8` using `dCsvData_c::c_CASTLE_ID`/`c_START_ID` (dynamic-init statics, hence needing `__sinit` rather than being compile-time constants) and three floats from the shared rodata pool, then calls `__register_global_object` with a destructor pointer `fn_2_16E520`. The identity/type of the constructed object is unresolved. Not required for any of the six named functions. |
| `fn_2_16E520` | 0x1C | The above global object's destructor wrapper (target of `__register_global_object`'s 2nd arg). Not declared. |

## Class layout — every offset is measured, not hand-counted

All of the following were read directly off a `bl __ct__.../__dt__...`,
a raw `stw`/`stfs` in the constructor/destructor, or a call argument in
`calcModel()`/`create()` whose signature pins the field — see the long
comment at the top of `d_a_wm_koopajr.cpp` for the exact instruction-level
citation of each one.

```
+0x184  int mUnk184                    -- raw store, ctor does NOT initialise it (family convention)
+0x188  dHeapAllocator_c mAllocator
+0x1a4  nw4r::g3d::ResFile mResFile    -- corrected from the scouting pass's "int(=0)", see below
+0x1a8  m3d::mdl_c mModel
+0x1e8  m3d::anmChr_c mAnimChrs[6]     -- 0x1e8 + 6*0x38 = 0x338
+0x338  int mUnk338                    -- untouched by ctor/dtor; existence proven by mProcState landing at +0x33c
+0x33c  int mProcState                 -- execute()'s PTMF index; fn_2_16D7F0/fn_2_16D7C0 also touch it
+0x340  int mUnk340                    -- set by lookupAction(); ALSO read directly by runMain()'s own
                                          outer 16-case jump table (`lwz r0,0x340(r3); cmplwi r0,0xf`) --
                                          it is the SAME field, not two separate ones.
+0x344  mVec3_c mJumpTargetPos          -- 3 floats, confirmed by runMain() case 0: three `stfs` at
                                          0x344/0x348/0x34c immediately after `daWmMap_c::GetPos()`,
                                          the result then passed straight to `_initDemoJumpBase`.
+0x350  int mJumpTimer                  -- runMain() case 0 sets it to 0x1e; case 1 decrements it every
                                          frame and branches on hitting 0. A second, structurally
                                          identical countdown reappears in case 3/6 (not yet confirmed
                                          to be the SAME field there vs. a related one -- case 6 was not
                                          authored, only skimmed).
+0x354  int mCurAnimIdx                 -- changeAnim()'s dirty-check cache (confirmed, see below)
+0x358  u8 pad358[0x4]                  -- untouched by every function authored so far
+0x35c  int mUnk35c                     -- set to -1 by resetState(); ALSO set to `fn_80103520(...)`'s
                                          return value by runMain() case 2 (an effect handle, matching
                                          the same cross-module call already seen in the landed
                                          `d_a_wm_kinoko_base.cpp` as `fn_80103420` -- here it is
                                          `fn_80103520`, a different DOL address, so a SEPARATE
                                          extern declaration, not a typo of the same one).
```
`sizeof == 0x360` — 0x35c + 4 = 0x360, closes exactly, matching `daWmKoopaJr_c_classInit__Fv`'s `li r3, 0x360` verbatim (this line was checked and now matches byte-for-byte after the `pad344` fix below).

Also confirmed, all **inherited** (`dBaseActor_c`/`dWmActor_c`) fields, each
pinned by TWO independent call sites (koopajr's own code, and the
already-landed `dWmActor_c::preExecute()`/`preDraw()` for `mPos`/`mClipSphere`):
`mMatrix`@0x7c, `mPos`@0xac, `mScale`@0xdc, `mAngle`@0x100, `mClipSphere.mCenter`@0x128, `mClipSphere.mRadius`@0x134.

### Two real bugs found and fixed during this session (not hand-waved)

1. **`mProcState` was one field short of +0x33c** (landed at +0x338) until
   `mUnk338` was inserted — caught because `execute()`'s
   `lwz r4, 0x33c(r29)` diffed against my `0x338(r29)`. Confirms this is a
   REAL field boundary, not padding folded into `mProcState`.
2. **`mUnk35c` was 0x18 bytes short** (landed at +0x344) until `pad344[0x18]`
   was inserted — caught the same way via `resetState()`'s
   `stw r0, 0x35c(r3)` vs my `0x344(r3)`, and independently corroborated by
   `daWmKoopaJr_c_classInit__Fv`'s `li r3, 0x360` (which had been silently
   producing `0x348` before the fix — the exact size of the missing gap).
3. **`resetScaleAndProc()`'s `mScale` reset used a temporary.** Writing
   `mScale = mVec3_c(0.01f, 0.01f, 0.01f);` compiled to a 5-extra-instruction
   stack-temp-construct-then-copy (13 lines vs the target's 8). Per the
   project's "stack-temp question" rule (no temp observed in target ⇒ direct
   field stores), rewriting as three explicit `mScale.x = mScale.y = mScale.z
   = 0.01f;` statements closed the size gap exactly (8/8) and left only the
   own-rodata-symbol-naming diff.

### `mResFile`@0x1a4 — corrected from the original scouting note

The scouting pass recorded this as a raw `int (= 0)` because the constructor
stores it with a plain `li r0,0; stw r0,0x1a4(r31)` — no `bl __ct__...`
visible. But `nw4r::g3d::ResFile`'s default constructor is a trivial in-class
one-liner (vague linkage), and `-inline noauto` still inlines an in-class
body, so a genuine `nw4r::g3d::ResFile mResFile;` member compiles to exactly
this pattern — indistinguishable from a raw int at the constructor alone.
Confirmed by `createModel()` (fn_2_16D590), which stores
`dResMng_c::m_instance->getRes(...)`'s return value directly into +0x1a4 via
a plain `stw` (consistent with ResFile's trivial 4-byte representation), then
calls `GetResMdl__Q34nw4r3g3d7ResFileCFPCc` with `this = &mResFile` — the
same shape and the SAME offset as the landed sibling
`daWmKinokoBase_c::mResFile`.

## Resource strings recovered for `createModel()` (from `original/d_basesNP.rel` directly)

Read directly out of the REL's `.data` (file offset `0x1D0C00 + addr`) at the
table based at `lbl_2_data_45DD8`, referenced by `fn_2_16D590` via
`r30 = lbl_2_data_45DD8`:

- `+0x78..+0x8c` (6x `const char*`, each relocated `Absolute` into `.data`):
  animation names `"wait"`, `"run"`, `"jump_st"`, `"jumpA"`, `"jump_ed"`,
  `"shock_wmap"` (verified by reading the raw bytes at each relocation
  target, e.g. `lbl_2_data_45E18` = `"wait\0"`).
- `+0x90` = `"g3d/koopaJr.brres"` (inline char array, no relocation — used
  directly as `r30+0x90` in the `getRes` call).
- `+0xa8` = `"koopaJr"` (inline; archive/model name, reused for both `getRes`
  and `GetResMdl`).
- `+0xb0` = `"mask"` (inline; `GetResNode` argument).
- `+0xb8` / `+0xc8` = `"character_SV"` / `"g3d/model.brres"` (inline;
  `CreateShadowModel` arguments — `r6` is a register copy of `r4`, so arg1
  and arg3 are the SAME string, `"character_SV"`, passed twice).

Not recovered: the `lbl_2_rodata_8C38` 4-entry int table read by
`lookupAction()` (`fn_2_16D920`), and the `GetResNode` flag-clear
(`rlwinm r0,r0,0,24,22`) — no header exposes a mutator for that ResNode flags
word. Both are left as explicit gaps in `createModel()`/`lookupAction()`
rather than guessed.

## Caveat on "MATCH" in this document

`difftool.py` is a raw textual diff of two `dtk elf disasm` dumps; it does
**not** perform the project's real symbol-pool canonicalisation (that lives
in `tools/auto_decomp/harness.py`'s `canonicalise()`, written for
`wiimj2d`/DOL-style 8-hex-digit symbols — `d_basesNP`'s REL-style
`fn_2_16D7F0`/`lbl_2_data_45F08` names don't match its regexes, so it would
not collapse these own-symbol differences either). Every "MATCH\*" above is a
reasoned claim (own vtable / own not-yet-split member, same address), not an
outcome verified against the project's actual link-time comparison — I have
not run, and was instructed never to run, `progress.py`/`land.py`.
