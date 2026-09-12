#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/dyn_contact_solver.hpp"

// The per-constraint impulse math the Dyn solver's ten iterations run.
//
// docs/DYN_LCP_IMPULSE_MATH.md carries the addresses, the original ABI and the
// uncertainty. Everything here is a semantic C++ interface for MSVC Win32, not a
// drop-in binary replacement, and every descriptive name is a hypothesis rather than a
// recovered symbol.
//
// Which routines actually run. docs/DYN_CONTACT_SOLVER.md attributed the per-group work
// to 00C5C7A0 (SolverPreStep) and 00C5C710 (SolveConstraints). Those two have a single
// caller each and it is Dyn_Scene_LCPSolver2Task_vslot0 (00403850), the class the world
// selects only when world+10h == 1. The shipped world stores 0 there, so the task that
// runs is Dyn_Scene_LCPSolverTask_vslot0 (00403720), whose body at 00403784..004037DB is
//
//     for (group = task+0Ch; group <= task+10h; ++group) {
//         00C4F040(context, group, dt);                     // 0040379A, RET 4
//         for (i = [[context+0h]+38h]; i != 0; --i) {       // 004037A3, world+38h = 10
//             00C42530(context);                            // 004037B4
//             00C42230(context);                            // 004037B9
//         }
//         00C37B50(context);                                // 004037C7, EDX = context
//         00C35020(context);                                // 004037CD, pushed
//     }
//
// so the ten iterations run 00C42530 then 00C42230, not 00C431D0 then 00C42ED0. The
// 00C431D0/00C42ED0 pair is the same formulation over a manifold-major row layout and is
// only reachable through the unselected LCPSolver2Task; this header does not project it.
//
// The context is a 0EAACh-byte stack object the task owns (00403733 alloca with
// EAX = 0EAACh; the object starts 0Ch below the task's saved registers, see the doc).
// Nothing but these routines touches it, so the layout below is a working projection of
// that object, not a native record.

namespace bsp {

// ---------------------------------------------------------------------------
// The solver context's fixed fields
//
// Offsets are the native byte offsets inside the stack context. 00403749..00403770
// zeroes +0EA6Ch, +0EA70h, +0EA74h, +0EA78h and +0EAB4h before the first group.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kDynSolverContextSize = 0xeaac;          // 00403733
inline constexpr std::size_t kDynSolverContextWorldOffset = 0x00;     // 0040379F, +38h read
inline constexpr std::size_t kDynSolverContextBodyCountOffset = 0x04; // 00C4DE4C writes 1
inline constexpr std::size_t kDynSolverContextBodyArrayOffset = 0x08; // body* by solver index
inline constexpr std::size_t kDynSolverContextGroupOffset = 0xea68;   // 00C4F044
inline constexpr std::size_t kDynSolverContextVelocityArrayOffset = 0xea6c;  // 00C4F0A4
inline constexpr std::size_t kDynSolverContextVelocityCapacityOffset = 0xea70;
inline constexpr std::size_t kDynSolverContextRowBlockOffset = 0xea74;  // 00C31C60
inline constexpr std::size_t kDynSolverContextRowCapacityOffset = 0xea78;
inline constexpr std::size_t kDynSolverContextVelocityCountOffset = 0xeaa0;  // 00C4F068
inline constexpr std::size_t kDynSolverContextNormalRowCountOffset = 0xeaa4; // 00C4F011
inline constexpr std::size_t kDynSolverContextFrictionRowBaseOffset = 0xeaa8;  // 00C4F017

// ---------------------------------------------------------------------------
// The row block, 00C31C30
//
// One allocation of rowCapacity * 7Ch bytes carved into nine parallel arrays. The row
// struct below is an array-of-structs projection of that structure-of-arrays; the base
// pointers and their strides are the constants, so a reader can map either way.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kDynSolverRowBytes = 0x7c;              // 00C31C41 IMUL 7Ch
inline constexpr std::size_t kDynSolverJacobianArrayOffset = 0xea7c;      // 30h per row
inline constexpr std::size_t kDynSolverMassJacobianArrayOffset = 0xea80;  // 30h per row
inline constexpr std::size_t kDynSolverBodyPairArrayOffset = 0xea84;      // two int16
inline constexpr std::size_t kDynSolverRhsVelocityArrayOffset = 0xea88;   // float
inline constexpr std::size_t kDynSolverImpulseArrayOffset = 0xea8c;       // float
inline constexpr std::size_t kDynSolverRhsBiasArrayOffset = 0xea90;       // float
inline constexpr std::size_t kDynSolverImpulseBiasArrayOffset = 0xea94;   // float
inline constexpr std::size_t kDynSolverEffectiveMassArrayOffset = 0xea98; // float
inline constexpr std::size_t kDynSolverFrictionArrayOffset = 0xea9c;      // float

// 00C4F04A: the row block is sized to eight rows per manifold in the group, which is the
// four-point manifold cap (00C3F760) times one normal and one friction row per point.
inline constexpr std::int32_t kDynSolverRowsPerManifold = 8;      // 00C4F04D..00C4F051
inline constexpr std::int32_t kDynSolverFrictionBaseRowsPerManifold = 4;  // 00C4DE5D

// ---------------------------------------------------------------------------
// The per-body velocity accumulator, 30h bytes, 00C4F0BA..00C4F131 zeroes all twelve
// floats every pre-step.
//
// The two pairs are the split-impulse arrangement: the first pair accumulates the change
// to the real velocity, the second the change to the pseudo-velocity M+20h/M+2Ch that
// bsp/rigid_body_integration.hpp already documents as "added to the position but not to
// the velocity, cleared every substep". 00C37B50 adds the first pair to M+00h/M+0Ch and
// the second to M+20h/M+2Ch, which is what identifies them.
//
// Solver index 0 is the shared static slot: 00C4DEDB gives every body with
// kDynBodyFlagStatic index 0, so all static geometry shares one accumulator and 00C37B50
// skips it (its loop starts at 1). A static body therefore never receives an impulse.
// ---------------------------------------------------------------------------
struct DynSolverBodyVelocity {
    float linear[3]{};        // +00h
    float angular[3]{};       // +0Ch
    float linear_bias[3]{};   // +18h
    float angular_bias[3]{};  // +24h
};

// ---------------------------------------------------------------------------
// One constraint row
//
// `jacobian` is (J_linA, J_angA, J_linB, J_angB); `mass_jacobian` is the same twelve
// numbers premultiplied by the two bodies' inverse mass and world inverse inertia, so it
// is the velocity change a unit impulse on this row produces. Both A blocks carry the
// sign, so every apply step is a plain `v += mass_jacobian * dLambda`.
//
// `friction_coefficient` is set on the normal rows only; the friction row at
// `friction_row_base + k` reads row `k`'s coefficient and row `k`'s `impulse` for its
// clamp (00C42373, 00C42379).
// ---------------------------------------------------------------------------
struct DynConstraintRow {
    float jacobian[12]{};       // +EA7C block
    float mass_jacobian[12]{};  // +EA80 block
    std::int16_t body_a{0};     // +EA84 low half, a solver index
    std::int16_t body_b{0};     // +EA84 high half
    float rhs_velocity{0.0f};   // +EA88, -(restitution target) - J.u
    float impulse{0.0f};        // +EA8C, the accumulated impulse, warm started
    float rhs_bias{0.0f};       // +EA90, erp * clamped depth / dt
    float impulse_bias{0.0f};   // +EA94, the accumulated bias impulse, warm started
    float effective_mass{0.0f}; // +EA98, 1 / (J . M^-1 J^T)
    float friction_coefficient{0.0f};  // +EA9C, normal rows only
};

// The rows and the accumulators for one contact group.
struct DynConstraintBatch {
    DynConstraintRow* rows{nullptr};
    DynSolverBodyVelocity* velocities{nullptr};
    std::int32_t velocity_count{0};     // context+0EAA0h
    std::int32_t normal_row_count{0};   // context+0EAA4h
    std::int32_t friction_row_base{0};  // context+0EAA8h
};

// ---------------------------------------------------------------------------
// The manifold and contact point fields the solver reads and writes
//
// bsp/dyn_contact_solver.hpp declares DynContactPoint with four fields and calls +24h
// `normal_scale`, "penetration depth and accumulated normal impulse both fit; this
// packet does not choose". The producer settles it: 00C3F760 zeroes +24h and +28h when it
// appends a point (00C3F9BD, 00C3F9D2), leaves both untouched when it refreshes a matched
// point, and writes the penetration depth to +2Ch. 00C4DE40 warm starts the two impulse
// accumulators from them and 00C35020 writes the solved impulses back. The struct below
// is the whole 30h-byte record; it supersedes, and does not replace, the four-field one.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kDynContactPointNormalImpulseOffset = 0x24;  // 00C4E0B2
inline constexpr std::size_t kDynContactPointBiasImpulseOffset = 0x28;    // 00C4E0C6
inline constexpr std::size_t kDynContactPointDepthOffset = 0x2c;          // 00C3F93C
inline constexpr std::size_t kDynManifoldFrictionOffset = 0x00;  // 00C4EFA8, stored 00C4EFB9
inline constexpr std::size_t kDynManifoldRestitutionOffset = 0x04;  // 00C4E679

struct DynSolverContactPoint {
    float normal[3]{};          // point+00h, unit, from B towards A
    float local_point_a[3]{};   // point+0Ch, in body A's frame
    float local_point_b[3]{};   // point+18h, in body B's frame
    float normal_impulse{0.0f}; // point+24h, accumulated, survives the frame
    float bias_impulse{0.0f};   // point+28h, accumulated, survives the frame
    float depth{0.0f};          // point+2Ch, positive when penetrating
};

// ---------------------------------------------------------------------------
// The world fields the pre-step reads. Register provenance for each: 00C4DEB6 loads
// EBX = [context+0h] then 00C4DEBE reads EBX+2Ch; 00C4E094 loads EAX = [context+0h] then
// 00C4E0AF reads EAX+20h; 00C4E0DA loads EDX = [context+0h] then 00C4E0DC reads EDX+28h;
// 00C4E5D1 loads EAX = [context+0h] then 00C4E63F reads EAX+18h.
// ---------------------------------------------------------------------------
struct DynSolverWorldSettings {
    // world+18h, descriptor+2Ch, 0.1f in the shipped world. Multiplies the clamped
    // penetration depth and is divided by dt, so it is the fraction of the overlap the
    // bias rows remove per substep.
    float position_correction{0.1f};
    // world+20h, descriptor+34h, 1.0f in the shipped world. Scales the stored normal
    // impulse on the way in, so 1.0f is full warm starting.
    float warm_start_scale{1.0f};
    // world+28h, descriptor+3Ch, 0.5f in the shipped world. The penetration depth is
    // clamped to this before it becomes a bias velocity.
    float max_correction_depth{0.5f};
    // world+38h, descriptor+24h, 10 in the shipped world.
    std::int32_t iterations{10};
};

// The two literals the row builder folds in.
// 00C4E67C FADDs the double at 00D7A270, whose value is exactly (double)0.05f, to the
// restitution term, and 00C4E68C keeps the result only while it is negative. An approach
// speed slower than threshold/restitution therefore produces no bounce.
inline constexpr float kDynRestitutionVelocityThreshold = 0.05f;
// 00C4EACA, the float at 00D7A288. Below this squared tangential speed the friction row
// keeps the arbitrary perpendicular instead of the sliding direction.
inline constexpr float kDynFrictionDirectionEpsilonSq = 1.0e-6f;

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// One three-term group of a row dot product, in the listing's order
// (00C4263A..00C4264B): the middle product first, then the first, then the last. Each
// group is rounded to float32 before it is folded into the running residual
// (00C4264F FSTP float / 00C42653 FLD float).
float dyn_row_group_dot(float j0, float j1, float j2, float v0, float v1, float v2) noexcept;

// rhs - J.u for one row, in the listing's order: body A linear, body A angular, body B
// linear, body B angular, each group subtracted and rounded in turn.
float dyn_row_residual(float rhs, const float jacobian[12], const float linear_a[3],
                       const float angular_a[3], const float linear_b[3],
                       const float angular_b[3]) noexcept;

// v += mass_jacobian * delta for both bodies, 00C425EB..00C428DA (normal) and
// 00C423E6..00C4250C (friction).
void dyn_row_apply_impulse(const float mass_jacobian[12], float delta,
                           float linear_a[3], float angular_a[3], float linear_b[3],
                           float angular_b[3]) noexcept;

// 1 / (J . M^-1 J^T), the twelve-term dot the normal row stores at 00C4E7E8 through the
// FLD1/FDIVRP at 00C4E7F8, and the friction row at 00C4EF85/00C4EF95. The native code divides
// without guarding the denominator; a row whose two bodies are both static divides by
// zero. The group fill (00C4B610) never seeds such a group, which is why it is safe.
float dyn_effective_mass(const float jacobian[12], const float mass_jacobian[12]) noexcept;

// The restitution target, 00C4E655..00C4E6AC. `normal_velocity` is the relative velocity
// at the contact dotted with the normal, negative while the bodies approach.
float dyn_restitution_target(float normal_velocity, float restitution) noexcept;

// The bias row's right-hand side, 00C4E0DC..00C4E651 with the clamp at 00C4E0F3: the depth is clamped to
// [0, max_correction_depth] first, then scaled by position_correction and divided by dt.
float dyn_penetration_bias(float depth, float max_correction_depth,
                           float position_correction, float dt) noexcept;

// The contact Jacobian for one point, stored at 00C4E1F7..00C4E31E. `ra` and `rb` are the contact
// point relative to each body's origin, `normal` points from B towards A. Body A takes
// the negative half, which is why both apply steps add.
void dyn_contact_jacobian(const float normal[3], const float ra[3], const float rb[3],
                          float jacobian[12]) noexcept;

// J premultiplied by the inverse mass and the world inverse inertia of each body,
// stored at 00C4E356..00C4E448. The inertia is a 3x3 stored by columns at M+60h, M+6Ch, M+78h.
void dyn_mass_jacobian(const float jacobian[12], float inverse_mass_a,
                       const float inverse_inertia_a[9], float inverse_mass_b,
                       const float inverse_inertia_b[9], float mass_jacobian[12]) noexcept;

// The friction row's direction, 00C4E828..00C4EB0C. When the tangential part of the
// relative velocity is longer than the epsilon it is normalised and used; otherwise the
// arbitrary perpendicular the routine builds from the normal's dominant axis is kept.
// Returns true when the sliding direction won.
bool dyn_friction_direction(const float normal[3], const float relative_velocity[3],
                            float tangent[3]) noexcept;

// The two shape properties the narrow phase combines into the manifold, 00C44154..00C4418B
// and 00C4419D..00C441AF. A negative friction on either shape is an override: its magnitude wins.
float dyn_combine_friction(float friction_a, float friction_b) noexcept;
float dyn_combine_restitution(float restitution_a, float restitution_b) noexcept;

// ---------------------------------------------------------------------------
// The three routines the iteration loop and the pre-step run over the rows
// ---------------------------------------------------------------------------

// 00C42BA0, the last thing 00C4F040 does. Pushes the warm-started impulses of every
// normal row into the accumulators, so the first iteration starts from last frame's
// answer. Friction rows are not warm started (00C4E807 zeroes their impulse).
void dyn_apply_warm_start_00c42ba0(DynConstraintBatch& batch) noexcept;

// 00C42530, rows [0, normal_row_count). Two Gauss-Seidel sweeps per row over the same
// Jacobian and the same effective mass: the velocity row against rhs_velocity into the
// first accumulator pair, then the bias row against rhs_bias into the second. Both clamp
// the accumulated impulse at zero from below and nothing from above.
void dyn_solve_normal_rows_00c42530(DynConstraintBatch& batch) noexcept;

// 00C42230, rows [friction_row_base, friction_row_base + normal_row_count). One sweep per
// row into the first accumulator pair only, with the accumulated impulse clamped to
// +/- (friction_coefficient[k] * impulse[k]) of the matching normal row k. There is one
// friction row per contact point, not two: the direction is rebuilt every pre-step from
// the sliding direction, so a single row tracks it.
void dyn_solve_friction_rows_00c42230(DynConstraintBatch& batch) noexcept;

// The 004037A3..004037C1 loop: `iterations` passes of the pair above, in that order.
void dyn_solve_group_00403720(DynConstraintBatch& batch, std::int32_t iterations) noexcept;

// ---------------------------------------------------------------------------
// The pre-step's row build and the two write-backs
//
// 00C4DE40 makes no call but BSP_Math_AbsFloat and the square root, so it is projected as
// a pure function over the inputs it reads rather than over a host. One contact point
// becomes one normal row at `next_normal_row` and one friction row at
// `friction_row_base + next_normal_row`.
// ---------------------------------------------------------------------------

// What the row builder needs from one body. `transform` is the 3x4 at B+08h/+14h/+20h
// with the origin at B+2Ch, the same layout bsp/rigid_body_integration.hpp declares.
struct DynSolverBodyInput {
    float row0[3]{1.0f, 0.0f, 0.0f};
    float row1[3]{0.0f, 1.0f, 0.0f};
    float row2[3]{0.0f, 0.0f, 1.0f};
    float position[3]{};
    float linear_velocity[3]{};   // M+00h
    float angular_velocity[3]{};  // M+0Ch
    float inverse_mass{0.0f};     // M+50h
    float inverse_inertia[9]{};   // M+60h..+80h, by columns
    std::int16_t solver_index{0}; // B+5Ch, 0 for every static body
};

// One contact point of one manifold, together with the manifold's two combined
// coefficients, is everything a row pair needs.
struct DynConstraintBuildInput {
    DynSolverBodyInput body_a{};
    DynSolverBodyInput body_b{};
    DynSolverContactPoint point{};
    float friction{0.0f};     // manifold+00h
    float restitution{0.0f};  // manifold+04h
};

// 00C4DEC7..00C4EFC7 for one point. Fills `normal_row` and `friction_row` completely.
// The world's settings and the substep dt are the only other inputs.
void dyn_build_contact_rows_00c4de40(const DynConstraintBuildInput& input,
                                     const DynSolverWorldSettings& settings, float dt,
                                     DynConstraintRow& normal_row,
                                     DynConstraintRow& friction_row) noexcept;

// 00C37B50. Adds each dynamic body's two accumulator pairs to its motion state, and
// resets B+58h to -1 so the next pre-step re-indexes it. Solver index 0 is skipped.
struct DynSolverVelocityWriteBack {
    float linear_velocity[3]{};   // M+00h, added
    float angular_velocity[3]{};  // M+0Ch, added
    float linear_bias[3]{};       // M+20h, added
    float angular_bias[3]{};      // M+2Ch, added
};
void dyn_write_back_velocities_00c37b50(const DynSolverBodyVelocity* velocities,
                                        std::int32_t velocity_count,
                                        DynSolverVelocityWriteBack* bodies) noexcept;

// 00C35020. Copies each normal row's two accumulated impulses back into its contact
// point, which is what makes the next frame's warm start possible. Rows are consumed in
// the same manifold-then-point order the builder produced them in.
void dyn_store_impulses_00c35020(const DynConstraintBatch& batch,
                                 DynSolverContactPoint* const* points_per_row,
                                 std::int32_t row_count) noexcept;

}  // namespace bsp
