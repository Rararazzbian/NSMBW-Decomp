#include <game/bases/d_a_en_kuribo_base.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_bg_ctr.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/sLib/s_lib.hpp>
#include <constants/sound_list.h>

const s16 daEnKuriboBase_c::smc_TURN_SPEED = 0x200;
const float daEnKuriboBase_c::smc_MAX_XSPEED = 1.0f;
const float daEnKuriboBase_c::smc_MAX_YSPEED = -4.0f;

static const sCcDatNewF l_kuribo_cc = {
    0.0f, 8.0f,
    8.0f, 8.0f,
    CC_KIND_ENEMY,
    CC_ATTACK_NONE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) |
        BIT_FLAG(CC_KIND_YOSHI) | BIT_FLAG(CC_KIND_ENEMY) |
        BIT_FLAG(CC_KIND_ITEM) | BIT_FLAG(CC_KIND_TAMA),
    (u32) ~(BIT_FLAG(CC_ATTACK_NONE) | BIT_FLAG(CC_ATTACK_YOSHI_MOUTH) | BIT_FLAG(CC_ATTACK_SPIN_LIFT_UP) | BIT_FLAG(CC_ATTACK_SAND_PILLAR)),
    CC_STATUS_NONE,
    dEn_c::normal_collcheck
};

STATE_VIRTUAL_DEFINE(daEnKuriboBase_c, Walk);
STATE_VIRTUAL_DEFINE(daEnKuriboBase_c, Turn);
STATE_VIRTUAL_DEFINE(daEnKuriboBase_c, TrplnJump);
STATE_VIRTUAL_DEFINE(daEnKuriboBase_c, DieOther);

int daEnKuriboBase_c::create() {
    createModel();

    mCc.set(this, (sCcDatNewF *) &l_kuribo_cc);
    mCc.entry();

    mSpeedMax.y = smc_MAX_YSPEED;

    mDirection = getPl_LRflag(mPos);
    mAngle.y = l_base_angleY[mDirection];
    mAmiLayer = ACTOR_PARAM(SubLayer);
    mBaseZPos = 0.0f;
    mScrOutFlags = 0;
    mFumiProc.mFumiCheck.m_00 = 0;

    initialize();

    return SUCCEEDED;
}

void daEnKuriboBase_c::initialize() {}

void daEnKuriboBase_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    createBodyModel();
    createOtherModel();

    mAllocator.adjustFrmHeap();
}

void daEnKuriboBase_c::createOtherModel() {}

void daEnKuriboBase_c::createBodyModel() {
    mResFile = dResMng_c::m_instance->getRes("kuribo", "g3d/kuribo.brres");

    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("kuribo");
    mModel.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC | nw4r::g3d::ScnMdl::ANM_TEXPAT, 1, nullptr);
    dActor_c::setSoftLight_Enemy(mModel);

    nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr("walk");
    mAnmChr.create(mdl, anm, &mAllocator, nullptr);

    mResAnmTexPat = mResFile.GetResAnmTexPat("walk");
    mAnmTexPat.create(mdl, mResAnmTexPat, &mAllocator, nullptr, 1);
}

int daEnKuriboBase_c::doDelete() {
    return SUCCEEDED;
}

int daEnKuriboBase_c::execute() {
    mStateMgr.executeState();

    if (!mNoRespawn) {
        setCcLine();

        if (!isWakidashi() && !isState(StateID_Ice)) {
            setLayerPos();
        }

        if (checkRyusa() && !isState(StateID_Ice)) {
            ryusaEffect();
        }

        if (HasamareBgCheck()) {
            setDeathInfo_Hasami();
        }
    }

    ActorScrOutCheck(mScrOutFlags);

    return SUCCEEDED;
}

bool daEnKuriboBase_c::isWakidashi() const {
    return false;
}

int daEnKuriboBase_c::draw() {
    drawModel();
    return SUCCEEDED;
}

void daEnKuriboBase_c::drawModel() {
    mModel.entry();
}

void daEnKuriboBase_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.YrotM(angle.y);
    mMatrix.concat(mMtx_c::createTrans(0.0f, mCenterOffs.y, 0.0f));
    mMatrix.XrotM(angle.x);
    mMatrix.concat(mMtx_c::createTrans(0.0f, -mCenterOffs.y, 0.0f));
    mMatrix.ZrotM(angle.z);

    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mBoyoMng.getScale());
    mModel.calc(false);
}

void daEnKuriboBase_c::Normal_VsEnHitCheck(dCc_c *self, dCc_c *other) {
    float xOfs = self->getXOffset(CC_KIND_ENEMY);
    if ((mDirection == 1 && xOfs > 0.0f) || (mDirection == 0 && xOfs < 0.0f)) {
        setTurnByEnemyHit();
    }
}

void daEnKuriboBase_c::setTurnByEnemyHit() {}

void daEnKuriboBase_c::Normal_VsPlHitCheck(dCc_c *self, dCc_c *other) {
    dActor_c *actor = other->getOwner();

    switch (Enfumi_check(self, other, 0)) {
        case 1:
            reactFumiProc(actor);
            break;

        case 3:
            reactSpinFumiProc(actor);
            break;

        case 0:
            if (!isDamageInvalid()) {
                dEn_c::Normal_VsPlHitCheck(self, other);
            }
            break;
    }
}

void daEnKuriboBase_c::reactSpinFumiProc(dActor_c *actor) {
    setDeathInfo_SpinFumi(actor, 1);
}

bool daEnKuriboBase_c::isDamageInvalid() {
    return false;
}

void daEnKuriboBase_c::Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other) {
    dActor_c *actor = other->getOwner();

    switch (Enfumi_check(self, other, 0)) {
        case 1:
            reactYoshiFumiProc(actor);
            break;

        case 0:
            if (!isDamageInvalid()) {
                dEn_c::Normal_VsYoshiHitCheck(self, other);
            }
            break;
    }
}

void daEnKuriboBase_c::reactYoshiFumiProc(dActor_c *actor) {
    setDeathInfo_YoshiFumi(actor);
}

void daEnKuriboBase_c::reactFumiProc(dActor_c *actor) {
    float ySpeed = mSpeed.y;
    float xSpeed = mSpeed.x;
    setDeathInfo_Fumi(actor, mVec2_c(xSpeed, ySpeed), dEn_c::StateID_DieOther, 0);
}

void daEnKuriboBase_c::reactMameFumiProc(dActor_c *actor) {}

bool daEnKuriboBase_c::checkRyusa() {
    if (mBc.isFoot()) {
        mVec3_c center = getCenterPos();
        u32 unitType = dBc_c::getUnitType(center.x, center.y, mLayer);
        u32 unitKind = dBc_c::getUnitKind(center.x, center.y, mLayer);
        u8 v = unitKind >> 16;
        if (unitType & 0x8000 && v == 3) {
            return true;
        }
    }
    return false;
}

void daEnKuriboBase_c::ryusaEffect() {
    mVec3_c center = getCenterPos();
    mVec3_c efPos(center.x, center.y, 5500.0f);
    mQuicksandEffect.createEffect("Wm_en_quicksand", 0, &efPos, nullptr, nullptr);
}

bool daEnKuriboBase_c::setDamage(dActor_c *actor) {
    dAcPy_c *player = (dAcPy_c *) actor;
    if (player->setDamage(this, daPlBase_c::DAMAGE_DEFAULT)) {
        setTurnByPlayerHit(actor);
        return true;
    }
    return false;
}

void daEnKuriboBase_c::setLayerPos() {
    if (mLayer == 0) {
        mPos.z = mBaseZPos + l_Ami_Zpos[mAmiLayer];
    } else {
        mPos.z = -2500.0f;
    }
}

void daEnKuriboBase_c::setDeathInfo_Hasami() {
    u8 dir = !(mPos.x - mLastPos.x >= 0.0f);

    mVec2_c pos2D(mPos.x, mPos.y);
    hitdamageEffect(mVec3_c(pos2D, 5500.0f));

    dAudio::g_pSndObjEmy->startSound(SE_EMY_DOWN, mPos, 0);

    mDeathInfo = (sDeathInfoData) {
        l_base_fall_speed_x[dir],
        smc_DEADFALL_YSPEED,
        smc_DEADFALL_YSPEED_MAX,
        smc_DEADFALL_GRAVITY,
        &StateID_DieFall,
        -1,
        -1,
        dir,
        0xFF
    };
}

void daEnKuriboBase_c::setCcLine() {
    float sizeX = mCc.mCcData.mBase.mSize.x;
    float ofs = sizeX + 3.0f;
    float offsX = mCc.mCcData.mBase.mOffset.x;
    float offsY = mCc.mCcData.mBase.mOffset.y;
    float x = mPos.x + offsX;
    float y = mPos.y + offsY;

    u32 typeR = dBc_c::getUnitType(x + ofs, y, mLayer);
    u32 kindR = (u8) dBc_c::getUnitKind(x + ofs, y, mLayer);
    u32 typeL = dBc_c::getUnitType(x - ofs, y, mLayer);
    u32 kindL = (u8) dBc_c::getUnitKind(x - ofs, y, mLayer);

    if ((typeR & 0x400 && kindR >= 2) || (typeL & 0x400 && kindL >= 2)) {
        if (mAmiLayer == 0) {
            mCc.mAmiLine = 1;
        } else {
            mCc.mAmiLine = 2;
        }
    } else {
        mCc.mAmiLine = 3;
    }
}

void daEnKuriboBase_c::setWalkAnm() {
    nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr("walk");
    mAnmChr.setAnm(mModel, anm, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnmChr, 2.0f);
    mAnmChr.setRate(2.0f);

    mModel.setAnm(mAnmTexPat, 0.0f);
    mAnmTexPat.setRate(1.0f, 0);
    mAnmTexPat.setPlayMode(m3d::FORWARD_LOOP, 0);
    mAnmTexPat.setFrame((u32) dGameCom::rndInt(180), 0);
}

void daEnKuriboBase_c::setTurnByPlayerHit(dActor_c *actor) {
    mDirection = getTrgToSrcDir_Main(actor->getCenterX(), getCenterX());

    if (isState(StateID_Turn)) {
        changeState(StateID_Walk);
    }

    mAngle.y = l_base_angleY[mDirection];
    setWalkSpeed();
}

void daEnKuriboBase_c::setWalkSpeed() {}

void daEnKuriboBase_c::landonEffect() {
    mVec3_c landPos(mPos.x, mPos.y, 5500.0f);
    u32 unitType = dBc_c::getUnitType(landPos.x, landPos.y - 8.0f, mLayer);
    u32 unitKind = dBc_c::getUnitKind(landPos.x, landPos.y - 8.0f, mLayer);
    u32 v = (u8) (unitKind >> 16);
    u16 footAttr = mBc.getFootAttr();
    if (footAttr == 12 || unitType & 0x8000 && v == 3) {
        mEf::createEffect("Wm_en_sndlandsmk_s", 0, &landPos, nullptr, nullptr);
    } else {
        mEf::createEffect("Wm_en_landsmoke_s", 0, &landPos, nullptr, nullptr);
    }
}

bool daEnKuriboBase_c::isOnTrampoline() {
    dBg_ctr_c *ctr = mBc.mpCtrHead;
    if (ctr == nullptr) {
        return false;
    }

    dActor_c *actor = ctr->mpActor;
    if (actor == nullptr) {
        return false;
    }

    return actor->mProfName == fProfile::EN_LIFT_REMOCON_TRPLN;
}

void daEnKuriboBase_c::initializeState_Walk() {
    if (*mStateMgr.getOldStateID() != StateID_Turn) {
        setWalkAnm();
    }

    setWalkSpeed();

    mAccelY = -0.1875f;
    mSpeedMax.set(0.0f, smc_MAX_YSPEED, 0.0f);
}

void daEnKuriboBase_c::finalizeState_Walk() {}

void daEnKuriboBase_c::executeState_Walk() {
    playWalkAnm();
    calcSpeedY();
    posMove();
    sLib::chaseAngle(&mAngle.y.mAngle, l_base_angleY[mDirection], smc_TURN_SPEED);
    walkEffect();

    u32 prevFoot = mBc.isFoot();
    u32 bgCheckRes = EnBgCheck();

    if (bgCheckRes & 1) {
        mFootPush2.x = 0.0f;
        mSpeed.y = 0.0f;

        if (isOnTrampoline()) {
            changeState(StateID_TrplnJump);
            return;
        }

        if (isBgmSync() && dAudio::isBgmAccentSign(1)) {
            mSpeed.y = 2.0f;
        }
    } else if (prevFoot && !mInLiquid && mSpeed.y <= 0.0f) {
        mFootPush2.x += m_1eb.x;
    }

    if (mBc.isWall(mDirection)) {
        changeState(StateID_Turn);
    }

    WaterCheck(mPos, 1.0f);
}

void daEnKuriboBase_c::playWalkAnm() {
    mModel.play();
    mAnmTexPat.play();
}

void daEnKuriboBase_c::walkEffect() {}

BOOL daEnKuriboBase_c::isBgmSync() const {
    return TRUE;
}

void daEnKuriboBase_c::initializeState_Turn() {
    if (*mStateMgr.getOldStateID() != StateID_TrplnJump) {
        mSpeed.x = 0.0f;
        mDirection ^= 1;
    }
}

void daEnKuriboBase_c::finalizeState_Turn() {}

void daEnKuriboBase_c::executeState_Turn() {
    playWalkAnm();
    calcSpeedY();
    posMove();
    walkEffect();

    u32 prevFoot = mBc.isFoot();
    u32 bgCheckRes = EnBgCheck();

    if (bgCheckRes & 1) {
        mSpeed.y = 0.0f;

        if (isOnTrampoline()) {
            changeState(StateID_TrplnJump);
            return;
        }

        if (isBgmSync() && dAudio::isBgmAccentSign(1)) {
            mSpeed.y = 2.0f;
        }
    } else if (prevFoot && !mInLiquid && mSpeed.y <= 0.0f) {
        mSpeed.x += m_1eb.x;
    }

    WaterCheck(mPos, 1.0f);

    if (sLib::chaseAngle(&mAngle.y.mAngle, l_base_angleY[mDirection], smc_TURN_SPEED)) {
        changeState(StateID_Walk);
    }
}

void daEnKuriboBase_c::initializeState_TrplnJump() {
    setWalkSpeed();
    mSpeed.y = 5.5f;
}

void daEnKuriboBase_c::finalizeState_TrplnJump() {}

void daEnKuriboBase_c::executeState_TrplnJump() {
    playWalkAnm();
    calcSpeedY();
    posMove();

    BOOL turned = sLib::chaseAngle(&mAngle.y.mAngle, l_base_angleY[mDirection], smc_TURN_SPEED);
    u32 bgCheckRes = EnBgCheck();

    if (bgCheckRes & 4) {
        mSpeed.x = -mSpeed.x;
        mDirection ^= 1;
    }

    if (bgCheckRes & 1) {
        mSpeed.y = 0.0f;

        if (isOnTrampoline()) {
            initializeState_TrplnJump();
        } else if (turned) {
            changeState(StateID_Walk);
        } else {
            changeState(StateID_Turn);
        }
    }

    WaterCheck(mPos, 1.0f);
}

void daEnKuriboBase_c::initializeState_DieOther() {
    nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr("damage");
    mAnmChr.setAnm(mModel, anm, m3d::FORWARD_ONCE);
    mModel.setAnm(mAnmChr, 2.0f);

    mSpeed.set(0.0f, 0.0f, 0.0f);

    removeCc();

    mAngle.y = 0;
    mTimer1 = 30;
}

void daEnKuriboBase_c::finalizeState_DieOther() {
    dEn_c::finalizeState_DieOther();
}

void daEnKuriboBase_c::executeState_DieOther() {
    mModel.play();

    if (mTimer1 == 0) {
        deleteActor(1);
    }
}

void daEnKuriboBase_c::finalUpdate() {
    calcModel();
}

BOOL daEnKuriboBase_c::isFunsui() const {
    return mIsFunsui;
}

void daEnKuriboBase_c::endFunsui() {
    mAnmChr.setRate(2.0f);
    mSpeed.x = mXSpeedBeforeFunsui;
    mIsFunsui = FALSE;
}

void daEnKuriboBase_c::beginFunsui() {
    mAnmChr.setRate(3.0f);
    mXSpeedBeforeFunsui = mSpeed.x;
    mSpeed.set(0.0f, 0.0f, 0.0f);
    mIsFunsui = TRUE;
}

void daEnKuriboBase_c::fumidamageSE(const mVec3_c &pos, int playerNo) {
    mVec3_c center = getCenterPos();
    int remote = dAudio::getRemotePlayer(playerNo);
    dAudio::SndObjctCmnEmy_c *obj = dAudio::g_pSndObjEmy;
    obj->startSound(SE_EMY_KURIBO_FUMU, center, remote);
}

daEnKuriboBase_c::~daEnKuriboBase_c() {}
