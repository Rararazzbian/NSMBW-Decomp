# Batch B report -- d_a_wm_smallcloud.cpp, last eight functions

Scope: `0x179D50`-`0x179FEC` (8 functions: `0x179D50`, `0x179E00`, `0x179EA0`,
`0x179EB0`, `0x179EC0`, `0x179F10`, `0x179F40`, `0x179FD0`).

Working files:
- Draft source: `wip/wm_smallcloud/scratch/batchB/d_a_wm_smallcloud.cpp`
- Shadow class header (scratch only, NOT for landing as-is):
  `wip/wm_smallcloud/scratch/batchB/shadow_include/game/bases/d_a_wm_smallcloud.hpp`
- Shadow `d_a_wm_map.hpp` with the proposed new overload:
  `wip/wm_smallcloud/scratch/batchB/shadow_include/game/bases/d_a_wm_map.hpp`
- Compile/verify driver: `wip/wm_smallcloud/scratch/batchB/run.py`
- Target objects disassembled for this batch:
  `bin/dtkspl/d_basesNP/obj/auto_00_001797B4_text.o` (covers `0x1797B4`-`0x179F40`),
  `bin/dtkspl/d_basesNP/obj/auto_00_00179FC4_text.o` (covers `0x179FC4`-`0x17AFC4`),
  `bin/dtkspl/d_basesNP/obj/auto_fn_2_179F40_text.o` (the single function `0x179F40`-`0x179FC4`,
  its own weak-symbol object -- **not** covered by either of the two above; found only by
  `find bin/dtkspl/d_basesNP/obj -iname "*179F*"`).

## Verifier table (final)

Produced by `python wip/wm_smallcloud/scratch/batchB/run.py`, which wraps
`wip/wm_units/verify_anon.py wip/wm_smallcloud/scratch/batchB/draft.txt 0x1797e0 0x179ff0 <3 objects above>`.
Rows outside my scope (`0x1797E0`-`0x179BB0`, batch A's) are included because the
tool always walks the whole `[lo, hi)` range; they show "differing" against my
functions only because the tool falls back to reporting the closest unused
candidate when there is no real match -- that is expected, not a defect.

```
addr       target                  size  result
0x001797e0 fn_2_1797E0               12  12 differing vs init_exec__16daWmSmallCloud_cFv
0x00179810 fn_2_179810               31  29 differing vs processCutsceneCommand__16daWmSmallCloud_cFib
0x00179890 fn_2_179890               57  52 differing vs __dt__7mVec3_cFv
0x00179980 fn_2_179980               62  58 differing vs initStateLike__16daWmSmallCloud_cFv
0x00179a80 fn_2_179A80               59  56 differing vs initStateLike__16daWmSmallCloud_cFv
0x00179b70 fn_2_179B70               12  12 differing vs init_exec__16daWmSmallCloud_cFv
0x00179ba0 fn_2_179BA0                2  2 differing vs mode_exec__16daWmSmallCloud_cFv
0x00179bb0 fn_2_179BB0              101  98 differing vs initStateLike__16daWmSmallCloud_cFv
0x00179d50 fn_2_179D50               44  MATCH  <- calcModel__16daWmSmallCloud_cFv
0x00179e00 fn_2_179E00               40  MATCH  <- initStateLike__16daWmSmallCloud_cFv
0x00179ea0 fn_2_179EA0                3  MATCH  <- init_exec__16daWmSmallCloud_cFv
0x00179eb0 fn_2_179EB0                1  MATCH  <- mode_exec__16daWmSmallCloud_cFv
0x00179ec0 fn_2_179EC0               17  MATCH  <- processCutsceneCommand__16daWmSmallCloud_cFib
0x00179f10 fn_2_179F10               10  MATCH  <- setPosFromCourseNode__16daWmSmallCloud_cFv
0x00179f40 fn_2_179F40               33  5 differing vs "__sinit_\d_a_wm_smallcloud_cpp"
0x00179fd0 fn_2_179FD0                7  MATCH  <- __arraydtor$12581

7/16 byte-identical modulo symbol names
```

**7 of my 8 target functions are confirmed byte-exact matches** (`0x179D50`,
`0x179E00`, `0x179EA0`, `0x179EB0`, `0x179EC0`, `0x179F10`, `0x179FD0`).
`0x179F40` is structurally 100% correct -- the only remaining diffs are two
verifier normalisation artifacts and a `.rodata` pool offset that depends on
content batch A owns (see its own section below).

I compiled with the flags `d_basesNP.json`'s `meta.defaultCompilerFlags`
actually specifies (`-sdata 0 -sdata2 0 -O4,p -char signed -rtti off`, plus
`-DREVOLUTION -I-`), **not** `tools/auto_decomp/harness.py`'s generic
`compile_draft` flags, which omit `-sdata 0 -sdata2 0` and use `-O4` instead
of `-O4,p`. Under the harness's default flags several float-constant loads got
routed through `@sda21` small-data addressing that the real `.rel` build never
uses, producing spurious diffs on every function that touches a float literal.
Confirmed by testing the same source under both flag sets (see "Two changes
that closed 179E00" below). **Anyone else driving this unit through
`harness.compile_draft` directly, without over-riding `CFLAGS`, will see
false negatives on any function with a float literal.**

## Method

Read `source/d_basesNP/bases/d_a_wm_cloud.cpp` and its header
(`include/game/bases/d_a_wm_cloud.hpp`) first, per the brief. Then
disassembled the twin's own compiled object
(`wip/wm_smallcloud/scratch/batchA/cloud_compiled.txt`, already present from
batch A's exploration) and diffed it function-by-function against my target's
disassembly. Every one of my functions matched a twin function's *shape*
(same offsets, same call sequence) closely enough to identify it before
writing a line of source, exactly as the brief describes. This is the
strongest evidence in this report by a wide margin -- most of what follows was
reading, not guessing.

## Per-function findings

### `0x179D50` -- `calcModel()`
- Evidence: instruction-for-instruction identical to the twin's
  `calcModel__11daWmCloud_cFv` (44/44 instructions, same registers, same
  literal offsets: `0x7c` mMatrix, `0xac` mPos, `0x100` mAngle, `0xdc` mScale,
  `0x1ac` mModel). All of those are `dBaseActor_c` base-class fields already
  landed and unchanged; `0x1ac` is `daWmSmallCloud_c`'s own `mModel`.
- Proposal: literally the twin's `calcModel()` body, copied verbatim onto the
  new class.
- Compiled: YES, MATCH.
- Confidence: high (byte-exact).
- Offset-perturbing: NO -- every offset used already exists in a landed base
  class or is read-only evidence about `daWmSmallCloud_c`'s own layout.

### `0x179EA0` -- `init_exec()`
- Evidence: `li r0,0; stw r0,0x1f4(r3); blr`, identical to the twin's
  `init_exec__11daWmCloud_cFv`.
- Proposal: `mCurrProc = PROC_TYPE_EXEC;`
- Compiled: YES, MATCH.
- Confidence: high. This also **proves `mCurrProc` sits at offset `0x1f4`**,
  the exact same absolute offset as in `daWmCloud_c` (see the "class-shape"
  section below for why that is a bigger finding than it looks).
- Offset-perturbing: NO.

### `0x179EB0` -- `mode_exec()`
- Evidence: single `blr`, identical to the twin's `mode_exec__11daWmCloud_cFv`.
- Proposal: empty body, `void daWmSmallCloud_c::mode_exec() {}`
- Compiled: YES, MATCH.
- Confidence: high.
- Offset-perturbing: NO.

### `0x179EC0` -- `processCutsceneCommand(int, bool)`
- Evidence: 17/17 instructions identical to the twin's
  `processCutsceneCommand__11daWmCloud_cFib`, including the `cmpwi r4,-1`
  (CUTSCENE_CMD_NONE), the `isStaff__14dWmDemoActor_cFv` call, and the
  `stb r0,0x139(r31)` write.
- Proposal: literally the twin's `processCutsceneCommand()` body, copied
  verbatim (the `dWmDemoActor_c` header already declares this override and
  already declares `mIsCutEnd` -- no new field needed at all).
- Compiled: YES, MATCH.
- Confidence: high. Confirms `mIsCutEnd` (a `dWmDemoActor_c` field, offset
  `0x139`) is used unmodified by `daWmSmallCloud_c`.
- Offset-perturbing: NO.

### `0x179F10` -- `setPosFromCourseNode()` (working name)
- Evidence:
  ```
  lwz r0, 0x4(r3)              ; mParam
  clrlslwi r0, r0, 24, 2       ; (mParam & 0xff) << 2  == ACTOR_PARAM(CourseNo) * 4
  lis/addi  -> table base
  lwzx r4, r4, r0              ; table[CourseNo]
  lis/lwz   -> daWmMap_c::m_instance
  addi r5, r5(=this), 0xac     ; &mPos
  b GetNodePos__9daWmMap_cFPCcR7mVec3_c   ; tail call
  ```
  The callee's mangled name is `GetNodePos(const char*, mVec3_c&)`. The
  **only** `GetNodePos` declared anywhere in the tree is
  `include/game/bases/d_a_wm_map.hpp`'s `void GetNodePos(long, mVec3_c&)` --
  a different overload (index, not name). `0x4` is `fBase_c::mParam`
  (`include/game/framework/f_base.hpp:62`, first field after `mUniqueID`,
  confirmed at offset `0x4` from the struct layout); `dWmObjActor_c` already
  declares `ACTOR_PARAM_CONFIG(CourseNo, 0, 8)`, so `ACTOR_PARAM(CourseNo)` is
  exactly `(mParam & 0xff)`, matching the `clrlslwi ...,24,2` mask-and-times-4.
- Proposal:
  ```cpp
  void daWmSmallCloud_c::setPosFromCourseNode() {
      static const char *nodeNames[] = { "F0C0" /* placeholder, see below */ };
      daWmMap_c::m_instance->GetNodePos(nodeNames[ACTOR_PARAM(CourseNo)], mPos);
  }
  ```
  plus a shadow-header overload
  `void daWmMap_c::GetNodePos(const char *nodeName, mVec3_c &pos);` added to
  `d_a_wm_map.hpp`.
- Compiled: YES, MATCH (instruction shape is exact; the pool relocation
  target is normalised away by the verifier, so the placeholder string
  `"F0C0"` and the 1-entry table size are **not** proven -- only that *some*
  `const char*[]` is indexed by `ACTOR_PARAM(CourseNo)`).
- Confidence: high on shape/signature, low on the actual table contents and
  size (I invented both; real values need the `.rodata`/`.data` for this TU,
  which is largely batch A's/whoever owns the `.data` layout).
- Offset-perturbing: the **class member offsets are NOT perturbed** (nothing
  new added to the class). The **shared header change is real** and must be
  landed alongside this TU: adding an overload to `daWmMap_c` in
  `include/game/bases/d_a_wm_map.hpp`. Per the rules I did not touch that
  header -- I shadow-copied it into
  `wip/wm_smallcloud/scratch/batchB/shadow_include/game/bases/d_a_wm_map.hpp`
  and compiled against the copy. **Proposing this addition to the lead.**

### `0x179E00` -- `initStateLike()` (working name, likely the real `initState()`)
- Evidence: same `setRate`/`setFrame` prologue and `init_exec()` tail-call as
  the twin's `initState()`, but with two real differences from the twin:
  1. No `mPos = mVec3_c::Zero;` reset at the top (the twin has one).
  2. An `if (ACTOR_PARAM(CourseNo) == 3) { ... }` block wrapping
     `mModel.setPriorityDraw(0,0)` and a course-clear-gated visibility store
     that the twin does not have at all.
  Full body (target, 40 instructions) reproduced exactly; see the compiled
  draft.
- Proposal:
  ```cpp
  void daWmSmallCloud_c::initStateLike() {
      mChrAnim[CS_Anim].setRate(1.0f);
      mChrAnim[CS_Anim].setFrame(0.0f);
      setPosFromCourseNode();
      init_exec();
      int courseNo = ACTOR_PARAM(CourseNo);
      if (courseNo == 3) {
          mModel.setPriorityDraw(0, 0);
          if (dWmLib::IsCourseClear(5, 0x17)) {
              mVisible = true;
          } else {
              mVisible = false;
          }
      }
  }
  ```
- Compiled: YES, MATCH (40/40 instructions).
- Confidence: high on shape and on `mVisible` (see below); medium on the
  literal `1.0f`/`0.0f` rate/frame values and the `5`/`0x17` (23) world/course
  pair, all of which are register-relocation-transparent to the verifier (any
  float/int literal compiles to the same instruction shape) so their exact
  values are asserted by convention with the twin, not proven.
- **Class-member proof: the byte stored at `0x124(r31)` is `dBaseActor_c::mVisible`**,
  not a new field. I wrote `mVisible = true/false;` using the *existing,
  already-landed* base class member (no shadow-header change needed for this
  one), and it placed at exactly the offset the target uses. This is a
  genuine, testable proof, not a guess: if `mVisible` were at a different
  offset, this function would not have matched.
- Offset-perturbing: NO. No new field added to any class.
- **Two changes needed together to close this one** (see "Two changes that
  closed 179E00" below) -- worth a callout since AGENT_CONTEXT flags coupled
  fixes as the highest-value class of finding.

### `0x179F40` -- `__sinit_d_a_wm_smallcloud_cpp` (compiler-synthesised, not a real class member)
- Evidence: this function is registered directly in `.ctors`
  (`bin/dtkspl/d_basesNP/obj/auto_fn_2_179F40_text.o` has a
  `.section .ctors` entry pointing straight at it, its own weak-symbol
  object, no relation to `daWmSmallCloud_c` data layout at all). It is
  **instruction-for-instruction the same shape** as the twin's
  `"__sinit_\d_a_wm_cloud_cpp"` (33 instructions both), which
  AGENT_CONTEXT's "header static with a non-trivial constructor is emitted
  into EVERY TU that odr-uses it" note already predicts: `dWmLib::sc_ForceList`
  and `dWmLib::c_StartPointKinokoHouseID` are `static` objects defined
  directly inside `include/game/bases/d_wm_lib.hpp` with runtime-dependent
  initialisers (`dCsvData_c::c_CASTLE_ID`/`c_START_ID` are not compile-time
  constants), so every TU that includes `d_wm_lib.hpp` gets its own private
  copy plus its own `__sinit`/`__arraydtor` pair -- confirmed identical
  symbol names (`c_CASTLE_ID__10dCsvData_c`, `c_START_ID__10dCsvData_c`,
  `__register_global_object`, `__dt__Q26dWmLib19ForceInCourseList_tFv`) in
  both TUs' sinit functions.
- Proposal: **no source needed**. Simply including
  `<game/bases/d_wm_lib.hpp>` is sufficient -- the compiler auto-emits this
  function and its paired `__arraydtor` (see `0x179FD0` below) with zero
  explicit code, exactly as it does in the twin.
- Compiled: 28/33 instructions exact; the remaining 5 diff lines are, in
  order: two verifier-normalisation artifacts (my object names its local pool
  symbol `...rodata.0`, an underscore-less anonymous form that
  `verify_anon.py`'s `norm()` regex doesn't recognise as a relocation token,
  so it shows as a raw diff even though both sides are "reference to an
  anonymous local rodata symbol") and **three real, small offset
  differences**: `lfs f2,0x24(r5)` / `f1,0x28(r5)` / `f0,0x2c(r5)` in the
  target vs `0x8`/`0xc`/`0x10` in mine. These are the position of
  `sc_ForceList`'s `mNodePos` floats (`2160.0f, -30.0f, -478.0f`, verbatim
  from the shared header) inside this TU's local `.rodata` constant pool.
  That position is a function of **everything else this TU puts in
  `.rodata`** before this point (batch A's `sGlobalData_c` template
  instantiation, `sGroupNodeNames`, any `DUMMY_UNUSED`-style filler) which I
  do not have. The gap is `0x24 - 0x8 = 0x1c` (28 bytes) of *other* rodata
  content that batch A's part of the file is expected to contribute.
- Confidence: high that the function needs zero explicit source and will
  close automatically once merged with batch A's part of the TU; **cannot be
  fully closed in isolation** -- flagging this as a coupling point rather
  than claiming a false MATCH.
- Offset-perturbing: NO (adds nothing to `daWmSmallCloud_c` itself; this is
  TU-wide compiler machinery, not a class member).

### `0x179FD0` -- array destructor for `dWmLib::sc_ForceList` (compiler-synthesised)
- Evidence: same reasoning as `0x179F40`. Confirmed **MATCH already, with zero
  code written for it** -- it appeared automatically once
  `<game/bases/d_wm_lib.hpp>` was included, exactly mirroring the twin's
  `__arraydtor$12712`.
- Proposal: none needed.
- Compiled: YES, MATCH (7/7 instructions).
- Confidence: high (byte-exact).
- Offset-perturbing: NO.

## Class-shape findings for the lead to reconcile with batch A

These are all **measured**, not guessed -- each one is the reason a specific
function above went from "differing" to "MATCH".

1. **`daWmSmallCloud_c`'s field layout is byte-identical to `daWmCloud_c`'s up
   through and including `mCurrProc` at offset `0x1f4`.** Proven by
   `calcModel()` (`0x7c`/`0xac`/`0x100`/`0xdc`/`0x1ac`), `init_exec()`
   (`0x1f4`), and `processCutsceneCommand()` (`0x139`, a base-class field) all
   matching byte-for-byte using exactly the twin's field offsets. This means:
   `mUnk188`(`0x188`), `mAllocator`, `mResFile`, `mModel`(`0x1ac`),
   `mChrAnim[ANIM_COUNT]`(`0x1b8`, so `ANIM_COUNT` is very likely `1`, same as
   the twin), then a 4-byte pad, then `mCurrProc`(`0x1f4`) -- same types, same
   order, same sizes as `daWmCloud_c`.
2. **Contradiction to report, not resolve**: `daWmCloud_c`'s own header names
   the pad field right before `mCurrProc` `mUnk250`, which by this codebase's
   own convention (`mUnk188` demonstrably sits at `0x188`) should mean offset
   `0x250`. But `mCurrProc` is proven at `0x1f4`, and C++ member order within
   one access-control block is fixed, so a field declared textually *before*
   `mCurrProc` cannot be at a *higher* address than it. Either `mUnk250`'s
   name predates a correction to `ANIM_COUNT`/some other earlier field's size
   and was never renamed, or I am misreading the header. I have not touched
   `d_a_wm_cloud.hpp` (already landed, not mine to edit) -- flagging for the
   lead rather than silently trusting the name.
3. **`daWmMap_c::GetNodePos` needs a second overload**, `(const char*, mVec3_c&)`,
   alongside the existing `(long, mVec3_c&)` in `include/game/bases/d_a_wm_map.hpp`.
   Evidenced by the mangled callee name in `0x179F10`
   (`GetNodePos__9daWmMap_cFPCcR7mVec3_c`). This is a **shared header change
   proposal**, not applied -- see the shadow copy at
   `wip/wm_smallcloud/scratch/batchB/shadow_include/game/bases/d_a_wm_map.hpp`.
4. **`dBaseActor_c::mVisible` (offset `0x124`) is used directly by
   `daWmSmallCloud_c`**, gated on `ACTOR_PARAM(CourseNo) == 3` and
   `dWmLib::IsCourseClear(5, 0x17)`. No new field required.
5. **`sGroupNodeNames`/`GlobalData_t`/`DUMMY_UNUSED`-style `.rodata` content
   that batch A owns directly determines whether `0x179F40` closes.** See its
   section above -- the gap is exactly 28 bytes of *something* batch A's part
   of the file needs to contribute before this point in file order.

## Two changes that closed `0x179E00`

Applied together, not isolated individually (no plateau was hit, so I did not
spend budget separating them further -- flagging both since AGENT_CONTEXT
calls out coupled fixes as high value):

1. **`int courseNo = ACTOR_PARAM(CourseNo); if (courseNo == 3)`** instead of
   `if (ACTOR_PARAM(CourseNo) == 3)` directly. The direct form produced
   `cmplwi` (unsigned compare); assigning the masked `mParam` bitfield to a
   named `int` first produced the target's `cmpwi` (signed compare). Net
   effect: the comparison must run over a variable typed `int`, not the raw
   unsigned macro expansion.
2. **Explicit `if (cond) { mVisible = true; } else { mVisible = false; }`**
   instead of `mVisible = dWmLib::IsCourseClear(5, 0x17);` directly. The
   direct assignment folded to a bare `stb r3, 0x124(r31)` (5 fewer
   instructions); the target instead does `cmpwi r3,0; beq; li r0,1/0;
   stb r0,0x124(r31)` -- proving the source really did write an explicit
   if/else, not a direct bool-to-bool assignment.

## Compiler-flag finding (affects anyone else using `harness.py` on this unit)

`tools/auto_decomp/harness.py`'s `CFLAGS` (`-proc gekko -fp hard -O4 -inline
noauto -Cpp_exceptions off -enum int -RTTI off -ipa file -enc SJIS
-DREVOLUTION -I-`) do **not** match `slices/d_basesNP.json`'s
`meta.defaultCompilerFlags` (`-sdata 0 -sdata2 0 -proc gekko -fp hard -O4,p
-inline noauto -char signed -rtti off -enum int -Cpp_exceptions off -ipa file
-enc SJIS`). Compiling `0x179E00` under the harness's flags produced `@sda21`
small-data-relative loads for the `1.0f`/`0.0f` literals where the target (and
a re-compile under the correct `-sdata 0 -sdata2 0 -O4,p -char signed` flags)
uses plain `lis`/`lfs` -- a spurious diff with nothing to do with the source.
`wip/wm_smallcloud/scratch/batchB/run.py` compiles with the correct flags
directly (following the precedent already set in
`scratch/gemini_round13/test_tower.py`, which hit and solved the same issue
for `d_a_wm_tower.cpp`).

## Variants tried and discarded

- `mVisible = dWmLib::IsCourseClear(5, 0x17);` (direct assignment) --
  compiled, 5 instructions short of target; discarded for the explicit
  if/else form above.
- `if (ACTOR_PARAM(CourseNo) == 3)` (no named local) -- compiled, one
  `cmplwi`/`cmpwi` mismatch; discarded for the named `int courseNo` form
  above.
- Compiling under `tools/auto_decomp/harness.py`'s default `CFLAGS` -- built,
  but every function touching a float literal used `@sda21` addressing not
  present in the target; discarded in favour of the `d_basesNP`-specific flag
  set in `run.py`.

## Full source

`wip/wm_smallcloud/scratch/batchB/d_a_wm_smallcloud.cpp`:

```cpp
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_a_wm_smallcloud.hpp>
#include <game/bases/d_a_wm_map.hpp>

void daWmSmallCloud_c::init_exec() {
    mCurrProc = PROC_TYPE_EXEC;
}

void daWmSmallCloud_c::mode_exec() {}

void daWmSmallCloud_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId != dCsSeqMng_c::CUTSCENE_CMD_NONE && !isStaff()) {
        mIsCutEnd = true;
    }
}

void daWmSmallCloud_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daWmSmallCloud_c::setPosFromCourseNode() {
    // batch A's createModel() (scratch/batchA/draft.cpp) independently found a 4-entry
    // table indexed the same way (resMdlNames[ACTOR_PARAM(CourseNo)]); mirroring that
    // table size here since it is likely the real number of small-cloud variants, but
    // the actual node-name strings are unconfirmed -- placeholders only.
    static const char *nodeNames[4] = {
        "F0C0", "F0C1", "F0C2", "F0C3"
    };
    daWmMap_c::m_instance->GetNodePos(nodeNames[ACTOR_PARAM(CourseNo)], mPos);
}

void daWmSmallCloud_c::initStateLike() {
    mChrAnim[CS_Anim].setRate(1.0f);
    mChrAnim[CS_Anim].setFrame(0.0f);
    setPosFromCourseNode();
    init_exec();
    int courseNo = ACTOR_PARAM(CourseNo);
    if (courseNo == 3) {
        mModel.setPriorityDraw(0, 0);
        if (dWmLib::IsCourseClear(5, 0x17)) {
            mVisible = true;
        } else {
            mVisible = false;
        }
    }
}
```

Shadow class header,
`wip/wm_smallcloud/scratch/batchB/shadow_include/game/bases/d_a_wm_smallcloud.hpp`
(scratch only -- batch A owns the real one; see class-shape findings above for
what is proven vs guessed):

```cpp
#pragma once

/// @unofficial SCRATCH SHADOW HEADER -- batchB working hypothesis only. NOT for landing.
/// Field layout up to mCurrProc (0x1f4) is proven byte-identical to the landed daWmCloud_c
/// (see wip/wm_smallcloud/BATCHB.md). Everything after mCurrProc is unconfirmed guesswork
/// carried over from the twin purely so the class compiles; batch A owns the real shape.

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_wm_bgm_sync.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/mLib/m_sphere.hpp>

class daWmSmallCloud_c : public dWmObjActor_c {
public:
    static const int NODE_COUNT = 20; ///< @unofficial guess, copied from twin; unused by batchB's functions

    enum ANIM_e {
        CS_Anim,
        ANIM_COUNT
    };

    typedef void (daWmSmallCloud_c::*ProcFunc)();

    daWmSmallCloud_c();
    ~daWmSmallCloud_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    // createModel(), initGroupNodeIds(), calcCulling() etc. are batch A's territory --
    // not declared here since batchB's functions never call them.

    void calcModel();

    void init_exec();
    void mode_exec();

    /// @unofficial batchB fn_2_179F10 -- looks up this actor's course-node position by name,
    /// indexed off ACTOR_PARAM(CourseNo), and writes it into mPos.
    void setPosFromCourseNode();

    /// @unofficial batchB fn_2_179E00 working name -- plays the same role as daWmCloud_c's
    /// initState() (sets up mChrAnim, calls init_exec()) but does NOT reset mPos to Zero and
    /// has extra CourseNo==3 / IsCourseClear handling daWmCloud_c's initState() does not have.
    /// Real name is very likely "initState" but is left renamed here to avoid colliding with
    /// batch A's own (currently absent) declaration. See BATCHB.md.
    void initStateLike();

    u32 mUnk188; ///< @unused, copied from twin
    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;
    m3d::smdl_c mModel; ///< proven @ 0x1ac (see BATCHB.md)
    m3d::anmChr_c mChrAnim[ANIM_COUNT]; ///< proven @ 0x1b8
    u32 mUnk1F0; ///< @unofficial proven-position pad @ 0x1f0; twin names the equivalent field
                 ///< mUnk250, which looks stale against the offset this batch measured
    PROC_TYPE_e mCurrProc; ///< proven @ 0x1f4

    int mGroupNodeIds[NODE_COUNT]; ///< @unofficial guess, not exercised by batchB
    mSphere_c mCurrNodeClipSphere; ///< @unofficial guess, not exercised by batchB
    dWmBgmSync_c *mpBgmSync; ///< @unofficial guess, not exercised by batchB
};
```

Shadow `d_a_wm_map.hpp` with the proposed overload (the only shared-header
change this batch needs), diffed against the real
`include/game/bases/d_a_wm_map.hpp`:

```diff
     int GetNodeCount(int); ///< @unofficial
     void GetNodePos(long nodeIdx, mVec3_c &pos);
+    /// @unofficial @proposed-by-batchB looks up a node by name instead of index; evidenced by
+    /// fn_2_179F10 in d_a_wm_smallcloud.cpp calling GetNodePos__9daWmMap_cFPCcR7mVec3_c.
+    void GetNodePos(const char *nodeName, mVec3_c &pos);
```

## Cross-batch corroboration (found while finishing up)

`wip/wm_smallcloud/scratch/batchA/` already has an in-progress
`include/game/bases/d_a_wm_smallcloud.hpp` and `draft.cpp` from the other
batch's agent (no `BATCHA.md` yet, so presumably still in progress). Worth
recording since it independently corroborates several of this report's
findings without either batch having seen the other's work while writing it:

- Batch A's field list also has the pad immediately before `mCurrProc` at
  `0x1f0` (`u32 mUnk1f0`), matching this report's `0x1f4 - 4 = 0x1f0` and the
  "`mUnk250` looks stale" finding above -- independent convergence on the
  same number.
- Batch A's `createModel()` indexes a **4-entry** table with
  `resMdlNames[ACTOR_PARAM(CourseNo)]` (`"CS_W7_MoveCloud01"`,
  `"CS_W7_MoveCloud02"`, `"CS_W7_MoveCloud03"`, `"CS_W6aCloud"`) -- the same
  `ACTOR_PARAM(CourseNo)`-as-table-index idiom this report found independently
  for `0x179F10`/`0x179E00`. I updated `setPosFromCourseNode()`'s placeholder
  table from 1 entry to 4 to match this likely-real table size (contents
  still unconfirmed placeholders; still MATCHes, since the verifier is
  relocation-transparent for this).
- Batch A's `draft.cpp` already stubs `initState`/`init_exec`/`mode_exec`/
  `updatePos` (their name for `0x179F10`) with a comment marking them as this
  batch's territory, and correctly does not attempt `0x179E00`'s real body --
  consistent with the split in this brief.
- One naming disagreement worth flagging to the lead: batch A calls the
  `0x179F10` function `updatePos()`; this report calls it
  `setPosFromCourseNode()`. Same function, same evidence, different working
  name -- pick one when merging.

## What would raise confidence further

- The real `nodeNames[]` table contents/size for `0x179F10` -- needs the
  `.rodata`/`.data` for this TU, which I do not have access to reconstruct in
  isolation (batch A's / the lead's territory).
- Closing `0x179F40` needs compiling this batch's source together with batch
  A's, so the `.rodata` pool offset (currently `0x8`, needs `0x24`) can
  actually be checked against the real value instead of predicted.
- The `mUnk250` vs `0x1f0` naming contradiction (see class-shape finding #2)
  should be checked against whatever originally justified that name in
  `d_a_wm_cloud.hpp`'s history, if that's recoverable.
