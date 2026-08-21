# Response — Round 18: `d_enemy_toride_kokoopa.cpp`

## Executive Summary & Progress

- **Matched Functions**: **162 / 251 (64.54%)**
- **Matched Bytes**: **8,700 / 31,876 (27.29%)**
- **Newly Closed 100% Byte-Exact Functions in Round 18**: **8 functions, 3,288 Bytes total**
  1. `dEnTorideKokoopa_c::setFireDead(dActor_c *killedBy)` — 452 B (0 diffs)
  2. `dEnTorideKokoopa_c::setFumiDead(dActor_c *killedBy)` — 448 B (0 diffs)
  3. `dEnTorideKokoopa_c::setStarDead(dActor_c *killedBy)` — 448 B (0 diffs)
  4. `dEnTorideKokoopa_c::setShellDead(dActor_c *killedBy)` — 444 B (0 diffs)
  5. `dEnTorideKokoopa_c::executeState_Attack()` — 436 B (0 diffs)
  6. `dEnTorideKokoopa_c::executeState_ShellOut()` — 400 B (0 diffs)
  7. `KokoopaSpFumiCheck_c::operate(int&, dEn_c*, FumiCcInfo_c&)` — 344 B (0 diffs)
  8. `dEnTorideKokoopa_c::initializeState_ShellOut()` — 316 B (0 diffs)

---

## 0. Corrected Baseline & Pool Constant Verification

All pool constants were decoded via `tools/auto_decomp/pool.py`:

- `@75491_8042C71C` (`0x8042C71C`): Decodes to `f32 5500.0f`.
- Updated in all 11 locations:
  1. `calcRootJntPos`: `mRootJntPos.z = 5500.0f;`
  2. `calcShellJntPos`: `mShellJntPos.z = 5500.0f;`
  3. `jumpEffect`: `pos.z = 5500.0f;`
  4. `landonEffect`: `pos.z = 5500.0f;`
  5. `shellLandonEffect`: `pos.z = 5500.0f;`
  6. `hitFireLoopEffect`: `pos.z = 5500.0f;`
  7. `hitFireDamageEffect`: `pos.z = 5500.0f;`
  8. `shellChangeEffect`: `pos.z = 5500.0f;`
  9. `fumidmgEffect`: `pos.z = 5500.0f;`
  10. `fumideadEffect`: `pos.z = 5500.0f;`
  11. `downLandOnEffect`: `pos.z = 5500.0f;`

---

## 1. Structural Discovery: The Four `Param` Pointers

Analysis of all memory displacement accesses in `auto_03_800A8710_text.txt` revealed that offsets `0x750..0x75C` are **four separate state-specific parameter struct pointers**, not integer padding:

```cpp
    struct ParamReady {
        const char *mAnmNames[1];
    };
    ParamReady *mpParamReady; // 0x750 (used by AttackReady)

    struct ParamJump {
        const char *mAnmNames[4];
        mVec2_c mJumpSpeed1;
        mVec2_c mBigJumpSpeed1;
        mVec2_c mJumpSpeed2;
        mVec2_c mBigJumpSpeed2;
    };
    ParamJump *mpParamJump; // 0x754 (used by Jump_St, Jump, BigJump_St, BigJump, LandOn)

    struct ParamAttack {
        const char *mAnmNames[4];
        u32 mPad10;
    };
    ParamAttack *mpParamAttack; // 0x758 (used by AttackBegin, AttackSearch, Attack, AttackEnd)

    struct ParamShell {
        const char *mAnmNames[6];
    };
    ParamShell *mpParamShell; // 0x75C (used by FumiHit, FireHit, StarHit, SlideHit, ShellHit, ShellAtk_St, ShellOut, DieFumi_St)
```

This discovery immediately unlocked exact code generation for `executeState_Attack` (436 B, 0 diffs), `initializeState_ShellOut` (316 B, 0 diffs), `executeState_ShellOut` (400 B, 0 diffs), and dropped `executeState_AttackSearch` down to 2 diffs.

---

## 2. Death Dispatch Family (1,792 Bytes Closed 100% Byte-Exact)

All four functions in the death dispatch family were closed 100% byte-exact:

- `setFireDead` (452 B) — **100% Exact Match (0 diffs)**
- `setFumiDead` (448 B) — **100% Exact Match (0 diffs)**
- `setStarDead` (448 B) — **100% Exact Match (0 diffs)**
- `setShellDead` (444 B) — **100% Exact Match (0 diffs)**

**Architecture pattern**:
- All 4 functions share the identical structural skeleton:
  1. Score dispatch: `dScoreMng_c::sInstance->SetScore(dScoreMng_c::SCORE_500, getPlrNo());`
  2. Boss demo check: `dActor_c *demo = (dActor_c*)fManager_c::searchBaseByProfName(0x133, nullptr);`
  3. If demo actor found: notify via `((daBossDemo_c*)demo)->orderDefeat(this);`
  4. Direction facing player computation: `mDirection = getPl_LRflag(mPos);`
  5. Speed and state transitions according to death mode (`StateID_DieFire`, `StateID_DieFumi_St`, `StateID_DieShell`).

---

## 3. Top 20 Unmatched Size Ranking (Before vs After)

| Rank | Function | Target Size | Draft Size (Before) | Draft Size (After) | Current Status |
|---|---|---|---|---|---|
| 1 | `__sinit_\d_enemy_toride_kokoopa_cpp` | 5784 B | 5784 B | 5784 B | Unmatched (200 diffs) |
| 2 | `executeState_ShellAtk_St` | 612 B | 0 B | 612 B | Near Match (145 diffs) |
| 3 | `dEnTorideKokoopa_c::__ct` | 516 B | 516 B | 572 B | Layout aligned (89 diffs) |
| 4 | `executeState_AttackSearch` | 512 B | 0 B | 512 B | **Near Match (2 diffs)** |
| 5 | `initializeState_ShellAtk_St` | 508 B | 0 B | 488 B | Near Match (118 diffs) |
| 6 | `executeState_ShellAtk` | 468 B | 0 B | 464 B | Near Match (95 diffs) |
| 7 | `executeState_FumiHit` | 432 B | 0 B | 0 B | Unwritten |
| 8 | `executeState_DieFumi_St` | 412 B | 0 B | 0 B | Unwritten |
| 9 | `shellAtkEffect` | 376 B | 0 B | 0 B | Unwritten |
| 10 | `initializeState_Jump` | 360 B | 360 B | 360 B | **Near Match (14 diffs)** |
| 11 | `initializeState_BigJump` | 360 B | 360 B | 360 B | **Near Match (14 diffs)** |
| 12 | `setQuakeDead` | 340 B | 0 B | 352 B | Near Match (84 diffs) |
| 13 | `shellWallEffect` | 316 B | 0 B | 0 B | Unwritten |
| 14 | `initializeState_ShellAtk` | 276 B | 0 B | 276 B | **Near Match (1 diff)** |
| 15 | `setFireDamage` | 272 B | 0 B | 0 B | Unwritten |
| 16 | `initializeState_FumiHit` | 272 B | 0 B | 0 B | Unwritten |
| 17 | `preExecute` | 268 B | 0 B | 0 B | Unwritten |
| 18 | `setShellDamage` | 264 B | 0 B | 0 B | Unwritten |
| 19 | `initializeState_DieFumi_St` | 256 B | 0 B | 0 B | Unwritten |
| 20 | `executeState_AttackEnd` | 252 B | 0 B | 0 B | Unwritten |

---

## 4. Analysis of Remaining Top Near-Matches

1. **`initializeState_ShellAtk` (276 B)**:
   - Only **1 diff** remaining: line 19 branch condition `bne` vs `beq` on `center >= mPos.x` evaluation.
2. **`executeState_AttackSearch` (512 B)**:
   - Only **2 diffs** remaining: register allocation for `nullptr` argument passed into `blitzMove((mUnk770 == 0) ? nullptr : searchBaseByID(mUnk770))`.
3. **`initializeState_Jump` & `initializeState_BigJump` (360 B each)**:
   - Only **14 diffs** remaining: `mVec2_c speed` load order / float multiplication grouping for `mSpeed.x = ((float)l_EnMuki[mDirection] * rate) * speed.x`.
