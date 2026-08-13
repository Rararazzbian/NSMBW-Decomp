#include <game/bases/d_a_en_hatena_balloon.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_actor_manager.hpp>

// ---------------------------------------------------------------------------
// BATCH 5 -- background / terrain checks.
//
// All four functions verified byte-exact against original/wiimj2d.dol, in
// .text address order: pointBgCheck 0x801122F0 (0x28C), goalpole_check
// 0x80112580 (0x3C), floor_check 0x801125C0 (0x214), all_bgcheck 0x801127E0
// (0x164). s_someCheckData verified byte-exact at 0x802F4E20 (0x50).
//
// .rodata ownership: s_someCheckData is the FIRST .rodata object of the TU
// (0x802F4E20), ahead of l_hatenaballoon_cullinfo / l_cc_data (B1, 0x802F4E70)
// and l_create_diff (B6, 0x802F4EA8). Its definition must therefore sit at the
// very top of the merged d_a_en_hatena_balloon.cpp, above create().
//
// Header note for whoever merges this: the committed
// include/game/bases/d_a_en_hatena_balloon.hpp declares goalpole_check /
// floor_check / all_bgcheck as returning void. They return bool / u8 / u8 --
// all three return a value here and callers in other batches consume it.
// CodeWarrior does not mangle return types, so the symbol names are unchanged
// and this is a safe header edit. d_actor_manager.hpp also needs
// dActorMng_c::mGoalPoleX at +0x44 and floorEntryBufferCheck(mVec2_c *).
// ---------------------------------------------------------------------------

const daEnHatenaBalloon_c::checkData_s daEnHatenaBalloon_c::s_someCheckData[4] = {
    { 3.0f, 31.0f, -3.0f, 31.0f, 1 },
    { 3.0f, 5.0f, -3.0f, 5.0f, 2 },
    { -7.5f, 10.0f, -7.5f, 20.0f, 4 },
    { 7.5f, 10.0f, 7.5f, 20.0f, 8 },
};

u32 daEnHatenaBalloon_c::pointBgCheck(const mVec3_c &pos, unsigned long xr, unsigned long yr, unsigned long skip) {
    sBcSensorLine oldFoot = mSensorFoot;
    sBcSensorLine oldHead = mSensorHead;
    sBcSensorLine oldWall = mSensorWall;
    mVec3_c oldPos = mPos;
    float wallOut = -1.0f;
    bool ret = false;

    mSensorFoot.mLineA = (s8) -xr << 12;
    mSensorFoot.mLineB = (s8) xr << 12;
    mSensorFoot.mDistanceFromCenter = (s8) yr << 12;
    mSensorHead.mLineA = (s8) -xr << 12;
    mSensorHead.mLineB = (s8) xr << 12;
    mSensorHead.mDistanceFromCenter = (s8) -yr << 12;
    mSensorWall.mLineA = (s8) -yr << 12;
    mSensorWall.mLineB = (s8) yr << 12;
    mSensorWall.mDistanceFromCenter = (s8) xr << 12;
    mPos = pos;

    if (!(skip & 2)) {
        if (mBc.checkFoot() && mBc.getFootAttr() != 3) {
            ret = true;
        }
    }

    if (!(skip & 1)) {
        if (mBc.checkHead(0) && mBc.getHeadAttr() != 3) {
            ret = true;
        }
    }

    if (!(skip & 4)) {
        if (mBc.checkWall(&wallOut) && mBc.getWallAttr(1) != 3) {
            ret |= 4;
        }
    }

    if (!(skip & 8)) {
        wallOut = 1.0f;
        if (mBc.checkWall(&wallOut) && mBc.getWallAttr(0) != 3) {
            ret |= 8;
        }
    }

    mSensorFoot = oldFoot;
    mSensorHead = oldHead;
    mSensorWall = oldWall;
    mPos = oldPos;
    return ret;
}

bool daEnHatenaBalloon_c::goalpole_check() {
    float goalX = dActorMng_c::m_instance->mGoalPoleX;
    if (-8000.0f != goalX) {
        if (8.0f + mPos.x >= goalX) {
            return true;
        }
    }
    return false;
}

u8 daEnHatenaBalloon_c::floor_check() {
    u8 ret = 0;
    mVec2_c pt;

    pt.set(mPos.x - 7.0f, 3.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 1;
    }
    pt.set(7.0f + mPos.x, 3.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 2;
    }
    pt.set(mPos.x - 7.0f, 29.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 4;
    }
    pt.set(7.0f + mPos.x, 29.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 8;
    }
    pt.set(mPos.x - 7.0f, 5.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 0x10;
    }
    pt.set(mPos.x - 7.0f, 24.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 0x20;
    }
    pt.set(7.0f + mPos.x, 5.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 0x40;
    }
    pt.set(7.0f + mPos.x, 24.0f + mPos.y);
    if (dActorMng_c::m_instance->floorEntryBufferCheck(&pt)) {
        ret |= 0x80;
    }
    return ret;
}

// Two shapes in this function are load-bearing for register allocation. Both
// were established by compiling the alternatives and diffing, not by taste:
//
//  * The loop variables are DECLARED HERE and assigned below. Moving each
//    declaration down to its natural point of use leaves the code semantically
//    identical and byte-identical in length, but rotates the callee-saved GPRs
//    (r27/r28/r29/r31 swap roles) and the function stops matching. Do not
//    "clean this up".
//  * The probe point is built as a copy of mPos with the two offsets added in
//    place, NOT as mVec3_c pt(mPos.x + ofs[0], mPos.y + ofs[1], mPos.z). The
//    constructor form emits the same instructions in the same order but with a
//    4-way permutation of f0-f3. Fifteen other spellings were tried; this is
//    the only one that colours the FP temporaries the way the original does.
u8 daEnHatenaBalloon_c::all_bgcheck(u8 &floorFlags) {
    u32 flag;
    const checkData_s *row;
    const float *ofs;
    u8 ret;
    u32 i;
    u32 hit;
    u32 j;

    if (goalpole_check()) {
        floorFlags = 0xFF;
        return 0xF;
    }

    if (m_81f != -1) {
        dAcPy_c *player = daPyMng_c::getPlayer(m_81f);
        if (player != nullptr) {
            mBc.mLayer = player->mLayer;
        }
    }

    ret = 0;
    row = s_someCheckData;
    for (i = 0; i < 4; i++) {
        flag = row->mFlag;
        ofs = &row->mOffsetX;
        hit = 0;
        for (j = 0; j < 2; j++) {
            mVec3_c pt(mPos);
            pt.x += ofs[0];
            pt.y += ofs[1];
            if (!(ret & flag)) {
                hit |= pointBgCheck(pt, 1, 1, hit);
                if (hit != 0) {
                    ret |= (u8) row->mFlag;
                }
            }
            ofs += 2;
        }
        row++;
    }

    floorFlags = floor_check();

    float waterOut = 0.0f;
    dAcPy_c *player = daPyMng_c::getPlayer(m_81f);
    if (player != nullptr) {
        int type = dBc_c::checkWater(mPos.x, mPos.y, player->mLayer, &waterOut);
        if (type >= dBc_c::WATER_CHECK_YOGAN && type <= dBc_c::WATER_CHECK_POISON) {
            ret = 1;
        }
    }
    return ret;
}
