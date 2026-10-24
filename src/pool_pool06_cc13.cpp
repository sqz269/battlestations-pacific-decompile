// cc13 pool06 (segment 33). Descriptive names are hypotheses (cc13-poolw3, 2026-10-24).

#include "bsp/pool_pool06_cc13.hpp"

namespace bsp {

// Name leaves. Evidence: each body is B8 imm32 C3 followed by CC padding to the next
// 10h-aligned function (`ghidra bytes`), with one DATA xref, its vtable slot +4h. ABI: virtual
// __thiscall, ECX unused, no stack args, plain RET; no x87, no globals, no EH states.
// Uncertainty: none in behaviour; the C++ returns equal text, not the .rdata address.

// 006F8EF0, slot +4h of 00CFB63C (DATA xref 00CFB640). B8 54 B6 CF 00 / C3:
// MOV EAX,0x00CFB654 ("moveonpath\0").
const char* command_class_moveonpath_name_006f8ef0() noexcept { return "moveonpath"; }

// 006F9000, slot +4h of 00CFB660 (DATA xref 00CFB664). MOV EAX,0x00CFB674 / RET;
// 00CFB674 = "Leave\0" (capital L, as in the binary).
const char* command_class_leave_name_006f9000() noexcept { return "Leave"; }

// 006F90A0, slot +4h of 00CFB67C (DATA xref 00CFB680). MOV EAX,0x00CFB690 / RET;
// 00CFB690 = "disband\0".
const char* command_class_disband_name_006f90a0() noexcept { return "disband"; }

// 006F9EE0..006FA316, ConvexObject box builder.
// Evidence: listing 006F9EE0..006FA314 and the caller 006FAE81..006FAE89. The minima start at
// [00D7A248] = 7F7FFFFF (FLT_MAX), the maxima at [00D7A244] = FF7FFFFF (-FLT_MAX). The point
// loop is unrolled by four (006F9F2D..) with a one-point remainder loop (006FA1B0..006FA248);
// both visit the points in order with the same per-axis tests, so one loop models them.
// Per axis: min replaced when FCOMIP(min, x) is above (JBE skips: equal or unordered keeps the
// old value); max replaced when FCOMIP(x, max) is above. The replacement is the point value
// re-read as a float (FLD/FSTP float/MOVSS), the same bits.
// Tail (006FA24E..006FA30E): sum = float(min + max) and diff = float(max - min) each stored to
// a float slot, then times the double [00D7A280] = 0.5 and stored as float; centre to +14h..+1Ch,
// half extents to +20h..+28h.
// ABI: __thiscall, ECX = object, [ESP+4] = points, [ESP+8] = count (signed: CMP EBX,4 / JL),
// RET 8. EBX/EBP/EDI/ESI saved. No EH states, no calls.
// x87: the float(+/-) roundings happen at FSTP float; the x0.5 product is exact. With count <= 0
// the centre is (FLT_MAX + -FLT_MAX) * 0.5 = 0 and the half extents float(-2 FLT_MAX) * 0.5 =
// -inf, as in the binary.
// Host overlap: src/hull_terrain_contact.cpp HullTerrainContactSolver::hull_shape inlines the
// centre half of this body (the half extents are not modelled there). Not switched in.
// Uncertainty: the object layout beyond +14h..+28h; SNaN inputs would be quieted by the FLD.
void convex_object_compute_box_006f9ee0(ConvexObjectBox& object, const float* points,
                                        int count) noexcept {
    float lo[3] = {3.402823466e+38f, 3.402823466e+38f, 3.402823466e+38f};
    float hi[3] = {-3.402823466e+38f, -3.402823466e+38f, -3.402823466e+38f};
    for (int i = 0; i < count; ++i) {
        const float* p = points + 3 * i;
        for (int k = 0; k < 3; ++k) {
            if (lo[k] > p[k]) lo[k] = p[k];
        }
        for (int k = 0; k < 3; ++k) {
            if (p[k] > hi[k]) hi[k] = p[k];
        }
    }
    for (int k = 0; k < 3; ++k) {
        const float sum = static_cast<float>(static_cast<double>(lo[k]) + hi[k]);
        object.centre[k] = static_cast<float>(static_cast<double>(sum) * 0.5);
    }
    for (int k = 0; k < 3; ++k) {
        const float diff = static_cast<float>(static_cast<double>(hi[k]) - lo[k]);
        object.half_extent[k] = static_cast<float>(static_cast<double>(diff) * 0.5);
    }
}

}  // namespace bsp
