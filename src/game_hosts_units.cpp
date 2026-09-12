// bsp_game.exe milestone 2i: the units the instantiate pass created, ticked.
//
// Milestone 2h created 32 `DestroyerGen` instances and reported that every unit
// pass of the frame and of the fixed step ticked 0 of them, because nothing
// walked them. This file is the other half of the fix: one state object per
// created unit, over which the reconstructions on main run.
//
// What runs here is recovered: 008255b0's twelve steps, 00813020's ring tick,
// 00825f20's gates and command chain with 0092d300, 0092e8c0 and 00937440,
// 0092be80's controller step and 004c0890's controlled-unit bind. What this
// file supplies, and labels, is listed in include/bsp/game_hosts_units.hpp.

#include "bsp/game_hosts_units.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_ship_ai.hpp"

#include "bsp/camera_affine.hpp"
#include "bsp/camera_projection.hpp"
#include "bsp/controlled_unit.hpp"
#include "bsp/cruise_speed_setting.hpp"
#include "bsp/ocean_height.hpp"
#include "bsp/pose_refresh.hpp"
#include "bsp/rigid_body_integration.hpp"
#include "bsp/ship_ai_throttle_ring.hpp"
#include "bsp/ship_ai_nav_block_ctor.hpp"
#include "bsp/ship_class_fields.hpp"
#include "bsp/ship_hull_body.hpp"
#include "bsp/ship_motion.hpp"
#include "bsp/unit_controller.hpp"
#include "bsp/unit_forces.hpp"
#include "bsp/unit_instance.hpp"
#include "bsp/unit_order_record.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/unit_rudder_curve.hpp"
#include "bsp/unit_state_message.hpp"
#include "bsp/vehicle_class.hpp"
#include "bsp/world_ocean.hpp"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

constexpr double kPi = 3.14159265358979323846;

float heading_degrees_of(const bsp::ShipMotionState& state) {
    return static_cast<float>(std::atan2(static_cast<double>(state.pose_row2[0]),
                                  static_cast<double>(state.pose_row2[2]))
        * 180.0 / kPi);
}

}  // namespace

// ---------------------------------------------------------------------------
// One created unit
// ---------------------------------------------------------------------------

struct GameUnitSlot {
    GameUnitRow row;

    // The pose the canonical projection borrows: +74h local, +C8h valid, +CCh
    // world, +10Ch derived-valid, with no parent because every entity of this
    // mission is top level (docs/GAME_EXECUTABLE.md milestone 2h, correction 8).
    bsp::CameraMatrix local{};
    bsp::CameraMatrix world{};
    std::uint8_t world_valid{1};
    std::uint8_t derived_valid{1};
    bsp::PoseRefreshView* parent{nullptr};
    std::unique_ptr<bsp::PoseRefreshView> pose;
    std::unique_ptr<bsp::UnitInstanceState> state;
    bsp::UnitClassBlock class_block{};

    // The motion half.
    bsp::ShipMotionState motion{};
    bsp::ShipMotionClass motion_class{};
    bsp::ShipClassFields fields{};
    bsp::UnitOrderRing ring{};
    bsp::UnitOrderQueue queue{};
    bsp::UnitOrderRecordStorage scratch{};
    bsp::UnitControllerState controller{};

    // The hull's rigid body. `motion_state` is M = *(B+4h) and `body` is B, which
    // the unit's force-model controller holds at controller+2Ch. Milestone 2r
    // builds both through the tail of 00937C90 (docs/SHIP_HULL_BODY.md), so the
    // mass, the box inertia, the row-1 torque lock, both damping rates and both
    // speed clamps are the game's own; `hull_material` is the record 00937CF1
    // selects for this hull.
    bsp::DynMotionState motion_state{};
    bsp::DynBody body{};
    bsp::ShipPhysicsMaterial hull_material{bsp::ShipPhysicsMaterial::kShip};

    int class_id{bsp::kUnitDestroyerClassId};  // unit+C4h, the descriptor's kind
    // Milestone 2p: the two load latches the middle of 009F3F80 raises with the
    // inlined bodies of 009D4FB0 (unit+102Ch) and 009D4FE0 (unit+1034h). Their
    // consumers are not in this process; the fields exist so the raise is a
    // real store rather than a discarded call, and the report prints them.
    float turn_assist_load_102c{0.0f};
    float secondary_load_1034{0.0f};
    bool standing_order{false};
    float standing_throttle{0.0f};
    float standing_rudder{0.0f};
};

struct GameUnitsHost::Impl {
    Impl(GameHostLog& log_in, GameMissionLuaHost& lua_in)
        : log(log_in), lua(lua_in), commands(log_in) {}

    GameHostLog& log;
    GameMissionLuaHost& lua;
    // Milestone 2l: the weapon director of every created unit, and the three
    // message hops between the authored `Command` token and its command slot.
    GameCommandsHost commands;
    std::vector<std::unique_ptr<GameUnitSlot>> slots;
    // A flat copy of the rows, rebuilt on demand so units() can hand the caller
    // one contiguous table without exposing the slots.
    mutable std::vector<GameUnitRow> rows;
    GameUnitsSummary summary{};
    // 00e188d8, the controlled unit. Held as an index because this process owns
    // no unit pointers; `bound` is the `!= 0` every reader of the global makes.
    bool controlled_bound{false};
    std::size_t controlled_index{0};
    // 00424c40 is a singleton, so the curve block is one table shared by every
    // ship in the mission, not a per-unit or per-class tuning. It is filled once
    // by load_gameplay_settings_0083b5e0; until then it carries the zeroes a
    // freshly allocated settings object has, which is why the load runs first.
    bsp::UnitRudderCurveSettings rudder_curve{};
    bool rudder_curve_loaded{false};
    // Milestone 2p: the seven AutoThrust keys of the same singleton, which
    // 009EC7C0 reads at +6CCh..+6ECh. Loaded beside the turn multipliers.
    bsp::ShipAiAutoThrustSettings auto_thrust{};
    bool auto_thrust_loaded{false};

    // The world fields 00c41550 and 00c5b1b0 read: gravity at world+04h..+0Ch,
    // the two sleep speed thresholds at world+3Ch/+40h and the countdown reload
    // at world+44h. None of them has a recovered producer
    // (docs/RIGID_BODY_INTEGRATION.md, follow-up `dyn_world_construction`), and
    // the Dyn world object itself does not exist in this process, so they stay
    // at zero: no gravity, and any non-zero speed keeps a body awake.
    bsp::DynWorldStepConstants physics_world{};

    // The notes are logged once per kind, not once per unit tick.
    bool logged_ocean{false};
    bool logged_scale{false};
    bool logged_curve{false};
    bool logged_integrator{false};
    bool logged_cruise{false};
    bool logged_gate{false};
    bool logged_precision{false};

    // Milestone 2n: the ship AI controller that publishes into each unit's own
    // 84-byte AI order slot, which the motion head at 00825f2c promotes.
    GameShipAiHost* ship_ai{nullptr};

    void record(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.unimplemented(method, text);
    }
    void done(const char* method, std::uint32_t address) {
        char text[16];
        std::snprintf(text, sizeof(text), "%08lx", static_cast<unsigned long>(address));
        log.implemented(method, text);
    }

    bool is_controlled(const GameUnitSlot& slot) const {
        return controlled_bound && controlled_index < slots.size()
            && slots[controlled_index].get() == &slot;
    }

    // The +CCh pose block and the motion state are two views of the same rows;
    // the motion path writes them, the pose refresh reads them.
    static void publish_pose(GameUnitSlot& slot) {
        for (int i = 0; i < 3; ++i) {
            slot.world[static_cast<std::size_t>(i)] = slot.motion.pose_row0[i];
            slot.world[static_cast<std::size_t>(4 + i)] = slot.motion.pose_row1[i];
            slot.world[static_cast<std::size_t>(8 + i)] = slot.motion.pose_row2[i];
            slot.world[static_cast<std::size_t>(12 + i)] = slot.motion.position[i];
        }
        slot.world[3] = 0.0f;
        slot.world[7] = 0.0f;
        slot.world[11] = 0.0f;
        slot.world[15] = 1.0f;
        slot.local = slot.world;
    }

    void refresh_row(GameUnitSlot& slot);
    void issue_into_ring(GameUnitSlot& slot, float throttle, float rudder);

    // The heading 00835ac0 latches. The unit virtual at primary slot 50h is a
    // RET 0 getter with no reconstruction, so this is the executable's own
    // value: atan2 over pose row 2, the same convention the trajectory dump and
    // the run log print in degrees.
    static float pose_heading_radians(const GameUnitSlot& slot) {
        return static_cast<float>(std::atan2(static_cast<double>(slot.motion.pose_row2[0]),
            static_cast<double>(slot.motion.pose_row2[2])));
    }
};

namespace {

// ---------------------------------------------------------------------------
// 0078cf20's two leaves, and the gameplay-modifier filter
// ---------------------------------------------------------------------------

// bsp::OceanHeightHost. The sampler itself is recovered and runs; what is not
// available is its receiver. 0078cf20 takes [[00e188a8]+19F0h] and hands
// [world+A8h] to both leaves, and that sub-object is the renderer/scene owner's
// (the foliage builder 00af0c50 takes its camera from the same pointer, and
// `construct_world` 004de610 is still a load record), so each leaf is a host
// record that answers the open-sea state docs/OCEAN_HEIGHT.md evidences: the
// wave field returns exactly 0.0f when field+F9h is set or field+24h is zero,
// and the coverage mask returns exactly 1.0f when no region covers the point.
class OceanFieldBinding final : public bsp::OceanHeightHost {
public:
    explicit OceanFieldBinding(GameUnitsHost::Impl& owner) : owner_(owner) {}

    float wave_height_0078c890(float, float) override {
        if (!owner_.logged_ocean) {
            owner_.logged_ocean = true;
            owner_.log.notef("ocean sampler 0078cf20 runs, both of its leaves are records: "
                "its receiver is [[00e188a8]+19F0h] and both calls take [world+A8h], the "
                "renderer/scene owner's field object, so the wave field answers its own "
                "disabled value 0.0f and the coverage mask its own open-sea value 1.0f, and "
                "the product 0078cf64 is exactly 0.0f");
        }
        owner_.record("ShipMotion::ocean_wave_field", 0x0078c890u);
        return 0.0f;
    }
    float coverage_mask_00b9cf50(float, float) override {
        owner_.record("ShipMotion::ocean_coverage_mask", 0x00b9cf50u);
        return 1.0f;
    }

private:
    GameUnitsHost::Impl& owner_;
};

// bsp::GameplayModifierHost. 008e6430 walks the list at manager+80h+category*0Ch
// and this process registers no modifier record, so the walk visits nothing and
// the filter is never reached; the product is the routine's own 1.0f at
// 00d7a24c rather than a literal a host returns.
class NoGameplayModifiers final : public bsp::GameplayModifierHost {
public:
    bool entry_matches_008e4680(const bsp::GameplayModifierEntry&) override { return false; }
};

// ---------------------------------------------------------------------------
// bsp::UnitInstanceHost, one method per native call site of 008255b0
// ---------------------------------------------------------------------------

class UnitInstanceBinding final : public bsp::UnitInstanceHost {
public:
    UnitInstanceBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot)
        : owner_(owner), slot_(slot) {}

    // 00b6db70 on the scene node at +4A4h. 00928860, the placement that would
    // give a created unit a scene node, is a milestone 2h record, so the state
    // carries has_scene_node = false and this is unreachable; it is here because
    // the interface has no default.
    float refresh_scene_node_world_matrix() override {
        owner_.record("UnitInstance::refresh_scene_node", 0x00b6db70u);
        return 0.0f;
    }
    void release_submerged_effect(std::size_t) override {
        owner_.record("UnitInstance::release_submerged_effect", 0x00867b10u);
    }
    // The ocean object's own world Y, which step 2 of 008255b0 gates on. That is
    // the object's pose, not a sample of the sampler, and its producer is the
    // renderer/scene owner; a surface at y = 0 leaves the bubble block alone,
    // which is why it is a stated contract rather than a claim.
    bool ocean_exists() override { return true; }
    float ocean_world_y() override { return 0.0f; }
    float draw_bubble_interval() override {
        owner_.record("UnitInstance::draw_bubble_interval", 0x00bd2f10u);
        return 0.0f;
    }
    void spawn_bubble_effect() override {
        owner_.record("UnitInstance::spawn_bubble_effect", 0x008687c0u);
    }
    // 0092d730 on the controller at +1018h: the body-axis forward speed, which
    // is reconstructed, so this is the recovered value rather than a record.
    float controller_time_value() override {
        bsp::UnitBodyAxisSpeedInputs in{};
        in.velocity[0] = slot_.motion.linear_velocity.x;
        in.velocity[1] = slot_.motion.linear_velocity.y;
        in.velocity[2] = slot_.motion.linear_velocity.z;
        in.axis[0] = slot_.motion.pose_row2[0];
        in.axis[1] = slot_.motion.pose_row2[1];
        in.axis[2] = slot_.motion.pose_row2[2];
        owner_.done("UnitInstance::controller_body_axis_speed", 0x0092d730u);
        return bsp::unit_forward_speed_0092d730(in);
    }
    // 00815aa0 is reconstructed as publish_unit_effect_intensity_00815aa0, but
    // it publishes into the effect groups at unit+FFCh, and a created unit has
    // none: 006fe590, the instance the descriptor's vtable +28h allocates, is a
    // milestone 2h record and nothing fills those groups.
    void sub_update_00815aa0(float) override {
        owner_.record("UnitInstance::publish_effect_intensity", 0x00815aa0u);
    }
    bsp::UnitAnchorPoint transform_bow_anchor() override {
        owner_.record("UnitInstance::transform_bow_anchor", 0x00413920u);
        return {};
    }
    bsp::UnitAnchorPoint transform_stern_anchor() override {
        owner_.record("UnitInstance::transform_stern_anchor", 0x00413920u);
        return {};
    }
    // The same 0078cf20 the motion tick's wave gate calls, run whole.
    float sample_ocean_height(float x, float z) override {
        OceanFieldBinding sea(owner_);
        const float height = bsp::ocean_water_height_0078cf20(x, z, sea);
        owner_.done("UnitInstance::ocean_height", 0x0078cf20u);
        return height;
    }
    void publish_bow_anchor(const bsp::UnitAnchorPoint&) override {
        owner_.record("UnitInstance::publish_bow_anchor", 0x004842c0u);
    }
    void publish_stern_anchor(const bsp::UnitAnchorPoint&) override {
        owner_.record("UnitInstance::publish_stern_anchor", 0x004842c0u);
    }
    // 0092be80 on the controller, whose whole recovered body is one RET
    // (docs/UNIT_CONTROLLER_UPDATE.md). Running it is the point: the negative
    // result is what the reconstruction records.
    void sub_update_0092be80(float scaled_delta) override {
        bsp::unit_controller_update_0092be80(slot_.controller, scaled_delta);
        owner_.done("UnitInstance::controller_update", 0x0092be80u);
    }
    void sub_update_0081c050() override {
        owner_.record("UnitInstance::sub_update_0081c050", 0x0081c050u);
    }
    bool is_controlled_unit() override { return owner_.is_controlled(slot_); }
    bool global_intensity_override() override { return false; }  // 00f87152
    void smooth_intensity_008227e0(float) override {
        owner_.record("UnitInstance::smooth_intensity", 0x008227e0u);
    }
    bool wake_enabled() override {
        owner_.record("UnitInstance::wake_setting", 0x00424c40u);
        return false;
    }
    void spawn_wake() override { owner_.record("UnitInstance::spawn_wake", 0x00935540u); }
    bool prop_wash_threshold_passed(float) override {
        owner_.record("UnitInstance::prop_wash_threshold", 0x00ce69d0u);
        return false;
    }
    void stop_prop_wash() override {
        owner_.record("UnitInstance::stop_prop_wash", 0x00b6da70u);
    }
    void update_attachment(std::size_t, const bsp::UnitAnchorPoint&) override {
        owner_.record("UnitInstance::update_attachment", 0x008689c0u);
    }
    // The three timed sub-updates of step 11. docs/UNIT_TIMED_SUBUPDATES.md
    // analyses them; packet cc_unit_subupdates owns their reconstruction.
    void sub_update_008252c0(float) override {
        owner_.record("UnitInstance::update_engine_audio", 0x008252c0u);
    }
    void sub_update_00956600(float) override {
        owner_.record("UnitInstance::update_unit_timers", 0x00956600u);
    }
    void sub_update_00834e90(float) override {
        owner_.record("UnitInstance::update_propellers", 0x00834e90u);
    }
    void tick_part(std::size_t, float) override {
        owner_.record("UnitInstance::tick_part", 0x00815370u);
    }

private:
    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
};

// ---------------------------------------------------------------------------
// bsp::UnitRudderHost, the yaw-rate curve 00811890 reads
// ---------------------------------------------------------------------------

class UnitRudderBinding final : public bsp::UnitRudderHost {
public:
    UnitRudderBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot)
        : owner_(owner), slot_(slot) {}

    // 0082e890 calls 00424c40 four times on the low branch and three on the
    // high one and reads +438h..+44Ch off each returned pointer. The settings
    // object is the singleton the Lua-driven loader 0083b5e0 filled, so this
    // hands back the one shared block rather than a per-unit copy.
    const bsp::UnitRudderCurveSettings& settings_00424c40() override {
        owner_.done("ShipMotion::rudder_curve_settings", 0x00424c40u);
        return owner_.rudder_curve;
    }
    bool scale_manager_present() override { return false; }
    const bsp::ShipClassFields& ship_class() override { return slot_.fields; }
    bool gameplay_scale_enabled() override { return false; }
    bool scale_manager_enabled() override { return false; }
    float gameplay_scale_008e6430(int) override {
        NoGameplayModifiers modifiers;
        const float scale = bsp::gameplay_modifier_product_008e6430(nullptr, 0, modifiers);
        owner_.done("ShipMotion::gameplay_scale", 0x008e6430u);
        return scale;
    }
    float turn_efficiency() override { return slot_.motion.turn_efficiency; }
    float forward_speed_0092d730() override { return forward_speed; }
    float steering_command() override { return slot_.motion.to_turn; }

    float forward_speed{0.0f};

private:
    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
};

// ---------------------------------------------------------------------------
// bsp::UnitOrderRecordIssueHost, 00816a40's network half
// ---------------------------------------------------------------------------

class UnitOrderIssueBinding final : public bsp::UnitOrderRecordIssueHost {
public:
    explicit UnitOrderIssueBinding(GameUnitsHost::Impl& owner) : owner_(owner) {}
    int session_mode() override { return 1; }  // game+1FE4h, single player
    bsp::UnitOrderMessageStorage construct_message_0075b430(int) override {
        owner_.record("UnitOrder::construct_message", 0x0075b430u);
        return bsp::UnitOrderMessageStorage{};
    }
    void send_message_0077c2a0(const bsp::UnitOrderMessageStorage&, std::uint32_t,
        std::uint32_t) override {
        owner_.record("UnitOrder::send_message", 0x0077c2a0u);
    }

private:
    GameUnitsHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// bsp::ShipMotionHost, one method per call site of 00825f20's motion path
// ---------------------------------------------------------------------------

class ShipMotionBinding final : public bsp::ShipMotionHost {
public:
    ShipMotionBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot,
        UnitRudderBinding& rudder)
        : owner_(owner), slot_(slot), rudder_(rudder) {}

    void tick_order_ring(float dt) override {
        bsp::tick_unit_order_ring_00813020(slot_.ring, dt, 1);
        slot_.motion.throttle = slot_.ring.current_param_a;
        slot_.motion.to_turn = slot_.ring.current_param_b;
        slot_.motion.order_kind = slot_.ring.current_kind;
        owner_.done("ShipMotion::order_ring_tick", 0x00813020u);
    }

    // 0078cf20 at 00826985, run whole: the wave field times the coverage mask,
    // formed at x87 precision and rounded once. Both leaves are records.
    float ocean_height(float x, float z) override {
        OceanFieldBinding sea(owner_);
        const float height = bsp::ocean_water_height_0078cf20(x, z, sea);
        owner_.done("ShipMotion::ocean_height", 0x0078cf20u);
        return height;
    }

    // 008e6430 at 00826a21, run whole over an empty category list. The
    // accumulator starts at the 1.0f at 00d7a24c and nothing multiplies into it,
    // so the result is the routine's own value.
    float gameplay_scale() override {
        if (!owner_.logged_scale) {
            owner_.logged_scale = true;
            owner_.log.notef("gameplay scale 008e6430 runs over an empty category list: no "
                "modifier record is registered in this process, so the product is the 1.0f "
                "the accumulator starts at (00d7a24c) and the filter 008e4680 is never "
                "reached");
        }
        NoGameplayModifiers modifiers;
        const float scale = bsp::gameplay_modifier_product_008e6430(nullptr, 0, modifiers);
        owner_.done("ShipMotion::gameplay_scale", 0x008e6430u);
        return scale;
    }

    // controller->vtable[0](dt), which for a surface ship is 00937440. Its
    // torque is computed so the run can report it; nothing applies it, because
    // the hydrodynamic tail 009329c0 and the rigid-body solver are external.
    bsp::OceanVec3 run_force_model(float) override {
        bsp::UnitSteeringTorqueInputs in{};
        in.velocity = slot_.motion.linear_velocity;
        in.axis.x = slot_.motion.pose_row2[0];
        in.axis.y = slot_.motion.pose_row2[1];
        in.axis.z = slot_.motion.pose_row2[2];
        in.reference_speed = reference_speed();
        in.hull_mass = slot_.motion_class.hull_mass;
        in.steering = slot_.motion.to_turn;
        in.pose_row0_y = slot_.motion.pose_row0[1];
        in.pose_row2.x = slot_.motion.pose_row2[0];
        in.pose_row2.y = slot_.motion.pose_row2[1];
        in.pose_row2.z = slot_.motion.pose_row2[2];
        in.settings_rudder_torque = 0.0f;  // settings+588h, not recovered
        owner_.done("ShipMotion::force_model", 0x00937440u);
        return bsp::unit_steering_torque_00937440(in);
    }

    float reference_speed() override {
        return bsp::unit_reference_speed_0080fc30(slot_.motion.max_speed,
            bsp::kUnitReferenceSpeedUnscaled);
    }

    bsp::OceanVec3 body_linear_velocity() override { return slot_.motion.linear_velocity; }

    bsp::ShipBodyBasis body_basis() override {
        bsp::ShipBodyBasis basis{};
        for (int i = 0; i < 3; ++i) {
            basis.row0[i] = slot_.motion.pose_row0[i];
            basis.row1[i] = slot_.motion.pose_row1[i];
            basis.row2[i] = slot_.motion.pose_row2[i];
        }
        return basis;
    }

    float forward_acceleration() override {
        return bsp::unit_forward_acceleration_00825ec0(slot_.motion_class.retardation,
            slot_.motion.acceleration_boost, 0.0f);
    }

    void body_set_linear_velocity(const bsp::OceanVec3& value) override {
        slot_.motion.linear_velocity = value;
    }

    bsp::OceanVec3 body_angular_velocity() override { return slot_.motion.angular_velocity; }

    float yaw_rate_target(float smoothed_rudder) override {
        rudder_.forward_speed = forward_speed();
        // 0092e950 then the FCHS at 0092e955.
        const float rate = -bsp::unit_yaw_rate_00811890(smoothed_rudder, rudder_);
        // 0092e966: settings+220h/+224h were not recovered; with a zero
        // propeller load the routine returns the rate unchanged whatever they
        // are, which is the state a created unit is in here.
        return bsp::unit_propeller_turn_assist_00825de0(rate, slot_.motion.propeller_load,
            0.0f, 0.0f);
    }

    // A destroyer's IsKindOf answers {0,1,2,4,5,6,7} plus its own class id, so
    // 0Eh is false; the same holds for every ship leaf this mission creates.
    bool unit_trait_0e() override { return false; }

    void body_set_angular_velocity(const bsp::OceanVec3& value) override {
        slot_.motion.angular_velocity = value;
    }

    void controller_step(float dt) override {
        bsp::unit_controller_update_0092be80(slot_.controller, dt);
        owner_.done("ShipMotion::controller_step", 0x0092be80u);
    }
    void unit_post_motion(float) override {
        owner_.record("ShipMotion::unit_post_motion", 0x00826b84u);
    }

    float forward_speed() const {
        bsp::UnitBodyAxisSpeedInputs in{};
        in.velocity[0] = slot_.motion.linear_velocity.x;
        in.velocity[1] = slot_.motion.linear_velocity.y;
        in.velocity[2] = slot_.motion.linear_velocity.z;
        in.axis[0] = slot_.motion.pose_row2[0];
        in.axis[1] = slot_.motion.pose_row2[1];
        in.axis[2] = slot_.motion.pose_row2[2];
        return bsp::unit_forward_speed_0092d730(in);
    }

private:
    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
    UnitRudderBinding& rudder_;
};

// ---------------------------------------------------------------------------
// 004c0890, the controlled-unit bind
// ---------------------------------------------------------------------------

class ControlledUnitQueryBinding final : public bsp::ControlledUnitQuery {
public:
    explicit ControlledUnitQueryBinding(int class_id) : class_id_(class_id) {}
    bool unit_is_kind_of(int query) override {
        // 006fe530 is MDestroyer's implementation of vtable +5Ch. A ship leaf
        // that is not MDestroyer shares every ancestor but the literal 7
        // (docs/LOCAL_PLAYER_UNIT_LISTS.md: every ship descriptor chain
        // contains 6), so the executable answers the same chain with that one
        // literal removed rather than inventing a second table.
        if (query == bsp::kUnitDestroyerClassId
            && class_id_ != bsp::kUnitDestroyerClassId) {
            return false;
        }
        return bsp::unit_is_kind_of_006fe530(query, class_id_);
    }
    bool has_driven_sub_unit() override { return false; }   // unit+3D0h
    bool sub_unit_is_kind_of(int) override { return false; }

private:
    int class_id_;
};

class SetControlledUnitBinding final : public bsp::SetControlledUnitHost {
public:
    explicit SetControlledUnitBinding(GameUnitsHost::Impl& owner) : owner_(owner) {}
    void store_controlled_unit(bool unit_present) override {
        owner_.controlled_bound = unit_present;
        owner_.done("ControlledUnit::store_global", 0x004c0893u);
    }
    bool driven_listener_handle() override {
        owner_.record("ControlledUnit::driven_listener_handle", 0x004c08f2u);
        return false;
    }
    void publish_listener(bool) override {
        // 00b0d7b0 stores the handle at renderResources+1C0h and kicks +30h.
        // The render-resources singleton at 00f8d39c is the renderer owner's.
        owner_.record("ControlledUnit::publish_listener", 0x00b0d7b0u);
    }

private:
    GameUnitsHost::Impl& owner_;
};

}  // namespace

// ---------------------------------------------------------------------------
// Impl helpers
// ---------------------------------------------------------------------------

void GameUnitsHost::Impl::refresh_row(GameUnitSlot& slot) {
    GameUnitRow& row = slot.row;
    for (int i = 0; i < 3; ++i) row.position[i] = slot.motion.position[i];
    row.heading_degrees = heading_degrees_of(slot.motion);
    bsp::UnitBodyAxisSpeedInputs speed{};
    speed.velocity[0] = slot.motion.linear_velocity.x;
    speed.velocity[1] = slot.motion.linear_velocity.y;
    speed.velocity[2] = slot.motion.linear_velocity.z;
    speed.axis[0] = slot.motion.pose_row2[0];
    speed.axis[1] = slot.motion.pose_row2[1];
    speed.axis[2] = slot.motion.pose_row2[2];
    row.forward_speed = bsp::unit_forward_speed_0092d730(speed);
    row.throttle = slot.motion.throttle;
    row.ordered_rudder = slot.motion.to_turn;
    row.rudder = slot.motion.smoothed_rudder;
    row.yaw_rate = slot.motion.angular_velocity.y;
    // The component about the hull's own up axis, which is what 0092e8c0 slews.
    row.yaw_rate_up_axis = slot.motion.angular_velocity.x * slot.motion.pose_row1[0]
        + slot.motion.angular_velocity.y * slot.motion.pose_row1[1]
        + slot.motion.angular_velocity.z * slot.motion.pose_row1[2];
    const double dx = static_cast<double>(row.position[0]) - static_cast<double>(row.start[0]);
    const double dz = static_cast<double>(row.position[2]) - static_cast<double>(row.start[2]);
    row.distance = static_cast<float>(std::sqrt(dx * dx + dz * dz));
}

void GameUnitsHost::Impl::issue_into_ring(GameUnitSlot& slot, float throttle, float rudder) {
    // 00816a40 builds the 20h-byte record through 00815440, which clamps both
    // parameters into [-2,+2], and publishes it into the slot the write cursor
    // names. The queue and the ring are two projections of the same native
    // storage at unit+838h, so the published slot is copied into the ring the
    // tick reads; 0080dad0 clears +8h, which marks the slot authoritative.
    UnitOrderIssueBinding issue(*this);
    slot.queue.slot_index = slot.ring.write_cursor;
    bsp::issue_unit_order_record_00816a40(issue, slot.queue, slot.scratch, throttle, rudder, 0);
    const int index = slot.queue.slot_index;
    slot.ring.slot[index].param_a = slot.queue.slot[index].param_a;
    slot.ring.slot[index].param_b = slot.queue.slot[index].param_b;
    slot.ring.slot[index].kind = slot.queue.slot[index].kind;
    slot.ring.slot[index].predicted = slot.queue.slot_active[index];
    done("ShipMotion::issue_order_record", 0x00816a40u);
}

namespace {

// ---------------------------------------------------------------------------
// Milestone 2q: bsp::CruiseSpeedSettingHost, the five call sites of 00822C20's
// property-bag arm (0082356C..008235FB).
// ---------------------------------------------------------------------------
//
// 00822C20 is slot 0A0h of the game-unit vtable family and the executable
// reaches it where the native does, from BSP_SEntity_InitAll's slot call at
// 00926110, which in this process is the per-entity pass create_units makes
// over the records the instantiate pass left. The two finds are answered from
// the merged property bag the scene-contents host kept on the entity record
// (docs/CRUISE_SPEED_SETTING.md); the three setters are the reconstructions
// already on main.
class StartSpeedSeedBinding final : public bsp::CruiseSpeedSettingHost {
public:
    StartSpeedSeedBinding(GameUnitsHost::Impl& owner, GameUnitSlot& slot,
                          const GameSceneEntityRecord& entity)
        : owner_(owner), slot_(slot), entity_(entity) {}

    bool find_shipyard_launch_00823576(std::uint32_t, const char*) override {
        owner_.done("SceneStartSpeed::find_shipyard_launch", 0x00823576u);
        return entity_.shipyard_launch;
    }

    bsp::SceneStartSpeedProperty find_start_speed_00823590(std::uint32_t,
                                                           const char*) override {
        owner_.done("SceneStartSpeed::find_start_speed", 0x00823590u);
        bsp::SceneStartSpeedProperty record;
        record.present = entity_.start_speed_present;
        record.type = static_cast<bsp::ScenePropertyType>(entity_.start_speed_type);
        record.value_int = entity_.start_speed_int;
        record.value_float = entity_.start_speed_float;
        return record;
    }

    float unit_reference_speed_0080fc30(std::uint32_t) override {
        // 008235BA and 008235DC, the same callee at two sites. unit+9C0h is
        // the class `MaxSpeed` 00822C4F/00822C65 set earlier in this same
        // routine, which create_units copies out of the VehicleClass row.
        ++reference_calls;
        owner_.done("SceneStartSpeed::unit_reference_speed", 0x0080fc30u);
        return bsp::unit_reference_speed_0080fc30(slot_.motion.max_speed,
            bsp::kUnitReferenceSpeedUnscaled);
    }

    void set_order_ring_throttle_0080d9b0(std::uint32_t, float throttle) override {
        // 008235D5 with ECX = unit+838h. The reconstruction fills the pending
        // span and writes the live field, which is ring+148h.
        bsp::set_unit_order_ring_param_a_0080d9b0(slot_.ring, throttle);
        owner_.done("SceneStartSpeed::set_order_ring_throttle", 0x0080d9b0u);
    }

    void set_controller_axial_speed_0092d770(std::uint32_t, float speed) override {
        // 008235F7 with ECX = [unit+1018h]. The body axis is the pose's row 2,
        // the same forward vector 0092D730 projects onto.
        bsp::OceanVec3 axis{};
        axis.x = slot_.motion.pose_row2[0];
        axis.y = slot_.motion.pose_row2[1];
        axis.z = slot_.motion.pose_row2[2];
        slot_.motion.linear_velocity
            = bsp::unit_set_axial_speed_0092d770(slot_.motion.linear_velocity, axis, speed);
        slot_.motion_state.linear_velocity = slot_.motion.linear_velocity;
        owner_.done("SceneStartSpeed::set_controller_axial_speed", 0x0092d770u);
    }

    int reference_calls{0};

private:
    GameUnitsHost::Impl& owner_;
    GameUnitSlot& slot_;
    const GameSceneEntityRecord& entity_;
};

}  // namespace

// ---------------------------------------------------------------------------
// GameUnitsHost
// ---------------------------------------------------------------------------

GameUnitsHost::GameUnitsHost(GameHostLog& log, GameMissionLuaHost& lua)
    : impl_(std::make_unique<Impl>(log, lua)) {}

GameUnitsHost::~GameUnitsHost() = default;

void GameUnitsHost::load_gameplay_settings_0083b5e0() {
    Impl& host = *impl_;
    if (host.rudder_curve_loaded) return;
    // 0083b5e0's head: run Scripts\datatables\ShipGlobals.lua and take the
    // `ShipGlobals` global. Then the fragment 0083ce56..0083d10d, which is what
    // fills +438h..+44Ch of the settings object 00424c40 hands out.
    const bool table = host.lua.load_ship_globals_0083b6e6();
    host.record("GameSettings::load_from_lua_globals", 0x0083b5e0u);
    if (!table) {
        host.log.notef("rudder curve: `ShipGlobals` did not load, so 00424c40()+438h..+44Ch "
            "keeps the zeroes a fresh settings object has and 0082e890 would divide "
            "MaxRotAngle by zero; the motion path is left with the curve unset");
        return;
    }
    // Milestone 2p: the AutoThrust half of the same loader, 0083cba8..0083ce3c,
    // read before the turn multipliers so an incomplete TurnMultipliers table
    // does not hide it. 009EC7C0 consumes seven of the eleven keys.
    if (host.lua.read_auto_thrust_0083cc2c(host.auto_thrust)) {
        host.auto_thrust_loaded = true;
        host.done("GameSettings::load_auto_thrust", 0x0083cc2cu);
        host.log.notef("auto thrust loaded from ShipGlobals[\"Navigator\"][\"AutoThrust\"]: "
            "hdg slow (%.4f, %.4f) thrust_min %.3f, hdg fast (%.4f, %.4f) thrust_min %.3f, "
            "danger mul %.3f; these are 00424c40()+6cch..+6ech, the block "
            "009ec7c0 BSP_UnitBot_ComputeThrottleCeiling interpolates over",
            static_cast<double>(host.auto_thrust.hdg_diff_value_min_slow),
            static_cast<double>(host.auto_thrust.hdg_diff_value_max_slow),
            static_cast<double>(host.auto_thrust.thrust_min_slow),
            static_cast<double>(host.auto_thrust.hdg_diff_value_min_fast),
            static_cast<double>(host.auto_thrust.hdg_diff_value_max_fast),
            static_cast<double>(host.auto_thrust.thrust_min_fast),
            static_cast<double>(host.auto_thrust.hdg_diff_danger_mul));
    } else {
        host.record("GameSettings::load_auto_thrust", 0x0083cc2cu);
        host.log.notef("auto thrust: ShipGlobals[\"Navigator\"][\"AutoThrust\"] is absent or "
            "incomplete, so 00424c40()+6cch..+6ech keeps the zeroes a fresh settings object "
            "has and 009ec7c0's five interpolation stages all run on zero endpoints");
    }

    bsp::UnitRudderCurveSettings settings{};
    if (!host.lua.read_turn_multipliers_0083ce56(settings)) {
        host.log.notef("rudder curve: ShipGlobals[\"Navigator\"][\"TurnMultipliers\"] is "
            "incomplete, so the six fields stay unset");
        return;
    }
    host.rudder_curve = settings;
    host.rudder_curve_loaded = true;
    host.done("GameSettings::load_turn_multipliers", 0x0083ce56u);
    host.log.notef("rudder curve loaded from ShipGlobals[\"Navigator\"][\"TurnMultipliers\"]: "
        "min (%.3f, %.3f) med (%.3f, %.3f) max (%.3f, %.3f); index 1 is the throttle "
        "coordinate and index 2 the turn-circle multiplier 0082ecb0 divides MaxRotAngle by",
        static_cast<double>(settings.speed_0444), static_cast<double>(settings.value_0440),
        static_cast<double>(settings.speed_044c), static_cast<double>(settings.value_0448),
        static_cast<double>(settings.speed_043c), static_cast<double>(settings.value_0438));
}

void GameUnitsHost::create_units(const std::vector<GameSceneEntityRecord>& entities) {
    Impl& host = *impl_;
    for (const GameSceneEntityRecord& entity : entities) {
        if (!entity.created) continue;
        auto slot = std::make_unique<GameUnitSlot>();
        GameUnitRow& row = slot->row;
        row.name = entity.name;
        row.class_name = entity.class_name;
        row.type_symbol = entity.type_symbol;
        row.type_id = entity.type_id;
        row.party = entity.party;
        // Milestone 2l: the two strings 004f0520's last step handed 00469610.
        row.command = entity.command;
        row.command_target = entity.command_target;

        // The frame 0046cf40 composed for the gate is the instance's world 4x4;
        // rows 0..2 are the body axes and row 3 the position, which is the same
        // shape the motion path reads at unit+CCh..+108h.
        for (int i = 0; i < 3; ++i) {
            slot->motion.pose_row0[i] = entity.world[i];
            slot->motion.pose_row1[i] = entity.world[4 + i];
            slot->motion.pose_row2[i] = entity.world[8 + i];
            slot->motion.position[i] = entity.world[12 + i];
            row.start[i] = slot->motion.position[i];
            row.position[i] = slot->motion.position[i];
        }
        row.heading_degrees = heading_degrees_of(slot->motion);

        // The class descriptor. 00964790, the factory that would build one, is
        // a milestone 2h record; what this reads is the installed
        // `VehicleClass` global the recovered global-script step 00886900
        // loaded, indexed by the `Type = E ShipClasses : <symbol>` id the enum
        // library resolved.
        const GameVehicleClassRow lua_row = host.lua.read_vehicle_class_row(row.type_id);
        row.class_row_found = lua_row.found;
        row.class_row_name = lua_row.name;
        if (lua_row.found) {
            ++host.summary.class_rows;
            row.max_speed = lua_row.max_speed;
            row.max_rot_angle = lua_row.max_rot_angle;
            slot->motion.max_speed = lua_row.max_speed;   // 00822c20 seeds unit+9C0h
            slot->motion_class.max_accel = lua_row.max_accel;
            slot->motion_class.retardation = lua_row.retardation;
            slot->fields.max_speed = lua_row.max_speed;
            slot->fields.max_rot_angle = lua_row.max_rot_angle;
            slot->fields.max_rot_angle_change_ratio = lua_row.max_rot_angle_change_ratio;
            // The descriptor kind is the recovered map from the row's `Type`
            // literal to the leaf class the factory's string chain selects.
            const bsp::VehicleClassDescriptorRow* kind
                = bsp::vehicle_class_kind_row(lua_row.type.c_str());
            if (kind != nullptr) slot->class_id = static_cast<int>(kind->kind);
        }
        // Milestone 2r. class+A0h, class+A8h and class+B0h are the `Length`,
        // `Height` and `Mass` keys 00960230 writes into the descriptor, and the
        // installed `VehicleClass` row carries all three (DeRuyter row 20:
        // 171, 5, 7688). Milestone 2q left them at zero because the keel gate
        // passes either way on a flat sea; they are read now because
        // 00937C90's hull body needs the mass and 00937440 needs it squared,
        // and because 00826866 places the keel sample point half a hull length
        // astern and half a hull height below the pose. A missing key leaves
        // the zero the reader stores, except `Mass`, whose reader default is
        // the 1.0f at 0096043A.
        slot->motion_class.hull_length = lua_row.length;   // class+A0h
        slot->motion_class.hull_height = lua_row.height;   // class+A8h
        constexpr float kClassMassReaderDefault = 1.0f;  // 0096043A
        slot->motion_class.hull_mass
            = lua_row.mass > 0.0f ? lua_row.mass : kClassMassReaderDefault;
        slot->motion_class.boost_refill_time = 1.0f;
        slot->motion.thrust_mod = 1.0f;        // 00823714
        slot->motion.turn_efficiency = 1.0f;   // 0082371c
        slot->motion.class_id = slot->class_id;

        bsp::construct_unit_order_ring_00812d40(slot->ring);

        // Milestone 2q: 00926110, BSP_SEntity_InitAll's call of the entity's
        // vtable slot 0A0h, which for this class family is 00822C20. Only that
        // routine's property-bag arm 0082356C..008235FB is run here, and only
        // its kind-1 branch: 0046d5b0 built the holder over a cloned scene
        // property bag, so 00823537's tag is 1 and 0082353D takes the arm.
        // The seed has to happen here, before issue_authored_commands latches
        // the scene's queued `Cruise` at 00835E17, because the latch captures
        // the ring's live throttle (docs/CRUISE_SPEED_SETTING.md).
        {
            StartSpeedSeedBinding seed_host(host, *slot, entity);
            const bsp::SceneStartSpeedSeed seed = bsp::run_start_speed_arm_0082356c(
                seed_host, static_cast<std::uint32_t>(host.slots.size()) + 1u,
                static_cast<std::uint32_t>(host.slots.size()) + 1u,
                bsp::kSceneEntityBagRefKindPropertyBag);
            host.done("SceneStartSpeed::init_slot_00a0", 0x00926110u);
            row.start_speed_authored = seed.authored;
            row.start_speed = seed.start_speed;
            row.start_speed_ratio = seed.ring_throttle;
            row.start_speed_axial = seed.axial_speed;
            if (seed.authored) {
                ++host.summary.start_speed_seeds;
                // ring.current_param_a and motion.throttle are two projections
                // of the one native field unit+980h, which 0080D9E8 has just
                // written; the ring tick copies it every step, so this only
                // keeps the two in step before the first tick runs.
                slot->motion.throttle = slot->ring.current_param_a;
            }
        }

        // Milestone 2r: the hull body the game builds, replacing milestone 2h's
        // freshly constructed one and the 1.0e30f speed clamps that stood in
        // for M+18h / M+1Ch. 00939E2A calls 00937C90 last in the controller
        // constructor 00939CB0 and its tail 009399C0..00939C05 is the writer:
        // the descriptor default 009391C2, the mass at 009399F7, the zeroed
        // inertia diagonal, the angular damping 1.0f at 00939A2F, the row-1
        // torque lock when the mass is under 100.0 (00939A57), then
        // 00C5D580 CreateBody and 00C37E70 with the box inertia at 00939C05.
        // The linear damping stays the descriptor's own 0.0f (00939295); it is
        // not a gap, and docs/SHIP_HULL_BODY.md says so.
        //
        // The collision AABB is the one input this cannot supply: its producer
        // is the shape attach 00C5C940 behind 00937D3F..009399BF, which no
        // packet has read, so the span is zero and the box inertia with it,
        // which 00C37E70 turns into a zero inverse inertia. Nothing here
        // applies a torque, so that decides nothing this run measures; it is
        // the same default the probe takes.
        {
            bsp::ShipHullBodyInputs hull{};
            hull.mass = slot->motion_class.hull_mass;             // 009399F7
            // 00937D09, the unit's answer to virtual slot 5Ch with category 8
            // (MSubmarine): it picks the third physics record.
            hull.unit_category_8 = bsp::unit_is_kind_of_006fe530(
                bsp::kUnitForceSubmarineClassId, slot->class_id);
            for (int i = 0; i < 3; ++i) {
                hull.row0[i] = slot->motion.pose_row0[i];
                hull.row1[i] = slot->motion.pose_row1[i];
                hull.row2[i] = slot->motion.pose_row2[i];
                hull.position[i] = slot->motion.position[i];
            }
            bsp::ship_hull_body_create_00937c90(hull, slot->body, slot->motion_state);
            slot->hull_material = bsp::ship_hull_material_00937cf1(hull.unit_category_8,
                hull.mass);
        }
        slot->body.motion = &slot->motion_state;

        slot->parent = nullptr;
        slot->pose = std::make_unique<bsp::PoseRefreshView>(
            bsp::PoseRefreshView{bsp::PoseRefreshParentSlot(slot->parent), slot->local,
                slot->world_valid, slot->world, slot->derived_valid});
        slot->state = std::make_unique<bsp::UnitInstanceState>(
            bsp::UnitInstanceState{*slot->pose});
        slot->state->class_id = slot->class_id;
        slot->state->active = true;      // +5Ch, the world tick gate
        slot->state->simulate = false;   // +5Dh; the list filter requires it clear
        slot->state->has_scene_node = false;  // +4A4h, 00928860 is a 2h record
        slot->state->part_count = 0;     // +A18h, the instance has no parts here
        Impl::publish_pose(*slot);
        host.slots.push_back(std::move(slot));
    }
    host.summary.units = host.slots.size();
    // Milestone 2l: the entities 0046aab0's target lookup would find, and the
    // owners its records name. entity+174h is the executable's own id space,
    // because the two handle tables at 00f89a0c / 00f89a60 are not built here.
    std::vector<GameCommandUnit> command_units;
    command_units.reserve(host.slots.size());
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        GameCommandUnit unit;
        unit.index = index;
        unit.name = host.slots[index]->row.name;
        unit.object_id = static_cast<std::uint16_t>(index + 1);
        for (int lane = 0; lane < 3; ++lane) {
            unit.position[lane] = host.slots[index]->motion.position[lane];
        }
        command_units.push_back(unit);
    }
    host.commands.register_units(std::move(command_units));
    host.log.notef("world units: %zu created instance(s) carried into the frame, %zu with a "
        "VehicleClass row out of the installed table", host.summary.units,
        host.summary.class_rows);
}

void GameUnitsHost::issue_authored_commands() {
    Impl& host = *impl_;
    if (!host.logged_cruise) {
        host.logged_cruise = true;
        host.log.notef("milestone 2l: the authored `Command` token is no longer a stand-in "
            "order. docs/CRUISE_COMMAND.md recovered `cruise` as a latch that captures the "
            "ring's ordered pair (unit+980h / unit+984h) and the heading when it becomes "
            "the unit's current command and re-applies what it captured, so a ship whose "
            "ring is still zero holds zero. Milestone 2i wrote throttle 1 / rudder 0 into "
            "the ring for the same token, which is what the latch would produce for a ship "
            "already running at full throttle and is wrong for a ship at rest");
    }
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        GameUnitSlot& slot = *host.slots[index];
        // The token every DestroyerGen of this scene authors. An entity that
        // authored none is skipped, exactly as 004f0520's last step skips the
        // queue call when the property is absent.
        if (slot.row.command.empty()) continue;
        const float heading = host.pose_heading_radians(slot);
        const GameCommandRow* row = host.commands.issue(index, slot.row.command,
            slot.row.command_target, slot.ring, heading);
        if (row == nullptr) continue;
        slot.row.command_current = row->current;
        slot.row.command_latched = row->latched;
        slot.row.latch_is_heading = row->fields.is_heading;
        slot.row.latch_steer = row->fields.steer_or_heading;
        slot.row.latch_thrust = row->fields.thrust;
        // Nothing on this path writes an order ring, so no standing order is
        // armed: the ring stays at whatever 00812d40 constructed it with.
        slot.standing_order = false;
        ++host.summary.cruise_orders;
    }
}

bool GameUnitsHost::issue_player_command(const std::string& token,
    const std::string& target_token, const std::string& unit_name) {
    Impl& host = *impl_;
    std::size_t index = host.controlled_index;
    if (!unit_name.empty()) {
        index = host.slots.size();
        for (std::size_t candidate = 0; candidate < host.slots.size(); ++candidate) {
            if (host.slots[candidate]->row.name == unit_name) { index = candidate; break; }
        }
        if (index >= host.slots.size()) {
            host.log.notef("--order-unit \"%s\" names no created instance; the command was "
                "not issued", unit_name.c_str());
            return false;
        }
    } else if (!host.controlled_bound) {
        return false;
    }
    if (index >= host.slots.size()) return false;
    GameUnitSlot& slot = *host.slots[index];
    const float heading = host.pose_heading_radians(slot);
    const GameCommandRow* row = host.commands.issue(index, token,
        target_token, slot.ring, heading);
    if (row == nullptr) return false;
    slot.row.command = row->command.empty() ? token : row->command;
    slot.row.command_target = target_token;
    slot.row.command_current = row->current;
    slot.row.command_latched = row->latched;
    slot.row.latch_is_heading = row->fields.is_heading;
    slot.row.latch_steer = row->fields.steer_or_heading;
    slot.row.latch_thrust = row->fields.thrust;
    ++host.summary.player_orders;
    host.log.notef("player command issued to \"%s\": token=\"%s\" resolved=\"%s\" "
        "outcome=%s current=%d latched=%d (%s %.3f, thrust %.3f)", slot.row.name.c_str(),
        token.c_str(), row->command.c_str(), row->resolve_outcome.c_str(),
        row->current ? 1 : 0, row->latched ? 1 : 0,
        row->fields.is_heading ? "heading" : "rudder",
        static_cast<double>(row->fields.steer_or_heading),
        static_cast<double>(row->fields.thrust));
    if (!row->blocked.empty()) host.log.notef("  %s", row->blocked.c_str());
    return true;
}

const GameCommandRow* GameUnitsHost::issue_script_command(std::size_t unit_index,
    std::uint32_t command_object, const bsp::SceneCommandTarget& target, int flags,
    const std::string& source, const std::string& target_name) {
    Impl& host = *impl_;
    if (unit_index >= host.slots.size()) return nullptr;
    GameUnitSlot& slot = *host.slots[unit_index];
    const float heading = host.pose_heading_radians(slot);
    const GameCommandRow* row = host.commands.issue_command_object(unit_index,
        command_object, target, flags, source, target_name, slot.ring, heading);
    // The unit's own row keeps the token the scene authored: a scripted order is
    // a second command on the same director, not a replacement for the first,
    // and GameScriptOrdersHost reports it in its own table.
    return row;
}

void GameUnitsHost::store_commanded_speed_00890e6f(std::size_t unit_index, float speed) {
    Impl& host = *impl_;
    host.commands.store_commanded_speed_00890e6f(unit_index, speed,
        host.summary.simulated_seconds);
}

bsp::CruiseSpeedSetting GameUnitsHost::commanded_speed(std::size_t unit_index) const {
    return impl_->commands.commanded_speed(unit_index);
}

float GameUnitsHost::mission_clock() const noexcept {
    return impl_->summary.simulated_seconds;
}

void GameUnitsHost::run_director_steps_00836920() {
    Impl& host = *impl_;
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        GameUnitSlot& slot = *host.slots[index];
        if (!slot.state->active) continue;
        const float heading = host.pose_heading_radians(slot);
        const GameDirectorStepOutcome outcome = host.commands.director_step_00836920(
            index, host.is_controlled(slot), host.summary.simulated_seconds, slot.ring,
            heading);
        if (outcome.reissued == bsp::DirectorDefaultCommand::None) continue;
        // The idle tail's own choice becomes the unit's standing command when
        // the slot push took it; 0071be40 is what answers that.
        slot.row.command_current = slot.row.command_current
            || host.commands.holds_cruise(index);
    }
}

const GameCommandsHost& GameUnitsHost::commands() const noexcept { return impl_->commands; }

void GameUnitsHost::set_controlled_unit_004c0890(std::size_t index) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    GameUnitSlot& slot = *host.slots[index];
    host.controlled_index = index;
    ControlledUnitQueryBinding query(slot.class_id);
    SetControlledUnitBinding binding(host);
    const bsp::ControlledUnitGlobals globals
        = bsp::set_controlled_unit_004c0890(true, query, binding);
    host.done("ControlledUnit::set_controlled_unit", 0x004c0890u);
    for (std::unique_ptr<GameUnitSlot>& other : host.slots) other->row.controlled = false;
    slot.row.controlled = true;
    host.summary.controlled_bound = globals.unit_present;
    host.summary.controlled_index = index;
    host.summary.controlled_name = slot.row.name;
    host.log.notef("controlled unit: 00e188d8 = \"%s\" (%s %s, party %d); 00e188dc %s, "
        "because the resolved object answers neither IsKindOf(0Fh) nor IsKindOf(18h)",
        slot.row.name.c_str(), slot.row.class_name.c_str(), slot.row.type_symbol.c_str(),
        slot.row.party, globals.listener_present ? "published a handle" : "was cleared");
}

void GameUnitsHost::issue_player_order(float throttle, float rudder) {
    Impl& host = *impl_;
    if (!host.controlled_bound || host.controlled_index >= host.slots.size()) return;
    GameUnitSlot& slot = *host.slots[host.controlled_index];
    slot.standing_order = true;
    slot.standing_throttle = throttle;
    slot.standing_rudder = rudder;
    host.issue_into_ring(slot, throttle, rudder);
    ++host.summary.player_orders;
    host.log.notef("player order issued to \"%s\": throttle=%.3f rudder=%.3f through "
        "00816a40, clamped into [-2,+2] by 00815440", slot.row.name.c_str(),
        static_cast<double>(throttle), static_cast<double>(rudder));
}

void GameUnitsHost::update_entity_008255b0(std::size_t index, float scaled_delta) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    GameUnitSlot& slot = *host.slots[index];
    UnitInstanceBinding binding(host, slot);
    bsp::update_unit_instance_008255b0(*slot.state, slot.class_block, binding, scaled_delta);
    ++slot.row.instance_updates;
    ++host.summary.instance_updates;
    host.done("World::update_entity", 0x008255b0u);
}

void GameUnitsHost::motion_step_00825f20(float step_seconds) {
    Impl& host = *impl_;
    if (host.slots.empty()) return;
    if (!host.logged_precision) {
        // The second half of docs/X87_CONTROL_WORD.md's read, taken where that
        // document says it matters: the fixed step that runs the reconstruction
        // of 00825f20, after the Direct3D 9 device exists. The first half is in
        // game_main before Direct3D is created. Neither read changes anything.
        host.logged_precision = true;
        const unsigned long field = x87_precision_field();
        host.log.notef("x87 precision at the fixed simulation step: %s (_controlfp_s & "
            "_MCW_PC = 0x%08lx). The shipped executable runs 00825f20's own float "
            "expressions on the x87 stack at this precision; this reconstruction computes "
            "them with SSE2 float32 under /fp:strict, which rounds every intermediate to "
            "24 bits of mantissa as well",
            x87_precision_name(field), field);
    }
    ++host.summary.motion_steps;
    host.summary.simulated_seconds += step_seconds;
    // Milestone 2m: the weapon director's own step, before the motion pass that
    // reads what the step decided. 00836920's caller is the unit update's
    // director block, which this process does not reach, so the position is the
    // executable's decision and is recorded as one.
    run_director_steps_00836920();
    // Milestone 2n: the ship AI controller 009f50e0 for every created unit, and
    // the weapon director's automatic target think 009f5da0 beside it. Neither
    // has a caller in the call graph, so the position is the executable's
    // decision and is recorded as one; it runs before the motion pass because
    // the motion's own head at 00825f2c consumes the order slot 009f4d10
    // published on this step.
    if (host.ship_ai != nullptr) {
        host.ship_ai->controller_step(step_seconds);
        host.ship_ai->log_sample(host.summary.motion_steps, 10);
    }
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        GameUnitSlot& slot = *host.slots[index];
        if (!slot.state->active) continue;
        // 00825f2c..00825f7c, the head of BSP_UnitInstance_UpdateShipMotion:
        // the promotion of the slot the AI controller just published, before
        // anything else the routine does.
        if (host.ship_ai != nullptr) host.ship_ai->promote_order_00825f2c(index);
        // The order under the write cursor is refilled every step so a standing
        // order keeps standing: the game does that from the HUD every frame the
        // key is held, and the ring's own forward copy at 00813186 would
        // otherwise only mark later slots predicted.
        if (slot.standing_order) {
            host.issue_into_ring(slot, slot.standing_throttle, slot.standing_rudder);
        }
        // Milestone 2n: 009e1170's AI arm is no longer run from here. It is the
        // `cruise` state's vtable +0Ch, and 009f5186 calls it on a re-plan tick
        // of the controller above, which is what milestone 2l's own correction
        // from docs/SHIP_AI_STATES.md says. When no controller is attached the
        // step does not run at all rather than running on the wrong schedule.
        UnitRudderBinding rudder(host, slot);
        ShipMotionBinding motion(host, slot, rudder);
        const float before[3] = {slot.motion.position[0], slot.motion.position[1],
            slot.motion.position[2]};
        const bsp::ShipMotionStepResult result
            = bsp::ship_motion_step_00825f20(slot.motion, slot.motion_class, motion,
                step_seconds);
        // The Dyn library's own two integration phases, in the order 00c5bb30
        // runs them. The motion tick has just written both velocities onto the
        // body through 00c37e50 / 00c37e20, so the velocity phase 00c41550 sees
        // no force, no gravity and no damping and only rebuilds the world
        // inverse inertia, and the position phase 00c5b1b0 is what turns the two
        // velocities into a pose. docs/RIGID_BODY_INTEGRATION.md.
        if (!host.logged_integrator) {
            host.logged_integrator = true;
            host.log.notef("rigid body: 00c41550 then 00c5b1b0, one substep of the whole "
                "%.4f s game step. Milestone 2r builds the hull body through the tail of "
                "00937c90, so \"%s\" carries mass %.1f (inverse %.8f), angular damping "
                "%.1f, linear damping %.1f, both speed clamps at %.1f, torque-lock=%d and "
                "physics material %d; the box inertia is zero because the collision AABB's "
                "producer 00c5c940 is unread. The linear damping of 0.0f is the game's own "
                "descriptor default (00939295), not a gap: the only routine that resists a "
                "hull's lateral velocity is 009329c0, the hydrodynamic tail of 00937440",
                static_cast<double>(step_seconds), slot.row.name.c_str(),
                static_cast<double>(slot.motion_class.hull_mass),
                static_cast<double>(slot.motion_state.inverse_mass),
                static_cast<double>(slot.motion_state.angular_damping),
                static_cast<double>(slot.motion_state.linear_damping),
                static_cast<double>(slot.motion_state.max_linear_speed),
                slot.motion_state.lock_torque_to_row1 ? 1 : 0,
                static_cast<int>(slot.hull_material));
        }
        host.record("ShipMotion::rigid_body_substep_schedule", 0x00c5bb30u);
        slot.motion_state.linear_velocity = slot.motion.linear_velocity;
        slot.motion_state.angular_velocity = slot.motion.angular_velocity;
        slot.body.motion = &slot.motion_state;
        for (int i = 0; i < 3; ++i) {
            slot.body.row0[i] = slot.motion.pose_row0[i];
            slot.body.row1[i] = slot.motion.pose_row1[i];
            slot.body.row2[i] = slot.motion.pose_row2[i];
            slot.body.position[i] = slot.motion.position[i];
        }
        bsp::dyn_body_integrate_velocity_00c41550(slot.body, host.physics_world,
            step_seconds);
        host.done("ShipMotion::rigid_body_velocity_phase", 0x00c41550u);
        bsp::dyn_body_integrate_position_00c5b1b0(slot.body, host.physics_world,
            step_seconds);
        host.done("ShipMotion::rigid_body_position_phase", 0x00c5b1b0u);
        slot.motion.linear_velocity = slot.motion_state.linear_velocity;
        slot.motion.angular_velocity = slot.motion_state.angular_velocity;
        for (int i = 0; i < 3; ++i) {
            slot.motion.pose_row0[i] = slot.body.row0[i];
            slot.motion.pose_row1[i] = slot.body.row1[i];
            slot.motion.pose_row2[i] = slot.body.row2[i];
            slot.motion.position[i] = slot.body.position[i];
        }
        Impl::publish_pose(slot);
        const double dx = static_cast<double>(slot.motion.position[0]) - before[0];
        const double dy = static_cast<double>(slot.motion.position[1]) - before[1];
        const double dz = static_cast<double>(slot.motion.position[2]) - before[2];
        slot.row.path_length += static_cast<float>(std::sqrt(dx * dx + dy * dy + dz * dz));
        slot.row.command_applied = result.gate.command_applies;
        if (!host.logged_gate) {
            host.logged_gate = true;
            // 00826994: the command is suppressed when the keel sample point has
            // risen above half the local wave height. With the flat-sea stand-in
            // that is a test of the authored hull height against zero, so a
            // reader can tell a closed gate from a missing class row.
            host.log.notef("ship motion gate on \"%s\": keel=(%.2f, %.2f, %.2f) wave=%.2f "
                "applies=%d throttle=%.3f target_speed=%.3f engine_gate=%.1f",
                slot.row.name.c_str(), static_cast<double>(result.keel_point.x),
                static_cast<double>(result.keel_point.y),
                static_cast<double>(result.keel_point.z),
                static_cast<double>(result.wave_height),
                result.gate.command_applies ? 1 : 0,
                static_cast<double>(result.gate.throttle),
                static_cast<double>(result.target_speed),
                static_cast<double>(result.engine_gate));
        }
        ++slot.row.motion_ticks;
        ++host.summary.motion_ticks;
        host.done("World::unit_motion_tick", 0x00825f20u);
        host.refresh_row(slot);
    }
    host.summary.total_path_length = 0.0f;
    for (const std::unique_ptr<GameUnitSlot>& slot : host.slots) {
        host.summary.total_path_length += slot->row.path_length;
    }
    if (host.controlled_bound && host.controlled_index < host.slots.size()) {
        host.summary.controlled_distance = host.slots[host.controlled_index]->row.distance;
    }
}

// ---------------------------------------------------------------------------
// Milestone 2n: what the ship AI controller reads off a unit
// ---------------------------------------------------------------------------

void GameUnitsHost::set_ship_ai(GameShipAiHost* ai) noexcept { impl_->ship_ai = ai; }

std::uint32_t GameUnitsHost::director_current_command_0071be40(std::size_t index) const {
    return impl_->commands.current_command_0071be40(index);
}

std::size_t GameUnitsHost::report_command_event_00984300(std::size_t index,
    std::uint32_t command_object, const char* status) {
    return impl_->commands.report_command_event_00984300(index, command_object, status);
}

GameCommandCompletion GameUnitsHost::end_command_0071e430(std::size_t index,
    std::uint32_t command_object, bool terminal) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return GameCommandCompletion{};
    GameUnitSlot& slot = *host.slots[index];
    return host.commands.end_command_0071e430(index, command_object, terminal,
        unit_player_controlled_0184(index), slot.ring, Impl::pose_heading_radians(slot));
}

bool GameUnitsHost::unit_player_controlled_0184(std::size_t index) const {
    const Impl& host = *impl_;
    return host.controlled_bound && host.controlled_index == index;
}

bool GameUnitsHost::unit_flag_005d(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    return host.slots[index]->state->simulate;
}

bool GameUnitsHost::unit_flag_0061(std::size_t index) const {
    // docs/UNIT_AUTOPILOT_PAIR.md scanned .text for a writer of unit+61h at the
    // direct displacement and at the seven shifted unit bases and found none, so
    // nothing in this process or in the recovered load can set it. The byte is
    // clear, which is what lets the ship AI controller run at all.
    static_cast<void>(index);
    return false;
}

float GameUnitsHost::unit_forward_speed_0092d730(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    const GameUnitSlot& slot = *host.slots[index];
    bsp::UnitBodyAxisSpeedInputs axis{};
    axis.velocity[0] = slot.motion.linear_velocity.x;
    axis.velocity[1] = slot.motion.linear_velocity.y;
    axis.velocity[2] = slot.motion.linear_velocity.z;
    axis.axis[0] = slot.motion.pose_row2[0];
    axis.axis[1] = slot.motion.pose_row2[1];
    axis.axis[2] = slot.motion.pose_row2[2];
    return bsp::unit_forward_speed_0092d730(axis);
}

float GameUnitsHost::unit_retardation_0508(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return host.slots[index]->motion_class.retardation;
}

float GameUnitsHost::unit_heading_radians(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return Impl::pose_heading_radians(*host.slots[index]);
}

float GameUnitsHost::unit_current_yaw_rate_00811940(std::size_t index) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    UnitRudderBinding rudder(host, *host.slots[index]);
    return bsp::unit_current_yaw_rate_00811940(rudder);
}

bool GameUnitsHost::run_cruise_state_step_009e1170(std::size_t index,
    bsp::ShipAiControlBlock& blk, bsp::ShipAiSetterHost& setters) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    GameUnitSlot& slot = *host.slots[index];
    bsp::CruiseOrderedValues ordered{};
    return host.commands.cruise_step(index, host.is_controlled(slot),
        unit_forward_speed_0092d730(index),
        bsp::unit_reference_speed_0080fc30(slot.motion.max_speed,
            bsp::kUnitReferenceSpeedUnscaled),
        ordered, &blk, &setters);
}

// ---------------------------------------------------------------------------
// Milestone 2o: the ring's write slot, which the tail of 009f3f80 reads and
// then writes back through 0080e170 / 0080e190.
// ---------------------------------------------------------------------------

float GameUnitsHost::unit_ring_write_slot_throttle(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    const bsp::UnitOrderRing& ring = host.slots[index]->ring;
    return bsp::ship_ai_ring_write_slot_throttle(ring);
}

float GameUnitsHost::unit_ring_write_slot_rudder(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    const bsp::UnitOrderRing& ring = host.slots[index]->ring;
    return bsp::ship_ai_ring_write_slot_rudder(ring);
}

void GameUnitsHost::unit_ring_set_write_slot_throttle_0080e170(std::size_t index,
    float value) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    bsp::ship_ai_ring_set_write_slot_throttle_0080e170(host.slots[index]->ring, value);
}

void GameUnitsHost::unit_ring_set_write_slot_rudder_0080e190(std::size_t index,
    float value) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    bsp::ship_ai_ring_set_write_slot_rudder_0080e190(host.slots[index]->ring, value);
}

float GameUnitsHost::unit_ring_current_throttle(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return host.slots[index]->ring.current_param_a;
}

float GameUnitsHost::unit_ring_current_rudder(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    return host.slots[index]->ring.current_param_b;
}

float GameUnitsHost::unit_yaw_authority_0524(std::size_t index, bool& derived) const {
    derived = false;
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    const bsp::ShipClassFields& fields = host.slots[index]->fields;
    const bsp::ShipClassAiDerivedMotion out = bsp::ship_class_ai_derived_motion_00828f20(
        fields.max_rot_angle, fields.max_rot_angle_change_ratio, fields.max_speed);
    derived = out.derived;
    return out.yaw_authority_0524;
}

bool GameUnitsHost::enable_ai_drive(const std::string& unit_name, float throttle,
    float rudder) {
    Impl& host = *impl_;
    if (host.ship_ai == nullptr) return false;
    for (std::size_t index = 0; index < host.slots.size(); ++index) {
        if (host.slots[index]->row.name != unit_name) continue;
        host.ship_ai->set_ai_drive(index, throttle, rudder);
        return true;
    }
    host.log.notef("--ai-drive \"%s\" names no created instance; nothing was driven",
        unit_name.c_str());
    return false;
}

bool GameUnitsHost::unit_pose_valid_00c8(std::size_t index) const {
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return false;
    return host.slots[index]->world_valid != 0;
}

void GameUnitsHost::unit_position_00fc(std::size_t index, float& x, float& y,
    float& z) const {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    const Impl& host = *impl_;
    if (index >= host.slots.size()) return;
    const bsp::ShipMotionState& motion = host.slots[index]->motion;
    x = motion.position[0];
    y = motion.position[1];
    z = motion.position[2];
}

std::size_t GameUnitsHost::count() const noexcept { return impl_->slots.size(); }

bool GameUnitsHost::unit_active(std::size_t index) const noexcept {
    if (index >= impl_->slots.size()) return false;
    return impl_->slots[index]->state->active;
}

bool GameUnitsHost::unit_is_kind_of(std::size_t index, int class_id) const {
    if (index >= impl_->slots.size()) return false;
    const int own = impl_->slots[index]->class_id;
    if (class_id == bsp::kUnitDestroyerClassId && own != bsp::kUnitDestroyerClassId) {
        return false;
    }
    return bsp::unit_is_kind_of_006fe530(class_id, own);
}

int GameUnitsHost::unit_class_id(std::size_t index) const noexcept {
    if (index >= impl_->slots.size()) return -1;
    return impl_->slots[index]->class_id;
}

bool GameUnitsHost::unit_alive_and_visible(std::size_t index) const {
    if (index >= impl_->slots.size()) return false;
    const bsp::UnitInstanceState& state = *impl_->slots[index]->state;
    // 0043f080: [+5Ch] != 0 && [+5Dh] == 0 && [+60h] == 0 && [+5Eh] == 0.
    return state.active && !state.simulate;
}

bool GameUnitsHost::unit_pose(std::size_t index, float right[3], float up[3],
    float forward[3], float translation[3]) const {
    if (index >= impl_->slots.size()) return false;
    const bsp::CameraMatrix& world = impl_->slots[index]->world;
    for (std::size_t lane = 0; lane < 3; ++lane) {
        right[lane] = world[lane];
        up[lane] = world[4 + lane];
        forward[lane] = world[8 + lane];
        translation[lane] = world[12 + lane];
    }
    return true;
}

// ---------------------------------------------------------------------------
// Milestone 2p: what the brain pre-pass and the drive's middle read off a unit
// ---------------------------------------------------------------------------

bool GameUnitsHost::world_bounds_box_00e188a8(float& min_x, float& max_x, float& min_z,
    float& max_z) const {
    min_x = 0.0f;
    max_x = 0.0f;
    min_z = 0.0f;
    max_z = 0.0f;
    // The world object at [00E188A8] is never built here, so there is no box.
    // Reporting its absence is what lets 0071C4F0's own rule run at its call
    // site without four comparisons against zeroes.
    return false;
}

bool GameUnitsHost::active_command_descriptor_0071eb60(std::size_t index,
    bsp::SceneCommandTarget& out, int& mode) const {
    return impl_->commands.active_command_descriptor_0071eb60(index, out, mode);
}

std::uint32_t GameUnitsHost::resolve_command_target_00521ea0(
    const bsp::SceneCommandTarget& target) const {
    return impl_->commands.resolve_command_target_00521ea0(target);
}

void GameUnitsHost::transform_by_unit_matrix_004142e0(std::size_t index, float in_x,
    float in_y, float in_z, float& out_x, float& out_y, float& out_z) const {
    out_x = in_x;
    out_y = in_y;
    out_z = in_z;
    if (index >= impl_->slots.size()) return;
    const std::array<float, 3> source{in_x, in_y, in_z};
    std::array<float, 3> result{};
    bsp::transform_point_004142e0(source, impl_->slots[index]->world, result);
    out_x = result[0];
    out_y = result[1];
    out_z = result[2];
}

int GameUnitsHost::unit_side_0054(std::size_t index) const {
    if (index >= impl_->slots.size()) return -1;
    // unit+54h is the party the scene record authored, which milestone 2h's
    // party_class_marks pass already carries on the row.
    return impl_->slots[index]->row.party;
}

float GameUnitsHost::director_target_hold_0040(std::size_t index) const {
    return impl_->commands.director_target_hold_0040(index);
}

int GameUnitsHost::director_leading_slot_categories_0071df83(std::size_t index, int* out,
    int max_out) const {
    return impl_->commands.director_leading_slot_categories_0071df83(index, out, max_out);
}

std::uint32_t GameUnitsHost::director_slot_command(std::size_t index, int slot_index) const {
    return impl_->commands.director_slot_command(index, slot_index);
}

const char* GameUnitsHost::command_name_of(std::uint32_t command_object) const {
    return impl_->commands.command_name_of(command_object);
}

const bsp::ShipAiAutoThrustSettings& GameUnitsHost::auto_thrust_settings(bool& loaded) const {
    loaded = impl_->auto_thrust_loaded;
    return impl_->auto_thrust;
}

bsp::ShipAiThrottleCeilingInputs GameUnitsHost::throttle_ceiling_inputs(
    std::size_t index) const {
    bsp::ShipAiThrottleCeilingInputs in{};
    if (index >= impl_->slots.size()) return in;
    const GameUnitSlot& slot = *impl_->slots[index];
    // 009EC97B, |unit+980h|: the order ring's published throttle. The absolute
    // value is taken inside the reconstruction, so the raw field goes in.
    in.live_throttle = slot.row.throttle;
    // 009EC99C / 009EC9A4, *(unit+73Ch)+28h and +24h: the commanded-speed pair
    // milestone 2m put on the navigator parameter block.
    const bsp::CruiseSpeedSetting speed = impl_->commands.commanded_speed(index);
    in.commanded_speed = speed.speed;
    in.commanded_speed_enabled = speed.enable >= 0.0f;
    // 009EC9AB, blk+3C4h: the cached reference speed, the same 0080FC30 value
    // 009E12BD divides by.
    in.reference_speed = bsp::unit_reference_speed_0080fc30(slot.motion.max_speed,
        bsp::kUnitReferenceSpeedUnscaled);
    return in;
}

float GameUnitsHost::unit_half_width_09cc(std::size_t index) const {
    static_cast<void>(index);
    // unit+9CCh is read at 009D8FDE and at 009F4174 and has no recovered
    // producer anywhere; the caller records it. 1.0f is the neutral divisor,
    // not a recovered value.
    return 1.0f;
}

float GameUnitsHost::unit_class_max_speed_0500(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->fields.max_speed;
}

float GameUnitsHost::unit_class_max_rot_angle_04f8(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->fields.max_rot_angle;
}

float GameUnitsHost::unit_class_max_accel_0504(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->motion_class.max_accel;
}

float GameUnitsHost::unit_class_turn_radius_0520(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    // class+520h, the field 00828F20 derives once per class at 00828F66 from
    // MaxSpeed and MaxRotAngle. 0082E970 is its second reader, beside 0082E850.
    const bsp::ShipClassFields& fields = impl_->slots[index]->fields;
    const bsp::ShipClassAiDerivedMotion out = bsp::ship_class_ai_derived_motion_00828f20(
        fields.max_rot_angle, fields.max_rot_angle_change_ratio, fields.max_speed);
    return out.derived ? out.turn_radius_0520 : 0.0f;
}

float GameUnitsHost::unit_class_turn_circle_radius_0082e960(std::size_t index,
    float throttle) {
    Impl& host = *impl_;
    if (index >= host.slots.size()) return 0.0f;
    GameUnitSlot& slot = *host.slots[index];
    bsp::ShipAiNavBlockClassInputs in{};
    in.max_rot_angle_04f8 = slot.fields.max_rot_angle;
    in.max_speed_0500 = slot.fields.max_speed;
    in.turn_radius_0520 = unit_class_turn_radius_0520(index);
    UnitRudderBinding rudder(host, slot);
    const float radius = bsp::ship_class_turn_circle_radius_0082e960(in, throttle, rudder);
    host.done("ShipAiNavBlock::class_turn_circle_0082e960", 0x0082e960u);
    return radius;
}

float GameUnitsHost::unit_hull_length_09c8(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    // 0081106E and 0081FA4D: twice the larger half-extent of the model box at
    // [class+50h], or the descriptor's +A0h `Length` when the class carries no
    // box. This process builds no box, so the fallback is the whole answer.
    return impl_->slots[index]->motion_class.hull_length;
}

float GameUnitsHost::unit_hull_mass_00b0(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->motion_class.hull_mass;
}

int GameUnitsHost::unit_hull_material(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0;
    return static_cast<int>(impl_->slots[index]->hull_material);
}

float GameUnitsHost::unit_hull_linear_damping(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->motion_state.linear_damping;
}

float GameUnitsHost::unit_hull_angular_damping(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->motion_state.angular_damping;
}

void GameUnitsHost::raise_turn_assist_load_102c(std::size_t index, float value) {
    if (index >= impl_->slots.size()) return;
    // 009D4FB0 inlined: a compare and a store, so the raise is the whole body.
    GameUnitSlot& slot = *impl_->slots[index];
    if (value > slot.turn_assist_load_102c) slot.turn_assist_load_102c = value;
}

void GameUnitsHost::raise_secondary_load_1034(std::size_t index, float value) {
    if (index >= impl_->slots.size()) return;
    GameUnitSlot& slot = *impl_->slots[index];
    if (value > slot.secondary_load_1034) slot.secondary_load_1034 = value;
}

float GameUnitsHost::turn_assist_load_102c(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->turn_assist_load_102c;
}

float GameUnitsHost::secondary_load_1034(std::size_t index) const {
    if (index >= impl_->slots.size()) return 0.0f;
    return impl_->slots[index]->secondary_load_1034;
}

void GameUnitsHost::unit_class_extents(std::size_t index, float& forward, float& right,
    float& up) const {
    forward = 0.0f;
    right = 0.0f;
    up = 0.0f;
    if (index >= impl_->slots.size()) return;
    const bsp::ShipMotionClass& motion_class = impl_->slots[index]->motion_class;
    forward = motion_class.hull_length;  // class+A0h
    up = motion_class.hull_height;       // class+A8h
    // class+A4h has no recovered Lua key, so it stays zero rather than being
    // guessed from either of the other two.
}

const GameUnitRow* GameUnitsHost::unit_row(std::size_t index) const noexcept {
    if (index >= impl_->slots.size()) return nullptr;
    return &impl_->slots[index]->row;
}

bool GameUnitsHost::controlled_bound() const noexcept { return impl_->controlled_bound; }

std::size_t GameUnitsHost::controlled_index() const noexcept { return impl_->controlled_index; }

const std::vector<GameUnitRow>& GameUnitsHost::units() const noexcept {
    // The rows live inside the slots; a flat copy is rebuilt on demand so the
    // caller sees one contiguous table.
    impl_->rows.clear();
    impl_->rows.reserve(impl_->slots.size());
    for (const std::unique_ptr<GameUnitSlot>& slot : impl_->slots) {
        impl_->rows.push_back(slot->row);
    }
    return impl_->rows;
}

const GameUnitsSummary& GameUnitsHost::summary() const noexcept { return impl_->summary; }

void GameUnitsHost::log_controlled_trajectory(unsigned long long mission_frame) {
    Impl& host = *impl_;
    if (!host.controlled_bound || host.controlled_index >= host.slots.size()) return;
    const GameUnitRow& row = host.slots[host.controlled_index]->row;
    host.log.notef("  controlled unit frame %-4llu t=%7.2f  x=%10.2f z=%10.2f  "
        "heading=%8.3f  fwd=%7.3f  throttle=%6.3f rudder=%6.3f  yaw=%8.5f",
        mission_frame, static_cast<double>(host.summary.simulated_seconds),
        static_cast<double>(row.position[0]), static_cast<double>(row.position[2]),
        static_cast<double>(row.heading_degrees), static_cast<double>(row.forward_speed),
        static_cast<double>(row.throttle), static_cast<double>(row.rudder),
        static_cast<double>(row.yaw_rate));
}

void GameUnitsHost::report() {
    Impl& host = *impl_;
    if (host.slots.empty()) return;
    host.log.notef("unit motion: %llu motion step(s) of %llu unit tick(s) over %.2f s of "
        "simulated time, %llu instance update(s) of 008255b0",
        host.summary.motion_steps, host.summary.motion_ticks,
        static_cast<double>(host.summary.simulated_seconds), host.summary.instance_updates);
    // Milestone 2k added the ordered pair each unit is running under, which is
    // unit+980h / unit+984h as the ring published them. Milestone 2l adds the
    // authored command and what its latch captured, and the two columns now
    // disagree on purpose: `cruise` writes the director's +243h / +244h / +248h
    // and never the ring, so a ship under an authored command alone shows a
    // zero ordered pair and does not move.
    host.log.notef("  %-20s %-12s %5s %5s %8s %8s %-8s %7s %9s %9s %8s %5s", "unit", "type",
        "party", "class", "throttle", "rudder", "command", "latched", "x", "z", "moved",
        "gate");
    for (const std::unique_ptr<GameUnitSlot>& owned : host.slots) {
        const GameUnitRow& row = owned->row;
        char latched[16];
        if (row.command_latched) {
            std::snprintf(latched, sizeof(latched), "%s%.2f",
                row.latch_is_heading ? "h" : "r",
                static_cast<double>(row.latch_steer));
        } else {
            std::snprintf(latched, sizeof(latched), "-");
        }
        host.log.notef("  %-20s %-12s %5d %5d %8.3f %8.3f %-8s %7s %9.1f %9.1f %8.2f "
            "%5d%s", row.name.c_str(), row.type_symbol.c_str(), row.party, owned->class_id,
            static_cast<double>(row.throttle), static_cast<double>(row.ordered_rudder),
            row.command.empty() ? "-" : row.command.c_str(), latched,
            static_cast<double>(row.position[0]), static_cast<double>(row.position[2]),
            static_cast<double>(row.distance), row.command_applied ? 1 : 0,
            row.controlled ? "  <- controlled" : "");
    }
    // Milestone 2q: what 00822C20's property-bag arm seeded at creation.
    // `ratio` is the float32 008235CA stored into ring+148h and `axial` the
    // float32 008235EC handed to 0092D770; `expected` is ratio times the class
    // reference speed times the seconds simulated, the straight-run distance a
    // cruise ship that never changes throttle should cover.
    host.log.notef("  %-20s %10s %10s %8s %9s %10s %10s %10s", "unit", "StartSpeed",
        "reference", "ratio", "axial", "expected", "moved", "delta");
    std::size_t seeded = 0;
    for (const std::unique_ptr<GameUnitSlot>& owned : host.slots) {
        const GameUnitRow& row = owned->row;
        if (!row.start_speed_authored || row.start_speed == 0.0f) continue;
        ++seeded;
        const double expected = static_cast<double>(row.start_speed_ratio)
            * static_cast<double>(row.max_speed)
            * static_cast<double>(host.summary.simulated_seconds);
        host.log.notef("  %-20s %10.4f %10.4f %8.4f %9.4f %10.2f %10.2f %10.2f",
            row.name.c_str(), static_cast<double>(row.start_speed),
            static_cast<double>(row.max_speed),
            static_cast<double>(row.start_speed_ratio),
            static_cast<double>(row.start_speed_axial), expected,
            static_cast<double>(row.distance),
            static_cast<double>(row.distance) - expected);
    }
    host.log.notef("summary mission start speed seeds=%zu non_zero=%zu key=%s",
        host.summary.start_speed_seeds, seeded, bsp::kSceneUnitStartSpeedKey);
    host.commands.report();
}

}  // namespace bsp::game
