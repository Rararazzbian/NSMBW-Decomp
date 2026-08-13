#pragma once
#include <types.h>
#include <lib/nw4r/snd.h>

class SndAudioMgr {
public:
    /// @note The map has TWO overloads: startSystemSe__11SndAudioMgrFUiUl
    /// (0x801954C0), declared here, and startSystemSe__11SndAudioMgrFUlUl
    /// (0x801954B0), which is NOT declared. Declaring both makes all seven
    /// existing call sites ambiguous -- they pass enum/int-typed constants
    /// that convert equally well to `unsigned int` and `unsigned long`, and
    /// MWCC rejects them (error 10199). Adding the second overload therefore
    /// requires first establishing each existing call site's true argument
    /// type, which is its own piece of work. Tried and reverted; recorded so
    /// the next person does not repeat it. @unofficial
    void startSystemSe(unsigned int soundID, unsigned long);
    u32 get3DCtrlFlag(unsigned long);
    void setSoundPosition(nw4r::snd::SoundHandle *p, const nw4r::math::VEC2 &pos);

    u8 mPad1[0x100];
    nw4r::snd::SoundArchive *mpSndArc;
    u8 mPad2[0x4b8];
    nw4r::snd::SoundArchivePlayer &mArcPlayer;

public:
    static SndAudioMgr *sInstance;
};
