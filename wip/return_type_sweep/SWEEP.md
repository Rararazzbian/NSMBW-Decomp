# Return-type sweep across 31 parked/in-progress WM units

Scope: every unit in the task list except `agent_lemmy` (already fixed) and the
non-unit probe directories. `agent_kinoko_red` compiled but has **no target
text files at all** in its directory (it is fully LANDED per HANDOFF -- there
is nothing left to diff against). 30 units were actually swept.

## Method (what was run, mechanically)

1. `wip/return_type_sweep/sweep.py` compiles each unit's own draft `.cpp`
   standalone via `harness.compile_draft(..., module='d_basesNP')` and
   `harness.disasm`, with the unit's own `shadow_include`/`include` directory
   (whichever exists) as the extra include path -- the same module and include
   convention every unit's own `build.py`/`iterate.py` already uses. Output
   goes to `wip/return_type_sweep/build/<unit>/draft.o` / `draft.txt`. **No
   file inside any `agent_*` directory was written.** `agent_castle` keeps no
   saved target text at all (its own `_scratch/iterate.py` disassembles three
   original split objects fresh every round instead), so the same three
   objects were disassembled into `wip/return_type_sweep/build/agent_castle/`.

2. A first pass tried `harness.extract()` (name-based lookup). It failed
   almost completely -- 1638 of 1667 functions read as "unauthored" -- because
   nearly every function in this WM family is an **anonymous `fn_2_XXXXXX`
   target** (dtk could not resolve a symbol), and a placeholder name bakes in
   its address, which is different in a standalone compile than in the linked
   REL. That is a naming artefact, not missing work, and reporting it would
   have been a false negative sweep. Caught before it went in the table.

3. `wip/return_type_sweep/match.py` re-does the pairing with the algorithm
   `wip/wm_units/verify_anon.py` already uses project-wide for exactly this
   family: greedy content-based matching (modulo pool-symbol names and local
   branch labels, plus the documented trailing-`blr` forgiveness), consuming
   draft functions in **ascending emission order** against targets in
   **ascending address order**. For every target function this gives either a
   real match (`delta 0`) or the **closest remaining unused draft function by
   content-diff count**, exactly as `verify_anon.py` reports it, plus the
   instruction-count delta between the two.

4. `wip/return_type_sweep/analyze2.py` filters to functions whose target/draft
   length differs by exactly 1-3 words, per the task's tell.

5. `wip/return_type_sweep/find_tell.py` / `dump_epilogue.py` read the raw
   (non-normalised) target instructions for every such function and check
   whether the epilogue has a write to `r3` positioned after a register
   restore (`lwz rN, off(r1)`) and before `mtlr` -- the literal shape from the
   ninth-return-type finding.

Per-unit function counts (target = functions found in that unit's target text;
draft = functions the standalone compile emits; exact = byte-identical modulo
pool-symbol names, verify_anon's own bar):

| unit | target | draft | exact |
|---|---|---|---|
| agent_river | 69 | 27 | 18 |
| agent_koopajr | 57 | 29 | 15 |
| agent_kinopio | 38 | 28 | 16 |
| agent_castle_bg | 72 | 55 | 35 |
| agent_castle | 20 | 38 | 19 |
| agent_koopa_castle | 48 | 35 | 20 |
| agent_floor_jr_a | 29 | 47 | 26 |
| agent_water_move | 48 | 38 | 23 |
| agent_antlion_mng | 36 | 30 | 19 |
| agent_hanachan | 65 | 42 | 18 |
| agent_antlion | 81 | 47 | 43 |
| agent_anchor | 59 | 29 | 22 |
| agent_board | 46 | 25 | 15 |
| agent_course | 37 | 39 | 23 |
| agent_ghost | 31 | 31 | 4 |
| agent_gun_battery | 75 | 68 | 49 |
| agent_item | 55 | 17 | 8 |
| agent_killer | 58 | 40 | 23 |
| agent_killerbullet | 78 | 46 | 23 |
| agent_kinoballoon | 34 | 44 | 27 |
| agent_kinoko_base | 65 | 38 | 37 |
| agent_kinoko_red | -- (LANDED, no target text) | -- | -- |
| agent_kinoko_star | 27 | 19 | 10 |
| agent_manta | 86 | 26 | 19 |
| agent_nice_coin | 58 | 27 | 18 |
| agent_note | 14 | 22 | 13 |
| agent_sandpillar | 126 | 98 | 78 |
| agent_sinkship | 28 | 26 | 13 |
| agent_smallcloud | 29 | 34 | 17 |
| agent_start | 130 | 29 | 15 |
| agent_dance_pakkun | 17 | 31 | 9 |

Total examined: 1616 target functions across the 30 compiled units. 675 exact
(modulo pool names). 766 differ by more than 3 words (structural, out of this
sweep's scope per the task). **175 differ by exactly 1-3 words** -- the
candidate pool for the tell.

## The 175 candidates do not survive scrutiny -- here is why, and the check that separates real from noise

`verify_anon`'s own fallback pairing picks the *closest remaining unused draft
function by content-diff count* when nothing matches exactly -- explicitly
documented there as informational, not identification ("the nearest candidate
is often an unrelated function that happens to be a similar size"). Applied
naively, 175 "candidates" is not 175 real correspondences: the same generic
draft function (a member destructor like `__dt__7mVec3_cFv`, `__dt__Q23m3d8anmChr_cFv`,
`__dt__14dWmDemoActor_cFv`, `GetActorType__12dBaseActor_cFv`, `checkCutEnd__14dWmDemoActor_cFv`)
gets reused as the "closest" filler for many unrelated, still-unauthored
targets purely because it happens to be a similar size. That is not a draft
function with a wrong return type; it is an **unwritten** target function
matched to unrelated already-written boilerplate.

**Filter applied:** keep only candidates where the picked draft function is
used as the closest candidate **exactly once** across the whole unit
(multiplicity 1). A generic filler reused for several different targets fails
this immediately; a real correspondence should be unique. This reduced 175 to
**18**.

### Table 1 -- the 18 unique-candidate small mismatches (highest-confidence real correspondences)

Every one of these was read by hand in the target's raw disassembly. **None
has a write to `r3` in the register-restore-then-`mtlr` epilogue shape.**

| unit | addr | target (len) | draft candidate (len) | delta | target epilogue does with r3 |
|---|---|---|---|---|---|
| agent_board | 0x15cc30 | fn_2_15CC30 (26) | `__dt__14dWmDemoActor_cFv` (29) | +3 | plain restore, no r3 write |
| agent_board | 0x15d920 | fn_2_15D920 (4) | `finalUpdate__12dBaseActor_cFv` (1) | -3 | `lwz;srwi.;bnelr;b` -- conditional early-return void, no r3 write |
| agent_board | 0x15de70 | fn_2_15DE70 (1) | `checkCutEnd__14dWmDemoActor_cFv` (2) | +1 | single `b` tail call, no epilogue at all |
| agent_dance_pakkun | 0x161a50 | fn_2_161A50 (54) | `__dt__17daWmDancePakkun_cFv` (53) | -1 | plain restore, no r3 write |
| agent_dance_pakkun | 0x1620c0 | fn_2_1620C0 (35) | `startStep__17daWmDancePakkun_cFv` (37) | +2 | plain restore, no r3 write |
| agent_floor_jr_a | 0x83b00 | fn_2_83B00 (84) | `unk_83B00__12daFloorJrA_cFv` (83) | -1 | `bl _restgpr_26`, plain restore, no r3 write |
| agent_gun_battery | 0xf8630 | fn_2_F8630 (23) | `__dt__sFStateMgr_c<...>` (25) | +2 | plain restore, no r3 write |
| agent_gun_battery | 0xf8e80 | fn_2_F8E80 (18) | `timerOrKeyGate__...` (19) | +1 | leaf, ends `mr r3,r6; blr` (real int return) -- but draft already returns int (`li r3,0x0`/`li r3,0x1` in its own body); confirmed by reading the draft, not a return-type gap |
| agent_gun_battery | 0xf8f70 | fn_2_F8F70 (116) | `executeState_ShowRule__...` (113) | -3 | plain restore, no r3 write |
| agent_killerbullet | 0x1697b0 | fn_2_1697B0 (76) | `unk_1697B0__18daWmKillerBullet_cFPCf` (78) | +2 | leaf, ends `mr r3,r0; blr`; draft already returns int (`li r3,0x0`/`li r3,0x1`) -- confirmed by full read, not a return-type gap |
| agent_killerbullet | 0x1698e0 | fn_2_1698E0 (167) | `unk_1698E0__18daWmKillerBullet_cFv` (166) | -1 | plain restore, no r3 write; draft's own epilogue is the same void shape -- confirmed by full read |
| agent_kinoballoon | 0x16a470 | fn_2_16A470 (77) | `createModel__17daWmKinoBalloon_cFv` (76) | -1 | `bl _restgpr_27`, plain restore, no r3 write |
| agent_kinoballoon | 0x16b120 | fn_2_16B120 (17) | `__ct__Q23m3d8anmChr_cFv` (15) | -2 | plain restore, no r3 write |
| agent_kinoko_base | 0x740 | `calcAnim__16daWmDokanRoute_cFv` (4) | `vf7C__16daWmKinokoBase_cFv` (1) | -3 | vtable-dispatch thunk (`lwzu;lwz;mtctr;bctr`), not a return path at all -- also belongs to a *different* class (`daWmDokanRoute_c`, a shared base bleeding in from a neighbouring split object) |
| agent_kinopio | 0x16d050 | fn_2_16D050 (44) | `checkAnmLoop__13daWmKinopio_cFv` (45) | +1 | paired-single/float restore, no r3 write |
| agent_koopa_castle | 0x192780 | fn_2_192780 (2) | `finalUpdate__12dBaseActor_cFv` (1) | -1 | `li r4,0x7d0; b procDemoLoseBase__10dWmEnemy_cFs` -- tail call, no r3 write |
| agent_nice_coin | 0x105b20 | fn_2_105B20 (3) | `executeState_EndWait__...` (1) | -2 | `li r0,0; stw r0,0(r3); blr` -- r3 is the `this` pointer for the store, not a return |
| agent_nice_coin | 0x106580 | fn_2_106580 (3) | `GetActorType__12dBaseActor_cFv` (2) | -1 | `li r0,0; sth r0,0x3fe(r3); blr` -- same, r3 is input not output |

Two of these (the killerbullet pair) were verified further than "read the
epilogue" -- their draft bodies were read in full (both ~80-170 instructions)
because the naming convention (`unk_<address>__ClassNameFv`) strongly suggests
a real, deliberate correspondence rather than a size coincidence. Both are
already correctly typed: `unk_1697B0` already returns int via `li r3,0x0`/
`li r3,0x1` branches matching the target's own `mr r3,r0` merge; `unk_1698E0`
already ends in the same void restore-block the target does. Their length
deltas are ordinary content/register differences, not return-type bugs.

### Table 2 -- 20 targets whose epilogue DOES have the tell shape, and why every one is a false lead

Running the mechanical epilogue check across all 175 candidates (not just the
unique 18) finds 20 target functions with a genuine
register-restore-then-`r3`-write-then-`mtlr` epilogue. This set is **disjoint**
from Table 1 -- none of these 20 addresses appear there. In every case the
picked "draft candidate" is a generic, multiply-reused filler (mostly
`__dt__7mVec3_cFv`, `__dt__Q23m3d8anmChr_cFv`, `__dt__14dWmDemoActor_cFv`,
`__ct__Q23m3d8anmChr_cFv`, `classInit_AC_NICE_COIN__Fv`), confirmed reused for
2 or more different targets in the same unit. The `li r3, 0x1` (or equivalent)
epilogue itself is the well-documented, already-known **`SUCCEEDED` return
convention** used throughout this actor framework (see
`AGENT_CONTEXT.md`'s vtable section: `virtual int doDelete() { return
SUCCEEDED; }`) -- it marks these targets as ordinary `int`-returning
`create()`/`classInit`-style functions that are simply **not yet authored**,
not an existing draft function with the wrong type.

| unit | addr | target (len) | filler candidate (len) | delta | multiplicity of this filler in the unit |
|---|---|---|---|---|---|
| agent_anchor | 0x15af10 | 12 | `__ct__Q23m3d8anmChr_cFv` (15) | +3 | reused elsewhere in unit |
| agent_antlion | 0x15a700 | 25 | `create__13daWmAntlion_cFv` (26) | +1 | reused |
| agent_antlion | 0x15b6a0 | 30 | `__dt__14dWmDemoActor_cFv` (29) | -1 | reused (2x in unit) |
| agent_antlion | 0x15b720 | 32 | `__dt__14dWmDemoActor_cFv` (29) | -3 | reused (2x in unit) |
| agent_castle_bg | 0xf6770 | 30 | `create__27daMiddleBGForCastleLudwig_cFv` (32) | +2 | reused |
| agent_course | 0x161d70 | 17 | `__dt__7mVec3_cFv` (16) | -1 | reused |
| agent_gun_battery | 0xf9ef0 | 19 | `__dt__13sStateFctIf_cFv` (16) | -3 | reused |
| agent_kinoko_base | 0x750 | 47 (`daWmDokanRoute_c::calcModel`) | `calcModel__16daWmKinokoBase_cFv` (44) | -3 | different class entirely (neighbour bleed-through) |
| agent_kinopio | 0x16d530 | 19 | `__dt__7mVec3_cFv` (16) | -3 | reused |
| agent_koopajr | 0x16eb10 | 18 | `__dt__7mVec3_cFv` (16) | -2 | reused |
| agent_manta | 0x172190 | 31 | `__dt__14dWmDemoActor_cFv` (29) | -2 | reused (many times in unit) |
| agent_manta | 0x175060 | 14 | `__ct__Q23m3d8anmChr_cFv` (15) | +1 | reused (many times) |
| agent_nice_coin | 0x1059c0 | 20 | `classInit_AC_NICE_COIN__Fv` (19) | -1 | reused |
| agent_river | 0x12ba60 | 21 | `__dt__15daRiverPakkun_cFv` (22) | +1 | reused (10x in unit) |
| agent_sandpillar | 0x1794b0 | 23 | `__dt__Q23m3d8anmChr_cFv` (22) | -1 | reused |
| agent_sandpillar | 0x179510 | 31 | `__dt__14dWmDemoActor_cFv` (29) | -2 | reused |
| agent_smallcloud | 0x17a900 | 12 | `__ct__Q23m3d8anmChr_cFv` (15) | +3 | reused |
| agent_start | 0x1811f0 | 19 | `__dt__7mVec3_cFv` (16) | -3 | reused (many times) |
| agent_start | 0x181240 | 18 | `__dt__7mVec3_cFv` (16) | -2 | reused (many times) |
| agent_water_move | 0x153690 | 18 | `__dt__Q23EGG8Vector3fFv` (16) | -2 | reused |

## What was TESTED (compiled, before/after)

No source change was tested. There is nothing in Table 1 that both (a) has the
tell's epilogue shape and (b) pairs to a real, non-generic candidate -- the
precondition for a fix even being available to try. The two candidates worth
testing on their face (killerbullet's address-named pair) were disproved by
reading the draft body directly: both already return the type their target
requires. No compile experiment was warranted beyond that read.

## Bottom line

**0 of 31 units move as a result of this sweep.** The specific tell from the
ninth-return-type finding -- a draft function one word short of its target,
where the target epilogue writes `r3` between a register restore and `mtlr` --
does not occur anywhere in the 30 units that could be checked (`agent_lemmy`
already fixed; `agent_kinoko_red` has no target text left to check, being
fully landed).

This is a genuine negative, not an absence of looking: 1616 target functions
were measured against their compiled drafts, 175 had a length delta in the
1-3-word range the tell requires, and every one of those 175 was traced back
to either (a) a unique, real correspondence with no r3-write in its epilogue
(18 cases, Table 1), or (b) a target that IS an int-returning `SUCCEEDED`-style
function but is simply **unauthored** -- there is no draft function to have a
wrong return type, only a size-coincidental filler picked by the nearest-
neighbour matcher (20 cases, Table 2, disjoint from Table 1). Two of the
strongest-looking Table 1 candidates were confirmed by a full manual read to
already be correctly typed.

If a tenth wrong return type exists in this project, it is not sitting in any
of these 30 units' currently-authored functions at a 1-3-word delta. It could
still exist among the 766 functions differing by more than 3 words, but that
is real missing content (multiple instructions, likely whole unauthored logic
blocks) rather than the narrow, mechanical single-instruction signature this
sweep was built to catch, and confirming a return type inside a >3-word gap
needs the function actually written first -- the A/B compile test
`AGENT_CONTEXT.md` prescribes, not a length diff.

## Files

- `wip/return_type_sweep/sweep.py` -- compiles every unit's draft standalone
- `wip/return_type_sweep/match.py` -- content-based target/draft pairing (reuses `wip/wm_units/verify_anon.py`'s algorithm)
- `wip/return_type_sweep/analyze2.py` -- filters to 1-3 word deltas
- `wip/return_type_sweep/find_tell.py` / `dump_epilogue.py` -- epilogue r3-write detector and raw-instruction dumper
- `wip/return_type_sweep/match_raw.json`, `small_mismatches.json`, `unique_hits.json` -- raw data behind the tables above
- `wip/return_type_sweep/build/<unit>/draft.o`, `draft.txt` -- the compiled drafts this sweep measured (never written into any `agent_*` directory)
