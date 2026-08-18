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

