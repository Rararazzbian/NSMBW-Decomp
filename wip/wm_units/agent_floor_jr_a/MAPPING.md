# FLOOR_JR_A -- function inventory (IN PROGRESS, 22/29)

`.text 0x834ac-0x8405c`, 0xBB0 bytes. Own `.ctors` (1 entry, matches target),
own `.bss`, 45-target `.rodata`. Real class name `daFloorJrA_c`, confirmed
from data (see `include/game/bases/d_a_floor_jr_a.hpp`, already landed as a
shadow-model header from FLOOR_JR_B's own authoring round; this unit's own
shadow copy in `shadow_include/` extends it with real fields/methods and
should replace the landed one once this unit lands).

Base of FLOOR_JR_B (`daFloorJrB_c : public daFloorJrA_c`), already landed.

## Tally: 22/29 byte-identical modulo symbol names

| target | size | status |
|---|---|---|
| 834B0 sFStateID_c\<daFloorJrA_c\>::~sFStateID_c | 0x58 | MATCH (auto, template) |
| 83510 ...::isSameName | 0x88 | MATCH (auto, template) |
| 835A0 ...::initializeState | 0x30 | MATCH (auto, template) |
| 835D0 ...::executeState | 0x30 | MATCH (auto, template) |
| 83600 ...::finalizeState | 0x30 | MATCH (auto, template) |
| 83630 classInit | 0x30 | MATCH |
| 83660 ctor | 0x7C | MATCH |
| 836E0 dtor | 0x98 | MATCH |
| 83780 setUnk_674_348 | 0xC | MATCH |
| 83790 create | 0x80 | MATCH |
| 83810 createMdl | 0xAC | MATCH |
| 838C0 resetToBasePos | 0x44 | NOT YET -- see below |
| 83910 execute | 0x5C | MATCH |
| 83970 setupBgCtr | 0xA0 | NOT YET, VERY CLOSE (5 diff lines) |
| 83A10 playCrumbleEffects | 0x80 | MATCH |
| 83A90 unk_83A90 | 0x68 | NOT YET -- stubbed empty, real body unread |
| 83B00 unk_83B00 (own new virtual, vtable tail slot 2) | 0x150 | NOT YET -- stubbed empty, real body unread (matrix/scale setup for draw) |
| 83C50 draw | 0x30 | MATCH |
| 83C80 doDelete | 0x28 | MATCH |
| 83CB0 finalizeState_DemoWait | 0x4 | MATCH |
| 83CC0 initializeState_DemoWait | 0x4 | MATCH |
| 83CD0 executeState_DemoWait | 0x4 | MATCH |
| 83CE0 finalizeState_Wait | 0x4 | MATCH |
| 83CF0 initializeState_Wait | 0x4 | MATCH |
| 83D00 executeState_Wait | 0x34 | NOT YET -- stubbed empty, real body unread |
| 83D40 initializeState_DieFall | 0x1C | MATCH |
| 83D60 finalizeState_DieFall | 0x4 | MATCH |
| 83D70 executeState_DieFall | 0x64 | NOT YET, CLOSE -- calcSpeedY/posMove/unk_83A90 confirmed, angle-increment tail (offsets 0x104/0x348, l_EnMuki table) not yet written |
| 83DE0 __sinit | 0x27C | NOT YET -- see below, real structural finding |

Reproduce:
```
python wip/wm_units/agent_floor_jr_a/build.py
```
`build.py` passes BOTH `auto_00_000834AC_text.o` AND the split
`auto_fn_2_83DE0_text.o` (confirmed necessary via the coordinator's own
`check_target_objs.py` warning -- `__sinit` is split into its own object here,
and passing only the `auto_00_*` object would have silently undercounted the
denominator at 28 instead of 29).

## Gates

- `.ctors`: GREEN. Draft emits exactly ONE `.ctors` entry
  (`__sinit_\d_a_floor_jr_a_cpp`), matching target's one entry
  (`0x144 -> __sinit at .text 0x83de0`).
- Function order: FLAGGED, root cause identified but NOT fixed. Every one
  of my own uniquely-bodied, byte-exact-matched functions (classInit through
  finalizeState_DieFall) is individually confirmed correct AND in the right
  relative order versus each OTHER (checked directly against
  `bin/dtk/d_basesNP_symbols.txt`, not the gate). The flag is a CASCADE from
  ONE thing: the 5 sFStateID_c\<daFloorJrA_c\> template instantiations
  (834B0-83600) land at the very END of my compiled object (after `__sinit`,
  even) instead of at the FRONT where target has them. All 5 are themselves
  BYTE-EXACT matches -- this is a pure POSITIONING issue, not a content one.
  Tried: reordering STATE_DEFINE/STATE_VIRTUAL_DEFINE (DieFall-first vs
  DemoWait-first) -- no effect on template positioning, and DieFall-first
  measurably WORSENED __sinit (74 -> 157 differing), so reverted. Genuinely
  parked, not guessed further per the coordinator's own "do not iterate
  against the gate" instruction -- reported here for their own judgement,
  same as the peach_castle_sequence precedent they cited (a LANDED unit that
  still flags this way).

## __sinit: a real structural finding, not yet resolved

Every symbol-normalized difference left in __sinit traces to ONE thing: my
three sFStateID_c\<daFloorJrA_c\>-based bss objects (StateID_DemoWait,
StateID_Wait, StateID_DieFall) patch their own vtable pointer to
__vt__27sFStateID_c\<12daFloorJrA_c\> (my own class's template instantiation,
exactly what STATE_DEFINE/STATE_VIRTUAL_DEFINE should produce) -- but the
TARGET patches its OWN three equivalent bss objects to lbl_2_data_1CBF8, an
EXTERNAL vtable belonging to a separate, small, not-yet-authored unit sitting
in the gap between FLOOR_JR_A's own end (0x8405c) and FLOOR_JR_B's start
(0x841e0) -- confirmed by address: that unit's own functions (fn_2_84060
etc, referenced from lbl_2_data_1CBF8's own vtable slots) sit at
0x84060-0x841b0, entirely outside both FLOOR_JR_A's and FLOOR_JR_B's own
claimed ranges.

In other words: Nintendo's own build appears to SHARE one sFStateID_c\<T\>
template instantiation across MULTIPLE classes (plausible if this specific
instantiation's machine code does not depend on which T is plugged in --
the state functions are called through function pointers, so the shared
vtable's OWN body genuinely does not care whose object it is patched onto).
I do not have a way to reproduce this SPECIFIC sharing from my own TU's
source -- it would require either that third small unit being authored first
(another landing-order dependency, this time in the OTHER direction) or some
other mechanism I have not identified. This also explains the constant
0x3b0-family offsets in the real __sinit running 0xBC bytes further into
g_profile_FLOOR_JR_A-relative space than mine: the correctly-sized version
of this pool depends on the shared external object's own layout, not mine.

Genuinely parked -- flagged for the coordinator's judgement rather than
guessed at further.

## Member layout (confirmed via ctor/dtor bytes)

```
class daFloorJrA_c : public dEn_c {
    dHeapAllocator_c mHeapAllocator;      // 0x524
    nw4r::g3d::ResFile mResFile;          // 0x540 (NOT a plain int -- confirmed by createMdl's own `stw r3,0x540` after a ResFile-returning call)
    m3d::mdl_c mModel;                    // 0x544
    dBg_ctr_c mBgCtr;                     // 0x584 (sizeof(dBg_ctr_c) == 0xe4, counted directly off its own landed header -- ends at 0x668, NOT 0x678)
    mVec3_c mBasePos;                     // 0x668 (confirmed: resetToBasePos copies mPos here)
    int m_674;                            // 0x674 (confirmed: set -1 in resetToBasePos, and by setUnk_674_348's own setter)
    mEf::effect_c mEffects[2];            // 0x678, confirmed already-landed class (include/game/mLib/m_effect.hpp), array ctor/dtor via __construct_array/__destroy_arr
    u8 mUnknown8A0[8];                    // 0x8a0, trailing, unaccounted
};
```
sizeof(daFloorJrA_c) == 0x8a8, confirmed TWICE independently: from THIS
unit's own classInit (fn_2_83630's `li r3,0x8a8`) and from FLOOR_JR_B's own
(already-landed) classInit allocating the identical constant.

## Proposed header change (shadow copy, not yet applied to the real one)

`shadow_include/game/bases/d_a_floor_jr_a.hpp` in this directory replaces the
landed `include/game/bases/d_a_floor_jr_a.hpp` (currently a padding-only
model from FLOOR_JR_B's own authoring round) with the real field layout
above, real STATE_FUNC_DECLARE(daFloorJrA_c, DemoWait) /
STATE_FUNC_DECLARE(daFloorJrA_c, Wait) / STATE_VIRTUAL_FUNC_DECLARE(daFloorJrA_c, DieFall)
in place of the old plain-virtual placeholders, and named methods
(createMdl, resetToBasePos, setupBgCtr, playCrumbleEffects) in place
of unk_83810/unk_838C0/one of the tail virtuals. unk_83B00 (the vtable
tail slot itself, called by execute()) is UNCHANGED -- its real content is
still unread.

Also proposed (in `shadow_include/game/bases/d_bg_ctr.hpp`): ONE new
overload of dBg_ctr_c::set(), `set(dActor_c*, const sBgSetInfo*, u8, u8,
mVec3_c*)`, plus the new sBgSetInfo struct itself -- confirmed necessary by
setupBgCtr's own call target mangled name
(set__9dBg_ctr_cFP8dActor_cPC10sBgSetInfoUcUcP7mVec3_c), which matches
NEITHER of the two overloads the real, landed header currently declares.

## What's confirmed VERIFIED vs INFERRED

VERIFIED (byte-exact, both function content and, where checked, .ctors
count): all 22 MATCH rows above.

INFERRED, not yet byte-verified: unk_83A90 and unk_83B00's bodies (read
enough to know unk_83B00 does per-frame matrix rotation via mAngle-style
short fields at 0x100/0x102/0x104 feeding PSMTXTrans/XrotM/YrotM/ZrotM
then mModel.setLocalMtx()/setScale() -- not yet translated to source);
resetToBasePos's own float-pool positioning (shape confirmed identical to
setupBgCtr's own fix, not yet applied -- likely the SAME per-function float
pool discrepancy, unattempted); executeState_Wait's body (0x34 bytes,
unread); executeState_DieFall's angle-increment tail (unread in detail,
uses l_EnMuki -- an ALREADY-VISIBLE symbol referenced by fn_2_83D70,
suggesting it's a real, findable constant table, not investigated further
this round).

## Status: NOT READY, genuine partial progress

22/29, .ctors count correct, order flag present with root cause identified
but not fixed (isolated to template-instantiation positioning, not content).
Seven functions remain unauthored or partially authored; __sinit blocked on
a cross-unit vtable-sharing question I could not resolve within this round.
Reporting plainly rather than forcing a false N/N.
