#pragma once
// Additional dWmLib free function this unit's target calls, not yet declared
// in the real include/game/bases/d_wm_lib.hpp. Distinct overload from the
// already-declared GetModelNodePos(const m3d::bmdl_c*, int) -- this one takes
// a node NAME (const char*), matching the mangled call site
// getModelNodePos__6dWmLibFPCQ23m3d6bmdl_cPCc in fn_2_191B70.
#include <game/bases/d_wm_lib.hpp>

namespace dWmLib {
    nw4r::math::VEC3 getModelNodePos(const m3d::bmdl_c *model, const char *nodeName);
}
