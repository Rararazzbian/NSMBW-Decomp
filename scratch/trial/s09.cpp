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
    f32 x = getOffset();
    f32 k = mK;
    f32 vel = mVel;
    f32 t = x * k;
    vel -= t;

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
}

void dPosShake_c::startShake(f32 amt) {
    mVel += amt;
}
