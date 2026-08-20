# FLOOR_JR_A -- function inventory (26/29 byte-identical modulo symbol names)

`.text 0x834ac-0x8405c`, 0xBB0 bytes, module `d_basesNP`. Own `.ctors` (1
entry, matches target), own `.bss`, 45-target `.rodata`. Real class name
`daFloorJrA_c`, confirmed from data (see
`include/game/bases/d_a_floor_jr_a.hpp`, already landed as a shadow-model
header from FLOOR_JR_B's own authoring round; this unit's own shadow copy in
`shadow_include/` extends it with real fields/methods and should replace the
landed one once this unit lands).

Base of FLOOR_JR_B (`daFloorJrB_c : public daFloorJrA_c`), already landed.

Started this round at a MEASURED 22/29 (rebuilt and confirmed before
touching anything). Closed 4 more functions to exact matches
(`resetToBasePos`, `unk_83A90`, `executeState_Wait`, `executeState_DieFall`)
and cut `__sinit`'s residual from 74 differing to 21 differing via a real
structural fix (see below). **NOT at 29/29** -- three items remain, all
detailed with exact evidence below, not guessed at further this round.

## Tally

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
| 838C0 resetToBasePos | 0x44 | **MATCH (closed this round)** |
| 83910 execute | 0x5C | MATCH |
| 83970 setupBgCtr | 0xA0 | NOT YET -- 5 differing, float-pool base offset only, see below |
| 83A10 playCrumbleEffects | 0x80 | MATCH |
| 83A90 unk_83A90 | 0x68 | **MATCH (closed this round)** |
| 83B00 unk_83B00 | 0x150 | NOT YET -- 55 differing, content fully correct, register-hoisting/pool-offset codegen quirk only, see below |
| 83C50 draw | 0x30 | MATCH |
| 83C80 doDelete | 0x28 | MATCH |
| 83CB0 finalizeState_DemoWait | 0x4 | MATCH |
| 83CC0 initializeState_DemoWait | 0x4 | MATCH |
| 83CD0 executeState_DemoWait | 0x4 | MATCH |
| 83CE0 finalizeState_Wait | 0x4 | MATCH |
| 83CF0 initializeState_Wait | 0x4 | MATCH |
| 83D00 executeState_Wait | 0x34 | **MATCH (closed this round)** |
| 83D40 initializeState_DieFall | 0x1C | MATCH |
| 83D60 finalizeState_DieFall | 0x4 | MATCH |
| 83D70 executeState_DieFall | 0x64 | **MATCH (closed this round)** |
| 83DE0 __sinit | 0x27C | NOT YET -- 21 differing, down from 74; real structural fix landed, residual is a `.data` layout gap, see below |

**26/29 byte-identical.** Reproduce:
```
python wip/wm_units/agent_floor_jr_a/build.py
```
`build.py` passes BOTH `auto_00_000834AC_text.o` AND the split
`auto_fn_2_83DE0_text.o` (confirmed necessary via
`check_target_objs.py` -- `__sinit` is split into its own object here).

## What was fixed this round, with evidence

**`resetToBasePos`** is `int`-returning, not `void`. Target's version builds
a full stack frame (`stwu`/`mflr`/`stw lr`) and ends with `bl setupBgCtr`
followed by `li r3,0x1` / restore-lr / `blr` -- NOT a tail branch. A
void-returning version tail-call-optimizes to a bare `b setupBgCtr` with no
frame, which is what the FIRST draft compiled to (14 differing, wrong
symbol). Changed signature to `int resetToBasePos()` returning `SUCCEEDED`
(mirroring `create()`'s own pattern) and it matched immediately. Shadow
header updated (`virtual int resetToBasePos();`, was `virtual void`).

**`unk_83A90`** is `mEffects[0].follow(&mPos, nullptr, nullptr);
mEffects[1].follow(&mPos, nullptr, nullptr);`. Read directly off the two
vtable calls: `lwz r12, 0x678(r3)` / `lwz r12, 0xb0(r12)` (vtable slot at
byte offset 0xb0 from the vtable pointer) dispatches on `mEffects[0]`
(member at 0x678), passing `(this+0xac, 0, 0)` -- `this+0xac` is `mPos`
(already established). Cross-checked slot 0xb0 against `mEf::effect_c`'s
OWN landed vtable dump (`lbl_2_data_11B40` in
`target_auto_04_00000000_data.txt`, line ~29217): counting from the vtable
pointer's own target (the dtor slot, offset 0), offset 0xb0 lands on
`follow__Q23mEf8effect_cFPC7mVec3_cPC7mAng3_cPC7mVec3_c` -- exactly matching
the already-landed `mEf::effect_c::follow(const mVec3_c*, const mAng3_c*,
const mVec3_c*)` signature.

**`unk_83B00`** (own new virtual, vtable tail slot 2, called from
`execute()`) is a per-frame matrix/scale rebuild:
```cpp
void daFloorJrA_c::unk_83B00() {
    mMatrix.trans(mPos);
    mMatrix.YrotM(mAngle.y);
    mMatrix.concat(mMtx_c::createTrans(l_EnMuki[mDirection] * 32.0f, 0.0f, 0.0f));
    mMatrix.XrotM(mAngle.x);
    mMatrix.concat(mMtx_c::createTrans(-l_EnMuki[mDirection] * 32.0f, 0.0f, 0.0f));
    mMatrix.ZrotM(mAngle.z);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
}
```
`mMatrix` (`mMtx_c`, from `dBaseActor_c`) sits at offset 0x7c -- confirmed
because `dBaseActor_c::mPos` (0xac, already established) is declared
immediately after `mMatrix` in `d_base_actor.hpp`, and `sizeof(mMtx_c) ==
0x30` closes the gap exactly (0x7c + 0x30 = 0xac). `mAngle` (2D, NOT
`mAngle3D`) sits at 0x100/0x102/0x104 for x/y/z -- confirmed by summing
`d_base_actor.hpp`'s own field list from `mSpeedMax` forward; `mAngle` (not
`mAngle3D`, which is declared right after it) lands exactly there.
`mDirection` (`u8`, at 0x348) is `dActor_c`'s own already-declared,
already-named field -- no raw offset needed. `l_EnMuki` (`extern const s8
l_EnMuki[]`, `{1, -1}`) is already declared in the landed `d_enemy.hpp` and
already used by many landed `dEn_c`-derived classes with this exact
`l_EnMuki[mDirection] * <float>` idiom. The negation on the SECOND
`createTrans` call is on the raw `s8` table value before the int->float
widen (matches the target doing `neg r0,r0` on the integer BEFORE the
magic-double float conversion, not an `fneg` on the final float -- confirmed
by reading the exact instruction sequence, not assumed).
`mMatrix.trans(mVec3_c&)`, `mMtx_c::createTrans(float,float,float)` and
`mMtx_c::concat(const mMtx_c&)` are all pre-existing INLINE helpers already
in the landed `m_mtx.hpp`; `mModel.setLocalMtx(&mMatrix); mModel.setScale
(mScale);` is a copy-paste-verified idiom used verbatim by a dozen already-
landed `d_a_en_*` files (e.g. `d_a_en_bros_base.cpp:187-188`).
**Content is 100% correct and size-exact (84/84 words)** -- see "Parked"
below for the remaining 55-line residual, which is a pure codegen/pool
quirk, not a logic error.

**`executeState_Wait`**:
```cpp
void daFloorJrA_c::executeState_Wait() {
    if (m_674 == 0)
        changeState(StateID_DemoWait);
    else if (m_674 > 0)
        m_674--;
}
```
Read directly off the `cmpwi`/`bne`/tail-`bctr`-into-`changeState` +
`blelr` + decrement shape -- a standard countdown-then-transition pattern
already used elsewhere in this same file (`m_674` is the SAME field
`resetToBasePos` initializes to -1 and `setUnk_674_348` sets directly).

**`executeState_DieFall`**'s previously-parked tail is
`mAngle.z += l_EnMuki[mDirection] * 0x60;` -- read directly off the
`lha`/`mulli r0,r0,0x60`/`extsh`/`add`/`sth` sequence at offset 0x104
(confirmed as `mAngle.z`, same field `unk_83B00` uses).

## `__sinit`: the real fix, and what's still unresolved

**The `STATE_VIRTUAL_DEFINE(daFloorJrA_c, DieFall)` in the previous round's
draft was the wrong macro, not a landing-order problem.** `dEn_c` (our
direct base) already declares `STATE_VIRTUAL_FUNC_DECLARE(dEn_c, DieFall)`
in `d_enemy.hpp:168`. Overriding `initializeState_DieFall` /
`executeState_DieFall` / `finalizeState_DieFall` as ordinary C++ virtuals
(matching a base class's existing virtual, no macro needed for the
override itself) is sufficient to land them in the right vtable slots --
`STATE_VIRTUAL_DEFINE`'s job is specifically for a DERIVED class that wants
to CHAIN to an ancestor's existing `StateID` object by name via the
`baseID_DieFall<T>()` lookup machinery, which is a DIFFERENT thing.

Proof this was wrong: **every one of target's three state-constructor
blocks in `__sinit` (DemoWait, Wait, DieFall) is byte-for-byte the SAME
shape** -- 9-word copy from `.data`, plain `bl __ct__10sStateID_cFPCc`,
vtable-pointer patch, `bl __register_global_object`. None of the three
calls a `baseID_DieFall<T>` lookup, which `STATE_VIRTUAL_DEFINE`'s expansion
would require if used. The OLD draft's `STATE_VIRTUAL_DEFINE` usage
compiled to a call to `baseID_DieFall<5dEn_c>` (real, verifiable symbol,
not a guess) that target simply does not have anywhere in `__sinit`.

Fix applied: replaced `STATE_VIRTUAL_FUNC_DECLARE(daFloorJrA_c, DieFall)` /
`STATE_VIRTUAL_DEFINE(daFloorJrA_c, DieFall)` with a manual declaration
(`virtual void initializeState_DieFall(); ... static sFStateID_c
<daFloorJrA_c> StateID_DieFall;` in the shadow header) plus a plain
`STATE_DEFINE(daFloorJrA_c, DieFall);` in the .cpp -- i.e. DieFall gets its
own ordinary, non-chained `sFStateID_c<daFloorJrA_c>`, exactly like
DemoWait and Wait, while its three methods stay virtual (inherited
virtual-ness from `dEn_c`, no macro required to keep that). **Confirmed
directly by `.data`: DieFall's three member-function-pointer slots in
`g_profile_FLOOR_JR_A`'s own trailing data ARE encoded as raw
`{vtable_byte_offset, 0x60, 0}` triples (0x180/0x17c/0x178 = vtable byte
offsets for slots 96/95/94), not `{0xFFFFFFFF, fn_addr, 0}` direct-address
triples like DemoWait/Wait's -- i.e. DieFall's pointer-to-member-function
IS the virtual encoding, proving the "virtual functions, no chained
StateID" read is right, not a guess.**

Result: `__sinit`'s residual dropped from **74 differing to 21 differing**,
and -- critically -- **instruction ORDER now matches exactly, all 159
words line up 1:1 with target's 159 words** (previously the draft was 166
words, wrong length, wrong shape). Every remaining differing line is either
a pure symbol-name difference (`SYM0` vs `g_profile_FLOOR_JR_A`,
`"__vt__27sFStateID_c<12daFloorJrA_c>"` vs `lbl_2_data_1CBF8`,
`"__dt__27sFStateID_c<12daFloorJrA_c>Fv"` vs `fn_2_84060` -- these are
almost certainly the SAME symbols, just stripped to raw addresses in
target's retail build, per the coordinator's own corrected finding that
`lbl_2_data_1CBF8` is this unit's own weak template vtable) or a
**consistent 0xBC-byte offset shift** on every `g_profile_FLOOR_JR_A`-
relative immediate (target reads from `+0x3b0`, draft from `+0x2f4`, etc.,
same relative spacing throughout).

**Root cause of the 0xBC (192-byte) shift, isolated but not fixed:**
target's own `g_profile_FLOOR_JR_A`-adjacent vtable-and-trailing-data blob
(`lbl_2_data_1C7E8`, `.data` size `0x410`) is laid out as: our own vtable
content (163 words -- confirmed EQUAL to FLOOR_JR_B's own already-landed
vtable, `lbl_2_data_1CC3C`, `0x28C`/163 words exactly, since FLOOR_JR_B
inherits ours unchanged bar its own dtor slot) **+ 48 words (192 bytes) of
solid zero** + the 27 words of DemoWait/Wait/DieFall's own
pointer-to-member-function triples + the 22 words of the three state-name
ASCII strings = 260 words = `0x410` exactly (checked arithmetically, not
approximated). Our own draft's compiled output has NO equivalent
zero-block: our vtable object ends and the very next word is already the
first pointer-to-member-function triple, with each triple compiled as its
OWN small separate anonymous local `.data` object (`"@15323"` etc.) rather
than one fused blob. **I could not identify what occupies those 192 zero
bytes** -- it is not the crumble-effect/model-resource strings (those are
a SEPARATE, earlier 4-string block at `0x1C78C-0x1C7E8`, already matched
and accounted for), and it is not extra vtable slots (`dEn_c` declares 12
OTHER `STATE_VIRTUAL_FUNC_DECLARE` states besides `DieFall` --
`DieFumi`/`DieBigFall`/`DieSmoke`/`DieYoshiFumi`/`DieIceVanish`/`DieGoal`/
`DieOther`/`EatIn`/`EatNow`/`EatOut`/`HitSpin`/`Ice` -- but those would sit
INSIDE the already-matched 163-word inherited vtable region, not appended
after it). Genuinely parked: I verified the block is solid zero, verified
its exact size and position, and could not attribute it to any known
declaration. This is the same *shape* of open question as the vtable
finding the coordinator corrected (anonymous/local `.data` objects that
might be merged differently at real link time than in an isolated-object
build), but unlike that one, I could not find a reference from anywhere in
FLOOR_JR_A's own `.text` that would let me apply the same "who references
it" test -- there is nothing in our own code that reads or writes this
span, so I cannot rule out OR confirm it belongs to us.

## `unk_83B00`'s own residual (55 differing, content already correct)

After the fix above, `unk_83B00`'s logic is complete and its SIZE is
already exact (84/84 words -- confirmed, not close). The 55 remaining
differing lines are ALL instruction-selection/scheduling, not logic:
target hoists the address of a repeated rodata float constant
(`lbl_2_rodata_31A8+0x14`, used twice within the function, once per
mirrored translate-and-concat block) into a saved register once via `addi`
and reuses it via plain-offset loads both times; our own compiler folds the
`@l` relocation directly into each of the two `lfs` instructions instead
(no address materialized/reused), a difference of exactly one `addi`
instruction plus the two different addressing-mode encodings that follow
from it, which cascades into every following instruction's file offset for
the rest of the function. I tried nothing further here since this is a
backend register-allocation heuristic (which of two equally-valid encodings
to pick for a twice-used address), not something I have evidence is
steerable from source structure -- the source already writes the value
identically both times (`l_EnMuki[mDirection] * 32.0f`, negated the second
time), matching target's own re-derivation of the value from scratch both
times (confirmed target does NOT cache the value in a float local -- it
redoes the full byte-lookup + sign-extend + int-to-double-magic-trick
conversion twice, which only makes sense if the source has two separate
expressions, not a shared local).

## `setupBgCtr`'s own residual (5 differing)

Content and instruction COUNT are already exact (40/40 words). The only
difference is the base offset into the shared rodata float pool
(`lbl_2_rodata_31A8`): target reads its five float constants (1.0, -32.0,
8.0, 32.0, -8.0) at offsets 0x1c-0x2c of a 0x30-byte pool object; our draft
reads the SAME five values, in the SAME order, at offsets 0x0-0x10 of its
own separate, smaller pool object. **Verified by content, not assumed**:
decoded all 12 words of target's `lbl_2_rodata_31A8` (offsets 0x00-0x2c);
the first 7 words (0.0, ~0.005, ~0.99, -5.0, a denormal, 32.0, 32.0) are
NOT referenced by ANY instruction anywhere in FLOOR_JR_A's own `.text`
(checked every `lbl_2_rodata_31A8`/`lbl_2_rodata_31E0`/`lbl_2_rodata_31D8`
cross-reference in the target disassembly -- only `setupBgCtr` at offsets
0x1c-0x2c and `unk_83B00` at offset 0x14 touch this object at all). Applying
the same "who references it" ownership test the coordinator used to correct
the `__sinit` vtable finding, but in the OPPOSITE direction this time: since
nothing in our own unit references those first 7 words, they are not ours
to reproduce, and this offset gap is a link-time artifact of OUR rodata
pool object landing immediately adjacent to an unauthored neighbor's own
float pool in the real binary, not a content bug. Not iterated on further.

## Gates

- `.ctors`: GREEN. Draft emits exactly ONE `.ctors` entry
  (`__sinit_\d_a_floor_jr_a_cpp`), matching target's one entry
  (`0x144 -> __sinit at .text 0x83de0`).
- Function order: FLAGGED, same root cause as last round, unaffected by
  this round's changes (re-checked after every edit). The 5
  `sFStateID_c<daFloorJrA_c>` template instantiations (834B0-83600) land at
  the very END of the compiled object instead of the FRONT where target has
  them. All 5 are individually byte-exact matches -- pure positioning, not
  content. Per the coordinator's own instruction not to iterate against
  this gate, and given the `d_a_peach_castle_sequence.cpp` precedent (a
  LANDED unit that still flags this way), this is reported for the
  coordinator's own judgement rather than guessed at further.

## Member layout (confirmed via ctor/dtor bytes, unchanged this round)

```
class daFloorJrA_c : public dEn_c {
    dHeapAllocator_c mHeapAllocator;      // 0x524
    nw4r::g3d::ResFile mResFile;          // 0x540
    m3d::mdl_c mModel;                    // 0x544
    dBg_ctr_c mBgCtr;                     // 0x584 (ends 0x668)
    mVec3_c mBasePos;                     // 0x668
    int m_674;                            // 0x674
    mEf::effect_c mEffects[2];            // 0x678 (ends 0x8a0)
    u8 mUnknown8A0[8];                    // 0x8a0, trailing, unaccounted
};
```
sizeof(daFloorJrA_c) == 0x8a8, confirmed via `classInit`'s `li r3,0x8a8` and
independently via FLOOR_JR_B's own identical constant. Unchanged this
round -- no member layout edits were needed for anything closed.

Base-class fields used this round, all already-declared/named in landed
headers (no raw-offset accesses needed for any of them):
`dBaseActor_c::mMatrix` (0x7c, `mMtx_c`), `dBaseActor_c::mPos` (0xac),
`dBaseActor_c::mScale` (0xdc), `dBaseActor_c::mAngle` (0x100, `mAng3_c`,
the 2D one, NOT `mAngle3D`), `dActor_c::mDirection` (0x348, `u8`).

## Proposed header change (shadow copy, still not applied to the real one)

`shadow_include/game/bases/d_a_floor_jr_a.hpp` in this directory replaces
the landed `include/game/bases/d_a_floor_jr_a.hpp` (currently a
padding-only model from FLOOR_JR_B's own authoring round) with:
- the real field layout above,
- `STATE_FUNC_DECLARE(daFloorJrA_c, DemoWait)` /
  `STATE_FUNC_DECLARE(daFloorJrA_c, Wait)` (unchanged from last round),
- **DieFall as a manual declaration, NOT `STATE_VIRTUAL_FUNC_DECLARE`** --
  `virtual void initializeState_DieFall(); virtual void
  executeState_DieFall(); virtual void finalizeState_DieFall(); static
  sFStateID_c<daFloorJrA_c> StateID_DieFall;` (see the `__sinit` section
  above for the full evidence trail on why),
- named methods (`createMdl`, `resetToBasePos` -- now `int`-returning,
  `setupBgCtr`, `playCrumbleEffects`, `unk_83A90`) in place of the old
  placeholders, and `unk_83B00` (the vtable tail slot itself) now with its
  real body authored, though the METHOD NAME itself is still a placeholder
  (no evidence yet for its real name).

Also still proposed (in `shadow_include/game/bases/d_bg_ctr.hpp`): the same
`dBg_ctr_c::set()` overload and `sBgSetInfo` struct as last round, unchanged
and unverified further this round (evidence stands from before: the real,
landed header declares neither overload matching `setupBgCtr`'s own call
target mangled name).

## What's confirmed VERIFIED vs INFERRED

VERIFIED (byte-exact both content and, where checked, `.ctors` count): all
26 MATCH rows above, including all 4 closed this round.

VERIFIED (content correct, size exact, only cosmetic codegen/pool-offset
residuals remain, evidence given in detail above): `setupBgCtr`,
`unk_83B00`.

INFERRED / genuinely open (evidence given, not resolved): the 192
zero-byte gap in `__sinit`'s underlying `.data` layout; whether
`lbl_2_data_1CBF8`/`fn_2_84060`'s TRUE identity is exactly
`__vt__27sFStateID_c<12daFloorJrA_c>` / its destructor stripped of debug
names (very likely, per the coordinator's own precedent, but not
independently reconfirmed this round beyond noting our own compiler
produces those exact real names at those exact call sites); `unk_83B00`'s
real (non-placeholder) method name.

## Status: 26/29, three items open, all evidenced and parked, not guessed further

- `setupBgCtr`: content/size exact, 5-line rodata-pool-offset residual,
  verified as a link-order artifact (nothing in our own unit references
  the extra bytes ahead of ours in the shared pool).
- `unk_83B00`: content/size exact, 55-line register-hoisting codegen
  residual, no further source-level lever identified.
- `__sinit`: real structural bug found and fixed (wrong STATE macro),
  residual cut from 74 to 21 differing, remaining 21 lines all trace to
  ONE 192-byte zero gap in `.data` whose owner could not be identified.
- Order gate: same pre-existing flag as last round, not touched, matches
  the `d_a_peach_castle_sequence.cpp` landed-but-flagged precedent.

Reporting plainly rather than forcing a false N/N.
