# Work order — round 17

**Read `AGENT_CONTEXT.md` first.** It is the standing briefing, and it gained
**two new levers and several new proven-negatives today** — see "what changed"
below. This file is only round 17.

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 16 verification — COMPLETE. Read this before continuing round 17.

I said I would recompile and compare independently. That is done, and the result
changes what you are working on. **Your core technique is real. Your headline
number is not, and the module attribution is wrong.**

### 1. This is DOL work, not REL work

`dEnTorideKokoopa_c` has **zero** symbols in `d_en_bossNP` and 150+ in
**`wiimj2d`**, the main DOL — constructor, vtable install, every state handler,
all of it, from `0x800A88A0` onward. Your own reference dumps came from the DOL.
Independent corroboration: `0x800B0A20`, the end of your range, is
`startFadeIn__8dFader_cFUs`, a DOL symbol.

So the "0.031% virgin REL" framing is wrong at the module level. This work is
measured against wiimj2d's 21.887%, and every landing artifact changes: source to
`source/dol/bases/`, the slice entry into `slices/wiimj2d.json`, and the compile
flags are wiimj2d's (`-O4 -fp hard`), **not** a REL's `-O4,p -sdata 0`. If you
have been compiling with REL flags, that alone may explain divergence.

### 2. The verified numbers

Recompiled fresh with wiimj2d flags and diffed all 251 target functions:

> **88 / 251 functions genuinely match** — very close to your 89, so your
> function-level work is sound.
> **2,724 / 26,100 bytes (10.44%)**, not 8,508 / 31,876 (26.69%).

The byte figure is inflated almost entirely by one function.

### 3. Two claimed matches that are not matches

- **`__sinit` (5,784 bytes — 68% of your claimed matched-byte total).** It sits
  in a gap that neither of your two reference dumps covers, so your
  `compare_emitted.py` could never have examined it — yet your table lists it
  MATCH. Diffed against the real pre-split object: **314 of 1,446 instructions
  differ.** Retail uses r28 as the shared base register for the state table; the
  draft uses r29 with different offsets throughout. Substantive, not cosmetic.
- **`tenmetsuFin` (36 bytes).** Listed MATCH. It is declared virtual in the
  header and **has no definition anywhere in your .cpp**. Never emitted.

**A function absent from your reference dump must be reported UNKNOWN, never
MATCH.** That single rule would have caught both.

### 4. The false-positive trap is live — I found two instances in your file

I warned about this last round. It is not theoretical:

> `calcRootJntPos` and `calcShellJntPos` both claim `...z = 0.0f;`. The
> instruction pattern matches perfectly. **The real retail float at that pool
> address is `5500.0f`.**

Decoded straight from `original/wiimj2d.dol` (map VA→file offset with
`dtk dol info`). The canonicaliser cannot catch this — it numbers pool symbols by
first appearance and never reads their value. Fix those two z-values.

Net: **≈84–86 solid, independently-verified matches** out of your 89. That is a
good result; it just is not 26.69%.

### 5. The free-functions technique — CONFIRMED, but 82% smaller than claimed

This is the part I cared most about, and **it is genuine.** The
`STATE_VIRTUAL_DEFINE` machinery really does emit byte-exact code before any
body is written: 28 `baseID_<State>` accessors, `__dt__33sFStateID_c`,
`__dt__40sFStateVirtualID_c`, `superID`, `number`, `isSameName`, and the three
`initializeState/executeState/finalizeState` dispatchers — **36 real functions,
verified.**

But **~1,276 bytes, not 7,008.** Your figure folded `__sinit` into the group, and
`__sinit` is the one that is wrong. Still a genuinely repeatable technique and
still worth writing up — just at its true size.

### 6. Blocking dependency you should know about

`dEnBoss_c`, `daBossDemo_c`, and `dBossLifeInf_c` have **zero** decompiled
functions in wiimj2d and no canonical headers. Your headers for them are your own
unverified hypothesis — I did not re-verify `dEnBoss_c`'s 0x600 layout or its
vtable slots, and that is a verification task the size of this one. Landing
`dEnTorideKokoopa_c` is not really possible until its base class is settled. Your
`sizeof == 0xE70` is plausible but unproven: no independent allocation-site
literal was found.

Your `d_cc.hpp` `isLinked()` addition is a real, mergeable improvement. Your
`d_actor_manager.hpp` change is layout-compatible but names a pointer to a class
that does not exist canonically yet.

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

### 0. First — correct the foundation (do this before writing any new body)

Fast and mechanical, but everything downstream depends on it:

- **Recompile with wiimj2d's flags** (`-O4 -fp hard`), not REL flags. If you have
  been using `-O4,p -sdata 0`, some of your unmatched functions may close for
  free.
- **Fix `calcRootJntPos` / `calcShellJntPos`** — the constant is `5500.0f`.
- **Re-audit your whole match table against your reference dumps.** Any function
  not covered by a dump is UNKNOWN. Report how many rows change status.
- Restate the module as `wiimj2d` everywhere, including the landing plan.

Then give me a corrected baseline number before you add anything to it. I would
rather have a true 10.44% than a claimed 26.69%.

### 1. Write the highest-value function bodies

Work down by **word count**, biggest first — the 251 functions are wildly uneven
and a handful of large ones are worth more than fifty stubs. Report a ranked list
of what is still unmatched by size before you start, so the choice is visible.

Apply levers 11 and 12 from the start. This is a float-heavy actor — positions,
angles, scale speeds — so both will come up.

### 2. Nail down the free-functions technique

Verification confirmed all 36 — so write up **how** the state
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
