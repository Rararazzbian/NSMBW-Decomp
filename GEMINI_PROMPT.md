# Work order — round 30

**Read `AGENT_CONTEXT.md` first.**

Write results to **`GEMINI_RESPONSE.md`** (overwrite it).

---

## Round 29 verified, and you were right where my tool was wrong

I re-scored your fresh object against the authoritative target listing
(`wip/kokoopa_verify13/`, the three `auto_*_text.txt` files concatenated) with
my own differ, and it initially reported **six** unmatched functions to your
three. Three of the six were my differ's fault:

- `executeState_LandOn`, `initializeState_ShellAtk_St` and
  `executeState_ShellAtk_St` differ only in `...bss.0` versus a retail-named
  datum, and in pooled-constant indices that desynchronise once one such
  reference is present. My canonicaliser renumbered pool symbols per side
  independently, so a single unmatched name offset every index after it.
- `__sinit` came out at 5 diffs, all of the form
  `bl "baseID_Jump<...>_800A8720"` against `bl "baseID_Jump<...>"`. That
  trailing `_800A8720` is dtk's disambiguating address suffix. **You said all
  nine were naming artifacts and you were right.**

`fndiff.py` is fixed: relocated operands are now compared pairwise and only
called artifacts when at least one side's symbol is generated, and the address
suffix is stripped. Re-scored, my count agrees with yours exactly —
**three unmatched functions: `setQuakeDead`, `initializeState_Jump`,
`initializeState_BigJump`.** 248/251 stands, independently confirmed.

The manifest is thorough and the `d_actor_manager.hpp` flag — that it *modifies*
a padding member rather than only adding — is precisely the distinction I asked
for and the one that decides whether I can promote safely.

---

## The important correction: variant B is a dead end, and its lower score is why

You tested five variants and picked **Variant B, the direct guard `if`**, on 50
canonical diffs against Variant A's 83. Variant B cannot ever match. Here is the
retail listing:

    lwz   r3, 0x770(r30)
    cmpwi r3, 0x0
    bne   .L_800A9B28
    li    r3, 0x0                 <-- null materialised into r3
    b     .L_800A9B2C             <-- and branched over the call
    .L_800A9B28:
    bl    searchBaseByID__10fManager_cF9fBaseID_e
    .L_800A9B2C:
    cmpwi r3, 0x0                 <-- ONE test, on the merged value
    beq   .L_800A9B38
    bl    deleteRequest__7fBase_cFv

`li r3, 0x0` followed by an unconditional `b` into a **single shared** `cmpwi
r3, 0x0` is a conditional-expression merge. **This is the ternary**, and it is
your Variant A's structure, not Variant B's. A guard `if` cannot produce it: it
would test `0x770`, branch straight to the end, call, test `r3` at a *different*
label, and emit no `li r3,0` and no `b` at all. Variant B scores better and is
structurally incapable of matching.

`AGENT_CONTEXT.md` already carries *"When the FRAME SIZE differs, diff count is
not a progress metric."* This is the sibling case: **when the two candidates
have different control-flow shapes, the diff count cannot choose between them —
the target listing chooses.** Read the branch structure before you rank.

## So the real problem is exactly the one you diagnosed in round 27

Keep the ternary. Kill the `r29` hoist.

Retail materialises `0` **twice**, and that is the whole point:

    li  r0, 0x0            <-- before the call, for the two sth stores
    sth r0, 0x792(r30)
    sth r0, 0x790(r30)
    ... bl UnKnownScoreSet__11dScoreMng_cFP8dActor_cUlff
    li  r3, 0x0            <-- after the call, for the null pointer

Your draft materialises it **once** and keeps it alive across
`UnKnownScoreSet` in callee-saved `r29`, which is what costs you `r29`, the
`0x40` frame, and the extra words. **MWCC is common-subexpressioning the
`s16` zero and the null pointer into one value.** Give it two values it cannot
unify. Things worth trying, cheapest first:

- type the null arm as a pointer explicitly — `(fBase_c *)NULL`, or a
  `fBase_c *` local initialised in both arms;
- write the two `sth` stores against a differently-typed zero;
- move the ternary's textual position relative to the stores;
- split the ternary into an `if/else` that assigns the same `fBase_c *` local in
  both arms — same control-flow shape as the ternary, different tree for CSE.

**Variant A is 88 words against 85 with the right shape. That is three words and
one register from a closed function**, and closing it takes the unit to
249/251 with only two bounded negatives left.

---

## Completeness

All three items are in scope. If a task's details seem incomplete, that is not
permission to skip it — make the most reasonable interpretation, do it, and
state your assumption. If something you need is missing, substitute the nearest
valid thing and say what you substituted. A task is only "not done" if you ran
out of budget, and then name it in a `NOT REACHED` list at the end.

## Round 30 — order of work

### 1. `setQuakeDead`, from Variant A only

Ternary shape throughout. Do not resurrect the guard `if`, and do not rank
variants by diff count alone — for each one, report **the frame size, the
non-volatile set, and whether `li r3,0` + `b` into a shared `cmpwi` is present**.
That last column is the one that decides whether a variant is on the right road.

**Acceptance:** ≥4 variants, all ternary-shaped, with those three columns and
the `fndiff.py` count. If none closes, say which of the four levers above you
ruled out and what each produced.

### 2. Fold whatever wins, and re-verify the whole object

Your best round-29 variant **is not in the object on disk** — the committed
`d_enemy_toride_kokoopa.o` still has the 88-word baseline. Whatever you land
this round, fold it into the canonical source, rebuild, and run
`fndiff.py --all` plus `poolcheck.py`.

**Acceptance:** the `--all` output, and confirmation that the 248 still match.
If folding loses one, say so and hand me the pieces separately.

### 3. The two hunks I need decided before I can promote

Your manifest is good enough to act on except in two places. Both are yes/no
questions and both block the landing:

- **`d_actor_manager.hpp`** — you split `mPad1[0x28]` into
  `mPad1[0x18]` + `daBossDemo_c *mpBossDemo` + `mPad1b[0xC]`. Size-neutral, but
  it changes a declaration other units already compile against. **Give me the
  retail evidence that `mpBossDemo` sits at that offset** — the symbol, the
  access in the retail listing, the arithmetic. If the evidence is only "my TU
  needs a pointer there", say so and I will keep the pad and cast at the use
  site instead.
- **`d_cc.hpp` `setKind`** — you say it is unused in this TU. Then it should not
  be in the promotion. Confirm it is unused and I will drop it, or show me the
  call site.

**Acceptance:** a direct answer to each, with evidence or an explicit "no
evidence, drop it".

`initializeState_Jump` / `BigJump` remain closed as bounded negatives.

---

## Constraints

Continue in `scratch/gemini_round24/`. Do not touch `wip/**`, `source/**`,
`include/**`, `slices/`, `syms.txt`, `configure.py`, `tools/**`, `QWEN_*`,
`CODEX_HANDOFF.md`, or `HANDOFF.md`. **Do not run `ninja`, `configure.py`,
`progress.py` or `land.py`**, and do not attempt the landing. I do the header
promotion and the five-binary verification.

---

## Reporting

- `setQuakeDead` variant table: file, ternary-shape yes/no, words, frame,
  non-volatile set, `fndiff.py` count.
- `fndiff.py --all` after folding, pasted whole.
- **GAINED and LOST by name**, including the artifact-matched pair by name.
- Answers to the two manifest questions.
- `poolcheck.py` output.
- `NOT REACHED`, if anything.
