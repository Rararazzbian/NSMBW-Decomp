# Work order for Codex — round 9

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 9.

Write results to **`CODEX_RESPONSE.md`** (overwrite). `CODEX_HANDOFF.md` is yours.

---

## Round 8 verdict: the diagnosis is right, the fix fails all five binaries

**Your analysis is correct and I want to be clear about that first.** The two
empty destructors in `include/lib/egg/math/eggVector.h` are exactly why
`EGG::Vector2f` and `EGG::Vector3f` are non-trivially destructible; the `nw4r`
bases have no destructor; the types would otherwise emit nothing; and the reason
`mVec2_c`/`mVec3_c` behave differently is that they declare their *own*
destructors, which do have weak copies at `0x80006DF0` and `0x8000FBF0`. That
chain is right, and the "zero matches in the symbol map, zero" check is the
correct evidence.

**But I applied it and the build fails all five binaries:**

```
wiimj2d.dol      7f05a5dc244721651c2c196acb139ad5  (should be ddab9e5d...)
d_profileNP.rel  fd55f24553af1b7c1400c4adb2da548b
d_basesNP.rel    282fd56b04f3feb38ae2d6f6c2b953ec
d_enemiesNP.rel  4d26ed49b9f684ea84b339df13e360cd
d_en_bossNP.rel  d9145fe64610ff83e0daef392844a0e2
```

Reverted; the tree is green again.

**All five failing, including three `.rel`s, means the change reaches far beyond
the three functions you were targeting.** `EGG::Vector2f`/`Vector3f` are used
across the whole codebase, and making them trivially destructible changes code in
TUs that are already byte-exact. Your blast-radius search looked for arrays,
deletes and explicit destructor calls — the right instinct, but the actual
exposure is every *local variable* of those types in every banked TU, which that
search would not surface.

**You flagged this exactly right.** You wrote "Offset-perturbing: YES — emitted
text and linkage change, but object layout does not. full build verification
needed", and you supplied a fallback. That is the correct confidence level and it
is why applying it cost one build instead of a day. This is now the fourth
shared-header change to fail five-binary verification on this project, and the
fourth time the propose-don't-apply rule paid for itself.

**Take the fallback.** Two small orphan functions the linker places is a known,
quantified cost — and the trial link has now priced it exactly: `.text` overflows
its claim by `0x90`, of which `0x80` is those two destructors. Real, but bounded,
and not worth breaking five binaries for.

### One narrower idea, if you want to try once more

The failure is that the change is *global*. A change scoped to **our TU only**
would not touch banked code. Is there a formulation inside
`d_a_player_manager.cpp` — a different local type, a different expression shape
in `incCoin` / `addRest` / `deleteCullingYoshi` — that avoids materialising an
`EGG::Vector2f`/`Vector3f` local at all, and so never triggers the destructor?

Note what round 8 already established and do not re-litigate it: the target calls
`cvtSndObjctPos(const mVec2_c &)`, so the parameter type **is** `mVec2_c`. The
question is not what type is passed, but whether an EGG-typed temporary gets
materialised on the way there.

**Acceptance test is unchanged and strict**: the three function bodies must stay
byte-identical against `wip/player_manager/target_text.txt`. A fix that removes
the symbols but perturbs those three is worse than the problem. **If the answer
is no, say so in one line and move to Task B** — this is a `0x80` optimisation on
a unit that has bigger problems, and I would rather have Task B.

## Task B: the 23 near-misses, characterised as a group

This is the bigger prize and it is where the unit actually stands or falls.

`wip/player_manager/assembled.cpp` is **42 of 65 byte-exact**. The 23 that differ
are individually documented across `wip/player_manager/BATCH1.md` … `BATCH8.md`.
Nobody has yet looked at them **as a set**.

Read the batch reports and produce a **taxonomy**: how many of the 23 fall into
each failure class, and which functions are in each. Candidate classes, from what
is already known:

- **base-register / anchor artifacts** of isolated compilation, which should have
  resolved at assembly and may already have
- **register allocation only**, logic identical
- **instruction count differs** — a real logic or shape difference
- **scheduling only**, same instructions in a different order
- **constant-folding differences** from the `.sdata` constants
- anything that does not fit — those are the interesting ones

**What I want out of it is a ranked attack order**, not fixes. Which class has
the most members? Which has a known lever already recorded in
`wip/player_manager/SHARED-BRIEF.md` (the frame-layout lever and the return-type
lever are both there and both have paid)? Which functions are large enough that
fixing one is worth more than fixing five small ones?

`.text` overflows by `0x90` and `0x80` of that is the two destructors — so **the
23 near-misses account for only about `0x10` of overflow between them.** Most are
therefore same-size-but-different-bytes, which is a much more tractable problem
than it sounds, and the taxonomy should confirm or refute that.

---

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/` (read the batch reports and `target_text.txt` freely,
  write nothing), `HANDOFF.md`, `AGENT_CONTEXT.md`, `CODEX_PROMPT.md`, or any
  `GEMINI_*.md`.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
