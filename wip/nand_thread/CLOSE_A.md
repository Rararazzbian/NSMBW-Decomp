# Closer A report — `save()`, `load()`

Work done in `wip/nand_thread/scratch/closer_a/`. Compiled and diffed
exclusively with `tools/auto_decomp/harness.py`'s `compile_draft` / `disasm` /
`extract` / `diff_fn` — no hand-rolled compiler line, no hand-rolled differ.
Extracted by ADDRESS, `instruction_count * 4` checked against
`bin/dtk/wiimj2d_symbols.txt` for both functions (`save` `0x17C` = 95
instructions * 4; `load` `0x284` = 161 instructions * 4 — both confirmed).
Started from Batch 3's `save()` draft and Batch 4's `load()` draft, per
instructions; did not start from scratch.

## Result summary

| Function | Address | Target size | Before (this session's start) | After |
|---|---|---|---|---|
| `save()` | `0x800CF200` | `0x17C` (95 instr) | 86/95, **zero** `cntlzw` occurrences | **97/95** — every `cntlzw`/`srwi.` occurrence now present and correctly shaped, register allocation self-corrected, 2 residual extra instructions, fully characterised below |
| `load()` | `0x800CF680` | `0x284` (161 instr) | 149/161, **zero** `cntlzw` occurrences | **162/161** — same story, 1 residual net extra instruction |

Neither function is MATCH. Both are far closer than either prior batch got,
and the remaining gap is small, structural, and precisely explained — not a
mystery, not a guess.

**No regression**: the full 15-function TU (everything Batches 1–4 already
closed, recompiled together with the header change this report proposes) was
re-verified end to end. All 11 previously-MATCHING functions
(`run`, `create`, `setNandError`, `getSaveData`, `cmdExistCheck`,
`existCheck`, `cmdSpaceCheck`, `createBanner`, `cmdLoad`, `checkCRC`,
`cmdDeleteFile`, `deleteFile`) still match byte-for-byte with the header
change applied. `spaceCheck`'s pre-existing, unrelated `r3`/`r4`
register-allocation wall (Batch 2's, not mine) is unchanged — same 3-line
diff as before, not touched by anything here.

## The central hypothesis: HELD, but only with one correction

**The hypothesis as stated — "the target stores its `mError` tests into
`bool` locals before branching, rather than branching on the comparison
directly" — is TRUE, but incomplete on its own.** Batch 3 and Batch 4 both
tried exactly this shape (`bool ok = (mError == 0); if (ok)`) and it produced
plain `cmpwi` for them, which reads like a refutation. It is not: the missing
ingredient is that **`mError` itself must be `volatile`.**

### The A/B that settles it

Four single-line-different compiles, all through the real class layout, all
via `harness.compile_draft`/`disasm` (not hand-assembled):

| # | `mError` type | Source shape | Result |
|---|---|---|---|
| A | plain `int` | `if (t->mError == 0) return 5; return 1;` | `cmpwi` |
| B | plain `int` | `bool ok = (t->mError == 0); if (ok) return 5; return 1;` | `cmpwi` (identical to A) |
| C | `volatile int` | `if (t->mError == 0) return 5; return 1;` | `cmpwi` (identical to A) |
| D | `volatile int` | `bool ok = (t->mError == 0); if (ok) return 5; return 1;` | **`cntlzw r0,r0` / `srwi. r0,r0,5`** — byte-identical to every non-chained occurrence in the target |

Row D is the only one of the four that produces the target's idiom, and it
takes **both** ingredients together — B (bool-storage alone, `mError`
plain) and C (`volatile` alone, direct compare) each independently give
plain `cmpwi`, matching what Batches 3 and 4 already reported and ruling out
either ingredient in isolation.

This also reproduces, unprompted, the exact mechanism Batch 2's
`CMD_SHAPE.md` found for `OSTryLockMutex`: that lever worked because
`OSTryLockMutex`'s return is an **opaque, arbitrary-int-typed** value the
optimizer cannot fold through — an external call's return, unprovable at
compile time. `mError` is an ordinary field read, which MWCC folds through
freely (proving the whole bool-materialise-then-branch idiom reduces to a
bare compare) **unless** it is marked `volatile`, at which point it becomes
equally opaque to the optimizer for the same reason. `volatile` is also the
semantically correct qualifier here regardless of this discovery:
`dNandThread_c` is a background thread and `mError` is genuinely read by
code that did not just write it in program order.

**General rule for the handoff**: *MWCC's `cntlzw`+`srwi.` 0/1-materialisation
idiom requires TWO conditions together: (1) the tested value is stored into a
real `bool` before branching, not compared inline, and (2) the tested
operand is opaque to the optimizer — either an external call's return of a
non-`bool` type, or a `volatile`-qualified read. Given only one of the two,
MWCC proves the materialisation is redundant and emits a plain `cmpwi`.*

**Proposed header change** (shadow-tested only, not applied to the real
header — see `wip/nand_thread/scratch/closer_a/shadow_include/`):
```cpp
volatile int mError; ///< [0x78] The last NAND error code.
```
Verified to cause **no regression** in any of the 11 already-matching
functions in the TU (full recompile-and-diff done, see above).

## Second finding: both prior batches had the `!= 6` / `== 6` polarity backwards

Batch 3's `save()` and Batch 4's `load()` both wrote
`if (mError != 6) { cancel(); }`. Reading the raw target bytes directly
(not from either batch's prose) at all five occurrences of this pattern —
`0x800CF288` and `0x800CF2EC` in `save()`, `0x800CF6D0`, `0x800CF724`, and
`0x800CF7D0` in `load()` — every one has the identical shape:

```
subi r0, r3, 0x6     ; r0 = mError - 6
cntlzw r0, r0
srwi. r0, r0, 5       ; r0 = (mError == 6) ? 1 : 0
beq   <skip-cancel>   ; branches (skips NANDSimpleSafeCancel) when mError != 6
<fallthrough>: bl NANDSimpleSafeCancel
```

`beq` branches to the *no-cancel* path precisely when `mError != 6`; the
`NANDSimpleSafeCancel` call only executes on fallthrough, i.e. when
`mError == 6`. This is **`if (mError == 6) cancel();`**, not
`if (mError != 6)`. Confirmed independently at all 5 occurrences (2 in
`save()`, 3 in `load()`), all identical. Both prior batches got the same
polarity wrong in the same direction, likely because their drafts never
reached a state where this specific branch's target could be checked against
real bytes (the diff was already broken from the first `cntlzw` divergence
onward, so everything past that point was unverified guesswork dressed as
a structural read).

This also happens to be diagnostic of the central-hypothesis mechanism: `!=`
against a `volatile` operand materialises via the *truthy* family
(`neg`/`or`/`srwi,31`), while `==` materialises via the *falsy* family
(`cntlzw`/`srwi,5`). Target uses the falsy family for every one of these
five sub-checks, which is only consistent with `== 6` in the source — the
polarity fix and the materialisation-family observation independently
confirm each other.

## What remains open: chained checks sharing one load

This is the actual, precisely-characterised residual. It accounts for
100% of both functions' remaining diffs (verified with a proper
`difflib.SequenceMatcher` opcode diff, not just a positional line diff —
see `wip/nand_thread/scratch/closer_a/final_diffs.txt`).

At every occurrence where the target tests `mError == 0` and then, only on
the failure path, immediately tests `mError == 6`, **target loads `mError`
exactly once** and reuses that same register for both materialisations:

```
lwz    r3, 0x78(r31)     ; ONE load
cntlzw r0, r3             ; test #1: == 0
srwi.  r0, r0, 5
bne    <ok>
subi   r0, r3, 0x6        ; test #2 reuses r3 -- no second load
cntlzw r0, r0
srwi.  r0, r0, 5
beq    <no-cancel>
```

I could not find a C++ shape that reproduces this. Systematically tried, all
via direct A/B compile against the volatile-qualified header, each on both
an isolated probe and inside the full `save()`/`load()` body:

1. **Two separate `bool`-materialised reads** (`bool ok = (mError==0); if
   (!ok) { bool eq6 = (mError==6); ... }`) — correctly produces `cntlzw`
   **twice**, but each is preceded by its own fresh `lwz` (volatile forces a
   real access per textual read, confirmed — MWCC does not merge two
   volatile reads even with literally nothing intervening but the branch
   itself). This is the shape used in the final draft; it is right about the
   instruction *shape* and wrong only about the extra load.
2. **Two separate direct comparisons, no `bool`** (`if (mError != 0) { if
   (mError == 6) ... } }`) on a `volatile` field — confirms MWCC still does
   *not* merge the two loads (fresh `lwz` for the second test), and, without
   the `bool` intermediary, the second test degrades to `cmpwi` anyway (no
   materialisation at all). Two independent negatives from one probe.
3. **Capture into a plain local first**, `int e = mError; bool ok =
   (e==0);` — the capture itself is one volatile read (satisfies C++
   semantics), but comparisons on the plain-typed copy `e` are no longer
   opaque to the optimizer and collapse straight back to `cmpwi`, on the
   *first* test too, not just the second. Tried `long`, and a
   `volatile int &` reference alias — identical result each time.
4. **A `switch (mError) { case 0: ...; case 6: ...; }`** — this *does*
   naturally reuse one load across both case tests (confirmed: single
   `lwz`, two `cmpwi`s against the same register), because switch-lowering
   is an ordinary CSE-friendly construct. But it gives `cmpwi` for both
   cases, never `cntlzw` — switch lowering doesn't route through the
   bool-materialisation path at all.

Every one of these is a genuine, compiled, checked result, not a guess. The
pattern that would need to exist — one volatile-forced `cntlzw`
materialisation immediately followed by a second materialisation of a
*different* constant reusing the same already-loaded register — was not
reproduced by any combination of: bool-storage, direct comparison, plain-int
capture, reference-alias capture, or switch-lowering. I'm reporting this as
a genuine open question rather than a guess. Two prior batches (independently,
on `save()` and `load()` respectively) hit the same wall from the opposite
direction — they had the register-reuse "for free" (via plain, non-volatile
`cmpwi` compares that naturally CSE) but could not get `cntlzw` at all. This
report gets `cntlzw` everywhere by paying for it with the extra reload; the
combination of both properties on the same occurrence remains unexplained.

**Precise cost**: `save()` has 2 such chained occurrences, `load()` has 3.
Each costs exactly one extra `lwz` in the current draft. `save()`'s net
delta is +2 (97 vs 95); `load()`'s net delta is +1 (162 vs 161) because one
of the three chains sits at a point where the tail-call branch orientation
for the next `if` (`if (ok5) return createBanner();`, in `save()` — see next
paragraph) also happens to save exactly one instruction relative to my
draft's layout, partially offsetting the count without being the same issue.

**A second, smaller, separate residual in `save()` only**: the final check
(`setNandError(NANDSimpleSafeClose(...)); if (ok5) return createBanner();
return 1;`) is structurally identical to what target does, and is not a
chained ==0/==6 pair, but the compiler chose the opposite branch orientation
(`beq` skip-to-a-relocated-fail-block vs. target's `bne` branch-to-body with
a short inline fail path), costing 2 more instructions. This is the same
class of branch-layout freedom seen early in this investigation for the very
first `mError` check (which *was* resolved by switching from
`if (ok) { body } return 1;` to the guard-clause form `if (!ok) return 1;
body;` — see the diff history in
`wip/nand_thread/scratch/closer_a/save_test.cpp`'s iteration). The same fix
was already applied to every check that has this shape; this last one, for
reasons not yet isolated, did not respond to it. Not chased further given
time already spent; flagging for whoever picks this up next.

## Final source

`wip/nand_thread/scratch/closer_a/d_nand_thread.cpp` (the full 15-function
TU, for regression verification) and `wip/nand_thread/scratch/closer_a/{save_test,load_test}.cpp`
(isolated per-function drafts used for iteration). The `save()`/`load()`
bodies are identical in both.

```cpp
// Header requires (shadow-tested, see wip/nand_thread/scratch/closer_a/shadow_include/):
//   volatile int mError;   // was: int mError;

s32 dNandThread_c::save() {
    setNandError(NANDCreate(sc_GAME_FILE, 0x3c, 0));
    bool ok1 = (mError == 0);
    if (!ok1) {
        return 1;
    }

    NANDFileInfo info;
    setNandError(NANDSimpleSafeOpen(sc_GAME_FILE, &info, 2, l_safeCopyBuf, sizeof(l_safeCopyBuf)));
    bool ok2 = (mError == 0);
    if (!ok2) {
        bool eq6 = (mError == 6);
        if (eq6)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    s32 written = NANDWrite(&info, l_tmpSave, sizeof(l_tmpSave));
    if (written < 0) {
        setNandError(written);
        bool ok3 = (mError == 0);
        if (!ok3) {
            bool eq6b = (mError == 6);
            if (eq6b) {
                setNandError(NANDSimpleSafeCancel(&info));
                bool ok4 = (mError == 0);
                if (ok4)
                    return 2;
            }
            return 1;
        }
    }
    setNandError(NANDSimpleSafeClose(&info));
    bool ok5 = (mError == 0);
    if (ok5)
        return createBanner();
    return 1;
}

s32 dNandThread_c::load() {
    NANDFileInfo info;
    setNandError(NANDSimpleSafeOpen(sc_GAME_FILE, &info, 1, l_safeCopyBuf, sizeof(l_safeCopyBuf)));
    bool ok1 = (mError == 0);
    if (!ok1) {
        bool eq6a = (mError == 6);
        if (eq6a)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    u32 length;
    setNandError(NANDGetLength(&info, &length));
    bool ok2 = (mError == 0);
    if (!ok2) {
        bool eq6b = (mError == 6);
        if (eq6b)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    if (length != sizeof(l_tmpSave)) {
        setNandError(NANDSimpleSafeClose(&info));
        bool ok3 = (mError == 0);
        if (ok3) {
            mError = 6;
        }
        return 1;
    }

    s32 written = NANDRead(&info, l_tmpSave, sizeof(l_tmpSave));
    if (written < 0) {
        setNandError(written);
        bool ok4 = (mError == 0);
        if (!ok4) {
            bool eq6c = (mError == 6);
            if (eq6c) {
                setNandError(NANDSimpleSafeCancel(&info));
                bool ok5 = (mError == 0);
                if (ok5)
                    return 2;
            }
            return 1;
        }
    }

    setNandError(NANDSimpleSafeClose(&info));
    bool ok6 = (mError == 0);
    if (!ok6) {
        return 1;
    }

    if (l_tmpSave[0] != "SMNP"[0] || l_tmpSave[1] != "SMNP"[1] ||
        l_tmpSave[2] != "SMNP"[2]) {
        mError = 6;
        return 1;
    }
    if (l_tmpSave[3] != "SMNP"[3]) {
        mError = 6;
        return 1;
    }

    if (!checkCRC()) {
        mError = 6;
        return 1;
    }
    return 0;
}
```

Notes on things that fell out correctly as a side effect of the above,
verified not asserted:

- **Register allocation self-corrected.** Both prior batches reported `this`
  (`r31`) and `sc_GAME_FILE`'s address (`r30`) swapped relative to target.
  Once the checks above were restructured into guard-clause form with the
  `volatile`+`bool` lever applied, the register allocator picked the
  target's exact assignment on its own — this was never touched directly
  and no longer differs (confirmed in the diffs above: line 6–9 in both
  functions match modulo the pool-symbol filename artifact).
- Guard-clause structuring (`if (!ok) return N;` followed by unindented
  continuation, rather than `if (ok) { ... } return N;`) was necessary for
  the branch orientation to come out right on the *first* check of each
  function (`ok1` in both `save()` and `load()`) — this alone fixed a
  `bne`-vs-`beq` mismatch that appeared even before the register-reuse
  problem was in scope. This generalises Batch 2's `cmdExistCheck` shape
  (`if (locked) {...} return false;`, no guard clause) — that shape works
  there because the true-branch body is short; here the true-branch body is
  the entire rest of the function, and MWCC's choice of which branch to
  place inline vs. out-of-line depends on this, not on some fixed rule for
  "if you get a bool from an opaque source."

## Files

- `wip/nand_thread/scratch/closer_a/d_nand_thread.cpp` — full 15-function
  regression TU (compiles and diffs all functions in the unit against
  `target_raw.txt`, confirms no regression from the header change)
- `wip/nand_thread/scratch/closer_a/save_test.cpp`,
  `wip/nand_thread/scratch/closer_a/load_test.cpp` — isolated per-function
  drafts
- `wip/nand_thread/scratch/closer_a/shadow_include/game/bases/d_nand_thread.hpp`
  — shadow header with `volatile int mError;` (only delta from the real
  header at time of writing, besides an unofficial `cmdSave` declaration
  needed only so this regression harness compiles standalone)
- `wip/nand_thread/scratch/closer_a/final_diffs.txt` — full instruction-level
  diff for both functions against target, this session's final state
- `wip/nand_thread/scratch/closer_a/probe.cpp` — the isolated A/B/C/D/…
  probes used to establish the central-hypothesis proof and rule out the
  register-sharing shapes, kept for anyone who wants to re-run or extend them
