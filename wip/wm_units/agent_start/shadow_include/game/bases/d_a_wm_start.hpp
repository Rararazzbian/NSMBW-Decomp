#pragma once

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_tex_pat.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/mLib/m_allocator.hpp>
#include <game/bases/d_wm_obj_actor.hpp>

/**
* @brief The actor for the World 7 "kinoko house" start point marker on the World Map.
* @unofficial Reconstructed from anonymous (unnamed) target symbols; class name, member names
* and every enum value name are inferred from codegen evidence (dWmLib::isStartPointKinokoHouseStar/
* Red/1up, dWmLib::getStartPointKinokoHouseKindNum), not from any mangled name.
* @ingroup bases
*/
class daWmStart_c : public dWmObjActor_c {
public:
    /// @unofficial offset 12, width 4. Read by #createModel via `extrwi. r0, r0, 4, 12`.
    ACTOR_PARAM_CONFIG(Kind, 16, 4);

    daWmStart_c(); ///< @copydoc dWmObjActor_c::dWmObjActor_c
    ~daWmStart_c(); ///< @copydoc dWmObjActor_c::~dWmObjActor_c

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    void createModel(); ///< Initializes the resources for the actor. @unofficial fn_2_17A940. MATCHES
                          ///< modulo a 2-slot stack-staging swap in the by-value ResMdl temporary's
                          ///< second use -- see this task's report.
    void calcModel(); ///< Updates the model's transformation matrix. @unofficial fn_2_17AA90. MATCHES.

    /// @unofficial fn_2_17A3C0 (0x39c bytes) -- NOT YET AUTHORED this round. Called from #create
    /// when IsCourseClear() && IsCourseFirstClear() are both true. Placeholder name/signature.
    void unk_17A3C0();
    /// @unofficial fn_2_17A760 (0x108 bytes) -- NOT YET AUTHORED this round. Called from #create
    /// when IsCourseClear() is false. Placeholder name/signature.
    void unk_17A760();

    u32 mUnk188; ///< @unused @unofficial offset 0x188, identical position to every other landed WM actor's mUnk188.
    dHeapAllocator_c mAllocator; ///< The allocator. @unofficial offset 0x18c.
    nw4r::g3d::ResFile mResFile; ///< The resource file. @unofficial offset 0x1a8.
    m3d::smdl_c mModel; ///< The model. @unofficial offset 0x1ac.
    m3d::anmTexPat_c mAnimTexPat; ///< The texture-pattern animation. @unofficial offset 0x1b8 --
                                   ///< confirmed by the constructor's vtable-patch sequence
                                   ///< (banm_c's own vtable written first, patching in its own
                                   ///< embedded mAllocator member at +0xc, then anmTexPat_c's
                                   ///< own vtable+mpChildren), not by any mangled name. Ends at
                                   ///< 0x1b8+sizeof(anmTexPat_c) = 0x1e4 (compiled sizeof, see
                                   ///< this task's report).
    u8 mPad_1e4[0x1c]; ///< @unofficial offset 0x1e4, size 0x1c. Between the end of
                         ///< #mAnimTexPat (0x1e4) and the flag byte at 0x200 the constructor
                         ///< does not construct or zero anything -- no `bl __ct__...` and no
                         ///< `stw`/`stb` in that range. Content and true type unrecovered;
                         ///< @unofficial padding by elimination.
    bool m_200; ///< @unofficial offset 0x200. Zeroed by the constructor; role unconfirmed.
};
