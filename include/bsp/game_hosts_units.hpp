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
// Milestone 2j replaced the four stand-ins milestone 2i carried with the
// producers packet cc_ship_inputs recovered, which are the same four
// src/ship_motion_probe.cpp now uses:
//
//   0078cf20  the ocean sampler, run as the recovered product of the wave field
//             0078c890 and the coverage mask 00b9cf50. The two leaves are host
//             records: their receiver is [[game+19F0h]+A8h], which belongs to
//             the renderer/scene owner this process does not build, and their
//             flat-sea pair (wave 0.0f, mask 1.0f) is the evidenced open-sea
//             state of the real routine (docs/OCEAN_HEIGHT.md).
//   008e6430  the gameplay-modifier product, run over an empty category list,
//             which is the routine's own exact 1.0f rather than a literal.
//   00424c40()+438h..+44Ch  the rudder curve settings, read out of the live Lua
//             state through the reconstructed loader fragment 0083ce56 of
//             0083b5e0 over ShipGlobals["Navigator"]["TurnMultipliers"].
//   00c41550 / 00c5b1b0  the Dyn library's own two integration phases, in place
//             of the explicit Euler step. The hull body's mass, inertia and
//             damping have no recovered producer, so the body carries none and
//             the substep schedule 00c5c540 stays a record.
//
// Milestone 2l removed the last of those decisions. The authored
// `Command = E CommandType : Cruise` token is no longer turned into an
// order-ring order: it runs the recovered command path of
// include/bsp/game_hosts_commands.hpp, and `cruise` turns out to be a latch
// that captures the ring's ordered pair rather than an order that fills it.
// A ship whose ring is zero therefore holds zero, which is what the authored
// state of this mission's 32 destroyers is. `--order` still writes the
// controlled unit's ring through 00816a40, which is the player's own path.
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

#include "bsp/game_hosts_commands.hpp"
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
    std::string command_target;  // the authored `CommandTarget`, "" when unset
    // Milestone 2l: what the recovered command path did with that token. The
    // latched triple is the weapon director's +243h / +244h / +248h after
    // 00835c70's `cruise` arm ran 00835ac0 over the unit's live ring.
    bool command_current{false};
    bool command_latched{false};
    bool latch_is_heading{false};
    float latch_steer{0.0f};
    float latch_thrust{0.0f};
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
    float ordered_rudder{0.0f};  // unit+984h, what the ring published
    float rudder{0.0f};       // controller+80h, the slewed rudder
    float yaw_rate{0.0f};     // the angular velocity's world y
    // dot(angular velocity, pose row 1), the component about the hull's own up
    // axis, which is the one 0092e8c0 slews toward the commanded rate. It
    // equals the world y only while the hull is upright, so the trajectory dump
    // reports this one and not the world component.
    float yaw_rate_up_axis{0.0f};
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

    // Milestone 2j. The head of 0083b5e0 on the live Lua state: run
    // Scripts\datatables\ShipGlobals.lua and read
    // ShipGlobals["Navigator"]["TurnMultipliers"] through the reconstructed
    // loader fragment, which is the producer of the six curve fields at
    // 00424c40()+438h..+44Ch. Called once, before the units are created.
    void load_gameplay_settings_0083b5e0();

    // One record per entity the instantiate pass created, in scene order. The
    // class-descriptor floats come from the installed `VehicleClass` global that
    // the recovered global-script step 00886900 loaded.
    void create_units(const std::vector<GameSceneEntityRecord>& entities);

    // Milestone 2l: the authored `Command` token of each unit, run through the
    // recovered path 0046aab0 -> 0077d600 -> 00816e30 -> 0071ecf0 -> 00721a40
    // -> 008358d0 -> 0071e6c0 and, when the pushed command becomes current,
    // 00835c70's own arm. Nothing here writes an order ring.
    void issue_authored_commands();

    // --order <command name>: the same path, issued to the controlled unit on
    // --order-frame. Returns false when no unit is bound.
    bool issue_player_command(const std::string& token, const std::string& target_token);

    // The command path's own rows and counters, for the report.
    const GameCommandsHost& commands() const noexcept;

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

    // ---- milestone 2k: what the two HUD world screens read off a unit ------
    // 0043f080 BSP_UnitInstance_IsAliveAndVisible, the four-byte filter both the
    // minimap walk (005c1628..005c164a) and the marker gate (006431a8) run.
    // bsp/unit_instance.hpp carries +5Ch and +5Dh; +5Eh and +60h have no field
    // there, so they are treated as the clear a created instance leaves them at.
    bool unit_alive_and_visible(std::size_t index) const;
    // The world matrix rows 0063a6c0 and 00427eb0 read: +CCh right, +DCh up,
    // +ECh forward and +FCh translation. False when the index is out of range.
    bool unit_pose(std::size_t index, float right[3], float up[3], float forward[3],
        float translation[3]) const;
    // [unit+538h] +A0h, +A4h and +A8h, the three class extents 0063a6c0 builds
    // its eight corners from. Only +A0h and +A8h have a recovered Lua key
    // (bsp::ShipMotionClass), and both are zero on this installation's ships;
    // +A4h has no recovered producer at all and is reported as zero.
    void unit_class_extents(std::size_t index, float& forward, float& right,
        float& up) const;
    // One row without rebuilding the flat table, for a per-frame reader.
    const GameUnitRow* unit_row(std::size_t index) const noexcept;
    bool controlled_bound() const noexcept;
    std::size_t controlled_index() const noexcept;

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
