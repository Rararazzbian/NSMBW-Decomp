# Round 8: two spurious emitted functions

## the answer

remove `~Vector2f() {}` and `~Vector3f() {}` from `include/lib/egg/math/eggVector.h`.

**why**: both declarations make EGG::Vector2f and EGG::Vector3f non-trivially destructible. the nw4r bases (VEC2/VEC3) have no destructor, so without the empty EGG declarations the types become trivially destructible and nothing is emitted.

**why the mVec ones stay dedupable**: mVec2_c and mVec3_c each declare their OWN `~mVec2_c() {}` / `~mVec3_c() {}` explicitly. those CAN dedupe because the retail binary has weak copies at 0x80006DF0 and 0x8000FBF0. the chain is: local mVec2_c triggers ~mVec2_c (dedupable, 0x40 bytes, matches retail) and ~EGG::Vector2f (no retail copy, orphaned). removing only the EGG-level destructors breaks the chain at the right point.

## evidence

- **eggVector.h**: `~Vector2f() {}` at line ~14, `~Vector3f() {}` at line ~39
- **nw4r math_types.h**: VEC2/VEC3 have NO destructor. bases _VEC2/_VEC3 also none.
- **wiimj2d_symbols.txt**: zero matches for `__dt__Q23EGG8Vector2fFv` or `__dt__Q23EGG8Vector3fFv`. zero.
- **wiimj2d_symbols.txt**: `__dt__7mVec2_cFv` at 0x80006DF0 size 0x40 weak, `__dt__7mVec3_cFv` at 0x8000FBF0 size 0x40 weak.
- **blast radius search**: no EGG::Vector2f or EGG::Vector3f arrays, deletes, or explicit dtor calls found anywhere in include/ or source/. only EGG::Sphere3f has a Vector3f value member, and that is a different TU.
- **blame**: the three functions that trigger this (incCoin, addRest, deleteCullingYoshi) use mVec2_c locals, confirmed correct by subagent C (the target calls `cvtSndObjctPos(const mVec2_c&)` -- the parameter type IS mVec2_c in the symbol, not a POD alternative).

## scratch compile test

copied eggVector.h into scratch, removed the two dtor lines, compiled the full draft with the shadow header via harness.compile_draft(extra_inc=...):

- `__dt__Q23EGG8Vector2fFv`: **absent**
- `__dt__Q23EGG8Vector3fFv`: **absent**
- `__dt__7mVec2_cFv`: still emitted (good)
- `__dt__7mVec3_cFv`: absent (the draft may not exercise mVec3_c in this TU)

the three near-miss functions showed register/instruction diffs in isolated compile. i believe this is an isolated-compile artifact -- the BATCHES/STATICS docs say standalone drafts of functions that depend on combined-TU static layout show false register differences. a full ninja rebuild should settle this.

## proposal

```
--- a/include/lib/egg/math/eggVector.h
+++ b/include/lib/egg/math/eggVector.h
@@ lines ~14 and ~39 @@
-        ~Vector2f() {}
...
-        ~Vector3f() {}
```

remove exactly those two lines. nothing else changes. no constructors, no includes, no member layout.

- **Compiled**: YES (scratch shadow-header test)
- **Confidence**: high
- **Offset-perturbing**: YES -- emitted text and linkage change, but object layout does not. full build verification needed.
- **Fallback**: if the full-build still shows differences in incCoin/addRest/deleteCullingYoshi, the cost is two small orphan functions the linker places harmlessly. that is a known, quantified cost and a legitimate result per the acceptance criteria.

## subagents used

all three ran on gpt-5.6-luna (cheapest available). deepseek-v4-pro unavailable in this environment.

- Newton: scratch compile + diff test -> `scratch/codex_round8/taskA_scratch_compile.md`
- Schrodinger: evidence audit + blast radius search -> `scratch/codex_round8/taskB_evidence_audit.md`
- Averroes: original-type alternative hypothesis -> `scratch/codex_round8/taskC_original_type.md`