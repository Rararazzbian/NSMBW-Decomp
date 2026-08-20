#pragma once

#include <types.h>

/// @brief Manages the in-stage pause menu and the course-out confirmation flow.
/// @details Reconstructed from the symbol map and disassembly; the class lives
/// at `0x800D0A90`-`0x800D15F0`, between `d_pad.cpp` and `d_pc.cpp`.
/// `__vt__14PauseManager_c` is `0xC`, i.e. one virtual slot, so the destructor
/// is the only virtual function.
///
/// The method list below is recovered from real mangled symbols, so the NAMES
/// and PARAMETERS are evidence. The instance layout is NOT: only `mFlags` at
/// `0x18` is pinned, by `setPauseEnable` doing `lbz`/`ori 0x2`/`stb` on
/// `0x18(r3)`. Everything before it is honest padding rather than invented
/// fields, and the total size is unknown because the class is heap-allocated
/// and nothing embeds it by value. @unofficial
///
/// SHADOW COPY (agent_gun_battery): extends `pad_1d` to reach `0x1d`, a field
/// `daMiniGameGunBatteryMgrObj_c::finalizeState_ShowRule` (`fn_2_F8F20`)
/// writes `1` to (`stb r0, 0x1d(r3)` immediately after loading
/// `m_instance__14PauseManager_c`). Nothing in this project embeds
/// `PauseManager_c` by value (confirmed by the real header's own comment
/// above), so widening the gap between `mFlags` and this new field is safe --
/// it only affects heap-pointer field access, never a `sizeof`.
class PauseManager_c {
public:
    PauseManager_c();
    virtual ~PauseManager_c();

    void CourseHoinitialize();
    void initialize();
    void execute();
    void setPauseEnable(bool enable);
    bool isDisable();
    void ProcMainInit();
    void ProcMainPauseOn();
    void SelectSoundSet(int sound);
    void KeyChack();
    void ProcMainPause();
    void ProcMainPauseOffInit();
    void ProcMainPauseOff();
    void CourseOutConfirmation();
    void ConfirmationSelectDecisionWait();
    void OtasukeDisp();
    void PauseSetUp(int playerNo);
    void setPause();
    void onDispOtasukeWindow();
    bool isOtasukePause();

    u8 pad4[0x14];  ///< [0x04] Not reconstructed. @unofficial
    /// @brief [0x18] Bit 1 (0x02) is "pause disabled" -- setPauseEnable(false)
    /// ors it in, setPauseEnable(true) ands it out. @unofficial
    u8 mFlags;

    u8 pad19[0x4]; ///< @unofficial SHADOW ADDITION, not reconstructed.
    /// @brief [0x1d] @unofficial SHADOW ADDITION. Set to 1 by
    /// `daMiniGameGunBatteryMgrObj_c::finalizeState_ShowRule`. Name/meaning
    /// unknown -- PLACEHOLDER.
    u8 m_1d;

    static PauseManager_c *m_instance;  ///< [.sbss:0x8042A2B8]
    static void *m_OtasukeInfo_p;       ///< [.sbss:0x8042A2BC] @unofficial
    static u8 m_Pause;                  ///< [.sbss:0x8042A2C0] @unofficial
    static u8 m_Created;                ///< [.sbss:0x8042A2C1] @unofficial
    static u8 m_OtasukeAfter;           ///< [.sbss:0x8042A2C2] @unofficial
};
