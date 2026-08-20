# PEACH_CASTLE_SEQUENCE_MGR / PEACH_CASTLE_SEQUENCE_MGR_OBJ -- d_basesNP, module 2

Unit: `.text 0x1204E0-0x120F00` (0xA20 bytes), 44 target functions (some are
compiler-generated template instantiations pulled in by `sFStateMgr_c`, not
hand-written). Draft: `d_a_peach_castle_seq.cpp`, header:
`shadow_include/game/bases/d_a_peach_castle_seq.hpp`.

## Classes

- `daPeachCastleSequenceMgr_c : public dActor_c` -- trivial manager. Ctor
  spawns the OBJ as its child (`fBase_c::createChild(fProfile::
  PEACH_CASTLE_SEQUENCE_MGR_OBJ, this, 0, 0)`) and stores the result in
  `daPeachCastleSequenceMgrObj_c::m_instance` (a `.bss` singleton pointer,
  `lbl_2_bss_D8F8`). `sizeof == 0x398` (read off `li r3, 0x398` at 0x1204E8).
  `create()` trivial; `doDelete()` deletes+clears the singleton;
  `execute()` calls `ActorScrOutCheck(SKIP_ACTOR_DELETE)` for its side effect
  only (result discarded).

- `daPeachCastleSequenceMgrObj_c : public dBase_c` -- **NOT dActor_c**. Its
  ctor (fn_2_120630) calls `__ct__7dBase_cFv` directly (verified by direct
  disassembly), contradicting `BSS_SINGLETONS.md`'s row for this singleton
  which lists base `dActor_c` -- **that row appears to be wrong; flagging it,
  not silently overriding it.** `sizeof == 0xB8` (`li r3, 0xB8` at 0x120518,
  matches the doc). Embeds `sFStateMgr_c<daPeachCastleSequenceMgrObj_c,
  sStateMethodUsr_FI_c> mStateMgr` at `+0x70` (ends `+0xac`, matches the
  ctor's own field stores exactly). One state, `Wait`
  (name string `daPeachCastleSequenceMgrObj_c::StateID_Wait` read directly out
  of `.data` at file offset 0x51b4 in `auto_04_000343E0_data.o`). Watches for
  the Peach's Castle end-of-world cutscene to end
  (`dScStage_c::m_isOtehonReturn`), then after a short countdown starts the
  world-map "control demo" (`daPyDemoMng_c`) and fires
  `dGameCom::ModelPlayMenuStart()`.

  Own fields after `mStateMgr` (`+0xac..+0xb5`): `int mPhase` (`+0xac`,
  0=idle/1=counting), `int mCountdown` (`+0xb0`, frames), `bool mTriggered`
  (`+0xb4`).

## Shadow header additions (proposed, not landed)

- `shadow_include/game/bases/d_game_com.hpp`: adds `dGameCom::
  ModelPlayMenuStart()` (mangled `ModelPlayMenuStart__8dGameComFv`, `void`,
  no args -- called via `bl`, return value unused by the caller).
- `shadow_include/game/bases/d_s_stage.hpp`: adds `dScStage_c::
  m_isOtehonReturn` (`static bool`, read-only in this TU via plain
  `lbz`+`cmpwi`, so no neg/or/srwi canonicalisation tail -- placement
  relative to the class's other statics NOT re-verified, appended rather than
  interleaved).
- `shadow_include/game/bases/d_a_peach_castle_seq.hpp`: the two classes
  themselves (not landed anywhere yet).

## Verified facts (direct disassembly / dtk symbol table / byte diff)

- `fProfile::PEACH_CASTLE_SEQUENCE_MGR_OBJ == 0x168` -- read directly off
  `li r3, 0x168` at 0x120568 (the profile ID passed to `createChild`), cross-
  checked against `g_profile_PEACH_CASTLE_SEQUENCE_MGR_OBJ`'s own packed
  `0x0168` id word in `.data`.
- `dScene_c::m_nextScene == fProfile::INVALID` guard in `demoStart()`:
  `0x2ee` resolved by counting `fProfile::PROFILE_NAME_e` entries
  (`PROFILE_COUNT`/`INVALID` sits at enum index 749, i.e. value 0x2ee) --
  landed code (`d_scene.cpp`) already uses `fProfile::INVALID` for this exact
  comparison shape, so that's the name used here too.
- `daPyDemoMng_c::m_58`/`m_94` are `public int` per the already-landed
  `include/game/bases/d_a_player_demo_manager.hpp` -- both written as plain
  ints (1/0), matching the target's `stw`, not a bool-canonicalising store.
- The `executeState_Wait` body is a genuine **switch**, not an if/else-if --
  confirmed empirically: the if/else-if source produced a "compare + skip,
  shared merge label" shape (26 differing instructions vs target), while a
  `switch (mPhase) { case 0: ...break; case 1: ...break; }` produces the
  target's exact "independent compare-and-branch per case, explicit final
  `b end`" shape. Byte-identical after the rewrite.
- `initializeState_Wait`'s reported "1 differing" (a trailing `blr` after the
  `if (...) demoStart();` tail-call) is a **confirmed tool artifact**, not a
  content defect -- same class of issue HANDOFF.md already documents for
  sandpillar ("the extra trailing blr was a TOOL ARTEFACT", dtk splits an
  unreachable `blr` after an unconditional branch into its own labelled pseudo
  -function). Proof: target's `fn_2_120970` (8 instr, ends `b`) concatenated
  with the immediately-following `fn_2_120990` (1 instr, `blr`) is
  **byte-identical, instruction for instruction**, to the draft's single
  9-instruction `initializeState_Wait`. The existing auto-forgiveness in
  `verify_anon.py` only fires when the target's last instruction is `bctr`;
  ours ends in a direct `b`, so the forgiveness doesn't trigger and it still
  reports as a difference. This is a tool-side gap, not something fixable
  from this unit's source.

## OPEN: function-order gate (NOT resolved)

`verify_anon.py` reports these 7 as "defined too late", all template
instantiations of the SAME state-machine framework the class embeds
(`sFStateFct_c<T>::build/dispose`, `sFState_c<T>::initialize/execute/
finalize`, `sStateMgr_c<T,...>::initializeState/finalizeState`). Their
CONTENT is byte-identical (confirmed MATCH); only their emission POSITION
relative to `changeState`/`refreshState`/the getters (which land correctly)
is wrong.

Attempts made, all unsuccessful or net-negative (each reverted cleanly, repo
is back at this same 43/44 baseline):
1. Moved `STATE_DEFINE(Wait)` from end-of-file to top-of-file (dYesNoWindow_c
   convention) -- zero effect, byte-for-byte identical draft.txt.
2. Reordered class member declarations (`STATE_FUNC_DECLARE`+`mStateMgr`
   before vs. after the private helper method declarations) -- zero effect.
3. `dActorState_c`-style inline unused `dummy()` method calling
   `mStateMgr.initializeState()`/`finalizeState()` -- zero effect (an unused,
   never-ODR-used inline apparently isn't enough to force instantiation
   ordering the way `dActorState_c`'s own `dummy()` presumably does for ITS
   callers).
4. `template class sStateMgr_c<daPeachCastleSequenceMgrObj_c,
   sStateMethodUsr_FI_c, sFStateFct_c, sStateIDChk_c>;` (explicit whole-class
   instantiation) -- DID move build/dispose/initialize/execute/finalize into
   the correct position, but as a side effect ALSO pulled `changeState`
   (previously correct) into the same wrong batch, netting the same 7 wrong
   (different membership) AND synthesized an extra, non-existent
   `sStateMgr_c` constructor not present in target. Reverted.
5. Explicit MEMBER (not whole-class) instantiation of exactly the 7 needed
   methods -- compiled cleanly but had **zero effect** on those 7, and as a
   side effect newly broke the previously-correct `sFStateID_c<T>`
   ptmf-dispatch trio (`initializeState`/`executeState`/`finalizeState` at
   0x120C70-0x120CD0). Reverted.

This smells like a genuine MWCC template-instantiation-batching quirk that
isn't controlled by any straightforward source reordering tried so far. Did
NOT try: checking actual symbol st_info/binding on a REAL (non-auto-split)
object -- the auto-split `.o` files here report `bind=1` (GLOBAL) uniformly
for every `fn_2_*` symbol regardless of true origin, so that check (which
resolved WM_KOOPAJR) isn't usable against these particular target files.

6. `template class sStateMgr_c<daPeachCastleSequenceMgrObj_c,
   sStateMethodUsr_FI_c, sFStateFct_c, sStateIDChk_c>;` ALONE (no member
   pre-instantiation, no ctor side effect since only used as a class-not-
   object instantiation) -- this is the closest partial result found:
   `build`/`dispose`/`initialize`/`execute`/`finalize` (the 5 NESTED-template
   members) become byte- and position-CORRECT. But `refreshState` and the 4
   getters (previously correct in the untouched baseline) get swept into the
   same wrong late slot as `initializeState`/`finalizeState` -- net still 7
   wrong, different membership (now exactly `sStateMgr_c`'s own 7 methods
   minus `changeState`/`executeState`, instead of the mixed nested+own-2
   set). A pre-instantiation of `changeState` alone before this had zero
   additional effect (it was already correctly positioned either way).
   Re-instantiating `refreshState`/getters explicitly AFTER the whole-class
   line to try to "re-lock" them is a hard compile error (10333, redefinition
   -- explicit instantiation is one-shot per entity). Reverted; current
   source has NO explicit instantiation lines at all (cleanest state, the
   43/44 + 7-flagged baseline from attempts 1-5 above).
7. `template class sFStateFct_c<daPeachCastleSequenceMgrObj_c>;` +
   `template class sFState_c<daPeachCastleSequenceMgrObj_c>;` together
   (skipping `sStateMgr_c` itself entirely) -- worse: 10 flagged, newly
   breaking the previously-correct `sFStateID_c<T>` ptmf trio
   (`initializeState`/`executeState`/`finalizeState` at 0x120C70-0xCD0) on
   top of the original 7. Reverted.

**Net conclusion:** explicit instantiation IS a real lever (attempt 6 proves
it moves things), but every combination tried either leaves the same 7-count
wrong (different membership) or makes it worse. The one encouraging data
point for a future attempt: attempt 6's result differs from the untouched
baseline in exactly the RIGHT direction for 5 of 7 -- the remaining gap is
purely `sStateMgr_c`'s OWN `initializeState`/`finalizeState`, which in the
TRUE target sit grouped with the nested build/dispose/sFState block (not with
their own class's `refreshState`/getters, and not with `changeState`). Worth
trying next: an explicit instantiation containing ONLY `initializeState`+
`finalizeState` as individual members (not the whole class), placed at the
SAME point attempt 6 used -- not yet tried in isolation without either the
whole-class line or the other 5 members alongside it.

## Tally

- 43/44 byte-identical by `verify_anon.py`'s own count; the 44th
  (`initializeState_Wait`) is a confirmed tool artifact (see above) --
  **44/44 real content match**.
- `.ctors`: exactly one entry for this profile
  (`python wip/wm_units/ctors_map.py d_basesNP PEACH_CASTLE` ->
  `0x2e4 -> 0x120d00 -> g_profile_PEACH_CASTLE_SEQUENCE_MGR_OBJ`), matching
  the established fact.
- Function order gate: **FAILING**, 7 functions, all template
  instantiations, content-correct but wrong emission position. See above.

## Round 2: followed the lead's precedent-check guidance directly

Checked `d_pausewindow.cpp`/`Pausewindow_c` and `d_CourseSelectGuide.cpp`/
`dCourseSelectGuide_c` structure in detail (both directly embed their own
unique `sFStateMgr_c<Self, sStateMethodUsr_FI_c>`, matching our shape, unlike
the inherited-`dActorState_c` classes below). Findings:

- **`fProfile::PROFILE_COUNT`/`INVALID` off-by-nothing correction**: none
  needed here, this was already right.
- **Content fix, unrelated to order**: moving `STATE_DEFINE(Wait)` to sit
  textually AFTER `daPeachCastleSequenceMgrObj_c`'s own constructor (matching
  `dCourseSelectGuide_c`'s file order: `m_instance` def, ctor, THEN
  `STATE_DEFINE` block) took the draft from 43/44 to **44/44** raw
  byte-identical (previously the `initializeState_Wait` trailing-`blr` tool
  artifact was the only gap; the coordinator has since said they'll fix
  `verify_anon.py`'s `bctr`-only forgiveness, which likely also explains this
  independently). Order gate: **unchanged**, same 7 flagged, regardless of
  which of 4 different `STATE_DEFINE` positions tried (top-of-file /
  right-after-`ACTOR_PROFILE`, right-after-OBJ-ctor, right-after-
  `OBJ::execute()`, and back to top) -- **all four positions produce
  byte-identical draft.txt**. `STATE_DEFINE`'s textual position is
  conclusively NOT the lever for the order gate (proven negative, not just
  untried).
- **Declaration order of `mStateMgr` vs `STATE_FUNC_DECLARE` in the header**:
  `Pausewindow_c`'s header (and `dYesNoWindow_c`'s) declares
  `STATE_FUNC_DECLARE` BEFORE `mStateMgr`; `dCourseSelectGuide_c`'s header
  declares its 8 `mStateMgrXxx` members BEFORE its 32 `STATE_FUNC_DECLARE`s --
  i.e. the landed files themselves are NOT consistent with each other on this
  point, which is itself evidence this ordering doesn't matter. Tried both
  ways against our unit; zero effect either way, confirming the landed
  files' inconsistency.
- **Out-of-line vs. inline destructor -- tried, made things WORSE, reverted,
  but is informative.** `Pausewindow_c` declares `Pausewindow_c();
  virtual ~Pausewindow_c();` in the header and defines
  `Pausewindow_c::~Pausewindow_c() {}` explicitly out-of-line in the .cpp,
  positioned immediately after the constructor. Copying that exact structure
  for both our classes (out-of-line empty dtor right after each ctor, instead
  of the inline `~X() {}` in the header) regressed 44/44 -> 44/44 content
  (unaffected) but **the order gate went from 7 flagged to 21 flagged** --
  forcing the dtor out-of-line at that early point drags the ENTIRE virtual
  set (both class dtors' vtable-adjacent groups, `changeState`, the nested
  build/dispose/sFState group, ALL of `sStateMgr_c`'s own methods, even
  `__sinit`) into one giant early batch, which does not match target at all.
  **Conclusion, not just observation:** in the TRUE target, `changeState`
  (0x120B20) sits AFTER both classes' own dtors (0x120A40, 0x120AA0) --
  meaning the real source's dtors do NOT trigger the vtable group early
  either, i.e. **the real source almost certainly also uses inline `{}`
  dtors, not `Pausewindow_c`'s out-of-line convention.** Reverted to inline;
  this is the correct choice for this file, confirmed by elimination rather
  than assumed.
- **The two 1-`STATE_DEFINE` files found (`d_a_right_base.cpp`,
  `d_a_spin_child_base.cpp`) are NOT true analogues.** Both are
  `: public dActorState_c`, and `dActorState_c` embeds
  `sFStateMgr_c<dActorState_c, sStateMethodUsr_FI_c> mStateMgr` -- templated
  on `dActorState_c` ITSELF, not on the derived class. Every class that
  inherits `dActorState_c` therefore shares the SAME, already-elsewhere-
  instantiated `sFStateMgr_c<dActorState_c,...>` (compiled once, wherever
  `dActorState_c`'s own machinery lives) -- their TUs never need to
  instantiate `build`/`dispose`/etc. themselves at all, so they can't exhibit
  (or fix) our problem. Also both have real, executed `changeState()` calls
  (state transitions), unlike our single-state, no-transition class. Every
  file that DOES embed a `sFStateMgr_c<Self,...>` uniquely templated on
  itself (`Pausewindow_c` 6 states, `dCourseSelectGuide_c` 32, `dYesNoWindow_c`
  8) also has real `changeState()` call sites from state transitions --
  **there is no landed file with a single, non-transitioning state and a
  self-templated `sFStateMgr_c`.** This may be a genuinely novel
  configuration for the project.

Current source has no explicit-instantiation lines and matches the simpler of
the two structural conventions found in landed code (inline dtors,
`STATE_FUNC_DECLARE` before `mStateMgr`, `STATE_DEFINE` at top-of-file after
`ACTOR_PROFILE`) -- reproducibly 44/44 content, same 7 order-flagged
functions every time.
