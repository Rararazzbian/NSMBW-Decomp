#pragma once
#include <game/bases/d_enemy_boss.hpp>
#include <game/bases/d_cc.hpp>
#include <game/mLib/m_3d.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/sLib/s_State.hpp>
#include <cstddef>


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
    virtual void lockonTurn();
    virtual void calcKokoopaMdl();
    virtual void calcShellMdl();
    virtual void drawKokoopa();
    virtual void drawShell();
    virtual void setBeginMoveState();
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
    virtual float getJumpDist() const;
    virtual mVec3_c calcBlitzPos();
    virtual void blitzShoot();
    virtual void setBlitzTarget();
    virtual mVec3_c calcFacePos();
    virtual void calcCcData();
    virtual void calcWandCcData();
    virtual float getKokoopaOffFrm() const;
    virtual float getShellOnFrm() const;
    virtual float getKokoopaOnFrm() const;
    virtual float getShellOffFrm() const;
    virtual bool checkGetUp() const;
    virtual float getCreateBlitzFrm() const;
    virtual float getShootFrm() const;
    virtual mVec3_c getPressScale();
    virtual int getPressTime();
    virtual int defaultDirAngle();
    virtual float getShellChangeEffectOffsetY() const;
    virtual void jumpEffect();
    virtual void jumpRootEffect();
    virtual void landonEffect();
    virtual void shellLandonEffect();
    virtual void hitFireLoopEffect();
    virtual void hitFireDamageEffect();
    virtual void shellChangeEffect();
    virtual void shellBumMarEffect();
    virtual void shellAtkEffect();
    virtual void downFallEffect();
    virtual void downLandOnEffect(float);
    virtual void hitShellDamageEffect();
    virtual void ikakuEffect();
    virtual void jumpSE();
    virtual void landonSE();
    virtual void shelllandonSE();
    virtual void shellinSE();
    virtual void shelloutSE();
    virtual void shellatkSE();
    virtual void getupSE();
    virtual void blitzchargeSE();
    virtual void notice1Vo();
    virtual void notice2Vo();
    virtual void wakeVo();
    virtual void escJumpVo();
    virtual void magicShotVo();
    virtual void shellOutVo();
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
    void changeShell();
    void changeKokoopa();
    int getTorideFunfareTime();
    
    // Members (Offset 0x600 to 0xE70, size 0x870, total sizeof == 0xE70)
        u32 mUnk600;
    m3d::mdl_c mMdlKokoopa;
    m3d::anmChr_c mAnmChrKokoopa;
    m3d::anmMatClr_c mAnmMatClr;
    u32 mUnk6A8;
    m3d::anmTexPat_c mAnmTexPat;
    m3d::mdl_c mMdlShell;
    m3d::anmChr_c mAnmChrShell;
    u32 mUnk750;
    u32 mUnk754;
    u32 mUnk758;
    u32 mUnk75C;
    u32 mUnk760;
    u32 mUnk764;
    u32 mUnk768;
    u32 mPad76C;
    u32 mUnk770;
    mVec3_c mBlitzPos;
    u32 mUnk780;
    mVec3_c mLookatPos;
    s16 mUnk790;
    s16 mUnk792;
    u32 mUnk794;
    u32 mAtkCnt;
    dCc_c mCc;
    u8 mPad840[0x10];
    mEf::levelEffect_c mLevelEffect1;
    mEf::levelEffect_c mLevelEffect2;
        u32 mUnkAA0;
    mVec3_c mUnkAA4;
    mVec3_c mUnkAB0;
    u32 mPadABC;
    s16 mUnkAC0;
    u8 mPadAC2[2];
    f32 mUnkAC4;
    s32 mUnk848;
    f32 mScaleSpeed;
    s32 mRootJntIdx;
    s32 mShellJntIdx;
    mVec3_c mRootJntPos;
    mVec3_c mShellJntPos;
    u32 mUnkAF0;
    mEf::levelEffect_c mLevelEffects[3];
    void *mVoiceParam;
};

char sizeof_dEnTorideKokoopa_c[sizeof(dEnTorideKokoopa_c) == 0xE70 ? 1 : -1];
