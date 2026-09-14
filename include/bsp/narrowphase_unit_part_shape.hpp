#pragma once
#include "bsp/hit_narrowphase.hpp"
#include <cstddef>

// The unit-part collision shape's element selection.
// docs/NARROWPHASE_UNIT_PART_SHAPE.md.
//
// 00723D60 is the step between `shape->vtable[0]` (00724510 -> 00723E90) and
// the per-element triangle test (00723AA0). It owns no geometry and no
// distance comparison: it walks every element of the mesh in index order and
// shortens the far endpoint to each accepted hit, so the element that survives
// is the closest one along the segment. Its return value becomes the element
// whose +4h and +8h 00723E90 copies into the hit record's +30h and +34h.
//
// The element stride and the element field offsets are NOT redeclared here;
// they live in bsp/hit_hull_segment.hpp, and the shape's own +1Ch/+20h/+24h in
// bsp/collision_shapes.hpp.

namespace bsp {

// ---------------------------------------------------------------------------
// A correction to bsp/hit_hull_segment.hpp's kGeomElementOffRejectScalar.
//
// That constant calls element+24h "a scalar, fed to 0085BF90, unread". It is a
// pointer. 007238E0 forms `[element+20h] + node*12` and `[element+24h] + node*12`
// (007238EB, 00723903, 00723908, 0072390A) and hands both to 0085CAD0 with the
// segment, and 00723B70 hands the same two cells to 0085BF90 at 00723B7E/81/8D
// against the squared radius. Both are therefore base pointers of two parallel
// arrays of three floats, min corners and max corners, indexed by a node index
// whose root is 0. The GeomMesh parser zeroes both at 007273D3/007273D7 and its
// teardown frees both through 00BF6989, which agrees: they are heap arrays
// built after the parse, not authored payload.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kGeomElementOffBoundsMin = 0x20;  // float3[nodeCount]
inline constexpr std::size_t kGeomElementOffBoundsMax = 0x24;  // float3[nodeCount]

// 007238E0's leaf threshold: a node covering more than this many triangle
// ordinals recurses (0072391E CMP EBP,14h / 00723922 JG), otherwise the leaf
// loop tests every ordinal in the range.
inline constexpr int kGeomElementLeafTriangleLimit = 0x14;

// ---------------------------------------------------------------------------
// 00723D60's per-element test, as an injected rule. The native callee is
// 00723AA0 (ECX = the element, three stack arguments, RET 0Ch); a host supplies
// whatever it can actually evaluate. The contract the walk depends on is the
// one 007238E0 keeps: when the element is crossed the test returns true and
// writes the hit point nearest to `from`, and it must reject any crossing that
// is not between `from` and `to`, because that rejection is the only thing
// that orders the elements.
// ---------------------------------------------------------------------------

struct ElementSegmentTester {
    virtual ~ElementSegmentTester() = default;
    virtual bool test_element(int element_slot, const HitQueryPoint& from,
                              const HitQueryPoint& to, HitQueryPoint& hit) = 0;
};

struct ClosestElementResult {
    // 00723E7E returns EBX = 0 when the vector is empty and nothing else can
    // produce a null, so -1 here is the native's "no element".
    int element_slot{-1};
    // The geometry-local hit point the accepted test last wrote. Untouched
    // when nothing hit, exactly as the native leaves the caller's buffer.
    HitQueryPoint hit{};
    bool hit_any{false};
};

// 00723D60, body 00723D60-00723E86, __thiscall(geometry in ECX; const float3*
// from, const float3* to, float3* outHit) returning const Element*, RET 0Ch at
// 00723E84. `element_count` is the native's ((geom+10h)-(geom+0Ch)) / 2Ch; a
// count of zero or less takes 00723DBF's JLE straight to the null return.
ClosestElementResult trace_closest_element_00723d60(ElementSegmentTester& tester,
                                                    int element_count,
                                                    const HitQueryPoint& from,
                                                    const HitQueryPoint& to) noexcept;

}  // namespace bsp
