#include <game/bases/d_a_zoom_pipe_base.hpp>

void daZoomPipeBase_c::init(u32 param) {
    u32 w = mParam;

    mSpeed[1] = (w & 0xF) * 16.0f + 16.0f;

    mSpeed[0] = ((w >> 4) & 0xF) * 16.0f + 16.0f;

    mStep = ((w >> 8) & 0xF) * 0.5f + 0.5f;

    mCurrent = mSpeed[0];

    fn_80045A10(param, mSpeed[1], (float) ((w >> 16) & 3), 1, 0, 0);
}
