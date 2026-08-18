#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/bases/d_effect.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/sLib/s_State.hpp>

/// @unofficial fn_2_171400, size 0xC (an `lwz`/`clrlwi`-style trivial getter),
/// called through `this` (r3 unmodified from entry) inside executeState_Ready
/// -- lives at 0x171400, well outside this unit's own .text range and still
/// unnamed anywhere in the project's symbol map (neighbouring functions in
/// that range are all anonymous `fn_2_` too -- an un-landed/un-decompiled
/// TU, same situation as kinoko_1up's dependency). Declared extern "C" under
/// its raw target symbol name so the linker resolves the `bl` directly,
/// matching the FUN_XXXXXXXX placeholder convention already used for
/// undocumented cross-TU calls (see FUN_80915600 in d_cs_seq_manager.hpp).
extern "C" int fn_2_171400();

/// @unofficial Provisional reconstruction of daWmSandPillar_c. NOT landed --
/// draft, sizeof/layout being verified this round via compiled probe.
///
/// Member layout traced from the target constructor (fn_2_1776C0):
///   dWmObjActor_c base       ends 0x188 (mResNodeIdx=-1 at +0x184, inlined)
///   u32 mUnk188               +0x188  never touched by the ctor
///   dHeapAllocator_c mAllocator  +0x18c  (ctor: __ct__16dHeapAllocator_cFv)
///   nw4r::g3d::ResFile mResFile   +0x1a8  (zeroed, matches d_a_enemy_ice.hpp's
///                                          mAllocator/mResFile/mModel pattern)
///   m3d::mdl_c mModel              +0x1ac  (ctor: __ct__Q23m3d5mdl_cFv)
///   m3d::anmChr_c mAnim              +0x1ec  (ctor: __ct__Q23m3d6fanm_cFv, then
///                                             vtable overwritten to anmChr_c's --
///                                             anmChr_c's own ctor is trivial)
///   m3d::anmTexSrt_c mAnimTexSrt      +0x228  (vtable set directly; its own
///                                             mAllocator_c sub-member confirmed
///                                             constructed at +0xc internal)
///   dEf::dLevelEffect_c mEffect1       +0x260  (EGG::Effect ctor, then
///                                             mEf::levelEffect_c's field-zero
///                                             list m_114/m_118/m_11c/m_11d/
///                                             m_120/m_124 -- matches the real,
///                                             landed levelEffect_c ctor
///                                             initializer list exactly)
///   dEf::dLevelEffect_c mEffect2        +0x388  (0x388-0x260 = 0x128 =
///                                             sizeof(dLevelEffect_c), confirmed
///                                             two independent ways: EGG::Effect
///                                             works out to exactly 0x114 --
///                                             matching levelEffect_c's own
///                                             field-name self-documentation --
///                                             AND the second instance lands
///                                             exactly one stride later)
///   sFStateMgr_c<daWmSandPillar_c,
///                sStateMethodUsr_FI_c> mStateMgr;  +0x4b0  (0x260 + 2*0x128
///                                             lands exactly here; matches the
///                                             embedded-member pattern
///                                             `d_actor_state.hpp` documents,
///                                             here embedded directly since
///                                             this class derives from
///                                             dWmObjActor_c, not dActor_c)
///
/// sizeof(daWmSandPillar_c) == 0x508 per classInit's `li r3, 0x508`. Verified
/// below with a compiled probe rather than trusted from arithmetic alone.
///
/// State names read directly out of `original/d_basesNP.rel` .data
/// (0x46fe8-0x47145, string literals `daWmSandPillar_c::StateID_<name>`):
/// Ready, BottomWait, MoveReady, MoveUp, TopWait, MoveDown,
/// BottomWaitForever, TopWaitForever, TopWaitFromTheStart. Nine states.
/// Idiom copied from source/dol/bases/d_a_enemy_ice.cpp (STATE_FUNC_DECLARE/
/// STATE_DEFINE), the smallest of the six already-landed TUs using this
/// framework (`grep -rl STATE_DEFINE source/`).
class daWmSandPillar_c : public dWmObjActor_c {
public:
    daWmSandPillar_c();
    virtual ~daWmSandPillar_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    /// @unofficial NEW this round: check_vtable.py caught that slot 24
    /// resolves to fn_2_178860 (IN-UNIT), not the inherited
    /// dWmDemoActor_c::processCutsceneCommand() my draft was silently
    /// falling back to. Body confirmed: the same
    /// `cutsceneCommandId != CUTSCENE_CMD_NONE && !isStaff()` idiom already
    /// landed in d_a_wm_cloud.cpp.
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);
    /// @unofficial doDelete() is deliberately NOT declared here. Its vtable
    /// slot (5) resolves to fn_2_15ABC0 (`li r3,1; blr`), which is
    /// `dWmDemoActor_c::doDelete()`'s own inline body (`return SUCCEEDED;`,
    /// d_wm_demo_actor.hpp) -- confirmed by a second, unrelated vtable
    /// (lbl_2_data_482D0) pointing at the exact same address for the exact
    /// same slot, which is only possible if neither class overrides it.
    /// Declaring our own doDelete() here would be a REAL defect, not a
    /// harmless duplicate.

    /// @unofficial slot 23, IN-UNIT (fn_2_177CE0, `blr`) -- confirmed by
    /// checking the vtable directly, unlike the 12 other trivial stubs below
    /// which do NOT appear in this 30-slot vtable at all and are therefore
    /// plain non-virtual helpers, not overrides.
    virtual void finalUpdate();

    /// @unofficial fn_2_177D20 (0x144 bytes), called first from create().
    /// Not yet analysed -- placeholder body.
    void createMdl();
    /// @unofficial fn_2_177C30 -- confirmed: builds mMatrix from mPos/mAngle
    /// (PSMTXTrans/ZXYrotM), then mModel.setLocalMtx/setScale/calc(false).
    /// Matches the calcModel() idiom already landed in every wm sibling.
    void calcMdl();
    /// @unofficial fn_2_1788B0, called from execute(). Reads a node world
    /// position off mModel via getNodeID/getNodeWorldMtxMultVecZero, then
    /// calls a virtual (vtable+0xa0) on both mEffect1 and mEffect2 with node
    /// name strings -- matches d_a_enemy_ice.cpp's calcEffectPos()/follow()
    /// idiom. Not yet fully analysed -- placeholder body.
    void calcEffectPos();
    /// @unofficial fn_2_177E70. Confirmed byte-for-byte: a classic
    /// "approach with clamp" utility -- drives mApproachCurrent toward
    /// mApproachTarget by mApproachStep, snapping exactly to the target
    /// once a step would overshoot it.
    void approach();

    STATE_FUNC_DECLARE(daWmSandPillar_c, Ready);
    STATE_FUNC_DECLARE(daWmSandPillar_c, BottomWait);
    STATE_FUNC_DECLARE(daWmSandPillar_c, MoveReady);
    STATE_FUNC_DECLARE(daWmSandPillar_c, MoveUp);
    STATE_FUNC_DECLARE(daWmSandPillar_c, TopWait);
    STATE_FUNC_DECLARE(daWmSandPillar_c, MoveDown);
    STATE_FUNC_DECLARE(daWmSandPillar_c, BottomWaitForever);
    STATE_FUNC_DECLARE(daWmSandPillar_c, TopWaitForever);
    STATE_FUNC_DECLARE(daWmSandPillar_c, TopWaitFromTheStart);

    /// @unofficial The remaining 2 confirmed-trivial (`blr`-only) functions
    /// that are NOT state methods -- the other 11 of the original 13 "unk"
    /// candidates turned out to be the 9 states' initializeState_X methods
    /// (all `blr`) plus executeState_BottomWaitForever/executeState_
    /// TopWaitForever (also `blr`), recovered this round from __sinit's own
    /// .bss layout of the 9 sStateID_c sub-objects (see executeState_Ready).
    /// These 2 remain unidentified; not present in the main 30-slot vtable.
    void unk1780F4();
    void unk1785D8();

    u32 mUnk188; ///< @unofficial never touched by the ctor
    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;
    m3d::mdl_c mModel;
    m3d::anmChr_c mAnim; ///< @unofficial name
    /// @unofficial GAP 1 -- NAMED this round from its use site in createMdl()
    /// (fn_2_177D20): `stw r3, 0x224(r29)` stores GetResAnmTexSrt()'s return
    /// value directly, and it's read back for the mAnimTexSrt.create() call.
    /// A persisted resource handle, matching d_a_enemy_ice.hpp's own
    /// mResAnmTexSrt member sitting in the equivalent position.
    nw4r::g3d::ResAnmTexSrt mResAnmTexSrt;
    m3d::anmTexSrt_c mAnimTexSrt; ///< @unofficial name
    /// @unofficial GAP 2 -- NAMED this round from its use site in create()
    /// (fn_2_177AB0): `stfs f1,0x254(r31)` stores mPos.x, `stfs f1,0x258`
    /// stores a per-type-looked-up height (overwriting a copy of mPos.y),
    /// `stfs f0,0x25c` stores mPos.z -- a 3-float (mVec3_c) starting
    /// position, saved once at create() time. @unofficial name.
    mVec3_c mStartPos;
    dEf::dLevelEffect_c mEffect1; ///< @unofficial name
    dEf::dLevelEffect_c mEffect2; ///< @unofficial name
    sFStateMgr_c<daWmSandPillar_c, sStateMethodUsr_FI_c> mStateMgr;
    /// @unofficial GAP 3, localised this round: 28 bytes AFTER mStateMgr --
    /// mStateMgr's own compiled size (0x3c, confirmed by this same probe) is
    /// the same real template on both sides, so this gap sits past it, not
    /// before it as originally guessed. mStateMgr ends at 0x4b0+0x3c=0x4ec.
    /// Split into 7 individually-typed fields this round (was a single u32
    /// array, but +0x4f4/+0x4f8/+0x500 are read/written with lfs/stfs in
    /// fn_2_177E70 -- confirmed float, not a homogeneous u32 array).
    s32 mUnk4EC; ///< @unofficial +0x4ec, zeroed by finalizeState_BottomWait.
                 ///< @unofficial signed: executeState_MoveReady's `== 1`
                 ///< check compiles to `cmpwi` (signed) against target, not
                 ///< `cmplwi`.
    s32 mUnk4F0; ///< @unofficial +0x4f0, a countdown: set by
                 ///< finalizeState_BottomWait, decremented by
                 ///< executeState_BottomWait.
    f32 mApproachCurrent; ///< @unofficial +0x4f4, driven toward
                          ///< mApproachTarget by approach().
    f32 mApproachTarget;  ///< @unofficial +0x4f8, approach()'s target value.
    u32 mUnk4FC; ///< @unofficial +0x4fc, unknown.
    f32 mApproachStep; ///< @unofficial +0x500, approach()'s step size.
    u32 mReadyFlag; ///< @unofficial +0x504, written by finalizeState_Ready,
                    ///< read by executeState_Ready.

    /// @unofficial Read as `(mParam & 0xff)` with no extra shift beyond the
    /// compiler's own array-of-float indexing -- bits[0:8) is the simplest
    /// fit. Name and true bit width unconfirmed.
    ACTOR_PARAM_CONFIG(Type, 0, 8);
};

STATE_DEFINE(daWmSandPillar_c, Ready);
STATE_DEFINE(daWmSandPillar_c, BottomWait);
STATE_DEFINE(daWmSandPillar_c, MoveReady);
STATE_DEFINE(daWmSandPillar_c, MoveUp);
STATE_DEFINE(daWmSandPillar_c, TopWait);
STATE_DEFINE(daWmSandPillar_c, MoveDown);
STATE_DEFINE(daWmSandPillar_c, BottomWaitForever);
STATE_DEFINE(daWmSandPillar_c, TopWaitForever);
STATE_DEFINE(daWmSandPillar_c, TopWaitFromTheStart);

ACTOR_PROFILE(WM_SANDPILLAR, daWmSandPillar_c, 0);

/// @unofficial Single merged per-Type constant table -- ONE 0x84-byte object
/// (lbl_2_rodata_8EF8, 33 words) confirmed against original/d_basesNP.rel's
/// .rodata+0x8ef8. The eleven per-function local `static const` tables this
/// TU used to declare each compiled into their OWN separate pool object;
/// the target has exactly one, so they must be a single file-scope
/// aggregate. ROW_OFFSET = row_index * 0xc matches every attributed
/// function's observed byte offset into the object -- confirmed for row3
/// (height, both create() and execute()), row4 (BottomWait-final scale AND
/// MoveDown-exec target -- the SAME row read from two different functions),
/// row5 (MoveUp target), row7 (MoveUp threshold), row9 (BottomWait
/// counter), row10 (TopWait counter). Rows 0/1/2/6/8 are read directly out
/// of the same blob but not yet attributed to a call site -- candidates are
/// finalizeState_MoveReady/executeState_MoveReady (mApproachTarget/
/// mApproachStep setup) and the not-yet-analysed BottomWaitForever/
/// TopWaitForever/TopWaitFromTheStart bodies. Row8's raw bits (0x3, 0x1,
/// 0x1) are not a plausible float (a subnormal near zero) -- typed s32 by
/// the same reasoning that types rows 9/10 (0x64/0xa/0x28, 0x64/0x64/0x64)
/// as s32 rather than float; those two are independently confirmed by their
/// consuming functions being declared `int`/`s32` already.
struct SandPillarTypeTable_t {
    float mUnk0[3];                 ///< @unofficial row0: {0.01, 0.01, 0.01}
    float mUnk1[3];                 ///< @unofficial row1: {0.001, 0.003, 0.003}
    float mUnk2[3];                 ///< @unofficial row2: {0.03, 0.05, 0.05}
    float mHeightTable[3];          ///< @unofficial row3: create()/execute()
    float mScaleTable[3];           ///< @unofficial row4: BottomWait-final scale / MoveDown-exec target
    float mMoveUpTargetTable[3];    ///< @unofficial row5
    float mUnk6[3];                 ///< @unofficial row6: {0.4, 0.1, 0.1}
    float mMoveUpThresholdTable[3]; ///< @unofficial row7
    s32 mUnk8[3];                   ///< @unofficial row8: {3, 1, 1}
    s32 mBottomWaitCounterTable[3]; ///< @unofficial row9
    s32 mTopWaitCounterTable[3];    ///< @unofficial row10
};

static const SandPillarTypeTable_t smc_TypeTable = {
    {0.01f, 0.01f, 0.01f},
    {0.001f, 0.003f, 0.003f},
    {0.03f, 0.05f, 0.05f},
    {-320.0f, -200.0f, -200.0f},
    {0.5f, 0.2f, 0.2f},
    {2.0f, 1.4f, 1.6f},
    {0.4f, 0.1f, 0.1f},
    {1.1f, 1.8f, 1.8f},
    {3, 1, 1},
    {100, 10, 40},
    {100, 100, 100},
};

daWmSandPillar_c::daWmSandPillar_c() : mStateMgr(*this, StateID_Ready) {}

/// @unofficial STALE COMMENT CORRECTED THIS ROUND: fn_2_177970 (0x140 bytes)
/// and all four "helper" functions previously listed as unaccounted for
/// (fn_2_177820/177860/1778A0/177900 -- the weak dtors for mVec3_c,
/// EGG::Vector3f, sStateMgr_c<...> and sFStateMgr_c<...>) were ALREADY
/// MATCHING before this round even started: an empty user destructor still
/// triggers the full compiler-generated member/base teardown cascade, and
/// that cascade was already correct from the class layout alone. There is
/// nothing left to write here -- confirmed via verify_anon.py, not assumed.
///
/// One order violation remains nearby (`__dt__Q23mEf8effect_cFv` "defined
/// too late") but it is NOT this destructor's fault. Investigated this
/// round: the target function verify_anon pairs it with, fn_2_179290 (near
/// __sinit/__arraydtor at the end of the TU), is NOT mEf::effect_c's dtor at
/// all on inspection -- its body is `bl __dt__10sStateID_cFv` then a
/// conditional `bl __dl__FPv`, i.e. it's sStateID_c's own scalar deleting
/// destructor, byte-identical in shape (a null check + one member-dtor call
/// + optional delete) to whatever weak dtor my draft has at that same
/// generic shape early in the file. Two different classes' deleting-dtor
/// wrappers collide byte-for-byte -- the same "two functions, one body"
/// trap check_vtable.py exists for, just hitting the anonymous-function
/// pairing in verify_anon.py instead of a vtable slot. My draft is missing
/// sStateID_c's OWN deleting destructor as a distinct emitted symbol
/// (likely triggered by the 9 static StateID_X objects' program-exit
/// teardown registration, which __sinit already matches byte-for-byte, so
/// whatever triggers this specific weak symbol is a step removed from
/// __sinit itself) -- not chased further this round; flagging rather than
/// guessing at the trigger site.
daWmSandPillar_c::~daWmSandPillar_c() {}

/// @unofficial fn_2_177AB0. createMdl()/calcMdl() calls and the clip-sphere
/// set confirmed by structure (matches every wm sibling's create()); the
/// 350.0f radius read directly out of original/d_basesNP.rel at
/// .rodata+0x8f7c (NOT a placeholder). mStartPos and the per-type height
/// table are read from the same evidence documented at their declarations.
int daWmSandPillar_c::create() {
    createMdl();
    mClipSphere.set(mPos, 350.0f);
    calcMdl();

    /// @unofficial Row3 of smc_TypeTable (see its declaration) -- indices
    /// 9,10,11 of the merged pool create() also draws 350.0f from. The
    /// ACTOR_PARAM name/bit-width it's indexed by is unconfirmed; `Type`/
    /// bits[0:8) is the simplest fit for the observed `(mParam & 0xff)`
    /// extraction with no shift beyond the array's own element size.
    mPos.y = smc_TypeTable.mHeightTable[ACTOR_PARAM(Type)];
    mStartPos = mPos;

    return SUCCEEDED;
}

/// @unofficial fn_2_177B40, rebuilt after the check_vtable.py pass. The
/// first call is NOT a vf7C-equivalent override -- it's a direct call
/// through mStateMgr's OWN vtable (`lwzu r12,0x4b0(r3)` loads mStateMgr's
/// vtable pointer, not this object's), landing on sStateMgrIf_c's
/// `executeState()` slot -- already confirmed correct indirectly:
/// fn_2_177C20 (that slot's real thunk, `sStateMgr_c<T,...>::executeState`)
/// already matched target bytes before this function was touched, so
/// calling `mStateMgr.executeState()` through the real declared type was
/// already right; nothing to guess. The two later virtual calls (vtable+0x14
/// on `mAnimTexSrt`, vtable+0x1c on `mModel`) resolve via the SAME
/// kinoko-family vtable-slot mapping already cross-checked on
/// daWmKinokoBase_c: banm_c's own slot5=play() for `mAnimTexSrt`,
/// scnLeaf_c's slot7=play() for `mModel`.
int daWmSandPillar_c::execute() {
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    mPos.y = smc_TypeTable.mHeightTable[ACTOR_PARAM(Type)];

    mStateMgr.executeState();

    if (csSeqMng->FUN_80915600()) {
        processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);
    }

    mAnimTexSrt.play();
    mModel.play();
    calcMdl();
    calcEffectPos();

    return SUCCEEDED;
}

/// @unofficial fn_2_177C30. Confirmed: mMatrix.trans(mPos)/ZXYrotM(mAngle),
/// mModel.setLocalMtx(&mMatrix)/setScale(mScale)/calc(false) -- the same
/// calcModel() idiom landed in d_a_wm_cloud.cpp, d_a_wm_dokan_route.cpp,
/// etc, just using mModel at +0x1ac instead of the leaf-actor offset.
void daWmSandPillar_c::calcMdl() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

/// @unofficial Confirmed target: fn_2_177CE0, `blr` only, slot 23 of the
/// vtable (verified in-unit this round).
void daWmSandPillar_c::finalUpdate() {}

/// @unofficial Placeholder body -- fn_2_177CF0 (draw) not yet analysed.
int daWmSandPillar_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

/// @unofficial fn_2_177D20. Archive/model names ("cobSandpillar",
/// "g3d/model.brres") read directly out of original/d_basesNP.rel's own
/// .data at 0x46c20/0x46c30 -- confirmed, not placeholders. The setAnm
/// blend-factor float (lbl_2_rodata_8f80) not yet extracted -- using 1.0f
/// pending that.
void daWmSandPillar_c::createMdl() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("cobSandpillar", "g3d/model.brres");
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl("cobSandpillar");
    /// @unofficial The 4-arg wrapper (mdl.hpp:52), NOT the 5-arg real
    /// function with an explicit nullptr -- confirmed on daWmKinokoBase_c
    /// that the wrapper's parameter binding anchors the by-value resMdl
    /// temporary in forward stack order, while spelling out the trailing
    /// nullptr leaves it in the reverse-order pool. Emitted call is
    /// byte-identical either way (same 5-arg mangled target, fully inlined
    /// at -O4) -- this is purely about which SOURCE call site the compiler
    /// binds the temporary through.
    mModel.create(resMdl, &mAllocator, 0x27, 1);

    nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr("cobSandpillar");
    mAnim.create(resMdl, resAnmChr, &mAllocator);

    mResAnmTexSrt = mResFile.GetResAnmTexSrt("cobSandpillar");
    mAnimTexSrt.create(resMdl, mResAnmTexSrt, &mAllocator, 1);
    mAnimTexSrt.setAnm(mModel, mResAnmTexSrt, 0, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnimTexSrt, 1.0f);

    mAllocator.adjustFrmHeap();
}

/// @unofficial The functions below are defined in target ADDRESS order, not
/// grouped by state -- the linker places .text in definition order, and
/// each state's init/exec/final trio is scattered across the address range,
/// interleaved with OTHER states' functions (confirmed by verify_anon.py's
/// "FUNCTION ORDER IS WRONG" diagnostic once these existed but were grouped
/// by state at the end of the file instead).

/// @unofficial fn_2_177E70. Confirmed byte-for-byte: classic approach/clamp.
void daWmSandPillar_c::approach() {
    if (mApproachCurrent < mApproachTarget) {
        mApproachCurrent += mApproachStep;
        if (!(mApproachCurrent > mApproachTarget)) {
            return;
        }
        mApproachCurrent = mApproachTarget;
    } else {
        if (!(mApproachCurrent > mApproachTarget)) {
            return;
        }
        mApproachCurrent -= mApproachStep;
        if (!(mApproachCurrent < mApproachTarget)) {
            return;
        }
        mApproachCurrent = mApproachTarget;
    }
}

/// @unofficial fn_2_177EC0. Sets mReadyFlag (+0x504) from ACTOR_PARAM(Type),
/// consumed by executeState_Ready below.
/// @unofficial Structurally right (both cmpwi are signed and correctly
/// ordered), but MWCC's own tail-sharing choice for the 2nd branch inverts
/// relative to target (`bne`-to-fail here vs target's `bne`-to-shared-tail)
/// no matter how this is phrased in source (tried ||, switch, goto, and a
/// bool intermediate -- all either range-merge into a subi/cmplwi/bgt or
/// invert this one branch). Parked at 3/12 differing; not chasing further
/// this round per the "don't chase a single-instruction residual" rule.
void daWmSandPillar_c::finalizeState_Ready() {
    int type = ACTOR_PARAM(Type);
    if (type == 1) {
        goto set;
    }
    if (type == 2) {
        goto set;
    }
    mReadyFlag = 0;
    return;
set:
    mReadyFlag = 1;
}

/// @unofficial fn_2_177EF0. Reads mReadyFlag (set by finalizeState_Ready),
/// and if set, looks up this pillar's own course via a fixed point name
/// ("W205", read directly out of original/d_basesNP.rel's .data at
/// 0x46c40) to pick the correct parked state: BottomWait if this is a fresh
/// clear, TopWaitFromTheStart if already fully cleared before,
/// BottomWaitForever if not cleared at all. If the flag is unset, goes
/// straight to BottomWait. State-ID targets (StateID_BottomWait/
/// TopWaitFromTheStart/BottomWaitForever) resolved from __sinit
/// (fn_2_178B30)'s own `.bss` layout: it constructs the 9 sStateID_c
/// sub-objects at lbl_2_bss_10040+{0x20,0x60,0xa0,0xe0,0x120,0x160,0x1a0,
/// 0x1e0,0x220} in STATE_DEFINE declaration order, and this function's own
/// changeState() call sites pass exactly those same offsets as arguments --
/// matching my own draft's compiled StateID_X layout (0x30-byte objects,
/// 0x40 stride) exactly.
void daWmSandPillar_c::executeState_Ready() {
    if (mReadyFlag != 0) {
        /// @unofficial `clrlwi r30,r3,24` truncates fn_2_171400()'s return
        /// value to 8 bits when saving it here -- the LOCAL is narrow (u8),
        /// not necessarily fn_2_171400() itself; its own declared return
        /// type is unconfirmed (see the extern "C" declaration's own note).
        u8 world = fn_2_171400();
        int course = dWmLib::GetCourseNoFromPointName("W205");
        if (dWmLib::IsCourseClear(world, course)) {
            if (dWmLib::IsCourseFirstClear(world, course)) {
                mStateMgr.changeState(StateID_BottomWait);
            } else {
                mStateMgr.changeState(StateID_TopWaitFromTheStart);
            }
        } else {
            mStateMgr.changeState(StateID_BottomWaitForever);
        }
    } else {
        mStateMgr.changeState(StateID_BottomWait);
    }
}

/// @unofficial fn_2_178000, confirmed `blr`.
void daWmSandPillar_c::initializeState_Ready() {}

/// @unofficial fn_2_178010. Resets mUnk4F0 (countdown) and mScale.y to
/// per-Type table values (the scale table is the SAME constant
/// executeState_MoveDown's target uses), replays the model's idle
/// animation, and clears mUnk4EC (a flag consumed by executeState_
/// BottomWait -- confirmed +0x4ec is zeroed here, per the earlier
/// gap-3 field note).
void daWmSandPillar_c::finalizeState_BottomWait() {
    mUnk4F0 = smc_TypeTable.mBottomWaitCounterTable[ACTOR_PARAM(Type)];
    mScale.y = smc_TypeTable.mScaleTable[ACTOR_PARAM(Type)];

    nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr("cobSandpillar");
    mAnim.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
    mAnim.setRate(1.0f);
    mModel.setAnm(mAnim, 1.0f);

    mUnk4EC = 0;
}

/// @unofficial fn_2_1780C0. If mUnk4EC is set, does nothing (parked
/// permanently -- BottomWaitForever's own final also sets this, presumably).
/// Otherwise counts mUnk4F0 down to 0 and transitions to MoveReady.
void daWmSandPillar_c::executeState_BottomWait() {
    if (mUnk4EC != 0) {
        return;
    }
    if (--mUnk4F0 > 0) {
        return;
    }
    mStateMgr.changeState(StateID_MoveReady);
}

/// @unofficial fn_2_1780F4, confirmed `blr`. Not a state method -- still
/// unidentified (not one of the 9 states' triples).
void daWmSandPillar_c::unk1780F4() {}

/// @unofficial fn_2_178100, confirmed `blr`.
void daWmSandPillar_c::initializeState_BottomWait() {}

/// @unofficial fn_2_178110. Confirmed by direct instruction read: same
/// shape as finalizeState_BottomWait (mScale.y from the SAME row4 table,
/// same setAnm/setRate/setAnm idle-animation reset) minus the mUnk4F0
/// countdown reset -- the Forever states never leave, so there's no counter
/// to arm.
void daWmSandPillar_c::finalizeState_BottomWaitForever() {
    mScale.y = smc_TypeTable.mScaleTable[ACTOR_PARAM(Type)];

    nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr("cobSandpillar");
    mAnim.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
    mAnim.setRate(1.0f);
    mModel.setAnm(mAnim, 1.0f);

    mUnk4EC = 0;
}

/// @unofficial fn_2_1781B0, confirmed `blr`.
void daWmSandPillar_c::executeState_BottomWaitForever() {}

/// @unofficial fn_2_1781C0, confirmed `blr`.
void daWmSandPillar_c::initializeState_BottomWaitForever() {}

/// @unofficial fn_2_1781D0. Confirmed by direct instruction read: reads
/// smc_TypeTable rows 0,1,2,4,8 (all at the SAME ACTOR_PARAM(Type) column,
/// via one shared `&smc_TypeTable + Type*4` pointer) and sets up the
/// movement approach for going UP (mApproachCurrent starts negative,
/// mApproachTarget negative -- i.e. counting up from a negative offset
/// toward 0, matching mScale.y growing during MoveUp). mAnim.setRate(0.0f)
/// pauses the animation. The 0.0f literal is read out of the SAME external
/// rodata object executeState_MoveReady also reads (lbl_2_rodata_8F84,
/// offset +0x0) -- not yet reconstructed as a source-level object, so
/// written here as a plain literal pending that (see check_sections.py
/// .rodata UNDER 0x14 finding).
void daWmSandPillar_c::finalizeState_MoveReady() {
    mUnk4FC = smc_TypeTable.mUnk8[ACTOR_PARAM(Type)];
    mApproachCurrent = -smc_TypeTable.mUnk0[ACTOR_PARAM(Type)];
    mScale.y = smc_TypeTable.mScaleTable[ACTOR_PARAM(Type)];
    mApproachStep = smc_TypeTable.mUnk1[ACTOR_PARAM(Type)];
    mApproachTarget = -smc_TypeTable.mUnk2[ACTOR_PARAM(Type)];
    mUnk4EC = 1;
    mAnim.setRate(0.0f);
}

/// @unofficial fn_2_178230 (0x180 bytes, the largest state method).
/// Confirmed by direct instruction read. Early-return unless mUnk4EC == 1
/// (the flag finalizeState_MoveReady just set). Drives mScale.y via
/// approach() same as MoveUp/MoveDown; on landing exactly on 0
/// (mApproachCurrent == 0.0f, i.e. approach() reached its target this
/// frame) re-aims mApproachTarget for the return trip based on which side
/// of 0 it was previously approaching from. Two threshold crossings (row7 =
/// the SAME MoveUp-threshold table, and row6, both read ONCE up front via a
/// shared pointer before the early-return check -- matches the "hoist
/// singleton loads" lever) each re-zero mApproachTarget on the frame
/// mScale.y crosses them; the row7 crossing also decrements mUnk4FC.
/// Transitions to TopWait once mUnk4FC reaches <= 0.
void daWmSandPillar_c::executeState_MoveReady() {
    float rowUnk6 = smc_TypeTable.mUnk6[ACTOR_PARAM(Type)];
    float rowThreshold = smc_TypeTable.mMoveUpThresholdTable[ACTOR_PARAM(Type)];
    if (mUnk4EC != 1) {
        return;
    }

    float oldScale = mScale.y;
    float oldApproachCurrent = mApproachCurrent;
    approach();

    mScale.y = mScale.y + mApproachCurrent;

    if (mApproachCurrent == 0.0f) {
        if (oldApproachCurrent > 0.0f) {
            mApproachTarget = -smc_TypeTable.mUnk2[ACTOR_PARAM(Type)];
        } else if (oldApproachCurrent < 0.0f) {
            mApproachTarget = smc_TypeTable.mUnk2[ACTOR_PARAM(Type)];
        }
    }

    if (oldScale < rowThreshold && mScale.y >= rowThreshold) {
        mApproachTarget = 0.0f;
        mUnk4FC--;
    } else if (oldScale > rowUnk6 && mScale.y <= rowUnk6) {
        mApproachTarget = 0.0f;
    }

    if (mUnk4FC > 0) {
        return;
    }
    mStateMgr.changeState(StateID_TopWait);
}

/// @unofficial fn_2_1783B0, confirmed `blr`.
void daWmSandPillar_c::initializeState_MoveReady() {}

/// @unofficial fn_2_1783C0. Confirmed by direct instruction read: sets up
/// approach() to drive mScale.y from the CURRENT (post-MoveReady) value up
/// toward 0 -- mApproachTarget/mApproachStep both read from row2/row1 (the
/// SAME two rows finalizeState_MoveReady uses), negating the target only
/// when mReadyFlag is set (the "already fully cleared, ramping up again"
/// case MoveUp's own body branches on). Both stores repeat unconditionally
/// before the branch and then again (mApproachStep redundantly) inside it --
/// -O4 keeps the loaded values live in f0/f1 across the branch with no
/// intervening call, so the repeated array-index expressions cost nothing
/// extra; this is not a hand-placed local-variable optimisation.
void daWmSandPillar_c::finalizeState_MoveUp() {
    mApproachTarget = smc_TypeTable.mUnk2[ACTOR_PARAM(Type)];
    mApproachStep = smc_TypeTable.mUnk1[ACTOR_PARAM(Type)];
    if (mReadyFlag != 0) {
        mApproachStep = smc_TypeTable.mUnk1[ACTOR_PARAM(Type)];
        mApproachTarget = -smc_TypeTable.mUnk2[ACTOR_PARAM(Type)];
    }
    mAnim.setRate(0.0f);
}

/// @unofficial fn_2_178410. mScale.y (+0xe0 -- confirmed via calcMdl's
/// already-matched setScale(mScale) call at +0xdc) is the pillar's own
/// vertical extrusion: it animates by SCALING, not translating. Grows by
/// mApproachCurrent (driven by approach()) each frame; once it reaches the
/// per-Type target height, clamps and transitions to TopWait (or
/// TopWaitForever if mReadyFlag is set and still ramping up).
void daWmSandPillar_c::executeState_MoveUp() {
    float target = smc_TypeTable.mMoveUpTargetTable[ACTOR_PARAM(Type)];
    float threshold = smc_TypeTable.mMoveUpThresholdTable[ACTOR_PARAM(Type)];

    approach();

    mScale.y = mScale.y + mApproachCurrent;

    if (mReadyFlag != 0) {
        if (mApproachCurrent <= 0.0f) {
            if (mScale.y <= target) {
                mScale.y = target;
                mStateMgr.changeState(StateID_TopWaitForever);
            }
        }
    } else {
        if (target >= threshold) {
            if (mScale.y >= target) {
                mScale.y = target;
                mStateMgr.changeState(StateID_TopWait);
            }
        } else {
            if (mScale.y <= target) {
                mScale.y = target;
                mStateMgr.changeState(StateID_TopWait);
            }
        }
    }
}

/// @unofficial fn_2_178540, confirmed `blr`.
void daWmSandPillar_c::initializeState_MoveUp() {}

/// @unofficial fn_2_178550. Resets mAnim's playback rate to 1.0 and
/// mUnk4F0 (countdown) to a per-Type table value (100 for all 3 types,
/// same rodata pool at +0x78).
void daWmSandPillar_c::finalizeState_TopWait() {
    mAnim.setRate(1.0f);

    mUnk4F0 = smc_TypeTable.mTopWaitCounterTable[ACTOR_PARAM(Type)];
}

/// @unofficial fn_2_1785B0. Mirrors executeState_BottomWait's countdown
/// (no mUnk4EC gate here though), transitioning to MoveDown once expired.
void daWmSandPillar_c::executeState_TopWait() {
    if (--mUnk4F0 > 0) {
        return;
    }
    mStateMgr.changeState(StateID_MoveDown);
}

/// @unofficial fn_2_1785D8, confirmed `blr`. Not a state method -- still
/// unidentified.
void daWmSandPillar_c::unk1785D8() {}

/// @unofficial fn_2_1785E0, confirmed `blr`.
void daWmSandPillar_c::initializeState_TopWait() {}

/// @unofficial fn_2_1785F0. Confirmed by direct instruction read: same
/// setRate(1.0f)/counter-reset shape as finalizeState_TopWait, plus an
/// mApproachCurrent reset to 0.0f. The 1.0f and 0.0f loads both resolve as
/// small positive DISPLACEMENTS off the already-live smc_TypeTable base
/// register (+0x88/+0x8c) rather than fresh `lis` loads -- purely a pool
/// placement coincidence (those two constants land adjacent to
/// smc_TypeTable in .rodata) which will only reproduce once the .rodata
/// layout gap this unit still has (see smc_TypeTable's own comment) is
/// closed; the SOURCE is the same plain literals either way.
void daWmSandPillar_c::finalizeState_TopWaitForever() {
    mAnim.setRate(1.0f);
    mApproachCurrent = 0.0f;
    mUnk4F0 = smc_TypeTable.mTopWaitCounterTable[ACTOR_PARAM(Type)];
}

/// @unofficial fn_2_178660, confirmed `blr`.
void daWmSandPillar_c::executeState_TopWaitForever() {}

/// @unofficial fn_2_178670, confirmed `blr`.
void daWmSandPillar_c::initializeState_TopWaitForever() {}

/// @unofficial fn_2_178680. Confirmed by direct instruction read: mirror of
/// finalizeState_MoveUp's approach() setup for the DOWN direction --
/// mApproachCurrent resets to 0.0f (rather than -row0[Type]) and
/// mApproachTarget is always -row2[Type] unconditionally (no mReadyFlag
/// branch, since MoveDown only ever runs one way).
void daWmSandPillar_c::finalizeState_MoveDown() {
    mAnim.setRate(0.0f);
    mApproachCurrent = 0.0f;
    mApproachTarget = -smc_TypeTable.mUnk2[ACTOR_PARAM(Type)];
}

/// @unofficial fn_2_1786F0. Much simpler mirror of executeState_MoveUp:
/// mScale.y shrinks by mApproachCurrent each frame, clamping to the
/// per-Type target and transitioning to BottomWait once reached. Table
/// value (0.5/0.2/0.2) is the SAME constant finalizeState_BottomWait reads
/// (rodata_8EF8+0x30) -- written identically here so -ipa file's pool
/// merge has a chance to unify them once both are present.
void daWmSandPillar_c::executeState_MoveDown() {
    float target = smc_TypeTable.mScaleTable[ACTOR_PARAM(Type)];

    approach();

    float newScaleY = mScale.y + mApproachCurrent;
    mScale.y = newScaleY;

    if (newScaleY <= target) {
        mScale.y = target;
        mStateMgr.changeState(StateID_BottomWait);
    }
}

/// @unofficial fn_2_178780, confirmed `blr`.
void daWmSandPillar_c::initializeState_MoveDown() {}

/// @unofficial fn_2_178790. Confirmed by direct instruction read: same
/// GetResAnmChr/setAnm/setRate(1.0f)/setAnm idle-animation reset as
/// finalizeState_BottomWaitForever, but sets mScale.y from row5 -- the SAME
/// table executeState_MoveUp's target comes from -- rather than row4, and
/// does not touch mUnk4EC. Consistent with "already fully cleared": start
/// already at full extrusion height, no counter to arm.
void daWmSandPillar_c::finalizeState_TopWaitFromTheStart() {
    nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr("cobSandpillar");
    mAnim.setAnm(mModel, resAnmChr, m3d::FORWARD_LOOP);
    mAnim.setRate(1.0f);
    mModel.setAnm(mAnim, 1.0f);

    mScale.y = smc_TypeTable.mMoveUpTargetTable[ACTOR_PARAM(Type)];
}

/// @unofficial fn_2_178830, confirmed `blr`... actually a tail-call: the
/// function's ONLY statement is mStateMgr.changeState(), so MWCC emits it
/// as `bctr` (tail call) rather than `bctrl`+`blr`. lbl_2_bss_10220 is
/// mStateMgr's own StateID_TopWaitFromTheStart sub-object (+0x220 in the
/// bss array, matching the STATE_DEFINE layout __sinit already confirmed).
void daWmSandPillar_c::executeState_TopWaitFromTheStart() {
    mStateMgr.changeState(StateID_TopWaitFromTheStart);
}

/// @unofficial fn_2_178850, confirmed `blr`.
void daWmSandPillar_c::initializeState_TopWaitFromTheStart() {}

/// @unofficial fn_2_178860, confirmed this round (see check_vtable.py
/// finding above).
void daWmSandPillar_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId != dCsSeqMng_c::CUTSCENE_CMD_NONE && !isStaff()) {
        mIsCutEnd = true;
    }
}

/// @unofficial fn_2_1788B0. Node name ("ef_cobSandpillar") and the two
/// effect names ("Wm_cs_sandpillar01"/"02") read directly out of
/// original/d_basesNP.rel's own .data (0x46c48/0x46c5c/0x46c70) -- confirmed,
/// not placeholders. The two virtual calls (vtable+0xa0 on mEffect1/
/// mEffect2) are `createEffect(const char*, ulong, const mVec3_c*,
/// const mAng3_c*, const mVec3_c*)` by argument count (5 args set:
/// name, 0, &localPos, 0, 0) -- matches mEf::effect_c's own declared
/// overload with that exact arity.
void daWmSandPillar_c::calcEffectPos() {
    nw4r::math::VEC3 localPos;
    int nodeId = m3d::getNodeID(mModel.getResMdl(), "ef_cobSandpillar");
    mModel.getNodeWorldMtxMultVecZero(nodeId, localPos);

    mEffect1.createEffect("Wm_cs_sandpillar01", 0, (const mVec3_c *)&localPos, nullptr, nullptr);
    mEffect2.createEffect("Wm_cs_sandpillar02", 0, (const mVec3_c *)&localPos, nullptr, nullptr);
}
