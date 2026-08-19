# WM_MANTA mapping and draft

Unit: `.text 0x170eb0-0x17140c` in `d_basesNP` (0x55c bytes, 16 functions).
Class name chosen: `daWmManta_c : public dWmDemoActor_c`.

## Base class

Ctor calls `bl __ct__14dWmDemoActor_cFv` directly, and (measured, not
assumed) the class's own first field lands at offset `0x184` -- exactly
`sizeof(dWmDemoActor_c)` per the same STATIC_ASSERT technique used on
dance_pakkun. This rules out `dWmObjActor_c` (would push the first own
field to `0x188`, since it adds `int mResNodeIdx`) and `dWmEnemy_c` (far
bigger). This is offset/layout evidence, not just code-shape inference from
the `bl` chain.

## Function map (all 16)

| addr | size | role |
|---|---|---|
| 0x170eb0 | 0x30 | classInit (auto, ACTOR_PROFILE macro) |
| 0x170ee0 | 0x74 | ctor |
| 0x170f60 | 0xac | dtor |
| 0x171010 | 0x68 | create() -- createModel(); mClipSphere.set(mPos, 250.0f); startStep(); calcModel(&mModel); return SUCCEEDED; |
| 0x171080 | 0x8c | execute() -- processCutsceneCommand(cutName, isFirstFrame) (virtual, through the secondary vtable -- same this+0x60/+0x60 mechanism proven on dance_pakkun), a 1-entry PTMF-table dispatch indexed by m_224, then calcModel(&mModel) |
| 0x171110 | 0x30 | draw() -- mModel.entry(); return SUCCEEDED; (vtable+0x14, same slot confirmed on dance_pakkun) |
| 0x171140 | 0x8 | doDelete() -- return SUCCEEDED; trivial |
| 0x171150 | 0xac | createModel() |
| 0x171200 | 0xbc | calcModel(m3d::mdl_c *mdl) -- the SIMPLE ghost-style shape (mMatrix.trans/ZXYrotM + setLocalMtx/setScale/calc), no per-frame table blend at all (unlike dance_pakkun's calcModelFor) |
| 0x1712c0 | 0x18 | startStep() -- mScale = {1.8,1.8,1.8} (measured), tail-calls resetStep() |
| 0x1712e0 | 0xc | resetStep() -- m_224 = 0; |
| 0x1712f0 | 0x4 | unusedStub() -- empty, the PTMF table's one entry |
| 0x171300 | 0x18 | processCutsceneCommand(int, bool) -- overridden (unlike dance_pakkun, which didn't): if (cutsceneCommandId != dCsSeqMng_c::CUTSCENE_CMD_NONE) { setCutEnd(); }, calling the inherited virtual setCutEnd() (not writing mIsCutEnd directly like dokan does) -- confirmed by the vtable+0x68 dispatch shape |
| 0x171318 | 0x4 | dBaseActor_c::finalUpdate() -- not authored, it's the inherited weak empty default, pulled in automatically |
| 0x171320 | 0xdc | countModelVariants() -- @unofficial name/purpose. Builds a "CS_W%X"-formatted per-world archive name (measured string, world+1 in hex), loads "g3d/model.brres" (measured, generic path) from it, then loops GetResMdl with an appended single letter ('a','b',...) until one fails, returning the count |
| 0x171400 | 0xc | getWorldNo() -- @unofficial name. return dScWMap_c::m_WorldNo; -- confirmed as a direct field read, not a call to the existing dScWMap_c::getWorldNo() (which is NOINLINE and would show as a bl) |

## Real strings and constants, measured directly from original/d_basesNP.rel

Per the coordinator's note (.data at file offset 0x1d0c00 + address,
.rodata at 0x1c6600 + address), read directly with Python rather than
guessing:

```
.data 0x46384 (0x11) = "g3d/togezo.brres\0"
.data 0x46398 (0x7)  = "togezo\0"
.data 0x46474 (0x7)  = "CS_W%X\0"
.data 0x46480 (0x10) = "g3d/model.brres\0"
.rodata 0x8d70 (float) = 1.7999999523162842 (1.8f)
.rodata 0x8d74 (float) = 250.0
.rodata 0x8d78-0x8d84  = PTMF entry {0, -1, <reloc>}, matching dance_pakkun's shape exactly
```

**The internal model/archive name is "togezo", not "manta".** This
surprised me enough to double-check it wasn't a bounds mistake (see below)
-- it's a real dev-name mismatch with the WM_MANTA profile enum, not a
misattribution.

## Section bounds

```
python wip/wm_units/check_bounds.py d_basesNP '{".text": "0x170eb0-0x17140c", ".data": "0x46378-0x46410", ".rodata": "0x8d70-0x8d88"}'
```
Result: clean on .text and .rodata. .data reports the family-rule
warning ("begins at a PROFILE symbol, very likely 0x34 too high") but I
investigated this explicitly rather than overriding it silently:

- The unit immediately before ours in .data is WM_KOOPASHIP
  (g_profile_WM_KOOPASHIP at 0x45fd8), which owns a large block of its
  own strings running all the way to 0x46378 (g_profile_WM_MANTA) with
  no two 5-byte strings anywhere in that span that could be ours.
- The family rule is specifically about units that use
  dWmLib::sc_ForceList (which opens with two 5-byte strings pointed to
  by its ForceInCourseList_t). Manta's 16 functions never reference
  dWmLib at all. The rule doesn't apply here because the precondition
  (using sc_ForceList) doesn't hold, not because the claim is wrong.
- .data 0x46378-0x46410 (g_profile_WM_MANTA + the two model-name
  strings + the secondary vtable) reports completely clean ownership --
  every symbol inside it is referenced from inside our own .text.

**One unresolved, reported-not-forced oddity**: countModelVariants()
(fn_2_171320) references lbl_2_data_46474 ("CS_W%X") and
lbl_2_data_46480 ("g3d/model.brres"), both at 0x46474-0x46490 --
outside the contiguous 0x46378-0x46410 claim, with 0x46410-0x46474
in between confirmed (via the ownership check, referrers at addresses like
0x12650/0x1757f2, nowhere near our .text claim) to belong to
WM_MAP, a separate, much larger, not-yet-landed unit. I re-verified my
own transcription of the lis/addi operands twice against the raw
disassembly before accepting this. I do not have an explanation for why
our own .data would be split around a foreign unit's chunk -- REL slice
files in this project only support one contiguous range per section
(checked: every existing d_basesNP.json entry has single-range values),
so this may need the lead's attention if it holds up. Reporting it
plainly rather than picking one range and hiding the other reference.

No .ctors/.bss claim needed -- unlike dance_pakkun, nothing in these 16
functions shows a guard-byte/static-local pattern.

## Build result

wip/wm_units/agent_manta/build.py, verify_anon against
bin/dtkspl/d_basesNP/obj/auto_00_00170E8C_text.o:

```
classInit MATCH | ctor MATCH | dtor MATCH | create() MATCH | execute() MATCH
draw() MATCH | doDelete() MATCH | createModel() MATCH | calcModel() MATCH
startStep() MATCH | resetStep() MATCH | unusedStub() MATCH
processCutsceneCommand() MATCH | finalUpdate() MATCH (inherited, unauthored)
countModelVariants() 19 differing (size 51 vs target 55)
getWorldNo() MATCH

15/16 byte-identical
```

## What worked immediately (reusing dance_pakkun's proven levers)

- The this+0x60 secondary-vtable mechanism needed zero manual code --
  declared the class normally and both execute()'s virtual
  processCutsceneCommand dispatch and the ctor's vtable store came out
  byte-exact on the first compile.
- draw()'s mModel.entry() (vtable+0x14) matched immediately, reusing
  the slot confirmed by compiling entry() vs play() on the dance_pakkun
  unit.
- The 1-entry PTMF table pattern (typedef void
  (daWmManta_c::*ProcFunc_t)(); const ProcFunc_t sProcTable[1] = {
  &daWmManta_c::unusedStub };) matched immediately, reusing the exact
  shape identified on dance_pakkun.
- mResFile's ctor handling: first attempt used an explicit
  mResFile = nw4r::g3d::ResFile(); (guessing it needed the same
  explicit zero dance_pakkun's ctor didn't need), which cost an extra
  saved register (r30) the target doesn't use -- ctor came out 28
  differing. Removing the explicit initialisation entirely (leaving the
  member's own default construction to zero it) closed the ctor to a
  MATCH. Negative, then fixed: dance_pakkun's mResFile didn't need
  touching either; I should have tried the empty-body ctor FIRST instead
  of assuming the explicit-zero shape from the raw disassembly's
  `li r0,0; stw r0,0x1a4` -- that store is emitted by the type's own
  default construction, not something the ctor body needs to spell out.

## Negatives

- countModelVariants() (19 differing, size 51 vs target 55) -- my least
  confident reconstruction. Confirmed one real bug (the IsValid()
  branch sense is inverted relative to target -- target's first check
  branches on equality where mine branches on inequality) but did not
  chase it further given this function's semantics are still genuinely
  uncertain (the loop's real buffer-reuse-plus-single-byte-suffix shape
  differs from my two-separate-snprintf-calls reconstruction -- target
  writes ONE letter byte into a fixed offset of an already-built buffer
  per iteration rather than re-running snprintf).
- The disjoint .data reference from countModelVariants()
  (0x46474/0x46480, separated from the main 0x46378-0x46410 claim by
  a confirmed-foreign WM_MAP chunk) is reported, not resolved. The slice
  JSON format doesn't appear to support a non-contiguous claim.
- Did not chase the FUNCTION ORDER IS WRONG warning verify_anon
  raises for getWorldNo(). Diagnosed it precisely: it's an artifact of
  compiling this TU in isolation. dBaseActor_c::clearCutEnd(),
  checkCutEnd(), GetActorType(), and m3d::anmChr_c::getType() are
  all inherited weak virtuals my class doesn't override; the REAL,
  fully-linked binary evidently resolves them from some OTHER,
  already-linked TU (this unit apparently isn't the first one in link
  order to need them), so the target's own copy of THIS TU only contains
  local weak copies of setCutEnd() and finalUpdate() -- the two it's
  first to need. My isolated compile can't know about other TUs' weak
  copies and must emit local copies of everything my class's vtables
  need, which pushes those four extra weak symbols to the very end of my
  object, past countModelVariants()/getWorldNo(), breaking definition
  order relative to the target. Confirmed by experiment: temporarily
  removing countModelVariants()/getWorldNo() made the warning
  disappear entirely (the weak block relocates immediately after
  processCutsceneCommand(), landing finalUpdate() at the exact right
  address) -- proving the extra weak pulls, not my own two functions'
  positions, are the cause. This is a real fact about isolated-draft
  compilation, but I don't know whether it would still block a real
  link alongside the rest of the (mostly not-yet-landed) project.
