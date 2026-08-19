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

int daWmDancePakkun_c::create() {
    // @unofficial -- mBgmSync's field init below is simplified. The real
    // function indexes lbl_2_data_445D0 by a byte extracted from mParam
    // (this+0x4) and derives mBgmSync->m_18/m_04/m_08 from two s16 fields at
    // a computed offset within that table; not reproduced -- see MAPPING.md.
    mBgmSync = new dWmBgmSync_c();

    createModel();

    mClipSphere.set(mPos, 0.0f); // @unofficial radius (lbl_2_rodata_87F0) unknown

    startStep();
    calcModelFor(&mModel);
    m_2d8 = 1.0f; // @unofficial constant (lbl_2_rodata_87F4) unknown
    return SUCCEEDED;
}

int daWmDancePakkun_c::execute() {
    mBgmSync->execute();

    if (m_2d8 == 1.0f) { // @unofficial constant (lbl_2_rodata_87F4) unknown
        if (mBgmSync->m_0c) {
            m_2d8 = mChrAnim[0].mFrameMax / mBgmSync->getAnmRate(mChrAnim[0].mFrameMax);
            mChrAnim[0].setRate(mBgmSync->getAnmRate(mChrAnim[0].mFrameMax));
        }
    }

    // Ordinary virtual dispatch -- daWmDancePakkun_c does not override
    // processCutsceneCommand(), so this resolves to dWmDemoActor_c's own
    // implementation through the this+0x60 secondary vtable slot (see the
    // block comment above the ctor).
    processCutsceneCommand(dCsSeqMng_c::ms_instance->GetCutName(), dCsSeqMng_c::ms_instance->m_164);

    // @unofficial -- the real function also does a member-function-pointer
    // dispatch here, indexed by m_2bc*0xc into a table at lbl_2_rodata_87F8
    // (`__ptmf_scall`); not reproduced.

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

void daWmDancePakkun_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("pakkun", "g3d/pakkun.brres");
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl("pakkun");

    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::ANM_TEXSRT | nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);
    mModel2.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    static const char *sAnmNames[1] = { "cs_wait" };
    nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr(sAnmNames[0]);
    mChrAnim[0].create(resMdl, resAnmChr, &mAllocator, nullptr);

    static const m3d::playMode_e sPlayModes[1] = { m3d::FORWARD_LOOP }; // @unofficial guessed
    mChrAnim[0].mPlayMode = sPlayModes[0];
    mChrAnim[0].setRate(1.0f);  // @unofficial constant (lbl_2_rodata_87F4) unknown
    mChrAnim[0].setFrame(1.0f); // @unofficial constant (lbl_2_rodata_87F4) unknown

    mModel.setAnm(mChrAnim[0]);

    dWmActor_c::setSoftLight_Enemy(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmDancePakkun_c::tailHelper() {
    mModel.play();
}

void daWmDancePakkun_c::calcModelFor(m3d::mdl_c *mdl) {
    // @unofficial -- the real function derives a dance-rotation offset from
    // mChrAnim[0].getFrame()/getRate() blended against lbl_2_data_445D0 and
    // mAngle before building mMatrix; not reproduced (MAPPING.md). Left
    // unauthored per the coordinator's explicit priority ordering.
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mdl->setLocalMtx(&mMatrix);
    mdl->setScale(mScale);
    mdl->calc(false);
}

namespace {
    // @unofficial -- lbl_2_data_445D0, 0x50 bytes. Real layout/values are
    // unknown; this stands in for the *shape* only: startStep() reads a
    // {dx,dy,dz} delta at +0x40 and a single "reset scale" float at +0x0,
    // while calcModelFor() indexes the leading region by frame*8 (unused
    // here). See MAPPING.md Open question 4.
    struct StepTable_t {
        float pairs[16];
        float dx, dy, dz, unused3;
    };
    const StepTable_t sStepTable = { { 1.0f }, 2.0f, 3.0f, 4.0f, 0.0f };
}

void daWmDancePakkun_c::startStep() {
    mPos.x += sStepTable.dx;
    mPos.y += sStepTable.dy;
    mPos.z += sStepTable.dz;
    mAngle.y = 0x4000;
    unusedStub();
    mScale.x = sStepTable.pairs[0];
    mScale.y = sStepTable.pairs[0];
    mScale.z = sStepTable.pairs[0];
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
