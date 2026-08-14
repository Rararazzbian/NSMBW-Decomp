#pragma once

#include <types.h>
#include <revolution/PAD.h>
#include <revolution/KPAD.h>
#include <revolution/WPAD.h>

namespace EGG {

class CoreStatus {
public:
    void init();
};

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

    void sceneReset();

    int mNum;
};

// @unofficial CoreControllerMgr is not decompiled beyond this batch's needs.
// The 0x10-byte gap below is a placeholder for whatever base/member data
// precedes CoreControllerMgr's own vtable pointer in the real object -- only
// its SIZE (0x10) and non-polymorphic-ness are load-bearing for reproducing
// beginPad()'s virtual call (which loads a vtable pointer from object offset
// 0x10, then calls slot 0 of that vtable). The real shape belongs to
// whoever decompiles EGG::CoreControllerMgr's own TU.
class CoreControllerMgrBase_unofficial {
    u32 mUnofficialPad0;
    u32 mUnofficialPad1;
    u32 mUnofficialPad2;
    u32 mUnofficialPad3;
};

class CoreControllerMgr : private CoreControllerMgrBase_unofficial {
public:
    static void createInstance();

    CoreController *getNthController(int idx);

    // @unofficial names/order/count guessed; only slot 0 being called from
    // beginPad() is evidenced.
    virtual void calc();
    virtual void unk1();

    static u32 sWPADWorkSize;
};

} // namespace EGG
