// bsp_game.exe milestone 2n: the ship AI controller per unit, and the weapon
// director's own automatic target selector.
//
// Nothing here is a reconstruction of native code. Every method is one call site
// of bsp::ShipAiControllerHost, bsp::ShipAiSyncHost, bsp::ShipAiSetterHost,
// bsp::ShipAiDirectControlHost, bsp::ShipAiPublishHost or bsp::BotFireTargetHost,
// satisfied either by a reconstruction already on main or by the explicit
// unimplemented policy in GameHostLog. Addresses, evidence and the decisions
// this file makes rather than recovers: include/bsp/game_hosts_ship_ai.hpp,
// docs/SHIP_AI_STATES.md, docs/UNIT_AUTOPILOT_PAIR.md, docs/BOT_FIRE_TARGET.md
// and the milestone 2n section of docs/GAME_EXECUTABLE.md.

#include "bsp/game_hosts_ship_ai.hpp"
#include "bsp/game_avoid_zone_runtime.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/attack_target_classify.hpp"
#include "bsp/hit_narrowphase.hpp"
#include "bsp/ship_ai_goal_vector_visibility.hpp"
#include "bsp/ship_ai_search_storage.hpp"
#include "bsp/session_participant_pools.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <stdexcept>
#include <vector>

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/command_completion.hpp"
#include "bsp/director_update_arms.hpp"
#include "bsp/ship_ai_approach_update.hpp"
#include "bsp/ship_ai_attackmove_substates.hpp"
// Packet cc8_ship_follow: the `follow` state's two halves and the unit group.
#include "bsp/ship_ai_follow_land.hpp"
#include "bsp/ship_ai_station_keeping.hpp"
#include "bsp/ship_ai_torpedo_response.hpp"
#include "bsp/ship_ai_arm_final_step.hpp"
#include "bsp/ship_ai_layer_selection.hpp"
#include "bsp/ship_ai_formation.hpp"
#include "bsp/game_hosts_gunnery.hpp"
#include "bsp/recon_sensor_pass.hpp"
#include "bsp/gamepad_force_events.hpp"
#include "bsp/unit_gunnery_pass.hpp"
#include "bsp/projectile_kinds.hpp"
#include "bsp/gun_aiming.hpp"
#include "bsp/gun_gravity_arc.hpp"
#include "bsp/gun_heading_snap.hpp"
#include "bsp/gameplay_settings_tail.hpp"
#include "bsp/ship_ai_approach_curves.hpp"
#include "bsp/ship_ai_approach_tune.hpp"
#include "bsp/ship_ai_bearing_rating.hpp"
#include "bsp/ship_ai_ring_scan.hpp"
#include "bsp/ship_ai_clearance_profile.hpp"
#include "bsp/ship_ai_nav_block_ctor.hpp"
#include "bsp/ship_ai_navigation.hpp"
#include "bsp/ship_ai_navigation_arm_tail.hpp"
#include "bsp/ship_ai_path_follower.hpp"
#include "bsp/ship_ai_path_planner.hpp"
#include "bsp/ship_ai_sector_scan.hpp"
#include "bsp/ship_ai_hull_geometry.hpp"
#include "bsp/ship_ai_path_point.hpp"
#include "bsp/ship_ai_path_refresh.hpp"
#include "bsp/ship_ai_path_search.hpp"
#include "bsp/ship_ai_goal_vector.hpp"
#include "bsp/ship_ai_obstacle_tables.hpp"
#include "bsp/ship_ai_state_steps.hpp"
#include "bsp/ship_ai_throttle_ring.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/unit_state_message.hpp"
#include "bsp/vector_helpers.hpp"
#include "bsp/vehicle_class.hpp"
#include "bsp/weapon_director.hpp"

namespace bsp::game {

// Packet cc9_ship_firepower, docs/SHIP_AI_FIREPOWER.md. True: the eight
// predicates 0095EB40 asks about a mount answer from the gun rows the gunnery
// host keeps (barrel timers, arcs, the refined projectile sub-type, the Bullets
// row's blast minimum). False: the earlier placeholders.
inline constexpr bool kShipFirepowerBound = true;
// Packet cc9_hit_accuracy, docs/WEAPON_HIT_ACCURACY.md. True: 006EB060's profile
// answer is 008386F0/008383D0 over the four WeaponHitAccuracy sub-objects loaded
// from ShipGlobals. False: the 00836EF0 default's small curve at int(10t).
inline constexpr bool kWeaponHitAccuracyBound = true;
// Packet cc9_target_curve, docs/SHIP_AI_TARGET_CURVE.md. True: 009F2FB1's curve
// is the TARGET unit's rating of this ship, over the block 009F28FA..009F29D8
// fills at nested+1238h from this ship's own fields. False: the old curve,
// this ship's own rating without the long-range bias.
inline constexpr bool kShipAiTargetCurveBound = true;
// Packet cc9_own_curve_target, docs/SHIP_AI_OWN_CURVE.md. True: the own curve's
// block at nested+127Ch describes the approach target as 009F2A26..009F2A77 read
// it, with 009F2A91..009F2AC1's constants when there is none. False: the no-target
// constants always, and a zero fire divisor.
inline constexpr bool kShipAiOwnCurveTargetBound = true;
// Packet cc9_ring_query, docs/SHIP_AI_RING_QUERY.md. True: the ring scan's copy of
// nested+127Ch (009E8192..009E81A2, REP MOVSD 11h dwords) carries the target
// fields 009F2A26..009F2AC1 wrote, as in the image. False: no-target constants
// and a zero fire divisor.
inline constexpr bool kShipAiRingQueryBound = true;
// Packet cc9_ring_query. True: unit+9C8h is the unit's length, as 00810F60 writes
// it (0081106E): 2 * max(zmax, -zmin) of the model bounds, or class+A0h Length
// without them. False: 0.
inline constexpr bool kUnitRadiusBound = true;
// Packet cc9_ship_traffic, docs/SHIP_AI_TRAFFIC.md. True: 009E9190 walks the
// side's contact list (the gunnery host's [recon+DE8h] stand-in), inserts every
// kind-5 contact other than the target that lies within its own longest weapon
// range plus 200 (009E92D5), keeps it while within that range plus 300
// (009E6170), rates its weapons against this ship through 0095EB40 (009E6240)
// and steers away by the weighted sum. False: no candidates, as before.
inline constexpr bool kShipAiTrafficBound = true;
// Packet cc9_target_release, docs/SHIP_AI_TARGET_RELEASE.md. True: a unit whose
// damage death has happened reads as torn down at +5Dh / +60h (00926C80 sets
// +60h, the 009273A0 flush's 00926390 sets +5Dh), so 009F3240 takes its hold arm
// and 00836920's attackmove arm (00836BC2, 0043F080) ends the command at stage 2.
// False: the dead unit keeps answering live, and the arm stays a record.
inline constexpr bool kShipAiTargetReleaseBound = true;
// Packet cc9_ship_natives_2, docs/SHIP_NATIVES_2.md. True: the 119-step standoff
// scan's scale is nested+1284h as 009F1BC0 fills it, [target+370h] (the held
// target's health, 009F2A44) or 10000.0f with no target (009F2AA9). False: the
// no-target constant on both arms.
inline constexpr bool kApproachScanScaleBound = true;
// Packet cc9_ship_natives_3, docs/SHIP_NATIVES_3.md. True: 0071F290's arm 6 runs
// vtable[78h] (00835C70) only while director+44h, the queue head's accepted
// byte that 00835C70 writes, is clear, so once per head command rather than on
// every update. False: the byte is always clear, as before.
inline constexpr bool kDirectorBeginCommandBound = true;
// Packet cc9_ship_natives_3. True: 009F0100 with an empty neighbour list
// (blk+604h below 1) returns at 009F0169 / 009F01C5 before any store, which is
// the only state this process reaches. False: the record, as before.
inline constexpr bool kShipAiOrderTailBound = true;
// Packet cc9_ship_formation_speed, docs/SHIP_FORMATION_SPEED.md. True: 009F4DA0's
// formation throttle ceiling runs before the drive reads blk+344h / +348h, over
// the group the units host keeps, the member speeds 009DF6AA publishes and the
// class speeds and turn radii. False: blk+344h is the arm tail's value (or 1.0)
// and blk+348h 1.0, as before.
inline constexpr bool kShipFormationSpeedBound = true;
// 0082E850's multiplier, [00424C40+438h] Navigator.TurnMultipliers.
// TurnMultiplierMaxSpeed[2]: the gameplay settings object is not loaded here, so
// its documented value stands in (docs/GAMEPLAY_SETTINGS.md). LABELLED.
inline constexpr float kShipTurnRadiusMultiplier438 = 2.0f;
// Packet cc9_torpedo_evasion, docs/TORPEDO_EVASION.md. 0082E850 skips the multiplier
// when the class descriptor answers vtable[18h](0Eh), which only the torpedo-boat
// class predicate 00963E90 does (0Eh/6/5/4). True: the host asks the instance's
// kind query for 0Eh (00857DC0 answers the same chain) and returns class+520h
// unmultiplied for a torpedo boat. False: every ship is multiplied, as before.
inline constexpr bool kShipTurnRadiusTorpedoBoatExempt = true;
// Packet cc9_ship_torpedo_response, docs/SHIP_TORPEDO_RESPONSE.md. True: the brain
// pre-pass's torpedo walk (009F163F..009F1855) admits live torpedoes into
// contact tracks (009F0AD0 / 009EACA0 / 009DC060) on blk+400h/+404h, 009E04E0
// consumes them behind the real 009DA1D0 gate (throttle profile and avoidance
// vector), and 009DE5B0's section 5 (009DE8F1) points blk+324h down the vector.
// False: the walk is recorded, the list stays empty and the gate answers false.
// Landed ON after the USN04 4500 and E2 9000 pairs (docs/SHIP_TORPEDO_RESPONSE.md section 5).
inline constexpr bool kShipTorpedoResponseBound = true;
// Packet cc9_ship_torpedo_response, secondary. 0082E850 multiplies class+520h by
// settings+438h (2.0) at every call site. True: the follow step's two sites
// (009E16F0 station latch, 009E1790 back-off), the path follower's (009E3EAE)
// and the arm tail's astern threshold (009EF112) take the same value the
// formation host's class_turn_radius_0082e850 returns. False: the first three
// return class+520h unmultiplied and the arm tail's a recorded 0, as before.
// Landed ON after the USN04 4500 pair (docs/SHIP_TORPEDO_RESPONSE.md section 5).
inline constexpr bool kShipTurnRadiusSitesBound = true;
// settings+1ECh / +1F0h, TorpedoAvoidance.CollectTimer: this installation's
// shipglobals.lua line 279, { 1.5, 2 }. The settings object is not loaded here.
// Packet cc9_heading_target_sections (docs/HEADING_TARGET_SECTIONS.md). True, with
// kShipTorpedoResponseBound: (1) the walk's candidate test is the image's: slot38
// 008561F0 is the record's active byte +458h (set once released, 006E1331) and slot2C
// 00855F00 answers non-zero only in the water (2 when height < 2 * swim depth, else
// the underwater flag +354h), which the host's `swimming` projects; (2) the brain
// constructor's seven stream-1 draws (009F12CD..009F13F0) are made for every built
// controller when the draw source binds, in the constructor's order, keeping B48h and
// B44h. False: any live round is a candidate and the two timer draws happen on the
// ship's first re-plan, as the torpedo-response packet landed it.
inline constexpr bool kShipTorpedoResponseImageTerms = true;
// Packet cc9_avoid_zone_escape, docs/AVOID_ZONE_ESCAPE.md. True: 009F51C6 runs
// 009ECA20 (the navigation layer selection) over the scene's avoid zones, which
// writes blk+160h, +14Ch, +150h/+154h, +164h and the layers +308h/+30Ch/+310h, and
// 009DE5B0's sections 3 and 4 (009DE67D..009DE8ED, the escape blend) run before
// section 5. False: 009ECA20 is recorded and blk+160h stays 0, as before. The
// planner's layer argument is NOT changed by this switch (kShipPlannerTravelLayerBound).
inline constexpr bool kShipAvoidZoneEscapeBound = true;
// 009ED3E0 hands nav+30Ch to 009E3780 at 009ED523/588/5FD/692. True: the planner
// gets 009ECA20's travel layer. False: the plan block's own zone_layer, as before.
inline constexpr bool kShipPlannerTravelLayerBound = false;
inline constexpr float kTorpedoCollectTimer1 = 1.5f;
inline constexpr float kTorpedoCollectTimer2 = 2.0f;
// Packet cc9_station_keeping, docs/STATION_KEEPING.md. True: the follow update's
// station request 009DA3B0 is stored (blk+38Ch..+3A6h, including blk+39Ch = 0
// and the enable byte blk+3A5h = brain+3ADh), the pre-pass 009F145E clears the
// enable, 009ED6B0's station-keeping arm 009EDA28 runs when it is set and
// blk+3A6h clear, and 009F4DA0's brain+3ADh arm (009F4F8C..009F5020) turns the
// arm's blk+39Ch into the follower's throttle limit and escape byte; 007788B0 in
// the direct-control arm answers the formation-follower test, so 009ED73F keeps
// the enable for a follower. False: the request is recorded, 007788B0 answers
// false and neither arm runs, as before.
inline constexpr bool kShipStationKeepingBound = true;
namespace {

bool has_ship_navigation_class(int kind) noexcept {
    // Actual VehicleClass.Type leaf kinds. The ship-family virtual+210
    // reaches00810DD0/009F3F20; other entity families own different brains.
    switch (static_cast<bsp::VehicleClassKind>(kind)) {
    case bsp::VehicleClassKind::Destroyer:
    case bsp::VehicleClassKind::Submarine:
    case bsp::VehicleClassKind::MotherShip:
    case bsp::VehicleClassKind::Cruiser:
    case bsp::VehicleClassKind::Cargo:
    case bsp::VehicleClassKind::LandingShip:
    case bsp::VehicleClassKind::BattleShip:
    case bsp::VehicleClassKind::TorpedoBoat:
        return true;
    default:
        return false;
    }
}

class HullGeometryUnitAccess final : public bsp::ShipAiHullPreStepAccess {
public:
    HullGeometryUnitAccess(GameUnitsHost& units, std::size_t index,
        const std::uint32_t& class_reference)
        : units_(units), index_(index), class_reference_(class_reference) {}

    std::uint32_t class_reference_0570() override { return class_reference_; }
    float unit_reference_speed_0080fc30() override {
        return units_.throttle_ceiling_inputs(index_).reference_speed;
    }
    float unit_turn_circle_00811a30(float fraction) override {
        // The represented modifier channel5 is empty, so its scale is1.
        return units_.unit_class_turn_circle_radius_0082e960(index_, fraction);
    }
    float unit_full_beam_09cc() override {
        return units_.unit_half_width_09cc(index_);
    }

    const bsp::CameraMatrix& unit_world_pose_3fc() override {
        if (!units_.unit_pose_valid_00c8(index_)) {
            throw std::runtime_error("ship hull geometry requires a valid unit pose cache");
        }
        float right[3], up[3], forward[3], position[3];
        if (!units_.unit_pose(index_, right, up, forward, position)) {
            throw std::runtime_error("ship hull geometry unit pose is unavailable");
        }
        // 004142E0 and009DE2F0 consume these twelve affine components.
        // The unused homogeneous lanes do not represent borrowed pose fields.
        for (std::size_t lane = 0; lane < 3; ++lane) {
            world_[lane] = right[lane];
            world_[4 + lane] = up[lane];
            world_[8 + lane] = forward[lane];
            world_[12 + lane] = position[lane];
        }
        return world_;
    }

    const bsp::HitQueryBounds* unit_model_vtable20() override {
        // The represented unit/class owner creates no model box or parts.
        // Native0087BE02..0087BF5B maps absent descriptor+50h to unit+360h=0;
        // the verified virtual20 target006D1E30 returns that field.
        return nullptr;
    }

private:
    GameUnitsHost& units_;
    std::size_t index_;
    const std::uint32_t& class_reference_;
    bsp::CameraMatrix world_{};
};

// The two re-plan interval getters, read from the image for this packet.
// 009DAC30 is `FLD [00CE3958]` and 00CE3958 holds 40 00 00 00 = 2.0f; 009DAA90
// is `FLD [00CE3850]` and 00CE3850 holds 40 A0 00 00 = 5.0f. Cruise is the one
// state that carries its own getter; every other leaf vtable's +28h is 009DAA90.
constexpr std::uint32_t kCruiseIntervalGetter = 0x009dac30u;
constexpr std::uint32_t kSharedIntervalGetter = 0x009daa90u;
constexpr float kCruiseIntervalTicks = 2.0f;
constexpr float kSharedIntervalTicks = 5.0f;

// The three state objects 009F3D00 can reach through its attack arm, by their
// brain offsets. docs/SHIP_AI_STATES.md names the objects; 009F3D86, 009F3D8E
// and 009F3D96 are the three LEA sites, in ai offsets.
constexpr std::uint32_t kAiOffsetKamikaze = 0x2254u;   // brain+21FCh
constexpr std::uint32_t kAiOffsetSubAttack = 0x217Cu;  // brain+2124h
constexpr std::uint32_t kAiOffsetAttackMove = 0x0C70u; // brain+0C18h

struct StateDescriptor {
    const char* name;
    std::uint32_t command_object;
    std::uint32_t step;      // the leaf vtable's +0Ch
    std::uint32_t interval;  // the leaf vtable's +28h
    bool step_concrete;
};

// Read out of the seven leaf vtables for this packet: 00D21598 (cruise),
// 00D215C8 (stop), 00D215F8 (follow), 00D21628 (movetopos), 00D21658 (land),
// 00D21688 (moveonpath) and 00D219D0 (attackmove), plus 00D216B8 (kamikaze).
// Only `cruise`'s step has a reconstruction; the rest are records with their
// own addresses, because docs/SHIP_AI_STATES.md read none of their bodies.
const StateDescriptor* state_for_ai_offset(std::uint32_t ai_offset) noexcept {
    static const StateDescriptor kStates[] = {
        {"cruise",          0x00e08f70u, 0x009e1170u, kCruiseIntervalGetter, true},
        {"stop",            0x00e08f88u, 0x009e14c0u, kSharedIntervalGetter, false},
        // MUST STAY false, and the field is misnamed. `step_concrete` does not
        // mean "this state's step has a reconstruction" - `stop`, `movetopos`,
        // `moveonpath` and `attackmove` all have one and all carry false. It
        // gates the block at the head of state_step_vtable0c that runs the
        // CRUISE step 009E1170 and returns, so it means "this state is cruise".
        // Packet cc8_ship_follow set it true on the strength of the name and
        // sent every follower into the cruise step, which returned before the
        // `follow` arm below could run: the state was selected, 009E1610 never
        // executed, and no ShipAiFollow record appeared in the run.
        {"follow",          0x00e08f60u, 0x009e1610u, kSharedIntervalGetter, false},
        {"land",            0x00e08fa0u, 0x009e1950u, kSharedIntervalGetter, false},
        {"movetopos",       0x00e08f68u, 0x009e5770u, kSharedIntervalGetter, false},
        {"moveonpath",      0x00e08f80u, 0x009e59c0u, kSharedIntervalGetter, false},
        {"attackmove",      0x00e08f78u, 0x009e8820u, kSharedIntervalGetter, false},
        {"kamikaze_attack", 0x00e08f78u, 0x009e2020u, kSharedIntervalGetter, false},
        {"sub_attack",      0x00e08f78u, 0u,          kSharedIntervalGetter, false},
    };
    static const std::uint32_t kOffsets[] = {
        0x0BC8u, 0x0BD8u, 0x0BE4u, 0x0C38u, 0x0C5Cu, 0x0C64u,
        kAiOffsetAttackMove, kAiOffsetKamikaze, kAiOffsetSubAttack,
    };
    for (std::size_t i = 0; i < sizeof(kOffsets) / sizeof(kOffsets[0]); ++i) {
        if (kOffsets[i] == ai_offset) return &kStates[i];
    }
    return nullptr;
}

const char* steering_mode_name(int mode) noexcept {
    switch (mode) {
        case 0: return "rudder";
        case 1: return "heading";
        case 2: return "navigate";
        case 3: return "navigate_astern";
        default: return "?";
    }
}

const char* direction_name(int direction) noexcept {
    switch (direction) {
        case 0: return "stopped";
        case 1: return "ahead";
        case 2: return "astern";
        default: return "?";
    }
}

}  // namespace

// ---------------------------------------------------------------------------

struct GameShipAiHost::Impl {
    Impl(GameHostLog& log_in, GameUnitsHost& units_in) : log(log_in), units(units_in),
        zones(log_in, application_camera_axes_crt()) {}

    GameHostLog& log;
    GameUnitsHost& units;
    GameAvoidZoneRuntime zones;
    GameMissionLuaHost* settings_owner{}; // borrowed stored-settings projection
    // Packet cc8_ship_ai_firepower_inputs: the gunnery host, borrowed through
    // GameGunneryHost::set_ship_ai. It is the only thing in this process that
    // runs 00956C20, so it owns unit+394h, +430h, +490h and +494h.
    const GameGunneryHost* gunnery{nullptr};
    // Packet cc9_avoid_zone_escape: settings+1F4h..+208h, LandAvoidance's
    // CheckMovePosZoneTime, CheckShipPosZoneTime, CheckTravelZoneTime pairs.
    std::array<float, 6> layer_timing{};
    bool layer_timing_loaded{false};
    // Packet cc9_ship_torpedo_response: the same host, for its stream-1 draw.
    GameGunneryHost* gunnery_draws{nullptr};
    std::vector<GameGunneryHost::LiveTorpedo> live_torpedo_cache;
    unsigned long long live_torpedo_cache_step{~0ull};

    // Packet cc9_target_release: whether a unit's damage death has happened.
    // The gunnery host owns the death (its kill funnel sets `sunk`), which is
    // the moment 00926C80 queues the unit and sets +60h.
    bool unit_dead(std::size_t index) const {
        if (gunnery == nullptr) return false;
        const std::vector<GameGunneryUnitRow>& unit_rows = gunnery->unit_rows();
        return index < unit_rows.size() && unit_rows[index].sunk;
    }

    // What 00836B45's arm reads off a command target, published to the
    // commands host (which has no route to the unit table).
    class CommandTargetFacts final : public GameCommandTargetFactsSource {
    public:
        explicit CommandTargetFacts(const Impl& owner) : owner_(owner) {}
        bool command_target_facts(std::size_t index,
                                  GameCommandTargetFacts& out) const override {
            if (index >= owner_.units.count()) return false;
            out.nav_point_41 = owner_.units.unit_is_kind_of(index, 0x41);
            out.command_building_1c = owner_.units.unit_is_kind_of(index, 0x1C);
            bsp::SceneNodeFlags flags;
            out.flag_05e = owner_.units.unit_scene_node_flags(index, flags) && flags.destroyed;
            out.live_0043f080 = owner_.units.unit_alive_and_visible(index)
                && !owner_.unit_dead(index);
            out.side_0054 = owner_.units.unit_side_0054(index);
            float y = 0.0f;
            owner_.units.unit_position_00fc(index, out.position_x, y, out.position_z);
            return true;
        }
    private:
        const Impl& owner_;
    };
    CommandTargetFacts command_target_facts{*this};
    // Packet cc8_ship_ai_approach_slot_tune: [unit+73Ch], which
    // BSP_ShipAi_BrainRecordConstruct copies into brain+0AB0h at 009F11AA.
    // BSP_UnitVehicleBase_Construct fills it from ten .rdata immediates at
    // 0081F214..0081F27D, the same ten for every unit it constructs, so one
    // shared copy is what the image's per-unit blocks all hold.
    bsp::ShipAiApproachTune tune{bsp::ship_ai_approach_tune_defaults_0081f200()};
    bool avoid_all_ship_collision() const {
        bool value;
        if (!settings_owner || !settings_owner->read_avoid_all_ship_collision(value))
            throw std::logic_error("Ship avoidance settings have no established producer");
        return value;
    }
    // settings+240h..+39Fh, or the 00836EF0 defaults when ShipGlobals did not load.
    void weapon_hit_accuracy(bsp::WeaponHitAccuracyProfile (&out)[4]) const {
        if (settings_owner && settings_owner->read_weapon_hit_accuracy(out)) return;
        for (int c = 0; c < 4; ++c) bsp::apply_weapon_hit_accuracy_defaults_00836ef0(out[c]);
    }
    std::array<float, 5> avoidance_tuning() const {
        std::array<float, 5> values;
        if (!settings_owner || !settings_owner->read_avoidance_tuning(values))
            throw std::logic_error("Ship avoidance tuning has no established producer");
        return values;
    }
    bsp::ShipAiPathSearchTurnRamp path_turn_ramp{};
    bool path_turn_ramp_loaded{};
    const bsp::SessionParticipantPools* session_participants{};
    bool cruise_avoidance_inputs(std::size_t index,
        bsp::ShipAiCruiseAvoidanceInputs& output) const {
        bsp::ShipAiCruiseAvoidanceInputs inputs;
        std::int32_t role;
        // 008FBC95 attaches this same ship at bot+50. The role table belongs
        // to that entity, independently of its formation or Party number.
        if (!units.unit_current_role_slot(index, 1, role)) return false;
        inputs.group_slot_unassigned = role == 8;
        if (!inputs.group_slot_unassigned) {
            std::uint8_t ai;
            if (!session_participants || !session_participants->try_ai_held_00927f10(role, ai))
                return false;
            inputs.group_slot_ai_held = ai != 0;
            if (!inputs.group_slot_ai_held) {
                output = inputs; // Helm arm never reads unit+184 or role zero.
                return true;
            }
        }
        inputs.unit_player_controlled = units.unit_player_controlled_0184(index);
        if (!inputs.unit_player_controlled) {
            if (!units.unit_current_role_slot(index, 0, role)) return false;
            inputs.own_slot_unassigned = role == 8;
            if (!inputs.own_slot_unassigned) {
                std::uint8_t ai;
                if (!session_participants || !session_participants->try_ai_held_00927f10(role, ai))
                    return false;
                inputs.own_slot_ai_held = ai != 0;
            }
        }
        // Unread Boolean members remain irrelevant to the selected arm.
        output = inputs;
        return true;
    }
    unsigned long long hull_geometry_updates{};
    unsigned long long avoidance_queries{}, avoidance_refills{}, avoidance_clears{};
    unsigned long long avoidance_role_reads{}, avoidance_role_unavailable{};
    std::int32_t session_mode{};

    // One controller per created unit. `ai` in docs/SHIP_AI_STATES.md is the
    // whole of this record: the timers are ai+0B14h / ai+0B18h, the block is
    // ai+60h, and the order slots live on the unit rather than on the AI.
    struct Controller {
        bsp::ShipAiControllerTimers timers{};
        bsp::ShipAiControlBlock blk{};
        // Semantic records cover +00h..+4Ch; the native +50h owner pointer is
        // supplied by this controller's unit index. Slot 0 is unit+AECh,
        // slot 1 is unit+A98h. Only these records retain the order fields.
        struct OrderStorage {
            int index{};
            bsp::UnitAiOrderRecord slots[2]{};
            OrderStorage() noexcept {
                // 0081EF85/91: both record timers start at 00D7A260 = -1.
                slots[0].timer_04 = -1.0f;
                slots[1].timer_04 = -1.0f;
            }
        } order{};
        bsp::AutoTargetState target{};          // director+38h
        std::uint32_t active_state_ai_offset{0};  // ai+2264h
        std::uint32_t active_state_command{0};
        bsp::NativeHandle fire_target{0};       // director+238h as this process holds it
        std::string sample;                     // the last line log_sample emitted
        // Milestone 2o. `drive` is the labelled diagnostic stand-in of
        // --ai-drive; the two live values are the previous step's ring+148h /
        // +14Ch, so a change can be counted rather than assumed.
        bool drive{false};
        float drive_throttle{0.0f};
        float drive_rudder{0.0f};
        float live_throttle{0.0f};
        float live_rudder{0.0f};
        // The state steps that landed with packet ship_ai_state_steps. Each is
        // a different C++ object over the same native block or state object:
        // `goal` is the blk fields 009DE050 owns (+1C4h, +1C8h, +1CCh,
        // +1DCh..+1F0h, +314h, +2FDh, +2FEh), `path` the fields 009DA4E0
        // clears, `avoidance` the trio `stop` writes at blk+3ECh / +3F0h /
        // +3F4h, `stop_state` the `stop` leaf's own state+8h byte latch and
        // `selector` the attackmove leaf's +14FCh..+1508h.
        bsp::ShipAiGoalPlan goal{};
        bsp::ShipAiPathPlan path{};
        bsp::ShipAiAvoidanceRequest avoidance{};
        std::unique_ptr<bsp::ShipAiSearchStorage> avoid_search;
        bsp::ShipAiStopStepState stop_state{};
        bsp::ShipAiAttackMoveSelector selector{};
        // Packet cc8_ship_follow: the `follow` leaf's own state object at
        // brain+0B8Ch. 009F39C0 constructs it with `making_way` SET (009F3A4C)
        // and `out_of_station` CLEAR (009F3A53); 009DF2D0 writes everything else
        // before the step reads it.
        bsp::ShipAiFollowState follow_state{};
        // What the follow step published, for the report only: the distance from
        // the ship to the station 009DE050 was given, last and worst.
        float follow_station_error{0.0f};
        float follow_station_error_max{0.0f};
        unsigned long long follow_steps{0};
        bool follow_ran{false};
        // Packet cc8_ship_moveonpath: the `moveonpath` leaf's own state+8h byte
        // (009E59DE, 009E5A39, 009E5BC5) and the 1.0f 009E5ACA stores at
        // brain+308h. That field has no reader in this process, so it is carried
        // and reported rather than consumed.
        bool moveonpath_announced{false};
        float moveonpath_leg_scale_0308{0.0f};
        // Milestone 2p. `goal_vector` is the brain fields 009F1420's head owns
        // (+0B20h, +0B24h, +0B28h, +0B2Ch..+0B34h, +0B38h, +0B54h, +0B58h) and
        // `latched` the record at brain+0AF8h that 009E2FB0 writes and
        // 009DBCC0 reads. `obstacle` is the blk half the middle of 009F3F80
        // owns: the twelve sectors at blk+808h, the 65-bin profile at blk+4h,
        // the danger level and its two dwell timers, the astern latch and the
        // escape state. `path_point` is the 22h-byte record 009E3C00 fills.
        bsp::ShipAiGoalVectorState goal_vector{};
        bsp::ShipAiGoalTargetRecord latched{};
        bsp::ShipAiObstacleState obstacle{};
        bsp::ShipAiPathPointRecord path_point{};
        // The five attackmove sub-state objects 009E8450 builds, each with its
        // own storage. 007B3DD0 has none: its whole body is one RET 4.
        bsp::ShipAiAttackMoveEngageState engage{};
        bsp::ShipAiAttackMoveLeadPursuitState lead_pursuit{};
        bsp::ShipAiAttackMoveTangentState tangent{};
        float substate_ring_timer_14b4{0.0f};  // sub+14B4h, the approach warn sweep
        // 009EF230's own round robin at blk+0A18h: a 0..3 counter that refreshes
        // three of the twelve sectors per frame.
        int sector_refresh_cursor_0a18{0};
        // Two blk fields include/bsp/ship_ai_states.hpp does not declare and
        // this packet does not add to it, because that header belongs to
        // another packet. blk+3A6h is the second half of the station-keeping
        // gate at 009EDA41 and blk+39Ch the slot 009EE5A5 pins to 1.25f before
        // the path refresh and the station-keeping arm computes instead.
        bool flag_3a6{false};
        // Packet cc9_station_keeping: blk+38Ch..+3A6h as 009DA3B0 stores them,
        // and the arm's three bytes blk+388h / +389h / +38Ah.
        bsp::ShipAiStationRequest station_request{};
        bool station_aligned_388{false};
        bool station_reversing_389{false};
        bool station_close_38a{false};
        float speed_scale_39c{0.0f};
        // The two 68h-byte plan blocks the navigator owns at nav+224h and
        // nav+28Ch, and the front / back pointers at nav+2F4h / +2F8h that
        // 009ED3E0 swaps. Packet ship_ai_path_planner, on main at 878325ba.
        bsp::ShipAiPathPlanBlock plan_a{};
        bsp::ShipAiPathPlanBlock plan_b{};
        int plan_front{0};                 // 0 selects plan_a, 1 plan_b
        // Milestone 2q: nav+2FCh, the "a plan is being computed" byte the
        // 009ED4E4 arm raises at 009ED528 / 009ED63B and clears at 009ED5B6.
        bool plan_computing_2fc{false};
        // The blk half 009EE671's output block owns. Every offset is named in
        // bsp/ship_ai_navigation.hpp; none of them is in ShipAiControlBlock.
        bsp::ShipAiNavState nav{};
        // The blk half 009EEAAB's tail owns: blk+2FCh, +2FDh, +2FEh, +344h and
        // the plan's search state. Packet ship_ai_navigation_arm_tail, on main
        // at 4491d04f. blk+2FCh is the same byte 009ED3E0 writes at 009ED4DE,
        // because 009EE5B5 hands 009ED3E0 the controls step's own `this`.
        bsp::ShipAiArmTailState tail{};
        // Whether the tail ran on this tick, which decides whether blk+344h
        // carries its ceiling or the 009F4DA0 record's 1.0f.
        bool arm_tail_ran{false};
        // A deque because a push_back never moves an existing element, and the
        // plan block links the nodes by address.
        std::deque<bsp::ShipAiPathNode> plan_nodes;
        // The attackmove approach sub-state's nested ring object, sub+8h.
        // Packet ship_ai_approach_update, on main at 89d4bb77.
        bsp::ShipAiApproachState approach{};
        // Milestone 2r: what 009E4330 wrote into this block when the brain
        // record was constructed. Packet ship_ai_nav_block_ctor, on main at
        // 3b4e07a6. It is the producer of blk+3C8h, +3CCh, +3D0h, +3D4h, +3D8h
        // and +604h, the five inputs milestone 2q reported as unwritten, and of
        // blk+340h, +318h, +1B4h, +1B8h and +3E4h beside them.
        bsp::ShipAiNavBlockFields nav_block{};
        bool nav_block_built{false};
        bsp::ShipAiHullGeometry hull_geometry{};
        std::uint32_t class_reference_0570{};
        bool class_depth_loaded{false};
        // Packet cc9_avoid_zone_escape: class+560h..+570h and 009ECA20's fields
        // that no other host struct carries. 009DBE6E/009DBE76 seed +320h/+31Ch
        // with 1e9f (00D217E8); 009DBE28..009DBE56 zero +308h..+314h.
        bsp::ShipLeafTuning leaf_tuning{};
        bool leaf_tuning_loaded{false};
        float escape_x_150{0.0f};
        float escape_z_154{0.0f};
        std::uint32_t class_floor_16c{0};
        std::uint32_t requested_layer_308{0};
        std::uint32_t travel_layer_30c{0};
        std::uint32_t goal_layer_310{0};
        float last_goal_x_31c{1.0e9f};
        float last_goal_z_320{1.0e9f};
        // Milestone 2r: the blk half 009EF910 owns - the refresh timer +374h,
        // the clearance +37Ch, the outcome +370h and the hold +354h. Packet
        // cc_ai_clearance_profile. 009F4D87 calls it from inside the publish,
        // one chain slot before the drive whose danger ramp divides +37Ch by
        // unit+9CCh.
        bsp::ShipAiClearanceBlock clearance{};
        // Milestone 2r: blk+4h, blk+34Ch, blk+350h and blk+354h, the fields
        // 009E04E0 owns. Its contact-track list at blk+400h / +404h is the
        // empty one 009E4653 leaves, so every step clears the avoidance vector
        // and the 65-bin profile stays in the bypass 009E435F set.
        bsp::ShipAiThrottleProfileBlock throttle_profile{};
        bsp::ShipAiContactTrack track_scratch{};
        // Packet cc9_ship_torpedo_response: brain+0B44h/+0B48h and blk+400h/+404h.
        bsp::ShipAiTorpedoTimer torpedo_timer{};
        std::vector<bsp::ShipAiTorpedoTrack> torpedo_tracks;
        // blk+608h with the count at blk+604h. Nothing appends to it: 009F0D20's
        // only call site is 009F1A25 inside the brain pre-pass's candidate walk,
        // and the node footprint that walk builds has no producer in this
        // process. The ageing pass 009F0EA0 runs over it anyway.
        std::vector<bsp::ShipAiObstacleNode*> neighbours;
        std::vector<bsp::ShipAiObstacleNode*> neighbours_expired;
        // Milestone 2r: the 60-slot approach ring at nested+30h and the 60
        // score records the four scorers fill. 009E5530's second pass builds
        // the ring once, when the attackmove sub-state object is constructed.
        bsp::ShipAiAttackMoveRingSlot approach_ring[bsp::kAttackMoveRingSlotCount]{};
        bsp::ShipAiApproachSlotScore approach_scores[bsp::kShipAiApproachSlotCount]{};
        bool approach_ring_built{false};
        // Packet cc9_ship_formation_speed: this unit's member record +30h, 999.0f
        // (00CF4888) from the join until 009DF6AA publishes (0070D100).
        float member_speed_30{999.0f};
        // Packet cc9_ship_natives_3: the head command director+44h was set for.
        std::uint64_t begun_head_key{0};
        // Packet cc9_ship_traffic: the list at nested+14A0h, one 124h-byte record
        // per entry (009E8360). Only the fields with a reader are kept.
        struct TrafficRecord {
            std::size_t unit{0};           // +14h, the observed entity
            float timer_108{-1.0f};        // +108h, 00D7A260
            float dir_10c[3]{0.0f, 0.0f, 0.0f}; // +10Ch..+114h
            float heading_118{0.0f};       // +118h
            float distance_11c{0.0f};      // +11Ch
            float weight_120{1.0f};        // +120h, 00D7A24C
        };
        std::vector<TrafficRecord> traffic;
        // Packet cc8_ship_ai_approach_curves: the two 60-sample range curves at
        // nested+12C0h and nested+13B0h, cleared by 00954940 at 009E55C3 and
        // 009E55CE. The first is the own unit's expected damage against the
        // target at 50*(i+1) metres, the second the target's against us.
        // 009F1BC0 refills them at 009F2F11 and 009F2FB1 through 0095F080; the
        // countdowns that gate the refill are nested+1220h and nested+1224h,
        // which the projection of 009F1BC0 already counts down at 009F1C07 and
        // 009F1C13 but, its tail being unread, never re-arms.
        bsp::ShipAiApproachRangeCurve approach_curve_own{};    // nested+12C0h
        bsp::ShipAiApproachRangeCurve approach_curve_target{}; // nested+13B0h
        bool approach_curves_built{false};
    };
    std::vector<Controller> controllers;
    std::vector<GameShipAiRow> rows;
    GameShipAiSummary summary{};
    unsigned long long steps{0};
    bool logged_position{false};
    bool logged_gates{false};
    bool logged_party_list{false};
    bool logged_accept_gate{false};
    bool logged_state_steps{false};

    // 009E5530's second pass: the 60-slot approach ring, built once when the
    // attackmove sub-state object is constructed. 009E5680..009E5758 is the
    // whole of the per-slot rule and is already reconstructed.
    void ensure_approach_ring(Controller& ctl, std::size_t index) {
        if (ctl.approach_ring_built) return;
        ctl.approach_ring_built = true;
        for (int i = 0; i < bsp::kAttackMoveRingSlotCount; ++i) {
            ctl.approach_ring[i] = bsp::ship_ai_attackmove_ring_slot_009e5530(i,
                static_cast<std::uint32_t>(index) + 1u,
                static_cast<std::uint32_t>(index) + 1u);
        }
        done("ShipAiApproach::build_ring_009e5530", 0x009e5530u);
    }

    // 009E55C3 and 009E55CE, the two 00954940 clears; then the refill 009F1BC0
    // performs at 009F2F11 (the own unit, prefer_long_range = 1, re-arming
    // nested+1220h with 1.5f at 009F2F16) and 009F2FB1 (the target,
    // prefer_long_range = 0, re-arming nested+1224h with 2.0f at 009F2FB6).
    // Both sites are in the span of 009F1BC0 that packet
    // ship_ai_approach_frame_state_tail still owns, so the countdown and the
    // re-arm live here rather than in src/ship_ai_approach_update.cpp.
    //
    // The query blocks at nested+127Ch and nested+1238h have no producer in
    // this process beyond the two window constants 009F2EA1 and 009F2EB1 write,
    // so only those are set; everything else is the zero the block is born
    // with. Whatever 0095EB40 then answers, the curve rules around it are the
    // image's.
    // The refill itself needs FirepowerBinding, which is declared far below, so
    // it lives in ApproachUpdateBinding::refresh_approach_curves. This clear is
    // all Impl can do on its own.
    // nested+127Ch as 009E7FC0 hands it to 009E5DA0: the frame-state block with
    // words 5, 6 and 7 and the two bytes overwritten by the ring path.
    // Packet cc9_own_curve_target / cc9_ring_query. 009F29E0..009F2AC1, the
    // target words of nested+127Ch. Returns false when there is no unit target, and
    // leaves q with the caller's no-target constants. See docs/SHIP_AI_OWN_CURVE.md.
    bool fill_target_block_127ch(const Controller& ctl, bsp::ShipAiFirepowerQuery& q) const {
        const std::uint32_t handle = ctl.goal_vector.raw_target_0b20;
        if (handle == 0u || handle - 1u >= units.count()) return false;
        const std::size_t target = static_cast<std::size_t>(handle - 1u);
        if (!units.unit_is_kind_of(target, 5)) return false;   // 009F29EC
        int type_id = -1;
        float health = 0.0f;
        if (const GameGunneryUnitRow* row = gunnery_unit_row(target)) {
            health = row->health;
            type_id = row->type_id;
        }
        const bool ship = units.unit_is_kind_of(target, 6);     // 009F2A65
        float armour = 0.0f;
        float length = 0.0f;
        float underwater = 0.0f;
        float threshold = 100.0f;
        if (settings_owner != nullptr && type_id >= 0) {
            armour = settings_owner->read_vehicle_class_number(type_id, "Armour", 0.0f);
            length = settings_owner->read_vehicle_class_number(type_id, "Length", 0.0f);
            underwater = settings_owner->read_vehicle_class_number(type_id,
                "UnderwaterArmour", 0.0f);
            threshold = settings_owner->read_vehicle_class_number(type_id,
                "DamageThreshold", 100.0f);
        }
        q.armour = armour;
        // class vtable[24h]: 009635D0 (UnderwaterArmour) on the ship class, the base
        // 004407A0 (Armour) on every other family read (plane, runway, door,
        // structure, wreckable).
        q.armour_torpedo = ship ? underwater : armour;
        q.damage_cap = health;
        q.target_length = length;
        q.damage_threshold = ship ? threshold : 10000.0f;
        return true;
    }

    // unit+9C8h for a unit index, the full hull length 0081106E writes (model
    // bounds, or class Length without them): the units host already produces it
    // (docs/UNIT_HULL_EXTENTS.md, GameUnitsHost::unit_hull_length_09c8).
    // The nested+1238h block that describes ship `index` as a target, as
    // 009F1BC0 fills it (009F293F..009F29D1). Shared by the target curve and
    // the traffic advance 009E6240.
    bsp::ShipAiFirepowerQuery target_block_1238h(std::size_t index) const {
        bsp::ShipAiFirepowerQuery q{};
        q.window_seconds = 20.0f;
        q.ready_horizon_seconds = 30.0f;
        // +123Ch, [unit+9C8h] of THIS ship (009F294B).
        q.target_length = kUnitRadiusBound ? unit_length_9c8(index) : 0.0f;
        q.damage_cap = 0.0f;
        int type_id = -1;
        if (gunnery != nullptr) {
            const std::vector<GameGunneryUnitRow>& unit_rows = gunnery->unit_rows();
            if (index < unit_rows.size()) {
                q.damage_cap = unit_rows[index].health;
                type_id = unit_rows[index].type_id;
            }
        }
        float armour = 0.0f;              // loader default 0 (0087CCBF)
        float underwater = 0.0f;          // loader default 0 (00831D7C)
        float threshold = 100.0f;         // loader default 100 (00831DC4)
        if (settings_owner != nullptr && type_id >= 0) {
            armour = settings_owner->read_vehicle_class_number(type_id, "Armour", armour);
            underwater = settings_owner->read_vehicle_class_number(type_id,
                "UnderwaterArmour", underwater);
            threshold = settings_owner->read_vehicle_class_number(type_id,
                "DamageThreshold", threshold);
        }
        q.armour = armour;
        q.armour_torpedo = underwater;
        q.damage_threshold = threshold;
        q.unused_word9 = 5.0f;
        q.allow_machine_gun = 1;
        q.allow_artillery = 1;
        q.allow_torpedo = 1;
        q.allow_depth_charge = 1;
        q.require_bearing = 0;   // 009F29CA
        q.use_ready_rounds = 0;  // 009F29D1
        return q;
    }

    // 0082E850 on a unit's class: class+520h, times [settings+438h] unless the
    // descriptor answers vtable[18h](0Eh) (0082E866 JNZ 0082E87D), i.e. unless the
    // class is the torpedo boat's (00963E90). The host asks the instance chain.
    float class_turn_radius_0082e850(std::size_t unit) const {
        const float radius = units.unit_class_turn_radius_0520(unit);
        if (kShipTurnRadiusTorpedoBoatExempt && units.unit_is_kind_of(unit, 0x0E)) {
            return radius;
        }
        return radius * kShipTurnRadiusMultiplier438;
    }

    // Packet cc9_ship_torpedo_response ------------------------------------------
    const std::vector<GameGunneryHost::LiveTorpedo>& live_torpedoes() {
        if (live_torpedo_cache_step != steps) {
            live_torpedo_cache.clear();
            if (gunnery != nullptr) live_torpedo_cache = gunnery->live_torpedoes();
            live_torpedo_cache_step = steps;
        }
        return live_torpedo_cache;
    }
    static bsp::ShipAiTorpedoCandidate torpedo_candidate(const GameGunneryHost::LiveTorpedo& t) {
        bsp::ShipAiTorpedoCandidate c;
        c.key = t.serial;
        c.x = t.position[0];
        c.z = t.position[2];
        c.vx = t.velocity[0];
        c.vz = t.velocity[2];
        // record+46Ch: the host round keeps its launch heading, so the velocity's
        // own heading in the pose convention atan2(x, z). SUBSTITUTION, labelled.
        c.heading = static_cast<float>(std::atan2(static_cast<double>(t.velocity[0]),
                                                  static_cast<double>(t.velocity[2])));
        c.run_seconds = t.swim_seconds;
        c.water_travel_speed = t.water_travel_speed;
        c.side = t.owner_side;
        c.from_submarine = false;   // record+49Ch: no host submarine launches underwater
        c.shooter = t.owner_unit;
        return c;
    }
    const GameGunneryHost::LiveTorpedo* live_torpedo(std::uint64_t key) {
        for (const GameGunneryHost::LiveTorpedo& t : live_torpedoes()) {
            if (t.serial == key) return &t;
        }
        return nullptr;
    }
    struct TorpedoDraw final : bsp::ShipAiTorpedoDraw {
        TorpedoDraw(Impl& owner, std::size_t index) : owner_(owner), index_(index) {}
        float uniform_00bd2f10(float low, float high) override {
            if (owner_.gunnery_draws == nullptr) return low;
            return owner_.gunnery_draws->ship_ai_draw(index_, low, high);
        }
        Impl& owner_;
        std::size_t index_;
    };
    // 009F12CD..009F13F0, the brain constructor 009F1160's seven stream-1 draws, in
    // order: B3C = U(1, 2.0 [00CE3958]); B40 = -U(0, B3C); B48 = -U(0, 1);
    // B50 = -U(0, 1); B58 = -U(0, 2); B44 = U(+1ECh, +1F0h); B4C = U(+190h, +194h)
    // (ShipAvoidance.CollectTimer, shipglobals.lua line 243, { 1, 2 }). Only B44/B48
    // have a host consumer; the other five are drawn for the stream's order.
    void seed_brain_draws_009f1160() {
        for (std::size_t index = 0; index < controllers.size(); ++index) {
            Controller& ctl = controllers[index];
            if (!ctl.nav_block_built || ctl.torpedo_timer.seeded) continue;
            TorpedoDraw draw(*this, index);
            const float b3c = draw.uniform_00bd2f10(1.0f, 2.0f);        // 009F12CD
            static_cast<void>(draw.uniform_00bd2f10(0.0f, b3c));        // 009F12F1, B40
            ctl.torpedo_timer.countdown_b48 = -draw.uniform_00bd2f10(0.0f, 1.0f);   // 009F1330
            static_cast<void>(draw.uniform_00bd2f10(0.0f, 1.0f));       // 009F1360, B50
            static_cast<void>(draw.uniform_00bd2f10(0.0f, 2.0f));       // 009F138C, B58
            ctl.torpedo_timer.period_b44
                = draw.uniform_00bd2f10(kTorpedoCollectTimer1, kTorpedoCollectTimer2); // 009F13BE
            static_cast<void>(draw.uniform_00bd2f10(1.0f, 2.0f));       // 009F13F0, B4C
            ctl.torpedo_timer.seeded = true;
            ++brain_seed_draws;
        }
    }
    unsigned long long brain_seed_draws{0};
    // 009DA1D0, whole: not a torpedo boat, not deeper than -15, blk+3ECh set
    // and the director's torpedoAvoidance +240h set.
    bool torpedo_gate_009da1d0(std::size_t index, const Controller& ctl) const {
        if (units.unit_is_kind_of(index, 0x0E)) return false;          // 009DA1E7
        float x = 0.0f, y = 0.0f, z = 0.0f;
        units.unit_position_00fc(index, x, y, z);
        if (-15.0 > static_cast<double>(y)) return false;              // 009DA211, 00CE3D58
        if (!ctl.avoidance.enable_3f4) return false;                   // 009DA21D, blk+3ECh
        GameDirectorAvoidance director;
        if (!units.director_avoidance(index, director)) return false;
        return director.torpedo;                                       // 009DA231, +240h
    }
    // 009F158A..009F15C6 and 009F163F..009F1855, the torpedo half of the walk.
    void torpedo_walk_009f158a(std::size_t index, Controller& ctl, GameShipAiRow& row,
                               float elapsed) {
        TorpedoDraw draw(*this, index);
        if (!ctl.torpedo_timer.seeded) {
            // With kShipTorpedoResponseImageTerms the constructor draws were made at
            // bind time (seed_brain_draws_009f1160); this is the fallback.
            // 009F1316..009F1330 and 009F139E..009F13BE, in the constructor's
            // order. The host makes these two of the constructor's seven draws,
            // on the first re-plan rather than at construction. SUBSTITUTION.
            ctl.torpedo_timer.countdown_b48 = -draw.uniform_00bd2f10(0.0f, 1.0f);
            ctl.torpedo_timer.period_b44
                = draw.uniform_00bd2f10(kTorpedoCollectTimer1, kTorpedoCollectTimer2);
            ctl.torpedo_timer.seeded = true;
        }
        if (!bsp::ship_ai_torpedo_timer_due_009f158a(ctl.torpedo_timer, elapsed)) return;
        ++row.torpedo_scans;
        int level = units.skill_level(index);
        if (level < 0 || level > 5) level = 1;
        const bsp::ShipAiNavigatorTorpedoRow& tuning = bsp::kShipAiNavigatorTorpedoRows[
            static_cast<std::size_t>(level)];
        const float horizon = bsp::ship_ai_torpedo_horizon(ctl.torpedo_timer.period_b44, tuning);
        float x = 0.0f, y = 0.0f, z = 0.0f;
        units.unit_position_00fc(index, x, y, z);
        // 00812090: the local forward axis times 0092D730's body-axis speed.
        const float heading = units.unit_heading_radians(index);
        const float speed = units.unit_forward_speed_0092d730(index);
        const float vx = static_cast<float>(std::sin(static_cast<double>(heading))) * speed;
        const float vz = static_cast<float>(std::cos(static_cast<double>(heading))) * speed;
        const float length = units.unit_hull_length_09c8(index);
        const int side = units.unit_side_0054(index);
        for (const GameGunneryHost::LiveTorpedo& t : live_torpedoes()) {
            if (t.owner_unit == index + 1) continue;    // entity+4F8h != self
            // 009F16A0..: slot38 (+458h active) and slot2C (in the water).
            if (kShipTorpedoResponseImageTerms && !t.swimming) continue;
            const bsp::ShipAiTorpedoCandidate candidate = torpedo_candidate(t);
            if (!bsp::ship_ai_torpedo_admits(x, z, vx, vz, length, horizon, candidate)) continue;
            ++row.torpedo_admits;
            if (bsp::ship_ai_admit_torpedo_track_009f0ad0(ctl.torpedo_tracks, candidate, tuning,
                    length, side, kTorpedoCollectTimer2, draw)) {
                ++row.torpedo_tracks_built;
            }
        }
        row.torpedo_tracks_max = std::max(row.torpedo_tracks_max, ctl.torpedo_tracks.size());
    }
    // 009DE5B6..009DE5E5, then sections 3 and 4 (kShipAvoidZoneEscapeBound), then
    // 009DE8F1..009DE96C. Sections 6 and 7 stay records (docs/HEADING_TARGET_SECTIONS.md).
    void torpedo_override_009de8f1(std::size_t index, Controller& ctl, GameShipAiRow& row) {
        const float signed_speed = units.unit_forward_speed_0092d730(index);
        if (!(std::fabs(signed_speed) > 1.0f)) return;  // 009DE5DE
        if (kShipAvoidZoneEscapeBound && ctl.nav_block.flag_160) {
            // 009DE67D..009DE6D7, section 3.
            const bool reverse = (signed_speed < 0.0f)
                ? (ctl.blk.direction == bsp::ShipAiThrottleDirection::Ahead)
                : (ctl.blk.direction == bsp::ShipAiThrottleDirection::Astern);
            float heading = units.unit_heading_radians(index);
            if (ctl.blk.direction == bsp::ShipAiThrottleDirection::Astern) {
                heading = bsp::wrapped_angle_add_00438aa0(heading, 3.14159265f);   // 00D7A264
            }
            // 009DE6DB..009DE8ED, section 4.
            const bsp::ShipAiArmFinalEscapeTurn escape = bsp::ship_ai_arm_final_escape_turn_009de6db(
                heading, ctl.blk.heading_target_324, ctl.blk.distance_330,
                ctl.nav_block.value_14c, ctl.nav.look_ahead_max_3c8,
                units.unit_hull_length_09c8(index),
                std::array<float, 2>{ctl.escape_x_150, ctl.escape_z_154}, reverse);
            if (escape.raise_clearance_hold) {
                // blk+354h, projected twice in this host: the hold 009E04E0 counts
                // down and the block's clamp.
                if (3.0f > ctl.throttle_profile.hold_354) ctl.throttle_profile.hold_354 = 3.0f;
                if (3.0f > ctl.blk.clamp_354) ctl.blk.clamp_354 = 3.0f;       // 009DE763
            }
            if (escape.applied) {
                units.raise_turn_assist_load_102c(index, escape.load_request);  // 009DE853
                ctl.blk.heading_target_324 = bsp::wrapped_angle_add_00438aa0(
                    ctl.blk.heading_target_324, escape.turn);                  // 009DE8E7
                ++row.zone_escape_turns;
                row.zone_escape_max_turn = std::max(row.zone_escape_max_turn,
                                                    std::fabs(escape.turn));
                if (row.zone_escape_first_s < 0.0f) {
                    row.zone_escape_first_s = static_cast<float>(steps) * 0.05f;
                }
            }
        }
        if (!kShipTorpedoResponseBound) return;
        const bool gate = torpedo_gate_009da1d0(index, ctl);
        const bsp::ShipAiArmFinalOverride o = bsp::ship_ai_arm_final_avoidance_override_009de8f1(
            gate, ctl.throttle_profile.hold_354, ctl.throttle_profile.avoid_x_34c,
            ctl.throttle_profile.avoid_z_350, ctl.blk.direction);
        if (!o.applied) return;
        double turn = static_cast<double>(o.heading_target) - ctl.blk.heading_target_324;
        while (turn > 3.141592653589793) turn -= 6.283185307179586;
        while (turn < -3.141592653589793) turn += 6.283185307179586;
        ctl.blk.heading_target_324 = o.heading_target;
        ++row.torpedo_overrides;
        row.torpedo_override_max_turn = std::max(row.torpedo_override_max_turn,
                                                 static_cast<float>(std::fabs(turn)));
        if (row.torpedo_first_override_s < 0.0f) {
            row.torpedo_first_override_s = static_cast<float>(steps) * 0.05f;
        }
    }

    // 009F4DA0 on blk+344h / +348h (brain+34Ch / +350h). docs/SHIP_FORMATION_SPEED.md.
    void formation_throttle_ceiling_009f4da0(std::size_t index, const Controller& ctl,
                                             float& limit_344, float& limit_348,
                                             bool& escape_36c, bool& station_arm) const {
        station_arm = false;
        limit_348 = 1.0f;                                     // 009F4DBC, 00D7A24C
        if (ctl.goal_vector.speed_commanded_0b38) return;     // 009F4DC1
        const float speed_scale_af0 = 1.0f;   // brain+0AF0h: 009F144F's reset, no host writer
        if (!(speed_scale_af0 > limit_344)) limit_344 = speed_scale_af0;   // 009F4DE3
        const std::int32_t group = units.unit_formation_group_0284(index);
        if (group < 0) return;
        const std::size_t leader = units.formation_leader_0014(group);
        const std::int32_t count = units.formation_member_count(group);
        // 0070D140: min of the records' +30h, 9999999.0f seed (00CFD6F4).
        float slot_min = 9999999.0f;
        // 0070DA00 / 0070D0F0: min of the members' class+500h, the same seed.
        float ceiling = 9999999.0f;
        // 0070E3C0: max of 0082E850 over the ship members other than the leader,
        // seeded 100.0f (00CE3D08).
        float group_turn = 100.0f;
        for (std::int32_t i = 0; i < count; ++i) {
            const std::size_t member = units.formation_member_unit(group, i);
            if (member == static_cast<std::size_t>(-1) || member >= controllers.size()) continue;
            const float slot = controllers[member].member_speed_30;
            if (slot_min > slot) slot_min = slot;
            const float max_speed = units.unit_class_max_speed_0500(member);
            if (ceiling > max_speed) ceiling = max_speed;
            if (member != leader && units.unit_is_kind_of(member, 6)) {
                const float r = class_turn_radius_0082e850(member);
                if (!(group_turn > r)) group_turn = r;
            }
        }
        const float reference = units.throttle_ceiling_inputs(index).reference_speed; // 0080FC30
        const float own_turn = class_turn_radius_0082e850(index);
        if (leader == index) {
            // 009F4E13..009F4F05, 00778890's arm.
            const float g = (slot_min > ceiling) ? ceiling : slot_min;
            const float ratio = static_cast<float>(static_cast<double>(g) / reference);
            if (limit_344 > ratio) limit_344 = ratio;
            const float r = static_cast<float>(static_cast<double>(own_turn) /
                (static_cast<double>(group_turn) * 1.2000000476837158));   // 00CEC160
            if (0.75f > r) {                                    // 00CEE07C
                limit_348 = 0.75f;
            } else {
                limit_348 = (r > 1.0f) ? 1.0f : r;
            }
            return;
        }
        // 009F4F1D..009F50B5, 007788B0's arm. 00863780(1) on unit+6DCh is the
        // weapon side effect the gunnery host owns (recorded by the caller).
        // 009F4F30..009F4F88: s = clamp((ceiling + 6.70421028) / reference, 1, 1.25)
        // (00D21B38; 00415620 with 1.0f and 1.25f).
        const float s_raw = static_cast<float>(
            (static_cast<double>(ceiling) + 6.70421028137207) / reference);
        const float s = (1.0f > s_raw) ? 1.0f : ((s_raw > 1.25f) ? 1.25f : s_raw);
        if (kShipStationKeepingBound && ctl.blk.flag_3a5) {
            // 009F4F8C..009F5020, brain+3ADh set: brain+3A4h is blk+39Ch, the
            // station arm's throttle command; brain+364h is blk+35Ch and brain+374h
            // the escape byte blk+36Ch.
            const float v = ctl.speed_scale_39c;
            if (ctl.blk.direction == bsp::ShipAiThrottleDirection::Ahead) {
                if (0.0f > v) {                                   // 009F4FAD
                    limit_344 = -0.0f - v;
                    escape_36c = true;
                } else {
                    escape_36c = false;
                    limit_344 = v;
                }
            } else if (0.0f < v) {                                // 009F4FD9, JC
                limit_344 = v;
                escape_36c = true;
            } else {
                limit_344 = -0.0f - v;
                escape_36c = false;
            }
            if (limit_344 > s) limit_344 = s;                     // 009F5014
            station_arm = true;
        }
        const float a = static_cast<float>(static_cast<double>(group_turn) * 1.25 /
                                           static_cast<double>(own_turn));   // 00CF87C0
        const float b = static_cast<float>(static_cast<double>(own_turn) / 200.0); // 00CE4D70
        float m = (a > b) ? b : a;                                          // 009F5075
        if (m < 1.0f) m = 1.0f;                                             // 00415690
        if (m > 1.25f) m = 1.25f;                                           // 00CF29A8
        limit_348 = m;
    }

    float unit_length_9c8(std::size_t index) const {
        return units.unit_hull_length_09c8(index);
    }

    bsp::ShipAiRingScanClassQuery ring_query(const Controller& ctl) const {
        bsp::ShipAiFirepowerQuery q{};
        // Word 0, 009F2A04 from nested+11E0h: the planar range to the
        // attackmove destination, which this host does produce.
        q.range = ctl.approach.goal_range_11e0;
        // Words 1 to 4, 009F2A2C..009F2A54 off the target, with the no-target
        // constants of 009F2A91..009F2AC1 standing in. Same substitution as the
        // range-profile path, recorded there.
        q.target_length = 100.0f;  // 009F2AC1, 00CE3D08
        q.damage_cap = 10000.0f;   // 009F2AA9, 00CE3D64
        q.armour = 0.0f;           // 009F2A91
        q.armour_torpedo = 0.0f;   // 009F2A99
        // Word 5, 009E8153 from nested+11DCh; 009E5DB4 then subtracts the
        // slot's own angle from it inside the adapter.
        q.bearing = ctl.approach.slot_scale_11dc;
        q.window_seconds = 20.0f;        // 009E814B, 00CE3930
        q.ready_horizon_seconds = 60.0f; // 009E8161, 00CEB4B0
        q.require_bearing = 1;   // 009E8171
        q.use_ready_rounds = 1;  // 009E8178
        // 009F2AE7 and the three beside it, from [0080E160(unit)+220h..+223h].
        // No producer in this process; with all four clear nothing is rated.
        q.allow_machine_gun = 1;
        q.allow_artillery = 1;
        q.allow_torpedo = 1;
        q.allow_depth_charge = 1;
        if (kShipAiRingQueryBound) {
            // 009F2AB1's no-target divisor, then the target words if there is one.
            q.damage_threshold = 10000.0f;
            fill_target_block_127ch(ctl, q);
        }
        bsp::ShipAiRingScanClassQuery out{};
        static_assert(sizeof(q) == sizeof(out.word), "the block is 17 dwords");
        std::memcpy(out.word, &q, sizeof(q));
        return out;
    }

    const GameGunneryUnitRow* gunnery_unit_row(std::size_t index) const {
        if (gunnery == nullptr) return nullptr;
        const std::vector<GameGunneryUnitRow>& gun_rows = gunnery->unit_rows();
        if (index >= gun_rows.size()) return nullptr;
        return &gun_rows[index];
    }

    void ensure_approach_curves(Controller& ctl) {
        if (ctl.approach_curves_built) return;
        ctl.approach_curves_built = true;
        bsp::ship_ai_approach_curve_clear_00954940(ctl.approach_curve_own);
        bsp::ship_ai_approach_curve_clear_00954940(ctl.approach_curve_target);
        done("ShipAiApproach::curve_clear_00954940", 0x00954940u);
    }

    void record(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.unimplemented(method, text);
    }
    void record_slot(const char* method, const char* text) { log.unimplemented(method, text); }
    // Milestone 2o, chain slot 16: 009F4DA0's tail call at 009F50C6 into
    // 009F3F80, and 009F3F80's own tail, which is the hop into the unit's
    // order ring. Defined below the host bindings it builds.
    void drive_order_ring_009f3f80(std::size_t index, Controller& ctl, GameShipAiRow& row,
        float seconds);
    // 009DE050, the navigation goal setter every state step but `cruise`'s goes
    // through. It owns a different set of blk fields than ShipAiControlBlock
    // covers, so the three both describe (+1C4h, +1C8h, +1CCh) are mirrored
    // across the call. Defined below the host bindings it builds.
    void run_navigation_goal_009de050(Controller& ctl, GameShipAiRow& row, std::size_t index,
        float goal_x, float goal_z, bool keep_mode, bool final_leg);
    void run_hull_pre_step(Controller& ctl, std::size_t index,
        bsp::ShipAiNavBlockFields& fields, std::uint32_t raw_argument);
    void done(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.implemented(method, text);
    }
};

namespace {

// ---------------------------------------------------------------------------
// bsp::ShipAiSyncHost, the four call sites inside 009F3DD0 plus the dispatch
// 009F3D00 that its last one enters.
// ---------------------------------------------------------------------------

class SyncBinding final : public bsp::ShipAiSyncHost {
public:
    SyncBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    void refresh_director_vtable_0114() override {
        // 009F3DE2, CALL EDX = [ai+0B00h]->vtable[114h]. The unit's own getter
        // for its weapon director; this process holds one director per unit in
        // GameCommandsHost, so the dispatch itself is the record.
        owner_.record_slot("ShipAi::unit_weapon_director", "00cfc3d0+vtable114");
    }
    std::uint32_t director_current_command_0071be40() override {
        const std::uint32_t command = owner_.units.director_current_command_0071be40(index_);
        owner_.done("ShipAi::director_current_command", 0x0071be40u);
        return command;
    }
    bool unit_player_controlled_0184() override {
        // 009F3DF3, [ai+0B00h]+184h. The executable's own byte is the controlled
        // unit 004C0890 bound, which is what unit+184h names.
        owner_.done("ShipAi::unit_player_controlled", 0x009f3df3u);
        return owner_.units.unit_player_controlled_0184(index_);
    }
    std::uint32_t active_state_command_vtable24() override {
        // 009F3E12, CALL EAX = [ai+2264h]->vtable[24h]. Every one of the eight
        // getters is `MOV EAX,<singleton>; RET`, read from the image for this
        // packet, so the slot is the table in game_hosts_ship_ai.hpp.
        owner_.done("ShipAiState::command_object_vtable24", 0x009f3e12u);
        return ctl_.active_state_command;
    }
    void select_state_for_command_009f3d00(std::uint32_t command) override {
        // 009F3D00, __thiscall(ai)(command), RET 4, body 009F3D00-009F3DCF,
        // read whole for this packet. Six equality rows, one shared arm for the
        // two attack command objects and a default that falls to `stop`.
        std::uint32_t ai_offset = 0;
        for (const ShipAiCommandStateRow& row : kShipAiCommandStates) {
            if (row.command_object == command) { ai_offset = row.ai_offset; break; }
        }
        if (ai_offset == 0
            && (command == kShipAiArtilleryCommandObject
                || command == kShipAiAttackMoveCommandObject)) {
            // 009F3D73: with [ai+0B0Ch] non-null the arm asks 00779AA0 and picks
            // `kamikaze_attack` or `sub_attack`; with it null it takes
            // `attackmove`. brain+0AB4h has no producer in this process, so the
            // null path is the one that runs and 00779AA0 is not reached.
            owner_.record("ShipAiState::attack_subject_00779aa0", 0x00779aa0u);
            ai_offset = kAiOffsetAttackMove;
        }
        if (ai_offset == 0) {
            ai_offset = kShipAiDefaultStateAiOffset;  // 009F3DA0, the `stop` state
        }
        if (ai_offset == ctl_.active_state_ai_offset) return;
        if (ctl_.active_state_ai_offset != 0) {
            // 009F3DB4..009F3DB9, the outgoing state's vtable[8].
            owner_.record_slot("ShipAiState::exit_vtable08", "00d21598+vtable08");
        }
        ctl_.active_state_ai_offset = ai_offset;
        const StateDescriptor* state = state_for_ai_offset(ai_offset);
        ctl_.active_state_command = state != nullptr ? state->command_object : 0u;
        row_.state = state != nullptr ? state->name : "?";
        row_.state_step = state != nullptr ? state->step : 0u;
        row_.state_step_concrete = state != nullptr && state->step_concrete;
        row_.command_object = command;
        ++row_.state_changes;
        // 009F3DC1..009F3DC8, the incoming state's vtable[4].
        owner_.record_slot("ShipAiState::enter_vtable04", "00d21598+vtable04");
        owner_.done("ShipAiState::select_for_command", 0x009f3d00u);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::ShipAiSetterHost, the two callees of 009DFFB0 and 009E0040
// ---------------------------------------------------------------------------

class SetterBinding final : public bsp::ShipAiSetterHost {
public:
    explicit SetterBinding(GameShipAiHost::Impl& owner) : owner_(owner) {}
    void on_steering_mode_change_009da4e0() override {
        owner_.record("ShipAiControls::steering_mode_changed", 0x009da4e0u);
    }
    void after_heading_stored_00605070(float) override {
        owner_.record("ShipAiControls::after_heading_stored", 0x00605070u);
    }

private:
    GameShipAiHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// The state steps, packet ship_ai_state_steps
// ---------------------------------------------------------------------------
//
// 009DE050 is the one writer of the AI's navigation goal and every state step
// but `cruise`'s exists to produce the two floats it takes
// (docs/SHIP_AI_STATE_STEPS.md). It forces blk+1C4h to Navigate, which is what
// hands the steering to 009ED6B0's navigation arm 009EDA26..009EF228 - a span
// this executable records. So a navigation state's step running is not the same
// thing as a navigation state producing a desired throttle, and the run counts
// both separately.

class PathPlanBinding final : public bsp::ShipAiPathPlanHost {
public:
    explicit PathPlanBinding(GameShipAiHost::Impl& owner) : owner_(owner) {}
    void release_path_object_vtable_0000(std::uint32_t) override {
        // 009DA4E9 and 009DA524, (*object)->vtable[0](1). Callee body unread.
        // Both fields are null here because nothing in this process builds a
        // path object, so the arm is recorded and never taken.
        owner_.record_slot("ShipAiPath::release_object", "path+0000+vtable00");
    }
    std::uint32_t path_limit_default_00cf58ec() override {
        // [00CF58EC], read once at 009DA4FB. No producer in this process.
        owner_.record("ShipAiPath::limit_default", 0x00cf58ecu);
        return 0u;
    }

private:
    GameShipAiHost::Impl& owner_;
};

class GoalBinding final : public bsp::ShipAiGoalHost {
public:
    explicit GoalBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl)
        : owner_(owner), ctl_(ctl) {}
    void clear_path_plan_009da4e0() override {
        PathPlanBinding path(owner_);
        bsp::ship_ai_clear_path_plan_009da4e0(ctl_.path, path);
        owner_.done("ShipAiGoal::clear_path_plan", 0x009da4e0u);
    }
    float planar_length_00414c60(float dx, float dz) override {
        owner_.done("ShipAiGoal::planar_length", 0x00414c60u);
        return bsp::length_2d_00414c60(std::array<float, 2>{dx, dz});
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
};

class HeadingHoldBinding final : public bsp::ShipAiHeadingHoldHost {
public:
    HeadingHoldBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                       std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}
    float unit_heading_vtable_0050() override {
        owner_.done("ShipAiHold::unit_heading", 0x009e00b2u);
        return owner_.units.unit_heading_radians(index_);
    }
    void clear_path_plan_009da4e0() override {
        PathPlanBinding path(owner_);
        bsp::ship_ai_clear_path_plan_009da4e0(ctl_.path, path);
        owner_.done("ShipAiHold::clear_path_plan", 0x009da4e0u);
    }
    void after_heading_stored_00605070(float) override {
        owner_.record("ShipAiControls::after_heading_stored", 0x00605070u);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

class StopStepBinding final : public bsp::ShipAiStopStepHost {
public:
    StopStepBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                    GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    bool unit_pose_valid_00c8() override {
        owner_.done("ShipAiStop::unit_pose_valid", 0x009e14d7u);
        return owner_.units.unit_pose_valid_00c8(index_);
    }
    void refresh_unit_pose_00414db0() override {
        owner_.record("ShipAiStop::refresh_unit_pose", 0x00414db0u);
    }
    void unit_position_00fc(float& x, float& y, float& z) override {
        owner_.done("ShipAiStop::unit_position", 0x009e14ecu);
        owner_.units.unit_position_00fc(index_, x, y, z);
    }
    bool position_outside_world_bounds_0071c4f0(float x, float y, float z) override {
        owner_.done("ShipAiStop::outside_world_bounds", 0x0071c4f0u);
        return owner_.zones.outside({x, y, z});
    }
    void set_navigation_goal_009de050(float goal_x, float goal_z, bool keep_mode,
                                      bool final_leg) override {
        owner_.run_navigation_goal_009de050(ctl_, row_, index_, goal_x, goal_z, keep_mode,
                                            final_leg);
    }
    float unit_heading_vtable_0050() override {
        owner_.done("ShipAiStop::unit_heading", 0x009e1534u);
        return owner_.units.unit_heading_radians(index_);
    }
    void set_desired_heading_009e0040(float heading) override {
        SetterBinding setters(owner_);
        bsp::ship_ai_set_desired_heading_009e0040(ctl_.blk, heading, setters);
        owner_.done("ShipAiStop::set_desired_heading", 0x009e0040u);
    }
    float unit_body_axis_speed_0092d730() override {
        owner_.done("ShipAiStop::body_axis_speed", 0x0092d730u);
        return owner_.units.unit_forward_speed_0092d730(index_);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// Packet cc8_ship_follow: 009DF2D0 and 009E1610, the `follow` state's two halves
// ---------------------------------------------------------------------------

// 009DF2D0's nine call sites. The unit group, the member record and the leader's
// wake all live on the units host, so the station comes back through one call.
class FollowFormationPointBinding final : public bsp::ShipAiFollowFormationPointHost {
public:
    FollowFormationPointBinding(GameShipAiHost::Impl& owner, std::size_t index,
                                std::size_t leader)
        : owner_(owner), index_(index), leader_(leader) {}

    bsp::ShipAiFormationStation station_point_0070d290(std::uint32_t, float across_scale,
                                                       float along_scale) override {
        const GameUnitsHost::FormationStation station =
            owner_.units.formation_station_0070d290(index_, across_scale, along_scale);
        owner_.done("ShipAiFollow::station_point", 0x0070d290u);
        bsp::ShipAiFormationStation out{};
        out.x = station.x;
        out.z = station.z;
        out.dir_x = station.dir_x;
        out.dir_z = station.dir_z;
        out.across = station.across;
        out.along = station.along;
        out.yaw_rate = station.wake_yaw_rate;
        // 00810630 never writes the yaw rate on its along<=0 arm and 0070D290
        // then leaves out[4] stale. The projection carries validity instead of
        // reproducing the uninitialised read (docs/SHIP_AI_FORMATION.md).
        out.yaw_rate_valid = station.wake_yaw_written;
        return out;
    }
    float leader_body_axis_speed_0092d730() override {
        // 009DF359 takes the LEADER's controller, not this ship's.
        owner_.done("ShipAiFollow::leader_body_speed", 0x0092d730u);
        return owner_.units.unit_forward_speed_0092d730(leader_);
    }
    float unit_hull_radius_9c8() override {
        // unit+9C8h. docs/SHIP_AI_FOLLOW_LAND.md calls it the hull radius at this
        // site; the units host named the same field `hull_length` for its own
        // packet. One field, two names, and no conversion is applied here.
        owner_.done("ShipAiFollow::hull_radius", 0x009df32eu);
        return owner_.units.unit_hull_length_09c8(index_);
    }
    std::uint32_t zone_set_vtable_218(std::uint32_t) override {
        owner_.record("ShipAiFollow::zone_set_218", 0x009df41au);
        return 0u;
    }
    bsp::ShipAiFollowLandXZ push_out_of_zones_00417b10(std::uint32_t,
                                                       bsp::ShipAiFollowLandXZ point,
                                                       float) override {
        // The same stand-in the rest of this host uses for 00417B10 (see
        // zone_exit_point_00417b10): the body is unread, so the point comes back
        // unchanged rather than pushed by an invented margin.
        owner_.record("ShipAiFollow::push_out_of_zones", 0x00417b10u);
        return point;
    }
    float ship_class_turn_radius_0082e850() override {
        // 009DF44A takes it off brain+0AACh, the ship class descriptor. The units
        // host's own name for the same class field is `unit_class_turn_radius_0520`.
        owner_.done("ShipAiFollow::turn_radius", 0x0082e850u);
        if (kShipTurnRadiusSitesBound) return owner_.class_turn_radius_0082e850(index_);
        return owner_.units.unit_class_turn_radius_0520(index_);
    }
    float leader_command_yaw_rate_00811940(float, float) override {
        owner_.record("ShipAiFollow::leader_yaw_rate", 0x00811940u);
        return 0.0f;
    }
    float unit_reference_speed_0080fc30() override {
        owner_.done("ShipAiFollow::reference_speed", 0x0080fc30u);
        return owner_.units.throttle_ceiling_inputs(index_).reference_speed;
    }
    void publish_member_speed_0070d100(std::uint32_t, float speed) override {
        // 0070D100 stores into this unit's own member record at +30h, which
        // 0070D140 reduces for the leader's ceiling (packet cc9_ship_formation_speed).
        if (!kShipFormationSpeedBound || index_ >= owner_.controllers.size()) {
            owner_.record("ShipAiFollow::publish_member_speed", 0x0070d100u);
            return;
        }
        owner_.controllers[index_].member_speed_30 = speed;
        owner_.done("ShipAiFollow::publish_member_speed", 0x0070d100u);
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
    std::size_t leader_;
};

// 009E1610's nine call sites.
class FollowStepBinding final : public bsp::ShipAiFollowStepHost {
public:
    FollowStepBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                      GameShipAiRow& row, std::size_t index, std::size_t leader)
        : owner_(owner), ctl_(ctl), row_(row), index_(index), leader_(leader) {}

    bool leader_matches_kind_5c(int kind) override {
        // 009E1642, leader->vtable[5Ch](6) with ECX = [[unit+284h]+14h]. A ship
        // whose formation has no leader of that kind does nothing at all this
        // tick, not even a heading hold.
        owner_.done("ShipAiFollow::leader_kind", 0x009e1642u);
        return owner_.units.unit_is_kind_of(leader_, kind);
    }
    float leader_body_axis_speed_0092d730() override {
        owner_.done("ShipAiFollow::leader_body_speed", 0x0092d730u);
        return owner_.units.unit_forward_speed_0092d730(leader_);
    }
    void update_formation_point_009df2d0(bsp::ShipAiFollowState& state) override {
        FollowFormationPointBinding point(owner_, index_, leader_);
        const std::uint32_t handle = static_cast<std::uint32_t>(index_ + 1u);
        const std::int32_t group = owner_.units.unit_formation_group_0284(index_);
        // 009DF2EC returns with the state untouched when [unit+284h] is null.
        if (bsp::ship_ai_follow_update_formation_point(
                point, handle, static_cast<std::uint32_t>(group + 1), state)) {
            owner_.done("ShipAiFollow::update_formation_point", 0x009df2d0u);
        }
    }
    void refresh_world_pose_00414db0() override {
        owner_.record("ShipAiFollow::refresh_world_pose", 0x00414db0u);
    }
    bsp::ShipAiFollowLandXZ unit_position() override {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        owner_.units.unit_position_00fc(index_, x, y, z);
        owner_.done("ShipAiFollow::unit_position", 0x009e16a7u);
        return bsp::ShipAiFollowLandXZ{x, z};
    }
    float ship_class_turn_radius_0082e850() override {
        owner_.done("ShipAiFollow::turn_radius", 0x0082e850u);
        // 009E16F0 and 009E1790.
        if (kShipTurnRadiusSitesBound) return owner_.class_turn_radius_0082e850(index_);
        return owner_.units.unit_class_turn_radius_0520(index_);
    }
    float min_float_by_ref_00415510(float a, float b) override {
        owner_.done("ShipAiFollow::min_float", 0x00415510u);
        return (a < b) ? a : b;
    }
    void set_navigation_goal_009de050(const bsp::ShipAiFollowLandXZ& goal, bool keep_mode,
                                      bool final_leg) override {
        // 009E1837, 009DE050(blk, &state+14h, 0, 1): a follower re-plans against
        // its station every tick and always reports the last leg.
        station_x_ = goal.x;
        station_z_ = goal.z;
        owner_.run_navigation_goal_009de050(ctl_, row_, index_, goal.x, goal.z, keep_mode,
                                            final_leg);
    }
    void publish_station_request_009da3b0(const bsp::ShipAiStationRequest& request) override {
        // 009E18B6, the nineteen-byte copy into blk+38Ch..+3A6h. The consumer is
        // the station-keeping arm of 009ED6B0, which runs only while blk+3A5h is
        // set and blk+3A6h clear (docs/SHIP_AI_GOAL_VECTOR.md).
        if (!kShipStationKeepingBound) {
            owner_.record("ShipAiFollow::publish_station_request", 0x009da3b0u);
            return;
        }
        bsp::ship_ai_publish_station_request_009da3b0(request, ctl_.station_request);
        ctl_.speed_scale_39c = request.unused_10;   // 009DA3DA, blk+39Ch
        ctl_.blk.flag_3a5 = request.enable;         // 009DA3F7, blk+3A5h = brain+3ADh
        ctl_.flag_3a6 = request.suppress;           // 009DA400, blk+3A6h
        ++row_.station_requests;
        owner_.done("ShipAiFollow::publish_station_request", 0x009da3b0u);
    }

    float station_x() const { return station_x_; }
    float station_z() const { return station_z_; }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
    std::size_t leader_;
    float station_x_{0.0f};
    float station_z_{0.0f};
};

class MoveToPosStepBinding final : public bsp::ShipAiMoveToPosStepHost {
public:
    MoveToPosStepBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                         GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    bool unit_pose_valid_00c8() override {
        owner_.done("ShipAiMoveTo::unit_pose_valid", 0x009e579au);
        return owner_.units.unit_pose_valid_00c8(index_);
    }
    void refresh_unit_pose_00414db0() override {
        owner_.record("ShipAiMoveTo::refresh_unit_pose", 0x00414db0u);
    }
    void unit_position_xz_00fc(float& x, float& z) override {
        float y = 0.0f;
        owner_.done("ShipAiMoveTo::unit_position", 0x009e57aau);
        owner_.units.unit_position_00fc(index_, x, y, z);
    }
    void brain_goal_xz_0b2c(float& x, float& z) override {
        // 009E57D0 and 009E57B4, brain+0B2Ch and brain+0B34h. Milestone 2p
        // closed the gap milestone 2o recorded here: the brain pre-pass
        // 009F1420 writes all three components on every AI sub-tick from the
        // unit's own active command, so this is now a recovered value.
        owner_.done("ShipAiMoveTo::brain_goal_0b2c", 0x009e57d0u);
        x = ctl_.goal_vector.goal_x_0b2c;
        z = ctl_.goal_vector.goal_z_0b34;
    }
    std::uint32_t director_command_slot_0071bff0(int index) override {
        // 009E57ED, 0071BFF0(director, 0). Milestone 2n established through
        // 0071BE48 that the first command slot holds the command object itself,
        // so this process's own slot 0 answers it.
        owner_.done("ShipAiMoveTo::director_command_slot", 0x0071bff0u);
        static_cast<void>(index);
        return owner_.units.director_current_command_0071be40(index_);
    }
    bool command_on_final_leg_007adc60(std::uint32_t) override {
        // 007ADC60's body was read by packet ship_ai_state_steps: it answers
        // true when the command's waypoint list is absent or empty. No command
        // in this process carries a waypoint list - `moveto` is issued with a
        // target or a position, never a path - so that is the arm the rule
        // itself selects here.
        owner_.done("ShipAiMoveTo::command_final_leg", 0x007adc60u);
        return true;
    }
    void set_navigation_goal_009de050(float goal_x, float goal_z, bool keep_mode,
                                      bool final_leg) override {
        owner_.run_navigation_goal_009de050(ctl_, row_, index_, goal_x, goal_z, keep_mode,
                                            final_leg);
    }
    bool state_goal_reached_vtable_002c(float goal_x, float goal_z) override {
        // 009E5821, state->vtable[2Ch]. Milestone 2p left this a record;
        // docs/SHIP_AI_PATH_PLANNER.md has since read the slot whole: both
        // navigation vtables hold 009DAB10, three instructions that load
        // [state+4h]+8h into ECX and tail-jump to 009DA590, which is projected.
        const bsp::ShipAiPathPlanBlock& live
            = (ctl_.plan_front == 0) ? ctl_.plan_a : ctl_.plan_b;
        const bsp::ShipAiPathArrivalResult arrival = bsp::ship_ai_path_arrival_009da590(
            ctl_.goal.flag_2fe, goal_x, goal_z, live.latched_goal_x, live.latched_goal_z);
        owner_.done("ShipAiMoveTo::goal_reached_009da590", 0x009da590u);
        if (arrival.clears_latch) ctl_.goal.flag_2fe = false;  // 009DA5F6
        // The latch itself is raised only at 009EF034, inside the navigation
        // arm's tail. Milestone 2r fills its input: 009EEF14's release test is
        // blk+3D8h + setback < blk+330h, and blk+3D8h is now the start radius
        // 009E453F wrote, so the stop is released only while the remaining
        // path is longer than it.
        return arrival.reached;
    }
    std::uint32_t director_current_command_0054() override {
        owner_.done("ShipAiMoveTo::director_current_command", 0x009e5831u);
        return owner_.units.director_current_command_0071be40(index_);
    }
    std::uint32_t resolve_command_target_00521ea0() override {
        // 009E5847 on director+58h. Milestone 2p: 0071EB60 answers with that
        // same descriptor and 00521EA0 resolves it, so the arm no longer
        // short-circuits on the null at 009E584E.
        bsp::SceneCommandTarget descriptor{};
        int mode = 0;
        const bool real = owner_.units.active_command_descriptor_0071eb60(index_, descriptor,
            mode);
        owner_.done("ShipAiMoveTo::resolve_command_target", 0x00521ea0u);
        if (!real) return 0u;
        return owner_.units.resolve_command_target_00521ea0(descriptor);
    }
    bool target_is_kind_vtable_005c(std::uint32_t target, int kind) override {
        // 009E585F with the literal 1Ch, through the recovered chain 006FE530.
        owner_.done("ShipAiMoveTo::target_is_kind", 0x009e585fu);
        if (target == 0u) return false;
        return owner_.units.unit_is_kind_of(static_cast<std::size_t>(target - 1u), kind);
    }
    bool target_pose_valid_00c8(std::uint32_t target) override {
        owner_.done("ShipAiMoveTo::target_pose_valid", 0x009e5869u);
        if (target == 0u) return true;
        return owner_.units.unit_pose_valid_00c8(static_cast<std::size_t>(target - 1u));
    }
    void refresh_target_pose_00414db0(std::uint32_t) override {
        owner_.record("ShipAiMoveTo::refresh_target_pose", 0x00414db0u);
    }
    void target_position_xz_00fc(std::uint32_t target, float& x, float& z) override {
        float y = 0.0f;
        x = 0.0f;
        z = 0.0f;
        if (target == 0u) return;
        owner_.done("ShipAiMoveTo::target_position", 0x009e5879u);
        owner_.units.unit_position_00fc(static_cast<std::size_t>(target - 1u), x, y, z);
    }
    int target_range_07a0(std::uint32_t) override {
        owner_.record("ShipAiMoveTo::target_range_07a0", 0x009e58ceu);
        return 0;
    }
    float unit_radius_09c8() override {
        // 009E58C8 loads the controlled unit from brain+AA8h; 009E58D4
        // subtracts its full +9C8h hull extent from the target's range.
        owner_.done("ShipAiMoveTo::unit_radius_09c8", 0x009e58d4u);
        return owner_.units.unit_hull_length_09c8(index_);
    }
    float planar_length_00414c60(float dx, float dz) override {
        owner_.done("ShipAiMoveTo::planar_length", 0x009e58b9u);
        return bsp::length_2d_00414c60(std::array<float, 2>{dx, dz});
    }
    void message_text_assign_0041e870(const char*) override {
        owner_.record("ShipAiMoveTo::message_text_assign", 0x0041e870u);
    }
    void post_command_message_00984300(std::uint32_t command) override {
        // 009E595C, 00984300 on [00F8A0C4] with the `finished` literal the
        // step assigned at 009E58EF.
        const std::size_t callbacks = owner_.units.report_command_event_00984300(
            index_, command, bsp::kCommandEventStatusFinished);
        owner_.done("ShipAiMoveTo::post_command_message", 0x00984300u);
        ++row_.command_events;
        ++owner_.summary.command_events;
        owner_.summary.command_event_callbacks += callbacks;
    }
    void release_message_text_00419cc0() override {
        owner_.record("ShipAiMoveTo::release_message_text", 0x00419cc0u);
    }
    void end_command_0071e430(std::uint32_t command, int flag) override {
        // 009E5997, 0071E430(director, 00E08F68, 1). The whole round trip is in
        // the commands host: the stage ladder, the 5Dh message, this process's
        // own receipt of it and the queue advance.
        const GameCommandCompletion done
            = owner_.units.end_command_0071e430(index_, command, flag != 0);
        owner_.done("ShipAiMoveTo::end_command", 0x0071e430u);
        ++row_.command_endings;
        ++owner_.summary.command_endings;
        if (done.queue_advanced) {
            ++row_.command_completions;
            ++owner_.summary.command_completions;
        }
    }
    void hold_heading_and_stop_009e00a0() override {
        HeadingHoldBinding hold(owner_, ctl_, index_);
        bsp::ship_ai_hold_heading_and_stop_009e00a0(ctl_.blk, hold);
        owner_.done("ShipAiMoveTo::hold_heading_and_stop", 0x009e00a0u);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// Packet cc8_ship_moveonpath: 009E59C0's own host. It is MoveToPosStepBinding
// with the goal taken from the command's path cursor (the commands host owns it,
// because 0071BFF0 reads it off the director) instead of brain+0B2Ch.
class MoveOnPathStepBinding final : public bsp::ShipAiMoveOnPathStepHost {
public:
    MoveOnPathStepBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                          GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    bool announce_latch_08() override {
        owner_.done("ShipAiMoveOnPath::announce_latch", 0x009e59deu);
        return ctl_.moveonpath_announced;
    }
    void set_announce_latch_08(bool value) override {
        ctl_.moveonpath_announced = value;
    }
    std::uint32_t director_command_slot_0071bff0(int) override {
        owner_.done("ShipAiMoveOnPath::director_command_slot", 0x0071bff0u);
        return static_cast<std::uint32_t>(index_) + 1u;
    }
    bool command_slot_has_no_legs_007adc30(std::uint32_t) override {
        // 007ADC30 reads the slot's path object and its point count; the
        // projection of the test itself is src/ship_ai_goal_vector.cpp's.
        const bool empty = owner_.units.commands().path_cursor_has_no_legs_007adc30(index_);
        owner_.done("ShipAiMoveOnPath::slot_has_no_legs", 0x007adc30u);
        return bsp::ship_ai_command_slot_has_no_legs_007adc30(!empty, empty ? 0 : 1);
    }
    bool command_on_final_leg_007adc60(std::uint32_t) override {
        owner_.done("ShipAiMoveOnPath::command_final_leg", 0x007adc60u);
        return owner_.units.commands().path_cursor_on_final_leg_007adc60(index_);
    }
    void hold_heading_and_stop_009e00a0() override {
        HeadingHoldBinding hold(owner_, ctl_, index_);
        bsp::ship_ai_hold_heading_and_stop_009e00a0(ctl_.blk, hold);
        owner_.done("ShipAiMoveOnPath::hold_heading_and_stop", 0x009e00a0u);
    }
    bool path_point_vtable_0004(std::uint32_t, int leg, float& x, float& z) override {
        owner_.done("ShipAiMoveOnPath::path_point", 0x009e5a59u);
        return owner_.units.commands().path_cursor_point(index_, leg, x, z);
    }
    bool unit_pose_valid_00c8() override {
        owner_.done("ShipAiMoveOnPath::unit_pose_valid", 0x009e5a78u);
        return owner_.units.unit_pose_valid_00c8(index_);
    }
    void refresh_unit_pose_00414db0() override {
        owner_.record("ShipAiMoveOnPath::refresh_unit_pose", 0x00414db0u);
    }
    void unit_position_xz_00fc(float& x, float& z) override {
        float y = 0.0f;
        owner_.done("ShipAiMoveOnPath::unit_position", 0x009e5a87u);
        owner_.units.unit_position_00fc(index_, x, y, z);
    }
    void set_brain_leg_scale_0308(float value) override {
        // 009E5ACA, brain+308h. The field has no reader anywhere in this
        // process, so the store is carried on the controller and reported, not
        // consumed: a named hole, not a proof.
        ctl_.moveonpath_leg_scale_0308 = value;
        owner_.record("ShipAiMoveOnPath::brain_leg_scale_0308", 0x009e5acau);
    }
    void set_navigation_goal_009de050(float goal_x, float goal_z, bool keep_mode,
                                      bool final_leg) override {
        owner_.run_navigation_goal_009de050(ctl_, row_, index_, goal_x, goal_z, keep_mode,
                                            final_leg);
    }
    bool state_goal_reached_vtable_002c(float goal_x, float goal_z) override {
        const bsp::ShipAiPathPlanBlock& live
            = (ctl_.plan_front == 0) ? ctl_.plan_a : ctl_.plan_b;
        const bsp::ShipAiPathArrivalResult arrival = bsp::ship_ai_path_arrival_009da590(
            ctl_.goal.flag_2fe, goal_x, goal_z, live.latched_goal_x, live.latched_goal_z);
        owner_.done("ShipAiMoveOnPath::goal_reached_009da590", 0x009da590u);
        if (arrival.clears_latch) ctl_.goal.flag_2fe = false;
        return arrival.reached;
    }
    std::uint32_t director_current_command_0054() override {
        owner_.done("ShipAiMoveOnPath::director_current_command", 0x009e5b19u);
        return owner_.units.director_current_command_0071be40(index_);
    }
    std::uint32_t resolve_command_target_00521ea0() override {
        bsp::SceneCommandTarget descriptor{};
        int mode = 0;
        const bool real = owner_.units.active_command_descriptor_0071eb60(index_, descriptor,
            mode);
        owner_.done("ShipAiMoveOnPath::resolve_command_target", 0x00521ea0u);
        if (!real) return 0u;
        return owner_.units.resolve_command_target_00521ea0(descriptor);
    }
    bool target_is_kind_vtable_005c(std::uint32_t target, int kind) override {
        // 009E5B47 with the literal 1Ch. The `moveonpath` descriptor carries the
        // PATH entity, which is not a unit in this process, so this answers
        // false and the arm returns at 009E5B4B - which is the native's own
        // behaviour for a path target too, and is why a circling carrier never
        // reaches the target-range test.
        owner_.done("ShipAiMoveOnPath::target_is_kind", 0x009e5b47u);
        if (target == 0u) return false;
        return owner_.units.unit_is_kind_of(static_cast<std::size_t>(target - 1u), kind);
    }
    void target_position_xz_00427eb0(std::uint32_t target, float& x, float& z) override {
        float y = 0.0f;
        x = 0.0f;
        z = 0.0f;
        if (target == 0u) return;
        owner_.done("ShipAiMoveOnPath::target_position", 0x00427eb0u);
        owner_.units.unit_position_00fc(static_cast<std::size_t>(target - 1u), x, y, z);
    }
    float planar_length_00414c60(float dx, float dz) override {
        owner_.done("ShipAiMoveOnPath::planar_length", 0x009e5b91u);
        return bsp::length_2d_00414c60(std::array<float, 2>{dx, dz});
    }
    int target_range_07a0(std::uint32_t) override {
        owner_.record("ShipAiMoveOnPath::target_range_07a0", 0x009e5ba6u);
        return 0;
    }
    float unit_radius_09c8() override {
        owner_.done("ShipAiMoveOnPath::unit_radius_09c8", 0x009e5bacu);
        return owner_.units.unit_hull_length_09c8(index_);
    }
    void message_text_assign_0041e870(const char*) override {
        owner_.record("ShipAiMoveOnPath::message_text_assign", 0x0041e870u);
    }
    void post_command_message_00984300(std::uint32_t command) override {
        const std::size_t callbacks = owner_.units.report_command_event_00984300(
            index_, command, bsp::kCommandEventStatusFinished);
        owner_.done("ShipAiMoveOnPath::post_command_message", 0x00984300u);
        ++row_.command_events;
        ++owner_.summary.command_events;
        owner_.summary.command_event_callbacks += callbacks;
    }
    void release_message_text_00419cc0() override {
        owner_.record("ShipAiMoveOnPath::release_message_text", 0x00419cc0u);
    }
    void end_command_0071e430(std::uint32_t command, int flag) override {
        const GameCommandCompletion done
            = owner_.units.end_command_0071e430(index_, command, flag != 0);
        owner_.done("ShipAiMoveOnPath::end_command", 0x0071e430u);
        ++row_.command_endings;
        ++owner_.summary.command_endings;
        if (done.queue_advanced) {
            ++row_.command_completions;
            ++owner_.summary.command_completions;
        }
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// Milestone 2p: 009E85B0, the approach-to-engage gate, over the real goal.
class EngageGateBinding final : public bsp::ShipAiAttackMoveEngageGateHost {
public:
    EngageGateBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                      std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}
    std::uint32_t brain_unit_0aa8() override {
        owner_.done("ShipAiEngageGate::brain_unit", 0x009e85b9u);
        return static_cast<std::uint32_t>(index_) + 1u;
    }
    void armament_readiness_0510(float& a, float& b) override {
        // 009E85CD, [[unit+538h]+510h] and +514h. The object at unit+538h has
        // no recovered class and neither field has a producer anywhere, so the
        // pair is recorded and left at the zero a fresh object carries, which
        // is the arm that fails the gate.
        owner_.record("ShipAiEngageGate::armament_readiness", 0x009e85cdu);
        a = 0.0f;
        b = 0.0f;
    }
    bool unit_pose_valid_00c8() override {
        owner_.done("ShipAiEngageGate::unit_pose_valid", 0x009e85f5u);
        return owner_.units.unit_pose_valid_00c8(index_);
    }
    void refresh_unit_pose_00414db0() override {
        owner_.record("ShipAiEngageGate::refresh_unit_pose", 0x00414db0u);
    }
    void unit_position_xz_00fc(float& x, float& z) override {
        float y = 0.0f;
        owner_.done("ShipAiEngageGate::unit_position", 0x009e8605u);
        owner_.units.unit_position_00fc(index_, x, y, z);
    }
    void brain_destination_0b2c(float& x, float& z) override {
        // 009E8610 / 009E862C, the goal vector 009F1420 now writes.
        owner_.done("ShipAiEngageGate::brain_destination", 0x009e8610u);
        x = ctl_.goal_vector.goal_x_0b2c;
        z = ctl_.goal_vector.goal_z_0b34;
    }
    std::uint32_t avoid_zone_containing_004178f0(float, float) override {
        // 009E864C 0082ADC0 then 009E8658 004178F0. The avoid-zone singleton
        // 004218E0 hands out is not built in this process, so the list is empty
        // and the walk finds nothing, which is the arm that passes the gate.
        owner_.record("ShipAiEngageGate::avoid_zone_list", 0x0082adc0u);
        return 0u;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

class AttackMoveSelectorBinding final : public bsp::ShipAiAttackMoveSelectorHost {
public:
    AttackMoveSelectorBinding(GameShipAiHost::Impl& owner,
                              GameShipAiHost::Impl::Controller& ctl, std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    std::uint32_t brain_attack_target_0b20() override {
        // 009E8714, [brain+0B20h]. Milestone 2p: 009F1420's head writes this
        // field from the command's own resolved target, so the selector no
        // longer takes the no-target arm for a ship whose `attackmove` names an
        // entity. The automatic target selector 009F5DA0 is a different
        // producer and still does not reach 00835860.
        owner_.done("ShipAiAttack::brain_target_0b20", 0x009e8714u);
        return ctl_.goal_vector.raw_target_0b20;
    }
    bool target_is_kind_vtable_005c(std::uint32_t target, int kind) override {
        // 009E8733 and 009E8799, with the literals 8 and 1Ch, answered through
        // the recovered class chain 006FE530.
        owner_.done("ShipAiAttack::target_is_kind", 0x009e8733u);
        if (target == 0u) return false;
        return owner_.units.unit_is_kind_of(static_cast<std::size_t>(target - 1u), kind);
    }
    bool call_00852860(std::uint32_t) override {
        // 009E873B. The routine's arithmetic is projected, but its two inputs
        // at entity+1200h and +1204h have no producer in the ledger, so the
        // call is a record; it is unreachable here anyway, because no target of
        // this mission answers the kind-8 test above.
        owner_.record("ShipAiAttack::call_00852860", 0x00852860u);
        return false;
    }
    bool brain_flag_0b28() override {
        // 009E8747, brain+0B28h, the goal refresh gate 009F1420 maintains.
        owner_.done("ShipAiAttack::brain_flag_0b28", 0x009e8747u);
        return ctl_.goal_vector.target_visible_0b28;
    }
    void set_current_substate_007b6ee0(std::uint32_t member) override {
        // 007B6EE0's body was read by packet ship_ai_state_steps: it returns at
        // once when the machine already holds the member, otherwise exits the
        // old one and enters the new one. The reconstruction of the selector
        // calls this method only on a real change.
        owner_.done("ShipAiAttack::set_current_substate", 0x007b6ee0u);
        ctl_.selector.current_1508 = member;
    }
    bool call_009e85b0() override {
        EngageGateBinding gate(owner_, ctl_, index_);
        const bool open = bsp::ship_ai_attackmove_engage_gate_009e85b0(gate);
        owner_.done("ShipAiAttack::call_009e85b0", 0x009e85b0u);
        return open;
    }
    void substate_exit_vtable_0008(std::uint32_t) override {
        owner_.record_slot("ShipAiAttack::substate_exit", "00d21994+vtable08");
    }
    void substate_enter_vtable_0004(std::uint32_t) override {
        owner_.record_slot("ShipAiAttack::substate_enter", "00d21994+vtable04");
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// Milestone 2p: 009F3240, the approach sub-state step at state+8h
// ---------------------------------------------------------------------------
//
// The one sub-state the selector reaches on this mission: every `attackmove`
// here names a surface entity, which answers neither the kind-8 test at
// 009E8733 nor the kind-1Ch test at 009E8799, so 009E87B6 puts the machine on
// state+8h and the engage gate above keeps it there. The body is projected
// whole by packet cc_ai_attackmove_substates; its approach point comes from the
// nested update 009F3090, which no packet has read, so that one step is a
// record and the four fields it would fill stay at the zeroes 009E5530 seeds.

// bsp::ShipAiApproachPointHost, the call sites inside 009F1BC0's frame state.
// Packet ship_ai_approach_update landed on main at 89d4bb77 during this
// packet's turn and was merged in before validation.
// 009E46F0's path arm, defined after PathFollowerBinding below because it needs
// it. Returns false on the arms 009E46F0 answers with FLDZ at 009E485B.
bool ship_ai_arc_centre_next_point_009e46f0(GameShipAiHost::Impl& owner,
                                            GameShipAiHost::Impl::Controller& ctl,
                                            std::size_t index, float& x, float& z);

class ApproachPointBinding final : public bsp::ShipAiApproachPointHost {
public:
    ApproachPointBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                         std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    float unit_heading_vtable_0050() override {
        owner_.done("ShipAiApproachPoint::unit_heading", 0x009f1c24u);
        return owner_.units.unit_heading_radians(index_);
    }
    bool arc_centre_next_point_009e46f0(float& x, float& z) override {
        // 009F27C6, 009E46F0 with ECX = brain+8h. Packet
        // cc8_ship_ai_ring_winner: nested+11DCh had no producer here, which
        // collapsed both standoff-arc edges onto bearing 0 and made slot 0's
        // arc cost 0/0.
        owner_.done("ShipAiApproachPoint::arc_centre_009e46f0", 0x009e46f0u);
        return ship_ai_arc_centre_next_point_009e46f0(owner_, ctl_, index_, x, z);
    }
    void refresh_unit_pose_00414db0() override {
        owner_.record("ShipAiApproachPoint::refresh_unit_pose", 0x00414db0u);
    }
    bsp::ShipAiApproachPoint unit_world_position() override {
        bsp::ShipAiApproachPoint out{};
        owner_.done("ShipAiApproachPoint::unit_position", 0x009f1c45u);
        owner_.units.unit_position_00fc(index_, out.x, out.y, out.z);
        return out;
    }
    bsp::ShipAiApproachPoint brain_goal_0b2c() override {
        // 009F1C6A, brain+0B2Ch / +0B30h / +0B34h: the goal vector 009F1420
        // writes. This is the whole reason the approach point is real now.
        bsp::ShipAiApproachPoint out{};
        owner_.done("ShipAiApproachPoint::brain_goal", 0x009f1c6au);
        out.x = ctl_.goal_vector.goal_x_0b2c;
        out.y = ctl_.goal_vector.goal_y_0b30;
        out.z = ctl_.goal_vector.goal_z_0b34;
        return out;
    }
    bool unit_is_kind_vtable_005c(int kind) override {
        owner_.done("ShipAiApproachPoint::unit_is_kind", 0x009f1d0fu);
        return owner_.units.unit_is_kind_of(index_, kind);
    }
    float shipclass_radius_0500() override {
        owner_.done("ShipAiApproachPoint::shipclass_radius", 0x009f1d1eu);
        return owner_.units.unit_class_max_speed_0500(index_);
    }
    float unit_turn_radius_00811a30(float) override {
        // 009F1D3C, 00811A30 with ECX = unit and the literal 1.0: the turn
        // radius at full helm. Body unread by every packet.
        owner_.record("ShipAiApproachPoint::unit_turn_radius", 0x00811a30u);
        return 0.0f;
    }
    float random_stream1_00bd2f10(float low, float) override {
        // 009F1DB4, the retarget timer's reseed in [2, 3). 00BD2F10 was not
        // read; the low end is taken and recorded, which makes the timer
        // deterministic rather than staggered and says so.
        owner_.record("ShipAiApproachPoint::random_stream1", 0x00bd2f10u);
        return low;
    }
    int target_zone_group_vtable_002c() override {
        owner_.record_slot("ShipAiApproachPoint::target_zone_group", "00cfc3d0+vtable2c");
        return 0;
    }
    int unit_zone_group_0570() override {
        owner_.record("ShipAiApproachPoint::unit_zone_group", 0x009f1e55u);
        return 0;
    }
    float unit_avoid_radius_0082adc0() override {
        owner_.record("ShipAiApproachPoint::unit_avoid_radius", 0x0082adc0u);
        return 0.0f;
    }
    bsp::ShipAiAttackMoveXZ zone_exit_point_00417b10(const bsp::ShipAiApproachPoint& from,
                                                     float) override {
        owner_.record("ShipAiApproachPoint::zone_exit_point", 0x00417b10u);
        return bsp::ShipAiAttackMoveXZ{from.x, from.z};
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// Milestone 2r: the six arms of 009F3090, and the four slot scorers behind them
// ---------------------------------------------------------------------------
// Every method below is one native call site. The ones that need a weapon
// inventory, a zone object or a traffic list answer "there is none", which is
// this process's own state and not a substitute: no gunnery device is built,
// construct_world 004DE610 is a load record and the avoid-zone manager has no
// producer. Their addresses are recorded so a reader can tell a produced value
// from an absent one.

// 0095EB40, the expected-damage rating 009E5DA0 asks for per slot.
class FirepowerBinding final : public bsp::ShipAiFirepowerHost {
public:
    FirepowerBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}

    // [unit+494h], 0095EB62. 00956C20 writes it at 00956E59 as the maximum of
    // 00731020's answer over every category's live guns, seeded to 10.0f at
    // 00956D51 (docs/GUNNERY_TABLES.md). The gunnery host runs that rebuild.
    float unit_max_weapon_range() override {
        const GameGunneryUnitRow* row = unit_row();
        if (row == nullptr) {
            owner_.record("ShipAiFirepower::unit_max_weapon_range_0494", 0x0095eb62u);
            return 0.0f;
        }
        return row->any_weapon_max_range;
    }

    // [unit+394h + category*0Ch], 0095EBB3: the list count.
    int category_device_count(int category) override {
        const GameGunneryUnitRow* row = unit_row();
        if (row == nullptr || category < 0 || category >= bsp::kUnitGunneryCategoryCount) {
            return 0;
        }
        return row->category_guns[static_cast<std::size_t>(category)];
    }

    // [unit+430h + category*4], 0095EBC4: that category's longest weapon range,
    // floored at 10.0f by 00956D63.
    float category_max_range(int category) override {
        const GameGunneryUnitRow* row = unit_row();
        if (row == nullptr || category < 0 || category >= bsp::kUnitGunneryCategoryCount) {
            return 0.0f;
        }
        return row->category_ranges[static_cast<std::size_t>(category)];
    }

    // [unit+398h + category*0Ch], 0095EC24. The handle is the category and the
    // position in that category's list, both one based, because the host keeps
    // the list as a vector rather than the 0Ch-byte nodes 00956C20 allocates.
    bsp::NativeHandle category_list_head(int category) override {
        const std::vector<std::size_t>* list = category_list(category);
        if (list == nullptr || list->empty()) return 0;
        return make_node(category, 0);
    }
    bsp::NativeHandle list_next(bsp::NativeHandle node) override {
        const int category = node_category(node);
        const std::size_t slot = node_slot(node);
        const std::vector<std::size_t>* list = category_list(category);
        if (list == nullptr || slot + 1 >= list->size()) return 0;
        return make_node(category, slot + 1);
    }
    bsp::NativeHandle list_device(bsp::NativeHandle node) override {
        const int category = node_category(node);
        const std::size_t slot = node_slot(node);
        const std::vector<std::size_t>* list = category_list(category);
        if (list == nullptr || slot >= list->size()) return 0;
        ++owner_.summary.firepower_mounts;
        return static_cast<bsp::NativeHandle>((*list)[slot] + 1u);
    }

    // 0095EC46, [[device]+5Ch](22h). Every row the gunnery host keeps is a
    // turning gun mount, so the class test cannot fail here. LABELLED
    // SUBSTITUTION: the class hierarchy has no producer in this process.
    // BOUND, packet cc9_ship_firepower: IsKindOf(22h) is the turning-gun
    // subtree, 22h, MRTGun 23h, MSTGun 24h and MDepthChargeLauncher 27h
    // (src/unit_kind_query.cpp). BSP_DeviceClass_ResolveFromLua 00443090 picks
    // the class from the device's Lua `Type`, and in this installation every
    // device whose Function is one the rating counts (1-4, 6-9) is a
    // Rapid_Turning_Gun, Single_Turning_Gun or Depth_Charge_Launcher; the only
    // Rapid_Fixed_Slave_Gun (MRFSGun 21h) devices are PLANEGUN.
    bool device_is_turning_gun(bsp::NativeHandle device) override {
        if (!kShipFirepowerBound) {
            owner_.record("ShipAiFirepower::device_is_turning_gun_005c", 0x0095ec46u);
            return true;
        }
        owner_.done("ShipAiFirepower::device_is_turning_gun_005c", 0x0095ec46u);
        const GameGunRow* gun = gun_row(device);
        return gun != nullptr && gun->category != 0 && gun->category != 0x0A
            && gun->category != 0x0B;
    }

    // 0095EC52, 00729F10: [[device+3F0h]+720h], [device+3B8h] and [device+5Dh]
    // all clear. LABELLED SUBSTITUTION: none of the three has a producer here.
    // BOUND: the same three bytes the gunnery host's own fire gate answers
    // (FireRequestBinding: unit_fire_blocked, gun_disabled, gun_suppressed),
    // all clear there, so every mount is operational as it is for firing.
    bool device_is_operational(bsp::NativeHandle) override {
        if (!kShipFirepowerBound) {
            owner_.record("ShipAiFirepower::device_is_operational_00729f10", 0x0095ec52u);
            return true;
        }
        owner_.done("ShipAiFirepower::device_is_operational_00729f10", 0x0095ec52u);
        return bsp::ship_ai_gun_is_operational_00729f10(false, false, false);
    }

    // 0095EC84, 00727D70: the [device+448h] reload timers at [device+414h] that
    // are at or below the horizon. The gunnery host carries the timer list but
    // nothing pushes to it (GameGunRow::pending_timers), so every barrel counts
    // as ready. LABELLED SUBSTITUTION.
    // BOUND: 00727D70 counts the barrels whose +414h timer is at or below the
    // horizon. The gunnery host keeps those timers in fire.barrel_timers
    // (0072CF00 sets one per shot, the fixed step counts them down).
    int device_ready_rounds(bsp::NativeHandle device, float horizon) override {
        const GameGunRow* gun = gun_row(device);
        if (!kShipFirepowerBound) {
            owner_.record("ShipAiFirepower::device_ready_rounds_00727d70", 0x0095ec84u);
            return gun == nullptr ? 0 : gun->barrel_num;
        }
        owner_.done("ShipAiFirepower::device_ready_rounds_00727d70", 0x0095ec84u);
        if (gun == nullptr) return 0;
        return bsp::ship_ai_gun_ready_rounds_00727d70(gun->fire.barrel_timers.data(),
            gun->barrel_num, static_cast<int>(gun->fire.barrel_timers.size()), horizon);
    }

    bool device_is_destroyed(bsp::NativeHandle) override { return false; }

    // [device+448h], 0095EC98.
    int device_barrel_count(bsp::NativeHandle device) override {
        const GameGunRow* gun = gun_row(device);
        return gun == nullptr ? 0 : gun->barrel_num;
    }

    // [[device+3F4h]+80h], 0095ECAA: the Function 007327B0 wrote, which is the
    // same GunneryCategory index 00956C20 filed the gun under.
    int device_weapon_function(bsp::NativeHandle device) override {
        const GameGunRow* gun = gun_row(device);
        return gun == nullptr ? 0 : gun->category;
    }

    // [[device+354h]+74h], 0095ECB7. The host reaches the ammunition through
    // the gun row, so the device handle is its own ammunition handle.
    bsp::NativeHandle device_ammo_record(bsp::NativeHandle device) override {
        return device;
    }

    // 0095ECD4, 0095CF80: the flak alternate at ammo+48h. No producer here.
    void ammo_select_flak_alternate(bsp::NativeHandle) override {
        owner_.record("ShipAiFirepower::ammo_select_flak_alternate_0095cf80", 0x0095ecd4u);
    }

    // [ammo+34h] and its fields, 0095ECDB..0095ED4B, from the authored Bullets
    // row the gun fires. blast_damage_min (+B4h) is the one field the gunnery
    // host does not carry, so it stays zero and is recorded.
    bsp::ShipAiFirepowerProjectileClass ammo_projectile_class(
        bsp::NativeHandle ammo) override {
        bsp::ShipAiFirepowerProjectileClass out{};
        out.handle = ammo;
        const GameBulletClassRow* bullet = bullet_row(ammo);
        if (bullet == nullptr) return out;
        // 006EA910's name chain, the image's own Type to sub-type mapping.
        const bsp::ProjectileClassInfo* info
            = bsp::projectile_class_for_lua_type(bullet->type);
        if (info != nullptr) out.sub_type = info->sub_type;
        // BOUND: the class's +8h is the REFINED sub-type (006E9968 rewrites
        // Bullet and Artillery), which the gun row carries. The bullet row's
        // `type` is never filled by the gunnery host, so the lookup above found
        // nothing and every probability fell through to 006EB0C8's 1.0.
        if (kShipFirepowerBound) {
            const GameGunRow* gun = gun_row(ammo);
            if (gun != nullptr && gun->bullet_sub_type != 0) out.sub_type = gun->bullet_sub_type;
        }
        out.max_range = bullet->range;
        out.damage_min = bullet->damage_min;
        out.damage_max = bullet->damage_max;
        out.blast_damage_max = bullet->blast_damage_max;
        out.water_damage = bullet->water_damage;
        out.fire_damage = bullet->fire_damage;
        out.fire_chance = bullet->fire_chance;
        if (kShipFirepowerBound) {
            // BOUND: Blast.BlastDamageMin, classDesc+B4h, the Bullets row reads it.
            out.blast_damage_min = bullet->blast_damage_min;
            owner_.done("ShipAiFirepower::projectile_blast_damage_min_00b4", 0x0095ed15u);
        } else {
            owner_.record("ShipAiFirepower::projectile_blast_damage_min_00b4", 0x0095ed15u);
        }
        // 0095ECDB then 0095EDC9: the class just built is the one the hit
        // probability is asked about a few instructions later.
        last_projectile_ = out;
        return out;
    }

    // [ammo+2Ch], 0095EE07: the period b[6] is divided by. The gun row's reload
    // time is the closest produced value. LABELLED SUBSTITUTION.
    // BOUND: ammo+2Ch is the fire record's ReloadTime upper value; 007313E0
    // stores a scalar ReloadTime into both +28h and +2Ch, and every one of this
    // installation's 500 ReloadTime entries is a scalar, which is the gun row's.
    float ammo_cycle_period(bsp::NativeHandle ammo) override {
        if (kShipFirepowerBound) {
            owner_.done("ShipAiFirepower::ammo_cycle_period_002c", 0x0095ee07u);
        } else {
            owner_.record("ShipAiFirepower::ammo_cycle_period_002c", 0x0095ee07u);
        }
        const GameGunRow* gun = gun_row(ammo);
        return gun == nullptr ? 0.0f : gun->reload_time;
    }

    // 0095EDC9, 006EB060 whole: zero at or past the class's +60h; otherwise the
    // WeaponHitAccuracy profile for the sub-type's Function, sampled at
    // range/[p+60h]; and 1.0f for a sub-type the chain does not recognise
    // (the final `return (float10)1` of 006EB060).
    //
    // The four profiles live at settings+240h, +298h, +2F0h and +348h. Nothing
    // in this process loads ShipGlobals["WeaponHitAccuracy"], so the profile
    // used here is the image's own default (00836EF0): both reference sizes and
    // all twenty accuracy slots. Because every slot of that default is the same
    // 0.5f, the answer does not depend on the part of 008386F0 this packet did
    // not read, which is how the target length picks between the small and
    // large curves.
    float weapon_hit_probability(bsp::NativeHandle projectile_class,
                                 float range,
                                 float target_length) override {
        (void)projectile_class;
        const bsp::ShipAiFirepowerProjectileClass& p = last_projectile_;
        if (p.max_range <= range) return 0.0f; // 006EB067, FCOMI then JBE
        const int sub = p.sub_type;
        const bool recognised = (sub == 4 || sub == 5 || sub == 6 || sub == 7)
                                || sub == 0x0A || sub == 0x0B || sub == 0x13
                                || sub == 1 || sub == 2 || sub == 3 || sub == 0x10;
        if (!recognised) {
            // 006EB0C8, the fall-through return of 1.0f. The image's own answer
            // for a sub-type the switch does not name.
            if (kShipFirepowerBound) {
                owner_.done("ShipAiFirepower::hit_probability_unclassified", 0x006eb0c8u);
            } else {
                owner_.record("ShipAiFirepower::hit_probability_unclassified", 0x006eb0c8u);
            }
            return 1.0f;
        }
        const float fraction = range / p.max_range;
        if (kWeaponHitAccuracyBound) {
            // 006EB078..006EB0C2: the function code 008386F0 takes, from the same
            // switch on the class's +8h: 4-7 -> 3, 0Ah -> 7, 0Bh/13h -> 8,
            // 1/2/3/10h -> 1. `target_length` is b[1]; t = range / [p+60h].
            int function = 1;
            if (sub >= 4 && sub <= 7) function = 3;
            else if (sub == 0x0A) function = 7;
            else if (sub == 0x0B || sub == 0x13) function = 8;
            bsp::WeaponHitAccuracyProfile profiles[4];
            owner_.weapon_hit_accuracy(profiles);
            owner_.done("ShipAiFirepower::hit_accuracy_profile_008386f0", 0x008386f0u);
            return bsp::weapon_hit_accuracy_008386f0(profiles, function, target_length, fraction);
        }
        owner_.record("ShipAiFirepower::hit_accuracy_profile_008386f0", 0x008386f0u);
        int bucket = static_cast<int>(fraction * 10.0f);
        if (bucket < 0) bucket = 0;
        if (bucket > bsp::kWeaponHitAccuracyBucketCount - 1) {
            bucket = bsp::kWeaponHitAccuracyBucketCount - 1;
        }
        bsp::WeaponHitAccuracyProfile profile{};
        bsp::apply_weapon_hit_accuracy_defaults_00836ef0(profile);
        return profile.small_target_accuracy[bucket];
    }

    // 0095EDFA, 0085B7D0. The ring path sets require_bearing at 009E8171, so
    // this one IS reached from there. Two of its three arms are transcribed:
    // 0085B7E5, Function 8 answers true without any test; 0085B7F1, a range
    // past the projectile class's +60h falls through to false. The third arm,
    // Function 7's traverse filter and the gravity-arc solve behind
    // BSP_Gun_SolveGravityArc for everything else, has no producer in this
    // process, so a mount in range is allowed to bear. LABELLED SUBSTITUTION.
    bool device_can_bear(bsp::NativeHandle device, bsp::NativeHandle,
                         float bearing, float range) override {
        // 0085B7E0 reads [[device+3F4h]+80h], the weapon Function, NOT the
        // projectile sub-type; Function 8 returns 1 at 0085B7E5 with no test.
        const GameGunRow* gun = gun_row(device);
        if (gun != nullptr && gun->category == 8) return true;
        const bsp::ShipAiFirepowerProjectileClass& p = last_projectile_;
        if (range > p.max_range) return false; // 0085B7F1
        if (!kShipFirepowerBound || gun == nullptr) {
            owner_.record("ShipAiFirepower::device_can_bear_arc_0085b7d0", 0x0085b7d0u);
            return true;
        }
        // BOUND, packet cc9_ship_firepower: the rest of 0085B7D0.
        owner_.done("ShipAiFirepower::device_can_bear_arc_0085b7d0", 0x0085b7d0u);
        const bsp::GunPlatformArcs arcs{gun->arcs.data(), gun->arcs.size()};
        bsp::ShipAiGunBearInputs in;
        in.function = gun->category;
        in.bearing = bearing;
        in.muzzle_speed = gun->muzzle_speed;   // [projectile+50h], V0
        in.range = range;
        return bsp::ship_ai_gun_can_bear_0085b7d0(in, arcs);
    }

    // 0095EEAD and 0095EED8, 00424C40 then [settings+3B0h] and [settings+3ACh].
    // Nothing in this process loads the gameplay settings object, so these are
    // the authored defaults docs/GAMEPLAY_SETTINGS.md records. LABELLED.
    bsp::ShipAiFirepowerTickDamage gameplay_tick_damage() override {
        // BOUND: this installation's shipglobals.lua:77-78 authors 100 and 40,
        // and the FailureDebug override at :746 is off (scriptoptions.lua:1).
        if (kShipFirepowerBound) {
            owner_.done("ShipAiFirepower::gameplay_tick_damage_00424c40", 0x0095eeadu);
        } else {
            owner_.record("ShipAiFirepower::gameplay_tick_damage_00424c40", 0x0095eeadu);
        }
        bsp::ShipAiFirepowerTickDamage out{};
        out.water_tick_damage = 100.0f; // settings+3B0h WaterTickDamage
        out.fire_tick_damage = 40.0f;   // settings+3ACh FireTickDamage
        return out;
    }

private:
    static bsp::NativeHandle make_node(int category, std::size_t slot) {
        return static_cast<bsp::NativeHandle>(
            (static_cast<std::uint32_t>(category + 1) << 16)
            | static_cast<std::uint32_t>(slot + 1));
    }
    static int node_category(bsp::NativeHandle node) {
        return static_cast<int>((static_cast<std::uint32_t>(node) >> 16) & 0xFFFFu) - 1;
    }
    static std::size_t node_slot(bsp::NativeHandle node) {
        return static_cast<std::size_t>(static_cast<std::uint32_t>(node) & 0xFFFFu) - 1u;
    }
    const std::vector<std::size_t>* category_list(int category) const {
        if (owner_.gunnery == nullptr) return nullptr;
        return owner_.gunnery->unit_category_guns(index_, category);
    }
    const GameGunneryUnitRow* unit_row() const {
        if (owner_.gunnery == nullptr) return nullptr;
        const std::vector<GameGunneryUnitRow>& rows = owner_.gunnery->unit_rows();
        if (index_ >= rows.size()) return nullptr;
        return &rows[index_];
    }
    const GameGunRow* gun_row(bsp::NativeHandle handle) const {
        if (owner_.gunnery == nullptr || handle == 0) return nullptr;
        const std::vector<GameGunRow>& rows = owner_.gunnery->guns();
        const std::size_t i = static_cast<std::size_t>(handle) - 1u;
        if (i >= rows.size()) return nullptr;
        return &rows[i];
    }
    const GameBulletClassRow* bullet_row(bsp::NativeHandle handle) const {
        const GameGunRow* gun = gun_row(handle);
        if (gun == nullptr || owner_.gunnery == nullptr) return nullptr;
        return owner_.gunnery->bullet_class_row(gun->bullet_class);
    }

    GameShipAiHost::Impl& owner_;
    std::size_t index_;
    bsp::ShipAiFirepowerProjectileClass last_projectile_{};
};

// 009E5DA0's own host: the one call it makes, into 0095EB40.
class RingScanClassScoreBinding final : public bsp::ShipAiRingScanClassScoreHost {
public:
    RingScanClassScoreBinding(GameShipAiHost::Impl& owner, GameShipAiRow& row,
                              std::size_t index)
        : owner_(owner), row_(row), index_(index) {}
    float rate_bearing_0095eb40(const bsp::ShipAiRingScanClassQuery& query) override {
        bsp::ShipAiFirepowerQuery q{};
        static_assert(sizeof(q) == sizeof(query.word), "the block is 17 dwords");
        std::memcpy(&q, query.word, sizeof(q));
        FirepowerBinding firepower(owner_, index_);
        const bsp::ShipAiFirepowerResult result
            = bsp::ship_ai_firepower_rating_0095eb40(q, firepower);
        owner_.done("ShipAiApproach::rate_bearing_0095eb40", 0x009e5dc4u);
        ++row_.firepower_ratings;
        ++owner_.summary.firepower_ratings;
        return result.total;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// 009E6640's own host, the obstacle probe behind the accept/reject test.
class RingScanProbeBinding final : public bsp::ShipAiRingScanHost {
public:
    RingScanProbeBinding(GameShipAiHost::Impl& owner,
                         GameShipAiHost::Impl::Controller& ctl, std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}
    float wrap_phase_00605070(float value) override {
        return bsp::wrapped_angle_add_00438aa0(value, 0.0f);
    }
    std::uint32_t probe_space_vtable_0218() override {
        owner_.record("ShipAiRingScan::probe_space_vtable_0218", 0x009e66ebu);
        return 0u;
    }
    bool unit_pose_fresh_00c8() override {
        return owner_.units.unit_pose_valid_00c8(index_);
    }
    void refresh_unit_pose_00414db0() override {
        owner_.record("ShipAiRingScan::refresh_unit_pose", 0x00414db0u);
    }
    bsp::ShipAiAttackMoveXZ unit_world_xz() override {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        owner_.units.unit_position_00fc(index_, x, y, z);
        owner_.done("ShipAiRingScan::unit_world_xz", 0x009e6710u);
        bsp::ShipAiAttackMoveXZ out{};
        out.x = x;
        out.z = z;
        return out;
    }
    bsp::ShipAiAttackMoveXZ probe_origin_00417b10(std::uint32_t,
                                                  const bsp::ShipAiAttackMoveXZ& point,
                                                  float, int) override {
        // 009E673E, 00417B10 on the probe space. No space exists here, so the
        // start point is the query point itself.
        owner_.record("ShipAiRingScan::probe_origin_00417b10", 0x00417b10u);
        return point;
    }
    bool probe_hit_0041b4e0(std::uint32_t, const bsp::ShipAiAttackMoveXZ&,
                            const bsp::ShipAiAttackMoveXZ&,
                            bsp::ShipAiAttackMoveXZ&) override {
        owner_.record("ShipAiRingScan::probe_hit_0041b4e0", 0x0041b4e0u);
        return false;
    }
    float planar_length_00414c60(const bsp::ShipAiAttackMoveXZ& delta) override {
        return bsp::length_2d_00414c60(std::array<float, 2>{delta.x, delta.z});
    }
    float tune_reject_penalty_04() override {
        return owner_.tune.slot_weight; // 009E784B, tune+4h
    }
    void rebuild_unit_world_matrix() override {
        owner_.record("ShipAiRingScan::rebuild_unit_world_matrix", 0x009e7cadu);
    }
    bsp::ShipAiAttackMoveXZ brain_goal_0b2c() override {
        bsp::ShipAiAttackMoveXZ out{};
        out.x = ctl_.goal_vector.goal_x_0b2c;
        out.z = ctl_.goal_vector.goal_z_0b34;
        owner_.done("ShipAiRingScan::brain_goal_0b2c", 0x009e7d7au);
        return out;
    }
    float unit_cruise_speed_0490() override {
        owner_.record("ShipAiRingScan::unit_cruise_speed_0490", 0x009e7de7u);
        return 0.0f;
    }
    void commit_bearing_009e5e90(float, float) override {}

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// 009E7FC0.
class ScoreResetBinding final : public bsp::ShipAiApproachScoreResetHost {
public:
    ScoreResetBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                      GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}
    void decay_slot_009e6400(int slot, float radius) override {
        // 009E8080, the bearing-decay scorer. Packet cc_ai_ring_scan read it
        // whole; the bearing it decays against is nested+1290h.
        if (slot < 0 || slot >= bsp::kShipAiApproachSlotCount) return;
        bsp::ship_ai_ring_scan_decay_slot_009e6400(ctl_.approach_scores[slot], radius,
            ctl_.approach_ring[slot].angle_08);
        owner_.done("ShipAiApproach::decay_slot_009e6400", 0x009e6400u);
    }
    void target_kind_probe_vtable_005c(int) override {
        owner_.record("ShipAiApproach::target_kind_probe_005c", 0x009e80abu);
    }
    std::uint32_t brain_target_0b20() override { return ctl_.goal_vector.raw_target_0b20; }
    bool brain_flag_0b28() override {
        // 009E80B0, brain+0B28h. The same field the attack binding above reads
        // from the produced goal vector; returning false here contradicted it.
        const bool flag = ctl_.goal_vector.target_visible_0b28;
        row_.gate_flag_0b28 = flag;
        if (!flag) ++row_.gate_flag_stops;
        return flag;
    }
    float nested_reference_127c() override {
        // 009E80BD reads nested+127Ch, which is word 0 of the firepower query
        // block: the planar range to the attackmove destination that 009F2A04
        // copies from nested+11E0h. The earlier binding answered with
        // nested+1290h (avoid_radius_1290), which is word 5 of the same block.
        row_.gate_reference_127c = ctl_.approach.goal_range_11e0;
        return ctl_.approach.goal_range_11e0;
    }
    float unit_lookahead_0494() override {
        // 009E80CD reads [unit+494h], the same max weapon range 0095EB62 gates
        // on and 00956C20 writes at 00956E59. The gunnery host produces it.
        const GameGunneryUnitRow* row = owner_.gunnery_unit_row(index_);
        if (row == nullptr) {
            owner_.record("ShipAiApproach::unit_lookahead_0494", 0x009e80cdu);
            return 0.0f;
        }
        row_.gate_lookahead_0494 = row->any_weapon_max_range;
        if (row_.gate_reference_127c > row_.gate_lookahead_0494) {
            ++row_.gate_range_stops;
        }
        return row->any_weapon_max_range;
    }
    bool zone_allows_target_00864fd0(std::uint32_t) override {
        // 009E8116. 00864FD0 is a thunk onto BSP_UnitGunneryVisibility_Test
        // (00864D90), the gunnery pass's own cached line-of-sight test, and the
        // gunnery host already answers that same routine with true for the same
        // stated reason: 00864680, the sight test itself, is unread, and over
        // open water with no terrain in this process the answer is yes.
        // Returning false here contradicted that sibling binding and was what
        // made 009E7FC0 return at 009E8135 before it scored a single slot.
        // LABELLED SUBSTITUTION, the same one src/game_hosts_gunnery.cpp makes.
        owner_.record("ShipAiApproach::zone_allows_target_00864680", 0x00864680u);
        return true;
    }
    bsp::ShipAiApproachPoint probe_point_009e6120() override {
        owner_.record("ShipAiApproach::probe_point_009e6120", 0x009e6120u);
        return bsp::ShipAiApproachPoint{};
    }
    bool zone_allows_point_00864ba0(const bsp::ShipAiApproachPoint&) override {
        owner_.record("ShipAiApproach::zone_allows_point_00864ba0", 0x00864ba0u);
        return false;
    }
    float score_slot_009e5da0(int slot) override {
        // 009E81A7, the ship-class rating. Packet cc_ai_ring_scan read the
        // adapter whole and packet cc_ai_bearing_rating the 0095EB40 behind it.
        if (slot < 0 || slot >= bsp::kShipAiApproachSlotCount) return 0.0f;
        // Packet cc8_ship_ai_approach_slot_scorers: 009E8197..009E81A2 copies
        // the seventeen dwords at nested+127Ch into the adapter's own block,
        // and 009E813D..009E8178 then overwrites words 5, 6 and 7 and the two
        // bytes. An all-zero block here was the reason every slot tied: with
        // the four allow bytes clear, 0095EBD7 skips every category, and with
        // damage_cap zero the output cap is zero as well.
        bsp::ShipAiRingScanClassQuery query{};
        query = owner_.ring_query(ctl_);
        RingScanClassScoreBinding score(owner_, row_, index_);
        bsp::ship_ai_ring_scan_class_score_009e5da0(ctl_.approach_ring[slot],
            ctl_.approach_scores[slot], query, score);
        owner_.done("ShipAiApproach::score_slot_009e5da0", 0x009e5da0u);
        return ctl_.approach_scores[slot].raw_18;
    }
    float tune_scale_00() override {
        return owner_.tune.slot_score_scale; // 009E81FA, tune+0h
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// 009E74D0.
class EvadeBinding final : public bsp::ShipAiApproachEvadeHost {
public:
    EvadeBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}
    float unit_evade_flag_1128() override {
        owner_.record("ShipAiApproach::unit_evade_flag_1128", 0x009e751fu);
        return 0.0f;
    }
    float tune_bearing_10() override {
        return owner_.tune.evade_bearing; // 009E75F2, tune+10h
    }
    float tune_evade_14() override { return owner_.tune.evade_weight; }   // tune+14h
    float tune_evade_span_18() override { return owner_.tune.evade_span; } // tune+18h

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

// 009E6E80.
class StandoffBinding final : public bsp::ShipAiApproachStandoffHost {
public:
    StandoffBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                    std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}
    void construct_scratch_00954940() override {
        owner_.record("ShipAiApproach::scratch_00954940", 0x00954940u);
    }
    float tune_range_override_1c() override {
        // 009E6EC5. The installed -1.0f is below the 0.0f at 00D7A218, so the
        // override arm at 009E6ECA is not taken and the curve scan runs. The
        // host guessed this value before the block was read; it is now sourced.
        return owner_.tune.range_override; // tune+1Ch
    }
    bool target_is_kind_vtable_005c(int) override {
        owner_.record("ShipAiApproach::target_kind_005c", 0x009e6efcu);
        return false;
    }
    bool shipclass_allows_close_00827f70() override {
        owner_.record("ShipAiApproach::shipclass_allows_close_00827f70", 0x00827f70u);
        return false;
    }
    std::int32_t target_radius_07c4() override {
        owner_.record("ShipAiApproach::target_radius_07c4", 0x009e6f4eu);
        return 0;
    }
    std::int32_t target_gun_range_07a0() override {
        owner_.record("ShipAiApproach::target_gun_range_07a0", 0x009e706fu);
        return 0;
    }
    bool unit_is_group_leader_00778890() override {
        owner_.record("ShipAiApproach::unit_is_group_leader_00778890", 0x00778890u);
        return false;
    }
    float unit_gun_reference_09c8() override {
        return owner_.units.unit_hull_length_09c8(index_);
    }
    float unit_cruise_speed_0490() override {
        owner_.record("ShipAiApproach::unit_cruise_speed_0490", 0x009e7140u);
        return 0.0f;
    }
    float random_stream1_00bd2f10(float low, float) override {
        owner_.record("ShipAiApproach::random_stream1_00bd2f10", 0x00bd2f10u);
        return low;
    }
    // Packet cc8_ship_ai_approach_curves. 009E71A5 puts nested+13B0h (the
    // target's curve) in EBX and 009E71B9 puts nested+12C0h (the own curve) in
    // EBP; 009E71AE calls 00952530 on EBX, 009E71EF calls 009523C0 on EBP,
    // 009E721A samples EBP and 009E722D samples EBX.
    float curve_base_00952530() override {
        return bsp::ship_ai_approach_curve_effective_range_00952530(
            ctl_.approach_curve_target);
    }
    float curve_reference_009523c0() override {
        return bsp::ship_ai_approach_curve_peak_009523c0(ctl_.approach_curve_own);
    }
    float curve_primary_00955a40(float x) override {
        return bsp::ship_ai_approach_curve_sample_00955a40(ctl_.approach_curve_own, x);
    }
    float curve_secondary_00955a40(float x) override {
        return bsp::ship_ai_approach_curve_sample_00955a40(ctl_.approach_curve_target, x);
    }
    float nested_scan_scale_1284() override {
        // nested+1284h is word 2 of the own unit's firepower query block at
        // nested+127Ch: the per-shot damage cap. 009F2A44 loads it from
        // [target+370h] and 009F2AA9 stores the 10000.0f at 00CE3D64 when there
        // is no target. Packet cc9_ship_natives_2: the same fill the own curve and
        // the ring use (fill_target_block_127ch), the gunnery host's health.
        if (!kApproachScanScaleBound) {
            owner_.record("ShipAiApproach::scan_scale_1284_target_0370", 0x009f2a44u);
            return 10000.0f;
        }
        bsp::ShipAiFirepowerQuery q{};
        q.damage_cap = 10000.0f;          // 009F2AA9, 00CE3D64
        owner_.fill_target_block_127ch(ctl_, q);
        owner_.done("ShipAiApproach::scan_scale_1284_target_0370", 0x009f2a44u);
        return q.damage_cap;
    }
    float unit_turn_radius_00811a30(float rudder) override {
        return owner_.units.unit_class_turn_circle_radius_0082e960(index_, rudder);
    }
    int unit_clearance_count_0080df40() override {
        owner_.record("ShipAiApproach::unit_clearance_count_0080df40", 0x0080df40u);
        return 0;
    }
    void score_slot_009e6870(int slot, float side_weight, float span_weight,
                             float slot_scale, float tune_04) override {
        // 009E74B7, the standoff-arc score. Packet cc_ai_ring_scan read it whole.
        if (slot < 0 || slot >= bsp::kShipAiApproachSlotCount) return;
        bsp::ship_ai_ring_scan_arc_slot_009e6870(ctl_.approach_ring[slot],
            ctl_.approach_scores[slot], slot_scale, side_weight, span_weight, tune_04);
        owner_.done("ShipAiApproach::score_slot_009e6870", 0x009e6870u);
    }
    float tune_slot_04() override {
        return owner_.tune.slot_weight; // 009E7489, tune+4h
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// 009E9190.
class AvoidBinding final : public bsp::ShipAiApproachAvoidHost {
public:
    AvoidBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                 GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}
    float random_stream1_00bd2f10(float low, float) override {
        owner_.record("ShipAiApproach::avoid_random_00bd2f10", 0x00bd2f10u);
        return low;
    }
    int candidate_count_008053c0() override {
        // 009E9220, the side's contact list [008053C0(unit+54h)+0DE8h]. There is
        // no recon slot object in this process; the stand-in is the one the
        // gunnery host's recon sweep uses (include/bsp/game_hosts_gunnery.hpp):
        // enemy side, rule (b)'s four bytes, not dead, a ship or plane base, and
        // a published recon level other than none. LABELLED SUBSTITUTION.
        contacts_.clear();
        if (!kShipAiTrafficBound || owner_.gunnery == nullptr) {
            owner_.record("ShipAiApproach::candidate_count_008053c0", 0x008053c0u);
            return 0;
        }
        const std::vector<GameGunneryUnitRow>& rows = owner_.gunnery->unit_rows();
        const bsp::ReconSensorPassState& recon = owner_.gunnery->recon_sensor_pass_state();
        const int own_side = owner_.units.unit_side_0054(index_);
        const std::size_t count = owner_.units.count();
        for (std::size_t i = 0; i < count; ++i) {
            if (i == index_ || owner_.units.unit_side_0054(i) == own_side) continue;
            if (!owner_.units.unit_alive_and_visible(i)) continue;
            if (i < rows.size() && rows[i].sunk) continue;
            if (!owner_.units.unit_is_kind_of(i, bsp::kUnitGunneryKindShipBase)
                && !owner_.units.unit_is_kind_of(i, bsp::kUnitGunneryKindPlaneBase)) {
                continue;
            }
            if (recon.level(own_side, i) == bsp::ReconDetectionLevel::none) continue;
            contacts_.push_back(i);
        }
        owner_.done("ShipAiApproach::candidate_count_008053c0", 0x008053c0u);
        return static_cast<int>(contacts_.size());
    }
    std::uint32_t candidate_at(int i) override {
        if (i < 0 || static_cast<std::size_t>(i) >= contacts_.size()) return 0u;
        return static_cast<std::uint32_t>(contacts_[static_cast<std::size_t>(i)] + 1u);
    }
    bool candidate_is_kind_vtable_005c(std::uint32_t handle) override {
        // 009E9253, candidate->vtable[5Ch](5), the same test 009F29EC applies to
        // the target.
        return handle != 0u && handle - 1u < owner_.units.count()
            && owner_.units.unit_is_kind_of(handle - 1u, 5);
    }
    std::uint32_t brain_target_0b20() override { return ctl_.goal_vector.raw_target_0b20; }
    void refresh_pose_00414db0(std::uint32_t) override {}
    bsp::ShipAiApproachPoint entity_world_position(std::uint32_t handle) override {
        bsp::ShipAiApproachPoint out{};
        if (handle == 0u || handle - 1u >= owner_.units.count()) return out;
        owner_.units.unit_position_00fc(handle - 1u, out.x, out.y, out.z);
        return out;
    }
    // [entity+494h] (009E92CF): the entity's longest weapon range, which
    // 00956C20 writes (docs/GUNNERY_TABLES.md), not a speed.
    float entity_speed_0494(std::uint32_t handle) override {
        return handle == 0u ? 0.0f : weapon_range_0494(handle - 1u);
    }
    bsp::ShipAiApproachPoint unit_world_position() override {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        owner_.units.unit_position_00fc(index_, x, y, z);
        bsp::ShipAiApproachPoint out{};
        out.x = x;
        out.y = y;
        out.z = z;
        owner_.done("ShipAiApproach::avoid_unit_position", 0x009e93f3u);
        return out;
    }
    void insert_traffic_record(std::uint32_t handle) override {
        if (handle == 0u || handle - 1u >= owner_.units.count()) return;
        GameShipAiHost::Impl::Controller::TrafficRecord rec{};
        rec.unit = handle - 1u;
        ctl_.traffic.push_back(rec); // 009E8360, then the splice at the end
        owner_.done("ShipAiApproach::insert_traffic_record_009e8360", 0x009e8360u);
        ++row_.traffic_inserts;
        if (row_.traffic_first_entity.empty() && owner_.gunnery != nullptr
            && rec.unit < owner_.gunnery->unit_rows().size()) {
            row_.traffic_first_entity = owner_.gunnery->unit_rows()[rec.unit].name;
        }
        row_.traffic_max_records = std::max(row_.traffic_max_records,
                                            static_cast<int>(ctl_.traffic.size()));
    }
    int traffic_record_count() override { return static_cast<int>(ctl_.traffic.size()); }
    std::uint32_t traffic_record_entity(int r) override {
        return static_cast<std::uint32_t>(ctl_.traffic[static_cast<std::size_t>(r)].unit + 1u);
    }
    bool traffic_record_active_009e6170(int r, const bsp::ShipAiApproachPoint& unit_pos,
                                        float range) override {
        const std::size_t e = ctl_.traffic[static_cast<std::size_t>(r)].unit;
        if (!entity_live(e)) return false;
        float x = 0.0f, y = 0.0f, z = 0.0f;
        owner_.units.unit_position_00fc(e, x, y, z);
        // 009E6170: float differences, the x87 sum against (+494h + range)^2.
        const float dx = unit_pos.x - x;
        const float dz = unit_pos.z - z;
        const float reach = weapon_range_0494(e) + range;
        return static_cast<double>(dx) * dx + 0.0 * 0.0 + static_cast<double>(dz) * dz
            < static_cast<double>(reach) * reach;
    }
    void erase_traffic_record(int r) override {
        ctl_.traffic.erase(ctl_.traffic.begin() + r);
        ++row_.traffic_erases;
    }
    void advance_traffic_record_009e6240(int r, float seconds,
                                         const bsp::ShipAiApproachPoint& unit_pos) override {
        GameShipAiHost::Impl::Controller::TrafficRecord& rec =
            ctl_.traffic[static_cast<std::size_t>(r)];
        if (!entity_live(rec.unit)) return;
        rec.timer_108 = rec.timer_108 - seconds;
        if (!(rec.timer_108 < 0.0f)) return;
        float x = 0.0f, y = 0.0f, z = 0.0f;
        owner_.units.unit_position_00fc(rec.unit, x, y, z);
        rec.dir_10c[0] = unit_pos.x - x;
        rec.dir_10c[1] = 0.0f;           // the y difference is overwritten with 0
        rec.dir_10c[2] = unit_pos.z - z;
        const float length = bsp::force_event_vector_length_0042b2f0(
            std::array<float, 3>{rec.dir_10c[0], rec.dir_10c[1], rec.dir_10c[2]});
        rec.distance_11c = length;
        rec.dir_10c[0] = rec.dir_10c[0] / length;
        rec.dir_10c[1] = rec.dir_10c[1] / length;
        rec.dir_10c[2] = rec.dir_10c[2] / length;
        rec.heading_118 = bsp::ship_ai_approach_heading_from_delta(rec.dir_10c[0],
                                                                   rec.dir_10c[2]);
        // nested+1238h, this ship as the target, with 009E9190's +41h and word
        // 7 (009E942B/009E9432) and 009E6240's words 5 and 0 and +40h.
        bsp::ShipAiFirepowerQuery q = owner_.target_block_1238h(index_);
        q.use_ready_rounds = 1;
        q.ready_horizon_seconds = bsp::kApproachAvoidReadyHorizon;
        q.bearing = rec.heading_118;   // 009E634C
        q.require_bearing = 1;         // 009E634F
        q.range = rec.distance_11c;    // 009E6359
        FirepowerBinding firepower(owner_, rec.unit); // ECX = [record+14h]
        const bsp::ShipAiFirepowerResult result =
            bsp::ship_ai_firepower_rating_0095eb40(q, firepower);
        float weight = result.total;
        if (weight < 1.0f) weight = 1.0f; // 009E6367..009E639B, 00D7A24C
        rec.weight_120 = weight;
        // 009E63A6, 00BD2F10(1, 2.0, 3.0): the approach's stream-1 stand-in
        // returns the low bound, as for the pass timer. LABELLED.
        owner_.record("ShipAiApproach::traffic_random_00bd2f10", 0x009e63a6u);
        rec.timer_108 = 2.0f;
        owner_.done("ShipAiApproach::advance_traffic_record_009e6240", 0x009e6240u);
        ++row_.traffic_refreshes;
        row_.traffic_weight_max = std::max(row_.traffic_weight_max, weight);
    }
    float traffic_record_weight_0120(int r) override {
        return ctl_.traffic[static_cast<std::size_t>(r)].weight_120;
    }
    bsp::ShipAiApproachPoint traffic_record_direction_010c(int r) override {
        const GameShipAiHost::Impl::Controller::TrafficRecord& rec =
            ctl_.traffic[static_cast<std::size_t>(r)];
        bsp::ShipAiApproachPoint out{};
        out.x = rec.dir_10c[0];
        out.y = rec.dir_10c[1];
        out.z = rec.dir_10c[2];
        return out;
    }
    float vector_length_0042b2f0(const bsp::ShipAiApproachPoint& v) override {
        return bsp::length_2d_00414c60(std::array<float, 2>{v.x, v.z});
    }
    float tune_avoid_strength_08() override {
        return owner_.tune.avoid_strength; // 009E964E, tune+8h
    }
    float tune_avoid_span_0c() override { return owner_.tune.avoid_span; } // tune+0Ch

private:
    // 009E6170 / 009E6240's live test: +5Ch set, +5Dh / +60h / +5Eh clear, which
    // unit_alive_and_visible answers; a unit the gunnery host sank is gone.
    bool entity_live(std::size_t e) const {
        if (e >= owner_.units.count() || !owner_.units.unit_alive_and_visible(e)) return false;
        if (owner_.gunnery != nullptr) {
            const std::vector<GameGunneryUnitRow>& rows = owner_.gunnery->unit_rows();
            if (e < rows.size() && rows[e].sunk) return false;
        }
        return true;
    }
    float weapon_range_0494(std::size_t e) const {
        if (owner_.gunnery == nullptr) return 0.0f;
        const std::vector<GameGunneryUnitRow>& rows = owner_.gunnery->unit_rows();
        return e < rows.size() ? rows[e].any_weapon_max_range : 0.0f;
    }

    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
    std::vector<std::size_t> contacts_;
};

// 009E76D0.
class SelectBinding final : public bsp::ShipAiApproachSelectHost {
public:
    SelectBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                  GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}
    float wrap_angle_00605070(float value) override {
        return bsp::wrapped_angle_add_00438aa0(value, 0.0f);
    }
    float score_slot_009e6640(int slot, float seconds, float slot_scale, bool override_a,
                              bool override_b, float turn_radius) override {
        // 009E7755, the obstacle probe. Packet cc_ai_ring_scan read it whole.
        if (slot < 0 || slot >= bsp::kShipAiApproachSlotCount) return 0.0f;
        RingScanProbeBinding probe(owner_, ctl_, index_);
        const float score = bsp::ship_ai_ring_scan_probe_009e6640(ctl_.approach_ring[slot],
            ctl_.approach_scores[slot], seconds, slot_scale, override_a, override_b,
            turn_radius, probe);
        owner_.done("ShipAiApproach::score_slot_009e6640", 0x009e6640u);
        return score;
    }
    float tune_reject_penalty_04() override {
        return owner_.tune.slot_weight; // 009E784B, tune+4h
    }
    void refresh_unit_pose() override {
        owner_.record("ShipAiApproach::select_refresh_pose", 0x009e7cb4u);
    }
    bsp::ShipAiAttackMoveXZ brain_goal_0b2c() override {
        bsp::ShipAiAttackMoveXZ out{};
        out.x = ctl_.goal_vector.goal_x_0b2c;
        out.z = ctl_.goal_vector.goal_z_0b34;
        owner_.done("ShipAiApproach::select_brain_goal_0b2c", 0x009e7d7au);
        return out;
    }
    bsp::ShipAiAttackMoveXZ unit_world_xz() override {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        owner_.units.unit_position_00fc(index_, x, y, z);
        bsp::ShipAiAttackMoveXZ out{};
        out.x = x;
        out.z = z;
        owner_.done("ShipAiApproach::select_unit_xz", 0x009e7d92u);
        return out;
    }
    float unit_cruise_speed_0490() override {
        owner_.record("ShipAiApproach::select_cruise_speed_0490", 0x009e7de7u);
        return 0.0f;
    }
    void commit_bearing_009e5e90(float bearing, float) override {
        // 009E7ECB. Packet cc_ai_approach_update read 009E5E90 whole; it is the
        // producer of the commanded heading at nested+120Ch.
        bool blocked[bsp::kShipAiApproachSlotCount]{};
        for (int i = 0; i < bsp::kShipAiApproachSlotCount; ++i) {
            blocked[i] = ctl_.approach_scores[i].blocked_40;
        }
        bsp::ship_ai_approach_commit_bearing_009e5e90(ctl_.approach, blocked, bearing);
        owner_.done("ShipAiApproach::commit_bearing_009e5e90", 0x009e5e90u);
        ++row_.ring_scan_bearings;
        ++owner_.summary.ring_scan_bearings;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// 009E6A90.
class ThrottleLimitBinding final : public bsp::ShipAiApproachThrottleHost {
public:
    ThrottleLimitBinding(GameShipAiHost::Impl& owner,
                         GameShipAiHost::Impl::Controller& ctl, std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}
    float unit_heading_vtable_0050() override {
        owner_.done("ShipAiApproach::limit_unit_heading", 0x009e6ab5u);
        return owner_.units.unit_heading_radians(index_);
    }
    float unit_evade_flag_1128() override {
        owner_.record("ShipAiApproach::limit_evade_flag_1128", 0x009e6b46u);
        return 0.0f;
    }
    std::uint32_t engagement_target_009e5e00() override {
        owner_.done("ShipAiApproach::engagement_target_009e5e00", 0x009e5e00u);
        return ctl_.goal_vector.raw_target_0b20;
    }
    std::int32_t target_radius_07c4(std::uint32_t) override {
        owner_.record("ShipAiApproach::limit_target_radius_07c4", 0x009e6b90u);
        return 0;
    }
    bool unit_is_group_leader_00778890() override {
        owner_.record("ShipAiApproach::limit_group_leader_00778890", 0x00778890u);
        return false;
    }
    bool target_accepted_vtable_0234(std::uint32_t) override {
        owner_.record("ShipAiApproach::limit_target_accepted_0234", 0x009e6c86u);
        return false;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// bsp::ShipAiApproachUpdateHost, the seven calls of 009F3090 in their fixed
// order. Only the first is run: it is the one that produces the approach point
// and the goal range, and the other six need the 60-slot ring the four unread
// scorers 009E6400, 009E5DA0, 009E6870 and 009E6640 fill.
class ApproachUpdateBinding final : public bsp::ShipAiApproachUpdateHost {
public:
    ApproachUpdateBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                          GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    void frame_state_009f1bc0(float seconds) override {
        // 009F309B. The projection covers 009F1BC0-009F1DBF and
        // 009F1E60-009F1F47: the frame timers, the planar range to the
        // attackmove destination, the turn radius, the retarget timer and the
        // approach point itself, which is that destination copied verbatim
        // unless the target carries a zone object.
        ApproachPointBinding point(owner_, ctl_, index_);
        const bool has_target = ctl_.goal_vector.raw_target_0b20 != 0u;
        // 009F1E36, [target+740h]: the target's own zone object. No producer in
        // this process, so the displacement arm at 009F1E94 is never taken.
        owner_.record("ShipAiApproach::target_zone_object_0740", 0x009f1e36u);
        const int committed_before = ctl_.approach.committed_slot_11e8;
        bsp::ship_ai_approach_frame_state_009f1bc0(ctl_.approach, has_target, false,
                                                   seconds, point);
        // Packet cc8_ship_ai_committed_slot: 009F28F1, the one per-frame writer
        // of nested+11E8h. Counted here because the store is the last act of
        // the projection.
        if (row_.approach_frames == 0) {
            row_.ring_winner_first = ctl_.approach.committed_slot_11e8;
        } else if (ctl_.approach.committed_slot_11e8 != committed_before) {
            ++row_.committed_slot_changes;
        }
        row_.ring_winner_last = ctl_.approach.committed_slot_11e8;
        // 009F2F11 and 009F2FB1, the two 0095F080 refills of the curve objects
        // the standoff scan then samples. They sit in the span of 009F1BC0 the
        // projection does not cover, and the countdowns they re-arm are the
        // ones 009F1C07 and 009F1C13 have just decremented.
        refresh_approach_curves(has_target);
        owner_.done("ShipAiApproach::frame_state", 0x009f1bc0u);
        owner_.record("ShipAiApproach::frame_state_unread_spans", 0x009f1dbfu);
        ++row_.approach_frames;
        ++owner_.summary.approach_frames;
        row_.approach_point_x = ctl_.approach.point_1228.x;
        row_.approach_point_z = ctl_.approach.point_1228.z;
        row_.approach_goal_range = ctl_.approach.goal_range_11e0;
    }
    // Milestone 2r. The six arms milestone 2q recorded now run: packets
    // cc_ai_ring_scan, cc_ai_approach_update and cc_ai_bearing_rating between
    // them cover 009E7FC0, 009E6E80, 009E9190, 009E74D0, 009E76D0 with the four
    // slot scorers 009E6400 / 009E5DA0 / 009E6870 / 009E6640, the commit
    // 009E5E90 and the throttle limiter 009E6A90.
    void reset_scores_009e7fc0() override {
        owner_.ensure_approach_ring(ctl_, index_);
        ScoreResetBinding reset(owner_, ctl_, row_, index_);
        bsp::ship_ai_approach_reset_scores_009e7fc0(ctl_.approach, ctl_.approach_scores,
            ctl_.goal_vector.raw_target_0b20 != 0u, reset);
        owner_.done("ShipAiApproach::reset_scores", 0x009e7fc0u);
    }
    void choose_standoff_range_009e6e80() override {
        StandoffBinding standoff(owner_, ctl_, index_);
        bsp::ship_ai_approach_choose_standoff_009e6e80(ctl_.approach,
            ctl_.goal_vector.raw_target_0b20 != 0u, standoff);
        owner_.done("ShipAiApproach::choose_standoff_range", 0x009e6e80u);
        // Packet cc8_ship_ai_approach_curves: what the scan actually chose,
        // nested+11E4h after 009E6E80.
        const float chosen = ctl_.approach.standoff_range_11e4;
        if (row_.standoff_choices == 0) {
            row_.standoff_range_first = chosen;
        }
        row_.standoff_range_last = chosen;
        ++row_.standoff_choices;
        ++owner_.summary.standoff_choices;
    }
    void refresh_avoidance_009e9190(float seconds) override {
        AvoidBinding avoid(owner_, ctl_, row_, index_);
        bsp::ship_ai_approach_refresh_avoidance_009e9190(ctl_.approach, ctl_.approach_ring,
            ctl_.approach_scores, seconds, avoid);
        owner_.done("ShipAiApproach::refresh_avoidance", 0x009e9190u);
        // Packet cc9_ship_traffic: whether the pass left a nonzero avoid term.
        float strongest = 0.0f;
        for (int i = 0; i < bsp::kShipAiApproachSlotCount; ++i) {
            strongest = std::max(strongest, ctl_.approach_scores[i].avoid_3c);
        }
        if (strongest > 0.0f) ++row_.avoid_active_passes;
        row_.avoid_strength_max = std::max(row_.avoid_strength_max, strongest);
    }
    void score_evade_009e74d0(float seconds) override {
        EvadeBinding evade(owner_, index_);
        bsp::ship_ai_approach_score_evade_009e74d0(ctl_.approach, ctl_.approach_ring,
            ctl_.approach_scores, seconds, evade);
        owner_.done("ShipAiApproach::score_evade", 0x009e74d0u);
    }
    void select_slot_009e76d0(float seconds) override {
        SelectBinding select(owner_, ctl_, row_, index_);
        // Packet cc8_ship_ai_committed_slot: the winner 009E79CA..009E7C19
        // settles on, which the image keeps only in a register.
        const int winner = bsp::ship_ai_approach_select_slot_009e76d0(ctl_.approach,
            ctl_.approach_ring, ctl_.approach_scores, seconds, select);
        owner_.done("ShipAiApproach::select_slot", 0x009e76d0u);
        if (row_.ring_scan_winner_first >= 0 && winner != row_.ring_scan_winner_last) {
            ++row_.ring_scan_winner_changes;
        }
        if (row_.ring_scan_winner_first < 0) row_.ring_scan_winner_first = winner;
        row_.ring_scan_winner_last = winner;
        // 009E7BE0's five-word sum over the whole ring, so the census can say
        // whether slot 0 wins on merit or on the strict > at 009E7BFA leaving
        // a tie with the seed.
        float best = bsp::ship_ai_approach_slot_total_009e76d0(ctl_.approach_scores[0]);
        float worst = best;
        for (int i = 1; i < bsp::kShipAiApproachSlotCount; ++i) {
            const float total =
                bsp::ship_ai_approach_slot_total_009e76d0(ctl_.approach_scores[i]);
            if (total > best) best = total;
            if (total < worst) worst = total;
        }
        row_.ring_total_best = best;
        row_.ring_total_worst = worst;
        // The five words 009E7BE0 sums, for slot 0, so a NaN total can be
        // attributed to the word that carries it.
        row_.ring_word_raw_18 = ctl_.approach_scores[0].raw_18;
        row_.ring_word_normalized_2c = ctl_.approach_scores[0].normalized_2c;
        row_.ring_word_penalty_30 = ctl_.approach_scores[0].penalty_30;
        row_.ring_word_bearing_34 = ctl_.approach_scores[0].bearing_34;
        row_.ring_word_evade_38 = ctl_.approach_scores[0].evade_38;
        row_.ring_word_avoid_3c = ctl_.approach_scores[0].avoid_3c;
        ++row_.ring_scans;
        ++owner_.summary.ring_scans;
        // Packet cc8_ship_ai_approach_slot_tune: what the selection settled on,
        // and how often the heading it publishes actually changed.
        const float previous_heading = row_.approach_heading_120c;
        row_.ring_scan_winner = ctl_.approach.committed_slot_11e8;
        row_.approach_heading_120c = ctl_.approach.commanded_heading_120c;
        // ring_winner_first / ring_winner_last moved to the frame-state
        // binding, which is where 009F28F1 writes nested+11E8h. Reading them
        // here reported the field one scan late and, before the store existed,
        // reported nothing at all.
        if (row_.ring_scans != 1 && row_.approach_heading_120c != previous_heading) {
            ++row_.heading_changes;
        }
    }
    void limit_throttle_009e6a90() override {
        // 009E6A90's first act is wrap(heading - nested+120Ch), and nested+120Ch
        // is now what 009E5E90 wrote behind the ring scan above rather than the
        // 0.0f the constructor leaves, so the limiter runs on a produced input.
        ThrottleLimitBinding limit(owner_, ctl_, index_);
        bsp::ship_ai_approach_limit_throttle_009e6a90(ctl_.approach, limit);
        owner_.done("ShipAiApproach::limit_throttle", 0x009e6a90u);
        row_.approach_throttle_1210 = ctl_.approach.commanded_throttle_1210;
    }

private:
    // 009F2F11 and 009F2FB1, the two 0095F080 refills inside 009F1BC0. The own
    // curve takes prefer_long_range = 1 and re-arms nested+1220h with 1.5f
    // (009F2F16); the target's takes 0 and re-arms nested+1224h with 2.0f
    // (009F2FB6). The query blocks at nested+127Ch and nested+1238h have no
    // producer in this process beyond the two window constants 009F2EA1 and
    // 009F2EB1 write, so only those two are set and the rest of the block is
    // the zero it is born with. The curve rules around the answer are the
    // image's; the answer itself is only as good as FirepowerBinding, whose
    // device list is empty in this process.
    // Packet cc9_target_curve. 009F28FA..009F29D8 fill the block at nested+1238h
    // from THIS ship (owner+0AA8h), because the target's rating is of this ship:
    //   +1238h range         nested+11E0h, then 0095F080 steps it (009F2995)
    //   +123Ch length        [unit+9C8h] (009F294B), the unit radius, which has
    //                        no producer in the image (docs/GAME_EXECUTABLE.md);
    //                        LABELLED: 0, so the small-target curve weighs 1
    //   +1240h damage cap    [unit+370h] health (009F2931)
    //   +1244h armour        [[unit+538h]+4Ch] Armour (009F2906)
    //   +1248h torpedo armr  [unit+538h]->vtable[24h]() (009F2920) = 009635D0,
    //                        FLD [class+6B4h] UnderwaterArmour
    //   +124Ch bearing       nested+11DCh (009F299B); unused with +40h clear
    //   +1250h window        20.0f (00CE3930, 009F2985)
    //   +1254h horizon       30.0f (00CE38C8, 009F29A1)
    //   +1258h threshold     [[unit+538h]+6B8h] DamageThreshold (009F2963)
    //   +125Ch word 9        5.0f (00CE3850, 009F2969), never read
    //   +1270h word 14       0.0f (009F29D8), never read
    //   +1274h..+1277h gates BL = 1 (009F2733 MOV EBX,1; EBX is callee-saved over
    //                        the calls up to 009F29B2), all four categories
    //   +1278h, +1279h       0, 0 (009F29CA, 009F29D1): no bearing test, whole
    //                        barrel counts
    // Packet cc9_own_curve_target. 009F29E0..009F2A02: EBX = [owner+0B20h] when it
    // answers vtable[5Ch](5), else 0. With a target, 009F2A26..009F2A8F:
    //   +1288h armour        [[t+538h]+4Ch] Armour (009F2A2C)
    //   +128Ch torpedo armr  [t+538h]->vtable[24h]() (009F2A3C): ship class 009635D0
    //                        = class+6B4h UnderwaterArmour; plane class 004407A0
    //                        = class+4Ch Armour (no Ghidra function, 004407A0-004407A3)
    //   +1284h damage cap    [t+370h] health (009F2A44)
    //   +1280h length        [[t+538h]+0A0h] Length (009F2A54)
    //   +129Ch fire divisor  [[t+538h]+6B8h] DamageThreshold when t answers
    //                        vtable[5Ch](6), a ship (009F2A6B); else 10000.0f (009F2A7F)
    // Without one, 009F2A91..009F2AC1: 0, 0, 10000.0f, 10000.0f, 100.0f.
    void fill_own_block_target_127ch(bsp::ShipAiFirepowerQuery& q) const {
        if (owner_.fill_target_block_127ch(ctl_, q)) {
            owner_.done("ShipAiApproach::curve_query_target_fields", 0x009f2a44u);
        }
    }

    bsp::ShipAiFirepowerQuery target_query_1238h() const {
        return owner_.target_block_1238h(index_);
    }

    void refresh_approach_curves(bool has_target) {
        owner_.ensure_approach_curves(ctl_);

        bsp::ShipAiFirepowerQuery query{};
        query.window_seconds = 20.0f;        // 009F2EA1, 00CE3930
        query.ready_horizon_seconds = 30.0f; // 009F2EB1, 00CE38C8
        // 009F2A44's arm reads the four target fields off the target entity;
        // 009F2A91..009F2AC1 is the arm with no target, and these are its
        // constants. Packet cc8_ship_ai_firepower_inputs uses them on both arms
        // because [target+370h] and [[target+538h]+4Ch/+0A0h] have no producer
        // in this process. LABELLED SUBSTITUTION, recorded below.
        query.damage_cap = 10000.0f;   // 009F2AA9, 00CE3D64
        query.target_length = 100.0f;  // 009F2AC1, 00CE3D08
        query.armour = 0.0f;           // 009F2A91
        query.armour_torpedo = 0.0f;   // 009F2A99
        // 009F2ED2 sets use_ready_rounds and 009F2ECB clears require_bearing.
        query.use_ready_rounds = 1;
        query.require_bearing = 0;
        // 009F2AE7, 009F2B20, 009F2BC2 and 009F2B6D read these from
        // [0080E160(unit)+220h..+223h]. No producer in this process; with all
        // four clear 0095EBD7 skips every category and the rating is always
        // zero, so every category is allowed here. LABELLED SUBSTITUTION.
        query.allow_machine_gun = 1;
        query.allow_artillery = 1;
        query.allow_torpedo = 1;
        query.allow_depth_charge = 1;
        if (kShipAiOwnCurveTargetBound) {
            // 009F2AB1: the no-target fire divisor is 10000.0f (00CE3D64), not 0.
            query.damage_threshold = 10000.0f;
            fill_own_block_target_127ch(query);
        } else {
            owner_.record("ShipAiApproach::curve_query_target_fields", 0x009f2a44u);
        }
        owner_.record("ShipAiApproach::curve_query_allow_bytes", 0x009f2ae7u);
        FirepowerBinding firepower(owner_, index_);

        if (ctl_.approach.timer_1220 <= 0.0f) {
            bsp::ship_ai_firepower_range_profile_0095f080(query,
                ctl_.approach_curve_own.samples, true, firepower);
            ctl_.approach.timer_1220 = 1.5f;
            owner_.done("ShipAiApproach::curve_refresh_own_0095f080", 0x009f2f11u);
            ++owner_.summary.approach_curve_refreshes;
        }
        if (ctl_.approach.timer_1224 <= 0.0f) {
            // 009F2F3C, target->vtable[5Ch](5): no such probe in this process,
            // so the presence of a target stands in for it.
            owner_.record("ShipAiApproach::curve_target_kind_005c", 0x009f2f3cu);
            const std::uint32_t target_handle = ctl_.goal_vector.raw_target_0b20;
            if (has_target && kShipAiTargetCurveBound && target_handle != 0u
                && target_handle - 1u < owner_.units.count()) {
                // 009F29E0..009F29FE: the target is [owner+0B20h] when it answers
                // vtable[5Ch](5); the one-based handle names the unit. Its rating
                // of THIS ship, over the nested+1238h block.
                const std::size_t target = static_cast<std::size_t>(target_handle - 1u);
                bsp::ShipAiFirepowerQuery them = target_query_1238h();
                FirepowerBinding target_firepower(owner_, target);
                bsp::ship_ai_firepower_range_profile_0095f080(them,
                    ctl_.approach_curve_target.samples, false, target_firepower);
                owner_.done("ShipAiApproach::curve_target_block_1238", 0x009f28fau);
            } else if (has_target) {
                bsp::ship_ai_firepower_range_profile_0095f080(query,
                    ctl_.approach_curve_target.samples, false, firepower);
                owner_.done("ShipAiApproach::curve_refresh_target_0095f080", 0x009f2fb1u);
                ++owner_.summary.approach_curve_refreshes;
            }
            ctl_.approach.timer_1224 = 2.0f;
        }

        // Packet cc8_ship_ai_firepower_inputs: how much of each curve the
        // refill actually filled, so a reader can tell an empty profile from a
        // real one without reading the samples.
        row_.curve_own_nonzero = count_positive(ctl_.approach_curve_own);
        row_.curve_target_nonzero = count_positive(ctl_.approach_curve_target);
        row_.unit_max_weapon_range = firepower.unit_max_weapon_range();
    }

    static int count_positive(const bsp::ShipAiApproachRangeCurve& curve) {
        int n = 0;
        for (int i = 0; i < bsp::kShipAiApproachCurveSamples; ++i) {
            if (curve.samples[i] > 0.0f) ++n;
        }
        return n;
    }

    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

class ApproachStepBinding final : public bsp::ShipAiAttackMoveApproachHost {
public:
    ApproachStepBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                        GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    std::uint32_t brain_target_0b20() override {
        owner_.done("ShipAiApproach::brain_target_0b20", 0x009f3262u);
        return ctl_.goal_vector.raw_target_0b20;
    }
    bool target_retired_005d(std::uint32_t target) override {
        // 009F3277, the byte at target+5Dh. 00926390 sets it (with +60h) when the
        // 009273A0 flush reaches a unit whose damage death queued it
        // (docs/ENTITY_DEAD_FLAG.md). Packet cc9_target_release binds that; the
        // host has no separate flush, so the death itself is the moment.
        owner_.done("ShipAiApproach::target_retired_005d", 0x009f3277u);
        if (!kShipAiTargetReleaseBound || target == 0u) return false;
        const bool retired = owner_.unit_dead(static_cast<std::size_t>(target - 1u));
        if (retired) ++row_.target_retired_holds;
        return retired;
    }
    void hold_heading_and_stop_009e00a0() override {
        HeadingHoldBinding hold(owner_, ctl_, index_);
        bsp::ship_ai_hold_heading_and_stop_009e00a0(ctl_.blk, hold);
        owner_.done("ShipAiApproach::hold_heading_and_stop", 0x009e00a0u);
    }
    void nested_update_009f3090(float seconds) override {
        // 009F328F, with ECX = sub+8h (009F3289), so the nested object's
        // offsets are the sub-state's minus 8. Packet ship_ai_approach_update
        // projected the driver and six of its seven callees; the executable
        // runs the driver and the frame state that produces the approach point,
        // and records the rest with their own addresses.
        ApproachUpdateBinding update(owner_, ctl_, row_, index_);
        bsp::ship_ai_approach_update_009f3090(update, seconds);
        owner_.done("ShipAiApproach::nested_update", 0x009f3090u);
    }
    float unit_depth_reference_0494() override {
        // 009F32A0 / 009F32A6, [unit+494h]. No producer in this process.
        owner_.record("ShipAiApproach::unit_depth_reference", 0x009f32a0u);
        return 0.0f;
    }
    float sub_throttle_bias_11e8() override {
        // 009F3294, sub+11E8h = nested+11E0h, the planar range from the unit to
        // the attackmove destination that 009F1BC0 rewrites every frame at
        // 009F1CDE. A recovered value now.
        owner_.done("ShipAiApproach::sub_throttle_bias", 0x009f3294u);
        return ctl_.approach.goal_range_11e0;
    }
    void sub_goal_1230(float& x, float& z) override {
        // 009F3300 / 009F32EB, sub+1230h and sub+1238h = nested+1228h/+1230h,
        // the approach point. 009F1F2D..009F1F3D copies the attackmove
        // destination into it verbatim unless the target carries a zone object.
        owner_.done("ShipAiApproach::sub_goal_1230", 0x009f3300u);
        x = ctl_.approach.point_1228.x;
        z = ctl_.approach.point_1228.z;
    }
    float sub_heading_command_1214() override {
        // 009F3314, sub+1214h = nested+120Ch, written by 009E5E90 behind the
        // recorded ring scan 009E76D0.
        owner_.record("ShipAiApproach::sub_heading_command", 0x009f3314u);
        return ctl_.approach.commanded_heading_120c;
    }
    float sub_throttle_command_1218() override {
        // 009F339A, sub+1218h = nested+1210h. 009F1BF7 seeds this field with
        // the 9999.0f sentinel at 00CE4C04 on every frame and 009E6A90 is the
        // only routine that replaces it. 009E6A90 is recorded here, so the
        // sentinel is still in the field, and 009F3635's clamp to [-1, +1]
        // would turn it into full ahead - a number that looks like an order and
        // is only the marker for "the producer has not run". The read is
        // recorded and the neutral zero is used instead, which is what every
        // other unproduced value in this file answers with.
        owner_.record("ShipAiApproach::sub_throttle_command", 0x009f339au);
        if (static_cast<double>(ctl_.approach.commanded_throttle_1210) > 1000.0) {
            return 0.0f;
        }
        return ctl_.approach.commanded_throttle_1210;
    }
    void set_navigation_goal_009de050(const bsp::ShipAiAttackMoveXZ& goal, int keep_mode,
                                      int final_leg) override {
        owner_.run_navigation_goal_009de050(ctl_, row_, index_, goal.x, goal.z,
                                            keep_mode != 0, final_leg != 0);
    }
    // The five brain displacements this step writes are the control block's
    // own fields, because blk is brain+8h: brain+1CCh is blk+1C4h the steering
    // mode, brain+1D0h is blk+1C8h the throttle hold, brain+1D4h is blk+1CCh
    // the requested direction, brain+1D8h is blk+1D0h the desired throttle and
    // brain+1E0h is blk+1D8h the desired heading - which is why 009F3360 wraps
    // it with the same 00605070 that 009E0040 uses on that field.
    int brain_steering_mode_01cc() override {
        return static_cast<int>(ctl_.blk.mode);
    }
    void clear_brain_turn_accumulators_0368() override {
        // 009F3335..009F3348, brain+370h and brain+368h, which are blk+368h and
        // blk+360h, the two timers 009ED6B0 counts down.
        ctl_.blk.timer_368 = 0.0f;
        ctl_.blk.timer_360 = 0.0f;
        owner_.done("ShipAiApproach::clear_turn_accumulators", 0x009f3348u);
    }
    void set_brain_heading_01e0(float heading) override {
        ctl_.blk.desired_heading = heading;
        owner_.done("ShipAiApproach::set_brain_heading", 0x009f335cu);
    }
    void wrap_brain_heading_00605070() override {
        // 009F3360, 00605070 on brain+1E0h in place. The reconstruction of that
        // wrap is bsp::wrap_angle_00605070 where it exists; the field it wraps
        // has no reader in this process, so the call is recorded.
        owner_.record("ShipAiApproach::wrap_brain_heading", 0x00605070u);
    }
    void set_brain_replan_01d4(int value) override {
        ctl_.blk.requested_direction = static_cast<bsp::ShipAiThrottleDirection>(value);
    }
    void set_brain_steering_mode_01cc(int mode) override {
        ctl_.blk.mode = static_cast<bsp::ShipAiSteeringMode>(mode);
    }
    void set_brain_goal_hold_01d0(int value) override {
        ctl_.blk.throttle_hold_1c8 = value;
    }
    void set_brain_throttle_0258(float throttle) override {
        // 009F337B and 009F3383, brain+258h and brain+2C0h. Neither field has a
        // reader in the recovered chain, so this is where the approach's own
        // throttle stops: it is NOT blk+1D0h, the desired throttle the ring
        // hop carries.
        brain_throttle_0258_ = throttle;
        owner_.record("ShipAiApproach::set_brain_throttle_0258", 0x009f337bu);
    }
    float sub_sweep_timer_14b4() override { return ctl_.substate_ring_timer_14b4; }
    void set_sub_sweep_timer_14b4(float seconds_left) override {
        ctl_.substate_ring_timer_14b4 = seconds_left;
    }
    bool target_is_kind_vtable_005c(std::uint32_t target, int kind) override {
        owner_.done("ShipAiApproach::target_is_kind", 0x009f33bfu);
        if (target == 0u) return false;
        return owner_.units.unit_is_kind_of(static_cast<std::size_t>(target - 1u), kind);
    }
    bool unit_and_target_share_side_0054(std::uint32_t target) override {
        owner_.done("ShipAiApproach::share_side", 0x009f3402u);
        if (target == 0u) return false;
        return owner_.units.unit_side_0054(index_)
            == owner_.units.unit_side_0054(static_cast<std::size_t>(target - 1u));
    }
    bool unit_is_group_leader_00778890() override {
        // 00778890's body was read: MOV EAX,[ECX+284h] then [group+14h] == unit.
        // No AI group object exists in this process (milestone 2m reports
        // ai_groups=0), so the group pointer is null and the answer is false,
        // which takes the non-leader arm at 009F35D8.
        owner_.record("ShipAiApproach::unit_is_group_leader", 0x00778890u);
        return false;
    }
    int group_member_count_04f8() override { return 0; }
    std::uint32_t group_member_at_0070d060(int) override {
        owner_.record("ShipAiApproach::group_member_at", 0x0070d060u);
        return 0u;
    }
    bool member_is_kind_vtable_005c(std::uint32_t, int) override { return false; }
    bool member_armament_ready_vtable_002c(std::uint32_t) override { return false; }
    bool unit_armament_ready_vtable_002c() override {
        // 009F35D8, [unit+538h]->vtable[2Ch](). The object at unit+538h has no
        // recovered class, so the predicate is a record and false, which leaves
        // the candidate list empty and skips the warn sweep.
        owner_.record("ShipAiApproach::unit_armament_ready", 0x009f35d8u);
        return false;
    }
    std::uint32_t brain_unit_0aa8() override {
        return static_cast<std::uint32_t>(index_) + 1u;
    }
    void target_position_xz_00427eb0(std::uint32_t target, float& x, float& z) override {
        float y = 0.0f;
        x = 0.0f;
        z = 0.0f;
        if (target == 0u) return;
        owner_.done("ShipAiApproach::target_position", 0x00427eb0u);
        owner_.units.unit_position_00fc(static_cast<std::size_t>(target - 1u), x, y, z);
    }
    float candidate_body_speed_0092d730(std::uint32_t candidate) override {
        if (candidate == 0u) return 0.0f;
        return owner_.units.unit_forward_speed_0092d730(
            static_cast<std::size_t>(candidate - 1u));
    }
    float candidate_reference_speed_0080fc30(std::uint32_t) override {
        owner_.record("ShipAiApproach::candidate_reference_speed", 0x0080fc30u);
        return 1.0f;
    }
    bool candidate_pose_valid_00c8(std::uint32_t candidate) override {
        if (candidate == 0u) return true;
        return owner_.units.unit_pose_valid_00c8(static_cast<std::size_t>(candidate - 1u));
    }
    void refresh_candidate_pose_00414db0(std::uint32_t) override {
        owner_.record("ShipAiApproach::refresh_candidate_pose", 0x00414db0u);
    }
    void candidate_position_xz_00fc(std::uint32_t candidate, float& x, float& z) override {
        float y = 0.0f;
        x = 0.0f;
        z = 0.0f;
        if (candidate == 0u) return;
        owner_.units.unit_position_00fc(static_cast<std::size_t>(candidate - 1u), x, y, z);
    }
    std::int32_t target_warn_radius_07c4(std::uint32_t) override {
        owner_.record("ShipAiApproach::target_warn_radius", 0x009f3534u);
        return 0;
    }
    bool candidate_accepts_warning_vtable_0234(std::uint32_t, std::uint32_t) override {
        owner_.record_slot("ShipAiApproach::candidate_accepts_warning", "00cfc3d0+vtable234");
        return false;
    }
    void route_warning_message_0077c2a0(std::uint32_t) override {
        owner_.record("ShipAiApproach::route_warning_message", 0x0077c2a0u);
    }
    void set_brain_command_01d8(float command) override {
        // 009F3635, brain+1D8h. That is blk+1D0h - brain+8h is blk, so
        // brain+1D8h is the block's desired throttle - and the value is the
        // clamped sub+1218h the nested update would have produced.
        ctl_.blk.desired_throttle = command;
        owner_.done("ShipAiApproach::set_brain_command_01d8", 0x009f3635u);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
    float brain_throttle_0258_{0.0f};
};

class AttackMoveStepBinding final : public bsp::ShipAiAttackMoveStepHost {
public:
    AttackMoveStepBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                          GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    std::uint32_t brain_unit_0aa8() override {
        // 009E8828, [brain+0AA8h]. Every controller this process builds owns a
        // created instance, so the field is the unit and never zero; the handle
        // is this process's own index, one-based so 0 stays "no unit".
        owner_.done("ShipAiAttack::brain_unit_0aa8", 0x009e8828u);
        return static_cast<std::uint32_t>(index_) + 1u;
    }
    bool entity_is_kind_vtable_005c(std::uint32_t entity, int kind) override {
        // 009E883F and 009E888C, entity->vtable[5Ch](9), answered through the
        // recovered class chain 006FE530 this process already uses for the
        // automatic target scan. Only the owner's own handle can be resolved
        // here; a group member cannot, and no group exists.
        owner_.done("ShipAiAttack::entity_is_kind", 0x009e883fu);
        if (entity == 0u) return false;
        return owner_.units.unit_is_kind_of(static_cast<std::size_t>(entity - 1u), kind);
    }
    std::uint32_t unit_group_0284() override {
        // 009E8852, [unit+284h]. No AI group object exists in this process
        // (milestone 2m: ai_groups=0), so the null arm runs and the member walk
        // at 009E8867..009E889C is not reached.
        owner_.record("ShipAiAttack::unit_group_0284", 0x009e8852u);
        return 0u;
    }
    int group_member_count_04f8(std::uint32_t) override {
        owner_.record("ShipAiAttack::group_member_count", 0x009e8861u);
        return 0;
    }
    std::uint32_t group_member_at_0070d060(std::uint32_t, int) override {
        owner_.record("ShipAiAttack::group_member_at", 0x0070d060u);
        return 0u;
    }
    std::uint32_t unit_director_vtable_0114() override {
        owner_.record_slot("ShipAiAttack::unit_director", "00cfc3d0+vtable114");
        return 0u;
    }
    void end_command_0071e430(std::uint32_t, std::uint32_t command, int flag) override {
        // 009E88C1. The director argument is the unit's own vtable[114h]
        // accessor, which this process answers with the unit index, so the
        // completion runs on this unit's director.
        const GameCommandCompletion done
            = owner_.units.end_command_0071e430(index_, command, flag != 0);
        owner_.done("ShipAiAttack::end_command", 0x0071e430u);
        ++row_.command_endings;
        ++owner_.summary.command_endings;
        if (done.queue_advanced) {
            ++row_.command_completions;
            ++owner_.summary.command_completions;
        }
    }
    void select_substate_009e86f0(float seconds) override {
        AttackMoveSelectorBinding selector(owner_, ctl_, index_);
        bsp::ship_ai_attackmove_select_009e86f0(ctl_.selector, kAttackMoveStateBase, seconds,
                                                selector);
        owner_.done("ShipAiAttack::select_substate", 0x009e86f0u);
        row_.substate = ctl_.selector.current_1508 - kAttackMoveStateBase;
    }
    void substate_step_vtable_000c(float seconds) override {
        // 009E88F0, [state+1508h]->vtable[0Ch]. Milestone 2p dispatches on the
        // member the selector settled on, over the table
        // bsp::kShipAiAttackMoveSubStates that packet ship_ai_state_steps read
        // out of 009E8450. Packet cc_ai_attackmove_substates projected four of
        // the five bodies; the fifth, 007B3DD0, is one RET 4.
        ++row_.substate_steps;
        ++owner_.summary.substate_steps;
        const std::uint32_t offset = ctl_.selector.current_1508 - kAttackMoveStateBase;
        if (offset == 0x0008u) {
            ApproachStepBinding approach(owner_, ctl_, row_, index_);
            bsp::ship_ai_attackmove_approach_step_009f3240(seconds, approach);
            owner_.done("ShipAiAttack::approach_step", 0x009f3240u);
            ++row_.substate_concrete;
            ++owner_.summary.substate_concrete;
            return;
        }
        // The other four members are records with their own addresses. The
        // selector never reaches them on this mission: 009E8733's kind-8 test
        // and 009E8799's kind-1Ch test both answer false for a surface target,
        // which is the arm that pins the machine to state+8h, and the engage
        // gate 009E85B0 fails on the two unrecovered readiness floats.
        if (offset == 0x14C0u) {
            owner_.record("ShipAiAttack::engage_step", 0x009e23b0u);
        } else if (offset == 0x14CCu) {
            owner_.record("ShipAiAttack::lead_pursuit_step", 0x009e26c0u);
        } else if (offset == 0x14E0u) {
            owner_.record("ShipAiAttack::tangent_step", 0x009f3670u);
        } else if (offset == 0x14F4u) {
            // 007B3DD0's whole body is `C2 04 00`, one RET 4, COMDAT-folded
            // across twenty vtables. Running it is running nothing, so the
            // step is concrete and does nothing.
            owner_.done("ShipAiAttack::initial_step", 0x007b3dd0u);
            ++row_.substate_concrete;
            ++owner_.summary.substate_concrete;
        } else {
            owner_.record_slot("ShipAiAttack::substate_step_vtable0c", "00d21994+vtable0c");
        }
    }

private:
    // The attackmove state object's own base. This process holds no native
    // pointers, so the sub-state members are named by their offsets from an
    // arbitrary non-zero base; only their identity matters to the selector.
    static constexpr std::uint32_t kAttackMoveStateBase = 0x10000000u;

    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::ShipAiRudderLawHost, the one call site inside 009DA250
// ---------------------------------------------------------------------------

class RudderLawBinding final : public bsp::ShipAiRudderLawHost {
public:
    RudderLawBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}
    float unit_body_axis_speed_0092d730() override {
        // 009DA2D7 with ECX = [unit+1018h] set at 009DA2D1: the hull's signed
        // forward speed, the same value the trajectory dump's fwd_speed carries.
        const float speed = owner_.units.unit_forward_speed_0092d730(index_);
        owner_.done("ShipAiRudder::body_axis_speed", 0x0092d730u);
        return speed;
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::ShipAiRingHopHost, the three call sites of 009F4B99..009F4D04
// ---------------------------------------------------------------------------

class RingHopBinding final : public bsp::ShipAiRingHopHost {
public:
    RingHopBinding(GameShipAiHost::Impl& owner, GameShipAiRow& row, std::size_t index)
        : owner_(owner), row_(row), index_(index) {}

    float unit_body_axis_speed_0092d730() override {
        // 009F4BD4, reached only when the throttle is already inside the
        // 009F4BB9 deadband, which is what makes the zeroing arm a measurement.
        const float speed = owner_.units.unit_forward_speed_0092d730(index_);
        owner_.done("ShipAiRing::hop_body_axis_speed", 0x0092d730u);
        return speed;
    }
    void set_ring_write_slot_rudder_0080e190(float value) override {
        // 009F4CE8, 0080E190 with ECX = [blk+3FCh]: [unit + ([unit+97Ch]<<5) +
        // 83Ch]. Always the first of the two.
        owner_.units.unit_ring_set_write_slot_rudder_0080e190(index_, value);
        owner_.done("ShipAiRing::set_write_slot_rudder", 0x0080e190u);
        row_.ring_slot_rudder = value;
    }
    void set_ring_write_slot_throttle_0080e170(float value) override {
        // 009F4CFB, 0080E170: [unit + ([unit+97Ch]<<5) + 838h].
        owner_.units.unit_ring_set_write_slot_throttle_0080e170(index_, value);
        owner_.done("ShipAiRing::set_write_slot_throttle", 0x0080e170u);
        row_.ring_slot_throttle = value;
        ++row_.ring_writes;
        ++owner_.summary.ring_writes;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::ShipAiDirectControlHost, the seven call sites of 009ED6B0's own arm
// ---------------------------------------------------------------------------

class DirectControlBinding final : public bsp::ShipAiDirectControlHost {
public:
    DirectControlBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}

    void prologue_0080e000(float seconds) override {
        auto& order = owner_.controllers[index_].order;
        bsp::unit_ai_order_slot_step_0080e000(order.slots[order.index], seconds);
        owner_.done("ShipAiControls::prologue", 0x0080e000u);
    }
    bool controller_belongs_to_another_007788b0() override {
        // 007788B0 BSP_Unit_IsFormationFollower: g = [unit+284h]; g && [g+14h] != unit.
        // 009ED73F clears blk+3A5h (brain+3ADh) unless this answers true, so with
        // the stub's false the station request never survived to the 009EDA34 gate
        // or to 009F4DA0 (packet cc9_station_keeping, docs/STATION_KEEPING.md).
        if (!kShipStationKeepingBound) {
            owner_.record("ShipAiControls::controller_belongs_to_another", 0x007788b0u);
            return false;
        }
        owner_.done("ShipAiControls::controller_belongs_to_another", 0x007788b0u);
        const std::int32_t group = owner_.units.unit_formation_group_0284(index_);
        if (group < 0) return false;
        const std::size_t leader = owner_.units.formation_leader_0014(group);
        return leader != index_;
    }
    float unit_body_axis_speed_0092d730() override {
        owner_.done("ShipAiControls::body_axis_speed", 0x0092d730u);
        return owner_.units.unit_forward_speed_0092d730(index_);
    }
    float ship_class_field_0508() override {
        // 009ED8E4/009ED8EC: FDIV [ECX+508h] with ECX = [unit+538h]. That field
        // is `Retardation` (docs/SHIP_CLASS_FIELDS.md, the loader at
        // 00831882..00831998, and bsp::ShipMotionClass::retardation), which this
        // process already reads out of the installed `VehicleClass` row, so the
        // divisor is the ship's own braking deceleration.
        owner_.done("ShipAiControls::class_retardation_0508", 0x009ed8ecu);
        return owner_.units.unit_retardation_0508(index_);
    }
    float unit_field_09c8() override {
        // 009ED8DA loads the controlled unit from blk+3FCh; 009ED902 adds
        // its +9C8h hull extent to the braking distance. The existing unit
        // owner supplies the native no-model-box fallback from class Length.
        owner_.done("ShipAiControls::unit_field_09c8", 0x009ed902u);
        return owner_.units.unit_hull_length_09c8(index_);
    }
    float unit_heading_vtable_0050() override {
        owner_.done("ShipAiControls::unit_heading", 0x009ed95du);
        return owner_.units.unit_heading_radians(index_);
    }
    float unit_current_yaw_rate_00811940() override {
        owner_.done("ShipAiControls::current_yaw_rate", 0x00811940u);
        return owner_.units.unit_current_yaw_rate_00811940(index_);
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::UnitHeadingTargetHost and bsp::ShipAiPublishHost, 009F4D10's own calls
// ---------------------------------------------------------------------------

class HeadingTargetBinding final : public bsp::UnitHeadingTargetHost {
public:
    HeadingTargetBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}
    float heading_virtual_0050() override {
        owner_.done("ShipAiOrder::unit_heading", 0x00811974u);
        return owner_.units.unit_heading_radians(index_);
    }
    float forward_speed_0092d730() override {
        owner_.done("ShipAiOrder::forward_speed", 0x008119a1u);
        return owner_.units.unit_forward_speed_0092d730(index_);
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

class PublishBinding final : public bsp::ShipAiPublishHost {
public:
    PublishBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                   std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    int order_slot_index_0b40() override {
        owner_.done("ShipAiOrder::slot_index_0b40", 0x009f4d2fu);
        return ctl_.order.index;
    }
    void set_heading_target_00811960(bsp::UnitHeadingTargetState& state,
                                     float desired_heading) override {
        HeadingTargetBinding binding(owner_, index_);
        bsp::unit_set_heading_target_00811960(state, desired_heading, binding);
        owner_.done("ShipAiOrder::set_heading_target", 0x00811960u);
    }
    void tail_009f0100(float) override {
        // 009F0100: both arms walk the neighbour list at blk+608h over the count
        // at blk+604h (009F0163 JLE 009F09FF, 009F01BF JLE 009F09FF), after
        // nothing but a speed read (0092D730). With no neighbours it stores
        // nothing. The non-empty body (009F01CB..009F09F9) is unread.
        if (kShipAiOrderTailBound && ctl_.nav_block.neighbour_count_604 < 1) {
            owner_.done("ShipAiOrder::tail_009f0100", 0x009f0100u);
            return;
        }
        owner_.record("ShipAiOrder::tail_009f0100", 0x009f0100u);
    }
    void tail_009ef350() override {
        owner_.record("ShipAiOrder::tail_009ef350", 0x009ef350u);
    }
    void tail_009ef910(float) override {
        owner_.record("ShipAiOrder::tail_009ef910", 0x009ef910u);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// Milestone 2p: 009F1420's head, the brain pre-pass that writes the goal vector
// ---------------------------------------------------------------------------
//
// docs/SHIP_AI_GOAL_VECTOR.md: `brain+0B2Ch..+0B34h` has one writer and it is
// this routine, which runs on every AI sub-tick from whatever command the
// entity is carrying. Milestone 2o recorded the call site and reported all 98
// of its `movetopos` goal sets as (0,0); this runs the producer instead. The
// executable already holds the director's command slots from milestones 2l,
// 2m and 2n, so 0071EB60 answers with a real descriptor rather than the empty
// singleton at 00E19B98 and the goal becomes the command's own position or the
// resolved target's world position.

class GoalVectorBinding final : public bsp::ShipAiGoalVectorHost {
public:
    GoalVectorBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                      std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    bsp::ShipAiGoalCommandDescriptor active_command_0071eb60() override {
        bsp::ShipAiGoalCommandDescriptor out{};
        bsp::SceneCommandTarget descriptor{};
        int mode = 0;
        const bool real = owner_.units.active_command_descriptor_0071eb60(index_, descriptor,
            mode);
        owner_.done("ShipAiGoal::active_command", 0x0071eb60u);
        descriptor_ = descriptor;
        singleton_ = !real;
        if (!real) {
            // The 00E19B98 singleton: +1h clear, +14h zero, the triple copied
            // from the zero vector at 00F87574.
            return out;
        }
        out.has_position = descriptor.position_valid != 0;
        out.x = descriptor.position[0];
        out.y = descriptor.position[1];
        out.z = descriptor.position[2];
        out.target = owner_.units.resolve_command_target_00521ea0(descriptor);
        return out;
    }
    std::uint32_t resolve_command_target_00521ea0() override {
        const std::uint32_t target = singleton_
            ? 0u
            : owner_.units.resolve_command_target_00521ea0(descriptor_);
        owner_.done("ShipAiGoal::resolve_command_target", 0x00521ea0u);
        return target;
    }
    void observer_unregister_006952a0(std::uint32_t) override {
        // 009E2FFD. The observer list the record joins is not built here, so
        // the pair is recorded; nothing downstream reads it.
        owner_.record("ShipAiGoal::observer_unregister", 0x006952a0u);
    }
    void observer_register_00694a60(std::uint32_t) override {
        owner_.record("ShipAiGoal::observer_register", 0x00694a60u);
    }
    bool target_is_kind_vtable_005c(std::uint32_t target, int kind) override {
        // 009F1491 with the literal 2 at 009F148D, answered through the same
        // recovered class chain 006FE530 the automatic target scan uses.
        owner_.done("ShipAiGoal::target_is_kind", 0x009f1491u);
        if (target == 0u) return false;
        return owner_.units.unit_is_kind_of(static_cast<std::size_t>(target - 1u), kind);
    }
    int unit_side_0054() override {
        owner_.done("ShipAiGoal::unit_side", 0x009f14e4u);
        return owner_.units.unit_side_0054(index_);
    }
    int target_side_0054(std::uint32_t target) override {
        owner_.done("ShipAiGoal::target_side", 0x009f14dbu);
        if (target == 0u) return -1;
        return owner_.units.unit_side_0054(static_cast<std::size_t>(target - 1u));
    }
    bool recon_knows_target_009dfbe0(int own_side, std::uint32_t target) override {
        // 009F14EC 008053C0 BSP_Recon_EnsureSlot(side) then 009F14FA 009DFBE0.
        // 009DFBE0's body is read now (docs/SHIP_AI_GOAL_VECTOR_VISIBILITY.md):
        // it walks the head at slot+0E0Ch, triple 4, the union of own / enemy /
        // neutral / unknown, and returns the record whose +4h is the target.
        // Membership is docs/RECON_SLOT_LISTS.md rules (a), (b) and the level
        // drain; this host answers (a) and (b) with the same unit facts the
        // gunnery host's contact sweep uses and does NOT build a second recon.
        //
        // Rule (c), the sensor pass 00806840/008048A0 that sets each entry's
        // level, runs in the gunnery host's tick (008073C0 before the gunnery
        // pass) and publishes its answer per (observing side, target). This
        // read takes that level. A side the pass never covered keeps
        // kReconDetectionUnknownLevel, which is `identified`: the permissive
        // answer the tree used before rule (c) ran, so binding the pass can
        // only ever make a target less visible where a sensor judged it.
        // docs/RECON_SENSOR_PASS_BINDING.md.
        owner_.done("ShipAiGoal::recon_knows_target", 0x009dfbe0u);
        if (target == 0u) return false;
        const std::size_t other = static_cast<std::size_t>(target - 1u);
        if (other >= owner_.units.count()) return false;
        bsp::ReconUnionMemberFacts facts;
        facts.present = true;
        facts.same_side = owner_.units.unit_side_0054(other) == own_side;
        facts.scanned_class =
            bsp::recon_scan_visits_class_00806480(owner_.units.unit_class_id(other));
        bsp::SceneNodeFlags flags;
        bool pending_destroy = false;
        if (owner_.units.unit_scene_node_flags(other, flags) &&
            owner_.units.unit_pending_destroy_0060(other, pending_destroy)) {
            facts.gate.live_5c = flags.active;
            facts.gate.simulate_5d = flags.torn_down;
            facts.gate.dead_5e = flags.destroyed;
            facts.gate.gate_60 = pending_destroy;
        }
        // The scan walks the world registry's per-class lists, so a unit the
        // registry no longer holds is in no list whatever its gate bytes say.
        if (!owner_.units.unit_active(other)) return false;
        facts.level = owner_.gunnery != nullptr
            ? owner_.gunnery->recon_sensor_pass_state().level(own_side, other)
            : bsp::kReconDetectionUnknownLevel;
        const bool known = bsp::recon_union_contains_009dfbe0(facts);
        if (index_ < owner_.rows.size() && known) ++owner_.rows[index_].goal_visible_recon;
        return known;
    }
    bool target_is_surface_00922dc0(std::uint32_t target) override {
        // 009F1519, 00922DC0 -> 00922C80 BSP_Entity_IsSurfaceTarget on the RAW
        // target, with DL = 1 from the thunk (00922DC0 MOV DL,1 / JMP). The
        // rule is already reconstructed in bsp/attack_target_classify.hpp from
        // the same listing; this binding fills its facts and does not restate
        // it. The ship family arm answers every surface ship true without
        // reaching 008DDF90.
        owner_.done("ShipAiGoal::target_is_surface", 0x00922dc0u);
        if (target == 0u) return false;
        const std::size_t other = static_cast<std::size_t>(target - 1u);
        if (other >= owner_.units.count()) return false;
        bsp::EntityTargetFacts tf;
        tf.present = true;
        tf.not_engageable = owner_.units.unit_flag_005d(other);
        tf.is_plane = owner_.units.unit_is_kind_of(other, 0x0f);
        tf.is_plane_squadron = owner_.units.unit_is_kind_of(other, 0x18);
        tf.is_ship_family = owner_.units.unit_is_kind_of(other, 0x06);
        tf.is_submarine = owner_.units.unit_is_kind_of(other, 0x08);
        tf.is_airfield = owner_.units.unit_is_kind_of(other, 0x45);
        tf.is_shipyard = owner_.units.unit_is_kind_of(other, 0x46);
        tf.is_command_building = owner_.units.unit_is_kind_of(other, 0x1c);
        tf.is_dummy_target = owner_.units.unit_is_kind_of(other, 0x35);
        tf.is_land_fort = owner_.units.unit_is_kind_of(other, 0x1b);
        float px = 0.0f, py = 0.0f, pz = 0.0f;
        owner_.units.unit_position_00fc(other, px, py, pz);
        tf.world_y = py;
        const bsp::SurfaceTargetAnswer answer = bsp::entity_is_surface_target_00922c80(tf);
        bool surface = answer == bsp::SurfaceTargetAnswer::kYes;
        if (answer == bsp::SurfaceTargetAnswer::kUnreadSetBranch) {
            // 008DDF90 BSP_SzurkeNyil_ContainsUnit over a set this process does
            // not build. The tail 00922C80 runs when the set does not hold the
            // entity is answerable, so take that and record the branch.
            owner_.record("ShipAiGoal::target_is_surface_set_branch", 0x008ddf90u);
            surface = bsp::entity_surface_target_tail_00922c80(tf, true);
        }
        if (index_ < owner_.rows.size() && surface) ++owner_.rows[index_].goal_visible_surface;
        return surface;
    }
    bool target_pose_valid_00c8(std::uint32_t target) override {
        owner_.done("ShipAiGoal::target_pose_valid", 0x009dbcceu);
        if (target == 0u) return true;
        return owner_.units.unit_pose_valid_00c8(static_cast<std::size_t>(target - 1u));
    }
    void refresh_target_pose_00414db0(std::uint32_t) override {
        owner_.record("ShipAiGoal::refresh_target_pose", 0x00414db0u);
    }
    void transform_by_target_matrix_004142e0(std::uint32_t target, float in_x, float in_y,
                                             float in_z, float& out_x, float& out_y,
                                             float& out_z) override {
        // 009DBCED, 004142E0 with the matrix at target+0CCh: the latched triple
        // is a point in the target's frame. With a command that named an entity
        // the triple is the zero vector 009E2FC4 substituted, so the answer is
        // the target's own world position.
        owner_.done("ShipAiGoal::transform_by_target_matrix", 0x004142e0u);
        if (target == 0u) {
            out_x = in_x;
            out_y = in_y;
            out_z = in_z;
            return;
        }
        owner_.units.transform_by_unit_matrix_004142e0(
            static_cast<std::size_t>(target - 1u), in_x, in_y, in_z, out_x, out_y, out_z);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
    bsp::SceneCommandTarget descriptor_{};
    bool singleton_{true};
};

// ---------------------------------------------------------------------------
// Milestone 2p: 009EF230, the obstacle sector refresh at chain slot 009F51FA
// ---------------------------------------------------------------------------
//
// docs/SHIP_AI_OBSTACLE_TABLES.md reads the routine and its schedule but does
// not project it, so this runs the two parts the doc establishes instruction by
// instruction - the round robin on blk+0A18h and the braking distance the
// store at 009EF32F writes into each visited sector's +8h - and records
// 009EB660 BSP_ShipAi_ScanObstacleSector once per visited sector, because the
// swept-arc geometry between 009EB6B7 and 009EBECC was not read by any packet
// and the neighbour list at blk+608h is empty in this process.

// 009EB660 borrows the controller's actual searcher-zero list. Its cache
// starts enabled and its head starts null; query refresh is a separate chain
// dependency. The neighbour list remains empty in this process.
class SectorScanBinding final : public bsp::ShipAiSectorScanHost {
public:
    SectorScanBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}

    float settings_blocked_margin_1d8() override {
        owner_.done("ShipAiSectorScan::settings_blocked_margin_1d8", 0x009eb686u);
        return owner_.avoidance_tuning()[2];
    }
    float settings_neighbour_memory_194() override {
        owner_.done("ShipAiSectorScan::settings_neighbour_memory_194", 0x009ebee3u);
        return owner_.avoidance_tuning()[0];
    }
    float unit_heading_vtable50() override {
        owner_.done("ShipAiSectorScan::unit_heading_vtable50", 0x009eb939u);
        return owner_.units.unit_heading_radians(index_);
    }
    bool avoid_zone_segment_crossing_004158e0(const std::array<float, 2>& from,
                                              const std::array<float, 2>& toward,
                                              std::array<float, 2>& hit) override {
        const auto& list = owner_.controllers[index_].avoid_search->list(0);
        owner_.done("ShipAiSectorScan::zone_segment_crossing_004158e0", 0x004158e0u);
        return owner_.zones.search_segment(list, from, toward, hit);
    }
    bool point_in_avoid_box_009d8160(const bsp::ShipAiObstacleNode&,
                                     const std::array<float, 2>&) override {
        owner_.record("ShipAiSectorScan::point_in_avoid_box_009d8160", 0x009d8160u);
        return false;
    }
    bool point_in_near_box_009d80c0(const bsp::ShipAiObstacleNode&,
                                    const std::array<float, 2>&) override {
        owner_.record("ShipAiSectorScan::point_in_near_box_009d80c0", 0x009d80c0u);
        return false;
    }
    bool clip_ray_against_node_009dd540(const bsp::ShipAiObstacleNode&,
                                        const std::array<float, 2>&,
                                        const std::array<float, 2>&, float&) override {
        owner_.record("ShipAiSectorScan::clip_ray_009dd540", 0x009dd540u);
        return false;
    }
    bool clip_arc_against_avoid_zones_00415970(const std::array<float, 2>& center,
                                               float radius, float start,
                                               float& end) override {
        const auto& list = owner_.controllers[index_].avoid_search->list(0);
        owner_.done("ShipAiSectorScan::clip_arc_zones_00415970", 0x00415970u);
        return owner_.zones.search_arc(list, center, radius, start, end);
    }
    bool clip_arc_against_node_009dd010(const bsp::ShipAiObstacleNode&,
                                        const std::array<float, 2>&, float, float,
                                        float&) override {
        owner_.record("ShipAiSectorScan::clip_arc_node_009dd010", 0x009dd010u);
        return false;
    }
    void raise_node_lifetime_78(bsp::ShipAiObstacleNode& node, float value) override {
        // 009EBEF7, an inlined compare and store on the blocking node. Reached
        // only when a node blocks, which needs a neighbour list.
        if (value > node.lifetime_78) node.lifetime_78 = value;
        owner_.record("ShipAiSectorScan::raise_node_lifetime_78", 0x009ebef7u);
    }
    bool avoid_zone_free_bearing_009dc2e0(bsp::ShipAiSectorFreeBearingQuery&) override {
        owner_.record("ShipAiSectorScan::free_bearing_009dc2e0", 0x009dc2e0u);
        return false;
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

class ObstacleSectorRefresh {
public:
    static void run(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                    std::size_t index) {
        // 009EF247..009EF283: v = max(0092D730(unit), class+500h * 0.1).
        const float speed = owner.units.unit_forward_speed_0092d730(index);
        owner.done("ShipAiSectors::body_axis_speed", 0x0092d730u);
        const float max_speed = owner.units.unit_class_max_speed_0500(index);
        const double floor_speed = static_cast<double>(max_speed) * 0.1;  // 00D7A3A0
        double v = static_cast<double>(speed);
        if (v < floor_speed) v = floor_speed;
        // 009EF289..009EF2C9, with the doubles 00D7A2B0 = 3.0, 00CEC8F0 = 0.55
        // and 00CEFF98 = 0.6. class+508h is `Retardation`, the same divisor
        // 009ED8EC uses.
        const double retardation = static_cast<double>(owner.units.unit_retardation_0508(index));
        const double reach = v + 3.0;
        const double half_width = static_cast<double>(owner.units.unit_half_width_09cc(index));
        double braking = 0.0;
        if (retardation != 0.0) {
            braking = (reach / retardation) * reach * 0.55 + half_width * 0.6;
        } else {
            // 009EF2A1 FDIV with a zero divisor. The class row this mission
            // installs carries a real Retardation for every ship, so this arm
            // is reported rather than reasoned about.
            owner.record("ShipAiSectors::retardation_zero", 0x009ef2a1u);
        }
        const float braking_distance = static_cast<float>(braking);
        // 009EF2BD..009EF323: the schedule. The cursor is a 0..3 counter and
        // the loop starts at (counter & 1) + (counter >= 2 ? 6 : 0), stepping
        // two strides three times, so the twelve sectors refresh over four
        // frames, three a frame.
        const int cursor = ctl.sector_refresh_cursor_0a18;
        int sector = (cursor & 1) + ((cursor >= 2) ? 6 : 0);
        for (int i = 0; i < 3; ++i) {
            if (sector >= 0 && sector < bsp::kShipAiObstacleSectorCount) {
                ctl.obstacle.sector[static_cast<std::size_t>(sector)].braking_distance
                    = braking_distance;
                // 009EF334, 009EB660(sector, blk). Milestone 2r runs packet
                // cc_ai_sector_scan's whole-body projection in place of
                // milestone 2p's record, so the probe geometry, the swept arc
                // and the range with the hysteresis margin are code. The
                // neighbour list at blk+608h is empty; the zone gates below
                // read the live cache and selected-list owner.
                bsp::ShipAiSectorScanInputs scan{};
                const auto& hull = ctl.hull_geometry;
                scan.pose.x = hull.position_184[0];
                scan.pose.z = hull.position_184[1];
                scan.pose.forward_x = hull.forward_1ac[0];
                scan.pose.forward_z = hull.forward_1ac[1];
                scan.pose.port_x = hull.beam_19c[0];
                scan.pose.port_z = hull.beam_19c[1];
                scan.pose.starboard_x = hull.opposite_beam_1a4[0];
                scan.pose.starboard_z = hull.opposite_beam_1a4[1];
                scan.pose.heading = owner.units.unit_heading_radians(index);
                scan.avoid_zones_present = ctl.avoid_search->list(0).head != nullptr;
                scan.avoid_zones_enabled = ctl.avoid_search->cache(0).enabled;
                SectorScanBinding scan_host(owner, index);
                const bsp::ShipAiSectorScanResult result
                    = bsp::ship_ai_scan_obstacle_sector_009eb660(
                        ctl.obstacle.sector[static_cast<std::size_t>(sector)], scan,
                        ctl.neighbours, scan_host);
                owner.done("ShipAiSectors::scan_sector", 0x009eb660u);
                ++owner.summary.sector_scans;
                if (result.blocked) ++owner.summary.sector_marks;
                // The scan executes recovered 009D84E0 at 009EBF67 only for
                // a blocking neighbour. An empty list reaches no such call.
                if (result.blocking_node >= 0) {
                    owner.done("ShipAiSectors::passing_corner_009d84e0", 0x009d84e0u);
                }
                ctl.obstacle.sector[static_cast<std::size_t>(sector)].blocked
                    = result.blocked;
                ctl.obstacle.sector[static_cast<std::size_t>(sector)].blocker = nullptr;
            }
            sector += 2;
        }
        ctl.sector_refresh_cursor_0a18 = (cursor + 1) & 3;
        owner.done("ShipAi::refresh_obstacle_sectors", 0x009ef230u);
    }
};

// ---------------------------------------------------------------------------
// Milestone 2p: bsp::ShipAiObstacleHost, the middle of 009F3F80
// ---------------------------------------------------------------------------

class ObstacleBinding final : public bsp::ShipAiObstacleHost {
public:
    ObstacleBinding(GameShipAiHost::Impl& owner, std::size_t index) : owner_(owner),
        index_(index) {}
    void raise_turn_assist_load_102c(float value) override {
        // 009F438C, 009F462A and 009F4A51, the inlined body of 009D4FB0.
        owner_.units.raise_turn_assist_load_102c(index_, value);
        owner_.done("ShipAiObstacle::raise_turn_assist_load", 0x009f438cu);
    }
    void raise_secondary_load_1034(float value) override {
        // 009F45FE, 009F4912 and 009F4AB4, the inlined body of 009D4FE0.
        owner_.units.raise_secondary_load_1034(index_, value);
        owner_.done("ShipAiObstacle::raise_secondary_load", 0x009f45feu);
    }
    float rudder_law_009da250(float heading_error) override {
        // 009F44F7 and 009F4694. The law itself is the same reconstruction the
        // 2o path already ran; the divisor is the derived class+524h.
        RudderLawBinding law(owner_, index_);
        bool derived = false;
        const float authority = owner_.units.unit_yaw_authority_0524(index_, derived);
        if (derived) {
            owner_.done("ShipAiRudder::class_yaw_authority_0524", 0x00828f20u);
        } else {
            owner_.record("ShipAiRudder::class_yaw_authority_ungated", 0x009da268u);
        }
        const float rudder = bsp::ship_ai_rudder_from_heading_error_009da250(direction_,
            heading_error, authority, law);
        owner_.done("ShipAiRudder::from_heading_error", 0x009da250u);
        ++calls_;
        return rudder;
    }
    float neighbour_body_axis_speed_0092d730(const bsp::ShipAiNeighbourRecord&) override {
        // 009F47B8 and 009D8BC1. Unreachable here: the blocked-sector arm needs
        // a sector with a blocker, and 009EB660's scan finds none.
        owner_.record("ShipAiObstacle::neighbour_body_axis_speed", 0x009f47b8u);
        return 0.0f;
    }

    void set_direction(bsp::ShipAiThrottleDirection direction) noexcept {
        direction_ = direction;
    }
    unsigned long long calls() const noexcept { return calls_; }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
    bsp::ShipAiThrottleDirection direction_{bsp::ShipAiThrottleDirection::Stopped};
    unsigned long long calls_{0};
};

// ---------------------------------------------------------------------------
// Milestone 2p: bsp::ShipAiPathPickHost, 009EE580..009EE670
// ---------------------------------------------------------------------------

// bsp::ShipAiPathPlannerHost, the call sites inside 009E3780. Packet
// ship_ai_path_planner landed on main at 878325ba during this packet's turn and
// was merged in before validation, so the plan request is no longer a record.
class PathPlannerBinding final : public bsp::ShipAiPathPlannerHost {
public:
    PathPlannerBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                       std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    std::uint32_t avoid_zone_manager_004218e0() override {
        owner_.done("ShipAiPlanner::avoid_zone_manager", 0x004218e0u);
        return owner_.zones.manager_handle();
    }
    std::uint32_t zone_containing_point_00417e40(std::uint32_t,
        const std::array<float, 2>& point, std::uint32_t layer) override {
        owner_.done("ShipAiPlanner::zone_containing_point", 0x00417e40u);
        return owner_.zones.containing(point, layer);
    }
    std::array<float, 2> push_point_out_of_zone_00417580(std::uint32_t zone,
        const std::array<float, 2>& point, float margin) override {
        owner_.done("ShipAiPlanner::push_point_out_of_zone", 0x00417580u);
        return owner_.zones.push_out(zone, point, margin);
    }
    std::uint32_t zone_group_for_layer_004120d0(std::uint32_t, std::uint32_t layer) override {
        owner_.done("ShipAiPlanner::zone_group_for_layer", 0x004120d0u);
        return owner_.zones.group_for_layer(layer);
    }
    std::array<float, 2> nearest_zone_boundary_0041b840(std::uint32_t group,
        const std::array<float, 2>& point, float slack, float push) override {
        owner_.done("ShipAiPlanner::nearest_zone_boundary", 0x0041b840u);
        return owner_.zones.nearest(group, point, slack, push);
    }
    bsp::ShipAiPathNode* allocate_path_node_00bf681b(std::size_t) override {
        // operator new. The nodes belong to the plan block, so this process
        // owns them for the life of the controller.
        ctl_.plan_nodes.emplace_back();
        owner_.done("ShipAiPlanner::allocate_path_node", 0x00bf681bu);
        return &ctl_.plan_nodes.back();
    }
    float owner_seed_vtable50() override {
        owner_.done("ShipAiPlanner::owner_seed_heading", 0x009e3982u);
        return owner_.units.unit_heading_radians(index_);
    }
    float owner_radius_09c8() override {
        owner_.done("ShipAiPlanner::owner_radius_09c8", 0x009e3adbu);
        return owner_.units.unit_hull_length_09c8(index_);
    }
    float owner_class_max_speed_0500() override {
        owner_.done("ShipAiPlanner::owner_class_max_speed", 0x009e3af0u);
        return owner_.units.unit_class_max_speed_0500(index_);
    }
    void release_node_list_vtable0(bsp::ShipAiPathNode*) override {
        owner_.record_slot("ShipAiPlanner::release_node_list", "00d214f4+vtable00");
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// Milestone 2q: bsp::ShipAiPathSearchHost, the nine call sites of 009EC680's
// five routines. The process geometry adapter now supplies retained authored
// scene paths, native corner storage, exact crossing/clearance dependencies,
// and actual Map bounds. Full native scene creators/singleton ABI and the
// manager's draft-layer tail remain outside that adapter; see GAME_AVOID_ZONE_RUNTIME.md.
class PathSearchBinding final : public bsp::ShipAiPathSearchHost {
public:
    PathSearchBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl)
        : owner_(owner), ctl_(ctl) {}

    bsp::ShipAiPathSearchTurnRamp game_settings_turn_ramp_00424c40() override {
        if (!owner_.path_turn_ramp_loaded) throw std::logic_error("Path turn ramp is not loaded");
        owner_.done("ShipAiSearch::turn_ramp", 0x00424c40u);
        return owner_.path_turn_ramp;
    }
    std::uint32_t avoid_zone_manager_004218e0() override {
        owner_.done("ShipAiSearch::avoid_zone_manager", 0x004218e0u);
        return owner_.zones.manager_handle();
    }
    bool segment_blocked_00417e90(std::uint32_t, std::uint32_t layer,
                                  const std::array<float, 2>& toward, const std::array<float, 2>& from,
                                  std::uint32_t& out_zone,
                                  std::int32_t& out_edge_index) override {
        owner_.done("ShipAiSearch::segment_blocked", 0x00417e90u);
        return owner_.zones.segment(layer, toward, from, out_zone, out_edge_index);
    }
    std::int32_t zone_detour_corners_00422500(std::uint32_t zone, const std::array<float, 2>& far_point,
                                              std::int32_t edge, std::int32_t near_hint, std::int32_t side_hint, float margin,
                                              std::array<float, 2>& left, std::array<float, 2>& right,
                                              std::int32_t& out_left_index,
                                              std::int32_t& out_right_index) override {
        owner_.done("ShipAiSearch::zone_detour_corners", 0x00422500u);
        const auto hit = owner_.zones.detour(zone, far_point, edge, near_hint, side_hint, margin);
        if (hit.has_backward) { left = hit.backward_point; out_left_index = hit.backward_index; }
        if (hit.has_forward) { right = hit.forward_point; out_right_index = hit.forward_index; }
        return hit.side_code;
    }
    bool point_outside_world_bounds_0071c4f0(const std::array<float, 3>& point) override {
        owner_.done("ShipAiSearch::point_outside_world_bounds", 0x0071c4f0u);
        return owner_.zones.outside(point);
    }
    bsp::ShipAiPathNode* allocate_path_node_00bf681b(std::size_t) override {
        ctl_.plan_nodes.emplace_back();
        owner_.done("ShipAiSearch::allocate_path_node", 0x00bf681bu);
        return &ctl_.plan_nodes.back();
    }
    std::uint32_t zone_corner_record_00417610(std::uint32_t zone, std::int32_t index) override {
        owner_.done("ShipAiSearch::zone_corner_record", 0x00417610u);
        return owner_.zones.corner(zone, index);
    }
    void ensure_zone_corner_metric_00423190(std::uint32_t zone, std::uint32_t corner) override {
        owner_.done("ShipAiSearch::zone_corner_metric", 0x00423190u);
        owner_.zones.ensure_clearance(zone, corner);
    }
    void release_path_node_vtable0(bsp::ShipAiPathNode*) override {
        // The nodes live in the controller's own deque, which the plan links by
        // address, so nothing is freed here. The same boundary the planner's
        // own release records.
        owner_.record_slot("ShipAiSearch::release_path_node", "00d214f4+vtable00");
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
};

// Milestone 2q: bsp::ShipAiPathPointHost, the four call sites of 009E3C00 the
// walk reaches.
// Milestone 2r: bsp::ShipAiPathFollowerHost. Packet cc_ai_path_follower read
// 009E3C00-009E432A whole, including the corner arm 009E3F1A..009E4222 that
// milestone 2q's projection stopped at, so the executable now runs that
// reconstruction instead and its host has the follower's nine call sites.
class PathFollowerBinding final : public bsp::ShipAiPathFollowerHost {
public:
    PathFollowerBinding(GameShipAiHost::Impl& owner, GameShipAiRow& row, std::size_t index)
        : owner_(owner), row_(row), index_(index) {}

    const bsp::ShipAiPathLateralAnchor* lateral_anchor_node_10(std::uint32_t handle)
        override {
        owner_.done("ShipAiPathFollower::lateral_anchor_node_10", 0x009e3d8cu);
        return owner_.zones.anchor(handle);
    }
    float order_turn_limit_at_00811d80(const std::array<float, 2>& xz) override {
        // 009E3DC1: unit+A98h+54h*index is the opposite, published slot.
        const auto& order = owner_.controllers[index_].order;
        const float limit = bsp::unit_ai_order_turn_limit_at_00811d80(
            order.slots[1 - order.index], xz);
        owner_.done("ShipAiPathFollower::order_turn_limit_00811d80", 0x00811d80u);
        return limit;
    }
    float owner_class_turn_radius_0082e850() override {
        // 009E3EAE, 0082E850 on [[plan+3Ch]+538h]: class+520h, which 00828F20
        // derives from MaxSpeed and MaxRotAngle. Milestone 2q recorded this
        // because no packet had read the deriver; packet ship_ai_class_field_0524
        // has, and the units host answers with the derived field.
        const float radius = kShipTurnRadiusSitesBound
            ? owner_.class_turn_radius_0082e850(index_)
            : owner_.units.unit_class_turn_radius_0520(index_);
        owner_.done("ShipAiPathFollower::class_turn_radius_0082e850", 0x0082e850u);
        return radius;
    }
    float owner_radius_09c8() override {
        // 009E3EC0, [plan+3Ch]+9C8h. Its producers are 0081106E and 0081FA4D,
        // and with no model box the answer is the descriptor's `Length`.
        const float length = owner_.units.unit_hull_length_09c8(index_);
        owner_.done("ShipAiPathFollower::owner_length_09c8", 0x009e3ec0u);
        return length;
    }
    std::uint32_t avoid_zone_manager_004218e0() override {
        owner_.done("ShipAiPathFollower::avoid_zone_manager", 0x004218e0u);
        return owner_.zones.manager_handle();
    }
    bool segment_hits_zone_00417ef0(std::uint32_t manager, std::uint32_t zone_layer,
                                    const std::array<float, 2>& from,
                                    const std::array<float, 2>& to,
                                    std::array<float, 2>& hit) override {
        static_cast<void>(manager);
        owner_.done("ShipAiPathFollower::segment_hits_zone", 0x00417ef0u);
        return owner_.zones.segment_point(zone_layer, from, to, hit);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// 009E46F0's path arm, forward-declared above ApproachPointBinding.
//
// The image tests [blk+2F4h]+1Ch at 009E46FC and takes the path arm when it is
// non-zero: 009D5AE0 and 009D5B90 fill nested+11D4h / +11D5h, 009E3C00 walks the
// plan for the next point, and the caller stores the bearing to it. The other
// arm, 009E4707, uses the command entity at blk+3FCh and 0071EB60's target
// descriptor; this host does not answer it and records the address instead.
bool ship_ai_arc_centre_next_point_009e46f0(GameShipAiHost::Impl& owner,
                                            GameShipAiHost::Impl::Controller& ctl,
                                            std::size_t index, float& x, float& z) {
    if (index >= owner.rows.size()) return false;
    bsp::ShipAiPathPlanBlock& live = (ctl.plan_front == 0) ? ctl.plan_a : ctl.plan_b;
    float unit_x = 0.0f, unit_y = 0.0f, unit_z = 0.0f;
    owner.units.unit_position_00fc(index, unit_x, unit_y, unit_z);
    if (live.node_count == 0) {
        // 009E4707's arm: no path plan, so the source is the active command's
        // own target descriptor. 009E4754 0071EB60 on the object
        // [blk+3FCh]->vtable[114h]() returns it; 009E4759 tests descriptor+1h
        // and 009E475F takes descriptor+8h, its position, or else the global
        // vector at 00F87574, which the image ships as twelve zero bytes, so
        // that arm is the world origin and not an unknown.
        bsp::SceneCommandTarget descriptor{};
        int mode = 0;
        if (!owner.units.active_command_descriptor_0071eb60(index, descriptor, mode)) {
            owner.record("ShipAiApproachPoint::arc_centre_no_command", 0x009e4726u);
            return false;
        }
        owner.done("ShipAiApproachPoint::arc_centre_command_arm", 0x009e4707u);
        const float src_x = descriptor.position_valid != 0 ? descriptor.position[0] : 0.0f;
        const float src_z = descriptor.position_valid != 0 ? descriptor.position[2] : 0.0f;
        // 009E477A and 009E4788: the descriptor's position comes first.
        x = src_x;
        z = src_z;
        return true;
    }
    bsp::ShipAiPathPointRecord record{};
    // 009E47F3..009E480D: the query is the unit's own world x and z, the pair
    // 009F1BC0 handed 009E46F0.
    record.query_x_00 = unit_x;
    record.query_z_04 = unit_z;
    PathFollowerBinding point(owner, owner.rows[index], index);
    const bsp::ShipAiPathFollowerResult result =
        bsp::ship_ai_path_follower_point_009e3c00(live, record, point);
    static_cast<void>(result);
    // The image does not test this: it subtracts whatever 009E3C00 left in the
    // record. Refusing on an unwritten point is a deliberate divergence, since
    // the alternative here is a bearing computed from a zeroed output field.
    if (record.node_18 == 0) return false;
    x = record.point_x_08;
    z = record.point_z_0c;
    return true;
}

// Milestone 2r: bsp::ShipAiThrottleProfileHost, the call sites of 009E04E0.
// The contact-track list is empty in this process, so the routine's list walk
// does nothing, the avoidance vector blk+34Ch/+350h is cleared every step and
// the 65-bin profile keeps the bypass byte 009E435F set. That is the run's own
// state: 009E4653 clears blk+400h and nothing in this process appends a track.
class ThrottleProfileBinding final : public bsp::ShipAiThrottleProfileHost {
public:
    ThrottleProfileBinding(GameShipAiHost::Impl& owner,
                           GameShipAiHost::Impl::Controller& ctl, std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    float hull_heading_vtable50() override {
        owner_.done("ShipAiThrottleProfile::hull_heading_vtable50", 0x009e0509u);
        return owner_.units.unit_heading_radians(index_);
    }
    bool track_range_within_target_488(int index) override {
        if (!kShipTorpedoResponseBound) {
            owner_.record("ShipAiThrottleProfile::track_range_488", 0x009e0613u);
            return false;
        }
        // 009E0606..009E0613: skip while the torpedo's run time +488h is below
        // the track's observation time +10h (JB).
        owner_.done("ShipAiThrottleProfile::track_range_488", 0x009e0613u);
        const bsp::ShipAiTorpedoTrack& t = ctl_.torpedo_tracks[static_cast<std::size_t>(index)];
        const GameGunneryHost::LiveTorpedo* live = owner_.live_torpedo(t.key);
        if (live == nullptr) return true;   // 009E0604: a null +48h skips the test
        return !(live->swim_seconds < t.track.range_10);
    }
    bool avoidance_active_009da1d0() override {
        if (!kShipTorpedoResponseBound) {
            // 009E061B. 009DA1D0 needs the unit's gameplay byte +240h, which has no
            // producer here; blk+3ECh is the byte `stop` writes.
            owner_.record("ShipAiThrottleProfile::avoidance_active_009da1d0", 0x009da1d0u);
            return false;
        }
        owner_.done("ShipAiThrottleProfile::avoidance_active_009da1d0", 0x009da1d0u);
        const bool open = owner_.torpedo_gate_009da1d0(index_, ctl_);
        if (open) ++owner_.rows[index_].torpedo_gate_open;
        return open;
    }
    bool refresh_track_009dc060(int index) override {
        if (!kShipTorpedoResponseBound) {
            owner_.record("ShipAiThrottleProfile::refresh_track_009dc060", 0x009dc060u);
            return false;
        }
        owner_.done("ShipAiThrottleProfile::refresh_track_009dc060", 0x009dc060u);
        bsp::ShipAiTorpedoTrack& t = ctl_.torpedo_tracks[static_cast<std::size_t>(index)];
        const GameGunneryHost::LiveTorpedo* live = owner_.live_torpedo(t.key);
        if (live == nullptr) return bsp::ship_ai_refresh_torpedo_track_009dc060(t, nullptr);
        const bsp::ShipAiTorpedoCandidate c = GameShipAiHost::Impl::torpedo_candidate(*live);
        return bsp::ship_ai_refresh_torpedo_track_009dc060(t, &c);
    }
    float class_length_three_quarters_00811a30() override {
        const float radius = owner_.units.unit_class_turn_circle_radius_0082e960(index_,
            bsp::kShipAiContactGapLengthArg);
        owner_.done("ShipAiThrottleProfile::class_length_00811a30", 0x00811a30u);
        return radius;
    }
    int track_count_400() override {
        owner_.done("ShipAiThrottleProfile::track_count_400", 0x009e05c0u);
        return kShipTorpedoResponseBound ? static_cast<int>(ctl_.torpedo_tracks.size()) : 0;
    }
    bsp::ShipAiContactTrack& track_at(int index) override {
        if (!kShipTorpedoResponseBound) {
            owner_.record("ShipAiThrottleProfile::track_at", 0x009e05c7u);
            return ctl_.track_scratch;
        }
        owner_.done("ShipAiThrottleProfile::track_at", 0x009e05c7u);
        return ctl_.torpedo_tracks[static_cast<std::size_t>(index)].track;
    }
    void destroy_track(int index) override {
        if (!kShipTorpedoResponseBound) {
            owner_.record("ShipAiThrottleProfile::destroy_track", 0x009e0fbdu);
            return;
        }
        owner_.done("ShipAiThrottleProfile::destroy_track", 0x009e0fbdu);
        bsp::ship_ai_destroy_track_009e0fbd(ctl_.torpedo_tracks, static_cast<std::size_t>(index));
    }
    bool track_has_source(int index) override {
        if (!kShipTorpedoResponseBound) {
            owner_.record("ShipAiThrottleProfile::track_has_source", 0x009e05efu);
            return false;
        }
        // 009E05EF: +48h (or +60h) still non-null. The observer clears +48h when
        // the torpedo entity goes.
        owner_.done("ShipAiThrottleProfile::track_has_source", 0x009e05efu);
        bsp::ShipAiTorpedoTrack& t = ctl_.torpedo_tracks[static_cast<std::size_t>(index)];
        if (owner_.live_torpedo(t.key) == nullptr) t.source_live = false;
        return t.source_live;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// Milestone 2r: bsp::ShipAiClearanceHost, the fifteen call sites of 009EF910.
// Packet cc_ai_clearance_profile read the routine whole; the executable runs it
// from inside 009F4D10 at 009F4D87. Every neighbour and zone test answers "no
// such thing" here, and that is the run's own state rather than a stand-in: the
// neighbour count blk+604h is the zero 009E4659 wrote and no world candidate
// producer is bound. Static-zone tests borrow the live selected-list head;
// they retain the 009EF96F sentinel while that list remains empty.
class ClearanceBinding final : public bsp::ShipAiClearanceHost {
public:
    ClearanceBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}

    float hull_heading_vtable50() override {
        owner_.done("ShipAiClearance::hull_heading_vtable50", 0x009ef97cu);
        return owner_.units.unit_heading_radians(index_);
    }
    bool obstacle_category_enabled_009ec770(int category) override {
        GameDirectorAvoidance director;
        if (!owner_.units.director_avoidance(index_, director))
            throw std::logic_error("Ship clearance has no live director");
        owner_.done("ShipAiClearance::category_enabled_009ec770", 0x009ec770u);
        return bsp::ship_ai_avoidance_party_accepted_009ec770(
            owner_.controllers[index_].avoidance.side_filter_3f8, category,
            director.ship, owner_.avoid_all_ship_collision());
    }
    bool avoidance_globally_enabled_0080e160_242() override {
        GameDirectorAvoidance director;
        if (!owner_.units.director_avoidance(index_, director))
            throw std::logic_error("Ship clearance has no live director");
        owner_.done("ShipAiClearance::avoidance_enabled_0080e160", 0x0080e160u);
        return director.land;
    }
    bool static_zone_blocks_009d57e0(float x, float z, float radius,
                                     float start, float end) override {
        const auto& search = *owner_.controllers[index_].avoid_search;
        owner_.done("ShipAiClearance::static_zone_blocks_009d57e0", 0x009d57e0u);
        // The original wrapper tests searcher+0, then passes searcher+18 to
        // 00415970. Its caller owns the temporary end-bearing slot.
        return search.cache(0).enabled
            && owner_.zones.search_arc(search.list(0), {x, z}, radius, start, end);
    }
    float static_zone_clearance_00415d70(float x, float z, float radius,
                                         float ax, float az, float bx, float bz) override {
        const auto& list = owner_.controllers[index_].avoid_search->list(0);
        owner_.done("ShipAiClearance::static_zone_clearance_00415d70", 0x00415d70u);
        return owner_.zones.search_clearance(list, {x, z}, radius, {ax, az}, {bx, bz});
    }
    int neighbour_count_604() override {
        owner_.done("ShipAiClearance::neighbour_count_604", 0x009efd5bu);
        return 0;
    }
    bool neighbour_owner_present_14(int) override {
        owner_.record("ShipAiClearance::neighbour_owner_14", 0x009efd80u);
        return false;
    }
    bool neighbour_owner_gone_5e(int) override {
        owner_.record("ShipAiClearance::neighbour_owner_5e", 0x009efd8du);
        return true;
    }
    int neighbour_owner_category_54(int) override {
        owner_.record("ShipAiClearance::neighbour_owner_54", 0x009efd9du);
        return -1;
    }
    bool neighbour_blocks_sweep_009dd010(int, float, float, float, float, float) override {
        owner_.record("ShipAiClearance::neighbour_blocks_sweep_009dd010", 0x009dd010u);
        return false;
    }
    void neighbour_support_point_009d8860(int, float, float, float& out_x,
                                          float& out_z) override {
        owner_.record("ShipAiClearance::neighbour_support_009d8860", 0x009d8860u);
        out_x = 0.0f;
        out_z = 0.0f;
    }
    void neighbour_closest_point_009d8a30(int, float, float, float& out_x,
                                          float& out_z) override {
        owner_.record("ShipAiClearance::neighbour_closest_009d8a30", 0x009d8a30u);
        out_x = 0.0f;
        out_z = 0.0f;
    }
    float neighbour_speed_0092d730(int) override {
        owner_.record("ShipAiClearance::neighbour_speed_0092d730", 0x009eff7fu);
        return 0.0f;
    }
    bool path_fade_applies_00778890() override {
        // 009F0000 / 009F0019: the group-leader query and the vtable identity
        // test against 00E08F80, the `moveonpath` command object. Neither has a
        // producer here; the same pair is recorded in the approach binding.
        owner_.record("ShipAiClearance::path_fade_00778890", 0x00778890u);
        return false;
    }
    float class_length_unit_00811a30() override {
        // 009F0038, 00811A30(unit, 1.0f): 0082E960(class, 1.0f) divided by the
        // gameplay modifier product for channel 5, which is 1.0f in this
        // process. The turn-circle half is now a recovered value, so only the
        // divide is a record.
        const float radius = owner_.units.unit_class_turn_circle_radius_0082e960(index_,
            1.0f);
        owner_.done("ShipAiClearance::class_length_unit_00811a30", 0x00811a30u);
        return radius;
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

// Milestone 2q: bsp::ShipAiNavHost, the two call sites of 009EE671's block.
class NavArmBinding final : public bsp::ShipAiNavHost {
public:
    NavArmBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                  std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}

    float remaining_path_length_009d9e50() override {
        // 009EE6AC, 009D9E50([nav+2F4h])(&pose). Packet ship_ai_path_planner
        // projected it; with the plan now filled it walks real nodes.
        float x = 0.0f, y = 0.0f, z = 0.0f;
        owner_.units.unit_position_00fc(index_, x, y, z);
        bsp::ShipAiPathPlanBlock& live
            = (ctl_.plan_front == 0) ? ctl_.plan_a : ctl_.plan_b;
        const float length = bsp::ship_ai_path_remaining_length_009d9e50(
            live, std::array<float, 2>{x, z});
        owner_.done("ShipAiNav::remaining_path_length", 0x009d9e50u);
        return length;
    }

    float unit_heading_vtable_0050() override {
        owner_.done("ShipAiNav::unit_heading", 0x009ee8c7u);
        return owner_.units.unit_heading_radians(index_);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

// Milestone 2q: bsp::ShipAiArmTailHost, the ten call sites of 009EEAAB..
// 009EF226. Packet ship_ai_navigation_arm_tail landed on main at 4491d04f
// during this packet's turn and was merged in before validation, so the tail is
// a projection rather than the record milestone 2p left.
// Packet cc9_avoid_zone_escape: bsp::ShipAiLayerSelectionHost, the call sites of 009ECA20.
class LayerSelectionBinding final : public bsp::ShipAiLayerSelectionHost {
public:
    LayerSelectionBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                          std::size_t index)
        : owner_(owner), ctl_(ctl), index_(index) {}
    bool has_owner_3fc() override { return true; }
    bool unit_group_leads_00778890() override {
        owner_.done("ShipAiLayer::unit_leads_00778890", 0x00778890u);
        const std::int32_t group = owner_.units.unit_formation_group_0284(index_);
        return group >= 0 && owner_.units.formation_leader_0014(group) == index_;
    }
    std::int32_t group_navigation_layer_0070e450() override {
        // 0070E450: the largest member->vtable[214h]() over the members that
        // answer vtable[5Ch](6), from 0.
        owner_.done("ShipAiLayer::group_layer_0070e450", 0x0070e450u);
        const std::int32_t group = owner_.units.unit_formation_group_0284(index_);
        std::uint32_t best = 0;
        const std::int32_t count = owner_.units.formation_member_count(group);
        for (std::int32_t i = 0; i < count; ++i) {
            const std::size_t member = owner_.units.formation_member_unit(group, i);
            if (member >= owner_.controllers.size()) continue;
            if (!owner_.units.unit_is_kind_of(member, 6)) continue;
            const auto& other = owner_.controllers[member];
            if (!other.leaf_tuning_loaded) continue;
            best = std::max(best, bsp::ship_ai_unit_navigation_layer_006dfd80(other.leaf_tuning));
        }
        return static_cast<std::int32_t>(best);
    }
    std::uint32_t unit_navigation_layer_v214() override {
        owner_.done("ShipAiLayer::unit_layer_006dfd80", 0x006dfd80u);
        return bsp::ship_ai_unit_navigation_layer_006dfd80(ctl_.leaf_tuning);
    }
    bool unit_is_kind_v5c(std::uint32_t kind) override {
        return owner_.units.unit_is_kind_of(index_, static_cast<int>(kind));
    }
    std::uint8_t call_unit_v10c() override {
        // Reached only for kind 0Ch (LandingShip), whose vtable+10Ch is 0042BB40:
        // XOR AL,AL; RET.
        owner_.done("ShipAiLayer::landing_ship_v10c", 0x0042bb40u);
        return 0;
    }
    std::uint32_t class_navigation_floor_0560() override {
        return ctl_.leaf_tuning.array[0];   // [[unit+538h]+560h]
    }
    const bsp::AvoidZoneTable& manager_004218e0() override {
        owner_.done("ShipAiLayer::avoid_zone_manager", 0x004218e0u);
        return owner_.zones.table();
    }
    bsp::AvoidZoneClearanceGroupView native_group(const bsp::AvoidZoneLayerGroup& g) override {
        return owner_.zones.native_group(g);
    }
    bsp::ShipAiLayerTimingView settings_00424c40() override {
        const auto& t = owner_.layer_timing;
        return bsp::ShipAiLayerTimingView{t[0], t[1], t[2], t[3], t[4], t[5]};
    }
    float uniform_float_00bd2f10(std::uint32_t, float low, float high) override {
        // Stream 1 at all three reseed sites. Under BSP_GUNNERY_RNG_STREAMS=1 the
        // key is (unit | 800000h), so these draws never move the unit's torpedo
        // draws; by default both go to the shared generator in call order.
        if (owner_.gunnery_draws == nullptr) return low;
        return owner_.gunnery_draws->ship_ai_draw(index_ | 0x800000u, low, high);
    }
    float unit_forward_speed_0092d730() override {
        return owner_.units.unit_forward_speed_0092d730(index_);
    }
    float unit_reference_speed_0080fc30() override {
        return ctl_.obstacle.reference_speed_3c4;
    }
    float vector_length_00414c60(const std::array<float, 2>& v) override {
        return bsp::length_2d_00414c60(v);
    }
private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    std::size_t index_;
};

class ArmTailBinding final : public bsp::ShipAiArmTailHost {
public:
    ArmTailBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}

    bsp::ShipAiNavPose normalize_004192e0_009eeb63(float x, float z) override {
        // 004192E0(&out, &in): 00419260 answers 1/sqrt(x*x + z*z), and +0.0f
        // for a zero-length vector, and both components are scaled by it.
        owner_.done("ShipAiArmTail::normalize", 0x004192e0u);
        const double square = static_cast<double>(x) * x + static_cast<double>(z) * z;
        bsp::ShipAiNavPose out{};
        if (square <= 0.0) return out;
        const float scale = static_cast<float>(1.0 / std::sqrt(square));
        out.x = x * scale;
        out.z = z * scale;
        return out;
    }
    int neighbour_list_count_009eeb8b() override {
        // [[00E188A8]+19CCh]+60h. construct_world 004DE610 is a load record, so
        // there is no world entity list to walk, the same boundary the obstacle
        // sector scan and the world-bounds box hit.
        owner_.record("ShipAiArmTail::neighbour_list_count", 0x009eeb8bu);
        return 0;
    }
    bsp::ShipAiArmTailEntity list_element_009dbbc0(int) override {
        owner_.record("ShipAiArmTail::list_element", 0x009dbbc0u);
        return nullptr;
    }
    bool entity_is_kind_009eebc8(bsp::ShipAiArmTailEntity, int) override {
        owner_.record_slot("ShipAiArmTail::entity_is_kind", "00cfc3d0+vtable5c");
        return false;
    }
    bsp::ShipAiArmTailEntity own_unit_009eebd2() override {
        owner_.done("ShipAiArmTail::own_unit", 0x009eebd2u);
        return reinterpret_cast<bsp::ShipAiArmTailEntity>(
            static_cast<std::uintptr_t>(index_) + 1u);
    }
    float entity_hull_radius_009eebe0(bsp::ShipAiArmTailEntity) override {
        owner_.record("ShipAiArmTail::entity_hull_radius", 0x009eebe0u);
        return 0.0f;
    }
    bsp::ShipAiNavPose entity_position_00427eb0_009eec41(
        bsp::ShipAiArmTailEntity) override {
        owner_.record("ShipAiArmTail::entity_position", 0x00427eb0u);
        return bsp::ShipAiNavPose{};
    }
    float unit_heading_vtable_0050_009ef0d6() override {
        owner_.done("ShipAiArmTail::unit_heading", 0x009ef0d6u);
        return owner_.units.unit_heading_radians(index_);
    }
    float ship_class_turn_radius_0082e850_009ef112() override {
        // class+520h, whose only writer 00828F20 is not reconstructed. It only
        // widens the astern test's distance threshold, so a zero answer makes a
        // ship decline to reverse rather than invent a reversal.
        if (kShipTurnRadiusSitesBound) {
            owner_.done("ShipAiArmTail::class_turn_radius", 0x0082e850u);
            return owner_.class_turn_radius_0082e850(index_);
        }
        owner_.record("ShipAiArmTail::class_turn_radius", 0x0082e850u);
        return 0.0f;
    }
    void after_arm_009de5b0(float) override {
        if (kShipTorpedoResponseBound || kShipAvoidZoneEscapeBound) {
            owner_.torpedo_override_009de8f1(index_, owner_.controllers[index_],
                                             owner_.rows[index_]);
        }
        owner_.record("ShipAiArmTail::after_arm", 0x009de5b0u);
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

class PathPickBinding final : public bsp::ShipAiPathPickHost {
public:
    PathPickBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                    GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}
    void refresh_path_plan_009ed3e0(float seconds) override {
        // 009EE5C2, 009ED3E0(nav)(seconds). Milestone 2q projects the arm
        // 009ED4E4..009ED69E out of that body: the two 009E3780 revalidation
        // sites, the two 009EC680 search ticks and the front/back swap. The
        // head 009ED3E0..009ED4E2, which builds the two corridor widths from
        // the unit's group, is not projected, so both widths are the literal
        // 20.0f at 00CE3930 that 009ED3E3 seeds them with.
        owner_.done("ShipAiPath::refresh_plan_009ed4e4", 0x009ed4e4u);
        owner_.record("ShipAiPath::refresh_plan_head", 0x009ed3e0u);
        ++row_.path_plan_refreshes;
        ++owner_.summary.path_plan_refreshes;

        ctl_.plan_a.owner = &ctl_;
        ctl_.plan_b.owner = &ctl_;
        float pose_x = 0.0f, pose_y = 0.0f, pose_z = 0.0f;
        owner_.units.unit_position_00fc(index_, pose_x, pose_y, pose_z);

        bsp::ShipAiPathRefreshState state{};
        state.in_use = (ctl_.plan_front == 0) ? &ctl_.plan_a : &ctl_.plan_b;
        state.computing = (ctl_.plan_front == 0) ? &ctl_.plan_b : &ctl_.plan_a;
        state.computing_flag_2fc = ctl_.plan_computing_2fc;

        PathPlannerBinding planner(owner_, ctl_, index_);
        PathSearchBinding search(owner_, ctl_);
        const bsp::ShipAiPathRefreshResult result = bsp::ship_ai_path_refresh_arm_009ed4e4(
            state, seconds,
            std::array<float, 2>{pose_x, pose_z},
            std::array<float, 2>{ctl_.goal.goal_x_1dc, ctl_.goal.goal_z_1e0},
            kShipPlannerTravelLayerBound ? ctl_.travel_layer_30c : state.in_use->zone_layer, 0.0f,
            bsp::kShipAiPathCorridorWidthDefault, bsp::kShipAiPathCorridorWidthDefault,
            planner, search);
        owner_.done("ShipAiPath::plan_009e3780", 0x009e3780u);
        owner_.done("ShipAiPath::search_step_009ec680", 0x009ec680u);
        owner_.done("ShipAiPath::corridor_width_009d9de0", 0x009d9de0u);

        ctl_.plan_computing_2fc = state.computing_flag_2fc;
        ctl_.plan_front = (state.in_use == &ctl_.plan_a) ? 0 : 1;
        if (result.seeded_fresh || result.reseeded_back) {
            ++row_.path_plan_seeds;
            ++owner_.summary.path_plan_seeds;
        }
        if (result.front_accepted) {
            ++row_.path_plan_accepts;
            ++owner_.summary.path_plan_accepts;
        }
        if (result.searched_back || result.searched_front) {
            ++row_.path_search_ticks;
            ++owner_.summary.path_search_ticks;
        }
        if (result.swapped) {
            ++row_.path_plan_swaps;
            ++owner_.summary.path_plan_swaps;
        }
        const bsp::ShipAiPathPlanBlock& live = *state.in_use;
        row_.path_plan_state = live.search_state;
        row_.path_plan_nodes = live.node_count;
    }
    void next_path_point_009e3c00(bsp::ShipAiPathPointRecord& record) override {
        // 009EE5F4, 009E3C00([nav+2F4h])(&record). Milestone 2r runs packet
        // cc_ai_path_follower's whole-body projection in place of milestone
        // 2q's partial one, so the corner arm 009E3F1A..009E4222, the shortcut
        // test and the cursor advance at 009E421F are code rather than records.
        // On a two-node open-sea plan the walk does not run, the target is the
        // goal node and the published point is the goal itself.
        bsp::ShipAiPathPlanBlock& live
            = (ctl_.plan_front == 0) ? ctl_.plan_a : ctl_.plan_b;
        PathFollowerBinding point(owner_, row_, index_);
        const bsp::ShipAiPathFollowerResult result
            = bsp::ship_ai_path_follower_point_009e3c00(live, record, point);
        owner_.done("ShipAiPath::next_point_009e3c00", 0x009e3c00u);
        if (result.exit == bsp::ShipAiPathFollowerExit::StraightAtPoint
            || result.exit == bsp::ShipAiPathFollowerExit::CornerTangent) {
            ++row_.path_points;
            ++owner_.summary.path_points;
            ++owner_.summary.path_follower_points;
            row_.path_point_x = record.point_x_08;
            row_.path_point_z = record.point_z_0c;
        }
        if (result.exit == bsp::ShipAiPathFollowerExit::CornerTangent) {
            ++row_.path_corner_arms;
            ++owner_.summary.path_corner_arms;
            ++owner_.summary.path_follower_corners;
        }
        if (result.advanced_cursor) ++owner_.summary.path_follower_advances;
    }
    float path_width_2f4_08() override {
        // Read the front after refresh_path_plan may have swapped it.
        const bsp::ShipAiPathPlanBlock& live
            = (ctl_.plan_front == 0) ? ctl_.plan_a : ctl_.plan_b;
        owner_.done("ShipAiPath::path_width_2f4", 0x009ee61eu);
        return live.published_width;
    }
    void publish_lateral_offset_00815f30(std::uint32_t node, int direction,
        float low, float high) override {
        // node_18 is the corner record handle; 009EE66B passes its x/z.
        const auto* anchor = owner_.zones.anchor(node);
        bsp::unit_ai_order_push_turn_limit_00815f30(
            ctl_.order.slots[ctl_.order.index], {anchor->x, anchor->z},
            direction, low, high);
        owner_.done("ShipAiPath::publish_lateral_offset", 0x00815f30u);
        ++row_.path_publishes;
        ++owner_.summary.path_publishes;
    }
    float path_node_width_20(std::uint32_t node) override {
        const float clearance = owner_.zones.corner_clearance(node);
        owner_.done("ShipAiPath::node_width", 0x009ee63au);
        return clearance;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::ShipAiControllerHost, the sixteen steps of 009F50E0
// ---------------------------------------------------------------------------

class AvoidSearchBinding final : public bsp::ShipAiAvoidZoneSearcherHost {
public:
    AvoidSearchBinding(GameShipAiHost::Impl& owner,
        GameShipAiHost::Impl::Controller& controller, std::size_t index)
        : owner_(owner), controller_(controller), index_(index) {}
    bool director_land_avoidance_0080e160_242() override {
        GameDirectorAvoidance flags;
        if (!owner_.units.director_avoidance(index_, flags))
            throw std::logic_error("Ship avoid search has no live director");
        owner_.done("ShipAiAvoidSearch::director_land", 0x0080e160u);
        return flags.land;
    }
    void avoid_zone_segment_list_clear_004158a0(std::size_t index) override {
        owner_.zones.clear_search(controller_.avoid_search->list(index));
        ++owner_.avoidance_clears;
        owner_.done("ShipAiAvoidSearch::clear", 0x004158a0u);
    }
    void avoid_zone_query_refresh_009d7050(std::size_t index,
        const bsp::ShipAiAvoidZoneQuery& query) override {
        auto& storage = *controller_.avoid_search;
        if (owner_.zones.refresh_search(storage.cache(index), storage.list(index), query))
            ++owner_.avoidance_refills;
        ++owner_.avoidance_queries;
        owner_.done("ShipAiAvoidSearch::query_refresh", 0x009d7050u);
    }
private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& controller_;
    std::size_t index_;
};

class ControllerBinding final : public bsp::ShipAiControllerHost {
public:
    ControllerBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                      GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    bool unit_present_0b00() override { return true; }           // 009F50E4
    bool unit_flag_005d() override {
        // 009F50F2. bsp/unit_instance.hpp names unit+5Dh `simulate`; milestone
        // 2i holds it clear for a live ship, which is what the eight
        // local-player unit lists of docs/LOCAL_PLAYER_UNIT_LISTS.md require.
        return owner_.units.unit_flag_005d(index_);
    }
    bool unit_flag_0061() override {
        // 009F50FC. The same byte 008266C1 reads to choose the autopilot pair.
        // docs/UNIT_AUTOPILOT_PAIR.md scanned `.text` for a writer and found
        // none outside the constructor, so it is clear here and the AI runs.
        return owner_.units.unit_flag_0061(index_);
    }
    bool sync_state_009f3dd0() override {
        SyncBinding sync(owner_, ctl_, row_, index_);
        const bool changed = bsp::ship_ai_sync_state_to_command_009f3dd0(sync);
        owner_.done("ShipAi::sync_state_to_command", 0x009f3dd0u);
        return changed;
    }
    void pre_step_009e0270(bool changed) override {
        owner_.run_hull_pre_step(ctl_, index_, ctl_.nav_block,
            static_cast<std::uint32_t>(changed));
    }
    void replan_prepare_009f1420(float elapsed) override {
        // 009F516C, the brain pre-pass, once per AI sub-tick with the
        // accumulated delta. Milestone 2o recorded this call site; packet
        // cc_ai_goal_vector read 009F1420-009F158A and this runs it, so the
        // goal vector at brain+0B2Ch..+0B34h is written from the unit's own
        // active command rather than left at the zeroes the block carries.
        // 009F158A..009F1BB8, the two further countdowns at brain+0B44h /
        // +0B4Ch and the proximity scan they drive, is not projected and is
        // recorded with its own address.
        GoalVectorBinding goal(owner_, ctl_, index_);
        // Packet cc9_station_keeping: 009F145E, MOV byte [brain+3ADh],0, beside the
        // 0B38h clear the goal refresh models. brain+3ADh is blk+3A5h.
        if (kShipStationKeepingBound) ctl_.blk.flag_3a5 = false;
        const bsp::ShipAiGoalRefreshResult result
            = bsp::ship_ai_refresh_goal_vector_009f1420(ctl_.goal_vector, ctl_.latched,
                                                        elapsed, goal);
        owner_.done("ShipAi::replan_prepare", 0x009f1420u);
        if (kShipTorpedoResponseBound) {
            owner_.torpedo_walk_009f158a(index_, ctl_, row_, elapsed);
            owner_.done("ShipAi::replan_prepare_threat_scan", 0x009f158au);
        } else {
            owner_.record("ShipAi::replan_prepare_threat_scan", 0x009f158au);
        }
        const auto request = bsp::ship_ai_avoidance_request_prepass_009f1b7b(
            owner_.avoid_all_ship_collision());
        ctl_.avoidance = request.request;
        ctl_.blk.early_out_3f5 = request.early_out_3f5;
        owner_.done("ShipAi::avoidance_request_prepass", 0x009f1b7bu);
        ++row_.goal_prepasses;
        ++owner_.summary.goal_prepasses;
        if (result.goal_rewritten) {
            ++row_.goal_refreshes;
            ++owner_.summary.goal_refreshes;
        }
        // Packet cc8_ship_ai_goal_vector_visibility: the gate the ring scan
        // stops at, counted where 009F1420 leaves it.
        if (result.timer_expired) ++row_.goal_timer_expiries;
        if (ctl_.goal_vector.target_visible_0b28) ++row_.goal_visible_true;
        row_.brain_goal_x = ctl_.goal_vector.goal_x_0b2c;
        row_.brain_goal_y = ctl_.goal_vector.goal_y_0b30;
        row_.brain_goal_z = ctl_.goal_vector.goal_z_0b34;
        row_.brain_target = ctl_.goal_vector.raw_target_0b20;
        row_.brain_target_name.clear();
        if (ctl_.goal_vector.raw_target_0b20 != 0u) {
            const GameUnitRow* target = owner_.units.unit_row(
                static_cast<std::size_t>(ctl_.goal_vector.raw_target_0b20 - 1u));
            if (target != nullptr) row_.brain_target_name = target->name;
        }
        {
            // The descriptor as it stands on this sub-tick, not as it stood on
            // the first one: a unit's slot 0 changes when a later command is
            // pushed, and the goal below is the value that descriptor produced.
            bsp::SceneCommandTarget descriptor{};
            int mode = 0;
            const bool real = owner_.units.active_command_descriptor_0071eb60(index_,
                descriptor, mode);
            char text[96];
            if (!real) {
                std::snprintf(text, sizeof(text), "mode=%d empty singleton 00e19b98", mode);
            } else if (descriptor.position_valid != 0) {
                std::snprintf(text, sizeof(text), "mode=%d position (%.1f, %.1f)", mode,
                    static_cast<double>(descriptor.position[0]),
                    static_cast<double>(descriptor.position[2]));
            } else {
                std::snprintf(text, sizeof(text), "mode=%d object id=%u", mode,
                    static_cast<unsigned>(descriptor.object_id));
            }
            row_.command_descriptor = text;
        }
    }
    void state_step_vtable0c(float elapsed) override {
        ++row_.replans;
        ++owner_.summary.replans;
        const StateDescriptor* state = state_for_ai_offset(ctl_.active_state_ai_offset);
        if (state != nullptr && state->step_concrete) {
            SetterBinding setters(owner_);
            bsp::ShipAiCruiseAvoidanceInputs inputs;
            if (!owner_.cruise_avoidance_inputs(index_, inputs)) {
                ++owner_.avoidance_role_unavailable;
                owner_.record("ShipAiCruise::unavailable_owner_roles", 0x009e119fu);
                ++owner_.summary.state_steps_recorded;
                return;
            }
            ++owner_.avoidance_role_reads;
            if (kShipTorpedoResponseBound) {
                // Packet cc9_ship_torpedo_response. 009E1170 selects its arm with
                // no test of the director's command, so for a controlled unit
                // (forced into cruise at 009F3DF3) arm 2's request store at
                // 009E11D6 happens whatever the director holds. The commands
                // host's cruise_step returns before it when the director holds
                // another command (attackmove), leaving blk+3ECh at the
                // pre-pass 1, which opened 009DA1D0 for the player's ship.
                bsp::ShipAiCruiseAvoidanceInputs live = inputs;
                live.unit_player_controlled = owner_.units.unit_player_controlled_0184(index_);
                if (bsp::ship_ai_cruise_step_arm_009e11a5(live)
                        != bsp::ShipAiCruiseAvoidanceArm::CruiseRule) {
                    bsp::ShipAiAvoidanceRequestBlock block{ctl_.avoidance, ctl_.blk.early_out_3f5};
                    bsp::ship_ai_cruise_step_request_009e11d6(block, live);
                    ctl_.avoidance = block.request;
                    ctl_.blk.early_out_3f5 = block.early_out_3f5;
                }
            }
            const bool drove = owner_.units.run_cruise_state_step_009e1170(
                index_, ctl_.blk, setters, ctl_.avoidance, inputs);
            row_.avoidance_enabled = ctl_.avoidance.enable_3f4;
            row_.avoidance_side = ctl_.avoidance.side_filter_3f8;
            if (drove) {
                ++owner_.summary.state_steps_concrete;
                ++row_.state_step_real;
                ++owner_.summary.state_steps_real;
                apply_ai_drive();
                return;
            }
            // Request fields are shared even when the selected player/helm
            // drive remainder is still partial. Commands records that boundary.
            ++owner_.summary.state_steps_recorded;
            return;
        }
        // Milestone 2o, second pass: four of the eight leaves now have a body.
        // Packet ship_ai_state_steps projected 009E14C0 `stop`, 009E5770
        // `movetopos` and 009E8820 `attackmove` with its selector 009E86F0
        // complete, and packet cc8_ship_moveonpath added 009E59C0 `moveonpath`,
        // so those run here instead of being recorded. `follow`, `land`,
        // `kamikaze_attack` and `sub_attack` are still records with their own
        // addresses.
        if (state != nullptr && state->step == 0x009e14c0u) {
            StopStepBinding stop(owner_, ctl_, row_, index_);
            bsp::ship_ai_stop_step_009e14c0(ctl_.stop_state, ctl_.blk, ctl_.avoidance, stop);
            owner_.done("ShipAiState::stop_step", 0x009e14c0u);
            row_.avoidance_enabled = ctl_.avoidance.enable_3f4;
            row_.avoidance_side = ctl_.avoidance.side_filter_3f8;
            ++owner_.summary.state_steps_concrete;
            ++row_.state_step_real;
            ++owner_.summary.state_steps_real;
            apply_ai_drive();
            return;
        }
        if (state != nullptr && state->step == 0x009e5770u) {
            MoveToPosStepBinding move(owner_, ctl_, row_, index_);
            bsp::ship_ai_movetopos_step_009e5770(move);
            owner_.done("ShipAiState::movetopos_step", 0x009e5770u);
            ++owner_.summary.state_steps_concrete;
            ++row_.state_step_real;
            ++owner_.summary.state_steps_real;
            apply_ai_drive();
            return;
        }
        if (state != nullptr && state->step == 0x009e1610u) {
            // Packet cc8_ship_follow. 009E1610's three gates are the formation at
            // unit+284h, its leader at +14h and leader->vtable[5Ch](6); the step
            // runs only for a unit that is actually a follower.
            const std::int32_t group = owner_.units.unit_formation_group_0284(index_);
            const std::size_t leader = owner_.units.formation_leader_0014(group);
            if (group >= 0 && leader != static_cast<std::size_t>(-1)
                && leader != index_) {
                FollowStepBinding follow(owner_, ctl_, row_, index_, leader);
                bsp::ship_ai_follow_step_009e1610(ctl_.follow_state, follow);
                float x = 0.0f;
                float y = 0.0f;
                float z = 0.0f;
                owner_.units.unit_position_00fc(index_, x, y, z);
                const float dx = follow.station_x() - x;
                const float dz = follow.station_z() - z;
                ctl_.follow_station_error = std::sqrt(dx * dx + dz * dz);
                if (ctl_.follow_station_error > ctl_.follow_station_error_max) {
                    ctl_.follow_station_error_max = ctl_.follow_station_error;
                }
                ++ctl_.follow_steps;
                ctl_.follow_ran = true;
                owner_.done("ShipAiState::follow_step", 0x009e1610u);
                ++owner_.summary.state_steps_concrete;
                ++row_.state_step_real;
                ++owner_.summary.state_steps_real;
                apply_ai_drive();
            }
            return;
        }
        if (state != nullptr && state->step == 0x009e59c0u) {
            MoveOnPathStepBinding move(owner_, ctl_, row_, index_);
            bsp::ship_ai_moveonpath_step_009e59c0(move);
            owner_.done("ShipAiState::moveonpath_step", 0x009e59c0u);
            ++owner_.summary.state_steps_concrete;
            ++row_.state_step_real;
            ++owner_.summary.state_steps_real;
            apply_ai_drive();
            return;
        }
        if (state != nullptr && state->step == 0x009e8820u) {
            AttackMoveStepBinding attack(owner_, ctl_, row_, index_);
            bsp::ship_ai_attackmove_step_009e8820(elapsed, attack);
            owner_.done("ShipAiState::attackmove_step", 0x009e8820u);
            ++owner_.summary.state_steps_concrete;
            ++row_.state_step_real;
            ++owner_.summary.state_steps_real;
            apply_ai_drive();
            return;
        }
        // Every remaining leaf's step was named by docs/SHIP_AI_STATES.md as a
        // vtable slot and its body was not read, so the step is a record with
        // its own address. `sub_attack`'s vtable was not read at all.
        ++owner_.summary.state_steps_recorded;
        apply_ai_drive();
        if (state == nullptr || state->step == 0u) {
            owner_.record_slot("ShipAiState::step_vtable0c", "00d21598+vtable0c");
            return;
        }
        char method[64];
        std::snprintf(method, sizeof(method), "ShipAiState::%s_step", state->name);
        owner_.record(method, state->step);
        static_cast<void>(elapsed);
    }
    // --ai-drive was milestone 2o's labelled diagnostic stand-in for the state
    // steps that produced no desired throttle. Milestone 2r retires it: all
    // four states of this mission now form their own pair.
    void apply_ai_drive() {
        // Milestone 2r: nothing. The switch is retired; see set_ai_drive.
    }

    float state_interval_vtable28() override {
        const StateDescriptor* state = state_for_ai_offset(ctl_.active_state_ai_offset);
        const bool cruise = state != nullptr && state->interval == kCruiseIntervalGetter;
        owner_.done("ShipAiState::replan_interval_vtable28",
            cruise ? kCruiseIntervalGetter : kSharedIntervalGetter);
        const float ticks = cruise ? kCruiseIntervalTicks : kSharedIntervalTicks;
        row_.replan_interval = ticks * bsp::kShipAiIntervalScale;
        return ticks;
    }
    void replan_finish_009ddbc0() override {
        owner_.record("ShipAi::replan_finish", 0x009ddbc0u);
    }
    void hold_009da0d0() override { owner_.record("ShipAi::hold", 0x009da0d0u); }
    void step_009eca20(float seconds) override {
        if (!kShipAvoidZoneEscapeBound || !ctl_.leaf_tuning_loaded || !owner_.zones.ready()) {
            owner_.record("ShipAi::step_009eca20", 0x009eca20u);
            return;
        }
        // 009F51C6, chain slot 12: 009ECA20(nav)(seconds).
        auto& nb = ctl_.nav_block;
        bsp::ShipAiLayerSelectionView view{
            nb.random_phase_148, nb.value_14c, ctl_.escape_x_150, ctl_.escape_z_154,
            nb.deadline_158, nb.deadline_15c, nb.flag_160, nb.value_164,
            ctl_.class_floor_16c, nb.value_170, ctl_.requested_layer_308,
            ctl_.travel_layer_30c, ctl_.goal_layer_310, ctl_.goal.crossing_314,
            ctl_.last_goal_x_31c, ctl_.last_goal_z_320, ctl_.hull_geometry, ctl_.blk.mode,
            ctl_.goal.goal_x_1dc, ctl_.goal.goal_z_1e0, nb.look_ahead_floor_318,
            ctl_.obstacle.reference_speed_3c4, ctl_.nav.look_ahead_max_3c8};
        LayerSelectionBinding binding(owner_, ctl_, index_);
        bsp::ship_ai_select_navigation_layer_009eca20(view, seconds, binding);
        owner_.done("ShipAi::step_009eca20", 0x009eca20u);
        ++row_.layer_selections;
        if (nb.flag_160) ++row_.zone_inside_steps;
        row_.travel_layer_min = std::min(row_.travel_layer_min, ctl_.travel_layer_30c);
        row_.travel_layer_max = std::max(row_.travel_layer_max, ctl_.travel_layer_30c);
    }
    void step_009da6e0(float) override {
        bsp::ShipAiAvoidZoneSearcherInputs inputs;
        inputs.hull_x = ctl_.hull_geometry.position_184[0];
        inputs.hull_z = ctl_.hull_geometry.position_184[1];
        inputs.look_ahead_3c8 = ctl_.nav.look_ahead_max_3c8;
        inputs.layer_key_168 = static_cast<std::int32_t>(ctl_.nav_block.class_reference_168);
        AvoidSearchBinding binding(owner_, ctl_, index_);
        bsp::ship_ai_refresh_avoid_zone_searchers_009da6e0(
            ctl_.avoid_search->cache_views(), ctl_.avoidance, inputs, binding);
        owner_.done("ShipAi::refresh_avoid_zone_searchers", 0x009da6e0u);
    }
    void step_009f0ea0(float seconds) override {
        // 009F51E4, chain slot 14, one slot before the sector refresh, so the
        // list the scan walks is aged and compacted first. Packet
        // cc_ai_sector_scan projected the pass; it runs here over the list this
        // process holds, which nothing appends to.
        const bsp::ShipAiNeighbourRefreshResult refresh
            = bsp::ship_ai_neighbour_list_refresh_009f0ea0(ctl_.neighbours, seconds,
                                                           ctl_.neighbours_expired);
        owner_.done("ShipAi::neighbour_list_refresh_009f0ea0", 0x009f0ea0u);
        // 009F1A25, the candidate walk's call into 009F0D20. The candidates come
        // from the world object's linked list at [[00E188A8]+19CCh], which
        // construct_world 004DE610 does not build here, and the node's own
        // footprint at +44h..+60h has no producer in this packet either.
        owner_.record("ShipAi::neighbour_list_add_009f0d20", 0x009f0d20u);
        ctl_.nav_block.neighbour_count_604 = refresh.survivors;
    }
    void step_009e04e0(float seconds) override {
        // 009F51F3, the chain slot that builds the 65-bin throttle profile at
        // blk+4h and the avoidance vector at blk+34Ch/+350h. Packet
        // cc_ai_clearance_profile projected it; the contact-track list at
        // blk+400h is the empty one 009E4653 left, so the walk finds nothing
        // and the profile keeps the bypass byte the constructor set at
        // 009E435F, which is what makes 009D6B40 a clamp and nothing more.
        bsp::ShipAiThrottleProfileInputs in{};
        in.reference_speed_3c4 = ctl_.obstacle.reference_speed_3c4;
        in.own_speed = owner_.units.unit_forward_speed_0092d730(index_);
        in.acceleration = owner_.units.unit_class_max_accel_0504(index_);
        in.hull_half_width = owner_.units.unit_half_width_09cc(index_);
        in.hull_beam = owner_.units.unit_hull_length_09c8(index_);
        const auto& hull = ctl_.hull_geometry;
        in.position_x = hull.position_184[0];
        in.position_z = hull.position_184[1];
        in.forward_x = hull.forward_1ac[0];
        in.forward_z = hull.forward_1ac[1];
        in.normal_x = hull.beam_19c[0];
        in.normal_z = hull.beam_19c[1];
        ThrottleProfileBinding profile(owner_, ctl_, index_);
        bsp::ship_ai_build_throttle_profile_009e04e0(ctl_.throttle_profile, in, seconds,
                                                     profile);
        owner_.done("ShipAi::build_throttle_profile_009e04e0", 0x009e04e0u);
        if (kShipTorpedoResponseBound && (ctl_.throttle_profile.avoid_x_34c != 0.0f
                || ctl_.throttle_profile.avoid_z_350 != 0.0f)) {
            ++row_.torpedo_vector_steps;
        }
        // The profile the drive's middle snaps the throttle with, and the
        // bypass byte that decides whether it does anything at all.
        ctl_.obstacle.profile = ctl_.throttle_profile.profile;
        ++row_.throttle_profiles;
        ++owner_.summary.throttle_profiles;
    }
    void step_009ef230() override {
        // 009F51FA, chain slot 15, one slot before the throttle ceiling at
        // 009F5248 whose tail runs 009F3F80, so the twelve sectors at blk+808h
        // are always one call fresh when the drive's middle reads them.
        ObstacleSectorRefresh::run(owner_, ctl_, index_);
        ++row_.sector_refreshes;
        ++owner_.summary.sector_refreshes;
    }
    bool navigate_009ed6b0(float seconds) override {
        DirectControlBinding binding(owner_, index_);
        const bool early_out
            = bsp::ship_ai_direct_control_arm_009ed6b0(ctl_.blk, seconds, binding);
        owner_.done("ShipAi::direct_control_arm", 0x009ed6b0u);
        // 009ED769..009ED779, three stores inside the per-step reset span that
        // src/ship_ai_states.cpp's projection of the arm does not model (it
        // covers 009ED74D..009ED780 and keeps only blk+330h). blk+2FDh and
        // blk+2FEh are cleared at the top of every tick, which is what makes
        // 009EF034 the only thing that can set the arrival latch for a
        // Navigate ship, and blk+340h is re-seeded from blk+3C8h.
        ctl_.arm_tail_ran = false;
        ctl_.tail.parked_2fd = false;        // 009ED772
        ctl_.tail.goal_reached_2fe = false;  // 009ED779
        ctl_.goal.flag_2fd = false;
        ctl_.goal.flag_2fe = false;
        ctl_.nav.look_ahead_340 = ctl_.nav.look_ahead_max_3c8;  // 009ED769
        owner_.done("ShipAi::per_tick_arrival_reset", 0x009ed779u);
        if (early_out) return true;
        // Milestone 2p. The rest of 009ED6B0 is two alternatives, not a
        // prologue and a tail (docs/SHIP_AI_GOAL_VECTOR.md, correction 4):
        // 009EDA28..009EE57B is the station-keeping arm and it leaves through
        // 009EE57B JMP 009EF206, so 009EE580 is reached only when that arm did
        // not run.
        if (ctl_.blk.flag_3a5 && !ctl_.flag_3a6) {
            // 009EDA34 / 009EDA41. blk+3A5h is the "another entity's controller
            // owns me" byte; its five writers (009D5B90, 009DA0D0, 009DA3B0,
            // 009DDBC0 and 009DE5B0) are all records here, two of them chain
            // steps of this same frame, so the byte is never set and this arm
            // is never entered. The whole span is one record with its address.
            ++row_.station_keeping;
            ++owner_.summary.station_keeping;
            if (!kShipStationKeepingBound) {
                owner_.record("ShipAi::station_keeping_arm", 0x009eda28u);
                return false;
            }
            // Packet cc9_station_keeping: the whole arm, 009EDA47..009EE57B.
            bsp::ShipAiStationKeepingInputs in;
            in.reference_speed_3c4 = ctl_.obstacle.reference_speed_3c4;
            in.retardation_508 = owner_.units.unit_retardation_0508(index_);
            in.goal_x_1dc = ctl_.goal.goal_x_1dc;
            in.goal_z_1e0 = ctl_.goal.goal_z_1e0;
            in.hull_x_184 = ctl_.hull_geometry.position_184[0];
            in.hull_z_188 = ctl_.hull_geometry.position_184[1];
            in.unit_heading = owner_.units.unit_heading_radians(index_);
            in.unit_length_9c8 = owner_.units.unit_hull_length_09c8(index_);
            in.unit_width_9cc = owner_.units.unit_half_width_09cc(index_);
            in.turn_radius = owner_.class_turn_radius_0082e850(index_);
            bsp::ShipAiStationKeepingState st;
            st.direction_35c = ctl_.blk.direction;
            st.timer_360 = ctl_.blk.timer_360;
            st.direction_value_374 = ctl_.blk.direction_value_374;
            st.direction_counter_384 = ctl_.blk.direction_counter_384;
            st.aligned_388 = ctl_.station_aligned_388;
            st.reversing_389 = ctl_.station_reversing_389;
            st.close_38a = ctl_.station_close_38a;
            st.throttle_39c = ctl_.speed_scale_39c;
            st.heading_target_324 = ctl_.blk.heading_target_324;
            st.distance_32c = ctl_.blk.distance_32c;
            bsp::ship_ai_station_keeping_arm_009eda28(ctl_.station_request, in, st);
            ctl_.blk.direction = st.direction_35c;
            ctl_.blk.timer_360 = st.timer_360;
            ctl_.blk.direction_value_374 = st.direction_value_374;
            ctl_.blk.direction_counter_384 = st.direction_counter_384;
            ctl_.station_aligned_388 = st.aligned_388;
            ctl_.station_reversing_389 = st.reversing_389;
            ctl_.station_close_38a = st.close_38a;
            ctl_.speed_scale_39c = st.throttle_39c;
            ctl_.blk.heading_target_324 = st.heading_target_324;
            ctl_.blk.distance_32c = st.distance_32c;
            owner_.done("ShipAi::station_keeping_arm", 0x009eda28u);
            // 009EE57B JMP 009EF206: the arm ends in 009DE5B0 like every other.
            if (kShipTorpedoResponseBound || kShipAvoidZoneEscapeBound)
                owner_.torpedo_override_009de8f1(index_, ctl_, row_);
            ++row_.station_arm_runs;
            row_.station_throttle_min = std::min(row_.station_throttle_min, st.throttle_39c);
            row_.station_throttle_max = std::max(row_.station_throttle_max, st.throttle_39c);
            return false;   // AL = [ESP+37h] = 0
        }
        // 009EE580..009EE670, projected by packet cc_ai_goal_vector: the path
        // plan refresh, the path point pick and the lateral-offset publish.
        bsp::ShipAiPathPickState pick{};
        pick.steering_mode_1c4 = static_cast<int>(ctl_.blk.mode);
        pick.astern = ctl_.blk.mode == bsp::ShipAiSteeringMode::NavigateAstern;
        float pose_x = 0.0f, pose_y = 0.0f, pose_z = 0.0f;
        owner_.units.unit_position_00fc(index_, pose_x, pose_y, pose_z);
        pick.pose_x_184 = pose_x;
        pick.pose_z_188 = pose_z;
        PathPickBinding path(owner_, ctl_, row_, index_);
        const bsp::ShipAiPathPickResult result = bsp::ship_ai_pick_path_point_009ee580(
            pick, seconds, ctl_.path_point, ctl_.speed_scale_39c, path);
        owner_.done("ShipAi::pick_path_point", 0x009ee580u);
        if (result.entered) {
            ++row_.path_picks;
            ++owner_.summary.path_picks;
        }
        // 009EE671..009EEAA2, the output block src/ship_ai_navigation.cpp
        // projects. Milestone 2p said the block was skipped when the record
        // came back without a node; that reads 009EE609 backwards. `JZ 009EE671`
        // JUMPS INTO the block: the only thing record+18h gates is the lateral
        // publish at 009EE66C. The block therefore runs on every pass the
        // 009EE59B / 009EE59F gate lets through, on whatever point 009E3C00
        // left in the record. A closed gate leaves through 009EE59F JZ 009EF206
        // and reaches neither.
        if (!result.entered) {
            owner_.record("ShipAi::navigation_arm_tail", 0x009eeaabu);
            return false;
        }
        bsp::ShipAiNavWaypoint waypoint{};
        waypoint.x = ctl_.path_point.point_x_08;            // 009EE671
        waypoint.z = ctl_.path_point.point_z_0c;            // 009EE694
        waypoint.next_x = ctl_.path_point.next_x_10;        // 009EE78B
        waypoint.next_z = ctl_.path_point.next_z_14;        // 009EE79D
        waypoint.more_path = ctl_.path_point.more_path_20;  // 009EE776
        waypoint.steer_enabled = ctl_.path_point.steer_enabled_21;  // 009EE801
        waypoint.side = static_cast<bsp::ShipAiNavTurnSide>(
            ctl_.path_point.direction_1c);                  // 009EE765
        bsp::ShipAiNavPose nav_pose{};
        nav_pose.x = pose_x;
        nav_pose.z = pose_z;
        NavArmBinding nav_host(owner_, ctl_, index_);
        const bsp::ShipAiNavResult nav = bsp::ship_ai_navigation_arm_009ee671(
            ctl_.blk, ctl_.nav, waypoint, nav_pose, nav_host);
        owner_.done("ShipAi::navigation_output_block", 0x009ee671u);
        ++row_.nav_output_blocks;
        ++owner_.summary.nav_output_blocks;
        if (nav.bearing_taken) {
            ++row_.nav_bearings;
            ++owner_.summary.nav_bearings;
        }
        row_.nav_distance_32c = nav.distance_to_waypoint;
        row_.nav_heading_324 = nav.heading_target;

        // 009EEAAB..009EF226, packet ship_ai_navigation_arm_tail, merged from
        // main at 4491d04f. It is the only code that latches blk+35Ch for a
        // ship in Navigate, the only writer of the approach ceiling blk+344h on
        // this arm, and the only writer of the arrival latch blk+2FEh after the
        // per-tick clear at 009ED779.
        bsp::ShipAiArmTailTuning tuning{};
        // blk+3C4h, the cached reference speed. The executable already fills it
        // for the obstacle middle from 009EC97B's inputs.
        tuning.reference_speed_3c4 = ctl_.obstacle.reference_speed_3c4;
        // [[blk+3FCh]+538h]+508h, `Retardation` out of the installed
        // VehicleClass row, the same divisor 009ED8EC uses.
        tuning.class_deceleration_508 = owner_.units.unit_retardation_0508(index_);
        owner_.done("ShipAiArmTail::class_deceleration_508", 0x009eed1au);
        // Milestone 2r: blk+3CCh, blk+3D4h, blk+3D8h and blk+604h are what
        // 009E4330 wrote when the brain record was built, not zeroes. The
        // constructor runs once per unit in register_units; 009E4568 and
        // 009E4537 / 009E453F are their only other writer's defaults, and
        // 009F0E69 / 009F114E move +604h every frame, which this process's
        // neighbour list does not, so the count stays the constructor's zero
        // and the traffic setback walk at 009EEAD5 stays shut.
        tuning.turn_distance_3cc = ctl_.nav_block.turn_circle_cruise_3cc;
        tuning.stop_radius_3d4 = ctl_.nav_block.stop_radius_3d4;
        tuning.start_radius_3d8 = ctl_.nav_block.start_radius_3d8;
        tuning.neighbour_count_604 = ctl_.nav_block.neighbour_count_604;
        owner_.done("ShipAiArmTail::nav_tuning_009e4330", 0x009e4330u);
        // [blk+3FCh]+9C8h, the full hull length 0081106E / 0081FA4D produce,
        // and the navigatorParams byte +21h.
        tuning.hull_radius_9c8 = owner_.units.unit_hull_length_09c8(index_);
        owner_.done("ShipAiArmTail::hull_radius_09c8", 0x009ef112u);
        tuning.keep_clear_of_traffic_21 = true;
        ctl_.tail.plan_reset_2fc = ctl_.plan_computing_2fc;
        ctl_.tail.leader_snapshot_3a6 = ctl_.flag_3a6;
        {
            const bsp::ShipAiPathPlanBlock& live
                = (ctl_.plan_front == 0) ? ctl_.plan_a : ctl_.plan_b;
            ctl_.tail.plan_search_state_1c = live.search_state;
        }
        // [ESP+43h] at 009EE6E5 / 009EE6EC: record+20h != 0 && blk+1E4h != 0.
        const bool goal_is_destination
            = waypoint.more_path && ctl_.goal.final_leg_1e4;
        ArmTailBinding tail_host(owner_, index_);
        const bsp::ShipAiArmTailResult tail = bsp::ship_ai_navigation_arm_tail_009eeaab(
            ctl_.blk, ctl_.tail, ctl_.nav, tuning, waypoint,
            bsp::ShipAiNavPose{ctl_.goal.goal_x_1dc, ctl_.goal.goal_z_1e0},
            nav_pose, goal_is_destination, seconds, tail_host);
        owner_.done("ShipAi::navigation_arm_tail", 0x009eeaabu);
        ctl_.arm_tail_ran = true;
        ++row_.arm_tails;
        ++owner_.summary.arm_tails;
        row_.throttle_limit_344 = tail.throttle_ceiling;
        row_.latched_direction_35c = static_cast<int>(tail.direction);
        if (tail.direction != bsp::ShipAiThrottleDirection::Stopped) {
            ++row_.arm_tail_latched;
            ++owner_.summary.arm_tail_latched;
        }
        if (tail.request_stop) {
            ++row_.arm_tail_stops;
            ++owner_.summary.arm_tail_stops;
        }
        if (ctl_.tail.goal_reached_2fe) {
            ctl_.goal.flag_2fe = true;   // 009EF034, the byte 009DA590 requires
            ++row_.arrival_latches;
            ++owner_.summary.arrival_latches;
        }
        ctl_.goal.flag_2fd = ctl_.tail.parked_2fd;   // 009EF045
        return false;
    }
    void publish_009f4d10(float seconds) override {
        PublishBinding publish(owner_, ctl_, index_);
        const bsp::ShipAiPublishResult result = bsp::ship_ai_publish_order_009f4d10(
            ctl_.blk.heading_target_324, ctl_.blk.distance_32c, ctl_.blk.distance_330,
            seconds, publish);
        // 009F4D10 writes +40h/+44h/+48h/+4Ch, preserving lateral memory.
        auto& slot = ctl_.order.slots[result.slot_index];
        slot.distance_40 = result.slot.distance_40;
        slot.heading_44 = result.slot.heading_44;
        slot.distance_48 = result.slot.distance_48;
        slot.valid_4c = result.slot.valid_4c;
        owner_.done("ShipAi::publish_order", 0x009f4d10u);
        ++row_.publishes;
        ++owner_.summary.publishes;
        row_.slot_index = result.slot_index;
        row_.slot_heading_44 = result.slot.heading_44;
        row_.slot_distance_40 = result.slot.distance_40;
        row_.slot_distance_48 = result.slot.distance_48;
        row_.slot_valid = result.slot.valid_4c;
        run_clearance_refresh(seconds);
    }
    void run_clearance_refresh(float seconds) {
        // 009F4D87, the unconditional call 009F4D10 makes into 009EF910.
        // Milestone 2q recorded it and left blk+37Ch at the zero a fresh block
        // carries, which made the danger ramp's ratio zero and pinned every
        // navigating ship at full danger. Packet cc_ai_clearance_profile read
        // the routine whole, so it runs here.
        bsp::ShipAiClearanceGeometry geometry{};
        // blk+1B4h and blk+3CCh, both written by 009E4330 at construction.
        geometry.sweep_half_angle = ctl_.nav_block.shoulder_angle_1b4;
        geometry.hull_radius = ctl_.nav_block.turn_circle_cruise_3cc;
        const auto& hull = ctl_.hull_geometry;
        geometry.shoulder_port_x = hull.shoulder_18c[0];
        geometry.shoulder_port_z = hull.shoulder_18c[1];
        geometry.shoulder_stbd_x = hull.shoulder_194[0];
        geometry.shoulder_stbd_z = hull.shoulder_194[1];
        geometry.forward_x = hull.forward_1ac[0];
        geometry.forward_z = hull.forward_1ac[1];
        bsp::ShipAiClearanceSettings settings{};
        const auto tuning = owner_.avoidance_tuning();
        settings.refresh_period_1d4 = tuning[1];
        settings.error_gate_committed_214 = tuning[3];
        settings.error_gate_free_218 = tuning[4];
        owner_.done("ShipAiClearance::settings_00424c40", 0x009ef948u);
        ctl_.clearance.heading_target_324 = ctl_.blk.heading_target_324;
        ctl_.clearance.path_length_330 = ctl_.blk.distance_330;
        ctl_.clearance.steering_mode_35c = static_cast<int>(ctl_.blk.direction);
        ctl_.clearance.category_3f0 = ctl_.avoidance.side_filter_3f8;
        ctl_.clearance.avoidance_enabled_3f4 = ctl_.avoidance.flag_3fc;
        ctl_.clearance.static_zone_present_a3c = ctl_.avoid_search->list(0).head != nullptr;
        ClearanceBinding clearance(owner_, index_);
        bsp::ship_ai_refresh_turn_clearance_009ef910(ctl_.clearance, geometry, settings,
                                                     seconds, clearance);
        owner_.done("ShipAi::refresh_turn_clearance_009ef910", 0x009ef910u);
        ++row_.clearance_refreshes;
        ++owner_.summary.clearance_refreshes;
        row_.clearance_37c = ctl_.clearance.clearance_37c;
    }
    void tail_009da8d0(float) override { owner_.record("ShipAi::tail_009da8d0", 0x009da8d0u); }
    void tail_009f4da0(float seconds) override {
        // 009F5248, chain slot 16. 009F4DA0's own body is the throttle ceiling
        // on brain+34Ch / +350h, which no packet has reconstructed, so it stays
        // a record with its own address. It is not on the way to anything here:
        // the routine's ONLY exit is the tail call at 009F50C6 with
        // ECX = brain+8h = blk, taken whether or not brain+0B38h skipped the
        // body (009F4DAF), so 009F3F80 runs on every step either way.
        if (kShipFormationSpeedBound) {
            // Packet cc9_ship_formation_speed: the body runs inside the drive, at
            // the point its two fields are read (formation_throttle_ceiling_009f4da0).
            owner_.done("ShipAi::throttle_ceiling", 0x009f4da0u);
        } else {
            owner_.record("ShipAi::throttle_ceiling", 0x009f4da0u);
        }
        owner_.drive_order_ring_009f3f80(index_, ctl_, row_, seconds);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// Milestone 2p: 0071F290, the command controller's own per-frame update
// ---------------------------------------------------------------------------
//
// docs/DIRECTOR_UPDATE_ARMS.md (packet cc2_director_update_arms) reads the
// whole of it: seven arms in order, of which arm 3 is the auto-target hold
// countdown 0071F314 that 0071DF70 tests, and arm 7 ticks
// [controller+38h]->vtable[4](dt), which on a director is 009F5DA0, then
// vtable[7Ch], which is 00836920. Milestone 2n supplied where the think runs;
// this milestone takes that placement from the routine instead.

class ControllerUpdateBinding final : public bsp::CommandControllerUpdateHost {
public:
    ControllerUpdateBinding(GameShipAiHost::Impl& owner,
                            GameShipAiHost::Impl::Controller& ctl, GameShipAiRow& row,
                            std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    void reset_path_vector() override {
        // 0071F2D1..0071F2F3, the shared zero vector at 00F87574 into path
        // object 0's +30h/+34h/+38h. The +1A4h path array is not built here.
        owner_.record("CommandController::reset_path_vector", 0x0071f2d1u);
    }
    bool begin_command(int) override {
        // vtable[78h], which on a director is 00835C70. Milestone 2l already
        // runs that body at its own site, right after the push that made a
        // command current; the two arms here are gated on the accepted bytes
        // at +44h / +4Ch, which this process does not model, so neither arm
        // runs and the call is recorded rather than made twice.
        owner_.record("CommandController::begin_command", 0x00835c70u);
        return true;
    }
    void raise_override_stage(int) override {
        owner_.record("CommandController::raise_override_stage", 0x0071d9e0u);
    }
    void raise_queue_stage(int) override {
        owner_.record("CommandController::raise_queue_stage", 0x0071d810u);
    }
    void step_auto_target(float frame_delta) override;
    void step_commands() override {
        // vtable[7Ch] = 00836920 BSP_WeaponDirector_Step. Milestone 2m runs
        // that body once per unit per fixed step from GameUnitsHost, which is
        // where the stage ladder, the `stop` arm and the idle tail already run.
        // Running it here as well would step every director twice, so the arm
        // is logged at the site it actually runs and not repeated.
        owner_.done("CommandController::step_commands", 0x00836920u);
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

// ---------------------------------------------------------------------------
// bsp::BotFireTargetHost, the sixteen call sites of 009F5DA0
// ---------------------------------------------------------------------------

class TargetBinding final : public bsp::BotFireTargetHost {
public:
    TargetBinding(GameShipAiHost::Impl& owner, GameShipAiHost::Impl::Controller& ctl,
                  GameShipAiRow& row, std::size_t index)
        : owner_(owner), ctl_(ctl), row_(row), index_(index) {}

    bool controller_belongs_to_another(void*) override {
        owner_.record("AutoTarget::controller_belongs_to_another", 0x007788b0u);
        return false;
    }
    void* director_command_slot() override {
        // 009F5DD0, [director+54h]. 0071BE48 establishes that the first command
        // slot holds the command object itself, which is what the follow
        // comparison at 009F5DE0 needs, so this process's own slot 0 answers it.
        owner_.done("AutoTarget::director_command_slot", 0x009f5dd0u);
        const std::uint32_t command = owner_.units.director_current_command_0071be40(index_);
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(command));
    }
    void release_controller(void*, int) override {
        owner_.record("AutoTarget::release_controller", 0x0077c980u);
    }
    bool unit_suppresses_targeting(void*) override {
        owner_.done("AutoTarget::unit_player_controlled", 0x009f5e06u);
        return owner_.units.unit_player_controlled_0184(index_);
    }
    bool selection_enabled() override {
        // 009F5610: [director+3Dh] must be set and, when the slot is non-null,
        // its vtable[0Ch] must not answer 1 or 2. bsp::WeaponDirectorState's
        // allow_move default is the byte 008363E0 writes for a unit of neither
        // gated class, which is every ship of this mission.
        const bsp::WeaponDirectorState defaults{};
        owner_.done("AutoTarget::director_allow_move", 0x009f5614u);
        const std::uint32_t command = owner_.units.director_current_command_0071be40(index_);
        int kind = 0;
        if (command != 0u) {
            owner_.record_slot("AutoTarget::command_slot_kind", "00e08f70+vtable0c");
        }
        const bool enabled = bsp::auto_target_selection_enabled_009f5610(
            defaults.allow_move, command != 0u, kind);
        owner_.done("AutoTarget::selection_enabled", 0x009f5610u);
        return enabled;
    }
    void send_command_state(void*, int) override {
        owner_.record("AutoTarget::send_command_state", 0x0071d9e0u);
    }
    void* director_current_target() override {
        owner_.done("AutoTarget::director_current_target", 0x009f5e2cu);
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(ctl_.fire_target));
    }
    bool director_target_locked() override {
        // [director+23Ch], the target-change gate 00835860 writes. Nothing in
        // this process sets it, and its only writer is the setter itself.
        owner_.record("AutoTarget::director_target_locked", 0x009f5e37u);
        return false;
    }
    int director_command_state() override {
        owner_.record("AutoTarget::director_command_state", 0x009f5e69u);
        return 0;
    }
    void* build_command_target(void* entity, float) override {
        // 00465080, read for this packet: it fills a SceneCommandTarget with the
        // zero vector at 00F87574, kind 1 and the entity's +174h object id, or
        // kind 0 for a null entity. The tick only passes the result on, so the
        // host records the routine and carries the entity through it.
        owner_.record("AutoTarget::build_command_target", 0x00465080u);
        return entity;
    }
    bool command_accepts_target(std::uint32_t, void*) override {
        owner_.record("AutoTarget::command_accepts_target", 0x0071d6d0u);
        return false;
    }
    bsp::AutoTargetScanResult scan_party_list() override {
        ++row_.target_scans;
        ++owner_.summary.scans;
        // 009F65E0 builds the priority list from the owner's own kinds, over the
        // recovered class chain this process already answers IsKindOf with.
        bsp::AutoTargetOwnerKinds kinds{};
        kinds.kind_0e = owner_.units.unit_is_kind_of(index_, 0x0e);
        kinds.kind_07 = owner_.units.unit_is_kind_of(index_, 0x07);
        kinds.kind_0a = owner_.units.unit_is_kind_of(index_, 0x0a);
        kinds.kind_0d = owner_.units.unit_is_kind_of(index_, 0x0d);
        kinds.kind_08 = owner_.units.unit_is_kind_of(index_, 0x08);
        kinds.kind_0b = owner_.units.unit_is_kind_of(index_, 0x0b);
        kinds.kind_09 = owner_.units.unit_is_kind_of(index_, 0x09);
        kinds.kind_0c = owner_.units.unit_is_kind_of(index_, 0x0c);
        const bsp::AutoTargetSearchConfig config
            = bsp::auto_target_build_priority_009f65e0(kinds);
        owner_.done("AutoTarget::build_priority_list", 0x009f65e0u);
        // The list itself is the recon slot 008053C0 returns for the owner's
        // party and the intrusive chain at slot+0DE8h. Nothing in this process
        // fills that chain, so the executable hands the recovered scan the
        // created instances of the opposing party and records the producer.
        owner_.record("AutoTarget::party_recon_slot", 0x008053c0u);
        owner_.record_slot("AutoTarget::candidate_owner_vtable140", "00cfc3d0+vtable140");
        const GameUnitRow* self = owner_.units.unit_row(index_);
        std::vector<bsp::AutoTargetCandidate> candidates;
        std::vector<std::size_t> candidate_index;
        if (self != nullptr) {
            for (std::size_t other = 0; other < owner_.units.count(); ++other) {
                if (other == index_) continue;
                const GameUnitRow* row = owner_.units.unit_row(other);
                if (row == nullptr || !row->active || row->party == self->party) continue;
                bsp::AutoTargetCandidate candidate{};
                candidate.entity = reinterpret_cast<void*>(
                    static_cast<std::uintptr_t>(other + 1));
                candidate.passes_candidate_gate
                    = owner_.units.unit_is_kind_of(other, bsp::kEntityKindCandidateGate);
                candidate.skip_all_entries = owner_.units.unit_flag_005d(other);
                candidate.entity_kind_matches = -1;
                for (std::size_t entry = 0; entry < config.priority.size(); ++entry) {
                    if (owner_.units.unit_is_kind_of(other,
                            config.priority[entry].entity_kind)) {
                        candidate.entity_kind_matches = static_cast<int>(entry);
                        break;
                    }
                }
                const double dx = static_cast<double>(row->position[0]) - self->position[0];
                const double dy = static_cast<double>(row->position[1]) - self->position[1];
                const double dz = static_cast<double>(row->position[2]) - self->position[2];
                candidate.distance
                    = static_cast<float>(std::sqrt(dx * dx + dy * dy + dz * dz));
                candidate.owner_can_engage = bsp::auto_target_owner_can_engage_009f59f0(
                    owner_.units.unit_is_kind_of(other, bsp::kEntityKindNeedsSpecialWeapon),
                    false, {});
                candidates.push_back(candidate);
                candidate_index.push_back(other);
            }
        }
        const bsp::AutoTargetScanResult scan
            = bsp::auto_target_scan_009f5d30(config, candidates);
        owner_.done("AutoTarget::scan_party_list", 0x009f5d30u);
        owner_.done("AutoTarget::score_candidate", 0x009f5b70u);
        if (scan.best != nullptr) {
            const std::size_t chosen
                = static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(scan.best)) - 1;
            const GameUnitRow* row = owner_.units.unit_row(chosen);
            row_.fire_target = row != nullptr ? row->name : std::string();
            row_.fire_target_score = scan.best_score;
        }
        return scan;
    }
    void* resolve_command_target_object() override {
        owner_.record("AutoTarget::resolve_command_target", 0x00521ea0u);
        return nullptr;
    }
    bool director_accepts_new_target() override {
        // Milestone 2p. Packet cc2_director_target_gate read 0071DF70 and its
        // three writers whole (docs/DIRECTOR_TARGET_GATE.md) and corrected
        // milestone 2n in two places, so this is no longer a record.
        //
        // Rule 1, 0071DF70..0071DF7E: the compare against the 0.0f at 00D7A218
        // runs the other way round from what 2n said. 0071DF7C is `JBE` to the
        // slot scan and the fall-through at 0071DF7E is `XOR AL,AL / RET`, so a
        // hold ABOVE 0.0f is the rejection and at most 0.0f passes. The field
        // has three producers: 00720225 stores -1.0f in the command controller
        // base constructor 00720180, which 008363E0 reaches at 00836403 with
        // ECX still the director; 0071F314 does `hold -= dt` only while the
        // value is at or above 0.0f, so the sentinel never moves; and 00817031
        // stores 3.0f in the `cleartarget` arm of 00816E30, which this mission
        // never issues. Every director here therefore holds -1.0f and passes.
        //
        // Rule 2 and 3, 0071DF83..0071DFC2: count the leading occupied slots at
        // director+54h with stride 1Ch, stop at the first null, and reject when
        // any of their commands answers category 1 (gunnery) or 2 (weapon run)
        // from vtable[0Ch]. That is exactly the set 008358D0 forces the fire
        // target for, so a queued attack command owns the target by design.
        const float hold = owner_.units.director_target_hold_0040(index_);
        owner_.done("AutoTarget::director_target_hold_0040", 0x0071df75u);
        row_.director_hold_0040 = hold;
        ++row_.target_gate_tests;
        if (hold > 0.0f) {
            row_.target_blocked = "0071df7e hold armed";
            return false;
        }
        int categories[16] = {};
        const int count = owner_.units.director_leading_slot_categories_0071df83(index_,
            categories, 16);
        owner_.done("AutoTarget::director_slot_categories", 0x0071df83u);
        owner_.done("AutoTarget::command_category_vtable0c", 0x0071dfaeu);
        const std::uint32_t slot0 = owner_.units.director_slot_command(index_, 0);
        row_.slot0_command = owner_.units.command_name_of(slot0);
        row_.slot0_category = (count > 0) ? categories[0] : -1;
        for (int i = 0; i < count; ++i) {
            if (categories[i] != 1 && categories[i] != 2) continue;
            char text[64];
            std::snprintf(text, sizeof(text), "0071dfc2 slot %d category %d", i,
                categories[i]);
            row_.target_blocked = text;
            if (!owner_.logged_accept_gate) {
                owner_.logged_accept_gate = true;
                owner_.log.notef("automatic target selection now stops at 0071dfc2, not at the "
                    "hold: director+40h is the constructed -1.0f and passes, and the slot scan "
                    "rejects because a leading command slot carries category %d, the gunnery "
                    "and weapon-run set 008358d0 forces the fire target for "
                    "(docs/DIRECTOR_TARGET_GATE.md)", categories[i]);
            }
            return false;
        }
        ++owner_.summary.units_accepting_new_target;
        owner_.done("AutoTarget::director_accepts_new_target", 0x0071df70u);
        row_.target_blocked.clear();
        return true;
    }
    void issue_command(std::uint32_t, void*) override {
        owner_.record("AutoTarget::issue_attackmove", 0x0071d980u);
        ++row_.attackmove_issues;
        ++owner_.summary.attackmove_issues;
    }
    void set_fire_target(void* entity, bool) override {
        owner_.record("AutoTarget::set_fire_target", 0x00835860u);
        ctl_.fire_target = static_cast<bsp::NativeHandle>(
            reinterpret_cast<std::uintptr_t>(entity));
        ++row_.fire_target_sets;
        ++owner_.summary.fire_target_sets;
    }
    bool director_allow_move() override {
        const bsp::WeaponDirectorState defaults{};
        return defaults.allow_move;
    }
    int command_slot_kind(void*) override {
        owner_.record_slot("AutoTarget::command_slot_kind", "00e08f70+vtable0c");
        return 0;
    }

private:
    GameShipAiHost::Impl& owner_;
    GameShipAiHost::Impl::Controller& ctl_;
    GameShipAiRow& row_;
    std::size_t index_;
};

}  // namespace

// ---------------------------------------------------------------------------
// Chain slot 16: 009F4DA0 -> 009F3F80, and 009F3F80's tail into the order ring
// ---------------------------------------------------------------------------
//
// COVERAGE. 009F3F80's body is 009F3F80-009F4D06 and this runs three parts of
// it and records the rest:
//   009F3FEB..009F3FF2  the early-out gate on blk+3F5h                projected
//   009F3FF8..009F402E  the ring slot under the write cursor          projected
//   009F4034..009F40C6  the speed, the heading, the astern flip and
//                       the heading error the rudder law is given     projected
//   009F40CA..009F44E3  the ten writers of blk+1D0h                   RECORD
//   009F44E4..009F44FC  the mode gate and the rudder law's store       projected
//   009F4502..009F4B98  the obstacle and reverse-manoeuvre arms,
//                       including the second rudder store at 009F46A0  RECORD
//   009F4B99..009F4D04  the hop                                       projected
// The recorded spans are the ones docs/SHIP_AI_THROTTLE_TO_RING.md read for the
// fields they write and did not project. They can change blk+1D0h and blk+1D4h
// before the hop reads them, so what this executable puts in the ring is the
// hop applied to the state's own desired pair, not to whatever those arms would
// have made of it. That is stated in the milestone and is a boundary, not a
// result.

// Defined here because it builds a TargetBinding, which the anonymous namespace
// above declares after ControllerUpdateBinding.
void ControllerUpdateBinding::step_auto_target(float frame_delta) {
    // 0071F395, [controller+38h]->vtable[4](dt) = 009F5DA0, the bot's own
    // fire-target think. Its countdown is what turns a per-frame call into a
    // once-a-second think.
    const float before = ctl_.target.think_countdown;
    TargetBinding target(owner_, ctl_, row_, index_);
    bsp::auto_target_tick_009f5da0(target, ctl_.target, nullptr, frame_delta);
    owner_.done("AutoTarget::tick", 0x009f5da0u);
    if (!(frame_delta < before)) {
        ++row_.target_thinks;
        ++owner_.summary.thinks;
    }
}

void GameShipAiHost::Impl::run_hull_pre_step(Controller& ctl, std::size_t index,
    bsp::ShipAiNavBlockFields& fields, std::uint32_t raw_argument) {
    if (!ctl.class_depth_loaded)
        throw std::logic_error("ship pre-step requires actual loaded class depth");
    HullGeometryUnitAccess access(units, index, ctl.class_reference_0570);
    auto& profile = ctl.throttle_profile.profile;
    bsp::ShipAiHullPreStepView view{fields.class_reference_168,
        ctl.obstacle.reference_speed_3c4, ctl.hull_geometry,
        fields.hull_scale_3e4, fields.shoulder_offset_1b8, ctl.obstacle.sector,
        fields.flag_3e8, fields.flag_3e9, fields.flag_3ea,
        profile.bin, profile.bypass_41};
    bsp::ship_ai_hull_pre_step_009e0270(view, access, raw_argument,
        application_camera_axes_crt());
    // Existing semantic consumer projection; the persistent profile above
    // remains the producer-owned storage also used by009E04E0.
    ctl.obstacle.profile = profile;
    ++hull_geometry_updates;
    done("ShipAi::hull_geometry_009de2f0", 0x009de2f0u);
    done("ShipAi::pre_step", 0x009e0270u);
}

void GameShipAiHost::Impl::run_navigation_goal_009de050(Controller& ctl, GameShipAiRow& row,
    std::size_t index, float goal_x, float goal_z, bool keep_mode, bool final_leg) {
    // The three fields ShipAiControlBlock and ShipAiGoalPlan both describe, in
    // before 009DE050 runs and out after it: +1C4h the steering mode, +1C8h the
    // throttle hold and +1CCh the requested direction.
    ctl.goal.mode = ctl.blk.mode;
    ctl.goal.throttle_hold_1c8 = ctl.blk.throttle_hold_1c8;
    ctl.goal.requested_direction = ctl.blk.requested_direction;
    // 009DE17C/009DE189 read the actual block fields produced by009DE2F0
    // during the constructor and each controller pre-step.
    static_cast<void>(index);
    done("ShipAiGoal::block_pose_0184", 0x009de17cu);
    ctl.goal.pose_x_184 = ctl.hull_geometry.position_184[0];
    ctl.goal.pose_z_188 = ctl.hull_geometry.position_184[1];

    const float planned_x = ctl.goal.planned_x_1e8;
    const float planned_z = ctl.goal.planned_z_1ec;
    GoalBinding goal(*this, ctl);
    bsp::ship_ai_set_navigation_goal_009de050(ctl.goal, goal_x, goal_z, keep_mode, final_leg,
                                              goal);
    done("ShipAiState::set_navigation_goal", 0x009de050u);

    ctl.blk.mode = ctl.goal.mode;
    ctl.blk.throttle_hold_1c8 = ctl.goal.throttle_hold_1c8;
    ctl.blk.requested_direction = ctl.goal.requested_direction;

    ++row.goal_sets;
    ++summary.goal_sets;
    if (ctl.goal.planned_x_1e8 != planned_x || ctl.goal.planned_z_1ec != planned_z) {
        ++row.goal_replans;
        ++summary.goal_replans;
    }
    row.goal_x = ctl.goal.goal_x_1dc;
    row.goal_z = ctl.goal.goal_z_1e0;
    row.goal_final_leg = ctl.goal.final_leg_1e4;
}

void GameShipAiHost::Impl::drive_order_ring_009f3f80(std::size_t index, Controller& ctl,
    GameShipAiRow& row, float seconds) {
    done("ShipAi::drive_order_ring", 0x009f3f80u);
    // 009F3FEB CMP byte [ESI+3F5h],0 / 009F3FF2 JNZ 009F4D00: the routine's one
    // early out, and it jumps past the hop to the epilogue. blk+3F5h is the
    // byte 009ED6B0 returns on (bsp::ShipAiControlBlock::early_out_3f5) and
    // nothing in this process writes it, so the gate is open. The count is
    // reported so a later reader does not have to take that on trust.
    if (ctl.blk.early_out_3f5) {
        done("ShipAiRing::early_out_3f5", 0x009f3ff2u);
        ++row.ring_gated_3f5;
        ++summary.ring_gated_3f5;
        return;
    }
    done("ShipAiRing::early_out_3f5", 0x009f3ff2u);

    // 009F3FF8..009F402E: two independent loads of [unit+97Ch], each shifted
    // left by 5. 009F400D reads the slot's +83Ch (the rudder) and 009F4025 its
    // +838h (the throttle). This is the slot the hop slews toward and then
    // writes back, and it is NOT the 84-byte AI order slot 009F4D10 published.
    const float previous_throttle = units.unit_ring_write_slot_throttle(index);
    const float previous_rudder = units.unit_ring_write_slot_rudder(index);
    done("ShipAiRing::write_slot_read", 0x009f400du);

    // 009F4034, 0092D730 with ECX = [unit+1018h]. The routine reads the hull's
    // signed forward speed once at its head and uses it for two byte flags.
    const float speed = units.unit_forward_speed_0092d730(index);
    done("ShipAi::drive_speed", 0x0092d730u);
    // 009F4072..009F407B, CALL [[unit]+50h]: the unit's own heading getter.
    float heading = units.unit_heading_radians(index);
    record_slot("ShipAi::drive_heading_vtable50", "00cfc3d0+vtable50");
    // 009F4081 CMP [ESI+35Ch],2 / 009F409E 00438AA0(heading, [00D7A264]): a
    // ship latched astern is steered about the reciprocal of its own heading.
    // 00D7A264 is the 3.14159265f bsp/unit_state_message.hpp names for the
    // MT_SHIP_SYNC heading range, which is the same constant.
    if (ctl.blk.direction == bsp::ShipAiThrottleDirection::Astern) {
        heading = bsp::wrapped_angle_add_00438aa0(heading,
            bsp::kUnitStateMessageHeadingRange);
        done("ShipAi::drive_astern_heading", 0x00438aa0u);
    }
    // 009F40BB, 00438B10(blk+324h, heading): the heading error, the one value
    // the rudder law is driven with.
    const float error = bsp::wrapped_angle_subtract_00438b10(ctl.blk.heading_target_324,
        heading);
    done("ShipAi::drive_heading_error", 0x00438b10u);
    row.heading_error = error;

    // Milestone 2p. 009F40CA..009F4B98, the whole middle, is no longer one
    // record: packet cc_ai_obstacle_tables projected it operation for operation
    // as bsp::ship_ai_drive_order_ring_middle_009f40ca. It owns the four
    // direction booleans, the danger ramp at blk+0A84h, the throttle through
    // 009EC7C0 and the 65-bin profile at blk+4h, the rudder law at 009F44F7
    // with its 009F44E4 gate, the obstacle sectors and the escape manoeuvre.
    bsp::ShipAiObstacleFrame frame{};
    frame.dt = seconds;
    frame.body_axis_speed = speed;
    frame.heading = heading;
    frame.heading_error = error;
    frame.unit_half_width_9cc = units.unit_half_width_09cc(index);
    done("ShipAiObstacle::unit_half_width_09cc", 0x009f4174u);
    bool settings_loaded = false;
    const bsp::ShipAiAutoThrustSettings& settings = units.auto_thrust_settings(settings_loaded);
    if (settings_loaded) {
        done("ShipAiObstacle::auto_thrust_block_00424c40", 0x00424c40u);
    } else {
        record("ShipAiObstacle::auto_thrust_block_00424c40", 0x00424c40u);
    }
    bsp::ShipAiThrottleCeilingInputs ceiling = units.throttle_ceiling_inputs(index);
    //009EC9AB consumes the cached block field, including009E0270's floor.
    ceiling.reference_speed = ctl.obstacle.reference_speed_3c4;
    done("ShipAiObstacle::throttle_ceiling_inputs", 0x009ec97bu);
    //009E0270 already wrote+3C4h with its exact floor. Keep that value for
    //009EC9AB/009F478B instead of replacing it with the raw reference speed.
    ctl.obstacle.position_x = ctl.hull_geometry.position_184[0];
    ctl.obstacle.position_z = ctl.hull_geometry.position_184[1];
    // blk+37Ch, the clearance the danger ramp divides by unit+9CCh. Its only
    // writer is 009EF910, which 009F4D10 called one chain slot earlier and
    // which milestone 2r runs, so the field carries what that routine left.
    ctl.obstacle.clearance_37c = ctl.clearance.clearance_37c;
    done("ShipAiObstacle::clearance_37c_producer", 0x009ef910u);
    // blk+344h and blk+348h. 009F4DBC stores 1.0f into blk+348h unconditionally
    // before the 009F4DC1 early out; blk+344h's own arms inside 009F4DA0,
    // 009F4DC7..009F50BA, are still the record milestone 2o left. Milestone 2q
    // adds the other writer: 009EEF0A, in the navigation arm's tail, which runs
    // earlier in the same tick for a ship in Navigate, so the ceiling the drive
    // caps against is the one the approach computed.
    ctl.obstacle.rudder_limit_348 = 1.0f;
    done("ShipAiObstacle::rudder_limit_348", 0x009f4dbcu);
    if (ctl.arm_tail_ran) {
        ctl.obstacle.throttle_limit_344 = ctl.tail.throttle_ceiling_344;
        done("ShipAiObstacle::throttle_ceiling_344", 0x009eef0au);
    } else {
        ctl.obstacle.throttle_limit_344 = 1.0f;
        if (!kShipFormationSpeedBound) {
            record("ShipAiObstacle::throttle_ceiling_344", 0x009f4dc7u);
        }
    }
    if (kShipFormationSpeedBound) {
        // Packet cc9_ship_formation_speed: 009F4DA0 ran one chain slot earlier and
        // rewrote both fields; 009F4DC7 is its min against brain+0AF0h.
        bool station_arm = false;
        formation_throttle_ceiling_009f4da0(index, ctl, ctl.obstacle.throttle_limit_344,
                                            ctl.obstacle.rudder_limit_348,
                                            ctl.obstacle.escape_enabled_36c, station_arm);
        done("ShipAiObstacle::throttle_ceiling_344", 0x009f4dc7u);
        if (station_arm) {
            ++row.station_limit_steps;
            row.station_limit_min = std::min(row.station_limit_min,
                                             ctl.obstacle.throttle_limit_344);
            row.station_limit_max = std::max(row.station_limit_max,
                                             ctl.obstacle.throttle_limit_344);
        }
        ++row.formation_ceiling_steps;
        if (ctl.obstacle.throttle_limit_344 < 1.0f) ++row.formation_limit_344_below_1;
        row.formation_limit_344_min = std::min(row.formation_limit_344_min,
                                               ctl.obstacle.throttle_limit_344);
        row.formation_limit_348_min = std::min(row.formation_limit_348_min,
                                               ctl.obstacle.rudder_limit_348);
        row.formation_limit_348_max = std::max(row.formation_limit_348_max,
                                               ctl.obstacle.rudder_limit_348);
        const std::int32_t fg = units.unit_formation_group_0284(index);
        row.formation_role = fg < 0 ? "none"
            : (units.formation_leader_0014(fg) == index ? "leader" : "follower");
    }
    ObstacleBinding obstacle(*this, index);
    obstacle.set_direction(ctl.blk.direction);
    const float rudder_before = ctl.blk.desired_rudder;
    bsp::ship_ai_drive_order_ring_middle_009f40ca(ctl.blk, ctl.obstacle, frame, settings,
                                                  ceiling, obstacle);
    done("ShipAi::drive_order_ring_body", 0x009f40cau);
    ++row.middle_runs;
    ++summary.middle_runs;
    row.danger_a84 = ctl.obstacle.danger_a84;
    row.throttle_limit_344 = ctl.obstacle.throttle_limit_344;
    row.turn_assist_load_102c = units.turn_assist_load_102c(index);
    row.rudder_law_calls += obstacle.calls();
    summary.rudder_law_calls += obstacle.calls();
    static_cast<void>(rudder_before);

    // 009F4B99..009F4D04, the hop, reconstructed operation for operation by
    // packet cc_ai_throttle_ring: the deadband that zeroes blk+1D4h, the slew of
    // both desired values toward the slot by at most dt * 1.5, and the two
    // setters that put them back.
    RingHopBinding hop_host(*this, row, index);
    const bsp::ShipAiRingHop hop = bsp::ship_ai_order_ring_hop_009f4b99(ctl.blk,
        previous_throttle, previous_rudder, seconds, hop_host);
    done("ShipAiRing::order_ring_hop", 0x009f4b99u);
    ++row.ring_hops;
    ++summary.ring_hops;
    if (hop.rudder_zeroed) {
        ++row.rudder_deadbands;
        ++summary.rudder_deadbands;
    }
}

// ---------------------------------------------------------------------------

GameShipAiHost::GameShipAiHost(GameHostLog& log, GameUnitsHost& units)
    : impl_(std::make_unique<Impl>(log, units)) {}
GameShipAiHost::~GameShipAiHost() {
    if (impl_ != nullptr) impl_->units.commands().bind_command_target_facts(nullptr);
}

void GameShipAiHost::bind_session_participants(
    const bsp::SessionParticipantPools& owner) noexcept {
    impl_->session_participants = &owner;
}

void GameShipAiHost::load_avoid_zone_geometry(const GameSceneContentsHost& scene,
    GameMissionLuaHost& lua, std::int32_t mode, std::uint8_t forced, std::int32_t session) {
    std::string error;
    if (!lua.read_path_turn_ramp(impl_->path_turn_ramp, error))
        throw std::runtime_error("Path turn-ramp load failed: " + error);
    impl_->path_turn_ramp_loaded = true;
    // MissionFrame registers controllers before this geometry load. Retire
    // borrowed segment pointers while retaining those live cache owners.
    // Epoch invalidation is a process lifetime adapter, not an assertion that
    // the original geometry loader writes these controller cache fields.
    for (auto& controller : impl_->controllers) {
        if (!controller.avoid_search) continue;
        for (std::size_t index = 0; index < 3; ++index) {
            impl_->zones.clear_search(controller.avoid_search->list(index));
            controller.avoid_search->cache(index).layer_key = -1;
        }
    }
    impl_->zones.rebuild(scene, mode, forced, session);
    impl_->log.notef("ship AI path turn ramp knee=%.9g limit=%.9g addon=%.9g",
        impl_->path_turn_ramp.knee_x, impl_->path_turn_ramp.limit_x, impl_->path_turn_ramp.limit_y);
}

namespace {

// Milestone 2r: bsp::ShipAiNavBlockCtorHost, the six call sites of 009E4330.
// 009F1160 builds the brain record once per ship and 009F118D runs this on
// `brain+8h`, so the executable runs it once per created unit at registration,
// before any state object exists.
class NavBlockCtorBinding final : public bsp::ShipAiNavBlockCtorHost {
public:
    NavBlockCtorBinding(GameShipAiHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}

    bsp::ShipAiNavBlockSteeringDefaults seed_steering_009dfcb0() override {
        // 009E43CC with ECX = blk+1C4h. Only the blk+3C4h..+3E4h window of
        // 009DFCB0 is projected; the constructor overwrites all of it except
        // +3DCh and +3E0h, so the window is what this process needs.
        owner_.done("ShipAiNavBlock::seed_steering_009dfcb0", 0x009dfcb0u);
        owner_.record("ShipAiNavBlock::seed_steering_unprojected", 0x009dfcb0u);
        return bsp::ship_ai_nav_block_steering_defaults_009dfcb0();
    }
    bool unit_answers_class_5c(int class_id) override {
        owner_.done("ShipAiNavBlock::unit_class_5c", 0x009e448eu);
        return owner_.units.unit_is_kind_of(index_, class_id);
    }
    float class_turn_circle_radius_0082e960(float throttle_fraction) override {
        return owner_.units.unit_class_turn_circle_radius_0082e960(index_,
            throttle_fraction);
    }
    float sqrt_00bf7030(float value) override {
        const auto* crt = &application_camera_axes_crt();
        float result;
        // Preserve the actual CRT domain/NaN handling at009E45F3.
        __asm {
            fld value
            mov ecx, crt
            call native_crt_sqrt_st0_00bf7030
            fstp result
        }
        owner_.done("ShipAiNavBlock::sqrt_00bf7030", 0x00bf7030u);
        return result;
    }
    float uniform_float_00bd2f10(float low, float high) override {
        // 009E465F. 00BD2F10's random stream has no producer in this process -
        // every other site in this file records it - and 009E4669 stores the
        // negated draw in blk+148h, a field with no traced reader. The low end
        // is taken and the call recorded, which is what the approach point's
        // own draw at 009F1BC0 does.
        owner_.record("ShipAiNavBlock::uniform_00bd2f10", 0x00bd2f10u);
        static_cast<void>(high);
        return low;
    }
    void build_sector_shapes_009e0270(bsp::ShipAiNavBlockFields& fields,
                                      std::uint32_t raw_argument) override {
        auto& ctl = owner_.controllers[index_];
        //009E435F/009E4363 precede this callback. Bind the constructor's
        // actual cleared65 bytes and flag to the persistent profile owner.
        ctl.throttle_profile.profile.bin.fill(0);
        ctl.throttle_profile.profile.bypass_41 = fields.flag_45;
        owner_.run_hull_pre_step(ctl, index_, fields, raw_argument);
        owner_.done("ShipAiNavBlock::build_sector_shapes_009e0270", 0x009e0270u);
    }

private:
    GameShipAiHost::Impl& owner_;
    std::size_t index_;
};

}  // namespace

void GameShipAiHost::register_units(GameMissionLuaHost& lua, std::int32_t session_mode) {
    Impl& host = *impl_;
    host.session_mode = session_mode;
    host.settings_owner = &lua;
    const std::size_t count = host.units.count();
    host.controllers.clear();
    host.controllers.resize(count);
    host.rows.assign(count, GameShipAiRow{});
    for (std::size_t index = 0; index < count; ++index) {
        const GameUnitRow* row = host.units.unit_row(index);
        host.rows[index].unit_index = index;
        host.rows[index].unit = row != nullptr ? row->name : std::string();
        host.rows[index].state = "none";
        // Milestone 2r: 009F118D, the navigation block constructor, once per
        // brain record. Its five turn fields are the inputs the arm tail and
        // the arrival release test read; milestone 2q had them at zero.
        const int kind = host.units.unit_class_id(index);
        host.log.notef("unit hull input unit=%s type_id=%d kind=%d length=%.9g width=%.9g",
            host.rows[index].unit.c_str(), row != nullptr ? row->type_id : -1, kind,
            host.units.unit_hull_length_09c8(index), host.units.unit_half_width_09cc(index));
        if (row != nullptr && row->class_row_found && has_ship_navigation_class(kind)) {
            Impl::Controller& ctl = host.controllers[index];
            ctl.avoid_search = std::make_unique<bsp::ShipAiSearchStorage>(
                host.zones.allocation_access());
            GameShipDepthInput depth{};
            std::string error;
            if (!lua.read_ship_depth_input(row->type_id, session_mode, depth, error))
                throw std::runtime_error("Ship depth load for " + row->name + ": " + error);
            ctl.class_reference_0570 = depth.class_reference_0570;
            ctl.class_depth_loaded = true;
            if (kShipAvoidZoneEscapeBound) {
                GameShipNavigationInput navigation{};
                if (!lua.read_ship_navigation_input(row->type_id, session_mode, navigation, error))
                    throw std::runtime_error("Ship navigation load for " + row->name + ": " + error);
                ctl.leaf_tuning = navigation.tuning;
                ctl.leaf_tuning_loaded = true;
                if (!host.layer_timing_loaded) {
                    if (!lua.read_ship_layer_timing_input(host.layer_timing, error))
                        throw std::runtime_error("Ship layer timing load: " + error);
                    host.layer_timing_loaded = true;
                }
            }
            bsp::ShipAiNavBlockUnitInputs in{};
            in.present = row != nullptr;
            in.handle = static_cast<std::uint32_t>(index) + 1u;
            in.hull_length_09c8 = host.units.unit_hull_length_09c8(index);
            in.ship_class.max_rot_angle_04f8
                = host.units.unit_class_max_rot_angle_04f8(index);
            in.ship_class.max_speed_0500 = host.units.unit_class_max_speed_0500(index);
            in.ship_class.turn_radius_0520 = host.units.unit_class_turn_radius_0520(index);
            in.ship_class.reference_0570 = ctl.class_reference_0570;
            NavBlockCtorBinding ctor(host, index);
            ctl.nav_block = bsp::ship_ai_nav_block_ctor_009e4330(in, ctor);
            ctl.nav_block_built = true;
            host.done("ShipAiNavBlock::construct_009e4330", 0x009e4330u);
            ++host.summary.nav_blocks;
            // The three fields the block carries into the navigation arm's own
            // state: blk+3C8h is the look-ahead ceiling 009ED769 re-seeds
            // blk+340h from every tick, and blk+3D0h the heading window.
            ctl.nav.look_ahead_max_3c8 = ctl.nav_block.turn_circle_full_3c8;
            ctl.nav.look_ahead_340 = ctl.nav_block.look_ahead_340;
            ctl.nav.turn_window_3d0 = ctl.nav_block.yaw_rate_3d0;
            const auto request = bsp::ship_ai_avoidance_request_constructed_009e468b();
            ctl.avoidance = request.request;
            ctl.blk.early_out_3f5 = request.early_out_3f5;
            host.rows[index].avoidance_enabled = ctl.avoidance.enable_3f4;
            host.rows[index].avoidance_side = ctl.avoidance.side_filter_3f8;
            host.rows[index].nav_turn_circle_3c8 = ctl.nav_block.turn_circle_full_3c8;
            host.rows[index].nav_turn_circle_3cc = ctl.nav_block.turn_circle_cruise_3cc;
            host.rows[index].nav_yaw_floor_3d0 = ctl.nav_block.yaw_rate_3d0;
            host.rows[index].nav_stop_radius_3d4 = ctl.nav_block.stop_radius_3d4;
            host.rows[index].nav_start_radius_3d8 = ctl.nav_block.start_radius_3d8;
            host.rows[index].nav_hull_length_9c8 = in.hull_length_09c8;
            host.rows[index].hull_mass_00b0 = host.units.unit_hull_mass_00b0(index);
            host.rows[index].hull_material = host.units.unit_hull_material(index);
            host.log.notef("ship pre-step input unit=%s type_id=%d session=%ld depth=%lu key=%s settings=%04lx source=%04lx reference_speed=%.9g width=%.9g",
                row->name.c_str(), row->type_id, static_cast<long>(session_mode),
                static_cast<unsigned long>(ctl.class_reference_0570), depth.class_key,
                static_cast<unsigned long>(depth.settings_block_offset),
                static_cast<unsigned long>(depth.scalar_source),
                ctl.obstacle.reference_speed_3c4, host.units.unit_half_width_09cc(index));
        } else {
            host.rows[index].state = row != nullptr && row->class_row_found
                ? "not_ship" : "no_class";
        }
        // 009F6A20 seeds the think countdown with the negation of a random draw
        // in [0, 1) so the once-a-second thinks of different directors fall on
        // different frames. This process has no 00BD2F10 on this path, so the
        // phase is the unit's own index spread over the interval and is the
        // executable's value rather than a recovered one.
        host.controllers[index].target.think_countdown
            = -static_cast<float>(index % 20) * 0.05f;
        if (row != nullptr && !host.units.unit_player_controlled_0184(index)) {
            ++host.summary.ai_owned;
        }
    }
    host.summary.units = count;
    host.log.notef("ship navigation controllers: %zu built for actual ship classes among %zu instances; "
        "generic command/director owners remain separate", host.summary.nav_blocks, count);
}

void GameShipAiHost::controller_step(float seconds) {
    Impl& host = *impl_;
    if (host.controllers.empty()) return;
    ++host.steps;
    for (std::size_t index = 0; index < host.controllers.size(); ++index) {
        Impl::Controller& ctl = host.controllers[index];
        GameShipAiRow& row = host.rows[index];
        if (!host.units.unit_active(index)) continue;
        if (ctl.nav_block_built) {
            ControllerBinding binding(host, ctl, row, index);
            const bool ran = bsp::ship_ai_controller_step_009f50e0(ctl.timers, seconds, binding);
            host.done("ShipAi::controller_step", 0x009f50e0u);
            if (ran) {
                ++row.controller_steps;
                ++host.summary.steps;
            } else {
                ++row.gated;
                ++host.summary.gated;
            }
            row.steering_mode = static_cast<int>(ctl.blk.mode);
            row.desired_throttle = ctl.blk.desired_throttle;
            row.desired_rudder = ctl.blk.desired_rudder;
            row.desired_heading = ctl.blk.desired_heading;
            row.latched_direction = static_cast<int>(ctl.blk.direction);
            row.heading_target = ctl.blk.heading_target_324;
            row.distance_32c = ctl.blk.distance_32c;
            row.distance_330 = ctl.blk.distance_330;
            row.distance_finite = std::isfinite(ctl.blk.distance_32c)
                && std::isfinite(ctl.blk.distance_330);
            // Milestone 2o: ring+148h / +14Ch as 00813020 left them on the previous
            // step. The tick runs inside 00825F20, after this pass, so reading them
            // here samples the value the motion actually integrated.
            const float live_a = host.units.unit_ring_current_throttle(index);
            const float live_b = host.units.unit_ring_current_rudder(index);
            if (live_a != ctl.live_throttle || live_b != ctl.live_rudder) {
                ++row.live_pair_changes;
                ++host.summary.live_pair_changes;
            }
            ctl.live_throttle = live_a;
            ctl.live_rudder = live_b;
            row.ring_live_throttle = live_a;
            row.ring_live_rudder = live_b;
            // 009DE050 forces blk+1C4h to Navigate on every state step but
            // `cruise`'s, which is what hands the steering to the unprojected
            // navigation arm rather than to the three setters.
            if (ctl.blk.mode == bsp::ShipAiSteeringMode::Navigate
                || ctl.blk.mode == bsp::ShipAiSteeringMode::NavigateAstern) {
                ++host.summary.navigate_mode_steps;
            }
        }
        // Milestone 2p: 0071F290, the command controller's own per-frame
        // update, instead of a bare call to the think. Packet
        // cc2_director_update_arms read the routine whole, and its arm 7 is
        // where 009F5DA0 runs: [controller+38h]->vtable[4](dt) with the
        // director's vtable 00D21B48, whose +4h is 009F5DA0. So the think's
        // position in the frame is recovered here rather than supplied, and
        // arm 3 is the hold countdown 0071DF70 tests.
        bsp::CommandControllerUpdateState state{};
        // Arm 1's session predicate. The session object is this process's own
        // (docs/GAME_EXECUTABLE.md milestone 2l: 0077C2A0's routing and the
        // queue are records and the executable delivers synchronously), so the
        // four lifecycle bytes are the live combination and the gate is open.
        state.session_present = true;
        state.session_flags.flag_5c = true;
        state.path_object0_present = false;
        state.auto_target_hold = host.units.director_target_hold_0040(index);
        state.mode = 1;
        state.slot0_occupied = host.units.director_slot_command(index, 0) != 0u;
        state.override_command_present = false;
        // director+44h / +4Ch, the accepted bytes vtable[78h] writes. Packet
        // cc9_ship_natives_3 keys +44h on the head command it was written for.
        const std::uint64_t head_key = host.units.commands().director_head_key(index);
        state.queue_accepted = kDirectorBeginCommandBound && head_key != 0u
            && ctl.begun_head_key == head_key;
        state.override_accepted = false;
        // Same actual scene/session mode that selected ShipGlobals depth.
        state.session_mode = host.session_mode;
        host.done("CommandController::session_mode_1fe4", 0x00e188a8u);
        state.auto_target_present = true;
        ControllerUpdateBinding update(host, ctl, row, index);
        const bsp::CommandControllerUpdateTrace trace
            = bsp::run_command_controller_update(state, seconds, update);
        host.done("CommandController::update", 0x0071f290u);
        if (trace.queue_begin_attempted && !trace.queue_terminated) {
            ctl.begun_head_key = head_key;   // 00835C70 set +44h
        }
        ++row.controller_updates;
        ++host.summary.controller_updates;
        row.controller_update_session_gate = trace.session_gate_passed;
    }
}

bool GameShipAiHost::promote_order_00825f2c(std::size_t unit_index) {
    Impl& host = *impl_;
    if (unit_index >= host.controllers.size()) return false;
    Impl::Controller& ctl = host.controllers[unit_index];
    if (!ctl.nav_block_built) return false;
    // The compact helper is a temporary projection, never another owner of
    // scalar order fields. Its only mutations are old.valid and the index.
    bsp::UnitAiOrderPromotion promotion{};
    promotion.index = ctl.order.index;
    for (int i = 0; i != 2; ++i) {
        const auto& slot = ctl.order.slots[i];
        promotion.slots[i] = {slot.distance_40, slot.heading_44,
            slot.distance_48, slot.valid_4c};
    }
    const int previous = ctl.order.index;
    bsp::unit_promote_ai_order_00825f2c(promotion);
    host.done("ShipAiOrder::promote_slot", 0x00825f2cu);
    if (!promotion.promoted) return false;
    ctl.order.slots[previous].valid_4c = promotion.slots[previous].valid_4c;
    ctl.order.index = promotion.index;
    bsp::unit_ai_order_copy_00811d10(ctl.order.slots[ctl.order.index],
        ctl.order.slots[previous]);
    host.done("ShipAiOrder::copy_slot", 0x00811d10u);
    ++host.rows[unit_index].promotions;
    ++host.summary.promotions;
    // 00825F7C..00826D6B. The name of this record is stale. Two packets have
    // now corrected it without renaming the census line mid-run, because the
    // counts stay comparable across runs that way.
    //
    // cc8_ship_ai_ring_winner said, quoting docs/UNIT_AI_ORDER_SLOT_READER.md:
    // "It carries the hop from the ring's own rudder at +984h through 00811890
    // to unit->vtable[50h] (00826C61, 00826C75, 00826CDB)". Packet
    // cc8_ship_ai_rudder_hop has since read 00826C34..00826D69 instruction by
    // instruction and that is not what those three sites are
    // (docs/SHIP_AI_RUDDER_HOP.md):
    //   - the ordered rudder was already applied to the body at 00826B54 by
    //     0092E8C0, forty instructions earlier, and docs/SHIP_MOTION.md has
    //     reconstructed that call since 2026-09-11;
    //   - 00826C75's yaw rate is the THIRD argument of 00810190 at 00826CEE,
    //     the wake-trail sampler, and reaches nothing else;
    //   - unit+1050h, the field vtable slot 50h returns, is WRITTEN here at
    //     00826C56 from atan2 over world row 2, so the heading is an output of
    //     the tick, not a steering input.
    // There is consequently no missing AI-heading-to-rudder hop at this
    // address. What is unimplemented here is the motion tail:
    // bsp/ship_ai_rudder_hop.hpp reconstructs it and names the eight host calls
    // it needs, all of which belong to the ShipMotionHost binding in
    // src/game_hosts_units.cpp.
    //
    // A third correction, packet cc8_ship_ai_heading_to_rudder
    // (docs/SHIP_AI_HEADING_TO_RUDDER.md). The older note said: "the rel32 scan
    // for 00816A40 and 0080DAD0 finds the ring's write cursor filled only from
    // three HUD order routines. On the image's evidence the AI never writes the
    // order ring at all." That is WRONG, and the scan is why: the AI uses
    // neither entry point. 009F3F80 BSP_ShipAi_DriveOrderRing ends with
    // 009F4CE8 0080E190(unit, rudder) and 009F4CFB 0080E170(unit, throttle),
    // five-instruction setters that write slot[ring+144h]+4 and +0 with
    // ECX = [blk+3FCh], the unit. 00813020 then clamps slot[ring+140h] and
    // steps unit+984h toward it, and 00813197 sets the read cursor to the write
    // cursor, so in a single-player session the live pair follows what the AI
    // wrote one tick earlier. Both setters are already reconstructed in
    // src/ship_ai_throttle_ring.cpp and bound as ShipAiRing::set_write_slot_*,
    // which is why this run reports writes=96000 and total_path=41584.83.
    //
    // Readers of slot+40h / +44h / +48h do exist, twelve of them, but the
    // steering does not use them: 009F40BB takes the heading target from
    // blk+324h, not from slot+44h. The published triple is inter-unit state.
    host.record("ShipAiOrder::slot_to_order_ring", 0x00825f7cu);
    if (!host.logged_position) {
        host.logged_position = true;
        host.log.notef("the promoted AI order slot reaches no order ring: 009f4d10 publishes "
            "the heading target and the two distances into unit+0aech - 84*[unit+0b40h], "
            "00825f2c flips the index and 00811d10 copies the slot across. Readers of "
            "slot+40h / +44h / +48h DO exist - twelve sites in "
            "docs/UNIT_AI_ORDER_SLOT_READER.md, which supersedes "
            "docs/UNIT_AUTOPILOT_PAIR.md's negative - and none is on the steering path, "
            "because 009f40bb takes the heading target from blk+324h. The AI DOES write the "
            "order ring, through 0080e190 at 009f4ce8 and 0080e170 at 009f4cfb, which the "
            "old rel32 scan for 00816a40 and 0080dad0 could not see "
            "(docs/SHIP_AI_HEADING_TO_RUDDER.md). The name of this record "
            "is stale for a second reason: 00826c34..00826d69 is the motion TAIL, not an "
            "ai-to-rudder hop. The ordered rudder is applied at 00826b54 by 0092e8c0, "
            "00826c75's yaw rate is only the third argument of the wake sampler 00810190, "
            "and unit+1050h - what vtable slot 50h returns - is written at 00826c56 from "
            "atan2 over world row 2, so the heading is an output of the tick "
            "(docs/SHIP_AI_RUDDER_HOP.md)");
    }
    return true;
}

void GameShipAiHost::set_ai_drive(std::size_t unit_index, float throttle, float rudder) {
    Impl& host = *impl_;
    if (unit_index >= host.controllers.size()) return;
    // Milestone 2r RETIRES the switch. Milestone 2q kept it for `attackmove`
    // alone, the one state of four that still produced no desired throttle,
    // because 009E76D0 and its four slot scorers were records. They are wired
    // now, so every state of this mission forms its own pair and there is
    // nothing left for a stand-in to stand in for. The switch is accepted and
    // ignored rather than removed from the command line, so a script that
    // passes it still runs and the log says what happened.
    host.log.notef("--ai-drive \"%s\" = throttle %.3f, rudder %.3f: RETIRED in milestone "
        "2r and IGNORED. The diagnostic stand-in existed for `attackmove`, whose ring scan "
        "009e76d0, four slot scorers 009e6400 / 009e5da0 / 009e6870 / 009e6640, bearing "
        "commit 009e5e90 and throttle limiter 009e6a90 are all wired now; the six "
        "attackmove ships move under their own commanded heading and throttle",
        host.rows[unit_index].unit.c_str(), static_cast<double>(throttle),
        static_cast<double>(rudder));
}

void GameShipAiHost::bind_gunnery(GameGunneryHost* gunnery) noexcept {
    impl_->gunnery = gunnery;
    impl_->gunnery_draws = gunnery;
    if (kShipTorpedoResponseBound && kShipTorpedoResponseImageTerms && gunnery != nullptr) {
        impl_->seed_brain_draws_009f1160();
        impl_->log.notef("ship ai brain constructor draws: %llu controllers seeded (009F1160's "
            "seven stream-1 draws, made when the draw source binds)", impl_->brain_seed_draws);
    }
    // Packet cc9_target_release: the dead-target facts need the gunnery rows.
    if (kShipAiTargetReleaseBound) {
        impl_->units.commands().bind_command_target_facts(
            gunnery != nullptr ? &impl_->command_target_facts : nullptr);
    }
}

const std::vector<GameShipAiRow>& GameShipAiHost::rows() const noexcept { return impl_->rows; }
const GameShipAiSummary& GameShipAiHost::summary() const noexcept { return impl_->summary; }

void GameShipAiHost::log_sample(unsigned long long step_index, unsigned long long interval) {
    Impl& host = *impl_;
    if (interval == 0 || host.controllers.empty()) return;
    if (step_index % interval != 0) return;
    for (std::size_t index = 0; index < host.rows.size(); ++index) {
        GameShipAiRow& row = host.rows[index];
        char line[320];
        std::snprintf(line, sizeof(line),
            "%-20s state=%-10s mode=%-7s dir=%-7s throttle=%7.3f rudder=%7.3f "
            "heading=%8.4f target=%8.4f d32c=%9.2f d330=%9.2f slot=%d fire=%s",
            row.unit.c_str(), row.state.c_str(), steering_mode_name(row.steering_mode),
            direction_name(row.latched_direction),
            static_cast<double>(row.desired_throttle),
            static_cast<double>(row.desired_rudder),
            static_cast<double>(row.desired_heading),
            static_cast<double>(row.heading_target),
            static_cast<double>(row.distance_32c),
            static_cast<double>(row.distance_330), row.slot_index,
            row.fire_target.empty() ? "-" : row.fire_target.c_str());
        // A sampled line is emitted only when something a reader would act on
        // moved, because 32 ships every ten steps is 1248 lines of the same row.
        if (host.controllers[index].sample == line) continue;
        host.controllers[index].sample = line;
        host.log.notef("  ship ai step %llu  %s", step_index, line);
    }
}

void GameShipAiHost::report() {
    Impl& host = *impl_;
    if (host.rows.empty()) return;
    // Packet cc8_ship_follow: what 009E1610 did for each follower. The error is
    // the distance from the ship to the station point 009DE050 was handed.
    {
        std::size_t followers = 0;
        for (std::size_t i = 0; i < host.controllers.size(); ++i) {
            if (!host.controllers[i].follow_ran) continue;
            ++followers;
        }
        if (followers != 0) {
            host.log.notef("summary ship follow steppers=%zu (009E1610 -> 009DF2D0 -> "
                "0070D290 -> 009DE050)", followers);
            host.log.notef("  %-20s %10s %12s %12s", "follower", "steps", "err_final",
                "err_max");
            for (std::size_t i = 0; i < host.controllers.size(); ++i) {
                const Impl::Controller& ctl = host.controllers[i];
                if (!ctl.follow_ran) continue;
                host.log.notef("  %-20s %10llu %12.2f %12.2f",
                    (i < host.rows.size()) ? host.rows[i].unit.c_str() : "?",
                    ctl.follow_steps,
                    static_cast<double>(ctl.follow_station_error),
                    static_cast<double>(ctl.follow_station_error_max));
            }
        }
    }
    host.log.notef("ship avoidance search: queries=%llu refills=%llu clears=%llu",
        host.avoidance_queries, host.avoidance_refills, host.avoidance_clears);
    host.log.notef("ship avoidance cruise owners: reads=%llu unavailable=%llu",
        host.avoidance_role_reads, host.avoidance_role_unavailable);
    host.log.notef("native ship pre-step: updates=%llu; pose=cache-valid; "
        "model=absent; class-depth=loaded; width=loaded; full009e0270=bound",
        host.hull_geometry_updates);
    host.log.notef("  %-20s %-10s %8s %8s %8s %8s %9s %7s %7s %-14s %s", "unit", "state",
        "steps", "replans", "publish", "promote", "heading", "thinks", "scans", "chose",
        "blocked at");
    for (const GameShipAiRow& row : host.rows) {
        host.log.notef("  %-20s %-10s %8llu %8llu %8llu %8llu %9.4f %7llu %7llu %-14s %s",
            row.unit.c_str(), row.state.c_str(), row.controller_steps, row.replans,
            row.publishes, row.promotions, static_cast<double>(row.slot_heading_44),
            row.target_thinks, row.target_scans,
            row.fire_target.empty() ? "-" : row.fire_target.c_str(),
            row.target_blocked.empty() ? "-" : row.target_blocked.c_str());
    }
    for (const GameShipAiRow& row : host.rows) {
        if (row.state == "cruise") ++host.summary.states_cruise;
        else if (row.state == "stop") ++host.summary.states_stop;
        else if (row.state == "attackmove") ++host.summary.states_attackmove;
        else if (row.state == "movetopos") ++host.summary.states_movetopos;
        else ++host.summary.states_other;
        if (!row.fire_target.empty()) ++host.summary.units_with_fire_target;
    }
    host.log.notef("summary mission ship ai units=%zu ai_owned=%zu steps=%llu gated=%llu "
        "replans=%llu state_steps{concrete=%llu records=%llu} publishes=%llu promotions=%llu",
        host.summary.units, host.summary.ai_owned, host.summary.steps, host.summary.gated,
        host.summary.replans, host.summary.state_steps_concrete,
        host.summary.state_steps_recorded, host.summary.publishes, host.summary.promotions);
    host.log.notef("summary mission ship ai states cruise=%zu stop=%zu attackmove=%zu "
        "movetopos=%zu other=%zu", host.summary.states_cruise, host.summary.states_stop,
        host.summary.states_attackmove, host.summary.states_movetopos,
        host.summary.states_other);
    // Milestone 2o: the hop's own table, one row per unit that wrote a ring
    // slot, so a reader can see what reached the ring rather than only what the
    // controller decided.
    host.log.notef("  %-20s %-10s %8s %8s %8s %9s %9s %9s %9s %8s %7s %7s %7s", "unit",
        "state", "hops", "writes", "deadband", "slot_thr", "slot_rud", "live_thr",
        "live_rud", "livechg", "danger", "thr_lim", "load102c");
    for (const GameShipAiRow& row : host.rows) {
        host.log.notef("  %-20s %-10s %8llu %8llu %8llu %9.4f %9.4f %9.4f %9.4f %8llu "
            "%7.3f %7.3f %7.3f",
            row.unit.c_str(), row.state.c_str(), row.ring_hops, row.ring_writes,
            row.rudder_deadbands, static_cast<double>(row.ring_slot_throttle),
            static_cast<double>(row.ring_slot_rudder),
            static_cast<double>(row.ring_live_throttle),
            static_cast<double>(row.ring_live_rudder), row.live_pair_changes,
            static_cast<double>(row.danger_a84),
            static_cast<double>(row.throttle_limit_344),
            static_cast<double>(row.turn_assist_load_102c));
    }
    // Milestone 2p: the goal vector, one row per unit whose brain pre-pass ran,
    // so the (0,0) of milestone 2o can be compared against what the command
    // actually asks for.
    host.log.notef("  %-20s %-10s %8s %8s %11s %11s %-16s %s", "unit", "state", "prepass",
        "refresh", "goal_x", "goal_z", "target", "0071eb60 descriptor");
    for (const GameShipAiRow& row : host.rows) {
        if (row.goal_prepasses == 0) continue;
        host.log.notef("  %-20s %-10s %8llu %8llu %11.1f %11.1f %-16s %s", row.unit.c_str(),
            row.state.c_str(), row.goal_prepasses, row.goal_refreshes,
            static_cast<double>(row.brain_goal_x), static_cast<double>(row.brain_goal_z),
            row.brain_target_name.empty() ? "-" : row.brain_target_name.c_str(),
            row.command_descriptor.c_str());
        if (row.brain_goal_x != 0.0f || row.brain_goal_z != 0.0f) {
            ++host.summary.units_with_nonzero_goal;
        }
        if (row.brain_target != 0u) ++host.summary.units_with_brain_target;
    }
    for (const GameShipAiRow& row : host.rows) {
        if (row.goal_sets > 0) ++host.summary.units_with_goal;
    }
    // Milestone 2p: the path plan 009E3780 builds and the approach point
    // 009F1BC0 copies, for the units whose state reaches either.
    host.log.notef("  %-20s %-10s %8s %8s %6s %6s %8s %11s %11s %11s", "unit", "state",
        "plan_req", "seeds", "state", "nodes", "approach", "point_x", "point_z",
        "goal_range");
    for (const GameShipAiRow& row : host.rows) {
        if (row.path_plan_refreshes == 0 && row.approach_frames == 0) continue;
        host.log.notef("  %-20s %-10s %8llu %8llu %6d %6d %8llu %11.1f %11.1f %11.1f",
            row.unit.c_str(), row.state.c_str(), row.path_plan_refreshes,
            row.path_plan_seeds, row.path_plan_state, row.path_plan_nodes,
            row.approach_frames, static_cast<double>(row.approach_point_x),
            static_cast<double>(row.approach_point_z),
            static_cast<double>(row.approach_goal_range));
    }
    host.log.notef("summary mission ship ai plan requests=%llu seeds=%llu accepts=%llu "
        "approach_frames=%llu controller_updates=%llu",
        host.summary.path_plan_refreshes, host.summary.path_plan_seeds,
        host.summary.path_plan_accepts, host.summary.approach_frames,
        host.summary.controller_updates);
    // Milestone 2q: the search, the swap and the point.
    host.log.notef("  %-20s %-10s %8s %6s %6s %10s %7s %11s %11s %9s %9s", "unit", "state",
        "searches", "state", "nodes", "swaps", "points", "point_x", "point_z", "nav_dist",
        "nav_hdg");
    for (const GameShipAiRow& row : host.rows) {
        if (row.path_search_ticks == 0 && row.nav_output_blocks == 0) continue;
        if (row.path_points > 0) ++host.summary.units_with_path_point;
        host.log.notef("  %-20s %-10s %8llu %6d %6d %10llu %7llu %11.1f %11.1f %9.2f %9.4f",
            row.unit.c_str(), row.state.c_str(), row.path_search_ticks, row.path_plan_state,
            row.path_plan_nodes, row.path_plan_swaps, row.path_points,
            static_cast<double>(row.path_point_x), static_cast<double>(row.path_point_z),
            static_cast<double>(row.nav_distance_32c),
            static_cast<double>(row.nav_heading_324));
    }
    host.log.notef("summary mission ship ai path search ticks=%llu swaps=%llu points=%llu "
        "corner_arms=%llu units_with_point=%zu output_blocks=%llu bearings=%llu",
        host.summary.path_search_ticks, host.summary.path_plan_swaps,
        host.summary.path_points, host.summary.path_corner_arms,
        host.summary.units_with_path_point, host.summary.nav_output_blocks,
        host.summary.nav_bearings);
    // Milestone 2r: what 009E4330 gave each ship, and the hull body beside it.
    host.log.notef("  %-20s %-10s %9s %9s %9s %9s %9s %9s %10s %4s", "unit", "state",
        "len_9c8", "turn_3c8", "turn_3cc", "yaw_3d0", "stop_3d4", "start_3d8", "mass_b0",
        "mat");
    for (const GameShipAiRow& row : host.rows) {
        host.log.notef("  %-20s %-10s %9.2f %9.2f %9.2f %9.5f %9.2f %9.2f %10.1f %4d",
            row.unit.c_str(), row.state.c_str(),
            static_cast<double>(row.nav_hull_length_9c8),
            static_cast<double>(row.nav_turn_circle_3c8),
            static_cast<double>(row.nav_turn_circle_3cc),
            static_cast<double>(row.nav_yaw_floor_3d0),
            static_cast<double>(row.nav_stop_radius_3d4),
            static_cast<double>(row.nav_start_radius_3d8),
            static_cast<double>(row.hull_mass_00b0), row.hull_material);
    }
    // Milestone 2r: the attackmove ring scan, one row per ship that reached it.
    host.log.notef("  %-20s %-10s %9s %9s %7s %10s %10s %9s %9s %8s", "unit", "state",
        "ring_scan", "bearings", "winner", "hdg_120c", "thr_1210", "clear_37c", "profiles",
        "substate");
    for (const GameShipAiRow& row : host.rows) {
        if (row.ring_scans == 0 && row.clearance_refreshes == 0) continue;
        host.log.notef("  %-20s %-10s %9llu %9llu %7d %10.4f %10.4f %9.1f %9llu %8lx",
            row.unit.c_str(), row.state.c_str(), row.ring_scans, row.ring_scan_bearings,
            row.ring_scan_winner, static_cast<double>(row.approach_heading_120c),
            static_cast<double>(row.approach_throttle_1210),
            static_cast<double>(row.clearance_37c), row.throttle_profiles,
            static_cast<unsigned long>(row.substate));
    }
    host.log.notef("summary mission ship ai nav blocks=%zu (009e4330 once per brain "
        "record, 009f118d) clearance=%llu throttle_profiles=%llu sector_scans=%llu "
        "sector_marks=%llu ring_scans=%llu ring_bearings=%llu firepower=%llu "
        "follower_points=%llu follower_corners=%llu follower_advances=%llu",
        host.summary.nav_blocks, host.summary.clearance_refreshes,
        host.summary.throttle_profiles, host.summary.sector_scans,
        host.summary.sector_marks, host.summary.ring_scans,
        host.summary.ring_scan_bearings, host.summary.firepower_ratings,
        host.summary.path_follower_points, host.summary.path_follower_corners,
        host.summary.path_follower_advances);
    // Packet cc8_ship_ai_approach_curves: what the 119-step scan at 009E71A5
    // chose once the two curve objects behind it were real.
    host.log.notef("summary mission ship ai standoff choices=%llu curve_refreshes=%llu "
        "(009e6e80 writes nested+11e4h; 0095f080 fills nested+12c0h and nested+13b0h)",
        host.summary.standoff_choices, host.summary.approach_curve_refreshes);
    for (const GameShipAiRow& row : host.rows) {
        if (row.standoff_choices == 0) continue;
        host.log.notef("  standoff %-20s choices=%llu first=%.1f last=%.1f "
            "curve_own_nonzero=%d curve_target_nonzero=%d max_weapon_range=%.1f "
            "committed_first=%d committed_last=%d heading_changes=%llu",
            row.unit.c_str(), row.standoff_choices,
            static_cast<double>(row.standoff_range_first),
            static_cast<double>(row.standoff_range_last),
            row.curve_own_nonzero, row.curve_target_nonzero,
            static_cast<double>(row.unit_max_weapon_range),
            row.ring_winner_first, row.ring_winner_last, row.heading_changes);
        host.log.notef("    gate %-18s flag_0b28=%d ref_127c=%.1f look_0494=%.1f "
            "flag_stops=%llu range_stops=%llu",
            row.unit.c_str(), row.gate_flag_0b28 ? 1 : 0,
            static_cast<double>(row.gate_reference_127c),
            static_cast<double>(row.gate_lookahead_0494),
            row.gate_flag_stops, row.gate_range_stops);
        host.log.notef("    visible %-16s expiries=%llu flag_true=%llu recon=%llu "
            "surface=%llu",
            row.unit.c_str(), row.goal_timer_expiries, row.goal_visible_true,
            row.goal_visible_recon, row.goal_visible_surface);
        host.log.notef("    slots %-18s commits=%llu winner_first=%d winner_last=%d "
            "total_best=%.4f total_worst=%.4f",
            row.unit.c_str(), row.committed_slot_changes,
            row.ring_scan_winner_first, row.ring_scan_winner_last,
            static_cast<double>(row.ring_total_best),
            static_cast<double>(row.ring_total_worst));
        host.log.notef("    words %-18s raw_18=%.4f norm_2c=%.4f pen_30=%.4f "
            "bear_34=%.4f evade_38=%.4f avoid_3c=%.4f",
            row.unit.c_str(),
            static_cast<double>(row.ring_word_raw_18),
            static_cast<double>(row.ring_word_normalized_2c),
            static_cast<double>(row.ring_word_penalty_30),
            static_cast<double>(row.ring_word_bearing_34),
            static_cast<double>(row.ring_word_evade_38),
            static_cast<double>(row.ring_word_avoid_3c));
        // Packet cc9_ship_traffic.
        host.log.notef("    traffic %-16s inserts=%llu erases=%llu refreshes=%llu "
            "max_records=%d weight_max=%.2f avoid_passes=%llu avoid_max=%.4f "
            "winner_changes=%llu first=%s",
            row.unit.c_str(), row.traffic_inserts, row.traffic_erases,
            row.traffic_refreshes, row.traffic_max_records,
            static_cast<double>(row.traffic_weight_max), row.avoid_active_passes,
            static_cast<double>(row.avoid_strength_max), row.ring_scan_winner_changes,
            row.traffic_first_entity.empty() ? "-" : row.traffic_first_entity.c_str());
    }
    // Packet cc9_ship_formation_speed: 009F4DA0's outputs for every ship that ran it.
    for (const GameShipAiRow& row : host.rows) {
        if (row.formation_ceiling_steps == 0) continue;
        host.log.notef("    formation %-14s role=%s steps=%llu limit344_min=%.4f below1=%llu "
            "limit348=[%.4f, %.4f]", row.unit.c_str(),
            row.formation_role.empty() ? "-" : row.formation_role.c_str(),
            row.formation_ceiling_steps, static_cast<double>(row.formation_limit_344_min),
            row.formation_limit_344_below_1, static_cast<double>(row.formation_limit_348_min),
            static_cast<double>(row.formation_limit_348_max));
        if (row.layer_selections != 0) {
            host.log.notef("    zone %-16s selections=%llu inside=%llu escape_turns=%llu max_turn=%.3f "
                "first=%.2f s travel_layer=[%u, %u]", row.unit.c_str(), row.layer_selections,
                row.zone_inside_steps, row.zone_escape_turns,
                static_cast<double>(row.zone_escape_max_turn),
                static_cast<double>(row.zone_escape_first_s),
                row.travel_layer_min, row.travel_layer_max);
        }
        if (row.torpedo_admits != 0 || row.torpedo_overrides != 0) {
            host.log.notef("    torpedo %-16s scans=%llu admits=%llu tracks_built=%llu tracks_max=%zu "
                "gate_open=%llu vector_steps=%llu overrides=%llu max_turn=%.3f first=%.2f s",
                row.unit.c_str(), row.torpedo_scans, row.torpedo_admits,
                row.torpedo_tracks_built, row.torpedo_tracks_max, row.torpedo_gate_open,
                row.torpedo_vector_steps, row.torpedo_overrides,
                static_cast<double>(row.torpedo_override_max_turn),
                static_cast<double>(row.torpedo_first_override_s));
        }
        if (row.station_requests != 0 || row.station_arm_runs != 0) {
            host.log.notef("    station %-16s requests=%llu arm_runs=%llu throttle39c=[%.4f, %.4f] "
                "limit_steps=%llu limit344=[%.4f, %.4f]", row.unit.c_str(),
                row.station_requests, row.station_arm_runs,
                static_cast<double>(row.station_throttle_min),
                static_cast<double>(row.station_throttle_max), row.station_limit_steps,
                static_cast<double>(row.station_limit_min),
                static_cast<double>(row.station_limit_max));
        }
    }
    host.log.notef("summary mission ship ai command completion events=%llu callbacks=%llu "
        "end_commands=%llu queue_advances=%llu",
        host.summary.command_events, host.summary.command_event_callbacks,
        host.summary.command_endings, host.summary.command_completions);
    host.log.notef("summary mission ship ai arm tail bodies=%llu latched=%llu stops=%llu "
        "arrival_latches=%llu",
        host.summary.arm_tails, host.summary.arm_tail_latched, host.summary.arm_tail_stops,
        host.summary.arrival_latches);
    host.log.notef("summary mission ship ai goal vector prepasses=%llu refreshes=%llu "
        "nonzero_goals=%zu brain_targets=%zu path_plan_refreshes=%llu path_picks=%llu "
        "path_publishes=%llu station_keeping=%llu sector_refreshes=%llu middle_runs=%llu "
        "substate_concrete=%llu",
        host.summary.goal_prepasses, host.summary.goal_refreshes,
        host.summary.units_with_nonzero_goal, host.summary.units_with_brain_target,
        host.summary.path_plan_refreshes, host.summary.path_picks,
        host.summary.path_publishes, host.summary.station_keeping,
        host.summary.sector_refreshes, host.summary.middle_runs,
        host.summary.substate_concrete);
    host.log.notef("summary mission ship ai state steps real=%llu goal_sets=%llu "
        "goal_replans=%llu units_with_goal=%zu substate_steps=%llu navigate_mode_steps=%llu",
        host.summary.state_steps_real, host.summary.goal_sets, host.summary.goal_replans,
        host.summary.units_with_goal, host.summary.substate_steps,
        host.summary.navigate_mode_steps);
    // driven= was retired by packet cc8_recon_sensor_pass_rule_c. units_driven was
    // never incremented anywhere in the tree, so driven=0 read as a gate on the AI
    // to rudder chain that does not exist: docs/SHIP_AI_HEADING_TO_RUDDER.md shows
    // the chain runs, and live_pair_changes is the field that measures it.
    host.log.notef("summary mission ship ai ring hops=%llu gated_3f5=%llu writes=%llu "
        "rudder_law=%llu deadbands=%llu live_pair_changes=%llu",
        host.summary.ring_hops, host.summary.ring_gated_3f5, host.summary.ring_writes,
        host.summary.rudder_law_calls, host.summary.rudder_deadbands,
        host.summary.live_pair_changes);
    host.log.notef("summary mission auto target thinks=%llu scans=%llu chose=%zu "
        "fire_target_sets=%llu attackmove_issues=%llu accepts=%zu",
        host.summary.thinks, host.summary.scans, host.summary.units_with_fire_target,
        host.summary.fire_target_sets, host.summary.attackmove_issues,
        host.summary.units_accepting_new_target);
    // Milestone 2p: what 0071DF70's two rules saw, per unit.
    host.log.notef("  %-20s %-10s %8s %10s %-12s %8s %s", "unit", "state", "gate",
        "hold+40h", "slot0", "category", "0071df70");
    for (const GameShipAiRow& row : host.rows) {
        if (row.target_thinks == 0) continue;
        // A unit whose scan found no candidate never reaches 0071DF70, so its
        // hold and its slot were never read and the row says so rather than
        // printing the field's default.
        if (row.target_gate_tests == 0) {
            host.log.notef("  %-20s %-10s %8llu %10s %-12s %8s %s", row.unit.c_str(),
                row.state.c_str(), row.target_gate_tests, "-", "-", "-",
                "not reached: no candidate");
            continue;
        }
        host.log.notef("  %-20s %-10s %8llu %10.3f %-12s %8d %s", row.unit.c_str(),
            row.state.c_str(), row.target_gate_tests,
            static_cast<double>(row.director_hold_0040),
            row.slot0_command.empty() ? "-" : row.slot0_command.c_str(),
            row.slot0_category, row.target_blocked.empty() ? "accepted"
                : row.target_blocked.c_str());
    }
}

}  // namespace bsp::game
