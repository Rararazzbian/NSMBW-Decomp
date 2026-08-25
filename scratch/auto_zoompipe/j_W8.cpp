#include <game/bases/d_a_zoom_pipe_base.hpp>

void daZoomPipeBase_c::init(u32 param) {
    u32 w = mParam;

    mSpeed[1] = (w & 0xF) * 16.0f + 16.0f;

    mCurrent = mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;

mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;

    fn_80045A10(param, mSpeed[1], (float) ((w >> 16) & 3), 1, 0, 0);
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
