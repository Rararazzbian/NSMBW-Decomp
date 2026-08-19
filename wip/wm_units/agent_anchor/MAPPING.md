# WM_ANCHOR (daWmAnchor_c) -- mapping / resume notes

Unit range: `.text 0x15a5a0-0x15ac80` (build.py's verify_anon window), 22 target
functions. `daWmAnchor_c : public dWmDemoActor_c`, `sizeof == 0x1f4`. Full class
layout, vtable analysis and member offsets are already documented at length in
the class-level doc comment at the top of `d_a_wm_anchor.cpp` -- not repeated
here.

## Tally history

- **Session start (measured, not taken on faith): 16/22.** `build.py`'s own
  `verify_anon` step reported `FUNCTION ORDER IS WRONG` -- see "Order-check
  finding" below; it is a false positive, not a real blocker.
  **CORRECTION (from the coordinator):** the "`check_fn_order.py` reported 0
  inversions" claim in an earlier version of this file was wrong in a way
  that matters generally, not just here. `check_fn_order.py` recovers a
  target address from a function's own NAME (`fn_2_ADDR`); this unit's
  functions already carry real mangled names, so the tool examined NOTHING
  for this file and silently printed "0 inversions" for an empty check -- it
  was never able to see WM_ANCHOR at all. The coordinator has since patched
  the tool to list unchecked files explicitly (confirmed: re-running it now
  prints WM_ANCHOR under "1 file(s) NOT CHECKED"). The only real order
  evidence for this unit has always been `build.py`'s own `verify_anon`
  step -- use that, not `check_fn_order.py`, for any file whose functions
  already have real names.
- **Session end (measured): 19/22.** Three functions fixed this round:
  `execute()`, `setNodePos()`, and a newly-split-out `resetProcessState()`
  (target `fn_2_15AAF0`, previously not written as its own function at all).

## What was fixed this round, and why (all VERIFIED by rebuild, not inferred)

### `execute()` -- was 42/63 differing, now MATCH

Two real logic/codegen bugs, found by full side-by-side disassembly comparison
against the target (`fn_2_15A770`):

1. **`dCsSeqMng_c::ms_instance` was being read TWICE** (once for
   `FUN_80915600()`'s receiver, again for `GetCutName()`/`m_164`). The target
   loads it once into r31 and reuses it, exactly like the already-landed
   `daWmAntlionMng_c::execute()`'s own `dCsSeqMng_c *csSeqMng = ...;` local.
   Fixed the same way here.
2. **`GetNodePos()` was being called unconditionally**, even when
   `fn_80100640()` returned null. The target's `beq` on a null result branches
   straight past the whole off/name/`GetNodePos` sequence to `mModel.play()`.
   Wrapped the block in `if (found) { ... }` (no `else`, matching the target --
   `mPos` is simply left untouched when the lookup fails). Also cached
   `daWmMap_c::m_instance` into a local `map`, reused for both `fn_80100640`'s
   first argument and `GetNodePos`'s receiver (target loads it once).

### `setNodePos()` -- was 31/55 differing, now MATCH

Three real fixes, all found by reading the target directly:

1. **The "not found" fallback for `mPos` is `mPos = mVec3_c(0.0f, 0.0f, 0.0f);`
   (construct-then-assign), NOT `mPos.x = mPos.y = mPos.z = 0.0f;` (chained
   assignment).** Proof: the target's own stack frame is `0x20`, not the `0x10`
   a plain chained assignment needs -- the extra `0xC` bytes hold a
   stack-built `mVec3_c(0,0,0)` temp (3 `stfs` to `0x8/0xc/0x10(r1)`) that then
   gets copied field-by-field into `mPos` (3 more `stfs`, to
   `0xac/0xb0/0xb4(r30)`) -- six stores total, both halves in ASCENDING x/y/z
   order. A chained assignment evaluates right-to-left (z first, no temp),
   which is the opposite of both observations. This is the HANDOFF
   "stack-temp question" lever in action: result stored -> constructor-plus-
   assignment.
2. **The `mScale` reset at the end is THREE independent statements**
   (`mScale.x = 1.0f; mScale.y = 1.0f; mScale.z = 1.0f;`), not a chained
   assignment either -- but unlike `mPos` above, this one needs NO stack temp
   (3 direct `stfs` to `0xdc/0xe0/0xe4(r30)`, ascending order). Confirms the
   "no temp -> direct field stores" half of the same lever, right next to the
   "result stored -> constructor-plus-assignment" case above in the SAME
   function -- two different idioms four lines apart, both now correct.
3. **`mUnk1f0 = 0` at the very end is a call to a SEPARATE out-of-line
   function (`fn_2_15AAF0`), not an inline `stw`.** See below.

### `resetProcessState()` -- new function, target `fn_2_15AAF0`, now MATCH

Previously not written as its own function at all -- the old draft inlined
`mUnk1f0 = 0;` directly into the tail of `setNodePos()`. The target has a
genuinely separate 3-instruction out-of-line function
(`li r0,0; stw r0,0x1f0(r3); blr`) at `0x15AAF0`, called via `bl` from
`setNodePos()`'s own tail, sitting in the target's address space between
`setNodePos` (`0x15AA10-0x15AAE8`) and `state_0` (`0x15AB00`). Declared in the
class right after `setNodePos()`, DEFINED in the source right after
`setNodePos()`'s own definition (matching target address order, keeping the
order-check gate green for free) and right before `state_0()`.

## What was tried and REVERTED (do not repeat)

**Constructor (`__ct__12daWmAnchor_cFv`, target `fn_2_15A5D0`), still 4/31
differing.** Tried moving `mUnk184 = -1;` from the initializer list
(`: mUnk184(-1) {}`) into the constructor body as the first statement. This
made things WORSE (4 differing -> 17 differing) and was reverted immediately;
confirmed by rebuild both times. **Do not try this again** -- the initializer-
list form is the right one; whatever is causing the 4-instruction residual is
something else.

The 4 differing instructions (verified via full target readout,
`grep -n "0015A5D0" target_auto_00_0015A4AC_text.txt`): target computes the
class's own vtable pointer into **r4** and stores `mUnk184=-1` into `0x184`
**before** computing `r3 = &mAllocator` (`addi r3,r31,0x188`); our draft
computes the vtable pointer into **r3** and computes `r3=&mAllocator`
**before** storing `mUnk184`. Content and semantics are 100% equivalent, this
is pure register-allocation/scheduling. Left alone as a real but small residual
-- not attempted further per the project's "don't grind a wall" guidance,
since one genuine attempt already regressed it.

## Parked, not attempted (documented reasoning, not a guess)

**Destructor (`__dt__12daWmAnchor_cFv`, target `fn_2_15A650`), 21/44
differing.** Full side-by-side instruction comparison against the target
(done by hand this round, not inferred) found **exactly ONE structural
difference: the target has a REDUNDANT, functionally-dead duplicate branch**
right before the base-class-inline destructor block:

```
target:  cmpwi r30,0x0 / beq .L_0015A6D4 / beq .L_0015A6D4   (TWO beq, same target, same untouched CR0)
draft:   cmpwi r30,0x0 / beq .L_000002D0                      (ONE beq)
```

Every other instruction in the function -- the `__destroy_arr` call for
`mUnusedAnim`, the `smdl_c`/`dHeapAllocator_c` destructor calls for our own
members, the `smdl_c`/`mHeapAllocator_c`/`dWmActor_c` destructor calls for the
inlined `dWmDemoActor_c` base portion, the `operator delete` guard -- matches
byte-for-byte. This single dead instruction is why the reported diff count
(21) is so much larger than the real defect (1): `verify_anon.py`'s comparison
is POSITIONAL (see its own source), so one missing instruction cascades into
every subsequent position reading as "differing."

**Not resolved.** The double `beq` is inert (both branches test the same
never-modified condition and go to the same target, so it changes nothing at
runtime) which is exactly why it's hard to derive from source: there is no
behavioural difference to reverse-engineer, only a compiler emission quirk.
Confirmed `dHeapAllocator_c : public mHeapAllocator_c`
(`include/game/bases/d_heap_allocator.hpp`) while chasing one hypothesis (that
the inheritance relationship between our own allocator member's type and the
base class's own differently-named allocator member might explain a doubled
destructor-chain emission) -- did not find a mechanism that explains the
duplicate from this alone, and did not pursue further since any fix would
require either changing the member layout (already independently confirmed
correct via three cross-checked offsets, too risky to touch for this) or
guessing blindly at source restructuring. The already-landed sibling
`daWmAntlionMng_c::~daWmAntlionMng_c()` (also `dWmDemoActor_c`-derived, also
an empty user body) has only ONE such check in its own target -- so this is
NOT a universal per-base-class artifact, it is specific to something about
`daWmAnchor_c`'s own member set (3 non-trivial own members: array + smdl_c +
dHeapAllocator_c, vs. antlion's 1: dHeapAllocator_c alone) that was not
isolated in the time available. **Recorded as a real, precisely-bounded
negative** for the next agent rather than guessed at.

### Round 2: HANDOFF's "explicit redundant null check is source-visible" lever, TESTED and it does NOT close this one

The coordinator pointed at HANDOFF ~line 9100 ("An explicit redundant null
check is SOURCE-VISIBLE - write it if the target has it", the WM_KINOPIO
`if (mpMdlMng) delete mpMdlMng;` case) and asked for a direct test: write the
same condition, checked again, in the destructor body. Two variants tried,
both rebuilt, **both measured as zero-effect**:

```cpp
daWmAnchor_c::~daWmAnchor_c() {
    if (this) {
    }
}
```
and
```cpp
daWmAnchor_c::~daWmAnchor_c() {
    if (this) {
        (void)this;
    }
}
```

Both compiled to the EXACT SAME bytes as the plain `{}` destructor - verified
by rebuild both times (`21 differing`, unchanged; `draft.txt`'s `__dt__` body
byte-for-byte identical to the pre-experiment version, single `beq` only, zero
extra instructions). MWCC eliminates the whole conditional as dead code in
both forms, empty-body or not.

**This is NOT the same situation as the WM_KINOPIO precedent**, and the
difference matters: in WM_KINOPIO the guarded statement is `delete mpMdlMng;`
- a REAL operation with an externally-visible effect (a call to
`operator delete`), so MWCC cannot eliminate the guard even though `delete`
is independently null-safe. Here, the only thing that WOULD belong inside the
guard - destruction of `dWmDemoActor_c`'s own inlined members
(`smdl_c@0x158`, `mHeapAllocator_c@0x13c`) and the call to
`dWmActor_c::~dWmActor_c()` - is not reachable as an independent, once-only
statement from `daWmAnchor_c`'s own source: it already happens automatically,
exactly once, as the implicit tail of the derived destructor. Writing it a
SECOND time explicitly (e.g. `dWmDemoActor_c::~dWmDemoActor_c();` as an
explicit statement) would not add one redundant branch - it would duplicate
the entire base-destruction block (real calls, real instructions), which is
not what the target shows (only ONE instance of that block exists, just with
an extra dead branch guarding it). There is no C++ construct available from
the DERIVED class that produces "guard present, guarded code runs once."

**Conclusion: the lever, tested directly rather than assumed, does not
transfer to this specific shape (a redundant check wrapping automatic
base-subobject destruction rather than wrapping an explicit call).** This
CONFIRMS, with a fresh empirical test rather than a re-statement, HANDOFF's
earlier "duplicate-`beq` destructor wall" finding (~line 8931, independently
found on this same unit and on WM_BOARD, "a derived class cannot reach a
construct emitted by inlining its base's destructor"). Reverted both
experiments; the destructor is back to the plain
`daWmAnchor_c::~daWmAnchor_c() {}` it started this session with. **Do not
retry either of the two shapes above** - both are now measured, not just
reasoned about.

**`fn_2_15ABC0`** (`li r3,0x1; blr`, 2 instructions, "1 differing" in the
table): this is a documented, INTENTIONAL exclusion carried over from a prior
round -- it is `d_a_wm_sandpillar.cpp`'s own placed copy of
`dWmDemoActor_c::doDelete()`, an unrelated weak duplicate that happens to sit
in this unit's address range. Confirmed again this round via its own target
disassembly and AGENT_CONTEXT's existing precedent. Left out of the source
entirely, as before -- no action needed.

## Order-check finding (verified, not a real blocker)

`build.py`'s `verify_anon` reports `FUNCTION ORDER IS WRONG`, flagging
`finalUpdate__12dBaseActor_cFv` (target `0x15aba0`, our `vf74()`) as "defined
too late." **This is a false positive from the greedy content-based pairing,
not a real ordering defect** -- verified directly:

- `draft.txt` contains **TWO separate weak `.fn` entries with byte-identical
  1-instruction bodies (`blr` alone)**: `finalUpdate__12dBaseActor_cFv`
  (emitted early, right after `processCutsceneCommand`) and
  `vf74__12daWmAnchor_cFv` (emitted later, in the correct
  reverse-declaration-order position between `vf78()` and `GetActorType()`,
  exactly where the class's own doc comment says declaration order should
  place it).
- `verify_anon.py`'s pairing is greedy-by-content and picks whichever
  UNUSED candidate matches first; it happened to consume the earlier,
  mispositioned `finalUpdate__12dBaseActor_cFv` copy for the target slot
  instead of the later, correctly-positioned `vf74__12daWmAnchor_cFv` copy.
  Both are weak and byte-identical, so the linker's COMDAT folding will keep
  exactly one of them regardless of which symbol name our disassembly shows --
  this is exactly the "two functions, one body" trap `verify_anon.py`'s own
  docstring warns about (the sandpillar `__dt__Q23mEf8effect_cFv` /
  `sStateID_c` example).
- `check_fn_order.py` (which cannot see named/weak symbols at all, only
  `fn_2_ADDR`-style names) reports 0 inversions, consistent with there being
  no real defect in what we control.

**No action taken** -- there is nothing in the source to change; the six
declared-inline virtuals are already in the class's own address-matching
declaration order per the existing doc comment, and the apparent duplicate
`finalUpdate` symbol is the compiler independently instantiating the base
class's own inherited method under its OWNER's name, which is outside this
TU's control. Flagged for the lead's own judgement, not silently ignored.

## Remaining work for the next agent

- Destructor's single redundant `beq` (see above) -- the only function still
  meaningfully "wrong" in content, everything else at 19/22 is either MATCH,
  the small ctor register-scheduling residual, or the documented foreign
  exclusion.
- Constructor's 4-instruction register-scheduling residual (see above).
