#pragma once

#include <game/bases/d_wm_enemy.hpp>

class daWmPlayer_c : public dWmDemoActor_c {
public:
    void setEnemyDieByStar(dWmEnemy_c *);

    static bool checkWalkPlayers();
    static bool isPlayerStarMode();
    static bool isWalkToAttackPoint(); ///< @unofficial Not in the landed header yet --
                                         ///< added here (shadow-only) for #unk_1695E0's
                                         ///< own static call, no `this` at the call site.
    void attackMapEnemy(bool); ///< @unofficial Not in the landed header yet -- added here
                                 ///< (shadow-only) for #unk_1695E0's own
                                 ///< `ms_instance->attackMapEnemy(true)` call.

    u8 mPad1[0x8];
    bool m_18c;
    u8 mPad2[0x9f];
    int m_22c;
    int m_230;
    int m_234;
    u8 mPad4[0xc4];
    bool m_2fc;

    static daWmPlayer_c *ms_instance;
};
