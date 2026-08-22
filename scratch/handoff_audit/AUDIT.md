# HANDOFF.md / AGENT_CONTEXT.md superseded-claim audit — 2026-08-22

Read-only audit. Nothing outside `scratch/handoff_audit/` was written.
Compiles were done via `harness.compile_draft(..., module='d_basesNP')` into this
directory.

## THE ROOT CAUSE, measured

`HANDOFF.md` was last committed at **e4da0d6, 2026-08-21 21:05**.
`HEAD` is **e9ae5dd, 2026-08-22 17:54**. **17 commits in between, none of them
appended to HANDOFF.** So HANDOFF's *newest* section — the one a brief-writer
reaches for — is a whole session out of date, and it is the newest sections that
carry the status claims.

`HANDOFF_INDEX.md` was regenerated in the same commit, so the index is exactly as
stale as the file, and the index is what agents grep.

---

## (A) CORRECTION LIST

Severity: **C** = a brief built on this is wrong before it starts;
**H** = will cost a round; **M** = misleading; **L** = tidy-up.

| # | Line range | Claim as written | What is actually true | Evidence | Sev |
|---|---|---|---|---|---|
| 1 | `HANDOFF 14508-14525` (+ index line 613) | "THE BUILD IS BROKEN -- and it predates this session… **Nothing can land until both are fixed**". Cause 1: `d_a_wm_manta.cpp` no longer compiles, MWCC 10319 ambiguous access. Cause 2: `slice_rel.py` emits a `.bss` filler of `-0x10360`. | Both fixed. The 10319 was a **cascade from a missing header**, not a name collision — `include/game/bases/d_a_wm_manta.hpp` had never existed in any commit. The `-0x10360` was `slices/d_basesNP.json` having been hand-resorted **alphabetically**, breaking slicelib's ascending-address requirement. Tree is 5/5 green. | Commit `45e72fe` (both fixes, with the reproduction in a detached worktree at `ca90c7d`); commit `6dbeaf3` "Tree is GREEN: all five binaries verify for the first time in ~10 days". I recompiled `source/d_basesNP/bases/d_a_wm_manta.cpp` at HEAD via `compile_draft`: **OK**. `include/game/bases/d_a_wm_manta.hpp` now exists. `tools/slicelib.py` now *refuses* an out-of-order slice file. | **C** |
| 2 | `HANDOFF 14438-14447` (+ index line 608) | `d_line_mng` "matched 165/182, 5031/7631 words = 65.9% BY BYTES". Also `wip/fix_bigtwo/tally_final.txt` says 101/182, 27.8%. | **181/182, 7531/7631 words = 98.7%.** One function open: `line_cross_chk2` (100w). | Measured today at 17:54 by a concurrent agent and left in `scratch/linemng_survey/tally_now.txt` + `SURVEY.md`. Corroborated by the commit chain `c26f683`(76.0%) → `f02048e`(95.5%) → `f803613`(97.1%) → `b3e2a80`(98.7%) → `ebc32ca`(98.7%, one false positive fixed), none of which was appended to HANDOFF. `tally_final.txt` is the round-0 baseline snapshot from `wip/fix_bigtwo/RESULT.md` §0, never refreshed. | **C** |
| 3 | `HANDOFF 7099-7106`, `8092-8096`, `11678-11687`, `12000-12012` (+ index lines 219, 287, 472, 489) | "SANDPILLAR LANDED"; "WM_MANTA LANDED — 16/16, Thirteenth unit"; "LANDED: `d_a_peach_castle_sequence.cpp`"; "LANDED: `d_a_floor_jr_b.cpp`". | **All four were SHELVED** — slice entries removed, sources retained — costing 11,548 bytes. `d_basesNP` reads 1.941% where it would read 2.562%. HANDOFF records this **nowhere**: `grep -ai "shelv" HANDOFF.md` returns zero hits. | `slices/d_basesNP.json` contains none of the four sources (28 entries, listed). All four `.cpp` still exist under `source/` and all four compile clean (I compiled each into `scratch/handoff_audit/`). Reasons per unit are in commit `6dbeaf3`'s message: sandpillar = five weak vtables placed that retail dedups; peach_castle_sequence = 55% of `.text` differs; manta = `.text` 0xF0 short + two `.rodata` ordering defects; floor_jr_b = `daFloorJrA_c` shadow declares virtuals in the wrong order. | **C** |
| 4 | `HANDOFF 8509-8526` (+ index line 312) | "Standing state of the WM units — LANDED (13): antlion, **sandpillar**, kinoko_star, sinkship, note, **manta**, + cannon, cloud, …" | Two of the named units are shelved. The block is also internally inconsistent: it says "(13)" and lists **19** names. | Same as #3. | **C** |
| 5 | `HANDOFF 11553-11565` (+ index line 464) | "LANDED: `d_a_dummy_door.cpp`" | That file no longer exists. It was split into `d_a_dummy_door_child.cpp` and `d_a_dummy_door_parent.cpp` (commit `5348871`), which is what the slice file holds. | `slices/d_basesNP.json`; `ls source/d_basesNP/bases/`. Note `bin/compiled/.../d_a_dummy_door.o` is still on disk as a stale artefact. | M |
| 6 | `HANDOFF 2725-2763`, esp. **2729**, 2733-2734, 2741; also 2630. **And `HANDOFF_INDEX.md` line 87, whose title *is* the rule.** | "**A slice's `.rodata` claim end must be 8-byte aligned**, or the module comes out 8 bytes wrong through quantisation." Presented as three facts "each measured". | Deleted at `HANDOFF 7068-7073`: "It is not a fact about the compiler or the game. Section claims may now end wherever the unit's real content ends." It was a description of `make_filler_slice` giving every filler its section's *nominal* alignment. | `tools/slicelib.py:92-114` `natural_alignment()` + line 140, applied to fillers. Commit `3894828`. The earlier text carries no marker and the index title asserts the retired rule verbatim. | **H** |
| 6b | `HANDOFF 6216` "Gotcha for `nonMatching` slices" | "Both ends of a `nonMatching` range must be **8-aligned** (16 for `.text`)" | **Probably still true and should NOT be deleted with #6.** `natural_alignment()` is applied to *filler* slices only (`slicelib.py:140`); real and `nonMatching` slices still take `section_info.align` (`slicelib.py:171`). The *end* half may now be relaxed by the filler fix. **I did not measure this** — flagging it so #6's cleanup does not over-reach. | Source read only, unverified by build. | M |
| 7 | `HANDOFF 2811-2875` (+ index lines 91, 92) | "antlion is 37/37 … **0xc of `.rodata` from landing**"; "**the object genuinely has to emit those `0xc` bytes**"; "Antlion is reverted." | Antlion **landed** (`HANDOFF 7029-7034`) with no source change to that pool. The `0xc` was our own filler alignment. The section's own conclusion — "the size is not a bounds question" — is exactly backwards. | `slices/d_basesNP.json` contains `d_a_wm_antlion.cpp`. `HANDOFF 7035-7067`. This is the section that produced one of the three stale briefs. | **H** |
| 7b | `HANDOFF 2298-2332`; `2589-2627`; `2628-2674`; `6767-6842` (+ index lines 73, 83, 84, 204, 205) | "antlion's `.rodata` gap: ownership **SETTLED**, and it does not dissolve"; "antlion: PARKED one byte short"; "antlion is ONE BYTE from landing"; "**Ownership is open again**". | 2298 was retracted at 6767 ("I closed it with an invalid test"); 6767 was then settled at **7074-7094** by reading the target's own split object `auto_fn_2_15B4E0_text.o`. All three park/blocker states are discharged. Four consecutive index titles say otherwise. | `HANDOFF 7074-7094`; slice file. | M |
| 8 | `HANDOFF 1861-1884` (+ index line 67) | "`syms.txt` holds DOL addresses ONLY — **REL-internal calls are a hard blocker**… there is **no mechanism for a REL-internal symbol**, so a unit that calls an un-decompiled function in its own module **cannot land**." | False. There is a mechanism and it is in daily use: the `R_<module>_<section>_<offset>` convention plus `alias_db.txt` (**781 entries**). `HANDOFF 12013-12025` states the general rule outright: "A slice does not need to own, or even to have decompiled, the definitions it references — only the bytes it claims." Both units this section names as held have moved on: `kinoko_1up` **landed** (`3148-3183`), and `course` is parked for unrelated reasons (`6627-6713`). | `alias_db.txt` (781 lines, `R_2_1_* = <mangled>`); `HANDOFF 9459-9468`, `9617-9643`, `12013-12025`; `slices/d_basesNP.json`. Commit `6dbeaf3` notes an `alias_db` entry "rescues `d_a_ac_switch` outright". | **H** |
| 9 | `HANDOFF 1348-1384` (+ index line 59) | "`d_a_wm_kinoko_1up.cpp` is COMPLETE and **NOT LANDABLE** — it depends on an un-decompiled TU" | Landed. Still in the slice file. | `HANDOFF 3148-3183`; `slices/d_basesNP.json`. Also asserted twice in `AGENT_CONTEXT.md` — see (B). | **H** |
| 10 | `HANDOFF 9510-9532` (+ index lines 366, 367) | "WM_KILLERBULLET … a **LANDING-ORDER DEPENDENCY** on WM_KILLER" | HANDOFF itself says at **12021-12023**: "The recorded WM_KILLERBULLET-on-WM_KILLER precedent should be re-examined on the same basis rather than cited as established." Three sibling "landing-order" objections (peach_castle_sequence, ac_switch, floor_jr_b) were each retired by one build. Not yet re-examined; the 9510 text carries no marker. | `HANDOFF 12013-12044`. | M |
| 11 | `HANDOFF 3011-3062` (+ index line 96) | "sandpillar's real blocker, finally identified: WM_ANTLION owns its state code" / earlier "PARKED at 61/66, correctly blocked on WM_MAP" | Corrected at `3588-3616`: "The count is right and the framing is wrong… Blocked-on-WM_MAP was inferred from one investigation and then written down as the unit's status; it never followed." | In-file correction exists and is titled CORRECTION; only the index title is the landmine. | L-M |
| 12 | `HANDOFF 793-813` (+ index line 43) | "Next target — and what is known about the rest. **This is the only 'next target' section for game code.**" | No longer the only one, and no longer current. `13264-13332` "THE LANE DECISION: measured throughput says DOL game code, not `d_basesNP` micro-units" is the live planning statement, and `13383` opens `d_line_mng.cpp` as the first DOL target. | `HANDOFF 13264-13332`, `13383-13422`. | M |
| 13 | `HANDOFF 14388-14397` | "A REAL INFRASTRUCTURE BUG found on the way, **still open** — `harness.canonicalise` … does not strip the surrounding QUOTES … should be fixed centrally before anyone trusts a canonical-only verdict." | **Already fixed when this was written.** Commit `10dff97` (2026-08-21 **19:22**) fixed all three causes — quotes, `...sbss.N`, and mangled `bl` targets. The HANDOFF entry is in `e4da0d6` (21:05), **1h43m later**. Stale on arrival, in the newest section of the file. | `tools/auto_decomp/harness.py:81-84` — `POOL_SYM` now `r'"?@\d+(?:_[0-9A-Fa-f]{8})?"?'` and `r'"?\.\.\.(?:data\|rodata\|s?bss\|sdata2?)\.\d+"?'`. `git log -1 --format=%ci 10dff97` vs `e4da0d6`. | **H** |
| 14 | `HANDOFF 14393-14396`; and the **title** of `13591` + **index line 565** | "The **byte gate is the honest one** and `tally.py` uses it, **so no result in this file is affected**." / section title "TOOLING DEFECT: `harness.canonicalise` reports FALSE MISMATCHES — **use raw BYTES as the gate**". | The byte gate is **blind to a wrong constant**: `lfs`/`lfd`/`bl` have their address and pool-offset fields zeroed on both sides. `poolcheck.py` (commit `ebc32ca`) found two live false positives **in `d_line_mng` itself** — `start_line_move` had `-0.1` where retail's pool holds `BFB99999A0000000`, i.e. `(double)(-0.1f)` — which "181/182 had been counting … for days". So results in this file *were* affected. The correct gate is the union **plus** `poolcheck.py`. (The 13591 *body* says the union correctly; only its title and the index entry say "raw BYTES".) | Commit `ebc32ca`; `tools/auto_decomp/poolcheck.py`; `AGENT_CONTEXT.md:1477-1521`. | **H** |
| 15 | `HANDOFF 13825-13835` (+ index line 576) | "`tally.py` has a NAME-KEY flaw: unnamed target functions read as MISSING" | Fixed — `13983-13998` and commit `10dff97` (mangled-name pairing pass). In-file correction exists. | — | L |
| 16 | `HANDOFF 673-676` and `4808-4812`, `4841-4848` (+ index lines 39, 142) | "Progress: **11.088%** … **70 commits unpushed**"; per-binary table `d_basesNP.rel 1.015%`. | `d_basesNP` measured **1.941%** after the shelving (`6dbeaf3`). Unpushed vs `origin/master` is **468**, not 70. The overall ~11% and `wiimj2d.dol` ~21.9% are still roughly right (nothing has landed in the DOL since), so `AGENT_CONTEXT.md:32`'s "roughly 11% / around 22%" survives. | `git log --oneline origin/master..HEAD \| wc -l` = 468; commit `6dbeaf3`. | M |
| 16b | `HANDOFF 673` "Where the work now stands" vs `4807` "Current state" | Two sections, ~4,100 lines apart, both presenting themselves as the authoritative status, carrying the same figures. | Neither is dated in its title. A reader cannot tell which is newer, and both are now stale. `592` ("Infrastructure state (as of the 2026-08-12/13 session)") shows the convention that works. | — | M |
| 17 | `HANDOFF 592-621` | "Infrastructure state (as of the 2026-08-12/13 session)" — CI blocked, decomp.dev, `objdiff.json` over-reports, `d_a_player_hio_ADJ.cpp` "the only `nonMatching` slice left". | **Not a finding.** The header is dated, and the one factual claim I could check cheaply still holds: `slices/wiimj2d.json` has exactly one `nonMatching` entry, `dol/bases/d_a_player_hio_ADJ.cpp`, out of 144. This is the section the whole file should look like. | Measured from `slices/wiimj2d.json`. | — |

### Checked and found NOT stale (recorded so nobody re-audits them)

- `AGENT_CONTEXT` "The FPR declaration-order lever governs CALLEE-SAVED registers
  only", using `ProcMain`'s 45-line residual. **Still current** — `QWEN_RESPONSE.md`
  round 23 reports `ProcMain` at 179/179 words, 45 lines differing, closed as a
  bounded negative, and `QWEN_PROMPT.md` line 26 says "do not open it again". The
  "ProcMain 170 → 179" in commit `a4029e9` closed the *length* gap, not the
  register residual.
- `AGENT_CONTEXT` "The static-const-float trap, **unresolved**" — still unresolved.
  Commit `20470d1` re-measured it: define-before-use gives retail's `.sdata2`
  order but folds `16.0f` into a reciprocal multiply; define-after restores
  181/182 but misplaces the symbol.
- `AGENT_CONTEXT` "`.bss` object alignment follows SIZE, not type alignment" —
  holds (`HANDOFF 4237-4280`, probe table). `HANDOFF 2577-2588` narrows rather
  than contradicts it, and says so explicitly.
- `AGENT_CONTEXT` "Unreferenced weak symbols are not placed … roughly two thirds
  of the banked units" — holds: `HANDOFF 527-540` measures 64% (91 of 142) over
  1,592 weak-symbol instances. Only the cross-reference is broken (see B1).
- `AGENT_CONTEXT` "A high tally score does NOT mean a unit is landable" — current,
  and the `d_line_mng` figure in it (181/182, 98.7%) is the *right* one.

---

## (B) PROPOSED `AGENT_CONTEXT.md` EDITS

### B1 — line 326-327, dangling cross-reference

`grep -i "weak.symbol linker placement" HANDOFF.md` returns **nothing**. The
section it means is at `HANDOFF 492-570`, titled "An object may be LARGER than its
slice claims — unreferenced weak symbols are not placed", and it is not a
top-level entry in `HANDOFF_INDEX.md` (it sits inside "409-581 Assembly"), so a
reader cannot find it from the index either.

REPLACE:

```
- **Unreferenced weak symbols are not placed by the linker.** An object whose
  `.text` exceeds its slice claim is normal, not a defect — it is the standard
  condition of roughly two thirds of the banked units. See the Weak-Symbol
  Linker Placement Rule in `HANDOFF.md` before reporting an overflow.
```

WITH:

```
- **Unreferenced weak symbols are not placed by the linker.** An object whose
  `.text` exceeds its slice claim is normal, not a defect — measured at 64%
  (91 of 142) of the banked units, over 1,592 weak-symbol instances. The test
  that matters is `compiled .text - sum(unplaced weak symbols) == slice claim`.
  Read `HANDOFF.md` lines **492-570** before reporting an overflow — that
  section also carries the boundary, which is that a weak symbol that IS the
  surviving definition does occupy its slice.
```

### B2 — line 494-498, the `kinoko_1up` blocker example is dead

`d_a_wm_kinoko_1up.cpp` landed (`HANDOFF 3148-3183`) and is in
`slices/d_basesNP.json`. Worse, the surrounding claim is now known to be too
strong: `HANDOFF 12013-12025` established that referencing an un-landed TU is not
by itself a landing blocker, after three separate "cannot land" objections were
each retired by one build.

REPLACE:

```
- a real **out-of-line** method in an un-decompiled TU. That IS a blocker --
  `d_a_wm_kinoko_1up.cpp` needs `daWmKinokoBase_c`'s ctor and dtor.
```

WITH:

```
- a real **out-of-line** method in an un-decompiled TU. Historically recorded as
  a hard blocker; it is not one on its own. A slice does not need to own, or
  even to have decompiled, the definitions it references — only the bytes it
  claims — and an un-landed region is still present as original binary, so the
  reference resolves. Name the address through `alias_db.txt` or the
  `R_<module>_<section>_<offset>` convention and try the build. Three
  well-argued "cannot land" objections were each retired by one `--verify-bin`
  (`d_a_peach_castle_sequence`, `d_a_ac_switch`, `d_a_floor_jr_b`), and
  `d_a_wm_kinoko_1up.cpp` — this entry's own former example — landed. **A
  structural argument about whether something can land is a hypothesis; the
  build is the experiment.**
```

### B3 — line 543-547, same dead example

REPLACE:

```
**None of them catches an unresolved symbol.** A unit can pass all four and
still fail to link — `d_a_wm_kinoko_1up.cpp` is 9/9 with every check clean and
cannot land, because it inherits from an un-decompiled TU. If `check_vtable.py`
prints `skip (inherited from another TU at 0x...)`, that unit is blocked on
whatever owns that address.
```

WITH:

```
**None of them catches an unresolved symbol, and none of them ever links.** A
unit can pass all four and still fail. `d_line_mng` passed at 181/182, 98.7%
and broke all five binaries: `dLineMng_c::smc_UNIT_SIZE_X` was declared
`static const float` and never defined anywhere, and an unresolved external
emits exactly the `lfs ...@sda21(r0)` retail has, so every affected function
measured as matching. If `check_vtable.py` prints
`skip (inherited from another TU at 0x...)`, resolve the address before
concluding anything — it is as likely to be an inherited inline virtual a
header already defines as a real dependency.
```

### B4 — after line 327, a new entry: the build gate is the only landed-ness authority

Nothing in `AGENT_CONTEXT.md` tells a reader that HANDOFF's "LANDED" headings can
go stale, and four of them have. Suggested insertion at the end of §6, or as its
own short section after §5:

```
- **"LANDED" in `HANDOFF.md` is a claim about the day it was written, not about
  today. `slices/*.json` is the authority.** Four units HANDOFF declares landed
  — `d_a_wm_sandpillar`, `d_a_wm_manta`, `d_a_peach_castle_sequence`,
  `d_a_floor_jr_b` — were SHELVED on 2026-08-21 (slice entries removed, sources
  retained) after the whole-binary gate ran for the first time in ten days and
  found six units independently broken. HANDOFF has no entry recording it.
  Before citing any unit as landed, byte-exact, or a matching sibling, check
  that its source path appears in the relevant `slices/*.json`. It costs one
  grep and it has already cost three agent-rounds not to.
- **`bin/compiled/` still holds objects for shelved units.** `d_a_wm_manta.o`,
  `d_a_wm_sandpillar.o`, `d_a_floor_jr_b.o`, `d_a_peach_castle_sequence.o` and
  the pre-split `d_a_dummy_door.o` are all on disk and are NOT byte-exact
  against retail. The "read a function that already MATCHES" technique above
  says to cross-reference `grep` hits against `slices/wiimj2d.json`; do the same
  for the RELs against `slices/d_basesNP.json`, or the strongest evidence in the
  project quietly becomes the weakest.
```

### B5 — line 1686-1689, add `poolcheck.py` to the gate statement

The entry already says "run `poolcheck.py` over the union", which is right. No
change needed — flagging only that this is the correct text and `HANDOFF 14393`
contradicts it, so HANDOFF is the one to fix, not this.

### B6 — line 32-33, leave alone

"roughly 11%, `wiimj2d.dol` around 22%" survives the shelving (the four units are
11,548 bytes out of 6.5M). If it is being touched anyway, the per-REL figure that
is now wrong lives in HANDOFF, not here.

---

## (C) PROCESS RECOMMENDATION

The failure is not that HANDOFF is append-only. It is that **status and rules are
interleaved in one append-only file**, so a status claim ages silently inside a
document whose other 95% ages perfectly well. Three cheap, one-person-realistic
changes, in order of payoff per hour:

### C1 — Move status OUT of HANDOFF into a generated file. (highest payoff)

Create `STATUS.md`, regenerated by a script, never hand-edited, containing only
things that can be **measured**:

- every entry in `slices/*.json`, per module, with the source path — this is the
  definitive landed list, and it would have caught findings #3, #4, #5, #9;
- the `progress.py --verify-bin` result and timestamp;
- the per-binary percentages;
- the live draft's tally line for whatever unit is in flight, with the command
  that produced it and the timestamp;
- `git rev-parse HEAD` and the count vs `origin/master`.

Then delete the four status blocks from HANDOFF (`673-742`, `793-813`,
`4807-4848`, `8509-8526`) and replace each with a one-line pointer. **Rule:
HANDOFF may state what was true on a date; it may not state what is true now.**
A brief-writer reads `STATUS.md` for state and HANDOFF for rules, and can no
longer confuse the two. This alone kills findings #1, #2, #3, #4, #5, #16, #16b.

### C2 — A `SUPERSEDED:` line, and teach the index generator about it.

Convention: when a later section corrects an earlier one, **go back and add one
line under the earlier heading**:

```
> SUPERSEDED by lines 7068-7073 (2026-08-19): the 8-alignment rule was our own
> filler-alignment bug, not a compiler fact.
```

Then change `wip/wm_units/make_handoff_index.py` to prefix such sections in
`HANDOFF_INDEX.md` with `[SUPERSEDED → 7068]`. This is the cheapest possible fix
for the real mechanism: **agents find sections through the index, and the index
currently reproduces retired rules as titles** — `2725-2763`'s title *is* the
deleted 8-alignment rule, and `2811-2849`'s title *is* the antlion `0xc` that
does not exist. Marking the ~12 sections listed above is under an hour of work
and it is the only change that fixes the search path rather than the document.

### C3 — A tiny lint, run before writing a brief.

`tools/handoff_lint.py`, maybe 60 lines:

1. Parse every `LANDED: <path>` / `<path> IS LANDED` in HANDOFF; assert the path
   is in a `slices/*.json`. Report the ones that are not.
2. Parse every `N/M` tally figure attributed to a named unit; report any unit
   with more than one distinct figure in the file, newest line first.
3. Assert `HANDOFF_INDEX.md` regenerates identically — i.e. the index is not
   older than the file.
4. Warn if `HANDOFF.md`'s last commit is older than HEAD by more than N commits.
   Check #4 alone would have flagged today's entire problem: **17 commits of
   drift**, including a build repair, a green tree, four shelvings, and a unit
   going from 65.9% to 98.7%.

Wire it into whatever the lead runs before dispatching a round. It does not need
to be perfect; it needs to be loud.

### C4 — One discipline change, free

Every status assertion gets a date and a command in the same sentence, the way
`HANDOFF 592` already does ("Infrastructure state (as of the 2026-08-12/13
session)"). `HANDOFF 14388`'s "still open" was false when the ink dried because
nothing forced the author to say *how* they knew.
