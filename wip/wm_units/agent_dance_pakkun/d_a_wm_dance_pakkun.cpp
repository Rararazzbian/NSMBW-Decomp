#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_dance_pakkun.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_wm_lib.hpp>

ACTOR_PROFILE(WM_DANCE_PAKKUN, daWmDancePakkun_c, 0);

/// @unofficial All three of these are unresolved -- see MAPPING.md "Open
/// question 1/3/4". sDanceTable's true type/contents are unknown; it stands
/// in for lbl_2_data_44648 (0x120 bytes) so the ctor/execute() at least
/// compile with the right *shape* of access. This will not byte-match.
namespace {
    u8 sDanceTable[0x120]; ///< @unofficial placeholder for lbl_2_data_44648
}

daWmDancePakkun_c::daWmDancePakkun_c() :
    m_184(-1)
{
    // @unofficial -- MAPPING.md Open question 1: this+0x60 is not a named
    // member of fBase_c/dBase_c/cOwnerSetMg_c in the current, landed headers
    // (measured: cOwnerSetMg_c sub-object starts at dBase_c+0x64, and
    // fBase_c's own last named field, mHeap, ends at +0x60 with sizeof
    // reporting a further 4 bytes of unclaimed padding). Spelled as a raw
    // cast rather than inventing a header field.
    *(void **)((char *)this + 0x60) = sDanceTable;
}

daWmDancePakkun_c::~daWmDancePakkun_c() {}

int daWmDancePakkun_c::create() {
    // @unofficial -- the real function also indexes lbl_2_data_445D0 by a
    // byte extracted from mParam and initialises mBgmSync's fields from it;
    // not reproduced (MAPPING.md Open question 1/4).
    mBgmSync = new dWmBgmSync_c();

    mClipSphere.set(mPos, 90.0f); // @unofficial radius guessed
    createModel();
    calcModelFor(&mModel2);
    m_2d8 = 1.0f; // @unofficial guessed constant
    return SUCCEEDED;
}

int daWmDancePakkun_c::execute() {
    // @unofficial -- ping-pong / bgm-sync rate check ahead of the cutscene
    // dispatch is not reproduced; see MAPPING.md.
    if (mChrAnim[0].isStop()) {
        updateStepAnim();
    }

    // @unofficial -- MAPPING.md Open question 1. Spelled as a raw function
    // pointer call through the same this+0x60/+0x60 shape the disassembly
    // shows, rather than as processCutsceneCommand() (which would compile to
    // an ordinary vtable call and not match).
    typedef void (*CutsceneFn_t)(daWmDancePakkun_c *, int, bool);
    void *table = *(void **)((char *)this + 0x60);
    CutsceneFn_t fn = *(CutsceneFn_t *)((char *)table + 0x60);
    fn(this, dCsSeqMng_c::ms_instance->GetCutName(), dCsSeqMng_c::ms_instance->m_164);

    unusedStub();
    mModel.play();
    m_2bc = 1; // @unofficial -- the real code sets a byte at 0x2d0, not m_2bc; unresolved
    calcModelFor(&mModel2);
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

    nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes("cobDancePakkun", "g3d/model.brres"); // @unofficial name guessed
    nw4r::g3d::ResMdl resMdl = resFile.GetResMdl("cobDancePakkun"); // @unofficial name guessed

    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);
    mModel2.create(resMdl, &mAllocator, 0x20, 1); // @unofficial bufferOption guessed

    nw4r::g3d::ResAnmChr resAnmChr = resFile.GetResAnmChr("cobDancePakkunWait"); // @unofficial name guessed
    mChrAnim[0].create(resMdl, resAnmChr, &mAllocator, nullptr);
    mChrAnim[0].mPlayMode = m3d::FORWARD_LOOP; // @unofficial guessed
    mChrAnim[0].setRate(1.0f); // @unofficial guessed
    mChrAnim[0].setFrame(1.0f); // @unofficial guessed

    mModel.play(); // @unofficial -- shape only, exact vtable dispatch target unconfirmed here

    dWmActor_c::setSoftLight_Enemy(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmDancePakkun_c::tailHelper() {
    mModel.play();
}

void daWmDancePakkun_c::calcModelFor(m3d::mdl_c *mdl) {
    // @unofficial -- the real function derives a dance-rotation offset from
    // mChrAnim[0].getFrame()/getRate() blended against lbl_2_data_445D0 and
    // mAngle before building mMatrix; not reproduced (MAPPING.md).
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mdl->setLocalMtx(&mMatrix);
    mdl->setScale(mScale);
    mdl->calc(false);
}

void daWmDancePakkun_c::startStep() {
    // @unofficial -- adds a per-step {x,y,z} delta from lbl_2_data_445D0 into
    // mPos, forces mAngle.y = 0x4000, and resets mScale; not reproduced.
    unusedStub();
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
