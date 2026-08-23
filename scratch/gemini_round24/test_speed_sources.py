import sys, os, re, subprocess
sys.path.append('.')
from scratch.gemini_round24.test_jump_sweep import test_variant

# Test 1: speed assigned component-wise in branches
c1 = """    const char *anmName = mpParamJump->mAnmNames[1];
    if (anmName != nullptr) {
        nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(anmName);
        mAnmChrKokoopa.setAnm(mMdlKokoopa, anm, (m3d::playMode_e)1);
        mMdlKokoopa.setAnm(mAnmChrKokoopa, 0.0f);
        mAnmChrKokoopa.setRate(1.0f);
    }
    int flag = 1;
    if (mpBossLife->isNonDamage() == 0 && mpBossLife->isOneDamage() == 0) {
        flag = 0;
    }
    mVec2_c speed;
    if (flag != 0) {
        speed.x = mpParamJump->mJumpSpeed1.x;
        speed.y = mpParamJump->mJumpSpeed1.y;
    } else {
        speed.x = mpParamJump->mJumpSpeed2.x;
        speed.y = mpParamJump->mJumpSpeed2.y;
    }
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;
    jumpEffect();
    jumpSE();"""

# Test 2: speed initialized with ternary
c2 = """    const char *anmName = mpParamJump->mAnmNames[1];
    if (anmName != nullptr) {
        nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(anmName);
        mAnmChrKokoopa.setAnm(mMdlKokoopa, anm, (m3d::playMode_e)1);
        mMdlKokoopa.setAnm(mAnmChrKokoopa, 0.0f);
        mAnmChrKokoopa.setRate(1.0f);
    }
    int flag = 1;
    if (mpBossLife->isNonDamage() == 0 && mpBossLife->isOneDamage() == 0) {
        flag = 0;
    }
    mVec2_c speed = (flag != 0) ? mpParamJump->mJumpSpeed1 : mpParamJump->mJumpSpeed2;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;
    jumpEffect();
    jumpSE();"""

# Test 3: pointer / reference to jump speed
c3 = """    const char *anmName = mpParamJump->mAnmNames[1];
    if (anmName != nullptr) {
        nw4r::g3d::ResAnmChr anm = mResFile.GetResAnmChr(anmName);
        mAnmChrKokoopa.setAnm(mMdlKokoopa, anm, (m3d::playMode_e)1);
        mMdlKokoopa.setAnm(mAnmChrKokoopa, 0.0f);
        mAnmChrKokoopa.setRate(1.0f);
    }
    int flag = 1;
    if (mpBossLife->isNonDamage() == 0 && mpBossLife->isOneDamage() == 0) {
        flag = 0;
    }
    const mVec2_c &speed = (flag != 0) ? mpParamJump->mJumpSpeed1 : mpParamJump->mJumpSpeed2;
    float rate = calcJumpRate();
    float muki = (float)l_EnMuki[mDirection];
    mSpeed.y = speed.y;
    float sx = speed.x;
    mSpeed.x = (muki * rate) * sx;
    jumpEffect();
    jumpSE();"""

test_variant("C1: component-wise in branches", c1)
test_variant("C2: ternary copy", c2)
test_variant("C3: reference ternary", c3)
