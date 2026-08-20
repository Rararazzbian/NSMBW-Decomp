# AC_FLAGON / AC_4SWICHAND / AC_4SWICHOR / AC_RANDSWICH / AC_CHNGESWICH / AC_IFSWICH / AC_RNSWICH

Coordinator-scoped as SIX profiles, `.text 0x7D400-0x7D5E0`. **Bounds correction: it is SEVEN,
`.text 0x7D400-0x7D630` (0x230 bytes).** See "Bounds" below -- flagged plainly, per the brief.

## The one question: ONE class, not six (or seven)

`daFlagObj_c`. All seven profiles construct the same class; there is exactly one vtable
(`lbl_2_data_1BC30`), not seven, and the classInit bodies carry no per-profile discriminator --
state selection happens inside `create()`/the state machine, entirely outside this unit.

Evidence, VERIFIED (not inferred):

1. **Relocation walk.** All 14 `.data` relocations from `.text 0x7D400-0x7D630` (2 per classInit:
   `lis`/`addi` `@ha`/`@l`) target the same address, `0x1BC30`. Checked directly against
   `profile_map.py`'s own relocation table, not eyeballed off a disassembly.
2. **The vtable's own trailing string-literal table**, decoded byte-for-byte from
   `target_auto_04_000132B0_data.txt:10403-10464` (raw `.4byte` words, big-endian, decoded with a
   small script, not read by eye): seven `ClassName::StateID_Name` strings, all under one class:
   `daFlagObj_c::StateID_NonMove`, `_Swich4andMove`, `_Swich4orMove`, `_RandSwichMove`,
   `_ChngeSwichMove`, `_IfSwichMove`, `_RenzokuOnMove` -- lining up 1:1 with the seven profiles
   (NonMove/FLAGON, Swich4and/4SWICHAND, Swich4or/4SWICHOR, RandSwich/RANDSWICH,
   ChngeSwich/CHNGESWICH, IfSwich/IFSWICH, RenzokuOn/RNSWICH).
3. **The seven `g_profile_AC_*` structs** immediately before the vtable (`.data 0x1BBD8-0x1BC30`,
   dumped fresh alongside it) each store a DIFFERENT classInit pointer (`fn_2_7D400` ..
   `fn_2_7D5E0`) but the SAME `properties` word (`0x00000000` in all seven) and sequential
   order words (`0x0040003E` .. `0x00460044`) -- nothing in the profile struct carries a
   per-profile class tag; my draft's own struct words match these exactly (see "Profile data"
   below).
4. **EMPIRICAL cross-check, the sibling's own method** (`probe.cpp`, `probe2.cpp`, both in this
   directory): the naive `ACTOR_PROFILE` macro invoked seven times for one shared class name
   FAILS TO COMPILE -- `void *className##_classInit()` (`f_profile.hpp:16`) keys the generated
   function name on the CLASS, not the profile, so seven invocations collide: `(10333) object
   'daProbe_c_classInit()' redefined`. That rules out the plain macro (my draft below hand-expands
   `CUSTOM_ACTOR_PROFILE`'s body per profile instead) and independently explains why a
   single-class, many-profile family looks the way it does in this codebase: it CAN'T be written
   with the standard macro, so whoever wrote the original source had to hand-expand it too.
   `probe2.cpp` (manually-named classInits, matching this draft's own shape) compiles its seven
   classInit bodies BYTE-SHAPE-IDENTICAL to target (differing only in the allocation-size
   immediate, expected since the probe class has no added members).

## Bounds: SEVEN profiles, not six -- corrected from the coordinator's dispatch

`scout_unit.py d_basesNP 0x7d400 0x7d5e0` matches the coordinator's own report exactly (six
profiles, one `.data` target, no `.ctors`) -- that part of the dispatch is accurate as far as it
goes. But `python -c` walking `profile_map.py`'s relocations directly (not just scout_unit's
profile-name filter) shows a SEVENTH profile, `g_profile_AC_RNSWICH`, whose own classInit resolves
to `fn_2_7D5E0` -- immediately adjacent, same 0x4C-byte-body + 4-byte-gap shape as the other six
(confirmed against `bin/dtk/d_basesNP_symbols.txt`: `fn_2_7D5E0` size `0x4C`, then
`fn_2_7D630` size `0x4D8` -- a much bigger REAL function, not another classInit).
`include/game/bases/d_profile.hpp` independently corroborates this, unprompted by anything
address-based: `g_profile_AC_FLAGON` through `g_profile_AC_RNSWICH` are declared as one unbroken
run of seven `extern fProfile::fActorProfile_c` lines, between `g_profile_EN_STAR_COIN_VOLT` and
`g_profile_EN_BKBLOCK` -- the header's own author already grouped these seven, not six.

Re-scouted the corrected range: `scout_unit.py d_basesNP 0x7d400 0x7d630` stays exactly as clean
as the six-profile range (one `.data` target, `0x1BC30`; no `.ctors`) -- extending the hi bound by
one profile's worth introduced NO new pool leakage. Extending one stride further,
`0x7d400-0x7d650`, immediately picks up a `.bss` target and a `sec11` target that belong to
`fn_2_7D630` (the real member-function body right after), not to any classInit -- confirming
`0x7D630` is the tight, correct upper bound and not an arbitrary stopping point.

**This is a real bounds correction, not a restatement of the coordinator's own numbers** -- said
plainly per the brief's own instruction to flag a mis-scoping directly.

## Tally: 7/7 byte-identical modulo symbol names, order GREEN

```
python wip/wm_units/agent_ac_switch/build.py
```
```
addr       target                  size  result
0x0007d400 fn_2_7D400                19  MATCH  <- classInit_AC_FLAGON__Fv
0x0007d450 fn_2_7D450                19  MATCH  <- classInit_AC_4SWICHAND__Fv
0x0007d4a0 fn_2_7D4A0                19  MATCH  <- classInit_AC_4SWICHOR__Fv
0x0007d4f0 fn_2_7D4F0                19  MATCH  <- classInit_AC_RANDSWICH__Fv
0x0007d540 fn_2_7D540                19  MATCH  <- classInit_AC_CHNGESWICH__Fv
0x0007d590 fn_2_7D590                19  MATCH  <- classInit_AC_IFSWICH__Fv
0x0007d5e0 fn_2_7D5E0                19  MATCH  <- classInit_AC_RNSWICH__Fv

7/7 byte-identical modulo symbol names
```
No "FUNCTION ORDER IS WRONG" (exit code 0). `.ctors`: absent in the draft (`grep '^\.'
draft.txt` shows only `.text`/`.data`, no `.ctors` section at all), matching the coordinator's own
prediction.

## Function inventory (all 7 matched)

| draft name | target | size | notes |
|---|---|---|---|
| `classInit_AC_FLAGON` | `fn_2_7D400` | 19/19 | naming-only diff (own `__vt__11daFlagObj_c` vs target's anonymous `lbl_2_data_1BC30`) |
| `classInit_AC_4SWICHAND` | `fn_2_7D450` | 19/19 | same |
| `classInit_AC_4SWICHOR` | `fn_2_7D4A0` | 19/19 | same |
| `classInit_AC_RANDSWICH` | `fn_2_7D4F0` | 19/19 | same |
| `classInit_AC_CHNGESWICH` | `fn_2_7D540` | 19/19 | same |
| `classInit_AC_IFSWICH` | `fn_2_7D590` | 19/19 | same |
| `classInit_AC_RNSWICH` | `fn_2_7D5E0` | 19/19 | same |

Every allocation constant matches: all seven emit `li r3, 0x3f8`, matching every target classInit
exactly (`sizeof(daFlagObj_c) == 0x3f8`, read directly off the target's own immediate, not
computed).

## Profile data (`g_profile_AC_*`) -- confirmed byte-identical field by field

My draft's `.data` (`draft.txt:189-...`) reproduces the target's own six `0xC`-byte structs
exactly:

| profile | target order word | draft order word |
|---|---|---|
| AC_FLAGON | `0x0040003E` | `0x0040003E` |
| AC_4SWICHAND | `0x0041003F` | `0x0041003F` |
| AC_4SWICHOR | `0x00420040` | `0x00420040` |
| AC_RANDSWICH | `0x00430041` | `0x00430041` |
| AC_CHNGESWICH | `0x00440042` | `0x00440042` |
| AC_IFSWICH | `0x00450043` | `0x00450043` |
| AC_RNSWICH | `0x00460044` | `0x00460044` |

`properties` is `0x00000000` in all seven, target and draft alike. AC_RNSWICH's own struct dumps
`0x10` bytes in the target (not `0xC` like its six siblings); that extra 4 bytes lands exactly at
`0x1BC30`, the vtable's own required 16-byte alignment boundary (`0x1BC20+0xC=0x1BC2C`, unaligned;
`+0x10=0x1BC30`, aligned) -- read as compiler-inserted padding on the LAST object before the
vtable, not a real extra field, so my draft declares its struct identically to the other six
(and, having no vtable object of its own immediately after it, correctly does NOT reproduce that
padding -- confirmed harmless since it is alignment filler, not data).

## STRUCTURAL FINDING: this unit cannot be independently LINKED yet (only verified)

Cross-checked the vtable's real-function slots directly against `include/game/framework/f_base.hpp`'s
own virtual declaration order (not guessed from position): `daFlagObj_c` overrides exactly THREE
of `fBase_c`'s virtuals --

- `create()` -- vtable slot 2 / offset `0x08` -> `fn_2_7D630`, **0x4D8 bytes**
- `execute()` -- vtable slot 8 / offset `0x20` -> `fn_2_7DB10`
- `~daFlagObj_c()` -- vtable slot 18 / offset `0x48` -> `fn_2_7EC90` (one-slot, flag-argument
  shape -- same ABI `d_a_dummy_door.cpp` already established)

-- plus seven `STATE_FUNC_DECLARE`-shaped (non-virtual) states whose init/execute/finalize
triples run `fn_2_7DD10` through `fn_2_7E9E0`. All of that lives from `0x7D630` to at least
`0x7EC90+` (**>0x2C90, >11KB**) -- far outside this unit's own `0x7D400-0x7D630` classInit span.
`grep -r "daFlagObj_c" source/ include/` is EMPTY: nobody has landed any part of that body
anywhere.

A TU emits a class's vtable as a strong local definition only where the class's key function is
DEFINED, not merely declared -- confirmed empirically, not textbook-quoted: `probe2.cpp` declares
`create()`/`execute()`/the destructor but does not define them, and its compiled classInit bodies
reference the vtable through an ORDINARY EXTERNAL relocation (`__vt__11daProbe2_c@ha`/`@l`), with
no local `.data` vtable object anywhere in that TU's own output. My real draft, `d_a_ac_switch.cpp`,
does the same thing for the same reason (`grep '__vt__' draft.txt` shows only relocations, and
`grep '^\.data'` shows only the seven profile structs, no vtable object).

Practically: this unit's `.text` is genuinely self-contained and 7/7-verified against the target
(the classInit bytes never touch create()/execute()/the states), but it will not LINK on its own
until whichever unit eventually implements `daFlagObj_c`'s real body (create/execute/dtor/seven
states, `0x7D630` onward) lands too -- that is a substantially bigger unit than this one, not a
detail to patch here. Not something I attempted to work around unilaterally (no `R_` symbol trick
applies -- that technique redirects a `bl`/data reference I control the name of; the vtable
reference here is compiler-generated from the class's own mangled name and isn't something a
single small TU can redirect).

## Member layout: opaque padding only, by design

`sizeof(daFlagObj_c) == 0x3f8`, read directly off every classInit's own `li r3, 0x3f8`.
`sizeof(dActorState_c) == 0x3d0`, confirmed empirically (`probe2.cpp`'s own same-shape class with
no added members allocates `0x3d0`). The `0x28`-byte remainder is declared as
`u8 mUnknown3D0[0x28];` -- completely unanalysed, since every consumer of that member data
(`create()`, `execute()`, the seven states) sits outside this unit's own range. Not a guess dressed
up as a field: explicitly commented as size-only in the draft.

## Nothing else parked

Every function in range matched; order gate green; `.ctors` gate green by confirmed absence;
profile-data fields confirmed byte-identical. The one open item is the structural
cannot-link-standalone finding above, which is a scope/sequencing fact for the coordinator to
route (land alongside or after whatever unit gets `daFlagObj_c`'s real body), not a defect in this
draft.

## Files

- `d_a_ac_switch.cpp` -- the draft, 7/7.
- `probe.cpp` -- empirical probe #1: proves the naive `ACTOR_PROFILE` macro can't be reused across
  profiles sharing one class (compile error, quoted above).
- `probe2.cpp` / `probe2.o` / `probe2.txt` -- empirical probe #2: proves classInit-shape-identical
  output and the external (not locally-defined) vtable reference when create/execute/dtor are
  declared but not defined.
- `target_auto_00_0007D050_text.txt` -- fresh target `.text` dump (this session, not reused).
- `target_auto_04_000132B0_data.txt` -- fresh target `.data` dump (this session, not reused;
  matches the file of the same name in `agent_dummy_door/` exactly where checked, confirming
  neither is stale).
