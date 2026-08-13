# EMISSIONS.md -- `d_a_player_manager.cpp` spurious-emission audit

Scope: stop `wip/player_manager/assembled.cpp` from emitting functions the
original `0x8005E9A0-0x800613B0` does not have, per `SHARED-BRIEF.md`'s task.
`assembled.cpp` is the only file edited here; header changes below are
**proposed, not applied**.

## Before this round (restated, and one correction)

The prompt's baseline table was accurate for the emission list and the
42/65 match count, but **could not be reproduced verbatim at the start of
this session**: `wip/player_manager/assembled.cpp` failed to compile with

```
950: SndAudioMgr::sInstance->startSystemSe(SE_SYS_ONE_DOWN, ...);
Error: (10199) ambiguous access to overloaded function
  'SndAudioMgr::startSystemSe(unsigned long, unsigned long)'
  'SndAudioMgr::startSystemSe(unsigned int, unsigned long)'
```

Root cause, confirmed by `git log`: commit `6ed7422` ("Break the
startSystemSe overload deadlock") landed a second `startSystemSe` overload
into `include/game/snd/snd_audio_mgr.hpp` **while this session was running**,
and our TU's two call sites (`decRest`, `setHipAttackQuake`) were not among
the four TUs that commit fixed. This is a real, load-bearing fix, not
optional cleanup -- without it nothing below could be tested. Applied,
following the established pattern from that commit and confirmed against the
target disassembly for which overload each site actually needs:

- `decRest`: `startSystemSe(SE_SYS_ONE_DOWN, ...)` -> `startSystemSe((u32)SE_SYS_ONE_DOWN, ...)`.
  Target (`0x8006062C`) calls `startSystemSe__11SndAudioMgrFUiUl` -- the `u32`
  overload, matching all seven other call sites already fixed project-wide.
- `setHipAttackQuake`: `startSystemSe(seTable[count-1], 1)` ->
  `startSystemSe((unsigned long)seTable[count - 1], 1)`. Target (`0x80060D54`)
  calls `startSystemSe__11SndAudioMgrFUlUl` -- the **other** overload. This
  matches commit `6ed7422`'s own note: "The FUlUl overload is now declared and
  pinned in syms.txt at 0x801954B0, which d_a_player_manager.cpp's
  setHipAttackQuake needs." `(u32)` here would have picked the wrong overload
  silently (no compile error, wrong target).

With that fix, `assembled.cpp` compiles again and reproduces the documented
baseline exactly: **42 exact, 22 differing**, ten emissions.

## Per-emission verdict

| Symbol | Size | Verdict | Action |
|---|---|---|---|
| `fn_8005f4d0__9daPyMng_cFP7mVec3_cii` | 39 | FINE | none (per brief) |
| `fn_80060DB0__Fv` | 78 | FINE | none (per brief) |
| `__sinit_\assembled_cpp` | 40 | near-right (target 39) | not investigated further this round; not a spurious function, out of scope per brief |
| `__dt__7mVec2_cFv` | 16 | **Must go -- no safe fix found** | see below, NOT fixed |
| `__dt__Q23EGG8Vector2fFv` | 16 | **Must go -- no safe fix found** | see below, NOT fixed |
| `__dt__Q23EGG8Vector3fFv` | 16 | **Must go -- no safe fix found** | see below, NOT fixed |
| `getPlrNo__8dActor_cFv` | 2 | Investigate | **FIXED in assembled.cpp** |
| `isItemKinopio__7dAcPy_cFv` | 5 | Investigate | **FIXED -- header change proposed** |
| `executeLastAll__10daPlBase_cFv` | 1 | Investigate | **FIXED -- header change proposed** |
| `executeLastPlayer__10daPlBase_cFv` | 1 | Investigate | **FIXED -- header change proposed** |

Net: 10 extras -> 6 extras. Of the remaining 6, three (`fn_8005f4d0`,
`fn_80060DB0`, `__sinit`) are explicitly not this round's problem per the
brief. The three `__dt__` entries remain, and the reason is the main finding
of this round.

---

## Fixed in `assembled.cpp`: `getPlrNo__8dActor_cFv`

**Cause.** `deleteCullingYoshi` called `p->getPlrNo()` by name (line ~1153).
`dActor_c::getPlrNo()` (`include/game/bases/d_actor.hpp:105`) is
`virtual s8 &getPlrNo() { return mPlayerNo; }` -- an inline virtual. This is
*exactly* the trap the header comment above `getVfunc6c` already documents
(added by a previous batch after hitting it in `getYoshi`): a named call to
this function drags a weak out-of-line copy into the object.

**Evidence it's an orphan.** `bl getPlrNo__8dActor_cFv` does not appear
anywhere in the compiled object (grepped the disassembly) -- the actual call
site resolves via vtable dispatch either way, matching target's
`lwz r12,0x60(r3); lwz r12,0x6c(r12); mtctr r12; bctrl` shape. The flush is
pure compiler eagerness, not a wrong call shape at the site that uses it.

**Fix.** Reused the existing `getVfunc6c` vtable-slot-cast helper (already
used by `getYoshi`, one function earlier in this same file) instead of the
named call:

```cpp
-        if (p->getPlrNo() != -1) continue;
+        if (getVfunc6c(p)((dActor_c *)p) != -1) continue;
```

**Result.** `getPlrNo__8dActor_cFv` is gone from the emission list.
`deleteCullingYoshi` stays byte-length-identical to target (86 = 86
instructions, unchanged from before the fix) and the overall 42/22 split is
unchanged -- this is a clean win, no trade.

**The "trade" the brief asked me to resolve.** That trade (named call gets
`getYoshi` fully correct at the cost of the orphan, vs. the vtable cast
avoiding the orphan at the cost of 2/39 wrong instructions in `getYoshi`) was
about `getYoshi` specifically, and it was **already resolved before this
session** -- `getYoshi` already uses `getVfunc6c` (line ~684), and is already
at `target=39 draft=39 (~2 lines)`. That earlier choice did not, by itself,
eliminate the orphan, because `deleteCullingYoshi`'s *independent* named call
to the same function was still flushing it. Fixing that second call site
(this round) removes the orphan with **no cost at all** to `deleteCullingYoshi`
(same instruction count before and after) -- there was no trade to make here,
only a second call site nobody had converted yet.

---

## Proposed header change (NOT applied): `isItemKinopio`, `executeLastAll`, `executeLastPlayer`

**Cause.** All three are virtual functions with inline bodies in shared
headers, called through vtable dispatch somewhere in this TU:

- `dAcPy_c::isItemKinopio()` -- `include/game/bases/d_a_player.hpp:187`,
  `virtual bool isItemKinopio() { return mIsRescueKinopio; }`. Called at five
  sites (`addNum`, `decNum`, `getItemKinopioNum`, `getItemKinopio`,
  `changeItemKinopioPlrNo`), **all five of which are already MATCH**.
- `daPlBase_c::executeLastPlayer()` / `executeLastAll()` --
  `include/game/bases/d_a_player_base.hpp:568-569`, both `virtual void ... {}`.
  Called once each, inside `daPyMng_c::executeLastPlayer` /
  `daPyMng_c::executeLastAll`, both of which are **already MATCH**.

Same shape as `getPlrNo`: confirmed by grep that none of these three symbols
is ever the target of a `bl` anywhere in the compiled object. Every call site
that uses them already produces correct, matching vtable-dispatch bytes; the
flush is an orphan the -ipa pass emits regardless.

**Why this is a *different* situation from the three `__dt__` entries below,
and testable independently.** These three are reached only through indirect
(`bctrl`) virtual dispatch. Virtual dispatch needs the function's *address*
for the vtable slot, not its inlined body at the call site -- so removing the
inline body should not change any call site's codegen, only stop the eager
flush. That is a testable, falsifiable claim, and I tested it rather than
asserting it.

**Test method.** Shadow-copied `d_a_player_base.hpp` and `d_a_player.hpp`
into `scratch/player_manager_emissions/shadow_include2/`, changed only:

```cpp
// d_a_player_base.hpp
-    virtual void executeLastPlayer() {}
-    virtual void executeLastAll() {}
+    virtual void executeLastPlayer();
+    virtual void executeLastAll();

// d_a_player.hpp
-    virtual bool isItemKinopio() { return mIsRescueKinopio; }
+    virtual bool isItemKinopio();
```

then recompiled `assembled.cpp` with that directory prepended to the `-i`
search path (via `harness.INCLUDES`, not by touching `include/`), so mwcc
picks up the shadow copies without any real header being modified. Full
`unit_verify.py` comparison run against this shadow build
(`scratch/player_manager_emissions/test_shadow2.py`).

**Result -- clean.**
- All three symbols gone from the emission list.
- **42 exact, 22 differing -- unchanged.**
- Every previously-MATCHing function that calls through these three
  (`addNum`, `decNum`, `getItemKinopioNum`, `getItemKinopio`,
  `changeItemKinopioPlrNo`, `executeLastPlayer__9daPyMng_cFv`,
  `executeLastAll__9daPyMng_cFv`) is **still MATCH**, byte-for-byte identical
  instruction count and canonical text to before the header change.
- `incCoin`/`addRest`/`getCoinAll` etc. (unrelated to these three) unchanged.

**Proposal, for the lead to apply and verify across five binaries:**

```diff
--- a/include/game/bases/d_a_player_base.hpp
+++ b/include/game/bases/d_a_player_base.hpp
@@
     virtual void executeMain() {}
-    virtual void executeLastPlayer() {}
-    virtual void executeLastAll() {}
+    virtual void executeLastPlayer();
+    virtual void executeLastAll();
     virtual bool isItemKinopio() { return false; }
```

```diff
--- a/include/game/bases/d_a_player.hpp
+++ b/include/game/bases/d_a_player.hpp
@@
-    virtual bool isItemKinopio() { return mIsRescueKinopio; }
+    virtual bool isItemKinopio();
```

**Caveat, stated plainly rather than assumed away.** A declared-but-undefined
virtual still needs a real out-of-line definition to exist somewhere in the
final link so the vtable entry resolves. I have not located or written that
definition -- I did not touch `source/dol/bases/d_a_player.cpp` or
`d_a_player_base.cpp` (out of scope: "the only file you may modify is
`assembled.cpp`"). Two things make me confident it already exists rather than
being a new gap: (1) these functions are genuinely *called* at runtime in the
retail game through real vtable slots, so a real body exists in the retail
binary regardless of what our header says; (2) `d_a_player.cpp` /
`d_a_player_base.cpp` are described in `SHARED-BRIEF.md` as banked neighbours
already landed. But I did not verify their `.o`/source actually provides
non-inline bodies for these three specific functions, or that they are
covered by an existing `syms.txt` pin (as the `startSystemSe` `FUlUl`
overload was, per the commit above) -- **that check belongs to the lead**,
same as the five-binary verification.

**`daPlBase_c::isItemKinopio() { return false; }`** (the *base* class's own
version, distinct from `dAcPy_c`'s override above) was deliberately **left
alone** -- it never appears in the emission list (only the `dAcPy_c`
override, 5 instructions, does), so touching it was not needed and would have
been an unproven change.

---

## NOT fixed, and why: `__dt__7mVec2_cFv`, `__dt__Q23EGG8Vector2fFv`, `__dt__Q23EGG8Vector3fFv`

**The task's suggested technique does not work here, and I tested it rather
than asserting that.** The brief's own worked example (`dPyEffect_c` /
`followEffect_c`) fixed an analogous-looking problem by declaring
constructors/destructors without bodies. I tried the same shape on
`mVec2_c`/`EGG::Vector2f`/`EGG::Vector3f` (all three have inline empty-body
destructors in `include/game/mLib/m_vec.hpp` and
`include/lib/egg/math/eggVector.h`) and it makes things worse, not better.

**Where the classes are used.** Not as embedded-by-value class members (the
`dPyEffect_c` shape) -- as **plain local variables with real automatic
storage duration**, inside functions that already differ from target:
`incCoin`/`addRest` construct `mVec2_c pos(fx, fy)`; `deleteCullingYoshi`
declares `mVec2_c mid`, `ppos`, `delta`; `createYoshi`/`create`/`fn_8005f4d0`
use `mVec3_c` locals similarly (those three are already MATCH or close, and
are not implicated in the flush -- see below).

**What the target actually does, read directly from
`target_text.txt`.** `incCoin`'s target computes the `mVec2_c`-shaped value as
two raw `stfs` stores into stack offsets (`0x20(r1)`/`0x24(r1)`) and later
passes `addi r3, r1, 0x18` / `addi r4, r1, 0x20` as bare stack-slot pointers
into `cvtSndObjctPos__6dAudioFRC7mVec2_c` -- **there is no constructor call
and no destructor call anywhere in the target's `incCoin`, `addRest`, or
`deleteCullingYoshi`.** The class's non-trivial-looking construction and
destruction are fully elided at every one of these call sites, on both sides
currently (draft included) -- this part already matches.

**Why the orphan exists at all, and why "declare without body" breaks the
part that already matches.** In the *current* (unmodified) headers, MWCC:
1. eagerly flushes a weak, uncalled copy of the destructor the first time the
   class is used as a local (once per class, not once per call site -- this
   matches the observed single occurrence each of `__dt__7mVec2_cFv` /
   `__dt__Q23EGG8Vector2fFv` / `__dt__Q23EGG8Vector3fFv`, confirmed by grep to
   have **zero** `bl` references anywhere in the object), **and**
2. still fully elides the actual destructor call at every real local-variable
   scope-exit, because it can see the (empty) body and prove the elision
   safe.

Declaring the destructor without a body removes (1) but also removes the
*information* the compiler needs to do (2): with no visible body, MWCC can no
longer prove the destructor does nothing, so it must conservatively emit a
real `bl` to the (now-external) destructor at every local scope exit that
previously had zero code.

**Tested, not asserted.** Same shadow-header method as above, three variants,
via `scratch/player_manager_emissions/test_shadow.py` (edit
`shadow_include/game/mLib/m_vec.hpp` /
`shadow_include/lib/egg/math/eggVector.h` between runs):

| Variant | Orphans removed | Orphans added | Regressions (real `bl` added where target has none) |
|---|---|---|---|
| `mVec2_c::~mVec2_c()` declared-only only | `__dt__7mVec2_cFv` | none | `incCoin` +9 insns (126->135), `addRest` +9 (74->83), `deleteCullingYoshi` +12 (86->98) |
| `~mVec2_c()` + `~Vector2f()` + `~Vector3f()` all declared-only | all three `__dt__` entries | **new**: `__dt__7mVec3_cFv` (22 insns) -- `mVec3_c`'s own destructor, previously never flushed at all because it fully collapsed into `EGG::Vector3f`'s inline body | same three functions regress by the same amounts, plus `fn_80060DB0` +3 (78->81) |

In both variants **the 42/22 split itself did not change** (these functions
were already in the "differing" bucket, not "exact"), so no *match* is lost
by the letter of the metric the brief asks me to watch. But the actual
instruction content moves further from target in three functions that
currently reproduce the target's true "no destructor call at all" behavior
exactly. Trading a cosmetic, unreferenced extra symbol for a real, measured
divergence in already-imperfect functions is the wrong direction, so **I did
not propose this header change** and left `m_vec.hpp` / `eggVector.h` alone.

**One more data point, unresolved, worth recording rather than guessing
past.** `wiimj2d_symbols.txt` shows `__dt__7mVec2_cFv` (`0x80006DF0`) and
`__dt__7mVec3_cFv` (`0x8000FBF0`) genuinely exist as weak symbols in the
retail binary -- somewhere, some TU legitimately flushes each of them, and
neither address is anywhere near our `0x8005E9A0-0x800613B0` range. By
contrast `__dt__Q23EGG8Vector2fFv` and `__dt__Q23EGG8Vector3fFv` appear
**nowhere at all** in the whole (~22%-decompiled) symbol map. I cannot tell
from here whether our TU's own compiled object (pre-link) is supposed to
contain a weak copy that a linker would fold away against the winning TU's
copy (which would make the "must be zero" framing about this tool's
per-TU-isolated-compile model rather than about the real link), or whether
the true original source for `mVec2_c`/`mVec3_c`'s locals in this file uses a
different idiom entirely (e.g. raw floats or the already-declared
`mVec2_c_POD_c`/`Vec` types, matching what the target's own disassembly shows
with zero object-construction codegen) that never triggers a flush to begin
with. Both are plausible; I did not have a way to settle it from the evidence
available to me, and I'd rather say so than guess. Left the three `__dt__`
entries as they are.

---

## Before / after summary

**Before (documented baseline, and reproduced after fixing the concurrent
`startSystemSe` compile break):**
```
42 exact, 22 differing, 0 matching-text-but-wrong-branches
Extras (10): __dt__7mVec2_cFv, __dt__Q23EGG8Vector2fFv, __dt__Q23EGG8Vector3fFv,
             __sinit_\assembled_cpp, executeLastAll__10daPlBase_cFv,
             executeLastPlayer__10daPlBase_cFv, fn_8005f4d0__9daPyMng_cFP7mVec3_cii,
             fn_80060DB0__Fv, getPlrNo__8dActor_cFv, isItemKinopio__7dAcPy_cFv
```

**After `assembled.cpp`'s `getPlrNo` fix (applied) + the proposed, tested,
not-yet-applied header change (`isItemKinopio`/`executeLastAll`/`executeLastPlayer`):**
```
42 exact, 22 differing, 0 matching-text-but-wrong-branches   <- unchanged, no match traded
Extras (6): __dt__7mVec2_cFv, __dt__Q23EGG8Vector2fFv, __dt__Q23EGG8Vector3fFv,
            __sinit_\assembled_cpp, fn_8005f4d0__9daPyMng_cFP7mVec3_cii, fn_80060DB0__Fv
```

Of the remaining 6: three (`fn_8005f4d0`, `fn_80060DB0`, `__sinit`) are
explicitly out of scope per the brief's own table. The three `__dt__` entries
are the only ones still genuinely open, and are a negative result: the
suggested technique was tried, measured, and found to cost more than it
saves. **Match count did not drop at any point in this round** -- everything
above is either a clean removal or a rejected, unapplied experiment.

## Files touched

- `wip/player_manager/assembled.cpp` -- two edits: the `startSystemSe`
  overload-disambiguation casts (`decRest`, `setHipAttackQuake`, needed just
  to get the file compiling again after a concurrent header landed mid-run),
  and the `getPlrNo` vtable-cast fix in `deleteCullingYoshi`.
- `wip/player_manager/EMISSIONS.md` -- this file.
- `scratch/player_manager_emissions/` -- disposable shadow-header test rig
  (`shadow_include/`, `shadow_include2/`, `test_shadow.py`, `test_shadow2.py`,
  `final_run.txt`). Not part of the deliverable; left in place per
  `AGENT_CONTEXT.md`'s "scratch/ is shared and disposable" in case the lead
  wants to re-run a variant.

---

## LEAD'S ADDENDUM — the proposed header change fails at LINK, and `scope:weak` changes the whole picture

**The three "declare without inline body" proposals were applied and reverted.**
They compile, and the agent's isolated test correctly showed no regression — but
the build fails at link:

```
undefined: 'daPlBase_c::executeLastPlayer()'   Referenced from 'daPlBase_c::__vt' in d_a_player_base.o
undefined: 'daPlBase_c::executeLastAll()'      Referenced from 'daPlBase_c::__vt' in d_a_player_base.o
undefined: 'dAcPy_c::isItemKinopio()'          Referenced from 'dAcPy_c::__vt' in d_a_player.o
```

A **vtable slot needs a real function address**, so a virtual with no body
anywhere is undefined the moment any TU emits the vtable. The agent's test
compiled only our own TU, where those vtables are not emitted, so its scope
genuinely could not see this. Not a mistake in its reasoning — a limit of the
only test available to it.

### The finding that matters more: most of these are `scope:weak`

```
isItemKinopio__7dAcPy_cFv        0x80038FD0  0x14  scope:weak
executeLastAll__10daPlBase_cFv   0x800588E0  0x04  scope:weak
executeLastPlayer__10daPlBase_cFv 0x800588F0 0x04  scope:weak
__dt__7mVec2_cFv                 0x80006DF0  0x40  scope:weak
```

**A weak symbol is deduplicated by the linker — every TU that flushes a copy
contributes nothing, because only one survives.** So our TU emitting these is
*correct behaviour*, not a defect, and they do NOT add bytes to our `.text`.
Four of the six "spurious emissions" are therefore not spurious at all.

`getPlrNo__8dActor_cFv` (`0x8001D200`) is listed **without** `scope:weak`, which
makes it the one genuinely worth avoiding — and the agent already removed it, by
converting `deleteCullingYoshi`'s named call to the vtable-slot cast that
`getYoshi` was already using.

That leaves **`__dt__Q23EGG8Vector2fFv` and `__dt__Q23EGG8Vector3fFv`**, which
appear **nowhere in the symbol map at all**. The original never emits them, so
ours doing so is real. The agent showed the "declare without body" fix trades
them for wrong bytes in `incCoin`/`addRest`/`deleteCullingYoshi`, because these
are real local variables and the compiler needs the visible body to prove the
destructor call can be elided. **That negative result stands and is the correct
answer for now.**

### What this means for the trial link

The emission list is no longer a blocker. Two symbols are genuinely extra, both
tiny, and the rest are either ours or weak-deduped. **Trial-link and let the
link arbitrate** rather than reasoning further about it — that is the doctrine,
and this is exactly the kind of question it settles cheaply.
