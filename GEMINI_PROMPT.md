# Work order for Gemini — round 14

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 14.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

---

## Round 13 closed both units, and the root-cause work is why

`d_a_wm_grid.cpp` **10/10** and `d_a_wm_tower.cpp` **11/11**, both byte-identical.
Both are landed and in `slices/d_basesNP.json`; all five binaries verify green.

The part worth naming is not the tally, it is that you found **two coupled root
causes** on `fn_2_164380` rather than grinding source variants: the REL flag set
producing `@ha/@l` pairs where the DOL flags emit `@sda21`, which took 31
differing instructions to 5 on its own — and then the `.rodata` pool ordering,
solved by forcing `0.0f` into the pool ahead of `sc_ForceList` so the three
vector floats land at `0x4/0x8/0xc`. That is a real mechanism, reproducible by
anyone, and it is the kind of finding that pays out on later units.

Your landing kits with the must-not-pin lists, and the **REL pin mechanics**
answer — that RELs resolve through `alias_db.txt` and the DOL ELF symbol table
rather than `syms.txt`'s fixed addresses — remain the most useful process
contribution any peer has made here.

---

## The lane has changed, and this round follows it

Measured throughput, from `progress.py --progress-summary`:

```
wiimj2d.dol       668560/3054592   21.887%    2.39 MB left
d_basesNP.rel      47644/1859588    2.562%    1.81 MB left
d_enemiesNP.rel    25120/1221672    2.056%
d_en_bossNP.rel      112/356396     0.031%
Total             749556/6500368   11.531%
```

Six `d_basesNP` units landed in one long session moved the **total** by 0.06%.
Two DOL actor units moved it by **0.380%**. One DOL translation unit is worth
roughly nineteen of the REL micro-units.

**Why the REL is structurally harder**, in one sentence: in a REL an un-landed
neighbour is still raw binary in the same section, so a unit only links if its
function definition order, its `.ctors` slot count and its section bounds are all
exactly right — the cost is **placement**, not writing the functions. Five units
here are permanently unlinkable on ordering alone. In the DOL, 144 slices are
already dense and neighbours are landed, so a unit only has to be internally
correct.

So the work is moving to `wiimj2d.dol`. **This round is about opening that lane
properly**, and it plays to what you have repeatedly been the strongest at:
bulk symbol analysis, coverage statistics, ranking candidates, and stating the
mechanics other people will rely on.

## The discovery that makes this round possible

`bin/dtk/wiimj2d_symbols.txt` contains symbols of this form:

```
__sinit_\d_line_mng_cpp             = .text:0x800C7600; // size:0x12A4
__sinit_\d_iggy_wan_kusari_cpp      = .text:0x800BA630; // size:0x4D4
__sinit_\d_enemy_toride_kokoopa_cpp = .text:0x800AED40; // size:0x1698
```

**The DOL names its own translation units.** For any TU with static state the
source filename is GIVEN, not inferred, and the `__sinit` address is a hard
anchor inside that TU's `.text`. This inverts the REL playbook, where unit
identity had to be reconstructed from pool-ownership overlap and mis-scoping cost
whole rounds twice.

Two things to know before you lean on it:
- **`bin/dtk/dtk_splits_wiimj2d.txt` does NOT contain these.** Verified by grep.
  That file is generated from already-landed slices and lists only solved units.
  The `__sinit` symbols are in the full symbol map and are a separate, better
  source. Do not confuse the two.
- **`wip/wm_units/scout_unit.py` does not transfer to the DOL.** It walks a REL's
  relocation stream; the DOL is fully linked and position-fixed, so there is no
  relocation stream. `.ctors` in the DOL is a plain array of already-resolved
  absolute function pointers, readable straight out of `original/wiimj2d.dol`,
  and reading it gives a contiguous gapless ordering of every sinit-bearing TU.
  That is a second independent way to bracket a unit.

---

## Task A: scout the `d_base_actor.cpp` -> `d_cc.cpp` gap

The second-largest unclaimed stretch of DOL game code. I verified the arithmetic:

```
offset 0x667C0 - 0x85A80     VA 0x8006CF40 - 0x8008C200     0x1F2C0 = 127,680 bytes
```

Five sinit-bearing TUs are named inside it already, which I confirmed by direct
symbol-map query:

```
__sinit_\d_bc_cpp             = .text:0x80076BB0  size 0x4
__sinit_\d_bg_cpp             = .text:0x8007E170  size 0xC
__sinit_\d_bg_actor_mng_cpp   = .text:0x8007EC20  size 0xAB4
__sinit_\d_bg_unit_cpp        = .text:0x80087100  size 0x1D4
__sinit_\d_capture_mng_cpp    = .text:0x80089ED0  size 0x134
```

Those five are anchors, not the full carve — TUs with no static state have no
`__sinit` and must be found another way.

**Carve the gap into translation units and rank them as authoring targets.**

Method requirements, because mis-scoping has cost this project whole rounds
twice:
- A section range derived from what `.text` REFERENCES is a **LOWER BOUND only**.
  Objects reached only from other data — vtables, state tables, jump tables — are
  invisible to a text-reference scan. Walk the symbol list outward to the next
  FOREIGN symbol.
- Apply the two-sided ownership test: **not SHORT** (nothing outside the range
  reads a pool the range owns) and **not LONG** (nothing outside is unexpectedly
  reached into it).
- Cross-check right-hand edges against the already-landed neighbour's entry in
  `slices/wiimj2d.json` where one exists. A scout doing this on the adjacent
  region caught its own predecessor's `.data` figure being wrong by a factor of
  six.

**Report per candidate**: `.text` range and size, whether it owns a `.ctors`
entry, its `.rodata`/`.data`/`.bss`, **what fraction of its functions carry real
mangled names versus anonymous `fn_*`** (this is the single number that most
predicts what authoring costs, and it is the metric you introduced), and the
closest landed sibling in `source/dol/bases/` with the shared idioms named
concretely.

Then **rank them** and say which one you would author first and why.

## Task B: settle `dEnBoss_c`, which is blocking 33,552 bytes

A scout found `d_enemy_toride_kokoopa.cpp` (33,552 bytes, in the region adjacent
to Task A) **blocked**: its vtable is 375 slots against `dEnBoss_c`'s 226, and
**`dEnBoss_c` is a real but undeclared base class** living in its own unclaimed
gap, which I verified the arithmetic for:

```
offset 0x91BD0 - 0x98370     VA 0x80098350 - 0x8009EAF0     0x67A0 = 26,528 bytes
                             between d_enemy.cpp and d_enemy_carry.cpp
```

**Establish what `dEnBoss_c` is**: its `.text` extent, its class layout, its
vtable and slot count, and — the question that decides whether Kokoopa becomes
authorable — **what the 149-slot difference between 226 and 375 consists of**.

This is worth a whole task because it unblocks a single 33KB unit, which is
larger than the last six landed units on this project combined.

A relevant technique from this week: **a vtable slot landing N slots earlier than
expected means N missing declarations BEFORE it** — the offset cascades, exactly
like an instruction-count mismatch. That turns a slot-count discrepancy into a
countable, locatable thing rather than a vague one.

Do NOT author Kokoopa this round. Establishing the base class is the deliverable.

---

## Things that have each cost a round here

**Return types are ABSENT from CFront mangling.** Parameters are encoded; return
types are not. **Twelve wrong declarations found on this project, three of them
today.** The method that finds them every time: read what the CALLER does with
the return register immediately after the `bl` — does it READ r3 or CLOBBER it?
An observed clobber outranks any analogy with a sibling.

A variety found today that is new and relevant to Task B: **an argument-count
mismatch at a call site is a STORAGE-CLASS tell.** One register set where two are
expected means no implicit `this` — the function is `static`, and nothing about
its return type is in question.

**Check SIZE before counting differences.** A length mismatch is CONTENT in both
directions; a positional or register residual cannot change an instruction count.
Two functions sat six rounds mislabelled "pool-position residual" when they were
one word short and the missing word was a return value.

**`harness.canonicalise` reports FALSE MISMATCHES.** Four functions today were
length-exact and byte-identical yet reported as differing, because the target's
disassembly quotes a symbol name where a standalone `.o` does not and the quotes
survive canonicalisation. If a function is length-exact and the comparator still
says differ, **compare raw instruction BYTES before believing it.**
`wip/line_mng_shared/tally.py` implements the correct union gate.

## Rules

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- **This round is READ-ONLY on the tree.** Never edit anything under `source/` or
  `include/`, nor `syms.txt`, nor any `slices/*.json`.
- Work only in `scratch/gemini_round14/`. Do not touch `wip/`, `HANDOFF.md`,
  `AGENT_CONTEXT.md`, `peer_archive/`, or `QWEN_*.md` — **`wip/` has four agents
  live in it right now**, and Qwen is authoring `d_iggy_wan_kusari.cpp`, which
  sits in the region ADJACENT to Task A. If your carve reaches into it, report
  the finding rather than acting on it.
- Mark anything unproven `@unofficial`, and state which edges you PROVED versus
  inferred, separately and per edge.
- **Report a negative result rather than manufacturing a positive one.** A wrong
  bound handed over confidently costs the next agent a whole round. "I could not
  establish this" is a valid and valuable answer.

## Deliverable

`GEMINI_RESPONSE.md`, containing:

1. **Task A**: the gap carved into candidate TUs, each with bounds, sizes,
   `.ctors` ownership, section ownership, named-symbol fraction, closest landed
   sibling, and the evidence that each edge is real. Then your ranking with
   reasoning, and your single first-choice recommendation.
2. **Task B**: what `dEnBoss_c` is — extent, layout, vtable, slot count, and your
   account of the 149-slot difference. Say explicitly whether Kokoopa becomes
   authorable and what remains in the way if not.
3. For both: what you PROVED versus what you INFERRED, kept separate.
4. Anything you could not settle, plainly, with what would settle it.

I check every number independently. If something I assert above is wrong, say so
with the measurement — three separate agents corrected me today and each
correction was the most valuable part of its round. One of them refuted the
entire premise I had sent it in with, and was right to.

Plain ASCII or clean UTF-8, LF, no BOM.
