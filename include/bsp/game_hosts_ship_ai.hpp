#pragma once
// bsp_game.exe milestone 2n: the ship AI controller per unit, and the weapon
// director's own automatic target selector.
//
// Addresses: 009f50e0 (the AI controller's frame sequence, sixteen steps),
// 009f3dd0 (which state the director's current command asks for), 009f3d00
// (the state the command object selects) with 00779aa0, the nine state objects
// of the family at 00d21598 and their vtable slots +0ch (the step), +24h (the
// command object) and +28h (the re-plan interval), 009ed6b0 (the direct-control
// arm that turns the desired values into a heading target and two distances),
// 009f4d10 with 00811960 (the publish into the unit's 84-byte AI order slot at
// unit+0aech - 84*[unit+0b40h]), 00825f2c..00825f7c with 00811d10 (the motion
// head's promotion of that slot) and 009f5da0 with 009f5610, 009f5d30, 009f5b70,
// 009f52f0 and 00835860 (the director's once-a-second automatic target think).
//
// Nothing in this file is a reconstruction of native code. Every method is one
// call site of bsp::ShipAiControllerHost, bsp::ShipAiSyncHost,
// bsp::ShipAiSetterHost, bsp::ShipAiDirectControlHost, bsp::ShipAiPublishHost or
// bsp::BotFireTargetHost, satisfied either by a reconstruction already on main
// or by the explicit unimplemented policy in GameHostLog.
//
// What this file supplies rather than recovers, each labelled at its site:
//   - where 009f50e0 and 009f5da0 run in a frame. Neither has a caller in the
//     call graph (009f50e0 has no Ghidra function at all and 009f5da0 is reached
//     only through the derived vtable slot 00d21b4c), so the executable runs the
//     controller once per unit per fixed simulation step, before the motion pass
//     whose head consumes what the controller published, and the target think
//     beside it with the same step delta.
//   - the party list 009f5d30 scans. Its source is the recon slot 008053c0
//     returns for the owner's party and the intrusive list at slot+0de8h, which
//     nothing in this process fills, so the executable hands the recovered scan
//     the created instances of the opposing party and records 008053c0. This is
//     the same substitution milestone 2i makes for walk 0 of 004c3cb0.
//   - the AI's own goal vector at brain+0b2ch / +0b34h / +0b38h has no recovered
//     writer, so every state step but `cruise` is a record with its own address.
//
// Evidence: docs/SHIP_AI_STATES.md, docs/UNIT_AUTOPILOT_PAIR.md,
// docs/BOT_FIRE_TARGET.md, docs/CRUISE_COMMAND.md, docs/UNIT_RUDDER_CURVE.md,
// docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bsp/bot_fire_target.hpp"
#include "bsp/ship_ai_states.hpp"
#include "bsp/unit_autopilot_pair.hpp"

namespace bsp::game {

class GameHostLog;
class GameUnitsHost;

// 009F3D00's own dispatch, read from the image at 009F3D04..009F3DA0. Each row
// is one `CMP EAX,<command>` and the `LEA EDI,[ESI+<ai offset>]` it takes; the
// brain offset is the ai offset minus 58h, which is what docs/SHIP_AI_STATES.md
// tabulates. The two attack rows share one arm and are not in this table.
struct ShipAiCommandStateRow {
    std::uint32_t command_object;  // the singleton 009F3DD0 read from the director
    std::uint32_t ai_offset;       // LEA ESI+<this>
    const char* state;             // the literal 009F39C0 registered
    std::uint32_t compare_site;    // the CMP that selects it
};

// The rows in the order 009F3D00 tests them.
inline constexpr ShipAiCommandStateRow kShipAiCommandStates[] = {
    {0x00e08f68u, 0x0C5Cu, "movetopos",  0x009f3d04u},
    {0x00e08fa0u, 0x0C38u, "land",       0x009f3d1au},
    {0x00e08f88u, 0x0BD8u, "stop",       0x009f3d29u},
    {0x00e08f80u, 0x0C64u, "moveonpath", 0x009f3d38u},
    {0x00e08f60u, 0x0BE4u, "follow",     0x009f3d47u},
    {0x00e08f70u, 0x0BC8u, "cruise",     0x009f3d64u},
};
inline constexpr int kShipAiCommandStateCount =
    static_cast<int>(sizeof(kShipAiCommandStates) / sizeof(kShipAiCommandStates[0]));

// The two command objects that share the attack arm at 009F3D56..009F3D96.
inline constexpr std::uint32_t kShipAiArtilleryCommandObject = 0x00e08f10u;
inline constexpr std::uint32_t kShipAiAttackMoveCommandObject = 0x00e08f78u;
// 009F3DA0: every command object no row matches falls to the `stop` state.
inline constexpr std::uint32_t kShipAiDefaultStateAiOffset = 0x0BD8u;

// One unit's AI controller as this process holds it, for the run log and the
// report. Every value is what a recovered routine left behind.
struct GameShipAiRow {
    std::size_t unit_index{0};
    std::string unit;
    // 009F3DD0 / 009F3D00
    std::uint32_t command_object{0};
    std::string state;              // the state the command selected
    std::uint32_t state_step{0};    // that state's vtable +0Ch
    bool state_step_concrete{false};
    unsigned long long state_changes{0};
    // 009F50E0's own counters
    unsigned long long controller_steps{0};  // bodies that passed all three gates
    unsigned long long gated{0};             // steps one of the gates stopped
    unsigned long long replans{0};
    float replan_interval{0.0f};             // vtable +28h times 0.05f
    // the control block blk = brain+8h
    int steering_mode{0};
    float desired_throttle{0.0f};
    float desired_rudder{0.0f};
    float desired_heading{0.0f};
    int latched_direction{0};
    float heading_target{0.0f};     // blk+324h
    float distance_32c{0.0f};
    float distance_330{0.0f};
    bool distance_finite{true};
    // the 84-byte order slot the publish fills and the motion head promotes
    unsigned long long publishes{0};
    unsigned long long promotions{0};
    int slot_index{0};
    float slot_heading_44{0.0f};
    float slot_distance_40{0.0f};
    float slot_distance_48{0.0f};
    bool slot_valid{false};
    // Milestone 2o: the hop at the tail of 009F3F80, chain slot 16.
    unsigned long long ring_hops{0};        // bodies of 009F3F80 that reached 009F4B99
    unsigned long long ring_gated_3f5{0};   // steps the 009F3FF2 gate on blk+3F5h stopped
    unsigned long long ring_writes{0};      // 0080E170 / 0080E190 pairs written
    unsigned long long rudder_law_calls{0}; // 009F44F7, the 009F44E4 gate open
    unsigned long long rudder_deadbands{0}; // the 009F4BC6 arm that zeroes blk+1D4h
    unsigned long long live_pair_changes{0}; // steps on which 00813020 moved +148h/+14Ch
    float ring_slot_throttle{0.0f};  // what 0080E170 last received
    float ring_slot_rudder{0.0f};    // what 0080E190 last received
    float ring_live_throttle{0.0f};  // ring+148h as the motion read it
    float ring_live_rudder{0.0f};    // ring+14Ch
    float heading_error{0.0f};       // 00438B10(blk+324h, heading) at 009F40BB
    bool ai_driven{false};           // --ai-drive named this unit
    // 009F5DA0, the automatic target selector
    unsigned long long target_thinks{0};   // ticks whose countdown was spent
    unsigned long long target_scans{0};
    unsigned long long fire_target_sets{0};
    unsigned long long attackmove_issues{0};
    std::string fire_target;               // the entity 00835860 last received
    float fire_target_score{0.0f};
    std::string target_blocked;            // the gate that stopped the think
};

struct GameShipAiSummary {
    std::size_t units{0};             // controllers built
    std::size_t ai_owned{0};          // units whose +184h is clear
    unsigned long long steps{0};      // 009F50E0 bodies run
    unsigned long long gated{0};
    unsigned long long replans{0};
    unsigned long long state_steps_concrete{0};
    unsigned long long state_steps_recorded{0};
    unsigned long long publishes{0};
    unsigned long long promotions{0};
    // Milestone 2o.
    unsigned long long ring_hops{0};
    unsigned long long ring_gated_3f5{0};
    unsigned long long ring_writes{0};
    unsigned long long rudder_law_calls{0};
    unsigned long long rudder_deadbands{0};
    unsigned long long live_pair_changes{0};
    std::size_t units_driven{0};
    unsigned long long thinks{0};
    unsigned long long scans{0};
    unsigned long long fire_target_sets{0};
    unsigned long long attackmove_issues{0};
    std::size_t units_with_fire_target{0};
    std::size_t states_cruise{0};
    std::size_t states_stop{0};
    std::size_t states_attackmove{0};
    std::size_t states_movetopos{0};
    std::size_t states_other{0};
};

// One ship AI controller per created unit, for the whole run.
class GameShipAiHost {
public:
    GameShipAiHost(GameHostLog& log, GameUnitsHost& units);
    ~GameShipAiHost();
    GameShipAiHost(const GameShipAiHost&) = delete;
    GameShipAiHost& operator=(const GameShipAiHost&) = delete;

    // One controller per created instance, in creation order. Called once,
    // after the instantiate pass and after the authored commands were issued.
    void register_units();

    // 009F50E0 for every registered unit, once per fixed simulation step, and
    // 009F5DA0 beside it. Run before the motion pass, because the motion's head
    // at 00825F2C consumes the slot 009F4D10 published.
    void controller_step(float seconds);

    // 00825F2C..00825F7C, the motion head's own promotion of the slot the
    // controller published. Returns true when a valid order was promoted.
    bool promote_order_00825f2c(std::size_t unit_index);

    // --ai-drive <name>=<throttle>,<rudder>, milestone 2o. A LABELLED
    // DIAGNOSTIC STAND-IN, not a reconstruction: eight of the nine state steps
    // have no body, so on a re-plan tick of a unit named here the executable
    // calls the two recovered setters 009DBF90 and 009DFFB0 on that unit's own
    // control block in place of the state step's decision. Everything after
    // that point - 009ED6B0, 009F4D10, 009F4DA0, 009F3F80's hop, 00813020 and
    // 00825F20 - is the game's own recovered path.
    void set_ai_drive(std::size_t unit_index, float throttle, float rudder);

    const std::vector<GameShipAiRow>& rows() const noexcept;
    const GameShipAiSummary& summary() const noexcept;

    // The per-unit table and the one-line summaries the milestone reports.
    void report();
    // One sampled line per unit every `interval` controller steps.
    void log_sample(unsigned long long step_index, unsigned long long interval);

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
