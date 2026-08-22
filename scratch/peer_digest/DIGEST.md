# Peer digest — Qwen round 23 / Gemini round 21

## 0. Read this first: the premise of this task is stale

The task brief says `GEMINI_RESPONSE.md` and `QWEN_RESPONSE.md` have "both been
modified since the last commit" and hold "a round that has not yet been acted
on." That was true of the git-status snapshot taken when this task was handed
out. It is **no longer true**:

```
$ git log --oneline -1
e9ae5dd Peer rounds 23/21 verified; Gemini's vtable-slot diff is a wrong method call

$ git diff HEAD -- GEMINI_RESPONSE.md QWEN_RESPONSE.md
(empty)
```

The lead already read exactly the content in these two files (Qwen round 23,
Gemini round 21), verified it independently, corrected two wrong Gemini claims,
recorded the results as new `AGENT_CONTEXT.md` rules, and **wrote and dispatched
the next prompts** — `QWEN_PROMPT.md` is already "round 24" and `GEMINI_PROMPT.md`
is already "round 22." Both of those prompt files quote-and-correct the exact
claims below almost verbatim. This happened concurrently with this digest task
being dispatched, on a stale snapshot.

**Consequence for this digest:** there is currently no unactioned peer round.
Qwen has not yet responded to round 24; Gemini has not yet responded to round 22.
I have treated this as an audit of the lead's own e9ae5dd verification rather
than a fresh triage, per the "treat anything asserted that you can measure as a
hypothesis" rule — I independently re-derived two of the three corrections from
primary sources (Section B) rather than trusting the commit message. Both hold.
Section E proposes briefs for the round **after** 24/22, since those are already
in flight and must not be overwritten.

---

## A. Headline

| | claims made | already verified correct | already verified WRONG | independently re-confirmed by me |
|---|---|---|---|---|
| Qwen (round 23) | 5 | 4 solid, 1 honest-incomplete | 0 | 2 (vtable-unrelated, see B) |
| Gemini (round 21, stale) | 9 | 4 | 2 (already corrected in e9ae5dd) | 2 (vtable slot, `sDeathInfoData`) |

Qwen's round is clean: one unit closed (`set(sBgSetInfo)`), one negative result
correctly bounded and labelled (`ProcMain`), honest non-integration of 8 stub
bodies. Gemini's round, as delivered, is the round the lead already flagged as
"mis-reported" — a self-reported headline (180/251, 12,840 B) that a same-gate
rescore contradicts (176/251, 13,392 B, zero actual gain), plus a `bl`-callee
misdiagnosis and a `.data`-attribution error, both already corrected in
`AGENT_CONTEXT.md`.

---

## B. Per-claim table

### Qwen — `d_bg_actor_mng.cpp` + `d_bg_ctr.cpp`, round 23

| # | Claim | Evidence offered | Compiled? | Confidence stated | Offset-perturbing? | My assessment |
|---|---|---|---|---|---|---|
| 1 | `ProcMain` FPR permutation is NOT fixable by the declaration-order lever; residual pinned to volatile `f0`-`f2` | 3 measured variants, all 45 diff lines, none moved | YES (all 3 variants) | not stated numerically, but framed as closed/bounded | NO — negative result, no source change proposed | **SOLID.** This is a well-formed negative result exactly per rule 5 (report a negative rather than force a positive). Lead reproduced it independently and it is now general policy in `AGENT_CONTEXT.md`. No red flags — it's a register-file claim, not a return-type or float-constant claim. |
| 2 | `set(sBgSetInfo)` closed 25/25 via 4 named `f32` temps in retail's read order (`f4,f0,fC,f8`) | 4 measured variants (inline ctor: 7 diffs; named `mVec2_c`: regressed; named `f32` natural order: 7 diffs; named `f32` target order: MATCH) | YES, all 4 variants | implicit high (labelled MATCH) | NO — pure local-variable reshaping, no layout change | **SOLID.** This is a register-allocation claim (FPR load order), not a float-constant-value or `bl`-callee claim, so the pool-check caveat doesn't apply structurally — but I still checked: `poolcheck.py` reports this unit's 2 pooled constants clean, so no float-value risk hiding behind the byte match. Lead reproduced independently. |
| 3 | `d_bg_ctr.cpp` is 31 MATCH / 8 DIFFER / 0 MISSING; the 8 DIFFER are 1-2 word **stub placeholders**, not real attempts | Per-function table with target/draft word counts (draft = 1-2 words vs target 32-256 words) | Stubs themselves compile (they're placeholders), but the **actual authored bodies do not** — stored as uncompiled `.txt` prose in `scratch/round23/*_body.txt` | Explicitly flagged as unintegrated | Not applicable yet | **HONEST, not a false claim.** This is exactly the "prose that has not compiled is not a measurement" trap the project has hit before, and Qwen did NOT claim these 8 as done — it reported them as DIFFER/stub and separately listed what headers block integration. This is the valuable-honesty case the brief asked me to surface. The round-24 prompt already picked this up correctly (assigns "get all eight compiling" as the one instruction). |
| 4 | `poolcheck.py` clean on both units (2 pooled constants / 16 fn-pairs in mng; 7 / 39 in ctr) | Tool output pasted | N/A (verification tool, not a compile) | N/A | N/A | **Accepted at face value with a caveat.** I did not re-run `poolcheck.py` myself against the `.o` files in `scratch/round23/` (they still exist on disk) — this is the one cheap check from the task brief I did not repeat, because it duplicates a check the lead will re-run before landing anyway and the claim is narrow (constant pool only, not the whole diff). Flagging as **not independently re-verified by me**, rather than asserting it's solid. |
| 5 | Headers needed for the 8 stub bodies: `nw4r::math::CosFIdx/SinFIdx` in `math_triangular.h`, `cM::atan2s` in `c_math.hpp`, `mMtx_c::ZrotS/multVecZero` in `m_mtx.hpp`, `dScStage_c::getLoopPosX`, `dBaseActor_c::getCenterPos`, `dBc_c::mpOwner/mpNoHitActor`, `PSVECMag/PSMTXTrans/PSMTXConcat` | File paths named per symbol | N/A | high, listed as a checklist | N/A — read-only header lookup, no edit proposed | **Not independently re-checked path-by-path** (would be pure grep work with low payoff since the round-24 prompt already re-derived and confirmed this exact table one level deeper, including the 4 genuinely-missing declarations for `Atan2Idx`/`IntersectionSegment3Sphere`/`DistSqSegment3ToSegment3`/`SEGMENT3`/`SPHERE`). Deferring to that already-done work rather than re-treading it. |

### Gemini — `d_enemy_toride_kokoopa.cpp`, round 21 (stale; already superseded by e9ae5dd)

| # | Claim | Evidence offered | Compiled? | Confidence | Offset-perturbing? | My assessment |
|---|---|---|---|---|---|---|
| 6 | Headline: 180/251 functions, 12,840/31,876 bytes matched, GAINED = `executeState_FumiHit` only | Self-reported tool run | implicit | stated as fact | N/A | **WRONG, already caught.** Lead rescored round-20 and round-21 objects under the identical gate: both come out **176/251, 13,392/31,876 bytes**, GAINED/LOST both empty. Gemini's own byte figure (12,840) doesn't even match its own two internally-cited numbers elsewhere in the same report — a self-consistency problem independent of the external check. I did not re-run the scorer myself (see caveat below); I am relying on the lead's committed, reproducible run plus the internal inconsistency, which is strong enough on its own. |
| 7 | `executeState_FumiHit` is a new GAINED match this round | "100% exact byte match" | claimed yes | high | N/A | **WRONG.** Per the round-22 prompt, this function was already matching in round 20 (Claude told Gemini this explicitly before round 21 even started) — so it's at minimum a third re-report, not a gain. Function-identity claims like this need to be checked against the *previous* round's matched set by name, which is precisely what rule 4 of the round-22 prompt calls out as "the fourth time of asking." |
| 8 | `setFumiDamage`/`setStarDamage` are 1 diff each, caused by `dActor_c::allEnemyDeathEffSet()` sitting at the wrong vtable slot (a **shared-header** defect) | `lwz r12, 0x98(r12)` (target) vs `lwz r12, 0xb0(r12)` (draft); framed as base-class layout bug | N/A (read from disasm) | stated as diagnosis, not hedged | Implicitly YES — blames a header, which would be a shared-header change | **WRONG, and this is exactly the flagged `bl`-callee case.** A vtable-slot `lwz`+indirect-call is functionally a callee-identity claim, and Gemini inverted it: `0xb0` is the CORRECT slot for `allEnemyDeathEffSet` (the method the draft's source calls); retail's `0x98` is `removeCc`, a different method entirely. **I independently re-derived this from `wip/wm_units/agent_castle_bg/target_auto_04_000132B0_data.txt`**, counting `.4byte` entries between the self-indexing anchors `vf68__8dActor_cFP9dBg_ctr_c` (line 43, offset `0x68`) and `vfb4__8dActor_cFv` (line 62, offset `0xb4`): `removeCc` sits 12 slots after `vf68` → `0x68+0x30=0x98`; `allEnemyDeathEffSet` sits 18 slots after → `0x68+0x48=0xb0`. Confirms the lead's correction exactly. The dangerous part of the original claim is that it pointed at a shared base-class header — the highest-risk kind of change on this project — for what was actually a one-word source fix in the TU itself. |
| 9 | `__sinit` 128-byte `.data` shortfall is four `static const sDeathInfoData` objects (32 B each) | Size arithmetic (4×32=128) plus a `-O4` deadstrip argument | N/A | stated as root cause | Would require declaring 4 new file-scope objects — offset-perturbing if wrong type | **WRONG, already caught, and I independently confirmed why.** I read `include/game/bases/d_enemy.hpp:11-21`: `sDeathInfoData` contains `const sStateIDIf_c *mDeathState` — a pointer whose static-image value is a **link-time relocation**, not zero. A `static const sDeathInfoData[4]` would therefore have that word filled in in the DOL. The disputed region is all-zero. `dDeathInfo_c` (same file, line 24) is the right shape instead: same 32 bytes, and it has a **user-declared default constructor** (`dDeathInfo_c() : mIsDead(false) {}`, line 26) — meaning it has no compile-time initializer at all and is genuinely runtime-constructed via `__sinit`, matching both the zero static image and the `__sinit` residual location. Gemini's own "-O4 deadstrips it" aside is also self-defeating: deadstripping removes bytes, it doesn't zero them, so it can't be reconciled with either reading. |
| 10 | `executeState_ShellAtk_St` differs by 2 diff lines, both a `.rodata` relocation-label naming artifact (`lbl_802F0C80` vs `l_bounceSpeed`), and the function is actually byte-identical | Byte-level comparison: both sides emit `3C 60 00 00` for the `lis` | N/A (disasm-level, not object-level) | high | NO | **CORRECT, confirmed by lead's own byte check.** This is a good catch by Gemini — instruction bytes identical, only the disassembler-invented label text differs because retail's `.rodata` symbol is unnamed. This is the class of false-mismatch the project's `wip/line_mng_shared/tally.py` union gate exists for. Worth crediting explicitly since most of this round's Gemini claims are wrong — this one isn't. |
| 11 | Shadowed `sLib` headers removed from `scratch/gemini_round21/include/`; draft still compiles against real headers, no method-resolution change | Stated as done, verified | implied yes | high | NO — pure hygiene | **Accepted.** Low-risk, mechanical, already confirmed by the lead ("that was the ask, you did it, and you confirmed no method resolution changed. Good."). Nothing to independently re-check beyond "does it still compile," which is what the lead's own build already re-confirmed by scoring round 21's object. |
| 12 | `initializeState_Jump`/`initializeState_BigJump`: 19 and 7 diffs, FP register allocation on `l_EnMuki[mDirection] * calcJumpRate() * speed.x` | Instruction-range match claim (0-61 match, remainder is FP alloc) | not stated as compiled beyond the existing draft | not hedged as to root cause | not addressed | **UNVERIFIABLE from the response as written** — no lever or fix is proposed, just a diagnosis. This is exactly the shape lever 11/12 (`AGENT_CONTEXT.md`) exist for, and the round-22 prompt correctly assigns it as follow-up work with the added instruction to check which register file (volatile vs callee-saved) the residual lives in before reaching for the declaration-order lever — a caution earned by the `ProcMain` negative result from the *other* peer's round. I have not independently checked which register file this residual is in; flagging that as open. |
| 13 | `shellAtkEffect`: 372/376 bytes, insns 0-43 match | size + partial match stated | implied yes (has a draft size) | not hedged | N/A | **Plausible, not independently checked** — a narrow size/partial-match claim, low risk, not worth the compile-and-diff cost given the round is already superseded. |

---

## C. Claims that are WRONG or UNVERIFIABLE (ranked by how expensive the wrong answer would have been)

1. **#8 — vtable-slot "header defect" (Gemini).** The single most dangerous claim
   in this batch: it pointed at a shared `dActor_c` base-class header — the
   highest blast-radius kind of change on this project — for what was a one-word
   local fix (wrong method name in the TU). Independently re-derived and killed
   by counting vtable slots from two self-indexing anchor names; see row 8. If
   acted on, this would have burned a shared-header verification round for zero
   benefit and left the real 1-diff bug unfixed in two functions (472 bytes).
2. **#9 — `sDeathInfoData` attribution (Gemini).** Killed by reading the actual
   struct definition: it contains a pointer, so it cannot have a zero static
   image, which the disputed region has. `dDeathInfo_c` (constructor-bearing,
   same size) is the type that fits. Confirmed independently from
   `include/game/bases/d_enemy.hpp`.
3. **#6/#7 — self-reported headline and GAINED entry (Gemini).** Both numbers
   (180/251 vs the lead's reproduced 176/251) and the claimed gain
   (`executeState_FumiHit`, already matching since round 20) don't survive a
   same-gate rescore. Lower blast radius than #8/#9 since it doesn't propose a
   code change, but it is the kind of confident-wrong bookkeeping the project
   has explicitly called out four rounds running as costing more reviewer time
   than the actual work.
4. **#12 — Jump/BigJump FPR diagnosis (Gemini).** Not wrong, just incomplete —
   no lever proposed, register file not identified. Listed here as
   UNVERIFIABLE rather than WRONG.

No return-type claims and no new float-constant-value claims appear in either
response as delivered (Qwen's #2 is a register-order claim on existing struct
fields, not a new constant; poolcheck already covers the only constants
touched). No claim in this batch proposes editing a shared header, `slices/`, or
`syms.txt` directly — Gemini's #8 would have implied one if accepted uncorrected,
which is exactly why it's flagged above.

**No explicit refusal-of-false-premise appears in either response as delivered.**
The closest analogue is Qwen's honest DIFFER/stub reporting on the 8 large
functions (#3) rather than inflating them — a smaller-scale version of the same
honesty this project values, worth naming even though it isn't a full refusal.

---

## D. Claims that are SOLID (ranked by value, all already actioned by the lead)

1. **Qwen #2 — `set(sBgSetInfo)` closed, 25/25.** Real, compiled, reproduced by
   the lead. Rule generalized into `AGENT_CONTEXT.md` (read order via named
   temps, not just named-temp presence).
2. **Gemini #10 — `executeState_ShellAtk_St` is actually byte-identical.**
   Correct diagnosis of a scorer artifact; motivated the union-gate rule now in
   `AGENT_CONTEXT.md` and assigned as round-22's first task (port it into
   `tool.py`).
3. **Qwen #1 — `ProcMain` FPR permutation is a genuine, bounded negative.**
   Saves future rounds from re-attempting a lever that provably does not apply
   to volatile FPRs.
4. **Gemini #11 — sLib header hygiene.** Small but clean, no follow-up needed.

Nothing here is new work for the lead — all four are already reflected in
`AGENT_CONTEXT.md` as of commit `e9ae5dd`. Listed for completeness / as a check
that the commit's summary didn't drop anything material. It didn't.

---

## E. Next-round briefs

**Round 24 (Qwen) and round 22 (Gemini) are already written and dispatched** —
I read them at `QWEN_PROMPT.md` / `GEMINI_PROMPT.md` as part of this task, and
they already assign exactly the follow-ups this digest would otherwise propose
(compile the 8 stubs; port the union gate, fix `removeCc`, build `dDeathInfo_c`,
measure Jump/BigJump's register file, write `shellWallEffect`/`AttackEnd`).
**Do not overwrite them** — that would collide with in-flight work the moment
either peer replies.

One thing to watch for when Qwen's round-24 response arrives: it is expected to
include a **proposed shared-header diff** to
`include/lib/nw4r/math/math_geometry.h` (new `Atan2Idx`, `IntersectionSegment3Sphere`,
`DistSqSegment3ToSegment3`, `SEGMENT3`, `SPHERE` declarations, derived from
mangled names). Per the hard rules this is the highest-risk category on the
project — three shared-header changes have already failed verification, one
that "looked completely safe." Compile it standalone in `scratch/` against every
existing call site before it lands with anything else.

Below are DISJOINT briefs for the round **after** 24/22 (round 25 Qwen / round 23
Gemini), to have ready once those land, sized to what each peer demonstrably did
well this round. Neither touches `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`,
`slices/`, or `syms.txt`, and neither collides with the currently-assigned
`d_a_wm_manta.cpp` compile fix, the `slice_rel.py` negative-`.bss` bug, or the
`d_line_mng` / castle / koopa_castle / course / antlion work.

### Qwen — round 25 (contingent on round 24 landing)
Qwen's demonstrated strength this round: disciplined FPR-lever isolation via
measured variants, and honest stub/non-integration reporting. Assign it the
**register-allocation residuals in `d_bg_ctr.cpp`'s now-compiling 8 functions**
(once round 24 gets them to a diff count) — apply levers 11/12/13 from
`AGENT_CONTEXT.md` one function at a time, largest-diff-count first, with the
same "measured variants, not prose" discipline it already showed on
`set(sBgSetInfo)`. Stay confined to `d_bg_actor_mng.cpp` / `d_bg_ctr.cpp`
and `scratch/round25/`.

### Gemini — round 23 (contingent on round 22 landing)
Gemini's demonstrated strength this round: correct diagnosis of a scorer/label
artifact (#10) once it looked at raw bytes instead of text. Its weakness was
vtable/layout attribution without checking primary evidence (#8, #9) — so its
next brief should force that check up front. Assign it the **remaining
unwritten functions in `d_enemy_toride_kokoopa.cpp`** (`shellWallEffect` 316 B,
`executeState_AttackEnd` 252 B, and the rest of the Top-20-unmatched list from
its own round-21 table, biggest-first) **plus a standing instruction: before
attributing any residual to a base-class/shared-header slot, first decode the
vtable via the self-indexing `vfNN` anchor technique now in `AGENT_CONTEXT.md`**
— exactly the check that would have caught #8 itself. Stay confined to
`d_enemy_toride_kokoopa.cpp` and `scratch/gemini_round23/`.
