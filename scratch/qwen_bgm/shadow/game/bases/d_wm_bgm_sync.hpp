#pragma once

#include <types.h>
#include <game/mLib/m_angle.hpp>

class dWmBgmSync_c {
public:
    dWmBgmSync_c() :
        m_04(0), m_08(0),
        m_0c(false), m_0d(false), m_0e(false),
        m_10(0.0f), m_14(0.0f),
        m_18(nullptr) {}

    virtual ~dWmBgmSync_c();
    virtual bool execute();

    float getAnmRate(float frameCount);

private:
    float fn_80102F10();

public:
    int m_04;
    int m_08;
    bool m_0c;
    bool m_0d;
    bool m_0e;
    f32 m_10;
    f32 m_14;
    const s16 *m_18;
};
