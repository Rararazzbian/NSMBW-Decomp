# Gemini Response — Round 1

All five targets investigated and verified against symbol tables, disassembly of original object files, and test compilation with CodeWarrior (`mwcceppc.exe`).
The proposed header files have been written to `scratch/` with clean LF line endings and no UTF-8 BOM.

---

## 1. PauseManager_c

- **Real class name**: `PauseManager_c`
  - Recovered from: `setPauseEnable__14PauseManager_cFb` (`.text:0x800D0C10`, size `0x28`) and 20 other symbols in `bin/dtk/wiimj2d_symbols.txt`.
  - Length prefix: `14` (`14` chars = `PauseManager_c`).
- **Evidence**:
  - `PauseManager_c` lives between `d_pad.cpp` and `d_pc.cpp` (address range `0x800D0A90`..`0x800D15F0`), corresponding to source file `d_pause_manager.cpp`.
  - Vtable symbol: `__vt__14PauseManager_c` at `.data:0x80317D78` (size `0xC`, containing virtual `~PauseManager_c()`).
  - Method called by `daPyMng_c::update()`: `setPauseEnable__14PauseManager_cFb` at `0x800D0C10` (`void setPauseEnable(bool enable)`). Disassembly confirms:
    ```assembly
    /* 800D0C10 */ cmpwi r4, 0
    /* 800D0C14 */ bne .L_800D0C28
    /* 800D0C18 */ lbz r0, 0x18(r3)
    /* 800D0C1C */ ori r0, r0, 0x2
    /* 800D0C20 */ stb r0, 0x18(r3)
    /* 800D0C24 */ blr
    .L_800D0C28:
    /* 800D0C28 */ lbz r0, 0x18(r3)
    /* 800D0C2C */ andi. r0, r0, 0xfd
    /* 800D0C30 */ stb r0, 0x18(r3)
    /* 800D0C34 */ blr
    ```
  - Static members in `.sbss`:
    - `m_instance__14PauseManager_c` (`.sbss:0x8042A2B8`, 4 bytes, `PauseManager_c *m_instance`)
    - `m_OtasukeInfo_p__14PauseManager_c` (`.sbss:0x8042A2BC`, 4 bytes)
    - `m_Pause__14PauseManager_c` (`.sbss:0x8042A2C0`, 1 byte)
    - `m_Created__14PauseManager_c` (`.sbss:0x8042A2C1`, 1 byte)
    - `m_OtasukeAfter__14PauseManager_c` (`.sbss:0x8042A2C2`, 1 byte)
  - Full method table recovered from symbols and verified via disassembly in `auto_03_800D09F8_text.o`:
    - `PauseManager_c()` (`__ct__14PauseManager_cFv`, `0x800D0A90`, size `0x24`)
    - `virtual ~PauseManager_c()` (`__dt__14PauseManager_cFv`, `0x800D0AC0`, size `0x4C`)
    - `void CourseHoinitialize()` (`CourseHoinitialize__14PauseManager_cFv`, `0x800D0B10`, size `0x20`)
    - `void initialize()` (`initialize__14PauseManager_cFv`, `0x800D0B30`, size `0x2C`)
    - `void execute()` (`execute__14PauseManager_cFv`, `0x800D0B60`, size `0xA4`)
    - `void setPauseEnable(bool enable)` (`setPauseEnable__14PauseManager_cFb`, `0x800D0C10`, size `0x28`)
    - `bool isDisable()` (`isDisable__14PauseManager_cFv`, `0x800D0C40`, size `0xC`)
    - `void ProcMainInit()` (`ProcMainInit__14PauseManager_cFv`, `0x800D0C50`, size `0x90`)
    - `void ProcMainPauseOn()` (`ProcMainPauseOn__14PauseManager_cFv`, `0x800D0CE0`, size `0xC`)
    - `void SelectSoundSet(int sound)` (`SelectSoundSet__14PauseManager_cFi`, `0x800D0CF0`, size `0xA8`)
    - `void KeyChack()` (`KeyChack__14PauseManager_cFv`, `0x800D0DA0`, size `0x194`)
    - `void ProcMainPause()` (`ProcMainPause__14PauseManager_cFv`, `0x800D0F40`, size `0x34`)
    - `void ProcMainPauseOffInit()` (`ProcMainPauseOffInit__14PauseManager_cFv`, `0x800D0F80`, size `0x90`)
    - `void ProcMainPauseOff()` (`ProcMainPauseOff__14PauseManager_cFv`, `0x800D1010`, size `0xB4`)
    - `void CourseOutConfirmation()` (`CourseOutConfirmation__14PauseManager_cFv`, `0x800D10D0`, size `0x11C`)
    - `void ConfirmationSelectDecisionWait()` (`ConfirmationSelectDecisionWait__14PauseManager_cFv`, `0x800D11F0`, size `0x90`)
    - `void OtasukeDisp()` (`OtasukeDisp__14PauseManager_cFv`, `0x800D1280`, size `0x54`)
    - `void PauseSetUp(int playerNo)` (`PauseSetUp__14PauseManager_cFi`, `0x800D12E0`, size `0x64`)
    - `void setPause()` (`setPause__14PauseManager_cFv`, `0x800D1350`, size `0x250`)
    - `void onDispOtasukeWindow()` (`onDispOtasukeWindow__14PauseManager_cFv`, `0x800D15A0`, size `0x24`)
    - `bool isOtasukePause()` (`isOtasukePause__14PauseManager_cFv`, `0x800D15D0`, size `0x1C`)
- **Proposed header**: `scratch/d_pause_manager.hpp` (New file for `include/game/bases/d_pause_manager.hpp`)
```cpp
#pragma once

#include <types.h>

/// @brief Manages stage pause state and course-out confirmation.
/// @ingroup bases
class PauseManager_c {
public:
    PauseManager_c();
    virtual ~PauseManager_c();

    void CourseHoinitialize();
    void initialize();
    void execute();
    void setPauseEnable(bool enable);
    bool isDisable();
    void ProcMainInit();
    void ProcMainPauseOn();
    void SelectSoundSet(int sound);
    void KeyChack();
    void ProcMainPause();
    void ProcMainPauseOffInit();
    void ProcMainPauseOff();
    void CourseOutConfirmation();
    void ConfirmationSelectDecisionWait();
    void OtasukeDisp();
    void PauseSetUp(int playerNo);
    void setPause();
    void onDispOtasukeWindow();
    bool isOtasukePause();

    int mState;         ///< Current execution state in MainProc_tbl. @unofficial
    int mUnk08;         ///< @unofficial
    int mUnk0C;         ///< @unofficial
    int mUnk10;         ///< @unofficial
    int mUnk14;         ///< @unofficial
    u8 mFlags;          ///< Bit 0: pause active, Bit 1: pause disabled. @unofficial
    u8 mUnk19;          ///< @unofficial
    u8 mUnk1A;          ///< @unofficial
    u8 mUnk1B;          ///< @unofficial
    u8 mUnk1C;          ///< @unofficial
    u8 mUnk1D;          ///< @unofficial
    u8 mPad1E[2];       ///< Padding to 0x20 alignment. @unofficial

    static PauseManager_c *m_instance;  ///< [.sbss:0x8042A2B8] Singleton instance.
    static void *m_OtasukeInfo_p;       ///< [.sbss:0x8042A2BC] Super Guide info pointer. @unofficial
    static u8 m_Pause;                  ///< [.sbss:0x8042A2C0] Pause active flag.
    static u8 m_Created;                ///< [.sbss:0x8042A2C1] Instance created flag.
    static u8 m_OtasukeAfter;           ///< [.sbss:0x8042A2C2] After Super Guide flag.
};
```
- **Confidence**: High. Every symbol name, length prefix, parameter type, and register usage matches the binary disassembly.
- **Offset-perturbing for existing TUs?**: **NO**. This is a brand new header; no currently matched TU includes it.

---

## 2. dScStage_c::getGameDisplay()

- **Real mangled symbol**: `getGameDisplay__10dScStage_cFv` (`.text:0x80101A70`, size `0x28`)
- **Static vs Member**: **Static accessor over singleton**.
- **Inlined vs Out-of-line**: **Out-of-line** (defined in `d_s_stage.cpp`, called with `bl getGameDisplay__10dScStage_cFv` across multiple TUs including `d_a_player_manager.cpp`, `d_pause_manager.cpp`, `d_stage_timer.cpp`). It must NOT have an inline body in `d_s_stage.hpp`.
- **Evidence**:
  - Disassembly of `0x80101A70` (`auto_03_801018A0_text.o`):
    ```assembly
    .fn getGameDisplay__10dScStage_cFv, global
    /* 80101A70 */ lwz r3, m_instance__10dScStage_c@sda21(r0)
    /* 80101A74 */ cmpwi r3, 0
    /* 80101A78 */ bne .L_80101A84
    /* 80101A7C */ li r3, 0
    /* 80101A80 */ blr
    .L_80101A84:
    /* 80101A84 */ lwz r3, 0x11d4(r3)
    /* 80101A88 */ cmpwi r3, 0
    /* 80101A8C */ bnelr
    /* 80101A90 */ li r3, 0
    /* 80101A94 */ blr
    .endfn getGameDisplay__10dScStage_cFv
    ```
  - Call site in `daPyMng_c::update()` (`0x8005F5DC`..`0x8005F5E4`):
    ```assembly
    /* 8005F5DC */ bl checkLastAlivePlayer__9daPyMng_cFv
    /* 8005F5E0 */ bl getGameDisplay__10dScStage_cFv
    /* 8005F5E4 */ cmpwi r3, 0x0
    /* 8005F5E8 */ mr r30, r3
    ```
    No `r3` argument is passed. It directly returns `dGameDisplay_c *` (loaded from `m_instance->mpGameDisplay` at `+0x11D4`).
- **Proposed header diff**: `scratch/d_s_stage.hpp`
```diff
--- include/game/bases/d_s_stage.hpp
+++ scratch/d_s_stage.hpp
@@ -4,6 +4,8 @@
 #include <game/bases/d_fader.hpp>
 #include <game/mLib/m_vec.hpp>
 #include <constants/game_constants.h>
+
+class dGameDisplay_c;
 
 class dScStage_c : public dScene_c {
 public:
@@ -57,6 +59,11 @@
     /// only copy of the symbol in the binary. @unofficial
     static NOINLINE bool getCourseIn() { return m_isCourseIn; }
 
+    /// @brief Gets the HUD display manager for the stage scene.
+    /// @note Out-of-line static accessor over `m_instance->mpGameDisplay` (+0x11D4).
+    /// Defined in `d_s_stage.cpp` at `0x80101A70`.
+    static dGameDisplay_c *getGameDisplay();
+
     static float getLoopPosX(float x);
     /// @brief [.sbss:0x8042A4D0] Pointer into the "otehon" (demo playback)
     /// clear-flag block; indexed with byte loads/stores at +0xb5..+0xb9.
```
- **Confidence**: High. Symbol name, out-of-line linkage, and register usage confirmed byte-for-byte in disassembly.
- **Offset-perturbing for existing TUs?**: **NO**. Declaring a non-virtual static member function adds no fields to `dScStage_c` and does not change `sizeof(dScStage_c)` or any member offsets.

---

## 3. Four dGameDisplay_c Methods

- **Mangled names, signatures, and return types**:
  1. `setPlayNum`:
     - Mangled: `setPlayNum__14dGameDisplay_cFPi` (`.text:0x801599C0`, size `0xE0`)
     - Signature: `void setPlayNum(int *playNum);`
     - Return type: `void` (disassembly at `0x80159A9C` restores non-volatile registers and executes `blr` with no value loaded to `r3`).
  2. `setCoinNum`:
     - Mangled: `setCoinNum__14dGameDisplay_cFi` (`.text:0x80159AA0`, size `0x15C`)
     - Signature: `void setCoinNum(int coinNum);`
     - Return type: `void` (disassembly at `0x80159BF8` restores registers and executes `blr`).
  3. `setScore`:
     - Mangled: `setScore__14dGameDisplay_cFi` (`.text:0x80159DF0`, size `0x64`)
     - Signature: `void setScore(int score);`
     - Return type: `void` (disassembly at `0x80159E4C` tail-calls `LayoutDispNumber` / `blr`).
  4. `setCollect`:
     - Mangled: `setCollect__14dGameDisplay_cFv` (`.text:0x80159C30`, size `0x1BC`)
     - Signature: `void setCollect();`
     - Return type: `void` (disassembly at `0x80159DE8` restores registers and executes `blr`).
- **Evidence**:
  - `daPyMng_c::update()` disassembly (`0x8005F5F0`..`0x8005F63C`):
    ```assembly
    /* 8005F5F0 */ addi r7, r31, 0x80
    /* 8005F5F4 */ lwz r8, 0x80(r31)
    /* 8005F5F8 */ lwz r6, 0x4(r7)
    /* 8005F5FC */ addi r4, r1, 0x8
    /* 8005F600 */ lwz r5, 0x8(r7)
    /* 8005F604 */ lwz r0, 0xc(r7)
    /* 8005F608 */ stw r8, 0x8(r1)
    /* 8005F60C */ stw r6, 0xc(r1)
    /* 8005F610 */ stw r5, 0x10(r1)
    /* 8005F614 */ stw r0, 0x14(r1)
    /* 8005F618 */ bl setPlayNum__14dGameDisplay_cFPi
    /* 8005F61C */ bl getCoinAll__9daPyMng_cFv
    /* 8005F620 */ mr r4, r3
    /* 8005F624 */ mr r3, r30
    /* 8005F628 */ bl setCoinNum__14dGameDisplay_cFi
    /* 8005F62C */ lwz r4, mScore__9daPyMng_c@sda21(r0)
    /* 8005F630 */ mr r3, r30
    /* 8005F634 */ bl setScore__14dGameDisplay_cFi
    /* 8005F638 */ mr r3, r30
    /* 8005F63C */ bl setCollect__14dGameDisplay_cFv
    ```
- **Proposed header diff**: `scratch/d_game_display.hpp`
```diff
--- include/game/bases/d_game_display.hpp
+++ scratch/d_game_display.hpp
@@ -1,6 +1,18 @@
 #pragma once
+#include <types.h>
 
 class dGameDisplay_c {
 public:
+    void setPlayNum(int *playNum);
+    void setCoinNum(int coinNum);
+    void setTime(int time);
+    void setCollect();
+    void setScore(int score);
+
     static const int c_PLAYNUM_DIGIT;
+    static const int c_COINNUM_DIGIT;
+    static const int c_TIME_DIGIT;
+    static const int c_SCORE_DIGIT;
+
+    static dGameDisplay_c *m_instance;
 };
```
- **Confidence**: High. Mangled names, parameter counts, argument types, and `void` return types verified directly in disassembly and symbol table.
- **Offset-perturbing for existing TUs?**: **NO**. Adding non-virtual member functions and static member declarations does not alter memory layouts of any object.

---

## 4. dStageTimer_c, Field at Offset 0xC

- **Field name & type**: `bool mStopped;` (or `u8 mStopped;`)
- **Offset**: `+0x0C`
- **Width**: `1 byte`
- **Total sizeof(dStageTimer_c)**: `0x10` (16 bytes)
- **Evidence**:
  - `dStageTimer_c` constructor (`__ct__13dStageTimer_cFs` at `0x800E38E0`):
    ```assembly
    /* 800E38E0 */ lis r6, __vt__13dStageTimer_c@ha
    /* 800E38E4 */ li r0, 0
    /* 800E38E8 */ addi r6, r6, __vt__13dStageTimer_c@l
    /* 800E38EC */ slwi r5, r4, 12             # (time << 12)
    /* 800E38F0 */ stw r6, 0x0(r3)             # vtable pointer (offset +0x00)
    /* 800E38F4 */ stw r5, 0x4(r3)             # mTimeValue (offset +0x04)
    /* 800E38F8 */ sth r4, 0x8(r3)             # mStartTimer (offset +0x08, s16)
    /* 800E38FC */ stb r0, 0xa(r3)             # mTimeUp (offset +0x0A, u8)
    /* 800E3900 */ stb r0, 0xb(r3)             # mHurryUpSoundPlayed (offset +0x0B, u8)
    /* 800E3904 */ stb r0, 0xc(r3)             # mStopped (offset +0x0C, u8/bool)
    /* 800E3908 */ stw r3, m_instance__13dStageTimer_c@sda21(r0)
    /* 800E390C */ blr
    ```
  - `createInstance` at `0x800E398C` calls `__nw__FUl(0x10)` (`sizeof = 0x10`).
  - `daPyMng_c::update()` writes `mStopped` with a byte store (`stb r0, 0xc(r3)`) at `0x8005F834` and `0x8005F844`.
  - `dStageTimer_c::execute()` reads `mStopped` with `lbz r0, 0xc(r31)` at `0x800E3AA0` to inhibit decrementing the timer.
- **Proposed header diff**: `scratch/d_stage_timer.hpp`
```diff
--- include/game/bases/d_stage_timer.hpp
+++ scratch/d_stage_timer.hpp
@@ -5,7 +5,12 @@
 public:
     virtual ~dStageTimer_c() {}
 
-    int mTimeValue;
+    int mTimeValue;                 ///< [+0x04] Fixed-point countdown timer (IGT << 12).
+    s16 mStartTimer;                ///< [+0x08] Initial time value in seconds. @unofficial
+    u8 mTimeUp;                     ///< [+0x0A] Time up flag. @unofficial
+    u8 mHurryUpSoundPlayed;         ///< [+0x0B] Hurry up sound played flag (time <= 100). @unofficial
+    bool mStopped;                  ///< [+0x0C] Timer freeze/stopped flag. Written by daPyMng_c::update().
+    u8 mPad0D[3];                   ///< [+0x0D] Padding to 0x10. @unofficial
 
     short convertToIGT() const {
         return (mTimeValue + 4095) >> 12;
```
- **Confidence**: High. Offset `+0x0C`, width `1 byte`, and total size `0x10` are confirmed directly by the constructor and allocation disassembly.
- **Offset-perturbing for existing TUs?**: **NO**. Existing callers (`d_a_player_base.cpp` and `d_a_player_demo_manager.cpp`) only invoke `convertToIGT()`, which accesses `mTimeValue` at offset `0x04`. Its offset remains unchanged.

---

## 5. Three Missing Bits in dQuake_c::FLAGS_e

- **The three bits**:
  - `FLAG_3 = BIT_FLAG(3)` = `0x08`: Unconditional player small quake hop (triggers `STATUS_QUAKE_SMALL` / `0x8C` on players).
  - `FLAG_4 = BIT_FLAG(4)` = `0x10`: Timed player small quake hop (triggers `STATUS_QUAKE_SMALL` / `0x8C` on players when `m_quakeTimer[i] == 0`).
  - `FLAG_5 = BIT_FLAG(5)` = `0x20`: Player big quake shock / stun (triggers `STATUS_QUAKE_BIG` / `0x8B` on players).
- **Mask `0x38`**:
  `0x38 == 0x20 | 0x10 | 0x08 == FLAG_5 | FLAG_4 | FLAG_3`.
- **Evidence**:
  - `daPyMng_c::update()` disassembly (`0x8005F750`..`0x8005F7C8`):
    ```assembly
    /* 8005F750 */ lwz r3, m_instance__8dQuake_c@sda21(r0)
    /* 8005F754 */ lwz r0, 0x30(r3)            # r0 = mFlags (offset +0x30)
    /* 8005F758 */ rlwinm. r0, r0, 0, 26, 28   # test mask 0x38 (bits 26..28 = 0x20 | 0x10 | 0x08)
    /* 8005F75C */ beq .L_8005F7E4             # if (mFlags & 0x38 == 0) skip
    ...
    /* 8005F788 */ lwz r4, 0x30(r4)            # r4 = mFlags
    /* 8005F78C */ rlwinm. r0, r4, 0, 26, 26   # test bit 26 (0x20 / FLAG_5)
    /* 8005F790 */ beq .L_8005F7A0
    /* 8005F794 */ li r4, 0x8b                 # onStatus(STATUS_QUAKE_BIG / 0x8B)
    /* 8005F798 */ bl onStatus__10daPlBase_cFi
    /* 8005F79C */ b .L_8005F7C8
    .L_8005F7A0:
    /* 8005F7A0 */ rlwinm. r0, r4, 0, 28, 28   # test bit 28 (0x08 / FLAG_3)
    /* 8005F7A4 */ beq .L_8005F7B4
    /* 8005F7A8 */ li r4, 0x8c                 # onStatus(STATUS_QUAKE_SMALL / 0x8C)
    /* 8005F7AC */ bl onStatus__10daPlBase_cFi
    /* 8005F7B0 */ b .L_8005F7C8
    .L_8005F7B4:
    /* 8005F7B4 */ lwz r0, 0x0(r30)            # check if daPyMng_c::m_quakeTimer[i] == 0
    /* 8005F7B8 */ cmpwi r0, 0x0               # (active when 0x10 / FLAG_4 is set)
    /* 8005F7BC */ bne .L_8005F7C8
    /* 8005F7C0 */ li r4, 0x8c                 # onStatus(STATUS_QUAKE_SMALL / 0x8C)
    /* 8005F7C4 */ bl onStatus__10daPlBase_cFi
    ```
  - Cross-checked with `d_a_player_base.hpp`:
    - `STATUS_QUAKE_BIG = 0x8B` (Line 487)
    - `STATUS_QUAKE_SMALL = 0x8C` (Line 488)
- **Proposed header diff**: `scratch/d_quake.hpp`
```diff
--- include/game/bases/d_quake.hpp
+++ scratch/d_quake.hpp
@@ -18,9 +18,12 @@
     };
 
     enum FLAGS_e {
-        FLAG_0 = BIT_FLAG(0),
-        FLAG_1 = BIT_FLAG(1),
-        FLAG_2 = BIT_FLAG(2)
+        FLAG_0 = BIT_FLAG(0), ///< 0x01: Screen/enemy ground shake (dEn_c::quakeAction).
+        FLAG_1 = BIT_FLAG(1), ///< 0x02: POW shock kill (set while mPOWLength > 0).
+        FLAG_2 = BIT_FLAG(2), ///< 0x04: Mega Ground Pound / Super POW shock kill (set while mMPGPLength > 0).
+        FLAG_3 = BIT_FLAG(3), ///< 0x08: Player small quake hop (unconditional, triggers STATUS_QUAKE_SMALL 0x8C). @unofficial
+        FLAG_4 = BIT_FLAG(4), ///< 0x10: Player small quake hop (triggers STATUS_QUAKE_SMALL 0x8C if m_quakeTimer == 0). @unofficial
+        FLAG_5 = BIT_FLAG(5), ///< 0x20: Player big quake stun (triggers STATUS_QUAKE_BIG 0x8B). @unofficial
     };
 
     void shockMotor(s8, TYPE_SHOCK_e, int, bool);
```
- **Confidence**: High. The bit positions `3`, `4`, `5` and corresponding player status actions match `update()` bitmask checks and player status definitions exactly.
- **Offset-perturbing for existing TUs?**: **NO**. `FLAG_0`, `FLAG_1`, `FLAG_2` retain their exact bit values (used in `d_enemy.cpp`), `FLAGS_e` remains a 32-bit enum (`-enum int`), and `mFlags` stays at offset `0x30`.

---

## Status Summary

All 5 gaps are resolved with byte-exact evidence from the disassembly, fully compiled with CodeWarrior 1.1, and ready for Claude to land.
I have finished round 1 early and am ready for further work orders.
