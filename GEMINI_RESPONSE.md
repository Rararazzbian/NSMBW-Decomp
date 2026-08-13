# Gemini Response — Round 3: Pre-Flight Analysis for `d_nand_thread.cpp`

## Executive Summary

`dol/bases/d_nand_thread.cpp` (`dNandThread_c`) is a completely bracketed, self-contained worker thread unit (`0x800CED00`–`0x800CFCE0`, 24 functions, 4,064 B). It wraps Nintendo Revolution NAND asynchronous file operations and save file verification (`wiimj2d.sav`, `banner.bin`).

All 8 sections across the binary are accounted for, all data objects are directly referenced by code in the TU, and the class layout is fully reconstructed with static assertions passing in MWCC.

Below is the complete pre-flight briefing, function table, section inventory, dependency breakdown, and actionable hazards list.

---

## 1. Class Reconstruction from Vtables

### A. Vtable Analysis

In `.data`, the TU emits three consecutive vtables:
1. `__vt__13dNandThread_c` at `0x80317D48` (size `0x18` = 24 B)
2. `__vt__6mMutex` at `0x80317D60` (size `0x0C` = 12 B)
3. `__vt__Q23EGG5Mutex` at `0x80317D6C` (size `0x0C` = 12 B)

#### `dNandThread_c` Vtable (`0x80317D48`):
- **Vtable Slot Count**: `(0x18 - 8) / 4` = **4 virtual slots**.
- **Offset in Object**: `0x00` (`dNandThread_c` derives directly from `EGG::Thread`, NOT `fBase_c` which would place it at `0x60`).
- **Slot Table**:
  - `+0x08` (Slot 0): `0x800CEEA0` -> `dNandThread_c::~dNandThread_c()` (scalar deleting destructor)
  - `+0x0C` (Slot 1): `0x800CFAC0` -> `dNandThread_c::run()` (worker loop override)
  - `+0x10` (Slot 2): `0x800CFCC0` -> `EGG::Thread::onEnter()` (inherited weak inline stub)
  - `+0x14` (Slot 3): `0x800CFCB0` -> `EGG::Thread::onExit()` (inherited weak inline stub)

#### `mMutex` & `EGG::Mutex` Vtables (`0x80317D60` & `0x80317D6C`):
- **Slot Count**: `(0x0C - 8) / 4` = **1 virtual slot** each (scalar deleting destructor).
- **Relationship**: `mMutex` derives from `EGG::Mutex` and is **embedded by value** in `dNandThread_c` at offset `0x50`.
- **Layout of `mMutex` (`sizeof == 0x24` = 36 B)**:
  - `+0x00`: `__vt__6mMutex` vtable pointer (4 B)
  - `+0x04`: `OSMutex mOSMutex` (size `0x18` = 24 B, initialized with `OSInitMutex`)
  - `+0x1C`: `OSCond mOSCond` (size `0x08` = 8 B, initialized with `OSInitCond`)

### B. Full `dNandThread_c` Memory Layout (`sizeof == 0x80` = 128 B)

```cpp
class dNandThread_c : public EGG::Thread {
public:
    dNandThread_c(int priority, EGG::Heap* heap);
    virtual ~dNandThread_c();
    virtual void* run();

    bool cmdExistCheck();
    void existCheck();
    bool cmdSpaceCheck();
    void spaceCheck();
    bool cmdSave(const void* src); // Mangled as fn_800CF170 in map
    u32 save();
    void createBanner();
    void writeBanner(NANDFileInfo* fileInfo);
    bool cmdLoad();
    u32 load();
    void checkCRC();
    bool cmdDeleteFile();
    void deleteFile();
    void setNandError(s32 error);
    void* getSaveData();

    static dNandThread_c* create(EGG::Heap* heap);

    static dNandThread_c* m_instance; // at .sbss:0x8042A298

    // 0x00 - 0x50: EGG::Thread base (size 0x50)
    //   0x00: __vt__13dNandThread_c
    //   0x08: OSThread* mOSThread
    //   0x0C - 0x50: Thread attributes / stack info (0x44 B)
    
    // 0x50 - 0x74: Embedded mMutex (size 0x24)
    mMutex mMutex;       // 0x50: vtable, 0x54: OSMutex (0x18), 0x6C: OSCond (0x08)
    
    // 0x74 - 0x80: Member fields (size 0x0C)
    u32 mCommand;        // 0x74: 0=idle, 1=existCheck, 2=spaceCheck, 3=deleteFile, 4=save, 5=load, 6=exit
    u32 mStatus;         // 0x78: Internal NAND status / error code from setNandError()
    u8  mFileExists;     // 0x7C: Result flag from existCheck (1 if file exists, 0 otherwise)
    u8  mPad[3];         // 0x7D: Alignment padding to 0x80
};
```

- **Compiled Verification**: Tested with `STATIC_ASSERT`s in `scratch/gemini_round3/test_scaffold.cpp` using `compilers/Wii/1.1/mwcceppc.exe`:
  - `sizeof(EGG::Mutex) == 0x4` (PASSED)
  - `sizeof(mMutex) == 0x24` (PASSED)
  - `sizeof(EGG::Thread) == 0x50` (PASSED)
  - `sizeof(dNandThread_c) == 0x80` (PASSED)
  - `offsetof(dNandThread_c, mMutex) == 0x50` (PASSED)
  - `offsetof(dNandThread_c, mCommand) == 0x74` (PASSED)
  - `offsetof(dNandThread_c, mStatus) == 0x78` (PASSED)
  - `offsetof(dNandThread_c, mFileExists) == 0x7C` (PASSED)

---

## 2. Complete Function Table (All 24 Functions)

All 24 functions are class methods (no file-scope static C functions). `r3` is uniformly the `this` pointer for all methods except static `create()` and the base class weak stubs.

| # | Address | Size | Mangled Name / Symbol | Class & Method | Description |
|---|---|---|---|---|---|
| 1 | `0x800CED00` | 280 B (`0x118`) | `__ct__13dNandThread_cFiPQ23EGG4Heap` | `dNandThread_c::dNandThread_c(int, EGG::Heap*)` | Constructor. Initializes `EGG::Thread` base (`stack=0x4000`, `msg=0`), `mMutex`, `mCond`, sets `m_instance = this`, copies initial save data from `dSaveMng_c`. |
| 2 | `0x800CEE20` | 64 B (`0x40`) | `__dt__Q23EGG5MutexFv` | `EGG::Mutex::~Mutex()` | Virtual scalar deleting destructor stub for `EGG::Mutex`. |
| 3 | `0x800CEE60` | 64 B (`0x40`) | `__dt__6mMutexFv` | `mMutex::~mMutex()` | Virtual scalar deleting destructor stub for `mMutex`. |
| 4 | `0x800CEEA0` | 100 B (`0x64`) | `__dt__13dNandThread_cFv` | `dNandThread_c::~dNandThread_c()` | Virtual scalar deleting destructor. Clears `m_instance = NULL`, invokes `EGG::Thread::~Thread()`. |
| 5 | `0x800CEF10` | 112 B (`0x70`) | `cmdExistCheck__13dNandThread_cFv` | `dNandThread_c::cmdExistCheck()` | Member method. Locks mutex with `OSTryLockMutex`, resets `mStatus`/`mFileExists`, sets `mCommand = 1`, signals `mCond`, unlocks mutex. |
| 6 | `0x800CEF80` | 216 B (`0xD8`) | `existCheck__13dNandThread_cFv` | `dNandThread_c::existCheck()` | Worker method. Calls `NANDGetType("wiimj2d.sav")` and `NANDGetType("banner.bin")`, translates result via `setNandError()`, sets `mFileExists`. |
| 7 | `0x800CF060` | 108 B (`0x6C`) | `cmdSpaceCheck__13dNandThread_cFv` | `dNandThread_c::cmdSpaceCheck()` | Member method. Locks mutex, sets `mCommand = 2`, signals `mCond`, unlocks mutex. |
| 8 | `0x800CF0D0` | 148 B (`0x94`) | `spaceCheck__13dNandThread_cFv` | `dNandThread_c::spaceCheck()` | Worker method. Checks NAND free space/inodes for save data via `NANDCheck()`, updates `mStatus`. |
| 9 | `0x800CF170` | 140 B (`0x8C`) | `fn_800CF170` *(unnamed in map)* | `dNandThread_c::cmdSave(const void* src)` | Member method. Locks mutex, sets `mCommand = 4`, copies `0x3FA0` B from `src` to `l_tmpSave`, signals `mCond`, unlocks mutex. |
| 10 | `0x800CF200` | 380 B (`0x17C`) | `save__13dNandThread_cFv` | `dNandThread_c::save()` | Worker method. Opens `"wiimj2d.sav"` with `NANDSimpleSafeOpen` (write), writes `l_tmpSave` (0x3FA0 B), closes via `NANDSimpleSafeClose`, calls `createBanner()`. Returns 2 on retry, 1 on success. |
| 11 | `0x800CF380` | 376 B (`0x178`) | `createBanner__13dNandThread_cFv` | `dNandThread_c::createBanner()` | Worker method. Creates `"/tmp/banner.bin"` via `NANDCreate`, opens it, calls `writeBanner(&fileInfo)`, closes, moves to target `"banner.bin"` via `NANDMove`. |
| 12 | `0x800CF500` | 264 B (`0x108`) | `writeBanner__13dNandThread_cFP12NANDFileInfo` | `dNandThread_c::writeBanner(NANDFileInfo*)` | Worker method. Initializes banner struct `a_banner` with `NANDInitBanner`, retrieves title string via `dMessage_c::getMsg`, loads BTI icons (`save_icon.bti`, `save_banner_EU.bti`), writes 0x72A0 B via `NANDWrite`. |
| 13 | `0x800CF610` | 108 B (`0x6C`) | `cmdLoad__13dNandThread_cFv` | `dNandThread_c::cmdLoad()` | Member method. Locks mutex, sets `mCommand = 5`, signals `mCond`, unlocks mutex. |
| 14 | `0x800CF680` | 644 B (`0x284`) | `load__13dNandThread_cFv` | `dNandThread_c::load()` | Worker method. Opens `"wiimj2d.sav"` with `NANDSimpleSafeOpen` (read), validates length, reads 0x3FA0 B into `l_tmpSave`, closes file, validates CRC with `checkCRC()`. |
| 15 | `0x800CF910` | 204 B (`0xCC`) | `checkCRC__13dNandThread_cFv` | `dNandThread_c::checkCRC()` | Worker method. Calculates CRC32 across header and save slots in `l_tmpSave` using `sCrc::calcCRC32` and compares against expected checksum words. |
| 16 | `0x800CF9E0` | 108 B (`0x6C`) | `cmdDeleteFile__13dNandThread_cFv` | `dNandThread_c::cmdDeleteFile()` | Member method. Locks mutex, sets `mCommand = 3`, signals `mCond`, unlocks mutex. |
| 17 | `0x800CFA50` | 108 B (`0x6C`) | `deleteFile__13dNandThread_cFv` | `dNandThread_c::deleteFile()` | Worker method. Deletes `"banner.bin"` and `"wiimj2d.sav"` from NAND using `NANDDelete`. |
| 18 | `0x800CFAC0` | 220 B (`0xDC`) | `run__13dNandThread_cFv` | `dNandThread_c::run()` | Virtual worker thread loop. Locks mutex, enters `OSWaitCond(&mCond, &mMutex)`, dispatches commands (1=existCheck, 2=spaceCheck, 3=deleteFile, 4=save, 5=load, 6=exit), loops until exit command. |
| 19 | `0x800CFBA0` | 120 B (`0x78`) | `create__13dNandThread_cFPQ23EGG4Heap` | `dNandThread_c::create(EGG::Heap*)` | Static factory function. Allocates 0x80 B on heap, computes thread priority (`OSGetThreadPriority() - 1`), instantiates `dNandThread_c`, resumes thread with `OSResumeThread`. |
| 20 | `0x800CFC20` | 120 B (`0x78`) | `setNandError__13dNandThread_cFl` | `dNandThread_c::setNandError(s32)` | Member method. Translates raw NAND return code via switch jump table `@67342` into internal error enum stored in `mStatus`. |
| 21 | `0x800CFCA0` | 12 B (`0x0C`) | `getSaveData__13dNandThread_cFv` | `dNandThread_c::getSaveData()` | Member method. Returns address of temporary save data buffer `l_tmpSave`. |
| 22 | `0x800CFCB0` | 4 B (`0x04`) | `onExit__Q23EGG6ThreadFv` | `EGG::Thread::onExit()` | Weak inline virtual stub inherited from `EGG::Thread` (empty `blr`). |
| 23 | `0x800CFCC0` | 4 B (`0x04`) | `onEnter__Q23EGG6ThreadFv` | `EGG::Thread::onEnter()` | Weak inline virtual stub inherited from `EGG::Thread` (empty `blr`). |
| 24 | `0x800CFCD0` | 8 B (`0x08`) | `run__Q23EGG6ThreadFv` | `EGG::Thread::run()` | Weak inline virtual stub inherited from `EGG::Thread` (`li r3, 0; blr`). |

---

## 3. Section Bounds & Complete Data Inventory

Every section was audited against `bin/dtk/dtk_splits_wiimj2d.txt` and `bin/dtk/wiimj2d_symbols.txt`.

### Section Bounds Summary

| Section | Start | End | Size | Left Bracket (End) | Right Bracket (Start) | Derivation / Bracket Strength |
|---|---|---|---|---|---|---|
| `.text` | `0x800CED00` | `0x800CFCE0` | `0xFE0` (4,064 B) | `d_multi_manager.cpp` (`0x800CED00`) | `d_next.cpp` (`0x800CFCE0`) | **Hard bracket** (`dtk_splits_wiimj2d.txt`) |
| `.rodata` | `0x802F1470` | `0x802F1498` | `0x28` (40 B) | `d_multi_manager.cpp` (`0x802F1470`) | `d_pause_manager.cpp` (`0x802F1498`) | **Hard bracket** (adjacent to `d_multi_manager` end & `PauseManager_c` jump table) |
| `.data` | `0x80317CD8` | `0x80317D78` | `0xA0` (160 B) | `d_multi_manager.cpp` (`0x80317CD8`) | `d_pause_manager.cpp` (`0x80317D78`) | **Hard bracket** (adjacent to `__vt__11dMultiMng_c` & `__vt__14PauseManager_c`) |
| `.bss` | `0x80359FC0` | `0x80371000` | `0x17040` (94,272 B) | `d_message.cpp` (`0x80359FB8` + pad) | `d_pad.cpp` (`0x80371000`) | **Hard bracket** (bounded by `l_dMessage_obj` and `dPad::m_ex`) |
| `.sdata` | `0x80427F78` | `0x80427F84` | `0x0C` (12 B) | `d_mj2d_data.cpp` (`0x80427F78`) | `d_pad.cpp` (`0x80427F88`) | **Hard bracket** (adjacent to `d_mj2d_data` `.sdata` end and `dPad` `.sdata`) |
| `.sbss` | `0x8042A298` | `0x8042A2A0` | `0x08` (8 B) | `d_multi_manager.cpp` (`0x8042A298`) | `d_next.cpp` (`0x8042A2A0`) | **Hard bracket** (`dtk_splits_wiimj2d.txt`) |
| `.sdata2` | `0x8042CC98` | `0x8042CC98` | `0x0` (0 B) | `d_multi_manager.cpp` (`0x8042CC98`) | `d_next.cpp` (`0x8042CC98`) | **Hard bracket** (`dtk_splits_wiimj2d.txt`, 0 bytes) |
| `.sbss2` | — | — | `0x0` (0 B) | — | — | Empty |
| `.ctors` | — | — | `0x0` (0 B) | — | — | Empty (0 static constructors) |
| `.dtors` | — | — | `0x0` (0 B) | — | — | Empty (0 static destructors) |
| `.init` | — | — | `0x0` (0 B) | — | — | Empty |

---

### Complete Data Inventory & Reference Audit

Every single data object in the TU is referenced by functions within `d_nand_thread.cpp`:

#### 1. `.rodata` (`0x802F1470` - `0x802F1498`, 40 B)
- `0x802F1470` (size `0x10`): `sc_TEMP_BANNER_FILE__27@unnamed@d_nand_thread_cpp@` (`"/tmp/banner.bin\0"`) — Referenced by `createBanner`.
- `0x802F1480` (size `0x0B`): `sc_BANNER_FILE__27@unnamed@d_nand_thread_cpp@` (`"banner.bin\0"`) — Referenced by `createBanner`, `deleteFile`.
- `0x802F148B` (size `0x01`): Alignment padding (`0x00`).
- `0x802F148C` (size `0x0C`): `sc_GAME_FILE__27@unnamed@d_nand_thread_cpp@` (`"wiimj2d.sav\0"`) — Referenced by `existCheck`, `spaceCheck`, `save`, `load`, `deleteFile`.

#### 2. `.data` (`0x80317CD8` - `0x80317D78`, 160 B)
- `0x80317CD8` (size `0x0E`): `@66576` (`"save_icon.bti\0"`) (+2B pad to `0x80317CE8`) — Target of pointer `c_icon_res`.
- `0x80317CE8` (size `0x13`): `@67228` (`"save_banner_EU.bti\0"`) (+1B pad to `0x80317CFC`) — Referenced by `writeBanner`.
- `0x80317CFC` (size `0x0C`): `@67229` (`"save_banner\0"`) — Referenced by `writeBanner`.
- `0x80317D08` (size `0x40`): `@67342` (Jump table for `setNandError` switch) — 16 pointers (0x40 B), referenced by `setNandError`.
- `0x80317D48` (size `0x18`): `__vt__13dNandThread_c` (vtable, 4 virtual slots + 2 prefix words).
- `0x80317D60` (size `0x0C`): `__vt__6mMutex` (vtable, 1 virtual slot + 2 prefix words).
- `0x80317D6C` (size `0x0C`): `__vt__Q23EGG5Mutex` (vtable, 1 virtual slot + 2 prefix words).

#### 3. `.bss` (`0x80359FC0` - `0x80371000`, 94,272 B)
- `0x80359FC0` (size `0x4000` = 16,384 B): `l_safeCopyBuf__27@unnamed@d_nand_thread_cpp@` — Staging buffer passed to `NANDSimpleSafeOpen` in `save()` and `load()`.
- `0x8035DFC0` (size `0x3FA0` = 16,288 B): `l_tmpSave__27@unnamed@d_nand_thread_cpp@` — Temporary save data buffer referenced by `__ct__`, `existCheck`, `cmdSave`, `save`, `load`, `checkCRC`, `getSaveData`.
- `0x80361F60` (size `0xF0A0` = 61,600 B): `@LOCAL@writeBanner__13dNandThread_cFP12NANDFileInfo@a_banner@0` — Function-scope static buffer in `writeBanner()` passed to `NANDInitBanner`.

#### 4. `.sdata` (`0x80427F78` - `0x80427F84`, 12 B)
- `0x80427F78` (size `0x04`): `@LOCAL@writeBanner__13dNandThread_cFP12NANDFileInfo@c_icon_res` — Function-scope static pointer pointing to `@66576` (`"save_icon.bti"`).
- `0x80427F7C` (size `0x05`): `@67269` (`"SMNP\0"`) — Game title code string passed to `NANDInitBanner` (+3B pad to `0x80427F84`).

#### 5. `.sbss` (`0x8042A298` - `0x8042A2A0`, 8 B)
- `0x8042A298` (size `0x04`): `m_instance__13dNandThread_c` (`dNandThread_c*`).
- `0x8042A29C` (size `0x04`): Alignment padding to `0x8042A2A0` (`d_next.cpp`).

---

## 4. SDK Dependencies & Symbol Pins

### Category (a): Already Banked in Compiled Translation Units (6 functions)
No `syms.txt` pins needed; linked automatically from banked objects:
1. `OSInitMutex` (`0x801B2F60` — `lib/revolution/os/OSMutex.c`)
2. `OSLockMutex` (`0x801B2FA0` — `lib/revolution/os/OSMutex.c`)
3. `OSTryLockMutex` (`0x801B31C0` — `lib/revolution/os/OSMutex.c`)
4. `OSUnlockMutex` (`0x801B3080` — `lib/revolution/os/OSMutex.c`)
5. `getRes__6dRes_cCFPCcPCc` (`0x800DF270` — `dol/bases/d_res.cpp`)
6. `setCurrentHeap__5mHeapFPQ23EGG4Heap` (`0x8016E630` — `dol/mLib/m_heap.cpp`)

---

### Category (b): Declared in Headers, Undecompiled
#### Already pinned in `syms.txt` (9 symbols):
1. `OSGetCurrentThread` (`0x801B4C70`)
2. `OSGetThreadPriority` (`0x801B60B0`)
3. `OSResumeThread` (`0x801B59A0`)
4. `__dl__FPv` (`0x802B93C0`)
5. `__nw__FUl` (`0x802B9350`)
6. `getMsg__10dMessage_cFUlUl` (`0x800CDD30`)
7. `getSaveGame__10dSaveMng_cFSc` (`0x800E0470`)
8. `getTempGame__10dSaveMng_cFSc` (`0x800E04A0`)
9. `memcpy` (`0x80004364`)

#### **NEEDS PIN in `syms.txt` (14 symbols)**:
These are declared in SDK headers but missing from `syms.txt`. Linking will fail with undefined symbols without these pins:

```ini
NANDCheck=0x801DB280
NANDClose=0x801D9990
NANDCreate=0x801D8620
NANDDelete=0x801D8920
NANDGetHomeDir=0x801DAC30
NANDGetLength=0x801D9180
NANDGetType=0x801DAFB0
NANDInitBanner=0x801DB0E0
NANDMove=0x801D9110
NANDOpen=0x801D96F0
NANDRead=0x801D8B30
NANDWrite=0x801D8C20
__ct__Q23EGG6ThreadFUliiPQ23EGG4Heap=0x802BA4F0
__dt__Q23EGG6ThreadFv=0x802BA640
```

---

### Category (c): Not Declared in Headers (7 symbols)
These are missing from `include/` and require header additions plus `syms.txt` pins:

#### 1. Revolution OS Condition Variables (`include/lib/revolution/OS/OSCond.h` or `OSMutex.h`):
```c
typedef struct OSCond {
    OSThreadQueue queue; // size 0x08
} OSCond;

void OSInitCond(OSCond* cond);
void OSWaitCond(OSCond* cond, OSMutex* mutex);
void OSSignalCond(OSCond* cond);
```
- Pins needed:
  ```ini
  OSInitCond=0x801B3280
  OSWaitCond=0x801B3290
  OSSignalCond=0x801B3370
  ```

#### 2. Revolution NAND SimpleSafe API (`include/lib/revolution/NAND/nand.h` or `NANDOpenClose.h`):
```c
s32 NANDSimpleSafeOpen(const char* path, NANDFileInfo* info, u8 accType, void* stageBuf, u32 stageBufSize);
s32 NANDSimpleSafeClose(NANDFileInfo* info);
s32 NANDSimpleSafeCancel(NANDFileInfo* info);
```
- Pins needed:
  ```ini
  NANDSimpleSafeOpen=0x801D9A90
  NANDSimpleSafeClose=0x801D9E50
  NANDSimpleSafeCancel=0x801DA0A0
  ```

#### 3. CRC32 Calculation Utility (`include/game/bases/d_crc.hpp` or `s_crc.hpp`):
```cpp
class sCrc {
public:
    static u32 calcCRC32(const void* data, unsigned long length);
};
```
- Pin needed:
  ```ini
  calcCRC32__4sCrcFPCvUl=0x8015F270
  ```

---

## 5. Specific Hazards & What Will Go Wrong

1. **`EGG::Thread` Inline Virtual Functions**:
   In `auto_03_800CED00_text.o`, functions 22 (`onExit`), 23 (`onEnter`), and 24 (`run`) are weak virtual functions emitted at the very end of `.text`.
   - **Hazard**: If `include/lib/egg/core/eggThread.h` declares these virtuals *without* inline default bodies, MWCC will NOT emit them, leaving `.text` short by `0x10` bytes and failing verification.
   - **Resolution**: `eggThread.h` must declare them inline:
     ```cpp
     virtual void* run() { return 0; }
     virtual void onEnter() {}
     virtual void onExit() {}
     ```

2. **Vtable Order in `.data`**:
   The emitted `.data` sequence is:
   `__vt__13dNandThread_c` -> `__vt__6mMutex` -> `__vt__Q23EGG5Mutex`.
   - **Hazard**: If `mMutex` is defined with an explicit out-of-line destructor before `dNandThread_c`, `__vt__6mMutex` would be emitted before `__vt__13dNandThread_c`, inverting the `.data` layout.
   - **Resolution**: Keep `mMutex`'s virtual destructor inline or declare `mMutex` prior to `dNandThread_c` without triggering early vtable emission before `dNandThread_c`.

3. **Anonymous Namespace Scoping**:
   - `sc_TEMP_BANNER_FILE`, `sc_BANNER_FILE`, `sc_GAME_FILE` (.rodata), `l_safeCopyBuf`, `l_tmpSave` (.bss) are mangled with `@unnamed@d_nand_thread_cpp@`.
   - **Resolution**: They must be placed inside an unnamed `namespace { ... }` block in `d_nand_thread.cpp`.

4. **Function-Scope Statics for Banner Buffer and Texture Resource**:
   - `a_banner` is placed at `.bss:0x80361F60` (`@LOCAL@writeBanner...a_banner@0`).
   - `c_icon_res` is placed at `.sdata:0x80427F78` (`@LOCAL@writeBanner...c_icon_res`).
   - **Hazard**: Declaring them as global file-scope variables will change their section placement and symbol pool IDs. They MUST be declared inside `writeBanner()` as:
     ```cpp
     static u8 a_banner[0xF0A0];
     static const char* c_icon_res = "save_icon.bti";
     ```

5. **`fn_800CF170` Mangling vs Pin**:
   - In `wiimj2d_symbols.txt`, this function is unnamed (`fn_800CF170`).
   - Its signature is `bool dNandThread_c::cmdSave(const void* src)`.
   - **Hazard**: When authored as `cmdSave`, MWCC will emit `cmdSave__13dNandThread_cFPCv`. Claude's slice map or `syms.txt` must either map `fn_800CF170` to the new mangled name or retain the pin.

---

## 6. Ready-to-Author Header Proposals

### Proposed Header: `include/game/bases/d_nand_thread.hpp`
```cpp
#pragma once

#include <types.h>
#include <lib/revolution/OS.h>
#include <lib/revolution/NAND.h>
#include <lib/egg/core/eggThread.h>
#include <lib/egg/core/eggMutex.h>

class mMutex : public EGG::Mutex {
public:
    virtual ~mMutex() {}

    OSMutex mOSMutex; // 0x04
    OSCond  mOSCond;  // 0x1C
};

class dNandThread_c : public EGG::Thread {
public:
    dNandThread_c(int priority, EGG::Heap* heap);
    virtual ~dNandThread_c();
    virtual void* run();

    bool cmdExistCheck();
    void existCheck();
    bool cmdSpaceCheck();
    void spaceCheck();
    bool cmdSave(const void* src);
    u32 save();
    void createBanner();
    void writeBanner(NANDFileInfo* fileInfo);
    bool cmdLoad();
    u32 load();
    void checkCRC();
    bool cmdDeleteFile();
    void deleteFile();
    void setNandError(s32 error);
    void* getSaveData();

    static dNandThread_c* create(EGG::Heap* heap);

    static dNandThread_c* m_instance;

    mMutex mMutex;       // 0x50
    u32 mCommand;        // 0x74
    u32 mStatus;         // 0x78
    u8 mFileExists;      // 0x7C
    u8 mPad[3];          // 0x7D
};
```

- **Offset-Perturbing**: NO. Class is brand new to the tree; all internal offsets (`0x50`, `0x74`, `0x78`, `0x7C`) exactly match the retail binary.
- **Compiled**: YES (verified in `scratch/gemini_round3/test_scaffold.cpp`).
- **Confidence**: High.
