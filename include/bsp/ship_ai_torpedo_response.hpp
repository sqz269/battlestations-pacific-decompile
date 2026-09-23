#pragma once

// The ship AI's torpedo response, packet cc9_ship_torpedo_response
// (docs/SHIP_TORPEDO_RESPONSE.md). Semantic C++ for MSVC Win32, not a binary
// replacement; names are hypotheses. Offsets are relative to blk = brain+8h.
//
//   009F163F..009F1855  the pre-pass torpedo walk's admission test
//   009F0AD0            admit: refresh a track's lifetime or draw and build one
//   009EACA0            the 68h contact track's constructor, torpedo arm
//   009DC060            the track refresh, torpedo arm (+48h live)
//
// The consumers, 009E04E0 (throttle profile and avoidance vector) and
// 009DE8F1 (the heading override), are already reconstructed in
// ship_ai_clearance_profile.cpp and ship_ai_arm_final_step.cpp.

#include "bsp/ship_ai_clearance_profile.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace bsp {

// NavigatorBot's torpedo fields per skill row (robot_config 009D53B0 reads
// them into the config at F8A688, header 0Ch, stride 24h). Indices are the
// reader's: Stun 0, SPNormal 1, SPVeteran 2, MPNormal 3, MPVeteran 4, Elite 5.
struct ShipAiNavigatorTorpedoRow {
    float predict_lo;          // +10h TorpedoPredict[1]
    float predict_hi;          // +14h TorpedoPredict[2]
    float reference_length;    // +18h TorpedoPredictReferenceLength
    float observation_lo;      // +1Ch TorpedoObservation[1]
    float observation_hi;      // +20h TorpedoObservation[2]
    float sub_addon;           // +24h TorpedoObservationSubAddon
    float spd_err_lo;          // +28h TorpedoSpdErr[1]
    float spd_err_hi;          // +2Ch TorpedoSpdErr[2]
};
// This installation's scripts/datatables/robots.lua (mtime 2025-06-01),
// NavigatorBot block, lines 490-549.
extern const std::array<ShipAiNavigatorTorpedoRow, 6> kShipAiNavigatorTorpedoRows;

inline constexpr double kShipAiTorpedoHullScale = 0.6000000238418579; // 00CEFF98, 0.6f widened
inline constexpr double kShipAiTorpedoHorizonAdd = 3.0;               // 00D7A2B0
inline constexpr int kShipAiTrackCapacity = 0x80;                     // 009F0AE9 CMP 80h
inline constexpr float kShipAiTrackLifetimeAdd = 1.0f;                // 00D7A210 (double 1.0)
inline constexpr double kShipAiOwnSidePredictFloor = 25.0;            // 00CE3880
inline constexpr float kShipAiOwnSidePredict = 25.0f;                 // 00CE89CC
inline constexpr float kShipAiTorpedoTrackHalfLength = 5.0f;          // 00CE3850, track+0h
inline constexpr float kShipAiTorpedoTrackLength = 12.0f;             // 00CEB4B8, track+8h

// One live torpedo as the walk and the refresh read it.
struct ShipAiTorpedoCandidate {
    std::uint64_t key{0};        // the entity identity node+48h holds
    float x{0.0f}, z{0.0f};      // entity+FCh / +104h
    float vx{0.0f}, vz{0.0f};    // vtable[34h] (006E2860): projectile+318h / +320h
    float heading{0.0f};         // record+46Ch, the commanded heading
    float run_seconds{0.0f};     // record+488h, 0085748A adds dt on each swim step
    float water_travel_speed{0.0f}; // [[record+314h]+E4h]
    int side{0};                 // entity+54h
    bool from_submarine{false};  // record+49Ch
    std::size_t shooter{0};      // entity+4F8h, one based, 0 = none
};

// The 68h track (docs/SHIP_TORPEDO_RESPONSE.md, section 2). `track` carries the
// fields 009E04E0 reads; the rest are the fields only the producer touches.
struct ShipAiTorpedoTrack {
    ShipAiContactTrack track{};
    std::uint64_t key{0};        // +48h, the observed torpedo
    float length_08{0.0f};       // +8h
    float spd_err_1c{0.0f};      // +1Ch
    bool source_live{true};      // +48h still non-null (the observer clears it)
};

// 009F1316..009F13BE: the brain constructor's timer draws for the torpedo
// timer. B48h = -U(0, 1); B44h = U(settings+1ECh, settings+1F0h).
struct ShipAiTorpedoTimer {
    float period_b44{1.0f};
    float countdown_b48{0.0f};
    bool seeded{false};
};

// The stream-1 draw 00BD2F10 with ECX = 1, supplied by the host.
struct ShipAiTorpedoDraw {
    virtual ~ShipAiTorpedoDraw() = default;
    virtual float uniform_00bd2f10(float low, float high) = 0;
};

// 009F158A..009F15C6 on B48h/B44h (the rule 009F15CC shares).
bool ship_ai_torpedo_timer_due_009f158a(ShipAiTorpedoTimer& timer, float seconds) noexcept;

// 009F163F..009F16xx: the horizon, TorpedoPredict[2] + TorpedoObservation[2]
// + (B44h + 3.0), with the x87 spill order of the walk.
float ship_ai_torpedo_horizon(float period_b44, const ShipAiNavigatorTorpedoRow& row) noexcept;

// 009F17xx..009F1839: admit when the torpedo is inside 0.6 * length, or when
// it can close the distance within the horizon and is closing.
bool ship_ai_torpedo_admits(float own_x, float own_z, float own_vx, float own_vz,
                            float own_length, float horizon,
                            const ShipAiTorpedoCandidate& torpedo) noexcept;

// 009F0AD0 then 009EACA0 and 009DC060. `collect_timer_2` is settings+1F0h.
// Returns true when a new track was built (the draws were made).
bool ship_ai_admit_torpedo_track_009f0ad0(std::vector<ShipAiTorpedoTrack>& tracks,
                                          const ShipAiTorpedoCandidate& torpedo,
                                          const ShipAiNavigatorTorpedoRow& row,
                                          float own_length, int own_side,
                                          float collect_timer_2, ShipAiTorpedoDraw& draw);

// 009DC060's +48h arm. `live` is null once the torpedo is gone, which is the
// +64h retire at 009DC2CC.
bool ship_ai_refresh_torpedo_track_009dc060(ShipAiTorpedoTrack& track,
                                            const ShipAiTorpedoCandidate* live) noexcept;

// 009E1046..009E106E: the destroy compaction, the last pointer moves into the
// hole and the count drops.
void ship_ai_destroy_track_009e0fbd(std::vector<ShipAiTorpedoTrack>& tracks,
                                    std::size_t index) noexcept;

} // namespace bsp
