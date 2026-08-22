// Proposed additions to nw4r math headers.
//
// These types and functions exist in the retail binary but are not declared
// in the current include tree.  Derived from mangled names and target
// disassembly of d_bg_ctr.cpp (fn_80080900, fn_8007FFA0, addDokanMoveDiff).
//
// SEGMENT3 layout: two VEC3s (start, end) = 24 bytes.
// SPHERE layout:   VEC3 center + f32 radius = 16 bytes.
//
// IntersectionSegment3Sphere returns bool (consumed as cmpwi/beq).
// DistSqSegment3ToSegment3 returns f32 (consumed as fcmpo/f1).
// Atan2Idx returns s16 (consumed as add r0, r0, r3 where r3 is the return).

#ifndef PROPOSED_NW4R_MATH_GEOMETRY_H
#define PROPOSED_NW4R_MATH_GEOMETRY_H

#include <nw4r/types_nw4r.h>
#include <nw4r/math/math_types.h>

namespace nw4r {
namespace math {

struct SEGMENT3 {
    VEC3 start;  // at 0x00
    VEC3 end;    // at 0x0C
};

struct SPHERE {
    VEC3 center; // at 0x00
    f32 radius;  // at 0x0C
};

// IntersectionSegment3Sphere(seg, sphere, &t, &unused) → bool
// Returns true if the segment intersects the sphere.
// t = parameter along segment at intersection point.
// Second float output is unused in the call site.
bool IntersectionSegment3Sphere(const SEGMENT3 *seg, const SPHERE *sphere,
                                f32 *t, f32 *unused);

// DistSqSegment3ToSegment3(segA, segB, &t, nullptr) → f32
// Returns squared distance between two 3D segments.
// t = parameter along segA at closest point.
// Second float output is unused (pass nullptr).
f32 DistSqSegment3ToSegment3(const SEGMENT3 *segA, const SEGMENT3 *segB,
                             f32 *t, f32 *unused);

// Atan2Idx(y, x) → s16
// Returns a signed 16-bit angle index from two float args (y, x).
// NOT declared in math_triangular.h (which has Atan2FIdx/Atan2Deg/Atan2Rad).
extern s16 Atan2Idx(f32 y, f32 x);

} // namespace math
} // namespace nw4r

#endif