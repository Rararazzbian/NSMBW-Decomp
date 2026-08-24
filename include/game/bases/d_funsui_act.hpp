#pragma once
#include <types.h>
#include <game/framework/f_base_id.hpp>
#include <game/mLib/m_effect.hpp>

/// @brief A fountain/geyser position actor helper.
/// @unofficial The only retail symbol in wiimj2d.dol is ::posMove. Nothing in
/// this module constructs the class, so the member roles below are inferred
/// from posMove's accesses alone.
class dFunsuiAct_c {
public:
    void posMove();

    fBaseID_e mBaseId;              ///< 0x00 base ID passed to searchBaseByID. @unofficial
    float mTargetY;                 ///< 0x04 value handed to updateFunsuiPos / stored to mPos.x. @unofficial
    float mCurrentY;                ///< 0x08 accumulated value written to mPos.y. @unofficial
    float mSpeed;                   ///< 0x0C per-frame increment, zeroed while revolving. @unofficial
    u32 mPad10;                     ///< 0x10 never read by posMove. @unofficial
    int mRevIndex;                  ///< 0x14 selects cs_rev_speed[mRevIndex] (+3/-3). @unofficial
    int mTimer;                     ///< 0x18 countdown gating the rev-speed branch. @unofficial
    mEf::levelEffect_c mEffect;     ///< 0x1C spawns "Wm_en_quicksand" at a computed position. @unofficial
};
