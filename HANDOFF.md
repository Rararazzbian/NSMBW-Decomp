# Handoff

Working notes for continuing the decompilation work on branch
`claude/game-decompilation-setup-bw30s7`.

---

# START HERE: the parallel work plan

**Strategy in one line:** stop pushing on Revolution SDK code (it stalls on
register allocation with no known lever) and drain game code in `wiimj2d.dol`,
where the recently-completed `d_wm_csvdata` work hit **no such wall at all**.

## What parallelises, and what does not

| Stage | Parallel? | Why |
|---|---|---|
| Authoring a function | **Yes** | Each agent writes its own `.cpp` in scratch, compiles it standalone, and diffs against the target disassembly. No shared state. |
| Reconstructing a class layout | **No** | It is the shared prerequisite for every function in the TU. Parallel agents would invent conflicting layouts. One agent, first. |
| Banking (slice + full build + verify) | **No** | `slices/wiimj2d.json`, `syms.txt`, `bin/` and `ninja` are all shared. One integrator, serially. |

**The rule that follows:** agents *author*, the lead *integrates*. Never let two
agents run `ninja` in the same checkout — they clobber each other's objects and
each other's diffs.

### How an agent iterates without the shared build

An agent does not need `configure.py`/`ninja` to check one function. Compile the
file standalone and diff the disassembly:

```bash
compilers\Wii\1.1\mwcceppc.exe -c -proc gekko -fp hard -O4 -inline noauto \
  -Cpp_exceptions off -enum int -RTTI off -ipa file -enc SJIS \
  <scratch>\draft.cpp -o <scratch>\draft.o -DREVOLUTION -I- \
  -i include -i include\lib -i include\lib\MSL -i include\lib\MSL\internal
.\bin\dtk-windows-x86_64.exe elf disasm <scratch>\draft.o <scratch>\draft.txt
python <scratch>\fndiff.py <target.txt> <scratch>\draft.txt <FunctionName>
```

Build the argument list as a PowerShell array and splat it (`& $exe @args`);
long inline arg lists are fragile.

## Read this first if you are picking the project up

- **Position: 9.681%** (629,272 / 6,500,368); `wiimj2d.dol` **18.891%**. Five
  binaries verify, tree clean, `master` and the working branch both pushed.
- **Next target is `d_a_en_lkuribo_base.cpp`** — pre-surveyed, bounds and vtable
  already verified. See its section below. `d_a_en_dfpakkun.cpp` is also ready.
- **Do not re-derive the technique rules.** They cost ~4,000 agent tool calls to
  establish. The levers list and the two whole-binary failure signatures below
  are the most valuable part of this file.

### Infrastructure state (as of the 2026-08-12 session)

- `tools/progress_page/make_progress_page.py` renders a local treemap from the
  **working tree**, including uncommitted work, and pulls upstream's public
  figures from `https://decomp.dev/{owner}/{repo}.json` for comparison. No
  token, no CI. This is the fastest way to see where things stand.
- A self-hosted decomp.dev lives at `C:\Users\Razz\Documents\Projects\decomp.dev`
  (see its `START-HERE.md`). Upstream's project loads in it; **this fork does
  not**, and cannot until CI produces a report artifact.
- **CI is blocked**, and not by our code. The `ci` job dies at ~18s on
  "Download and decrypt blob" because the fork has no `BLOB_URL` /
  `BLOB_PASSPHRASE` secrets. Those come from a token at
  `rootcubed.dev/decomp-token`, whose submission endpoint was returning **500**
  on 2026-08-12. Retry later; nothing else is missing. `workflow_dispatch` has
  been added so a re-run needs no throwaway commit.
- The `deploy` job also fails (404) because GitHub Pages is not enabled on the
  fork. Harmless — decomp.dev only reads the report artifact.
- **`objdiff.json` over-reports.** It marks a unit complete whenever a source
  file exists, including `nonMatching` slices, so it currently calls
  `d_a_en_dpakkun_base.cpp` complete at 60/64. `progress.py` is the authority;
  the local progress page corrects for this, decomp.dev will not.

## Where the work now stands

**9.681%** (629,272 / 6,500,368 bytes); `wiimj2d.dol` at **18.891%**. Five
binaries verifying, working tree clean.

The most recent session took the project from 9.133% to 9.681% across two
batches of parallel agents. What landed, all banked *whole*:

| TU | Functions | Notes |
|---|---|---|
| `d_a_fireball_base.cpp` | 51 | Validated the guessed header of its own derived TU |
| `d_a_en_net_nokonoko_base.cpp` | 37 | Verified by raw instruction-word compare |
| `d_a_enemy_ice.cpp` | 37 | Derives from `dActorState_c`, **not** `dEn_c` |
| `d_a_rot_objs_base.cpp` | 31 | Range turned out to be two TUs |
| `d_a_spin_child_base.cpp` | 23 | |
| `d_a_sink_dokan.cpp` | 14 | **Was not on any target list** |
| `d_a_cursor.cpp` | 9 | Smallest TU in the project |
| `d_a_en_kuribo_base.cpp` | 66 | Goomba base; pre-surveyed its sibling on the way past |
| `d_a_en_door.cpp` | 50 | Closed on a coupled pair after ~300 failed builds |
| `d_a_rot_block.cpp` | 5 | **Was not on any target list** |

Plus `d_a_en_dpakkun_base.cpp` at **60/64**, landed `nonMatching` — see the wall
note below. Its header is fully resolved and validated, so it is ready to serve
`d_a_en_dfpakkun.cpp` even before the last four functions close.

**Three TUs were discovered mid-batch** that our enumeration cannot see, and the
`.text` end boundary was wrong on **five** separate assignments, always the same
way. Both failure modes and their fixes are documented below — read those two
sections before assigning anything.

### Earlier session, for context

8.475% → 8.823%: `d_wm_csvdata.cpp` (41), `d_a_en_super_bigpile.cpp` (46, the
file the actor playbook was written from), `d_tag_processor.cpp` (39).

### THE lesson: authoring is cheap, section bounds are not

In all three files the **code was essentially right on the first full build**.
Every hour after that went into the slice's `memoryRanges`. Plan for this.

**Establish the full set of sections a TU needs before splitting work up.**
`.ctors`, `.bss` and `.sbss` are easy to omit entirely and were missing from the
first `d_wm_csvdata` attempt. Read the true bounds out of
`bin/dtk/wiimj2d_symbols.txt` by listing the symbols on **either side** of the
TU's objects — the neighbours pin down both ends. A previous TU's slice ending
exactly at your TU's start in every section is strong corroboration.

**Diagnose in this order, cheapest first:**

1. Compare **section sizes**. A short `.text` means a missing function.
2. Compare **per-function sizes** against the symbol map. If they all match, the
   code is right and the problem is placement — stop reading instructions.
3. Only then compare bytes.

**Scattered single-byte diffs spread across the whole binary, at odd addresses,
thousands of functions away from your file, mean a wrong small-data bound — never
wrong code.** A 4-byte shortfall in one `.sdata2` bound produced ~6,800 diffs,
none of them in the file being worked on. This symptom cost hours the first time
and one step the third time.

Useful constants for decoding SDA references:
`_SDA_BASE_` (r13) = `.sdata` base + 0x8000 = **0x8042F980**;
`_SDA2_BASE_` (r2) = `.sdata2` base + 0x8000 = **0x80433360**.
Decoding the r2 offsets in a target `__sinit` settled a `.sdata2` layout question
in one minute that had absorbed two hours of theorising.

### Whole-TU or nothing

A slice gets **one contiguous range per section**, so a TU with any data object
in the middle of its range cannot be partially banked. Both `d_wm_csvdata` and
`d_tag_processor` hit this: in the latter, `preProcess`'s jump table sits between
another function's table and the vtable in `.data`, so dropping any function
after `getScissor` would leave a hole. Budget for the whole file up front.

Also: **the TU may be bigger than the plan says.** `d_tag_processor` was recorded
as 38 functions / 7,380 B; it is 39 / 8,384. `preProcess` began at exactly the
address the file was thought to end, and was found only because an agent noticed
a 0x50 block in `.data` that nothing in the file referenced. Verify the end of a
TU by checking what follows it, not by trusting a prior note.

## Next targets

`tools/find_targets.py` still ranks by header coverage. **Do not follow it
blindly**: its two top-ranked runs (`0x801C8570`, 2,464 B, 100% coverage;
`0x801C91E0`, 2,904 B, 95%) are both **gated by the two known register-wall
functions**. `GXSetTevColor` sits at the head of `GXTev.c`'s queue and
`GXGetViewportv` immediately before `GXTransform.c`'s banked start, so extending
backwards must cross it. One contiguous range per section means neither can be
skipped. High header coverage says the *types* exist, not that the run is
reachable.

**The best next move is the remaining ~29 actor TUs**, using the playbook below.
`d_a_en_super_bigpile.cpp` was run specifically to price that pattern, and it
matched 46/46 on the first integration.

### The remaining actor TUs, with what is known about each

Regenerate with `scratchpad/tu_extent.py`; cross-check with `tools/tu_split.py`.
Sizes are `.text` bytes. Annotations are hard-won — read them before assigning.

| TU | Bytes | Fns | Notes |
|---|---|---|---|
| `d_a_en_dfpakkun` | 9,688 | 59 | **Ready** — its base's header is resolved and validated |
| `d_a_en_jimen_pakkun_base` | 7,896 | 58 | Pakkun family; likely shares idioms with the base |
| `d_a_en_lkuribo_base` | 9,552 | 75 | **Pre-surveyed, ready** — see below |
| `d_a_en_bros_base` | 12,072 | 97 | Largest clean base |
| `d_a_en_blockmain` | 12,392 | 90 | |
| `d_a_player_manager` | 10,764 | 68 | |
| `d_a_player_demo_manager` | 9,276 | 51 | |
| `d_a_bullet` | 7,316 | 73 | |
| `d_a_lift_down_on_base` | 6,280 | 58 | **3–4 TUs** — see `tu_split.py` |
| `d_a_move_pipe` | 5,380 | 29 | **2–3 TUs** |
| `d_a_en_obj_coinblock` | 5,096 | 34 | |
| `d_a_en_coin_main` | 4,312 | 27 | |
| `d_a_wm_player_static` | 3,268 | 24 | |
| `d_a_boss_demo` | 2,772 | 49 | **BLOCKED** — see below |
| `d_a_wm_Map_static` | 2,308 | 17 | **17/18 done**, blocked on a 0x9C8 table — see below |
| `d_a_player_hio_ADJ` | 2,172 | 20 | Five HIO classes; check whether it is one TU |
| `d_a_en_hatena_balloon` | 18,376 | 76 | |
| `d_a_farBG` | 18,808 | 53 | |
| `d_a_ice` | 31,880 | 147 | |
| `d_a_yoshi` | 39,944 | 239 | Largest; also contains `daPlyIce_c` |

### `d_a_en_lkuribo_base.cpp` is pre-surveyed — start here

Handed over by the `d_a_en_kuribo_base.cpp` agent, bounds already verified:

- `.text` **0x800331E0 – 0x800356D0** (0x24F0; the next TU is `daEnNetNoko_c`,
  already decompiled, so both ends are pinned)
- `.ctors` index 21 (`0x50-0x54`… **recheck against the index rule above**),
  `.rodata` 0x802EE950 – 0x802EE9B8, `.sdata` 0x80427BA8 – 0x80427BC8,
  `.sdata2` 0x8042B7E8 – 0x8042B810
- `__vt__17daEnLkuriboBase_c` at `.data:0x80305110`, size 0x2F0 — the same size
  as the base's, so it adds no new virtuals and only overrides
- States: Walk, Turn, Press, Split, HipSplit (new, `sStateID_c`-based) plus
  DieFall overriding `dEn_c`'s

**It is a sibling, not a subclass.** Despite the name it does not derive from
`daEnKuriboBase_c` — it overrides `createMdl`/`calcMdl`/`calcJnt` where the base
has `createModel`/`calcModel`. It is another `dEn_c` subclass, so expect the
member layout to differ from the base's (allocator 0x524, model 0x544, anmChr
0x584, texpat res 0x5BC / anm 0x5C0).

Its `.rodata` also owns the two 0x20 death-info templates that sit past a 4-byte
gap and look like they belong to the next TU — see the gap caveat above.

### `d_a_wm_Map_static.cpp` is 17/18 — one table away

Everything except `__sinit` matches. The gap is a guard-protected `mVec3_c` init
inside a **0x9C8-byte static object at 0x8031CCC8** (guard byte at 0x8042A470) —
a world-map parameter table whose initialiser pulls in 18 `.data` string
literals (`"desert01"`…`"group04_2"`), ~0x220 of `.sdata` literals, and one
`.sdata2` float. None of it emits code, which is why all 16 real functions match
without it. Reconstructing it means naming ~2,500 bytes of struct fields.

**Its slice ranges are unusable until that table is written** — a partial object
would place the table's contents at the wrong addresses. Do not bank it early.
Two signature findings from that work are already in the levers list.

### `d_a_boss_demo.cpp` is blocked on `d_en_boss.cpp` — schedule it after

Do not assign this TU yet. It was surveyed and deliberately not authored, which
was the right call: `initializeState_BattleIn` calls `dEnBoss_c::setBattleReady`
through vtable slot 0x2C8, and this TU *emits* that empty weak function. Getting
the slot right means declaring the whole `dEn_c` → `dEnBoss_c` virtual chain —
115 undecompiled symbols — and `d_en_boss.hpp` does not exist. Any stand-in
would be a fabricated 176-slot class in a *shared* header.

Everything else is banked ready for when it unblocks:

- True `.text` is **0x8001CBB0–0x8001DA50** (0xEA0, 59 functions), not the
  heuristic end — the eight template instantiations and `__arraydtor$70930`
  after `__sinit` are ours. It ends where `daBullet_c`'s first stub begins.
- Ranges: `.text 0x16430-0x172d0`, `.ctors 0x20-0x24`, `.data 0x3140-0x3420`,
  `.bss 0xfd8-0x10e8`, `.sdata 0x180-0x190`, `.sbss 0x90-0x98`,
  `.sdata2 0x188-0x198`. No `.rodata`.
- The class definition is **verified byte-exact**: compiled against real headers
  it emits a `__vt__12daBossDemo_c` identical to the target's, all 79 entries.
- Its `.data` opens with `sc_ForceList__6dWmLib` and `.sbss` holds
  `c_StartPointKinokoHouseID__6dWmLib` — header-scope statics duplicated into
  every TU including `d_wm_lib.hpp`. Including that header reproduces them, their
  dynamic initialisers in `__sinit`, and the `.bss` dtor record.

**Integration hazard, flagged in advance:** this TU owns `finalUpdate`,
`GetActorType`, `funsuiMoveX`, `setCarryFall`, `isSpinLiftUpEnable`, `getPlrNo`
and `vf68` at 0x8001D1B0–0x8001D218. All seven are currently supplied by
`syms.txt`, and four are in the global `deadstrip` list. When it lands, delete
those `syms.txt` lines **and move those four from `deadstrip` to `keepWeak`**, or
`.text` comes out short and every binary fails.

### Prefer base classes — they unblock families

Ranking by size alone is wrong. A *base* actor TU is worth more than its byte
count because derived TUs cannot be attempted honestly until it exists, and any
placeholder header written from a derived class's usage carries guesses that
will mislead later work. The pakkun family is the clearest case:
`d_a_en_dpakkun_base.cpp` (~8.2 KB) gates `d_a_en_dfpakkun.cpp` (~9.7 KB), and
its placeholder header still has an unidentified 68-byte member region.

Second preference is a TU whose *shape* is already solved elsewhere — e.g.
`d_a_rot_objs_base.cpp`, whose `searchParent_*` functions were reported as
sharing an existing implementation verbatim.

### All five binaries failing at once means a section changed size

This looks catastrophic and is usually trivial. `d_a_fireball_base.cpp` matched
51/51 functions and every data section, and still failed the hash on all five
binaries — including `d_profileNP.rel`, which it cannot touch.

**Diagnose from the DOL header, not the bytes.** Decode the section table of
`bin/wiimj2d.dol` and `original/wiimj2d.dol` and compare sizes:

```python
off = struct.unpack('>18I', d[0x00:0x48])   # file offsets
adr = struct.unpack('>18I', d[0x48:0x90])   # load addresses
siz = struct.unpack('>18I', d[0x90:0xD8])   # sizes
```

`.text` was 64 bytes short, so every later section sat 0x40 low and every hash
broke. The delta *is* the missing object: search the TU's address range for a
function of exactly that size. Here it was `__dt__18dCircleLightMask_cFv`, 0x40.

**Cause: an unreferenced weak function your TU emits gets deadstripped.** The
original kept it, the linker drops it. Fix is one line — add the mangled name to
`keepWeak` in `slices/wiimj2d.json`. Note `deadstrip` and `keepWeak` are
separate lists there and neither is inferred; a weak symbol in neither list is
dropped if unreferenced.

Byte-diffing the two DOLs first is a trap: a size change shifts everything, so
you get hundreds of thousands of diff runs and no signal. Section sizes first,
always.

### A one-byte whole-binary diff means a `.ctors` index error

Distinct from the size-change failure above, and even easier to fix. Symptom:
the DOL fails but the RELs pass, the two section tables are **identical**, and a
byte-diff yields exactly **one** differing byte.

That is your `__sinit` pointer written into the wrong `.ctors` slot. Decode the
words either side and the off-by-one is immediately visible:

```
0x802edd2c  built 80032ab0   orig 80030ab0   <-- ours, one slot early
0x802edd30  built 80032ab0   orig 80032ab0   <-- where it belongs
```

`.ctors` offsets are relative to **0x802edce0 directly**. Do not subtract the
`"offset": "0x4"` that appears on the `.ctors` entry in `meta` — that is what
went wrong here. The slot at `0x4c` belonged to the undecompiled
`d_a_en_jimen_pakkun_base`, whose `__sinit` is at 0x80030ab0.

Entries are in link order, so a TU's index follows its slice position. Sanity
check by confirming the neighbouring words point at the `__sinit` addresses of
the neighbouring TUs.

### The target list is incomplete: TUs with no `__sinit` are invisible

`tu_extent.py` delimits TUs by `__sinit` symbols. **A TU with no file-scope
static objects emits no `__sinit`**, so it does not appear in the list at all —
it is silently absorbed into a neighbouring TU's reported range. This is not
hypothetical: `d_a_sink_dokan.cpp` (`daSinkDokan_c`, ~0x920 bytes) was found
sitting undetected between `d_a_rot_objs_base` and `d_a_spin_child_base`.

`scratchpad/tu_split.py` detects the condition. It demangles the class name out
of every function in a reported range and counts them; a *second* class with a
double-digit count means the range is two or more TUs. Current output:

| Reported as | Actually contains |
|---|---|
| `d_a_lift_down_on_base` | `daLiftDownOnBase_c`, `daIceAshibaBase_c`, `daFlyDokan_c`, `daKawanagareObj_c` |
| `d_a_move_pipe` | `daLiftRemoconMain_c`, `daMovePipe_c`, `daLiftMain_c` |
| `d_a_yoshi` | `daYoshi_c`, `daPlyIce_c` |
| `d_a_en_dfpakkun` | `daEnDfpakkun_c`, plus 11 `daEnDpakkunBase_c` weak copies |

Counts of 1–5 are usually inlined helpers or genuinely co-located effect classes
(`d_a_ice.cpp`'s four `dIce*Ef_c`), not separate files. Treat ≥6 as the signal
and verify before acting. The hidden TUs are a *bonus* — they are small and
self-contained — but only once you know they exist.

### A class's functions can appear outside its own TU — weak copies

Header-defined and inline members are emitted by **every** TU that includes the
header, and the linker keeps an arbitrary one. So a symbol map can attribute a
function to your class at an address nowhere near your TU. `daEnDpakkunBase_c`
has eleven such functions living inside `d_a_en_dfpakkun.cpp`'s range;
`daSpinChildBase_c`'s destructor links from `d_a_obj_spin_child_base.cpp`.

Do **not** stretch your slice to cover them — one contiguous range per section
means that boundary is unreachable. Read them instead as evidence about *where
the definition belongs*: a member whose surviving copy sits in another TU is
almost certainly **inline in the header**, not out-of-line in your `.cpp`.
Virtuals remain the exception — an empty virtual defined in the class body is
inlined away by `-ipa file` and breaks the vtable slot.

The verification that closes the loop: compile, then check the weak copies your
object emits are byte-identical to the ones the original linked elsewhere. That
validates your header for the TU that owns those bytes, before anyone starts it.

### `tu_extent.py` had a boundary bug — check the ranges it gave you

Its "banked slice tightens the start" test was containment
(`prev_end <= lo < addr`) when it should have been **overlap** (`lo < addr and
hi > prev_end`). A banked slice that *straddled* `prev_end` was silently
ignored, so nine TUs were reported starting inside already-decompiled
territory — `d_a_en_dpakkun_base` by 1,000 bytes, `d_a_fireball_base` by 992,
`d_a_rot_objs_base` by 932. Fixed, but the lesson generalises: **a heuristic
range is a starting hypothesis, and every agent must re-derive its own bounds
from the symbol map and `slices/wiimj2d.json` before writing code.**

## The actor-TU playbook

Look up first, in this order — ~10 minutes for 80% of the file:

1. `grep '18daEnXxx_c' bin/dtk/wiimj2d_symbols.txt` → function list with sizes,
   the `__vt__` address, the `StateID_*` and `.bss` objects.
2. **Disassemble the `auto_*_data.o` containing the vtable.** With slot offsets
   computed, `__vt__` *is* the class declaration: base class, which base virtuals
   are overridden, the new virtuals **in declaration order**, and the state names.
   Do this before writing a line.
3. Disassemble the matching `auto_*_rodata.o` / `_sdata2.o` / `auto_sinit_*.o` —
   free float values and the `__sinit`.
4. Read `source/dol/bases/d_a_en_shell.cpp` and `d_a_en_super_bigpile.cpp` for
   house style.

**Boilerplate — copy it:**
- The four `baseID_Xxx<10sStateID_c>` stubs and the **entire `__sinit`** are
  generated by one `STATE_VIRTUAL_DEFINE(Class, Name);` per state, in vtable
  order. `__sinit` runs ~0xDE per state — 888 bytes in bigpile that nobody wrote
  or debugged.
- `.bss` is always `n × (0xC dtor-chain record + 0x34 sFStateVirtualID_c)`,
  stride 0x40. Plain `STATE_DEFINE` states are 0x30.
- `.data` always carries 3 ptmf constants (0xC each) and one
  `"Class::StateID_Name"` string per state.
- The 8 `sFStateID_c<T>` / `sFStateVirtualID_c<T>` instantiations after `__sinit`
  come out automatically.

**Per-actor:** resource/model/anim names, param bit layout, state logic, the
rodata float table, SE id, effect name.

**Traps:**
1. **Inline wrappers change stack-temp slots.** Temps for arguments of *directly
   called* out-of-line functions are allocated top-down first in source order;
   temps created inside *inlined* wrapper calls come after. If a function differs
   *only* in `addi rN, r1, 0x..` offsets, toggle inline-wrapper vs explicit-full-args
   calls — do not touch the logic. This is how the missing
   `anmTexSrt_c::create` wrappers were found.
2. **Every empty virtual must be defined out of line**, never in the class body,
   or it is implicitly inline, vanishes from the TU, and the vtable slot breaks.
3. **`float * int` evaluates its calls right-to-left.** To get "call A, then B,
   then `fmuls fD, A, B`", split: `float d = getA(); s16 r = d * getB();`.
4. **A 2-float `static const` array lands in `.sdata2`, a 4-float one in
   `.rodata`.** One `.rodata` label with offsets 0/4/8/0xC is *one* array.
5. Sound uses `dAudio::SoundEffectID_t(ID).playMapSound(pos, 0)`, not
   `g_pSndObjMap->startSound(...)` — the helper takes the object as a parameter,
   so it loads into a saved register before the argument expressions.
6. `extrwi. rX, rX, 1, N` is `ACTOR_PARAM(bitfield)`; `clrlslwi. r3, r0, 24, 4`
   is `ACTOR_PARAM(byte) * 16`. Use `ACTOR_PARAM_CONFIG`, don't hand-roll shifts.
7. `fBase_c`'s vtable pointer is at object offset **0x60**, not 0. Slot index =
   `(0xNNN - 8) / 4`.
8. Compile flags need the seven extra `-i include\lib
evolution\BTE\...` paths
   from `build.ninja`, or anything including `d_audio.hpp` fails.
9. **`.text` is always 16-aligned**, so a run of zero bytes at the start of what
   looks like your TU belongs to the *previous* one. Do not include it.
10. **Argument string literals are emitted right-to-left.** If a call takes two
    literals, the second one appears first in `.rodata`.
11. When a TU defines a profile, **delete its `g_profile_<NAME>` line from
    `syms.txt`** or the link fails on a duplicate.
12. **CFront mangling does not mark static members.** If `r3` holds a real
    argument rather than `this`, the function is `static`.
13. **Base actor classes can have pure virtuals.** Do not assume every vtable
    slot needs a body in the base TU.

**Track D per-function false-diff note:** compiler-pool symbol names differ
run-to-run (`...data.0` and friends). Normalise them away before diffing or you
will chase phantom differences — `diffall.py` already does.

## Briefing template for authoring agents

Every agent brief should carry, at minimum:

1. The exact function(s) it owns and their addresses/sizes.
2. Where the target disassembly already is (do not make them re-derive it).
3. The standalone compile+diff loop above — and that they must **not** run
   `ninja` or edit `slices/wiimj2d.json`.
4. **Deliverable is source code in the reply**, not edits to the shared tree.
5. The hard-won context: declaration order controls register assignment (GPRs
   first-declared → highest, ending at r9; **FPR direction is not fixed — sweep
   it**); adding one extra local can fix colouring at zero instruction cost; the
   size-delta heuristic; bisect before theorising.
8. **Report data objects with their sections.** The lead needs string literals,
   floats, statics and vtables for the slice bounds — that is where integration
   time actually goes.
9. **Do not claim MATCHING unless the diff tool printed nothing.** Say so
   explicitly. A well-characterised near-miss is far more useful than a false
   pass, which has cost this project a full day.
6. Environment gotchas: dtk relative paths with forward slashes fail on Windows;
   PowerShell 5.1 parses 8-hex-digit literals as negative Int32, so do address
   maths in Python; splat native-exe arguments.
7. **No background processes.** Everything foreground, confirm exits, check for
   strays before finishing. (One agent leaked a script that span at 100% CPU for
   21 minutes.)

## Monitoring agents — what actually works

Most obvious signals are useless. Verified this session:

- Agent transcript files under `tasks/` are **always 0 bytes**, including for
  agents that completed successfully. Not a liveness signal.
- The short-random-name `.output` files in `tasks/` are the **lead's own** shell
  invocations, not agent activity.
- Process listings only catch the instant a command runs; `Read`/`Grep` spawn
  nothing at all.

The only real signal is **writes to the repo or scratchpad**. And note that a
healthy agent on a task like this ran **35 minutes with 97 tool calls** and was
silent for the first several minutes while reading reference material. Do not
set an impatient kill threshold — a working agent was nearly killed at 12
minutes on exactly that mistake. Agents on the hardest functions here ran 30–45
minutes and 60–115 tool calls, and all of them succeeded.

## Verify your verification tool

`fndiff.py` silently reported **`IDENTICAL` when it could not find the function
at all**. Template-mangled names (anything with `PrintContext<w>`) appear
*quoted* in the dtk dump, the name comparison never matched, and empty-vs-empty
compared equal. Nine in-flight functions were affected.

It is fixed two ways: it strips quotes from both sides, **and it hard-exits with
an error if either extraction is empty**. A tool that cannot find the function
must say so, not congratulate you. One agent then confirmed the fix with a
deliberate negative control before trusting its own results — do that.

**Treat the verification tooling with the same scepticism as the code.**

## Relay findings between running agents

Several results only emerged because one agent's finding reached another
mid-flight (`SendMessage`). Worth relaying immediately:

- A matched caller pins down its callees' exact signatures — `SetRouteInfo`
  handed five signatures to two agents still guessing.
- A statement-ordering trick found in one function often applies verbatim to a
  sibling another agent owns.
- Corrections to rules *you* gave them. Both the FPR direction and a `.sdata`
  vs `.sbss` slip were caught by agents and had to be pushed back out.

## Check `syms.txt` before inferring a name

Two functions believed unnamed were already named there, one of them called by an
already-matching destructor. Grep `syms.txt` and `bin/dtk/wiimj2d_symbols.txt`
first; mark genuinely inferred names `@unofficial`.

---

## Current state

- **Progress: 8.823%** (573,528 / 6,500,368 code bytes)
- All five binaries verify byte-for-byte (`progress.py --verify-bin` → 5 OK)
- Development moved to **native Windows**; see "Local setup" below.
- Three TUs completed and banked whole this session (22,688 bytes):
  `d_wm_csvdata.cpp` (41 fns), `d_a_en_super_bigpile.cpp` (46 fns),
  `d_tag_processor.cpp` (39 fns).

Per-binary:

| Binary | Progress |
|---|---|
| `wiimj2d.dol` | 17.06% |
| `d_profileNP.rel` | 100% |
| `d_enemiesNP.rel` | 2.06% |
| `d_basesNP.rel` | 1.02% |
| `d_en_bossNP.rel` | 0.03% |

## Local setup

Development now happens **natively on Windows**. CodeWarrior is a Windows
binary, so no `wibo`/WINE layer is involved and the Shift-JIS hazard described
in `tools/linux_env/README.md` does not apply here.

1. Clone and check out the branch.
2. Place the original binaries in `original/` (see the main README, steps 1–4).
   Verify their MD5s against the README's list.
3. Extract CodeWarrior into `compilers/` so `compilers\Wii\1.1\mwcceppc.exe` exists.
4. `pip install ninja pyyaml`

Build and verify:

```bash
python configure.py; ninja; python progress.py --verify-bin
```

`prepare_objdiff.py` regenerates `bin/dtk/` symbol maps and the dtk splits. Run
it once after setup; it fetches dtk v1.8.0 into `bin/` if not already present.

### Windows tool locations

- dtk: `bin\dtk-windows-x86_64.exe`
- readelf: `C:\devkitPro\devkitPPC\bin\powerpc-eabi-readelf.exe` (not on `PATH`;
  invoke by full path)
- `objdiff-cli` is not installed; the objdiff GUI reads the generated
  `objdiff.json` directly.

For Linux setup instructions, see the main README and `tools/linux_env/`.

## The working loop

1. Pick a target (see "Next targets" below).
2. Disassemble the region that contains it:
   `.\bin\dtk-windows-x86_64.exe elf disasm bin\dtkspl\obj\<auto_..._text.o> <out>`
3. Write the C, add a slice entry to `slices/wiimj2d.json`, add any call targets
   to `syms.txt`.
4. `python configure.py; ninja`
5. **Check the object's function sizes against the target first**, before reading
   any diff:
   `& "C:\devkitPro\devkitPPC\bin\powerpc-eabi-readelf.exe" -sW bin\compiled\wiimj2d\<path>.o`
6. If sizes match but verification fails, disassemble your own object and diff it
   against the target instruction by instruction.

### Rules the build imposes

- **One slice entry per source file.** Two entries with the same `source`
  generate duplicate `.o` targets and ninja fails.
- **A slice must be one contiguous range per section.** Gaps between slices are
  auto-filled from the original binary by `make_filler_slice` in
  `tools/slicelib.py`, so partial translation units are fine — but you cannot
  skip a function in the middle of your own range.
- Slice offsets are **section-relative**. `.text` base is `0x80006780`.
- Slices must appear in the JSON in ascending order per section.
- Every function or object you reference but have not decompiled needs a
  `syms.txt` entry. If you *have* decompiled it, it must **not** be in
  `syms.txt` — that is a duplicate definition.
- `deadstrip` in `slices/wiimj2d.json` keys on bare symbol name and is global.
  Safe for mangled C++ names; unsafe for C names that collide across units.
- **`.data` must be contiguous too, and it constrains which `.text` you can take.**
  Your object emits string literals and vtables in source order, so a partial TU
  can leave them at the wrong addresses even when every function matches. If a
  TU's data pool ends with a vtable, you generally must decompile *every*
  literal-emitting function up to that point in one pass, or omit the function
  that pulls the vtable in. This bit `d_wm_csvdata.cpp` — see below.

### Diagnosing failures

- **"4 of 5 binaries failed"** usually means one function is the wrong *size*.
  A size mismatch shifts every DOL address after it, so RELs that reference
  post-shift symbols break too. It looks catastrophic and almost never is.
- **Bisect before theorising.** Cut the slice down to a single function, confirm
  it verifies, then add functions back one at a time. Three sessions were lost
  to diagnosing from symptoms instead of doing this.

## Techniques established

### Data-section slicing

A slice can carve `.data`, `.rodata`, `.sdata2` etc. alongside `.text`. Needed
whenever a function has a local static table or an anonymous float literal.

To find the data: look up the symbol in `bin/dtk/wiimj2d_symbols.txt`, then read
its bytes out of the original DOL by walking the DOL header (text offsets at
`0x00`, addresses at `0x48`, sizes at `0x90`).

Used for `GXSetPixelFmt` (`.data` lookup table) and `GXSetViewportJitter`
(`.sdata2` float constant).

### Paired singles

`typedef __vec2x32float__ v2f;` is CodeWarrior's paired-single type and emits
`psq_l` / `psq_st` when you assign through it.

Two things worth knowing:

- Most `psq_*` in already-matched objects is **prologue/epilogue spill** of
  non-volatile `f31`, not data movement. Counting `psq` per object is a
  misleading signal.
- CodeWarrior auto-pairs adjacent float field stores when it can prove 8-byte
  alignment — e.g. `mPos.x = …; mPos.y = …;` on a struct member compiles to one
  `psq_st` (see `dAcPy_c::eatMove` in `d_a_player.cpp`). Through a bare `f32 *`
  parameter it cannot prove alignment, which is when the explicit type is needed.

### Declaration order controls register assignment

This is the lever the earlier sessions were missing. **CodeWarrior assigns
registers in order of local declaration**, so when output is instruction-identical
but register-shuffled, reorder the declarations rather than restructuring the code.

- **GPRs — first-declared gets the *highest* register.** `AXAcquireVoice` put
  `old` in r30 and `vpb` in r31; the target had them swapped. Moving
  `BOOL old = OSDisableInterrupts();` above the other locals fixed all 23
  non-volatile mismatches in one pass. (A declaration with an initialiser can
  legally precede other declarations in C89.)
- **FPRs — the direction is NOT fixed. Reorder and bisect; do not assume.**
  This file previously stated "first-declared gets the lowest", on the strength
  of `GXGetViewportv` (which needed `f2, f1, f0` in load order, produced by
  declaring the locals in reverse and assigning them in address order). That
  generalisation is **wrong**. In `d_tag_processor.cpp`, `setScissorStart` gave
  first-declared the *highest* FPR (f31 down to f28) while `setScissorCursorX`
  in the same file gave it the *lowest* (f28 up to f29). Treat FPR order as a
  lever to sweep, not a formula.

### The GPR block rule (verified over ~140 recompiles on `getScissor`)

- Locals in the **leading declaration block** take the top *N* of r6–r9,
  **ascending in declaration order, ending at r9**.
- Values created **later** — CSE temps, and locals declared mid-body — fill
  **downward from just below that block**.

Verified directly by inserting a materialised local at each of four positions
and watching it land on r6/r7/r8/r9 in step.

### Adding a variable fixes register allocation — three independent proofs

The strongest lever found this session, and it is counter-intuitive: **declaring
one extra local, changing no logic, fixes register colouring at zero instruction
cost.** It worked three separate times on three different kinds of value:

1. **A pointer.** `setScissorStart` was instruction-identical but used four
   scratch GPRs where the target used three. Adding
   `nw4r::lyt::DrawInfo *info = mScissor[idx].mpDrawInfo;` before the block
   fixed it.
2. **A float constant.** `RuBySet` — `f32 rubyScale = c_RUBY_SCALE;` on the line
   before its use moved the folded constant off `f0` and corrected **nine**
   register assignments at once.
3. **A raw integer offset.** `getScissor` — see below; this is the subtle one.

**But the lever is not universal, and knowing when it is excluded matters.** In
`getScissor` a named `f32 negate = -1.0f;` is structurally impossible: the
target *re-materialises* `0.0f` after `-1.0f` clobbers `f0` between two
compares, and a named local pins the constant into a live FPR across the
branches, deleting that reload. **The trick only applies where the constant is
used unconditionally.**

### `getScissor`: offset vs pointer — why a pointer local always failed

Two agents spent ~140 recompiles at 82/82 instructions with r6 and r8
transposed. The answer was a **fourth leading local holding a raw byte offset**,
declared before `idx` and assigned after it:

```cpp
nw4r::lyt::Pane *pane = info->mScissorPane;          // r6
nw4r::lyt::DrawInfo *drawInfo = info->mScissorDrawInfo;  // r7
int entryOffset;                                     // r8 — slot, no instruction
u16 idx = info->mScissorIndex;                       // r9
entryOffset = idx * sizeof(ScissorEntry_s);
```

It costs nothing because MWCC CSEs it with the index scaling `mScissor[idx]`
already performs — hence exactly one `mulli`. The second half is essential:
**the first store block must use `mScissor[idx]` and the second must use the
byte-pointer form.** Pointer form in both makes `(u8 *)mScissor + entryOffset` a
source-level CSE and collapses the target's *two* `add`s into one; `mScissor[idx]`
in both leaves `entryOffset` with nothing to do.

That is why every `ScissorEntry_s *entry` attempt failed: a **pointer** local is
`this + off`, which CSEs the add. Only a raw **offset** keeps them apart.

Try all of this before concluding a function has hit the register-allocation wall.

### `.sdata2` ordering within a TU — creation order, not three buckets

**An earlier version of this section claimed named objects always come first,
then anonymous literals. That is wrong — do not act on it.** There is one
ordering, not three buckets:

1. **Creation order.** Named file-scope objects and anonymous folded literals
   share a single sequence, ordered by *where in the TU the object is first
   created* — a definition for a named object, a first use for a literal. They
   interleave freely.
2. **Function-local `static`s — last**, after everything else.

The refutation is `d_a_player_base.cpp`, visible in the symbol map without
compiling anything: twelve named `sc_*` constants, then ~46 anonymous literals,
then five more named `scDokan*` constants. Under "named first" that layout is
impossible. It is exactly what creation order predicts, because the `scDokan*`
definitions sit further down the file, next to the dokan functions that use
them. `d_actor.cpp` agrees (four named statics, then `l_Ami_Line`/`l_Ami_Zpos`,
then anonymous).

To check this for yourself on any decompiled TU, list the `.sdata2` symbols
inside its slice range in address order and mark which are named — the shape
falls straight out. Beware `lbl_########` names: those are dtk placeholders for
objects it could *not* name, so they count as anonymous, not named.

Rule 2 came from `d_tag_processor.cpp` and still holds: a
`static const u16 cTagCode[4]` declared *inside* `MsgIDSet`, a function in the
middle of the file, landed at the **end** of the TU's `.sdata2`. Lifting the
same array to file scope moved it up to its definition position, matching the
original.

**If a TU's `.sdata2` content is right but in the wrong order, move the
definitions — do not reshape the code.**

### Before sweeping, check whether a sibling TU already solved the shape

`rotation_move` in `d_a_en_door.cpp` cost ~300 builds across 8 sweeps and then
closed in half an hour once someone compared it against `rotation_move` in
`d_a_right_base.cpp`, which was **already byte-exact and used the answer
verbatim**. Actor TUs repeat each other heavily. Grep the decompiled sources for
a function of the same name or shape *first*; it is the cheapest lever available
and nothing else in this file comes close to that return.

### Narrowing to 16 bits is a reassociation barrier

The lever that closed it, worth its own entry. With an `int` intermediate MWCC
rewrites `(raw + 0x4000) + base` into `(raw + base) + 0x4000` and then
rematerialises the `addi` at every use. Narrowing the intermediate to `s16`
(or `mAng`, or `short`) is a barrier it will not reassociate across, so the add
stays eager and single.

It only works **combined** with the compound form — `angle = raw; angle += K;`
rather than `angle = raw + K;` — which is what makes the value load straight into
its final register. Each half alone fails: `int` + compound gave 47 diffs, 16-bit
+ eager gave 45. `u16` does not work either; zero-extension changes the load.

**Four separate functions this session needed a coupled pair after a large
single-axis sweep plateaued.** Sweeps of 25, 115, ~120 and ~300 builds all failed
where two simultaneous changes succeeded. Treat a hard floor as a signal to
change axis, not to enumerate harder.

### Levers found while decompiling the rot/fireball/cursor batch

Each of these was the single change that closed a function; all are zero- or
near-zero cost at the source level.

- **Weak/implicit functions flush right after the function that first needs
  them**, not at end of TU. `__dt__12daRotBlock_cFv` sat at position 3 in the
  target and position 40 in the draft; declaring `~daRotBlock_c()` and defining
  it out of line dragged it — and two more implicit dtors — into the right slots.
- **`fmuls` operand order cannot be set by writing `x * 1.5f` vs `1.5f * x`.**
  MWCC canonicalises the literal to the first operand either way. Only the
  compound form `size = x; size *= 1.5f;` produces the target's variable-first
  `fmuls f6, f7, f6`.
- **Hoist a loop bound into a local** (`dBg_ctr_c *end = mpBgCtrEnd;`) to turn a
  reload-every-iteration loop into the target's hoisted `do…while`. Three
  functions in one file needed it.
- **Assign struct fields directly rather than through a `set()` helper** when the
  helper builds a temporary: routing through `set()`'s `mPos = mVec3_c(x,y,z)`
  leaves a dead 12-byte stack temp.
- **A named local pins a value across two calls.** `u32 flags = mLayer << 16;`
  hoisted as its own local is what keeps the shift in r31 across both calls.
- **Return the base type, not the derived one, when the target copies.**
  `GetPos` returning `mVec3_c` lets NRV collapse it into a 5-instruction tail
  call; returning `nw4r::math::VEC3` forces the target's 12-instruction bitwise
  copy. This alone took the function from 5 instructions to a byte match.
- **Do not declare a constructor the original does not have.** An implicit ctor
  is fully inlined into the class-init function; declaring one emits a
  `__ct__` symbol that is simply absent from the target.
- **Inlined parameter binding perturbs FPR colouring — field stores do not.**
  This is a distinct lever from declaration order and it closed `checkQuakeDeath`
  after ~120 failed builds. Writing `check[i].set(a, b)` binds two parameters
  into an inlined call, and that binding is what moves the float registers; the
  identical arithmetic written as `check[i].x = a; check[i].y = b;` does not.
  Pair it with reading inputs as individual accessor calls rather than one
  `getBounds(&a, &b, &c, &d)` — neither change works alone.
- **Deadness, not construction form, triggers scalar replacement.** An unused
  local aggregate gets split into scattered stack slots no matter how you build
  it; make it genuinely live and MWCC keeps it contiguous. If the target has a
  contiguous dead-looking aggregate, look for the use you have not modelled
  rather than for a cleverer way to spell the construction.
- **Bind a reference before consecutive stores through a pointer member.**
  `mVec3_c &pos = p->mPos; pos.x = …; pos.y = …;` — writing `p->mPos.x` twice
  reloads `p` between the stores.

### `.sdata2` literal pooling — the per-function rules

Established by standalone micro-benchmark while closing `d_a_enemy_ice.cpp`, and
they explain `.sdata2` orders that otherwise look arbitrary:

1. Literals are pooled **per function, in source order** within that function.
2. `if (x == A) { x = B; }` creates **B before A**.
3. The int→float conversion magic double is **appended last** within its own
   function's group.
4. A **dead** literal is still pooled. This is the useful one: if the target's
   `.sdata2` order requires a constant to exist before one you can account for,
   the original had code there that you cannot see — most likely leftover or
   debug code. Reproducing the bytes may require a dead local. Comment it as a
   reconstruction; do not pretend it is the original text.

### The 4-byte-gap boundary signal needs a referencing check

The inter-TU 8-byte alignment rule is sharp but **not sufficient on its own**.
`d_a_en_net_nokonoko_base.cpp` has a 4-byte `.rodata` gap at 0x802EE974 that is
*not* a TU boundary: the two `sDeathInfoData` pools after it belong to the
previous TU, one of them referenced from `setDeathInfo_Hasami__17daEnLkuriboBase_c`.

Before splitting on a gap, **find who references the objects on each side**. A
reference from the earlier TU settles it immediately.

### A constant appearing twice is evidence, not redundancy

A `static const f32` **class member** is emitted as a real named object *and*
its uses are constant-folded into fresh anonymous duplicates — **but only if the
definition precedes the use in the file.** With the definitions below the
functions that use them, the uses referenced the named symbols and no anonymous
duplicates appeared.

So the original's `.sdata2` containing both `c_RUBY_SCALE` (0.6f) and an
anonymous 0.6f proves the definitions sit **above** the ruby functions in the
source. Read duplicate constants as a layout clue.

### A function-scope `static const int` allocates storage

`c_ACTION_NAME_LEN` had to become `enum { c_ACTION_NAME_LEN = 20 };`. As a
function-local `static const int`, MWCC emits a real word into `.sdata2` even
though the value is folded into an immediate at the use site. **Use `enum` for
function-scope integer constants unless the binary shows an object.**

### `va_list` must be an array of one

```c
typedef struct __va_list_struct { ... } __va_list_struct;
typedef __va_list_struct va_list[1];
```

As a plain struct, `va_arg` passes all 16 bytes **by value** and MWCC copies
them to the stack before every `__va_arg` call — six extra instructions. The
original passes the pointer, which only happens when the type decays. Found via
`preProcess`; verified codegen-neutral for the already-matching
`d_lyttextbox.cpp` (all 310 words identical both ways).

### Size-delta heuristic

- Your function is **shorter** than the target → you factored out something the
  original wrote inline. Duplicate it at each call site.
- Your function is **longer** → you left something out of line that the original
  inlined, or used a costlier idiom.

This fixed `__AXServiceCallbackStack` in one pass (was 140 vs 172; the original
pops the callback stack inline at both sites rather than via a helper).

### CodeWarrior gotchas found

- Built-in `HID2` assembles to **SPR 979**. Broadway's HID2 is **SPR 920**. Use
  the number. `WPAR` is fine.
- `types.h`'s `ROUND_UP` masks with `-(align)` (one `neg`). `ROUND_UP_PTR` uses
  `~(align-1)` (`subi` + `nor`). The SDK generally wants the second form; picking
  the wrong one is a one-instruction size difference.
- A signed shift feeding a bitfield insert costs a separate `srawi`. Cast to
  unsigned and CodeWarrior folds it into the `rlwimi` rotate.
- GX register writes come in two shapes and it varies per function: build the
  value in a local then write back (`GXSetColorUpdate`), or update the `GXData`
  field in place and reload it for the FIFO (`GXSetZCompLoc`). Read the target.
- Library/SDK files want `-proc gekko -fp hard -O4 -Cpp_exceptions off -enum int
  -RTTI off` (no `-inline noauto`), set per-slice via `compilerFlags`.

## The register-allocation wall

**The broad "wrong compiler" theory is dead.** `GXSetTevColorIn` matches
byte-exactly while using **10 live GPRs** — more than `GXSetTevColor` (7), in the
same file, with the same flags and the same idiom. `OSLockMutex` matches with
five calls and four callee-saved registers. The compiler in this repo
demonstrably reproduces Nintendo's allocation under real pressure, so for
integer/GPR code a mismatch means **our reconstruction is shaped wrong**, not
that the toolchain is.

`GXGetViewportv` is the counter-case that proves this is not about pressure at
all: 8 instructions and 6 registers, less than ~24 functions that already match.

**One narrow compiler-shaped exception survives — paired singles.** mwcceppc
never emits a paired-single load/store with a literal 0 displacement; it always
picks the indexed form (`psq_stx f2, r0, r3` instead of `psq_st f2, 0x0(r3)`).
Verified by minimal probe on Wii 1.0/1.1/1.3/1.7 **and** GC 3.0/3.0a3. The
immediate form only appears after the -O2+ address-folding pass creates a
displacement node, and offset 0 has nothing to fold. The same quirk blocks
`__GXSetProjection` / `GXLoadPosMtxImm`. Corroborating: GC 3.0 compiles the
identical source to `lwz r4` — the target's register — where Wii 1.1 gives r6.
So a compiler hunt is justified **only for paired-single code**, not generally.

The remaining GPR cases stop at the same place: **exact size, exact instruction
sequence, wrong register numbers**.

Affected: `MEMAllocFromFrmHeapEx`, `OSAllocFromMEM1ArenaLo`, `AXAcquireVoice`,
`GXSetTevColor` + `GXSetTevColorS10`, `GXInitLightSpot`, and the store half of
`GXGetViewportv`. Every one is a Revolution SDK file. Everything already matched
in these units is simple enough that allocation is unambiguous; divergence
begins exactly where register pressure gives the allocator a real choice.

**Ruled out** (do not re-test):

- *Compiler version.* `GXInitLightSpot` was built with Wii 1.0, 1.1, 1.3 and 1.7.
  All four produce byte-identical output.
- *Optimisation flags.* Swept `-O3`, `-O4`, `-opt speed`, `-opt space`,
  `-opt all`, `-opt nopeephole`, `-schedule off`, `-ipa file`, `-inline auto`,
  `-inline nobottomup`. `-O4` (the current setting) is already the best; nothing
  changes the allocation.
- *Struct alignment.* `-align mac68k4byte` would repad `GXData` and break the
  functions that already match.
- *Source shape.* Declaration order, scoping, live-range splitting and variable
  coalescing all move which *named* local gets which register — but never fix
  the last one or two. See the per-function notes below for what was tried.

The declaration-order lever (below) reliably places named locals, so start
there; but when only compiler-generated temporaries are left, no source-level
lever has been found. The most plausible remaining explanation is that the
SDK objects were produced by a CodeWarrior build not present in
`compilers_20230715.zip`. Confirming that would need a different compiler drop.

## Open blockers

Both are "which C formulation produces this codegen" problems, not
comprehension problems. In each case the output is already the right size with
the right instructions.

### 1. Register allocation

Affected: `MEMAllocFromFrmHeapEx` (288 B), `OSAllocFromMEM1ArenaLo` (52 B),
`GXSetTevColor` + `GXSetTevColorS10` (196 B).

Each reaches exact size with instruction-identical output, differing only in
which registers CodeWarrior picked. Roughly a dozen formulations tried across
them without finding a lever. Worth studying how the large already-matched units
(`d_a_player.cpp`, `d_enemy.cpp`) express similar shapes.

### 2. `GXGetViewportv` (32 B)

Gates extending `GXTransform.c` **backward** — 1,064 bytes across 14
header-described functions (`__GXSetViewport`, the `GXLoadPosMtxImm` family,
`GXProject`, …) sit between it and the current slice start.

**Now much closer.** Exact size, and the first four instructions — the `__GXData`
load and all three `psq_l` with immediate offsets 0x544/0x54c/0x554 — match
exactly, as do the `f2, f1, f0` register choices (fixed via the declaration-order
lever above). Best formulation is saved in the scratchpad and reproduced here:

```c
typedef __vec2x32float__ v2f;

void GXGetViewportv(f32 view[GX_VIEWPORT_SZ]) {
    v2f near, sx, ox;           // reverse order => ox=f2, sx=f1, near=f0

    ox = *(v2f *)&gxdt->vpOx;
    sx = *(v2f *)&gxdt->vpSx;
    near = *(v2f *)&gxdt->vpNear;

    *(v2f *)view = ox;
    *(v2f *)&view[2] = sx;
    *(v2f *)&view[4] = near;
}
```

Remaining gap is **only the three stores**: the target emits them in order
(`psq_st f2,0(r3)`, `f1,8(r3)`, `f0,0x10(r3)`); CodeWarrior emits them as
`0x10`, then `psq_stx f2,r0,r3` for offset 0, then `0x8`. Four store
formulations tried (array index, `&view[n]`, bare deref, struct field
assignment) — all produce the identical scrambled order, so the store scheduling
does not appear to be source-controllable from this shape. A whole-struct copy
is *not* the answer: it degenerates into integer `lwz`/`stw` pairs.

Note: `WGPIPE` in `GXHardware.h` has no paired-single member. The matrix loaders
will need one added.

### 3. `GXSetTevColor` / `GXSetTevColorS10` (196 B)

**Now instruction-for-instruction identical to the target** — 24/24 and 25/25
instructions, same opcodes in the same order. All that remains is a three-way
rotation of the volatile temps. Full working source is in
`scratchpad/GXTev.c.best`; the shape is:

```c
void GXSetTevColor(GXTevRegID id, GXColor color) {
    u32 c = *(u32 *)&color;
    u32 regLo, regHi;

    regLo = (GX_BP_REG_TEVREG0LO + id * 2) << 24;
    regLo = GX_BITSET(regLo, 24, 8, c >> 24);   /* red   */
    regLo = GX_BITSET(regLo, 12, 8, c);         /* alpha */
    GX_BP_LOAD_REG(regLo);

    regHi = (GX_BP_REG_TEVREG0HI + id * 2) << 24;
    regHi = GX_BITSET(regHi, 24, 8, c >> 8);    /* blue  */
    regHi = GX_BITSET(regHi, 12, 8, c >> 16);   /* green */
    GX_BP_LOAD_REG(regHi);                      /* BG is written three times */
    GX_BP_LOAD_REG(regHi);
    GX_BP_LOAD_REG(regHi);

    gxdt->lastWriteWasXF = FALSE;
}
```

Two findings got it there, both reusable elsewhere:

- **Read the colour through a `u32`, not through `color.r` / `color.a`.** The
  target loads the struct once (`lwz r8`) and selects each channel purely by
  varying the `rlwimi` rotate. Field access instead emits four `lbz` byte loads
  (29 instructions) — CodeWarrior will not merge them, presumably because
  `GXColor` is four `u8`s and so has alignment 1. Writing `c >> 24` etc. lets it
  fold the shift into the rotate and reproduces the target's encodings exactly.
  The rule is **rotate = (src_bit − dst_bit) mod 32**.
- **Build the BP register address with a shift, not `GX_BITSET` on zero.**
  `reg = 0; reg = GX_BITSET(reg, 0, 8, addr);` costs `li 0` + `rlwimi`;
  `reg = addr << 24;` gives the target's single `slwi`. (Note `GXSetFieldMask` in
  the already-matched `GXPixel.c` sets the address *last* — that pattern is not
  what these two use.)

**`GXSetTevColorS10` now MATCHES byte-exactly** (25/25, verified). The winning
shape declares *both* registers as initialised locals up front:

```c
void GXSetTevColorS10(GXTevRegID id, GXColorS10 color) {
    u32 rg = ((u32 *)&color)[0];
    u32 ba = ((u32 *)&color)[1];
    u32 regLo = (GX_BP_REG_TEVREG0LO + id * 2) << GX_BP_OPCODE_SHIFT;
    u32 regHi = (GX_BP_REG_TEVREG0HI + id * 2) << GX_BP_OPCODE_SHIFT;

    GX_BP_SET_TEVREGLO_RED(regLo, rg >> 16);
    GX_BP_SET_TEVREGLO_ALPHA(regLo, ba);
    GX_BP_SET_TEVREGHI_BLUE(regHi, ba >> 16);
    GX_BP_SET_TEVREGHI_GREEN(regHi, rg);

    GX_BP_LOAD_REG(regLo);
    GX_BP_LOAD_REG(regHi);
    GX_BP_LOAD_REG(regHi);
    GX_BP_LOAD_REG(regHi);

    gxdt->lastWriteWasXF = FALSE;
}
```

It cannot be banked alone: a slice is one contiguous range, and `GXSetTevColor`
sits immediately before it. Both must match together.

Remaining gap, `GXSetTevColor` only: target assigns r4 = WGPIPE base, r5 = 0x61,
r6 = `regHi`; CodeWarrior assigns r5, r6, r4. `c` = r8 and `regLo` = r7 already
match. **The whole problem is that `regHi` is treated as a compiler temporary
rather than a declared local**, so it takes the lowest free register (r4) instead
of continuing the declaration-order sequence into r6.

Eight source shapes tried, allocation **invariant across all of them** — do not
retry: single `reg` reused; separate `regLo`/`regHi`; `regHi` scoped to an inner
block after the first FIFO write; both registers initialised up front (this is
what fixed S10, but for `GXSetTevColor` it also perturbs the schedule and moves
`c` off r8); both up front with the FIFO writes interleaved; `regHi`'s
initialiser hoisted between the two `regLo` `GX_BITSET`s so the live ranges
overlap; declaring `regHi` before `regLo`; initialiser vs plain assignment.
`-align mac68k4byte` is not an option: it would repad `GXData` and break the four
functions in this TU that already match.

Cracking the remaining rotation unlocks the largest fully-described run in the
DOL (2,464 B, 15 functions, 100% header coverage — the whole `GXSetTevKColor` …
`GXSetFogRangeAdj` stretch).

## Next targets

`AXFreeVoice` landed. `AXAcquireVoice` is parked (see blockers), and because a
slice must be one contiguous range, everything after it in `AXAlloc.c`
(`AXSetVoicePriority`, and `__AXAuxInit` in `AXAux.c`) is gated behind it.

### Game code in `wiimj2d.dol` — the better pool

A survey of all ~90 undone game-code TUs corrected a long-standing assumption:
**header completeness is mostly a *consequence* of a TU being finished, not a
resource available beforehand.** Most `include/game/**` headers covering undone
classes are on-demand stubs (`u8 mPad[0x74]`), declaring only what some
already-finished file needed to call. Only `dIceMng_c` and `daPyDemoMng_c` have
genuinely useful pre-existing layout. So game code does trade register-allocation
stalls for class reconstruction — but that trade is worth taking, because class
reconstruction is mechanical (read member offsets off `lwz`/`stw` displacements)
and it *terminates*, whereas the GPR wall above has no known source-level lever.

Useful technique found: **`__sinit_<file>_cpp` symbols recover the original TU
filenames** — 180 in the DOL, 129 of them in fully-undone TUs. Filenames are not
guesswork.

#### `d_wm_csvdata.cpp` — DONE, all 41 functions banked

Landed in full: 8.475% → **8.624%** (+9,728 bytes), all five binaries verifying.
The 28 remaining functions were authored by six parallel agents and **every one
matched**. This TU is closed; the notes below are kept for what they teach.

The blocker recorded earlier was real but narrower than it looked.
`d_wm_lib.hpp` *is* required — it supplies `sc_ForceList__6dWmLib` at
`0x8031C000`, where the TU's `.data` starts — and the `__sinit` it drags in lands
correctly at `0x800F6050`, immediately after `isLineEnd`. It only looked fatal
while the TU was being banked in pieces. **Whole-TU-or-nothing was the right
diagnosis; "the include is poison" was not.**

##### The real cost was section bounds, not code

All 42 functions compiled to the right instructions on the first full build. The
next two hours went entirely into the slice's `memoryRanges`, which were missing
`.ctors`, `.bss` and `.sbss` **entirely**, and whose `.sdata` stopped 8 bytes
short of `ReadRouteFlag`'s `"A"`/`"B"`/`"C"` literals.

**Every wrong bound shifts every `r2`/`r13`-relative offset after it**, and that
shows up as hundreds of scattered single-byte diffs spread across the *whole*
binary — 846 bytes in 447 regions, nearly all of them thousands of functions away
from the file being worked on. Do not chase those individually. When diffs are
one byte wide, land on odd addresses, and are spread binary-wide, the cause is a
small-data section bound, not code.

Diagnostic order that worked, cheapest first:

1. Compare **section sizes** first (`.text` short by 0x20 → a missing function).
2. Then per-**function sizes** ours vs the symbol map — if they all match, the
   code is right and the problem is placement.
3. Only then compare bytes.

Read the true bounds out of `bin/dtk/wiimj2d_symbols.txt` by listing the symbols
on either side of the TU's objects; the neighbours give both ends exactly.

##### `.sdata2` order is not what it looks like

The TU's `.sdata2` is **constants first, then the three `__sinit` floats**
(`0x8042D240`–`0x8042D278`), not floats-then-constants. The floats at
`0x8042D230` belong to the previous TU. The only way to see this is to **decode
the `r2`-relative offsets in the original `__sinit`**: `_SDA2_BASE_` = `.sdata2`
start + `0x8000` = `0x80433360`, so `lfs f2, -0x60F4(r2)` resolves to
`0x8042D26C` — after the constants, not before. Two hours of layout theorising
collapsed into one arithmetic check; do that check first next time.

##### `static const int` at function scope allocates storage

`c_ACTION_NAME_LEN` had to become an `enum { ... }`. As a function-local
`static const int` MWCC emits a real word into `.sdata2`, which the original does
not have — even though the value itself is folded into an immediate at the use
site. Use `enum` for function-scope integer constants unless the binary shows an
object.

##### Two source idioms that are load-bearing

Both are in the file with comments; do not "clean them up":

- `SearchChildPointName` writes its sentinel through
  `(&route->mChildPointName[n * 5])[5]`. Folded to `[n * 5 + 5]`, MWCC computes
  `(n + 1) * 5` and emits `addi/slwi/add/stbx` instead of the target's `stb` with
  a `0x5` displacement.
- `appendChildFromModel` re-reads the child chain through a separate *non-const*
  `ResNode`. Read through the const parameter, MWCC common-subexpresses the load
  with the one at function entry and loses an instruction.

##### Names: check `syms.txt` before inferring

Two of the six "unnamed" functions were already named.
`RouteData_t::deleteChildPointName` (`0x800F5B90`) was sitting in `syms.txt` all
along, and `appendChildFromModel`'s exact signature fell out of `SetRouteInfo`'s
call sites once that matched. **Grep `syms.txt` and the symbol map before
inventing a name.** Genuinely inferred names are marked `@unofficial`.

##### Alignment rule (verified across all 140 compiled objects, 682 placements)

**MWCC gives a `.data`/`.rodata` object 8-byte alignment iff its size is a
multiple of 8; otherwise 4.** It is a property of size alone — no source-level
change alters it. Do not waste time trying to re-shape a declaration to move an
object to a 4-mod-8 offset; if an object will not sit where you expect, the
section's *base address* is wrong, not its alignment.

##### Corrections to earlier notes in this file

- The `.sdata2` value `0x14` at `0x8042D244` is **`c_GHOST_ID__10dCsvData_c`**, not
  the inferred `c_ACTION_NAME_LEN`. The value matching 20 was a coincidence, and it
  was wrongly cited as confirmation. **Superseded:** the TU's `.sdata2` is
  `0x8042D240`–`0x8042D278` — the eleven `c_*_ID` constants **first**, then three
  `__sinit` floats. The earlier reading (`0x8042D230`–`0x8042D26C`, floats first)
  was also wrong; see "`.sdata2` order is not what it looks like" above.
  The eleven values are `c_COURSE_ID` 0, `c_GHOST_ID` 20, `c_TOWER_ID` 21,
  `c_CASTLE_ID` 23, `c_KINOKO_ID` 25, `c_ENEMY_ID` 32, `c_CANON_ID` 35,
  `c_TRSHIP_ID` 36, `c_AIRSHIP_ID` 37, `c_START_ID` 38, `c_PEACH_ID` 40.
- The `.sdata` base is `0x80428C18` (`"F7C0"`, `"W7C0"` first), not `0x80428C28`.
- The static-numbering rule (first unsuffixed, then `@0`, `@1`, …) **is** correct —
  independently verified on `createLayout` in `d_CourseSelectGuide.cpp`, which has
  eight statics numbered exactly that way.

##### Gotcha for `nonMatching` slices

Both ends of a `nonMatching` range must be **8-aligned** (16 for `.text`), or the
filler objects get re-aligned and everything after them shifts. An end of `0x1f0c`
on `.sdata2` produced 6,523 byte diffs.

#### The original `d_wm_csvdata.cpp` result — the proof this pool works

Four functions matched byte-for-byte (`read`, `~dCsvData_c`, `initialize`,
`RouteInfoInit` — 1,380 B). **No register-allocation wall was hit.** The hardest
function in the TU (`RouteInfoInit`, 996 B of scheduled `stb` storms) went from
first draft to byte-exact in ~6 iterations, every one a shape question with an
objective answer. That is the contrast with the SDK work, and it is why this pool
is the right place to spend effort.

The `dCsvData_c` layout (0x16518) is fully reconstructed and verified in
`include/game/bases/d_wm_csv_data.hpp`, so **the expensive shared prerequisite is
already paid** — remaining functions read their offsets straight out of the header.

**Next run: `ReadCsvData` → `ReadAction` (~7.4 KB, 12 functions).** Two things
gate it:

1. **Unblock `d_res_mng.hpp` first.** `ReadCsvData` calls the 3-argument
   `dRes_c::getResSilently` through `dResMng_c::mRes`, which is private with no
   3-arg wrapper. Adding two inline overloads is one line each and has no codegen
   effect elsewhere (unused inlines), but nothing can proceed without it.
2. **The `.data` pool must close in one pass.** The TU's data runs
   `0x8031C030`–`0x8031C144` and ends with `__vt__10dCsvData_c` at `0x8031C138`.
   Any partial set of literal-emitting functions puts the vtable at the wrong
   address. So everything from `ReadCsvData` to `ReadAction` lands together, or
   not at all.

If fanning out agents here: functions can be **authored** in parallel (each diffs
independently against the target disassembly), but they must be **banked in a
single integration pass** because of that `.data` constraint. One agent per
function, then one integration commit.

Best game-code candidates (`fp%` = float instruction density; keep it low, since
float/virtual-heavy code is where allocation actually goes wrong):

| Start | Bytes | Fn | TU | fp% | Notes |
|---|---|---|---|---|---|
| `0x800F3550` | 10,740 | 41 | `d_wm_csvdata.cpp` | **0%** | zero indirect calls, 6 ext classes all declared; cleanest object in the DOL |
| `0x800E5510` | 7,380 | 36 | `d_tag_processor.cpp` | 19% | `.cpp` + header already exist, 0x30 done; extend backwards |
| `0x8003C9F0` | 4,576 | 46 | `d_a_en_super_bigpile.cpp` | 9% | smallest complete enemy actor; prices the actor-TU pattern for ~40 near-identical siblings |

Avoid until the method is proven: `dBc_c` (fp 36%), `dBg_c` (39%), `daMask_c`
(27%), `dWmSpline_c` (45%), `daYoshi_c` (55 external classes).

Caveat worth keeping in view: ~99% of all remaining work is in the four `.rel`
modules, which have 0.3–2.3% symbol coverage and are not workable until a symbol
map exists. DOL game code is a 2.47 MB pool drained a few kB at a time.

### SDK targets, ranked by expected cost:

1. **`GXLight.c` forward** — 1,560 B across 11 functions from `0x801C65B0`,
   contiguous with the current slice, all header-described. Gated on
   `GXInitLightSpot` (412 B), which is **one register away** — see the
   reconstruction below. Behind it sit five trivial field-copy functions
   (`GXInitLightPos` 16 B, `GXGetLightPos` 28 B, `GXInitLightDir` 28 B,
   `GXGetLightDir` 40 B, `GXInitLightColor` 12 B) and `GXSetChanAmbColor` /
   `GXSetChanMatColor` (216 B each).

   The reconstruction below gives **exact size (412 B), 103/103 instructions in
   the same order, and a byte-identical `.sdata2` literal pool**. `aa`=f3,
   `ac`=f6 and `cr`=f5 all match the target; only `ab` lands in f2 where the
   target uses f1. Requires `cos=0x802E82AC` in `syms.txt` (already added),
   `#include <math.h>`, slice `.text` `0x1bfdf0-0x1bffd0` and `.sdata2`
   `0x3140-0x316c`.

   Note the `.sdata2` pool order is fixed by *source order of first use* —
   `0.0, 90.0, M_PI, 180.0, -1000.0, 1000.0, 1.0, 2.0, -4.0, 4.0, -2.0` — so the
   statement order inside each case is load-bearing and must not be rearranged.

```c
void GXInitLightSpot(GXLightObj *light, f32 angle, GXSpotFn fn) {
    GXLightObjImpl *impl = (GXLightObjImpl *)light;
    f32 cr;
    f32 aa, ab, ac;

    if (angle <= 0.0f || angle > 90.0f) {
        fn = GX_SP_OFF;
    }

    cr = cos(angle * M_PI / 180.0f);

    switch (fn) {
    case GX_SP_FLAT:
        aa = -1000.0f * cr;  ab = 1000.0f;  ac = 0.0f;  break;
    case GX_SP_COS:
        ab = 1.0f / (1.0f - cr);  aa = -cr * ab;  ac = 0.0f;  break;
    case GX_SP_COS2:
        ac = 1.0f / (1.0f - cr);  ab = -cr * ac;  aa = 0.0f;  break;
    case GX_SP_SHARP: {
        f32 d = 1.0f / ((1.0f - cr) * (1.0f - cr));
        aa = d * (cr * (cr - 2.0f));  ab = 2.0f * d;  ac = -d;  break;
    }
    case GX_SP_RING1: {
        f32 d = 1.0f / ((1.0f - cr) * (1.0f - cr));
        ab = -4.0f * (1.0f + cr) * d;  ac = 4.0f * d;  aa = ac * cr;  break;
    }
    case GX_SP_RING2: {
        f32 d = 1.0f / ((1.0f - cr) * (1.0f - cr));
        ab = 4.0f * cr * d;  ac = -2.0f * d;
        aa = 1.0f - d * (2.0f * cr * cr);  break;
    }
    case GX_SP_OFF:
    default:
        aa = 1.0f;  ab = 0.0f;  ac = 0.0f;  break;
    }

    impl->aa = aa;
    impl->ab = ab;
    impl->ac = ac;
}
```

   Tried without effect on `ab`: every ordering of the `aa`/`ab`/`ac`
   declarations; `d` at function scope first, last, and scoped per case; an
   explicit `r = angle * M_PI / 180.0f` intermediate; and writing the `GX_SP_COS`
   case as `ab = 1.0f - cr; ab = 1.0f / ab;` to force coalescing (that one does
   coalesce, but the allocator then puts the whole chain in f2 and moves the
   constant to f1, so it is strictly worse).
2. **`GXAttr.c` forward** — 1,884 B from `0x801C4910`, contiguous with the
   current slice. Gated on `GXSetTexCoordGen2` (580 B).
3. **`GXSetTevColor` pair** (196 B) — unlocks 2,464 B. See blocker #3. Now
   instruction-identical; only a temp-register rotation is left.
4. **`GXGetViewportv`** (32 B) — unlocks 1,064 B. See blocker #2; only the
   store scheduling remains.
5. **`OSAlloc.c`** (368 B) — needs the `Heap` struct reconstructed; it is
   file-local, not in `OSAlloc.h`.

### Finding new targets

Rank candidates by whether the project's headers already describe everything the
function touches. That predictor has been near-perfect: units with complete
headers match first try, units needing new struct reconstruction do not.

`tools/find_targets.py` automates this — it cross-references declarations in the
project's include dirs against contiguous undecompiled runs in
`bin/dtk/wiimj2d_symbols.txt` and ranks them:

```bash
python tools/find_targets.py 300 100000 0.85
```

Args are `min_size`, `max_size`, `min_coverage`. Runs break on a decompiled
function or a gap wider than CodeWarrior's 16-byte intra-TU function alignment.

What it currently reports, and the important caveat: only **two** runs in the DOL
score above 85% header coverage, and *both* are gated behind the blockers above.
Lowering the threshold to 0.5 surfaces much larger runs — 69,644 B at
`0x801B3280` (63%), 37,888 B at `0x801A04C0` (58%), 25,488 B at `0x801AC980`
(57%) — but these span several translation units and contain functions with no
header declaration, so they need reconstruction work first.

Note the tool reports *runs*, not translation units. A new slice may start at any
TU boundary (the `bin/dtkspl/obj/auto_NN_ADDR_text.o` split points are good
candidates) and cover a prefix of that TU, but it cannot skip a function in the
middle of its own range, and a source file gets exactly one slice entry.

## Original-game bugs found — reproduce, do not fix

Three this session. Each is required for the match and each will look like our
error later:

- `getOkCancellDisp(MsgRes_c*, void*)` and `getCourseSelectIcon` /
  `getCourseSelectButtonFunction` leave their message-id local **uninitialised**
  on the fall-through path. The target never sets `r6` before the tail call.
  No `default:`, no final `else`.
- `RuBySet` sets the ruby scale **transposed** — `mScale.x` from `mScale.y` and
  `mScale.y` from `mScale.x`.

Source idioms that are load-bearing and look like mistakes — all commented in
place, do not tidy:

- `(&route->mChildPointName[n * 5])[5] = '0';` in `SearchChildPointName`.
  Folded to `[n * 5 + 5]`, MWCC computes `(n + 1) * 5`.
- `appendChildFromModel` re-reads the child chain through a **separate non-const**
  `ResNode`; reading through the const parameter lets MWCC CSE it away.
- `ScaleCalcRect` makes three `GetFontHeight`/`GetFontDescent` calls whose
  results are **discarded**. Keep them, in order.
- `SetRouteInfo` increments a counter and immediately uses `counter - 1`.
- `x / 2.0f` and `x * 0.5f` are **not** interchangeable: MWCC folds the divide
  into `fmuls` by 0.5f but keeps the converted value as the *left* operand. Both
  orders legitimately coexist in one function.
- `switch` and `else if` are **not** interchangeable: a `switch` emits all
  compares up front then the bodies; interleaved compare/body/branch means an
  `else if` chain.
- `GX_U16`, not `GX_S16`, for `GX_VA_TEX0` (compType 2). The nw4r-idiomatic
  choice is the wrong one.

## What not to repeat

- Do not hand-write assembly for functions the original wrote in C
  (`GXLoadPosMtxImm`, `GXGetViewportv`). It would match, but it is not a
  decompilation. Inline `asm` is correct only where the SDK itself used it —
  `PPCArch.c`, the `OSCache.c` cache ops.
- Do not spend more than two or three attempts on a function whose size has
  stopped changing. That is the register-allocation wall, and further guesses do
  not converge. Park it and move on; a function left in the filler costs nothing.
