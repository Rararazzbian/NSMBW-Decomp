# `cmdXxx` shape — from Batch 2, for Batches 3 and 4

Written the moment `cmdExistCheck` and `cmdSpaceCheck` were both confirmed
byte-exact (`diff_fn` prints nothing, sizes checked against
`bin/dtk/wiimj2d_symbols.txt`, emitted order checked against target order).
Full detail and the rest of Batch 2's work is in `wip/nand_thread/BATCH2.md`.

## The shape is NOT identical across all `cmdXxx` functions — verified, not assumed

`SHARED-BRIEF.md` hypothesised "lock the mutex, store the command id into
`mState`, signal the condition variable, unlock" as one shape shared by all of
`cmdExistCheck` (0x70), `cmdSpaceCheck` (0x6C), `cmdLoad` (0x6C),
`cmdDeleteFile` (0x6C), and probably `fn_800CF170` (0x8C). **The control flow
and the lock/unlock/signal skeleton is shared by all five. The unlock/signal
order is unlock-THEN-signal, not signal-then-unlock. But the BODY they wrap is
not identical: `cmdExistCheck` writes three fields, the other four write only
two (or, for `fn_800CF170`, two fields plus an extra `memcpy`).**

Read directly out of `wip/nand_thread/target_raw.txt`:

| Function | Address | Size | Fields written before unlock | `mState` value |
|---|---|---|---|---|
| `cmdExistCheck` | `0x800CEF10` | `0x70` | `mError = 0; mFileExists = false;` | `1` |
| `cmdSpaceCheck` | `0x800CF060` | `0x6C` | `mError = 0;` (no `mFileExists`) | `2` |
| `cmdDeleteFile` | `0x800CF9E0` | `0x6C` | `mError = 0;` (no `mFileExists`) | `3` |
| `fn_800CF170` (probably `cmdSave`) | `0x800CF170` | `0x8C` | `mError = 0;` **plus** `memcpy(l_tmpSave, arg, 0x3fa0)` | `4` |
| `cmdLoad` | `0x800CF610` | `0x6C` | `mError = 0;` (no `mFileExists`) | `5` |

**Do not copy `cmdExistCheck`'s body wholesale into `cmdLoad`/`cmdDeleteFile`
and just change the constant — it will emit an extra `stb ...,0x7c(...)`
instruction that doesn't exist in the target and the byte count will be off by
`0x4`.** `cmdSpaceCheck`, `cmdDeleteFile`, and `cmdLoad` are the ones that are
identical to each other modulo the `mState` constant (all `0x6C`, all two
field-writes). `fn_800CF170` is a fourth, distinct shape (has the extra
`memcpy`, which is why it's `0x8C` not `0x6C`).

## `cmdExistCheck` — exact final source (proven byte-exact)

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

Confirmed via `harness.diff_fn` against `wip/nand_thread/target_raw.txt`:
28/28 instructions, byte-for-byte, size `0x70` matches
`bin/dtk/wiimj2d_symbols.txt`.

## `cmdSpaceCheck` — exact final source (also proven byte-exact), confirming the two-field shape

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

27/27 instructions byte-for-byte, size `0x6C` matches the symbol map. This is
the shape to copy for `cmdDeleteFile` (`mState = 3`) and `cmdLoad`
(`mState = 5`) — same body, same size, only the `mState` literal differs.

## Helper calls, and the exact order (all five functions)

1. `bool locked = OSTryLockMutex(&mMutex.mOSMutex);` — `mMutex` is at object
   offset `0x50`, its `mOSMutex` is at `mMutex`+`0x04`, i.e. object offset
   `0x54`, matching `addi r3, r3, 0x54` in every one of these functions.
2. `if (locked) { ... }` — **do not write `if (!OSTryLockMutex(...)) return
   false;`.** That compiles to a plain `cmpwi`+`beq`, not the target's
   `neg r0,r3; or r0,r0,r3; srwi. r0,r0,31; beq`. The target's 3-instruction
   normalise-to-bool sequence only appears when `OSTryLockMutex`'s result is
   first **stored into a `bool` local** — assigning an arbitrary int-typed
   `BOOL` into a real `bool` forces MWCC to canonicalise it to 0/1, and the
   `srwi.` from that canonicalisation is reused directly as the branch
   condition (no separate compare instruction is emitted). This is a real
   lever, not a stylistic preference — confirmed by direct A/B compile.
3. Field writes (see table above).
4. `OSUnlockMutex(&mMutex.mOSMutex);` — **before** the signal, not after.
5. `OSSignalCond(&mMutex.mOSCond);` — `mOSCond` is at `mMutex`+`0x1C`, object
   offset `0x6C`, matching `addi r3, r31, 0x6c`.
6. `return true;` on the locked path, `return false;` if the lock failed.

## Header contradiction — report, do not fix yet

`include/game/bases/d_nand_thread.hpp` currently declares:
```cpp
void cmdExistCheck();
void cmdSpaceCheck();
```
Both are **actually `bool`**. The target's own bytes prove it: each function
ends with an explicit `li r3,0x1 / b <end>` on the success path and
`li r3,0x0` on the failure path converging at one shared epilogue — the
canonical codegen for `return true;` / `return false;`, not a void function
falling off the end. Verified independently for both functions (each
compiles byte-identical only when declared `bool` and given explicit
`return` statements). **This almost certainly applies to `cmdLoad` and
`cmdDeleteFile` too** — their disassembly (`wip/nand_thread/target_raw.txt`
lines 722–753 and 1017–1048) shows the exact same `li r3,0x1 / b ... / li
r3,0x0` pattern. Batches 3/4 should treat their `cmdXxx` functions as `bool`
as well and verify independently rather than assume.

I did **not** edit the real header (out of scope per the brief). Proved this
in a shadow copy: `wip/nand_thread/scratch/batch2/shadow/game/bases/d_nand_thread.hpp`.
Every caller of these functions in `d_s_boot.cpp` already discards the return
value, so the header fix is safe and has no caller-side fallout.

## Also found while proving this: `setNandError`'s real parameter type

`setNandError`'s mangled name in the target is
`setNandError__13dNandThread_cFl` — the trailing `l` is Watcom/MWCC mangling
for `long`, not `int`. The header currently declares
`void setNandError(s32 err);`, and `s32` is `signed int` (`include/types.h`
line 28), which mangles to `...Fi`, not `...Fl`. **This is a real mismatch,
confirmed against every `bl setNandError__13dNandThread_cFl` call site in
`target_raw.txt` and against `bin/dtk/wiimj2d_symbols.txt` line 5582.** The
parameter should be declared as plain `long`. Also shadow-tested, not applied
to the real header. This affects every batch that calls `setNandError`
(all four).
