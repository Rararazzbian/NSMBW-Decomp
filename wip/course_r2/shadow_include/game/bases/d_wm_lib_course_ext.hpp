#pragma once
// Additional dWmLib free functions this unit's target calls, not yet declared
// in the real include/game/bases/d_wm_lib.hpp. Names/signatures are inferred
// from mangled symbols in the target disassembly (isSpecialWorld__6dWmLibFv
// etc.) -- proposed here, not claimed correct until verified byte-exact.
#include <game/bases/d_wm_lib.hpp>
#include <game/framework/f_base.hpp>

namespace dWmLib {
    bool isSpecialWorld();
    bool IsAllComplete();
    bool isKoopaShipOnCurrentWorld();
    bool isSpecialWorldCourseOpen(int course);
    fBase_c *SearchMapObjFromCsvIndex(u16 profName, int csvIndex);
}
