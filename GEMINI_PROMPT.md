# Work order for Gemini — round 9

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 9.

Write results to **`GEMINI_RESPONSE.md`** (overwrite).

---

## First: I sent you to pre-flight a unit that was already finished. My fault.

`d_multi_mng.cpp` is landed. It has been landed for some time. The source file is
`source/dol/bases/d_multi_manager.cpp` — note the spelling, `manager` not `mng`,
which is why my check for `d_multi_mng` came back empty — and it is registered in
`slices/wiimj2d.json` at line 719 and verifying in all five binaries.

You even noticed this and said so plainly in §6.1: *"already recorded with exact
ranges"*. You reported it as an audit result rather than stopping, which was the
right call given the instruction you were handed, but you should not have been
handed it. I did not check before assigning. **The `sizeof(dMultiMng_c) == 0x5C`
confirmation is still genuinely useful** — it was an unproven assumption in
`d_a_player_manager.cpp`'s `.bss` and now it is not — and you derived it
independently rather than reading it off the landed header, which is what makes
it worth anything. The rest of Task B was wasted motion and that is on me.

## Second: §5's hazard is refuted by the very file you were analysing

You reported that the inline destructors in `m_vec.hpp` / `eggVector.h` cause
MWCC to emit `__dt__7mVec2_cFv`, `__dt__Q23EGG8Vector2fFv` and
`__dt__Q23EGG8Vector3fFv` into `d_multi_manager.o`, expanding `.text` from
`0x410` to `0x4D0`, and you proposed removing them.

**The emission is real. The hazard is not.** Here is the current, verifying
build's own object, `bin/compiled/wiimj2d/dol/bases/d_multi_manager.o`:

```
__ct__11dMultiMng_cFv, global          initStage__11dMultiMng_cFv, global
__dt__11dMultiMng_cFv, global          __dt__7mVec2_cFv, weak
__dt__Q23EGG8Vector2fFv, weak          setClapSE__11dMultiMng_cFv, global
setRest__11dMultiMng_cFii, global      addScore__11dMultiMng_cFii, global
incCoin__11dMultiMng_cFi, global       incEnemyDown__11dMultiMng_cFi, global
__dt__Q23EGG8Vector3fFv, weak          setBattleCoin__11dMultiMng_cFii, global
setCollectionCoin__11dMultiMng_cFv, global
```

All three weak destructors are present, the object is `0xC0` larger than the
`0x410` its slice claims, **and the DOL is byte-identical to retail.**

That is a general rule nobody in this project had written down, and it matters
far beyond this unit: **an unreferenced weak symbol emitted into an object does
not have to fit inside that object's slice `.text` claim. The linker does not
place it.**

Do not feel bad about the proposal — Codex reached the same conclusion from a
different unit, I applied it, and it broke all five binaries. Your version was
better evidenced than its. The reason it is wrong is a fact about linking that
neither of you could see from a compile.

**Do not propose removing those destructors again**, and treat any future
"my object's `.text` is bigger than the slice claim" observation as an open
question rather than a defect, until Task A below settles it.

---

## Task A: settle the weak-symbol placement rule properly

This is now the highest-value forensic question in the project, it is exactly
your kind of work, and it is blocking a real unit.

I have one data point: three unreferenced weak destructors in one object, not
placed, binary still exact. I want the rule, with its boundary.

Work it out from the link, not from compiles:

1. **How general is it?** Sweep the objects under
   `bin/compiled/wiimj2d/` and find every case where an object emits symbols
   whose total size exceeds its slice's `.text` claim. How many banked, verifying
   units are carrying unplaced weak symbols? If the answer is "most of them",
   that is the rule confirmed at scale.
2. **What is the boundary?** A weak symbol that IS referenced by another linked
   object must be placed. So the rule cannot be "weak symbols never count". Find
   a banked unit where a weak symbol *is* the surviving definition and confirm it
   occupies space inside its slice claim. The distinction I expect is
   referenced-vs-unreferenced, but **prove it rather than assuming my phrasing.**
3. **What does the project's own tooling do about it?** Read `tools/` — the lcf
   generation, the slicer, and anything handling `keepWeak`. Is this deliberate
   (a deadstrip directive somewhere) or is it the linker's default? If there is
   a `keepWeak` mechanism, when does it force a weak symbol to be placed, and
   would any of it apply here?
4. **The payoff, and please state it explicitly.**
   `wip/player_manager/TRIAL_LINK.md` measured `d_a_player_manager.cpp`'s
   compiled object at `0x2AA0` against a `0x2A10` claim and called it a `0x90`
   overflow. `0x80` of that is `__dt__Q23EGG8Vector2fFv` and
   `__dt__Q23EGG8Vector3fFv`. **Given your rule, does that unit have a real
   `.text` problem at all, and if so how big is it?** That is the question I
   actually need answered.

Write the rule up in a form I can paste into `HANDOFF.md`, with the evidence
that establishes each half of it and the boundary case that limits it.

## Task B: pre-flight `d_a_en_coin_main.cpp`

**I checked this time.** Not in `slices/wiimj2d.json`, no source file, not
started.

`0x800272F0`–`0x800281C0`, 3,792 B span / 3,652 B code / 23 functions. It is a
**base class** — `__vt__14daEnCoinMain_c` is `0x2EC`, the same size as
`daEnBlockMain_c`'s — so it gates the whole coin family living in
`d_enemiesNP.rel`. All its bounds are free.

Same standard as your `m_pad.cpp` round, which was the best pre-flight this
project has had: full function table with addresses, sizes, mangled names and
signatures; class reconstruction with the vtable proved entry-by-entry against
the original; complete data inventory with **referenced-by-anything marked per
object**; hazard proofs from an empty-bodied scaffold rather than hazard
predictions; the link-blocker list; and the pin list with the banked-slice
filter already run.

Two things the handoff records about this one, so you can confirm or refute them:

- It is described as having a milder version of a "shape problem" — worth
  characterising precisely rather than repeating.
- **Despite the matching vtable size, its function names barely overlap
  `daEnBlockMain_c`'s.** So the "blockmain just landed, this will be cheap"
  intuition is recorded as *not* paying. Check that: run the sibling comparison
  and tell me the real precedent rate by bytes, not by name. `tools/sibmap.py`
  does the mechanical part, and note its `FAMILY` list rots silently — a stale
  entry contributes nothing and just makes the map thinner, so check that the
  recently-landed units are in it and capture its stderr warning.

Apply the backward-bound audit from your round 8 to every section low bound.
That method is now standard and it is the one that caught the `0x70` I missed.

---

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`, `GEMINI_PROMPT.md`,
  or any `CODEX_*.md`. Codex is hunting a single structural cause behind
  `d_a_player_manager`'s register-allocation near-misses; stay out of that unit.
  `wip/nand_thread/` is my agents'.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
