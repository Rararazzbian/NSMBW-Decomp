# Work order — round 19

**Read `AGENT_CONTEXT.md` first.** It is the standing briefing, and it gained
**two new levers and several new proven-negatives today** — see "what changed"
below. This file is only round 19.

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 18 verified — and you did the two hardest things right

**You found a real fix by yourself.** Changing `(u32)` to `(int)` on the
float-to-integer conversions moved the compiler off the `__cvt_fp2unsigned`
runtime helper and onto inline `fctiwz`, which is what retail does. That is the
correct diagnosis and the correct fix, and you got there from the disassembly
rather than by guessing.

**You refused to edit a shared header, and said so.** That was the right call,
and the hypothesis was right too. **I tested it. Here is your answer.**

`m_vec.hpp:140`'s user-declared copy constructor is *exactly* what suppresses
MWCC's bitwise copy. Remove it and the struct copy flips from float
`lfs`/`stfs` to retail's integer `lwz`/`stw`, precisely as you predicted. The
destructor and the `(const&, float)` constructor are red herrings — neither
matters. Your diagnosis was correct in full.

**And we are not going to do it.** Measured across all 66 landed source files
that use `mVec3_c`: **160 currently-matching functions regress**, across 49
files. Nothing is gained, because the instruction *count* is identical either
way — 6 loads and 6 stores whichever form is used — so the copy shape alone
could never have closed `ProcMain`'s length gap. If retail really was built with
a POD `mVec3_c`, matching it is a whole-project migration, not a header tweak.

This is now recorded in `AGENT_CONTEXT.md` as a proven negative, credited to
your diagnosis. **Do not spend any more time on the struct copy.**

### The real cause of `ProcMain`'s 17 words — measured, and it is yours to close

With the copy question settled, the full aligned diff reads clearly:

> **Retail RECOMPUTES `obj = &m_pObjList[i]` before every use** —
> `lwz r0, 0x38(r26)` then `add rX, r0, r30` — **11 times.**
> Our draft CACHES it in one register and reuses it via cheap `mr`, 9 times.

That recompute-versus-cache split accounts for essentially the whole gap. It is
a source-shape question of exactly the kind you are good at: look at how the
`obj` local is scoped and reused inside the loop body. Retail's shape is the one
that does *not* keep a pointer alive across the iteration.

One more correction, to you this time: you reported a missing `xoris`. **Both
sides have exactly two `xoris` instructions.** Whatever you saw was positional
rather than a count difference — worth knowing, because it means that particular
2-instruction explanation was never real.

### A correction — to something I told YOU last round

I gave you this rule:

> Different length -> content. Same length, different bytes -> registers.

**The first half is right and the second half was too absolute, and your own
round-18 work is what shows it.** You wrote that `createObjList`'s frame is
`0x60` with `_savegpr_17` against our `0x40` with `_savegpr_19`, and that the
difference "accounts for 8 instructions". That is correct, and it means register
*pressure* CAN change instruction count — through the prologue and epilogue,
because saving more registers costs more instructions and a bigger frame.

So the refined rule, which you should use from now on:

> **Different length → content, OR a different number of saved registers.
> Check the `_savegpr`/`_restgpr` level and the `stwu` frame size FIRST.**
> If those match and the length still differs, it is content.
> Register *allocation* — which register holds what — still cannot change the
> count. Register *pressure* — how many are live at once — can, but only via
> the prologue/epilogue.

One thing to fix in your own reasoning: you attributed **15 words** of
`ProcMain`'s gap to "a register allocation cascade". That cannot be right by
either version of the rule — a cascade that only renames registers is free. The
`_savegpr` level there is `_savegpr_26` (retail) against `_savegpr_25` (yours),
which is **one** register, not fifteen. So there is still unexplained length in
`ProcMain`. Leave it — it may well be the `mVec3_c` answer — but do not file it
as register allocation.

---

## What changed in `AGENT_CONTEXT.md` today — read these before starting

Two new levers, both discovered and verified this session, both likely to apply
to your remaining functions:

- **Lever 11** — a float product of a member and a literal. The `fmuls` operand
  order in the target *tells you the source shape*: this was validated against
  115 samples of already-matching code with no exceptions, so treat it as a
  lookup, not a guess.
- **Lever 12** — the residual where the instructions are right but the FP
  register *numbers* are rotated. This is evaluation order, and there is a
  one-line source fix.

Also newly recorded as **proven negatives** — do not spend a round on any of
them: commutative operand order; naming a float constant in any foldable form;
translation-unit or literal-pool ordering; compiler flags (~145 variants tested);
and `fmuls` slot choice when both operands are already live.

**One tooling change that affects you directly:** `harness.canonicalise` had
three bugs that made it report UNEQUAL for functions whose every byte matched —
it was not stripping quotes around pool symbols, did not know about `sbss`
sections, and did not handle mangled-versus-placeholder `bl` targets. All three
are fixed. Your round-18 table lists `initialize` as *"DIFFER (symbol names
only)"*. **Re-check it — it may already have been a match.**

---

## Round 19 — four items, all inside `d_bg_actor_mng`

Stay in your own unit. Do not touch `wip/fix_bigtwo/**` or
`scratch/gemini_round16/**`; both are actively being worked.

### 1. `initialize` (66/66, "symbol names only")

Re-measure with the fixed canonicaliser. If it now matches, say so and move on —
that is a free function. If it still differs, the symbol names themselves are
the finding: report exactly which symbols differ and what they resolve to.

### 2. `execute` (16/16, "register swap")

Same length, different bytes, and no `_savegpr` difference in a 16-word
function — so this is genuinely a register question, and it is the exact shape
**lever 12** was written for. Apply it. This is a 16-word function, so if lever
12 does not close it you should be able to characterise the residual completely,
instruction by instruction.

### 3. `createObjList` (116 target / 107 draft, −9)

You have already localised this correctly: frame `0x60` vs `0x40`,
`_savegpr_17` vs `_savegpr_19`. Retail keeps **two more values live in
registers** across the loop nest than we do.

That is a source-shape question, not an allocator mystery. Retail holds
`bg + 0x90000` in a register across the whole nest; find what source shape makes
MWCC do that. Things worth trying: hoisting the base pointer into a named local
outside the loops; computing the offset once rather than per iteration; keeping
`x0`/`y0` live rather than recomputing; and the `extrwi` versus `srwi` selection
you already spotted, which is a hint about what type the value has.

Measure the `_savegpr` level and frame size after each attempt — those two
numbers tell you immediately whether you moved the thing that matters, without
reading the diff.

---

### 4. `ProcMain` (179 target / 162 draft, −17) — the recompute-vs-cache gap

Covered above. Retail recomputes the `obj` pointer 11 times where we cache it
and `mr` it 9 times. Change how the `obj` local is scoped and reused; do not
touch the struct copy, and do not go looking for the `xoris` again.

Sanity check before you start: retail's `_savegpr` level here is `_savegpr_26`
against your `_savegpr_25` — **one** register, so at most a word or two of the
gap is prologue. The rest is the pointer pattern.

---

## Reporting

Per function: **target length, draft length, `_savegpr` level and frame size for
both, then match status.** The length and the save level come first, before any
instruction analysis — that ordering is the point.

Report negatives precisely. "Lever 12 did not close `execute`, and here is the
complete 16-instruction diff" is a good result. Do not relabel a length gap as
register allocation, and if you find yourself about to write "cascade", check the
`_savegpr` level instead and report that number.

If you hit another blocked hypothesis that needs a shared header or a build
config change, do exactly what you did last round: state it plainly, say what
test would settle it, and stop. That worked.
