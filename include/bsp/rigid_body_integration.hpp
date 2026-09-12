#pragma once

#include <cstdint>

#include "bsp/world_ocean.hpp"

// The rigid-body integrator the ship motion tick drives.
//
// docs/RIGID_BODY_INTEGRATION.md carries the addresses, the original ABI and the
// uncertainty. Everything here is a semantic C++ interface for MSVC Win32, not a
// drop-in binary replacement, and every descriptive name is a hypothesis rather than a
// recovered symbol.
//
// The library is the in-house "Dyn" block at [00C30930,00C5DF60). Its own RTTI names one
// class (Dyn::Scene::LCPSolver2Task, vtable 00D7A090, TypeDescriptor 00E1736C), so the
// `Dyn` prefix below is the library's own namespace, not an invention. The rest of the
// naming is descriptive.
//
// Two objects matter. The body `B`, which the unit controller holds at controller+2Ch,
// and the motion state `M = *(B+4h)`. The field offsets in the comments are the native
// ones; the structs below are working layouts, not the native records.

namespace bsp {

// M, the motion state at B+4h. Offsets are native byte offsets inside M.
struct DynMotionState {
    OceanVec3 linear_velocity{};   // M+00h, read by 00C31F40, written by 00C37E50
    OceanVec3 angular_velocity{};  // M+0Ch, read by 00C31F20, written by 00C37E20
    float max_linear_speed{0.0f};  // M+18h
    float max_angular_speed{0.0f}; // M+1Ch
    OceanVec3 linear_bias{};       // M+20h, a pseudo-velocity cleared every substep
    OceanVec3 angular_bias{};      // M+2Ch, likewise
    OceanVec3 force{};             // M+38h, set by 00C32050, accumulated by 00C35360
    OceanVec3 torque{};            // M+44h, set by 00C32030, accumulated by 00C35330
    // M+50h. 00C37F40 writes 1/mass here and 00C41550 multiplies the force by it, so the
    // stored quantity is the INVERSE mass, and 00C31FC0 (which returns its reciprocal)
    // returns the mass. See the Corrections section of docs/RIGID_BODY_INTEGRATION.md.
    float inverse_mass{0.0f};
    OceanVec3 inverse_inertia_body{};  // M+54h..+5Ch, written by 00C37E70 as 1/I per axis
    float inverse_inertia_world[9]{};  // M+60h..+80h, rebuilt every substep by 00C41550
    // M+B4h. When set, 00C41550 rebuilds the world inverse inertia keeping only the
    // row-1 (up) contribution, which confines torque response to that axis.
    bool lock_torque_to_row1{false};
    float linear_damping{0.0f};    // M+B8h, written by 00C37E00
    float angular_damping{0.0f};   // M+BCh, written by 00C37DE0
};

// B+50h flag bits, from the sites that test or set them.
inline constexpr std::uint32_t kDynBodyFlagStatic = 0x1u;       // 00C31FC0 tests bit 0
inline constexpr std::uint32_t kDynBodyFlagAsleep = 0x2u;       // 00C5B1B0 sets bit 1
inline constexpr std::uint32_t kDynBodyFlagNoGravity = 0x4u;    // 00C41550 tests bit 2
inline constexpr std::uint32_t kDynBodyFlagNoIntegrate = 0x10u; // both phases test bit 4
// Every mutator clears bits 1 and 4 (AND 0FFFFFFEDh); that is the wake.
inline constexpr std::uint32_t kDynBodyWakeMask = 0x12u;

// B, the body. Rows 0..2 are the body axes in world space; row 2 is the hull's forward
// axis (0092D730 dots the linear velocity with it) and row 1 the up axis.
struct DynBody {
    float row0[3]{1.0f, 0.0f, 0.0f};  // B+08h
    float row1[3]{0.0f, 1.0f, 0.0f};  // B+14h
    float row2[3]{0.0f, 0.0f, 1.0f};  // B+20h
    float position[3]{};              // B+2Ch
    std::uint32_t flags{0};           // B+50h
    std::int32_t sleep_countdown{0};  // B+54h
    DynMotionState* motion{nullptr};  // B+04h
};

// The world fields the two integration phases read. Offsets are native.
struct DynWorldStepConstants {
    OceanVec3 gravity{};                    // world+04h..+0Ch, added when bit 2 is clear
    float sleep_linear_speed{0.0f};         // world+3Ch
    float sleep_angular_speed{0.0f};        // world+40h
    std::int32_t sleep_countdown_reload{0}; // world+44h
};

// Image constants the two phases use, named so a reader can see where they come from.
inline constexpr float kDynAngularScale = 100.0f;      // float of the double at 00D7A220
inline constexpr float kDynAngularEpsilon = 1.0e-5f;   // the float at 00D7A310
inline constexpr float kDynSleepVelocityScale = 0.9f;  // float of the double at 00D7A390
inline constexpr double kDynRemainderSubstepMin = 5.0e-5; // the double at 00D7A398

// ---------------------------------------------------------------------------
// The body mutators the game side uses. All __thiscall with B in ECX.
// ---------------------------------------------------------------------------

// 00C37F40, RET 4: M+50h = float(1.0f / mass). No zero guard.
void dyn_body_set_mass_00c37f40(DynBody& body, float mass) noexcept;
// 00C31FC0, RET 0, ST0: 0.0f when B+50h bit 0 is set, else float(1.0f / M+50h).
float dyn_body_mass_00c31fc0(const DynBody& body) noexcept;
// 00C37E70, RET 4: per axis, M+54h/+58h/+5Ch = (x == 0) ? 0.0f : float(1.0f / x).
void dyn_body_set_inertia_00c37e70(DynBody& body, const OceanVec3& inertia) noexcept;
// 00C37E00 / 00C37DE0, RET 4: M+B8h / M+BCh, then no wake (they do not clear the flags).
void dyn_body_set_linear_damping_00c37e00(DynBody& body, float damping) noexcept;
void dyn_body_set_angular_damping_00c37de0(DynBody& body, float damping) noexcept;
// 00C37E50 / 00C37E20, RET 4: write M+00h / M+0Ch, then B+50h &= ~12h.
void dyn_body_set_linear_velocity_00c37e50(DynBody& body, const OceanVec3& v) noexcept;
void dyn_body_set_angular_velocity_00c37e20(DynBody& body, const OceanVec3& w) noexcept;
// 00C35360 / 00C35330, RET 4: M+38h += f / M+44h += t, then B+50h &= ~12h.
void dyn_body_add_force_00c35360(DynBody& body, const OceanVec3& f) noexcept;
void dyn_body_add_torque_00c35330(DynBody& body, const OceanVec3& t) noexcept;

// ---------------------------------------------------------------------------
// The two integration phases, per body
// ---------------------------------------------------------------------------

// 00C41550, the first phase of one substep. Called with the world in ESI and dt on the
// stack; the native routine walks the world's body list at world+204h (sentinel
// world+208h, next at body+84h) and runs this block for every body whose B+50h bit 4 is
// clear. Force to velocity, gravity, linear damping, the world inverse inertia, torque to
// angular velocity, angular damping, then the row-1 lock rebuild.
void dyn_body_integrate_velocity_00c41550(DynBody& body, const DynWorldStepConstants& world,
                                          float dt) noexcept;

// 00C5B1B0, the "UpdatePosition" phase of the same substep (profiler label at 00C5B1B0's
// call site in 00C5BB30; the routine takes the world in EBX and dt on the stack). Position
// from velocity plus bias, orientation by a Rodrigues rotation of rows 2 and 1 followed by
// a Gram-Schmidt rebuild, damping again, the two speed clamps, the sleep countdown, and
// the accumulator reset. Gated by B+50h bit 4 exactly as the first phase is.
void dyn_body_integrate_position_00c5b1b0(DynBody& body, const DynWorldStepConstants& world,
                                          float dt) noexcept;

// Both phases of one substep for one body, in the order 00C5BB30 runs them.
void dyn_body_substep(DynBody& body, const DynWorldStepConstants& world, float dt) noexcept;

// ---------------------------------------------------------------------------
// The substep schedule
// ---------------------------------------------------------------------------

// What 00C5C540 does with one 0.05 s game step. `fixed_substep` is world+00h, `budget`
// world+34h (a float used as a countdown), `accumulator` world+48h, which the routine
// leaves at zero. The returned plan says how many full substeps of `fixed_substep` ran and
// whether a final partial substep of `remainder_dt` ran after them.
struct DynWorldSubstepPlan {
    int full_substeps{0};
    float substep_dt{0.0f};
    bool remainder_substep{false};
    float remainder_dt{0.0f};
};

DynWorldSubstepPlan dyn_world_substep_plan_00c5c540(float fixed_substep, float budget,
                                                    float& accumulator, float dt) noexcept;

}  // namespace bsp
