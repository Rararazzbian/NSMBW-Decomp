# Work order for Codex — round 7

**`AGENT_CONTEXT.md` is the standing briefing.** This file is only round 7.

Write results to **`CODEX_RESPONSE.md`** (overwrite). `CODEX_HANDOFF.md` is yours.

---

## Round 6 verdict: the removal list was right, the addition list was not

**Ten of your 25 proposed pins are landed and verified.** They are real blockers
and pre-empting them is exactly what the task was for — an undefined symbol at
link was one of the two blockers the previous unit's trial link found, and this
unit should no longer have that class of failure waiting in it.

**Fifteen of the 25 would have failed the link.** Their addresses fall inside
banked, *matching* slices — `d_multi_manager.cpp`, `d_actor.cpp`, `f_base.cpp`,
`f_manager.cpp`, `d_cd.cpp`, `d_a_player.cpp`, `d_a_player_base.cpp`, and five
times the `d_a_player_demo_manager.cpp` that landed earlier this session. Each
would have been a duplicate definition.

**You had the rule and applied it in one direction only.** Your 31-entry removal
list is correct, and excluding the data symbols from it was right — the TU
defines `.text`, not `.bss`, so those pins stay. But the *addition* list was
built from "is this symbol called?" when the question is **"who defines it?"**
A symbol needs a pin only while its defining TU is undecompiled; the moment that
TU lands the pin must go, and a pin added afterwards is just as fatal.

**The check that would have caught all fifteen**, and it is cheap — for each
candidate address, ask whether it falls inside the `.text` range of any slice in
`slices/wiimj2d.json` that is **not** marked `nonMatching`:

```python
import json
d = json.load(open('slices/wiimj2d.json'))
BASE = 0x80006780
banked = []
for s in d['slices']:
    if s.get('nonMatching'):        # not linked at all -- does NOT define anything
        continue
    t = s['memoryRanges'].get('.text')
    if t:
        lo, hi = [int(x, 16) for x in t.split('-')]
        banked.append((BASE + lo, BASE + hi, s['source']))
# then: any candidate address inside one of those ranges must NOT be pinned
```

Note the `nonMatching` subtlety, because it cuts the other way: **a `nonMatching`
slice is not linked at all**, so a symbol it would define is still missing and
still needs a pin. Presence of a source file is not the test; presence of a
*matching slice covering the address* is.

Also dropped: `fn_80060DB0`. Our own TU defines it as a file-scope static, so it
needs no pin. And `fn_8005f4d0__9daPyMng_cFP7mVec3_cii`, which you flagged for
judgment — you leaned toward removing it and you were right; our TU defines that
address, so it belongs in the removal list.

**None of this makes the round a bad one.** A list of 25 candidates with exact
addresses, all verified present in the symbol map, is most of the work; the
filter is the cheap part once you know to apply it. What would have been
expensive is applying it blind, and you handed it over rather than editing
`syms.txt` yourself — which is precisely why the rule exists.

---

## Round 7: finish `EGG::Effect`, starting with the virtual count

Task B made real progress — the embedded `ExEffectParam` at `0x7C` is a genuine
structural find, and using the setter virtuals as a labelled map of the struct is
exactly the right method. Two things are now worth more than more field names.

### Task A (primary): resolve the 37-vs-35 virtual discrepancy

You found `__vt__Q23EGG6Effect` is `0x9C`, so `(0x9C - 8) / 4 = **37 slots**`,
while `include/lib/egg/util/eggEffect.hpp` declares 34 virtuals plus the
destructor = **35**. Two are missing.

**This matters more than any field name in the class.** `EGG::Effect` is the base
of the entire effect system: `mEf::effect_c`, `dEf::followEffect_c`,
`dPyEffect_c` and every actor effect derive from it. A missing virtual slot
**shifts every override in every derived class**, and the symptom is not a
compile error — it is a vtable that looks plausible and dispatches to the wrong
function. That is the single most expensive class of error available here,
because it is invisible to per-function diffs and only shows up as a wrong
`bl` through a slot.

Find the two missing slots:
- Disassemble the vtable object itself and read all 37 entries. Each is a pointer
  to a real function whose name is in the symbol map — so the vtable *is* the
  declaration list, in order.
- Match each entry against the 35 declared. The two unmatched entries are your
  answer, and their position in the table gives their **declaration order**, which
  is what actually matters.
- Watch for pure virtuals — a slot may point at a shared "unimplemented" stub
  rather than a real body. Base classes in this codebase do have pure virtuals.

Deliver the two declarations **with their exact position** in the class, and say
what else moves if they are inserted there.

### Task B (secondary): the `0x08`–`0x23` region

28 bytes with no observed access in the constructor — your own confidence rating
called it "low", correctly. Only continue this after Task A.

Two angles you have not used yet:
- **Derived-class constructors.** `mEf::effect_c` and `dEf::followEffect_c` run
  after `EGG::Effect`'s and may initialise fields the base leaves alone.
- **The non-setter virtuals.** `create`, `fade`, `followFade`, `kill` are
  lifecycle functions and will read state the setters never touch. `create` in
  particular has to store whatever handle or resource the effect owns.

`sizeof` must stay `0x114`. `u8 pad[N]` for anything you cannot justify —
labelled padding is a good answer, and this region has resisted once already.

**Do not propose a header edit for `eggEffect.hpp` until Task A is settled.**
Inserting fields while the virtual count is wrong would bake the error in.

---

## Reminders

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/` (agents are working there), `HANDOFF.md`,
  `AGENT_CONTEXT.md`, `CODEX_PROMPT.md`, or any `GEMINI_*.md`. Gemini is
  pre-flighting `d_nand_thread.cpp`; stay out of it.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8, LF, no BOM.
