#include <game/bases/d_a_player_demo_manager.hpp>
#include <game/bases/d_a_player_manager.hpp>
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_a_player_base.hpp>
#include <game/mLib/m_effect.hpp>
#include <game/bases/d_audio.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_s_stage.hpp>
#include <game/bases/d_save_mng.hpp>
#include <game/bases/d_mj2d_data.hpp>
#include <game/bases/d_fader.hpp>

// ===========================================================================
// batch 3 of 6 -- goal-demo tail, fireworks, castle
// 0x8005C090-0x8005C6D0 (executeGoalCastle ends at 0x8005C6C4)
//
// STATUS (see wip/demo_manager/dm-b3-report.md for full detail):
//   executeGoalDemo        BYTE-EXACT (verified)
//   setGoalDemoKimeAll     BYTE-EXACT (verified)
//   setGoalDemoRunCastle   BYTE-EXACT (verified)
//   isAllPlayerGoalIn      BYTE-EXACT (verified)
//   setHanabiEffect        very close, NOT byte-exact -- see report
//   executeGoalCastle      BYTE-EXACT (verified: verify.py MATCH, and
//                          confirmed with harness.extract()'s stricter
//                          raw-branch-word comparison, 173/173, 0 diffs).
//                          dScStage_c::m_OtehonClear_p/m_goalType and
//                          dInfo_c::m_64/m_68 are now real fields in the
//                          tracked include/ headers, so this compiles
//                          against them directly -- no scratch override
//                          needed for this function anymore.
// ===========================================================================

// ---------------------------------------------------------------------------
// executeGoalDemo -- 0x8005C090, 156 B
// ---------------------------------------------------------------------------
void daPyDemoMng_c::executeGoalDemo() {
    switch (m_08) {
    case 0:
        executeGoalDemo_Pole();
        break;
    case 1:
        executeGoalDemo_PoleDown();
        break;
    case 2:
        executeGoalDemo_JumpCheck();
        break;
    case 3:
        executeGoalDemo_Jump();
        break;
    case 4:
        executeGoalDemo_Land();
        break;
    case 5:
        executeGoalDemo_KimeWait();
        break;
    }

    calcNotGoalPlayer();
    calcGoalCenterPos();
}

// ---------------------------------------------------------------------------
// setGoalDemoKimeAll -- 0x8005C130, 100 B
// ---------------------------------------------------------------------------
void daPyDemoMng_c::setGoalDemoKimeAll() {
    for (int i = 0; i < 4; i++) {
        if (mGoalDemoList[i] != -1) {
            dAcPy_c *player = daPyMng_c::getCtrlPlayer(mGoalDemoList[i]);
            if (player != NULL) {
                player->onStatus(0x6b);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// setGoalDemoRunCastle -- 0x8005C1A0, 112 B
// ---------------------------------------------------------------------------
void daPyDemoMng_c::setGoalDemoRunCastle() {
    setDemoMode(MODE_0, 0);

    for (int i = 0; i < 4; i++) {
        if (mGoalDemoList[i] != -1) {
            dAcPy_c *player = daPyMng_c::getCtrlPlayer(mGoalDemoList[i]);
            if (player != NULL) {
                player->onStatus(0x6d);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// isAllPlayerGoalIn -- 0x8005C210, 160 B
// ---------------------------------------------------------------------------
bool daPyDemoMng_c::isAllPlayerGoalIn() {
    dAcPy_c *player;
    for (int i = 0; i < 4; i++) {
        if (daPyMng_c::checkPlayer(i)) {
            player = daPyMng_c::getCtrlPlayer(i);
            if (player != NULL) {
                if (player->isStatus(0x65) && !player->isStatus(0x6e)) {
                    return false;
                }
            }
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// setHanabiEffect -- 0x8005C2B0, 348 B
//
// Owns 11 @LOCAL@ function-scope statics (the brief's headline "nine" is the
// scHanabiOffset_1..9 family; scHanabiOffsetDt and scHanabiEffectID are the
// two additional ones named in the same paragraph -- see report):
//   scHanabiOffset_1..9   .rodata  1..9-element position/id tables
//   scHanabiOffsetDt      .data    9 pointers, one per scHanabiOffset_N
//   scHanabiEffectID      .data    4 pointers to the 4 firework colour names
//
// A tenth, UNNAMED 10-entry string-pointer table also lives in .data
// (0x803099F0-0x80309A18, three distinct strings: "..._1up" x3, "..._k" x6,
// "..._star" x1) with no @LOCAL@ symbol in wiimj2d_symbols.txt at all -- it
// is written here as a switch so the compiler synthesises it anonymously,
// matching the target's anonymous "lbl_803099ED" blob rather than minting a
// twelfth named object that doesn't exist on the target side.
// ---------------------------------------------------------------------------
void daPyDemoMng_c::setHanabiEffect() {
    struct HanabiPos_t {
        float x, y;
        u16 m_08, m_0a;
    };

    static const HanabiPos_t scHanabiOffset_1[1] = {
        { 0.0f, 16.0f, 26, 0 },
    };
    static const HanabiPos_t scHanabiOffset_2[2] = {
        { 64.0f, 16.0f, 26, 0 },
        { -64.0f, 16.0f, 26, 0 },
    };
    static const HanabiPos_t scHanabiOffset_3[3] = {
        { 0.0f, 16.0f, 20, 0 },
        { 64.0f, 16.0f, 10, 0 },
        { -64.0f, 16.0f, 20, 0 },
    };
    static const HanabiPos_t scHanabiOffset_4[4] = {
        { 0.0f, 16.0f, 20, 0 },
        { -32.0f, 8.0f, 20, 0 },
        { 64.0f, 24.0f, 10, 0 },
        { -64.0f, 32.0f, 20, 0 },
    };
    static const HanabiPos_t scHanabiOffset_5[5] = {
        { 0.0f, 16.0f, 20, 0 },
        { -32.0f, 8.0f, 20, 0 },
        { 80.0f, 24.0f, 10, 0 },
        { 32.0f, 24.0f, 10, 0 },
        { -64.0f, 32.0f, 20, 0 },
    };
    static const HanabiPos_t scHanabiOffset_6[6] = {
        { 0.0f, 16.0f, 20, 0 },
        { 64.0f, 32.0f, 10, 0 },
        { -32.0f, 8.0f, 20, 0 },
        { 80.0f, 24.0f, 10, 0 },
        { 32.0f, 8.0f, 10, 0 },
        { -64.0f, 32.0f, 20, 0 },
    };
    static const HanabiPos_t scHanabiOffset_7[7] = {
        { 0.0f, 16.0f, 20, 0 },
        { 64.0f, 24.0f, 10, 0 },
        { -16.0f, 16.0f, 20, 0 },
        { 80.0f, 16.0f, 10, 0 },
        { -32.0f, 8.0f, 20, 0 },
        { 32.0f, 8.0f, 10, 0 },
        { -64.0f, 32.0f, 20, 0 },
    };
    static const HanabiPos_t scHanabiOffset_8[8] = {
        { 0.0f, 16.0f, 20, 0 },
        { 48.0f, 24.0f, 10, 0 },
        { -32.0f, 32.0f, 20, 0 },
        { 16.0f, 16.0f, 20, 0 },
        { 80.0f, 16.0f, 10, 0 },
        { -32.0f, 8.0f, 20, 0 },
        { 32.0f, 8.0f, 10, 0 },
        { -64.0f, 32.0f, 20, 0 },
    };
    static const HanabiPos_t scHanabiOffset_9[9] = {
        { 0.0f, 16.0f, 20, 0 },
        { 48.0f, 40.0f, 10, 0 },
        { -32.0f, 40.0f, 20, 0 },
        { 80.0f, 16.0f, 10, 0 },
        { 0.0f, 16.0f, 20, 0 },
        { 16.0f, 32.0f, 20, 0 },
        { -32.0f, 8.0f, 20, 0 },
        { 80.0f, 16.0f, 10, 0 },
        { -64.0f, 32.0f, 20, 0 },
    };

    static const HanabiPos_t *const scHanabiOffsetDt[9] = {
        scHanabiOffset_1, scHanabiOffset_2, scHanabiOffset_3,
        scHanabiOffset_4, scHanabiOffset_5, scHanabiOffset_6,
        scHanabiOffset_7, scHanabiOffset_8, scHanabiOffset_9,
    };

    static const char *const scHanabiEffectID[4] = {
        "Wm_ob_fireworks_y",
        "Wm_ob_fireworks_b",
        "Wm_ob_fireworks_g",
        "Wm_ob_fireworks_p",
    };

    if (m_40 != 0 && m_44 < m_41) {
        const HanabiPos_t &offset = scHanabiOffsetDt[m_41 - 1][m_44];
        mVec3_c pos = mFireworkPos;
        pos.x += offset.x;
        pos.y += offset.y;

        if (m_44 == 0) {
            if (m_40 != 0) {
                if (m_41 < 10) {
                    static const char *const names[10] = {
                        "Wm_ob_fireworks_1up", "Wm_ob_fireworks_1up", "Wm_ob_fireworks_1up",
                        "Wm_ob_fireworks_k", "Wm_ob_fireworks_k", "Wm_ob_fireworks_k",
                        "Wm_ob_fireworks_k", "Wm_ob_fireworks_k", "Wm_ob_fireworks_k",
                        "Wm_ob_fireworks_star",
                    };
                    const char *name = names[m_41];
                    mEf::createEffect(name, 0, &pos, NULL, NULL);
                    dAudio::g_pSndObjMap->startSound(0x251, pos, 0);
                }
            }
        } else {
            m_45 = (m_45 + dGameCom::rndInt(6) + 1) & 3;
            mEf::createEffect(scHanabiEffectID[m_45], 0, &pos, NULL, NULL);
            dAudio::g_pSndObjMap->startSound(0x250, pos, 0);
        }
    }
}

// ---------------------------------------------------------------------------
// executeGoalCastle -- 0x8005C410, 692 B -- BYTE-EXACT (173/173 instructions)
//
// Key structural findings, for whoever assembles the final file:
//
// - case 0's early "if (m_54 < 0x168) return;" is actually a `break;` --
//   its `blt` branches to the switch's shared exit block (which still calls
//   calcGoalCenterPos()), not straight to the epilogue. verify.py's table
//   could not see this (its canonicaliser strips branch-target addresses
//   down to a bare ".L", so "branches somewhere" reads the same as
//   "branches to the right somewhere") -- only harness.extract()'s
//   raw-branch-word comparison (which keeps the encoded displacement)
//   caught the 2-instruction offset this caused. Worth flagging generally:
//   a verify.py MATCH is necessary but not sufficient for control-flow
//   correctness; extract()'s stricter form is the real check for functions
//   with any internal branching. (verify.py's own module docstring says
//   the same about pooled-literal values, for the same underlying reason.)
//
// - case 2's OTEHON-fail logic is NOT a simple if/else between the two
//   isCourseDataFlag() checks, and NOT a bool accumulator either (a bool
//   `ok` variable forces a `mr r4,r3` register-preserving copy the target
//   does not have, because it defers the failure test past the second
//   call). The target tests each call's result *immediately*: the 0x90
//   check's failure branches forward past the whole second check straight
//   to the (shared) OTEHON-write block; the 0x120 check's failure falls
//   through into that same block with no branch instruction at all, because
//   the block is laid out immediately after it. That shared-fail-block
//   shape needed an explicit `goto` -- writing the fail code twice in
//   source (once per check) does not get folded by MWCC and was measured
//   6 instructions too many; an `ok` accumulator was 2 too many (the
//   `mr r4,r3` pair). `if (mGoalType == 0 || save->isCourseDataFlag(...))
//   goto castle_success;` reproduces the target's short-circuit branch
//   pair exactly.
//
// - dInfo_c::getInstance()->m_68/m_64 must NOT be cached in a local
//   `dInfo_c *info` across the `if (m_42)` branch: the target reloads
//   `dInfo_c::m_instance` fresh at each of the three uses (three separate
//   `lwz ...,m_instance__7dInfo_c@sda21(r0)`, including once more at the
//   post-branch merge point) rather than keeping it live in a register.
//   Caching it in one register was 2 instructions too few.
//
// - `dScStage_c::m_OtehonClear_p` (a plain, non-const global pointer)
//   reloads at EVERY dereference if you write
//   `dScStage_c::m_OtehonClear_p[i] = ...` three times in a row; hoisting
//   it to `u8 *otehon = dScStage_c::m_OtehonClear_p;` once per block
//   collapses that to a single load, matching target exactly (this cost 4
//   instructions -- 2 redundant reloads x 2 duplicated OTEHON-write sites).
//
// - The final case-2 success call is `dFader_c::FADER_CIRCLE_TARGET` (5),
//   not `FADER_MARIO` (4) -- confirmed from the raw `li r6, 0x5` in target;
//   an earlier guess had this wrong.
//
// - Signedness lever confirmed on a second axis (see HANDOFF.md "Signedness
//   is visible and load-bearing"): the `world <= 9 && level <= 0x29` bound
//   check needs the raw `u8` locals compared directly (-> `cmplwi`), but the
//   `level == 3 && world == 2` equality check right after needs those same
//   byte values copied into fresh `int` locals first (-> `cmpwi`) -- same
//   two registers, same values, different instruction per operator kind.
// ---------------------------------------------------------------------------
void daPyDemoMng_c::executeGoalCastle() {
    switch (m_08) {
    case 0:
        if (m_40 != 0) {
            if (m_54 < 0x168) {
                break;
            }
            m_44 = m_41;
            m_0c = 10;
            m_08 = 1;
        } else if (isAllPlayerGoalIn()) {
            m_08 = 2;
            m_0c = 0x1c;
        }
        break;

    case 1:
        if (m_0c == 0) {
            m_44--;
            setHanabiEffect();
            m_0c = 0x1c;
            if (m_44 == 0) {
                m_08 = 2;
                m_0c = 0x3c;
            }
        }
        break;

    case 2:
        if (m_0c == 0) {
            dInfo_c::getInstance()->m_68 = 0;
            if (m_42) {
                dInfo_c::getInstance()->m_68 = 1;
            }
            dInfo_c::getInstance()->m_64 = m_41;
            dScStage_c::m_goalType = (mGoalType != 0);

            if (dInfo_c::m_startGameInfo.mGameMode == dInfo_c::GAME_MODE_SUPER_GUIDE) {
                u8 world = dInfo_c::m_startGameInfo.mWorld1;
                u8 level = dInfo_c::m_startGameInfo.mLevel1;

                if (world <= 9 && level <= 0x29) {
                    int l = level;
                    int w = world;
                    if (l == 3 && w == 2) {
                        dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
                        if (mGoalType == 0) {
                            if (!save->isCourseDataFlag(world, level, 0x90)) {
                                goto otehon_fail1;
                            }
                        }
                        if (mGoalType == 0 || save->isCourseDataFlag(world, level, 0x120)) {
                            goto castle_success;
                        }
                    otehon_fail1: {
                        u8 *otehon = dScStage_c::m_OtehonClear_p;
                        otehon[0xb9] = 0;
                        otehon[0xb8] = 1;
                        otehon[0xb5] = 1;
                        m_08 = 3;
                        break;
                    }
                    } else if (!dWmLib::IsCourseClear(world, level)) {
                        u8 *otehon = dScStage_c::m_OtehonClear_p;
                        otehon[0xb9] = 0;
                        otehon[0xb8] = 1;
                        otehon[0xb5] = 1;
                        m_08 = 3;
                        break;
                    }
                }
            }

        castle_success:
            dScStage_c::setNextScene(3, 0, dScStage_c::EXIT_0, dFader_c::FADER_CIRCLE_TARGET);
            mMode = MODE_0;
        }
        break;

    case 3:
        if (dScStage_c::m_OtehonClear_p[0xb6]) {
            if (dScStage_c::m_OtehonClear_p[0xb7]) {
                dScStage_c::setNextScene(3, 0, dScStage_c::EXIT_0, dFader_c::FADER_CIRCLE_MIDDLE);
            } else {
                dScStage_c::setNextScene(3, 0, dScStage_c::EXIT_2, dFader_c::FADER_CIRCLE_MIDDLE);
            }
        }
        break;
    }

    calcGoalCenterPos();
}
