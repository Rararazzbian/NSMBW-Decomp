#pragma once

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_wm_bgm_sync.hpp>

/**
* @brief The actor for the small decorative clouds used in the World 7 map.
* @details A near-twin of #daWmCloud_c: a single-bone animated cloud model, synchronised to the
* background music via #mpBgmSync, with no per-node culling (unlike daWmCloud_c, which culls each
* of its named bone groups individually every frame). Instead, every frame it repositions itself to
* a single named world-map node selected by ACTOR_PARAM(CourseNo) (see #setPosFromCourseNode).
* @unofficial Reconstructed from anonymous (unnamed) target symbols; class name, member names and
* the exact GlobalData_t shape are inferred from codegen evidence, not from any mangled name.
* @ingroup bases
*/
class daWmSmallCloud_c : public dWmObjActor_c {
public:
    /// @brief The global configuration for the actor.
    /// @unofficial Confirmed byte-for-byte against the target's .rodata (0x8fa8-0x8fb0, the two
    /// leading words of the TU's local pool): 00080000 00080000 decodes as {8,0},{8,0}. Exactly
    /// 8 bytes, no padding -- an earlier guess at an 8-byte mUnofficialPad tail was speculative
    /// and is dropped now that the real bytes are known.
    union GlobalData_t {
        struct {
            s16 mBgmValueW5[2]; ///< @unofficial BGM sync value used when dScWMap_c::m_WorldNo == 5. {8, 0}.
            s16 mBgmValue[2]; ///< @unofficial BGM sync value used otherwise. {8, 0}.
        };
        u32 mRaw[2];
    };

    /// @brief The available animations for this actor.
    /// @unofficial Name copied from daWmCloud_c's ANIM_e; only the single-entry shape is confirmed.
    enum ANIM_e {
        CS_W7_SmallCloud,
        ANIM_COUNT
    };

    typedef void (daWmSmallCloud_c::*ProcFunc)();

    daWmSmallCloud_c(); ///< @copydoc dWmObjActor_c::dWmObjActor_c
    ~daWmSmallCloud_c(); ///< @copydoc dWmObjActor_c::~dWmObjActor_c

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    void createModel(); ///< Initializes the resources for the actor. @unofficial NOT byte-matched, see MERGED.md.
    void calcModel(); ///< Updates the model's transformation matrix.
    void initState(); ///< Sets up the actor's initial state (course-node position, anim rate, and the
                       ///< CourseNo == 3 castle-clear visibility gate).
    void init_exec(); ///< Process initialization function for the @ref dWmObjActor_c::PROC_TYPE_EXEC "exec" process type.
    void mode_exec(); ///< Process function for the @ref dWmObjActor_c::PROC_TYPE_EXEC "exec" process type.

    /// @brief Repositions the actor to a named world-map node, indexed by ACTOR_PARAM(CourseNo).
    /// @unofficial daWmCloud_c has no equivalent; that unit culls named bone GROUPS instead of
    /// repositioning to a single node. Table contents are unconfirmed placeholders, see MERGED.md.
    void setPosFromCourseNode();

    u32 mUnk188; ///< @unused @unofficial offset 0x188, identical position to daWmCloud_c::mUnk188.
    dHeapAllocator_c mAllocator; ///< The allocator.
    nw4r::g3d::ResFile mResFile; ///< The resource file.
    m3d::smdl_c mModel; ///< The model.
    m3d::anmChr_c mChrAnim[ANIM_COUNT]; ///< The model animations.
    u32 mUnk1f0; ///< @unused @unofficial offset 0x1f0 (daWmCloud_c's equivalent field is
                 ///< misleadingly named mUnk250 -- see MERGED.md "layout contradiction" section).
    PROC_TYPE_e mCurrProc; ///< The current process type. See dWmObjActor_c::PROC_TYPE_e.
    dWmBgmSync_c *mpBgmSync; ///< The background music synchronization helper, @ 0x1f8.
    // Total size 0x1fc (measured, classInit's operand). Deliberately NO mGroupNodeIds/
    // mCurrNodeClipSphere/sGroupNodeNames -- see MERGED.md for the full proof and the
    // arraydtor contradiction this raised and how it was resolved.
};
