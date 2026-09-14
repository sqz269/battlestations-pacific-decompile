#include "bsp/narrowphase_unit_part_shape.hpp"

// 00723D60, instruction for instruction. docs/NARROWPHASE_UNIT_PART_SHAPE.md.
//
// 00723D67..00723D8D copies the caller's far endpoint into the frame; the
// element index runs 0..count-1 (00723DB9 XOR EDI,EDI, 00723E5E ADD EDI,1,
// 00723E64 CMP against the count) with the byte offset in EBP advancing by 2Ch
// (00723E61). The call at 00723E06 pushes (from, &localTo, outHit) with ECX =
// [geom+0Ch] + EBP. On a hit 00723E35..00723E58 copies the three floats the
// callee wrote into the local far endpoint and records the element address;
// there is no distance comparison anywhere in the body, so the shortening is
// the whole ordering rule.

namespace bsp {

ClosestElementResult trace_closest_element_00723d60(ElementSegmentTester& tester,
                                                    int element_count,
                                                    const HitQueryPoint& from,
                                                    const HitQueryPoint& to) noexcept {
    ClosestElementResult result;
    // 00723DBB CMP [ESP+24h],EBX / 00723DBF JLE: a signed test, so a negative
    // count is the empty case too.
    if (element_count <= 0) return result;

    HitQueryPoint far_end = to;
    for (int slot = 0; slot < element_count; ++slot) {
        HitQueryPoint hit{};
        if (!tester.test_element(slot, from, far_end, hit)) continue;
        far_end = hit;
        result.hit = hit;
        result.element_slot = slot;
        result.hit_any = true;
    }
    return result;
}

}  // namespace bsp
