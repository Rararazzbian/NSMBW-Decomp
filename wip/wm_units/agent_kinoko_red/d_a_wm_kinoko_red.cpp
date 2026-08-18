#include <types.h>
#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_chr_blend.hpp>
#include <game/mLib/m_3d/mdl.hpp>

/// @unofficial "cobKinokoRed" -- verified present at 0x45A70 in
/// bin/dtkspl/d_basesNP/obj/auto_04_00044A68_data.txt, at the very front of
/// this unit's own .data (before dWmLib::sc_ForceList's own F7C0/W7C0 pair).
/// The preceding "cobKinokoRed" occurrence (embedded in the base unit's own
/// merged data object at 0x458F0-0x45938, used by
/// daWmKinokoBase_c::createModel()'s `getRes("cobKinokoRed", ...)`
/// animation-file lookup) already exhausts that TU's one legal pooled copy of
/// the literal -- MWCC pools an identical string literal once PER TRANSLATION
/// UNIT, never twice -- so this second, separately labelled occurrence cannot
/// belong to the base's TU; it is RED's own natural model-resource name.
///
/// MWCC's own .data emission order for file-scope statics tracks
/// DECLARATION order (confirmed empirically: moving this line earlier/later
/// in the file moves the resulting object earlier/later in .data), and this
/// TU's REAL `smc_modelResName_red` pointer variable (below, after
/// smc_animResNames_red) has to sit LATE to match the target -- but the
/// STRING LITERAL it points to has to be POOLED early to match the target.
/// A single declaration can't do both (the declaration point fixes both the
/// pointer's AND, on first use, the string's position together), so the
/// early half is forced here as a deliberately unreferenced pooling site,
/// exactly mirroring the base unit's own confirmed
/// smc_unusedAppearName/smc_unusedAppearName2 dead-pointer idiom.
static const char *smc_poolCobKinokoRedEarly_red = "cobKinokoRed";

#include <game/bases/d_wm_lib.hpp>

/// @unofficial Local, MINIMAL restatement of the shared base class. The
/// authoritative provisional definition (with full method bodies) lives in
/// wip/wm_units/agent_kinoko_base/d_a_wm_kinoko_base.cpp (sizeof 0x290,
/// confirmed by that unit's own classInit -- `li r3, 0x290` at fn_2_16BDA0's
/// prologue, i.e. THIS unit's classInit, matches exactly). This TU does not
/// call any inherited method non-virtually and never instantiates a bare
/// daWmKinokoBase_c, so only the CLASS SHAPE (member layout + virtual slot
/// order) is needed here, not the bodies -- mirroring the same minimal-restate
/// approach wip/wm_kinoko_1up/complete/d_a_wm_kinoko_1up.cpp already uses
/// (that file's own copy is the OLDER/pre-correction layout; this one uses the
/// corrected fields now established in agent_kinoko_base's file).
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

/// @unofficial UNRESOLVED: the target's own dWmLib::sc_ForceList float pool
/// (lbl_2_rodata_8AF0) is 0x10 bytes (2160.0f, -30.0f, -478.0f, 0.0f) --
/// FOUR floats -- while this TU's own compile of the identical
/// `mVec3_c(2160.0f, -30.0f, -478.0f)` triple (from dWmLib::sc_ForceList's
/// header-side initializer, included via d_wm_lib.hpp above) only pools
/// THREE (0xC bytes), confirmed both by check_sections.py (`.rodata UNDER
/// 0x4`) and by directly reading the raw bytes at .rodata:0x8AF0 out of
/// original/d_basesNP.rel. Nothing in this TU's own code reads a 4th float
/// there (fn_2_16BEC0's __sinit, read directly from the REL, does exactly
/// three `lfs` at relative offsets 0, 4, 8 -- never 0xC), and mVec3_c /
/// nw4r::math::VEC3 are confirmed tightly-packed 3-float PODs with no
/// alignment padding of their own (include/lib/nw4r/math/math_types.h,
/// include/game/mLib/m_vec.hpp), so the pad is not part of the type's own
/// layout. A trailing `static const float = 0.0f;` declared adjacent in this
/// file does NOT reproduce it -- an unreferenced float constant with no side
/// effects is simply dead-code-eliminated, unlike the string-literal case
/// above (which survived because the same literal was reachable from the
/// later real use). daWmKinokoBase_c's OWN much larger rodata pool
/// (lbl_2_rodata_8AC8, 0x28 bytes, containing several OTHER floats from
/// createModel()/create() plus the SAME 3-float triple) shows NO equivalent
/// trailing pad after its own copy of this triple, so it is not a blanket
/// "every mVec3_c triple gets padded to 4 floats" rule either. Left open --
/// the 4-byte `.rodata` shortfall is the one real defect remaining in this
/// draft.

/// @brief The actor for the Red Mushroom-house marker (course start point) on
/// the World Map.
/// @unofficial class name; every symbol in this unit is anonymous (fn_2_*) in
/// the map. daWmKinokoRed_c mirrors the sibling naming convention
/// (daWmKinokoBase_c/daWmKinoko1up_c/daWmKinokoStar_c).
class daWmKinokoRed_c : public daWmKinokoBase_c {
public:
    daWmKinokoRed_c();
    virtual ~daWmKinokoRed_c();

    virtual void vf80();
    virtual void vf7C();
    virtual void vf84();
    virtual const char *getModelName();
};

ACTOR_PROFILE(WM_KINOKO_RED, daWmKinokoRed_c, 0);

daWmKinokoRed_c::daWmKinokoRed_c() {}

daWmKinokoRed_c::~daWmKinokoRed_c() {}

void daWmKinokoRed_c::vf7C() {}
void daWmKinokoRed_c::vf80() {}

/// @unofficial "cobKinokoRed", a SEPARATE instance from the one above --
/// verified present at 0x45B68, immediately AFTER this unit's own vtable
/// (which ends this unit's claimed .data span at 0x45B68). getModelName()
/// returns this by address-of (no `lwz`), matching fn_2_16BEB0's `lis/addi`
/// shape -- and exactly mirroring 1up's own getModelName(), which likewise
/// returns the FOLLOWING unit's leading string, not this unit's own
/// mModelResName. Not owned by this TU; the following unit's own agent will
/// give it a real name once landed.
extern "C" const char lbl_2_data_45B68[];

static const char *smc_animResNames_red[daWmKinokoRed_c::ANIM_COUNT] = {
    "cobKinokoAppear",
    "cobKinokoAppear",
};

/// @unofficial The REAL `mModelResName` pointer variable. Declared here (after
/// smc_animResNames_red) so it emits into .data in the position the target
/// actually has it -- immediately before the vtable, at unit offset 0x68 (see
/// smc_poolCobKinokoRedEarly_red above for why the string it points to is
/// pooled far earlier, at offset 0). Consumed by the inherited createModel()'s
/// `getRes(mModelResName, "g3d/model.brres")` call.
static const char *smc_modelResName_red = "cobKinokoRed";

void daWmKinokoRed_c::vf84() {
    mAnimResNames = smc_animResNames_red;
    mModelResName = smc_modelResName_red;
}

const char *daWmKinokoRed_c::getModelName() {
    return lbl_2_data_45B68;
}
