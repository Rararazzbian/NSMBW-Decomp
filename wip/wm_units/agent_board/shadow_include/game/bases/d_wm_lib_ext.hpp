#pragma once
#include <game/bases/d_wm_lib.hpp>

namespace dWmLib {
    /// @unofficial Not in the real header yet. Takes the 4-bit course-index field of mParam,
    /// returns whether that "special world" course slot is open. Mangled
    /// `isSpecialWorldCourseOpen__6dWmLibFi` -- free function in namespace dWmLib, one int param.
    bool isSpecialWorldCourseOpen(int courseIdx);
}
