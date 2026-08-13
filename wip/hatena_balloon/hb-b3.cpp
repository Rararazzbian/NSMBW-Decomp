// ---------------------------------------------------------------------------
// BATCH 3 -- collision core + the verified twin cluster.
// 15 functions, .text 0x80110DE0 .. 0x8011198C, canonical address order.
// Every function below is byte-exact against the DOL.
//
// NOTE for the integrator: block_hit_init (0x801118D0) sits between hitYoshiEat
// and hipattackhit in .text and is owned by batch 1; insert it there.
// ---------------------------------------------------------------------------
#include <game/bases/d_a_en_hatena_balloon.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_actor_manager.hpp>

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

// ---------------------------------------------------------------- 0x801118C0
void daEnHatenaBalloon_c::hitYoshiEat(dCc_c *self, dCc_c *other) {
    PlYsHitCheck(other->mpOwner, (daEnHatenaBalloon_c *)self->mpOwner);
}

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
