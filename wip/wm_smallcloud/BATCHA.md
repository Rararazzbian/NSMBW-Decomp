# Batch A report -- d_a_wm_smallcloud.cpp, first eight functions

Scope: `0x1797E0`-`0x179D44` (8 functions: `0x1797E0`, `0x179810`, `0x179890`,
`0x179980`, `0x179A80`, `0x179B70`, `0x179BA0`, `0x179BB0`).

Working files:
- Draft source: `wip/wm_smallcloud/scratch/batchA/draft.cpp`
- Shadow class header (scratch only, NOT for landing as-is):
  `wip/wm_smallcloud/scratch/batchA/include/game/bases/d_a_wm_smallcloud.hpp`
- Compile driver using the SLICE's real flags (`-sdata 0 -sdata2 0 -O4,p
  -char signed -rtti off`, not `harness.py`'s generic `-O4`/`-RTTI off`):
  `wip/wm_smallcloud/scratch/batchA/build.py`
- Target objects: `bin/dtkspl/d_basesNP/obj/auto_00_001797B4_text.o` (covers
  `0x1797B4`-`0x179F40`, i.e. all of this batch plus the start of batch B's).
- Twin reference: `d_a_wm_cloud.o` disassembled to
  `wip/wm_smallcloud/scratch/batchA/cloud_compiled.txt`.

## Class reconstruction (state this first, per the brief)

**`daWmSmallCloud_c` is a near-exact structural twin of the landed
`daWmCloud_c`, minus the per-node culling machinery.** Concretely, measured
from three independently byte-matching functions (constructor, destructor,
`create()`), not guessed:

- `daWmSmallCloud_c : public dWmObjActor_c` -- identical base to `daWmCloud_c`.
  Proven by the constructor: `bl __ct__14dWmDemoActor_cFv`, then the SAME
  literal `-1` store at `0x184` (this is `dWmObjActor_c::mResNodeIdx`,
  inlined per `AGENT_CONTEXT.md`'s "`-inline noauto` still inlines a member
  defined IN the class body" rule -- `dWmObjActor_c()`'s ctor is defined
  in-class), then the SAME own-vtable store at `0x60` that `daWmCloud_c`'s
  ctor does. Both are automatic compiler output for this base chain, not
  something either unit's source writes explicitly.
- Field layout, offsets **identical to `daWmCloud_c`** through `mChrAnim`:
  `u32 mUnk188` (`0x188`), `dHeapAllocator_c mAllocator` (`0x18c`),
  `nw4r::g3d::ResFile mResFile` (`0x1a8`), `m3d::smdl_c mModel` (`0x1ac`),
  `m3d::anmChr_c mChrAnim[ANIM_COUNT]` (`0x1b8`, `ANIM_COUNT == 1`, same as
  the twin -- `__construct_array` count operand is `1` in both). Proven by
  the constructor/destructor's addresses matching the twin's ctor/dtor
  address-for-address (`0x184`, `0x18c`, `0x1a8`, `0x1ac`, `0x1b8`, array
  element size `0x38`) with zero deviation.
- **After `mChrAnim`, `daWmSmallCloud_c` DROPS `daWmCloud_c`'s
  `mGroupNodeIds[NODE_COUNT]` (`int[20]`, `0x50` bytes) and
  `mCurrNodeClipSphere` (`mSphere_c`, `0x10` bytes) -- 0x60 bytes total.**
  This is the whole reason this unit needs no per-node culling function at
  all (see below). Evidence, all measured via compiled/matched code, not
  inferred:
  - `mCurrProc` is at `0x1f4` in **both** classes (confirmed for the twin by
    reading its `execute()`; confirmed for this class by my own `execute()`
    matching byte-exact at that same offset).
  - `mpBgmSync` is at `0x1f8` here (my `create()`/dtor both match using that
    literal offset) vs `0x258` in the twin -- a difference of exactly `0x60`.
  - The whole object's size is `0x1fc` here (the `li r3, 0x1fc` operand in
    `classInit`, matched byte-exact) vs `0x25c` for the twin -- again exactly
    `0x60` smaller.
  - `0x1f4 (mCurrProc) + 4 = 0x1f8 (mpBgmSync)` with **no gap** -- so nothing
    sits between them. `0x1fc (total) - 0x1f8 (mpBgmSync) - 4 (pointer) = 0`
    -- so nothing sits after `mpBgmSync` either. The class is exactly:
    everything through `mChrAnim` (byte-identical to the twin) + `mUnk1f0`
    (pad) + `mCurrProc` + `mpBgmSync`, full stop.
  - Consequently there is **no `sGroupNodeNames` table, no `calcCulling()`,
    no `initGroupNodeIds()`, no `hideNode`/`showNode`** in this class --
    functionally replaced by a single `updatePos()` (`0x179F10`, batch B's)
    that repositions the whole actor to ONE named world-map node per frame,
    rather than culling several named bone groups.
- `ACTOR_PROFILE(WM_SMALLCLOUD, daWmSmallCloud_c, 0)` -- `WM_SMALLCLOUD`
  already exists as an `fProfile` enumerator immediately after `WM_CLOUD`
  (`include/game/framework/f_profile_name.hpp:685`), and
  `g_profile_WM_SMALLCLOUD` is a real, already-linked `.data` symbol in this
  exact TU's data (`bin/dtkspl/d_basesNP/obj/auto_04_00046BE0_data.txt:549`,
  `.4byte fn_2_1797E0` as its first word -- i.e. `classInit`'s address,
  confirming both the profile name AND that `fn_2_1797E0` is `classInit`).
- **`0x179BB0` (`createModel()`) uses a per-instance selector,
  `ACTOR_PARAM(CourseNo)` (already declared by `dWmObjActor_c`,
  `ACTOR_PARAM_CONFIG(CourseNo, 0, 8)`), to index a 4-entry table of model
  names.** Evidence: `lwz r0, 0x4(r27)` (`fBase_c::mParam`, first field after
  `mUniqueID`) then `clrlslwi r0, r0, 24, 2`, which is exactly
  `ACTOR_PARAM_LOCAL(mParam, CourseNo) << 2` for `PARAM_CourseNo == (0<<8)|8`
  (offset 0, size 8 -- no shift, just mask-and-times-4 for a `const char*[4]`
  index). Confirmed against real bytes: writing `resMdlNames[ACTOR_PARAM
  (CourseNo)]` in source reproduced `clrlslwi r0, r0, 24, 2` **exactly**, and
  the 4 real string literals decoded from `.data` (see per-function section
  below) are:
  `"CS_W7_MoveCloud01"`, `"CS_W7_MoveCloud02"`, `"CS_W7_MoveCloud03"`,
  `"CS_W6aCloud"`.

### Contradiction to flag (Rule 4), not silently resolve

Same one **batch B independently found** in their own report -- cross-batch
convergence, worth stating loudly: `daWmCloud_c`'s own header
(`include/game/bases/d_a_wm_cloud.hpp`) names the pad field right before
`mCurrProc` `mUnk250`. By this codebase's own naming convention
(`mUnk188` genuinely sits at `0x188`), that name should mean offset `0x250`.
But `mCurrProc` is measured at `0x1f4` in the twin's own `execute()`
(`lwz r0, 0x1f4(r30)`), and C++ member order within one access block is
fixed, so a field declared textually *before* `mCurrProc` cannot be at a
*higher* address. The name `mUnk250` is almost certainly stale (predates a
correction to `ANIM_COUNT` or an earlier field's size and was never renamed).
I have not touched `d_a_wm_cloud.hpp` -- flagging for the lead. My own
class's equivalent field is named `mUnk1f0` (matches its real, measured
offset) to avoid repeating the same trap.

### Correction to batch B's guess

Batch B's shadow header (written before seeing this batch A analysis, per
their own "Cross-batch corroboration" section) carries `int
mGroupNodeIds[NODE_COUNT]`, `mSphere_c mCurrNodeClipSphere`, and
`dWmBgmSync_c *mpBgmSync` all marked "not exercised by batchB" -- i.e. copied
from the twin as an unverified placeholder. **This batch has hard evidence
those first two fields do not exist** (see the `0x60`-byte size-difference
argument above, confirmed by three independently matching functions:
constructor, destructor, `create()`, and `classInit`'s own `0x1fc` size
operand). `mpBgmSync` is correct, just at `0x1f8` not wherever the twin has
it. Flagging this for the lead to reconcile when merging the two batches'
headers, per "report contradictions rather than reconciling them."

### Proposed header (full, `@unofficial` markers per field)

```cpp
#pragma once

#include <game/bases/d_heap_allocator.hpp>
#include <game/mLib/m_3d/anm_chr.hpp>
#include <game/mLib/m_3d/smdl.hpp>
#include <game/bases/d_wm_obj_actor.hpp>
#include <game/bases/d_wm_bgm_sync.hpp>

/**
* @brief The actor for the small decorative clouds used in the World 7 map.
* @details A near-twin of #daWmCloud_c: a single-bone animated cloud model, synchronised to the
* background music via #mpBgmSync, with no per-node culling (unlike daWmCloud_c, which culls each
* of its named bone groups individually every frame).
* @unofficial Reconstructed from anonymous (unnamed) target symbols; class name, member names and
* the exact GlobalData_t shape are inferred from codegen evidence, not from any mangled name.
* @ingroup bases
*/
class daWmSmallCloud_c : public dWmObjActor_c {
public:
    /// @brief The global configuration for the actor.
    /// @unofficial Shape inferred purely from create()'s pool-data references: the two s16[2]
    /// arrays are read with NO extra offset added after the GLOBAL_DATA base address, so they are
    /// believed to be the object's first (and, as far as this batch's evidence goes, only) members.
    /// mUnofficialPad exists ONLY to push mData past MWCC's small-data threshold under
    /// -sdata 0 -sdata2 0 is NOT actually 0 for this object at the C++ source level -- see
    /// "compiler-flag finding" below; the pad's true size/content is unconfirmed.
    struct GlobalData_t {
        s16 mBgmValueW5[2]; ///< @unofficial BGM sync value used when dScWMap_c::m_WorldNo == 5.
        s16 mBgmValue[2]; ///< @unofficial BGM sync value used otherwise.
        u8 mUnofficialPad[8]; ///< @unofficial Padding only, see struct comment.
    };

    /// @brief The available animations for this actor.
    /// @unofficial Name copied from daWmCloud_c's ANIM_e; only the single-entry shape is confirmed.
    enum ANIM_e {
        CS_W7_SmallCloud,
        ANIM_COUNT
    };

    typedef void (daWmSmallCloud_c::*ProcFunc)();

    daWmSmallCloud_c(); ///< @copydoc dWmObjActor_c::dWmObjActor_c
    ~daWmSmallCloud_c(); ///< @copydoc dWmObjActor_c::~dWmObjActor_c

    virtual int create();
    virtual int execute();
    virtual int draw();
    virtual int doDelete();

    /// @unofficial Owned by the other batch (0x179EC0); declared here only so this header is
    /// complete and the vtable shape matches.
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    void createModel(); ///< Initializes the resources for the actor.

    /// @unofficial Owned by the other batch (0x179D50).
    void calcModel();
    /// @unofficial Owned by the other batch (0x179E00). Batch B calls it initStateLike().
    void initState();
    /// @unofficial Owned by the other batch (0x179EA0).
    void init_exec();
    /// @unofficial Owned by the other batch (0x179EB0).
    void mode_exec();
    /// @unofficial Owned by the other batch (0x179F10). Batch B calls it setPosFromCourseNode() --
    /// same function, same evidence, different working name; repositions the actor to a named
    /// world map node every frame (indexed by ACTOR_PARAM(CourseNo)). daWmCloud_c has no
    /// equivalent; that unit culls named bone GROUPS instead of repositioning to a single node.
    void updatePos();

    u32 mUnk188; ///< @unused @unofficial offset 0x188, identical position to daWmCloud_c::mUnk188.
    dHeapAllocator_c mAllocator; ///< The allocator.
    nw4r::g3d::ResFile mResFile; ///< The resource file.
    m3d::smdl_c mModel; ///< The model.
    m3d::anmChr_c mChrAnim[ANIM_COUNT]; ///< The model animations.
    u32 mUnk1f0; ///< @unused @unofficial offset 0x1f0 (daWmCloud_c's equivalent field is
                 ///< misleadingly named mUnk250 -- see "contradiction to flag" above).
    PROC_TYPE_e mCurrProc; ///< The current process type. See dWmObjActor_c::PROC_TYPE_e.
    dWmBgmSync_c *mpBgmSync; ///< The background music synchronization helper, @ 0x1f8.
    // Total size 0x1fc (measured, classInit's operand). NO mGroupNodeIds/mCurrNodeClipSphere/
    // sGroupNodeNames -- see class-reconstruction section for the 0x60-byte proof.
};
```

## Verifier table (final)

Produced by `python wip/wm_units/verify_anon.py
wip/wm_smallcloud/scratch/batchA/draft.txt 0x1797e0 0x179ff0
bin/dtkspl/d_basesNP/obj/auto_00_001797B4_text.o`, compiling
`draft.cpp` with `build.py`'s slice-accurate flags first. Rows outside my
scope (`0x179D50` onward, batch B's) are included because the tool always
walks the whole `[lo, hi)` range against everything in my draft; they show
"differing" against stub bodies I wrote only so this batch's OWN functions
would compile -- expected, not a claim about batch B's territory.

```
addr       target                  size  result
0x001797e0 fn_2_1797E0               12  MATCH  <- daWmSmallCloud_c_classInit__Fv
0x00179810 fn_2_179810               31  MATCH  <- __ct__16daWmSmallCloud_cFv
0x00179890 fn_2_179890               57  MATCH  <- __dt__16daWmSmallCloud_cFv
0x00179980 fn_2_179980               62  MATCH  <- create__16daWmSmallCloud_cFv
0x00179a80 fn_2_179A80               59  MATCH  <- execute__16daWmSmallCloud_cFv
0x00179b70 fn_2_179B70               12  MATCH  <- draw__16daWmSmallCloud_cFv
0x00179ba0 fn_2_179BA0                2  MATCH  <- doDelete__16daWmSmallCloud_cFv
0x00179bb0 fn_2_179BB0              101  95 differing vs createModel__16daWmSmallCloud_cFv
0x00179d50 fn_2_179D50               44  MATCH  <- calcModel__16daWmSmallCloud_cFv
0x00179e00 fn_2_179E00               40  36 differing vs __dt__14dWmDemoActor_cFv
0x00179ea0 fn_2_179EA0                3  MATCH  <- initState__16daWmSmallCloud_cFv
0x00179eb0 fn_2_179EB0                1  MATCH  <- mode_exec__16daWmSmallCloud_cFv
0x00179ec0 fn_2_179EC0               17  MATCH  <- processCutsceneCommand__16daWmSmallCloud_cFib
0x00179f10 fn_2_179F10               10  10 differing vs init_exec__16daWmSmallCloud_cFv

11/14 byte-identical modulo symbol names
```

**7 of my 8 target functions are confirmed byte-exact matches**: `0x1797E0`
(classInit), `0x179810` (ctor), `0x179890` (dtor), `0x179980` (`create()`),
`0x179A80` (`execute()`), `0x179B70` (`draw()`), `0x179BA0` (`doDelete()`).
Only `0x179BB0` (`createModel()`, also the largest function in this batch at
101 instructions) does not close -- documented honestly below rather than
claimed. The `0x179D50`/`0x179E00`/`0x179EA0`/`0x179EB0`/`0x179EC0`/
`0x179F10` rows are batch B's functions; several happen to MATCH against my
placeholder stubs (`calcModel`, `initState`, `mode_exec`,
`processCutsceneCommand` -- because I copied the twin's bodies verbatim as
stubs and they turned out to be genuinely correct) or batch B's real,
already-landed bodies once merged, but I am not claiming credit for those;
see `wip/wm_smallcloud/BATCHB.md` for that batch's own report.

## Compiler-flag finding (matches batch B's independent discovery)

`tools/auto_decomp/harness.py`'s `CFLAGS` omit `-sdata 0 -sdata2 0` and use
`-O4` where `slices/d_basesNP.json`'s `meta.defaultCompilerFlags` specifies
`-O4,p` and `-char signed -rtti off`. Compiling `create()` under the
harness's default flags produced `dScWMap_c::m_WorldNo` and (once the
`GlobalData_t` struct is small enough) `GLOBAL_DATA` references through
`@sda21` small-data addressing that the real `.rel` build (link-time
`-sdata 0 -sdata2 0`, but the COMPILE-time flag matters too, independently)
never uses -- a spurious diff with nothing to do with the source. I compiled
everything in this batch through `wip/wm_smallcloud/scratch/batchA/build.py`,
which uses the slice's real flags directly instead of
`harness.compile_draft`. **Anyone else driving this unit through
`harness.compile_draft` without overriding `CFLAGS` will see false negatives
on every function referencing a small extern global or a small aggregate
static.**

A second, related finding: my initial 8-byte `GlobalData_t` (just the two
`s16[2]` arrays) compiled to `@sda21` addressing for `GLOBAL_DATA` itself
even under the correct `-sdata 0 -sdata2 0` compile flags applied to the rest
of the TU -- because my SCRATCH TEST FILE didn't have those flags applied
consistently at first. Once compiled under `build.py`'s flags throughout,
that resolved on its own; the `mUnofficialPad[8]` in the proposed header is a
leftover safety margin from before that was isolated and may not be needed
-- flagged as unconfirmed in the header itself.

## Per-function findings

### `0x1797E0` -- `daWmSmallCloud_c_classInit()` (compiler-generated, `CUSTOM_ACTOR_PROFILE` macro)
- Evidence: `li r3, 0x1fc; bl __nw__7fBase_cFUl; cmpwi r3,0; beq skip; bl
  fn_2_179810` -- identical shape to the twin's
  `daWmCloud_c_classInit__Fv` (`li r3, 0x25c; ...; bl __ct__11daWmCloud_cFv`),
  which is the exact expansion of `CUSTOM_ACTOR_PROFILE`'s
  `void *className##_classInit() { return new className(); }`
  (`include/game/framework/f_profile.hpp:16`).
- Proposal: `ACTOR_PROFILE(WM_SMALLCLOUD, daWmSmallCloud_c, 0);` -- no
  function body written by hand at all, it's macro-generated exactly like
  the twin's.
- Compiled: YES, MATCH (12/12 instructions).
- Confidence: high on shape (byte-exact); medium on `0` as the `properties`
  macro argument (untestable from this function's bytes alone -- the twin
  also uses `0`, and nothing in this function's instructions depends on that
  argument's value).
- Offset-perturbing: NO.

### `0x179810` -- `daWmSmallCloud_c::daWmSmallCloud_c()`
- Evidence: 31/31 instructions identical in SHAPE to
  `__ct__11daWmCloud_cFv`, and identical in every LITERAL OFFSET
  (`0x184`, `0x18c`, `0x1a8`, `0x1ac`, `0x1b8`, array size `0x38`, count
  `1`) -- see class-reconstruction section above.
- Proposal: `daWmSmallCloud_c::daWmSmallCloud_c() {}` (empty body; all
  visible work is base-class/member construction the compiler emits
  automatically).
- Compiled: YES, MATCH.
- Confidence: high (byte-exact).
- Offset-perturbing: NO.

### `0x179890` -- `daWmSmallCloud_c::~daWmSmallCloud_c()`
- Evidence: 57/57 instructions identical in shape to `__dt__11daWmCloud_cFv`,
  with the ONE literal difference being `lwz r0, 0x1f8(r3)` (mine) vs
  `lwz r0, 0x258(r3)` (twin) -- reading `mpBgmSync`, proving its offset.
- Proposal: `if (mpBgmSync != nullptr) { delete mpBgmSync; }` (identical to
  the twin's dtor body).
- Compiled: YES, MATCH.
- Confidence: high (byte-exact).
- Offset-perturbing: NO.

### `0x179980` -- `create()`
- Evidence: matches the twin's `create()` shape (allocate `dWmBgmSync_c`,
  zero-init on success, store into `mpBgmSync`, `createModel()`,
  `mClipSphere.set(mPos, radius)`, `calcModel()`, `initState()`, return
  `SUCCEEDED`) but with a genuine behavioural difference the twin does not
  have: a `dScWMap_c::m_WorldNo == 5` branch selecting between two different
  `s16[2]` pairs for `bgmSync->m_18/m_04/m_08`. Both branches' data decode to
  identical values (`{8, 0}` in both, from `.rodata` bytes
  `00 08 00 00 00 08 00 00`), but are computed DIFFERENTLY (`GLOBAL_DATA
  .mBgmValueW5[0] - 1` in the true branch vs the literal `7` in the false
  branch) -- a genuine source-level difference the compiler preserves even
  though the numeric result is the same, not a mistake on my part.
  `mClipSphere.set(mPos, 400.0f)` (vs the twin's `2500.0f`) confirmed by the
  matched pool-float instruction pattern; `dWmBgmSync_c` field offsets
  (`0x0` vtable, `0x4`, `0x8`, `0xc`, `0xd`, `0xe`, `0x18`) match
  `include/game/bases/d_wm_bgm_sync.hpp` exactly, no changes needed there.
- Proposal: see full source below.
- Compiled: YES, MATCH (62/62 instructions).
- Confidence: high (byte-exact, modulo the usual float/pooled-symbol caveat
  `harness.diff_fn`/`verify_anon.py` document: the PATTERN of pool
  references is proven, not that `400.0f` and `5`/`7`/`0` are the only
  possible literals that would compile to this pattern -- though `5` for
  `m_WorldNo` is corroborated by decoded `.rodata` content matching a `{8,0}`
  BGM pair either way, so the branch condition itself is solid).
- Offset-perturbing: NO. `mpBgmSync` at `0x1f8` matches the dtor's proven
  offset.

### `0x179A80` -- `execute()`
- Evidence: matches the twin's `execute()` shape exactly (`mpBgmSync->
  execute()`, conditional `getAnmRate`/`setRate`, cutscene-command dispatch
  via `dCsSeqMng_c::ms_instance`, `Proc_tbl` PTMF dispatch through
  `__ptmf_scall`), with ONE structural difference: where the twin calls
  `calcCulling()` (looping over `mGroupNodeIds`), this class calls
  `updatePos()` (`fn_2_179F10`, batch B's -- consistent with this class
  having no per-node culling members at all, see class-reconstruction
  section).
- Proposal: see full source below.
- Compiled: YES, MATCH (59/59 instructions).
- Confidence: high (byte-exact).
- Offset-perturbing: NO.

### `0x179B70` -- `draw()`
- Evidence: `lwzu r12,0x1ac(r3); lwz r12,0x14(r12); mtctr r12; bctrl` --
  identical to the twin's `draw__11daWmCloud_cFv` (`mModel.entry()`, a
  virtual call through `mModel`'s vtable slot `0x14`).
- Proposal: `mModel.entry(); return SUCCEEDED;` (identical to twin).
- Compiled: YES, MATCH.
- Confidence: high (byte-exact).
- Offset-perturbing: NO.

### `0x179BA0` -- `doDelete()`
- Evidence: `li r3,1; blr` -- identical to `doDelete__11daWmCloud_cFv`.
- Proposal: `return SUCCEEDED;` (identical to twin).
- Compiled: YES, MATCH.
- Confidence: high (byte-exact).
- Offset-perturbing: NO.

### `0x179BB0` -- `createModel()` -- NOT CLOSED, reporting the negative result

- Evidence for the parts I'm confident about (all confirmed via matching
  sub-sequences during iteration, not final MATCH):
  - `mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT],
    nullptr, 0x20)` -- identical to the twin's first statement.
  - A dynamic archive name built via `sprintf(buf, "CS_W%d",
    dScWMap_c::m_WorldNo + 1)` then `dResMng_c::m_instance->getRes(buf,
    "g3d/model.brres")` -- the twin instead uses the static literal
    `"CS_W7"` directly (no `sprintf`), a genuine difference: this class's
    clouds are apparently reused across more than one world, unlike the
    twin's, which is world-7-specific.
  - Model/animation name selection via `resMdlNames[ACTOR_PARAM(CourseNo)]`,
    proven by the exact `clrlslwi r0, r0, 24, 2` match (see class-
    reconstruction section). The 4-entry table content was recovered by
    decoding raw `.data` bytes at `lbl_2_data_47298`/`472AC`/`472C0`/`472D4`
    (`bin/dtkspl/d_basesNP/obj/auto_04_00046BE0_data.txt:555-615`):
    `"CS_W7_MoveCloud01"`, `"CS_W7_MoveCloud02"`, `"CS_W7_MoveCloud03"`,
    `"CS_W6aCloud"` (the last one breaks the `"CS_W7_MoveCloud0N"` naming
    pattern -- read directly off the bytes, not invented).
  - A lazily-initialised, flag-guarded static `resAnmNames[4]` array that
    copies from `resMdlNames[4]` exactly once (`lbz`/`extsb.`/`bne` guard
    matching a `static bool` pattern), reused for `GetResAnmChr`.
  - The tail (`mChrAnim[0].create(...)`, `.mPlayMode = FORWARD_LOOP`,
    `.setRate(0.0f)`, `.setFrame(0.0f)`, `mModel.setAnm(...)`,
    `dWmActor_c::setSoftLight_Map(mModel)`, `mAllocator.adjustFrmHeap()`) is
    structurally identical to the twin's `createModel()` tail.
- What's blocking closure: the target uses `_savegpr_25`/`_restgpr_25`
  (7 saved registers, `r25`-`r31`, frame `0x40`) where my best draft uses 4
  saved registers manually (`r28`-`r31`, frame `0x30`) -- 4 extra
  instructions purely from register-save-strategy, which per
  `AGENT_CONTEXT.md`'s explicit rule ("a pure register-permutation residual
  is not source-addressable... spend the effort on unit selection instead")
  I am not chasing further by permuting declarations. More substantively,
  several `addi rX, rBASE, 0xNN` immediates (`0x88`, `0x98`, `0xa0`) in the
  target imply the sprintf-format string, the `"g3d/model.brres"` path
  string, and the `resMdlNames` table are laid out **contiguously inside ONE
  larger aggregate object** (all reachable from a single base register with
  different constant offsets), not as separate string/array literals the
  way I wrote them. I confirmed this hypothesis structurally (an experiment
  wrapping them in one `struct { u8 pad[0x88]; const char *resMdlNames[4];
  char fmtStr[8]; char pathStr[16]; }` got the instruction count from 105
  down to 104, and DID reproduce the target's cached-table-pointer pattern,
  `addi r28, table, 0x88` then reused via `lwzx`/direct offsets, matching
  the target's `addi r25, r30, 0x88` role) but could not fully close it
  because **I do not know what occupies the leading `0x88` (136) bytes** of
  that aggregate -- decoded `.data` bytes there (`lbl_2_data_47258` =
  `"F7C0\0\0\0\0"`, `lbl_2_data_47260` = `"W7C0\0"`, plus a `g_profile_
  WM_SMALLCLOUD`-adjacent parameter table) don't obviously belong to
  `createModel()` and may instead be unrelated file-scope statics (e.g.
  `ACTOR_PARAM_CONFIG`-adjacent editor metadata) that the compiler simply
  placed before this data in **file declaration order**, which is a whole-TU
  concern outside this batch's 8 functions.
- Proposal (best-effort, does not close, offered as the strongest
  documented starting point): see "Full source" below -- the simpler,
  4-separate-object version, NOT the aggregate-struct experiment (which
  used unconfirmed padding content and is not something I am proposing to
  land).
- Compiled: NO full MATCH. Closest measured: 95 differing lines against
  target size 101 with the simple version; 104 vs target 101 (only
  register-shape + a handful of offset lines differing) with the untested-
  content aggregate-struct experiment. Neither is a match.
- Confidence: high on the overall algorithm/control-flow shape (every
  branch, every call, every loop-free structure verified against real
  bytes); low on the exact `.data` layout needed to reproduce the register
  save/restore choice and the `0x88`/`0x98`/`0xa0` immediates exactly.
- Offset-perturbing: NO new class members either way -- this is purely a
  `.data`/`.rodata` layout question, not a class-shape question.
- **What would settle it**: the full `.data`/`.rodata` content for this
  entire TU in file order (not just this batch's 8 functions), so the
  aggregate object's true leading content and size can be read directly
  instead of guessed. That's a whole-TU concern; flagging for the lead
  rather than continuing to guess padding content.

## Variants tried and discarded (createModel(), the only unclosed function)

1. Four separate objects (`static const char *resMdlNames[4]`, two
   independent string literals for the sprintf format and the path) --
   compiled, 98 instructions differing vs target's shape (before the
   `ACTOR_PARAM(CourseNo)` index was added; **not** a real 98, just an
   intermediate step) -- superseded by adding the `ACTOR_PARAM` index.
2. Same as above but WITH `ACTOR_PARAM(CourseNo)` indexing both accesses --
   compiled, 105 instructions vs target 101; matches everything except the
   register-save strategy and address computation for the table (no shared
   base register). This is the version kept in `draft.cpp` as my proposal,
   since it needs no unconfirmed struct padding.
3. One aggregate `struct { u8 pad[0x88]; const char *resMdlNames[4]; char
   fmtStr[8]; char pathStr[16]; }` referenced via `sData.resMdlNames[idx]`
   directly (no intermediate pointer local) -- compiled, 104 instructions;
   reproduced the shared-base-register addressing but via `add r4,base,r0;
   lwz r4,0x88(r4)` (add-then-load) instead of the target's
   `addi r25,base,0x88` (cache-then-indexed-load) -- discarded for variant 4.
4. Same aggregate struct, but with an explicit `const char *const
   *resMdlNames = sData.resMdlNames;` local -- compiled, still 104
   instructions; this DID reproduce the target's cached-pointer addressing
   form (`addi r28, table, 0x88` then reused via `lwzx`/plain offsets,
   matching the target's `r25` role exactly) but a redundant `addi r29, r31,
   0` register-to-register copy appeared (the compiler allocated a wasted
   extra register for the pointer local rather than reusing the table-base
   register directly) and the `_savegpr_25`/manual-save gap remained. Kept
   as scratch evidence only, not proposed for landing, since `pad[0x88]`'s
   content is unconfirmed and it did not close.
5. Compiling under `tools/auto_decomp/harness.py`'s default `CFLAGS` (no
   `-sdata 0 -sdata2 0`, plain `-O4`) -- built, but `dScWMap_c::m_WorldNo`
   and `GLOBAL_DATA` (once small enough) used `@sda21` addressing not
   present in the target; discarded in favour of `build.py`'s slice-accurate
   flags for every compile in this batch.

## Full source

`wip/wm_smallcloud/scratch/batchA/draft.cpp` (the 7 MATCHing functions are
final; `createModel()` is the best-effort, non-closing proposal from variant
2 above; the `initState`/`init_exec`/`mode_exec`/`updatePos`/`calcModel`
bodies are explicitly marked as placeholders for batch B's territory, needed
only so this file compiles standalone):

```cpp
#include <game/bases/d_cs_seq_manager.hpp>
#include <game/bases/d_res_mng.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_a_wm_smallcloud.hpp>
#include <game/sLib/s_GlobalData.hpp>

ACTOR_PROFILE(WM_SMALLCLOUD, daWmSmallCloud_c, 0);

daWmSmallCloud_c::daWmSmallCloud_c() {}

daWmSmallCloud_c::~daWmSmallCloud_c() {
    if (mpBgmSync != nullptr) {
        delete mpBgmSync;
    }
}

int daWmSmallCloud_c::create() {
    dWmBgmSync_c *bgmSync = new dWmBgmSync_c();
    mpBgmSync = bgmSync;

    if (dScWMap_c::m_WorldNo == 5) {
        bgmSync->m_18 = GLOBAL_DATA.mBgmValueW5;
        bgmSync->m_04 = GLOBAL_DATA.mBgmValueW5[0] - 1;
        bgmSync->m_08 = GLOBAL_DATA.mBgmValueW5[1];
    } else {
        bgmSync->m_18 = GLOBAL_DATA.mBgmValue;
        bgmSync->m_04 = 7;
        bgmSync->m_08 = 0;
    }

    createModel();
    mClipSphere.set(mPos, 400.0f);

    calcModel();
    initState();
    return SUCCEEDED;
}

int daWmSmallCloud_c::execute() {
    mpBgmSync->execute();
    if (mpBgmSync->m_0c) {
        float rate = mpBgmSync->getAnmRate(mChrAnim->mFrameMax);
        mChrAnim->setRate(rate);
    }

    static const ProcFunc Proc_tbl[PROC_COUNT] = {
        &daWmSmallCloud_c::mode_exec
    };

    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    if (csSeqMng->FUN_80915600()) {
        processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);
    } else {
        (this->*Proc_tbl[mCurrProc])();
    }

    updatePos();
    mModel.play();
    calcModel();

    return SUCCEEDED;
}

int daWmSmallCloud_c::draw() {
    mModel.entry();
    return SUCCEEDED;
}

int daWmSmallCloud_c::doDelete() {
    return SUCCEEDED;
}

void daWmSmallCloud_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId != dCsSeqMng_c::CUTSCENE_CMD_NONE && !isStaff()) {
        mIsCutEnd = true;
    }
}

void daWmSmallCloud_c::calcModel() {
    mVec3_c pos = mPos;
    mAng3_c angle = mAngle;
    mMatrix.trans(pos);
    mMatrix.ZXYrotM(angle);
    mModel.setLocalMtx(&mMatrix);
    mModel.setScale(mScale);
    mModel.calc(false);
}

// NOTE: initState/init_exec/mode_exec/updatePos (0x179E00, 0x179EA0, 0x179EB0,
// 0x179F10) belong to the OTHER batch. These bodies are placeholders only, so
// that THIS batch's functions (which call them) compile; they are not offered
// as a reconstruction of those functions' real bytes. See BATCHB.md for the
// real, matching bodies (batch B calls 0x179F10 setPosFromCourseNode()).
void daWmSmallCloud_c::initState() {
    mCurrProc = PROC_TYPE_EXEC;
}

void daWmSmallCloud_c::init_exec() {
    mCurrProc = PROC_TYPE_EXEC;
}

void daWmSmallCloud_c::mode_exec() {}

void daWmSmallCloud_c::updatePos() {}

// NOT CLOSED -- see "0x179BB0" section above. Best-effort proposal, 95
// instructions differing from the 101-instruction target (register-save
// strategy and a shared-base-register table address computation this
// version does not reproduce).
void daWmSmallCloud_c::createModel() {
    mAllocator.createFrmHeap(-1, mHeap::g_gameHeaps[mHeap::GAME_HEAP_DEFAULT], nullptr, 0x20);

    char arcName[8];
    sprintf(arcName, "CS_W%d", dScWMap_c::m_WorldNo + 1);
    mResFile = dResMng_c::m_instance->getRes(arcName, "g3d/model.brres");

    static const char *resMdlNames[4] = {
        "CS_W7_MoveCloud01",
        "CS_W7_MoveCloud02",
        "CS_W7_MoveCloud03",
        "CS_W6aCloud"
    };

    nw4r::g3d::ResMdl resMdl = mResFile.GetResMdl(resMdlNames[ACTOR_PARAM(CourseNo)]);
    mModel.create(resMdl, &mAllocator, nw4r::g3d::ScnMdl::BUFFER_RESMATMISC, 1);

    static bool sInit = false;
    static const char *resAnmNames[4];
    if (!sInit) {
        resAnmNames[0] = resMdlNames[0];
        resAnmNames[1] = resMdlNames[1];
        resAnmNames[2] = resMdlNames[2];
        resAnmNames[3] = resMdlNames[3];
        sInit = true;
    }

    static const m3d::playMode_e playModes[ANIM_COUNT] = {
        m3d::FORWARD_LOOP
    };

    nw4r::g3d::ResAnmChr resAnmChr = mResFile.GetResAnmChr(resAnmNames[ACTOR_PARAM(CourseNo)]);
    mChrAnim[CS_W7_SmallCloud].create(resMdl, resAnmChr, &mAllocator, nullptr);
    mChrAnim[CS_W7_SmallCloud].mPlayMode = playModes[CS_W7_SmallCloud];
    mChrAnim[CS_W7_SmallCloud].setRate(0.0f);
    mChrAnim[CS_W7_SmallCloud].setFrame(0.0f);
    mModel.setAnm(mChrAnim[CS_W7_SmallCloud]);

    dWmActor_c::setSoftLight_Map(mModel);
    mAllocator.adjustFrmHeap();
}
```

## Cross-batch notes for the lead

- **Naming disagreement**: I call `0x179F10` `updatePos()`; batch B calls it
  `setPosFromCourseNode()`. Same function, same `ACTOR_PARAM(CourseNo)`
  evidence found independently by both batches -- pick one when merging.
- **Class-shape contradiction to resolve**: my header has NO
  `mGroupNodeIds`/`mCurrNodeClipSphere` (proven absent, see above); batch
  B's shadow header still carries both as unverified twin-copied
  placeholders. Batch A's version should win when merging -- it's backed by
  three independently matching functions' literal offsets, not a guess.
- **`resMdlNames`/`nodeNames` table size cross-check**: batch B's
  `setPosFromCourseNode()` (their `0x179F10`) independently converged on a
  **4-entry** table indexed by the same `ACTOR_PARAM(CourseNo)`, matching
  this batch's `createModel()` table size exactly -- strong independent
  corroboration that `CourseNo` really is a 4-way cloud-variant selector for
  this actor, even though neither batch has proven the node-name strings
  (batch B's `nodeNames[]` content is a placeholder; mine's `resMdlNames[]`
  content is decoded from real `.data` bytes and should be treated as the
  more trustworthy of the two tables' *contents*, though not necessarily the
  same table as `nodeNames[]`).
- **`createModel()`'s `.data` layout is the one open item this batch could
  not close**, and it may be entangled with batch B's `0x179F40`
  (`__sinit_...`) closure, which per their report is ALSO blocked on
  needing this TU's full `.rodata` content in file order. Recommend the
  lead compile both batches' sources together against the full TU's data
  section once merged, rather than either batch continuing to guess in
  isolation.
