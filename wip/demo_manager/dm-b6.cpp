// Batch 6 of 6 for dol/bases/d_a_player_demo_manager.cpp (daPyDemoMng_c).
// Covers: fn_8005D280 (invented name below), setEnemyStageClearDemo(int),
// __sinit_\d_a_player_demo_manager_cpp, __arraydtor$72504.
//
// PROVEN byte-and-symbol-exact in isolation via tools/auto_decomp/harness.py
// (compile_draft + dtk disasm + diff_fn, extracted BY ADDRESS/size, raw words
// AND callee/pool symbol names checked, negative control fired) for all 4
// items except one non-reproducible compiler-session counter -- see the
// per-function notes below and the batch report.
//
// NOT part of the deliverable, but required to reproduce this locally: a
// scratch copy of include/game/bases/d_a_player_manager.hpp with
// `static int mCourseInList[4];` added to daPyMng_c (see note on
// makeCourseInList below) -- the real include/ header was NOT touched.

#include <game/bases/d_a_player_demo_manager.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_game_com.hpp>
// Pulls in dWmLib::sc_ForceList[] and dWmLib::c_StartPointKinokoHouseID (both
// file-scope statics declared IN THIS HEADER, d_wm_lib.hpp:84-88). Their
// presence is what makes the compiler emit __sinit_/__arraydtor$ below --
// confirmed identical, symbol-for-symbol, to five other TUs that also
// include this header (DEMO-MANAGER-SIBMAP.md Finding 3): d_md_actor.cpp,
// d_wm_actor.cpp, d_wm_demo_actor.cpp, d_wm_enemy.cpp, d_wm_obj_actor.cpp.
#include <game/bases/d_wm_lib.hpp>

/// @unofficial Unnamed in the symbol map (fn_8005D280, 0x8005D280, 1064 B --
/// the 2nd-largest function in the unit). Invented name/signature; grepped
/// the ENTIRE unit's .text (all 51 functions, not just this batch) for a
/// caller and found none, so this is NOT declared `static` -- consistent
/// with its "global" (non-local) scope entry in wiimj2d_symbols.txt. Reads
/// `this` via an explicit `daPyDemoMng_c*` parameter rather than as a member
/// function's implicit `this`, because the class header
/// (include/game/bases/d_a_player_demo_manager.hpp) is FROZEN and this
/// function is not among its declared methods; all members it touches
/// (mCourseOutList, mDemoNoQueue) are public there for exactly this reason.
/// Given no in-TU caller and this TU's __sinit ties (see above) to the
/// world-map REL TUs, the most likely caller is world-map code deciding
/// which players enter the next course.
///
/// @note REQUIRES `daPyMng_c::mCourseInList` (confirmed real:
/// `mCourseInList__9daPyMng_c = .bss:0x80355130; size:0x10`, i.e.
/// `static int mCourseInList[4];`), which is NOT currently declared in
/// include/game/bases/d_a_player_manager.hpp. Flagged to the lead rather
/// than edited -- include/ is out of scope for this batch. This file will
/// not compile against the real header until that member is added.
///
/// Behaviour, reverse-engineered and then compile-verified byte-for-byte:
/// 1. Commit any pending explicit setCourseOutList() requests (mCourseOutList,
///    scanned front-to-back, stopping at the first -1 sentinel) into
///    mDemoNoQueue in order, and build a bitmask of which player VALUES they
///    used.
/// 2. Scan players 0..3 not already claimed by step 1, with an entry
///    (daPyMng_c::mPlayerEntry), and not excluded by
///    daPyMng_c::mCreateItem[type] bit 2; insert each into a 4-slot
///    candidate buffer (default {-1,-1,-1,-1} -- confirmed byte-for-byte
///    against the real .rodata at 0x802EF0F0, read directly from
///    original/wiimj2d.dol) at a uniformly random position via
///    dGameCom::rndInt(count), shifting existing candidates up and dropping
///    any that fall off the end -- this is the standard incremental
///    random-insertion construction of a bounded-size random sample/order.
/// 3. Fill any mDemoNoQueue slots step 1 left empty with the candidate
///    buffer, in order.
/// 4. Publish the finished mDemoNoQueue into daPyMng_c::mCourseInList and
///    reset mCourseOutList back to all -1.
void makeCourseInList(daPyDemoMng_c *pMgr) {
    int used = 0;
    int explicitCount = 0;
    for (int i = 0; i < 4; i++) {
        int v = pMgr->mCourseOutList[i];
        if (v == -1) {
            break;
        }
        pMgr->mDemoNoQueue[i] = v;
        used |= (1 << v);
        explicitCount++;
    }

    int cand[4] = {-1, -1, -1, -1};
    int count = 0;
    for (int i = 0; i < 4; i++) {
        if (used & (1 << i)) {
            continue;
        }
        if (daPyMng_c::mPlayerEntry[i] == 0) {
            continue;
        }
        if (daPyMng_c::mCreateItem[daPyMng_c::mPlayerType[i]] & 4) {
            continue;
        }
        count++;
        int r = dGameCom::rndInt(count);
        for (int j = 3; j > r; j--) {
            cand[j] = cand[j - 1];
        }
        cand[r] = i;
    }

    for (int i = explicitCount; i < 4; i++) {
        pMgr->mDemoNoQueue[i] = cand[i - explicitCount];
    }

    daPyMng_c::mCourseInList[0] = pMgr->mDemoNoQueue[0];
    daPyMng_c::mCourseInList[1] = pMgr->mDemoNoQueue[1];
    daPyMng_c::mCourseInList[2] = pMgr->mDemoNoQueue[2];
    daPyMng_c::mCourseInList[3] = pMgr->mDemoNoQueue[3];
    pMgr->mCourseOutList[0] = -1;
    pMgr->mCourseOutList[1] = -1;
    pMgr->mCourseOutList[2] = -1;
    pMgr->mCourseOutList[3] = -1;
}

/// Loop-body pointer `player` is hoisted out of the `for` on purpose -- with
/// it declared inside the loop body the compiler swaps the GPRs it and the
/// loop index `i` get allocated (r29/r30 instead of the target's r30/r29).
/// Direct (non-virtual) `bl` calls throughout, matching the target exactly;
/// no vtable dispatch here (double-checked -- no `lwz r12` / `mtctr` /
/// `bctrl` anywhere in this function's target disassembly).
void daPyDemoMng_c::setEnemyStageClearDemo(int playerNo) {
    dAcPy_c *player;
    for (int i = 0; i < 4; i++) {
        if (daPyMng_c::checkPlayer(i)) {
            player = daPyMng_c::getCtrlPlayer(i);
            if (player != nullptr) {
                player->onStatus(0x60);
                if (playerNo == i) {
                    player->setEnemyStageClearDemo();
                }
            }
        }
    }
}
