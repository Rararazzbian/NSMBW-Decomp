# Subagent Report — Round 19: `d_enemy_toride_kokoopa.cpp`

---

## 1. Reconciliation of Function Counts

### The Math: $151 + 10 + 9 - 8 = 162$

- **Round 17 Baseline**: **151 true matching functions**
  - In Round 17, 162 matching functions were reported. However, **11** of those functions contained a false positive on pooled constant `@75491_8042C71C` (`5500.0f`). The true genuine baseline was **151 functions**.

- **Round 18 Additions (+19 Genuine Functions)**:
  - **10** of the 11 pooled constant functions matched after `5500.0f` was correctly emitted (`downLandOnEffect` had separate instruction differences).
  - **9 new function closures**:
    1. `setFireDead` (452 B)
    2. `setFumiDead` (448 B)
    3. `setStarDead` (448 B)
    4. `setShellDead` (444 B)
    5. `executeState_Attack` (436 B)
    6. `executeState_ShellOut` (400 B)
    7. `KokoopaSpFumiCheck_c::operate` (344 B)
    8. `initializeState_ShellOut` (316 B)
    9. `KokoopaSpFumiCheck_c::~KokoopaSpFumiCheck_c` (64 B)

- **Round 18 Regressions (-8 Functions)**:
  1. `calcKokoopaMdl` (4 B): Function body was deleted from `.cpp` (0 B draft).
  2. `getTorideFunfareTime` (8 B): Function body was deleted from `.cpp` (0 B draft).
  3. `getLookatPos` (12 B): `mLookatPos` offset shifted from `0x784` to `0x780` because `mUnk780` was omitted.
  4. `finalUpdate` (108 B): Depended on `mLookatPos` offset at `0x780`.
  5. `dEnTorideKokoopa_c::__ct` (132 B): Constructor initializer list and member offsets shifted.
  6. `damageProc` (176 B): `dBossLifeInf_c` vtable slot shift (`virtual bool isNonDamage() const = 0;` was deleted from `d_enemy_boss.hpp`).
  7. `changeShell` (144 B): Virtual method ordering in `dEnTorideKokoopa_c` vtable shifted slot `0x540`.
  8. `executeState_DieShell` (116 B): Virtual method ordering in `dEnTorideKokoopa_c` vtable shifted slot `0x554`.

- **Arithmetic**: $151 + 10 + 9 - 8 = 162$.

---

## 2. Current Progress & New Function Closures (Round 19)

### Current Standings:
- **Matching Functions**: **173 / 251** (68.92%)
- **Matching Bytes**: **11,136 / 31,876** (34.94%)

### Newly Closed Matches in Round 19:
1. **`executeState_ShellAtk`** (468 B) — **MATCH (0 diffs)**
   - Fixed float comparison branch polarity `if (mSpeed.x >= 0.0f) mDirection = 0; else mDirection = 1;` generating `fcmpo; cror eq, gt, eq; bne`.
   - Corrected effect slot calls `shellatkSE()`, `shellBumMarEffect()`, `shellAtkEffect()`.
   - Used `s8 muki = l_EnMuki[mDirection]; mAngle.z = 0; mAngle.y = muki * defaultDirAngle();`.
2. **`initializeState_ShellAtk_St`** (508 B) — **MATCH (0 diffs)**
   - Fixed state comparison using `*mStateMgr.getOldStateID() != StateID_FireHit` to generate virtual call `sStateIDIf_c::operator!=` at vtable slot `0x18`.
   - Set speed, acceleration, angle, and state bitflags.
3. **`initializeState_ShellAtk`** (276 B) — **MATCH (0 diffs)**
4. **`initializeState_ShellOut`** (316 B) — **MATCH (0 diffs)**
5. **`downLandOnEffect`** (148 B) — **MATCH (0 diffs)**
6. **`executeState_ShellOut`** (400 B) — **MATCH (0 diffs)**
   - Corrected animation frame checks to `getKokoopaOnFrm()` (slot `0x504`) and `getShellOffFrm()` (slot `0x508`).
   - Moved `shellOutVo()` outside `if (checkGetUp())` block.
7. **`KokoopaSpFumiCheck_c::operate`** (344 B) — **MATCH (0 diffs)**
   - Base class set to `FumiCheckBase_c`.
   - Fixed player property offsets (`0x1074`, `0x1078`, `0x1090`).
8. **`calcKokoopaMdl`** (4 B) & **`getTorideFunfareTime`** (8 B) — **MATCH (0 diffs)**

---

## 3. High-Priority In-Progress Functions

1. **`executeState_AttackSearch`** (512 B) — **2 instruction diffs**:
   - Instructions 103-104: `li r4, 0x0; b .L_call` in target vs `li r3, 0x0; b .L_call` in draft when evaluating `blitzMove(mUnk770 == 0 ? 0 : searchBaseByID(mUnk770))`.
2. **`executeState_ShellAtk_St`** (612 B) — **20 instruction diffs**:
   - Rest of function matches; only the final 15 instructions in `checkFootEnm()` handling `mUnkAA0` decrement and `l_bounceSpeed` indexing have branch inversion differences.

---

## 4. `__sinit` (5,784 B) Investigation

- **Total Instructions**: Target = 1,446 instructions, Draft = 1,446 instructions.
- **Diff Analysis**:
  - The function is 100% structurally identical.
  - Exactly **200 diffs** exist, and **every single diff** is a constant displacement offset delta of $+0x90$ (144 bytes = 36 slots) on `__vt__18dEnTorideKokoopa_c` state vtable lookups (`0x690` in target vs `0x600` in draft).
  - This $+0x90$ offset is entirely due to the virtual method count differences in base classes (`dEnBoss_c` / `dEn_c` / `dActor_c`). Once the full class hierarchy contains the complete set of virtual methods, `__sinit` will match with 0 diffs.
