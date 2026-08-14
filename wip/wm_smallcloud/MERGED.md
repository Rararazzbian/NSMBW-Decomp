# MERGED -- d_a_wm_smallcloud.cpp

Reconciliation of batch A (`wip/wm_smallcloud/BATCHA.md`) and batch B
(`wip/wm_smallcloud/BATCHB.md`) into one TU. Worked in
`wip/wm_smallcloud/scratch/merged/`.

**The claimed 14/16 union is now a measured fact from a single real compile**,
not arithmetic over two separate compiles. Verifier table below, produced by
`python wip/wm_smallcloud/scratch/merged/build.py`, which compiles the merged
source through `harness.compile_draft(..., module='d_basesNP')` (the module
flag fix mentioned in the brief) and then runs `wip/wm_units/verify_anon.py`
against all three target objects:
`bin/dtkspl/d_basesNP/obj/auto_00_001797B4_text.o`,
`bin/dtkspl/d_basesNP/obj/auto_fn_2_179F40_text.o`,
`bin/dtkspl/d_basesNP/obj/auto_00_00179FC4_text.o`.

## Verifier table (verbatim)

```
addr       target                  size  result
0x001797e0 fn_2_1797E0               12  MATCH  <- daWmSmallCloud_c_classInit__Fv
0x00179810 fn_2_179810               31  MATCH  <- __ct__16daWmSmallCloud_cFv
0x00179890 fn_2_179890               57  MATCH  <- __dt__16daWmSmallCloud_cFv
0x00179980 fn_2_179980               62  MATCH  <- create__16daWmSmallCloud_cFv
0x00179a80 fn_2_179A80               59  MATCH  <- execute__16daWmSmallCloud_cFv
0x00179b70 fn_2_179B70               12  MATCH  <- draw__16daWmSmallCloud_cFv
0x00179ba0 fn_2_179BA0                2  MATCH  <- doDelete__16daWmSmallCloud_cFv
0x00179bb0 fn_2_179BB0              101  95 differing vs createModel__16daWmSmallCloud_cFv
0x00179d50 fn_2_179D50               44  MATCH  <- calcModel__16daWmSmallCloud_cFv
0x00179e00 fn_2_179E00               40  MATCH  <- initState__16daWmSmallCloud_cFv
0x00179ea0 fn_2_179EA0                3  MATCH  <- init_exec__16daWmSmallCloud_cFv
0x00179eb0 fn_2_179EB0                1  MATCH  <- mode_exec__16daWmSmallCloud_cFv
0x00179ec0 fn_2_179EC0               17  MATCH  <- processCutsceneCommand__16daWmSmallCloud_cFib
0x00179f10 fn_2_179F10               10  MATCH  <- setPosFromCourseNode__16daWmSmallCloud_cFv
0x00179f40 fn_2_179F40               33  5 differing vs "__sinit_\d_a_wm_smallcloud_cpp"
0x00179fd0 fn_2_179FD0                7  MATCH  <- __arraydtor$12784

14/16 byte-identical modulo symbol names
```

**No regressions**: every function either batch reported MATCH on its own
still MATCHes in the merged TU (A's 7: classInit, ctor, dtor, create,
execute, draw, doDelete; B's 7: calcModel, initState, init_exec, mode_exec,
processCutsceneCommand, setPosFromCourseNode, and now also the arraydtor --
see below). The two functions neither batch closed alone (`createModel`,
`__sinit`) remain unclosed, unchanged in shape, after merging.

## The layout contradiction, resolved with evidence

**The premise as stated ("B's arraydtor matches, A's does not") is true but
does not mean what it looks like it means. It has nothing to do with
`mChrAnim` or with `daWmSmallCloud_c`'s field layout at all.**

Directly tested by compiling batch A's own, unmodified `draft.cpp` (its
existing placeholder bodies, no `d_wm_lib.hpp`) against the same three
target objects the merge uses:

```
0x00179f40 fn_2_179F40               33  32 differing vs __dt__14dWmDemoActor_cFv
0x00179fd0 fn_2_179FD0                7  7 differing vs init_exec__16daWmSmallCloud_cFv

11/16 byte-identical modulo symbol names
```

Batch A's solo draft does not produce anything resembling `__sinit`/
`__arraydtor` at that address at all -- the closest "match" is an unrelated
leftover function, i.e. batch A's compiled object never emits those symbols.

**Why**: `0x179FD0`'s target is the array destructor for `dWmLib::sc_ForceList`,
a `static ForceInCourseList_t sc_ForceList[]` defined directly in
`include/game/bases/d_wm_lib.hpp:84` with a **runtime-dependent initialiser**
(`dCsvData_c::c_CASTLE_ID`, not a compile-time constant, feeding an
`mVec3_c(...)` element). Per `AGENT_CONTEXT.md`'s rule ("a header static with
a non-trivial constructor is emitted into EVERY TU that odr-uses it"), any TU
that includes `d_wm_lib.hpp` gets its own private copy of `sc_ForceList` plus
its own compiler-synthesised `__sinit_<file>_cpp` (registers it) and
`__arraydtor$NNNNN` (destroys it at exit) -- confirmed by batch B's own report
(`BATCHB.md`, `0x179F40`/`0x179FD0` sections), which already worked this out
correctly and is corroborated here directly.

**Batch A's 8 functions never call anything that needs `d_wm_lib.hpp`**
(`initState()`/`init_exec()`/`updatePos()` in A's scope are placeholders that
only touch `mCurrProc`). Batch B's real `initState()` body, in contrast, calls
`dWmLib::IsCourseClear(5, 0x17)`, which requires `<game/bases/d_wm_lib.hpp>`.
That's the entire explanation: **A's solo TU has no reason to include
`d_wm_lib.hpp`, so the compiler never synthesises `sc_ForceList`'s
`__sinit`/`__arraydtor` pair there; B's solo TU does include it (because its
own function needs `IsCourseClear`), so the pair appears.** It is a
header-inclusion fact about which functions call what, not a disagreement
about `daWmSmallCloud_c`'s own members.

**Consequently `mChrAnim[ANIM_COUNT=1]` is not "the array" behind
`__arraydtor$12784`.** `mChrAnim` is destructed in place, inline, inside
`daWmSmallCloud_c::~daWmSmallCloud_c()` itself (already proven MATCH,
57/57 instructions, byte-identical to the twin's dtor shape) -- a 1-element
array of a class member does not need or get a separate
`__register_global_object`/`__arraydtor` pair; that machinery is specifically
for **file/namespace-scope statics with dynamic initialisation**, which is
exactly and only what `sc_ForceList` is.

**Confirmation in the merged TU**: once `d_wm_lib.hpp` is included (needed for
real, by `initState()`'s `dWmLib::IsCourseClear` call) `0x179FD0` MATCHes
byte-exact with zero source written for it, exactly as batch B predicted.

**Net effect on the layout question**: this fully supports, and does not
contradict, batch A's finding that `daWmSmallCloud_c` drops
`mGroupNodeIds[20]`/`mCurrNodeClipSphere` (`sizeof == 0x1fc`). That claim
rests on `classInit`'s `li r3, 0x1fc` operand and three independently
byte-matching functions (ctor/dtor/`create()`) all agreeing on `mpBgmSync`
sitting at `0x1f8`, `0x60` less than the twin's `0x258` -- untouched by any of
the above, and now additionally cross-validated because the merged TU's
`execute()`, `initState()`, `init_exec()`, and `calcModel()` (all reading/
writing base-class and own-class offsets up to and including `mCurrProc` at
`0x1f4`) also all MATCH using that same, smaller layout.

**The stale-name observation both batches flagged independently stands, not
resolved, correctly left alone**: `daWmCloud_c`'s own landed header names the
pad field immediately before its `mCurrProc` `mUnk250`, which by this
codebase's own convention should mean offset `0x250`, but `mCurrProc` is
measured at `0x1f4` in that class too (its own `execute()`). Not touched here
-- `d_a_wm_cloud.hpp` is landed and out of scope for this unit -- flagged again
for whoever next touches that header.

## Open items (not closed by the merge, documented not manufactured)

### `0x179BB0` -- `createModel()` -- 95/101 differing

Unchanged from batch A's own finding. The algorithm/control-flow is fully
verified against real bytes (heap alloc, `sprintf`-built archive name,
`ACTOR_PARAM(CourseNo)`-indexed 4-entry model/anim name tables, tail
identical to the twin's `createModel()`). What blocks closure: the target's
`_savegpr_25`/`_restgpr_25` register-save strategy (7 saved registers) and
`addi rX, rBASE, {0x88, 0x98, 0xa0}` immediates imply the sprintf format
string, `"g3d/model.brres"`, and `resMdlNames` sit contiguously inside one
larger aggregate `.data` object whose leading `0x88` (136) bytes are unknown
-- see `BATCHA.md`'s "Variants tried and discarded" for four attempted shapes,
none matching. Not re-attempted here; the merge did not change anything
relevant to this function (it does not touch anything batch B owns).

### `0x179F40` -- `__sinit_d_a_wm_smallcloud_cpp` -- 5/33 differing

Structurally 100% correct (compiler-synthesised, zero hand-written source, and
now genuinely present given the merged TU includes `d_wm_lib.hpp`). The 5
remaining diff lines, checked instruction-by-instruction against the merged
draft:

- **2 are a `verify_anon.py` normalisation artifact**, not a real difference:
  `dtk` names this object's own local-pool relocation `...rodata.0@ha` (an
  anonymous-form symbol with a leading `...`); `verify_anon.py`'s `norm()`
  regex requires the match to start at a word/`@`/`$` character, so it
  rewrites `...rodata.0@ha` to `...SYM@ha` (leading dots survive) while the
  target's differently-named local symbol normalises cleanly to `SYM@ha`.
  Both sides are references to an anonymous local `.rodata` symbol; this is
  the same class of artifact both batch reports already called out for their
  own compiles.
- **3 are real**: `lfs f2,0x24(r5)` / `f1,0x28(r5)` / `f0,0x2c(r5)` in the
  target (the pooled floats for `sc_ForceList[0].mNodePos`,
  `2160.0f, -30.0f, -478.0f`) vs `0x1c`/`0x20`/`0x24` in the merged draft --
  an **8-byte gap**, down from the 28-byte gap batch B measured testing its
  own functions in isolation (`BATCHB.md`: "the gap is `0x24 - 0x8 = 0x1c`").
  **Merging shrank the gap by exactly 20 bytes** by supplying the rest of the
  TU's real `.rodata`-contributing code (`create()`'s `mClipSphere.set`,
  `ACTOR_PROFILE`'s data, etc.), which is itself evidence the remaining 8
  bytes come from the same place: real TU content this merge still does not
  have -- most likely additional pooled constants from `createModel()`'s own
  still-unclosed body (see above), which sits earlier in the file and would
  contribute to the same per-TU local `.rodata` pool before `__sinit`'s
  floats do. Not chased further; flagging the coupling for whoever next
  attempts `createModel()`.

Both open items were already honestly reported as not-closed by their
originating batch; the merge did not close either, and did not introduce
any new open item.

## Proposed shared-header change (not applied, per the rules)

One shared-header addition is required to compile `setPosFromCourseNode()`
(batch B's finding, `0x179F10`): `daWmMap_c` needs a `GetNodePos` overload
taking a name instead of an index. Evidenced by the callee's mangled name in
the target, `GetNodePos__9daWmMap_cFPCcR7mVec3_c`.

```diff
--- include/game/bases/d_a_wm_map.hpp
+++ include/game/bases/d_a_wm_map.hpp
@@
     int GetNodeCount(int); ///< @unofficial
     void GetNodePos(long nodeIdx, mVec3_c &pos);
+    /// @unofficial looks up a node by name instead of index; evidenced by
+    /// daWmSmallCloud_c::setPosFromCourseNode() (0x179F10) calling
+    /// GetNodePos__9daWmMap_cFPCcR7mVec3_c.
+    void GetNodePos(const char *nodeName, mVec3_c &pos);
```

Compiled against a shadow copy only
(`wip/wm_smallcloud/scratch/merged/shadow_include/game/bases/d_a_wm_map.hpp`),
never against the real header, per the rules.

## Naming decisions made when merging (both were legitimate working names for the same function)

- `0x179F10`: kept batch B's `setPosFromCourseNode()` over batch A's
  `updatePos()` -- more descriptive of the measured behaviour, and it's
  batch B's own function (A only stubbed it as a placeholder). A's
  `execute()` call site updated to match (`setPosFromCourseNode();` instead
  of `updatePos();`) -- purely a rename, no behavioural or offset change.
- `0x179E00`: named `initState()` (dropping batch B's working name
  `initStateLike()`, which existed only to avoid colliding with batch A's
  own placeholder `initState()` declaration during independent authoring).
  Matches the twin's own `daWmCloud_c::initState()` naming and there is no
  longer a collision once merged.
- `ANIM_e` enumerator: kept batch A's `CS_W7_SmallCloud` (matches the twin's
  `CS_W7_Cloud` naming convention) over batch B's `CS_Anim`; batch B's
  `initState()` body updated to reference `mChrAnim[CS_W7_SmallCloud]`
  instead of `mChrAnim[CS_Anim]` -- purely a rename.
- `mGroupNodeIds`/`mCurrNodeClipSphere`/`NODE_COUNT`: dropped. Batch A's
  header omits them (proven absent); batch B's shadow header carried them
  only as untested twin-copied placeholders and explicitly deferred to
  batch A on this point in `BATCHB.md`.

## Compiler-flag finding (confirmed a third time)

Both batches independently found `tools/auto_decomp/harness.py`'s old
hardcoded `CFLAGS` (no `-sdata 0 -sdata2 0`, plain `-O4`) produce spurious
`@sda21` small-data diffs that have nothing to do with the source, versus
`slices/d_basesNP.json`'s real `-sdata 0 -sdata2 0 -O4,p -char signed
-rtti off`. This merge used `harness.compile_draft(..., module='d_basesNP')`,
confirming the fix mentioned in the brief is in place and produces the correct
flag set without any hand-rolled `CFLAGS` list.

## Merged header

`wip/wm_smallcloud/scratch/merged/include/game/bases/d_a_wm_smallcloud.hpp`:

```cpp
#pragma once

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_wm_bgm_sync.hpp>

/**
* @brief The actor for the small decorative clouds used in the World 7 map.
* @details A near-twin of #daWmCloud_c: a single-bone animated cloud model, synchronised to the
* background music via #mpBgmSync, with no per-node culling (unlike daWmCloud_c, which culls each
* of its named bone groups individually every frame). Instead, every frame it repositions itself to
* a single named world-map node selected by ACTOR_PARAM(CourseNo) (see #setPosFromCourseNode).
* @unofficial Reconstructed from anonymous (unnamed) target symbols; class name, member names and
* the exact GlobalData_t shape are inferred from codegen evidence, not from any mangled name.
* @ingroup bases
*/
class daWmSmallCloud_c : public dWmObjActor_c {
public:
    /// @brief The global configuration for the actor.
    /// @unofficial Shape inferred purely from create()'s pool-data references: the two s16[2]
    /// arrays are read with NO extra offset added after the GLOBAL_DATA base address, so they are
    /// believed to be the object's first (and, as far as measured evidence goes, only) members.
    /// mUnofficialPad exists ONLY to push mData past MWCC's small-data threshold under
    /// -sdata 0 -sdata2 0; its true size/content is unconfirmed. See MERGED.md.
    struct GlobalData_t {
        s16 mBgmValueW5[2]; ///< @unofficial BGM sync value used when dScWMap_c::m_WorldNo == 5.
        s16 mBgmValue[2]; ///< @unofficial BGM sync value used otherwise.
        u8 mUnofficialPad[8]; ///< @unofficial Padding only, see struct comment.
    };

    /// @brief The available animations for this actor.
    /// @unofficial Name copied from daWmCloud_c's ANIM_e; only the single-entry shape is confirmed.
    enum ANIM_e {
        CS_W7_SmallCloud,
        ANIM_COUNT
    };

    typedef void (daWmSmallCloud_c::*ProcFunc)();

    daWmSmallCloud_c(); ///< @copydoc dWmObjActor_c::dWmObjActor_c
    ~daWmSmallCloud_c(); ///< @copydoc dWmObjActor_c::~dWmObjActor_c

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    void createModel(); ///< Initializes the resources for the actor. @unofficial NOT byte-matched, see MERGED.md.
    void calcModel(); ///< Updates the model's transformation matrix.
    void initState(); ///< Sets up the actor's initial state (course-node position, anim rate, and the
                       ///< CourseNo == 3 castle-clear visibility gate).
    void init_exec(); ///< Process initialization function for the @ref dWmObjActor_c::PROC_TYPE_EXEC "exec" process type.
    void mode_exec(); ///< Process function for the @ref dWmObjActor_c::PROC_TYPE_EXEC "exec" process type.

    /// @brief Repositions the actor to a named world-map node, indexed by ACTOR_PARAM(CourseNo).
    /// @unofficial daWmCloud_c has no equivalent; that unit culls named bone GROUPS instead of
    /// repositioning to a single node. Table contents are unconfirmed placeholders, see MERGED.md.
    void setPosFromCourseNode();

    u32 mUnk188; ///< @unused @unofficial offset 0x188, identical position to daWmCloud_c::mUnk188.
    dHeapAllocator_c mAllocator; ///< The allocator.
    nw4r::g3d::ResFile mResFile; ///< The resource file.
    m3d::smdl_c mModel; ///< The model.
    m3d::anmChr_c mChrAnim[ANIM_COUNT]; ///< The model animations.
    u32 mUnk1f0; ///< @unused @unofficial offset 0x1f0 (daWmCloud_c's equivalent field is
                 ///< misleadingly named mUnk250 -- see MERGED.md "layout contradiction" section).
    PROC_TYPE_e mCurrProc; ///< The current process type. See dWmObjActor_c::PROC_TYPE_e.
    dWmBgmSync_c *mpBgmSync; ///< The background music synchronization helper, @ 0x1f8.
    // Total size 0x1fc (measured, classInit's operand). Deliberately NO mGroupNodeIds/
    // mCurrNodeClipSphere/sGroupNodeNames -- see MERGED.md for the full proof and the
    // arraydtor contradiction this raised and how it was resolved.
};
```

## Merged source

`wip/wm_smallcloud/scratch/merged/d_a_wm_smallcloud.cpp`:

```cpp
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_a_wm_smallcloud.hpp>
#include <game/sLib/s_GlobalData.hpp>

ACTOR_PROFILE(WM_SMALLCLOUD, daWmSmallCloud_c, 0);

daWmSmallCloud_c::daWmSmallCloud_c() {}

daWmSmallCloud_c::~daWmSmallCloud_c() {
    if (mpBgmSync != nullptr) {
        delete mpBgmSync;
    }
}

int daWmSmallCloud_c::create() {
    dWmBgmSync_c *bgmSync = new dWmBgmSync_c();
    mpBgmSync = bgmSync;

    if (dScWMap_c::m_WorldNo == 5) {
        bgmSync->m_18 = GLOBAL_DATA.mBgmValueW5;
        bgmSync->m_04 = GLOBAL_DATA.mBgmValueW5[0] - 1;
        bgmSync->m_08 = GLOBAL_DATA.mBgmValueW5[1];
    } else {
        bgmSync->m_18 = GLOBAL_DATA.mBgmValue;
        bgmSync->m_04 = 7;
        bgmSync->m_08 = 0;
    }

    createModel();
    mClipSphere.set(mPos, 400.0f);

    calcModel();
    initState();
    return SUCCEEDED;
}

int daWmSmallCloud_c::execute() {
    mpBgmSync->execute();
    if (mpBgmSync->m_0c) {
        float rate = mpBgmSync->getAnmRate(mChrAnim->mFrameMax);
        mChrAnim->setRate(rate);
    }

    static const ProcFunc Proc_tbl[PROC_COUNT] = {
        &daWmSmallCloud_c::mode_exec
    };

    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    if (csSeqMng->FUN_80915600()) {
        processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);
    } else {
        (this->*Proc_tbl[mCurrProc])();
    }

    setPosFromCourseNode();
    mModel.play();
    calcModel();

    return SUCCEEDED;
}

int daWmSmallCloud_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmSmallCloud_c::doDelete() {
    return SUCCEEDED;
}

void daWmSmallCloud_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId != dCsSeqMng_c::CUTSCENE_CMD_NONE && !isStaff()) {
        mIsCutEnd = true;
    }
}

// NOT CLOSED -- see MERGED.md. Best-effort proposal, 95 instructions differing from the
// 101-instruction target (register-save strategy and a shared-base-register table address
// computation this version does not reproduce; the target's addi rX,rBASE,{0x88,0x98,0xa0}
// immediates imply the sprintf format string, "g3d/model.brres" and resMdlNames are laid out
// contiguously in one larger .data/.rodata aggregate this batch could not fully identify).
void daWmSmallCloud_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    char arcName[8];
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

    static bool sInit = false;
    static const char *resAnmNames[4];
    if (!sInit) {
        resAnmNames[0] = resMdlNames[0];
        resAnmNames[1] = resMdlNames[1];
        resAnmNames[2] = resMdlNames[2];
        resAnmNames[3] = resMdlNames[3];
        sInit = true;
    }

    static const m3d::playMode_e playModes[ANIM_COUNT] = {
        m3d::FORWARD_LOOP
    };

    nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr(resAnmNames[ACTOR_PARAM(CourseNo)]);
    mChrAnim[CS_W7_SmallCloud].create(resMdl, resAnmChr, &mAllocator, nullptr);
    mChrAnim[CS_W7_SmallCloud].mPlayMode = playModes[CS_W7_SmallCloud];
    mChrAnim[CS_W7_SmallCloud].setRate(0.0f);
    mChrAnim[CS_W7_SmallCloud].setFrame(0.0f);
    mModel.setAnm(mChrAnim[CS_W7_SmallCloud]);

    dWmActor_c::setSoftLight_Map(mModel);
    mAllocator.adjustFrmHeap();
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

void daWmSmallCloud_c::initState() {
    mChrAnim[CS_W7_SmallCloud].setRate(1.0f);
    mChrAnim[CS_W7_SmallCloud].setFrame(0.0f);
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

void daWmSmallCloud_c::init_exec() {
    mCurrProc = PROC_TYPE_EXEC;
}

void daWmSmallCloud_c::mode_exec() {}

void daWmSmallCloud_c::setPosFromCourseNode() {
    // batchA's createModel() independently found a 4-entry table indexed the same way
    // (resMdlNames[ACTOR_PARAM(CourseNo)]); mirroring that table size here since it is
    // likely the real number of small-cloud variants, but the actual node-name strings
    // are unconfirmed -- placeholders only. See MERGED.md.
    static const char *nodeNames[4] = {
        "F0C0", "F0C1", "F0C2", "F0C3"
    };
    daWmMap_c::m_instance->GetNodePos(nodeNames[ACTOR_PARAM(CourseNo)], mPos);
}
```

## What would raise confidence further

- **`createModel()`**: the real `.data`/`.rodata` content for the whole TU
  in file order, to identify the leading `0x88` bytes of the aggregate the
  target's register-save strategy implies. Same open question batch A left.
- **`__sinit`**: the same content would settle the remaining 8-byte pool
  offset (very likely a consequence of closing `createModel()` first, since
  that function's own pooled constants precede `__sinit`'s in file order).
- **`setPosFromCourseNode()`'s `nodeNames[]` and `createModel()`'s
  `resMdlNames[]` contents**: `resMdlNames[]` is decoded from real `.data`
  bytes (high confidence); `nodeNames[]` is still an unconfirmed placeholder
  guess (`"F0C0".."F0C3"`) -- the verifier is relocation-transparent to
  string pool contents, so this cannot be settled without the real `.data`.
