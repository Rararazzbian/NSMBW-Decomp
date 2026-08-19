#pragma once
#include <types.h>
#include <game/mLib/m_vec.hpp>
/// @file

/// @brief Provides common utilities.
/// @ingroup clib
namespace cLib {

    s16 targetAngleY(const mVec3_c &vec1, const mVec3_c &vec2);
    /// @brief Steps @p pos toward @p target by at most @p step. @unofficial
    /// @note Returns non-bool; callers do their own 0/1 conversion.
    int chasePos(mVec3_c *pos, const mVec3_c &target, float step);

    /// @brief Eases @p pos toward @p target. @unofficial PROPOSED (WM_NOTE round): mangled
    /// addCalcPos__4cLibFP7mVec3_cRC7mVec3_cfff. Returns a float (register allocation implies
    /// non-void; callers test fabsf() of the result against a small epsilon), likely the
    /// remaining/delta magnitude after the step.
    float addCalcPos(mVec3_c *pos, const mVec3_c &target, float speed, float accel, float max);

} // namespace cLib
