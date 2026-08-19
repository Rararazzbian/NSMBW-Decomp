#pragma once

#include <game/bases/d_wm_enemy.hpp>

/// @unofficial SHADOW COPY, unmodified copy of the real include/game/bases/d_a_wm_player.hpp.
/// `fn_2_19B170` (a REL-local, non-static, non-virtual daWmPlayer_c member with no exported
/// name, called four times from daWmAntlionMng_c::processCutsceneCommand, fn_2_15B830, as
/// `ms_instance->fn_2_19B170()`) is NOT declared as a member here -- a member-call spelling
/// compiles but does not link into daWmPlayer_c's own not-yet-landed TU. It is instead called
/// via the `R_2_1_19B170` free-function convention, declared directly in
/// d_a_wm_antlion_mng.cpp (see that file's own top-of-file comment).
class daWmPlayer_c : public dWmDemoActor_c {
public:
    void setEnemyDieByStar(dWmEnemy_c *);

    static bool checkWalkPlayers();
    static bool isPlayerStarMode();

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
