#pragma once

#include <cstddef>

namespace bsp {
// Semantic interfaces, not native object layouts or binary replacements.
// Names are hypotheses. Native addresses, ABI and limits:
// docs/TORPEDO_FLY_TO_SOLVER.md.

// 009FD570, the shared fly-to-a-point heading solver. Body 009FD570-009FDEDE,
// 2414 bytes, six callers (007B5AF0, 009A3CF0, 009AD480, 009B5760, 009C47D0 the
// dive bomb, 009D0C10 the torpedo break-off). `RET 18h` says SIX stack
// arguments; ECX carries the receiver.
//
//   float __thiscall solve(void* cache,        // ECX, the obstacle cache
//                          const Vec3* point,  // arg1, approach->vtable[0]()
//                          Unit* unit,         // arg2
//                          float standoff,     // arg3
//                          float* side,        // arg4, IN AND OUT
//                          float range,        // arg5
//                          float offset_scale) // arg6
//
// The return is a compass heading already wrapped into [0, 2pi).
//
// Two constants of the frame, because a by-value reading loses them:
//  * `side` is passed BY POINTER and the solver writes it back (009FDC48) when
//    the avoidance term disagrees with the direct bearing by more than 0.1 rad.
//  * arg1 is the ONLY argument the caller pushes after the vtable[0] call. The
//    other five are staged before it and vtable[0] cleans only its own sret
//    pointer (`RET 4`), which is why the call site shows one push for a
//    six-argument call. docs/TORPEDO_FLY_TO_SOLVER.md section 1.

// One entry of the solver's own cached obstacle list (`cache+0Ch`, stride 18h).
// The native rebuilds it every 21st call by walking GGame+19CCh's unit list;
// the caller supplies the walk here because the list is a world query, not
// arithmetic.
struct FlyToObstacle {
    // unit+FCh / +100h / +104h, the world position. Only x and z are used: the
    // solver zeroes the y term explicitly (FLDZ / FMUL ST0 at 009FD829).
    float position[3] = {0.0f, 0.0f, 0.0f};
    // max(unit+434h, unit+444h, unit+448h), 009FD86E-009FD8A4.
    float extent_max = 0.0f;
    // unit+474h + unit+464h + unit+478h, 009FD94C. The weight the solver sorts
    // the obstacles by AND scales each contribution with.
    float extent_sum = 0.0f;
};

// The world-edge leg, GGame+711Ch/7124h (min x, z) and +7128h/7130h (max x, z).
// 00681F40 answers "is this point inside `margin` of an edge" and 009FA510
// answers "by how much", both with margin 500.0 at 009FDD58 / 009FDD6F.
struct FlyToWorldEdge {
    bool near_edge = false;        // 00681F40, TEST AL,AL at 009FDD67
    float penetration = 0.0f;      // 009FA510's largest margin overrun
    float centre[2] = {0.0f, 0.0f};// 0.5*(min+max) in x and z, 009FDDAF-009FDDC5
};

struct FlyToSolverInputs {
    // arg1. 009D3517 / 009D36E4 / 009D3DC8 already name approach->vtable[0] as
    // the target point, so at the torpedo break-off this is the ship.
    float point[3] = {0.0f, 0.0f, 0.0f};

    // arg2's pose: unit+FCh..104h plus 3.0 * unit->vtable[34h](). The 3.0 is
    // the DOUBLE at 00D7A2B0 loaded by `FLD qword` at 009FD5B1 and multiplied
    // into each of the three floats, so the solver steers to where the unit
    // will be three of whatever vtable[34h]'s unit is, not to where it is.
    float unit_position[3] = {0.0f, 0.0f, 0.0f};
    float unit_lead_vector[3] = {0.0f, 0.0f, 0.0f};  // vtable[34h]'s vec3

    float standoff = 0.0f;      // arg3
    float range = 0.0f;         // arg5
    float offset_scale = 0.0f;  // arg6

    const FlyToObstacle* obstacles = nullptr;  // cache+0Ch
    std::size_t obstacle_count = 0;            // cache+10h

    FlyToWorldEdge world_edge{};
};

struct FlyToSolverResult {
    float heading = 0.0f;       // ST0 at 009FDEDC, already in [0, 2pi)
    float side = 0.0f;          // the write-back through arg4
    bool side_written = false;  // 009FDC48 ran
    // Reported so a caller can say which arm the image took rather than guess:
    bool blend_ran = false;     // range - standoff > -200, i.e. NOT the early arm
    bool avoidance_ran = false; // at least one obstacle contributed
    float steer[2] = {0.0f, 0.0f};  // the accumulated x/z the heading comes from
};

// The three named constants of the blend, at the width of the instruction that
// loads them.
namespace fly_to_solver {
inline constexpr float kLeadSeconds = 3.0f;          // 00D7A2B0, FLD qword
inline constexpr float kObstacleRadiusPad = 100.0f;  // 00D7A220, FADD qword
inline constexpr float kFalloffNear = 0.7f;          // 00CE3E18, FLD dword
inline constexpr float kEdgeMargin = 500.0f;         // 00CE397C, FLD dword
inline constexpr float kEdgeGainDivisor = 60.0f;     // 00CE3D68, FDIV qword
inline constexpr float kBlendLow = -200.0f;          // 00D21CE8, FLD qword
inline constexpr float kBlendHigh = 300.0f;          // 00CE3AE8, FLD dword
inline constexpr float kOffsetHigh = 3.1415927f;     // 00D7A264, FLD dword (pi)
inline constexpr float kWeightLow = 1.0f;            // FLD1 at 009FDCF9
inline constexpr float kWeightHigh = 4.0f;           // 00CE3D34, FLD dword
inline constexpr float kSideSwitch = 0.1f;           // 00D7A3A0 / 00CE3928
// The only constant here that is NOT a float promoted to a double in the image:
// 00CE3820 is a true double and float(1e-10) is a different number, so it is
// declared at the width the FLD qword reads.
inline constexpr double kEpsilon = 1e-10;            // 00CE3820, FLD qword
}  // namespace fly_to_solver

// 009FD749-009FDA7D, the obstacle term on its own. `out_steer` is the x/z pair
// the caller adds to the direct term; the return is false when no obstacle
// passed the two range gates, in which case `out_steer` is left at (0, 0) - the
// zero vector the native seeds it from at 00F87574.
bool fly_to_avoidance_009fd749(const float lead_point[3],
                               const FlyToObstacle* obstacles,
                               std::size_t count, float out_steer[2]) noexcept;

// 009FD570 whole. `side` is read and, on the 009FDC48 arm, written back.
FlyToSolverResult fly_to_point_heading_009fd570(const FlyToSolverInputs& in,
                                                float side) noexcept;

// 009FDE8F and 009FDB6C/009FDBBD share one conversion: a compass heading out of
// an x/z pair, `wrap0to2pi(pi/2 - atan2(z, x))`. Exposed because the break-off
// and the aim tick both need it and reimplementing it invites the sign error.
float fly_to_bearing_of_009fde8f(float x, float z) noexcept;

}  // namespace bsp
