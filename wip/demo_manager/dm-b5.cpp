// Batch 5: land/toride and the demo-number queue.
// daPyDemoMng_c, .text 0x8005CC00-0x8005D27C.
//
// NOTE (blocking, reported to lead -- see report): this file currently
// requires two changes to shared headers that are NOT applied here (batch
// rules forbid editing include/):
//   1. include/game/bases/d_a_player_demo_manager.hpp:
//        void startControlDemoLandPlayer();
//      must become
//        bool startControlDemoLandPlayer();
//      Proven: the tail of the target function computes a canonical 0/1
//      value into r3 right before the epilogue (neg/or/srwi, the same
//      idiom the header itself already documents as "confirmed bool" for
//      startControlDemoAll/isAllPlayerControlDemo), AND its caller in this
//      same batch (executeEndToride, case m_08==1) uses the return value:
//      `bl startControlDemoLandPlayer; cmpwi r3,0x0; beq ...`.
//   2. include/game/bases/d_s_stage.hpp needs a new declaration:
//        static void ReplayEnd();
//      Proven: executeEndToride (case m_08==1) calls
//      `bl ReplayEnd__10dScStage_cFv` with no `this` load beforehand (the
//      only value in r3 at that point is startControlDemoLandPlayer's
//      just-returned boolean), so it must be a static member with no
//      explicit object argument. Not currently declared anywhere in the
//      repo (grepped both include/ and every corpus/dis text file).
//
// Both were proven byte-exact against SCRATCH copies of the two headers
// (with exactly the above two lines added/changed) via harness.py's
// extra_inc, not against the real frozen headers.
#include <game/bases/d_a_player_demo_manager.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_s_stage.hpp>

// ---------------------------------------------------------------------
// Two unnamed (fn_800XXXXX in the symbol map) file-scope statics.
// @unofficial Names kept in the project's existing fn_800XXXXX convention
// for unnamed-in-symbol-map functions (see d_a_player_manager.hpp's
// fn_8005f4d0/fn_8005f570) rather than invented descriptive names, so the
// literal identifier matches the placeholder the target disassembly and
// this TU's own frozen header prose ("this TU's fn_8005CCD0 dispatcher")
// already use for it.
//
// Both take an explicit daPyDemoMng_c* first argument rather than being
// true member functions of daPyDemoMng_c: adding them as declared class
// members would require editing the frozen header, which is out of scope
// for this batch. See report for the linkage finding on fn_8005CCD0 (its
// declared `static` was checked, not assumed -- see below).
// ---------------------------------------------------------------------

// @unofficial fn_8005CE50 (0x8005CE50): loops the 4 controllable players
// and plays a control-demo cutscene animation on each. Verified in-TU
// callers: executeStartToride (once) and executeEndToride (twice) -- three
// call sites, all within this batch -- so `static` (internal linkage) is
// correct per the brief's own test.
extern "C" static void fn_8005CE50(daPyDemoMng_c *mgr, daPlBase_c::AnimePlayArg_e animID);

// @unofficial fn_8005CCD0 (0x8005CCD0): a 0-4 step dispatcher that tries to
// advance the control-demo/toride state machine, gating each transition on
// isDemoMode()/isLandAll() and writing daPyDemoMng_c::m_08 on success.
// NOT marked `static`: grepped the FULL unit disassembly
// (tools/auto_decomp/work/dol_bases_d_a_player_demo_manager/target.txt,
// all 8,976 B / 51 functions) for "fn_8005CCD0" and found only its own
// .fn/.endfn pair -- zero in-TU call sites. Per the brief's own criterion
// ("if they do [have in-TU callers], static is correct"), the absence of
// any in-TU caller means something outside this TU must call it, so it
// cannot have internal linkage. This directly contradicts the brief's
// premise that both functions are file-statics; reported rather than
// silently reconciled. (Linkage does not affect the emitted instruction
// bytes either way -- every .fn in this unit's disassembly, static or not,
// shows "global" -- so this does not block proving the body byte-exact.)
extern "C" bool fn_8005CCD0(daPyDemoMng_c *mgr, int step);

// -----------------------------------------------------------------------
// 0x8005CC00 (208 B)
// -----------------------------------------------------------------------
bool daPyDemoMng_c::startControlDemoLandPlayer()
{
    if (daPyMng_c::mNum == 0) {
        return false;
    }

    int allDone = 1;
    dAcPy_c *player;
    for (int i = 0; i < 4; i++) {
        if (!daPyMng_c::checkPlayer(i)) {
            continue;
        }
        player = daPyMng_c::getCtrlPlayer(i);
        if (player == nullptr) {
            continue;
        }
        if (player->isBossDemoLand()) {
            if (!player->isDemoType(daPlBase_c::DEMO_PLAYER)) {
                player->startControlDemo();
                allDone = 0;
            }
        } else {
            allDone = 0;
        }
    }
    return allDone;
}

// -----------------------------------------------------------------------
// 0x8005CCD0 (252 B) -- unnamed, see fn_8005CCD0 above
// -----------------------------------------------------------------------
extern "C" bool fn_8005CCD0(daPyDemoMng_c *mgr, int step)
{
    switch (step) {
    case 0:
        mgr->setDemoMode(daPyDemoMng_c::MODE_4, 0);
        return true;
    case 1:
        if (!mgr->isDemoMode(daPyDemoMng_c::MODE_4, 0)) {
            break;
        }
        mgr->m_08 = 1;
        return true;
    case 2:
        mgr->setDemoMode(daPyDemoMng_c::MODE_5, 0);
        return true;
    case 3:
        if (!mgr->isLandAll()) {
            break;
        }
        if (!mgr->isDemoMode(daPyDemoMng_c::MODE_5, 2)) {
            break;
        }
        mgr->m_08 = 3;
        return true;
    case 4:
        if (!mgr->isDemoMode(daPyDemoMng_c::MODE_5, 4)) {
            break;
        }
        mgr->m_08 = 5;
        return true;
    default:
        break;
    }
    return false;
}

// -----------------------------------------------------------------------
// 0x8005CDD0 (124 B)
// -----------------------------------------------------------------------
bool daPyDemoMng_c::isLandAll()
{
    dAcPy_c *player;
    for (int i = 0; i < 4; i++) {
        if (!daPyMng_c::checkPlayer(i)) {
            continue;
        }
        player = daPyMng_c::getCtrlPlayer(i);
        if (player == nullptr) {
            continue;
        }
        if (!player->isBossDemoLand()) {
            return false;
        }
    }
    return true;
}

// -----------------------------------------------------------------------
// 0x8005CE50 (120 B) -- unnamed, see fn_8005CE50 above.
// NOT a bit-identical twin of endControlDemoAll (batch 4's near-twin
// finding) -- calls setControlDemoCutscene, not endControlDemo. Only the
// loop skeleton is shared; body NOT copied from batch 4.
// -----------------------------------------------------------------------
extern "C" static void fn_8005CE50(daPyDemoMng_c *mgr, daPlBase_c::AnimePlayArg_e animID)
{
    dAcPy_c *player;
    for (int i = 0; i < 4; i++) {
        if (!daPyMng_c::checkPlayer(i)) {
            continue;
        }
        player = daPyMng_c::getCtrlPlayer(i);
        if (player == nullptr) {
            continue;
        }
        player->setControlDemoCutscene(animID);
    }
}

// -----------------------------------------------------------------------
// 0x8005CED0 (216 B)
// -----------------------------------------------------------------------
void daPyDemoMng_c::executeStartToride()
{
    switch (m_08) {
    case 0:
        m_8c = -1;
        break;
    case 1:
        if (m_8c == -1) {
            dAcPy_c *player;
            for (int i = 0; i < 4; i++) {
                if (!daPyMng_c::checkPlayer(i)) {
                    continue;
                }
                player = daPyMng_c::getCtrlPlayer(i);
                if (player == nullptr) {
                    continue;
                }
                if (!player->isBossDemoLand()) {
                    continue;
                }
                m_8c = i;
                break;
            }
        }
        if (isLandAll()) {
            fn_8005CE50(this, daPlBase_c::DEMO_ANIME_BOSS_SET_UP);
            m_08 = 2;
        }
        break;
    default:
        break;
    }
}

// -----------------------------------------------------------------------
// 0x8005CFB0 (160 B)
// -----------------------------------------------------------------------
void daPyDemoMng_c::executeEndToride()
{
    switch (m_08) {
    case 0:
        onLandStopReq();
        m_08 = 1;
        break;
    case 1:
        if (startControlDemoLandPlayer()) {
            dScStage_c::ReplayEnd();
            m_08 = 2;
        }
        break;
    case 3:
        fn_8005CE50(this, daPlBase_c::DEMO_ANIME_BOSS_GLAD);
        m_08 = 4;
        break;
    case 5:
        fn_8005CE50(this, daPlBase_c::DEMO_ANIME_BOSS_ATTENTION);
        m_08 = 6;
        break;
    default:
        break;
    }
}

// -----------------------------------------------------------------------
// 0x8005D050 (64 B)
// -----------------------------------------------------------------------
void daPyDemoMng_c::setCourseOutList(s8 playerNo)
{
    for (int i = 0; i < 4; i++) {
        if (mCourseOutList[i] == -1) {
            mCourseOutList[i] = playerNo;
            return;
        }
    }
}

// -----------------------------------------------------------------------
// 0x8005D090 (40 B)
// -----------------------------------------------------------------------
bool daPyDemoMng_c::checkDemoNo(s8 playerNo)
{
    if (mDemoNoQueue[0] == -1) {
        goto ret_true;
    }
    if (mDemoNoQueue[0] != playerNo) {
        goto ret_false;
    }
ret_true:
    return true;
ret_false:
    return false;
}

// -----------------------------------------------------------------------
// 0x8005D0C0 (8 B)
// -----------------------------------------------------------------------
int daPyDemoMng_c::getNextDemoNo()
{
    return mDemoNoQueue[1];
}

// -----------------------------------------------------------------------
// 0x8005D0D0 (36 B)
// -----------------------------------------------------------------------
void daPyDemoMng_c::turnNextDemoNo()
{
    mDemoNoQueue[0] = mDemoNoQueue[1];
    mDemoNoQueue[1] = mDemoNoQueue[2];
    mDemoNoQueue[2] = mDemoNoQueue[3];
    mDemoNoQueue[3] = -1;
}

// -----------------------------------------------------------------------
// 0x8005D100 (380 B) -- largest in this batch, no usable precedent.
// -----------------------------------------------------------------------
void daPyDemoMng_c::clearDemoNo(s8 playerNo)
{
    int value = playerNo;
    daPyDemoMng_c *dst = this;
    int count = 0;

    for (int i = 0; i < 4; i++) {
        int loaded = mDemoNoQueue[i];
        if (loaded != -1 && value != loaded) {
            dst->mDemoNoQueue[0] = loaded;
            dst = (daPyDemoMng_c *)((char *)dst + 4);
            count++;
        }
    }

    if (count >= 4) {
        return;
    }

    for (int i = count; i < 4; i++) {
        mDemoNoQueue[i] = -1;
    }
    for (int i = 0; i < 4; i++) {
        mCourseOutList[i] = -1;
    }
}
