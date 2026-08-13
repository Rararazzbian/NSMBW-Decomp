#pragma once
#include <types.h>
#include <game/mLib/m_vec.hpp>

/**
 * @brief The cross-player "demo" (cutscene) coordinator for the goal pole,
 * castle-goal, tow-rope (toride) and control-demo sequences. One process-wide
 * singleton, not an actor.
 *
 * @details Proof summary (see the wip/demo_manager proof report for the full
 * evidence trail):
 * - TU is dol/bases/d_a_player_demo_manager.cpp, .text 0x8005B3A0-0x8005D7E0.
 * - @p __vt__13daPyDemoMng_c (.data:0x80309A18, size 0xC) is
 *   `{ 0, 0, &daPyDemoMng_c::~daPyDemoMng_c }` read directly out of
 *   original/wiimj2d.dol. Zero offset-to-top, zero RTTI pointer (-RTTI off),
 *   one function slot: this class has exactly ONE virtual function -- the
 *   destructor -- and NO base class. Reproduced byte-for-byte by compiling
 *   this header's ctor/dtor and diffing the emitted @p __vt__13daPyDemoMng_c
 *   and the emitted `stw r4, 0x0(r3)` vtable-store in the constructor against
 *   the real disassembly (dol_bases_d_a_player_demo_manager/target.txt).
 * - `sizeof(daPyDemoMng_c) == 0x98`, read from the gap between consecutive
 *   file-scope static objects in `__sinit_\d_a_player_manager_cpp`
 *   (.text:0x80061310): the singleton is placement-constructed into static
 *   storage at `&m_playerID__9daPyMng_c + 0xD0` and the NEXT static object's
 *   destructor-chain node starts at `+0x168` (`0xD0 + 0x98(sizeof) + 0xC
 *   (chain-node size) == 0x168`, exact, zero slack). Cross-checked against
 *   the last field touched by init() (m_94, ends at 0x98) -- two independent
 *   derivations agree exactly.
 * - There is NO `new`/`__nw__` call anywhere for this class -- it is a static
 *   global, not heap-allocated. This contradicts the usual
 *   "read sizeof from the allocation site" method for this project; flagged
 *   to the lead rather than silently reconciled.
 * - Field offsets 0x10 (mFlags), 0x14 (mGoalType), 0x1c, 0x42, 0x48/0x4c/0x50
 *   (mFireworkPos), 0x5c, 0x70, 0x80 (mPlayerNo), 0x84, 0x88 (with its
 *   get/inc accessor pair), 0x94 are independently re-confirmed against
 *   ALREADY-MATCHED compiled code in dol/bases/d_a_player_base.cpp,
 *   dol/bases/d_a_player.cpp, dol/bases/d_wipe_circle.cpp and
 *   dol/bases/d_wipe_dokan.cpp (disassembled straight out of
 *   bin/dtkspl/obj/dol/bases/*.o), not merely from this TU's own bytes.
 *
 * @note include/game/bases/d_a_player_demo_manager.hpp ALREADY EXISTS in this
 * repo and is used by the four already-matched files above. Its field offsets
 * agree with everything re-derived here everywhere they overlap. It does NOT
 * declare a virtual destructor or a vtable pointer, undercounts several real
 * fields as opaque `char` padding (0x18, the 0x20 array, 0x30 vector, 0x3c
 * timer, the 0x60 and 0x70 arrays, 0x8c, 0x90), and only models the ~10
 * methods its four callers need rather than the whole 46-method TU. This is
 * a genuine contradiction against this task's premise ("this is the blocking
 * prerequisite -- produce a header") and is reported to the lead rather than
 * silently resolved; see the proof report.
 *
 * @unofficial All member names except mFlags, mGoalType, mFireworkPos,
 * mPlayerNo are invented; `m_NN`-style names mark fields whose storage and
 * type are proven but whose game-semantic purpose is not. See the proof
 * report for which of the 46 methods have a confirmed vs. inferred return
 * type.
 */
class daPyDemoMng_c {
public:
    /// @unofficial Numeric values 0/1/4/5 are directly evidenced (external
    /// setDemoGoal passes 1; this TU's fn_8005CCD0 dispatcher passes 4 and
    /// 5). 2 and 3 are inferred by the obvious sequential gap and are NOT
    /// independently confirmed.
    enum Mode_e {
        MODE_0 = 0,
        MODE_1 = 1,
        MODE_2 = 2, ///< @unofficial inferred, not directly observed.
        MODE_3 = 3, ///< @unofficial inferred, not directly observed.
        MODE_4 = 4,
        MODE_5 = 5,
    };

    daPyDemoMng_c();
    virtual ~daPyDemoMng_c();

    void initStage();
    void initCourseIn();
    void init();
    void update();
    void setDemoMode(Mode_e mode, int param);
    void releaseDemoMode(int);
    bool isDemoMode(Mode_e mode) const;
    bool isDemoMode(Mode_e mode, int param) const;
    void deleteNotGoalPlayer();
    void calcNotGoalPlayer();
    int setGoalDemoList(int playerNo);
    bool isGoalAllEntryPlayer();
    void stopBgmGoalDemo();
    int getPoleBelowPlayer(int playerNo);
    void executeGoalDemo_Pole();
    void executeGoalDemo_PoleDown();
    void executeGoalDemo_JumpCheck();
    void executeGoalDemo_Jump();
    void executeGoalDemo_Land();
    void executeGoalDemo_KimeWait();
    void executeGoalDemo();
    void setGoalDemoKimeAll();
    void setGoalDemoRunCastle();
    bool isAllPlayerGoalIn();
    void setHanabiEffect();
    void executeGoalCastle();
    void calcGoalCenterPos();
    void setZoromeGoal();
    bool startControlDemoAll();      ///< Every path sets r3 via `srwi r3,rN,31` (a `neg|or` reduction), which is always exactly 0 or 1 -- confirmed bool.
    bool isAllPlayerControlDemo();   ///< Every exit path explicitly sets r3 to 0 or 1 (`li r3,0x0` / `li r3,0x1`) -- confirmed bool.
    void endControlDemoAll(int);
    int getControlDemoPlayerNum() const;
    void setBossDownPlayerNo(int playerNo);
    void onLandStopReq();
    void startControlDemoLandPlayer();
    bool isLandAll();
    void executeStartToride();
    void executeEndToride();
    void setCourseOutList(s8 playerNo);
    bool checkDemoNo(s8 playerNo);
    int getNextDemoNo();
    void turnNextDemoNo();
    void clearDemoNo(s8 playerNo);
    void setEnemyStageClearDemo(int);

    /// @brief 0x88 accessor pair, CONFIRMED byte-for-byte against
    /// dol/bases/d_a_player.cpp's `initializeState_DemoOutDoor`, which reads
    /// m_88 twice (once converted to float via the classic int-to-double
    /// magic-constant trick, once as the increment source) then stores
    /// m_88+1 back -- exactly matching `get_88() * 64.0f; inc_88();` at the
    /// two known call sites in d_a_player.cpp.
    int get_88() const { return m_88; }
    void inc_88() { m_88++; }

    int getPlrNo() const { return mPlayerNo; }
    void setPlrNo(int playerNo) { mPlayerNo = playerNo; }

private:
    // ------------------------------------------------------------------
    // Members, in offset order. Every offset in this list was read off a
    // store/load instruction, either in this TU's own disassembly or (where
    // noted) in an already-matched caller. None of it is a guess about
    // WHERE a field is -- only some of the NAMES are guesses.
    // ------------------------------------------------------------------

    Mode_e mMode;       ///< 0x04. setDemoMode's 1st param; isDemoMode compares against it; update() switches on it (cases 1,2,4,5 seen).
    int m_08;            ///< 0x08. setDemoMode's 2nd param; also reused as a step/sub-state counter through executeGoalDemo_*/executeGoalCastle/executeEndToride.
    int m_0c;            ///< 0x0c. Zeroed by init(); set to 10 in executeGoalDemo_Jump (countdown-style use).
    u32 mFlags;          ///< 0x10. CONFIRMED: dol/bases/d_a_player_base.cpp tests bit 2 (`& 4`) in setDemoGoal and bit 3 in executeDemoGoal_Pole.
    u32 mGoalType;       ///< 0x14. CONFIRMED: daPlBase_c::setDemoGoal stores its `u8 goalType` parameter here.
    u32 m_18;             ///< 0x18. Zeroed by init(); read/written in executeGoalDemo_Jump/JumpCheck. Real field, not padding.
    int mGoalEntryCount; ///< 0x1c. isGoalAllEntryPlayer() compares `daPyMng_c::getEntryNum()` against this; getPoleBelowPlayer() uses it as a loop bound over mGoalDemoList. (Called m_1c in the pre-existing include/ header.)
    int mGoalDemoList[4]; ///< 0x20. setGoalDemoList(playerNo) linear-scans this for a free (-1) slot and stores playerNo there, returning the index -- matches the method name exactly.
    mVec3_c mGoalCenterPos; ///< 0x30. Three floats, zeroed by init(); read/written by calcGoalCenterPos() -- matches the method name exactly.
    int mNotGoalPlayerTimer; ///< 0x3c. Set to 0x50 (80) by deleteNotGoalPlayer(), decremented every frame by calcNotGoalPlayer() until it hits 0.
    u8 m_40;              ///< 0x40. Byte, touched by executeGoalCastle/executeGoalDemo_Jump/setHanabiEffect/setZoromeGoal.
    u8 m_41;              ///< 0x41. As above.
    bool m_42;            ///< 0x42. CONFIRMED: daPlBase_c::setDemoGoal sets this to 1 when the boss-down branch is taken. (Same offset as the pre-existing include/ header's m_42.)
    bool mBgmStopped;     ///< 0x43. stopBgmGoalDemo() sets this to 1 immediately before telling SndSceneMgr to stop the goal BGM; init() clears it.
    u8 m_44;              ///< 0x44. Byte, touched by init()/setHanabiEffect.
    u8 m_45;              ///< 0x45. As above.
    mVec3_c mFireworkPos; ///< 0x48. CONFIRMED: daPlBase_c::setDemoGoal stores three floats here in sequence (x/y/z) computed from the goal-castle position. (Same offset as the pre-existing include/ header's mFireworkPos.)
    int m_54;             ///< 0x54. update() increments this once per frame while nonzero; also touched by executeGoalCastle/executeGoalDemo_Jump/JumpCheck.
    int m_58;             ///< 0x58. Only ever zeroed by init(); no other reference found in this TU.
    int m_5c;             ///< 0x5c. CONFIRMED: read by daPlBase_c::setDemoOutDokanAction and dAcPy_c::setDoorDemo. (Same offset as the pre-existing include/ header's m_5c.)
    int mDemoNoQueue[4]; ///< 0x60. A 4-slot FIFO: checkDemoNo() tests slot 0 only; turnNextDemoNo() shifts 1<-2<-3<-(-1). getNextDemoNo() reads slot 1 (0x64).
    int mCourseOutList[4]; ///< 0x70. setCourseOutList(playerNo) linear-scans this for a free (-1) slot exactly like mGoalDemoList does -- matches the method name. Slot 0 (offset 0x70) is independently read as a scalar by daPlBase_c::changeNextScene in the already-matched d_a_player_base.cpp, which is why the pre-existing include/ header modeled only "int m_70".
    int mPlayerNo;        ///< 0x80. CONFIRMED: daPlBase_c::setDemoGoal stores its own mPlayerNo here; setBossDownPlayerNo(int) also writes it (aliased with m_90). initStage() resets it to -1. (Same offset as the pre-existing include/ header's mPlayerNo.)
    int m_84;             ///< 0x84. CONFIRMED: read/written by dAcPy_c::stopOtherDownDemo/playOtherDownDemo in d_a_player.cpp. (Same offset as the pre-existing include/ header's m_84.)
    int m_88;             ///< 0x88. CONFIRMED, see get_88()/inc_88() above. (Same offset as the pre-existing include/ header's m_88.)
    int m_8c;             ///< 0x8c. Zeroed by init(); read/written by executeStartToride().
    int m_90;             ///< 0x90. Zeroed by init(); written together with mPlayerNo by setBossDownPlayerNo(int) (same value stored to both).
    int m_94;             ///< 0x94. CONFIRMED: read by daPlBase_c::checkSideViewLemit in d_a_player_base.cpp. (Same offset as the pre-existing include/ header's m_94.)
    // sizeof == 0x98 here.

public:
    static daPyDemoMng_c *mspInstance;
};
