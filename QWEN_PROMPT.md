# Work order — round 17

**Read `AGENT_CONTEXT.md` first.** It is the standing briefing for this repo and
it assumes nothing about you. This file is only round 17.

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 16 verified, and it held up

I recompiled your `scratch/round16/d_iggy_wan_kusari.cpp` myself with
`harness.compile_draft`, disassembled it, and compared it against the three
delinked target objects with my own comparator. **Your table reproduces exactly:
34 of 47 byte-exact.** It compiled clean with no errors on the first try.

I then ran a stricter gate than yours. Yours compares raw instruction words only,
and in a delinked target every `bl` is `48 00 00 01` and every `lis ...@ha` is
`3C 80 00 00` — so a *wrong callee* or a *wrong global* compares EQUAL under
bytes alone. I re-ran with the relocation operand text included. **33 of your 34
survived that too**, and the one that flagged
(`createMdl__21dIggyWanKusariPiece_c`) is the known quote artifact, not a real
difference. That is a genuinely good result and the honest DIFF rows for the
stubbed bodies are why I trust the rest of it.

Your constants all check out against the DOL bytes: `smc_LENGTH` = 6.0f at
`0x8042C9A8`, `cs_init_angle` = `0x4000`/`0xC000` at `0x8042C97C`, `cs_dir_prm` =
{1.0f, -1.0f} at `0x8042C9C8`, `cs_mdl_name` pointing at `wanwan_chainA` and
`wanwan_chainB`.

### Two corrections, both worth reading before you start

**1. A wrong string literal that your comparator structurally cannot see.**
Line 208 of your draft has:

```
"g3d/wanwan_boss_iggy.bres"
```

The DOL, read at `0x80315E28`, has **`g3d/wanwan_boss_iggy.brres`** — 26 chars,
not 25. String content lives in `.rodata`, and the `.text` reference to it is a
relocated pointer, so this is invisible to any `.text` comparison and would shift
every `.rodata` object after it by one byte at link time. **Fix it in place in
`scratch/round16/d_iggy_wan_kusari.cpp` as the first thing you do this round, and
say in your response that you did.** Then note the general lesson in your own
words: a `.text` match says nothing about `.rodata` content.

**2. Your `.text` bound correction was wrong, and contradicted your own table.**
You reported the unit ending at `0x800BAB04` and everything above belonging to
`d_info.cpp`. But eight functions in your own per-function table —
`createMdl`, `calcMdl`, `draw`, `calcForDemo`, `calcPosAngle`, `collapseMove`,
`setCollapseSpeed` on `dIggyWanKusariPiece_c`, plus `__dt__` and `isSameName` on
`sFStateID_c` — live above `0x800BAB04`. Measured:

```
setCollapseSpeed__21dIggyWanKusariPiece_cFi = .text:0x800BB060  size 0x74  -> ends 0x800BB0D4
__ct__7dInfo_cFv                            = .text:0x800BB0E0
```

The original bound `0x800B90A0 - 0x800BB0E0` was correct. `0x800BAB04` is a
*split-object* seam, not a unit edge — the same class of confusion as
`dtk_splits` versus the full symbol map.

**3. Your `.bss` figure understates the unit.** You reported "six `StateID_*`
objects, `0x80358ED8`-`0x80359018`, 6 x 0x30 = 0x120" as PROVED. That was the
figure I asserted in the round-16 order and you restated it rather than measuring
it. The real run interleaves an anonymous `0xC` object after each state:

```
0x80358ED8 StateID_Ready   0x30      0x80358F08 @62489  0xC
0x80358F18 StateID_Normal  0x30      0x80358F48 @62493  0xC
0x80358F58 StateID_Tight   0x30      0x80358F88 @62497  0xC
0x80358F98 StateID_Release 0x30      0x80358FC8 @62501  0xC
0x80358FD8 StateID_Collapse 0x30     0x80359008 @62505  0xC
0x80359018 StateID_Dead    0x30      0x80359048 @68605  0xC
0x80359054 m_startInfo__7dInfo_c  <- next unit starts here
```

Real extent `0x80358ED8`-`0x80359054`, **`0x17C`**, not `0x120`. This is exactly
the failure mode the round-16 order warned you about with the neighbouring unit's
figure being wrong by a factor of six — and it arrived because something I
asserted came back to me marked PROVED. **Treat every figure I hand you as a
hypothesis, including in this file.** Saying "I could not measure this" is worth
more to me than an echo.

The Iggy unit is otherwise parked: its five stubbed bodies need `daEnIggy_c`'s
member layout, which Gemini is working the boss-class side of this round. You are
not on Iggy this round beyond the one-line string fix.

---

## Your task: author `d_bg_actor_mng.cpp`

A DOL unit (`wiimj2d.dol`), not a REL. A background-actor manager: a singleton
that owns a rail list and spawns background objects.

**This unit was carved by a peer last round and I verified every edge of it
myself, object by object.** The bounds below are measured, not inferred — but
check them anyway, because that is how the round-16 `.bss` error would have been
caught.

```
.text    0x8007E180 - 0x8007F7A0   (0x1620 = 5,664 bytes)   22 functions
.ctors   0x802EDD94 -> __sinit_\d_bg_actor_mng_cpp @ 0x8007EC20  (size 0xAB4)
.rodata  0x802EFC68 - 0x802EFC98   (0x30)    one pool object @68155
.data    0x8030F820 - 0x80310068   (0x848)
.sbss    0x8042A0B8 - 0x8042A0C0   (0x8)
.sdata2  0x8042C130 - 0x8042C180   (0x50)    17 pool constants
.bss     NONE — the four anonymous .bss objects in this region belong to d_bg_unit
```

**The `.data` and `.sdata2` edges above include trailing alignment padding, and
you should see why rather than take my word for it.** The last named object in
each ends earlier than the bound:

```
.data    last object __vt__17dBgActorManager_c ends 0x80310064
         next unit's first object @66816 starts  0x80310068   -> 4 bytes padding
.sdata2  last object @71467 ends                 0x8042C17C
         next unit's first object @68048 starts  0x8042C180   -> 4 bytes padding
```

A bound taken from "where the last object I can name ends" is a **lower bound**,
and a slice cut there drops the padding and shifts the next unit. I made exactly
this mistake drafting this file and caught it on re-check. Confirm both edges
yourself.

### Why this unit was chosen for you

**All 22 functions carry real mangled names. Zero anonymous `fn_*`.** Every
parameter list is handed to you by the mangling; only return types are open.
Two classes, the second nested inside the first:

```
0x8007E180  0x50   __ct__17dBgActorManager_cFv
0x8007E1D0  0xEC   __dt__17dBgActorManager_cFv
0x8007E2C0  0x108  initialize__17dBgActorManager_cFv
0x8007E3D0  0x58   create__17dBgActorManager_cFv
0x8007E430  0xB0   CreateHeap__17dBgActorManager_cFv
0x8007E4E0  0x40   execute__17dBgActorManager_cFv
0x8007E520  0x2CC  ProcMain__17dBgActorManager_cFv
0x8007E7F0  0x6C   addObj__17dBgActorManager_cFUsUsUsUc
0x8007E860  0x1D0  createObjList__17dBgActorManager_cFb
0x8007EA30  0x20   init__Q217dBgActorManager_c7BgObj_cFv
0x8007EA50  0x4    clear__Q217dBgActorManager_c7BgObj_cFv
0x8007EA60  0x14   set__Q217dBgActorManager_c7BgObj_cFUsUsUsUc
0x8007EA80  0xF0   createActor__Q217dBgActorManager_c7BgObj_cFUlR7mVec3_c
0x8007EB70  0x44   deleteActor__Q217dBgActorManager_c7BgObj_cFv
0x8007EBC0  0x2C   getOffset__Q217dBgActorManager_c7BgObj_cFv
0x8007EBF0  0x24   getSize__Q217dBgActorManager_c7BgObj_cFv
0x8007EC20  0xAB4  __sinit_\d_bg_actor_mng_cpp
0x8007F6E0  0x1C   __arraydtor$67758
0x8007F700  0x40   __dt__Q217dBgActorManager_c11BgObjName_tFv
0x8007F740  0x1C   __arraydtor$67764
0x8007F760  0x1C   __arraydtor$67766
0x8007F780  0x1C   __arraydtor$67768
```

Note the shape this tells you outright. `__arraydtor$` thunks and a
`BgObjName_t` destructor mean **arrays of a non-trivially-destructible struct at
file scope** — that is what the `0xAB4` `__sinit_` is doing, and it is where most
of the unit's bytes are. The `.data` side of it is already named:

```
0x8030F820  0x40   l_object_name
0x8030F860  0x380  l_Pa3_rail
0x8030FBE0  0x260  l_Pa3_MG_house_ami_rail
0x8030FE40  0x1A0  l_Pa3_daishizen
0x8030FFE0  0x14   l_rail_list
0x8030FFF4  0x9 / 0xF / 0xE / 0x16 / 0x1F   five anonymous string pool objects
0x80310058  0xC    __vt__17dBgActorManager_c
```

`__vt__17dBgActorManager_c` is `0xC`, which is one virtual slot plus the two
header words. One virtual method on the manager, not more.

**Do the file-scope array declarations and the singleton before you hand-author a
single method body.** On other units here, getting the declaration framework
right has emitted eight, twenty, and most recently sixty-seven functions for free
— byte-exact, none hand-written. The four `__arraydtor$` thunks and the
`BgObjName_t` destructor are all compiler-synthesised: you do not write them, you
cause them.

`bin/dtk/wiimj2d_symbols.txt` is the FULL DOL symbol map and is the richest
source you have. `syms.txt` is a small curated list; absence from it proves
nothing.

---

## Six things that have each cost this project a round

**1. DOL flags, not REL flags.** These are different programs:

```
wiimj2d    -O4                                      (small data ON)
d_basesNP  -O4,p  -sdata 0  -sdata2 0  -char signed
```

You are on the DOL. **Call `harness.compile_draft(src, obj)` from
`tools/auto_decomp/harness.py`** and let it supply them. Never hand-build the
command line — seven mandatory include paths, and people have lost rounds to one
missing path.

**2. Return types are ABSENT from CFront mangling.** Well over two dozen wrong
declarations have been found on this project so far, and the count is still
climbing. The method that finds them every time: **read what the
CALLER does with the return register immediately after the `bl` — does it READ
r3, or CLOBBER it?** An observed clobber outranks any analogy with a sibling.
`getOffset` and `getSize` on the nested class are the obvious candidates here and
their names are not evidence of anything.

**3. An argument-count mismatch at a call site is a STORAGE-CLASS tell**, not a
return-type one. One register set where two are expected means no implicit
`this` — the function is `static`.

**4. Check the SIZE before you count differences.** A length mismatch is CONTENT
in both directions. A pool-position or register residual physically cannot change
an instruction count. **Always report length before any differing count.**

**5. A `.data` block that looks like a constant table may be a SWITCH JUMP
TABLE.** Suspect a `switch` before you transcribe constants. Here the reverse
risk also applies: `l_Pa3_rail` and friends genuinely are data, and their
*contents* must be read out of the DOL, not invented.

**6. Know when to stop.** If a function reaches the correct instruction count and
differs only in register numbers, stop and report the count. Declaration order
has been measured not to influence register assignment at all.

## A tooling caveat, and a second one you found the hard way

`harness.canonicalise` **reports false mismatches** when the target's
disassembly quotes a symbol name and a standalone `.o` does not. If a function is
length-exact and the comparator still says differ, compare the raw instruction
BYTES. `wip/line_mng_shared/tally.py` implements the correct union gate.

And from your own round: **byte comparison of `.text` proves nothing about
`.rodata`, `.data` or `.sdata2`.** Relocated operands are zero on both sides, so
a wrong string, a wrong float, or a wrong callee can all pass. This unit is
data-heavy — most of its bytes are in those sections. **Compare the emitted data
sections against the DOL directly and report that comparison as a separate
result from the function table.** That is the single most valuable thing you can
add to your method this round.

## Rules

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`. I am the only
  integrator and two builds in this checkout clobber each other.
- **Never edit a shared header, `slices/*.json`, or `syms.txt`.** Shadow-copy the
  header into your own include directory, prove your change there, and put the
  diff in your response as a proposal.
- Work only in `scratch/round17/`, plus the one-line string fix in
  `scratch/round16/d_iggy_wan_kusari.cpp`. Do not touch `wip/`, `HANDOFF.md`,
  `AGENT_CONTEXT.md`, `peer_archive/`, or `GEMINI_*.md`. **Gemini is settling the
  `dEnBoss_c` vtable this round** — stay off the enemy/boss region entirely.
- **Name your draft file `d_bg_actor_mng.cpp`.** Anonymous-namespace symbols
  mangle the source filename into them.
- Extract by ADDRESS and assert `instruction_count * 4` against the symbol map
  before writing any C++.
- Mark anything unproven `@unofficial`. A `u8 pad[N]` for a region you cannot
  explain is a good answer; an invented member name is not.
- **Report a negative result rather than manufacturing a positive one.**

## Deliverable

`QWEN_RESPONSE.md`, containing:

1. **A per-function table** — name, target length, your length, differing count,
   MATCH or not. Length column first.
2. **A separate `.data`/`.rodata`/`.sdata2` comparison** against the DOL bytes,
   reported independently of the function table. New this round; see above.
3. **How many functions the file-scope declarations alone emitted**, before you
   hand-authored anything — including the four `__arraydtor$` thunks and the
   `BgObjName_t` destructor. Report this separately; it is the number I most want.
4. The proposed classes and header in a fenced block, offsets argued from
   evidence.
5. Your source in a fenced block.
6. Every variant you tried and its result, so nobody repeats it.
7. Whether the bounds above survived your check, per section, PROVED versus
   inferred stated separately. **Measure them; do not restate mine.**
8. Confirmation of the `.brres` fix in the round-16 draft.
9. Anything you could not settle, plainly, with what would settle it.

A table of twelve matches and ten characterised residuals is a better round than
twenty-two claimed matches I cannot reproduce. **I check every number
independently** — I recompiled your entire round-16 draft to do it, and it came
back clean.

Plain ASCII or clean UTF-8, LF, no BOM.
