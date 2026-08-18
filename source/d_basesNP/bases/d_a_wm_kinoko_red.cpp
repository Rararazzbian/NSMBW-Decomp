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

/// @unofficial UNRESOLVED (re-investigated on request, still open): the
/// target's own dWmLib::sc_ForceList float pool (lbl_2_rodata_8AF0) is 0x10
/// bytes -- (2160.0f, -30.0f, -478.0f, 0.0f) -- while this TU's own compile
/// of the identical `mVec3_c(2160.0f, -30.0f, -478.0f)` triple (from
/// dWmLib::sc_ForceList's header-side initializer, included via d_wm_lib.hpp
/// above) only pools THREE floats (0xC bytes). Confirmed both by
/// check_sections.py (`.rodata UNDER 0x4`) and by directly reading the raw
/// bytes at .rodata:0x8AF0 out of original/d_basesNP.rel.
///
/// Two landed siblings show the analogous LEADING-pad shape, both explained:
/// - source/d_basesNP/bases/d_a_wm_grid.cpp opens its pool with a 0.0f via a
///   deliberate `DECL_WEAK void DUMMY_ORDERING() { static const float
///   UNUSED[] = { 0.0f }; }`, hand-documented as "required to ensure correct
///   .rodata pool ordering" and "deadstripped by the linker later" -- i.e. a
///   known, accepted workaround in this project for exactly this class of
///   problem, not real game logic.
/// - source/d_basesNP/bases/d_a_wm_tower.cpp has NO such trick and no 0.0f
///   literal anywhere in its own source, yet its own pool (.rodata:0x9320)
///   also opens with a leading word -- 100.0f, not 0.0f -- read directly out
///   of original/d_basesNP.rel. That is tower's own real, referenced
///   constant: `setClipSphere()` (include/game/bases/d_a_wm_tower.hpp) is
///   `mClipSphere.set(mPos, 100.0f)`, called from `create()`, tower's own
///   first-defined function -- so it is a genuine "missing/earlier use",
///   confirming the coordinator's deduplication theory for the LEADING case.
///
/// For RED's TRAILING case, both explanations were tested and ruled out:
/// - fn_2_16BEC0 (this unit's own __sinit, read directly from the REL) does
///   exactly three `lfs` at relative pool offsets 0, 4, 8 -- never 0xC. It
///   does not read the fourth word.
/// - Every other function in this unit (ctor, dtor, vf7C, vf80, vf84,
///   getModelName, classInit) is independently confirmed byte-identical by
///   verify_anon.py, and none of them contains a floating-point instruction
///   at all -- there is no candidate "missing use" of 0.0f anywhere in this
///   TU's own compiled functions for a fifth pool entry to come from.
/// - Empirically reproducing grid's own trick (a DECL_WEAK function-local
///   `static const float UNUSED[] = { 0.0f };`) in this draft, placed at the
///   VERY END of the file (textually after getModelName, and landing at
///   .text offset 0x120 -- after every one of this unit's own real
///   functions, and still before the compiler-generated __sinit, which is
///   always the last-compiled function in every unit observed in this
///   family, base included), still pools the dummy 0.0f BEFORE the triple,
///   not after -- reproducing grid's shape, not red's. Nothing tried moves a
///   user-declared float to the trailing position, because __sinit itself
///   appears to always be the last thing compiled, so nothing textual can
///   compile "after" it to explain a TRAILING pad the way grid/tower's
///   LEADING pad is explained.
///
/// Net: this is a genuinely different finding from grid/tower's case, not
/// the same mechanism with an undiscovered use. Recording it as such rather
/// than forcing a fix. The 4-byte `.rodata` shortfall remains the one real
/// defect left in this draft.

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
    /// @unofficial INLINE ON PURPOSE. A weak (inline) function's anonymous
    /// literal pool is deferred to the vtable-construction pass, which is what
    /// puts this string at unit offset 0xf8, AFTER __vt__, exactly as the target
    /// has it. An out-of-line definition emits it eagerly, ahead of the vtable,
    /// and nothing else recovers the position -- see d_a_wm_kinoko_base.cpp for
    /// the same fix and the rule behind it.
    ///
    /// Note this TU therefore carries "cobKinokoRed" TWICE: the strong copy at
    /// unit offset 0, referenced by smc_modelResName, and this weak one at 0xf8.
    /// `#pragma reuse_strings` does not merge a strong-bound literal with a
    /// weak-bound one, and the TARGET has both copies too.
    virtual const char *getModelName() { return "cobKinokoRed"; }
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

