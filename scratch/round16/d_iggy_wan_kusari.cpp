#include <game/bases/d_iggy_wan_kusari.hpp>
#include <game/bases/d_actor.hpp>
#include <game/bases/d_bg_parameter.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/cLib/c_math.hpp>
#include <game/mLib/m_allocator_dummy_heap.hpp>
#include <game/mLib/m_mtx.hpp>

// --- static members ---------------------------------------------------------

s16 dIggyWanKusari_c::smc_ANGLE_DIST_RATE;
const float dIggyWanKusariPiece_c::smc_LENGTH = 6.0f;

// --- state framework --------------------------------------------------------

STATE_DEFINE(dIggyWanKusari_c, Ready);
STATE_DEFINE(dIggyWanKusari_c, Normal);
STATE_DEFINE(dIggyWanKusari_c, Tight);
STATE_DEFINE(dIggyWanKusari_c, Release);
STATE_DEFINE(dIggyWanKusari_c, Collapse);
STATE_DEFINE(dIggyWanKusari_c, Dead);

// --- dIggyWanKusari_c -------------------------------------------------------

void dIggyWanKusari_c::create(int param) {
    mPieceCount = param + 2;
    allocate();
    init();
    mStateMgr.changeState(StateID_Ready);
    mStateMgr.refreshState();
}

void dIggyWanKusari_c::allocate() {
    createFrmHeapToCurrent(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20, mHeap::OPT_NONE);
    make_kusari();
    createMdl();
    adjustFrmHeapRestoreCurrent();
}

void dIggyWanKusari_c::execute() {
    mStateMgr.executeState();
    calcMdl();
}

void dIggyWanKusari_c::calcMdl() {
    dIggyWanKusariPiece_c *p = mpHeadPiece;
    for (int i = 0; i < mPieceCount; i++) {
        p->calcMdl();
        p = p->mpNext;
    }
}

void dIggyWanKusari_c::draw() {
    dIggyWanKusariPiece_c *p = mpHeadPiece;
    for (int i = 0; i < mPieceCount; i++) {
        p->draw();
        p = p->mpNext;
    }
}

void dIggyWanKusari_c::remove() {
    if (mpHeap != mAllocatorDummyHeap_c::getInstance()) {
        dIggyWanKusariPiece_c *p = mpHeadPiece;
        for (int i = 0; i < mPieceCount; i++) {
            p->mSmdl.remove();
            p->mAnmTexSrt.remove();
            dIggyWanKusariPiece_c *next = p->mpNext;
            delete p;
            p = next;
        }
    }
}

void dIggyWanKusari_c::make_kusari() {
    mpHeadPiece = new dIggyWanKusariPiece_c(0);
    dIggyWanKusariPiece_c *prev = mpHeadPiece;
    for (int i = 1; i < mPieceCount; i++) {
        dIggyWanKusariPiece_c *p = new dIggyWanKusariPiece_c(i);
        prev->mpNext = p;
        p->mpPrev = prev;
        prev = p;
    }
}

void dIggyWanKusari_c::createMdl() {
    dIggyWanKusariPiece_c *p = mpHeadPiece;
    for (int i = 0; i < mPieceCount; i++) {
        p->createMdl(*this);
        p = p->mpNext;
    }
}

void dIggyWanKusari_c::init() {
    dIggyWanKusariPiece_c *p = mpHeadPiece;
    for (int i = 0; i < mPieceCount; i++) {
        p->mAng.y = mpBoss->mAngle.y;
        p = p->mpNext;
    }
}

float dIggyWanKusari_c::getLength() const {
    return (mPieceCount - 2) * dIggyWanKusariPiece_c::smc_LENGTH;
}

void dIggyWanKusari_c::setAlphaForKameckMagic(u8 alpha) {
    dIggyWanKusariPiece_c *p = mpHeadPiece;
    for (int i = 0; i < mPieceCount; i++) {
        p->mAlpha = alpha;
        p = p->mpNext;
    }
}

float dIggyWanKusari_c::calcTightRate() {
    // @unofficial: blocked. Reads boss (daEnIggy, not decompiled) fields at
    // 0x62c/0x630/0x634 (pos), 0x638/0x63c/0x640 (pos2) and the player position,
    // then clamps to [0,1]. Cannot be authored without the boss layout.
    return 1.0f;
}

void dIggyWanKusari_c::ready() {
    // @unofficial: blocked on boss layout (0x62c-0x640, 0x65c-0x664, 0x102).
}

void dIggyWanKusari_c::normal() {
    // @unofficial: blocked on boss layout (0x62c-0x640, 0x78c).
}

void dIggyWanKusari_c::tight() {
    // @unofficial: blocked on boss layout (0x62c-0x640, 0x65c-0x664).
}

void dIggyWanKusari_c::release() {
    // @unofficial: blocked on boss layout (0x62c-0x640, 0x65c-0x664).
}

// --- state bodies -----------------------------------------------------------

void dIggyWanKusari_c::initializeState_Ready() {}
void dIggyWanKusari_c::finalizeState_Ready() {}
void dIggyWanKusari_c::executeState_Ready() {
    // @unofficial: blocked. PSVECMag on boss pos2 (0x638-0x640); if > 0 calls
    // ready() then changeState(Normal). ready() itself is blocked.
    mVec3_c v = *(mVec3_c *)((u8 *)mpBoss + 0x638);
    if (PSVECMag(v) > 0.0f) {
        ready();
        mStateMgr.changeState(StateID_Normal);
    }
}

void dIggyWanKusari_c::initializeState_Normal() {}
void dIggyWanKusari_c::finalizeState_Normal() {}
void dIggyWanKusari_c::executeState_Normal() {
    dIggyWanKusariPiece_c *p = mpHeadPiece;
    for (int i = 0; i < mPieceCount; i++) {
        p->calcForDemo();
        p = p->mpNext;
    }
    normal();
}

void dIggyWanKusari_c::initializeState_Tight() {}
void dIggyWanKusari_c::finalizeState_Tight() {}
void dIggyWanKusari_c::executeState_Tight() {
    tight();
}

void dIggyWanKusari_c::initializeState_Release() {}
void dIggyWanKusari_c::finalizeState_Release() {}
void dIggyWanKusari_c::executeState_Release() {
    release();
}

void dIggyWanKusari_c::initializeState_Collapse() {
    // @unofficial: blocked. Compares boss pos2.x (0x638) vs pos.x (0x62c).
    bool dir = *(float *)((u8 *)mpBoss + 0x638) > *(float *)((u8 *)mpBoss + 0x62c);
    dIggyWanKusariPiece_c *p = mpHeadPiece;
    for (int i = 0; i < mPieceCount; i++) {
        p->setCollapseSpeed(dir);
        dir = !dir;
        p = p->mpNext;
    }
}

void dIggyWanKusari_c::finalizeState_Collapse() {}
void dIggyWanKusari_c::executeState_Collapse() {
    int remaining = mPieceCount;
    dIggyWanKusariPiece_c *p = mpHeadPiece;
    for (int i = 0; i < mPieceCount; i++) {
        if (p->collapseMove()) {
            remaining--;
        }
        p = p->mpNext;
    }
    if (remaining <= 0) {
        mStateMgr.changeState(StateID_Dead);
    }
}

void dIggyWanKusari_c::initializeState_Dead() {}
void dIggyWanKusari_c::finalizeState_Dead() {}
void dIggyWanKusari_c::executeState_Dead() {}

// --- dIggyWanKusariPiece_c --------------------------------------------------

void dIggyWanKusariPiece_c::createMdl(mHeapAllocator_c &alloc) {
    static const char *cs_mdl_name[2] = { "wanwan_chainA", "wanwan_chainB" };

    mpResFile = dResMng_c::m_instance->getRes("wanwan_boss_iggy", "g3d/wanwan_boss_iggy.brres");
    nw4r::g3d::ResMdl mdl = mpResFile.GetResMdl(cs_mdl_name[(mPieceNo + 1) & 1]);
    mSmdl.create(mdl, &alloc, 0x120, 1, nullptr);

    mpResAnmTexSrt = mpResFile.GetResAnmTexSrt("magic_chainAB");
    mAnmTexSrt.create(mdl, mpResAnmTexSrt, &alloc, nullptr, 1);
    mAnmTexSrt.setAnm(mSmdl, mpResAnmTexSrt, 0, m3d::FORWARD_LOOP);
    mSmdl.setAnm(mAnmTexSrt);
    dActor_c::setSoftLight_Enemy(mSmdl);
}

void dIggyWanKusariPiece_c::calcMdl() {
    if (!mDone) {
        mVec3_c pos = mPos;
        mAng3_c ang = mAng;
        dActor_c::changePosAngle(&pos, &ang, 1);

        mMtx_c mtx;
        PSMTXTrans(mtx, pos.x, pos.y, pos.z);
        mtx.YrotM(ang.y);
        mtx.ZrotM(ang.z);
        mtx.XrotM(ang.x);

        mSmdl.setLocalMtx(&mtx);
        nw4r::math::VEC3 scale(1.5f, 1.5f, 1.5f);
        mSmdl.setScale(scale);
    }
}

void dIggyWanKusariPiece_c::draw() {
    if (!mDone) {
        _GXColor color;
        d3d::getTevKColor(&color, &mSmdl, 0, GX_KCOLOR2);
        color.a = mAlpha;
        mSmdl.setTevKColor(0, GX_KCOLOR2, color, false);
        mSmdl.entry();
    }
}

void dIggyWanKusariPiece_c::calcForDemo() {
    mAnmTexSrt.play();
}

void dIggyWanKusariPiece_c::calcPosAngle(dActor_c *boss) {
    mVec3_c d = mAnchorB - mAnchorA;
    mAng.y = cM::atan2s(d.x, d.z);

    float hDist = nw4r::math::FSqrt(d.x * d.x + d.z * d.z);
    mAng.x = cM::atan2s(-d.y, hDist);

    mPos.x = 0.5f * (mAnchorA.x + mAnchorB.x);
    mPos.y = 0.5f * (mAnchorA.y + mAnchorB.y);
    mPos.z = 1440.0f;
    mAng.z = 0;
}

bool dIggyWanKusariPiece_c::collapseMove() {
    if (mDone) {
        return true;
    }

    mSpeed.y -= 0.15f;
    if (mSpeed.y < -4.5f) {
        mSpeed.y = -4.5f;
    }

    mPos.x += mSpeed.x;
    mPos.y += mSpeed.y;
    mPos.z += mSpeed.z;

    mAng.x += 0x800;
    mAng.z += 0x400;

    float groundY = dBgParameter_c::getInstance()->pos().y - dBgParameter_c::getInstance()->size().y;
    if (mPos.y < groundY - 32.0f) {
        mDone = true;
        return true;
    }
    return false;
}

void dIggyWanKusariPiece_c::setCollapseSpeed(int dir) {
    static const float cs_dir_prm[2] = { 1.0f, -1.0f };

    mSpeed.x = cs_dir_prm[dir] * (dGameCom::rndF(1.5f) + 0.25f);
    mSpeed.y = dGameCom::rndF(2.0f) + 3.0f;
    mSpeed.z = 0.0f;
}
