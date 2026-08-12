#include <game/bases/d_a_en_dfpakkun.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_res_mng.hpp>
#include <constants/sound_list.h>

STATE_VIRTUAL_DEFINE(daEnDfpakkun_c, Wait);
STATE_VIRTUAL_DEFINE(daEnDfpakkun_c, Appear);
STATE_VIRTUAL_DEFINE(daEnDfpakkun_c, Attack);
STATE_VIRTUAL_DEFINE(daEnDfpakkun_c, Disappear);
STATE_VIRTUAL_DEFINE(daEnDfpakkun_c, DieVanish);
STATE_VIRTUAL_DEFINE(daEnDfpakkun_c, DieIceBreak);

/// @brief The Y angle to face for each direction. @unofficial
static const s16 l_dir_angle[2] = { DEG_TO_ANGLE(70), -DEG_TO_ANGLE(70) };

daEnDfpakkun_c::daEnDfpakkun_c() {}

daEnDfpakkun_c::~daEnDfpakkun_c() {}

void daEnDfpakkun_c::initialize() {
    mEatBehavior = EAT_TYPE_FIREBALL;
    mTimer = 1;
}

void daEnDfpakkun_c::createMdl() {
    mResFile = dResMng_c::m_instance->getRes("pakkun", "g3d/pakkun.brres");
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("pakkun");
    mModel.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC | nw4r::g3d::ScnMdl::BUFFER_RESTEV, 1);
    dActor_c::setSoftLight_Enemy(mModel);

    nw4r::g3d::AnmObjChr *obj;

    nw4r::g3d::ResAnmChr neckRes = mResFile.GetResAnmChr("dokan_fire_st");
    mNeckAnm.create(mdl, neckRes, &mAllocator);
    mNeckAnm.setAnm(mModel, neckRes, m3d::FORWARD_ONCE);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mNeckAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 0, nw4r::g3d::AnmObjChr::BIND_PARTIAL);

    nw4r::g3d::ResAnmChr bodyRes = mResFile.GetResAnmChr("dokan_fire_st");
    mAnm.create(mdl, bodyRes, &mAllocator);
    mAnm.setAnm(mModel, bodyRes, m3d::FORWARD_ONCE);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 6, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 1, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 2, nw4r::g3d::AnmObjChr::BIND_PARTIAL);

    mBlendAnm.create(mdl, 2, &mAllocator);
    mBlendAnm.attach(0, &mNeckAnm, 1.0f);
    mBlendAnm.attach(1, &mAnm, 1.0f);
    mModel.setAnm(mBlendAnm, 1.0f);
}

/// @brief The speed sign for each direction. @unofficial
static const float l_move_dir_sign[2] = { 1.0f, -1.0f };

void daEnDfpakkun_c::setMoveSpeed(int reverse) {
    static const mVec2_c cs_move_speed[4] = {
        mVec2_c(0.0f, 1.0f),
        mVec2_c(0.0f, -1.0f),
        mVec2_c(1.0f, 0.0f),
        mVec2_c(-1.0f, 0.0f)
    };

    mSpeed.x = l_move_dir_sign[reverse] * cs_move_speed[mPakkunDir].x;
    mSpeed.y = l_move_dir_sign[reverse] * cs_move_speed[mPakkunDir].y;
    mSpeed.z = 0.0f;
}

void daEnDfpakkun_c::setMoveAnm(float blendFrame) {
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("pakkun");
    nw4r::g3d::AnmObjChr *obj;

    mNeckAnm.setAnm(mModel, mResFile.GetResAnmChr("dokan_fire_st"), m3d::FORWARD_ONCE);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mNeckAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 0, nw4r::g3d::AnmObjChr::BIND_PARTIAL);

    mAnm.setAnm(mModel, mResFile.GetResAnmChr("dokan_fire_st"), m3d::FORWARD_ONCE);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 6, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 1, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 2, nw4r::g3d::AnmObjChr::BIND_PARTIAL);

    mBlendAnm.attach(0, &mNeckAnm, 1.0f);
    mBlendAnm.attach(1, &mAnm, 1.0f);
    mModel.setAnm(mBlendAnm, blendFrame);
}

void daEnDfpakkun_c::setSearchAnm(float startFrame, float blendFrame) {
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("pakkun");
    nw4r::g3d::AnmObjChr *obj;

    mNeckAnm.setAnm(mModel, mResFile.GetResAnmChr("dokan_fire_neck"), m3d::FORWARD_ONCE);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mNeckAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 0, nw4r::g3d::AnmObjChr::BIND_ONE);
    obj->Bind(mdl, 3, nw4r::g3d::AnmObjChr::BIND_ONE);
    obj->Bind(mdl, 4, nw4r::g3d::AnmObjChr::BIND_ONE);
    obj->Bind(mdl, 5, nw4r::g3d::AnmObjChr::BIND_ONE);
    mNeckAnm.setFrame(startFrame);

    mAnm.setAnm(mModel, mResFile.GetResAnmChr("dokan_fire_wait"), m3d::FORWARD_LOOP);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 6, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 1, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 2, nw4r::g3d::AnmObjChr::BIND_PARTIAL);

    mBlendAnm.attach(0, &mNeckAnm, 1.0f);
    mBlendAnm.attach(1, &mAnm, 1.0f);
    mModel.setAnm(mBlendAnm, blendFrame);
}

void daEnDfpakkun_c::setAttackAnm(float startFrame, float blendFrame) {
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("pakkun");
    nw4r::g3d::AnmObjChr *obj;

    mNeckAnm.setAnm(mModel, mResFile.GetResAnmChr("dokan_fire_neck"), m3d::FORWARD_ONCE);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mNeckAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 0, nw4r::g3d::AnmObjChr::BIND_ONE);
    obj->Bind(mdl, 3, nw4r::g3d::AnmObjChr::BIND_ONE);
    obj->Bind(mdl, 4, nw4r::g3d::AnmObjChr::BIND_ONE);
    obj->Bind(mdl, 5, nw4r::g3d::AnmObjChr::BIND_ONE);
    mNeckAnm.setFrame(startFrame);

    mAnm.setAnm(mModel, mResFile.GetResAnmChr("dokan_fire_attack"), m3d::FORWARD_ONCE);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 6, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 1, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 2, nw4r::g3d::AnmObjChr::BIND_PARTIAL);

    mBlendAnm.attach(0, &mNeckAnm, 1.0f);
    mBlendAnm.attach(1, &mAnm, 1.0f);
    mModel.setAnm(mBlendAnm, blendFrame);
}

void daEnDfpakkun_c::setEatAnm(float blendFrame) {
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("pakkun");
    nw4r::g3d::AnmObjChr *obj;

    mNeckAnm.setAnm(mModel, mResFile.GetResAnmChr("dokan_attack"), m3d::FORWARD_ONCE);
    mNeckAnm.setFrame(0.0f);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mNeckAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 0, nw4r::g3d::AnmObjChr::BIND_ONE);
    obj->Bind(mdl, 3, nw4r::g3d::AnmObjChr::BIND_ONE);
    obj->Bind(mdl, 4, nw4r::g3d::AnmObjChr::BIND_ONE);
    obj->Bind(mdl, 5, nw4r::g3d::AnmObjChr::BIND_ONE);

    mAnm.setAnm(mModel, mResFile.GetResAnmChr("dokan_attack"), m3d::FORWARD_ONCE);
    mAnm.setFrame(0.0f);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 6, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 1, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 2, nw4r::g3d::AnmObjChr::BIND_PARTIAL);

    mBlendAnm.attach(0, &mNeckAnm, 1.0f);
    mBlendAnm.attach(1, &mAnm, 1.0f);
    mModel.setAnm(mBlendAnm, blendFrame);
}

void daEnDfpakkun_c::setIceAnm() {
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("pakkun");
    nw4r::g3d::AnmObjChr *obj;

    mNeckAnm.setAnm(mModel, mResFile.GetResAnmChr("dokan_freeze"), m3d::FORWARD_ONCE);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mNeckAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 0, nw4r::g3d::AnmObjChr::BIND_PARTIAL);

    mAnm.setAnm(mModel, mResFile.GetResAnmChr("dokan_freeze"), m3d::FORWARD_ONCE);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 6, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 1, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 2, nw4r::g3d::AnmObjChr::BIND_PARTIAL);

    mBlendAnm.attach(0, &mNeckAnm, 1.0f);
    mBlendAnm.attach(1, &mAnm, 1.0f);
    mModel.setAnm(mBlendAnm, 0.0f);

    for (int i = 0; i < 9; i++) {
        mNeckAngle[i] = 0;
    }
}

void daEnDfpakkun_c::setIceBreakAnm() {
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("pakkun");
    nw4r::g3d::AnmObjChr *obj;

    mNeckAnm.setAnm(mModel, mResFile.GetResAnmChr("dead_ice"), m3d::FORWARD_LOOP);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mNeckAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 0, nw4r::g3d::AnmObjChr::BIND_PARTIAL);

    mAnm.setAnm(mModel, mResFile.GetResAnmChr("dead_ice"), m3d::FORWARD_LOOP);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 6, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 1, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 2, nw4r::g3d::AnmObjChr::BIND_PARTIAL);

    mBlendAnm.attach(0, &mNeckAnm, 1.0f);
    mBlendAnm.attach(1, &mAnm, 1.0f);
    mModel.setAnm(mBlendAnm, 0.0f);

    for (int i = 0; i < 9; i++) {
        mNeckAngle[i] = 0;
    }
}

void daEnDfpakkun_c::setVanishAnm() {
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("pakkun");
    nw4r::g3d::AnmObjChr *obj;

    mNeckAnm.setAnm(mModel, mResFile.GetResAnmChr("dead_leaf"), m3d::FORWARD_ONCE);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mNeckAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 0, nw4r::g3d::AnmObjChr::BIND_PARTIAL);

    mAnm.setAnm(mModel, mResFile.GetResAnmChr("dead_leaf"), m3d::FORWARD_ONCE);
    obj = static_cast<nw4r::g3d::AnmObjChr *>(mAnm.getObj());
    obj->Release();
    obj->Bind(mdl, 6, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 1, nw4r::g3d::AnmObjChr::BIND_PARTIAL);
    obj->Bind(mdl, 2, nw4r::g3d::AnmObjChr::BIND_PARTIAL);

    mBlendAnm.attach(0, &mNeckAnm, 1.0f);
    mBlendAnm.attach(1, &mAnm, 1.0f);
    mModel.setAnm(mBlendAnm, 0.0f);

    for (int i = 0; i < 9; i++) {
        mNeckAngle[i] = 0;
    }
}

void daEnDfpakkun_c::returnAnm_Ice() {
    setMoveAnm(0.0f);
    calcMdl();

    if (mStateMgr.getMainStateID()->isEqual(StateID_Attack)) {
        initializeState_Attack();
    }
}

/// @brief Bends the neck towards one of four preset frames.
/// @return Whether the neck has reached the target frame.
BOOL daEnDfpakkun_c::neckChase(int target) {
    static const float cs_dest_frame[4] = { 0.0f, 90.0f, 10.0f, 80.0f };

    BOOL done = sLib::chase(&mNeckFrame, cs_dest_frame[target], 3.0f);
    mNeckAnm.setFrame(mNeckFrame);
    return done;
}

/// @brief Spawns a fireball out of the plant's mouth.
/// @note The parameter word is built here rather than by #calcFirePrm, which
/// nothing in the game calls. Building @p pos with the three-float constructor
/// is load-bearing: the copy constructor goes through @p set(), whose
/// right-to-left argument evaluation assigns f0 to z where the original assigns
/// it to x.
void daEnDfpakkun_c::fireSet() {
    u32 param = (mDirection << 8) | (mSearchResult << 4) | mPakkunDir;
    mVec3_c pos(mFirePos.x, mFirePos.y, mFirePos.z);
    dActor_c::construct(fProfile::PAKKUN_FIREBALL, param, &pos, nullptr, 0);
}

void daEnDfpakkun_c::initializeState_DieVanish() {
    setVanishAnm();
    removeCc();
    mBc.mFlags = 0;
    mAngle.y = 0;
}

void daEnDfpakkun_c::finalizeState_DieVanish() {}

void daEnDfpakkun_c::executeState_DieVanish() {
    mAnm.play();
    mNeckAnm.play();
    mModel.play();

    if (mAnm.isStop() || mNeckAnm.isStop()) {
        deleteActor(1);
    }
}

void daEnDfpakkun_c::initializeState_DieIceBreak() {
    daEnDpakkunBase_c::initializeState_DieIceBreak();
}

void daEnDfpakkun_c::finalizeState_DieIceBreak() {}

void daEnDfpakkun_c::executeState_DieIceBreak() {
    mAnm.play();
    mNeckAnm.play();
    daEnDpakkunBase_c::executeState_DieIceBreak();
}

void daEnDfpakkun_c::initializeState_Wait() {
    removeCc();
    mPos = mStartPos;
    mAttacking = 0;
}

void daEnDfpakkun_c::finalizeState_Wait() {}

void daEnDfpakkun_c::executeState_Wait() {
    if (isPlayerDemo()) {
        mTimer = 72;
    }

    if (--mTimer <= 0 && checkAppear()) {
        reviveCc();
        changeState(StateID_Appear);
    }
}

void daEnDfpakkun_c::initializeState_Appear() {
    setMoveAnm(0.0f);
    mSearchDir = searchDir();
    mTimer = 32;
    mAngle.y = 0;
    setMoveSpeed(0);
}

void daEnDfpakkun_c::finalizeState_Appear() {}

void daEnDfpakkun_c::executeState_Appear() {
    mAnm.play();
    mNeckAnm.play();
    mModel.play();
    posMove();

    mDirection = searchDir();

    mTimer--;
    if (mTimer <= 18) {
        mAttacking = 1;
    }

    if (mTimer <= 0) {
        changeState(StateID_Attack);
    } else if (mTimer >= 4) {
        mAngle.y += l_EnMuki[mSearchDir] * 0x924;
    } else {
        sLib::chaseAngle(&mAngle.y.mAngle, l_dir_angle[mDirection], 0x924);
    }
}

void daEnDfpakkun_c::initializeState_Attack() {
    static const int cs_shoot_num[4] = { 1, 2, 3, 6 };

    int blendFrame = 70;
    setSearchAnm(90.0f, blendFrame);
    mNeckAnm.setRate(0.0f);
    mSpeed.x = 0.0f;
    mSpeed.y = 0.0f;
    mSpeed.z = 0.0f;
    mTimer = 20;

    if (*mStateMgr.getOldStateID() != dEn_c::StateID_Ice) {
        mShotsLeft = cs_shoot_num[mParam & 3];
    }

    mNeckAngle[6] = 0;
    m_23b = 1;
}

void daEnDfpakkun_c::finalizeState_Attack() {}

void daEnDfpakkun_c::executeState_Attack() {
    mAnm.play();
    mNeckAnm.play();
    mModel.play();

    mAttacking = 1;

    switch (m_23b) {
        case 1: {
            mDirection = searchDir();
            sLib::chaseAngle(&mAngle.y.mAngle, l_dir_angle[mDirection], 0x600);
            if (--mTimer <= 0) {
                mNeckFrame = mNeckAnm.getFrame();
                mTimer = 60;
                m_23b = 2;
            }
            break;
        }

        case 2: {
            mDirection = searchDir();
            sLib::chaseAngle(&mAngle.y.mAngle, l_dir_angle[mDirection], 0x600);
            mSearchResult = search();
            if (mSearchResult >= 0) {
                neckChase(mSearchResult);
                if (--mTimer <= 0) {
                    m_23b = 3;
                } else if (mTimer < 40) {
                    adjustNeck();
                }
            }
            break;
        }

        case 3: {
            mDirection = searchDir();
            BOOL angleDone = sLib::chaseAngle(&mAngle.y.mAngle, l_dir_angle[mDirection], 0x600);
            BOOL neckDone = neckChase(mSearchResult);
            adjustNeck();
            if (neckDone && angleDone) {
                adjustNeck();
                mTimer = 5;
                m_23b = 4;
            }
            break;
        }

        case 4: {
            BOOL neckReady = adjustNeck();
            if (--mTimer <= 0 && neckReady) {
                setAttackAnm(mNeckAnm.getFrame(), 0.0f);
                mNeckAnm.setRate(0.0f);
                dAudio::g_pSndObjEmy->startSound(SE_EMY_PAKKUN_FIRE, mPos, 0);
                fireSet();
                m_23b = 5;
            }
            break;
        }

        case 5: {
            if (mAnm.isStop()) {
                if (--mShotsLeft > 0) {
                    setSearchAnm(mNeckAnm.getFrame(), 0.0f);
                    mNeckAnm.setRate(0.0f);
                    mTimer = 5;
                    m_23b = 2;
                } else {
                    int blendFrame = 70;
                    setMoveAnm(blendFrame);
                    mNeckAnm.setRate(0.0f);
                    mTimer = 20;
                    m_23b = 6;
                }
            }
            break;
        }

        case 6: {
            sLib::chaseAngle(&mNeckAngle[6], 0, 0x200);
            if (--mTimer <= 0) {
                mNeckAnm.setRate(1.0f);
                mAnm.setRate(1.0f);
                changeState(StateID_Disappear);
            }
            break;
        }
    }
}

int daEnDfpakkun_c::search() {
    return -1;
}

bool daEnDfpakkun_c::adjustNeck() {
    return true;
}

void daEnDfpakkun_c::initializeState_Disappear() {
    if (mAngle.y >= 0) {
        mDirection = 1;
    } else {
        mDirection = 0;
    }

    mTimer = 32;
    setMoveSpeed(1);
}

void daEnDfpakkun_c::finalizeState_Disappear() {}

void daEnDfpakkun_c::executeState_Disappear() {
    mAnm.play();
    mNeckAnm.play();
    mModel.play();
    mModel.play();
    posMove();

    sLib::chaseAngle(&mNeckAngle[6], 0, 0x200);
    mAngle.y += l_EnMuki[mDirection] * 0x924;

    mTimer--;
    if (mTimer <= 18) {
        mAttacking = 0;
    }

    if (mTimer <= 0) {
        mTimer = 72;
        changeState(StateID_Wait);
    }
}
