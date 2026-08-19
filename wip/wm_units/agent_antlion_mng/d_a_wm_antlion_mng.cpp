#include <game/framework/f_profile.hpp>
#include <game/bases/d_wm_demo_actor.hpp>
#include <game/bases/d_heap_allocator.hpp>
#include <game/bases/d_wm_lib.hpp>
#include <game/bases/d_wm_actor.hpp>
#include <game/bases/d_wm_csv_data.hpp>
#include <game/bases/d_info.hpp>
#include <game/bases/d_a_wm_map.hpp>
#include <game/bases/d_a_wm_player.hpp>
#include <game/bases/d_wm_se_manager.hpp>
#include <game/bases/d_s_world_map_static.hpp>
#include <game/bases/d_game_com.hpp>
#include <game/bases/d_cs_seq_manager.hpp>

/// @unofficial `fn_2_19B170` is a still-undecompiled, REL-local, non-virtual `daWmPlayer_c` member
/// (no exported name) living in daWmPlayer_c's own not-yet-landed TU. A member-call spelling
/// (`ms_instance->someMethod()`) compiles but does NOT link into an un-landed region of the same
/// REL -- `R_<module>_<section>_<offset>` (module 2 = d_basesNP, section 1 = .text) is the
/// established convention instead, declared as a free `extern "C"` function taking the instance
/// pointer explicitly (this project's course unit uses the identical pattern for its own
/// not-yet-landed callees -- see `wip/wm_units/agent_course/d_a_wm_course.cpp`'s `R_2_1_191BF0`).
/// Called four times from daWmAntlionMng_c::processCutsceneCommand (fn_2_15B830), always as
/// `ms_instance->fn_2_19B170()` in the original, taking no arguments beyond `this` and returning
/// a value tested with `cmpwi r3,0` (bool-shaped).
extern "C" bool R_2_1_19B170(daWmPlayer_c *);

/// @unofficial DRAFT, scouted this round but NOT yet verified function-by-function against every
/// target instruction. See the accompanying task report for the full per-function match table and
/// the three independent confirmations of the .text/.ctors/.rodata/.data/.bss bounds below.
///
/// SCOPE CORRECTION, the headline finding of this round: the given expectation ("~79 functions,
/// the largest unit attempted") was for the COMBINED span of WM_ANTLION_MNG + WM_BOARD (+ possibly
/// more) -- this unit alone, correctly bounded between its own classInit (`fn_2_15B590`) and
/// WM_BOARD's own classInit (`fn_2_15C200`, read directly out of `g_profile_WM_BOARD`'s own
/// `.4byte fn_2_*` field), is 22 functions over `.text 0x15b590-0x15c200` (0xc70 bytes).
/// `fn_2_15C200` is independently confirmed to be the START of WM_BOARD's own unit, not merely its
/// classInit: it is byte-identical in SHAPE to this unit's own classInit
/// (`li r3, sizeof; bl __nw__7fBase_cFUl; ...; bl fn_2_15C230`), the same idiom antlion's own
/// classInit (`fn_2_15AC80`) uses at ITS unit's start.
///
/// Class hierarchy confirmed directly from the target constructor (`fn_2_15B5C0`): it calls
/// `bl __ct__14dWmDemoActor_cFv` (NOT `__ct__10dWmEnemy_cFv` -- this class is a manager, not an
/// enemy), installs this class's OWN vtable (`lbl_2_data_43968`) at +0x60, then constructs exactly
/// ONE added member, `dHeapAllocator_c mAllocator`, at +0x188. `daWmAntlionMng_c : public
/// dWmDemoActor_c`, confirmed independently by the vtable dump: every non-overridden slot resolves
/// to a `dWmDemoActor_c`/`dBaseActor_c`/`dBase_c`/`fBase_c` name (`checkCutEnd__14dWmDemoActor_cFv`,
/// `setCutEnd__14dWmDemoActor_cFv`, `clearCutEnd__14dWmDemoActor_cFv`,
/// `GetActorType__14dWmDemoActor_cFv`, `getKindString__7dBase_cCFv`, ...) with NO `dWmEnemy_c`-only
/// slot anywhere (no `doWalk`, `initDemoAnger`, `GetWalkWaitFrame`, ...).
///
/// sizeof(daWmAntlionMng_c) == 0x1b0, read directly off the allocator wrapper
/// (`fn_2_15B590`: `li r3, 0x1b0; bl __nw__7fBase_cFUl`).
///
/// Member offsets read directly from the constructor/destructor (`fn_2_15B5C0`/`fn_2_15B610`):
///   dWmDemoActor_c base            ends 0x188 (mAllocator constructed there; the base's own
///                                   `mModel`/`mHeapAllocator` are destructed by the SAME
///                                   destructor at +0x158/+0x13c, which is dWmDemoActor_c's own
///                                   inline-empty `~dWmDemoActor_c(){}` being folded into this
///                                   class's own dtor rather than emitting a separate call --
///                                   the same vague-linkage idiom documented for antlion)
///   dHeapAllocator_c mAllocator     +0x188  (ctor: __ct__16dHeapAllocator_cFv)
/// 0x188 + sizeof(dHeapAllocator_c) [0x1c, confirmed by antlion's own reconstruction] == 0x1a4,
/// leaving 0xc (three ints) before sizeof ends at 0x1b0 -- mState/mProcessIndex/mTimer below,
/// each confirmed by a direct `stw`/`lwz` at its own fixed offset from several functions.
///
/// VTABLE, confirmed by direct dump of `lbl_2_data_43968` (`bin/dtkspl/d_basesNP/obj/
/// auto_04_0003A960_data.txt`) against the fBase_c -> dBase_c -> dBaseActor_c -> dWmActor_c ->
/// dWmDemoActor_c declaration order (HANDOFF's "class DECLARATION order sets vtable SLOTS" rule):
/// every inherited slot up to and including `finalUpdate` is IMPORTED (this class does not touch
/// it) except create/doDelete/execute/draw (all four overridden, all four EMPTY of dWmDemoActor_c's
/// own trivial bodies) and the class's own destructor (`fn_2_15B610`, vtable+0x48, sitting where
/// `fBase_c::~fBase_c()`'s slot falls -- REL-local rather than an imported name, matching every
/// other landed sibling's dtor slot). dWmDemoActor_c then APPENDS four new slots in its own
/// declaration order (processCutsceneCommand, checkCutEnd, setCutEnd, clearCutEnd); only the first
/// is overridden here (`fn_2_15B830`), the other three stay imported.
class daWmAntlionMng_c : public dWmDemoActor_c {
public:
    daWmAntlionMng_c();
    ~daWmAntlionMng_c();

    /// @unofficial fn_2_15B6A0. `ACTOR_PARAM(startMode)`-shaped 8-bit param (offset 0, width 8) at
    /// mParam's low byte; branches on it == 1. Spawns a child WM_ANTLION (`0x2ae`, WM_ANTLION's own
    /// profile ID -- distinct from `WM_ANTLION_MNG`'s own `0x271`) unconditionally at the end via
    /// `dWmActor_c::construct`.
    virtual int create();
    /// @unofficial fn_2_15B720. `processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164)`
    /// through this class's OWN vtable slot 0x60 (matches antlion's own execute() idiom exactly),
    /// THEN a second dispatch through a 2-entry pointer-to-member-function table indexed by
    /// mProcessIndex (`lbl_2_rodata_85D8`, decoded via the REL's own relocation stream: entry 0
    /// points at `fn_2_15B7D0` (this class's own no-op `procNone()`), entry 1 at `fn_2_15B7F0`
    /// (`procCheck()`) -- resolved this round, not guessed.
    virtual int execute();
    /// @unofficial fn_2_15B7A0. `li r3,1; blr` -- trivial, matches dWmDemoActor_c's own default
    /// body but declared out-of-line (strong-batch placement, same reasoning as every sibling).
    virtual int draw();
    /// @unofficial fn_2_15B7B0. `li r3,1; blr` -- trivial, same reasoning as draw().
    virtual int doDelete();
    /// @unofficial fn_2_15B830 (0x240 B, by far this unit's largest function). MATCHED this
    /// round (byte-identical modulo symbol names). A `switch`-shaped dispatch on
    /// cutsceneCommandId (0x48/0x4a/0x59/0x8e/0x90), gated on isFirstFrame for one half and
    /// unconditional for the other -- the same two-pass shape as antlion's own
    /// processCutsceneCommand, but with five commands instead of two and several branches that
    /// call back into `dWmDemoActor_c::setCutEnd()` (this class's OWN vtable slot 0x68) and into
    /// `R_2_1_19B170(daWmPlayer_c::ms_instance)` (a still-undecompiled, REL-local, non-virtual
    /// daWmPlayer_c member; linked via the `R_2_1_<offset>` convention, declared at the top of
    /// the .cpp -- a plain member-call spelling compiles but does not link). Three real fixes
    /// found by reading the target directly, all wrong in the prior draft: a leading
    /// `if (cutsceneCommandId == -1) return;`; first-frame case 0x90 calls `clearAllModels()`,
    /// not `setCutEnd()`; and case 0x59's timer-expired branch calls
    /// `rebuildAllModels(false, true)` before `clearAllModels()`/`setCutEnd()`, not `setActive()`.
    virtual void processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame);

    /// @unofficial fn_2_15B7C0. Resets mTimer to 0. Also serves as PTMF table entry for
    /// mProcessIndex == 0's "idle" state via procNone(), and is called directly by procCheck()
    /// when the attack sequence (fn_2_15BF80) reports done.
    void resetTimer();
    /// @unofficial fn_2_15B7D0. Empty -- `blr`. This is PTMF table entry 0 (`sProcTable[0]`).
    void procNone();
    /// @unofficial fn_2_15B7E0. Sets mState = 1, mTimer = 1.
    void setActive();
    /// @unofficial fn_2_15B7F0. PTMF table entry 1 (`sProcTable[1]`): if the attack-sequence state
    /// machine (fn_2_15BF80) is done, resets the timer via resetTimer().
    void procCheck();

    /// @unofficial fn_2_15BF80 (0x150 B). A 7-state sequential state machine over mState
    /// (1..7), driving mTimer as a countdown and calling into fn_2_15BDA0 (rebuild),
    /// `dWmMapModel_c::endAntlionEffect()` and this class's own vtable slot 0x68 (setCutEnd).
    /// Returns true only when state 7 is reached with mTimer's countdown exhausted.
    /// MATCHED this round: the fix was a local `bool result` accumulator set once (case 7) and
    /// returned once at the bottom, instead of `return true;`/`return false;` at two separate
    /// exit points -- MWCC only pre-reserves a persistent return-value register (target: r31,
    /// initialised 0 at entry, all cases funnel to one `mr r3,r31` epilogue) when the source has
    /// a single accumulate-then-return shape.
    bool checkAttackSequenceDone();

    /// @unofficial fn_2_15BDA0 (0xD4 B). Nested 2x2 loop over `dInfo_c::GetMapEnemyInfo`; for every
    /// slot with a valid path index, resolves `dCsvData_c::GetPointName` and calls
    /// `dWmMapModel_c::setAntlion(...)` on `daWmMap_c::m_instance->mModels[currIdx]`.
    /// 8/53 differing this round (down from 30), pure register numbering -- see the definition.
    void rebuildAllModels(bool param0, bool param1);

    /// @unofficial fn_2_15BE80 (0xA0 B). Nested 2x2 loop clearing every antlion enemy-info slot
    /// (`dInfo_c::SetMapEnemyInfo(..., -1)`) whose GetMapEnemyInfo result is valid.
    /// 8 differing this round (down from 18), pure register numbering -- see the definition.
    void clearAllModels();

    /// @unofficial fn_2_15BC30 (0x170 B). For each of 2 "route" slots: waits for
    /// pickRevivedIndices to pick a revived index, then for each picked index resolves the point
    /// name, calls `dInfo_c::SetMapEnemyInfo`, `dWmMapModel_c::setAntlion(...)` and
    /// `dWmSeManager_c::playSound(...)` at the antlion's world position.
    /// 29 differing this round (down from 46, from 89 at the start of last round). Widening
    /// `pos`'s scope to before the outer loop (instead of a per-call `mVec3_c pos = ...`) fixed
    /// a genuine 17-instruction gap: the target constructs `GetPos`'s by-value return into ITS
    /// OWN stack temp and copies element-by-element into `pos` for `playSound`'s by-ref
    /// argument, rather than eliding the copy -- matching that copy required widening, not the
    /// inline-wrapper lever (`playSound`'s 3-arg overload is the target's own direct callee, not
    /// a wrapper around the 5-arg one, so there is no trailing default argument to spell/omit
    /// here). Residual: the SAME two stack regions land at swapped high/low offsets (target
    /// anchors GetPos's temp high, ours low) plus a 5-way register rotation
    /// (map/info/route/accum/picked-ptr) -- three more scope/order permutations tried this round
    /// beyond the ones already measured, none closed either gap further. Also tried this round:
    /// giving `GetPos` a sibling overload with a trailing default argument
    /// (`GetPos(int, bool = true)`) to test the inline-wrapper lever from the OTHER side --
    /// MWCC rejects it outright, `(10199) ambiguous access to overloaded function`, because our
    /// one-argument call site is an exact match for BOTH candidates once a default fills the
    /// second slot. The lever needs a genuine second call site (outer vs. loop) to have any
    /// wrapper to go through; reviveOnRoute only ever calls GetPos once, so there is nothing to
    /// pair it against within this function.
    void reviveOnRoute();

    /// @unofficial Full reconstruction this round -- see the definition. CORRECTION carried over
    /// from last round: this is `fn_2_15BA70`, NOT the function the verifier's nearest-neighbour
    /// heuristic keeps pairing it with (`~clearAllModels`/`~reviveOnRoute`, depending on what
    /// else is unmatched at the time). 39 differing against the real implementation (down from
    /// 107 against the old stub) -- shape and every call site confirmed correct; hoisting
    /// `playerPoint`'s declaration above the early-return check (alongside `numPicked`, same
    /// widen-scope lever) got `numPicked` matching target's `r31` exactly, 40->39. Residual is a
    /// 3-way register rotation in the rejection-sampling loop plus one target-only quirk (a
    /// redundant second test of `excludeCurrent` at a control-flow merge point) that every
    /// equivalent source spelling tried -- `||`, nested `if`, explicit `goto`, and a separate
    /// `bool reject = excludeCurrent && playerPoint == cand;` statement -- either reproduced the
    /// same 2-branch shape or (the separate-statement form) made it 5 differing WORSE. The
    /// four-branch target shape is still unexplained, not just unattempted.
    ///
    /// fn_2_15BA70 (0x1C0 B). Scans `dCsvData_c::GetRouteFlag` for the
    /// current world's 0xc0 points against two flag masks (0x400/0x800) selected by
    /// `worldIndex`, builds a local `candidates[9]` list, rejection-samples `count` of them via
    /// `dGameCom::getRandom` (proposed addition to `d_game_com.hpp`, `getRandom(unsigned long)`)
    /// against `daWmPlayer_c::ms_instance->m_22c` when `excludeCurrent`, and finally re-checks
    /// `dWmLib::getEnemyRevivalCount` on each of 2 world slots, forcing the corresponding `out[]`
    /// entry back to -1 if positive.
    bool pickRevivedIndices(int *out, int count, int worldIndex, bool excludeCurrent);

    /// @unofficial fn_2_15BF20 (0x60 B). True iff `dWmLib::getEnemyRevivalCount` is 0 for both
    /// world slots (0/1) of `dScWMap_c::getWorldNo()`.
    bool checkAllRevivalCountsZero();

    /// @unofficial fn_2_15C0D0 (0x68 B). For both world slots (0/1): if the revival count is 0,
    /// sets it to 4 via `dWmLib::setEnemyRevivalCount` and stops (does not touch the other slot).
    /// UNVERIFIED against target bytes this round; `setEnemyRevivalCount` is not yet declared in
    /// `d_wm_lib.hpp` (proposed shadow addition, see the task report).
    void primeRevivalCount();

    /// @unofficial +0x184. NOT base-class padding -- the coordinator independently probed
    /// `sizeof(dWmObjActor_c) == 0x188` (dWmObjActor_c : public dWmDemoActor_c, `sizeof
    /// (dWmDemoActor_c) == 0x184` unchanged) and dWmObjActor_c's own added member sits at
    /// exactly this offset, so the base is NOT short: every `dWmDemoActor_c`-derived class that
    /// adds a member of its own gets ONE at +0x184 first. Confirmed this is a genuine POD member
    /// of THIS class (not shared tail padding) by checking every one of this unit's own 22
    /// functions for a `0x184(rX)` access: there is exactly one such access in the whole
    /// `0x15b590-0x15c200` span's covering objects, and it belongs to `fn_2_15C230` --
    /// WM_BOARD's OWN constructor (`stw r0, 0x184(r30)` with `r0 = -1`), not to this class. So
    /// antlion_mng's own +0x184 is a plain POD field (no constructor call for it appears between
    /// `__ct__14dWmDemoActor_cFv` and `__ct__16dHeapAllocator_cFv`), always zero via
    /// `fBase_c::operator new`'s documented zero-init, and never read or written by any function
    /// in this unit -- its concrete meaning is UNRESOLVED, left as a raw-byte placeholder rather
    /// than guessed at (same policy as antlion's own +0x6c4-0x6e8 POD gap).
    int mUnk184;
    dHeapAllocator_c mAllocator;
    /// @unofficial +0x1a4. Attack-sequence state (0 = idle; 1..7 driven by checkAttackSequenceDone).
    int mState;
    /// @unofficial +0x1a8. Selects the PTMF entry execute() dispatches through every frame.
    int mProcessIndex;
    /// @unofficial +0x1ac. Generic countdown used by both the attack sequence and procCheck.
    int mTimer;

    typedef void (daWmAntlionMng_c::*ProcFunc_t)();
    /// @unofficial `lbl_2_rodata_85D8`, decoded via the REL's own relocation stream (relocations
    /// exist against 0x85e0 -> fn_2_15B7D0 and 0x85ec -> fn_2_15B7F0): entry 0 is procNone(),
    /// entry 1 is procCheck().
    static const ProcFunc_t sProcTable[2];

    /// @unofficial Private to this class; spells create()'s `mParam & 0xff` extraction. Bit
    /// offset/width not independently cross-checked against another consumer -- open item.
    ACTOR_PARAM_CONFIG(startMode, 0, 8);
};

/// @unofficial Named object, `0x85c0-0x85d8` in the retail binary, sitting immediately before
/// `sProcTable` (named objects emit before function pools -- confirmed by `__sinit`'s own pool
/// anchor: the target's `__sinit` reads `mNodePos` at pool-base+0x48, which only resolves to a
/// sane address if this TU's own rodata contribution runs the whole `0x85c0-0x8608` span, not
/// just the `sProcTable`/`worldIndexTable`/`mNodePos` pieces already identified). `fn_2_19B170`
/// (daWmPlayer_c's own TU) reads one word out of the middle of it (`+4`, confirmed via a
/// relocation from `0x19b198/0x19b1a4` straight into `0x85c4`), consistent with this being
/// declared here and referenced `extern` there. Byte-exact from the retail binary; shape read as
/// {count?=9, index?=2, flags={0,1,1,0}, pad=0, timerPair={60,20}, pad=0} but NOT confirmed
/// against any consumer inside this unit -- no function here reads it.
extern const unsigned int g_unofficial_85C0[6] = {9, 2, 0x00010100, 0, 0x003c0014, 0};

const daWmAntlionMng_c::ProcFunc_t daWmAntlionMng_c::sProcTable[2] = {
    &daWmAntlionMng_c::procNone,
    &daWmAntlionMng_c::procCheck,
};

ACTOR_PROFILE(WM_ANTLION_MNG, daWmAntlionMng_c, 0);

daWmAntlionMng_c::daWmAntlionMng_c() {}
daWmAntlionMng_c::~daWmAntlionMng_c() {}

int daWmAntlionMng_c::create() {
    if ((int)ACTOR_PARAM_LOCAL(mParam, startMode) == 1) {
        rebuildAllModels(false, false);
    } else {
        rebuildAllModels(true, true);
        setActive();
    }

    dWmActor_c::construct(0x2ae, this, 0, nullptr, nullptr);
    return SUCCEEDED;
}

int daWmAntlionMng_c::execute() {
    dCsSeqMng_c *csSeqMng = dCsSeqMng_c::ms_instance;
    processCutsceneCommand(csSeqMng->GetCutName(), csSeqMng->m_164);

    (this->*sProcTable[mProcessIndex])();

    return SUCCEEDED;
}

int daWmAntlionMng_c::draw() {
    return SUCCEEDED;
}

int daWmAntlionMng_c::doDelete() {
    return SUCCEEDED;
}

void daWmAntlionMng_c::resetTimer() {
    mProcessIndex = 0;
}

/// @unofficial fn_2_15B7D0, declared out-of-line here (rather than in-class) so its STRONG
/// linkage places its definition in target address order -- the in-class inline default defers
/// to the trailing weak block, same lever as antlion's own 16 explicit re-declarations.
void daWmAntlionMng_c::procNone() {}

void daWmAntlionMng_c::setActive() {
    mState = 1;
    mProcessIndex = 1;
}

void daWmAntlionMng_c::procCheck() {
    if (checkAttackSequenceDone()) {
        resetTimer();
    }
}

/// @unofficial MATCHED-EFFORT this round (0x240 B / 144 instrs). Reconstructed by reading
/// fn_2_15B830's full disassembly directly. Three real fixes found against the prior draft's
/// guesses, all confirmed by the target's own instruction sequence (not diffed byte-for-byte
/// yet, but every branch/call site below is read straight off the target, not guessed):
///   1. A leading `if (cutsceneCommandId == -1) return;` -- the target's very first two
///      instructions are `cmpwi r4,-1; beq <epilogue>`, before either switch is reached.
///   2. First-frame case 0x90 calls `clearAllModels()` (`bl fn_2_15BE80`), NOT `setCutEnd()` --
///      the prior draft's guess was wrong.
///   3. Second-switch case 0x59's "timer expired" branch calls `rebuildAllModels(false, true)`
///      (`bl fn_2_15BDA0` with r4=0,r5=1) before `clearAllModels()`/`setCutEnd()`, NOT
///      `setActive()` -- the prior draft's guess was wrong here too.
/// `daWmPlayer_c::ms_instance->unofficialFn_19B170()` is gone: `fn_2_19B170` sits in daWmPlayer_c's
/// own not-yet-landed TU, and a member-call spelling compiles but will NOT link. Switched to the
/// established `R_2_1_<offset>` convention (extern "C" free function taking the instance pointer
/// explicitly) per the class declaration's own doc comment.
void daWmAntlionMng_c::processCutsceneCommand(int cutsceneCommandId, bool isFirstFrame) {
    if (cutsceneCommandId == -1) {
        return;
    }

    if (isFirstFrame) {
        switch (cutsceneCommandId) {
        case 0x4a:
            mState = 1;
            break;
        case 0x48:
            reviveOnRoute();
            break;
        case 0x59:
            mTimer = 0x14;
            break;
        case 0x8e:
            if (R_2_1_19B170(daWmPlayer_c::ms_instance)) {
                mState = 0;
                mTimer = 0x14;
            }
            break;
        case 0x90:
            if (R_2_1_19B170(daWmPlayer_c::ms_instance)) {
                clearAllModels();
            }
            break;
        }
    }

    switch (cutsceneCommandId) {
    case 0x4a:
        if (R_2_1_19B170(daWmPlayer_c::ms_instance) && daWmPlayer_c::isPlayerStarMode()) {
            setCutEnd();
        } else if (checkAttackSequenceDone()) {
            setCutEnd();
        }
        break;
    case 0x48:
        setCutEnd();
        break;
    case 0x59:
        if (mTimer > 0) {
            mTimer--;
        } else {
            rebuildAllModels(false, true);
            clearAllModels();
            setCutEnd();
        }
        break;
    case 0x8e:
        if (R_2_1_19B170(daWmPlayer_c::ms_instance)) {
            if (mState >= 1) {
                if (checkAttackSequenceDone()) {
                    setCutEnd();
                }
            } else if (mTimer > 0) {
                mTimer--;
                mState = 1;
            }
        } else {
            setCutEnd();
        }
        break;
    default:
        mIsCutEnd = true;
        break;
    }
}

/// @unofficial Full reconstruction this round, read directly off fn_2_15BA70's own disassembly
/// (0x1C0 B / 112 instrs) -- see the class declaration's doc comment for the identity
/// correction. Placed here (not after reviveOnRoute, which calls it) to match the TARGET's
/// address order, not call-graph order.
///
/// Three phases, each confirmed against its own block of the target:
///   1. `candidates[9]` (9 individual `-1` stores, no loop -- MWCC unrolls a fixed 9-element
///      init) collects every point `i` in `[0,0xc0)` whose `dCsvData_c::GetRouteFlag(i, mask)`
///      is set, `mask` selected by `worldIndex` (0 -> 0x400, 1 -> 0x800; any other value finds
///      nothing). If fewer than `count` were found, returns false immediately.
///   2. Rejection-sampling loop: draws `dGameCom::getRandom(foundCount)`, rejects a candidate
///      that is already consumed (target tests `<= 0`, so a consumed slot is marked back to -1
///      and 0 is ALSO treated as "unusable" -- reproduced exactly, not "fixed", since this is
///      the original game's own behaviour) or equals `daWmPlayer_c::ms_instance->m_22c` when
///      `excludeCurrent` is set; otherwise writes it to `out[]`, marks the slot consumed, and
///      keeps drawing until `count` picks are made.
///   3. Re-validates both halves of the pair (`worldIndex*2 + j` for j in {0,1}): if
///      `dWmLib::getEnemyRevivalCount` on that slot is positive, the corresponding `out[j]` is
///      forced back to -1 -- this walks `out` from its ORIGINAL start again, independent of how
///      many were written by phase 2, so it only makes sense for the `count == 2` caller
///      (`reviveOnRoute`); a different `count` would leave `out[2..]` untouched by this phase.
bool daWmAntlionMng_c::pickRevivedIndices(int *out, int count, int worldIndex, bool excludeCurrent) {
    int candidates[9];
    for (int k = 0; k < 9; k++) {
        candidates[k] = -1;
    }

    daWmMap_c *map = daWmMap_c::m_instance;
    int foundCount = 0;
    for (int i = 0; i < 0xc0; i++) {
        if (worldIndex == 0) {
            if (map->mCsvData[map->currIdx].GetRouteFlag(i, 0x400)) {
                candidates[foundCount] = i;
                foundCount++;
            }
        } else if (worldIndex == 1) {
            if (map->mCsvData[map->currIdx].GetRouteFlag(i, 0x800)) {
                candidates[foundCount] = i;
                foundCount++;
            }
        }
    }

    int playerPoint;
    int numPicked = 0;
    if (foundCount < count) {
        return false;
    }

    playerPoint = daWmPlayer_c::ms_instance->m_22c;
    do {
        int randIdx = dGameCom::getRandom(foundCount);
        int cand = candidates[randIdx];
        if (cand > 0 && (!excludeCurrent || playerPoint != cand)) {
            out[numPicked] = cand;
            numPicked++;
            candidates[randIdx] = -1;
        }
    } while (numPicked != count);

    map = daWmMap_c::m_instance;
    for (int j = 0; j < 2; j++) {
        if (dWmLib::getEnemyRevivalCount(*(int *)((char *)map + 0x3388), worldIndex * 2 + j) > 0) {
            out[j] = -1;
        }
    }

    return true;
}

/// @unofficial fn_2_15BC30: down to 46 differing this round (from 89), all of it register
/// numbering -- structure now confirmed correct against the target disassembly. Two real logic
/// fixes found by reading the target directly: (1) `SetMapEnemyInfo`'s first argument is the
/// same unnamed `daWmMap_c` +0x3388 field as clearAllModels/rebuildAllModels use, NOT `route`.
/// (2) the second argument is an outer-loop accumulator (+2 per route, matching the established
/// "accumulator not multiply" family), not `i + route * 2`. See `worldIndexTable`'s own comment
/// below for the third fix (confirmed independently via the retail REL's raw rodata bytes).
void daWmAntlionMng_c::reviveOnRoute() {
    dInfo_c *info = dInfo_c::m_instance;
    daWmMap_c *map = daWmMap_c::m_instance;

    /// @unofficial `lbl_2_data_43960` (8 B, right before this unit's own vtable in `.data`,
    /// initial raw bytes `01 00 00 00 00 00 00 00`) -- a persistent flag, NOT a function-local
    /// `static bool` (which would land in `.bss`, not `.data`, and is why an earlier draft of
    /// this function scored a real `.bss` overclaim under `check_sections.py --layout`). Given a
    /// non-zero initial VALUE so the compiler places it in `.data`; the exact type/size (8 B, not
    /// 1) is still open -- this is a `bool` stand-in, not a verified reconstruction.
    static bool sToggle = true;

    /// @unofficial `lbl_2_rodata_85F0` (8 B: `{0, 1}`), copied onto the stack at function entry --
    /// the target reads `pickRevivedIndices`'s worldIndex argument through a pointer that walks
    /// this stack copy (+4 per outer iteration), not the raw `route` loop counter directly. Values
    /// are numerically identical to `route` (0, 1) but the codegen shape (aggregate-initialised
    /// local array, not a bare loop variable) is what the target's stack layout requires.
    const int worldIndexTable[2] = {0, 1};

    /// @unofficial Second unidentified rodata run this round: `0x85f8-0x8608` (4 words), same
    /// `{0,1,0,1}` shape as `worldIndexTable` immediately preceding it, sitting right before
    /// `sc_ForceList`'s own `mNodePos` vec3 in the pool. Not consumed by anything found in this
    /// unit; declared unused, purely to reproduce the retail byte layout and close `__sinit`'s
    /// pool-offset residual (`+0x38` without it, target wants `+0x48`).
    const int unofficialTable85F8[4] = {0, 1, 0, 1};
    (void)unofficialTable85F8;

    for (int route = 0, accum = 0; route < 2; route++, accum += 2) {
        int picked[2];
        if (!pickRevivedIndices(picked, 2, worldIndexTable[route], sToggle)) {
            continue;
        }
        sToggle = false;

        mVec3_c pos;
        dBase_c *antlion = dBase_c::searchBaseByProfName(0x28e, nullptr);
        for (int i = 0; i < 2; i++) {
            if (picked[i] < 0) {
                continue;
            }

            const char *pointName = map->mCsvData[map->currIdx].GetPointName(picked[i]);
            int world = pointName[3] - '0';
            info->SetMapEnemyInfo(*(int *)((char *)map + 0x3388), i + accum, map->currIdx, picked[i]);
            map->mModels[map->currIdx].setAntlion(true, world, true);

            pos = map->GetPos(picked[i]);
            dWmSeManager_c::m_pInstance->playSound(0x58, pos, 1);
        }
    }
}

/// @unofficial fn_2_15BDA0: down to 8 differing this round (from 30), pure register numbering
/// (a 3-way rotation among map/base/slot that several declaration-order permutations did not
/// fully resolve -- see the task report). Real fix found: `setAntlion`'s receiver
/// (`daWmMap_c::m_instance->mModels[...]`) is a FRESH `m_instance` read, not the cached `map`
/// local reused from the `GetPointName` access earlier in the same function -- the target
/// reloads `daWmMap_c::m_instance` a second time even though `map` is still live.
/// @unofficial Same two fixes as clearAllModels (see its own doc comment): the unnamed
/// `daWmMap_c` +0x3388 field as GetMapEnemyInfo's first argument, and an outer-loop accumulator
/// for `idx` instead of `sub + slot * 2`. Also: `setAntlion`'s two bool arguments are this
/// function's own two parameters, passed straight through (`param0`, `param1`), not hardcoded
/// `true`/`false` as the first draft guessed.
void daWmAntlionMng_c::rebuildAllModels(bool param0, bool param1) {
    daWmMap_c *map = daWmMap_c::m_instance;
    int base = 0;
    dInfo_c *info = dInfo_c::m_instance;

    for (int slot = 0; slot < 2; slot++) {
        for (int sub = 0; sub < 2; sub++) {
            int idx = sub + base;
            dInfo_c::enemy_s enemy;
            info->GetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, enemy);
            if (enemy.mPathIndex >= 0) {
                const char *pointName = map->mCsvData[map->currIdx].GetPointName(enemy.mPathIndex);
                int world = pointName[3] - '0';
                daWmMap_c::m_instance->mModels[daWmMap_c::m_instance->currIdx].setAntlion(param0, world, param1);
            }
        }
        base += 2;
    }
}

/// @unofficial fn_2_15BE80: down to 8 differing this round (from 18), pure register numbering
/// (control flow confirmed structurally identical to the target by direct disassembly
/// comparison). Fixed by hoisting `idx` out of the inner loop (widen-scope lever) and declaring
/// it before `base`; the residual 3-way rotation among map/base/idx did not yield to further
/// declaration-order permutations this round -- see the task report for what was tried.
/// @unofficial Two fixes this round, both found by direct comparison against the target's
/// register allocation: (1) the outer/inner loop var is NOT the first argument to
/// GetMapEnemyInfo/SetMapEnemyInfo -- the target reads an UNNAMED field at `daWmMap_c` +0x3388
/// (the last 4 bytes of the still-opaque `mPad1[0x20c]`, immediately before `currIdx` at
/// +0x338c) fresh on every inner iteration and passes THAT. (2) `idx` is an outer-loop
/// ACCUMULATOR incremented by 2 per outer pass, not `sub + slot * 2` recomputed by
/// multiplication -- matches rebuildAllModels' own target shape (fn_2_15BDA0) exactly, and is
/// the "decouple declaration order from usage order" family of levers applied to a loop instead
/// of a constructor argument list.
void daWmAntlionMng_c::clearAllModels() {
    int idx;
    int base = 0;
    daWmMap_c *map = daWmMap_c::m_instance;
    dInfo_c *info = dInfo_c::m_instance;

    for (int slot = 0; slot < 2; slot++) {
        for (int sub = 0; sub < 2; sub++) {
            idx = sub + base;
            dInfo_c::enemy_s enemy;
            info->GetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, enemy);
            if (enemy.mPathIndex >= 0) {
                info->SetMapEnemyInfo(*(int *)((char *)map + 0x3388), idx, map->currIdx, -1);
            }
        }
        base += 2;
    }
}

/// @unofficial fn_2_15BF20. Signedness/branch-polarity fix this round: the target's `beq`/`bne`
/// shape (`cmpwi r3,0; bne loop_continue; li r3,1; b end`) is `== 0 -> return true`, an ANY not
/// an ALL -- the opposite of the first draft's `!= 0 -> return false`, which inverted which
/// branch lays out first even though both are logically equivalent as a loop predicate.
bool daWmAntlionMng_c::checkAllRevivalCountsZero() {
    for (int i = 0; i < 2; i++) {
        if (dWmLib::getEnemyRevivalCount(dScWMap_c::m_WorldNo, i) == 0) {
            return true;
        }
    }
    return false;
}

/// @unofficial MATCHED against fn_2_15BF80 this round (byte-identical modulo symbol names).
bool daWmAntlionMng_c::checkAttackSequenceDone() {
    bool result = false;
    switch (mState) {
    case 1:
        mTimer = 0x3c;
        mState = 2;
        break;
    case 2:
        if (--mTimer < 0) {
            mState = 3;
        }
        break;
    case 3:
        rebuildAllModels(false, true);
        mState = 4;
        break;
    case 4:
        rebuildAllModels(false, true);
        mState = 5;
        mTimer = 0x14;
        break;
    case 5:
        if (mTimer > 0) {
            mTimer--;
        } else {
            daWmMap_c::m_instance->mModels[daWmMap_c::m_instance->currIdx].endAntlionEffect();
            mTimer = 0x1e;
            mState = 6;
        }
        break;
    case 6:
        if (mTimer > 0) {
            mTimer--;
        } else {
            mState = 7;
        }
        break;
    case 7:
        result = true;
        break;
    }
    return result;
}

/// @unofficial UNVERIFIED. Best-effort translation of fn_2_15C0D0's control flow.
void daWmAntlionMng_c::primeRevivalCount() {
    for (int i = 0; i < 2; i++) {
        if (dWmLib::getEnemyRevivalCount(dScWMap_c::m_WorldNo, i) == 0) {
            dWmLib::setEnemyRevivalCount(dScWMap_c::m_WorldNo, i, 4);
            return;
        }
    }
}
