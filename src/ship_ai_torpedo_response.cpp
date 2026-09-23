#include "bsp/ship_ai_torpedo_response.hpp"

#include "bsp/ship_ai_neighbour_candidates.hpp"
#include "bsp/vector_helpers.hpp"

namespace bsp {

// robots.lua lines: Stun 540-548, SPNormal 490-499, SPVeteran 500-509,
// MPNormal 510-519, MPVeteran 520-529, Elite 530-539.
const std::array<ShipAiNavigatorTorpedoRow, 6> kShipAiNavigatorTorpedoRows{{
    {4.0f, 5.0f, 150.0f, 3.5f, 5.0f, 1.0f, -0.0f, 1.0f},    // 0 Stun
    {3.5f, 5.0f, 100.0f, 3.5f, 5.0f, 4.0f, -3.0f, 1.0f},    // 1 SPNormal
    {20.0f, 22.0f, 150.0f, 0.0f, 1.0f, 0.0f, -0.1f, 0.1f},  // 2 SPVeteran
    {6.0f, 8.0f, 150.0f, 1.0f, 3.0f, 3.0f, -2.0f, 1.0f},    // 3 MPNormal
    {8.0f, 10.0f, 150.0f, 0.5f, 2.0f, 1.5f, -1.5f, 1.0f},   // 4 MPVeteran
    {20.0f, 22.0f, 150.0f, 0.0f, 1.0f, 0.0f, -0.1f, 0.1f},  // 5 Elite
}};

bool ship_ai_torpedo_timer_due_009f158a(ShipAiTorpedoTimer& timer, float seconds) noexcept {
    volatile float countdown = timer.countdown_b48;
    const volatile float period = timer.period_b44;
    const bool due = ship_ai_neighbour_timer_due_009f15cc(countdown, period, seconds);
    timer.countdown_b48 = countdown;
    return due;
}

float ship_ai_torpedo_horizon(float period_b44, const ShipAiNavigatorTorpedoRow& row) noexcept {
    // FLD observation; FADD prediction; (spill of period + 3.0); FADD; FSTP.
    const float extra = static_cast<float>(static_cast<double>(period_b44) + kShipAiTorpedoHorizonAdd);
    const double sum = static_cast<double>(row.observation_hi) + row.predict_hi;
    return static_cast<float>(sum + extra);
}

bool ship_ai_torpedo_admits(float own_x, float own_z, float own_vx, float own_vz,
                            float own_length, float horizon,
                            const ShipAiTorpedoCandidate& torpedo) noexcept {
    // Hull radius squared: binary32(length * 0.6), then squared.
    const float radius = static_cast<float>(static_cast<double>(own_length) * kShipAiTorpedoHullScale);
    const float hull = radius * radius;
    const float dx = own_x - torpedo.x;
    const float dz = own_z - torpedo.z;
    const std::array<float, 2> relative{torpedo.vx - own_vx, torpedo.vz - own_vz};
    const float reach_speed = length_2d_00414c60(relative);
    const float reach = reach_speed * horizon;
    const float reach_sq = reach * reach;
    const float distance_sq = static_cast<float>(static_cast<double>(dx) * dx + static_cast<double>(dz) * dz);
    if (hull > distance_sq) return true;             // FCOMIP / JBE: strictly inside
    if (!(reach_sq > distance_sq)) return false;      // strict dynamic reach
    const float dot = dx * relative[0] + dz * relative[1];
    return dot > 0.0f;                                // closing (SETA)
}

bool ship_ai_admit_torpedo_track_009f0ad0(std::vector<ShipAiTorpedoTrack>& tracks,
                                          const ShipAiTorpedoCandidate& torpedo,
                                          const ShipAiNavigatorTorpedoRow& row,
                                          float own_length, int own_side,
                                          float collect_timer_2, ShipAiTorpedoDraw& draw) {
    // 009F0AE9: a full list admits nothing.
    if (static_cast<int>(tracks.size()) >= kShipAiTrackCapacity) return false;
    // 009F0AF9..009F0B3A: a known torpedo only refreshes its lifetime.
    for (ShipAiTorpedoTrack& existing : tracks) {
        if (existing.key == torpedo.key) {
            existing.track.lifetime_14 = collect_timer_2 + kShipAiTrackLifetimeAdd;
            return false;
        }
    }
    // 009F0B45..009F0B6B: predict, scaled by length / reference when longer.
    float predict = draw.uniform_00bd2f10(row.predict_lo, row.predict_hi);
    if (row.reference_length < own_length) {
        predict = (own_length / row.reference_length) * predict;
    }
    // 009F0B6F..009F0B9C: observation, plus the submarine addon.
    float observation = draw.uniform_00bd2f10(row.observation_lo, row.observation_hi);
    if (torpedo.from_submarine) observation = row.sub_addon + observation;
    // 009F0BA0..009F0BB4: the speed error.
    const float spd_err = draw.uniform_00bd2f10(row.spd_err_lo, row.spd_err_hi);
    // 009F0BB9..009F0BE8: an own-side torpedo is seen at once and early.
    if (torpedo.side == own_side) {
        observation = 0.0f;
        if (static_cast<double>(predict) <= kShipAiOwnSidePredictFloor) {
            predict = kShipAiOwnSidePredict;
        }
    }
    // 009F0BEC..009F0C40, then 009EACA0(5.0, 12.0, reach, torpedo, observation,
    // spd_err, 1).
    ShipAiTorpedoTrack fresh;
    fresh.key = torpedo.key;
    fresh.track.half_length_00 = kShipAiTorpedoTrackHalfLength;              // +0h
    fresh.track.lead_time_04 = kShipAiTorpedoTrackLength * 0.5f;             // +4h, 00D7A280
    fresh.length_08 = kShipAiTorpedoTrackLength;                             // +8h
    fresh.track.max_horizon_0c = (spd_err + torpedo.water_travel_speed) * predict; // +0Ch
    fresh.track.range_10 = observation;                                      // +10h
    fresh.spd_err_1c = spd_err;                                              // +1Ch
    fresh.track.retired_64 = false;                                          // +64h
    fresh.track.lifetime_14 = collect_timer_2 + kShipAiTrackLifetimeAdd;     // +14h, kind 1
    ship_ai_refresh_torpedo_track_009dc060(fresh, &torpedo);                 // 009EADC2
    tracks.push_back(fresh);
    return true;
}

bool ship_ai_refresh_torpedo_track_009dc060(ShipAiTorpedoTrack& track,
                                            const ShipAiTorpedoCandidate* live) noexcept {
    if (track.track.retired_64) return false;
    if (live == nullptr) {
        track.source_live = false;
        track.track.retired_64 = true;   // 009DC2CC
        return false;
    }
    track.track.pos_x_2c = live->x;                        // 009DC09C
    track.track.pos_z_30 = live->z;                        // 009DC0A1
    const std::array<float, 2> velocity{live->vx, live->vz};
    const float speed = length_2d_00414c60(velocity);      // 009DC0CC
    track.track.speed_18 = speed;
    track.track.dir_x_24 = velocity[0] / speed;
    track.track.dir_z_28 = velocity[1] / speed;
    track.track.heading_20 = live->heading;                // record+46Ch
    track.track.speed_18 = track.spd_err_1c + track.track.speed_18;
    return true;
}

void ship_ai_destroy_track_009e0fbd(std::vector<ShipAiTorpedoTrack>& tracks,
                                    std::size_t index) noexcept {
    if (index >= tracks.size()) return;
    if (index + 1 < tracks.size()) tracks[index] = tracks.back();
    tracks.pop_back();
}

} // namespace bsp
