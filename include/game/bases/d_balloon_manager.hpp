#pragma once
#include <types.h>
#include <game/framework/f_base_id.hpp>

/// @brief Tracks the "?" balloon that is currently carrying a player. @unofficial
class dBalloonMng_c {
public:
    void setItemId(fBaseID_e id);

    u8 mPad0[0x18];
    int m_18; ///< Cleared by daEnHatenaBalloon_c::createItem. @unofficial

    static dBalloonMng_c *m_instance;
};
