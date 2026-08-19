#pragma once

#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_player_model_manager.hpp>

/// @brief The actor for the Toad (kinopio) NPC found on the World Map.
/// @unofficial Class name and most member names are guesses -- see
/// wip/wm_units/agent_kinopio/MAPPING.md. sizeof == 0x1bc matches the
/// target's `li r3,0x1bc; bl __nw__7fBase_cFUl` exactly. mHeapAllocator
/// (0x13c) and mModel (0x158), destructed directly in the target's dtor,
/// are dWmDemoActor_c's OWN inherited members (confirmed by a
/// STATIC_ASSERT/Probe compile against the real header already in
/// include/) -- NOT new fields on this class.
/// @ingroup bases
class daWmKinopio_c : public dWmDemoActor_c {
public:
    /// @unofficial startJump()'s 2nd parameter shape, read off fn_2_16D100's
    /// field offsets (+0x4 float, +0x8 s16, +0xc/+0x10 floats). Caller
    /// (fn_2_16C810, not yet authored) not yet examined to confirm this is
    /// really a distinct type vs. a slice of some larger table row.
    struct JumpParam_t {
        u32 m_00;
        float mSpeed;
        s16 mFrames;
        s16 pad;
        float mStartScaleSrc;
        float mTargetScaleSrc;
    };

    daWmKinopio_c();
    virtual ~daWmKinopio_c();

    virtual int doDelete();          ///< @unofficial fn_2_16C3E0, trivial `return 1;`
    virtual int draw();              ///< @unofficial fn_2_16C3A0
    virtual int execute();           ///< @unofficial fn_2_16C2F0
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame); ///< @unofficial fn_2_16C5E0

    virtual int create();  ///< @unofficial fn_2_16C270 -- confirmed by the target's mid-function
                            ///< `li r3,SUCCEEDED` (scheduled early, held across later stores)
    void createModel();     ///< @unofficial fn_2_16C3F0
    void calcModel();        ///< @unofficial fn_2_16C490
    void resetPosition();    ///< @unofficial fn_2_16C530
    void resetStep();        ///< @unofficial fn_2_16C5C0, sets m_190 = 0
    void unusedStub();       ///< @unofficial fn_2_16C5D0, empty (ptmf table target)
    void checkAnmLoop();      ///< @unofficial fn_2_16D050
    void startJump(const char *nodeName, const JumpParam_t *param); ///< @unofficial fn_2_16D100
    bool checkSpawnGate();       ///< @unofficial fn_2_16D190, IsSingleEntry() && !fn_800FCB30(0)

    /// @unofficial fn_2_16C810, the huge (0x834-byte) per-frame cutscene-
    /// 0x70 state machine, dispatched via a 20-entry jump table on m_1a8.
    /// NOT attempted this round -- see MAPPING.md.
    void stepCutscene70();

    u32 m_184;               ///< @0x184, unobserved -- role unconfirmed
    dPyMdlMng_c *mpMdlMng;    ///< @0x188
    u32 m_18c;                ///< @0x18c, unobserved -- role unconfirmed
    int m_190;                 ///< @0x190, step/ptmf-table index; reset to 0 by resetStep()
    u32 m_194;                  ///< @0x194, unobserved -- role unconfirmed
    int m_198;                   ///< @0x198, cutscene sub-state (set to 0xe/0xf in stepCutscene70)
    mVec3_c m_19c;                 ///< @0x19c, target position (set by processCutsceneCommand)
    int m_1a8;                      ///< @0x1a8, main step-table index (0-19), the stepCutscene70 dispatch key
    int m_1ac;                       ///< @0x1ac, loop counter, wraps at 1000
    int m_1b0;                        ///< @0x1b0, an effect ID passed to dWmEffectManager_c::endEffect() (case 11)
    u8 m_1b4;                          ///< @0x1b4, byte flag
    u8 pad_1b5[3];                      ///< @unofficial alignment
    dWmActor_c *m_1b8;                   ///< @0x1b8, result of dWmActor_c::construct(0x28f, ...) -- a spawned child actor
};
