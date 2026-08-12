#include <game/bases/d_a_en_bigpile.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_quake.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/mLib/m_effect.hpp>
#include <constants/sound_list.h>

static const s16 l_step_target_normal[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    16, 17, 19, 21, 25, 29, 34, 41, 48, 55, 64, 74, 84, 94, 104, 114,
    124, 134, 144, 154, 164, 174, 184, 194, 204, 214, 224, 234, 244, 254, 256, 256
};

static const s16 l_step_target_quick[] = {
    8, 15, 23, 32, 40, 49, 58, 67, 77, 86, 96, 107, 117, 128,
    139, 150, 162, 173, 185, 198, 210, 223, 236, 249, 256, 256, 256
};

static const s16 l_step_target_slow[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 18, 19, 21, 22,
    24, 26, 28, 30, 33, 36, 39, 42, 45, 48, 51, 54, 57, 60, 63, 66,
    69, 72, 75, 78, 81, 84, 87, 90, 93, 96, 99, 102, 105, 108, 111, 114,
    117, 120, 123, 126, 129, 132, 135, 138, 141, 144, 147, 150, 153, 156, 159, 162,
    165, 168, 171, 174, 177, 180, 183, 186, 189, 192, 195, 198, 201, 204, 207, 210,
    213, 216, 219, 222, 225, 228, 231, 234, 237, 240, 243, 246, 249, 252, 255, 256
};

BigPileMng_c daEnBigPile_c::m_manager[3];

/// @brief Scales a raw cycle offset by the pile's height in tiles. @unofficial
static inline void adjustOffsetSub(float &offset, float rate) {
    offset = offset * rate / 16.0f;
}
bool BigPileMng_c::entry(daEnBigPile_c *pile) {
    if (mCount > MAX_PILE_COUNT) {
        return false;
    }

    if (mCount == 0) {
        mStartFrame = dScStage_c::m_exeFrame;
    }

    for (int i = 0; i < MAX_PILE_COUNT; i++) {
        if (mpPiles[i] == nullptr) {
            mpPiles[i] = pile;
            mCount++;
            return true;
        }
    }

    return false;
}

void BigPileMng_c::remove(daEnBigPile_c *pile) {
    for (int i = 0; i < MAX_PILE_COUNT; i++) {
        if (mpPiles[i] == pile) {
            mpPiles[i] = nullptr;
            mCount--;
            if (mCount <= 0) {
                mIsMoving = false;
            }
        }
    }
}

void BigPileMng_c::wait() {
    for (int i = 0; i < MAX_PILE_COUNT; i++) {
        if (mpPiles[i] != nullptr && mpPiles[i]->wait()) {
            mStartFrame = dScStage_c::m_exeFrame;
            mIsMoving = true;
        }
    }
}

bool daEnBigPile_c::wait() {
    return false;
}

void BigPileMng_c::move() {
    for (int i = 0; i < MAX_PILE_COUNT; i++) {
        if (mpPiles[i] != nullptr) {
            mpPiles[i]->move();
        }
    }
}

int daEnBigPile_c::create() {
    mType = ACTOR_PARAM(Type);

    if (!m_manager[mType].entry(this)) {
        deleteRequest();
        return NOT_READY;
    }

    createMdl();

    mScale.x = 1.0f;
    mScale.y = 1.0f;
    mScale.z = 1.0f;

    mPos.z = -2500.0f;
    mPos.z += getAllPileNum();

    mStartPos.set(mPos.x, mPos.y);

    mEndWait = ACTOR_PARAM(EndWait);
    mStartWait = ACTOR_PARAM(StartWait);

    mpCcs[0] = &mCc;
    mpCcs[1] = &mCc2;

    initMove();
    initCnt();
    initCc();
    initCullInfo();
    initWaterSt();

    mCc.mAmiLine = 3;
    mCc2.mAmiLine = 3;

    return SUCCEEDED;
}

void daEnBigPile_c::initCc() {}

void daEnBigPile_c::initCullInfo() {}

void daEnBigPile_c::initWaterSt() {}

void daEnBigPile_c::createMdl() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    mResFile = dResMng_c::m_instance->getRes("daikonbou", "g3d/daikonbou.brres");
    nw4r::g3d::ResMdl mdl = mResFile.GetResMdl("daikonbou");
    mModel.create(mdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC | nw4r::g3d::ScnMdl::ANM_TEXSRT | nw4r::g3d::ScnMdl::ANM_TEXPAT);
    dActor_c::setSoftLight_MapObj(mModel);

    mResAnmTexPat = mResFile.GetResAnmTexPat("daikonbou");
    mAnmTexPat.create(mdl, mResAnmTexPat, &mAllocator);
    mModel.setAnm(mAnmTexPat, 1.0f);
    mAnmTexPat.setAnm(mModel, mResAnmTexPat, 0, m3d::FORWARD_ONCE);

    if (ACTOR_PARAM(TexPattern) == 1) {
        mAnmTexPat.setFrame(1.0f, 0);
    } else {
        mAnmTexPat.setFrame(0.0f, 0);
    }

    mResAnmTexSrt = mResFile.GetResAnmTexSrt("daikonbou");
    mAnmTexSrt.create(mdl, mResAnmTexSrt, &mAllocator);
    mAnmTexSrt.setPlayMode(m3d::FORWARD_LOOP, 0);
    mModel.setAnm(mAnmTexSrt, 1.0f);
    mAnmTexSrt.setAnm(mModel, mResAnmTexSrt, 0, m3d::FORWARD_ONCE);
    mAnmTexSrt.setFrame(0.0f, 0);

    mAllocator.adjustFrmHeap();
}

int daEnBigPile_c::execute() {
    moveProc();
    calcMdl();
    ActorScrOutCheck(0);
    return SUCCEEDED;
}

int daEnBigPile_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

void daEnBigPile_c::deleteReady() {}

int daEnBigPile_c::doDelete() {
    mCc.release();
    mCc2.release();
    m_manager[mType].remove(this);
    return SUCCEEDED;
}

void daEnBigPile_c::calcMdl() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    calcDrawPosAngle(pos, angle);
    changePosAngle(&pos, &angle, 1);

    mMatrix.trans(pos.x, pos.y, pos.z);
    mMatrix.ZrotM(angle.z);
    mMatrix.YrotM(angle.y);
    mMatrix.XrotM(angle.x);

    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

void daEnBigPile_c::calcDrawPosAngle(mVec3_c &pos, mAng3_c &angle) {}

bool daEnBigPile_c::PlDamageCheck(dCc_c *self, dCc_c *other) {
    return false;
}

void daEnBigPile_c::Normal_VsPlHitCheck(dCc_c *self, dCc_c *other) {
    setDamage(other->mpOwner);
}

bool daEnBigPile_c::EtcDamageCheck(dCc_c *self, dCc_c *other) {
    if (other->mCcData.mAttack == CC_ATTACK_FIREBALL && hitCallback_Fire(self, other)) {
        return true;
    }
    if (other->mCcData.mAttack == CC_ATTACK_ICEBALL && hitCallback_Ice(self, other)) {
        return true;
    }
    return false;
}

bool daEnBigPile_c::hitCallback_Fire(dCc_c *self, dCc_c *other) {
    fireballInvalid(self, other);
    return false;
}

bool daEnBigPile_c::hitCallback_Ice(dCc_c *self, dCc_c *other) {
    iceballInvalid(self, other);
    return false;
}

void daEnBigPile_c::initMove() {
    static const u32 cs_go_frame[3] = { 0x30, 0x1B, 0x70 };
    static const u32 cs_ret_frame[3] = { 0x6E, 0x1B, 0xC8 };
    static float (daEnBigPile_c::*const cs_go_func[3])(ulong) = {
        &daEnBigPile_c::moveGo_Normal, &daEnBigPile_c::moveGo_Quick, &daEnBigPile_c::moveGo_Slow
    };
    static float (daEnBigPile_c::*const cs_ret_func[3])(ulong) = {
        &daEnBigPile_c::moveRet_Normal, &daEnBigPile_c::moveRet_Quick, &daEnBigPile_c::moveRet_Slow
    };

    mGoFrame = cs_go_frame[mType];
    mRetFrame = cs_ret_frame[mType];
    mGoFunc = cs_go_func[mType];
    mRetFunc = cs_ret_func[mType];
}

int daEnBigPile_c::getAllPileNum() {
    int num = 0;
    for (int i = 0; i < 3; i++) {
        num += m_manager[i].mCount;
    }
    return num;
}

void daEnBigPile_c::initCnt() {
    if (ACTOR_PARAM(Mode) == 2) {
        mCounter = mStartWait + mGoFrame;
    } else if (ACTOR_PARAM(StartAtZero) == 1) {
        mCounter = 0;
        mFrameOffset = 0;
    } else {
        mCounter = mStartWait;
        mFrameOffset = mStartWait;
    }
}

void daEnBigPile_c::moveProc() {
    static u32 s_saveFrame = -1;

    u32 frame = dScStage_c::m_exeFrame;
    if (frame != s_saveFrame) {
        BigPileMng_c *mng = m_manager;
        for (int i = 0; i < 3; i++, mng++) {
            if (mng->mCount > 0) {
                if (!mng->mIsMoving) {
                    mng->wait();
                } else {
                    mng->move();
                }
            }
        }
        s_saveFrame = frame;
    }
}

int daEnBigPile_c::move() {
    u32 frame;

    if (ACTOR_PARAM(Mode) == 0) {
        u32 total = mEndWait + mStartWait + mGoFrame + mRetFrame;
        BigPileMng_c *mng = &m_manager[mType];
        frame = mFrameOffset + dScStage_c::m_exeFrame - mng->mStartFrame;
        frame %= total;
    } else {
        u32 total = mEndWait + mStartWait + mGoFrame + mRetFrame;
        total = (mCounter + 1) % total;
        frame = total;
        mCounter = frame;
    }

    if (frame == mStartWait + mGoFrame) {
        callQuake();

        if (ACTOR_PARAM(CrashEffect)) {
            mVec3_c pos(mPos.x, mPos.y, 5500.0f);
            mEf::createEffect("Wm_ob_cmnspark", 0, &pos, nullptr, nullptr);
        }

        if (!ACTOR_PARAM(NoSound)) {
            dAudio::g_pSndObjMap->startSound(SE_OBJ_DAIKONBOU_LAND, mPos, 0);
        }
    }

    float offset;
    ulong procFrame = 0;
    int proc = chkProcFrame(procFrame, frame);

    switch (proc) {
        case PROC_START_WAIT:
            offset = 0.0f;
            break;
        case PROC_GO:
            offset = (this->*mGoFunc)(procFrame);
            break;
        case PROC_END_WAIT:
            offset = 256.0f;
            break;
        case PROC_RETURN:
            offset = (this->*mRetFunc)(procFrame);
            break;
    }

    adjustOffset(offset);
    calcPos(offset);

    float dist = std::fabs(getMoveDist());
    s16 roll = 512.0f * dist;
    if (roll > 0x800) {
        roll = 0x800;
    }

    if (proc == PROC_GO) {
        mAngle.y += roll;
    } else if (proc == PROC_RETURN) {
        mAngle.y -= roll;
    }

    afterMove();

    return SUCCEEDED;
}

void daEnBigPile_c::calcPos(float offset) {}

float daEnBigPile_c::getMoveDist() {
    return 0.0f;
}

void daEnBigPile_c::afterMove() {}

void daEnBigPile_c::adjustOffset(float &offset) {
    switch (ACTOR_PARAM(Size)) {
        case 1:
            adjustOffsetSub(offset, 7.0f);
            break;
        case 2:
            adjustOffsetSub(offset, 14.0f);
            break;
        case 3:
            adjustOffsetSub(offset, 10.0f);
            break;
    }
}

int daEnBigPile_c::chkProcFrame(ulong &frameOut, ulong frame) {
    ulong cs_frame[5];
    cs_frame[0] = 0;
    cs_frame[1] = mStartWait;
    cs_frame[2] = cs_frame[1] + mGoFrame;
    cs_frame[3] = cs_frame[2] + mEndWait;
    cs_frame[4] = cs_frame[3] + mRetFrame;

    for (int i = 0; i < 4; i++) {
        if (frame >= cs_frame[i] && frame < cs_frame[i + 1]) {
            frameOut = frame - cs_frame[i];
            return i;
        }
    }

    return -1;
}

void daEnBigPile_c::callQuake() {
    dQuake_c::m_instance->startShockAll(dQuake_c::TYPE_0, 1, 0, false);
}

float daEnBigPile_c::moveGo_Normal(ulong frame) {
    return l_step_target_normal[frame];
}

float daEnBigPile_c::moveGo_Quick(ulong frame) {
    return l_step_target_quick[frame];
}

float daEnBigPile_c::moveGo_Slow(ulong frame) {
    return l_step_target_slow[frame];
}

float daEnBigPile_c::moveRet_Normal(ulong frame) {
    float rate = 256.0f / mRetFrame;
    return 256.0f - rate * frame;
}

float daEnBigPile_c::moveRet_Quick(ulong frame) {
    return 256.0f - l_step_target_quick[frame];
}

float daEnBigPile_c::moveRet_Slow(ulong frame) {
    float rate = 256.0f / mRetFrame;
    return 256.0f - rate * frame;
}

