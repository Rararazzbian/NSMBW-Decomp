# Closer F report - `writeBanner`

Reopened from Closer B's near-miss state (63/66 instructions). Compiled and
diffed exclusively with `tools/auto_decomp/harness.py`'s `compile_draft` /
`disasm` / `extract` / `diff_fn` (imported directly, never hand-invoked).
Extraction verified by ADDRESS against `bin/dtk/wiimj2d_symbols.txt`, and the
full TU re-verified against the lead's merged regression file so a change to
this one function could not silently break another. Work in
`wip/nand_thread/scratch/closer_f/`.

**Not closed.** Reported as a precisely characterised negative result, per
the brief's rule: an honest "here is exactly where it diverges" over a false
match. The gap narrowed from 3 instructions (63/66) to 2 (64/66) with a
structurally better-justified source, and the SDK-call hypothesis this round
was assigned to test was checked directly against the target bytes and is
**refuted**, not confirmed.

## Status table

| Function | Address | Size (map) | Status |
|---|---|---|---|
| `writeBanner(NANDFileInfo*)` | `0x800CF500` | `0x108` (66 instr) | **NOT matching.** Best draft this round: 64/66 (up from Closer B's 63/66), single remaining divergence: the field-0 `iconSpeed` write folds to two immediate ops where the target keeps a genuine two-register runtime shift. |

Verified via `wip/nand_thread/scratch/closer_f/verify/d_nand_thread.cpp`
(same basename as the real TU file, required so anonymous-namespace symbol
suffixes match target textually - a mismatched temp filename produces a
false "NO MATCH" purely from the `@unnamed@<file>_cpp@` suffix, see
"A tooling trap" below). Size re-confirmed against
`bin/dtk/wiimj2d_symbols.txt`: `writeBanner__13dNandThread_cFP12NANDFileInfo`
= `0x108` = 66 instructions, matching `wip/nand_thread/target_raw.txt`
line 643-644 exactly.

## Byte-level table: everything `writeBanner` itself stores into the banner buffer

Derived directly from `target_raw.txt` lines 646-711 (the target's own
instruction stream only - not what the opaque `NANDInitBanner` call writes,
which this function's disassembly cannot see).

| Offset in `a_banner` | Width | Value | Instruction(s) | Order |
|---|---|---|---|---|
| `0xA0` (`bannerTexture`) | `0x6000` bytes | copied from `bannerRes` resource | `bl memcpy` at `0x800CF584` | 1st |
| `0x60A0` (`iconTexture`) | `0x1200` bytes | copied from `iconRes` resource | `bl memcpy` at `0x800CF5BC` | 2nd |
| `0x8` (`iconSpeed`) | **one halfword** (`sth`) | `(existing & ~3 \| 2) & 0xFFFFFFF3` | `lhz`/`slw`/`slw`/`andc`/`or`/`clrlwi`/`rlwinm`/`sth`, `0x800CF5C0`-`0x800CF5E8` | 3rd, immediately before `bl NANDWrite` |

Confirms two of the task's three checkpoints directly from bytes:

1. **Width is unchanged.** `iconSpeed` is written as a **halfword** in both
   target and every draft (`sth`), matching the header's `u16 iconSpeed`.
   No width mismatch - ruled out.
2. **It is one store, not two.** Despite touching two adjacent 2-bit
   sub-fields (bits `0-1` set to `2`, bits `2-3` cleared to `0`), the target
   issues exactly **one** `lhz` and exactly **one** `sth` - the two logical
   field writes are fused into a single read-modify-write by the optimiser
   before it ever reaches memory. This directly falsifies a literal reading
   of the brief's "two stores -> two statements" heuristic: the two fields
   *are* two separate source-level operations (confirmed by field 1's
   independent, unconditional `rlwinm` mask), but they collapse to one
   memory transaction because MWCC recognises two adjacent read-modify-write
   accesses to the same halfword, with no aliasing call in between, and
   forwards the intermediate value through a register instead of a second
   `lhz`.
3. **`NANDInitBanner`'s call site is checked directly and does not carry a
   speed argument** - see below.

## The SDK-call hypothesis (this round's assigned angle), tested and refuted

The lead's update asserted `NANDInitBanner` is in the TU's external-symbol
set (true, confirmed independently by its `bl` at `0x800CF54C`) and asked
whether its real signature carries "a speed/flags value" that would explain
the missing instructions. The call site itself settles this:

```
660 lis r27, ...a_banner@ha
661 mr r6, r3        ; r6 = subtitle
662 mr r5, r28        ; r5 = title
663 li r4, 0x0        ; r4 = 0                      <-- the "flags" argument
664 addi r3, r27, ...a_banner@l   ; r3 = &a_banner
665 bl NANDInitBanner
```

`r4` (the second argument) is a **plain literal `0`** (`li r4, 0x0`), loaded
immediately before the call and used nowhere else. It is not `2`, not
derived from `r28`/`r29`/`r30` (the registers that later hold the icon-speed
mask/value/shift), and it is not read back afterward. The icon-speed
computation happens **40 instructions later** (`0x800CF5C0`-`0x800CF5E8`),
after both `getRes`/`memcpy` pairs, using registers loaded independently at
`0x800CF594`-`0x800CF5A0`. There is no register or data-flow connection
between `NANDInitBanner`'s argument and the icon-speed write.

**Conclusion: `NANDInitBanner`'s existing declaration
(`void NANDInitBanner(NANDBanner*, u32 flags, const wchar_t*, const wchar_t*)`
in `include/lib/revolution/NAND/NANDCore.h`) is correct as-is.** No header
change is proposed for it. This is a contradiction of the round's working
hypothesis, reported rather than reconciled, per the brief's rule 4.

## New evidence: the `clrlwi r0,r0,16` step, and what it implies

Not noted by Closer B. The target's field-0 sequence is:

```
694 lhz r4, 0x8(r27)          ; r4 = iconSpeed (zero-extended by lhz)
695 slw r3, r28, r30          ; r3 = 3 << shift
696 slw r0, r29, r30          ; r0 = 2 << shift
698 andc r3, r4, r3           ; r3 = iconSpeed & ~mask
700 or   r0, r3, r0           ; r0 = r3 | val
701 clrlwi r0, r0, 16         ; r0 &= 0xFFFF   <-- extra safety mask
703 rlwinm r0, r0, 0, 30, 27  ; r0 &= 0xFFFFFFF3  (field 1 clear, exact
                               ;   same instruction/mask as every folded
                               ;   draft - this part is already proven)
704 sth r0, 0x8(r27)
```

Every folded draft (mine and Closer B's) needs no `clrlwi`, because once the
shift is known to be `0` at compile time, the compiler also knows the result
already fits in 16 bits and elides the safety mask. The presence of
`clrlwi r0,r0,16` in the target is itself evidence that **MWCC did not treat
the shift amount as a proven-small compile-time constant** - a shift by an
amount the optimiser cannot bound could in principle push bits past bit 15,
and the mask exists specifically to clean that up. This is consistent with
(not proof of) the shift genuinely being a runtime value in the original
source, rather than a literal `0` obscured by any local-variable trick.

## Every variant tried this round

All compiled and diffed via `wip/nand_thread/scratch/closer_f/wb_batch.py`
and `wb_batch2.py`, isolated single-function compiles against the real
(unmodified) header, cross-checked in the full-TU harness described below.
None of the following had been tried by Closer B; the objective was to make
the fold-defeating value opaque to MWCC's optimiser without volatile's
per-use-reload cost or static's illegal storage cost:

| Variant | Idea | Result |
|---|---|---|
| `A_two_calls_same_inline` | Same `setIconSpeed(banner,frame,speed)` inline helper, called twice: `(0,2)` then `(1,0)`, instead of one call plus a hand-written field-1 mask | **64/66**, fully folded (no `slw`) |
| `B_const_array_lookup` | `mask`/`speed`/`shift` read from `const u8` arrays indexed by a literal `frame` | 64/66, folded |
| `C_const_array_field0_only` | Same, but only field 0 goes through named `const u8` scalars | 64/66, folded |
| `D_for_loop_over_frames` | A real `for (frame = 0; frame < 2; frame++)` loop, letting the compiler decide whether to unroll | 64/66, folded (fully unrolled, no back-branch emitted) |
| `E_two_calls_reordered` | Same as A, but frame 1 called before frame 0 | 63/66, folded (order changes the fold outcome, not the fold itself) |
| `F_helper_shift_first_param` | Inline helper taking `(shift, mask, speed)` as three explicit parameters | 64/66, folded |
| `G_u16_typed_locals` | `u16 mask=3, val=2, shift=0;` (typed to match `iconSpeed` exactly, not `int`) | 64/66, folded |
| `H_interleaved_decl` | `int shift/mask/val` declared and computed immediately before the second `getRes` call, matching the target's interleaved instruction position | 64/66, folded |
| `I_address_taken_shift` | `int shift = 0; int *pShift = &shift;` dereferenced in the shift expression, to defeat register-level constant propagation without `volatile` | 64/66, folded - MWCC's `-O4` optimiser proved the pointer's target unchanged and propagated through it anyway |

**Every one of these folds completely** (no `slw`, needs only 4 saved
registers `r28`-`r31`, individual `stw`/`lwz` prologue - never the target's
`_savegpr_27`/`_restgpr_27`). Combined with Closer B's 7 fold-defeat
attempts (`volatile frame` -> 73 instr, `volatile mask/val` -> 69 instr,
`static s_frame` -> 69 instr but illegal storage, `extern` -> illegal
storage, a real C bitfield -> wrong instruction shape entirely), **16 source
shapes have now been tried across two rounds** to make a proven-zero shift
amount opaque enough to survive to codegen without either overshooting the
instruction count or requiring a data object the section bounds do not have
room for. None succeeded. MWCC's constant propagation at this
optimisation level sees through plain locals, address-taken locals, array
element reads at literal indices, and cross-call liveness alike - only
`volatile` (reloads-every-use, too many instructions) and `static`/`extern`
(different storage class, illegal placement) defeat it, and both do so for
reasons unrelated to what the target's register pattern actually needs.

### The register-pressure account, quantified

Confirms and sharpens Closer B's "one root cause, not two" finding.
Comparing my 64/66 draft's register save shape against the target's:

- **Draft** (fold succeeds): needs `r28` (`fileInfo`, called `mr r3,r28`),
  `r29` (title, short-lived), `r30` (message-ID high half, reused for two
  string addresses), `r31` (`&a_banner`) - **4** non-volatile registers,
  saved/restored with 4 individual `stw`/`lwz` pairs (7-instruction
  prologue, 8-instruction epilogue).
- **Target** (fold does not happen): needs the same 4 roles **plus** a
  dedicated register for the icon-speed *value* (`r29`) and *shift amount*
  (`r30`), with the *mask* reusing the freed title slot (`r28`) and
  `&a_banner`/title-hi reusing one slot (`r27`) - **5** non-volatile
  registers (`r27`-`r31`), saved via the shared `_savegpr_27`/`_restgpr_27`
  helper (2-instruction prologue call, 2-instruction epilogue call, plus the
  fixed `lr`/`stwu` overhead - 5-instruction prologue, 6-instruction
  epilogue).

The crossover from "4 registers, individual stores" to "5 registers, shared
helper" is a real MWCC codegen threshold, and it is **caused by**, not
independent of, whatever defeats the fold: the two *new* live values (val,
shift) only exist at all if `3 << shift` and `2 << shift` are not reduced to
plain immediates. Every experiment that keeps the fold intact needs 0 new
registers for them (fully collapses to `li r0,0x2`-class code); every
experiment that defeats the fold (`volatile`, `static`) does produce the
extra registers, but pays for it in reload instructions the target does not
have. No variant found the middle ground the target's bytes show: two
constants held in registers *once*, reused *twice*, never reloaded, yet
never folded.

## A tooling trap hit and resolved this round

Running the full-TU regression check (see below) through a *temp-named*
source file (`full_tu_test.cpp`) produced false "NO MATCH" results on six
functions that Closer B had verified matching (`existCheck`, `checkCRC`,
`deleteFile`, `createBanner`, the constructor, `getSaveData`) with **equal**
instruction counts but different pooled-symbol names. Cause: the anonymous
namespace's mangled suffix embeds the **source filename**
(`sc_GAME_FILE__27@unnamed@d_nand_thread_cpp@` vs
`sc_GAME_FILE__26@unnamed@full_tu_test_cpp@` - note the length-prefix digit
changes too, `27` vs `26`, matching the filename's own length). Renaming the
scratch file to the real basename (`wip/nand_thread/scratch/closer_f/verify/d_nand_thread.cpp`)
made all six re-match. **Not a code regression - a filename artifact.**
Recorded here because it would otherwise look exactly like a shared-header
regression on the next reopen.

## Full-TU regression check (after the tooling trap above was fixed)

Built on top of the lead's `wip/nand_thread/scratch/merge_lead/d_nand_thread.cpp`
(the current cross-batch merge, read-only, not modified), with only
`writeBanner`'s body changed to the two-call form below. Compiled and
diffed as `wip/nand_thread/scratch/closer_f/verify/d_nand_thread.cpp` against
every function's ADDRESS-extracted counterpart in `target_raw.txt`:

```
__ct__13dNandThread_cFiPQ23EGG4Heap        MATCH
__dt__Q23EGG5MutexFv                       MATCH
__dt__6mMutexFv                            MATCH
__dt__13dNandThread_cFv                    NO MATCH (24 vs 25)   -- pre-existing, unrelated to this change
run__13dNandThread_cFv                     MATCH
create__13dNandThread_cFPQ23EGG4Heap       MATCH
setNandError__13dNandThread_cFl            MATCH
getSaveData__13dNandThread_cFv             MATCH
cmdExistCheck__13dNandThread_cFv           MATCH
existCheck__13dNandThread_cFv              MATCH
cmdSpaceCheck__13dNandThread_cFv           MATCH
spaceCheck__13dNandThread_cFv              NO MATCH (37 vs 37)   -- pre-existing (Closer B), unrelated
createBanner__13dNandThread_cFv            MATCH
writeBanner__13dNandThread_cFP12NANDFileInfo  NO MATCH (64 vs 66)  -- this function
cmdLoad__13dNandThread_cFv                 MATCH
checkCRC__13dNandThread_cFv                MATCH
cmdDeleteFile__13dNandThread_cFv           MATCH
deleteFile__13dNandThread_cFv              MATCH
save__13dNandThread_cFv                    NO MATCH (86 vs 95)   -- pre-existing (Closer A's area), unrelated
load__13dNandThread_cFv                    NO MATCH (147 vs 161) -- pre-existing, unrelated
onExit__Q23EGG6ThreadFv                    MATCH
onEnter__Q23EGG6ThreadFv                   MATCH
```

This is byte-identical to the merge_lead baseline's match/no-match set
except for `writeBanner` itself (63/66 -> 64/66). **`createBanner` matches,
and every other currently-matching function in the TU still matches** with
this change. `cmdSave` ("`fn_800CF170`" in the brief) is not found by name
in the target because the target has no symbol for it - expected, matches
the brief's note, not a regression.

## Return type re-verified independently (bool vs s32)

The task asked for this to be confirmed from codegen, not asserted from
`createBanner`'s `cmplwi r3, 0x72a0` alone. Compiled `writeBanner` **both
ways** via a shadow copy of the header
(`wip/nand_thread/scratch/closer_f/shadow_inc/game/bases/d_nand_thread.hpp`,
only the `writeBanner` declaration changed to `bool`, never touching the
real header):

- **Declared `bool`**: MWCC appends a boolification tail after the
  `bl NANDWrite` and before the epilogue:
  ```
  53 bl NANDWrite
  54 neg r0, r3
  55 lwz r31, 0x1c(r1)
  56 or  r0, r0, r3
  57 lwz r30, 0x18(r1)
  58 lwz r29, 0x14(r1)
  59 srwi r3, r0, 31
  60 lwz r28, 0x10(r1)
  ```
  (`neg`/`or`/`srwi 31` is the exact `(x != 0)` materialisation idiom this TU
  already uses elsewhere - e.g. `createBanner`'s own final `mError != 0`
  conversion at `0x800CF4D4`-`0x800CF4DC` in the target.)
- **Declared `s32`** (current header): no such tail. The target's own
  instruction stream (`0x800CF5EC`-`0x800CF604`) goes straight from
  `bl NANDWrite` to the `_restgpr_27`/`lr`/`blr` epilogue - **no**
  `neg`/`or`/`srwi` anywhere.

**Confirmed: `writeBanner` returns `s32`, matching the raw `NANDWrite`
result untouched.** The header's existing declaration
(`s32 writeBanner(NANDFileInfo *fileInfo)`) is correct; no change proposed.

## Final source (unmatched, 64/66)

Delivered as the best-characterised state: reproduces both `iconSpeed`
sub-fields as two genuine source statements (matching the byte-level table
above, and superseding Closer B's single-field placeholder), at the cost of
being 2 instructions short because MWCC still folds field 0's shift.

```cpp
namespace {
inline void setIconSpeed(NANDBanner *banner, int frame, int speed) {
    banner->iconSpeed = (banner->iconSpeed & ~(3 << (frame * 2))) | (speed << (frame * 2));
}
} // namespace

s32 dNandThread_c::writeBanner(NANDFileInfo *fileInfo) {
    static NANDBanner a_banner;
    static const char *c_icon_res = "save_icon.bti";

    const wchar_t *title = dMessage_c::getMsg(0x15f91, 1);
    const wchar_t *subtitle = dMessage_c::getMsg(0x15f91, 0);
    NANDInitBanner(&a_banner, 0, title, subtitle);

    nw4r::g3d::ResFile bannerRes = dResMng_c::m_instance->getRes("save_banner", "save_banner_EU.bti");
    const u8 *bannerBase = (const u8 *)bannerRes.ptr();
    memcpy(a_banner.bannerTexture, bannerBase + *(const u32 *)(bannerBase + 0x1c), sizeof(a_banner.bannerTexture));

    nw4r::g3d::ResFile iconRes = dResMng_c::m_instance->getRes("save_banner", c_icon_res);
    const u8 *iconBase = (const u8 *)iconRes.ptr();
    memcpy(a_banner.iconTexture, iconBase + *(const u32 *)(iconBase + 0x1c), 0x1200);

    /* @unofficial: both fields of iconSpeed are reproduced (frame 0 speed=2,
       frame 1 speed=0, see the byte-level table above), and the resulting
       code correctly collapses to ONE lhz/sth pair, matching the target's
       shape. What does not match: MWCC folds frame 0's shift-by-0 to a
       plain `clrrwi`/`ori` pair; the target keeps a genuine two-register
       runtime `slw`/`slw`/`andc`/`or` sequence plus a `clrlwi r0,r0,16`
       safety mask that only makes sense if the shift amount is NOT a
       provable compile-time 0. 16 source shapes across two rounds
       (Closer B's 7, this round's 9) all either fold completely (this one,
       64/66) or defeat the fold at the cost of extra per-use reloads
       (volatile, 69-73/66) or illegal new storage (static/extern, disallowed
       by the .bss/.sdata section bounds in SHARED-BRIEF.md). The lever that
       makes MWCC treat a literal 0 as a genuine unbounded runtime shift,
       while still holding it in a register across a call and reusing it
       twice without a reload, was not found. */
    setIconSpeed(&a_banner, 0, 2);
    setIconSpeed(&a_banner, 1, 0);

    return NANDWrite(fileInfo, &a_banner, 0x72a0);
}
```

Compiles against the real, unmodified `include/game/bases/d_nand_thread.hpp`
- no shadow header needed for the deliverable itself (the shadow header was
used only for the bool-vs-s32 A/B test above, and is not part of what is
proposed).

## What would settle this

The `clrlwi r0,r0,16` step is the sharpest remaining clue: it says MWCC
genuinely did not know the shift was bounded, which rules out every plain,
address-taken, array-indexed, or reordered local variable tried so far
(all of which the optimiser proved bounded and folded through). The
remaining untried categories, in order of plausibility:

1. **A real multi-frame loop whose trip count itself is not a small literal**
   (e.g. driven by `NAND_BANNER_ICON_MAX_FRAME` used in a form the compiler
   cannot trivially prove terminates after exactly 2 useful iterations) -
   `D_for_loop_over_frames` used a hard-coded `< 2` bound, which the
   compiler still fully unrolled and folded; a bound tied to the real
   8-frame constant, with the other 6 frames' writes provably eliminated for
   an unrelated reason (e.g. dead-written-then-overwritten), was not tried.
2. **A value obtained through an actual function call this round did not
   try** - every SDK call this TU makes was checked against the icon-speed
   registers' data flow and none connect (see the `NANDInitBanner` section
   above), but the TU's *own* helpers (`setNandError`, or an unnamed
   function like `fn_800CF170` owned by another batch) were not
   cross-checked for a return value that could feed the shift.
3. Accept Closer B's plateau conclusion and move on - this is the
   recommendation if categories 1-2 are also exhausted by a future round,
   since 16 source shapes across two rounds is a large, still-negative
   sample.

## Files

- `wip/nand_thread/scratch/closer_f/wb_run.py` - single-function compile/diff driver
- `wip/nand_thread/scratch/closer_f/wb_batch.py` - variants A-F
- `wip/nand_thread/scratch/closer_f/wb_batch2.py` - variants G-I
- `wip/nand_thread/scratch/closer_f/shadow_inc/game/bases/d_nand_thread.hpp` - shadow header, `writeBanner` changed to `bool`, used only for the return-type A/B test, never applied to the real header
- `wip/nand_thread/scratch/closer_f/verify/d_nand_thread.cpp` - full-TU regression file (copy of the lead's `merge_lead` file with only `writeBanner`'s body changed), used for the full-TU match table above

No shared header, `slices/wiimj2d.json`, or `syms.txt` was edited or
proposed for change. `ninja`, `configure.py`, `progress.py` and `land.py`
were never invoked.
