#pragma once
#include <game/bases/d_scene.hpp>

/**
 * @brief State block for the "crsin" (course-insert?) display overlay.
 * @unofficial Only the one member this TU needs is declared; the class has no
 * header of its own yet.
 */
class dScCrsin_c {
public:
    /// [.sbss:0x8042A490] Written by dScRestartCrsin_c before a scene switch.
    /// @unofficial
    static bool m_isDispOff;
};

/**
 * @brief Scene helper that restarts the game from a crash / file-select path.
 * @details Both entry points are plain static functions: they fill the shared
 * start-info block, arm the fader, and schedule this scene itself
 * (fProfile::RESTART_CRSIN) as the next scene.
 * @ingroup bases
 * @unofficial
 */
class dScRestartCrsin_c : public dScene_c {
public:
    /// Start-parameter block filled before switching scenes.
    /// Layout recovered from the stores in startTitle/reStartPeachCastle;
    /// bytes 0x00-0x04 are never touched there. @unofficial
    struct StartGameInfo {
        u8 pad00[5]; ///< 0x00-0x04, untouched by this TU. @unofficial
        u8 _05;      ///< 0x05, written 0xFF. @unofficial
        u8 _06;      ///< 0x06, written 0. @unofficial
        u8 _07;      ///< 0x07, restart flag. @unofficial
        u32 _08;     ///< 0x08, scene parameter word. @unofficial
        u8 _0C;      ///< 0x0C. @unofficial
        u8 _0D;      ///< 0x0D. @unofficial
        u8 _0E;      ///< 0x0E, copied from 0x0C by startTitle. @unofficial
        u8 _0F;      ///< 0x0F, copied from 0x0D by startTitle. @unofficial
    };

    static void startTitle(unsigned char, bool);
    static void reStartPeachCastle();

    /// [.bss:0x80374060] @unofficial
    static StartGameInfo m_startGameInfo;
};
