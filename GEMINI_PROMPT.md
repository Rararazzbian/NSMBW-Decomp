# Work order — round 17

**Read `AGENT_CONTEXT.md` first.** It is the standing briefing, and it gained
**two new levers and several new proven-negatives today** — see "what changed"
below. This file is only round 17.

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 16 — received, and independent verification is running

You opened `d_enemy_toride_kokoopa.cpp` on a module that was at **0.031%** with
6 of 2,384 functions named. Essentially nothing existed there. You reported
**89/251 functions and 8,508/31,876 bytes (26.69%)**, a full `dEnTorideKokoopa_c`
layout at `sizeof == 0xE70` with per-field provenance, and the striking claim
that **36 functions match byte-for-byte from the 28 state declarations alone,
before any function body was authored**.

I have not confirmed those numbers yet — an independent recompile-and-compare is
running now and I will report the result next round. That is not scepticism about
you specifically; nothing goes into the tree here on a self-reported figure, and
three separate agents today had claims corrected on verification. If my numbers
come back lower than yours I will tell you exactly where they diverge.

**The 36-free-functions result is the part I care most about**, more than the
percentage. If the state-declaration machinery really does emit that many
byte-exact functions before anyone writes a line of logic, that is a repeatable
technique worth applying to every remaining boss actor in this REL — and there
are a lot of them. Verification is checking it specifically.

One thing being checked hard, and worth you knowing about: **raw-byte equality
can be a false positive.** Relocated address fields are zeroed in both
disassemblies, so two instructions that call *different functions* can compare
byte-identical. In a file of 251 functions, many of them small state-machine
boilerplate, that trap is live. When you claim a match on a small function,
verify the symbols it references resolve correctly — not just that the bytes
line up. Please build that check into round 17 rather than leaving it to me.

---

## What changed in `AGENT_CONTEXT.md` today — read before starting

Two new levers, both found and verified this session:

- **Lever 11** — a float product of a member and a literal. The `fmuls` operand
  order in the target *tells you which source shape produced it*. Validated
  against 115 samples of already-matching code with no exceptions, so treat it
  as a lookup rather than a hypothesis to test.
- **Lever 12** — the residual where the instructions are all correct but the FP
  register *numbers* are rotated. That is operand evaluation order, and there is
  a one-line source fix.

Newly recorded **proven negatives** — do not spend a round on any of these:
commutative float operand order; naming a float constant in any foldable form;
translation-unit or literal-pool ordering; compiler flags (~145 variants, four
compiler versions); and `fmuls` slot choice when both operands are already live
in registers.

**A tooling fix that affects your measurements:** `harness.canonicalise` had
three bugs that made it report UNEQUAL for functions whose every byte matched —
unstripped quotes around pool symbols, no handling of `sbss` sections, and
mangled-versus-placeholder `bl` targets. All three are fixed. If any round-16
function was written off as "differs by symbol names only", re-check it; some of
those were always matches.

---

## Round 17 — function bodies

The framework is (pending verification) proven. The next step is content.

Stay in `d_enemy_toride_kokoopa.cpp` and its header, working in
`scratch/gemini_round16/` or a new `scratch/gemini_round17/`. Do not touch
`wip/**`, `src/**`, `include/**`, `slices/`, `syms.txt`, `configure.py`,
`QWEN_*`, `CODEX_HANDOFF.md`, or `HANDOFF.md`. Another agent is working
`d_bg_actor_mng`, and `wip/fix_bigtwo/**` is mine and moving constantly.

### 1. Write the highest-value function bodies

Work down by **word count**, biggest first — the 251 functions are wildly uneven
and a handful of large ones are worth more than fifty stubs. Report a ranked list
of what is still unmatched by size before you start, so the choice is visible.

Apply levers 11 and 12 from the start. This is a float-heavy actor — positions,
angles, scale speeds — so both will come up.

### 2. Nail down the free-functions technique

Whether or not my verification confirms all 36, write up **how** the state
declarations produce matching functions: what exactly you declare, in what
order, and which parts of the emitted code that fixes. Be specific enough that
it could be applied to the next boss actor by someone who has never seen this
one.

If the technique has limits — state kinds it does not cover, or cases where the
generated function is close but not exact — say so. The limits are as useful as
the technique.

### 3. Layout evidence

Your layout table marks fields as proved or inferred, which is the right
instinct. For round 17, please separate them explicitly: which offsets are
**forced** by an instruction you can point at, and which are **plausible
guesses** that merely do not contradict anything. A wrong-but-consistent field
name is much more expensive to remove later than an honest `mUnk`.

---

## Reporting

- Ranked list of remaining unmatched functions by size, before and after.
- Per function attempted: target words, draft words, match status. **Length
  first** — a length mismatch is content or a different `_savegpr` level, never
  plain register allocation.
- Your symbol-reference spot-checks: how many, and any false positives caught.
- The free-functions write-up, with its limits.
- Negatives stated plainly. A function you could not close, with a precise
  characterisation of the residual, is worth more than a vague claim of partial
  progress.

If you hit something that needs a shared header or a build-config change, do not
work around it — state the hypothesis, say what test would settle it, and stop.
The other peer did exactly that this round and it was the right call.
