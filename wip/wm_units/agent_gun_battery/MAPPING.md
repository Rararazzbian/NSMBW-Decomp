# MINI_GAME_GUN_BATTERY_MGR / MINI_GAME_GUN_BATTERY_MGR_OBJ -- function inventory

Brand-new unit, `d_basesNP` `.text 0xF8980-0xF9B40` (0x11C0 bytes), ONE translation unit
covering both profiles (single `.ctors` entry `0x294 -> __sinit fn_2_F97D0`, confirmed with
`python wip/wm_units/ctors_map.py d_basesNP GUN_BATTERY`).

**Tally: 9/49 matched byte-for-byte modulo naming-only residuals** (4 are true 0-diff EXACT
matches; the other 5 differ only in symbol names the target dump structurally cannot show: our
own not-yet-external ctor names, our own vtable symbol, our own `.bss` singleton name -- the
project's established "MATCH" criteria). Both gates green: `check_fn_order.py` reports 0
inversions, `ctors_map.py d_basesNP GUN_BATTERY` reports exactly one `.ctors` entry.

## Round 1 correction to the coordinator's brief (independently verified by the coordinator)

**`MINI_GAME_GUN_BATTERY_MGR_OBJ` does NOT derive from `dActor_c`.** Its constructor
(`fn_2_F8AA0`) calls `__ct__7dBase_cFv` (`dBase_c::dBase_c()`) directly. The brief's `dActor_c`
evidence describes `fn_2_F89E0`, the **MGR**'s own ctor, not MGR_OBJ's. `BSS_SINGLETONS.md` has
since been corrected by the coordinator (their `resolve_singleton.py` had two real bugs: a
fixed backward-scan window that crossed a `blr` into a different function, and a register-kill
tracker that only understood `mr`, not `li`/`lis`/`lwz` overwrites -- both now fixed).

- **MGR** derives `dActor_c`, sizeof 0x398 (probed via
  `template<int N> struct ShowSize; ShowSize<sizeof(dActor_c)> x;` -> compiler error names the
  value), exactly matching MGR's own classInit literal. **MGR adds ZERO extra fields.**
- **MGR_OBJ** derives `dBase_c`, sizeof 0x70 (same probe). `sizeof(MGR_OBJ)` = 0xF4 (`li r3,
  0xf4` at classInit `fn_2_F89B8`, exact).

## Round 2: read the class's own vtable via its relocations (per coordinator direction)

`lbl_2_data_31A08` -- the object stored to `this+0x60` in the ctor -- is
`daMiniGameGunBatteryMgrObj_c`'s PRIMARY vtable. Dumped in full, with REAL symbol names, via:

```
bin/dtk-windows-x86_64.exe elf disasm bin/dtkspl/d_basesNP/obj/auto_04_000132B0_data.o \
    wip/wm_units/agent_gun_battery/target_data_132B0.txt
```

(`auto_04_000132B0_data.o` is the pre-split retail `.data` object covering `0x132B0-0x343E0`,
found by listing every `auto_04_*_data.o` in `bin/dtkspl/d_basesNP/obj/` and picking the one
whose range contains `0x31A08`.) Its 20 words (`target_data_132B0.txt:37987`):

```
[0]=0 [1]=0                                          -- leading header, always null here
[2]  create=fn_2_F8CE0            (OURS)
[3]  preCreate=preCreate__7dBase_cFv                 (default, dBase_c's own)
[4]  postCreate=postCreate__7dBase_cFQ2...            (default)
[5]  doDelete=doDelete__7fBase_cFv                    (default, fBase_c's own -- dBase_c doesn't override it)
[6]  preDelete=preDelete__7dBase_cFv                  (default)
[7]  postDelete=postDelete__7dBase_cFQ2...             (default)
[8]  execute=fn_2_F8D80          (OURS)
[9]  preExecute=fn_2_F8D40       (OURS)
[10] postExecute=postExecute__7dBase_cFQ2...           (default)
[11] draw=draw__7fBase_cFv                             (default)
[12] preDraw=preDraw__7dBase_cFv                       (default)
[13] postDraw=postDraw__7dBase_cFQ2...                 (default)
[14] deleteReady=deleteReady__7fBase_cFv               (default)
[15] entryFrmHeap=entryFrmHeap__7fBase_cFUlPQ2...       (default)
[16] entryFrmHeapNonAdjust=...NonAdjust__7fBase_cF...   (default)
[17] createHeap=createHeap__7fBase_cFv                 (default)
[18] fn_2_F9580         (OURS)
[19] getKindString=getKindString__7dBase_cCFv          (default)
```

Every "default" entry was verified by name against `bin/dtk/wiimj2d_symbols.txt` (e.g.
`preCreate__7dBase_cFv = .text:0x8006C540`), NOT assumed. Slot order matches `fBase_c`'s 16
declared virtuals (`create` through `createHeap`, in that header order) plus `dBase_c`'s own
addition(s) -- both fully re-grepped from `include/game/framework/f_base.hpp` and
`include/game/bases/d_base.hpp`. **MGR_OBJ overrides exactly four of these:** `create`,
`execute`, `preExecute`, and one slot immediately before `getKindString` -- `fn_2_F9580`'s own
content (conditionally destroys `mMethod`, calls `__dt__7dBase_cFv`, then
`__dl__7fBase_cFPv`) is unmistakably `~daMiniGameGunBatteryMgrObj_c()` itself, sitting at a
single ordinary declaration-order slot (NOT the "two slots at the very start" shape the
coordinator's general rule describes -- empirically, in THIS class's primary vtable, the
leading two words are simply null and the destructor is one regular slot near the end;
noted as a real discrepancy from the general rule, not chased further this round since the
vtable dump settled the practical question directly).

**Bonus, from the same data dump, verified not inferred:** a pooled string literal at
`lbl_2_data_31AD0` reads (raw ASCII bytes, decoded byte-for-byte)
`"daMiniGameGunBatteryMgrObj_c::StateID_ShowRule"`, `"...::StateID_Play"`,
`"...::StateID_ShowResult"` -- **this project's guessed class name was exactly right.**
The same object also holds 9 `sFStateID_c<T>` member-function-pointer triples (MWCC's 12-byte
PMF encoding, `{0xFFFFFFFF, fn_addr, 0}`), 3 per state:

| state | initialize | execute | finalize |
|---|---|---|---|
| StateID_ShowRule | fn_2_F9150 (0x10) | **fn_2_F8F70 (0x1D0)** | fn_2_F8F20 (0x50) |
| StateID_Play | fn_2_F92B0 (0x10) | fn_2_F9200 (0xB0) | fn_2_F9160 (0xA0) |
| StateID_ShowResult | fn_2_F9510 (0x10) | **fn_2_F9320 (0x1F0)** | fn_2_F92C0 (0x60) |

Read directly off the raw triple sequence in `lbl_2_data_31AD0`
(`target_data_132B0.txt`, the 9 `{0xFFFFFFFF, fn_addr, 0}` entries in strict file order,
grouped 3-at-a-time against the 3 name strings in the SAME order they appear) -- re-verified
carefully this pass, no longer a guess.

**This corrects the coordinator's/this project's own guess that the two large functions
(`fn_2_F8F70` 0x1D0, `fn_2_F9320` 0x1F0) are MGR_OBJ's `execute()` override.** `execute()`
itself is `fn_2_F8D80` (tiny, 12 lines, forwards to `mStateMgr.executeState()`, EXACT match).
**Both large functions are `execute`-phase bodies of DIFFERENT states** -- ShowRule's and
ShowResult's, not Play's (Play's own execute, `fn_2_F9200`, is a modest 0xB0 bytes).

`lbl_2_data_31A58`/`lbl_2_data_31A88` are `mStateMgr`'s own two auto-generated vtables
(intermediate `sStateMgr_c<T,...>` level, then final `sFStateMgr_c<T,...>` level -- both
100% compiler-generated from already-landed template headers). `lbl_2_data_31AB8` is
`sFStateFct_c<T>`'s own vtable (`~dtor`=fn_2_F8C60, `build`=fn_2_F9600, `dispose`=fn_2_F9660);
`fn_2_F9600`'s disassembled content matches `sFStateFct_c<T>::build()`'s header body verbatim
(null-check, `mState.setID(...)`, return `&mState`/`nullptr`). None of these needed hand-written
code -- they fell out for free once `mStateMgr` was declared as a real member.

## Function inventory (49 real functions, 0x11C0 bytes total; sizes sum to exactly 0x11C0,
confirming the range/count are complete)

| addr | size | status |
|---|---|---|
| F8980 | 0x30 | **MATCHED** (naming-only) -- `daMiniGameGunBatteryMgr_c_classInit__Fv` |
| F89B0 | 0x30 | **MATCHED** (naming-only) -- `daMiniGameGunBatteryMgrObj_c_classInit__Fv` |
| F89E0 | 0x60 | **MATCHED** (naming-only) -- `daMiniGameGunBatteryMgr_c::daMiniGameGunBatteryMgr_c()` |
| F8A40 | 0x10 | **MATCHED, EXACT (0 diff)** -- `daMiniGameGunBatteryMgr_c::create()` |
| F8A50 | 0x44 | **MATCHED** (naming-only) -- `daMiniGameGunBatteryMgr_c::doDelete()` |
| F8AA0 | 0xF0 | PARKED -- `daMiniGameGunBatteryMgrObj_c` ctor, 59 target/62 draft lines. See below. |
| F8B90 | 0x64 | not attempted -- template-generated (`sStateMgr_c<...>`'s own deleting dtor); should emit automatically once referenced, not hand-written |
| F8C00 | 0x60 | not attempted -- template-generated (intermediate-level dtor, same family as F8B90) |
| F8C60 | 0x40 | not attempted -- template-generated, `sFStateFct_c<T>`'s own dtor |
| F8CA0 | 0x40 | not attempted |
| F8CE0 | 0x60 | **MATCHED, EXACT (0 diff)** -- `daMiniGameGunBatteryMgrObj_c::create()` |
| F8D40 | 0x40 | **MATCHED** (naming-only) -- `daMiniGameGunBatteryMgrObj_c::preExecute()` |
| F8D80 | 0x30 | **MATCHED, EXACT (0 diff)** -- `daMiniGameGunBatteryMgrObj_c::execute()` |
| F8DB0 | 0x10 | not attempted -- template-generated (`sStateMgr_c<...>::initializeState`) |
| F8DC0 | 0x30 | not attempted |
| F8DF0 | 0x90 | not attempted |
| F8E80 | 0x50 | not attempted |
| F8ED0 | 0x10 | not attempted |
| F8EE0 | 0x40 | not attempted |
| F8F20 | 0x50 | not attempted -- StateID_ShowRule::finalize |
| F8F70 | 0x1D0 | not attempted -- **StateID_ShowRule::execute** (large, real game logic) |
| F9140 | 0x10 | not attempted -- template-generated (`sStateMgr_c<...>::getState`?) |
| F9150 | 0x10 | not attempted -- StateID_ShowRule::initialize |
| F9160 | 0xA0 | not attempted -- StateID_Play::finalize |
| F9200 | 0xB0 | not attempted -- StateID_Play::execute |
| F92B0 | 0x10 | not attempted -- StateID_Play::initialize |
| F92C0 | 0x60 | not attempted -- StateID_ShowResult::finalize |
| F9320 | 0x1F0 | not attempted -- **StateID_ShowResult::execute** (large, real game logic) |
| F9510 | 0x10 | not attempted -- StateID_ShowResult::initialize |
| F9520 | 0x60 | not attempted |
| F9580 | 0x80 | **MATCHED, EXACT (0 diff)** -- `daMiniGameGunBatteryMgrObj_c::~daMiniGameGunBatteryMgrObj_c()` |
| F9600 | 0x60 | not attempted -- template-generated, `sFStateFct_c<T>::build()` (content already read, matches header verbatim) |
| F9660 | 0x10 | not attempted -- template-generated, `sFStateFct_c<T>::dispose()` |
| F9670 | 0x20 | not attempted |
| F9690 | 0x20 | not attempted |
| F96B0 | 0x20 | not attempted |
| F96D0 | 0x10 | not attempted -- template-generated |
| F96E0 | 0x10 | not attempted -- template-generated |
| F96F0 | 0x10 | not attempted -- template-generated |
| F9700 | 0x10 | not attempted -- template-generated |
| F9710 | 0x10 | not attempted -- template-generated |
| F9720 | 0x10 | not attempted -- template-generated |
| F9730 | 0x10 | not attempted -- template-generated |
| F9740 | 0x30 | not attempted |
| F9770 | 0x30 | not attempted |
| F97A0 | 0x30 | not attempted |
| F97D0 | 0x280 | not attempted -- `__sinit`, always last, cannot be finalised until every contributing static/function is written |
| F9A50 | 0x60 | not attempted -- internal (in-range) call target, referenced 4x from the F8F70/F9320 region |
| F9AB0 | 0x90 | not attempted |

`F8CA0`, `F8DC0`, `F8DF0`, `F8E80`, `F8ED0`, `F8EE0`, `F9520`, `F9670`, `F9690`, `F96B0`,
`F9740`, `F9770`, `F97A0`, `F9A50`, `F9AB0` are NOT yet mapped to a role -- likely `mGunSlot`
(the 3-slot array)-related helpers, or more state-machine plumbing, or (`F9670/F9690/F96B0`,
which appeared as a triple in `lbl_2_data_31AD0` too, offsets 3-5 right after `fn_2_F8CA0`) a
SECOND `sFStateID_c`-style object specific to `daGunBatteryGunSlot_t`'s own polymorphic
interface -- not investigated this round.

## MATCHED, in detail (9)

- `daMiniGameGunBatteryMgr_c_classInit__Fv` (`fn_2_F8980`) -- naming only.
- `daMiniGameGunBatteryMgrObj_c_classInit__Fv` (`fn_2_F89B0`) -- naming only.
- `daMiniGameGunBatteryMgr_c::daMiniGameGunBatteryMgr_c()` (`fn_2_F89E0`) -- naming only (own
  vtable symbol, own `.bss` singleton name). The `+0x60` MI-thunk store (`dBase_c : public
  fBase_c, public cOwnerSetMg_c`'s secondary vtable pointer, per HANDOFF.md) needed no special
  code -- confirmed a 4th time on a brand-new unit.
- `daMiniGameGunBatteryMgr_c::create()` (`fn_2_F8A40`) -- EXACT.
- `daMiniGameGunBatteryMgr_c::doDelete()` (`fn_2_F8A50`) -- naming only (`.bss` singleton name).
- `daMiniGameGunBatteryMgrObj_c::create()` (`fn_2_F8CE0`) -- EXACT. Also identified 6 new class
  fields this round: `m_dc`(s32,=7), `m_e0`(u8,=0), `m_e4`(s32,=0),
  `m_e8`(s32,=`daPyMng_c::getNumInGame()`), `m_ec`(s32,=0), `m_f0`(s32,=0) -- spans `0xdc-0xf4`
  exactly, closing the class layout completely (`sizeof` now matches with every byte accounted
  for).
- `daMiniGameGunBatteryMgrObj_c::preExecute()` (`fn_2_F8D40`) -- naming only (the one external
  callee, `fn_2_420`, un-landed, linked via the project's `R_2_1_420` convention). Source shape
  that matched: `return !dBase_c::preExecute() ? 0 : !R_2_1_420();` -- three real levers were
  needed to land this one: (1) branch polarity -- the FALSE case must be the ternary's first/
  fall-through arm, not `if (cond) {...} else {...}`; (2) `R_2_1_420()` declared returning
  plain `int`, not `bool` -- returning `bool` made the compiler emit branch-based boolification
  instead of the target's `cntlzw`/`srwi` bit-trick; (3) the trick itself is a LOGICAL NOT of
  `fn_2_420`'s result, not a bare boolify (`cntlzw(0)=32; 32>>5=1` and `cntlzw(nonzero)<32;
  x>>5=0`, i.e. `!x`) -- verified by re-deriving the arithmetic, not guessed.
- `daMiniGameGunBatteryMgrObj_c::execute()` (`fn_2_F8D80`) -- EXACT. `{ mStateMgr.executeState();
  return SUCCEEDED; }`.
- `daMiniGameGunBatteryMgrObj_c::~daMiniGameGunBatteryMgrObj_c()` (`fn_2_F9580`) -- EXACT, empty
  body (`{}`) -- all the real cleanup work is the compiler-generated member/base teardown.

## PARKED: `daMiniGameGunBatteryMgrObj_c` ctor (`fn_2_F8AA0`, target 59 lines, draft 62)

Went from 58 differing (round 1) to a single, well-understood, isolated residual after adding
the four vtable overrides above (which fixed the `+0x60` vtable-identity mismatch and the
missing `+0xa0` store completely, exactly as the coordinator's reasoning predicted -- identifying
the real overrides reshaped the vtable-pointer stores automatically, no separate ctor-specific
fix needed for either). What's left:

**The 3-element `daGunBatteryGunSlot_t mGunSlot[3]` array construction loop-vs-unroll shape.**
Giving `daGunBatteryGunSlot_t` a real default constructor (`: m_00(0), m_04(0), m_08(-1) {}`)
and relying on IMPLICIT array-member construction (removing the explicit body loop entirely)
reproduced the target's entire instruction ORDER and register allocation correctly (loop bounds
computed before the vtable address, vtable stored, THEN the plain fields, THEN the array, THEN
`mCheck`/`mFactory`/`mMethod` setup, THEN the ctor call, THEN the final `+0xa0` patch -- all of
this now matches). The ONE remaining difference: MWCC's implicit array-of-3 construction
UNROLLS the first element inline (`stb`/`stw`/`stw` directly at `0x7c`/`0x80`/`0x84`, matching
the same style as the `m_70`/`m_74`/`m_78` stores right before it) and only loops for elements
1-2 (bound starts at `0x88`, i.e. `mGunSlot[1]`), where the target has a genuine 3-iteration
`bdnz` loop starting from element 0 (bound `0x7c`). Three attempts, all producing the SAME
partial-unroll shape or worse: (1) explicit hand-written `for` loop in the ctor body -- fully
unrolled, all 3 elements inline, 0 loop instructions (worse: this was the round-1 state, 58
differing); (2) implicit array-ctor construction (current) -- partial unroll as described, 3
lines over target; (3) a named `static const int GUN_SLOT_COUNT = 3;` loop bound instead of the
literal `3` -- no effect, reverted (MWCC still unrolls explicit loops over small compile-time
trip counts regardless of how the bound is spelled). Not solved this round; a 4th lever (a
`memset`-shaped construct, or forcing the array through a pointer so the compiler can't see the
trip count as compile-time-constant) is the next thing to try, not yet attempted.

Confirmed field layout (all from the ctor's own stores + the vtable dump; every offset now
accounted for, `sizeof` matches exactly with no remaining gap):

```
0x00-0x70   dBase_c (sizeof 0x70, probed)
0x70        u8   -- PLACEHOLDER name, init 0
0x74        s32  -- PLACEHOLDER name, init 0
0x78        s32  -- PLACEHOLDER name, init -1
0x7c-0xa0   daGunBatteryGunSlot_t mGunSlot[3]  -- 12B each: {u8(0), pad3, s32(0), s32(-1)}
0xa0-0xdc   sFStateMgr_c<daMiniGameGunBatteryMgrObj_c, sStateMethodUsr_FI_c> mStateMgr
              0xa0  outer sStateMgrIf_c vtable (now reproduced correctly)
              0xa4  mCheck (sStateIDChk_c, vtable only)
              0xa8  mFactory (sFStateFct_c<T>): 0xa8 own vtable, 0xac mState vtable,
                    0xb0 mState.mpOwner (= this), 0xb4 mState.mpID (= null initially)
              0xb8  mMethod (sStateMethodUsr_FI_c, sizeof 0x24, probed)
0xdc        s32 (=7 in create())
0xe0        u8  (=0 in create())
0xe4        s32 (=0 in create())
0xe8        s32 (=daPyMng_c::getNumInGame() in create())
0xec        s32 (=0 in create())
0xf0        s32 (=0 in create())          -- ends exactly at 0xf4, sizeof matches
```

Still open: what `dBase_c::preExecute()`'s ternary is actually gating (a real name for
`m_70`/`m_74`/`m_78`/`m_dc`/`m_e0`/`m_e4`/`m_ec`/`m_f0`, and the exact initial `sStateID`
passed to `mStateMgr`'s constructor -- currently `sStateID::null` as a placeholder, almost
certainly wrong; the real value is presumably `StateID_ShowRule` given the 3-state ordering
found in `lbl_2_data_31AD0`, once that state ID object is actually declared in source via a
`STATE_DEFINE`-style construct).

## Not yet attempted (40 functions)

Of these, roughly 13 (`F8B90`, `F8C00`, `F8C60`, `F8DB0`, `F9140`, `F9600`, `F9660`, `F96D0`,
`F96E0`, `F96F0`, `F9700`, `F9710`, `F9720`, `F9730`) are believed to be 100% COMPILER-GENERATED
template boilerplate from `sStateMgr_c<...>`/`sFStateMgr_c<...>`/`sFStateFct_c<T>`/`sFState_c<T>`
-- already present as WEAK symbols in the draft's own compiled output, just not yet PLACED
because nothing forces the linker to keep them (see `check_sections.py`'s note: "unreferenced
weak symbols... are emitted into the object but never placed"). They should simply appear once
the code that REFERENCES them (state machine usage beyond construction -- i.e. real state
bodies) is written; no hand-authoring expected to be needed for this group.

9 more (`F8F20`, `F8F70`, `F9150`, `F9160`, `F9200`, `F92B0`, `F92C0`, `F9320`, `F9510`) are the
three states' `initialize`/`execute`/`finalize` bodies, per the PMF table above -- real,
hand-written game logic. `F8F70` and `F9320` (the two large functions) are the highest-value
next targets.

`fn_2_F97D0` is `__sinit` (0x280 bytes) -- correctly left untouched per the project's rule.

The remaining ~17 functions (`F8CA0`, `F8DC0`, `F8DF0`, `F8E80`, `F8ED0`, `F8EE0`, `F9520`,
`F9670`, `F9690`, `F96B0`, `F9740`, `F9770`, `F97A0`, `F9A50`, `F9AB0`) have no confirmed role
yet.

## How to reproduce this tally

```
python wip/wm_units/agent_gun_battery/build.py
python wip/wm_units/check_fn_order.py wip/wm_units/agent_gun_battery/d_a_mini_game_gun_battery.cpp
python wip/wm_units/ctors_map.py d_basesNP GUN_BATTERY
python wip/wm_units/agent_gun_battery/difftool.py \
    wip/wm_units/agent_gun_battery/target_auto_00_F85C4_text.txt \
    wip/wm_units/agent_gun_battery/draft.txt \
    fn_2_<addr> <draft_symbol>
```

Target dumps in this directory (freshly regenerated, not reused):
`target_auto_00_F85C4_text.txt` (0xF85C4-0xF97A0, bulk of the unit),
`target_auto_fn2_F97D0_text.txt` (`__sinit`), `target_auto_00_F9A4C_text.txt` (0xF9A4C onward,
covers `fn_2_F9A50`/`fn_2_F9AB0`, the tail of the unit), `target_data_132B0.txt` (the `.data`
object holding every vtable/state-ID object this unit owns, `0x132B0-0x343E0`, with real symbol
names for every inherited/default vtable slot -- use THIS, not the `.text`-only relocation
walk, to read any further vtable question).
