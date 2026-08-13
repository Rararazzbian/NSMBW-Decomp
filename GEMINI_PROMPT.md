# Work order for Gemini — round 2

**Read `AGENT_CONTEXT.md` first.** It is new since your round 1 and it now holds
all the standing material — the rules, the tooling, the evidence hierarchy, the
MWCC behaviours that have cost people rounds. Round 1's prompt carried a lot of
that inline; this file no longer repeats it.

Write results to **`GEMINI_RESPONSE.md`** (overwrite). `GEMINI_HANDOFF.md` is
yours if you want a notebook that persists.

---

## Round 1 verdict: all five landed, all five binaries verify

That was a strong first round. Every target came with a mangled symbol, real
disassembly, and an explicit offset-perturbation verdict — which is exactly the
format that lets me apply something quickly and safely.

Specifically worth calling out:

- **`PauseManager_c` recovered from nothing.** The class existed nowhere in the
  tree and you got it from `setPauseEnable__14PauseManager_cFb` via the CFront
  length prefix, plus 20 sibling methods and five `.sbss` statics. That technique
  is now written into `AGENT_CONTEXT.md` §5 for everyone.
- **You caught that `getGameDisplay()` must not have an inline body**, because
  several TUs reach it with a `bl`. That is precisely the distinction that makes
  the difference between a header that works and one that emits stray weak copies
  into already-matching units.
- **`dStageTimer_c`** — using `createInstance`'s `__nw__FUl(0x10)` to pin the
  total size, not just the field offset, is the right instinct. A field offset
  alone would have left the tail ambiguous.

**One change I made when applying:** your `PauseManager_c` invented five named
instance fields (`mState`, `mUnk08`…) to fill `0x04`–`0x18`. Only `mFlags` at
`0x18` is actually evidenced — by `setPauseEnable` doing `lbz`/`ori 0x2`/`stb` on
`0x18(r3)`. The rest is now an honest `u8 pad4[0x14]`. Nothing embeds the class
by value, so the unknown region costs us nothing, whereas five invented names
would have looked like knowledge to the next reader. **Your method list survived
intact**, because every entry there is a real symbol.

The rule, now in `AGENT_CONTEXT.md` §4: a pad you label as a pad is a good
answer; an invented member name is not.

---

## Task A (primary): break the `startSystemSe` overload deadlock

This is a real, bounded blocker that I hit and deliberately backed out of, and it
is the kind of forensic work your round 1 was good at.

### What happened

A batch needed `SndAudioMgr::startSystemSe` and reported its first parameter as
the wrong type. That report was wrong, but it surfaced something true: **the
symbol map has TWO overloads.**

```
startSystemSe__11SndAudioMgrFUiUl = .text:0x801954C0   // (unsigned int,  unsigned long)
startSystemSe__11SndAudioMgrFUlUl = .text:0x801954B0   // (unsigned long, unsigned long)
```

`include/game/snd/snd_audio_mgr.hpp` declares only the `FUiUl` one. I added the
second and **the build failed**: MWCC error 10199, *ambiguous access to
overloaded function*, at all seven existing call sites. They pass enum and int
constants that convert equally well to `unsigned int` and `unsigned long`, so
with both overloads visible no call resolves.

I reverted it and wrote the whole finding into the header so nobody repeats it.

### Why this is worth solving

Right now our reconstruction can only ever call **one** of two functions that both
exist in the retail binary. Every future TU that needs the `FUlUl` form hits the
same wall. And the seven existing call sites are currently resolving to `FUiUl`
**by default rather than by evidence** — nobody has checked whether that is even
correct for each one.

### What to produce

For each of the seven call sites, determine **which overload the original
actually calls**, from the disassembly:

```
source/dol/bases/d_a_player_base.cpp:3967
source/dol/bases/d_pausewindow.cpp:357
source/d_profileNP/bases/d_controller_information.cpp:87
source/d_profileNP/bases/d_yes_no_window.cpp:428, 529, 551, 595
```

All of those TUs are **already banked and byte-exact**, so the answer is in the
binary: find each call site's address and read whether the `bl` targets
`0x801954C0` or `0x801954B0`. That is a fact, not an inference — see
`AGENT_CONTEXT.md` §5.

Then propose the minimal change that lets **both** overloads be declared without
ambiguity. Options to weigh, and I want your judgement on which is right:

1. Cast at each call site to the argument type that selects the correct overload.
   Honest, but it edits seven already-matching TUs — every one would need
   re-verification, and a cast that changes the resolved overload changes the
   emitted `bl`.
2. Change the *declared* parameter type of the existing overload so the natural
   argument type resolves unambiguously. Cheaper, but it changes a mangled name,
   which is never cosmetic — check the new name against the map.
3. Something else. If the constants passed have a type we have reconstructed
   wrongly (an enum that should be `u32`, say), fixing *that* might make every
   call site unambiguous with no cast at all. **Check this before the other two**
   — it would be the real answer rather than a workaround.

**Do not apply anything.** Propose it, and say for each of the seven call sites
whether your change would alter the emitted code. If your conclusion is "this
cannot be fixed without touching banked TUs, and here is the cost", that is a
perfectly good deliverable — I backed out of this once already and would rather
have an accurate map of the cost than a change that fails four binaries.

## Task B (secondary): scout the next unit

Only start this once Task A is reported.

`d_a_player_manager.cpp` is nearly done and I need the next target. The standing
notes rank candidates by header coverage, and **that ranking is not to be trusted
blindly** — the top two entries are both gated by functions that have defeated
every attempt so far, and several attractive-looking candidates are traps.

Known traps, so you do not re-derive them:

- **`0x80041C00`–`0x80044940`** (11,584 B, 86 fns) looks like one clean haul
  between two banked neighbours. It is **at least six TUs**, and only two
  `__sinit`s exist, so four internal boundaries are invisible and every one must
  be derived.
- **`d_a_en_obj_coinblock.cpp`** — fully bracketed, cheap-looking. But
  `__vt__18daEnObjCoinBlock_c` **does not exist anywhere in the symbol map**, and
  the range has no constructor, destructor, `create` or `execute` — its lifecycle
  lives in a `.rel`. Cheap bytes, expensive class.
- **`d_a_farBG.cpp`** — 55 functions across 18.6 KB is 339 B per function, the
  worst ratio of any candidate, and it is float/matrix-heavy, which is the shape
  to avoid.

**What I want:** a ranked shortlist of 3 candidates, each with its section bounds
**derived using `bin/dtk/dtk_splits_wiimj2d.txt`** (see `AGENT_CONTEXT.md` §5 —
this file gives hard bracketing and turns the weakest kind of bound into the
strongest). For each: function count, total bytes, bytes-per-function, whether
the class has a vtable in the map, how many `__sinit`s fall in the range, and
what could go wrong.

A candidate you **reject** with a reason is as useful as one you recommend —
the traps above cost real time before anyone wrote them down.

---

## Reminders that apply every round

- Never run `ninja`, `configure.py`, `progress.py`, `land.py`.
- Never edit a shared header, `slices/wiimj2d.json`, or `syms.txt` — propose.
- **Do not touch** `wip/`, `HANDOFF.md`, `AGENT_CONTEXT.md`, `GEMINI_PROMPT.md`,
  or any `CODEX_*.md`.
- Report contradictions rather than reconciling them; report a negative result
  rather than manufacturing a positive one.
- Plain ASCII or clean UTF-8 in `GEMINI_RESPONSE.md`, LF, no BOM.
