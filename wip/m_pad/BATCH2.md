# `m_pad.cpp` -- batch 2: `beginPad`

## Round 2 (coordinator follow-up) -- both proposed fixes tested, both fail, negative results below

The coordinator asked me to test two specific hypotheses after landing round 1.
Both were tested to hard numbers. Neither closes the gap. Full detail in the
two new subsections below ("Round 2: the destructor hypothesis" and
"Round 2: register-order variants") in their normal places in this doc; the
short version:

1. **A real local `PadAdditionalData_t` (the coordinator's exact suggested
   source) does NOT reproduce the target.** It compiles to **149 instructions**
   with the real (out-of-line, empty-bodied) ctor/dtor from Batch 3 present in
   the same file, or **152** with only the declarations visible (my original
   isolated setup) -- both far above the target's 121, because `-inline noauto`
   does not inline an out-of-line, non-`inline`-marked constructor/destructor
   even when its definition is textually present in the same translation
   unit. This produces two real `bl`s (`__ct__...`/`__dt__...`) that do not
   exist anywhere in the target's disassembly. **This is conclusive, not
   inconclusive** -- the mechanism as literally proposed cannot produce the
   target's instruction count, let alone its byte pattern. My `float[6]`
   placeholder from round 1 remains the closest working approximation (121
   instructions, wrong placement) despite not sharing the coordinator's
   destructor-based explanation for *why* the stores survive.
2. **Three more declaration-order permutations tried for the register map**
   (`pad, isConnected&, core`; `isConnected&, pad, core`; direct
   `g_IsConnected[i]` access with no named reference) -- none reproduce the
   target's `i=25, IsConn=26, pad=27, g_core=28, core=29` consecutive-ascending
   assignment. The best of all 6 orderings tried across both rounds remains
   `pad, core` (my round-1 draft), which gets `g_core` correct (`r28`) and
   permutes the other four. Every variant's exact register map is in the new
   "Round 2" subsection below, so nobody repeats these six.

The draft, shadow headers, and residual diff are otherwise unchanged from
round 1 -- current state is still 121/121 instruction count, 60/121 lines
differing (same two independent causes as before). Reporting this as I found
it rather than re-framing round 1 as closer to done than it is.

## Target vs mine

`beginPad__4mPadFv` at `0x8016F360`, size `0x1E4` = **121 instructions**. Asserted
before writing any C++ (`0x1E4 / 4 == 121`, confirmed again by `harness.extract`
against `scratch/gemini_round8/auto_03_8016F330_text.o.txt`).

My draft compiles to **121 instructions -- the count matches** -- but it is
**NOT byte-exact**. 60 of the 121 lines differ, and every one of those 60 is
one of exactly two things, both explained below and neither of which is a
logic error:

1. A **register-numbering permutation** among `r25..r29` (`_savegpr_25` /
   `_restgpr_25` is correct on both sides -- same 7 saved registers, same
   count, just a different assignment of which local gets which number).
2. The **stack placement of one write-only local** (frame `0x50` vs my `0xd0`,
   and consequently every frame-relative instruction).

Every branch target, every call, every load/store *address expression*
(module register renaming), every comparison, and the overall control-flow
shape are identical. This is reported as a genuine partial result, not a
match -- see "Open question" below for what would close it.

## Residual diff (all 60 lines, grouped)

### Group A -- prologue/epilogue frame size (11 lines)
Purely a consequence of Group C (the write-only local). Once C is resolved,
these resolve for free.
```
  0 | want: stwu r1, -0x50(r1)          got: stwu r1, -0xd0(r1)
  2 | want: stw r0, 0x54(r1)            got: stw r0, 0xd4(r1)
  3 | want: addi r11, r1, 0x40          got: addi r11, r1, 0xc0
  4 | want: stfd f31, 0x40(r1)          got: stfd f31, 0xc0(r1)
  5 | want: psq_st f31, 0x48(r1), 0, qr0 got: psq_st f31, 0xc8(r1), 0, qr0
113 | want: psq_l f31, 0x48(r1), 0, qr0 got: psq_l f31, 0xc8(r1), 0, qr0
114 | want: lfd f31, 0x40(r1)           got: lfd f31, 0xc0(r1)
115 | want: addi r11, r1, 0x40          got: addi r11, r1, 0xc0
117 | want: lwz r0, 0x54(r1)            got: lwz r0, 0xd4(r1)
119 | want: addi r1, r1, 0x50           got: addi r1, r1, 0xd0
```

### Group B -- register permutation (~35 lines)
Target: `i=r25, IsConnectedPtr=r26, padPtr=r27, g_corePtr=r28, core=r29,
const1=r30, const0=r31` -- a perfectly consecutive 25..29 assignment in that
order. Mine: `i=r26, IsConnectedPtr=r27, padPtr=r29, g_corePtr=r28, core=r25`
(constants match: `r30`/`r31` are right on both sides). `g_corePtr` lands on
the correct register (`r28`) in my best variant; the other four are a
permutation of the same five physical registers, not a different *count* --
every line in this group is a pure `rN`-for-`rM` substitution, same opcode,
same immediate, same offset.

### Group C -- the write-only local's placement (14 lines)
Same computed float values, same relative order, wrong stack address because
the local gets scalarized instead of packed -- see "Open question."

## Every variant tried

### The virtual call (`g_padMg->beginFrame()`, offset `+0x10`/slot `+0x8`)
Not something I had to solve myself: Batch 1 finished first and proved (byte-
exact, against `endPad`'s twin `+0xc` slot) that `EGG::CoreControllerMgr` needs
a `0x10`-byte non-polymorphic base before its own vtable, plus
`virtual beginFrame()` / `virtual endFrame()` in that order, plus a static
`sInstance`. I adopted that shape verbatim in my shadow header (see below) and
it reproduces instructions 11-14 exactly (`lwz r12,0x10(r3)` / `lwz
r12,0x8(r12)` / `mtctr` / `bctrl`). Neither batch has determined what the real
`0x10` bytes are; only size and non-polymorphic-ness are load-bearing.

### The interval-check placement -- a real logic bug I found and fixed
My first draft put the `s_GetWPADInfoInterval` / `getWPADInfoAsync` block
**after** the connected/not-connected `if`/`else`, unconditionally. That is
wrong. Re-reading the raw disassembly's labels directly (not my own
restated "want" column) shows the block sits **only inside the connected
branch**, reached by fallthrough from `bne`/`stb r30,...`, and both its exit
paths (`beq` on interval==0, or after the `bl getWPADInfoAsync`) jump straight
to `.L_8016F4CC` (loop increment) -- never through the not-connected block.
The not-connected branch (`.L_8016F48C`) does its own reset/clear/`stb r31`
and falls straight to the loop increment, with **no** interval check at all.
Moving the block inside the `if (connected)` branch fixed every structural
diff from instruction 58 onward (58-96 went from ~35 mismatches to 3, all
register-only). This is the single highest-value finding in this batch --
before the fix I had the wrong function shape entirely, despite a coincidentally-matching total instruction count.

### Register-order experiments (Group B)
Tried, in order, each measured against the same target:
1. `core` declared before `pad`, `pad` declared inside the `if` (my very
   first draft): `i=26,IsConn=27,pad=28,g_core=29,core=25`. `g_core` wrong.
2. `pad` (as `PadAdditionalData_t&`) hoisted above `core`, declared
   unconditionally at loop top: `i=26,IsConn=27,g_core=28,pad=29,core=25`.
   **`g_core` becomes correct (r28)**; kept this ordering as the base for
   everything after.
3. `core` re-declared before `pad` again (reverted #2): `g_core` reverts to
   r29 (wrong). Confirms #2's ordering, not something else, is what fixes
   `g_core`.
4. Removed the `pad` reference entirely, accessed
   `g_PadAdditionalData[i].m*` directly everywhere (no named local, hoping
   the compiler's own strength reduction would pick a different order):
   `g_core` reverts to r29 (wrong) -- same regression as #3. A named `pad`
   reference declared before `core` is necessary, not incidental.
5. Removing the `unexplainedTemp` array (Group C) entirely: **no effect** on
   the Group B register numbers at all (same permutation with or without
   it) -- the two groups are independent, confirmed empirically, not just
   assumed.

None of these reached the target's fully-consecutive `25..29` ordering for
`i, IsConnectedPtr, padPtr, g_corePtr, core`. I could not find a rule that
predicts it: it is not first-textual-reference order (in variant 2, `pad`'s
first *read* precedes `IsConnected`'s first read, yet `pad` gets the higher
number), not declaration order alone (variant 2 vs 3 shows the same two
variables swapping which gets r28 based on unrelated reordering), and not
use-count (`pad` is read/written far more than `g_core`, yet in every variant
either both or neither land where the count-based theory predicts). Flagging
as unresolved rather than asserting a rule I can't back up.

#### Round 2: the coordinator's exact target map, and three more orderings

The coordinator supplied the target's real prologue and the resulting map:
`r25=i, r26=&g_IsConnected, r27=&g_PadAdditionalData, r28=&g_core, r29=core,
r30=1, r31=0` -- a perfectly consecutive ascending assignment in that order.
Two things worth separating out before the new variants: `r30`/`r31` already
matched in every one of my round-1 drafts (both are `li` immediates hoisted
before the loop on both sides, confirmed again this round), and `r26` was
already an `li ...@sda21` immediate (not `lis`/`addi`) in every draft too --
so the two mechanical checks the coordinator suggested were both already
satisfied; the residual really is just *which* of r25-r29 each identity gets,
not a wrong addressing mode or a wrong hoisting decision. Three more
orderings, on top of the four from round 1:

8. **`bool &isConnected = g_IsConnected[i];` declared FIRST (before `pad`,
   before `core`)**, matching the map's `IsConnected` position (r26) coming
   right after `i` (r25): result `i=26, pad=28, g_core=27, IsConn=29,
   core=25`. Worse on every axis -- `g_core` regresses off r28 (my best-so-far
   value) and `IsConn` ends up highest, the opposite of the target.
9. **`pad` first, then `bool &isConnected = g_IsConnected[i];`, then `core`**
   (i.e. keeping round 1's best `pad`-first ordering but inserting the new
   reference between `pad` and `core` rather than before everything):
   `i=26, g_core=29, pad=27(!), IsConn=28, core=25`. `pad` actually lands on
   the *target's* register (`r27`) for the first time in any variant, but
   `g_core` regresses off r28 to fix it -- the two never land correctly
   together in the same draft.
10. **Direct `g_PadAdditionalData[i].m*` / `g_core[i]` access with no named
    references at all**, core declared first (round 1's variant 4, re-run
    after the interval-check fix to make sure the fix didn't change it): same
    result as before the fix, `g_core=29` (wrong) -- confirms a named `pad`
    reference declared before `core` is doing real work, not incidental to
    the interval-check bug.

Ten orderings tried in total across both rounds (7 in round 1 counting the
`unexplainedTemp`-removal control, 3 here). `g_core=r28` is the only one of
the five identities that has ever landed on the target's register, and only
in the `pad, core` ordering (round 1's #2, still the shipped draft). No
ordering has gotten more than that one right at the same time as the others.
I do not have a rule that predicts the target's map from source structure,
and I'd rather say that plainly than present a permutation search as
progress it isn't.

### The write-only local (Group C) -- every variant tried
The target writes the same 6 floats (`ddX, ddY, dX, dY, newX, newY`, in that
program order) to `r1+0x8` through `r1+0x1f`, in addition to the six `pad.m*`
stores. No instruction anywhere in the function ever reads those bytes back
(checked instruction-by-instruction across the whole `0x8016F360-0x8016F540`
range, not just the branch containing the writes). It is a real, load-bearing
part of the target's instruction count -- omitting it costs exactly 6
instructions (121 -> 115, confirmed) -- but its *purpose* is unknown.
1. **Plain scalar named floats only, no aggregate** (`float ddX = ...;` etc.,
   no extra local): compiles to 115 instructions. MWCC's dead-store
   elimination removes anything with no observable use, confirming the
   target's 6 extra stores need a genuine, non-eliminable object in the
   source, not just "the values happen to be computed."
2. **`PadAdditionalData_t` used as a stack local for the temp**: rejected on
   paper before compiling. Batch1 (and my own compile of the struct's
   `__ct`/`__dt` pair, function #14/#15 in the unit's table) shows the
   destructor is 0x40 bytes -- not empty -- so a real local of this type
   would need a `bl` to its destructor at scope exit. No such extra `bl`
   exists anywhere near the write site. Ruled out without spending a compile.
3. **Anonymous POD `struct { float accX,accY,velX,velY,posX,posY; } unused
   = {...};`** (declaration order chosen to match the target's low-to-high
   address contents): compiles to **121 instructions** (count matches!) but
   frame balloons to `0xd0` and the 6 floats land at scattered offsets
   (`0x28, 0x80, 0xc, 0x44, 0x60, 0x7c` -- not packed, not 8-aligned as a
   unit). MWCC is scalarizing the struct into 6 independent dead-but-kept
   values instead of treating it as one memory object, because nothing ever
   takes its address.
4. **`float unused[6] = {...}` (plain array, same values/order)**: **byte-
   identical result to #3** -- same instruction count, same scattered
   offsets. Struct-vs-array makes no difference; whatever is scalarizing it
   doesn't care about the aggregate's kind.
5. **Forcing address-taken via `(void)&unused;`**: no change whatsoever from
   #4. The optimizer still proves the pointer itself is unused and discards
   the "use."
6. **A real `Vec2`-by-value chain** (`struct Vec2 { float x,y; }`, a free
   `vsub(Vec2,Vec2)` returning by value, called three times for
   `newPos`/`dPos`/`dVel`, writing `pad.m*` via scalar field stores
   afterward): compiles to **159 instructions** -- far worse. PowerPC/EABI
   struct-by-value parameter and return conventions round-trip every Vec2
   through memory (visible as `lwz`/`stw` word copies between stack slots
   interleaved with the float ops), which the target does not do at all
   (target has zero integer loads/stores in this region). Conclusively the
   wrong mechanism, not merely unoptimized.
7. **The same `Vec2` idea but assigning whole structs to `pad.mPos`/`mVel`
   instead of scalar fields** (an earlier, cruder attempt before #6): **143
   instructions**, same class of bloat (`lwz`/`stw` word copies for the
   struct assignment). Confirms the bloat source is struct-by-value ABI
   traffic, not the scalar-field-store choice.

#### Round 2: the coordinator's destructor hypothesis -- tested, conclusively fails

The coordinator's theory: `PadAdditionalData_t` has a user-declared
destructor (confirmed by Batch3: real, out-of-line, empty `{ }` body, `0x40`
bytes compiled), so a genuine local instance can't have its storage elided
even though every store into it is dead -- and suggested exactly this source,
with the field-write order reordered to match the target's address pattern:
```cpp
PadAdditionalData_t t;
t.mAccX = ddX;  t.mAccY = ddY;
t.mVelX = dX;   t.mVelY = dY;
t.mPosX = newX; t.mPosY = newY;
pad = t;
```
Tested exactly as given, in two configurations:
8. **With only `PadAdditionalData_t`'s ctor/dtor *declared* in the shadow
   header (my batch's normal setup -- another batch owns the bodies)**:
   compiles to **152 instructions**. Two real calls appear that do not exist
   anywhere in the target: `bl __ct__Q24mPad19PadAdditionalData_tFv` right
   before the field stores, and `bl __dt__Q24mPad19PadAdditionalData_tFv`
   right after `pad = t`'s stores, with an extra `li r4,-0x1` feeding it (the
   `__dt` calling convention apparently takes a flag arg). The `pad = t`
   assignment itself did NOT become the hoped-for "free" re-store of
   already-live registers -- it also round-trips, and six extra callee-saved
   FPRs (`f25`-`f30`) get spilled/restored in the prologue/epilogue that
   don't exist in the target at all, because the values now have to survive
   across the two calls.
9. **With the real ctor/dtor *bodies* from Batch3 also defined in the same
   file** (`PadAdditionalData_t::PadAdditionalData_t() { }` /
   `::~PadAdditionalData_t() { }`, copied verbatim from `BATCH3.md`, to test
   whether `-ipa file` lets MWCC inline them away since their definitions are
   now visible in the same translation unit): compiles to **149
   instructions** -- still two real `bl`s to the ctor/dtor, unchanged from
   #8 apart from 3 fewer instructions elsewhere. **`-inline noauto` blocks
   inlining a non-`inline`-marked, out-of-line member function regardless of
   whether its body is visible in the same file** -- `-ipa file` does not
   override that. (I removed these two definitions again afterward; they are
   not mine to define, and Batch3 already owns them in the real merge.)

Both configurations are far above the target's 121 and contain two `bl`
instructions with no counterpart anywhere in the target's disassembly. This
is not "didn't quite match" -- it is a different, larger set of instructions
in a fundamentally different shape (real calls where the target has none), so
I'm treating it as a closed negative rather than a partial lead: **a literal
local instance of `PadAdditionalData_t`, constructed and assigned as
suggested, cannot produce the target's 121-instruction body under this
project's compiler flags.** Whatever mechanism keeps the 6 dead stores alive
in the real source, it is not "a local of this exact class, this exact way."
My round-1 `float[6]` remains the closest working substitute -- right count,
wrong placement -- without sharing the destructor-based explanation for why.

**Open question, stated plainly**: I can reproduce the *quantity* and
*values* of the extra stores (variant 4, 121 instructions total) but not
their *placement*. Getting the placement right needs a source shape that
forces MWCC to treat the 6 floats as one non-scalarizable memory object
without also forcing an ABI-level struct-by-value round trip (which
overshoots badly, per #6/#7). I don't have that shape. What would settle it:
someone with a way to see MWCC's actual scalarization/packing heuristic (or
a reference decomp with the identical pattern) rather than more blind
variants -- I already covered POD-struct, POD-array, address-forced, and
by-value-class, which is the space I could think to search.

## Header/global findings for the lead

All **proposals**, not landed; shadow-copied only, per the standing rules.

### `PadAdditionalData_t` -- naming correction to Batch1's proposal
Batch1 (without `beginPad` in scope) proposed `mCurX/mCurY/mPrevX/mPrevY/
mDeltaX/mDeltaY` from offsets alone. `beginPad`'s actual data flow disproves
the "prev" and "delta" semantics:
- offset `0x0/0x4`: the **new** sample read straight from the controller
  (`core+0x6c`/`core+0x70`), stored unconditionally every connected frame.
- offset `0x8/0xc`: `new - old(0x0/0x4)` -- a **velocity** (rate of change of
  the position sample), not a second raw sample.
- offset `0x10/0x14`: `velocity - old(0x8/0xc)` -- an **acceleration** (rate
  of change of the velocity), not a "delta" of the position.

I've proposed `mPosX/mPosY/mVelX/mVelY/mAccX/mAccY` in my shadow header
instead. Offsets `0x0/0x4/0x8/0xc/0x10/0x14` (hence `sizeof == 0x18`) are
unchanged and solid either way -- this is a naming-only correction, **not
offset-perturbing**.

Both the "connected" update and the "not connected" reset write these three
pairs in **pos, then acc, then vel** order (not declaration/address order)
-- confirmed independently in two different places in the function (the
live update at instructions 38/41/47/48/49/55 and the reset at 82/84-88).
That consistency across two unrelated code paths is itself evidence this is
a real, intentional field-write order in the original source (e.g. a
`reset()`-style helper that touches its members out of declaration order),
not a compiler artifact -- but I have not determined *why* that specific
order, only that both paths agree on it.

### `core+0x6c` / `core+0x70` / `core+0xb1c` / `core+0x18`
Read via raw offset casts, not named members, per AGENT_CONTEXT's rule for a
"genuinely not decompiled" class -- `EGG::CoreController` is a 12-virtual/
1-member stub, nowhere near sized to `0xb1c+`. Not proposing member names for
these; `0xb1c` bit 0 is "connected", `0x6c`/`0x70` are a 2-float pointer
position (X then Y, in read order), `0x18` is the start of an embedded
`EGG::CoreStatus` (confirmed by the direct non-virtual `bl
init__Q23EGG10CoreStatusFv` with `this = core+0x18`, no vtable indirection).
None of this is enough evidence to propose a member layout for
`CoreController` -- flagging the offsets as facts, not proposing a struct.

### `s_GetWPADInfoInterval` type
Batch1 already established `ulong` from the setter's mangled parameter. My
compile confirms the SAME type also produces the correct unsigned
`cmplwi`/`bgt` codegen for the `interval <= 3` comparison inside `beginPad`
(instructions 66-67) -- corroborating evidence for an already-made call, not
a new finding.

### `EGG::CoreControllerMgr::getNthController`
Confirmed non-static instance method (called as `g_padMg->getNthController(i)`,
`this` in `r3`, `i` in `r4`, real `bl` to
`getNthController__Q23EGG17CoreControllerMgrFi` -- matches instructions
24-26 exactly on both sides). Added to my shadow `eggController.h`; not in
Batch1's version since their functions didn't call it.

### `EGG::CoreController::sceneReset()` and `EGG::CoreStatus::init()`
Both non-virtual (direct `bl`, no vtable load before the call), confirmed by
instructions 78-79 and 80-81 matching exactly. Added declarations to the
shadow `eggController.h`.

### The float constant is confirmed `0.0f`
Read directly out of `bin/wiimj2d.dol`'s `.sdata2` at `0x8042E010` (DOL
section 7, file offset `0x352c50`): `struct.unpack('>f', ...)` gives exactly
`0.0`. This is the value `beginPad` loads into `f31` and stores into all 6
`pad.m*` fields on disconnect. Matches the brief's note that this constant
belongs to `m_mtx.cpp`, not to `m_pad.cpp` -- I did not add any `.sdata2`
declaration of my own; writing the literal `0.0f` six times in source is
enough for MWCC to pool it into one `lfs`, matching the single reference
(canonicalised as `SYM0` on both sides, instruction 17).

## Compiled: YES, count-exact, not byte-exact

`tools/auto_decomp/harness.py`'s `compile_draft` / `disasm` / `extract` /
`diff_fn`, called directly from a driver script (not the CLI, since I needed
to diff one address-identified function out of a larger split-object target
file rather than a single-function target). Extracted **by symbol name**
(`beginPad__4mPadFv`), which is unambiguous in this target file (checked --
only one function normalises to that name), and cross-checked its starting
address/size against `bin/dtk/wiimj2d_symbols.txt` before writing any C++, as
instructed. Driver: `wip/m_pad/scratch/batch2/fulldiff.py` (prints every
line, unlike the harness's own 40-line-truncated report).

Never ran `ninja`, `configure.py`, `progress.py`, or `land.py`. Never edited
`slices/wiimj2d.json`, `syms.txt`, or the real
`include/game/mLib/m_pad.hpp`/`include/lib/egg/core/eggController.h` --
shadow copies only, in `wip/m_pad/scratch/batch2/include/`.

## The source

`wip/m_pad/scratch/batch2/m_pad.cpp`:

```cpp
#include <game/mLib/m_pad.hpp>

namespace mPad {

// --- data (declared for compile purposes only; another batch owns the real
// definitions and __sinit/ctor/dtor bookkeeping) ---
EGG::CoreController *g_currentCore;
CH_e g_currentCoreID;
EGG::CoreController *g_core[4];

EGG::CoreControllerMgr *g_padMg;
u32 g_PadFrame;
bool g_IsConnected[4];
ulong s_GetWPADInfoInterval;
u32 s_GetWPADInfoCount;
PadAdditionalData_t g_PadAdditionalData[4];

void beginPad() {
    g_PadFrame++;
    g_padMg->beginFrame();

    for (int i = 0; i < 4; i++) {
        PadAdditionalData_t &pad = g_PadAdditionalData[i];
        EGG::CoreController *core = g_padMg->getNthController(i);
        g_core[i] = core;

        if (*((u8 *)core + 0xb1c) & 1) {
            float newX = *(float *)((u8 *)core + 0x6c);
            float newY = *(float *)((u8 *)core + 0x70);
            float dX = newX - pad.mPosX;
            float dY = newY - pad.mPosY;
            pad.mPosX = newX;
            pad.mPosY = newY;
            float ddX = dX - pad.mVelX;
            float ddY = dY - pad.mVelY;
            // @unofficial UNEXPLAINED. The target writes these same 6 values to
            // r1+0x8..0x1F (never read back anywhere in the function) in
            // addition to the pad.m* stores below. This array reproduces the
            // instruction COUNT (dead-store retention -- MWCC does not DCE
            // this) but not the exact stack OFFSETS or frame size; see
            // BATCH2.md "Open question" for every variant tried.
            float unexplainedTemp[6] = { ddX, ddY, dX, dY, newX, newY };
            pad.mAccX = ddX;
            pad.mAccY = ddY;
            pad.mVelX = dX;
            pad.mVelY = dY;

            if (!g_IsConnected[i])
                g_IsConnected[i] = true;

            if (s_GetWPADInfoInterval != 0) {
                if (s_GetWPADInfoInterval == 1 || s_GetWPADInfoCount == (u32)i ||
                    (s_GetWPADInfoInterval <= 3 &&
                     (s_GetWPADInfoCount & 1) == ((u32)i & 1))) {
                    getWPADInfoAsync((CH_e)i);
                }
            }
        } else {
            if (g_IsConnected[i]) {
                ((EGG::CoreStatus *)((u8 *)core + 0x18))->init();
                core->sceneReset();
                pad.mPosX = 0.0f;
                pad.mPosY = 0.0f;
                pad.mAccX = 0.0f;
                pad.mAccY = 0.0f;
                pad.mVelX = 0.0f;
                pad.mVelY = 0.0f;
                clearWPADInfo((CH_e)i);
                g_IsConnected[i] = false;
            }
        }
    }

    if (s_GetWPADInfoInterval != 0) {
        if (++s_GetWPADInfoCount > s_GetWPADInfoInterval)
            s_GetWPADInfoCount = 0;
    }

    g_currentCore = g_core[g_currentCoreID];
}

} // namespace mPad
```

## Shadow headers used

`wip/m_pad/scratch/batch2/include/game/mLib/m_pad.hpp` (my proposal, layered
on Batch1's -- adopts their `CH_e setCurrentChannel`/`g_currentCoreID : CH_e`/
ctor-dtor-bearing `PadAdditionalData_t` shape, renames the struct's members
per the finding above):

```cpp
#pragma once

#include <types.h>
#include <revolution/WPAD/WPAD.h>
#include <lib/egg/core/eggController.h>

namespace mPad {
    enum CH_e {
        MPAD_CH_0,
        MPAD_CH_1,
        MPAD_CH_2,
        MPAD_CH_3
    };

    // Batch1 proposed mCurX/mCurY/mPrevX/mPrevY/mDeltaX/mDeltaY from offsets
    // alone (beginPad wasn't in their scope). beginPad's actual data flow
    // shows offsets 0x8/0xc are NOT a raw "previous" sample and 0x10/0x14 are
    // NOT a plain "delta" -- see above for the derivation. Renamed here to
    // match the derived semantics (position / velocity / acceleration);
    // offsets 0x0/0x4/0x8/0xc/0x10/0x14 are solid either way.
    struct PadAdditionalData_t {
        PadAdditionalData_t();
        ~PadAdditionalData_t();

        f32 mPosX; // 0x0
        f32 mPosY; // 0x4
        f32 mVelX; // 0x8
        f32 mVelY; // 0xc
        f32 mAccX; // 0x10
        f32 mAccY; // 0x14
    };
    STATIC_ASSERT(sizeof(PadAdditionalData_t) == 0x18);

    void create();
    void beginPad();
    void endPad();
    CH_e setCurrentChannel(CH_e ch);
    s32 getBatteryLevel_ch(CH_e ch);
    void setWPADInfo(CH_e ch, const WPADInfo &info);
    void clearWPADInfo(CH_e ch);
    void initWPADInfo();
    void getWPADInfoAsync(CH_e ch);
    void setGetWPADInfoInterval(ulong interval);
    ulong getGetWPADInfoInterval();

    extern EGG::CoreController *g_currentCore;
    extern CH_e g_currentCoreID;
    extern EGG::CoreController *g_core[4];
};
```

`wip/m_pad/scratch/batch2/include/lib/egg/core/eggController.h` (Batch1's
proven `CoreControllerMgr` shape, plus `getNthController`/`sceneReset`/
`CoreStatus::init` which their functions didn't need):

```cpp
#pragma once

#include <types.h>
#include <revolution/PAD.h>
#include <revolution/KPAD.h>
#include <revolution/WPAD.h>

namespace EGG {

class CoreStatus {
public:
    void init();
};

class CoreController {
public:
    virtual void setPosParam(float a, float b) { KPADSetPosParam(mNum, a, b); }
    virtual void setHoriParam(float, float);
    virtual void setDistParam(float, float);
    virtual void setAccParam(float, float);
    virtual bool down(ulong) const;
    virtual bool up(ulong) const;
    virtual bool downTrigger(ulong) const;
    virtual bool upTrigger(ulong) const;
    virtual bool downAll(ulong) const;
    virtual bool upAll(ulong) const;
    virtual void beginFrame(PADStatus *);
    virtual void endFrame();

    void startPatternRumble(const char *, int, bool);
    int getDpdNumMarks() const;

    void sceneReset();

    int mNum;
};

// TEST HYPOTHESIS (proven byte-exact by batch1 against endPad, and reused
// here for beginPad's identical this+0x10 / slot pattern): CoreControllerMgr's
// own vtable sits at offset 0x10 because a non-polymorphic base of that size
// precedes it. The true shape of those 0x10 bytes is NOT known -- only the
// size and non-polymorphic-ness are load-bearing. @unofficial
class CoreControllerMgrTestBase {
    u8 mPad0x10[0x10];
};

class CoreControllerMgr : public CoreControllerMgrTestBase {
public:
    static void createInstance();
    static CoreControllerMgr *sInstance;
    static u32 sWPADWorkSize;

    CoreController *getNthController(int idx);

    virtual void beginFrame(); // slot 0, offset+0x8 -- called by beginPad
    virtual void endFrame();   // slot 1, offset+0xc -- called by endPad
};

} // namespace EGG
```

## Status

**Not byte-exact, after two rounds.** 61/121 instructions match exactly
(including every branch target, every call, and the full control-flow
shape); the remaining 60 are a register-numbering permutation plus one
write-only local's stack placement, both isolated and explained above rather
than left as an unexplained blob. Round 2 tested both of the coordinator's
specific hypotheses to hard numbers -- the destructor-backed local
(149-152 instructions, two spurious `bl`s, conclusively wrong) and three more
declaration orderings for the register map (none improve on round 1's best,
`g_core` correct and the other four permuted) -- and neither closes the gap.
Ten total register-order variants and nine total local-placement variants
have been tried and recorded across both rounds so nobody repeats them.
Reporting the negative result rather than manufacturing a match.

Work is in `wip/m_pad/scratch/batch2/` (`include/` = shadow headers,
`m_pad.cpp` = draft, `fulldiff.py`/`check.py` = the compile+diff drivers,
importing `harness.py` directly). The shipped draft is unchanged from round 1
-- `pad, core` declaration order, `float unexplainedTemp[6]` placeholder for
the write-only local -- since none of round 2's variants improved on it.
