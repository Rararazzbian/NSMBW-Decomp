# Closer-E report -- `__dt__13dNandThread_cFv` (primary), `spaceCheck__13dNandThread_cFv` (secondary)

Work in `wip/nand_thread/scratch/closer_e/`. Compiled and diffed exclusively
through `tools/auto_decomp/harness.py` (`compile_draft` / `disasm` /
`extract` / `diff_fn` / `list_functions`), imported directly, never
hand-invoked. Extraction verified by ADDRESS via `diff_fn`, which extracts by
name after a uniqueness check; sizes cross-checked against
`bin/dtk/wiimj2d_symbols.txt`. `ninja`, `configure.py`, `progress.py` and
`land.py` were never run. No shared header, `slices/wiimj2d.json`, or
`syms.txt` was edited -- the one header change already reflected in the real
tree (Closer-C's `EGG::Mutex(OSMutex*)` constructor) was re-verified against
the real header, not re-proposed.

**Neither function closed.** Both remain precisely characterised negative
results, consistent with CLOSE_C's and CLOSE_B's prior conclusions. This
round tried every angle the brief pointed at plus several more, and adds two
genuinely new findings: the destructor's missing guard is **not** an
artifact of translation-unit size/complexity (definitively ruled out by
compiling almost the whole real TU together), and it **is not affected** by
either destructor's virtual-ness. `spaceCheck`'s `r3`-vs-`r4` plateau is
reconfirmed under type/signature variation the prior two batches did not try.

## Status table

| Function | Address | Target size | Result |
|---|---|---|---|
| `__dt__13dNandThread_cFv` | `0x800CEEA0` | `0x64` (25 instr) | **NOT matching.** 24/25, single missing instruction (the redundant `beq`), unchanged after 3 whole-file/member experiments + 8 targeted variants beyond CLOSE_C's 5. |
| `spaceCheck__13dNandThread_cFv` | `0x800CF0D0` | `0x94` (37 instr) | **NOT matching.** 34/37, identical `r4`-vs-`r3` register diff, unchanged after 8 signature/type variants beyond CLOSE_B's 24. |

Verified against the **real, unmodified** `include/game/bases/d_nand_thread.hpp`:

```
=== __ct__13dNandThread_cFiPQ23EGG4Heap: MATCH ===
=== __dt__Q23EGG5MutexFv: MATCH ===
=== __dt__6mMutexFv: MATCH ===
=== __dt__13dNandThread_cFv: DIFF (24/25) ===

  __ct__13dNandThread_cFiPQ23EGG4Heap           size=0x118  (expect 0x118)
  __dt__Q23EGG5MutexFv                          size=0x40   (expect 0x40)
  __dt__6mMutexFv                               size=0x40   (expect 0x40)
  __dt__13dNandThread_cFv                       size=0x60   (expect 0x64)
  run__13dNandThread_cFv                        size=0xdc
  create__13dNandThread_cFPQ23EGG4Heap          size=0x78
  setNandError__13dNandThread_cFl               size=0x78
  getSaveData__13dNandThread_cFv                size=0xc
  onExit__Q23EGG6ThreadFv                       weak
  onEnter__Q23EGG6ThreadFv                      weak
```

The constructor and both weak destructors are confirmed byte-exact and in
the correct emitted order **against the real header**, not a shadow copy.
Script: `wip/nand_thread/scratch/closer_e/verify_final.py`.

---

## Primary target -- `__dt__13dNandThread_cFv`

### The gap, unchanged from CLOSE_C

```
want (target)                          got (draft)
------------------------------------   ------------------------------
beq end            (this==0 guard)     beq end
li r0, 0x0                             li r0, 0x0
stw r0, m_instance@sda21               stw r0, m_instance@sda21
beq L_800CEED8      <-- MISSING        li r4, 0x0
li r4, 0x0                             bl __dt__Q23EGG6ThreadFv
bl __dt__Q23EGG6ThreadFv               cmpwi r31, 0x0
L_800CEED8:                            ble end
cmpwi r31, 0x0                         mr r3, r30
ble end                                bl __dl__FPv
mr r3, r30
bl __dl__FPv
```

Re-confirmed directly from raw bytes (not re-derived): between the two
`beq`s only `li r0,0x0` and `stw r0,...` execute, neither of which touches
`cr0`. The second `beq` tests the exact same, already-known-false condition
as the first. It is provably dead code and MWCC emitted it anyway.

### New this round: ruling out "it needs a bigger/more complex TU"

The brief's hypothesis 1 asks whether `mMutex`'s presence (a member with a
virtual destructor whose body inlines away) is what leaves the guard behind
elsewhere. Testing this required knowing whether the *compiled draft's*
overall TU size/shape matters -- CLOSE_C's test already compiled 6
functions together (not just the destructor in isolation), but the brief
raises the concern seriously enough to warrant a harder test: compile
**nearly the entire real TU** (20 of 24 functions -- everything except
`fn_800CF170`, `deleteFile`'s neighbours already covered, and the two
functions genuinely still open) against the **real, unmodified header**, and
recheck the destructor's shape.

File: `wip/nand_thread/scratch/closer_e/probes/megafile.cpp` (research probe
only, never a deliverable -- combines already-written bodies from
`closer_a`/`closer_b`/`closer_c` verbatim; some of those bodies do not
individually match their own targets, which is irrelevant here since nothing
in this file is landed).

Result: **still 24/25, identical shape**, identical missing `beq`. This
**rules out** "whole-file complexity" or "-ipa file needs more context" as
the trigger -- CLOSE_C's finding holds at both TU sizes.

### New this round: an isolated minimal reproduction, then systematic knob-turning

Built a tiny two-class reproduction (`Base`/`Derived`, no headers, no
project dependencies) that reproduces the exact 24-word failure shape in six
lines of source (`wip/nand_thread/scratch/closer_e/probes/p1.cpp`):

```cpp
class Base {
public:
    virtual ~Base();
};
class Derived : public Base {
public:
    virtual ~Derived();
    static int *m_instance;
};
int *Derived::m_instance;
Derived::~Derived() { m_instance = 0; }
```

This compiles to the **same** 24-instruction shape as the real destructor,
missing the same redundant `beq`, with `cmpwi r3,0x0` / `stw .../ li r4,0x0 /
bl __dt__4BaseFv` in place of the real names. It is a faithful, minimal
model of the bug, and is used as the base for every subsequent knob below
(`wip/nand_thread/scratch/closer_e/probes/battery.py`).

| Variant | What changed | Result |
|---|---|---|
| `p2` | Add an `mMutex`-shaped member: `Member : public MutexBase`, both with empty inline virtual dtors, exactly mirroring `mMutex : EGG::Mutex` | Still 24 words. The member's implicit destroy call folds away completely (guard **and** call, no residue), base guard still missing. |
| `p3` | p2 + `Base` gets 3 more virtuals (`run`/`onEnter`/`onExit`, mirroring `EGG::Thread`) + `Derived` overrides `run` too | Still 24 words. Vtable slot count/virtual-function count has no effect. |
| `base_dtor_nonvirtual` | `Base`'s destructor declared **non-virtual** (probe only -- the real vtable proves `~Thread` must stay virtual, this is pure compiler-behaviour research, not a candidate fix) | Still 24 words, byte-identical shape. |
| `derived_dtor_nonvirtual` | `Derived`'s own destructor declared **non-virtual** (same caveat) | Still 24 words, byte-identical shape. |
| `base_dtor_inline_empty` | `Base`'s destructor given an inline **empty** body (`{}`) instead of declared-only | Drops to **18 words** -- now the base call is transparent too, and IPA folds guard+call for the base exactly as it already does for the member. Confirms: opacity of the callee is what keeps the call itself alive, but does not explain why the *redundant guard* around a still-live call survives in the target and not here. |
| `h3_guarded_store` | Body written as `if (this) { m_instance = 0; }` | 26 words: the extra `beq` guards the **body store**, not the base-dtor call, plus a spurious extra `mr r3,r30`. Independently reconfirms CLOSE_C's attempt 1 -- wrong location, not a fix. |
| `h3_qualified_store` | `Derived::m_instance = 0;` (explicitly qualified) | Byte-identical to baseline, no effect. Reconfirms CLOSE_C's attempt 4 analogue. |
| `h3_helper_store` | Body calls a `static inline` helper `clearInstance()` that does the store | 27 words, and a **new, different-shaped** wrong result: MWCC inserts a fresh, unconditional vtable re-store (`lis/addi/stw` to the vtable slot) at destructor entry, not present in the target at all. Reaching a member/static function from inside the destructor via a call apparently triggers a defensive vtable stamp; the target has no such stamp anywhere. New negative finding, not tried before. |
| `h3_selfstore_padding` | Body does `m_instance = 0; mState = mState;` (adds a dead self-store touching `this`, hoping to add IR complexity without net instructions) | Byte-identical 24 words -- the self-store folds away before it can perturb anything. |
| `h3_two_static_stores` | Two distinct static stores: `m_instance = 0; s_flag = 0;` | **25 words -- matches target's instruction count** -- but wrong content: the extra word is a second real `stw` for `s_flag`, not the guard. This is the same "coincidental length match" trap CLOSE_C already found with `mFileExists = false;`, reproduced here in a clean, minimal, unrelated-to-the-real-header setting, which rules out "it was something specific to `mFileExists`'s offset/type" as an explanation for that earlier near-miss. |

None of the eleven variants (3 whole-TU/structural + 8 targeted knobs, on
top of CLOSE_C's original 5) reproduce the target's exact shape: a
second, provably redundant `this == 0` test immediately preceding the base
destructor call, with nothing else disturbed.

### What this rules out, concretely

- **Not TU size or `-ipa file` context** -- reproduced with 2 functions,
  reproduced with ~20 real functions compiled together, same result both
  times.
- **Not the member's presence, depth, or virtual-function count** -- p1
  (no member) and p2/p3 (member with its own virtual base, extra virtuals on
  `Base`) are byte-identical.
- **Not either destructor's virtual-ness** -- `base_dtor_nonvirtual` and
  `derived_dtor_nonvirtual` are byte-identical to baseline (probe only; the
  real class must stay virtual per the proven vtable).
- **Not the store's qualification or an inert self-store** -- `Derived::x =
  0`, and a dead `mState = mState;` padding statement, are both no-ops for
  this purpose.
- **Confirmed (not new, but reconfirmed independently)**: an explicit
  `if (this)` around the body produces a guard in the *wrong place*
  (guarding the store, not the base call), and adding any real second
  statement that survives to codegen fills the extra instruction slot with
  real content rather than the guard, which is a trap, not a fix.

### Conclusion

This remains what CLOSE_C characterised it as: a case where MWCC's
redundant-branch-fold pass did not fire on the retail compile for reasons
internal to the compiler's pass ordering, not reproducible by any source
restructuring found so far -- now tested across whole-TU size, member
shape, virtual-ness, and five different body-statement shapes, all
converging on the same two outcomes: either the byte-identical 24-word
miss, or a differently-wrong shape (guard in the wrong place, spurious
vtable restamp, or a coincidental-length/wrong-content match). No lever
found. Reporting the gap as unclosed rather than guessing further.

### Final source (unmatched, 24/25 -- unchanged from CLOSE_C)

```cpp
dNandThread_c::~dNandThread_c() {
    m_instance = nullptr;
}
```

No header change is proposed for the destructor. The one header change in
this TU's history (`EGG::Mutex`'s constructor taking `OSMutex*`, from
CLOSE_C) is already present in the real, landed
`include/game/bases/d_nand_thread.hpp` and was re-verified against it
directly (see "Status table" above), not re-proposed.

---

## Secondary target -- `spaceCheck`

### The gap, unchanged from CLOSE_B

```
   21 | want: lwz r3, 0x8(r1)                              got: lwz r4, 0x8(r1)
   22 | want: andi. r0, r3, 0x5                             got: andi. r0, r4, 0x5
   27 | want: andi. r0, r3, 0xa                             got: andi. r0, r4, 0xa
```

Reproduced first against the real header with CLOSE_B's exact source, no
changes: identical 3-line diff, confirming the plateau before spending any
new attempts (`wip/nand_thread/scratch/closer_e/probes/sc_run.py`).

### Variants tried this round, all new (none from CLOSE_B's 24 or Batch 2's 13)

Via a shadow copy of `NANDCheck.h` (never the real header) and, for one
variant, a shadow copy of `d_nand_thread.hpp`
(`wip/nand_thread/scratch/closer_e/probes/sc_battery.py`):

| Variant | Change | Result |
|---|---|---|
| `sig_int_params` | `NANDCheck(int, int, u32*)` instead of `(u32, u32, u32*)` | Identical 3-line diff, still `r4`. |
| `sig_long_return` | `NANDCheck` returns `long` instead of `s32` | Identical 3-line diff, still `r4`. (Note: `long` and `s32` are typically the same underlying type on this target, so this is a weak test, but it was on the brief's list and is reported for completeness.) |
| `sig_void_return_adjusted` | `NANDCheck` returns `void`; call site restructured to not capture a return value into `err` | **Different program, wrong shape entirely** -- 32 instructions, register allocation changes throughout. Not a lever; removing `err` changes the semantics MWCC has to compile, not just the register choice. |
| `answer_array1` | `answer` declared `u32 answer[1] = {0xFFFFFFFF}` instead of a plain `u32`, passed as `answer` (array-decay) instead of `&answer` | **Worse**: adds a *new* divergence at the top of the function (the array initialiser goes through a pooled constant/`SYM0` reference the target does not have) on top of the same unchanged `r4` diff at the bottom. |
| `answer_struct_byaddr` | `answer` wrapped in a one-member struct passed by address | Same new top-of-function divergence as `answer_array1`, same unchanged `r4` diff. |
| `extra_unused_param` | `NANDCheck` given a 4th, unused `u32 reserved` parameter, called with `0` | Different instruction count (38, not 37) -- wrong shape, extra parameter marshalling throughout. |
| `params_reordered` | `answer` moved to be `NANDCheck`'s *first* parameter instead of its third | Different diff pattern from the start of the function (parameter registers shuffle throughout) -- not closer, and still not `r3` at the reload point. |
| `setnanderror_bool_return` | `setNandError` declared to return `bool` instead of `void` (shadow copy of `d_nand_thread.hpp`, not a real edit) | Identical 3-line diff, still `r4`. |

### Conclusion

None of the eight new angles -- `NANDCheck`'s parameter types, its return
type, `answer`'s storage shape (scalar vs. array vs. struct-by-address), an
extra parameter, parameter order, and `setNandError`'s return type -- move
the register choice off `r4`. Combined with CLOSE_B's 24 and Batch 2's 13
source-shape variants, that is **32 source/signature variants tried across
three rounds**, all landing on the identical `r4` choice whenever the
instruction count is preserved. This confirms CLOSE_B's plateau conclusion:
this reads as an MWCC register-allocator implementation detail invisible at
the C++ level, not a source-shape problem. No lever found.

### Final source (unmatched, 34/37 -- unchanged from CLOSE_B)

```cpp
bool dNandThread_c::spaceCheck() {
    u32 answer = 0xFFFFFFFF;
    s32 err = NANDCheck(3, 2, &answer);
    setNandError(err);
    if (mError == 0) {
        if (err == 0) {
            if (answer & 5) {
                mError = 7;
            } else if (answer & 0xa) {
                mError = 8;
            }
        }
    }
}
```

No header change is proposed for `spaceCheck` or `NANDCheck`. Every
signature variant tried was shadow-tested only, in
`wip/nand_thread/scratch/closer_e/shadow_include/`, and reverted after each
test; the real headers were never modified.

---

## Files

- `wip/nand_thread/scratch/closer_e/d_nand_thread.cpp` -- deliverable
  source (Batch 1's six functions with CLOSE_C's constructor fix; the
  destructor and `spaceCheck` are unchanged from CLOSE_C/CLOSE_B since
  neither closed this round), compiles against the **real, unmodified**
  header.
- `wip/nand_thread/scratch/closer_e/verify_final.py` -- compiles the
  deliverable against the real header, diffs the constructor, both weak
  destructors and the destructor by address, prints emitted order with
  expected sizes.
- `wip/nand_thread/scratch/closer_e/probes/p1.cpp`,
  `run_probe.py` -- the minimal two-class destructor reproduction.
- `wip/nand_thread/scratch/closer_e/probes/p2.cpp`, `p3.cpp` -- member/
  virtual-count extensions of the minimal reproduction.
- `wip/nand_thread/scratch/closer_e/probes/megafile.cpp` -- the ~20-function
  whole-TU research probe (never a deliverable, see above).
- `wip/nand_thread/scratch/closer_e/probes/battery.py` -- the 8 destructor
  knob-turning variants (virtual-ness, body-statement shape), with output.
- `wip/nand_thread/scratch/closer_e/probes/sc_run.py`,
  `sc_battery.py` -- the `spaceCheck` baseline reproduction and the 8 new
  signature/type variants, with output.
- `wip/nand_thread/scratch/closer_e/shadow_include/` -- shadow copies of
  `d_nand_thread.hpp` and `revolution/NAND/NANDCheck.h` used for every
  signature-variant test above; never applied to the real tree.

No shared header, `slices/wiimj2d.json`, or `syms.txt` was edited. `ninja`,
`configure.py`, `progress.py` and `land.py` were never invoked.
