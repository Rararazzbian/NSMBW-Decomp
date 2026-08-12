#pragma once
#include <types.h>
#include <game/mLib/m_vec.hpp>

namespace dPad {

void beginPad_BR();
void endPad_BR();

/// @brief Extended controller state shared by the whole game.
/// @unofficial
class ex_c {
public:
    ex_c();
    ~ex_c();

    mVec2_c mRawPointerPos; ///< The raw pointer position. @unofficial
    mVec2_c mPointerPos; ///< The smoothed pointer position, in screen space. @unofficial
    u8 mPad[0x70]; ///< @todo Reconstruct the rest of the class.

    static ex_c m_ex; ///< The shared instance.
    static ex_c *m_currentEx; ///< The currently active instance.
};

} // namespace dPad
