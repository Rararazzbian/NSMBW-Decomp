#include <game/bases/d_line_mng.hpp>
#include <cstddef>

template <int N> struct Probe;
template <> struct Probe<0x00> {};
template <> struct Probe<0x38> {};
template <> struct Probe<0x40> {};
template <> struct Probe<0x48> {};
template <> struct Probe<0x50> {};
template <> struct Probe<0x58> {};
template <> struct Probe<0x60> {};
template <> struct Probe<0x64> {};
template <> struct Probe<0x66> {};
template <> struct Probe<0x67> {};
template <> struct Probe<0x68> {};
template <> struct Probe<0x69> {};
template <> struct Probe<0x6c> {};

struct Peek : dLineMng_c {
    static void check() {
        Probe<offsetof(dLineMng_c, mDirVec)> a;
        Probe<offsetof(dLineMng_c, mSpeed)> b;
        Probe<offsetof(dLineMng_c, mPos)> c;
        Probe<offsetof(dLineMng_c, mOldPos)> d;
        Probe<offsetof(dLineMng_c, mUnitBasePos)> e;
        Probe<offsetof(dLineMng_c, mUnk58)> f;
        Probe<offsetof(dLineMng_c, mBaseSpeed)> g;
        Probe<offsetof(dLineMng_c, mAngle)> h;
        Probe<offsetof(dLineMng_c, mType)> i;
        Probe<offsetof(dLineMng_c, mUnk67)> j;
        Probe<offsetof(dLineMng_c, mReverse)> k;
        Probe<offsetof(dLineMng_c, mLineType)> l;
        Probe<offsetof(dLineMng_c, mStateMgr)> m;
    }
};
