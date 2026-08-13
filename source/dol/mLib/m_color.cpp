#include <game/mLib/m_color.hpp>
#include <revolution/GX.h>

mColor mColor::lerp(const GXColor &a, const GXColor &b, float t) {
    return nw4r::ut::Color(
        (int)(a.r * (1.0f - t) + b.r * t),
        (int)(a.g * (1.0f - t) + b.g * t),
        (int)(a.b * (1.0f - t) + b.b * t),
        (int)(a.a * (1.0f - t) + b.a * t)
    );
}
