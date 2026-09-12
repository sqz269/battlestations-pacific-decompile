#pragma once
// bsp_game.exe milestone 2l: the mission's orders as the game issues them.
//
// Addresses: 0046aab0 (the scene deferred-reference resolve, call site 0046ac0b),
// 0077d600 BSP_Entity_IssueCommand with the builder 007798d0, 00816e30 (the
// unit's MT_COMMAND apply, primary vtable slot 160h), 0071ecf0 (the weapon
// director's issue hop) with 0071d880 / 0071e550 / 0071c830, 00721a40's 5Ch arm
// (the MT_GAMEUNIT_SETCMD receive) with 007216d0, 008358d0 and 0071e6c0 (the
// command-slot push), 0071be40 (the current-command read), 00835c70's `cruise`
// arm with the latch 00835ac0, and 009e1170's AI arm with the three controller
// setters 009dbf90 / 009dffb0 / 009e0040.
//
// Nothing in this file is a reconstruction of native code. Every method is one
// call site of bsp::SceneDeferredReferenceHost, bsp::EntityOrderHost or
// bsp::CruiseCommandHost, satisfied either by a reconstruction already on main
// or by the explicit unimplemented policy in GameHostLog.
//
// Milestone 2i turned the authored `Command = E CommandType : Cruise` token into
// one order-ring order of throttle 1 and rudder 0, because no reconstruction of
// the command object existed. docs/CRUISE_COMMAND.md recovered it, and the
// answer is the opposite of a throttle: `cruise` is a latch that captures the
// ring's ordered pair and the heading when it becomes current and re-applies
// what it captured, so a ship whose ring is zero stays stopped. This file runs
// that path instead, and the ring is no longer written by an authored command.
//
// What this file supplies rather than recovers, each labelled at its site:
//   - the session. 0077c2a0's routing, the local queue on session+24ch and the
//     drain 0076c600 / 00780670 / 00780120 are records, and the executable
//     delivers each of the three messages to the same process synchronously.
//   - the `every slot` clear. 0071d880 builds MT_GAMEUNIT_CLEARCMD with +24h =
//     -1 and the receive arm 00720ca0 is not projected, so the executable
//     performs the documented clear and records both addresses.
//   - where 00835c70 and 009e1170 run in a frame. 00835c70 has no direct caller
//     (00d09fd0 and 00d09fd4, the director vtable's slot 78h, are its only
//     references) and 009e1170 belongs to the ship AI state class family at
//     00d21598, which has no reconstruction, so the executable runs the first
//     right after the push that made the command current and the second once per
//     unit per fixed simulation step.
//
// Evidence: docs/CRUISE_COMMAND.md, docs/SCENE_COMMAND_TYPES.md,
// docs/ENTITY_ORDER_MESSAGE.md, docs/SCENE_DEFERRED_REFS.md,
// docs/WEAPON_DIRECTOR.md, docs/UNIT_STATE_MESSAGE.md, docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bsp/cruise_command.hpp"
#include "bsp/scene_deferred_refs.hpp"
#include "bsp/unit_state_message.hpp"

namespace bsp::game {

class GameHostLog;

// One created instance as the command path sees it. `object_id` stands in for
// entity+174h: the two handle tables at 00f89a0c / 00f89a60 are not built in
// this process, so the executable numbers its own entities and says so.
struct GameCommandUnit {
    std::size_t index{0};
    std::string name;
    std::uint16_t object_id{0};
    float position[3]{};
};

// What one issued command did, end to end. Every flag is the answer of a
// recovered test, not a decision of this file.
struct GameCommandRow {
    std::size_t unit_index{0};
    std::string unit;
    std::string token;          // the authored `Command` token, or --order's name
    std::string target_token;   // the authored `CommandTarget`, "" when unset
    std::string command;        // the registry name the token matched
    int ordinal{-1};
    int category{-1};
    std::string resolve_outcome;   // 0046aab0's own outcome enum
    bool issued{false};            // 0077d600 built and routed MT_COMMAND
    bool ai_group_notified{false}; // 0077d787..0077d7a3, the entity's own group
    bool ai_group_forwarded{false};// 0071ed54, the endpoint subject's group
    bool projected_arm{false};     // 00816e30's movement fall-through was taken
    bool slots_cleared{false};     // the flags != 0 arm reached 0071d880
    bool slot_pushed{false};       // 0071e6c0 stored a slot
    int slot_index{-1};
    bool current{false};           // 0071be40 answers this command
    bool latched{false};           // 00835c70's `cruise` arm ran 00835ac0
    bsp::CruiseAutopilotFields fields{};
    float descriptor_position[3]{};
    unsigned long long steps{0};   // 009e1170 runs over this unit
    std::string blocked;           // why the latched pair reaches no ring
};

struct GameCommandsSummary {
    std::size_t units{0};
    std::size_t resolved{0};
    std::size_t issued{0};
    std::size_t pushed{0};
    std::size_t current{0};
    std::size_t latched{0};
    std::size_t moving{0};         // latched with a non-zero thrust
    std::size_t ai_groups{0};      // entity+16ch non-null
    std::size_t ai_forwards{0};
    unsigned long long steps{0};
    std::string command_name;      // the one token this mission authors
};

// The weapon directors this process owns, one per created unit, and the three
// message hops between them.
class GameCommandsHost {
public:
    explicit GameCommandsHost(GameHostLog& log);
    ~GameCommandsHost();
    GameCommandsHost(const GameCommandsHost&) = delete;
    GameCommandsHost& operator=(const GameCommandsHost&) = delete;

    // The entities 0046aab0's target lookup 00925a90 would find, and the owners
    // its records name. Called once, after the instantiate pass.
    void register_units(std::vector<GameCommandUnit> units);

    // The whole chain for one authored record: 0046aab0 resolves the token
    // against the 26-row registry, 0077d600 builds and routes MT_COMMAND,
    // 00816e30 applies it, 0071ecf0 issues it to the director, 00721a40's 5Ch
    // arm receives MT_GAMEUNIT_SETCMD, 008358d0 and 0071e6c0 push the slot, and
    // 00835c70's own arm latches when the pushed command became current.
    // `ring` is the unit's live order ring and `heading_radians` its vtable[50h].
    const GameCommandRow* issue(std::size_t unit_index, const std::string& token,
        const std::string& target_token, const bsp::UnitOrderRing& ring,
        float heading_radians);

    // 0071be40 with the director's own mode: does this unit hold `cruise`?
    bool holds_cruise(std::size_t unit_index) const;

    // 009e1170's AI arm for one unit. False when the unit holds no current
    // `cruise`, which is the state 009f3dd0 would have left the AI state in.
    // `player_controlled` is unit+184h: with it set the native takes the second
    // arm at 009e11e8 instead, which forwards the ring's confirmed pair at
    // +15ch / +160h and touches no cruise field. That arm is not projected, so
    // it is recorded and the AI arm is not run in its place.
    bool cruise_step(std::size_t unit_index, bool player_controlled,
        float body_axis_speed, float reference_speed, bsp::CruiseOrderedValues& out);

    const std::vector<GameCommandRow>& rows() const noexcept;
    const GameCommandsSummary& summary() const noexcept;

    // The per-unit table and the one-line summary the milestone reports.
    void report();

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
