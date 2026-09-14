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

#include "bsp/entity_think_dispatch.hpp"
#include "bsp/lua_binding_mission.hpp"
#include "bsp/lua_binding_navigator.hpp"
#include "bsp/mission_blackout.hpp"

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

// Packet cc_lua_binding_audit. This process's stand-in for the 0x1E4-byte script
// entity CreateScript allocates at 00898841. It carries only the fields the five
// script bindings and the think walk 00929460 read; every other byte of the native
// allocation is untouched here and nothing depends on its layout.
struct GameScriptEntity {
    std::uint32_t id{0};          // entity+174h, the u16 the self-table key formats
    std::string created_for;      // the global CreateScript called
    std::string think_name;       // +1D8h, null when empty
    bool delay_armed{false};      // +1DCh
    float delay_seconds{0.0f};    // +1E0h
    bool initialised{true};       // +5Ch, set by the construct at 0089886F
    bool blocked_5d{false};       // +5Dh
    bool blocked_5e{false};       // +5Eh, the byte DeleteScript tests at 00898BD9
    bool blocked_60{false};       // +60h
    bool dead{false};             // thisTable[key].Dead, set by 00929800 on the kill
    unsigned long long thinks{0};
};

// Packet cc_mission_blackout: what the fade at `*(00E198C4 + A4h) + C0h` did
// across a run. `callbacks` is the count of 005B9969 calls, the only route from
// the native fade back into the mission script.
struct GameBlackoutSummary {
    std::size_t arms{0};              // 005B9BA0 entries
    std::size_t callbacks{0};         // 005B9969 calls
    std::size_t updates{0};           // 005B9800 entries
    std::size_t completions{0};       // the 005B9842 arm
    std::size_t interface_requests{0};  // 004CC460 with id 20h
    std::string last_callback;
    float level{0.0f};
    float remaining{0.0f};
};

struct GameScriptTimerSummary {
    std::size_t scripts_created{0};
    std::size_t think_registrations{0};
    std::size_t waits_armed{0};
    std::size_t clears{0};
    std::size_t deletes{0};
    std::size_t passes{0};
    unsigned long long timed_fires{0};
    unsigned long long untimed_fires{0};
    unsigned long long call_failures{0};
    std::string first_error;
};

// The host the reconstructed binding bodies run over. Owned for the whole run
// because the rows are per run and the units it addresses are the created scene
// instances.
class GameScriptOrdersHost final : public bsp::LuaBindingNavigatorHost,
                                   public bsp::LuaCommandTargetSource,
                                   public bsp::LuaBindingArgumentReader,
                                   public bsp::LuaBindingResultWriter,
                                   public bsp::LuaBindingMissionHost,
                                   public bsp::EntityThinkHost,
                                   public bsp::MissionBlackoutHost {
public:
    GameScriptOrdersHost(GameHostLog& log, GameUnitsHost& units);

    // The eight navigator rows src/lua_binding_navigator.cpp reconstructs plus the
    // nine rows src/lua_binding_mission.cpp reconstructs. A row this answers false
    // for keeps milestone 2l's record.
    static bool handles(const char* binding_name) noexcept;

    // Runs the named binding's recovered body over `state`'s call frame and
    // returns its Lua result count.
    int dispatch(lua_State* state, const char* binding_name, int argument_count);

    // Packet cc_lua_binding_audit: one call of 00929460 with this process's script
    // entities, driven once per mission frame by the caller that owns the step.
    // The machine is the one the first dispatch arrived on; before any binding has
    // been dispatched the pass is a no-op, exactly as the native walk is with an
    // empty list.
    void run_script_timers(float step);
    const GameScriptTimerSummary& timers() const noexcept { return timers_; }
    const GameBlackoutSummary& blackout() const noexcept { return blackout_summary_; }

    // 008980E8 / 0088A330, the single callee of SetThink's reconstructed body
    // (bsp::lua_binding_set_think). Public so the small bsp::LuaBindingCoreHost
    // adapter in the .cpp can reach it without this file growing a second copy of
    // the binding. 0088A34B appends to the pending list only on the null-to-name
    // transition, which bsp::register_pending_think_entity_0088a240 carries.
    void entity_set_think_script_name_0088a330(void* entity, const std::string& name);

    // Packet cc_lua_find_entity: the small bsp::LuaBindingCoreHost adapter in the
    // .cpp reports the SetParty virtuals through this rather than growing its own
    // reference to the log.
    void record_unimplemented(const char* method, const char* address);

    // Packet cc_lua_find_entity: a scene entity that is not a unit but that
    // BSP_SEntity_InitAll 00925F20 still hands to entity virtual slot 39 at
    // 0092604E, so it carries a `thisTable` slot and answers `FindEntity`. The
    // pose is the entity's own world matrix translation at +0FCh, which for a
    // `FixedInstance` marker class is the `localframe` the scene authored and
    // which nothing in the mission moves; that is the value `GetPosition`
    // 008A7B00 reads at 008A7C3C. `id` is this process's entity number, the
    // same substitution for the u16 at +174h that the unit rows already make.
    void register_scene_marker(int id, const std::string& name,
        const float world_position[3]);

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

    // --- bsp::LuaBindingArgumentReader, the reads the nine rows of packet
    // cc_lua_binding_audit make through 00B677E0 --------------------------------
    int count() override;
    int get_integer(int index) override;
    double get_number(int index) override;
    bool get_boolean(int index) override;
    std::string get_string(int index) override;
    bool is_string(int index) override;
    bool is_nil(int index) override;
    bool is_entity_table(int index) override;
    void* entity_at(int index) override;

    // --- bsp::LuaBindingResultWriter, the three pushes of 00B664B0/50/30 --------
    void push_number(int value) override;
    void push_boolean(bool value) override;
    void push_nil() override;

    // --- bsp::LuaBindingMissionHost, one method per native call site ------------
    void push_number_float_00b66480(float value) override;
    float random_uniform_00bd2f10(float minimum, float maximum) override;
    bool unit_health_gate_5d_00923be4(void* entity) override;
    float unit_health_vtable_110_00923bf6(void* entity) override;
    void unit_health_cache_store_00923c16(void* entity, float value) override;
    bool entity_pose_stale_008a7c24(void* entity) override;
    void entity_pose_refresh_00414db0(void* entity) override;
    bool entity_pose_translation_008a7c3c(void* entity, float out[3]) override;
    void push_vector3_table_0088ba30(const float xyz[3]) override;
    bool measure_is_imperial_0088d9bd() override;
    void push_global_path_value_00b672b0(const char* dotted_path) override;
    float game_clock_seconds_008a93fe() override;
    void* script_entity_create_00898841() override;
    void script_entity_vcall_98_0089892c(void* entity) override;
    void script_entity_call_00927610_00898932(void* entity) override;
    int lua_stack_top_00b65eb0() override;
    void entity_call_named_009290a0(void* entity, const std::string& name,
        int stack_first, int stack_last) override;
    bool push_self_table_slot_008989f6(void* entity) override;
    void entity_arm_think_delay_008982c9(void* entity, float seconds) override;
    void entity_clear_think_name_008985a6(void* entity) override;
    bool entity_flag_5e_00898bd9(void* entity) override;
    void entity_kill_00926d90(void* entity, int cause) override;

    // --- bsp::MissionBlackoutHost, one method per native call site -------------
    void blackout_icon_set_visible(bool visible) override;
    void blackout_icon_set_colour(const bsp::BlackoutFillColour& colour) override;
    void blackout_icon_get_colour(bsp::BlackoutFillColour& colour) override;
    void mission_lua_call_named_00887e50(const std::string& name) override;
    void* local_player_unit_00e188d8() override;
    bool interface_request_pending_005b66d0() override;
    void ingame_interface_store_1c_00644220(int value) override;
    void push_interface_request_004cc460(int request_id, void* payload) override;
    int game_session_kind_1fe4h() override;
    void session_broadcast_blackout_0076d310(float level, float duration) override;
    void force_show_please_wait_screen_00e19698() override;

    // The 004C40F0 step at 004C429A: one 005B9800 pass with the frame delta.
    void run_blackout_update(float step);

    // --- bsp::EntityThinkHost, the walk 00929460 makes over those entities ------
    void run_entity_think_00929150(std::uint32_t entity) override;
    void free_think_node_0092952c(const bsp::EntityThinkNode& node) override;
    bool gc_gate_predicate_0109cefc_vtable0c() override;
    void lua_run_string_006b8ad0(const char* chunk, int mode) override;
    void splice_pending_into_live_00928380(bsp::EntityThinkList& live,
        const bsp::EntityThinkList& pending) override;
    void clear_pending_00928330(bsp::EntityThinkList& pending) override;

    // The `ID` field 00928a00 seeds, turned into a created instance. Null when
    // the argument is not one of this process's entity tables.
    void* entity_from_argument(int index);
    // 008A4C90 PilotSetTarget. Resolves and reports; see the definition for why
    // it does not yet issue.
    int run_pilot_set_target(GameScriptOrderRow& row);
    std::size_t index_of(void* entity) const noexcept;
    std::string name_of(void* entity) const;

    // The script entities CreateScript made. Not a native structure: it is this
    // process's stand-in for the 0x1E4-byte allocation at 00898841, carrying only
    // the fields 00929460 and the five script bindings read.
    GameScriptEntity* script_entity(void* handle) noexcept;
    const GameScriptEntity* script_entity(void* handle) const noexcept;
    bool build_script_self_table(const GameScriptEntity& entity);
    bool push_script_self_table(const GameScriptEntity& entity);
    void call_script_global(const GameScriptEntity& entity, const std::string& name,
        int stack_first, int stack_last);

    // Packet cc_lua_find_entity.
    struct SceneMarker {
        int id{0};
        std::string name;
        float position[3]{0.0f, 0.0f, 0.0f};
    };
    const SceneMarker* marker_for_id(int id) const noexcept;

    GameHostLog& log_;
    GameUnitsHost& units_;
    // 008A4C90's tally. `calls` counts what the scripts asked for; the two
    // `resolved` counters say whether the Lua argument path actually reached a
    // unit and a target, which was the open question the wiring settles.
    unsigned long long pilot_set_target_calls_{0};
    unsigned long long pilot_set_target_unit_resolved_{0};
    unsigned long long pilot_set_target_target_resolved_{0};
    std::vector<SceneMarker> markers_;
    lua_State* state_{nullptr};
    // The mission machine, kept past a dispatch so the per-frame timer pass can
    // call the think globals on it. The native reaches the same machine through
    // *(*(00E188A8)+1A08h)+4h; this process has no such chain and keeps the
    // pointer the trampoline handed it.
    lua_State* machine_state_{nullptr};
    int argument_count_{0};
    GameScriptOrderRow* row_{nullptr};
    GameScriptOrdersSummary summary_{};
    std::vector<GameScriptOrderRow> rows_;
    std::vector<std::size_t> ordered_units_;
    bool logged_path_{false};
    bool logged_predicate_{false};

    // Packet cc_lua_binding_audit.
    std::vector<GameScriptEntity> script_entities_;
    bsp::EntityThinkList think_live_{};
    bsp::EntityThinkList think_pending_{};
    float think_countdown_{0.0f};   // 00F89A04, zero at process start
    // A DeleteScript from inside a think function would erase from the live list
    // while the walk is iterating it. The native's cursor captured its successor
    // first; this reconstruction holds the erase until the walk returns.
    bool in_think_walk_{false};
    std::vector<std::uint32_t> think_erase_after_walk_{};
    float mission_clock_{0.0f};     // 00F876A4, advanced by the caller's step
    bool measure_imperial_{false};  // 00F88988, untouched by this process
    // The stream 00BD2ED0 hands 00BD2F10. This process has no thread-local random
    // state object, so the sequence is this generator's and not the game's; a run
    // is reproducible, which is what a headless comparison needs.
    std::uint32_t random_state_{0x13579BDFu};
    unsigned long long random_draws_{0};
    GameScriptTimerSummary timers_{};

    // Packet cc_mission_blackout. The five fields at `*(00E198C4 + A4h) + C0h`,
    // and the widget colour the +54h getter would answer with. 005BA7B0 leaves
    // +C0h, +C4h and +C8h unwritten; this process starts them at zero, which is
    // the state a first `Blackout` overwrites anyway.
    bsp::MissionBlackoutFade blackout_{};
    bsp::BlackoutFillColour blackout_colour_{};
    GameBlackoutSummary blackout_summary_{};
    // *(float*)(00432650() + E0h), the configured default duration. That field
    // was not read by this packet; every `Blackout` in usn_2_java and
    // commandhelpers.lua passes an explicit numeric argument 3, so the default is
    // never consumed. The host logs it if a call ever reaches it.
    float blackout_configured_duration_{0.0f};
    bool blackout_configured_duration_used_{false};
};

}  // namespace bsp::game
