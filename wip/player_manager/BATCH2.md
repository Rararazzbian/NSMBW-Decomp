# Batch B2 report — `0x8005EEE0`–`0x8005F5BF`

Compiled and diffed with `tools/auto_decomp/harness.py`'s `compile_draft`/`disasm`/
`extract`/`diff_fn` (imported directly, not reimplemented), against
`wip/player_manager/target_text.txt`, using the exact `mwcceppc.exe` flags from
`SHARED-BRIEF.md`. Driver script and raw diff logs are in
`wip/player_manager/scratch/b2/` (`run.py`, `deliverable.cpp`, `full_diff.txt`).

**No function below is claimed MATCHING unless the diff printed nothing,
explicitly stated per function**, per the brief's rule.

## Status table

| Function | Address | Size | Status |
|---|---|---|---|
| `create(int, mVec3_c*, int, u8)` | `0x8005EEE0` | `0x64` | **MATCHING** — 25/25 instructions, diff prints nothing |
| `fn_8005f4d0(mVec3_c*, int, int)` | `0x8005F4D0` | `0x9C` | Near-exact: 39/39 instructions, **1 line differs and it is a canonicalisation artifact of isolated per-function compilation, not a code difference** — see below |
| `createCourseInit()` | `0x8005EF50` | `0x580` | **NOT matching.** Target 352 instructions, draft 345. Fully authored, compiles, correct control flow on all three branches. Root causes of the remaining diff are identified and listed below, not hand-waved |
| `fn_8005f570(PLAYER_POWERUP_e, int)` | `0x8005F570` | `0x50` | **NOT matching.** 20/20 instructions present, same operations in the same order; 6 lines differ purely in *which* register holds an already-correct value (see below) |

## `create` — MATCHING, and a header contradiction

25/25 instructions, byte-for-byte, confirmed via `diff_fn`.

**Header contradiction, reported not reconciled:** `d_a_player_manager.hpp`
currently declares `static void create(int, mVec3_c*, int, u8);`. The
disassembly is unambiguous that it isn't void — both exit paths explicitly
load `r3` with `0` or `1` immediately before the shared epilogue, the same
evidence pattern the brief already used to correct `fn_8005f4d0` from `void`
to `bool`. I did not edit the real header (out of scope, and it is marked
COMPLETE); I proved the fix in a **shadow copy**
(`wip/player_manager/scratch/b2/shadow/game/bases/d_a_player_manager.hpp`,
only `create`'s return type changed) per the brief's "shadow-copy it into
your scratch, prove the change there, and report it" instruction. The real
header needs the same `void`→`bool` edit `fn_8005f4d0` already got, or every
caller of `create()` will silently discard a real return value once this
lands.

**MAP.md row 11 also has the branch polarity backwards**, worth flagging
since it's a live document other agents may still be reading: it says
"Tests `mPlayerEntry[idx]`; **on 0**, construct...". The actual condition is
inverted — `create()` only constructs (and returns `true`) when
`mPlayerEntry[plrNo]` is **non-zero**; on `0` it does nothing and returns
`false`. This matters semantically: `mPlayerEntry[i]` acts as an
"entry granted" gate that `fn_8005f570`/`addNum` set *before* `create()` is
ever expected to do anything, not a "free slot" flag `create()` itself is
checking.

**MAP.md's fn_8005f4d0 note is also slightly off** (see next section) —
`create()`'s own 3rd/4th arguments there are not `(flag, 0)` verbatim.

## `fn_8005f4d0` — 39/39 instructions, 1 canonicalisation-only line

```
19 | want: li r3, SYM0@sda21          got: li r3, scBaseID@sda21
```

Both sides load the address of the same 2-entry `{0x19, 0x1a}` table with the
identical `li rD, <addr>@sda21` encoding — same instruction, same operand
shape, different **name** attached to the relocation. The target's copy has
**no linker symbol at all** (dtk falls back to `lbl_8042BD70`, which is why
`canonicalise()` can turn it into `SYM0` — it matches the tool's own
`lbl_[8 hex]` pool pattern). My draft, compiled and disassembled in total
isolation (no link/strip pass), keeps the real local-linkage name
`scBaseID`, which the comparator's pool regex intentionally does **not**
touch (the docstring on `POOL_SYM`/`canonicalise()` explains why: it must not
paper over an actually-different reference). I could not find a source form
that reproduces a name-*stripped* symbol without going through the project's
real link+strip step, which this per-function isolated harness doesn't run.
I'm reporting this rather than claiming MATCHING; a whole-TU/land.py pass
should confirm the last line falls away on its own.

**Corrected vs. MAP.md's row-13/flag#2 prose:** the "found a free slot"
branch calls `create(i, pos, table[flag], 0)`, **not** `create(i, &pos[i],
flag, 0)`. Two things in the old note were wrong: the position argument is
the raw incoming `pos` pointer (never indexed by `i` — the same position is
reused for whichever slot is found), and the *third* argument (`type`) is
`table[flag]` (the `{0x19,0x1a}` lookup keyed by the incoming `flag` param),
not `flag` itself — `flag` never reaches `create()` directly. Confirmed
straight from the bytes (`mr r4,r28` unindexed; `lwzx r5,...` feeding `r5`,
which is `create`'s 3rd argument slot; `li r6,0` for the 4th).

```cpp
static const int scBaseID[2] = {0x19, 0x1a};  // .sdata2 0x8042BD70, do not hand-name

bool daPyMng_c::fn_8005f4d0(mVec3_c *pos, int mode, int flag) {
    for (int i = 0; i < 4; i++) {
        if (getPlayer(i) == nullptr) {
            fn_8005f570((PLAYER_POWERUP_e)mode, i);
            create(i, pos, scBaseID[flag], 0);
            return true;
        }
    }
    return false;
}
```

## `fn_8005f570` — 20/20 instructions present, 6 register-choice mismatches

Every operation the target performs is present, in the same order, doing the
same work; the diff is entirely about *which physical register* holds an
already-correctly-computed value (e.g. target keeps `(u8)i` in `r0` then
truncates the shift result into `r4`; my draft keeps the same two values in
`r5`→`r4` instead). Tried and rejected as levers (no effect on the shape):
splitting `mActPlayerInfo |= mask` into a separate read/or/write, reordering
the `type`/`mask`/`idx` locals, casting the shift operand vs. the shift
result. This is a plateau per the brief's own escalation note ("same
difference N times... apply two changes together") that I did not close
within the batch's time budget — reporting the near-miss rather than forcing
a claim.

**Important caveat about the number above:** 20/20 is only reached when the
class's own `.bss` static members (`m_playerID`, `mPlayerType`,
`mPlayerEntry`, `mCreateItem`, ...) are actually **defined** somewhere in the
same compiled translation unit — I confirmed this by temporarily adding their
definitions to my scratch draft (not part of the deliverable below; the real
definitions belong wherever the assembled `.cpp` puts them). Without those
definitions present, the isolated per-batch compile has no layout knowledge
and computes `mPlayerType`/`mPlayerEntry`/`mCreateItem`'s addresses via three
*separate* `@ha`/`@l` relocations, costing one extra instruction (21 instead
of 20) — this is exactly the same `m_playerID`-relative shared-base-register
pattern `MAP.md` already flagged for `initGame` (`+0x40/+0x50/+0x60/+0x70`).
**This is real, reproducible evidence that the pattern is a compile-time
`.bss`-layout effect** (the compiler folding multiple of its own module's
statics onto one `@ha` when it can see their definitions), not a link-time
relaxation — worth relaying to whoever assembles the final TU: once the real
file has all these members defined together, this class of diff should
improve on its own, independent of anything a single batch does.

```cpp
bool daPyMng_c::fn_8005f570(PLAYER_POWERUP_e mode, int i) {
    u8 idx = i;
    int type = mPlayerType[i];
    u8 mask = 1 << idx;
    mActPlayerInfo |= mask;
    mPlayerEntry[i] = 1;
    mCreateItem[type] = 8;
    mKinopioMode = mode;
}
```

Note: no explicit `return` — the header already declares this `bool`. The
target never re-loads `r3` before `blr`, so it returns whatever was in `r3`
at function exit, i.e. the incoming `mode` parameter, unused by its only
caller (`fn_8005f4d0`, which discards the result immediately). Compiles
clean under `-O4`; MWCC does not error on the implicit fall-off, only a
plain function that returns the untouched parameter register.

## `createCourseInit` — the hard one. Characterised, not matching.

352 target instructions, 345 in my draft. Every branch is authored (the
"respawn elsewhere" fallthrough, the staff-credits fixed formation, and the
full course-in path: `getFileP`, the mid-scroll camera override, the
gameMode==2 fixed order vs. the random-insertion sort, the no-balloon count,
and the spacing loop). This is not a stub — every value and every call site
in the target's disassembly has a corresponding line in the source below.
Four concrete, separately-diagnosed gaps remain open:

1. **The `action==0 || action==1` fold.** The target keeps three
   independent `cmpwi`/`beq` checks (one per `{0, 1, 0x17}`). MWCC's `-O4`
   optimizer recognises `action != 0 && action != 1` (or the `||`-negated
   membership form, or a `switch` with fall-through cases, or three nested
   single-condition `if`s — **all four tried**) as a contiguous range and
   folds it to one `cmplwi r31, 0x1; ble`. I could not find a source shape
   that defeats this fold within budget. It costs 2 instructions and shifts
   every branch offset after it, which is why the diff looks large even
   though the rest of the function is structurally identical — see point 4.
2. **`getFileP` inlines in my draft instead of emitting a `bl`.** This is
   the brief's own flagged open question, and I now have a concrete data
   point on it: at 345 instructions my function is apparently still under
   whatever per-caller size/shape threshold MWCC uses, so the exact same
   `getFileP` body (`d_cd.hpp:101`) that the target calls out-of-line gets
   inlined here instead — the identical `mulli...0x3b0; lwz
   m_instance__5dCd_c@sda21; add; lwz...0x2c; cmpwi` sequence the brief's
   MAP.md already documented for the *other* two inline sites. If fixing (1)
   grows the function by those 2+ instructions, it may cross the threshold
   and start emitting the real call — I ran out of budget before confirming
   this, but it's the most promising next lever, and it is direct evidence
   about my function's size/shape, not a header problem, exactly as the
   brief predicted. **Do not reach for `NOINLINE`.**
3. **One boolean-materialisation idiom.** The `action ∉ {0,1,0x17}`
   branch's `flag = (pos.x <= dispCenterX) ? 0 : 1` compiles to the generic
   `mfcr/extrwi/cntlzw/extrwi` normalise-to-bool sequence in my draft,
   against the target's direct `cror eq,lt,eq; bne; li 0/li 1` branch
   pair — despite the `fcmpo`/`cror` comparison itself matching exactly.
   Tried both a ternary and an explicit `if`/`else`; both compiled to the
   same bit-trick form. This single difference is also what pulls in one
   extra callee-saved GPR (`_savegpr_26` instead of the target's
   `_savegpr_27`).
4. **Everything downstream of (1)–(3) is a register/offset *shift*, not a
   different program.** Past the point where the frame layout first
   diverges, essentially every remaining line-for-line mismatch in
   `full_diff.txt` is the same operation on the same operand, just a
   different physical register or a different immediate stack offset
   because the two sides' frames disagree from (1)/(3) onward. I read every
   line of the full diff (`wip/player_manager/scratch/b2/full_diff.txt`,
   361 lines) to confirm this rather than assuming it.

**One fixed bug worth flagging on its own:** my first draft called
`dScStage_c::getCourseIn()` (the `NOINLINE` accessor) for the mid-scroll
camera check. That is **wrong** — per `SHARED-BRIEF.md`'s own hazard-2 notes,
`getCourseIn()`'s only triggering call site in this whole TU is
`initStage` (function #3, B1's). `createCourseInit` reads
`dScStage_c::m_isCourseIn` directly (a plain `lbz ...@sda21`, no call at
all). Calling the accessor here would have forced a spurious `bl` that
doesn't exist in the target and would have falsely looked like it was
"only capturing the field, not the call". Fixed in the delivered source.

**Frame-layout finding, likely the single highest-value one in this
report:** my first complete draft needed a `-0xe0` frame and four
callee-saved FPRs (`f28`-`f31`) against the target's `-0xa0`/one FPR
(`f31`). The entire gap traced to declaring a separate `mVec3_c pos` inside
*each* of the three mutually-exclusive branches instead of **one** `mVec3_c
pos;` at function scope, assigned differently per branch. Once hoisted to a
single function-scope local, the frame and FPR count matched the target
**exactly** through the whole prologue, dropping the instruction count from
355 to 345 in one change. Recorded here in case it generalises — a function
with several mutually-exclusive branches each wanting "the working position"
should declare that local once at the top, not per-branch, or MWCC's
register allocator treats them as independently-live and spills far more
than the source actually needs.

**Undeclared fields touched, house-styled as `reinterpret_cast` accessors
per `d_a_player_demo_manager.cpp`'s established convention** (never as
hand-named members in a frozen header):
- `dScStage_c` `+0x120e` (u8, the `getFileP`/`getPlayerSetPos` "file" index)
  and `+0x1211` (u8, "gotoNo") — **also touched by `getPlayerCreateAction`
  (B1's function, same TU)**. I don't have visibility into what B1 named
  these; whoever assembles the file needs to deduplicate the two batches'
  accessor definitions for the same two fields, or pick one and repoint the
  other call site.
- `dInfo_c` `+0xaf4` (int), and `+0x10..+0x1c` treated as one `mVec3_c`-sized
  copy plus a trailing `+0x1c` int. These fall inside `d_info.hpp`'s own
  documented `pad11[0x712]` (which spans object offset `0x3ec`..`0xafe`,
  computed by summing every member ahead of it in the header) — real,
  currently-padded fields, not a foreign object.

**`dCdFile_c::mpScrollData->mID`** — used the *already-named* header member
(`sScrollData::mID`, `d_cd_data.hpp`) rather than a raw offset cast, since
the target's `lbz ...,0x11(r3)` reads exactly the low byte of that `u16`
field. This is a byte-narrowing read the compiler is free to satisfy with a
single-byte load instead of a half-word load + mask, and it let me use the
named member instead of reaching for a cast.

## Data objects emitted, by section

| Object | Section | Value | Notes |
|---|---|---|---|
| `scBaseID[2]` | `.sdata2` (indexed table at `0x8042BD70`) | `{0x19, 0x1a}` | File-scope `static const int[2]`, **not** function-local (see `fn_8005f4d0` section above for why). Owned by this batch per `SHARED-BRIEF.md`'s explicit instruction not to hand-name it. |
| `scOfsX[4]` | `.sdata2`/`.rodata` pool floats inside `lbl_802EF5D8` (`0x802EF5D8`-`0x802EF5E7`, confirmed byte-for-byte from `target_rodata.txt`) | `{-184.0f, 200.0f, -208.0f, 224.0f}` | Function-local `static const float[4]` inside `createCourseInit`'s staff-credits branch. Do not hand-declare a named `.rodata` object for it — it's emitted by the code that uses it. |
| `scOfsY[4]` | same object, `+0x10..+0x1f` (`lbl_802EF5D8+0x10`) | `{-48.0f, -48.0f, 0.0f, 0.0f}` | Same object as above, second half. |
| `504.0f` | `.sdata2` `lbl_8042BD58` | pool float | Referenced, not declared — matches `MAP.md`'s existing note. |
| `0.1f`, `12.0f`, `24.0f`, `0.0f` (threshold) | `.sdata2` pool floats already catalogued in `MAP.md`'s flag-#4 `.sdata2` list (`@80386_8042BD5C`, `@80387_8042BD60`, `@80388_8042BD64`, `"@80186_8042BD48"`) | — | Referenced only, confirming `MAP.md`'s existing values against my own read of `target_sdata2.txt`; no new claims. |
| the `{0,1,3,2}` table at `lbl_802EF478` | `.rodata` | — | Turned out **not** to be used by any B2 function — it's `l_start_pos_ofs`'s *predecessor* object and the base `createCourseInit` computes `+0x160`/`+0x170` offsets from, but the table's own 4 words are never read by anything in this batch. Reporting so nobody double-claims it. |

Two contradictions already called out above, repeated here since the brief
asks contradictions to be reported rather than reconciled:
1. `create`'s header return type (`void`, should be `bool` — same evidence
   class as `fn_8005f4d0`'s existing fix).
2. `MAP.md` row 11's branch polarity for `create` is inverted from the actual
   bytes.
3. `getFileP`'s inlining-vs-`bl` boundary (open question, now with one
   concrete "still inlines at 345 instructions" data point rather than a
   guess) — belongs to whoever assembles the final TU, since the answer
   depends on the *exact* final byte count of `createCourseInit`, which this
   batch could not fully close.
4. The `dScStage_c` `+0x120e`/`+0x1211` accessor is needed by both this
   batch and B1's `getPlayerCreateAction` — needs deduplication at assembly
   time, not something either batch can resolve alone.

## Deliverable source

Compiles clean (`compile_draft` returns success) against the real project
headers plus the one shadow copy noted above; `wip/player_manager/scratch/b2/deliverable.cpp`
is the exact file compiled for every number in this report.

See the reply for the full source (identical to
`wip/player_manager/scratch/b2/deliverable.cpp`).
