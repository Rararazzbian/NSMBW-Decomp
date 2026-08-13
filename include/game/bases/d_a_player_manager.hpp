#pragma once
#include <game/bases/d_a_player.hpp>
#include <game/bases/d_player_model_manager.hpp>
#include <game/bases/d_a_yoshi.hpp>
#include <game/bases/d_a_player_demo_manager.hpp>
#include <game/bases/d_multi_manager.hpp>
#include <game/bases/d_attention.hpp>
#include <game/bases/d_player_effect_manager.hpp>
#include <constants/game_constants.h>

class daPyMng_c {
public:
    /// @note Was declared `void`. The target sets r3 to 1 or 0 on both
    /// converging return paths before `blr`. Proven by compiling both ways
    /// against the target: only the `bool` form is byte-exact. @unofficial
    static bool changeItemKinopioPlrNo(int &);
    static int getPlayerIndex(PLAYER_TYPE_e); ///< @unofficial
    static dAcPy_c *getPlayer(int);
    static dAcPy_c *getCtrlPlayer(int);
    static daYoshi_c *getYoshi(int);
    static daYoshi_c *getYoshiDirectP(int); ///< @unofficial
    static int getYoshiNum(); ///< @unofficial
    /// @return Whether a Yoshi was deleted. @unofficial
    static int deleteCullingYoshi();
    /// @brief Spawns a player-linked actor at @p pos, in the first free player
    /// slot. @unofficial
    /// @return Whether a slot was found and the actor created.
    /// @note Unnamed in the symbol map (0x8005F4D0); the name here is
    /// invented and pinned by a syms.txt entry.
    /// @note The return type was `void` until the body was disassembled. Every
    /// exit path explicitly loads r3 with 0 or 1, so it cannot be. CFront
    /// mangling omits return types, so the syms.txt pin and the mangled name
    /// could never have caught this -- only the body could.
    static bool fn_8005f4d0(mVec3_c *pos, int mode, int flag);
    static void incCoin(int);
    static void addScore(int, int);
    static void setHipAttackQuake(int, u8);
    static int getNumInGame();
    static int getItemKinopioNum();
    static dPyMdlMng_c::ModelType_e getCourseInPlayerModelType(u8);
    static void setPlayer(int, dAcPy_c *);
    static bool fn_8005f570(PLAYER_POWERUP_e, int); ///< @unofficial
    static void setCarryOverYoshiInfo(u8 plrNo, u8 yoshiColor, int fruitCount);
    static bool addNum(int);
    static bool decNum(int);
    /// @note Was declared `addRest(int)`. The real symbol is
    /// `addRest__9daPyMng_cFiib`, so it takes three parameters. Nothing in the
    /// tree called it, so correcting the mangling broke nothing. @unofficial
    static bool addRest(int, int, bool);
    /// @note Was declared `bool`. The target returns a raw `rest - 1` and
    /// falls through with no set value on one path -- neither fits a bool.
    /// Proven by compiling both ways: as `int` the body is byte-exact at
    /// 36/36 instructions. @unofficial
    static int decRest(int);
    static u32 getEntryNum();
    static bool isEntryNum1() { return getEntryNum() == 1; }
    static void startYoshiBGM();
    static void stopYoshiBGM();
    static void startMissBGM(int plrNo);
    static void startStarBGM();
    static void stopStarBGM();
    static bool isCreateBalloon(int plrNo);
    static int getYoshiColor(u8 plrNo);
    static int getYoshiFruit(u8 plrNo);
    static daYoshi_c *createYoshi(mVec3_c &, int, dAcPy_c *);

    static bool isItemKinopio(int plrNo) {
        bool res = false;
        daPlBase_c *player = getPlayer(plrNo);
        if (player != nullptr && player->isItemKinopio()) {
            res = true;
        }
        return res;
    }

    static bool checkPlayer(u8 plrNo) { return mActPlayerInfo & (1 << plrNo); }
    static int getRest(PLAYER_TYPE_e plrNo) { return mRest[plrNo]; }
    static PLAYER_TYPE_e getPlayerType(int plrNo) { return mPlayerType[plrNo]; }
    static int getPlayerMode(int plrNo) { return mPlayerMode[plrNo]; }
    static nw4r::math::VEC3 getPlayerSetPos(u8 file, u8 gotoNo);

    // ---------------------------------------------------------------------
    // Declared while decompiling d_a_player_manager.cpp. Every one of these
    // is a real symbol in the map, so the NAMES are authoritative. The TYPES
    // are inferred from load/store widths in the disassembly, and the RETURN
    // types of the functions are guesses -- CFront omits return types from
    // mangling, so nothing in the symbol map can confirm them. A wrong return
    // type is invisible to every symbol comparison and shows up only in the
    // body (as fn_8005f4d0's `void` did). Correct them from the bodies.
    // ---------------------------------------------------------------------

    static void initGame();
    static void initStage();
    static void exitStage();
    static void courseIn();
    static void setDefaultParam();
    static int getPlayerCreateAction();
    /// @note Was declared `void`; the body proves `bool`, the same evidence
    /// pattern as fn_8005f4d0. Sixth wrong return type in this class --
    /// CFront omits return types from mangling, so none of them were
    /// visible to any symbol comparison. @unofficial
    static bool create(int plrNo, mVec3_c *pos, int type, u8 flag);
    static void createCourseInit();
    static void update();
    static bool isPlayerPauseEnable(s8 plrNo);
    static void decideCtrlPlrNo();
    /// @note Was declared `void`. Every exit path loads a clean 0 or 1 into
    /// r3, the same shape as fn_8005f4d0. @unofficial
    static bool setYoshi(daPlBase_c *);
    static void releaseYoshi(daPlBase_c *);
    static int getActScrollInfo();
    static int getScrollNum();
    /// @note Both were declared `bool`. The target never sets r3 on any path.
    /// Proven by compiling both ways: as `bool`, MWCC reserves r3 for the
    /// return and allocates the `mNum` temp into r4, which mismatches; as
    /// `void` it lands in r3 and is byte-exact. @unofficial
    static void addNum();
    static void decNum();
    static dAcPy_c *getItemKinopio();
    static int getCourseInListPlrNo(int);
    static int getCoinAll();
    static void incRestAll(bool);
    static void setCourseInStarBGM();
    static bool checkLastAlivePlayer();
    static void executeLastPlayer();
    static void executeLastAll();
    static void checkBonusNoCap();
    static void initYoshiPriority(daPlBase_c *);
    static void setYoshiPriority(daPlBase_c *);
    static bool isEffectStop(int plrNo);
    static bool isAcceptQuake(int plrNo);
    static void checkCorrectCreateInfo();

    static int mNum;
    static u32 mPauseDisable;
    static u8 mActPlayerInfo;
    static int mPauseEnableInfo;
    static u32 mStopTimerInfo;
    static PLAYER_TYPE_e mPlayerType[4];
    static PLAYER_POWERUP_e mPlayerMode[4];
    static PLAYER_POWERUP_e mKinopioMode;
    static int mKinopioCarryCount;
    static u32 mCreateItem[4];
    static int mPlayerEntry[4];
    static int mRest[4];
    static u32 mCtrlPlrNo;
    static s16 m_star_time[4];
    static s16 m_star_count[4];
    static int mAllBalloon;
    static int mTimeUpPlayerNum;

    /// @brief [.bss:0x80355130, 0x10] The published course-entry order, filled
    /// from `daPyDemoMng_c`'s course-out list. Declaration only -- the
    /// definition lives in the still-undecompiled `d_a_player_manager.cpp`.
    /// @unofficial
    static int mCourseInList[4];

    // --- .bss statics, in address order (0x80355110 onwards) --------------
    static int m_playerID[4];        ///< [.bss:0x80355110] fBaseID handles.
    static int m_yoshiID[4];         ///< [.bss:0x80355120] fBaseID handles.
    // mCourseInList                    [.bss:0x80355130] declared above.
    static int m_yoshiFruit[4];      ///< [.bss:0x80355140]
    // mPlayerEntry/mPlayerType/mPlayerMode/mCreateItem/mRest declared above.
    /// @brief [.bss:0x803551A0] Indexed by PLAYER_TYPE_e, not by player slot --
    /// getCoinAll() reads mPlayerType[i] and uses THAT as the index. @unofficial
    static int mCoin[4];
    static int m_quakeTimer[4];      ///< [.bss:0x803551B0]
    static int m_quakeEffectFlag[4]; ///< [.bss:0x803551C0]

    /// @brief The four managers daPyMng_c owns by value. Their sizes are fixed
    /// by the .bss layout -- 0x98 / 0x5C / 0x58 / 0xC5C -- and __sinit
    /// constructs them in exactly this order. If any sizeof is wrong the whole
    /// section shifts, and no per-function diff can see it. daPyDemoMng_c is
    /// proven; the other three are still being reconstructed. @unofficial
    static daPyDemoMng_c mDemoManager;  ///< [.bss:0x803551E0, 0x98]
    static dMultiMng_c mMultiManager;   ///< [.bss:0x80355284, 0x5C]
    static dAttention_c mAttention;     ///< [.bss:0x803552F0, 0x58]
    static dPyEffectMng_c mEffectMng;   ///< [.bss:0x80355354, 0xC5C]

    // --- .sbss statics not previously declared ----------------------------
    static u8 m_yoshiColor[4];   ///< [.sbss:0x80429F8C] byte-indexed, unscaled.
    static int mScore;           ///< [.sbss:0x80429FA0] aggregate, not per-player.
    static u32 mStopTimerInfoOld; ///< [.sbss:0x80429FBC]
    static int mQuakeTrigger;    ///< [.sbss:0x80429FC0]
    static int mBgmState;        ///< [.sbss:0x80429FC4]
    static int mBonusNoCap;      ///< [.sbss:0x80429FC8]
};
