# Handoff

Working notes for continuing the decompilation work on branch
`claude/game-decompilation-setup-bw30s7`.

---

# START HERE: the parallel work plan

**Strategy in one line:** stop pushing on Revolution SDK code (it stalls on
register allocation with no known lever) and drain game code in `wiimj2d.dol`,
where actor TUs hit **no such wall at all** — the two most recent landed 99/99
(`d_a_en_bros_base.cpp`) and 97/97 (`d_a_en_blockmain.cpp`), 24,716 bytes
between them, both byte-exact with nearly every function right on its first
compile.

## What parallelises, and what does not

| Stage | Parallel? | Why |
|---|---|---|
| Mapping the TU against solved siblings | **Yes, and do it first** | Pure reference work, no shared state. It is what makes the authoring stage cheap. |
| Reconstructing a class layout | **No** | It is the shared prerequisite for every function in the TU. Parallel agents would invent conflicting layouts. One agent, first — but it runs *alongside* the bounds and mapping agents. |
| Deriving section bounds | **Yes** | Symbol-map arithmetic, independent of the code. Run it while the class is being reconstructed. |
| Authoring a function | **Yes** | Each agent writes its own `.cpp` in scratch, compiles it standalone, and diffs against the target disassembly. No shared state. |
| Banking (slice + full build + verify) | **No** | `slices/wiimj2d.json`, `syms.txt`, `bin/` and `ninja` are all shared. One integrator, serially. |

**The rule that follows:** agents *author*, the lead *integrates*. Never let two
agents run `ninja` in the same checkout — they clobber each other's objects and
each other's diffs.

### How an agent iterates without the shared build

An agent does not need `configure.py`/`ninja` to check one function. Compile the
file standalone and diff the disassembly. **The seven `BTE` include paths are
required** — without them anything including `d_audio.hpp` fails, which is most
actor code:

```
compilers\Wii\1.1\mwcceppc.exe -c -proc gekko -fp hard -O4 -inline noauto
  -Cpp_exceptions off -enum int -RTTI off -ipa file -enc SJIS -DREVOLUTION -I-
  <scratch>\draft.cpp -o <scratch>\draft.o
  -i include -i include\lib -i include\lib\MSL -i include\lib\MSL\internal
  -i include\lib\revolution\BTE\include -i include\lib\revolution\BTE\stack\include
  -i include\lib\revolution\BTE\stack\btm -i include\lib\revolution\BTE\bta\include
  -i include\lib\revolution\BTE\bta\sys -i include\lib\revolution\BTE\gki\common
  -i include\lib\revolution\BTE\gki\platform

bin\dtk-windows-x86_64.exe elf disasm <scratch>\draft.o <scratch>\draft.txt
```

Then diff with `tools/auto_decomp/harness.py`'s `extract` / `diff_fn` — import
them, do not write your own. **A negative control is necessary and no longer
sufficient**: of the six confirmed tooling defects, three were invisible to one.
Before trusting any comparison, work the checklist in "Verify your verification
tool" below — in particular, extract by **address** and assert the extracted
instruction count × 4 against the symbol map.

`harness.compile_draft(src, obj)` already encodes the flags above, so calling it
directly is less error-prone than reproducing the command line.

Build the argument list as a PowerShell array and splat it (`& $exe @args`);
long inline arg lists are fragile on PowerShell 5.1. An earlier version of this
section pointed at a `fndiff.py` that no longer exists in the repo.

## Read this first if you are picking the project up

- **Position: 11.088%** (720,792 / 6,500,368); `wiimj2d.dol` **21.887%**. Five
  binaries verify, tree clean.
- **70 commits are unpushed** (this handoff commit included). Nothing has been pushed for the whole of the
  2026-08-12/13 session. Ask before pushing.
- **The whole pakkun family is DONE and linked** — `d_a_en_dpakkun_base.cpp`
  64/64, `d_a_en_dfpakkun.cpp` 33/33, `d_a_en_jimen_pakkun_base.cpp` 67/67. The
  last of those landed on its **first build** with no `keepWeak`, no `syms.txt`
  entry and no deadstrip diagnosis — the accumulated rules now front-load that
  work instead of paying for it in failed builds.
- **`d_a_en_bros_base.cpp` is DONE and linked — 99/99, 12,112 B.** Every
  function byte-exact and the emitted symbol order matching the target's address
  order. 58 of the 99 were bit-identical to code already in the repo; the front
  stage found that before anyone wrote a line.
- **`d_a_en_blockmain.cpp` is DONE and linked — 97/97, 12,604 B of code in a
  13,232 B span** (`.text 0x1a130-0x1d4e0`; the 628 B difference is 16-byte
  function alignment, and progress counts the span). It is the largest clean
  base in the DOL by code bytes — its derived block actors live in
  `d_enemiesNP.rel`. **Ten of its functions are file-statics with no symbol-map
  name** (2,800 B, 22% of the unit, including its two largest); the names in our
  source are invented and marked `@unofficial`.
- **`d_a_en_hatena_balloon.cpp` is DONE and linked — 81/81, 18,216 B of bodies
  in an 18,768 B span.** The whole `wip/hatena_balloon/` scaffolding has been
  retired; it is recoverable from history if a verifier is ever wanted again.
  Two findings from landing it are written up below and are worth more than the
  file: **trial-link a unit before its last function closes** (see "Trial-link
  early"), and the two-word gap needed **two coupled source changes**, either of
  which alone scores far worse than the defect it repairs.
- **The "43% sibling precedent by bytes" figure for hatena_balloon was WRONG**,
  and the way it was wrong generalises: it was true at the *name* level and false
  at the *body* level. `model_set` and `setCcLine` both score **e=0.077** against
  their same-named banked counterparts, and `model_set` has no body precedent
  anywhere. **Name-sharing is not body-sharing** — check a score before planning
  around reuse.
- `d_a_player_hio_ADJ.cpp` still has one function left but it is a characterised
  dead end on its current axis; see its entry below before spending on it.
- **New headers this session:** `d_a_en_bros_base.hpp`, `d_a_en_blockmain.hpp`,
  `d_block_mng.hpp`. `d_a_en_blockmain.hpp` is now the best model in the repo
  for a large `dEn_c` actor — read it alongside the playbook's style references.
- **Do not re-derive the technique rules.** They cost ~4,000 agent tool calls to
  establish. The levers list and the three whole-binary failure signatures below
  (section size, `.ctors` index, `.sbss` size) are the most valuable part of
  this file.
- **Read "The method that works" and "Running the parallel pipeline" below
  before assigning anything.** The first says what the stages are — it took
  `d_a_en_lkuribo_base.cpp` from nothing to byte-perfect with every function
  matching on its first compile — and the second says how to run them, from two
  ~100-function units in one session.

## The method that works: map the siblings BEFORE authoring

This is the single biggest process finding so far, and it is worth more than any
individual lever in this file.

Before fanning out authoring agents, run **one agent whose only job is to
mechanically compare the target's instruction words against every
already-matching function in the repo**, masking branch targets and `@ha`/`@l`
halves (those are relocations). It produces, per function, the precedent to copy
and what specifically to take from it.

On `d_a_en_lkuribo_base.cpp` that map found:

- one function **byte-for-byte identical** to its Goomba counterpart;
- four more differing from theirs in **1–6 words out of 31–84**, every difference
  a relocation — i.e. the register allocation was already correct;
- three state initialisers **bit-identical to each other**, and two state
  bodies differing in **one word**;
- two sound functions bit-identical, a third differing in one immediate;
- and, just as usefully, the **four functions with no precedent anywhere**, so
  effort could be concentrated there instead of spread evenly.

Result: 58 functions, six parallel authoring agents, **every function matched on
the first compile** and only two needed any sweeping at all. Compare with the
previous session, where a single function cost ~300 builds across 8 sweeps and
then closed in half an hour once someone finally compared it against a sibling.

**Cost one agent. Do it every time.** Also give every authoring agent the
verified class declaration up front (see below) so nobody re-derives it.

**`tools/sibmap.py` now does the mechanical part.** It disassembles a target
range and scores every function against a corpus built from all matching slices
*plus* our own compiled objects — the latter matters, because those carry the
weak copies the linker discarded. Two views per pair: raw instruction words, and
a shape view with immediates masked so "same code, different member offsets"
still scores high. On `d_a_en_dfpakkun.cpp` it found seven animation setters
that are one body, 34% of the file. Its target range comes from argv
(`083deab`). `tools/datarefs.py` is its counterpart for data bounds — see below.

**Its `FAMILY` list rots, silently, and that is the part to remember.** A
FAMILY entry matching no corpus file contributes no hits and raises nothing, so
a stale list just makes the map thinner — one run silently lost 199 functions.
This session needed **six new members** added (`f9f8821`) plus a `CMP_`-prefix
tolerance and a dead-entry warning (`8f323f0`). **Add every newly banked enemy
TU to `FAMILY` the day it lands** — the newest entries are the most valuable
(`d_a_en_bros_base` alone contributed 99 precedent functions), and the dead-entry
warning goes to **stderr**, so capture it. Full account under "Verify your
verification tool".

Repeated on `d_a_en_dfpakkun.cpp` (72 functions, six agents): **33/33 authored
functions matched on first compile**, including a 944-byte state machine and a
572-byte `createMdl`, neither with any precedent. `d_a_en_bros_base.cpp` (99)
and `d_a_en_blockmain.cpp` (97) then landed on the same pipeline back to back.
The method is confirmed on four units at scale.

### The order that worked

1. **Two agents in parallel, blocking:** one reconstructs the class from the
   vtable and *proves* it byte-exact; one derives the section bounds. These are
   independent, and the bounds work is pure symbol-map arithmetic that needs no
   code. (**Skip the bounds agent when both neighbours are banked** — the lead
   can subtract in a few minutes. See "Bounds may not need an agent at all".)
2. **The mapping agent**, in parallel with those.
3. **Six authoring agents**, each given the verified class, the map's entry for
   its functions, and the shared data inventory.
4. **The lead assembles, integrates and builds.** Never let an agent run `ninja`.

Relaying findings between agents mid-flight paid repeatedly — a corrected vtable
table, a comparator bug, a caller-set correction and an argument-ordering lever
all reached agents while they were still working.

### Prove the class before anyone writes against it

Compile the class declaration plus an out-of-line stub for every virtual,
disassemble, and compare the emitted `__vt__` against the original's, entry for
entry. Run a negative control (swap two entries; the comparator must report
exactly 2 diffs) so the pass is not vacuous. Six agents then authored against
that declaration and none of them had to touch it.

**A vtable proof does not prove the data members**, and a constructor's own
store pattern is not sufficient evidence for a layout. `dPyMdlBase_HIO_c` was
reconstructed from its constructor, matched byte-for-byte locally, summed to the
class's independently known `sizeof`, and was **still wrong**: the constructor's
stores were consistent with `float m_08[8]` starting at 0x4, but the real layout
is a separate `float` at 0x4 and `m_08[7]` starting at 0x8. Both models produce
identical constructor bytes and identical totals.

What caught it was an **external consumer**: `daPlBase_c::setLandSmokeEffectLight`
in the already-banked, already-matching `d_a_player_base.cpp` indexes
`m_hio.m_08[]` from a fixed address **+8**, not +4. That file could not be
wrong, so the header was.

**So: for any member another file touches, grep the repo for its uses and
confirm your layout agrees with already-matching code.** Local self-consistency
is not proof. It also means the real test of a layout is the full link, not the
per-function diff — this one only surfaced when the TU was actually linked.

## Running the parallel pipeline

Two units landed this session on the same pipeline: `d_a_en_bros_base.cpp`
(99 functions, 12,112 B) and `d_a_en_blockmain.cpp` (97 functions, 12,604 B,
ten of them file-static with no name in the symbol map). Both byte-exact, both
linked, and **nearly every authored function matched on its first compile**.
The method above is not changed by this; what follows is what running it twice
back to back taught about the *orchestration* — where an agent slot is wasted,
where a brief has to be sharper, and the three checks that catch what
per-function diffs cannot.

Read this as the operational companion to "The method that works": that section
says *what* the stages are, this one says how to run them.

### The front stage

#### Bounds may not need an agent at all

If the TUs on either side are already banked and verifying, **your TU is exactly
the gap between them in every section** — subtract, do not derive. On blockmain
**every** section range fell out of pure subtraction, and three were
cross-checked independently — `.ctors` against a direct read of the DOL,
`.rodata` against the address of a known function-local static, `.sdata2`
against the address of a known static. No bounds agent was assigned, and that
freed an entire slot for authoring.

So the front stage is now: **check the neighbours first, and only staff a bounds
agent if one side is unbanked.** With both sides banked, the lead does the
arithmetic in a few minutes while the class-proof and mapping agents run. Note
that the `.ctors` index falls out the same way, and it is otherwise the source
of the one-byte whole-binary failure documented below.

Two things to expect while doing it:

- **A section where the two neighbours are ADJACENT means your TU claims
  NOTHING there.** That is a legitimate, safe answer, not a derivation failure.
  Write the empty range down explicitly so nobody re-opens it.
- The `baseID_*` rule below turns the `.text` low bound into subtraction too,
  which is usually the one range people expect to have to derive.

#### An actor TU's `.text` begins on its own `baseID_*` block — ten slices, no counterexamples

The weak `baseID_<StateName><10sStateID_c>` instantiations belong to the object
that **uses** them and are emitted at that object's **start**, not appended to
the previous one. So a TU's `.text` low bound is the first `baseID_*` naming one
of *its* states, not its first real function. Getting it wrong breaks all five
binaries; on bros it was worth 0x60 bytes at the low end.

Cheapest confirmation, unchanged and still worth the two minutes: read the
`.text` low bound of the banked slice immediately **above** yours. If that
address holds a `baseID_*`, the rule holds in your neighbourhood *and* that
address is your upper bound.

(It does not bite on a TU whose states are plain `sFStateID_c` rather than
`sFStateVirtualID_c` — those emit no `baseID_*` blocks at all.)

#### Hold the sibling map to a higher standard than last session

The split verdict from bros repeats exactly: **the map's similarity SCORES have
been reliable across three units; its PROSE has not.** On one unit four written
claims were wrong — a named precedent that was not the closest body, a "twin
pair" that was two unrelated bodies sharing only a 14-word head (an unaligned
differ lining up dissimilar tails), a claim that a class had no texture-pattern
animation when it did, and a member described as a speed field that was a centre
offset.

But the shortcut is still worth taking: two *other* twin claims in the same map
were real and each saved authoring a ~400-byte function outright. The asymmetry
is the point — a false twin costs more than no twin, because the agent spends
its effort deriving instead of authoring.

**The fix is to brief the map agent with those four failures as its spec.** Done
that way, the next map built a target-vs-target differ comparing **raw words AND
disassembly text** — raw words alone cannot see relocated callees, which is
precisely how the false twin scored high — then **confirmed four twin pairs,
rejected eight candidate pairs, and listed the rejects** so nobody would chase
them.

That is the standard: a twin claim ships **verified or not at all**, and the
rejected candidates are part of the deliverable.

### Briefing the batches

Everything in "Briefing authoring agents" below still applies. Add these five,
all of which cost something this session:

#### Start the batch that owns the shared helpers FIRST

On blockmain one batch owned four of the five shared helpers and four other
batches were waiting on its signatures. Either launch it first, or — at minimum
— **tell it explicitly that four batches are blocked on its signatures and that
it must report them early**, not in its final reply.

Corollary worth relaying when it happens: **if a helper's final signature turns
out to match what the proven header already declared, the callers need no
rework.** Confirm it and say so; silence leaves four agents assuming they owe
themselves a revision pass.

#### Assign every function from the map's per-function TABLE, not its prose

Two batches were briefed with the same function, because the brief followed a
prose summary while the map's own table assigned it elsewhere. Both authored it
and the two versions were **textually identical** — cheap, and two independent
derivations agreeing is real evidence. But the conflict should have been caught
at briefing time. **Trust the per-function table over any prose summary**, in
the map and in your own brief.

#### Assign DATA-object ownership explicitly

This one is not cheap. Two batches independently defined the **same data object
under different names**, and only one copy can land. Function ownership is
always spelled out; data ownership usually is not. Put every shared float table,
string and static in the brief with a named owner, and tell the others to
reference it, not define it.

#### Say who authors NOTHING

Some functions are **emitted but authored by nobody** — weak template
instantiations, inline flushes pulled in by a caller. They land in the right
place only if the call sites and the includes are right. Tell agents plainly:
**do not hand-write these, and report it if one comes out in the wrong place.**
An agent that "helpfully" writes one produces a duplicate or a misordered flush
block, and per-function diffs will not show it.

#### Shared headers are frozen once authoring starts

State it as a rule: **agents must not edit the shared class header.** If an
agent believes a change is needed, it should **shadow-copy the header into its
own scratch, verify the change there, and report it.** Several agents did
exactly this and were right to.

### Mid-flight

#### Relay findings WITH THEIR SCOPE

The failure mode this session was not late relays, it was over-broad ones. A
finding that was true within one batch's functions ("this TU writes float
comparisons constant-first") was relayed as a file-wide rule; **two agents
correctly pushed back with counter-evidence** — it holds for `fcmpu` against a
literal, not for member-vs-member `fcmpo`. An over-broad relay costs more than a
late one, because it invites correct work to be undone. Attach the scope — "in
these functions", "against a literal, not member-vs-member" — to every relay.

#### Brief agents to REPORT contradictions, not reconcile them

This is the instruction that earned the most this session. Two agents disagreed
about a `sizeof`. Because both were briefed to report contradictions rather than
quietly pick one, the disagreement surfaced — and taken the wrong way it would
have made a base class **0x12C bytes too small**, an error **invisible to every
per-function diff** and detectable only at the link.

Put it in the shared brief in the same breath as "do not modify the class
declaration": *if what you find contradicts the brief, the map, or another
agent's finding, stop and report the contradiction. Do not reconcile it
yourself.*

#### A tooling defect is worth interrupting everyone for

Unchanged and reconfirmed: when a defect turns up in a shared tool, relay it to
every running agent **immediately**. Several had already hit it independently
and were working around it in private.

#### Changing a shared header mid-flight

Sometimes the header does have to change. Two rules:

- **A change that alters mangling is not cosmetic.** Parameter-type changes
  change the mangled name — verify the new name against
  `bin/dtk/wiimj2d_symbols.txt` **before** applying it.
- **After editing, re-probe `sizeof` and the key offsets, and tell the running
  agents it is layout-neutral.** They are authoring against the old copy; they
  need to know whether to redo anything.

### Landing a change to a header that already-matching TUs use

Give this its own step, because the same class of change has broken all five
binaries earlier in the project.

Giving an inline body to a method that three already-matching, already-landed
units call is a **whole-project** change, not a local one. The procedure that
worked: **apply the change, rebuild, confirm all five binaries still verify —
BEFORE landing it with the new unit.** Done that way this session, and the
change proved safe. Landing it together with the new unit makes a five-binary
failure ambiguous between the header and the unit, which is the expensive kind
of failure to diagnose.

### Assembly

#### Assemble by ADDRESS, not by batch

The old rule — interleave the batches' functions, do not concatenate them — is
right but too weak, because it reads as though only functions interleave.
**Data-object ordering interleaves across batches too.** Source order controls
emission order for the literal pools as well as the code. On blockmain two
batches' `.rodata` tables sat **between** two of another batch's function-local
statics. There is no batch-level ordering to
preserve; there is only the target's address order. Build the canonical order
from the symbol map before the reports start arriving, and place every function
*and every data object* into it individually.

#### Some objects belong to no batch and must still be written

One unit's first `.sdata2` object was a static that **no agent authored**. It
had to be defined at the top of the file or the whole pool would shift. When you
lay out the address order, any object in your range that no report claims is
yours to write — treat an unclaimed object as a finding, not as noise.

#### Verify three ways

Per-function byte equality is one view of three, and it is the weakest on its
own:

1. **Byte equality per function.**
2. **Per-function SIZE against the symbol map.** Catches a missing or extra
   function faster than reading instructions.
3. **Emitted symbol ORDER against target address order.** This is the one the
   other two cannot see. Control it by deliberately moving one definition and
   confirming the check fires.

The full checklist those three sit inside is under "Verify your verification
tool".

#### Trial-link early — do NOT wait for the last function to close

**This is the most valuable process finding from the hatena_balloon landing, and
it generalises to every unit.** That unit sat at "80 of 81 byte-exact" and was
treated as one function away from done. It was not. Putting it in the build
while it still had a known-bad function — slice entry added, then
`nonMatching` cleared — found **two independent landing blockers in one build**,
neither of which any per-function diff can see:

- **An undefined symbol at link.** `dActorMng_c::envAllWaterCheck()` was called
  by our TU, but its defining TU is still undecompiled, so nothing in the link
  provides it. It needed a `syms.txt` entry at the original's address. Its two
  siblings already had one; this function had simply never been *called* by
  banked code before.
- **`.rodata` 0x20 short**, which shifted every following section and failed
  four of the five binaries with thousands of scattered single-byte diffs — the
  "wrong small-data bound, never wrong code" signature. Two compounding causes,
  and **each is a general trap**:
  - **`l_speed_ratiodt` (0x40) is emitted by the original but referenced by
    nothing** — not by this TU, not by anything. No authoring batch claimed it
    because no function calls it. This is the "some objects belong to no batch"
    rule with a sharper edge: an object can be invisible to the *entire* call
    graph and still have to be written. **`extern` is load-bearing on it** — at
    namespace scope a `const` array has internal linkage in C++, so as a plain
    file-scope `const` it is stripped as unused and the shortfall persists.
  - **The recorded range was itself wrong.** `0x6e40-0x6ef8` predated knowing
    about that object; the next TU's first object starts at `0x802F4EF8`, so the
    true range was `0x6e40-0x6f18`. A bound derived before the contents are
    known is a hypothesis, not a fact.

The payoff is that the trial link converts "80/81 functions match" into a much
stronger statement: **the whole 18,768-byte unit linked and the DOL differed
from the original in exactly 6 bytes**, with the other four binaries
byte-identical. That is a far better place to hunt a last defect from, and the
structural work is then *proven* rather than assumed.

**So: as soon as a unit is mostly assembled, put it in the build.** Land it
`nonMatching` if it still has a gap — the tree keeps verifying 5/5 and the
structural work is banked and reviewable. Do not save integration for the end.

**And note the `syms.txt` interaction runs BOTH ways**, which is easy to get
backwards: while a slice is `nonMatching` its bytes are spliced in, so other
binaries still resolve its profile symbol from `syms.txt` and the line must
**stay** — dropping it early failed `d_profileNP.rel` on its own. The moment the
flag clears and the object really links, that same line becomes a duplicate
definition and must **go**, in the same commit.

#### An object may be LARGER than its slice claims — unreferenced weak symbols are not placed

**This one cost three peer-AI rounds and two shared-header changes before anyone
checked it, and it invalidates the obvious reading of a section-size comparison.**

`bin/compiled/wiimj2d/dol/bases/d_multi_manager.o` — landed, banked, byte-exact,
verifying in all five binaries **right now** — emits thirteen functions. Ten are
its own. The other three are weak inline destructors pulled in by an `mVec2_c`
local: `__dt__7mVec2_cFv`, `__dt__Q23EGG8Vector2fFv`, `__dt__Q23EGG8Vector3fFv`.
Its slice claims `.text 0xc8170-0xc8580`, exactly `0x410`, which is the ten real
functions and nothing else. **The object is `0xC0` bigger than its claim and the
DOL is byte-identical to retail.**

So: **an unreferenced weak symbol emitted into an object does not have to fit
inside that object's slice `.text` claim.** The linker does not place it. Every
banked unit in this project that touches an `mVec2_c` or `mVec3_c` local has been
carrying `__dt__Q23EGG8Vector2fFv` / `__dt__Q23EGG8Vector3fFv` the whole time,
invisibly and harmlessly — neither has ever existed in the retail symbol map.

Two expensive consequences, both of which actually happened:

- **Comparing a compiled OBJECT's section size against a slice claim is not a
  link-time overflow measurement**, and must not be written up as one.
  `wip/player_manager/TRIAL_LINK.md` does exactly that — its table is correctly
  headed "compiled object vs claim" and was then read as though `.text`
  overflowed by `0x90`. `0x80` of that `0x90` is those two destructors, which
  will never be placed.
- **Both peer AIs independently proposed removing the inline destructors from
  `eggVector.h`**, from two different units, with good evidence each time. It was
  applied once and **failed all five binaries** — because it removes something
  the whole project legitimately relies on. Do not propose it again.

##### Settled at corpus scale, with its boundary

The above was one data point. It is now a measured rule, from a sweep of all
**143 landed units** and the **1,592 weak symbol instances** they emit:

| Fate of an emitted weak symbol | Instances | Share |
|---|---|---|
| Deadstripped — referenced nowhere in the binary | 161 | 10.1% |
| Deduplicated — surviving copy linked from its home TU | 818 | 51.4% |
| Placed — this TU holds the surviving definition | 613 | 38.5% |

**91 of the 142 landed units with `.text` (64%) compile to objects strictly
larger than their slice claims.** Carrying unplaced weak symbols is the normal
condition of this repository, not a defect. The test that matters is

```
effective .text = compiled .text - sum(unplaced weak symbols) == slice claim
```

**The boundary — the half that stops the rule being over-applied.** A weak
symbol that *is* the surviving definition does occupy its slice, at its retail
address. Confirmed on three banked units: `__ct__9fLiMgBa_cFv` (`0x10`) inside
`f_manager.cpp`'s slice, `__dt__Q23m3d11calcRatio_cFv` (`0x40`) inside
`calc_ratio.cpp`'s, and `__dt__26__partial_array_destructorFv` (`0xBC`) inside
`class_arrays.cpp`'s. So the rule is about *placement*, never about weakness on
its own.

**Why it happens, from the tooling.** `tools/gen_lcf.py`'s
`make_elf_force_directives` adds every `STB_GLOBAL` symbol to the linker
control file's `FORCEACTIVE` block, and **excludes `STB_WEAK` symbols unless the
slice names them in `keepWeak`**. `mwldeppc` dead-strips per function from those
roots. `keepWeak` is therefore the escape hatch for the case where retail kept
an unreferenced weak symbol anyway — `m_2d.cpp` uses it for `__dt__7mVec3_cFv`.

**Consequence for `d_a_player_manager.cpp`, which is what this was blocking:**
of its apparent `0x90`, `0x80` deadstrips (`__dt__Q23EGG8Vector2fFv`,
`__dt__Q23EGG8Vector3fFv`), `0x64` deduplicates to home TUs (`__dt__7mVec2_cFv`
to `d_2d.o`, `isItemKinopio__7dAcPy_cFv` to `d_ac_py.o`, two `daPlBase_c`
executors to `d_a_player_base.o`, `getPlrNo__8dActor_cFv` to `d_actor.o`), and
`getCourseIn__10dScStage_cFv` (`0x8`) is the surviving definition and belongs in
the slice at `0x8005EC90`. **Net overflow: zero. The unit is unblocked.**

The boundary is presumably referenced-vs-unreferenced: a weak symbol that is the
surviving definition for some other object must occupy space. That half is not
yet proven and is queued as a peer task.

#### Keep the name→address mapping from the merge

A mistake the lead made and paid for. Units contain file-static functions with
**no symbol-map name**, which agents name themselves — blockmain had ten. When
verifying, those invented names must be mapped back to their addresses.
Discarding them as "unmatched extras" **desynchronises a positional comparison
and produces dozens of spurious differences**.

So: as you merge the reports into address order, **keep the authoritative
name→address mapping as an artefact** and feed it to the verifier. Do not
reconstruct it from names afterwards.

### What this predicts for the next unit

With both neighbours banked, the front stage is one class-proof agent and one
mapping agent, no bounds agent, and the lead doing bounds arithmetic in
parallel. Five or six authoring batches, the shared-helper batch first, data
ownership assigned in the briefs, and the map's twin claims already verified or
rejected before anybody is briefed. Integration is address-order assembly plus
the three verifications. On that shape, two ~100-function units landed in a
session with almost everything matching first compile.

## Infrastructure state (as of the 2026-08-12/13 session)

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
  `d_a_player_hio_ADJ.cpp` complete at 15/16 — the only `nonMatching` slice left
  in `slices/wiimj2d.json`. `progress.py` is the authority; the local progress
  page corrects for this, decomp.dev will not.

- **The scratchpad is not tracked, and things have been lost in it.**
  `scratchpad/GXTev.c.best` and `diffall.py` are both referenced below and
  neither still exists. (`tu_extent.py` and `tu_split.py` were rescued — both
  now live under `tools/` and are git-tracked.) Anything worth keeping —
  a generator, a best-known-formulation, a comparator — belongs under `tools/`
  and committed, or pasted into this file.

## `tools/auto_decomp/` — the unattended harness, and what it is good for

Built to let a cheap model grind functions without supervision. Three parts,
deliberately separated so a weak model cannot damage the project:

- **`harness.py`** proposes, compiles, disassembles and diffs, writing only to
  `tools/auto_decomp/work/`. The model never reports success — it emits source,
  and the harness computes the match from real bytes — so a hallucinated "this
  matches" is structurally ignored. A missing function is a hard failure.
  `--auto` grinds a whole TU smallest-first and logs to `work/<unit>/decomp_log.jsonl`.
- **`land.py`** is the only script that touches `source/`, `include/`, `slices/`
  or `syms.txt`. It refuses a dirty tree, then runs configure, ninja and
  `progress.py --verify-bin`; unless all five binaries are byte-identical it
  restores every file it touched and exits non-zero. It never commits.
- **`prepare.py`** collects a TU's split objects into a work directory.
- `config.json` is gitignored and holds model endpoints. **It contains an API
  key — do not print it or paste it into a report.**

**The honest assessment: the cheap models cannot do this work.** A ~90-minute
unattended run against `d_a_en_lkuribo_base.cpp` closed exactly one function, an
empty one, and its final draft contained pseudo-C that does not compile —
`stfs(vol, 0xf8(this))`, a float declared as a string literal. The same TU was
then finished by six Claude agents with every function matching on its first
compile. Do not spend budget re-testing this; spend it on the sibling-mapping
method at the top of this file.

What the harness *is* still good for: its `extract` / `diff_fn` are the shared
comparator every authoring agent should import rather than rewriting, and
`land.py`'s all-or-nothing gate is a sound way to land a TU without leaving a
half-applied change behind. The `--auto` loop had never been run end to end as
of the lkuribo session and has not been exercised since; treat that claim as
unverified rather than as a finding.

**Two caveats on `prepare.py`, both found the hard way.** Its fuzzy `__sinit`
matcher once picked the *derived* actor's `__sinit` instead of the base's and
handed an agent a target **missing a 1,348-byte function** — a range sweep never
sees the omission. And a target usually spans several split objects and carries
functions from neighbouring TUs, so "N functions in target.txt" is not "N
functions in your TU". Cross-check the function list against
`bin/dtk/wiimj2d_symbols.txt` for your address range before trusting a count.

**`bin/dtkspl` is stale for banked ranges** — it predates every TU landed since
`d_a_en_dpakkun_base.cpp` (six of them: dfpakkun, lkuribo_base,
jimen_pakkun_base, player_hio_ADJ, bros_base, blockmain), so none of those has a
split object there. For those, ground truth is the `auto_*` objects or our own
compiled output. **It remains authoritative for any range that is not yet
banked**, which is the only way it is used for target selection — e.g.
`auto_03_801102B0_text.o` exists and its header reads
`# 0x801102B0..0x80114580`. Read it as valid for undone ranges, stale for done
ones.

## Where the work now stands

**11.088%** (720,792 / 6,500,368 bytes); `wiimj2d.dol` at **21.887%**. Five
binaries verifying, working tree clean, **70 commits unpushed**.

The 2026-08-12/13 session landed six TUs (~74,000 bytes), took the project
past 10%, landed five tool fixes across three tools and added the rules below.
The most recent is `d_a_en_hatena_balloon.cpp` (81/81, 18,216 B code / 18,768 B
span), the largest single unit landed so far; before it
`d_a_en_blockmain.cpp` (97/97, 12,604 B code / 13,232 B
span); before it `d_a_en_bros_base.cpp` (99/99, 12,112 B), and before that
`d_a_en_jimen_pakkun_base.cpp` (67/67), which landed on its **first build** with
no linker archaeology at all — the clearest sign the accumulated rules are now
doing real work up front.

Tooling changed this session, all committed, in commit order:
- `tools/datarefs.py` — **update-form load base-register writeback** (`f82d77e`).
  `lwzu`/`stwu` write the effective address back into rA; not modelling that left
  a stale base and resolved seven following `lwz`es ~0x1900 too high, into the
  next unit's `.rodata`.
- `tools/sibmap.py` — **six enemy TUs added to `FAMILY`** (`f9f8821`).
- `tools/sibmap.py` — **`CMP_` prefix tolerance and a dead-entry warning**
  (`8f323f0`), plus the `REL_` tag correction.
- `tools/auto_decomp/harness.py` — **placeholder names no longer collapse**
  (`47d15ca`). Every `fn_800XXXXX` was normalising to the bare string `fn`, so
  a diff against any unnamed function compared the wrong body.
- `tools/auto_decomp/harness.py` — **operand-level placeholder collapse**, plus
  the `sibmap` `REL_` tag correction (`3681c28`).

Earlier, and still current: `harness.py` **no longer erases branch destinations**
(`7fe054f`) — that defect was silently comparing every loop and conditional in
the project blind — and `datarefs.py`'s clobber-tracking bug is fixed; it was
corrupting every address chained off a `lis`/`addi` pair. All of these are
written up under "Verify your verification tool".

An earlier session added `d_a_en_lkuribo_base.cpp` (58 functions, 9,456
bytes, every function matching on its first compile) using the sibling-mapping
method documented at the top of this file. The session before took the project
from 9.133% to 9.681% across two batches of parallel agents. What has landed,
all banked *whole*:

| TU | Functions | Notes |
|---|---|---|
| `d_a_en_blockmain.cpp` | 97 | Largest clean base by code bytes; ten unnamed file-statics |
| `d_a_en_bros_base.cpp` | 99 | 58 of 99 bit-identical to code already in the repo |
| `d_a_en_jimen_pakkun_base.cpp` | 67 | Landed on its first build |
| `d_a_en_dpakkun_base.cpp` | 64 | Flipped in one commit with dfpakkun |
| `d_a_en_dfpakkun.cpp` | 33 of 72 | The rest are weak `daEnDpakkunBase_c` copies |
| `d_a_en_lkuribo_base.cpp` | 58 | Every function matched on its first compile |
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

The pakkun pair (19,808 bytes, the jump past 10%) had to be flipped out of
`nonMatching` in a single commit — see "The pakkun pair — DONE, and the two
rules it cost" for why, because it will recur on any base/derived pair in
flight together.

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

## Next target — and what is known about the rest

This is the only "next target" section for game code. The SDK list further down
is deprioritised (see the register-allocation wall) and
"### The `find_targets.py` tool and its limits" is about the tool, not about
what to do next.

`tools/find_targets.py` still ranks by header coverage. **Do not follow it
blindly**: its two top-ranked runs (`0x801C8570`, 2,464 B, 100% coverage;
`0x801C91E0`, 2,904 B, 95%) are both **gated by the two known register-wall
functions** — re-verified this session, still exactly those two. `GXSetTevColor`
sits at the head of `GXTev.c`'s queue and `GXGetViewportv` immediately before
`GXTransform.c`'s banked start, so extending backwards must cross it. One
contiguous range per section means neither can be skipped. High header coverage
says the *types* exist, not that the run is reachable.

**The best next move is the remaining ~24 actor TUs**, using the playbook below.
`d_a_en_bros_base.cpp` (99/99) and `d_a_en_blockmain.cpp` (97/97) landed back to
back on the same pipeline in one session, so the pattern is now priced at
roughly 100 functions per unit with nearly everything matching first compile.

### NOT READY TO LAND — and this corrects what I wrote an hour ago

`d_a_wm_grid.cpp` and `d_a_wm_tower.cpp` are **10/10 and 11/11 byte-identical in
`.text`**. I called them ready to land on that basis. **I tried it, and it fails
two binaries.**

```
d_profileNP.rel  Failed
d_basesNP.rel    Failed
```

Reverted immediately; the tree is green at 5/5 again. The cause, measured from
the compiled object against the slice claim:

```
section     claim   object
.text       0x1f4   0x2d0   over 0xdc   (weak symbols, expected, not placed)
.data       0x090   0x0a4   over 0x14   <-- THIS
.rodata     0x010   0x010   ok
.ctors      0x004   0x004   ok
.bss        0x010   0x010   ok
```

`.data` emits `0x14` more than the slice claims. Extra `.data` shifts every
object after it, including `g_profile_WM_GRID` — and `d_profileNP/d_profile.cpp`
holds a pointer table referencing that symbol, so a moved profile changes
`d_profileNP.rel` as well. That is why a `d_basesNP` unit broke the profile REL,
which otherwise looks unrelated and is 100% complete.

#### The method gap this exposes, which matters more than the unit

**`wip/wm_units/verify_anon.py` only checks `.text`.** Every "N of N" figure
quoted this week — grid, tower, smallcloud, kinoko_1up — is a statement about
code only. A unit can be byte-perfect in `.text` and still be unlandable because
its data sections are the wrong size or order, and nothing we run would say so.

**Before calling any unit ready, compare every section of the compiled object
against its slice claim**, the way `.text` is already compared. Note the
asymmetry: `.text` over-claim is normal (unreferenced weak symbols are not
placed — see the weak-symbol rule above), but **`.data` over-claim is real**,
because data objects are placed unconditionally.

Do not repeat the landing until grid's `0x14` of surplus `.data` is identified.
Likely candidates, in order: a duplicated copy of `dWmLib::sc_ForceList`, the
`DUMMY_ORDERING` pool seed emitting more than intended, or padding from a
declaration order that differs from the target's.

**Use the round-13 kit, not the round-10 one. They contradict each other and I
have settled it.** For `d_a_wm_grid.cpp`:

```json
{
  "source": "d_basesNP/bases/d_a_wm_grid.cpp",
  "memoryRanges": {
    ".text": "0x164210-0x164404",
    ".ctors": "0x3e4-0x3e8",
    ".rodata": "0x88b8-0x88c8",
    ".data": "0x44c90-0x44d20",
    ".bss": "0xfdd0-0xfde0"
  }
}
```

The evidence, and it generalises to every `wm` unit:

- At `0x164210-0x164404` the verifier finds **10 functions and matches all 10**.
  At round 10's `0x164230-0x164430` it finds 9. The wider range is the unit.
- `.data` starts at `lbl_2_data_44C90`, a `0x24` float object, which is
  **`dWmLib::sc_ForceList` — a header static in `d_wm_lib.hpp`**. Round 10
  started at `g_profile_WM_GRID` (`0x44CB4`) and so cut it off. The vtable
  follows at `0x44CC0`.

**`sc_ForceList` is the single most under-modelled thing in this REL.** It has
now explained, in four separate units: two whole functions in
`d_a_wm_kinoko_1up.cpp` that needed no hand-written source at all (its
static-init and destroy thunks), the `.data` aggregate that blocked
`d_a_wm_smallcloud.cpp`'s `createModel`, the `.rodata` pool ordering that
blocked `d_a_wm_grid.cpp`'s `__sinit`, and now a `.data` low bound. **If a `wm`
unit has an unexplained object, function or pool offset, check `sc_ForceList`
before anything else.**

**Two bound errors found in the surveys this week, so re-derive rather than
trust.** `d_a_wm_kinoko_1up.cpp`'s `.ctors` is `0x3fc-0x400` and *both* surveys
said `0x3f8-0x3fc` — caught from the REL's own relocation table and the split
object's emitted `.section` comment. And `daWmKinokoBase_c` is `sizeof 0x284`,
not the `0x2B0` recorded as "proven": the derived leaf's `classInit` allocates
`0x294`, which cannot exceed its own base.

**REL landing differs from DOL landing** and no REL unit has been landed yet:
RELs resolve through `alias_db.txt` and the DOL ELF symbol table rather than
`syms.txt`'s fixed addresses. Expect that to need working out on the first one.

### PARKED: `d_nand_thread.cpp` — 16 of 21 authored functions byte-exact, in one verified TU

**Status: parked as characterised, not abandoned.** The landing kit is complete
and recorded below, the header is landed and verifying, and the five remaining
functions have each been worked by three or four independent agents. What is
left is register-allocation and codegen-shape work of the category that has
converted **zero times** on this project; fresh authoring converts reliably. The
progress arithmetic says the same thing: this whole unit is worth `+0.063`
points, less than a third of what one `d_a_player_manager` is worth.

**The number is now measured rather than claimed.** Every previous count came
from separate agents each verifying its own subset in its own copy of the
source. Compiling all of them as one TU for the first time immediately exposed a
signature clash four agents had never hit (`cmdSave(const void *)` versus
`cmdSave(void *)`). **Merge before you trust a count.** The merged file is
`wip/nand_thread/scratch/merge_lead/d_nand_thread.cpp`.

Of the 21, `cmdSave` can only be verified by hand: the target calls it
`fn_800CF170` because it has no symbol-map entry, so no name-based diff reaches
it. Extract both by address and compare instruction lists — it matches.

Section evidence at the same time: `.rodata` `0x28`, `.data` `0xA0` and `.bss`
`0x17040` all come out **exactly** on claim, which is strong independent
confirmation that the data reconstruction is right. Do not compare the object's
`.text` size against the claim — see the weak-symbol rule above; that comparison
is meaningless and it has already cost this project three rounds once.

#### One open structural question, worth settling before landing

The target's TU tail holds `onExit` (`0x800CFCB0`), `onEnter` (`0x800CFCC0`) and
`run` (`0x800CFCD0`) — the `EGG::Thread` weak flushes. **Our object emits the
first two and not the third**, and an agent established across ~20
configurations a rule with no exceptions:

> MWCC weakly flushes exactly the subset of a base class's inline virtuals that
> the derived class does **not** override, regardless of vtable slot position,
> and never flushes an overridden slot.

`dNandThread_c` overrides `run` — proven from the retail vtable at `0x80317D48`,
whose slots are `[0, 0, ~dNandThread_c, dNandThread_c::run, EGG::Thread::onEnter,
EGG::Thread::onExit]`. So the rule predicts our behaviour, not the target's.

**The likely resolution is that those bytes are not ours.** `run__Q23EGG6ThreadFv`
is referenced by nothing in the TU and by no other object in the repo, and the
symbol map shows at least two other thread classes exist
(`run__Q24mDvd10MyThread_cFv`, `run__Q23EGG17ConfigurationDataFv`). If a
different `EGG::Thread`-deriving TU overrides none of the three, it flushes all
three and obeys the rule exactly. That would mean `d_nand_thread.cpp` ends at
`getSaveData` (`0x800CFCAC`) and its `.text` claim should be
**`0xc8580-0xc9530`**, not `0xc9560`, leaving `0x30` to the neighbouring TU. Test
that at landing time; if it holds, the eight-byte shortfall disappears and the
weak copies we do emit are deduplicated away harmlessly.

`0x800CED00`–`0x800CFCE0`, **24 emitted functions, 0xF48 (3,912) B of code in a
0xFE0 span.** Five of the 24 are compiler-emitted weak flushes nobody authors
(`__dt__Q23EGG5MutexFv`, `__dt__6mMutexFv`, and `EGG::Thread`'s `onExit` /
`onEnter` / `run`), all landing in the right places. Scaffolding is in
`wip/nand_thread/`; `SHARED-BRIEF.md` there is the authoritative fact sheet.

**Five near-misses remain**: `__dt__`, `spaceCheck`, `save`, `writeBanner`,
`load`. Each is characterised in `wip/nand_thread/CLOSE_A.md` / `CLOSE_B.md` /
`CLOSE_C.md`, with every variant already tried listed so nobody repeats them:

- **`__dt__`** — one word short. The target keeps a second, provably-redundant
  `this == 0` check before calling `~EGG::Thread()`. Five shapes ruled out,
  including one that hits the right instruction count with the wrong content.
- **`spaceCheck`** — 34/37. A register-allocator plateau: **24 source shapes
  across two agents**, every one preserving the instruction count, all landing
  on the same `r3`-vs-`r4` choice. Structurally diffed against the byte-exact
  `existCheck`, which explained why no third saved register is needed but not
  the divergence. Treat as a wall, not a to-do.
- **`writeBanner`** — 64/66. The `iconSpeed` store touches **two** adjacent
  2-bit sub-fields (frame 0 set, frame 1 cleared), not the one first assumed;
  reproducing both narrowed it. A real C bitfield emits a single `rlwimi` and is
  provably wrong; `volatile` and `static` locals defeat the fold but overshoot
  or allocate outside zero-slack section bounds. Its register-pressure gap and
  its bitfield-fold gap are one root cause, not two.
- **`save` / `load`** — now 2 and 1 instructions LONG rather than 9 and 12
  short, under the refined lever below. That overshoot is itself the clue.

A real defect was found in both drafts along the way and is worth carrying: the
guard around `NANDSimpleSafeCancel` is `mError == 6`, not `!= 6`, proven from
which side of the `beq` actually reaches the call, at all five occurrences.

**The header is landed and all five binaries verify with it.** It went from an
18-line `u8 mPad[0x74]` stub to the real class. The three vtables read out of
the DOL pin `EGG::Thread`'s virtual order as `~Thread`, `run`, `onEnter`,
`onExit`, and confirm that `dNandThread_c` introduces no new virtuals.

#### Nine signature corrections, and only two were visible to the symbol map

This unit is the strongest evidence yet for how blind symbol comparison is.

**Provable by name — check these mechanically, every time:**

- `setNandError` takes `long`, not `s32`. The symbol is
  `setNandError__13dNandThread_cFl`; `s32` is `signed int` here and mangles `Fi`.
- `EGG::Thread`'s constructor takes `unsigned long`, not `u32` — symbol
  `__ct__Q23EGG6ThreadFUliiPQ23EGG4Heap`, `Ul`. Same trap, same cause.
- `sCrc::calcCRC32` takes `unsigned long` — `calcCRC32__4sCrcFPCvUl`.

**Invisible to the symbol map — CFront omits return types entirely:** the four
`cmd*` functions are `bool` not `void`; `save`, `load` and `writeBanner` are
`s32` not `bool`; `deleteFile` is `void` not `bool`. Each was settled by
codegen alone. Two witnesses exist and both are worth using: **`run()`'s own
consumption of a result** (it tests `save() == 2`, which is not a truth test),
and **the function's own epilogue shape** (`li r3,1` / `li r3,0` converging at
one epilogue is `return true`/`return false`, not falling off the end).

#### The bool-materialisation lever — proven, and probably general

Writing `if (!OSTryLockMutex(...))` emits a plain `cmpwi`+`beq`. Writing

```cpp
bool locked = OSTryLockMutex(&mMutex.mOSMutex);
if (locked) { ... }
```

emits the target's three-instruction normalise sequence (`neg`/`or`/`srwi.`),
because assigning an arbitrary int-typed value into a real `bool` forces MWCC to
canonicalise it to 0/1 — and the flag-setting shift is then reused as the branch
condition, with no separate compare. Confirmed by A/B compile; it closed four
functions.

**`cntlzw`+`srwi` is the same family** — that is MWCC materialising `(x == 0)`
as a *value* rather than as a branch. Two batches independently failed on it and
neither connected it to the lever above.

**And the connection alone is not enough — this is the refined rule.**
`bool ok = (mError == 0); if (ok)` does **not** produce `cntlzw`+`srwi.`, which
is exactly why both batches tried it and got a plain `cmpwi`. Proven by A/B
compile: the tested value must ALSO be **opaque to the optimiser**. Bool-storage
alone does nothing; opacity alone does nothing; together they reproduce the
idiom byte-for-byte.

`OSTryLockMutex` supplied its own opacity — it is an external call returning a
non-`bool`. A plain member read does not, and marking it `volatile` supplies it
artificially. **So state the rule as: MWCC only pays for the canonicalisation
when an opaque non-`bool` value is stored into a real `bool`.**

`volatile` is NOT applied in the header and **is now refuted outright**, not
merely declined. It buys the materialisation but forces a fresh load on every
textual read, so the chained `mError == 0` then `mError == 6` tests lose the
target's shared register load — `save` and `load` go from 9 and 12 instructions
SHORT to 2 and 1 instructions LONG. And `mError` is read by the banked,
byte-exact `d_s_boot.cpp` in four places, so it is a shared-header change that
was never tested outside this TU.

**The decisive argument, though, is inside this TU and settles it for good.**
`existCheck` is byte-exact with plain `int mError`, and it performs the
identical test after the identical call:

```
bl setNandError__13dNandThread_cFl      bl setNandError__13dNandThread_cFl
lwz    r0, 0x78(r29)                    lwz    r0, 0x78(r31)
cmpwi  r0, 0x0                          cntlzw r0, r0
bne    .L_800CF03C                      srwi.  r0, r0, 5
                                        bne    .L_800CF250
  existCheck (0x800CEFC0) — MATCHES       save (0x800CF238) — near-miss
```

Same member, same type, same preceding call, same TU, same compile — one gets
`cmpwi`, the other gets `cntlzw`. **No property of the member declaration can
therefore be the cause**, because any qualifier or width change would also move
`existCheck` and break a function that already matches. Whatever produces the
idiom is *local source structure inside `save`/`load`*.

##### Deferred use materialises; immediate use folds — and it still is not enough

Three agents have now worked this. The rule they extracted is new, general, and
worth carrying to every unit:

**MWCC materialises a `bool` into a register (`cntlzw`/`srwi.`) when its use is
deferred across a block boundary from its definition. A `bool` that is branched
on immediately folds to a plain `cmpwi`.** Proven on a plain, non-`volatile`,
CSE-able field read, at full function scale as well as in isolated probes: in
`bool ok = (e==0); bool busy = (e==6); if (!ok) { if (busy) ... }`, `busy`
materialises and `ok` does not.

**And the `volatile` family is closed, for a structural reason worth
remembering.** The target shares ONE load between two materialisations. A
`volatile` read is not CSE-able by definition, so it can buy the `cntlzw` or the
shared load, never both — whether the qualifier sits on the declaration
(CLOSE_A) or on the read as `*(volatile int *)&mError` (CLOSE_D). Two agents
reached that wall from opposite directions. Do not send a third.

##### `createBanner` retires the opacity theory outright

There is a byte-exact worked example of materialisation-from-a-plain-read
sitting in this TU, and everyone — three agents and me — walked past it. At
`0x800CF4D0` in `createBanner`, which **matches**:

```
bl     setNandError__13dNandThread_cFl
lwz    r3, 0x78(r30)
neg    r0, r3
or     r0, r0, r3
srwi   r3, r0, 31        <- no recording dot; this is the RETURN VALUE
<epilogue>
```

Plain `int mError`, no cast, no qualifier, no external call feeding it — and it
materialises. The source is `return (mError != 0);`.

**So opacity was never the mechanism.** The whole `volatile`-declaration and
`volatile`-cast programme, two agents and roughly a session each, was explaining
a property the target does not rely on. **The trigger is the boolean being
produced as a value.** `createBanner` pays for the 0/1 because it returns it;
`existCheck` never needs one, only a branch, and gets `cmpwi`.

**Confirmed by direct A/B, and this is the rule to carry forward:** an explicit
second consumer forces the recording form from a plain, non-`volatile` field —
`bool ok = (mError == 0); if (!ok) return 1; ...; return ok;` emits exactly the
target's `cntlzw`/`srwi.`, as do variants storing `ok` to a member or passing it
to a call. No cast anywhere. Three further results bound it:

- **Contagion is refuted.** A materialising bool later in a function does not
  retroactively change an earlier plain guard in the same function. Nor does a
  tail call returning a bool.
- **Giving the guard itself a derived second use works but overpays.**
  `return !ok1;` in place of `return 1;` does flip the branch to the recording
  form, but MWCC recomputes the materialisation for the return value instead of
  reusing the register — 2 instructions the target does not spend.
- **`checkCRC` is not a boundary case.** Its guards are direct
  `if (a != b) return false;` comparisons with no named bool at all, which makes
  it the clean opposite extreme from `createBanner` on the same axis rather than
  an anomaly.

So `save`/`load` are parked with the mechanism understood and no lever applied:
their guards have no natural second consumer in any shape tried, and every way
of manufacturing one costs bytes the target does not spend. **Getting the right
idiom at the wrong price is not progress**, and an agent declining to ship one
was the correct call.

That reframes the residual precisely. `save` and `load` use the **recording**
form (`srwi. r0, r0, 5` feeding a `bne`) — MWCC produced the value *and*
branched on it — so something makes the 0/1 a live value there even though a
branch is its only visible consumer. `checkCRC` returns a constant and does not
materialise, which makes it the boundary case worth studying.

**The evidence that defeats every hypothesis so far**, and the right place for
the next person to start: `save`'s *first* check at `0x800CF238` materialises
via `cntlzw` and **has no sibling at all** — nothing else is derived from that
load, there is no second test, no deferred use, nothing for the deferred-use
mechanism to apply to. Yet the target materialises. So the trigger is not
opacity, not register pressure, not use-count, and not the chained pair. The
open question is no longer "how is the load shared" but **"why does an isolated
`mError == 0` guard materialise at all, from a plain field, when the identical
guard in `existCheck` twenty lines away compiles to `cmpwi`?"** Nobody has a
lead on that, and saying so is more useful than another variant sweep.

**General rule, and it is worth applying beyond this unit: when two functions in
one TU compile the same expression differently, every explanation that lives in
the shared header is already dead.** Look for the difference locally. The
sharpest untested candidate here is the shape of the arm — `existCheck` guards a
block and falls through, while `save`/`load` take an early `return <constant>`.
Note also that the occurrence at `0x800CF238` materialises into `r0`, is used
exactly once, and still uses `cntlzw`, which rules out register pressure and
"the value is needed twice" as triggers.

#### Two process findings from running it

- **A wrong data bound was caught before authoring, not after.** The pre-flight
  put `.data` at `0x80317D48`; the true low bound is `0x80317CD8`, `0x70` lower,
  and the missing objects were three banner strings and a 16-entry jump table
  belonging to `setNandError`. Both the terminal-vtable rule and the
  consecutive-pool-ID rule catch it. **Walk backwards from every claimed low
  bound and ask what the object below it belongs to** — subtracting correctly
  from a wrong starting point still gives a wrong answer.
- **The shared brief's own hypothesis was wrong and an agent said so.** It
  predicted the five `cmd*` functions were one body with one constant changed.
  They are not: one writes three fields, three write two, one adds a `memcpy`.
  The agent wrote the correction into the relay file the other batches read,
  which is what stopped it propagating into three more functions.

#### The landing kit, pre-computed — apply it the moment the five close

All of this is derived mechanically from the target's own relocations and from
`slices/wiimj2d.json`, not estimated. It is the integrator's half of the job and
it is done, so closing the last five functions is the only remaining work.

**Slice block.** Insert between `dol/bases/d_multi_manager.cpp` and
`dol/bases/d_next.cpp` (currently array indices 52 and 53):

```json
{
    "source": "dol/bases/d_nand_thread.cpp",
    "memoryRanges": {
        ".text": "0xc8580-0xc9560",
        ".rodata": "0x3490-0x34b8",
        ".data": "0x19638-0x196d8",
        ".bss": "0x8640-0x1f680",
        ".sdata": "0x5f8-0x608",
        ".sbss": "0x3f8-0x400"
    }
}
```

**These bounds are confirmed five independent ways, and the confirmation method
generalises — use it on every future unit.** Checked against every other slice
for overlap (zero) and for adjacency: `.text` starts exactly where
`d_multi_manager` ends and ends exactly where `d_next` begins; `.rodata` and
`.data` both begin exactly where `d_multi_manager`'s end; `.sbss` sits exactly
in the one-word hole between `d_multi_manager`'s `0x3f0-0x3f8` and `d_next`'s
`0x400-0x408`. And `d_multi_manager`'s `.sdata2` (`0x1930-0x1938`) is already
adjacent to `d_next`'s (`0x1938-0x1950`), which independently confirms this TU
has **no** `.sdata2` — a negative that is otherwise easy to get wrong.

**`syms.txt`: remove 4, add 22.** Remove these — once our object defines them,
pinning them to an address is a contradiction:

```
cmdExistCheck__13dNandThread_cFv=0x800CEF10      (line 392)
cmdSpaceCheck__13dNandThread_cFv=0x800CF060      (line 393)
create__13dNandThread_cFPQ23EGG4Heap=0x800CFBA0  (line 394)
m_instance__13dNandThread_c=0x8042A298           (line 1092)
```

Add these — every external the TU references that no landed slice defines:

```
NANDCheck=0x801db280            NANDClose=0x801d9990
NANDCreate=0x801d8620           NANDDelete=0x801d8920
NANDGetHomeDir=0x801dac30       NANDGetLength=0x801d9180
NANDGetType=0x801dafb0          NANDInitBanner=0x801db0e0
NANDMove=0x801d9110             NANDOpen=0x801d96f0
NANDRead=0x801d8b30             NANDSimpleSafeCancel=0x801da0a0
NANDSimpleSafeClose=0x801d9e50  NANDSimpleSafeOpen=0x801d9a90
NANDWrite=0x801d8c20            OSInitCond=0x801b3280
OSSignalCond=0x801b3370         OSWaitCond=0x801b3290
__ct__Q23EGG6ThreadFUliiPQ23EGG4Heap=0x802ba4f0
__dt__Q23EGG6ThreadFv=0x802ba640
calcCRC32__4sCrcFPCvUl=0x8015f270
m_instance__9dResMng_c=0x8042a318
```

**Do not pin these four** even though the TU calls them —
`OSInitMutex`, `OSLockMutex`, `OSUnlockMutex` and `OSTryLockMutex` are already
defined by the landed `lib/revolution/os/OSMutex.c`, as are
`getRes__6dRes_cCFPCcPCc` (`dol/bases/d_res.cpp`) and
`setCurrentHeap__5mHeapFPQ23EGG4Heap` (`dol/mLib/m_heap.cpp`). Pinning a symbol
a landed slice already defines is the error this check exists to catch.

Note that `OSMutex.c`'s slice covers only the four mutex functions; the three
condition-variable functions live at `0x801b3280`–`0x801b3370`, outside it, and
so still need pins. Same source file in the SDK, different slice — worth
remembering, because "the file is landed" is not the same as "the symbol is
defined".

**Method, for reuse:** parse the relocations out of the target disassembly,
subtract the symbols the TU defines itself (including `@NNNNN` pool objects,
`@LOCAL@` statics, and the `__vt__` tables, all of which are ours), then test
each survivor's symbol-map address against every landed slice's `.text` range.
What is left needs a pin. Guessing this list by reading the source is how
symbols get pinned twice or missed.

### Recommended: `d_a_player_manager.cpp` — `daPyMng_c`

`0x8005E9A0`–`0x800613B0`, 10,768 B span / 10,300 B code / 68 fns.
**It is now unblocked**: it embeds `daPyDemoMng_c` by value in its `.bss` and
needed that class's exact `sizeof`, which is 0x98, proven three independent ways
and landed. `include/game/bases/d_a_player_manager.hpp` already exists with real
signatures because the banked `d_a_player.cpp` and `d_a_player_base.cpp` call
into it constantly, and this session added `mCourseInList` to it.

The class is **all-static with no vtable**, so there is no layout to reconstruct
— every static member is a named symbol with a size in the map. `.text`,
`.ctors` (`0x88-0x8c`, one free slot), `.bss` (`0x3790-0x4640`), `.sbss`
(**`0xe0-0x138`**, corrected — the `0xe0-0x110` recorded here earlier was
`0x28` bytes short) and `.sdata` (`0x280-0x290`) are exact by subtraction.

**The `.sbss` error is worth reading even if you never touch this unit.** It was
derived "by subtraction" and stated as exact, and it omitted nine members —
`mPauseEnableInfo`, `mPauseDisable`, `mStopTimerInfo`, `mStopTimerInfoOld`,
`mQuakeTrigger`, `mBgmState`, `mBonusNoCap`, `mKinopioCarryCount` and one unnamed
byte — **every one of which our own `.text` references by name.** Two agents
caught it independently. Had it survived to the link it would have shifted every
following small-data object and failed four of five binaries with thousands of
scattered single-byte diffs, with nothing wrong in any function.

What actually settled it is a file this handoff never mentioned:
**`bin/dtk/dtk_splits_wiimj2d.txt`**, an official per-source-file section range
list for already-split TUs. `d_actor.cpp`'s `.sbss` starts at exactly
`0x80429FD8`, which brackets our end hard. **Check that file before deriving any
bound by subtraction or elimination** — it converts the weakest kind of claim
into the strongest one, and it has been sitting in `bin/dtk/` the whole time.

Three hazards, all pre-characterised:

1. Its `.bss` embeds **four static class instances by value**, each with a 0xC
   dtor record: `mDemoManager` (0x98, `daPyDemoMng_c` — **now done**),
   `mMultiManager` (0x5C), `mAttention` (0x58), `mEffectMng` (0xC5C). The other
   three `sizeof`s must be exact or the whole `.bss` shifts.
2. Two **foreign weak inline copies sit mid-range** —
   `getCourseIn__10dScStage_cFv` (8 B, `0x8005EC90`) and `getFileP__5dCd_cFi`
   (32 B, `0x8005EE70`) — from classes whose TUs are already banked. That is the
   weak-copy / `keepWeak` / `syms.txt` collision, and it surfaces only at the
   full link. **Trial-link early** (see that section) rather than discovering it
   at the end.
3. `.data` (~`0xb388-0xb3b8`) and `.sdata2` (~`0xa18-0xa20`) need pool
   attribution. **Note `.data 0xb388-0xb3b8` is exactly the 0x30 that
   `d_a_player_demo_manager` was wrongly assumed to own** — the two strings
   `"Wm_mr_vshipattack"` / `"Wm_mr_vshipattack_ind"` at `0x80309A28`. They
   belong to whichever TU follows demo_manager in link order; check
   `d_a_player_hio_ADJ` first, since its slice claims no `.data` only because it
   is `nonMatching` and nobody ever derived one.

#### What `d_a_player_demo_manager` established, and what it cost

Landed 51/51. The functions were the easy part — **every one of the three
defects that actually blocked the link was in DATA placement**, and none was
visible to a per-function diff:

- **`T *const arr[]` is a const-qualified type and lands in `.rodata`.** The
  original put the value tables in `.rodata` and the POINTER tables in `.data`.
  Dropping the outer `const` moved them. Check the symbol map's section per
  object rather than assuming a table is a table.
- **MWCC emits a class's vtable as the TERMINAL `.data` object, unconditionally.**
  Verified twice. So anything after the vtable in the target belongs to the NEXT
  TU — which is a free upper bound on any `.data` claim, and it disproved a
  bounds derivation that had been made by elimination.
- **A header static with a non-trivial constructor is emitted into EVERY TU that
  odr-uses it**, pooled strings and all. `dWmLib::sc_ForceList` has 30 such
  copies in the original. That is why this TU had a `.sdata` claim its bounds
  derivation had confidently called empty.
- **Consecutive `@NNNNN` pool IDs identify a TU.** The two `.sdata` strings are
  `@72502`/`@72503` and this TU's array destructor is `__arraydtor$72504`. When
  ownership of an anonymous object is unclear, look at the neighbouring pool
  numbers — it is the cheapest attribution evidence available.
- **A "bounds by elimination" claim is the weakest kind**, and the bounds agent
  said so at the time. Both of its two flagged weak spots turned out wrong, and
  both were caught only by linking.

### Runners-up

**`d_a_player_manager.cpp` (`daPyMng_c`)** — `0x8005E9A0`–`0x800613B0`,
10,768 B span / 10,300 B code / 68 fns.
`include/game/bases/d_a_player_manager.hpp` **already exists with 79 lines of
real signatures** (not a `u8 mPad[]` stub) because the banked `d_a_player.cpp`
and `d_a_player_base.cpp` call into it constantly. The class is **all-static
with no vtable**, so there is no layout to reconstruct: every static member is a
named symbol with a size in the map. `.text`, `.ctors` (`0x88-0x8c`, one free
slot), `.bss` (`0x3790-0x4640`), `.sbss` (`0xe0-0x138`, corrected) and `.sdata`
(`0x280-0x290`) are exact by subtraction. Three hazards keep it second:
1. Its `.bss` embeds **four static class instances by value**, each with a 0xC
   dtor record: `mDemoManager` (0x98, type `daPyDemoMng_c` — **undone**),
   `mMultiManager` (0x5C), `mAttention` (0x58), `mEffectMng` (0xC5C). Their
   `sizeof`s must be exact or the whole `.bss` shifts.
2. Two **foreign weak inline copies sit mid-range** —
   `getCourseIn__10dScStage_cFv` (8 B, `0x8005EC90`) and `getFileP__5dCd_cFi`
   (32 B, `0x8005EE70`) — from classes whose TUs are *already banked*. That is
   the pakkun weak-copy / `keepWeak` / `syms.txt` collision with live
   neighbours, and it will only surface at the full link.
3. `.data` (~`0xb388-0xb3b8`) and `.sdata2` (~`0xa18-0xa20`) need pool
   attribution, because the un-banked `d_a_player_demo_manager.cpp` sits between
   it and the previous banked claim in those two sections.

**`d_a_player_demo_manager.cpp`** has been **promoted to the recommendation
above** — see that section. Note its `.ctors` slot is `0x80-0x84`, one free slot.

### Attractive-looking candidates with traps

- **`0x80041C00`–`0x80044940` (11,584 B, 86 fns)** — the `d_a_lift_down_on_base`
  / `d_a_move_pipe` gap. Perfect size, both neighbours banked, looks like one
  free-bounds haul. It is **at least six TUs**: `daLiftDownOnBase_c` (20 fns),
  `daIceAshibaBase_c` (15), `daLiftRemoconMain_c` (11), `daFlyDokan_c` (10),
  `daMovePipe_c` (6), `daKawanagareObj_c` (5), `daLiftMain_c` (2),
  `dRideRoll_c` (1). Only two `__sinit`s exist, so four internal boundaries are
  invisible and every one must be derived. Outer bounds free, inner bounds not.
- **`d_a_en_obj_coinblock.cpp` (`0x80036930`–`0x80037EA0`, 5,488 B, 39 fns)** —
  fully bracketed, all bounds free, seven clean states, looks cheap. **The trap:
  `__vt__18daEnObjCoinBlock_c` does not exist anywhere in the symbol map**, and
  the range contains **no constructor, no destructor, no `create`, no
  `execute`** — its lifecycle lives in a `.rel`. Playbook step 2 is simply
  unavailable; the layout must come from `lwz`/`stw` displacements. Cheap bytes,
  expensive class.
- **`d_a_en_coin_main.cpp` (`0x800272F0`–`0x800281C0`, 3,792 B, 23 fns)** — all
  bounds free, and it **is** a base class (`__vt__14daEnCoinMain_c` is 0x2EC,
  same size as `daEnBlockMain_c`'s), so it gates the coin family in
  `d_enemiesNP.rel`. But it has T-2's shape problem in milder form, and despite
  the matching vtable size **its function names barely overlap blockmain's** —
  siblings, not twins, so the "blockmain just landed" intuition does not pay.
  Good filler, not a headline target.
- **`d_a_farBG.cpp` (`0x80115BD0`–`0x8011A5B0`, 18,912 B, 55 fns)** — fully
  bracketed, single `__sinit`, `__vt__9daFarBG_c` 0xD4 (no new virtuals over
  `dActor_c`). But 55 functions across 18.6 KB is **339 B per function**, the
  largest average of any candidate, with only 12% name precedent, and
  background-scroll code is exactly the float/matrix-heavy shape to avoid. It
  becomes the natural pick *after* hatena_balloon, which is its lower neighbour.
- **`0x800451F0`–`0x800460D0` (3,808 B, 33 fns)** — highest precedent rate
  measured (**68% of bytes share a name with banked code**) and it contains
  **three base classes** (`daObjMoveOnBase_c`, `daObjPipeBase_c`,
  `daObjSpinChildBase_c`), so it unblocks three families for 3.8 KB. The trap:
  **no `__sinit` anywhere in the range**, so all three TU boundaries are
  invisible — the `d_a_sink_dokan.cpp` condition below. Worth doing, but budget
  the boundary derivation as its own stage.
- **`d_a_boss_demo.cpp`** — still blocked on `d_en_boss.cpp`; nothing has
  changed. Its pre-derived ranges below remain valid.

### The pakkun pair — DONE, and the two rules it cost

Both TUs are landed and linked: `d_a_en_dpakkun_base.cpp` 64/64 and
`d_a_en_dfpakkun.cpp` 33/33, 19,808 bytes, the jump from 9.826% to **10.131%**.
Two lessons are worth more than the files.

#### A `nonMatching` slice is not linked at all

`gen_lcf.py`, `slice_dol.py` and `configure.py` all skip it and splice the
original bytes in. So the flag is not a progress annotation — it decides whether
your object participates in the link, and **clearing it is the first real test
your object has ever had**. Per-function diffs cannot see any of what follows.

Three things surfaced only at that moment, and all three will recur:

- **Weak-copy deadlock.** Inline members are emitted by every TU that includes
  the header and the linker keeps one copy. The original kept
  `daEnDpakkunBase_c`'s in `d_a_en_dfpakkun.cpp`. Flip the base on alone and its
  object becomes the sole provider of ~35 weak copies, `.text` grows 0xE0, all
  five binaries fail. **The pair had to be flipped together.** Expect this
  whenever a base and its derived TU are both in flight.
- **`.text` too long, from a symbol you are the only one defining.** Your object
  emits a weak inline member the original resolved from a **still undecompiled**
  TU; spliced bytes are not a definition the linker can see, so your copy wins.
  Fix with a `syms.txt` entry at the original's address — that is what removed
  `__dt__Q33m3d5mdl_c10callback_cFv` and the `timingB`/`timingC` stubs here.
- **`keepWeak` and `syms.txt` are global, not per-slice.** Adding the entries a
  `nonMatching` TU will need forces those symbols in whichever sibling *is*
  linked, and breaks the build. They must land in the **same commit** as the
  flag clears.

#### Overrides are a free lever — the flush-order rule

**Within a class, the end-of-TU inline flush block is emitted in strict reverse
declaration order.** Declaration order is pinned by the vtable only for virtuals
the class *introduces*; an **override** inherits its slot from the base and can
be declared anywhere without moving the vtable. So for overrides, declaration
order is free — and it is the *only* lever that moves the flush block.

**Between classes, groups come out in reverse parse order**, base before derived,
so `#include` order is the lever: parsing a class earlier pushes its flush group
later. This mirrors exactly where the weak vtables land in `.data`, which gives
you a second, independent way to check it.

Eliminated, so nobody repeats them: out-of-class `inline` definitions, access
specifiers, const-qualification, signature changes, interleaved members, taking a
member's address — none of them move anything. Only *being called* moves a
function, and that removes it from the block entirely.

A ~120-TU sweep found **zero real inversions across every banked TU**, so the
rule holds project-wide and was simply mis-declared here. `tools/` has no
committed version of that sweep; it is worth rebuilding if this recurs.

The concrete fix was two declaration moves and one `#include`, with no function
body touched — see commit `5dc4095`.

### The remaining actor TUs, with what is known about each

Cross-check with `tools/tu_split.py` and `tools/tu_extent.py` (both tracked
under `tools/`), or with the `__sinit_<file>_cpp` symbols in
`bin/dtk/wiimj2d_symbols.txt`. Annotations are hard-won — read them before
assigning.

**These sizes are a hypothesis, and the errors run in both directions.** The
size and count columns come from `tu_extent.py`'s heuristic ranges; on the last
row that closed they were wrong by **7 functions and 840 bytes** — and that
under-count survived into the plan. Re-derive from the symbol map before
assigning. Where both figures are known, `Span` is what progress counts (it
includes 16-byte inter-function alignment) and `Code` is the sum of function
sizes; they are not the same number and neither is a typo for the other.

| TU | Span B | Code B | Fns | Notes |
|---|---|---|---|---|
| `d_a_en_dfpakkun` | 10,624 | — | 72 | **DONE 33/33** authored (rest are weak base copies), landed and linked |
| `d_a_en_jimen_pakkun_base` | 8,848 | — | 67 | **DONE 67/67**, landed and linked. Derives from `dEn_c`, NOT the pakkun base |
| `d_a_en_bros_base` | 12,112 | 12,112 | 99 | **DONE 99/99**, landed and linked. Derives from `dEn_c` |
| `d_a_en_blockmain` | 13,232 | 12,604 | 97 | **DONE 97/97**, landed and linked. Ten file-static functions (2,800 B, 22%) have no symbol-map name; the names in our source are invented |
| `d_a_en_hatena_balloon` | 18,768 | 18,216 | 81 | **DONE 81/81**, landed and linked. Derives from `dEn_c` |
| `d_a_player_manager` | 10,768 | 10,300 | 68 | Runner-up; all-static class, no vtable |
| `d_a_player_demo_manager` | 9,280 | 8,976 | 51 | **DONE 51/51**, landed and linked |
| `d_a_bullet` | 7,316 | — | 73 | |
| `d_a_lift_down_on_base` | 6,280 | — | 58 | **At least six TUs** across the wider gap — see `tu_split.py` and the traps list above |
| `d_a_move_pipe` | 5,380 | — | 29 | Part of the same multi-TU gap |
| `d_a_en_obj_coinblock` | 5,488 | 5,204 | 39 | No `__vt__` in the symbol map — see the traps list |
| `d_a_en_coin_main` | 3,792 | 3,652 | 23 | Base class, gates the coin family |
| `d_a_wm_player_static` | 3,268 | — | 24 | |
| `d_a_boss_demo` | 2,772 | — | 49 | **BLOCKED** — see below |
| `d_a_wm_Map_static` | 2,308 | — | 17 | **17/18 done**, blocked on a 0x9C8 table — see below |
| `d_a_player_hio_ADJ` | 2,032 | — | 15 | **15/16**, banked `nonMatching`. One function left, well-characterised — see below |
| `d_a_farBG` | 18,912 | 18,636 | 55 | 339 B per function, the worst average measured |
| `d_a_ice` | 32,176 | — | 151 | |
| `d_a_yoshi` | 64,592 | — | 347 | The gap to `d_pausewindow.cpp` holds **three `__sinit`s** — `d_a_yoshi`, `d_fukidashiManager`, `d_gamedisplay` — so at least four TUs, not two. Also contains `daPlyIce_c` |

### `d_a_en_lkuribo_base.cpp` — DONE, 58/58, banked whole

Landed byte-exact: 9.681% -> **9.826%** (+9,456 bytes), `wiimj2d.dol` 18.891% ->
**19.200%**. Six parallel authoring agents, every function matching on its first
compile. The method that produced that is documented at the top of this file; the
three findings it cost are the lazy-flush rule, the shadowed-state-ID trap and
the comparator bug, all recorded below.

Its `.rodata` owns the two 0x20 death-info templates that sit past a 4-byte gap
and look like they belong to the next TU. They do not — `hitCallback_Fire` and
`setDeathInfo_Hasami` reference them, and net_nokonoko's banked range starts
exactly after them. The gap is alignment padding.

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
will mislead later work. The pakkun family was the clearest case —
`d_a_en_dpakkun_base.cpp` gated `d_a_en_dfpakkun.cpp`, and until the base
landed, the derived TU's placeholder header carried an unidentified 68-byte
member region. Both are now done; the live instances are `d_a_en_coin_main.cpp`
(gates the coin family in `d_enemiesNP.rel`) and the three base classes in the
`0x800451F0` run.

Second preference is a TU whose *shape* is already solved elsewhere — e.g.
`d_a_rot_objs_base.cpp`, whose `searchParent_*` functions were reported as
sharing an existing implementation verbatim.

Note the trade-off is real and does not always favour the base: the recommended
next target is not a base class at all, because all seven of its bounds are free
and 43% of it has banked precedent. Weigh unblocking against cost, do not apply
the rule mechanically.

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

### A constant shift in SDA-relative operands means an `.sbss` size error

Third distinct whole-binary signature, and the cheapest to misdiagnose because
**every section size matches and the `.ctors` table is byte-identical**. Symptom:
the DOL fails, section sizes all agree, and a byte diff yields many single-byte
differences scattered across `.text`, each one the low byte of a `d13`/`r13`
operand, all off by the **same constant**:

```
ours 3bedb740   orig 3bedb738     addi r31, r13, ...   <-- +8
ours 386db728   orig 386db720     addi r3,  r13, ...   <-- +8
```

Every affected instruction is SDA-relative (`ra == 13`). That is not a content
error — it is your `.sbss` claim being the wrong *size*, which shifts every
downstream `.sbss`-relative reference project-wide.

**The rule: every `.sbss` claim in this project is a multiple of 8.** A 4-byte
claim links cleanly, passes `--verify-obj`, and produces exactly the failure
above. Check with:

```python
for sl in slices:
    r = sl['memoryRanges'].get('.sbss')
    if r:
        lo, hi = (int(x, 16) for x in r.split('-'))
        assert (hi - lo) % 8 == 0, sl['source']
```

Bisect it by removing the `.sbss` claim entirely and rebuilding: if the shift
moves or changes character, `.sbss` is where to look. Note that a *symbol* in
`.sbss` may genuinely be 4 bytes (`ms_num_of_instance` is `size:0x4`) — it is
the **slice claim**, not the symbol, that must round up to 8.

### The target list is incomplete: TUs with no `__sinit` are invisible

`tu_extent.py` delimits TUs by `__sinit` symbols. **A TU with no file-scope
static objects emits no `__sinit`**, so it does not appear in the list at all —
it is silently absorbed into a neighbouring TU's reported range. This is not
hypothetical: `d_a_sink_dokan.cpp` (`daSinkDokan_c`, ~0x920 bytes) was found
sitting undetected between `d_a_rot_objs_base` and `d_a_spin_child_base`.

`tools/tu_split.py` detects the condition. It demangles the class name out
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

## MWCC aligns a `.bss` object to 8 when its SIZE is a multiple of 8

Regardless of the type's own alignment. This is a placement rule, not a fact
about the class, and it is worth knowing before you spend a round on one.

The situation that produced it: `daPyMng_c` embeds four managers by value, each
preceded by a `0xC` `__register_global_object` destructor-chain node. Two of the
four had a **4-byte hole** between the node and the object, and two did not. The
obvious reading is that those two classes carry 8-byte alignment, which under
MWCC means a `double` or a type containing one. Two workers reasoned about it
that way, one concluded the alignment hypothesis was right, and the other
correctly said it could not tell — because a survey of the class's code found no
`lfd`/`stfd` anywhere, so there was no double to find.

Both were wrong, and a four-line probe settled it. Compile some structs holding
nothing but `int`s, interleaved with `0xC` nodes:

| Object size | `size % 8` | Placed at | Gap after the 0xC node? |
|---|---|---|---|
| `0x98` | 0 | `0x10` | **yes, 4 bytes** |
| `0x5C` | 4 | `0xB4` | no |
| `0x58` | 0 | `0x120` | **yes, 4 bytes** |
| `0xC5C` | 4 | `0x184` | no |

That reproduces the original's pattern exactly, from types whose alignment is 4.
The clincher is a `char[0x18]` — **alignment 1** — which still gets placed
8-aligned, because `0x18 % 8 == 0`. The size, not the alignment, is doing it.

**Three consequences:**

1. **A 4-byte hole in `.bss` is not evidence about a class's members.** Do not
   go looking for a `double` to explain one, and do not accept a reconstruction
   that invents a member to produce alignment. `dAttention_c` is `0x58` with
   nothing wider than an `int` in it.
2. `sizeof % 8 == 0` is enough to get the placement right on its own, so a class
   whose size is already correct needs no alignment work.
3. **`__alignof__` is not the check.** An earlier instruction told a worker its
   header was wrong if `__alignof__` did not come out to 8. That was wrong and
   would have sent it hunting for a member that does not exist. It refused to
   invent one and reported "cannot distinguish" instead, which was the right
   call — the honest non-answer is what made the probe worth running.

Probe it directly the moment a hole appears; it costs four lines and one compile.

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
   house style, and `include/game/bases/d_a_en_blockmain.hpp` for a large
   `dEn_c` actor's header — it is the best model in the repo. (New this session,
   with `d_a_en_bros_base.hpp` and `d_block_mng.hpp`.)

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
2. **An empty virtual that is NEVER CALLED in the TU must be defined out of
   line**, or it is implicitly inline, vanishes, and the vtable slot breaks.
   **But one that IS called must be inline in the class body** — see the flush
   rule below. This trap previously read "every empty virtual", which is too
   strong and cost a full round trip on `d_a_en_lkuribo_base.cpp`.
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
will chase phantom differences. `tools/auto_decomp/harness.py`'s `extract` does
this correctly; the older `diffall.py` it referred to is **lost with the
scratchpad**.

## Briefing authoring agents

This is the standing list. "Running the parallel pipeline" above adds five more
briefing rules that each cost something on bros/blockmain — shared-helper batch
first, assign from the map's table, assign data ownership, say who authors
nothing, freeze the shared header. Read both.

**Write the shared material to files and point every agent at them, rather than
pasting it into each brief.** On `d_a_en_lkuribo_base.cpp` that was two files —
a `prelude.cpp` holding the verified class declaration and file-scope data, which
each agent pasted at the top of its draft verbatim, and a `SHARED-BRIEF.md` with
the compile loop, the levers, the data inventory and the rules. The per-agent
brief then shrank to its own function list plus what was specific to it.

Two reasons this beats duplicating the text. Six copies of a fact drift, and when
one turns out to be wrong you must correct it six times — this session shipped a
vtable displacement table that was one slot off, and fixing the shared file plus
one relay each was the whole remedy. And an agent that pastes a *shared* class
declaration cannot quietly fork it, which is the failure that would poison an
entire TU.

State plainly: **do not modify the class declaration; if you believe it is
wrong, stop and report that.** Six agents did, none had to.

Each per-agent brief still needs:

1. The exact functions it owns, with addresses and sizes, in target order.
2. Where the target disassembly already is — never make them re-derive it.
3. Its entry from the sibling correspondence map, which is usually most of the
   answer. See "The method that works" at the top of this file.
4. Anything already read out of the target for those functions: statement
   orders, member offsets, sound ids, resource names.
5. Which of its callees belong to *other* agents, and that it must call but not
   author them.
6. **Deliverable is source code in the reply**, not edits to the shared tree, and
   it must **not** run `ninja` or edit `slices/wiimj2d.json`.
7. **Report every data object with its section** — string literals, floats,
   statics, vtables. The lead needs these for the slice bounds, and that is where
   integration time actually goes.
8. **Do not claim MATCHING unless the diff printed nothing**, said explicitly,
   per function. A well-characterised near-miss is far more useful than a false
   pass, which has cost this project a full day.
9. Environment gotchas: dtk relative paths with forward slashes fail on Windows;
   PowerShell 5.1 parses 8-hex-digit literals as negative Int32, so do address
   maths in Python; splat native-exe arguments.
10. **No background processes.** Everything foreground, confirm exits, check for
    strays before finishing. (One agent leaked a script that span at 100% CPU for
    21 minutes.)

Tell them where the shared comparator is (`harness.py`'s `extract` / `diff_fn`)
so they do not each write one — and point them at the checklist in "Verify your
verification tool". A negative control is not enough on its own: three of the
six confirmed defects were invisible to one. The two checks to insist on are
**extract by address** and **instruction count × 4 == the symbol-map size**.

### Splitting a TU between agents

Group by *cohesion*, not by size: lifecycle, collision, callbacks, one group per
family of states. Related functions share idioms, data and often whole bodies, so
a group that owns all three of a near-identical trio solves it once. On lkuribo
one group's twelve functions collapsed into effectively three distinct bodies.

Watch for groups that are the dependency of others — the one owning the shared
helpers should be told so, launched first, and told to report a signature change
immediately rather than at the end.

Assembly is by target address order, not by group; see "Assemble by ADDRESS, not
by batch" above, which is where the emission-order rules now live.

## Monitoring agents — what actually works

Most obvious signals are useless. Verified across two sessions:

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
minutes and 60–115 tool calls, and all of them succeeded. The lkuribo batch ran
7–24 minutes per authoring agent, and the one that solved the flush rule ran 24
minutes across 77 tool calls.

**Use the wait productively.** While agents author, the lead can verify the
baseline build, derive the slice entry from the neighbouring banked slices, and
write out the canonical source order. All of that is independent of the code and
it is most of the integration work.

## Verify your verification tool — six defects so far, and the checklist that catches them

**Every tool in this project that reports a match has, at some point, reported a
match that was false.** Six confirmed defects now, across three tools. Not one of
them was a *miss* — a miss is loud and cheap. Each produced a **confident wrong
answer**: a clean `MATCHING`, a plausible address list, a family view that simply
did not mention the 199 functions it had lost. Wrong answers of that shape do not
cost you an hour, they cost you the decisions you make afterwards.

Read the pattern before the particulars:

| # | tool | what it reported | what was true |
|---|---|---|---|
| 1 | `fndiff.py` (since deleted) | `IDENTICAL` | it never found the function |
| 2 | `harness.canonicalise` | spurious diff on every `.sdata2` ref, and `0.0f` == `8.0f` | pool naming leaked in; literal values erased |
| 3 | `harness.extract` | equal | branch destinations erased — control flow invisible |
| 4 | `harness.norm_name` | a clean diff of function A | it was diffing function B |
| 5 | `sibmap.FAMILY` | a family view with N hits | 199 functions silently excluded |
| 6 | `datarefs.py` | seven cross-unit data references | phantom addresses ~0x1900 too high |

Three of those six are **invisible to a negative control**, which is why "I ran a
negative control" is no longer a sufficient answer. A negative control proves the
comparator can say *no*. It cannot detect a comparison that is internally
consistent but aimed at the wrong object: defect 4 diffed the wrong function
against the wrong function and every sanity check it had passed. Defect 5's dead
FAMILY entry contributed no hits and raised nothing, because contributing nothing
is indistinguishable from being unlucky. Defect 6's phantom addresses were
contiguous, plausible, and in a real section.

What actually caught defect 4 was an agent asserting that **the extracted body's
instruction count × 4 equals the size in the symbol map**. That is a *positive*
check — it ties the comparison to an independent fact about the target rather
than asking the comparator to grade itself. That is the model to copy.

So this section is a checklist, not a warning list. **Each item below is a
different, independent view of the same function; each has a way to make it
non-vacuous; and only the last one is authoritative.**

### How to verify a function is really matching

Run these in order. Steps 1–3 are cheap and catch most real errors; 4–7 exist
because a per-function diff is structurally blind to them; 8 is the only proof.

**1. Extract by ADDRESS, not by name.**
Names are not unique in a dtk dump. dtk appends `_<8 hex>` to disambiguate real
duplicate symbols, and invents `fn_<ADDR>` / `lbl_<ADDR>` / `func_<ADDR>` for
functions that have no symbol at all. `harness.extract` returns the **first**
name match and now warns when a name is ambiguous, but the warning goes to
**stderr** — if you capture stdout only, you will not see it. Take the address
out of `bin\dtk\wiimj2d_symbols.txt` and confirm the `.fn` you extracted starts
there. Not optional on any TU with unnamed functions: `d_a_en_blockmain.cpp` has
ten, and every diff taken against one of them before the fix was comparing the
wrong body.

**2. Assert the extracted size against the symbol map.**
The single highest-value check in this file, and it costs nothing:

```python
import sys; sys.path.insert(0, 'tools/auto_decomp')
import harness as h
path = 'tools/auto_decomp/work/<unit>/target.txt'
for name, size in h.list_functions(path, with_size=True):
    body = h.extract(path, name)
    got = len(body) * 4 if body else 0
    assert got == size, (name, size, got)
```

`list_functions(..., with_size=True)` reads the size out of dtk's own
`# .text:0x0 | 0x800331E0 | size: 0xC` comment, so the assertion cross-checks
the extractor against the disassembler. Verified clean over all 64 functions of
the lkuribo target. Do it on **both** sides — target and your own object — and
also against the symbol map (`name = .text:0xADDR; // type:function size:0xNN`).
Make it non-vacuous by extracting a name you know is ambiguous and confirming
the assertion fires.

**3. Diff the canonicalised text (`harness.diff_fn`).**
This is the iteration-speed view and nothing more. It proves the instruction
stream, the register allocation, the immediates and — since `7fe054f` — the
branch displacements. It does **not** prove which symbol you called, which
literal you loaded, or where the function landed. Read its `NOTE:` output: when
it prints the pooled-literals caveat, steps 5 and 6 are mandatory, not optional.

**4. Compare the raw instruction words — and the callee NAMES, separately.**
dtk zeroes relocated fields, so every `bl` in every function renders as
`48000001`. **Two functions calling completely different callees compare equal
at the word level.** This has hidden a wrong callee twice: an inherited-but-
hidden method (`posMove` where the original calls `dBaseActor_c::posMove`), and
a `u32`-vs-`unsigned long` mangling difference that named a symbol which does
not exist. Whoever compares by words MUST also compare the disassembly text or
the relocation symbol names:

```
& "C:\devkitPro\devkitPPC\bin\powerpc-eabi-readelf.exe" -rW bin\compiled\wiimj2d\dol\bases\<file>.o
```

Better still, resolve every `.text` relocation to **(section, offset)** and
compare those, not names — pool symbols are anonymous, and the target embeds the
address in names like `@73081_803536E0`, so both sides are resolvable. Done that
way for all 67 functions of the jimen TU.

**5. Check the DATA relocations, not only the `.text` ones.**
A symbol reached through a compound-literal or aggregate initialiser never
appears in `.text` at all. The `&StateID_DieOther` vs `&dEn_c::StateID_DieOther`
experiment on `d_a_en_jimen_pakkun_base.cpp` passed `diff_fn`, raw bytes **and**
`.text` relocation names while being knowingly wrong; only `.rela.rodata` caught
it. If your function emits or reads a `.rodata`/`.data` template, read that
template's relocations and confirm each target symbol by name.

**6. Compare pool literals and table CONTENTS as BYTES, against the original.**
Canonicalisation reduces a pool reference to a positional marker, so **the
literal's value is never compared**. Confirmed experimentally: change `16.0f` to
`15.0f` in a matching function and `diff_fn` still reports MATCH. The same holds
for `.rodata` tables — the indexing code matches while the table is wrong. Read
the bytes on both sides and compare them yourself:

```python
import sys; sys.path.insert(0, 'tools')
from datarefs import load_dol, read
d, secs = load_dol('original/wiimj2d.dol')
print(read(d, secs, 0x8042B7E8, 16).hex())     # target slot
```

and against your own object:

```
& "C:\devkitPro\devkitPPC\bin\powerpc-eabi-readelf.exe" -x .sdata2 bin\compiled\wiimj2d\dol\bases\<file>.o
& "C:\devkitPro\devkitPPC\bin\powerpc-eabi-readelf.exe" -x .rodata bin\compiled\wiimj2d\dol\bases\<file>.o
```

**Make this check non-vacuous before you trust it**: deliberately corrupt one
value — one float, one table entry — and confirm the comparison fails. A byte
comparison against the wrong address range passes trivially and forever.

**7. Verify the emitted symbol ORDER.**
Not a bug, a structural limit: `diff_fn` looks a function up by name and compares
its contents, so **moving a definition changes where it lands, not what it
contains**, and every per-function view — canonicalised text, raw bytes,
relocation names, pool values — passes on a file whose functions are in the wrong
order. This project fails on emission order regularly (the lkuribo 32 bytes, the
pakkun flush block, `downSE`). Extract both function lists with
`h.list_functions()` — the same parser, both in emission order — and compare the
sequences. Control it by moving one definition and confirming the check fires;
on `d_a_en_jimen_pakkun_base.cpp` that control caught a misplaced `downSE` at
position 36, and no other view would have.

**8. Only the full link plus MD5 is authoritative — and only on a build that
actually linked.**

```
python configure.py; ninja; if ($?) { python progress.py --verify-bin }
```

`--verify-bin` MD5s `bin\wiimj2d.dol` and the four RELs against `original\`. It
reads whatever is on disk. **After a link failure the previous build's outputs
are still sitting there, and `--verify-bin` will happily verify them and print
five cheerful OKs.** A verification tool that passes on stale output is a false
green, and it is the most dangerous one in the list because it is the check
everything else defers to. So:

- Chain it off ninja's exit status (`ninja; if ($?) { ... }`) — never run it as a
  separate command after eyeballing the build log.
- Confirm freshness independently before believing a green:
  `Get-Item bin\wiimj2d.dol, source\dol\bases\<file>.cpp | Select Name, LastWriteTime`
  — the binary must be newer than every source you touched.
- `ninja` on an up-to-date tree says `no work to do`. If it says anything else
  right after a "successful" build, the build was not successful.

Everything in steps 1–7 exists to make step 8 likely, and to localise the failure
when step 8 says no. Nothing in 1–7 is evidence that a TU is done. Note that
every TU banked as matching passed the full-link MD5 even during the periods when
defects 2, 3 and 4 were live — **the link is the only check that a bad comparator
has never been able to fool.**

### The six defects, and where each one now stands

**1. `fndiff.py` reported `IDENTICAL` when it could not find the function.**
Template-mangled names (anything with `PrintContext<w>`) appear *quoted* in the
dtk dump, the name comparison never matched, and empty-vs-empty compared equal.
Nine in-flight functions affected. **Fixed:** quotes stripped on both sides, and
it hard-exits if either extraction is empty. A tool that cannot find the function
must say so, not congratulate you. **Historical only — `fndiff.py` no longer
exists anywhere in the tree**; it is recorded for the pattern, not for use.

**2. `harness.canonicalise` was wrong in both directions on pool symbols.**
dtk names a pool object `@71831_8042B7EC` in the original where a fresh object has
a bare `@21389`; collapsing both to one marker produced a spurious diff on every
`.sdata2` reference, and erased *which* literal was referenced, so `0.0f` and
`8.0f` compared equal. **Fixed:** each distinct pool symbol is numbered by first
appearance **per side**, so "the same literal twice" stays distinguishable from
"two different literals". **The caveat survives and is yours to close:** this
proves the *pattern* of references, not the values — see checklist step 6.

**3. The same comparator erased branch destinations.**
`ADDR_SUFFIX_INLINE` reduced both `.L_8005DB2C` and `.L_00000B38` to a bare `.L`,
so two functions whose branches went to different places compared equal. Every
loop, conditional and switch in the project was being diffed with its control
flow invisible. **Fixed in `7fe054f`** by keeping the raw instruction word for
local branches: those are PC-relative and carry no relocation, so identical code
always yields an identical word — exact, not heuristic.
*The instructive part is the fix that did not work.* Numbering labels per side by
first appearance — the trick that fixed the pool symbols — looks right and fails,
because it numbers by **use** order: swap two branches and both sides renumber
identically. Caught only because the negative control was written before the fix
was trusted.

**4. `harness.norm_name` collapsed every unnamed function to one name.**
dtk invents `fn_<ADDR>` / `lbl_<ADDR>` for functions with no symbol. `norm_name`
stripped a trailing `_<8 hex>` — correct for dtk's disambiguation of genuine
duplicate symbol names, catastrophic for a placeholder where **the address IS the
name**. Every `fn_800XXXXX` normalised to the bare string `fn`, so `extract()`
returned whichever unnamed function appeared *first in the file*. On
`d_a_en_blockmain.cpp` — ten unnamed functions across three batches — every diff
against one of them was comparing the wrong body, and all ten returned the same
50-instruction body. **Fixed in `47d15ca`:** placeholder names are left intact,
and `extract()` pre-counts matches and warns on an ambiguous name. Verified on
all ten; each now extracts at exactly its symbol-map size.
**This is the defect that justifies checklist step 2.** No negative control could
have caught it — the wrong-function comparison was self-consistent and passed
every check the tool had.

**5. `sibmap.py`'s FAMILY list rots silently.**
A FAMILY entry that matches no corpus file contributes no hits and raises
nothing. `bin/dtkspl` is regenerated rarely and lags the newest units, so a
just-landed TU exists **only** as our own compiled `CMP_`-prefixed object; the
plain `dol_bases_<file>` name then resolved to nothing and one run silently lost
199 functions from the family view. **Fixed in `8f323f0`:** `in_family()` strips
the `CMP_` prefix, and `check_family()` warns by name about dead entries (to
**stderr** — capture it). Verified against the real 319-file corpus: 18 of 19
live, the dead one named. `3681c28` then corrected the `REL_` tag spelling for
entries whose object lives in a REL slice.
Two standing traps here, both still yours:
- **Do not validate FAMILY against `slices/wiimj2d.json` alone.** Some banked
  matching units live in the REL slices (`slices/d_enemiesNP.json` and friends),
  and the corpus builder does read REL objects. A checker that only reads
  `wiimj2d.json` will wrongly reject a correct entry. Note also that
  `build_cache()` collects our own compiled objects **only** from
  `bin/compiled/wiimj2d`, so `bin/compiled/d_enemiesNP/...` is not a fallback.
- **Add every newly banked enemy TU the day it lands.** The newest entries are
  the most valuable, not the least — `d_a_en_bros_base` alone contributed 99
  matching precedent functions. The list is already one TU behind:
  `dol_bases_d_a_en_blockmain` is missing, and so is `d_a_fireball_player`.

**6. `datarefs.py` did not model update-form load writeback.**
`lwzu`/`stwu` write the effective address back into rA. One
`lwzu r12, -0x1910(r5)` left a stale base, and the following seven `lwz N(r5)`
resolved ~0x1900 too high — into the **next** unit's `.rodata`. The phantom
addresses were plausible, contiguous, and looked exactly like a genuine
cross-unit reference, which is evidence you would act on when deriving section
bounds. **Fixed in `f82d77e`** and verified against the exact case: the eleven
update-form opcodes are tracked and rA is updated (or dropped when unresolvable).
Note the general shape — a data-flow tool that models *most* of the machine will
hand you a wrong answer in the same format as a right one.

### Residual blind spots — read before relying on the fixes

Confirmed against the current source this session. None is fixed.

- **The `fn_<ADDR>` collapse is only half fixed.** `norm_name` no longer strips
  the address from a placeholder *function name*, but `canonicalise()` still
  applies `ADDR_SUFFIX_INLINE` to instruction **operands**, and `POOL_SYM`
  numbers `lbl_########` positionally. Directly reproduced:

  ```
  A: ['bl fn_800A1234', 'bl lbl_800B0000']  ->  ['bl fn', 'bl SYM0']
  B: ['bl fn_800CDEF0', 'bl lbl_800C1111']  ->  ['bl fn', 'bl SYM0']
  EQUAL? True
  ```

  So a call to one unnamed function still compares equal to a call to a
  different one. This mostly bites when **both** sides come from dtk dumps —
  target-against-corpus or split-against-split comparisons, i.e. exactly the
  sibling-mapping workflow — because a freshly compiled object names all its
  functions. Treat any `bl fn`/`bl SYM` line in a `diff_fn` report as
  uncompared, and fall back to checklist step 4. (`3681c28` fixed the
  operand-level case in `harness.py`; re-confirm the reproduction above before
  assuming it is closed for your workflow.)

- **Both new warnings print to stderr.** `extract()`'s ambiguity warning and
  `check_family()`'s dead-entry warning are invisible to any agent or script
  that captures stdout only. Redirect and read them, or the fixes are decorative.

- **The sibmap disassembly cache does not exist in a fresh working tree**
  (`tools/dis`, or `$env:SIBMAP_DIS`). `check_family` is called from
  `load_corpus`, so on a missing cache it reports *every* entry dead — which is
  the correct loud failure, but do not mistake it for list rot.

### Adding a check, or a tool

1. **State what the check cannot see, in the tool, next to the code.** Every fix
   above is documented in-source for exactly this reason; that is why defects 2
   and 3 did not recur.
2. **Break it on purpose and confirm it fires** — before you trust one green from
   it. A check you have never seen fail is not a check.
3. **Prefer positive assertions against independent facts** (size from the symbol
   map, bytes from `original/wiimj2d.dol`) over asking a comparator whether it
   agrees with itself.
4. **Relay a tooling bug the moment you find one.** The pool-normaliser bug
   reached three agents before they wasted time on phantom diffs; two had already
   hit it and worked around it independently, which is pure duplicated cost.
5. **Anything worth keeping goes under `tools/` and gets committed.** The
   scratchpad is untracked and comparators have been lost from it before.

Six down. **Assume the seventh exists.**

## Relay findings between running agents

Several results only emerged because one agent's finding reached another
mid-flight (`SendMessage`). It is cheap and it has paid every time. Worth
relaying immediately:

- **A bug in the shared tooling.** The pool-normaliser bug reached three agents
  before they wasted time on phantom diffs; two had already hit it and worked
  around it independently.
- **Corrections to rules *you* gave them.** The FPR direction, a `.sdata` vs
  `.sbss` slip, a vtable displacement table that was one slot off, and a wrong
  claim about which state called a shared helper — all originated with the lead
  and had to be pushed back out.
- **A matched caller pins down its callees' exact signatures.** `SetRouteInfo`
  handed five signatures to two agents still guessing; on lkuribo, a matched
  `create()` confirmed a pointer-to-member signature for the agent still writing
  the callee.
- **A statement-ordering trick** found in one function often applies verbatim to
  a sibling another agent owns.
- **Corrections to the correspondence map.** Its structural claims held
  throughout, but three statement-level details were wrong, and each correction
  transferred: one told an agent to hoist a call into its own statement when the
  target needed the opposite — both calls inlined into the argument list, letting
  right-to-left argument evaluation do the ordering.
- **Which pool literals are already accounted for.** As groups finish, the
  unclaimed `.sdata2` slots narrow, and telling the remaining agents which are
  uniquely theirs turns the pool into a positive check on their hardest function.

Relay results *between* groups too, not just corrections: "group 6 matched using
slot 0x2E8, so the corrected table is confirmed by bytes rather than inference"
is worth more than repeating the table.

## Check `syms.txt` before inferring a name

Two functions believed unnamed were already named there, one of them called by an
already-matching destructor. Grep `syms.txt` and `bin/dtk/wiimj2d_symbols.txt`
first; mark genuinely inferred names `@unofficial`.

**A TU can be mostly unnamed, and that changes how you verify it.**
`d_a_en_blockmain.cpp` has ten file-static functions with no symbol-map name —
2,800 B, 22% of the unit, including its two largest. Every one of those names is
invented and marked `@unofficial`. Two consequences, both paid for:

- **Diff those functions by ADDRESS.** Before `47d15ca`, `harness.py` collapsed
  every `fn_800XXXXX` placeholder to the bare string `fn`, so all ten extracted
  the *same* body and every diff against them was comparing the wrong function
  while reporting cleanly. Assert instruction count × 4 against the symbol map.
- **Keep the invented name→address mapping as an artefact** through integration.
  Discarding those names as "unmatched extras" desynchronises any positional
  comparison and manufactures dozens of spurious differences.

---

## Current state

- **Progress: 11.088%** (720,792 / 6,500,368 code bytes)
- All five binaries verify byte-for-byte (`progress.py --verify-bin` → 5 OK)
- **70 commits unpushed.** Ask before pushing.
- Development happens on **native Windows**; see "Local setup" below.
- Last TU banked: `d_a_player_demo_manager.cpp` (51 fns, 9,280-byte span).
- Before it: `d_a_en_hatena_balloon.cpp` (81 fns, 18,216 bytes of code in an
  18,768-byte span) -- the largest single unit landed so far.
- Previously: `d_a_en_blockmain.cpp` (97 fns, 12,604 bytes of code in a
  13,232-byte span), whole and byte-exact. Before that:
  `d_a_en_bros_base.cpp` (99), `d_a_en_jimen_pakkun_base.cpp` (67),
  `d_a_en_dfpakkun.cpp` (33), `d_a_en_dpakkun_base.cpp` (64),
  `d_a_en_lkuribo_base.cpp` (58), `d_a_en_kuribo_base.cpp` (66), `d_a_en_door.cpp` (50),
  `d_a_fireball_base.cpp` (51), `d_a_en_net_nokonoko_base.cpp` (37),
  `d_a_enemy_ice.cpp` (37), `d_a_rot_objs_base.cpp` (31),
  `d_a_spin_child_base.cpp` (23), `d_a_sink_dokan.cpp` (14), `d_a_cursor.cpp` (9),
  `d_a_rot_block.cpp` (5), and earlier `d_wm_csvdata.cpp` (41),
  `d_a_en_super_bigpile.cpp` (46), `d_tag_processor.cpp` (39).
- `d_a_en_dpakkun_base.cpp` (64/64) and `d_a_en_dfpakkun.cpp` (33/33) are landed
  and linked.
- `d_a_player_hio_ADJ.cpp` is banked `nonMatching` at **15/16**, with `.rodata`
  and `.sdata2` byte-identical. Defining the TU's `.rodata` for real (rather
  than `extern`) reproduced the address sharing and closed two of three; the
  `const` lever above closed them. **One function is left**,
  `resetParam__14dPyModel_HIO_cFi`: 46/53 words match and the residual is a
  single two-register permutation. ~400 variants were swept, and the useful
  negative result is that **every 53-instruction form puts the base in r7 and
  every form that puts it in r8 costs 54** — the two have never been obtained
  together, which points at a virtual-register count this source shape does not
  reproduce. Do not grind that axis further without a new idea.

Per-binary:

| Binary | Progress |
|---|---|
| `wiimj2d.dol` | 21.887% |
| `d_profileNP.rel` | 100% |
| `d_enemiesNP.rel` | 2.056% |
| `d_basesNP.rel` | 1.015% |
| `d_en_bossNP.rel` | 0.031% |

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

1. Pick a target — see "Next target — and what is known about the rest" above,
   which is the only game-code target list. (The SDK list further down is
   deprioritised.)
2. Disassemble the region that contains it:
   `.\bin\dtk-windows-x86_64.exe elf disasm bin\dtkspl\obj\<auto_..._text.o> <out>`
   — `bin/dtkspl` is authoritative for undone ranges, which is what you are
   disassembling here; it is stale only for ranges already banked.
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

The newest set — the source-shape levers that closed bros and blockmain, stated
symptom-first — is at the end of this section under "Code-generation levers from
the bros/blockmain pair". It refines several of the entries below rather than
replacing them, so read those first if a lever there points back at one.

### A one-word difference in a destructor is the last member's OFFSET, not `sizeof`

The most expensive near-miss of the bros unit, and the one most likely to recur.

`__dt__14daEnBrosBase_cFv` differs from `daEnLkuriboBase_c`'s destructor in
**exactly one word out of fifty**: `addic. r31, r3, 0x724` where lkuribo has
`0x5f0`. That reads irresistibly like an object size, and one agent reported it
as `sizeof == 0x724`. It is not. `0x5F0` is lkuribo's **`mEffect` member
offset** — lkuribo's actual `sizeof` is `0x770`. The word is the offset of the
last destructible member in both. Bros's real `sizeof` is `0x850`.

Why this matters more than an ordinary slip: **no per-function diff can catch
it.** The destructor is byte-identical either way. It surfaces only when a
derived actor is laid out, and this is a *base* class — every actor built on it
would inherit the error. Had it gone in, the file would have matched 99/99 and
still been wrong.

**Rules.** When two agents disagree, resolve it against the repo, do not pick
the more confident report. Confirm `sizeof` by *compiling* — a `char x[sizeof
(T)];` probe disassembles to a `.skip` you can read directly — and cross-check
against a banked sibling's member table rather than reasoning from one operand.
And brief agents to **report contradictions rather than reconcile them**; that
instruction is what surfaced this at all.

### An inline destructor can misorder the whole trailing flush block

Bros first assembled with all 99 functions byte-exact and the trailing cascade
in the wrong order: `mEf::effect_c`'s destructor was hoisted to the front of the
block, ahead of `~daEnBrosBase_c`. The unit was 99/99 on content and still would
not have matched.

The cause was **`virtual ~daEnBrosBase_c() {}` defined inline in the header.**
Declaring it in the header and defining it out of line **last in the `.cpp`** —
exactly the form `d_a_en_lkuribo_base.cpp` already uses — fixes it, and the
whole cascade (`levelEffect_c` → `effect_c` → `nodeCallback_c` → `callback_c` →
`anmChr_c` → `timingC` → `timingB`) then falls out in target order for free.

Two things worth knowing before you spend time on this:

- **`#include` order is NOT the lever here.** The misordering only appears once
  `d_a_player.hpp` is in the include set, which makes it look like an include
  problem. Sweeping that header through all nine positions changed nothing;
  removing it is impossible (the unit needs `dAcPy_c`). The definition *site*
  of the destructor is the lever, not the include order.
- **A partial build lies about this.** Two agents independently reported the
  tail order correct from their own subsets, and both were right about their
  subsets. Flush order is a property of the *whole* TU and can only be checked
  after assembly.

The general form: if content matches but the trailing block is misordered, look
at which functions are defined inline in the header versus out of line in the
`.cpp`, and compare against a banked sibling with the same member shape.

### Weak destructors of embedded effect members need `keepWeak`

Bros's first full build failed all five binaries with `.text` exactly **0xE0
short**. The cause: the weak copies of `mEf::levelEffect_c::~levelEffect_c` and
`mEf::effect_c::~effect_c` were resolved from another object, leaving a
224-byte hole — 0x80025F60 to 0x80026040 inclusive of alignment, exactly 0xE0.
Adding both to `keepWeak` fixed it.

The diagnosis path is worth copying, because the obvious first move is wrong.
The DOL section table gives you the size delta but the first differing `.text`
byte is meaningless — it sits near the start of the file, because a missing
0xE0 shifts every later branch displacement. **Parse the linked
`bin/wiimj2d.elf` symbol table and compare each function's address against
`bin/dtk/wiimj2d_symbols.txt`**; the functions reported `ABSENT` inside your
range are the hole, and the ones that shift by exactly the deficit bracket it.
That took one step where byte-diffing took several and pointed nowhere.

Note also what *didn't* help: six other weak symbols added to `keepWeak` on
suspicion changed the output not at all, and were removed. Add `keepWeak`
entries against evidence, not on principle — it is a global list.

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

### The lazy-flush rule: called inline virtuals and template instantiations

This was the **last 32 bytes** of `d_a_en_lkuribo_base.cpp` — every function
matched, every section was the right size, the DOL section table was identical,
and the binary still failed because two adjacent functions came out swapped.

**The rule.** A called-but-inline virtual and a template instantiation queued by
the *same* caller are both flushed immediately after that caller, with the
**inline virtual emitted first**. So:

- an empty virtual that is **called** in the TU belongs **inline in the class
  body** — an out-of-line copy is still emitted, lazily, at its first caller's
  flush point, which is where the original has it;
- defining it out of line instead emits it one slot too early, ahead of the
  template instantiation the same caller queues.

Confirmed against `d_a_en_shell.cpp`, which is byte-exact and has the identical
shape: `block_hit_init` calls both `mStateMgr.initializeState()` and the inline
`isBlockHitDeath()`, and the two flushed bodies follow it in exactly that order.

**What was disproved on the way, so nobody repeats it:** the flush is *always*
immediate, so moving the function's definition around does nothing — six
placements were tried and every one produced the wrong order. Naming the member
through the abstract base suppresses the instantiation entirely; dead code
(`if (0)`) does not queue it at all, because MWCC drops the branch first. And the
calling function is byte-identical under all eight variants, so **its own code
can never tell you which form the original used** — only the neighbouring bytes
can.

Side effect to handle: inline-in-class flips the symbol from `GLOBAL` to `WEAK`,
so it needs a `keepWeak` entry. If `.text` comes out 4–16 bytes short, that is why.

**`d_a_en_dfpakkun.cpp` added a third case, so the rule is now three-way** and
each case was measured against the alternative rather than argued:

| The empty virtual is… | Put it | Why |
|---|---|---|
| never called, and sits at a natural source position | **out of line** | `-ipa file` would inline it away and break the vtable slot |
| called | **inline in the class body** | flushes at its first caller, which is where the original has it |
| never called, but sits **inside the end-of-TU flush block** | **inline in the class body** | out of line it is emitted at its source position, which is always ahead of that block, so it can never reach the address |

`calcFirePrm` is the third case: it is called nowhere in the entire DOL, yet it
lives at 0x8002A1E0 between `hitCallback_Spin` and `m3d::banm_c::play`. Two
never-called virtuals in the *same file* went out of line and two others went
inline, so "is it called?" alone does not decide it — **check where the target
address falls** as well.

### `.text` too long? Look for weak symbols you are the only one defining

The mirror of the deadstrip signature (`.text` short), and the fix is the
opposite one. Diagnose by listing your object's `FUNC` symbols against the
original's symbols in your address range: anything of yours that the original
places *elsewhere* is a candidate, and the fix is a `syms.txt` entry at the
original's address. Full account, with the two other link-time failures that
travel with it, under "The pakkun pair — DONE, and the two rules it cost".

### New lever: all words identical, only the `0xNN(r1)` slots wrong

MWCC allocates by-value argument temporaries in **two passes**: temps for calls
written directly in the function body first, downward from the top of the temp
area, then temps created while expanding an **inline wrapper**, below them. Each
pass is in first-use order.

So a diff where every instruction word is right and *only* stack displacements
are wrong means some calls go through an inline wrapper in the real source and
yours do not. Counting the two descending blocks tells you exactly how many.

On `createMdl` the target's blocks were 6 and 6. Writing all four `create()`
calls explicitly gave one block and 22 wrong slots; switching three of them to
their inline `nullptr`-defaulting overloads fixed 10 and left exactly 2 — which
is how a **missing overload in `m3d::anmChrBlend_c`** was found rather than
ground at with register permutations. Do not reach for the register levers on
this signature; it is a source-shape problem.

Quantified on `daEnJimenPakkunBase_c::createMdl`, which needed the same fix in
`m3d::anmMatClr_c`: **of the 12 plausible overload combinations exactly one
matched** (4-arg / 3-arg / 3-arg). So enumerate rather than guess. The
diagnostic that tells you an object is outside the temp pool: with N pool temps
MWCC allocates top-down from `0x8 + 4*(N-1)`, so a sequence that starts one slot
below the top and wraps means something is not in the pool.

### `const` on a source table changes the whole copy strategy

The highest-value lever found this session, and it is invisible in the
statements — only the qualifier moves.

**MWCC copies a struct field by field. When it can see the source cannot change
under it — a `const` object, or a source reached through a pointer it must
assume aliases the destination — it hoists every load ahead of every store.**
Drop the `const` and the identical source interleaves them one load/store pair
at a time.

The batched form is what produces a big `_savegpr_14`/`_restgpr_14` frame and a
pile of spill slots. On `dAcPy_HIO_Speed_c::init` the target's 157 instructions
with ten spill slots are simply the batched form of two struct assignments; the
same source without `const` emits the same 60 loads and stores in 127
instructions with no saved registers.

**If the target has far more live registers and stack spill than your version,
and the statements already look right, suspect this before touching statement
order.** It also explains "an FPR pair lands swapped" — that was the same cause
on `dPyAnm_HIO_c::resetParam`, not a register-allocation problem at all.
Roughly 30 register-permutation builds were spent on that function before a
three-build micro-benchmark found the qualifier.

### A non-zero constant in a brace initialiser can become a static template

`{ 1, offset, mVec3_c(...) }` emitted 93 instructions — MWCC built a `.data`
template and copied it in word by word. Routing the value through a plain
`int mode = 1;` gave the target's 76. **`const int` folds and does NOT work.**

The sibling actor escapes this only because its equivalent value is `0`, so a
precedent that looks identical can be silently inapplicable. If a brace
initialiser comes out far longer than the target and you see a `.data` template
you did not expect, this is why.

### A shadowed state ID compiles, diffs clean, and is wrong

`daEnLkuriboBase_c` declares its own `StateID_DieFall`, shadowing `dEn_c`'s.
Written unqualified, the code compiles and the function diff is **clean**,
because the field is a relocation and relocations read as zero in a `.o`. It was
caught only by reading `.rela.rodata` directly. Write
`&dEn_c::StateID_DieFall`.

Note the trap arrives by *copying a correct line*: `d_a_en_kuribo_base.cpp` writes
it unqualified and is right to, because that class has no `DieFall` of its own.

**Generalise this: a function-body diff cannot see which symbol a relocated word
points at.** After a match, read the object's relocations for any `.rodata` or
`.data` template you emit and confirm the target symbol by name. Two death-info
templates in this TU differ only in fields that are relocations plus three words.

**And it is worse than "check the relocations" — checking the `.text` ones is
not enough.** On `d_a_en_jimen_pakkun_base.cpp` an agent deliberately broke
`setDeathInfo_Quake` with a bare `&StateID_DieOther` where the original binds
`&dEn_c::StateID_DieOther`, then ran every check this project has:

| check | verdict on knowingly-wrong code |
|---|---|
| `harness.diff_fn` | **MATCH** |
| raw instruction-byte comparison | **MATCH** |
| `.text` relocation symbol names | **MATCH** |

All three pass, because the wrong binding never appears in `.text` at all — the
state-ID address is baked into a **`.rodata` template's data relocation**
(`@73419`, +0x10 into the object). Only reading `.rela.rodata` catches it.

**So: any function that reaches a symbol through a compound-literal or aggregate
initialiser must have its DATA relocations checked, not just its code.** Both
directions genuinely occur in the same file — `setDeathInfo_IceBreak`'s template
binds this class's own `StateID_DieIceBreak` (unqualified is correct) while
`setDeathInfo_Quake`'s binds the base's `StateID_DieOther` (qualification
required). Never infer the direction; read it.

Related lever from the same TU: a call feeding a `(sDeathInfoData){...}`
initialiser must be hoisted into a **named local first**. Written inline the call
is scheduled after the template copy and costs +7 instructions;
`d_enemy_death.cpp` writes it the hoisted way.

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

### What eight parallel batches cost, and what only assembly can catch

The hatena_balloon run used eight authoring batches instead of six. Seven
reported byte-exact; assembly still found four defects, and **three of them were
invisible to every per-batch check** because they were disagreements *between*
batches, each of which was locally correct:

- **A return type two batches disagreed on.** `all_bgcheck` returns `int`, not
  `u8` — `u8` makes callers emit a `clrlwi` mask the target does not have. The
  callee's author said `u8`, the caller's author said `int`; the caller was
  right, and only linking them together showed it.
- **A member signedness two batches disagreed on**, in the opposite direction.
  `dEnemyMng_c::m_110` is `u32`, not `int`.
- **A `.data` ordering defect that surfaced as seven wrong words in a function
  that was byte-exact standalone** (see the string-literal lever below).

**So: when two batches disagree about a shared declaration, do not pick the more
confident report — record both and let assembly decide.** Both disagreements
here were resolved correctly by the byte evidence at assembly, and both would
have been landed wrong if I had chosen at briefing time.

Also worth carrying forward: **agents shadow-copy shared headers rather than
editing them**, which worked well across eight batches — but a *stale* shadow
tree left by a previous run masked a real header fix and cost a build cycle.
Tell agents to delete any pre-existing `inc/` shadow before starting.

And **preserve drafts to the repo, not the scratch directory.** A session limit
killed all eight mid-flight; the two batches whose drafts had been copied into
`wip/` resumed from real work, the six that had not started cold. One agent
repointed its verifiers at the repo's own target dump so they still run after
the session ends — copy that habit.

### Levers from d_a_en_hatena_balloon (81 functions, eight batches)

Added by the hatena_balloon run. Several sharpen or contradict entries above —
where they do, this section is the later measurement.

#### Two coupled changes: why single-axis sweeping can be structurally blind

The unit's last gap — two adjacent independent `lfs` emitted in the wrong order
in `fly_ydisp_check`, 2 words of 55 — **needed two source changes at once, and
each one alone scores far WORSE than the defect it repairs**:

1. read the two members into named locals (`bgpY`, then `bgDispY`) and subtract
   those, instead of reading them inline in the expression; **and**
2. declare the unrelated local `lim` **after** those two.

Change (1) alone fixes the load order but rotates the FP allocation — `lim`
lands in f2 rather than f3 and roughly 30 lines cascade wrong through the rest
of the body. So a sweep along *either* axis alone sees only regressions and
correctly concludes "dead end". Roughly **120 variants across three independent
sweeps** (expression spelling, statement/local ordering, and a 3x3 grid of the
two accessor spellings) all failed for exactly this reason. Two agents each
independently concluded the cause must lie in *the other's* axis — which, in
hindsight, was the tell.

**The lesson is about search shape, not about this function.** When a defect is
down to a pure scheduling or register rotation and every single-axis sweep comes
back worse, stop sweeping wider on one axis and start sweeping **pairs**. And
treat "two independent sweeps each blame the other's axis" as positive evidence
of a coupled cause rather than as two dead ends.

**What actually found it was mining an already-matching sibling for the idiom.**
The exact lever — those two named locals, in that order — was already present
and *commented* in the same file, in `executeState_DispFlyMove`, which needs it
for the identical pair of loads. Minutes, against three exhausted sweeps. This
is the "map the siblings first" rule applying at the level of a single statement,
and it is the cheapest move available when a function is down to a rotation.

#### Inline accessors are NOT transparent substitutes at -O4

Measured on `dBgParameter_c`: replacing `ms_Instance_p->mPos.y` with the
class's own `pos().y` or `yStart()` — which return the same member and look
purely cosmetic — took a 2-word miss to a **32-line** one. Inlining `yStart()`
forces a reload of `ms_Instance_p` later in the body and grows the function by an
instruction. So an accessor and a direct member read are **different source
programs** here, not stylistic alternatives. Sweep them as their own axis, and
do not assume the "prettier" spelling is what the original used.

#### A literal can be unwriteable as itself

The target holds **0x3ED70A3E** in a slot where the literal `0.42f` compiles to
**0x3ED70A3D** — one ULP off, and no decimal spelling reaches it. `0.3f * 1.4f`
folds at compile time to exactly 0x3ED70A3E. The surrounding branch is
`base x 1.4f` throughout (0.9/0.3/1.8/0.45), and the other three fold identically
either way, so only that slot discriminates — and only in favour of the product.

**A one-ULP miss is a folding question, not a wrong number.** The original wrote
an expression; you cannot always write the result back as a decimal. Verified
independently.

#### Vector-construction spelling permutes FP temporaries with no instruction change

`mVec3_c pt(base.x + dx, base.y + dy, base.z)` and
`mVec3_c pt(base); pt.x += dx; pt.y += dy;` emit **the same 11 instructions in
the same order**, with f0-f3 permuted four ways. Sixteen spellings were swept;
hoisting operands into named locals reached 6-8 differing lines and only the
copy-then-offset form landed. **If a function is down to a pure FP rotation with
the schedule already correct, sweep the construction spelling before anything
else.** `pos.y += K` versus `pos.y = pos.y + K` cost **38 words** in another
function — the sharpest confirmation of the compound-assignment form yet.

#### Naming a local fixes whichever operand you name into first position

The strongest single lever in this unit, and it cuts both ways:

- Three functions stuck at 2 differing lines closed by **deleting** the local:
  `getLoopScrollDispPosX(mPos.x) + xSize() - 16.0f` with no `float w`. Binding
  either operand to a name produced the wrong order, and "bind BOTH operands"
  made it *worse* (10 lines).
- Another function needed the opposite: binding only the **left** value
  (`float y = ...; mPos.y = y + l_create_diff[m_7f0];`) flipped `fadds` to
  (value, diff).
- A third closed only by binding **both**, declared in the target's load order,
  after 13 variants had plateaued at 2.

So the rule is not "locals are good" or "locals are bad". **A named local forces
that operand into first position; choose which one you need there.** Related:
**declaration order is evaluation order, not argument order** — `float dy = ...;
float dx = ...; mVec3_c diff(dx, dy, 0.0f);` with `dy` declared first though it
is the second argument. Unnamed temps number right-to-left; named ones number in
declaration order. And **a named local for a value being read can force an extra
callee-saved FPR** — reading the member directly at the comparison closed a frame
that `float speedF = mSpeedF` had widened by 4 words.

#### Use the class's inline accessors

`dBgParameter_c::xSize()/ySize()/yStart()/yEnd()` versus open-coded `mSize.x` /
`mPos.y` **transposes f0 and f2** on the surrounding `fmuls`/`fsubs`. Four
functions closed only with the accessor form. Same rule as `getCenterX()` above,
now confirmed on a second class. A shimmed variant that read members directly
inside the accessors made no difference, so the lever is about *calling* the
accessor, not how it is written.

#### A stubborn `fmuls` operand order may be a folded divide

`fmuls f29, f4, f0` came from `half / 8.0f`, not `half * 0.125f`. CodeWarrior
folds a power-of-two divide and the folded form emits `fmuls(numerator,
reciprocal)`. The already-matching sibling `fly_xspeed_set`'s `half / 6.0f` is
what pointed at it. **When a `fmuls` against a power-of-two constant has stubborn
operand order, try spelling it as a divide.**

#### Signedness is visible and load-bearing

- A **`u8` return makes callers emit `clrlwi. r0,r3,24`**; `bool` and `int`
  returns both emit `cmpwi r3,0x0`. So `u8` is distinguishable from the object
  code but **`bool` vs `int` is not** — identical codegen in callee and caller.
  Do not sweep that axis; do check for the mask.
- A **`u8` member compared to a constant gives `cmplwi` read directly, `cmpwi`
  copied into an `int` local first.** Both shapes occur in one file.
- `m_110--` followed by `> 3` is an **unsigned wrap check**: the member must be
  `u32` or the compare comes out `cmpwi` instead of `cmplwi`.
- **`unsigned long` vs `u32` remains invisible except in the mangled name.**

#### A switch whose compare order differs from its body order is not a switch

Proven by exhaustive probe — all six case permutations against four default
positions — that a plain `switch` **always** emits compares and bodies in source
order. Reproducing compares 1, 0, 3 with bodies 2, 3, 0, default needs a **shared
case arm with an inner test**, where the inner condition folds into the case
dispatch compare and costs nothing:

```cpp
case 1: mDirection = 2; break;
case 0:
case 3: if (dir == 3) mDirection = 3; else mDirection = 0; break;
default: mDirection = 1; break;
```

#### Guard shape

`if (cond) { body; return X; } return Y;` rather than an early-return guard — the
tell is a branch-if-true to a trailing block. An `||` early return costs an extra
`beq +8; b end` and duplicates the `li r3,1`. Separately, **an unreachable
trailing branch in the target is a positive signal** to write explicit `if/else`,
not evidence the shape is wrong.

#### `const` on a by-value parameter, and memory across calls

A top-level **`const` on a by-value `mVec3_c` parameter** lets MWCC prove it
cannot alias the sret buffer, hoisting all loads above all stores and freeing an
FPR. It closed a function that had plateaued at 11-12 lines across ~45 variants,
and it is invisible in the mangled name. Separately, **CodeWarrior invalidates
memory across calls**: two source reads of one global separated by a call become
two real loads, and reading it into a local *before* the call is what forces the
callee-saved register the target uses.

#### String-literal order in `.data` follows declaration position

This cost an assembly cycle and surfaced as **seven wrong words in an unrelated
function**. A file-scope `static const char *[]` declared at the top of the file
put its strings at `g_profile+0xC`, displacing the strings the target has there
and shifting every later literal. Moving the array next to its only user fixed
it. **Declare literal-bearing file-scope data next to the function that uses it,
not in a preamble block** — and if a function differs only in a `.data`-base
displacement, suspect ordering rather than code.

### Code-generation levers from the bros/blockmain pair

`d_a_en_bros_base.cpp` (99 functions) and `d_a_en_blockmain.cpp` (97 functions)
both went byte-exact and linked this session. These are the source-shape levers
that closed them, ordered by how often they will be the answer. Each is stated
with the **symptom** first, because that is how you will be looking for it: you
have a diff in front of you, not a hypothesis.

Three of these were over-generalised on first report and had to be narrowed
before they were true. The narrow statement is the one written down. Do not
widen them again without a measurement.

#### Declare the local at the TOP of the function body, assign it later

**Symptom:** every instruction word is right and the callee-saved registers are
rotated as a *group* — not one pair swapped, a whole cycle of three or four.

**Fix:** move one local's *declaration* to the top of the function body while
leaving its assignment where it is. Declaration site and assignment site are
**independent axes**, and this is the axis nobody sweeps.

The strongest single result of the session. On one 768-byte function this was
the only thing that moved it after **~15 other variants** — declaration order,
types, init placement, `static`, return type — had all sat stable at the wrong
colouring. Hoisting one `int` declaration to the top rotated four registers into
the target's arrangement: **84 differing lines to 0**. It closed a second
function the same day, a pointer declared at the top and assigned inside an
`else` branch, 6 diffs to 0.

This refines "Declaration order controls register assignment" and "The GPR block
rule" above rather than replacing them. Those describe how the leading
declaration block is coloured; this says **which locals are in that block is
decided by where they are declared, not where they get a value**. That is why a
pure declaration-order sweep plateaus: it permutes the block without changing
its membership.

#### The same lever inside an inline helper — and the variable need not exist

**Symptom:** a small residual gap (here, the session's last 4 instructions) in a
function whose named locals you have already swept exhaustively, in registers
that never move no matter how you reorder those locals.

The contested register pair was not holding a source variable at all. It was an
**anonymous compiler temp created by a sound-playing template**. Making it a
named local inside the inline helper, and then top-declaring it there, moved the
pair.

Two things this proved, both reusable:

- **The shape is per-instantiation.** The defect could be *relocated* from one
  loop to the other at will, which is what established that the helper is
  coloured afresh at each expansion. Two differently-shaped helpers were needed,
  one per call site. If a helper is right at one call site and wrong at another,
  that is expected — write two.
- **The register the caller cannot reach is a temp.** Declaration order in the
  caller permuted three other registers freely and never once touched the
  contested pair. That asymmetry is the diagnostic: **if a sweep moves some
  registers but provably never the ones you need, the ones you need are
  allocated in a different pass, and the lever lives inside the inlined
  callee.** Compare "all words identical, only the `0xNN(r1)` slots wrong",
  which is the stack-slot form of the same two-pass behaviour.

#### A value live across two calls belongs in an inline helper's BODY

**Symptom:** the value is correctly kept in a non-volatile across both calls, but
in the wrong non-volatile, and no amount of reordering the named locals reaches
the target's register.

**Fix:** compute it inside an inlined helper rather than in a named local at the
call site. A named local colours into the **named-local band**; the same value
computed inside an inlined helper colours **after the loop-invariant temps**,
which is where the original puts it.

One function went **33 differing instructions to 0** on this, after **~90
variants** of declaration order, types and init placement had all stuck at 12.
Ninety variants at a constant floor is the signal, not bad luck — see "Narrowing
to 16 bits is a reassociation barrier" for the same lesson stated as a rule.

**This interacts with "A named local pins a value across two calls" above, and
the two are easy to read as contradicting.** They do not. The named local is
what forces the value into a non-volatile at all; the *band* it lands in is
decided by whether it is a named local or a helper-body temp. So: named local to
make it live across the calls, helper body to choose which register that is. If
your value is already surviving both calls and only the register is wrong, the
first lever is done and the second is the one left.

#### Binding a pointer to a named local at function scope flips r30/r31

**Symptom:** `self` and the other object are in each other's registers, and
nothing else differs.

| Source form | Result |
|---|---|
| `X *p = (X *)self;` at the top of the function | `r31 = self` |
| the same declared inside the guarded block | `r31 = other, r30 = self` |
| cast written inline at the tail call | `r31 = other, r30 = self` |

This was the **last diff in three separate functions**. Related, and worth
trying in the same pass: **declaring a shared temp before the return variable
flips which of them gets r4.**

#### Argument and expression shape

Each of these is zero-cost at the source level and changes register numbering or
scheduling. They are cheap to try and should be exhausted before any
register-colouring sweep.

- **`mVec3_c pos(v.x, v.y, K)` and `mVec3_c pos(v); pos.z = K;` are not
  interchangeable.** The 3-arg constructor allocates `x`→f1, `y`→f2. The
  copy-then-assign form allocates **strictly right-to-left** (`z`→f0, `y`→f1,
  `x`→f2) and dead-store-eliminates the `v.z` load. Worth 4 lines in each of
  four functions; found after **14 probe variants**.
- **Sharper form of the same thing: pass `mVec3_c` BY VALUE to a helper.**
  By-value fixes the FPR numbering; `const mVec3_c &` plus a local copy gives
  the reverse ordering and costs 4 lines. When the vector-argument FPRs are
  reversed, change the parameter, not the caller.
- **Force left-to-right evaluation by hoisting into a local.**
  `f(a, a->field, a->call())` evaluates right-to-left and loads `field` *after*
  the call. Writing `int x = a->field;` first puts it in a non-volatile before
  the call — which is what creates the saved-register slot the original has.
  Symptom: your version has one fewer non-volatile live across a call.
- **Collapse an OR-accumulation into one expression, with the nesting written
  explicitly.** `p = (a<<18) | ((b<<16) | (c|d|e));` took a function from **34
  differing lines to 8**. Split across two statements, MWCC schedules the second
  shift late and flips the operands of the final `or`.
- **`v.y += K` is not `v.y = v.y + K`.** The compound form changes `fadds`
  operand order *and* which register each `lfs` targets. Subtraction is
  unaffected, being non-commutative — so one file can legitimately need `+=` on
  one line and `= x - K` on the next. Do not normalise these for tidiness.
  (Compare the `fmuls` rule above: there only the compound form gives
  variable-first.)
- **Two stores of the same member need an explicit temp**, or aliasing forces a
  reload between them.

#### Comparison and branch shape — read the mnemonic, then pick the source

The target's compare sequence names the source form almost uniquely. Most of
these are one word either way, so **the mnemonic is the only evidence** and
guessing costs a whole rebuild.

| Target emits | Write |
|---|---|
| `cror eq,lt,eq; beq` | `if (x <= 0.0f) return;` |
| `ble` | `if (x > 0.0f) { … }` |
| `subic.` | `if (count - 1 != 0)` |
| `cmpwi` against 1 | `if (count != 1)` |
| `cmpwi rN, 0` | `if (num != 0)` |
| `beq END / bne +8 / b END` | `if (a == X \|\| a == Y) return;` — **not** two `beq END` |

Three that need more than a table row:

- **Float comparisons are written constant-first in this project's actor code**
  — `0.0f != mSpeed.x` — **but only for `fcmpu` against a literal.**
  Member-vs-member comparisons emit `fcmpo` and take the natural source order.
  The wide version of this rule ("always constant-first") was reported once and
  correctly pushed back on by two agents. Check which mnemonic the target uses
  before applying it.
- **Range optimisation of `a == X || a == Y` is type-dependent.** On a **u8**
  MWCC folds it into `(u8)(a-1) <= 1`, and folds the equivalent `switch` the same
  way; the explicit two-compare form (`cmplwi 1; beq; cmplwi 2; bne`) comes
  **only** from the negated chain `if (!(a != A && a != B))`. On a **u32** there
  is no folding at all — `a == X || a == Y` gives two explicit compares
  directly. Ten formulations were probed to establish this. Read the target's
  shape, then pick by the *type* of the operand.
- **A `subi / clrlwi 24 / cmplwi / ble` sequence is not a switch.** It is an
  explicit u8-truncated range test, `(u8)(v - LO) <= N`. A real `switch` over
  the same cases emits `extsb` plus a signed `cmpwi` chain and is one word
  longer.

Two scope limits found the hard way:

- **A dead branch in the target is a positive signal, not a mistake.**
  `if (c) { x = k; } else { return; }` leaves an unreachable `b END` after
  branch-to-branch simplification. When you see one, write the explicit
  if/else — do not "fix" it into an early-return guard.
- **Animation-frame checks are not a file-wide convention.** Some functions
  compare `getFrame()` against a constant explicitly; others genuinely call
  `fanm_c::checkFrame()`. Both occur in the same file. Match per function.

#### Types and signatures that no instruction diff can see

The dangerous class: **byte-identical code, wrong symbol.** Nothing that
compares words, and no per-function diff, will fire on either of these. Only a
comparison of *callee symbol names* catches them.

- **`unsigned long` vs `u32` is load-bearing in declarations.** The former
  mangles `Ul`, the latter `Ui`. The wrong choice names a symbol that does not
  exist while emitting byte-identical instructions. **This bit twice in one
  unit** — four member functions, and one external manager method.
- **An inherited method the derived class also declares is hidden.**
  `posMove()` had to be written `dBaseActor_c::posMove()` because `dEn_c`
  declares its own. Same word count, wrong callee, invisible to raw-word
  comparison.

The rest cost words, so an ordinary diff will find them, but knowing the mapping
saves the sweep:

- **`u8` vs `bool` return on a table lookup is three words** — `bool` adds
  `neg`/`or`/`srwi`.
- **A member read as `lhz` must be declared unsigned.** `s16` emits `lha` and
  costs a word.
- **Use the inline accessor** (`getCenterX()`) rather than open-coding
  `mPos.x + mCenterOffs.x`. The arithmetic is identical; the FPR numbering is
  not. This is the same mechanism as the by-value `mVec3_c` rule above — an
  inlined call is a colouring boundary — so when float registers are numbered
  wrong, look for arithmetic you have open-coded that the original reached
  through an accessor.
- **Free functions with no in-TU caller must NOT be `static`.** Their callers
  live in the RELs, so MWCC dead-strips the whole chain and emits an empty
  object. If a slice compiles to nothing, check this before anything else.
- **Hoist a non-zero constant out of a brace initialiser into a named local.**
  85 words versus 68 here. This is the same defect as "A non-zero constant in a
  brace initialiser can become a static template" above, recurring in a second
  unit with different counts — treat that entry as confirmed, and reach for it
  early whenever a brace initialiser comes out long.

#### When to stop sweeping an axis

The effort numbers above are the useful part of this section. A sweep that
plateaus at a **constant non-zero** diff — ~90 variants stuck at 12, ~15
variants stable at the wrong colouring, 14 probes on argument form, 10 on
comparison shape — is telling you the axis is wrong, not that you have not
enumerated hard enough. The plateau value is stable *because* every variant on
that axis is equivalent to the compiler.

When you hit one, change category rather than continue: declaration order →
declaration *site*; call site → inlined callee body; named local → helper-body
temp; source statement → operand type. That is the same conclusion "Narrowing to
16 bits is a reassociation barrier" reached from a different direction, and it
now has four more instances behind it.

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
`scratchpad/GXTev.c.best` — **which is lost with the scratchpad**, so the shape
below is now the only surviving record of it:

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

## SDK targets (deprioritised — see the register-allocation wall)

**This is not the list to work from.** The game-code target is under "Next
target — and what is known about the rest" near the top of this file; everything
below is kept so the SDK analysis is not lost, not because it is next.

`AXFreeVoice` landed. `AXAcquireVoice` is parked (see blockers), and because a
slice must be one contiguous range, everything after it in `AXAlloc.c`
(`AXSetVoicePriority`, and `__AXAuxInit` in `AXAux.c`) is gated behind it.

### Why game code in `wiimj2d.dol` is the better pool

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
filenames** — 180 in the DOL, **126** of them now outside banked territory.
Filenames are not guesswork.

#### `d_wm_csvdata.cpp` — DONE, all 41 functions banked

Landed in full: 8.475% → **8.624%**, all five binaries verifying. (The banked
slice spans **11,120 B**; the +9,728 figure recorded at the time was the sum of
function sizes, not the span progress counts. Same span-vs-code distinction as
blockmain's 13,232 / 12,604.)
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

**How the whole-TU `.data` constraint was handled here**, because it recurs: the
TU's data runs `0x8031C030`–`0x8031C144` and ends with `__vt__10dCsvData_c` at
`0x8031C138`, so any partial set of literal-emitting functions puts the vtable at
the wrong address. Functions can be **authored** in parallel (each diffs
independently against the target disassembly) but must be **banked in a single
integration pass**. One agent per function, then one integration commit.

**How the first three game-code TUs were chosen** — kept for the selection
method, not as candidates. All three are DONE and banked (11,120 B, 8,384 B and
4,576 B respectively). `fp%` is float instruction density; keeping it low was the
selection rule, since float/virtual-heavy code is where allocation goes wrong:

| Start | Bytes | Fn | TU | fp% | Notes |
|---|---|---|---|---|---|
| `0x800F3550` | 10,740 | 41 | `d_wm_csvdata.cpp` — **DONE** | **0%** | zero indirect calls, 6 ext classes all declared; cleanest object in the DOL |
| `0x800E5510` | 7,380 | 36 | `d_tag_processor.cpp` — **DONE** | 19% | turned out to be 39 fns / 8,384 B |
| `0x8003C9F0` | 4,576 | 46 | `d_a_en_super_bigpile.cpp` — **DONE** | 9% | smallest complete enemy actor; priced the actor-TU pattern |

Still worth avoiding on the same reasoning: `dBc_c` (fp 36%), `dBg_c` (39%),
`daMask_c` (27%), `dWmSpline_c` (45%), `daYoshi_c` (55 external classes).

**Where the remaining work actually is.** The DOL holds **2,950,464 B** of the
6,500,368-B total (45.4%) and is 21.887% done, so **~2.30 MB of undone work is
DOL game code — roughly 40% of everything remaining, and it is workable today**.
The other ~60% is in the four `.rel` modules, which have 0.3–2.3% symbol
coverage and are not workable until a symbol map exists. (An earlier version of
this note claimed ~99% of remaining work was in the RELs. That was wrong, and it
argued against the pool this project's strategy depends on.)

### The SDK list, ranked by expected cost

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

### The `find_targets.py` tool and its limits

This is about the tool, not about what to do next — for that, see "Next target"
near the top.

For SDK code, rank candidates by whether the project's headers already describe
everything the function touches. That predictor has been near-perfect there:
units with complete headers match first try, units needing new struct
reconstruction do not. **It does not transfer to game code** — every function in
an actor TU is a class method, so the tool scores a perfectly workable actor at
0% (verified on `d_a_en_hatena_balloon.cpp`). Header coverage is a *consequence*
of a TU being finished, not a resource available beforehand.

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

- `d_a_en_lkuribo_base.cpp`'s `.data` holds a **fourth** pointer-to-member
  constant `{0, -1, &nonBoyoProc}` at 0x80305104 that nothing references — all
  three `setBoyoFunc` call sites are accounted for by the other three. The
  original had a statement after `fireBoyoProc` that materialises the constant
  without emitting code. Reproduced with a dead local, **commented in place as a
  reconstruction rather than passed off as the original text**; it can sit in any
  function between `fireBoyoProc` and the vtable. This is the same phenomenon as
  the dead-literal rule in the `.sdata2` pooling section: a pooled object you
  cannot account for means there was code there you cannot see.

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

## STATE: `d_a_player_manager.cpp` — assembled, trial-linked, parked in `wip/`

**It links.** 42 of 65 functions byte-exact. The unit is NOT in the build; it
lives in `wip/player_manager/assembled.cpp` with a full resume procedure in
`wip/player_manager/TRIAL_LINK.md`. The tree is green, 5/5.

### Trial-link result — the numbers that matter

| Section | Compiled | Claim | Verdict |
|---|---|---|---|
| `.text` | `0x2AA0` | `0x2A10` | **+0x90 over** |
| `.rodata` | `0x1A0` | `0x1A0` | exact |
| `.sdata2` | `0x38` | `0x38` | exact |
| `.ctors` | `0x4` | `0x4` | exact |
| `.data`/`.bss`/`.sdata`/`.sbss` | — | — | 4–7 under = linker alignment, fine |

**`0x80` of the `0x90` overflow is two functions**: `__dt__Q23EGG8Vector2fFv` and
`__dt__Q23EGG8Vector3fFv`, 16 instructions each, which appear **nowhere** in the
retail symbol map. So the 23 remaining near-misses account for only ~`0x10`
between them — most are same-size-but-different-bytes, which is tractable.

**Do not try removing the two empty destructors from `eggVector.h`.** The
diagnosis is right — they are what makes those types non-trivially destructible —
but it was applied and **failed all five binaries**, including three `.rel`s.
Those types have locals in banked TUs everywhere. Reverted. The fallback (accept
two orphan functions) is the current position.

### What the trial link found that nothing else could

Six undefined symbols, now pinned in `syms.txt`:
`m_instance__14PauseManager_c`, `m_isCourseIn__10dScStage_c`,
`__vt__12dAttention_c`, `__dt__12dAttention_cFv`, `__ct__14dPyEffectMng_cFv`,
`__dt__14dPyEffectMng_cFv`. Every one is a consequence of a *correct* earlier
decision — classes declared while their TUs stay undecompiled, and constructors
deliberately left bodyless to stop MWCC synthesising weak copies.

### Six wrong return types in one class

`fn_8005f4d0` (`void`→`bool`), `addNum()`/`decNum()` (`bool`→`void`),
`changeItemKinopioPlrNo` (`void`→`bool`), `setYoshi` (`void`→`bool`),
`decRest` (`bool`→`int`), `create` (`void`→`bool`). **CFront omits return types
from mangling**, so no symbol comparison can catch one. All six were settled by
compiling the function *both ways* and letting the diff decide. Declared `bool`,
MWCC reserves `r3` and pushes a temp to `r4`; declared `void`, the temp lands in
`r3`.

## Levers and traps learned this session

- **MWCC aligns a `.bss` object to 8 when its SIZE is a multiple of 8**,
  regardless of the type's alignment. A `char[0x18]` still gets 8-aligned. **So a
  gap in `.bss` is not evidence about a class's members** — do not invent a
  `double` to explain one.
- **The base-anchor effect is real but needs two conditions**: the static
  definitions present in the TU *and* enough arrays and uses. Then MWCC emits
  `lis r31, ...bss.0` and reaches everything as offsets. **Write `mRest[i]`
  normally; never pointer arithmetic off `m_playerID`** — that is a workaround for
  isolated compilation and wrong in the assembled file.
- **`extern` is load-bearing on an unreferenced `const` array.** At namespace
  scope a `const` array has internal linkage and is stripped by `-O4`. This
  fixed `lbl_802EF478` here and `l_speed_ratiodt` on the previous unit.
- **`scope:weak` in the symbol map means the linker deduplicates it.** A TU
  flushing a weak copy costs nothing. Check this before treating an extra emitted
  function as a defect — four of six here were not defects.
- **`(vtable size - 8) / 4` = the virtual count.** Cheapest possible check on a
  reconstruction; it has caught extra virtuals twice.
- **A more accurate header is not automatically a better one.** Giving
  `dPyEffect_c` real members instead of a pad made MWCC synthesise a constructor
  that dragged in four weak copies. Declaring `dPyEffect_c()` and
  `dPyEffectMng_c()` *without bodies* fixed it.
- **Declaring a virtual without a body breaks the link** if any TU emits the
  vtable — a slot needs a real address. Tried on `executeLastPlayer`,
  `executeLastAll`, `isItemKinopio`; reverted.
- **`bin/dtk/dtk_splits_wiimj2d.txt`** — official per-file section ranges. Hard
  bracketing. It caught a recorded `.sbss` bound that was `0x28` short. **Check
  it before deriving any bound by subtraction or elimination.**
- **Marking a slice `nonMatching` does NOT park a unit** — the source still
  compiles and links and collides with the filler. Remove the source file.
- **Do not rewrite `slices/wiimj2d.json` or `syms.txt` programmatically.**
  Loading and re-dumping the JSON reformatted 1500 lines; a remove-and-restore
  cycle churned 32 `syms.txt` lines out of position. Insert as text.

