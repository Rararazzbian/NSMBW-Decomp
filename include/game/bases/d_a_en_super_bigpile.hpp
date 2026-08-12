#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_tex_pat.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>

/**
 * @brief Base implementation of a giant pile ("daikonbou") enemy.
 * @details The pile rises out of the ground, waits, then sinks back down.
 * Derived actors supply the movement distances and speeds.
 * @statetable
 * @paramtable
 */
class daEnSuperBigPile_c : public dEn_c {
public:
    virtual ~daEnSuperBigPile_c();

    // Base class overrides

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual void postExecute(fBase_c::MAIN_STATE_e status);
    virtual int draw();
    virtual void deleteReady();

    virtual bool EnDamageCheck(dCc_c *self, dCc_c *other);
    virtual bool PlDamageCheck(dCc_c *self, dCc_c *other);
    virtual bool YoshiDamageCheck(dCc_c *self, dCc_c *other);
    virtual bool EtcDamageCheck(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Fire(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Ice(dCc_c *self, dCc_c *other);

    STATE_VIRTUAL_FUNC_DECLARE(daEnSuperBigPile_c, GoWait); ///< Waiting before moving out.
    STATE_VIRTUAL_FUNC_DECLARE(daEnSuperBigPile_c, GoMove); ///< Moving out.
    STATE_VIRTUAL_FUNC_DECLARE(daEnSuperBigPile_c, RetWait); ///< Waiting before returning.
    STATE_VIRTUAL_FUNC_DECLARE(daEnSuperBigPile_c, RetMove); ///< Returning.

    // New virtual functions

    virtual float getCamDist();
    virtual void calcFallSpeed();
    virtual void calcRiseSpeed();
    virtual void createMdl();
    virtual void setTexRotate();
    virtual void initialize();
    virtual float getMoveDist();
    virtual void setCcInfo();
    virtual mVec3_c getDrawPos();
    virtual void setQuake();
    virtual void fallDownSE();
    virtual void riseUpSE();
    virtual bool isEffectCall();
    virtual s16 getDefaultRollSpeed() const;
    virtual void crashEffect();

    // Nonvirtuals

    void calcMdl();
    void rolling();

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;
    m3d::mdl_c mModel;
    nw4r::g3d::ResAnmTexSrt mResAnmTexSrt;
    m3d::anmTexSrt_c mAnmTexSrt;
    nw4r::g3d::ResAnmTexPat mResAnmTexPat;
    m3d::anmTexPat_c mAnmTexPat;
    mVec3_c mStartPos; ///< The position the pile was spawned at.
    dCc_c mCc; ///< The pile's collider.
    int mIsEffectCall; ///< Whether effects and sounds should be played this frame. @unofficial
    int mGoDist; ///< How far the pile travels when moving out. @unofficial
    int mRetDist; ///< How far the pile travels when returning. @unofficial

    ACTOR_PARAM_CONFIG(GoDist, 0, 8); ///< @unofficial
    ACTOR_PARAM_CONFIG(RetDist, 8, 8); ///< @unofficial
    ACTOR_PARAM_CONFIG(CrashEffect, 16, 1); ///< @unofficial
    ACTOR_PARAM_CONFIG(TexPattern, 17, 1); ///< @unofficial
};
