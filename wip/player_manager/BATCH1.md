# Batch B1 — `0x8005E9A0`–`0x8005EEDF` (8 functions)

Verified with the real compiler (`compilers/Wii/1.1/mwcceppc.exe`) and
`tools/auto_decomp/harness.py`'s `compile_draft`/`disasm`/`diff_fn`, not by
hand-reasoning alone. The scratch draft used to produce these results lived
in this session's own scratchpad (outside the repo, not committed anywhere).
The source in my reply is what should be folded into the final
`d_a_player_manager.cpp`.

## Per-function status

| # | Function | Address | Size | Status |
|---|---|---|---|---|
| 1 | `createYoshi(mVec3_c&, int, dAcPy_c*)` | `0x8005E9A0` | `0xB8` | **MATCHING** — diff printed nothing |
| 2 | `initGame()` | `0x8005EA60` | `0xA4` | NEAR MISS — see below |
| 3 | `initStage()` | `0x8005EB10` | `0x180` | NEAR MISS (isolation artifact only) — see below |
| 4 | `exitStage()` | `0x8005ECA0` | `0x4` | **MATCHING** — diff printed nothing |
| 5 | `courseIn()` | `0x8005ECB0` | `0x30` | **MATCHING** — diff printed nothing |
| 6 | `setDefaultParam()` | `0x8005ECE0` | `0xA4` | NEAR MISS — see below |
| 7 | `getPlayerSetPos(u8, u8)` | `0x8005ED90` | `0xDC` | NEAR MISS — see below |
| 8 | `getPlayerCreateAction()` | `0x8005EE90` | `0x50` | **MATCHING** — diff printed nothing |

4 of 8 are byte-exact MATCHING per the harness's `diff_fn` (canonicalised
token-stream equality). None of the 8 are claimed MATCHING without the diff
actually printing nothing.

Both foreign-inline requirements were checked directly in the disassembly of
my own compiled draft, not assumed:
- `dScStage_c::getCourseIn()` is called with `bl` from `initStage` (the call
  site that makes the weak `0x8005EC90` body reappear) — confirmed present.
- `dCd_c::getFileP(int)` is **inlined** (no `bl`) in both `getPlayerSetPos`
  and `getPlayerCreateAction` — confirmed by grepping my compiled disassembly
  for `bl getFileP` in each function's range: **zero hits in both**, i.e. no
  `bl` at all, matching the brief's requirement exactly.

---

## 1. `createYoshi` — MATCHING, with a real finding

The finished body:

```cpp
daYoshi_c *daPyMng_c::createYoshi(mVec3_c &pos, int type, dAcPy_c *rider) {
    if (rider == nullptr) {
        u32 param = ACTOR_PARAM_GEN(dAcPy_c, PlayerNo, type) | ACTOR_PARAM_GEN(dAcPy_c, CreateAction, 1);
        return (daYoshi_c *) dActor_c::construct(fProfile::YOSHI, param, &pos, nullptr, 0);
    }
    u32 param = (rider->mParam & BIT_FLAG(dAcPy_c::PARAM_Direction >> 8)) | ACTOR_PARAM_GEN(dAcPy_c, PlayerNo, type);
    daYoshi_c *yoshi = (daYoshi_c *) dActor_c::construct(fProfile::YOSHI, param, &pos, nullptr, 0);
    if (yoshi != nullptr && yoshi->fn_8014eb70(rider, 1)) {
        yoshi->setCreateAction((rider->mParam >> (dAcPy_c::PARAM_CreateAction >> 8)) &
                               ((1 << (dAcPy_c::PARAM_CreateAction & 0xff)) - 1));
    }
    return yoshi;
}
```

**Finding: the vtable dispatch at `actor+0x270` resolved to `setCreateAction(int)`.**
The target's tail sequence (`lwz r12,0x60(r31)`; `lwz r12,0x270(r12)`;
`mtctr`; `bctrl`) is an ordinary virtual call, not a secondary/multiple-
inheritance vtable — confirmed empirically: I compiled a throwaway
`yoshi->block_hit_init()` (a known `dActor_c` virtual) in isolation and it
produced `lwz r12, 0x60(r3)` for the vtable-pointer fetch. **The vtable
pointer for this whole hierarchy lives at object offset `0x60`, not offset
`0`** — worth recording project-wide, since it means any `vfXX`-style name in
this codebase's headers denotes a *vtable-table* byte offset, not an
*object* byte offset; the two numbers live in unrelated spaces and this
batch initially conflated them.

I then brute-force-compiled calls to every `daPlBase_c`/`dAcPy_c` virtual
taking an `int`-like single parameter against a `daYoshi_c*` and read off
each one's vtable-slot offset from the disassembly. `setCreateAction(int)`
— declared virtual in `daPlBase_c` (`d_a_player_base.hpp:635`) and overridden
in `dAcPy_c` (`d_a_player.hpp:216`) — is the **only** one that lands at
`0x270`. The call's argument, `extrwi r4,r0,8,8` on `rider->mParam`, is
exactly `ACTOR_PARAM_LOCAL(rider->mParam, CreateAction)` (offset 16, size 8
per `dAcPy_c`'s own `ACTOR_PARAM_CONFIG(CreateAction, 16, 8)`), i.e. the
newly-created Yoshi inherits the **riding player's own create-action state**.
This is semantically coherent with `dAcPy_c::setCreateAction` itself calling
`daPyMng_c::createYoshi` (`d_a_player.cpp:9112-9125`) — the two functions are
a matched pair.

No header edit was needed or made; `setCreateAction` was already declared at
the right access level in both classes.

---

## 2. `initGame` — NEAR MISS (register-pair swap + scheduling)

Body (delivered, in my reply):
```cpp
void daPyMng_c::initGame() {
    mPlayerMode[0] = (PLAYER_POWERUP_e) 0;
    mCreateItem[0] = 0;
    mPlayerMode[1] = (PLAYER_POWERUP_e) 0;
    mCreateItem[1] = 0;
    mPlayerMode[3] = (PLAYER_POWERUP_e) 0;
    mCreateItem[3] = 0;
    mPlayerEntry[0] = 0;
    mPlayerType[0] = (PLAYER_TYPE_e) 0;
    mPlayerEntry[1] = 0;
    mPlayerType[1] = (PLAYER_TYPE_e) 1;
    mPlayerEntry[2] = 0;
    mPlayerType[2] = (PLAYER_TYPE_e) 3;
    mPlayerEntry[3] = 0;
    mPlayerType[3] = (PLAYER_TYPE_e) 2;
    mPlayerMode[2] = (PLAYER_POWERUP_e) 0;
    mCreateItem[2] = 0;
    mActPlayerInfo |= 1;
    setDefaultParam();
    mBonusNoCap = 0;
    mKinopioCarryCount = 0;
}
```

The *scrambled* store order above (mode/create for 0,1,3, then all of
entry/type, then mode/create for 2 last) is not a guess — it is transcribed
directly from the target's store-address order and reproduces every store's
**address and value** exactly, including the odd default
`mPlayerType[] = {MARIO, LUIGI, BLUE_TOAD, YELLOW_TOAD}` assignment (indices
0,1,2,3 → values 0,1,3,2). Same instruction count (41 target, 41 draft).

Two residual differences, both cosmetic/register-allocation, not structural:
- `lis r10, m_playerID__9daPyMng_c@ha` vs `lis r10, SYM0@ha` (and the
  matching `addi`) — see the isolation-artifact note under `initStage`
  below; same root cause here.
- One register-pair is consistently swapped: target uses `r5` for the
  literal `1` and `r6` for `&mPlayerType`; my draft allocates the same two
  values to the same two physical registers but with the roles reversed
  (`r6`=1, `r5`=base), so `stw r5,0x4(r6)` / `stw r4,0x8(r6)` /
  `stw r3,0xc(r6)` land as `stw r6,0x4(r5)` / `stw r4,0x8(r5)` /
  `stw r3,0xc(r5)`. Tried four source permutations (hoisting a named local
  for the literal `1`, reordering the index-2/3 assignments) — none changed
  the allocation. This reads as a genuine MWCC scheduler-heuristic
  difference, not a wrong-value or wrong-shape bug.

## 3. `initStage` — NEAR MISS, isolation artifact only

Body (delivered, in my reply) — 96 target instructions, 96 draft
instructions, and only **two** lines differ, both the identical
`lis r30, m_playerID__9daPyMng_c@ha` / `addi r30, r30, ...@l` pair.

**This is a proven artifact of testing 8 functions in isolation, not a code
bug.** In a from-scratch single-file compile, nothing outside this file
references `daPyMng_c::m_playerID` externally, and dtk's disassembler
displays it as an anonymous `...bss.0` / pool-numbered reference instead of
its name (verified: it happens identically whether `m_playerID` is used by
one function or three, so it is not a "used-once" heuristic; it is that
`m_playerID` sits at byte 0 of my scratch file's own `.bss`, with no
preceding sibling to disambiguate against). In the real, fully assembled
`d_a_player_manager.cpp` — linked against every other TU that reads
`daPyMng_c::m_playerID`/`mCreateItem`/etc. by name — the named relocation is
what target already shows, and every other line of this function (all 4
loops, both the `dScStage_c::getCourseIn()` gate, the `dInfo_c` field
reads, and the `checkBonusNoCap`/`daPyDemoMng_c`/`dMultiMng_c` tail calls)
matches address-for-address and register-for-register.

**Correction to `MAP.md`'s row #3, worth flagging directly:** MAP.md
describes the `addNum` loop as iterating "over `m_playerID`". The actual
target reads **`mPlayerEntry`** (`m_playerID_base + 0x40`), not
`m_playerID` itself — confirmed by both the raw disassembly (`addi r31,
r30, 0x40` before the loop) and by compiling the `mPlayerEntry` version,
which is what produced the 2-line-only diff above (the `m_playerID` version
compiled but the register-base offset came out `0x0`, i.e. visibly wrong).
The `checkCorrectCreateInfo`... `getCourseIn` conditional's inner loop
(calling `fn_8005f570`) also walks `mPlayerEntry`, not `mCourseInList` as I
first assumed from the MAP summary — the real condition, read from the raw
target bytes, is:

```cpp
if (dScStage_c::getCourseIn() &&
    /* dInfo_c field @0xaf4, s32, signed >= 0 */ &&
    isEntryNum1() &&
    /* dInfo_c field @0x24, u8, != 0 */) {
    for (int i = 0; i < 4; i++) {
        if (mPlayerEntry[i] == 0) {
            fn_8005f570((PLAYER_POWERUP_e) 0, i);
            break;
        }
    }
}
```

`isEntryNum1()` (the header's own inline `getEntryNum() == 1`) reproduces
the target's `subi/cntlzw/srwi./beq` idiom exactly — worth noting since it
means that accessor was already correctly shaped in the header before this
batch touched it.

**Two `dInfo_c` fields are read that are not in the frozen `d_info.hpp`:**
`@0xaf4` (word, read as signed, compared `>= 0`) and `@0x24` (byte, compared
`!= 0`). Accessed via file-scope raw-offset helpers in my draft
(`info_field_af4`/`info_field_24`), not added to the header.

## 4. `exitStage` — MATCHING

Single `blr`, defined out of line as instructed (not inline in the class
body, since it is genuinely called from `courseIn`... actually from nothing
in this batch's scope directly, but the brief is explicit that it must stay
out-of-line because it *is* called elsewhere in the TU).

## 5. `courseIn` — MATCHING

```cpp
void daPyMng_c::courseIn() {
    createCourseInit();
    mPauseDisable = 0;
    daPyDemoMng_c::mspInstance->initCourseIn();
}
```

## 6. `setDefaultParam` — NEAR MISS, and a correction to my own first read

**This function's actual behaviour is not what `MAP.md` row #7 describes**
("Initialises default per-player type/mode arrays and `mScore=0`"). Reading
the raw target bytes directly (not the summary) shows it touches
`mRest`/`mCoin`/`m_playerID`/`m_yoshiID`/`mScore` — **not**
`mPlayerType`/`mPlayerMode`/`mCreateItem` at all:

```cpp
void daPyMng_c::setDefaultParam() {
    for (int i = 0; i < 4; i++) {
        mRest[mPlayerType[i]] = 5;
        mCoin[mPlayerType[i]] = 0;
    }
    for (int i = 0; i < 4; i++) {
        m_playerID[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
        m_yoshiID[i] = 0;
    }
    mScore = 0;
}
```

`mRest`/`mCoin` are indexed **by `mPlayerType[i]`**, not by `i` directly —
same indirect-indexing idiom the header already documents for `mCoin` via
`getCoinAll()`. Every store's address and the literal `5` match target
exactly (verified against `target_sdata.txt`'s already-confirmed
`scRestMax`-adjacent constant `5` = default life count). Instruction *count*
differs (41 target vs 35 draft) because target keeps four `mPlayerType[i]`
reads live simultaneously before any store (needing the callee-saved `r30`/
`r31`, hence a stack frame: `stwu`/`stw r31`/`stw r30` in, matching pair
out), while every source shape I tried (for-loop, fully manually unrolled,
coin-before-rest) let MWCC schedule it using only volatile registers,
skipping the frame entirely. Same values, same target addresses, fewer
instructions — a scheduling-driven near miss, not a semantic one.

## 7. `getPlayerSetPos` — NEAR MISS

```cpp
nw4r::math::VEC3 daPyMng_c::getPlayerSetPos(u8 file, u8 gotoNo) {
    nw4r::math::VEC3 result;
    dCdFile_c *cdFile = dCd_c::getFileP(file);
    sNextGotoData *nextGoto = cdFile->getNextGotoP(gotoNo);
    float y = -(float) nextGoto->mY;
    float x = (float) nextGoto->mX;
    result.z = 0.0f;
    result.y = y;
    result.x = x;
    int idx = nextGoto->mType;
    result.x = x + l_start_pos_ofs[idx].x;
    result.y = y + l_start_pos_ofs[idx].y;
    if (!(nextGoto->mFlags & 0x40)) {
        result.x = result.x + l_start_pos_ofs[idx].z;
    }
    return result;
}
```

`dCd_c::getFileP` inlines here as required (verified, no `bl`). 54 of 55
target instructions land in the right place with the right opcode. The one
structural gap: target negates `mY` via `fsubs` (already-single) →
`fneg` → an explicit `frsp` before the first `stfs`, while every phrasing I
tried (`-(float)x`, two-statement negate, `0.0f - (float)x`, declaration
reordering) produces `fsubs` → `fneg` straight into `stfs`, with no `frsp`.
Semantically identical (negation is exact; the extra rounding is a no-op on
an already-single value), but a literal byte mismatch. Reporting as
unresolved rather than guessing further at the source shape that produces
it.

**Contradiction with the brief, worth flagging:** the `nextGoto->mFlags`
test (`rlwinm. r0,r3,0,25,25`, i.e. bit `0x40`, `BIT_FLAG(6)`) has no named
entry in `d_cd_data.hpp`'s `NextGotoFlags_e` (which only documents bits
0, 2, 3, 7). Used as a raw hex mask (`nextGoto->mFlags & 0x40`), not a
named enumerator — reporting, not adding to the shared header.

## 8. `getPlayerCreateAction` — MATCHING

```cpp
int daPyMng_c::getPlayerCreateAction() {
    dScStage_c *stage = dScStage_c::getInstance();
    dCdFile_c *cdFile = dCd_c::getFileP(stage_field_120e(stage));
    sNextGotoData *nextGoto = cdFile->getNextGotoP(stage_field_1211(stage));
    return nextGoto->mType;
}
```

`dCd_c::getFileP` inlines here too (verified, no `bl`), matching the "second
inline site" the brief predicted. Reads two `dScStage_c` fields the frozen
header does not declare:

- **`0x120e`, width 1 byte (`lbz`)** — used as the `file` index into
  `dCd_c::getFileP`.
- **`0x1211`, width 1 byte (`lbz`)** — used as the `gotoNo` index into
  `getNextGotoP`.

Both accessed via file-scope raw-offset helpers (`stage_field_120e`,
`stage_field_1211`) in my draft, per the "report the offsets, don't touch
the header" instruction. Return type kept as the header's `int` — the body
loads a single byte into `r3` (`lbz r3,0xb(r3)` on `nextGoto->mType`) which
is consistent with `int` (no contradiction either way; `u8`/`int` both
produce the same code here).

---

## Data objects emitted, by section

None of this batch's functions define any named static data object of
their own — everything they touch is either an already-declared
`daPyMng_c` static (owned by the class, already in the header) or a pool
literal the compiler emits automatically for a constant I wrote in source.
Enumerating the latter for the integrator's benefit:

| Value | Section | Emitted by | Note |
|---|---|---|---|
| `0.0f` | `.sdata2` (pool) | `getPlayerSetPos` (`result.z = 0.0f`) | Matches `target_sdata2.txt`'s `@80186_8042BD48 = 0x00000000`. Not hand-placed; the compiler pools it. |
| bias double `4503599627370496.0` (`0x4330000000000000`) | `.sdata2` (pool) | `getPlayerSetPos`, from the `(float)u16` casts on `nextGoto->mX`/`mY` | The standard MWCC unsigned-to-float codegen trick; not something I wrote directly. Matches `target_sdata2.txt`'s `@80189_8042BD50`. |
| `0x43300000`/`0x1` constant used by `oris` | n/a (immediate) | `createYoshi`'s `ACTOR_PARAM_GEN(..., CreateAction, 1)` | Not pooled; it's an immediate in the instruction stream. |

**`l_start_pos_ofs` (`.rodata:0x802EF488`, size `0x150`) is explicitly NOT
ours** — confirmed by `STATICS.md`, outside our claimed `.rodata` bound
(`0x802EF608-0x802EF618`). `getPlayerSetPos` references it as
`extern const mVec3_c l_start_pos_ofs[];` in my draft purely so the file
compiles standalone; whoever owns `daPyDemoMng_c`/`dAcPy_HIO_Speed_c`'s
`.rodata` should provide the real declaration (likely in that class's own
header, as a `static const` table or a free array — not determined here,
out of scope for this batch).

## Contradictions / corrections filed against MAP.md (not reconciled, per the brief)

1. **Row #3 (`initStage`)**: the `addNum` loop and the course-in-slot search
   loop both walk **`mPlayerEntry`** (`m_playerID_base+0x40`), not
   `m_playerID` as the row's prose says. Confirmed by compiling both
   versions; only the `mPlayerEntry` version reproduces the target's
   `addi r31, r30, 0x40`.
2. **Row #3's course-in conditional** does not touch `mCourseInList` at all
   (I initially assumed it did, before reading the raw bytes) — it walks
   `mPlayerEntry` for the first zero slot, calls `fn_8005f570(0, i)`, and
   breaks.
3. **Row #7 (`setDefaultParam`)**: the summary ("default per-player
   type/mode arrays") does not match the target bytes. The real function
   sets `mRest[mPlayerType[i]]=5`, `mCoin[mPlayerType[i]]=0` for all four
   slots, then zeroes `m_playerID`/`m_yoshiID`, then `mScore=0`. It never
   writes `mPlayerType` or `mPlayerMode` or `mCreateItem`.

## Rules compliance

- Did not run `ninja`/`configure.py`/`progress.py`/`land.py`.
- Did not edit `include/game/bases/d_a_player_manager.hpp`, `d_s_stage.hpp`,
  `d_cd.hpp`, `d_info.hpp`, `slices/wiimj2d.json`, or `syms.txt`.
- Did not author `getCourseIn__10dScStage_cFv` or `getFileP__5dCd_cFi`;
  verified both appear as the expected inline/call outcomes in my own
  compiled output instead.
- All compile/diff numbers above came from the real `mwcceppc.exe` +
  `dtk-windows-x86_64.exe` + `harness.py` toolchain, not hand-simulation.
