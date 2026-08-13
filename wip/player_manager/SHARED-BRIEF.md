# Shared brief — `d_a_player_manager.cpp` (`daPyMng_c`)

Every agent on this unit reads this file. Your own brief adds only what is
specific to you.

## The target

`source/dol/bases/d_a_player_manager.cpp`, class `daPyMng_c`.

| Section | Range (offset in section) | Absolute | Confidence |
|---|---|---|---|
| `.text` | `0x58220-0x5AC30` | `0x8005E9A0`–`0x800613B0` | **Exact** — the auto object covers `0x8005E9A0..0x80061310` and `__sinit` occupies `0x80061310`+`0x9C` |
| `.ctors` | `0x88-0x8c` | — | Exact by subtraction, one free slot |
| `.bss` | `0x3790-0x4640` | `0x80355110`–`0x80355FC0` | Exact by subtraction |
| `.sbss` | **`0xe0-0x138`** | `0x80429F80`–`0x80429FD8` | **CORRECTED — see below** |
| `.sdata` | `0x280-0x290` | `0x80427C00`–`0x80427C10` | Exact, self-attributing |
| `.rodata` | `0x1628-0x1638` | `0x802EF608`–`0x802EF618` | Exact — **one object only** |
| `.data` | `0xb388-0xb3b8` | `0x80309A28`–`0x80309A58` | **RESOLVED — ours. See below** |
| `.sdata2` | `0x9e8-0xa20` | `0x8042BD48`–`0x8042BD80` | **RESOLVED — ours, and larger than first thought** |

### Corrections and resolutions from the recon round — read these

**`.sbss` was recorded wrong, by `0x28` bytes.** The handoff said `0xe0-0x110`
(`0x30`). The true claim is **`0xe0-0x138` (`0x58`)**, `0x80429F80`–`0x80429FD8`.
Two agents found this independently and the lead confirmed it directly against
the symbol map. The section base is `0x80429EA0`. The high side is hard-bracketed:
`d_actor.cpp`'s `.sbss` starts at exactly `0x80429FD8`. Nine members were missing
from the old bound — `mPauseEnableInfo`, `mPauseDisable`, `mStopTimerInfo`,
`mStopTimerInfoOld`, `mQuakeTrigger`, `mBgmState`, `mBonusNoCap`,
`mKinopioCarryCount`, and one unnamed byte — **and every one of them is
referenced by name from our own `.text`.** This is the exact "wrong small-data
bound, never wrong code" signature that fails four of five binaries with
thousands of scattered single-byte diffs. It is fixed now; do not re-derive it
from the handoff.

**`lbl_80429FD0` is ours, is unnamed, and nobody has claimed it.** One byte, the
last object in our `.sbss`, read at `0x80060D04` (`lbz`) and written at
`0x80060D30` (`stb`) — both inside `setHipAttackQuake`. It carries no class
mangling, so it is a **file-scope static in the `.cpp`**, not a class member.
Whoever owns `setHipAttackQuake` names and defines it. If nobody defines it the
section comes up short.

**`.data` and `.sdata2` ownership is RESOLVED, and the answer is "ours".** This
was Codex's question; our own recon answered it first, with reference evidence
rather than elimination. `fn_80060DB0` — unambiguously ours — directly loads both
`.data` strings and the `.sdata2` float, and the three pool IDs are
**consecutive**: `@81204` (`.data 0x80309A28`), `@81205` (`.sdata2 0x8042BD7C`),
`@81206` (`.data 0x80309A3C`). Consecutive pool IDs spanning two sections is the
strongest attribution evidence this project has. The strings are
`"Wm_mr_vshipattack"` and `"Wm_mr_vshipattack_ind"`; the float is `3800.0f`.
`.sdata2` is also **larger than the handoff thought** — `0x8042BD48`–`0x8042BD80`,
six further objects referenced by `getPlayerSetPos`, `createCourseInit`,
`deleteCullingYoshi` and `fn_8005F4D0`.

**There are no sibling twins, and that is a real finding rather than a gap.**
Every `daPyMng_c` static member has a name unique to this class, and the
comparator only canonicalises *pool* references, not named globals — so no
external function can ever canonicalise equal to one of ours. Seven candidate
pairs were tested and rejected; they are listed in `MAP.md` so nobody chases
them. **Author from the disassembly.** `MAP.md`'s per-function table carries the
statement order, the callees and the members touched for all 67 functions, which
is the substitute.

**`.ctors` is a three-slot gap, not one free slot.** `0x802EDD60`/`64`/`68`, and
**ours is the last of the three** (`0x802EDD68`), directly adjacent to
`d_a_right_base.cpp`. The other two most plausibly belong to
`d_a_player_hio_ADJ.cpp` and `d_a_player_demo_manager.cpp`.

**New tool worth knowing: `bin/dtk/dtk_splits_wiimj2d.txt`.** An official
per-source-file section range list for already-split TUs. Where one of our bounds
is adjacent to an entry in it, that is address data about the *original*
binary — the strongest bracketing evidence available, and better than any
derivation by elimination. It is what settled `.sbss`, `.rodata` and `.bss`.

Section bases: `.text` `0x80006780`, `.rodata` `0x802edfe0`, `.data` `0x802fe6a0`,
`.bss` `0x80351980`, `.sdata` `0x80427980`, `.sdata2` `0x8042b360`,
`.ctors` `0x802edce0`.

**The class is all-static and has no vtable.** There is no object layout to
reconstruct — every static member is a named symbol with a size in the map. This
is why the unit was picked; do not spend effort looking for a vtable.

## The target disassembly is already extracted — do not re-derive it

| File | Contents |
|---|---|
| `wip/player_manager/target_text.txt` | All 68 functions, `0x8005E9A0..0x80061310` |
| `wip/player_manager/target_rodata.txt` | `.rodata` neighbourhood from `0x802EEEA0` |
| `wip/player_manager/target_data.txt` | `.data` neighbourhood from `0x80309908` |
| `wip/player_manager/target_bss.txt` | `.bss` neighbourhood from `0x80354F20` |
| `wip/player_manager/target_sdata.txt` | `.sdata` neighbourhood from `0x80427BE8` |
| `wip/player_manager/target_sdata2.txt` | `.sdata2` neighbourhood from `0x8042BCF8` |

`__sinit_\d_a_player_manager_cpp` at `0x80061310` (`0x9C`) is in a *later* auto
object; disassemble `bin/dtkspl/obj/auto_03_80061310_text.o` if you need it (use
an **absolute** path to `bin/dtk-windows-x86_64.exe` — relative paths fail here).

## What is NOT ours this round

Codex (a peer AI working the same tree) owns these. **Do not work them, do not
answer them, do not edit the files involved.** If you find evidence bearing on
them, *report it* and keep going:

1. The `sizeof` of `dMultiMng_c` (`0x5C`), `dAttention_c` (`0x58`) and
   `dPyEffectMng_c` (`0xC5C`) — three classes `daPyMng_c` embeds by value.
   Their headers are stubs today and will change under you. **Assume the sizes
   above are correct** and write code against them.
2. Ownership of `.data 0x80309A28-0x80309A58` (two strings) and
   `.sdata2 ~0x8042BD78`.

## The three hazards, so you recognise them if you hit one

1. **Four static class instances embedded by value in `.bss`**, each followed by
   a `0xC` destructor-chain record: `mDemoManager` (`0x98`, `daPyDemoMng_c` —
   proven and landed), `mMultiManager` (`0x5C`), `mAttention` (`0x58`),
   `mEffectMng` (`0xC5C`). If a size is wrong the whole `.bss` shifts, and **no
   per-function diff can see it.**
2. **Two foreign weak inline copies sit mid-range** and are emitted *inside* our
   `.text`: `getCourseIn__10dScStage_cFv` (8 B, `0x8005EC90`) and
   `getFileP__5dCd_cFi` (0x20, `0x8005EE70`). **Nobody authors these.** They
   appear only if the right header is included and the right call is made, in
   the right place. If one comes out missing or misplaced, that is a finding —
   report it, do not hand-write it.

   **RESOLVED BY THE LEAD — both headers are already fixed and landed. Read
   this before you touch either call site:**

   Each symbol has **exactly one copy in the entire binary**, and both copies
   are in our range. Both are also genuinely **called** with a `bl`, not merely
   flushed: `bl getCourseIn` at `0x8005EBA0` (inside `initStage`) and
   `bl getFileP` at `0x8005F0AC` (inside `createCourseInit`).

   - **`dScStage_c::getCourseIn()`** did not exist in `d_s_stage.hpp` at all. It
     is now declared `static NOINLINE bool getCourseIn() { return m_isCourseIn; }`,
     matching the `getExitMode()` precedent two lines above it, along with
     `static bool m_isCourseIn` (`.sbss:0x8042A4FC`). Committed; five binaries
     verified. When our TU odr-uses `m_isCourseIn` it will need a `syms.txt`
     entry at `0x8042A4FC` — **the lead adds that, not you.**
   - **`dCd_c::getFileP(int)`** was declared as a **non-static** member and was
     wrong. The out-of-line copy takes the index in `r3` and loads
     `dCd_c::m_instance` itself; a non-static member's out-of-line copy would
     take `this` in `r3` and the index in `r4`. It is now `static`. CFront
     mangling does not mark static members, which is why the symbol name could
     never have shown this. Committed; five binaries verified, including
     `d_cd.cpp`'s own slice and all ~20 banked call sites.

   **The open question, and it belongs to whoever authors `createCourseInit`:**
   `getFileP` is *inlined* at every one of its ~20 banked call sites (that is
   why only one out-of-line copy exists) but *called* from `createCourseInit`.
   Same source, different inlining outcome. **`NOINLINE` is therefore NOT the
   answer** — marking it `NOINLINE` would make all 20 banked callers emit `bl`
   and break them. The likely mechanism is MWCC's per-caller inline budget:
   `createCourseInit` is `0x580` bytes, the largest function in the TU, and
   MWCC stops inlining once a caller grows past its threshold. Write the call
   normally. If it inlines when it should not, **that is evidence about the
   surrounding function's size or shape, not about the header** — report it
   rather than reaching for an attribute.
3. `.data` / `.sdata2` attribution — Codex's, above.

## Rules for every agent on this unit

- **Deliverable is a file under `wip/player_manager/`** plus a summary in your
  reply. Do not edit anything outside `wip/player_manager/` and your own
  `scratch/` area.
- **Never run `ninja`, `configure.py`, `progress.py`, or `land.py`.** The lead is
  the only integrator; two `ninja` runs in this checkout clobber each other.
- **Never edit `slices/wiimj2d.json`, `syms.txt`, or any shared header.** If you
  believe a shared header is wrong, shadow-copy it into your scratch, prove the
  change there, and **report** it.
- **Report contradictions; do not reconcile them.** If what you find contradicts
  this brief, the symbol map, or another agent's finding, stop and say so. This
  instruction has repeatedly caught errors that were invisible to every diff.
- **Do not claim MATCHING unless the diff printed nothing**, stated explicitly,
  per function. A well-characterised near-miss is more useful than a false pass;
  a false pass has cost this project a full day.
- **Report every data object with its section** — strings, floats, statics.
  Integration time goes here, not into the functions.
- Windows: dtk relative paths with forward slashes fail; PowerShell 5.1 parses
  8-hex-digit literals as negative Int32, so do address maths in Python; splat
  native-exe arguments.

## Compiling and diffing a draft, without the shared build

```
compilers\Wii\1.1\mwcceppc.exe -c -proc gekko -fp hard -O4 -inline noauto
  -Cpp_exceptions off -enum int -RTTI off -ipa file -enc SJIS -DREVOLUTION -I-
  <scratch>\draft.cpp -o <scratch>\draft.o
  -i include -i include\lib -i include\lib\MSL -i include\lib\MSL\internal
  -i include\lib\revolution\BTE\include -i include\lib\revolution\BTE\stack\include
  -i include\lib\revolution\BTE\stack\btm -i include\lib\revolution\BTE\bta\include
  -i include\lib\revolution\BTE\bta\sys -i include\lib\revolution\BTE\gki\common
  -i include\lib\revolution\BTE\gki\platform
```

The seven `BTE` paths are **mandatory** — without them anything including
`d_audio.hpp` fails, which is most of this file.
`tools/auto_decomp/harness.py`'s `compile_draft(src, obj)` already encodes all of
this; **call it rather than reproducing the command line.** Then
`bin\dtk-windows-x86_64.exe elf disasm <obj> <txt>` and compare with
`harness.extract` / `harness.diff_fn` — import them, do not write your own.

`tools/unit_verify.py` is the unit-level comparator (two views: canonicalised
text **and** raw instruction words, plus a `--neg` negative control). It has a
standing blind spot it prints on every run: **it cannot see a wrong constant.**

## Two findings from batch B5, WITH THEIR SCOPE

**0. CONFIRMED BY PROBE — write the natural array access, never pointer
arithmetic.**

The base-anchor effect below is real and it is now measured, not theorised.
Compiling twelve `int[4]` statics **with their definitions present** plus a
function containing several separate loops makes MWCC emit:

```
lis  r31, ...bss.0@ha
addi r31, r31, ...bss.0@l
addi r4,  r31, 0xa0        <- m_quakeTimer
addi r5,  r31, 0xb0        <- m_quakeEffectFlag
addi r4,  r31, 0x40        <- mPlayerEntry
addi r5,  r31, 0x50        <- mPlayerType
```

Those are **exactly** the target's offsets. The anchor is the start of the TU's
`.bss`, which in the assembled file is `m_playerID`.

Two conditions are required, and a smaller probe with only two arrays and one
loop does **not** reproduce it — so an isolated batch draft cannot:

1. the static **definitions** must be present in the TU, and
2. there must be enough arrays and uses for MWCC to prefer one anchor.

**Therefore: write `mRest[i]`, `m_quakeTimer[i]`, `mPlayerType[i]` normally.**
Do **not** write `(char *) m_playerID + 0x80` or any equivalent pointer
arithmetic to force the shape — it produces the right instructions in isolation
and the wrong source, and the lead will have to undo it. One batch did this and
flagged it honestly as possibly-not-the-original; it is not the original.

**1. A register-allocation-only difference may be unfixable in isolation on this
unit, and that is not your failure.**

B5's `getNumInGame` and `getCoinAll` are logic-verified — every load, compare,
shift and add present, same order, same values — and differ from the target only
in register allocation. The cause: **the target anchors several adjacent `.bss`
arrays off `m_playerID`'s own relocation**, reaching `mPlayerEntry` as `0x40(r31)`
and so on, *including arrays the function never reads*. A standalone draft cannot
reproduce that, because MWCC only shares one base register across several objects
once it can see all their definitions in a single TU — which does not happen
until the lead assembles the file.

**Scope, stated precisely:** this applies to functions that touch **two or more
of the `.bss` arrays in the `0x80355110`–`0x803551D0` block**. It does not apply
to functions touching one array, and it does not apply to `.sbss` scalars, which
are `@sda21`-addressed and need no anchor.

So: if a function is otherwise identical and differs only in which register holds
what, **say so and stop** — do not restructure working logic to chase it. Report
it as "logic-verified, register allocation only, suspected base-anchor artifact"
and the lead will re-diff it after assembly. A forced match here would be a real
regression.

**2. Return types in the header are guesses, and three have now been wrong.**

`fn_8005f4d0` (`void` → `bool`), `addNum()`/`decNum()` (`bool` → `void`), and
`changeItemKinopioPlrNo` (`void` → `bool`). CFront omits return types from
mangling, so no symbol comparison can ever catch one — the *only* signal is
register allocation, and it is a real signal:

> Declared `bool`, MWCC reserves `r3` for the eventual return and allocates a
> temp into `r4`. Declared `void`, the same temp lands in `r3`.

**The method that settled all three: compile the function BOTH ways and let the
diff decide.** Do not argue it from the disassembly. If a function of yours is
close but differs in low register numbers, try the other return type before you
touch the logic. Report the finding; do not edit the shared header.

## House style

`source/dol/bases/d_a_player.cpp` and `d_a_player_base.cpp` are the two banked
neighbours that call into `daPyMng_c` constantly — they are the best style
reference *and* the best evidence about its signatures.
`source/dol/bases/d_a_player_demo_manager.cpp` landed 51/51 last session and is
the closest structural sibling (all-static-ish manager, same family).
