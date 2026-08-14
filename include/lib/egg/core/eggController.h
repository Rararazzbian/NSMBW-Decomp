#pragma once

#include <types.h>
#include <revolution/PAD.h>
#include <revolution/KPAD.h>

namespace EGG {

class CoreController {
public:
    virtual void setPosParam(float a, float b) { KPADSetPosParam(mNum, a, b); }
    virtual void setHoriParam(float, float);
    virtual void setDistParam(float, float);
    virtual void setAccParam(float, float);
    virtual bool down(ulong) const;
    virtual bool up(ulong) const;
    virtual bool downTrigger(ulong) const;
    virtual bool upTrigger(ulong) const;
    virtual bool downAll(ulong) const;
    virtual bool upAll(ulong) const;
    virtual void beginFrame(PADStatus *);
    virtual void endFrame();

    void startPatternRumble(const char *, int, bool);

    /// @brief Gets the number of DPD light sources currently detected.
    int getDpdNumMarks() const;

    int mNum;
};

/// @brief The unidentified base that precedes CoreControllerMgr's own vtable.
/// @unofficial Its existence and its size are both forced by codegen, but what
/// it actually is remains unknown. `mPad::endPad` reads the vtable pointer from
/// `*(mgr + 0x10)` and `mPad::beginPad` does the same, which is only possible if
/// a **non-polymorphic** base of exactly 0x10 bytes sits ahead of it — a base
/// with virtuals of its own would put the pointer at offset 0. Named for what it
/// does rather than what it is, because nothing yet identifies it. Do not treat
/// the member as a real field.
class CoreControllerMgrBase {
    u8 mPad0x00[0x10]; ///< [0x00]
};

class CoreControllerMgr : public CoreControllerMgrBase {
public:
    static void createInstance();

    /// @note Slot order is fixed by two call sites: `beginPad` calls through
    /// `+0x8` and `endPad` through `+0xc`, so `beginFrame` precedes `endFrame`.
    virtual void beginFrame();
    virtual void endFrame();

    static CoreControllerMgr *sInstance;
    static u32 sWPADWorkSize;
};

} // namespace EGG
