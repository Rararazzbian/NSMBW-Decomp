#pragma once
#include <types.h>

/// SHADOW COPY (agent_gun_battery): adds `fn_8019C010(int)`, following this
/// header's own existing convention for not-yet-understood member functions
/// (`fn_8019be60`, `fn_8019bd90`). Called from
/// `daMiniGameGunBatteryMgrObj_c::finalizeState_Play` as
/// `SndSceneMgr::sInstance->fn_8019C010(3)` (`bl fn_8019C010`,
/// `bin/dtk/wiimj2d_symbols.txt`: `fn_8019C010 = .text:0x8019C010; size:0x280`).
/// Pure addition, no field/layout change -- cannot disturb any landed TU.
class SndSceneMgr {
public:
    void moveMissFin();

    /// @brief Starts the goal jingle. @p isCastle selects the castle variant.
    /// @unofficial Signature pinned by `startGoal__11SndSceneMgrFb`.
    void startGoal(bool);

    /// @unofficial Signature pinned by `onPowerImpact__11SndSceneMgrFv`
    /// (0x8019C620). Undeclared until d_a_player_manager.cpp needed it.
    void onPowerImpact();

    /// @unofficial Signature pinned by `startMiss__11SndSceneMgrFv`
    /// (0x8019C470).
    void startMiss();
    void FUN_8019d5b0(u8); ///< @unofficial
    void fn_8019be60(int); ///< @unofficial
    void fn_8019bd90(int); ///< @unofficial
    void fn_8019C010(int); ///< @unofficial SHADOW ADDITION.

    u8 mPad1[0x10];
    int m_10;
    int m_14;

    static SndSceneMgr *sInstance;
};
