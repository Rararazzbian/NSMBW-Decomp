# WM_ANTLION_MNG (daWmAntlionMng_c) -- mapping / resume notes

Unit range (build.py's own verify_anon window): `.text 0x15b564-0x15c1d4`, 22
target functions -- confirmed consistent with the HANDOFF-recorded corrected
scope (`0x15b590-0x15c200`, 22 functions, NOT the ~79 that combined span
through WM_BOARD). `daWmAntlionMng_c : public dWmDemoActor_c`, `sizeof == 0x1b0`.
Full class layout, vtable analysis and per-function provenance are already
documented at exhausting length in the class-level and per-function doc
comments in `d_a_wm_antlion_mng.cpp` -- not repeated here.

## Tally (measured, not taken on faith): 18/22, UNCHANGED this session

**No source changes made to this unit this round.** See "Why this unit was
left alone" below -- this was a deliberate decision, not an oversight.

```
0x0015b570  __arraydtor$12465                 7   MATCH  (see order-check note)
0x0015b590  daWmAntlionMng_c_classInit__Fv    12   MATCH
0x0015b5c0  __ct__16daWmAntlionMng_cFv        17   MATCH
0x0015b610  __dt__16daWmAntlionMng_cFv        34   MATCH
0x0015b6a0  create__16daWmAntlionMng_cFv      30   MATCH
0x0015b720  execute__16daWmAntlionMng_cFv     32   MATCH
0x0015b7a0  draw__16daWmAntlionMng_cFv         2   MATCH
0x0015b7b0  doDelete__16daWmAntlionMng_cFv     2   MATCH
0x0015b7c0  resetTimer__16daWmAntlionMng_cFv   3   MATCH
0x0015b7d0  procNone__16daWmAntlionMng_cFv     1   MATCH
0x0015b7e0  setActive__16daWmAntlionMng_cFv    4   MATCH
0x0015b7f0  procCheck__16daWmAntlionMng_cFv   15   MATCH
0x0015b830  processCutsceneCommand...        144   MATCH
0x0015ba70  pickRevivedIndices...             112   39 differing (register allocation)
0x0015bc30  reviveOnRoute...                   92   29 differing (register allocation)
0x0015bda0  rebuildAllModels...                53    8 differing (register allocation)
0x0015be80  clearAllModels...                  40    8 differing (register allocation)
0x0015bf20  checkAllRevivalCountsZero...       24   MATCH
0x0015bf80  checkAttackSequenceDone...         84   MATCH
0x0015c0d0  primeRevivalCount...               26   MATCH
0x0015c140  GetActorType__14dWmDemoActor_cFv    2   MATCH
0x0015c150  "__sinit_..."                      33   MATCH

18/22 byte-identical modulo symbol names
```

## Why this unit was left alone this session

**Verified: every one of the 22 target functions already has a full,
non-stub C++ implementation in the draft.** There is no unwritten function
here -- the 4 remaining gaps (`pickRevivedIndices`, `reviveOnRoute`,
`rebuildAllModels`, `clearAllModels`) are all STRUCTURALLY CORRECT (same
instruction count/shape as the target in the two smaller ones; confirmed
`reviveOnRoute`/`pickRevivedIndices` too, per their own doc comments) and
differ ONLY in register numbering. Each one's own doc comment already records
a substantial iteration history:

- `pickRevivedIndices`: 107 -> 39 differing across the session that wrote it.
- `reviveOnRoute`: 89 -> 46 -> 29 differing, with FIVE additional source
  permutations measured this past round alone (widened scope, a sibling
  default-argument overload rejected by MWCC with error 10199, three
  register/declaration-order variants) -- all recorded as tried and failed.
- `rebuildAllModels`: 30 -> 8 differing.
- `clearAllModels`: 18 -> 8 differing, explicitly recorded as "Logic right,
  allocation wrong; parked rather than chased" days before this session even
  started (HANDOFF, WM_ANTLION_MNG 15/22 entry).

**This is precisely the "known wall with 20+ variants already measured"
situation this session's brief explicitly says to avoid grinding further
without a genuinely new axis** -- and I did not have one. Per the project's
own hard-won finding (a sibling agent's full round against four such walls
produced one real fix and zero landings), effort was redirected to WM_ANCHOR,
which had genuinely unwritten/first-draft-only functions this round.

**This also means the "a unit's pool cannot be right while any contributing
function is unwritten" rule does not apply here** -- there is no unwritten
function to finish first; all four residuals are pure register-allocation
walls on functions whose logic is already confirmed correct, not
pool-completeness artifacts.

If a future round wants to attack these four, do NOT re-try any of the
specific variants already recorded as measured-and-failed in their own doc
comments (widened scope, `goto`, separate `bool reject` statement, the
default-argument overload, `||` vs nested `if`, `pos` scope changes) -- all
already tried, all already documented as not closing the gap further.

## Order-check finding (verified via the two target objects, not rebuilt)

`build.py`'s `verify_anon` reports `FUNCTION ORDER IS WRONG`, flagging every
function AFTER `__arraydtor$12465` as "defined too late." This is the SAME
class of tooling artifact found and fully explained on WM_ANCHOR this same
session (see that unit's own `MAPPING.md`), not a defect in this unit's
authored code:

- `__arraydtor$12465` (target `fn_2_15B570`, the very first address in the
  unit's range, `0x15b570`) is **NOT part of `daWmAntlionMng_c` at all.**
  Confirmed directly from its own target disassembly
  (`target_auto_0015B564.txt`): it references `lbl_2_data_43798` and
  `__dt__Q26dWmLib19ForceInCourseList_tFv` -- `dWmLib`'s own static array
  destructor (`sc_ForceList`, referenced in this unit's own top-of-file
  comment about `d_wm_lib.hpp`), a vague-linkage (weak) symbol that every TU
  including `d_wm_lib.hpp` generates its own byte-identical copy of. The
  linker keeps exactly ONE surviving copy via weak-symbol folding, and in the
  retail binary that surviving copy happens to link immediately before this
  unit's own `classInit` -- a link-order fact outside this TU's control, not
  something the source can position.
- The draft's own compile ALSO emits this same weak destructor (with a
  different auto-generated disambiguator, `$12465` here vs `$12782` on
  ANCHOR's copy of the same idiom) and it already reads byte-identical
  (`MATCH` in the table above) -- the content is right, only its REPORTED
  "position" in the source-order check is a false signal, because it is a
  different unit's retained COMDAT copy, not a function this unit defines at
  a controllable point in its own source.
- Analogous to ANCHOR's confirmed case (a duplicate weak `blr`-only symbol
  causing the same false "too late" report there), but **not independently
  rebuilt/re-confirmed here** since no source changes were made to this unit
  this session -- flagged as an inference by strong analogy, not a verified
  fact for THIS unit specifically. The lead should treat it as "very likely
  benign, same mechanism as the confirmed ANCHOR case" rather than "proven."

`check_fn_order.py` (blind to named/weak symbols) reports 0 inversions for
this file, consistent with there being nothing wrong in what the source
actually controls.

## Remaining work for the next agent

Four register-allocation walls (`pickRevivedIndices`, `reviveOnRoute`,
`rebuildAllModels`, `clearAllModels`), all logic-confirmed-correct, all with
substantial prior-round iteration recorded in their own doc comments. Per this
session's own finding, these do not look like they will close without a
genuinely new lever -- consider whether the project's broader
register-allocation-wall patterns (documented elsewhere in HANDOFF.md for
other units) suggest anything not yet tried here, rather than another
permutation of scope/order on the same four functions.
