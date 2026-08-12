#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/anm_tex_pat.hpp>
#include <game/mLib/m_effect.hpp>

/**
 * @brief Base class for Big Goombas ("large kuribo").
 * @note Despite the name, this is a sibling of daEnKuriboBase_c rather than a
 * subclass: it derives from dEn_c directly, and overrides createMdl/calcMdl/calcJnt
 * where daEnKuriboBase_c has createModel/calcModel.
 * @statetable
 * @paramtable
 */
class daEnLkuriboBase_c : public dEn_c {
public:
    virtual ~daEnLkuriboBase_c();

    // Base class overrides

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual void finalUpdate();

    virtual void Normal_VsEnHitCheck(dCc_c *self, dCc_c *other);
    virtual void Normal_VsPlHitCheck(dCc_c *self, dCc_c *other);
    virtual void Normal_VsYoshiHitCheck(dCc_c *self, dCc_c *other);

    virtual bool hitCallback_Spin(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_HipAttk(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_YoshiHipAttk(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Fire(dCc_c *self, dCc_c *other);

    STATE_VIRTUAL_FUNC_DECLARE(daEnLkuriboBase_c, DieFall);

    virtual void fumidamageSE(const mVec3_c &pos, int playerNo);
    virtual bool setDamage(dActor_c *actor);
    virtual void calcBoyonScale();
    virtual void FumiJumpSet(dActor_c *actor);

    // New virtual functions

    STATE_VIRTUAL_FUNC_DECLARE(daEnLkuriboBase_c, Walk); ///< Walking on the ground.
    STATE_VIRTUAL_FUNC_DECLARE(daEnLkuriboBase_c, Turn); ///< Turning around.
    STATE_VIRTUAL_FUNC_DECLARE(daEnLkuriboBase_c, Press); ///< Being squashed.
    STATE_VIRTUAL_FUNC_DECLARE(daEnLkuriboBase_c, Split); ///< Splitting into smaller Goombas.
    STATE_VIRTUAL_FUNC_DECLARE(daEnLkuriboBase_c, HipSplit); ///< Splitting after a ground pound.

    virtual void initialize();
    virtual void setTurnByEnemyHit();
    virtual void setTurnByPlayerHit(dActor_c *actor);
    virtual void setWalkSpeed();
    virtual void setWalkAnm();
    /// @note Inline in the class body on purpose. It is called from
    /// executeState_Walk, so an out-of-line copy is still emitted -- lazily, at
    /// that caller's flush point, which is where the original has it. Defining
    /// it out of line instead emits it one slot too early, ahead of the template
    /// instantiation the same caller queues.
    virtual void walkEffect() {}
    virtual void firehitSE();
    virtual void splitSE();
    virtual void splitEffect();
    virtual void hipsplitSE();
    virtual void hipsplitEffect();
    virtual void split();
    virtual void hipsplit();

    // Nonvirtual functions

    void createMdl();
    void calcMdl();
    void calcJnt();
    void setDeathInfo_Hasami();
    void setBoyoFunc(void (daEnLkuriboBase_c::*func)());
    void nonBoyoProc();
    void fireBoyoProc();
    void setAnm(char *name, m3d::playMode_e playMode, float rate);
    void setTexAnm(char *name, m3d::playMode_e playMode);
    void pressAttach();

    dHeapAllocator_c mAllocator; ///< [0x524] The actor's allocator.
    nw4r::g3d::ResFile mResFile; ///< [0x540] The actor's model resource file.
    m3d::mdl_c mModel; ///< [0x544] The actor's model.
    m3d::anmChr_c mAnmChr; ///< [0x584] The actor's character animation.
    nw4r::g3d::ResAnmTexPat mResAnmTexPat; ///< [0x5BC] The texture pattern animation resource.
    m3d::anmTexPat_c mAnmTexPat; ///< [0x5C0] The actor's texture pattern animation.
    u32 m_5ec; ///< [0x5EC] Unused by this class.
    mEf::levelEffect_c mEffect; ///< [0x5F0] The actor's effect.

    mVec3_c mLegLeftPos; ///< [0x718] World position of the ``leg_left`` joint. @unofficial
    mVec3_c mLegRightPos; ///< [0x724] World position of the ``leg_right`` joint. @unofficial
    mVec3_c mBrowPos; ///< [0x730] World position of the ``brow`` joint. @unofficial
    float mPressX; ///< [0x73C] The attacker's X position when ground pounded. @unofficial
    float m_740; ///< [0x740] Unused by this class.
    int mPressPlayerNo; ///< [0x744] The ground pounding player's number. @unofficial
    s16 mFireHp; ///< [0x748] Remaining fireball hits. Set by the derived actor. @unofficial
    void (daEnLkuriboBase_c::*mBoyoFunc)(); ///< [0x74C] The squash scale callback. @unofficial
    int mBoyoStep; ///< [0x758] The squash callback's step counter. @unofficial
    mVec3_c mBoyoScale; ///< [0x75C] Scale offset applied by the squash callback. @unofficial
    int m_768; ///< [0x768] Remaining squash repetitions. @unofficial
    s16 m_76c; ///< [0x76C] The squash wave's angle. @unofficial
};
