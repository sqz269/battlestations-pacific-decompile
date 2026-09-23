#pragma once
// Packet cc9_neighbour_clips, docs/SHIP_NEIGHBOUR_AVOIDANCE.md section 4.
// The neighbour node's avoid-box services and the order tail's pass-side walk.
// Names are hypotheses. Every routine here is a SEMANTIC reconstruction with a
// new C++ interface, not an ABI replacement: the native bodies keep x87
// extended intermediates in a few sums (noted per routine) that plain float
// arithmetic does not reproduce bit for bit.
//
// The avoid box is the node's +44h..+60h: centre (+44h, +48h), beam axis
// (+4Ch, +50h) with half extent +5Ch, forward axis (+54h, +58h) with half
// extent +60h (bsp/ship_ai_sector_scan.hpp, ShipAiObstacleNode).

#include "bsp/ship_ai_sector_scan.hpp"
#include "bsp/system_camera_axes.hpp"

#include <array>
#include <cstdint>

namespace bsp {

// 009D8A30, `float* __thiscall(node)(float* out, const float* point)`, RET 8,
// body 009D8A30-009D8B88. The point of the avoid box closest to `point`: both
// local coordinates clamped into [-half, +half] (a NaN coordinate clamps to
// -half, 009D8A7x's `-half <= t` test failing). With node+68h set, the centre.
std::array<float, 2> ship_ai_obstacle_closest_point_009d8a30(
    const ShipAiObstacleNode& node, const std::array<float, 2>& point) noexcept;

// 009D8860, `float* __thiscall(node)(float* out, const float* dir)`, RET 8,
// body 009D8860-009D8A2C. The corner extreme along `dir`: beam sign minus when
// dir.beam < 0 (else plus), forward sign plus when 0 <= dir.forward (else
// minus, so a NaN takes the minus side). With node+68h set, the centre.
std::array<float, 2> ship_ai_obstacle_support_point_009d8860(
    const ShipAiObstacleNode& node, const std::array<float, 2>& dir) noexcept;

// 009DD010, `bool __thiscall(node)(const float* centre, float radius,
// float from_bearing, float* io_to_bearing)`, RET 10h, body 009DD010-009DD530.
// False with node+68h or +69h set, when the box's closest point is farther than
// `radius` from `centre` (009DD298), or when corners 0, 2, 3 and 0 again (the
// native tests corner 0 twice and never corner 1, 009DD2F8..009DD376) all lie
// strictly inside the circle. Otherwise each of the four edges (corners
// ++, +-, --, -+ in beam/forward sign order, then back to ++) is cut by the
// circle (004F3BA0); each crossing's world bearing pi/2 - atan2(dz, dx),
// wrapped into [0, 2pi), narrows *io_to_bearing toward `from_bearing` when it
// lies strictly inside the swept delta, which is captured once per edge
// (009DD3DE). Returns whether any crossing narrowed it.
bool ship_ai_obstacle_clip_arc_009dd010(const ShipAiObstacleNode& node,
                                        const std::array<float, 2>& centre, float radius,
                                        float from_bearing, float& io_to_bearing,
                                        const CameraAxesCrtAccess& crt);

// 009D8210, `bool __thiscall(node)(const float* a, const float* b,
// const float* origin, const float* dir, float* io_range)`, RET 14h, body
// 009D8210-009D84D2. One edge a..b against the ray origin + t*dir, t in
// (0, *io_range): false with node+68h or +69h set, when both ends are behind
// (along a <= 0 and along b < 0) or both at or past the range, or when both
// ends lie on one side of the ray. Otherwise the crossing's distance along the
// ray replaces *io_range when 0 < t < *io_range.
bool ship_ai_obstacle_segment_ray_009d8210(const ShipAiObstacleNode& node,
                                           const std::array<float, 2>& a,
                                           const std::array<float, 2>& b,
                                           const std::array<float, 2>& origin,
                                           const std::array<float, 2>& dir,
                                           float& io_range) noexcept;

// 009DD540, `bool __thiscall(node)(const float* origin, const float* dir,
// float* io_range)`, RET 0Ch, body 009DD540-009DD9BA. False with no owner, an
// owner whose +5Eh is set, or node+68h set (009DD54B..009DD565); then four
// support-point rejections through 009D8860 (the box wholly behind the origin,
// wholly beyond the range, wholly on either side of the ray line), then the
// four edges through 009D8210, OR-ing their answers. `owner_gone_5e` is the
// observed owner's +5Eh byte, which the caller reads.
bool ship_ai_obstacle_clip_ray_009dd540(const ShipAiObstacleNode& node, bool owner_gone_5e,
                                        const std::array<float, 2>& origin,
                                        const std::array<float, 2>& dir,
                                        float& io_range) noexcept;

// The node fields the order tail's pass-side walk owns and ShipAiObstacleNode
// does not carry. 009E537B seeds +7Ch with 0.0f; +75h is 0 from 009E536A.
struct ShipAiNeighbourPassState {
    float timer_7c{0.0f};  // node+7Ch
    bool flag_75{false};   // node+75h
};

// 009D8010, `bool __thiscall(node)(void* unit, float dt)`, RET 8, body
// 009D8010-009D80B4. False with no owner or a gone owner (the timer is not
// touched). Otherwise +7Ch -= dt; node+68h set clears +88h; else, when `unit`
// is non-null, leads its group (00778890) and shares the observed owner's
// group word unit+284h, +7Ch = max(+7Ch, 0.5 [00CE3800]) (00415550) and +88h
// is cleared. True when +68h is clear and 0 > +7Ch.
bool ship_ai_neighbour_pass_gate_009d8010(ShipAiObstacleNode& node,
                                          ShipAiNeighbourPassState& pass,
                                          bool owner_gone_5e, bool unit_present,
                                          bool unit_leads_owner_group, float dt) noexcept;

// 009F0100's inputs, as the controller block holds them.
struct ShipAiOrderTailPassInputs {
    int mode_35c{0};              // blk+35Ch, 2 = astern
    float body_speed{0.0f};       // 0092D730 on unit+1018h (009F013A)
    float heading{0.0f};          // unit vtable+50h (009F022E)
    float heading_target_324{0.0f};
    std::array<float, 2> pose_184{};      // blk+184h
    std::array<float, 2> forward_1ac{};   // blk+1ACh
    std::array<float, 2> port_18c{};      // blk+18Ch, the port shoulder
    std::array<float, 2> starboard_194{}; // blk+194h
    float remaining_32c{0.0f};            // blk+32Ch
    float turn_radius_3cc{0.0f};          // blk+3CCh
    float hull_scale_3e4{0.0f};           // blk+3E4h
    float corner_reach_add_1c8{0.0f};     // settings+1C8h (via +180h +48h)
    float line_check_1cc{0.0f};           // settings+1CCh (via +180h +4Ch)
    float own_width_9cc{0.0f};            // [blk+3FCh]+9CCh
    bool unit_present{true};              // blk+3FCh non-null
};

struct ShipAiOrderTailPassHost {
    virtual ~ShipAiOrderTailPassHost() = default;
    virtual int count_604() = 0;
    virtual ShipAiObstacleNode& node_608(int index) = 0;
    virtual ShipAiNeighbourPassState& pass_state(int index) = 0;
    virtual bool owner_gone_5e(int index) = 0;
    // 00778890(unit) and unit+284h == [node+14h]+284h.
    virtual bool unit_leads_owner_group(int index) = 0;
    // 009D8C60 with side 1 or 2 and a non-null unit and owner: 009D66B0 builds
    // a message for (owner, side) and 0077C2A0 routes it to the unit
    // (009D8CB0..009D8CC4). No field of the node is written on this path.
    virtual void post_pass_side_009d8c60(int index, int side) = 0;
};

struct ShipAiOrderTailPassResult {
    bool clear_arm{false};   // 009F015F: mode 0 and |speed| < 1.0
    bool any_due{false};     // 009F01FF
    bool turning{false};     // 009F0387: |error| > 5 degrees
    int processed{0};        // nodes that reached 009F09D1
    int posts{0};            // 009D8C60 calls with a side
    int clears{0};           // +88h/+75h clears in the body
};

// 009F0100, `void __thiscall(blk)(float dt)`, RET 4. Its body runs from
// 009F0100 to the exclusive end 009F0ACE: Ghidra's function body stops at
// 009F0A17 and misses the three tail blocks 009F0A1A..009F0ACE the loop jumps
// to. Constants: 1.0 (00D7A24C), 5 degrees 0.0872664675f (00CEDF5C), pi
// (00D7A264), -0.0f (00D7A208), pi/2 and 2pi as doubles (00CE3830, 00CE3828),
// 200.0 (00CE4D70), 1e-10 (00CE3820), 0.9f (00CE3860).
ShipAiOrderTailPassResult ship_ai_order_tail_neighbour_pass_009f0100(
    const ShipAiOrderTailPassInputs& in, float dt, ShipAiOrderTailPassHost& host,
    const CameraAxesCrtAccess& crt);

} // namespace bsp
