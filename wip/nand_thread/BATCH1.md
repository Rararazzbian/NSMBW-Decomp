# Batch 1 report — `__ct__`, `__dt__`, `run`, `create`, `setNandError`, `getSaveData`

Scratch work lives in `wip/nand_thread/scratch/batch1/`: `d_nand_thread.cpp` (the
draft below), `shadow_include/` (three shadow-copied headers, see "Header
changes proposed"). Nothing outside `wip/nand_thread/` was touched.

Compiled with `harness.compile_draft(..., extra_inc=['wip/nand_thread/scratch/batch1/shadow_include'])`.
**Against the unmodified real headers the draft does not compile at all** —
`OSGetThreadPriority(OSThread*)` does not match the declared
`OSGetThreadPriority()` — so the shadow directory is load-bearing, not
cosmetic. Details and proof for each shadowed header are below.

## Result summary

| Function | Target size | Result |
|---|---|---|
| `__ct__13dNandThread_cFiPQ23EGG4Heap` | 0x118 | **67/70 words** — 1 instruction short, characterised below |
| `__dt__13dNandThread_cFv` | 0x64 | **24/25 words** — 1 instruction short, characterised below |
| `run__13dNandThread_cFv` | 0xDC | **MATCH** |
| `create__13dNandThread_cFPQ23EGG4Heap` | 0x78 | **MATCH** |
| `setNandError__13dNandThread_cFl` | 0x78 | **MATCH** |
| `getSaveData__13dNandThread_cFv` | 0xC | **MATCH** |

4 of 6 close byte-exact. The other 2 are each short by exactly **one
instruction that MWCC proves dead and elides in my build but that survives in
the target** — not a wrong-instruction diff, an extra-word diff. Both are
written up in detail below because I could not close them and want the exact
shape on record rather than a vague "close but not exact."

Verified three ways for every function: (1) byte/word equality via
`harness.diff_fn`, (2) `extract()`'s instruction count × 4 against
`bin/dtk/wiimj2d_symbols.txt` (table above — the two non-matches are short by
exactly 4 bytes, i.e. exactly the one missing instruction, nothing else), (3)
emitted symbol order (below).

## Final source

```cpp
#include <game/bases/d_nand_thread.hpp>
#include <game/bases/d_save_mng.hpp>
#include <game/mLib/m_heap.hpp>
#include <lib/revolution/NAND.h>
#include <lib/revolution/OS.h>

// ---------------------------------------------------------------------------
// Batch-1 scratch stand-in for objects this batch does not own. LEAD owns the
// real definitions at file top of d_nand_thread.cpp; these exist here only so
// this TU compiles standalone. Do not bank this block.
namespace {
    u8 l_safeCopyBuf[0x4000];
    u8 l_tmpSave[0x3fa0];
}

dNandThread_c *dNandThread_c::m_instance;
// ---------------------------------------------------------------------------

dNandThread_c::dNandThread_c(int msgCount, EGG::Heap *heap)
    : EGG::Thread(0x4000, 0, msgCount, heap) {
    mState = 0;
    m_instance = this;

    u8 *saveGame;
    u8 *tempGame;
    u8 buf[0x3fa0] ALIGN(32);
    dSaveMng_c *saveMng = dSaveMng_c::m_instance;
    memcpy(buf, &saveMng->mHeader, sizeof(dMj2dHeader_c));
    saveGame = buf + sizeof(dMj2dHeader_c);
    tempGame = buf + sizeof(dMj2dHeader_c) + 3 * sizeof(dMj2dGame_c);

    for (s8 i = 0; i < 3; i++) {
        memcpy(saveGame, saveMng->getSaveGame(i), sizeof(dMj2dGame_c));
        memcpy(tempGame, saveMng->getTempGame(i), sizeof(dMj2dGame_c));
        saveGame += sizeof(dMj2dGame_c);
        tempGame += sizeof(dMj2dGame_c);
    }

    memcpy(l_tmpSave, buf, sizeof(l_tmpSave));
}

dNandThread_c::~dNandThread_c() {
    m_instance = nullptr;
}

void *dNandThread_c::run() {
    OSLockMutex(&mMutex.mOSMutex);
    for (;;) {
        mState = 0;
        OSWaitCond(&mMutex.mOSCond, &mMutex.mOSMutex);
        switch (mState) {
        case 1:
            existCheck();
            break;
        case 2:
            spaceCheck();
            break;
        case 4:
            while (save() == 2) {}
            break;
        case 5:
            while (load() == 2) {}
            break;
        case 3:
            deleteFile();
            break;
        case 6:
            OSUnlockMutex(&mMutex.mOSMutex);
            return 0;
        }
    }
}

void dNandThread_c::create(EGG::Heap *heap) {
    EGG::Heap *prevHeap = mHeap::setCurrentHeap(heap);
    dNandThread_c *thread = new dNandThread_c(OSGetThreadPriority(OSGetCurrentThread()) - 1, nullptr);
    mHeap::setCurrentHeap(prevHeap);
    // EGG::Thread does not yet name the field at +0x8 (inside mPad); it holds
    // the OSThread handle. Unofficial cast pending eggThread.h attribution.
    OSResumeThread(*(OSThread **)((u8 *)thread + 0x8));
}

void dNandThread_c::setNandError(long err) {
    switch (err) {
    case NAND_RESULT_NOEXISTS:
    case NAND_RESULT_EXISTS:
    case NAND_RESULT_OK:
        mError = 0;
        break;
    case NAND_RESULT_CORRUPT:
        mError = 1;
        break;
    case NAND_RESULT_MAXBLOCKS:
        mError = 2;
        break;
    case NAND_RESULT_MAXFILES:
        mError = 3;
        break;
    case NAND_RESULT_BUSY:
    case NAND_RESULT_ALLOC_FAILED:
        mError = 4;
        break;
    case NAND_RESULT_AUTHENTICATION:
    case NAND_RESULT_ECC_CRIT:
        mError = 6;
        break;
    default:
        mError = 5;
        break;
    }
}

void *dNandThread_c::getSaveData() {
    return l_tmpSave;
}
```

## Per-function detail

### `run__13dNandThread_cFv` — MATCH

Straight `switch (mState)` inside `for(;;)`, no jump table (only 6 cases,
non-contiguous 1,2,4,5,3,6 — MWCC emits a compare chain, and the compare
**order in the chain is exactly the case declaration order**, so the case
labels above must stay 1, 2, 4, 5, 3, 6 in that order or the branch chain
reorders and stops matching). `default:`/unmatched falls through to the loop
top with no code of its own, matching the target's plain `b` to the loop head
with no explicit default case needed. Confirms the brief's per-command IDs:
1=existCheck, 2=spaceCheck, 4=save (loops while it returns 2), 5=load (same),
3=deleteFile, 6=exit or worker.

**This function is the direct evidence for `save()`'s and `load()`'s return
type** — see "Return-type findings for Batch 3/4" below, which the lead asked
for by name.

### `create__13dNandThread_cFPQ23EGG4Heap` — MATCH

Straightforward `mHeap::setCurrentHeap` / `new` / restore-heap /
`OSResumeThread` shape, matching the precedent in `d_cd.cpp`'s
`dCd_c::createInstance`. Two things this function proves:

- The constructor's `int` first parameter is populated here as
  `OSGetThreadPriority(OSGetCurrentThread()) - 1` — see "Contradiction:
  the ctor's `int` parameter" below.
- `EGG::Thread` has an unnamed field at `+0x8` holding an `OSThread*` — see
  "Header change 3" below.

### `setNandError__13dNandThread_cFl` — MATCH

Case mapping, derived from the jump table in the brief plus the disassembly,
cross-checked against `include/lib/revolution/NAND/nand.h`'s `NANDResult`:

| `err` (raw NAND result) | Constant | `mError` |
|---|---|---|
| `-15` | `NAND_RESULT_AUTHENTICATION` | `6` |
| `-14` | `NAND_RESULT_OPENFD` | `5` (default) |
| `-13` | `NAND_RESULT_NOTEMPTY` | `5` (default) |
| `-12` | `NAND_RESULT_NOEXISTS` | `0` |
| `-11` | `NAND_RESULT_MAXFILES` | `3` |
| `-10` | `NAND_RESULT_MAXFD` | `5` (default) |
| `-9` | `NAND_RESULT_MAXBLOCKS` | `2` |
| `-8` | `NAND_RESULT_INVALID` | `5` (default) |
| `-7` | *(no named constant — a gap in the enum)* | `5` (default) |
| `-6` | `NAND_RESULT_EXISTS` | `0` |
| `-5` | `NAND_RESULT_ECC_CRIT` | `6` |
| `-4` | `NAND_RESULT_CORRUPT` | `1` |
| `-3` | `NAND_RESULT_BUSY` | `4` |
| `-2` | `NAND_RESULT_ALLOC_FAILED` | `4` |
| `-1` | `NAND_RESULT_ACCESS` | `5` (default) |
| `0` | `NAND_RESULT_OK` | `0` |
| anything else (`< -15` or `> 0`) | — | `5` (default) |

**Getting MWCC to reuse ONE body address for cases sharing a result requires C
fallthrough grouping** (`case A: case B: case C: mError = N; break;`), not
separate case blocks each doing `mError = N; break;` — the latter compiles to
one *correct but separate* body per case (10 distinct bodies instead of 6),
which still produces a 16-slot jump table but makes it 30 bytes too long
(42 vs 30 words) because nothing dedupes the identical bodies. Grouping fixed
it in one shot, and **case declaration order in the source is emission order
for the bodies**, which had to match the target's address order
`0x…44`(0)→`50`(1)→`5C`(2)→`68`(3)→`74`(4)→`80`(6)→`8C`(5, default, last) — the
switch above declares them in exactly that order for that reason, not
alphabetically or by raw value.

### `getSaveData__13dNandThread_cFv` — MATCH

One line, returns `l_tmpSave`.

### `__ct__13dNandThread_cFiPQ23EGG4Heap` — 67/70, one instruction short

**Diff** (only difference; everything else realigns and matches once this one
word is accounted for — see the "one instruction" note directly below):

```
   15 | want: lis r4, __vt__Q23EGG5Mutex@ha        got: lis r4, __vt__6mMutex@ha
   18 | want: addi r4, r4, __vt__Q23EGG5Mutex@l    got: addi r4, r4, __vt__6mMutex@l
   22 | want: lis r4, __vt__6mMutex@ha             got: addi r3, r27, 0x6c      [rest is a position shift]
```

Concretely: the target stores `EGG::Mutex`'s own vtable into `mMutex` at
`+0x50`, calls `OSInitMutex`, **then** stores `mMutex`'s own vtable into the
same `+0x50` slot, then calls `OSInitCond`. My build stores `EGG::Mutex`'s
vtable and `mMutex`'s vtable into the same address with nothing observable
between them, so MWCC's dead-store elimination removes the first (dead) store
outright — correct on its own terms (nothing reads `+0x50` between the two
writes), just not what the target's compile did.

I traced this to `mMutex`'s own constructor, not `dNandThread_c`'s. The header
currently declares `mMutex() {}` (empty). Putting `OSInitMutex(&mOSMutex)` /
`OSInitCond(&mOSCond)` **inside `mMutex`'s own constructor body** (tested via
shadow copy, not applied to the real header) reproduces the target's exact
interleaving of the two vtable stores with the two `OSInit*` calls — proof
that this is genuinely where those calls live in the source, not an inlining
artifact of where I happened to write them. What it does **not** do is stop
MWCC eliminating the redundant `EGG::Mutex` vtable store; I could not find a
source phrasing of the trivial 2-line `mMutex` constructor that defeats that
specific optimisation. This is a genuine, precisely one-instruction (4-byte)
gap, not a wrong-content diff — see "Header change 2" below for the tested
`mMutex` body, which I recommend adopting anyway since it is otherwise
byte-for-byte correct and is the more accurate class definition regardless of
this one elided store.

Everything else in the function — the constructor's real content, structure,
and **register allocation** — matches exactly once the missing word is
subtracted (verified by comparing `want[i]` against `got[i-1]` for `i` past
the gap: every remaining line is identical, including the loop's register
choices `r29`=`saveMng`, `r31`=`saveGame`, `r30`=`tempGame`, `r28`=loop index,
which took five rounds of declaration-order experiments to land exactly:
`saveGame`/`tempGame` must be **declared (as bare pointers) before `buf` and
`saveMng`**, but **assigned** only after the header `memcpy`, or the register
numbers land on the wrong locals even though the code is semantically
identical either way).

### `__dt__13dNandThread_cFv` — 24/25, one instruction short

**Diff**:

```
    8 | want: beq .L|41820028|    got: beq .L|41820024|
   11 | want: beq .L|4182000C|    got: li r4, 0x0            [everything after is a position shift, content identical]
```

The target has **two** `beq`s testing `this == 0` — the outer one (guarding
the whole function, standard for any destructor reachable through `delete`)
and a second one, immediately after storing `m_instance = 0`, that is
**provably always false at that point** (the same `cmpwi r3, 0` result is
still live, and the only path that reaches the second `beq` is the one where
the first `beq` was *not* taken, i.e. `r3 != 0`). My build produces the first
check and omits the dead second one — again correct on its own terms.

I tried writing the second check explicitly (`if (this) { m_instance =
nullptr; }`) and it does **not** reproduce the target: it adds a check but at
the wrong place, changing the branch target of the *outer* check too and
costing 2 words instead of saving one (tested, not kept — see
`wip/nand_thread/scratch/batch1/` history if this needs revisiting). The
9-instruction chain after the (missing) second `beq` — `li r4,0` /
`bl __dt__Q23EGG6ThreadFv` / `cmpwi r31,0` / `ble` / `mr r3,r30` /
`bl __dl__FPv` / restore / `blr` — is otherwise identical in both content and
register choice. I could not find a source shape producing the second check
without disturbing the first; reporting this as a clean, single-instruction
open gap rather than guessing further.

## Structural finding 1 — the constructor's ~0x3FE0-byte aligned frame

**Fully explained, not just characterised.** The `clrlwi r11,r1,27` /
`subfic r11,r11,-0x3fe0` / `stwux` prologue is MWCC's standard idiom for a
**32-byte-aligned dynamically-sized stack frame** — `clrlwi r11,r1,27` computes
`r1 mod 32` (keeping only the low 5 bits) and `subfic` folds that misalignment
into the same instruction that allocates the frame, so this is a **proof**,
not an inference, that something on the stack needs 32-byte alignment.

That something is a `u8 buf[0x3fa0] ALIGN(32);` local — confirmed by compiling
an isolated repro (`u8 buf[0x3fa0] ALIGN(32);` inside an otherwise-trivial
function) and getting byte-identical prologue shape, differing only in the
exact frame size because that test had no other locals. `ALIGN(32)` is not
invented syntax — it is `include/types.h`'s existing
`#define ALIGN(x) __attribute__((aligned(x)))`, already used for exactly this
purpose in `source/dol/bases/d_main.cpp:32` (`u8 stack[STACK_SIZE] ALIGN(32);`).

What the buffer is for: it is an **on-stack staging copy of the entire NAND
save blob**, laid out to mirror `l_tmpSave` exactly —

```
buf[0x000..0x6a0)   dMj2dHeader_c            (sizeof confirmed == 0x6a0 by compiled static_assert)
buf[0x6a0..0x2320)  dMj2dGame_c[3]  (save)   (sizeof(dMj2dGame_c) confirmed == 0x980, ×3 == 0x1c80)
buf[0x2320..0x3fa0) dMj2dGame_c[3]  (temp)   (same size again)
```

`0x6a0 + 0x1c80 + 0x1c80 == 0x3fa0`, which is exactly `l_tmpSave`'s size from
the brief's data inventory, and the constructor's last act is
`memcpy(l_tmpSave, buf, 0x3fa0)` — one wholesale flush of everything just
staged. `sizeof(dMj2dHeader_c) == 0x6a0` and `sizeof(dMj2dGame_c) == 0x980`
are not assumed; both were confirmed by compiling
`typedef char c[sizeof(dMj2dHeader_c) == 0x6a0 ? 1 : -1];`-style static
assertions against the real, already-banked
`include/game/bases/d_mj2d_data.hpp` / `source/dol/bases/d_mj2d_data.cpp`.

So: large local buffer (not an alignment requirement alone, not an inlined
callee) forces the frame size, and a genuine 32-byte alignment requirement on
that buffer forces the dynamic-align prologue shape. Why 32 specifically
rather than the default 8 is not proven here — plausibly matching whatever
alignment `l_tmpSave` itself carries, since the flush is a single wholesale
`memcpy` between the two — but that is inference, not proof, and is flagged
as such.

## Structural finding 2 — the two weak `mMutex`/`EGG::Mutex` destructors

**Measured directly from my compiled object's emitted order**:

```
1. __ct__13dNandThread_cFiPQ23EGG4Heap     global
2. __dt__Q23EGG5MutexFv                    weak,  size 0x40   (EGG::Mutex::~Mutex)
3. __dt__6mMutexFv                         weak,  size 0x40   (mMutex::~mMutex)
4. __dt__13dNandThread_cFv                 global
5. run__13dNandThread_cFv                  global
6. create__13dNandThread_cFPQ23EGG4Heap    global
7. setNandError__13dNandThread_cFl         global
8. getSaveData__13dNandThread_cFv          global
9. onExit__Q23EGG6ThreadFv                 weak
10. onEnter__Q23EGG6ThreadFv               weak
```

This is an **exact match** to the target's address order and to the brief's
"Functions nobody authors" table: both weak Mutex destructors land **between**
the constructor and `~dNandThread_c`, each is exactly `0x40` bytes, and
nothing else moves. I did not write either destructor — they are emitted
purely because `mMutex`/`EGG::Mutex` are declared with virtual destructors and
the vtables reference them. No finding to flag here; the placement is exactly
what the brief predicted, now backed by a real compile rather than an
assumption.

## Header changes proposed

None of these were applied to the real headers — all were shadow-copied into
`wip/nand_thread/scratch/batch1/shadow_include/` and tested with
`compile_draft(extra_inc=[...])`.

**1. `include/lib/egg/core/eggThread.h` — `Thread`'s first constructor
parameter is `unsigned long`, not `u32`.**
Proven, not guessed: with the header as `Thread(u32 stackSize, ...)`, calling
`EGG::Thread(0x4000, 0, msgCount, heap)` compiles to a call to
`__ct__Q23EGG6ThreadFUiiiPQ23EGG4Heap` (`Ui` = unsigned int). The target calls
`__ct__Q23EGG6ThreadFUliiPQ23EGG4Heap` (`Ul` = unsigned long). `u32` is
`unsigned int` in this codebase's `types.h`, so it mangles wrong regardless of
argument value. Changing only the first parameter's declared type to
`unsigned long` (or `ulong`, already typedef'd) makes the constructor call
match exactly, with no other change needed. This is a pure type-mangling fix,
not a layout change.

**2. `include/game/bases/d_nand_thread.hpp` — `mMutex`'s constructor should
initialise its own members, not be empty.**
```cpp
mMutex() {
    OSInitMutex(&mOSMutex);
    OSInitCond(&mOSCond);
}
```
Evidence: see "`__ct__13dNandThread_cFiPQ23EGG4Heap` — 67/70" above. This
reproduces the target's exact interleaving of vtable stores and `OSInit*`
calls (proving these calls live in `mMutex`'s own constructor, not
`dNandThread_c`'s body) but does not by itself close the one-word gap (a
provably-dead store MWCC elides regardless of where the calls are written).
Recommended anyway: it is more accurate than the current empty body and
changes nothing else about the class's proven layout or vtables.

**3. `include/lib/revolution/OS/OSThread.h` — `OSGetThreadPriority` takes an
`OSThread*`.**
```c
int OSGetThreadPriority(OSThread *thread);
```
Proven, not guessed: the real, unmodified header (`OSGetThreadPriority(void)`)
makes `create__13dNandThread_cFPQ23EGG4Heap` **fail to compile outright** —
`OSGetThreadPriority(OSGetCurrentThread())` does not match a zero-argument
declaration. The target's disassembly calls `OSGetCurrentThread` immediately
before `OSGetThreadPriority` with the result still live in `r3`, which only
makes sense if the second call consumes it. Real Revolution SDK also declares
this function as taking an `OSThread*`. This is the one change in this batch
that is a hard compile-time contradiction, not a byte-level nicety.

**4. `include/lib/egg/core/eggThread.h` — unnamed field at `+0x8` (not
applied, no confident name proposed).**
`create()`'s tail does `OSResumeThread(*(OSThread**)((u8*)thread + 0x8))`
(`+0x8` inside `EGG::Thread`, i.e. `mPad[4]`) — some field holding the
`OSThread*` handle populated by construction. I used a raw offset cast rather
than inventing a member name, per the "cast only when genuinely
undecompiled" rule — `EGG::Thread` is only partially decompiled (`mPad[0x48]`
blob) and I don't have independent evidence for the field's exact bounds or
what else might live near it. Flagging for whoever picks up `EGG::Thread`
properly rather than guessing a name now.

## Contradiction: the constructor's `int` parameter is not `msgCount`

The header names `dNandThread_c`'s own constructor parameter `msgCount`
(`dNandThread_c(int msgCount, EGG::Heap *heap)`), and this batch's job was
**not** to rename it (mangling doesn't encode parameter names, so there's no
proof from the symbol map either way). But `create()` — this batch's only
caller of this constructor — passes `OSGetThreadPriority(OSGetCurrentThread())
- 1` for it, and passes a hard-coded `0` for the base `EGG::Thread`
constructor's own `msgCount`-slot (position 2 of `Thread(stackSize, ?, ?,
heap)`) while forwarding *this* parameter into position 3 (`priority` by
`eggThread.h`'s own naming). Both pieces of evidence point the same way: this
parameter is a **thread priority**, not a message count. Reporting as a
contradiction between the header's chosen name and what its one caller
actually does with it — not fixing it, since it is a naming-only question the
type system can't adjudicate and it's outside this batch's scope to touch the
header.

## Return-type findings for Batch 3/4 (per the lead's relay)

`run()` is the only witness for several of these — a return type never
appears in the symbol map under this ABI. What `run()` actually shows, per
callee:

- **`save()`** — tested via `cmpwi r3, 0x2` after the call, looping while
  true. **Not a truth test.** Already confirmed `s32` and landed by the lead;
  my `run()` matches byte-exact using the real (now-fixed) header.
- **`load()`** — **identical pattern to `save()`**: `cmpwi r3, 0x2` after the
  call, looped the same way (`while (load() == 2) {}`). This is exactly the
  same evidence class the lead used to fix `save()`, and it points to the same
  conclusion: **`load()` should be `s32`, not `bool`.** The real header still
  says `bool load();` as of this writing; I compiled `run()` against a shadow
  copy declaring `int load();` to get the match above, but did **not** touch
  the real header since `load()` belongs to Batch 4. This needs the same fix
  `save()` got.
- **`existCheck()`**, **`spaceCheck()`**, **`deleteFile()`** — all three are
  called in `run()` with their return value **completely discarded** (no
  compare, no branch on `r3` at all, straight back to the loop top). `run()`
  provides **zero evidence** on these three's return types either way; the
  header's current `bool` for all three is neither confirmed nor contradicted
  by anything in this batch's functions.
- **`checkCRC()`** — not called by `run()` at all (only by `load()`, which is
  Batch 4's function, not mine). No evidence from this batch.

## Rules checklist

- Byte equality: 4/6 exact, 2/6 short by exactly one instruction each, both
  fully characterised above with the precise diff, not just "close."
- Size vs. symbol map: table at the top; the two non-matches are short by
  exactly 4 bytes each — corroborates "one missing instruction," not a
  structural miscount.
- Symbol order: measured directly from the compiled object (structural
  finding 2), exact match to target address order.
- Contradictions reported, not reconciled: the ctor parameter's real meaning
  vs. its header name; `load()`'s return type vs. the currently-landed header.
- No shared header, `slices/wiimj2d.json`, or `syms.txt` was edited. All
  header changes are proposals in `wip/nand_thread/scratch/batch1/shadow_include/`.
