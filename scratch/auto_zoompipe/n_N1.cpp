#include <game/bases/d_a_zoom_pipe_base.hpp>

void daZoomPipeBase_c::init(u32 param) {
        u32 w = mParam;
        u32 a = (w >> 8) & 0xF;
        u32 b = (w >> 4) & 0xF;
        u32 c = w & 0xF;
        u32 d = (w >> 16) & 3;
    mSpeed[0] = b * 16.0f + 16.0f;
    mCurrent = mSpeed[0];
    mSpeed[1] = c * 16.0f + 16.0f;
    mStep = a * 0.5f + 0.5f;
    fn_80045A10(this, param, mSpeed[1], (float) d, 1, 0, 0);
}



int daZoomPipeBase_c::execute() {
    if (((dActor_c *) this)->ActorScrOutCheck(0)) {
        return 1;
    }

    float tgt;
    float step;
    float cur;

    cur = mCurrent;
    step = mStep;
    tgt = mTargetLength;
    float x;

    if (tgt <= cur) {
        x = tgt + step;
        if (x >= cur) {
            u8 idx = mIdx;
            idx += 1;
            idx &= 1;
            mIdx = idx;
            mCurrent = mSpeed[idx];
            x = cur;
        }
    } else {
        x = tgt - step;
        if (x <= cur) {
            u8 idx = mIdx;
            idx += 1;
            idx &= 1;
            mIdx = idx;
            mCurrent = mSpeed[idx];
            x = cur;
        }
    }

    calcDownLength(x);
    return daObjPipeBase_c::execute();
}
