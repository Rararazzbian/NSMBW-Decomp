#pragma once

#include <game/bases/d_wm_enemy.hpp>

/// @unofficial SHADOW COPY, proposed addition to the real include/game/bases/d_a_wm_player.hpp.
/// Adds a placeholder for `fn_2_19B170` (`0x15c9b170`? no -- `0x19b170`, a REL-local,
/// non-static, non-virtual daWmPlayer_c member with no exported name, called four times from
/// daWmAntlionMng_c::processCutsceneCommand (fn_2_15B830) as `ms_instance->fn_2_19B170()`,
/// taking no arguments beyond `this` and returning a value tested with `cmpwi r3,0` (bool-shaped).
/// `0x19b170` is itself still undecompiled, so its REAL name/signature is unknown -- this is a
/// placeholder name only, and calling it (even once daWmPlayer_c's own TU is landed) needs the
/// `R_2_1_19B170` linking convention, not a plain call, until that TU is banked.
class daWmPlayer_c : public dWmDemoActor_c {
public:
    void setEnemyDieByStar(dWmEnemy_c *);

    static bool checkWalkPlayers();
    static bool isPlayerStarMode();
    /// @unofficial Placeholder name for fn_2_19B170; see the file-level comment above.
    bool unofficialFn_19B170();

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
