#pragma once
#include <game/bases/d_actor.hpp>
#include <game/bases/d_base.hpp>
#include <game/sLib/s_State.hpp>

/// @unofficial WIP draft, not landed. daPeachCastleSequenceMgrObj_c is the actual worker: a
/// single-state (`Wait`) state-machine actor (`dBase_c`-derived, NOT `dActor_c`-derived --
/// its ctor calls `__ct__7dBase_cFv` directly, confirmed by direct disassembly of
/// fn_2_120630) that watches for the Peach's Castle end-of-world cutscene to finish, then
/// (after a short countdown) starts the world-map "control demo" and kicks off the menu
/// model animation. daPeachCastleSequenceMgr_c is a trivial dActor_c-derived manager that
/// exists only to spawn the OBJ as its child and hold a static pointer to it.
class daPeachCastleSequenceMgrObj_c : public dBase_c {
public:
    daPeachCastleSequenceMgrObj_c();
    ~daPeachCastleSequenceMgrObj_c() {}

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

private:
    STATE_FUNC_DECLARE(daPeachCastleSequenceMgrObj_c, Wait);

    /// @unofficial +0x70..+0xac. sFStateMgr_c<T,Method> itself is 0x3c bytes (own vtable +
    /// mCheck + mFactory + mMethod), landing exactly on the +0xac field boundary confirmed
    /// below -- verified by matching MEMBER OFFSETS against the target's ctor stores, not
    /// merely by getting the byte count to add up.
    sFStateMgr_c<daPeachCastleSequenceMgrObj_c, sStateMethodUsr_FI_c> mStateMgr;

    /// @unofficial fn_2_120890. Starts/stops the world-map "control demo" via the
    /// `daPyDemoMng_c` singleton. `stop=true` -> endControlDemoAll(0); `stop=false` ->
    /// startControlDemoAll(). Both calls are tail calls in the target (both branches are the
    /// final statement of a `void` function), matching MWCC's tail-branch optimisation.
    void controlDemo(bool stop);

    /// @unofficial fn_2_1208C0. Called once the wait timer trips. Guarded on
    /// `dScene_c::m_nextScene == fProfile::PROFILE_COUNT` (i.e. no scene transition already
    /// pending). Sets mTriggered, stops the control demo, and marks the demo manager's m_58/
    /// m_94 flags.
    void demoStart();

    /// @unofficial fn_2_120920. Mirror of demoStart(): clears mTriggered, restarts the
    /// control demo, and clears the demo manager's m_58/m_94 flags.
    void demoEnd();

    /// @unofficial +0xac. Phase within the Wait state: 0 = idle, 1 = counting down.
    int mPhase;
    /// @unofficial +0xb0. Countdown, in frames, before demoStart()'s effects are unwound and
    /// ModelPlayMenuStart() fires.
    int mCountdown;
    /// @unofficial +0xb4. Set by demoStart()/demoEnd(); read by executeState_Wait() to decide
    /// whether to enter the countdown.
    bool mTriggered;

public:
    /// @unofficial [.bss] Set by daPeachCastleSequenceMgr_c's constructor right after
    /// `createChild`, cleared by its doDelete(). A true singleton pointer (`.bss`, not a
    /// per-instance field) -- matches the BSS_SINGLETONS.md convention.
    static daPeachCastleSequenceMgrObj_c *m_instance;
};

/// @unofficial WIP draft, not landed. See daPeachCastleSequenceMgrObj_c above.
class daPeachCastleSequenceMgr_c : public dActor_c {
public:
    daPeachCastleSequenceMgr_c();
    ~daPeachCastleSequenceMgr_c() {}

    virtual int create();
    virtual int execute();
    virtual int doDelete();
};
