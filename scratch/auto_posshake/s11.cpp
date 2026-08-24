#include <game/bases/d_pos_shake.hpp>

void dPosShake_c::init(f32 k, f32 damp, f32 limit, f32 offset, f32 vel, f32 snap) {
    mK = k;
    mDamp = damp;
    mLimit = limit;
    mOffset = offset;
    mVel = vel;
    mSnap = snap;
}

void dPosShake_c::move() {
    mVel -= mOffset * mK;

    if (mVel < 0.0f) {
        mVel += mDamp;
        if (mVel >= 0.0f) {
            mVel = 0.0f;
        }
    } else {
        mVel -= mDamp;
        if (mVel <= 0.0f) {
            mVel = 0.0f;
        }
    }

    if (mVel >= mLimit) {
        mVel = mLimit;
    } else if (mVel <= -mLimit) {
        mVel = -mLimit;
    }

    if (mVel >= -mSnap && mVel <= mSnap) {
        if (mOffset < 0.0f) {
            mOffset += mSnap;
            if (mOffset >= 0.0f) {
                mOffset = 0.0f;
                mVel = 0.0f;
            }
        } else {
            mOffset -= mSnap;
            if (mOffset <= 0.0f) {
                mOffset = 0.0f;
                mVel = 0.0f;
            }
        }
    } else {
        mOffset += mVel;
    }
}

void dPosShake_c::startShake(f32 amt) {
    mVel += amt;
}
