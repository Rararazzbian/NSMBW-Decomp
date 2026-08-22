#pragma once
#include <game/bases/d_enemy_boss.hpp>
#include <game/mLib/m_effect.hpp>

class KokoopaSpFumiCheck_c : public FumiCheckBase_c {
public:
    virtual ~KokoopaSpFumiCheck_c() {}
    virtual bool operate(int &result, dEn_c *en, FumiCcInfo_c &fumi);
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
    virtual int lockonTurn(); // 0x480
    virtual void calcKokoopaMdl(); // 0x484
    virtual void calcShellMdl(); // 0x488
    virtual void drawKokoopa(); // 0x48C
    virtual void drawShell(); // 0x490
    virtual void setBeginMoveState(); // 0x494
    virtual void moveAdjust_HIO(); // 0x498
    virtual int getAtkEndTime(); // 0x49C
    virtual int getAtkEndTime_Wait(); // 0x4A0
    virtual int getAtkSearchTime(); // 0x4A4
    virtual int getAtkSearch2ndTime(); // 0x4A8
    virtual int getDownTime(); // 0x4AC
    virtual void setAtkCnt(); // 0x4B0
    virtual float getJumpGravity(); // 0x4B4
    virtual float getDrawScale(); // 0x4B8
    virtual void speedUp(); // 0x4BC
    virtual int beginDance(); // 0x4C0
    virtual int getTurnSpeed(); // 0x4C4
    virtual int getFumiRecoverTime(); // 0x4C8
    virtual int createBlitz(); // 0x4CC
    virtual float createBlitz_sub() = 0; // 0x4D0
    virtual mVec3_c getMagicStickEffectOffset() const; // 0x4D4
    virtual void setKokoopaCc(); // 0x4D8
    virtual void setShellCc(); // 0x4DC
    virtual float getJumpDist() const; // 0x4E0
    virtual mVec3_c calcBlitzPos(); // 0x4E4
    virtual void blitzShoot(); // 0x4E8
    virtual void setBlitzTarget(); // 0x4EC
    virtual mVec3_c calcFacePos(); // 0x4F0
    virtual void calcCcData(); // 0x4F4
    virtual void calcWandCcData(); // 0x4F8
    virtual float getKokoopaOffFrm() const; // 0x4FC
    virtual float getShellOnFrm() const; // 0x500
    virtual float getKokoopaOnFrm() const; // 0x504
    virtual float getShellOffFrm() const; // 0x508
    virtual bool checkGetUp() const; // 0x50C
    virtual float getCreateBlitzFrm() const; // 0x510
    virtual float getShootFrm() const; // 0x514
    virtual mVec3_c getPressScale(); // 0x518
    virtual int getPressTime(); // 0x51C
    virtual int defaultDirAngle(); // 0x520
    virtual float getShellChangeEffectOffsetY() const; // 0x524
    virtual void jumpEffect(); // 0x528
    virtual void jumpRootEffect(); // 0x52C
    virtual void landonEffect(); // 0x530
    virtual void shellLandonEffect(); // 0x534
    virtual void hitFireLoopEffect(); // 0x538
    virtual void hitFireDamageEffect(); // 0x53C
    virtual void shellChangeEffect(); // 0x540
    virtual void shellBumMarEffect(); // 0x544
    virtual void shellAtkEffect(); // 0x548
    virtual void downFallEffect(); // 0x54C
    virtual void downLandOnEffect(float); // 0x550
    virtual void hitShellDamageEffect(); // 0x554
    virtual void ikakuEffect(); // 0x558
    virtual void jumpSE(); // 0x55C
    virtual void landonSE(); // 0x560
    virtual void shelllandonSE(); // 0x564
    virtual void shellinSE(); // 0x568
    virtual void shelloutSE(); // 0x56C
    virtual void shellatkSE(); // 0x570
    virtual void getupSE(); // 0x574
    virtual void blitzchargeSE(); // 0x578
    virtual void notice1Vo(); // 0x57C
    virtual void notice2Vo(); // 0x580
    virtual void wakeVo(); // 0x584
    virtual void escJumpVo(); // 0x588
    virtual void magicShotVo(); // 0x58C
    virtual void shellOutVo(); // 0x590
    virtual void deadVo(); // 0x594
    virtual void loseFirstVo(); // 0x598
    virtual void loseSecondVo(); // 0x59C

    // Demo States (Slots 358..372)
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoAwake); // 0x5A0
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoAwake_Wait); // 0x5AC
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoIkaku); // 0x5B8
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoIkaku_Wait); // 0x5C4
    STATE_VIRTUAL_FUNC_DECLARE(dEnTorideKokoopa_c, DemoEscape_St); // 0x5D0

    virtual void awakeSE(); // 0x5DC
    virtual void ikakuSE(); // 0x5E0

    void calcRootJntPos();
    void calcShellJntPos();
    bool isTorideBoss();
    bool checkDownJump();
    void changeShell();
    void changeKokoopa();
    int getTorideFunfareTime();
    float calcJumpRate();
    bool movelimitCheck(float);
    int calcDirAngle(short);
    static void wandCcCallback(dCc_c *self, dCc_c *other);
    bool isCreateBlitz() const;
    bool isShootBlitz() const;
    void blitzMove(dActor_c *blitz);
    u32 calcAttackTarget();
    void calcLookAngle();
    void shellWallEffect();
    void moveRevise();
    
    // Member variables (0x600..0xE70)
    nw4r::g3d::ResFile mResFile; // 0x600
    m3d::mdl_c mMdlKokoopa; // 0x604
    m3d::anmChr_c mAnmChrKokoopa; // 0x644
    m3d::anmMatClr_c mAnmMatClr; // 0x688
    nw4r::g3d::ResAnmTexPat mUnk6A8; // 0x6A8
    m3d::anmTexPat_c mAnmTexPat; // 0x6AC
    m3d::mdl_c mMdlShell; // 0x6D8
    m3d::anmChr_c mAnmChrShell; // 0x718
    struct ParamReady {
        const char *mAnmNames[1];
    };
    ParamReady *mpParamReady; // 0x750
    struct ParamJump {
        const char *mAnmNames[4];
        u32 mPad10;
        mVec2_c mJumpSpeed1;
        mVec2_c mBigJumpSpeed1;
        mVec2_c mJumpSpeed2;
        mVec2_c mBigJumpSpeed2;
    };
    ParamJump *mpParamJump; // 0x754
    struct ParamAttack {
        const char *mAnmNames[4];
        const char *mClrName;
    };
    ParamAttack *mpParamAttack; // 0x758
    struct ParamShell {
        const char *mAnmNames[6];
    };
    ParamShell *mpParamShell; // 0x75C
    struct ParamDemo {
        const char *mAnmNames[5];
    };
    ParamDemo *mpParamDemo; // 0x760
    u32 mUnk764; // 0x764
    s32 mUnk768; // 0x768
    u32 mPad76C; // 0x76C
    u32 mUnk770; // 0x770
    mVec3_c mBlitzPos; // 0x774..0x780
    u32 mUnk780; // 0x780
    mVec3_c mLookatPos; // 0x784..0x790
    s16 mUnk790; // 0x790
    s16 mUnk792; // 0x792
    s32 mUnk794; // 0x794
    s32 mAtkCnt; // 0x798
    dCc_c mCc; // 0x79C..0x840
    f32 mUnk840; // 0x840
    f32 mUnk844; // 0x844
    s32 mUnk848; // 0x848
    s32 mFireTimer; // 0x84C
    mEf::levelEffect_c mLevelEffect1; // 0x850
    mEf::levelEffect_c mLevelEffect2; // 0x978
    s32 mUnkAA0; // 0xAA0
    mVec3_c mUnkAA4; // 0xAA4
    mVec3_c mUnkAB0; // 0xAB0
    u32 mPadABC; // 0xABC
    s16 mUnkAC0; // 0xAC0
    u8 mPadAC2[2]; // 0xAC2
    f32 mUnkAC4; // 0xAC4
    s32 mUnkAC8; // 0xAC8
    f32 mUnkACC; // 0xACC
    s32 mRootJntIdx; // 0xAD0
    s32 mShellJntIdx; // 0xAD4
    mVec3_c mRootJntPos; // 0xAD8
    mVec3_c mShellJntPos; // 0xAE4
    u32 mUnkAF0; // 0xAF0
    mEf::levelEffect_c mLevelEffects[3]; // 0xAF4
    void *mVoiceParam; // 0xE6C
};
