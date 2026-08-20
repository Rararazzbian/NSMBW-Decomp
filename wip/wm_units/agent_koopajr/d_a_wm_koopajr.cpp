#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_a_wm_player.hpp>
#include <game/bases/d_wm_se_manager.hpp>
#include <game/bases/d_wm_effect_manager.hpp>

// @unofficial Cross-unit calls into un-landed functions elsewhere in this same REL
// (d_basesNP), NOT this unit's own code -- bin/dtk/d_basesNP_symbols.txt:
// fn_2_1709B0 = .text:0x001709B0 size 0x74; fn_2_15F8F0 = .text:0x0015F8F0 size 0x54.
extern "C" void R_2_1_1709B0(int);
extern "C" void R_2_1_15F8F0(void *, float, float);

// @unofficial Cross-module call into the DOL, same shape as the already-landed
// d_a_wm_kinoko_base.cpp's fn_80103420 (mgr, effectId, model, kindStr, int, int) but a
// DIFFERENT DOL address -- bin/dtk/wiimj2d_symbols.txt: fn_80103520 = .text:0x80103520
// size 0x10 (a thin trampoline, unlike fn_80103420's 0x74). Return value IS consumed
// here (stored into mUnk35c), unlike fn_80103420's callers which ignore it.
extern "C" int fn_80103520(dWmEffectManager_c *mgr, int effectId, m3d::mdl_c *model,
                            const char *kind, int, int);

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
    static const char *sc_animNames[6];
    static const m3d::playMode_e sc_playModes[6];

    /// @unofficial Read directly out of the REL: .rodata file offset 0x1C6600+0x8BB8
    /// through +0x8BCC, immediately after sc_playModes (which ends at +0x8BB8) with
    /// NO gap. Modelled as an array-of-structs, NOT plain scalar class statics --
    /// tested both ways. Plain `static const float` members got folded by the
    /// compiler at case 0's use site (`0.2f * 2.5f` collapses to a fresh anonymous
    /// 0.5f, and even the untouched `sc_jumpSpeed` got copied into a fresh
    /// anonymous 10.0f instead of being referenced), which does NOT match the
    /// target: the target computes `startScale`/`targetScale` with a RUNTIME
    /// `fmuls`, reading four values through a pointer computed ONCE
    /// (`r5 = &pool[0x18]`) and then offset from it (`r5[0x8]`, `r5[0x10]`,
    /// `r5[0x14]`) -- the classic shape of `const JumpParams_t &p = table[idx];`
    /// followed by `p.field` accesses, not scalar named constants.
    struct JumpParams_t {
        float scaleMul;   ///< +0x0 (pool +0x18/+0x30) = 2.5 / 6.8
        u32 unk1c;           ///< +0x4 (pool +0x1c/+0x34) = 0x00000000 / 0x000a000a (packed shorts; NOT a float)
        /// @unofficial jumpSpeed/startScaleBase kept as raw u32 (not float) so index 1's
        /// unclaimed, non-round-trippable denormal bit patterns (0x0004000f, 0x001e0000)
        /// can be stored exactly; case 0 reinterprets index 0's genuinely-float values
        /// via a pointer cast rather than risk a decimal literal not round-tripping a
        /// denormal through the compiler's float parser.
        u32 jumpSpeedRaw;    ///< +0x8 (pool +0x20/+0x38) = 0x41200000(10.0f) / 0x0004000f (idx1 unclaimed)
        u32 unk24;            ///< +0xc (pool +0x24/+0x3c) = 0x003c0000 / 0x43480000(200.0f, idx1 unclaimed)
        u32 startScaleBaseRaw; ///< +0x10 (pool +0x28/+0x40) = 0x3e4ccccd(0.2f) / 0x001e0000 (idx1 unclaimed)
        float targetScaleBase; ///< +0x14 (pool +0x2c/+0x44) = 1.0 / 0.0
    };
    /// @unofficial Index 0 fully confirmed (runMain() case 0, every field consumed
    /// or independently value-checked). Index 1: ONLY `.scaleMul` is confirmed
    /// (case 2 reads `sc_jumpParams[1].scaleMul` into `mSpeedF`, proven by the
    /// SAME "`+0x18` then a further `+0x18`" pointer-arithmetic shape as case 0's
    /// access to index 0); its other three fields hold the RAW bytes from the REL
    /// but are unclaimed by any authored code, so their type/meaning is a guess --
    /// `unk1c`/`unk24` are plainly NOT floats (0x000a000a, 0x0004000f each look
    /// like two packed 16-bit values, not a valid IEEE float), so u32 is honest
    /// where scalar-float would not be. Growing this array further needs a case
    /// that reads pool offset 0x48+ the same way.
    static const JumpParams_t sc_jumpParams[2];

    void procNone(); ///< @unofficial fn_2_16D7E0. The idle/no-op state-0 handler. Defined out-of-line
                      ///< (NOT inline in the class body): the target marks fn_2_16D7E0 `global`, not
                      ///< `weak`, which an in-class trivial inline would compile as (vague linkage).
                      ///< This also fixes its .text POSITION -- an in-class inline landed wherever the
                      ///< compiler first needed it (adjacent to execute()'s PTMF table reference),
                      ///< which put it far from its true target address between resetScaleAndProc()
                      ///< and startAction().
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

const char *daWmKoopaJr_c::sc_animNames[6] = {
    "wait", "run", "jump_st", "jumpA", "jump_ed", "shock_wmap"
};
const m3d::playMode_e daWmKoopaJr_c::sc_playModes[6] = {
    m3d::FORWARD_LOOP, m3d::FORWARD_LOOP,
    m3d::FORWARD_ONCE, m3d::FORWARD_ONCE, m3d::FORWARD_ONCE, m3d::FORWARD_ONCE
};
const daWmKoopaJr_c::JumpParams_t daWmKoopaJr_c::sc_jumpParams[2] = {
    {2.5f, 0x00000000, 0x41200000, 0x003c0000, 0x3e4ccccd, 1.0f},
    {6.8f, 0x000a000a, 0x0004000f, 0x43480000, 0x001e0000, 0.0f}
};

daWmKoopaJr_c::daWmKoopaJr_c() {}

daWmKoopaJr_c::~daWmKoopaJr_c() {}

// @unofficial FUNCTION DEFINITION ORDER BELOW IS DELIBERATE, not source-cosmetic. The
// linker places .text in definition order, so these must appear in the SAME relative
// order as the target's real addresses: create(0x16d3f0), execute(0x16d460),
// draw(0x16d530), doDelete(0x16d580), createModel(0x16d590), calcModel(0x16d700),
// resetState(0x16d7b0), resetScaleAndProc(0x16d7c0), procNone(0x16d7e0),
// startAction(0x16d7f0), procMain(0x16d830), processCutsceneCommand(0x16d870),
// lookupAction(0x16d920), runMain(0x16d940), changeAnim(0x16e3a0). An EARLIER draft of
// this file had these in an unrelated order (grouped by discovery order instead), which
// verify_anon.py's greedy ascending-pairing flagged directly (FUNCTION ORDER IS WRONG)
// -- confirmed against d_a_wm_smallcloud.cpp's documented precedent in that tool's own
// docstring, where a clean-looking per-function tally still failed to link because every
// `bl` past the misordered point had the wrong displacement. This also matters for the
// anonymous rodata POOL: literal constants are pooled in definition order, so an out-of-
// order function steals another function's expected pool slot and produces exactly the
// kind of "logic matches, displacement is off by N words" symptom this unit had.
int daWmKoopaJr_c::create() {
    createModel();
    mClipSphere.set(mPos, 250.0f);
    calcModel();
    resetState();
    return SUCCEEDED;
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

int daWmKoopaJr_c::draw() {
    mModel.entry();
    DrawShadow(true);
    return SUCCEEDED;
}

int daWmKoopaJr_c::doDelete() {
    return SUCCEEDED;
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

void daWmKoopaJr_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daWmKoopaJr_c::resetState() {
    mUnk35c = -1;
    resetScaleAndProc();
}

void daWmKoopaJr_c::resetScaleAndProc() {
    mProcState = 0;
    mScale.x = 0.01f;
    mScale.y = 0.01f;
    mScale.z = 0.01f;
}

void daWmKoopaJr_c::procNone() {}

void daWmKoopaJr_c::startAction(int type) {
    lookupAction(type);
    mProcState = 1;
}

void daWmKoopaJr_c::procMain() {
    if (runMain()) {
        resetScaleAndProc();
    }
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

bool daWmKoopaJr_c::runMain() {
    /// @unofficial PARTIAL. fn_2_16D940 (0xA60) is a 16-case state machine dispatched
    /// through `jumptable_2_data_45EC4` on mUnk340 (`cmplwi r0,0xf; bgt <epilogue>`),
    /// implementing Bowser Jr.'s appear/land/run/disappear cutscene sequence. Cases 0,
    /// 1, 2, 14 and 15 are authored (each verified against the disassembly
    /// instruction-by-instruction); case 15 is the ONLY path that returns true, via
    /// the same `this->vtable+0x60 -> +0x68` dispatch as processCutsceneCommand()'s
    /// case 0x45, i.e. `setCutEnd()`. Cases 3-13 are explicit stubs -- NOT attempted,
    /// not guessed. See MAPPING.md for the case-target address table and what each
    /// stubbed case's first few instructions show, for the next round.
    switch (mUnk340) {
    case 0: {
        mVec3_c targetPos = daWmMap_c::m_instance->GetPos(daWmPlayer_c::ms_instance->m_22c);
        mJumpTargetPos = targetPos;
        changeAnim(3, 5.0f, 1.0f, 0.0f);
        const JumpParams_t &jp = sc_jumpParams[0];
        float jumpSpeed = *(const float *)&jp.jumpSpeedRaw;
        float startScaleBase = *(const float *)&jp.startScaleBaseRaw;
        _initDemoJumpBase(mJumpTargetPos, 0, 0x3c, jumpSpeed,
                          startScaleBase * jp.scaleMul,
                          jp.targetScaleBase * jp.scaleMul, mVec3_c::Ey);
        mUnk340 = 1;
        mJumpTimer = 0x1e;
        break;
    }

    /// @unofficial Cases 1-14 NOT authored. Confirmed target addresses (from
    /// `jumptable_2_data_45EC4`, unit .text-relative): 1=0x16DA3C 2=0x16DAD8
    /// 3=0x16DB68 4=0x16DCDC 5=0x16DDA8 6=0x16DDD8 7=0x16DE8C 8=0x16DF10
    /// 9=0x16DFA8 10=0x16E0EC 11=0x16E140 12=0x16E220 13=0x16E240 14=0x16E340.
    case 1: {
        mJumpTimer--;
        if (mJumpTimer == 0) {
            R_2_1_15F8F0(this, 1.0f, 0.0f);
            dWmSeManager_c::m_pInstance->playSound(0x3f, mPos, 1);
        }
        if (!_procDemoJumpBase()) {
            break;
        }
        dWmSeManager_c::m_pInstance->playSound(0x40, mPos, 1);
        dWmEffectManager_c::m_pInstance->playEffect(0xc, &mPos, nullptr, nullptr);
        changeAnim(4, 5.0f, 0.5f, 0.0f);
        mUnk340 = 2;
        break;
    }

    case 2: {
        if (!mAnimChrs[4].isStop()) {
            break;
        }
        mJumpTimer = 0x2d;
        changeAnim(1, 5.0f, 1.5f, 0.0f);
        mSpeedF = sc_jumpParams[1].scaleMul;
        setDirection(mVec3_c(1.0f, 0.0f, 0.0f));
        mUnk35c = fn_80103520(dWmEffectManager_c::m_pInstance, 2, &mModel, "koopaJr_all_root", 0, 0);
        mUnk340 = 3;
        break;
    }

    /// @unofficial Case 3 (0x16DB68-0x16DCDC) authored directly off the disassembly.
    /// Every constant read, none guessed:
    /// - `mAnimChrs[1].checkFrame(9.0f)` / `checkFrame(28.0f)` -- `this+0x220` is
    ///   `mAnimChrs[1]` (`+0x1e8 + 1*0x38 = +0x220`); `fanm_c::checkFrame(float) const`
    ///   (`include/game/mLib/m_3d/fanm.hpp:35`) matches `checkFrame__Q23m3d6fanm_cCFf`
    ///   exactly; `anmChr_c : public fanm_c` needs no vtable adjustment, matching the
    ///   direct `this+0x220` call target with no extra indirection. Pool values 9.0f/
    ///   28.0f read directly (pool offsets 0xb0/0xb4).
    /// - `adjustHeightBase(mJumpTargetPos, {x+200,y,z}, 5)` -- SAME shape as case 4,
    ///   confirmed by both cases loading the identical pool value (`sc_jumpParams[1].
    ///   unk24` = 200.0f) through the identical `&sc_jumpParams[0]+0x24` pointer
    ///   arithmetic.
    /// - The `0 < mJumpTimer <= 10` branch computes an angle via
    ///   `(pool0xb8 / 10.0f) * mAng::DegreeToAngleCoefficient` then `rotDirectionY`
    ///   (pool0xb8 = -180.0f, read directly). The target computes the literal `10`'s
    ///   int-to-float conversion via a magic-double-bias trick (`lis 0x4330`/
    ///   `xoris .../0x8000`/`lfd` against pool+0xd8, the exact 0x4330000080000000 bias
    ///   constant) rather than folding it to a plain `10.0f` immediate -- a known MWCC
    ///   codegen quirk for int-literal divisors, NOT reproduced here (this C++ writes
    ///   the mathematically identical `/ 10.0f`); pool offset 0xd8 is logically
    ///   accounted for by this case even though the compiled bytes will not reference
    ///   it the same way.
    /// - `mSpeedF` (this+0x10c, not a new field) computed by header field-offset
    ///   arithmetic in `dBaseActor_c` (`mMatrix@0x7c` -> `mPos@0xac` -> ... ->
    ///   `mAngle@0x100` -> `mAngle3D@0x106`, ending exactly at `0x10c` with no padding
    ///   gap), corroborated by case 2 already writing this same field from
    ///   `sc_jumpParams[1].scaleMul` at the same offset.
    /// - The final-branch dispatch (`mJumpTimer<=0`) is `changeAnim(4,...)` +
    ///   `mUnk340=4` + `setCutEnd()`, identical in shape to case 4's ending; pool0xa8
    ///   (5.0f, the blend-frame argument) matches every other `changeAnim` call's
    ///   observed blend value.
    case 3: {
        if (mJumpTimer > 10) {
            if (mCurAnimIdx == 1) {
                if (mAnimChrs[1].checkFrame(9.0f) || mAnimChrs[1].checkFrame(28.0f)) {
                    dWmSeManager_c::m_pInstance->playSound(0x41, mPos, 1);
                }
            }
            mJumpTimer--;
            calcSpeed();
            posMove();
            float farThreshold = *(const float *)&sc_jumpParams[1].unk24;
            mVec3_c heightTarget(mJumpTargetPos.x + farThreshold, mJumpTargetPos.y, mJumpTargetPos.z);
            adjustHeightBase(mJumpTargetPos, heightTarget, 5);
        } else if (mJumpTimer > 0) {
            if (mUnk35c >= 0) {
                dWmEffectManager_c::m_pInstance->endEffect(mUnk35c);
                mUnk35c = -1;
            }
            mJumpTimer--;
            rotDirectionY((short)((-180.0f / 10.0f) * mAng::DegreeToAngleCoefficient), true);
            mSpeedF = 0.0f;
        } else {
            changeAnim(4, 5.0f, 1.0f, 0.0f);
            mUnk340 = 4;
            setCutEnd();
        }
        break;
    }


    /// @unofficial Case 4 (0x16DCDC-0x16DDA8) authored directly off the disassembly --
    /// every constant is directly visible, none guessed:
    /// - `fn_80103520`'s 4th arg is "koopaJr_all_root" (read straight out of the REL's
    ///   .data at lbl_2_data_45EB0, the SAME string case 2 already uses).
    /// - `sc_jumpParams[1].unk24` is loaded here via `lfs` (not as a raw word), and its
    ///   bit pattern 0x43480000 IS a clean round-trippable float (200.0f) -- unlike
    ///   index 0's same field (0x003c0000, a denormal), so this is the first authored
    ///   evidence that `unk24` is genuinely a float field, just read through the
    ///   existing reinterpret-cast convention (matching case 0's jumpSpeedRaw/
    ///   startScaleBaseRaw handling) rather than widening the struct's declared type,
    ///   since index 0's value still does not round-trip through a decimal literal.
    /// - `adjustHeightBase(startPos, targetPos, directionType)` matches
    ///   `include/game/bases/d_wm_demo_actor.hpp:53` exactly; `mVec3_c::distTo`
    ///   (`include/game/mLib/m_vec.hpp:214`) matches the `PSVECSquareDistance`+`sqrt`
    ///   pair exactly, argument order confirmed (`this`=&mPos, arg=&mJumpTargetPos).
    /// - The trailing "this+0x60 -> +0x68" dispatch is `setCutEnd()`, the SAME pattern
    ///   already confirmed in case 15 and processCutsceneCommand()'s case 0x45.
    case 4: {
        if (mUnk35c < 0) {
            mUnk35c = fn_80103520(dWmEffectManager_c::m_pInstance, 2, &mModel, "koopaJr_all_root", 0, 0);
        }
        calcSpeed();
        posMove();
        float farThreshold = *(const float *)&sc_jumpParams[1].unk24;
        mVec3_c heightTarget(mJumpTargetPos.x + farThreshold, mJumpTargetPos.y, mJumpTargetPos.z);
        adjustHeightBase(mJumpTargetPos, heightTarget, 5);
        if (mPos.distTo(mJumpTargetPos) > farThreshold) {
            if (mUnk35c >= 0) {
                dWmEffectManager_c::m_pInstance->endEffect(mUnk35c);
                mUnk35c = -1;
            }
            setCutEnd();
        }
        break;
    }

    /// @unofficial Case 5 (0x16DDA8-0x16DDD8) authored directly off the disassembly.
    /// `changeAnim(0, 5.0f, 1.0f, 0.0f)` matches the same argument shape as every
    /// other `changeAnim` call seen so far (pool 0xa8=5.0f blend, pool 0x8c=1.0f
    /// rate, pool 0x90=0.0f start frame -- all three already-established pool
    /// slots, no new float introduced here). `mSpeedF = 0.0f` (this+0x10c, see the
    /// case 3 comment for the field derivation), `mJumpTimer = 10`, `mUnk340 = 6`.
    case 5: {
        changeAnim(0, 5.0f, 1.0f, 0.0f);
        mSpeedF = 0.0f;
        mJumpTimer = 0xa;
        mUnk340 = 6;
        break;
    }

    case 6:
        break;

    /// @unofficial Case 9 (0x16DFA8-0x16E0EC) authored directly off the disassembly.
    /// Pool: 180.0f (0xcc) / literal 4 -> DegreeToAngleCoefficient angle, same
    /// magic-double-for-int-literal quirk noted in case 3 (not reproduced, logically
    /// equivalent `/ 4.0f` used instead). `mSpeedF = sc_jumpParams[1].scaleMul`
    /// (SAME field/address case 2 already reads, confirmed by the identical
    /// `&sc_jumpParams[0]+0x18+0x18` pointer arithmetic). `dBase_c::
    /// searchBaseByProfName(0x284, nullptr)` matches `include/game/bases/
    /// d_base.hpp:31` exactly; the landed precedent for reading a field off its
    /// result is a plain C-style cast to the concrete actor type (grepped across
    /// `source/dol/bases/*.cpp`, e.g. `d_a_en_door.cpp:18`), so the result is cast
    /// to `dBaseActor_c *` before reading `mPos` at the already-established +0xac
    /// offset. The trailing `daWmPlayer_c::ms_instance` dispatch is at vtable
    /// slot "this+0x60 -> +0x64", ONE SLOT BEFORE the already-confirmed setCutEnd
    /// slot ("+0x60 -> +0x68") -- `checkCutEnd()` is declared immediately before
    /// `setCutEnd()` in `d_wm_demo_actor.hpp`, so by declaration-order vtable
    /// layout this is `daWmPlayer_c::ms_instance->checkCutEnd()`.
    case 9: {
        if (mJumpTimer > 0) {
            mJumpTimer--;
            rotDirectionY((short)((180.0f / 4.0f) * mAng::DegreeToAngleCoefficient), true);
        } else {
            if (mUnk35c < 0) {
                mUnk35c = fn_80103520(dWmEffectManager_c::m_pInstance, 2, &mModel, "koopaJr_all_root", 0, 0);
            }
            mSpeedF = sc_jumpParams[1].scaleMul;
            setDirection(mVec3_c(1.0f, 0.0f, 0.0f));
            calcSpeed();
            posMove();
            dBaseActor_c *target = (dBaseActor_c *)dBase_c::searchBaseByProfName(0x284, nullptr);
            mVec3_c heightTarget(target->mPos.x, mJumpTargetPos.y, mJumpTargetPos.z);
            adjustHeightBase(mJumpTargetPos, heightTarget, 5);
            if (daWmPlayer_c::ms_instance->checkCutEnd()) {
                setCutEnd();
            }
        }
        break;
    }


    /// @unofficial Case 7 (0x16DE8C-0x16DF10) authored directly off the disassembly --
    /// `changeAnim(5, 5.0f, 1.0f, 0.0f)` (same three already-established pool
    /// slots), two `playSound` calls (ids 0x42/0x43, same shape as case 3's),
    /// `mJumpTargetPos = mPos` (field-by-field, same convention as case 10),
    /// `setDirection(mVec3_c(-1.0f, 0.0f, 0.0f))` (pool0xbc = -1.0f, already seen
    /// in the dump; pool0x90 = 0.0f reused for y/z), `mUnk340 = 8`.
    case 7: {
        changeAnim(5, 5.0f, 1.0f, 0.0f);
        dWmSeManager_c::m_pInstance->playSound(0x42, mPos, 1);
        dWmSeManager_c::m_pInstance->playSound(0x43, mPos, 1);
        mJumpTargetPos.x = mPos.x;
        mJumpTargetPos.y = mPos.y;
        mJumpTargetPos.z = mPos.z;
        setDirection(mVec3_c(-1.0f, 0.0f, 0.0f));
        mUnk340 = 8;
        break;
    }

    /// @unofficial Case 8 (0x16DF10-0x16DFA8) authored directly off the disassembly.
    /// `this+0x300` is `mAnimChrs[5]` (`0x1e8 + 5*0x38 = 0x300`). Pool values 34.0f/
    /// 46.0f (checkFrame thresholds) and 2.0f (a DIFFERENT rate than every other
    /// `changeAnim` call seen so far -- confirmed distinctly by `lfs f2, 0xc8(r31)`
    /// landing in the rate argument slot, not the blend or start-frame slots).
    case 8: {
        if (mAnimChrs[5].checkFrame(34.0f) || mAnimChrs[5].checkFrame(46.0f)) {
            dWmEffectManager_c::m_pInstance->playEffect(0xc, &mPos, nullptr, nullptr);
            dWmSeManager_c::m_pInstance->playSound(0x40, mPos, 1);
        }
        if (mAnimChrs[5].isStop()) {
            mUnk340 = 9;
            mJumpTimer = 4;
            changeAnim(1, 5.0f, 2.0f, 0.0f);
        }
        break;
    }


    /// @unofficial Case 10 (0x16E0EC-0x16E140) authored directly off the disassembly.
    /// `mSpeedF = sc_jumpParams[1].scaleMul + pool0xc8` (pool0xc8 = 2.0f read
    /// directly; `sc_jumpParams[1].scaleMul` = 6.8f, the SAME field case 2 already
    /// reads via `&sc_jumpParams[0]+0x18` pointer arithmetic -- here the target
    /// re-derives the identical address, `&sc_jumpParams[0]+0x18+0x18`). Then
    /// `setDirection(mVec3_c(1,0,0))` (byte-identical shape to case 0's identical
    /// call), `mJumpTargetPos = mPos` (three individual `lfs`/`stfs` pairs, no
    /// `mVec3_c` copy-ctor call, matching the project's "no temp observed ->
    /// direct field stores" convention), `mUnk340 = 11`.
    case 10: {
        mSpeedF = sc_jumpParams[1].scaleMul + 2.0f;
        setDirection(mVec3_c(1.0f, 0.0f, 0.0f));
        mJumpTargetPos.x = mPos.x;
        mJumpTargetPos.y = mPos.y;
        mJumpTargetPos.z = mPos.z;
        mUnk340 = 11;
        break;
    }

    case 11:
        break;

    /// @unofficial Case 12 (0x16E220-0x16E240) authored directly off the disassembly --
    /// trivial, identical shape to case 5/case 4's tail: `changeAnim(2, 5.0f, 1.0f,
    /// 0.0f)` (same three already-established pool slots), `mUnk340 = 13`.
    case 12: {
        changeAnim(2, 5.0f, 1.0f, 0.0f);
        mUnk340 = 13;
        break;
    }

    case 13:
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
