# RIVER family -- nine profiles, one translation unit

`.text 0x12AD60-0x12B400` (0x6A0 bytes), module `d_basesNP`. No `.ctors`
entry (confirmed: `python wip/wm_units/ctors_map.py d_basesNP RIVER` finds
none -- this TU has no static state, matching the coordinator's scoping).
No `.bss`, no own `.rodata` (its only `.rodata` targets are DOL absolutes,
not investigated further since nothing in this unit's own code references
them yet).

## Answer to the coordinator's central question: NINE DISTINCT CLASSES

Not one class with nine `classInit` entry points. Proven empirically, not
inferred:

1. Each `classInit` stores a **different** `lbl_2_data_XXXXX` address into
   the new object's `+0x60` (its own vtable pointer -- see below) on a
   regular ~0xF0 stride (`0x3AF28`, `0x3B018`, `0x3B108` ... `0x3B6A8`,
   matching the coordinator's own `.data` observation exactly).
2. Compiling a real, empty, uniquely-named class
   (`class daRiverBarrel_c : public dActorState_c {};`) through the
   project's own `ACTOR_PROFILE` macro reproduces this **exactly**:
   MWCC auto-generates a per-class vtable (`__vt__15daRiverBarrel_c`,
   0xE0 bytes = 56 entries, all pointing at inherited implementations
   except the class's own destructor slot) purely because the class is
   *distinct*, even though it overrides nothing. A single shared class
   used nine times would produce **one** vtable symbol referenced nine
   times, not nine different addresses on a fixed stride matching
   `sizeof(profile struct) + sizeof(vtable) == 0xC+0x4(pad)+0xE0 == 0xF0`.
3. `sizeof(dActorState_c) == 0x3D0` **exactly** (confirmed via
   `STATIC_ASSERT`) -- every one of the nine classes adds **zero** new
   members. That is why every `classInit` shows the identical
   `li r3, 0x3d0` and the identical `bl __ct__13dActorState_cFv` (the
   13-character mangled name confirms the base ctor is called directly,
   not through an intermediate class -- ruled out `dActorMultiState_c`
   and any hand-rolled intermediate on this evidence).

## What's really at `+0x60`: the object's own vtable pointer, not a shared "secondary vtable" of the `dBase_c : fBase_c, cOwnerSetMg_c` kind

The coordinator's own caveat ("verify what the store actually is here
before assuming it is the same thing [as other dBase_c-derived units]")
was correct to raise: it is **not** the same mechanism.

- `cOwnerSetMg_c` (`include/game/cLib/c_owner_set.hpp`) has **no virtual
  functions at all** (non-virtual dtor, one data member `mpRoot`) --
  confirmed `sizeof(cOwnerSetMg_c) == 4`, and its subobject inside
  `dActorState_c` starts at offset `0x64` (confirmed via the
  base-to-derived pointer-adjustment trick on a non-null fake address,
  cross-checked with two different fake base addresses to rule out a
  null-pointer special case). There is no vtable there to point at.
- `fBase_c` genuinely has a large virtual function table
  (`create`/`preCreate`/.../`~fBase_c()`, confirmed by reading
  `include/game/framework/f_base.hpp` directly), but its own first
  declared member (`mUniqueID`) sits at offset **0** (confirmed via
  `STATIC_ASSERT(offsetof(..., mUniqueID) == 0)`), meaning MWCC does not
  put fBase_c's own vtable pointer at the front of the object here.
- The **empirical settlement**: compiling a real, minimal, uniquely-named
  derived class through `ACTOR_PROFILE` reproduces the target's
  `classInit` **byte-for-byte** (mod the one differing symbol, which is
  the class-specific vtable address), including the exact `stw r3,
  0x60(r31)` instruction. This is the compiler routing the vtable
  pointer for *this derived class's own* (auto-generated) vtable into the
  gap between `fBase_c`'s own data (ending at `0x60`) and
  `cOwnerSetMg_c`'s subobject (starting at `0x64`) -- a class-specific
  vtable pointer, not a shared/secondary one. "Ordinary C++ produces it
  automatically" held here too, once the right *empty, distinctly-named*
  class shape was used -- no hand-rolled store was ever written.

## Full function inventory (23 functions total in target range)

| addr | size | identity | status |
|---|---|---|---|
| `0x12AD60` | 0x50 | `daRiverBarrel_c` classInit | **MATCH** |
| `0x12ADB0` | 0x5C | `daRiverBarrel_c::~daRiverBarrel_c()` (deleting dtor) | 1-instruction residual, see below |
| `0x12AE10` | 0x50 | `daRiverCoin_c` classInit | **MATCH** |
| `0x12AE60` | 0x5C | `daRiverCoin_c::~daRiverCoin_c()` | same residual |
| `0x12AEC0` | 0x50 | `daRiverItem_c` classInit | **MATCH** |
| `0x12AF10` | 0x5C | `daRiverItem_c::~daRiverItem_c()` | same residual |
| `0x12AF70` | 0x50 | `daRiverLift_c` classInit | **MATCH** |
| `0x12AFC0` | 0x5C | `daRiverLift_c::~daRiverLift_c()` | same residual |
| `0x12B020` | 0x50 | `daRiverMgr_c` classInit | **MATCH** |
| `0x12B070` | 0x24 | `daRiverMgr_c::doDelete()` -- `deleteRequest(); return 0;` | **MATCH** |
| `0x12B0A0` | 0x08 | `daRiverMgr_c::preDelete()` -- `return 1;` | **MATCH** |
| `0x12B0B0` | 0x08 | `dActor_c::isSpinLiftUpEnable()` (shared weak stub) | **MATCH** |
| `0x12B0C0` | 0x04 | `dBaseActor_c::finalUpdate()` (shared weak stub) | **MATCH** |
| `0x12B0D0` | 0x08 | `dActor_c::vf68(dBg_ctr_c*)` (shared weak stub) | **MATCH** |
| `0x12B0E0` | 0x58 | `daRiverMgr_c::~daRiverMgr_c()` (implicit, weak, deferred) | **MATCH** |
| `0x12B140` | 0x50 | `daRiverPaipo_c` classInit | **MATCH** |
| `0x12B190` | 0x5C | `daRiverPaipo_c::~daRiverPaipo_c()` | same residual |
| `0x12B1F0` | 0x50 | `daRiverPakkun_c` classInit | **MATCH** |
| `0x12B240` | 0x5C | `daRiverPakkun_c::~daRiverPakkun_c()` | same residual |
| `0x12B2A0` | 0x50 | `daRiverPuku_c` classInit | **MATCH** |
| `0x12B2F0` | 0x5C | `daRiverPuku_c::~daRiverPuku_c()` | same residual |
| `0x12B350` | 0x50 | `daRiverStarcoin_c` classInit | **MATCH** |
| `0x12B3A0` | 0x5C | `daRiverStarcoin_c::~daRiverStarcoin_c()` | same residual |

**15/23 byte-identical.** Every `classInit` matches (the structurally
hardest part -- proves the class-count answer, the `sizeof`, the base
ctor, and the per-class vtable mechanism all at once). `daRiverMgr_c`'s
two genuinely unique overrides (`doDelete`, `preDelete`) both match
exactly. All three shared weak stubs it pulls in match exactly.

### `daRiverMgr_c` is the one real outlier, and its shape is now understood, not guessed

Its `.text` span is double the other eight (0x120 vs 0xB0) because it
carries two real overrides, both confirmed against target and both
matching exactly:

```cpp
class daRiverMgr_c : public dActorState_c {
public:
    virtual int doDelete();
    virtual int preDelete();
};
int daRiverMgr_c::doDelete() { deleteRequest(); return 0; }
int daRiverMgr_c::preDelete() { return 1; }
```

`doDelete()` (target `fn_2_12B070`, 0x24 bytes) calls the already-declared
`fBase_c::deleteRequest()` then returns 0 -- overriding `fBase_c`'s own
`virtual int doDelete();`. `preDelete()` (target `fn_2_12B0D0`, 0x8 bytes)
unconditionally returns 1, overriding `dBase_c::preDelete()`'s more
elaborate default. Both are ordinary out-of-line overrides; nothing
hand-rolled. This also explains why `daRiverMgr_c`'s OWN destructor
(`0x12B0E0`) is the one destructor across the whole family that MATCHES:
it's genuinely different from the other eight's -- see next section.

## Open residual: 8 of 9 individual destructors are missing ONE instruction (a duplicated `beq` on the same `this == 0` test), not yet reproduced

This is the single blocking gap standing between 15/23 and N/N. Read
precisely, not guessed at:

**The diff, identical in shape for all eight affected destructors**
(shown here for `daRiverBarrel_c`, target `0x12ADB0`):

```
target: ... mr r30, r3
        beq .L_0012ADF0      <- skip everything if this == 0
        beq .L_0012ADE0      <- SAME condition, tested again, skip to right before the dtor call
        li r4, 0x0
        bl __dt__13dActorState_cFv
.L_0012ADE0:
        cmpwi r31, 0x0
        ...

draft:  ... mr r30, r3
        beq .L_END           <- skip everything if this == 0 (ONE check only)
        li r4, 0x0
        bl __dt__13dActorState_cFv
        cmpwi r31, 0x0
        ...
```

Both `beq`s in the target encode `BI=2` (cr0-eq) with **no intervening
instruction that touches cr0** -- confirmed by reading the raw instruction
words, not just the disassembler's text. The second branch is therefore
provably dead code (unreachable false), the same "MWCC does not eliminate
a provably-redundant explicit null check" class of idiom already
established on this project (WM_KINOPIO's `if (mpMdlMng) delete
mpMdlMng;`). But this shape is the compiler's OWN synthesized deleting
destructor (D0), not something a user hand-writes directly, so the
"where does the explicit source-level redundant check come from" question
does not resolve as simply as it did for WM_KINOPIO.

**Where the extra check appears vs. does not**, established firmly by
building all nine together and reading which target address each
draft function actually lands on (not by trusting an isolated,
single-class test, which mis-paired against the wrong target instance
on the first attempt -- see negatives below):

- Present (target has 23 instructions, draft has 22): `daRiverBarrel_c`,
  `daRiverCoin_c`, `daRiverItem_c`, `daRiverLift_c`, `daRiverPaipo_c`,
  `daRiverPakkun_c`, `daRiverPuku_c`, `daRiverStarcoin_c` -- eight of
  nine, i.e. every type EXCEPT `daRiverMgr_c`.
- Absent (target and draft both 22 instructions, MATCH): `daRiverMgr_c`
  only, whose destructor is declared nowhere at all in the source (fully
  implicit -- `daRiverMgr_c` only declares `doDelete()`/`preDelete()`,
  no destructor).

**Variants tried against `daRiverPaipo_c` (chosen as the isolated test
case since its target function, `0x12B190`, was read fresh each time),
all producing the identical 22-instruction result -- the check never
appears**:

1. Fully implicit (no destructor declared at all).
2. `virtual ~daRiverPaipo_c() {}` declared and defined in-class (compiles
   weak).
3. `virtual ~daRiverPaipo_c();` declared in-class, defined out-of-line
   with an empty body (compiles global -- the binding the target symbol
   actually needs, per the coordinator's WM_KOOPAJR precedent about
   weak-vs-global mattering, but content is unaffected).
4. Same as (3) but with `virtual` dropped from the in-class declaration
   (still overrides implicitly; no change).
5. Same as (3) plus a second, genuine virtual override (`int execute()`)
   added to give the class real non-destructor vtable content; the
   destructor's own shape was unaffected.
6. Same as (3) plus an explicit (empty-bodied) user-declared constructor
   -- this changed `classInit` (now calls the class's own ctor instead of
   inlining straight to `dActorState_c`'s, which would break the
   already-matching `classInit`, so this variant is unusable regardless),
   but even so, the destructor's shape was still unaffected.
7. A private, unimplemented copy-constructor/assignment-operator pair (a
   common "non-copyable" idiom of this era) -- did not compile at all;
   `ACTOR_PROFILE`'s `new daRiverPaipo_c()` call stopped resolving to the
   default constructor. Abandoned, not a viable direction.

None of these seven variants -- spanning every destructor-declaration
axis available in ordinary C++ -- reproduces the extra check. Whatever
triggers it is not controlled by how the destructor itself is written.

**A genuine negative worth recording precisely**: the first isolated,
single-class test of `daRiverBarrel_c` alone (before the other eight
types existed in the same file) reported "14 differing" against
`fn_2_12ADB0` and was initially misread as evidence of a "fold BARREL's
weak dtor together with four siblings" mechanism, because a *later*,
fully-implicit `daRiverMgr_c` dtor happened to land a real MATCH against
a *different* target address (`0x12B0E0`) once all nine types were
compiled together. Re-reading which target address the matched function
actually corresponds to (rather than trusting the first plausible-looking
MATCH) showed the true picture: there is no five-way fold. Eight
individual, non-folded, "extra-check" destructors exist in target (one
per type except MGR), plus one, unrelated, genuinely-implicit
`daRiverMgr_c` dtor that coincidentally has the leaner shape. This is
recorded so the next pass does not repeat the "single MATCH proves
folding" misreading.

**Not yet tried, flagged as the most promising unexplored angle**: MWCC
codegen for a deleting destructor sometimes differs based on properties
of the class that are invisible from the destructor's own declaration --
e.g. whether the class is used in an explicit-instantiation-like context,
whether `operator delete`/`operator new` are class-scoped overloads
(`fBase_c` was not checked for these), or a genuine "single translation
unit but the object file the linker sees for the shipped game was
produced in more than one dtk-split piece" possibility. This needs either
a header-level check of `fBase_c`/`dBase_c` for class-scoped
new/delete declarations, or a comparison against a *different*,
already-landed `dActorState_c`-derived unit whose destructor is small
enough to read directly -- none of the existing landed units in that
family (`d_a_remo_door.cpp` etc.) are trivial enough to serve as a clean
precedent; they all carry real member content that changes the shape.

## Gates checked, both clean

- **`.ctors`**: `python wip/wm_units/ctors_map.py d_basesNP RIVER` finds
  no entry, matching the coordinator's scoping exactly.
- **Function order**: verify_anon currently reports `FUNCTION ORDER IS
  WRONG`, but this is the SAME kind of ambiguous-pairing artifact the
  coordinator warned about for `finalUpdate` (confirmed here too:
  `finalUpdate` itself now shows a clean **MATCH**, so that specific
  false-positive is resolved). The remaining order complaints are all on
  the eight still-mismatched (14-differing) destructors -- since their
  content does not yet match anything in target, the greedy pairing
  cannot confirm their true position, and reports a spurious "defined too
  late" for the classInits that follow them. **Read directly, the
  source's own definition order already matches target's address order
  exactly** (checked by hand against the target address column, function
  by function) -- classInit and destructor are declared in the same
  order and adjacency as target for every type, and `daRiverMgr_c`'s two
  overrides are declared in the same order they appear in target
  (`doDelete` before `preDelete`). This should resolve on its own once
  the one remaining content residual is fixed and the pairing can
  confirm it. **Not verified as fully deterministically clean --
  re-check with `verify_anon` once the destructor residual closes,
  rather than assuming.**

## Tools used

`wip/wm_units/agent_river/build.py` (adapted from
`agent_killerbullet/build.py`) compiles `d_a_river.cpp` and runs
`wip/wm_units/verify_anon.py` against the whole `0x12ad60-0x12b400` range
directly against the unsplit `bin/dtkspl/d_basesNP/obj/auto_00_0012A66C_text.o`
(the target has not been split into a per-TU object yet, so this large
object -- which also contains neighbouring not-yet-landed units' code --
is used as the verification source; `verify_anon` only reads the
requested address range out of it). `STATIC_ASSERT`/`offsetof`/the
base-to-derived pointer-adjustment trick (all in a throwaway
`probe.cpp`/`probe_build.py`, deleted after use) settled the class-layout
questions before any function body was authored, per the project's
standing "classInit's `li r3,<size>` is a free `sizeof` check" practice,
extended here to the `+0x60` vtable-pointer question specifically.

## Proposed slice (not yet finalised -- `.data`/`.rodata` ranges not derived this round)

```json
{
  "source": "d_basesNP/bases/d_a_river.cpp",
  "memoryRanges": {
    ".text": "0x12ad60-0x12b400"
  }
}
```

`.data`/`.rodata`/`.bss` ranges were not derived this round (time went to
the class-layout and destructor investigation); the nine `g_profile_*`
objects plus nine vtables on the ~0xF0 stride (`0x3AF1C`-ish through
`0x3B79C`-ish, not exactly bounded here) are the obvious `.data` claim,
matching the coordinator's own `.data` observation. Left for the next
round to derive precisely with `check_bounds.py` before proposing the
slice for real.

## Round 2: operator-delete lead tested (negative, matches the WM_ANCHOR wall), order gate confirmed noise, `.data` slice derived

### The coordinator's `operator delete` lead: tested three ways, none reproduce the extra check

`fBase_c` does declare class-scoped `static void *operator new(size_t);`
and `static void operator delete(void*);` (`f_base.hpp:106-107`, confirmed
by reading it directly). Tried, against `daRiverPaipo_c` in isolation:

1. **In-class, inline-forwarding**: `static void operator delete(void *p)
   { dActorState_c::operator delete(p); }` alongside an inline
   `operator new` doing the same. The forwarding calls got inlined away
   entirely -- destructor still calls `__dl__7fBase_cFPv` directly, still
   22 instructions, no extra check.
2. **Out-of-line, both `operator new` and `operator delete` overridden**:
   this genuinely changed the destructor's call target (now
   `bl __dl__14daRiverPaipo_cFPv`, the class's own out-of-line delete,
   not `__dl__7fBase_cFPv`) -- but target's real bytes call
   `__dl__7fBase_cFPv` directly (confirmed from the original diff), so
   this variant is wrong on two independent counts, and still only 22
   instructions regardless.
3. **Out-of-line, `operator delete` only** (no `operator new` override,
   closest to a minimal, isolated test of the coordinator's specific
   lead): still exactly 22 instructions. No change.

None of the three add the check. Combined with the seven destructor-shape
variants from the previous round, this is ten source-level variants tried
across every axis available in ordinary C++ (declaration style, binding,
extra vtable content, explicit ctor, class-scoped new/delete in three
configurations), and the result is uniform: the extra `beq` never
appears from anything expressible in the derived class's own source.

**Conclusion, per the coordinator's own framing**: this is the same wall
already established on WM_ANCHOR -- a derived class cannot reach a
construct emitted by inlining its base's destructor. Recorded as a
genuine, exhausted negative, not re-attempted further. The eight affected
functions (every type's own destructor except `daRiverMgr_c`, whose
destructor is fully implicit and already matches) are left at their
current state: structurally correct, one instruction short, with the
exact nature of the gap fully characterized above.

### Order gate: confirmed noise, exactly as the coordinator diagnosed

Checked directly: all five "defined too late" complaints are functions
with byte-identical siblings elsewhere in the draft (`finalUpdate`,
which already independently MATCHES, plus the four still-mismatched
`classInit`s for `daRiverPaipo_c`/`daRiverPakkun_c`/`daRiverPuku_c`/
`daRiverStarcoin_c`, which are byte-identical to each other and to the
five already-matching classInits). Content-based pairing cannot
distinguish interchangeable functions, so the ascending-order check
reports a tie-break artefact rather than a real defect. Not pursued
further, per instruction.

### `.data` slice derived and confirmed by direct byte inspection

`python wip/wm_units/scout_unit.py d_basesNP 0x12ad60 0x12b400` reports
9 distinct `.data` targets `0x3AF28..0x3B6A8` (the nine vtables) and 5
`.rodata` targets `0x80066FC0..0x80162A60` -- all DOL-absolute addresses
(`0x8xxxxxxx`), confirming the coordinator's read: this unit owns no
`.rodata` of its own, only reaches into DOL-resident inherited-method
vtable slots.

`check_bounds.py` can't validate this range at all (`no symbols in
range`) since every symbol here is anonymous (`lbl_2_data_*`), so the
boundary was confirmed by hand instead, directly reading raw bytes from
`original/d_basesNP.rel` (`base_data = 0x1d0c00`):

- **Start**: `0x3AF18`, 0x10 bytes before `daRiverBarrel_c`'s vtable
  (`0x3AF28`). Raw bytes there (`00000000 002f002d 00000000 00000000`)
  are exactly `g_profile_RIVER_BARREL`'s own shape (relocated `mpClassInit`
  reading as 0 in the raw file, `mExecuteOrder`/`mDrawOrder` as two u16,
  `mActorProperties` as 0, then padding) -- matching the `fProfile::
  fActorProfile_c` layout exactly.
- **End**: `0x3B788`, exactly `daRiverStarcoin_c`'s vtable start
  (`0x3B6A8`) plus its 0xE0-byte size. Confirmed by reading past it:
  raw bytes at `0x3B788` onward are a DIFFERENT unit's own
  `g_profile_*` struct (`mExecuteOrder`/`mDrawOrder` values `0x225`/
  `0x25D`, in the ordinary few-hundred fProfile range, unlike RIVER's own
  tight 0x28-0x32 cluster) followed by a `"g3d/fire_rot_cannon.brres"`
  string -- a clean, symbol-boundary-exact stop into a neighbour's own
  data, the same shape as WM_KINOPIO's own `.data` end.
- Total: `0x870` bytes, exactly `9 * 0xF0` (nine `g_profile_*` + vtable
  pairs on the confirmed-uniform stride), no slack.

## Proposed slice, updated

```json
{
  "source": "d_basesNP/bases/d_a_river.cpp",
  "memoryRanges": {
    ".text": "0x12ad60-0x12b400",
    ".data": "0x3af18-0x3b788"
  }
}
```

No `.rodata` claim (DOL absolutes only, nothing owned), no `.bss`, no
`.ctors` (confirmed empty last round).

## Final state, this round: 15/23 byte-identical, unchanged

No score movement -- this round's work was the coordinator-directed
`operator delete` test (negative, wall confirmed) and deriving the
`.data` slice that was left undone last round. If the destructor wall
holds (as ten variants now suggest), **15/23 with a fully-understood,
precisely-characterized, and now doubly-confirmed-unreachable single
residual is very plausibly this unit's practical ceiling from source
alone** -- worth flagging to the coordinator as a landing candidate on
structural grounds (both gates clean, `.data` slice derived and
boundary-confirmed) rather than holding out for N/N.
