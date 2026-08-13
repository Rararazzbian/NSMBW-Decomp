// BATCH 4 -- heavy collision, execute, and the cc lines.
// Functions in .text address order.
#include <game/bases/d_a_en_hatena_balloon.hpp>
#include <game/bases/d_a_en_carry.hpp>
#include <game/bases/d_enemy_manager.hpp>
#include <game/bases/d_actor_manager.hpp>

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
