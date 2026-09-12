#pragma once
// bsp_game.exe milestone 2i: the created scene units, as process bindings.
//
// Addresses: 008255b0 (BSP_UnitInstance_Update, the entity vtable slot 0DCh the
// world walk calls), 00825f20 (BSP_UnitInstance_UpdateShipMotion, the separate
// motion virtual) with 00813020 (the order ring tick), 0092d300 / 0092e8c0 /
// 00937440 (the speed command, the steering half and the force model),
// 0092be80 (the controller step), 0092d730 (the body-axis forward speed),
// 00816a40 / 00815440 (the order record issue and its clamp) and 004c0890
// (BSP_Game_SetControlledUnit).
//
// Nothing in this file is a reconstruction of native code. Every method is one
// call site of bsp::UnitInstanceHost, bsp::ShipMotionHost, bsp::UnitRudderHost,
// bsp::UnitOrderRecordIssueHost, bsp::ControlledUnitQuery or
// bsp::SetControlledUnitHost, satisfied either by a reconstruction already on
// main or by the explicit unimplemented policy in GameHostLog.
//
// Four things this file supplies are stand-ins, each recorded as a host method
// that says so. They are the four docs/SHIP_MOTION.md names for the probe
// src/ship_motion_probe.cpp: the ocean sampler 0078cf20 (a flat sea at y = 0),
// the gameplay scale 008e6430 (the literal 1.0f), the rudder curve settings at
// 00424c40()+438h..+44Ch (the denominator forced to 1) and the rigid-body
// integrator the game reaches through the 00C3xxxx physics imports (an explicit
// Euler step). A fifth is this milestone's own: the authored
// `Command = E CommandType : Cruise` token is turned into one order-ring order,
// because the command object 0046aab0 resolves has no reconstruction.
//
// Evidence: docs/UNIT_INSTANCE_UPDATE.md, docs/SHIP_MOTION.md,
// docs/UNIT_CONTROLLER.md, docs/UNIT_CONTROLLER_UPDATE.md,
// docs/CONTROLLED_UNIT.md, docs/UNIT_STATE_MESSAGE.md,
// docs/UNIT_ORDER_RECORD.md, docs/SCENE_DEFERRED_REFS.md,
// docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bsp/game_hosts_scene_contents.hpp"

namespace bsp::game {

class GameHostLog;
class GameMissionLuaHost;

// One created scene unit as this process holds it, for the run log and the
// report. Positions are world units; the heading is degrees of
// atan2(row2.x, row2.z), which is what the probe prints.
struct GameUnitRow {
    std::string name;         // 0041dd40, the instance name the creator set
    std::string class_name;   // the scene class token, `DestroyerGen` here
    std::string type_symbol;  // the `Type = E ShipClasses : <symbol>` token
    int type_id{-1};          // the symbol resolved through the enum library
    int party{-1};
    std::string command;      // the authored `Command` token
    bool class_row_found{false};  // VehicleClass[type_id] exists
    std::string class_row_name;
    float max_speed{0.0f};
    float max_rot_angle{0.0f};
    bool controlled{false};
    bool active{true};        // unit+5Ch
    float start[3]{};
    float position[3]{};
    float heading_degrees{0.0f};
    float forward_speed{0.0f};
    float throttle{0.0f};     // unit+980h, what the ring published
    float rudder{0.0f};       // controller+80h, the slewed rudder
    float yaw_rate{0.0f};
    float distance{0.0f};     // straight-line, start to current
    float path_length{0.0f};  // summed per step
    unsigned long long instance_updates{0};
    unsigned long long motion_ticks{0};
    bool command_applied{false};  // the throttle gate's last answer
};

struct GameUnitsSummary {
    std::size_t units{0};
    std::size_t class_rows{0};
    std::size_t cruise_orders{0};
    bool controlled_bound{false};
    std::size_t controlled_index{0};
    std::string controlled_name;
    unsigned long long instance_updates{0};
    unsigned long long motion_ticks{0};
    unsigned long long motion_steps{0};   // fixed steps that ran the motion pass
    unsigned long long player_orders{0};
    float simulated_seconds{0.0f};
    float total_path_length{0.0f};
    float controlled_distance{0.0f};
};

// The units the instantiate pass of 004d4df0 created, owned for the whole run.
class GameUnitsHost {
public:
    GameUnitsHost(GameHostLog& log, GameMissionLuaHost& lua);
    ~GameUnitsHost();
    GameUnitsHost(const GameUnitsHost&) = delete;
    GameUnitsHost& operator=(const GameUnitsHost&) = delete;

    // One record per entity the instantiate pass created, in scene order. The
    // class-descriptor floats come from the installed `VehicleClass` global that
    // the recovered global-script step 00886900 loaded.
    void create_units(const std::vector<GameSceneEntityRecord>& entities);

    // The authored `Command` token of each unit, turned into one order through
    // 00816a40 and copied into the ring 00813020 reads. The token-to-order
    // mapping is this milestone's own; see the header comment.
    void issue_authored_commands();

    // 004c0890 on one created unit, through bsp::set_controlled_unit_004c0890.
    void set_controlled_unit_004c0890(std::size_t index);

    // --order throttle=<f>,rudder=<f>: one player order into the controlled
    // unit's ring, through the same 00816a40 the authored command takes.
    void issue_player_order(float throttle, float rudder);

    // The entity vtable slot 0DCh the world walk calls, which for a unit is
    // 008255b0. `index` is the chain position, which is the creation order.
    void update_entity_008255b0(std::size_t index, float scaled_delta);

    // 00825f20 for every active unit, once per fixed simulation step. The
    // virtual's own caller is not established (docs/SHIP_MOTION.md names three
    // call sites and reads none), so running it here is the executable's
    // decision and is recorded as one.
    void motion_step_00825f20(float step_seconds);

    std::size_t count() const noexcept;
    bool unit_active(std::size_t index) const noexcept;
    // The unit whose 5Ch/5Dh/5Eh/60h bytes the list filter reads.
    bool unit_is_kind_of(std::size_t index, int class_id) const;
    int unit_class_id(std::size_t index) const noexcept;

    const std::vector<GameUnitRow>& units() const noexcept;
    const GameUnitsSummary& summary() const noexcept;

    // One line per sampled unit, for --order's trajectory log.
    void log_controlled_trajectory(unsigned long long mission_frame);
    // The end-of-run distance table.
    void report();

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
