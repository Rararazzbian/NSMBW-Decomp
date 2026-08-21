# Work order — round 18

**Read `AGENT_CONTEXT.md` first.** It is the standing briefing. This file is only
round 18.

Write results to **`QWEN_RESPONSE.md`** (overwrite it).

---

## Round 17 verified, and it is strong work

I recompiled `scratch/round17/d_bg_actor_mng.cpp` myself against your shadow
headers — clean compile, first try — and checked every function's length against
`bin/dtk/wiimj2d_symbols.txt`. **Your table reproduces exactly.** Sixteen named
functions at the correct length, four `__arraydtor$` thunks, and
`__sinit_\d_bg_actor_mng_cpp` **byte-exact at 685 words**, which is the single
largest thing in the unit and the part I expected to be hardest.

You also confirmed both padded section edges yourself rather than taking my word
for them, which is exactly what round 17 asked for and what the round-16 `.bss`
error existed to teach.

### One correction, and it is the most consequential kind on this project

You reported your two remaining failures as *"register allocation in grid
loops."* They are not. Measured:

```
ProcMain__17dBgActorManager_cFv        target 179   your draft 160   -19 words
createObjList__17dBgActorManager_cFb   target 116   your draft 107   -9 words
```

**A length mismatch is CONTENT. Register allocation physically cannot change an
instruction count** — it only changes which register appears in an operand. Those
two functions are **28 words of MISSING CODE** between them.

This matters because of what the label does next: "register allocation" is a
known unfixable wall here, so anything filed under it gets parked forever. A
content gap filed that way is a real, closable defect that nobody ever returns
to. **Four separate agents made this exact misdiagnosis today**, so you are in
good company — but the check is mechanical and needs no judgement at all:

> **Compare lengths BEFORE reading a single instruction.**
> Different length -> content. Same length, different bytes -> then, and only
> then, consider registers or scheduling.

And its partner rule, which cost me two wrong calls today: **a matching length is
not proof either.** Four functions on another unit were length-exact *by
cancellation* — a spurious instruction masking a real gap — and a correct fix made
the length column look worse. Only BYTE equality settles anything.

---

## Your task: close `ProcMain` and `createObjList`

Same unit, same directory — continue in `scratch/round17/` or start
`scratch/round18/`, your choice. Those two functions are 295 target words
between them and are all that stands between this unit and 21/22.

```
ProcMain__17dBgActorManager_cFv        0x8007E520   0x2CC   179 words   -19
createObjList__17dBgActorManager_cFb   0x8007E860   0x1D0   116 words   -9
```

Both are missing content, so the question is **what**, not how it is scheduled.
Suggestions, in the order I would try them:

1. **Diff the two against each other.** They are the only two failures and both
   are loop-heavy over the same object grid. A shared missing construct is more
   likely than two independent ones.
2. **Re-read branch targets ARITHMETICALLY** rather than trusting how the
   disassembly reads. That caught a real structural bug on another unit today: a
   conditional was skipping an entire two-call block, not the single call it
   appeared to guard. A 19-word gap is very plausibly one such block.
3. **Suspect a `switch` before transcribing constants.** Four `.data` blocks on
   another unit were flagged as hand-authored lookup tables and were nothing of
   the kind — MWCC-generated jump tables that the right `switch`/`case` structure
   reproduces automatically. Transcribing is the expensive mistake.
4. **Check for a missing early-out or bounds test.** A loop that the target
   guards and your draft does not is a cheap way to be exactly one block short.

## Rules of evidence that have each cost a round here

- **Return types are ABSENT from CFront mangling; parameters are encoded.** Well
  over two dozen wrong declarations found so far. Read what the CALLER does with
  the return register right after the `bl` — read, or clobber. An observed clobber
  outranks any analogy with a sibling.
- **A declaration is only tested by a CALL SITE.** An uncalled function's
  declaration is unverified however byte-exact its body is. Fourteen functions on
  another unit had wrong return types purely because nothing had ever called
  them.
- **An argument-count mismatch at a call site is a STORAGE-CLASS tell** — one
  register set where two are expected means no implicit `this`, so the function
  is a static member. Found four times today. **Note the trap: a static member
  function whose body never uses `this` compiles BYTE-IDENTICALLY to the
  non-static one**, so the error is invisible in the function and appears only at
  its callers.
- **The `.fn <name>, global` tag in a disassembly answers LINKAGE, not the
  static-member question.** `static` at file scope means internal linkage and the
  tag sees it; `static` on a member means no implicit `this` and the tag is
  silent. Do not conflate them.
- Read actual float/double literals out of `original/wiimj2d.dol`. Do not assume
  sibling symmetry — one function elsewhere uses `0.5f` where its siblings use
  bare double `0.5`.
- If a function reaches the correct instruction count and differs ONLY in
  register numbers, **stop and report the count.** That wall has taken 100+
  source variants across six functions here with zero successes.

## Rules

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- **Never edit a shared header, `slices/*.json`, or `syms.txt`.** Shadow-copy,
  prove your change locally, and put the diff in your response as a proposal.
- Work only in `scratch/round17/` or `scratch/round18/`. Do not touch `wip/`,
  `HANDOFF.md`, `AGENT_CONTEXT.md`, `peer_archive/`, or `GEMINI_*.md`. **`wip/`
  has live agent work in it**, and Gemini is authoring
  `d_enemy_toride_kokoopa.cpp` this round.
- Keep the draft named `d_bg_actor_mng.cpp` — anonymous-namespace symbols mangle
  the source filename into them.

## Deliverable

`QWEN_RESPONSE.md`, containing:

1. **The per-function table, length column FIRST**, for all 22 functions.
2. For each of the two target functions: **what the missing content was**, or —
   if you could not close one — every place you proved it is NOT, so the next
   person does not re-search there.
3. Your source in a fenced block, and any header proposal with its evidence.
4. Every variant tried and its result.
5. Anything you could not settle, plainly, with what would settle it.

**Closing one of the two is a good round. Closing neither but locating both gaps
precisely is an acceptable one.** I re-measure everything independently, so an
honest DIFF row is worth more to me than a claimed MATCH — and your honest DIFF
rows last round are exactly why I trusted the rest of your table.

Plain ASCII or clean UTF-8, LF, no BOM.
