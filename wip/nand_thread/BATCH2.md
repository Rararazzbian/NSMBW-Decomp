# Batch 2 report — `cmdExistCheck`, `existCheck`, `cmdSpaceCheck`, `spaceCheck`

Compiled and diffed with `tools/auto_decomp/harness.py`'s `compile_draft` /
`disasm` / `extract` / `diff_fn` (imported directly, never reimplemented),
against `wip/nand_thread/target_raw.txt`. Driver script, deliverable source,
shadow header, raw diff logs, and every register-allocation experiment tried
are in `wip/nand_thread/scratch/batch2/`.

**No function below is claimed MATCHING unless the diff printed nothing.**
Verified three ways for every function: byte equality (`diff_fn`), size
against `bin/dtk/wiimj2d_symbols.txt` (`instruction_count * 4` checked before
trusting any comparison), and emitted symbol order (all four came out of the
isolated compile in the same relative order as the target: `cmdExistCheck`,
`existCheck`, `cmdSpaceCheck`, `spaceCheck`).

## Status table

| Function | Address | Size | Status |
|---|---|---|---|
| `cmdExistCheck()` | `0x800CEF10` | `0x70` | **MATCHING** — 28/28 instructions |
| `existCheck()` | `0x800CEF80` | `0xD8` | **MATCHING** — 54/54 instructions |
| `cmdSpaceCheck()` | `0x800CF060` | `0x6C` | **MATCHING** — 27/27 instructions |
| `spaceCheck()` | `0x800CF0D0` | `0x94` | **NOT matching.** 34/37 instructions identical, in the same order, at the same offsets. 3 lines differ, all the same difference: the target holds a reloaded local in `r3`, my draft holds it in `r4`. See below — this is a register-allocation wall, not a different program. |

**This batch was the blocking one per `SHARED-BRIEF.md`.**
`wip/nand_thread/CMD_SHAPE.md` was written as soon as both `cmdExistCheck` and
`cmdSpaceCheck` were confirmed byte-exact, for Batches 3/4 to consume. It
contains the exact final source of both, the field-write table, the helper
call order, and two header contradictions (see below) — read it before
authoring `cmdLoad`/`cmdDeleteFile`/`fn_800CF170`.

## `cmdExistCheck` — MATCHING, and the brief's assumed shape verified only partially

`SHARED-BRIEF.md` guessed "lock the mutex, store the command id into `mState`,
signal the condition variable, unlock" for all five `cmdXxx`-shaped
functions, and asked Batch 2 to verify rather than assume. Verified, with two
corrections:

1. **Unlock happens before signal**, not signal-then-unlock: the target calls
   `OSUnlockMutex` first, then `OSSignalCond`. Confirmed from the raw bytes
   at `0x800CEF54`/`0x800CEF5C` and identically in `cmdSpaceCheck`,
   `cmdLoad`, `cmdDeleteFile`.
2. **The body is not one shape shared by all five — it is two.**
   `cmdExistCheck` writes three fields before unlocking (`mError`,
   `mFileExists`, `mState`). `cmdSpaceCheck`, `cmdDeleteFile`, and `cmdLoad`
   write only two (`mError`, `mState` — no `mFileExists`). `fn_800CF170`
   (Batch 3's, probably `cmdSave`) writes two fields *plus* an extra
   `memcpy` of `0x3fa0` bytes, which is why it's `0x8C` instead of `0x6C`.
   This is exactly the contradiction-worth-reporting the brief anticipated;
   full detail and the source-level table is in `CMD_SHAPE.md`.

```cpp
bool dNandThread_c::cmdExistCheck() {
    bool locked = OSTryLockMutex(&mMutex.mOSMutex);
    if (locked) {
        mError = 0;
        mFileExists = false;
        mState = 1;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}
```

**Header contradiction:** `include/game/bases/d_nand_thread.hpp` currently
declares `void cmdExistCheck();`. The target's own bytes are unambiguous that
it's `bool` — the compiled function ends with an explicit
`li r3,0x1 / b <end>` on the locked path and `li r3,0x0` on the failed-lock
path, both converging on one shared epilogue. That is the standard codegen
for `return true;`/`return false;`, and it does not appear at all if the
function is void with no return statement (I confirmed this directly: a void
version with a plain `if (OSTryLockMutex(...)) { ... }` compiles to a bare
`cmpwi r3,0x0; beq ...` with no trailing branch-and-materialise, 5
instructions shorter). Proved the `bool` fix in a shadow copy
(`wip/nand_thread/scratch/batch2/shadow/game/bases/d_nand_thread.hpp`); did
not touch the real header, which is out of scope for this batch. Every
caller in `source/dol/bases/d_s_boot.cpp` (`dNandThread_c::m_instance->cmdExistCheck();`,
line 710) already discards the return value, so this fix has no fallout.

**Second header contradiction, found while proving the first:**
`setNandError`'s mangled name in the target is
`setNandError__13dNandThread_cFl` (trailing `l` = `long`). The header
declares `void setNandError(s32 err);`, and `s32` is `signed int`
(`include/types.h:28`), which mangles `...Fi`, not `...Fl`. Confirmed against
every `bl setNandError__13dNandThread_cFl` call site in `target_raw.txt` (9
call sites across the whole TU, all consistent) and against
`bin/dtk/wiimj2d_symbols.txt:5582`. The parameter must be declared plain
`long`. This affects every batch that calls `setNandError` — all four do.
Also shadow-tested only, not applied to the real header.

## `existCheck` — MATCHING, 54/54

```cpp
bool dNandThread_c::existCheck() {
    u8 count = 0;
    u8 type;

    s32 err = NANDGetType(sc_GAME_FILE, &type);
    setNandError(err);
    if (mError == 0) {
        if (err == 0 && type == 1) {
            count = 1;
        }

        err = NANDGetType(sc_BANNER_FILE, &type);
        setNandError(err);
        if (mError == 0) {
            if (err == 0 && type == 1) {
                count++;
            }

            if (count == 2) {
                mFileExists = true;
            }
        }
    }
}
```

Notes on structure, all confirmed directly from the bytes, not guessed:
- **No explicit `return` anywhere**, despite the header declaring `bool`.
  The target never writes `r3` before `blr` — whatever's left over from the
  last call/store is what gets returned, and every caller ignores it anyway.
  Adding an explicit `return mFileExists;` (or any return) costs an extra
  `lbz`+narrowing instruction the target doesn't have and breaks the match.
  This is the same idiom `wip/player_manager/BATCH2.md` already documented
  for `daPyMng_c::fn_8005f570` — a second independent confirmation that
  "declared `bool`, compiles fine, no return statement, garbage return value
  nobody reads" is a real, repeatable pattern in this codebase's original
  source, not a decompiler artifact.
- **Nested `if (mError == 0) { ... }`, not an early `if (mError != 0)
  return;`** — the latter isn't even legal C++ here (`return;` with no value
  in a `bool` function is a hard error), and the former is exactly what
  produces the target's two `bne <end>` early-exits landing on the same
  final label.
- `type` is `u8` (matches `NANDGetType(const char*, u8*)`'s real signature in
  `include/lib/revolution/NAND/NANDCore.h:26`), and `count` is `u8` too — the
  `count++` path needs an explicit 8-bit truncation (`clrlwi r30,r0,24`)
  that the `count = 1` literal path doesn't, and the source reproduces that
  distinction exactly because only the increment needs integer-promotion
  narrowing back down.
- `sc_GAME_FILE` / `sc_BANNER_FILE` are the anonymous-namespace string
  constants `SHARED-BRIEF.md` assigns to LEAD (file top, `.rodata`). I only
  reference them (`extern const char sc_GAME_FILE[];` etc. in an anonymous
  namespace in my scratch file), never define them, per the brief.
  **Getting a byte-exact match on these references required naming my
  scratch source file `d_nand_thread.cpp`, not `draft.cpp`** — MWCC's
  anonymous-namespace mangling bakes in the TU's own file stem
  (`sc_GAME_FILE__27@unnamed@d_nand_thread_cpp@` vs.
  `...@unnamed@draft_cpp@`), so an isolated compile under any other filename
  will show a spurious diff on every anon-namespace reference that is not a
  real code difference — worth flagging for whoever else hits this.

## `cmdSpaceCheck` — MATCHING, 27/27, and it resolves the brief's open question

```cpp
bool dNandThread_c::cmdSpaceCheck() {
    bool locked = OSTryLockMutex(&mMutex.mOSMutex);
    if (locked) {
        mError = 0;
        mState = 2;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}
```

Same header contradiction as `cmdExistCheck` (should be `bool`, not `void`) —
confirmed independently by the same evidence pattern. This is the function
that proves `cmdExistCheck`'s `mFileExists` write is **not** part of the
shared shape — `cmdSpaceCheck` is otherwise byte-identical in skeleton
(lock → two field-writes → unlock → signal → return) but simply doesn't touch
`mFileExists` at all, because `spaceCheck()` (the worker this command
triggers) doesn't report file existence.

## `spaceCheck` — NOT matching. 34/37, one register choice unresolved

```cpp
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            if (answer & 5) {
                mError = 7;
            } else if (answer & 0xa) {
                mError = 8;
            }
        }
    }
}
```

Every operation, every branch, every stack offset, every immediate matches
the target exactly, in the same order — including the `u32 answer =
0xFFFFFFFF;` pre-initialisation before the out-param call (`li r0,-1; stw
r0,0x8(r1)` ahead of `bl NANDCheck`, reproduced exactly), the `NANDCheck(3, 2,
&answer)` argument order, the `err` value living in the callee-saved `r31`
across the `setNandError` call, and the same no-explicit-return idiom as
`existCheck`. The **only** difference, three lines out of 37:

```
21 | want: lwz r3, 0x8(r1)      got: lwz r4, 0x8(r1)
22 | want: andi. r0, r3, 0x5    got: andi. r0, r4, 0x5
27 | want: andi. r0, r3, 0xa    got: andi. r0, r4, 0xa
```

The target reloads `answer` from its stack slot into `r3` for the two mask
tests; my draft reloads the identical value into `r4`. Both registers are
dead and free at that point (whatever last held `r3`/`r4` was consumed by the
preceding `bl setNandError`), so this is a pure register-allocator choice,
not a structural or logical difference.

**Tried and rejected as levers** (13 source variants, all in
`wip/nand_thread/scratch/batch2/try_variants*.py`, full output logged when
run):
- Reordering the `answer`/`err` local declarations (both orders, split vs.
  combined decl+init).
- An unused dummy local (`int`/`u32`) inserted at five different points in
  the function body — all five were dead-code-eliminated with zero codegen
  effect, ruling out the "add a variable to shift colouring" lever from
  `HANDOFF.md` for this specific case (that lever needs the added local to
  survive optimisation to have any effect, and nothing here gives it a
  reason to).
- Merging the two nested `if`s back into one `&&` condition, and the reverse
  (three-way `if/else if` chain with the condition repeated per branch) —
  both change the branch structure and make the diff *worse* (10–22 lines).
- A `u32`→`s32` retype of `answer` (with a `(u32*)` cast at the call site).
- A `u32`→`s16`/`u8` narrowing "reassociation barrier" local, per
  `HANDOFF.md`'s narrowing lever — both change the instruction count (38 vs.
  37), so they're a different program, not a register fix.
- A ternary assignment (`mError = (answer&5) ? 7 : (answer&0xa) ? 8 :
  mError;`) — changes the branch shape (10-line diff).
- Two `bool` intermediates (`over`/`under`) holding the two mask tests
  before branching on them — far worse (22-line diff), the boolean
  materialisation itself costs extra instructions here.

This reads as a genuine register-allocation-wall case per `HANDOFF.md`'s own
description of the phenomenon ("same difference N times... a plateau"), not
a function I under-authored. Reporting the near-miss rather than forcing a
claim, per the brief's own rule. Whoever has more budget or a stronger model
for this one function should start from the working 34/37 source above —
the only open question is which single value the compiler puts in `r3` vs.
`r4`, everything else is already correct.

## Header contradictions summary (reported, not reconciled)

1. `cmdExistCheck()` and `cmdSpaceCheck()` should be `bool`, not `void` —
   both proven independently, see above. `CMD_SHAPE.md` flags that
   `cmdLoad`/`cmdDeleteFile` almost certainly have the same issue (same
   `li r3,0x1 / li r3,0x0` codegen pattern visible directly in
   `target_raw.txt`), for Batches 3/4 to verify rather than assume.
2. `setNandError(s32 err)` should take `long`, not `s32`/`int` — proven from
   the mangled symbol name, affects all four batches.

Both proved only in the shadow copy at
`wip/nand_thread/scratch/batch2/shadow/game/bases/d_nand_thread.hpp`; the
real header was not touched.

## Command-id constants for the `Command` enum

The header currently declares no such enum. Read directly out of each
`cmdXxx` function's `mState` store (not guessed):

| Value | Command | Evidence |
|---|---|---|
| `0` | none / idle | Constructor (`0x800CED6C`–`0x800CED70`: `li r0,0x0; stw r0,0x74(r27)`) initialises `mState` to `0`. `source/dol/bases/d_s_boot.cpp:698` reads `if (dNandThread_c::m_instance->mState == 0)` as its "is the thread idle" check, confirming `0` is the idle/no-command sentinel, not just a zero-init artifact. |
| `1` | existCheck | `cmdExistCheck` (`0x800CEF10`), `li r0,0x1; stw r0,0x74(r31)` — this batch, byte-exact. |
| `2` | spaceCheck | `cmdSpaceCheck` (`0x800CF060`), `li r0,0x2; stw r0,0x74(r31)` — this batch, byte-exact. |
| `3` | deleteFile | `cmdDeleteFile` (`0x800CF9E0`, Batch 4's), `li r0,0x3; stw r0,0x74(r31)` at `0x800CFA10`. Read directly from `target_raw.txt` lines 1017–1048, not from Batch 4's work — confirming, not claiming credit. |
| `4` | save (via the unnamed `fn_800CF170`, Batch 3's) | `fn_800CF170` (`0x800CF170`, size `0x8C`), `li r0,0x4; stw r0,0x74(r30)` at `0x800CF1A8`. This function also `memcpy`s its argument into `l_tmpSave` before signalling — consistent with a save-request handoff, but naming it is Batch 3's job per the brief; I'm only reporting the constant. |
| `5` | load | `cmdLoad` (`0x800CF610`, Batch 4's), `li r0,0x5; stw r0,0x74(r31)` at `0x800CF640`. Read directly from `target_raw.txt` lines 722–753. |

Proposed enum (not applied to the real header — this batch may not edit it):
```cpp
enum Command_e {
    CMD_NONE = 0,
    CMD_EXIST_CHECK = 1,
    CMD_SPACE_CHECK = 2,
    CMD_DELETE_FILE = 3,
    CMD_SAVE = 4,
    CMD_LOAD = 5,
};
```

## Deliverable source

`wip/nand_thread/scratch/batch2/d_nand_thread.cpp` is the exact file compiled
for every number above (must be compiled with the filename
`d_nand_thread.cpp` — see the anonymous-namespace mangling note under
`existCheck`). Driver: `wip/nand_thread/scratch/batch2/run.py`, which imports
`harness.compile_draft`/`disasm`/`extract` directly and asserts each
function's target size against `bin/dtk/wiimj2d_symbols.txt` before
reporting a match. Full run output: `wip/nand_thread/scratch/batch2/full_diff.txt`.
Register-allocation experiments for `spaceCheck`:
`wip/nand_thread/scratch/batch2/try_variants.py` through `try_variants4.py`.

```cpp
#include <game/bases/d_nand_thread.hpp>

namespace {
extern const char sc_TEMP_BANNER_FILE[];
extern const char sc_BANNER_FILE[];
extern const char sc_GAME_FILE[];
} // namespace

bool dNandThread_c::cmdExistCheck() {
    bool locked = OSTryLockMutex(&mMutex.mOSMutex);
    if (locked) {
        mError = 0;
        mFileExists = false;
        mState = 1;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}

bool dNandThread_c::existCheck() {
    u8 count = 0;
    u8 type;

    s32 err = NANDGetType(sc_GAME_FILE, &type);
    setNandError(err);
    if (mError == 0) {
        if (err == 0 && type == 1) {
            count = 1;
        }

        err = NANDGetType(sc_BANNER_FILE, &type);
        setNandError(err);
        if (mError == 0) {
            if (err == 0 && type == 1) {
                count++;
            }

            if (count == 2) {
                mFileExists = true;
            }
        }
    }
}

bool dNandThread_c::cmdSpaceCheck() {
    bool locked = OSTryLockMutex(&mMutex.mOSMutex);
    if (locked) {
        mError = 0;
        mState = 2;
        OSUnlockMutex(&mMutex.mOSMutex);
        OSSignalCond(&mMutex.mOSCond);
        return true;
    }
    return false;
}

bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            if (answer & 5) {
                mError = 7;
            } else if (answer & 0xa) {
                mError = 8;
            }
        }
    }
}
```

This source requires the two shadow header changes above
(`bool cmdExistCheck()`/`bool cmdSpaceCheck()`, `setNandError(long)`) to
compile and match at all — against the unmodified real header, `cmdExistCheck`
and `cmdSpaceCheck` still compile but come out 5 instructions short each
(no boolean materialisation/return-value codegen), and `existCheck`/
`spaceCheck` fail to link-name-match `setNandError` (`...Fi` vs. the target's
`...Fl`) though their own bodies still diff clean once that's accounted for
by the caller side. I did not re-verify the exact byte counts against the
unmodified header since the fix is unambiguous from the mangled name and the
brief's process is to shadow-test and report, not to keep grinding a known
header defect from the wrong side.
