# CREATEMODEL -- closing `fn_2_179BB0` (`daWmSmallCloud_c::createModel()`)

Scope: exactly one function, `0x179BB0`, size `0x194` (101 instructions). Starting
point was `wip/wm_smallcloud/BATCHA.md` / `MERGED.md`, which left it at
95/101 differing and named the blocker precisely: a `.data` aggregate whose
leading `0x88` bytes could not be attributed from batch A's 8-function scope.

Work directory: `wip/wm_smallcloud/scratch/createmodel/` (copied from
`wip/wm_smallcloud/scratch/merged/`, same `include/`, `shadow_include/`,
`build.py`). Compiled throughout with
`harness.compile_draft(..., module='d_basesNP')` (REL flags: `-sdata 0
-sdata2 0 -O4,p -char signed -rtti off`, per the brief).

## Verifier line -- before and after

Command (per the brief, run from repo root):

```
python wip/wm_units/verify_anon.py wip/wm_smallcloud/scratch/createmodel/draft.txt 0x1797e0 0x179ff0 bin/dtkspl/d_basesNP/obj/auto_00_001797B4_text.o bin/dtkspl/d_basesNP/obj/auto_fn_2_179F40_text.o bin/dtkspl/d_basesNP/obj/auto_00_00179FC4_text.o
```

Before (inherited from `MERGED.md`, unchanged at the start of this round):

```
0x00179bb0 fn_2_179BB0              101  95 differing vs createModel__16daWmSmallCloud_cFv
...
14/16 byte-identical modulo symbol names
```

After:

```
addr       target                  size  result
0x001797e0 fn_2_1797E0               12  MATCH  <- daWmSmallCloud_c_classInit__Fv
0x00179810 fn_2_179810               31  MATCH  <- __ct__16daWmSmallCloud_cFv
0x00179890 fn_2_179890               57  MATCH  <- __dt__16daWmSmallCloud_cFv
0x00179980 fn_2_179980               62  MATCH  <- create__16daWmSmallCloud_cFv
0x00179a80 fn_2_179A80               59  MATCH  <- execute__16daWmSmallCloud_cFv
0x00179b70 fn_2_179B70               12  MATCH  <- draw__16daWmSmallCloud_cFv
0x00179ba0 fn_2_179BA0                2  MATCH  <- doDelete__16daWmSmallCloud_cFv
0x00179bb0 fn_2_179BB0              101  MATCH  <- createModel__16daWmSmallCloud_cFv
0x00179d50 fn_2_179D50               44  MATCH  <- calcModel__16daWmSmallCloud_cFv
0x00179e00 fn_2_179E00               40  MATCH  <- initState__16daWmSmallCloud_cFv
0x00179ea0 fn_2_179EA0                3  MATCH  <- init_exec__16daWmSmallCloud_cFv
0x00179eb0 fn_2_179EB0                1  MATCH  <- mode_exec__16daWmSmallCloud_cFv
0x00179ec0 fn_2_179EC0               17  MATCH  <- processCutsceneCommand__16daWmSmallCloud_cFib
0x00179f10 fn_2_179F10               10  MATCH  <- setPosFromCourseNode__16daWmSmallCloud_cFv
0x00179f40 fn_2_179F40               33  3 differing vs "__sinit_\d_a_wm_smallcloud_cpp"
0x00179fd0 fn_2_179FD0                7  MATCH  <- __arraydtor$12784

15/16 byte-identical modulo symbol names
```

**`fn_2_179BB0`/`createModel()` is now MATCH, byte-exact, 101/101.** The unit
goes from 14/16 to 15/16. The one remaining open item, `__sinit` (3/33
differing, down from 5/33 in `MERGED.md`), is **not** this function -- see
"Side effect on `__sinit`" below; it is compiler-synthesised machinery for
`dWmLib::sc_ForceList`, out of this brief's scope.

## The `.data` aggregate -- what it actually is

The brief's premise (an aggregate whose leading `0x88` bytes were unexplained)
was correct as stated, but the aggregate is **not something `createModel()`
declares**. It is the pre-existing `dWmLib::sc_ForceList` static from
`include/game/bases/d_wm_lib.hpp:84`, already odr-used by this same TU's
`initState()` (which calls `dWmLib::IsCourseClear`, forcing `d_wm_lib.hpp`'s
inclusion) -- and `BATCHB.md`/`MERGED.md` had already identified and used this
fact to close `__sinit`'s array-registration and `0x179FD0`'s array-destructor
functions. What neither prior round had checked is that `createModel()`'s own
`.data` also sits immediately after it, sharing the SAME base register.

Struct (from the header, unchanged, not proposed):

```cpp
struct ForceInCourseList_t {
    int mNodeWorld;
    const char *mNodeName;
    int mWorld;
    int mLevel;
    int mEntrance;
    const char *mLevelNode;
    mVec3_c mNodePos;
};
static ForceInCourseList_t sc_ForceList[] = {
    {WORLD_7, "F7C0", WORLD_7, dCsvData_c::c_CASTLE_ID, 4, "W7C0", mVec3_c(2160.0f, -30.0f, -478.0f)}
};
```

Target bytes proving it (`bin/dtkspl/d_basesNP/obj/auto_04_00046BE0_data.txt`,
addresses `0x47258`-`0x472E0`, decoded byte-for-byte, base register `r30` in
`createModel()`'s own disassembly is `lis r30, lbl_2_data_47258@ha`):

```
0x47258  lbl_2_data_47258  8B   "F7C0\0\0\0\0"        <- sc_ForceList[0].mNodeName
0x47260  lbl_2_data_47260  5B   "W7C0\0" (+3 pad)      <- sc_ForceList[0].mLevelNode
0x47268  lbl_2_data_47268  0x24 sc_ForceList[0] itself:
             .4byte 0x00000006            mNodeWorld = WORLD_7 (=6)
             .4byte lbl_2_data_47258      mNodeName -> "F7C0"
             .4byte 0x00000006            mWorld = WORLD_7
             .4byte 0x00000000            mLevel (patched at runtime by __sinit:
                                           lwz c_CASTLE_ID__10dCsvData_c; stw 0xc(r6) --
                                           c_CASTLE_ID is not a compile-time constant)
             .4byte 0x00000004            mEntrance = 4
             .4byte lbl_2_data_47260      mLevelNode -> "W7C0"
             .4byte 0x00000000 x3         mNodePos (patched at runtime by __sinit,
                                           stfs 0x18/0x1c/0x20(r6) = 2160.0f,-30.0f,-478.0f)
0x4728C  g_profile_WM_SMALLCLOUD  0xC    ACTOR_PROFILE(WM_SMALLCLOUD,...)'s own object
0x47298  lbl_2_data_47298  0x12+2pad     "CS_W7_MoveCloud01\0"  <- resMdlNames[0]
0x472AC  lbl_2_data_472AC  0x12+2pad     "CS_W7_MoveCloud02\0"  <- resMdlNames[1]
0x472C0  lbl_2_data_472C0  0x12+2pad     "CS_W7_MoveCloud03\0"  <- resMdlNames[2]
0x472D4  lbl_2_data_472D4  0x34          "CS_W6aCloud\0" (12B, resMdlNames[3]) +
                                          4 pointers (resMdlNames[0..3], the table
                                          itself, at +0xC = 0x472E0) +
                                          "CS_W%d\0\0" (8B, sprintf format) +
                                          "g3d/model.brres\0" (16B, archive path)
```

`0x472E0 - 0x47258 = 0x88`, `0x472F0 - 0x47258 = 0x98`, `0x472F8 - 0x47258 =
0xA0` -- exactly the `addi rX, rBASE, {0x88, 0x98, 0xa0}` immediates the
target uses. MWCC computed **one** `lis/addi` for the first object it touches
in this TU's `.data` (`sc_ForceList`'s own `mNodeName` buffer) and reached
everything else -- including `g_profile_WM_SMALLCLOUD` and all of
`createModel()`'s own string/table data -- via constant offsets from it,
because all of it is emitted by the same TU's compile and the relative
layout is therefore known before linking. This is not a new declaration to
make; `sc_ForceList` already exists (via `#include <game/bases/d_wm_lib.hpp>`,
already present in the merged draft) and `resMdlNames`/the sprintf format
string/the path string are ordinary local statics/literals inside
`createModel()` itself, in the order they appear in source. **No new `.data`
declaration was needed at all** -- the fix was entirely inside
`createModel()`'s own control flow and local-variable types, described below.

In the **standalone** compile used for verification, this object doesn't
exist (it's `dWmLib::sc_ForceList`, an anonymous local `.data` object named
`...data.0` by the isolated compile instead of `lbl_2_data_47258`) --
producing 2 of the 3 remaining lines in `__sinit`'s diff and would have shown
as 2 of `createModel()`'s diff lines too, except `verify_anon.py`'s `norm()`
requires the symbol to start with a word/`@`/`$` character and `...data.0`
starts with a literal dot, so it isn't recognised as the "same anonymous
relocation, different name" case its own docstring describes. This is a
pre-existing, already-documented tooling limitation (`MERGED.md` flagged the
identical thing for `__sinit`), not a new problem, and it does not appear in
`createModel()`'s final result because the relocation target address (`r30`
base + `0x88`/`0x98`/`0xa0`) matched exactly regardless of the symbol's own
name -- the 2-line artifact only shows up when the base symbol name ITSELF is
the compared token, which happens in `__sinit` (`lis r30,
lbl_2_data_47258@ha` is a top-level line there) but not in `createModel()`
(where the base symbol's name never differs at the *instruction* level
being compared, only its two defining `lis`/`addi` lines do -- and those
ended up matching too once the real declaration order was reproduced).

## What actually closed the function (source changes, not data changes)

Three changes, applied in this order, each measured independently:

1. **A real `for` loop, matching the twin's `daWmCloud_c::createModel()`
   shape**, instead of the previous draft's hand-unrolled
   `mChrAnim[CS_W7_SmallCloud].foo()` statements repeated four times.
   `ANIM_COUNT == 1` so this has no runtime effect, but it changes what MWCC
   hoists: with a real loop, `&mChrAnim[i]` gets computed once into a
   register and reused across `create()`/`setRate()`/`setFrame()`, exactly
   as the byte-exact twin already does (confirmed by reading
   `daWmCloud_c::createModel()`'s own compiled object,
   `wip/wm_smallcloud/scratch/batchA/cloud_compiled.txt`, before writing
   this -- per `AGENT_CONTEXT.md`'s "read a function that already MATCHES"
   rule). Result: 95 -> 71 differing.
2. **The `resAnmNames`-fill-once guard moved inside the loop body**
   (`for (i...) { if (!sInit) {...} ...loop body using i... }` instead of
   `if (!sInit) {...}` before a separate loop). This changes where MWCC's
   loop-invariant-code-motion places the loop's other invariants
   (`playModes`' rodata address, the float-literal-pool address, `&mChrAnim[i]`)
   -- they get hoisted to the loop preheader, **before** the `sInit`
   branch, matching the target's instruction order exactly. Also grew the
   stack frame from `-0x30`/`_savegpr_27` (5 saved registers) to
   `-0x40`/`_savegpr_25` (7 saved registers), matching the target's frame
   size and register-save helper exactly. Result: 71 -> 36 differing.
3. **`static char sInit = 0;` instead of `static bool sInit = false;`.**
   Under this TU's `-char signed` flag, testing a plain `char` with `!sInit`
   compiles to `lbz r0,...; extsb. r0,r0; bne ...` (sign-extend-and-test);
   testing a `bool` compiles to `lbz r0,...; cmpwi r0,0x0; bne ...`. The
   target uses the former. This is a real, testable type difference, not a
   register artifact. Result: 36 -> 5 differing.

Two more small, independently-verified fixes:

4. **`char arcName[6]` instead of `char arcName[8]`.** The sprintf target is
   `"CS_W%d"` with a single-digit world number (`dScWMap_c::m_WorldNo + 1`
   for World 7 is `7`), so `"CS_W7\0"` needs exactly 6 bytes. The extra 2
   bytes of slack in an 8-byte buffer shifted every later stack-frame local
   by 4 bytes (`0x14` in the target vs `0x18` in the 8-byte draft, for the
   `getRes()` argument computed from `arcName`'s address) -- an
   offset-perturbing local-variable-size fact, not a register issue. Result:
   5 -> 3 differing.
5. **`mModel.setAnm(mChrAnim[CS_W7_SmallCloud])` (the compile-time constant
   index) instead of `mModel.setAnm(mChrAnim[i])` (the loop variable),
   for this one call only** -- every other statement in the loop body still
   uses `mChrAnim[i]`. This is asymmetric and looks odd, but it is exactly
   what closes the register choice: the target recomputes
   `&mChrAnim[CS_W7_SmallCloud]` fresh (`addi r4, r27, 0x1b8`) for this one
   argument instead of reusing the register already holding `&mChrAnim[i]`
   from the three preceding calls (`mr r4, r29` is what every other
   plausible form produces, including the twin's own identically-shaped
   loop, which DOES reuse the cached register for this exact call --
   confirmed by reading `cloud_compiled.txt` line-for-line). Using the named
   enumerator here, and only here, is what reproduces that. Result: 3 -> 0
   differing, **MATCH**.

## Variants tried and discarded (in addition to the numbered progression above)

- **`resAnmNames` as a single aggregate initializer** referencing
  `resMdlNames[0..3]` directly (`static const char *resAnmNames[4] = {
  resMdlNames[0], ... };`), instead of the guarded `if (!sInit)` block --
  hypothesis was that MWCC's own compiler-synthesised static-init guard
  would reproduce the `extsb.` idiom automatically. Compiled, but made
  things WORSE (71 differing, worse than the 36 achieved by nesting the
  existing guard inside the loop) -- discarded. The `extsb.` idiom turned
  out to come from `char` vs `bool`, not from who writes the guard.
- **`int i` declared before the loop** (`int i = 0; for (; i < ANIM_COUNT;
  i++)`) instead of in the loop's init-statement -- compiled, no change (still
  5 differing at that point in the sequence) -- discarded as a no-op, kept
  the more natural `for (int i = 0; ...)` form.
- **`char arcName[4]`** -- not tried numerically since `[6]` already closed
  the stack-offset diff exactly; `[6]` is the minimum that fits `"CS_W7\0"`
  and is therefore the best-supported size.

## Source

Final `wip/wm_smallcloud/scratch/createmodel/d_a_wm_smallcloud.cpp` (only
`createModel()` shown; every other function is unchanged from
`wip/wm_smallcloud/MERGED.md` and still MATCHes):

```cpp
void daWmSmallCloud_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    char arcName[6];
    sprintf(arcName, "CS_W%d", dScWMap_c::m_WorldNo + 1);
    mResFile = dResMng_c::m_instance->getRes(arcName, "g3d/model.brres");

    static const char *resMdlNames[4] = {
        "CS_W7_MoveCloud01",
        "CS_W7_MoveCloud02",
        "CS_W7_MoveCloud03",
        "CS_W6aCloud"
    };

    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl(resMdlNames[ACTOR_PARAM(CourseNo)]);
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    static char sInit = 0;
    static const char *resAnmNames[4];
    static const m3d::playMode_e playModes[ANIM_COUNT] = {
        m3d::FORWARD_LOOP
    };

    for (int i = 0; i < ANIM_COUNT; i++) {
        if (!sInit) {
            resAnmNames[0] = resMdlNames[0];
            resAnmNames[1] = resMdlNames[1];
            resAnmNames[2] = resMdlNames[2];
            resAnmNames[3] = resMdlNames[3];
            sInit = true;
        }
        nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr(resAnmNames[ACTOR_PARAM(CourseNo)]);
        mChrAnim[i].create(resMdl, resAnmChr, &mAllocator, nullptr);
        mChrAnim[i].mPlayMode = playModes[i];
        mChrAnim[i].setRate(0.0f);
        mChrAnim[i].setFrame(0.0f);
        mModel.setAnm(mChrAnim[CS_W7_SmallCloud]);
    }

    dWmActor_c::setSoftLight_Map(mModel);
    mAllocator.adjustFrmHeap();
}
```

No header change was needed beyond what `MERGED.md` already established
(`#include <game/bases/d_wm_lib.hpp>` in the `.cpp`, already present). The
class layout (`daWmSmallCloud_c`, `sizeof == 0x1fc`) is unchanged from
`MERGED.md` -- this round touched no offsets, added no members, and is
**not offset-perturbing**.

## Side effect on `__sinit` (out of scope, reported not chased)

`__sinit`'s diff dropped from 5/33 (`MERGED.md`) to 3/33 as a side effect of
closing `createModel()` first: `MERGED.md` had predicted "most likely
additional pooled constants from `createModel()`'s own still-unclosed body
... sits earlier in the file and would contribute to the same per-TU local
`.rodata` pool before `__sinit`'s floats do" -- confirmed. The remaining 3
lines in `__sinit` are the same `...data.0` vs `lbl_2_data_47258`-style
naming artifact described above (2 lines) plus one genuine remaining
`.rodata` pool-offset gap, both **out of this brief's scope** (`__sinit` is
compiler-synthesised machinery for `dWmLib::sc_ForceList`, not a
`daWmSmallCloud_c` member function) -- flagging for whoever next touches
that function, not fixing here.

## Confidence

High on all five changes: each is independently measured (compiled and
diffed before and after, in the sequence above), not stacked speculatively.
`createModel()` is now **byte-exact, 101/101**, verified with the exact
command the brief specifies, against all three real target objects.
