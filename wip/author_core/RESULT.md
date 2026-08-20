# author_core -- RESULT.md

Scope: lifecycle core, circular-movement family, and unnamed file-scope
helpers of `dLineMng_c` (`d_line_mng.cpp`, `wiimj2d.dol`). Compiled standalone
via `tools/auto_decomp/harness.py` against `wip/line_mng_shared/target.txt`,
with `-i wip/author_core/local_shadow -i wip/line_mng_shared/shadow_include`
(local overrides checked first). Draft: `wip/author_core/d_line_mng.cpp`.
Test driver: `wip/author_core/try.py`.

**Never touched**: `source/`, `include/`, `syms.txt`, `slices/*.json`,
`wip/line_mng_shared/` (read only, refreshed once mid-round after the
coordinator's header update -- see below), any other agent's directory.
Everything proposed below is local-only (`wip/author_core/local_shadow/`) and
needs the lead's review before merge.

## Headline numbers (measured)

| function | target words | draft words | differing | status |
|---|---:|---:|---:|---|
| `init__10dLineMng_cFRC7mVec2_cfiUc` | 70 | 70 | 0 | **MATCH** |
| `move__10dLineMng_cFv` | 21 | 21 | 0 | **MATCH** |
| `SetPos__10dLineMng_cFRC7mVec2_c` | 5 | 5 | 0 | **MATCH** |
| `SetBaseSpeed__10dLineMng_cFf` | 6 | 6 | 0 | **MATCH** |
| `change_dir__10dLineMng_cFv` | 7 | 7 | 0 | **MATCH** |
| `getLineUnitNo__10dLineMng_cFff` | 26 | 26 | 0 | **MATCH** |
| `is_unit_circle2x2__10dLineMng_cFUl` | 14 | 14 | 0 | **MATCH** |
| `is_unit_circle4x4__10dLineMng_cFUl` | 14 | 14 | 0 | **MATCH** |
| `CalcAdjustPosY__10dLineMng_cFff` | 128 | 128 | 10 | length-complete, register permutation only |
| `move_on_circle_speedset__10dLineMng_cFff` | 75 | 75 | 14 | length-complete, register permutation only |
| `move_on_circle1__10dLineMng_cFff` | 72 | 72 | 18 | length-complete, register permutation only |
| `move_on_circle2__10dLineMng_cFff` | 79 | 79 | 30 | length-complete, register permutation only |
| `move_on_circle3__10dLineMng_cFff` | 74 | 74 | 19 | length-complete, register permutation only |
| `move_on_circle4__10dLineMng_cFff` | 77 | 77 | 29 | length-complete, register permutation only |
| `start_line_move__10dLineMng_cFv` | 94 | 95 | 77* | 1 word over, real residual |
| `check_term__10dLineMng_cFv` | 73 | 72 | 59* | 1 word short, real residual |
| `init_term_ck_pos__10dLineMng_cFv` | 37 | 39 | 38* | 2 words over, real residual |
| `fn_800C15B0` (unnamed) | 7 | 7 | 0 | **MATCH** (as `setArrElem_800C15B0`) |
| `fn_800C3B20` (unnamed) | 15 | 15 | 8 | length-complete, register/branch-form residual |
| `fn_800C3B60` (unnamed) | 15 | 16 | 12 | 1 word over, real residual |

\* Per AGENT_CONTEXT.md's warning, a length mismatch means the differing
count past the first divergence is not meaningful (offset cascades) -- I
report it but the SIZE gap is the real signal, not the count.

**8 of 17 named functions are byte-exact matches.** All 6 remaining named
functions in my assignment (`CalcAdjustPosY`, the whole `move_on_circle*`
family) are **length-complete** -- same instruction count as the target, only
differing by which physical register (e.g. f29 vs f30) holds a cross-call
local. Per AGENT_CONTEXT.md ("Declaration order does NOT drive MWCC's saved
register assignment... pure register-permutation residual is not source
addressable"), I spent bounded effort trying to close these via declaration
reordering and stopped once that rule's prediction held -- further churn
looked like it would cost tokens without new information.

Not attempted this round: `fn_800C1EE0` (depends on the `line_cross_chk1`
signature, owned by another agent's family -- see "Left open" below) and
`fn_800C31C0` (549 words, deliberately saved for last per the brief and not
reached in this round's budget).

## Header correction found mid-round, applied to my local copy

**`check_term` was declared `void` in the shared header with no unproven-type
flag. It should be `bool`.** Not one of the two the brief named -- found while
authoring it. Evidence: the body explicitly sets `r3` to `0x1` or `0x0` at
BOTH exit points, immediately before the epilogue (`li r3,0x1`/`li r3,0x0`).
This is the same "declared bool -> MWCC reserves r3" signature AGENT_CONTEXT.md
documents, and it is a *deliberate* two-way return value, not incidental
register reuse (nothing else uses r3 there). I declared it `bool` in my local
header override and it compiles/matches on that basis (72 vs target's 73,
residual described below is unrelated to the return type).

## The two flagged unproven return types

### `getLineUnitNo(f32, f32)` -- proven `u32` (or plain `int`), **not** `u8`

The coordinator's mid-round header update independently confirmed this is
non-void (every `mov_to_*`/`mov_frm_*` caller does `mr r31,r3` right after the
`bl`). My own contribution is nailing the **width**, which the coordinator
explicitly left open:

- I first assumed `u8`, reasoning from the callee's own body
  (`clrlwi r31,r3,24` before returning). **This was wrong**, caught by testing
  a caller, exactly as AGENT_CONTEXT.md prescribes ("do not argue a return
  type from the disassembly -- test it").
- Declaring it `u8` makes `check_term`'s call site (`getLineUnitNo(...)==0x22`)
  emit an **extra** `clrlwi r0,r3,24` before the comparison that the target
  does not have -- measured: `check_term` went from 73 to 74 words with the
  `u8` declaration, and the one extra instruction is exactly that mask.
  `start_line_move`'s and `CalcAdjustPosY`-adjacent call sites showed the same
  extra instruction.
- Declaring it `u32` removes the extra mask at every call site and reproduces
  the target exactly at those call sites (the `clrlwi` inside
  `getLineUnitNo`'s own body is still present either way -- it belongs to an
  internal `u8 result` local, not the return type; a u8-typed local widened
  back to `u32`/`int` on return needs no further instruction since it is
  already a clean 32-bit value in the register).
- `getLineUnitNo` itself, compiled standalone, is a **26/26 byte-exact match**
  under the `u32` declaration.

This is a genuine correction to record: the internal-masking signal that
looks like return-type evidence is not reliable on its own; the call-site
test is what settles it, and it settled it in the *opposite* direction from
my first guess.

### `CalcAdjustPosY(f32, f32)` -- proven `f32`, with an important caveat

**There is no call site for this function anywhere in `d_line_mng.cpp`** (I
grepped the whole `target.txt` for `bl CalcAdjustPosY` -- zero hits). So the
project's primary test (read the caller's register use) is **unavailable**
for this one, and I want to be explicit that what follows is the
next-best evidence, not the gold-standard test.

Evidence for `f32`, not `void`:
- Both exit paths of the compiled function set `f1` (the float return
  register) to the value that would be the natural "current Y position"
  right before the epilogue, with **no further use of `f1`** afterward.
- If the function were genuinely `void`, `f1` is not part of the ABI's return
  contract and nothing reads it after the call returns (nothing in this TU
  calls it at all) -- `-O4` dead-code elimination has no reason to keep a
  useless final `fmr f1, ...` and no reason to preserve `f28`/`f31` etc.
  across the loop bodies just to feed that dead move.
- Empirically: declaring it `f32` and writing the natural
  `if (close) return y; ... return y;` structure (two return points, not
  merged) reproduces the target's exact length (128/128) including the two
  separate `fmr f1,f28`-then-epilogue sequences at both exit points. An
  earlier draft that used a single shared `return y;` (reachable via
  fallthrough from both paths) came out 126/128 -- 2 words short, because
  MWCC merged the two logical returns into one shared tail where the target
  does not. That structural test is what proves the two returns are written
  as **separate statements** in the source, not merely that the return type
  is non-void, but it also corroborates non-void: a void function has no
  reason to carry a value in `f1` across either exit path at all.
- **Caveat, stated plainly**: this is the strongest test available *within
  this file*, but it is not the caller-register-read test AGENT_CONTEXT.md
  calls the reliable one. If `CalcAdjustPosY` is called from the (currently
  undecompiled) owning class's TU, that call site would be the real proof
  and could in principle contradict this. I'm reporting `f32` as my best
  finding, not as settled beyond doubt.

## Levers that worked (new to this unit, worth recording)

1. **`mVec2_c::set(x, y)` triggers right-to-left argument evaluation;
   direct field assignment (`v.x = a; v.y = b;`) does not.** This is the
   single highest-value finding of the round -- it closed `init()`
   (71→70 words, fmuls→fdivs correction) the moment I switched from
   `mPos.set(pos.x, pos.y)` to `mPos.x = pos.x; mPos.y = pos.y;`. Applies
   throughout: every `mVec2_c` field-copy in my functions uses direct
   assignment for this reason.
2. **A `static const float` class member with its *initializer visible in
   the same TU* gets constant-folded into a multiply-by-reciprocal by `-O4`,
   even for a genuine runtime division against a variable.** `init()`'s
   `pos.x / smc_UNIT_SIZE_X` compiled to `fmuls` with zero `fdivs` in the
   whole function when I defined `smc_UNIT_SIZE_X = 16.0f` in this TU; the
   target has two real `fdivs`. Declaring it and leaving it **undefined** in
   this TU (matching "its real definition lives in a different,
   undecompiled TU") restored the two `fdivs` exactly. This is now also
   validated independently: `smc_UNIT_SIZE_X`'s value (16.0f) came from
   reading raw bytes at `.sdata2:0x8042CB18` in `original/wiimj2d.dol`
   directly, not by inference.
3. **A `do`-`while (p != end)` loop (not `<`) is needed to stop MWCC from
   converting a short, unconditional, provably-fixed-trip-count loop into a
   counted (`mtctr`/`bdnz`) loop.** `init_term_ck_pos`'s two array-fill loops
   both flipped from a `mtctr`-based form to the target's plain
   compare-and-branch form purely by changing `<` to `!=` in the loop
   condition, with no other change. This is a new, project-worthy lever --
   I did not find it named in AGENT_CONTEXT.md's existing lever list.
4. **`!(a < b)` as an early-return guard, with the return written as a
   *separate, explicit statement* rather than folded into a shared tail via
   `if (!cond) { ...; return y; }`, changes the compiled control-flow
   shape.** Confirmed on `CalcAdjustPosY` (see above) -- this is a
   generalisation of AGENT_CONTEXT's lever #7 (branch polarity) combined
   with a new observation about explicit vs. implicit return merging.
5. **`static mVec2_POD_c` (no user constructor) vs `static mVec2_c` (has
   one) for a function-local static.** Using the real class
   (`mVec2_c`, which has a non-trivial, never-inlined-out-of-line
   constructor per the project's established rule) for
   `init_term_ck_pos`'s lazily-initialized direction table pulled in
   `__construct_array`/`__register_global_object`/`__arraydtor` machinery
   the target does not have at all. Switching to `mVec2_POD_c` (the
   plain-old-data sibling class declared in `m_vec.hpp` specifically for
   this purpose) removed all of it. The guard bool for this lazy init is a
   **hand-written `static u8 s_init = 0;` with an explicit `if`**, not
   MWCC's auto-generated static-local guard -- confirmed necessary because
   the auto-generated guard's symbol name/placement didn't match, and this
   TU's construction machinery is absent from the target entirely.

## A structural class-layout correction, found while authoring (not merged into the shared header -- reporting only)

**Offset `0x6a` is a real one-byte field, not padding.** The previous round's
`MAPPING.md` recorded `0x6a`-`0x6c` as "2 bytes of implicit padding... not a
named member," reasoned from `init()` never touching it. That reasoning was
correct as far as it went, but `init()` isn't the only function that touches
the object:

- `start_line_move()`: `stb r30, 0x6a(r29)` with `r30=0`, unconditionally,
  early in the function.
- `check_term()`: `stb r0, 0x6a(r28)` with `r0=1`, on the "found it" return
  path.

Both are real, deliberate byte writes to `this+0x6a`, not stack spills (both
use the object-pointer register as the base, not `r1`). I added
`u8 mUnk6a;` to my **local** header copy (documented `@unofficial`, not
merged) and only `0x6b` remains true alignment padding for `mStateMgr`'s
vtable pointer. This is a genuine finding for the lead to fold into the
shared `MAPPING.md`/header -- I did not touch the shared file myself, per the
rules.

## Assumed helper signatures (declare-and-let-unresolved, per the brief)

These are functions/classes I call or forward-declare that other agents own,
listed so the merge can check them:

| symbol | assumed signature | why |
|---|---|---|
| `dBc_c::getUnitType` | `static u32 getUnitType(float, float, u8)` | copied verbatim from `include/game/bases/d_bc.hpp:217` (already-established, not my guess) |
| `dBc_c::getUnitKind` | `static u32 getUnitKind(float, float, u8)` | same, `d_bc.hpp:218` |
| `dLineMng_c::mov_frm_rightlower/leftlower/rightupper/leftupper` | `void (const mVec2_c&, bool)` | taken from the shared header (already declared there; not guessed by me), called with a locally-constructed `mVec2_c` or `mUnitBasePos` and a literal `true`/`false` per branch, matching the target's `r5` argument exactly |
| `sStateStateMgr_c::getStateID()->isEqual(const sStateIDIf_c&)` | from `include/game/sLib/s_StateInterfaces.hpp`/`s_StateStateMgr.hpp` | confirmed by vtable slot arithmetic: `getStateID` is slot 8 (offset `0x28`) on `sStateMgrIf_c`, and the returned pointer's slot 2 (offset `0x10`) is `isEqual(const sStateIDIf_c&)` per `sStateIDIf_c`'s declared virtual order (dtor=0, isNull=1, isEqual=2, ...) -- this is a correction of my own first guess, which was `isSameName(const char*)` before I checked the real header and found `isSameName` takes a `const char*`, incompatible with passing `StateID_FallDown` by reference |
| `nw4r::math::SinIdx(short)` / `CosIdx(short)` | from `include/lib/nw4r/math/math_triangular.h` | matches the observed `psq_l`-based u16-to-float conversion (`U16ToF32`) plus a `1/256` scale (`0.00390625`, read directly from `.sdata2:0x8042CB80` in the DOL and confirmed to match `NW4R_MATH_IDX_TO_FIDX`'s constant) feeding `SinFIdx`/`CosFIdx` |

`d_bc.hpp` itself could not be `#include`d directly -- it pulls in
`d_bg_ctr.hpp`/`d_actor.hpp`, which fail to compile standalone in isolation
(`undefined dBg_ctr_c`, `illegal use of 'virtual'` outside a class). This
looks like a pre-existing gap in that header chain when compiled outside the
full project context, not something introduced by me. I used a minimal
forward declaration of `dBc_c` with just the two static methods instead.

## Proposed header additions (not applied -- shared header is read-only to me)

```cpp
// In dLineMng_c, public section (needed by init(), check_term(), start_line_move()):
static const float smc_UNIT_SIZE_X;   // 16.0f, read from .sdata2:0x8042CB18.
                                        // Do NOT give it an initializer in
                                        // d_line_mng.cpp -- see lever #2 above.

// In dLineMng_c, private section, replacing the "0x6a-0x6c: 2 bytes of
// implicit padding" comment:
u8 mUnk6a;   // 0x6a -- see "structural class-layout correction" above.
             // 0x6b remains true padding.

// Return type correction (not one of the two originally flagged):
bool check_term();   // was `void check_term();`
```

`CalcAdjustPosY`'s return type (`f32`, not `void`) and `getLineUnitNo`'s width
(`u32`, not `u8`) are already reflected in the coordinator's live shared
header as of this round; I'm not proposing a second change for those, just
recording the proof above.

## Left open

- **`fn_800C1EE0`** (0x90 bytes): calls
  `line_cross_chk1__10dLineMng_cFffRC7mVec2_c7mVec2_c7mVec2_cR7mVec2_c`, a
  signature belonging to the `*_cross_chk` family another agent owns, and
  writes to `this+0x40/0x44/0x50/0x54` (`mPos`/`mUnitBasePos`) through a
  first parameter that is itself a `dLineMng_c*` distinct from the object
  `line_cross_chk1` is called on -- i.e. it takes at least two
  `dLineMng_c`-related pointers, not the single-`self` shape of the other
  unnamed helpers. Given the dependency on an unlanded family and the
  remaining time budget, I did not author it this round. Whoever picks it up
  should read `line_cross_chk1`'s real signature first once it lands, rather
  than guessing it here.
- **`fn_800C31C0`** (549 words): not reached, per the brief's own guidance to
  leave it for last.
- **`fn_800C3BA0`** (0x48/18 words) and **`fn_800C3BF0`** (0x20/8 words): read
  in `target.txt` but not authored -- `fn_800C3BA0` is an angle-normalization
  loop (`cmplwi r3,0x4000` gate, then a `bdnz`-based repeated `-0x8000`
  reduction, then a `-0x4000` remainder loop, `extsh r3` at the end) that
  looks like a generic "wrap an angle into `s16` range" helper, not
  obviously `dLineMng_c`-specific (no `this`-shaped access at all -- it's a
  pure `s16 f(s16)` transform). `fn_800C3BF0` starts with an unconditional
  branch (`b .L_800C3BFC`) I did not trace further.
- **The `move_on_circle*` family's register-permutation residuals**
  (10-30 differing instructions each, always at *exactly* matching length):
  I made two or three bounded attempts at declaration reordering per
  function (per AGENT_CONTEXT's own account of the cost/benefit here) and
  stopped once the pattern matched the documented "not source-addressable"
  finding. If a later round wants to push these to byte-exact, the reusable
  technique that closed load-order issues elsewhere in this round was
  swapping *comparison operand order* (`a < b` vs `b > a`) to influence which
  side loads first -- worth trying systematically here before concluding
  it's truly unfixable.

## Files

- `wip/author_core/d_line_mng.cpp` -- the draft, self-contained (includes the
  `dBc_c` forward declaration and all bodies above).
- `wip/author_core/local_shadow/game/bases/d_line_mng.hpp` -- local header
  override: adds `smc_UNIT_SIZE_X`, `mUnk6a`, corrects `check_term` to
  `bool`, and two `friend` declarations for the `clampPosX_800C3B20`/
  `clampPosY_800C3B60` test helpers. Refreshed from the shared header after
  the coordinator's mid-round update, then my own unmerged additions
  re-applied on top -- diff it against
  `wip/line_mng_shared/shadow_include/game/bases/d_line_mng.hpp` to see
  exactly what's mine.
- `wip/author_core/try.py` -- compile+diff driver used for every result
  above (`python wip/author_core/try.py <mangled-name> [...]`).
