# `.rodata` contradiction, settled — `d_a_player_manager.cpp` (`daPyMng_c`)

Task: reconcile `BOUNDS.md`'s claim ("one object only", `0x10` bytes) against
two independent findings (`BATCH2.md`'s `createCourseInit` local statics,
`ASSEMBLY.md`'s compiled-object reproduction) that put `scOfsX`/`scOfsY`
ahead of `scModelTypeDt` too, for a hypothesised `0x40`-byte claim.

**Neither number is right. The correct claim is larger than both: `0x1A0`
bytes, five objects, `0x802EF478`–`0x802EF618`.** Two more objects that
*every* prior pass — `BOUNDS.md`, `STATICS.md`, `BATCH1.md`, `BATCH2.md`,
`ASSEMBLY.md` — missed or actively misattributed are ours: `l_start_pos_ofs`
(`0x150` bytes) and an unresolved `0x10`-byte anchor object. This is not a
subtle miss; `l_start_pos_ofs` is directly loaded by name inside our own
`getPlayerSetPos`, and `STATICS.md`'s claim that it belongs to
`daPyDemoMng_c`/`dAcPy_HIO_Speed_c` had already propagated into
`assembled.cpp` as a broken `extern` declaration with **no definition
anywhere in the project** — that would have been an undefined-symbol link
failure the moment this unit was actually linked, the exact "costs a link"
scenario this task exists to catch.

## 1. The corrected claim

| | Value |
|---|---|
| Absolute range | `0x802EF478`–`0x802EF618` |
| Section-offset range (base `0x802EDFE0`) | `0x1498`–`0x1638` |
| Size | `0x1A0` (416 bytes) |
| Object count | 5 |

Upper bound (`0x802EF618`) is unchanged from `BOUNDS.md` and is the
strongest evidence in the file: `dtk_splits_wiimj2d.txt` gives
`d_a_sink_dokan.cpp .rodata start:0x802EF618` exactly, and it is also
`scModelTypeDt`'s end. Nobody has ever disputed that end. What was wrong is
everything on the low side of it.

## 2. Per-object table

| Address | Size | Symbol | What | Referenced by (ours) | Ours? | Confidence |
|---|---|---|---|---|---|---|
| `0x802EF478` | `0x10` | `lbl_802EF478` (unnamed, global-scope, `int[4] = {0,1,3,2}`) | Anchor object, see §4 | Indirectly — base for `createCourseInit`'s `+0x160`/`+0x180` offsets, own 4 words never directly read by any of the TU's 67 functions | **Yes**, but unresolved — see §4 | High on ownership, but the mechanism that keeps it alive is not identified |
| `0x802EF488` | `0x150` | `l_start_pos_ofs` (named global, `mVec3_c`/`Vec`-shaped `[28]`) | Player start-position offset table, indexed by `nextGoto->mType` (stride `0xC` = `sizeof(Vec)`) | `getPlayerSetPos__9daPyMng_cFUcUc`, direct `lis/addi @ha/@l` at `0x8005EDD8`/`0x8005EDDC` | **Yes** | High — direct name reference from an unambiguous `daPyMng_c` method inside our own `.text` range |
| `0x802EF5D8` | `0x20` | `lbl_802EF5D8` (unnamed, `float[4]+float[4]`) | `createCourseInit`'s `static const float scOfsX[4]` / `scOfsY[4]` (staff-credits branch) | `createCourseInit__9daPyMng_cFv`, reached via `r30 = r29+0x160` (scOfsX) and `r29 = r29+0x170` (scOfsY) | **Yes** | High — byte values decoded from `target_rodata.txt` (`{-184,200,-208,224}` / `{-48,-48,0,0}`) match `assembled.cpp`'s existing `scOfsX`/`scOfsY` literals exactly |
| `0x802EF5F8` | `0x10` | `@77211` (pooled, local, `int[4] = {-1,-1,-1,-1}`) | Compiler-pooled initializer for `createCourseInit`'s plain local `int order[4] = {-1,-1,-1,-1};` | `createCourseInit__9daPyMng_cFv`, `r4/r6 = 0x180(r29)` at `0x8005F110`/`0x8005F114`, unconditionally copied to the stack before the `m_gameMode==2` check; the `cmpwi r0,-1` checks later in the weight-sort loop match `if (order[i]==-1)` | **Yes** | High — value match plus pool-ID proximity to our own confirmed `.bss` destructor-chain nodes (`@77033`-`@77036`, per `AGENT_CONTEXT.md` §5's technique) |
| `0x802EF608` | `0x10` | `@LOCAL@getCourseInPlayerModelType__9daPyMng_cFUc@scModelTypeDt` | `unsigned char[4] = {0,1,2,3}` | `getCourseInPlayerModelType__9daPyMng_cFUc`, `0x8005FC10`-`14` | **Yes** | Exact — mangled name, already established, unchanged from `BOUNDS.md` |

All five are contiguous with **zero gap** in `target_rodata.txt` (each
object's end address equals the next one's start), and the block is bounded
on the low side by `scCloudOffset` (`0x802EF448`–`0x478`, confirmed
`d_a_player_hio_ADJ.cpp`'s own already-landed source, see §3) and on the
high side by the official split boundary at `0x802EF618`.

## 3. What's immediately before `0x802EF478`, and who owns it

`scStoopOffset` (`0x802EF3B8`, `0x30`), `scYoshiOffset` (`0x802EF3E8`,
`0x60`) and `scCloudOffset` (`0x802EF448`, `0x30`) are **not ours**. They are
directly named in `wiimj2d_symbols.txt` (unmangled global names — real,
matching the retail symbol table) and defined, byte-for-byte, in
**already-landed banked source**:

```
source/dol/bases/d_a_player_hio_ADJ.cpp:197  static const dPyModelData_s scStoopOffset[3] = {...};
source/dol/bases/d_a_player_hio_ADJ.cpp:203  static const dPyModelData_s scYoshiOffset[3][2] = {...};
source/dol/bases/d_a_player_hio_ADJ.cpp:209  static const dPyModelData_s scCloudOffset[3] = {...};
```

This is the strongest evidence tier available (already-matching code in
`source/`, per `AGENT_CONTEXT.md` §5 rank 1) and it draws a hard, provable
line: everything from `scCloudOffset`'s end (`0x802EF478`) onward, up to the
official split at `0x802EF618`, is *not* `d_a_player_hio_ADJ.cpp`'s. Combined
with the reference evidence in §2, that "everything onward" is entirely
`daPyMng_c`'s.

`lbl_802EF2E0` (`0xD8`, further back at `0x802EF2E0`) is also not ours —
`source/dol/bases/d_a_player_hio_ADJ.cpp`'s own header comments (lines 51,
60) name it as `d_a_player_hio_ADJ.cpp`'s `scStoopOffset`-adjacent data
directly by that address, independently confirming the same boundary.

## 4. `lbl_802EF478` — ours, but the trigger is unresolved

**Why it must be ours**, independent of whether its own 4 words are ever
read: `createCourseInit` computes `addi r30, r29, 0x160` and (in the
staff-credit branch) `addi r29, r29, 0x170`, where `r29` was set up moments
earlier via `lis r29, lbl_802EF478@ha` / `addi r29, r29, lbl_802EF478@l`
(`0x8005EF6C`-`74`). `0x802EF478 + 0x160 = 0x802EF5D8` (`scOfsX`, proven ours
by value match) and `+0x170 = 0x802EF5E8` (`scOfsY`, inside the same
object). A fixed, compile-time-constant offset between two `.rodata` objects
is only knowable to the compiler if **both** objects are being laid out by
the **same compilation** — MWCC cannot know at compile time how far away an
object in a *different* translation unit will end up, only the linker can,
and that requires a distinct relocation naming the real target, not an
`ADDI` immediate computed off an unrelated base. So `lbl_802EF478` is
necessarily emitted by `d_a_player_manager.cpp`'s own compile.

**Its own value is never read.** I grepped every `@ha` reference in all 67
functions across the whole `0x8005E9A0-0x80061310` `.text` range (not just
`createCourseInit`) for any load off `lbl_802EF478`'s own offset (`0(r29)`
while `r29` holds that base) and found none. `BATCH2.md` reported the same
negative result independently ("not used by any B2 function ... the table's
own 4 words are never read by anything in this batch"). No later batch
resolved it. This is consistent with — not contradicting — this project's
own documented `.bss` anchor-sharing behaviour (`SHARED-BRIEF.md`'s B5
finding: MWCC reaches `mPlayerEntry` off `m_playerID`'s relocation "including
arrays the function never reads"); the same mechanism plausibly applies here
in `.rodata`, except the anchor object itself must still be kept alive by
*some* genuine reference for the linker not to have stripped it (a plain
unreferenced `static const` gets folded away entirely at `-O4`, per
`AGENT_CONTEXT.md` §6 — "`const` can delete your object"). I could not find
that reference.

**One tempting, and wrong, lead**: the values `{0, 1, 3, 2}` exactly match
`daPyMng_c::initGame()`'s default `mPlayerType[] = {0,1,3,2}` assignment.
But `initGame`'s target disassembly (`0x8005EA60`-`0x8005EB00`) stores those
via four individual `li` immediates, not a table read — confirmed already
in `BATCH1.md` ("Same instruction count (41 target, 41 draft)") and
reconfirmed here. Writing a loop-driven table version of `initGame` would
regress an already-matching function, so this is not the answer, even though
the coincidence is suspicious enough to be worth recording.

**Position is load-bearing, not just presence.** I compiled the current
`assembled.cpp` (with `l_start_pos_ofs` now properly defined, §5) through
`tools/auto_decomp/harness.py`'s `compile_draft` and read the resulting
object's `.rodata` with `dtk elf disasm`. Without `lbl_802EF478`, our output
is exactly `0x190` bytes (`l_start_pos_ofs` `0x150` + `scOfsX` `0x10` +
`scOfsY` `0x10` + the `order[]` pool `0x10` + `scModelTypeDt` `0x10`) —
**short by exactly `0x10`**, the missing object, with everything else
byte-identical and in the right relative order. This is reported as an
**open finding for the next agent**, not fabricated: I did not add a dummy
`static const int[4]` to `assembled.cpp` for it, because an unread one would
be optimized away and would not reproduce the object; doing so anyway would
be inventing a positive result, which `AGENT_CONTEXT.md` §5 rule 5 says not
to do. What would settle it: find which of the other 66 functions in this TU
reads `{0,1,3,2}` (or takes this table's address) in a way my `@ha`-anchored
grep didn't catch — e.g. reached through a register already carrying a
different symbol's base, the way `createCourseInit` itself reaches it.

## 5. Fix applied to `assembled.cpp`

`assembled.cpp` (lines ~155-183, before this session) declared:

```cpp
// l_start_pos_ofs -- NOT ours. .rodata:0x802EF488, outside our claimed
// .rodata bound (0x802EF608-18). Belongs to daPyDemoMng_c /
// dAcPy_HIO_Speed_c per STATICS.md. Declared extern only so this TU compiles
// and links against whichever TU defines it (B1).
extern const mVec3_c l_start_pos_ofs[];
```

Nothing in `source/` defines `l_start_pos_ofs` — I grepped the whole tree.
That `extern` was a live undefined-symbol link failure waiting to happen the
first time this unit was actually linked, not merely a missing 0x150-byte
`.rodata` claim.

Replaced with a real definition, values read directly out of
`target_rodata.txt` (`.rodata:0x802EF488`, size `0x150`, verified
byte-for-byte against the compiled object's own emitted `.4byte`s — all 84
words match), using `Vec` (`include/lib/revolution/MTX/mtxtypes.h`, a plain
`{f32 x,y,z;}` POD) instead of `mVec3_c` — `mVec3_c` has user constructors
and MWCC rejected an aggregate-style array initializer against it (`error
10174: illegal initialization`); `Vec` is the same shape as
`dPyModelData_s`, the POD pattern this codebase already uses for the
sibling `scStoopOffset`/`scYoshiOffset`/`scCloudOffset` tables in
`d_a_player_hio_ADJ.cpp`:

```cpp
const Vec l_start_pos_ofs[28] = {
    {0.0f, -16.0f, 8.0f}, {0.0f, -16.0f, 8.0f}, {16.0f, -16.0f, 0.0f}, {16.0f, -32.0f, 0.0f},
    {16.0f, -16.0f, 0.0f}, {16.0f, -32.0f, 0.0f}, {0.0f, -32.0f, 0.0f}, {0.0f, 0.0f, 8.0f},
    {0.0f, 0.0f, 8.0f}, {0.0f, 0.0f, 8.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 8.0f}, {0.0f, 0.0f, 0.0f}, {8.0f, -16.0f, 0.0f}, {8.0f, -16.0f, 0.0f},
    {8.0f, -16.0f, 0.0f}, {8.0f, -16.0f, 0.0f}, {8.0f, -16.0f, 0.0f}, {8.0f, -16.0f, 0.0f},
    {0.0f, 0.0f, 8.0f}, {0.0f, 0.0f, 8.0f}, {8.0f, -32.0f, 8.0f}, {0.0f, -16.0f, 8.0f},
    {0.0f, 0.0f, 8.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, -16.0f, 8.0f},
};
```

Plus a comment block explaining the unresolved `lbl_802EF478` situation
in-place (§4's content, condensed), so the gap isn't silently reintroduced.

**Compiled: YES.** `compile_draft` succeeds. Confirmed via `dtk elf disasm`
that the emitted `.rodata` for `l_start_pos_ofs` is byte-identical to
`target_rodata.txt` (all 84 words), correctly followed immediately by
`scOfsX`/`scOfsY`/the `order[]` pool/`scModelTypeDt` in the right order —
only `lbl_802EF478` itself (§4) is still missing from the compiled output.

**Offset-perturbing: YES, and it was already actively broken.** Before this
fix, `l_start_pos_ofs` was `extern` (emits nothing) and the section would
have been short by `0x150` bytes at minimum, on top of the `extern` link
failure. After this fix, the remaining gap is `0x10` bytes
(`lbl_802EF478`), not `0x1A0`.

## 6. Secondary finding: this fix shifts `getPlayerSetPos`'s own codegen

Not this task's mandate to fix, but worth flagging: with `l_start_pos_ofs`
now a real, fully-sized `const Vec[28]` instead of an incomplete `extern`
array, `getPlayerSetPos`'s compiled body diverges from target by more than
`BATCH1.md`'s previously-reported single-instruction gap (the missing
`frsp`) — there is now also a consistent `r4`/`r5` register swap and the
`nextGoto->mFlags & 0x40` check has moved relative to the `y`-offset
addition. This is plausibly because MWCC's optimizer treats a fully known
constant array differently from an incomplete `extern` one for scheduling
purposes. Reporting per `AGENT_CONTEXT.md` §4 (report contradictions rather
than reconcile) — not fixed here, out of scope for a `.rodata`-bounds task,
but the next agent on `getPlayerSetPos` should know the near-miss picture
changed once `l_start_pos_ofs` became real.

## 7. Summary for the lead

- `BOUNDS.md`'s `.rodata` line (`0x802EF608`-`18`, one object, `0x10`) is
  **wrong** — confirmed contradiction, not reconciled away.
- `STATICS.md`'s claim that `l_start_pos_ofs` belongs to
  `daPyDemoMng_c`/`dAcPy_HIO_Speed_c` is **wrong** and had already produced a
  broken `extern` in `assembled.cpp` with no definition anywhere in the
  project — fixed.
- `ASSEMBLY.md`/`BATCH2.md`'s hypothesised `0x40`-byte / three-object
  correction was **on the right track but incomplete** — it caught
  `scOfsX`/`scOfsY` but not `l_start_pos_ofs` or `lbl_802EF478`.
- Corrected claim: **`0x802EF478`–`0x802EF618`, `0x1A0` bytes, five
  objects.** Four are proven ours by direct reference or exact byte-value
  match. The fifth (`lbl_802EF478`, `0x10` bytes) is proven ours by a
  compile-time-offset argument but its source-level trigger is unresolved —
  flagging rather than guessing, per project rules.
- `assembled.cpp` fixed for the `0x150`-byte piece (`l_start_pos_ofs`,
  compiled and byte-verified). The `0x10`-byte piece (`lbl_802EF478`)
  remains an open item; adding a plausible-looking unused local would not
  actually reproduce it and was deliberately not done.
