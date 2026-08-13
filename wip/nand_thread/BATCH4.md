# Batch 4 report — `cmdLoad`, `load`, `checkCRC`, `cmdDeleteFile`, `deleteFile`

Working dir: `wip/nand_thread/scratch/batch4/`. Final source:
`wip/nand_thread/scratch/batch4/d_nand_thread.cpp`. Compiled and diffed with
`tools/auto_decomp/harness.py`'s `compile_draft` / `extract` / `diff_fn`,
extracted **by address**, `instruction_count * 4` checked against
`bin/dtk/wiimj2d_symbols.txt`, and emitted symbol order checked against target
order. A driver script (`run_diff.py`) in the same directory reproduces all of
this.

## Result summary

| Function | Address | Size | Result |
|---|---|---|---|
| `cmdLoad` | `0x800CF610` | `0x6C` | **MATCH** |
| `load` | `0x800CF680` | `0x284` | **NOT closed** — see below, exact and isolated |
| `checkCRC` | `0x800CF910` | `0xCC` | **MATCH** |
| `cmdDeleteFile` | `0x800CF9E0` | `0x6C` | **MATCH** |
| `deleteFile` | `0x800CFA50` | `0x6C` | **MATCH** |

Emitted order (compiled as `d_nand_thread.cpp` so anon-namespace mangling is
real, not an artifact of a scratch filename):
`cmdLoad, load, checkCRC, cmdDeleteFile, deleteFile` — matches target address
order exactly.

4 of 5 are byte-identical, instruction-count-verified against the symbol map,
and in the right order. `load` is not closed; the divergence is fully
characterized below and it is the *only* thing wrong in that function.

## Header changes already landed by others, confirmed correct here

- `bool cmdLoad()`, `bool cmdDeleteFile()` (not `void`) — my own derivation,
  done before the lead's relay confirmed it. Two independent derivations
  agreeing.
- `s32 load()` (not `bool`) — my own derivation from the `li r3, 0x2` path,
  done before Batch 1 confirmed it from `run()`'s `cmpwi r3, 0x2` dispatch.
  Two independent derivations agreeing, from two different kinds of evidence
  (my own function's codegen vs. its caller's).
- `void setNandError(long err)` (not `s32`) — confirmed; `deleteFile` and
  `cmdDeleteFile` both call it and both close byte-exact with this signature.
- `NANDSimpleSafeOpen`/`Close`/`Cancel` now in
  `include/lib/revolution/NAND/NANDOpenClose.h` — confirmed, `load()` calls
  all three with exactly the signatures now declared there.

## Finding: `deleteFile()` must be `bool` → `void`

The header currently has `bool deleteFile()`. **I am the only witness** (Batch
1 reports `run()` discards this return value, so it gives no evidence either
way). My own codegen is conclusive:

- Compiled as `bool` (with `return true;` / `return false;`): 31 instructions,
  4 more than the target's 27, with an extra branch and an `li r3, 0/1`
  materialization the target does not have.
- Compiled as `void` (falls off the end on all paths): **27/27 instructions,
  byte-exact**, including a real structural detail — the target has a
  **dead trailing `cmpwi`** (loads `mError` a second time, compares it to 0,
  and never branches on it, just falls into the epilogue). That dead compare
  only appears because the source has a second, pointless
  `if (mError != 0) { return; }` as the last statement — with a real `bool`
  return this would need to materialize a value and wouldn't be dead.

Proposed header change:
```cpp
void deleteFile();   // was: bool deleteFile();
```

## Finding: `checkCRC()` stays `bool` — confirmed independently

**I am the only witness** here too (Batch 1 reports nothing in `run()` calls
`checkCRC()`; it's only called from `load()`, which is mine). My codegen
converges to one epilogue with explicit `li r3, 0x0` / `li r3, 0x1` on every
path, matching the target exactly with `bool`. No change needed.

## Finding: I own `@67269` (`"SMNP"`, `.sdata` `0x80427F7C`), not Batch 3

The shared brief assigned this to Batch 3. Batch 3 checked and it's correct:
the address is referenced only inside `load()`. It is **not** a named
anonymous-namespace object — its symbol is the bare pool form `@67269`, with
no `__27@unnamed@d_nand_thread_cpp@` suffix, meaning it's a genuine compiler
literal pool object. I write it as a plain inline string literal `"SMNP"` at
each of its four use sites in `load()`; the compiler pools it into one
`.sdata` object on its own, and I verified this produces the exact
`li r4, "SYM0"@sda21` / indexed-`lbz` shape the target has (canonicalised
pool name `SYM0` in the harness's diff — real name will be `@67269` once
assembled in place, right after Batch 3's own literals in source order).

**Consumption**: `load()`, after the final `NANDSimpleSafeClose` succeeds,
checks `l_tmpSave[0..3]` against this literal as a 4-byte magic-number guard
before trusting the loaded save data, split as **two separate `if`
statements** — `l_tmpSave[0..2]` compared via a short-circuit `||` in one
`if`, and `l_tmpSave[3]` compared in a second, separate `if` — not one 4-term
`||`. Each has its own `mError = 6; return 1;` body (code is genuinely
duplicated in the target, not a single shared exit). Getting this exact split
right, plus using bare literal-array indexing (`l_tmpSave[0] != "SMNP"[0]`,
not pointer variables), reproduced the target byte-for-byte from the magic
check through `checkCRC()`'s call and the epilogue (49 consecutive
instructions, verified individually).

## Types needed for the LEAD's `.bss` declarations

- **`l_safeCopyBuf`**: `u8 l_safeCopyBuf[0x4000];` — only ever passed as a
  `void*` staging buffer to `NANDSimpleSafeOpen`; element type doesn't matter
  functionally, but `u8` is the natural byte-buffer type and produces the
  right codegen (`li r7, 0x4000` for its `sizeof`).
- **`l_tmpSave`**: **`char l_tmpSave[0x3FA0];`** — NOT `u8`. This is load-bearing.
  The magic-number check in `load()` does `extsb` (sign-extend byte) on
  **both** operands before `cmpw` — one operand is a `char` string-literal
  byte (always signed on this platform/compiler, confirmed independently
  because the literal side also needs `extsb`), and for the *other* operand
  (`l_tmpSave[i]`) to also need `extsb`, its element type must be signed
  `char`, not unsigned `u8`. An unsigned element would zero-extend via the
  `lbz` alone with no extra instruction, which is not what the target does.
  I did not discover this until testing the magic-check byte-for-byte; my
  earlier `u8 l_tmpSave[0x3FA0]` compiles and even passes `checkCRC()`
  (pointer-arithmetic-only, signedness doesn't matter there), but silently
  fails the magic check with the wrong instruction shape.

## Proposed new declarations (not yet anywhere in this codebase)

`calcCRC32` (`0x8015F270`, unresolved, will be a `syms.txt` pin) needs a
class. I found no `sCrc` anywhere under `include/`. Minimal declaration that
produces the exact mangled name `calcCRC32__4sCrcFPCvUl`:
```cpp
class sCrc {
public:
    static unsigned long calcCRC32(const void *p, unsigned long len);
};
```
Note **`unsigned long`, not `u32`** — `u32` is `unsigned int` here (mangles
`Ui`), and the real symbol mangles `Ul`. Same class of bug the lead already
flagged for `setNandError`. I'd suggest this lives wherever the lead intends
to eventually bank the real `sCrc::calcCRC32` — I have no evidence for a
specific header location, only for the signature.

## The one thing I could not close: `load()`'s `mError` test idiom

`load()` tests `mError` nine separate times (after every NAND call that can
fail). **Every single one**, with zero exceptions, compiles in the target to
this shape:
```
lwz  r3, 0x78(r31)      # or r0, depending on whether the raw value is reused
cntlzw r0, r3
srwi. r0, r0, 5
bne/beq  <target>
```
i.e. a leading-zero-count materialization of "is this zero", instead of the
plain `lwz; cmpwi; beq/bne` that **every other function in this batch uses for
the identical `mError == 0` / `mError != 0` test** (`deleteFile`, and the
`==6` sub-check reuses the same pattern in the target too — `subi r0,r3,0x6;
cntlzw; srwi.; beq`).

My reconstruction produces the plain `cmpwi` form throughout, which is
functionally correct — control flow, branch targets, all constants (`0`, `6`,
`2`), the `s32` return, and every surrounding call all check out — but it is
**12 instructions (48 bytes) shorter** than target (149 vs. 161), and the
deficit is fully and exactly accounted for by these 9 occurrences (3 paired
`!=0`+`==6` checks costing 3 instructions each = 9, plus 3 solitary `!=0`
checks costing 1 each = 3; 9+3=12, matching the gap exactly). **Nothing else
in `load()` is wrong** — I confirmed this directly: the entire tail of the
function, from the magic check through the `checkCRC()` call to the
epilogue (49 straight instructions), matches the target byte-for-byte, and
every unresolved diff line above position 16 in the current dump is this same
idiom recurring, not a new problem.

I tried to reproduce the target's idiom and could not, across 7 source-level
formulations, all compiled and checked against the exact same target line:
1. Plain `if (mError != 0)` — gives `cmpwi`.
2. `bool hasError = (mError != 0); if (hasError)` — gives `cmpwi` (the
   `bool`-materialization lever that fixed `cmdLoad`'s `OSTryLockMutex` test
   does not apply here; a comparison operator's result is already canonical
   0/1, unlike an arbitrary `BOOL`-typed external call's return).
3. `if (mError)` (implicit truthy conversion) — gives `cmpwi`.
4. `if (!!mError)` (double negation) — gives `cmpwi`.
5. `(unsigned long)mError != 0` — gives `cmplwi` for the `==6` sub-check
   (unsigned immediate form) but still not `cntlzw`.
6. `long errCode = mError; if (errCode != 0)` (typed local matching `err`'s
   type elsewhere in the function) — gives `cmpwi`.
7. Full realistic context — actual `NANDFileInfo`, actual preceding
   `NANDSimpleSafeOpen` + `setNandError` calls, actual stack frame size,
   even the complete real function with every other check present — still
   gives `cmpwi`. This rules out "later code in the function changes earlier
   codegen" as an explanation; the complete function was in place for this
   test and the result did not change.

I could not find this idiom documented in `HANDOFF.md` either — its
comparison-shape table (`cmpwi rN, 0` → `if (num != 0)`) documents plain
`cmpwi` as standard for exactly this test, with no mention of a `cntlzw`
alternative, which is consistent with what I could reproduce but not with
what the target actually has.

**This is a genuine open question, not a guess I'm papering over.** I'm
reporting it exactly where it diverges (all 9 occurrences, all in `load()`,
all identical in shape, all costed and accounted for) rather than presenting
`load()` as closed. Whoever picks this up next should treat "why does MWCC
choose `cntlzw` here and only here" as the one remaining question — possibly
a helper method I have no evidence for (an inlined `bool
dNandThread_c::???() const` wrapping the test), possibly a source idiom I
haven't tried, or possibly something build-configuration-level outside the
`.cpp` itself.

## Final source

```cpp
#include <game/bases/d_nand_thread.hpp>

// ---- proposed, not-yet-decompiled dependency (see report) ----------------
class sCrc {
public:
    static unsigned long calcCRC32(const void *p, unsigned long len);
};

// ---- objects the LEAD owns at file scope (shown for context only --------
// I do NOT define these; shown here with the types they must have.
namespace {
u8 l_safeCopyBuf[0x4000];
char l_tmpSave[0x3FA0];   // char, NOT u8 -- see report
} // namespace

// ---------------------------------------------------------------- cmdLoad
bool dNandThread_c::cmdLoad() {
    bool locked = OSTryLockMutex(&mMutex.mOSMutex);
    if (locked) {
        mError = 0;
        mState = 5;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------- load
// NOT CLOSED -- structurally and constant-wise correct (see report for the
// one remaining divergence: the mError-test idiom, 12 instructions).
s32 dNandThread_c::load() {
    NANDFileInfo info;
    long err = NANDSimpleSafeOpen(sc_GAME_FILE, &info, 1, l_safeCopyBuf,
                                   sizeof(l_safeCopyBuf));
    setNandError(err);
    if (mError) {
        if (mError != 6) {
            err = NANDSimpleSafeCancel(&info);
            setNandError(err);
        }
        return 1;
    }

    u32 length;
    err = NANDGetLength(&info, &length);
    setNandError(err);
    if (mError != 0) {
        if (mError != 6) {
            err = NANDSimpleSafeCancel(&info);
            setNandError(err);
        }
        return 1;
    }

    if (length != sizeof(l_tmpSave)) {
        err = NANDSimpleSafeClose(&info);
        setNandError(err);
        if (mError != 0) {
            return 1;
        }
        mError = 6;
        return 1;
    }

    err = NANDRead(&info, l_tmpSave, sizeof(l_tmpSave));
    if (err < 0) {
        setNandError(err);
        if (mError != 0) {
            if (mError != 6) {
                err = NANDSimpleSafeCancel(&info);
                setNandError(err);
                if (mError != 0) {
                    return 2;
                }
            }
            return 1;
        }
    }

    err = NANDSimpleSafeClose(&info);
    setNandError(err);
    if (mError != 0) {
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

// ---------------------------------------------------------------- checkCRC
bool dNandThread_c::checkCRC() {
    if (*(unsigned long *)(l_tmpSave + 0x69c) !=
        sCrc::calcCRC32(l_tmpSave + 4, 0x698)) {
        return false;
    }
    char *dataA = l_tmpSave + 0x6a0;
    char *p = l_tmpSave;
    char *dataB = l_tmpSave + 0x2320;
    for (int i = 0; i < 3; i++) {
        unsigned long crcA = sCrc::calcCRC32(dataA, 0x97c);
        if (crcA != *(unsigned long *)(p + 0x101c)) {
            return false;
        }
        unsigned long crcB = sCrc::calcCRC32(dataB, 0x97c);
        if (crcB != *(unsigned long *)(p + 0x2c9c)) {
            return false;
        }
        dataA += 0x980;
        p += 0x980;
        dataB += 0x980;
    }
    return true;
}

// ---------------------------------------------------------------- cmdDeleteFile
bool dNandThread_c::cmdDeleteFile() {
    bool locked = OSTryLockMutex(&mMutex.mOSMutex);
    if (locked) {
        mError = 0;
        mState = 3;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------- deleteFile
// Header needs `void deleteFile()`, not `bool` -- see report.
void dNandThread_c::deleteFile() {
    long err = NANDDelete(sc_BANNER_FILE);
    setNandError(err);
    if (mError == 0) {
        err = NANDDelete(sc_GAME_FILE);
        setNandError(err);
        if (mError != 0) {
            return;
        }
    }
}
```

## `checkCRC()` register-allocation notes (in case they generalize)

Not a finding to act on, just documented since it cost real effort: getting
`checkCRC()`'s loop pointers into the right registers (`p`→r30, `dataA`→r31,
`dataB`→r29, `i`→r28 — target order, not simple declaration order) required
declaring the header-check pointer (`p`) *after* `dataA`, sourced directly
from `l_tmpSave` rather than from `p`, even though `p` is used first at
runtime. And the `cmplw` operand order for the loop's two CRC checks
(`crcX != *(...)`, call-result first) only came out right after naming the
`calcCRC32()` result in a local (`unsigned long crcA = ...`) instead of
comparing the call expression inline — same lever HANDOFF.md documents
generally ("add one named local holding a value already computed"), just not
previously seen needed for comparison-operand ordering specifically.
