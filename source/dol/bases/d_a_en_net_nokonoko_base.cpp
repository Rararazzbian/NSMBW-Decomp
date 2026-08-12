#include <game/bases/d_a_en_net_nokonoko_base.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_enemy_manager.hpp>
#include <game/bases/d_res_mng.hpp>

extern const sCcDatNewF l_noko_cc;

const sCcDatNewF l_noko_cc = {
    0.0f, 11.0f,
    8.0f, 11.0f,
    CC_KIND_ENEMY,
    CC_ATTACK_NONE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) |
        BIT_FLAG(CC_KIND_YOSHI) | BIT_FLAG(CC_KIND_ENEMY) |
        BIT_FLAG(CC_KIND_TAMA),
    (u32) ~(BIT_FLAG(CC_ATTACK_NONE) | BIT_FLAG(CC_ATTACK_YOSHI_MOUTH) | BIT_FLAG(CC_ATTACK_SPIN_LIFT_UP) | BIT_FLAG(CC_ATTACK_SAND_PILLAR)),
    CC_STATUS_NONE,
    dEn_c::normal_collcheck
};

const sBcSensorPoint daEnNetNoko_c::smc_noko_head = { SENSOR_IS_POINT, 0, 0xc000 };
const sBcSensorPoint daEnNetNoko_c::smc_noko_foot = { SENSOR_IS_POINT, 0, 0 };
const sBcSensorPoint daEnNetNoko_c::smc_noko_wall = { SENSOR_IS_POINT, 0x3000, 0x6000 };

const float daEnNetNoko_c::smc_MOVE_SPEED[2] = { 0.5f, -0.5f };
const float daEnNetNoko_c::smc_MOVE_SPEED_HIGH[2] = { 0.75f, -0.75f };
const s16 daEnNetNoko_c::smc_ANGLE_Y[2] = { -0x8000, 0 };

STATE_VIRTUAL_DEFINE(daEnNetNoko_c, DieFumi);
STATE_VIRTUAL_DEFINE(daEnNetNoko_c, DieFall);

int daEnNetNoko_c::create() {
    createMdl();

    mColor = ACTOR_PARAM(Color);
    mIceMng.setIceStatus(0, 3, 3);

    mScale.set(1.0f, 1.0f, 1.0f);
    mCenterOffs.set(0.0f, 16.0f, 0.0f);
    mVisibleAreaOffset.set(0.0f, 16.0f);

    mBc.set(this, smc_noko_foot, smc_noko_head, smc_noko_wall);

    mCc.set(this, (sCcDatNewF *) &l_noko_cc);
    mCc.entry();

    mFumiProc.mFumiCheck.m_00 = 3;
    mAmiLayer = ACTOR_PARAM(AmiLayer);
    mAngle.y = smc_ANGLE_Y[ACTOR_PARAM(AmiLayer)];
    mFlags |= EN_IS_HARD;
    mEatBehavior = EAT_TYPE_EAT;

    setColor();

    initialize();

    mFlags = EN_IS_HARD | BIT_FLAG(2);

    entryHIO();

    return SUCCEEDED;
}

void daEnNetNoko_c::initialize() {}

void daEnNetNoko_c::entryHIO() {}

void daEnNetNoko_c::createMdl() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("nokonokoB", "g3d/nokonokoB.brres");

    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("nokonokoB");
    mModel.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC | nw4r::g3d::ScnMdl::ANM_TEXPAT);

    dActor_c::setSoftLight_Enemy(mModel);

    nw4r::g3d::ResAnmChr anim = mResFile.GetResAnmChr("net_walk1");
    mAnim.create(mdl, anim, &mAllocator);

    mResAnmTexPat = mResFile.GetResAnmTexPat("nokonokoB");
    mAnimTex.create(mdl, mResAnmTexPat, &mAllocator);
    mAnimTex.setAnm(mModel, mResAnmTexPat, 0, m3d::FORWARD_ONCE);

    mAllocator.adjustFrmHeap();
}

int daEnNetNoko_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

void daEnNetNoko_c::deleteReady() {}

int daEnNetNoko_c::doDelete() {
    removeHIO();
    return SUCCEEDED;
}

void daEnNetNoko_c::removeHIO() {}

void daEnNetNoko_c::calcMdl() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.YrotM(angle.y);
    mMatrix.concat(mMtx_c::createTrans(0.0f, 16.0f, 0.0f));
    mMatrix.XrotM(angle.x);
    mMatrix.concat(mMtx_c::createTrans(0.0f, -16.0f, 0.0f));
    mMatrix.ZrotM(angle.z);

    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daEnNetNoko_c::setColor() {
    float frame = mColor;
    mAnimTex.setPlayMode(m3d::FORWARD_ONCE, 0);
    mModel.setAnm(mAnimTex, 0.0f);
    mAnimTex.setFrame(frame, 0);
    mAnimTex.setRate(0.0f, 0);
}

int daEnNetNoko_c::wireBgCheck(const mVec2_c &offset) {
    mVec3_c center = getCenterPos();
    float x = center.x + offset.x;
    float y = center.y + offset.y;

    if (dBc_c::getUnitType(x, y, mLayer) & 0x400) {
        u8 kind = dBc_c::getUnitKind(x, y, mLayer);
        if (kind >= 2) {
            return kind;
        }
    }

    return -1;
}

bool daEnNetNoko_c::PlDamageCheck(dCc_c *self, dCc_c *other) {
    if (other->mCcData.mAttack != CC_ATTACK_WIRE_NET &&
        (self->mAmiLine & ((dAcPy_c *) other->getOwner())->mAmiLayer) == 0) {
        return true;
    }

    return dEn_c::PlDamageCheck(self, other);
}

bool daEnNetNoko_c::setEatSpitOut(dActor_c *eatingActor) {
    calcSpitOutPos(eatingActor);

    u32 param = (eatingActor->getPlrNo() << 24) | 0x20000000;
    mVec3_c pos = mPos;
    dActor_c *noko = dActor_c::construct(fProfile::EN_NOKONOKO, param, &pos, nullptr, 0);

    if (noko != nullptr) {
        noko->mDirection = eatingActor->mDirection;
        noko->mEatenByID = mEatenByID;
    }

    deleteRequest();

    return true;
}

void daEnNetNoko_c::MameFumiJumpSet(dActor_c *actor) {
    float jumpSpeed = (dAcPy_c::msc_JUMP_SPEED + 0.2815f);
    jumpSpeed *= 0.8125f;
    float speedF = actor->mSpeedF;
    ((daPlBase_c *) actor)->_setJump(jumpSpeed, speedF, true, 0, 2);
    dEnemyMng_c::m_instance->m_138 = 1;
}

void daEnNetNoko_c::Normal_VsPlHitCheck(dCc_c *self, dCc_c *other) {
    dActor_c *player = other->getOwner();

    int fumiRes = Enfumi_check(self, other, 0);
    if (fumiRes == 0) {
        dEn_c::Normal_VsPlHitCheck(self, other);
    } else if (fumiRes == 1) {
        setDeathInfo_Fumi(player, mVec2_c(0.0f, 0.0f), dEn_c::StateID_DieFumi, 1);
    } else if (fumiRes == 3) {
        setDeathInfo_SpinFumi(player, 1);
    }
}

void daEnNetNoko_c::Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other) {
    dActor_c *yoshi = other->getOwner();

    int fumiRes = Enfumi_check(self, other, 0);
    if (fumiRes == 0) {
        dEn_c::Normal_VsYoshiHitCheck(self, other);
    } else if (fumiRes == 1) {
        setDeathInfo_YoshiFumi(yoshi);
    }
}

void daEnNetNoko_c::initializeState_DieFumi() {
    mAnim.setAnm(mModel, mResFile.GetResAnmChr("damage"), m3d::FORWARD_LOOP);
    mModel.setAnm(mAnim, 1.0f);

    s16 angleY = mAngle.y;
    dEn_c::initializeState_DieFall();
    mAngle.y = angleY;
}

void daEnNetNoko_c::finalizeState_DieFumi() {}

void daEnNetNoko_c::executeState_DieFumi() {
    static const s16 cs_spin_speed[2] = { 0x100, -0x100 };

    mModel.play();

    mAngle.x -= 0x300;
    if (mDirection == mIceDeathDirection) {
        mAngle.y -= cs_spin_speed[mIceDeathDirection];
    } else {
        mAngle.y += cs_spin_speed[mIceDeathDirection];
    }

    calcSpeedY();
    posMove();

    if (mAmiLayer == 0) {
        WaterCheck(mPos, 1.0f);
    }
}

void daEnNetNoko_c::initializeState_DieFall() {
    mAnim.setAnm(mModel, mResFile.GetResAnmChr("damage"), m3d::FORWARD_LOOP);
    mModel.setAnm(mAnim, 1.0f);

    s16 angleY = mAngle.y;
    dEn_c::initializeState_DieFall();
    mAngle.y = angleY;

    mSpeed.x *= 0.75f;

    if (mAmiLayer == 1) {
        mSpeed.z = -0.25f;
    } else {
        mSpeed.z = 0.25f;
    }
}

void daEnNetNoko_c::finalizeState_DieFall() {}

void daEnNetNoko_c::executeState_DieFall() {
    static const s16 cs_spin_speed[2] = { 0xc0, -0xc0 };

    mModel.play();

    mAngle.x -= 0x600;
    if (mDirection == mIceDeathDirection) {
        mAngle.y -= cs_spin_speed[mIceDeathDirection];
    } else {
        mAngle.y += cs_spin_speed[mIceDeathDirection];
    }

    float scaleDelta = 0.0015f;
    float scaleRange = 0.1f;
    if (mAmiLayer == 1) {
        mScale.x -= scaleDelta;
        mScale.y -= scaleDelta;
        mScale.z -= scaleDelta;
        if (mScale.x < 1.0f - scaleRange) {
            mScale.set(1.0f - scaleRange, 1.0f - scaleRange, 1.0f - scaleRange);
        }
    } else {
        mScale.x += scaleDelta;
        mScale.y += scaleDelta;
        mScale.z += scaleDelta;
        if (mScale.x > 1.0f + scaleRange) {
            mScale.set(1.0f + scaleRange, 1.0f + scaleRange, 1.0f + scaleRange);
        }
    }

    calcSpeedY();
    dBaseActor_c::posMove();

    if (mAmiLayer == 0) {
        WaterCheck(mPos, 1.0f);
    }
}

void daEnNetNoko_c::mdlPlay() {
    mModel.play();
}

BOOL daEnNetNoko_c::isQuakeDamage() {
    return TRUE;
}

daEnNetNoko_c::~daEnNetNoko_c() {}
