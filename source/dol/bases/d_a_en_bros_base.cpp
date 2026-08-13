#include <game/bases/d_a_en_bros_base.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_actor_manager.hpp>
#include <game/bases/d_cc.hpp>
#include <game/bases/d_ef.hpp>
#include <game/bases/d_ice_manager.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/mLib/m_heap.hpp>
#include <game/sLib/s_lib.hpp>

/// @brief The foot sensor. @unofficial
extern const sBcSensorPoint l_bros_foot;

/// @brief The wall sensor. @unofficial
extern const sBcSensorPoint l_bros_wall;

/// @brief The collider template. @unofficial
extern const sCcDatNewF l_bros_cc;

const sBcSensorPoint l_bros_foot = { SENSOR_IS_POINT, 0, 0 };
const sBcSensorPoint l_bros_wall = { SENSOR_IS_POINT, 0x8000, 0x8000 };

const sCcDatNewF l_bros_cc = {
    0.0f, 16.0f,
    10.0f, 16.0f,
    CC_KIND_ENEMY,
    CC_ATTACK_NONE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) |
        BIT_FLAG(CC_KIND_YOSHI) | BIT_FLAG(CC_KIND_ENEMY) |
        BIT_FLAG(CC_KIND_ITEM) | BIT_FLAG(CC_KIND_TAMA),
    (u32) ~(BIT_FLAG(CC_ATTACK_NONE) | BIT_FLAG(CC_ATTACK_YOSHI_MOUTH) |
        BIT_FLAG(CC_ATTACK_SPIN_LIFT_UP) | BIT_FLAG(CC_ATTACK_SAND_PILLAR)),
    CC_STATUS_NONE,
    dEn_c::normal_collcheck
};

STATE_VIRTUAL_DEFINE(daEnBrosBase_c, Move);
STATE_VIRTUAL_DEFINE(daEnBrosBase_c, Attack);
STATE_VIRTUAL_DEFINE(daEnBrosBase_c, JumpSt);
STATE_VIRTUAL_DEFINE(daEnBrosBase_c, Jump);
STATE_VIRTUAL_DEFINE(daEnBrosBase_c, JumpEd);
STATE_VIRTUAL_DEFINE(daEnBrosBase_c, AirAttack);
STATE_VIRTUAL_DEFINE(daEnBrosBase_c, DieFumi);
STATE_VIRTUAL_DEFINE(daEnBrosBase_c, DieFall);


// #7 0x80023CC0 (580 B)
int daEnBrosBase_c::create() {
    createMdl();

    mDirection = getPl_LRflag(mPos);
    mMoveDir = mDirection;
    mAngle.y = l_base_angleY[mDirection];
    mFlags = EN_IS_HARD;

    mModel.setCallback(&mNodeCallback);

    mEatBehavior = 0;

    mCenterOffs.set(0.0f, 16.0f, 0.0f);
    mScale.set(1.0f, 1.0f, 1.0f);

    mBc.set(this, l_bros_foot, nullptr, l_bros_wall);

    mCc.set(this, (sCcDatNewF *) &l_bros_cc);
    mCc.entry();

    mVisibleAreaOffset.set(0.0f, 14.0f);
    mVisibleAreaSize.set(16.0f, 40.0f);
    mMaxBound.mOffset.set(smc_CULL_XLIMIT, smc_CULL_YLIMIT);

    mAnmTexPat.setPlayMode(m3d::FORWARD_ONCE, 0);
    mModel.setAnm(mAnmTexPat, 0.0f);
    mAnmTexPat.setFrame(getColor(), 0);
    mAnmTexPat.setRate(0.0f, 0);

    mCombo.mType = dEnCombo_c::COMBO_SHORT;

    initType();
    initPosLv();
    initMoveCnt();

    mMoveRangeFront = 16.0f;
    mMoveRangeBack = 16.0f;

    setSpeed();

    mHomePos = mPos;

    setJumpCnt();
    mJumpCnt >>= 1;

    changeState(StateID_Attack);
    entryHIO();

    return SUCCEEDED;
}

// #8 0x80023F10
int daEnBrosBase_c::getColor() {
    return -1;
}

// #9 0x80023F20
void daEnBrosBase_c::initType() {}

// #10 0x80023F30
void daEnBrosBase_c::initPosLv() {}

// #11 0x80023F40
void daEnBrosBase_c::initMoveCnt() {
    mMoveCnt = 0;
}

// #12 0x80023F50
void daEnBrosBase_c::setSpeed() {}

// #13 0x80023F60
void daEnBrosBase_c::setJumpCnt() {}

// #14 0x80023F70
void daEnBrosBase_c::entryHIO() {}

// #15 0x80023F80 (296 B)
void daEnBrosBase_c::createMdl() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("bros", "g3d/bros.brres");

    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("bros");
    mModel.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC | nw4r::g3d::ScnMdl::ANM_TEXPAT, 1);
    dActor_c::setSoftLight_Enemy(mModel);

    nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr("walk");
    mAnmChr.create(mdl, anm, &mAllocator);

    mResAnmTexPat = mResFile.GetResAnmTexPat("bros");
    mAnmTexPat.create(mdl, mResAnmTexPat, &mAllocator);

    mAllocator.adjustFrmHeap();

    mAnmTexPat.setAnm(mModel, mResAnmTexPat, 0, m3d::FORWARD_ONCE);
}

// #16 -- 0x800240B0, 76 B. MATCHING (19 instructions).
int daEnBrosBase_c::execute() {
    mStateMgr.executeState();
    ActorScrOutCheck(0);
    return SUCCEEDED;
}

// #18 -- 0x80024120, 48 B. MATCHING (12 instructions).
int daEnBrosBase_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

// #19 -- 0x80024150, 48 B. MATCHING (12 instructions).
// The map's one differing word vs dpakkun_base is the vtable slot: 0x320 is
// removeHIO, which the header already places there.
int daEnBrosBase_c::doDelete() {
    removeHIO();
    return SUCCEEDED;
}

// #20 0x80024180 -- INTERLEAVE: goes after batch 2's doDelete, before calcMdl
void daEnBrosBase_c::removeHIO() {}

// #21 -- 0x80024190, 296 B. MATCHING (74 instructions).
// The map named net_nokonoko as the precedent (7 differing words), but
// d_a_en_lkuribo_base's calcMdl is the exact body -- it already uses
// mCenterOffs.y where netnoko has the 16.0f literal. The only edit from
// lkuribo's is `mScale` in place of `mScale + mBoyoScale`.
void daEnBrosBase_c::calcMdl() {
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
    mModel.setScale(mScale);
    mModel.calc(false);
}

// #22 -- 0x800242C0, 120 B. MATCHING (30 instructions).
// Scored 0.500 in the map ("NO PRECEDENT") but it is five straight calls.
void daEnBrosBase_c::calcJntMtx() {
    mModel.getNodeWorldMtx(6, &mJntMtx6);
    mModel.getNodeWorldMtx(4, &mJntMtx4);
    mModel.getNodeWorldMtx(0, &mJntMtx0);
    mModel.getNodeWorldMtx(14, &mJntMtx14);
    mModel.getNodeWorldMtx(11, &mJntMtx11);
}

// #23 0x80024340, 0xF4
void daEnBrosBase_c::Normal_VsPlHitCheck(dCc_c *self, dCc_c *other) {
    dActor_c *player = other->getOwner();
    u8 dir = getTrgToSrcDir_Main(getCenterX(), player->getCenterX());

    int fumiRes = Enfumi_check(self, other, 0);
    if (fumiRes == 0) {
        dEn_c::Normal_VsPlHitCheck(self, other);
    } else if (fumiRes == 1) {
        // NOTE: must be qualified. daEnBrosBase_c declares its own
        // StateID_DieFumi, which shadows dEn_c's, and the target relocates this
        // word to StateID_DieFumi__5dEn_c.
        setDeathInfo_Fumi(other->getOwner(), mVec2_c(0.5f * l_base_fall_speed_x[dir ^ 1], 2.0f),
            dEn_c::StateID_DieFumi, 1);
    } else if (fumiRes == 3) {
        setDeathInfo_SpinFumi(player, 1);
    }
}

// #24 0x80024440, 0x84
void daEnBrosBase_c::Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other) {
    dActor_c *yoshi = other->getOwner();

    int fumiRes = Enfumi_check(self, other, 0);
    if (fumiRes == 0) {
        dEn_c::Normal_VsYoshiHitCheck(self, other);
    } else if (fumiRes == 1) {
        setDeathInfo_YoshiFumi(yoshi);
    }
}

// ---------------------------------------------------------------------- #25
bool daEnBrosBase_c::hitCallback_Ice(dCc_c *self, dCc_c *other) {
    if (!mIceMng.mActive) {
        daPlBase_c *player = (daPlBase_c *) other->getOwner();

        if (player->mSpeed.x >= 0.0f) {
            mBoyoMng.mDirection = 0;
        } else {
            mBoyoMng.mDirection = 1;
        }

        mIceMng.mPlrNo = player->getPlrNo();
        changeState(StateID_Ice);
    }

    return true;
}

// ---------------------------------------------------------------------- #26
void daEnBrosBase_c::setIceAnm() {
    setAnm("walk", m3d::FORWARD_LOOP, 0.0f);
}

// ---------------------------------------------------------------------- #27
void daEnBrosBase_c::returnState_Ice() {
    changeState(StateID_Move);
}

// #28 0x800245A0, 0x180
bool daEnBrosBase_c::hitCallback_Spin(dCc_c *self, dCc_c *other) {
    daPlBase_c *player = (daPlBase_c *) other->getOwner();
    u8 dir = getTrgToSrcDir_Main(getCenterX(), player->getCenterX());

    hipatkEffect(player->mPos);
    setDeathSound_HipAttk();

    mDeathInfo = (sDeathInfoData) {
        l_base_fall_speed_x[dir],
        smc_DEADFALL_YSPEED,
        smc_DEADFALL_YSPEED_MAX,
        smc_DEADFALL_GRAVITY,
        &dEn_c::StateID_DieFumi,
        mCombo.getComboScore(dEnCombo_c::calcPlFumiCnt(player)),
        -1,
        dir,
        player->getPlrNo()
    };

    return true;
}

// #29 0x80024720, 0x19C
bool daEnBrosBase_c::hitCallback_HipAttk(dCc_c *self, dCc_c *other) {
    daPlBase_c *player = (daPlBase_c *) other->getOwner();
    u8 dir = getTrgToSrcDir_Main(getCenterX(), player->getCenterX());

    // The Y-before-X declaration order is load-bearing: it is what puts the
    // player's Y in f1 and X in f2. Constructing mVec3_c straight from
    // player->mPos.x / .y instead swaps that FPR pair.
    float py = player->mPos.y;
    float px = player->mPos.x;
    mVec3_c efPos(px, py, 5500.0f);
    dEf::createEffect_change("Wm_mr_hardhit", 0, &efPos, nullptr, nullptr);

    setDeathSound_HipAttk();

    mDeathInfo = (sDeathInfoData) {
        l_base_fall_speed_x[dir],
        smc_DEADFALL_YSPEED,
        smc_DEADFALL_YSPEED_MAX,
        smc_DEADFALL_GRAVITY,
        &dEn_c::StateID_DieFumi,
        mCombo.getComboScore(dEnCombo_c::calcPlFumiCnt(player)),
        -1,
        dir,
        player->getPlrNo()
    };

    return true;
}

// #30 0x800248C0, 0x1A4
bool daEnBrosBase_c::hitCallback_YoshiBullet(dCc_c *self, dCc_c *other) {
    daPlBase_c *player = (daPlBase_c *) other->getOwner();
    u8 dir = getTrgToSrcDir_Main(getCenterX(), player->getCenterX());

    dActorMng_c::m_instance->createJumpCoin(getCenterPos(), 5, mLayer);

    s8 plrNo = player->getPlrNo();
    setDeathSound_Fire();

    // efPos must be a NAMED local: passing the temporary directly to
    // hitdamageEffect gives it the lower stack slot and pushes getCenterPos's
    // sret temp above it, which is the wrong way round.
    mVec3_c efPos(self->getCollPosX(), self->getCollPosY(), 5500.0f);
    hitdamageEffect(efPos);

    mDeathInfo = (sDeathInfoData) {
        l_base_fall_speed_x[dir],
        smc_DEADFALL_YSPEED,
        smc_DEADFALL_YSPEED_MAX,
        smc_DEADFALL_GRAVITY,
        &dEn_c::StateID_DieFall,
        mCombo.getDamageScore(),
        -1,
        dir,
        (u8) plrNo
    };

    return true;
}

// #31 0x80024A70, 0x98
bool daEnBrosBase_c::setDamage(dActor_c *actor) {
    dAcPy_c *player = (dAcPy_c *) actor;
    if (player->setDamage(this, daPlBase_c::DAMAGE_DEFAULT)) {
        mDirection = getTrgToSrcDir_Main(actor->getCenterX(), getCenterX());
        mAngle.y = l_base_angleY[mDirection];
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------- #32
bool daEnBrosBase_c::createIceActor() {
    mVec3_c offset(mPos.x, mPos.y - 2.0f, 2.0f + mPos.z);

    int mode = 1;

    dIceInfo iceInfo[] = {
        {
            mode,
            offset,
            mVec3_c(1.3f, 1.1f, 1.13f)
        }
    };
    return mIceMng.createIce(iceInfo, ARRAY_SIZE(iceInfo));
}

/// @note This TU is where dIceInfo's destructor is emitted out of line --
/// d_ice_manager.hpp only declares it. createIceActor's __destroy_arr call
/// above is what pulls it in, and it lands immediately afterwards at
/// 0x80024C20 as a GLOBAL symbol.
dIceInfo::~dIceInfo() {}

// ---------------------------------------------------------------------- #34
bool daEnBrosBase_c::calcTurnAngle() {
    static const s16 cs_turn_speed[2] = { 0x800, -0x800 };

    mAngle.y += cs_turn_speed[mDirection];
    s16 target = l_base_angleY[mDirection];
    int absTarget = abs(target);
    int absAngle = abs(mAngle.y);
    if (absAngle >= absTarget) {
        mAngle.y = target;
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------- #35
void daEnBrosBase_c::calcMoveDir() {
    bool flip = false;

    if (mDirection == 0) {
        if (mSpeed.x >= 0.0f) {
            if (mPos.x >= mHomePos.x + mMoveRangeFront) {
                flip = true;
            }
        } else {
            if (mPos.x <= mHomePos.x - mMoveRangeBack) {
                flip = true;
            }
        }
    } else {
        if (mSpeed.x >= 0.0f) {
            if (mPos.x >= mHomePos.x + mMoveRangeBack) {
                flip = true;
            }
        } else {
            if (mPos.x <= mHomePos.x - mMoveRangeFront) {
                flip = true;
            }
        }
    }

    if (flip) {
        mMoveDir ^= 1;
        setSpeed();
    }
}

// ---------------------------------------------------------------------- #36
void daEnBrosBase_c::landonEffect() {
    mVec3_c pos(mPos.x, mPos.y, 5500.0f);
    mVec3_c scale(1.3f, 1.3f, 1.3f);

    if (mBc.getFootAttr() == 12) {
        mEf::createEffect("Wm_en_sndlandsmk_s", 0, &pos, nullptr, &scale);
    } else {
        mEf::createEffect("Wm_en_landsmoke_s", 0, &pos, nullptr, &scale);
    }
}

// ---------------------------------------------------------------------- #37
// *** DEPENDENCY FOR EVERY OTHER BATCH -- this is the confirmed form. ***
// Byte-identical to daEnLkuriboBase_c::setAnm, all three statements included.
void daEnBrosBase_c::setAnm(char *name, m3d::playMode_e playMode, float blendFrame) {
    mAnmChr.setAnm(mModel, mResFile.GetResAnmChr(name), playMode);
    mModel.setAnm(mAnmChr, blendFrame);
    mAnmChr.setRate(1.0f);
}

// ---------------------------------------------------------------------- #38
void daEnBrosBase_c::setMoveAnm(float blendFrame) {
    if (mAnmDir == 0) {
        setAnm("walk", m3d::FORWARD_LOOP, blendFrame);
    } else {
        setAnm("walk_back", m3d::FORWARD_LOOP, blendFrame);
    }
}

// ---------------------------------------------------------------------- #39
void daEnBrosBase_c::dirProc() {
    setMoveAnm(6.0f);
}

// ---------------------------------------------------------------------- #40
void daEnBrosBase_c::setAttackAnm() {
    setAnm("throw", m3d::FORWARD_ONCE, 4.0f);
}

void daEnBrosBase_c::initializeState_Move() {
    u8 dir = mMoveDir;
    m_6f4 = 0;

    if (dir == mDirection) {
        mAnmDir = 0;
    } else {
        mAnmDir = 1;
    }

    mStopXCnt = 0;
    setMoveAnm(0.0f);

    if (dir != mAnmDir) {
        dirProc();
    }

    setSpeed();
    setAtkTimer();
}

// ---- #42  0x80025010  size 0x4 --------------------------------------------
void daEnBrosBase_c::setAtkTimer() {}

void daEnBrosBase_c::finalizeState_Move() {}

void daEnBrosBase_c::executeState_Move() {
    mModel.play();
    mDirection = getPl_LRflag(mPos);

    bool turnDone = calcTurnAngle();
    calcMoveDir();

    int oldAnmDir = mAnmDir;

    if (mMoveDir == mDirection) {
        mAnmDir = 0;
    } else {
        mAnmDir = 1;
    }

    if (oldAnmDir != mAnmDir && 0.0f != mSpeed.x) {
        dirProc();
    }

    calcSpeedY();
    posMove();

    if (mStopXCnt > 0) {
        mPos.x = mLastPos.x;
        mStopXCnt--;
    }

    if (mBc.checkFootEnm()) {
        mSpeed.y = 0.0f;
    } else {
        mPos.x = mLastPos.x;

        if (mJumpCnt < 16) {
            mJumpCnt = 16;
        }
    }

    if (mBc.checkWall(nullptr)) {
        mSpeed.x = 0.0f;
    }

    if (--mJumpCnt <= 0) {
        setJumpCnt();
        beginJump();
    } else if (turnDone && isAttackOK() && --mAtkTimer <= 0) {
        mSpeed.x = 0.0f;
        beginAttk();
    }
}

void daEnBrosBase_c::beginJump() {
    setJump();
    changeState(StateID_AirAttack);
}

void daEnBrosBase_c::setJump() {}

// ---- #47  0x80025270  size 0x8 --------------------------------------------
bool daEnBrosBase_c::isAttackOK() const {
    return true;
}

// ---- #48  0x80025280  size 0x20 -------------------------------------------
void daEnBrosBase_c::beginAttk() {
    mAttackCnt = 1;
    changeState(StateID_Attack);
}

// ---- #49  0x800252A0  size 0x40 -------------------------------------------
void daEnBrosBase_c::initializeState_Attack() {
    setAttackAnm();
    m_23b = 1;
}

// ---- #50  0x800252E0  size 0x4 --------------------------------------------
void daEnBrosBase_c::finalizeState_Attack() {}

// ---- #51  0x800252F0  size 0x1F4 ------------------------------------------
void daEnBrosBase_c::executeState_Attack() {
    mModel.play();
    calcSpeedY();
    posMove();

    bool landed = false;
    if (!isInvalidBg()) {
        if (mBc.checkFootEnm()) {
            landed = true;
            mSpeed.x = 0.0f;
            mSpeed.y = 0.0f;
            mSpeed.z = 0.0f;
        }
        if (mBc.checkWall(nullptr)) {
            mSpeed.x = 0.0f;
        }
    }

    switch (m_23b) {
        case 1:
            if (mAnmChr.getFrame() >= getCreateWeaponFrm()) {
                weaponCreate();
                m_23b = 2;
            }
            break;

        case 2: {
            float frm = getAttackFrm();
            if (mAnmChr.checkFrame(frm)) {
                weaponAttack();
            } else if (mAnmChr.getFrame() < frm) {
                calcMdl();
                setWeaponPos();
            } else if (mAnmChr.isStop()) {
                if (--mAttackCnt > 0) {
                    initializeState_Attack();
                } else if (landed) {
                    changeState(StateID_Move);
                }
            }
            break;
        }
    }
}

// ---- #52  0x800254F0  size 0x8 --------------------------------------------
bool daEnBrosBase_c::isInvalidBg() {
    return false;
}

// ---- #53  0x80025500  size 0x8 --------------------------------------------
float daEnBrosBase_c::getCreateWeaponFrm() const {
    return 0.0f;
}

// ---- #54  0x80025510  size 0x4 --------------------------------------------
void daEnBrosBase_c::weaponCreate() {}

// ---- #55  0x80025520  size 0x8 --------------------------------------------
float daEnBrosBase_c::getAttackFrm() const {
    return 0.0f;
}

// ---- #56  0x80025530  size 0x4 --------------------------------------------
void daEnBrosBase_c::weaponAttack() {}

// ---- #57  0x80025540  size 0x4 --------------------------------------------
void daEnBrosBase_c::setWeaponPos() {}

void daEnBrosBase_c::initializeState_JumpSt() {
    setAnm("jump_st", m3d::FORWARD_ONCE, 4.0f);
    mSpeed.set(0.0f, 0.0f, 0.0f);
}

void daEnBrosBase_c::finalizeState_JumpSt() {}

void daEnBrosBase_c::executeState_JumpSt() {
    mModel.play();
    calcSpeedY();
    posMove();

    if (!isInvalidBg()) {
        if (mBc.checkFootEnm()) {
            mSpeed.y = 0.0f;
        }

        if (mBc.checkWall(nullptr)) {
            mPos = mLastPos;
        }
    }

    if (mAnmChr.isStop()) {
        changeState(StateID_Jump);
    } else if (7.0f == mAnmChr.getFrame()) {
        setJump();
    }
}

void daEnBrosBase_c::initializeState_Jump() {
    setAnm("jump_md", m3d::FORWARD_ONCE, 0.0f);
}

void daEnBrosBase_c::finalizeState_Jump() {}

void daEnBrosBase_c::executeState_Jump() {
    mModel.play();
    calcSpeedY();
    posMove();

    if (!isInvalidBg()) {
        if (mBc.checkWall(nullptr)) {
            mSpeed.x = 0.0f;
        }

        if (mBc.checkFootEnm()) {
            landonEffect();
            changeState(StateID_JumpEd);
        }
    }
}

// ---- #64  0x800257A0  size 0x4C -------------------------------------------
void daEnBrosBase_c::initializeState_AirAttack() {
    setAttackAnm();
    mArmAngle = 0;
    mHandAngle = 0;
    m_23b = 1;
}

// ---- #65  0x800257F0  size 0x10 -------------------------------------------
void daEnBrosBase_c::finalizeState_AirAttack() {
    mArmAngle = 0;
    mHandAngle = 0;
}

// ---- #66  0x80025800  size 0x1FC ------------------------------------------
// Identical to executeState_Attack except for the head: the frame of mAnmChr is
// read and DISCARDED before mModel.play(). The bare call is required -- it is
// what produces the extra `addi r3, this, 0x584; bl getFrame` and forces the
// `this` pointer into r30 for the play() dispatch that follows.
void daEnBrosBase_c::executeState_AirAttack() {
    mAnmChr.getFrame();
    mModel.play();
    calcSpeedY();
    posMove();

    bool landed = false;
    if (!isInvalidBg()) {
        if (mBc.checkFootEnm()) {
            landed = true;
            mSpeed.x = 0.0f;
            mSpeed.y = 0.0f;
            mSpeed.z = 0.0f;
        }
        if (mBc.checkWall(nullptr)) {
            mSpeed.x = 0.0f;
        }
    }

    switch (m_23b) {
        case 1:
            if (mAnmChr.getFrame() >= getCreateWeaponFrm()) {
                weaponCreate();
                m_23b = 2;
            }
            break;

        case 2: {
            float frm = getAttackFrm();
            if (mAnmChr.checkFrame(frm)) {
                weaponAttack();
            } else if (mAnmChr.getFrame() < frm) {
                calcMdl();
                setWeaponPos();
            } else if (mAnmChr.isStop()) {
                if (--mAttackCnt > 0) {
                    initializeState_Attack();
                } else if (landed) {
                    changeState(StateID_Move);
                }
            }
            break;
        }
    }
}

void daEnBrosBase_c::initializeState_JumpEd() {
    float blend = 0.0f;

    if (*mStateMgr.getOldStateID() != StateID_Jump) {
        blend = 2.0f;
    }

    setAnm("jump_ed", m3d::FORWARD_ONCE, blend);
    mSpeed.set(0.0f, 0.0f, 0.0f);
}

void daEnBrosBase_c::finalizeState_JumpEd() {}

void daEnBrosBase_c::executeState_JumpEd() {
    mModel.play();
    calcSpeedY();
    posMove();

    if (mBc.checkFootEnm()) {
        mSpeed.y = 0.0f;
    }

    if (mAnmChr.isStop()) {
        setJumpCnt();
        changeState(StateID_Move);
    }
}

// ---------------------------------------------------------------------- #71
void daEnBrosBase_c::initializeState_DieFumi() {
    setAnm("dead", m3d::FORWARD_LOOP, 0.0f);
    dEn_c::initializeState_DieFall();
}

// ---------------------------------------------------------------------- #72
void daEnBrosBase_c::finalizeState_DieFumi() {}

// ---------------------------------------------------------------------- #73
void daEnBrosBase_c::executeState_DieFumi() {
    mModel.play();
    mAngle.x += 0x800;
    calcSpeedY();
    posMove();
    WaterCheck(mPos, 1.0f);
}

// ---------------------------------------------------------------------- #74
void daEnBrosBase_c::initializeState_DieFall() {
    setAnm("dead", m3d::FORWARD_LOOP, 0.0f);
    dEn_c::initializeState_DieFall();
}

// ---------------------------------------------------------------------- #75
void daEnBrosBase_c::finalizeState_DieFall() {}

// ---------------------------------------------------------------------- #76
void daEnBrosBase_c::executeState_DieFall() {
    mModel.play();

    if (mDirection == mIceDeathDirection) {
        mAngle.x += 0x800;
    } else {
        mAngle.x -= 0x800;
    }

    calcSpeedY();
    posMove();
    WaterCheck(mPos, 1.0f);
}

// #77 -- 0x80025D20, 248 B. MATCHING (62 instructions).
// Same body as daEnDpakkunBase_c::nodeCallback_c::timingA, twice, under a
// two-case switch: node 0x0B takes mArmAngle, node 0x0E takes mHandAngle, and
// both go into the Z rotation (dpakkun's goes into X).
// The two `rot` locals must stay in separate case scopes -- that is what puts
// them in distinct stack slots (0x14 and 0x8).
void daEnBrosBase_c::nodeCallback_c::timingA(ulong nodeId, nw4r::g3d::ChrAnmResult *anmRes, nw4r::g3d::ResMdl resMdl) {
    switch (nodeId) {
        case 0x0b: {
            float deg = mpOwner->mArmAngle * 90 / 0x4000;

            nw4r::math::VEC3 rot;
            anmRes->GetRotateDeg(&rot);
            rot.z += deg;
            anmRes->SetRotateDeg(&rot);
            break;
        }

        case 0x0e: {
            float deg = mpOwner->mHandAngle * 90 / 0x4000;

            nw4r::math::VEC3 rot;
            anmRes->GetRotateDeg(&rot);
            rot.z += deg;
            anmRes->SetRotateDeg(&rot);
            break;
        }
    }
}

// #78 -- 0x80025E20, 20 B. MATCHING (5 instructions). See BATCH OVERLAP above.
// Returns by value: three 0.0f stores through the sret pointer in r3.
mVec3_c daEnBrosBase_c::getAdjustOffset() {
    return mVec3_c(0.0f, 0.0f, 0.0f);
}

// #79 -- 0x80025E40, 8 B. MATCHING (2 instructions). See BATCH OVERLAP above.
bool daEnBrosBase_c::checkAtkArea() {
    return true;
}

// #80 -- 0x80025E50, 52 B. MATCHING (13 instructions).
void daEnBrosBase_c::finalUpdate() {
    calcMdl();
    calcJntMtx();
}

/// @note Defined out of line, last in the file: the trailing destructor
/// cascade (levelEffect_c -> effect_c -> nodeCallback_c -> callback_c ->
/// anmChr_c) is flushed from here, which is what puts it in target order.
daEnBrosBase_c::~daEnBrosBase_c() {}
