# Closer B report — `spaceCheck`, `writeBanner`

Both functions were reopened from the near-miss states left by Batch 2
(`spaceCheck`, 34/37) and Batch 3 (`writeBanner`, 63/66). Compiled and diffed
exclusively with `tools/auto_decomp/harness.py`'s `compile_draft` / `disasm` /
`extract` / `diff_fn` (imported directly). Extraction verified by ADDRESS,
sizes checked against `bin/dtk/wiimj2d_symbols.txt`, emitted order checked.
Work in `wip/nand_thread/scratch/closer_b/`.

**Neither function closed.** Both are reported as precisely characterised
negative results, per the brief's own rule: a plain "could not close this and
here is exactly where it diverges" over a false claim of a match.

## Status table

| Function | Address | Size (map) | Status |
|---|---|---|---|
| `spaceCheck()` | `0x800CF0D0` | `0x94` (37 instr) | **NOT matching.** 34/37, single divergence (register choice), unchanged from Batch 2 after 11 further attempts. |
| `writeBanner(NANDFileInfo*)` | `0x800CF500` | `0x108` (66 instr) | **NOT matching.** 63/66, unchanged from Batch 3's instruction count; new structural finding about *what* the missing 3 instructions actually do (see below), but the source-level lever to reproduce them was not found. |

Both sizes independently re-verified against `bin/dtk/wiimj2d_symbols.txt`
(line 5570 for `spaceCheck`, 0x94; line 5574 for `writeBanner`, 0x108) via
`wip/nand_thread/scratch/closer_b/final_check.py`. Emitted order in the
isolated compile is `spaceCheck` then `writeBanner`, matching target order.

---

## `spaceCheck` — 34/37, the register-allocation wall confirmed, not opened

### The assigned angle, applied and ruled out

The task pointed at diffing `spaceCheck`'s source against the byte-exact
`existCheck`'s line by line to find a structural difference that explains the
size gap (`existCheck` 0xD8, `spaceCheck` 0x94). Done. The real structural
difference is real but doesn't produce a lever:

- `existCheck` makes **two** calls to `NANDGetType`/`setNandError` and needs
  **three** non-volatile registers (`r29`=this, `r30`=count, `r31`=err)
  because `count` is a value that must survive *across calls*.
- `spaceCheck` makes **one** call to `NANDCheck`/`setNandError` and needs only
  **two** non-volatile registers (`r30`=this, `r31`=err). The value that
  differs between drafts — the reloaded `answer` — does **not** survive across
  any call; it is read from its stack slot and used twice in straight-line
  code with no intervening `bl`. That is exactly why the target is content to
  put it in a plain **volatile** scratch register (`r3`) rather than promote
  it to a third non-volatile save slot. Confirmed: our draft also avoids a
  third non-volatile register (uses `r4`, also volatile) — the register class
  is right, only the specific register number is wrong.

So the existCheck/spaceCheck structural diff explains *why no third saved
register is needed*, which both drafts already agree on. It does not explain
*why the target's free-register choice for the reload is `r3` and not `r4`* —
that remains open.

### The bool-materialisation lever, checked and inapplicable here

The other assigned angle (materialising `(x==0)` into a `bool` local) does
not apply to `spaceCheck`'s two guard checks. Confirmed directly: the target's
`mError == 0` and `err == 0` tests already compile to plain `cmpwi`+`bne` in
**both** the target and every draft (lines 17–20 and 23, never in the diff).
The target performs **no** `cntlzw`/`srwi.`/`neg`/`or` materialisation
anywhere in this function. Forcing one via a `bool` intermediate would only
break the 34 lines that already match, so it was not attempted here (Batch 2
already tried the analogous `over`/`under` bool-intermediate lever on the
mask tests themselves and it made the diff *worse*, 22 lines — reconfirmed
below).

### Every variant tried (this batch), all landing on the identical 3-line diff

11 new source shapes, none tried by Batch 2, all compiled and diffed via
`wip/nand_thread/scratch/closer_b/try_new.py` and `try_new2.py`:

| Variant | Idea | Result |
|---|---|---|
| `P_enum_flags` | `NAND_CHECK_TOO_MANY_APP_BLOCKS\|...` instead of literal `5`/`0xa` | 3-line diff, `r4` |
| `Q_err_reused_as_scratch` | `err = (0 != err);` (bool-materialise err) | 22-line diff (confirms Batch 2's `J_bool_flag_intermediate` finding independently) |
| `R_answer_addr_reused_ptr` | `u32 *pAnswer = &answer;` then `*pAnswer & mask` | 3-line diff, `r4` |
| `S_mError_local_first` | Named `s32 noErr` declared before `answer`/`err`, assigned from `mError` before the branch | 3-line diff, `r4` |
| `T_answer_after_err_check_decl` | Alias local declared inside the outer `if`, before the inner `if` | 3-line diff, `r4` (also shifts one branch instruction — worse) |
| `U_this_call_explicit` | `this->setNandError(...)`, `this->mError` explicit everywhere | 3-line diff, `r4` |
| `V_inline_helper_first_param` | `inline bool hasFlags(u32 v, u32 mask)`, hoping the callee's first-parameter register (`r3`) convention would stick after inlining | 3-line diff, `r4` |
| `W_answer_volatile` | `register u32 a = answer;` (register-storage-class hint) | 3-line diff, `r4` |
| `X_answer_first_decl_before_err_no_init` | Both locals declared before either is initialised, `answer` first | 3-line diff, `r4` |
| `Y_answer_via_deref_local_ptr_no_intermediate` | Mask tests wrapped in a `switch(0) default:` block (defeats simple if/else restructuring) | 3-line diff, `r4` |
| `Z_pad_before_answer_used` | Extra `s32 pad = 0;` local, folded into the `setNandError` call's argument (`err + pad`) so it isn't dead-code-eliminated | 3-line diff, `r4` |

Combined with Batch 2's 13 variants (declaration order and split/combined
init, an unused dummy at 8 different points, both nesting shapes, `s32`
retyping, `s16`/`u8` narrowing barriers, a ternary, and a bool-intermediate
pair), **24 source shapes have now been tried**. Every one that preserves
the instruction count (20 of 24) reproduces the identical `r4` choice for the
`answer` reload; the other 4 change the instruction count and are therefore a
different program, not a register fix.

### Conclusion

This is a genuine MWCC register-allocator plateau: two registers (`r3`, `r4`)
are both dead and both eligible at the reload point, the target picks `r3`,
every source shape we can construct makes the compiler pick `r4`. No semantic
or structural difference between the two registers has been found — this
reads as an allocator implementation detail (e.g. round-robin over
previously-used argument registers) rather than anything visible at the
C++ level. Reporting it as such rather than continuing to guess.

### Final source (unmatched, 34/37)

```cpp
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            if (answer & 5) {
                mError = 7;
            } else if (answer & 0xa) {
                mError = 8;
            }
        }
    }
}
```

Unchanged from Batch 2's source (it is already the best available). The
target's own real header (`include/game/bases/d_nand_thread.hpp`) already
declares `spaceCheck()`/`setNandError(long)` correctly; no shadow header is
needed to reproduce the 34/37 result.

---

## `writeBanner` — 63/66, a sharper characterisation of the missing 3 instructions, still not opened

### Re-deriving what the missing instructions actually compute

Batch 3 characterised the gap as "a bitfield-store idiom that keeps constant-
folding away" and modelled it as one 2-bit read-modify-write:
`iconSpeed = (iconSpeed & ~(3 << (frame*2))) | (2 << (frame*2))` with
`frame == 0`. Re-deriving the target bytes directly (not reusing that
assumption) shows it is **not** a single 2-bit field write. The full
tail sequence is:

```
lhz r4, 0x8(r27)         ; r4 = a_banner.iconSpeed
slw r3, r28, r30         ; r28=3, r30=0  -> r3 = 3<<0 = 3
slw r0, r29, r30         ; r29=2, r30=0  -> r0 = 2<<0 = 2
andc r3, r4, r3          ; r3 = r4 & ~3
or   r0, r3, r0          ; r0 = r3 | 2
clrlwi r0, r0, 16        ; r0 &= 0xFFFF
rlwinm r0, r0, 0, 30, 27 ; r0 &= 0xFFFFFFF3  (clears bits 0x4 and 0x8 -- a SECOND, unrelated 2-bit field)
sth r0, 0x8(r27)         ; a_banner.iconSpeed = r0
```

Decoding the `rlwinm` mask (`SH=0, MB=30, ME=27`) gives `0xFFFFFFF3` (worked
out with a small script, not by inspection alone — MB>ME wraps around).
That instruction clears bits `0x4`/`0x8` **unconditionally**, a *different*
2-bit region from the one the `andc`/`or` pair touches (`0x1`/`0x2`). Net
effect on the low 4 bits of `iconSpeed`: bits `[1:0]` (call it "field 0") are
set to `0b10` = 2 via the runtime-shift path; bits `[3:2]` ("field 1") are
unconditionally cleared to `0b00` via a plain, fully-folded immediate AND.

This is new: **two adjacent 2-bit sub-fields of `iconSpeed` are written in
one instruction sequence**, not one. That is consistent with
`NAND_BANNER_ICON_MAX_FRAME` being `8` (`include/lib/revolution/NAND/nand.h`)
— `iconSpeed`'s 16 bits packing eight 2-bit per-frame speed codes — and this
banner using one real animation frame (frame 0, speed 2) with frame 1 forced
to 0. Field 1's write folds to a plain immediate mask (no `slw`) precisely
*because* its value (0) makes the `or` a no-op that the optimiser deletes,
leaving only a mask instruction — which is foldable regardless of what
"levers" are applied. Field 0's write is the one that refuses to fold.

### Reproducing the two-field write narrows the gap but does not close it

Writing both fields explicitly:

```cpp
int frame = 0;
a_banner.iconSpeed = (a_banner.iconSpeed & ~(3 << (frame * 2))) | (2 << (frame * 2));
a_banner.iconSpeed &= ~(3 << ((frame + 1) * 2));
```

compiles to `clrrwi r0,r0,2 / ori r0,r0,0x2 / andi. r0,r0,0xfff3` — **64/66**
instructions, one closer than Batch 3's 63/66, and now shaped like the
target's two-field semantics (an AND-immediate for field 1's clear, an
OR-immediate for field 0's set) — but MWCC still folds field 0's `3<<0`/`2<<0`
into immediates rather than emitting the target's `slw` pair. This confirms
the two-field theory narrows the *semantic* gap without closing the
*register-allocation/instruction* gap, which is still the same root cause
Batch 3 found: MWCC's constant folder is simply better at removing a
provably-zero shift than any source reshaping tried here can defeat.

### What was tried to defeat the constant fold, and why each was rejected

All tested via `wip/nand_thread/scratch/closer_b/wb_try2.py` against the
single-field baseline (fastest signal: does the shift survive as `slw`?):

| Attempt | Result |
|---|---|
| `int frame = 0;` declared immediately before use | Folds (63 instr, as Batch 3 found) |
| `int frame = 0;` declared at the top of the function, forced to survive across all 5 calls (`NANDInitBanner` + 2×`getRes` + 2×`memcpy`) | Still folds (63 instr) — MWCC's constant propagation is not defeated by call-crossing live ranges |
| A real C bitfield lvalue (`struct { u16 pad:14; u16 speed0:2; } bits;` over a union with `u16 raw`), per the task's suggested lever | Compiles to a **single** `rlwimi` instruction (63 instr, wrong shape entirely) — this *rules out* the "real bitfield struct member" theory: MWCC's dedicated bitfield-store codegen is a one-instruction rotate-and-mask-insert, nothing like the target's 7-instruction `slw`/`andc`/`or`/`clrlwi`/`rlwinm` sequence. The target's shape is **not** a compiler-native bitfield store. |
| `volatile int frame = 0;` | Genuinely defeats the fold — produces a real `slw`-based sequence — but costs **73** instructions, far more than target's 66, because `volatile` forces a reload of `frame` before *every* use instead of keeping it live in a register. Wrong shape. |
| `volatile int mask = 3, val = 2;` (volatile the operands directly, not the shift index) | Defeats the fold, but **69** instructions — still too many, and it is the operand values (not the shift) that becomes volatile-reloaded, an inversion of what the target's registers show (`r28`/`r29`/`r30` are held in **non-volatile** registers across a call, not reloaded per-use). |
| `static int s_frame = 0;` (file/function-local mutable static, opaque to the optimiser like `volatile` but normally register-allocated) | Defeats the fold (69 instr) but **is disqualified on ground-truth grounds independent of instruction count**: it allocates a new `.sbss` object. `SHARED-BRIEF.md`'s already-cross-checked `.sbss` section is exactly `0x8` bytes, entirely owned by `m_instance` — there is no room for another object, named or anonymous. Any lever that adds persistent storage this function does not already own is wrong by construction, regardless of what it does to the instruction count. |
| `extern int g_iconFrame;` | Same disqualification as `static` — requires a data object nothing in the data inventory accounts for. |

### Register pressure and the fold are the same root cause, not two bugs

Batch 3 reported the `_savegpr_27`/bitfield gap as two separate findings.
Re-examined here: they are one. The extra (5th) non-volatile register
(`r27`) is only needed because a **non-folded**, runtime-shift computation
needs three live operand registers (`r28`=mask-width-constant, `r29`=value-
constant, `r30`=shift-amount-constant) held across the second `memcpy` call.
Every source shape here that keeps the shift *foldable* needs only 4 saved
registers (`r28`-`r31`) and a plain `stw`/`lwz` prologue, exactly matching
Batch 3's 63-instruction result. The single missing lever — whatever makes
MWCC treat a compile-time-zero shift amount as a genuine runtime value while
still inlining it as straight-line code (no `bl`) — was not found. It is not
`volatile` (wrong instruction count), not `static`/`extern` (illegal data
object), and not a real C bitfield member (wrong instruction shape entirely,
proven by direct compilation).

### Final source (unmatched, 63/66 — the best of the shapes tried, and the one with correct instruction count for everything except the iconSpeed tail)

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

    /* @unofficial: reproduces only field 0 of iconSpeed (frame-0 speed=2).
       Target also unconditionally clears field 1 (bits 2-3, frame-1 speed)
       via a folded `andi. r0,r0,0xfff3` immediately after -- see CLOSE_B.md.
       Both fields' folding behaviour was reproduced independently; adding
       field 1 narrows the gap to 64/66 but does not close it and is left
       out of this deliverable to keep the source honest about what is
       proven vs. guessed (field 1's existence is proven from the target
       bytes; its C-level expression is not). */
    setIconSpeed(&a_banner, 0, 2);

    return NANDWrite(fileInfo, &a_banner, 0x72a0);
}
```

This is unchanged in instruction count from Batch 3's version (kept as the
deliverable because it is simpler and the two-field version does not close
the gap either); the two-field refinement is documented above as a finding,
not shipped, because it trades a proven-correct single-field source for an
unproven second field expression while still not reaching a match.

Uses only headers already present in the real tree
(`<game/bases/d_message.hpp>`, `<game/bases/d_res_mng.hpp>`, `<string.h>`,
`<types.h>`) plus the real, already-updated `d_nand_thread.hpp`
(`s32 writeBanner(NANDFileInfo*)` is already landed there). No shadow header
needed for this function either.

---

## Files

- `wip/nand_thread/scratch/closer_b/d_nand_thread.cpp` — deliverable source, both functions, compiles against the real (unmodified) header
- `wip/nand_thread/scratch/closer_b/final_check.py` — compiles the deliverable, diffs both functions by name, checks both sizes against `bin/dtk/wiimj2d_symbols.txt`, prints emitted order
- `wip/nand_thread/scratch/closer_b/run.py` — single-function compile/diff driver used while isolating `spaceCheck`
- `wip/nand_thread/scratch/closer_b/try_new.py`, `try_new2.py` — the 11 new `spaceCheck` variants, all reproducing the identical `r3`-vs-`r4` diff
- `wip/nand_thread/scratch/closer_b/wb_run.py` — single-function compile/diff driver used while isolating `writeBanner`
- `wip/nand_thread/scratch/closer_b/wb_try2.py` — the `volatile`/`static`/`extern` fold-defeat experiments for `writeBanner`'s `iconSpeed` write, with their instruction counts

No shared header, `slices/wiimj2d.json`, or `syms.txt` was edited. `ninja`,
`configure.py`, `progress.py` and `land.py` were never invoked.
