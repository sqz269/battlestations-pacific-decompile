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

#include "bsp/camera_projection.hpp"
#include "bsp/controlled_unit.hpp"
#include "bsp/ocean_height.hpp"
#include "bsp/pose_refresh.hpp"
#include "bsp/rigid_body_integration.hpp"
#include "bsp/ship_class_fields.hpp"
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
    // the unit's force-model controller holds at controller+2Ch. No producer for
    // the mass, the inertia or either damping rate was found
    // (docs/RIGID_BODY_INTEGRATION.md, follow-up `ship_hull_body_creation`), so
    // 00c37f40, 00c37e70, 00c37e00 and 00c37de0 are never called here and the
    // fields keep the values a freshly constructed body has.
    bsp::DynMotionState motion_state{};
    bsp::DynBody body{};

    int class_id{bsp::kUnitDestroyerClassId};  // unit+C4h, the descriptor's kind
    bool standing_order{false};
    float standing_throttle{0.0f};
    float standing_rudder{0.0f};
};

struct GameUnitsHost::Impl {
    Impl(GameHostLog& log_in, GameMissionLuaHost& lua_in) : log(log_in), lua(lua_in) {}

    GameHostLog& log;
    GameMissionLuaHost& lua;
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
        // class+A0h and class+A8h place the keel sample point. Only +A0h has a
        // recovered Lua key (bsp/ship_class_fields.hpp: 00960230 writes it from
        // `Length`), and with a flat sea at y = 0 and an upright hull the gate
        // at 00826994 passes either way, which is what the probe reports.
        slot->motion_class.hull_length = 0.0f;
        slot->motion_class.hull_height = 0.0f;
        slot->motion_class.hull_mass = 0.0f;   // class+B0h, only 00937440 reads it
        slot->motion_class.boost_refill_time = 1.0f;
        slot->motion.thrust_mod = 1.0f;        // 00823714
        slot->motion.turn_efficiency = 1.0f;   // 0082371c
        slot->motion.class_id = slot->class_id;

        bsp::construct_unit_order_ring_00812d40(slot->ring);

        // M+18h and M+1Ch, the two speed clamps 00c5b1b0 applies at the end of
        // every substep. Their producer was not found either, and a zero there
        // would zero the velocity on the first substep, so they are set out of
        // range and the clamps never fire. That is a stated contract, not a
        // recovered value.
        slot->motion_state.max_linear_speed = 1.0e30f;
        slot->motion_state.max_angular_speed = 1.0e30f;
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
    host.log.notef("world units: %zu created instance(s) carried into the frame, %zu with a "
        "VehicleClass row out of the installed table", host.summary.units,
        host.summary.class_rows);
}

void GameUnitsHost::issue_authored_commands() {
    Impl& host = *impl_;
    if (!host.logged_cruise) {
        host.logged_cruise = true;
        host.log.notef("authored command stand-in: the `Command = E CommandType : Cruise` "
            "token every DestroyerGen of this scene carries is queued by 00469610 and "
            "resolved by 0046aab0 against a command registry whose command objects have no "
            "reconstruction, so the executable turns the token into one order-ring order "
            "(throttle 1, rudder 0) through the recovered 00816a40 and says so");
    }
    for (std::unique_ptr<GameUnitSlot>& slot : host.slots) {
        slot->row.command = "Cruise";
        slot->standing_order = true;
        slot->standing_throttle = 1.0f;
        slot->standing_rudder = 0.0f;
        host.issue_into_ring(*slot, slot->standing_throttle, slot->standing_rudder);
        ++host.summary.cruise_orders;
    }
    host.record("SceneCommand::resolve_command_object", 0x0046aab0u);
}

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
    ++host.summary.motion_steps;
    host.summary.simulated_seconds += step_seconds;
    for (std::unique_ptr<GameUnitSlot>& owned : host.slots) {
        GameUnitSlot& slot = *owned;
        if (!slot.state->active) continue;
        // The order under the write cursor is refilled every step so a standing
        // order keeps standing: the game does that from the HUD every frame the
        // key is held, and the ring's own forward copy at 00813186 would
        // otherwise only mark later slots predicted.
        if (slot.standing_order) {
            host.issue_into_ring(slot, slot.standing_throttle, slot.standing_rudder);
        }
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
                "%.4f s game step. The substep schedule 00c5c540 is a record because "
                "world+00h has no recovered producer, and the hull body carries no mass, "
                "inertia or damping because 00c37f40 / 00c37e70 / 00c37e00 / 00c37de0 have "
                "no caller on the hull path", static_cast<double>(step_seconds));
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
    host.log.notef("  %-20s %-12s %5s %5s %9s %9s %9s %9s %8s %5s", "unit", "type", "party",
        "class", "start x", "start z", "x", "z", "moved", "gate");
    for (const std::unique_ptr<GameUnitSlot>& owned : host.slots) {
        const GameUnitRow& row = owned->row;
        host.log.notef("  %-20s %-12s %5d %5d %9.1f %9.1f %9.1f %9.1f %8.2f %5d%s",
            row.name.c_str(), row.type_symbol.c_str(), row.party, owned->class_id,
            static_cast<double>(row.start[0]), static_cast<double>(row.start[2]),
            static_cast<double>(row.position[0]), static_cast<double>(row.position[2]),
            static_cast<double>(row.distance), row.command_applied ? 1 : 0,
            row.controlled ? "  <- controlled" : "");
    }
}

}  // namespace bsp::game
