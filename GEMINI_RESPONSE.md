# Gemini Response - Round 2

## Task A: Breaking the `startSystemSe` Overload Deadlock

### 1. Disassembly Audit of All Seven Call Sites

Each of the seven call sites was inspected in the disassembly of the original banked objects/binaries (`tools/dis/` and `bin/`):

1. **`source/dol/bases/d_a_player_base.cpp:3967`** (`daPlBase_c::executeDemoGoal_KimePose`)
   - Disassembly address: `0x80051A20` (`tools/dis/corpus_dol_bases_d_a_player_base.txt:15809`)
   - Target symbol: `bl startSystemSe__11SndAudioMgrFUiUl` (`.text:0x801954C0`)
   - Arguments passed: `r4 = 0x252` (`SE_OBJ_GOAL_GET_COIN_BONUS`), `r5 = 0x1`
   - Overload called: **`FUiUl`** (`unsigned int, unsigned long`)

2. **`source/dol/bases/d_pausewindow.cpp:357`** (`Pausewindow_c::executeState_PauseDisp`)
   - Disassembly address: `0x8015B124` (`tools/dis/corpus_dol_bases_d_pausewindow.txt:1084`)
   - Target symbol: `bl startSystemSe__11SndAudioMgrFUiUl` (`.text:0x801954C0`)
   - Arguments passed: `r4 = 0x78` (`SE_SYS_CURSOR`), `r5 = 0x1`
   - Overload called: **`FUiUl`** (`unsigned int, unsigned long`)

3. **`source/d_profileNP/bases/d_controller_information.cpp:87`** (`dControllerInformation_c::execute`)
   - Disassembly address: `.text:0x00000424` (`tools/dis/corpus_REL_d_profileNP_bases_d_controller_information.txt:255`)
   - Target symbol: `bl startSystemSe__11SndAudioMgrFUiUl` (`.text:0x801954C0`)
   - Arguments passed: `r4 = 0x85` (`SE_SYS_BUTTON_SKIP`), `r5 = 0x1`
   - Overload called: **`FUiUl`** (`unsigned int, unsigned long`)

4. **`source/d_profileNP/bases/d_yes_no_window.cpp:428`** (`dYesNoWindow_c::initializeState_OpenAnimeEndWait`)
   - Disassembly address: `.text:0x0000105C` (`tools/dis/corpus_REL_d_profileNP_bases_d_yes_no_window.txt:938`)
   - Target symbol: `bl startSystemSe__11SndAudioMgrFUiUl` (`.text:0x801954C0`)
   - Arguments passed: `r4 = 0x7F` (`SoundEffects[SOUND_OPEN_ANIME]`), `r5 = 0x1`
   - Overload called: **`FUiUl`** (`unsigned int, unsigned long`)

5. **`source/d_profileNP/bases/d_yes_no_window.cpp:529`** (`dYesNoWindow_c::executeState_SelectWait`)
   - Disassembly address: `.text:0x00001400` (`tools/dis/corpus_REL_d_profileNP_bases_d_yes_no_window.txt:1271`)
   - Target symbol: `bl startSystemSe__11SndAudioMgrFUiUl` (`.text:0x801954C0`)
   - Arguments passed: `r4 = 0x78` (`SoundEffects[SOUND_CURSOR_MOVE]`), `r5 = 0x1`
   - Overload called: **`FUiUl`** (`unsigned int, unsigned long`)

6. **`source/d_profileNP/bases/d_yes_no_window.cpp:551`** (`dYesNoWindow_c::initializeState_HitAnimeEndWait`)
   - Disassembly address: `.text:0x000014BC` (`tools/dis/corpus_REL_d_profileNP_bases_d_yes_no_window.txt:1335`)
   - Target symbol: `bl startSystemSe__11SndAudioMgrFUiUl` (`.text:0x801954C0`)
   - Arguments passed: `r4 = 0x79` (`SoundEffects[SOUND_WAIT]`), `r5 = 0x1`
   - Overload called: **`FUiUl`** (`unsigned int, unsigned long`)

7. **`source/d_profileNP/bases/d_yes_no_window.cpp:595`** (`dYesNoWindow_c::initializeState_ClouseAnimeEndWait`)
   - Disassembly address: `.text:0x000015F4` (`tools/dis/corpus_REL_d_profileNP_bases_d_yes_no_window.txt:1447`)
   - Target symbol: `bl startSystemSe__11SndAudioMgrFUiUl` (`.text:0x801954C0`)
   - Arguments passed: `r4 = 0x7A` (`SoundEffects[SOUND_CLOSE]`), `r5 = 0x1`
   - Overload called: **`FUiUl`** (`unsigned int, unsigned long`)

**Key Finding**: Every single one of the 7 existing call sites in the retail binary calls `startSystemSe__11SndAudioMgrFUiUl` (`0x801954C0`). None of them calls `startSystemSe__11SndAudioMgrFUlUl` (`0x801954B0`). The `FUlUl` overload is genuinely required elsewhere (e.g. `d_a_player_manager.cpp:setHipAttackQuake` at `0x80060D54`).

---

### 2. Analysis of the Options

- **Option 2 (Change declared parameter type)**: REJECTED.
  Parameter types are encoded in the CFront mangling (`Ui` = `unsigned int`, `Ul` = `unsigned long`). Changing the declared types would change the exported symbol names away from `startSystemSe__11SndAudioMgrFUiUl` and `startSystemSe__11SndAudioMgrFUlUl`, breaking the link with the retail DOL.

- **Option 3 (Root cause in passed argument types)**:
  - In `sound_list.h`, `enum SOUND_e` has underlying type `int` (promoted to `int`). In C++ overload resolution, converting `int` to `unsigned int` and converting `int` to `unsigned long` both have identical ranking (Standard Conversion rank: Conversion). Therefore, passing any uncasted enum or int literal to an overloaded function with `(unsigned int, unsigned long)` vs `(unsigned long, unsigned long)` will always fail with MWCC error 10199.
  - In `d_yes_no_window.cpp:289`, `SoundEffects` was declared as `const int SoundEffects[]`. Changing it to `const u32 SoundEffects[]`:
    1. Emits the exact same 16 bytes in `.rodata` (`0x7A, 0x79, 0x78, 0x7F`).
    2. Makes `SoundEffects[idx]` have type `u32` (`unsigned int`).
    3. `u32` is an EXACT match for `FUiUl` (no conversion) and requires conversion for `FUlUl`, resolving all four call sites in `d_yes_no_window.cpp` (lines 428, 529, 551, 595) to `FUiUl` unambiguously without adding any inline casts.

- **Option 1 (Explicit `(u32)` cast at enum call sites)**:
  For call sites 1, 2, 3 (`d_a_player_base.cpp:3967`, `d_pausewindow.cpp:357`, `d_controller_information.cpp:87`), casting the enum argument to `(u32)` provides an exact type match for `unsigned int`, resolving the overload to `FUiUl` with 0 ambiguity.

---

### 3. Proposed Changes

#### Target 1: `include/game/snd/snd_audio_mgr.hpp`
- **Evidence**: Retail DOL has both `startSystemSe__11SndAudioMgrFUiUl` (0x801954C0) and `startSystemSe__11SndAudioMgrFUlUl` (0x801954B0).
- **Proposal**:
```diff
--- a/include/game/snd/snd_audio_mgr.hpp
+++ b/include/game/snd/snd_audio_mgr.hpp
@@ -6,14 +6,8 @@
 class SndAudioMgr {
 public:
-    /// @note The map has TWO overloads: startSystemSe__11SndAudioMgrFUiUl
-    /// (0x801954C0), declared here, and startSystemSe__11SndAudioMgrFUlUl
-    /// (0x801954B0), which is NOT declared. Declaring both makes all seven
-    /// existing call sites ambiguous -- they pass enum/int-typed constants
-    /// that convert equally well to `unsigned int` and `unsigned long`, and
-    /// MWCC rejects them (error 10199). Adding the second overload therefore
-    /// requires first establishing each existing call site's true argument
-    /// type, which is its own piece of work. Tried and reverted; recorded so
-    /// the next person does not repeat it. @unofficial
     void startSystemSe(unsigned int soundID, unsigned long);
+    void startSystemSe(unsigned long soundID, unsigned long);
     u32 get3DCtrlFlag(unsigned long);
```
- **Compiled**: YES (tested with mock include in `scratch/`).
- **Confidence**: High.
- **Offset-perturbing**: NO (adds member function declaration, no member variables changed).

#### Target 2: `syms.txt`
- **Evidence**: `0x801954B0` in DOL.
- **Proposal**:
```diff
--- a/syms.txt
+++ b/syms.txt
@@ -583,2 +583,3 @@
 startSystemSe__11SndAudioMgrFUiUl=0x801954C0
+startSystemSe__11SndAudioMgrFUlUl=0x801954B0
 setSoundPosition__11SndAudioMgrFPQ34nw4r3snd11SoundHandleRCQ34nw4r4math4VEC2=0x801962D0
```
- **Confidence**: High.
- **Offset-perturbing**: NO.

#### Target 3: Call Site Updates in Banked Units
1. **`source/dol/bases/d_a_player_base.cpp:3967`**:
```diff
-                        SndAudioMgr::sInstance->startSystemSe(SE_OBJ_GOAL_GET_COIN_BONUS, 1);
+                        SndAudioMgr::sInstance->startSystemSe((u32)SE_OBJ_GOAL_GET_COIN_BONUS, 1);
```
2. **`source/dol/bases/d_pausewindow.cpp:357`**:
```diff
-            SndAudioMgr::sInstance->startSystemSe(SE_SYS_CURSOR, 1);
+            SndAudioMgr::sInstance->startSystemSe((u32)SE_SYS_CURSOR, 1);
```
3. **`source/d_profileNP/bases/d_controller_information.cpp:87`**:
```diff
-                SndAudioMgr::sInstance->startSystemSe(SE_SYS_BUTTON_SKIP, 1);
+                SndAudioMgr::sInstance->startSystemSe((u32)SE_SYS_BUTTON_SKIP, 1);
```
4. **`source/d_profileNP/bases/d_yes_no_window.cpp:289`**:
```diff
-const int SoundEffects[] = { SE_SYS_BACK, SE_SYS_DECIDE, SE_SYS_CURSOR, SE_SYS_DIALOGUE_IN };
+const u32 SoundEffects[] = { SE_SYS_BACK, SE_SYS_DECIDE, SE_SYS_CURSOR, SE_SYS_DIALOGUE_IN };
```
(No edits needed at lines 428, 529, 551, 595 of `d_yes_no_window.cpp` because `SoundEffects` indexing naturally yields `u32`).

- **Emitted Code Impact**: **ZERO change to emitted bytes**.
  Every single call site produces bitwise identical machine instructions (`li r4, 0x...`, `li r5, 0x1`, `bl startSystemSe__11SndAudioMgrFUiUl`), and `SoundEffects` produces the exact same 16 bytes in `.rodata`. Verified by compiling each TU in `scratch/` and diffing the disassembled instruction streams against the current landed objects.

---

## Task B: Scout the Next Translation Unit

Using `bin/dtk/dtk_splits_wiimj2d.txt` and `bin/dtk/wiimj2d_symbols.txt`, all gaps between landed TUs were surveyed and hard-bracketed against adjacent split units.

### Ranked Shortlist (Top 3 Candidates)

#### Rank 1: `dol/bases/d_nand_thread.cpp` (`dNandThread_c`)
- **Section Bounds (derived from `dtk_splits_wiimj2d.txt`)**:
  - `.text`: `0x800CED00` - `0x800CFCE0` (size: `0xFE0` = 4,064 B)
    - Left `.text` bracket: `dol/bases/d_multi_manager.cpp` (ends `0x800CED00`)
    - Right `.text` bracket: `dol/bases/d_next.cpp` (starts `0x800CFCE0`)
  - `.sbss`: `0x8042A298` - `0x8042A2A0` (size: 8 B, `m_instance__13dNandThread_c`)
    - Left `.sbss` bracket: `dol/bases/d_multi_manager.cpp` (ends `0x8042A298`)
    - Right `.sbss` bracket: `dol/bases/d_next.cpp` (starts `0x8042A2A0`)
  - `.sdata2`: `0x8042CC98` - `0x8042CC98` (0 B)
- **Metrics**:
  - Function count: **24 functions**
  - Total text bytes: **4,064 B**
  - Bytes per function: **169.3 B/fn** (very clean, consistent function size)
  - `__sinit` count: **0** (no static constructor ordering hazards)
  - Vtables in map: **YES** — `__vt__13dNandThread_c` @ `0x80317D48` and `__vt__6mMutex` @ `0x80317D60`
- **Why recommended**: Perfect hard-bracketing on both boundaries across `.text` and `.sbss`. Clean thread wrapper class with no template explosion and no static initializers.
- **Risk / What could go wrong**: Calls Nintendo NAND SDK functions (`NANDInit`, `NANDCreate`, `NANDWrite`, etc.). If SDK headers in `include/lib/revolution/nand/` have missing prototypes, minor header declarations may be required.

---

#### Rank 2: `dol/mLib/m_pad.cpp` (`mPad`)
- **Section Bounds (derived from `dtk_splits_wiimj2d.txt`)**:
  - `.text`: `0x8016F330` - `0x80170AC0` (size: `0x1790` = 6,032 B)
    - Left `.text` bracket: `dol/mLib/m_mtx.cpp` (ends `0x8016F330`)
    - Right `.text` bracket: `dol/mLib/m_vec.cpp` (starts `0x80170AC0`)
  - `.ctors`: `0x802EDEFC` - `0x802EDF00` (size: 4 B)
    - Left: `m_mtx.cpp` (ends `0x802EDEFC`), Right: `m_vec.cpp` (starts `0x802EDF00`)
  - `.bss`: `0x80377F88` - `0x803780C8` (size: `0x140` = 320 B)
    - Left: `m_mtx.cpp` (ends `0x80377F88`), Right: `m_vec.cpp` (starts `0x803780C8`)
  - `.sbss`: `0x8042A740` - `0x8042A760` (size: `0x20` = 32 B)
  - `.sdata2`: `0x8042E010` - `0x8042E030` (size: `0x20` = 32 B)
    - Left: `m_mtx.cpp` (ends `0x8042E010`), Right: `m_vec.cpp` (starts `0x8042E030`)
- **Metrics**:
  - Function count: **56 functions**
  - Total text bytes: **6,032 B**
  - Bytes per function: **107.7 B/fn** (short, direct register/SDK accessors)
  - `__sinit` count: **1** (`__sinit_\m_pad_cpp` at `0x80170A50`)
  - Vtables in map: **None** (pure static utility class `mPad`)
- **Why recommended**: Every single section (.text, .ctors, .bss, .sbss, .sdata2) is hard-bracketed on both sides between `m_mtx.cpp` and `m_vec.cpp`.
- **Risk / What could go wrong**: Depends on `WPADInfo` / `WPADStatus` Revolution SDK structs; needs accurate member offset layout in `m_pad.hpp`.

---

#### Rank 3: `dol/bases/d_a_mask.cpp` (`daMask_c`)
- **Section Bounds (derived from `dtk_splits_wiimj2d.txt`)**:
  - `.text`: `0x80124EB0` - `0x80126650` (size: `0x17A0` = 6,048 B)
    - Left `.text` bracket: `dol/bases/d_last_actor.cpp` (ends `0x80124EB0`)
    - Right `.text` bracket: `dol/bases/d_a_player.cpp` (starts `0x80126650`)
  - `.data`: `0x80324D78` - `0x80324E58` (size: `0xE0` = 224 B, contains `__vt__8daMask_c` @ `0x80324D84`)
  - `.bss`: `0x80375F08` - `0x80375F20` (size: 24 B, `l_flashlightA`, `l_flashlightB`)
  - `.sbss`: `0x8042A5E8` - `0x8042A5F0` (size: 8 B, guard words)
- **Metrics**:
  - Function count: **29 functions**
  - Total text bytes: **6,048 B**
  - Bytes per function: **208.6 B/fn**
  - `__sinit` count: **0**
  - Vtables in map: **YES** (`__vt__8daMask_c` @ `0x80324D84`)
- **Why recommended**: Hard-bracketed between `d_last_actor.cpp` and `d_a_player.cpp`. Single actor class with verified vtable in symbol map.
- **Risk / What could go wrong**: Moderate 2D/3D matrix and lighting calculations (`calcDarkHandLight`, `drawCaveMask`).

---

### Rejected Candidates (with Analysis)

1. **`0x800CA150`–`0x800CD800` (`d_mask_draw.cpp` / `d_mask_mng.cpp` / `dMarioMdl_c`)**:
   - 14,000 B across 104 functions.
   - Contains 2 distinct `__sinit` functions (`__sinit_\d_mask_draw_cpp` and `__sinit_\d_mask_mng_cpp`) plus `dMarioMdl_c`.
   - **Reason for rejection**: Multi-TU composite region where internal TU boundaries are invisible and must be manually derived.

2. **`0x800E2070`–`0x800E46E0` (`d_screen.cpp` / `d_stage.cpp` / `dStageTimer_c` / `dScoreMng_c`)**:
   - 9,840 B across 61 functions.
   - Contains 2 `__sinit`s (`d_screen_cpp` and `d_stage_cpp`) spanning at least 4 separate logical units.
   - **Reason for rejection**: Internal boundaries between `dScoreMng_c`, `dStageTimer_c`, and `d_stage.cpp` are not separated in `dtk_splits_wiimj2d.txt`.

3. **`0x8010D270`–`0x8010F080` (`d_WarningManager.cpp`)**:
   - 7,696 B across 67 functions.
   - Single TU with 1 `__sinit`, but has 17 template-instantiated state machine methods (`sStateMgr_c<dWarningManager_c...>`, `sFStateID_c<dWarningManager_c>`).
   - **Reason for rejection**: Template instantiation ordering and inlining in MWCC creates high risk of subtle register allocation or symbol ordering diffs.
