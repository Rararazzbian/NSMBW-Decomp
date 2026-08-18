 # Original local type analysis
 
 ## Evidence
 
 The target uses only two-float stack objects and emits no constructor or destructor calls for them. The stack offsets below are relative to r1 after the prologue.
 
 incCoin (0x80060250):
 
 - The map center is computed as yStart - ySize * 0.5f, then xStart + xSize * 0.5f. The resulting values are stored in 0x24 first (the y expression) and 0x20 second (the x expression).
 - The address r1+0x20 is passed to cvtSndObjctPos__6dAudioFRC7mVec2_c. The converted result is written to 0x18, then r1+0x18 is passed to startSound__14SndObjctCmnMapFUlRCQ34nw4r4math4VEC2Ul.
 - The same input at 0x20 is converted again, producing 0x10, and r1+0x10 is passed to the second startSound call.
 - There are no reads from the original two-float object after conversion in this function.
 
 addRest (0x80060460):
 
 - The same map-center calculations occur, with y stored at 0x24 and x at 0x20, in that order.
 - r1+0x20 is passed as const mVec2_c& to cvtSndObjctPos; the result is stored at 0x18 and passed as const nw4r::math::VEC2& to startSound.
 - The input is converted a second time, producing 0x10, which is passed to the second startSound.
 - The two-float input is otherwise not read by the generated code.
 
  deleteCullingYoshi (0x80060AB0):
 
 - mid.y is stored at 0x1c, then mid.x at 0x18. These are read later from those same slots.
 - For each eligible Yoshi, position y from mPos+0xb0 and x from mPos+0xac are stored at 0x10 and 0x14 respectively.
 - Distance x delta is stored at 0x08 and y delta at 0x0c, then immediately used to form the squared distance.
 - The target does not call any vector helper for this calculation. The stack stores are ordinary stfs operations and the arithmetic is scalar floating-point arithmetic.
 
 The draft currently spells these locals as mVec2_c: pos in incCoin and addRest, and mid, ppos, and delta in deleteCullingYoshi.
 
 ## Candidate
 
 mVec2_c remains the most likely source-level type for the two locals passed to the audio conversion. The target call name is explicit: cvtSndObjctPos(const mVec2_c&). In the current headers, mVec2_c is the type used by playMapSound and supplies the conversion to nw4r::math::VEC2 needed by startSound. A plain float, a two-float anonymous struct, and nw4r::math::VEC2 cannot naturally produce that exact typed call without an explicit cast or a different overload.
 
 mVec2_POD_c is plausible from the raw code shape, especially for deleteCullingYoshi, because it is exactly two floats and has no user-declared destructor. However, it cannot bind to the present cvtSndObjctPos(const mVec2_c&) declaration without an API change or explicit cast.
 
 nw4r::math::VEC2 is layout-compatible and trivially destructible, and is the type consumed by startSound, but it is the output type of the conversion, not the input type named by the target call. Plain float locals or a struct with x/y members explain deleteCullingYoshi but not the audio calls without an alternate API or explicit casts.
 
 The mVec2_c definition has an empty user-provided destructor and constructors, but optimization can inline and remove empty operations. Absence of destructor-call instructions alone does not prove that the source type was trivially destructible.
 
 ## Confidence
 
 Moderate that the audio-position locals were mVec2_c, and low-to-moderate that the distance-calculation locals were specifically mVec2_c rather than scalar temporaries or a POD pair. The target does not contain enough type information to distinguish layout-compatible choices for mid, ppos, and delta.
 
 The alternative hypothesis is inconclusive overall and does not justify replacing mVec2_c in the draft. The strongest evidence against replacement is the explicit cvtSndObjctPos(const mVec2_c&) call in each audio path.
 
 What would settle it is an original source/header reference, a neighboring retail function passing a POD pair through the same conversion API, or a controlled harness build showing that a candidate declaration produces the exact calls and stack spills while preserving symbols. A mVec2_POD_c test would need an API-compatible conversion path to be a valid source-level alternative.
 
 ## Impact on the two spurious EGG destructor symbols
 
 This analysis does not justify removing the two EGG vector destructor symbols by changing these locals to POD types. The target's lack of destructor calls is compatible with optimized empty mVec2_c cleanup, while the audio calls identify mVec2_c as the natural input type. If those symbols are truly spurious, their cause is elsewhere, such as compiler-emitted weak out-of-line destructor bodies or another use in the translation unit, rather than these three locals alone.
 
