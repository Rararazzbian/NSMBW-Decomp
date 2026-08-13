# Shared brief — `d_nand_thread.cpp`

Everybody working on this unit reads this file first. It is the single copy of
the facts; do not re-derive anything in it.

## The unit

`source/dol/bases/d_nand_thread.cpp`, class `dNandThread_c`, a background thread
that does NAND (Wii save-file) I/O. **24 functions, 0xF48 (3,912) bytes of code
in a 0xFE0 (4,064) byte `.text` span**, `0x800CED00`–`0x800CFCE0`. It is small.

Ground truth disassembly of the whole unit: **`wip/nand_thread/target_raw.txt`**
(from `bin/dtkspl/obj/auto_03_800CED00_text.o`, which covers the TU exactly and
nothing else). Every claim about instructions must come from that file.

## The compile/diff loop

Do **not** run `ninja`, `configure.py`, `progress.py` or `land.py`. Ever. The
lead is the only integrator; two builds in one checkout clobber each other.

Import `tools/auto_decomp/harness.py` and use `compile_draft` / `extract` /
`diff_fn`. Do not write your own compiler invocation and do not write your own
differ. `compile_draft(src, obj)` already encodes the required flags including
the seven `include\lib\revolution\BTE\...` paths.

```
bin\dtk-windows-x86_64.exe elf disasm <your>.o <your>.txt
```

**Extract by ADDRESS, not by name**, and assert `instruction_count * 4` against
the size in `bin/dtk/wiimj2d_symbols.txt`. Three of the six tooling defects
found on this project were invisible to a negative control.

Work in `wip/nand_thread/scratch/<your-batch>/`. Write nowhere else except your
own report.

## The class — already proven, do not change it

`include/game/bases/d_nand_thread.hpp` will hold this. It is derived from the
retail disassembly and from a compiled probe, and the vtables below confirm it.
**If you think it is wrong, report the contradiction — do not edit it.** If you
need to test a change, shadow-copy the header into your own scratch directory
and use `compile_draft(extra_inc=...)`.

```
dNandThread_c : EGG::Thread          sizeof 0x80
  0x00..0x4B   EGG::Thread base      sizeof(EGG::Thread) == 0x4C
  0x4C..0x4F   u8 mPad4C[4]          unreferenced by every function in the TU
  0x50..0x73   mMutex mMutex         sizeof 0x24
  0x74         int mState            command/state id
  0x78         int mError            NAND error code
  0x7C         bool mFileExists
  0x7D..0x7F   tail padding

mMutex : EGG::Mutex                  sizeof 0x24
  0x00  vptr (EGG::Mutex base)
  0x04  OSMutex mOSMutex  (0x18)
  0x1C  OSCond  mOSCond   (0x08)

EGG::Mutex                           sizeof 0x04, one virtual (its destructor)
```

**`mState` and `mError` keep those names.** The already-banked, already-matching
`source/dol/bases/d_s_boot.cpp` reads `m_instance->mState` and
`m_instance->mError` by those names in eight places. Renaming them to
`mCommand`/`mStatus` would churn a banked file for nothing.

### The three vtables, decoded from the DOL

These are facts, read out of the binary, and they pin the virtual order:

```
__vt__13dNandThread_c   0x80317D48  0x18
  +0x00 0
  +0x04 0
  +0x08 0x800CEEA0  dNandThread_c::~dNandThread_c
  +0x0C 0x800CFAC0  dNandThread_c::run
  +0x10 0x800CFCC0  EGG::Thread::onEnter
  +0x14 0x800CFCB0  EGG::Thread::onExit

__vt__6mMutex           0x80317D60  0x0C   -> 0x800CEE60 mMutex::~mMutex
__vt__Q23EGG5Mutex      0x80317D6C  0x0C   -> 0x800CEE20 EGG::Mutex::~EGG::Mutex
```

So **`EGG::Thread`'s virtual order is `~Thread`, `run`, `onEnter`, `onExit`**,
and `dNandThread_c` introduces **no new virtuals** — it overrides `~` and `run`
only, and inherits `onEnter`/`onExit`. That is why this TU emits the base's weak
copies of all three (see "Functions nobody authors").

## Section bounds — derived and cross-checked, use as given

Every one of these is hard-bracketed by a named neighbour on both sides.

| Section | Address range | Slice range | Size |
|---|---|---|---|
| `.text` | `0x800CED00`–`0x800CFCE0` | `0xc8580-0xc9560` | `0xFE0` |
| `.rodata` | `0x802F1470`–`0x802F1498` | `0x3490-0x34b8` | `0x28` |
| `.data` | **`0x80317CD8`**–`0x80317D78` | **`0x19638-0x196d8`** | `0xA0` |
| `.bss` | `0x80359FC0`–`0x80371000` | `0x8640-0x1f680` | `0x17040` |
| `.sdata` | `0x80427F78`–`0x80427F88` | `0x5f8-0x608` | `0x10` |
| `.sbss` | `0x8042A298`–`0x8042A2A0` | `0x3f8-0x400` | `0x8` |

No `.ctors`, no `.sdata2`, no `.sbss2`. There is no `__sinit` for this TU.

**The `.data` low bound was wrong in the pre-flight and is corrected here.** It
was recorded as `0x80317D48` (the first vtable). It is actually `0x80317CD8`,
`0x70` bytes lower, and the missing `0x70` is four objects this TU owns — see
the data inventory. Two independent arguments agree: `__vt__11dMultiMng_c` sits
at `0x80317CC8` and a class vtable is unconditionally the **terminal** `.data`
object of its TU, so the previous TU ends at `0x80317CD8`; and the pool IDs
`@67228`/`@67229`/`@67269`/`@67342` are consecutive with this TU's own known
`.sdata` object. A `0x70` shortfall in a small-data bound is the signature that
fails four of five binaries with thousands of scattered single-byte diffs and
nothing wrong in any function.

## Data inventory — every object, with its owner

Ordering in the source controls emission order. **Do not define an object that
is not assigned to you; reference it.** If you believe an object is missing from
this list, that is a finding — report it.

### `.rodata` `0x28` — three anonymous-namespace string constants

| Address | Size | Symbol | Contents | Owner |
|---|---|---|---|---|
| `0x802F1470` | `0x10` | `sc_TEMP_BANNER_FILE` | `"/tmp/banner.bin"` | LEAD (file top) |
| `0x802F1480` | `0x0B` | `sc_BANNER_FILE` | `"banner.bin"` | LEAD (file top) |
| `0x802F148C` | `0x0C` | `sc_GAME_FILE` | `"wiimj2d.sav"` | LEAD (file top) |

Their symbol-map names carry the `__27@unnamed@d_nand_thread_cpp@` suffix, so
they are **named objects in an anonymous namespace**, not pooled literals.

### `.data` `0xA0`

| Address | Size | Symbol | What it is | Owner |
|---|---|---|---|---|
| `0x80317CD8` | `0x0E` | `@66576` | `"save_icon.bti"` | BATCH 3 |
| `0x80317CE8` | `0x13` | `@67228` | `"save_banner_EU.bti"` | BATCH 3 |
| `0x80317CFC` | `0x0C` | `@67229` | `"save_banner"` | BATCH 3 |
| `0x80317D08` | `0x40` | `@67342` | **jump table**, 16 entries, all inside `setNandError` | BATCH 1 |
| `0x80317D48` | `0x18` | `__vt__13dNandThread_c` | vtable | automatic |
| `0x80317D60` | `0x0C` | `__vt__6mMutex` | vtable | automatic |
| `0x80317D6C` | `0x0C` | `__vt__Q23EGG5Mutex` | vtable | automatic |

`@67342`'s sixteen entries are `0x800CFC80 0x800CFC8C 0x800CFC8C 0x800CFC44
0x800CFC68 0x800CFC8C 0x800CFC5C 0x800CFC8C 0x800CFC8C 0x800CFC44 0x800CFC80
0x800CFC50 0x800CFC74 0x800CFC74 0x800CFC8C 0x800CFC44`. Every one is inside
`setNandError` (`0x800CFC20`, size `0x78`). **This is a `switch` on the NAND
error code with 16 cases, several sharing a body and one default** — that is
what `setNandError` has to be written as, and getting the switch shape right is
what produces the table. Do not hand-write the table.

### `.bss` `0x17040`

| Address | Size | Symbol | Owner |
|---|---|---|---|
| `0x80359FC0` | `0x4000` | `l_safeCopyBuf` (anon namespace) | LEAD (file top) |
| `0x8035DFC0` | `0x3FA0` | `l_tmpSave` (anon namespace) | LEAD (file top) |
| `0x80361F60` | `0xF0A0` | `@LOCAL@writeBanner...@a_banner@0` | BATCH 3 |

`a_banner` is a **function-local static inside `writeBanner`**, 61,600 bytes —
a full Wii save banner. Its `@0` suffix and `@LOCAL@` prefix mean exactly that.

### `.sdata` `0x10`

| Address | Size | Symbol | Contents | Owner |
|---|---|---|---|---|
| `0x80427F78` | `0x04` | `@LOCAL@writeBanner...@c_icon_res` | pointer to `0x80317CD8` (`"save_icon.bti"`) | BATCH 3 |
| `0x80427F7C` | `0x05` | `@67269` | `"SMNP"` | BATCH 3 |

`c_icon_res` is a function-local static in `writeBanner` **with a non-zero
initialiser**, which is why it is in `.sdata` and not `.bss`.

### `.sbss` `0x8`

`0x8042A298` `m_instance__13dNandThread_c`, `0x4`. Declared in the header,
defined once at file scope. LEAD owns it.

## Functions nobody authors

Five of the 24 emitted functions are **weak inline flushes** — the compiler
emits them because of the headers and the vtable, and they land in the right
place only if the declarations and the call sites are right.

**Do not hand-write these.** If one comes out in the wrong place or does not
come out at all, that is a finding — report it.

```
0x800CEE20  0x40  __dt__Q23EGG5MutexFv     EGG::Mutex::~Mutex
0x800CEE60  0x40  __dt__6mMutexFv          mMutex::~mMutex
0x800CFCB0  0x04  onExit__Q23EGG6ThreadFv  blr
0x800CFCC0  0x04  onEnter__Q23EGG6ThreadFv blr
0x800CFCD0  0x08  run__Q23EGG6ThreadFv     li r3,0 / blr
```

The tail three are the end-of-TU inline flush block, **emitted in strict reverse
declaration order** — `eggThread.h` declares `run`, `onEnter`, `onExit` and they
come out `onExit`, `onEnter`, `run`. That is a confirmation of the rule, not a
thing to adjust.

The two `Mutex` destructors are **not** in the tail block; they sit between the
constructor and `~dNandThread_c`. Whoever owns Batch 1 should note where they
actually land in the compiled object and report it, because that placement is a
structural fact the lead needs for assembly and nobody has measured it yet.

## The batches

| Batch | Functions | Code bytes |
|---|---|---|
| 1 | `__ct__`, `__dt__13dNandThread_cFv`, `run`, `create`, `setNandError`, `getSaveData` | `0x2A8` |
| 2 | `cmdExistCheck`, `existCheck`, `cmdSpaceCheck`, `spaceCheck` | `0x21C` |
| 3 | `fn_800CF170`, `save`, `createBanner`, `writeBanner` | `0x48C` |
| 4 | `cmdLoad`, `load`, `checkCRC`, `cmdDeleteFile`, `deleteFile` | `0x4A8` |

`fn_800CF170` (`0x800CF170`, `0x8C`) has **no name in the symbol map** — it is a
file-static or an inline the map did not name. Batch 3 must work out what it is
from its callers and its body, name it, and mark the name `@unofficial`.

### The `cmdXxx` functions are near-certainly one shape

`cmdExistCheck` `0x70`, `cmdSpaceCheck` `0x6C`, `cmdLoad` `0x6C`,
`cmdDeleteFile` `0x6C`, and probably `fn_800CF170` `0x8C`. Four of them are
within four bytes of each other. **Batch 2 owns `cmdExistCheck` and must report
its exact final source shape early**, because Batches 3 and 4 can then copy it
and change one constant. Do not wait until your final report to hand that over.

The expected shape, from the disassembly, is: lock the mutex, store the command
id into `mState`, signal the condition variable, unlock. Verify it; do not
assume it.

## Rules

- **Report contradictions, do not reconcile them.** If what you find disagrees
  with this brief, with the symbol map, or with another batch, stop and say so.
  Two agents disagreeing about a `sizeof` is how a `0x12C` error was caught that
  no per-function diff could see.
- **Do not edit a shared header, `slices/wiimj2d.json`, or `syms.txt`.** Propose.
- A negative result reported plainly is worth more than a positive one invented.
  Say "I could not close this and here is exactly where it diverges."
- Verify three ways, not one: byte equality, **size against the symbol map**, and
  where your function lands relative to its neighbours.
- Plain ASCII or clean UTF-8, LF, no BOM.
