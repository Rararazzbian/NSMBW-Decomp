#pragma once
#include <game/bases/d_actor_state.hpp>
#include <game/bases/d_bg_ctr.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>
#include <game/mLib/m_effect.hpp>

/**
 * @brief The block of ice an enemy is encased in when frozen.
 * @details The actor is connected to the enemy it freezes; it follows the enemy
 * around, and melts, breaks or thaws depending on how it is destroyed.
 * @ingroup bases
 */
class daEnemyIce_c : public dActorState_c {
public:
    daEnemyIce_c() : m_588(0) {}
    virtual ~daEnemyIce_c();

    // Base class overrides

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual void postExecute(fBase_c::MAIN_STATE_e status);
    virtual int draw();
    virtual void deleteReady();

    // Nonvirtuals

    void createMdl();
    void calcMdl();
    void updatePos();
    void calcMeltScaleRate();
    void calcMeltSpeed();
    void calcEffectPos();

    void freezeBeginEffect();
    void freezeEffect();
    void revivalEffect();
    void meltEffect();
    void breakEffect();

    STATE_FUNC_DECLARE(daEnemyIce_c, Freeze); ///< The enemy is frozen.
    STATE_FUNC_DECLARE(daEnemyIce_c, Revival); ///< The enemy thaws out.
    STATE_FUNC_DECLARE(daEnemyIce_c, Melt); ///< The ice block melts away.
    STATE_FUNC_DECLARE(daEnemyIce_c, Break); ///< The ice block shatters.

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;
    m3d::mdl_c mModel;
    nw4r::g3d::ResAnmTexSrt mResAnmTexSrt;
    m3d::anmTexSrt_c mAnmTexSrt;
    dBg_ctr_c mBgCtr; ///< The ice block's collision.
    mVec3_c mIceSize; ///< @unofficial
    mVec3_c mPosOffset; ///< Offset from the frozen enemy's position. @unofficial
    mVec3_c mEffectPos1; ///< @unofficial
    mVec3_c mEffectPos2; ///< @unofficial
    mVec3_c mMeltScaleRate; ///< How much the block shrinks per frame while melting. @unofficial
    int *mpMeltTimer; ///< @unofficial
    int m_584; ///< @unofficial
    int m_588; ///< @unofficial
    mEf::levelEffect_c mEffect;
    int m_6b4; ///< @unofficial

    static mVec3_c smc_ICE_DEFSIZE_SQUARE;
    static mVec3_c smc_ICE_DEFSIZE_TATE;
    static mVec3_c smc_ICE_DEFSIZE_YOKO;
    static mVec3_c smc_ICE_DEFSIZE_BIG_SQUARE;
    static mVec3_c smc_ICE_DEFSIZE_BIG_TATE;
    static mVec3_c smc_ICE_DEFSIZE_BIG_YOKO;

    ACTOR_PARAM_CONFIG(IceType, 0, 4); ///< @unofficial
    ACTOR_PARAM_CONFIG(EffectOfs, 4, 1); ///< @unofficial
    ACTOR_PARAM_CONFIG(NoMelt, 8, 1); ///< @unofficial
};
