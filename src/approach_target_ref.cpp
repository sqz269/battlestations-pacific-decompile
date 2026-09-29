#include "bsp/approach_target_ref.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "bsp/geom_mesh_resource.hpp"      // GeomMeshResourcePayload
#include "bsp/ship_ai_throttle_ring.hpp"  // heading_to_direction_006bc0c0
#include "bsp/unit_rudder.hpp"            // wrapped_angle_subtract_00438b10

// Packet cc8_hull_aim_point. See include/bsp/approach_target_ref.hpp for the
// addresses, the ABI and what is proved versus substituted.

namespace bsp {
namespace {

// 009FB2E7: the first timer value is drawn over [0, 2.0).
constexpr float kFirstTimerLo = 0.0f;

// 008120E0's constants. The two doubles are float values widened.
constexpr double kShipPredictMinTurn = 0.10000000149011612;   // 00D7A3A0
constexpr double kShipPredictMinSpeed = 0.8333333730697632;   // 00D09450
constexpr float kShipPredictTurnLo = -1.0f;                   // 00D7A260
constexpr float kShipPredictTurnHi = 1.0f;                    // 00D7A24C
// The projtime cap: double 30.0 [00CE7630] compared, float 30.0 [00CE38C8]
// stored; the same pair the torpedo estimate uses at 009D3D3C/009D3D4A.
constexpr double kProjtimeCap = 30.0;

}  // namespace

ApproachTargetRefState approach_target_ref_construct_009fb200(
    float first_timer_draw) noexcept {
    ApproachTargetRefState state;
    // 009FB276/7B/80.
    state.spread_48 = {kApproachTargetRefSpreadX, kApproachTargetRefSpreadY,
                       kApproachTargetRefSpreadZ};
    // 009FB285-009FB2AA then 009FB2AF-009FB2D5: the offset, the bias and the
    // centre are all seeded from 00F87574/78/7C, copied twice. That address is
    // .data past raw size, so on disk it is zero; a runtime writer would show
    // up as a non-zero seed here.
    state.body_offset_28 = {0.0f, 0.0f, 0.0f};
    state.bias_34 = {0.0f, 0.0f, 0.0f};
    state.centre_54 = {0.0f, 0.0f, 0.0f};
    // 009FB25F.
    // sub+44h is 0.0; the tail it gates is not modelled.
    state.repick_timer_60 = first_timer_draw;  // 009FB2E7, Uniform(0, 2.0)
    state.section_chance_64 = kApproachTargetRefSectionChance;  // 009FB2F9
    state.weight_68 = kApproachTargetRefSectionWeight;          // 009FB2FE
    state.weight_6c = kApproachTargetRefSectionWeight;          // 009FB306
    state.weight_70 = kApproachTargetRefSectionWeight;          // 009FB30B
    state.dirty_41 = true;   // 009FB272
    state.tracking = true;
    state.frozen = false;
    (void)kFirstTimerLo;
    return state;
}

std::array<float, 3> approach_target_ref_pick_009fa260(
    ApproachTargetRefState& state, const LeadAimHullExtents& hull,
    const ShipLeadRandomDraws& draws, bool target_is_hull_sampler,
    const ApproachTargetSections* sections) noexcept {
    // 009FA266/009FA26B: with no target the body only clears the dirty byte.
    if (!state.tracking) {
        state.dirty_41 = false;  // 009FA2CE
        return state.body_offset_28;
    }

    std::array<float, 3> pick{{0.0f, 0.0f, 0.0f}};
    if (target_is_hull_sampler) {
        // 009FA2A0 CALL EAX with slot +100h == 00816650, `this` the target.
        //
        // Under the constructor's seed (section_chance = -1.0 fails 00816659)
        // the records are never read; 009C3DA0's dive rows store 0.0 to 1.0
        // there (009C3E56). With no records passed they stay empty and
        // unavailable, which is the host's shape before packet
        // cc9_ship_section_points.
        const ApproachTargetSections none{};
        const ApproachTargetSections& s = sections != nullptr ? *sections : none;
        const bool live = sections != nullptr;
        pick = ship_lead_point_00816650(
            s.sections, hull, state.spread_48, state.centre_54,
            state.section_chance_64, state.weight_68, state.weight_6c,
            state.weight_70, draws, live && s.engine_room_available,
            live && s.magazine_available, live && s.fuel_tank_available);
    } else {
        // slot +100h == 0042D810: the origin, whatever the arguments are.
        pick = entity_lead_point_0042d810();
    }

    // 009FA2A2-009FA2B0 stores the pick, then 009FA2B3-009FA2CB adds the bias.
    state.body_offset_28[0] = pick[0] + state.bias_34[0];
    state.body_offset_28[1] = pick[1] + state.bias_34[1];
    state.body_offset_28[2] = pick[2] + state.bias_34[2];
    state.dirty_41 = false;  // 009FA2CE
    return state.body_offset_28;
}

bool approach_target_ref_needs_pick_009fada0(ApproachTargetRefState& state,
                                             float dt, float rearm_draw,
                                             bool validity_still_holds) noexcept {
    // 009FAE1C: a frozen or untargeted sub-object does nothing further.
    if (!state.tracking) return false;

    bool pick = false;
    // 009FAE26-009FAE35: the timer counts UP by dt and is stored back.
    state.repick_timer_60 += dt;
    // 009FAE38-009FAE44 against 00CE3958 = 2.0.
    if (state.repick_timer_60 > kApproachTargetRefRepickPeriod) {
        // 009FAE61: re-armed to Uniform(-0.5, 0.5), so the next expiry is 1.5
        // to 2.5 s away.
        state.repick_timer_60 = rearm_draw;
        // 009FAE69-009FAEA3: slot +104h is handed the pick with the bias taken
        // back off, by value. 009FAEA5 JNZ skips the re-pick when it answers
        // true. Every class that samples a hull carries 0042BB20 there, which
        // always answers true, so this branch does not fire for them.
        if (!validity_still_holds) pick = true;  // 009FAEAB
    }
    // 009FAEB0-009FAEB8: the dirty byte forces a pick regardless.
    if (state.dirty_41) pick = true;
    return pick;
}

void approach_target_ref_store_world_point_009faeea(
    ApproachTargetRefState& state, const CameraMatrix& matrix) noexcept {
    // 009FAEDF CALL 004142E0, then the three stores at 009FAEEA/F5/00.
    std::array<float, 3> world{};
    transform_point_004142e0(state.body_offset_28, matrix, world);
    state.world_point_1c = world;
}

void approach_target_ref_freeze_009fadae(ApproachTargetRefState& state,
                                         const CameraMatrix& matrix) noexcept {
    // 009FADAE-009FAE18: one last sample through the same transform, then
    // sub+14h, sub+18h and the dirty byte are cleared, which freezes the point.
    approach_target_ref_store_world_point_009faeea(state, matrix);
    state.tracking = false;   // 009FAE11
    state.dirty_41 = false;   // 009FAE18
    state.frozen = true;
}

// ---------------------------------------------------------------------------
// The deterministic draw substitution
// ---------------------------------------------------------------------------
std::array<float, 4> approach_target_ref_unit_draws_substitute(
    std::uint32_t seed) noexcept {
    // LABELLED SUBSTITUTION for 00BD2F10 on stream ECX=1. A counter-based
    // integer hash, so the same approach gets the same hull point on every run
    // and the per-round census lines stay comparable. Not the image's sequence.
    std::array<float, 4> out{};
    for (std::uint32_t i = 0; i < 4U; ++i) {
        std::uint32_t h = seed * 0x9E3779B1U + i * 0x85EBCA6BU;
        h ^= h >> 16;
        h *= 0x7FEB352DU;
        h ^= h >> 15;
        h *= 0x846CA68BU;
        h ^= h >> 16;
        // [0,1) from the top 24 bits.
        out[i] = static_cast<float>(h >> 8) * (1.0f / 16777216.0f);
    }
    return out;
}

void approach_target_ref_lead_tail_009faf05(ApproachTargetRefState& state,
                                            const std::array<float, 3>& predicted,
                                            const std::array<float, 3>& origin) noexcept {
    for (std::size_t i = 0; i < 3; ++i) {
        // 009FAF3C-009FAF5E: FSUB the pose origin, FSTP float.
        const float delta = static_cast<float>(static_cast<double>(predicted[i]) -
                                               static_cast<double>(origin[i]));
        // 009FAF62-009FAF7D: FADD into sub+1Ch..24h, FSTP float.
        state.world_point_1c[i] = static_cast<float>(
            static_cast<double>(state.world_point_1c[i]) + static_cast<double>(delta));
    }
}

void approach_target_ref_apply_aim_error(ApproachTargetRefState& state,
                                         const ApproachAimErrorRow& row,
                                         float h_draw, float v_draw) noexcept {
    // 009FA380: bias = (d_h, 0.0, d_v), dirty = 1 (009FA395).
    state.bias_34 = {h_draw, 0.0f, v_draw};
    // 009C3E2E-009C3E38 / 009D032C-009D033B: one row value on all three axes.
    state.spread_48 = {row.select_prec, row.select_prec, row.select_prec};
    state.dirty_41 = true;  // 009C3E3D / 009D0348
    if (row.sets_sections) {
        state.section_chance_64 = row.section_chance;  // 009C3E56
        state.weight_68 = row.engine_room_weight;      // 009C3E59
        state.weight_6c = row.magazine_weight;         // 009C3E66
        state.weight_70 = row.fuel_tank_weight;        // 009C3E6B
    }
}

float dive_bomb_projtime_009c7e3c(bool impact_arm, float fall_time,
                                  float aim_time_error_c8) noexcept {
    if (!impact_arm) return 0.0f;  // 009C7D61-009C7D65
    // 009C7E3C FLD [ESI+C8h], FADD [ESP+38h], FSTP float.
    const float t = static_cast<float>(static_cast<double>(aim_time_error_c8) +
                                       static_cast<double>(fall_time));
    if (0.0f > t) return 0.0f;                                        // 009C7E50
    if (static_cast<double>(t) > kProjtimeCap) return static_cast<float>(kProjtimeCap);  // 009C7E63
    return t;                                                         // 009C7E85
}

std::array<float, 3> ship_predict_position_008120e0(const ShipPredictInputs& in,
                                                    float t) noexcept {
    const double td = static_cast<double>(t);
    // 008120F8-00812101: the turn over t, rounded to [ESP+4].
    float turn = static_cast<float>(static_cast<double>(in.yaw_rate) * td);
    // 00812105-00812121: |turn|, the negative arm as -0.0 - turn.
    const float magnitude = (turn > 0.0f) ? turn : -turn;
    if (static_cast<double>(magnitude) > kShipPredictMinTurn &&
        static_cast<double>(in.speed) > kShipPredictMinSpeed) {
        // 00812185 ClampInPlace: lo first, then hi.
        if (kShipPredictTurnLo > turn) {
            turn = kShipPredictTurnLo;
        } else if (turn > kShipPredictTurnHi) {
            turn = kShipPredictTurnHi;
        }
        // 0081219D-008121C8: the velocity's heading, y = v.z and x = v.x as
        // 009F9E40 hands them to the library.
        float heading = static_cast<float>(
            kHeadingBasisQuarterTurn -
            static_cast<double>(static_cast<float>(std::atan2(
                static_cast<double>(in.velocity[2]), static_cast<double>(in.velocity[0])))));
        if (0.0f > heading) {
            heading = static_cast<float>(static_cast<double>(heading) + kHeadingBasisFullTurn);
        }
        const float half_turn = static_cast<float>(static_cast<double>(turn) * 0.5);
        const float mean = wrapped_angle_subtract_00438b10(heading, half_turn);  // 008121E8
        const std::array<float, 2> dir = heading_to_direction_006bc0c0(mean);    // 008121FD
        const double s = static_cast<double>(in.speed);                          // 00812209
        const float sx = static_cast<float>(static_cast<double>(dir[0]) * s);    // 00812224
        const float sz = static_cast<float>(static_cast<double>(dir[1]) * s);    // 0081222C
        const float dx = static_cast<float>(static_cast<double>(sx) * td);       // 0081223E
        const float dz = static_cast<float>(static_cast<double>(sz) * td);       // 00812246
        return {static_cast<float>(static_cast<double>(in.position[0]) + dx),
                static_cast<float>(static_cast<double>(in.position[1]) + 0.0),   // 00D7A258
                static_cast<float>(static_cast<double>(in.position[2]) + dz)};
    }
    // 0081229E-00812304, the straight arm.
    std::array<float, 3> out{};
    for (std::size_t i = 0; i < 3; ++i) {
        const float step = static_cast<float>(static_cast<double>(in.velocity[i]) * td);
        out[i] = static_cast<float>(static_cast<double>(in.position[i]) +
                                    static_cast<double>(step));
    }
    return out;
}

ShipLeadRandomDraws approach_target_ref_draws_from_unit(
    const std::array<float, 4>& unit,
    const std::array<float, 3>& spread, float section_total) noexcept {
    ShipLeadRandomDraws draws;
    // 0081667C: the roll over [0, 1). 00816796: the pick over
    // [0, total - 1e-4); it only matters when the roll lands under a positive
    // chance and some section survives. 00816883/008168A0/008168D5 give the
    // three box ranges: x and z are two sided over the spread, y is one sided
    // from zero.
    draws.section_roll = unit[3];
    const float span = section_total - static_cast<float>(kInterceptCoefficientEpsilon);
    draws.pick = span > 0.0f ? unit[0] * span : 0.0f;
    draws.box_x = (unit[0] * 2.0f - 1.0f) * spread[0];
    draws.box_y = unit[1] * spread[1];
    draws.box_z = (unit[2] * 2.0f - 1.0f) * spread[2];
    return draws;
}

ShipLeadSections ship_section_points_0081f980(
    const std::vector<GeomMeshResourcePayload>& meshes) {
    ShipLeadSections out;
    out.engine_room.id = 5;
    out.magazine.id = 8;
    out.fuel_tank.id = 6;
    for (const GeomMeshResourcePayload& mesh : meshes) {
        for (const GeomMeshElement& el : mesh.elements) {
            // 00820566 (8), 008205D7 (5), 00820648 (6).
            ShipLeadSection* slot = el.kind == 5 ? &out.engine_room
                : el.kind == 8 ? &out.magazine : el.kind == 6 ? &out.fuel_tank : nullptr;
            if (slot == nullptr) continue;
            float lo[3] = {1e30f, 1e30f, 1e30f};
            float hi[3] = {-1e30f, -1e30f, -1e30f};
            bool any = false;
            for (std::uint16_t ord : el.triangle_ordinals) {
                if (ord >= mesh.triangles.size()) continue;
                const GeomMeshTriangle& t = mesh.triangles[ord];
                for (std::uint16_t vi : {t.v0, t.v1, t.v2}) {
                    if (vi >= mesh.vertices.size()) continue;
                    any = true;
                    for (int k = 0; k < 3; ++k) {
                        lo[k] = std::min(lo[k], mesh.vertices[vi][k]);
                        hi[k] = std::max(hi[k], mesh.vertices[vi][k]);
                    }
                }
            }
            if (!any) continue;
            slot->present = true;  // +A74h / +A94h / +A84h
            for (int k = 0; k < 3; ++k) {
                slot->point[static_cast<std::size_t>(k)] = (lo[k] + hi[k]) * 0.5f;  // 00723030
            }
        }
    }
    return out;
}

float approach_target_ref_section_total(const ApproachTargetRefState& state,
                                        const ApproachTargetSections& s) noexcept {
    if (!(state.section_chance_64 > 0.0f)) return 0.0f;  // 00816659
    float total = 0.0f;
    if (state.weight_68 > 0.0f && s.sections.engine_room.present && s.engine_room_available)
        total += state.weight_68;
    if (state.weight_6c > 0.0f && s.sections.magazine.present && s.magazine_available)
        total += state.weight_6c;
    if (state.weight_70 > 0.0f && s.sections.fuel_tank.present && s.fuel_tank_available)
        total += state.weight_70;
    return total;
}

}  // namespace bsp
