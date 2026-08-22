# Parked-unit audit — measured, not quoted, where measurement was possible

Produced read-only. All compiled objects for this audit live under
`scratch/parked_audit/`; nothing under `wip/`, `source/`, `include/`, `slices/`
or `syms.txt` was touched. Every unit below was checked against `source/` and
`git log` first to rule out "already landed" before being scored as open.

**Directories skipped per instruction** (actively worked this session):
`wip/castle_r2`, `wip/kokoopa_r6`, `wip/antlion_r2`, `wip/course_r2`,
`wip/linemng_cross`, `wip/linemng_circle`, `wip/kinoko_unblock` (the last two
of the seven and `antlion_r2` do not currently exist on disk).

**Methodology note on provenance.** Three tiers of confidence appear below,
labelled per row:
- **MEASURED-NOW** — I compiled the draft myself this session (via
  `harness.compile_draft`, objects in `scratch/parked_audit/`) and ran the
  unit's own scorer (`tally.py`, `eval.py`, or `verify_anon.py`) plus
  `poolcheck.py` where applicable.
- **RECORDED** — taken from `HANDOFF.md` or a `wip/*/RESULT.md`-equivalent
  written by the round that produced it, cross-checked for internal
  consistency (mtime order, session continuity) but not recompiled by me this
  session, usually because the residual is a well-documented, already
  multiply-measured wall (10-20+ prior variants) where re-running the compiler
  would only reproduce the same number.
- **STALE** — a figure that is superseded by a later, more-authoritative
  figure in the same lineage; flagged explicitly, not used for ranking.

---

## A. Ranked table — expected value = (functions/words that would land) / (effort to unblock)

Effort basis stated per row: LOW = one more mechanical step or a single build
attempt; MEDIUM = a few more source-level variants against an understood
mechanism; HIGH = the residual is an already-exhausted wall (10+ negative
variants on record) needing a genuinely new lever; UNCERTAIN = record itself
says "cannot land" or "structural limit."

| Rank | Unit | Measured score | Recorded score | Agree? | Blocker class | Blocker (one line) | Value | Effort |
|---|---|---|---|---|---|---|---|---|
| **0** | **4 already-landed units' slice claims** (`d_a_wm_manta.cpp`, `d_a_wm_sandpillar.cpp`, `d_a_floor_jr_b.cpp`, `d_a_peach_castle_sequence.cpp`) | **CONFIRMED missing from live `slices/d_basesNP.json`** — verified directly against the file on disk (28 entries now vs. 31 in the last known-good version, `45e72fe`) | All four `LANDED`/5-binaries-green per `git log`; source still compiles clean standalone | **DISAGREE — record says landed, live manifest says unclaimed** | BUILD (metadata regression, not source) | An unrelated commit (`01bdab9`, entirely about `d_line_mng`) silently deleted these 4 slice blocks and was never caught; see section C for the full chain and the exact fix | 4 units' worth of already-verified functions, recovered by a JSON edit alone | **LOWEST — pure metadata restore, zero source risk, exact fix already known** |
| 1 | **d_line_mng** (`wip/fix_bigtwo`) | **181/182 fns, 7531/7631w = 98.7%** MEASURED-NOW | 181/182, 98.7% (`LANDING.md`) | **AGREE** | CODEGEN RESIDUAL (link-blocking) | `smc_UNIT_SIZE_X` static-const-float: define-before-uses gets right `.sdata2` order but folds codegen wrong (2 fns regress); define-after gets right codegen but wrong order; omit entirely compiles but the link fails on the undefined symbol — no spelling found yet that satisfies both | 7631 words, the single largest unit on the board | MEDIUM — narrow, single question, extensively bounded already |
| 2 | **player_manager** | not re-run (already trial-linked) | 44/65 fns individually (67.7%), but **actually LINKED** in a full trial with 6 pins added | — | SHARED HEADER (syms.txt, lead-only) | Needs 6 new `syms.txt` pins + 32 old ones removed (all six targets identified, addresses known) + drop 2 stray destructors that cause the only real `.text` overflow (`+0x80` of `+0x90`) | 65 functions, a whole actor-manager class | LOW — mechanical; the hard part (does it link at all) is already proven yes |
| 3 | **agent_koopajr** (`wip/wm_units`) | not re-run | "PARKED as blocked-on-LINK, on three independent measurements" (HANDOFF) | — | possible BUILD/argument-class, unconfirmed | Same symptom class as `d_a_ac_switch`/`d_a_floor_jr_b`/`d_a_peach_castle_sequence`, all three of which turned out to be byte-complete and were unblocked by an actual build attempt, not more authoring | full small unit | LOW — try a real build/trial-link before authoring further |
| 4 | **agent_gun_battery** | **45/49 fns count CONFIRMED, but `verify_anon.py` re-run MEASURED-NOW additionally flags `FUNCTION ORDER IS WRONG`** | 45/49 fns (91.8%), `__sinit` exact 159/159 — doc does not mention any order problem | **DISAGREE** — count agrees, but a real defect is undocumented (see B8) | CODEGEN RESIDUAL + undocumented STRUCTURAL (definition order) | 4 narrow constructor-family residuals as recorded, **plus** a `.text` definition-order violation the unit's own doc never surfaced — per `AGENT_CONTEXT.md`'s "Function DEFINITION ORDER is part of the object," this can make every `bl` past the violation wrong even when every individual function is byte-identical | 49 fns | MEDIUM — but the order defect must be fixed FIRST, before the 4 known residuals are worth chasing further |
| 5 | **agent_floor_jr_a** | not re-run | 26/29 fns (89.7%) | — | BOUNDS + CODEGEN RESIDUAL (two items) | `setupBgCtr`: rodata pool-offset link-order artifact (BOUNDS-adjacent); `unk_83B00`: register-hoisting residual, no lever found; `__sinit`: 21 lines trace to one unclaimed 192-byte `.data` gap | 29 fns | MEDIUM |
| 6 | **d_enemy_toride_kokoopa** (`wip/kokoopa_verify5`) | **180/251 fns = 71.7%, 12840/31876 bytes = 40.3%, poolcheck 0 wrong constants (74 checked)** MEASURED-NOW | 180/251 (`matched_list.txt`, same round) — but see **B7**: a *later*, independently poolcheck-verified figure exists at 176/251 | AGREE with `kokoopa_verify5` itself, **DISAGREE with the true current state** | CODEGEN RESIDUAL, mostly **unfinished authoring**, not a wall | Most of the 71 unmatched functions are declared-but-not-yet-defined (see `check_defined.py` output), not failed matches — this is a large unit still being written, not blocked | 251 fns, largest un-landed unit after line_mng | MEDIUM-HIGH — bulk authoring volume, not a specific wall |
| 7 | **agent_castle** | not re-run | 19/20 (95%), "registration/guard entanglement, mechanism found" | — | CODEGEN RESIDUAL | Mechanism identified but not yet a working source shape | 20 fns | HIGH — already an identified-but-unsolved mechanism |
| 8 | **agent_koopa_castle** | not re-run | 16/17 (94.1%), "19 measured shapes on one construct" | — | CODEGEN RESIDUAL | One MWCC register-allocation decision, extensively tried | 17 fns | HIGH — 19 prior negative variants |
| 9 | **agent_course** | not re-run | 22/23 (95.7%), "21 measured variants" | — | CODEGEN RESIDUAL | `createModel`'s 2-attractor wall | 23 fns | HIGH |
| 10 | **agent_killer** | not re-run | 22/23 (95.7%) | — | CODEGEN RESIDUAL | "genuine CROSS-AXIS wall" | 23 fns | HIGH |
| 11 | **agent_kinoballoon** | not re-run | 24/26 (92.3%) | — | CODEGEN RESIDUAL | register-reuse scheduling, `_savegpr_27` already correct, one register does double duty | 26 fns | HIGH |
| 12 | **agent_lemmy** (`LEMMY_FOOTHOLD`) | not re-run | 42/51 (82.4%) | — | CODEGEN RESIDUAL | blocked on register choice, "not abandoned — if a lever for MWCC register allocation is ever [found]" | 51 fns | HIGH |
| 13 | **agent_antlion_mng** | not re-run | 18/22 (81.8%), "four walls, each with converging independent rewrites" | — | CODEGEN RESIDUAL | 4 separate residuals, actively converging | 22 fns | MEDIUM-HIGH |
| 14 | **agent_anchor** | not re-run | 19/22 (86.4%) | — | CODEGEN RESIDUAL | same inline-base-destructor class as `agent_river` (see below), but closer | 22 fns | MEDIUM-HIGH |
| 15 | **m_pad** (`clearWPADInfo`) | not re-run | 12/14 (85.7%) | — | CODEGEN RESIDUAL | register-allocation wall; 21 measured source shapes, all negative; no landed store-first sibling exists anywhere in the matched corpus to compare against | 14 fns, small | HIGH — exhaustively negative |
| 16 | **d_nand_thread** | not re-run | 16/21 (76.2%) | — | CODEGEN RESIDUAL | "chained-load residual... a genuine structural limit of expressing [it] in standard C++ under MWCC -O4, not a gap in technique" (author's own words) | 21 fns | UNCERTAIN — record itself doubts it's reachable |
| 17 | **agent_river** | not re-run | 15/23 (65.2%), "CANNOT land" | — | CODEGEN RESIDUAL | 8 of 9 destructors one instruction short (missing inner `this`-guard); 10 source-shape variants across 2 rounds, all negative; same wall class as `agent_anchor`, unsolved | 23 fns | UNCERTAIN — explicitly says cannot land as-is |

**Remaining open wm_units, RECORDED only (not independently re-run this
session — thinner documentation, lower individual value, ranked below the
cut line):** `agent_board` 11/15, `agent_start` 11/14, `agent_castle_bg`
24/33 (5 walls, `__sinit` puzzle open), `agent_kinopio` 14/19 (pool
short-and-misordered), `agent_item` 8/12, `agent_killerbullet` 19/37,
`agent_hanachan` 16/32, `agent_dance_pakkun` 9/16 (register-reuse wall, same
class as kinoballoon), `agent_water_move` last recorded 19/27 though a later
note says its `execute()` specifically reached N/N — **worth a fast re-check
before ranking**, `agent_nice_coin` 15/19 as of its first round only — likely
stale, **worth a fast re-check**, `d_bg_actor_mng` (see contradiction C4
below) — status unclear, ProcMain still far off on measured evidence.

---

## B. Contradictions and stale figures found

**B1 — `wip/fix_bigtwo/tally_final.txt` is stale by ~20 hours and 80
functions.** It reports `matched 101/182 functions 2122/7631 words = 27.8%
BY BYTES` (file mtime 2026-08-21 03:17). The same directory's `LANDING.md`
(mtime 2026-08-21 23:49) reports 181/182, 98.7%, and I confirmed 181/182,
98.7% MEASURED-NOW from the live `d_line_mng.cpp` in that same directory. The
101/182 figure is simply an earlier intermediate save left in place after
~20 more hours of work closed 80 more functions in the same session
(consistent with `HANDOFF.md`'s own session log: 101/182 → 165/182 → the
181/182 this document confirms) — not a genuine disagreement about the
unit's state, but exactly the kind of stale artifact that misleads a reader
who opens `tally_final.txt` without checking its neighbour. **Anyone citing
`wip/fix_bigtwo/tally_final.txt` is citing a ~20-hour-stale number; cite
`LANDING.md` or re-run `tally.py` instead.**

**B2 — `HANDOFF.md`'s own last word on `d_line_mng` (165/182, 65.9%,
line 14440) is itself now stale relative to `wip/fix_bigtwo/LANDING.md`
(181/182, 98.7%).** `HANDOFF.md` was not updated after the session that
closed the unit from 165→181; that later work lives only in
`wip/fix_bigtwo/LANDING.md` and the git-visible session log implied by file
mtimes. `HANDOFF.md` is the master record and should absorb this before the
next reader trusts its own "current state" framing for this unit.

**B3 — `wip/qwen_verify1` is not part of the `d_line_mng` cluster despite its
directory-name proximity to the other line_mng rounds.** Its draft
(`draft.cpp` = `d_bg_actor_mng.cpp` "SECOND DRAFT", targeting `ProcMain` at
`.text 0x8007E180-0x8007F7A0`) is a different unit entirely (Qwen's, per
`wip/fix_bigtwo/LANDING.md`'s own note that the shared `.text` hole also
contains `d_bg_actor_mng`). Its own `procmain_round20_diff.txt` shows the
draft and target are still structurally different (different loop shape,
different call sequence around `createActor`), not a register-permutation
residual — this function looks far from closed, and I could not establish
the whole-unit score for `d_bg_actor_mng` from the material in this
directory alone (out of this audit's declared scope, since it is not one of
the enumerated `wip/` directories in the task list, but it is a
misclassification worth flagging: **do not read `qwen_verify1` as evidence
about `d_line_mng`'s state.**

**B4 — `wip/wm_units/agent_wall`, `agent_kinoko`, `agent_pool_order`,
`agent_poolwall`, `agent_tempwall` are not parked ACTOR units at all**,
despite names that read like WM actors. Their contents are probe/scratch
workspaces reused for adjacent investigations: `agent_wall` holds a
`d_a_wm_kinoko_base.cpp` probe and a `dokan_route` disassembly; `agent_kinoko`
holds the (already-landed) `d_a_wm_kinoko_1up.cpp`; `agent_pool_order` is
scratch for "the passes rule" investigation (`HANDOFF.md:6568`);
`agent_poolwall` holds `cannon`/`dokan_route` probes; `agent_tempwall` holds a
wrapper-vs-bypass stack-slot experiment against the (already-landed) ghost
unit. **None of these five should be counted as open WM_WALL / AC_KINOKO /
AC_POOL_ORDER / WM_POOLWALL / WM_TEMPWALL drafts** — no such open units exist
in the current `wip/` tree under those names.

**B5 — `wip/return_type_sweep/SWEEP.md`'s table is not a liveness signal.**
Its 31-row table (target/draft/exact function counts per unit, dated
2026-08-20) includes 9 units that were **already landed** before the sweep
ran (`agent_antlion`, `agent_ghost`, `agent_manta`, `agent_note`,
`agent_sandpillar`, `agent_sinkship`, `agent_smallcloud`, `agent_kinoko_base`,
`agent_kinoko_star` — all landed 2026-08-17 through 2026-08-19, per `git
log`). The sweep script globs each directory's leftover `target_*.txt` files
regardless of landed status, so their presence in that table means nothing
about whether the unit is still open. Only `git log -- source/.../<file>`
settled this; do not use `SWEEP.md`'s table alone to judge openness.

**B6 — Sixteen `wm_units` sub-directories are entirely HISTORICAL
(landed), contrary to what a directory listing alone suggests** — see
section D for the full map (count corrected here: the table in section D
lists 16 landed `agent_*` directories plus 2 top-level duplicates
(`wm_kinoko_1up`, `wm_smallcloud`), not twelve).

**B7 — `kokoopa_verify5`'s 180/251 disagrees with a LATER figure of 176/251
from a parallel peer-AI round lineage — but the wrong-constant explanation
below does NOT hold up under an actual poolcheck run, so treat this as an
open sync question between two tracks, not a caught false positive.**
`git log` shows a later commit on `HEAD`, `e9ae5dd` ("Peer rounds 23/21
verified..."), whose `GEMINI_RESPONSE.md` reports **176/251 functions**, 4
*fewer* than `kokoopa_verify5`'s 180, despite being chronologically later.
It's true `kokoopa_verify5/eval.py` never calls `poolcheck.py` itself (grep
confirms zero references) — but a separate `poolcheck.py` pass **was**
independently run against `kokoopa_verify5`'s actual matched set this
session (both by a pre-existing `_poolcheck/` subdirectory already sitting
in that folder, and again from scratch by the sub-agent covering this
cluster, reading `source`/`include` fresh and writing only into
`scratch/parked_audit/kokoopa/`): **74 pooled constants compared by value
across the 180 paired functions, 0 mismatched, 0 unresolved.** So the
wrong-constant trap does NOT appear to be the explanation for the 180-vs-176
gap — kokoopa_verify5's 180 survives the exact check that would catch it.
The likelier explanation is that the peer-AI round lineage (Qwen/Gemini,
tracked outside `wip/` per this project's ownership rules — see
`AGENT_CONTEXT.md` §3) is simply a **different, not-yet-synced draft** that
hasn't incorporated `kokoopa_verify5`'s incremental gains yet, rather than a
regression or a false positive. **Still worth the lead's five-minute
reconciliation** (diff the two matched-function lists by name to see whether
176 ⊂ 180 or whether they actually disagree on which functions match), but
the specific "wrong-constant trap resurfaced" theory is not supported by the
poolcheck evidence gathered this session.

**B8 — `agent_gun_battery`'s own documentation is silent about a real
`FUNCTION ORDER IS WRONG` defect that a fresh `verify_anon.py` run surfaces
immediately.** The 45/49 count itself is confirmed and not in question, but
`AGENT_CONTEXT.md`'s own precedent (`d_a_wm_smallcloud.cpp`'s "clean 16/16"
that still failed to match because of definition order) is exactly this
situation: a per-function MATCH count says nothing about whether the
functions are defined in the address order the linker will place them in,
and a violation there corrupts every `bl` displacement past it even though
every individual function still diffs clean. **This unit should not be
treated as "45/49, close" without first fixing the order problem** — the
45 matching functions could still fail to link correctly as a whole even if
the remaining 4 residuals were solved, exactly as `d_a_wm_smallcloud.cpp`
did before its order fix. This is a genuine, previously-unrecorded finding
from this session, not a restatement of anything in `wip/wm_units/agent_gun_battery`'s
own docs.

---

**B9 — `m_pad` and `nand_thread`'s own fraction disagrees between two
independent measurement passes within this same audit, and I did not
reconcile it.** This document's ranked table (row 15/16) cites `m_pad`
12/14 and `d_nand_thread` 16/21, both marked RECORDED from `wip/`
documentation. A separate pass run in parallel this session reported
MEASURED figures of `m_pad` 10/12 and `d_nand_thread` 12/15 — smaller
denominators in both cases, suggesting the two passes scoped "the unit"
differently (e.g. named functions only vs. every function in the target
range, or a different target-file cut). Both numbers land in a similar
~80-85% ballpark, so the qualitative picture (small, deeply residual,
near-done) does not change, but the two counts cannot both be the
denominator of the same unit and I have not settled which is right.
**Re-run both units' own scorer once and record which target-function list
it used before citing either number.**

## C. DONE-PENDING-LANDING — what the integrator should pick up the moment the build is green

**Nothing in this audit is unconditionally DONE-PENDING-LANDING** — every
candidate needs at least one more concrete action, but two are extremely
close and should be first in line:

1. **`player_manager`** — already proven to LINK in a full trial. Remaining
   steps are entirely mechanical and known in advance: copy
   `wip/player_manager/assembled.cpp` to `source/dol/bases/`, insert the
   13-line slice block (exact text preserved in `wip/player_manager` git
   history per its own note), remove 2 stray destructors
   (`__dt__Q23EGG8Vector2fFv`, `__dt__Q23EGG8Vector3fFv`) to close the
   `.text` overflow from `+0x90` to `+0x10`, and apply the 6 `syms.txt` pins
   documented in `TRIAL_LINK.md` (addresses already resolved). This is the
   single lowest-effort, highest-confidence landing candidate on the board.
2. **`d_line_mng`** — landable in every section except the one
   `smc_UNIT_SIZE_X` question (see A1/B1/B2). All seven section bounds were
   independently re-derived and cross-checked against `dtk_splits_wiimj2d.txt`
   and confirmed adjacent to landed neighbours on six axes at once — that
   part of the work is finished and needs no repeating.

Both were also thought, at first pass, to be gated by the **BUILD** blocker
`HANDOFF.md` records as open at its last entry (2026-08-21 21:05, commit
`e4da0d6`): "`d_a_wm_manta.cpp` fails to compile (MWCC 10319, ambiguous
`daWmManta_c` access)" and `slice_rel.py` emits a negative `.bss` filler
(`-0x10360`) for `d_basesNP`. **Full git-history reconstruction (below)
shows both of those specific problems were fixed six minutes after HANDOFF's
entry — but a THIRD, later, currently-live regression replaced them, and it
is the real blocker today.** This required three rounds of correction within
this audit itself (an earlier draft of this section wrongly called the
manta issue "stale with nothing to do"); the full chain, reconstructed from
`git log`/`git show`, not guessed:

1. `0cf2553` (dummy_door landing) hand-resorted `slices/d_basesNP.json`
   **alphabetically**. `slicelib` computes each section's filler as
   "previous claim's end to this claim's start," so the file must stay in
   **address** order; alphabetical order happens to coincide with address
   order across the `d_a_wm_*` family, so this stayed latent, but it moved
   both `runtime/*` entries to the tail — producing the `-0x10360` `.bss`
   filler crash HANDOFF describes.
2. `ca90c7d` (WM_MANTA "LANDED", Aug 19) never actually promoted
   `include/game/bases/d_a_wm_manta.hpp` out of its authoring sandbox into
   the real `include/` tree — so from that point the file could not compile
   in a fresh checkout, producing the "ambiguous access" MWCC 10319 error
   (a cascade from the failed `#include`) HANDOFF describes. **HANDOFF's
   21:05 entry was an accurate account of the tree at the time it was
   written**, not stale reporting.
3. `45e72fe` (2026-08-21 21:11 — six minutes after HANDOFF's entry) fixed
   **both**: re-sorted `slices/d_basesNP.json` back to address order and
   hardened `slicelib` to refuse an out-of-order file rather than silently
   miscompute a filler; and promoted the real `d_a_wm_manta.hpp` header.
   Evidence in the commit: all four RELs sliced, every already-working
   module's `.o` reported MD5-identical before/after. **Both of HANDOFF's
   two documented problems were genuinely fixed at this point.**
4. `01bdab9` (2026-08-21 23:35) — a commit whose message is entirely about
   unrelated `d_line_mng` linkage work and never mentions `d_basesNP` —
   **silently deleted five `memoryRanges` blocks from `slices/d_basesNP.json`**:
   `d_a_dummy_door.cpp`, `d_a_floor_jr_b.cpp`, `d_a_peach_castle_sequence.cpp`,
   `d_a_wm_manta.cpp`, `d_a_wm_sandpillar.cpp`. Confirmed by
   `git show 01bdab9 -- slices/d_basesNP.json`: pure deletions, no
   replacement, 41 lines removed.
5. `5348871` (newest commit on `HEAD`) restored `d_a_dummy_door` — in split
   `_child.cpp`/`_parent.cpp` form — but **did not restore the other four**.

**Verified directly against the file on disk right now** (not inferred):
`slices/d_basesNP.json` currently has 28 slice entries where the
post-`45e72fe` known-good version had 31; a source-name diff between the two
confirms `d_a_wm_manta.cpp`, `d_a_wm_sandpillar.cpp`, `d_a_floor_jr_b.cpp`
and `d_a_peach_castle_sequence.cpp` are the four still missing. I also
independently re-checked address ordering in every section of the *current*
file (`.text`/`.data`/`.bss`/`.rodata`/`.ctors` all still ascending) — the
`01bdab9` deletions did not reintroduce the ordering bug, so the filler
crash really is fixed; the ordering-bug and the missing-slices problem are
two different bugs from two different commits, and only the second is
currently live.

**Net, corrected: neither of HANDOFF's two documented BUILD problems is the
current blocker — both were fixed by `45e72fe`.** The actual current blocker
is a third, undocumented one: four already-landed, already-verified units
are silently unclaimed in the live slice manifest. **Fix:** restore the four
`memoryRanges` blocks from `45e72fe`'s tree (`git show
45e72fe:slices/d_basesNP.json`) into the current file, preserving address
order, then run the build. This is a pure-metadata restore with no source
risk — the `.cpp`/`.hpp` files themselves are untouched and still compile
clean standalone (confirmed this session) — and it is now ranked #0 below,
ahead of every unit requiring new authoring, because it recovers four
already-verified units' worth of progress in one edit with the least
uncertainty of anything in this document.

---

## D. Live vs. historical directory map

### `d_line_mng` lineage (one TU, nine rounds, oldest → newest by mtime)

| dir | role | verdict |
|---|---|---|
| `author_mov`, `author_geom`, `author_states`, `author_core` | four parallel first-authoring rounds (movement/geometry/state-machine/core), Aug 20 18:59-19:23 | HISTORICAL — superseded by the merge |
| `line_mng_merge` | first merge of the four author rounds, Aug 20 19:39 | HISTORICAL |
| `fix_states`, `fix_bighelper` | two fix-up rounds on the merge, Aug 21 02:32-02:36 | HISTORICAL |
| `line_mng_merge2` | second merge incorporating the fixes, Aug 21 02:56 | HISTORICAL |
| `fix_bigtwo` | final round: `RESULT.md` (03:18) then a full session logged only in `LANDING.md` (23:49) that closed 101→181/182 | **LIVE** — `wip/fix_bigtwo/d_line_mng.cpp` is the current state of the unit |
| `line_mng_shared` | not a round — shared infrastructure: `tally.py` (the scorer every round above calls), `target.txt` (the 182-function reference), `merge_agents.py` | **LIVE, infrastructure** |
| `agent_line_mng` | prerequisite round: class layout + state-framework reconstruction, before any authoring | HISTORICAL, foundational (its `MAPPING.md` is the origin of the 182-function target list) |
| `agent_line_mng_bounds`, `dol_scout` | bounds-derivation-only side studies | HISTORICAL — absorbed into `fix_bigtwo/LANDING.md` section 3a, which re-derived and confirmed the same seven ranges independently |
| `return_type_sweep` | cross-unit sweep, **NOT `d_line_mng`-specific** — it swept 31 `wip/wm_units/agent_*` drafts (1616 target functions) for the CFront-return-type tell, not this unit at all | COMPLETE, genuine negative result: 0 of 31 units need a return-type fix; every 1-3-word delta was traced to either a real correspondence with no r3-write (18 cases) or an unauthored stub filled by the nearest-neighbour matcher (20 cases). Worth keeping as a closed question, not just "absorbed" — it also independently confirms `agent_kinoko_red` is fully LANDED (no target text left to diff), corroborating section D's landed list |
| `vec3pod` | the `mVec3_c` POD-copy-constructor experiment | absorbed into `AGENT_CONTEXT.md`'s "Levers PROVEN NOT to work" list (confirmed: 160-function regression blast radius, do not land) |
| `gapA`, `line_mng_r2`, `line_mng_r3`, `line_mng_r4` | per-function/per-statement lever experiments (dozens of source-shape variants each, e.g. `gapA/a1_namedconst/a_addr_cos.cpp` etc.) | HISTORICAL — their positive findings are already written into `AGENT_CONTEXT.md`'s numbered "MWCC levers" list (11-13 and the register-rule corrections); nothing found in a spot-check of these directories that isn't already reflected there |
| `qwen_verify1` | **not this unit** — see contradiction B3 | out of scope, misfiled by name only |

### `d_enemy_toride_kokoopa` lineage

| dir | mtime | role |
|---|---|---|
| `kokoopa_verify` | Aug 21 19:44-19:57 | round 1 — first full-verify pass |
| `kokoopa_verify2` | Aug 21 20:47-20:54 | round 2 |
| `kokoopa_verify3` | Aug 21 23:21-23:23 | round 3 |
| `kokoopa_verify4` | Aug 21 23:58-00:00 | round 4 |
| `kokoopa_verify5` | Aug 22 03:22-03:23 | **LIVE** — round 5, 180/251 confirmed MEASURED-NOW this session |

`wip/kokoopa_r6` (out of scope, actively worked) is presumably round 6.

### `wm_units` — LANDED (historical), confirmed via `git log -- source/...`

| `wip/wm_units/` sub-dir | landed as | commit |
|---|---|---|
| `agent_antlion` | `d_a_wm_antlion.cpp` | `3894828` |
| `agent_ghost` | `d_a_wm_ghost.cpp` | `dd4955e` |
| `agent_kinoko_1up` | `d_a_wm_kinoko_1up.cpp` | `8183912` |
| `agent_kinoko_base` | `d_a_wm_kinoko_base.cpp` | `27ec194` |
| `agent_kinoko_red` | `d_a_wm_kinoko_red.cpp` | `8b385ab` |
| `agent_kinoko_star` | `d_a_wm_kinoko_star.cpp` | `5e3bc0d` |
| `agent_manta` | `d_a_wm_manta.cpp` | `ca90c7d` |
| `agent_note` | `d_a_wm_note.cpp` | `adf9cdf` |
| `agent_sandpillar` | `d_a_wm_sandpillar.cpp` | `2cad2bb` |
| `agent_sinkship` | `d_a_wm_sinkship.cpp` | `9e151f9` |
| `agent_smallcloud` | `d_a_wm_smallcloud.cpp` | `17238cf` |
| `agent_peach_castle_seq` | `d_a_peach_castle_sequence.cpp` (name changed) | `2b7ee05` |
| `agent_ac_switch` | `d_a_ac_switch.cpp` | `435e104` |
| `agent_branch` | `d_a_branch.cpp` | `16b8f7f` |
| `agent_dummy_door` | `d_a_dummy_door_child.cpp` + `_parent.cpp` (split in two) | `0cf2553` |
| `agent_floor_jr_b` | `d_a_floor_jr_b.cpp` | `eeae741` |
| top-level `wip/wm_kinoko_1up`, `wip/wm_smallcloud` | same two files above, earlier/duplicate authoring attempts | superseded by the `wm_units` versions that actually landed |
| `fix`, `fix_tower`, `off` (wm_units top level) | grid/tower/smallcloud/kinoko probe scratch | all landed already (`d_a_wm_grid.cpp`, `d_a_wm_tower.cpp`) |
| `agent_wall`, `agent_kinoko`, `agent_pool_order`, `agent_poolwall`, `agent_tempwall` | **not real units** | see contradiction B4 |

**Independent re-verification this session (not just `git log`):** for
`agent_branch` and `agent_dummy_door`, I compiled both drafts fresh into
`scratch/parked_audit/wm_units/` and re-ran `verify_anon.py` and
`check_vtable.py` against the real target objects — `agent_branch` 3/3
MATCH, vtable CLEAN (51/51 slots); `agent_dummy_door` 4/4 MATCH, both
classes' vtables CLEAN (PARENT's required isolating its `.obj` block into a
standalone snippet first, because `check_vtable.py` greps the FIRST `__vt__`
symbol in a file and silently reused CHILD's when checking PARENT directly
— a real, previously-undocumented tool limitation, not a defect in the
draft; `agent_dummy_door/MAPPING.md` already flags a version of this same
limitation). Both fully corroborate the `git log` landed-status finding at
the instruction level, not just the file-existence level.

**Recommendation:** the 16 landed-draft directories plus `fix`/`fix_tower`/`off`
and the two top-level duplicates (19 directories total) are safe to archive —
their content is fully superseded by `source/`. Do not delete; the git history
inside them (e.g. probe variants) has occasionally been cited later as
evidence (`AGENT_CONTEXT.md` cites `d_a_wm_cloud.cpp`/`grid` compiles from
exactly this kind of directory repeatedly).

### `wm_units` — genuinely OPEN (still parked or in progress)

`agent_river`, `agent_koopajr`, `agent_kinopio`, `agent_castle_bg`,
`agent_castle`, `agent_koopa_castle`, `agent_floor_jr_a`, `agent_water_move`,
`agent_antlion_mng`, `agent_hanachan`, `agent_anchor`, `agent_board`,
`agent_course`, `agent_gun_battery`, `agent_item`, `agent_killer`,
`agent_killerbullet`, `agent_kinoballoon`, `agent_nice_coin`, `agent_start`,
`agent_dance_pakkun`, `agent_lemmy` — 21 units, all scored in section A or the
appendix list below it.

### Other standalone units

- **`m_pad`** — one function (`clearWPADInfo`) blocking 12/14; no `.cpp`
  survives at the top level, only `scratch/` sub-rounds (batch1-3,
  lead_clear, sibling, merge_lead); `wip/m_pad/scratch/merge_lead/m_pad.cpp`
  is the most-merged/most-recent candidate. Untouched since 2026-08-14
  16:33.
- **`nand_thread`** — similarly scratch-only, most recent is
  `wip/nand_thread/scratch/merge_lead/d_nand_thread.cpp` (or `closer_d`,
  same mtime cluster). Untouched since 2026-08-14 12:40.
- **`player_manager`** — `wip/player_manager/assembled.cpp` is the live,
  trial-linked draft (see section C). Untouched since 2026-08-14 12:20.

None of these three has been touched since mid-August; all three are
genuinely parked, not abandoned mid-edit.

---

## What I could not settle

- **`agent_water_move`'s and `agent_nice_coin`'s current fraction.** Both have
  HANDOFF entries suggesting progress past their last clean N/M count
  (water_move's `execute()` specifically reached N/N; nice_coin's `__sinit`
  matched byte-exact, which the record treats as strong indirect evidence for
  the surrounding declarations) but I did not re-run their scorers this
  session — flagged in section A rather than given a number I did not
  measure.
- **The BUILD blocker is now fully diagnosed, not just partially** (see
  section C's corrected account): both problems `HANDOFF.md` documented
  (manta's missing header, the alphabetical-resort `.bss` filler crash) were
  genuinely fixed by commit `45e72fe`, six minutes after HANDOFF's entry.
  What actually still gates `d_basesNP` today is a third, later regression —
  commit `01bdab9` silently deleted four unrelated units' slice claims — and
  the fix is a known, specific JSON restore (rank 0 in section A), not an
  open question. The one thing I still could not settle without running
  `ninja`: whether restoring those four blocks alone is *sufficient* to make
  the build succeed end-to-end, or whether something else has drifted in the
  ~20 hours since `45e72fe`'s own verification. The evidence (address
  ordering still intact, source still compiles clean standalone) is
  consistent with "sufficient," but only an actual build attempt confirms it.
- **`d_bg_actor_mng`'s whole-unit score** (see B3) — out of this audit's
  enumerated scope, flagged only as a misfiling risk for `qwen_verify1`.
