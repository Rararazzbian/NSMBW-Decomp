# Handoff

Working notes for continuing the decompilation work on branch
`claude/game-decompilation-setup-bw30s7`.

> **This file is ~570KB across 400+ sections. Do not read it whole, and do not
> rely on `tail` — the finding you need is usually older than the tail shows.**
> Read [`HANDOFF_INDEX.md`](HANDOFF_INDEX.md) instead: every section with its
> line range, so you can `sed -n '<start>,<end>p' HANDOFF.md` for just the one
> you need. Grep it (`grep -an "<term>" HANDOFF.md`, `-a` because parts of this
> file read as binary to grep) BEFORE designing any experiment. Two rounds were
> spent in one day re-deriving results already recorded here, once on a rule
> whose worked example was the very function being looked at. This file is this
> size so nobody pays twice for the same answer.
> Regenerate the index with `python wip/wm_units/make_handoff_index.py`.

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

### `d_a_wm_grid.cpp` IS LANDED — 5/5 green. It is the project's first REL unit.

Progress moved 11.088% -> **11.096%** (721,304 / 6,500,368). `d_basesNP.rel` is
now 1.043% rather than 0.000%. Every earlier unit landed in the DOL; the REL
landing mechanics were listed as an open unknown and they are now exercised
end to end.

It took **four defects** to get there, and I had earlier called this unit
"10/10, complete". Every one of the four was invisible to the check I was using.

**1. The `.data` bounds started 0x10 too high.** The claim opened at the first
*named* symbol, `sc_ForceList`. But a unit's `.data` opens with the anonymous
string literals that header static points at -- `"F7C0"` and `"W7C0"` from
`dWmLib::sc_ForceList` in `d_wm_lib.hpp`. They are `LOCAL`, so the linker can
neither merge nor drop them; they are as much part of the unit as the vtable.
Proof, and the technique worth copying: compile the **landed, byte-exact**
`d_a_wm_cloud.cpp` and read its object. Its `.data` opens with the same two
strings at offsets 0x00/0x08 and its claim is exact. A known-good unit settles
a bounds question in one step.

**2. The `.text` bounds were wrong at both ends.** The array destructor at
`0x164210` belongs to `d_a_wm_ghost.cpp` below it, not to grid; grid's own is at
`0x164410`, *past* the claimed end -- its `__sinit` references it, which is how
you find it. Real range: `0x164230-0x164430`. MWCC emits the array destructor
for a header static LAST in the object, so a unit's `.text` extends beyond its
`__sinit`. Do not assume `__sinit` closes the unit.

**3. The class declared a virtual the original does not have, and missed one it
does.** `(vtable size - 8) / 4` gave 22 slots against our 23. Comparing
slot-for-slot with landed `d_a_wm_cloud.cpp` showed `processCutsceneCommand`
does not exist on this class at all, and that `fn_2_164370` -- which a peer had
labelled `processCutsceneCommand` -- is an override of `GetActorType`. Both
functions are `li r3, 0; blr`, so nothing in `.text` could tell them apart.

**4. Four functions were assigned to the wrong vtable slots.** This is the one
worth internalising. `create`, `execute`, `draw` and `doDelete` are all
`li r3, 1; blr` -- **identical bytes**. Any permutation of them produces a
byte-identical `.text`. Only the vtable relocations say which is which, and ours
had `doDelete` where the original has `execute`. The fix was reordering the
definitions in the `.cpp` to `create, execute, draw, doDelete`.

### The rule this establishes: `.text` byte-identity does not mean a unit is correct

A unit can be 10/10 in `.text`, with every function verified instruction for
instruction, and still be wrong in four independent ways. `.text` cannot see
section sizes, cannot see vtable shape, and **cannot distinguish two functions
that compile to the same bytes**. I declared four `wm` units complete on
`.text` alone this week. Do not repeat that.

### How to find the defect when `--verify-bin` fails

This localised all of the above and is much faster than re-reading source:

1. Compare section SIZES of `bin/<mod>.rel` against `original/<mod>.rel`.
   Grid's `.text` was 0x10 over -- that alone identified a placed function that
   should not have been placed.
2. Compare section CONTENTS byte by byte. When all six sections match and the
   md5 still differs, the defect is in the REL metadata.
3. Decode the relocation table. Grid's final failure was **three bytes**, a
   permutation of `0x40/0x50/0x60` across three 8-byte relocation entries --
   the vtable slots for functions with identical bodies. Nothing else would have
   found it.

### Seeding the `.rodata` constant pool: use `DECL_WEAK`

Grid's pool needed a leading `0.0f` that no placed function references. All
three forms behave differently and only one works:

| form | result |
|---|---|
| `static void f() { static const float U[] = {0.0f}; }` | dropped outright, pool entry lost |
| `void f() { ... }` (plain global) | placed -- `.text` overflows the claim by 0x10 |
| `DECL_WEAK void f() { ... }` | **correct** -- code deadstripped, pool entry kept |

`d_a_wm_cloud.cpp` uses a plain global `DUMMY_UNUSED()` because its own `.text`
claim has room for it. Grid's does not. Match the idiom to the unit, and note
that a global also collides: `DUMMY_UNUSED` already exists in `d_a_wm_cloud.cpp`
in the same module.

### Tooling: `progress.py --verify-obj` already did half of this

Before writing anything new, know that `progress.py --verify-obj` checks every
slice section's length against the object. It found grid's `.text` overflow the
moment the unit was in the slice file. **It only warns, and `.text` over-claim
is normal and expected**, so the real signal sits in a stream of benign
warnings -- which is why this went unnoticed.

`wip/wm_units/check_sections.py` complements it rather than replacing it:

```
python wip/wm_units/check_sections.py <draft.cpp|draft.o> <module> '<slice JSON>'
python wip/wm_units/check_sections.py <draft.o> <module> --dump
```

- works on a **draft**, before the unit is in the slice file and can build
- treats `.data`/`.rodata`/`.bss`/`.ctors` over-claim as **fatal**, `.text` as expected
- dumps that section's symbol table on mismatch, which is what localises the defect

Validated against landed `d_a_wm_cloud.cpp`: all five sections `SECTIONS CLEAN`.

**It still does not check function order or vtable slot assignment.** Both bit
this unit. Until it does, read the vtable relocations by hand before landing.

### `d_a_wm_tower.cpp` is also LANDED

5/5 green. Progress **11.114%**; `d_basesNP.rel` 1.103%. Two REL units now.

Tower needed the identical bounds correction and then failed on **one byte**:

```
.rodata 0x9320   orig 42 c8 00 00 = 100.0f
                 ours 42 f0 00 00 = 120.0f
```

`setClipSphere()` was written `mClipSphere.set(mPos, 120.0f)`; the original is
`100.0f`. **`create` matched byte-for-byte with the wrong number in it**, because
the constant lives in the `.rodata` pool and the instruction only carries a
relocation to it. `verify_anon.py` normalises relocation symbol names, so a
wrong float is completely invisible to it.

Add that to the list of things `.text` cannot see: section sizes, vtable shape,
which of two identical-bodied functions occupies a slot, and **the value of any
pooled constant**. After a unit is 11/11 with clean sections, the remaining
failure modes all live in `.rodata` and the relocation tables.

### Corrections to what I wrote about tower an hour ago

Both figures I put in this file were wrong, and both came from measuring a
stale object:

- I said tower's vtable was `0x70` against a target `0x78`, "two virtuals too
  few". It is `0x78` and always was. The `0x70` came from `tower_relflags.o`,
  built from an older draft.
- I said the target has 30 slots and we declare 28. `(0x78 - 8) / 4` is **28**,
  not 30.

The real `.data` surplus was a weak `__vt__13dWmObjActor_c` (0x78) -- see below.

### Weak symbols in `.data`, not just `.text`

`check_sections.py` originally called any `.data` over-claim a real defect. That
is wrong. Tower emits a **weak** `__vt__13dWmObjActor_c`; so does landed
`d_a_wm_cloud.cpp`. Exactly one copy gets placed, so the object is 0x78 "over"
either way and it is not a defect.

The tool now compares the claim against the extent of the **strong** (GLOBAL and
LOCAL) symbols, which are always placed, and treats a purely-weak surplus as ok.

One trap found while doing it: **that rule must not be applied to `.text`.**
Weak functions are interleaved *between* strong ones there, so
`max(value + size)` over strong symbols sweeps up every weak gap below the last
one. Applied to `.text` it failed the landed, 5/5-verified `d_a_wm_grid.cpp`.
It is restricted to the STRICT sections, and both landed units are regression
cases: `d_a_wm_grid.o` and `d_a_wm_cloud.o` must both report `SECTIONS CLEAN`.

### The `wm` bounds error is systematic — assume it on every unit in this family

Grid and tower had the *same* two bounds errors, and the pattern is mechanical:

- `.data` opens **0x10 before** the first named symbol, on the two anonymous
  strings from `dWmLib::sc_ForceList`.
- `.text` starts **after** the previous unit's array destructor and ends
  **after** its own, which sits past `__sinit`.

```
              claimed                    actual
grid    .text 0x164210-0x164404    0x164230-0x164430
        .data 0x44c90-0x44d20      0x44c80-0x44d20
tower   .text 0x1856f0-0x185b44    0x185710-0x185b70
        .data 0x48090-0x48158      0x48080-0x48158
```

Codex's round-14 `.text` figures were right for both and I went with Gemini's.
Assume `d_a_wm_smallcloud.cpp` and `d_a_wm_kinoko_1up.cpp` have it too, and
re-derive their bounds from the split objects before touching their source.
Neither has had a section check.

### `d_a_wm_smallcloud.cpp` and `d_a_wm_kinoko_1up.cpp` — bounds now derived, both still short

Both had never had a section check. Bounds derived from the split objects the
same way grid and tower were, and these are now settled:

```
smallcloud  .text   0x1797e0-0x179ff0      kinoko_1up  .text   0x16b0f0-0x16b2d0
            .ctors  0x430-0x434                        .ctors  0x3fc-0x400
            .rodata 0x8fa8-0x8fd8                       .data   0x457b8-0x458a0
            .data   0x47258-0x47450
            .bss    0x102a0-0x102c8
```

kinoko's `.text` claim was already right -- the only one in this family that
was. smallcloud's was not.

To find a unit's `.ctors` slot, disassemble its `__sinit` split object: dtk
emits the `.ctors` word alongside it with its address in the header comment.
That is faster than counting entries.

**`d_a_wm_kinoko_1up.cpp` — 8/9, and it had grid's vtable defect.** The target
vtable lists `0x16B1E0` **before** `0x16B1D0`; our class declared them the other
way round. Both are a bare `blr`, so `.text` was 8/9 either way. Fixed by
swapping the two *declarations* while leaving the *definitions* in place --
declaration order sets vtable slots, definition order sets `.text` addresses,
and here the original has them deliberately opposite. Verified: vtable order now
matches and `.text` is unchanged at 8/9.

Still open on kinoko:
- `vf84` at `0x16b1f0` is **5 of 7 differing**.
- `.data` is **0x40 too large**. The original stores one string,
  `"cobKinokoAppear"` at `0x457F8`, and points at it twice from `0x45808`, plus
  one pointer at `0x45810` to `0x457A8` outside the unit. Our draft emits six
  separate strings for `smc_animResNames`/`smc_modelResName`. The resource-name
  tables are the wrong shape, and that is almost certainly the same defect as
  `vf84`, which is what fills them.
- Its vtable is `0x88` (32 slots) and ours already matches. `create`, `execute`,
  `draw` and `doDelete` are **inherited** from `daWmKinokoBase_c`, a different
  TU at `0x16B470+` -- do not try to author them here.

**`d_a_wm_smallcloud.cpp` — 14/16, not 15/16.** With the corrected bounds:

```
0x00179bb0  createModel   101 instrs   95 differing
0x00179f40  __sinit        33 instrs    3 differing
```

`.rodata`, `.bss` and `.ctors` are exact. `.data` is **0x10 short**, and the
shortfall is in the same place as the failing function: the original has a
single `0x34` object at unit offset `0x7c` where our draft emits a `0xc` table
plus several loose strings. Our vtable block is right (`0x78` own + `0x78` weak
`dWmObjActor` + `0x18` weak `anmChr` = `0x108`, matching the target's merged
label), it just sits `0x10` early. Fix the resource table shape and
`createModel` and `.data` should close together.

Neither unit is close to landing, and neither is blocked on the register wall --
both are blocked on getting a data table's shape right, which is ordinary work.

### smallcloud update: `.data` is now exact, and a bounds figure of mine was circular

**`.data` is clean.** The shortfall was `setPosFromCourseNode`'s `nodeNames`,
which the draft itself flagged as placeholders: four 5-byte strings where the
original has `"MoveCloud01"`, `"MoveCloud02"`, `"MoveCloud03"`, `"CloudLarge"`
(0xC/0xC/0xC/0xB at `0x47308`/`0x47314`/`0x47320`/`0x4732C`). That is exactly the
0x10. All five sections now report `SECTIONS CLEAN`.

The `createModel` data was never wrong: our `"CS_W6aCloud"`, the 4-pointer
table, `"CS_W%d"` and `"g3d/model.brres"` already sit at unit offsets
`0x7c`/`0x88`/`0x98`/`0xa0`, matching the target. dtk merges them into one
`0x34` label only because just the first is referenced -- the batch note
guessing at "one larger aggregate" was reading a dtk artefact, not a real
difference.

**Correction: the `.rodata` bound I committed an hour ago was derived
circularly.** I took `0x8fa8-0x8fd0` because 0x28 was our object's size. The
target's `__sinit` loads `lbl_2_rodata_8FA8` and reads the vec3 at `0x24/0x28/0x2c`,
i.e. up to `0x8FD8`. The real bound is **`0x8fa8-0x8fd8` (0x30)**, and our pool
is **8 bytes short at the front**:

```
        target                          draft
+0x00   00080000                        (missing)
+0x04   00080000                        (missing)
+0x08   43c80000  400.0                 +0x00
...                                     ...
+0x24   45070000  2160.0                +0x1c   <- __sinit reads here
```

Word for word ours is the target's words 2..11. That single fact explains
`__sinit`'s 3 differing instructions completely -- they are the three `lfs`
offsets, 8 low. Same shape as grid, where a missing leading pool word moved
every offset.

**Never derive a section bound from your own object's size.** Take it from the
target: the next unit's first symbol, or an offset the target's own code reads.

What emits the two `0x00080000` words is unsettled. `0x80000` is a plausible
frame-heap size, but I tested `createFrmHeap(0x80000, ...)` against the draft's
`-1` and it changed nothing -- correctly, since that would be an immediate via
`lis`, not a pool entry. A pool word of `0x00080000` is a denormal as a float,
so it is unlikely to be a float literal. They are almost certainly referenced by
`createModel`, which is still **95 of 101 differing** -- the one genuinely
unfinished function in this unit, and the only thing between smallcloud and a
landing now that its sections are clean.

### smallcloud is 15/16 — and `createModel` was already solved yesterday, in a directory I did not check

**Correction to everything I wrote about `createModel` today.** I reported it as
95 of 101 differing and "the one genuinely unfinished function in this unit".
It was closed on 2026-08-14 in commit `f7c9744`, in
`wip/wm_smallcloud/scratch/createmodel/`, documented in
`wip/wm_smallcloud/CREATEMODEL.md`. I was reading
`wip/wm_smallcloud/scratch/merged/`, where the fix had never been carried over,
and I took that file's own stale comment at face value.

Independently verified by recompiling the combined source myself:

```
0x00179bb0 fn_2_179BB0  101  MATCH  <- createModel__16daWmSmallCloud_cFv
15/16 byte-identical modulo symbol names
.data  0x1f8  0x1f8  ok
```

`createModel` and `setPosFromCourseNode` were fixed in **two different working
copies by two different sessions**, and neither knew about the other. `.data`
is exact only with both applied. **Before declaring a function unfinished,
`git log --all` for it and check every sibling directory under its `wip/`
tree** — this unit has five (`batchA`, `batchB`, `createmodel`, `merged`,
`merge_lead`) and the best version of a given function is not always in the one
named `merged`.

**My `0x00080000` hypothesis is refuted.** I claimed those two words were
"almost certainly referenced by `createModel`". They are not: `createModel` is a
full 101/101 MATCH with our pool lacking them, and so are the other fourteen
matching functions. Whatever emits them is used earlier in the TU than anything
we currently emit, since pool order is order of first use.

The unit's ONLY remaining blocker is that one pool gap:

```
.rodata 0x8fa8-0x8fd8 (0x30)   our object 0x28   UNDER 0x8
__sinit  3 of 33 differing     -- the three lfs offsets, all 8 low
```

These are one defect. The fix pattern is grid's: a `DECL_WEAK` global function
holding a `static const` array, which is deadstripped while its pool entries
survive. What produces two words of `0x00080000` is still unknown -- as a float
that is a denormal, so a plain float literal is implausible; a `double` may
explain the pair as one 8-byte entry.

Also still needed to land: the `daWmMap_c::GetNodePos(const char*, mVec3_c&)`
overload that `MERGED.md` proposed, evidenced by `setPosFromCourseNode`'s callee
`GetNodePos__9daWmMap_cFPCcR7mVec3_c`. It is a shared-header change and has not
been applied.

### smallcloud is 16/16 in `.text` and STILL DOES NOT LINK — the pool words are `mData`

A `.text` check, a section check and a vtable check can all pass on a unit that
will not link. Trying to land it:

```
Error: Symbol mData__33sGlobalData_c<16daWmSmallCloud_c> not found!   (x5)
Error: Symbol GetNodePos__9daWmMap_cFPCcR7mVec3_c not found!
```

`GetNodePos(const char*, mVec3_c&)` is solved: it lives in the **DOL** at
`0x801007F0` (`bin/dtk/wiimj2d_symbols.txt`) and needs a `syms.txt` entry
alongside the existing `GetNodePos__9daWmMap_cFlR7mVec3_c=0x801007D0`. The
header overload is proved by the target's own tail call.

**The two `0x00080000` words are `mData`.** `GlobalData_t` is
`{s16 mBgmValueW5[2]; s16 mBgmValue[2];}` and `00 08 00 00 00 08 00 00`
decodes big-endian as `{8,0},{8,0}`. So they are the specialization
`sGlobalData_c<daWmSmallCloud_c>::mData`, which the header declares, `create()`
uses via the `GLOBAL_DATA` macro, and **nothing defines**. A `DECL_WEAK` dummy
holding `static const u32 UNUSED[] = {0x80000, 0x80000}` reproduces the bytes
and gets 16/16 with clean sections -- but it is the right bytes via the wrong
construct, and the symbol stays unresolved.

Two obvious fixes, both tried, both FAILED -- do not repeat:

| form | result |
|---|---|
| specialization defined before the functions | sections clean, but `create` **37 of 62 differing** -- MWCC constant-folds it |
| specialization defined after the functions | 14/16 AND `.rodata` wrong -- emitted after the pool instead of before |

The folding is the crux. The target LOADS at runtime where ours folds:

```
target: addi r5, r4, lbl_2_rodata_8FA8@l    ; address materialised
        lha  r4, lbl_2_rodata_8FA8@l(r4)    ; loads mBgmValueW5[0]
        subi r4, r4, 0x1
draft:  li   r4, 0x7                        ; FOLDED 8-1
        li   r0, 0x0                        ; FOLDED
```

So `mData` must be emitted FIRST in `.rodata`, defined in this TU (the target's
`__sinit` uses it as the base for the float pool at `+0x24`, which is only
possible same-TU), and NOT visible as a constant at `create`'s point of use.

**The mechanism is `-ipa file`**, which is in `d_basesNP`'s
`defaultCompilerFlags`. File-scope interprocedural analysis gives MWCC whole-TU
visibility, so it folds a `const` **integer** scalar read wherever the definition
sits. Four placements were tested -- top of file, end of file, forward
declaration plus definition at either end, and reads through a local
`const s16 *` -- and **all four fold identically**, 37 of 62 differing.

Why no landed unit has hit this: `d_a_wm_cloud.cpp` only ever takes `mData`'s
ADDRESS (`bgmSync->m_18 = ...`), never reads a scalar field, and indexes
`mGroupNodeRadii[i]` at runtime. `d_a_wm_dokan_route.cpp` does read a scalar
field, but a **float** -- PowerPC has no float-immediate load, so even a folded
float still needs an `lfs` from some address and the codegen looks unchanged.
Only an **integer** field gives MWCC a cheaper alternative (`li`), and
smallcloud's `mBgmValueW5[0] - 1` is exactly that.

**A data-only sibling TU does NOT solve it.** Moving the specialization into its
own TU defeats the fold -- `create` matches -- but then `__sinit` breaks: it
falls back to our own pool base with offsets `0x1c/0x20/0x24` where the target
uses `0x24/0x28/0x2c` off `mData`. That is 15/16 and `.rodata` 8 short, i.e. the
same total defect count, just moved. The two requirements are in direct tension:

```
mData IN this TU   -> __sinit right, .rodata right, create FOLDS      (15/16)
mData in a sibling -> create right,  __sinit wrong, .rodata 8 short   (15/16)
target             -> BOTH right: unfolded reads AND mData as pool base
```

The target proves `mData` is same-TU (nothing else explains `__sinit`'s base)
AND unfolded. So the answer is a source form that is same-TU but not a
compile-time constant at the point of use. Untested: giving `GlobalData_t` a
user-declared constructor, which would make it non-constant-initialisable and
therefore unfoldable -- note the TU already has a `__sinit`, so dynamic
initialisation is not automatically disqualifying.

Also settled: `GlobalData_t` is exactly `{s16 mBgmValueW5[2]; s16 mBgmValue[2];}`
with values `{8,0},{8,0}`. The speculative `u8 mUnofficialPad[8]` was wrong and
is gone.

### `d_a_wm_ghost.cpp` is 12/13 with clean sections — and its `.data` bound was the RIGHT SIZE and the WRONG SPAN

The experimental local model's draft is genuinely good: 12 of 13 functions
byte-identical, and an independent audit confirms the vtable slot-for-slot.

Its bounds claim was `.data 0x44a9c-0x44cb4`. The real span is
**`0x44a68-0x44c80`**. Both are `0x218` bytes. The claim started at
`g_profile_WM_GHOST` and ran to `g_profile_WM_GRID` -- it silently swapped
ghost's leading string/`sc_ForceList` block for **grid's**, which I landed this
morning and can confirm independently: grid's own strings start at `0x44c80`.

**A size-only section check passes this.** `check_sections.py` reports
`0x218 == 0x218  ok` on the wrong span. Only cross-referencing the claim's
START ADDRESS against the target's actual symbol layout catches it. Add that
step before landing anything: the first thing in a `wm` unit's `.data` is always
the two anonymous `sc_ForceList` strings, so if the claim begins at the profile,
it is 0x34 too high.

Corrected and verified bounds, all five sections `SECTIONS CLEAN`:

```
.text   0x163620-0x164230      .data   0x44a68-0x44c80
.ctors  0x3e0-0x3e4            .bss    0xfdc0-0xfdd0
.rodata 0x8880-0x88b8
```

`.text` needed no correction -- ghost is the one unit in this family whose
`.text` claim was right, because its array destructor at `0x164210` is inside
its own range, which is also why grid's claim wrongly reached down to `0x164210`.

Remaining: `createModel` at `0x163940`, **6 of 77 differing**. Seven variants
swept and it did not move -- a documented negative:

| variant | result |
|---|---|
| merged `resMdl` declare+init, `int i` into the `for` | 6, byte-identical |
| `resAnmNames` moved after `mModel.create()` (cloud's order) | 6, byte-identical |
| named `ResAnmChr` local instead of inline temporary | 6, byte-identical |
| split declare-then-assign `resMdl` | 6, byte-identical |
| `const ResFile` / `const ResMdl` | 6, byte-identical |
| `while` loop instead of `for` | 6, byte-identical |
| inline `GetResAnmChr(...)` as call argument | 6, byte-identical |

Every one produced **literally identical bytes**, not merely the same count.
`-O4` normalises all of these surface rewrites to one internal representation
before stack slots are assigned.

The defect is precise: identical registers (`_savegpr_24`, r24-r31), identical
77-instruction sequence, identical `-0x40(r1)` frame. The only difference is a
**3-way cyclic rotation** of three stack-slot immediates holding by-value
argument temporaries -- target `[0x8, 0xc, 0x10]` in order of first appearance,
draft `[0x10, 0x8, 0xc]`.

**This is a third wall, distinct from the two already known.** Not register
allocation, and not the stack-slot-order problem that declaration order fixes
(smallcloud's `char arcName[8]` -> `[6]`). Do not spend more variants on the
declaration-order family; it is ruled out. The one untested structural
difference (cloud and smallcloud use a **member** `mResFile` where ghost's
target uses a **local** `resFile`) is also REFUTED: making it a member left the
6 rotation lines byte-identical, and the 3 lines that did change confirmed the
target really is stack-relative (`r1+0x14`).

### The mechanism, confirmed against landed retail bytes

MWCC allocates these by-value argument temporaries in **groups**, and the groups
come out in REVERSE order while the slots WITHIN a group ascend. Established by
a synthetic N=4 probe and then independently confirmed against a landed,
byte-exact function of the same shape --
`d_a_wm_dokan_route.cpp::createModel`, which has two by-value consumers per
iteration:

```
dokan_route (LANDED, byte-exact):
  resMdl-outer   0x8    <- lowest
  group 1: resAnmChr 0x14, resMdl-copy1 0x18
  group 2: resAnmTexSrt 0xc, resMdl-copy2 0x10   <- later group, LOWER addresses
```

The decisive observation: in **both landed units and in ghost's target**, the
pre-loop `resMdl` consumer sits at the LOWEST address with everything after it
ascending. Our draft puts it HIGHEST. So the loop is not the problem -- the
question is why our pre-loop consumer is grouped as though it came last.

Forcing the trip count to 1 makes the registers identical to landed
`d_a_wm_cloud.cpp` (`r27`-`r31` all match) and the rotation is STILL wrong, so
loop count is not the lever either.

**Both remaining hypotheses are now refuted, and this is a WALL. Stop grinding it.**

| lever | result |
|---|---|
| array size (`ANIM_COUNT` 1 vs 6, destructive) | rotation byte-identical; registers moved, slots did not |
| `mModel.create()` moved after the loop | slots become exactly `[0x8, 0xc, 0x10]` -- **and 39 instructions differ** |
| outer consumer folded into the loop's `i == 0` branch | same rotated shape, NOT the single ascending group the rule predicts |
| member vs local `resFile` | rotation byte-identical (previous round) |
| decl order / loop shape / naming / `const` (7 variants) | literally identical bytes (round one) |

**A trap worth remembering from this:** moving the call after the loop reproduces
the target's exact slot triple *for the wrong reason* -- the call then sits in
the wrong place in the instruction stream and wrecks 39 instructions. **The
right numbers are not the right answer.** Always re-check the whole function,
not the metric you were steering on.

One methodological catch worth copying: the `i == 0` probe was first run at
`ANIM_COUNT = 1`, where the optimizer proved the guard always true and erased
the branch, silently reproducing the unmodified baseline. It had to be re-run
with a real trip count for the guard to be genuine control flow. **A "no change"
result is only meaningful once you have confirmed your change survived
optimisation.**

The rule explains the LOOP's two-item ordering in every variant. What remains
unexplained is why the single pre-loop consumer sits at the group's LOW end in
landed cloud, landed dokan_route and ghost's target, but at the HIGH end in
every draft that keeps the call before the loop. That is a fourth wall. Ghost
stays at 12/13 with all sections and the vtable clean; it is the closest
unlanded unit in the family.



### `d_a_wm_kinoko_1up.cpp` is COMPLETE and NOT LANDABLE — it depends on an un-decompiled TU

All three checks pass, verified independently:

```
9/9 byte-identical modulo symbol names
.text 0x1e0/0x310 over (weak, expected)   .ctors ok   .data 0xe8/0xe8 ok
SECTIONS CLEAN            VTABLE CLEAN
```

Bounds: `.text 0x16b0f0-0x16b2d0`, `.ctors 0x3fc-0x400`,
`.data 0x457b8-0x458a0`, `.bss 0xfe70-0xfe80`. No `.rodata`.

**It still fails to link**, and the reason is not a defect in the unit:

```
Error: Symbol __ct__16daWmKinokoBase_cFv not found!
Error: Symbol __dt__16daWmKinokoBase_cFv not found!
Error: Symbol lbl_2_data_458A0 not found!
```

`daWmKinokoBase_c` is a **different, un-decompiled TU** at `0x16B300+`, and this
unit inherits `create`/`execute`/`draw`/`doDelete` from it and calls its
constructor and destructor. `d_a_wm_kinoko_1up.cpp` therefore cannot land before
`d_a_wm_kinoko_base.cpp` does, or before those two symbols get `syms.txt`
entries. **This is the first unit blocked on another unit rather than on its own
correctness**, and it is worth checking for before starting any leaf actor with
a non-trivial base: if the target vtable's slots point outside the unit's
`.text`, the base lives elsewhere. `check_vtable.py` reports exactly that as
`skip (inherited from another TU at 0x...)` -- five slots here.

**The `extern "C" const char lbl_2_data_XXXX[]` idiom does NOT work.** Both
labels exist in `bin/dtk/d_basesNP_symbols.txt`, but `lbl_2_data_458A0` does not
resolve at link. No landed unit uses this idiom and it should not be adopted:
those two `"cobKinoko1up"` strings are owned by the sibling TUs before and after
this one, and referencing them by dtk label is a bridge, not a decompilation.

### A real MWCC finding from this unit: file-scope vs function-local statics change scheduling

`vf84` sat at 5 of 7 differing through six source permutations -- swapping
declaration order, swapping assignment order, the comma operator, explicit
intermediate locals. None moved it.

**Moving the two tables from function-local `static` to file scope made it
byte-exact immediately.** As function-local statics, `-O4,p` hoisted the
address computation of the `lwz`-needing variable ahead of the `addi`-needing
one regardless of source order; at file scope it emits in strict program order.
Worth trying whenever a small function's instruction SCHEDULE is wrong but its
instruction set is right.

Also settled: `const char *const` (const pointer) pushes the object into
`.rodata` and breaks `__sinit`; a plain `const char *` keeps it in `.data`.

### Verified bounds kits — three units ready to author

Derived by a scouting pass and independently re-validated with
`wip/wm_units/check_bounds.py`. All three report `BOUNDS PLAUSIBLE`.

```
WM_CASTLE      .text 0x15ecc0-0x15fbe0   .ctors 0x3c8-0x3cc
               .rodata 0x86e8-0x8728     .data 0x43fd0-0x441b0
               .bss 0xfd48-0xfd60
               20 fns, ~3768 code bytes, vtable 0x108 (64 slots), no external base

WM_COURSE      .text 0x1604a0-0x161940   .ctors 0x3d0-0x3d4
               .rodata 0x87b0-0x87f0     .data 0x44400-0x44590
               .bss 0xfd70-0xfd80
               22 fns, ~5160 code bytes, vtable 0xf0 (58 slots), no external base

WM_SANDPILLAR  .text 0x177690-0x179380   .ctors 0x428-0x42c
               .rodata 0x8ef8-0x8fa8     .data 0x46be0-0x47180
               .bss 0x10040-0x10290
               66 fns, ~7400 code bytes, main class PLUS a nested sStateID_c helper
```

Castle is the easiest: both `.text` edges are pinned by LANDED neighbours -- it
starts exactly where `d_a_wm_cannon.cpp` ends and ends exactly where
`d_a_wm_cloud.cpp` begins.

Two cautions carried from the scout, neither resolved:

- Course's `.rodata` end has no landed neighbour beyond it. Internally
  consistent, one step less firmly pinned than the rest.
- Sandpillar's vtable has one slot resolving to `fn_2_15ABC0`, a trivial 2-instruction
  stub OUTSIDE its `.text` and before the whole `wm` cluster. Most likely
  identical-code folding of a one-line override shared with an already-compiled
  TU rather than a real cross-TU dependency like kinoko_1up's -- but check
  before landing, because which TU emits the folded copy decides the bytes.

**Method note worth reusing:** where dtk could split a section cleanly, the split
file's own header comment (`# 0xSTART..0xEND | size:`) is ground truth and needs
no arithmetic. Most of this family is merged into a few large blobs where that
is unavailable, and there the `profile - 0x34` rule is only a heuristic --
`WM_CLOUD`'s real `.data` start sits 0xCC before what it would predict.

The rest of the family is NOT bounded. `WM_BUBBLE` is inside a 0x9670-byte blob
shared with five unrelated actors; fourteen more (`HANACHAN`, `ISLAND`, `ITEM`,
`KILLER`, `KINOKO_RED`, `KINOKO_STAR`, `KINOPIO`, `KOOPAJR`, `KOOPASHIP`,
`MANTA`, `MAP`, `NOTE`, ...) share one 0x2078-byte blob. Each needs the same
per-unit disassembly-and-reference sweep; none of it is derivable from
filenames.

### `d_a_wm_course.cpp` — 12/23 first pass, in `wip/wm_units/agent_course/`

A large novel unit: 22 authored functions plus `__sinit` and the array
destructor, ~5.1KB, three functions of 224/233/136 instructions. Bounds
validated by `check_bounds.py`. `.data 0x190` and `.bss 0x10` are EXACT;
`.ctors` exact; `.text` and `.rodata` are UNDER purely because several bodies
are still stubs -- symptoms, not independent defects.

`check_vtable.py` reports **VTABLE CLEAN** with all 28 real slots correctly
assigned, and it auto-trimmed the 30 trailing zero words of the merged label
without a manual workaround, which is the fix from earlier today working as
intended. The 3 `unverifiable` lines are the still-differing functions, not
misassignments.

Confirmed byte-exact: `classInit`, the destructor, `draw`, `doDelete`,
`calcModel`, `updateState`, `setMatClrAnm`, `searchOpenNeighbor`,
`getMatClrFrame`, `updateSpecialWorld`(one of two), `vf78`, the array
destructor.

Class layout established from codegen: `dHeapAllocator_c mAllocator@0x188`,
`nw4r::g3d::ResFile mResFile@0x1a4` (a MEMBER, like cloud and dokan_route,
straight from `stw r3, 0x1a4(r30)`), `m3d::smdl_c mModel@0x1a8`,
`m3d::anmMatClr_c mMatClrAnim[3]@0x1b4` (stride 0x2c, from the ctor's
`__construct_array`), `mCurrentIndex@0x238`, `mState@0x240`, `mOpenState@0x244`.

**Two things must be resolved before this unit is landable, neither of which is
a matching problem:**

1. RESOLVED. The field at `this+4` is **`fBase_c::mParam`**. `execute()` reads
   it as a full unmasked word for the `c_START_ID` comparison; every other use
   masks the low byte, which is `ACTOR_PARAM(CourseNo)` -- `dWmObjActor_c`
   already declares `ACTOR_PARAM_CONFIG(CourseNo, 0, 8)` and landed
   `d_a_wm_smallcloud.cpp` already uses it that way. Use
   `ACTOR_PARAM(CourseNo)` for the object's own, `ACTOR_PARAM_LOCAL(obj->mParam,
   CourseNo)` for searched objects, and bare `mParam` unmasked for the
   `c_START_ID` compare. Per-call-site `cmpwi` vs `cmplwi` signedness had to be
   confirmed by testing each site. No cast survives.
2. Five `dWmLib` free functions the target calls do not exist in
   `include/game/bases/d_wm_lib.hpp`: `isSpecialWorld`, `IsAllComplete`,
   `isKoopaShipOnCurrentWorld`, `isSpecialWorldCourseOpen`,
   `SearchMapObjFromCsvIndex`. Being proven in a shadow header. **Apply them one
   at a time** with five-binary verification between each -- a batch that fails
   says nothing about which member is wrong.

Now **15/23**. `execute`, the constructor and `isWorld2SpecialType` all closed.
Cheapest remaining: `updateHelpFade` (1 real differing instruction), then
`openNeighbors` (57).

`__sinit`'s 3 differing is **entangled, not isolated**: the missing leading pool
content is the 8-byte bias-double for a runtime int-to-float conversion
(`xoris`+`lfd`+`fsubs`) of the literal `60`, because the target converts an int
at runtime where the draft's stub bodies pass a folded `60.0f`. `__sinit`,
`.rodata`, `openNeighbors` and `updateOpenAnim` all close together.

**MWCC lever worth reusing:** `return a == b;` compiles branchlessly via the
`cntlzw`/`srwi` trick; nested `if`/`return` statements produce branch-based
early returns (`bnelr`). Reach for this when a residual is "right logic, wrong
branch structure".

**Tool correction:** `verify_anon.py`'s `differing vs <name>` was only the
closest remaining draft function BY SIZE, and I relayed those labels to an agent
as if they were identities -- `fn_2_161840` was reported as
`processCutsceneCommand` while actually being `isWorld2SpecialType`. It now
prints `differing vs ~<name>`; only a name after `MATCH <-` is a real pairing.
`__sinit` at 3-of-33 is the same signature seen twice today and both times it
was the three `lfs` offsets from a `.rodata` pool short at the FRONT -- check
for a real undefined object (smallcloud's `mData`) before reaching for a
`DECL_WEAK` seed (grid's fix).

### `d_a_wm_kinoko_base.cpp` — 15/17, and `daWmKinokoBase_c` sizeof is 0x290

Authored from scratch this session in `wip/wm_units/agent_kinoko_base/`. Bounds,
all evidence-backed, `SECTIONS CLEAN` and `VTABLE CLEAN`:

```
.text 0x16b2d0-0x16bda0   .ctors 0x400-0x404   .rodata 0x8ac8-0x8af0
.data 0x458b0-0x45a70     .bss   0xfe80-0xfe90
```

**The sizeof is 0x290. Both 0x2B0 and 0x284 were wrong, and I recorded 0x284 as
settled.** The argument is structural rather than arithmetic: `createModel` is
`daWmKinokoBase_c`'s OWN method -- called non-virtually by `create()` -- and it
reads `0x280`/`0x288`/`0x28c` through `this`. A base method cannot see derived
members through `this`, so `mAnimResFile`, `mCutsceneTimer`, `mAnimResNames` and
`mModelResName` belong to the BASE. The component sum then lands exactly on
0x290 with no gap, and the leaf's single `mFlag` gives 0x294, matching
`daWmKinoko1up_c`'s own `classInit` literal.

**Consequence for the finished leaf:** `wip/wm_kinoko_1up/complete/d_a_wm_kinoko_1up.cpp`
declares `mAnimResNames`/`mModelResName` as its OWN members. Once this base
lands owning them, the leaf will have duplicates at wrong offsets. The leaf must
be changed to inherit them before it can land.

Two functions remain:

- **`createModel` at 10 of 161. It is the SAME WALL as `d_a_wm_ghost.cpp`.**
  The "missing 16-byte object" reading was wrong and has been retracted on
  evidence: both frames are `stwu r1, -0x60(r1)`, so there is no room for a
  missing local. All three `.create()` sites pass literal zero for the
  out-param, killing that lead too. What is actually happening is a PERMUTATION
  of five already-present by-value temporaries across the same offset set
  `{0x8, 0xc, 0x10, 0x14, 0x18}` -- target puts the first `resMdl` copy at
  `+0x8`, the draft at `+0x18`.

  That is the identical signature to ghost: **the target's FIRST by-value
  consumer sits LOWEST, every draft puts it HIGHEST.** Two independent units now
  exhibit it, which makes it far more tractable than one. See the ghost section
  for the group-allocation mechanism and the eight levers already ruled out.

  The `getRes()` lead is RULED OUT from the target bytes: both calls are a `bl`
  followed by a single `stw`, with no hidden-pointer copy -- `ResFile` fits in
  one register and neither call materialises a by-value temporary.

### The refined rule: the reversal is scoped to the LOOP

This unit has a POST-loop by-value consumer (`mChrBlend.create`), which landed
`d_a_wm_dokan_route.cpp` does not. That extra data point separates two readings
the earlier work could not:

```
                        target        draft
PRE   before loop        0x8          0x18
LOOP-1  1st in body      0x14, 0x18   0x10, 0x14
LOOP-2  2nd in body      0x10         0xc
POST  after loop         0xc          0x8
```

**Non-loop groups (PRE, POST) are numbered in plain FORWARD appearance order and
anchor the low end (`PRE < POST`). Only the loop's OWN per-iteration groups
reverse relative to each other, in the block above them (`LOOP-2 < LOOP-1`).**

The draft's loop-internal ordering already matches. What is wrong is that MWCC
reverses EVERYTHING for our source, as if there were no loop/non-loop
distinction at all. No source-level lever has been found that reproduces the
distinction.

**CLOSED AS A WALL after a deliberate attempt to break the model.** Rather than
test an eleventh lever, the model itself was doubted and re-derived from raw
bytes: (a) exactly five `0xNN(r1)` slots exist, each traced to the instruction
that writes and the one that reads it -- every group is a genuine re-spill of a
value already live in a callee-saved register; (b) the loop is an ordinary
post-test shape with ONE body instantiation, no peeling or unrolling, and PRE
executes in straight-line code before the loop label; (c) POST's spill is the
first instruction after the back-edge. The model survived.

The target's behaviour is now certain to instruction level. What is unknown is
which compiler-internal mechanism scopes the reversal to loop-local groups for
the original source. Two units, ten levers, no eleventh idea that is not a
restatement of the tenth.

**Compiler VERSION is also ruled out.** The repo ships four
(`compilers/Wii/1.0`, `1.1`, `1.3`, `1.7`) and the harness hardcodes 1.1, which
nobody had ever questioned. Compiling `d_a_wm_kinoko_base.cpp` with all four
gives **byte-identical output** -- 10 differing, 16/17, every time. That rules
out a whole untested dimension and independently validates the hardcoded 1.1
for this module.

**The wall now blocks FIVE units**, which makes it the single highest-value
unsolved problem on the project: `d_a_wm_ghost.cpp` (12/13),
`d_a_wm_kinoko_base.cpp` (16/17) and `d_a_wm_koopa_castle.cpp` (11/17) hit it
directly; the finished `d_a_wm_kinoko_1up.cpp` (9/9) and `d_a_wm_course.cpp`
(15/23) sit blocked behind those two.

Do not re-test:
declaration order, loop shape, naming, `const` qualifiers, member-vs-local
storage, array size, statement position, folding into the loop's first
iteration, an extra `getRes()` temporary, or swapping the two `getRes()` calls
(that last one regresses 10 -> 15).

  Also measured here: moving `playModes` to file scope -- the lever that fixed
  a scheduling problem in `d_a_wm_kinoko_1up.cpp` -- **regressed this unit**,
  10 -> 18 differing AND breaking a previously-clean `__sinit`, by pulling
  `playModes` out of the merged `.rodata` pool `__sinit` also draws from. A
  lever that works in one unit can actively hurt another.
- `processCutsceneCommand` is **CLOSED (192 -> 0)**. The unit is 16/17 with
  `SECTIONS CLEAN` and `VTABLE CLEAN` and no `unverifiable` slots at all.

  **The tail-merge diagnosis above was WRONG, and checking it is what broke the
  function open.** All three `setCutEnd()` sites are byte-identical and the
  target keeps two of them separate, so MWCC was never merging anything. The
  real defect: the draft called it in both command arms where the target calls
  it once, in the `0x61` guard chain's `else`. Hunting for a way to suppress an
  optimisation that was not happening would have produced nothing.

  **Biggest single lever: `if`/`else if` chains vs `switch`.** Converting the
  second dispatch alone took it 192 -> 24. MWCC collapses a branch in the chain
  form where the target emits explicit sequential compares. Reach for this on
  any dispatch-shaped function.

  Also fixed along the way: a wrong `playSound` id (0x26 not 0x25), a wrong
  argument order on the cross-module effect call (confirmed from the target's
  own `mr`/`addi` register moves), `setRate`/`setFrame` before `mPlayMode`
  rather than after, two missing `mVisible` writes, and one genuine tail merge
  closed by flipping an `if (!X) {A} else {B}` to `if (X) {B} else {A}` --
  polarity alone, no `return` needed.

**A rising differing-count mid-round is NOT evidence of a wrong turn.**
`verify_anon.py` scores by raw position, not a realigned diff, so one missing or
extra instruction early cascades into "differing" for everything downstream.
This unit's interim counts went 176 -> 182 -> 190 -> 192 through four
individually-correct fixes, because a structural dispatch defect still dominated
the score. Anyone steering on that number alone would have reverted all four.

Findings worth keeping from this unit:

- Function DEFINITION order (`.cpp` order) controls `.text` placement; class
  DECLARATION order controls vtable slots. They are independent, and getting
  them backwards cost two wrong `check_vtable` runs.
- Comparison operand order changes register scheduling: writing
  `X == c_StartPointKinokoHouseID` rather than the reverse closed 4 of 5
  differing instructions in `execute`; a `==`/`!=` polarity flip closed the 5th.
  **Settle branch polarity from the target's `beq`/`bne` bytes, not from guessed
  semantics.**
- An unreferenced `"cobKinokoAppear"` string sits in this unit's `.data` and is
  loaded by no instruction in any of its functions, yet is required for
  `createModel`'s stack offsets. Kept as a dead local; origin unsettled.
- `dsChrLib::bindAnimToNode` is real -- implemented in the DOL at `0x800DFA80`
  -- only its header is missing here. Several `dWmLib` free functions are
  likewise missing. All shadowed, none applied.

### `d_a_wm_castle.cpp` — 13/20 first pass, and a probable SECOND header static

In `wip/wm_units/agent_castle/`. Class `daWmCastle_c : dWmObjActor_c`,
sizeof 0x2b8, definition order correct, vtable slot assignment clean.

**The lead worth following: `.data`/`.rodata`/`.bss` are short by exactly
0x20/0x4/0x8, traced to a second header static with a runtime initialiser.**
The target's `__sinit` has an independently-guarded lazy-init block AFTER the
`sc_ForceList` construction -- a byte guard at `lbl_2_bss_FD48+0x10`, an
unconditional `c_START_ID` store at `+0xc`, and three floats
`{0.0f, 100.0f, 50.0f}` written into a 28-byte `.data` object. That is the same
"header static emitted per including TU" shape as `sc_ForceList`, which
explained four separate mysteries across four units before anyone recognised it.
First hypothesis -- a function-local `static` inside an inline function -- is
**refuted in that shape**, but produced a real positive: with the three floats
referenced, `.rodata` closes **exactly** 0x40/0x40, confirming
`{0.0f, 100.0f, 50.0f}` are the right pool values. `.data` did not move and
`.bss` got worse (a bare `mVec3_c` function-local static drags in guard, atexit
and weak-destructor machinery the target lacks). Both the magic-static and a
hand-written guard behaved identically, so the machinery is not the variable.

**The diagnosis points straight at a pattern already proven in this codebase.**
`-ipa file` folds any provably-constant object into `.rodata`; the target's is a
genuinely MUTABLE 28-byte `.data` object written by three runtime `stfs`. That
is exactly what `dWmLib::sc_ForceList` does: `ForceInCourseList_t` ends in an
`mVec3_c mNodePos` which is ZERO in retail `.data`, with the values living in
`.rodata` and written at runtime by `__sinit` -- because `mVec3_c` has a
non-trivial constructor, so the aggregate is not constant-initialisable.

So the second static is very likely **another aggregate with an `mVec3_c`
member**, not a bare `mVec3_c`. The arithmetic fits: `ForceInCourseList_t` is
0x24, and **0x1c is exactly four 4-byte fields plus an `mVec3_c`**. **CONFIRMED as the shape.** With
`struct KoopaShipStopConfig_t { float; float; mVec3_c; float; float; }` --
non-const, file scope, `sc_ForceList`'s own recipe -- both sections close
exactly and two functions improve:

```
.data    0x1e0 / 0x1e0   EXACT
.rodata   0x40 /  0x40   EXACT
createModel   10 -> 6 differing   (every pool-offset instruction now matches)
__sinit       52 -> 16 differing  (first 27 instructions byte-for-byte)
```

A control with plain `float`s instead of the `mVec3_c` regressed `.rodata` and
`.bss` back to UNDER, because MWCC then proves the whole object POD-constant and
skips runtime init entirely. **The `mVec3_c` member is load-bearing** -- it is
what forces dynamic initialisation, exactly as in `sc_ForceList`.

Two residuals remain, and they are probably one thing:
`.bss` is 4 OVER (two 12-byte `__register_global_object` blocks against the
target's one), and the target's three `stfs` for the `mVec3_c` are GUARDED and
conditional while a namespace-scope object initialises unconditionally.

**REFUTED, and the reason is a new MWCC fact worth keeping: a function-local
`static`'s guard-and-init is emitted at the CALL SITE and evaluated lazily, NOT
inside `__sinit`.** The target's guard sequence is physically inside `__sinit`,
so only unconditional namespace-scope construction can produce it.

Four shapes measured. The namespace-scope ARRAY is kept:

| shape | .data | .rodata | .bss | `__sinit` | `createModel` |
|---|---|---|---|---|---|
| **namespace array (kept)** | exact | exact | +4 over | **16** | **6** |
| function-local single | exact | exact | -7 under | 52 | 6 |
| namespace single, no array | exact | exact | -8 under | 40 | 6 |
| bare `mVec3_c` local static | under | exact | +8 over | 52 | 10 |

`createModel` sits at 6 across every shape, which proves that residual depends
only on `.data`/`.rodata` byte layout and is now decoupled from the guard
question -- attack it separately.

Shape 5 -- `mOffset` initialised by a call to a separate inline accessor holding
its own guarded static -- is also REFUTED. The physical-placement half was
right (the sequence DID inline into `__sinit`), but a by-value return does the
three `stfs` into the accessor's own local and then three more copying the
return into the aggregate: six writes where the target has three. A real
accessor would have to write through a reference or pointer.

**The guard is parked**, with five shapes measured and round 2's namespace array
kept. It is not what blocks this unit anyway.

### The two "unauthorable" functions were castle's own — a profile ID one digit off

**Both my inline-method hypothesis AND the original "different class" reading
were wrong.** The binding check killed mine immediately: `fn_2_15F8F0`,
`fn_2_15F950` and `fn_2_15FAA0` are all **GLOBAL**, and an inline-in-header
method is always weak. But the falsification led to the real answer -- the
functions were read as another class's because of a misread profile ID.
Compiler-verified: `WM_ANTLION_MNG` is `0x271`; **`0x272` is `WM_CASTLE`**.
`fn_2_15F8F0` searches for another `daWmCastle_c`, and `fn_2_15F950` operates on
its `mModel`/`mChrAnim`/`mPos` at this class's own established offsets, because
it IS this class.

That is the second time today that **verifying an enum by compiling a probe**
caught a value source-line counting would have got wrong -- and this one had
been frozen as a false "belongs to an undecompiled class" conclusion for three
rounds. Never count enum positions in `f_profile_name.hpp`; it has macro-driven
skew.

Now **15/20**, with `checkCourseResult` **189 -> 25** -- the best single-function
result of the session. The path, in order: `switch` conversion (189 -> 184),
hoisting singleton loads into named locals (184 -> 167), rewriting a ternary
between two adjacent constants as an explicit `if`/`else` (167 -> 78),
reordering the CASE LABELS without touching dispatch order (78 -> 30), a branch
polarity flip (30 -> 27), and staging `mVec3_c::Zero` through a local instead of
passing its address (27 -> 25). All eight levers are catalogued in
`AGENT_CONTEXT.md`.

The residual 25 is frame size (target `-0x30`, draft `-0x40`: the target reuses
one stack slot for two `mVec3_c` temporaries) plus one `f2`/`f3` register-role
swap -- scheduling only.

**`processCutsceneCommand` MATCHES; castle is 16/20 with a fully clean vtable
and zero unverifiable slots.** The defect was not devirtualisation as I guessed
-- the vtable call was present all along, six instructions later than a
misaligned diff suggested. The real cause: the draft's
`if (A && (B||C)) {...} else { setCutEnd(); }` collapsed what the source has as
**two separate nested `if`/`else` blocks, each with its own call site**
(173 -> 162). Then the ternary-to-arithmetic lever again, on
`m_2b4 = (found != nullptr) ? 1 : 0x3c` (162 -> 3), and a branch polarity flip
closed the last 3.

Superseded diagnosis, kept for the record: At the
divergence point the target does `lwz r12,0x60(r31); lwz r12,0x68(r12); bctrl`
-- a VIRTUAL dispatch through the vtable -- where the draft falls straight into
the next case body. A qualified call (`dWmDemoActor_c::setCutEnd()`) compiles to
a direct `bl`; an unqualified one on `this` goes through the vtable. Count slot
`0x68` against the class's own vtable first to confirm which method is really
being called.

Earlier this round: `TriggerCastleStopReaction` exact, `applyStopReaction` 58 -> 18
(the residual is entirely the missing `fn_80103420` call and its epilogue), and
`fn_2_15FAA0` 14 -> 6 as a real member `getKoopaShipStopPos()`. The last 6 are
pure instruction scheduling among identical operands on identical registers --
stopped there, that is the wall.

`fn_80103420` is **already resolvable**: it is in `syms.txt` as
`fn_80103420=0x80103420`, added for `d_a_wm_course.cpp` which hits the same call.
Signature corroborated from that unit's call-site registers:
`extern "C" void fn_80103420(dWmEffectManager_c *, int kind, m3d::bmdl_c &, const char *, int, int)`.

Superseded, for the record:
`fn_2_15F8F0`/`fn_2_15F950` take their `this` from
`searchBaseByProfName(WM_ANTLION_MNG, nullptr)`, so they belong to the ant-lion
manager, not to `daWmCastle_c` -- correct, and correctly refused. But the next
question was not asked: **why is another class's method inside castle's `.text`
at all?** An out-of-line method could not be. An INLINE method declared in that
class's header must be, in every TU that calls it -- the same mechanism as
`sc_ForceList`. `fn_2_15FAA0` being referenced from nowhere fits the same
picture.

So the shape to test is a shadow header for the ant-lion manager with those
methods defined inline IN THE CLASS BODY, called from castle at the sites the
disassembly shows. **Check the target's symbol binding first**: inline-in-body
gets `weak`, out-of-line gets `global`, so the binding says immediately whether
the reading is right. Three residuals may close together -- those two functions,
`fn_2_15FAA0`'s identity, and possibly the `.bss` registration count.

**Correction to my own earlier steer:** I said a file-scope aggregate would avoid
the `.bss` overspend "since that is what the landed units do". The measurement
says otherwise for this object.

Three symptoms probably share that one cause: the section shortfall;
`createModel`'s 10 residual instructions, which are **offset-only drift of
exactly 0x20** from a shared base register; and `fn_2_15FAA0`, a helper
referenced from nowhere in this TU's own `.text` or `.data` -- which is what an
inline-in-header function looks like when an out-of-line copy is also emitted.

Two functions were deliberately NOT authored, correctly: `fn_2_15F8F0` and
`fn_2_15F950` take their `this` from
`searchBaseByProfName(WM_ANTLION_MNG, nullptr)`, so they belong to a different,
undecompiled class. Inventing a member relationship to raise a count would have
been worse than leaving them out.

**Technique: verify enum values by COMPILING a probe, not by counting source
lines.** `fProfile` has macro-driven skew before index 644, and three profile
IDs compile one lower than their apparent position suggests. Compiler-verified:
`WM_KOOPASHIP=0x284`, `WM_SURRENDER=0x29d`, `WM_KOOPAJR=0x2a3`. A wrong
immediate here is invisible until it is a differing instruction.

Also found: a missing `u32 mUnk188` between `mResNodeIdx` and `mAllocator`.
Its absence shifted every later member store by 4 and made the ctor, dtor,
`execute`, `calcModel` and `resetReaction` all differ; adding it turned all five
into MATCH at once. **A single wrong member offset presents as many broken
functions.**

Shared-header proposals pending, to be applied and verified SEPARATELY:
four `dWmLib` declarations (`GetModelNodePos` name overload, `hasKoopaShipStop`,
`isKoopaShipOnCurrentWorld`, `isSpecialWorld`), two `CUTSCENE_CMD_e` values
(18 and 95), and the inline-static hypothesis above.

### `syms.txt` holds DOL addresses ONLY — REL-internal calls are a hard blocker

Every entry in `syms.txt` is a `0x8xxxxxxx` DOL address. There is **no mechanism
for a REL-internal symbol**, so a unit that calls an un-decompiled function in
its own module cannot land until whichever TU owns that address lands. Two units
are held this way:

- `d_a_wm_kinoko_1up.cpp` (finished, 9/9) needs `daWmKinokoBase_c`'s ctor/dtor.
- `d_a_wm_course.cpp` calls `fn_2_191BF0` (`.text:0x191BF0`, size 0x3C) from
  `openNeighbors` and `updateOpenAnim`.

**Do not stub such a call to make a unit link** -- a stub is wrong bytes and
hides the dependency. Leave it correct and flagged.

For anonymous **DOL** functions there IS a convention and it works:
`syms.txt` already carries `fn_800CDD60=0x800CDD60` style entries. I added
`fn_80103420=0x80103420` for course's effect-manager call; tree still verifies
5/5. Declare such a function `extern "C"` in the draft, as
`source/dol/bases/d_a_player_demo_manager.cpp` does.

**Check for this BEFORE authoring a unit**, not after: if `check_vtable.py`
prints `skip (inherited from another TU at 0x...)`, or a call target is a REL
address you cannot name, the unit is blocked no matter how many functions close.

### course, round 3: structural fixes that the MATCH count did not show

The raw count held at 15/23 while two signature errors were found and fixed --
the kind that would have poisoned every later round:

- **`openNeighbors` is `static`.** The `fastRate` bool sits in `r3`, not `r4`,
  so there is no `this` at all. Only readable from the raw bytes.
- **`searchOpenNeighbor` returns `daWmCourse_c*`**, and `updateHelpFade` calls
  `getMatClrFrame()` on the NEIGHBOUR, not on `this`. Fixing that closed the
  whole first half of the function.

Also confirmed exactly as predicted: forcing the `60` to a genuine runtime `int`
rather than a folded float literal moved `__sinit`'s pool offset `0x1c -> 0x18`
(target wants `0x30`) and shrank the `.rodata` gap correspondingly. The rest
waits on `create`/`createModel`.

`.data 0x190`, `.bss 0x10` and `.ctors` remain exact throughout.

**Round 4: course is at 15/23 MATCH with two functions one instruction short.**
`openNeighbors` is 84 of 85 (a missing trailing `addi` for a rodata base) and
`updateHelpFade` 43 of 44 (an `r4`-vs-`r5` register choice in two `setRate`
calls) -- the latter stopped deliberately, it is the register wall.
`fn_80103420` is implemented for real against the call site's argument registers
now that `syms.txt` resolves it.

`fn_2_191BF0` is declared **argument-less**, corrected from an earlier
assumption: re-reading the bytes shows `r3` holds a leftover address from an
unrelated preceding load, not `this` and not the search result. Declared and
documented as unverifiable-while-blocked rather than given a confident wrong
signature.

**Do not start `create` (219 of 224)** until the blocker below lands.

### The blocker is `daWmKoopaCastle_c`, it is bounded, and it is NOT itself blocked

`0x191BF0` belongs to `g_profile_WM_KOOPA_CASTLE` (`.data:0x4A2F4`, profile id
`0x29e`), the Bowser's Castle world-map icon. Bounds derived by the standard
method and independently re-validated with `check_bounds.py`:

```
.text   0x1910d0-0x191d40     .data   0x4a2c0-0x4a478
.ctors  0x490-0x494           .bss    0x10538-0x10568
.rodata 0x9860-0x9898
```

15 real functions plus `__sinit` and the array destructor, all anonymous.
sizeof `0x288`. Vtable is the standard 28-slot `dWmDemoActor_c` shape (dtk
reports `0x108` -- the merged-trailing-zero artifact again); `vf78` is
INHERITED here, where course overrides it.

**Every `bl` target resolves inside the unit's own range**, and `fn_2_191BF0`
itself only calls `fManager_c::searchBaseByProfName` -- already declared and
already used in course's draft. It searches for another instance of its own
profile and reads one byte at `+0x284`. **No chain behind it.** Landing this
unit resolves course's blocker completely.

This is the highest-value authorable unit on the project: self-contained, and it
unblocks a 15/23 draft sitting next door. One large function (`fn_2_1917E0`,
226 instructions) and one medium (`fn_2_1915D0`, 121); nothing else notable.
Being authored in `wip/wm_units/agent_koopa_castle/`.

### `d_a_wm_sandpillar.cpp` — NOT a leaf actor; budget accordingly

Its suspicious vtable slot is cleared (see the corrected out-of-unit rule above),
so it is authorable. But it is structurally unlike everything else in this
family, and the leaf-actor playbook will not carry it.

From the constructor (`fn_2_1776C0`, 0x160 bytes), read directly: it calls
`__ct__14dWmDemoActor_cFv`, then constructs `dHeapAllocator_c` `+0x18c`,
`m3d::mdl_c` `+0x1ac`, `m3d::fanm_c` `+0x1ec`, an `anmChr_c`-vtable'd member
`+0x234`, an `anmTexSrt_c`-vtable'd object `+0x260`, **two**
`dEf::dLevelEffect_c` objects, and `sStateMethodUsr_FI_c` at `+0x4b0`. Its
`__sinit` is `0x740` -- almost certainly building a state-descriptor table, not
the usual `sc_ForceList` boilerplate.

So it hand-rolls state machinery as a direct member while deriving from
`dWmDemoActor_c`. The machinery already exists landed in this project:
`include/game/sLib/s_State.hpp`, `s_StateMethod.hpp`, `s_StateInterfaces.hpp`,
`s_StateIDChk.hpp`, `s_FStateMgr.hpp`, and **`include/game/bases/d_actor_state.hpp`
shows the exact pattern** (`sFStateMgr_c<T, sStateMethodUsr_FI_c> mStateMgr;`) --
read that first rather than reversing `s_State.hpp` bottom-up.

**The reframe: most of the 66 functions are per-state method TRIPLES.**
`s_State.hpp` defines `STATE_FUNC_DECLARE(class, name)`, expanding to
`initializeState_##name` / `executeState_##name` / `finalizeState_##name` plus a
`static sFStateID_c<class> StateID_##name`. The
`daWmSandPillar_c::StateID_ToWaitFromTheStart` string literal is
`STATE_DEFINE`'s `#class "::StateID_" #name`. So the unit is WIDE (many named
states, each contributing three tiny methods), not deep -- much better news than
"66 functions" sounds.

**The idiom is already landed six times over.** `grep -rln "STATE_DEFINE" source/`
gives `d_a_enemy_ice.cpp`, `d_a_en_door.cpp`, `d_a_en_hatena_balloon.cpp`,
`d_a_en_togezo_base.cpp`, `d_a_iceball.cpp`, `d_a_player.cpp`.
`d_a_enemy_ice.cpp` is the smallest. Copy the idiom rather than deriving it.

Member layout traced from the constructor, `sizeof == 0x508` from `classInit`:

```
u32 mUnk188                      +0x188   (untouched by the ctor)
dHeapAllocator_c mAllocator      +0x18c
m3d::mdl_c mModel                +0x1ac
m3d::anmChr_c mAnim              +0x1ec
m3d::anmTexSrt_c mAnimTexSrt     +0x228
dEf::dLevelEffect_c mEffect1     +0x260
dEf::dLevelEffect_c mEffect2     +0x388   (stride 0x128)
sFStateMgr_c<...> mStateMgr      +0x4b0
```

`dLevelEffect_c`'s `0x128` stride is confirmed TWO independent ways: the landed
header's own field names (`m_114`, `m_118`, ...) self-document `EGG::Effect`
ending at `+0x114`, and the second instance lands exactly one stride later.
`mStateMgr`'s `0x58` is still only a subtraction, not a component sum.

**Now 40/66 with the layout exact.** `classInit` reads `li r3, 0x508` and all
four confirmed member offsets (`0x228`/`0x260`/`0x388`/`0x4b0`) land on target.

The 44 bytes were **three gaps, not one** -- `0x4` before `mAnimTexSrt`, `0xc`
before `mEffect1`, `0x1c` after `mStateMgr` -- found by laying the skeleton's own
constructor offsets beside the target's and watching the divergence grow twice
then hold flat. `mStateMgr` measured `0x3c`, not the `0x58` assumed earlier from
a subtraction, and that correction is what revealed the third gap. They are
sized placeholders (`u32`, `u32[3]`, `u32[7]`), correct in size but not yet a
decompilation; a `MoveUp`/`MoveDown` actor needs target heights, a speed and a
timer, which is exactly the shape of field that leaves no constructor trace.

**The 40 came mostly from the class declaration, not from writing bodies.**
Declaring `sFStateMgr_c<daWmSandPillar_c, sStateMethodUsr_FI_c>` instantiated a
family of template methods from landed headers and **15 matched immediately with
zero code written**. Only ONE of the 14 trivial `blr` stubs is in the vtable
(slot 23, `finalUpdate`); the other 13 are plain non-virtual helpers.

**Now 60/66.** The table merge landed: all 33 words read from
`.rodata+0x8EF8`, row-major confirmed at `row * 0xc`, replaced by one file-scope
`smc_TypeTable`. **Rows 8/9/10 are `s32`, not float** (raw bits 3,1,1 / 100,10,40
/ 100,100,100 -- not plausible floats). That plus a statement reorder in
`create`/`execute` (the target sets `mPos.y` BEFORE the `mStartPos = mPos`
struct copy) closed five functions at once. All three `*Forever`/`FromTheStart`
variants and `finalizeState_MoveReady` also match.

**Two functions the handoff called "written" were empty `{}` stubs** with real
19- and 26-instruction bodies (`finalizeState_MoveUp` at `fn_2_1783C0`,
`finalizeState_MoveDown` at `fn_2_178680`). Implementing them also cleared the
function-order violations. **"Written" in a status report is not evidence** --
verify before building on it.

**Now 61/66** (the normaliser fix cleared `__ct__`'s four phantom differences).

**Both of the "remaining work" items above were false premises. Corrected:**

- **The destructor and its four helpers were ALREADY MATCHING** -- verified
  fresh, not assumed. An empty user destructor still triggers the full
  compiler-generated member/base teardown cascade, and that cascade was already
  right from the class layout alone. The stale docstring claiming otherwise has
  been replaced in the source.
- **The order violation is not an ordering defect.** `verify_anon.py` paired
  `__dt__Q23mEf8effect_cFv` with target `fn_2_179290`, which on inspection is
  `sStateID_c`'s scalar deleting destructor -- a different class. Deleting-dtor
  wrappers (null check, one member-dtor call, optional `__dl__`) are
  byte-identical in shape across unrelated classes, so the greedy content
  matcher collided. **A `FUNCTION ORDER IS WRONG` report can be a mis-pairing
  artefact; confirm by reading the target function before acting on it.** Now
  documented in the tool. The real gap is `sStateID_c`'s own deleting destructor
  as a distinct symbol, tied to exit-time teardown of the nine static
  `StateID_X` objects.

**The `.rodata` UNDER `0x14` is a SECOND, distinct occurrence of the
`sc_ForceList` triple, and it is unexplained.** My hypothesis that the missing
floats were simply `sc_ForceList`'s `mVec3_c` was checked and is wrong in the
way that matters: the draft ALREADY reproduces that correctly. Verified by
disassembling the landed siblings' compiled objects (neither `d_a_wm_grid.cpp`
nor `d_a_wm_smallcloud.cpp` names `sc_ForceList` in source -- it is pulled in
transitively and invisibly in both):

```
grid.o        the triple appears ONCE, as three bare 4-byte pool objects
smallcloud.o  identical shape
sandpillar    already the same -- and still 0x14 short
```

The target has **two** blocks:
```
lbl_2_rodata_8F84  5 words  { 0.0, 2160.0, -30.0, -478.0, 0.0 }
lbl_2_rodata_8F98  4 words  { 100.0, 2160.0, -30.0, -478.0 }
```
A leading AND trailing zero on the first, and a wholly separate second
occurrence of the same triple with a different leading scalar. **Neither landed
sibling has anything like the second block.** No inline method in
`d_wm_obj_actor.hpp` pulls in a second `dWmLib` reference, and no
`mVec3_c`/radius-pairing call site exists in any analysed function. Open.

Also note: the `bne`/`bgt` hypothesis for `executeState_MoveReady` was to be
re-measured "once the destructor is complete" -- but the destructor turned out
to be complete already, so that condition was met all along and the residual
persists. The `-ipa file` range-analysis explanation is weaker than it looked.

Parked: `finalizeState_Ready` (3), `createMdl` (4, a stack-slot swap -- note the
inline-wrapper fix does NOT resolve every instance of that symptom), two
1-instruction trailing-`blr` cases, and `executeState_MoveReady`'s single
`bne`-vs-`bgt`. That last is HYPOTHESISED to be an `-ipa file` range-analysis
effect of incomplete neighbours, which would resolve for free when the TU is
complete -- plausible, currently unfalsifiable, re-measure rather than assume.

`.data` UNDER `0x4`: `__vt__31sFStateID_c<...>` is `0x34` here and `0x38` in the
target, the extra word a trailing zero rather than a function pointer. The same
padding appears and is correctly reproduced on `__vt__32sFStateFct_c<...>`, so
the likely reading is MWCC padding a weak vtable for whatever follows it in the
NEXT TU -- not reproducible from a single-file compile.

Earlier: `__sinit` gave up the whole 27-function state map as predicted.
`executeState_Ready` and `approach()` (`fn_2_177E70`) both MATCH.

Three findings from that round, all in `AGENT_CONTEXT.md`:
- `!(a < b)` and `a >= b` are NOT equivalent to MWCC -- the negated form emits
  fast `bge`/`ble`, the direct form `cror`-combined branches. Closed `approach()`.
- The `u32 mUnkTrailing[7]` placeholder was hiding types: `lfs`/`stfs` proved
  three of its seven slots are `float`. Now seven individually named fields
  (`mApproachCurrent`, `mApproachTarget`, `mApproachStep`, `mReadyFlag`, ...).
- The 27 state methods were grouped by state; the target **interleaves them by
  address** across all nine states. Restructuring cut order violations 15 -> 1.

**The sandpillar animates by SCALING vertically, not translating** --
`mScale.y` at `+0xe0`, confirmed against `calcMdl`'s already-matched
`setScale(mScale)` (`mScale.x/.y/.z` at `+0xdc/+0xe0/+0xe4`). That one field
unblocked all six movement-state bodies.

Six more bodies written and structurally exact (branch shapes, `cror` patterns,
register flow and `changeState` targets all matching): `executeState_MoveUp`,
`executeState_MoveDown`, and the `finalize`/`execute` pairs for `BottomWait`
and `TopWait`. `MoveUp`/`MoveDown`'s asymmetry is NOT a sign flip on
`mApproachTarget` -- that is set upstream in `finalizeState_MoveReady` -- but
`MoveUp` needing two per-Type tables against `MoveDown`'s one.

**One residual across five functions has a concrete fix:** eleven per-function
`static const float[3]` tables each compile to their own `lfsx`-indexed pool,
where the target has ONE `lbl_2_rodata_8EF8` of 33 words = 11 tables x 3 Types.
That is not a merge that happens once enough siblings exist -- it is what a
single file-scope `static const float smc_TypeTables[11][3]` produces. Read the
33 words out of the retail binary and let their order define the row order.

**Row-major layout CONFIRMED** (measured, before the agent was cut off by a
session limit): `ROW_OFFSET = row_index * 0xc` matches every function's observed
byte offset into `lbl_2_rodata_8EF8`. Five of the eleven rows are mapped:

```
row 3   +0x24   smc_heightTable      (per-Type height, used by create/execute)
row 5   +0x3c   MoveUp target
row 7   +0x54   MoveUp threshold     (decides TopWait vs TopWaitForever)
row 9   +0x6c   BottomWait counter
row 10  +0x78   TopWait counter
```

Remaining step is mechanical: read all 33 words from `original/d_basesNP.rel` at
`.rodata+0x8EF8`, lay them out as `smc_TypeTables[11][3]` in that order, and
replace the eleven per-function `static const float[3]` locals with indexed
accesses. That should close the `.rodata` gap and the five affected functions
together.


`finalizeState_Ready` parked at 3/12 (a tail-sharing polarity that resisted
`||`, `switch`, `goto` and a bool intermediate).

**This unit has a REL-internal dependency too:** `extern "C" int fn_2_171400()`.
Like `course` and `kinoko_1up`, it cannot land until whatever owns `0x171400`
lands -- the finish line is further than 66/66.

Earlier: `check_vtable.py` caught a
**missing virtual override** at slot 24 -- the draft declared no
`processCutsceneCommand`, so the compiler silently filled the slot with the
INHERITED one while the target has a real in-unit function there. That is
invisible to every other check; it now MATCHes.

The inline-wrapper fix applied here too, and generalised: three arity-mismatched
call sites in one function (`mdl.hpp:52`, `anm_chr.hpp:19`,
`anm_tex_srt.hpp:30`) took `createMdl` from 12 differing to 4. `calcEffectPos`
is 3/44, all `addi r4, r31, N` off by a constant `0x38` -- pool position, not a
wrong call, and it should resolve once the remaining pool contributors exist.

GAP 1 is named: `nw4r::g3d::ResAnmTexSrt mResAnmTexSrt`, from
`stw r3, 0x224(r29)`, mirroring `d_a_enemy_ice.hpp`'s own field. Strings
`"cobSandpillar"`, `"g3d/model.brres"`, `"ef_cobSandpillar"`,
`"Wm_cs_sandpillar01"`/`"02"` all read from retail `.data`, none invented.

Remaining: the nine state triples (27 functions -- the largest block), then six
larger unknowns. Watch `.data` as the `sFStateID_c` statics land; if it
overshoots, the state list is wrong.

**How to get the state-to-address map: read `__sinit`.** It is `0x740` bytes,
far too large for `sc_ForceList` boilerplate, and it builds the state-descriptor
table. `STATE_DEFINE` constructs one `sFStateID_c<T>` per state, each taking
three pointer-to-member-functions, so `__sinit` contains nine construction
sequences each loading three `.text` addresses -- **the whole map, in
declaration order, from one function.** The `.data` state-name strings appear in
the same order.

Do NOT try to extract it from the REL relocation table: that was attempted and
did not converge, returning a repeating `0x1b0`-stride pattern of addresses
(`0x128c8`, `0x129a8`, `0x129c8`) outside the unit's own `.text` entirely.
Unexplained; the dump is kept at
`wip/wm_units/agent_sandpillar/reloc_dump.txt` in case the stride means
something later.

GAP 3's four fields now have confirmed roles from real reads and writes, though
they are not yet named or typed precisely: `+0x4f4` a current value stepped
toward a target by `fn_2_177E70`; `+0x4f8` the target, from the per-type rodata
table with a sign flip in one branch; `+0x4fc` an integer counter gating a
`mStateMgr` vtable call at slot 6 when it hits zero; `+0x504` a flag shared with
`fn_2_1783C0`, which calls `setRate()` on `mAnim`.

When writing the triples, get ONE of each near-mirror pair (`MoveUp`/`MoveDown`,
`TopWait`/`BottomWait`) fully exact before writing its twin -- a shared mistake
copied into both costs twice as much to unpick, and the twin is nearly free once
the first is right.

Earlier: The destructor was already matching from the layout work, which
independently corroborates the member order. `calcMdl` (`fn_2_177C30`) is a full
MATCH -- the same `mMatrix.trans`/`ZXYrotM` + `setLocalMtx`/`setScale`/`calc`
idiom as every wm sibling.

**GAP 2 is named, not just sized: `mVec3_c mStartPos`**, from `create()`'s three
stores at `+0x254/+0x258/+0x25c` (`mPos.x`, a per-type table lookup overwriting a
copy of `mPos.y`, `mPos.z`). The destructor still matching after the type change
corroborates it.

Two real constants pulled from retail `.rodata` rather than invented: clip-sphere
radius `350.0f` (`+0x8f7c`) and a 3-entry height table
`{-320.0f, -200.0f, -200.0f}` (`+0x8f1c`) that `create()` indexes by
`ACTOR_PARAM(Type)`.

Remaining, in dependency order: `createMdl` (`fn_2_177D20`, 0x144) and
`calcEffectPos` (`fn_2_1788B0`, 0xB0) are still EMPTY placeholders and some of
`create()`'s 14 differing is attributable to them -- write those before tuning
`create()` further, or you are tuning against noise from your own callees. Then
`execute()`, whose first instruction is a virtual call through a slot not yet
identified (it currently calls `mStateMgr.executeState()`, flagged in-source as
probably wrong). Then the nine state triples, then six larger unknowns.

`check_vtable.py` has still not been run on this unit -- it must be, before
`execute()`, precisely because it identifies that slot.

Superseded: All nine state names came straight out of `original/d_basesNP.rel`'s
`.data` at `0x46fe8-0x47145`: Ready, BottomWait, MoveReady, MoveUp, TopWait,
MoveDown, BottomWaitForever, TopWaitForever, TopWaitFromTheStart.

**The 44 bytes are BEFORE `mStateMgr`**, by arithmetic: target `mStateMgr` is at
`+0x4b0` with `sizeof 0x508`, so it occupies `0x58`; the draft's `sizeof 0x4dc`
puts its own at `0x484`, and `0x4b0 - 0x484 = 0x2c`. Tighter still, target
`mEffect2` at `+0x388` with stride `0x128` ends exactly at `0x4b0`, so there is
NO gap between it and `mStateMgr` -- the missing member sits at or before
`mEffect1` (`+0x260`).

Localise by compiling the skeleton and laying its own constructor's
member-construction offsets beside the target's confirmed
`0x18c / 0x1ac / 0x1ec / 0x228 / 0x260 / 0x388 / 0x4b0`. The first divergence is
where the member goes. One compile-and-read, not a search.

**The `static const int x[cond?1:-1]` static-assert idiom does NOT compile under
this MWCC** (`illegal constant expression`). Read `classInit`'s own `li r3, N`
instead -- that is the technique that works for checking a class size.

66 functions, all anonymous, largest ~96 instructions. **13 `blr`-only stubs
already located by address** (`0x177CE0, 0x178000, 0x1780F4, 0x178100,
0x1781B0, 0x1781C0, 0x178540, 0x1785D8, 0x1785E0, 0x178660, 0x178670,
0x178780, 0x178850`), plus six `0x10` and three `0x1C` functions that are
probably more of the same family. Those should close almost immediately once the
class compiles -- get the skeleton and the member offsets right FIRST, since a
single wrong member offset presents as many broken functions at once.

### castle is PARKED at 16/20

`.data` and `.rodata` exact, vtable clean with zero unverifiable slots. Four
characterised residuals, none a logic defect: `createModel` (6, the ordering
wall), `checkCourseResult` (25, frame size plus one `f2`/`f3` role swap),
`getKoopaShipStopPos` (6, scheduling), `__sinit` (16, the parked guard).

**Narrowing a local's lifetime does NOT reclaim its stack slot.** Putting each
`mVec3_c` temporary in its own braced scope, so the two lifetimes are disjoint,
left the frame at `-0x40` against the target's `-0x30` and did not move a single
instruction. That is the obvious first thing to try whenever a draft's frame is
larger than the target's, and at this optimisation level it does nothing.

### kinoko_base's last defect, diagnosed precisely

The unit is **17/17**; one `.data` layout detail blocks the landing.

`getModelName()` currently does `return "";`, which MWCC pools as a 1-byte
object at unit offset `0x88` -- pushing `__vt__16daWmKinokoBase_c` to `0x90`
where the target has it at `0x88`. That single byte is the whole defect: all six
sections are otherwise byte-identical and only one relocation addend differs.

The target's `getModelName` returns `lbl_2_data_45A68` = **unit offset `0x1b8`,
an 8-byte ZERO object at the very END of `.data`** (`0x1b8 + 8 = 0x1c0`, exactly
the claimed size), i.e. after the weak vtables, not in the string pool.

Progress made, and where it stops:

| attempt | result |
|---|---|
| `static const char smc_emptyModelName[8] = "";` | vtable moves to `0x88` and **17/17 holds**, but `const` sends the array to `.rodata` |
| `static char smc_emptyModelName[8] = "";` | vtable at `0x88`, 17/17 holds, but an all-zero array goes to **`.bss`**, which overshoots `0x10` -> `0x18` |

So the shape is right -- taking the literal out of the string pool fixes the
vtable position -- but an all-zero object needs to land in `.data` at `0x1b8`,
and MWCC puts zero-initialised data in `.bss`.

Two readings worth testing:
1. Something forces an all-zero object into `.data` here (a shape that is not
   plainly zero-initialised).
2. The `.data` claim is 8 bytes too long: the unit really ends at `0x45a68`
   (`0x1b8`) and that 8-byte object belongs to the NEXT unit, with
   `getModelName` referencing it across units. Note `check_bounds.py` reports
   the current claim plausible, so this needs the neighbour's layout to settle.

## antlion's `.rodata` gap: ownership SETTLED, and it does not dissolve

I hoped the 12 unclaimed bytes at `0x85b4-0x85c0` were WM_ANTLION_MNG's, which
would have unblocked three units at once. **They are not.** Decoded word by word
from the relocation stream:

```
0x8598  100.0   -> referenced from antlion's own .text (0x15ae1a .. 0x15b4f2)
0x859c  0.0     -> referenced from antlion's own .text (0x15b00e .. 0x15b4b6)
0x85a0  1.0     -> referenced from antlion's own .text (0x15b3f2/0x15b3f6)
0x85a4  -1.0    -> no relocation (reached by displacement)
0x85a8  2160.0  -> no relocation
0x85ac  -30.0   -> no relocation
0x85b0  -478.0  -> referenced ONLY from 0x8f5f6 / 0x8f606
0x85b4  1       -> NO RELOCATION ANYWHERE IN THE MODULE
0x85b8  0       -> none
0x85bc  0       -> none
```

`0x8f5f6`/`0x8f606` are inside `fn_2_8F4C0`, a function in a completely
different and much earlier TU. And **`dtk` has no symbol boundary at `0x85b4`
for any claim to land on**: `lbl_2_rodata_85A0` is a single indivisible `0x20`
object spanning `0x85a0-0x85c0`, confirmed in the symbol map
(`size:0x20 align:4 data:float`).

So WM_ANTLION_MNG cannot start its `.rodata` there -- no boundary, and no
reference from its own `.text`. Its claim correctly begins at `0x85c0`, exactly
where antlion's validated claim ends. **Antlion's remaining byte is a same-TU
dead-pool-literal problem, now confirmed rather than assumed** -- the same shape
kinoko_red independently documented.

Note this also explains `check_bounds`'s "END cuts `lbl_2_rodata_85A0` short"
warning on antlion's 8-aligned claim: the claim genuinely does bisect a labelled
object, and that is unavoidable given the object spans the boundary.

## WM_ANTLION_MNG 15/22

`+0x184` is settled and the header stands: exactly ONE access to `0x184(rX)`
exists in the whole covering range and it belongs to `fn_2_15C230` --
**WM_BOARD's constructor, not this unit's**. So within `daWmAntlionMng_c` it is a
genuine always-zero POD member that none of the unit's own code touches. Recorded
as `int mUnk184` with that evidence.

Two more functions moved with today's levers:
- A predicate closed to MATCH once read correctly as `== 0 -> return true` (an
  ANY test) rather than `!= 0 -> return false` (an ALL test) -- branch polarity
  read off the target rather than inferred from the name.
- `rebuildAllModels` 49 -> 30 and `clearAllModels` to structurally identical,
  after a direct register-map comparison showed the first argument to
  `GetMapEnemyInfo`/`SetMapEnemyInfo` is an unnamed `daWmMap_c+0x3388` field
  re-read fresh at each call, NOT the loop variable, and that the index is an
  outer-loop accumulator (`+= 2`) rather than `sub + slot*2`.

`clearAllModels` is now byte-for-byte structurally identical -- same size, same
instructions -- and stuck purely on register NUMBERING (`r31/r30/r29` against the
target's differently-assigned `r26-r31`). Logic right, allocation wrong; parked
rather than chased.

VTABLE CLEAN, order clean, bounds unchanged. `processCutsceneCommand` is the
largest remaining target.

## WM_ANTLION_MNG is 22 functions, not 79 — and the landed header is NOT wrong

**Scope correction, mine again.** I briefed this unit as "roughly 79 functions,
the largest attempted here". It is **22**, over `.text 0x15b590-0x15c200`. The 79
was the COMBINED span through WM_BOARD (and possibly WM_CASTLE): WM_BOARD's own
classInit is `fn_2_15C200`, read out of the `.4byte fn_2_*` inside
`g_profile_WM_BOARD`, and everything from there to `0x15e7e0` is its, not this
unit's. Independently corroborated: `fn_2_15C200` has the same
`li r3, sizeof; bl __nw__7fBase_cFUl` unit-start idiom as every other classInit
in this family.

That is the SECOND time I have mis-scoped a unit by reading a profile's
neighbourhood rather than its classInit pointer. **Derive a unit's range only
from the `.4byte fn_2_*` inside its own and its neighbour's profile objects.**

Bounds, validated three ways (`check_bounds`, decoding the unit's own `__sinit`
`fn_2_15C150` and array destructor `fn_2_15C1E0`, and `check_sections`):
```
.text 0x15b590-0x15c200   .ctors 0x3b8-0x3bc   .rodata 0x85c0-0x8618
.data 0x43920-0x439d8     .bss   0xfce0-0xfcf0
```

`daWmAntlionMng_c : public dWmDemoActor_c`, `sizeof 0x1b0`, VTABLE CLEAN across
26 slots. **14/22 byte-identical** on a first pass, order clean.

### The `dWmDemoActor_c` "4-byte shortfall" is NOT a header bug

The round flagged `sizeof(dWmDemoActor_c)` probing to `0x184` where the target
needs `0x188`, and proposed it as a real shortfall in the already-landed
`include/game/bases/d_wm_demo_actor.hpp` affecting every derived class. **Probed
directly, and it is not:**

```
sizeof(dWmDemoActor_c) = 0x184
sizeof(dWmObjActor_c)  = 0x188
```

`dWmObjActor_c` DERIVES from `dWmDemoActor_c` and adds exactly 4 bytes. So a
derived class whose own first member sits at `0x188` has a 4-byte member of its
own at `0x184` -- exactly as `dWmObjActor_c` does -- rather than the base being
short. The local compensating pad is the right model; the header needs no
change.

**This mattered:** seven landed units depend on that header, and they verify
byte-exact. A 4-byte error in it would have shifted every one of their member
offsets and they could not pass. **When a proposed fix would invalidate already-
landed work, that is evidence against the fix, and it is worth checking before
touching a shared header.**

Still open, all marked UNVERIFIED in-source: `processCutsceneCommand` (the
largest, partly blocked on an un-decompiled `daWmPlayer_c` member),
`checkAttackSequenceDone`, `rebuildAllModels`, `clearAllModels`, `reviveOnRoute`,
`checkAllRevivalCountsZero`, `pickRevivedIndices`.

## course 18/23 -> 22/23, SECTIONS CLEAN. The DUMMY_ORDERING hack was WRONG.

Four functions closed at once, and the reason matters more than the count:
**the previous round's `DUMMY_ORDERING` fix was right by accident.** That pool
is not dead-seeded padding -- it is LIVE. Five functions (`createModel`'s two
`ANM_OPEN` calls, `updateOpenAnim`, `openNeighbors`, `updateClearAnim`, and
`updateHelpFade` twice) all read a shared `1.0f` from `lbl_2_rodata_87b0+0x4`,
which is a DIFFERENT object from the separately-addressed `0.0f`/`1.0f` pair at
`0x87d0`/`0x87d4` that every other call in the unit uses.

Replacing the dead-code wrapper with a real, referenced `static const u32[4]`
declared before `create()` puts it first in the pool -- correct placement for the
right reason, since a named `static const` pools eagerly at its declaration
point. `updateHelpFade` 22 -> MATCH, `openNeighbors` 41 -> MATCH,
`updateOpenAnim` 40 -> MATCH, `updateClearAnim` 95 -> MATCH.

**The lesson: a hack that produces the right bytes can still be the wrong
explanation, and the wrong explanation blocks everything downstream.** Four
functions were stuck behind a mis-diagnosis, not behind a hard problem.

`updateClearAnim` also needed two known levers: `u32 courseNo` -> `int` for
`cmpwi`/`bne` instead of `cmplwi`/`beq`, and inverting a byte check so the
branches lay out in the target's order.

### And the inline-wrapper rule closed half of what remained

`createModel` was 125 -> 12 after the pool fix. I applied the inline-wrapper rule
directly -- its OUTER `mModel.create(resMdl, &mAllocator, 0x128, 1, nullptr)`
was spelling the trailing `nullptr`, bypassing the 4-arg wrapper, while the loop
call correctly bypasses its own. Dropping it: **12 -> 6**. Sixth unit that rule
has fixed; it had been flagged to the round twice and not tried.

**Remaining 6, characterised exactly** -- a scheduling difference around the
`mParam` / `c_StartPointKinokoHouseID` compare setup:

```
     target                              draft
75   lis  r4, lbl_2_bss_FD7C@ha          li   r0, 0xff
76   li   r5, 0xff                       stw  r0, 0x238(r30)
77   lwz  r4, lbl_2_bss_FD7C@l(r4)       lis  r4, c_StartPointKinokoHouseID@ha
78   mr   r29, r3                        lwz  r0, 0x4(r30)
79   lwz  r0, 0x4(r30)                   lwz  r4, c_StartPointKinokoHouseID@l(r4)
80   stw  r5, 0x238(r30)                 mr   r29, r3
```

The target BEGINS the static load, materialises `0xff` into `r5`, finishes the
load, and stores `0xff` LAST. The draft stores it first, from `r0`. Swapping the
`mCurrentIndex = 0xff;` and `courseType` statements makes it WORSE (6 -> 8) --
measured, do not retry.

Unit is **22/23, SECTIONS CLEAN, BOUNDS PLAUSIBLE, VTABLE CLEAN**.

## castle PARKED at 18/20 — the registration/guard entanglement, mechanism found

The `.bss` +4 and the `__sinit` residual are one defect, and its cause is now
confirmed rather than suspected:

**`mVec3_c` has a user-declared `~mVec3_c() {}`** (`m_vec.hpp:128`). A member with
a user-declared destructor -- even an empty one -- makes the containing struct
non-trivial, and **MWCC always emits array registration for a dynamically
initialised array of a non-trivial type.** That single `bl __register_global_object`
plus its `__arraydtor` is the entire extra call; the target has none for this
object.

Proved by Probe E: replacing the member's type with a hand-rolled trivial POD
makes the registration DISAPPEAR. But it overshoots -- with nothing non-trivial
left, MWCC folds the whole aggregate into `.data` at compile time and emits no
runtime write at all, so `.rodata` goes 4 UNDER and `.bss` 8 UNDER. Worse, not
better.

**The target needs "guard present, registration absent" and no shape produces
both.** Array-of-`mVec3_c` gives registration without guard; POD gives neither;
Probes A-D gave guard at the cost of un-baking the `.data` scalars or growing the
frame. Seven shapes now, plus 19 on the sibling unit's identical construct.

Also worth recording: rows 0-27 of that `__sinit`, long assumed to be matching by
luck, are `sc_ForceList__6dWmLib` -- a completely different object from the shared
header that is already correct. Reading the target disassembly with REAL symbol
names rather than normalised ones is what made that visible.

## koopa_castle PARKED at 16/17 — 19 measured shapes on one construct

The declaration-versus-usage lever does NOT transfer, and the reason is a real
limit on that lever worth knowing.

**Staging through named locals breaks MWCC's cross-statement constant-folding
scope.** koopa_castle's `__sinit` writes two `mVec3_c` values that SHARE two of
their three constants, and the direct-temporary form lets MWCC reuse the same
registers (`f2` for `0.0`, `f0` for `-100.0`) across BOTH constructions -- it
treats the whole `doInit()` body as one folding scope. Every variant that
introduced named locals broke that scope: the compiler stopped sharing registers
across the two constructions and loaded each local independently, producing FOUR
loads where the target has three.

So the lever controls WHICH register holds WHICH value. koopa_castle's residual
is WHEN the derived pointer is materialised. Different axis; the lever cannot
reach it.

Measured this round, load order recorded rather than just diff counts. The
target's sequence for the first vector is `f2(x=0.0), f0(z=-100.0), f1(y=50.0)`
-- and **the current landed baseline already matches that exactly**:

| shape | load order | diffs |
|---|---|---|
| baseline, direct temporaries | `f2(x), f0(z), f1(y)` -- matches target | **13** |
| locals in x,z,y, natural-order call | collapsed to 4 loads, sharing broken | 20 |
| locals for both vectors, natural order | same collapse | 20 |
| locals in strict right-to-left z,y,x | `f2(x), f0(y), f1(z)` -- position right, VALUES swapped | 14 |
| locals in y,z,x | different swap | 17 |
| base pointer as a local declared first in the guarded block | identical to baseline | 13 |
| that plus locals | same collapse | 20 |

The closest miss (14) only reordered two already-matching loads. **None of the
seven touched the pointer-materialisation instructions at all** -- they stayed
byte-for-byte identical to baseline in every variant.

Also checked and excluded: the ABI-shape caveat from castle does not apply here.
`__sinit` writes through a global in every variant, not a hidden result pointer.

**That is 19 distinct measured shapes across two units** (12 in the earlier
sweep, 7 here) landing on the same construct without closing it. Recording the
construct as a documented wall rather than an unexplored gap.

`d_a_wm_koopa_castle.cpp` is **16/17, SECTIONS CLEAN, VTABLE CLEAN**, with only
`__sinit`'s 13 open.

## castle 18/20 — NEW LEVER: decouple DECLARATION order from USAGE order

`checkCourseResult` closed 4 -> MATCH, and the lever is new and general.

The residual was an `f2`/`f3` register swap in a three-float constructor. MWCC
evaluates constructor arguments RIGHT-TO-LEFT, so the target's instruction order
is `z, y, x` -- but float register assignment follows DECLARATION order. Those
are two separate orders and a single expression couples them:

- `mVec3_c pos2(mPos.x, mPos.y, mPos.z - 100.0f)` -- evaluation order right,
  registers assigned descending where the target assigns ascending. 4 differing.
- Staging all three through locals declared `x2, y2, z2` -- registers now
  ascending, but evaluation flips to `x, y, z` and no longer matches. Still 4,
  the mirror-image swap.
- **Declare the locals in the TARGET'S EVALUATION ORDER (`z2, y2, x2`), then pass
  them in natural order `(x2, y2, z2)`.** Byte-exact.

**The general form: when evaluation order and register assignment disagree,
decouple them by staging through locals whose DECLARATION order matches the
evaluation order and whose USE order matches the call.** Staging only one or two
of the three does nothing -- all three must be staged.

Measured limit, worth knowing: the lever did NOT transfer to
`getKoopaShipStopPos`, which returns via a hidden result pointer in `r3` rather
than storing into a stack temporary passed by address. Different ABI shape,
different problem.

`getKoopaShipStopPos` stays at 6 and is now well characterised as two coupled
defects -- `x`'s operand load order is backwards relative to `z`/`y`, and an
extra `result.z` store is scheduled before `x` is even computed where the target
defers all three stores to the end. **Six more shapes measured with no effect**,
all recorded in the source: all six store-statement orderings (MWCC schedules
independent stores by its own readiness heuristic and ignores written order);
dropping the cached `offset` reference (CSE reproduces the same address
computation); `return mVec3_c(x,y,z)` versus named result plus field assignment
(RVO makes them identical); a fully inlined return with no locals; and flipping
`x`'s addend order (which DID change the instructions but not the count).

### castle's `.bss` +4 is REAL, and the alignment rule does not apply

I suggested it might be quantisation like antlion's. It is not: `0xfd60` is
already 8-byte aligned, so the 4 bytes are content, not padding. Good check --
the antlion rule is real but narrow, and applies only when a claim end is NOT
8-aligned.

Also re-confirmed: `check_bounds.py`'s ownership check flags a unit's own
`__sinit` unless `.ctors` is in the claim, because its only reference is the
`.ctors` slot. Castle's claim was missing it. **Always pass the complete
five-section claim.**

## antlion: PARKED one byte short. The mechanism is understood and closed.

Every path to producing that `u32` `1` has now been tested. The result is a
clean impossibility argument rather than a list of failed guesses:

**Surviving `-ipa file` as a dead symbol requires a FREE function.** `DECL_WEAK`
preserves a free function whose code is never called -- all three earlier
attempts survived, just at the wrong position. Applied to a MEMBER function it
does NOT: `-ipa file` proves a member with no caller and no vtable slot is
unreachable from any TU and removes code and pool together. Measured twice, with
and without `DECL_WEAK`; the symbol is absent from `.text` entirely.

**Deferring a pool past the last strong emission requires `virtual` + an
ANONYMOUS literal.** That is the kinoko condition, re-read from kinoko_base's own
notes: being called by name is fine, but the definition must be an in-class
inline VIRTUAL, and the literal must be anonymous -- **a named `static const`
pools eagerly regardless of which function uses it.**

**The two requirements are mutually exclusive here.** A free function cannot be
virtual. Making it a virtual member would add a vtable slot and grow `.data` by
4 bytes -- and `.data` is already byte-identical to the target, as is `.text`.
That trades a correct section for an incorrect one, so it is a regression, not a
fix. The agent declined to compile it destructively, which was right.

**Final state: 37/37, order clean, VTABLE CLEAN, `.text`/`.ctors`/`.data`/`.bss`
all byte-identical, one byte of `.rodata` short.** The whole REL differs from the
original by that single byte with claim `.rodata 0x8598-0x85b8`.

**The open possibility, for a future pass:** that the `1` is not antlion's at all
but the first word of the NEXT unit's pool, with antlion ending at `0x85b4`. That
bound makes `.rodata` 8 bytes LARGE through alignment quantisation rather than
1 byte wrong, so the slice format cannot currently express it -- but WM_ANTLION_MNG
(`0x15b590`, 79 functions) is the neighbour, and landing it would settle
ownership the same way kinoko_base settled kinoko_red's.

Do not spend more rounds forcing this from antlion's own source. It gates
sandpillar, which is unfortunate, but breaking two exact sections to fix one byte
is the wrong trade.

## antlion is ONE BYTE from landing

Built with `.rodata 0x8598-0x85b8` (8-aligned) and diffed the whole REL against
the original:

```
sizes 0x2e1228 vs 0x2e1228 | 1 run, 1 byte differs
  .rodata+0x85b7   built=00   orig=01
```

**One byte in the entire module.** `.text`, `.ctors`, `.data`, `.bss` and the
whole relocation table are byte-identical. The byte is the low byte of a `u32`
`1` the target pools at `.rodata` unit offset `0x1c`, immediately after
`sc_ForceList`'s `(2160, -30, -478)` triple.

Everything else about the unit is finished: **37/37, order clean, VTABLE CLEAN,
`.text`/`.data`/`.bss`/`.ctors` all exact.** Landing it also unblocks sandpillar
at 66/66, so one byte currently gates two units.

**What is ruled out, each by measurement:**
- The array-destructor / `__register_global_object` path: `fn_2_15B570` passes
  element size and count as IMMEDIATES (`li r5, 0x24`, `li r6, 0x1`), not pool
  loads. Falsified from the target's own bytes.
- The `0x20` "structured object" at `0x85A0`: a mirage. `fn_2_15B3F0` is a
  three-instruction getter returning the single float `1.0f`; dtk's `0x20` is
  distance-to-next-label across a pool it cannot subdivide.
- Grid's `DECL_WEAK` idiom in three forms (named `static const int[]`, anonymous
  literal, private non-virtual member): all pool at offset `0x10`, AHEAD of the
  triple. Consistent with the leading/trailing rule -- every user-written
  function compiles before `__sinit`.
- Declaring a `ForceInCourseList_t[1]`: lands in `.bss` with a construction
  guard, wrong mechanism entirely.

**The one shape NOT yet tried**, and the reason it is worth trying: the linkage
deferral that landed three kinoko units was an IN-CLASS INLINE definition, not a
`DECL_WEAK` out-of-line one. In `.data` that demonstrably defers a weak
function's literals PAST the vtable pool -- i.e. past the last strong emission.
If `.rodata` has the same two-pass behaviour, an in-class inline member whose
body pools a `u32` `1` would defer past `__sinit`'s triple, which is exactly the
position needed. The three attempts so far were all out-of-line and therefore
strong.

Note also what the constant is: an integer `1` in a POOL rather than an `li`
immediate. MWCC uses `li` for a plain integer literal, so a pooled `1` implies
it is an ELEMENT of an aggregate -- a `static const` array or struct initialiser
-- not a bare scalar.

## course 18/23, SECTIONS CLEAN — and the LEADING vs TRAILING pool distinction

The pool gap is solved and it produced the rule that explains all three units
that have hit this.

**A deadstripped function's pooled literals SURVIVE. That fixes a LEADING gap
and cannot fix a TRAILING one.** Course's five missing words sat BEFORE
`create()`'s own first use, so a `DECL_WEAK void DUMMY_ORDERING()` holding them
works -- the linker strips the code and keeps the constants. Antlion's and
kinoko_red's gaps sit AFTER their `__sinit`, and `__sinit` always compiles last,
so no user-written function can pool behind it. **Same symptom, opposite
solvability.** That is why grid's landed idiom transfers to one and not the
others.

Two mechanical details that mattered:
- **One `u32[4]` array, not four separate typed statics.** Separate declarations
  get regrouped by MWCC -- the integer array jumps to the front and the floats
  defer to the very end, past the real content. A single array preserves element
  order.
- **Pool position follows FIRST USE, not declaration.** Moving `sPlayModes`'s
  declaration down to immediately before `createModel()` (its actual first use)
  put it after `create()`'s `80.0f`, matching the target. Declaring it at file
  scope above `create()` did not.

**And a correction to my own byte table.** I read `43300000` / `80000000` at
`+0x28`/`+0x2c` as two floats, `176.0f` and negative zero. They are ONE 8-byte
DOUBLE, `0x4330000080000000` -- the standard MWCC int-to-float magic constant,
confirmed by three `lfd` (not `lfs`) instructions in `updateOpenAnim`,
`openNeighbors` and `updateClearAnim` converting integers to float. The bytes
were right; the reading was wrong. **A `43300000` word followed by anything is
almost certainly the top half of that magic double, not a float.**

`__sinit` closed to MATCH; `createModel` went 165 -> 125 with the offset cascade
gone. Note the other four functions did NOT move: their gaps are a different
register-allocation choice (whether a persistent base pointer is cached at all),
so they were never downstream of the pool.

### antlion: the 0x20 "object" is a mirage

`check_bounds` flags `lbl_2_rodata_85A0` as a `0x20` object, which looked like a
structured table worth chasing. It is not: `fn_2_15B3F0` is a three-instruction
getter --
`lis r3, lbl_2_rodata_85A0@ha; lfs f1, lbl_2_rodata_85A0@l(r3); blr` --
returning the single float `1.0f` at `0x85A0`. dtk's `0x20` is just
distance-to-the-next-label across a pool it cannot subdivide.

Also falsified from target bytes: the array-destructor lead. `fn_2_15B570` uses
`li r5, 0x24` and `li r6, 0x1` -- element size and count are IMMEDIATES, not pool
loads -- so the registration machinery is not the source of the missing word.

## antlion needs ONE integer word, and the claim end must be 8-ALIGNED

Sharpened by building, not by argument. Three facts, each measured:

**1. A slice's `.rodata` claim end must be 8-byte aligned.** With
`.rodata 0x8598-0x85b4` -- the bound that exactly matches the object's `0x1c` --
the module's `.rodata` comes out `0xa604` against the original's `0xa5fc`, EIGHT
BYTES LARGE, purely from alignment quantisation. With the end at `0x85b8` or
`0x85bc` (both 8-aligned) `.rodata` is **exactly `0xa5fc`** and the whole REL is
the right size, `0x2e1228`. `0x85b4` is not 8-aligned; that was the entire
size discrepancy, and it had nothing to do with missing content.

**2. `.text`, `.data`, `.bss` and `.ctors` all link EXACTLY.** Confirmed from the
built REL's own section table against the original's. Antlion does NOT have
sandpillar's placed-weak-symbol problem.

**3. The real shortfall is ONE WORD, not three.** With the 8-aligned claim the
first differing byte is at `.rodata:0x85b7` -- the low byte of the integer `1` at
`0x85b4`. The object provides seven words ending at `0x85b4`; the target has an
eighth. The `0, 0` after it are inside the alignment tail, not content the object
owes.

So antlion is **37/37, order clean, vtable clean, bounds plausible, four of five
sections exact, and one pooled integer `1` from landing** -- which then unblocks
sandpillar at 66/66.

**What has been ruled out for producing it**, all measured: grid's
`DECL_WEAK` + named `static const int[]` idiom; the same with the `static`
dropped (anonymous literal); and the same as a private non-virtual member. All
three pool at unit offset `0x10`, AHEAD of the `sc_ForceList` triple, pushing it
to `0x1c` -- size right, layout backwards.

That matches `d_a_wm_kinoko_red.cpp`'s independently-recorded dead end on the
same shape. **But note kinoko_red LANDED**, and its resolution was that its
trailing bytes were zero and the linker filled them. Antlion's missing word is
`1`, so that escape does not apply -- the object genuinely has to emit it.

Reverted; tree green at 5/5.

## Relocations target a pool's BASE, never its entries. Do not search for
## "who references this constant".

This invalidated three separate pieces of reasoning today -- one of mine and one
in each of two agent reports -- so it is worth stating on its own.

**MWCC addresses a constant pool by materialising its BASE once (`lis`/`addi`)
and then loading with displacements** (`lfs f1, 0x10(r31)`). The only relocation
emitted is against the pool base. So decoding the relocation stream to ask "who
references `.rodata:0x85a8`" returns nothing for an entry that is loaded
constantly, and the absence proves nothing at all.

What the relocation stream IS good for is settled and still true -- identifying
which functions touch a `.bss`/`.data` OBJECT (each has its own relocation), and
ownership of a claimed range. It is `.rodata` POOL ENTRIES specifically that are
invisible to it.

**The right test for whether a pool entry is live is the per-function diff.**
Because entries are reached by displacement, a missing entry shifts the
displacements of everything after it -- so if a function loads a constant you do
not have, THAT FUNCTION CANNOT MATCH. Contrapositive, and this is the useful
form: **if every function in the unit matches, any extra pool entry the target
has is DEAD** -- pooled by something deadstripped, never loaded.

That is how antlion's missing `1, 0, 0` was classified as dead entries rather
than a missing use: 37/37 with correct displacements throughout means nothing
loads them. `d_a_wm_grid.cpp`'s landed `DUMMY_ORDERING()` idiom -- a `DECL_WEAK`
function whose `static const` array survives the function being stripped -- is
the known shape that produces exactly this.

### The two pools, dumped, for whoever picks these up

**antlion `.rodata 0x8598-0x85b4`** -- draft emits the first seven words in the
right positions; missing the trailing three:
`100.0, 0.0, 1.0, -1.0, | 2160.0, -30.0, -478.0 | 1, 0, 0`

**course `.rodata 0x87b0-0x87f0`** -- draft emits `0x2c` of `0x40`, and the five
missing words are all AHEAD of the triple, which is why its `__sinit` reads the
triple at +0x18 where the target reads +0x30:
```
0, 1.0, 003C000A, 0, 80.0, 1, 0, 0, 0, 1.0, 176.0, 80000000, | 2160.0, -30.0, -478.0 | 0
```
`003C000A` is not a float -- as two `u16`s it is 60 and 10. `80000000` is
NEGATIVE zero, not zero, which is a distinctive fingerprint: it comes from a
literal `-0.0f` or a negation. Both are strong leads for identifying the
expressions that pool them.

## antlion is 37/37, VTABLE CLEAN, BOUNDS PLAUSIBLE — 0xc of `.rodata` from landing

Order block empty, `check_vtable` exit 0, `check_bounds` exit 0 with zero
ownership problems, and `.text`/`.data`/`.bss`/`.ctors` all exact. Verified here
over `0x15ac80-0x15b590` with all three covering objects.

**THE PLACEMENT RULE, one layer deeper than this file had it.** Definition order
does set `.text` placement -- but only among STRONG functions. An explicit
out-of-line override has ordinary strong linkage and joins the definition-order
batch **even when its body is byte-identical to the inherited default**. A
virtual left purely inherited stays WEAK and is deferred to a block at the very
end, regardless of where the strong functions around it sit.

So a unit whose target interleaves overridden and inherited virtuals cannot be
ordered by moving definitions alone: **the inherited ones must be written out as
explicit out-of-line overrides to become strong and take their place.** Antlion
needed 16 of them. The whole cascade traced to ONE pinned function --
`isWaitWalkEnd()` at `0x15b320` was staying weak, and every function after it in
target order was flagged as a consequence.

The tell was `setCutEnd()`: weak, but CALLED BY NAME from
`processCutsceneCommand`, and therefore already floating to the right place.

Where a body cannot be spelled (private base members),
`return dWmEnemy_c::isWaitWalkEnd();` works -- the base's inline body still
inlines byte-identically.

**`draw()` and `doDelete()` were semantically SWAPPED** from the first round, and
the vtable dump proves which is which: `preDelete`/`postDelete` immediately
follow slot 5 and `preDraw`/`postDraw` follow slot 11, so **each lifecycle
stage's hooks sit directly after that stage's own action slot**. Slot 5 is
`doDelete`, slot 11 is `draw` -- the reverse of what every earlier round assumed.
`mModel.entry(); return SUCCEEDED;` belongs on `draw()`. That took
`check_vtable` from 8 wrong slots to 0.

**The inline-wrapper rule closed `createModel` -- six units now.** Spelling the
trailing `size_t*` explicitly on `mAnimTexSrt.create(...)` bypasses
`anm_tex_srt.hpp`'s double wrapper and fixed the last stack-slot swap.

### The remaining `0xc`, and what the build says about it

`.rodata` is `0xc` under: the target has a SECOND copy of the
`(2160.0, -30.0, -478.0)` triple plus `(1, 0, 0)` at `0x85a8-0x85bc`.

**Both bound choices were tested by building, and neither works:**
- claim `0x8598-0x85b4` (matching the object's `0x1c`, reports SECTIONS CLEAN):
  module comes out **8 bytes too LARGE**.
- claim `0x8598-0x85c0` (the full span): module comes out **8 bytes too SMALL**.

So the size is not a bounds question -- **the object genuinely has to emit those
`0xc` bytes.** Every one of the 37 functions was diffed individually; the two
that touch this pool (`GetTerritory`, `getWalkAnmRate`) are exact against the
target's own `lbl_2_rodata_859C`/`85A0`, and the other 35 reference `.rodata` at
all. Decoding the relocation stream, the ONLY references to `0x85a8-0x85c0` in
the whole module come from `0x8f5f6` and `0x8f606` — far outside antlion.

A second copy of the same triple in one TU is the shape the kinoko family had
with its model-name string: **`reuse_strings` does not merge a strong-bound copy
with a weak-bound one.** The candidate is a member that should be an in-class
inline rather than out-of-line — but note the tension with the placement rule
above, which needed the opposite. Not resolved.

Antlion is reverted; the tree is green at 5/5. **Landing it unblocks sandpillar
at 66/66.**

## antlion 36/37 on the corrected range; course 17/23 with `create` byte-exact

**antlion**, re-measured over `0x15ac80-0x15b590` with all three covering
objects: **36/37**. The corrected bounds were right at both ends -- the old head
cluster is confirmed a neighbour's, and `fn_2_15B4E0`/`fn_2_15B570` are
antlion's own `__sinit` and array destructor, both MATCH.

Validated 5-section claim, **BOUNDS PLAUSIBLE with zero ownership problems**:
```
.text 0x15ac80-0x15b590   .ctors 0x3b4-0x3b8   .rodata 0x8598-0x85c0
.data 0x43788-0x43920     .bss   0xfcd0-0xfce0
```
Note `.rodata`'s LOWER bound was wrong too, the same neighbour-boundary mistake
as `.text` -- `0x8570-0x8584` belongs to the previous unit.

**Two probe-based identifications worth copying**, both settled by compiling
rather than reading headers:
- `doDelete()` wanted vtable slot 5, and three one-line probes (`remove()`,
  `setAnm()`, `play()`) placed those at 4/6/7 -- so slot 5 is
  `scnLeaf_c::entry()`, never re-declared by `bmdl_c`/`mdl_c` and therefore
  keeping its original slot. The call is `mModel.entry()`, not `remove()`.
- `sizeof(m3d::anmTexSrt_c)` is **0x2c, not 0x34**, which revealed two
  undocumented trailing `int` members after `mAnimTexSrt`; `GetIndex()` reads
  the second at `+0x7ac`.

Ten vtable slots turned out to be LOCAL OVERRIDES that redeclare a `dWmEnemy_c`
default which is otherwise DOL-imported -- several behaviourally identical to the
DOL body, just compiled locally. Dumping the target's real vtable object
(`lbl_2_data_43810`) rather than inferring from headers is what found them.

**The open defect is PLACEMENT, and the fix is DEFINITION order.** Eight vtable
slots hold byte-identical trivial functions sitting at each other's addresses --
content right, position wrong. Two attempts at reordering class declarations did
not fix it, and one introduced a `.rodata` regression.

**That is because the two orders are independent, and this file has said so all
along: function DEFINITION order sets `.text` placement; class DECLARATION order
sets vtable slots.** Reordering declarations moves slots, not addresses. The
`FUNCTION ORDER IS WRONG` block in `verify_anon.py` already prints the exact
target sequence and flags each function `defined too late` -- reorder the
DEFINITIONS in the `.cpp` to that address order and leave the class declaration
alone.

**course** is **17/23**: `create` went 217 differing -> byte-exact, 224
instructions, decoding the whole Koopa-ship/Anchor dispatch the stub had
deferred. Two levers did it -- one `m_WorldNo` comparison must read the static
FRESH rather than reuse the cached register the rest of the function keeps it
in, and a branch-polarity flip on the `world == 7` split.

`lbl_2_bss_FD7C` is identified: it is **`dWmLib::c_StartPointKinokoHouseID`**, an
existing namespace-scope static in `d_wm_lib.hpp`, not a symbol to invent. A
first attempt hand-declared a duplicate and doubled the `__sinit` registration --
caught by `check_sections.py --dump` showing two symbols storing the same load.

`createModel`'s logic is now verified correct line by line; its residual is the
whole-file `.rodata` pool cascade, as are `updateOpenAnim`'s and
`updateClearAnim`'s. Those three close when the pool fills.

## CORRECTION: WM_ANTLION is `0x15AC80-0x15B590`. I gave the wrong bounds.

I dispatched antlion with `.text 0x15ab40-0x15b450`, derived by scanning
`g_profile_*` symbols and taking their addresses. **That scan read each profile
OBJECT's own `.data` address instead of the classInit it points at.** Read
properly, from the profile objects' first word:

```
g_profile_WM_ANTLION      -> fn_2_15AC80
g_profile_WM_ANTLION_MNG  -> fn_2_15B590
```

So WM_ANTLION is **`0x15AC80-0x15B590`**, and both ends of what I supplied were
wrong: `0x15ab40` reaches back into the PREVIOUS unit, and `0x15b450` cuts the
unit short by `0x140`.

The agent suspected the upper bound independently and was right: `fn_2_15B4E0`
(0x84) and `fn_2_15B570` (0x1c) read antlion's OWN `sc_ForceList`
(`lbl_2_data_43798`), confirmed from the relocation stream, and both sit past
`0x15b450`. A unit does not reach across a boundary for another TU's copy of a
per-TU header static -- so those functions are antlion's, and the claim had to
extend.

**A unit's `.text` starts AT its classInit**, as every landed unit does. Deriving
a range from a profile's own address rather than from `.4byte fn_2_*` inside the
profile object is a mistake to avoid repeating.

**This also confirms the sandpillar dependency.** The shared state-framework
functions at `0x15B320-0x15B4C0` fall inside the corrected antlion range, so
antlion really is the owner and really does supply sandpillar's dedup partner.
The relocation stream shows those functions referenced from vtables scattered
across `.data` (around `0x43868`, `0x496f8`, `0x4a1a0`, `0x4a500`, `0x4a730`,
`0x4b0a0`), i.e. by many different actors -- exactly what shared framework code
looks like.

**Antlion progress so far, on the WRONG bounds and so worth re-measuring:**
26/36, with constructor, destructor, `create`, `execute`, `calc` and
`processCutsceneCommand` byte-identical. `daWmAntlion_c : public dWmEnemy_c`,
`sizeof 0x7b0`, members `dHeapAllocator_c +0x6e8`, `m3d::mdl_c +0x704`,
`m3d::anmChr_c +0x744`, `m3d::anmTexSrt_c +0x77c`. `createModel` is 106/110 with
one stack-slot pair swapped; `doDelete` is 11/12 with `mModel.remove()` landing
on vtable slot 4 where the target wants slot 5.

## course: `create` is NOT blocked on the mystery static. Only `createModel` is.

The course round deferred BOTH `create` (217 differing) and the back half of
`createModel` (198) as hinging on an unidentified `.bss` word,
`lbl_2_bss_FD7C`. **Decoding the REL's relocation stream shows `create` never
touches it.**

Every reference to `.bss:0xFD7C` in the whole module:

```
from .text 0x160bce  (inside fn_2_160AA0 = createModel)
from .text 0x160bd6  (inside fn_2_160AA0 = createModel)
from .text 0x1618fa  (inside fn_2_161890 = __sinit)
from .text 0x161902  (inside fn_2_161890 = __sinit)
```

Four sites, two functions: `createModel` and `__sinit`. Nothing in
`create` (`fn_2_160610`, `0x160610-0x160AA0`) references it at all.

So the word is an ordinary file-scope static with a dynamic initialiser --
written by `__sinit`, read by `createModel` -- the same shape as `sc_ForceList`,
not the cross-function state flag it was taken for. And `create` is unblocked:
its 217 differing are its own problem and can be worked on now.

**The technique generalises and is cheap.** To find out what an unidentified
symbol IS, decode who references it rather than reasoning about what it might
mean. `original/<module>.rel`'s import table is at header offset `0x28`
(offset, size); each relocation is `>HBBI` = (running offset delta, type,
section, addend); type 202 restarts the running offset, 203 ends the table, 201
is a no-op. Map the resulting `.text` addresses back through
`bin/dtk/<module>_symbols.txt` to name the referring functions. That is the same
decode that identified sandpillar's `.rodata` bound and the ghost wrong-constant
defect.

## sandpillar's real blocker, finally identified: WM_ANTLION owns its state code

The unit is 66/66 with every section exact. It cannot land, and the reason is a
LANDING-ORDER dependency, not a defect.

Four weak symbols are live in sandpillar's object, referenced only from its own
weak vtables, and have **no dedup partner anywhere in the currently-landed
tree**:

| symbol | size |
|---|---|
| `__dt__13sStateIDChk_cFv` | 0x40 |
| `isNormalID__13sStateIDChk_cCFRC12sStateIDIf_c` | 0x8 |
| `__dt__29sFState_c<16daWmSandPillar_c>Fv` | 0x40 |
| `__dt__32sFStateFct_c<16daWmSandPillar_c>Fv` | 0x40 |

That is `0xC8`, and the link grows `.text` by `0x150` -- the rest being alignment
slack across the dispersed insertions. Every other section is byte-exact.

**Where the target keeps them: `0x15B320-0x15B4C0`.** Read straight out of the
target's own `.data` -- sandpillar's vtable slots in `auto_04_00046BE0_data`
point at `fn_2_15B320`, `fn_2_15B350`, `fn_2_15B370`, `fn_2_15B440`,
`fn_2_15B480`, `fn_2_15B490`, `fn_2_15B4C0` and friends, all tiny (0x4, 0x8,
0x18) and all OUTSIDE `[0x177690, 0x179380)`. Those addresses sit between
`g_profile_WM_ANTLION`'s classInit (`0x15AB40`) and `g_profile_WM_BOARD`'s, i.e.
inside **WM_ANTLION / WM_ANTLION_MNG**.

So the shared `sLib/s_State.hpp` framework's inline functions were first
instantiated by the antlion units, and every later user -- sandpillar included --
dedupes against those copies. With antlion un-landed there is nothing to dedupe
against, so sandpillar's copies get placed and shift the module.

**This also means sandpillar's vtable slot CONTENTS differ from the target's**:
ours point at our own emitted copies, the target's point into antlion. That is
the same defect seen from the other side, not a second one.

**Sizes, for whoever picks this up:** WM_ANTLION is `0x15ab40-0x15b450`,
**36 functions**, 3 covering objects. WM_ANTLION_MNG is `0x15b450-0x15e7e0`,
**79 functions**, 9 covering objects -- much larger.

**Correction to a long-standing note:** this file recorded sandpillar as blocked
on WM_MAP. That was wrong twice over -- the `fn_2_171400` call is fine through
the `R_2_1_171400` form, and the real dependency is antlion.

**Also resolved while investigating:** `fn_2_179290` is
`__dt__31sFStateID_c<16daWmSandPillar_c>Fv`, NOT `__dt__Q23mEf8effect_cFv`.
Traced through this TU's own `.rela.data`: offset `0x570` is the dtor slot of
`__vt__31sFStateID_c<16daWmSandPillar_c>`, and the target's vtable at the
corresponding address `0x47148` has `fn_2_179290` in exactly that slot, flanked
by already-matched neighbours. The long-standing `FUNCTION ORDER IS WRONG` flag
on that function is now explained, not merely waived.

## sandpillar is 66/66. The "extra trailing blr" was a TOOL ARTEFACT.

Two agent rounds and ~17 measured source reformulations went into
`executeState_BottomWait` and `executeState_TopWait`, each reporting
"1 differing -- an extra trailing `blr`". **There was never a defect.**

`dtk ends a function at an unconditional branch.** When MWCC emits an
unreachable `blr` after a tail call, dtk splits that `blr` off as its own 4-byte
"function", so a draft that correctly emits it reads one instruction longer.
The target's own bytes settle it:

```
fn_2_1780C0 = .text:0x001780C0; size:0x34   <- ends 4E800420  (bctr)
fn_2_1780F4 = .text:0x001780F4; size:0x4    <- 4E800020       (blr)
fn_2_1785B0 = .text:0x001785B0; size:0x28   <- ends 4E800420
fn_2_1785D8 = .text:0x001785D8; size:0x4    <- 4E800020
```

The tell was in the agent's own report: it found that ANY conditional branch
before a tail call produces the trailing `blr`, while the target supposedly had
two guards and no `blr`. That contradiction was the evidence, and it was read as
a deeper mystery instead of as a wrong premise.

`verify_anon.py` now accepts a draft that is the target plus one dead `blr`,
**and only when the target's last instruction is `bctr`** -- after a tail call a
`blr` is unreachable, so it cannot be a real function.

**Two wrong fixes were measured first, both worth recording.** Restitching the
TARGET's function list is the obvious move and it cascades: merging a lone `blr`
after `b`/`blr`/`bctr` took a correct 64/66 down to **42/52**; restricting to
`bctr` still swallowed four legitimate functions and invented two new
differences. Whether the `blr` belongs to the previous function depends on
whether the DRAFT emitted one, and only the comparison knows that. **Fix
comparison bugs at the comparison.**

### Landing sandpillar: one convention learned, one blocker left

**To call into a still-un-landed region of the SAME REL, the symbol must be
named `R_<module>_<section>_<offset>`, all hex** (`tools/elfconsts.py`:
`REL_SYM`). Module 2 is d_basesNP, section 1 is `.text`. So
`extern "C" int fn_2_171400();` compiles and verifies byte-identically and then
FAILS TO LINK -- nothing defines that name. `extern "C" int R_2_1_171400();`
links. The `fn_2_*` spelling has been recorded here as "the correct convention"
for a long time; it is correct for CODEGEN and wrong for LINKING.

**Remaining blocker: `.text` grows by `0x150`.** Built `.text` is `0x1c6154`
against the original's `0x1c6004`; every other section is exact. The object is
`0x648` over its claim, which is normally benign weak symbols -- but here some
of them are being PLACED, because sandpillar is a heavy template user
(`sFStateID_c<daWmSandPillar_c>`, `sFStateFct_c<...>`, `sStateMgr_c<...>`) and
is currently the only landed provider of those instantiations. This is the
already-recorded rule biting: **a weak symbol defined only in an un-landed
region gets placed.** The unit is reverted and the tree is green at 5/5.

## castle `__sinit`: a clean negative, and my triangulation premise was wrong

I dispatched castle's and koopa_castle's `__sinit` as "two instances of one
wall", expecting the setup that broke the temporary-materialisation wall. **They
are not the same shape**, and four measured probes establish why:

| probe | result |
|---|---|
| config struct with its own ctor + guarded `doInit()` | 28 differing, and REGRESSED `createModel` 0 -> 4 |
| drop the array, no constructor | 40 differing |
| bare aggregate, values written by an external trigger object | 38; guard at `.bss+0x18` not `+0x10`, frame `0x40` not `0x30` |
| same, with the guarded write as an ordinary member function | byte-identical to the above, 38 |

koopa_castle's guarded static contains ONLY `mVec3_c` members -- everything is
guard-driven. Castle's is a MIXED aggregate: four compile-time-constant scalars
that must stay baked in `.data`, plus one guarded `mVec3_c`. Any user-declared
constructor stops MWCC baking the scalars and makes it write them at runtime;
any external-reference assignment forces a temporary-then-copy whose stack slots
alias `sc_ForceList`'s staging.

**Castle's `.bss` +4 is REAL and is the same defect as its `__sinit` residual**,
not independent: declaring the config as a one-element ARRAY emits a second
`bl __register_global_object` plus an `__arraydtor`, because `mVec3_c` has a
user-declared destructor and MWCC always registers arrays of such types. The
target has no second registration at all -- it byte-guards a plain `.bss` byte.

Also: `check_bounds.py`'s new ownership check flags a unit's own `__sinit` as
unreferenced. That is a **known false positive** -- `__sinit` is reached from the
static-init table, not a `bl` in `.text`. Include `.ctors` in the claim and it
resolves; the castle claim omitted it.

## LANDED: kinoko_1up, seventh REL unit. The whole kinoko family is in.

Landed straight after kinoko_base unblocked it -- 9/9 with SECTIONS CLEAN and
BOUNDS PLAUSIBLE on the first compile. Three things were still needed, and all
three are the same pattern kinoko_red needed, which makes them a family recipe
rather than one-offs:

1. **`getModelName()` inline**, deferring its literal to the end of `.data`.
   The out-of-line form leaves a dangling reference past the slice, and the link
   error points straight at the address.
2. **`.data` claim widened at BOTH ends**: `0x457a8-0x458b0`, not
   `0x457b8-0x458a0`. The strong `"cobKinoko1up"` copy OPENS the unit's `.data`,
   ahead of `sc_ForceList`'s F7C0/W7C0 strings, and the weak deferred copy
   closes it. The recorded bounds were `0x10` too high at the bottom and `0x10`
   short at the top -- and the size was right, so no size check could see it.
3. **A `.rodata` claim that did not exist at all.** The unit was recorded as
   having no `.rodata`; its object emits `0xc` for `sc_ForceList`'s triple, and
   with no claim the linker appended it and shifted the whole module. Real
   range: `0x8ab8-0x8ac8`, immediately below kinoko_base's.

**THE FAMILY RECIPE, now confirmed on two leaves.** A kinoko leaf needs its model
name string in TWO places -- pooled FIRST, ahead of the header's own strings, and
pointed at by a `smc_modelResName` variable that sits LATE, just before the
vtable. One declaration cannot do both: its position fixes the pointer AND, on
first use, the literal. So declare a deliberately unreferenced pointer to the
same literal ABOVE the `d_wm_lib.hpp` include:

```cpp
static const char *smc_poolCobKinoko1upEarly_1up = "cobKinoko1up";
#include <game/bases/d_wm_lib.hpp>
```

`-ipa file` deletes the pointer as an unreferenced global; **the pooled literal
survives**, which is the entire point. This is the same dead-pointer idiom as
kinoko_base's `smc_unusedAppearName`/`smc_unusedAppearName2` pair.

## check_bounds.py now checks OWNERSHIP, from the REL's relocation stream

sandpillar's `.rodata` "0x14 missing" was never missing. **The claim's upper
bound was 0x10 too high**: `0x8ef8-0x8fa8` swept in `lbl_2_rodata_8F98`
(`{100.0, 2160.0, -30.0, -478.0}`), which belongs to a different class
entirely -- a `m3d::scnLeaf_c` user with members at `0x7c`/`0xac`/`0x1a4`,
nothing like `daWmSandPillar_c` whose members start at `0x18c`.

Proved from the REL's own relocation table: every reference to `0x8F98` comes
from `0x1794ca`, `0x1794de`, `0x17973a`, `0x179742` -- all outside the unit's
`.text` claim `[0x177690, 0x179380)`. With the bound corrected to `0x8f98` the
unit reports **SECTIONS CLEAN**. The real `.rodata` is `0x8ef8-0x8f98`.

**And `check_bounds.py` said PLAUSIBLE**, because `0x8fa8` IS a real symbol
boundary. Checks 1-3 in that tool all ask whether ADDRESSES line up; none asked
whether the CONTENT is ours. That is the same blind spot as ghost's
`0x218 == 0x218` on the wrong span, in a new place.

So the tool now decodes the REL's relocation stream and, per symbol, asks
whether ANY reference originates inside ANY range the unit claims.

**Two wrong versions of this check, both worth recording:**
- A whole-RANGE test is useless: shared data, vtables and profile objects are
  legitimately referenced from other units, and landed ghost has 15 of 25
  references coming from outside.
- A `.text`-ONLY test is also useless: strings are routinely referenced from the
  unit's own `.data` (`sc_ForceList` points at its two 5-byte names; animation
  arrays point at model names), and that version reported **10 problems on
  landed, byte-exact ghost**.

Only "no reference from anywhere inside the unit" is evidence. Validated: passes
landed ghost, kinoko_red and kinoko_base on their full claims; fails sandpillar's
wrong claim; passes sandpillar's corrected one. **Pass the COMPLETE claim
including `.ctors`** -- a `__sinit` is referenced only from `.ctors`, so omitting
it produces a false flag.

## course 15/23 -> 16/23, and `processCutsceneCommand` is byte-exact

`processCutsceneCommand` went from a stub at 129 differing to **0**. Function
identities were pinned first, which mattered: `.text` address order matches
member-definition order exactly, resolving the duplicate `~openNeighbors` /
`~updateHelpFade` size-guess labels. `0x160610` = `create`,
`0x160AA0` = `createModel`, `0x160F50` = `processCutsceneCommand`,
`0x161220` = `updateOpenAnim`, `0x161420` = `openNeighbors`,
`0x1615F0` = `updateClearAnim`, `0x161790` = `updateHelpFade`.

Also found by reading bytes rather than guessing: `mUnk248` compiles with SIGNED
opcodes (`cmpwi`/`ble`), so its type is `int`, not `u32` -- the same signedness
lever that closed a sandpillar function. And `updateOpenAnim` actually **returns
`bool`**, not `void`.

**Proposed header diff** for `include/game/bases/d_a_wm_course.hpp`:
`bool updateOpenAnim()` (was `void`), `int mUnk248` (was `u32`).

Two functions are confirmed STRUCTURALLY CORRECT already -- `openNeighbors` and
`updateHelpFade` -- with every remaining difference being the self-resolving
pool-offset artefact from `create`/`createModel` not yet pooling their floats.
Do not chase those counts.

`create` (217) and the back half of `createModel` (198) both hinge on an
unidentified `.bss` word `lbl_2_bss_FD7C`, compared against the RAW packed actor
param rather than the extracted courseNo byte. Deliberately not guessed at.

## LANDED: kinoko_red, sixth REL unit — and a SECOND check_sections false alarm

11.235% -> 11.243%. Landed by applying kinoko_base's linkage rule: an in-class
inline `getModelName()` returning a raw literal, deferring the string to unit
offset `0xf8`, after `__vt__`, where the target has it. The out-of-line form
emitted it eagerly and left a dangling reference to an address outside the
slice, which is how the link failure pointed straight at the fix.

**The `.data` claim had to grow** from `0x45b68` to `0x45b78` to cover the
deferred string and its padding.

**And the unit carries `"cobKinokoRed"` TWICE** -- a strong copy at offset 0 for
the model-name pointer, and the weak deferred copy at `0xf8`. So does the
target. That **refutes an argument I made earlier this session**, that one TU
cannot pool the same literal twice and therefore the second copy must belong to
the next unit. It can, when the copies differ in linkage, because
`reuse_strings` does not merge across it.

### The 4-byte `.rodata` "defect" never existed

Two full agent rounds went into kinoko_red's `.rodata` reading
`UNDER 0x4 -- something is missing`. It landed 5/5 with the claim UNCHANGED.
The four bytes are zero in the original and **the linker fills them**.

`check_sections.py` now reads the ORIGINAL BINARY to settle this: it parses the
REL section table (index order is fixed -- 1 `.text`, 2 `.ctors`, 3 `.dtors`,
4 `.rodata`, 5 `.data`, 6 `.bss`) and, when an object is short, checks whether
every byte of the shortfall is zero in the target. All zero -> the linker fills
it, clean. Otherwise -> still a real defect.

Validated three ways: the landed kinoko_red now reports SECTIONS CLEAN;
sandpillar's `.rodata` `0x14` shortfall is still flagged REAL (non-zero bytes);
and sandpillar's `.data` `0x4` is correctly downgraded to benign.

**That last one is a live correction:** I told the sandpillar round that both its
`.data` and `.rodata` shortfalls were real. Only the `.rodata` one is.

**This is the tool's SECOND false alarm and both cost multiple rounds.** The
lesson is not about this tool: **when a check says a unit cannot land and the
unit otherwise looks complete, try landing it.** The build is the authority and
it is cheap; the checks are heuristics standing in for it.

## SOLVED + LANDED: kinoko_base. The post-vtable emission rule is LINKAGE.

The 8-byte object that gated three units is fixed, and the unit is landed. Five
binaries verify.

**The fix is one line.** `getModelName()` moved from an out-of-line definition
returning a named `static char[8]` to an IN-CLASS (inline) definition returning
a raw string literal:

```cpp
virtual const char *getModelName() { return "       "; }
```

No helper, no pragma, no dummy. `.data` is exactly `0x1c0` with
`@STRING@getModelName__16daWmKinokoBase_cFv` at unit offset `0x1b8`, after all
three weak vtables, and the vtable back at `0x88`.

**THE RULE, and it corrects what this file previously recorded.** MWCC emits
`.data` in passes -- named objects, then per-function anonymous literal pools,
then vtables -- and what decides whether a function's pool lands in the
pre-vtable pass is **VAGUE LINKAGE, not whether the function is called by name**:

- A **STRONG** (out-of-line) function's literals are ALWAYS eager, whether or
  not it is ever called. That is why ten earlier shapes failed, and why a
  never-called out-of-line helper could never have worked.
- A **WEAK** (inline) function's anonymous literal pool IS deferred to the
  vtable-construction pass -- **even when the function is called by name
  elsewhere in the TU.** The earlier reading, that kinoko_base's two direct
  calls to `getModelName()` disqualified it, was WRONG and cost several rounds.
- **Named objects are unconditionally pass-1**, no matter who references them.
  Verified by probe: a named `#pragma explicit_zero_data` static referenced only
  from a deferred function still lands before the vtable. So the literal must be
  ANONYMOUS -- a raw literal in the function body, not a named static.
- `#pragma reuse_strings on` does NOT merge a strong-bound copy with a
  weak-bound one, so splitting across two functions to share the text does not
  work. One function must be both inline and own the literal itself.

The `d_a_en_noko.cpp` precedent was the right lead and the wrong explanation:
its deferred strings belong to inline virtuals, and it is the INLINE part that
matters, not the never-called-by-name part.

**This unblocks `d_a_wm_kinoko_red.cpp` and `d_a_wm_kinoko_1up.cpp`**, whose
vtables inherit slots defined in this TU.

## sandpillar 61/66 -> 64/66, and a signedness lever worth knowing

Verified over the full range with all three objects. Three functions closed.

**NEW LEVER: `unsigned x > 0` compiles as `x != 0`.** `executeState_MoveReady`
differed by exactly one instruction -- target `bgt`, draft `bne` -- because the
compared field was declared `u32`, and MWCC legally folds an unsigned
"greater than zero" into "not equal to zero". Declaring the field `s32` makes
the comparison signed and emits `bgt`. **Whenever a single branch differs
between `bgt`/`blt` and `bne`/`beq`, suspect the SIGNEDNESS of the operand's
type before rephrasing the condition.**

**The inline-wrapper rule closed `createMdl`** -- five units now. Here it was
the DOUBLE-wrapper form: calling the 3-arg `create(mdl, anmTexSrt, allocator)`,
which forwards through `anm_tex_srt.hpp`'s own nested wrapper, rather than the
4-arg wrapper with an explicit trailing `1`.

**`goto` label ORDER pins branch polarity and block layout together.**
`finalizeState_Ready` had two checks sharing one `goto` target, which compiled
as a forward `beq` with the failure body placed inline first -- backwards from
the target's physical layout. Giving the second check its own negative goto
(`if (type != 2) goto clear;`) and placing `set:` before `clear:` in source
order pinned both. Note what did NOT work, measured: `||`, `switch`, and an
if/else chain all range-merge into a `subi`/`cmplwi`/`bgt` shape at 5 differing,
WORSE than the 3 they replaced.

**The two remaining residuals are one instruction each and precisely
characterised.** `executeState_BottomWait` and `executeState_TopWait` both
compile to the target's exact sequence plus one extra trailing `blr` after a
tail-call `bctr`; the target ends cleanly with no epilogue. The trigger is
isolated: **at least one conditional early-return preceding a final tail-call**
-- the guard-free `executeState_TopWaitFromTheStart` compiles clean. Ruled out
by measurement: positive-if wrap, guard-return, a combined `&&` condition, a
`while(){break;}` reformulation, an explicit trailing `return;`, and fully
nested if/else with `return` in every leaf. All canonicalise to the same
extra `blr`.

**But the unit is further from landing than 64/66 suggests.** The section checks
the round did not run: **`.rodata` is `0x14` under and `.data` `0x4` under** --
real shortfalls under the repaired checker, not padding. `.bss` and `.ctors` are
exact and `check_bounds` reports PLAUSIBLE. Those missing constants are work
nobody has scoped yet.

**The `FUNCTION ORDER IS WRONG` flag on `__dt__Q23mEf8effect_cFv` is a KNOWN
false alarm** -- `verify_anon.py`'s own docstring records that exact pairing:
target `fn_2_179290` is `sStateID_c`'s scalar deleting destructor, not
`effect_c`'s, and deleting-destructor wrappers are byte-identical across
unrelated classes. Only that one function is flagged. Waive it.

## A `__sinit` pool-offset difference is a SYMPTOM, not a defect

`d_a_wm_course.cpp` is 15/23, and its `__sinit` reads as only **3 differing** --
temptingly close. It is not close, and chasing it would be wasted work.

Both are 33 instructions and the only difference is three load offsets:

```
target                 draft
lfs f2, 0x30(r5)       lfs f2, 0x18(r5)
lfs f1, 0x34(r5)       lfs f1, 0x1c(r5)
lfs f0, 0x38(r5)       lfs f0, 0x20(r5)
```

`r5` is the `.rodata` pool base, so the target's `sc_ForceList` triple sits
`0x18` further into the pool than the draft's. That is not a defect in `__sinit`
at all -- it is **six words of constants that the unit's eight still-unwritten
functions have not pooled yet.** `check_sections` confirms the shape: `.rodata`
is `0x5c` under, `.text` `0x57c` under, `.data` `0x1d8` under.

`__sinit` will close by itself once the missing functions are authored, and no
amount of work on `__sinit` will close it before then. **A low differing-count on
`__sinit` where the whole difference is pool offsets means the unit is
INCOMPLETE, not nearly done** -- read it as a progress indicator for the other
functions, not as a target.

Course's real claim, for the record:
`.text 0x1604a0-0x161940`, `.ctors 0x3d0-0x3d4`, `.rodata 0x87b0-0x87f0`,
`.data 0x44400-0x44590`, `.bss 0xfd70-0xfd80`.

## castle 16/20 -> 17/20, and a new lever: WIDEN the scope, do not narrow it

Verified over `0x15ecc0-0x15fbe0` with all three objects. `check_vtable` CLEAN,
`check_bounds` PLAUSIBLE, `.data` and `.rodata` byte-exact.

**The inline-wrapper rule closed `createModel` 6 -> 0.** That is now FOUR units
fixed by it -- ghost, koopa_castle, castle and kinoko_base. Castle's outer
`mModel.create(...)` was spelling the trailing `nullptr` and bypassing the
wrapper; its loop call was already correct. Exactly the ghost shape.

**NEW LEVER — one function-wide local beats two block-scoped ones (25 -> 4).**
`checkCourseResult` has three widely separated `mVec3_c pos` uses. The target
packs the FIRST and the LAST into ONE stack slot (`r1+0x14`) while a
concurrently-live `pos2` gets its own (`r1+0x8`), giving frame `-0x30`.

The fix is to declare `mVec3_c pos;` ONCE before the first block and REASSIGN it
at the last use site, rather than re-declaring it in each block. That collapsed
the frame to the target's `-0x30` and fixed 21 of 25 differing instructions in
one step.

**Note the direction, because the obvious move is the wrong one.** A prior round
tried the opposite -- narrowing each temporary into its own disjoint brace scope
-- and measured that it does nothing at this optimisation level. Widening the
scope so two uses SHARE a slot is what matters; narrowing to hint that they
should not share does nothing. Reach for this whenever a frame is larger than
the target's.

Residual 4 on that function is a pure `f2`/`f3` register-name swap in a
three-float constructor. Staging through named float locals produces the same
swap in the opposite direction -- no net gain, reverted.

`getKoopaShipStopPos` stays at 6, a scheduling wall: two reorderings of the
`x`/`y`/`z` local declarations both made it WORSE (13 and 7, up from 6). The
original `z, y, x` order is the best found. Recorded so nobody re-tries it.

### Two units now share the SAME `__sinit` shape

castle's `__sinit` is 16 differing and koopa_castle's is 13, and both are the
same construct: a guarded second header static -- byte guard, unconditional
field store, guarded float writes, guard set. castle's instructions 0-27 and
47-52 are byte-identical; only 28-46 differ.

That is a triangulation opportunity of exactly the kind that broke the
temporary-materialisation wall in one round after a dozen single-unit failures.
**Attack these two together, not separately.**

## koopa_castle `__sinit`: 19 -> 13, with a 12-shape measured sweep

Unit is 16/17, verified over `0x1910d0-0x191d40` with all three objects. Only
`__sinit` (`fn_2_191C30`, 58 instructions) is open, at 13 differing.

What moved it: guard test at inlining depth 0 (in the constructor), writes
pushed to depth 1 via an inline `doInit()`. Also settled -- the guard field must
be **`s8`, not `bool`**: the target tests with `extsb.`, and `bool` compiles to
a separate `cmpwi`. That did not change the count but it makes the instruction
content correct, so keep it.

The sweep, all measured against the same target:

| shape | diffs |
|---|---|
| guard + writes both at depth 0 | 19 |
| guard depth 0 / writes depth 1 (`doInit()`) | **13** |
| writes pushed to depth 2 | 19 |
| guard test through a depth-2 accessor | 13 |
| guard-true assignment moved before the writes | 20 |
| `doInit()` as a static taking an explicit reference | 13 |
| `doInit()` as a free function naming the global | **57** -- breaks array registration, length 58 -> 34 |
| writes as direct scalar field stores | 33 -- length 58 -> 52 |
| `this` captured into a named local first | 13 |
| `if (g.mDone == 0)` instead of `if (!g.mDone)` | 13 |
| writes split into two per-vector helpers | 13 |

Eight distinct shapes all land on exactly 13. That is a strong signal it is one
register-allocation decision, not a family of source-level choices.

**The residual, read off the instructions.** The target materialises the derived
pointer only AFTER the `bne`, and then writes the FIRST field through the
ORIGINAL base (`stfs f2, 0x10(r30)`), switching to the derived register only
from the second field on. The draft materialises the pointer once, before the
branch, and uses it uniformly for all six stores.

**CORRECTION — I claimed the stack staging was asymmetric. It is not.** I read
a `difflib` opcode dump, which prints only the DIFFERING runs from each side,
and mistook the two lists of differing lines for the two full instruction
streams. Zip-aligned, both sides stage three values, write three fields, stage
three more, write three more, at IDENTICAL stack offsets. An agent spent a round
testing eight shapes against a premise that was an artefact of how I read the
diff. **When comparing two instruction streams, align them index-by-index; never
infer structure from a diff that shows only the differing lines.**

The real residual, zip-aligned, is 13 instructions in two groups and ONE cause:

```
#    target                       draft
31   extsb. r0, r0                addi r4, r30, 0x10     <- draft hoists the pointer
32   stw  r3, 0xc(r30)            extsb. r0, r0
33   bne  .L_00191D00             stw  r3, 0xc(r30)
34   lfs  f2, 0x1c(r31)           bne  .L_00000F00
35   addi r3, r30, 0x10           lfs  f2, 0x1c(r31)     <- target computes it HERE
36   lfs  f0, 0x34(r31)           li   r0, 0x1
37   li   r0, 0x1                 lfs  f0, 0x34(r31)
...
42   stfs f2, 0x10(r30)           stfs f2, 0x0(r4)       <- first field via BASE
43   stfs f1, 0x4(r3)             stfs f1, 0x4(r4)
44   stfs f0, 0x8(r3)             stfs f0, 0x8(r4)
48   stfs f2, 0xc(r3)             stfs f2, 0xc(r4)
49   stfs f2, 0x10(r3)            stfs f2, 0x10(r4)
50   stfs f0, 0x14(r3)            stfs f0, 0x14(r4)
```

The draft materialises the derived pointer EARLY, before the guard test, and
uses it for all six stores. The target materialises it LATE, after the branch,
and still writes the FIRST field through the original base at `0x10(r30)`.
Seven instructions of ordering plus six of register choice = 13.

Twenty shapes have now been measured against this across two rounds and none
moves it. Also newly ruled out, with measurements: named-local vs temporary for
either vector (13, no change); the reverse (19, and it REVIVES the already-solved
stack-window defect, so direction does matter for that one); mixed
field-by-field plus temporary either way round (33, frame collapses);
`mPos1.set(...)` vs the constructor either way round (33).

On the wrapper question specifically: `mVec3_c(f32,f32,f32)` in `m_vec.hpp` does
NOT forward to `set()` -- it assigns fields in its own body -- and `set()`,
inherited from `EGG::Vector3f`, is a plain field-setter, not a forwarding
wrapper. So the `create()` wrapper-vs-bypass distinction has no analogue here.
(`mVec2_c` DOES have real wrapper chains, if that ever matters elsewhere.)

**koopa_castle is PARKED at 16/17** on one MWCC register-allocation decision.

## kinoko_base's 8 bytes are the CRITICAL PATH for three units

Tried landing `d_a_wm_kinoko_red.cpp` on its own -- it declares
`daWmKinokoBase_c` inline in its own `.cpp`, so it looked self-contained. **It
is not.** The link fails with 9 unresolved symbols: `create`, `doDelete`,
`execute`, `draw`, `processCutsceneCommand` and four more
`__16daWmKinokoBase_cF*`, all inherited vtable slots whose definitions live in
kinoko_base's un-landed TU.

The `extern "C" fn_2_XXXXXX` convention cannot help here. It works for a CALL,
because the call site names the symbol; it cannot work for a compiler-generated
VTABLE, whose slots carry the mangled names the class declaration produced.
Aliasing them is not an option either: `bin/dtk/*_symbols.txt` is generated and
gitignored, so an edit there is not durable.

**So kinoko_red, kinoko_1up and kinoko_base all wait on kinoko_base's 8-byte
`.data` object.** That single unexplained object is now the highest-value open
question in this family by a wide margin -- it gates three units, not one.

### And `.rodata` has a two-pass emission rule, measured

From the kinoko_red round, compiled and measured rather than argued: **every
named object is emitted before any function's anonymous pooled literals**,
unconditionally. Tested with grid's `DUMMY_ORDERING` trick moved to the physical
end of the file, with a bare unreferenced array, and with an
`__attribute__((used))` array at end of file. All three land at `.rodata+0x0`,
ahead of the triple. Textual position does not matter.

That **refutes the "unreferenced trailing array" hypothesis outright** -- no
named-array construction can produce a trailing pool word.

A shape that DOES reproduce kinoko_red's `.rodata` byte-for-byte exists: a
second dynamically-initialised static declared after `sc_ForceList` in parse
order. But it is disqualified as-is -- `-ipa file` will not strip a class-typed
static with a user-declared constructor even when completely unreferenced, so
`.bss` grows by `0xC` and `__sinit` by `0x24`, and both of those are already
exact. Since the target's `__sinit` is 33 instructions and the draft MATCHES it,
the target has no such second object, so this is the right mechanism and the
wrong source.

What remains: the target's TU pools a fourth `0.0f` after the triple that no
instruction loads, with a byte-identical `__sinit`. A pool entry with no
corresponding code is possible -- pool entries are data -- but nothing yet
explains what emits it.

## CORRECTION: sandpillar is NOT blocked on WM_MAP. It is 5 small functions out.

This file records sandpillar as "PARKED at 61/66, correctly blocked on WM_MAP".
**The count is right and the framing is wrong.** Re-measured with the full
object set: 61/66, and NONE of the five open functions involves `fn_2_171400`.
That call works through the existing `extern "C"` declaration, which remains the
correct convention. Blocked-on-WM_MAP was inferred from one investigation into
that symbol and then written down as the unit's status; it never followed.

Its `__sinit` is 464 instructions and MATCHES, which is worth noting on its own
given how much of this unit is state machinery.

The five, with measured diffs:

| function | addr | differing | shape |
|---|---|---|---|
| `executeState_BottomWait` | `0x1780C0` | 1 | draft has ONE EXTRA TRAILING `blr` (target 13 insns, draft 14) |
| `executeState_TopWait` | `0x1785B0` | 1 | same shape (target 10, draft 11) |
| `executeState_MoveReady` | `0x178230` | 1 | target `bgt`, draft `bne` -- both 96 insns |
| `createMdl` | `0x177D20` | 4 | 81 insns; prime candidate for the inline-wrapper rule |
| `finalizeState_Ready` | `0x177EC0` | 3 | 12 insns |

Three of them differ by a single instruction. This is one of the closest units
in the family, not a parked one.

**The general lesson: a blocker found while investigating a unit is not the same
as the unit's status.** Recording "blocked on X" without checking that X actually
accounts for the open functions parks work that was nearly done.

## Run `text_objects.py` BEFORE quoting any per-function count

dtk does not split on unit boundaries. A unit's `.text` is routinely spread over
two or three objects, and a lone function -- almost always the compiler-generated
`__sinit` -- frequently gets an `auto_fn_2_*_text.o` of its very own. Pass fewer
objects than that and `verify_anon.py` cannot tell: a function in an object
nobody handed it simply does not exist, and its absence looks exactly like a
smaller unit.

**This has now produced a wrong count three times, in both directions:**

- `d_a_wm_koopa_castle.cpp` read 15/16 while its `__sinit` was open and
  differing -- flattering, and it would have been landed broken.
- `d_a_wm_ghost.cpp` read 11/11 against one object; with all three it is 13/13.
- `d_a_wm_kinoko_base.cpp` read 16/16 against two objects; with all three it is
  **17/17**, its `__sinit` matching too. Conservative this time, but wrong.

`wip/wm_units/text_objects.py <module> <lo> <hi>` lists every covering object,
prints a ready-to-paste `verify_anon.py` command line, and names any function in
range with NO covering object at all -- those exist, and the only way to check
them is raw bytes from `original/<module>.rel` at file offset `0xF0 + address`.

Note it matches BOTH naming forms, `auto_NN_<ADDR>_text.o` and
`auto_fn_2_<ADDR>_text.o`. Matching only the first is precisely how koopa_castle's
`__sinit` went unmeasured for a round.

### kinoko_base is 17/17

Every function matches, `__sinit` and array destructor included. The unit's ONLY
remaining defect is the 8-byte `.data` object at unit offset `0x1b8`, after the
weak vtables. Nothing in `.text` is open.

## LANDED: `d_a_wm_ghost.cpp` — the fourth REL unit. 11.145% -> 11.193%

All five binaries verify. `d_basesNP.rel` md5 `17096d0ed441d44a0c31039138a8d7f8`.

Landing needed three things beyond the 13/13 draft, and the last two are the
interesting ones.

1. `syms.txt` needed `OSReport=0x8015F870`. The original code has a leftover
   `OSReport("testtest
")` debug print, and nothing in the REL resolved it.

2. **`verify_anon.py` reported a clean 13/13 on a unit that loaded the WRONG
   CONSTANT.** `create()` passed `0.0f` to `mClipSphere.set()` where the
   original passes `180.0f`. The tool normalises the relocation symbol -- it has
   to, since target symbols are anonymous -- so `lfs f1, <this>@l` and
   `lfs f1, <that>@l` are the same string. `check_sections.py` could not see it
   either: the pool was the right SIZE, just permuted. Nor could
   `check_vtable.py`.

   It surfaced ONLY in the linked binary, as **two bytes** in the REL's
   relocation table: an `@ha`/`@l` pair with addend `0x88a0` where the original
   has `0x8884`. Decoding those entries pointed straight at the defect.

   **The technique is now the standard last step: when a unit reads clean and
   the link still fails, byte-diff `bin/*.rel` against `original/*.rel` and
   decode the differing relocation entries as `>HBBI` (offset, type, section,
   addend).** It localises to the exact instruction. Recorded in the tool's own
   docstring so nobody trusts an N/N as more than it is.

3. **A `const` scalar at namespace scope is FOLDED and never reaches `.rodata`.**
   The pool head had to be two ONE-ELEMENT ARRAYS --
   `sGhostUnusedFloat[] = {1.3f}` at `+0x0` and `sGhostClipRadius[] = {180.0f}`
   at `+0x4` -- because `const float x = 180.0f;` is inlined at its use site and
   `__attribute__((used))` does not save it. Arrays are emitted; const scalars
   are not. The first is genuinely unreferenced and holds slot 0, the same job
   grid's `DUMMY_ORDERING()` does.

   Getting this wrong is expensive rather than merely wrong: a pool 4 bytes
   short shifted every downstream symbol and took the diff from 59 bytes to
   419,171.

## THE TEMPORARY-MATERIALISATION WALL IS BROKEN

Three units were blocked on it and more than a dozen single-unit attempts had
failed. Attacking all three instances together found the rule in one round.

**The rule.** MWCC anchors a by-value temporary's stack slot to the LOW end of
its region only when that temporary is passed through an INLINE WRAPPER call --
an overload that omits a trailing default argument and forwards it from inside
its own inlined body -- and NOT when it is passed to the real function directly
with the trailing argument spelled at the call site. For the shape "outer
by-value consumer, then a loop or statements reusing that value": **the outer
call must go through one level of inline wrapper, and the inner/loop call must
BYPASS its own wrapper by spelling the trailing argument explicitly.** Any other
combination puts the outer temporary at the HIGH end.

Measured 2x2, synthetic probes plus both real units:

| outer call | inner/loop call | outer temp slot | matches target |
|---|---|---|---|
| wrapper | bypass | LOW (0x8) | **YES** |
| wrapper | wrapper | HIGH | no |
| bypass | bypass | HIGH | no -- ghost before the fix |
| bypass | wrapper | HIGH | no |

**This corrects a rule recorded earlier in this file.** "Call the 4-arg inline
wrapper, not the 5-arg overload with an explicit `nullptr`" was stated as a
general fix. It is not general -- it is correct for the OUTER call and exactly
BACKWARDS for the inner one. That is why it closed kinoko_base and left
koopa_castle untouched: koopa_castle's outer call was already right and its
LOOP call was the wrong one.

**Two units fixed by one line each, both verified here:**

- `d_a_wm_koopa_castle.cpp` -- the loop's `mChrAnim[i].create(...)` was going
  through `anmChr_c`'s DOUBLE wrapper (`create` -> `create2` -> real 4-arg).
  Adding the explicit trailing `nullptr` makes `createModel` byte-exact.
  **Unit is now 16/17**, only `__sinit` open.
- `d_a_wm_ghost.cpp` -- the mirror image: its loop call was already correct and
  its OUTER `mModel.create(...)` spelled the trailing `nullptr`, bypassing its
  own wrapper. Removing it makes `createModel` byte-exact. **Unit is now 13/13
  -- every function including `__sinit` and the array destructor -- with
  SECTIONS CLEAN, BOUNDS PLAUSIBLE and VTABLE CLEAN.**

**A methodological note that mattered:** the landed precedent
`d_a_wm_dokan_route.cpp` was cited in this file as evidence about loops with
by-value consumers. Reading its COMPILED OBJECT rather than its source shows its
loop has `ANIM_COUNT == 1` and is fully unrolled with no back-branch at all. It
was never evidence about loops. Read the object, not the source, before citing a
landed unit as precedent.

Also genuinely refuted along the way, by measurement rather than by argument:
loop vs straight-line, unrolled vs real loop, call count per iteration,
preceding unrelated statements, register pressure, and distinct vs shared
temporary types. None of them moves the slot assignment.

`koopa_castle::__sinit` improved 19 -> 13 by the same depth intuition (pushing
the guarded writes into an inline member so the guard test and the writes sit at
different inlining depths), which also fixed a secondary symptom -- two
`mVec3_c` staging groups that had been landing reversed -- proving that anomaly
was a downstream cascade rather than independent. The residual 13 is the root
cause alone: the target materialises the derived pointer after the guard branch.

### `check_sections.py` had a FALSE-ALARM defect, affecting six landed units

Every previously-found defect in this tool produced a false CLEAN. This one
produced a false ALARM, and it cost two full investigation rounds on
kinoko_red's `.rodata` before anyone questioned the tool instead of the unit.

A slice claim runs to where the NEXT unit's first symbol begins, not to where
this unit's own content ends. The bytes between are inter-unit padding: they
belong inside the claim and no compiled object will ever contain them. So a
claim can legitimately exceed the object.

**Six already-landed, in-tree units have exactly this shape and all six were
reported `NOT ready to land`:** `d_awa.cpp`, `d_a_wm_cannon.cpp`,
`d_a_wm_dokan_route.cpp`, `d_a_remo_door.cpp`, `d_a_en_noko.cpp`,
`d_a_en_snake_block.cpp`. Verified directly here: compiling landed
`d_a_wm_cannon.cpp` against its own committed slice claim reported
`.rodata UNDER 0x4 -- something is missing`.

Fixed, and NOT with a fudge factor. The tool now reads the target's own symbol
map and asks whether the object covers every real symbol in the claimed range,
ignoring dtk's `gap_*`/`pad_*` labels. Covers them -> the remainder is padding
and the section is clean. Does not -> a real object is missing and it still
fails. Landed cannon and dokan_route now report SECTIONS CLEAN; kinoko_base,
whose missing 8 bytes ARE a real labelled target symbol, still fails.

### …and the fix REFUTES the generalisation that prompted it

kinoko_red was reported as "the same benign artifact". **It is not, and the
repaired tool is what shows it.** The six landed cases have UNLABELLED trailing
bytes -- dtk knows the last object's real size and leaves a hole. kinoko_red's
`lbl_2_rodata_8AF0` is a labelled object whose size covers the full `0x10`, so
the repaired check still reports `UNDER 0x4`.

The two situations look identical in the old size-only output and are distinct
in the map. **kinoko_red's 4 bytes remain unexplained**, and the unit is still
not ready to land. Do not clear it on the "known false positive" reading.

### The post-vtable-pool wall: data CAN follow it, under one narrow condition

The general question -- can source ever cause data to be emitted after the
vtable pool -- has a precedent, and the answer is yes:
`source/d_enemiesNP/bases/d_a_en_noko.cpp` places
`@STRING@slideEffect__10daEnNoko_cFv` and two siblings AFTER the weak template
vtables `__vt__32sFStateVirtualID_c<10daEnNoko_c>` and
`__vt__25sFStateID_c<10daEnNoko_c>`, themselves after the class's own strong
vtable. Those strings are literals inside `slideEffect()`/`kickEffect()` --
inline virtuals declared in `d_a_en_shell.hpp`/`d_a_en_noko.hpp`, inherited and
NOT overridden, reachable in that TU only through a vtable slot and never called
by name. Reproduced synthetically and confirmed.

**It cannot apply to kinoko_base.** Deferral to the late pool is gated on the
emitting function having no direct textual call in the TU, and kinoko_base's
`getModelName()` is called by name twice in its own `processCutsceneCommand`.
Making it inline plus `#pragma explicit_zero_data` got `.data`/`.bss` totals to
the target exactly (`0x1c0`/`0x10`) but still placed the object at offset
`0x40`, before `g_profile`. That is a tenth tested shape, failing for an
identifiable structural reason rather than by chance.

So: the flat rule "vtables are always absolutely last" is WRONG and should not
be recorded as such. The accurate rule is narrower -- **a function reachable
only through an inherited, unoverridden vtable slot has its literals deferred to
a late pool** -- and kinoko_base is structurally excluded from it.

### koopa_castle 15/17 — and how to add padding under `-ipa file`

Re-verified over the full range `0x1910d0-0x191d40` with all three target
objects: **15/17**, no order violation, SECTIONS CLEAN, VTABLE CLEAN with zero
unverifiable slots. `createModel` (6) and `__sinit` (19, down from 22) remain.
`diffdump.py`'s `TARGET_OBJS` now includes `auto_fn_2_191C30_text.o`, so the
unit's `__sinit` is visible to the tooling at last.

**The most transferable finding: `-ipa file` deletes an unreferenced global, but
keeps an unreferenced MEMBER.** Padding a section out to the target's size with
a standalone unused global does not work -- the flag eliminates it outright and
the `.bss` UNDER silently returns. Bundling the padding into a struct alongside
a member that IS referenced keeps it. **A referenced member preserves its unused
struct siblings; a referenced global does not preserve anything.** That is the
general way to add otherwise-unmotivated bytes to a section in this project.

**Declaration order controls `.bss` placement**, exactly as it already does for
`.text`, for vtable slots and for `.data`. Splitting the `__sinit` guard out of
the struct put it at `+0x10` instead of the target's `+0x28` until the
DEFINITION was moved after the struct -- reachable early via a forward `extern`
so an inline constructor can still name it. That combination moved the guard
byte to the exact target address, confirmed instruction-for-instruction.

**A warning worth recording: an out-of-line constructor reshuffles the ENTIRE
`__sinit`.** Tried as an alternative route to the same ordering, it took the
function from 21 differing to 56, moving unrelated code with it. Revert rather
than chase.

**The remaining 19 is a third instance of one wall.** The target materialises
the struct's derived pointer only AFTER the guard's `bne`; the draft computes it
eagerly, before testing. Three levers were tried against exactly this and none
moved the count: an early-return form, capturing the pointer into a named local
after the check, and building the two `mVec3_c` values as named locals first
(the lever that closed `constructCompanion`).

So this unit now carries TWO instances of the same wall -- `createModel`'s
three-way stack-slot rotation and `__sinit`'s pointer-materialisation timing --
and `d_a_wm_ghost.cpp` carries a third. **All three are the same question: WHEN
MWCC materialises a temporary or derived pointer relative to the code that uses
it.** That is worth attacking with the three instances side by side rather than
one unit at a time; single-unit attempts have now failed on it more than a dozen
times.

### kinoko_red's 4-byte `.rodata`: dedup CONFIRMED for the leading pad, and the
### trailing pad is a different phenomenon with no candidate in this TU

The constant-pool dedup hypothesis is now confirmed -- but only for the family
it explains, and that family is not red's.

**The leading-pad units are explained, and one of them is a documented hack
already in the landed tree.** `source/d_basesNP/bases/d_a_wm_grid.cpp` opens
with:

```cpp
DECL_WEAK
void DUMMY_ORDERING() {
    static const float UNUSED[] = { 0.0f };
}
```

commented as "required to ensure correct .rodata pool ordering … deadstripped by
the linker later". So grid's leading `0.0` is a deliberate seed. Tower needs no
such trick and has no `0.0f` anywhere: its leading word is **`100.0`, not
`0.0`**, from `mClipSphere.set(mPos, 100.0f)` in `setClipSphere()`, called from
`create()` -- tower's first-defined function. Re-extracted from
`original/d_basesNP.rel` to confirm, since this contradicts the assumption that
the leading word is always a zero. Tower is a genuine earlier-use dedup; grid is
an accepted hack achieving the same ordering.

**Red's trailing pad is NOT the same thing, and has no candidate:**

- `fn_2_16BEC0` (red's `__sinit`), read from the REL directly, performs exactly
  three `lfs` at pool offsets `0`, `4`, `8`. **It never reads `0xC`.**
- Red's other six functions are byte-identical and contain **zero
  floating-point instructions between them**. There is no missing `0.0f` use
  anywhere in this TU's own code.

**And the trick cannot be inverted, for a mechanical reason worth keeping:**
reproducing grid's `DUMMY_ORDERING` at the far end of the file (so it compiles
after every real function, landing at `.text:0x120`) still pools the constant
BEFORE the triple. **The auto-generated `__sinit` is always the last thing
compiled in a TU** -- true of every unit examined, base included -- so nothing
user-written can compile after it. That is why a leading-pad trick exists in
this codebase and a trailing-pad one does not.

This mirrors the `.data` finding on kinoko_base exactly: there too, nothing the
source can say gets an object emitted after the compiler's own end-of-TU output.
**Two different sections, two different units, same shape of wall** -- the last
thing MWCC emits is not addressable from source. That is now the single most
valuable open question in this family, because it blocks kinoko_base's `.data`
and kinoko_red's `.rodata` both, and solving either likely solves both.

Recorded in red's source as an open residual rather than forced. The unit is
otherwise complete: 8/8, `.data` and `.bss` exact, VTABLE CLEAN, bounds
plausible.

### koopa_castle: `execute` MATCHES, but the unit is 14/16, not 15/16

`execute` closed 20 -> 0 and `check_sections --layout` is now SECTIONS CLEAN
and `check_vtable` VTABLE CLEAN with zero unverifiable slots. Both re-verified
here.

**The 15/16 count is measured on a range that excludes one of the unit's own
functions.** `fn_2_191C30` (0xE8, 58 instructions) is koopa_castle's `__sinit`,
and it lives in `auto_fn_2_191C30_text.o` -- an object that is NOT in the
`TARGET_OBJS` map in `wip/wm_units/agent_koopa_castle/diffdump.py`, so neither
that script nor a verify run built on the same two objects can see it. Over the
unit's real range `0x1910d0-0x191d40` with that object included, the count is
**14/16**: `createModel` (6 differing) and `__sinit` (differing) are both open.

Structure of the unit's `.text`, for whoever picks it up:
`0x1910d0` classInit … `0x191bf0` isReady, `0x191c30` `__sinit`,
`0x191d20` the array destructor (0x1c), next unit at `0x191d40`. The
`__arraydtor$12805` at `0x1910B0`, which sits BEFORE classInit, is the PREVIOUS
unit's -- consistent with the rule that a wm unit ends after its own array
destructor. A verify run whose `lo` reaches back to `0x1910B0` therefore picks
up a foreign function and reports a spurious `FUNCTION ORDER IS WRONG`. **That
warning was an artefact of the range, not a defect**; over `0x1910d0-0x191d40`
no ordering violation is reported, so it can be waived.

**`__sinit`'s difference is addressing, not layout.** Both are 58 instructions
and the object layout is already right -- the guard sits at `r30+0x28`, which is
struct-base `+0x18` inside a 0x20 struct, in both. What differs:

```
target                         draft
lbz  r0, 0x28(r30)             addi r4, r30, 0x10     <- pointer computed FIRST
extsb. r0, r0                  lbz  r0, 0x18(r4)
addi r3, r30, 0x10             cmpwi r0, 0x0
```

Two separable signals. The target computes the struct pointer AFTER testing the
guard and reads the guard off the original anchor `r30`, while the draft hoists
the derived pointer and addresses everything through it. And the target tests
with **`extsb.`** where the draft uses **`cmpwi`** -- `extsb.` is the signed-char
test MWCC emits under `-char signed`, which points at the guard being declared
as a `char`/`s8` and tested as `if (!g)`, not as a `bool` member reached through
the struct pointer.

That suggests the guard is a separate file-scope static declared adjacent to the
struct rather than a member of it -- same address, same total `.bss`, different
addressing. Worth trying before anything more elaborate.

**`createModel` is a clean 3-way stack-slot rotation**, characterised exactly:

| temporary | target slot | draft slot |
|---|---|---|
| outer `resMdl` copy passed to `mModel.create` | `0x8(r1)` | `0x10(r1)` |
| per-iteration `ResAnmChr` copy | `0xc(r1)` | `0x8(r1)` |
| per-iteration `resMdl` re-copy | `0x10(r1)` | `0xc(r1)` |

Six instructions, all `stw`/`addi` immediates; everything else byte-identical.
This is the same loop-local group wall as ghost, and the inline-wrapper fix is
confirmed dead for this instance.

**A `.bss` lesson worth generalising:** an unexplained `.bss` symbol is not
automatically a missing declaration. `lbl_2_bss_10538` needed NO new static --
the anonymous `@12806` (0xc, `sc_ForceList`'s array-registration bookkeeping)
plus `dWmLib::c_StartPointKinokoHouseID` (0x4) already summed to 0x10 at the
right offset, pulled in transitively by the same `-ipa file` mechanism that
keeps `sc_ForceList` itself. Declaring a new one put `.bss` 10 bytes over.
**Check `check_sections`'s symbol dump for something already present at the
right size and offset before declaring anything.**

Also confirmed against the target bytes: the guarded float writes are plain
`.rodata` constant loads (`0.0f`, `50.0f`, `-100.0f`), with no `bl` in the
block -- the earlier "runtime CSV lookup" reading is dead. And a guard given
`: mDone(false)` in the constructor init-list gets constant-folded away; relying
on implicit static-storage zero-init is what makes the check get emitted.

### CORRECTION: kinoko_base's 8-byte tail IS its own. I got this wrong.

I concluded that the `0x18` block at `0x45A68` belonged to WM_KINOKO_RED and
that kinoko_base's `.data` was therefore `0x1b8` -- what the draft already
produces. **That is wrong and the commit that recorded it overstates.**

The string-pooling argument in it is sound and still holds: kinoko_base already
emits `"cobKinokoRed"` at its own offset `0x68`, one TU cannot pool that literal
twice, so the copy at `0x45A70` is RED's. What I then did was lump the 8 bytes
at `0x45A68` in with it, and those are a separate object with separate evidence:
`fn_2_16BCD0` is inside kinoko_base's own `.text` and is its own vtable slot 32,
and it returns the ADDRESS of `lbl_2_data_45A68`. A unit's own method returning
its own static is the ordinary reading; a method returning a pointer into the
next TU's data is not, and I flagged that as a consequence rather than treating
it as the refutation it was.

WM_KINOKO_RED's `.data` starts at **`0x45A70`**, now confirmed constructively
rather than by elimination -- the unit was authored against that split and its
`.data` matches the target offset-for-offset at `0xf8`.

**So kinoko_base's `.data` is `0x458b0-0x45a70` = `0x1c0`, the original claim,
and the 8-byte object at `0x1b8` is still unexplained.** Everything else in the
kinoko_base entry above stands: 16/16, the two-pointer fix, the weak-vtable
reading, and the finding that MWCC emits every `.data` object before the vtable
pool across nine tested shapes. That last one is what makes the object hard, and
it is now a live blocker again rather than a dissolved one.

### `d_a_wm_kinoko_red.cpp` — 8/8, exact layout, one 4-byte residual

Authored this session in `wip/wm_units/agent_kinoko_red/`. Independently
re-verified here, not taken on report:

```
.text 0x16BDA0-0x16BF70   .ctors 0x404-0x408   .rodata 0x8AF0-0x8B00
.data 0x45A70-0x45B68     .bss   0xFE90-0xFEA0
```

`verify_anon` 7/7 over the object that covers it (the 8th, the array
destructor, and the 9th, `sc_ForceList`'s `__sinit` at `fn_2_16BEC0`, are not
in that `.o`). `check_bounds` PLAUSIBLE on all four mapped sections.
`check_sections --layout` gives `.data` **ok at `0xf8` with every strong offset
matching the target** -- `"cobKinokoRed"`@0, F7C0@0x10, W7C0@0x18,
`sc_ForceList`@0x20, profile@0x44, `"cobKinokoAppear"`@0x50, the two-pointer
array@0x60, the model-name pointer@0x68, vtable@0x70 -- and `.bss` **ok**.
`check_vtable` **CLEAN** across all 32 slots. No shadow headers needed; it
compiles against real `include/` only.

**A real gap in `bin/dtkspl`:** no split `.o` covers `fn_2_16BEC0`. It was
checked instead against raw bytes read out of `original/d_basesNP.rel` at file
offset `0xF0 + 0x16BEC0`. Worth knowing that the split tree is not complete and
that the REL itself is the fallback.

**The residual, and why the obvious explanations are dead.** `.rodata` is 4
bytes under: the target's block at `0x8AF0` is `0x10` -- the
`(2160.0f, -30.0f, -478.0f)` triple followed by one zero word -- and the draft
pools only the triple at `0xC`.

The triple is `dWmLib::sc_ForceList`'s, a header static in `d_wm_lib.hpp`
emitted into every TU that includes it, so **identical source cannot pool
differently between TUs** and the difference must be positional. I dumped every
`.rodata` block in the module and found the triple in ~70 of them. That killed
both candidate rules at once:

- It is NOT a blanket "the triple is always padded to four floats". Both shapes
  occur throughout: `0x88B8` (landed grid) and `0x9320` (landed tower) are
  `0x10` with the triple LAST and a word BEFORE it; `0x8AF0` (red) and `0x8B00`
  (star) are `0x10` with the triple FIRST and a zero word after.
- It is NOT end-of-section alignment. **Every** block ends 8-aligned in both
  shapes, so alignment does not discriminate between them.

The most promising remaining reading, untested: MWCC pools each distinct
constant once per TU, and `0.0f` is deduped against an EARLIER pool slot in TUs
that already use it. kinoko_base's own block (`0x8AC8`, `0x28`) does contain
`0x00000000` before the triple and has no trailing zero, which fits. If that is
right, the draft is missing a `0.0f` use rather than a padding word, and the
question is which expression in RED's source produces one.

**Not ready to land** on that 4 bytes alone.

### koopa_castle 14/16 — and three new MWCC levers, all verified

`processCutsceneCommand` went **218 differing -> MATCH** at 226 instructions,
and `constructCompanion` **4 -> MATCH**. Both independently re-verified here
against the target range `0x1910d0-0x191c60`: 13/15 in the unit's own range,
with only `execute` (20) and `createModel` (6) still open.

**The offset probe — a reusable technique, not a one-off.** To read an unknown
member offset out of the compiler rather than guessing it: declare
`struct Probe : TheClass {};`, take `&((Probe *)0)->member`, compile, and read
the offset straight off the address-materialising instruction. This is how the
unidentified virtual call in `processCutsceneCommand` was resolved, and it
applies to every "which member is at 0x…" question this family keeps raising.

**`daWmKoopaCastle_c`'s vtable pointer lives at `this+0x60`, not at offset 0.**
Confirmed from the constructor, which is byte-identical to the target and does
`stw r4, 0x60(r30)` with `r4 = __vt__17daWmKoopaCastle_c`. So a virtual call
reading `lwz r12, 0x60(r30); lwz r12, 0x68(r12); bctrl` is dispatching through
THIS class's own vtable at slot `0x68/4`, and `execute`'s own dispatch to
`processCutsceneCommand` independently pins slot `0x60/4 = 24` to it. Reading
such a sequence as an access through a data member is the wrong default for
this family.

`cOwnerSetMg_c` is ruled out as the source: it has no vtable at all, and a
probed cast lands at `0x64`, not `0x60`.

**Three new levers for the catalogue:**

- **MWCC does not tail-merge identical else-branches.** Writing a combined
  `if (A && B && C)` gives the three failure paths ONE shared else-block; the
  target duplicates the block per exit. Matching the target means writing the
  nested `if`/`else` form with the block written out at each exit, which reads
  worse and compiles right. This closed the last instructions of
  `processCutsceneCommand`.
- **Naming a string literal into a local** (`const char *nodeName = "Koopa0";`
  rather than passing it inline) — took `constructCompanion` 4 -> 2.
- **Splitting a copy-initialisation into declaration plus assignment**
  (`mVec3_c pos; pos = f(...);` rather than `mVec3_c pos = f(...);`) — took the
  same function 2 -> 0. Cheap; try both first on any small residual.

**Two corrections to earlier records for this unit:**

- `field_0x139` is NOT a koopa_castle member. `0x139` is below `0x188`, where
  this class's own fields start — it is the INHERITED
  `dWmDemoActor_c::mIsCutEnd`, confirmed by probe and consistent with every
  landed sibling.
- `lbl_2_bss_10538` and `lbl_2_bss_10548` are **two separate `.bss` symbols**
  (0x10 and 0x20 per the target's `.bss` map), not one object read at `+0x10`.
  `execute` references `lbl_2_bss_10548` directly. The guarded float writes in
  `__sinit` are plain `.rodata` constant loads, not a runtime CSV lookup —
  there is no `bl` anywhere in that path.

**Not ready to land**: `.rodata` is 8 under and `.bss` 0x20 under, both owed to
`execute`'s unbuilt object.

### kinoko_base: 16/16, layout exact to `0x88`, and the tail is 8 bytes

Re-measured from the target bytes rather than from the earlier summary, and two
things changed.

**1. `createModel` MATCHES.** The wrapper fix took: all 16 target functions in
`0x16b2d0-0x16bda0` are byte-identical modulo symbol names, `createModel`
included (161 instructions). The "same wall as ghost" note for this unit is
stale -- ghost's instance of it is still open, kinoko_base's is not.

**2. A second `.data` defect, invisible to every size check, is fixed.**
`lbl_2_data_458F0` carries `.4byte lbl_2_data_458F0` TWICE, at unit offsets
`0x50` AND `0x54` -- two distinct function-local `const char *` statics both
initialised to `"cobKinokoAppear"`. The draft had one. Because `0x54` is
padding either way, `.data` totals `0x1b8` with one pointer or two, so
`check_sections.py` reports the identical size and `--layout` shows no gap.
The only way to see it is to read the target's relocated words. Fixed by
declaring `smc_unusedAppearName2` alongside the first; `.text` is unchanged at
`0xe60` and the layout now matches the target symbol-for-symbol through `0x88`.

**3. `getModelName`'s target is confirmed, not inferred.** `fn_2_16BCD0` (the
unit's last vtable slot) is `lis r3, lbl_2_data_45A68@ha; addi; blr`. So the
8-byte object at unit offset `0x1b8` is real and is what `getModelName` returns.

**4. The weak vtables ARE placed.** `lbl_2_data_45938` is `0x130` = the `0x88`
`__vt__16daWmKinokoBase_c` plus `0xA8` that dtk renders as zeros. `0xA8` is
exactly `__vt__13dWmObjActor_c` (0x78) + `__vt__Q23m3d13anmChrBlend_c` (0x18) +
`__vt__Q23m3d8anmChr_c` (0x18), the three weak vtables the draft emits. They
render as zeros because their entries are cross-module REL imports, not
self-relocations. Do not read a zero run in a REL data dump as absent data.

**5. `#pragma explicit_zero_data on` is the third MWCC data category.** The
open question was how to get an all-zero object into `.data` when MWCC sends
zero-initialised data to `.bss`. This pragma does it, scoped to the declaration:
`.data` becomes exactly `0x1c0` and `.bss` exactly `0x10`, both matching the
claim. It is a real, contained answer to a question that had been open.

**It is still not enough, and the reason is a rule worth keeping:** MWCC emits
every `.data` object BEFORE the vtable pool, without exception. Nine shapes were
tested -- plain static, `const`, `__declspec(weak)`, `__declspec(section)`,
anonymous namespace, class static member, template static member, a static
local of an inline (weak) function, and a definition physically at end of file
behind a forwarder. **All nine land at `0x88`, in front of `__vt__`**, pushing
the vtable to `0x90`. Weakness does not move an object into the vtable group;
they are separate pools and the vtable pool is last.

So the target's object at `0x1b8` was not emitted by this TU's data pass, and
the question is no longer "which construct" but "whose object".

**The neighbour settles the shape of the answer.** `g_profile_WM_KINOKO_RED` is
at `0x45AB4`, and WM_KINOKO_RED's `.data` reproduces kinoko_base's pattern
exactly -- `F7C0` `0x45A80`, `W7C0` `0x45A88`, `sc_ForceList` `0x45A90`, profile
`0x45AB4`, `"cobKinokoAppear"` `0x45AC0`, **two pointers to it** `0x45AD0`, then
`0x45AD8` = `{ptr to "cobKinokoRed", 0}`, then its vtable at `0x45AE0`. The
two-pointer idiom is in BOTH units, which confirms finding 2 independently.

That leaves `0x45A68` (8 zeros) and `0x45A70` (`"cobKinokoRed"`, 0xD, +3 pad) as
a `0x18` block between the two units, and a hard constraint on who owns it:
**MWCC pools identical string literals, and kinoko_base already has its own
`"cobKinokoRed"` at unit offset `0x68`.** One TU cannot emit that string twice.
So the `0x45A70` copy is NOT kinoko_base's, and the `0x18` block belongs to
WM_KINOKO_RED -- whose `.data` therefore starts at `0x45A68`, not `0x45A80`.

**Which makes kinoko_base's `.data` `0x458b0-0x45a68` = `0x1b8`, exactly what
the draft already produces**, and moves the whole 8-byte puzzle into the
neighbour, where it appears as a leading rather than a trailing object.

Two consequences, both needing a build to settle:
- `getModelName` would then return a pointer into the NEXT unit's `.data`, which
  no landed unit does and which the slice format cannot express directly.
- The `.bss` claim `0xfe80-0xfe90` needs re-deriving; the draft wants `0x18`.

**Do not treat the `.data 0x458b0-0x45a70` claim as settled.** It was validated
by `check_bounds.py`, and `check_bounds.py`'s wm family rule -- "a unit's `.data`
opens on F7C0/W7C0" -- is the very assumption this evidence puts in doubt.

### `fn_2_171400`'s owner is `WM_MAP` / `dScWMap_c` — do NOT chase it

Scouted to settle whether sandpillar's blocker was authorable. It is not:

```
sizeof            0x6C510  (433,664 bytes -- from classInit's own lis/subi pair)
functions         72, ALL anonymous, ~19KB of code
member arrays     4x dWmMapModel_c (0xbf8), 4x a dCsvData_c-derived type at
                  0x16518 EACH (~365KB alone), 4x another at 0x3f08
own dependencies  THREE, all themselves anonymous and un-decompiled
                  (0x18cff0, 0x161420, 0x161220)
```

This is the world-map/course-configuration data for the whole game, not an actor
in the normal sense, and chasing it opens a second chain behind the first.

**A methodological catch worth copying:** the walk-backwards pattern found the
WRONG owner first. `WM_MANTA`'s classInit at `0x170EB0` matched the
`pad_*` / array-destructor / `gap_*` / `classInit` pattern perfectly, and Manta
is a real, small, bounded unit. The tell that `fn_2_171400` was not Manta's:
it sits AFTER Manta's last function with **no array-destructor pattern between
them**. Reporting Manta would have sent someone to author the wrong unit.

**But `fn_2_171400` is `dScWMap_c::getWorldNo()`**, declared in
`include/game/bases/d_s_world_map_static.hpp:13` as
`static u8 getWorldNo() NOINLINE { return m_WorldNo; }` -- **defined inside the
class body in a shared header**, so it is an inline member with weak linkage,
not an out-of-line function in an un-decompiled TU. That is a materially
different situation from what `extern "C" int fn_2_171400()` assumes, and it is
**TESTED, and the `extern "C"` convention is CORRECT.** Calling
`dScWMap_c::getWorldNo()` for real emitted `getWorldNo__9dScWMap_cFv, weak`
as a PLACED 3-instruction function at the tail of `.text` (`+0x10`),
byte-identical to the target's own `fn_2_171400`, NOT stripped. The build
compiles landed slices fresh and copies un-landed regions verbatim, so there is
no linkable WM_MAP object to weak-dedupe against -- sandpillar's own copy is the
only candidate and shifts everything downstream. The call site also went
0 -> 11 differing: spelling it `ClassName::method()` rather than through the bare
`extern "C"` declaration changed MWCC's register scheduling around the call.

**SANDPILLAR IS PARKED at 61/66**, correctly blocked on WM_MAP. Do not
"improve" that `extern "C"` declaration into a typed call until WM_MAP lands.

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


## New tool: `wip/wm_units/profile_map.py` — resolve a profile to its classInit

**A unit's `.text` starts AT its classInit, and that address exists only as a
relocation in the first word of the profile OBJECT.** It is not in the symbol
map, and the `profile - 0x34` folklore is a heuristic that does not hold across
the family. Reading a profile's own `.data` address instead has mis-scoped a
unit twice, both times expensively (WM_ANTLION dispatched with BOTH ends wrong;
WM_ANTLION_MNG scoped at ~79 functions when it is 22 — that 79 was the combined
span running on through WM_BOARD).

The tool walks the REL relocation stream, looks up the relocation patching each
`g_profile_*` address, and reports the addend. Sorted by classInit, consecutive
rows give each unit's range directly — a unit runs from its own classInit to the
next one.

```
python wip/wm_units/profile_map.py d_basesNP 0x15e000 0x165000
```

**It reproduces the exact landed ranges of six shipped units** — WM_CANNON,
WM_CLOUD, WM_DOKAN, WM_DOKANROUTE, WM_GHOST, WM_GRID — so it is validated
against ground truth, not just self-consistent. Run it before scoping any new
unit in a REL. Deriving a range any other way is the mistake this file has now
recorded three times.

### The map for the WM region of `d_basesNP`

```
WM_CANNON        .text 0x15e7e0-0x15ecc0  (0x4e0)   LANDED
WM_CASTLE        .text 0x15ecc0-0x15fbe0  (0xf20)   parked 18/20
WM_CLOUD         .text 0x15fbe0-0x1604a0  (0x8c0)   LANDED
WM_COURSE        .text 0x1604a0-0x161940  (0x14a0)  22/23
WM_DANCE_PAKKUN  .text 0x161940-0x1622b0  (0x970)   16 functions, OPEN
WM_DOKAN         .text 0x1622b0-0x162580  (0x2d0)   LANDED
WM_DOKANROUTE    .text 0x162580-0x163620  (0x10a0)  LANDED
WM_GHOST         .text 0x163620-0x164230  (0xc10)   LANDED
WM_GRID          .text 0x164230-0x164430  (0x200)   LANDED
WM_HANACHAN      .text 0x164430-0x165c70  (0x1840)  open
```

WM_DANCE_PAKKUN is the gap immediately after course and is the natural next
unit: landing course and then it makes `0x15fbe0-0x163620` contiguous except
for castle.

## MEASURED CORRECTION: `.rodata` placement follows DECLARATION ORDER. The "passes" rule is false.

Two rules were written down here and they contradicted each other:

- "MWCC emits constant data in PASSES: named objects first, then per-function
  anonymous literal pools, then vtables."
- "A named `static const` pools EAGERLY AT ITS DECLARATION POINT."

Every piece of evidence we had was consistent with both, because the one
experiment that seemed to settle it changed two variables at once: course's
`sOpenFullRateSeed` was both NAMED and DECLARED FIRST, and it landed first.
"Named objects go first" and "declaration point wins" predict that equally.

A probe agent built discriminating files and read the raw `.rodata` bytes out of
the compiled objects (`wip/wm_units/agent_pool_order/`). **The passes rule is
falsified.**

```
Probe 1  fnA, fnB, then named sTail declared LAST
         .rodata: fnA-pool(0x00) fnB-pool(0x0c) sTail(0x18)      <- sTail LAST

Probe 2  named sTail declared FIRST (control)
         .rodata: sTail(0x00) fnA-pool(0x0c) fnB-pool(0x18)      <- sTail FIRST

Probe 4  fnA, fnB, named sMid, fnC
         .rodata: fnA-pool fnB-pool sMid fnC-pool                <- interleaved
```

Probe 4 is the one that closes it. A named object declared in the MIDDLE lands in
the middle, between two function pools. There is no "named first" pass and no
hybrid; **placement tracks source declaration order, full stop.**

### The corollaries, all measured in the same round

- **Unreferenced `static const` declared last: STRIPPED** by `-O4` (internal
  linkage at namespace scope). Already recorded; re-confirmed.
- **Unreferenced `extern const` declared last: SURVIVES, and still lands LAST.**
  This is the useful one. `extern` defeats stripping without needing a reference,
  and the surviving object still obeys declaration-point placement.
- **A composite `static const` array reproduces a trailing integer.**
  `struct R { float a, b, c; unsigned int flag; };` with records
  `{2160.0f, -30.0f, -478.0f, 0}` and `{2160.0f, -30.0f, -478.0f, 1}` declared
  last emits both records after the function pools, integer flags and all.

### Why this matters: it un-parks antlion

Antlion was parked with an impossibility argument built on the passes rule:
MWCC never pools integer literals (**still true**), so its missing trailing `1`
had to be a named object; the passes rule said a named object can never land
after the function pools; therefore unreachable from source. **The second step
is now known to be wrong.** An `extern const` array declared after every function
definition is exactly the shape that produces a trailing integer word.

And the target's own bytes read like the composite probe: `.rodata` has
`{2160.0f, -30.0f, -478.0f, 0}` at `0x8588` and `{2160.0f, -30.0f, -478.0f, 1}`
at `0x85a8` -- a repeated record, the second of which is what antlion is short of.

**Do not treat "MWCC emits in passes" as a rule again.** It is deleted, not
qualified. The weak-versus-strong deferral rule from kinoko_base is separate and
still stands: a WEAK (in-class inline) function's ANONYMOUS pool defers past the
vtable pool even when called by name. That is about linkage, not declaration
order, and the two compose.

### Methodology note for whoever probes next

With `wiimj2d`'s flag set a lone 4-byte float literal is small-data eligible and
lands in `.sdata2`, not `.rodata`, so a naive probe produces no competing
anonymous data at all and both arms look identical. The fix was to give each
function a 3-float local array, which exceeds the threshold and forces a real
per-function pool entry. `d_basesNP` compiles with `-sdata 0 -sdata2 0` and does
not have this problem -- but a probe run under the wrong module's flags can
silently answer a different question than the one asked.

## course PARKED at 22/23. The `createModel` residual is a 2-attractor wall, measured 19 ways.

The unit is **22/23, SECTIONS CLEAN, BOUNDS PLAUSIBLE, VTABLE CLEAN**, and its
LINK blocker is fixed (see below). Only `createModel` remains, at 6 differing.

The whole defect is *which register holds the constant `0xff`*. The two
instruction multisets are otherwise identical:

```
     target                              draft
75   lis  r4, c_StartPointKinokoHouseID@ha    li   r0, 0xff
76   li   r5, 0xff                            stw  r0, 0x238(r30)
77   lwz  r4, ...@l(r4)                       lis  r4, ...@ha
78   mr   r29, r3                             lwz  r0, 0x4(r30)
79   lwz  r0, 0x4(r30)                        lwz  r4, ...@l(r4)
80   stw  r5, 0x238(r30)                      mr   r29, r3
```

With `0xff` in a non-r0 register the post-pass scheduler is free to sink the
`stw` into the load-use delay slots, which is the target's shape. Pinned in r0,
it must complete before the `lwz r0` that reuses the register. Everything else
follows from that one allocation.

**There are exactly two reachable attractors, and neither is the target:**

| shape | 0xff | address value | differing |
|---|---|---|---|
| baseline (`0xff` a bare literal) | **r0** | r4 | 6 |
| any variant caching the static in a local | **r4** | r5 | 6 |
| TARGET | **r5** | **r4** | 0 |

The second attractor is the target's pair *exactly swapped*, which looked like
one bit of information. **It is not.** The declaration-order lever -- MWCC
assigning registers by declaration order even where evaluation order differs --
does NOT apply here, and that was tested properly rather than assumed:

- `idx` declared before `kinokoId`, and after it: **byte-identical output**,
  verified instruction-by-instruction, not just by differing count.
- `idx` as `int` / `u8` / `s32` / `u32`: byte-identical to each other.
- both locals hoisted to the top of the function, in BOTH orders: 76 differing,
  and identical to each other -- so that axis is order-independent too, and is a
  dead end regardless because it disrupts the loop's own allocation.

Full list of measured negatives, so nobody re-runs them: bare local for `idx`
(6, baseline shape); `idx` declared at top (6, baseline); `mCurrentIndex` retyped
`u32` (6, baseline); 1-arg inline setter `setCurrentIndex(0xff)` (fully inlined
to the baseline shape); caching the static (6, swapped shape); caching it after
the store (6, reverts to baseline); setter + cached static (swapped shape);
extra `idx` on top of cached static (swapped shape); `kinokoId` as `int` not
`u32` (swapped shape); `const u32 kinokoId` (swapped shape); caching `mParam`
INSTEAD (**7 -- worse**); collapsing the condition to a `bool` (**153 -- much
worse**); flipping the comparison operand order (6, also flips `cmplw`'s
operands); swapping the `mCurrentIndex`/`courseType` statements (**8 -- worse**).

The only untried idea is perturbing register pressure with an unrelated dead
store to move the allocator's free list. **Do not.** It is a shape we could not
justify in landed source, and this project does not ship hacks that produce the
right bytes for the wrong reason -- course itself lost four functions to exactly
that mistake (the `DUMMY_ORDERING` mis-diagnosis).

### The LINK blocker course actually had, and the spelling rule

Independent of the 6 instructions, course **could have reached 23/23 and still
not landed.** It declared three functions living in un-landed regions of
`d_basesNP` using the `fn_2_*` spelling. That spelling compiles and verifies
byte-identically and then FAILS TO LINK -- nothing defines those names.

```
fn_2_191BF0 -> R_2_1_191BF0     fn_2_189A20 -> R_2_1_189A20
fn_2_189990 -> R_2_1_189990
```

Renamed, and the count **held at 22/23** across the rename with every previously
matching function still matching -- including the two that call these -- so the
form is invisible to codegen and `verify_anon` normalises `bl R_2_1_*` against
the target's `bl fn_2_*` correctly. No tooling gap.

**Keep the two spellings separate in source.** Comments should say `fn_2_*`,
because that is what you search the symbol map for; only the declarations and
call sites take the `R_2_1_*` form. A blanket rename swept the comments too and
left them claiming the map uses the linker's spelling, which it does not.

`fn_80103420` is unaffected -- a DOL address resolved through `syms.txt`, a
different mechanism entirely.

**Checked repo-wide: course was the only draft with this defect.**

## TECHNIQUE: make the compiler tell you any offset or size

`offsetof`/`sizeof` are compile-time constants, so you can extract one without a
symbol map, a debugger or an inference from disassembly: instantiate an
INCOMPLETE template with it and read the value out of the error message.

```cpp
#include <game/bases/d_base_actor.hpp>
template<int N> struct S;
struct P : public dBaseActor_c {      // derive, so protected members are reachable
    S<offsetof(P, mMng)> a;           // one per line, at CLASS scope
    S<sizeof(fBase_c)> b;
};
```

```
(10136) illegal use of incomplete struct/union/class 'S<100>'
```

The value is DECIMAL. Two practical notes, both learned by getting them wrong:

- **Put the probes at class scope, not inside a function body.** Inside a
  function MWCC aborts after the first error and you get one value per compile.
  At class scope it reports them all in one pass.
- **Check the values come out monotonically increasing in declaration order.**
  If they do not, the errors are not being reported in source order and the
  mapping from name to value is guesswork -- fall back to one member per compile.
  A quick pass of mine produced offsets that could not be reconciled with the
  header's own declaration order, which is how this note exists.

This is strictly better than the STATIC_ASSERT sweep for *finding* a layout
(STATIC_ASSERT confirms a value you already guessed; this one tells you the
value). Use STATIC_ASSERT afterwards to lock the answer in.

## PROCESS: a blocked function is not a blocked unit

The dance_pakkun scout produced a compiler-verified class layout and a full
16-function map, and then wrote **no source at all**, because two of the sixteen
functions depend on one unidentified field.

That is the wrong trade and it is worth naming, because it is a tempting one. A
draft with 14 functions authored and 2 stubbed is worth much more than a mapping
document: the next round starts from a measured per-function MATCH table instead
of from prose, and the 14 either match or they do not -- which is information
nobody has until someone compiles. **Author everything that is not blocked,
stub what is, and report the count.**

The related failure is stopping at "I could not identify X" when X is *lookupable*.
"Do not invent a field on a shared header" is right -- seven landed units depend
on `d_wm_demo_actor.hpp` and `f_base.hpp`. "Do not look the field up" is not the
same instruction. The offset technique above exists precisely so that this class
of unknown gets measured instead of guessed or abandoned.

## RETRACTION: antlion's `.rodata` ownership is NOT settled. I closed it with an invalid test.

Last session I recorded the question of who owns `0x85b4-0x85c0` as settled, on
the grounds that `0x85b4` has **no relocation anywhere in the module** and the
only neighbouring reference comes from a far earlier TU.

**That test is invalid, by a rule written down in this very file.** Relocations
target a constant pool's BASE, never its entries -- MWCC materialises the base
once and loads with displacements, so a pool entry reached by displacement
generates no relocation of its own. "No relocation at this address" is therefore
the EXPECTED observation for a live pool entry, and proves nothing about
ownership. I used the exact search I had documented as structurally blind, and
the section immediately below this one in the file says so.

The rest of that note stands: `lbl_2_rodata_85A0` really is one indivisible
`0x20` object with no boundary at `0x85b4`, and that part was checked properly.
But the ownership conclusion drawn from the relocation search is withdrawn.

**Ownership is open again**, and the live hypothesis is the one I dismissed: that
`0x85b4-0x85c0` belongs to the NEXT TU. If so, the 8-byte quantisation that made
a `.rodata 0x8598-0x85b4` claim come out large is an artefact of a DANGLING claim
end -- nothing claimed immediately after it -- rather than evidence that antlion
owes the content. WM_ANTLION_MNG is in active development at 16/22, and its own
pool has been traced to start with a `0x18` unidentified run; if that run begins
at `0x85c0` the three words at `0x85b4` belong to neither and the question is
still open, so this needs settling by layout, not by relocation search.

## antlion: the trailing word is unreachable from its own source. Measured, not argued.

The `extern const` idea from today's placement probe was tried and **fails, for a
reason that composes with the probe rather than contradicting it.**

`extern const u32 sAntlionTrailing[1] = {1};` declared at the very end of the
file gives the right `.rodata` SIZE (`0x20`) and the wrong layout: it lands at
unit offset `0x10`, ahead of `sc_ForceList`'s triple, pushing the triple to
`0x14-0x1c`. Declared at the top instead it lands at `0x00`. Position within the
file does not move it past the pool at all.

**The governing rule is one we already had: `__sinit` is compiled LAST.** Its
pooled constants are therefore always the final entries in the TU, and an object
declared at the "very end" of the source is still declared before `__sinit`
exists. Today's probe result (declaration order governs placement) is not wrong
-- its TU simply had no `__sinit` competing for last place. **Do not record these
as two rules.** It is declaration order, with `__sinit` always last in line.

Confirmed at the relocation level, which is what makes this measured rather than
inferred: `__sinit`'s three `lfs` instructions hardcode displacements
`0x10/0x14/0x18` off a section-relative base (addend 0, read from `.rela.text`),
**byte-for-byte identical in every variant.** Inserting a named object anywhere
does not move them, so the triple is read from the wrong place and `sc_ForceList`
would be constructed with corrupted coordinates. Since antlion's `.text` is
already byte-identical to the target, the target's `__sinit` uses those same
fixed displacements -- so the `0x00-0x1c` layout is already exactly right and
cannot be disturbed. The trailing word has to appear at `0x1c` **without moving
anything before it.**

**The one shape that does reach the position:** a second genuine `__sinit`
consumer declared AFTER `sc_ForceList`. A probe object
(`static ExpProbeRec_t sExpProbe = {mVec3_c(11,22,33), 1}`) sequenced its floats
correctly after the triple. It is disqualified as written because it adds real
instructions to `__sinit`, growing `.text` from `0xb00` to `0xb40` -- and `.text`
is currently byte-identical, so that is a regression, not a fix. Its trailing
`u32` also did not pool (stored via `li`/`stw`), re-confirming that MWCC never
pools scalar integers, even inside `__sinit`.

**Currently being tested:** the same shape with the initialising code stripped --
`DECL_WEAK` or internal-linkage on a dynamically-initialised object declared
after `sc_ForceList`, exploiting the recorded rule that a deadstripped function's
pooled literals survive. This is grid's `DUMMY_ORDERING` idiom moved to the far
side of `sc_ForceList`. It is the first candidate that is on the correct side of
the `__sinit`-compiles-last boundary.

**Success criterion for any variant: `.rodata` gains the word AND `.text` stays
`0xb00` at 37/37 with `.ctors`/`.data`/`.bss` untouched.** Measure `.text`'s size
every time; that is what disqualified the probe.

## WM_DANCE_PAKKUN opened: 8/16 on its first authored round

`.text 0x161940-0x1622b0`, 16 functions, `wip/wm_units/agent_dance_pakkun/`.
`daWmDancePakkun_c : public dWmDemoActor_c`, `sizeof` `0x2e0`, every offset
verified against the compiler rather than read off the disassembly. All five
section bounds report clean:

```
.text 0x161940-0x1622b0   .data 0x44590-0x44768   .rodata 0x87f0-0x8830
.bss 0xfd80-0xfd98        .ctors 0x3d4-0x3d8
```

MATCH on first authoring: `classInit`, `draw`, `doDelete`, `resetStep`,
`updateStepAnim`, an unused stub, a one-line tail call, and the `sc_ForceList`
array-dtor thunk.

**That last one matched without being written.** `dWmLib::sc_ForceList` is a
namespace-scope static fully declared in the landed `d_wm_lib.hpp`, so including
the header emits the thunk identically -- the "a header static is emitted into
every TU that odr-uses it" rule. It also answers the open question of which
function owns the `.ctors`-registered static local: **none of ours, it is
automatic.** Worth remembering before anyone writes one by hand again.

### The unit's real strings, read out of the retail binary

`.data` file offset is `0x1d0c00`; add the address. The whole claim contains
only these printable runs:

```
0x44590  "F7C0"              <- the anonymous sc_ForceList pair
0x44598  "W7C0"
0x44620  "cs_wait"           <- animation name
0x4462c  "g3d/pakkun.brres"  <- resource path
0x44640  "pakkun"            <- model / archive name
```

Two uses: they remove the guesswork from `createModel`, and the claim opening
exactly on the two anonymous 5-byte strings independently confirms the `.data`
bound via the family rule in `check_bounds.py`. **Extract a unit's strings this
way before authoring anything that names a resource.**

### `+0x60` is the SECONDARY VTABLE POINTER. (Corrected -- see below.)

Measured, not inferred. `mHeap` -- `fBase_c`'s last named field -- ends at
`0x60`, and `sizeof(fBase_c)` is `0x64`. To rule out the secondary base reusing
that tail padding, compile the upcast and read the adjustment:

```cpp
cOwnerSetMg_c *getBase(dBase_c *d) { return d; }   // emits: addi r3, r3, 0x64
```

So `cOwnerSetMg_c` starts at `0x64` and **`0x60` is unclaimed padding with no
alignment reason to exist** -- strong evidence our `fBase_c` is missing a field.
dance_pakkun's ctor and `execute()` both store and double-dereference a `.data`
table address there.

**Do not add the field.** `f_base.hpp` is depended on by every landed unit; the
change belongs to the lead and only behind a full five-binary verify. Because the
hole already exists, naming it would not change `sizeof`, so the change is
plausible -- but it needs a corpus search first for another `this+0x60` access
that would show how the field is really spelled. Meanwhile the access is a raw
cast inside the unit's own `.cpp`, which touches nothing shared.

**The upcast-adjustment trick is worth keeping**: compiling a conversion and
reading its `addi` off the disassembly gives a secondary base's offset directly.

### Reminder that cost a round elsewhere today

`verify_anon`'s target names are a **nearest-neighbour heuristic, not
identifications.** This unit's table currently labels three different targets
`__ct__17daWmDancePakkun_cFv`, and on antlion_mng the same heuristic labelled a
function `clearAllModels` that is really `pickRevivedIndices` -- which I passed
to an agent as established fact and had to be corrected on. Read the target
disassembly.

## antlion_mng 16/22 -> 17/22. `processCutsceneCommand` MATCHES, and `R_2_1_*` is proven twice.

`processCutsceneCommand` (144 instructions, the unit's largest) went from 130
differing to **MATCH**. Three things did it, and only one was a compiler trick:

- `extern "C" bool R_2_1_19B170(daWmPlayer_c *);` declared in the `.cpp` and
  called with `daWmPlayer_c::ms_instance`. **Second independent confirmation of
  the `R_2_1_*` linking form this session** (course was the first).
- Two real logic errors read off the target: the first-frame `0x90` case calls
  `clearAllModels()`, not `setCutEnd()`; and `0x59`'s timer-expired branch calls
  `rebuildAllModels(false, true)` before `clearAllModels()`/`setCutEnd()`, not
  `setActive()`.
- A missing leading `if (cutsceneCommandId == -1) return;` the draft never had.

`pickRevivedIndices` 107 -> 40 with its logic fully reconstructed:
`candidates[9]`, a scan over `[0, 0xc0)` through `dCsvData_c::GetRouteFlag` with
mask `0x400`/`0x800` by world index, an early `false` when fewer than `count` are
found, then rejection sampling on `dGameCom::getRandom(foundCount)`, then a
re-validation pass against `dWmLib::getEnemyRevivalCount` forcing rejected slots
back to `-1`.

`reviveOnRoute` 46 -> 29 by **widening `pos`'s scope** -- declared once and
assigned per iteration rather than constructed per call. The target builds
`GetPos`'s return into its own stack temp and copies float-by-float into `pos`
for `playSound`'s by-reference argument instead of eliding the copy.

**Note this was NOT the inline-wrapper lever, and the agent checked rather than
assumed.** `playSound`'s 3-argument overload is the target's own direct callee,
confirmed from the mangled name -- there is no wrapper around a 5-argument form
and so no trailing default argument to spell or omit. The inline-wrapper rule is
powerful and has fixed six units, which makes it tempting to reach for on any
stack-slot difference; check for the wrapper's existence first.

### `dGameCom::getRandom` -- and a mangling trap worth the space

New declaration needed, in `include/game/bases/d_game_com.hpp` inside
`namespace dGameCom`, after `getRandomSeed()`:

```cpp
u32 getRandom(unsigned long max);
```

`getRandom__8dGameComFUl` is a straight tail call into
`cM_rand_c::ranqd1(unsigned int)` (`mr r4,r3; li r3,m_rnd@sda21; b ranqd1__...`),
so the return type is `ranqd1`'s own and passes through untouched.

**The parameter must be spelled `unsigned long`, not `u32`.** `u32` is
`typedef unsigned int` in `types.h`, which mangles to `Ui`; the target's symbol
ends `Ul`. The two are the same width on this ABI, the code is identical, and the
only symptom is a silent link failure. Parameter types ARE in the CFront mangled
name -- check the mangling, not the width.

### Measured negatives, so nobody re-runs them

`pickRevivedIndices`: swapping the `||` operand order made it 41 (worse); nested
`if` and explicit `goto` restructurings both compiled to byte-identical output at
40. Its residual includes one genuinely unexplained target quirk -- a redundant
second test of `excludeCurrent` at a control-flow merge, four target branches
against our two, which three different spellings could not reproduce.

`reviveOnRoute`: `pos` at function top, before the outer loop, and inside it all
give 29 -- placement does not matter once the scope is widened; explicit
copy-construction gives 40; removing `pos` and inlining the call gives 46, which
confirms the widening is what earned the 17 instructions.

## CORRECTION: `+0x60` is a secondary vtable pointer. No header change is needed.

I recorded `+0x60` as "unclaimed padding inside `fBase_c` with no alignment
reason to exist", called it "strong evidence our `fBase_c` is missing a field",
and reserved the header change for myself behind a full verify. **All of that was
wrong, and the correct answer needs no header change at all.**

It was settled by disassembling all 177 landed objects in `bin/compiled/` and
grepping for the offset. `d_base.o`'s own `dBase_c::dBase_c()` shows it in the
clear:

```
stw  r4, 0x60(r31)        ; r4 = &__vt__7dBase_c, computed fresh via lis/addi
...
lwz  r12, 0x60(r31)       ; then a virtual call straight back through it
lwz  r12, 0x4c(r12)
bctrl
```

It is **the secondary vtable pointer for the multiple-inheritance base**, written
by the compiler as part of ordinary construction. `dBase_c` inherits from both
`fBase_c` and `cOwnerSetMg_c`; the secondary vptr sits immediately before the
secondary base's own fields, which is exactly why the earlier measurement found
`mHeap` ending at `0x60` and `cOwnerSetMg_c` starting at `0x64`. The gap is not a
hole and it is not padding -- it is a slot the ABI requires and the compiler
fills.

**So plain C++ produces it.** The raw `*(void**)((char*)this + 0x60)` cast has
been deleted from dance_pakkun; `daWmGhost_c`'s landed ctor and `execute()` show
the identical shape, double-dereference included, written as ordinary member
calls. dance_pakkun's ctor went **46 differing -> 4** on this.

Two lessons, and the second is the one that cost the time:

- **An unexplained gap in a class layout is a vtable slot before it is a missing
  field.** Check the MI structure first; it is cheaper than any layout probe.
- **The corpus is evidence and it was the last thing anyone tried.** Three rounds
  went into offset probes, upcast-adjustment tricks and `sizeof` arithmetic --
  all sound technique, all pointed at the wrong question -- when 177 compiled
  objects were sitting in the tree with the answer written in them. **Grep the
  landed corpus for an offset before deriving anything about it.**

The `sizeof`/`offsetof`-via-template-error technique and the upcast-adjustment
trick are still good and stay recorded. They answered exactly what they were
asked; the questions were the problem.

## ANTLION IS LANDED. The blocker was our own tooling, not the source.

**`d_a_wm_antlion.cpp` is in `source/`, five binaries verify, progress
11.250% -> 11.286%.** Eighth unit. It had been parked for several rounds one word
short, and the missing word was never antlion's to emit.

### What was actually wrong

`make_filler_slice` gave every filler its section's NOMINAL alignment. A filler
is raw bytes lifted out of the original binary at a known address -- it has no
alignment requirement of its own, it just has to land where it came from. Giving
it `.rodata`'s nominal `8` made the linker round its start UP, silently moving
every byte after it.

That is invisible as long as every claim ends on an 8-aligned boundary, **and
every landed `.rodata` claim in this module does** -- all sixteen of them. So the
defect never showed up as a bug. It showed up as a rule: "a slice's `.rodata`
claim end must be 8-byte aligned, or the module comes out 8 bytes wrong through
quantisation." That rule was measured correctly and believed for good reason. It
was a description of our own tooling's limitation.

Antlion's real content ends at `0x85b4`, which is 4-aligned. The claim could not
be expressed, so the end was rounded to `0x85b8` and the linker zero-filled the
gap -- but the target has `00000001` there, not zero. Hence "one word short", and
hence seven measured attempts to make antlion emit a word it never owned.

The fix, in `tools/slicelib.py`:

```python
def natural_alignment(start: int, max_align: int) -> int:
    """The largest power of two up to max_align that divides start."""
```

applied to filler sections only. **For a filler already starting section-aligned
it returns the section alignment unchanged, so every existing module is
unaffected** -- confirmed by verifying 5/5 green with the change in and no slice
edits. Then antlion with `.rodata 0x8598-0x85b4` links exactly and the filler
supplies `0x85b4-0x85c0` from the original.

### Delete the 8-alignment rule

It is not a fact about the compiler or the game. Section claims may now end
wherever the unit's real content ends. Any other unit parked on a "section is a
few bytes wrong" symptom is worth re-measuring against this.

### What actually settled the ownership question

Not argument -- the TARGET's own compiled objects. `auto_fn_2_15B4E0_text.o` is
antlion's real `__sinit`, and disassembling it directly shows:

```
lis r5, lbl_2_rodata_8598@ha
lfs f2, 0x10(r5)   ; 0x85A8
lfs f1, 0x14(r5)   ; 0x85AC
lfs f0, 0x18(r5)   ; 0x85B0
```

It anchors at `0x8598` and never addresses past `0x85b4`. antlion_mng's own
`__sinit` anchors at `0x85c0`. Two relocations from shipped code, plus
`ForceInCourseList_t` being 0x24 bytes with `mNodePos` last, all put the cut at
`0x85b4`/`0x85c0` with twelve bytes belonging to neither unit.

**Read the target's own split objects.** Three rounds of source-shape probes and
a retracted relocation search were all trying to infer what two `lis`
instructions in `bin/dtkspl/` state outright.

### Next

**Sandpillar (66/66) was blocked behind antlion** and should now be attemptable.

## SANDPILLAR LANDED, on the first attempt, with no source change at all.

**Ninth unit. 11.286% -> 11.400%**, `d_basesNP` 1.704% -> 2.103%. 66 functions,
~7,400 code bytes -- the largest unit landed in this module so far.

It went in **immediately after antlion**, with its source untouched from the
state it had been parked in. Nothing about sandpillar was ever wrong.

### The dependency was real and is now discharged

Sandpillar's recorded blocker was `.text` growing by `0x150`: the object came out
`0x648` over its claim, which is normally benign weak symbols, except some were
being PLACED. Sandpillar is a heavy template user
(`sFStateID_c<daWmSandPillar_c>`, `sFStateFct_c<...>`, `sStateMgr_c<...>`) and was
the only landed provider of those instantiations, so there was nothing to
deduplicate against and its own copies had to be placed.

**A weak symbol defined only in an un-landed region gets placed.** Antlion
instantiates the same templates. The moment antlion landed, sandpillar's weak
copies had a target to dedupe against and the `0x150` disappeared on its own.

### Two units for one fix

The whole chain traces back to `make_filler_slice`'s alignment, one line of our
own tooling. Antlion was parked seven measured attempts deep trying to emit a
word it did not own, and sandpillar -- already 66/66 and byte-exact -- was parked
behind antlion. Neither unit had a defect.

**When a unit is parked on a symptom in a section rather than in a function, and
especially when its own functions all match, suspect the tooling and the claim
before suspecting the source.** Both of these were sitting finished.

### The parked-unit list is worth re-reading against this

Any unit parked on "section is N bytes wrong", or on placed weak symbols, should
be re-measured now:
- **castle** 18/20 -- parked on `.bss` +4 and a `__sinit` residual
- **koopa_castle** 16/17
- **course** 22/23 -- its residual is a register allocation inside a function,
  so this does NOT apply to it

## KINOKO_STAR LANDED — 9/9 on its first authored round. Tenth unit.

**11.400% -> 11.407%.** `.text 0x16bf70-0x16c150`, 9 functions, no shadow header
changes needed, no round two.

The method is the whole story: **the agent read the immediately-preceding landed
sibling AND its compiled object before writing a line.** All nine target
functions mapped 1:1 onto `d_a_wm_kinoko_red.cpp`'s nine by shape and vtable slot,
so the draft is red's file with the strings and class name swapped plus one new
member. That is the sibling-mapping stage doing exactly what it is supposed to do.

Two things it got right that were NOT guessable:

- **`vf7C`/`vf80` ordering.** Both are trivial `blr` bodies, so the disassembly
  cannot distinguish them -- this is the same ambiguity recorded as
  `d_a_wm_kinoko_1up.cpp`'s known defect in `check_vtable.py`'s docstring.
  Resolved by reading the TARGET VTABLE's slot order out of
  `auto_04_00044A68_data.txt`: slot 29 is `fn_2_16C060`, slot 30 is `fn_2_16C050`.
  The lower address must be defined first, giving `vf7C` then `vf80`, mirroring
  red. **When two functions have identical bodies, the vtable is the tiebreak.**
- **A new 4-byte member.** The ctor allocates `0x294` against red's `0x290` and
  explicitly zeroes `0x290`. Declared `int mUnk290` with `mUnk290(0)`. Nothing in
  the unit reads it back, so its purpose is left open rather than invented.

Also re-confirmed rather than re-derived: the `+0x60` store is the secondary
vtable pointer (see the correction above) and falls out of ordinary construction;
and the 4-byte `.rodata` shortfall is the same family-wide quirk red has, where
the linker zero-fills from the original.

### One bound was inferred, not measured

`.ctors 0x408-0x40c`. dtk does not split `.ctors` into per-unit objects, so there
are no symbols to check against. It was inferred from unbroken sequential
adjacency: kinoko_1up `0x3fc-0x400`, kinoko_base `0x400-0x404`, kinoko_red
`0x404-0x408`, then star. The agent flagged this as its one unmeasured bound
rather than presenting it as verified, which was the right call -- and the full
five-binary verify then confirmed it. **Flagging an inferred bound is worth more
than quietly asserting it; the build is what settles it.**

## A relocation proves a READ, not OWNERSHIP. And the displacement is what settles it.

antlion_mng's round 3 found a relocation from inside `fn_2_19B170` (a
`daWmPlayer_c` function, a different TU) targeting `0x85c4`, and concluded the
`0x14`-byte block there belongs to that TU rather than antlion_mng. **That
inference does not hold, and the correction generalises.**

`.rodata` is one shared section. A TU can read a constant another TU defines --
that is what an `extern` declaration produces. So a relocation tells you who
READS an address. It never tells you who EMITS it.

**What does settle ownership is the compile-time displacement.**

```
our    __sinit:  reads mNodePos at pool base + 0x20
target __sinit:  reads mNodePos at pool base + 0x48
```

That displacement is baked in when the TU is compiled, relative to **that
object's own** `.rodata` contribution. For the target's `__sinit` to carry
`+0x48`, the target's object must itself contain `0x48` bytes ahead of
`mNodePos`. It cannot be `+0x48` into another object's data, because the compiler
has no idea where another object will be placed. The addresses agree: the
target's `__sinit` anchors at `0x85c0`, and `0x85c0 + 0x48 = 0x8608`, which is
exactly where the `{2160.0, -30.0, -478.0}` triple sits -- the only copy in the
module preceded by `1` and followed by `9`.

So antlion_mng owns `0x85c0-0x8614`, the `0x14` block included, and its
`__sinit` residual is `0x28` bytes of leading data the TU is not declaring. **A
gap in our source, not a cross-TU dependency.**

### The rule, stated generally

- **Relocation into a range => somebody reads it. Says nothing about ownership.**
- **A compile-time displacement in a TU's own instruction => that TU emits
  everything up to that displacement.** This is the strongest ownership evidence
  available and it is available from the target's own split objects.

Note how this sits with the antlion result, which looks superficially opposite.
There, the target's `__sinit` anchored at `0x8598` and never used a displacement
past `+0x18`, so antlion demonstrably did NOT own `0x85b4`. Same test, opposite
answer, both times decisive. **Read the displacements out of the target's own
compiled `__sinit`; it is the one measurement that answers this question.**

### Also measured this round

- **The inline-wrapper lever needs TWO call sites.** It routes an outer consumer
  through a wrapper while a loop call bypasses it -- with a single call site there
  is nothing to pair against. `reviveOnRoute` calls `GetPos` once, so the lever
  cannot apply, independently of whether a wrapper exists. Adding a trailing
  default-argument overload to test it fails to compile at all:
  `(10199) ambiguous access to overloaded function`. Both halves recorded so the
  idea is not re-proposed.
- `pickRevivedIndices` 40 -> 39 by widening `playerPoint`'s scope above the
  early return. Splitting the condition into a separate `bool reject = ...`
  statement gives **44, worse**.

## dance_pakkun 8/16 -> 9/16: `execute()` MATCHES

The unit's largest function closed on three fixes, each compiled and diffed
rather than reasoned about:

- `mBgmSync->execute()` lands on vtable slot `0xc`, confirmed by isolated-compiling
  a one-line `b->execute()` test rather than counting slots by hand.
- **The `__ptmf_scall` really was a pointer-to-member-function call.**
  `lbl_2_rodata_87F8` is `0xc` bytes -- one CodeWarrior generalised PTMF entry, a
  3-word stride. Declared `typedef void (daWmDancePakkun_c::*ProcFunc_t)();` with
  a one-entry table, called as `(this->*sProcTable[m_2bc])()`. The index is only
  ever reset to zero across all 16 functions, which is consistent with a
  one-entry table, so the shape is probably right even though the target function
  is still unknown and is flagged as a placeholder.
- Two scheduling fixes: one value's two uses needed **two separately named
  locals** (one per occurrence -- not shared, not absent) to match the target's
  call-preserved-register shape; and a singleton pointer needed an explicit local
  positioned as the SECOND statement specifically. Three positions were tried and
  only one matched.

### `create()`: formula correct, parked at 49 on three instructions

The `mParam`-indexed lookup was derived exactly from the shift/mask sequence:
`(mParam & 0xFF)` indexes the `u32` region at table `+0x34` with stride 4 -- **not**
the six 8-byte `{float, u16, u16}` records, which this code never touches. Since
all three `u32` entries are identically `0x00020000`, the formula yields the same
result whichever is selected, which is a useful self-check that the indexing is
right.

The residual is three instructions: our `&sStepTable` address is computed early
and hoisted above the `new dWmBgmSync_c()` call, forcing a saved register the
target does not need -- the target computes it fresh and late, in volatile
registers only. Three variants (named local, no local, differently-scoped local)
all failed to suppress the hoist. **All three varied the LOCAL. The untried axis
is statement ORDER relative to the allocation** -- if the target computes the
address late, the source likely reads the table after the `new`, not before.

**Note the count went 38 -> 49 and this was NOT a regression.** The old 38 was
measured against a mislabelled target; 49 is the first fair comparison. Second
time this unit has been bitten by `verify_anon`'s nearest-neighbour labels.

### Why `createModel`'s register anchoring should be solvable

Its 76 differing are entirely register allocation -- every call and argument
already matches. The target holds five values live via `_savegpr_27`; ours needs
three, because our string references each pull a fresh anchor. The reason the
target can share one is visible in the retail `.data` (unit base `0x44590`):

```
0x44628  (+0x98)  pointer, relocated to "cs_wait"
0x4462c  (+0x9c)  "g3d/pakkun.brres"    (17 bytes with terminator)
0x44640  (+0xb0)  "pakkun"              (7 bytes)
```

**All three lie within `0x18` bytes of each other**, so one anchor at `+0x98`
reaches all of them by displacement. The sharing is a consequence of ADJACENCY in
the emitted data, not of an addressing trick -- so the fix is a source shape that
emits a pointer and two literals contiguously in that order. Inline literals are
placed where the compiler chooses; named `static const char[]` arrays are placed
at their declaration point, which is controllable.

## SINKSHIP LANDED — 11/11 on its first authored round. Eleventh unit.

**11.407% -> 11.424%.** `.text 0x179380-0x1797e0`, 11 functions, no shadow header
changes, no stubs, no second round. `daWmSinkShip_c : public dWmObjActor_c`.

### The bounds were self-checking, which is the standard to aim for

Every bound was derived independently from symbol-boundary and ownership
evidence, and then found to **exactly close the gap between two already-landed
neighbours**: `d_a_wm_sandpillar.cpp` ends at `.data 0x47180`, `.rodata 0x8f98`,
`.bss 0x10290`, `.ctors 0x42c`, and `d_a_wm_smallcloud.cpp` begins at exactly
those four addresses. Two independent derivations agreeing is much stronger than
either alone. `.data` also opens on the two anonymous 5-byte `sc_ForceList`
strings per the family rule.

### The base class was settled from the vtable, not from the disassembly

`dWmObjActor_c`, not `dWmDemoActor_c`. Confirmed from the target vtable dump: the
`GetActorType` slot resolves to the imported `GetActorType__13dWmObjActor_cFv`,
and two trailing slots to `vf74`/`vf78__13dWmObjActor_cFv` -- all three
`dWmObjActor_c`-only symbols. **A store initially misread as a new field turned
out to be `dWmObjActor_c`'s own in-class `mResNodeIdx(-1)` initialiser**, which
the correct base class explains for free. Getting the base right first removes
phantom members.

### The one real fix: dead stores that are correct

`calcModel()` began at 40/44 differing by passing `mPos`/`mAngle` straight to
`mMatrix.trans()`/`ZXYrotM()`. The target stages them through locals first:

```cpp
mVec3_c pos = mPos;
mAng3_c angle = mAngle;
```

This is the already-recorded "dead stores that are correct" idiom from
`dCourseSelectGuide_c::PlayerIconSet`, and the landed `daWmSmallCloud_c::calcModel()`
uses it too. Straight to MATCH. **Check the landed sibling before assuming a
by-value argument can be passed directly.**

### Confirmations worth having in writing

- **`dWmLib::sc_ForceList` and `dWmLib::c_StartPointKinokoHouseID` are forced into
  every TU that includes `d_wm_lib.hpp`, with zero source-level reference needed.**
  Nothing in `daWmSinkShip_c` reads either, yet the include alone reproduces
  `__sinit` and the trailing array destructor byte-exact. Both are non-`extern`
  namespace-scope statics with non-trivial construction. This is now confirmed on
  three units; stop hand-writing these.
- The model name `"cobSunkenShip"` was read out of the target's own `.data`
  (`lbl_2_data_471D0`) and is reused for both the archive lookup and the model
  name -- no per-world `sprintf`, unlike `daWmSmallCloud_c`.
- The clip-sphere radius was recovered by decoding `lbl_2_rodata_8F98`'s raw bytes
  (`0x42C80000` = 100.0f) rather than guessed. Note the pool merges the unit's own
  constant at offset 0 -- emitted first because `create()` compiles before
  `__sinit` -- with `sc_ForceList`'s `mNodePos` floats at 4/8/c. Consistent with
  declaration-order placement and `__sinit` last.

## antlion_mng 17/22 -> 18/22: `__sinit` MATCHES, and the ownership argument is confirmed empirically

The disputed `.rodata` really was this TU's to declare. Two named objects closed
it, and **the confirmation is better than the argument was**: adding the first
object shifted the compiled displacement from `+0x20` to `+0x38` -- exactly its
own size, `0x18`. That verifies position and size independently of knowing what
the object means. The second closed the remaining `0x10`, taking the
displacement to `+0x48` and `__sinit` straight to MATCH.

```cpp
// @unofficial -- bytes read from the retail binary, semantics unidentified.
extern const unsigned int g_unofficial_85C0[6] = {9, 2, 0x00010100, 0, 0x003c0014, 0};
// declared immediately before sProcTable

const int unofficialTable85F8[4] = {0, 1, 0, 1};
// declared inside reviveOnRoute, right after worldIndexTable
```

The first keeps `extern` because `fn_2_19B170`, in `daWmPlayer_c`'s TU, reads its
`+4` word through a confirmed relocation -- which is exactly the
declared-here-referenced-there pattern predicted when the earlier
"the relocation proves another TU owns it" reading was overturned.

**Neither object's meaning is identified, and neither was invented.** The bytes
come straight out of the retail binary and are marked `@unofficial`. That is the
right trade when the position and size are provable but the semantics are not --
several landed units already carry data on the same footing.

### The rule this validates, now with a positive case

**A compile-time displacement in a TU's own instruction proves that TU emits
everything up to it.** Antlion gave the negative case (its `__sinit` never
reached past `+0x18`, so it did not own the bytes below); antlion_mng gives the
positive one (`+0x48` could only be computed by a compiler that laid out both
ends of that span itself). Same measurement, opposite answers, decisive twice.

And the practical test for a candidate object: **add it and check the
displacement moves by exactly its size.** That confirms placement without needing
to know what the data is for.

### Still open on this unit

`pickRevivedIndices` 39, `reviveOnRoute` 29, and the two 8s. The
`excludeCurrent` four-branch merge-point quirk remains unexplained after three
rounds; both compare instructions are confirmed against the same parameter
register, so "the two tests compare different things" is now ruled out too.

## `verify_anon`'s pairing is GREEDY and CONSUMING. One bad function poisons the rest.

Worth stating on its own, because it has now misled three separate rounds.

The tool pairs each target against the **closest remaining draft function by
instruction content**, consuming drafts as it goes. So a draft function that is
badly size-mismatched does not merely report a bad number for itself -- **it can
consume the draft that belonged to a different target**, and every target after
it is then paired against leftovers.

dance_pakkun hit this cleanly: `calcModelFor` is 47 instructions in the draft
against the target's 101 (deliberately left unwritten), and `__sinit` was
consequently reported as "50 differing vs ~calcModelFor" -- a meaningless number.
The agent refused to read content off that comparison, which was correct. **A
number you cannot trust is worse than no number.**

### Consequences to internalise

- **Only `MATCH` lines are trustworthy without cross-checking.** Those are exact
  by construction. Every `differing vs ~` figure is a heuristic pairing and can
  be attached to the wrong target.
- **A count can move without the code changing**, if a neighbouring function's
  size changed. dance_pakkun's `createModel` went 76 -> 75 and `create` went
  38 -> 49 across rounds partly for this reason; both were re-baselined as "first
  fair number" rather than treated as regressions.

### The fix: diff by explicit name

```
python wip/wm_units/agent_course/difftool.py <target.txt> <draft.txt> <target-fn> <draft-fn>
```

Explicit target name, explicit draft name, no heuristic, no consumption, no
cascade. **Use it for any function whose `~name` looks wrong**, and always when
another function in the unit is a stub or is unwritten.

The tool's own docstring already warned that pairing is greedy and that two
functions with identical bodies can be paired to each other's targets. The part
that was not obvious, and is now: the damage is not confined to the mis-paired
function.

## dance_pakkun: `createModel` 76 -> 70, and a guard block found that may unblock castle

### The register mechanism, confirmed by measurement rather than by count

`createModel`'s residual was diagnosed as missing persistent anchors. Making
`mChrAnim[0]` a persistent `m3d::anmChr_c &anim` reference, instead of
recomputing it at each of its five use sites, gave it a dedicated register reused
across all five calls -- matching the target's `addi r29, r27, 0x22c` followed by
repeated `mr r3, r29`, instruction for instruction.

```
target                        bl _savegpr_27   -> 5 registers (27-31)
struct anchor only            individual stw   -> 2 registers
+ persistent anim reference   individual stw   -> 3 registers   (76 -> 70)
```

**Track the saved-register level, not just the differing count.** It moves first
and it tells you the mechanism is working before the count follows.

Still short one anchor: the target keeps a `.rodata` play-mode table live across
calls, and a `.rodata` table can never share the `.data` struct's register. An
early named pointer to it changed nothing -- **source position of a
side-effect-free address computation does not control this compiler's scheduling**,
now confirmed via a named local as well as via statement order. Parked at 70.

### A suggestion of mine that was correctly refused

I proposed reading the play-mode table inside a loop so its base would have to
survive the calls. **The target has no loop at all** -- no back-branch anywhere in
the function. Fabricating one to exercise the mechanism would have been an
unverified claim about source shape presented as a test. The agent used a
persistent reference instead: same "value must survive a call" lever, no invented
structure. **Test the mechanism, not the guess.**

### `__sinit`: a guard block, and it may be worth more than this unit

Measured directly with `difftool.py` (bypassing the poisoned pairing): target 52
instructions, draft 33, **51 differing**. The unconditional half matches
call-for-call -- `sc_ForceList` construction, `__register_global_object`,
`c_StartPointKinokoHouseID` assignment. What is missing is an entire conditional
block: a guard-byte test at `lbl_2_bss_FD80+0x10` that, **on first execution
only**, patches `sStepTable`'s `{dx, dy, dz}` fields from `lbl_2_rodata_87F0`.

**A guard byte plus once-only initialisation is the signature of a function-local
`static`.** The guard living in this unit's own `.bss` says the owner is in this
TU.

**This matters beyond dance_pakkun.** `castle` is parked at 18/20 needing exactly
"guard present, registration absent", and 26 measured shapes across two units
have failed to produce a guard. dance_pakkun's target **contains a working
example of one.** Whatever source shape produces this block is the answer castle
has been looking for.

### `calcModelFor`: instruction reversed, author it

It was left alone as the worst ratio in the unit -- reproducing unknown rotation
maths. That was right when it was one function among many and is wrong now: at 47
draft instructions against the target's 101, **it is the function corrupting
`verify_anon`'s greedy pairing for the whole unit**, which is what made `__sinit`
report a meaningless 50. It is simultaneously the largest function still open and
the obstacle to trustworthy measurement everywhere else.

## antlion_mng PARKED at 18/22 — four walls, each with converging independent rewrites

Final state: 18/22, `__sinit` and `processCutsceneCommand` both MATCH. Four open:
`pickRevivedIndices` 39, `reviveOnRoute` 29, `rebuildAllModels` 8,
`clearAllModels` 8.

### The register defect was mis-described for several rounds

It is **not a three-way rotation.** It is a clean **two-element swap**: `slot`
already matches in both functions, and only `map` and `base` have their register
numbers exactly exchanged -- identically in both functions. Everyone including me
had been repeating "3-way rotation" and attacking the wrong description.
**Re-characterise before attacking; it is worth more than another attempt.**

The likely mechanism, stated as a hypothesis: MWCC's allocator prefers `base` (a
plain int accumulator) over `map` (a two-instruction `lis`/`lwz` pointer load) for
the higher-numbered callee-saved register, and no declaration-order or
expression-shape lever reaches that preference.

Measured this round on `clearAllModels`, none carried to `rebuildAllModels`
because none helped: reordering declarations to `idx; map; base; info` (no
change, byte-identical); folding `base` into the `for`-increment, the lever that
worked for `reviveOnRoute`'s accumulator (**13, worse**); dropping the cached
`map` local and reading `daWmMap_c::m_instance` fresh at both sites (**29, much
worse**); hoisting `map` to the top of the function (no change).

### `pickRevivedIndices`: FIVE spellings, byte-identical output

`||`, nested `if`, `goto`, a separate `bool`, and an explicit two-`continue` form
with genuinely separate tests -- **all normalise to the same 2-branch shape under
`-O4`.** A ternary gives 43, worse.

That is a complete result about the optimiser rather than a failure: **the
target's four-branch merge shape is unreachable from the condition's spelling.**
Whatever produces it is not a way of writing the condition.

### `reviveOnRoute`: the stack anchor did not move

An unrelated `mVec3_c` local declared before `pos` was dead-code-eliminated
entirely (confirmed by identical instruction count); copying `pos` into a second
named local before passing it gives **47, worse**; reordering `pos` against
`antlion` changes nothing.

### Why this is a park and not a failure

All four items now have **multiple independently-shaped source rewrites
converging on identical compiled output.** That is much stronger evidence of a
real wall than one failed attempt per item, and it is the standard this file
should hold before recording anything as parked.

## castle 18/20 -> 19/20: `__sinit` MATCHES. The 26-shape wall is BROKEN.

And the lever is new, general, and worth more than the unit.

### NEW LEVER: brace-init pools an aggregate constant; field-by-field assignment does not

Staging the guarded write through a local built with **brace initialisation**
gives 25 differing **and regresses two other functions** (18/20 -> 16/20) --
the aggregate literal perturbs the `.rodata` pool order for the whole TU.
Building the same local by **individual field assignment** gives **MATCH**.

```cpp
Vec3Pod_t v;  v.x = ...; v.y = ...; v.z = ...;   // MATCH
Vec3Pod_t v = { ..., ..., ... };                 // 25 differing, and breaks two neighbours
```

Same values, same local, same writes. **An aggregate initialiser becomes a
pooled `.rodata` constant; field-by-field assignment stays immediate.** Reach for
this on any residual that looks like pool-order perturbation, and note the damage
is not local -- it moved functions elsewhere in the unit.

### My hypothesis was wrong, and it was killed with ground truth

I proposed the object was a **function-local static**, on the reasoning that a
guard byte is that construct's signature. **Wrong, and disproved directly:**
disassembling `auto_fn_2_15FAE0_text.o` shows castle's `__sinit` is referenced
from `.ctors` at `0x3c8-0x3cc`, so it runs exactly once from the constructor
table and never from a call site. The agent checked the thing I flagged as
possibly fatal, found it fatal, and said so instead of forcing the hypothesis.

### What the wall actually was

Two mistakes compounded, neither of them about registration:

1. The unconditional `.bss+0xc` write in **both** castle's and koopa_castle's
   `__sinit` is **`dWmLib::c_StartPointKinokoHouseID`'s own per-TU dynamic
   initialiser**, already supplied by the shared header -- not a field of any
   castle-local struct. Giving the guard-holder its own `mStartID` field produced
   a second, wrong load/store. Removing it took 31 -> 25 and made the guard
   read/branch byte-exact.
2. **The guarded write never calls `mVec3_c`'s constructor** -- it is three plain
   `stfs`. So the registration question was a red herring for this object.

Also `bool mDone` -> `s8 mDone` to match the target's `extsb.` idiom (25 -> 24),
and a trailing `u8 pad_unofficial[7]` after the guard byte to take `.bss` from
`0x11` to `0x18`. Those 7 bytes' content is not recoverable from castle alone and
is marked `@unofficial`.

**Castle's `.bss` is exactly `0xfd48-0xfd60`**, cross-checked against landed
neighbours cannon (`0xfd38-0xfd48`) and cloud (`0xfd60-0xfd70`). The registration
node for `sc_ForceList` and castle's own guard both live INSIDE that span --
earlier probes had assumed they were adjacent to it.

### State

**19/20, SECTIONS CLEAN, BOUNDS PLAUSIBLE, VTABLE CLEAN.** Only
`getKoopaShipStopPos` open at 6 differing -- an `f0`/`f1` pair misassignment
through the hidden result pointer of a by-value `mVec3_c` return. Pre-existing,
a different problem, untouched this round to keep the `__sinit` result isolated.

### Try this on koopa_castle immediately

koopa_castle is parked at 16/17 with only its `__sinit` open at 13 differing, on
what was recorded as the same construct. **The brace-init lever and the
"`c_StartPointKinokoHouseID` is the header's own initialiser, not your field"
correction both apply directly.**

## WM_NOTE LANDED — 13/13 on its first authored round. Twelfth unit.

**11.424% -> 11.450%.** `.text 0x175f90-0x176630`, 13 functions, no stubs, all four
checks clean, function definition order correct on the first try.
`daWmNote_c : public dWmDemoActor_c` **directly** -- no `dWmObjActor_c` in
between, unlike sinkship. Settled from the vtable before authoring: `GetActorType`
resolves to `dWmDemoActor_c`'s, and the vtable is 26 slots against sinkship's 28
(no `vf74`/`vf78`).

Its `processCutsceneCommand` is `0x224` bytes -- the largest single function
matched on a first round in this module.

### Two shared-header additions, applied by the lead behind a full verify

Both purely additive declarations. `include/game/cLib/c_lib.hpp`:

```cpp
float addCalcPos(mVec3_c *pos, const mVec3_c &target, float speed, float accel, float max);
```

`include/game/bases/d_a_wm_map.hpp`:

```cpp
mVec3_c GetPos(int nodeIdx);
```

Both needed `syms.txt` entries, since a REL calling into the DOL resolves through
it and neither symbol was present:

```
GetPos__9daWmMap_cFi=0x80100310
addCalcPos__4cLibFP7mVec3_cRC7mVec3_cfff=0x80160C20
```

The agent proposed both in shadow copies with the mangled names and the evidence,
and did NOT touch the real headers. That is the correct division and it is what
made them safe to apply.

**It also declined to propose a third one.** A camera-setup block writes five
fields at offsets past the current header's padding in a class three landed TUs
already reference. Rather than restructure that header, it used a local `u8 *`
offset cast confined to the `.cpp`. Right call -- a header used by landed units is
not worth destabilising for one block.

### Levers confirmed or newly recorded

- **`switch`, not `if`/`else if`**, for the dispatch -- **third independent
  confirmation** of this lever.
- **Do not reach for an accessor when the target shows a raw store.**
  `mIsCutEnd = true;` is a direct field write; routing it through `setCutEnd()`
  added ~15 spurious instructions per call site across three sites.
- **An unnamed temporary and a named local land on DIFFERENT stack slots.**
  `cLib::addCalcPos`'s target-position argument needed `mVec3_c(mTargetPos)` as an
  unnamed temporary; a named local matched the target's implicit temp's slot
  incorrectly.
- **An undecompiled callee's argument count is easy to under-read.**
  `fn_2_172AE0` takes THREE arguments, not one -- `r4=1, r5=-1` are set well before
  the call and stay live, which reads like dead immediate loads. Declared
  `extern "C" int R_2_1_172AE0(daWmMap_c *, int, int)`; the two ints' meaning is
  unresolved.
- A ~2-instruction load-order swap resisted every declaration-order permutation
  and **stopped mattering once the four real defects were fixed.** Worth
  remembering before spending a round on a scheduling residual: fix the semantic
  defects first and re-measure.

## The castle guard shape TRANSFERRED. dance_pakkun `__sinit` 51 -> 14, size now exact.

Castle solved the guard construct; dance_pakkun's agent read castle's file and
applied it, with one adaptation for a real structural difference: its guard
patches three fields of a **pre-existing global** rather than one field of the
trigger's own array, so the three fields were grouped into a `Vec3Pod_t` member
and written with a single struct assignment, reproducing the target's 3-`stfs`
store the way castle's `mOffset = offset;` does. `const` had to come off the
patched global since it is now written at runtime.

Everything else was copied verbatim and worked: `s8 mDone`, `u8
pad_unofficial[7]`, `if (!mDone) { ...; mDone = true; }`, and the staged local
built **field by field, never brace-init**.

**Cross-unit transfer is worth looking for explicitly.** Two units were parked on
the same construct; one round on one of them unblocked both.

### NEW TRAP: placeholder constants that are accidentally EQUAL

The first attempt used `0.0f` for both new constants and stopped at 30 differing,
**one `lfs` short** -- MWCC folded the two identical literals into one shared
register. That reads exactly like a structural defect. Giving them distinct
placeholder values (`1.0f`/`2.0f`) took it to size-exact immediately.

**While a shape is still being established, always make placeholder constants
DISTINCT.** Equal placeholders silently change codegen and send you looking for a
missing instruction that was never missing.

### Cost, reported rather than buried

`startStep` regressed 13 -> 15: accessing the patched fields through the new
nested struct compiles slightly differently from the old flat fields. Two
instructions to close 51 on a 52-instruction function is a good trade, but it is
a real measured cost and was reported as one.

### Remaining: a 28-byte pool drift, and the fix is to stop guessing constants

`__sinit`'s residual is a pool-offset drift (`0x38` vs `0x54`) from several rounds
of independent placeholder structs all competing for the whole-TU float pool. The
unit's real `.rodata` `0x87f0-0x8830`:

```
0x87f0  250.0f      0x87f4  0.0f
0x87f8  { 0, -1, <reloc> }   <- the 0xc-byte PTMF entry (sProcTable)
0x8804  1 (integer)
0x8808  1.0f        0x880c  0.0f
0x8810  2.0f        0x8814  0.0f
0x8818  180.0f
0x881c  2160.0f  0x8820  -30.0f  0x8824  -478.0f   <- sc_ForceList mNodePos
0x8828  -68.0f      0x882c  0.0f
```

**Use the pool's ORDER as a specification for declaration order**, since placement
follows declaration order with `__sinit` last. Replacing each placeholder with its
real value should resolve the drift as a consequence rather than by attacking it.

### The PTMF idiom is a family pattern, found independently twice

`{0x0, -1, <function>}`, `0xc` bytes, dispatched through `__ptmf_scall` -- the
CodeWarrior non-virtual pointer-to-member encoding. dance_pakkun and anchor's
agents each found it in their own unit without knowing of the other. Model it as
`typedef void (Class_c::*Fn_t)(); static const Fn_t scTable[N];` called as
`(this->*scTable[idx])()`. On anchor it unblocked three functions at once.

## anchor: the definition-order lever works, and dtk's symbol names hid a real override

Order violations **13 -> 3** by applying the recorded rule: an explicit
out-of-line override joins the definition-order batch even with a body identical
to the inherited default, while a purely inherited virtual defers to a block at
the end. `doDelete()` declared as a strong override right after `draw()` moved to
its correct slot immediately.

### dtk mislabels weak base-class symbols by BYTE CONTENT

`vf74`/`vf78`/`GetActorType` in anchor's range are labelled
`__13dWmObjActor_c` in the symbol map. **They are `dWmActor_c`'s, inherited
through `dWmDemoActor_c`.** dtk matched them by byte content against
already-landed units of the `dWmObjActor_c` family -- same bytes, wrong
attribution. Anchor's base is `dWmDemoActor_c`, proved from its constructor
calling `__ct__14dWmDemoActor_cFv` directly.

**And the mislabel hid a real defect.** `GetActorType`'s target body is
`li r3, 0x2; blr` -- `ACTOR_MAP_OBJECT`, not the inherited `ACTOR_MAP_DEMO` (1).
It is a genuine behavioural override, and checking only the symbol name would
have left it as a permanent "1 differing". **Read the target's BYTES, not dtk's
name for them.**

### `0xbf8` is not `sizeof(dWmMapModel_c)`

`setNodePos` computes `base + idx*0xbf8 + 0x1a0` and calls
`setAnchorShadow__13dWmMapModel_cFb`. The mangled name makes `this` a
`dWmMapModel_c *`, so **the `dWmMapModel_c` is the thing at `+0x1a0`, and `0xbf8`
is the size of a LARGER containing struct.** Our header models
`dWmMapModel_c { u8 mPad[0xbf8]; }`, which is the wrong object.

**The header change was proposed correctly and declined deliberately.**
`dWmMapModel_c` is referenced by three landed units and changing its size risks
binaries that currently verify -- not worth it for one function in a unit with
other blockers. The call is to be modelled with a cast confined to the unit's own
`.cpp`, the same way WM_NOTE handled its camera-field block and landed cleanly.
Recorded here so whoever eventually corrects that header has the evidence.

### Open hypothesis for the last 3 order violations

The target puts `GetActorType` at `0x15abb0`, at the very END of the weak cluster,
*with* a non-default body. A strong out-of-line override cannot land there. **An
in-class inline virtual is weak and defers to the end-of-TU block while still
carrying its own body** -- the same lever that landed the whole kinoko family via
`getModelName()`. Being tested now, along with declaring `clearCutEnd`/`vf74` as
in-class inline overrides to see whether a *declared* weak function's relative
order within the cluster is controllable at all.

## castle 19/20: `getKoopaShipStopPos` is a SCHEDULING wall, measured exhaustively

Not closed, and now properly evidenced as a wall rather than an unexplored gap.
Castle stays 19/20, all four checks clean.

**All six component permutations are now measured**: `zyx` 6 (best, baseline),
`xyz`/`zxy` 8, `xzy`/`yxz`/`yzx` 10, plus 13 and 7 already on record. The sweep is
complete, not sampled.

**The by-value return is confirmed correct from the ABI, not from the mangled
name.** An output-reference form (`void getKoopaShipStopPos(mVec3_c &out) const`)
is **worse at 8**, and it contradicts the target's own register roles: the target
writes `r3` (result) and reads `r4` (`this`), which is CodeWarrior's
hidden-return-pointer-first convention for a class returned by value. An output
reference puts `this` back in `r3`. That is a positive confirmation of the
declaration, not merely a failed variant -- and it is how a return type should be
settled, since return types are absent from the mangled name.

Also measured: `offset` as a plain pointer instead of a reference is
byte-identical at 6; as a value copy it is **14** (the copy costs real
instructions); flipping x's addend order stays 6 and only swaps which of `f0`/`f1`
holds which operand.

**The residual is two instructions and the same two in every shape**: x's operand
load order, and the z-store being scheduled immediately after y's add while the
target defers it past x's add. Both candidate instructions are ready at that
point in both versions -- zero dependency difference -- and MWCC's list scheduler
simply picks differently. Same class as the saved-register-assignment residual
already recorded as not source-addressable, manifesting as schedule order.

**One axis left untested by that sweep**, being tried now: the six permutations
varied the order the components are COMPUTED; the residual is about the order
they are STORED. The recorded "decouple declaration order from usage order" lever
separates exactly those two. If varying store order independently of compute
order changes nothing, the wall is confirmed on both axes.

## The in-class inline lever generalises beyond the kinoko family

anchor's `GetActorType` needed a NON-DEFAULT body (`ACTOR_MAP_OBJECT`, not the
inherited `ACTOR_MAP_DEMO`) **and** a position at the very end of the weak
cluster. Those look contradictory: a strong out-of-line override carries your body
but joins the definition-order batch and lands early.

```cpp
virtual int GetActorType() { return ACTOR_MAP_OBJECT; }   // in-class inline
```

**An in-class inline virtual is WEAK -- it defers to the end-of-TU block -- while
still carrying its own body.** Content and position both correct. This is the same
lever that landed the whole kinoko family via `getModelName()`, and it is now
confirmed to be general rather than a quirk of that family.

**Reach for it whenever a function needs a non-inherited body AND a late
position.** The out-of-line form can only ever give you one of the two.

### And a declared weak function's POSITION is influenced by source

A second, weaker result from the same round, worth recording because it
contradicts the natural assumption: declaring `clearCutEnd`/`vf74` in-class inline
**moved them** within the weak cluster. A purely inherited virtual is inert -- you
have nothing to order -- but a *declared* in-class inline is not.

Declaring only three of the cluster's six did not reproduce the target's sequence:

```
draft:   setCutEnd, checkCutEnd, vf74,        clearCutEnd, GetActorType, vf78
target:  setCutEnd, clearCutEnd, checkCutEnd, vf78,        vf74,         GetActorType
```

The three declared ones moved; the two left purely inherited are exactly the ones
still out of place. **Declaring the COMPLETE cluster in the target's address
order** is the outstanding test. If a declared subset moves but the full set still
comes out wrong, cluster-internal ordering is not source-addressable and should be
recorded as such.

**Measure order from the draft's own function sequence, not from `verify_anon`** --
its pairing mislabels one-instruction bodies against each other by content.

### Modelling an unlanded callee without touching a shared header

anchor needed `setAnchorShadow__13dWmMapModel_cFb` on a class whose header models
the wrong object (`0xbf8` is the outer struct's size, not `dWmMapModel_c`'s). The
call is modelled with a cast confined to the unit's own `.cpp`:

```cpp
u8 *node = reinterpret_cast<u8 *>(daWmMap_c::m_instance)
         + daWmMap_c::m_instance->currIdx * 0xbf8 + 0x1a0;
setAnchorShadow__13dWmMapModel_cFb(node, true);
```

with the evidence recorded in a comment. **That is how a finding survives a unit**
-- the header stays untouched, three landed units stay safe, and the next person
to correct it inherits the reasoning instead of re-deriving it. WM_NOTE used the
same approach for a camera-field block and landed 13/13.

## castle PARKED 19/20 and koopa_castle PARKED 16/17 — both walls now exhaustively measured

### castle `getKoopaShipStopPos`: both axes closed

The outstanding test was whether the earlier sweep had varied the wrong thing.
It had not. With compute order held fixed at the known best (`zyx`), **all six
STORE-order permutations were swept independently and all six give the identical
6 differing.** Store order has zero effect when compute order is fixed.

Combined with the exhaustive compute-order sweep, **both axes are fully measured
and neither moves the residual.** Twelve permutations plus the signature and
access-pattern variants. The residual really is MWCC choosing between two
simultaneously-ready instructions with no dependency difference on either side.

**Permuting the right thing correctly and getting nothing is a much stronger
result than permuting the wrong thing.** This is a park with the axis my
instruction distinguished explicitly closed.

### koopa_castle: 22 measured shapes, and no free win from castle

Both castle findings -- `s8 mDone` rather than `bool`, and the
`c_StartPointKinokoHouseID`-is-the-header's-own-initialiser correction -- were
**already present in koopa_castle's draft**, independently discovered by an
earlier round on the same evidence. The agent read the unit's section before
touching anything and reported that rather than re-deriving it.

Two genuinely new shapes were then tried, both regressing to 33 with the frame
collapsing `-0x40 -> -0x30` and length `58 -> 55`: an explicit local pointer
declared after the first field write with the remaining five writes through it,
and a mixed form splitting the first vector into scalar stores while leaving the
second as a constructor call. Both **break the register sharing across the two
vectors** (they stop sharing `f2`=0.0 and `f0`=-100.0), which is the failure
class already on record. New shapes, known failure mode -- a reconfirmation, not
a lever, and reported as such.

Both files were restored cleanly to baseline. **That matters as much as the
results** -- a parked unit left in an experimental state costs the next round.

### Where these two stand

```
castle        19/20   getKoopaShipStopPos, 6 differing, 12 permutations + variants
koopa_castle  16/17   __sinit, 13 differing, 22 measured shapes
```

Both are one function from landing and both residuals are instruction-scheduling
or register-preference decisions with no known source-level lever. Do not spend
further rounds without a genuinely new axis.

## NEW RULE: weak-cluster emission is LIFO — last declared, first emitted

anchor's ordering is **solved**, and the mechanism is general.

Declaring all six of the class's in-class-inline overrides in the target's
address order produced a draft whose `.text` sequence was the **exact reverse**:

```
declared in target order  ->  GetActorType, vf74, vf78, checkCutEnd, clearCutEnd, setCutEnd
target's actual order         setCutEnd, clearCutEnd, checkCutEnd, vf78, vf74, GetActorType
```

A clean LIFO relationship, not noise. Declaring the six in **reverse** of the
desired order produced the target's sequence exactly, confirmed from `draft.txt`'s
own function list rather than from `verify_anon`'s pairing.

**For a class's own in-class-inline (weak) overrides, `.text` emission is
last-declared-first-emitted.** Note this is the opposite of strong out-of-line
functions, which emit in definition order. Two batches, two directions:

```
strong, out-of-line   ->  definition order, early
weak, in-class inline ->  REVERSE declaration order, deferred to the end
```

The only remaining `FUNCTION ORDER` line on anchor is a known content-collision
mislabel (`finalUpdate__12dBaseActor_cFv`), which the target has no separate entry
for at all. **Effectively zero real order violations.**

### And a real content fix: a string reached through a named pointer

`fn_80100640`'s node-name argument was written as a direct string literal. The
target loads it **indirectly** (`lwz r4, lbl_2_data_436A8@l(r4)`) through a named
pointer variable at `.data+0x10` -- not the literal's own pooled address.

```cpp
static const char *smc_koopaShipNodeName = "cobKoopaShip";  // before the d_wm_lib.hpp include
```

Same early-declaration-controls-`.data`-position idiom as the kinoko family. This
gave an **exact `.data` layout match at every offset**:

```
0x0 "cobKoopaShip" | 0x10 smc_koopaShipNodeName | 0x14 "F7C0" | 0x1c "W7C0"
0x24 sc_ForceList  | 0x48 g_profile_WM_ANCHOR   | 0x58 "g3d/model.brres"
0x68 "cobAnchor"   | 0x78 vtable
```

`setNodePos` 37 -> 31, `execute` 43 -> 42.

**A string argument loaded via `lwz` through a `.data` slot is a named pointer
variable, not a literal.** A literal's address is materialised directly with
`lis`/`addi`.

### Arity ruled out properly

`fn_80100640` takes 3 arguments, confirmed by scanning **both call sites' entire
enclosing functions** for any `r6`/`r7`/`r8` touch -- none anywhere. That is the
right way to rule out the under-read-arity trap WM_NOTE hit, rather than
inspecting only the instructions immediately before the call.

## dance_pakkun PARKED at 9/16 — an allocation call, not a verdict on the unit

Nothing to nothing-but-9/16 including both of its largest functions, the
pointer-to-member-function idiom identified, castle's guard shape transferred and
adapted, and every constant corrected from guessed to measured. What remains is
compiler-behaviour residual rather than reconstruction, and fresh units in this
module have been landing 9/9, 11/11 and 13/13 in a single round each. That is the
whole reason for the park.

```
classInit MATCH | ctor 4 | dtor 21 | create() ~36 | execute() MATCH | draw() MATCH
doDelete() MATCH | createModel() ~70-75 | tailHelper() MATCH | calcModelFor() 101 (size 105/101)
startStep() ~15 | resetStep() MATCH | updateStepAnim() MATCH | unusedStub() MATCH
__sinit 14 (size 52/52 exact), pool drift 0x38 vs 0x54 unresolved
```

### Two process points from its last round, both worth keeping

**A contradiction between the disassembly and the measurement was documented
in-source rather than silently reverted.** `calcModelFor`'s `getRate()` comparison
reads as `<=` from the branch instructions (`fcmpo` + `ble`), but `>` measures
closer (103 versus 101 differing). Rather than quietly keeping the better number,
both readings are recorded in the source with the disagreement stated. **A future
attempt inherits the conflict instead of re-deriving it and landing in the same
confusion.**

**And a correction of mine worth repeating.** I told the agent to fix the function
and then re-measure the pool drift. It pointed out that two *failed* attempts not
moving the drift says nothing about whether a correct reconstruction would, so
the check I asked for was never actually performed. I would have over-read that.
**"The variable did not move" is only evidence if the thing you changed actually
worked.**

`calcModelFor` negatives, measured: separate scalar locals to force the
three-float `trans` overload give size 113 / 112 differing, worse -- the target's
per-component stack staging is real but is not produced by separate scalar
locals.

## Two bit-numbering conventions run in OPPOSITE directions

`ACTOR_PARAM_CONFIG`'s `offset` counts from the **LSB**; PowerPC's `extrwi`
counts its bit position from the **MSB**. They convert as
**`b = 32 - offset - size`**.

```
declared offset 8   ->  (x >> 8)  & 0xff  ->  LSB 8..15   ->  extrwi 8,16
declared offset 16  ->  (x >> 16) & 0xff  ->  LSB 16..23  ->  extrwi 8,8
```

kinoballoon's most common residual across three functions was `extrwi ...,8,16`
against the target's `...,8,8`, and both `(8,8)` and `(8,16)` had been tried as
declarations. The answer is `(16, 8)`. **Right width at the wrong position means
you have the convention backwards, not the field wrong.**

## dtk's reported VTABLE SIZE can be an over-merge

`(vtable size - 8) / 4` has been the standard virtual-count check. **It is wrong
whenever dtk merges adjacent content into the vtable symbol.**

kinoballoon's `lbl_2_data_456A0` reports `size:0x108`. Reading the relocations
instead (`dtk rel info -r`) shows the real slots stop where the `Absolute`
relocations end, at `+0x74` -- 0x78 bytes, 28 slots -- and what follows is
`PpcRel24` `__ptmf_scall` thunk code that dtk merged in. The 28 slots then match
`daWmSinkShip_c`'s independently-confirmed vtable slot for slot.

**Read the relocation types, not the symbol size.** `Absolute` relocations are
vtable entries; `PpcRel24` is code.

## `mParam` can carry PACKED CONFIG FIELDS, not runtime state

kinoballoon's apparent "state machine" in `modeExec`/`processCutsceneCommand` is
dispatch on a **spawn-time config value** extracted from `mParam` by the
`extrwi`/`clrlwi` bitfield idiom -- not on mutable state. Recognising that changes
what the functions mean.

Reading the base headers first also prevented two phantom members here:
`mVisible` (`dBaseActor_c`, +0x124) and `mClipSphere` (`dWmActor_c`, +0x128) had
both been mistaken for new fields. **A field you cannot account for is an
inherited member before it is a new one** -- the same lesson as `+0x60` being a
secondary vtable pointer.

## Session note

Two agents were terminated mid-round by a usage limit, not by any defect. Their
work was verified and banked: **WM_MANTA 15/16** (only `countModelVariants` open
at 19 differing) and **WM_START 8/14**. Both resumed from their committed state.
**Bank in-flight work when an agent stops unexpectedly** -- both drafts were
intact and re-verifiable, and nothing needed redoing.

## WM_MANTA LANDED — 16/16. Thirteenth unit. And the FUNCTION ORDER warning is a FALSE ALARM.

**11.450% -> 11.471%.** `.text 0x170eb0-0x17140c`, 16 functions, two rounds.
`daWmManta_c : public dWmDemoActor_c`.

### `verify_anon`'s FUNCTION ORDER check can be a false alarm, and now we know why

The unit reported **FUNCTION ORDER IS WRONG right up to the moment it landed**,
flagging two functions as "defined too late" with the warning that it "will not
link even at 16/16". **It linked, and all five binaries verify.**

The agent diagnosed this before the landing rather than after, and the experiment
was the right one: temporarily removing the two trailing functions made the
warning vanish. Cause: **four inherited weak virtuals that an ISOLATED compile
must emit locally, while the real link resolves them from another already-linked
TU.** The isolated object therefore has extra functions the target's own object
does not, and every position after them shifts.

**So: an order warning caused by weak inherited virtuals is not a blocker.** It
is still worth checking by hand -- kinoballoon caught a *real* order bug the same
day by tracing address-versus-definition order rather than dismissing the warning
-- but "the draft defines these out of order" is not decisive on its own. **The
real link is the only authority, and it is cheap: land it and find out.**

### The two real bugs in the last function

Both found by re-reading the target bytes instruction by instruction rather than
trusting an earlier summary:

- **An inverted branch sense on the outer validity check.** The target branches
  INTO the loop when the base lookup is invalid and falls through to `return 1`
  when valid; the draft had it backwards. The loop's own internal check had the
  correct polarity, and the agent had copied that polarity onto the outer check
  without re-verifying that specific branch target. **Two checks of the same
  predicate in one function can have opposite senses -- verify each branch
  target separately.**
- **One buffer, one byte per iteration -- not two `snprintf` calls.** The target
  formats the name once, then each iteration does a raw store at a fixed offset
  (`name[5] = 'a' + count; name[6] = 0;`). A re-format per iteration is a
  different instruction sequence entirely.

The expected signedness fix turned out to be unnecessary once those two were
right. **Fix the semantic defects before the scheduling ones** -- the same lesson
WM_NOTE recorded when a load-order residual stopped mattering.

### No `.ctors`, no `.bss`

Confirmed rather than assumed: nothing in the 16 functions needs either. This
unit does not reference `dWmLib::sc_ForceList` at all, which is also why the
`.data` family rule about the two anonymous 5-byte strings does not apply to it.

One open item, flagged rather than hidden: `countModelVariants` references two
`.data` objects (`0x46474`/`0x46480`) on the far side of a confirmed-foreign
`WM_MAP` chunk from the main claim. The slice claims only the contiguous range
and the module still verifies.

## DIAGNOSTIC: how to tell `switch` from `if`/`else if` in the disassembly

We have long recorded "use `switch`, not `if`/`else if`" as a lever. kinoballoon
found the **signature that tells you which one the target used**, which turns it
from a thing to try into a thing to read:

```
if / else if chain :  cmpwi rX, A ; bne <skip to the next check>      <- one compare, one branch,
                                                                        the else folded into a skip
switch             :  cmpwi rX, A ; beq <case A>
                      cmpwi rX, B ; beq <case B>
                      b <default>                                     <- each case independent,
                                                                         explicit final branch
```

`processCutsceneCommand`'s second dispatch went **100 -> 34 differing and from a
size mismatch to exactly 185/185** on this one change. **Read the branch shape
before choosing the construct.** This is the fourth unit the `switch` lever has
fixed, and the first time we can identify it from the target rather than guessing.

## The bitfield conversion, confirmed in use

`ACTOR_PARAM_CONFIG(BalloonType, 16, 8)` applied as derived:
`findBySubIndex` straight to MATCH, `markDone` 6 -> 5, `modeExec` 15 -> 11, all
three `extrwi` mismatches gone at once. **One convention error was producing the
single most common residual across an entire unit.**

## Named pointer versus bare literal, confirmed per-string

The rule that a string reached via `lwz` through a `.data` slot is a named
pointer variable, while `lis`/`addi` means a bare literal, was tested **per
string** rather than applied wholesale -- and the target does both in one
function:

```
GetResAnmChr's string  ->  lwz  ->  static const char *animName = "cobKinopio";
getRes / GetResMdl     ->  lis/addi  ->  bare literals
```

Two measured negatives confirm it: declaring a `static const char names[2][16]`
table for the other two strings creates a **new separate `.data` object with its
own base** rather than landing in the target's existing block (67 differing,
worse), and mirroring the pointer pattern onto all three gives 75, clearly worse.
**Check each string's addressing mode individually.**

## Where kinoballoon stands: 19/26

`createModel` is the keystone. The target caches **three** address bases via
`_savegpr_27` (5 registers) -- `.rodata`, `.data`, and the `animName` slot -- and
the draft triggers no saved-register block at all, so every downstream register
differs even though the call sequence now matches structurally.
`modeExec` and `processCutsceneCommand` carry the **same** pool-offset shift, so
closing `createModel` should move all three together.

## NEW LEVER: a ternary MERGES two calls; explicit if/else keeps them separate

WM_START's `unk_17A760` went **66 differing (stub) to 7** and one of the two
levers is new:

```cpp
setKind(cond ? A : B);          // MWCC merges to ONE call site, r3 pre-loaded then overridden
if (cond) setKind(A); else setKind(B);   // TWO separate li+bl blocks -- what the target has
```

The ternary form is not merely a different spelling; it produces a different
instruction count and shape. **If the target shows two separate `li`/`bl` blocks
for what looks like one call with a chosen argument, the source duplicates the
call in each arm.**

Alongside the already-recorded rule that a ternary *between two adjacent
constants* becomes arithmetic rather than a branch, this gives two distinct
ternary behaviours to watch for.

The other lever was polarity: **all three** of the branch conditions were
inverted from the first reading, and were settled from the target's own `beq`
bytes rather than from what the function names suggest. Same lesson as manta's
outer/inner check having opposite senses.

## A 104-differing count that is ONE defect

WM_START's `create()` reads as 104 differing, and it is not 104 defects. **A
single missing prologue instruction (`stw r30, 0x28(r1)` -- the target saves a
register up front that it does not load until much later) shifts every subsequent
line by one position in the raw diff.**

The agent applied the recorded misaligned-diff warning, re-read content-aligned,
and confirmed the call sequence, argument registers and branch targets all match.
**A large count immediately after a size mismatch is one defect until proven
otherwise.** Always check alignment before reading a count as a defect list.

## Two more `dWmLib` declarations applied

Both confirmed present in the symbol map before applying, and both added with
`syms.txt` entries; five binaries re-verified green.

```cpp
u8 getZoromeTime();     // getZoromeTime__6dWmLibFv  = 0x800FB440  (size 0xC)
bool IsSingleEntry();   // IsSingleEntry__6dWmLibFv  = 0x800FCAD0  (size 0x2C)
```

Return types are the authoring agent's inference -- the mangled name cannot carry
them -- and remain testable by the diff.

## Judgement worth copying

`unk_17A3C0` (231 instructions) was **read and characterised but deliberately not
authored**: it introduces a dual-child-spawn pattern not used anywhere else in the
unit, and the agent judged it not safely completable to the same standard in the
time left. An honest stub beats a guessed body -- a guessed body of the right
size is worse than none, because it looks finished and poisons `verify_anon`'s
pairing for its neighbours.

## Reading a `.rodata` pool dump is the fastest way to find a SIZE error

kinoballoon had a pool-offset shift shared by `createModel`, `modeExec` and
`__sinit`, and four rounds of register and declaration-order work never touched
it. Dumping the unit's real `.rodata` and reading its structure located the
likely cause in one pass:

```
+0x00  120.0f
+0x04  {0, -1, <reloc>}   \
+0x10  {0, -1, <reloc>}    >  THREE pointer-to-member-function entries, 0xc stride, 0x24 total
+0x1c  {0, -1, <reloc>}   /
+0x28  -10000.0f, 0.0f, 0.0f, 1.0f, 0.01f, 10.0f
+0x40  {1, 0, 3}  integers   \  two IDENTICAL integer triples, so two declarations
+0x4c  {1, 0, 3}  integers   /  rather than one shared object
+0x58  2160, -30, -478       <- sc_ForceList's mNodePos
```

The draft models a **one-entry** proc table where the target has **three**. That
puts everything after it `0x18` bytes too early -- **exactly the shared shift**,
and it explains why three functions shifted together: one cause upstream of all
of them, not three separate problems.

**When several functions in a unit share an identical pool-offset shift, look for
a SIZE error in an object near the front of the pool before attacking any of them
individually.** A shared symptom usually has a single upstream cause.

Two supporting notes:
- A duplicated constant in the pool (the two identical `{1, 0, 3}` triples)
  usually means **two separate declarations**, not one object reused. The same
  signal appeared on kinoballoon's `.data`, where `"cobKinopio"` sits at two
  addresses.
- Integers in `.rodata` are named objects, since this compiler never pools scalar
  integer literals. That makes them declaration-order-placed and therefore
  controllable.

### `createModel`'s register gap: parked, and complete

Four independent restructurings -- inlining the call as a direct argument,
materialising the address explicitly in the window, reference versus pointer, and
declaration-order swaps -- all produce **byte-identical** output at 76/77
instructions, 51 differing, `_savegpr_27`. The saved-register level already
matches the target.

Root cause, at instruction granularity: the target's `r30` does double duty as
the `.data` pool base and is then **reassigned mid-function** to `&mChrAnim[0]`,
with the reassignment landing between an address computation and its dereference,
which splits one load into two instructions. At `-O4` MWCC normalises all four
source forms to the same internal representation before scheduling, so no
source-level restructuring reaches the decision.

## NEW LEVER: De Morgan inversions are NOT equivalent to this compiler

```cpp
if (A && !B) { X } else { Y }        // logically identical...
if (!A || B) { Y } else { X }        // ...different bytes
```

WM_START's `create()` matched only the second form; the target's `beq`/`bne`
pattern at that branch point distinguished them. We had branch *polarity*
recorded; this is different -- **restructuring a compound condition through De
Morgan changes the emitted code even though the logic is unchanged.** Read the
branch pattern and try both arrangements.

## The remedy for a missing prologue register save

Two functions in WM_START showed the same symptom: a very large differing count
caused by **one absent `stw rN, off(r1)` in the prologue**, cascading a one-line
shift through the entire function. `create()`'s was 104 differing; the fix took
it to MATCH.

**Bind the value to a local declared BEFORE the intervening call.** The string
address had to survive a `getResMdl()` call; declaring `const char *nodeName =
"s1";` ahead of that call -- rather than passing the literal directly afterwards
-- produced the target's staged `lis`/`addi` and forced the register to be saved.

So the full pattern for this symptom is:
1. A large count immediately after a size mismatch is **one defect** until proven
   otherwise -- check content-aligned before reading it as a defect list.
2. If the missing instruction is a prologue save, the cause is a value the target
   keeps live across a call that your version recomputes.
3. Declare it as a local **before** the call.

## Parameter types ARE in the mangled name -- use them instead of guessing

WM_START's agent held off authoring a function partly because
`dWmEffectManager_c::playEffect`'s signature was "not yet confirmed". It is fully
determined, and was already in `syms.txt`:

```
playEffect__18dWmEffectManager_cFiPC7mVec3_cPC7mAng3_cPC7mVec3_c
  ->  playEffect(int, const mVec3_c *, const mAng3_c *, const mVec3_c *)
```

**Only RETURN types are absent from a CFront mangled name.** Parameters are fully
encoded. Before treating a callee's signature as unknown, decode its symbol --
and when declaring it, spell the parameter types so the mangled name reproduces
exactly, since a same-width alias with a different spelling mangles differently
and fails to link silently.

## The established way to use a class whose header models the wrong thing

`dWCamera_c` is `char pad[0x4f8]` plus an empty view-clip member, and three landed
units reference it. Units needing its real fields **do not change the header** --
they use a local `u8 *` offset cast confined to their own `.cpp`, with the
evidence in a comment.

Landed precedent: `source/d_basesNP/bases/d_a_wm_note.cpp` writes five fields at
`0x5f0`-`0x624` this way and shipped today. anchor used the same approach for
`dWmMapModel_c`, whose header models the wrong object entirely (`0xbf8` is the
size of a containing struct, not of the class).

**This is not a compromise, it is the correct division:** the shared header stays
safe for everything already landed, the unit progresses, and the next person to
model the class properly inherits the offsets and the reasoning.

## kinoballoon 19/26 -> 21/26 on ONE missing named zero word

My table-size hypothesis was **wrong** -- the proc table already had three entries
and had done for rounds. The agent verified that against both its own emitted
`.rodata` and the target disassembly before rejecting it, which is the correct
response to a confident instruction from the lead.

**The pool-dump method was still right, and it found the real cause.** Diffing the
emitted pool byte-for-byte against the retail dump showed the target has an extra
`0.0f` word between `-10000.0f` and the shared constant cluster. Confirmed
exhaustively unreferenced -- every `lfs`/`lwz` against every pool base across all
three target `.text` dumps, cross-checked against the relocation table -- so it is
a genuinely dead pool slot, not something reachable by restructuring logic.

```cpp
extern const float g_unofficial_kino_zero = 0.0f;   // file scope, before the ctor
```

Same idiom as antlion_mng's `g_unofficial_85C0`. The payoff was disproportionate
to the change:

```
modeExec                10 differing -> MATCH
__sinit                  3 differing -> MATCH
processCutsceneCommand  34 -> 28, and size now exact at 185/185
createModel             unchanged at 51 (confirms its residual is independent)
```

**A single missing pool word can hold three functions open.** When several
functions share a pool-offset shift, diff the emitted pool against the retail
bytes *word by word* -- the cause may be one dead slot, and a size error is far
easier to find that way than through the functions.

### Two bounds on the declaration-order rule

- **Pool ordering for an out-of-class table definition is NOT driven by lexical
  source position.** Moving `sProcTable`'s definition had zero effect. The
  declaration-order rule governs ordinary named constants; it does not reach this.
- A second orphaned zero word sits after `mNodePos` at the very end of the
  section, also unreferenced. Left alone deliberately: nothing indexes past
  `mNodePos`, so it cannot affect any remaining diff, **and it may belong to a
  neighbouring TU's pool that the linker placed adjacently** rather than to this
  unit at all.

### What remains, and why it is one problem

```
moveUp                  14   \
moveDown                14    >  the same mScale += mVec3_c(mRate,mRate,mRate) float reshuffle
processCutsceneCommand  28   /   (two of its lines are naming artifacts only)
markDone                 5       register swap
createModel             51       parked: register-reuse scheduling, four forms byte-identical
```

Three of the five carry **one** shared pattern. Being attacked together, on the
precedent that three units sharing a stack-materialisation defect resisted twelve
individual attempts and all fell in a single round once addressed as one problem.

## NEW LEVER: read WHAT IS IN the stack temp to identify the operator form

kinoballoon went **21/26 -> 24/26** — three functions to MATCH on one substitution,
found by asking what the target's stack scratch slot actually holds.

```cpp
mScale += mVec3_c(mRate, mRate, mRate);                              // 14 differing
mScale = mScale + mVec3_c(mRate, mRate, mRate);                      // 28 -- WORSE
mScale = mVec3_c(mScale.x+mRate, mScale.y+mRate, mScale.z+mRate);    // MATCH
```

**The diagnostic:**

- **Compound assignment (`+=`) materialises the ADDEND** at the stack slot.
  `mVec3_c::operator+=(const mVec3_c &v)` binds its by-reference parameter to a
  real stack slot, so the unmodified `{mRate, mRate, mRate}` appears there.
- **Constructor-plus-assignment writes the RESULT** to the slot — the `fadds`
  happens *before* the `stfs`, and the same registers go straight to the real
  fields with no reload. **No addend struct ever appears.**
- **Binary `operator+` produces TWO temps** — the addend plus the by-value return
  slot — and grew the frame `0x20 -> 0x30`. It is not in play here at all.

So: **trace whether the value stored to the stack temp is the addend or the sum.**
That single question distinguishes three source forms that all look equivalent.

The substitution applied unchanged to `moveDown` and to both sites inside
`processCutsceneCommand` — **four sites, no per-site tuning, all straight to
MATCH.** Attacking a shared pattern as one problem paid off again.

### Method note: read the raw block, not the diff, once order diverges

The agent found this by pulling the full `.fn`-to-`.endfn` block from **both**
target and draft and reading them in true program order, because **the diff
tool's line-by-line alignment is misleading once instruction order diverges** —
it pairs lines that are not counterparts. This is the same hazard as the
misaligned-diff warning, in a different guise: there the cause was a size
mismatch, here it is reordering.

**When a diff stops making sense, stop reading the diff and read both blocks.**

### kinoballoon stands at 24/26

```
createModel   51   parked: four source forms byte-identical, _savegpr_27 already correct,
                   cause is a mid-function register reuse the compiler normalises toward
markDone       5   parked: info/idx register-order swap, resisted several attempts
```

## kinoballoon PARKED at 24/26 — both residuals characterised

```
createModel   51   four source forms byte-identical, _savegpr_27 already correct.
                   Cause: target's r30 does double duty as the .data pool base and
                   is reassigned mid-function to &mChrAnim[0], splitting one load
                   into two. MWCC normalises every writable source form away from
                   that decision at -O4.
markDone       5   the value graph and register ROLES are identical on both sides;
                   only the physical assignment differs. Target loads the singleton
                   into r6, freeing r4 for the index; the draft loads it back into
                   r4 and puts the index in r5.
```

`markDone`'s three restructurings — declaration-order swap, binding the singleton
to a named local, and both orders of that — produced **byte-identical output**,
including a form that mirrors an already-MATCHED sibling function's exact pattern.

**The distinction that makes this a park rather than a gap:** `moveUp`'s residual
looked similar and turned out to be a *wrong value* in the stack temp, reachable
from source. `markDone`'s values are all correct and only the allocator's register
choice differs. **Ask whether the values are right before concluding a register
difference is a wall** — if a value is wrong, the source can reach it.

The unit went from nothing to 24/26 and produced the session's most transferable
lever on the way.

## Standing state of the WM units

```
LANDED (13):  antlion, sandpillar, kinoko_star, sinkship, note, manta,
              + cannon, cloud, dokan, dokan_route, ghost, grid, kinoko_1up,
              kinoko_base, kinoko_red, peach, peach_castle, smallcloud, tower
PARKED:       course 22/23   castle 19/20   koopa_castle 16/17
              antlion_mng 18/22   kinoballoon 24/26   dance_pakkun 9/16
IN PROGRESS:  start 10/14    anchor 16/22   item (round 1)   board (round 1)
```

**Every parked unit is one to four functions short, and every remaining residual
is instruction scheduling or register allocation.** The units that LANDED all
reached N/N within one or two rounds. That asymmetry is the single most useful
planning fact in this file: **a unit that does not close quickly tends not to
close at all**, so prefer opening a fresh unit over a fourth round on a
near-miss.

## WM_START: the prologue-save lever confirmed by REVERSION, and an informative anti-lever

`unk_17A3C0` went **222 differing -> 23**, size exact, saved-register level now
matching the target (`r28`-`r31`). The cause was the recorded one: two values used
across all four branches had to survive an intervening call, and declaring them
*before* it produced the missing `stw r28, 0x20(r1)`.

**Causation was proved, not assumed:** reverting the declaration to after the call
— which is where the target's own instruction order puts it — made the save
disappear and the count return to ~222. **A fix that survives being reverted and
re-applied is a mechanism; one that is only ever applied is a correlation.**

Note the declaration sits one line *earlier* than the target's own instruction
order. Source order and emitted order are not the same thing.

### An anti-lever that explained the target's structure

Hoisting a member read into a shared local before the dispatch **added** a
register save and **still made things worse**: it moved the null check ahead of
the compare chain, restructuring the dispatch itself. That revealed something
positive — **the target reads that field fresh inside each case body rather than
sharing one read across the switch.**

Two lessons: a change that improves a count while breaking the structure is a
regression, and **a negative that explains the target's shape is worth more than
a neutral one.**

### Open hypothesis: the `clrlwi.` masking wall is a RETURN TYPE

The same residual appears in two functions — draft emits `clrlwi. rD, rS, 24`
(a comparison on a **byte**) where the target has plain `cmpwi r3, 0` (the **full
word**). Shared residual, so look upstream.

The likely cause is the declaration `u8 getZoromeTime();`. Its mangled name
`getZoromeTime__6dWmLibFv` **cannot carry a return type** — the `u8` was an
inference, and this project has been caught by exactly that six times.
**`int` is being tested now.** The same caution applies to `IsSingleEntry()`,
applied as `bool` on the same kind of inference.

**Any declaration whose return type we inferred is a suspect whenever a spurious
mask or width-narrowing shows up in the diff.** Compile it both ways and let the
diff decide.

## WM_ITEM opened at 8/12, and a SYSTEMIC residual is now confirmed across three units

`daWmItem_c : public dBaseActor_c`, `sizeof 0x210`, `.text 0x167120-0x16793c`.
MATCH on the first round: `classInit`, ctor, dtor, `create`, three step functions,
`doDelete`.

### Walk the REL's own relocation stream for hard evidence

The agent loaded `tools/relfile.py`'s `Rel` class and walked
`original/d_basesNP.rel`'s self-relocations directly, settling the class layout,
six per-item-type data tables and the `.data` bounds from evidence rather than
inference.

That let it **correctly override `check_bounds.py`'s family heuristic**. The rule
"a `wm` unit's `.data` opens on the two anonymous `sc_ForceList` strings" flagged
the claim — but `g_profile_WM_ITEM`'s `classInit` field relocates straight to this
unit's first function, so `.data` really does start at the profile here. **Knowing
when a rule's precondition fails, and proving it, beats obeying the rule.**

### Two new levers

- **Signed versus unsigned members change the compare.** `mItemType` and two
  others had to be `int`, not `u32`, because the target uses signed compares.
- **Chained vector assignment `a = b = c = v` stores in `z, y, x` order**, not
  `x, y, z`. Three separate statements were needed to get the target's order.

Also re-confirmed: **never call the base destructor explicitly** — it doubles the
`bl __dt__...` and adds an unwanted vtable-pointer reset.

### The systemic residual: one dedicated base register versus per-site recomputation

**Third unit with this exact defect class**, and it is what parked the other two:

```
dance_pakkun  createModel   parked at 70   target holds 5 registers, draft 3
kinoballoon   createModel   parked at 51   _savegpr_27 matched, one register does double duty
item          createModel / calcModel / cycleAnm   116 / 60 / 14
```

**The target caches a `.rodata` constant-table base in one dedicated register for
the whole function; the draft recomputes it at each access site.**

**Untested hypothesis, now being measured on item:** declare the constants as
**ONE named array** and index it, rather than as separate named objects. One
object has one base, so every access shares an anchor by construction; separate
objects each get their own `lis`/`addi`, which is exactly the symptom. Neither of
the parked units tried consolidating — both had separate scalars in the pool.

Two supports for this being the right axis:
- It is the same reasoning that fixed dance_pakkun's string anchoring — three
  strings shared one base **because they were adjacent in one region**, not
  because of any addressing trick.
- Integers in `.rodata` are named objects, so their placement is
  declaration-controlled and the bytes can be kept identical while consolidating.

**Track the saved-register level and whether a single base register appears, not
the differing count** — the register signature moves first. If this works it may
unpark two other units; if it does not, it is a valuable negative on a systemic
problem.

## The return-type hypothesis was RIGHT, and the wrong type was in the shared header

`int getZoromeTime()` — not `u8` — eliminated the `clrlwi.`-versus-`cmpwi`
masking wall in **both** functions at once:

```
unk_17A760   7 -> 3 differing
unk_17A3C0  23 -> 20, then to MATCH with one further fix
```

**`include/game/bases/d_wm_lib.hpp` has been corrected** from `u8` to `int`, five
binaries re-verified green. I had applied the `u8` on an agent's inference; the
mangled name `getZoromeTime__6dWmLibFv` cannot carry a return type, so it was
never evidence.

`IsSingleEntry()` was tested as `int` too — **no change**, so it was reverted to
`bool` as the more natural type for a boolean-named function. A neutral result is
not evidence for either type; say so rather than picking the one you tested last.

**This is the seventh wrong return type on this project.** The rule stands and
should be applied more aggressively: **a spurious mask or width-narrowing in a
diff means suspect an inferred return type before anything else.**

## WM_START 10/14 -> 11/14, `unk_17A3C0` MATCHES

The second `unsigned long param` needed the same declare-before-the-call
treatment as the first — the lever applies **per value**, not once per function.
That took it to 9 differing, all pure symbol-name cosmetics, and `verify_anon`
reports MATCH.

### Two walls characterised rather than forced

**`unk_17A760`, 3 differing — block ordering.** At three symmetric branches the
target's `beq`/`bne` polarity matches neither phrasing available:

```
if (!X) {A} else {B}    right VALUES, wrong branch type (bne where target has beq, same label)
if (X)  {B} else {A}    De Morgan inversion: wrong values, 6 differing -- worse
X == 0 instead of !X    compiles identically, no change
ternary                 reproduces the known merged-call-site regression
```

MWCC physically reorders the two blocks — putting the `!X` block first as the
jump target rather than the fallthrough — independently of which logically
equivalent form is written. **The version with correct values was kept over the
version with the matching branch type**, which is the right trade: correct values
and a wrong branch beat wrong constants.

**`createModel`, 4 differing — the by-value stack-slot wall.** `resMdl` is passed
by value to two `create()` calls and stages at `r1+0x8` then `r1+0x10` in the
target, the reverse in the draft. **Same slots, reversed order.** Both wrapper
forms were measured in an earlier round. The agent explicitly checked whether this
round's new levers applied — register-binding-before-a-call, branch polarity — and
concluded they do not reach a same-slots-reversed-order permutation, rather than
retrying blind. That is the right way to decline a retry.

## The single-anchor hypothesis: CONFIRMED, but it decomposes into TWO axes

Tested on WM_ITEM. Consolidating a unit's scattered constants into **one named
array in the target's byte order** does force single-register anchoring — but it
is not sufficient on its own, and the precise conditions matter more than the
headline.

```cpp
static const float sConstTable[12] = {
    -32.0f, 200.0f, 3.2f, 0.65f, 0.0f, 1000.0f,
    1.0f, 0.65f, 2.0f, 3.2f, -32.0f, 200.0f,
};
```

**Axis 1 — multiple accesses in one function: CONFIRMED.** `calcModel` went from
**four separate `lis`/`addi` pairs to one shared `lis r31, sConstTable@ha` reused
via four offsets**, matching the target's single-anchor shape exactly. 68 -> 60.

**Axis 2 — a single access at a NON-ZERO offset: does not work, and it is a
compiler heuristic, not a phrasing problem.** `cycleAnm` was byte-identical under
both array-index and explicit pointer-arithmetic phrasing. **MWCC folds an offset
into the `lfs` immediate only when the address is an object's OWN first element,
never for an indexed read into a shared array.** No C++ phrasing reaches it.

**Axis 3 — a second shared anchor needs EAGER hoisting too.** `createModel` routes
eight tables through the profile symbol. Pointer arithmetic off `&g_profile_WM_ITEM`
did establish the target's addressing mode, but **lazily, mid-function**, where the
target hoists both anchors — profile and `.rodata` — before its first real call.
It reached 4 saved registers against the target's 5, and the count went **116 ->
165. Reverted.**

**So the correct statement is not "one array fixes it".** It is:
- consolidate for multi-access-in-one-function — that part works;
- a lone non-zero-offset read is unreachable;
- multiple anchors must additionally be hoisted eagerly, and getting the
  addressing mode right while missing the hoist is *worse* than not trying.

**Check this against dance_pakkun's and kinoballoon's specific access patterns
before assuming it transfers** — their `createModel`s may be axis 3, which is the
one that got worse here.

The register-signature reminder holds: the `_savegpr_27`/`_restgpr_27` helper-call
shape is the convergence signal, not the differing count.

## calcModel's remaining axis looks like the stack-temp lever, in reverse

`calcModel`'s other 60 is a stack-frame size difference (`0x20` against the
target's `0x30`): **the target stages a `mVec3_c`-shaped temporary separately
where ours coalesces it.**

That is the kinoballoon operator-form question running the other way. There, the
draft materialised the *addend* and the target held the *result*, so
constructor-plus-assignment was correct. Here the target stages an extra temp that
ours does not — which is what **compound assignment** produces, since
`operator+=` binds its by-reference parameter to a real stack slot.

**The lever is the QUESTION, not the two answers.** Trace what is actually stored
in the slot, then find the form that produces it -- do not work down a list of
candidate spellings.

WM_ITEM proved the point. I predicted compound assignment; the agent traced the
store first and found the target writes the addend three times and **never reads
the slot back**, and that `mVec3_c` has no `operator*=(const mVec3_c &)` at all,
so my theory could not literally apply. The form that reproduced it was a plain
three-argument constructor called with the same value three times:

```cpp
mVec3_c tmp(k[7], k[7], k[7]);
float v = k[9] * tmp.x;
mScale.x = v; mScale.y = v; mScale.z = v;
```

**`calcModel` 60 -> 15, with instruction count and frame size both exact**
(70/70, `0x30` on both sides) -- the largest single jump on that unit.

## A single object's `.bss` contribution is CONTIGUOUS — so "non-contiguous ownership" is impossible

WM_KILLER's first round concluded its `.bss` was two non-contiguous runs with
0x30 bytes of other units' statics between them, and asked whether the slice
format could express that. **It cannot, and it does not need to.**

Two independent arguments:

1. **A relocation proves a READ, not OWNERSHIP.** The evidence was that a function
   in this unit's `.text` references `lbl_2_bss_FE40`. `.bss` is one shared section
   across the module and a function can reference a static another TU defines --
   that is what `extern` produces. This is the same inference I made in the
   opposite direction earlier today and had to retract.
2. **The linker CONCATENATES input sections; it does not interleave one object's
   `.bss` with another's.** So no translation unit can own two non-contiguous
   `.bss` runs. The premise is structurally impossible, which means the ownership
   reading is wrong rather than the tooling being inadequate.

Resolution: the unit's `.bss` is the standard `0xfe00-0xfe10` -- registration node
plus the `c_StartPointKinokoHouseID` copy -- and the far address is another TU's
static that this unit merely reads. Declare it `extern`.

**Flagging it rather than forcing a contiguous claim was still right**: forcing one
would have overclaimed another unit's bytes.

### The ownership test that IS valid, restated

**A compile-time displacement in a TU's own instruction proves that TU emits
everything up to that displacement**, because the compiler cannot address into an
object whose placement it does not know. Distinguish that from a relocated
address, which proves only a read.

Three uses of this test are now on record, decisive every time: antlion did NOT
own bytes below its pool (its `__sinit` never reached past `+0x18`); antlion_mng
DID own a disputed block (`+0x48` could only be computed by a compiler that laid
out both ends); and adding a candidate object and watching the displacement move
by exactly its size confirms placement without knowing what the data means.

## The ownership check's false positives on virtual-only methods mean WRONG NEIGHBOURING BOUNDS

WM_BOARD's `check_bounds.py` run flagged five of the unit's own virtual overrides
— dtor, `execute`, `draw`, `doDelete`, `processCutsceneCommand` — as "never
referenced from inside the unit". **All five cleared the moment the `.rodata` and
`.data` bounds were corrected**, with no change to the `.text` claim at all.

The cause: a virtual override is reached through the vtable, which lives in
`.data`. If the `.data` claim does not contain the vtable, the tool cannot see the
reference and reports the function as unowned.

**So an ownership warning on a virtual-only method is a symptom of a wrong
neighbouring-section bound, not a `.text` problem. Fix `.data` and `.rodata`
first, then re-run.** On this unit the real bounds were `.rodata` ending `0x8648`
rather than a guessed `0x8654`, and `.data` extending to `0x43b58` rather than a
guessed `0x43a78` — the secondary-vtable object turned out to be `0xf0` bytes, not
the ~`0x10` assumed.

## THIRD occurrence: a constructor call you cannot account for is an EMBEDDED MEMBER

WM_BOARD declared a separate `mAllocator_c` member to explain a
`__ct__12mAllocator_cFv` call, producing `sizeof 0x208` against the real `0x1f0`
— a clean `0x18` overage, **caught by `classInit`'s own `li r3, 0x1f0`**. A probe
compile showed `m3d::anmTexSrt_c` is `0x2c` bytes and already contains that
allocator as a sub-object. Removing the redundant member fixed `classInit` and
another function to MATCH and took the ctor from 13 differing to 4.

Same finding as WM_START's `m3d::banm_c` (`0x28`, embeds an allocator) and
anchor's `+0x60` secondary vtable. **An unaccounted-for constructor call, field or
gap belongs to an embedded member or the ABI before it is a new field of yours.**
Probe the member's real size with `char[sizeof(X)]` rather than inferring it from
the constructor's gaps.

**`classInit`'s `li r3, <size>` is a free `sizeof` check** — it is the allocation
size, so a wrong class size shows up there immediately.

## MWCC rejects in-class default member initialisers

`(10123) ';' expected`. Not supported by this compiler; use the constructor's
initialiser list. Worth knowing before designing a header around them.

## NEW RULE: source DECLARATION order controls stack slot order

Independent of use order and of scope nesting.

WM_ITEM's `calcModel` had two by-value temporaries on **swapped** stack slots: the
target put the conditional, first-used one low (`0x8-0x10`) and the unconditional,
second-used one high (`0x14-0x1c`); the draft had it reversed.

```
give the unconditional local its own block scope   ->  no change, swap persists
declare it BEFORE the `if`, assign its fields later ->  15 -> 7, swap GONE, slots exact
```

**Declaring the second temporary earlier — while still using it later — put both
on the target's slots.** So the allocator orders slots by declaration, not by
first use and not by lexical nesting.

**The wrapper check was done FIRST, and correctly ruled the other lever out.**
Grepping for `bl` inside the function showed neither temporary generates a
constructor call at all — both inline to field stores — so no inline wrapper was
in play and the six-unit wrapper rule could not have applied. **Confirm the
precondition before restructuring**; this is the second time today that check
prevented a misapplication.

Residual then reduced to field-assignment order, swept exhaustively:

```
x,y,z 7   y,x,z 7   z,x,y 7   x,z,y 6   y,z,x 8   z,y,x 8
```

`calcModel` trajectory across the session: **68 -> 60 -> 15 -> 6**, with
instruction count and frame size exact and both temporaries on correct slots.

## WM_ITEM PARKED at 8/12

`.data` reordered to the real layout, the single-array/single-anchor shape landed
on two functions, and the slot-order mechanism demonstrated and reusable. All
three remaining gaps have a specific measured cause:

```
createModel  116   axis 3 -- two anchors needing eager hoisting; half-applied made it WORSE
cycleAnm      14   a lone non-zero-offset read; a compiler heuristic no phrasing reaches
calcModel      6   register-number/load-scheduling on one triple, decoupled from slot placement
__sinit       15   same offset-folding rule; re-measured after calcModel's fix and did not move,
                   confirming no cross-function pool sharing here
```

## Read the VTABLE to identify which target address is which function

WM_KILLER's first round had `execute()` placed at `0x167d20` — an 8-byte function
— purely because `verify_anon`'s greedy content matcher paired a
`return SUCCEEDED;` stub against it. **`check_vtable.py` against the class's own
vtable proved that address is `doDelete`, and `execute` is the 0x124-byte function
at `0x167b10`.**

The slot map came straight out and matched the family layout exactly:

```
slot  2  0x167aa0  0x6c   create
slot  5  0x167d20  0x8    doDelete
slot  8  0x167b10  0x124  execute
slot 11  0x167c40  0x30   draw
slot 24  0x168060  0x88   processCutsceneCommand
```

**Run `check_vtable.py` before authoring anything.** It gives the function
identities for free, and it is immune to the content-collision trap that
misidentifies small functions. Two units today had a function identified wrongly
by the pairing heuristic; the vtable settles it in one command.

Closing `draw()` and `doDelete()` followed immediately once their identities were
known — both are family-standard one-liners.

## The `.bss` ownership test applied correctly

Rather than accepting my resolution, the agent checked the actual instruction:

```
lis r6, lbl_2_bss_FE40@ha ; addi r5, r6, lbl_2_bss_FE40@l
```

A **full relocated reference to the symbol's own address** — the same shape this
unit uses for known externs — and **no instruction anywhere in the unit computes
that address as an owned base plus a constant.** By the displacement test, the TU
never demonstrates it knows the symbol's placement, so it does not own it.

`.bss` claim stands at `0xfe00-0xfe10`; the far symbol needs an `extern`.

**That is the test applied properly: not "is there a reference" but "is there a
COMPILE-TIME DISPLACEMENT off a base this TU establishes".**

## The duplicate-`beq` destructor wall: independently confirmed on a SECOND unit

WM_BOARD's dtor and anchor's dtor show the identical defect, diagnosed separately:

```
cmpwi r30, 0x0
beq .L_xxxx      <- target
beq .L_xxxx      <- target, redundant, same label
                 <- draft emits only one
```

Both agents read the raw `.fn`-to-`.endfn` blocks in program order and found the
two versions otherwise **identical** — same registers, same calls, same branch
structure. Both attribute it to the base class's own trivial destructor being
inlined with its call-boundary null-check preserved, which is outside the derived
class's source entirely.

**Two independent confirmations. Treat it as a wall and do not spend rounds on
it.** A derived class cannot reach a construct emitted by inlining its base's
destructor.

Note both agents also correctly refused to read the resulting count as a defect
list: **21 differing with a one-instruction size gap is one defect.** That rule
has now applied to four functions today.

## A hand-rolled constant that duplicates a shared-header object breaks the pool

WM_BOARD's `__sinit` matches in SIZE (33/33) but differs in pool composition. The
target has `0x18` bytes of `ForceInCourseList_t`-shaped fields between the
clip-sphere radius and the `mNodePos` floats; the draft emits a hand-declared
`static const short sBgmSyncData[2] = {4, 0};` — **a different, smaller object in
the wrong position** — instead of participating in the shared template.

The fix is to **source the values from `dWmLib::sc_ForceList[0]` itself** rather
than hand-declaring a local constant. Same class as kinoballoon's dead pool word:
a pool that is the right size but the wrong composition.

**Third confirmation today** that `sc_ForceList` and `c_StartPointKinokoHouseID`
are instantiated into every TU that includes `d_wm_lib.hpp`, with no source
reference needed. **Before hand-declaring any constant, check whether a shared
header already provides an object with those bytes.**

## Distinct stub VALUES are not enough if the TYPE collapses them

The rule "give every stub a distinct body" has a second-order trap, caught on
WM_KILLER **before compiling**:

```cpp
bool m_208;
m_208 = 0x2;  m_208 = 0x3;  m_208 = 0x4;   // twelve "distinct" stubs...
                                            // ...all compile to  stb 1, 0x208(this)
```

**A `bool` collapses every nonzero value to `1`**, so twelve stubs written with
different constants emit byte-identical code and poison the verifier's pairing
exactly as if they had been copy-pasted.

**Write stub distinctness into something that preserves it** — an in-bounds
padding byte taking distinct index values works — and **verify the emitted bytes
differ**, do not assume distinct source implies distinct output.

## Two more ways to close a function without inventing anything

- **`create()` closed with five of its own sub-calls still unauthored.** Each is a
  plain `bl` with no arguments to reproduce, so the family-standard clip-sphere
  shape matched the whole function regardless of the callees' bodies. **A function
  whose callees take no arguments can match before those callees exist.**
- **`processCutsceneCommand()` closed through a REAL virtual call** rather than a
  raw offset. `checkCutEnd()` is already declared in the landed header, so the
  compiler produced the correct vtable dispatch with no raw pointer code at all.
  **Check the landed headers for a declared virtual before reaching for an
  offset cast** — the cast is for classes whose headers model the wrong thing,
  not a first resort.

WM_KILLER stands at **7/23** with the vtable slot map settled, bounds plausible,
and every unauthored function carrying a genuinely distinct stub.

## A RELOCATED word reads as ZERO in the file — that is what a live pointer looks like

WM_BOARD's agent rejected a region as a function-pointer table because "a real
function-pointer slot should never show as `0`". **In a REL it always does.** The
address is supplied by the relocation; the file stores zero (or an addend).

Decoded with relocations, the region is unambiguous:

```
+0x00  00040000   two u16 {4, 0}  -- a real local constant, first in the pool
+0x04  43160000   150.0f
+0x08  00000000  \
+0x0c  ffffffff   >  ONE pointer-to-member entry {0, -1, <reloc>}
+0x10  00000000  /   reloc -> .text:0x15c960 == procNone
+0x14  00000000
+0x18  00000000
+0x1c  3f800000   1.0f
+0x20  2160, -30, -478   sc_ForceList's mNodePos
```

**Count the RELOCATIONS to count the entries.** One relocation, one entry. The
draft had a two-entry table with a placeholder duplicate — `0xc` bytes too many,
shifting everything after it, which is the `__sinit` residual.

**Always dump a pool WITH its relocations**, not as raw words. The decoding script
is three lines against `wip/wm_units/profile_map.py`'s `relocations()`.

### Mirror image of kinoballoon

kinoballoon's draft had **one** entry where the target had **three**; WM_BOARD's
has **two** where the target has **one**. Correcting kinoballoon's took two
functions straight to MATCH and a third to exactly the right size.

**A pool of the wrong size or wrong composition is the single most common cause of
a shared offset residual**, and it is invisible from the functions themselves.

### Two process points from the same round

- My suggested fix — source the values from `dWmLib::sc_ForceList[0]` rather than
  a local array — was **wrong**. The agent implemented it exactly, measured a
  clean negative (`__sinit` unchanged, `create` worse), tried two further
  variations, and **reverted rather than keeping a change that was neither
  correct nor better**.
- It also **retracted its own previous diagnosis** when four variants all left
  `__sinit` at exactly 3, proving the residual was not caused by the object it
  had blamed. **A residual that does not move across four independent changes to
  the thing you blamed is evidence you blamed the wrong thing.**

## WM_BOARD 7/15 -> 9/15: the proc-table fix, and the MIRROR of the stack-temp rule

**`__sinit` MATCH.** Trimming the proc table to its one real entry was **necessary
but not sufficient** — re-decoding the emitted pool against the region afterwards
showed the target has **two** zero words between the table and the `1.0f` where
the draft had one. A second named filler closed it.

**Fix one pool defect, then re-measure the POOL, not just the functions.** The
first correction exposes the second.

**`resetState` MATCH**, on two real bugs read out of the raw block: an inverted
branch condition, and — the interesting one — **the target has NO stack temp at
all**, just three direct field stores:

```cpp
mScale = mVec3_c(1.0f, 1.0f, 1.0f);              // draft: constructs a temp
mScale.x = 1.0f; mScale.y = 1.0f; mScale.z = 1.0f; // target: no temp
```

**This is the exact mirror of kinoballoon**, where three functions needed the
constructor form *because* the target held a result in a stack temp. Same
question, opposite answer.

**That is the proof that the stack-temp rule is a QUESTION, not a preference for
one form.** Trace what is actually in the slot — and "there is no slot" is a live
answer.

### `create` 18 -> 13, and a folding residual with a linkage hypothesis

Two real fixes: dropped a null guard the target never performs (the store follows
the allocation unconditionally), and bound the freshly-allocated pointer to a
local reused for all three field stores rather than re-reading the member.

Residual: the target performs genuine `lha` loads from the pool where the draft
constant-folds, because MWCC can prove a file-local `static const short[2]`'s
contents. **Two attempts to force a real load by changing the ACCESS PATH — through
the member pointer, and through an intermediate local — both still folded.**

**Hypothesis being tested: the folding is licensed by the object's LINKAGE, not by
the access path.** At namespace scope a `const` array has internal linkage, which
is what lets MWCC both strip it when unreferenced (already recorded) and fold it
when read. `extern const short[2]` should be a much weaker basis for folding. If
changing the path cannot fix it, change the linkage.

## An explicit redundant null check is SOURCE-VISIBLE — write it if the target has it

WM_KINOPIO's dtor needed a logically-redundant `if (mpMdlMng)` around
`delete mpMdlMng;` to match. **MWCC does not eliminate an explicitly written null
check even when the very next operation performs the identical check itself.**

And WM_BOARD found the same thing in the opposite direction the same day: its
`create()` had an `if (mBgmSync != nullptr)` guard the target never performs, and
**dropping it** was one of two fixes that took the function 18 -> 13.

**So a redundant guard is not noise in either direction — it is a source
construct you can see in the target.** Write it when the target checks; omit it
when the target does not. Both directions are now confirmed on the same day.

## WM_KINOPIO opened at 7/19, layout settled before authoring

`daWmKinopio_c : public dWmDemoActor_c`, `sizeof 0x1bc` — **confirmed from
`classInit`'s `li r3, 0x1bc` before a line was written.** That check is free and
should always come first.

**Five "mystery" offsets dissolved into inherited members** via an
`offsetof` probe against the already-declared base: `0x13c` and `0x158` are the
base's own allocator and model (the dtor destructs them directly), and `0x100`,
`0x124`, `0x139` are `mAngle`, `mVisible` and `mIsCutEnd`. The class's own new
members are exactly `[0x184, 0x1bc)` — 14 four-byte slots with no remainder.

**Fourth unit today where unaccounted-for offsets turned out to be inherited.**
Probe the base class before inventing a single field.

The `.data` family heuristic **applies here** — the claim genuinely opens on the
`"F7C0"`/`"W7C0"` pair — and the agent confirmed that from a `.data`-internal
relocation rather than assuming it, having seen the neighbouring unit where the
same heuristic correctly did *not* apply. **Check the precondition either way.**

`fn_2_16C810` is **0x834 bytes** with a 20-state jump table — the largest single
function met this session. Flagged and not attempted rather than rushed.

## NEGATIVE: external linkage does NOT stop MWCC's constant folding

My hypothesis was that a `const` array's **linkage** licenses the folding, since
internal linkage is what lets MWCC strip an unreferenced one. Tested properly:

```cpp
extern const short sBgmSyncData[2] = {4, 0};   // file scope -- block scope is a hard error
```

The linkage change **took effect** — the symbol now appears by its real name
instead of an `@LOCAL@` label — and MWCC **still folded** `sBgmSyncData[0]-1` and
`[1]` into `li r4,0x3` / `li r0,0x0`, identical to the file-local version.

**The definition is visible in the same translation unit either way, and that is
what licenses constant propagation — not the linkage.** Reverted, since the
extern version added a namespace-level object for no benefit.

Three access-path variants and one linkage variant have now failed on this. Note
`extern` with an initialiser at block scope does not compile: `(10123)`.

## WM_BOARD 9/15 -> 10/15: `calcModel` MATCH, on two bugs found in sequence

1. **The draft computed into a throwaway local `mMtx_c mtx;` instead of the
   inherited `mMatrix` member** (`+0x7c`, from `dBaseActor_c`), silently
   discarding the result into a stack temp that never persisted. 41 -> 34.
   **A local that shadows what should be a member write is a correctness bug the
   diff shows as a register difference.**
2. The stack-temp question again, and the answer was **two** captures: the target
   stores `mPos` as floats **and** `mAngle` as shorts into stack slots before the
   first call, then reloads `mAngle` from that stack copy for the second call
   rather than re-reading the member. That is the signature of two locals captured
   up front:

```cpp
mVec3_c pos = mPos;
mAng3_c ang = mAngle;    // both captured before the calls, both reused after
```

**Reloading from a stack copy rather than re-reading a member is the tell for a
captured local.** Third distinct answer the stack-temp question has produced
today — constructor form, no temp at all, and now two captured locals.

## A MISSING CALL cascades — check for one before chasing register differences

WM_BOARD's `execute` went **76 differing -> 18 on a single added line.** The
target's very first action is an unconditional virtual call — `mBgmSync->execute()`,
declared `virtual void execute();` on `dWmBgmSync_c` — and the draft never called
it at all, having gone straight to the `if (mBgmSync->m_0c)` check that follows.

A missing call shifts everything after it and reads as dozens of register and
scheduling differences. **Before attributing a large residual to allocation,
check that every call the target makes is present, in order.** The
`.fn`-to-`.endfn` block read in program order shows this immediately; a
line-by-line diff hides it.

## Cross-unit notes finally paid, and a member recovered from a sibling class

Reading `wip/wm_units/agent_anchor/`'s notes gave the shared extern's exact
signature and call pattern without re-deriving it:

```cpp
extern "C" void *fn_80100640(daWmMap_c *map, const char *name, int unused);
// returned pointer's +8 field is an offset (0 if invalid), added back to the
// pointer to produce a node-name string, then passed to GetNodePos
```

Board's own `lbl_2_data_439E8` proved to be a **named pointer variable holding the
address of `"cobKoopaShip"` — the same string anchor uses** — confirmed from the
REL's relocation table rather than assumed.

And `+0x1a8` turned out to be **`nw4r::g3d::ResFile mResFile`, a real member**,
not the guessed `int mUnk1a8` and not a stack local: the target stores `getRes()`'s
result directly to `this+0x1a8`, exactly as anchor's own class does. **When a
sibling class has a member at the same offset, take its type before inventing
one.** That plus the missing RTTI-cast follow-up call took `createModel`
112 -> 88.

**Read the neighbouring unit's notes before deriving anything.** This agent noted
three rounds running that it would and did not; the round it finally did, two
functions moved substantially.

## Enumerate the real vtable CHAIN before writing a shadow header — the method may already exist

I told WM_BOARD to declare an undeclared method in a shadow header to test a
natural virtual call. **It did not need to.** Enumerating the actual chain from
the headers already in `include/` placed slot `+0x20` exactly:

```
G3dObj :  IsDerivedFrom@0x8  G3dProc@0xc  ~G3dObj@0x10  GetTypeObj@0x14  GetTypeName@0x18
ScnObj :  overrides those four IN PLACE, then APPENDS from 0x1c --
          ForEach@0x1c   SetScnObjOption(ulong,ulong)@0x20   GetScnObjOption@0x24
          GetValueForSortOpa@0x28   GetValueForSortXlu@0x2c   CalcWorldMtx@0x30
```

`SetScnObjOption(ulong, ulong)` matches both the slot and the observed
two-argument shape, and is **already declared** in `g3d_scnobj.h` and inherited
unchanged. The manual vtable dereference was replaced with a real call and that
section became byte-exact. 88 -> 80.

**A derived class's vtable is its base's slots overridden in place, then new
slots appended.** Walking that chain from the existing headers identifies a slot
without guessing a name, without a shadow header, and without a header change.
**Do it before proposing any new declaration.**

## A LOWER differing count can hide a NEW regression — read the diff, not the number

Making a string pointer `const` took `createModel` **79 -> 71**, and the agent
inspected the diff instead of banking it: the `const` had shifted the surrounding
**string layout**, moving two strings off their correct `+0x68`/`+0x78` offsets.
**It traded one gap for a different, worse one and was reverted.**

Second instance of this shape today — WM_START's member hoist also improved a
count while restructuring a dispatch. **A change that improves the count while
breaking structure is a regression.** Always look at what moved.

## And a wrong enum constant, caught by reading the raw block

The target has `li r6, 0x40`; the draft had `0x20`. `BUFFER_RESMATMISC` is
`1<<5 = 0x20` and `BUFFER_RESANMVIS` is `1<<6 = 0x40` — the wrong flag from an
adjacent enumerator. 80 -> 79. **Read constants off the target's immediates and
check them against the enum, rather than picking the plausible-sounding name.**

## Decode a `.data` object WITH relocations before calling any of it unidentified

WM_KILLER's `createModel` was blocked on what its agent called an unidentified
static occupying bytes `0x00`-`0x77` of its `.data` object. **Every byte was
accounted for**, and most of it is automatic:

```
+0x00  "F7C0"  +0x08 "W7C0"                    \
+0x10  {6,->F7C0} {6,0} {4,->W7C0}              >  FREE from #include <d_wm_lib.hpp>
+0x34  "Fk00"   +0x3c -> "Fk00"                /
+0x40  -> .text:classInit                      <- g_profile_*, from ACTOR_PROFILE
+0x44  02750277                                <- two u16 profile ids
+0x4c  "cobKillerShot"   +0x5c -> pointer to it
+0x60  "cobRotaryShot"   +0x70 -> pointer to it
+0x74  0
+0x78  "g3d/model.brres"   +0x88 "cobKiller"   +0x94 "cobRotary"   <- bare literals
```

**Only two named pointers actually need declaring.** The `sc_ForceList` block and
the profile object are emitted with no source reference at all — now confirmed on
five units.

**And the pointer/literal distinction is per string, visible in the relocations:**
the two `*Shot` strings each have a pointer to them, the other three do not. That
is why a `static const char *const[2]` table made it *worse* — a table is one
object with one base, but the target has **two independent pointers, each emitted
directly after its own string.**

**Dump the object with its relocations before concluding anything is unknown.**
Three units today have had a "mystery" region dissolve entirely on one decode.

## Two more findings from the same round

- **`clrlslwi rD, rS, 24, 8` is a FUSED mask-and-reposition.** It decodes as a
  field in the LOW byte, explicitly shifted `<< 8` at the call site — not a field
  at bit 16. Getting that wrong gives the right value from the wrong place.
- **A layout bug found from a single target store offset.** `mPad_1f8` declared
  `[0x1c]` instead of `[0x10]` pushed a member 12 bytes late, caught from the
  target's own `stw r3, 0x214(r31)` rather than from any diff. **A store's offset
  in the target is a direct assertion about your layout** — check members against
  target store offsets, not just against `classInit`'s size.

## WM_KILLER 8/23 -> 13/23, and a correction to my byte map

I decoded the unit's `.data` object and attributed everything up to `+0x40` to the
shared header include. **That was wrong.** `"Fk00"` at `+0x34`, with its own
pointer at `+0x3c`, is **unit-specific** — it is in no header in `include/`, and
it needs declaring:

```cpp
static const char *smc_nodeNameTemplate = "Fk00";   // before ACTOR_PROFILE(...)
```

**Check each object in a decode against the headers rather than assuming a whole
region is automatic.** The block genuinely is automatic up to `+0x34`; the mistake
was extending that to the next object because it sat inside the same run.

### The findings from that round

- **A hidden-return-pointer function with RVO through an out-parameter.**
  `unk_1681C0` is `mVec3_c f(daWmKiller_c *)` — `r3` is the out-buffer and `r4` is
  `self` — and `daWmMap_c::GetNodePos`'s **reference out-param IS the hidden-return
  slot**. The result is written to a member and returned through the same buffer.
  Sizes match exactly; the residual is a register-pair swap.
- **`this+0xac` was already `mPos`**, inherited from `dBaseActor_c` — confirmed
  from the landed `d_base_actor.o` corpus rather than a header read. **Fifth unit
  today where an unaccounted-for offset turned out to be inherited.**
- **Naming the left-hand operand as its own local fixed a stack-slot swap** in a
  binary operator's two-temporaries pattern:
  `mVec3_c targetPos(...); mVec3_c dir = targetPos - mPos;` rather than an
  anonymous temporary on the left. Consistent with the declaration-order rule.
- **`(int)ACTOR_PARAM(Kind) == 1`** — the signed cast turns `cmplwi` into `cmpwi`.
  The bitfield macro yields an unsigned value; cast it when the target compares
  signed.
- **A "closed" function can carry residual lines that are purely symbol naming.**
  `unk_1682F0` reports 6 differing, all `lbl_2_*` versus real mangled names, zero
  real bytes. Check what the differing lines *are* before treating a small count
  as an open defect.

## WM_KILLER 13/23 -> 17/23: eight transferable findings in one round

- **An unconditional fallback computed BEFORE the branch, not in an `else`.** The
  target computes `(mPos.x, mPos.y + 50.0f, mPos.z)` up front and overwrites it
  only on success. Writing it as an `else` arm does not match.
- **Two declare-before-call locals, and their ORDER mattered.**
  `mVec3_c pos = mPos; mAng3_c angle = mAngle;` — declaring `pos` first was the
  one that closed it. Store-order swap, same lever as castle's.
- **A `static` free function taking NO arguments — not even `this`.** Proven from
  the only call site setting up nothing in `r3` before the `bl`. Do not assume a
  function inside a class's range is a member.
- **Two DIFFERENT mangled classes can share a method name.**
  `fManager_c::searchBaseByProfName` and `dBase_c::searchBaseByProfName` are both
  used in this unit, by different functions. Resolved from the target's own
  symbol, not assumed from the name.
- **A field treated as padding was a real `int`**, proven by `stw`/`lwz`/`cmpw`
  against it. Retyping it also fixed two other stubs that had been aliasing it.
  **Access width and instruction kind identify a field's type.**
- **Name a comparison's boolean result as its own local** rather than branching
  straight off the `fcmpo` — closed a `fabs`-compare-branch sequence.
- **Do NOT cache a static pointer when the target re-reads it.** A four-call chain
  re-reads `daWmPlayer_c::ms_instance` fresh before each call; caching it into a
  local breaks the shape. **This is the inverse of the bind-a-repeatedly-accessed-
  value lever — hold both and read the target.**
- **A four-way boolean chain must be a single `||` expression**, not `if`/`else if`
  — the latter duplicates the "set flag" store four times against the target's one
  shared label.

## The offset-folding rule, and its MIRROR

WM_ITEM established: **MWCC folds an offset into the `lfs` immediate only when the
address is an object's OWN first element, never for an indexed read into a shared
array.** There the draft carried an extra `addi` the target lacked, and no
phrasing removed it — a wall.

**WM_KILLER has the mirror.** Its target HAS the `addi`, which is the signature of
an indexed read into a shared array, while the draft's bare `300.0f` gets its own
pool slot with the offset folded. So the fix is to **declare the constants as one
named array in the target's order and index it** — producing exactly the `addi`
the wall produced unwanted elsewhere.

**Same rule, opposite sign, depending on which side has the extra instruction.**
Third rule today that turns out to be a question rather than a fixed preference.

## New tool: `wip/wm_units/dump_obj_section.py` — dump a compiled object's section bytes

Comparing a draft's emitted `.rodata`/`.data` against the retail bytes word by
word has been decisive on several units, but **there was no way to do it** — no
`elftools`, no `objdump` in this toolchain. An agent correctly reported the check
as "not attempted" rather than faking a result. This closes that gap.

```
python wip/wm_units/dump_obj_section.py <object.o>              # list sections
python wip/wm_units/dump_obj_section.py <object.o> .rodata      # dump with float/ASCII decode
python wip/wm_units/dump_obj_section.py <object.o> .data 0x0 0x80
```

Minimal self-contained ELF parsing, big-endian 32-bit only, which is all this
project emits. Compare against the retail bytes with the REL reader — `.text` at
file offset `0xF0`, other sections from the section table at `0x10`.

### It answered WM_KILLER's open question immediately

```
draft   1.0  300  100    0   50  1.0 | 2160 -30 -478
target  1.0  300    0   50   70  500 | 100  0  50  1.0 | 2160 -30 -478  0
```

**The draft's pool is four words short, and `70.0f` and `500.0f` appear nowhere in
the draft at all** — they belong to `createModel` and `execute`, both still
unauthored. So `createModel`'s 76 differing is not a mystery: **the constants it
reads are not in the pool, so every displacement after them is wrong.**

**A unit's pool cannot be right while any function that contributes to it is
unwritten.** Author the functions, then judge the pool.

## Adjacency in the byte layout does NOT imply one shared C++ array

I gave WM_KILLER a ten-value pool dump and suggested declaring it as one array.
The agent applied it, **regressed three previously-matching functions**
(`create` 6->22, `unk_1680F0` 5->37, `unk_1682F0` 6->33), diagnosed why, and
reverted to a two-element array used by one function.

The reason: those targets use a **direct `lis`+`lfs` on the constant's own
symbol, with no `addi`** — each is a standalone scalar object in the real source.
Only one function's target has the `addi`-then-displacement shape of an indexed
array read.

**Direct-symbol versus indexed-read is a PER-ACCESS-SITE fact, and the target
bytes are the only arbiter.** This is the third "check per site, not wholesale"
finding today, after per-string pointer-versus-literal and per-value operator
form. **A pool dump shows you the bytes, not the source structure.**

## An outer guard plus `while` reproduces a redundant jump that `do/while` does not

The target had an extra unconditional branch into a loop body that a
`do { } while()` never produces, even though an earlier `if (x == nullptr) return;`
already proves the list non-empty. Writing **both** the guard and a plain
`while (x != nullptr)` reproduced it exactly — **the compiler treats the outer
guard and the loop condition as unrelated** and does not elide the entry jump.

## WM_KILLER 18/23 -> 22/23. My pool diagnosis was right for one function and WRONG for the other.

I told the agent `createModel`'s 76 differing was caused by its constants being
absent from the pool. **The pool converged to an exact match and the function
stayed at 76.**

The real cause was a **missing statement**: `mAnmChr.mPlayMode = m3d::FORWARD_ONCE;`
before `setRate()` in both branches. The field's offset (`0x1e8`) was computed
from `mAnmChr`'s own known layout — vtable + `mpObj` + `mpHeap` + `mAllocator_c`
(`0x1c`) + three floats — landing on a real public `u8` field, so **no offset cast
was needed once it was identified.**

My reasoning was correct for `execute`: the two constants unaccounted for
anywhere else (`50.0f`, `70.0f`) really were its arguments to a far function.

**So: a short pool explains a residual only for the function whose constants are
missing. Check that every STATEMENT is present before blaming data layout** —
sibling of the missing-call finding, and the second time today a large residual
turned out to be one absent line.

### The `R_<module>_<section>_<offset>` convention extends beyond `.text`

```cpp
extern "C" float R_2_6_FE40[6];   // module 2, section 6 = .bss
```

Section numbers come from `relfile.py`'s own table. We had only ever used section
1. A far `.bss` symbol in an un-landed region is reachable the same way a far
function is.

### And two more "it was already declared" wins

`bmdl_c::play()` is a real declared virtual at slot `0x1c`, and `fanm_c::mPlayMode`
is a real public field — **both needed no offset cast once identified.** Together
with `SetScnObjOption` earlier on the same unit, that is three times on one unit
where walking the existing headers beat writing a cast.

**Reach for a cast only after checking the headers and walking the vtable chain.**

## WM_KILLER PARKED at 22/23 — a genuine CROSS-AXIS wall

Five axes tried on one five-instruction residual, every variant measured with its
register assignment reported:

```
operand order        reversed addition            7 real, worse
cast spelling        fused, and cast-at-declaration   12 and 7, worse
declaration order    value before the buffer copy     ~33, much worse
signature            hidden-return vs out-pointer vs out-reference
                     -> ALL THREE byte-identical to each other, 5 real
```

**The signature result is the notable one.** A by-value class return, a `void`
function with an `mVec3_c *` first argument, and one with an `mVec3_c &` first
argument produced **byte-identical output through the entire function body** —
same size, same register roles, same residual. Usually a genuine hidden-return
pointer is observable because `r3` stays reserved; **here it is not observable at
all.**

So the return-type axis — which has caught seven wrong types on this project,
including one this session that fixed two functions at once — **does not apply to
every function.** Test it, but do not assume it must be the answer.

The unit is 23 functions with `.rodata` matching the retail bytes exactly through
the whole constant run, parked one register-pair from shipping. Trajectory across
the session: **4 -> 7 -> 8 -> 13 -> 17 -> 18 -> 22 of 23.**

Residual: the target assigns `r4` to `mParam`'s extracted byte and `r5` to
`name[3]`'s pre-add value; the draft has them swapped, and no source-level
phrasing across five axes flips the allocator's choice.

## WM_KILLERBULLET opened at 4/37, and a LANDING-ORDER DEPENDENCY on WM_KILLER

`daWmKillerBullet_c` derives **directly from `dWmDemoActor_c`** — unlike its parent
`daWmKiller_c`, which goes through `dWmObjActor_c`. Confirmed by disassembling
the landed `daWmPeach_c` object, a sibling that derives the same way, rather than
by inference. `sizeof 0x208`, vtable slot map clean.

**`sizeof(m3d::smdl_c)` is genuinely `0xc`** — vtable plus two pointers, probed
directly rather than inferred from constructor gaps. That habit has caught three
phantom members today.

### The cross-unit dependency

WM_KILLER's `execute()` calls `fn_2_169550` and declares it as a free function,
`R_2_1_169550(dWmActor_c *)`. **That address is inside WM_KILLERBULLET's own
`.text`** — so it is an ordinary member call with an implicit `this`, not a free
function taking an explicit pointer.

WM_KILLER matches either way because the bytes are identical, **but once
WM_KILLERBULLET lands the symbol becomes real and the two declarations must
agree.** Recorded as a landing-order dependency: land WM_KILLERBULLET's side
first, or fix WM_KILLER's declaration at the same time.

### Its state table has FIVE entries, counted from the relocations

```
entry 0 -> .text:0x168eb0    entry 3 -> .text:0x1690f0
entry 1 -> .text:0x168ff0    entry 4 -> .text:0x168f10
entry 2 -> .text:0x169280
```

`execute()` dispatches through a pointer-to-member table at `lbl_2_rodata_89F8`
indexed by `m_1b0 * 0xc` via `__ptmf_scall` — a genuine five-state machine, so
**five of the thirty remaining functions are the per-state handlers, identified by
address before a line is written.**

**Note entries 3 and 4 are out of address order.** The table's order is the STATE
order, not the layout order — do not infer one from the other.

**Counting relocations to get the entry count is now the standard opener for any
table.** Two units today lost rounds to a wrong count — one too few, one too many
— and in both cases every constant after the table shifted.

## WM_KILLERBULLET 4/37 -> 9/37, bounds pinned on all five sections

```json
{".text": "0x1686e0-0x16a150", ".data": "0x453e8-0x45630",
 ".rodata": "0x89f0-0x8a3c", ".bss": "0xfe10-0xfe3c", ".ctors": "0x3f4-0x3f8"}
```

The five-entry table, declared as a real
`mVec3_c (daWmKillerBullet_c::*)[5]` pointer-to-member array, **closed two state
handlers immediately** — confirming the relocation-count method for table sizing.

### Name what the symbol names, and no more

Two undeclared members, handled differently and correctly:

- `m_1fc`'s call has a **real mangled name** (`calcRotate__12dWmRotater_cFv`), so
  the class was named `dWmRotater_c` and forward-declared minimally.
- `m_200`'s call has **no name at all** — just a raw vtable index — so it was left
  as raw vtable dispatch rather than inventing a class.

**A mangled name is a licence to name a type; a bare vtable index is not.**

### The ownership check caught a SHARED EXTERNAL table

`lbl_2_data_45428` sits inside the unit's address neighbourhood but has **50
references from outside its `.text` claim** — it is a shared external constant
table reached by `extern`, not this unit's data to declare or duplicate. The
placeholder using its values was flagged as known-wrong rather than left looking
finished.

**Run the ownership check on any data object before declaring it**; a table near
your unit is not necessarily yours.

## TOO MANY saved registers means the INVERSE lever

`execute()`'s target uses **2 registers and a `-0x10` frame**; the draft uses **5
and `-0x20`**. Every other register problem this session has been a shortfall —
this is the first over-preservation.

**When you have MORE saved registers than the target, you are caching something
the target re-materialises.** That is the inverse of the bind-a-repeatedly-
accessed-value lever, and WM_KILLER established it in the other direction: a
four-call chain there re-read a static pointer fresh before each call, and caching
it broke the shape.

**Track the frame size and saved-register count, not the differing count** — those
move first, and their direction tells you which lever you need.

## A failed variant that PROVES the structure beats one that fails neutrally

WM_KILLERBULLET's `execute()` resisted five variants. The most valuable was the
one that failed worst: replacing the pointer-to-member table dispatch with an
explicit `switch` on the state index went to **105 differing**, which
**positively confirms the target really is table-dispatched** rather than merely
being consistent with it.

**When a residual is stubborn, deliberately try the structurally-wrong
alternative.** If it gets much worse, you have converted an assumption into a
measurement.

Frame shape remains `-0x20`/5 registers against the target's `-0x10`/2, across
re-reading the singleton, dropping `const` on the table, moving the table to an
in-class `static const`, and the `switch`. Wall on those axes.

## The shared parameter table, and the `R_<module>_<section>_` convention for `.data`

`lbl_2_data_45428` is `0x180` bytes of mixed floats and packed shorts — a shared
game-parameter block with 50 referrers across the module, sitting inside
WM_KILLERBULLET's address neighbourhood but belonging to nobody in it:

```
+0x00 2000.0  +0x04 2.5   +0x08 60.0  +0x0c 60.0
+0x10 0.8     +0x14 0.7   +0x18 200   +0x1c {400,10}
+0x20 {3000,1050}  +0x24 0x100  +0x28 0.5  +0x2c {20,2}
+0x30 200.0   +0x34 -100.0  +0x38 0.0  +0x3c 20.0  ...
```

Reference it, never duplicate it. **Section 5 is `.data`:**

```cpp
extern "C" const float R_2_5_45428[];   // @unofficial, shared parameter table
```

The `R_<module>_<section>_<offset>` convention now has confirmed uses in **three
sections**: `.text` (section 1), `.bss` (section 6), and `.data` (section 5).
Section numbers come from `relfile.py`'s own table.

**A table sitting near your unit is not necessarily yours** — run the ownership
check, and if it has referrers from outside your `.text` claim, declare it
`extern` rather than defining it.

## WM_KILLERBULLET 10/37 -> 15/37: one `extern` unblocked five functions

Declaring the shared parameter table as `extern "C" const float R_2_5_45428[];`
closed `state2` and `state3` outright and is now read by **five** functions.

**And `unk_169510` turned out to be the table-indexing primitive itself:**

```
R_2_5_45428 + 0x54 + (u8)mParam * 0x18
```

— a per-"kind" sub-table inside the shared block. That explains why five separate
functions read through it, and it means **the shared table has internal structure
worth mapping** rather than being a flat constant pool.

## FIFTH time today: check `include/` before calling a class unowned

The agent identified `m_200` as `dWmBgmSync_c *` from its vtable symbol, then
reported the class as "not landed anywhere yet (no header)".

**`include/game/bases/d_wm_bgm_sync.hpp` exists and is complete** — real
constructor, virtual destructor, `execute()`, `getAnmRate()`, all fields.

**And the "inline construction — manual vtable plus field writes rather than a
real constructor call" that looked nontrivial to model is simply what
`new dWmBgmSync_c()` emits.** The constructor is defined in-class, so MWCC inlines
it: the manual vtable store and field initialisations **are** the inlined
constructor. `wip/wm_units/agent_board/` does exactly this.

Running tally of things treated as unowned that were already declared: a
secondary vtable pointer, `SetScnObjOption`, `bmdl_c::play()`, `fanm_c::mPlayMode`,
and now `dWmBgmSync_c`. **Grep `include/` for the class name first. It costs one
command.**

## Branch polarity that no phrasing reaches

`unk_169F00` is size-exact (39/39) with correct content, calls and arguments, and
its early return compiles with the opposite branch type from the target under a
direct early return, an explicit `== false`, and a result-flag form — the last of
which also cost two instructions and a spilled register. Kept the best at 26 and
moved on.

**Distinct from the De Morgan finding**, where restructuring a compound condition
*did* change the output. Here there is no compound condition to restructure —
a single predicate's branch type is not always reachable.

## dtk UNDER-sizes objects as well as over-sizing them. Relocations are the authority.

WM_KILLERBULLET hit an architectural puzzle: `create()` writes a plain
`lbl_2_data_43E34` address into an object's offset 0 with **no `__vt__`-named
symbol anywhere**, yet the destructor dispatches through that offset as if it
were a vtable.

**It IS a vtable.** dtk reports `size:0xC`; the relocations run well past it:

```
+0x00  0            \  offset-to-top and RTTI, both null in this ABI
+0x04  0            /  -- the standard C++ vtable header
+0x08  -> .text:0x15cdc0     +0x14  -> .text:0x15ce00
+0x18  -> DOL:0x800f28e0     +0x1c  -> DOL:0x800f2910
+0x20  -> .text:0x15d200     +0x24  -> DOL:0x800f2920
+0x28  -> DOL:0x800f2950
```

Two null words followed by function pointers is a vtable. It is unnamed only
because the class (`dWmRotater_c`) is not landed, so there is no `__vt__` symbol
for dtk to attach — and dtk then sized the object from its own heuristics rather
than from where the relocations stop.

**So the class is genuinely polymorphic**, its destructor release is an ordinary
virtual call, and the symmetry with the neighbouring polymorphic member — which
looked anomalous — is real.

### The general caveat

```
earlier today:  a vtable reported 0x108, real slots ended at 0x78   (OVER-merge)
here:           an object reported 0xC,  relocations run to 0x2c+   (UNDER-size)
```

**dtk's reported object size is unreliable in BOTH directions. Count the
relocations and read where they stop.** This now applies to vtables, tables of
function pointers, and constant pools alike — the same measurement has settled
table entry counts on three units today.

## An unnamed vtable does not mean a non-polymorphic class

If an object's offset 0 receives a `.data` address and something later dispatches
through it, **decode that address's relocations before concluding anything.** A
class that is not landed has no `__vt__` symbol, so its vtable looks like an
anonymous data blob — but it is still a vtable, and the class should be modelled
with the virtuals it implies rather than with a raw pointer cast that matches
bytes.

## WM_KILLERBULLET 15/37 -> 17/37: the vtable read closed the destructor outright

Modelling `dWmRotater_c` as polymorphic — on the strength of `lbl_2_data_43E34`'s
relocations rather than dtk's wrong `size:0xC` — closed the destructor to 59/59
with both residual lines symbol-name only. **The doubled-`beq` question resolved
on its own**; the recorded wall never had to be invoked.

**And the agent scoped the change correctly.** It added the virtual destructor for
the *release* side only, and left `create()`'s construction exactly as it was —
raw `operator new` plus a manual field write of the external vtable-shaped table
— because **`create()` never calls `dWmRotater_c`'s own constructor.** Modelling a
class as polymorphic does not mean every site becomes a constructor call; match
what each site actually does.

## "A unit's pool cannot be right while any contributing function is unwritten" — second unit

`unk_168C80` is this unit's `createModel`-shaped function. Its content is complete
and confirmed — real strings read from the unit's own `.data`, including one
string reused for two arguments, established from the target's own register reuse
rather than assumed. Adding the shared-header include moved its string offsets
from `0xc` to `0x40` against a target of `0x184`.

**The pool is still short by roughly `0x144` bytes belonging to functions nobody
has written yet**, so the function cannot close in isolation regardless of how
correct it is.

WM_KILLER hit exactly this and it resolved as its remaining functions were
authored. **When a function's content is confirmed correct but its pool offsets
are short, stop working on that function and write the others.**

## A MEASURED NEGATIVE CAN BE CONTEXT-DEPENDENT. Re-test after fixing a real bug.

WM_BOARD's `execute` closed **18 -> MATCH** on two fixes, and the second one had
already been measured as a failure:

1. A real control-flow bug: when the lookup returns null the target branches past
   the **entire** call, while the draft always called it with a null argument.
   Wrapping the call in the guard rather than defaulting its argument: 18 -> 14.
2. **Retrying the no-arg virtual `mAnim.play()` — previously measured as a
   regression (size 85 -> 87) and recorded as a dead end.** After the null-check
   fix changed the surrounding register pressure, **the identical source compiled
   to the exact target shape.** 14 -> 0.

**So a negative measured while another defect is present may not hold once that
defect is fixed.** This qualifies the measured-negative discipline that the rest
of this file depends on — negatives are still worth recording, but they are
recorded *in a context*.

**Practical rule: after fixing a genuine bug in a function, re-test the variants
you had already ruled out for that same function.** The cost is one recompile and
it recovered a MATCH here.

Note the distinction from the walls in this file: those were measured across
*many* variants on functions with no outstanding real bugs. A negative measured
alongside a known-wrong neighbour is much weaker evidence.

## Two more clean negatives, and the limit of the declaration-order rule

`createModel` (79) resisted both suggested levers, and both failures are
instructive:

- **Declaration-order hoist: 79 -> 124.** Default-constructing
  `nw4r::g3d::ResMdl`/`ResAnmTexSrt` at the top and assigning later is **not
  semantically free** the way it is for a POD int or pointer. **The
  declaration-order rule was drawn from trivial temporaries and does not
  transfer to types with real constructors.**
- **Indexed-array-read form: 79 -> 71 by count, reverted.** Same failure mode as
  the earlier `const`-pointer attempt: the new object's storage shifted two
  strings off their correct `+0x68`/`+0x78` offsets, and the access still did not
  reach the real symbol (it stayed an anonymous `@LOCAL@` label).

**Second time on this unit that a lower count concealed a string-offset
regression.** Read the diff, not the number.

## WM_BOARD PARKED at 11/15 — and both retests HELD, which sharpens the rule

Applying the context-dependence finding to this unit's own two walls:

```
ctor  (4)   the initializer-list-versus-body-statement variant, retried post-fix:  4 -> 21, worse
create (13) extern linkage, retried post-fix:                                     13 -> 13, no change
create (13) reading back through the member pointer, retried post-fix:            13 -> 15, worse
```

**Both held identically before and after the surrounding bugs were fixed.** That
is a much stronger basis for calling them genuine walls than a single measurement
— and it draws the line the discovery needed:

- **A negative measured alongside a known-wrong neighbour is weak** — `execute`'s
  `mAnim.play()` regressed in that state and was correct once the neighbour was
  fixed.
- **A negative that survives a retest after real bugs are fixed is strong.**

**So the rule is: re-test after fixing a genuine bug, and treat what survives as a
wall.** That costs one recompile per variant and converts an assumption into
evidence in both directions.

Final state: **11/15**, `.rodata` exact, four characterised residuals — `ctor` 4,
`create` 13, `dtor` 21 (twice-confirmed across two units), `createModel` 79 (four
negatives across three mechanisms). Working tree clean.

## A base vtable written then overwritten is ORDINARY DERIVED-CLASS CONSTRUCTION

WM_HANACHAN's scout flagged an unexplained pattern: a `m3d::fanm_c` constructed
at `+0x3cc`, whose vtable is then **overwritten** with `__vt__Q23m3d8anmChr_c`
immediately afterwards.

That is not an anomaly. The headers give the chain:

```
m3d::anmChr_c : public fanm_c : public banm_c
```

Constructing an `anmChr_c` runs `fanm_c`'s constructor first — which installs
`fanm_c`'s vtable — and then `anmChr_c`'s own constructor installs the derived
vtable over it. **So the member is an `m3d::anmChr_c`, not a `fanm_c`.**

**A vtable written and then replaced during construction identifies the DERIVED
type, and the sequence of vtables names the whole inheritance chain.** Read it as
information rather than as a puzzle.

Sixth case today where checking `include/` dissolved something reported as
unexplained or unowned.

## WM_HANACHAN scouted: the largest class of the session

`daWmHanachan_c : public dWmDemoActor_c`, **`sizeof 0xf00`** — read off
`classInit`'s `li r3, 0xf00` before a line was written, and confirmed by the
layout arithmetic closing exactly: the last member is a 5-element array of a
custom `0x38`-byte struct at `+0xde8`, and `0xde8 + 5 * 0x38 = 0xf00`.

**When the final member's offset plus its size equals `sizeof`, the layout is
closed** — that is a complete check, not a plausible one.

Dominated by embedded arrays: `m3d::mdl_c[4]`, `m3d::anmChr_c[4]`, a single
`mdl_c`, the `anmChr_c` above, `m3d::smdl_c[5]`, a **200-element** `0xc`-byte
array at `+0x484` (a position-history buffer, `mVec3_c`-shaped by its real
destructor), and the custom 5-element array whose constructor and destructor are
**both functions inside this unit's own 32** — so the element type is declared by
this class itself.

### The ownership check caught an over-wide claim

Two rounds of bound iteration: several upper bounds were too short (`check_bounds`
reporting "END cuts short" against a real symbol — one object alone is `0xc8`
bytes), and **one over-wide `.data` guess swallowed the NEXT unit's
`g_profile_WM_ITEM`, caught by the ownership check rather than by size.** A
same-size wrong-owner claim is exactly what that check exists for.

## A VIRTUAL DESTRUCTOR CONSUMES TWO VTABLE SLOTS, not one

WM_KINOPIO found `dPyMdlBase_c`'s real vtable offset **+2 slots** from a naive
declaration-order count off its header: the **scalar and vector deleting
destructor pair** occupies two slots, not one.

**Two independent virtual-dispatch call sites were both 8 bytes high**, and that
identical error is what made it diagnosable — **a systematic offset shared by two
call sites is a counting error, not two separate bugs.**

Add this to the vtable-walking procedure already recorded: a derived class's
vtable is its base's slots overridden in place then new slots appended, **and a
virtual destructor in that chain costs two slots.**

## Including `d_wm_lib.hpp` costs a `.ctors` entry — and some units' targets do NOT include it

`sc_ForceList` is a namespace-scope `static` array *with an initialiser* in
`d_wm_lib.hpp`, so **every TU that includes the header gets its own copy and its
own dynamic initialiser**, adding a `.ctors` entry. Five units today matched
*with* that — their targets do include it.

**WM_KINOPIO's target does not.** Its `.ctors` has one entry where the draft has
two, purely because the draft included the header to reach `dWmLib::IsSingleEntry()`.

**The fix is to declare, not include:**

```cpp
namespace dWmLib { bool IsSingleEntry(); }   // in the .cpp, no include
```

A declaration with no definition pulls in nothing — the same technique this
project already uses for `fn_80103420` and the `R_2_1_*` externs. Hand-mirroring
`sc_ForceList` locally was tried and made things *worse*; the point is to declare
only the **function you call**, not to reproduce the header's data.

**So the `.ctors` entry count tells you whether the target's TU included a header
with a self-initialising static.** That is a cheap, decisive signal about the
original source's include list, and it is worth checking before assuming a unit
should include what its siblings include.

## Converting vtable BYTE OFFSETS to SLOT NUMBERS resolves overrides without a draft

WM_HANACHAN read its vtable's relocations directly — `check_vtable.py` needs a
dtk-dumped target data object, and unlanded units do not have one. It got five
own-address slots but declined to map them to functions by size, which was right.

**The slot numbers plus the recorded lifecycle-adjacency rule resolve four of
them outright.** Convert with `slot = (offset - 0x08) / 4 + 2`, then apply *each
lifecycle stage's pre/post hooks sit directly after that stage's own action slot*:

```
2  create     3 preCreate    4 postCreate
5  doDelete   6 preDelete    7 postDelete
8  execute    9 preExecute  10 postExecute
11 draw      12 preDraw     13 postDraw
```

which reproduces the observed 3-slot spacing exactly, and matches
WM_KILLERBULLET's independently confirmed map (`create` 2, `doDelete` 5,
`execute` 8, `draw` 11, `processCutsceneCommand` 24).

**Cross-checked independently**: the 8-byte function landing on slot 5 is
`li r3, 0x1; blr` — the family's standard `doDelete` one-liner. Two routes
agreeing is as strong as this gets.

**So an unlanded unit's overrides can be identified with no draft at all**, from
the vtable relocations plus arithmetic. That removes the chicken-and-egg problem
where `check_vtable.py` needs a draft to produce the map you need to write one.

## Reading a trivial constructor/destructor pair identifies a struct as POD

WM_HANACHAN's custom `0x38`-byte element type has a **4-byte constructor**
(`blr`, empty) and a **0x40-byte destructor** of the standard vector-deleting
shape with an otherwise empty body.

**Together those prove the struct is plain data** — no nested objects, no owned
pointers. That is a complete answer about a type's nature from two very small
functions, and it is worth reading them early: it tells you whether the rest of
the layout work needs to account for construction at all.

One field was then pinned from a *user*: a 0x1c-byte function copying three
floats from `array[0] + 0x10` into the inherited `mPos` fixes an `mVec3_c`-shaped
field at struct offset `+0x10`.

## The registration trigger is DYNAMIC INITIALISATION, not a non-trivial destructor

WM_KINOPIO tried to avoid a spurious `.ctors` entry by hand-mirroring
`ForceInCourseList_t` instead of including `d_wm_lib.hpp`. **All three variants
made MWCC stop emitting `__register_global_object` entirely**, losing the
array-destructor callback that was already matching.

The agent correctly ruled out the obvious explanation: `mVec3_c` has a
user-declared destructor in both the real header and the mirror, so
non-triviality is not the discriminator.

**The discriminator is how the array is INITIALISED.** The real one:

```cpp
static ForceInCourseList_t sc_ForceList[] = {
    {WORLD_7, "F7C0", WORLD_7, dCsvData_c::c_CASTLE_ID, 4, "W7C0",
     mVec3_c(2160.0f, -30.0f, -478.0f)}      // a CONSTRUCTOR CALL
};
```

A **constructor call** in the initialiser makes the array dynamically
initialised, which is what forces the registration and the destructor callback.
Brace-initialising the vector as `{2160.0f, -30.0f, -478.0f}` instead lets MWCC
fold the whole aggregate into `.data` at compile time — no runtime write, no
registration.

**Same mechanism as castle's**, where a hand-rolled POD member removed the
registration and took `.rodata` and `.bss` *under*; and the same
brace-init-versus-constructor distinction that closed castle's `__sinit`.

**So: to reproduce a registration, the initialiser must call a constructor.**

## Two ordering constraints that CONFLICT mean a NAMED constant

WM_KINOPIO's `resetPosition` (3 differing) looked like a genuine deadlock: the
shared constants pool in function-**definition** order, but the target's pool
order implies the opposite function order to the one `.text` requires, and both
functions are strong fixed-address symbols that cannot be reordered.

**A real conflict between those two constraints means the constants are not both
anonymous pool entries.** An anonymous literal pools with its function; a **named
`static const` pools at its own declaration point**, independent of where any
function sits. Declaring the contested constant as a named object, positioned to
satisfy the pool while the functions stay in target address order, satisfies both.

That is the mechanism behind course's pool fix and antlion_mng's, and **it is the
only way the real source could produce an order that a purely
anonymous-pool shape cannot.**

## "1 differing, and it is the same offset 4 bytes out" = a MISSING MEMBER

WM_HANACHAN's three state-setters each compiled to exactly **1 differing**:
`stw r0, 0x478(r3)` against the target's `0x47c`. Four bytes, everywhere, in
every function that touched the region.

Root cause: the `+0x184` convention — **`dWmDemoActor_c`'s base ends at `0x184`
and each derived class's own first member sits there** — had been skipped, so
`mAllocator` started at `0x184` instead of `0x188`. **Adding the missing
`int mUnk184;` fixed all four functions plus the destructor in one shot.**

**A constant small offset error repeated across unrelated functions is a layout
bug, not four bugs.** The same reasoning identified a vtable counting error on
another unit today from two call sites being 8 bytes high. **Look for the shared
delta before looking at any individual function.**

Note the `+0x184` convention has now appeared on antlion_mng, board, killer,
killerbullet and hanachan — **every `dWmDemoActor_c`-derived unit this session
starts its own members at `0x184`, with an unused `int` there.** Put it in the
skeleton before authoring anything.

## Vtable slot 18 is the DESTRUCTOR

Identified by reading the function rather than by elimination: full member
teardown via `__destroy_arr` on every array member **in reverse declaration
order**, then the base chain, then the optional `fBase_c::operator delete`. That
is the destructor shape from every landed sibling this session.

So the family's slot map is now: **2 create, 5 doDelete, 8 execute, 11 draw, 18
destructor, 24 processCutsceneCommand.**

## A partially-authored unit will report FUNCTION ORDER IS WRONG. Expect it.

With 7 of 32 functions written, MWCC bunches **every** weak helper the draft could
need — base destructors, `EGG::Vector3f`'s, `anmChr_c`'s ctor/dtor, the unit's own
custom struct helpers — into one group immediately after the first explicit
function, **because nothing later in the file references them yet to pull them
toward their real positions.**

The target needs only two of them that early. **This resolves as more functions
are authored and must not be "fixed" by reordering what is already written.**
Another unit today carried an identical warning right up to the moment it linked
cleanly.

## The constructor-call theory does NOT explain WM_KINOPIO's `.ctors` gap

I proposed that a hand-mirrored `sc_ForceList` failed to emit
`__register_global_object` because its vector field was brace-initialised rather
than constructed. **The agent checked its existing mirror, found it already used
`mVec3_c(2160.0f, -30.0f, -478.0f)` — a genuine constructor call — retried
explicitly, and got the identical result: no registration, no callback.**

So the brace-versus-constructor distinction, **whatever it correctly explains on
castle**, is not the discriminator here. Something else about including the real
header versus mirroring it drives the registration, and it is not yet identified.

**Verifying that a lead's premise is already satisfied, rather than assuming an
earlier attempt got it wrong, is the right response to a theory from above.**
Two agents have now falsified one of my hypotheses this way today.

Two further clean negatives on the same unit: named-constant positioning for a
pool rotation, tried with all four constants named and again with only the
genuinely shared one — **byte-identical both times**; and binding a float result
to an explicit local to force a non-volatile-register-across-call shape — the
instruction sequence was unchanged.

## Prefer an UNWRITTEN function over a fourth variant on a known wall

WM_KINOPIO stands at 13/19 with three characterised walls totalling 69
differing instructions — and **two functions that have never been attempted at
all**, plus the session's largest single function (`0x834`) deferred four times.

**A round spent on a fourth variant against a measured wall is worth less than a
round spent on a function nobody has read.** The walls are recorded; they will
still be there. This is the same allocation logic that produced six landings
today while near-misses stayed near-misses.

## `+0x184` is DECLARED on every unit, but whether the ctor WRITES it varies

WM_HANACHAN's constructor regressed 11 -> 66 after the correct `mUnk184` layout
fix, and the cause was the initialiser rather than the member: the draft had
`mUnk184(0)`, emitting `stw r31, 0x184(r29)`, while **the target's constructor
never touches that offset at all** — it is zero purely from
`fBase_c::operator new`'s blanket zero-initialisation.

Removing the initialiser while keeping the bare `int mUnk184;` declaration closed
the constructor to MATCH.

**So the convention has two halves and only one is universal:**
- **Declare** the `int` at `+0x184` — five units this session, no exceptions.
- **Initialise it only if the target's constructor writes it.** antlion_mng and
  hanachan do not; board does.

**Check the target's constructor for a store to `+0x184` before adding an
initialiser.** And note the general form, which the lead predicted correctly here:
**a regression caused by a correct change usually means a second error the first
one was masking.**

## The state-dispatch table, decoded the same way as the vtable

`lbl_2_rodata_88CC`, **4 entries of `0xc`** resolved from relocations —
`fn_2_164EB0`, `0x1650A0`, `0x165120`, `0x1651B0` — matching `mState`'s observed
range of 0..3 from the three `setState` functions.

**Entry 3 is the `0x404` function** that had been deferred as "largest, leave for
last"; it is the `mState == 3` handler. Knowing that gives it a shape before
anyone reads it.

**Direct relocation reading now covers class vtables, pointer-to-member tables and
state tables alike** — no draft and no dtk-split object required, which is what
makes it work on unlanded units.

WM_HANACHAN stands at **12/32**, with `create`, `draw`, the constructor and the
destructor all matching, and `execute` size-matched at 5 differing (two constants
sourced from separate anonymous pool slots instead of one named table).

## WM_KINOPIO 13/19 -> 14/19, and a NEW residual class: paired-single vectorisation

`startJump` matched **first attempt**, with its signature settled purely from
field offsets inside its own body (`+0x4` float, `+0x8` s16, `+0xc`/`+0x10`
floats) — **a parameter struct's shape can be read from the callee's own
accesses**, without needing the caller.

`processCutsceneCommand` (0x230) was authored for the first time and its dispatch
shape confirmed correct — a single if-chain with an early return, not a switch,
matching the target's branch/skip structure exactly.

**Its residual is a class we have not met before.** The target computes the
weighted-position maths with **paired-single instructions** — `ps_muls0`,
`psq_l`/`psq_st`, holding `f30`/`f31` across both calls — while `mVec3_c`'s
`operator*`/`operator+` compile to plain scalar float ops. **No wrong branch,
constant or argument**; it is a vectorisation-level difference.

**Lead for whoever returns to it:** `include/lib/revolution/mtx/vec.h` declares
`PSVECAdd(const Vec *, const Vec *, Vec *)` and
`PSVECScale(const Vec *, Vec *, f32)`, and `m_vec.hpp` already calls
`PSVECSquareDistance`/`PSVECMag`. **If the real source used the SDK's
paired-single helpers rather than `mVec3_c`'s operators, that explains the
instruction class exactly.** One measurement would settle it.

## A jump table with every case address known is twenty small problems

`fn_2_16C810` is `0x834` — the largest function of the session, deferred four
times. The groundwork that makes it tractable is done: **20 entries counted from
relocations** (not guessed), cross-checked against the function's own
`cmplwi r0, 0x13` bound, with all 20 case addresses catalogued.

**A 20-case dispatch with every case's address known in advance is far more
tractable than its size suggests.** Relocation counting has now sized tables
correctly on four units and caught two wrong entry counts.

Reminder for authoring it: with a bound check and a jump table this is a `switch`,
but **case label order sets body layout independently of the compare order the
compiler picks** — take the ordering from the table.

## The same lever can close one function and BREAK its sibling

WM_HANACHAN's `state1` closed on the "two apparent call sites actually converge
on ONE shared call site" reading — the same shape as antlion_mng's
`checkAttackSequenceDone`, rewritten as a single combined condition:

```cpp
if (R_2_1_1994B0(player) == 1 || (player->m_18c && R_2_1_1994D0(player) == 1)) { state4WhenNear(); }
```

**Applying the identical lever to `state2` made it worse** (21 -> 24):
materialising an explicit `bool` produced `li r0,0/1; cmpwi` instead of the
target's direct fall-through branching.

**So "one shared call site" is a reading of the target, not a preference.** Check
the branch targets before combining conditions.

### A live operator-spelling sensitivity, characterised not solved

`state2`'s residual is now specific: the target uses a `cror`-combine on the
**first** of two `>=` comparisons and not on the second — **the same logical
comparison, spelled two ways, compiling to two different instruction shapes.**
That is a source-visible distinction nobody has pinned down yet, and it is a
better handoff than "21 differing".

## WM_HANACHAN paused at 13/32 — a responsible stop, not a wall

Vtable fully mapped, layout confirmed and closed against `sizeof`, seven
functions closed across two rounds, and **every open item has a specific recorded
next step** rather than an unknown:

```
state2        21   operator-spelling sensitivity, characterised above
execute        5   two constants from separate anonymous pool slots, want one named table
createModel   --   structure fully read; needs an unhurried reconstruction of a
                   PowerPC int-to-double bit-pattern expression (xoris + lfd/fsub/fmul)
state3        --   fn_2_1651B0, 0x404, known to be the mState==3 handler
6 functions   --   never examined
```

**Declining to rush `createModel` was right** — this project has already misread
that exact bit-pattern idiom once, taking a single double for two floats.

## The `+0x60` "secondary vtable" needs NO special handling — third confirmation

WM_KINOPIO's case 19 calls `setCutEnd()` — an ordinary virtual — and it compiled
to the exact `this+0x60`-based dispatch shape **automatically, with no
vtable-pointer code written at all.** Same as `execute()`'s
`processCutsceneCommand(...)` call in the same unit, and the same as anchor's
finding that `+0x60` is the secondary vtable pointer written by ordinary
construction.

**What looked like a special "secondary vtable" is just this hierarchy's own
vtable at a non-zero base offset, and the compiler handles it for any normal
virtual call.** Three independent confirmations. **Write the ordinary call; never
hand-roll the dispatch.**

## A jump-table case can have ZERO bytes

WM_KINOPIO's case 3 has **no unique code at all** — its jump-table entry points
straight at the function's shared epilogue. In source that is an empty `case 3:`
falling to the common exit.

**A case address that equals the epilogue is an empty case, not a missing read.**
Worth recognising before hunting for a body that does not exist.

## WM_KINOPIO's largest function: 9 of 20 cases authored

`fn_2_16C810` (`0x834`) went from untouched to a real compiling `switch` with 9
cases decoded and authored in ascending size, 11 still placeholders.

**A countdown-timer idiom recurs in 4 of the 9** —
`if (m_198 > 0) m_198--; else { transition }` — which is the dominant per-state
pattern and should make the remaining 11 substantially faster to read. **Finding
the repeated idiom early is what makes a 20-case dispatch tractable.**

Open on it: case 2 writes into a shared `.bss` singleton (`lbl_2_bss_11B70`) at
raw offsets via byte casts because its real type was not identified — a real gap
worth closing rather than leaving as casts. Case 14 has only its first call read.

## Triage a lever by APPLICABILITY before trying it

course's agent was handed six levers discovered since the unit was parked. It
**checked each against the actual residual before spending a compile on it**, and
reported why four of them cannot apply:

```
declaration order controls stack slots  -> no stack temp here; the store is to a MEMBER (this+0x238)
"what is in the stack temp"             -> there is no stack temp in the 6-instruction window at all
ternary merges calls / De Morgan        -> no ternary, and no if/else pair to invert -- a single
                                           unconditional store then a single-branch if with no else
"do not cache what the target re-reads" -> already measured; the surrounding code reads mParam twice
                                           and caching it is on record as worse
```

It then tried the **two that were genuinely untried** — moving the specific local
across the store (**8, worse**) and naming the comparison's boolean result
(**byte-identical, the lever produced literally no change here**) — and stopped.

**A lever is a reading of the target, not a preference to apply blindly.** That
was established when the same "one shared call site" lever closed one function and
broke its sibling, and this is the discipline generalised: **check that the
construct the lever addresses is actually present before measuring it.**

Note *why* the toolkit does not reach this residual: **this session's levers are
overwhelmingly about control-flow shape and stack-temporary identity, and course's
`createModel` residual has neither.** It is a pure register-assignment preference
on a direct member store. That is a useful characterisation of the toolkit's own
coverage, not just of this function.

**course stays parked at 22/23**, now with 21 measured variants and an explicit
account of which levers cannot apply.

## WM_KOOPAJR scouted — clean handover position, no functions authored

`daWmKoopajr_c : public dWmDemoActor_c`, **`sizeof 0x360`** read from
`classInit`'s own allocation before anything else. Layout closes exactly:

```
+0x184  int mUnk184        -- NOT written by this ctor, so declare without an initialiser
+0x188  dHeapAllocator_c mAllocator
+0x1a4  int (= 0)
+0x1a8  m3d::mdl_c mModel
+0x1e8  m3d::anmChr_c mAnimChrs[6]        0x1e8 + 6 * 0x38 = 0x360
```

Slice validated BOUNDS PLAUSIBLE:

```json
{".text": "0x16d290-0x16e540", ".ctors": "0x410-0x414", ".data": "0x45dd8-0x45f90",
 ".rodata": "0x8ba0-0x8c90", ".bss": "0xfec0-0xfed0"}
```

**The `+0x184` refinement was applied correctly on its first use** — the member
declared, the initialiser omitted because this constructor does not store there.

**And the ownership-warning rule paid off again**: the first `.data` guess made a
function look like a neighbour's, purely because the claim did not yet reach the
vtable object referencing it. Same pattern as board — **an ownership warning on a
function means the neighbouring-section bounds are wrong, not the `.text` claim.**

`fn_2_16D940` is `0xA60` — 2,656 bytes, larger than most entire units this
session, and unexamined.

## Session summary — where the WM region of `d_basesNP` stands

```
LANDED this session (6):  antlion, sandpillar, kinoko_star, sinkship, note, manta
LANDED previously (13 total incl. above): + cannon, cloud, dokan, dokan_route,
                          ghost, grid, kinoko_1up, kinoko_base, kinoko_red,
                          peach, peach_castle, smallcloud, tower

PARKED, one function short:   course 22/23   castle 19/20   koopa_castle 16/17
                              killer 22/23
PARKED, further out:          kinoballoon 24/26   antlion_mng 18/22
                              start 11/14   board 11/15   anchor 16/22
                              item 8/12     dance_pakkun 9/16
IN PROGRESS:                  kinopio 14/19 (9 of 20 jump-table cases authored)
                              killerbullet 17/37   hanachan 13/32
SCOUTED ONLY:                 koopajr (bounds + layout confirmed, 0 functions)
```

**Progress 11.250% -> 11.471%**; `d_basesNP` 1.704% -> 2.354%.

**The planning fact this session established:** every unit that landed reached
N/N within one or two rounds. Every parked unit is one to four functions short on
scheduling or register-allocation residuals. **A unit that does not close quickly
tends not to close at all** — so prefer opening a fresh unit, or authoring an
unwritten function, over a fourth variant against a measured wall.

## `lbl_2_bss_11B70` — a shared singleton whose type is genuinely UNIDENTIFIED

WM_KINOPIO's `stepCutscene70` writes into it, and the search was exhaustive
rather than skipped:

- **Ownership check: 130+ relocation references spanning nearly the whole
  module** — confirming a widely-shared singleton, not a unit-local object.
- **Three candidates checked against the confirmed field offsets**
  (`+0x544`/`+0x545`/`+0x546`/`+0x54d` booleans, `+0x55c` an int):
  `dCsSeqMng_c` (documented fields stop at `0x1b4`, far short),
  `dWmEffectManager_c` (no data members at all), `dGameKey_c` (wrong shape).
- **Grepped all of `include/` for the literal offsets** — five hits, every one an
  unrelated enemy-actor class.

**No match. The raw offset-cast form is kept with every offset documented**, which
is the established handling used by two landed units. Recorded as a real negative
so the next person does not repeat the search — and as a genuine gap: a singleton
this widely referenced is worth identifying properly at some point.

## WM_KINOPIO's largest function: 12 of 20 cases

Up from 9. Three new cases, each teaching something:

- **Case 8 reads live controller input** —
  `dGameKey_c::m_instance->mRemocon[mPad::g_currentCoreID]->mDownButtons & 0x900`
  — with both types already declared in `include/`.
- **Case 11 identified `m_1b0`**, previously recorded as "unobserved", as an
  **effect ID passed to `dWmEffectManager_c::endEffect()`.** A field's meaning can
  fall out of a single case body.
- Case 14 is explicitly labelled **incomplete** — one confirmed call out of an
  estimated four or five — rather than left looking finished. **Labelling a
  partial case as partial is what keeps the pairing honest** on a 20-case switch.

Remaining: cases 0, 1, 5, 10, 12, 18 untouched; 14 partial; 2, 6, 8 complete but
using the unidentified singleton's offsets.

## WM_KOOPAJR: six roles named with ZERO lines of draft written

The vtable-relocation technique at its best. The class's own secondary vtable
(`lbl_2_data_45F08`, installed at `+0x60` by the constructor) read directly via
`dtk rel info -r`, converted with `slot = (offset - 0x08) / 4 + 2`, matched
against the family map:

```
slot  2  +0x08  fn_2_16D3F0  create                  0x64
slot  5  +0x14  fn_2_16D580  doDelete                0x8   <- the family's trivial 8-byte convention
slot  8  +0x20  fn_2_16D460  execute                 0xD0
slot 11  +0x2c  fn_2_16D530  draw                    0x4C
slot 18  +0x48  fn_2_16D340  destructor              0xAC
slot 24  +0x60  fn_2_16D870  processCutsceneCommand  0xB0
```

Every other family slot resolved to the same external inherited targets seen on
every prior unit — a consistent cross-check that the class overrides nothing else.

**And the negative is as valuable as the map: `fn_2_16D940` (0xA60, the unit's
largest by far) is NONE of the six.** It is a plain non-virtual member. Ruling out
the obvious candidates before reading a 2,656-byte function is worth a great deal.

### A 2-entry function-pointer table, found by scanning for the stride

Searching the same relocation dump for consecutive `0xc`-strided `Absolute`
entries pointing into the unit's own `.text` found exactly two, at
`0x8c18`/`0x8c24` — the `procNone`-plus-one-real-handler shape already seen on
kinoballoon and hanachan.

**A third candidate was correctly excluded**: it sits `0xdc` away and its target
falls inside the neighbouring unit's claimed range, so it belongs to that unit's
own trailing table. **Check a candidate entry's target against the claim before
counting it.**

**Handover state: bounds validated, `sizeof` closed by layout arithmetic, member
layout confirmed, six of twenty functions named — and no draft code at all.**
That is the cheapest possible position for the next round to start from.

## TRAP: a load's DISPLACEMENT is not an address suffix

WM_KINOPIO's agent rebuilt its `.rodata` table from first principles rather than
reusing shortcuts, and caught a real error in its own earlier work: **displacement
digits had been read as absolute addresses.**

```
lfs f1, 0x48(rN)   where rN = lbl_2_rodata_8B10
  WRONG:  0x8b48        <- the displacement pasted onto the base's leading digits
  RIGHT:  0x8b10 + 0x48 = 0x8b58
```

Two cases had wrong constants from this — `setAnm(0, -500, 0.25, 0.5)` where the
target has `setAnm(0, 1.0, 20.0, 0.0)`. **The trap is that the base and the wrong
answer share leading digits**, so the result looks plausible.

**Always add the displacement to the base register's actual value.** Neither case
had been showing as matched, so nothing false was masked — but this is exactly
what the "read the diff, do not trust an improving count" caution exists to catch,
and the agent found it by rebuilding rather than by being told.

## Two cases stopped on real header gaps, correctly

- **Case 12** indexes `daWmMap_c`'s internal model array by a computed stride and
  touches six `dWCamera_c` fields at `0x5f0`-`0x71c` — **far beyond that header's
  documented `pad[0x4f8]`**, confirming again that `dWCamera_c`'s real layout is
  much larger than `include/` captures. Two landed units already write that region
  through a local cast.
- **Case 10** hit a vtable dispatch **whose return value is used by a later call,
  while the header declares that slot `void`.** So the header is wrong at that
  slot. The agent left it unauthored rather than hand-count a slot number —
  **exactly the failure mode the "+2 slots for a virtual destructor" lesson warns
  against.**

**Declining to guess a slot is right.** Walk the vtable chain from the headers, or
read the class's own vtable relocations; do not count by hand.

WM_KINOPIO's largest function stands at **14 of 20 cases**, with 0 and 1 (the two
largest remaining) unattempted, 14 partial, and 2/10/12 blocked or offset-dependent.

## HEADER CORRECTED: `dPyMdlBase_c::getBodyMdl()` returns `m3d::mdl_c *`, not `void`

**Applied to `include/game/bases/d_player_model_base.hpp`, five binaries re-verified
green.** Proven, not inferred:

- **Vtable byte offset `0x28` -> compiled slot 10 -> declaration index 8** (`10 - 2`
  for the destructor pair), landing precisely on `getBodyMdl()` in the header's
  own declaration order. **The slot arithmetic and the declaration order agree**,
  which is what made it safe to change.
- **The type came from a LANDED precedent, not from the method's name.** Both call
  sites pass the return straight through as the third argument to
  `fn_80103520(dWmEffectManager_c *, int, m3d::mdl_c *, const char *, int, int)`
  — the identical argument shape as `fn_80103420` in the landed
  `d_a_wm_kinoko_base.cpp`, whose corresponding parameter is `m3d::mdl_c *model`.
- **Proof was a byte-for-byte match**, not a plausibility argument: with the
  correction, both cases compile the dispatch exactly
  (`lwz r12,0x0(r3); lwz r12,0x28(r12); mtctr; bctrl`) followed by `mr r5,r3`
  passing the result onward. **Before the fix that code path could not exist at
  all**, since the method had no usable return value.

**Seventh wrong return type found on this project, and the first found through a
vtable slot rather than a mask or width narrowing.** Return types are absent from
the mangled name, so a call site consuming a `void` method's result is a header
defect by definition — and two independent occurrences are what make it
actionable rather than a misread.

**The workflow that produced it is now three-for-three today:** agent proves the
change in a shadow header, reports the exact declaration and the evidence, lead
applies it behind a full five-binary verify. `dWmLib::getZoromeTime`,
`dWmLib::isStartPointKinokoHouse1up`/`IsSingleEntry`, `daWmMap_c::GetPos`,
`cLib::addCalcPos` and now this one all went in that way.

WM_KINOPIO's largest function is at **18 of 20 cases**. Remaining: case 12 blocked
on `dWCamera_c`'s real layout extending past its documented `pad[0x4f8]`, and case
14 partial.

## `lbl_2_bss_11B70` IDENTIFIED — and the earlier search failed on a FALSE PREMISE

**It is a 4-byte POINTER, not the object.** The previous exhaustive search
(recorded above as "a shared singleton whose type is genuinely UNIDENTIFIED")
looked for a class whose layout extends to `+0x55c` and found nothing. It could
not have found anything: **the bss label is a global instance pointer, and the
`+0x544`/`+0x55c` offsets are fields of the POINTED-TO object.**

The access shape says so unambiguously — every one of the 65 reference pairs is a
double indirection:

```
lis  r3, lbl@ha
lwz  r5, lbl@l(r3)      <- LOADS A POINTER out of .bss
stb  r4, 0x544(r5)      <- field access off the LOADED value
```

**This is the `DISPLACEMENT is not an address suffix` trap wearing a second hat.**
The trap is usually stated as "do not read `lwz r3, 0x1234(r5)` as address
0x1234". The stronger form: **a large displacement off a register that was itself
loaded FROM .bss means the .bss cell is a pointer, and the object is on the heap.**
Confirmed by the relocation stream: `lbl+0x544` is never a relocation target, and
would have to be if the field were addressed directly.

### What it actually is

**The `COURSE_SELECT_MANAGER` singleton instance pointer.** Every step verified,
none inferred:

- **130 .text relocations = 65 reference pairs**, classified by the opcode of the
  patched instruction: **68 `lwz` (read), 2 `stw` (write), 60 `lis`.**
  **Exactly two writes is the singleton create/destroy signature** — and both
  writes fall inside `.text 0x1c18b0-0x1c57f0`, which `profile_map.py` resolves to
  `g_profile_COURSE_SELECT_MANAGER`.
- **`sizeof` = `0x570`**, read straight off the allocation:
  `li r3, 0x570; bl __nw__7fBase_cFUl` at `0x1c18b8`.
- **Derived from `dBase_c`** — the constructor calls `__ct__7dBase_cFv`
  (DOL `0x8006c420`) with `r3 = this`.
- **Contains a `dCourseSelectGuide_c` at `+0xC8`** — `addi r3, 200(r30)` then
  `bl __ct__20dCourseSelectGuide_cFv` (DOL `0x8000fc30`). That class is the world
  map HUD and is **already decompiled in our tree**
  (`include/game/bases/d_CourseSelectGuide.hpp`, `source/dol/bases/`).
- The pointer is stored with `stw r30, lbl@l(r3)` where `r30` is the value
  returned by `operator new` — i.e. **`this`**. The destroy site stores literal
  zero.
- Constructor also inlines stores at `+0x518/+0x51c/+0x520/+0x524/+0x53c` and
  constructs an `sStateMethodUsr_FI_c`. All known offsets are `< 0x570`, so the
  layout is self-consistent.

**The C++ class NAME is still unknown** and that is fine — it is an undecompiled
REL class, so no header in the tree names it. What is now known is its profile,
its size, its base, and one member's exact type and offset. **Resolve external
`bl` targets through `bin/dtk/wiimj2d_symbols.txt`** — that is what named the
base constructor and the member, and it is a far bigger symbol map than
`syms.txt` (which is only our curated project list and does not contain them).

### The reusable method

**Classify a bss label's references by the OPCODE of the patched instruction
before theorising about its type.** Read-vs-write counts alone identify the
pattern: two writes and many reads is a singleton pointer; many writes is state;
`addi`-only is address-taking. It cost four commands and settled a question an
exhaustive type search had already failed to answer.

## A unit reported "one function short" WAS NOT LINKABLE — check function ORDER early

WM_KILLER sat on record at 22/23 for rounds. Its `build.py` `verify_anon` was
reporting **"FUNCTION ORDER IS WRONG"** the whole time, and nobody acted on it,
because the tally looked like the only thing standing between the unit and a
landing.

It was not. The draft defined `unk_168590()` before `unk_1684A0(bool)`; the
target places `0x1684a0` immediately before `0x168590`. **The linker places
`.text` in definition order, so this unit would have failed to link even at
23/23.** Fixed by swapping the two definitions (pure reordering — `unk_1684A0`
calls `unk_168590` regardless of definition order, since both are declared in the
class header). Order warning gone, tally unchanged at 22/23.

**The lesson is about WHEN, not what.** Function order is free to check and free
to fix, and it is invisible in the per-function diff that drives the tally. A
matched-function count is NOT a landability measure:

- **N/N is necessary, not sufficient.** Order, `.rodata`/`.data` placement,
  `.ctors` presence and pool contents are all separate gates.
- **Check order on the FIRST round of a unit, not at landing time.** A unit that
  spends five rounds at N-1/N with a standing order warning has been burning
  rounds against a residual while a second, unrelated blocker sat unfixed.
- It is checkable WITHOUT building: the target address is in the function's own
  name (`unk_1684A0`, `fn_2_16D940`), so **definition order in the source must be
  ascending by that address**. That is a text comparison, not a compile.

Recorded because the misjudgement was one of PRIORITY, not of technique: the
warning was on screen, in the tool's own output, for every round.

## The singleton method GENERALISES — four more resolved, and one that self-flags

`bss_classify.py` + `resolve_singleton.py` run over the whole module, not just
the one label that prompted them. Every `.bss` singleton in `d_basesNP` now
resolves to a profile, a `sizeof` read off its own allocation, and a base class:

```
lbl_2_bss_11B70  COURSE_SELECT_MANAGER              sizeof 0x570  : dBase_c
                 + sStateMethodUsr_FI_c, + dCourseSelectGuide_c @ +0xC8
lbl_2_bss_C778   MINI_GAME_WIRE_MESH_MGR_OBJ        sizeof 0x708  : dActor_c
lbl_2_bss_5AE8   BGM_INTERLOCKING_DUMMY_BLOCK_MGR   sizeof 0x400  : dActor_c
                 + sStateMethodUsr_FI_c
lbl_2_bss_C460   MINI_GAME_GUN_BATTERY_MGR_OBJ      sizeof 0x0F4  : dActor_c
lbl_2_bss_FEE0   WM_KOOPASHIP                       CANDIDATE ONLY (see below)
```

**`sizeof` here is read, not derived.** It is the immediate in the `li rN, SIZE`
feeding `operator new` — so it is exact, and it does not depend on having found
every field. That is a much better starting point for a class layout than
bounding it from the highest offset any function happens to touch.

### The one that self-flags is the important one

`lbl_2_bss_FEE0` (WM_KOOPASHIP) reports `sizeof 0x40` — and the tool warns,
because **the allocation it found sits `0x28C` bytes before the pointer store.**
That distance is the tell: in the four clean cases the allocation is within
`0x120` of the store, because the shape is tight (`li size; bl new; bl ctor;
stw ptr`). A far allocation is likely a MEMBER or a temporary — and here the
intervening calls include `__dt__11dWmSpline_cFv`, so `0x40` is plausibly the
size of a `dWmSpline_c`, not of the KOOPASHIP singleton.

**The tool does not prove the stored pointer is the one `operator new` returned**,
and it now says so in its own output rather than presenting every result with
equal confidence. Treat a warned result as a candidate until the dataflow is
checked by eye. An honest "candidate" is worth more than a confident wrong
`sizeof` that someone then builds a class layout on.

### Two rules worth carrying

- **Resolve external `bl` targets through `bin/dtk/wiimj2d_symbols.txt`**, the
  full DOL map. `syms.txt` is our small curated list; the base-class and member
  constructors are simply not in it. Searching the wrong map and finding nothing
  is not evidence of absence.
- **Never report a fixed lookback window as "the constructor's calls."** The
  first version of this tool did, and it listed `strcmp`, `strrchr` and
  `sStateID_c` — the callees of the PRECEDING function — as if they were members
  of three different classes. Scope the window to the allocation, or print
  nothing.

### The order check covers 3 drafts out of 56 — and now SAYS so

`check_fn_order.py` recovers the target address from the function's own name
(`unk_1684A0`, `fn_2_16D940`). **Drafts that use real names carry no address in
the source text, so the check cannot see them at all** — 53 of 56 files.

Its first version returned silently on those and printed only a findings count.
That reads as a clean bill of health for the whole tree, which is worse than not
running it: it converts "not checked" into "checked and fine". It now lists every
unchecked file by name and points at the unit's own `build.py`, whose
`verify_anon` step catches the same defect after compiling.

**A sweep that cannot see most of its targets must say which ones**, or its
summary line is a lie by omission.

## RETRACTION: `dWCamera_c`'s layout gap is NOT a header defect. It is already solved.

I recorded case 12 of WM_KINOPIO's `stepCutscene70` as "blocked on a second,
unrelated header gap — `dWCamera_c`'s real layout extends past its documented
`pad[0x4f8]` — not this unit's to fix." That framing was wrong, and it would have
cost a round: the next agent would have gone off to prove a header change that
nobody needs.

**The project already has an accepted technique for this gap, in LANDED code.**
`source/d_basesNP/bases/d_a_wm_note.cpp:164` writes past the documented padding
with a local raw cast confined to its own `.cpp`:

```cpp
dWCamera_c *camera = dWCamera_c::m_instance;
u8 *cam = (u8 *) camera;
```

`wip/wm_units/agent_start` uses the same shape. So the correct fix for case 12
was **no header change at all** — re-derive the offsets from the unit's own
disassembly, confirm they agree with the landed precedent, and write the same
local cast.

**The lesson is the ordering of two checks.** "Is this a header defect?" is the
SECOND question. The first is **"has a landed unit already hit this, and what did
it do?"** `getBodyMdl` was a genuine header defect and the shadow-header workflow
was right for it. This looked identical — same class, same kind of gap — and was
not. What separated them was one grep of `source/` for landed precedent, which
costs a few seconds and was skipped on the strength of the resemblance.

A related over-claim corrected in the same round: `daWmMap_c::mModels`/`currIdx`
were assumed missing from the real header. They are correctly declared;
`offsetof(currIdx) == 0x338c` was verified directly. Only one genuinely absent
method (`dWmMapModel_c::GetEndNodePos`) needed a raw extern.

**All 20 of `stepCutscene70`'s cases now have authored code** (was 18). No logic
or missing-case gaps remain in it.

## Two bugs in my own singleton resolver, and the WM_KOOPASHIP answer

I shipped `resolve_singleton.py` with two defects. Both produced confident,
wrong-looking output, and one of them was masked by a warning that fired for the
wrong reason.

**Bug 1: the create is NOT the lower-addressed write.** The tool sorted the two
write sites and called the first one "create". **WM_KOOPASHIP's destroy sits at
the LOWER address** (`0x16EBB0`, storing a literal zero) and its create at the
higher (`0x16FC38`). So the tool hunted for an allocation backwards from the
TEARDOWN, found an unrelated nearby `li r3, 0x40`, and reported it as `sizeof`.

The fix is to read the stored VALUE, not the address: **the destroy is the write
whose stored register is fed by `li rN, 0`.**

**Bug 2: proximity is not proof.** Even after fixing that, the tool was still
only assuming that an allocation near the store is the object being stored. It
now TRACES THE REGISTER: `operator new` returns in `r3`, the value is carried
through `mr` moves (and through a constructor, which returns `this` in `r3`), and
the tool checks the register actually stored is in that live set. All five
singletons now report **dataflow VERIFIED**. A result that cannot be traced says
`NOT VERIFIED` and tells the reader the `sizeof` probably belongs to a member or
a temporary.

### The corrected table

```
lbl_2_bss_11B70  COURSE_SELECT_MANAGER              sizeof 0x570  : dBase_c
                 + sStateMethodUsr_FI_c, + dCourseSelectGuide_c @ +0xC8
lbl_2_bss_C778   MINI_GAME_WIRE_MESH_MGR_OBJ        sizeof 0x708  : dActor_c
lbl_2_bss_5AE8   BGM_INTERLOCKING_DUMMY_BLOCK_MGR   sizeof 0x400  : dActor_c
lbl_2_bss_C460   MINI_GAME_GUN_BATTERY_MGR_OBJ      sizeof 0x0F4  : dActor_c
lbl_2_bss_FEE0   WM_KOOPASHIP                       sizeof 0x018  : dWmSpline_c
```

**`lbl_2_bss_FEE0` is a `dWmSpline_c *`** — not a manager, which is what the unit
name led me to assume. `__ct__11dWmSpline_cFiif` is called on the allocated
pointer and that same pointer is stored. The `dWmRouteManager_c` constructed
later in the same function is a SEPARATE object; it was only ever in the call
list because it fell inside the reporting window.

**All five were previously reported with equal confidence, and one of the five
was wrong.** That is the argument for the dataflow check existing at all: a tool
that cannot distinguish what it proved from what it assumed will eventually hand
someone a `sizeof` to build a class layout on.

## WM_KILLERBULLET 17/37 -> 19/37, and the order fix that moved the tally by zero

The most valuable change in the round did not move the count at all: **six
function-definition-order inversions cleared**, verified green by
`check_fn_order.py` (17 addressed definitions, ascending). The unit could not
have linked at any tally with those present. This is the second unit in one day
found unlinkable while sitting on a respectable-looking match count — see the
WM_KILLER entry above.

Matched: `unk_168F00` (0xC, a tail-call setting `m_1b0=4` then falling into
`unk_169E10`) and `unk_169B80` (0x40, a wrapping 16-bit counter, exact).

**A layout fact proven the right way:** `int m_1c8` at offset `0x1c8` was
previously unverified padding. `unk_169B80`'s own **word-width `stw`/`lwz`**
proves the width and the offset together. Access width is evidence about type,
and it is stronger than any inference from surrounding fields.

Four functions parked after three genuine attempts each — `unk_1693C0`,
`unk_169080`, `unk_169DA0`, `unk_169E10` — all size-exact or near, with content
and structure confirmed and only register-allocation residuals left.

**Also worth recording, as a discipline rather than a finding:** the agent
counted a residual as closed only when every differing line was a
symbol-naming artifact (a real mangled name against the target's address-only
placeholder), **checked line by line for all 37 functions rather than inferred
from a low diff count.** A tally built any other way is not comparable across
rounds, and this project has had tallies drift for exactly that reason.

An open, falsifiable prediction to settle next round: `unk_168C80`'s 7-diff
pool-offset-short residual is expected to **self-resolve as sibling stubs are
replaced with real code**. If it does, that is a reusable rule about pool
residuals in partially-authored units. If it does not, it is a measured negative.
Either answer is worth having; the hypothesis alone is not.

## "Exactly two writes" is NOT sufficient for a singleton. Dereference is.

I wrote the two-write rule up as the singleton signature and it produced two
false positives immediately:

- **`lbl_2_bss_5B30` (BIGHANA_MGR)** — a plain `int` counter. What the tool called
  the "create" is a DECREMENT: `lwz r3, lbl; addi r0, r3, -1; stw r0, lbl`.
- **`lbl_2_bss_D6EC` (OBJ_WENDY)** — a plain `int` state value: loaded, compared
  against 1, assigned 2.

Both have exactly two write sites. Neither points at anything. Hunting for a
class behind either would have dead-ended exactly the way `lbl_2_bss_11B70` did,
arrived at from the opposite direction — and the write-up would have been just as
confident.

**The real test is whether the LOADED value is used as a BASE REGISTER:**

```
lwz  r5, lbl@l(r3)
stb  r4, 0x544(r5)     <- base register  => POINTER; there is an object
```
```
lwz  r3, lbl@l(r10)
addi r0, r3, -1        <- arithmetic only => VALUE; there is nothing to find
```

`bss_classify.py` now requires dereference before reporting a singleton. All
seven real singletons still classify correctly; the two counters no longer do.

**The general shape of the mistake is worth more than the fix.** A signature that
matched every example I had was promoted to a rule without being tested against
things it should REJECT. Two writes is necessary and not sufficient, and the only
reason I found out is that I went to resolve the two leftovers instead of leaving
them in the table as "unresolved". **An unresolved row is an invitation to check;
a confidently wrong row is not.**

## WM_KOOPAJR: 0 -> 15/20 in one round, and the pool rule predicted the blocker

The clean-handover bet paid. From zero draft lines: **15/20 authored, all six named
target functions logically complete, four of them BYTE-IDENTICAL** (`doDelete`
0/2, `draw` 0/19, destructor 0/43, and the rest differing only on own-symbol
naming).

**`create` and `execute` are blocked, and the block was PROVEN rather than
assumed.** Several shared rodata pool slots (`0x8bc4`, `0x8bd4`, `0x8bd8`,
`0x8be0`, `0x8bec`) appear in NO function the agent disassembled — so they belong
to `fn_2_16D940` (0xA60) or `fn_2_16E3A0`, both unauthored. That is exactly the
recorded rule, "a unit's pool cannot be right while any contributing function is
unwritten", arrived at independently and correctly identified as a reason to STOP
working those two functions rather than a residual to grind.

### Two layout bugs, each caught by a DIFFERENT function's diff

- `mProcState` landed at `+0x338` instead of `+0x33c` — a missing field
  (`mUnk338`). Caught by `execute()`'s diff, **confirmed independently by the
  destructor's own layout.**
- `mUnk35c` landed at `+0x344` instead of `+0x35c`, an `0x18` gap. Caught by
  `resetState()`'s diff and corroborated by the classInit trampoline's
  `li r3, 0x360`, which had been silently emitting `0x348`.

**A silently wrong `sizeof` in a trampoline is not visible in any function's
diff.** It took a second, unrelated symptom to expose it.

### Scouting note CORRECTED

`+0x1a4` is **`nw4r::g3d::ResFile mResFile`, not a raw `int`.** Verified against
landed code: `source/d_basesNP/bases/d_a_wm_kinoko_base.cpp:78` declares
`nw4r::g3d::ResFile mResFile` at the identical `0x1a4`, with the same
`getRes()` -> `GetResMdl()` usage shape. Landed precedent again beat inference
from the raw disassembly.

Also: `resetScaleAndProc()`'s `mScale = mVec3_c(...)` compiled a stack-temp copy;
rewriting as three explicit field assignments closed an 8-vs-13-line size
mismatch. **That is the stack-temp question answering "no temp at all ⇒ direct
field stores"** — a fourth distinct answer observed for it on this project.

**Caveat the agent stated plainly and which I am preserving:** its "match" figures
are raw text diffs, not the project's link-time comparison. Byte-identity here
means identical disassembly text, which is necessary but not the landing gate.

## "FUNCTION ORDER IS WRONG" can be a TOOLING artifact. Here is how to tell.

Both of these are true and they were established hours apart:

- **WM_KILLER's was REAL.** `unk_168590` was defined before `unk_1684A0`; the
  addresses are in the names and they were inverted. That unit could not link.
- **WM_ANCHOR's is a FALSE POSITIVE.** Its draft emits **two separate weak symbols
  with byte-identical one-instruction bodies** — `finalUpdate__12dBaseActor_cFv`
  at `0x760` and `vf74__12daWmAnchor_cFv` at `0x7B0`, each a lone `blr`
  (`4E 80 00 20`). `verify_anon.py` pairs symbols by CONTENT, greedily, so it
  consumed the wrong one of two indistinguishable candidates. Verified directly in
  `draft.txt`. The positions are correct: the auto-emitted one early, our own
  declaration late, per the documented LIFO rule for weak in-class inlines.

**This closes the "Open hypothesis for the last 3 order violations" thread**
(~line 7788): it is a COMDAT/tooling artifact, not a source defect.

**The discriminator is cheap: check the ADDRESSES, not the tool's verdict.**
Definition order must be ascending by target address. Where the address is in the
symbol name that is a text comparison. Where it is not, read the two symbols'
bodies — **if they are byte-identical, content-based pairing cannot distinguish
them and the warning carries no information.**

## A tool that cannot see its target must not be read as a pass

`check_fn_order.py` recovers the target address from the function's own name, so
it cannot see drafts that use real names. An agent ran it on WM_ANCHOR — which
uses real names — and reported **"check_fn_order.py showed 0 inversions"** as
evidence the order was fine. The tool had checked nothing at all.

I had already fixed exactly this an hour earlier, making it list every unchecked
file by name; the agent was running the older build. The fix caught a real reader
within the hour, which is the argument for it.

**"No findings" and "nothing was examined" must never render identically.** Any
sweep that can silently skip its targets has to name what it skipped, or its
summary line converts absence of evidence into evidence of absence — and someone
downstream will cite it as a clean bill of health, exactly as happened here.

## FIXED: the order check no longer invents violations out of a tie

`verify_anon.py` paired each target with the FIRST unused draft function whose
normalised body matched. Two functions with byte-identical bodies are equally
valid candidates, so the tie was broken arbitrarily — and when it broke the wrong
way the tool reported `FUNCTION ORDER IS WRONG` for a draft whose order was
correct. WM_ANCHOR's two lone-`blr` weak symbols did exactly that.

**The fix collects every candidate and prefers one that keeps the matched
sequence ascending.** That is sound, not a papering-over: **byte-identical
functions emit identical `.text`, so if an ascending assignment exists the object
is consistent with correct definition order and there is nothing to report.** A
genuine inversion admits no ascending assignment and still reports.

Verified in both directions, which is the part that matters:

- **WM_ANCHOR:** warning gone, tally unchanged at 19/22, and `vf74__13dWmObjActor_cFv`
  now pairs with `vf74__12daWmAnchor_cFv` instead of stealing
  `finalUpdate__12dBaseActor_cFv`.
- **Negative control:** took the same draft, exchanged two DISTINCT-bodied
  functions to synthesise a real inversion, and the tool still reports
  `FUNCTION ORDER IS WRONG` and names `calcModel` as defined too late.

**A fix that only removes false positives is untested.** The control that proves
it still catches the real thing is the half that stops this from becoming a
silent hole in the linkability gate — and that gate has already caught two
genuinely unlinkable units today.

This does NOT fix the other half of the pairing trap, documented in the tool's
own header: a draft function paired to a target that is genuinely a different
function of the same shape (the sandpillar deleting-destructor case). Confirming
the target at the reported address is still required.

## The redundant-guard lever has an EXCEPTION: automatic base-subobject destruction

I pointed WM_ANCHOR's destructor at the recorded lever "an explicit redundant
null guard is source-visible in BOTH directions" — its whole 21-line gap is one
extra `beq` the target has. **The lever does not apply, and the agent proved it
rather than assuming either way.** Two shapes tried, both rebuilt, both
**zero-effect** — byte-identical to a plain `{}` destructor:

```cpp
daWmAnchor_c::~daWmAnchor_c() { if (this) {} }
daWmAnchor_c::~daWmAnchor_c() { if (this) { (void) this; } }
```

**The mechanism, which is the part worth keeping.** In the WM_KINOPIO precedent
where the lever DID work, the guarded statement was `delete mpMdlMng;` — a real
operation with an external effect, so MWCC cannot eliminate the guard. Here the
only thing that would belong inside the guard is destruction of the base's own
inlined members plus the base destructor call, and **that is not reachable as an
independent, once-only statement from the derived class's source.** It already
runs automatically as the implicit tail of the derived destructor. Writing it
again explicitly duplicates the whole block of real calls, which is not what the
target shows.

So the rule refines to: **a redundant guard is source-visible only when the
guarded code is something the source can STATE. It is not, when the guarded code
is automatic base-subobject destruction.** This independently reconfirms the
earlier "duplicate-`beq` destructor wall" finding (~line 8949, "a derived class
cannot reach a construct emitted by inlining its base's destructor") — by fresh
experiment rather than by restatement, which is worth strictly more.

WM_ANCHOR closes at **19/22**, WM_ANTLION_MNG unchanged at 18/22.

## WM_KINOPIO's pool: BOTH short and misordered, for two separate reasons

The three-way question — short, misordered, or neither — answered as **both**, and
neither cause was the WM_KILLER "unwritten sibling" precedent (ruled out: all 19
functions have code).

**A TRUNCATED DUMP caused a wrong constant, and that is the transferable lesson.**
Case 0's multiply was written as `(m_19c.x - mPos.x) * 1.0f` — a no-op. A freshly
completed `.rodata` table showed the target multiplies by `-2.0f`, a constant the
draft never referenced. **An earlier partial dump had silently cut off before that
entry.** A dump that ends early does not announce itself; it presents as a
complete table with the answer absent. Re-dump before trusting a table you did
not generate in the current round.

**A `.data` label that is a POINTER, not the thing.** An inherited note claimed
`lbl_2_data_45CBC` was the `"W101"` string. It is not: the string lives at
`lbl_2_data_45CB4`, and `0x45CBC` holds a RELOCATION pointing at it. Verified
directly — `0x45CB4` contains the bytes `W101\0`, and `0x45CBC` reads as zero in
the file with a relocation addend of `0x45CB4`. **That is this project's own
recorded signature: a relocated word reads as ZERO.** Modelled correctly as
`static const char *sW101 = "W101";` — and the **non-const-qualified pointer
matters**: a top-level `const` let the compiler cache the value across a call,
one instruction short of the target's independent reloads.

**Scope promotion, which changes what to do next.** The remaining `.data`
misorder traces to the known `sc_ForceList` double-init issue — and it is now
confirmed to block `.data` order for **everything declared after `sForceList`**,
not just one function in isolation. It was parked as a local residual; it is a
unit-wide blocker.

**A methodological trap worth more than the round's numbers:** `verify_anon.py`'s
diff is strictly positional and unaligned, so **the raw differing-line count is a
poor progress signal while the draft's total instruction count differs from the
target's.** The prologue's frame size alone (`-0x70` target vs `-0x50` draft)
explained most of the apparent flatness after an earlier, verified-correct fix.
A verified fix that does not move the count is not evidence the fix was wrong.

## HEADER ADDED: `daWmMap_c::GetPos(const char *)` — the name overload of `GetPos(int)`

Applied to `include/game/bases/d_a_wm_map.hpp` plus a `syms.txt` entry, **five
binaries re-verified green**. Evidence, all checked before applying:

- **`GetPos__9daWmMap_cFPCc = 0x80100380`** exists in `bin/dtk/wiimj2d_symbols.txt`,
  sitting **immediately after** the already-landed `GetPos__9daWmMap_cFi = 0x80100310`
  (size `0x64`, so the previous function ends at `0x374`).
- **The header already contains the identical overload pattern** one method up:
  `GetNodePos(long, mVec3_c &)` and `GetNodePos(const char *, mVec3_c &)`, both
  landed. An index/name pair is this class's established shape, not a guess.
- Return type taken from the sibling `GetPos(int)`, already documented as
  returned by value via a hidden pointer.

Fifth shared-header change to go in through the shadow-header workflow today.

## WM_HANACHAN 13/32 -> 16/32, and a DOL vtable read that closed a function outright

`unkFn164B10` closed because the agent **dumped the real `m3d::mdl_c` vtable out
of `original/wiimj2d.dol`** (`__vt__Q23m3d5mdl_c` at `0x80329980`, size `0x20`, so
8 slots) and resolved each slot against the DOL symbol map — slot `0x1c` is
`play()`. Reading the actual vtable beats inferring a method from call shape, and
the DOL is right there to read.

Two more closed: `resetTrail` and `resetTargetPositions`.

**Layout correction:** the prior round's `mUnk454`/`mUnk46c` were guessed as single
floats. Right offset, **wrong width — both are `mVec3_c`**, with a new `mUnk460`
between them. Also confirmed `mPos` (not `mTargetPos`) at `+0xac`, from
`create()`'s own already-matching bytes — i.e. from code already known correct
rather than from the function under investigation.

### A CROSS-UNIT residual class: paired-single vectorisation, now on TWO units

`getBasePos`/`getPosVariant2`/`getPosVariant3` have the correct call graph and
correct values (confirmed by reading retail `.data` bytes out of the REL), but
**the target inlines its Vec3 addition as raw paired-single (`psq_l`/`ps_add`/
`psq_st`) while calling the SDK's `PSVECAdd` compiles to a real `bl`** — verified
against a landed caller in `source/dol/cLib/c_m3d.cpp`, whose retail bytes really
do call out.

**This is the same residual class already flagged on WM_KINOPIO.** Two independent
units now, which promotes it from a unit quirk to a project-level gap: the likely
requirement is CodeWarrior's `v2f` / `__vec2x32float__` intrinsic type rather than
any source-level rearrangement. Worth solving once, centrally — it is currently
costing functions on two units and will cost more.

## SOLVED (the construct, at least): the paired-single Vec3 add is `nw4r::math::VEC3Add`

Two units independently stalled on the same residual — WM_KINOPIO and WM_HANACHAN
both found the target inlining a Vec3 addition as `psq_l`/`ps_add`/`psq_st` while
their source compiled to a real `bl`. It was promoted to a project-level gap and
guessed to need CodeWarrior's `v2f`/`__vec2x32float__` intrinsic.

**It needs no intrinsic and no header change. The construct is already in the
tree, fully implemented**, at `include/lib/nw4r/math/math_types.h:315`:

```cpp
inline VEC3* VEC3Add(register VEC3* pOut, register const VEC3* pA,
                     register const VEC3* pB) {
    register f32 work0, work1, work2;
    asm {
        psq_l  work0, VEC3.x(pA),   0, 0     // XY as a pair
        psq_l  work1, VEC3.x(pB),   0, 0
        ps_add work2, work0, work1
        psq_st work2, VEC3.x(pOut), 0, 0
        psq_l  work0, VEC3.z(pA),   1, 0     // then Z alone
        psq_l  work1, VEC3.z(pB),   1, 0
        ps_add work2, work0, work1
        psq_st work2, VEC3.z(pOut), 1, 0
    }
    return pOut;
}
```

**That is the reported shape exactly**, including the two-step structure both
agents described: the XY pair first, then Z with the single-element scale. There
is also `nw4r::math::VEC3::operator+` at line 119 which simply wraps it, so
`VEC3 + VEC3` inlines to paired singles on its own.

### Why both agents missed it, which is the reusable part

They were looking at the two vector types their units already used:

- **`PSVECAdd`** — declared in `include/lib/revolution/MTX/vec.h` as a plain
  extern. It compiles to a `bl` and it is CORRECT that it does: landed code in
  `source/dol/cLib/c_m3d.cpp` calls it out-of-line and matches the retail bytes.
  So "make PSVECAdd inline" would have been exactly the wrong fix, and one agent
  had already proven the call is genuine.
- **`mVec3_c::operator+`** (`include/game/mLib/m_vec.hpp:190`) — three scalar
  float adds, `lfs`/`fadds`/`stfs`. Never going to vectorise.

**Three vector families coexist in this codebase — `mVec3_c` (EGG-derived),
`Vec`/`PSVEC*` (revolution MTX), and `nw4r::math::VEC3` — and only the third has
an inlined paired-single add.** When a target's arithmetic does not match, ask
which FAMILY the original used before concluding the compiler is doing something
you cannot reproduce. The shape of the emitted code identifies the family.

**What is verified and what is not.** I have verified the inline exists and emits
precisely those instructions in that order. I have NOT verified that switching
either unit's call site to it closes those functions — the surrounding register
allocation still has to agree, and both units also need the right operand types
at the call. That measurement belongs to the agents; the construct question is
settled.

## The pool mechanism, MEASURED rather than asserted (WM_KOOPAJR)

The prediction was that authoring the missing contributors would move `create`
and `execute`'s pool displacements toward the target. It was tested and it moved:

```
                         before      after      target
PTMF table               +0x08       +0x50      +0x70
CalcShadow constants     +0x20/+0x24 +0x48/+0x3c +0x88/+0x8c
```

**The gap roughly halved after authoring `changeAnim` plus one state-machine
case's literals.** Both functions are still blocked, but blocked-and-closing is a
different state from blocked-and-stuck, and the difference is only visible
because the displacements were recorded before and after. **Record pool
displacements as NUMBERS across rounds, not as "still short".**

`fn_2_16E3A0` is now `changeAnim(int, float, float, float)` — 57/57 lines, the
only 4 differing being this unit's own symbol names. Closing it required hoisting
`sc_animNames[6]`/`sc_playModes[6]` from `createModel()`-local statics to shared
`static const` class members, **because the target has both functions referencing
the SAME symbols rather than independent per-function copies.** Two functions
sharing a pool entry is itself evidence about where a static was declared.

`fn_2_16D940` is a **16-case state machine** (`jumptable_2_data_45EC4`,
`cmplwi r0,0xf`) — Bowser Jr.'s appear/jump/land/run/disappear cutscene. Cases 0,
14 and 15 authored and verified instruction-by-instruction; case 15 is the only
path returning `true`.

**Cases 1-13 deliberately NOT attempted, and the reason is the right one:** the
float/angle math is intricate enough that guessing risks **adding WRONG pool
entries**, which would move the displacements further from the target rather than
closer. In a unit whose remaining blocker IS the pool, a wrong guess is not
neutral — it is negative. Precise verified partial progress beats a larger
unverified block.

New fields, evidence-based: `+0x344` `mVec3_c mJumpTargetPos` (was
undifferentiated pad), `+0x350` `int mJumpTimer`, `+0x354` `int mCurAnimIdx`
(`changeAnim`'s dirty-check cache). `lookupAction`'s table is no longer a
placeholder — recovered from `.rodata` as `{0, 5, 7, 11}` and **independently
verified against the retail bytes at `0x8C38`: `00000000 00000005 00000007
0000000b`.**

## `sc_ForceList` is NOT a shared-header defect. Ten landed units prove it.

WM_KINOPIO is blocked on a `sc_ForceList` "double-init" that governs `.data`
order for everything declared after it, and WM_KILLERBULLET independently
identified its `fn_2_169FA0` as the compiler-generated `.ctors` static-init for
the same `dWmLib::sc_ForceList`. Two units converging on one shared static looks
like a header defect. **It is not, and that is checkable in one command.**

`include/game/bases/d_wm_lib.hpp:96` declares

```cpp
static ForceInCourseList_t sc_ForceList[] = {
    {WORLD_7, "F7C0", WORLD_7, dCsvData_c::c_CASTLE_ID, 4, "W7C0",
     mVec3_c(2160.0f, -30.0f, -478.0f)}
};
```

`static` at this scope gives internal linkage, so **every TU that includes the
header gets its own copy**, and the `mVec3_c(...)` initialiser is a constructor
call, so each copy is DYNAMICALLY initialised and costs a `.ctors` entry. That is
the already-recorded rule "including `d_wm_lib.hpp` costs a `.ctors` entry".

**And it is correct as written: `grep -l d_wm_lib.hpp source/d_basesNP/bases/*.cpp`
returns TEN-PLUS LANDED units** — antlion, cannon, cloud, dokan, dokan_route,
ghost, grid, kinoko_1up, kinoko_base, kinoko_red and more — every one of them
byte-perfect against retail with this exact declaration. A shared header cannot
be simultaneously wrong and produce ten byte-perfect units.

**So the fault is TU-side, and the recorded rule already names the likely cause:
"some units' targets do NOT include it."** If a draft includes `d_wm_lib.hpp`
where the original TU did not, the draft gains an extra `sc_ForceList` copy and
an extra `.ctors` entry — which presents exactly as a "double init" and shifts
`.data` order for everything after it.

**The general rule worth carrying: before treating a shared header as defective,
count the landed units that include it.** If any land byte-perfect, the header is
right and the defect is in your own TU — most often an include the original did
not have. That check is one `grep -l` and it inverts the whole search.

## `.ctors` resolves "which function is `__sinit`" EXACTLY — and the count is a test

`.ctors` is a table of one-word entries, each a relocation into `.text` pointing
at a static-initialiser function. Resolving those against the profile ranges
answers two questions that are otherwise guessed at: **which function IS a unit's
`__sinit`, and how many static initialisers the unit has.**

Three agents independently guessed a function was "probably this unit's `__sinit`"
on the same day. `ctors_map.py` confirmed **all three in one run**:

```
.ctors   __sinit .text   unit
0x3e8    0x165b20        WM_HANACHAN        <- agent: "looks like a static object's __sinit"
0x3f4    0x169fa0        WM_KILLERBULLET    <- agent: ".ctors static-init for sc_ForceList"
0x40c    0x16d1e0        WM_KINOPIO
0x410    0x16e490        WM_KOOPAJR         <- agent: "the file's __sinit"
```

**And the COUNT is the diagnostic.** `WM_KINOPIO's target has exactly ONE `.ctors`
entry.** A draft emitting two has an extra static initialiser — which is precisely
what a spurious `#include <game/bases/d_wm_lib.hpp>` produces, since that header's
`sc_ForceList` is a `static` array with an `mVec3_c(...)` initialiser and every
including TU pays a dynamically-initialised copy.

So the whole "sc_ForceList double-init" question reduces to **comparing two
numbers**, which replaces a round of source-order experiments. A unit with NO
matching entry is equally informative: its TU includes no header that costs one.

**318 `.ctors` entries across the module, one per unit that has static state.**
The table is small, exact, and was sitting in the binary the whole time — three
separate agents spent effort inferring what one relocation walk states outright.

## MY ERROR: a tally gap is NOT a measure of unwritten work

I assigned WM_DANCE_PAKKUN (9/16) and WM_ITEM (8/12) as authoring targets on the
reasoning that they had "~7 and ~4 unwritten functions". **Both had ZERO.** Every
non-matching function already had a full draft — dance_pakkun with ten rounds of
documented iteration behind it, item with four. The agent measured this properly,
spent its budget on the closest candidates in each, moved nothing, reverted
cleanly, and reported it.

I read `N/M` in the session summary as "M-N unwritten". It means "M-N not
MATCHING", and on a parked unit those are almost always walls with history. The
distinction is the entire basis for choosing a unit, and I had it backwards.

**Before assigning a unit as an authoring target, MEASURE how many of its
functions have no source at all** — either ask the agent to report that split
before starting, or read its `MAPPING.md`. "Parked at N/M" says nothing about
whether the remaining work is writing or grinding.

Two things that did come out of the round:

- **A named family pattern, now on two independent units.** WM_DANCE_PAKKUN's
  constructor residual is byte-for-byte the same shape as WM_ANCHOR's: base-ctor
  call, then the vtable installed into `r4` with `+0x184` stored FIRST in the
  target, versus `r3` and the reversed order in the draft. The same lever (move
  the init-list member into the body) regressed both — ANCHOR 4 -> 17,
  DANCE_PAKKUN 4 -> 38. **Two independent confirmations make this a family
  pattern, not a coincidence.** Do not spend the lever on this ctor skeleton again.
- dance_pakkun's destructor carries the same duplicate-`beq` shape confirmed
  unfixable-from-derived-source earlier this session. Noted, correctly not
  re-tested.

## THERE ARE UNLANDED UNITS OF 0x30 BYTES, and we have been grinding walls

While five agents worked units of 0x1000-0x3000 bytes, a scan of every profile
range against the landed slices turned up this:

```
0x30   0xa8470    JR_FLOOR_FIRE_MGR              0x50   0x7d400   AC_FLAGON
0x30   0xc5c90    LEMMY_FOOTHOLD                 0x50   0x7d450   AC_4SWICHAND
0x30   0xd1450    AC_LIFT_REMOCON_SEESAW         0x50   0x7d4a0   AC_4SWICHOR
0x30   0xf5130    MIDDLE_BG_FOR_CASTLE_LUDWIG    0x50   0x7d4f0   AC_RANDSWICH
0x30   0xf8980    MINI_GAME_GUN_BATTERY_MGR      0x60   0x1c5cb0  WM_TEST
0x30   0xfc8d0    MINI_GAME_WIRE_MESH_MGR        0xa0   0x152010  AC_WATER_MOVE
0x30   0x1204e0   PEACH_CASTLE_SEQUENCE_MGR      0xb0   0x77af0   DUMMY_DOOR_CHILD
```

**0x30 bytes is roughly twelve instructions.** The project has landed nothing in
this session while agents fought register-allocation residuals on units two
orders of magnitude larger.

**The caveat that makes this scoping work, not a free lunch:** a slice is per
TRANSLATION UNIT, not per profile. Six `AC_*` switch profiles sitting at
consecutive `0x50` offsets are very likely ONE `.cpp` file, and mis-scoping a
unit from a profile boundary has cost this project expensively twice already
(WM_ANTLION scoped with both ends wrong; WM_ANTLION_MNG scoped at 79 functions
when it is 22). Establish the TU boundary before authoring.

Some come with a head start already in hand: `BSS_SINGLETONS.md` gives
`MINI_GAME_GUN_BATTERY_MGR_OBJ` as `sizeof 0xF4 : dActor_c`,
`PEACH_CASTLE_SEQUENCE_MGR_OBJ` as `sizeof 0xB8 : dActor_c`, and
`MINI_GAME_WIRE_MESH_MGR_OBJ` as `sizeof 0x708 : dActor_c` — size, base class and
singleton pointer already resolved before anyone opens the disassembly.

## `sc_ForceList` double-init SOLVED: it was our own duplicate declaration

`fn_2_16D1E0` went **32 differing -> 3**. The cause was in the draft all along: a
hand-authored `static dWmLib::ForceInCourseList_t sForceList = {...}` carrying
**the exact same literal values as the header's own `dWmLib::sc_ForceList`**, and
referenced nowhere in the file. A pure unused duplicate, paying for a second
static initialisation.

Reading the target disassembly fresh — rather than trusting the inherited framing
— showed the target constructs **exactly one** `ForceInCourseList_t`, at the same
address with the same values. **There was never a second object to reconcile.**
The whole "double-init" framing was an artefact of our own draft.

**The chain that solved it is worth noting, because no single step would have.**
WM_KILLERBULLET identified its `fn_2_169FA0` as the `.ctors` static-init for the
same shared static. That made it look like a header defect, which `grep -l
d_wm_lib.hpp source/d_basesNP/bases/*.cpp` refuted immediately — ten-plus LANDED
units include it and are byte-perfect. That put the fault TU-side and produced a
two-way branch: an extra `.ctors` entry (spurious include) versus one entry
initialising twice internally. `ctors_map.py` then made the branch decidable by
counting, and the answer was neither — it was a third case, a local re-declaration.

**Verified both ways, with numbers:** the target has exactly one `.ctors` entry
(`0x40c -> 0x16d1e0`); the draft's `.ctors` is now exactly `0x4` bytes, one entry,
for the same function. Confirmed independently here.

The remaining 3-line diff is a pure pool-POSITION shift — the same
`2160.0f/-30.0f/-478.0f` constants sitting 4 bytes later than the target — traced
to case 0's still-open divisor.

**Case 0's divisor: three spellings now exhausted.** `15.0f`, `(int)15` and bare
`15` all constant-fold to an immediate, where the target genuinely converts at
runtime (`li r3,0xf; lis r0,0x4330; xoris; stw/stw; lfd; fsubs`). Note what that
implies: **the compiler DID know the value is 15** — it is an immediate in `r3` —
**and still emitted the conversion**, which is what happens converting an `int`
that has been constant-propagated rather than a literal. So the source divides by
something the compiler treats as an `int` OBJECT, not a literal. It now gates two
functions, which promotes it.

Also re-verified this round: all five `setAnm(...)` call sites in `stepCutscene70`
against the complete `.rodata` table, after the truncated-dump incident. **No
further bugs found** — the one wrong multiplier was the only casualty. A clean
re-check after a known-bad input is worth recording precisely because it came back
empty.

## Scoping a fresh unit: `scout_unit.py`, and the singleton exception to the ownership check

`bin/dtk/dtk_splits_*.txt` is generated from the slices already LANDED, so it
lists only solved units and is no help scoping a new one. Bounds have to be
derived, and deriving them from a profile boundary has mis-scoped units twice
(WM_ANTLION with both ends wrong; WM_ANTLION_MNG at ~79 functions when it is 22).

`wip/wm_units/scout_unit.py` walks the relocation stream for a `.text` range and
reports every section it reaches plus its `.ctors` ownership. Worked example, the
`PEACH_CASTLE_SEQUENCE_MGR` pair:

```
0x1204E0-0x120510 (0x30)   MGR      -> reaches ONE external address: operator new
0x120510-0x120F00 (0x9F0)  MGR_OBJ  -> own .rodata/.data/.bss, .ctors 0x2E4 -> __sinit 0x120D00
```

**One `.ctors` entry across both profiles is evidence of ONE translation unit** —
a manager whose whole body is "allocate the object", plus the object itself.

### The refinement, which the ownership check needs

Running the standard check — does any code OUTSIDE the claim read pools the claim
owns? — returned two hits, and **both are false alarms for opposite reasons**:

- **`.data 0x418`** is read from all over the module (`0x7d6`, `0x1cc6`, `0x5346`
  …). That is a SHARED object, not unit-owned. Exclude anything with module-wide
  readers before treating it as evidence.
- **`.bss 0xD8F8` is this unit's SINGLETON INSTANCE POINTER.** Of course other
  code reads it — that is what a singleton is for. **A singleton pointer being
  read from outside the claim is expected and proves nothing about the bounds.**

**So the ownership check must exclude singleton pointers and module-wide shared
objects before its result means anything.** Run `bss_classify.py` on a `.bss`
label before counting its external readers as evidence that a claim is short. The
check that caught WM_ANTLION's short claim is still right; it just needs those two
exclusions or it produces a false positive on every unit that owns a singleton.

With both excluded, **no unit-owned pool is read from outside** — the claim
`0x1204E0-0x120F00` is NOT short, and the unit is scoped and ready to author with
`sizeof 0xB8`, base `dActor_c`, and its singleton already resolved.

## Case 0's divisor: CONFIRMED — an `int` OBJECT, not a literal

The reading was right and the fix survived the session-limit kill inside the
preservation commit:

```cpp
int n = 15;
float speed = dist / (float) n;
m_194 = speed / (float) n;
```

`15.0f`, `(int)15` and bare `15` all constant-fold to an immediate. **A named
`int` local does not** — the value propagates into `r3` as an immediate while the
int-to-float conversion survives, which is exactly what the target shows
(`li r3,0xf; lis r0,0x4330; xoris; stw/stw; lfd; fsubs`). The `0x4330` bias is
the SIGNED idiom, which is what pinned it to `int` rather than `u32`.

**`stepCutscene70` went 485 -> 383 differing.** But `fn_2_16D1E0` did NOT close:
its remaining 3-line gap is a pool-POSITION residual (the target dedupes a `1.0f`
slot this draft never reaches), not the divisor. **My prediction that solving the
divisor would close it was wrong** — the two shared a symptom, not a cause.

Round 15 added one clean negative: hoisting `mpMdlMng->mpMdl->m_152` into an
explicit local before the footstep-sound call produced **byte-for-byte identical
codegen**, reverted. And a tangent closed rather than left open: the
`.rodata 0x8b10-0x8ba0` table was re-derived from `original/d_basesNP.rel` against
a verified anchor and case 6's constants are correct. Its unreferenced
`0.2f/0.25f/0.75f` slots belong to `processCutsceneCommand` and `checkAnmLoop`,
both already walled — **not a gap in `stepCutscene70`.**

### WM_KINOPIO is parked at 14/19, and the numbers say why

```
resetPosition 3   fn_2_16D1E0 3   checkAnmLoop 34
processCutsceneCommand 136        stepCutscene70 383
```

**383 differing lines is not a residual, it is a distance.** Fifteen rounds have
gone into this unit. Both gates are clean (one `.ctors` entry matching target;
definition order ascending across all 18 functions), so nothing structural is
wrong — it is simply far from done, and the project's own planning fact applies:
**a unit that does not close quickly tends not to close at all.** Capacity moves
to a small unscoped unit instead. This is a park, not an abandonment; MAPPING.md
carries fifteen rounds of measured negatives for whoever returns to it.

## THIRD unlinkable unit today — and this one NO TOOL could see

WM_KOOPAJR's definition order did not match the target's address order: **eight
functions defined too late.** A silent link-breaker, invisible to the per-function
tally, and **invisible to `check_fn_order.py` as well** — that tool recovers the
target address from the function's own name, and this draft uses real names.

**The method that found it is the general answer for real-name drafts:**
reproduce `verify_anon.py`'s own ascending-pairing against the target dumps. That
works regardless of naming, because it pairs on instruction CONTENT and then asks
whether the matched draft indices ascend. Independently re-verified here: after
the fix, no order violation, 15/21 byte-identical.

Running tally for the day: **WM_KILLER, WM_KILLERBULLET and now WM_KOOPAJR were
all unlinkable while showing respectable match counts.** Three units, three
different discovery routes. Check order EARLY on every unit; it is free and it is
not what the tally measures.

## An in-class inline compiles WEAK. If the target symbol is GLOBAL, go out-of-line.

`procNone` was defined in-class inline, which MWCC emits as a **weak** symbol. The
target's `fn_2_16D7E0` is **global**. Moving the definition out-of-line fixed it
and the function now pairs MATCH in its correct slot.

**Symbol BINDING is checkable evidence about where a definition was written**, and
it is independent of the instruction diff — a function can be byte-identical and
still wrong if its binding differs, because the linker treats the two differently.
This pairs with the already-recorded emission rules: strong out-of-line functions
emit in definition order; weak in-class inlines emit in REVERSE declaration order,
deferred to the end (LIFO). **Read the binding, not just the bytes.**

## Fix ORDER before diagnosing a pool gap — a wrong order MASKS the real picture

Before the order fix, WM_KOOPAJR's `create`/`execute` pool displacements were
scattered and uninterpretable: `8`, `0x40`, `0x50` on three affected symbols.
**After the order fix alone, they collapsed to a single uniform `0x20` gap on all
three.** Same code, same pool contents — the disagreement was the ordering.

So pool diagnosis performed on a mis-ordered file is measuring noise. **Order is a
precondition for the pool numbers to mean anything**, which makes it a cheap first
step rather than a landing-time checklist item.

### A negative established by exhaustive search AND by experiment

The missing 9-word block (retail `0x8be8`-`0x8c08`) has **no consumer anywhere in
this unit** — every target instruction referencing the rodata pool base register
was checked across all 16 cases of `runMain`, every other function, and `__sinit`.
Then the prediction was TESTED rather than assumed: case 4 was authored with every
constant read directly off the disassembly (none guessed), and **the create/execute
gap did not move at all**, exactly as predicted.

Conclusion, correctly labelled by the agent as INFERENCE not fact: that specific
gap is not closable from this unit's own code, and is likely a sibling class's
pooled constant resolvable only at real multi-object link time. **If true,
WM_KOOPAJR cannot reach N/N alone** — which is a planning fact, not a defect.

A *different* unclaimed pool range (`0x90`-`0xd8`) DOES have confirmed consumers
in cases 5-13, so that part should keep closing as those are authored.

## `order_sweep.py`: a project-wide gate, and it immediately found two MORE

Three units were found unlinkable one at a time today, each by a different route.
That is a systematic failure, so it now has a systematic check.
`wip/wm_units/order_sweep.py` runs the order gate across every unit whose
`build.py` records its range and target objects.

It works for ANY naming style, because it drives `verify_anon.py`, which pairs
target to draft on instruction CONTENT and then asks whether the matched draft
indices ascend. **`check_fn_order.py` covers only the 3 drafts of 56 whose symbol
names carry the target address**; this covers the rest. It is read-only — no
compile, no disassembly — so it is safe to run while agents are working.

**First run found two more genuinely unlinkable parked units:**

```
ORDER WRONG  agent_antlion_mng   18/22   17 defined too late
ORDER WRONG  agent_hanachan      17/32   16 defined too late
```

**WM_ANTLION_MNG matters most.** It was surveyed this session and reported as
"zero unwritten functions, all four residuals are register-allocation walls with
substantial documented iteration." All true — **and it could not have linked
regardless**, with seventeen functions defined too late. A unit can be
simultaneously wall-bound and unlinkable, and the tally shows neither.

**That makes FIVE units found unlinkable in one day** — WM_KILLER,
WM_KILLERBULLET, WM_KOOPAJR, WM_ANTLION_MNG, WM_HANACHAN.

### The sweep's own first run was wrong, in an instructive way

It reported **`WM_MANTA` as "16/16 and ORDER WRONG"** — a fully-matching unit that
cannot link. WM_MANTA LANDED hours earlier. The leftover `draft.txt` in its old
scratch directory is a **stale artefact**; the real source is in `source/` and is
byte-perfect by definition, re-verified against retail on every build.

**A sweep over scratch directories must exclude units that have already landed**,
or it will confidently indict shipped, verified code. The tool now skips them and
says which it skipped — alongside naming every unit it could not check, because
"not checked" must never render as "checked and fine".

Five units cannot be checked at all: their `build.py` records no range or object
list. Fixing those `build.py` files is cheap and puts the whole tree under the
gate.

## WM_KILLERBULLET: zero fake stubs, and a PREDICTION DISPROVED by its own author

Tally held at 20/37, and that number understates the round: **every fake stub in
the unit is gone**, a first for this unit. The gains turned fake or broken content
into genuinely-close real content rather than crossing the match threshold.

- `unk_168990` (the last true stub) authored from scratch: `3/71` undefined ->
  **29/71 size-exact**, 10 of those naming-only.
- `unk_168D50`: `60/67` -> **33/67 size-exact** via statement-order restructuring
  into separate `pos`/`angle`/`scale`/`offset` locals. Three other variants
  measured worse (65, 37, 59) and rejected.
- `unk_1698E0` (167 lines, the largest): **`short` locals were forcing spurious
  `extsh` instructions the target does not have.** Retyping to `int` dropped
  `163 -> 112/166`. A switch->if-chain conversion — the lever that worked on
  `processCutsceneCommand` — made it WORSE here (161) and was reverted.
  **The same lever closing one function and breaking its sibling is already on
  record; this is another instance.**
- `unk_1691A0`: one more variant regressed to 15, reverted. Two attempts now on
  record.

**A local's declared WIDTH is a source-visible lever.** `short` where the target
used `int` costs a spurious `extsh` per use — visible, mechanical, and worth
checking before reaching for scheduling explanations.

### The disproved prediction

Last round this agent predicted `fn_2_169FA0`'s `.ctors`/`__sinit` residual would
**close on its own once every stub was authored**, by analogy with the recorded
rule that a unit's pool cannot be right while contributing functions are
unwritten. It authored all four, re-diffed, and found **exactly 33/97, no change
whatsoever.** It recorded the correction against its own prediction.

**That bounds the pool doctrine usefully.** "Pool offsets short because siblings
are unwritten" describes `.data`/`.rodata` POOL POSITION. It does not extend to
compiler-generated `__sinit` content, which is driven by the static declarations
themselves, not by how many ordinary functions exist. `unk_168C80` also held at
7/49 — correctly predicted, since nothing authored this round was a `.data`
string-pool contributor.

**A prediction volunteered and then falsified by its own author is worth more
than a cautious one never tested.** Both of this agent's predictions were
explicit and both got measured; one held, one did not, and the doctrine is
sharper for it.

## LANDED: `d_a_dummy_door.cpp` — the session's first, and the small-unit bet paying off

**`DUMMY_DOOR_CHILD` + `DUMMY_DOOR_PARENT`, `.text 0x77AF0-0x77C50` (0x160 bytes),
`.data 0x1AA08-0x1ABC8`. 4/4 byte-identical. Five binaries verified green.**
`d_basesNP` 2.354% -> 2.373%, total 11.471% -> 11.477%.

**352 bytes of `.text`.** It landed in a single round on a unit nobody had opened,
while five agents spent the session on units of 0x1000-0x3000 bytes and landed
nothing. The scouted small-unit queue was the right call and this is the evidence.

Both classes are trivial: `daDummyDoorChild_c` / `daDummyDoorParent_c :
public dActor_c`, no members, overriding exactly one vtable slot each.

### Three corrections the agent made to MY briefing, all checked before concluding

- **No `createChild` call exists in this unit.** I had said a PARENT/CHILD profile
  pair usually means one creates the other. The agent read both vtables: slot 2
  (`create()`) is `dActor_c`'s own unmodified base in both. **The naming does not
  cash out as a call.**
- **The destructor occupies ONE vtable slot here, not two.** My standing
  two-slot rule (scalar + vector deleting) is a general hint; **this ABI uses a
  single flag-argument destructor**, read directly off the vtable data.
- Both destructors were declared and defined **out-of-line**, because an in-class
  default would compile weak and defer to end-of-TU while the target's four
  functions sit in plain non-deferred order.

### A defect the tally could never have caught: VTABLE ORDER in `.data`

The first draft emitted the two compiler-synthesised vtables in `.data` in the
**wrong relative order** — PARENT before CHILD, backwards from the target. Same
class of bug as a `.bss` ordering miss, and with **no `.ctors` entry in this unit
there was no structural check that would have caught it.**

**Root cause, established by experiment: vtable emission order tracks CLASS
DECLARATION order, not out-of-line definition order.** Swapping the two
`class {...};` declarations fixed it with zero effect on `.text` order. That is a
new lever and it is independent of the `.text` ordering rules already on record.

### And a tool limitation found the same way

`check_vtable.py` grabs the FIRST `__vt__` object in `draft.txt` with a bare
`re.search`, so **a two-vtable file silently checks the wrong one against a
mismatched target label.** The agent isolated each vtable and confirmed both
clean independently. Any unit defining more than one class will hit this.

## Read vtables from the `.data` SPLIT OBJECT — it prints real symbol names

Every vtable question on this project has been answered by walking `.text`
relocations and converting offsets to slots by hand. There is a better way, found
while authoring MINI_GAME_GUN_BATTERY:

**Disassemble the `.data` split object that CONTAINS the vtable** (here
`bin/dtkspl/d_basesNP/obj/auto_04_000132B0_data.o`). It prints **every slot with
its real mangled symbol name**, not a bare address. Overrides are then obvious by
inspection: MGR_OBJ overrides exactly `create`, `execute`, `preExecute` and its
destructor out of `fBase_c`/`dBase_c`'s 18 primary slots — identified from
CONTENT, not derived from arithmetic.

**Reach for the `.data` object disasm on any vtable question**, in preference to
the `.text`-relocation walk. The relocation technique is still what you need when
no split object covers the vtable, but it should not be the first tool.

### Pooled STRING LITERALS name the class and its states outright

The same dump contained
`"daMiniGameGunBatteryMgrObj_c::StateID_ShowRule"`, `"...::StateID_Play"` and
`"...::StateID_ShowResult"` — **byte-for-byte confirmation of a class name this
project had only guessed**, plus the three real state names and their
`initialize`/`execute`/`finalize` triples.

`sStateID_c` objects carry their name as a string, so **any unit using the state
framework has its class name and every state name sitting in `.data` in plain
ASCII.** Look there before inventing names.

## A prediction that HELD: name the overrides before touching the constructor

MINI_GAME_GUN_BATTERY's constructor was parked at 58 differing with a
vtable-identity mismatch at `+0x60` and a missing `+0xa0` store. The instruction
was to leave it alone and identify the real virtual overrides first, on the theory
that **the vtable stores would reshape themselves** once the overrides existed.

They did. After authoring the four overrides the constructor went **58 differing
-> 1**, and both the `+0x60` and `+0xa0` problems resolved with no direct work.
The single remaining residual is a partial-loop-unroll on a 3-element array
construction, three variants measured.

Recorded because it is the counterpart to a rule already here: **a stuck
constructor is often a SYMPTOM of undeclared virtuals, not a defect in its own
right.** Attacking it directly is working on the wrong function.

Two more from the same round: `create()` and the destructor matched exact 0-diff,
and `preExecute()` closed on three levers — branch polarity in a ternary, an `int`
(not `bool`) return type on an un-landed callee, and recognising the target's
**`cntlzw`/`srwi` pair as a logical NOT rather than a boolify**. That idiom is
worth knowing on sight.

Also corrected: I framed the unit's two largest functions as candidates for
`execute()`. **`execute()` is 12 lines and forwards to `mStateMgr.executeState()`;
the large functions are two of the STATE bodies.** In a state-framework unit the
dispatcher is trivial and the size lives in the states.

## Tool fix: the trailing-`blr` forgiveness only fired for `bctr`, not for `b`

`verify_anon.py` forgives a draft that is the target plus one dead `blr`, because
dtk splits an unreachable `blr` after a tail call into its own labelled
pseudo-function. **The gate was `bctr` only.** A direct tail call ends in a plain
`b <label>`, and those were not forgiven.

That cost a round on `d_a_peach_castle_sequence`: the target's `fn_2_120970`
(8 instructions, ending `b fn_2_1208C0`) plus the lone `blr` dtk lists as
`fn_2_120990` reassemble byte-for-byte into the draft's single 9-instruction
function. **A complete unit reported 43/44 against a defect that does not exist**
— the second time this exact artefact has burned a round, after sandpillar.

Widened to any unconditional tail transfer: `bctr`, or `b <label>`. Deliberately
NOT widened to conditional branches or to `blr`-ended targets, and deliberately
kept at the COMPARISON rather than restitching the target's function list — the
file already records that restitching cascades (a correct 64/66 became 42/52).

**Measured tree-wide before and after, which is the part that makes it safe to
keep:** across every unit the sweep covers, exactly one number moved —
`agent_peach_castle_seq` from `43/44` to `44/44`. **No other unit's tally changed
at all.** A forgiveness rule that is too loose shows up as tallies rising
everywhere; this one moved precisely where the artefact was.

## LANDED: `d_a_peach_castle_sequence.cpp` — and the ORDER GATE WAS A FALSE POSITIVE

**`.text 0x1204E0-0x120F00`, `.ctors 0x2E4-0x2E8`, `.data 0x39320-0x395F8`,
`.bss 0xD8B8-0xD900`. 44/44. Five binaries verified green.**
`d_basesNP` 2.373% -> 2.512%, total 11.477% -> 11.517%. Two landings this session.

**The unit had been reported as failing the order gate for multiple rounds, with
seven separate experiments run against it. The violation did not exist.** It
links, and all five binaries match retail byte-for-byte.

### Why the gate lied, and how to tell next time

The draft contains **four groups of BYTE-IDENTICAL functions — twenty functions
in total**:

```
7 functions with the same 16-instruction body  (incl. two of the flagged ones:
                                                sFStateFct_c and sFState_c dtors)
6 functions with the same  2-instruction body
4 functions with the same  1-instruction body
3 functions with the same 22-instruction body
```

`verify_anon` pairs target to draft on instruction CONTENT. With seven
indistinguishable candidates for one target, the pairing is ambiguous, and the
"matched indices must ascend" test then reports an order that is an artefact of
tie-breaking rather than of the source.

**And the violation is semantically void even if the pairing were arbitrary:
swapping two byte-identical functions emits identical `.text`.** An ordering
complaint that exists only among interchangeable functions cannot affect
linking, by construction.

### The rule that follows

**The order gate is a PROXY. `progress.py --verify-bin` is the AUTHORITY.**

When a unit is at N/N on content and the only remaining objection is an order
violation, **check whether the flagged functions have byte-identical siblings —
and if they do, try landing it.** The link settles in one command what cost this
unit seven experiments and a round of file-shape guessing, and what I compounded
by sending it after landed precedent for a problem it did not have.

This does NOT weaken the gate. Three units were genuinely unlinkable today and it
caught them, and WM_ANCHOR's earlier false positive was a two-way tie that a
fix already addresses. But **a gate whose false positives look exactly like its
true positives must be checked against ground truth before it drives a round of
work** — and here the ground truth is cheap.

### The agent's own negatives still stand and are worth keeping

All proven, all recorded in its MAPPING.md: `STATE_DEFINE`'s position is
conclusively not a lever (four positions, byte-identical output); the declaration
order of `mStateMgr` versus `STATE_FUNC_DECLARE` does not matter (the landed files
disagree with each other, which is itself the evidence); and forcing the
destructor out-of-line in `Pausewindow_c`'s convention regressed the flag count
7 -> 21, **proving the real source uses inline `{}` destructors** — established by
elimination, not assumption. Those remain true and useful even though the gate
they were aimed at was misfiring.

## HEADER WIDENED: `PauseManager_c` gains `m_1d` at `0x1d` — five binaries green

Applied to `include/game/bases/d_pause_manager.hpp`, verified 5/5. Evidence
checked before applying, and it is the SAME evidence class that pinned the only
other known field in this class:

```
lis  r3, m_instance__14PauseManager_c
li   r0, 1
lwz  r3, 0(r3)
stb  r0, 29(r3)          <- 29 = 0x1d
```

in `daMiniGameGunBatteryMgrObj_c::finalizeState_ShowRule` (`fn_2_F8F20`). The
header already pins `mFlags` at `0x18` by exactly this reasoning — a `lbz`/`ori`/
`stb` through the instance pointer — so this is the established standard for this
class, not a new kind of claim.

**Why widening was safe, and why that mattered:** the header's OWN comment states
the total size is unknown *because the class is heap-allocated and nothing embeds
it by value*. Confirmed independently by grep. **A field added past the last known
offset only affects pointer-based field access; it cannot change any `sizeof`**,
so no already-landed TU can be disturbed. Had anything embedded it by value this
would have been a much riskier change and would have needed a different argument.

Sixth shared-header change to go in through the shadow-header workflow today.

## MINI_GAME_GUN_BATTERY 9/49 -> 14/49, and "confirm cheaply" done properly

I asked the agent to *cheaply confirm* its belief that ~13 functions were
compiler-generated template boilerplate. **It read the content of all sixteen
instead**, and confirmed every one: `sStateMgr_c`/`sFStateFct_c`/`sFStateID_c`
destructors and thunks, `sFStateFct_c<T>::build()` and
`sFStateID_c<T>::isSameName()` matching their landed headers verbatim, and
`__ptmf_scall` adjustor thunks for the pointer-to-member triples. **None needs
hand-written code.** A labelled guess turned into a closed question for the next
agent, which is worth more than the round it cost.

Two further results from doing that reading:

- **A function the inventory had missed entirely.** `daMiniGameGunBatteryMgr_c`
  needs its own destructor (`F9520`, calling `__dt__8dActor_cFv`). Declared and
  defined empty — exact match immediately. **An inventory built from a profile
  range can omit a function; reading the PMF triples and thunks found it.**
- **The parked constructor's residual now has a named cause.** The PMF triples
  revealed `m_70`/`m_74`/`m_78` are conceptually gun slot 0 of a real FOUR-gun
  structure — so the array-unroll residual is likely "declare `mGunSlot[4]`"
  rather than the current "three separate fields plus `mGunSlot[3]`". Left as a
  documented next lever rather than re-attempted, per instruction.

Also confirmed: following `d_pausewindow.cpp`'s structure verbatim
(`STATE_FUNC_DECLARE` in-class, `STATE_DEFINE` right after `BASE_PROFILE`,
`mStateMgr(*this, StateID_ShowRule)` instead of a `sStateID::null` placeholder)
needed **no explicit-instantiation trick** — consistent with none of the ten
landed TUs needing one.

## An array's LENGTH is a source-visible lever — `mGunSlot[4]` closed a constructor

MINI_GAME_GUN_BATTERY's constructor sat at 58 differing across two rounds with a
partial-loop-unroll residual that no unroll variant touched. The unit's own
pointer-to-member triples revealed that three separate fields (`m_70`/`m_74`/
`m_78`) were really **slot 0 of a four-element array**.

Declaring `mGunSlot[4]` instead of "three fields plus `mGunSlot[3]`" took it from
**58 differing to a fully matched 59/59 exact line count**, every remaining diff
naming-only.

**The mechanism is worth more than the fix.** The target has a genuine
3-iteration loop starting at element 0. Against a 3-element array that shape is
inexplicable and invites unroll experiments. Against a 4-element array it is
obvious: element 0 folds into the same array-construction codegen path as 1-3
instead of being modelled as separate fields. **A loop whose trip count does not
match your array is evidence about the ARRAY, not about unrolling.**

### Write `arr[i].field`, not hand-rolled pointer arithmetic

The three slot helpers (`addToSlot`, `setM_f0`, `markSlotUsed`) matched exactly
when written as ordinary `mGunSlot[gunIndex].field` access — **letting the
compiler generate the multiply-and-index addressing itself** rather than
hand-writing the pointer arithmetic the disassembly appears to show. The
disassembly shows the compiler's OUTPUT; reproducing that output literally in
source is usually the wrong move.

### The landed-precedent rule paid off again, on the first ask

Told to grep `source/` before proposing a `dBg_c` header change, the agent found
`source/d_basesNP/bases/d_a_wm_note.cpp` reaching past `dWCamera_c`'s documented
layout with a local raw byte-pointer cast confined to its own `.cpp`. It applied
the identical technique to `dBg_c::m_bg_p` — **two more functions matched, no
header touched.** Same precedent that earlier retired a "camera layout defect"
that was never a defect.

**MINI_GAME_GUN_BATTERY: 9/49 -> 14/49 -> 21/49** (12 exact 0-diff, 9
naming-only), both gates green. The agent flagged its own honest caveat: ~16
further functions are template boilerplate whose content was read and matched
against known template source but **not individually diffed**, so they are NOT
counted in the 21. A tally that excludes what has not been individually verified
is the only kind that survives comparison across rounds.

## WM_KOOPAJR PARKED as blocked-on-LINK, on three independent measurements

The bound was: author the cases whose pool constants are confirmed present, and
if the `create`/`execute` displacement still has not moved, park it. It did not
move, and the evidence is now much stronger than "it did not move once."

**Three separate measurement points, one answer:**
```
                     after order fix   after case 4   after 7 more cases   target
PTMF table               +0x50            +0x50            +0x50           +0x70
CalcShadow constants   +0x6c/+0x68      +0x6c/+0x68      +0x6c/+0x68     +0x8c/+0x88
```

Eight of sixteen `runMain` cases were authored across two rounds with **every
constant read directly off the disassembly, none guessed**, and the `0x90`-`0xd8`
pool range was substantially closed — new entries matching retail exactly
(`9.0f`/`28.0f` at `0xb0`/`0xb4`, `34.0f`/`46.0f` at `0xc0`/`0xc4`, `-1.0f` at
`0xbc`, `2.0f` at `0xc8`). **Substantial correct new pool content, and the
displacement did not shift by a single byte.**

Combined with the earlier exhaustive search — every target instruction touching
the rodata pool base register, across all 16 cases, every other function, and
`__sinit`, finding **no consumer anywhere in this unit** for the missing 9-word
block `0x8be8`-`0x8c08` — the conclusion is that the gap belongs to a sibling
class and resolves only at real multi-object link time.

**WM_KOOPAJR is therefore blocked-on-LINK, not blocked-on-effort**, and that is a
planning fact rather than a defect. 15/19 with both gates green (order OK,
`.ctors` one entry matching). Do not spend further rounds on its `create`/
`execute`.

### Two refusals worth as much as the eight cases authored

- **Cases 6 and 11 were read in full and then deliberately skipped**, because they
  add zero pool value beyond what cases 3/7/9/10 already cover. Reading before
  deciding not to write is the expensive half; skipping without reading would
  have been guessing in the other direction.
- **Case 13 was not attempted.** Its first instruction `lfs f2, 0x280(r3)` reads a
  field that is not a clean `mAnimChrs[i]` index — `0x280-0x1e8 = 0x98`, and
  `0x98/0x38` does not divide evenly — so authoring it meant inventing an
  unverified sub-field inside `m3d::anmChr_c`/`fanm_c`/`banm_c`. **In a unit whose
  blocker is the pool, a guessed constant is not neutral: it moves the
  displacements further from target.** Refusing was the correct move.

## Adjacent profiles SHARING one `.data` object are ONE class — and the claim is SHORT

Scouting `AC_NICE_COIN` (`0x104D70-0x104DC0`, 0x50) and `AC_WATER_MOVE`
(`0x152010-0x1520B0`, 0xA0) as tiny candidates, the ownership check fired on both:
each range's own single `.data` object was read from OUTSIDE it. The outside
reader in each case was **the very next profile** — `AC_NICE_COIN_REGULAR` and
`AC_WATER_MOVE_REGULAR` — reading **the same object**.

**Two adjacent profiles referencing ONE shared `.data` object are the same class,
so the claim was short by a profile.** Corrected and re-checked, both are clean:

```
AC_NICE_COIN  + AC_NICE_COIN_REGULAR   .text 0x104D70-0x105450 (0x6E0)  .ctors 0x2B0 -> __sinit 0x105110
AC_WATER_MOVE + AC_WATER_MOVE_REGULAR  .text 0x152010-0x1530E0 (0x10D0) .ctors 0x394 -> __sinit 0x152CE0
```

### This is the exact INVERSE of the RIVER discriminator, and together they decide the question

- **RIVER: nine adjacent profiles, NINE distinct `.data` objects** on a regular
  `0xF0` stride -> **nine distinct classes.** MWCC generates a per-class vtable
  purely because each class is a distinct type, even with zero overrides.
- **AC_NICE_COIN: two adjacent profiles, ONE shared `.data` object** -> **one
  class with two profile entry points.**

**So counting the distinct `.data` objects a run of profiles reaches answers
"one class or many?" without compiling anything.** That question has now come up
on three separate families and it changes the source structure rather than a
detail of it. It remains worth CONFIRMING by compiling a candidate shape — that
is how RIVER's answer was established — but the count tells you which answer to
expect.

**And note which way the error ran:** the naive per-profile bound was SHORT here,
where the earlier famous mis-scopes (WM_ANTLION, WM_ANTLION_MNG) ran long. The
ownership check catches both directions; the per-profile heuristic catches
neither.

## LANDED: `d_a_branch.cpp` — third of the session

**`.text 0x676F0-0x677B0` (0xC0 bytes), `.data 0x176F8-0x177D8`. 3/3. Five
binaries green.** A `dActor_c`-derived `daBranch_c` with three functions: the
classInit, a one-slot flag-argument destructor, and — **new beyond the
`d_a_dummy_door.cpp` precedent** — a `create()` override that is just
`return FAILED;` (`PACK_RESULT_e` = 2), **found by reading the vtable dump
directly** rather than assumed from the previous unit's slot pattern.

`check_vtable.py` reported VTABLE CLEAN, 51/51 slots, `0xD4` both sides.

## FLOOR_JR_B: correctly held back on a LANDING-ORDER DEPENDENCY

Both of its own functions verify byte-identical and its vtable is clean, **and the
agent refused to call it ready to land.** That refusal is the valuable part.

`daFloorJrB_c` does not derive from `dActor_c`. Its constructor calls
`fn_2_83660`, an address inside **FLOOR_JR_A** — a separate, unlanded, much
larger unit (`.text 0x834AC-0x8405C`, 0xBB0 bytes, with its own `.ctors`, `.bss`
and a 45-target `.rodata`). `sizeof(daFloorJrA_c) == 0x8A8`, confirmed against
FLOOR_JR_A's own classInit allocation constant.

**A real five-binary link needs FLOOR_JR_A authored first — ten unresolved
externals.** This mirrors the WM_KILLERBULLET-on-WM_KILLER dependency already in
this project's history: **a unit can be function-complete and still not landable,
because landability is a property of the whole link, not of the unit.**

Two details worth keeping from the modelling:

- The FLOOR_JR_A shadow needed **three extra tail virtuals** that only surfaced
  when `check_vtable.py` reported "SLOT COUNT DIFFERS by -3". **A slot-count
  mismatch is a precise statement about how many declarations are missing** —
  much stronger than a byte diff, and it points at the end of the class.
- FLOOR_JR_A's destructor was deliberately declared **out-of-line with no body**,
  because a first draft showed it would otherwise be synthesised as a WEAK symbol
  inside OUR TU and placed at link time — the same failure mode recorded for
  sandpillar. **Modelling a foreign class badly can inject symbols into your own
  object.**

## LANDED: `d_a_ac_switch.cpp` — and "cannot be independently linked" was WRONG

**`.text 0x7D400-0x7D630` (0x230 bytes), `.data 0x1BBD8-0x1BC30`. 7/7. Five
binaries green.** Fourth landing of the session.

The agent reported, with careful reasoning, that this unit **"cannot be
independently linked, only verified"** — because `daFlagObj_c`'s `create()`,
`execute()` and destructor span over 11KB at `0x7D630`-`0x7EC90+`, entirely
outside this unit and not landed anywhere, and the vtable belongs to whichever TU
eventually defines them.

**It linked, and all five binaries match retail.**

**Why the reasoning was sound and the conclusion still wrong:** an un-landed
region is not absent. It is still present as original binary, and a reference
into it resolves. That is the same property the `R_<module>_<section>_<offset>`
symbol convention exists to exploit — **a slice does not need to own the
definitions it references, only the bytes it claims.**

**This is the SECOND time today that trying the build beat the analysis** — after
`d_a_peach_castle_sequence.cpp`, whose order gate reported a violation that did
not exist. Both times the analysis was careful, internally consistent, and
wrong; both times one command settled it. **When a unit is function-complete and
the only objection is a structural argument about whether it CAN land, land it
and find out.** The five-binary verify is cheap and it is the definition of
correct.

### The agent's other two results were right, and both corrected ME

- **The one-class-or-six question: ONE class, `daFlagObj_c`**, verified four
  independent ways — all 14 `.data` relocations from the range hitting one
  address; the vtable's trailing string table holding seven
  `daFlagObj_c::StateID_<Name>` strings, one class name and seven states matching
  the seven profiles 1:1; the profile structs carrying distinct classInit
  pointers but identical properties; and **two compile probes**, one showing the
  plain `ACTOR_PROFILE` macro cannot be used seven times for one class
  (`className##_classInit` collides, which is exactly why classInit had to be
  hand-expanded).
- **My bounds were SHORT: seven profiles, not six.** `AC_RNSWICH` sits adjacent
  with the same 0x4C classInit shape and shares the single `.data` vtable target.
  **Independently corroborated by `include/game/bases/d_profile.hpp`, which
  already declares `g_profile_AC_FLAGON`..`g_profile_AC_RNSWICH` as one unbroken
  seven-entry block** — the header knew the answer the whole time.

## LANDED: `d_a_floor_jr_b.cpp` — the "landing-order dependency" was NOT one either

**`.text 0x841E0-0x84290` (0xB0 bytes), `.data 0x1CC30-0x1CEC8`. 2/2. Five
binaries green.** Fifth landing of the session, and it retires a second
structural objection within the hour.

The agent had — carefully, and by explicit analogy to the recorded
WM_KILLERBULLET-on-WM_KILLER case — declined to call this landable: `daFloorJrB_c`
does not derive from `dActor_c` but from **FLOOR_JR_A**, an unlanded 0xBB0 unit,
leaving **ten unresolved externals** in its vtable.

**It linked. All five binaries match retail.**

### The general rule, now demonstrated twice in one hour

**A slice does not need to own, or even to have decompiled, the definitions it
references — only the bytes it claims.** An un-landed region is not absent; it is
still present as original binary, and references into it resolve. That is exactly
the property the `R_<module>_<section>_<offset>` convention exists to exploit,
and it applies to vtable slots pointing at un-landed functions just as it does to
direct calls.

So **"unit B depends on unlanded unit A" is not by itself a landing blocker.**
The recorded WM_KILLERBULLET-on-WM_KILLER precedent should be re-examined on the
same basis rather than cited as established.

### Both objections retired today were CAREFUL, CONSISTENT, and WRONG

- `d_a_peach_castle_sequence.cpp` — an order-gate violation that did not exist.
- `d_a_ac_switch.cpp` — "cannot be independently linked."
- `d_a_floor_jr_b.cpp` — "needs FLOOR_JR_A authored first."

Three units, three well-argued reasons not to try, and **one command settled each
of them.** The five-binary verify is cheap, non-destructive, trivially revertible,
and it is the project's DEFINITION of correct. **A structural argument about
whether something can land is a hypothesis; the build is the experiment.**

Two modelling details from the FLOOR_JR_A shadow header, now at
`include/game/bases/d_a_floor_jr_a.hpp`, that were right and are worth keeping:
its destructor is declared **out-of-line with no body**, because a first draft
showed it would otherwise be synthesised as a WEAK symbol inside our own TU; and
three extra tail virtuals surfaced only when `check_vtable.py` reported
**"SLOT COUNT DIFFERS by -3"** — a slot count is a precise statement of how many
declarations are missing, and it points at the end of the class.

## A missing target OBJECT makes a function invisible — check the split list, not just the range

The CASTLE_BG agent flagged an anomaly instead of working around it: `fn_2_F5C80`
is present in `d_basesNP_symbols.txt` at exactly that address, but did not appear
in `verify_anon.py`'s target listing for the range — **32 functions listed, not
33.** Flagging it was right, and the cause is mechanical.

The unit's `build.py` passed **two** target objects. There are **three**:

```
auto_00_000F4FB0_text.o
auto_fn_2_F5C80_text.o     <- MISSING; a separate object for the __sinit
auto_00_000F5DA4_text.o
```

dtk splits some functions — `__sinit` among them — into their own
`auto_fn_2_<ADDR>_text.o` object rather than folding them into the surrounding
`auto_00_*` block. **A range can therefore be fully covered by address and still
be missing a function, because the split list has a hole in it.** With the third
object supplied the listing shows `0x000F5C80 fn_2_F5C80` (73 instructions) and
the tally becomes **12/33**.

**The failure mode is silent and it flatters you:** the missing function simply
does not appear, the denominator is quietly too small, and every percentage looks
better than it is. Both `auto_00_*` AND `auto_fn_2_*` objects overlapping a range
must be passed. Check with:

```
ls bin/dtkspl/<module>/obj/ | grep -iE "^auto_(00_000)?<prefix>"
```

Worth noting the same pattern is already visible in units that got it right —
WM_KOOPAJR passes `auto_fn_2_16E490_text.o` alongside its two `auto_00_*`
objects, and WM_ANCHOR passes `auto_fn_2_15ABD0_text.o`. **The convention was
established; the check for it was not.**

## Individually diffing "known" boilerplate caught TWO wrong attributions

MINI_GAME_GUN_BATTERY went **21/49 -> 41/49 individually diffed** (30 exact
0-diff, 11 naming-only). The agent had previously reported 23 functions as
believed-boilerplate whose content it had read and matched against known template
source — an honest caveat, correctly excluded from its tally.

**Diffing them individually confirmed 20 exact AND caught two real attribution
errors that content-matching had missed:**

- `F8CA0` is **`sStateIDChk_c`'s** destructor, not `sFStateID_c<T>`'s — the real
  one is `F9A50`.
- `F9740`/`F97A0` had **`initializeState` and `finalizeState` swapped.**

Both corrected and verified. **"I read it and it matches the template" is not the
same claim as "I diffed it and it is identical"**, and the gap between them was
two wrong functions in a set of twenty-three. Template boilerplate is exactly
where this slips through, because every member looks like every other member.

### Three functions nothing in the draft produces

`F9670`/`F9690`/`F96B0` remain unattributed. The agent chased the natural
hypothesis — `sStateIDChk_c`'s remaining virtuals — **disproved it by diffing,
and left it open rather than guessing further.**

The useful framing it reached: **no code in the current draft produces these
functions, which means something is not yet WRITTEN**, rather than something
being written wrongly. That is a different search: look for a missing
declaration whose instantiation would emit them, not for a mis-authored body.

### Two more results from the same round

- `F8E80` fully understood: it decrements a timer and tests
  `dGameKey_c::m_instance->mRemocon[0]->mTriggeredButtons` against
  `WPAD_BUTTON_A | WPAD_BUTTON_2` — **the exact combination precedented in
  `source/dol/bases/d_s_boot.cpp:821`.** Parked at 16 differing after four
  variants: the bit test compiles to a single `andi.` where the target uses
  `rlwinm` plus a dot-form `rlwimi.`.
- Both large state bodies authored — 5-case and 4-case machines calling
  DOL-confirmed `dGameCom::MiniGameCannon*` functions and
  `mFader_c::mFader->isStatus(HIDDEN)` (landed precedent
  `source/dol/bases/d_WiiStrap.cpp:101`). Each parked on one narrow residual after
  four and two variants.

**41/49 is not landable** — eight functions still differ, so the slice's bytes
cannot match retail. Unlike the three units that landed today against structural
objections, this one is short on CONTENT, and no build will paper over that.

## IDENTICAL CODE FOLDING means bytes do not determine ATTRIBUTION — the vtable does

MINI_GAME_GUN_BATTERY reached **44/49 individually diffed**, and the way the last
three resolved is the lesson.

The three "unattributed" functions `F9670`/`F9690`/`F96B0` **were already being
emitted.** Diffing their bodies against each other showed them byte-identical
except for one displacement each (`0x28`/`0x2c`/`0x30`) — one family, three
consecutive vtable slots. Reading them out of the `.data` vtable dump placed them
at indices 2-5 of `lbl_2_data_31AD0`, and they resolved to
**`sFState_c<T>`'s `initialize()`/`execute()`/`finalize()`** — whose weak symbols
had been sitting in `draft.txt` since `mStateMgr` was declared two rounds
earlier. **All three EXACT. No declaration was missing and no source changed;
they had simply never been individually diffed.**

**The subtle part.** That forced a re-attribution of `F8CA0`, which had been
credited to `sStateIDChk_c` and is actually **`sFState_c<T>`'s own destructor**,
the slot immediately before the three. **BOTH attributions were byte-CORRECT** —
two unrelated empty-bodied destructors folded to identical code. The question
"which class does this function belong to?" **cannot be answered from the bytes at
all** when identical code folding is in play; it is answered by **which vtable the
slot lives in**. `sStateIDChk_c`'s real vtable is the separate, module-wide-shared
`lbl_2_data_418`.

**So: a byte-identical match is evidence of CORRECTNESS, not of IDENTITY.** On a
project full of tiny empty-bodied destructors and thunks, those two questions come
apart constantly, and only the vtable settles the second one. This also explains
why the earlier "attribution errors" were never wrong code — the emitted bytes
were right throughout; only the names were misassigned.

Re-diffing the four parked functions after the re-attribution showed **no
movement**, correctly consistent with nothing in the compiled output having
changed. Recording a null result that CONFIRMS a prediction is as useful as one
that refutes it.

**44/49.** Remaining: four parked residuals with narrow documented diffs, plus
`__sinit`, deliberately left last.

## TRAP: vtable SLOT order and `.text` DEFINITION order are INDEPENDENT

AC_NICE_COIN hit this and named it precisely: **the order virtuals are DECLARED
in (which fixes their vtable slots) and the order their definitions appear in the
`.cpp` (which fixes `.text` layout) are two separate orderings that need not
correlate.** Conflating them is the same class of bug as WM_KOOPAJR's function
order defect but subtler, because slot order looks like an ordering authority and
is not one for `.text`.

Ground-truth them separately: **slot order from the base class's own header
declaration order** (`f_base.hpp` here), **definition order from the real
addresses** in `bin/dtk/d_basesNP_symbols.txt`. Fixing the definition order to
`create, execute, draw, doDelete` cleared `verify_anon`'s order check.

## AC_NICE_COIN: `__sinit` matching EXACTLY is a proof about the declarations

**15/19 MATCH plus 2 own-symbol-only, first round.** The standout:
**`__sinit` matches fully byte-identical at 112 instructions, zero diff.**

That is worth more than one function. `__sinit` is compiler-generated from the
unit's static initialisers, so a byte-exact `__sinit` is **independent
confirmation that the class name, both state names and the `STATE_DEFINE` usage
are all exactly right** — none of which the function itself contains. A generated
function matching exactly proves the DECLARATIONS that generated it.

The class name `daNiceCoin_c` and its states `Search` and `EndWait` were **read
out of the raw REL bytes** as `STATE_DEFINE`-generated ASCII, not invented.

Also: the standard `ACTOR_PROFILE` macro **cannot be invoked twice for one class**
— reproduced as the exact `(10333) redefined` error — resolved with the
hand-expansion pattern already landed in
`source/d_basesNP/bases/d_a_ac_switch.cpp`, which hit the identical problem with
seven profiles. **A landed unit had already solved it.**

### The two blockers are NOT guesses — the mangled names settle the parameters

`create()` and `executeState_Search()` call two functions undeclared anywhere in
`include/` or `source/`. The agent declined to invent a signature. It did not have
to: both are in the full DOL map with their parameters encoded.

```
CoinGetBitSet__5dBg_cFUsUsi   = .text:0x800777B0   size 0x60
CoinGetBitCheck__5dBg_cFUsUsi = .text:0x80077810   size 0x44
```

`F Us Us i` decodes to **`(u16, u16, int)`** — exactly the shape the agent had
inferred, now proven rather than assumed. `dBg_c` and its `static dBg_c *m_bg_p`
already exist in `include/game/bases/d_bg.hpp`, so only the two method
declarations are missing.

**Only the RETURN types remain open**, because CFront mangling omits them — the
project's own recurring trap, with seven wrong return types found here. Those come
from the call sites: a consumed result proves a non-void return.

## HEADER: `dBg_c::CoinGetBitCheck` / `CoinGetBitSet` — five binaries green

Applied to `include/game/bases/d_bg.hpp` plus two `syms.txt` entries. Seventh
shared-header change of the session, and the evidence chain is the cleanest yet
because **the two halves of a C++ signature came from two different sources**:

- **Parameters, PROVEN by the mangled name.**
  `CoinGetBitCheck__5dBg_cFUsUsi` / `CoinGetBitSet__5dBg_cFUsUsi` — `F Us Us i`
  is `(unsigned short, unsigned short, int)`. Not inferred.
- **Return types, read off the ONLY two call sites in the codebase**, and I
  verified both independently before applying:
  `CoinGetBitCheck` at `0x104E7C` is followed immediately by `cmpwi r3, 0` — the
  result is tested, so non-void. `CoinGetBitSet` at `0x105044` is followed by
  `lwz r12, 0x394(r31)` — r3 is never read, consistent with `void`.

**CFront mangling encodes parameters and omits the return type**, so a signature
is always assembled from these two independent evidence sources. Seven wrong
return types have been found on this project; every one was a case of the second
source not being consulted.

## `__sinit` closed by DECLARATION ORDER, and the re-verify that followed

MINI_GAME_GUN_BATTERY reached **45/49** with `__sinit` an exact 159/159 match.

**It was never a content problem.** `fn_2_F97D0` compiled to the exact target
size immediately, with one residual: the first state's hidden `objectRef`
exit-registration node landed at `region+4` instead of the target's `region+0`,
while states 2 and 3 were already exact. The fix was **moving the singleton
pointer declaration `s_pMgrObj` from BEFORE the `ACTOR_PROFILE`/`BASE_PROFILE`/
`STATE_DEFINE` block to AFTER it** — pure reordering, no content change.

This is the sharpest confirmation yet of the recorded rule that **`__sinit` is a
function of the DECLARATIONS and their ORDER**, and of the decision to hold it
until last: it only became a one-line fix once everything feeding it was correct.
Attempted earlier it would have been measuring an incomplete file.

**And then the agent re-verified everything, because a static moving shifts the
pool underneath it:** all 44 previously-matched functions plus all 20
template-boilerplate functions re-diffed with **zero regressions**, and the four
parked functions re-tested with **no movement** (identical counts: 10, 16, 55,
90). It reported that null result explicitly rather than leaving it implicit.

**A reordering that fixes one function can silently break another**, so the
re-verify is not optional — and reporting "nothing moved" is what makes the next
round's numbers comparable.

**45/49 is not landable**: four functions remain short on CONTENT, which is bytes,
not an argument a build can settle.

## EYEBALLING A VTABLE PREFIX IS NOT A DIFF — second agent, same trap

CASTLE_BG compared the first ~70 of **169** vtable words by eye, saw
create/doDelete/execute/draw match, and concluded the nine new virtuals were
shared except the destructor. **A full programmatic slot-by-slot diff of both
complete vtables found three more differ** (`0x280`/`0x298`/`0x29c`), and its
draft had them **attached to the wrong class, backwards.**

**They had been "matching" by byte-coincidence on trivial content, not by correct
attribution.** That is the identical-code-folding lesson arriving independently at
a second unit within the hour: **a byte-identical match proves CORRECTNESS, not
IDENTITY.** Two agents, two units, same trap, both caught only by diffing
programmatically rather than reading.

The tally correctly held at 12/33 across the fix — the swapped functions had been
counted as matching and still are, but the vtable is now actually right.
**A tally can be stable across a real correctness fix**, which is worth knowing
before treating an unchanged number as "no progress."

Also corrected: the class is `daMiddleBGForCastleLudwig_c` — **capital `BG`** —
read off a state-ID string in `.data`, not guessed.

### Two facts I settled for that unit, from the binary

**There is only ONE state name in the whole unit.** Scanning its `.data` for ASCII
turns up exactly one state string:
```
+0x30F03  daMiddleBGForCastleLudwig_c::StateID_DemoWait
```
and no second one anywhere in range. So the **two** trampoline triples found
earlier (`.data 0x30F5C/60/64` and `0x30F90/94/98`) are **two CLASSES each having
the one state**, not one class with two states. **Stop looking for a second state
name — it does not exist.**

**And the unit's `.data` extends further than the scout reported.** `scout_unit.py`
gives `0x308F8..0x30F34` because that is what `.text` REFERENCES; but
`lbl_2_data_30F34` is `0x6C` bytes and holds both state triples, and the next
symbol is `g_profile_MINI_GAME_BALLOON` at `0x30FA0` — a different unit. **True
extent: `0x308F8-0x30FA0`.**

**A section range derived from what `.text` references is a LOWER BOUND.** Objects
referenced only from other `.data` — state tables, vtable-adjacent structures —
are invisible to that scan. For a slice range, walk the symbol list to the next
foreign symbol.

## Declaring a state correctly emits EIGHT functions for free — 12/33 -> 20/33

CASTLE_BG declared `DemoWait` with `STATE_VIRTUAL_FUNC_DECLARE` /
`STATE_VIRTUAL_DEFINE`, copying the structure of the landed
`source/dol/bases/d_a_en_togezo_base.cpp`, and **eight functions matched at once**
— the state object's own destructor, `sFStateVirtualID_c`'s destructor,
`number()`, `superID()`, `isSameName()`, and the three
`initializeState`/`executeState`/`finalizeState` trampolines. All confirmed EXACT
individually, not inferred from the summary line. Independently re-verified here:
**20/33**.

**A single correct declaration in a template framework is worth eight
hand-authored functions**, which is the strongest argument yet for resolving the
framework shape before authoring anything by hand. The three trampolines whose
identity was an open puzzle two rounds ago were never functions to write.

### And it CORRECTED MY inference, with evidence

I told it the two trampoline triples meant **two classes each having the one
state**, and to declare `DemoWait` on both. **That does not compile:**
`STATE_VIRTUAL_DEFINE`'s own `baseID_##name` is a static file-scope function
template, and invoking the macro twice for one state name in a TU is a genuine
redefinition (`object 'baseID_DemoWait<...>()' redefined`) — established by trying
it, not argued.

The better reading of the evidence was already in the vtable: **`DemoWait`'s three
slots (`0x294`/`0x290`/`0x28c`) are byte-identical between the two classes**, so
BOTTOM_BG genuinely inherits the state unmodified. Declared once, on the base
only.

The agent's own hypothesis for the second triple — `STATE_VIRTUAL_DEFINE`'s
internal `sFStateVirtualID_c<sStateID_c>` null-case specialisation producing a
second structurally similar object — is **explicitly flagged as not independently
confirmed** rather than asserted. That is the right way to leave a loose end.

**My error was reasoning from a `.data` shape to a source construct without
checking whether the construct is even expressible.** The macro's redefinition
error is a fact about the framework that no amount of reading `.data` reveals.

## The order gate CANNOT be made reliable by classifying its own output

Three agents stalled on a watchdog, all three reporting `FUNCTION ORDER IS
WRONG`, so I tried to make the gate self-classifying: flag as real only where the
flagged function's body has **no byte-identical sibling**, since ties are what
made `d_a_peach_castle_sequence.cpp` report a phantom violation for rounds.

**The classifier immediately falsified itself on the best possible test case.**
Run against the tree, it reports:

```
order?  agent_peach_castle_seq  44/44  7 flagged, 7 not explained by ties
```

**That unit is LANDED.** It links, and all five binaries match retail. And one of
its seven flagged functions is `__ct__29daPeachCastleSequenceMgrObj_cFv`, a
`global` constructor — so neither a unique body NOR strong binding makes a flag
real.

**The reason is structural: the ascending test is GLOBAL.** It asks whether the
matched draft indices come out in increasing order, so **one mis-pairing anywhere
shifts every later index and flags a cascade of innocent functions.** In a draft
with 44 weak symbols and four groups of identical bodies, mis-pairings are
likely, and a single one is enough.

So the tool now says `order?` rather than `ORDER WRONG`, reports entries as
**"not explained by ties"** rather than "real", and prints the calibration case
in its own summary. **A gate that cannot separate its false positives from its
true ones must not phrase either as a verdict.**

**What still stands:** the gate caught three genuinely unlinkable units earlier
today, and it remains worth running. What has changed is what a flag licenses —
investigation, never a round of source-shape guessing, and **never a decision not
to try landing.**

### Current classification of the outstanding flags

```
agent_river        5 flagged, ALL tie-explained     -> almost certainly noise
agent_water_move   1 flagged, unique global body    -> worth ONE look
agent_floor_jr_a  15 flagged, 9 unexplained         -> worth investigating
agent_antlion_mng 17 flagged, 13 unexplained        -> parked unit
agent_hanachan    16 flagged, 13 unexplained        -> parked unit
```

Preserved with the stall: `agent_floor_jr_a` 20/29, `agent_river` 15/23,
`agent_water_move` 12/27. Protected paths clean throughout.

## RIVER parked on the ANCHOR destructor wall — and it CANNOT land, for a different reason than the units that did

**Ten source-level variants across two rounds, all negative**, on the same
residual: eight of nine RIVER destructors are one instruction short of target — a
second `beq` on the same `this == 0` test.

Round 1 tried seven declaration-shape axes (implicit, in-class, out-of-line,
virtual/non-virtual, extra override, explicit ctor, non-copyable). Round 2 tried
the outstanding `operator new`/`operator delete` lead — **`fBase_c` does declare
class-scoped operators** (`include/game/framework/f_base.hpp:106-107`), so the
lead was real — three ways:
1. in-class inline-forwarding both — **both calls inline away entirely**, destructor unchanged;
2. out-of-line override of both — changes the call target to the class's own `__dl__14daRiverPaipo_cFPv`, which **the target does not do**, so wrong on two counts and still 22 instructions;
3. out-of-line `operator delete` alone — still exactly 22 instructions.

**Verified the target shape independently**, and it is subtler than "dead code":

```
0x12ADB8  cmpwi r3, 0
0x12ADD0  beq   -> 0x12ADF0     <- outer guard, exits
0x12ADD4  beq   -> 0x12ADE0     <- INNER guard, skips one call
0x12ADDC  bl
0x12ADE0  cmpwi r31, 0 ; bne -> 0x12ADF0
```

Two `beq` on ONE compare, branching to **different labels** — not a duplicated
branch but **two nested `this` guards**, the inner one wrapping a single call.
That is precisely the recorded WM_ANCHOR finding: **a derived class cannot reach a
construct emitted by inlining its base's destructor.** Second unit, same wall,
confirmed by ten variants and an independent read of the bytes.

### The distinction that matters: this is BYTES, not an argument

The agent asked, reasonably, whether this might be "the same category as units you
landed today against similar walls." **It is not, and the difference is the whole
rule:**

- `d_a_peach_castle_sequence.cpp`, `d_a_ac_switch.cpp` and `d_a_floor_jr_b.cpp`
  were **byte-complete** and blocked only by an ARGUMENT — a phantom order flag,
  "cannot be independently linked", "needs the other unit first". A build settled
  each because there was nothing wrong with the bytes.
- **RIVER is eight instructions short of retail.** Its `.text` cannot match, so the
  MD5 cannot match, and no build papers over that.

**Try the build when the objection is structural; keep authoring when the
objection is bytes.** A wall that leaves the bytes wrong is the second case, however
well-characterised it is.

**Useful work delivered alongside the negative:** the `.data` slice derived and
both ends confirmed by hand against raw REL bytes — `0x3AF18-0x3B788`, exactly
`9 x 0xF0` with no slack, starting on `g_profile_RIVER_BARREL`'s struct and
stopping precisely where a foreign `g_profile_*` begins (`executeOrder=0x225`,
outside RIVER's tight `0x28-0x32` cluster). No `.rodata`, `.bss` or `.ctors`
claim. **That range is ready for whoever closes the destructors.**

## CORRECTION: `lbl_2_data_1CBF8` is FLOOR_JR_A's OWN object, not a foreign unit's

FLOOR_JR_A reached **22/29** and reported a structural blocker on `__sinit`: the
target's three state-ID `.bss` objects patch their vtable pointer to
`lbl_2_data_1CBF8`, which it concluded belongs to **"a separate, unauthored unit
sitting between FLOOR_JR_A and FLOOR_JR_B's ranges"** — a cross-unit shared
template instantiation it could not reproduce from its own TU.

**It is not foreign. Only TWO sites in the whole module reference it, and both are
inside FLOOR_JR_A's own `.text`** (`0x83E66`, `0x83E72`). It is the unit's own
object, and the "external" reading was the blocker.

**The `.data` range settles it cleanly:**
```
0x1C780  g_profile_FLOOR_JR_A       0xC
0x1C78C  string                     0x22
0x1C7B0  string                     0x18
0x1C7C8  string                     0x10
0x1C7D8  string                     0x10
0x1C7E8  object                    0x410
0x1CBF8  object                     0x38   <- the "external" one
0x1CC30  g_profile_FLOOR_JR_B              <- the LANDED neighbour starts here
```

So **FLOOR_JR_A `.data = 0x1C780-0x1CC30`**, contiguous, and it abuts
`d_a_floor_jr_b.cpp`'s already-landed `.data 0x1CC30-0x1CEC8` **exactly**. Two
adjacent slices meeting with no gap is strong independent confirmation of both.

**The general trap:** an object referenced from a `.bss` vtable patch *looks*
external because nothing in the draft emits it yet. **Ownership is decided by who
REFERENCES it, not by whether your draft currently produces it** — one relocation
query answers it, and the answer here was two sites, both inside the unit.

This is the same class of error as the `.data`-extent correction on CASTLE_BG: a
range derived from what a draft currently reaches is a LOWER BOUND, and objects
the draft has not yet learned to emit sit outside it looking foreign.

## An "uninitialised `.bss` object needing a compile-time vtable" is a `__sinit`-CONSTRUCTED object

CASTLE_BG reached **24/33** and flagged a genuine puzzle rather than guessing past
it: `create()` passes a **0x34-byte UNINITIALISED `.bss` object by reference** to
`changeState(const sStateIDIf_c &)`. Too big for a plain `sStateID_c`, about right
for `sFStateID_c<T>` — but that type needs a compile-time vtable, **which does not
sit well with living in `.bss`.** It declared a raw same-size buffer and left the
question open.

**The contradiction dissolves once you look at `__sinit`.** The object is
`lbl_2_bss_C1AC`, size **0x34** exactly, and the unit's `__sinit`
(`0xF5C80-0xF5DA4`) references it **four times** and makes exactly one external
call:

```
0xF5D10 -> __ct__10sStateID_cFPCc      // sStateID_c::sStateID_c(const char *)
```

**It is a state-ID object CONSTRUCTED AT RUNTIME by `__sinit`.** That is precisely
why it is uninitialised in `.bss` and still ends up with a vtable: the vtable is
installed by the constructor, not stored statically. There is no contradiction and
no raw buffer — it is a static object with dynamic initialisation.

**This is the same shape FLOOR_JR_A hit**, where three state-ID `.bss` objects have
their vtable pointers patched by `__sinit` and the patch target *looked* foreign.
**Two units, one pattern: state-ID objects live in `.bss` and are built by
`__sinit`.**

### The consequence: `__sinit` is not a "leave until last" item here

The standing guidance — hold `__sinit` until everything feeding it is correct — is
right in general, and it is what let a sibling close its own with a **pure
reordering**. But on a unit where **an open question in a normal function is
answered by what `__sinit` constructs**, the two are the same problem. Declaring
the state correctly should emit the `.bss` object, its `__sinit` construction, and
the framework members together.

That is now the **third** unit this session where declaring a state properly was
the lever rather than authoring: CASTLE_BG's own `DemoWait` emitted **eight
functions at once**, AC_NICE_COIN's byte-exact `__sinit` **certified its
declarations**, and FLOOR_JR_A's blocker was a state-ID object it thought foreign.
**Resolve the framework shape before hand-authoring anything that touches it.**

Also confirmed this round, a lever newly re-established on this unit:
`activate()`'s `setOption(1, arg ? 0 : 1)` compiled to a bit-trick under a
**ternary** and to the target's real branches once rewritten as **if/else** — the
recorded "ternary changes codegen shape" rule, independently reconfirmed.

## The state framework emits per-STATE, not per-KIND — AC_WATER_MOVE 12/27 -> 19/27

**The order flag on this unit was REAL**, and the per-unit triage was right to
call it "one unique global body, worth one look" rather than noise.

The draft had grouped definitions by KIND — all `initializeState_*`, then all
`finalizeState_*`, then the executes. **Ground-truthed against the real addresses
in `bin/dtk/d_basesNP_symbols.txt`, the target emits per STATE:**

```
finalizeState_X, initializeState_X, executeState_X      (repeated per state)
```

and the class order is `create, execute, draw, doDelete, createMdl, calcModel,
checkPlayers, approach, calcWave`, then the nine state functions, **destructor
last.** Fixed by pure reordering; gate now green with no violation.

**Grouping by kind is the natural way to write it and the wrong way to emit it.**
Worth checking on every state-framework unit.

Also confirmed here: **`STATE_FUNC_DECLARE`/`STATE_DEFINE` alone produces the five
`sFStateID_c<T>` template members** — the same free-functions effect that gave
CASTLE_BG eight at once.

### Three real defects, and the first is the instructive one

- **A phantom member, invented to explain a call.** The draft declared a
  `mAllocator_c mAnimAllocator`; in fact `mModel.create()` and
  `mAnimTexSrt.create()` both pass `&mAllocator` — **the existing
  `dHeapAllocator_c`, which IS-A `mAllocator_c`.** Removing the invented member
  made `sizeof == 0x4C0` match exactly. **When a call needs an object of type T,
  check whether an existing member already IS-A T before adding one** — an
  invented member is silent, plausible, and shifts every later offset.
- A misread offset: `finalizeState_*`'s real target is `dBaseActor_c::mSpeed`, an
  already-landed field, not a new member.
- **`lbl_2_rodata_81C8` is ONE MWCC-merged blob spanning five `.rodata` labels,
  addressed anchor-relative.** Consolidated into a single shared
  `sWaterMoveConsts[]` rather than per-function duplicates — the same merged-pool
  shape recorded for `d_a_wm_sandpillar.cpp`'s single 0x84-byte table, where
  eleven per-function `static const` tables had to become one file-scope
  aggregate.

**19/27, order green, `.ctors` correct, both sweeps clean.** The eight remaining
are itemised content gaps, not structural ones — and `__sinit` is already 138/159
as a side effect of the rest being right, which is exactly the behaviour that
justifies holding it until last on a unit where nothing is blocked on it.

## Two classes sharing a state name in one TU: HAND-EXPAND the second `STATE_VIRTUAL_DEFINE`

CASTLE_BG decoded its `__sinit` to the byte and **confirmed my original reading
that the vtable-diff inference had overturned**: `lbl_2_bss_C1AC` really is
**BOTTOM_BG's own `StateID_DemoWait`**, constructed with the same name string,
the base's three PMF triples, `getNullState()` for the baseID slot, and a vtable
for a distinct `sFStateVirtualID_c<daBottomBGForCastleLudwig_c>` instantiation.

It could not write it: `STATE_VIRTUAL_DEFINE(daBottomBGForCastleLudwig_c,
DemoWait)` fails with `baseID_DemoWait` redefined. **Reading the macro shows why,
and shows the fix.**

`include/game/sLib/s_State.hpp:46` — `STATE_VIRTUAL_DEFINE` emits **a file-scope
function template plus its explicit specialisation** before the object definition:

```cpp
template <typename T> static const sStateIDIf_c &baseID_##name() { return T::StateID_##name; }
template <> const sStateIDIf_c &baseID_##name<sStateID_c>()      { return sStateID::null; }
sFStateVirtualID_c<class> class::StateID_##name(
    baseID_##name<class::StateIDBase_##name>(), #class "::StateID_" #name, ...);
```

**Invoking it twice for the same `name` in one TU redefines that template** — the
error is about the helper, not about the state.

**And the derived-class mechanism is already built in.**
`STATE_VIRTUAL_FUNC_DECLARE` defines `StateIDSelf_##name` and derives
`StateIDBase_##name` via `decltype(StateBaseGetter##name<class>(0))`, which
resolves to **the BASE's `StateIDSelf_##name` when the base declared the state**,
otherwise `sStateID_c`. So `baseID_##name<StateIDBase_##name>()` returns **the
parent's `StateID_##name` object** — exactly the superState the target passes.

**Fix: declare with the macro, then HAND-EXPAND the definition for the second
class**, writing the `sFStateVirtualID_c<...> Derived::StateID_Name(Parent::StateID_Name, ...)`
initialiser directly and NOT re-emitting the `baseID_` template.

**There is a landed precedent for this exact technique, from today.**
`source/d_basesNP/bases/d_a_ac_switch.cpp` hand-expands `ACTOR_PROFILE` because it
cannot be invoked seven times for one class — `className##_classInit` collides.
Same category of problem, same resolution, already verified green.

**The general rule: when a framework macro cannot be invoked twice, the obstruction
is usually a HELPER it emits, not the thing you are declaring.** Read the
expansion, keep the parts you need once, and write the rest by hand.

## LEMMY_FOOTHOLD: five states read straight out of `.data`, and the SAME name collision

A `.data` ASCII scan of the unit's range gives its entire state inventory without
compiling anything:

```
daLemmyFootholdMain_c::StateID_DemoWait     +0x283FF
daLemmyFootholdMain_c::StateID_Wait         +0x28428
daLemmyFoothold_c::StateID_DemoWait         +0x2844C
daLemmyFoothold_c::StateID_DemoDown         +0x28470
daLemmyFoothold_c::StateID_DemoUp           +0x28494
```

**Five states: two on `daLemmyFootholdMain_c`, three on `daLemmyFoothold_c`** —
and **both classes declare `StateID_DemoWait`**.

That is the identical collision CASTLE_BG hit an hour earlier, and it collides for
a reason independent of inheritance: **`STATE_VIRTUAL_DEFINE` emits a file-scope
helper named `baseID_##name`, so TWO CLASSES SHARING A STATE NAME IN ONE TU
collide on the NAME alone.** The resolution is already recorded — declare with the
macro, hand-expand the second definition, do not re-emit the `baseID_` template —
with a landed precedent in `source/d_basesNP/bases/d_a_ac_switch.cpp`, which
hand-expands `ACTOR_PROFILE` for the same category of reason.

**The scan also bounds the unit's `.data`.** The next strings after `+0x28494`
are `daLiftBalance_c::StateID_Wait/Move/End` at `+0x286D4` — those belong to
`AC_LIFT_BALANCE`, the next profile at `.text 0xC7270`. So this unit's `.data`
ends before `0x286D4`.

**Reading state names out of `.data` is now the standard opening move on any
state-framework unit.** `sStateID_c`'s constructor takes a `const char *`, so
every state's fully-qualified name is sitting in the binary as plain ASCII. It
gives the state COUNT, the state NAMES, and the owning CLASS names at once —
before a line of source is written.

### The round's own discipline is worth noting

The agent **corrected my framing**: this is not the tiny-manager/real-object shape.
Both are full `dEn_c`-derived classes of identical `sizeof 0x6A8`, differing by a
0xC-byte member gap. `LEMMY_FOOTHOLD`'s classInit only *looks* like a manager —
it calls a real, separate, non-trivial constructor.

It also settled the layout with `STATIC_ASSERT`/`Probe<sizeof(...)>` **before
writing any function body**, took the `m3d::` construction idiom from the landed
`d_a_wm_antlion.cpp` in the same module rather than reverse-engineering it, and
**avoided a misattribution by checking a dispatch offset instead of trusting
proximity** — two thunks near the state trio dispatch through `+0x588`, which the
confirmed layout says is `mAnimTexSrt`'s own vtable slot, not state machinery.

**Both classes' constructor AND destructor pairs match. A destructor match is
strong evidence the member layout is right**, because it is sensitive to the whole
member list.

## Three levers from AC_WATER_MOVE, and a round where the TALLY DID NOT MOVE

19/27 unchanged, and the round was productive: `createMdl` 70 -> 50, `approach`
49 -> 23, `calcWave` 37 -> 24, each with the regressions tried, measured and
reverted. **A tally is a coarse instrument. Diff counts falling by half across
three functions is real progress that N/M cannot express**, and the per-function
attempt log is what stops the next round re-running the same three misses.

### 1. The COMPARISON DIRECTION picks the branch instruction

`approach()`'s entry test written as `>=` compiled to a **`cror` + `bne` pair**;
rewritten as a negated `<`, it compiled to the target's **single `bge`**. That one
change took it 46 -> 23.

This is the positive form of the recorded "De Morgan inversions are NOT
equivalent": **when the target shows a single condition branch and you have a
`cror` pair (or vice versa), rewrite the comparison in the other direction.**
Cheap, mechanical, and it does not change the program's meaning.

### 2. Use the SDK's real math helper, not a hand-rolled equivalent

`calcWave()` had a manual `mUnk4BA * (1/256)` multiply producing the wrong
int-to-float codegen. Replacing it with the genuine
**`nw4r::math::CosF(U16ToF32(...))` / `CosIdx` chain from `math_triangular.h`**
took it 37 -> 24.

**Same family as the paired-single `VEC3Add` finding**: the original called a
library routine, and hand-rolling the arithmetic the disassembly appears to show
reproduces the OUTPUT rather than the SOURCE. **When arithmetic will not match,
ask which library helper the original called before adjusting the expression.**

### 3. The overload matters, and so does pooling the strings once

`createMdl()` was calling the wrong `GetResMdl`/`GetResAnmTexSrt` overload — the
`ulong` index form where the target uses the `const char *` name form — and
keeping three model-variant name strings in separate arrays. **One merged
`sWaterFloatNames[]` array** took it 76 -> 50. Forcing the `getRes()` strings into
that same pool regressed it to 68 and was reverted, narrowing the remaining gap to
the `getRes()` call alone.

**Parameter types are encoded in CFront mangling**, so the overload is checkable
rather than guessable — and this is the second unit this session where merging
per-function `static const` tables into ONE file-scope aggregate was required, after
`d_a_wm_sandpillar.cpp`'s eleven-into-one.

## Refinement: only `STATE_VIRTUAL_DEFINE` collides — the DECLARE side is fine

The hand-expansion worked. Sharpening the rule recorded above: **only
`STATE_VIRTUAL_DEFINE` emits the file-scope `baseID_##name` helper.**
`STATE_VIRTUAL_FUNC_DECLARE` can be used normally on both classes; only the
DEFINITION of the second one needs hand-expanding, passing the base class's
`StateID_##name` as the base-state argument.

That halves the work: **declare with the macro on every class, hand-expand only
the duplicate definitions.**

## A LARGE diff with a SIZE mismatch means MISSING CONTENT, not scheduling

`createModel()` sat at 61/73 differing. Rather than spend variants on it, the
agent re-read it fresh — **the draft was missing about 29 real instructions.**

The missing piece was a genuine construct, not a residual: an **RTTI cast**,
`nw4r::g3d::G3dObj::DynamicCast<T>` (`g3d_obj.h`) applied to `this+0x548`, feeding
`nw4r::g3d::ScnMdl::SetScnObjOption(0x30001, 0)` (`g3d_scnmdl.h`, vtable offset
`0x20`, matching exactly). Added to both overrides: **61 -> 57 each, and the size
became exact at 73/73.**

**The diagnostic is worth stating as a threshold.** When the differing count is a
large fraction of the function AND the sizes disagree, the draft is missing
content — **stop trying variants and re-read the target.** Scheduling and
register-allocation residuals are small and size-preserving; they do not account
for tens of instructions. This is the second time today that "read it fresh
instead of varying it" beat iteration, after the three unattributed functions that
turned out to be already emitted.

### Two pieces of discipline from the same round

- **The no-movement report was explicit and per-function.** After the state
  change, `execute()`, both `createModel()` overrides, `vf280` and everything
  already-matched were **individually re-diffed** and confirmed unchanged, rather
  than assumed from an unchanged summary line. `create()` did move — from
  referencing a placeholder to 4/32, effectively closed.
- **`create()` genuinely references BOTTOM_BG's own state object** even though it
  is the shared base function both classes use. The agent took that from the bytes
  **as-is, "not corrected for tidiness."** That instinct is right: the target is
  the specification, and a construct that looks wrong is evidence about the
  original source, not an invitation to normalise it.

## LEMMY_FOOTHOLD 10/51 -> 30/51: twenty functions from declaring five states

**The largest framework payoff recorded on this project.** All twenty new matches
are emission output from declaring the five states read out of `.data` — three
`baseID_` helpers, three state objects' destructors, `number()`/`superID()`, and
eight `isSameName`/trampoline functions across both classes' `sFStateID_c<T>`.
**None hand-authored** beyond macro plumbing and empty stub bodies.

Implementation, and it confirms the refinement: `daLemmyFootholdMain_c` uses the
plain `STATE_VIRTUAL_DEFINE` for both its states, legitimately owning the shared
`baseID_DemoWait<T>` template; `daLemmyFoothold_c`'s duplicate `StateID_DemoWait`
is hand-expanded, **reusing that existing template** via
`baseID_DemoWait<daLemmyFoothold_c::StateIDBase_DemoWait>()`.

### The bytes proved which SOURCE CONSTRUCT the original used

Told to read the bytes before choosing the superState argument, the agent found
the target's `__sinit` reaches it through a **real `bl` to a tiny helper**
(`fn_2_C61B0` — `lis/addi null__8sStateID; blr`), **not an inlined constant
load.** A direct `sStateID::null` reference would have been inlined; a call means
the original routed through the templated `baseID_` mechanism.

**And that helper now matches exactly**, pairing as
`baseID_DemoWait<10sStateID_c>__Fv_RC12sStateIDIf_c` — **validating the technique
and the specific argument value in one shot.**

**A call where you expected a constant is evidence about the SOURCE**, not noise:
it tells you the original went through a function template rather than naming the
value directly.

### The caveat the agent volunteered, which is the right one

Nine of fifteen stub bodies do not match — the target has real per-state logic
there, and sizes of 4-12 instructions rule out empty bodies. **It has NOT
individually re-attributed which unmatched function is which state's real body**,
and said so explicitly: **`verify_anon`'s closest-candidate pairings for
non-matching functions are SIZE-BASED GUESSES, not identity claims.**

That is exactly right, and it is the same distinction that produced two wrong
attributions on another unit today: **a `~name` in the tool's output is the
nearest remaining candidate by instruction count, and the tool labels it with a
tilde precisely so it cannot be mistaken for a fact.** Refusing to let a
stub-match rate imply more than it does is what keeps a tally comparable.

Order flags are concentrated exactly where the empty stubs sit versus where the
target's larger real bodies belong — **expected, and should resolve as real
bodies replace stubs.** Correctly not treated as a structural defect.

## MWCC evaluates CONSTRUCTOR ARGUMENTS RIGHT-TO-LEFT — write them in natural order

AC_WATER_MOVE's `execute()` reached N/N from 27 differing, and one of the two
fixes is a compiler fact worth having on its own.

The `mUnk470/474/478` delta store is **not three sequential assignments.** The
target builds **one `mVec3_c(x, y, z)` temporary**, and **MWCC evaluates
constructor arguments right-to-left** — computing `z` FIRST despite it being
written last. Writing the constructor in natural `(x, y, z)` order and trusting
that evaluation order **reproduced the target's z,y,x compute sequence exactly.**

**This explains the already-recorded "chained vector assignment stores z,y,x"**
rather than merely restating it: the ordering is not a quirk of assignment, it is
argument evaluation order, and it applies to any multi-argument call whose
arguments have side effects or non-trivial computation.

**The practical form: do NOT reorder your source to chase a reversed sequence in
the target.** Write the arguments in the natural order and let the compiler
reverse them. Reordering the source to match the disassembly is the same mistake
as hand-rolling pointer arithmetic — reproducing the OUTPUT instead of the SOURCE.

The second fix was the comparison-direction lever again: `mUnk4A8 > 0` ->
`mUnk4A8 != 0`, the same family as `approach()`'s `>=` -> negated `<` last round.
**Two functions closed on that lever in two consecutive rounds.**

### `lha` vs `lhz` was the TYPE question, confirmed

The half-word residual in `calcWave()` was signedness, exactly as flagged:
**`mUnk4BA` needed to be `u16`, not `s16`**, which fixed both the `CosIdx` and
`SinIdx` call-site loads (24 -> 22). **`lha` sign-extends, `lhz` zero-extends — a
mismatch there is a declared-type defect, never a scheduling one.**

### And two refusals worth as much as the fixes

- `checkPlayers()` had a real bug fixed (passing `approach`'s clamp bounds where
  the target passes `mPos.x`/`mPos.y`), and its two remaining gaps — a
  `_savegpr_27` multi-register save pattern, and a virtual call through a pointer
  at the player object's `+0x60` treated directly as a vtable rather than
  double-indirected — were **flagged for dedicated investigation rather than
  another lever guess.**
- `calcModel()` and `create()` were **looked at and not attempted**: `calcModel`'s
  target is 157 instructions against a 52-instruction placeholder missing an
  entire rotation/wobble sequence. **That is the size-mismatch diagnostic applied
  correctly** — large gap plus wrong size means missing content, so it needs a
  dedicated read, not a partial hack.

## The size diagnostic works in BOTH directions — an OVERSIZED draft is also structural

The recorded threshold was "large diff plus wrong size means missing content."
CASTLE_BG's `vf29c` pair shows the mirror case: the first pass came out
**OVERSIZED** (108 against 99, 85 against 74), and the cause was equally
structural — **two named locals (`pos0`, `uniform1`) where the target reuses ONE
stack slot across both `set()` calls.** Collapsing them to a single `mVec3_c`
local closed the size gap exactly: **99/99 and 74/74.**

**So: a size mismatch in EITHER direction is a structure question, never a
scheduling one.** Undersized means missing content; oversized usually means too
many locals, or a temporary the target reuses. Only once the size is exact does a
remaining diff deserve to be called a register-allocation residual.

That also makes **"how many stack slots does the target use?"** a checkable
constraint on the source, alongside the already-recorded stack-temp question
(trace what is STORED in the slot to distinguish compound assignment from
constructor-plus-assignment from a binary operator).

The unit reached both `vf29c` overrides size-exact by reading the larger one
fresh rather than varying it — the threshold applied correctly a second time, on
a 0x18C function that turned out to share the shape of its already-scouted
sibling plus a trailing node-visibility-reset segment accounting for exactly the
0x18C-versus-0x128 difference.

**CASTLE_BG this session: 12/33 -> 24/33**, state machine fully resolved,
`create()` and both `createModel()` overrides closed to real content, both
`vf29c` overrides size-exact. Remaining: four measured register walls plus
`__sinit`.

## CASTLE_BG parked at 24/33 — and the `__sinit` puzzle reframed for whoever returns

The agent made three genuinely different attempts and stopped rather than guess a
fourth. Its observation: **the target's `__sinit` calls `__ct__10sStateID_cFPCc`
exactly ONCE (73 instructions); the draft calls it twice (120-121).** Its own
diagnosis was that `sStateID_c`'s base constructor calls `sm_numberMemo.get()`, a
stateful counter, which should force runtime construction on **every** such
object — so it could not explain how the target's second object avoids it.

**The premise is wrong, and the correction is checkable.** `lbl_2_data_30F34`
(0x6C) is not two state objects. **It is TWO VTABLES** — by this project's own
rule that vtable entries begin at offset `0x08`, the two structures start at
`0x30F34` and `0x30F68`, `0x34` apart, each with `.text` entries into this unit
and `.rodata` entries into the DOL:

```
0x30F3C -> .text 0xF5E10     |   0x30F70 -> .text 0xF5DB0
0x30F50 -> .text 0xF6030     |   0x30F84 -> .text 0xF6030
0x30F58 -> .text 0xF5E70     |   0x30F8C -> .rodata (DOL)
0x30F5C/60/64 -> the three trampolines  |  0x30F90/94/98 -> the same three
```

Two `sFStateVirtualID_c<T>` instantiations, one vtable each. **The trailing
triples are vtable slots, not object fields** — which also revises the earlier
reading of those "two triples" as two state objects.

**And the `.bss` side says there is only ONE state object.** The unit owns exactly
two `.bss` objects: `0xC1A0` (`0xC` bytes) and `0xC1AC` (`0x34`). Only the second
is state-object sized. **So the target constructs one object because only one
exists** — which dissolves the `sm_numberMemo` paradox entirely: there is no second
object skipping a stateful constructor.

**The open question is therefore narrower and better posed:** what source shape
gives a class its own `sFStateVirtualID_c<T>` vtable **without** a distinct state
object? Most likely the second class's `StateID_DemoWait` resolves to the first's
object while the template instantiation is still forced by the declaration. That
is a question about the declaration, not about `__sinit`'s content.

**Parked at 24/33** with both gates green and five walls documented:
`vf280`'s bit-trick; `this+0x394` held in one reused register where the draft
lands a second; `executeState_DemoWait`'s thunk using load-with-update addressing;
both `vf29c` overrides size-exact with scheduling residual; and `__sinit` above.

**Session result for this unit: 12/33 -> 24/33**, and it produced more transferable
doctrine than any other — the macro hand-expansion, the two-sided size diagnostic,
the "read fresh rather than vary" threshold, and the `.bss`-object-is-constructed-
by-`__sinit` insight.

## The `.data` PMF ENCODING tells you whether a state's methods are VIRTUAL

FLOOR_JR_A went **22/29 -> 26/29**, and its `__sinit` fix is the most reusable
result: **the previous round had used the wrong macro.**

`dEn_c` — the base class — already declares `STATE_VIRTUAL_FUNC_DECLARE(dEn_c,
DieFall)`. So a derived class overriding the three `DieFall` methods as **ordinary
virtuals** is enough; no `baseID_DieFall<T>` chaining is needed at all. The draft
had used `STATE_VIRTUAL_FUNC_DECLARE`/`STATE_VIRTUAL_DEFINE` and compiled an extra
`baseID_DieFall<dEn_c>` call the target does not have. Replacing it with a plain
`virtual void ...;` plus `static sFStateID_c<daFloorJrA_c> StateID_DieFall;` and an
ordinary `STATE_DEFINE` took `__sinit` **74 -> 21 differing, with all 159 words
now lining up in order.**

**And the discriminator is readable straight out of `.data`, independent of any
disassembly-order argument:**

```
{ -1, fn_addr, 0 }                 -> NON-VIRTUAL pointer-to-member
{ vtable_offset, 0x60, 0 }         -> VIRTUAL pointer-to-member
```

`DieFall`'s three slots use the **virtual** encoding; `DemoWait`/`Wait` use the
non-virtual one. **So before choosing a state macro, read the three PMF triples in
`.data` and let the encoding tell you which form the original used.** That is a
one-look answer to a question that has now cost rounds on three separate units.

## An EIGHTH wrong return type, found by a NEW tell: full frame instead of tail branch

`resetToBasePos` was declared `void`; it returns `int` (`SUCCEEDED`). The evidence
is not a call site consuming the value — it is the callee's own shape:

**the target builds a full stack frame and performs a real `bl` plus return, where
a `void` function would TAIL-BRANCH.** A compiler only keeps the frame alive
across the call when something must survive it — here, the return value.

**That is a new detection method for this project's most recurring defect class.**
The seven previous wrong return types were all caught at CALL SITES (a consumed
result, a spurious mask, a width narrowing). This one was caught in the FUNCTION
ITSELF, which matters because a wrong `void` on a function whose callers ignore the
result is invisible from every call site.

Three more closed the same round: `unk_83A90` (two `mEf::effect_c::follow` calls,
cross-checked against that class's own landed vtable dump), `executeState_Wait`,
and `executeState_DieFall`'s tail. `unk_83B00` was authored content-complete and
**size-exact at 84/84** using only already-landed inline helpers and named
base-class fields — no raw offsets.

**Three residuals remain, all evidenced:** `setupBgCtr` (5 differing, only a base
offset into a shared rodata pool — and the agent applied the "who references it"
ownership test **in the opposite direction** to show the extra leading bytes are
referenced by nothing in this unit, so the gap is link-order-dependent rather than
theirs); `unk_83B00` (55, a register-hoisting instruction-selection difference);
and `__sinit` (21, tracing to one 192-byte solid-zero `.data` gap it could not
attribute, with the obvious candidates ruled out rather than assumed).

## CROSS-CONFIRMED within the hour: the PMF encoding difference IS the virtual/non-virtual distinction

Two agents on unrelated units found the same phenomenon from opposite directions,
and together they close it.

**FLOOR_JR_A** found the encodings and what they mean:
```
{ -1, fn_addr, 0 }          -> NON-VIRTUAL pointer-to-member
{ vtable_offset, 0x60, 0 }  -> VIRTUAL pointer-to-member
```
and used it to prove its `DieFall` state needed **plain virtuals plus an ordinary
`STATE_DEFINE`**, not the virtual-state macro — taking `__sinit` 74 -> 21 with all
159 words in order.

**LEMMY_FOOTHOLD**, independently, observed that its two classes use **different
PMF encodings** — `daLemmyFootholdMain_c`'s states store direct relocatable
function addresses, while `daLemmyFoothold_c`'s store **vtable byte offsets with
no relocation at all.** It resolved the addresses by reading FOOTHOLD's own vtable
at those offsets, which worked, and **explicitly flagged that it could not explain
why the two classes differ** — noting both use the identical macro shape and
neither derives from the other.

**The explanation is FLOOR_JR_A's finding.** The encoding is not arbitrary: it
records whether the pointed-to method is virtual.

- `daLemmyFootholdMain_c` -> direct addresses -> its state methods are
  **NON-VIRTUAL** -> plain `STATE_FUNC_DECLARE` / `STATE_DEFINE`.
- `daLemmyFoothold_c` -> vtable offsets -> its state methods are **VIRTUAL** ->
  the virtual-state form.

**So "both use the identical macro shape" is the defect, not the puzzle** — one of
them should not.

**The general rule, now confirmed on two units:** before choosing a state macro,
**read the three PMF triples out of `.data` and let the encoding decide.** A
vtable offset where you wrote a non-virtual method — or a direct address where you
wrote a virtual one — is a declaration defect visible without compiling anything.
This question has now cost rounds on three separate units.

**LEMMY 30/51 -> 34/51**, with all 9 real state bodies attributed to exact target
addresses **by reading each state object's own PMF fields** rather than trusting
size-based pairing — the technique that also resolved another unit's three
"unattributed" functions. Seven of nine authored and matching, including
`mAnimTexSrt.play()` confirmed at vtable offset `0x14` **by a probe compile**
rather than by size match.

**Two bodies deliberately left unauthored rather than guessed**, both correctly
triaged by the size diagnostic: one read in full with three genuine unknowns
still open (an unnamed float field, an unnamed vtable-slot-`0xd4` method, an
untyped `.bss` singleton), the other not yet read at all.

## PRECONDITION on the hand-expansion technique: first check you are using the RIGHT macro

The PMF-encoding fix landed on LEMMY exactly as predicted — **`__sinit` 267
differing -> 35**, proportionally matching FLOOR_JR_A's 74 -> 21. And the
**function MATCH count held at 34/51 straight through the change**, which is
itself the proof that **the state bodies were never wrong — only the declaration
shape was.**

**But the important consequence is a correction to doctrine I recorded earlier
today.** Once `daLemmyFootholdMain_c`'s states switched to the non-virtual
`STATE_FUNC_DECLARE`/`STATE_DEFINE`, **they stopped emitting a `baseID_` helper at
all — so there was nothing left for the other class's `DemoWait` to collide with,
and the round-3 hand-expansion became unnecessary.** All three of the other
class's states now use the ordinary macro directly.

**So the `baseID_` collision was a SYMPTOM of the wrong macro, not an independent
problem.** The recorded fix — declare with the macro, hand-expand the duplicate
definition — is still correct where two classes genuinely both need the virtual
form. But it now carries a precondition:

**Before hand-expanding to dodge a `baseID_` collision, read the PMF encodings and
confirm BOTH classes actually need the virtual macro.** If either one's states use
the direct-address encoding, it should not be using the virtual form, and fixing
that removes the collision for free. **Reach for the workaround only after the
encoding says the collision is real.**

That ordering matters because the workaround is invisible once applied — it
compiles, it looks deliberate, and it silently preserves a wrong declaration.

### The attribution work paid off as a bug-catcher

`executeState_DemoDown`'s first draft **omitted `calcSpeedY()`/`posMove()`**, and a
diff against the **correctly attributed** target function exposed it immediately.
That is the payoff of reading each state object's PMF fields rather than trusting
size-based pairing: **you can only diff against the right function if you know
which one it is.** Both bodies now sit at 6/47 and 2/33, entirely the
pool-position and register-choice residual classes documented all session.

`+0x5b8` was identified as a genuine `mTargetPosY` field, **confirmed by two
independent call sites computing the same distance expression** — not by a single
usage.

**Tally 34/51, unchanged in raw count, and the honest framing the agent gave is
the right one:** the unit went from two unread bodies plus an unexplained 267-word
`__sinit` gap to two small well-understood residuals, a 35-word gap, and **zero
open declaration questions.** A tally cannot express that, which is exactly why
the uncertain surface is worth reporting separately.

## A "differing" count means NOTHING for a function you have not authored

LEMMY's survey classified all 17 unmatched functions and produced a clarification
about the tool that affects how everyone reads its output:

- **2 are genuine residuals** — authored, size-exact, diffs of 2 and 6, already
  parked.
- **15 have NO SOURCE AT ALL.** For those, **`verify_anon`'s "differing" count is
  its fallback pairing against the nearest unrelated AUTHORED function — not a
  diff of anything.** The number is noise.

**So a per-function "N differing" is only meaningful once you have written that
function.** Reading a large count on an unauthored function as "this one is hard"
is reading the tool's closest-remaining-candidate heuristic as a measurement. This
is the same `~name` caveat the tool prints for its guesses, one level up: **the
tilde marks the NAME as a guess, and the COUNT beside it is equally a guess.**

**The practical consequence: a unit's tally splits into three populations, not
two** — matched, authored-with-residual, and unwritten — and only the middle one
has meaningful diff counts. Report them separately; "17 unmatched" hid the fact
that 15 of them had never been attempted.

### The boundary correction: my upper bound was a CEILING, not an edge

I gave LEMMY `0x286D4` as the point where foreign strings begin. **The true `.data`
edge is `0x284F0`** — the agent read past its own ownership-check cap by hand and
found this unit's last string ending at `0x284B7`, then padding, then genuinely
foreign content (a profile struct whose `executeOrder`/`drawOrder` sit far outside
this unit's tight range, plus another unit's `.brres` strings).

**An upper LIMIT derived from "where foreign content starts appearing" is not the
same as the boundary**, and the gap between them was `0x1E4` bytes. Both ends
hand-confirmed against raw REL bytes is the standard the RIVER agent set, and it
is the right one.

`.rodata` derived as `0x4A80-0x4AAC` and **flagged explicitly as tentative** —
three of the fifteen unwritten functions reference `lbl_2_rodata_4A80` and have not
been read, so the bound needs rechecking once they are authored. **Marking a
derived range as provisional because of what has not been read yet is the right
habit**; a slice range asserted from an incomplete draft is exactly how units get
mis-scoped.
