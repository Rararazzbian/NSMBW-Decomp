#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_chr_blend.hpp>
#include <game/mLib/m_3d/mdl.hpp>

/// @unofficial "cobKinokoStar" -- verified present at 0x45B78 in
/// bin/dtkspl/d_basesNP/obj/auto_04_00044A68_data.txt, at the very front of
/// this unit's own .data (before dWmLib::sc_ForceList's own F7C0/W7C0 pair),
/// exactly mirroring d_a_wm_kinoko_red.cpp's smc_poolCobKinokoRedEarly_red
/// idiom one unit later. Same mechanism, same reason: the REAL
/// smc_modelResName_star pointer (below) has to sit LATE in .data to match
/// the target, but the string literal it points to has to be POOLED early,
/// and a single declaration cannot do both -- so the early half is forced
/// here as a deliberately unreferenced pooling site.
static const char *smc_poolCobKinokoStarEarly_star = "cobKinokoStar";

#include <game/bases/d_wm_lib.hpp>

/// @unofficial Local, MINIMAL restatement of the shared base class -- same
/// approach and same caveat as d_a_wm_kinoko_red.cpp's copy: only the CLASS
/// SHAPE (member layout + virtual slot order) is needed here, because this
/// TU never calls an inherited method non-virtually and never instantiates a
/// bare daWmKinokoBase_c. See wip/wm_units/agent_kinoko_base/ for the
/// authoritative version with full bodies.
class daWmKinokoBase_c : public dWmObjActor_c {
public:
    enum ANIM_e {
        ANIM_0,
        ANIM_1,
        ANIM_COUNT
    };

    daWmKinokoBase_c();
    virtual ~daWmKinokoBase_c();

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);
    virtual void vf80();
    virtual void vf7C();
    virtual void vf84();
    virtual const char *getModelName();

    void createModel();
    void calcModel();
    void mode_exec();

    dHeapAllocator_c mAllocator;          ///< @unofficial 0x188
    nw4r::g3d::ResFile mResFile;          ///< @unofficial 0x1a4
    m3d::mdl_c mModel;                    ///< @unofficial 0x1a8
    m3d::anmChr_c mChrAnim[ANIM_COUNT];   ///< @unofficial 0x1e8
    m3d::anmChrBlend_c mChrBlend;         ///< @unofficial 0x258
    nw4r::g3d::ResFile mAnimResFile;      ///< @unofficial 0x280
    int mCutsceneTimer;                   ///< @unofficial 0x284
    const char *const *mAnimResNames;     ///< @unofficial 0x288. Set by the LEAF's vf84().
    const char *mModelResName;            ///< @unofficial 0x28c. Set by the LEAF's vf84().
};
// sizeof(daWmKinokoBase_c) == 0x290.

/// @unofficial UNRESOLVED, same finding as d_a_wm_kinoko_red.cpp's identical
/// note, re-measured here rather than assumed: the target's own
/// dWmLib::sc_ForceList float pool for THIS unit (lbl_2_rodata_8B00, read
/// directly out of bin/dtkspl/d_basesNP/obj/auto_03_00008880_rodata.o) is
/// 0x10 bytes -- (2160.0f, -30.0f, -478.0f, 0.0f) -- byte-identical to red's
/// own lbl_2_rodata_8AF0, including the trailing all-zero word. This unit's
/// own __sinit (fn_2_16C0A0 in the target, matching red's __sinit shape
/// exactly) does exactly three `lfs` at relative pool offsets 0/4/8 and never
/// reads offset 0xC, so there is no candidate "missing use" of a fourth float
/// anywhere in this TU either. Recording it as the same open, unexplained
/// 4-byte `.rodata` shortfall as red's, not re-deriving a new theory.

/// @brief The actor for the Star (permanent) Mushroom-house marker (course
/// start point) on the World Map.
/// @unofficial class name; every symbol in this unit is anonymous (fn_2_*) in
/// the map. daWmKinokoStar_c mirrors the sibling naming convention
/// (daWmKinokoBase_c/daWmKinoko1up_c/daWmKinokoRed_c).
class daWmKinokoStar_c : public daWmKinokoBase_c {
public:
    daWmKinokoStar_c();
    virtual ~daWmKinokoStar_c();

    virtual void vf7C();
    virtual void vf80();
    virtual void vf84();
    /// @unofficial INLINE ON PURPOSE, for the same reason as
    /// d_a_wm_kinoko_red.cpp's getModelName(): a weak (inline) function's
    /// anonymous literal pool is deferred to the vtable-construction pass,
    /// which is what puts this unit's second "cobKinokoStar" copy at
    /// lbl_2_data_45C70, immediately AFTER __vt__16daWmKinokoStar_c, exactly
    /// as the target has it.
    virtual const char *getModelName() { return "cobKinokoStar"; }

    /// @unofficial Measured, not guessed: the ctor stores a literal 0 to
    /// offset 0x290 -- the first byte past daWmKinokoBase_c's own 0x290
    /// bytes -- via `li r0,0x0 / stw r0,0x290(r31)`, which is what makes
    /// this class's own `__nw__7fBase_cFUl` allocation 0x294 rather than
    /// red's 0x290. No function in this 9-function unit (vf7C/vf80/vf84/
    /// getModelName) reads it back, so its purpose is unresolved from this
    /// TU alone; recorded as a plain zero-initialized field rather than
    /// invented as anything more specific.
    int mUnk290;
};

ACTOR_PROFILE(WM_KINOKO_STAR, daWmKinokoStar_c, 0);

daWmKinokoStar_c::daWmKinokoStar_c() : mUnk290(0) {}

daWmKinokoStar_c::~daWmKinokoStar_c() {}

void daWmKinokoStar_c::vf7C() {}
void daWmKinokoStar_c::vf80() {}

/// @unofficial "cobKinokoStar", a SEPARATE instance from the one above --
/// verified present at 0x45BC8, immediately after this unit's copy of
/// "cobKinokoAppear" -- read directly out of
/// bin/dtkspl/d_basesNP/obj/auto_04_00044A68_data.txt. Same idiom as red's
/// smc_animResNames_red.

static const char *smc_animResNames_star[daWmKinokoStar_c::ANIM_COUNT] = {
    "cobKinokoAppear",
    "cobKinokoAppear",
};

/// @unofficial The REAL `mModelResName` pointer variable, declared here
/// (after smc_animResNames_star) so it emits into .data in the position the
/// target actually has it -- immediately before the vtable, at unit offset
/// 0x68 -- mirroring smc_modelResName_red exactly.
static const char *smc_modelResName_star = "cobKinokoStar";

void daWmKinokoStar_c::vf84() {
    mAnimResNames = smc_animResNames_star;
    mModelResName = smc_modelResName_star;
}
