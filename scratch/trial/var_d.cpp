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
    f32 vel = mVel;
    vel -= mOffset * mK;

    if (vel < 0.0f) {
        vel += mDamp;
        if (vel >= 0.0f) {
            vel = 0.0f;
        }
    } else {
        vel -= mDamp;
        if (vel <= 0.0f) {
            vel = 0.0f;
        }
    }

    if (vel >= mLimit) {
        vel = mLimit;
    } else if (vel <= -mLimit) {
        vel = -mLimit;
    }

    if (vel >= -mSnap && vel <= mSnap) {
        if (mOffset < 0.0f) {
            mOffset += mSnap;
            if (mOffset >= 0.0f) {
                mOffset = 0.0f;
                vel = 0.0f;
            }
        } else {
            mOffset -= mSnap;
            if (mOffset <= 0.0f) {
                mOffset = 0.0f;
                vel = 0.0f;
            }
        }
    } else {
        mOffset += vel;
    }

    mVel = vel;
}

void dPosShake_c::startShake(f32 amt) {
    mVel += amt;
}
