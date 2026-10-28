#pragma once

// 004F49F0 BSP_Segment2f_SegmentDistance as the image computes it (packet cc13-p3b).
// Header-only switch.

namespace bsp {

// kSegmentDistance2dCc13Bound (OFF): src/game_hosts_ship_ai.cpp's
// segment_distance_2d_004f49f0 is a LABELLED SUBSTITUTION (a textbook cross-product
// crossing test and a clamped point-to-segment distance, in double). ON runs the image's
// algorithm, every x87 op rounded at PC24 (main-thread caller) and every listing store a
// float:
//   004F3730 BSP_Math_SegmentCrossing2D: da = a1 - a0, db = b1 - b0 as floats; 004F3630
//     BSP_Math_SegmentParameters2D: den = db.x*da.y - db.y*da.x, parallel when 004F3560
//     BSP_Math_RelativeEqualFloat(den, 0) (|den| < max(1, |den|) * 1e-4f [00E0830C]);
//     else t along a, u along b; crossing when 0 <= t <= 1 and 0 <= u <= 1 (unordered
//     fails). A crossing returns 0.
//   Otherwise the least of the four 00419AB0 BSP_Segment2f_PointDistance values (b0 and b1
//     to segment a, a0 and a1 to segment b): 00419260 BSP_Vector2f_ReciprocalLength of the
//     segment, the unit normal, 004F3630 for (t, u); t < 0 -> |p - start|, t > 1 ->
//     00414C60 |p - end| (both 0 at or below 1e-10 squared), else |u|.
// The ON wrapper passes the callers' second segment as ECX/EDX and the first as the stack
// pair, the order 008206F0's call at 0082178B uses.
// Same-address copies, not switched: segment_pair_distance_004f49f0
// (avoid_zone_distance_followup.cpp:91), the naked native_segment_crossing_004f3730
// (avoid_zone_clearance.cpp:94) and avoid_zone_segment_distance_00419ab0
// (avoid_zone_segment_math.cpp:363).
// Pairing: flip together with kTransportLaunchCraftCc13Bound. The OFF transport caller
// (game_hosts_ship_ai.cpp:14704-14705, seg_b) passes the craft keel ends reversed.
// Movers when ON: troop transport landing-craft launch (008206F0): which launch slots count
// as blocked by a nearby ship's keel, so the craft's placement and launch timing.
inline constexpr bool kSegmentDistance2dCc13Bound = false;

}  // namespace bsp
