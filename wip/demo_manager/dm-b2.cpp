// Batch 2/6: the goal-pole demo sequence.
// daPyDemoMng_c: executeGoalDemo_Pole, executeGoalDemo_PoleDown,
// executeGoalDemo_JumpCheck, executeGoalDemo_Jump, executeGoalDemo_Land,
// executeGoalDemo_KimeWait.
// See wip/demo_manager/DEMO-MANAGER-SIBMAP.md for the per-function proof
// trail this draft is based on.
//
// KNOWN BLOCKING GAPS (reported, not resolved here -- see the batch report):
//   - SndSceneMgr::startGoal(bool) is called from executeGoalDemo_Jump
//     (target symbol startGoal__11SndSceneMgrFb) but is NOT declared in the
//     real include/game/snd/snd_scene_manager.hpp.
//   - dScStage_c::ReplayEnd() is called from executeGoalDemo_Pole (target
//     symbol ReplayEnd__10dScStage_cFv) but is NOT declared in the real
//     include/game/bases/d_s_stage.hpp.
//   Both were verified locally against a *scratch* copy of those two headers
//   (never against the real include/ tree, which this batch does not touch)
//   so the rest of this file's logic could still be compile-tested.

#include <game/bases/d_a_player_demo_manager.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/mLib/m_vec.hpp>

// ---------------------------------------------------------------------
// Unnamed dAcPy_c fields this batch touches. Offsets are proven by this
// TU's own disassembly (see the per-field comment at each use site); no
// name or type beyond what the access itself implies is known. Left as
// raw casts per the brief: dAcPy_c/daPlBase_c ARE decompiled elsewhere,
// but these specific bytes are not yet named in the frozen headers, and
// this batch may not edit include/.
// ---------------------------------------------------------------------
static inline int &pole_order_ref(dAcPy_c *p) {
    // 0x430: read in executeGoalDemo_Pole (0x8005B978) as an index used to
    // scatter a player number into a 4-slot local array -- looks like a
    // per-player "pole slot" index.
    return *reinterpret_cast<int *>(reinterpret_cast<u8 *>(p) + 0x430);
}
static inline int &jump_order_ref(dAcPy_c *p) {
    // 0x434: written in executeGoalDemo_Jump (0x8005BEE0) with
    // (mGoalEntryCount - (m_18+1)); read nowhere else in this batch.
    return *reinterpret_cast<int *>(reinterpret_cast<u8 *>(p) + 0x434);
}
static inline u8 &field_38c_ref(dAcPy_c *p) {
    // 0x38c: read in executeGoalDemo_Pole (0x8005BBC4) right after a
    // checkGround() call, compared against 2 -- looks like a ground/contact
    // type byte, but is not dBc_c::mLayer (that's confirmed at 0x38f).
    return *reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(p) + 0x38c);
}
static inline float &field_438_ref(dAcPy_c *p) {
    // 0x438: written in executeGoalDemo_Pole (0x8005BC64), just past the
    // already-proven mPos (0xac) and STATUS_GOAL_POLE_READY_FOR_JUMP_OFF's
    // sibling field 0x430/0x434 above.
    return *reinterpret_cast<float *>(reinterpret_cast<u8 *>(p) + 0x438);
}
static inline int &field_1090_ref(dAcPy_c *p) {
    // 0x1090: read in executeGoalDemo_Pole (0x8005BBE4), compared to 0 and 3.
    return *reinterpret_cast<int *>(reinterpret_cast<u8 *>(p) + 0x1090);
}
static inline float &field_1030_ref(dAcPy_c *p) {
    // 0x1030: read in executeGoalDemo_Pole (0x8005BC68), a float multiplied
    // into the height accumulator.
    return *reinterpret_cast<float *>(reinterpret_cast<u8 *>(p) + 0x1030);
}

void daPyDemoMng_c::executeGoalDemo_PoleDown() {
    m_08 = 2;
    for (int i = 0; i < 4; i++) {
        int playerNo = mGoalDemoList[i];
        if (playerNo == -1) {
            continue;
        }
        dAcPy_c *ctrlPl = daPyMng_c::getCtrlPlayer(playerNo);
        if (ctrlPl != NULL) {
            ctrlPl->onStatus(daPlBase_c::STATUS_GOAL_POLE_CAN_SLIDE);
        }
    }
}

void daPyDemoMng_c::executeGoalDemo_JumpCheck() {
    if (mFlags & 2) {
        dAcPy_c *ctrlPl;
        for (int i = 0; i < 4; i++) {
            if (!daPyMng_c::checkPlayer(i)) {
                continue;
            }
            ctrlPl = daPyMng_c::getCtrlPlayer(i);
            if (ctrlPl == NULL) {
                continue;
            }
            if (!ctrlPl->isStatus(daPlBase_c::STATUS_GOAL_POLE_TOUCHED)) {
                continue;
            }
            if (!ctrlPl->isStatus(daPlBase_c::STATUS_GOAL_POLE_FINISHED_SLIDE_DOWN)) {
                return;
            }
        }
        m_54 = 0;
        m_08 = 3;
        m_18 = mGoalEntryCount - 1;
        m_0c = 10;
    }
}

void daPyDemoMng_c::executeGoalDemo_Jump() {
    // m_18 is declared u32 in the frozen header, but this comparison in the
    // target disassembly is a SIGNED cmpwi/bge pair, not the unsigned
    // cmplwi a real u32>=0 compile-time-true comparison would need (and a
    // true unsigned >=0 would just be optimized away, which is what
    // happened here before this cast was added). Flagged: contradicts the
    // frozen header's declared type for this field; worked around here with
    // a local signed reinterpretation instead of editing the header.
    if ((int)m_18 < 0) {
        m_08 = 4;
        return;
    }
    if (m_18 == 0) {
        if (m_54 == 0) {
            if (m_40) {
                SndSceneMgr::sInstance->startGoal(true);
            } else {
                SndSceneMgr::sInstance->startGoal(false);
            }
            m_54++;
        }
    }
    if (m_0c != 0) {
        return;
    }
    dAcPy_c *ctrlPl = daPyMng_c::getCtrlPlayer(mGoalDemoList[m_18]);
    int remain = mGoalEntryCount - (m_18 + 1);
    m_18--;
    if (ctrlPl == NULL) {
        return;
    }
    jump_order_ref(ctrlPl) = remain;
    ctrlPl->onStatus(daPlBase_c::STATUS_GOAL_POLE_READY_FOR_JUMP_OFF);
    m_0c = 10;
}

void daPyDemoMng_c::executeGoalDemo_Land() {
    dAcPy_c *ctrlPl;
    for (int i = 0; i < 4; i++) {
        if (!daPyMng_c::checkPlayer(i)) {
            continue;
        }
        ctrlPl = daPyMng_c::getCtrlPlayer(i);
        if (ctrlPl == NULL) {
            continue;
        }
        if (!ctrlPl->isStatus(daPlBase_c::STATUS_GOAL_POLE_TOUCHED)) {
            continue;
        }
        if (!ctrlPl->isStatus(daPlBase_c::STATUS_GOAL_POLE_TURN)) {
            return;
        }
    }
    u32 flags = mFlags;
    m_08 = 5;
    mFlags = flags | 8;
}

void daPyDemoMng_c::executeGoalDemo_KimeWait() {
    bool found = false;
    dAcPy_c *ctrlPl;
    for (int i = 0; i < 4; i++) {
        if (!daPyMng_c::checkPlayer(i)) {
            continue;
        }
        ctrlPl = daPyMng_c::getCtrlPlayer(i);
        if (ctrlPl == NULL) {
            continue;
        }
        if (!ctrlPl->isStatus(daPlBase_c::STATUS_GOAL_POLE_TOUCHED)) {
            continue;
        }
        if (!ctrlPl->isStatus(daPlBase_c::STATUS_6C)) {
            continue;
        }
        found = true;
        break;
    }
    if (found) {
        mFlags |= 0x10;
    } else {
        mFlags &= ~0x10;
    }
}

void daPyDemoMng_c::executeGoalDemo_Pole() {
    int order[4] = { -1, -1, -1, -1 };

    dAcPy_c *ctrlPl;
    for (int i = 0; i < 4; i++) {
        if (!daPyMng_c::checkPlayer(i)) {
            continue;
        }
        ctrlPl = daPyMng_c::getCtrlPlayer(i);
        if (ctrlPl == NULL) {
            continue;
        }

        if (mFlags & 4) {
            if (ctrlPl->isStatus(daPlBase_c::STATUS_GOAL_POLE_TOUCHED)) {
                if (!ctrlPl->isStatus(daPlBase_c::STATUS_GOAL_POLE_WAIT_BELOW_PLAYER)) {
                    return;
                }
            }
        } else {
            if (!ctrlPl->isStatus(daPlBase_c::STATUS_GOAL_POLE_WAIT_BELOW_PLAYER)) {
                return;
            }
        }

        if (ctrlPl->isStatus(daPlBase_c::STATUS_GOAL_POLE_WAIT_BELOW_PLAYER)) {
            order[pole_order_ref(ctrlPl)] = i;
        }
    }

    int sorted[4] = { -1, -1, -1, -1 };

    int playerNo;
    dAcPy_c *ctrlPl2;
    dAcPy_c *other;
    for (int i = 0; i < 4; i++) {
        playerNo = order[i];
        if (playerNo == -1) {
            continue;
        }
        ctrlPl2 = daPyMng_c::getCtrlPlayer(playerNo);
        if (ctrlPl2 == NULL) {
            continue;
        }

        if (sorted[0] == -1) {
            sorted[0] = playerNo;
            continue;
        }

        for (int j = 0; j < 4; j++) {
            if (sorted[j] == -1) {
                sorted[j] = playerNo;
                break;
            }
            other = daPyMng_c::getCtrlPlayer(sorted[j]);
            if (ctrlPl2->mPos.y <= other->mPos.y) {
                for (int k = 3; k > j; k--) {
                    sorted[k] = sorted[k - 1];
                }
                sorted[j] = playerNo;
                break;
            }
        }
    }

    mGoalEntryCount = 0;
    // Constants read directly from original/wiimj2d.dol's .sdata2, at the
    // addresses this TU's own disassembly references here (0x8042bcf8,
    // 0x8042bcfc, 0x8042bd00, 0x8042bd04, 0x8042bd08): 0.0f, 6.0f, 2.0f,
    // 4.0f, 0.7f. The text-only comparator cannot see a wrong constant here
    // (it canonicalises pool references), so these were checked as raw
    // bytes per the brief's verification standard, not guessed.
    float heightAccum = 0.0f;

    if (sorted[0] != -1) {
        dAcPy_c *ctrlPl = daPyMng_c::getCtrlPlayer(sorted[0]);
        if (ctrlPl != NULL) {
            ctrlPl->stopGoalOther();
            dBc_c::checkGround(&ctrlPl->mPos, &heightAccum, ctrlPl->mLayer, 1, -1);
            if (field_38c_ref(ctrlPl) == 2) {
                heightAccum -= 6.0f;
            } else if (field_1090_ref(ctrlPl) != 3 && field_1090_ref(ctrlPl) != 0) {
                heightAccum -= 2.0f;
            }
        }

        for (int k = 0; k < 4; k++) {
            int playerNo = sorted[k];
            mGoalDemoList[k] = playerNo;
            if (playerNo == -1) {
                continue;
            }
            mGoalEntryCount++;
            dAcPy_c *pl = daPyMng_c::getCtrlPlayer(playerNo);
            if (pl == NULL) {
                continue;
            }
            float accum = heightAccum;
            float cap = pl->mPos.y + 4.0f;
            if (accum > cap) {
                heightAccum = cap;
            }
            field_438_ref(pl) = heightAccum;
            heightAccum += 0.7f * field_1030_ref(pl);
        }
    }

    dScStage_c::ReplayEnd();
    daPyDemoMng_c::mspInstance->stopBgmGoalDemo();
    m_08 = 1;
    m_0c = 10;
    m_18 = 0;
    deleteNotGoalPlayer();
    setZoromeGoal();
}
