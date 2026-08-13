#include <game/bases/d_a_en_hatena_balloon.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_actor_manager.hpp>

// ---------------------------------------------------------------------------
// BATCH 5 -- background / terrain checks.
//
// .rodata ownership: s_someCheckData is the FIRST .rodata object of the TU
// (0x802F4E20), ahead of l_hatenaballoon_cullinfo / l_cc_data (B1, 0x802F4E70)
// and l_create_diff (B6, 0x802F4EA8). Its definition must therefore sit at the
// very top of the merged d_a_en_hatena_balloon.cpp, above create().
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

u8 daEnHatenaBalloon_c::all_bgcheck(u8 &floorFlags) {
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

    u8 ret = 0;
    const checkData_s *row = s_someCheckData;
    for (u32 i = 0; i < 4; i++) {
        u32 flag = row->mFlag;
        const float *ofs = &row->mOffsetX;
        u32 hit = 0;
        for (u32 j = 0; j < 2; j++) {
            mVec3_c pt(mPos.x + ofs[0], mPos.y + ofs[1], mPos.z);
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
