# BATCH: d_a_wm_kinoko_1up.cpp (d_basesNP.rel)

## Executive summary

- `.text` bound (`0x16b0f0-0x16b2d0`, 9 functions, 412 code bytes) is confirmed
  by both peer surveys (`GEMINI_round10.md` unit 6, `GEMINI_round11.md` 1.6)
  **and** independently re-derived here from `bin/dtk/d_basesNP_symbols.txt`
  address arithmetic. No changes to that bound.
- `.rodata`, `.data`, `.bss` are confirmed against `GEMINI_round11.md`'s numbers
  (round10's numbers for these three sections were wrong — shifted by 0x24,
  0x24 and 0x8 respectively).
- **`.ctors` is wrong in BOTH surveys.** Both claimed `0x3f8-0x3fc`. The real
  slot, read directly out of the REL's own relocation table and confirmed by
  the dedicated split object's own `.section .ctors` comment, is
  **`0x3fc-0x400`** — one slot later. See "The `.ctors` finding" below.
- **The base class's `sizeof` in `GEMINI_round11.md` is wrong.** It claims
  `sizeof(daWmKinokoBase_c) == 0x2B0`, proven (their words) by a compiled
  `STATIC_ASSERT`. The classInit function for the base itself
  (`fn_2_16B2D0`, disassembled fresh in this round) allocates `li r3, 0x290`,
  already contradicting round11. Neither number turns out to be exactly
  right, though: the true value, reverse-engineered from a
  component-by-component layout and confirmed by a compiled `STATIC_CHECK`,
  is **`0x284`**. See "The sizeof finding" below.
- Verifier result: **8 of 9 functions byte-identical** (modulo symbol names).
  The 9th (`vf84`) has the right opcodes, right registers per role, right
  store order — but two instruction *pairs* are scheduling-swapped relative
  to the target. Documented below with the exact residual diff; not resolved
  after an exhaustive 4-way source-order sweep plus a const-removal fix that
  DID close two other functions.

---

## 1. Bounds: both checks, run myself

### 1.1 `.text` — re-derived independently

`bin/dtk/d_basesNP_symbols.txt` lists every function (even anonymous
`fn_2_*`) with address and size. Summing the 9 consecutive functions from
`0x16b0f0` up to (not including) `0x16b2d0`:

```
0x16b0f0  0x30
0x16b120  0x44
0x16b170  0x58
0x16b1d0  0x04
0x16b1e0  0x04
0x16b1f0  0x1c
0x16b210  0x0c
0x16b220  0x84
0x16b2b0  0x1c
----------------
sum       0x1a4 = 412 bytes  (matches "412 B code" in both surveys)
span      0x16b2d0-0x16b0f0 = 0x1e0 = 480 bytes  (matches "480 B span")
```

The function immediately before (`fn_2_16B0D0`, ends `0x16b0ec`) references
`lbl_2_data_45640` — a *different* TU's copy of `dWmLib::sc_ForceList` (see
§3) — confirming `0x16b0f0` is a real TU boundary, not an artifact of the
`.text` gap. **`.text: 0x16b0f0-0x16b2d0`, base `0x0` (REL section offsets
are direct; `d_basesNP.rel`'s `meta.baseAddr` is not subtracted for split
symbol tables, unlike the DOL).**

### 1.2 Overlap-and-adjacency, run against `slices/d_basesNP.json`

Only 13 TUs are landed in `d_basesNP.rel`; none is adjacent to this unit
(nearest landed neighbours are `d_a_wm_dokan_route.cpp` well below and
`d_a_wm_peach.cpp` well above), so the adjacency half of the check has no
landed neighbour to compare against. The overlap half was run programmatically
for all 5 sections against all 13 slices:

```python
proposed = {
    '.text':   (0x16b0f0, 0x16b2d0),
    '.ctors':  (0x3fc,    0x400),
    '.rodata': (0x8ab8,   0x8ac8),
    '.data':   (0x457c8,  0x458c0),
    '.bss':    (0xfe70,   0xfe80),
}
# for each: zero overlaps with any of the 13 landed slices
```
Result: **zero overlaps on every section.** (Full script output in the
session; omitted here for length — all 5 lines printed `overlaps: []`.)

### 1.3 `.rodata`, `.data`, `.bss` — confirmed via hard evidence, not survey trust

Both surveys disagree with each other on these three sections (round10 vs
round11 differ by exactly the size of one intervening object in each case),
which is exactly the situation the task brief warned about. Rather than pick
one, I derived them from the actual referenced symbols in the target
disassembly (`bin/dtkspl/d_basesNP/obj/auto_00_0016B0C4_text.o`,
`auto_fn_2_16B220_text.o`, `auto_00_0016B2A4_text.o`, all disassembled with
`dtk elf disasm`) and cross-checked every referenced object's address/size
against `bin/dtk/d_basesNP_symbols.txt`:

- `.rodata`: only object referenced by any of the 9 functions is
  `lbl_2_rodata_8AB8` (size `0x10`, the `mVec3_c` float triple used by the
  TU's `dWmLib::sc_ForceList` copy — see §3). `.rodata: 0x8ab8-0x8ac8`.
  Matches round11; **round10's `0x8b58-0x8b70` is wrong.**
- `.data`: walking the symbol table in address order from
  `lbl_2_data_457C8` (the TU's own `sc_ForceList`, size `0x24`) through
  `g_profile_WM_KINOKO_1UP` (`0x457EC`, size `0xC`) and the string/pointer
  objects that follow, the block ends cleanly at `0x458C0`, which is exactly
  where the *next* TU's own `sc_ForceList` copy (`lbl_2_data_458C0`) begins —
  the same per-TU-header-static pattern repeating. `.data: 0x457c8-0x458c0`
  (248 B). Matches round11; **round10's `0x457ec-0x458e4` is wrong** (it
  starts the range at the profile symbol instead of at the leading
  `sc_ForceList` object, which precedes the profile by `0x24` bytes).
- `.bss`: `lbl_2_bss_FE70` (size `0xC`, the `__register_global_object`
  bookkeeping node) + `lbl_2_bss_FE7C` (size `0x4`, the TU's own
  `dWmLib::c_StartPointKinokoHouseID` copy) = `0x10` total.
  `.bss: 0xfe70-0xfe80`. Matches round11; **round10's `0xfe78-0xfe88` is
  wrong.**

### 1.4 The `.ctors` finding — wrong in both surveys

Both surveys state `.ctors: 0x3f8-0x3fc`. I did not trust this and instead
read the REL's own relocation table:

```
bin\dtk-windows-x86_64.exe rel info -r original/d_basesNP.rel
```

Filtering for `Absolute` relocations whose source section is `.ctors` and
target section is `.text` (module 2, both same module), sorted by source
address, around this TU:

```
0x3f8 -> 0x16b040   (previous TU's sinit)
0x3fc -> 0x16b220   (THIS TU's sinit)
0x400 -> 0x16bcf0   (daWmKinokoBase_c's sinit, per GEMINI_round11.md's own
                      function table, entry 16: __sinit_\d_a_wm_kinoko_base_cpp)
0x404 -> 0x16bec0   (next TU's sinit)
```

This is confirmed a second, independent way: disassembling
`bin/dtkspl/d_basesNP/obj/auto_fn_2_16B220_text.o` (the dedicated split
object for this TU's `fn_2_16B220`, which turns out to be its `__sinit`, see
§3) directly, dtk emits the `.ctors` slot as part of the object:

```
# 0x000003FC..0x00000400 | size: 0x4
.section .ctors, "a"
.balign 4
	.4byte fn_2_16B220
```

**`.ctors: 0x3fc-0x400`, not `0x3f8-0x3fc`.** Both surveys are off by one
slot (4 bytes) low. This does not change my `.text` bound (0x16b0f0-0x16b2d0
is still right) but it is a real error in both surveys' landing kits and
would have written a wrong `.ctors` bound if trusted, exactly the kind of
survey error the task brief warned might exist.

---

## 2. The `sizeof` finding

`GEMINI_round11.md` states, with a compiled `STATIC_ASSERT` as evidence:
`sizeof(daWmKinokoBase_c) == 0x2B0`, with a layout ending
`mAllocator, mResFile, mModel, mChrAnim[2], mChrBlend, m_284, m_288, m_28C,
u8 mPad[0x20]`.

Two independent hard facts contradict this:

1. **The base's own classInit** (`fn_2_16B2D0`, in
   `bin/dtkspl/d_basesNP/obj/auto_00_0016B2A4_text.o`, address matches
   round11's own function table entry 1 exactly) allocates:
   ```
   li r3, 0x290
   bl __nw__7fBase_cFUl
   ```
   `0x290`, not `0x2B0`.
2. **This leaf's own classInit** (`fn_2_16B0F0`, this unit) allocates
   `li r3, 0x294`. A derived class can never be *smaller* than its base. If
   `daWmKinokoBase_c` were really `0x2B0`, `daWmKinoko1up_c` at `0x294` would
   be impossible — yet this leaf demonstrably derives from
   `daWmKinokoBase_c` (its own constructor, `fn_2_16B120`, calls
   `bl fn_2_16B300`, which is round11's own address for
   `daWmKinokoBase_c::daWmKinokoBase_c()`).

Neither `0x290` nor `0x2B0` turned out to be exactly right either — compiling
a component-by-component reconstruction (same `dHeapAllocator_c`/`mdl_c`/
`anmChr_c[]`/`anmChrBlend_c` members already used, matching layout, by the
landed `daWmPeachCastle_c` in
`include/game/bases/d_a_wm_peach_castle.hpp`) and checking each component's
real `sizeof` via a compiled template trick revealed
**`sizeof(m3d::anmChrBlend_c) == 0x28`, not `0x2C`** as round11 assumed. With
the corrected component size:

```
sizeof(dWmObjActor_c)        = 0x188  (compiled, confirmed)
+ mAllocator  dHeapAllocator_c        0x1C
+ mResFile    nw4r::g3d::ResFile      0x04
+ mModel      m3d::mdl_c              0x40
+ mChrAnim[2] m3d::anmChr_c[2]        0x70
+ mChrBlend   m3d::anmChrBlend_c      0x28   <- was assumed 0x2C
+ mUnk280     u32 (zeroed by the base ctor, confirmed: `stw r31,0x280(r30)`
               in fn_2_16B300 with r31=0)    0x04
---------------------------------------------
= 0x284
```

Compiled and verified:
```cpp
STATIC_CHECK<sizeof(daWmKinokoBase_c) == 0x284> checkBase;   // PASSED
STATIC_CHECK<sizeof(daWmKinoko1up_c) == 0x294> check1up;     // PASSED
```

`daWmKinoko1up_c` (this leaf) adds 4 fields beyond the base's `0x284`:
`mUnk284` (never written by any of this TU's functions — plausibly set
elsewhere, or genuinely unused; left `@unofficial`), `mAnimResNames`
(`const char *const *`, written by `vf84()`, confirmed by
`stw r5,0x288(r3)` after an `addi` — array-decay, not a load), `mModelResName`
(`const char *`, written by `vf84()`, confirmed by `stw r0,0x28c(r3)` after
an `lwz` — a dereferenced load, matching a plain `const char *` variable, not
an array), and `mFlag` (zeroed by the constructor: `stw r0,0x290(r31)` with
r0=0).

**Report, not reconcile:** the true base-class size is neither survey's
number. `0x2B0` (round11) is wrong by `0x2C`; `0x290` (a naive read of the
base's own classInit alone) is also wrong by `0xC`, because two of what
round11 called base members (`m_288`/`m_28C`) are actually **this leaf's
own** members, not the base's. This only became visible by reading the
*leaf's* own field-setting function (`vf84`, `fn_2_16B1F0`) and noting the
base constructor (`fn_2_16B300`) never touches those two offsets itself —
only the one at `+0x280`.

---

## 3. The `dWmLib::sc_ForceList` finding (this TU's `__sinit`)

`fn_2_16B220` (33 instructions) and `fn_2_16B2B0` (the array-destructor
helper) turned out to need **zero hand-written source**. They are the
compiler-synthesized static-initializer and static-destructor-thunk for a
**header static already declared in `include/game/bases/d_wm_lib.hpp`**:

```cpp
static ForceInCourseList_t sc_ForceList[] = {
    {WORLD_7, "F7C0", WORLD_7, dCsvData_c::c_CASTLE_ID, 4, "W7C0",
     mVec3_c(2160.0f, -30.0f, -478.0f)}
};
static int c_StartPointKinokoHouseID = dCsvData_c::c_START_ID;
```

`ForceInCourseList_t` is `{int, const char*, int, int, int, const char*,
mVec3_c}` = `0x24` bytes, matching the object size measured at
`lbl_2_data_457C8` exactly. This is the same "header static with a
non-trivial initializer gets a fresh copy in every TU that includes it"
pattern AGENT_CONTEXT documents (it names 30 copies of one such object
elsewhere in the binary) — this TU's copy is one of that family. Simply
`#include <game/bases/d_wm_lib.hpp>` and both functions were emitted
automatically, and (after the const-removal fix below) matched byte-for-byte.

---

## 4. Verifier output (verbatim)

```
python wip/wm_units/verify_anon.py wip/wm_kinoko_1up/scratch/d_a_wm_kinoko_1up.txt \
    0x16b0f0 0x16b2d0 \
    bin/dtkspl/d_basesNP/obj/auto_00_0016B0C4_text.o \
    bin/dtkspl/d_basesNP/obj/auto_fn_2_16B220_text.o \
    bin/dtkspl/d_basesNP/obj/auto_00_0016B2A4_text.o

addr       target                  size  result
0x0016b0f0 fn_2_16B0F0               12  MATCH  <- daWmKinoko1up_c_classInit__Fv
0x0016b120 fn_2_16B120               17  MATCH  <- __ct__15daWmKinoko1up_cFv
0x0016b170 fn_2_16B170               22  MATCH  <- __dt__15daWmKinoko1up_cFv
0x0016b1d0 fn_2_16B1D0                1  MATCH  <- vf7C__15daWmKinoko1up_cFv
0x0016b1e0 fn_2_16B1E0                1  MATCH  <- vf80__15daWmKinoko1up_cFv
0x0016b1f0 fn_2_16B1F0                7  5 differing vs vf84__15daWmKinoko1up_cFv
0x0016b210 fn_2_16B210                3  MATCH  <- getModelName__15daWmKinoko1up_cFv
0x0016b220 fn_2_16B220               33  MATCH  <- "__sinit_\d_a_wm_kinoko_1up_cpp"
0x0016b2b0 fn_2_16B2B0                7  MATCH  <- __arraydtor$12483

8/9 byte-identical modulo symbol names
```

Compiled with `harness.compile_draft(..., module='d_basesNP')`, i.e. the
REL's own flags (`-O4,p -sdata 0 -sdata2 0 -char signed`), read from
`slices/d_basesNP.json`'s `meta.defaultCompilerFlags` — not the DOL's flags.

### A fix that closed two functions along the way

Initial draft used `static const char *const smc_animResNames[...]` /
`static const char *const smc_modelResName` (fully `const`) for the two
locals `vf84()` sets. That put them in `.rodata`, which (a) meant `vf84`
itself referenced a `.rodata` symbol where the target references `.data`
symbols (`lbl_2_data_45808`/`lbl_2_data_45810`), and (b) pushed the TU's
`sc_ForceList` float literal to offset `+0x10` of a merged `.rodata` pool
object instead of a dedicated `0x0`-based one, which broke `__sinit`'s
`lfs` instructions (`0x10(r5)` instead of `0x0(r5)`). Dropping the outer
`const` (`static const char *smc_animResNames[...]`, `static const char
*smc_modelResName` — pointer itself not const, matching the
"`T *const arr[]` is const-qualified and lands in `.rodata`, dropping the
outer `const` moves it" lesson in AGENT_CONTEXT) moved both into `.data` and
fixed the `.rodata` pool layout, which closed `__sinit` outright and fixed
the operand symbols in `vf84`.

### The one open residual: `vf84`

```
TARGET:                                    DRAFT (best of 4 source-order variants):
  lis r5, lbl_2_data_45808@ha                lis r4, SYM(smc_modelResName)@ha
  lis r4, lbl_2_data_45810@ha                lis r5, SYM(smc_animResNames)@ha
  addi r5, r5, lbl_2_data_45808@l            lwz r0, SYM(smc_modelResName)@l(r4)
  stw r5, 0x288(r3)                          addi r5, r5, SYM(smc_animResNames)@l
  lwz r0, lbl_2_data_45810@l(r4)             stw r5, 0x288(r3)
  stw r0, 0x28c(r3)                          stw r0, 0x28c(r3)
  blr                                         blr
```

Right instruction count (7), right opcodes, **right register per role**
(r5 = the array/`addi` operand -> `+0x288`; r4/r0 = the scalar/`lwz` operand
-> `+0x28c`; this is not a bare register-renumbering — the *roles* match),
right store order. The only difference is which pair of independent
instructions (`lis`+`lis`, then `lwz` vs `addi`) MWCC schedules first. I
swept all 4 combinations of (declaration order of the two statics) x
(assignment-statement order) — `anim-first`/`anim-first`,
`anim-first`/`model-first`, `model-first`/`anim-first`,
`model-first`/`model-first` — and an inlined-setter variant
(`void setResNames(...) { ... }` defined in-class, called from `vf84`). In
every variant the "model" operand's `lis` and its `lwz` are scheduled before
the "anim" operand's `addi`+store, regardless of source order — this looks
like a fixed MWCC scheduling preference (hoist the load) that source-level
statement reordering in this shape cannot reach. Reported as an open
residual rather than resolved; kept the variant with matching register roles
and store order (`anim`-first in both declaration and assignment) since it
is the closest and most natural to read.

---

## 5. Proposed class / header

Both classes are defined directly in the draft `.cpp` (not in a shared
header) because `daWmKinokoBase_c` is not landed. This is a **proposal**,
not an edit to any real header, per the ground rules. If/when
`d_a_wm_kinoko_base.cpp` lands, `daWmKinokoBase_c` belongs in
`include/game/bases/d_wm_kinoko_base.hpp`, matching this layout (corrected
sizeof `0x284`, not round11's `0x2B0`).

```cpp
#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_chr_blend.hpp>
#include <game/mLib/m_3d/mdl.hpp>

/// @unofficial Provisional reconstruction of the shared base class for the
/// world-map Kinoko-house markers (1-Up / Red / Star). Not landed yet; the
/// pre-flight in peer_archive/GEMINI_round11.md claimed sizeof == 0x2B0, but
/// that is contradicted by the classInit operator-new literal for both this
/// class and the derived leaf. sizeof verified by compiled STATIC_CHECK
/// against a component-by-component layout, matching the same
/// dHeapAllocator_c/mdl_c/anmChr_c[]/anmChrBlend_c pattern already landed in
/// daWmPeachCastle_c (include/game/bases/d_a_wm_peach_castle.hpp).
class daWmKinokoBase_c : public dWmObjActor_c {
public:
    enum ANIM_e {
        ANIM_0, ///< @unofficial name
        ANIM_1, ///< @unofficial name
        ANIM_COUNT
    };

    daWmKinokoBase_c();
    virtual ~daWmKinokoBase_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);
    virtual void vf7C();
    virtual void vf80();
    virtual void vf84();
    virtual const char *getModelName();

    void createModel();
    void calcModel();
    void mode_exec();

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;
    m3d::mdl_c mModel;
    m3d::anmChr_c mChrAnim[ANIM_COUNT];
    m3d::anmChrBlend_c mChrBlend;
    u32 mUnk280; ///< @unofficial zeroed by the base constructor; purpose unknown
};

/// @brief The actor for the 1-Up Mushroom Kinoko-house marker on the World Map.
/// @unofficial class name; every symbol in this unit is anonymous (fn_2_*) in
/// the map, so nothing pins the real name. daWmKinoko1up_c mirrors the sibling
/// naming convention (daWmKinokoBase_c/daWmKinokoRed_c/daWmKinokoStar_c).
class daWmKinoko1up_c : public daWmKinokoBase_c {
public:
    daWmKinoko1up_c();
    virtual ~daWmKinoko1up_c();

    virtual void vf7C();
    virtual void vf80();
    virtual void vf84();
    virtual const char *getModelName();

    u32 mUnk284; ///< @unofficial never written by this TU's own functions
    const char *const *mAnimResNames; ///< @unofficial set by vf84(); consumed by daWmKinokoBase_c::createModel()
    const char *mModelResName; ///< @unofficial set by vf84(); consumed by daWmKinokoBase_c::createModel()
    u32 mFlag; ///< @unofficial zeroed by the constructor
};
```

---

## 6. Source (this TU)

`wip/wm_kinoko_1up/scratch/d_a_wm_kinoko_1up.cpp`, compiled clean with
`module='d_basesNP'`:

```cpp
#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_chr_blend.hpp>
#include <game/mLib/m_3d/mdl.hpp>

/* ... daWmKinokoBase_c / daWmKinoko1up_c class bodies exactly as in section 5 ... */

ACTOR_PROFILE(WM_KINOKO_1UP, daWmKinoko1up_c, 0);

daWmKinoko1up_c::daWmKinoko1up_c() {
    mFlag = 0;
}

daWmKinoko1up_c::~daWmKinoko1up_c() {}

void daWmKinoko1up_c::vf7C() {}
void daWmKinoko1up_c::vf80() {}

void daWmKinoko1up_c::vf84() {
    static const char *smc_animResNames[ANIM_COUNT] = {
        "wm_1up_kinoko_appear", ///< @unofficial placeholder text; real bytes unverified
        "wm_1up_kinoko_wait",   ///< @unofficial placeholder text; real bytes unverified
    };
    static const char *smc_modelResName = "wm_1up_kinoko"; ///< @unofficial placeholder text

    mAnimResNames = smc_animResNames;
    mModelResName = smc_modelResName;
}

const char *daWmKinoko1up_c::getModelName() {
    return "wm_1up_kinoko"; ///< @unofficial placeholder text; real bytes unverified
}
```

**`@unofficial` — string content is a placeholder.** `verify_anon.py`
normalizes symbol operands, so instruction-level matching (the thing this
round verifies) does not depend on the actual bytes of these strings — but a
real landing needs the true text. Not extracted this round: the raw string
bytes live in `.data` at `lbl_2_data_457F8` (size `0x10`),
`lbl_2_data_45808`/`lbl_2_data_45810` (the two anim-name/model-name
pointers, sizes `0x8` each), and `lbl_2_data_458A0` (size `0xD`, the
`getModelName()` return string) — reading them out of
`original/d_basesNP.rel` at file offset `0x1D0C00 + address` would settle
this in one pass, flagged here as a follow-up rather than guessed.

---

## 7. Section table (final, for landing)

```json
{
    "source": "d_basesNP/bases/d_a_wm_kinoko_1up.cpp",
    "memoryRanges": {
        ".text": "0x16b0f0-0x16b2d0",
        ".ctors": "0x3fc-0x400",
        ".rodata": "0x8ab8-0x8ac8",
        ".data": "0x457c8-0x458c0",
        ".bss": "0xfe70-0xfe80"
    }
}
```

| Section | Range | Size | Offset-perturbing? |
|---|---|---|---|
| `.text` | `0x16b0f0-0x16b2d0` | 480 B (412 B code) | No — matches both surveys, re-derived independently by address arithmetic |
| `.ctors` | `0x3fc-0x400` | 4 B | **Corrects both surveys** (they said `0x3f8-0x3fc`); verified two independent ways (REL relocation table + dedicated split object's own `.section .ctors` output) |
| `.rodata` | `0x8ab8-0x8ac8` | 16 B | Confirms round11, corrects round10 |
| `.data` | `0x457c8-0x458c0` | 248 B | Confirms round11, corrects round10 |
| `.bss` | `0xfe70-0xfe80` | 16 B | Confirms round11, corrects round10 |

Zero overlaps with any of the 13 currently-landed `d_basesNP` slices, checked
programmatically for all 5 sections (§1.2). No landed neighbour is adjacent
on either side, so adjacency could not be cross-checked against a second
landed TU — only the overlap half of the check had material to run against.

---

## 8. Not landed this round — do not run `land.py`

8 of 9 functions are proven byte-exact; `vf84` (fn_2_16B1F0, 7 instructions)
is an open residual with the diff shown in §4. Per the "whole TU or nothing"
rule this unit is **not ready to land**. Everything above is for review; the
draft, size-check scratch files, and disassembly captures are all under
`wip/wm_kinoko_1up/scratch/`.
