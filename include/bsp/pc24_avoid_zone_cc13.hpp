#pragma once

// PC24 wave 2, packet pc24w2-avoid. The avoid-zone geometry runs on the main thread, whose
// x87 runs at 24-bit precision control: every FADD/FMUL rounds to a 24-bit significand
// before the next op. ON evaluates the 2-D length per op through the cc13x87 helpers
// (include/bsp/cc13_x87_model.hpp); OFF keeps the earlier double sum rounded once.
namespace bsp {

// kPc24AvoidZoneCc13Bound (OFF), src/avoid_zone_geometry.cpp length_of:
//   00419260 reciprocal length (normalized()): x*x and y*y each FMUL and FSTP float
//            (00419276, 00419282), FADD 0041928A, FSTP float, then _CIsqrt 00BF7030.
//            Live only as the per-tick detour normalize, through
//            zone_detour_corners_00422500.
//   0041A2FC..0041A325 edge length in the corner rebuild: FMUL 0041A304 / FMULP 0041A308,
//            FADDP 0041A30A, FSTP float, then _CIsqrt 0041A320.
//            Not live. The switched copy is avoid_zone_rebuild_corner_data in
//            avoid_zone_geometry.cpp, reached only from that file's projection,
//            which is itself not live.
//   The image's mission-load zone build is 0041CCD0, which calls 0041A200 at 0041D063.
//   The host runs it as ship_ai_lateral_records_derive_0041a200
//   (ship_ai_lateral_record.cpp, called from avoid_zone_owner.cpp), an inline-asm per-op
//   copy that needs no switch.
// Movers: avoid-zone detour normals only.
inline constexpr bool kPc24AvoidZoneCc13Bound = false;

// Census (cc9-nondet1, log only): length_of calls in the corner rebuild and elsewhere,
// and how often the per-op length differs from the double one.
struct AvoidZonePc24Census {
    unsigned long long rebuild_calls{0}, rebuild_differs{0};
    unsigned long long detour_calls{0}, detour_differs{0};
};
const AvoidZonePc24Census& avoid_zone_pc24_census() noexcept;

}  // namespace bsp
