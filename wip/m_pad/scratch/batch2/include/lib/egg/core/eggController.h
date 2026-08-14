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
    int getDpdNumMarks() const;

    void sceneReset();

    int mNum;
};

// TEST HYPOTHESIS (proven byte-exact by batch1 against endPad, and reused
// here for beginPad's identical this+0x10 / slot pattern): CoreControllerMgr's
// own vtable sits at offset 0x10 because a non-polymorphic base of that size
// precedes it. The true shape of those 0x10 bytes is NOT known -- only the
// size and non-polymorphic-ness are load-bearing. @unofficial
class CoreControllerMgrTestBase {
    u8 mPad0x10[0x10];
};

class CoreControllerMgr : public CoreControllerMgrTestBase {
public:
    static void createInstance();
    static CoreControllerMgr *sInstance;
    static u32 sWPADWorkSize;

    CoreController *getNthController(int idx);

    virtual void beginFrame(); // slot 0, offset+0x8 -- called by beginPad
    virtual void endFrame();   // slot 1, offset+0xc -- called by endPad
};

} // namespace EGG
