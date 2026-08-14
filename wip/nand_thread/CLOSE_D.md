# Closer D report -- `save()`, `load()`

**Baseline note (in response to the lead's merge-TU correction):** every
number in this report was measured against the real, landed
`include/game/bases/d_nand_thread.hpp` (plain `int mError`) from the start --
`wip/nand_thread/scratch/closer_d/` never contains a `shadow_include/`
directory, and `check.py`/`probe.py` call `harness.compile_draft(src, obj)`
with no `extra_inc`, so only the project's real `include/` is ever searched.
This report's `save()`/`load()` source is **not** CLOSE_A's bare source
carried forward (which, recompiled against the real header with no
`volatile` anywhere, is exactly the lead's corrected short baseline --
confirmed by inspecting `wip/nand_thread/scratch/merge_lead/d_nand_thread.cpp`,
whose `save()`/`load()` use bare `bool ok1 = (mError == 0);` with no cast and
no header change, which is why it comes up short). This report's source
instead casts the read at each site that needs it, `*(volatile int
*)&mError`, which is what produces this report's **97/95 and 162/161 (long,
not short)** against the same real header -- see "The central finding"
below. The `spaceCheck` residual reported here (37/37, target loads `answer`
into `r3` where the draft uses `r4`) is byte-identical to what the lead's
independent merge found, which cross-checks that this session's compiler
environment and header resolution match the lead's.

Work done in `wip/nand_thread/scratch/closer_d/`. Compiled and diffed
exclusively with `tools/auto_decomp/harness.py`'s `compile_draft` / `disasm` /
`extract` / `diff_fn` (driven from small Python scripts, `check.py` and
`probe.py`, that import `harness` and call nothing else) -- no hand-rolled
compiler line, no hand-rolled differ. Extracted by ADDRESS and
`instruction_count * 4` checked against `bin/dtk/wiimj2d_symbols.txt` for both
targets (`save` `0x17C` = 95 instructions; `load` `0x284` = 161 instructions).

**No header change of any kind was made or is being proposed.**
`include/game/bases/d_nand_thread.hpp` is untouched, and `mError` stays a
plain `int`. CLOSE_A's `volatile int mError` proposal is refuted by the
prompt's `existCheck` evidence and is **withdrawn**, not resubmitted.

## Status table

| Function | Address | Target | Before (CLOSE_A's end state) | After (this report) |
|---|---|---|---|---|
| `save()` | `0x800CF200` | 95 instr | 97/95 (2 long, two separate residuals) | **97/95 (2 long, ONE residual, fully characterised)** |
| `load()` | `0x800CF680` | 161 instr | 162/161 (1 long) | **162/161 (1 long, same residual, unchanged)** |

Neither function is MATCH. The instruction counts are unchanged from CLOSE_A,
but the *mechanism* is now understood without any header risk, and `save()`'s
second residual (branch-layout orientation on the final check) is fully
resolved -- what's left in both functions is exactly the chained-load
residual, characterised below, and nothing else.

## The central finding: CLOSE_A's mechanism was right; its *location* was wrong

CLOSE_A proved that MWCC's `cntlzw`+`srwi.` 0/1-materialisation idiom needs
two ingredients together: (1) the tested value is stored into a named `bool`
before branching, not compared inline, and (2) the read feeding that `bool` is
**opaque to the optimiser** -- something the compiler cannot algebraically
fold through. They supplied opacity by declaring `mError` `volatile` in the
header. That is refuted by the prompt's evidence: `existCheck()` reads the
same `mError`, right after the same `setNandError()` call, and is already
byte-exact with plain `cmpwi` -- so no property of the field's *declaration*
can be the differentiator, because the header applies to every reader.

**Opacity does not have to come from the field. It can come from the read.**
`*(volatile int *)&mError` at one specific call site is a real, standard
C++ volatile access -- the compiler must treat it as an observable side
effect it cannot reorder, merge, or algebraically simplify away -- without
qualifying the member itself. `existCheck()`'s source is never touched by
this change, so it keeps reading `mError` as a plain field and keeps emitting
`cmpwi`. This is not a trick specific to this codebase; it is what `volatile`
means applied at the narrowest possible scope, and it lets `save()`/`load()`
opt in individually while every other function in the TU is provably
unaffected (their source text does not change).

### The A/B that proves it (all compiled via `harness.compile_draft`/`disasm`, real header, plain `int mError`)

Free functions taking `dNandThread_c *t`, so no header edit was needed even
to run the probes. Full text in `wip/nand_thread/scratch/closer_d/probe.py`.

| # | Read | Source shape | Result |
|---|---|---|---|
| EC | `t->mError` (plain) | `if (mError == 0) { block }` (existCheck's own shape) | `cmpwi` |
| G | `t->mError` (plain) | `if (mError != 0) return 1; ...body...; return 0;` (direct compare, asymmetric guard, no bool) | `cmpwi` |
| F | `t->mError` (plain) | `bool ok = (mError==0); if (!ok) return 1; ...body...; return 0;` (bool storage, asymmetric guard) | `cmpwi` |
| **H** | `*(volatile int*)&t->mError` | `bool ok = (*(volatile int*)&mError==0); if (!ok) return 1; ...body...; return 0;` | **`cntlzw`+`srwi.`+`bne`, byte-identical to target's `0x800CF238` idiom** |
| EC2 | `*(volatile int*)&t->mError` | `if (*(volatile int*)&mError == 0) { block }` (cast, but NO bool storage) | `cmpwi` |
| EC3 | `*(volatile int*)&t->mError` | `bool ok = (*(volatile int*)&mError==0); if (ok) { block }` (cast + bool storage, existCheck's block-fallthrough ARM SHAPE, not save's early-return shape) | **`cntlzw`+`srwi.`** |

Row H is the target idiom, exactly. Row EC3 is the decisive control: it uses
the **same block-with-fallthrough arm shape as `existCheck`**, not the
early-return shape from the prompt's "Lead 1", and it *still* materialises,
as soon as the read is cast-opaque and stored into a bool first. That rules
out arm shape (early-return-of-a-constant vs. block-fallthrough) as the
trigger -- **Lead 1, tested directly, is refuted.** Rows F and G (asymmetric
guard-clause shape, matching save's real structure, but on a *plain* read)
both still give `cmpwi`, confirming shape alone (without opacity) never
triggers it either, in either direction. The two ingredients are: named bool
+ opaque read. Nothing about *where* the branch goes matters.

**Lead 3 (inline predicate accessor) was not pursued further**: rows H/EC3
already reproduce the idiom with a bare cast at the use site, so there is no
need to add an inline accessor to get the same effect, and doing so would add
an unverifiable header claim for no additional explanatory power.

**Lead 4 (`== 6` polarity) is unchanged and correct**: CLOSE_A's proof from
raw branch targets holds regardless of how opacity is obtained. Not
re-derived here; carried forward as-is.

## Second finding: the "second, smaller residual" in `save()` is a separate, now-fixed problem

CLOSE_A flagged that `save()`'s final check (`NANDSimpleSafeClose` guarding
`return createBanner()` vs `return 1`) picked the wrong branch orientation and
said the guard-clause fix that worked everywhere else "did not respond" here.
Retried under this report's lever:

```cpp
// before (matches CLOSE_A's shape, and their earlier finding for it):
bool ok5 = (*(volatile int *)&mError == 0);
if (ok5)
    return createBanner();
return 1;

// after:
bool ok5 = (*(volatile int *)&mError == 0);
if (!ok5)
    return 1;
return createBanner();
```

Compiled instruction-by-instruction diff (`difflib.SequenceMatcher`, exact
opcodes) before this change, at the tail of `save()`:

```
want[84:88]: bne .L(4082000C) / li r3,0x1 / b .L(4800000C) / mr r3,r31 / bl createBanner
 got[86:90]: beq .L(41820010) / mr r3,r31 / bl createBanner / b .L(48000008) / li r3,0x1
```

After the reorder, the generated sequence is **structurally identical** to
target at this point (`bne` branches away to a relocated `mr r3,r31; bl
createBanner` block that falls straight into the epilogue; `li r3,0x1` sits
inline on the fallthrough path) -- confirmed by re-running the same
`SequenceMatcher` diff and finding this span no longer appears in the opcode
list at all. The instruction *count* for `save()` did not change (97 stays
97) because the original mis-oriented form happened to be equal-length (a
`beq`+2-instruction-relocated-tail vs `bne`+3-instruction-inline-tail +
1-instruction-skip-branch, net zero) -- but the earlier report treated this
as an unexplained, separate residual, and it no longer is. **This was a
separate branch-layout problem, not a consequence of Lead 1 or of the
opacity mechanism** -- it responded to source reordering on its own, exactly
the way every *other* guard clause in this TU already does. Why it didn't
respond under CLOSE_A's draft and does here was not chased further (plausibly
some other nearby instruction-count difference shifted MWCC's layout
heuristic across a threshold); it is reported as observed, not modelled.

**`save()`'s entire remaining diff is now exactly the two chained-load
occurrences below and nothing else** -- confirmed by re-running the
instruction-level diff after this fix: every remaining opcode is one of the
two `insert`/`delete` pairs shown in the next section, plus pure
branch-displacement `replace`s that are a direct consequence of those two
(same instruction, shifted target address because earlier code moved).

## What remains open: the chained-load residual (same wall as CLOSE_A, reconfirmed by a different route)

At every point where target tests `mError == 0` then, only on failure,
`mError == 6`, target loads `mError` **once** and reuses the register for
both materialisations:

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

This report's lever needs an opaque **read**, and a volatile read is by
C++ definition an observable operation that cannot be merged with an earlier
one, textually or through an alias. Three independent ways of trying to get a
shared load were tested, matching (and reconfirming, via a different
mechanism) exactly what CLOSE_A already found under the header-volatile
approach:

1. **Two separate cast reads** (what the final source uses): correctly
   produces `cntlzw` **twice**, but each is preceded by its own fresh `lwz`.
   Compiled probe (`wip/nand_thread/scratch/closer_d/probe.py`, "Chain"):
   ```
   lwz r0, 0x78(r3)
   cntlzw r0, r0 / srwi. r0, r0, 5 / bne ...
   lwz r4, 0x78(r3)          <- second load, target has none
   subi r0, r4, 0x6
   cntlzw r0, r0 / srwi. r0, r0, 5 / beq ...
   ```
2. **Capture into a plain local, reuse it for both tests** (`int e =
   *(volatile int*)&mError; bool ok=(e==0); ... bool eq6=(e==6);`): the
   single volatile read is preserved (one `lwz`), but **both** tests
   collapse to `cmpwi` -- the opacity that forces materialisation does not
   survive being copied into an ordinary variable, even though the value was
   obtained through a genuinely volatile access. Compiled probe ("Chain2"):
   ```
   lwz r0, 0x78(r3)
   cmpwi r0, 0x0 / beq ...
   cmpwi r0, 0x6 / bne ...
   ```
   This is the same "capture into a plain local" result CLOSE_A already
   reported under full field volatility -- reconfirmed here under a
   completely different source of opacity, which rules out "it was specific
   to how CLOSE_A's header change worked" as an explanation.
3. **A volatile pointer alias, dereferenced twice** (`volatile int *p =
   &mError; bool ok=(*p==0); ... bool eq6=(*p==6);`): identical result to
   variant 1 -- two loads, both materialise. Compiled probe ("Chain3").
   Confirms the pointer indirection changes nothing; two syntactic volatile
   reads are two accesses regardless of how the address is spelled.

**No third way was found.** Every attempt to make the SECOND test's read
share the first test's already-loaded register loses the `cntlzw`
materialisation on one or both tests; every attempt to keep both tests
materialising costs a second load. This is the same trade-off CLOSE_A
characterised, reached this time from cast-based opacity instead of a header
qualifier, which is independent evidence that the wall is structural to how
MWCC's volatile-forced materialisation interacts with load sharing, not an
artifact of one particular way of asking for it.

**Precise cost, unchanged from CLOSE_A**: `save()` has 2 chained occurrences
(the `ok2`/`eq6` pair, and the `ok3`/`eq6b` pair), each costing exactly one
extra `lwz`, for a net +2 (97 vs 95) now that the branch-orientation residual
is separately fixed. `load()` has 3 chained occurrences (`ok1`/`eq6a`,
`ok2`/`eq6b`, `ok4`/`eq6c`), which would cost +3, but the `ok3` occurrence (a
single, non-chained test, `if (ok3) { mError = 6; } return 1;`) happens to
land where MWCC tail-merges the `li r3,0x1; b <epilogue>` sequence shared by
both its arms into one physical copy instead of two -- a 2-instruction saving
unrelated to this report's lever, present in CLOSE_A's draft too -- bringing
`load()`'s net to +1 (162 vs 161). Tried de-duplicating that merge
deliberately (writing `return 1;` explicitly in both the `if` body and after
it, and separately trying the guard-clause form `if (!ok3) return 1; mError =
6; return 1;`): both made `load()` **worse** (164 instead of 162), confirming
this merge is a favourable accident of the current shape, not something to
fight.

## Regression: every previously-matching function in the TU, re-verified

Full 24-function-class regression compile (`wip/nand_thread/scratch/closer_d/d_nand_thread.cpp`,
compiled and diffed by address against `target_raw.txt` via `check.py`,
which wraps nothing but `harness.compile_draft`/`disasm`/`diff_fn`). Uses the
**real, unmodified header** -- no shadow include directory was needed for
this report, because no header change is proposed.

```
[MATCH] existCheck__13dNandThread_cFv
[MATCH] cmdExistCheck__13dNandThread_cFv
[MATCH] cmdSpaceCheck__13dNandThread_cFv
[DIFF ] spaceCheck__13dNandThread_cFv   <- pre-existing, unrelated (Batch 2's r3/r4
                                            register-allocation wall, per SHARED-BRIEF;
                                            not touched by this report, unchanged size 37/37)
[DIFF ] save__13dNandThread_cFv          97/95, characterised above, only the chain residual
[MATCH] createBanner__13dNandThread_cFv
[MATCH] cmdLoad__13dNandThread_cFv
[DIFF ] load__13dNandThread_cFv          162/161, characterised above, only the chain residual
[MATCH] checkCRC__13dNandThread_cFv
[MATCH] cmdDeleteFile__13dNandThread_cFv
[MATCH] deleteFile__13dNandThread_cFv
[MATCH] run__13dNandThread_cFv
[MATCH] create__13dNandThread_cFPQ23EGG4Heap
[MATCH] setNandError__13dNandThread_cFl
[MATCH] getSaveData__13dNandThread_cFv
```

All 12 of the functions the prompt asked to be explicitly reverified
(`existCheck`, `cmdExistCheck`, `cmdSpaceCheck`, `createBanner`, `cmdLoad`,
`checkCRC`, `cmdDeleteFile`, `deleteFile`, `run`, `create`, `setNandError`,
`getSaveData`) still MATCH byte-for-byte. `spaceCheck` carries the same
pre-existing, unrelated diff Batch 2/CLOSE_A already reported; it is
untouched by anything in this report.

## Variants tried, for the record (do not repeat these)

| # | Shape | Header | Result |
|---|---|---|---|
| EC | `if (mError==0) {block}` | plain | `cmpwi` (sanity baseline, matches existCheck) |
| G | `if (mError!=0) return1; body; return0;` | plain | `cmpwi` (asymmetric guard alone insufficient) |
| F | `bool ok=(mError==0); if(!ok) return1; body; return0;` | plain | `cmpwi` (bool storage alone, on plain field, insufficient) |
| **H** | `bool ok=(*(volatile int*)&mError==0); if(!ok) return1; body; return0;` | plain | **`cntlzw` -- the lever** |
| EC2 | `if (*(volatile int*)&mError==0) {block}` | plain | `cmpwi` (cast alone, no bool storage, insufficient -- confirms both ingredients still needed) |
| EC3 | `bool ok=(*(volatile int*)&mError==0); if(ok) {block}` | plain | `cntlzw` (refutes Lead 1: block-fallthrough shape ALSO materialises once cast+bool are present) |
| Chain | two separate cast reads, chained ok/eq6 | plain | `cntlzw` twice, but 2 loads (residual) |
| Chain2 | one cast read captured into plain `int`, reused | plain | `cmpwi` twice, 1 load (opacity lost on capture) |
| Chain3 | `volatile int *p = &mError;` dereferenced twice | plain | same as Chain -- 2 loads |
| save() ok5 orientation A | `if (ok5) return createBanner(); return 1;` | plain (+cast) | wrong orientation, net-zero-length diff |
| save() ok5 orientation B | `if (!ok5) return 1; return createBanner();` | plain (+cast) | **matches target's orientation exactly** |
| load() ok3 dedup A | `if (ok3) { mError=6; return 1; } return 1;` | plain (+cast) | 164/161, worse -- do not use |
| load() ok3 dedup B | `if (!ok3) return 1; mError=6; return 1;` | plain (+cast) | 164/161, worse -- do not use |

## Final source

`wip/nand_thread/scratch/closer_d/d_nand_thread.cpp` (full regression TU) and
`wip/nand_thread/scratch/closer_d/probe.py` / `probe.cpp` (the A/B probes).

Header requirement: **none.** `include/game/bases/d_nand_thread.hpp` is not
modified and no change to it is proposed.

```cpp
s32 dNandThread_c::save() {
    setNandError(NANDCreate(sc_GAME_FILE, 0x3c, 0));
    bool ok1 = (*(volatile int *)&mError == 0);
    if (!ok1) {
        return 1;
    }

    NANDFileInfo info;
    setNandError(NANDSimpleSafeOpen(sc_GAME_FILE, &info, 2, l_safeCopyBuf, sizeof(l_safeCopyBuf)));
    bool ok2 = (*(volatile int *)&mError == 0);
    if (!ok2) {
        bool eq6 = (*(volatile int *)&mError == 6);
        if (eq6)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    s32 written = NANDWrite(&info, l_tmpSave, sizeof(l_tmpSave));
    if (written < 0) {
        setNandError(written);
        bool ok3 = (*(volatile int *)&mError == 0);
        if (!ok3) {
            bool eq6b = (*(volatile int *)&mError == 6);
            if (eq6b) {
                setNandError(NANDSimpleSafeCancel(&info));
                bool ok4 = (*(volatile int *)&mError == 0);
                if (ok4)
                    return 2;
            }
            return 1;
        }
    }
    setNandError(NANDSimpleSafeClose(&info));
    bool ok5 = (*(volatile int *)&mError == 0);
    if (!ok5)
        return 1;
    return createBanner();
}

s32 dNandThread_c::load() {
    NANDFileInfo info;
    setNandError(NANDSimpleSafeOpen(sc_GAME_FILE, &info, 1, l_safeCopyBuf, sizeof(l_safeCopyBuf)));
    bool ok1 = (*(volatile int *)&mError == 0);
    if (!ok1) {
        bool eq6a = (*(volatile int *)&mError == 6);
        if (eq6a)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    u32 length;
    setNandError(NANDGetLength(&info, &length));
    bool ok2 = (*(volatile int *)&mError == 0);
    if (!ok2) {
        bool eq6b = (*(volatile int *)&mError == 6);
        if (eq6b)
            setNandError(NANDSimpleSafeCancel(&info));
        return 1;
    }

    if (length != sizeof(l_tmpSave)) {
        setNandError(NANDSimpleSafeClose(&info));
        bool ok3 = (*(volatile int *)&mError == 0);
        if (ok3) {
            mError = 6;
        }
        return 1;
    }

    s32 written = NANDRead(&info, l_tmpSave, sizeof(l_tmpSave));
    if (written < 0) {
        setNandError(written);
        bool ok4 = (*(volatile int *)&mError == 0);
        if (!ok4) {
            bool eq6c = (*(volatile int *)&mError == 6);
            if (eq6c) {
                setNandError(NANDSimpleSafeCancel(&info));
                bool ok5 = (*(volatile int *)&mError == 0);
                if (ok5)
                    return 2;
            }
            return 1;
        }
    }

    setNandError(NANDSimpleSafeClose(&info));
    bool ok6 = (*(volatile int *)&mError == 0);
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

Note on style: `*(volatile int *)&mError` is unusual to read repeatedly. If
this lands, it is worth a named private helper (e.g. a one-line
`int readErrorVolatile() const { return *(volatile int*)&mError; }`, NOT
declared `inline` in a way that changes anything -- it would need its own
A/B verification that inlining it produces byte-identical code to the manual
cast at every call site before being trusted) purely for readability. Not
done here because the prompt's hard rule is not to add anything to the shared
header without a proposal, and a member function needs a declaration; the
raw cast keeps this report's change entirely inside the two `.cpp` function
bodies.

## Addendum: the coordinator's "two live bools from one load" hypothesis, tested exhaustively

The coordinator argued the volatile-based route is structurally exhausted
(agreed -- a volatile read is by definition not CSE-able, so it can produce
`cntlzw` or the shared load but never both) and proposed the untried
alternative: capture `mError` into a **plain** `int` once, then compute
**both** the `==0` and `==6` bools from that one capture *before* branching,
so both are simultaneously live. Tested exhaustively; the mechanism is real
but does not close the residual, for a specific, now-well-evidenced reason:
it only ever explains the *second*, deferred-use test, never the first.

### The result: a genuine new mechanism, but it materialises the wrong half

All probes below use a **completely plain, non-volatile** field access --
no cast, no header change, exactly as requested.

| # | Shape | Result |
|---|---|---|
| DualA | `int e=mError; bool ok=(e==0); bool busy=(e==6); if(!ok){ if(busy){...} return1;} ...` (both declared eagerly, before the branch, nested-if control flow) | `ok` -> plain `cmpwi`+`beq`. `busy` -> **materialises**, `cntlzw`+`srwi` (no dot) + a separate `cmpwi` to consume it later. ONE load. |
| DualC | same as DualA, declaration order swapped (`busy` before `ok`) | identical codegen to DualA -- declaration order doesn't matter, only use-site position does |
| DualG | three bools (`ok`, `busy`, `corrupt`) from one capture, use-count test the coordinator asked for | `ok` (immediate branch) -> still plain `cmpwi`. `busy` and `corrupt` (both deferred, used inside the nested block) -> both materialise. Confirms the trigger is "does this bool's use cross a block boundary from its definition," not use-count. |
| DualD | `if (!ok & busy) {...} if (!ok) return1; ...` (coordinator's bitwise-AND suggestion, forces both operands evaluated as values, not short-circuited) | **Both** `ok` and `busy` materialise via `cntlzw`+`srwi.`/`srwi`, from one load. The only shape found that gets `ok` itself to materialise from a plain read. |
| save_dualtest.cpp | DualA's pattern applied to the **real, full-size `save()`** (register pressure / function scale ruled out as the missing ingredient) | Identical result to the isolated DualA probe: `ok2` plain `cmpwi`, `eq6` materialises. Confirms probe scale was never the issue. |
| Split-single | `int rawVal = *(volatile int*)&mError; bool ok=(rawVal==0); if(!ok) return1;` -- does opacity survive being captured into a plain local for a SINGLE use (not even reused)? | **No.** Collapses straight to `cmpwi`, exactly like an un-cast read. Opacity requires the volatile read to be lexically inside the comparison expression itself; splitting it into "read, then compare" loses it even with only one consumer. This closes off any read-once-materialise-twice hybrid. |

### Why this doesn't close the gap

1. **DualD is the only shape that materialises both from one load, and it
   doesn't match target.** It computes `busy` unconditionally (even on the
   `ok`-true path, where target computes nothing extra), and needs an extra
   `cmpwi` to consume `busy`'s value because its `srwi` (no dot, computed
   speculatively before it's known to be needed) can't leave usable flags
   across the earlier branch. That's strictly more instructions than
   target's lazy, sequential form, in the common case.
2. **Every lazy, nested-if shape (matching target's actual control flow
   exactly) that keeps the read plain gives `ok` as `cmpwi`, full stop** --
   DualA, DualC, DualG, and the real-function `save_dualtest.cpp` agree.
   `ok`'s only consumer is the branch immediately following it; nothing
   about eagerness, sibling count, or function scale changes that a
   same-block define-then-branch on a provably-plain value folds.
3. **Target's `ok`-equivalent test materialises even when there is no
   sibling at all.** `save()`'s very first check (`0x800CF238`, `ok1`) is
   not part of a chain -- there is no `==6` test anywhere near it, nothing
   else derived from that load needs to survive anything -- and target still
   emits `cntlzw`+`srwi.` for it. The same is true of `ok5`/`ok6` and their
   `load()` equivalents. The "value must stay live across a block" story has
   nothing to explain there, yet target materialises anyway. This is direct,
   already-in-hand counter-evidence that a plain, CSE-able read cannot be
   the whole story -- confirmed by every one of `EC`/`F`/`G` (this report,
   earlier section) plus `existCheck`'s own already-matching source, all of
   which keep an immediately-branched plain-field test at `cmpwi`.

**Conclusion, stated plainly per the coordinator's request**: the two-live-
bools mechanism is real and is a genuinely new, correct finding -- a plain,
non-volatile read *can* materialise via `cntlzw` when its consumer is
deferred across a block boundary, which is new information about MWCC that
neither CLOSE_A nor this report's earlier sections had. But it is not
sufficient, alone or combined with anything tried, to reproduce target's
bytes: it never materialises the *first* test of a chain in target's
sequential, lazy shape, and target materialises standalone tests that have
no sibling for the mechanism to apply to at all. The residual is not solved
and I'm not aware of a remaining untried shape; treating it as reconfirmed
closed unless someone has a fundamentally different angle on the standalone
case specifically (why does an isolated `mError==0` guard with literally
nothing else touching the load materialise in target, using a plain field
and no adjacent chain?) -- that question, not the chained-load-sharing one,
is now the sharpest open lead if anyone wants to keep pursuing this.

## Addendum 2: the `createBanner` "value production" lead, tested exhaustively

The coordinator found that `createBanner()` -- byte-exact, real header, plain
non-`volatile` `int mError` -- materialises its final `return mError != 0;`
via `neg`/`or`/`srwi` (no recording dot: it is a return value, not a branch
condition) with **no opacity mechanism anywhere**. Confirmed directly by
recompiling `wip/nand_thread/scratch/merge_lead/d_nand_thread.cpp` unmodified
through the harness: `createBanner`, `existCheck`, and `checkCRC` all still
MATCH. This retires "the read must be opaque" as the master explanation --
agreed, and it reframes the question correctly: the trigger is the bool being
needed as a **value**, not how it's read. That part of the new lead is now
proven, not just argued.

### Confirmed: an explicit second consumer forces the RECORDING form, from a plain field, no cast

```cpp
// SecondA -- ok returned again after its own branch
bool ok = (t->mError == 0);
if (!ok) { return 1; }
t->mFileExists = true;
return ok;
```
compiles to:
```
lwz r0, 0x78(r3)
cntlzw r0, r0
srwi. r4, r0, 5      <- RECORDING form (the dot), matching target's idiom
bne .L_...
```
Same result for `SecondB` (`ok` stored to a member after the branch) and
`SecondC` (`ok` passed to a call after the branch) -- full text and output in
`wip/nand_thread/scratch/closer_d/probe.py`. This is a real, new, validated
mechanism, independent of `volatile`, and it is the correct generalisation of
what `createBanner` demonstrates: `createBanner`'s bool is a value because it
*is* the return; these are values because they are read again after the
branch that first tested them.

### Tested and refuted: "contagion" from a sibling materialising bool

Built a function with `ok1` (plain guard, single consumer, exactly `save`'s
real `ok1` shape) followed later by `ok2` which genuinely materialises
(`return ok2;` after its own guard), both in one function, both preceded by
real `setNandError()` calls so the reads can't be trivially CSE'd away:

```cpp
int probeContagion2(dNandThread_c *t) {
    t->setNandError(1);
    bool ok1 = (t->mError == 0);
    if (!ok1) { return 1; }
    t->setNandError(2);
    bool ok2 = (t->mError == 0);
    if (!ok2) { return ok2; }
    t->mFileExists = true;
    return 0;
}
```
Result: `ok2`'s branch materialises (`cntlzw`+`srwi.`), `ok1`'s stays plain
`cmpwi`. A materialising bool elsewhere in the same function, including one
that itself feeds the function's eventual return, does **not** pull an
unrelated standalone guard into the recording form. This directly refutes
the "whole function adopts one lowering style because it returns a mix of
bools and constants" version of the hypothesis -- also tested with the tail
call literally present (`probeChainReturn2`: two plain standalone guards
ending in `return t->createBanner();`) with the same result, both guards
stayed `cmpwi`.

### Tested: does giving `ok1` itself a derived second consumer work?

```cpp
bool ok1 = (t->mError == 0);
if (!ok1) {
    return !ok1;      // value derived from ok1 itself, not an unrelated literal
}
```
This **does** flip the branch to the recording form -- but it costs 2 extra
instructions target doesn't have: MWCC does not reuse the already-computed
0/1 in `r0` for the return, it re-runs `cntlzw`+`srwi` a second time to
compute `!ok1` from `ok1`'s own materialised value. Target's actual failure
arm is a bare `li r3, 0x1` -- no recomputation, no second `cntlzw` anywhere
in that arm. So this shape gets the mechanism (`cntlzw` appears at the
branch) but not the bytes (extra instructions target doesn't have). It's a
genuine data point -- second-consumer-of-itself works as a trigger -- but not
usable as-is.

### `checkCRC`, re-checked as requested

`checkCRC()`'s real, matching source (quoted from `merge_lead`) never once
stores a comparison into a named `bool` -- every guard is
`if (a != b) { return false; }`, a direct compare feeding a literal return,
exactly the `EC2`/`G` shape from the main report's A/B (row `G`: direct
compare, asymmetric guard, plain field -> `cmpwi`, confirmed) repeated a
dozen times at larger scale. It is not a boundary case that needs new theory
-- it is a big, clean confirmation of the existing rule ("no named bool
intermediate -> never materialises, no matter how many literal-returning
guards the function has"). It does not distinguish anything new from
`createBanner`; it only shows the *other* extreme of the same axis
(`createBanner`: named bool, used as a value, twice -> materialises;
`checkCRC`: no named bool anywhere -> never materialises). `save`'s `ok1` is
the genuinely awkward middle case: a named bool, but (as far as any shape
tried can tell) with only one consumer, and target still materialises it.

### Where this leaves `save`/`load`'s standalone tests (not just the chain)

The second-consumer mechanism is real, and it correctly explains
`createBanner`'s tail and (very plausibly) explains the deferred-use half of
each chained pair (`eq6`/`busy` in Addendum 1 -- consumed in a different
block than its definition, which is a form of "not a same-block, single,
immediate consumer" the same family of reasoning covers). **It does not
explain `save`'s `ok1`, `ok5`, or `load`'s `ok6`, or in general the first
test of every occurrence** -- every attempt to give one of these a natural
second consumer either left it at plain `cmpwi` (contagion, tail-call
unification) or triggered materialisation at the cost of extra instructions
target does not have (`return !ok1`). No shape found reproduces target's
exact bytes for these specific tests: `cntlzw`+`srwi.` at the branch, and
nothing else different about the arm.

Per the coordinator's framing: **two live consumers do not, on their own,
reproduce `save`/`load`'s standalone materialisations byte-for-byte.** They
reproduce the *mechanism* (recording-form `cntlzw` from a plain read) but
not the *specific instance* -- `save`'s `ok1` shows no natural second use in
any shape tried, and manufacturing one costs bytes target doesn't spend.
Reporting this plainly, as requested, rather than landing a lever that gets
the idiom to appear at the wrong price. Current best draft (the volatile-cast
lever from the main report, 97/95 `save`, 162/161 `load`, no header change,
all 12 required functions still matching) is unchanged and remains this
report's result.

## Answering the prompt's specific questions

- **Is the header-volatile proposal wrong?** Yes, confirmed independently:
  it is unnecessary. A per-read cast at the exact call sites that need
  opacity achieves the identical instruction sequence with zero blast radius
  and zero regression risk, because it does not touch any declaration any
  other function reads through.
- **Is Lead 1 (arm shape) the answer?** No. Row EC3 above (cast + bool
  storage + `existCheck`'s own block-fallthrough shape, not an early return)
  still materialises. Arm shape is irrelevant to whether `cntlzw` appears;
  it only affects which branch orientation MWCC picks, which is a separate
  layout decision (see `save()`'s ok5 fix).
- **Is the second, smaller residual (`save()`'s final check orientation) the
  same problem as Lead 1, or separate?** Separate, and now resolved: simple
  guard-clause reordering (`if (!ok5) return 1; return createBanner();`
  instead of `if (ok5) return createBanner(); return 1;`) reproduces target's
  exact branch layout there. It did not change `save()`'s instruction count
  (the two orientations happened to be equal length) but it does remove that
  span from the diff entirely, leaving `save()`'s whole remaining gap
  attributable to one cause: the chained-load residual.
- **Is the chained-load residual solved?** No. Reconfirmed open, via three
  independent attempts (two separate reads / captured plain local / pointer
  alias to the same volatile location), all giving the same trade-off
  CLOSE_A already found under a different opacity mechanism. This looks like
  a genuine structural limit of expressing "materialise via `cntlzw` twice,
  loading only once" in standard C++ under MWCC -O4, not a gap in technique.
