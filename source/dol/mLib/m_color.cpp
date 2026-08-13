#include <game/mLib/m_color.hpp>
#include <revolution/GX.h>

mColor mColor::lerp(const GXColor &a, const GXColor &b, float t) {
    mColor out;
    out.r = (u8)(a.r * (1.0f - t) + b.r * t);
    out.g = (u8)(a.g * (1.0f - t) + b.g * t);
    out.b = (u8)(a.b * (1.0f - t) + b.b * t);
    out.a = (u8)(a.a * (1.0f - t) + b.a * t);
    return out;
}
