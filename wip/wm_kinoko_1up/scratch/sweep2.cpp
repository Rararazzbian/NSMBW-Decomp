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
    virtual void vf7C();
    virtual void vf80();
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

    void setResNames(const char *const *anim, const char *model) { mAnimResNames = anim; mModelResName = model; } ///< @unofficial
    const char *const *mAnimResNames; ///< @unofficial moved here from daWmKinoko1up_c for the inline-setter experiment
    const char *mModelResName; ///< @unofficial
};

/// @brief The actor for the 1-Up Mushroom Kinoko-house marker on the World Map.
/// @unofficial class name; every symbol in this unit is anonymous (fn_2_*) in
/// the map, so nothing pins the real name. daWmKinoko1up_c mirrors the sibling
/// naming convention (daWmKinokoBase_c/daWmKinokoRed_c/daWmKinokoStar_c).
class daWmKinoko1up_c : public daWmKinokoBase_c {
public:
    daWmKinoko1up_c();
    virtual ~daWmKinoko1up_c();

    virtual void vf7C();
    virtual void vf80();
    virtual void vf84();
    virtual const char *getModelName();

    u32 mUnk284; ///< @unofficial never written by this TU's own functions
    u32 mFlag; ///< @unofficial zeroed by the constructor
};

ACTOR_PROFILE(WM_KINOKO_1UP, daWmKinoko1up_c, 0);

daWmKinoko1up_c::daWmKinoko1up_c() {
    mFlag = 0;
}

daWmKinoko1up_c::~daWmKinoko1up_c() {}

void daWmKinoko1up_c::vf7C() {}
void daWmKinoko1up_c::vf80() {}

void daWmKinoko1up_c::vf84() {
    static const char *const smc_animResNames[ANIM_COUNT] = {
        "wm_1up_kinoko_appear",
        "wm_1up_kinoko_wait",
    };
    static const char *const smc_modelResName = "wm_1up_kinoko";
    setResNames(smc_animResNames, smc_modelResName);
}

const char *daWmKinoko1up_c::getModelName() {
    return "wm_1up_kinoko"; ///< @unofficial placeholder text; real bytes unverified
}
