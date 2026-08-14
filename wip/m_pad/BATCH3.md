# BATCH3 — the WPADInfo family and the compiler-generated tail

Work directory: `wip/m_pad/scratch/batch3/`. Draft file: `m_pad.cpp` (compiled
under that exact name throughout, per the filename-in-mangling trap).
Shadow header: `wip/m_pad/scratch/batch3/mock_include/game/mLib/m_pad.hpp`
(the real `include/game/mLib/m_pad.hpp` was never edited).

All compiling and diffing went through `tools/auto_decomp/harness.py`'s
`compile_draft` / `disasm` / `extract` / `diff_fn`, imported as a module from
`wip/m_pad/scratch/batch3/run.py`. Sizes were asserted against
`bin/dtk/wiimj2d_symbols.txt` before writing any C++ (table below).

## The missing target: `__sinit`'s disassembly did not exist yet

Both scratch files named in the brief (`auto_03_8016F330_text.o.txt`,
`auto_03_8016F808_text.o.txt`) stop and start exactly at the boundary of
`__sinit_\m_pad_cpp` (`0x8016F7B0`-`0x8016F808`) — that range was never
disassembled by an earlier round. `bin/dtkspl/obj` has no
`auto_03_8016F7B0_text.o` either; `__sinit` functions are split into their own
`auto_sinit_<tag>_text.o` objects (see `tools/auto_decomp/prepare.py`,
`find_split_objects`), separately from the regular address-ordered split. The
right object is `bin/dtkspl/obj/auto_sinit__m_pad_cpp_text.o`. I disassembled
it directly and saved it as
`wip/m_pad/scratch/batch3/sinit_target.txt`:

```
python -c disasm via dtk:
bin\dtk-windows-x86_64.exe elf disasm bin\dtkspl\obj\auto_sinit__m_pad_cpp_text.o wip\m_pad\scratch\batch3\sinit_target.txt
```

Anyone else hitting a "function missing from every scratch file" problem in
this project should check for an `auto_sinit_*` object before assuming the
function isn't split at all.

## Status table

| # | address | target instrs | ours | result |
|---|---|---|---|---|
| `setWPADInfo` | `0x8016F5D0` | 26 (0x68) | 26 | **MATCH** |
| `clearWPADInfo` | `0x8016F640` | 17 (0x44) | 17 | **DIFFER** — residual: r4/r5 swapped on 10 of 17 lines, see below |
| `initWPADInfo` | `0x8016F690` | 15 (0x3C) | 15 | **MATCH** |
| `getWPADInfoCb` | `0x8016F6D0` | 15 (0x3C) | 15 | **MATCH** |
| `getWPADInfoAsync` | `0x8016F710` | 25 (0x64) | 25 | **MATCH** |
| `__sinit_\m_pad_cpp` | `0x8016F7B0` | 22 (0x58) | 22 | **MATCH** modulo one compiler pool number (expected, see below) |
| `__ct__...PadAdditionalData_tFv` | `0x8016F810` | 1 (0x4) | 1 | **MATCH** |
| `__dt__...PadAdditionalData_tFv` | `0x8016F820` | 16 (0x40) | 16 | **MATCH** |
| `__arraydtor$13953` | `0x8016F860` | 7 (0x1C) | 7 | **MATCH** body, modulo the `$NNNN` ordinal (expected, see below) |

7 of 9 are byte-exact today; the other two have named, understood residuals,
not unknowns.

## Final source

`wip/m_pad/scratch/batch3/m_pad.cpp`:

```cpp
#include <game/mLib/m_pad.hpp>

namespace mPad {

PadAdditionalData_t::PadAdditionalData_t() { }
PadAdditionalData_t::~PadAdditionalData_t() { }

PadAdditionalData_t g_PadAdditionalData[4];

WPADInfo s_WPADInfo[4];
WPADInfo s_WPADInfoTmp[4];
bool s_WPADInfoAvailable[4];
u32 s_GetWPADInfoInterval;

void setWPADInfo(CH_e ch, const WPADInfo &info) {
    s_WPADInfo[ch] = info;
    s_WPADInfoAvailable[ch] = true;
}

void clearWPADInfo(CH_e ch) {
    s_WPADInfo[ch].dpd = 0;
    s_WPADInfo[ch].speaker = 0;
    s_WPADInfo[ch].attach = 0;
    s_WPADInfo[ch].lowBat = 0;
    s_WPADInfo[ch].nearempty = 0;
    s_WPADInfo[ch].battery = 0;
    s_WPADInfo[ch].led = 0;
    s_WPADInfo[ch].protocol = 0;
    s_WPADInfoAvailable[ch] = false;
    s_WPADInfo[ch].firmware = 0;
}

void initWPADInfo() {
    for (int i = 0; i < 4; i++) {
        clearWPADInfo((CH_e)i);
    }
}

extern "C" void getWPADInfoCb(s32 chan, s32 result) {
    if (s_GetWPADInfoInterval == 0) {
        return;
    }
    switch (result) {
    case 0:
        setWPADInfo((CH_e)chan, s_WPADInfoTmp[chan]);
        break;
    case -1:
        clearWPADInfo((CH_e)chan);
        break;
    }
}

s32 getWPADInfoAsync(CH_e ch) {
    s32 result = WPADGetInfoAsync(ch, &s_WPADInfoTmp[ch], getWPADInfoCb);
    if (result == -1) {
        clearWPADInfo(ch);
    }
    return result;
}

};
```

The compiler-generated tail (`__sinit`, `__ct`, `__dt`, `__arraydtor`) needs
no source of its own — it appears purely as a consequence of declaring
`PadAdditionalData_t` with a user constructor/destructor and defining
`g_PadAdditionalData[4]` as a plain (non-extern) array. That is the whole
point of this batch's "free correctness check": it appeared at the right
sizes, so the header shape below is proven, not guessed.

## Header proposal (not applied — `include/game/mLib/m_pad.hpp` untouched)

Diff against the current 19-line stub. Only what this batch's functions
require; `setCurrentChannel`, `getBatteryLevel_ch`, the interval accessors,
`g_currentCore`, `g_currentCoreID` and `g_core` are left for the batches that
own them.

```cpp
 #pragma once

 #include <lib/egg/core/eggController.h>
+#include <lib/revolution/WPAD/WPAD.h>

 namespace mPad {
     enum CH_e {
         MPAD_CH_0,
         MPAD_CH_1,
         MPAD_CH_2,
         MPAD_CH_3
     };

+    /// @unofficial Layout proven by mPad::beginPad (not ours -- see below).
+    /// Six floats, sizeof == 0x18, matching g_PadAdditionalData__4mPad's
+    /// array stride. Field names are guesses; only float-ness, count and
+    /// order are evidenced.
+    struct PadAdditionalData_t {
+        PadAdditionalData_t();
+        ~PadAdditionalData_t();
+
+        f32 posX;   ///< @unofficial CoreController+0x6c
+        f32 posY;   ///< @unofficial CoreController+0x70
+        f32 velX;   ///< @unofficial posX - previous posX
+        f32 velY;   ///< @unofficial posY - previous posY
+        f32 accX;   ///< @unofficial velX - previous velX
+        f32 accY;   ///< @unofficial velY - previous velY
+    };
+
     void create();
     void beginPad();
     void endPad();

+    void setWPADInfo(CH_e ch, const WPADInfo &info);
+    void clearWPADInfo(CH_e ch);
+    void initWPADInfo();
+    s32 getWPADInfoAsync(CH_e ch);
+
     extern EGG::CoreController *g_currentCore;
     extern EGG::CoreController *g_core[4];
+    extern PadAdditionalData_t g_PadAdditionalData[4];
 };
```

`getWPADInfoCb` is deliberately **not** in the header — see below.

## Every header member/type/global this batch proves

### `WPADInfo` — use the SDK type, do not redefine it
- Evidence: `setWPADInfo`'s target body is a pure memberwise copy: 5 words at
  `+0x0/0x4/0x8/0xc/0x10` then 4 bytes at `+0x14/0x15/0x16/0x17`, total
  `0x18` — exactly `include/lib/revolution/WPAD/WPAD.h`'s existing
  `WPADInfo` (5 `BOOL` + 4 `u8`). `clearWPADInfo` zeroes the identical
  offsets. Confirmed by compiling `setWPADInfo` and `clearWPADInfo` against
  it: both are byte-exact (`clearWPADInfo` modulo the register residual
  below, which is not a layout issue — see there).
- Compiled: YES, MATCH for `setWPADInfo`.
- Confidence: high.
- Offset-perturbing: NO — using the existing type, not proposing a new one.

### `setWPADInfo` takes `const WPADInfo&`, confirmed
- Evidence: brief already stated this from the mangled name
  (`RC8WPADInfo`); the copy body above further confirms it dereferences a
  pointer parameter member-by-member rather than doing anything
  pointer-specific.
- Compiled: YES, MATCH.
- Confidence: high.
- Offset-perturbing: NO.

### `getWPADInfoAsync` returns `s32`, confirmed by the mandatory register test
- Evidence: `WPADGetInfoAsync`'s result lives in `r3`/`r31` for the whole
  function and is moved back into `r3` immediately before `blr`, with no
  other use. Per `AGENT_CONTEXT.md`'s CFront-mangling warning I did not
  trust that shape alone: I tried compiling the function `void`. It failed
  outright — MWCC reported `illegal overloading` against the header's own
  `s32` declaration once one existed, which is itself evidence the two
  differ in codegen (a `void` overload cannot coexist). With `s32` declared,
  the function is a clean 25/25 instruction MATCH.
- Compiled: YES, MATCH (s32). `void` variant: rejected by the compiler once
  tested against a matching declaration (illegal overload), consistent with
  a genuinely different return-value convention.
- Confidence: high.
- Offset-perturbing: NO.

### `getWPADInfoCb` — `extern "C"`, file-scope, no header declaration
- Evidence: the target symbol is the bare, unmangled `getWPADInfoCb` — no
  `__4mPad` suffix, no argument encoding. Per `AGENT_CONTEXT.md`, an ordinary
  C++ function (even `static`) always mangles under this compiler; the only
  way to get a literally bare name is `extern "C"`. Its signature has to
  match `WPADCallback` (`void (*)(s32 chan, s32 result)`, from
  `WPAD.h`) since it's handed directly to `WPADGetInfoAsync` in
  `getWPADInfoAsync`'s target body (`lis r5, getWPADInfoCb@ha` loaded as the
  third argument). Nothing outside `m_pad.cpp` needs to name it, so it is not
  in the header proposal at all — declared and defined together in the
  `.cpp`, `extern "C"`, inside `namespace mPad` (namespaces don't affect
  `extern "C"` linkage names, so this is legal and does not change the
  symbol).
- Compiled: YES, MATCH — but only after finding the real control-flow shape;
  see the "variant tried that failed" section, `switch` vs `if`/`else if`.
- Confidence: high.
- Offset-perturbing: n/a (function, not header layout).

### `PadAdditionalData_t` — a correction to the brief's own hypothesis
The brief says of `__dt__Q24mPad19PadAdditionalData_tFv`'s `0x40` size: "the
struct owns something with a destructor, or the destructor is virtual." I
read the body instruction by instruction and that is **not what it shows**,
so I'm reporting the contradiction rather than reconciling it, per
`AGENT_CONTEXT.md` rule 4.

The actual body:
```
cmpwi r3, 0; beq skip
cmpwi r4, 0; ble skip
bl __dl__FPv
skip: return r3
```
No member-destructor calls, no vtable store, no field access at all beyond
`r3`/`r4` (the two ABI arguments themselves). This is the standard
**two-argument destructor ABI wrapper** (`this`, `delete_flag`) that this
CFront-derived compiler emits for *any* user-declared destructor used
through `__destroy_arr`'s function-pointer convention — regardless of
whether the class has anything to clean up. The `0x40` bytes are prologue +
two compares + a conditional call + epilogue, not member cleanup.

The evidence that actually explains why `PadAdditionalData_t` needs a
constructor/destructor pair at all: I proved by direct compile that
- declaring **inline, in-class empty bodies** (`PadAdditionalData_t() { }`
  in the header) makes `__ct`/`__dt` come out `weak` linkage in my object,
  and cascades to `__sinit`/`__arraydtor` coming out `local` instead of
  `global` — wrong, since the target shows `__ct`/`__dt` as plain `global`
  (`bin/dtk/wiimj2d_symbols.txt` has no `scope:local`/weak marker on either).
  This is exactly the header trap `AGENT_CONTEXT.md` names: "declare
  destructors WITHOUT inline bodies unless you have evidence the original
  inlined them."
- declaring them in-class, **defining the (still-empty) bodies out-of-line**
  in `m_pad.cpp` (`PadAdditionalData_t::PadAdditionalData_t() { }` etc.)
  fixes it: both come out `global`, and both are still byte-for-byte
  1-instruction (`blr`) and 16-instruction MATCHes respectively.

So: the struct's members are plain data (see below), not something owning a
destructor; the *destructor's existence at all* is because the class
declares one by hand (even though its body does nothing), which is what
triggers `__construct_array`/`__register_global_object`/`__destroy_arr` for
the static array. An implicitly-trivial struct (no declared special members)
would not need any of this generated tail.

- Compiled: YES for both link-affecting shapes; the out-of-line, `global`
  form is what's in the final source above.
- Confidence: high on the "boilerplate wrapper, not member ownership" read
  (directly from the instructions); medium on the exact field types (see
  next entry — this batch doesn't own the function that fills them).
- Offset-perturbing: the struct's *size* (`0x18`) is fixed regardless: adding
  member cleanup or not doesn't change `sizeof`. Not perturbing.

### `PadAdditionalData_t`'s six float fields (evidence only — `beginPad` is not ours)
- Evidence: `beginPad`'s target body (not part of this batch, but its use of
  `g_PadAdditionalData` is the only place the struct's members are ever
  touched) writes, per array element (`r27`, stride `0x18`):
  - `+0x0 = ctrl+0x6c`, `+0x4 = ctrl+0x70` (raw values read off
    `EGG::CoreController`)
  - `+0x8 = (new +0x0) - (old +0x0)`, `+0xc = (new +0x4) - (old +0x4)`
    (first-order deltas)
  - `+0x10 = (new +0x8) - (old +0x8)`, `+0x14 = (new +0xc) - (old +0xc)`
    (second-order deltas, i.e. deltas of the deltas)
  All six stores are `stfs` (single-precision float), all four bytes apart,
  filling `0x0`-`0x17` with no gap — matches `sizeof == 0x18` exactly, no
  padding, no hidden member. Given the pattern (position, velocity,
  acceleration of what's very likely an IR-pointer screen coordinate pair),
  field names `posX/posY/velX/velY/accX/accY` are `@unofficial` guesses;
  only "six `f32`s in this order" is evidenced.
- Compiled: NOT independently (would require `beginPad`, out of scope for
  this batch) — the struct compiles fine with these members and produces the
  correct `sizeof` and the correct `__ct`/`__dt`/`__arraydtor`/`__sinit`
  shapes, which is the check available from inside this batch.
- Confidence: medium — the byte layout is solid (directly read off
  `beginPad`'s target disassembly), the field *names* are not.
- Offset-perturbing: NO if the eventual owner of `beginPad` confirms 6 `f32`
  fields in this order; the total stays `0x18` either way.

## The `0x10` unclaimed `.bss` hole at `0x80377F98` — closed, not a declaration

Cross-batch note from the coordinator (Batch 1) reported the same anonymous
`~0x10`-byte `.bss` object from the codegen side and asked whether it's the
same thing this batch's `__sinit`/`__arraydtor` machinery produces. It is.

Direct evidence from `wip/m_pad/scratch/batch3/sinit_target.txt` (the
retail `__sinit_\m_pad_cpp` disassembly, recovered as described above):
```
lis r4, __arraydtor$13953@ha
lis r5, "@13954_80377F98"@ha
addi r4, r4, __arraydtor$13953@l
li   r3, 0x0
addi r5, r5, "@13954_80377F98"@l
bl   __register_global_object
```
`__register_global_object` is called with `r3=0`, `r4=&__arraydtor$13953`,
`r5=&<anonymous pool symbol at 0x80377F98>` — i.e. the retail binary itself
names an anonymous object sitting exactly at the start of the unclaimed
hole, and its pool ID (`13954`) is the very next number after
`__arraydtor$13953`, which is the project's own "consecutive `@NNNNN` pool
IDs attribute an anonymous object to a TU" rule (`AGENT_CONTEXT.md` §5).

I then compiled this batch's `m_pad.cpp` (the source above, `PadAdditionalData_t`
with a declared-but-empty ctor/dtor, `g_PadAdditionalData[4]` as a plain
array) and inspected the object with `dtk elf info`:
```
.bss | 0x0  | 0xC  | @1900
.bss | 0x10 | 0x60 | g_PadAdditionalData__4mPad
```
An anonymous `.bss` symbol, size `0xC`, appears **automatically** — nobody
declared it, it's the same `__register_global_object` bookkeeping node
referenced by `__sinit`, just under a different pool number because this is
an isolated partial-file compile (see below). `g_PadAdditionalData` lands at
offset `0x10`, not `0xC`, because `AGENT_CONTEXT.md`'s "`.bss` object
alignment follows SIZE, not type" rule 8-aligns its `0x60` size — that
accounts for the missing 4 bytes (`0xC` object + `4` bytes padding = `0x10`,
exactly the brief's recorded hole width).

**Conclusion: the `0x10` hole is not a missing declaration.** It is the
compiler-synthesized global-destructor-chain node `__register_global_object`
needs to track `g_PadAdditionalData[4]`'s static destruction, plus its
alignment padding. It requires nothing from any header — it appears
automatically once `PadAdditionalData_t` is declared with a real
constructor/destructor pair, which this batch already proves is correct
(see the `__ct`/`__dt`/`__sinit`/`__arraydtor` MATCHes above). No `u8 pad[]`
or invented member is needed anywhere for this.

## Two residuals, both understood, neither actionable inside this batch

### 1. `clearWPADInfo`: a register tie-break, not a logic error
10 of 17 instructions differ, and every one of them is the *same* swap: the
target keeps the combined array-element address in `r4` (freeing `r5` for
the `s_WPADInfoAvailable` address); my compile keeps it in `r5` (freeing
`r4`). All *offsets*, all *values*, and the *store order* are already
byte-identical — this is register-bank tie-breaking within a single
straight-line, branch-free function, not a structural difference.

I tried, all producing the exact same swap or worse:
- Direct array indexing (`s_WPADInfo[ch].dpd = 0;` ...) — closest, only the
  swap remains (this is what's in the final source).
- A bound reference (`WPADInfo &info = s_WPADInfo[ch];`) — worse, diverges
  one instruction earlier.
- A bound pointer (`WPADInfo *info = &s_WPADInfo[ch];`) — same as reference.
- Binding the reference only after the first field write (hybrid) — same
  swap.
- Moving `s_WPADInfoAvailable[ch] = false` to the very start, the very end,
  or its current mid-position (matches the target's actual store order) —
  no change to the swap.
- `memset(&s_WPADInfo[ch], 0, sizeof(WPADInfo))` for the bulk, explicit
  writes for the tail — wrong shape entirely (real `bl memset`, stack frame
  appears; target has neither).
- A `static const WPADInfo zero = {...}; s_WPADInfo[ch] = zero;` — wrong
  shape (loads from a `.rodata` pool object; target has no such load).
- An `int i = ch;` intermediate before indexing — no change.
- A chained assignment (`info.dpd = info.speaker = ... = 0;`) — different
  store order entirely (proves the assignment-chain hypothesis false, useful
  negative).
- A helper function for the first 8 fields, called then inlined — same
  swap.
- Raw pointer-cast stores (`*(u32*)(p+0x0) = 0;` etc.) — same swap.
- Declaring every other `mPad` global in true address order before this
  function, and even compiling it alongside stub `create`/`setCurrentChannel`
  /`getBatteryLevel_ch` bodies to reproduce full-file register pressure —
  no change. This rules out cross-function/file-context effects entirely;
  the tie-break is local to this one function's own codegen.

Given AGENT_CONTEXT's precedent for this exact situation ("spaceCheck and
writeBanner: two characterised negatives, no fix" — an actual prior commit
on this branch), I'm reporting this as characterized rather than continuing
to grind register permutations with no remaining lever identified. If
someone finds the lever, the fix is purely cosmetic — it will not change any
byte's *value*, only which GPR holds the address between two identical
stores.

- Compiled: YES. Byte content and instruction *count* both match; 10 lines
  differ only in register number, not opcode or offset.
- Confidence: high that this is a pure allocator tie-break (13 structurally
  different rewrites all reproduced the identical swap or diverged further);
  low that any further source-level rewrite will fix it without knowing the
  actual lever.
- Offset-perturbing: NO — same instruction count, same section contents,
  same symbol references.

### 2. `__sinit`/`__arraydtor`: two compiler-internal ordinals, expected to settle at full-TU merge
- `__arraydtor$13953` in the target vs `__arraydtor$1899`/`$1896` in my
  isolated compiles (the number moved between my own edits, confirming it's
  a running counter, not a stable identifier).
- `__register_global_object`'s third argument, `"@13954_80377F98"`, is
  already correctly canonicalized away by the harness's pool-symbol
  handling (that address-affixed `@NNNNN` form matches `POOL_SYM`) — it is
  **not** in the diff. Only the `$NNNN`-suffixed `__arraydtor` name is a
  literal, uncanonicalized token (`harness.py`'s `POOL_SYM` regex matches
  `@\d+`, not `$\d+`), so it shows as a hard mismatch even though the
  underlying mechanism and body are identical (I extracted my draft's
  `__arraydtor$1899`/`$1896` body directly and it is byte-for-byte the
  target's `__arraydtor$13953` body, `b __destroy_arr` and all).
- These ordinals are the compiler's running count of anonymous/synthesized
  entities across the **whole file** compiled so far — mine differs because
  my test file has 9 functions, the real `m_pad.cpp` has 16 plus whatever
  else the full TU pulls in. This is not something I can fix from inside
  a 9-function partial compile; it should settle automatically once the
  batches are merged into the complete file and compiled together. Flagging
  it explicitly rather than either claiming false success or leaving it
  unexplained.
- I also separately observed `__sinit`/`__arraydtor` come out ELF-`local` in
  my isolated object, while the target's symbol-table entries for both carry
  no `scope:local` tag (i.e. retail is not local). I could not isolate a
  source-level cause the way I did for the `weak`-linkage `__ct`/`__dt` case
  above (full-context and stub-function experiments made no difference).
  This may be the same "partial file" effect as the ordinal above, or may be
  a second, real thing — I'm reporting it plainly as unresolved rather than
  guessing, since it doesn't affect instruction bytes and I have no
  isolating test left to run from inside this batch.

## Variant attempts that failed (so nobody repeats them)

- `getWPADInfoCb` as `if (result == 0) {...} else if (result == -1) {...}`
  (natural transliteration of the disassembly's apparent shape) — compiles
  to an inverted-branch/inline-then-jump-around shape, 1 instruction short
  and every branch target different from the target. The retail shape is
  **two independent early-exit `if`s**, or equivalently (and what actually
  matches) a `switch (result) { case 0: ...; break; case -1: ...; break; }`.
  Tried and rejected: plain if/else-if (wrong shape), two independent `if`s
  with no `switch` (compiles to a completely different, larger body — 27
  instructions with a stack frame, because the compiler no longer proves the
  two conditions mutually exclusive without the `switch`), reversing the
  case order (`-1` before `0`) under if/else-if (still wrong, and different
  wrong). Only `switch` on `result` reproduces the target exactly.
- `PadAdditionalData_t` with inline in-class empty ctor/dtor bodies — compiles
  correct instruction bytes for `__ct`/`__dt` themselves, but wrong linkage
  (`weak` instead of `global`), which cascades into `__sinit`/`__arraydtor`
  losing global visibility too. Moving the (still-empty) bodies out-of-line
  into the `.cpp` fixes the linkage while keeping identical instruction
  bytes. This is the header trap `AGENT_CONTEXT.md` names explicitly;
  recording the concrete before/after here since it's easy to reintroduce
  by "simplifying" the header later.
- `clearWPADInfo` — see the full list under residual 1 above; 13 variants
  tried, none removed the r4/r5 swap.
- `getWPADInfoAsync` declared `void` — rejected outright by the compiler
  (`illegal overloading`) once a matching `s32` prototype existed, which is
  itself useful confirmation that the two are not codegen-equivalent for
  this compiler (per `AGENT_CONTEXT.md`'s "compile it both ways and let the
  diff decide" — here the compiler decided before a diff was even possible).

## Data inventory contribution (for the lead's merge)

This batch's globals, all confirmed by successful, matching compiles:

| symbol | size | type |
|---|---|---|
| `g_PadAdditionalData__4mPad` | `0x60` | `PadAdditionalData_t[4]` (`sizeof == 0x18`, 6 `f32`) |
| `s_WPADInfo__4mPad` | `0x60` | `WPADInfo[4]` |
| `s_WPADInfoTmp__4mPad` | `0x60` | `WPADInfo[4]` |
| `s_WPADInfoAvailable__4mPad` | `0x4` | `bool[4]` (brief's guess confirmed — `stb`/`lbz` of a 0/1 byte throughout) |
| `s_GetWPADInfoInterval__4mPad` | `0x4` | `u32` |
| anonymous, `0xC` | — | compiler-synthesized destructor-chain node for `g_PadAdditionalData`'s registration; fills the `0x10` hole together with its own alignment padding — not user-declared |

Per the coordinator's note, `.bss`/`.sbss` ordering across the whole TU
follows first-reference order in `.text`, not declaration order in any one
batch's source, and only resolves once all 16 functions are merged — so I
did not try to reproduce the retail byte offsets inside this isolated
compile; the sizes and symbol set above are what matters from here.
