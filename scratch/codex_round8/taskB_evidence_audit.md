Evidence

- Current EGG declarations: EGG::Vector2f has an inline empty destructor, ~Vector2f() {}, at include/lib/egg/math/eggVector.h:14. EGG::Vector3f likewise has ~Vector3f() {} at line 39.
- Current mLib declarations: mVec2_c has ~mVec2_c() {} at include/game/mLib/m_vec.hpp:32, and mVec3_c has ~mVec3_c() {} at line 128. Both derive directly from the corresponding EGG vector.
- nw4r::math::VEC2 and VEC3 have no declared destructor. Their bases _VEC2 and _VEC3 also have no destructor. Removing the EGG destructor declarations makes both EGG vectors trivially destructible.
- The retail symbol map has no __dt__Q23EGG8Vector2fFv or __dt__Q23EGG8Vector3fFv entries, so neither has an address, size, or scope.
- __dt__7mVec2_cFv is .text:0x80006DF0, size 0x40, scope:weak. __dt__7mVec3_cFv is .text:0x8000FBF0, size 0x40, scope:weak.
- A local mVec2_c or mVec3_c causes emission of the derived mVec destructor and its inline EGG base destructor through the destructor chain. The mVec destructor can deduplicate against its matching retail weak symbol. The EGG destructor cannot deduplicate because neither EGG symbol exists in the retail map.
- The include/source search found 12 relevant textual matches in three headers: the EGG class definitions, mVec inheritance declarations, and EGG::Sphere3f's Vector3f value member at include/lib/egg/geom/eggSphere.h:12 and :14. No EGG vector arrays, locals, delete expressions, or explicit EGG destructor calls were found.
- assembled.cpp uses mVec2_c and mVec3_c locals, parameters, references, and a mVec3_c[28] object, but no explicit EGG destructor calls. This does not contradict removal.
- Blast radius is small in source terms. All TUs including these shared headers need recompilation, but no found use makes removal unsafe.

Proposal

Proposal only. Do not apply this diff to the real shared header.

Proposed exact header diff:

--- a/include/lib/egg/math/eggVector.h
+++ b/include/lib/egg/math/eggVector.h
@@
-        ~Vector2f() {}
@@
-        ~Vector3f() {}

This removes only the two empty EGG destructor declarations. It preserves layout and observed call-site types, should remove the unmatched EGG destructor emissions, and leaves the explicitly declared mVec destructors available for their retail weak symbols.

Compiled: N/A
Confidence: high
Offset-perturbing: YES

The object layout is not expected to change, but emitted text and linkage do change, so this is offset-perturbing until a permitted compile/object comparison confirms the final effect.

