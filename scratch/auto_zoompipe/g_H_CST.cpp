#include <game/bases/d_a_zoom_pipe_base.hpp>

void daZoomPipeBase_c::init(u32 param) {
    u32 w = mParam;

    mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;
    mSpeed[1] = (w & 0xF) * 16.0f + 16.0f;
    mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;
    mCurrent = mSpeed[0];

    fn_80045A10(param, mSpeed[1], (float) ((w >> 16) & 3), 1, 0, 0);
}

int daZoomPipeBase_c::execute() {
    if (((dActor_c *) this)->ActorScrOutCheck(0)) {
        return 1;
    }

    float cur = mCurrent;
    float step = mStep;
    float tgt = mTargetLength;

    float x;

    if (tgt <= cur) {
        x = tgt + step;
        if (x >= cur) {
            mIdx += 1;
            mIdx &= 1;
            mCurrent = mSpeed[mIdx];
            x = cur;
        }
    } else {
        x = tgt - step;
        if (x <= cur) {
            mIdx += 1;
            mIdx &= 1;
            mCurrent = mSpeed[mIdx];
            x = cur;
        }
    }

    calcDownLength(x);
    return daObjPipeBase_c::execute();
}
