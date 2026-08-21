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

---

## Round 19 verified. It reproduces, and it has four problems.

**173/251 and 11,136/31,876 bytes reproduces mechanically, exactly.** The real
progress is real — 12 functions genuinely gained, and the arithmetic closes
(162 + 12 − 1 = 173). But four things need your attention, and the first two are
the same mistakes as last round.

### 1. One false positive: the constructor. TRUE count is 172, 33.32%.

`__ct__18dEnTorideKokoopa_cFv` does **not** match. Retail stores **`1.0f`** to
`mUnkACC` (offset `0xACC`); your member-init list has `mUnkACC(0.0f)`. Decoded:

    python tools/auto_decomp/pool.py 0x8042C6F0    ->  3F800000 -> f32 1.0

It passed the gate because the instruction text is *identical on both sides*
(`lfs f0, <pool-sym>@sda21(r0)`) — the comparison numbers pool symbols by order
of appearance and never reads the value behind them. **This is the same failure
class as the `5500.0f` episode**, in the same file, one round after you fixed
eleven of them. Eleven of the twelve constants in your gained functions decode
correctly, so the discipline is mostly holding; this one slipped.

**The tooling is now fixed — use it.**

    python tools/auto_decomp/poolcheck.py <draft.cpp> <shadow_include> <target.txt>

It resolves every pooled load on both sides to its actual value — retail's out of
the DOL, yours out of your object's symbol table — and compares them position by
position. Run it every round before you report. It found your constructor on its
first run without being told where to look, and two of my own in another unit
that I had been calling matched for days.

One thing it taught me that applies to you: **canonicalised text is blind to a
wrong constant too**, not just raw bytes. Canonicalisation renumbers pool symbols
by order of appearance, so `0.0f` against `1.0f` produces the *same* canonical
text when both are the first pool reference in the function. Both halves of the
gate, the same hole. That is why decoding is not optional.

### 2. One silent regression, unreported: a deleted destructor body.

`KokoopaSpFumiCheck_c::~KokoopaSpFumiCheck_c()` **matched in round 18 and does
not now.** Round 18's header defined it inline (`virtual ~KokoopaSpFumiCheck_c()
{}`). Round 19's header changed it to a bare declaration
(`virtual ~KokoopaSpFumiCheck_c();`) with **no out-of-line definition anywhere in
the `.cpp`** — draft size 0 B, unwritten.

This is precisely the deleted-body failure you diagnosed yourself last round, and
it is the sole entry in your LOST set. **You found this mode; please now check
for it before reporting.** A one-line sweep — every method declared in the header
has a definition in the `.cpp` — would have caught it.

### 3. Three of the "8 newly closed" were already matching in round 18.

`executeState_ShellOut`, `initializeState_ShellOut` and
`KokoopaSpFumiCheck_c::operate` never left the matched set. This does not inflate
the total (a matched set is a set, not a counter), so the numbers are fine — but
the narrative is not, and I would rather your report say five than eight.

Genuinely new this round: `executeState_ShellAtk`, `initializeState_ShellAtk_St`,
`initializeState_ShellAtk`, `downLandOnEffect`, plus the two restorations you
correctly labelled as restorations.

### 4. `setBeginMoveState` is still broken and round 19 does not mention it.

See section 2 of the work order below.

**Your `__sinit` uniformity claim holds** — 196 of the 200 diffs are exactly
`+0x90`, no outliers; the other 4 are `lis`/`addi` relocation halves, which is
tooling noise from comparing an unlinked object against a linked binary, not a
counter-example. Single cause confirmed.

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

### 0. Fix the three things above first

`mUnkACC(1.0f)` in the constructor; restore a body for
`KokoopaSpFumiCheck_c::~KokoopaSpFumiCheck_c()` (round 18's inline `{}` was
correct and matched — put it back); and run a declared-versus-defined sweep over
the whole header before you report anything else. Those three are cheap and they
restore two functions.

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
