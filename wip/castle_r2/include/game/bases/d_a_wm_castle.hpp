#pragma once

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_obj_actor.hpp>

/**
* @brief The actor for the World 7 boss castle on the World Map.
* @unofficial Reconstructed from anonymous (unnamed) target symbols; class name, member names
* and every enum value name are inferred from codegen evidence (dCsSeqMng_c::CUTSCENE_e values
* SMC_DEMO_CASTLE_CLR/FAIL/FAIL2 and SMC_DEMO_W1_CASTLE_CLR/W3_CASTLE_CLR match exactly), not
* from any mangled name.
* @ingroup bases
*/
class daWmCastle_c : public dWmObjActor_c {
public:
    /// @brief The available animations for this actor.
    /// @unofficial Names inferred from the target's own resource-name strings
    /// ("cobCastleOpen"/"cobCastleClose"/"cobCastleOut"/"cobCastleShake").
    enum ANIM_e {
        ANIM_OPEN,
        ANIM_CLOSE,
        ANIM_OUT,
        ANIM_SHAKE,
        ANIM_COUNT
    };

    typedef void (daWmCastle_c::*ProcFunc)();

    daWmCastle_c(); ///< @copydoc dWmObjActor_c::dWmObjActor_c
    ~daWmCastle_c(); ///< @copydoc dWmObjActor_c::~dWmObjActor_c

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    void createModel(); ///< Initializes the resources for the actor.
    void calcModel(); ///< Updates the model's transformation matrix.

    /// @brief Decides which SMC_DEMO_CASTLE_* cutscene to play based on the course's clear
    /// status, updates the door/shake animation accordingly, and queues the cutscene.
    void checkCourseResult();

    void mode_exec(); ///< Process function for the @ref dWmObjActor_c::PROC_TYPE_EXEC "exec" process type. Empty.
    void resetReaction(); ///< Resets #mCurrProc to PROC_TYPE_EXEC.

    /// @brief Spawns a one-shot child actor near the actor's associated Koopa-ship node.
    /// @unofficial Profile ID 0x29d (fProfile::WM_SURRENDER, per this project's current
    /// f_profile_name.hpp) confirmed against the target's `li r3, 0x29d` immediate. The enum
    /// member NAME is whatever this project calls slot 0x29d, not asserted to be semantically
    /// correct -- only the numeric ID is verified.
    void spawnKoopaNodeEffect();

    /// @brief Computes the world position of the "Koopa" bone on #mModel.
    /// @unofficial Return value confirmed against the target's `li r3, 1` right after the
    /// GetModelNodePos call; always returns true.
    bool getKoopaPos(mVec3_c &out) const;

    /// @brief Looks up a live daWmCastle_c via fManager_c::searchBaseByProfName(WM_CASTLE,
    /// nullptr) and, if found, plays its "stop" reaction (shake animation, sound, effect).
    /// @unofficial GLOBAL (not weak) symbol binding in the target rules out an inline-in-header
    /// method of another class -- see this task's report. fProfile::WM_CASTLE == 0x272,
    /// compiler-verified (not counted from source), confirmed against the target's `li r3,
    /// 0x272` immediate.
    static void TriggerCastleStopReaction(float rate, float frame);

    /// @brief Plays the "stop" reaction on THIS castle: shake animation, sound, effect, unless
    /// it is already stopped and at the target rate.
    /// @unofficial Operates on #mChrAnim[ANIM_SHAKE] and #mModel; offsets 0x1ac/0x260/0xac match
    /// daWmCastle_c's own layout exactly, which is what shows the "found" object in
    /// TriggerCastleStopReaction is genuinely another daWmCastle_c, not a different class.
    void applyStopReaction(float rate, float frame);

    /// @brief mPos offset by the (guarded, dynamically-initialised) config in #sc_KoopaShipStopConfig.
    /// @unofficial GLOBAL binding and no callers within this TU's own .text -- likely part of the
    /// public API, called from an undecompiled sibling unit.
    /// NOT `const`, and that is load-bearing rather than an oversight -- see the note on the
    /// definition in the .cpp. Declaring it `const` costs 6 of its 15 instructions.
    mVec3_c getKoopaShipStopPos();

    u32 mUnk188; ///< @unused @unofficial offset 0x188, identical position to daWmCloud_c::mUnk188 and daWmSmallCloud_c::mUnk188.
    dHeapAllocator_c mAllocator; ///< The allocator. @unofficial offset 0x18c.
    nw4r::g3d::ResFile mResFile; ///< The resource file. @unofficial offset 0x1a8.
    m3d::smdl_c mModel; ///< The model. @unofficial offset 0x1ac.
    m3d::anmChr_c mChrAnim[ANIM_COUNT]; ///< The model animations. @unofficial offset 0x1b8.

    /// @brief The pending dCsSeqMng_c::CUTSCENE_e to queue once #checkCourseResult finishes, or
    /// -1 for none. @unofficial offset 0x298.
    int mCutscene;
    PROC_TYPE_e mCurrProc; ///< The current process type. @unofficial offset 0x29c.
    bool m_2a0; ///< @unofficial offset 0x2a0. Set when the door/shake reaction anim is playing.
    mVec3_c mKoopaSpawnPos; ///< @unofficial offset 0x2a4. Filled by #getKoopaPos, used to spawn #spawnKoopaNodeEffect's child actor.
    bool m_2b0; ///< @unofficial offset 0x2b0. Guards #spawnKoopaNodeEffect to run at most once per #checkCourseResult call.
    int m_2b4; ///< @unofficial offset 0x2b4. A frame counter, decremented in #processCutsceneCommand.

private:
    static const ProcFunc Proc_tbl[PROC_COUNT];
};
