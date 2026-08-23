#include <game/bases/d_rot_shake.hpp>

// Argument -> member mapping, proven by init__11dRotShake_cFssssssss:
//   r4 (p1) -> 0xe   r5 (p2) -> 0x4   r6 (p3) -> 0x6   r7 (p4) -> 0x8
//   r8 (p5) -> 0x2   r9 (p6) -> 0x0   r10 (p7) -> 0xa  stack (p8) -> 0xc
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

    int landed = 0;

    if (v <= -m00a || v >= m00a) {
        v += x;
    } else if (v < 0) {
        v += m00a;
        if (v > -m00c) {
            v = 0;
            x = 0;
            landed = 1;
        }
    } else {
        v -= m00a;
        if (v < m00c) {
            v = 0;
            x = 0;
            landed = 1;
        }
    }

    m000 = x;
    m002 = v;
    m010 = landed;
    return v;
}
