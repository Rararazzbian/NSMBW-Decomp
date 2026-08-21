#pragma once

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <nw4r/g3d.h>

/// @brief The actor for the manta ray found on the World Map.
/// @unofficial Class name and most member names are guesses -- see
/// wip/wm_units/agent_manta/MAPPING.md.
/// @details Internal model/archive name measured from the retail .data as
/// "togezo" / "g3d/togezo.brres" -- an internal dev-name mismatch with the
/// WM_MANTA profile name, not a bounds error (see MAPPING.md).
/// @ingroup bases
class daWmManta_c : public dWmDemoActor_c {
public:
    daWmManta_c();
    ~daWmManta_c();

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    void createModel();
    void calcModel(m3d::mdl_c *mdl);   ///< @unofficial fn_2_171200
    void startStep();                  ///< @unofficial fn_2_1712C0
    void resetStep();                  ///< @unofficial fn_2_1712E0
    void unusedStub();                 ///< @unofficial fn_2_1712F0 (ptmf table target, empty)
    int countModelVariants();          ///< @unofficial fn_2_171320
    u8 getWorldNo();                   ///< @unofficial fn_2_171400

    int m_184;                  ///< @unofficial, untouched by ctor (zero-init)
    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile; ///< measured -- explicitly zeroed in the ctor, unlike dance_pakkun's equivalent
    m3d::mdl_c mModel;
    m3d::anmChr_c mChrAnim[1];   ///< array of 1, per __construct_array in the ctor
    u32 m_220;                   ///< @unofficial, untouched by ctor
    int m_224;                   ///< @unofficial "current step index", reset by resetStep()
};
