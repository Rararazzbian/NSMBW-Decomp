# `d_line_mng.cpp` section bounds in `wiimj2d.dol`

Scope: bounds derivation only. No class-layout work done, no files outside
`wip/agent_line_mng_bounds/` touched, nothing under `source/`, `include/`,
`syms.txt`, or `slices/*.json` edited (`slices/wiimj2d.json` was read-only
queried for cross-checks).

Method: every non-`.text` boundary below was checked by direct instruction
decode against the raw bytes of `original/wiimj2d.dol`, not by symbol-name
guessing. `.text` in a fully-linked DOL embeds absolute addresses as
`lis rX,addr@ha` / `addi|lwz|stw|lfs|stfs|...  rY,addr@l(rX)` pairs. I wrote a
small scanner (`wip/agent_line_mng_bounds/scan_all_refs.py`) that decodes every
`lis` in a chosen `.text` window, follows the same register through the next
D-form instruction that uses it as a base, and reports the resulting absolute
address. **I deliberately excluded X-form (opcode 31) instructions from the
"use" side** after an early run produced a false positive: X-form's low 16
bits are not a signed immediate (they encode rB+XO+Rc), and treating them as
one manufactured phantom references. Every address relationship reported below
survived a re-check restricted to real D-form immediates only, and I hand-
verified the raw opcode/rD/rA/imm split for the load-bearing ones (shown
inline).

Where the *neighbouring* unit is already landed (`d_lytbase.cpp`), I cross-
checked my measured boundary against its recorded `memoryRanges` in
`slices/wiimj2d.json` directly — that is independent, official ground truth,
not something I derived, and it matched on all four non-`.text` sections
(see below). Where the neighbour is *not* landed (everything on the left/low
side — items 14-22 in `wip/dol_scout/DOL_TARGETS.md`), the boundary rests on
my own instruction-level measurement only; I say so explicitly per edge.

---

## `.text`

**`0x800C0DC0`–`0x800C89A0`**, offset `0xBA640`–`0xC2220` (base `0x80006780`),
size `0x7BE0` (31712 bytes).

This was handed to me as already-established and I did not re-derive it. I did
re-confirm it is self-consistent: `__ct__10dLineMng_cFv` sits at `0x800C0DC0`
(first symbol), `isSameName__25sFStateID_c<10dLineMng_c>CFPCc` (last symbol)
ends at `0x800C8998`, `d_lytbase.cpp`'s `__ct__9LytBase_cFv` starts at
`0x800C89A0` (8-byte pad between, ordinary function alignment), and
`slices/wiimj2d.json`'s landed `d_lytbase.cpp` entry gives `.text` start
`0xc2220` verbatim — hard bracket, both ends. **Status: given fact,
independently re-confirmed, not re-derived.**

## `.ctors`

**`0x802EDDF8`–`0x802EDDFC`**, offset `0x118`–`0x11C` (base `0x802EDCE0`),
one 4-byte entry, value `0x800C7600` (`__sinit_\d_line_mng_cpp`).

I read the raw `.ctors` array myself (file offset `0x2E9DE0`) rather than
trusting a transcription:

```
slot 69 (0x802EDDF4) -> 0x800BFEA0   (item 19's sinit)
slot 70 (0x802EDDF8) -> 0x800C7600   (d_line_mng.cpp's sinit)   <-- this unit
slot 71 (0x802EDDFC) -> 0x800C9B00   (d_lytbase.cpp's own sinit)
```

`slices/wiimj2d.json`'s landed `d_lytbase.cpp` entry claims `.ctors`
`0x11c-0x120`, i.e. exactly slot 71 — zero slack against this unit's slot 70.
**Status: PROVEN, both ends** (left edge by the adjacent slot's own sinit
target being a different, later function; right edge cross-confirmed against
the official landed slice).

## `.rodata`

**`0x802F12E8`–`0x802F1318`**, offset `0x3308`–`0x3338` (base `0x802EDFE0`),
size `0x30` (48 bytes). Two objects:
- `@LOCAL@is_unit_circle2x2__10dLineMng_cFUl@d_unit` @ `0x802F12E8`, size `0x10`
- `@LOCAL@is_unit_circle4x4__10dLineMng_cFUl@d_unit` @ `0x802F12F8`, size `0x20`

Measured, not assumed: `is_unit_circle2x2__10dLineMng_cFUl` (`.text
0x800C1750`) does `lis r5,0x8031 / addi r5,r5,0x12e8` at `0x800C1750`/
`0x800C1754`; `is_unit_circle4x4__10dLineMng_cFUl` (`.text 0x800C1790`) does
the same for `0x802F12F8` at `0x800C1790`/`0x800C1794`. I then scanned the
**whole** of `d_line_mng.cpp`'s `.text` (`0x800C0DC0`-`0x800C89A0`) against
`0x802F1000`-`0x802F12E8` (the neighbour to the left) and against
`0x802F1318`-`0x802F1400` (the neighbour to the right): **zero hits either
side**. So this unit's `.rodata` footprint is exactly these two objects, no
float/string-pool tail — the exact failure mode `AGENT_CONTEXT.md` warns about
does not occur here.

Right edge is additionally cross-confirmed: `slices/wiimj2d.json`'s landed
`d_lytbase.cpp` claims `.rodata` `0x3338-0x33a0`, i.e. starting at
`0x802F1318` verbatim — the address my scan already isolated as the neighbour.
Left edge (item 21/`daLiftNormalModelDraw_c`'s cluster, unlanded) rests on my
own measurement only (absence of reference + a clean, contiguous symbol-table
boundary at `0x802F12E8`), not an official cross-check.
**Status: right edge PROVEN twice over; left edge MEASURED (no landed
neighbour to cross-check against).**

This also **confirms DOL_TARGETS.md's `.rodata` claim was correct as written**
— no correction needed here.

## `.data`

**`0x80316CA0`–`0x80317738`**, offset `0x18600`–`0x19098` (base `0x802FE6A0`),
size `0xA98` (2712 bytes).

**This corrects `DOL_TARGETS.md`, which under-scoped this section by `0x8F8`
(2296) bytes** — it named only the seven state-framework vtables
(`0x80316E98`-`0x80317738`, `0x1A0`+`0x34` bytes with a gap) as "the" `.data`,
missing a further `0x1F8` bytes *before* the six-vtable group and the
`0x384`+`0x358`+`0x80` bytes of the unit's own per-state tables *between* the
six-vtable group and the seventh vtable. Full breakdown, low to high address,
every boundary either instruction-confirmed or bracketed by a confirmed
neighbour:

| range | size | content | evidence |
|---|---|---|---|
| `0x80316CA0`-`0x80316D24` | `0x84` | `@55792` (unnamed) | referenced by an anonymous helper (`fn_800C31C0`, `.text 0x800C31C0`) at `0x800C3328`, and by `__sinit` at `0x800C761C` — both `lis r*,0x8031 / addi r*,r*,0x6ca0` pairs, hand-verified |
| `0x80316D24`-`0x80316DA4` | `0x80` | `@55889` | referenced by `mov_to_rightupper__10dLineMng_cFUlRC7mVec2_cb` at `0x800C3D38` |
| `0x80316DA4`-`0x80316E20` | `0x7C` | `@55993` | referenced by `mov_to_leftupper__10dLineMng_cFUlRC7mVec2_cb` at `0x800C4438` |
| `0x80316E20`-`0x80316E98` | `0x74` | `@56054` | referenced by `mov_to_leftlower__10dLineMng_cFUlRC7mVec2_cb` at `0x800C4818` |
| `0x80316E98`-`0x80317028` | `0x190`+pad | 6 framework vtables: `sFStateStateMgr_c`, `sStateStateMgr_c`, `sFStateMgr_c`, `sStateMgr_c`, `sFStateFct_c`, `sFState_c` (`<10dLineMng_c,...>`) | all six addresses (`0x80316E98/ED8/F18/F48/F78/F90`) are stored directly by `__ct__10dLineMng_cFv` itself, `.text 0x800C0DF4`-`0x800C0E9C` — this is the constructor installing its own members' vtable pointers |
| `0x80316FA8`-`0x80317028` | `0x80` | **unattributed, all-zero** | see "Open item" below |
| `0x80317028`-`0x803173AC` | `0x384` (75 × `0xC`) | `@56660`-`@56734`, unnamed | sandwiched, no competing owner in either neighbour (see negative check below); `75 = 3 × 25` states is a strong thematic fit but I did not individually instruction-confirm each of the 75 |
| `0x803173AC`-`0x80317704` | `0x358` (25 strings) | `@56736`-`@56760` | count is exactly 25 — matches the 25 `StateID_*` objects independently found in `.bss` (see below); not individually instruction-confirmed, same sandwiching argument |
| `0x80317704`-`0x80317738` | `0x34` | `__vt__25sFStateID_c<10dLineMng_c>` | referenced by `__sinit` at `.text 0x800C7690` (`lis r30,0x8031/addi r30,r30,0x7704`), hand-verified |

**Negative check for the sandwiched block** (`0x80317000`-`0x80317710`): I
scanned all of items 14-22's `.text` (`0x800BBD80`-`0x800C0DC0`) against this
range and got zero hits, and `d_lytbase.cpp`'s earliest reference into this
neighbourhood is `0x80317738` (see right-edge check below) — i.e. nothing else
reaches in here. Combined with d_line_mng.cpp's own confirmed bracketing on
both sides, I'm treating the sandwiched block as this unit's, but flag that the
75-object array and the 25-string array were bracketed, not individually
proven the way the four `0xC`+vtable+sinit objects were.

**Right edge, doubly confirmed**: `slices/wiimj2d.json`'s landed
`d_lytbase.cpp` claims `.data` `0x19098-0x190e0`, i.e. starting at
`0x80317738` — the exact address where the sFStateID_c vtable ends. I also
found `d_lytbase.cpp`'s own `ReadResource3__9LytBase_cFPCci`
(`.text 0x800C8DB0`-`0x800C8E50`) referencing `0x80317738` directly at
`0x800C8DE4`.

**Left edge, instruction-confirmed on both sides of the boundary**: item 21's
`modelset__23daLiftNormalModelDraw_cFv` (`.text 0x800C0360`-`0x800C0484`)
loads `l_modeldt` via `lis r31,0x8031 / addi r31,r31,0x6998` then a mid-
function rebase `addi r31,r31,0x2a8` landing exactly on `l_modeldt`
(`0x80316C40`, size `0x60`) — and goes no further right. `l_modeldt` ends at
`0x80316CA0`, exactly where `d_line_mng.cpp`'s own `@55792` object starts and
is itself referenced from. (My first pass at this comparison used an X-form-
as-D-form decode bug and produced a false "shared ownership" result reaching
into `0x80316D6C` — corrected once I re-decoded the actual opcodes by hand;
noting this so the correction isn't silently lost.)

**Open item — I could not resolve this**: `0x80316FA8`-`0x80317028` (`0x80`
bytes, immediately after the six-vtable group) is entirely zero bytes and has
**no symbol table entry at all** — not a `lbl_`, not an `@NNNNN`, nothing.
Every other boundary in this section is backed by a named or pool-numbered
object; this one isn't. It sits inside a range I'm otherwise confident belongs
to this unit (bracketed on both sides by confirmed d_line_mng.cpp material),
so I'm reporting it as **this unit's, unattributed** rather than guessing at a
declaration that would produce it. Candidates I did not chase down: reserved/
padding slots in the 75-entry array if its true declared length is larger than
what's populated, or an artifact of how dtk's symbol scraper handles a
run of literal zero words. Flagging per the "guess must be labelled a guess"
rule — I'm not proposing a struct for it.

## `.bss`

**`0x80359100`–`0x80359760`**, offset `0x7780`–`0x7DE0` (base `0x80351980`),
size `0x660` (1632 bytes).

**This corrects `DOL_TARGETS.md` on two counts**: it said 27 `StateID_*`
objects (`0x510` bytes); the true count is **25**, matching the VERIFIED-FACT
brief, and independently reconfirmed here by direct enumeration of the `.bss`
symbol table (`StateID_Idle` through `StateID_Circle4x4RightDown`, counted by
hand, 25 entries — the "27 direction/shape states" language in
`DOL_TARGETS.md`'s prose over-counts, likely by folding the four
`Circle2x2`/`Circle4x4` sub-variants into the state count twice). It also
missed a leading `0xC`-object-per-state pattern and a trailing 8-float table,
both of which I confirmed by instruction reference — true total is `0x660`,
not `0x510` (`0x150`/336-byte undercount).

Structure, each `StateID_*` preceded by its own unnamed `0xC` pool object with
a 4-byte pad between (`0xC`+`0x4`+`0x30` = `0x40` per state × 25 = `0x640`),
plus a trailing `0x20`-byte float table:

- `0x80359100` (`@49614`, `0xC`) through `0x80359740` — 25 × (`0xC` pool
  object + `StateID_<Name>__10dLineMng_c`, `0x30` each). Full list: Idle,
  FallDown, Left45, Right45, Side, Height, CornerHeightLine, CornerSideLine,
  Left30Left, Left30Right, Right30Left, Right30Right, Left60Up, Left60Down,
  Right60Down, Right60Up, Circle, Circle2x2Leftup, Circle2x2Rightup,
  Circle2x2LeftDown, Circle2x2RightDown, Circle4x4Rightup, Circle4x4LeftUp,
  Circle4x4LeftDown, Circle4x4RightDown (25).
- `0x80359740`-`0x80359760` (`lbl_80359740`, `0x20`, 8 floats) — referenced
  twice by `init_term_ck_pos__10dLineMng_cFv` (`.text 0x800C1888` and
  `0x800C18C0`, both `lis r4,0x8036/addi r4,r4,0x9740`).

**Left edge, instruction-confirmed**: the leading `0xC` object at `0x80359100`
is used as a base register in **14 separate places** across
`d_line_mng.cpp`'s `.text` (every `executeState_*`/`initializeState_*`-style
function I sampled loads it), a far stronger signal than the other sandwiched
blocks got. I also confirmed **zero** references from `d_line_mng.cpp` into
`0x803590F0`-`0x80359100` (the preceding object, `dInfo_c`'s own — item 13,
unlanded, so this is my own measurement, not an official cross-check) — clean
break exactly at `0x80359100`.

**Right edge, doubly confirmed**: zero references from `d_line_mng.cpp`'s
`.text` into `0x80359760`+; `d_lytbase.cpp`'s own `.text`
(`0x800C89A0`-`0x800CA080`) references `0x80359760` directly (its own
`@14095`/`s_TagPrc__9LytBase_c` pair, e.g. `0x800C9B20`). And
`slices/wiimj2d.json`'s landed `d_lytbase.cpp` entry claims `.bss`
`0x7de0-0x7eb8`, i.e. starting at `0x80359760` (base `0x80351980`) — exact
match.

## Not asked for, but found: `.sdata2`

`smc_UNIT_SIZE_X__10dLineMng_c` at `.sdata2:0x8042CB18`, size `0x4` (one
float), scope global (not `@LOCAL@`). Noted per `DOL_TARGETS.md`'s mention but
not bounded — out of the four sections this task asked for, and I did not
search for a `.sdata`/`.sbss` footprint at all.

---

## The `__vt__25sFStateID_c<10dLineMng_c>` separation — answered

**It belongs to this unit, not a neighbour**, and here is the mechanism,
measured rather than guessed:

The six vtables at `0x80316E98`-`0x80316FA8` are for `dLineMng_c`'s own
**member** objects (the `sFStateMgr_c`/`sStateMgr_c`/etc. framework instances
it embeds) — I confirmed this directly: `__ct__10dLineMng_cFv`
(`0x800C0DC0`, `dLineMng_c`'s regular constructor) stores all six of those
exact addresses into the new object between `0x800C0DF4` and `0x800C0E9C`.
That is ordinary per-instance vtable installation, done once, at the point in
the source where the class body's members get their vtables assigned.

The seventh, `sFStateID_c<10dLineMng_c>`, is a **different template
instantiation** — the type of the 25 file-scope `StateID_*` static objects in
`.bss`. Those are static-storage-duration objects with non-trivial
constructors, so *their* vtable pointers get installed by
`__sinit_\d_line_mng_cpp` (confirmed: `__sinit` references `0x80317704`
directly at `0x800C7690`), not by `dLineMng_c`'s regular constructor. It is a
categorically different call site.

Between the two groups sits `0x80317028`-`0x80317704`: the 75-entry `0xC`
numeric table and the 25-entry name-string table that (by count and
thematic fit) belong to those same 25 `StateID_*` objects — plausibly
constructor arguments or lookup data feeding them. MWCC lays out a TU's
static-storage objects roughly in declaration/first-use order
(`AGENT_CONTEXT.md` documents this repeatedly for both `.text` and `.data`/
`.bss`). The simplest reading consistent with everything measured here: the
six member vtables are tied to the class's own constructor, emitted early;
the numeric/string support tables and the 25 `StateID_*` objects are declared
later in the file (after the class body, as file-scope statics), and the
`sFStateID_c<10dLineMng_c>` template's vtable — being first odr-used only by
*those* objects' constructors — doesn't get emitted until the compiler
reaches that later point. **The gap is not foreign material and does not
belong to a neighbour**: everything in `0x80316FA8`-`0x80317704` is
bracketed on both sides by confirmed d_line_mng.cpp references and nothing
else reaches into it, with the sole exception of the unattributed `0x80`-byte
zero span noted above, which I'm reporting as unresolved rather than folding
into this explanation.

---

## `memoryRanges` block, ready to paste into `slices/wiimj2d.json` format

```json
{
    "source": "dol/bases/d_line_mng.cpp",
    "memoryRanges": {
        ".text": "0xba640-0xc2220",
        ".ctors": "0x118-0x11c",
        ".rodata": "0x3308-0x3338",
        ".data": "0x18600-0x19098",
        ".bss": "0x7780-0x7de0"
    }
}
```

Overlap-and-adjacency check run against the live `slices/wiimj2d.json`
(script inline, not saved elsewhere): **zero overlaps** against every landed
slice, in every section. Adjacent-neighbour check: exactly abuts
`d_lytbase.cpp`'s claimed `.text`/`.ctors`/`.rodata`/`.data`/`.bss` ranges with
no slack on any of the five, in both directions where a landed neighbour
exists (right side only — nothing on the left of this unit is landed yet).

## Summary: proven vs inferred, edge by edge

| section | edge | status |
|---|---|---|
| `.text` | both | given fact, re-confirmed not re-derived |
| `.ctors` | both | PROVEN (raw array read + landed-neighbour cross-check) |
| `.rodata` | right | PROVEN (instruction ref + landed-neighbour cross-check) |
| `.rodata` | left | MEASURED (no landed neighbour to cross-check) |
| `.data` | right | PROVEN (instruction ref + landed-neighbour cross-check) |
| `.data` | left | PROVEN (instruction ref both sides of the boundary) |
| `.data` | interior (`0x80316FA8`-`0x80317028`) | **UNRESOLVED** — bracketed, unattributed, flagged, not guessed |
| `.data` | interior (75-obj + 25-string block) | MEASURED by bracketing/elimination + thematic count match, not individually instruction-confirmed per object |
| `.bss` | right | PROVEN (instruction ref + landed-neighbour cross-check) |
| `.bss` | left | MEASURED (14-site instruction ref; no landed neighbour to cross-check) |

Corrections to `wip/dol_scout/DOL_TARGETS.md` (that document invited exactly
this kind of check, and labelled its own `.data`/`.bss` claims lower-
confidence than its `.text` bounds):
- `.data`: true size `0xA98`, not `0x1A0` — it named only the vtable cluster
  and missed the four leading per-`mov_to_*` tables, the sandwiched 75+25
  object block, and the unattributed `0x80`-byte gap.
- `.bss`: 25 `StateID_*` objects, not 27; true size `0x660`, not `0x510`.
- `.rodata`: confirmed correct as written, no change.
- `.ctors`: confirmed correct as written, no change.
