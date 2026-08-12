#pragma once
#include <game/bases/d_enemy.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/mdl.hpp>
#include <game/mLib/m_3d/anm_tex_pat.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>

class daEnBigPile_c;

/**
 * @brief Manager for one group of ("daikonbou") stalactite piles.
 * @details One manager exists per pile type. It keeps the piles of that type in
 * sync by remembering the frame on which the group started moving.
 * @unofficial
 */
class BigPileMng_c {
public:
    enum {
        MAX_PILE_COUNT = 12 ///< @unofficial
    };

    BigPileMng_c() {
        mIsMoving = false;
        mCount = 0;
        mStartFrame = 0;
        for (int i = 0; i < MAX_PILE_COUNT; i++) {
            mpPiles[i] = nullptr;
        }
    }

    ~BigPileMng_c() {}

    bool entry(daEnBigPile_c *pile); ///< Registers a pile with this manager.
    void remove(daEnBigPile_c *pile); ///< Unregisters a pile from this manager.
    void wait(); ///< Polls every registered pile for the start signal.
    void move(); ///< Ticks every registered pile.

    bool mIsMoving; ///< Whether the group has started moving. @unofficial
    int mCount; ///< How many piles are registered. @unofficial
    u32 mStartFrame; ///< The frame the group started moving on. @unofficial
    daEnBigPile_c *mpPiles[MAX_PILE_COUNT]; ///< The registered piles. @unofficial
};

/**
 * @brief Base implementation of a falling stalactite ("daikonbou") enemy.
 * @details The pile falls down, waits, then rises back up on a fixed cycle.
 * Derived actors supply the collision setup and the drawing position.
 * @paramtable
 */
class daEnBigPile_c : public dEn_c {
public:
    /// @brief The movement speed presets. @unofficial
    enum TYPE_e {
        TYPE_NORMAL,
        TYPE_QUICK,
        TYPE_SLOW
    };

    /// @brief The cycle phases returned by chkProcFrame(). @unofficial
    enum PROC_e {
        PROC_START_WAIT,
        PROC_GO,
        PROC_END_WAIT,
        PROC_RETURN
    };

    virtual ~daEnBigPile_c() {}

    // Base class overrides

    virtual int create();
    virtual int doDelete();
    virtual int execute();
    virtual int draw();
    virtual void deleteReady();

    virtual bool EnDamageCheck(dCc_c *self, dCc_c *other) { return false; }
    virtual bool PlDamageCheck(dCc_c *self, dCc_c *other);
    virtual bool YoshiDamageCheck(dCc_c *self, dCc_c *other) { return false; }
    virtual bool EtcDamageCheck(dCc_c *self, dCc_c *other);
    virtual void Normal_VsPlHitCheck(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Fire(dCc_c *self, dCc_c *other);
    virtual bool hitCallback_Ice(dCc_c *self, dCc_c *other);

    // New virtual functions

    virtual bool wait(); ///< Returns whether this pile wants its group to start moving.
    virtual void initCc(); ///< Sets up the two colliders.
    virtual void calcPos(float offset); ///< Applies the current cycle offset to the position.
    virtual void initWaterSt();
    virtual void afterMove(); ///< Runs at the end of move(). @unofficial
    virtual void initCullInfo();
    virtual void calcDrawPosAngle(mVec3_c &pos, mAng3_c &angle);
    virtual float getMoveDist(); ///< The distance travelled this frame, used for the rolling animation.

    // Nonvirtuals

    void createMdl();
    void calcMdl();
    void initMove();
    void initCnt();
    void moveProc();
    int move();
    void adjustOffset(float &offset);
    int chkProcFrame(ulong &frameOut, ulong frame);
    void callQuake();

    float moveGo_Normal(ulong frame);
    float moveGo_Quick(ulong frame);
    float moveGo_Slow(ulong frame);
    float moveRet_Normal(ulong frame);
    float moveRet_Quick(ulong frame);
    float moveRet_Slow(ulong frame);

    static int getAllPileNum(); ///< The total number of piles in the stage. @unofficial

    dHeapAllocator_c mAllocator;
    nw4r::g3d::ResFile mResFile;
    m3d::mdl_c mModel;
    nw4r::g3d::ResAnmTexPat mResAnmTexPat;
    m3d::anmTexPat_c mAnmTexPat;
    nw4r::g3d::ResAnmTexSrt mResAnmTexSrt;
    m3d::anmTexSrt_c mAnmTexSrt;
    int mType; ///< See TYPE_e. @unofficial
    mVec2_c mStartPos; ///< The position the pile was spawned at. @unofficial
    u32 mEndWait; ///< How long the pile waits at the bottom. @unofficial
    u32 mStartWait; ///< How long the pile waits before falling. @unofficial
    u32 mGoFrame; ///< How long the pile takes to fall. @unofficial
    u32 mRetFrame; ///< How long the pile takes to rise back up. @unofficial
    u32 mCounter; ///< The unsynchronised cycle counter. @unofficial
    u32 mFrameOffset; ///< The offset into the group's cycle. @unofficial
    dCc_c mCc2; ///< The pile's second collider. @unofficial
    float (daEnBigPile_c::*mGoFunc)(ulong); ///< The fall curve for this type. @unofficial
    float (daEnBigPile_c::*mRetFunc)(ulong); ///< The rise curve for this type. @unofficial
    dCc_c *mpCcs[2]; ///< Both colliders. @unofficial

    static BigPileMng_c m_manager[3]; ///< One manager per type. @unofficial

    ACTOR_PARAM_CONFIG(EndWait, 0, 8); ///< @unofficial
    ACTOR_PARAM_CONFIG(StartWait, 8, 8); ///< @unofficial
    ACTOR_PARAM_CONFIG(CrashEffect, 16, 1); ///< @unofficial
    ACTOR_PARAM_CONFIG(TexPattern, 17, 1); ///< @unofficial
    ACTOR_PARAM_CONFIG(NoSound, 18, 1); ///< @unofficial
    ACTOR_PARAM_CONFIG(Size, 20, 4); ///< @unofficial
    ACTOR_PARAM_CONFIG(Mode, 24, 2); ///< @unofficial
    ACTOR_PARAM_CONFIG(StartAtZero, 27, 1); ///< @unofficial
    ACTOR_PARAM_CONFIG(Type, 28, 2); ///< @unofficial
};
