#pragma once
#include <game/bases/d_scene.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_fader.hpp>
#include <game/mLib/m_vec.hpp>
#include <constants/game_constants.h>

class dGameDisplay_c;

class dScStage_c : public dScene_c {
public:
    enum Exit_e {
        EXIT_0,
        EXIT_1,
        EXIT_2,
        EXIT_3
    };

    /// @brief The possible stage loop types.
    enum LOOP_TYPE_e {
        LOOP_NONE, ///< No stage looping occurs.
        LOOP_EDGES, ///< The stage loops around on the zone edges. Only works for specific zone sizes.
        LOOP_SECTION, ///< The stage loops in specific sections.
        LOOP_COUNT,
    };

    static void play();

    /// @brief Ends replay recording at the goal. @unofficial
    /// Signature pinned by `ReplayEnd__10dScStage_cFv`.
    static void ReplayEnd();

    typedef void (*changePosFunc)(mVec3_c *);
    static void setChangePosFunc(int);

    static void setTitleReplayRandomTable();

    static void setNextScene(u16, int, Exit_e, dFader_c::fader_type_e);

    static void createReplayDataHeap(EGG::Heap *heap, ulong size, int options);

    char pad[0x1198];
    u8 mCurrWorld;
    u8 mCurrCourse;
    u8 mCurrFile;
    u8 mCurrAreaNo;
    u8 mCurrLayer;

    u8 getCurrWorld() const { return mCurrWorld; }
    u8 getCurrArea() const { return mCurrAreaNo; }

    static dScStage_c *getInstance() { return m_instance; }
    static NOINLINE Exit_e getExitMode() { return m_exitMode; }

    /// @brief Whether the game is transitioning into a stage scene.
    /// @note `NOINLINE` for the same reason as getExitMode(): the original
    /// emits an out-of-line copy and a `bl` to it. That copy is at
    /// `0x8005EC90`, flushed inside `d_a_player_manager.cpp`, and it is the
    /// only copy of the symbol in the binary. @unofficial
    static NOINLINE bool getCourseIn() { return m_isCourseIn; }

    /// @brief Gets the stage HUD display.
    /// @note Out-of-line static accessor over `m_instance->+0x11D4`, defined in
    /// the undecompiled d_s_stage.cpp at 0x80101A70 and reached with a `bl`
    /// from several TUs -- so it must NOT be given an inline body here.
    /// @unofficial
    static dGameDisplay_c *getGameDisplay();

    static float getLoopPosX(float x);
    /// @brief [.sbss:0x8042A4D0] Pointer into the "otehon" (demo playback)
    /// clear-flag block; indexed with byte loads/stores at +0xb5..+0xb9.
    /// The pointee type is not yet known -- `u8 *` is a placeholder that makes
    /// those byte offsets indexable. @unofficial
    static u8 *m_OtehonClear_p;

    /// @brief [.sbss:0x8042A4DC] `int`, NOT `bool`: the target emits the full
    /// neg/or/srwi canonicalisation tail when writing it, which MWCC would skip
    /// for a bool destination. @unofficial
    static int m_goalType;

    static u32 m_exeFrame;
    static int m_loopType;
    static PLAYER_TYPE_e mCollectionCoin[STAR_COIN_COUNT];

    static const char mCdArcName[];

    /// @brief [.sbss:0x8042A4FC] Whether the game is transitioning from a
    /// non-stage scene into a stage scene. Declared before m_isCourseOut to
    /// match .sbss address order. @unofficial
    static bool m_isCourseIn;
    static bool m_isCourseOut; ///< Whether the game is transitioning from a stage scene to a non-stage scene.
    static bool m_KoopaJrEscape;
    static dInfo_c::GameMode_e m_gameMode;
    static Exit_e m_exitMode;

    static int m_miniGame;
    static bool m_isStaffCredit;
    static changePosFunc changePos;
    static dScStage_c *m_instance;

    ACTOR_PARAM_CONFIG(File, 8, 4);
    ACTOR_PARAM_CONFIG(NextGotoID, 0, 8);
};
