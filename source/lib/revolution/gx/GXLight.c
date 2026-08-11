#include <revolution/GX/GXLight.h>

#include <revolution/GX/GXInternal.h>

void GXInitLightAttn(GXLightObj *light, f32 aa, f32 ab, f32 ac, f32 ka, f32 kb,
                     f32 kc) {
    GXLightObjImpl *impl = (GXLightObjImpl *)light;

    impl->aa = aa;
    impl->ab = ab;
    impl->ac = ac;
    impl->ka = ka;
    impl->kb = kb;
    impl->kc = kc;
}

void GXInitLightAttnA(GXLightObj *light, f32 a, f32 b, f32 c) {
    GXLightObjImpl *impl = (GXLightObjImpl *)light;

    impl->aa = a;
    impl->ab = b;
    impl->ac = c;
}

void GXInitLightAttnK(GXLightObj *light, f32 a, f32 b, f32 c) {
    GXLightObjImpl *impl = (GXLightObjImpl *)light;

    impl->ka = a;
    impl->kb = b;
    impl->kc = c;
}
