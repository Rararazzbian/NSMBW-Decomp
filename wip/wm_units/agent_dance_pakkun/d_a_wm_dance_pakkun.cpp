#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_dance_pakkun.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_wm_lib.hpp>

ACTOR_PROFILE(WM_DANCE_PAKKUN, daWmDancePakkun_c, 0);

// Open question 1 from earlier rounds is RESOLVED: `this+0x60` is an
// ordinary secondary vtable pointer, set automatically by the compiler for
// every dBase_c-derived class in the chain -- verified against the landed,
// byte-exact corpus: d_base.cpp's own ctor (`dBase_c::dBase_c()`) does
// `stw r4,0x60(r31)` with r4 = &__vt__7dBase_c computed via lis+addi, right
// after `stw r0,0x64(r31)` (cOwnerSetMg_c::mpRoot = nullptr); d_wm_demo_actor
// and daWmGhost_c's OWN ctors do the identical `stw r4,0x60(r29)` with their
// OWN `__vt__<Class>` symbol; and daWmGhost_c::execute() does the identical
// `lwz r12,0x60(r30); ...; lwz r12,0x60(r12); bctrl` to dispatch
// processCutsceneCommand(). `lbl_2_data_44648` (0x120 bytes) in our own
// unit's .data IS this class's own secondary vtable -- dtk just can't name
// it because this class isn't in the symbol map. No manual code is needed
// anywhere in this file for it; ordinary member declarations and ordinary
// virtual calls produce it automatically.

daWmDancePakkun_c::daWmDancePakkun_c() :
    m_184(-1)
{
}

daWmDancePakkun_c::~daWmDancePakkun_c() {}

namespace {
    // lbl_2_data_445D0, 0x50 bytes. Layout below is measured (coordinator
    // read it from the retail .data): a lone float at +0x0, six 8-byte
    // {float,u16,u16} records at +0x4 (three {65.0,20,2}, then three
    // {5.0,20,2}), three words of 0x00020000 at +0x34, then the {dx,dy,dz}
    // position delta startStep() reads at +0x40 (whose real values are
    // still unknown -- placeholders below).
    struct BgmRecord_t { float a; u16 b; u16 c; };
    struct StepTable_t {
        float scale0;
        BgmRecord_t records[6];
        u32 words[3];
        float dx, dy, dz, unused3; // @unofficial dx/dy/dz values still unknown
    };
    const StepTable_t sStepTable = {
        1.8f,
        { {65.0f,20,2}, {65.0f,20,2}, {65.0f,20,2}, {5.0f,20,2}, {5.0f,20,2}, {5.0f,20,2} },
        { 0x00020000, 0x00020000, 0x00020000 },
        2.0f, 3.0f, 4.0f, 0.0f
    };
}

int daWmDancePakkun_c::create() {
    mBgmSync = new dWmBgmSync_c();

    const s16 *bgmEntry = (const s16 *)&sStepTable.words[mParam & 0xff];
    mBgmSync->m_18 = bgmEntry;
    mBgmSync->m_04 = bgmEntry[0] - 1;
    mBgmSync->m_08 = bgmEntry[1];

    createModel();

    mClipSphere.set(mPos, 0.0f); // @unofficial radius (lbl_2_rodata_87F0) unknown

    startStep();
    calcModelFor(&mModel);
    m_2d8 = 1.0f; // @unofficial constant (lbl_2_rodata_87F4) unknown
    return SUCCEEDED;
}

namespace {
    // @unofficial -- lbl_2_rodata_87F8, 0xc bytes (one CW generalised-PTMF
    // entry). Real target function is unknown; unusedStub is a placeholder
    // so the table's SHAPE (one member-function-pointer, stride 0xc) and
    // the __ptmf_scall call convention are reproduced, not its target.
    const daWmDancePakkun_c::ProcFunc_t sProcTable[1] = {
        &daWmDancePakkun_c::unusedStub
    };
}

int daWmDancePakkun_c::execute() {
    mBgmSync->execute();

    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;

    if (m_2d8 == 1.0f) { // @unofficial constant (lbl_2_rodata_87F4) unknown
        if (mBgmSync->m_0c) {
            float frameMax = mChrAnim[0].mFrameMax;
            m_2d8 = frameMax / mBgmSync->getAnmRate(frameMax);
            float frameMax2 = mChrAnim[0].mFrameMax;
            mChrAnim[0].setRate(mBgmSync->getAnmRate(frameMax2));
        }
    }

    // Ordinary virtual dispatch -- daWmDancePakkun_c does not override
    // processCutsceneCommand(), so this resolves to dWmDemoActor_c's own
    // implementation through the this+0x60 secondary vtable slot (see the
    // block comment above the ctor).
    processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);

    // lbl_2_rodata_87F8 (0xc bytes -- ONE entry) is a table of
    // pointer-to-member-functions, matching CW's generalised-PTMF stride
    // (0xc = 3 words) and calling convention (__ptmf_scall). m_2bc is only
    // ever reset to 0 by resetStep() in these 16 functions, so a 1-entry
    // table is consistent with what's referenced. @unofficial the real
    // target function is unknown -- unusedStub is a placeholder so the
    // call SHAPE matches.
    (this->*sProcTable[m_2bc])();

    unusedStub();
    tailHelper(); // mModel.play()
    m_2d0 = true;
    calcModelFor(&mModel);
    return SUCCEEDED;
}

int daWmDancePakkun_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmDancePakkun_c::doDelete() {
    if (mBgmSync) {
        delete mBgmSync;
    }
    return SUCCEEDED;
}

namespace {
    // @unofficial -- coordinator-measured retail layout: a pointer
    // (relocated to "cs_wait") at unit-base+0x98, then "g3d/pakkun.brres"
    // (17 bytes incl. null) at +0x9c, then "pakkun" (7 bytes incl. null)
    // at +0xb0, all contiguous with no padding. Modelled as one aggregate
    // so the compiler anchors all three off one base register.
    struct ModelNames_t {
        const char *anmName;
        char brresPath[20]; // measured: retail pads this field to a 4-byte boundary (17 bytes of string + 3 padding)
        char modelName[7];
    };
    ModelNames_t sModelNames = { "cs_wait", "g3d/pakkun.brres", "pakkun" };
    const m3d::playMode_e sPlayModes[1] = { m3d::FORWARD_LOOP }; // @unofficial guessed
}

void daWmDancePakkun_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes(sModelNames.modelName, sModelNames.brresPath);
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl(sModelNames.modelName);

    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::ANM_TEXSRT | nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);
    mModel2.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr(sModelNames.anmName);
    m3d::anmChr_c &anim = mChrAnim[0];
    anim.create(resMdl, resAnmChr, &mAllocator, nullptr);

    anim.mPlayMode = sPlayModes[0];
    anim.setRate(1.0f);  // @unofficial constant (lbl_2_rodata_87F4) unknown
    anim.setFrame(1.0f); // @unofficial constant (lbl_2_rodata_87F4) unknown

    mModel.setAnm(anim);

    dWmActor_c::setSoftLight_Enemy(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmDancePakkun_c::tailHelper() {
    mModel.play();
}

namespace {
    // @unofficial -- rodata_87F0 sub-offset constants used by calcModelFor;
    // real values unknown, placeholders below so the SHAPE (same base,
    // several sub-offsets, one used as a double) matches. See MAPPING.md.
    struct CalcConsts_t {
        float f00, f04;         // +0x0, +0x4  (K2 at +0x4)
        u8 pad08[0x14];
        float f18;               // +0x18      (K1)
        double f20;               // +0x20     (K4, read as a double)
        float f28;                 // +0x28    (K3)
    };
    const CalcConsts_t sCalcConsts = { 0.0f, 2.0f, {0}, 1.0f, 3.0, 4.0f };
}

void daWmDancePakkun_c::calcModelFor(m3d::mdl_c *mdl) {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;

    float frameMax = mChrAnim[0].mFrameMax;
    u8 idx = mParam & 0xff;
    // @unofficial -- lbl_2_data_445D0 indexed by idx*8 from its BASE (not
    // the +0x34 offset create() uses); the float read is at +4 of that
    // 8-byte slot. Real semantics/values unresolved -- see MAPPING.md.
    const float *entry = (const float *)((const u8 *)&sStepTable + (u32)idx * 8);

    float frame1 = mChrAnim[0].getFrame();
    float denom1 = frameMax - sCalcConsts.f18;
    pos.y = pos.y + (frame1 / denom1) * entry[1];

    frameMax = mChrAnim[0].mFrameMax;
    float frame2 = mChrAnim[0].getFrame();
    float denom2 = frameMax - sCalcConsts.f18;
    float ratio = frame2 / denom2;
    if (mChrAnim[0].getRate() > sCalcConsts.f04) {
        ratio = (float)(sCalcConsts.f20 - ratio);
    }

    float angleRad = sCalcConsts.f28 * ratio * mAng::DegreeToAngleCoefficient;
    angle.y = (mAng)(int)angleRad;

    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mdl->setLocalMtx(&mMatrix);
    mdl->setScale(mScale);
    mdl->calc(false);
}

void daWmDancePakkun_c::startStep() {
    mPos.x += sStepTable.dx;
    mPos.y += sStepTable.dy;
    mPos.z += sStepTable.dz;
    mAngle.y = 0x4000;
    unusedStub();
    mScale.x = sStepTable.scale0;
    mScale.y = sStepTable.scale0;
    mScale.z = sStepTable.scale0;
    resetStep();
}

void daWmDancePakkun_c::resetStep() {
    m_2bc = 0;
}

void daWmDancePakkun_c::updateStepAnim() {
    if (mChrAnim[0].isStop()) {
        mChrAnim[0].setRate(-mChrAnim[0].getRate());
    }
}

void daWmDancePakkun_c::unusedStub() {
}
