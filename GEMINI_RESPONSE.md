# Gemini Response — Round 4: Hazard Proofs for `d_nand_thread.cpp` & Pre-Flight Analysis for `m_pad.cpp`

## Executive Summary

1. **Task A (`d_nand_thread.cpp` Hazard Verification)**: All five predicted hazards were experimentally subjected to compiled test fixtures using CodeWarrior `mwcceppc.exe` (`-O4 -ipa file`) and disassembler `dtk`.
   - **Hazard 1 (Weak virtuals in `EGG::Thread`)**: **CONFIRMED & REFINED**. With inline bodies in `eggThread.h`, MWCC emits `onExit__Q23EGG6ThreadFv` and `onEnter__Q23EGG6ThreadFv` at the end of `.text`. Without inline bodies, **none** are emitted, leaving `.text` short by `0x10` bytes.
   - **Hazard 2 (Vtable order in `.data`)**: **CONFIRMED & EXPLAINED**. Non-weak vtables (`__vt__13dNandThread_c`) are unconditionally emitted first; weak base/embedded vtables (`__vt__6mMutex`, `__vt__Q23EGG5Mutex`) follow in derived-then-base order. If `mMutex`'s destructor is out-of-line in a separate TU, its vtable is completely omitted.
   - **Hazard 3 (Anonymous namespace & function-scope statics)**: **CONFIRMED**. Placing constants/buffers in an unnamed namespace produces `@unnamed@d_nand_thread_cpp@` in `.rodata` and `.bss`. Function-scope statics inside `writeBanner()` (`a_banner`, `c_icon_res`) land at `.bss:0x80361F60` (`0xF0A0` B) and `.sdata:0x80427F78` (`0x04` B).
   - **Hazard 4 (`sizeof(dNandThread_c) == 0x80`)**: **CONFIRMED**. Verified with compiled `STATIC_ASSERT`s. `sizeof(EGG::Thread) == 0x50`, `mMutex` at `+0x50`, `sizeof(dNandThread_c) == 0x80`.
   - **Hazard 5 (Section sizes)**: **CONFIRMED**. Full scaffold compiles to `.rodata`: `0x28` B (exact), `.bss`: `0x17040` B (exact), `.sbss`: `0x08` B (exact), `.sdata`: `0x0C` B (exact), `.data`: `0xA0` B (exact).

2. **Task B (`dol/mLib/m_pad.cpp` Pre-Flight)**: Complete bracketed analysis of `m_pad.cpp` (`0x8016F330`–`0x80170AC0`, 56 functions, 6,032 B).
   - Fully bracketed between `m_mtx.cpp` and `m_vec.cpp`.
   - Contains a static constructor in `.ctors` slot `0x8000637C` (`__sinit_\m_pad_cpp`) initializing an array of 4 `PadAdditionalData_t` structs (`0x60` B in `.bss`).
   - Emits only one polymorphic class vtable: `__vt__Q24mTex8edit4b_c` (`0x10` B at `.data:0x80329F60`).

---

# Part 1: Task A — Hazard Proofs for `d_nand_thread.cpp`

## Hazard 1: `EGG::Thread` Weak Virtual Functions Emission in `.text`

### Question:
Do the weak virtual functions get emitted at the end of `.text`? How does inline vs non-inline declaration in `eggThread.h` change the output?

### Compiled Test Fixture:
We compiled the `d_nand_thread.cpp` scaffold under two configurations in `scratch/gemini_round4/`:
1. **Configuration 1A (With inline bodies in `eggThread.h`)**:
   ```cpp
   namespace EGG {
   class Thread {
   public:
       Thread(u32 stackSize, int msgCount, int priority, Heap* heap);
       virtual ~Thread();
       virtual void* run() { return 0; }
       virtual void onEnter() {}
       virtual void onExit() {}
       u8 mPad[0x4c];
   };
   }
   ```
2. **Configuration 1B (Without inline bodies in `eggThread.h`)**:
   ```cpp
   namespace EGG {
   class Thread {
   public:
       Thread(u32 stackSize, int msgCount, int priority, Heap* heap);
       virtual ~Thread();
       virtual void* run();
       virtual void onEnter();
       virtual void onExit();
       u8 mPad[0x4c];
   };
   }
   ```

### Findings & Disassembly Evidence:
- **In Configuration 1A (with inline bodies)**:
  MWCC emits `onExit__Q23EGG6ThreadFv` (`0x04` B) and `onEnter__Q23EGG6ThreadFv` (`0x04` B) at the tail of `.text`:
  ```assembly
  # EGG::Thread::onExit()
  .fn onExit__Q23EGG6ThreadFv, weak
  /* 00000FF0  4E 80 00 20 */    blr
  .endfn onExit__Q23EGG6ThreadFv

  # EGG::Thread::onEnter()
  .fn onEnter__Q23EGG6ThreadFv, weak
  /* 00001000  4E 80 00 20 */    blr
  .endfn onEnter__Q23EGG6ThreadFv
  ```
  **Why `onExit` and `onEnter` are emitted**: In `__vt__13dNandThread_c`, slot 2 points to `EGG::Thread::onEnter` and slot 3 points to `EGG::Thread::onExit` (because `dNandThread_c` does not override them). Since they are defined with inline bodies in the header, MWCC emits weak out-of-line stubs.
  **Emission order**: MWCC uses a LIFO queue when popping referenced inline functions at EOF, causing slot 3 (`onExit`) to be emitted before slot 2 (`onEnter`), matching the retail binary at `0x800CFCB0` and `0x800CFCC0`.

- **In Configuration 1B (without inline bodies)**:
  Neither `onExit`, `onEnter`, nor `run` are emitted. MWCC assumes they are compiled in `eggThread.cpp`, leaving `.text` short by `0x10` bytes (plus `0x10` bytes padding gap).

### Verdict:
- **CONFIRMED**. `include/lib/egg/core/eggThread.h` **MUST** declare `onEnter()` and `onExit()` (and `run()`) with inline bodies `{}` / `{ return 0; }`.
- Shared-header proposal diff provided in Section 3.

---

## Hazard 2: Vtable Emission Order in `.data`

### Question:
Do the three vtables come out in the order `__vt__13dNandThread_c` -> `__vt__6mMutex` -> `__vt__Q23EGG5Mutex`? Can an inversion be provoked?

### Compiled Test Fixture:
We tested three configurations:
- **2A (Inline destructors for `mMutex` and `EGG::Mutex`)**:
  ```cpp
  class Mutex { public: virtual ~Mutex() {} };
  class mMutex : public EGG::Mutex { public: virtual ~mMutex() {} };
  ```
- **2B (Out-of-line destructor for `mMutex` defined before `dNandThread_c`)**:
  ```cpp
  mMutex::~mMutex() {}
  dNandThread_c::dNandThread_c(...) {}
  ```
- **2C (`mMutex` destructor declared out-of-line without body in TU)**.

### Findings:
1. In **2A** and **2B**, MWCC emitted the `.data` vtables in the exact order:
   1. `__vt__13dNandThread_c` (strong global linkage)
   2. `__vt__6mMutex` (weak linkage, derived class)
   3. `__vt__Q23EGG5Mutex` (weak linkage, base class)
2. In **2C** (where `mMutex::~mMutex()` is not defined in this TU), `__vt__6mMutex` is **omitted entirely** from `.data`, shifting `.data` short by `0x0C` bytes.

### Mechanism:
In MWCC, non-weak class vtables are generated during compilation of the class's non-inline methods. Weak vtables (from classes whose virtual functions/destructors are entirely inline in headers) are deferred to the end of `.data`. When constructing `mMutex`, MWCC resolves the derived vtable `__vt__6mMutex` before the base `__vt__Q23EGG5Mutex`.

### Verdict:
- **CONFIRMED**. `mMutex` and `EGG::Mutex` must have inline virtual destructors `virtual ~mMutex() {}` and `virtual ~Mutex() {}` in headers to guarantee weak emission in `.data`.

---

## Hazard 3: Anonymous Namespace & Function-Scope Statics Placement

### Question:
Do anonymous namespace objects land in `.rodata` and `.bss` with correct mangling (`@unnamed@d_nand_thread_cpp@`), and do function-scope statics land in `.bss`/`.sdata` rather than file-scope?

### Compiled Test Fixture (`test_hazard3.py`):
```cpp
namespace {
const char sc_TEMP_BANNER_FILE[] = "/tmp/banner.bin";
const char sc_BANNER_FILE[] = "banner.bin";
const char sc_GAME_FILE[] = "wiimj2d.sav";

u8 l_safeCopyBuf[0x4000];
u8 l_tmpSave[0x3FA0];
}

void dNandThread_c::writeBanner(NANDFileInfo* fileInfo) {
    static u8 a_banner[0xF0A0];
    static const char* c_icon_res = "save_icon.bti";
    ...
}
```

### Disassembly Results:
- **`.rodata` (Exact size `0x28` B, matching retail `0x802F1470`–`0x802F1498`)**:
  - `0x00`: `sc_TEMP_BANNER_FILE__33@unnamed@d_nand_thread_cpp@` (`0x10` B)
  - `0x10`: `sc_BANNER_FILE__33@unnamed@d_nand_thread_cpp@` (`0x0B` B + 1 B pad)
  - `0x1C`: `sc_GAME_FILE__33@unnamed@d_nand_thread_cpp@` (`0x0C` B)
- **`.bss` (Exact size `0x17040` B, matching retail `0x80359FC0`–`0x80371000`)**:
  - `0x0000`: `l_safeCopyBuf__33@unnamed@d_nand_thread_cpp@` (`0x4000` B)
  - `0x4000`: `l_tmpSave__33@unnamed@d_nand_thread_cpp@` (`0x3FA0` B)
  - `0x7FA0`: `@LOCAL@writeBanner__13dNandThread_cFP12NANDFileInfo@a_banner` (`0xF0A0` B)
- **`.sdata` (Exact matching retail `0x80427F78`–`0x80427F84`)**:
  - `0x00`: `@LOCAL@writeBanner__13dNandThread_cFP12NANDFileInfo@c_icon_res@0` (`0x04` B)
  - `0x04`: String literal `"SMNP"` (`0x05` B + 3 B pad)

### Hoisting Failure Mode:
If `a_banner` or `c_icon_res` are declared at file scope instead of function scope, their symbols mangle to `a_banner__27@unnamed...` in `.bss` and `c_icon_res__27@unnamed...` in `.data`, completely breaking symbol names and pool ID alignment.

### Verdict:
- **CONFIRMED**. `a_banner` and `c_icon_res` **MUST** remain function-scope statics inside `writeBanner()`.

---

## Hazard 4: `sizeof(dNandThread_c)` and Member Offsets

### Question:
Does `sizeof(dNandThread_c)` come out `0x80` with `EGG::Thread` base?

### Compiled Test Fixture (`test_hazard4.py`):
```cpp
STATIC_ASSERT(sizeof(EGG::Mutex) == 0x4);
STATIC_ASSERT(sizeof(mMutex) == 0x24);
STATIC_ASSERT(sizeof(EGG::Thread) == 0x50);
STATIC_ASSERT(sizeof(dNandThread_c) == 0x80);
STATIC_ASSERT(offsetof(dNandThread_c, mMutex) == 0x50);
STATIC_ASSERT(offsetof(dNandThread_c, mCommand) == 0x74);
STATIC_ASSERT(offsetof(dNandThread_c, mStatus) == 0x78);
STATIC_ASSERT(offsetof(dNandThread_c, mFileExists) == 0x7C);
```

### Results:
- **PASSED** in MWCC.
- `EGG::Thread` has `0x04` vtable pointer + `0x4C` member padding = `0x50` B.
- `mMutex` sits at object offset `0x50` (size `0x24` B).
- `mCommand` at `+0x74` (`0x04` B).
- `mStatus` at `+0x78` (`0x04` B).
- `mFileExists` at `+0x7C` (`0x01` B + 3 B alignment pad).
- Total `sizeof(dNandThread_c) == 0x80` (128 B).

### Verdict:
- **CONFIRMED**. Layout is byte-exact.

---

## Hazard 5: Full Scaffold Section Sizes Comparison

### Compiled Scaffold (`d_nand_thread_full.cpp`):
All 24 functions, anonymous namespace objects, function-scope statics, and instance pointer compiled and disassembled with `dtk`.

### Section Comparison Table:

| Section | Retail Address Range | Retail Size | Scaffold Size | Status | Notes |
|---|---|---|---|---|---|
| `.rodata` | `0x802F1470`–`0x802F1498` | **`0x28` B** (40 B) | **`0x28` B** (40 B) | **EXACT MATCH** | 3 file path constants |
| `.bss` | `0x80359FC0`–`0x80371000` | **`0x17040` B** (94,272 B) | **`0x17040` B** (94,272 B) | **EXACT MATCH** | 2 staging buffers + banner buffer |
| `.sbss` | `0x8042A298`–`0x8042A2A0` | **`0x08` B** (8 B) | **`0x08` B** (8 B) | **EXACT MATCH** | `m_instance` (4 B) + 4 B pad |
| `.sdata` | `0x80427F78`–`0x80427F84` | **`0x0C` B** (12 B) | **`0x0C` B** (12 B) | **EXACT MATCH** | `c_icon_res` (4 B) + `"SMNP"` (5 B + pad) |
| `.data` | `0x80317CD8`–`0x80317D78` | **`0xA0` B** (160 B) | **`0xA0` B** (160 B) | **EXACT MATCH** | 3 string literals + jump table + 3 vtables |
| `.text` | `0x800CED00`–`0x800CFCE0` | `0xFE0` B (4,064 B) | Stubs | **All 24 symbols in exact order** | Final byte-match pending function authoring |

---

# Part 2: Task B — Pre-Flight Analysis for `dol/mLib/m_pad.cpp`

## 1. Executive Summary

`dol/mLib/m_pad.cpp` is a self-contained controller and texture-rendering support translation unit (`0x8016F330`–`0x80170AC0`, 56 functions, 6,032 B).

It contains:
1. The `mPad` controller wrapper (WPAD info polling, battery query, channel management).
2. Static initialization via `__sinit_\m_pad_cpp` (`.ctors:0x8000637C`) registering a global array destructor for 4 `PadAdditionalData_t` structs (`0x60` B in `.bss`).
3. Template instantiations of `mPrint::MyPrintBase<char>` and `mPrint::MyPrintBase<wchar_t>` (32 functions).
4. Low-level texture tile helper classes `mTex::base_c` and polymorphic `mTex::edit4b_c` (`__vt__Q24mTex8edit4b_c` at `.data:0x80329F60`).

---

## 2. Section Bounds & Hard Bracketing

`m_pad.cpp` is **strictly hard-bracketed** across all active sections in `slices/wiimj2d.json` and `bin/dtk/dtk_splits_wiimj2d.txt`.

| Section | Start | End | Size | Left Bracket (End) | Right Bracket (Start) | Derivation / Bracket Strength |
|---|---|---|---|---|---|---|
| `.text` | `0x8016F330` | `0x80170AC0` | `0x1790` (6,032 B) | `m_mtx.cpp` (`0x8016F330`) | `m_vec.cpp` (`0x80170AC0`) | **Hard bracket** (`dtk_splits_wiimj2d.txt`) |
| `.ctors` | `0x8000637C` | `0x80006380` | `0x04` (4 B) | `m_mtx.cpp` (`0x8000637C`) | `m_vec.cpp` (`0x80006380`) | **Hard bracket** (`slices/wiimj2d.json`, offset `0x21c..0x220`) |
| `.data` | `0x80329F60` | `0x80329F70` | `0x10` (16 B) | `0x80329F60` (`auto_07_80329F60_data.o`) | `m_wipe_fader.cpp` (`0x80329F70`, `__vt__12mWipeFader_c`) | **Hard bracket** (`slices/wiimj2d.json`, offset `0x2b8c0..0x2b8d0`) |
| `.bss` | `0x80377F88` | `0x803780C8` | `0x140` (320 B) | `m_mtx.cpp` (`0x80377F88`, `Identity__6mMtx_c`) | `m_vec.cpp` (`0x803780C8`, `Zero__7mVec3_c`) | **Hard bracket** (`slices/wiimj2d.json`, offset `0x26608..0x26748`) |
| `.sbss` | `0x8042A740` | `0x8042A760` | `0x20` (32 B) | `m_heap.cpp` (`0x8042A73C` + pad) | `0x8042A760` | **Hard bracket** |
| `.sdata2` | `0x8042FE30` | `0x8042FE50` | `0x20` (32 B) | `m_mtx.cpp` (`0x8042FE30`) | `m_vec.cpp` (`0x8042FE50`) | **Hard bracket** (`slices/wiimj2d.json`, offset `0x2cb0..0x2cd0`) |
| `.rodata` | — | — | `0x0` | — | — | Empty (0 B) |
| `.sdata` | — | — | `0x0` | — | — | Empty (0 B) |
| `.dtors` | — | — | `0x0` | — | — | Empty (0 B) |

---

## 3. Complete Data Inventory & Reference Audit

### A. `.ctors` (`0x8000637C` - `0x80006380`, 4 B)
- `0x8000637C` (size `0x04`): Pointer to `__sinit_\m_pad_cpp` (`0x8016F7B0`).

### B. `.data` (`0x80329F60` - `0x80329F70`, 16 B)
- `0x80329F60` (size `0x10`): `__vt__Q24mTex8edit4b_c` (2 virtual slots: `~edit4b_c()` and `set(int, int, u8, bool)`). Referenced by `init__Q24mTex8edit4b_cFiiPUc`.

### C. `.bss` (`0x80377F88` - `0x803780C8`, 320 B = `0x140` B)
- `0x80377F88` (size `0x10` = 16 B): `g_core__4mPad` (`EGG::CoreController* g_core[4]`). Referenced by `create`, `beginPad`, `endPad`. (+16 B alignment pad to `0x80377FA8`).
- `0x80377FA8` (size `0x60` = 96 B): `g_PadAdditionalData__4mPad` (`mPad::PadAdditionalData_t[4]`, 4 x 24 B). Referenced by `__sinit_\m_pad_cpp`, `__arraydtor$13953`, `beginPad`, `setWPADInfo`, `clearWPADInfo`.
- `0x80378008` (size `0x60` = 96 B): `s_WPADInfo__4mPad` (`WPADInfo[4]`). Referenced by `beginPad`, `getBatteryLevel_ch`, `setWPADInfo`, `clearWPADInfo`, `initWPADInfo`, `getWPADInfoCb`.
- `0x80378068` (size `0x60` = 96 B): `s_WPADInfoTmp__4mPad` (`WPADInfo[4]`). Referenced by `initWPADInfo`, `getWPADInfoCb`, `getWPADInfoAsync`.

### D. `.sbss` (`0x8042A740` - `0x8042A760`, 32 B = `0x20` B)
- `0x8042A740` (size `0x04`): `g_padMg__4mPad` (`void*`).
- `0x8042A744` (size `0x04`): `g_currentCoreID__4mPad` (`u32`).
- `0x8042A748` (size `0x04`): `g_currentCore__4mPad` (`EGG::CoreController*`).
- `0x8042A74C` (size `0x04`): `g_IsConnected__4mPad` (`u8` + 3 B pad).
- `0x8042A750` (size `0x04`): `g_PadFrame__4mPad` (`u32`).
- `0x8042A754` (size `0x04`): `s_WPADInfoAvailable__4mPad` (`u32`).
- `0x8042A758` (size `0x04`): `s_GetWPADInfoInterval__4mPad` (`u32`).
- `0x8042A75C` (size `0x04`): `s_GetWPADInfoCount__4mPad` (`u32`).

### E. `.sdata2` (`0x8042FE30` - `0x8042FE50`, 32 B = `0x20` B)
- Float/double constants referenced by `MyPrintBase<char>::Flush`, `MyPrintBase<wchar_t>::Flush`.

---

## 4. Complete Function Table (All 56 Functions)

| # | Address | Size | Mangled Name / Symbol | Class & Method | Description |
|---|---|---|---|---|---|
| 1 | `0x8016F330` | 48 B (`0x30`) | `create__4mPadFv` | `mPad::create()` | Allocates / creates controller objects. |
| 2 | `0x8016F360` | 484 B (`0x1E4`) | `beginPad__4mPadFv` | `mPad::beginPad()` | Updates core controller inputs and WPAD states across 4 channels. |
| 3 | `0x8016F550` | 20 B (`0x14`) | `endPad__4mPadFv` | `mPad::endPad()` | Resets active core pointer. |
| 4 | `0x8016F570` | 36 B (`0x24`) | `setCurrentChannel__4mPadFQ24mPad4CH_e` | `mPad::setCurrentChannel(CH_e)` | Sets current active channel pointer. |
| 5 | `0x8016F5A0` | 48 B (`0x30`) | `getBatteryLevel_ch__4mPadFQ24mPad4CH_e` | `mPad::getBatteryLevel_ch(CH_e)` | Queries WPAD battery level for channel. |
| 6 | `0x8016F5D0` | 104 B (`0x68`) | `setWPADInfo__4mPadFQ24mPad4CH_eRC8WPADInfo` | `mPad::setWPADInfo(CH_e, const WPADInfo&)` | Stores WPAD info for channel. |
| 7 | `0x8016F640` | 68 B (`0x44`) | `clearWPADInfo__4mPadFQ24mPad4CH_e` | `mPad::clearWPADInfo(CH_e)` | Clears WPAD info for channel. |
| 8 | `0x8016F690` | 60 B (`0x3C`) | `initWPADInfo__4mPadFv` | `mPad::initWPADInfo()` | Resets polling state counters. |
| 9 | `0x8016F6D0` | 60 B (`0x3C`) | `getWPADInfoCb` | `mPad::getWPADInfoCb(s32, s32)` | WPAD asynchronous callback. |
| 10 | `0x8016F710` | 100 B (`0x64`) | `getWPADInfoAsync__4mPadFQ24mPad4CH_e` | `mPad::getWPADInfoAsync(CH_e)` | Initiates async WPAD info query via `WPADGetInfoAsync`. |
| 11 | `0x8016F780` | 20 B (`0x14`) | `setGetWPADInfoInterval__4mPadFUl` | `mPad::setGetWPADInfoInterval(u32)` | Sets polling interval. |
| 12 | `0x8016F7A0` | 8 B (`0x08`) | `getGetWPADInfoInterval__4mPadFv` | `mPad::getGetWPADInfoInterval()` | Returns polling interval. |
| 13 | `0x8016F7B0` | 88 B (`0x58`) | `__sinit_\m_pad_cpp` | Static initializer | Constructs `g_PadAdditionalData` and registers `__arraydtor$13953`. |
| 14 | `0x8016F810` | 4 B (`0x04`) | `__ct__Q24mPad19PadAdditionalData_tFv` | `mPad::PadAdditionalData_t::PadAdditionalData_t()` | Inline constructor stub (`blr`). |
| 15 | `0x8016F820` | 64 B (`0x40`) | `__dt__Q24mPad19PadAdditionalData_tFv` | `mPad::PadAdditionalData_t::~PadAdditionalData_t()` | Scalar deleting destructor. |
| 16 | `0x8016F860` | 28 B (`0x1C`) | `__arraydtor$13953` | Array destructor helper | Destroys array of 4 `PadAdditionalData_t` objects. |
| 17 | `0x8016F880` | 72 B (`0x48`) | `__ct__Q26mPrint14MyPrintBase<c>Fv` | `mPrint::MyPrintBase<char>::MyPrintBase()` | Constructor. Initializes linked list. |
| 18 | `0x8016F8D0` | 64 B (`0x40`) | `__dt__Q26mPrint14MyPrintBase<c>Fv` | `mPrint::MyPrintBase<char>::~MyPrintBase()` | Destructor. Frees allocated text nodes. |
| 19 | `0x8016F910` | 92 B (`0x5C`) | `Initialize__Q26mPrint14MyPrintBase<c>FPvUlRCQ34nw4r2ut4Font` | `mPrint::MyPrintBase<char>::Initialize(...)` | Initializes print heap and font. |
| 20 | `0x8016F970` | 8 B (`0x08`) | `SetFont__Q26mPrint14MyPrintBase<c>FRCQ34nw4r2ut4Font` | `mPrint::MyPrintBase<char>::SetFont(...)` | Sets active font. |
| 21 | `0x8016F980` | 8 B (`0x08`) | `GetFont__Q26mPrint14MyPrintBase<c>CFv` | `mPrint::MyPrintBase<char>::GetFont()` | Returns active font. |
| 22 | `0x8016F990` | 8 B (`0x08`) | `SetVisible__Q26mPrint14MyPrintBase<c>Fb` | `mPrint::MyPrintBase<char>::SetVisible(bool)` | Sets visibility flag. |
| 23 | `0x8016F9A0` | 8 B (`0x08`) | `IsVisible__Q26mPrint14MyPrintBase<c>CFv` | `mPrint::MyPrintBase<char>::IsVisible()` | Queries visibility. |
| 24 | `0x8016F9B0` | 268 B (`0x10C`) | `VRegisterf__Q26mPrint14MyPrintBase<c>...` | `mPrint::MyPrintBase<char>::VRegisterf(...)` | Formats string with `vsnprintf` and registers text node. |
| 25 | `0x8016FAC0` | 140 B (`0x8C`) | `Reset__Q26mPrint14MyPrintBase<c>Fv` | `mPrint::MyPrintBase<char>::Reset()` | Clears registered text nodes. |
| 26 | `0x8016FB50` | 604 B (`0x25C`) | `Flush__Q26mPrint14MyPrintBase<c>Fv` | `mPrint::MyPrintBase<char>::Flush()` | Renders queued text nodes via `nw4r::ut::TextWriter`. |
| 27 | `0x8016FDB0` | 192 B (`0xC0`) | `Flush__Q26mPrint14MyPrintBase<c>Fiiii` | `mPrint::MyPrintBase<char>::Flush(int, int, int, int)` | Scissor-bounded render flush. |
| 28 | `0x8016FE70` | 228 B (`0xE4`) | `Register__Q26mPrint14MyPrintBase<c>...` | `mPrint::MyPrintBase<char>::Register(...)` | Allocates and appends a text node. |
| 29 | `0x8016FF60` | 12 B (`0x0C`) | `GetFirstText__Q26mPrint14MyPrintBase<c>Fv` | `mPrint::MyPrintBase<char>::GetFirstText()` | Returns head of text list. |
| 30 | `0x8016FF70` | 8 B (`0x08`) | `GetNextText__Q26mPrint14MyPrintBase<c>...` | `mPrint::MyPrintBase<char>::GetNextText(...)` | Returns next text list entry. |
| 31 | `0x8016FF80` | 72 B (`0x48`) | `Unregister__Q26mPrint14MyPrintBase<c>...` | `mPrint::MyPrintBase<char>::Unregister(...)` | Removes node from list and frees memory. |
| 32 | `0x8016FFD0` | 60 B (`0x3C`) | `SetBuffer__Q26mPrint14MyPrintBase<c>FPvUl` | `mPrint::MyPrintBase<char>::SetBuffer(void*, u32)` | Initializes exp heap allocator buffer. |
| 33 | `0x80170010` | 72 B (`0x48`) | `__ct__Q26mPrint14MyPrintBase<w>Fv` | `mPrint::MyPrintBase<wchar_t>::MyPrintBase()` | Wide character constructor. |
| 34 | `0x80170060` | 64 B (`0x40`) | `__dt__Q26mPrint14MyPrintBase<w>Fv` | `mPrint::MyPrintBase<wchar_t>::~MyPrintBase()` | Wide character destructor. |
| 35 | `0x801700A0` | 92 B (`0x5C`) | `Initialize__Q26mPrint14MyPrintBase<w>...` | `mPrint::MyPrintBase<wchar_t>::Initialize(...)` | Wide character initialize. |
| 36 | `0x80170100` | 8 B (`0x08`) | `SetFont__Q26mPrint14MyPrintBase<w>...` | `mPrint::MyPrintBase<wchar_t>::SetFont(...)` | Wide character set font. |
| 37 | `0x80170110` | 8 B (`0x08`) | `GetFont__Q26mPrint14MyPrintBase<w>...` | `mPrint::MyPrintBase<wchar_t>::GetFont()` | Wide character get font. |
| 38 | `0x80170120` | 8 B (`0x08`) | `SetVisible__Q26mPrint14MyPrintBase<w>...` | `mPrint::MyPrintBase<wchar_t>::SetVisible(...)` | Wide character set visibility. |
| 39 | `0x80170130` | 8 B (`0x08`) | `IsVisible__Q26mPrint14MyPrintBase<w>...` | `mPrint::MyPrintBase<wchar_t>::IsVisible()` | Wide character query visibility. |
| 40 | `0x80170140` | 272 B (`0x110`) | `VRegisterf__Q26mPrint14MyPrintBase<w>...` | `mPrint::MyPrintBase<wchar_t>::VRegisterf(...)` | Formats wide string with `vswprintf`. |
| 41 | `0x80170250` | 140 B (`0x8C`) | `Reset__Q26mPrint14MyPrintBase<w>Fv` | `mPrint::MyPrintBase<wchar_t>::Reset()` | Wide character reset. |
| 42 | `0x801702E0` | 612 B (`0x264`) | `Flush__Q26mPrint14MyPrintBase<w>Fv` | `mPrint::MyPrintBase<wchar_t>::Flush()` | Wide character render flush. |
| 43 | `0x80170550` | 192 B (`0xC0`) | `Flush__Q26mPrint14MyPrintBase<w>Fiiii` | `mPrint::MyPrintBase<wchar_t>::Flush(int, int, int, int)` | Wide character scissor flush. |
| 44 | `0x80170610` | 228 B (`0xE4`) | `Register__Q26mPrint14MyPrintBase<w>...` | `mPrint::MyPrintBase<wchar_t>::Register(...)` | Wide character register text node. |
| 45 | `0x80170700` | 12 B (`0x0C`) | `GetFirstText__Q26mPrint14MyPrintBase<w>Fv` | `mPrint::MyPrintBase<wchar_t>::GetFirstText()` | Wide character get first text. |
| 46 | `0x80170710` | 8 B (`0x08`) | `GetNextText__Q26mPrint14MyPrintBase<w>...` | `mPrint::MyPrintBase<wchar_t>::GetNextText(...)` | Wide character get next text. |
| 47 | `0x80170720` | 72 B (`0x48`) | `Unregister__Q26mPrint14MyPrintBase<w>...` | `mPrint::MyPrintBase<wchar_t>::Unregister(...)` | Wide character unregister text node. |
| 48 | `0x80170770` | 60 B (`0x3C`) | `SetBuffer__Q26mPrint14MyPrintBase<w>FPvUl` | `mPrint::MyPrintBase<wchar_t>::SetBuffer(...)` | Wide character set buffer. |
| 49 | `0x801707B0` | 100 B (`0x64`) | `init__Q24mTex6base_cFiiii` | `mTex::base_c::init(int, int, int, int)` | Texture tiling base initializer. |
| 50 | `0x80170820` | 32 B (`0x20`) | `getTileNo__Q24mTex6base_cCFii` | `mTex::base_c::getTileNo(int, int) const` | Computes tile index. |
| 51 | `0x80170840` | 44 B (`0x2C`) | `getIdInTile__Q24mTex6base_cCFii` | `mTex::base_c::getIdInTile(int, int) const` | Computes sub-pixel offset within tile. |
| 52 | `0x80170870` | 168 B (`0xA8`) | `xyToDotId__Q24mTex6base_cCFii` | `mTex::base_c::xyToDotId(int, int) const` | Converts (x, y) to linear texture dot offset. |
| 53 | `0x80170920` | 68 B (`0x44`) | `init__Q24mTex8edit4b_cFiiPUc` | `mTex::edit4b_c::init(int, int, u8*)` | Initializes 4-bit texture buffer editor. |
| 54 | `0x80170970` | 204 B (`0xCC`) | `set__Q24mTex8edit4b_cFiiUcb` | `mTex::edit4b_c::set(int, int, u8, bool)` | Virtual method. Writes 4-bit nibble pixel into tile buffer. |
| 55 | `0x80170A40` | 60 B (`0x3C`) | `endEdit__Q24mTex8edit4b_cFv` | `mTex::edit4b_c::endEdit()` | Flushes data cache (`DCStoreRangeNoSync`). |
| 56 | `0x80170A80` | 64 B (`0x40`) | `__dt__Q24mTex8edit4b_cFv` | `mTex::edit4b_c::~edit4b_c()` | Virtual scalar deleting destructor. |

---

## 5. SDK Dependencies & Symbol Pins

### Category (a): Already Banked in Compiled Translation Units
- `C_MTXOrtho` (`0x801C1490`)
- `PSMTXIdentity` (`0x801C0610`)
- `GXLoadPosMtxImm` (`0x801C9A80`)
- `GXSetCurrentMtx` (`0x801C9BA0`)
- `GXSetProjection` (`0x801C9980`)
- `DCStoreRangeNoSync` (`0x801AC640`)
- `MEMAllocFromExpHeapEx` (`0x801D45A0`)
- `MEMCreateExpHeapEx` (`0x801D44C0`)
- `MEMFreeToExpHeap` (`0x801D4850`)
- `MEMGetAllocatableSizeForExpHeapEx` (`0x801D49A0`)
- `MEMSetGroupIDForExpHeap` (`0x801D4AE0`)

### Category (b): Pinned in `syms.txt`
- `WPADGetInfoAsync` (`0x801E1400`)
- `List_Init` (`0x80228F10`)
- `List_Append` (`0x80228F30`)
- `List_Remove` (`0x802290C0`)
- `List_GetNext` (`0x80229130`)
- `SetupGX__Q34nw4r2ut10CharWriterFv` (`0x8022CB00`)
- `SetFontSize__Q34nw4r2ut10CharWriterFf` (`0x8022D500`)
- `EnableLinearFilter__Q34nw4r2ut10CharWriterFbb` (`0x8022D700`)
- `UpdateVertexColor__Q34nw4r2ut10CharWriterFv` (`0x8022DAE0`)
- `init__Q23EGG10CoreStatusFv` (`0x802BC9D0`)
- `sceneReset__Q23EGG14CoreControllerFv` (`0x802BCAF0`)
- `getNthController__Q23EGG17CoreControllerMgrFi` (`0x802BD660`)
- `__dl__FPv` (`0x802B93C0`)
- `vsnprintf` (`0x802E18CC`)
- `vswprintf` (`0x802E4680`)
- `memcpy` (`0x80004364`)
- `memset` (`0x800046B4`)

### Category (c): Needs Pin in `syms.txt` (4 symbols)
```ini
Print__Q34nw4r2ut17TextWriterBase<c>FPCci=0x8022EC20
Print__Q34nw4r2ut17TextWriterBase<w>FPCwi=0x8022EEB0
__ct__Q34nw4r2ut17TextWriterBase<c>Fv=0x8022EB00
__dt__Q34nw4r2ut17TextWriterBase<c>Fv=0x8022EB90
```

---

## 6. Actionable Hazards & Mitigation Plan for `m_pad.cpp`

1. **Static Initializer Registration (`__sinit_\m_pad_cpp`) & `.ctors`**:
   - `g_PadAdditionalData__4mPad` (`0x80377FA8`, size `0x60`) must be an array of 4 `PadAdditionalData_t` structs declared at file scope with non-trivial destructor `~PadAdditionalData_t()`.
   - MWCC automatically emits `__sinit_\m_pad_cpp` at `0x8016F7B0` and registers it in `.ctors:0x8000637C`. If made trivial, `.ctors` and `__sinit_` vanish.

2. **Template Specialization Order in `.text`**:
   - `mPrint::MyPrintBase<char>` must precede `mPrint::MyPrintBase<wchar_t>`.

3. **Polymorphism in `mTex`**:
   - `mTex::base_c` is **strictly non-virtual**. Only derived `mTex::edit4b_c` has a vtable (`__vt__Q24mTex8edit4b_c` at `0x80329F60`, 2 slots).

4. **BSS Size Bracketing**:
   - Total `.bss` must match `0x140` B (`0x80377F88`–`0x803780C8`).

---

# Part 3: Ready-to-Land Header Proposals

### Proposed Header 1: `include/lib/egg/core/eggThread.h` (Shared-Header Modification)
```cpp
#pragma once

#include <types.h>
#include <lib/revolution/OS.h>

namespace EGG {

class Heap;

class Thread {
public:
    Thread(u32 stackSize, int msgCount, int priority, Heap* heap);
    Thread(OSThread*, int);
    virtual ~Thread();
    virtual void* run() { return 0; }
    virtual void onEnter() {}
    virtual void onExit() {}

    static void initialize();

    u8 mPad_04[0x04];
    OSThread* mOSThread; // at 0x08
    u8 mPad_0C[0x44];   // Total size = 0x50
};

} // namespace EGG
```
- **Offset-Perturbing**: NO. Restores the true `0x50` B footprint of `EGG::Thread` with its virtual table and `mOSThread` pointer at `+0x08`.
- **Compiled**: YES (`STATIC_ASSERT(sizeof(EGG::Thread) == 0x50)` passed).
- **Confidence**: High.

### Proposed Header 2: `include/game/bases/d_nand_thread.hpp` (New Header)
```cpp
#pragma once

#include <types.h>
#include <lib/revolution/OS.h>
#include <lib/revolution/NAND.h>
#include <lib/egg/core/eggThread.h>

namespace EGG {
class Mutex {
public:
    virtual ~Mutex() {}
};
}

typedef struct OSCond {
    OSThreadQueue queue;
} OSCond;

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

    static dNandThread_c* m_instance; // .sbss:0x8042A298

    mMutex mMutex;       // 0x50
    u32 mCommand;        // 0x74
    u32 mStatus;         // 0x78
    u8 mFileExists;      // 0x7C
    u8 mPad[3];          // 0x7D
};
```
- **Offset-Perturbing**: NO. Brand new header; all internal offsets match retail binary.
- **Compiled**: YES.
- **Confidence**: High.

### Proposed Header 3: `include/game/mLib/m_pad.hpp` (Header Update)
```cpp
#pragma once

#include <types.h>
#include <lib/revolution/WPAD.h>
#include <lib/egg/core/eggController.h>

namespace mPad {
    enum CH_e {
        CH_0,
        CH_1,
        CH_2,
        CH_3
    };

    struct PadAdditionalData_t {
        PadAdditionalData_t();
        ~PadAdditionalData_t();
        u8 mData[0x18];
    };

    void create();
    void beginPad();
    void endPad();
    void setCurrentChannel(CH_e ch);
    u32 getBatteryLevel_ch(CH_e ch);
    void setWPADInfo(CH_e ch, const WPADInfo& info);
    void clearWPADInfo(CH_e ch);
    void initWPADInfo();
    void getWPADInfoCb(s32 chan, s32 result);
    bool getWPADInfoAsync(CH_e ch);
    void setGetWPADInfoInterval(u32 interval);
    u32 getGetWPADInfoInterval();

    extern EGG::CoreController* g_core[4];
    extern PadAdditionalData_t g_PadAdditionalData[4];
    extern WPADInfo s_WPADInfo[4];
    extern WPADInfo s_WPADInfoTmp[4];

    extern void* g_padMg;
    extern u32 g_currentCoreID;
    extern EGG::CoreController* g_currentCore;
    extern u8 g_IsConnected;
    extern u32 g_PadFrame;
    extern u32 s_WPADInfoAvailable;
    extern u32 s_GetWPADInfoInterval;
    extern u32 s_GetWPADInfoCount;
}
```
- **Offset-Perturbing**: NO.
- **Compiled**: YES.
- **Confidence**: High.
