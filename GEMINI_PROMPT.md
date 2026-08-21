# Work order — round 20

**Read `AGENT_CONTEXT.md` first.** Several sections are new since your last read,
including one written because of your round 18 and one about a landing failure
this week that is directly relevant to your `__sinit` finding.

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 19 — the reconciliation was exactly right

I asked you to find why the arithmetic did not close, and to name the cause. You
did, and **your answer matches an independent verification of round 18
line for line**: 151 true + 10 constant fixes + 9 new − 8 regressions = 162.

You found all eight regressions yourself, with causes, including the two that
were not regressions at all but **deleted bodies** — `calcKokoopaMdl` and
`getTorideFunfareTime` declared in the header with no definition left in the
`.cpp`. That is the hardest failure mode to catch, because a per-function diff
cannot see it: there is nothing left to compare. You found it by chasing an
arithmetic gap you could have quietly rounded away instead.

That habit is now a standing rule in `AGENT_CONTEXT.md` for every agent on this
project, and both of you are asked to report GAINED and LOST by name from now on
rather than a net count.

Round 19 itself reports **173/251 and 34.94% by bytes**, up from 27.29%, with the
`ShellAtk` family closing as a group. Independent verification is running; I will
tell you plainly if it disagrees.

---

## The `__sinit` finding is right, and it is now MY job, not yours

You reported: 1,446 instructions on both sides, exactly 200 diffs, **every one a
constant `+0x90` displacement on `__vt__18dEnTorideKokoopa_c` lookups** (target
`0x690`, draft `0x600`), caused by missing virtual methods in the base classes.

**That is the correct shape and you stopped in the right place.** It is the same
signature as a unit I closed this week where 175 diffs turned out to be one
substitution repeated — and confirming the delta is *uniform* is exactly what
distinguishes one problem from two hundred.

**Do not attempt the fix.** A change to `d_enemy_boss.hpp` / `d_en.hpp` /
`d_actor.hpp` touches most of the codebase, and the project rule is that a
shared-header change must be verified **alone**, before anything else lands with
it. The tree is currently green for the first time in about ten days and I am not
risking it on a change made in parallel with other work.

**What I want from you instead is the exact proposal.** `+0x90` is 144 bytes =
36 vtable slots, which is a large number of missing virtuals — enough that I want
the slot map derived rather than guessed. The technique that worked on a similar
problem this week:

- take your object's `.rela.data` relocation offsets within the vtable's extent;
- take retail's relocations over the same extent;
- align the two lists and read off, per slot, which function each side puts
  there.

That gives you the true slot map: which slots retail has that we do not, and at
what index they must be inserted. **Report that table.** Class, slot index,
mangled name if known, and how you derived it. If some slots are only
identifiable as "retail relocates this one into the DOL", say that rather than
inventing a name.

Do not edit anything under `include/`. Propose into `scratch/gemini_round20/`.

---

## Round 20 — the rest

Work in `scratch/gemini_round20/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `QWEN_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`.

**Do not run `ninja`, `configure.py`, `progress.py` or `land.py`** — the tree is
green and a concurrent build in this checkout would destroy that.
`harness.compile_draft` is unaffected.

### 1. The two functions you left mid-flight

- **`executeState_AttackSearch`** (512 B) — **2 diffs**, `li r4, 0x0` versus
  `li r3, 0x0` on the `blitzMove(mUnk770 == 0 ? 0 : searchBaseByID(mUnk770))`
  ternary. Two instructions is worth finishing. A ternary whose arms disagree
  about which register holds the result usually means the null arm is being
  materialised into the *return* register rather than the *argument* register —
  try hoisting the ternary into a named local of the callee's parameter type and
  passing that, so the argument slot is fixed before the call.
- **`executeState_ShellAtk_St`** (612 B) — **20 diffs**, all in the last 15
  instructions, branch inversion around the `mUnkAA0` decrement and
  `l_bounceSpeed` indexing. Relevant levers, all measured and in
  `AGENT_CONTEXT.md`: a literal `>=`/`<=` lowers through `cror` where a direct
  `<`/`>` gives a bare hardware branch; which block you make the "then" changes
  the emitted polarity; and `return A && B;` compiles to an early-return shape
  where retail may want a shared exit label.

### 2. `setBeginMoveState` — fourth round of asking

`stw r0, 0x848(r31)` in retail versus `0xac8` in the draft. Flagged in rounds 17,
18 and 19 and still not addressed; last round the field was *renamed* to match the
wrong offset rather than the offset being corrected. Two instructions out of 38.
**Please just fix it or tell me why it cannot be fixed.**

### 3. Then continue biggest-first

After those, work down the ranked unmatched list as you have been. Report the
list before and after.

---

## Reporting

- **GAINED and LOST by name**, as a set difference against round 19. Not a net
  count.
- The `__sinit` slot-map table, with its derivation.
- Ranked unmatched list by size, before and after.
- Per function: target bytes, draft bytes, match status. **Draft size first** — a
  `0 B` draft is unwritten, not mismatched, and that distinction is what hid the
  two deleted bodies.
- Constants decoded with `pool.py`, and any false positive caught.
- Negatives stated plainly with the residual characterised.

One thing worth knowing, because it cost me a day: **a high match score does not
mean a unit is landable.** I had a unit at 98.7% that broke all five binaries on
landing, because the scoring tool never runs the linker — it cannot see an
undefined symbol, a weak symbol we place that retail takes from elsewhere, or a
wrong data-section order. It is in `AGENT_CONTEXT.md` now. Your score is real;
just do not read it as "done".
