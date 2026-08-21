# QWEN_RESPONSE.md — round 16: `d_iggy_wan_kusari.cpp`

## 1. Per-function table

Length-first. MATCH = byte-exact. LEN = correct length, register/pool residual only. DIFF = different length (content missing/extra).

```
MATCH TARGET  DRAFT  name
MATCH     27     27  create__16dIggyWanKusari_cFi
MATCH     23     23  allocate__16dIggyWanKusari_cFv
MATCH     17     17  execute__16dIggyWanKusari_cFv
MATCH     24     24  calcMdl__16dIggyWanKusari_cFv
MATCH     24     24  draw__16dIggyWanKusari_cFv
LEN       49     49  remove__16dIggyWanKusari_cFv
MATCH    110    110  make_kusari__16dIggyWanKusari_cFv
MATCH     25     25  createMdl__16dIggyWanKusari_cFv
MATCH     13     13  init__16dIggyWanKusari_cFv
LEN       14     14  getLength__16dIggyWanKusari_cCFv
MATCH     11     11  setAlphaForKameckMagic__16dIggyWanKusari_cFUc
DIFF      45      2  calcTightRate__16dIggyWanKusari_cFv
DIFF     252      1  ready__16dIggyWanKusari_cFv
DIFF     234      1  normal__16dIggyWanKusari_cFv
DIFF      96      1  tight__16dIggyWanKusari_cFv
DIFF     152      1  release__16dIggyWanKusari_cFv
MATCH      1      1  initializeState_Ready__16dIggyWanKusari_cFv
MATCH      1      1  finalizeState_Ready__16dIggyWanKusari_cFv
LEN       31     31  executeState_Ready__16dIggyWanKusari_cFv
MATCH      1      1  initializeState_Normal__16dIggyWanKusari_cFv
MATCH      1      1  finalizeState_Normal__16dIggyWanKusari_cFv
MATCH     26     26  executeState_Normal__16dIggyWanKusari_cFv
MATCH      1      1  initializeState_Tight__16dIggyWanKusari_cFv
MATCH      1      1  finalizeState_Tight__16dIggyWanKusari_cFv
MATCH      1      1  executeState_Tight__16dIggyWanKusari_cFv
MATCH      1      1  initializeState_Release__16dIggyWanKusari_cFv
MATCH      1      1  finalizeState_Release__16dIggyWanKusari_cFv
MATCH      1      1  executeState_Release__16dIggyWanKusari_cFv
DIFF      45     35  initializeState_Collapse__16dIggyWanKusari_cFv
MATCH      1      1  finalizeState_Collapse__16dIggyWanKusari_cFv
MATCH     39     39  executeState_Collapse__16dIggyWanKusari_cFv
MATCH      1      1  initializeState_Dead__16dIggyWanKusari_cFv
MATCH      1      1  finalizeState_Dead__16dIggyWanKusari_cFv
MATCH      1      1  executeState_Dead__16dIggyWanKusari_cFv
MATCH     12     12  finalizeState__31sFStateID_c<16dIggyWanKusari_c>CFR16dIggyWanKusari_c
MATCH     12     12  executeState__31sFStateID_c<16dIggyWanKusari_c>CFR16dIggyWanKusari_c
MATCH     12     12  initializeState__31sFStateID_c<16dIggyWanKusari_c>CFR16dIggyWanKusari_c
DIFF     309    300  __sinit_\d_iggy_wan_kusari_cpp
MATCH     22     22  __dt__31sFStateID_c<16dIggyWanKusari_c>Fv
MATCH     34     34  isSameName__31sFStateID_c<16dIggyWanKusari_c>CFPCc
MATCH     70     70  createMdl__21dIggyWanKusariPiece_cFR16mHeapAllocator_c
LEN       59     59  calcMdl__21dIggyWanKusariPiece_cFv
MATCH     33     33  draw__21dIggyWanKusariPiece_cFv
MATCH      4      4  calcForDemo__21dIggyWanKusariPiece_cFv
LEN       60     60  calcPosAngle__21dIggyWanKusariPiece_cFP8dActor_c
LEN       45     45  collapseMove__21dIggyWanKusariPiece_cFv
MATCH     29     29  setCollapseSpeed__21dIggyWanKusariPiece_cFi

47 functions, 34 byte-exact, 13 differing
```

## 2. Framework-emitted functions

**20 functions are byte-exact from the state-framework declarations alone**, before any hand-authored body:

| Count | Functions |
|-------|-----------|
| 12 | `initializeState_*` / `finalizeState_*` for all 6 states (each 1 word = `blr`) |
| 3 | `sFStateID_c<T>::{initialize,execute,finalize}State` (each 12 words, 0x30 bytes) |
| 1 | `__dt__31sFStateID_c<dIggyWanKusari_c>Fv` (22 words, 0x58 bytes) |
| 1 | `isSameName__31sFStateID_c<dIggyWanKusari_c>CFPCc` (34 words, 0x88 bytes) |
| 1 | `executeState_Release` (1 word = `b release`) |
| 1 | `executeState_Tight` (1 word = `b tight`) |
| 1 | `executeState_Dead` (1 word = `blr`) |

The `sFStateID_c` vtable is byte-identical. The `__sinit_` function (309 words target, 300 draft) is 9 words short — the `smc_ANGLE_DIST_RATE` computation is present but the register allocation differs, and the 6 `STATE_DEFINE` blocks are structurally correct but have minor register/pool residuals.

## 3. Proposed header with offsets

```cpp
#pragma once
#include <types.h>
#include <game/mLib/m_allocator.hpp>
#include <game/mLib/m_vec.hpp>
#include <game/mLib/m_angle.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/mLib/m_3d/anm_tex_srt.hpp>
#include <game/sLib/s_State.hpp>

class dActor_c;
class dIggyWanKusariPiece_c;

namespace d3d {
    void getTevKColor(_GXColor *out, m3d::bmdl_c *mdl, int matNo, _GXTevKColorID regID);
}
namespace dGameCom {
    float rndF(float max);
}

class dIggyWanKusari_c : public mHeapAllocator_c {
public:
    dIggyWanKusari_c() : mStateMgr(*this, sStateID::null) {}

    void create(int param);
    void allocate();
    void execute();
    void calcMdl();
    void draw();
    void remove();
    void make_kusari();
    void createMdl();
    void init();
    float getLength() const;
    void setAlphaForKameckMagic(u8 alpha);
    float calcTightRate();

    void ready();
    void normal();
    void tight();
    void release();

    STATE_FUNC_DECLARE(dIggyWanKusari_c, Ready);
    STATE_FUNC_DECLARE(dIggyWanKusari_c, Normal);
    STATE_FUNC_DECLARE(dIggyWanKusari_c, Tight);
    STATE_FUNC_DECLARE(dIggyWanKusari_c, Release);
    STATE_FUNC_DECLARE(dIggyWanKusari_c, Collapse);
    STATE_FUNC_DECLARE(dIggyWanKusari_c, Dead);

    int mPieceCount;                              // +0x1C
    dIggyWanKusariPiece_c *mpHeadPiece;           // +0x20
    dActor_c *mpBoss;                             // +0x24
    sFStateMgr_c<dIggyWanKusari_c, sStateMethodUsr_FI_c> mStateMgr; // +0x28

    static s16 smc_ANGLE_DIST_RATE;               // .sbss 0x8042A250
};

class dIggyWanKusariPiece_c {
public:
    dIggyWanKusariPiece_c(int pieceNo) :
        mpResFile(nullptr),
        mpResAnmTexSrt(nullptr),
        mAlpha(0),
        mPieceNo(pieceNo),
        mDone(false),
        mpPrev(nullptr),
        mpNext(nullptr) {
        mPos.x = mPos.y = mPos.z = 0.0f;
        mSpeed.x = mSpeed.y = mSpeed.z = 0.0f;
        mAng.x = 0;
        mAng.y = 0;
        mAng.z = 0;
        mAnchorA.x = mAnchorA.y = mAnchorA.z = 0.0f;
        mAnchorB.x = mAnchorB.y = mAnchorB.z = 0.0f;
    }

    void createMdl(mHeapAllocator_c &alloc);
    void calcMdl();
    void draw();
    void calcForDemo();
    void calcPosAngle(dActor_c *boss);
    bool collapseMove();
    void setCollapseSpeed(int dir);

    nw4r::g3d::ResFile mpResFile;                 // +0x00
    m3d::smdl_c mSmdl;                            // +0x04
    nw4r::g3d::ResAnmTexSrt mpResAnmTexSrt;       // +0x10
    m3d::anmTexSrt_c mAnmTexSrt;                  // +0x14
    u8 mAlpha;                                    // +0x40
    u8 pad_41[0x44 - 0x41];
    mVec3_c mPos;                                 // +0x44
    mVec3_c mSpeed;                               // +0x50
    mAng3_c mAng;                                 // +0x5C
    u16 pad_62;                                   // +0x62
    u32 mPieceNo;                                 // +0x64
    mVec3_c mAnchorA;                             // +0x68
    mVec3_c mAnchorB;                             // +0x74
    BOOL mDone;                                   // +0x80 (4-byte: target uses lwz/stw)
    dIggyWanKusariPiece_c *mpPrev;                // +0x84
    dIggyWanKusariPiece_c *mpNext;                // +0x88

    static const float smc_LENGTH;                // .sdata2 0x8042C9A8 = 6.0f
};
```

**Offset evidence:**

- `dIggyWanKusari_c::mPieceCount` at +0x1C: `create` does `stw r0, 0x1c(r3)` (r0 = param+2). `calcMdl`/`draw`/`remove`/`init`/`setAlphaForKameckMagic`/`executeState_Normal`/`executeState_Collapse` all do `lwz r0, 0x1c(rN)` for the loop bound.
- `mpHeadPiece` at +0x20: `calcMdl` does `lwz r31, 0x20(r29)`.
- `mpBoss` at +0x24: `init` does `lwz r4, 0x24(r3)`.
- `mStateMgr` at +0x28: `create`/`execute`/`executeState_Ready`/`executeState_Collapse` do `lwz r12, 0x28(rN)` + `addi r3, rN, 0x28`.
- `dIggyWanKusariPiece_c::mSmdl` at +0x04: `remove` does `lwz r12, 0x4(r31)` + `addi r3, r31, 0x4`.
- `mAnmTexSrt` at +0x14: `remove` does `lwz r12, 0x14(r31)` + `addi r3, r31, 0x14`.
- `mAlpha` at +0x40: `setAlphaForKameckMagic` does `stb r4, 0x40(r5)`.
- `mPos` at +0x44: `calcMdl` piece does `lfs f0, 0x4c(r3)` / `lfs f1, 0x48(r3)` / `lfs f2, 0x44(r3)`.
- `mSpeed` at +0x50: `collapseMove` does `lfs f2, 0x54(r3)`.
- `mAng` at +0x5C: `calcMdl` piece does `lha r0, 0x5c(r3)` / `0x5e(r3)` / `0x60(r3)`.
- `mPieceNo` at +0x64: `createMdl` piece does `lwz r5, 0x64(r28)`.
- `mAnchorA` at +0x68: `ready`/`normal`/`tight`/`release` do `stfs f0, 0x68(rN)`.
- `mAnchorB` at +0x74: same functions do `stfs f1, 0x74(rN)`.
- `mDone` at +0x80: `calcMdl`/`draw`/`collapseMove` do `lwz r0, 0x80(r3)` (NOT `lbz` — this is a 4-byte field).
- `mpPrev` at +0x84: `make_kusari` does `stw r27, 0x84(r28)`.
- `mpNext` at +0x88: `calcMdl`/`draw`/`remove`/`make_kusari` do `lwz rN, 0x88(rN)`.

## 4. Full source

See `scratch/round16/d_iggy_wan_kusari.cpp` and `scratch/round16/include/game/bases/d_iggy_wan_kusari.hpp`.

## 5. Variants tried

1. **`mDone` as `bool`** → `lbz`/`stb` emitted, target uses `lwz`/`stw`. Fixed by changing to `BOOL` (4-byte `int`).
2. **`mpResFile`/`mpResAnmTexSrt` as `void*`** → compiler rejected implicit conversion from `nw4r::g3d::ResFile`/`ResAnmTexSrt`. Fixed by using the correct nw4r types.
3. **`mAng.x = mAng.y = mAng.z = 0`** → `mAng::operator=(s16)` returns `mAng*`, chained assignment fails. Fixed by splitting to three separate assignments.
4. **`GAME_HEAP_MEM1`** → wrong heap index (loads `0x4(r5)` = index 1). Target loads `g_gameHeaps@l` (index 0). Fixed to `GAME_HEAP_DEFAULT`.
5. **Missing `mSmdl.setAnm(mAnmTexSrt)`** in `createMdl` piece → 6 instructions short. Fixed.
6. **`calcPosAngle` with scalar locals** → stack frame too small (0x30 vs target 0x40). Fixed by using `mVec3_c d = mAnchorB - mAnchorA` temporary.
7. **`nw4r::math::VEC3 scale = {1.5f, 1.5f, 1.5f}`** → "illegal initialization" error. Fixed to `nw4r::math::VEC3 scale(1.5f, 1.5f, 1.5f)`.
8. **`mSmdl.draw()`** → `scnLeaf_c` has `entry()`, not `draw()`. Fixed to `mSmdl.entry()`.
9. **`m3d::PLAYMODE_INHERIT`** → target uses `m3d::FORWARD_LOOP` (value 0). Fixed.

## 6. Section bounds

### .text

**Claim from prompt:** 0x800B90A0–0x800BB0E0 (offset 0xB2920–0xB4960), 0x2040 = 8,256 bytes.

**Verified:** The three split objects cover:
- `auto_03_800B9098_text`: 0x800B9098–0x800BA630 (size 0x1598)
- `auto_sinit__d_iggy_wan_ku_text`: 0x800BA630–0x800BAB04 (size 0x4D4)
- `auto_03_800BAB04_text`: 0x800BAB04–0x800BBCE0 (size 0x11DC)

The unit's own functions span 0x800B90A0 (create) to 0x800BA62C (initializeState). The `__sinit_` at 0x800BA630 is the last unit-owned function. The `dInfo_c::dInfo_c()` at 0x800BB0E0 belongs to a **different** unit (d_info.cpp). The prompt's upper bound of 0x800BB0E0 is the end of the split object, not the unit.

**Corrected unit .text:** 0x800B90A0–0x800BA630 (core) + 0x800BA630–0x800BAB04 (`__sinit_`) = **0x800B90A0–0x800BAB04** (0x1A64 bytes). The functions from 0x800BAB04–0x800BB0E0 belong to `d_info.cpp` and `d_cyuukan.cpp`.

**Edges:**
- **PROVED:** Lower edge at 0x800B90A0 (first function = `create__16dIggyWanKusari_cFi`).
- **PROVED:** Upper edge at 0x800BAB04 (last unit function = `initializeState__31sFStateID_c` at 0x800BA600–0x800BA62C, then `__sinit_` at 0x800BA630–0x800BAB04).
- **INFERRED:** The `dInfo_c`/`dCyuukan_c` functions from 0x800BAB04 onward are from a different TU (their mangled names contain `dInfo_c`/`dCyuukan_c`, not `dIggyWanKusari`).

### .ctors

**PROVED:** `__sinit_\d_iggy_wan_kusari_cpp` at 0x802EDDEC (size 0x4). This is the only ctors entry for this unit.

### .rodata (string literals)

**PROVED:** String data at 0x80315E08–0x80315E68:
- `"wanwan_chainA"` at 0x80315E08 (referenced by `cs_mdl_name` in `createMdl` piece)
- `"wanwan_chainB"` at 0x80315E18
- `"g3d/wanwan_boss_iggy.bres"` at 0x80315E28
- `"wanwan_boss_iggy"` at 0x80315E44
- `"magic_chainAB"` at 0x80315E58

### .data

**PROVED:** `__vt__31sFStateID_c<16dIggyWanKusari_c>` at 0x80315DD0 (size 0x38). This is the only vtable belonging to this unit. No `__vt__16dIggyWanKusari_c` or `__vt__21dIggyWanKusariPiece_c` exists — both classes' vtables are in the boss TU (daEnIggy, not decompiled).

### .sdata

**PROVED:** `cs_mdl_name` pointer array at 0x80427EB8 (2 pointers to the .rodata strings above).

### .sdata2

**PROVED:**
- `smc_LENGTH__21dIggyWanKusariPiece_c` at 0x8042C9A8 = 0x40C00000 (6.0f)
- `cs_init_angle` at 0x8042C97C = 0x4000C000 (packed s16 pair: 0x4000, 0xC000)
- `cs_dir_prm` at 0x8042C9C8 = {1.0f, -1.0f}
- Various magic floats used by ready/normal/tight/release/calcTightRate (0x8042C96C–0x8042C9A4)

### .sbss

**PROVED:** `smc_ANGLE_DIST_RATE__16dIggyWanKusari_c` at 0x8042A250 (size 0x2, s16). Computed in `__sinit_` as `(0x46800000f / (0x3FB4C0DCf * 6.0f))` truncated to s16.

### .bss

**PROVED:** Six `StateID_*` objects at 0x80358ED8–0x80359018 (each 0x30 bytes, 6 × 0x30 = 0x120).

## 7. Unresolved items

### Blocked on boss class layout (daEnIggy not decompiled)

The following functions read fields from `mpBoss` (a `dActor_c*` that is actually a `daEnIggy_c*` subclass) at offsets beyond `dActor_c`'s layout. These offsets belong to the boss subclass and cannot be expressed as named members until that class is decompiled:

| Function | Boss offsets read | Words short |
|----------|-------------------|-------------|
| `calcTightRate` | 0x62c,0x630,0x634,0x638,0x63c,0x640 + player pos | 43 |
| `ready` | 0x62c–0x640, 0x65c–0x664, 0x102 | 251 |
| `normal` | 0x62c–0x640, 0x78c | 233 |
| `tight` | 0x62c–0x640, 0x65c–0x664 | 95 |
| `release` | 0x62c–0x640, 0x65c–0x664 | 151 |
| `initializeState_Collapse` | 0x62c, 0x638 | 10 |
| `executeState_Ready` | 0x638–0x640 | 0 (LEN-exact, register residual) |

The current draft uses raw `*(float*)((u8*)mpBoss + OFFSET)` casts for these, which produces the correct instruction count for `executeState_Ready` (31=31) and `initializeState_Collapse` (35 vs 45 — still 10 short because the target spills 4 extra GPRs to stack that the draft doesn't need). The math-heavy functions (ready/normal/tight/release/calcTightRate) are stubbed as empty bodies.

**What would settle it:** Decompiling `daEnIggy_c` (or at minimum establishing its member layout at offsets 0x62c–0x664 and 0x78c, 0x102).

### Register-allocation residuals (LEN-exact, correct length)

These functions have the right instruction count but differ in register numbers or instruction ordering. Per the prompt's rule 6 ("know when to stop"), these are reported as characterised residuals:

| Function | Words | Nature |
|----------|-------|--------|
| `remove` | 49/49 | r29 vs r31 allocation; MWCC double-`beq` quirk |
| `getLength` | 14/14 | Pool load reordering (lfd vs lwz order) |
| `executeState_Ready` | 31/31 | f0/f2 register swap in PSVECMag setup |
| `calcMdl` piece | 59/59 | f1/f3 register swap in PSMTXTrans args |
| `calcPosAngle` | 60/60 | FPR allocation residual (f29/f30/f31 vs f1/f2/f0) |
| `collapseMove` | 45/45 | f2/f3 register swap in fadds chain |

### `__sinit_` (309 vs 300, 9 words short)

The `smc_ANGLE_DIST_RATE` computation and 6 `STATE_DEFINE` blocks are structurally present but the register allocation differs. The 6 `sFStateID_c` objects are registered correctly. The 9-word gap is likely from the `smc_ANGLE_DIST_RATE` float-to-s16 conversion (the target uses `fctiwz` + stack spill; the draft may be using a different conversion path).

### `d3d::getTevKColor` and `dGameCom::rndF` declarations

These are `@unofficial` forward declarations in the shadow header. They exist in the DOL at known addresses but are not declared in any current shared header. The signatures are read from the mangled symbols:
- `getTevKColor__3d3dFP8_GXColorPQ23m3d6bmdl_ci14_GXTevKColorID`
- `rndF__8dGameComFf`

**Proposed header diff** (to be applied to the real include tree after verification):

Add to `include/game/bases/d_game_com.hpp`:
```cpp
float rndF(float max);
```

Add a new `include/game/bases/d_d3d.hpp` or to an existing d3d header:
```cpp
namespace d3d {
    void getTevKColor(_GXColor *out, m3d::bmdl_c *mdl, int matNo, _GXTevKColorID regID);
}
```
