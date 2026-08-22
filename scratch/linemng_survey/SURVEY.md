# `d_line_mng.cpp` status survey — 2026-08-22

## Headline (measured just now, not taken from any doc)

```
python wip/line_mng_shared/tally.py wip/fix_bigtwo/d_line_mng.cpp wip/fix_bigtwo/shadow_include
(7 paired by CONTENT -- unnamed target vs mangled draft name)
matched 181/182 functions   7531/7631 words = 98.7% BY BYTES
```

**Only ONE function is not yet byte-matched: `line_cross_chk2` (100 words, LEN OK).**
Full raw tool output is in `scratch/linemng_survey/tally_now.txt`.

### Both stale figures in the repo are wrong, and wrong in opposite directions

| Source | Claims | Actual |
|---|---|---|
| `wip/fix_bigtwo/tally_final.txt` | 101/182, 27.8% | **181/182, 98.7%** |
| `HANDOFF.md` (newest `d_line_mng` session) | 165/182, 65.9% | **181/182, 98.7%** |

`tally_final.txt` is simply an old snapshot (from the round-0 baseline check in
`wip/fix_bigtwo/RESULT.md`, section 0) that was never refreshed after later
rounds landed real fixes.

`HANDOFF.md` is more interesting: it is not stale by accident, it is stale by
**omission**. Its own git history (`git log --oneline -- wip/fix_bigtwo/d_line_mng.cpp`)
shows five commits *after* the 165/182 session that each moved the unit
further and were never appended to `HANDOFF.md`:

```
c26f683  169/182  76.0%   fn_800C31C0 closes, 221 diffs -> 0
f02048e  174/182  95.5%   move_on_circle family closes
f803613  175/182  97.1%   line_cross_chk1 closes; chk2 bounds the rule
b3e2a80  181/182  98.7%   all six small functions close; one left
ebc32ca  181/182  98.7%   poolcheck.py closes the wrong-constant hole (count unchanged,
                          but a false positive in `start_line_move` was caught and fixed)
```

`HANDOFF.md`'s narrative simply stops after the 65.9% session (its last
`d_line_mng` entry ends with "THE BUILD IS BROKEN" and a build-repair TODO);
whoever ran the five commits above did not go back and write a new `HANDOFF.md`
entry. **Anyone briefed off `HANDOFF.md` alone right now would be told the unit
is a third done. It is at 98.7%, with one 100-word function left.**

## 1. Which draft is live

`wip/fix_bigtwo/d_line_mng.cpp` is the sole live draft. Evidence:

- **Modification time**: `wip/fix_bigtwo/d_line_mng.cpp` — Aug 22 00:09 (repo
  clock), 100,212 bytes. This is the newest `d_line_mng.cpp` on disk by a wide
  margin.
- **All ten `d_line_mng`-touching commits in `git log`** operate on this exact
  path; there is no commit history at all for the sibling drafts below.
- The current working tree has **no uncommitted diff** against `d_line_mng.cpp`
  itself (only `wip/fix_bigtwo/_tally/d.o` / `d.txt` — regenerated tally
  artifacts — show as modified in `git status`), so what's on disk is exactly
  what's committed at `ebc32ca`.

Other candidates, checked and confirmed **not** live (all frozen, all older):

| Draft | mtime | Tally (as last measured) | Relationship to fix_bigtwo |
|---|---|---|---|
| `wip/fix_bigtwo/d_line_mng.cpp` | Aug 22 00:09 | **181/182, 98.7%** | **live** |
| `wip/line_mng_merge2/d_line_mng.cpp` | Aug 21 02:52 | 101/182, 27.8% | direct parent — `fix_bigtwo` was forked from this file unmodified (per `RESULT.md` §0) and then carried forward through 8 more rounds that were never applied back here |
| `wip/fix_states/d_line_mng.cpp` | Aug 21 02:29 | 100/182, 27.7% | sibling fork off the same 100/182 baseline, assigned a disjoint task (states), frozen at that baseline |
| `wip/fix_bighelper/d_line_mng.cpp` | Aug 21 02:34 | 100/182, 27.7% | sibling fork off the same 100/182 baseline, assigned a disjoint task (helpers), frozen at that baseline |
| `wip/line_mng_merge/d_line_mng.cpp` | Aug 20 19:33 | 100/182, 27.7% | earlier merge, superseded by `line_mng_merge2` |
| `wip/agent_line_mng/` | Aug 20 18:35 | (67/182 by count, ~6.9% by bytes, per HANDOFF's first-declaration-round note) | earliest authoring round, only `work/named/` subtree, superseded |
| `wip/agent_line_mng_bounds/` | Aug 20 18:37 | no `.cpp` — bounds analysis only (`BOUNDS.md`) | not a draft |
| `wip/line_mng_r2/`, `_r3/`, `_r4/` | Aug 21 20–22 | no top-level `d_line_mng.cpp` — each is a set of per-experiment subdirectories (`crosschk/`, `moveon/`, `bigfn/`, `fpperm/`, etc.), i.e. scratch rigs for individual levers, not full drafts | experiment scaffolding, feeds into `fix_bigtwo` |
| `wip/line_mng_shared/_base.cpp`, `base_d_line_mng.cpp` | Aug 21 22:18 / Aug 20 18:38 | not independently tallied | shared scaffolding/target file staging area (also holds `target.txt`, `tally.py`), not a competing draft |
| `wip/gapA/*.cpp` | Aug 21 18:0x–18:2x | not full-unit tallies (per-function experiment variants) | Gap A/B lever-discovery rig, already folded into `fix_bigtwo` |

No collision risk: exactly one draft has moved since the 100/182 fork point.

## 2. The one remaining function

```
100w   LEN OK   line_cross_chk2__10dLineMng_cFfRC7mVec2_c7mVec2_c7mVec2_cRf
```

Length-exact (100/100 words), **46 of 100 words currently differ** by raw text
(measured with `wip/fix_bigtwo/chk2_diff.py`). This is *worse* than the 27/100
recorded in commit `f803613`'s message — expected, because `chk2_diff.py`
compares raw instruction text, and several later, unrelated fixes elsewhere in
the file shifted this function's pooled-constant/label numbering without
touching its own source.

Root cause, as diagnosed and left in `f803613`'s commit message (still the
current state — this function has not been touched since):

- The **draft hoists the first `0.0f` pool load to instruction slot 8**, right
  after the callee-saved-register spills in the prologue.
- **Retail defers that same load to slot 19**, scheduling it into the register
  freed once `p4.x`'s raw value is consumed mid-subtraction.
- All 46 differing words are downstream of that one scheduling decision —
  register *numbers* differ throughout (`f1`↔`f0`↔`f2` renumbering, branch
  labels), but register *contents* and control flow are correct on both sides.
- **This is a documented, bounded negative**: ~35 measured variants (per
  `f803613`) established that this lever governs which register/slot a value
  gets, not *when* it is scheduled. Two probes pinned the trigger precisely — a
  `0.0f` comparison anywhere before the function's first `bl` reproduces the
  early hoist; the same comparison placed after the call does not hoist. Tied
  to the two-call prologue shape (forcing `f30`/`f31` spills), not to
  occurrence count or textual position of the comparison in source.
- No successful lever has been found for this specific residual. It is parked,
  not being actively worked, per `LANDING.md`: *"the unit is landable as-is;
  landing does not require closing it."*

## 3. Family breakdown — now moot, kept for the record

The brief for this survey anticipated ranking families by word count to guide
where to spend effort. That exercise no longer applies: **there is exactly one
family with open work, containing exactly one function.**

| Family | Members remaining | Words remaining | Share of unit (7631w) |
|---|---:|---:|---:|
| `*_cross_chk` (line_cross_chk2 only) | 1 of ~30 in the family | 100 | 1.3% |
| `move_on_circle*` | 0 | 0 | closed (`f02048e`) |
| `executeState_*` / `initializeState_*` / `finalizeState_*` | 0 | 0 | closed |
| `__sinit_\d_line_mng_cpp` | 0 (see §5 — matched in isolation, landing-gated) | 0 | closed as a source question |
| `fn_800C31C0` | 0 | 0 | closed (`c26f683`) |
| `CalcAdjustPosY` | 0 | 0 | closed (in the 165/182 session per `HANDOFF.md`) |

Everything named in the original brief's example family list is closed except
the single `line_cross_chk2`.

## 4. Function DEFINITION ORDER — WRONG, in two places

Ran a from-scratch order check (`scratch/linemng_survey/order_check.py`,
output in `scratch/linemng_survey/order_full.txt`) because `tally.py` itself
does **not** check order (confirmed by reading it — no such logic exists) and
`wip/wm_units/verify_anon.py`'s order check is REL/anonymous-symbol specific
and doesn't apply here. The check pairs every target `.fn` (in target address
order) to its draft counterpart (same name/mangled-suffix/content-fallback
logic `tally.py` uses) and reads the draft's own compiled object-order index
for each.

**Result: 14 of 182 paired functions are out of order**, in exactly two
contiguous blocks. Both blocks are template-instantiated framework methods
(`sFStateStateMgr_c<dLineMng_c,...>` and friends) — confirmed **not** weak
(`bin/dtk/wiimj2d_symbols.txt` shows no `scope:weak` tag on any
`<10dLineMng_c,...>` instantiation, unlike the `<10daPlBase_c,...>` sibling
instantiations of the same templates, which *are* tagged weak and hosted
elsewhere). So this TU is the sole owner of these functions' code, and their
relative order is real `.text` placement, not a link-time no-op.

### Block 1 — six destructors right after the constructor, emitted in EXACTLY REVERSED order

Target order (what the source needs to produce):
```
1. __dt__77sFStateStateMgr_c<10dLineMng_c,20sStateMethodUsr_FI_c,20sStateMethodUsr_FI_c>Fv
2. __dt__91sStateStateMgr_c<10dLineMng_c,12sFStateMgr_c,20sStateMethodUsr_FI_c,20sStateMethodUsr_FI_c>Fv
3. __dt__49sFStateMgr_c<10dLineMng_c,20sStateMethodUsr_FI_c>Fv
4. __dt__79sStateMgr_c<10dLineMng_c,20sStateMethodUsr_FI_c,12sFStateFct_c,13sStateIDChk_c>Fv
5. __dt__26sFStateFct_c<10dLineMng_c>Fv
6. __dt__23sFState_c<10dLineMng_c>Fv
```
Draft's current object order for the same six: **6, 5, 4, 3, 2, 1** — the
exact reverse.

### Block 2 — 13 framework accessor/lifecycle methods, target INTERLEAVES by class, draft GROUPS by class

Target order:
```
 1. initializeState__91sStateStateMgr_c<...>Fv
 2. finalizeState__91sStateStateMgr_c<...>Fv
 3. isSubState__91sStateStateMgr_c<...>CFv
 4. returnState__91sStateStateMgr_c<...>Fv
 5. getOldStateID__91sStateStateMgr_c<...>CFv
 6. build__26sFStateFct_c<...>FRC12sStateIDIf_c
 7. dispose__26sFStateFct_c<...>FRP10sStateIf_c
 8. initialize__23sFState_c<...>Fv
 9. execute__23sFState_c<...>Fv
10. finalize__23sFState_c<...>Fv
11. refreshState__91sStateStateMgr_c<...>Fv
12. changeToSubState__91sStateStateMgr_c<...>FRC1...
13. getState__91sStateStateMgr_c<...>CFv
```
Draft's current object order: `build, dispose, initialize(FState), execute(FState),
finalize(FState), initializeState(SSMgr), finalizeState(SSMgr),
changeToSubState(SSMgr), returnState(SSMgr), getOldStateID(SSMgr),
refreshState(SSMgr), isSubState(SSMgr), getState(SSMgr)` — i.e. the draft
currently groups all methods of one template class together, where target
**interleaves by address** across the two classes. This is exactly the pattern
AGENT_CONTEXT already names ("interleave by ADDRESS, not by logical grouping",
found on `d_a_wm_sandpillar.cpp`).

**Caveat, stated plainly**: these are compiler-synthesized instantiations of
methods declared in a shared framework header (`s_State.hpp`), not hand-written
function bodies in `d_line_mng.cpp` itself. Their emission order is presumably
driven by first-ODR-use order (which member is referenced first, from where in
`dLineMng_c`'s own code — the constructor, `changeState`, etc.), not by a
literal reordering of definitions in the `.cpp`. Nobody has investigated what
source-level change would fix this order; that is unstarted work, flagged here
per the brief's instruction not to attempt fixes.

**Every other function in the unit (168 of 182 paired) is in correct target
order.** All 46 `*_cross_chk`/`line*_cross_chk`/`circle*_cross_chk` predicates,
all `executeState_*`/`initializeState_*`/`finalizeState_*` triples, all
`mov_to_*`/`mov_frm_*`, `move_on_circle*`, and the free functions
(`fn_800C31C0`, `fn_800C1EE0`, `fn_800C3B20`, `fn_800C3B60`) are in the right
relative order.

This has not been checked against a real link (per the hard rules, no build
was run), so whether it actually breaks `--verify-bin` is unconfirmed — but per
AGENT_CONTEXT's own standing rule ("function order alone can stop a unit
linking even at 100% per-function match"), it is a real, unresolved risk that
should be closed before landing is attempted, not after.

## 5. `__sinit_\d_line_mng_cpp` — status, and the header-change claim is UNSUPPORTED

**Current measured state is much better than the old written record: length-exact
at 1193/1193 words, and only 4 of 1193 words differ in raw disassembly TEXT**
(re-measured just now with `wip/fix_bigtwo/chk2_diff.py __sinit_`). All four are
the already-documented tooling artifact, not real content: retail's
disassembly quotes a resolved symbol name (`"@55792_80316CA0"`,
`"@49614_80359100"`) for two `.data`/`.bss`-relative address loads, where a
standalone, unlinked `.o` necessarily shows the unresolved form
(`...data.0`, `...bss.0`) — the underlying relocation and its zeroed immediate
field are byte-identical on both sides, only the disassembler's rendering of
the (still-unresolved) symbol differs. This is the exact defect class
AGENT_CONTEXT/HANDOFF already name for `mov_to_*`, and it's why `tally.py`
gates on raw BYTES first, not text: `__sinit` is genuinely byte-identical and
correctly counts as matched — it does not appear anywhere in
`tally_now.txt`'s remaining-function list.

**This directly supersedes `wip/fix_bigtwo/RESULT.md` §2's finding of a
uniform `+0x40` displacement shift across 175/1193 words**, attributed there
to weak-vtable link-time deduplication. That measurement is from the very
first `fix_bigtwo` round, at the 101/182 baseline. Whatever combination of the
eight subsequent rounds' fixes did it, **the `+0x40` residual is gone in the
current draft** — re-measured directly, not inferred. Either the `.data` pool
composition changed enough (e.g. once all 25 states were fully declared) that
the local vtable set the compiler emits no longer includes the redundant weak
copies, or the earlier reading was itself imprecise. Worth flagging as a
finding in its own right: **the RESULT.md file describing this unit's
`__sinit` is now materially wrong about the size of the residual**, even
though its root-cause reasoning (weak-symbol dedup, link-time, unfixable from
this TU) may still be correct for whatever 4-word-class of difference remains
— except the 4 remaining words are not that; they are the symbol-quoting
tooling artifact, a different and already-solved problem.

15.6% of the unit's word count (1193/7631), as stated in the brief.

### HANDOFF.md's claim, checked directly

HANDOFF.md's newest `d_line_mng` session says verbatim:

> **`__sinit` (1193w, 15.6% of the unit) is SOLVED but NOT LANDED.** It is
> gated on a shared-header change and on the build, which is broken -- see
> below.

I searched the entirety of `HANDOFF.md` (all `line_mng` occurrences, full text
read) and **found no description anywhere of what that shared-header change
actually is.** The "see below" points at a section titled "THE BUILD IS
BROKEN", which describes two *unrelated* build failures
(`d_a_wm_manta.cpp` ambiguous-name compile error, and a `slice_rel.py`
`.bss`-filler-size bug in `d_basesNP`) — neither mentions `d_line_mng`,
`smc_UNIT_SIZE_X`, or any header at all. **The "gated on a shared-header
change" claim is not backed by any text in HANDOFF.md.** This is the
contradiction the brief asked to be flagged, not reconciled.

### What `wip/fix_bigtwo/LANDING.md` (Aug 21 23:49 — the newest file in the
directory, postdating the HANDOFF session by hours) actually says the gate is

> **The header story is clean.** Diffing the whole shadow tree against
> `include/` reports exactly one entry, `NEW game/bases/d_line_mng.hpp`. No
> shared header is modified, so AGENT_CONTEXT rule 2 ("verify a
> shared-header change alone") does not apply to this landing.
>
> **What actually blocked it:** `dLineMng_c::smc_UNIT_SIZE_X` is declared but
> never defined, and adding the definition changes codegen. See AGENT_CONTEXT,
> "the static-const-float trap". That is the only remaining blocker, and it
> is a source question, not a slice question.

Verified directly against the live draft: `smc_UNIT_SIZE_X` is declared
`static const float smc_UNIT_SIZE_X;` in
`wip/fix_bigtwo/shadow_include/game/bases/d_line_mng.hpp:148` and has **no
definition anywhere** in `wip/fix_bigtwo/d_line_mng.cpp` (grepped for
`smc_UNIT_SIZE_X\s*=`, zero hits). This matches AGENT_CONTEXT's own recorded
rule: *"a file-scope `const` with a constant initialiser gets folded away
entirely by `-O4`... three constants had to be made non-`const` to exist at
all"* — i.e. defining it risks changing codegen for every one of the ~9 call
sites that already reference it byte-exactly today, which is presumably why
nobody has just added the definition yet.

**So: HANDOFF.md's "shared header change" gate and LANDING.md's
"`smc_UNIT_SIZE_X` definition" gate are two different claims, and only the
second is backed by evidence in the tree.** Treat HANDOFF.md's framing as
stale/unsupported on this specific point.

### The old `+0x40` link-time story (for context only — no longer the live residual)

`wip/fix_bigtwo/RESULT.md` §2, from the very first `fix_bigtwo` round at the
101/182 baseline, traced a then-measured `+0x40`/175-word residual to
**weak-symbol linker deduplication**: `d_line_mng.cpp`'s own state manager
pulls in 12 framework vtables from `s_State.hpp`, six of which are plain
(non-templated) interface vtables shared project-wide and physically hosted by
some other, earlier-linked TU (confirmed via `scope:weak` tags in
`bin/dtk/wiimj2d_symbols.txt`, addressed at `0x802FEDC8`-`0x802FEE48`, nowhere
near this unit's own `0x80316xxx`-`0x80317xxx` `.data` slice). As measured in
§5 above, **that residual is gone in the current draft** — re-measured just
now at 4/1193, and those 4 are the unrelated symbol-quoting tooling artifact.
Whether the weak-vtable mechanism is still silently present but now
coincidentally netting to zero displacement, or whether it stopped applying
once the class layout settled, is unknown and not worth chasing — the
practical answer is that `__sinit` needs no further source work.

### Net: what actually still gates landing the unit

1. `smc_UNIT_SIZE_X` needs an actual definition, unproven not to perturb the
   ~9 existing byte-exact call sites that reference it today (source
   question, addressable, not attempted here per instructions). This is now
   the ONLY documented source-level blocker for the whole unit.
2. `line_cross_chk2`'s scheduling residual (§2) — a bounded, currently-unsolved
   negative; landing does not require closing it.
3. The function-order defect in §4 — unconfirmed against a real link, per the
   hard rules, but a real risk per AGENT_CONTEXT's own standing rule that
   order alone can break a 100%-per-function-matched unit.
4. The unit has never been run through a real `ninja` + `--verify-bin` link at
   all in this state, so none of the above is confirmed against the actual
   linker — only against isolated per-function compiles.

## Files produced by this survey

- `scratch/linemng_survey/tally_now.txt` — full raw `tally.py` output (the
  authoritative current headline).
- `scratch/linemng_survey/order_check.py` / `order_full.txt` — the
  from-scratch function-order audit (§4).
- `scratch/linemng_survey/target_order.txt` — target `.fn` order, extracted
  standalone for reference.
- `scratch/linemng_survey/handoff_tail.txt` — the full `d_line_mng`-relevant
  tail of `HANDOFF.md` (offset 706000 to EOF), extracted because `grep`
  reports the file as binary (CRLF + some non-ASCII byte trips its heuristic)
  and Read's line-based access doesn't let you jump straight to a byte offset.

No file under `wip/` was modified. This report and its scratch outputs are the
only things written.
