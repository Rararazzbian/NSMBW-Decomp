# NO: no TU-only fix removes both EGG vector dtors while preserving all three bodies

## What I tried

I inspected wip/player_manager/assembled.cpp, the three target functions in wip/player_manager/target_text.txt, include/lib/egg/math/eggVector.h, and include/game/mLib/m_vec.hpp. The target uses raw stack slots containing two floats and passes their addresses to cvtSndObjctPos(const mVec2_c &). It emits no constructor or destructor calls in incCoin, addRest, or deleteCullingYoshi.

I tested TU-local shadow-header variants, without modifying shared headers or wip/:

- Declaring only mVec2_c::~mVec2_c() out-of-line removed its emitted destructor, but added real destructor calls at scope exits: incCoin grew by 9 instructions, addRest by 9, and deleteCullingYoshi by 12.
- Declaring EGG::Vector2f::~Vector2f() and EGG::Vector3f::~Vector3f() out-of-line as well removed the two EGG symbols, but also caused a new mVec3_c destructor emission and retained the same three function regressions.
- A POD wrapper or raw-float expression cannot satisfy cvtSndObjctPos(const mVec2_c&) without either an implicit/materialized mVec2_c temporary or an explicit reinterpretation/cast. The latter would change the source expression/codegen and is not a safe way to preserve byte identity.
- Replacing the locals with mVec2_c_POD_c/nw4r::math::VEC2-shaped storage is likewise not a transparent type substitution: the call still requires an mVec2_c reference, and a conversion would materialize the non-trivial type.

The existing target-shaped code already relies on MWCC seeing the inline empty destructor body and eliding every actual local destructor call. Removing the body hides the proof of emptiness, so the compiler emits calls even though the target has none.

## Result

No approach removed __dt__Q23EGG8Vector2fFv and __dt__Q23EGG8Vector3fFv without perturbing the three functions. The only reliable fix is the shared-header change that makes the vector types trivially destructible, but that changes banked code globally and is explicitly out of scope.

The exact target evidence is visible in target_text.txt: incCoin stores the two coordinates directly, then passes stack addresses at 0x18/0x20; deleteCullingYoshi does the equivalent at 0x18/0x1c, 0x10/0x14, and 0x08/0x0c. The current source locals compile to the same useful no-dtor call-site behavior, despite the TU-level weak EGG destructor emissions.

## Confidence

High. The shadow-header experiments were compiled and compared against the target-shaped output. Given the exact reference parameter and MWCC's need to see the inline destructor body to elide automatic-object destruction, a TU-only source workaround would necessarily alter one of the three function bodies or introduce a temporary/conversion.
