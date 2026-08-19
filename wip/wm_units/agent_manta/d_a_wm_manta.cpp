#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_manta.hpp>
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_s_world_map_static.hpp>

ACTOR_PROFILE(WM_MANTA, daWmManta_c, 0);

namespace {
    // lbl_2_rodata_8D78, 0xc bytes -- one CW generalised-PTMF entry
    // (function/vtable-slot word + delta + flags), matching the same
    // idiom found on the dance_pakkun unit. @unofficial the real target
    // function is unknown -- unusedStub is a placeholder so the table's
    // SHAPE and __ptmf_scall call convention are reproduced, not its
    // target.
    typedef void (daWmManta_c::*ProcFunc_t)();
    const ProcFunc_t sProcTable[1] = { &daWmManta_c::unusedStub };
}

daWmManta_c::daWmManta_c() {
}

daWmManta_c::~daWmManta_c() {}

int daWmManta_c::create() {
    createModel();
    mClipSphere.set(mPos, 250.0f); // measured: lbl_2_rodata_8D74
    startStep();
    calcModel(&mModel);
    return SUCCEEDED;
}

int daWmManta_c::execute() {
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);

    (this->*sProcTable[m_224])();

    calcModel(&mModel);
    return SUCCEEDED;
}

int daWmManta_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmManta_c::doDelete() {
    return SUCCEEDED;
}

void daWmManta_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    // measured: lbl_2_data_46384 / lbl_2_data_46398 -- the internal model
    // name is "togezo", not "manta" (a dev-name mismatch with the
    // WM_MANTA profile enum -- see MAPPING.md).
    mResFile = dResMng_c::m_instance->getRes("togezo", "g3d/togezo.brres");
    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl("togezo");

    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    dWmActor_c::setSoftLight_MapObj(mModel);
    mAllocator.adjustFrmHeap();
}

void daWmManta_c::calcModel(m3d::mdl_c *mdl) {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mdl->setLocalMtx(&mMatrix);
    mdl->setScale(mScale);
    mdl->calc(false);
}

void daWmManta_c::startStep() {
    mScale.x = 1.8f; // measured: lbl_2_rodata_8D70
    mScale.y = 1.8f;
    mScale.z = 1.8f;
    resetStep();
}

void daWmManta_c::resetStep() {
    m_224 = 0;
}

void daWmManta_c::unusedStub() {
}

void daWmManta_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId != dCsSeqMng_c::CUTSCENE_CMD_NONE) {
        setCutEnd();
    }
}

// @unofficial fn_2_171320 -- counts how many lettered model-name variants
// ("a","b","c",...) resolve via GetResMdl() inside a per-world archive
// named "CS_W<worldNo+1 in hex>", loaded via the generic "g3d/model.brres"
// path. Real purpose/return-type semantics not fully confirmed -- see
// MAPPING.md.
int daWmManta_c::countModelVariants() {
    char nameA[0x10];
    char nameB[0x10];
    snprintf(nameA, 0x10, "CS_W%X", dScWMap_c::m_WorldNo + 1);
    snprintf(nameB, 0x10, "CS_W%X", dScWMap_c::m_WorldNo + 1);

    nw4r::g3d::ResFile resFile = dResMng_c::m_instance->getRes(nameA, "g3d/model.brres");
    nw4r::g3d::ResMdl resMdl = resFile.GetResMdl(nameB);
    if (!resMdl.IsValid()) {
        return 1;
    }

    int count = 0;
    char letter[3];
    for (;;) {
        letter[0] = (char)('a' + count);
        letter[1] = 0;
        resMdl = resFile.GetResMdl(nameB); // @unofficial -- real 2nd arg likely combines nameB+letter
        if (!resMdl.IsValid()) {
            break;
        }
        count++;
    }
    return count;
}

// @unofficial fn_2_171400 -- reads dScWMap_c::m_WorldNo directly (not
// through the NOINLINE dScWMap_c::getWorldNo() accessor -- confirmed by
// the inlined `lis`/`lbz` shape rather than a `bl`).
u8 daWmManta_c::getWorldNo() {
    return dScWMap_c::m_WorldNo;
}
