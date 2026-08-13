# Closer-C report — `__ct__13dNandThread_cFiPQ23EGG4Heap`, `__dt__13dNandThread_cFv`

Scratch work lives in `wip/nand_thread/scratch/closer_c/`: `d_nand_thread.cpp` (the
draft, copied from Batch 1's `wip/nand_thread/scratch/batch1/d_nand_thread.cpp`
and modified only in the two ways described below), `shadow_include/` (one
shadow-copied header, see "Header change proposed"), and `test.py` (the
harness driver used for every result in this report — `compile_draft` /
`disasm` / `diff_fn` / `list_functions`, nothing homemade). Nothing outside
`wip/nand_thread/` was touched; no shared header, `slices/wiimj2d.json`, or
`syms.txt` was edited.

## Result summary

| Function | Target size | Result |
|---|---|---|
| `__ct__13dNandThread_cFiPQ23EGG4Heap` | 0x118 | **MATCH — 70/70 instructions, byte-exact** |
| `__dt__13dNandThread_cFv` | 0x64 | **24/25 words — still 1 instruction short**, characterised below, not closed |

The brief's instruction was to re-measure the constructor against the current
header before assuming it was still short. It was still short — the landed
`mMutex()` change (calling `OSInitMutex`/`OSInitCond` in its own body) closed
the *interleaving* problem Batch 1 diagnosed but did not by itself close the
byte count; re-verified directly (see "Constructor" below) before doing
anything else, per the brief's instruction.

## Constructor — now MATCH

### What was wrong, re-measured

Compiled Batch 1's draft verbatim against the *current* (already-updated)
header — no changes of my own yet. Result: **67/70 instructions**, short by
exactly one word, and it is the **same** dead-store gap Batch 1 reported
before the `mMutex()` header change landed:

```
   15 | want: lis r4, __vt__Q23EGG5Mutex@ha        got: lis r4, __vt__6mMutex@ha
   18 | want: addi r4, r4, __vt__Q23EGG5Mutex@l    got: addi r4, r4, __vt__6mMutex@l
   22 | want: lis r4, __vt__6mMutex@ha             got: addi r3, r27, 0x6c
```

So the brief's warning was correct to raise but the landed header change did
not close this by itself: with `mMutex()`'s body now calling
`OSInitMutex`/`OSInitCond` directly, MWCC still puts the derived class's own
vtable store immediately adjacent to the base class's vtable store (nothing
observable between them), and dead-store elimination removes the base's
store — same failure mode Batch 1 already fully characterised, just moved
into `mMutex`'s own constructor instead of `dNandThread_c`'s.

### The fix: `OSInitMutex` belongs to `EGG::Mutex`'s constructor, not `mMutex`'s

Re-reading the target's instruction order for the whole mutex-setup sequence:

```
26 lis r4, __vt__Q23EGG5Mutex@ha
29 addi r4, r4, __vt__Q23EGG5Mutex@l
30 stw r4, 0x50(r27)        <- EGG::Mutex's OWN vtable stored (base subobject)
31 addi r3, r27, 0x54       <- &mOSMutex
32 bl OSInitMutex             <- called HERE, before mMutex's own vtable store
33 lis r4, __vt__6mMutex@ha
35 addi r4, r4, __vt__6mMutex@l
36 stw r4, 0x50(r27)        <- mMutex's OWN vtable stored (SAME slot, second write)
37 bl OSInitCond
```

`OSInitMutex` is called **between** the two vtable stores, not after both. A
call sandwiched between them is an *opaque* external call (this TU cannot see
`OSInitMutex`'s body), and MWCC will not eliminate a store as dead across an
opaque call it cannot prove doesn't observe that memory — this is the same
"opaque call blocks dead-store elimination" mechanism, just needing the call
positioned correctly relative to the two stores.

For `OSInitMutex` to land textually between "store base vtable" and "store
own vtable", it cannot be in `mMutex`'s own constructor body — a derived
class's own vtable is set upon entering its own constructor, before any of
*its* body statements run, so anything **`mMutex`** writes in its own body
necessarily comes **after** both vtable stores are already adjacent. The only
place code can run **between** the base constructor finishing and the
derived constructor's own vtable store completing is **inside the base
constructor itself**. So `EGG::Mutex`'s constructor must take the
`OSMutex*` and call `OSInitMutex` itself:

```cpp
namespace EGG {
class Mutex {
public:
    Mutex(OSMutex *mutex) { OSInitMutex(mutex); }
    virtual ~Mutex() {}
};
}

class mMutex : public EGG::Mutex {
public:
    mMutex() : EGG::Mutex(&mOSMutex) {
        OSInitCond(&mOSCond);
    }
    virtual ~mMutex() {}
    OSMutex mOSMutex;
    OSCond mOSCond;
};
```

This reproduces the target's shape exactly: `EGG::Mutex(&mOSMutex)` runs
first — stores the base vtable, computes `&mOSMutex` (`r27+0x54`, since
`&mOSMutex` is a legal address-of on a not-yet-constructed-but-already-live
POD member — no read occurs, only its address is taken), calls
`OSInitMutex`. Control returns to `mMutex`'s own constructor, which stores
`mMutex`'s own vtable (now no longer dead — the intervening call blocks the
elimination) and then runs its own body, `OSInitCond(&mOSCond)`.

Compiling this alone (with everything else unchanged from Batch 1) closed
the vtable-store gap completely but left a **register swap** in the copy
loop:

```
   53 | want: addi r30, r30, 0x980   got: addi r31, r31, 0x980
   55 | want: addi r31, r31, 0x980   got: addi r30, r30, 0x980
```

i.e. `tempGame` (`r30`) must be incremented **before** `saveGame` (`r31`) at
the bottom of the loop. Batch 1's draft had:

```cpp
saveGame += sizeof(dMj2dGame_c);
tempGame += sizeof(dMj2dGame_c);
```

Swapping the two lines —

```cpp
tempGame += sizeof(dMj2dGame_c);
saveGame += sizeof(dMj2dGame_c);
```

— closed it. **70/70 instructions, byte-exact.** Verified three ways:
`diff_fn` reports `MATCHING (70 instructions)`; `extract()`'s instruction
count × 4 = `0x118`, matching `bin/dtk/wiimj2d_symbols.txt`'s
`__ct__13dNandThread_cFiPQ23EGG4Heap ... size:0x118` exactly; and the
function's position in the emitted object is first, as expected (see
"Structural check" below).

### Final source — constructor (unchanged from Batch 1 except the one swapped pair of lines)

```cpp
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
        tempGame += sizeof(dMj2dGame_c);
        saveGame += sizeof(dMj2dGame_c);
    }

    memcpy(l_tmpSave, buf, sizeof(l_tmpSave));
}
```

### Header change proposed (not applied to the real header — shadow-copied and tested only)

`include/game/bases/d_nand_thread.hpp`, `EGG::Mutex`'s constructor takes the
`OSMutex*` it initialises, and `mMutex`'s constructor passes `&mOSMutex` to
it via the base-initialiser list instead of calling `OSInitMutex` itself:

```cpp
namespace EGG {
class Mutex {
public:
    Mutex(OSMutex *mutex) { OSInitMutex(mutex); }
    virtual ~Mutex() {}
};
}

class mMutex : public EGG::Mutex {
public:
    mMutex() : EGG::Mutex(&mOSMutex) {
        OSInitCond(&mOSCond);
    }
    virtual ~mMutex() {}
    OSMutex mOSMutex;
    OSCond mOSCond;
};
```

Tested via `compile_draft(extra_inc=['wip/nand_thread/scratch/closer_c/shadow_include'])`
against the full shadow-copied `d_nand_thread.hpp`
(`wip/nand_thread/scratch/closer_c/shadow_include/game/bases/d_nand_thread.hpp`).
Nothing else in the header changes: `sizeof(EGG::Mutex) == 0x4`,
`sizeof(mMutex) == 0x24`, `sizeof(dNandThread_c) == 0x80`, and the vtable
contents are untouched — only the constructor's calling convention for
`OSInitMutex` moves from `mMutex`'s body to `EGG::Mutex`'s. I re-ran all four
of Batch 1's other already-matching functions (`run`, `create`,
`setNandError`, `getSaveData`) against this shadow header to confirm nothing
regressed — all four still MATCH (see "Structural check" below).

## Destructor — still 1 instruction short, not closed

### Re-confirmed diff (unchanged from Batch 1's finding, header changes are irrelevant to this function)

```
    8 | want: beq .L|41820028|    got: beq .L|41820024|
   11 | want: beq .L|4182000C|    got: li r4, 0x0            [everything after is a position shift, content identical]
```

Concretely, want vs. got, aligned:

```
want (target)                          got (draft)
------------------------------------   ------------------------------
beq end            (this==0 guard)     beq end
li r0, 0x0                             li r0, 0x0
stw r0, m_instance@sda21               stw r0, m_instance@sda21
beq L_800CEED8      <-- MISSING        li r4, 0x0
li r4, 0x0                             bl __dt__Q23EGG6ThreadFv
bl __dt__Q23EGG6ThreadFv               cmpwi r31, 0x0
L_800CEED8:                            ble end
cmpwi r31, 0x0                         mr r3, r30
ble end                                bl __dl__FPv
mr r3, r30                             ... (return sequence)
bl __dl__FPv
... (return sequence)
```

The target has a second `beq` testing the same `this == 0` condition
(`cmpwi r3, 0` from function entry, still live in `cr0` — nothing between the
two branches touches it) immediately before the call to
`EGG::Thread::~Thread()` (`bl __dt__Q23EGG6ThreadFv`). Nothing observable
happens between the first `beq` and the second, so the second is provably
dead by ordinary dataflow reasoning — the only way to reach it is via the
path where the first `beq` was *not* taken (`this != 0`), so it can never be
taken. MWCC's own build kept it; mine (correctly, by the same reasoning)
optimises it away.

### What was tried and ruled out this round (on top of Batch 1's own attempt)

1. **Batch 1's exact `if (this) { m_instance = nullptr; }` wrap**, reproduced
   independently: gives a 26-word function, an extra `beq` in the wrong
   place (guarding the body statement, not the base-dtor call), and an
   extra `mr r3, r30` that doesn't belong — confirms Batch 1's finding
   rather than adding anything new.
2. **Explicit `mMutex.~mMutex();`** as a body statement (before or after
   `m_instance = nullptr;`): compiles, but MWCC compiles an *explicit*
   pseudo-destructor call on an object (not a pointer) through the vtable —
   `lwzu`/`lwz`/`mtctr`/`bctrl` — rather than a direct non-virtual call. That
   is not what the target does (the target has **zero** instructions
   attributable to `mMutex`'s teardown at all — its guard-and-call, if ever
   generated by the implicit member-destruction step, folds away completely
   because with the delete-flag hardcoded to 0 and `this` known non-null,
   every branch in it is statically decidable). Writing it explicitly is
   provably the wrong mechanism, not just a wrong result.
3. **Explicit `EGG::Thread::~Thread();`** as an extra body statement: compiles,
   but produces a **second, real** call to the base destructor (double
   destruction) on top of the implicit one the compiler still inserts
   automatically — 30 words, clearly wrong, and it also re-stores
   `dNandThread_c`'s own vtable a second time as part of setting up the
   explicit qualified call. Confirms that base-class destruction is not
   something that can be re-triggered from source without duplicating it.
4. **`dNandThread_c::m_instance = nullptr;`** (explicitly qualified) and
   **routing the store through a local `dNandThread_c *self = this;`**:
   byte-identical to the unmodified 24-word result — no effect on codegen at
   all.
5. **An extra, semantically-inert field write** (`mFileExists = false;`
   appended after `m_instance = nullptr;`): this is the one experiment that
   changed the *instruction count* to 25 (matching target's size), but the
   content is wrong — the target's second `beq` does not appear; instead the
   extra `stb r0, 0x7c(r3)` for `mFileExists` occupies that word, and nothing
   in the brief or the disassembly evidences the destructor ever touching
   `mFileExists`. This is a coincidental length match, not a fix, and I did
   not keep it.

None of these produce the target's exact shape: a second, provably-redundant
`this == 0` check specifically guarding the call to `EGG::Thread::~Thread()`,
with nothing else disturbed. Every C++-legal way of nudging the source either
leaves the codegen unchanged (attempts 4) or forces MWCC down a visibly
different and wrong path (attempts 1, 2, 3) rather than reproducing the
target's redundant-but-silent guard.

### What this looks like, structurally

`EGG::Thread::~Thread()` is not defined in this TU (only declared,
`virtual ~Thread();`, in `eggThread.h`) — it is a genuinely opaque call from
this compilation's point of view, same as `OSInitMutex` was for the
constructor. But unlike the constructor's fix, there is no analogous "move
the call into a different subobject's method" restructuring available here:
a base class's destructor is invoked automatically, exactly once, at a fixed
point (after the derived class's own body and member destructors, before the
delete-flag check) — C++ gives no legal way to write a *second*,
independent-looking guard around that specific, singular, compiler-inserted
call without either (a) duplicating the call (attempt 3) or (b) guarding
some other statement instead and having the compiler treat that as a
completely different, already-tried shape (attempt 1). I could not find a
third option. This reads as a case, like the ones Batch 1 flagged, where
MWCC's redundant-branch folding did not fire on the target's original
compile for reasons internal to the compiler (plausibly: the local guard the
front end synthesizes for each destructor-call "item" being emitted before a
whole-function redundant-branch pass runs, and MWCC's optimizer not
re-visiting already-folded control flow after a *different*, adjacent guard
collapses to nothing) — not a defect in how the source is being read.
Reporting a clean, single-instruction, unclosed gap rather than guessing
further or reaching for an offset cast.

### Final source — destructor (unchanged from Batch 1)

```cpp
dNandThread_c::~dNandThread_c() {
    m_instance = nullptr;
}
```

## Structural check requested by the lead — Mutex destructor placement

Re-measured directly from the compiled object's emitted order (not assumed),
using the **current** header (with the `EGG::Mutex(OSMutex*)` change from
this report applied via shadow-copy) — this needed re-checking after the
`mMutex` constructor changed, per the brief:

```
1. __ct__13dNandThread_cFiPQ23EGG4Heap     size 0x118   MATCH
2. __dt__Q23EGG5MutexFv                    weak,  size 0x40   (EGG::Mutex::~Mutex)
3. __dt__6mMutexFv                         weak,  size 0x40   (mMutex::~mMutex)
4. __dt__13dNandThread_cFv                 size 0x60 (target 0x64, 1 short)
5. run__13dNandThread_cFv                  size 0xdc    MATCH
6. create__13dNandThread_cFPQ23EGG4Heap    size 0x78    MATCH
7. setNandError__13dNandThread_cFl         size 0x78    MATCH
8. getSaveData__13dNandThread_cFv          size 0xc     MATCH
9. onExit__Q23EGG6ThreadFv                 weak
10. onEnter__Q23EGG6ThreadFv               weak
```

**Confirmed: `__dt__Q23EGG5MutexFv` and `__dt__6mMutexFv` still land between
the constructor and `~dNandThread_c`, each exactly `0x40` bytes**, unchanged
from Batch 1's original measurement and matching the target's address order
from the brief's "Functions nobody authors" table
(`0x800CEE20`/`0x800CEE60`, both `0x40`, both between `__ct__` at
`0x800CED00` and `__dt__13dNandThread_cFv` at `0x800CEEA0`). The
`EGG::Mutex(OSMutex*)` constructor change does not touch either weak
destructor's body, declaration, or the vtables, so this placement was not
expected to move, and it did not.

## Rules checklist

- Byte equality: constructor 1/1 exact now; destructor still short by
  exactly one instruction, fully characterised, not closed.
- Size vs. symbol map: constructor `0x118` == `bin/dtk/wiimj2d_symbols.txt`'s
  recorded size, exactly; destructor `0x60` emitted vs. `0x64` recorded, the
  short-by-4-bytes gap corroborated three independent ways (`diff_fn`,
  instruction count, and this report's manual alignment above).
- Function order: measured directly from the compiled object twice (once
  before, once after the header proposal), both matching target's address
  order; the two weak Mutex destructors' placement was re-verified per the
  lead's explicit request.
- No shared header, `slices/wiimj2d.json`, or `syms.txt` was edited. The one
  header change is a proposal, shadow-copied to
  `wip/nand_thread/scratch/closer_c/shadow_include/game/bases/d_nand_thread.hpp`
  and tested only via `compile_draft(extra_inc=[...])`.
- A negative result reported plainly: the destructor's one-instruction gap
  is not closed. Five source variants were tried beyond Batch 1's one; none
  reproduced the target's exact shape; each result and why it fails is
  recorded above rather than glossed over.
