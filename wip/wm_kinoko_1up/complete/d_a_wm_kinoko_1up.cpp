#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_chr_blend.hpp>
#include <game/mLib/m_3d/mdl.hpp>

/// @unofficial Provisional reconstruction of the shared base class for the
/// world-map Kinoko-house markers (1-Up / Red / Star). Not landed yet; the
/// pre-flight in peer_archive/GEMINI_round11.md claimed sizeof == 0x2B0, but
/// that is contradicted by the classInit operator-new literal for both this
/// class (0x290 in round11's own captured disassembly -- actually 0x284,
/// see BATCH.md) and the derived leaf (0x294). sizeof verified by compiled
/// STATIC_CHECK against a component-by-component layout, matching the same
/// dHeapAllocator_c/mdl_c/anmChr_c[]/anmChrBlend_c pattern already landed in
/// daWmPeachCastle_c (include/game/bases/d_a_wm_peach_castle.hpp).
class daWmKinokoBase_c : public dWmObjActor_c {
public:
    enum ANIM_e {
        ANIM_0, ///< @unofficial name
        ANIM_1, ///< @unofficial name
        ANIM_COUNT
    };

    daWmKinokoBase_c();
    virtual ~daWmKinokoBase_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);
    // Declaration order sets the VTABLE slot order; definition order in this
    // file sets the .text addresses. The two are deliberately opposite here:
    // the target vtable lists 0x16B1E0 before 0x16B1D0, while 0x16B1D0 is the
    // function defined first. Both bodies are a bare `blr`, so .text cannot
    // show this -- only the vtable relocations can.
    virtual void vf80();
    virtual void vf7C();
    virtual void vf84();
    virtual const char *getModelName();

    void createModel();
    void calcModel();
    void mode_exec();

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;
    m3d::mdl_c mModel;
    m3d::anmChr_c mChrAnim[ANIM_COUNT];
    m3d::anmChrBlend_c mChrBlend;
    u32 mUnk280; ///< @unofficial zeroed by the base constructor; purpose unknown
};

/// @brief The actor for the 1-Up Mushroom Kinoko-house marker on the World Map.
/// @unofficial class name; every symbol in this unit is anonymous (fn_2_*) in
/// the map, so nothing pins the real name. daWmKinoko1up_c mirrors the sibling
/// naming convention (daWmKinokoBase_c/daWmKinokoRed_c/daWmKinokoStar_c).
class daWmKinoko1up_c : public daWmKinokoBase_c {
public:
    daWmKinoko1up_c();
    virtual ~daWmKinoko1up_c();

    // Declaration order sets the VTABLE slot order; definition order in this
    // file sets the .text addresses. The two are deliberately opposite here:
    // the target vtable lists 0x16B1E0 before 0x16B1D0, while 0x16B1D0 is the
    // function defined first. Both bodies are a bare `blr`, so .text cannot
    // show this -- only the vtable relocations can.
    virtual void vf80();
    virtual void vf7C();
    virtual void vf84();
    virtual const char *getModelName();

    u32 mUnk284; ///< @unofficial never written by this TU's own functions
    const char *const *mAnimResNames; ///< @unofficial set by vf84(); consumed by daWmKinokoBase_c::createModel()
    const char *mModelResName; ///< @unofficial set by vf84(); consumed by daWmKinokoBase_c::createModel()
    u32 mFlag; ///< @unofficial zeroed by the constructor
};

ACTOR_PROFILE(WM_KINOKO_1UP, daWmKinoko1up_c, 0);

daWmKinoko1up_c::daWmKinoko1up_c() {
    mFlag = 0;
}

daWmKinoko1up_c::~daWmKinoko1up_c() {}

void daWmKinoko1up_c::vf7C() {}
void daWmKinoko1up_c::vf80() {}

/// @unofficial "cobKinoko1up" -- verified present at 0x457A8 in
/// bin/dtkspl/d_basesNP/obj/auto_04_00044A68_data.txt, immediately BEFORE
/// this unit's own .data (which begins at 0x457B8). Not owned by this TU;
/// referenced here by the address-based DTK label so the relocation target
/// matches until whichever sibling TU actually owns it is landed and gets a
/// real name in syms.txt.
extern "C" const char lbl_2_data_457A8[];

/// @unofficial Also "cobKinoko1up", a SEPARATE instance from the one above --
/// verified present at 0x458A0, immediately AFTER this unit's own vtable
/// (which ends the unit's claimed .data span at 0x458A0). getModelName()
/// returns this by address-of, not by loading a pointer variable, matching
/// fn_2_16B210's `lis/addi` (no `lwz`) shape. Also not owned by this TU.
extern "C" const char lbl_2_data_458A0[];

static const char *smc_animResNames_1up[daWmKinoko1up_c::ANIM_COUNT] = {
    "cobKinokoAppear",
    "cobKinokoAppear",
};
static const char *smc_modelResName_1up = lbl_2_data_457A8;

void daWmKinoko1up_c::vf84() {
    mAnimResNames = smc_animResNames_1up;
    mModelResName = smc_modelResName_1up;
}

const char *daWmKinoko1up_c::getModelName() {
    return lbl_2_data_458A0;
}
