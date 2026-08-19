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
    void resetState();       ///< @unofficial fn_2_16D7B0. Sets mUnk35c = -1, tail-calls resetScaleAndProc().
    void resetScaleAndProc(); ///< @unofficial fn_2_16D7C0. mProcState = 0; mScale = (0.01, 0.01, 0.01).
    void startAction(int type); ///< @unofficial fn_2_16D7F0. Looks up sc_actionTable[type] into mUnk340, then mProcState = 1.
    void lookupAction(int type); ///< @unofficial fn_2_16D920. mUnk340 = sc_actionTable[type].

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

    int mUnk338;      ///< @unofficial +0x338. Not touched by ctor/dtor; role unknown. Confirmed to
                       ///< exist (not padding folded into mProcState) because execute()'s target
                       ///< reads mProcState from +0x33c, not +0x338.
    int mProcState;   ///< @unofficial +0x33c. Index into sc_procTable, consumed by execute().
    int mUnk340;      ///< @unofficial +0x340. Set by lookupAction() from sc_actionTable.
    u8 pad344[0x18];   ///< @unofficial +0x344. Confirmed present (not a hand-count): resetState()'s
                        ///< target stores to +0x35c, 0x18 bytes after mUnk340. Role/subfields unknown.
    int mUnk35c;        ///< @unofficial +0x35c. Set to -1 by resetState(); role otherwise unknown.
};
// sizeof(daWmKoopaJr_c) == 0x360, matching fn_2_16D290's `li r3, 0x360`.

ACTOR_PROFILE(WM_KOOPAJR, daWmKoopaJr_c, 0);

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

bool daWmKoopaJr_c::runMain() {
    /// @unofficial STUB. fn_2_16D940 (0xA60) is not authored -- see the class
    /// declaration comment. This body exists only so procMain()/execute()
    /// compile; it is not expected to match and is not one of the six
    /// required functions.
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
    /// @unofficial The 4-entry lookup table (lbl_2_rodata_8C38) has not been
    /// dumped/named -- see MAPPING.md. This is a placeholder that will not
    /// match; lookupAction() and startAction() are not required for the six
    /// named functions, only reachable through them.
    static const int sc_actionTable[4] = {0, 0, 0, 0};
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

    static const char *const sc_animNames[6] = {
        "wait", "run", "jump_st", "jumpA", "jump_ed", "shock_wmap"
    };
    static const m3d::playMode_e sc_playModes[6] = {
        m3d::FORWARD_LOOP, m3d::FORWARD_LOOP,
        m3d::FORWARD_ONCE, m3d::FORWARD_ONCE, m3d::FORWARD_ONCE, m3d::FORWARD_ONCE
    };

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
