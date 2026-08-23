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
    f32 x = mOffset;
    f32 vel = mVel;
    vel -= x * mK;

    dPosShake_ChaseLanded(vel, mDamp);

    if (vel >= mLimit) {
        vel = mLimit;
    } else if (vel <= -mLimit) {
        vel = -mLimit;
    }

    if (vel >= -mSnap && vel <= mSnap) {
        if (dPosShake_ChaseLanded(x, mSnap)) {
            vel = 0.0f;
        }
    } else {
        x += vel;
    }

    mOffset = x;
    mVel = vel;
}

void dPosShake_c::startShake(f32 amt) {
    mVel += amt;
}
