#pragma once
#include <game/bases/d_enemy_boss.hpp>
#include <game/bases/d_cc.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/sLib/s_State.hpp>
#include <cstddef>


class daPlBase_c : public dActor_c {
public:
    u8 mPadPlBase[0xCE0];
    u32 mUnk1074;
    u32 mUnk1078;
    u8 mPadPlBase2[0x1090 - 0x107C];
    int mUnk1090;
};

class KokoopaSpFumiCheck_c : public FumiCheckBase_c {
public:
    virtual ~KokoopaSpFumiCheck_c() {}
    virtual bool operate(int &arg0, dEn_c *en, FumiCcInfo_c &info);
};

class dEnTorideKokoopa_c : public dEnBoss_c {
public:
    dEnTorideKokoopa_c();
    virtual ~dEnTorideKokoopa_c();

    // 41 Overrides from dEnBoss_c / dEn_c / dActor_c / fBase_c
    virtual int preExecute();
    virtual void postExecute(fBase_c::MAIN_STATE_e status);
    virtual int draw();
    virtual void finalUpdate();
    virtual mVec2_c getLookatPos() const;
    virtual bool hitCallback_PenguinSlide(dCc_c *self, dCc_c *other);
    virtual BOOL isQuakeDamage();

    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoWait);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DieFire);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DieShell);

    virtual void setBattleReady();
    virtual void tenmetsuProc();
    virtual void tenmetsuFin();
    virtual int getTenmetsuTime_Fire();
    virtual int getTenmetsuTime_Press();
    virtual void setFumiDamage(dActor_c *killedBy);
    virtual void setFumiDead(dActor_c *killedBy);
    virtual void setFireDamage(dActor_c *killedBy);
    virtual void setFireDead(dActor_c *killedBy);
    virtual void setStarDamage(dActor_c *killedBy);
    virtual void setStarDead(dActor_c *killedBy);
    virtual void setQuakeDamage();
    virtual void setQuakeDead();
    virtual void setShellDamage(dActor_c *killedBy);
    virtual void setShellDead(dActor_c *killedBy);
    virtual void damageProc();
    virtual void deadProc();
    virtual BOOL isFumiInvalid() const;
    virtual BOOL isFireInvalid() const;
    virtual BOOL isStarInvalid() const;
    virtual void fumideadEffect();
    virtual void fumidmgEffect();
    virtual void damageSVo();
    virtual void damageLVo();

    // 20 New State Declarations (Slots 226..285)
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, Jump_St);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, Jump);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, BigJump_St);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, BigJump);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, LandOn);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, AttackReady);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, AttackBegin);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, AttackSearch);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, Attack);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, AttackEnd);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, FumiHit);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, FireHit);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, SlideHit);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, StarHit);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, QuakeHit);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, ShellHit);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, ShellAtk_St);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, ShellAtk);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, ShellOut);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DieFumi_St);

    // 89 New Virtual Methods (Slots 286..374)
    virtual bool lockonTurn(); // 0x480
    virtual void calcKokoopaMdl();
    virtual void calcShellMdl();
    virtual void drawKokoopa();
    virtual void drawShell();
    virtual void setBeginMoveState(); // 0x494
    virtual void moveAdjust_HIO();
    virtual int getAtkEndTime();
    virtual int getAtkEndTime_Wait();
    virtual int getAtkSearchTime();
    virtual int getAtkSearch2ndTime();
    virtual int getDownTime();
    virtual void setAtkCnt();
    virtual float getJumpGravity();
    virtual float getDrawScale();
    virtual void speedUp();
    
    virtual int beginDance();
    virtual void getTurnSpeed();
    virtual int getFumiRecoverTime();
    virtual int createBlitz();
    virtual void createBlitz_sub() = 0;
    virtual mVec3_c getMagicStickEffectOffset() const;
    virtual void setKokoopaCc();
    virtual void setShellCc();
    virtual float getJumpDist() const; // 0x4E0
    virtual mVec3_c calcBlitzPos(); // 0x4E4
    virtual void magicHoldOffEffect(); // 0x4E8
    virtual void magicShootEffect(); // 0x4EC
    virtual void setBlitzTarget(); // 0x4F0
    virtual mVec3_c calcFacePos(); // 0x4F4
    virtual void blitzShoot(); // 0x4F8
    virtual void calcCcData(); // 0x4FC
    virtual void calcWandCcData(); // 0x500
    virtual float getKokoopaOffFrm() const; // 0x504
    virtual float getShellOnFrm() const; // 0x508
    virtual bool checkGetUp() const; // 0x50C
    virtual float getKokoopaOnFrm() const; // 0x510
    virtual float getShellOffFrm() const; // 0x514
    virtual float getCreateBlitzFrm() const; // 0x518
    virtual float getShootFrm() const; // 0x51C
    virtual int defaultDirAngle(); // 0x520
    virtual float getShellChangeEffectOffsetY() const; // 0x524
    virtual void jumpEffect(); // 0x528
    virtual void jumpRootEffect(); // 0x52C
    virtual void landonEffect(); // 0x530
    virtual void downLandOnEffect(float); // 0x534
    virtual void downFallEffect(); // 0x538
    virtual void hitFireDamageEffect(); // 0x53C
    virtual void hitFireLoopEffect(); // 0x540
    virtual void shellLandonEffect(); // 0x544
    virtual void shellAtkEffect(); // 0x548
    virtual void hitShellDamageEffect(); // 0x54C
    virtual void shellinSE(); // 0x550
    virtual void shellatkSE(); // 0x554
    virtual void shelloutSE(); // 0x558
    virtual void jumpSE(); // 0x55C
    virtual void landonSE(); // 0x560
    virtual void shelllandonSE(); // 0x564
    virtual void shellBumMarEffect(); // 0x568
    virtual void shellOutEffect(); // 0x56C
    virtual void shellChangeEffect(); // 0x570
    virtual void ikakuEffect(); // 0x574
    virtual void ikakuEffect2(); // 0x578
    virtual void getupSE(); // 0x57C
    virtual void blitzchargeSE(); // 0x580
    virtual void notice1Vo(); // 0x584
    virtual void wakeVo(); // 0x588
    virtual void magicShotVo(); // 0x58C
    virtual void shellOutVo(); // 0x590
    virtual void deadVo();
    virtual void loseFirstVo();
    virtual void loseSecondVo();

    // Demo States (Slots 358..372)
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoAwake);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoAwake_Wait);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoIkaku);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoIkaku_Wait);
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoEscape_St);

    virtual void awakeSE();
    virtual void ikakuSE();

    // Member functions
    void calcRootJntPos();
    void calcShellJntPos();
    bool isTorideBoss();
    bool checkDownJump();
    void changeShell();
    void changeKokoopa();
    int getTorideFunfareTime();
    float calcJumpRate();
    bool isCreateBlitz() const;
    bool isShootBlitz() const;
    void blitzMove(dActor_c *blitz);
    u32 calcAttackTarget();
    void calcLookAngle();
    int getPressTime();
    mVec3_c getPressScale();
    void notice2Vo();
    void escJumpVo();
    
    // Member variables (0x600..0xE70)
    nw4r::g3d::ResFile mResFile;
    m3d::mdl_c mMdlKokoopa;
    m3d::anmChr_c mAnmChrKokoopa;
    m3d::anmMatClr_c mAnmMatClr;
    u32 mUnk6A8;
    m3d::anmTexPat_c mAnmTexPat;
    m3d::mdl_c mMdlShell;
    m3d::anmChr_c mAnmChrShell;
    struct ParamReady {
        const char *mAnmNames[1];
    };
    ParamReady *mpParamReady; // 0x750
    struct ParamJump {
        const char *mAnmNames[4];
        mVec2_c mJumpSpeed1;
        mVec2_c mBigJumpSpeed1;
        mVec2_c mJumpSpeed2;
        mVec2_c mBigJumpSpeed2;
    };
    ParamJump *mpParamJump; // 0x754
    struct ParamAttack {
        const char *mAnmNames[4];
        u32 mPad10;
    };
    ParamAttack *mpParamAttack; // 0x758
    struct ParamShell {
        const char *mAnmNames[6];
    };
    ParamShell *mpParamShell; // 0x75C
    u32 mUnk760;
    u32 mUnk764;
    s32 mUnk768;
    u32 mPad76C;
    u32 mUnk770;
    mVec3_c mBlitzPos;
    mVec3_c mLookatPos;
    u32 mPad78C;
    s16 mUnk790;
    s16 mUnk792;
    s32 mUnk794;
    s32 mAtkCnt;
    dCc_c mCc;
    f32 mUnk840;
    f32 mUnk844;
    u32 mPad848;
    s32 mFireTimer;
    mEf::levelEffect_c mLevelEffect1;
    mEf::levelEffect_c mLevelEffect2;
    u32 mUnkAA0;
    mVec3_c mUnkAA4;
    mVec3_c mUnkAB0;
    u32 mPadABC;
    s16 mUnkAC0;
    u8 mPadAC2[2];
    f32 mUnkAC4;
    s32 mUnkAC8;
    f32 mUnkACC;
    s32 mRootJntIdx;
    s32 mShellJntIdx;
    mVec3_c mRootJntPos;
    mVec3_c mShellJntPos;
    u32 mUnkAF0;
    mEf::levelEffect_c mLevelEffects[3];
    void *mVoiceParam;
};

char sizeof_dEnTorideKokoopa_c[sizeof(dEnTorideKokoopa_c) == 0xE70 ? 1 : -1];
