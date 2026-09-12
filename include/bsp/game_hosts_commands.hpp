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
#include "bsp/ship_ai_states.hpp"
#include "bsp/unit_commanded_speed.hpp"
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
    // Milestone 2m. Who issued this command: the scene's own authored token, one
    // of the mission script's navigator bindings, `--order`, or the weapon
    // director's own idle tail at 00836dc9.
    std::string source{"scene"};
};

// Milestone 2m. What 00836920's stage spine did to one director in one fixed
// simulation step. Every field is the answer of a recovered test.
struct GameDirectorStepOutcome {
    bool ran{false};
    bool prepass_flag{false};     // the local flag at [ESP+0Bh], 00836941
    bool stop_arm_raised{false};  // 00836a8b raised the primary stage to 2
    bsp::DirectorDefaultCommand reissued{bsp::DirectorDefaultCommand::None};
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
    // Milestone 2m: the director stage ladder and the commanded-speed pair.
    unsigned long long director_steps{0};   // 00836920 bodies run
    unsigned long long idle_reissues{0};    // 00836dc9 chose a default command
    std::size_t idle_stop{0};
    std::size_t idle_cruise{0};
    std::size_t idle_follow{0};
    std::size_t commanded_speeds{0};        // units whose +28h is active
    std::size_t script_issues{0};           // commands the mission script issued
    std::size_t script_blocked{0};          // of those, stopped at 00816f7c
    // Milestone 2q: the completion round trip, 0071E430 through 00721A40's 5Dh
    // arm and out the other side into 00720850.
    unsigned long long end_commands{0};      // 0071E430 bodies
    unsigned long long stage_raises{0};      // 0071D810 raises that changed the stage
    unsigned long long clear_messages{0};    // 0071C730 messages built and routed
    unsigned long long clear_receives{0};    // 00721A40's 5Dh arm bodies
    unsigned long long queue_advances{0};    // 00720850 bodies
    unsigned long long command_events{0};    // 00984300 bodies
    unsigned long long command_event_callbacks{0};  // Lua handlers the channel matched
    unsigned long long restarts{0};          // 0071E430's arm D
};

// What one 0071E430 -> 5Dh -> 00720850 round trip did, for the report.
struct GameCommandCompletion {
    bool ran{false};
    std::string arm;                 // which arm of 0071E430 answered
    int requested_stage{0};
    bool raised_queue_stage{false};
    bool message_routed{false};      // the 5Dh message was built and delivered
    bool queue_advanced{false};      // 00720850 ran on slot 0
    bool restarted_head{false};      // arm D
    std::uint32_t promoted_command{0};  // what slot 0 holds afterwards
    int stage_after{0};
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

    // Milestone 2m. The navigator bindings' own issue. 008a30d0 and 008a2f20
    // push a fixed command object and the descriptor 0088a810 built, so the
    // chain starts at 0077d600 rather than at 0046aab0's registry walk; the
    // rest of the hops are the same ones `issue` runs.
    const GameCommandRow* issue_command_object(std::size_t unit_index,
        std::uint32_t command_object, const bsp::SceneCommandTarget& target,
        int flags, const std::string& source, const std::string& target_name,
        const bsp::UnitOrderRing& ring, float heading_radians);

    // Milestone 2m. The commanded-speed store both producers make on the
    // navigator parameter block at *(unit+73Ch): +24h = max(requested, 0) and
    // +28h = the mission clock (00890e6f in luaMW_SetShipSpeed, 008a38d5 in
    // luaMW_NavigatorMoveOnPath). The director reaches the same block through
    // [director+24Ch]+73Ch, which is why this process holds it beside the
    // director rather than inside it.
    void store_commanded_speed_00890e6f(std::size_t unit_index, float requested,
        float mission_clock);
    bsp::CruiseSpeedSetting commanded_speed(std::size_t unit_index) const;

    // Milestone 2m. 00836920's stage spine for one unit, once per fixed
    // simulation step: the pre-pass 00836941, the `stop` arm 00836a8b and the
    // idle tail 00836dc9 that re-issues a default command through 0071ecf0.
    GameDirectorStepOutcome director_step_00836920(std::size_t unit_index,
        bool player_controlled, float mission_clock, const bsp::UnitOrderRing& ring,
        float heading_radians);

    // Milestone 2q. 0071E430 BSP_WeaponDirector_EndCommand on one unit's
    // director, and the whole consequence the image gives it: a stage that
    // reaches 2 builds MT_GAMEUNIT_CLEARCMD (5Dh) through 0071C730, this
    // process routes that message back into its own receiver 00721A40, whose
    // 5Dh arm runs 00720850 on the named slot, and the queue advances with the
    // stage pair cleared by 0071C130. The call sites are 009E5997 (movetopos),
    // 009E5C70 (moveonpath) and 009E88C1 / 009F3718 (attackmove).
    GameCommandCompletion end_command_0071e430(std::size_t unit_index,
        std::uint32_t command_object, bool terminal, bool player_controlled,
        const bsp::UnitOrderRing& ring, float heading_radians);

    // 00984300 BSP_MissionEvents_ReportCommand with the `finished` literal at
    // 00D09FD8, the call the state steps make at 009E595C just before they end
    // the command. Returns how many Lua handlers the `command` channel matched.
    std::size_t report_command_event_00984300(std::size_t unit_index,
        std::uint32_t command_object, const char* status);

    // Milestone 2p. 0071eb60 on the unit's own weapon director, the routine
    // 009f1420's brain pre-pass calls at 009f146b with ECX = [brain+0ab8h]:
    // director+30h == 1 hands back the slot-0 descriptor at director+58h,
    // == 2 the override descriptor at director+18ch, anything else the empty
    // singleton at 00e19b98. False is that singleton; `mode` is the raw +30h.
    bool active_command_descriptor_0071eb60(std::size_t unit_index,
        bsp::SceneCommandTarget& out, int& mode) const;
    // 00521ea0 BSP_CommandTarget_ResolveObject on a descriptor: the created
    // instance its +2h id names, one-based, or 0.
    std::uint32_t resolve_command_target_00521ea0(const bsp::SceneCommandTarget& target) const;
    // 0071df70's first test: the float at director+40h. See GameDirector for
    // the three producers and which of them this process reaches.
    float director_target_hold_0040(std::size_t unit_index) const;
    // 0071df83..0071dfc2: the categories of the leading occupied command slots
    // at director+54h, stride 1ch, stopped at the first null. Returns how many
    // were written; a category of 1 or 2 rejects the automatic target think.
    int director_leading_slot_categories_0071df83(std::size_t unit_index, int* out,
        int max_out) const;
    // The command object one slot carries, for the report's "what slot 0 holds".
    std::uint32_t director_slot_command(std::size_t unit_index, int slot_index) const;
    // The registry name of a command object, or "" when no row matches.
    const char* command_name_of(std::uint32_t command_object) const;

    // 0071be40 with the director's own mode: does this unit hold `cruise`?
    bool holds_cruise(std::size_t unit_index) const;
    // Milestone 2n. The same read without the `cruise` test, which is what
    // 009f3dd0 asks the director for at 009f3de6 before it picks an AI state.
    std::uint32_t current_command_0071be40(std::size_t unit_index) const;

    // 009e1170's AI arm for one unit. False when the unit holds no current
    // `cruise`, which is the state 009f3dd0 would have left the AI state in.
    // `player_controlled` is unit+184h: with it set the native takes the second
    // arm at 009e11e8 instead, which forwards the ring's confirmed pair at
    // +15ch / +160h and touches no cruise field. That arm is not projected, so
    // it is recorded and the AI arm is not run in its place.
    //
    // Milestone 2n: `blk` and `setters` are the AI controller's own control
    // block, blk = brain+8h. With them the three setters run the reconstructions
    // include/bsp/ship_ai_states.hpp carries instead of recording the three
    // addresses, so the desired throttle, rudder and heading are the clamped
    // values 009dbf90 and 009dffb0 store and 009ed6b0 reads.
    bool cruise_step(std::size_t unit_index, bool player_controlled,
        float body_axis_speed, float reference_speed, bsp::CruiseOrderedValues& out,
        bsp::ShipAiControlBlock* blk = nullptr, bsp::ShipAiSetterHost* setters = nullptr);

    const std::vector<GameCommandRow>& rows() const noexcept;
    const GameCommandsSummary& summary() const noexcept;

    // The per-unit table and the one-line summary the milestone reports.
    void report();

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
