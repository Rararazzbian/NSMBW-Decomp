// p01: move() returns f32 (return type invisible in mangling; sibling
// dRotShake_c::move returns its value). Return x so the value is already
// in f1 at the epilogue stores -- no extra fmr.
#include <game/bases/d_pos_shake.hpp>

void dPosShake_c::init(f32 k, f32 damp, f32 limit, f32 offset, f32 vel, f32 snap) {
    mK = k;
    mDamp = damp;
    mLimit = limit;
    mOffset = offset;
    mVel = vel;
    mSnap = snap;
}

f32 dPosShake_c::move() {
    f32 x = mOffset;
    f32 vel = mVel;
    vel -= x * mK;

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
        if (x < 0.0f) {
            x += mSnap;
            if (x >= 0.0f) {
                x = 0.0f;
                vel = 0.0f;
            }
        } else {
            x -= mSnap;
            if (x <= 0.0f) {
                x = 0.0f;
                vel = 0.0f;
            }
        }
    } else {
        x += vel;
    }

    mOffset = x;
    mVel = vel;
    return x;
}

void dPosShake_c::startShake(f32 amt) {
    mVel += amt;
}
