# LEVERS.md -- new source-to-codegen correspondences mined from the matched corpus

Round type: corpus mining, not experiment. Every number below is measured against
already-landed, byte-exact objects and their retail-derived sources. No draft was
compiled; per `AGENT_CONTEXT.md` section 5, a matching function is stronger evidence than
an A/B compile, and the point of this round was to use that.

Everything here was grepped against `AGENT_CONTEXT.md` (levers 1-13, the
proven-negatives list) and `HANDOFF_INDEX.md` / `HANDOFF.md` before being written
up. Prior art is cited where it exists, and two sections below **correct or bound
an existing catalogue entry** rather than adding a new one.

---

## 1. The corpus, and how it was built

`scratch/lever_mining/build_index.py` walks all five slice files
(`slices/wiimj2d.json` plus the four REL slice files), and for each slice entry
pairs

    source: <rel>.cpp   ->   bin/compiled/<module>/<rel>.o   ->   source/<rel>.cpp

then disassembles the object with an absolute path to
`bin\dtk-windows-x86_64.exe elf disasm`.

| | |
|---|---|
| slice entries across the five modules | 182 |
| **paired, disassembled units (the corpus)** | **167** |
| unpaired | 15, all `lib/revolution/*.c` -- banked C units with no object under `bin/compiled/` |
| functions parsed | 6,319 |
| instructions | 185,723 |
| functions resolvable to an out-of-line body in the `.cpp` | 4,046 |

The 2,273 unresolvable functions are overwhelmingly header-defined weak symbols
(`~mVec3_c`, `nw4r::lyt::*` destructors, template instantiations) that have no
body in the `.cpp` at all -- expected, and they are simply excluded from every
source-correlation figure below.

The catalogue's recurring figure of "145 landed byte-exact objects" is now stale;
the true corpus is **167**.

Tooling, all in `scratch/lever_mining/`:

| file | purpose |
|---|---|
| `build_index.py` | build `index.json`, disassemble into `disasm/` |
| `corpus.py` | parse a dtk listing into functions, instructions and labels |
| `srcmap.py` | comment-stripping C++ body extractor; maps a demangled name to its `.cpp` body |
| `sweep_cmp.py`, `sweep_cmp2.py` | sweep 1 |
| `sweep_loops*.py`, `sweep_unroll.py` | sweep 2 |
| `sweep_switch*.py`, `sweep_ladder.py` | sweep 3 |

---

## 2. Sweep 1 -- comparison spelling and branch shape (lever 7, extended)

### 2.1 The float rule, restated mechanically

Lever 7 currently says `!(a < b)` emits `bge`/`ble` while `a >= b` emits
`cror`-combined branches, and tells you to "try both phrasings". Measured against
the whole corpus, the rule is **not about negation at all** -- it is about which
operator you wrote, and it is deterministic, so there is nothing to try:

> **A float `>=` or `<=` ALWAYS emits a `cror`. A float `<`, `>`, `==` or `!=`
> NEVER does.**

The mechanism is IEEE, not a compiler preference. `fcmpo` sets exactly one of
LT/GT/EQ/UN. `a < b` is the single bit LT, so one branch suffices. `a >= b` is
`GT | EQ` -- which is *not* the same as `!(a < b)` (they differ on NaN), so MWCC
must OR two condition bits before branching. That is the `cror`.

Counts:

| | |
|---|---|
| float compare sites (`fcmpo` 841 + `fcmpu` 70) | **911** |
| of which `cror`-combined | **201** |
| functions with a float compare AND a resolvable source body | 364 |
| **clean 1:1 cases** (exactly one float compare, zero integer compares, exactly one relational operator in the body) | **44** |
| clean cases where the rule holds | **44 / 44** |
| clean cases where a source `>=`/`<=` produced a PLAIN branch | **0** |

The full 364-function aggregate produces exactly two classes of apparent
exception, and both dissolve on inspection:

- **6 functions emit a `cror` with no `>=`/`<=` in their own body.** All six are
  inlining: `mVec3_c::normalize` and `normalizeRS` (`m_vec.cpp:17,26`) inline
  `isZero()`, whose body is `std::fabs(val) <= FLT_EPSILON` at `m_vec.cpp:14`;
  `dMdActor_c::preDraw` and `dWmActor_c::preDraw` inline `mClipSphere.isZero()`;
  `cM::atan2s` (`c_math.cpp:167`) inlines `atan2i`; `mMtx_c::toRot`.
- **11 functions have a `>=`/`<=` in the body and emit no `cror`.** Every one is
  an **integer** comparison -- `mPopupType >= POPUP_TYPE_COIN_2`
  (`d_SmallScore.cpp:303`), `angle.y >= 0` (`d_a_en_shell.cpp:95`),
  `getTallType(-1) <= 2` (`d_a_player.cpp:840`). See 2.2.

### 2.2 The integer half, which the lever does not cover at all -- a hard negative

> **`cror` never appears on an integer comparison. 0 occurrences in 8,606 integer
> compare sites.**

`cmpw`/`cmpwi`/`cmplw`/`cmplwi` are two's-complement trichotomies with no
unordered state, so `>=` is the single bit `!LT` and needs no combine. The
consequence for authoring is blunt: **the `!(a < b)` versus `a >= b` rewrite is a
no-op on integers.** `a >= b`, `!(a < b)` and `b <= a` all emit the same `bge`
family branch. If an integer branch's polarity is wrong, the lever to reach for is
lever 5 (settle polarity from the target's bytes) or swapping which block is the
"then" -- not the negation rewrite.

### 2.3 The `cror` operand names the operator -- read it, do not guess

Only **two** `cror` operand forms occur anywhere in 185,723 instructions:

| target emits | source operator | corpus count |
|---|---|---|
| `cror eq, gt, eq` | first `fcmpo` operand is on the `>=` side | 116 |
| `cror eq, lt, eq` | first `fcmpo` operand is on the `<=` side | 98 |

14/14 of the clean 1:1 `cror` cases obey this. `HANDOFF.md:5800` already records
one row of this table (`cror eq,lt,eq; beq` -> `if (x <= 0.0f) return;`) from a
single unit; this is that row generalised and counted.

### 2.4 Citations -- three comparisons in ONE matched file

`source/dol/bases/d_a_en_blockmain.cpp`, object
`bin/compiled/wiimj2d/dol/bases/d_a_en_blockmain.o`:

| source | line | emitted |
|---|---|---|
| `return self->mSpeed.y > 0.0f;` | 286 | `+0x8E8 fcmpo cr0, f1, f0` -- **no cror** |
| `if (mSpeed.y <= -2.0f)` | 1160 | `+0x2728 fcmpo` / `cror eq, lt, eq` / `bne` |
| `if (mSpeed.y >= 2.0f)` | 1184 | `+0x2808 fcmpo` / `cror eq, gt, eq` / `bne` |

Second citation, `source/d_basesNP/bases/d_awa.cpp` (also a single file carrying
both halves):

| source | line | emitted |
|---|---|---|
| `if (mSpeed.y > 2.0f)` | 93 | `+0x55C fcmpo cr0, f1, f0` / `blelr` |
| `if (mLifeTimer == 0 \|\| mPos.y >= dBg_c::m_bg_p->getLiquidHeight())` | 45 | `+0x2D0 fcmpo` / `cror eq, gt, eq` / `bne` |

### 2.5 Boundary conditions

1. **Float only.** Integer comparisons never produce a `cror` (2.2). Check the
   compare mnemonic before applying anything in this section.
2. **The rule survives inlining, so read the callee too.** Six of the corpus's
   `cror` sites have no relational operator in the caller's own source. If the
   target has a `cror` and your body has no `>=`/`<=`, look at what you are
   inlining before changing the body.
3. **`cror` identifies the OPERATOR, not the branch polarity.** `cror`+`bne` and
   `cror`+`beq` both occur (178 and 19). Which one you get is decided by whether
   the guarded block is the "then" or the "else" -- that is lever 5's territory
   and this rule says nothing about it.
4. **A float `==`/`!=` is single-bit and gets a plain `beq`/`bne`** (43 and 33
   sites). Do not expect a `cror` from an equality test.
5. This governs the branch *shape*. It says nothing about operand slots inside
   `fcmpu`/`fcmpo`, which is a separate and already-documented axis (see the
   "`fcmpu` operand order IS addressable" section of `AGENT_CONTEXT.md`).

---

## 3. Sweep 2 -- loop shapes (the catalogue's largest gap)

`bdnz` appears **nowhere** in `AGENT_CONTEXT.md` or `HANDOFF.md`. This is new ground.

367 back-edges in 288 functions, classified by the branch that closes the loop:

| emitted shape | count |
|---|---|
| compare-at-bottom (conditional back-edge) | 279 |
| `bdnz` (CTR-counted) | 75 |
| top-test (unconditional back-edge, test at the loop head) | 13 |

### 3.1 RULE L1 -- `bdnz` proves the loop body contains NO call. Zero exceptions.

| | back-edges | of which `bdnz` |
|---|---|---|
| loop body contains a `bl`, `bctrl` or `bctr` | 257 | **0** |
| loop body is call-free | 110 | 75 |

CTR is volatile across a call and MWCC will not risk it, so a single call
anywhere in the body demotes the loop to a compare-at-bottom.

**Why this is actionable, and it cuts both ways.** If the target has a `bdnz` and
your draft has a compare-at-bottom, you have a call in the body that the original
did not -- a helper the original inlined, or the wrong overload. If the target has
a compare-at-bottom and your draft emits `bdnz`, you have inlined away a call the
original really made. Either way the defect is *the call*, not the loop spelling,
and no amount of `for`/`while` rewriting will move it.

Citation pair, both in `source/dol/bases/d_a_en_bigpile.cpp`, same class, same
bound (`MAX_PILE_COUNT == 12`), differing only in whether the body calls:

```
d_a_en_bigpile.cpp:56  BigPileMng_c::remove()   body: mpPiles[i] = nullptr; mCount--;
  +0x80  li r0, 0x3 ; mtctr r0 ... +0x130 bdnz          <- call-free -> bdnz

d_a_en_bigpile.cpp:81  BigPileMng_c::move()     body: mpPiles[i]->move();
  +0x208 bl move__13daEnBigPile_cFv
  +0x214 cmpwi r30, 0xc ; +0x218 blt .L_000001FC        <- one call -> compare-at-bottom
```

### 3.2 RULE L2 -- `for` versus `while` is codegen-NEUTRAL. A clean negative.

Among 210 single-loop functions with an unambiguous source keyword:

| source | emitted |
|---|---|
| `for` (135 of the non-`bdnz` cases) | compare-at-bottom |
| `while` (41) | compare-at-bottom |

Both produce the identical rotated shape. **Rewriting a `for` as a `while` (or
back) to chase a loop residual is a wasted round.** What actually varies is
covered by L1 and L3, neither of which is reachable from the keyword.

### 3.3 RULE L3 -- the loop's ENTRY tells you whether the bound is a compile-time constant

Three entry shapes exist, and they are mutually exclusive:

| what precedes the loop | what it proves about the source |
|---|---|
| nothing -- the body starts immediately | the trip count is a **compile-time constant** (literal or named constant) |
| an unconditional `b` forward into the middle of the loop | the bound is a **runtime value** and the loop is compare-at-bottom |
| `cmplwi rN, 0` / `ble <after the loop>` right after the `mtctr` | the bound is a **runtime value** and the loop is `bdnz` |

Measured on `for` loops with a resolvable induction variable:

| bound form | entry jump present | absent |
|---|---|---|
| literal or named constant, starting from 0 | **0** | **84** |
| runtime value | 58 | 17 (of which 15 are the `bdnz` guard form) |

**Zero exceptions in the constant direction.** The two apparent exceptions in the
runtime direction are not: `dWmEnPath_c::GetPathPointNo` uses
`ARRAY_SIZE(mPoints)`, which is a compile-time constant my classifier binned
wrongly, and `TagProcessor_c::preProcess` is `for (;;)`.

Citations:

```
d_a_en_bigpile.cpp:36  for (int i = 0; i < MAX_PILE_COUNT; i++)   <- constant bound
  +0x24  li r0, 0xc ; +0x30 mtctr r0 ; +0x38 <body>               <- no guard, no entry jump

d_cd.cpp:69            for (u32 i = 0; i < mScrollDataCount; i++) <- runtime bound
  +0x32C mtctr r0
  +0x330 cmplwi r0, 0x0 ; +0x334 ble .L_00000350                  <- zero-trip guard
  +0x34C bdnz .L_00000338
```

And `do`/`while` at the other end: **5/5 `do { } while ()` loops in the corpus
have no entry jump**, which is the positive form of the already-recorded finding
at `HANDOFF.md:9431` ("an outer guard plus `while` reproduces a redundant jump
that `do/while` does not"). That entry is confirmed and now has a corpus count
behind it.

### 3.4 What decides `bdnz` among CALL-FREE loops (the second-order rule)

Of the 110 call-free back-edges, 75 are `bdnz` and 30 are compare-at-bottom.
Restricting to the 30 unambiguous single-`for` call-free cases, the split is
24 `bdnz` / 6 compare-at-bottom, and all 6 are accounted for:

- **3 are not counted loops at all** -- `for (...; curr != nullptr; ...)` pointer
  chases (`f_list.cpp:48`, `f_list.cpp:62`, `nw4hbm/ut/list.cpp:75`). No trip
  count exists to hoist. (This is consistent with the separately-recorded
  `HANDOFF.md:13729` finding that `!=` in a loop condition defeats counted-loop
  conversion.)
- **3 have a MEMBER-LOAD bound and store to memory inside the body**:
  `d_a_en_snake_block.cpp:451, 645, 687`, all
  `for (int i = 0; i < mBlockCount; i++) { mBlocks[i]... = ...; }`. MWCC cannot
  prove the store does not alias `mBlockCount`, so the bound is not loop-invariant
  and cannot be hoisted into CTR.

The contrast that proves it is the aliasing and not the member load: the six
`d_cd.cpp` accessors (`getScrollDataP` etc.), `d_rail.cpp:4`, `d_next.cpp:82` and
`d_res.cpp:206` all have a member-load bound (`mScrollDataCount`, `mRailCount`,
`mNumArcs`) and all get `bdnz` -- because their bodies only **read**.
`TagProcessor_c::TagProcessor_c()` (`d_tag_processor.cpp:22`) stores in the body
and still gets `bdnz`, because its bound `&mScissor[4]` is address arithmetic, not
a load.

So: **member-load bound + a memory store in the body = no `bdnz`.** Constant
bound, or address-arithmetic bound, or a read-only body = `bdnz`.

### 3.5 Unrolling -- characterised, low-n, NOT yet a rule

The corpus contains genuinely unrolled `bdnz` loops, and the discriminator looks
like an early exit, but only one clean A/B pair exists so this is reported as an
observation:

```
d_a_en_bigpile.cpp:36  entry()   for i < 12, body contains `return true`
     li r0, 0xc  -> CTR = 12, ONE body instantiation

d_a_en_bigpile.cpp:56  remove()  for i < 12, body has no early exit
     li r0, 0x3  -> CTR = 3, FOUR body instantiations
                    (offsets 0xc / 0x10 / 0x14 / 0x18, then `addi r7, r7, 0x10`)
```

Same file, same class, same constant bound, same array. **Read the `mtctr` seed
against the source bound: seed == bound means no unroll; seed == bound/4 means
MWCC unrolled by 4.** If your draft's seed is 4x the target's, look for an early
`return`/`break` you have added; if it is 1/4 of the target's, look for one you
have dropped. n = 1 clean pair; do not treat the ratio as guaranteed to be 4.

### 3.6 Boundary conditions for the whole loop section

1. L1 counts calls in the **emitted** body. A call the original inlined is not a
   call for this purpose -- that is exactly why it is a useful diagnostic.
2. L3's "no entry jump" case requires the bound to be constant **and** the
   induction variable to start at a constant. 82 of the 84 confirmations start at
   0; the other 2 start at another constant and behave identically.
3. L3 says nothing about *nested* loops; every figure above is from single-loop
   functions, where the back-edge attribution is unambiguous.
4. None of this reaches instruction *scheduling* inside the body. Consistent with
   the "LIMIT of the declaration-order rule" section already in `AGENT_CONTEXT.md`:
   these rules pick the loop's control-flow skeleton, nothing more.

---

## 4. Sweep 3 -- `switch` versus `if`/`else if` dispatch (lever 1, given a predictor)

Lever 1 says the `switch` rewrite "does not always transfer" and to measure rather
than assume. Three measurements below turn parts of that into something readable
off the target.

### 4.1 A `bctr` is almost never a switch -- 280 of 291 are virtual tail calls

| | |
|---|---|
| `bctr` instructions in the corpus | **291** |
| **real jump tables** | **11** |
| virtual tail calls | 280 |

The two shapes are unmistakable and never overlap:

```
virtual tail call (280x)      lwz  r12, 0x0(r3)
                              lwz  r12, 0x3c(r12)
                              mtctr r12
                              bctr

jump table (11x, uniform)     cmplwi rX, N          <- N is the case-value SPAN
                              bgt    <default>
                              lis    rY, tbl@ha
                              slwi   rZ, rX, 2
                              addi   rY, rY, tbl@l
                              lwzx   r0, rY, rZ
                              mtctr  r0
                              bctr
```

There are **no `jumptable_*` labels anywhere** in the corpus, so an agent looking
for one will conclude there is no table when there is. Match on the
`cmplwi`/`bgt`/`lwzx` prologue instead.

Corollary worth having: **the jump table's own `cmplwi` immediate is the case-value
span.** `cmplwi rX, 0x13` means the original switch's case values run 0..19 with
20 table slots; `bgt` goes to `default`. That reads the case range straight off the
target with no guesswork.

### 4.2 A `switch` with 7 or fewer case labels NEVER becomes a jump table

140 corpus functions contain exactly one `switch` (unambiguous attribution):

| case labels | jump table | ladder |
|---|---|---|
| 2..7 | **0** | **127** |
| 8 | 1 | 2 |
| >= 9 | 9 | 1 |

So:

- **Hard**: fewer than 8 case labels, no table, 127/127. A small `switch` and a
  small `if`/`else if` chain are indistinguishable on this axis, which is
  precisely why lever 1's rewrite is worth trying on small dispatches at all.
- **Hard in the other direction**: if the target HAS a jump table, the original
  `switch` had at least 8 case labels. That is a floor on how many cases you must
  find before the shape can possibly match.
- **Soft, 8-12 labels**: mixed, and the discriminator is the number of distinct
  case **bodies**, not labels. The two ladder cases at >= 8 labels both collapse:
  `daWmGhost_c::initState` (`d_a_wm_ghost.cpp:105`) has 8 labels but only 4 bodies
  (`case 2,3,4,5: goto openGhost;` and `case 0,8:` share targets);
  `dNext_c::searchNextNum` (`d_next.cpp:82`) has 9 labels and 4 bodies. The
  8-label case that DID get a table, `dAcPy_c::executeState_Cloud`
  (`d_a_player.cpp:6659`), has near-one-body-per-label over the dense range 0..8.
  A third ladder, `dAcPy_c::ccCheckAttack` (`d_a_player.cpp:9504`), has 8 labels
  and 8 bodies but its `CC_ATTACK_*` values span roughly 4..23 -- too sparse.

### 4.3 CORRECTION AND BOUND on the existing `switch` diagnostic (`HANDOFF.md:8148`)

The catalogue records the disassembly signature as:

```
if / else if chain :  cmpwi rX, A ; bne <next check>
switch             :  cmpwi rX, A ; beq <case A>
```

Measured across the corpus, **this is only reliable in its strict form, and the
loose form is a coin flip.**

| ladder detection | `beq` ladders from a `switch` | precision |
|---|---|---|
| compares of one register, each with its own branch within 4 instructions | 98 of 184 | **53%** |
| **`cmpwi`/`beq` pairs with NOTHING between them, >= 3 in a row** | **99 of 104** | **95%** |

And the `bne` half: in the strict adjacent form there are **zero** `bne` chains in
the whole corpus, because an `if`/`else if` chain necessarily has the previous
block's code sitting between its compares. So the strict signature is one-sided:
adjacency proves `switch`, non-adjacency proves nothing.

**Four of the five strict exceptions have one shared cause, checked in the
source:** a chain of equality tests against the same value written as a **single
condition** also compiles to a perfectly adjacent `beq` ladder. `dWmActor_c::preExecute`
(`d_wm_actor.cpp:38`) and `preDraw` (`d_wm_actor.cpp:66`) both hold
`mProfName != A && mProfName != B && mProfName != C` -- De Morgan's dual of an
`||`-chain of `==`, and MWCC emits the same shape. `dCsvData_c::ReadOpenRouteName`
(`d_wm_csvdata.cpp:439, 446`) does the same on characters
(`line[pos] != ',' && line[pos] != '"'`). The fifth,
`daPyDemoMng_c::startControlDemoLandPlayer` (`d_a_player_demo_manager.cpp:1078`),
I did not individually account for -- reported as unexplained rather than assumed
to be the same cause.

> **Restated rule: an adjacent `cmpwi`/`beq` ladder of 3 or more means the source
> is EITHER a `switch` OR one chain of equality tests against the same value in a
> single condition (`x == A || x == B` or its `!=`/`&&` dual). Those are the two
> known producers; one corpus occurrence out of 104 is unexplained. It does NOT
> mean "not an if/else if chain" in the loose case, where the shape is 53%
> informative and should not be used to pick a construct.**

This matters because the catalogue presents the diagnostic as decisive ("read the
branch shape before choosing the construct"). It is decisive only when the
compares are literally adjacent.

---

## 5. Bonus negatives, cheap and worth recording

- **`fsel` occurs ZERO times in 185,723 instructions.** Also zero `subfe`, zero
  `isel`, zero `sraw`, zero `bdz`. Whatever branchless-select shape a residual
  has, it is not `fsel`, and the flags this project uses do not produce it.
  Lever 2's "ternary between adjacent small constants compiles to arithmetic" is
  therefore an *integer* phenomenon exclusively; there is no floating-point
  analogue in this compiler configuration.
- The branchless integer idiom the corpus does have is `cntlzw`+`srwi`
  (102 occurrences). Its two dominant continuations are `cntlzw; srwi; blr`
  (a `bool`-returning leaf -- lever 8's case, e.g.
  `daEnKuriboBase_c::isOnTrampoline`, `d_a_en_kuribo_base.cpp:321`) and
  `cntlzw; srwi.; beq` (25 occurrences: a `bool`-returning helper that got
  inlined and whose result is then branched on). The second shape is worth
  flagging as **a possible tell that the original called a `bool` inline rather
  than testing in place**, but I did not verify enough of the 25 to state it as a
  rule, so it is listed here as an open lead rather than a lever.
- **`addze` occurs 8 times, `subfic` 31** -- too few to sweep. If a residual is one
  of these, the corpus can name every site that produces it; ask for the list
  rather than experimenting.

---

## 6. Proposed additions to `AGENT_CONTEXT.md`

Written in the catalogue's voice, for direct paste. Items A and D are
*corrections/bounds on existing entries*, not new levers, and should replace or
annotate what is there rather than being appended.

---

**A. CORRECTION AND SHARPENING OF LEVER 7 -- it is the OPERATOR, not the negation,
and it is FLOAT-ONLY.**

Lever 7 currently reads "`!(a < b)` and `a >= b` are NOT the same to MWCC ... try
both phrasings whenever a float comparison's branch shape is wrong." There is
nothing to try -- the mapping is deterministic, and the reason is IEEE rather than
compiler taste. `fcmpo` sets exactly one of LT/GT/EQ/UN, so a single-bit test
(`<`, `>`, `==`, `!=`) needs one branch, while `>=` (`GT|EQ`) and `<=` (`LT|EQ`)
must OR two condition bits first. Measured over the whole matched corpus:

- **911 float compare sites, 201 `cror`-combined.** Of 44 functions with exactly
  one float compare, zero integer compares and exactly one relational operator in
  the source, **44/44** obey `>=`/`<=` -> `cror` and `<`/`>`/`==`/`!=` -> plain.
  There is not one counterexample.
- **The `cror` operand names the operator.** Only two forms exist anywhere:
  `cror eq, gt, eq` = the first `fcmpo` operand is on the `>=` side (116 sites);
  `cror eq, lt, eq` = it is on the `<=` side (98 sites). Read it off the listing
  instead of trying phrasings.
- **This is FLOAT ONLY. `cror` appears on ZERO of 8,606 integer compare sites.**
  On integers `a >= b`, `!(a < b)` and `b <= a` are byte-identical. Do not spend a
  round on the negation rewrite when the compare is `cmpw`/`cmpwi`/`cmplw`/`cmplwi`
  -- for an integer polarity problem the lever is 5, or swapping which block is the
  "then".

Three comparisons in one matched file make the A/B: `d_a_en_blockmain.cpp:286`
`return self->mSpeed.y > 0.0f;` -> bare `fcmpo`; `:1160` `if (mSpeed.y <= -2.0f)`
-> `cror eq, lt, eq`; `:1184` `if (mSpeed.y >= 2.0f)` -> `cror eq, gt, eq`. Second
pair in `d_awa.cpp:93` (`> 2.0f`, `blelr`) against `d_awa.cpp:45` (`>= ...`,
`cror eq, gt, eq`).

Boundary: the rule survives inlining, so six corpus functions carry a `cror` whose
`>=` lives in an inlined callee (`mVec3_c::normalize` -> `isZero()` ->
`std::fabs(val) <= FLT_EPSILON`, `m_vec.cpp:14`). If the target has a `cror` and
your body has no `>=`/`<=`, look at what you are inlining before touching the
body. And the `cror` fixes the OPERATOR, never the polarity -- `cror`+`bne` (178)
and `cror`+`beq` (19) both occur.

---

**B. NEW: `bdnz` PROVES THE ORIGINAL LOOP BODY HAS NO CALL. Zero exceptions in 367
loops.** Nothing in this file covered loops at all before this.

Across all 367 back-edges in the 167 matched objects: **257 loops whose body
contains a `bl`/`bctrl`/`bctr` -- not one is `bdnz`. All 75 `bdnz` loops are
call-free.** CTR is volatile across a call, so one call anywhere in the body
demotes the loop to a compare-at-bottom.

The diagnostic cuts both ways and names the defect precisely:

- target has `bdnz`, draft has a compare-at-bottom -> **your body calls something
  the original inlined** (wrong overload, or an out-of-line member where retail
  has an in-class one);
- target has a compare-at-bottom, draft has `bdnz` -> **you inlined away a call
  the original really made.**

Either way the defect is the CALL, not the loop spelling. Citation pair, one file,
one class, same bound: `d_a_en_bigpile.cpp:56` `BigPileMng_c::remove()` is
call-free and gets `li r0,0x3; mtctr; ... bdnz`; `d_a_en_bigpile.cpp:81`
`BigPileMng_c::move()` differs only by `mpPiles[i]->move()` in the body and gets
`bl` + `cmpwi r30, 0xc; blt`.

Second-order, for call-free loops (75 `bdnz` vs 30 not): `bdnz` additionally needs
a trip count MWCC can hoist. A **member-load bound plus a memory store in the
body** kills it, because the store may alias the bound --
`d_a_en_snake_block.cpp:451, 645, 687` are all
`for (int i = 0; i < mBlockCount; i++) { mBlocks[i]... = ...; }` and all get a
compare-at-bottom, while the six `d_cd.cpp` accessors, `d_rail.cpp:4`,
`d_next.cpp:82` and `d_res.cpp:206` have the same member-load bound with
**read-only** bodies and all get `bdnz`. A constant or address-arithmetic bound
(`&mScissor[4]`, `d_tag_processor.cpp:22`) is invariant regardless of stores.

---

**C. NEW: the loop's ENTRY says whether the source bound is a compile-time
constant.** Three mutually exclusive shapes, and the constant direction has zero
exceptions in 84 cases:

| before the loop | the source bound is |
|---|---|
| nothing at all | a **compile-time constant** (literal or named const), from a constant start |
| an unconditional `b` forward into the middle of the loop | a **runtime value**, compare-at-bottom |
| `cmplwi rN, 0` / `ble <past the loop>` after the `mtctr` | a **runtime value**, `bdnz` |

Measured on `for` loops: constant bound from 0 -> **0 entry jumps in 84**; runtime
bound -> 58 entry jumps, and the 15 remaining use the `bdnz` zero-trip guard
instead. `d_a_en_bigpile.cpp:36` (`i < MAX_PILE_COUNT`) enters with nothing;
`d_cd.cpp:69` (`i < mScrollDataCount`) enters with `mtctr r0; cmplwi r0, 0; ble`.

Two things this retires:

- **`for` versus `while` is codegen-NEUTRAL** -- 135 `for` and 41 `while` loops all
  produce the identical rotated compare-at-bottom shape. Rewriting one into the
  other to chase a loop residual is a wasted round. (`do { } while ()` is
  different: 5/5 have no entry jump, which is the positive form of the recorded
  "outer guard plus `while` reproduces a redundant jump" finding.)
- An **unexplained `b` into the middle of a loop is not a scheduling artefact**;
  it is the compiler telling you the bound is not constant.

Related and low-n but readable: **the `mtctr` seed against the source bound
detects unrolling.** `d_a_en_bigpile.cpp:36` `entry()` has `return true` in the
body and seeds CTR with 12 (one body); its sibling `remove()` at `:56` has no
early exit and seeds CTR with 3 with the body emitted four times
(`0xc/0x10/0x14/0x18`, `addi r7, r7, 0x10`). Same file, same bound. One clean pair
only -- do not assume the factor is always 4.

---

**D. CORRECTION: the `switch` vs `if`/`else if` disassembly signature is only
decisive when the compares are LITERALLY ADJACENT -- and a `bctr` is almost never a
switch.**

Two bounds on lever 1 and on the diagnostic recorded in `HANDOFF.md:8148`.

**(i) `bctr` is a virtual tail call 96% of the time.** 291 `bctr` in the corpus,
**11** are jump tables and **280** are `lwz r12,0(r3); lwz r12,0xNN(r12); mtctr;
bctr` virtual dispatch. There are no `jumptable_*` labels anywhere, so match on
the table's uniform prologue instead: `cmplwi rX, N; bgt <default>; lis; slwi;
addi; lwzx; mtctr; bctr`. **That `cmplwi` immediate is the case-value span** -- `N`
means slots 0..N, so it reads the original's case range straight off the target.

**(ii) A `switch` with fewer than 8 case labels NEVER gets a jump table** --
127/127 in the corpus. So a small `switch` and a small `if`/`else if` chain are
indistinguishable on that axis (which is why lever 1's rewrite is worth trying at
all), and conversely, a jump table in the target puts a **floor of 8 case labels**
on the construct you have to write. Between 8 and 12 labels it is decided by the
number of distinct case BODIES and their density, not the label count:
`d_a_wm_ghost.cpp:105` has 8 labels but 4 bodies and stays a ladder;
`d_a_player.cpp:6659` has 8 labels over a dense 0..8 and gets a table.

**(iii) The `beq`-ladder tell is 53% at function granularity and 95% only in its
strict form.** Grouping compares of one register anywhere in a window: 98 of 184
`beq` ladders come from a `switch` -- a coin flip. Requiring `cmpwi`/`beq` pairs
with **nothing between them**, three or more in a row: **99 of 104**. And there is
not one strictly-adjacent `bne` chain in the corpus, because an `else if`
necessarily puts the previous block's code between its compares. So:

> An adjacent `cmpwi`/`beq` ladder means the source is EITHER a `switch` OR a
> single `||`-chain of equality tests against the same value -- those are the only
> two known producers (four of the five corpus exceptions are that case, including
> its `!=`/`&&` dual at `d_wm_actor.cpp:38` and `:66`; the fifth is unexplained).
> Non-adjacent compares prove nothing, and the shape must not be used to pick a
> construct in that case.

---

**E. NEW NEGATIVE: there is no `fsel` in this compiler configuration.** Zero
occurrences in 185,723 instructions of matched code -- likewise zero `subfe`, zero
`isel`, zero `bdz`. Lever 2's "ternary between two adjacent small constants
compiles to ARITHMETIC" is an **integer**-only phenomenon; there is no
floating-point branchless-select to reach for, and a float residual that looks
like it wants one is something else. The branchless integer idiom that does exist
is `cntlzw`+`srwi` (102 sites), already covered by lever 8.
