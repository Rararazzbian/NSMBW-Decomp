#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_a_wm_player.hpp>

// @unofficial Cross-unit call into an un-landed function elsewhere in this same REL
// (d_basesNP), NOT this unit's own code -- bin/dtk/d_basesNP_symbols.txt:
// fn_2_1709B0 = .text:0x001709B0, size 0x74. Argument role inferred only from the
// single call site (runMain() case 14 passes literal 0; return value unused).
extern "C" void R_2_1_1709B0(int);

/// @unofficial Provisional reconstruction of the world map Koopa Jr. actor.
/// `daWmKoopaJr_c : public dWmDemoActor_c`, sizeof 0x360 (confirmed directly
/// from the target's own `li r3, 0x360` at fn_2_16D290, the profile's
/// allocate+construct trampoline).
///
/// Member layout confirmed directly from the constructor (fn_2_16D2C0) and
/// destructor (fn_2_16D340) disassembly -- every offset below is read off a
/// `bl __ct__`/`bl __dt__` call or a raw store, not counted by hand:
///   +0x184  int mUnk184        -- raw store, NOT touched by this ctor (family convention)
///   +0x188  dHeapAllocator_c mAllocator     (__ct__16dHeapAllocator_cFv at +0x188)
///   +0x1a4  nw4r::g3d::ResFile mResFile     (see note below)
///   +0x1a8  m3d::mdl_c mModel              (__ct__Q23m3d5mdl_cFv at +0x1a8)
///   +0x1e8  m3d::anmChr_c mAnimChrs[6]     (__construct_array, elem 0x38, count 6)
/// +0x1e8 + 6*0x38 = 0x338; the remaining 0x28 bytes up to sizeof (0x360) are
/// untouched by both the ctor and the dtor (no `bl __ct__`/`__dt__` reaches
/// past 0x338), so they hold only POD fields. Two are pinned by other
/// functions in this unit: +0x33c (an int state index, read/written by
/// execute()'s PTMF dispatch and by fn_2_16D7F0/fn_2_16D7C0) and +0x35c (an
/// int/timer set to -1 by fn_2_16D7B0). The rest is undetermined and left as
/// padding.
///
/// `mResFile` at +0x1a4: the ORIGINAL scouting pass recorded this as a raw
/// `int (= 0)` because the constructor stores it with a plain
/// `li r0,0; stw r0,0x1a4(r31)` -- no `bl __ct__...` visible. But
/// `nw4r::g3d::ResFile`'s default constructor is a trivial one-liner defined
/// IN the class body (vague linkage), and `-inline noauto` still inlines an
/// in-class body (see AGENT_CONTEXT.md), so a genuine
/// `nw4r::g3d::ResFile mResFile;` member compiles to exactly this same
/// `li 0; stw` pattern -- indistinguishable from a raw `int`. The type is
/// confirmed by `createModel()` (fn_2_16D590), which stores
/// `dResMng_c::m_instance->getRes(...)`'s return value directly into +0x1a4
/// via a plain `stw` (no copy-ctor call, consistent with ResFile's trivial
/// representation), then calls `GetResMdl__Q34nw4r3g3d7ResFileCFPCc` with
/// `this = &mResFile` -- exactly the same shape as `daWmKinokoBase_c::mResFile`
/// at the SAME offset (+0x1a4) in the landed sibling.
///
/// `mMatrix`@0x7c, `mPos`@0xac, `mScale`@0xdc, `mAngle`@0x100 and
/// `mClipSphere`@0x128 (all inherited from dBaseActor_c/dWmActor_c) are
/// confirmed directly, not hand-counted: `calcModel()` (fn_2_16D700) passes
/// `&mMatrix` to `PSMTXTrans`/`ZXYrotM`/`setLocalMtx` at this+0x7c, `mPos` at
/// this+0xac/0xb0/0xb4 (three lfs), `mScale` at this+0xdc to `setScale`, and
/// `mAngle` at this+0x100/0x102/0x104 (three lha) to `ZXYrotM`. `mClipSphere`
/// at this+0x128 (mCenter) / this+0x134 (mRadius) is independently confirmed
/// by BOTH koopajr's own create() (`mClipSphere.set(mPos, 250.0f)`, the
/// 250.0f read directly out of the REL's .rodata at 0x8c0c) and by the
/// ALREADY-LANDED `dWmActor_c::preExecute()`/`preDraw()`
/// (`mClipSphere.mCenter = mPos;`), which compiles to the byte-identical
/// `lfs 0xac/0xb0/0xb4; stfs 0x128/0x12c/0x130` sequence -- two independent
/// call sites landing on the same four offsets.
class daWmKoopaJr_c : public dWmDemoActor_c {
public:
    daWmKoopaJr_c();
    virtual ~daWmKoopaJr_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    void createModel();
    void calcModel();
    void changeAnim(int animIdx, float blendFrame, float rate, float startFrame); ///< @unofficial fn_2_16E3A0. Skips if mCurAnimIdx == animIdx (dirty check); else GetResAnmChr, mAnimChrs[i].setAnm(mModel, resAnmChr, sc_playModes[i]), mModel.setAnm(mAnimChrs[i], blendFrame), setRate(rate), setFrame(startFrame); then mCurAnimIdx = animIdx.

    void resetState();       ///< @unofficial fn_2_16D7B0. Sets mUnk35c = -1, tail-calls resetScaleAndProc().
    void resetScaleAndProc(); ///< @unofficial fn_2_16D7C0. mProcState = 0; mScale = (0.01, 0.01, 0.01).
    void startAction(int type); ///< @unofficial fn_2_16D7F0. Looks up sc_actionTable[type] into mUnk340, then mProcState = 1.
    void lookupAction(int type); ///< @unofficial fn_2_16D920. mUnk340 = sc_actionTable[type].

    /// @unofficial Shared between createModel() and changeAnim(): both reference the SAME
    /// symbols (lbl_2_data_45E50 for the name pointers, lbl_2_rodata_8BA0-relative for the
    /// play modes), not two independent per-function anonymous copies -- confirmed by
    /// changeAnim() indexing `lbl_2_rodata_8BA0 + idx*4` directly, the identical base symbol
    /// execute()/create() also use.
    static const char *const sc_animNames[6];
    static const m3d::playMode_e sc_playModes[6];

    void procNone() {} ///< @unofficial fn_2_16D7E0. The idle/no-op state-0 handler.
    void procMain();   ///< @unofficial fn_2_16D830. The state-1 handler.

    bool runMain(); ///< @unofficial fn_2_16D940. NOT authored -- the unit's largest function
                     ///< by far (0xA60), confirmed by vtable elimination to be a plain
                     ///< non-virtual member, not one of the six named overrides. Stubbed
                     ///< to keep procMain()/execute() compiling; body is a placeholder.

    int mUnk184;                 ///< @unofficial +0x184. Not written by this ctor.
    dHeapAllocator_c mAllocator;  ///< @unofficial +0x188
    nw4r::g3d::ResFile mResFile;  ///< @unofficial +0x1a4
    m3d::mdl_c mModel;             ///< @unofficial +0x1a8
    m3d::anmChr_c mAnimChrs[6];    ///< @unofficial +0x1e8

    int mUnk338;       ///< @unofficial +0x338. Not touched by ctor/dtor; role unknown. Confirmed to
                        ///< exist (not padding folded into mProcState) because execute()'s target
                        ///< reads mProcState from +0x33c, not +0x338.
    int mProcState;    ///< @unofficial +0x33c. Index into sc_procTable, consumed by execute().
    int mUnk340;       ///< @unofficial +0x340. Set by lookupAction() from sc_actionTable; read by
                        ///< runMain()'s outer jump table (16 cases, `cmplwi r0,0xf`).
    mVec3_c mJumpTargetPos; ///< @unofficial +0x344. Confirmed a 3-float mVec3_c, not a raw pad: runMain()
                        ///< case 0 stores `daWmMap_c::GetPos(...)`'s x/y/z result here via three
                        ///< `stfs` (0x344/0x348/0x34c), then passes it straight to `_initDemoJumpBase`.
    int mJumpTimer;     ///< @unofficial +0x350. A frame counter: runMain() case 0 sets it to 0x1e (30),
                        ///< case 1 decrements it each frame (`subic. r0,r0,1`) and branches on hitting 0.
    int mCurAnimIdx;    ///< @unofficial +0x354. changeAnim()'s dirty-check/cache of the last-set animation
                        ///< index; skips all work when the new index already matches.
    u8 pad358[0x4];      ///< @unofficial +0x358. Untouched by every function authored so far; role unknown.
    int mUnk35c;         ///< @unofficial +0x35c. Set to -1 by resetState(). runMain() case 2 also stores
                        ///< `fn_80103520(...)`'s return value here (an effect handle/ID, same
                        ///< cross-module call already seen in the landed `d_a_wm_kinoko_base.cpp`).
};
// sizeof(daWmKoopaJr_c) == 0x360, matching fn_2_16D290's `li r3, 0x360`.

ACTOR_PROFILE(WM_KOOPAJR, daWmKoopaJr_c, 0);

const char *const daWmKoopaJr_c::sc_animNames[6] = {
    "wait", "run", "jump_st", "jumpA", "jump_ed", "shock_wmap"
};
const m3d::playMode_e daWmKoopaJr_c::sc_playModes[6] = {
    m3d::FORWARD_LOOP, m3d::FORWARD_LOOP,
    m3d::FORWARD_ONCE, m3d::FORWARD_ONCE, m3d::FORWARD_ONCE, m3d::FORWARD_ONCE
};

daWmKoopaJr_c::daWmKoopaJr_c() {}

daWmKoopaJr_c::~daWmKoopaJr_c() {}

int daWmKoopaJr_c::doDelete() {
    return SUCCEEDED;
}

int daWmKoopaJr_c::draw() {
    mModel.entry();
    DrawShadow(true);
    return SUCCEEDED;
}

void daWmKoopaJr_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daWmKoopaJr_c::resetScaleAndProc() {
    mProcState = 0;
    mScale.x = 0.01f;
    mScale.y = 0.01f;
    mScale.z = 0.01f;
}

void daWmKoopaJr_c::resetState() {
    mUnk35c = -1;
    resetScaleAndProc();
}

int daWmKoopaJr_c::create() {
    createModel();
    mClipSphere.set(mPos, 250.0f);
    calcModel();
    resetState();
    return SUCCEEDED;
}

void daWmKoopaJr_c::procMain() {
    if (runMain()) {
        resetScaleAndProc();
    }
}

void daWmKoopaJr_c::changeAnim(int animIdx, float blendFrame, float rate, float startFrame) {
    if (mCurAnimIdx == animIdx) {
        return;
    }

    nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr(sc_animNames[animIdx]);
    mAnimChrs[animIdx].setAnm(mModel, resAnmChr, sc_playModes[animIdx]);
    mModel.setAnm(mAnimChrs[animIdx], blendFrame);
    mAnimChrs[animIdx].setRate(rate);
    mAnimChrs[animIdx].setFrame(startFrame);
    mCurAnimIdx = animIdx;
}

bool daWmKoopaJr_c::runMain() {
    /// @unofficial PARTIAL. fn_2_16D940 (0xA60) is a 16-case state machine dispatched
    /// through `jumptable_2_data_45EC4` on mUnk340 (`cmplwi r0,0xf; bgt <epilogue>`),
    /// implementing Bowser Jr.'s appear/land/run/disappear cutscene sequence. Cases 0,
    /// 14 and 15 are authored (each verified against the disassembly
    /// instruction-by-instruction); case 15 is the ONLY path that returns true, via
    /// the same `this->vtable+0x60 -> +0x68` dispatch as processCutsceneCommand()'s
    /// case 0x45, i.e. `setCutEnd()`. Cases 1-13 are explicit stubs -- NOT attempted,
    /// not guessed. See MAPPING.md for the case-target address table and what each
    /// stubbed case's first few instructions show, for the next round.
    switch (mUnk340) {
    case 0: {
        mVec3_c targetPos = daWmMap_c::m_instance->GetPos(daWmPlayer_c::ms_instance->m_22c);
        mJumpTargetPos = targetPos;
        changeAnim(3, 5.0f, 1.0f, 0.0f);
        _initDemoJumpBase(mJumpTargetPos, 0, 0x3c, 10.0f, 0.2f * 2.5f, 1.0f * 2.5f, mVec3_c::Ey);
        mUnk340 = 1;
        mJumpTimer = 0x1e;
        break;
    }

    /// @unofficial Cases 1-14 NOT authored. Confirmed target addresses (from
    /// `jumptable_2_data_45EC4`, unit .text-relative): 1=0x16DA3C 2=0x16DAD8
    /// 3=0x16DB68 4=0x16DCDC 5=0x16DDA8 6=0x16DDD8 7=0x16DE8C 8=0x16DF10
    /// 9=0x16DFA8 10=0x16E0EC 11=0x16E140 12=0x16E220 13=0x16E240 14=0x16E340.
    case 1: case 2: case 3: case 4: case 5: case 6: case 7:
    case 8: case 9: case 10: case 11: case 12: case 13:
        break;

    case 14: {
        if (_procDemoJumpBase()) {
            R_2_1_1709B0(0);
            mUnk340 = 15;
        }
        break;
    }

    case 15: {
        setCutEnd();
        return true;
    }

    default:
        break;
    }

    return false;
}

int daWmKoopaJr_c::execute() {
    static void (daWmKoopaJr_c::*const sc_procTable[2])() = {
        &daWmKoopaJr_c::procNone,
        &daWmKoopaJr_c::procMain,
    };

    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    if (csSeqMng->FUN_80915600()) {
        processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);
    }

    (this->*sc_procTable[mProcState])();

    mModel.play();
    CalcShadow(0.5f, 1.0f, 1.0f, 1.0f);
    calcModel();

    return SUCCEEDED;
}

void daWmKoopaJr_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == dCsSeqMng_c::CUTSCENE_CMD_NONE) {
        return;
    }

    if (isFirstFrame) {
        switch (cutsceneCommandId) {
        case 0x43:
            startAction(0);
            break;

        case 0x45:
            setCutEnd();
            break;

        case 0x46:
            startAction(2);
            break;

        case 0x44:
            startAction(3);
            break;

        default:
            break;
        }
    }

    if ((u32)(cutsceneCommandId - 0x43) > 3) {
        mIsCutEnd = true;
    }
}

void daWmKoopaJr_c::lookupAction(int type) {
    /// @unofficial The lookup table's CONTENT is recovered directly from the REL
    /// (.rodata file offset 0x1C6600+0x8C3C, right after lbl_2_rodata_8C38's leading
    /// 0 word): {0, 5, 7, 11}. These plausibly index runMain()'s case dispatch
    /// (mUnk340), i.e. startAction()'s four cutscene-triggered entry points map to
    /// runMain() cases 0, 5, 7 and 11 -- not yet cross-checked against the case
    /// bodies themselves.
    static const int sc_actionTable[4] = {0, 5, 7, 11};
    mUnk340 = sc_actionTable[type];
}

void daWmKoopaJr_c::startAction(int type) {
    lookupAction(type);
    mProcState = 1;
}

void daWmKoopaJr_c::createModel() {
    /// @unofficial NOT verified byte-exact. Logic and every string/constant
    /// below are read directly out of the REL: the resource table at
    /// lbl_2_data_45DD8 (.data file offset 0x1D0C00+0x45DD8) and the shared
    /// float pool at lbl_2_rodata_8BA0 (.rodata file offset
    /// 0x1C6600+0x8BA0). What remains unverified is ORDER: the shared rodata
    /// pool (0x8ba0-0x8c90) holds constants used not just here but by
    /// execute(), create(), AND several values that belong to NEITHER --
    /// e.g. 0x8bc4/0x8bd4/0x8bd8/0x8be0/0x8bec, which never appear in this
    /// function's own disassembly and must belong to fn_2_16D940 or
    /// fn_2_16E3A0 (both unauthored). Until those are written, this
    /// function's rodata objects cannot land at the retail addresses, which
    /// is why create()/execute() still show `SYM0`-vs-`lbl_2_rodata_8C0C`
    /// style diffs even though their own logic matches. See MAPPING.md.
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("koopaJr", "g3d/koopaJr.brres");
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl("koopaJr");

    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1, nullptr);

    for (int i = 0; i < 6; i++) {
        nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr(sc_animNames[i]);
        mAnimChrs[i].create(resMdl, resAnmChr, &mAllocator, nullptr);
        mAnimChrs[i].mPlayMode = sc_playModes[i];
        mAnimChrs[i].setRate(0.0f);
        mAnimChrs[i].setFrame(0.0f);
    }

    /// @unofficial `GetResNode("mask")`'s result has bit 0x200 cleared out of
    /// its flags word (`rlwinm r0,r0,0,24,22` in the target) when the node
    /// exists. Not modelled here -- the exact API for mutating a ResNode's
    /// flags in-place hasn't been located in include/, so this is left as a
    /// gap rather than guessed.

    dWmActor_c::setSoftLight_Boss(mModel);
    mAllocator.adjustFrmHeap();

    CreateShadowModel("character_SV", "g3d/model.brres", "character_SV", true);
}
