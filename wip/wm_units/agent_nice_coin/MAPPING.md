# AC_NICE_COIN / AC_NICE_COIN_REGULAR (`daNiceCoin_c`) -- function inventory

Unit bounds: `.text 0x104d70-0x105450` (0x6E0 bytes), ONE `.ctors` entry
(`0x2b0 -> __sinit .text 0x105110`, confirmed via `ctors_map.py`, matches the
coordinator's stated address exactly). Confirmed clean via
`check_target_objs.py` after adding the missing split object
`auto_fn_2_105110_text.o` (dtk splits `__sinit` into its own
`auto_fn_2_<ADDR>_text.o`, distinct from the two `auto_00_*` blocks
`auto_00_00104BE8_text.o`/`auto_00_001052D0_text.o` that cover the rest of
the range) -- the coordinator caught this mid-round; the true denominator is
19 target functions, not 18.

## ONE CLASS, TWO PROFILES -- confirmed, not assumed

Both classInits (`fn_2_104D70` for `AC_NICE_COIN`, `fn_2_104DC0` for
`AC_NICE_COIN_REGULAR`) are byte-identical in shape: same `li r3, 0x3f0`
allocation size, same `bl __ct__13dActorState_cFv` (a BASE ctor call, not a
derived one -- this class declares no ctor of its own), same vtable-pointer
patch to the same symbol (`lbl_2_data_33790`) at the same `+0x60` offset
(this engine's consistent non-zero vtable-pointer location for
`fBase_c`-derived classes, independently corroborated by WM_KOOPAJR's
identical `this+0x60` dispatch in an unrelated class hierarchy branch).

Standard `ACTOR_PROFILE` macro CANNOT be used twice for one class --
reproduced directly: `className##_classInit()` (f_profile.hpp:16) collides
("(10333) object 'daNiceCoin_c_classInit()' redefined"). Fixed using the
exact pattern already landed in `source/d_basesNP/bases/d_a_ac_switch.cpp`
(seven profiles, one class, same problem): hand-expand
`CUSTOM_ACTOR_PROFILE`'s body per profile with distinctly-named `static void
*classInit_PROFNAME()` functions. `properties` = 0 for both (read directly
off each profile struct's third `.4byte` word, both `0x00000000`).

## Class name and states -- read directly from raw REL bytes, not invented

`lbl_2_data_33790` (the vtable, `.data` file offset `0x1D0C00+0x33790`) has,
immediately after its ~52-entry vtable body, two `sFStateID_c<daNiceCoin_c>`
static objects (from two `STATE_DEFINE` invocations) whose embedded ASCII
name strings were read directly out of the REL: `"daNiceCoin_c::StateID_Search"`
and `"daNiceCoin_c::StateID_EndWait"`, matching `STATE_DEFINE`'s own
`#class "::StateID_" #name` string literal (`include/game/sLib/s_State.hpp`).
Class is `daNiceCoin_c : public dActorState_c` (state framework precedent:
`grep -rl STATE_DEFINE source/` shows ten already-landed TUs use it, most
directly comparable being `source/d_basesNP/bases/d_a_remo_door.cpp`, same
REL, whose `STATE_FUNC_DECLARE`/`STATE_DEFINE`/`mStateMgr.changeState(...)`
idiom this draft follows).

`sizeof(dActorState_c) == 0x3d0`, confirmed empirically in the landed
`d_a_ac_switch.cpp` (a same-shape class adding no members allocates 0x3d0).
`sizeof(daNiceCoin_c) == 0x3f0` (every classInit's own `li r3, 0x3f0`), so
this class adds exactly 0x20 bytes of its own fields (offsets `0x3d0`-
`0x3ef`), not yet laid out/typed (see below).

## Vtable slot identity vs TEXT ADDRESS order -- two independent axes, do not conflate

Cross-checked against `fBase_c`'s own virtual declaration order
(`include/game/framework/f_base.hpp`): `create, preCreate, postCreate,
doDelete, preDelete, postDelete, execute, preExecute, postExecute, draw,
preDraw, postDraw, deleteReady, ...`. Every `preX`/`postX` slot in the
target's vtable dump names an INHERITED, unmodified `dActor_c` symbol, so
the four anonymous slots preceding them are `create()`, `doDelete()`,
`execute()`, `draw()` -- NOT `preCreate`/`preDelete`/`preExecute`/`preDraw`
overrides (a naming-adjacency trap avoided by cross-checking the header's
declaration order directly rather than assuming from position).

BUT vtable slot order and `.text` DEFINITION order are independent axes --
an early mistake this round conflated them (see the reorder note in the
source, kept as a paper trail). Ground truth for TEXT address comes from
`bin/dtk/d_basesNP_symbols.txt` directly: `create()=fn_2_104E10`,
`execute()=fn_2_104F20`, `draw()=fn_2_104F50`, `doDelete()=fn_2_104F60` --
ascending address order is `create, execute, draw, doDelete`, and that is
the required `.cpp` definition order (confirmed: after fixing to this order,
`verify_anon`'s FUNCTION-ORDER-IS-WRONG check cleared). `execute()` is
doubly confirmed -- both by the vtable slot AND by a genuine 12-instruction
non-trivial content match (`mStateMgr.executeState(); return SUCCEEDED;`).
`draw()`/`doDelete()` are both trivial one-liners (`return SUCCEEDED;`) and
their MATCH status should be treated as plausible, not independently
verified -- verify_anon's content-based pairing cannot distinguish two
identical trivial stubs from each other; only the vtable-derived identity
argument above is solid ground for WHICH slot each one is.

The destructor (`fn_2_1050B0`, confirmed via direct disassembly read: the
standard `(this, shouldFree)` scalar-deleting-destructor shape, same as the
landed `d_a_dummy_door.cpp`'s `fn_2_77B40`, calling `dActorState_c`'s
destructor directly -- this class declares no destructor logic of its own
either) is declared out-of-line (`virtual ~daNiceCoin_c(); ... {}`) to force
GLOBAL binding (target's `fn_2_1050B0` is `global`, not `weak`) and is
defined LAST in the `.cpp`, after all six state methods -- matching its real
text address (`0x1050B0`, after `0x1050A0`).

## Status table

| target | role | size | status |
|---|---|---|---|
| `fn_2_104D70` | classInit AC_NICE_COIN | 0x4C | MATCH* (1/19 lines, own vtable symbol) |
| `fn_2_104DC0` | classInit AC_NICE_COIN_REGULAR | 0x4C | MATCH* (1/19 lines, own vtable symbol) |
| `fn_2_104E10` | `create()` | 0x10C | NOT AUTHORED -- real logic identified, not yet translated (see below) |
| `fn_2_104F20` | `execute()` | 0x30 | MATCH (0/12 lines) |
| `fn_2_104F50` | `draw()` | 0x8 | MATCH (trivial stub; identity from vtable slot, not independently content-verified) |
| `fn_2_104F60` | `doDelete()` | 0x8 | MATCH (trivial stub; same caveat) |
| `fn_2_104F70` | `initializeState_Search` | 0x4 | MATCH (trivial, `{}`) |
| `fn_2_104F80` | `finalizeState_Search` | 0x4 | MATCH (trivial, `{}`) |
| `fn_2_104F90` | `executeState_Search` | 0xE8 | NOT AUTHORED -- real logic identified, not yet translated (see below) |
| `fn_2_105080` | `executeState_EndWait` | 0x4 | MATCH (trivial, `{}`) |
| `fn_2_105090` | `initializeState_EndWait` | 0x4 | MATCH (trivial, `{}`) |
| `fn_2_1050A0` | `finalizeState_EndWait` | 0x4 | MATCH (trivial, `{}`) |
| `fn_2_1050B0` | `~daNiceCoin_c()` | 0x58 | MATCH (0/22 lines) |
| `fn_2_105110` | `__sinit` | 0x1C0 | MATCH (0/112 lines) -- fully byte-identical, confirms class name/state names/STATE_DEFINE usage exactly right |
| `fn_2_1052D0` | `sFStateID_c<daNiceCoin_c>::~sFStateID_c()` | 0x58 | MATCH (auto-generated template instantiation, not hand-written) |
| `fn_2_105330` | `sFStateID_c<daNiceCoin_c>::isSameName()` | 0x88 | MATCH (auto-generated) |
| `fn_2_1053C0` | `sFStateID_c<daNiceCoin_c>::initializeState()` | 0x30 | MATCH (auto-generated) |
| `fn_2_1053F0` | `sFStateID_c<daNiceCoin_c>::executeState()` | 0x30 | MATCH (auto-generated) |
| `fn_2_105420` | `sFStateID_c<daNiceCoin_c>::finalizeState()` | 0x30 | MATCH (auto-generated) |

15/19 MATCH + 2/19 MATCH* (classInits) = 17/19 logic-complete. 2/19
(`create()`, `executeState_Search()`) genuinely not yet authored.

## `create()` (`fn_2_104E10`) -- read, not yet translated

Reads `this+0x8` (a `u16`, likely `mProfName`) and compares to `0x251`
(`fProfile::AC_NICE_COIN_REGULAR`'s own enum value per the profile struct's
packed word -- this is precisely how the two profiles' shared class tells
them apart at runtime: not a constructor argument, a runtime profile-name
check), setting a new field `+0x3d8` (bool-ish) accordingly. Then computes
`(u16)mPos.x` and `(u16)-mPos.y` (via `fctiwz`, truncating float-to-int) and
calls `dBg_c::m_bg_p->CoinGetBitCheck(u16, u16, int)` -- UNDECLARED
anywhere in `include/` or landed in `source/` (`grep -rln CoinGetBitCheck`
is empty) -- would need a shadow header addition with written proof before
it can be called; not attempted this round. If that check returns true,
`create()` returns early with `2` (`fBase_c::MAIN_STATE_e::SUCCESS`, need to
confirm the enum ordering) -- otherwise it reads THREE `ACTOR_PARAM`-style
bit-fields out of `mParam` (`this+0x4`, bits `[28:32)`, `[16:24)`, `[8:16)`)
into three new fields (`+0x3d0`, `+0x3dc`, `+0x3e0`), derives a fourth field
(`+0x3d4`, a small enum/mode: 0, 1, or 2, from a two-way comparison of the
other two), and ends with a call through the (inherited) state manager's own
dispatch table at `this+0x394` (i.e. `mStateMgr`'s internal current-state
pointer) slot `+0x18`, passing `&lbl_2_bss_C9E0` -- very likely
`mStateMgr.changeState(...)` with a `.bss`-resident state-ID-shaped
argument, not yet matched to a specific C++ call shape.

## `executeState_Search()` (`fn_2_104F90`) -- read, not yet translated

Checks the new fields (`+0x3d4` against 0/2, `+0x3e4` vs `+0x3dc`, and a
second comparable pair `+0x3e8` vs `+0x3e0`) to decide whether a "reveal"
condition is met. If so: calls `dMultiMng_c::mspInstance->setClapSE()`
(header-declared? not yet checked), then repeats the same `(u16)mPos.x,
(u16)-mPos.y` conversion as `create()` plus a `this+0x38f` byte flag, and
calls `dBg_c::m_bg_p->CoinGetBitSet(u16, u16, int)` -- same undeclared-symbol
situation as `CoinGetBitCheck`. Ends with the same `this+0x394`/`+0x18`
dispatch pattern as `create()`, this time with `&lbl_2_bss_CA20` (a
DIFFERENT `.bss` object than `create()`'s `lbl_2_bss_C9E0` -- two distinct
state-transition targets, consistent with `create()` and
`executeState_Search()` each transitioning to a different state under
different conditions).

## Not yet laid out

The 0x20 bytes of `daNiceCoin_c`'s own fields (`+0x3d0`-`+0x3ef`) are
identified by OFFSET and BY WHICH BITS OF `mParam` feed them, but not yet
declared as real typed class members (parked pending `create()`'s full
translation, to avoid guessing field types/names ahead of understanding
their consumers). `+0x38f` sits INSIDE `dActorState_c`'s own 0x3d0-byte
footprint (not ours to declare -- it's a base-class field, likely part of
`mStateMgr` or another inherited member).

## Caveat on "MATCH" in this document

Same caveat as every other unit: `verify_anon.py` compares modulo relocation
SYMBOL NAMES, not actual link-time pool/relocation identity, and its
content-based pairing can mismatch two functions with identical trivial
bodies (documented explicitly in its own docstring). `execute()`,
`~daNiceCoin_c()`, and `__sinit` are HIGH CONFIDENCE (non-trivial, unique
content). `draw()`/`doDelete()`'s identity rests on the vtable-slot argument
above, not content uniqueness -- worth an independent recheck once real
content differentiates them (unlikely to ever happen, both are genuinely
trivial in the target too).

## ROUND 2 — shadow header proposed, create() authored, one blocker remains

Coordinator supplied the proof I was missing: both `CoinGetBitCheck`/
`CoinGetBitSet` are in `bin/dtk/wiimj2d_symbols.txt` (the full DOL map),
mangled `__5dBg_cFUsUsi` = `(u16, u16, int)`, confirming the inferred
signature outright. Proposed shadow addition at
`wip/wm_units/agent_nice_coin/shadow_include/game/bases/d_bg.hpp`:
`bool CoinGetBitCheck(u16 x, u16 negY, int index);` and
`void CoinGetBitSet(u16 x, u16 negY, int index);` — return types read off the
only two call sites in the codebase (both in this draft): CoinGetBitCheck's
result is tested (`cmpwi r3,0; beq ...`) and consumed as `if
(CoinGetBitCheck(...)) return 2;`, proving non-void, `bool` being the
natural type for a value used only as a condition; CoinGetBitSet's result is
never read after the call, consistent with `void`.

`create()` authored using this: profName check (`this+0x8` vs `0x251`,
`AC_NICE_COIN_REGULAR`'s own profile-order word — a RUNTIME check, since
both classInits call the identical base ctor), `CoinGetBitCheck((u16)mPos.x,
(u16)-mPos.y, dActor_c::m_mbgchoice_keep)`, and the three `ACTOR_PARAM`-
shaped `mParam` bit-fields (now declared as real fields `mUnk3d0`/`mUnk3dc`/
`mUnk3e0`/`mUnk3d4`, offsets `+0x3d0`/`+0x3dc`/`+0x3e0`/`+0x3d4` — corrected
an initial bit-position mixup between `mUnk3dc`/`mUnk3e0` by re-deriving
`extrwi`'s exact semantics rather than trusting a first guess).

**One piece deliberately NOT authored: the trailing `mStateMgr.changeState(...)`-
shaped dispatch** (confirmed as `changeState`'s vtable slot — `+0x18` off
`this+0x394` — by cross-referencing `s_StateMgr.hpp`'s declared virtual
order against `execute()`'s already-confirmed `+0x10` `executeState()`
dispatch). `create()`'s call passes `&lbl_2_bss_C9E0` (0x40 bytes);
`executeState_Search()`'s passes `&lbl_2_bss_CA20` (0x30 bytes) — different
sizes, so different types, and NEITHER matches `StateID_Search`/
`StateID_EndWait` (both already accounted for, in `.data`, proven by the
byte-perfect `__sinit` match) nor `sStateID::null` (an `extern`, named
symbol — would show its real mangled name, not an anonymous `.bss` label).
`executeState_Search()` also needs two more fields (`mUnk3e4`, `mUnk3e8`)
that are read/compared in that function but never WRITTEN anywhere in
`create()` or `executeState_Search()` itself — some other, not-yet-
identified function must set them. Left as a flagged gap rather than
guessed, consistent with the project's standing "a wrong constant/target
moves things further from correct, not closer" rule.

Tally unchanged at 15/19 MATCH-class (the two unauthored functions still
don't byte-match, expected — the missing `changeState()` call and register-
allocation drift from the rest of the body being present now cascade
through the whole function, same effect WM_KOOPAJR's cascading-diff lesson
described). Real, structural progress: `create()`'s closest-candidate size
match went from an unrelated function to itself, and its logic is now
correctly shaped for everything except the one flagged call.
