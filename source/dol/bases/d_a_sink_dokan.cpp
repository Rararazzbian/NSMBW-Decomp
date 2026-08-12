#include <game/bases/d_a_sink_dokan.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/mLib/m_allocator_dummy_heap.hpp>
#include <constants/sound_list.h>

const float daSinkDokan_c::smc_MAX_SPEED = 3.0f;

int daSinkDokan_c::create() {
    u32 param = mParam;
    mDokanType = ACTOR_PARAM_LOCAL(param, DokanType);
    m_63c = ACTOR_PARAM_LOCAL(param, Unk63c);

    createMdl();

    param = mParam;
    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;
    mLength = ACTOR_PARAM_LOCAL(param, UnitCount) * 16.0f;
    mPlayerNo = ACTOR_PARAM_LOCAL(param, PlayerNo);

    adjustPos();
    calcMoveDistMax();
    setMoveDist();
    vfe8();
    vfd8();

    mBgCtr.entry();

    s16 angleZ = mAngle.z;
    mTopAngle = angleZ;
    if (mDokanType == DOKAN_TWIN) {
        mRootAngle = angleZ + 0x8000;
    } else {
        mRootAngle = angleZ;
    }

    dAudio::SoundEffectID_t(SE_OBJ_DOKAN_BREAK).playMapSound(mPos, 0);

    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;

    mStartPos.x = mPos.x;
    mStartPos.y = mPos.y;
    mStartPos.z = mPos.z;

    return SUCCEEDED;
}

void daSinkDokan_c::adjustPos() {}

void daSinkDokan_c::calcMoveDistMax() {}

void daSinkDokan_c::setMoveDist() {
    mMoveDist = 16.0f;
    if (mMoveDist > mMoveDistMax) {
        mMoveDist = mMoveDistMax;
    }
}

void daSinkDokan_c::createMdl() {
    static const char *cs_mdl_top_name[3] = { "obj_dokan_A", "obj_dokan_CU", "obj_dokan_A" };
    static const char *cs_mdl_root_name[3] = { "obj_dokan_B", "obj_dokan_B", "obj_dokan_A" };

    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("obj_dokan", "g3d/obj_dokan.brres");

    mTopModel.create(mResFile.GetResMdl(cs_mdl_top_name[mDokanType]), &mAllocator, 0x20);
    dActor_c::setSoftLight_MapObj(mTopModel);

    for (int i = 0; i < 0x20; i++) {
        mUnitModels[i].create(mResFile.GetResMdl("obj_dokan_B"), &mAllocator, 0x20);
        dActor_c::setSoftLight_MapObj(mUnitModels[i]);
    }

    mRootModel.create(mResFile.GetResMdl(cs_mdl_root_name[mDokanType]), &mAllocator, 0x20);
    dActor_c::setSoftLight_MapObj(mRootModel);

    mAllocator.adjustFrmHeap();
}

int daSinkDokan_c::execute() {
    if (mMoveDist == 0.0f || vfd4()) {
        setDokanUnit();
        deleteRequest();
    } else {
        mBgCtr.calc();
    }
    return SUCCEEDED;
}

int daSinkDokan_c::draw() {
    static const float cs_root_ofs[3] = { 8.0f, 8.0f, 16.0f };

    float cos = mAng((s16) mAngle.z + 0x4000).cos();
    float sin = mAng((s16) mAngle.z + 0x4000).sin();

    int cnt = ACTOR_PARAM(UnitCount);
    float ofs = cnt * 16.0f;
    float half = ofs * 0.5f;
    ofs -= 16.0f;
    float baseX = mPos.x - half * cos;
    float baseY = mPos.y - half * sin;

    mVec3_c pos;
    mAng3_c angle;

    pos.x = baseX + cos * (ofs - half);
    pos.y = baseY + sin * (ofs - half);
    pos.z = mPos.z;
    angle = mAngle;
    angle.z = mTopAngle;
    changePosAngle(&pos, &angle, 1);
    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.ZrotM(angle.z);
    mTopModel.setLocalMtx(&mMatrix);
    mTopModel.setScale(mScale);
    mTopModel.calc(false);
    mTopModel.entry();

    ofs -= 8.0f;

    m3d::smdl_c *model = mUnitModels;
    int end = cnt - 2;
    for (int i = 0; i < end; i++) {
        pos.x = baseX + cos * (ofs - half);
        pos.y = baseY + sin * (ofs - half);
        pos.z = mPos.z;
        angle = mAngle;
        changePosAngle(&pos, &angle, 1);
        mMatrix.trans(pos.x, pos.y, pos.z);
        mMatrix.ZrotM(angle.z);
        model->setLocalMtx(&mMatrix);
        model->setScale(mScale);
        model->calc(false);
        model->entry();
        ofs -= 16.0f;
        model++;
    }

    pos.x = baseX + cos * (cs_root_ofs[mDokanType] - half);
    pos.y = baseY + sin * (cs_root_ofs[mDokanType] - half);
    pos.z = mPos.z;
    angle = mAngle;
    angle.z = mRootAngle;
    changePosAngle(&pos, &angle, 1);
    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.ZrotM(angle.z);
    mRootModel.setLocalMtx(&mMatrix);
    mRootModel.setScale(mScale);
    mRootModel.calc(false);
    mRootModel.entry();

    return SUCCEEDED;
}

void daSinkDokan_c::deleteReady() {}

int daSinkDokan_c::doDelete() {
    mBgCtr.release();

    if (mAllocator.mpHeap != mAllocatorDummyHeap_c::getInstance()) {
        for (int i = 0; i < 0x20; i++) {
            mUnitModels[i].remove();
        }
        mTopModel.remove();
        mRootModel.remove();
    }

    return SUCCEEDED;
}

void daSinkDokan_c::setDokanUnit() {
    switch (mDokanType) {
        case DOKAN_NORMAL:
            setDokanUnit_Normal();
            break;
        case DOKAN_BREAK:
            setDokanUnit_Break();
            break;
        case DOKAN_TWIN:
            setDokanUnit_Twin();
            break;
    }
}

void daSinkDokan_c::setDokanUnit_Normal() {}

void daSinkDokan_c::setDokanUnit_Break() {}

void daSinkDokan_c::setDokanUnit_Twin() {}

daSinkDokan_c::~daSinkDokan_c() {}
