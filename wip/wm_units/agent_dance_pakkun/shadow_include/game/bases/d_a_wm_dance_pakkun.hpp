#pragma once

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_chr_blend.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_wm_bgm_sync.hpp>
#include <nw4r/g3d.h>

/// @brief The actor for the dancing piranha plant found on the World Map.
/// @unofficial Class name and most member names are guesses -- see
/// wip/wm_units/agent_dance_pakkun/MAPPING.md, "Open questions".
/// @ingroup bases
class daWmDancePakkun_c : public dWmDemoActor_c {
public:
    daWmDancePakkun_c();
    ~daWmDancePakkun_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();
    // NB: does NOT override processCutsceneCommand() -- execute() calls the
    // inherited dWmDemoActor_c default through the ordinary (secondary)
    // vtable slot. See d_a_wm_dance_pakkun.cpp for the corpus evidence.

    void createModel();
    void calcModelFor(m3d::mdl_c *mdl); ///< @unofficial fn_2_161F20
    void startStep();                   ///< @unofficial fn_2_1620C0
    void resetStep();                   ///< @unofficial fn_2_162150
    void updateStepAnim();              ///< @unofficial fn_2_162160 (ping-pong anim rate)
    void unusedStub();                  ///< @unofficial fn_2_1621B0 (empty)
    void tailHelper();                  ///< @unofficial fn_2_161F10

    int m_184;   ///< @unofficial, ctor sets -1
    int m_188;   ///< @unofficial, left at operator-new's zero-init
    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;     ///< measured -- createModel() spills its local ResFile here rather than the stack
    m3d::mdl_c mModel;
    m3d::mdl_c mModel2;              ///< @unofficial name
    m3d::anmChr_c mChrAnim[1];       ///< NB: array of 1 -- ctor uses __construct_array
    m3d::anmChrBlend_c mBlend;       ///< @unofficial name
    m3d::anmTexSrt_c mTexSrt;        ///< @unofficial name
    u32 m_2b8;                       ///< @unofficial, untouched by ctor
    int m_2bc;                       ///< @unofficial, "current dance step index"
    u8 pad_2c0[0x10];                ///< @unofficial -- UNRESOLVED, see MAPPING.md Open question 4
    bool m_2d0;                      ///< @unofficial, set to true near the end of execute()
    u8 pad_2d1[0x7];                 ///< @unofficial -- UNRESOLVED
    float m_2d8;                     ///< @unofficial, compared/recomputed from mChrAnim[0].mFrameMax and mBgmSync->getAnmRate()
    dWmBgmSync_c *mBgmSync;
};
