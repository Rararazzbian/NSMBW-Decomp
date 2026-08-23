# dWmBgmSync_c work order response

## Headline

**2/5 at `DIFFS 0`**.

Exact matches:

- `getAnmRate__12dWmBgmSync_cFf`: `DIFFS 0`
- `__sinit_\\d_wm_bgm_sync_cpp`: `DIFFS 0` apart from 10 expected naming artifacts

Not yet exact:

- `execute__12dWmBgmSync_cFv`: `DIFFS 48`
- `fn_80102F10`: `DIFFS 19`
- `__arraydtor$53530`: not emitted by this draft

The draft was compiled with the supplied `scratch/qwen_bgm/build.py`, producing `scratch/qwen_bgm/d_wm_bgm_sync.o` and `scratch/qwen_bgm/d_wm_bgm_sync.txt`.

## Header proposal

Copied and changed only in:

`scratch/qwen_bgm/shadow/game/bases/d_wm_bgm_sync.hpp`

Changes:

```diff
-    virtual void execute();
+    virtual bool execute();

     float getAnmRate(float frameCount);
+
+private:
+    float fn_80102F10();
+
+public:
@@
-    u8 mPad2[0x8];
+    f32 m_10;
+    f32 m_14;
     const s16 *m_18;
```

The two floats replace the existing 8-byte padding at offsets `0x10` and `0x14`; total class size and all following offsets are unchanged. `execute` is declared `bool` because the retail body returns `0` for a null `m_18` and `1` on every non-null path. The helper is private and takes `this`, matching the local call relocations.

## `execute__12dWmBgmSync_cFv`

- Evidence: target is 268 bytes, frame `0x10`, saves only `r31`; target writes live floats at `0x10` and `0x14`, and returns `0/1` in `r3`.
- Proposal: implemented in `scratch/qwen_bgm/d_wm_bgm_sync.cpp` with the target branch structure, beat counter updates, elapsed-frame accumulator, next-beat value, accent sign, and boolean return.
- Compiled: YES. Draft shape is 67 words, frame `0x10`, GPR `[31]`, FPR none.
- Result: `DIFFS 48`.
- Verbose residual: the target enters the non-null path with `bne`, calls `getBgmBeatTrg` immediately, and uses `clrlwi.`. The draft takes the complementary `beq` path and places the early return before the call. The target later uses `clrlwi.` followed by a branch and explicit `li 1`/`li 0` stores for the accent value; the draft's `u8` declaration produces a boolean normalisation sequence (`neg`, `or`, `srwi`).
- Confidence: medium on the behavioral reconstruction; low on the exact source shape until the audio declarations and return/control-flow idiom are aligned with the landed project declarations.
- Offset-perturbing: NO. The member replacement is exactly 8 bytes and the draft frame matches the target.

## `getAnmRate__12dWmBgmSync_cFf`

- Evidence: target is 76 bytes, frame `0x20`, saves `r31` and `f31`; it calls `getBgmTempo`, calls the helper, then divides `frameCount` by the helper result.
- Proposal: `dAudio::getBgmTempo(); return frameCount / fn_80102F10();`
- Compiled: YES.
- Result: `DIFFS 0`.
- Confidence: high.
- Offset-perturbing: NO.

## `fn_80102F10`

- Evidence: target is 116 bytes, frame `0x20`, saves `r31`; it calls `getBgmTempo`, masks the low 16 bits, reads `m_18[0]` as `s16`, and performs the standard MWCC integer-to-double conversions for `3600.0f * tempo / beat`.
- Proposal: implemented as:

```cpp
float dWmBgmSync_c::fn_80102F10() {
    int tempo = dAudio::getBgmTempo();
    const s16 *beat = m_18;
    return 3600.0f * (f32)(tempo & 0xffff) / (f32)*beat;
}
```

- Compiled: YES. Draft shape is 29 words, frame `0x20`, GPR `[31]`, FPR none.
- Result: `DIFFS 19`.
- Verbose residual: target order after the call is `lwz r4, 0x18(r31)`, `lis r5, 0x4330`, `clrlwi r3, r3, 16`, then stores/converts; the draft performs `clrlwi` before loading `m_18`, uses different stack slots and FPR assignment, and reverses the final arithmetic register order. Size and saved-register shape match exactly.
- Confidence: medium on arithmetic semantics, low on exact MWCC expression/declaration shape. The next useful experiment is to vary declaration order and split the masked tempo / beat conversions while preserving the target's required stack-store order.
- Offset-perturbing: NO. The function has the target size and frame.

## `__sinit_\\d_wm_bgm_sync_cpp`

- Evidence: including `<game/bases/d_wm_lib.hpp>` emits the static initializer; target and draft are both 28 words, frame `0x20`, no saved GPR/FPR registers.
- Compiled: YES.
- Result: `DIFFS 0` with 10 naming artifacts for draft-local static symbol names.
- Confidence: high. The initializer instruction sequence is byte-identical apart from draft symbol naming.
- Offset-perturbing: NO.

## `__arraydtor$53530`

- Evidence: target is 28 bytes at `0x80103000`, calls `__destroy_arr` with element size `0x24` and count `1` for `dWmLib::sc_ForceList`.
- Proposal: no hand-written destructor, per the work order.
- Compiled: NO: the current draft disassembly contains the initializer and `.ctors` entry but no emitted `__arraydtor$53530` symbol.
- Confidence: low on the missing emission mechanism. The include trick successfully emits `__sinit`, but this standalone draft does not reproduce the separately placed array-destructor object.
- Offset-perturbing: not applicable; no destructor object was emitted.

## Verification commands

```text
python -c "import sys; sys.path.insert(0,'scratch/qwen_bgm'); import build; print(build.build('d_wm_bgm_sync.cpp','d_wm_bgm_sync','execute__12dWmBgmSync_cFv'))"
python tools/auto_decomp/fndiff.py scratch/qwen_bgm/target.txt scratch/qwen_bgm/d_wm_bgm_sync.txt --all
```

No `ninja`, `configure.py`, `progress.py`, `land.py`, `syms.txt`, `include/**`, or `source/**` files were modified.

## Landing readiness

**Not landing-ready yet.** The header proposal is complete and two functions plus the initializer are proven exact, but `execute` and `fn_80102F10` still have instruction residuals and the required array destructor is absent from the draft object. The current artifacts and verbose residuals identify the remaining source-shape/API-declaration work.
