#pragma once
#include <types.h>
// VERIFICATION-ONLY SHADOW COPY -- not part of the assembled.cpp deliverable
// and NOT written into include/. Real header is missing startMiss(), needed
// by daPyMng_c::startMissBGM (B6). B6's own scratch shadow copy already
// found and flagged this same gap. See BATCH6.md / ASSEMBLY.md.

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

    /// SHADOW-ONLY addition -- not in the real header. Called from
    /// startMissBGM (B6); no mangled-name evidence beyond the call itself
    /// (this batch never got far enough to confirm the symbol, since the
    /// whole TU wouldn't compile without it).
    void startMiss();

    u8 mPad1[0x10];
    int m_10;
    int m_14;

    static SndSceneMgr *sInstance;
};
