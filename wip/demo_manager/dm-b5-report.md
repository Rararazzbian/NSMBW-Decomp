# Batch 5 report — land/toride and the demo-number queue

Session cut short by time limit. Status below is accurate as of last compile.
`wip/demo_manager/dm-b5.cpp` is written and was last verified to COMPILE
(against scratch-patched headers, see "Blocking header findings" below).

## Verification method used

Compiled with `harness.compile_draft(src, obj, extra_inc=[<scratch dir with
two patched headers>])`, disassembled with dtk, compared per function via
`harness.diff_fn` against
`tools/auto_decomp/work/dol_bases_d_a_player_demo_manager/target.txt`.
Additionally ran, and confirmed:
- instruction-count x 4 == symbol-map size (`bin/dtk/wiimj2d_symbols.txt`)
  for all 11 functions — all matched exactly (see table below).
- scanned all 11 extracted target bodies for pooled-literal (`SYM`)
  references: **none found** — this batch touches no float/rodata pool
  constants, so the "compare pool constants as bytes" check has nothing to
  check for this batch.
- did NOT get to run the negative-control check before being cut off. Given
  everything above is a real per-function `diff_fn` comparison (not a
  same-name normalisation artifact — the two unnamed functions were
  extracted and compared as literal `fn_8005CCD0`/`fn_8005CE50` strings,
  confirmed by their sizes matching exactly against the symbol map), I am
  confident in the MATCH results below, but flagging the missing negative
  control explicitly per the verification standard.

## Per-function status

| Addr | Function | Bytes | Status |
|---|---|---:|---|
| 0x8005CC00 | `startControlDemoLandPlayer` | 208 | **BYTE-EXACT** |
| 0x8005CCD0 | `fn_8005CCD0` (unnamed, see below) | 252 | **BYTE-EXACT** |
| 0x8005CDD0 | `isLandAll` | 124 | **BYTE-EXACT** |
| 0x8005CE50 | `fn_8005CE50` (unnamed, see below) | 120 | **BYTE-EXACT** |
| 0x8005CED0 | `executeStartToride` | 216 | **BYTE-EXACT** |
| 0x8005CFB0 | `executeEndToride` | 160 | **BYTE-EXACT** |
| 0x8005D050 | `setCourseOutList(s8)` | 64 | **BYTE-EXACT** |
| 0x8005D090 | `checkDemoNo(s8)` | 40 | **BYTE-EXACT** |
| 0x8005D0C0 | `getNextDemoNo` | 8 | **BYTE-EXACT** |
| 0x8005D0D0 | `turnNextDemoNo` | 36 | **BYTE-EXACT** |
| 0x8005D100 | `clearDemoNo(s8)` | 380 | **CLOSE, NOT EXACT** — see below |

**10 of 11 functions confirmed byte-exact.** All instruction counts x 4
matched their `wiimj2d_symbols.txt` size exactly (including for the two
unnamed functions, extracted and compared by literal address-based name,
not by a same-name collision — the harness's known `fn_800XXXXX`-normalises-
to-bare-`fn` defect does not apply here since both sides matched on the
full `fn_8005CCD0`/`fn_8005CE50` string).

## The one unresolved function: `clearDemoNo(s8 playerNo)`

Current source (in the landed file) produces **97 instructions vs target's
95**, structurally very close but NOT byte-exact. Two distinct, independent
residual issues, both purely register-allocation/codegen-shape artifacts —
the algorithm itself (filter out `-1`/`playerNo` entries from the 4-slot
`mDemoNoQueue`, compact survivors to the front via a pointer that starts at
`this` and advances 4 bytes per kept entry — addressing every store via
`this_offset->mDemoNoQueue[0]` rather than a flat `int*`, confirmed against
the real disassembly's `stw r6, 0x60(r3)` then `stw r6, 0x60(r5)` where
`r5 = this+4` — then fill the remaining slots through the end of the
COMBINED 8-word `mDemoNoQueue`+`mCourseOutList` span with -1) is proven
correct: every store address, branch shape, and the first compaction block
(slot 0) are exactly byte-for-byte matching.

**Issue 1 (3 of 4 compaction blocks): register swap, not a logic bug.**
Target keeps `playerNo` (sign-extended once) in r0 and reuses r6 for each
loaded slot value across all 4 blocks. My draft gets slot 0 correct (r0/r6,
exact match, confirmed) but slots 1-3 land the loaded temp in r4 instead of
r6 (with `value` still correctly in r0) — a pure register-allocator
preference difference. I tried, and ruled out:
- reusing one `int loaded` variable across all 4 slots (makes block 0
  ALSO wrong — worse)
- one fresh variable per block, `loaded0`..`loaded3` (**this is what's
  landed** — gets block 0 exactly right, blocks 1-3 wrong in the same way)
- reordering the prologue statement order (value/dst/count/first-load) —
  made it worse (broke block 0 too)
- `int *dst` vs `daPyDemoMng_c *dst` pointer typing for the compaction
  write target — byte-identical result either way, not the lever
- `register` keyword on the loaded locals — no effect (MWCC ignores it
  here, as expected at -O4)
- swapping comparison operand order (`loaded != value` vs `value !=
  loaded`) — this WAS a real lever, already applied and confirmed correct
  (it fixes operand order in the emitted `cmpw`, independent of the
  register-choice issue)

**Issue 2 (tail fill loop): one redundant bounds check.**
Target's tail ("fill from `count` through index 7 with -1") compiles to a
genuine MWCC Duff's-device-style unrolled-by-8 loop with NO separate
loop-entry guard beyond the earlier `if (count>=4) return;` (i.e. only ONE
`cmpwi ...,4; bgelr` appears in the whole tail). My `for (int i = count; i
< 8; i++) mDemoNoQueue[i] = -1;` reproduces the SAME Duff's-device shape
(confirmed — this was the key unlock, see ruled-out variants below) but
adds a second, redundant `cmpwi r7,0x8; bgelr` immediately before it, from
the for-loop's own entry check that the compiler doesn't elide even though
it's provably dead given the preceding `count>=4` return.
Tried and ruled out:
- pointer-walk (`int *p = &mDemoNoQueue[count]; ...; *p=-1; p++;`) instead
  of an indexed loop — compiles to a completely different, SIMPLER
  non-Duff's-device loop (doesn't match target's shape at all, was my
  original/naive draft)
- `mDemoNoQueue[count]` reused directly as the loop variable (`for (;
  count<8; count++)`) instead of a fresh `int i = count` — collapses to a
  genuine runtime `bdnz`/`andi.`/`beqlr` loop (62 instructions, totally
  different family, worse)
- `do { ... } while (i<8)` instead of `for` — MUCH worse (134
  instructions, spills a callee-saved register r31, real loop with no
  unrolling at all)
- **`for (int i = count; i < 8; i++)` (indexed, fresh `i`) is the version
  landed** — closest by far (97 vs 95), reproduces the Duff's-device shape,
  just carries the one redundant guard

**Next step for whoever picks this up:** the redundant guard (Issue 2) and
the r4-vs-r6 register swap (Issue 1) are very likely both symptoms of the
SAME underlying difference in how the real source expressed the
"count already known < 4" fact to the compiler — I did not find the right
phrasing before running out of time. Given issue 1 has a WORKING block-0
precedent (only need to make blocks 1-3 imitate block 0's allocation), and
issue 2's shape (Duff's device) is already unlocked, this is a real but
narrow gap, not a wrong-algorithm problem.

## Invented names for the two file-statics — REPORT THESE ADDRESSES EXPLICITLY

Per the brief, both unnamed-in-symbol-map functions are given **literal
`fn_0x8005XXXX`-style names as their real C++ identifiers**, matching this
project's existing convention for this exact situation (see
`include/game/bases/d_a_player_manager.hpp`'s `fn_8005f4d0`/`fn_8005f570`,
which are kept in placeholder form rather than given descriptive names).
This was necessary, not just stylistic: the harness's `extract()` only
recognises the literal `fn_[8 hex digits]` pattern as a name-preserving
placeholder (`PLACEHOLDER_FN` regex in `tools/auto_decomp/harness.py`), so
extracting/diffing against the target's placeholder-named symbol REQUIRES
the draft to emit that exact literal name too. Both are declared
`extern "C"` specifically to suppress C++ name mangling so the compiled
symbol literally is `fn_8005CCD0`/`fn_8005CE50` (verified empirically: an
ordinary mangled C++ name made `diff_fn` report "DRAFT MISSING" even
though the function compiled fine — the body was right, the name lookup
just couldn't find it).

- **`fn_8005CCD0`** at **0x8005CCD0** (252 B). Signature:
  `bool fn_8005CCD0(daPyDemoMng_c *mgr, int step)`. A 0-4 step dispatcher
  for the control-demo/toride state machine: gates each transition on
  `isDemoMode()`/`isLandAll()` and writes `mgr->m_08` (1, 3, or 5) on
  success. **NOT marked `static`** — see finding below, this contradicts
  the brief's premise.
- **`fn_8005CE50`** at **0x8005CE50** (120 B). Signature:
  `static void fn_8005CE50(daPyDemoMng_c *mgr, daPlBase_c::AnimePlayArg_e
  animID)`. Loops the 4 controllable players and calls
  `player->setControlDemoCutscene(animID)` on each. Confirmed 3 in-TU
  callers (`executeStartToride` once, `executeEndToride` twice), so
  `static` (internal linkage) is correct here per the brief's own test.
  Both parameters are explicit (free-function style, not a real member of
  `daPyDemoMng_c`) specifically so nothing needs to be added to the frozen
  header — `mgr` is unused inside the body but IS genuinely passed by
  every caller (confirmed: `executeStartToride` does an explicit `mr r3,
  r29` reload right before the call because r3 had been clobbered by an
  earlier call, which only makes sense if the callee's first argument is
  meaningful/expected, even though the body itself ignores it).

**Finding, reported not reconciled: `fn_8005CCD0` likely cannot actually be
`static`.** I grepped the FULL unit disassembly (all 8,976 B / 51
functions, `tools/auto_decomp/work/dol_bases_d_a_player_demo_manager/target.txt`)
for the string `fn_8005CCD0` and found only its own `.fn`/`.endfn` pair —
**zero in-TU call sites anywhere in the 51-function unit.** The brief's own
stated test ("if they do [have in-TU callers], static is correct") implies
the converse holds too: with no in-TU caller found anywhere, something
outside this TU must call it, so internal linkage would be wrong. I did
NOT find that external caller (checked `tools/dis/corpus_CMP_dol_bases_d_a_player.txt`
and `..._d_a_player_base.cpp`'s corpus text, both come back clean) — this
is a genuine open question for the lead, not something I resolved. Landed
without `static` as the safer default. This does not affect the proven
byte-exactness of the function body itself (linkage doesn't change codegen
here — every `.fn` in this unit's disassembly shows `global` regardless of
whether the brief says a function should be static, so this is a linkage
question for final assembly/linking, not a body-correctness one).

## Blocking header findings — reported, NOT applied to the real headers

Both proven correct via a SCRATCH COPY of the two headers (with exactly
these changes) and `harness.compile_draft`'s `extra_inc` parameter to
shadow the real `include/` path for testing only. The real
`include/game/bases/d_a_player_demo_manager.hpp` and
`include/game/bases/d_s_stage.hpp` were NOT touched, per the hard rule.

1. **`include/game/bases/d_a_player_demo_manager.hpp` line 110:**
   ```
   void startControlDemoLandPlayer();
   ```
   must become
   ```
   bool startControlDemoLandPlayer();
   ```
   Doubly proven: (a) the tail of the target function computes a canonical
   0/1 value into r3 immediately before the epilogue via the same
   `neg`/`or`/`srwi` idiom the header's own comments already call
   "confirmed bool" for `startControlDemoAll`/`isAllPlayerControlDemo`; (b)
   its caller, `executeEndToride` (also in this batch, case `m_08==1`),
   uses the return value directly: `bl startControlDemoLandPlayer; cmpwi
   r3,0x0; beq ...`. A void function cannot produce either of these.

2. **`include/game/bases/d_s_stage.hpp`** needs a new declaration added
   (there is currently no `ReplayEnd` anywhere in the repo — grepped both
   `include/` and every `tools/dis/*.txt` corpus/disassembly file):
   ```
   static void ReplayEnd();
   ```
   Proven: `executeEndToride` (case `m_08==1`, success path) calls `bl
   ReplayEnd__10dScStage_cFv` with NO `this`-pointer load beforehand — the
   only value in r3 at that call site is `startControlDemoLandPlayer`'s
   just-returned boolean (nonzero, since the branch guarding the call
   requires it), which cannot be a valid `dScStage_c*`. This only makes
   sense for a static member with no object argument.

## `isLandAll`'s `bool` return type

**Held up exactly, no lever needed.** Compiled correctly on the very first
attempt with a straightforward `bool` return and an early `return false;`
inside the loop (no accumulator needed here, unlike
`startControlDemoLandPlayer` — see below). Matches the sibling map's
Finding 5 exemplar pairing (`isAllPlayerControlDemo`/`isLandAll` as "the
boolean-return variant").

## Levers that worked (for the lead's cross-batch notes)

- `daPyMng_c::checkPlayer(i)` (inline accessor) instead of a hand-written
  `mActPlayerInfo & (1<<i)` bitmask — used throughout, confirmed correct
  everywhere it appears in this batch.
- **`startControlDemoLandPlayer`'s accumulator needed `int`, not `bool`**
  (confirms the cross-batch relay's finding 3): with `int allDone`, MWCC
  emits the `neg`/`or`/`srwi` canonicalisation tail matching the target
  exactly. Also needed an `if (cond) {...} else { allDone = 0; }` shape
  rather than `if (!cond) { allDone = 0; continue; }` — the `continue`
  form produced a different (still logically equivalent, but byte-wrong)
  branch layout.
- `switch` (not `if`/`else if` chain) for `fn_8005CCD0`, `executeStartToride`,
  `executeEndToride` — the target's dense case-comparison-chain (all
  `cmpwi`/`beq` pairs emitted up front, before any case body) only came out
  of a real `switch`; an if/else-if chain interleaved each test with its
  own body instead.
- `fn_8005CCD0`: **failure paths must all funnel to ONE shared trailing
  `return false;`** via `break;` inside each `case`, not a `return false;`
  written inline in each case — the latter duplicated the false-path
  `li r3,0` sequence per case (code bloat) instead of every case's failure
  jumping to one shared tail, which is what the target does.
- `checkDemoNo`: **explicit `goto` beat every structured-control-flow
  phrasing.** `mDemoNoQueue[0]==-1 || ==playerNo` (direct `||`),
  `if/else if` with a `bool result` temp, and nested `if`/`return`
  variants were all tried; every one of them either got optimised into a
  branchless `subf`/`cntlzw`/`srwi` idiom (when the two "true" returns
  were adjacent enough for MWCC to prove trivial reducibility) or came out
  with correct logic but the wrong block ORDER (true-block after
  false-block, opposite of target). Landed form:
  ```cpp
  if (mDemoNoQueue[0] == -1) { goto ret_true; }
  if (mDemoNoQueue[0] != playerNo) { goto ret_false; }
  ret_true: return true;
  ret_false: return false;
  ```
  This is the one lever in this batch I'd flag as most likely to recur —
  worth trying early on any other small bool-OR-shaped predicate in
  remaining batches.

## Files

- `wip/demo_manager/dm-b5.cpp` — the 11 functions, current state as
  described above (10/11 byte-exact, `clearDemoNo` close-not-exact).
- This report: `wip/demo_manager/dm-b5-report.md`.
- Scratch header copies (proof only, not part of the deliverable):
  `%TEMP%/.../scratchpad/b5_inc/game/bases/d_a_player_demo_manager.hpp`
  and `.../d_s_stage.hpp` — these live in my session scratch directory,
  NOT under version control, and will not survive past this session. The
  lead needs to apply the two header changes above independently.
