#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_demo_manager.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_actor.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_cd.hpp>
#include <game/bases/d_cd_data.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_info.hpp>
#include <game/framework/f_profile_name.hpp>
#include <game/mLib/m_vec.hpp>

// Cross-TU: daPyDemoMng_c's own unnamed helper (0x8005D280 in the target,
// where it has no symbol -- but it IS a real, already-landed global function
// in d_a_player_demo_manager.cpp). Forward-declared here rather than pulling
// a header, since it isn't a class member and has no header declaration.
void makeCourseInList(daPyDemoMng_c *pMgr);

// ---------------------------------------------------------------------
// Undeclared dScStage_c fields, read via daPyMng_c::createCourseInit and
// (per MAP.md) also daPyMng_c::getPlayerCreateAction. Offsets proven by this
// TU's own disassembly. dScStage_c is a frozen shared header this batch may
// not edit.
// ---------------------------------------------------------------------
static inline u8 &stageField_0x120e(dScStage_c *p) {
    // 0x120e: read in createCourseInit (0x8005EF9C, 0x8005F00C, 0x8005F098)
    // as the first (u8 file) argument to getPlayerSetPos/getFileP.
    return *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(p) + 0x120e);
}
static inline u8 &stageField_0x1211(dScStage_c *p) {
    // 0x1211: read alongside 0x120e as the second (u8 gotoNo) argument to
    // getPlayerSetPos.
    return *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(p) + 0x1211);
}

// ---------------------------------------------------------------------
// Undeclared dInfo_c fields, read only in createCourseInit's m_isCourseIn
// branch (0x8005F0C4-0x8005F108). dInfo_c's own header has pad11[0x712]
// spanning exactly this address range (0x3ec..0xafe from the object base),
// so these are real, currently-padded-out fields, not a foreign object.
// ---------------------------------------------------------------------
static inline int &infoField_0xaf4(dInfo_c *p) {
    return *reinterpret_cast<int *>(reinterpret_cast<u8 *>(p) + 0xaf4);
}
static inline mVec3_c &infoField_0x10(dInfo_c *p) {
    // 0x10..0x1c: 3 consecutive floats, copied wholesale into the local
    // spawn position when a mid-scroll course-in interrupts the camera.
    return *reinterpret_cast<mVec3_c *>(reinterpret_cast<u8 *>(p) + 0x10);
}
static inline int &infoField_0x1c(dInfo_c *p) {
    return *reinterpret_cast<int *>(reinterpret_cast<u8 *>(p) + 0x1c);
}

bool daPyMng_c::create(int plrNo, mVec3_c *pos, int type, u8 flag) {
    if (mPlayerEntry[plrNo] != 0) {
        dActor_c::construct(fProfile::PLAYER, (plrNo & 0xf) | ((type & 0xff) << 16) | ((flag & 1) << 24),
                             pos, nullptr, 0);
        return true;
    }
    return false;
}

// .sdata2 literal -- indexed by `flag`, NOT hand-declared as a named object
// per SHARED-BRIEF (0x8042BD70, {0x19, 0x1a}). File scope (not a function-
// local static) is deliberate: a function-local `static const` here mangles
// as `@LOCAL@fn_8005f4d0...@scBaseID` (proven -- see BATCH2.md), which does
// NOT match the target's anonymous `lbl_8042BD70` pool form.
static const int scBaseID[2] = {0x19, 0x1a};

bool daPyMng_c::fn_8005f4d0(mVec3_c *pos, int mode, int flag) {
    for (int i = 0; i < 4; i++) {
        if (getPlayer(i) == nullptr) {
            fn_8005f570((PLAYER_POWERUP_e)mode, i);
            create(i, pos, scBaseID[flag], 0);
            return true;
        }
    }
    return false;
}

void daPyMng_c::createCourseInit() {
    dScStage_c *stage = dScStage_c::getInstance();
    u8 action = getPlayerCreateAction();
    mVec3_c pos;

    // Written as three nested single-value checks, not `action != 0 && != 1
    // && != 0x17`: the flattened form lets MWCC fold the 0/1 pair into one
    // `cmplwi/ble` range test, but the target emits three separate `beq`s
    // (see BATCH2.md) -- this shape is the closest source found that keeps
    // them separate, though it did not fully suppress the fold either.
    if (action != 0) {
        if (action != 1) {
            if (action != 0x17) {
                makeCourseInList(daPyDemoMng_c::mspInstance);
                pos = getPlayerSetPos(stageField_0x120e(stage), stageField_0x1211(stage));
                u8 flag;
                if (pos.x <= dGameCom::getDispCenterX()) {
                    flag = 0;
                } else {
                    flag = 1;
                }
                for (int i = 0; i < 4; i++) {
                    create(i, &pos, action, flag);
                }
                return;
            }
        }
    }

    daPyDemoMng_c::mspInstance->init();
    decideCtrlPlrNo();

    if (dScStage_c::m_isStaffCredit) {
        static const float scOfsX[4] = {-184.0f, 200.0f, -208.0f, 224.0f};
        static const float scOfsY[4] = {-48.0f, -48.0f, 0.0f, 0.0f};
        pos = getPlayerSetPos(stageField_0x120e(stage), stageField_0x1211(stage));
        pos.x = 504.0f;
        for (int i = 0; i < 4; i++) {
            u8 flag = (scOfsX[i] >= 0.0f) ? 1 : 0;
            mVec3_c p2;
            p2.x = pos.x + scOfsX[i];
            p2.y = pos.y + scOfsY[i];
            p2.z = pos.z;
            create(i, &p2, action, flag);
        }
        return;
    }

    pos = getPlayerSetPos(stageField_0x120e(stage), stageField_0x1211(stage));
    dCdFile_c *file = dCd_c::getFileP(stageField_0x120e(stage));
    u8 scrollDir = (u8)file->mpScrollData->mID;

    if (dScStage_c::m_isCourseIn) {
        dInfo_c *info = dInfo_c::getInstance();
        if (infoField_0xaf4(info) >= 0) {
            pos = infoField_0x10(info);
            scrollDir = infoField_0x1c(info) & 1;
        }
    }

    int order[4] = {-1, -1, -1, -1};
    if (dScStage_c::m_gameMode == 2) {
        order[0] = 0;
        order[1] = 1;
        order[2] = 2;
        order[3] = 3;
    } else {
        float weight[4];
        for (int i = 0; i < 4; i++) {
            weight[i] = 0.1f + dGameCom::rnd();
        }
        int count = 0;
        for (int i = 0; i < 4; i++) {
            int j = 0;
            for (; j < count; j++) {
                if (weight[i] < weight[j]) {
                    break;
                }
            }
            for (int k = count; k > j; k--) {
                order[k] = order[k - 1];
            }
            if (j < 4) {
                order[j] = i;
            }
            if (count < 4) {
                count++;
            }
        }
    }

    int noBalloonCount = 0;
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i] != 0 && !isCreateBalloon(i)) {
            noBalloonCount++;
        }
    }

    float xStep = 12.0f * (float)(noBalloonCount - 1);
    if (scrollDir != 0) {
        pos.x -= xStep;
    } else {
        pos.x += xStep;
    }

    for (int i = 0; i < 4; i++) {
        int plrNo = order[i];
        if (plrNo == -1) {
            continue;
        }
        if (!create(plrNo, &pos, action, scrollDir)) {
            continue;
        }
        if (isCreateBalloon(plrNo)) {
            continue;
        }
        if (scrollDir == 0) {
            pos.x += 24.0f;
        } else {
            pos.x -= 24.0f;
        }
    }
}

bool daPyMng_c::fn_8005f570(PLAYER_POWERUP_e mode, int i) {
    u8 idx = i;
    int type = mPlayerType[i];
    u8 mask = 1 << idx;
    mActPlayerInfo |= mask;
    mPlayerEntry[i] = 1;
    mCreateItem[type] = 8;
    mKinopioMode = mode;
}
