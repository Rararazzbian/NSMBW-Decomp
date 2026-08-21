# Work order — round 20

**Read `AGENT_CONTEXT.md` first.** It changed substantially today: **one of its
rules was WRONG in a way that directly blocked you last round**, and the
correction hands you a working technique for `execute`. See below.

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 19 verified. You confirmed my diagnosis, and I owe you a correction.

**`ProcMain`: your fix worked and it worked for the reason predicted.** Removing
the cached `obj` pointer took the draft 162 → 170 words and moved `_savegpr_25`
→ `_savegpr_26`, matching retail exactly. That is the recompute-versus-cache
mechanism confirmed by measurement. Good.

**Now the correction, and it is mine, not yours.**

`AGENT_CONTEXT.md` told you:

> Declaration order does NOT drive MWCC's saved-register assignment. Treat a pure
> register-permutation residual as not source-addressable.

You cited that to close out `execute` as unfixable. **The second sentence was
wrong.** The measurement behind it was done on three hoisted base *pointers* —
GPRs — and I generalised it to all registers without testing. It does not
transfer to floating-point registers.

Measured today, on `d_line_mng`:

> **Callee-saved FP registers `f31…f28` are handed out in DECLARATION order,
> while the instruction schedule follows ASSIGNMENT order.** Retail sometimes
> needs those two to disagree, and you decouple them by splitting the
> declaration from the assignment.

A 128-word function whose residual looked sprawling turned out to be exactly one
FP register pair swapped, fixed purely from source. The note has been split:
believe it for GPRs, disbelieve it for FPRs.

---

## Round 20 — three items

### 1. `execute` (16/16, FP register swap) — now addressable, do this first

Your own measurement is the whole brief:

```
want: lfs f0, 0x44(r4)    got: lfs f3, 0x44(r4)    # mSize.y
want: lfs f3, 0x3c(r4)    got: lfs f0, 0x3c(r4)    # mPos.y
want: fsubs f0, f3, f0    got: fsubs f0, f0, f3
```

Retail puts **`mSize.y` in the lower-numbered register**, meaning `mSize.y` is
*declared* first. You tried def-points on the value and on temps, and both made
it worse or did nothing — that is the expected outcome, because a def-point is
the wrong tool here. Try the **declare-early / assign-late split**:

```cpp
f32 sizeY;                    // declaration fixes WHICH register
f32 posY = param->mPos.y;
sizeY = param->mSize.y;       // assignment fixes WHERE it is computed
mMin.y = posY - sizeY;
```

Vary which of the two is declared first, and note that a combined
`f32 sizeY = ...;` is NOT the same thing — collapsing declaration and assignment
is precisely what removes your control. On `d_line_mng` the combined form fixed
the registers but moved the computation into the prologue and cost a word; the
split satisfied both at once.

This is a 16-word function, so you can characterise it completely. If the split
does not work, give me the full 16-instruction diff for the best variant and say
which register each of the four values landed in — that is a clean negative and
it bounds the rule.

### 2. `ProcMain` (179 target / 170 draft, −9) — your explanation cannot be right

You attributed the remaining 9 words to the `mVec3_c` struct copy. **Check that
against your own sentence:** you wrote that both forms are 12 instructions, and
that the difference is instruction *selection* only.

> **If the instruction count is identical either way, the struct copy cannot
> account for a LENGTH gap of 9 words.** It is arithmetically impossible.

So there are still 9 words of unexplained *content* in `ProcMain`, and the copy
is a red herring for this particular gap — exactly as I told you last round when
rejecting the header change. Please find the real content difference. The
`_savegpr` level and frame now match on both sides, so it is not prologue. Look
for the same class of thing that the pointer cache turned out to be: a value
retail recomputes, an extra bounds test, or a helper inlined on one side only.

### 3. `createObjList` (116 target / 111 draft, −5) — this one IS a GPR case

Here the old note still stands, so do not spend the round reordering
declarations. Retail runs `_savegpr_17` with frame `0x60`; you run `_savegpr_19`
with `0x50`. Retail keeps **two more values live** across the loop nest.

You correctly observed the compiler already holds `bg + 0x90000` in a register —
just a higher-numbered one. That means the question is not "make it cache the
base pointer", it is **"give it two more things that must survive the whole loop
nest"**. Candidates: keeping `x0`/`y0` live rather than recomputing them per
iteration; hoisting a second derived pointer; holding the loop bounds in
variables that are read after the nest rather than only inside it.

The `clrlslwi` versus `slwi`+`clrlwi` selection you spotted is a type hint — a
value that is `u16` in one form and `int` in the other. Worth chasing, since your
`(u32)`→`(int)` change already bought you 4 words.

Measure `_savegpr` level and frame size after each attempt. Those two numbers
tell you instantly whether you moved the thing that matters.

### Not a task: `initialize`

Your diagnosis is right — `l_object_name` versus `SYM0` is a naming gap in an
unsplit data section, and every byte matches. **Treat it as a match.** I will fix
the tooling; do not spend time on it.

---

## Reporting

Per function: target length, draft length, `_savegpr` level and frame size for
both, then match status. Length and save level first, before any instruction
analysis.

State negatives precisely. And apply the arithmetic check from item 2 generally
before attributing any gap: **if a proposed cause does not change the instruction
count, it cannot explain a length difference.** That one check would have caught
the `ProcMain` attribution and the `xoris` claim before either was written down.
