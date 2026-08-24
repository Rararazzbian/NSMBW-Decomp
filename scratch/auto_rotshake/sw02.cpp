/* sw02: sw01 + m00a/m00c hoisted into named locals a/c before landed = 0. */
#include <game/bases/d_rot_shake.hpp>

void dRotShake_c::init(s16 p1, s16 p2, s16 p3, s16 p4, s16 p5, s16 p6, s16 p7, s16 p8) {
    m000 = p6;
    m002 = p5;
    m004 = p2;
    m006 = p3;
    m008 = p4;
    m00a = p7;
    m00c = p8;
    m00e = p1;
}

s16 dRotShake_c::move() {
    s16 v;
    v = m002;
    s16 x;
    x = m000 + -v / m004;

    if (x < 0) {
        x += m006;
        if (x > 0) {
            x = 0;
        }
    } else {
        x -= m006;
        if (x < 0) {
            x = 0;
        }
    }

    if (x > m008) {
        x = m008;
    } else if (x < -m008) {
        x = -m008;
    }

    s16 a = m00a;
    s16 c = m00c;
    int landed = 0;

    if (x > -a && x < a) {
        if (v < 0) {
            v += a;
            if (v > -c) {
                v = 0;
                x = 0;
                landed = 1;
            }
        } else {
            v -= a;
            if (v < c) {
                v = 0;
                x = 0;
                landed = 1;
            }
        }
    } else {
        v += x;
    }

    m000 = x;
    m002 = v;
    m010 = landed;
    return v;
}
