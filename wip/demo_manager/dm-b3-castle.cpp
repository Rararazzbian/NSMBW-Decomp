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
//   executeGoalCastle      NOT byte-exact, TODO: this file was edited but
//                          NOT recompiled/rediffed after the last change to
//                          the isCourseDataFlag "ok" logic below -- recompile
//                          and re-diff against target FIRST, see report for
//                          exactly what to check.
//
// This file was compiled, during authoring, against a SCRATCH-ONLY patched
// copy of d_s_stage.hpp/d_info.hpp (outside the repo, see report) because
// executeGoalCastle needs three fields that do not exist in the tracked
// include/ headers: dScStage_c::m_OtehonClear_p, dScStage_c::m_goalType,
// dInfo_c's two fields hidden in its documented pad4[0x8]. It will NOT
// compile as-is against the real, currently-committed include/ headers.
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
// executeGoalCastle -- 0x8005C410, 692 B
//
// BLOCKED on two dScStage_c statics not present in the frozen
// include/game/bases/d_s_stage.hpp: dScStage_c::m_OtehonClear_p (.sbss
// 0x8042A4D0, 4 bytes) and dScStage_c::m_goalType (.sbss 0x8042A4DC, 4
// bytes) -- see report. Written against a SCRATCH-ONLY local copy of that
// header (not committed, not touching include/) that adds:
//   static u8 *m_OtehonClear_p;
//   static int m_goalType;
// so the logic below can be authored and verified; the lead must add the
// real fields (with real types/names) to the tracked header before this
// function will compile against it.
// ---------------------------------------------------------------------------
void daPyDemoMng_c::executeGoalCastle() {
    switch (m_08) {
    case 0:
        if (m_40 != 0) {
            if (m_54 < 0x168) {
                return;
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
            dInfo_c *info = dInfo_c::getInstance();
            info->m_68 = 0;
            if (m_42) {
                info->m_68 = 1;
            }
            info->m_64 = m_41;
            dScStage_c::m_goalType = (mGoalType != 0);

            if (dInfo_c::m_startGameInfo.mGameMode == dInfo_c::GAME_MODE_SUPER_GUIDE) {
                u8 world = dInfo_c::m_startGameInfo.mWorld1;
                u8 level = dInfo_c::m_startGameInfo.mLevel1;

                if (world <= 9 && level <= 0x29) {
                    if (level == 3 && world == 2) {
                        dMj2dGame_c *save = dSaveMng_c::m_instance->getSaveGame(-1);
                        bool ok = true;
                        if (mGoalType == 0) {
                            ok = save->isCourseDataFlag(world, level, 0x90);
                        }
                        if (ok && mGoalType != 0) {
                            ok = save->isCourseDataFlag(world, level, 0x120);
                        }
                        if (!ok) {
                            dScStage_c::m_OtehonClear_p[0xb9] = 0;
                            dScStage_c::m_OtehonClear_p[0xb8] = 1;
                            dScStage_c::m_OtehonClear_p[0xb5] = 1;
                            m_08 = 3;
                            break;
                        }
                    } else if (!dWmLib::IsCourseClear(world, level)) {
                        dScStage_c::m_OtehonClear_p[0xb9] = 0;
                        dScStage_c::m_OtehonClear_p[0xb8] = 1;
                        dScStage_c::m_OtehonClear_p[0xb5] = 1;
                        m_08 = 3;
                        break;
                    }
                }
            }

            dScStage_c::setNextScene(3, 0, dScStage_c::EXIT_0, dFader_c::FADER_MARIO);
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
