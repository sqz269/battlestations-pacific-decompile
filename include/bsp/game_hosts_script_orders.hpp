#pragma once
// bsp_game.exe milestone 2m: the mission script's own orders, run through the
// bindings' recovered bodies instead of being counted.
//
// Addresses: 008a30d0 (NavigatorAttackMove) and 008a2f20 (NavigatorMoveToRange,
// with 008a2bc0 NavigatorMoveToPos and 008a2d70 NavigatorDirectMoveToRange as
// the same body), 0088a810 (the Lua value to SceneCommandTarget reader) with
// 00888aa0 / 00888760 / 00b67910 / 00b65fb0, 00899d10 (JoinFormation) through
// 0077c8d0 with the availability predicate at vtable 16Ch and the type-76h
// message, 00895250 (SetSkillLevel) with vtable 128h, 008ad330 (RepairEnable)
// with vtable 5Ch and the type-9Fh message, and 008ab850 (SetRoleAvailable)
// with 00888d20, 004bca50, vtable 148h, the type-4Ch message and 004c3840.
//
// Milestone 2l ran usn_2_java's own order function and watched it address 21 of
// the 32 created instances through ten bindings that were all host records, so
// no order reached a ship. Packet cc_lua_navigator then read eight of those ten
// rows (docs/LUA_BINDING_NAVIGATOR.md, include/bsp/lua_binding_navigator.hpp,
// src/lua_binding_navigator.cpp). This file is the executable's host for that
// reconstruction: nothing in it is a reconstruction of native code, every method
// is one call site of bsp::LuaBindingNavigatorHost or
// bsp::LuaCommandTargetSource, and each is satisfied either by state this
// process owns or by the explicit unimplemented policy in GameHostLog.
//
// What this file supplies rather than recovers, each labelled at its site:
//   - the entity behind an argument. The native resolves a Lua entity table
//     through 00888aa0 to the native object; this process has no such object and
//     resolves the table's `ID` field, the one 00928a00 seeds, to the created
//     instance of that index. That is the same identity milestone 2l reports.
//   - the object id. entity+174h is the handle-table ordinal; the two tables at
//     00f89a0c / 00f89a60 are not built here, so the executable numbers its own
//     entities, exactly as GameCommandUnit already does.
//
// What stays a record, and therefore stops an order:
//   - vtable 16Ch, the command-availability predicate 008162b0 that 0077c8d0
//     asks before it does anything at all. Its body was not read by the packet
//     that reconstructed the binding, so the host answers the neutral false and
//     JoinFormation has no effect.
//   - vtable 128h (skill), vtable 148h (role) and the three session messages.
//   - 00816e30's own arms 00816f7c..00817330, which is where an issued
//     `attackmove` or `moveto` stops: docs/CRUISE_COMMAND.md reads them in
//     pseudocode and projects none of them, so the command never reaches a
//     weapon director slot. That is packet `entity_command_arms`.
//
// Evidence: docs/LUA_BINDING_NAVIGATOR.md, docs/UNIT_COMMANDED_SPEED.md,
// docs/CRUISE_COMMAND.md, docs/SCENE_COMMAND_TYPES.md, docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/lua_binding_navigator.hpp"

struct lua_State;

namespace bsp::game {

class GameHostLog;
class GameUnitsHost;

// One call of one binding, as the report prints it.
struct GameScriptOrderRow {
    std::string binding;             // the Lua global's name
    std::uint32_t row_address{0};    // the binding table row 006b8610 registers
    std::size_t unit_index{0};
    std::string unit;                // the created instance the call addressed
    std::string command;             // `attackmove` / `moveto`, "" otherwise
    std::string target;              // the named entity, or "(position)"
    bool issued{false};              // 0077d600 built and routed MT_COMMAND
    bool reached_director{false};    // 00816e30's movement fall-through was taken
    std::string blocked;             // why it stopped, when it did
    int skill_level{-1};             // 00895250's argument 1
    int repair{-1};                  // 008ad330's argument 1
    std::string formation_leader;    // 00899d10's argument 1
    int role{-1};                    // 008ab850's argument 1
    int role_value{-1};              // 008ab850's argument 2
};

struct GameScriptOrdersSummary {
    std::size_t calls{0};              // binding calls this host took
    std::size_t attack_moves{0};
    std::size_t move_tos{0};
    std::size_t issued{0};             // commands 0077d600 routed
    std::size_t reached_director{0};   // of those, past 00816e30's arm test
    std::size_t formations_requested{0};
    std::size_t formations_refused{0}; // vtable 16Ch answered false
    std::size_t skills{0};
    std::size_t repairs{0};
    std::size_t roles{0};
    std::size_t units_ordered{0};      // distinct instances a command reached
};

// The host the eight reconstructed binding bodies run over. Owned for the whole
// run because the rows are per run and the units it addresses are the created
// scene instances.
class GameScriptOrdersHost final : public bsp::LuaBindingNavigatorHost,
                                   public bsp::LuaCommandTargetSource {
public:
    GameScriptOrdersHost(GameHostLog& log, GameUnitsHost& units);

    // The eight rows src/lua_binding_navigator.cpp reconstructs. A row this
    // answers false for keeps milestone 2l's record.
    static bool handles(const char* binding_name) noexcept;

    // Runs the named binding's recovered body over `state`'s call frame and
    // returns its Lua result count, which for all eight is zero.
    int dispatch(lua_State* state, const char* binding_name, int argument_count);

    void report();
    const GameScriptOrdersSummary& summary() const noexcept { return summary_; }
    const std::vector<GameScriptOrderRow>& rows() const noexcept { return rows_; }

private:
    // --- bsp::LuaCommandTargetSource, the four reads inside 0088a810 --------
    bool argument_id_field_is_nil(int index) override;
    void* argument_entity(int index) override;
    bool argument_vector3(int index, float out[3]) override;
    std::uint16_t entity_object_id(void* entity) override;

    // --- bsp::LuaBindingNavigatorHost --------------------------------------
    int argument_integer(int index) override;
    bool argument_boolean(int index) override;
    void* argument_ptr_field(int index) override;
    void entity_issue_command(void* entity, std::uint32_t command_object,
        const bsp::SceneCommandTarget& target, int flags) override;
    bool entity_command_is_available(void* entity, const char* command_name,
        void* target) override;
    int entity_route_slot(void* entity) override;
    void slot_counter_increment(int slot) override;
    void session_route_formation_message(void* follower,
        std::uint16_t leader_object_id) override;
    void entity_set_skill_level(void* entity, int level) override;
    bool entity_is_kind_of(void* entity, int class_id) override;
    void entity_set_repair_enabled_field(void* entity, bool enabled) override;
    void session_route_repair_enable_message(void* entity, bool enabled) override;
    int game_session_mode() override;
    int game_effective_game_mode() override;
    void role_owner_set_role_available(void* owner, int role, int value) override;
    void session_route_role_message(void* owner, int role, int value) override;
    void game_assign_party_player_slots(int value) override;

    // The `ID` field 00928a00 seeds, turned into a created instance. Null when
    // the argument is not one of this process's entity tables.
    void* entity_from_argument(int index);
    std::size_t index_of(void* entity) const noexcept;
    std::string name_of(void* entity) const;

    GameHostLog& log_;
    GameUnitsHost& units_;
    lua_State* state_{nullptr};
    int argument_count_{0};
    GameScriptOrderRow* row_{nullptr};
    GameScriptOrdersSummary summary_{};
    std::vector<GameScriptOrderRow> rows_;
    std::vector<std::size_t> ordered_units_;
    bool logged_path_{false};
    bool logged_predicate_{false};
};

}  // namespace bsp::game
