#pragma once
#include <types.h>

class SndSceneMgr {
public:
    void moveMissFin();

    /// @brief Starts the goal jingle. @p isCastle selects the castle variant.
    /// @unofficial Signature pinned by `startGoal__11SndSceneMgrFb`.
    void startGoal(bool);

    /// @unofficial Signature pinned by `onPowerImpact__11SndSceneMgrFv`
    /// (0x8019C620). Undeclared until d_a_player_manager.cpp needed it.
    void onPowerImpact();
    void FUN_8019d5b0(u8); ///< @unofficial
    void fn_8019be60(int); ///< @unofficial
    void fn_8019bd90(int); ///< @unofficial

    u8 mPad1[0x10];
    int m_10;
    int m_14;

    static SndSceneMgr *sInstance;
};
