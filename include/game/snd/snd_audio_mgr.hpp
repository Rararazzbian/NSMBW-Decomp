#pragma once
#include <types.h>
#include <lib/nw4r/snd.h>

class SndAudioMgr {
public:
    /// @note BOTH overloads exist in the retail binary --
    /// startSystemSe__11SndAudioMgrFUiUl (0x801954C0) and
    /// startSystemSe__11SndAudioMgrFUlUl (0x801954B0). Declaring both once
    /// failed to build: every existing call site passed an int-typed enum,
    /// which converts to `unsigned int` and `unsigned long` at identical
    /// rank, so MWCC rejected them all as ambiguous (error 10199). The fix
    /// was not to the header but to the ARGUMENTS -- see the (u32) casts at
    /// the three enum call sites and the `const u32 SoundEffects[]` in
    /// d_yes_no_window.cpp. All seven call sites were confirmed against the
    /// binary to target FUiUl.
    void startSystemSe(unsigned int soundID, unsigned long);
    void startSystemSe(unsigned long soundID, unsigned long);
    u32 get3DCtrlFlag(unsigned long);
    void setSoundPosition(nw4r::snd::SoundHandle *p, const nw4r::math::VEC2 &pos);

    u8 mPad1[0x100];
    nw4r::snd::SoundArchive *mpSndArc;
    u8 mPad2[0x4b8];
    nw4r::snd::SoundArchivePlayer &mArcPlayer;

public:
    static SndAudioMgr *sInstance;
};
