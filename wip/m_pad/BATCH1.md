# `m_pad.cpp` -- batch 1: header reconstruction + the six simplest functions

All six functions in this batch compile byte-exact against the target. Every
claim below was compiled and diffed through `tools/auto_decomp/harness.py`'s
`compile_draft` / `disasm` / `diff_fn`, not reasoned from disassembly alone.
Work is in `wip/m_pad/scratch/batch1/` (`inc/` = shadow headers, `m_pad.cpp` =
draft, `test_endpad.py` = the compile+diff driver I used, imports `harness.py`
directly instead of going through the model-loop CLI).

## What the other two batches need to know

1. **`setCurrentChannel` returns `CH_e`, not `void`.** The disassembly loads
   the *old* `g_currentCoreID` directly into r3 before overwriting it, and
   nothing touches r3 again before `blr`. That is not leftover-register
   garbage (AGENT_CONTEXT's "garbage in r3" case) -- it is a clean, deliberate
   load with no later clobber, i.e. `return old;`. Tested: a `void` variant
   compiles to 7 instructions with no load of the old value at all; the target
   is 9. Both forms below are real measurements, not reasoning:
   ```
   void variant (WRONG):  size: target 9, draft 7
     0 | want: mr r5, r3                  got: lis r4, g_core__4mPad@ha
     ...missing the old-value load and two trailing stores entirely
   CH_e variant (RIGHT):  MATCH
   ```
   This is the single most important correction in this batch -- the original
   stub header didn't declare this function at all, so there was no prior
   wrong answer to fix, but it would have been an easy one to get wrong.

2. **`EGG::CoreControllerMgr` needs a 0x10-byte non-polymorphic base before its
   own vtable, plus `virtual beginFrame()` / `virtual endFrame()`, plus a
   static `sInstance` member.** `endPad` tail-calls (`bctr`, no `blr`/stack
   frame) through `*(this+0x10)` then `+0xc` into that pointer -- i.e. the
   object's *own* vtable pointer sits at offset 0x10, not 0. That only happens
   in this compiler's layout when a non-virtual base class occupies the first
   0x10 bytes and `CoreControllerMgr` is the first class in the chain to
   declare virtuals. I proved the shape compiles byte-exact with:
   ```cpp
   class CoreControllerMgrTestBase { u8 mPad0x10[0x10]; };
   class CoreControllerMgr : public CoreControllerMgrTestBase {
   public:
       static void createInstance();
       static CoreControllerMgr *sInstance;
       static u32 sWPADWorkSize;
       virtual void beginFrame();
       virtual void endFrame();
   };
   ```
   This is a `beginPad` finding as much as an `endPad` one -- `beginPad` uses
   the identical `this+0x10` / slot `+0x8` pattern for `beginFrame`. I have
   **not** determined what the real 0x10-byte base actually is (tried
   `EGG::Disposer` mentally: it doesn't fit, because Disposer is itself
   polymorphic, so its vtable would be reused/extended at offset 0, not pushed
   to 0x10 -- I did not test this empirically, just reasoned it out, so treat
   it as a hypothesis I rejected on paper, not a tested negative). The 0x10
   raw-byte placeholder is intentionally opaque; `EGG::CoreControllerMgr` is
   not decompiled anywhere in this repo (`include/lib/egg/core/eggController.h`
   is a 2-member stub, and `grep -rl CoreControllerMgr source/` finds nothing),
   so per AGENT_CONTEXT this is exactly the documented case for a raw-offset
   comment rather than a named member. **I did not touch the real
   `eggController.h`** -- this is a shadow copy in my scratch include dir, and
   landing anything into the real header is out of this batch's charter and
   would need its own propose-and-verify round.

3. **The `.bss` ordering in the brief (g_core, unclaimed-0x10, g_PadAdditionalData,
   s_WPADInfo, s_WPADInfoTmp) will NOT fall out of declaration order alone.**
   MWCC orders unreferenced-by-initializer `.bss` objects by first-reference
   in the compiled `.text`, not by source declaration order. My 6 functions
   only reference `g_core` and `s_WPADInfo`/`s_WPADInfoAvailable`; they never
   touch `g_PadAdditionalData` or `s_WPADInfoTmp`, so compiling my subset alone
   produces a different (wrong, and expected-to-be-wrong) `.bss` layout than
   the target's. This is a whole-TU property -- it will only resolve once
   `beginPad` (first touches `g_core` then `g_PadAdditionalData`),
   `setWPADInfo`/`clearWPADInfo`/`getBatteryLevel_ch` (first touch
   `s_WPADInfo`), and `getWPADInfoCb`/`getWPADInfoAsync` (first touch
   `s_WPADInfoTmp`) are all present in one file, in that order. Per-function
   diffs stay valid regardless (dtk renders relocations symbolically, not as
   raw addresses), so this doesn't invalidate any MATCH claimed here -- it's
   only relevant once the three batches are merged and the full `.bss` region
   is diffed as a whole.

4. **Compiling `PadAdditionalData_t` with a declared-but-undefined
   constructor and destructor correctly produces `__sinit_\m_pad_cpp` and an
   `__arraydtor$NNNN` symbol** (numbers won't match the target's `$13953`
   until the real ctor/dtor bodies and all 16 functions are present -- pool
   numbering is TU-wide). This is a positive signal for whoever owns the
   ctor/dtor: the struct shape (6 floats, no padding, non-trivial special
   members) is consistent with what the compiler needs to see to emit those
   two symbols at all. One loose end I could not chase down without the real
   bodies: compiling my minimal 6-function subset also emits an extra,
   unexplained ~0x10-byte anonymous `.bss` object (`@1889` in my build, number
   meaningless) that has no counterpart in the target's complete data
   inventory. I believe this is an artifact of only having stub ctor/dtor
   declarations and 6 of 16 functions present, not a real problem, but I did
   not prove that -- flagging it rather than asserting it's fine.

5. A genuine contradiction with the SHARED-BRIEF, reported rather than
   resolved: the brief says the current header's `g_currentCore` "has the
   wrong name against the symbol map." I read
   `include/game/mLib/m_pad.hpp` directly and it declares exactly
   `extern EGG::CoreController *g_currentCore;`, which mangles to
   `g_currentCore__4mPad` -- that **is** the symbol map's name. What the
   header actually lacks is `g_currentCoreID` (a distinct, separate symbol).
   I'm treating "the wrong name" as loose phrasing for "the companion symbol
   is missing," but I can't reconcile it with what I actually read in the
   file, so both are stated here rather than silently picked.

## The proposed header

Shadow-copied and compiled at `wip/m_pad/scratch/batch1/inc/game/mLib/m_pad.hpp`
against `wip/m_pad/scratch/batch1/inc/lib/egg/core/eggController.h` (my shadow
extension of the real stub, NOT landed -- see finding 2). This is what I
propose for the real `include/game/mLib/m_pad.hpp`; the `eggController.h`
piece is shown separately below as a finding for whoever owns that class, not
as something to land now.

```cpp
#pragma once

#include <types.h>
#include <revolution/WPAD/WPAD.h>
#include <lib/egg/core/eggController.h>

namespace mPad {
    enum CH_e {
        MPAD_CH_0,
        MPAD_CH_1,
        MPAD_CH_2,
        MPAD_CH_3
    };

    /// @unofficial -- offsets 0x0/0x4/0x8/0xc/0x10/0x14 are solid (read directly
    /// out of beginPad's disassembly, which is not in this batch). Member names
    /// are a guess at semantics (looks like cur/prev/delta for a 2-axis value);
    /// the ctor/dtor bodies belong to a different batch and are declared only,
    /// not defined, here.
    struct PadAdditionalData_t {
        PadAdditionalData_t();
        ~PadAdditionalData_t();

        f32 mCurX;   // 0x0
        f32 mCurY;   // 0x4
        f32 mPrevX;  // 0x8
        f32 mPrevY;  // 0xc
        f32 mDeltaX; // 0x10
        f32 mDeltaY; // 0x14
    };
    STATIC_ASSERT(sizeof(PadAdditionalData_t) == 0x18);

    void create();
    void beginPad();
    void endPad();

    /// @returns the previous channel. Confirmed by codegen: the old
    /// g_currentCoreID value is loaded directly into r3 immediately before the
    /// stores, with no other use before blr. A void variant compiles to 7
    /// instructions with no load of the old value; the target is 9. Tested.
    CH_e setCurrentChannel(CH_e ch);

    /// @returns -1 (sentinel) if unavailable, else WPADInfo::battery. Codegen
    /// only proves the return is NOT u8 (li r3,-0x1 loads a full 0xFFFFFFFF;
    /// a u8 return of -1 would load 0xff instead -- tested, it does not match).
    /// s8/s16/s32/u32/u16 all produce byte-identical code for this function;
    /// s32 is chosen as the most idiomatic sentinel-bearing type and is
    /// unproven beyond ruling out u8/the-obvious-wrong-guess.
    s32 getBatteryLevel_ch(CH_e ch);

    void setWPADInfo(CH_e ch, const WPADInfo &info);
    void clearWPADInfo(CH_e ch);
    void initWPADInfo();
    void getWPADInfoAsync(CH_e ch);

    void setGetWPADInfoInterval(ulong interval);
    ulong getGetWPADInfoInterval();

    extern EGG::CoreController *g_currentCore;
    extern CH_e g_currentCoreID;
    extern EGG::CoreController *g_core[4];
};
```

### Globals NOT proposed for the header (file-private to `m_pad.cpp`)

No evidence any other TU references these -- only functions inside this one
TU touch them anywhere in the disassembly I've read. Defined directly in the
`.cpp`, declared `extern`-free (default external linkage; whether the real
code marked them `static` is unprovable from mangling and doesn't affect
codegen either way):

| symbol | proposed type | evidence |
|---|---|---|
| `g_padMg__4mPad` | `EGG::CoreControllerMgr *` | `create` reads `sInstance` into it; `endPad`/`beginPad` dispatch through it |
| `g_PadFrame__4mPad` | `u32` | incremented every `beginPad` call (not mine, brief-only) |
| `g_IsConnected__4mPad` | `bool[4]` | byte-indexed load/store, `+1`-incremented pointer in `beginPad`'s loop (not mine, brief-only) |
| `s_WPADInfoAvailable__4mPad` | `bool[4]` | byte-indexed by channel in `getBatteryLevel_ch`, `setWPADInfo`, `clearWPADInfo` -- confirmed by my compile |
| `s_GetWPADInfoInterval__4mPad` | `ulong` | matches the setter's `Ul`-mangled parameter; getter returns it verbatim -- confirmed by my compile (the cheapest probe in the batch, run first as instructed) |
| `s_GetWPADInfoCount__4mPad` | `u32` | `cmplw` (unsigned compare) against it in `beginPad` (not mine, brief-only) |
| `s_WPADInfo__4mPad` | `WPADInfo[4]` | `mulli r0,r3,0x18` indexing, `sizeof(WPADInfo)==0x18` already in `WPAD.h` -- confirmed by my compile |
| `s_WPADInfoTmp__4mPad` | `WPADInfo[4]` | same shape, used by `getWPADInfoCb`/`getWPADInfoAsync` (not mine, brief-only) |
| `g_PadAdditionalData__4mPad` | `PadAdditionalData_t[4]` | see struct above (not mine, brief-only) |
| `pad_80377F98` (no real symbol) | `u8[0x10]` | **unclaimed**, see below |

**The unclaimed 0x10 gap** between `g_core` (ends `0x80377F98`) and
`g_PadAdditionalData` (starts `0x80377FA8`): no symbol in the map, and none of
my six functions reference it. I'm proposing a plain `u8 pad_80377F98[0x10];`
placeholder purely to keep the `.bss` total correct if this gets compiled in
isolation; I have no evidence for its real shape. Whoever lands `beginPad` or
the WPADInfo family should check whether their functions touch this address
before accepting the placeholder.

## Status table

| # | address | target instrs | mine | result |
|---|---|---|---|---|
| 1 | `0x8016F7A0` | 2 (`0x8`) | 2 | **MATCH** |
| 2 | `0x8016F780` | 5 (`0x14`) | 5 | **MATCH** |
| 3 | `0x8016F550` | 5 (`0x14`) | 5 | **MATCH** |
| 4 | `0x8016F570` | 9 (`0x24`) | 9 | **MATCH** |
| 5 | `0x8016F330` | 12 (`0x30`) | 12 | **MATCH** |
| 6 | `0x8016F5A0` | 12 (`0x30`) | 12 | **MATCH** |

All six MATCH byte-exact via `harness.diff_fn`. Verified with
`compile_draft` + `disasm` + `diff_fn` per function; no `ninja`/`configure.py`/
`progress.py`/`land.py` was run.

## The source

`wip/m_pad/scratch/batch1/m_pad.cpp` (draft; compiled against the shadow
header above via `-i wip/m_pad/scratch/batch1/inc`):

```cpp
#include <types.h>
#include <game/mLib/m_pad.hpp>

namespace mPad {
    // ---- public API globals (declared in m_pad.hpp) ------------------------
    EGG::CoreController *g_currentCore;
    CH_e g_currentCoreID;
    EGG::CoreController *g_core[4];

    // ---- file-private bookkeeping (not exposed via the header) -------------
    // No evidence any other TU references these; only functions inside this
    // TU touch them in the disassembly seen so far. @unofficial linkage call.
    EGG::CoreControllerMgr *g_padMg;
    u32 g_PadFrame;
    bool g_IsConnected[4];
    bool s_WPADInfoAvailable[4];
    ulong s_GetWPADInfoInterval;
    u32 s_GetWPADInfoCount;
    WPADInfo s_WPADInfo[4];
    WPADInfo s_WPADInfoTmp[4];
    PadAdditionalData_t g_PadAdditionalData[4];

    // Unclaimed 0x10 gap in .bss between g_core (ends 0x80377F98) and
    // g_PadAdditionalData (starts 0x80377FA8). No symbol in the map, and none
    // of the six functions in this batch reference it. @unofficial placeholder
    // only -- shape unknown, only the total size (0x10) is evidenced.
    u8 pad_80377F98[0x10];

    // ---- this batch's six functions -----------------------------------------

    void endPad() {
        g_padMg->endFrame();
    }

    void create() {
        g_padMg = EGG::CoreControllerMgr::sInstance;
        initWPADInfo();
        beginPad();
        endPad();
    }

    CH_e setCurrentChannel(CH_e ch) {
        CH_e old = g_currentCoreID;
        g_currentCoreID = ch;
        g_currentCore = g_core[ch];
        return old;
    }

    s32 getBatteryLevel_ch(CH_e ch) {
        if (!s_WPADInfoAvailable[ch])
            return -1;
        return s_WPADInfo[ch].battery;
    }

    void setGetWPADInfoInterval(ulong interval) {
        s_GetWPADInfoInterval = interval;
        if (interval == 0)
            initWPADInfo();
    }

    ulong getGetWPADInfoInterval() {
        return s_GetWPADInfoInterval;
    }
}
```

Not landed anywhere -- `slices/wiimj2d.json`, `syms.txt`, and the real headers
are all untouched. Draft and shadow headers are in
`wip/m_pad/scratch/batch1/` for the lead to pull from.
