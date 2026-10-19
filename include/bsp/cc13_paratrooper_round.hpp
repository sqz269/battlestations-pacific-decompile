#pragma once

// Packet cc13-para2 (paratrooper option 2): the dropped MParatrooper round
// (class id 31h) as a host-side world. docs/CC13_PARATROOPER.md section
// "Option 2".
//
// The per-routine bodies are the segment-46 reference models in
// bsp/gameplay_seg46_cc13.hpp (007AB8C0, 007AB230, 007ABB30, 007AB360,
// 007AB0B0, 007AC000, 007AB590, 007AAE10). This file adds what those models
// leave to a host:
// - the round's own position and 006E1510 / 006E2220 steps;
// - the order the image runs the round's slots in each fixed step: the
//   per-step callback list (00874DE0, which runs 007AB590 once it has been
//   registered), then job wave 1 (tick element +4h 006E1060, which calls
//   entity +194h = 007AC000), then job wave 3 (tick element +8h = 007AB360);
// - 006F2C30 for the landing search (kind 0, the building's +7C8h);
// - the building capturer list +7DCh (006F39B0 appends, 006F34B0 erases);
// - 00803510.
// The command buildings, the ground, the avoid zones and the draws are
// RoundServices callbacks, so nothing here invents a global.
//
// LABELLED substitutions (each is named again where it is used):
// - +DCh (007AB0B0, the animation clock and the capture timer) is an entity
//   frame slot. The host runs it once per fixed step, after wave 3, with the
//   fixed step as dt.
// - The chute's "fall", "open" and "close" lengths come from the model's
//   animation resource (00B6DC90), which the host does not load. The host
//   takes the class OpenDuration and CloseDuration as the open and close
//   lengths, and 1.0 as the fall length (only its fmod reads it).
// - The class step scale +5Ch is the BombClass constructor's 1.0f (no Lua key).
// - +174h (the network id that seeds the avoid-zone radius) is the host's
//   round serial, cut to 16 bits.
// - The round's entity kill 00926D90 runs its +7Ch (007AAE10) at once, and
//   the round leaves the world at the end of the step.
// - The soldier (SoldierClass vslot +8h, then +98h, +114h, +A0h) is not a host
//   entity: the round keeps a nonzero handle so 007AC000 stops searching, and
//   the soldier's own update is not modelled. Its 004B0F80 start-phase draw is
//   taken only under its sub-switch.

#include "bsp/gameplay_seg46_cc13.hpp"
#include "bsp/soldier_traffic_cc13.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {
namespace cc13_paratrooper {

using gameplay_seg46_cc13::ParatrooperClassFields;
using gameplay_seg46_cc13::ParatrooperState;
using gameplay_seg46_cc13::Vec3;

// One entry of [[game+19CCh]+16Ch], the list 006F2C30 walks (host world list
// 28). `handle` is the host's unit index + 1.
struct BuildingView {
    std::uint32_t handle{0};
    int side_54{0};
    Vec3 position_fc{};
    std::int32_t inferior_range_7c8{0};
};

struct RoundServices {
    virtual ~RoundServices() = default;
    // 00BD2F10(ECX = stream, lo, hi).
    virtual float uniform(int stream, float lo, float hi) = 0;
    // 00903860: -1000.0 where no terrain answers.
    virtual float ground_height_00903860(const Vec3& point) = 0;
    // 009038F0: false where no terrain answers.
    virtual bool ground_normal_009038f0(const Vec3& point, Vec3& normal) = 0;
    // 004218E0 -> 004120D0(0) -> 0041B840(out, xz, slack, push). False when
    // the host has no avoid-zone group for layer 0 (xz unchanged).
    virtual bool avoid_zone_nearest_0041b840(float xz[2], float slack, float push) = 0;
    virtual std::size_t building_count() = 0;
    virtual BuildingView building(std::size_t index) = 0;
    virtual int building_side_54(std::uint32_t handle) = 0;
    virtual float building_armour_368(std::uint32_t handle) = 0;
    // The building's vslot +1ACh (006F1F20 for a CommandBuilding).
    virtual void building_damage_1ac(std::uint32_t handle, float amount) = 0;
    virtual Vec3 building_position_fc(std::uint32_t handle) = 0;
    virtual void record(const char* name, std::uint32_t address) = 0;
    virtual void note(const std::string& line) = 0;
};

// What 006E4D50's bomb arm gives the round at the drop (006E0A70 and
// 006E1F00 through the host's bomb release): the start position, the release
// velocity +318h and the drift acceleration +348h (y = 0).
struct ReleaseState {
    Vec3 position{};
    Vec3 velocity_318{};
    Vec3 accel_348{};
    int side_54{0};
    std::size_t dropper_unit{0};
    int bullet_class{-1};
};

struct Capturer {
    int side_54{0};
    float capture_power_d8{0.0f};
};

struct Round {
    std::uint32_t id{0};
    ParatrooperState s{};
    ParatrooperClassFields cls{};
    Vec3 position{};        // +A4h; the dropped round has no parent, so local is world
    Vec3 accel_348{};
    std::size_t dropper_unit{0};
    int bullet_class{-1};
    bool alive{true};
    bool callback_registered{false};   // 00875A80(+3E4h, 0)
    float dropped_at{0.0f};
    float landed_at{-1.0f};
    const char* death{nullptr};
};

struct RoundCounters {
    unsigned long long constructs{0};
    unsigned long long construct_draws{0};
    unsigned long long drops{0};
    unsigned long long landed{0};
    unsigned long long in_water{0};
    unsigned long long bad_landings{0};
    unsigned long long building_found{0};
    unsigned long long soldiers{0};
    unsigned long long soldier_draws{0};
    unsigned long long capturer_adds{0};
    unsigned long long capturer_removes{0};
    unsigned long long damage_calls{0};
    double damage_total{0.0};
    unsigned long long kills{0};
    unsigned long long kills_friendly_building{0};
    unsigned long long kills_no_building{0};
    unsigned long long kills_capture_done{0};
    unsigned long long kills_bad_landing{0};
    unsigned long long kills_water{0};
    unsigned long long steps{0};
};

// The class lengths the chute animation takes (LABELLED, see the file head).
struct ChuteLengths {
    float fall{1.0f};
    float open{0.0f};
    float close{0.0f};
};

class RoundWorld {
public:
    // 007AB8C0 then 007AC530 for a round 006E3E70 hangs on a 31h rack. The
    // constructor's U(ParachuteOpenAltitudeMin, Max) is stream 1 (007AB94A);
    // without `draw` it is not taken and +468h is the midpoint (recorded).
    ParatrooperState construct(const soldier_traffic_cc13::SoldierGlobals& g, int side_54,
                               const ChuteLengths& lengths, bool draw, RoundServices& svc);
    // 006E4D50 for a hung 31h round: 006E1F00's flying byte, velocity and
    // drift, then (when `has_landing`, the rack+4F8h branch) 007AB230 with the
    // landing point 006E541F drew. Returns the round's id.
    std::uint32_t drop(const ParatrooperState& hung, const ParatrooperClassFields& cls,
                       const ReleaseState& release, bool has_landing, const Vec3& landing,
                       float now, RoundServices& svc);
    // One fixed step: the callback list (007AB590), wave 1 (007AC000), wave 3
    // (007AB360), then +DCh (007AB0B0), each over the live rounds in drop order.
    void step(float dt, float now, const soldier_traffic_cc13::SoldierGlobals& g,
              bool soldier_draw, RoundServices& svc);
    // 006F6760 arm 3's input: the building's capturer list in 006F39B0 order.
    void capturers(std::uint32_t building, std::vector<Capturer>& out) const;
    std::size_t live() const noexcept { return rounds_.size(); }
    const RoundCounters& counters() const noexcept { return counters_; }

private:
    friend class RoundAdapter;
    std::vector<Round> rounds_;
    std::vector<std::pair<std::uint32_t, std::uint32_t>> capturer_list_;   // (building, round id)
    std::uint32_t next_id_{1};
    RoundCounters counters_{};
};

// 00803510 BSP_Party_RelativeTo(subject, reference): 2 when exactly one side is
// neutral (2), 0 when they match, 1 otherwise.
int party_relative_00803510(int subject, int reference) noexcept;

// 006F2C30 with kind 0 (the +7C8h range), the paratrooper's call at 007AC077:
// the nearest entry whose +54h differs from `side`, by 3-D squared distance
// against (float)range squared (<=), the first of equals kept. 0 for none.
std::uint32_t nearest_other_side_building_006f2c30(const Vec3& position, int side,
                                                   RoundServices& svc);

}  // namespace cc13_paratrooper
}  // namespace bsp
