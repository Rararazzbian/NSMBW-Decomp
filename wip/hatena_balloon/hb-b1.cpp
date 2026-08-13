#include <game/bases/d_a_en_hatena_balloon.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_manager.hpp>

// ---------------------------------------------------------------------------
// B1: lifecycle, class layout, __sinit and the sFStateID_c tail.
//
// Functions delivered, in .text address order:
//   0x801102B0 daEnHatenaBalloon_c_classInit  (emitted by ACTOR_PROFILE)
//   0x80110410 create
//   0x80110C40 doDelete
//   0x801118D0 block_hit_init
//   0x80114470 isQuakeDamage
//   0x80114480 ~daEnHatenaBalloon_c
//   0x80114580 __sinit_\d_a_en_hatena_balloon_cpp     (emitted)
//   0x80114A80 sFStateID_c<daEnHatenaBalloon_c> tail: __dt, isSameName,
//              initializeState, executeState, finalizeState  (emitted)
//
// ORDERING CONSTRAINTS FOR THE INTEGRATOR -- all four are load-bearing:
//
//  1. ACTOR_PROFILE and the six STATE_DEFINEs come first: they are the first
//     seven .data / .bss objects of the TU and the first six initialisers in
//     __sinit, and the .bss addresses (0x80375408 .. 0x80375548, 0x30 apart)
//     pin the state order exactly as written.
//
//  2. The three sm_bg_check_size_* definitions must sit HERE -- after the six
//     STATE_DEFINEs and BEFORE create(). This corrects the earlier note that
//     said they go last in the TU. They are .bss 0x80375578/0x584/0x590,
//     i.e. +0x180/+0x18c/+0x198 from the TU's first .bss object, and create()
//     addresses all three AND StateID_DispFlyWait off ONE base register
//     (`lis r31, <bss base>; addi r6, r31, 0x180`). MWCC only anchors a static
//     that way once it has seen the DEFINITION; with the definitions after
//     create() it falls back to a separate lis/addi per symbol and create()
//     mismatches in ~40 instructions. Their .sdata2 literals are unaffected by
//     the move: they are materialised only in __sinit, which is emitted last
//     whatever the source order, so 1.5/18/10/22 still land at the end of the
//     pool (0x8042D6F0, 0x8042D704..0x8042D70C) as in the original.
//
//  3. l_hatenaballoon_cullinfo and l_cc_data are .rodata 0x802F4E70 and
//     0x802F4E80: AFTER B5's s_someCheckData (0x802F4E20, 0x50) and BEFORE
//     B6's l_create_diff (0x802F4EA8).
//
//  4. ~daEnHatenaBalloon_c is defined OUT OF LINE and LAST among this batch's
//     functions (0x80114480, immediately before __sinit).
// ---------------------------------------------------------------------------

ACTOR_PROFILE(EN_HATENA_BALLOON, daEnHatenaBalloon_c, 0x46);

STATE_DEFINE(daEnHatenaBalloon_c, DispFlyWait);
STATE_DEFINE(daEnHatenaBalloon_c, DispFlyMove);
STATE_DEFINE(daEnHatenaBalloon_c, Fly);
STATE_DEFINE(daEnHatenaBalloon_c, Escape);
STATE_DEFINE(daEnHatenaBalloon_c, HipAttack);
STATE_DEFINE(daEnHatenaBalloon_c, SearchSpace);

mVec3_c daEnHatenaBalloon_c::sm_bg_check_size_mame(1.5f, 1.5f, 16.0f);
mVec3_c daEnHatenaBalloon_c::sm_bg_check_size_normal(4.0f, 4.0f, 18.0f);
mVec3_c daEnHatenaBalloon_c::sm_bg_check_size_super(4.0f, 10.0f, 22.0f);

float daEnHatenaBalloon_c::sm_hio_gravity = -0.08f;
float daEnHatenaBalloon_c::sm_hio_base_fly_timer_x = 200.0f;
float daEnHatenaBalloon_c::sm_hio_fly_yspeed = 0.4f;
float daEnHatenaBalloon_c::sm_hio_mask_size = 80.0f;
float daEnHatenaBalloon_c::sm_hio_mask_y_diff = 15.0f;

/// @brief The visible-area rectangle: [0] offset, [1] size. @unofficial
static const mVec2_POD_c l_hatenaballoon_cullinfo[] = {
    {0.0f, 0.0f},
    {64.0f, 64.0f}
};

/// @brief The collider template. @unofficial
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

// ---------------------------------------------------------------- 0x80110C40
int daEnHatenaBalloon_c::doDelete() {
    return true;
}

// ---------------------------------------------------------------- 0x801118D0
void daEnHatenaBalloon_c::block_hit_init() {}

// ---------------------------------------------------------------- 0x80114470
BOOL daEnHatenaBalloon_c::isQuakeDamage() {
    return false;
}

// ---------------------------------------------------------------- 0x80114480
daEnHatenaBalloon_c::~daEnHatenaBalloon_c() {}
