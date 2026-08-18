#pragma once

#include <types.h>

/// @brief Manages stage pause state and course-out confirmation.
/// @ingroup bases
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

    int mState;         ///< Current execution state in MainProc_tbl. @unofficial
    int mUnk08;         ///< @unofficial
    int mUnk0C;         ///< @unofficial
    int mUnk10;         ///< @unofficial
    int mUnk14;         ///< @unofficial
    u8 mFlags;          ///< Bit 0: pause active, Bit 1: pause disabled. @unofficial
    u8 mUnk19;          ///< @unofficial
    u8 mUnk1A;          ///< @unofficial
    u8 mUnk1B;          ///< @unofficial
    u8 mUnk1C;          ///< @unofficial
    u8 mUnk1D;          ///< @unofficial
    u8 mPad1E[2];       ///< Padding to 0x20 alignment. @unofficial

    static PauseManager_c *m_instance;  ///< [.sbss:0x8042A2B8] Singleton instance.
    static void *m_OtasukeInfo_p;       ///< [.sbss:0x8042A2BC] Super Guide info pointer. @unofficial
    static u8 m_Pause;                  ///< [.sbss:0x8042A2C0] Pause active flag.
    static u8 m_Created;                ///< [.sbss:0x8042A2C1] Instance created flag.
    static u8 m_OtasukeAfter;           ///< [.sbss:0x8042A2C2] After Super Guide flag.
};
