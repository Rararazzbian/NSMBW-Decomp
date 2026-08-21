#pragma once
#include <types.h>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>
#include <game/sLib/s_State.hpp>

class dActor_c;
class dIggyWanKusariPiece_c;

// @unofficial: these live in the shared d3d helpers, which are not declared in
// any current header. Signatures read from the mangled symbols:
//   getTevKColor__3d3dFP8_GXColorPQ23m3d6bmdl_ci14_GXTevKColorID
namespace d3d {
    void getTevKColor(_GXColor *out, m3d::bmdl_c *mdl, int matNo, _GXTevKColorID regID);
}

// @unofficial: dGameCom::rndF(float) exists at .text:0x800B2F10 but is absent
// from d_game_com.hpp. Mangled rndF__8dGameComFf.
namespace dGameCom {
    float rndF(float max);
}

/// @brief Iggy Koopa's swinging wrecking-ball chain manager.
/// @unofficial Reconstructed from the symbol map and disassembly.
class dIggyWanKusari_c : public mHeapAllocator_c {
public:
    dIggyWanKusari_c() : mStateMgr(*this, sStateID::null) {}

    void create(int param);
    void allocate();
    void execute();
    void calcMdl();
    void draw();
    void remove();
    void make_kusari();
    void createMdl();
    void init();
    float getLength() const;
    void setAlphaForKameckMagic(u8 alpha);
    float calcTightRate();

    void ready();
    void normal();
    void tight();
    void release();

    STATE_FUNC_DECLARE(dIggyWanKusari_c, Ready);
    STATE_FUNC_DECLARE(dIggyWanKusari_c, Normal);
    STATE_FUNC_DECLARE(dIggyWanKusari_c, Tight);
    STATE_FUNC_DECLARE(dIggyWanKusari_c, Release);
    STATE_FUNC_DECLARE(dIggyWanKusari_c, Collapse);
    STATE_FUNC_DECLARE(dIggyWanKusari_c, Dead);

    int mPieceCount;                              // +0x1C
    dIggyWanKusariPiece_c *mpHeadPiece;           // +0x20
    dActor_c *mpBoss;                             // +0x24
    sFStateMgr_c<dIggyWanKusari_c, sStateMethodUsr_FI_c> mStateMgr; // +0x28

    static s16 smc_ANGLE_DIST_RATE;               // .sbss
};

/// @brief A single link of Iggy's chain. @unofficial
class dIggyWanKusariPiece_c {
public:
    dIggyWanKusariPiece_c(int pieceNo) :
        mpResFile(nullptr),
        mpResAnmTexSrt(nullptr),
        mAlpha(0),
        mPieceNo(pieceNo),
        mDone(false),
        mpPrev(nullptr),
        mpNext(nullptr) {
        mPos.x = mPos.y = mPos.z = 0.0f;
        mSpeed.x = mSpeed.y = mSpeed.z = 0.0f;
        mAng.x = 0;
        mAng.y = 0;
        mAng.z = 0;
        mAnchorA.x = mAnchorA.y = mAnchorA.z = 0.0f;
        mAnchorB.x = mAnchorB.y = mAnchorB.z = 0.0f;
    }

    void createMdl(mHeapAllocator_c &alloc);
    void calcMdl();
    void draw();
    void calcForDemo();
    void calcPosAngle(dActor_c *boss);
    bool collapseMove();
    void setCollapseSpeed(int dir);

    nw4r::g3d::ResFile mpResFile;                 // +0x00
    m3d::smdl_c mSmdl;                            // +0x04
    nw4r::g3d::ResAnmTexSrt mpResAnmTexSrt;       // +0x10
    m3d::anmTexSrt_c mAnmTexSrt;                  // +0x14
    u8 mAlpha;                                    // +0x40
    u8 pad_41[0x44 - 0x41];
    mVec3_c mPos;                                 // +0x44
    mVec3_c mSpeed;                               // +0x50
    mAng3_c mAng;                                 // +0x5C
    u16 pad_62;                                   // +0x62
    u32 mPieceNo;                                 // +0x64
    mVec3_c mAnchorA;                             // +0x68
    mVec3_c mAnchorB;                             // +0x74
    BOOL mDone;                                   // +0x80 (4-byte: target uses lwz/stw)
    dIggyWanKusariPiece_c *mpPrev;                // +0x84
    dIggyWanKusariPiece_c *mpNext;                // +0x88

    static const float smc_LENGTH;                // .sdata2 = 6.0f
};
