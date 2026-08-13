#include <game/bases/d_a_en_hatena_balloon.hpp>
#include <game/bases/d_a_en_carry.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_actor_manager.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_balloon_manager.hpp>
#include <game/bases/d_bg.hpp>
#include <game/bases/d_bg_parameter.hpp>
#include <game/bases/d_enemy_manager.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_quake.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/cLib/c_lib.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/sLib/s_lib.hpp>


ACTOR_PROFILE(EN_HATENA_BALLOON, daEnHatenaBalloon_c, 0x46);

STATE_DEFINE(daEnHatenaBalloon_c, DispFlyWait);
STATE_DEFINE(daEnHatenaBalloon_c, DispFlyMove);
STATE_DEFINE(daEnHatenaBalloon_c, Fly);
STATE_DEFINE(daEnHatenaBalloon_c, Escape);
STATE_DEFINE(daEnHatenaBalloon_c, HipAttack);
STATE_DEFINE(daEnHatenaBalloon_c, SearchSpace);

// Must sit AFTER the six STATE_DEFINEs and BEFORE create(): create addresses
// all three of these AND StateID_DispFlyWait off one base register, and MWCC
// only anchors a static that way once it has seen the definition. With them
// at the end of the file, create misses by ~40 instructions.

mVec3_c daEnHatenaBalloon_c::sm_bg_check_size_mame(1.5f, 1.5f, 16.0f);
mVec3_c daEnHatenaBalloon_c::sm_bg_check_size_normal(4.0f, 4.0f, 18.0f);
mVec3_c daEnHatenaBalloon_c::sm_bg_check_size_super(4.0f, 10.0f, 22.0f);


// .rodata definition order is cross-batch and fixed by the target's addresses:
// s_someCheckData, then the cullinfo/cc pair, then l_create_diff.

const daEnHatenaBalloon_c::checkData_s daEnHatenaBalloon_c::s_someCheckData[4] = {
    { 3.0f, 31.0f, -3.0f, 31.0f, 1 },
    { 3.0f, 5.0f, -3.0f, 5.0f, 2 },
    { -7.5f, 10.0f, -7.5f, 20.0f, 4 },
    { 7.5f, 10.0f, 7.5f, 20.0f, 8 },
};

static const mVec2_POD_c l_hatenaballoon_cullinfo[] = {
    {0.0f, 0.0f},
    {64.0f, 64.0f}
};

static const sCcDatNewF l_cc_data = {
    0.0f, 13.0f,
    20.0f, 20.0f,
    CC_KIND_BALLOON,
    CC_ATTACK_NONE,
    BIT_FLAG(CC_KIND_PLAYER) | BIT_FLAG(CC_KIND_PLAYER_ATTACK) |
        BIT_FLAG(CC_KIND_YOSHI) | BIT_FLAG(CC_KIND_ENEMY) |
        BIT_FLAG(CC_KIND_BALLOON) | BIT_FLAG(CC_KIND_TAMA),
    (u32) ~(BIT_FLAG(CC_ATTACK_NONE) | BIT_FLAG(CC_ATTACK_YOSHI_MOUTH) |
        BIT_FLAG(CC_ATTACK_SPIN_LIFT_UP) | BIT_FLAG(CC_ATTACK_YOSHI_BULLET) |
        BIT_FLAG(CC_ATTACK_YOSHI_FIRE) | BIT_FLAG(CC_ATTACK_ICE_2) |
        BIT_FLAG(CC_ATTACK_SAND_PILLAR)),
    CC_STATUS_NONE,
    dEn_c::normal_collcheck
};


float daEnHatenaBalloon_c::sm_hio_gravity = -0.08f;
float daEnHatenaBalloon_c::sm_hio_base_fly_timer_x = 200.0f;
float daEnHatenaBalloon_c::sm_hio_fly_yspeed = 0.4f;
float daEnHatenaBalloon_c::sm_hio_mask_size = 80.0f;
float daEnHatenaBalloon_c::sm_hio_mask_y_diff = 15.0f;

static const float l_create_diff[] = { 0.0f, -32.0f, 32.0f, -64.0f };

/// @brief Unreferenced in this TU, but the original emits it: 0x40 of `.rodata`
/// at 0x802F4EB8, between `l_create_diff` and the next TU's first object.
/// @note `extern` is load-bearing. At namespace scope a `const` array has
/// internal linkage in C++, so as a plain `static`/`const` it would be stripped
/// as unused and the whole `.rodata` section -- and therefore every section
/// after it -- would come out 0x40 short. @unofficial
extern const float l_speed_ratiodt[] = {
    0.0f, 0.125f, 0.25f, 0.375f,
    0.5f, 0.625f, 0.75f, 0.875f,
    1.0f, 0.0f,   1.2f,  1.5f,
    2.0f, 4.0f,   0.0f,  0.0f,
};

// ---------------------------------------------------------------- 0x80110410
int daEnHatenaBalloon_c::create() {
    m_822 = 1;

    mPlayerNo = ACTOR_PARAM(PLAYER_NO);
    m_814 = ACTOR_PARAM(SUB_TYPE);
    mBalloonType = ACTOR_PARAM(BALLOON_TYPE);

    mVec3_c sizes[3] = {
        sm_bg_check_size_mame,
        sm_bg_check_size_normal,
        sm_bg_check_size_super
    };

    u8 tallType;
    if (mBalloonType == 0) {
        tallType = daPyMng_c::getPlayer(mPlayerNo)->getTallType(-1);
    } else {
        tallType = 1;
    }
    m_7d4 = sizes[tallType];

    model_set();

    mLightMask.init(&mMaskAllocator, 2);

    if (mBalloonType == 1) {
        mItemDrawPos.set(1.0f, 1.0f, 1.0f);
        m_80c = 0;
        mActorProperties |= 0x20;
    }

    mScale.set(1.0f, 1.0f, 1.0f);

    // Must precede mSpeedMax: the original stores mAccelY (0x114) between the
    // mScale stores and the mSpeedMax stores.
    mAccelY = sm_hio_gravity;

    mSpeedMax.x = 4.0f;
    mSpeedMax.y = -4.0f;
    mSpeedMax.z = 0.0f;

    m_81f = -1;

    setVisibleArea(l_hatenaballoon_cullinfo[0], l_hatenaballoon_cullinfo[1]);

    mEatBehavior = 0;

    if (mBalloonType == 0) {
        dAcPy_c *player = daPyMng_c::getPlayer(mPlayerNo);
        mPos.z = 8000.0f;
        m_821 = 1;
        if (player->mLayer == 0 && player->mPos.z < 0.0f) {
            m_821 = 2;
        }
    } else {
        m_821 = 1;
    }

    mSensorFoot.mFlags = 0x80000001;
    mSensorFoot.mLineA = -0x7000;
    mSensorFoot.mLineB = 0x7000;
    mSensorFoot.mDistanceFromCenter = 0x3000;

    mSensorHead.mFlags = 0x80000001;
    mSensorHead.mLineA = -0x7000;
    mSensorHead.mLineB = 0x7000;
    mSensorHead.mDistanceFromCenter = 0x1D000;

    mSensorWall.mFlags = 0x80000001;
    mSensorWall.mLineA = 0x5000;
    mSensorWall.mLineB = 0x18000;
    mSensorWall.mDistanceFromCenter = 0x7000;

    mBc.set(this, mSensorFoot, mSensorHead, mSensorWall);

    mCc.set(this, (sCcDatNewF *) &l_cc_data);

    // The original really does write mLayer / mAmiLine, then the fumi-check
    // field, and only then mShape -- the four stores are emitted in source
    // order (0x1e8, 0x1e7, 0x518, 0x1e6).
    mCc.mLayer = 0;
    mCc.mAmiLine = m_821;

    mFumiProc.mFumiCheck.m_00 = 4;

    mCc.mShape = CC_SHAPE_CIRCLE;

    m_87c = 0;
    m_800 = 0;
    m_880 = 0;
    m_881 = 0;

    // The cast is load-bearing: the original compares m_814 with cmplwi, so the
    // member is UNSIGNED. It is declared `int m_814` in the header -- see the
    // report; the cast goes away once the header says u32.
    if (mBalloonType == 0 && (u32) m_814 == 2) {
        m_7fc = 0x28;
    }

    changeState(StateID_DispFlyWait);

    return SUCCEEDED;
}

// ---------------------------------------------------------------- 0x80110720
int daEnHatenaBalloon_c::execute() {
    if (m_818 != 0) {
        m_818--;
    }
    if (m_81b != 0) {
        m_81b--;
    }
    if (m_7fc != 0) {
        m_7fc--;
    }
    if (m_800 != 0) {
        m_800--;
    }
    if (m_808 != 0) {
        m_808--;
    }

    shake_disp_check();

    if (dEnemyMng_c::m_instance->m_15c != 0) {
        if (!isState(StateID_Escape)) {
            changeState(StateID_Escape);
        }
    }

    if (!pause_check()) {
        mModel.play();
        mAnmTexSrt.play();
        mBalloonModel.play();
        if (mBalloonType == 1) {
            mItemModel.play();
        }
        mStateMgr.executeState();

        m_87c--;
        if (m_87c < 0) {
            m_87c = 0;
        }
    }

    setCcLine();

    float maskY = mPos.y + sm_hio_mask_y_diff;
    float maskX = mPos.x;
    float maskR = sm_hio_mask_size;
    mLightMask.set(maskX, maskY, 8800.0f, maskR);
    mLightMask.execute();

    return SUCCEEDED;
}

// ---------------------------------------------------------------- preDraw
int daEnHatenaBalloon_c::preDraw() {
    if (m_822 != 0) {
        if (mBalloonType == 0) {
            dAcPy_c *player = daPyMng_c::getPlayer(mPlayerNo);
            if (player == nullptr || !player->isStatus(0x53)) {
                deleteActor(1);
                return dActor_c::preDraw();
            }
            mVec3_c pos(mPos);
            pos.y += 6.0f;
            pos.z -= 64.0f;
            player->setDrawBalloonInPlayer(pos);
        } else {
            mVec3_c pos(mPos);
            pos.y += 8.0f;
            pos.z -= 64.0f;
            item_draw_calc(&pos);
        }
    }
    return dActor_c::preDraw();
}

// ---------------------------------------------------------------- draw
int daEnHatenaBalloon_c::draw() {
    if (mBalloonType == 1) {
        mItemModel.entry();
    }

    mMtx_c nodeMtx;
    mMtx_c mtx;
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;

    // The original compares against a float that is NOT a bare literal -- a bare
    // 0.0f makes CodeWarrior canonicalise the constant into fcmpu's FIRST operand,
    // and the target has it second. Binding a const reference keeps the source
    // operand order without allocating an extra FPR (a plain `float zero = 0.0f;`
    // local matches instruction-for-instruction but rotates f0/f1/f2 in the mPos
    // copy above). The `!(a != b)` double negation, materialised through `isZero`,
    // is what produces the mfcr/extrwi/xori/cntlzw/srwi. sequence; folding it to
    // `if (m_7c4 == zero)` collapses to a bare fcmpu/beq and loses 5 instructions.
    float shake = m_7c4;
    const f32 &zero = 0.0f;
    bool isZero = !(m_7c4 != zero);
    if (isZero) {
        shake = m_7c8;
    }
    pos.x += shake;
    pos.y += 16.0f;
    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.YrotM(angle.y);
    mMatrix.XrotM(angle.x);
    mMatrix.ZrotM(angle.z);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
    mModel.getNodeWorldMtx(0, &nodeMtx);
    // set() -- not three field assignments: the argument list is evaluated
    // right-to-left, which hoists all three loads above all three stores.
    m_78c.set(nodeMtx.m[0][3], nodeMtx.m[1][3], nodeMtx.m[2][3]);
    mModel.entry();

    pos.z = m_7d0 - 64.0f;
    mtx.trans(pos.x, pos.y, pos.z);
    mtx.YrotM(angle.y);
    mtx.XrotM(angle.x);
    mtx.ZrotM(angle.z);
    mBalloonModel.setLocalMtx(&mtx);
    mBalloonModel.setScale(mScale);
    mBalloonModel.calc(false);
    mBalloonModel.entry();

    mLightMask.draw();

    return 1;
}

// ---------------------------------------------------------------- 0x80110C40
int daEnHatenaBalloon_c::doDelete() {
    return true;
}

// ---------------------------------------------------------------- 0x80110C50
u32 daEnHatenaBalloon_c::ccLineCheck(float x, float y) {
    if (x >= 8192.0f || x < 0.0f) {
        return 0;
    }
    if (y >= 8192.0f || y < 0.0f) {
        return 0;
    }
    return dBc_c::checkWireNet(x, y, mLayer);
}

// ---------------------------------------------------------------- 0x80110CA0
void daEnHatenaBalloon_c::setCcLine() {
    if (mLayer == 1) {
        mCc.mAmiLine = 3;
        return;
    }

    bool hit = false;
    float sizeY = mCc.mCcData.mBase.mSize.y;
    float offsX = mCc.mCcData.mBase.mOffset.x;
    float offsY = mCc.mCcData.mBase.mOffset.y;
    float x = mPos.x + offsX;
    float y = mPos.y + offsY;
    float sizeX = mPos.x + offsX;

    if (ccLineCheck(x, y + sizeY) && !hit) {
        hit = true;
    }
    if (ccLineCheck(x, y - sizeY) && !hit) {
        hit = true;
    }
    if (ccLineCheck(x - sizeX, y) && !hit) {
        hit = true;
    }
    if (ccLineCheck(x + sizeX, y) && !hit) {
        hit = true;
    }

    if (hit) {
        mCc.mAmiLine = m_821;
    } else {
        mCc.mAmiLine = 3;
    }
}

// ---------------------------------------------------------------- 0x80110DE0
void daEnHatenaBalloon_c::PlYsHitCheck(dActor_c *player, daEnHatenaBalloon_c *self) {
    daPlBase_c *pl = (daPlBase_c *)player;
    if (self->mLayer == 0) {
        if ((player->mPos.z > 0.0f && self->m_821 == 2) || (player->mPos.z < 0.0f && self->m_821 == 1)) {
            if (pl->isNowBgCross(daPlBase_c::BGC_VINE_TOUCH) ||
                pl->isNowBgCross((daPlBase_c::BgCross2_e)0x40000) ||
                pl->isNowBgCross(daPlBase_c::BGC_VINE_TOUCH_L) ||
                pl->isNowBgCross(daPlBase_c::BGC_VINE_TOUCH_R)) {
                return;
            }
        }
    }
    if (self->m_800 != 0) {
        return;
    }
    m_7b0 = self->hipattackhit(pl->getPlrNo(), player->getCenterPos());
    float border = dActorMng_c::m_instance->mGoalPoleX;
    if (-8000.0f != border) {
        if (8.0f + mPos.x >= border) {
            m_7b0.x = border - 16.0f;
        }
    }
}

// ---------------------------------------------------------------- 0x80110F20
void daEnHatenaBalloon_c::Normal_VsPlHitCheck(dCc_c *self, dCc_c *other) {
    if (m_87c <= 0 && m_880 == 0) {
        daEnHatenaBalloon_c *balloon = (daEnHatenaBalloon_c *)self->mpOwner;
        if (!balloon->isState(StateID_SearchSpace)) {
            dActor_c *player = other->mpOwner;
            PlYsHitCheck(player, balloon);
            mHitPos = player->getCenterPos();
            mHitFlag = 1;
        }
    }
}

// ---------------------------------------------------------------- 0x80111000
void daEnHatenaBalloon_c::Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other) {
    if (m_87c <= 0 && m_880 == 0) {
        daEnHatenaBalloon_c *balloon = (daEnHatenaBalloon_c *)self->mpOwner;
        if (!balloon->isState(StateID_SearchSpace)) {
            daYoshi_c *yoshi = (daYoshi_c *)other->mpOwner;
            if (yoshi->m_94 != 0) {
                PlYsHitCheck(yoshi, balloon);
                mHitPos = yoshi->getCenterPos();
                mHitFlag = 1;
            }
        }
    }
}

// ---------------------------------------------------------------- 0x801110E0
void daEnHatenaBalloon_c::Normal_VsEnHitCheck(dCc_c *self, dCc_c *other) {
    daEnHatenaBalloon_c *o = (daEnHatenaBalloon_c *)other->mpOwner;
    daEnHatenaBalloon_c *s = (daEnHatenaBalloon_c *)self->mpOwner;

    if (o->mProfName == fProfile::EN_HATENA_BALLOON) {
        mVec3_c posS = s->mPos;
        mVec3_c posO = o->mPos;
        float dy = posS.y - posO.y;
        float dx = posS.x - posO.x;
        mVec3_c diff(dx, dy, 0.0f);

        float mag = PSVECMag(diff);
        if (mag > 0.0f) {
            diff.normalize();
        } else {
            diff = mVec3_c::Ex;
        }

        if (!(mag > 33.0f)) {
            if (s->isState(StateID_SearchSpace)) {
                if (o->isState(StateID_SearchSpace)) {
                    diff *= 2.0f;
                    s->mSpeed = diff;
                    mHitFlag = 0;
                    s->changeState(StateID_Fly);
                }
            } else {
                diff *= 1.0f;
                s->mSpeed = diff;
            }
        }
    }
}

// ---------------------------------------------------------------- 0x801112F0
bool daEnHatenaBalloon_c::hitCallback_Fire(dCc_c *self, dCc_c *other) {
    if (m_87c <= 0 && m_880 == 0) {
        daEnHatenaBalloon_c *balloon = (daEnHatenaBalloon_c *)self->mpOwner;
        if (!balloon->isState(StateID_SearchSpace)) {
            PlYsHitCheck(other->mpOwner, balloon);
            mHitPos = other->mpOwner->getCenterPos();
            mHitFlag = 1;
        }
    }
    return true;
}

// ---------------------------------------------------------------- 0x801113D0
bool daEnHatenaBalloon_c::hitCallback_YoshiBullet(dCc_c *self, dCc_c *other) {
    return true;
}

// ---------------------------------------------------------------- 0x801113E0
bool daEnHatenaBalloon_c::hitCallback_Cannon(dCc_c *self, dCc_c *other) {
    daEnHatenaBalloon_c::hitCallback_HipAttk(self, other);
    return true;
}

// ---------------------------------------------------------------- 0x80111410
bool daEnHatenaBalloon_c::hitCallback_Ice(dCc_c *self, dCc_c *other) {
    return daEnHatenaBalloon_c::hitCallback_Fire(self, other);
}

// ---------------------------------------------------------------- 0x80111420
bool daEnHatenaBalloon_c::hitCallback_Star(dCc_c *self, dCc_c *other) {
    daEnHatenaBalloon_c::hitCallback_HipAttk(self, other);
    return true;
}

// ---------------------------------------------------------------- 0x80111450
bool daEnHatenaBalloon_c::hitCallback_Large(dCc_c *self, dCc_c *other) {
    daEnHatenaBalloon_c::hitCallback_HipAttk(self, other);
    return true;
}

// ---------------------------------------------------------------- 0x80111480
bool daEnHatenaBalloon_c::hitCallback_Spin(dCc_c *self, dCc_c *other) {
    daEnHatenaBalloon_c::hitCallback_HipAttk(self, other);
    return true;
}

// ---------------------------------------------------------------- 0x801114B0
bool daEnHatenaBalloon_c::hitCallback_Shell(dCc_c *self, dCc_c *other) {
    dEn_c *o = (dEn_c *)other->mpOwner;
    daEnHatenaBalloon_c *s = (daEnHatenaBalloon_c *)self->mpOwner;

    if (o->isState(daEnCarry_c::StateID_Carry)) {
        return false;
    }
    if (o->getPlrNo() == -1) {
        return false;
    }
    if (s->m_800 != 0) {
        return false;
    }
    if (s->m_808 != 0) {
        return false;
    }
    s->m_808 = 0x28;

    if (m_87c <= 0 && m_880 == 0) {
        mVec3_c pos = o->getCenterPos();
        m_7b0 = s->hipattackhit(o->getPlrNo(), pos);
        mHitPos = o->getCenterPos();
        mHitFlag = 1;
    }

    if (-8000.0f != dActorMng_c::m_instance->mGoalPoleX) {
        if (8.0f + mPos.x >= dActorMng_c::m_instance->mGoalPoleX) {
            m_7b0.x = dActorMng_c::m_instance->mGoalPoleX - 16.0f;
        }
    }

    return true;
}

// ---------------------------------------------------------------- 0x80111680
bool daEnHatenaBalloon_c::hitCallback_Slip(dCc_c *self, dCc_c *other) {
    daEnHatenaBalloon_c::hitCallback_HipAttk(self, other);
    return true;
}

// ---------------------------------------------------------------- 0x801116B0
bool daEnHatenaBalloon_c::hitCallback_WireNet(dCc_c *self, dCc_c *other) {
    return true;
}

// ---------------------------------------------------------------- 0x801116C0
bool daEnHatenaBalloon_c::hitCallback_HipAttk(dCc_c *self, dCc_c *other) {
    if (m_87c <= 0 && m_880 == 0) {
        daEnHatenaBalloon_c *balloon = (daEnHatenaBalloon_c *)self->mpOwner;
        if (!balloon->isState(StateID_SearchSpace)) {
            PlYsHitCheck(other->mpOwner, balloon);
            mHitPos = other->mpOwner->getCenterPos();
            mHitFlag = 1;
        }
    }
    return true;
}

// ---------------------------------------------------------------- 0x801117A0
bool daEnHatenaBalloon_c::hitCallback_YoshiHipAttk(dCc_c *self, dCc_c *other) {
    if (!isState(StateID_SearchSpace)) {
        dActor_c *o = other->mpOwner;
        float dx = m_828.x - o->mPos.x;
        float dy = m_828.y - o->mPos.y;
        if (dx * dx + dy * dy >= 1.44f) {
            daEnHatenaBalloon_c *s = (daEnHatenaBalloon_c *)self->mpOwner;
            if (s->m_800 == 0) {
                s->m_7b0 = s->hipattackhit(o->getPlrNo(), o->getCenterPos());
            }
        }
    }
    return true;
}

// ---------------------------------------------------------------- 0x801118C0
void daEnHatenaBalloon_c::hitYoshiEat(dCc_c *self, dCc_c *other) {
    PlYsHitCheck(other->mpOwner, (daEnHatenaBalloon_c *)self->mpOwner);
}

// ---------------------------------------------------------------- 0x801118D0
void daEnHatenaBalloon_c::block_hit_init() {}

// ---------------------------------------------------------------- 0x801118E0
mVec3_c daEnHatenaBalloon_c::hipattackhit(s8 plrNo, const mVec3_c pos) {
    mVec3_c ret;
    float ty = pos.y - 16.0f;
    float tz = pos.z;
    float tx = pos.x;
    m_81f = plrNo;
    ret.x = tx;
    ret.z = tz;
    ret.y = ty;
    if (plrNo != -1) {
        daPlBase_c *player = daPyMng_c::getPlayer(plrNo);
        if (player != nullptr && player->isStatus(0x4f)) {
            ret.y += 9.0f;
        }
    }
    changeState(StateID_HipAttack);
    return ret;
}

// ---------------------------------------------------------------- model_set
void daEnHatenaBalloon_c::model_set() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("balloon", "g3d/balloon.brres");

    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("balloon");
    mModel.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC);
    mModel.setPriorityDraw(-1, 0x98);

    nw4r::g3d::ResAnmChr anmChr = mResFile.GetResAnmChr("float");
    mAnmChr.create(mdl, anmChr, &mAllocator);
    mAnmChr.setAnm(mModel, anmChr, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnmChr, 1.0f);

    nw4r::g3d::ResAnmTexSrt anmTexSrt = mResFile.GetResAnmTexSrt("float");
    mAnmTexSrt.create(mdl, anmTexSrt, &mAllocator);
    mAnmTexSrt.setAnm(mModel, anmTexSrt, 0, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnmTexSrt, 1.0f);

    nw4r::g3d::ResAnmTexPat anmTexPat = mResFile.GetResAnmTexPat("balloon");
    mAnmTexPat.create(mdl, anmTexPat, &mAllocator);
    mAnmTexPat.setAnm(mModel, anmTexPat, 0, m3d::FORWARD_ONCE);
    mModel.setAnm(mAnmTexPat);

    float frame = 0.0f;
    if (dActorMng_c::m_instance->envAllWaterCheck()) {
        frame = 1.0f;
    }
    mAnmTexPat.setFrame(frame, 0);
    mAnmTexPat.setRate(0.0f, 0);

    nw4r::g3d::ResMdl backMdl = mResFile.GetResMdl("balloon_back");
    mBalloonModel.create(backMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC);

    nw4r::g3d::ResAnmChr backAnmChr = mResFile.GetResAnmChr("float_back");
    mBalloonAnmChr.create(backMdl, backAnmChr, &mAllocator);
    mBalloonAnmChr.setAnm(mBalloonModel, backAnmChr, m3d::FORWARD_LOOP);
    mBalloonModel.setAnm(mBalloonAnmChr, 1.0f);

    nw4r::g3d::ResAnmTexPat backAnmTexPat = mResFile.GetResAnmTexPat("balloon_back");
    mBalloonAnmTexPat.create(backMdl, backAnmTexPat, &mAllocator);
    mBalloonAnmTexPat.setAnm(mBalloonModel, backAnmTexPat, 0, m3d::FORWARD_ONCE);
    mBalloonModel.setAnm(mBalloonAnmTexPat);

    float backFrame = 0.0f;
    if (dActorMng_c::m_instance->envAllWaterCheck()) {
        backFrame = 1.0f;
    }
    mBalloonAnmTexPat.setFrame(backFrame, 0);
    mBalloonAnmTexPat.setRate(0.0f, 0);

    if (mBalloonType == 1) {
        nw4r::g3d::ResFile itemRes = dResMng_c::m_instance->getRes("I_kinoko", "g3d/I_kinoko.brres");
        nw4r::g3d::ResMdl itemMdl = itemRes.GetResMdl("I_kinoko");
        mItemModel.create(itemMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC);
        dActor_c::setSoftLight_Item(mItemModel);

        nw4r::g3d::ResAnmChr itemAnmChr = itemRes.GetResAnmChr("wait");
        mItemAnmChr.create(itemMdl, itemAnmChr, &mAllocator);
        mItemAnmChr.setAnm(mItemModel, itemAnmChr, m3d::FORWARD_LOOP);
        mItemModel.setAnm(mItemAnmChr, 1.0f);
        mItemAnmChr.setRate(0.5f);
    }

    mAllocator.adjustFrmHeap();
}

// ---------------------------------------------------------------- item_draw_calc
void daEnHatenaBalloon_c::item_draw_calc(mVec3_c *inPos) {
    mVec3_c pos = *inPos;
    mVec3_c scale = mItemDrawPos;
    mAng3_c angle(0, 0, 0);

    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.concat(mMtx_c::createTrans(0.0f, 8.0f, 0.0f));
    mMatrix.XrotM(angle.x);
    mMatrix.concat(mMtx_c::createTrans(0.0f, -8.0f, 0.0f));

    mItemModel.setLocalMtx(&mMatrix);
    mItemModel.setScale(scale);
    mItemModel.calc(false);
}

static const char *l_anm_name[] = {"float", "vibrate"};
static const char *l_anm_name_back[] = {"float_back", "vibrate_back"};


void daEnHatenaBalloon_c::anm_set(int anmNo) {
    nw4r::g3d::ResAnmChr anmChr = mResFile.GetResAnmChr(l_anm_name[anmNo]);
    mAnmChr.setAnm(mModel, anmChr, m3d::FORWARD_LOOP);
    mModel.setAnm(mAnmChr, 1.0f);
    mAnmChr.setRate(1.0f);

    nw4r::g3d::ResAnmChr backAnmChr = mResFile.GetResAnmChr(l_anm_name_back[anmNo]);
    mBalloonAnmChr.setAnm(mBalloonModel, backAnmChr, m3d::FORWARD_LOOP);
    mBalloonModel.setAnm(mBalloonAnmChr, 1.0f);
    mBalloonAnmChr.setRate(1.0f);

    mAnmNo = anmNo;
}

// ---------------------------------------------------------------- 0x80111F90
u8 daEnHatenaBalloon_c::pause_check() {
    if (!isState(StateID_HipAttack)) {
        if (m_81d != 0) {
            m_81d--;
        }

        if (m_81d != 0) {
            m_7c4 = -m_7c4;
        } else {
            m_7c4 = 0.0f;
        }

        return m_81d;
    }
    return 0;
}

// ---------------------------------------------------------------- 0x80112040
// Unnamed in the symbol map (fn_80112040), 0x88 bytes.  File-static free
// function with two in-TU callers (fly_xdisp_check, executeState_DispFlyMove),
// so `static` is correct.  Name invented -- see the report.
static float bg_dispx_get(daEnHatenaBalloon_c *balloon) {
    float bgX = dBg_c::m_bg_p->m_8fea8;
    if (std::fabs(bgX - dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(balloon->mPos.x))
        >= dBgParameter_c::ms_Instance_p->xSize()) {
        return dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(balloon->mPos.x);
    }
    return dBg_c::m_bg_p->m_8fea8;
}

// ---------------------------------------------------------------- 0x801120D0
void daEnHatenaBalloon_c::shake_disp_check() {
    if (m_7e4 != 0) {
        m_7e4--;
    }

    m_7c8 = 0.0f;
    if (m_7e4 == 1) {
        anm_set(0);
    }
}

// ---------------------------------------------------------------- 0x80112110
void daEnHatenaBalloon_c::createItem() {
    mVec3_c pos(mPos);
    pos.y += 8.0f;
    pos.z = 600.0f;

    dActor_c *item = dActor_c::construct(fProfile::EN_ITEM, 0x07000008, &pos, nullptr, 0);
    if (item != nullptr) {
        dBalloonMng_c::m_instance->setItemId(item->mUniqueID);
    }
    dBalloonMng_c::m_instance->m_18 = 0;

    dAudio::SndObjctCmnMap_c *snd = dAudio::g_pSndObjMap;
    snd->SndObjctCmnMap::startSound(0x286, dAudio::cvtSndObjctPos(mPos), 0);
}

// ---------------------------------------------------------------- 0x801121C0
void daEnHatenaBalloon_c::break_balloon(s16 mode) {
    if (mBalloonType == 0) {
        dAcPy_c *breaker = daPyMng_c::getPlayer(m_81f);
        if (breaker != nullptr) {
            dAcPy_c *owner = daPyMng_c::getPlayer(mPlayerNo);
            if (owner != nullptr) {
                owner->setBreakBalloonJump(m_81f, mode);
                owner->mLayer = 0;
                owner->mAmiLayer = breaker->mAmiLayer;
            }
        }
    } else {
        createItem();
    }
}

// ---------------------------------------------------------------- 0x80112260
bool daEnHatenaBalloon_c::player_set() {
    u8 bgHit = 0;
    int hit = all_bgcheck(bgHit);
    if (bgHit != 0 || hit != 0) {
        return true;
    }

    if (daPyMng_c::mAllBalloon == 0) {
        break_balloon(0);
        break_effect();
        mCc.release();
        m_822 = 0;
    }
    return false;
}

u32 daEnHatenaBalloon_c::pointBgCheck(const mVec3_c &pos, unsigned long xr, unsigned long yr, unsigned long skip) {
    sBcSensorLine oldFoot = mSensorFoot;
    sBcSensorLine oldHead = mSensorHead;
    sBcSensorLine oldWall = mSensorWall;
    mVec3_c oldPos = mPos;
    float wallOut = -1.0f;
    bool ret = false;

    mSensorFoot.mLineA = (s8) -xr << 12;
    mSensorFoot.mLineB = (s8) xr << 12;
    mSensorFoot.mDistanceFromCenter = (s8) yr << 12;
    mSensorHead.mLineA = (s8) -xr << 12;
    mSensorHead.mLineB = (s8) xr << 12;
    mSensorHead.mDistanceFromCenter = (s8) -yr << 12;
    mSensorWall.mLineA = (s8) -yr << 12;
    mSensorWall.mLineB = (s8) yr << 12;
    mSensorWall.mDistanceFromCenter = (s8) xr << 12;
    mPos = pos;

    if (!(skip & 2)) {
        if (mBc.checkFoot() && mBc.getFootAttr() != 3) {
            ret = true;
        }
    }

    if (!(skip & 1)) {
        if (mBc.checkHead(0) && mBc.getHeadAttr() != 3) {
            ret = true;
        }
    }

    if (!(skip & 4)) {
        if (mBc.checkWall(&wallOut) && mBc.getWallAttr(1) != 3) {
            ret |= 4;
        }
    }

    if (!(skip & 8)) {
        wallOut = 1.0f;
        if (mBc.checkWall(&wallOut) && mBc.getWallAttr(0) != 3) {
            ret |= 8;
        }
    }

    mSensorFoot = oldFoot;
    mSensorHead = oldHead;
    mSensorWall = oldWall;
    mPos = oldPos;
    return ret;
}

bool daEnHatenaBalloon_c::goalpole_check() {
    float goalX = dActorMng_c::m_instance->mGoalPoleX;
    if (-8000.0f != goalX) {
        if (8.0f + mPos.x >= goalX) {
            return true;
        }
    }
    return false;
}

u8 daEnHatenaBalloon_c::floor_check() {
    u8 ret = 0;
    mVec2_c pt;

    pt.set(mPos.x - 7.0f, 3.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 1;
    }
    pt.set(7.0f + mPos.x, 3.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 2;
    }
    pt.set(mPos.x - 7.0f, 29.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 4;
    }
    pt.set(7.0f + mPos.x, 29.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 8;
    }
    pt.set(mPos.x - 7.0f, 5.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 0x10;
    }
    pt.set(mPos.x - 7.0f, 24.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 0x20;
    }
    pt.set(7.0f + mPos.x, 5.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 0x40;
    }
    pt.set(7.0f + mPos.x, 24.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 0x80;
    }
    return ret;
}

// Two shapes in this function are load-bearing for register allocation. Both
// were established by compiling the alternatives and diffing, not by taste:
//
//  * The loop variables are DECLARED HERE and assigned below. Moving each
//    declaration down to its natural point of use leaves the code semantically
//    identical and byte-identical in length, but rotates the callee-saved GPRs
//    (r27/r28/r29/r31 swap roles) and the function stops matching. Do not
//    "clean this up".
//  * The probe point is built as a copy of mPos with the two offsets added in
//    place, NOT as mVec3_c pt(mPos.x + ofs[0], mPos.y + ofs[1], mPos.z). The
//    constructor form emits the same instructions in the same order but with a
//    4-way permutation of f0-f3. Fifteen other spellings were tried; this is
//    the only one that colours the FP temporaries the way the original does.
int daEnHatenaBalloon_c::all_bgcheck(u8 &floorFlags) {
    u32 flag;
    const checkData_s *row;
    const float *ofs;
    u8 ret;
    u32 i;
    u32 hit;
    u32 j;

    if (goalpole_check()) {
        floorFlags = 0xFF;
        return 0xF;
    }

    if (m_81f != -1) {
        dAcPy_c *player = daPyMng_c::getPlayer(m_81f);
        if (player != nullptr) {
            mBc.mLayer = player->mLayer;
        }
    }

    ret = 0;
    row = s_someCheckData;
    for (i = 0; i < 4; i++) {
        flag = row->mFlag;
        ofs = &row->mOffsetX;
        hit = 0;
        for (j = 0; j < 2; j++) {
            mVec3_c pt(mPos);
            pt.x += ofs[0];
            pt.y += ofs[1];
            if (!(ret & flag)) {
                hit |= pointBgCheck(pt, 1, 1, hit);
                if (hit != 0) {
                    ret |= (u8) row->mFlag;
                }
            }
            ofs += 2;
        }
        row++;
    }

    floorFlags = floor_check();

    float waterOut = 0.0f;
    dAcPy_c *player = daPyMng_c::getPlayer(m_81f);
    if (player != nullptr) {
        int type = dBc_c::checkWater(mPos.x, mPos.y, player->mLayer, &waterOut);
        if (type >= dBc_c::WATER_CHECK_YOGAN && type <= dBc_c::WATER_CHECK_POISON) {
            ret = 1;
        }
    }
    return ret;
}

// ---------------------------------------------------------------- 0x80112950
void daEnHatenaBalloon_c::fly_yspeed_set() {
    float sy = dBgParameter_c::ms_Instance_p->mSize.y;
    float half = 0.5f * sy;
    float midY = dBgParameter_c::ms_Instance_p->mPos.y - half;
    float dist = std::fabs((16.0f + mPos.y) - midY);
    // `half / 8.0f`, not `half * 0.125f`: MWCC folds the divide to a reciprocal
    // multiply and the folded form emits fmuls(numerator, reciprocal).  Spelling
    // it as an explicit multiply transposes the operands.
    float unit = half / 8.0f;
    float r = dGameCom::rnd();
    int flip = 0;
    if (dist < 6.0f * unit) {
        if (r < 0.5f) {
            flip = 1;
        }
    } else if (dist < 7.0f * unit) {
        if (r < 0.7f) {
            flip = 1;
        }
    } else {
        flip = 1;
    }

    if (m_81e == 0) {
        m_81e = 1;
        m_7cc = -sm_hio_fly_yspeed;
    } else {
        m_81e = 0;
        m_7cc = sm_hio_fly_yspeed;
    }

    if (flip != 0) {
        m_820 = 1;
        mTimer2 = 100;
        if (m_81e == 0) {
            if (mPos.y < midY) {
                mTimer2 = 150;
            }
        } else {
            if (mPos.y > midY) {
                mTimer2 = 150;
            }
        }
    } else {
        mTimer2 = 100;
        if (m_81e == 0) {
            if (mPos.y > midY) {
                mTimer2 = 150;
            }
        } else {
            if (mPos.y < midY) {
                mTimer2 = 150;
            }
        }
    }
}

// ---------------------------------------------------------------- 0x80112B00
void daEnHatenaBalloon_c::fly_xspeed_set(bool force) {
    float sp = 0.4f;
    if (!force) {
        float sx = dBgParameter_c::ms_Instance_p->mSize.x;
        float half = 0.5f * sx;
        float midX = half + dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x);
        float unit = half / 6.0f;
        float dist = std::fabs(mPos.x - midX);
        float r = dGameCom::rnd();
        int flip = 0;
        if (dist < 3.0f * unit) {
            if (r < 0.5f) {
                flip = 1;
            }
        } else if (dist < 5.0f * unit) {
            if (r < 0.7f) {
                flip = 1;
            }
        } else {
            flip = 1;
        }
        if (flip != 0) {
            if (mPos.x < midX) {
                mSpeedF = sp;
            } else {
                mSpeedF = -sp;
            }
            return;
        }
    }

    float r2 = dGameCom::rnd();
    if (r2 < 0.8f || force) {
        if (r2 < 0.3f) {
            mSpeedF = sp;
        } else {
            mSpeedF = -sp;
        }
    }
}

// ---------------------------------------------------------------- 0x80112C70
// NOT byte-exact: 2 words out of 55.  The original loads dBgParameter_c's mPos.y
// BEFORE dBg_c's m_8feac; this spelling schedules them the other way round.  Same
// registers, same instructions, same count -- purely the order of two adjacent,
// independent lfs.  ~80 source variants were swept without moving it; the axes that
// were eliminated are listed in the report.  Everything else in the function,
// including the `!scroll` below, is confirmed exact.
bool daEnHatenaBalloon_c::fly_ydisp_check(bool bounce) {
    float lim = 7.0f;
    float scroll = -(dBg_c::m_bg_p->m_8feac - dBgParameter_c::ms_Instance_p->mPos.y);
    if (scroll < -lim) {
        scroll = -lim;
    }
    if (scroll > lim) {
        scroll = lim;
    }

    float ySpeed = mSpeed.y;
    int hit = 0;
    float top = dBgParameter_c::ms_Instance_p->mPos.y - 24.0f;
    if (mPos.y > top) {
        mPos.y = top;
        hit = 1;
        if (ySpeed > scroll) {
            mSpeed.y = 0.5f * scroll;
        }
    } else {
        float bottom = dBgParameter_c::ms_Instance_p->mPos.y - dBgParameter_c::ms_Instance_p->mSize.y;
        if (mPos.y < bottom) {
            mPos.y = bottom;
            hit = 2;
            if (ySpeed < scroll) {
                mSpeed.y = 0.5f * scroll;
            }
        }
    }

    if (hit != 0 && bounce) {
        // `!scroll`, not `scroll == 0.0f`: the explicit comparison emits
        // fcmpu(0.0, scroll); this emits fcmpu(scroll, 0.0), which is what the
        // original has.  Reversing the operands in the source does not help.
        if (!scroll) {
            mSpeed.y = -ySpeed;
        }
    }
    return hit;
}

// ---------------------------------------------------------------- 0x80112D50
bool daEnHatenaBalloon_c::fly_xdisp_check(bool bounce) {
    float lim = 7.0f;
    float scroll = -(bg_dispx_get(this) - dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x));
    if (scroll < -lim) {
        scroll = -lim;
    }
    if (scroll > lim) {
        scroll = lim;
    }

    // `u32`, not `int`: the original compares `hit == 1` with cmplwi.  With a
    // signed int MWCC emits cmpwi there (and `hit != 0` stays cmpwi either way).
    u32 hit = 0;
    if (mPos.x < 16.0f + dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)) {
        hit = 1;
        mPos.x = 16.0f + dBgParameter_c::ms_Instance_p->xStart();
        if (mSpeedF < scroll) {
            mSpeedF = 0.5f * scroll;
        }
    } else if (mPos.x > dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                            + dBgParameter_c::ms_Instance_p->xSize() - 16.0f) {
        mPos.x = dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                     + dBgParameter_c::ms_Instance_p->xSize() - 16.0f;
        // `hit = 2` after the store, unlike `hit = 1` above: the original schedules
        // the `li` late in this branch and early in the other one.
        hit = 2;
        if (mSpeedF > scroll) {
            mSpeedF = 0.5f * scroll;
        }
    }

    if (hit != 0 && bounce) {
        if (std::fabs(scroll) < 0.4f) {
            if (hit == 1) {
                mSpeedF = 0.4f;
            } else {
                mSpeedF = -0.4f;
            }
        }
    }
    return hit;
}

// ---------------------------------------------------------------- 0x80112EF0
bool daEnHatenaBalloon_c::fly_dispin_check() {
    float by = dBgParameter_c::ms_Instance_p->mPos.y;
    if (mPos.y <= by - 24.0f
        && mPos.y >= by - dBgParameter_c::ms_Instance_p->mSize.y
        && mPos.x >= 16.0f + dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)) {
        if (mPos.x <= dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                          + dBgParameter_c::ms_Instance_p->xSize() - 16.0f) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------- 0x80112FC0
bool daEnHatenaBalloon_c::escape_dispout_check() {
    float by = dBgParameter_c::ms_Instance_p->mPos.y;
    if (mPos.y <= 32.0f + by
        && mPos.y >= by - dBgParameter_c::ms_Instance_p->mSize.y - 32.0f
        && mPos.x >= dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x) - 48.0f) {
        if (mPos.x <= 48.0f + (dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                                   + dBgParameter_c::ms_Instance_p->xSize())) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------- 0x80113090
// @note The four coin-battle constants below are written as `base * 1.4f`
// rather than as folded literals.  Three of them fold to the same word either
// way, but `0.3f * 1.4f` folds to 0x3ED70A3E while the literal `0.42f` folds to
// 0x3ED70A3D -- and 0x3ED70A3E is what the DOL holds.  The .text comparator
// cannot see this: it canonicalises pool references positionally.
void daEnHatenaBalloon_c::remocon_speed_set() {
    mVec2_c delta;
    dAcPy_c *player = searchNearPlayer(delta);
    if (player == nullptr) {
        return;
    }

    if (dInfo_c::mGameFlag & dInfo_c::GAME_FLAG_IS_COIN_BATTLE) {
        if (player->mPos.x > mPos.x) {
            if (mSpeed.x > 0.0f) {
                mSpeed.x += (0.9f * 1.4f);
                mSpeedF += (0.3f * 1.4f);
            } else {
                mSpeed.x += (1.8f * 1.4f);
                mSpeedF += (0.45f * 1.4f);
            }
        } else {
            if (mSpeed.x < 0.0f) {
                mSpeed.x -= (0.9f * 1.4f);
                mSpeedF -= (0.3f * 1.4f);
            } else {
                mSpeed.x -= (1.8f * 1.4f);
                mSpeedF -= (0.45f * 1.4f);
            }
        }

        if (player->mPos.y > 16.0f + mPos.y) {
            if (mSpeed.y > 0.0f) {
                mSpeed.y += (0.9f * 1.4f);
                m_7cc += (0.3f * 1.4f);
            } else {
                mSpeed.y += (1.8f * 1.4f);
                m_7cc += (0.45f * 1.4f);
            }
        } else {
            if (mSpeed.y < 0.0f) {
                mSpeed.y -= (0.9f * 1.4f);
                m_7cc -= (0.3f * 1.4f);
            } else {
                mSpeed.y -= (1.8f * 1.4f);
                m_7cc -= (0.45f * 1.4f);
            }
        }
    } else {
        if (player->mPos.x > mPos.x) {
            if (mSpeed.x > 0.0f) {
                mSpeed.x += 0.9f;
                mSpeedF += 0.3f;
            } else {
                mSpeed.x += 1.8f;
                mSpeedF += 0.45f;
            }
        } else {
            if (mSpeed.x < 0.0f) {
                mSpeed.x -= 0.9f;
                mSpeedF -= 0.3f;
            } else {
                mSpeed.x -= 1.8f;
                mSpeedF -= 0.45f;
            }
        }

        if (player->mPos.y > 16.0f + mPos.y) {
            if (mSpeed.y > 0.0f) {
                mSpeed.y += 0.9f;
                m_7cc += 0.3f;
            } else {
                mSpeed.y += 1.8f;
                m_7cc += 0.45f;
            }
        } else {
            if (mSpeed.y < 0.0f) {
                mSpeed.y -= 0.9f;
                m_7cc -= 0.3f;
            } else {
                mSpeed.y -= 1.8f;
                m_7cc -= 0.45f;
            }
        }
    }
}

// ---------------------------------------------------------------- 0x801133A0
bool daEnHatenaBalloon_c::break_speed_set() {
    mVec3_c target(m_7b0);
    target.z = mPos.z;
    return cLib::chasePos(&mPos, target, 1.5f);
}

// ---------------------------------------------------------------- 0x80113400
void daEnHatenaBalloon_c::remocon_times_check() {
    m_7f8++;
    if (m_7f8 >= 3) {
        dGameCom::hideFukidashiForLevel(mPlayerNo, 0x14, 0);
        m_7f8 = 3;
    }
}

// ---------------------------------------------------------------- 0x80113460
bool daEnHatenaBalloon_c::player_out_check() {
    if (mBalloonType == 1) {
        return false;
    }

    dAcPy_c *player = daPyMng_c::getPlayer(mPlayerNo);
    if (player != nullptr && !player->isStatus(0x53)) {
        break_balloon(0);
        break_effect();
        mCc.release();
        m_822 = 0;
        return true;
    }
    return false;
}

// ---------------------------------------------------------------- 0x801134F0
void daEnHatenaBalloon_c::remocon_shake_check() {
    if (mBalloonType == 1) {
        return;
    }

    if (m_7e0 != 0) {
        m_7e0--;
    } else {
        dAcPy_c *player = daPyMng_c::getPlayer(mPlayerNo);
        if (player->mKey.triggerShakeJump()) {
            m_7e0 = 30;
            m_7e4 = 31;
            remocon_times_check();
            dQuake_c::m_instance->shockMotor(mPlayerNo, (dQuake_c::TYPE_SHOCK_e)10, 0, false);
            player->setBalloonHelpVoice();
            anm_set(1);
            remocon_speed_set();
        }
    }
}

// ---------------------------------------------------------------- 0x801135B0
void daEnHatenaBalloon_c::ButtonPlayerColSet() {
    if (m_7fc == 1) {
        mCc.entry();
    }
}

// ---------------------------------------------------------------- 0x801135D0
void daEnHatenaBalloon_c::break_effect() {
    if (mBalloonType == 0) {
        dGameCom::hideFukidashiTemporarily(mPlayerNo, 0x14, 0);
    }
    mEf::createEffect("Wm_mr_balloonburst", 0, &m_78c, nullptr, nullptr);
    deleteActor(1);
}

// ---------------------------------------------------------------- 0x80113640
bool daEnHatenaBalloon_c::dispInFlyInitCheck(int mode) {
    if (fly_dispin_check()) {
        bool isButton = false;
        if (mBalloonType == 0 && m_814 == 2) {
            isButton = true;
        }
        if (!isButton) {
            mCc.entry();
        }

        if (mode != 0) {
            dEnemyMng_c::m_instance->m_110--;
            if (dEnemyMng_c::m_instance->m_110 > 3) {
                dEnemyMng_c::m_instance->m_110 = 0;
            }
        }

        fly_xspeed_set(true);

        switch (mDirection) {
            case 2:
                mSpeed.y = -1.7f;
                break;
            case 3:
                mSpeed.y = 1.7f;
                break;
        }

        changeState(StateID_Fly);
        return true;
    }
    return false;
}

// ---------------------------------------------------------------- 0x80113740
void daEnHatenaBalloon_c::create_wait_pos_set() {
    if (m_814 != 0) {
        return;
    }

    mPos.x = dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x);
    float top = dBgParameter_c::ms_Instance_p->mPos.y;
    mPos.y = top;

    // The three `float x`/`float y` locals below are required.  Adding the
    // l_create_diff term directly to the full expression emits fadds(diff, value);
    // binding the value first emits fadds(value, diff), which is the original.
    // The mDirection == 3 branch is different again: there the original keeps BOTH
    // stores to mPos.x, which only survives if the mPos.y statement sits between
    // them.
    if (mDirection == 2) {
        float x = mPos.x + 0.5f * dBgParameter_c::ms_Instance_p->xSize();
        mPos.x = x + l_create_diff[m_7f0];
    } else if (mDirection == 3) {
        mPos.x = mPos.x + 0.5f * dBgParameter_c::ms_Instance_p->xSize();
        mPos.y = top - (32.0f + dBgParameter_c::ms_Instance_p->ySize());
        mPos.x = mPos.x + l_create_diff[m_7f0];
    } else if (mDirection == 0) {
        mPos.x = mPos.x + (32.0f + dBgParameter_c::ms_Instance_p->xSize());
        float y = top - (32.0f + 0.5f * dBgParameter_c::ms_Instance_p->ySize());
        mPos.y = y + l_create_diff[m_7f0];
    } else {
        mPos.x = mPos.x - 32.0f;
        float y = top - (32.0f + 0.5f * dBgParameter_c::ms_Instance_p->ySize());
        mPos.y = y + l_create_diff[m_7f0];
    }
}

// ---------------------------------------------------------------- 0x801138D0
void daEnHatenaBalloon_c::initializeState_DispFlyWait() {
    m_798 = mPos;
    mTimer1 = 10;
    mMaxFallSpeed = -4.0f;
    mAccelF = 0.02f;
    mAccelY = 0.0f;
    mSpeed.y = 0.0f;
    mSpeedF = 0.0f;

    // Rotating 0..3 slot index, shared by every balloon waiting to fly in;
    // create_wait_pos_set() uses it to fan the off-screen wait positions out.
    u32 idx = dEnemyMng_c::m_instance->m_110;
    if (idx > 3) {
        dEnemyMng_c::m_instance->m_110 = 0;
        idx = 0;
    }
    m_7f0 = idx;

    if (m_7f4 == 0) {
        int dir = dBg_c::m_bg_p->m_90009;
        if (m_814 == 0) {
            // The shared `case 0: case 3:` arm is load-bearing: it is what
            // puts the compare chain in the order 1, 0, 3 while leaving the
            // case bodies laid out 2, 3, 0, default. A plain switch emits
            // compares and bodies in the SAME order and is 4 words wrong.
            switch (dir) {
                case 1:
                    mDirection = 2;
                    break;
                case 0:
                case 3:
                    if (dir == 3) {
                        mDirection = 3;
                    } else {
                        mDirection = 0;
                    }
                    break;
                default:
                    mDirection = 1;
                    break;
            }
        } else if (dir == 1 || dir == 3) {
            if (dBgParameter_c::ms_Instance_p->yStart()
                - 0.5f * dBgParameter_c::ms_Instance_p->ySize() > mPos.y) {
                mDirection = 3;
            } else {
                mDirection = 2;
            }
        } else {
            float half = 0.5f * dBgParameter_c::ms_Instance_p->xSize();
            if (half + dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x) > mPos.x) {
                mDirection = 1;
            } else {
                mDirection = 0;
            }
        }
    }

    create_wait_pos_set();
    dEnemyMng_c::m_instance->m_110 = dEnemyMng_c::m_instance->m_110 + 1;
}

// ---------------------------------------------------------------- 0x80113A80
void daEnHatenaBalloon_c::finalizeState_DispFlyWait() {}

// ---------------------------------------------------------------- 0x80113A90
void daEnHatenaBalloon_c::executeState_DispFlyWait() {
    if (m_814 == 2) {
        fly_xdisp_check(true);
        fly_ydisp_check(true);
    }
    create_wait_pos_set();

    if (mTimer1 != 0) {
        return;
    }
    if (m_814 == 0) {
        switch (mDirection) {
            case 2:
            case 3:
                fly_ydisp_check(false);
                if (mDirection == 2) {
                    mPos.y = 32.0f + dBgParameter_c::ms_Instance_p->yStart();
                } else {
                    mPos.y = dBgParameter_c::ms_Instance_p->yEnd() - 32.0f;
                }
                break;
            case 0:
            case 1:
                fly_xdisp_check(false);
                if (mDirection == 0) {
                    mPos.x = 32.0f + (dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x)
                                      + dBgParameter_c::ms_Instance_p->xSize());
                } else {
                    mPos.x = dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x) - 32.0f;
                }
                break;
        }
    }
    changeState(StateID_DispFlyMove);
}

// ---------------------------------------------------------------- 0x80113BF0
void daEnHatenaBalloon_c::initializeState_DispFlyMove() {
    if (mBalloonType != 0) {
        return;
    }
    dGameCom::showFukidashi(mPlayerNo, 20);
}

// ---------------------------------------------------------------- 0x80113C10
void daEnHatenaBalloon_c::finalizeState_DispFlyMove() {}

// ---------------------------------------------------------------- 0x80113C20
void daEnHatenaBalloon_c::executeState_DispFlyMove() {
    // bgpY / bgDispY must be named locals, in this order: they are what puts
    // the two loads in the target's order without transposing f0 and f2.
    float bgpY = dBgParameter_c::ms_Instance_p->mPos.y;
    float bgDispY = dBg_c::m_bg_p->m_8feac;
    float yScroll = -(bgDispY - bgpY);
    float xScroll = -(bg_dispx_get(this)
                      - dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x));

    int dir = mDirection;
    switch (dir) {
        case 2:
            fly_xdisp_check(false);
            mSpeed.y = -1.7f;
            if (mSpeed.y > yScroll) {
                mSpeed.y = yScroll;
            }
            break;
        case 3:
            fly_xdisp_check(false);
            mSpeed.y = 1.7f;
            if (mSpeed.y < yScroll) {
                mSpeed.y = yScroll;
            }
            break;
        case 0:
            fly_ydisp_check(false);
            mSpeedF = -1.7f;
            if (mSpeedF > xScroll - 1.0f) {
                mSpeedF = xScroll - 1.0f;
            }
            break;
        case 1:
            fly_ydisp_check(false);
            mSpeedF = 1.7f;
            if (mSpeedF < 1.0f + xScroll) {
                mSpeedF = 1.0f + xScroll;
            }
            break;
    }

    dispInFlyInitCheck(1);
    ButtonPlayerColSet();
    mSpeedMax.x = mSpeedF;
    mSpeed.z = 0.0f;
    calcSpeedX();
    posMove();

    if (m_814 == 2) {
        fly_xdisp_check(true);
        fly_ydisp_check(true);
    }
}

// ---------------------------------------------------------------- 0x80113DC0
void daEnHatenaBalloon_c::initializeState_Fly() {
    mTimer1 = 0;
    mTimer2 = 50;
    m_7cc = -sm_hio_fly_yspeed;
    m_820 = 0;
    m_81e = 1;
}

// ---------------------------------------------------------------- 0x80113DF0
void daEnHatenaBalloon_c::finalizeState_Fly() {}

// ---------------------------------------------------------------- 0x80113E00
void daEnHatenaBalloon_c::executeState_Fly() {
    if (mTimer1 == 0) {
        fly_xspeed_set(false);
        mTimer1 = sm_hio_base_fly_timer_x;
    }

    if (mTimer2 == 0) {
        fly_yspeed_set();
    } else {
        sLib::chase(&mSpeed.y, m_7cc, 0.02f);
    }

    fly_ydisp_check(true);
    fly_xdisp_check(true);
    ButtonPlayerColSet();
    mSpeedMax.x = mSpeedF;
    calcSpeedX();
    remocon_shake_check();
    mSpeed.z = 0.0f;
    posMove();
    player_out_check();
}

// ---------------------------------------------------------------- 0x80113ED0
void daEnHatenaBalloon_c::initializeState_Escape() {
    mCc.release();

    float half = 0.5f * dBgParameter_c::ms_Instance_p->xSize();
    float midX = half + dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x);

    if (dEnemyMng_c::m_instance->m_15c == 1) {
        if (midX >= mPos.x) {
            mSpeedF = -0.8f;
        } else {
            mSpeedF = 0.8f;
        }
    } else {
        mSpeedF = -0.8f;
    }
}

// ---------------------------------------------------------------- 0x80113F70
void daEnHatenaBalloon_c::finalizeState_Escape() {}

// ---------------------------------------------------------------- 0x80113F80
void daEnHatenaBalloon_c::executeState_Escape() {
    if (escape_dispout_check()) {
        if (mTimer2 == 0) {
            fly_yspeed_set();
        } else {
            sLib::chase(&mSpeed.y, m_7cc, 0.02f);
        }
        mSpeedMax.x = mSpeedF;
        mSpeed.z = 0.0f;
        calcSpeedX();
        posMove();
    }

    if (dEnemyMng_c::m_instance->m_15c == 0) {
        if (dBgParameter_c::ms_Instance_p->getLoopScrollDispPosX(mPos.x) >= mPos.x) {
            mDirection = 1;
        } else {
            mDirection = 0;
        }
        if (m_814 == 2) {
            m_814 = 0;
        }
        if (fly_dispin_check()) {
            mCc.entry();
            changeState(StateID_Fly);
        } else {
            m_7f4 = 1;
            changeState(StateID_DispFlyWait);
        }
    }

    ButtonPlayerColSet();
    player_out_check();
}

// ---------------------------------------------------------------- 0x801140C0
void daEnHatenaBalloon_c::initializeState_HipAttack() {}

// ---------------------------------------------------------------- 0x801140D0
void daEnHatenaBalloon_c::finalizeState_HipAttack() {}

// ---------------------------------------------------------------- 0x801140E0
void daEnHatenaBalloon_c::executeState_HipAttack() {
    player_set();
    changeState(StateID_SearchSpace);
    fly_ydisp_check(true);
    fly_xdisp_check(true);
}

// ---------------------------------------------------------------- 0x80114140
void daEnHatenaBalloon_c::initializeState_SearchSpace() {
    m_7e0 = 30;
    m_804 = 0;
}

// ---------------------------------------------------------------- 0x80114160
void daEnHatenaBalloon_c::finalizeState_SearchSpace() {
    m_881 = 0;
    mSpeed.x = m_890.x;
    mSpeed.y = m_890.y;
    mSpeed.z = 0.0f;
}

// ---------------------------------------------------------------- 0x80114190
void daEnHatenaBalloon_c::executeState_SearchSpace() {
    if (m_881 == 0) {
        m_884 = mPos;
    }

    int moved = 0;
    if (break_speed_set()) {
        moved = 1;
    }

    if (m_881 == 0) {
        // `m_890 = mPos - m_884;` and not a named difference vector: the
        // by-value operator- temporary lands in the LOW stack slot, which is
        // where the target keeps it. A named local takes the high slot and
        // swaps places with checkPos below.
        m_890 = mPos - m_884;
        m_890.z = 0.0f;
        m_881 = 1;
    }

    bool yHit = fly_ydisp_check(true);
    if (fly_xdisp_check(true) || yHit) {
        moved = 1;
    }

    if (moved != 0) {
        float dz = m_7d4.z;
        mVec3_c checkPos(mPos.x, mPos.y + dz, mPos.z);
        u32 bgRes = pointBgCheck(checkPos, (u32)m_7d4.x, (u32)m_7d4.y, 0);
        int pole = 0;
        if (goalpole_check()) {
            pole = 0xFF;
        }

        int doBreak = 0;
        if (mHitFlag != 0) {
            if (bgRes == 0 && pole == 0) {
                doBreak = 1;
                mHitFlag = 0;
                m_87c = 30;
                m_880 = 1;
            } else {
                float dx = std::fabs(mPos.x - mHitPos.x);
                if (dx <= 2.0f) {
                    float dy = std::fabs(mPos.y - mHitPos.y);
                    if (dy <= 2.0f) {
                        doBreak = 0;
                    }
                }
            }
        }
        if (daPyMng_c::mAllBalloon != 0) {
            doBreak = 0;
        }

        if (doBreak != 0) {
            break_balloon(0);
            break_effect();
            mCc.release();
            m_822 = 0;
        } else {
            changeState(StateID_Fly);
            m_828 = m_7b0;
            m_87c = 30;
            mHitFlag = 0;
        }
    } else if (mLastPos.distTo(mPos) < 0.02f) {
        m_804 = m_804 + 1;
        if (m_804 >= 5) {
            changeState(StateID_Fly);
            m_828 = m_7b0;
            m_804 = 0;
        }
    }
}

// ---------------------------------------------------------------- 0x80114470
BOOL daEnHatenaBalloon_c::isQuakeDamage() {
    return false;
}

// ---------------------------------------------------------------- 0x80114480
daEnHatenaBalloon_c::~daEnHatenaBalloon_c() {}
