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
#include <map>
#include <string>
#include <vector>

#include "bsp/entity_think_dispatch.hpp"
#include "bsp/lua_binding_mission.hpp"
#include "bsp/lua_binding_navigator.hpp"
#include "bsp/mission_blackout.hpp"

struct lua_State;

namespace bsp::game {

class GameUnitsHost;
class GameHostLog;

// The plane-squadron member write-back, as a free function so that a caller
// which holds no scripts-orders host can drive it. It fills the registry's
// +3D0h array by name from the units host, which is the only thing the body
// ever needed: `bsp::plane_squadron_registry()` is process-wide and
// `count()` / `unit_row()` are the units host's own public API. Same shape as
// `game_objective_sets()` - a producer and a reader in different hosts with
// neither owning the other.
//
// It exists as a free function because the consumer that needs it earliest,
// GameAiCoordinatorHost::Impl::build_squadrons, runs from `create_units`'s
// tail, while the member's own call sites are both in the mission loop: a
// scene-row squadron's array was therefore still empty when the AI asked, and
// USN04 built 7 AI squadrons over 15 member planes where the mission has 5.
// Adding a scripts-orders pointer to the units host instead would need a
// wiring line in the Codex-owned src/game_hosts.cpp.
//
// `only_unresolved` decides what happens to a record that already has its
// array. The body WIPES with assign(NoUnit) before re-resolving, so it can
// un-fill as well as fill: a caller that runs while a record holds a correct
// answer from another route would destroy it if a lookup missed. The air-ops
// launch path fills +3D0h inline from its batch position, so:
//
//  - false, the mission loop's long-standing behaviour, re-resolves everything
//    every time. Safe there because it runs after the launch path has finished
//    and the names always match: the launch pushes `plan.members[wing].name`
//    into BOTH the unit record and member_names, from the same expression.
//  - true, which is what a call from `create_units` passes, touches only a
//    record whose array is still empty. `create_units` runs in the MIDDLE of
//    the air-ops launch, before the record at all, so a wipe there could only
//    ever hit an older squadron; skipping the filled ones removes the question
//    rather than relying on the name argument holding forever.
//
// Returns the number of slots that resolved.
std::size_t resolve_plane_squadron_members(const GameUnitsHost& units,
                                           GameHostLog* log,
                                           bool only_unresolved = false);

// Packet cc9_difficulty, docs/GAME_DIFFICULTY.md. The switch for the game
// difficulty and the per-unit skill level. True: 0058BF37/0058BF58 store the
// effective difficulty, GetDifficulty (008AE12A) reads it back, and
// SetSkillLevel (0089539A -> 007B8AE0 / 009565A0) sets the units-host slot's
// skill index, which the dive-bomb approach captures at 009F9D22. False: the
// old behaviour, difficulty 0 everywhere and every skill call recorded only.
inline constexpr bool kSkillLevelBound = true;

// game+6ACh, the effective difficulty, one process-wide word as in the image
// (`*(00E188A8)+6ACh`). The mission host's MissionStart store writes it and
// the script host's GetDifficulty reads it. Zero until the first store, the
// value the game object's constructor leaves.
std::int32_t game_effective_difficulty_6ac() noexcept;
void set_game_effective_difficulty_6ac(std::int32_t value) noexcept;

// Packet cc9_entity_dead, docs/ENTITY_DEAD_FLAG.md. True: a unit whose damage
// death the gunnery host has recorded gets `thisTable[id].Dead = true` and
// `KillReason = "harm"` before the next script think, as the destroy-list
// flush does in the image (009273A0 -> vtable[74h] 00926390 -> vtable[7Ch] ->
// 00929B60 -> 00929800). False: the old behaviour, `Dead` stays false.
inline constexpr bool kEntityDeadBound = true;

// Packet cc9_mission_end, docs/MISSION_END.md. True: the dialog registry that
// StartDialog / KillDialog / GetActDialogIDs share (the case-insensitive map at
// [game+21E4h]+1Ch), and the census row for the script's mission end. False:
// the three bindings stay host records and no row is written.
inline constexpr bool kMissionEndBound = true;

class GameHostLog;
class GameUnitsHost;
struct GameSceneEntityRecord;

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
    // Packet cc8_navigator_path: 008a3600's 5Bh message, as the native built it.
    int path_follow_mode{-1};        // msg+24h, Lua argument 2, default 1
    int path_parameter{-1};          // msg+28h, Lua argument 3, default 5
    int path_object_id{-1};          // msg+20h, the path entity's uint16 +174h
};

struct GameScriptOrdersSummary {
    std::size_t calls{0};              // binding calls this host took
    std::size_t attack_moves{0};
    std::size_t move_tos{0};
    std::size_t issued{0};             // commands 0077d600 routed
    std::size_t reached_director{0};   // of those, past 00816e30's arm test
    std::size_t formations_requested{0};
    std::size_t formations_refused{0}; // vtable 16Ch answered false
    // Packet cc8_ship_follow: joins the script path actually made, once
    // 00779D50 was transcribed and 0077FE80's type-76h arm bound.
    std::size_t formations_joined{0};
    // Packet cc8_navigator_path.
    std::size_t path_orders{0};            // 008a3600's 5Bh message
    std::size_t commanded_speed_stores{0}; // 008a3901 / 008a3912
    std::size_t land_avoidance_orders{0};  // 008a3b10, 5Ah selector 9
    std::size_t torpedo_evasion_orders{0}; // 008a3cd0, 5Ah selector 7
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

    // Packet cc8_ship_moveonpath: `GetSelectedUnit` 008AB070 reads the global
    // 00E188D8, which 004C0893 stores in BSP_Game_SetControlledUnit 004C0890.
    // This host is the only object the Lua host holds that reaches the units.
    const GameUnitsHost& units() const noexcept { return units_; }

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
    // Packet cc9_entity_dead. 00929800's two Lua writes for every unit whose
    // death is new since the last call; see the .cpp.
    void publish_unit_deaths_00929800();
    // Packet cc9_mission_end: stamps the frame `Mission.EndMission` first reads
    // true, with the fail/complete status and the objectives at that moment.
    void observe_mission_end();
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

    // Packet cc8_airops_launch_tick. The unit side of 006C5050: the launch start
    // 006C7490 calls it at 006C74C6 and stores what it returns in slot+28h. The
    // native fills a scene property bag and hands it to 004F0AD0, the same
    // creator a `PlaneSquadronGen` row uses, which is how the squadron becomes an
    // ordinary created unit and reaches the script's table. This host owns the
    // units host, so the creation lands here. `wing_count_out` carries the count
    // the deck's plane-count reader then reports for the new squadron.
    // Returns the entity id (unit index plus one), or 0 when nothing was made.
    std::uint32_t create_air_ops_squadron_006c5050(std::uint32_t vehicle_class,
        std::int32_t wing_count, std::int32_t equipment, const std::string& home_base,
        std::string& created_name, std::int32_t& wing_count_out);

    // Packet cc8_lua_generate_object. 0046DB4B runs the descriptor's own
    // instantiate-pass creator on an authored record; this host owns the units
    // host, so the creation lands here as it does for the squadron. The record is
    // the one the scene pass held back, with its world frame already overridden
    // by any position or yaw the script passed. Returns the entity id, or 0.
    std::uint32_t create_unit_from_scene_record_0046db4b(
        const GameSceneEntityRecord& record);

    // 007F4B55's array, filled in. The scene pass queues one entity record per
    // wing and create_units turns them into units afterwards, so the squadron
    // table's +3D0h holds names until something resolves them to unit indices.
    // This does that, by name, and is idempotent: it re-runs only when the unit
    // count has moved, which is what the air-ops launch seam does when it
    // appends. Called from the order path rather than from create_units, because
    // the mission frame owns that call site.
    void resolve_plane_squadron_members();

    // The squadron's live plane count, entity+3CCh, for the tick 006C0510 and for
    // 006BD3F0. A squadron whose unit is gone reports zero.
    std::int32_t air_ops_squadron_plane_count(std::uint32_t squadron) const noexcept;
    std::size_t air_ops_squadrons_created() const noexcept { return squadrons_.size(); }
    // 006CDC70's walk, driven from run_script_timers. See the .cpp for why it is
    // not in the unit motion pass, where the executable has it.
    void run_air_ops_update_006cdc70(float step);
    unsigned long long air_ops_slot_ticks() const noexcept { return air_ops_ticks_; }
    unsigned long long air_ops_slot_refills() const noexcept { return air_ops_refills_; }
    unsigned long long air_ops_slot_releases() const noexcept { return air_ops_released_; }
    std::size_t air_ops_slots_tracking() const noexcept { return air_ops_tracking_; }

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
    // Packet cc8_navigator_path: 008a3600, 008a3b10 and 008a3cd0.
    int argument_count() override;
    float argument_number(int index) override;
    float entity_class_max_speed(void* entity) override;
    void session_route_path_order_message(void* entity,
        const bsp::NavigatorPathOrder& order) override;
    void entity_store_commanded_speed(void* entity, float speed) override;
    void* entity_weapon_director(void* entity) override;
    void session_route_avoidance_message(void* director, int selector,
        bool enabled) override;
    void unit_parts_land_avoidance_disabled(void* entity) override;

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
    int run_pilot_move_to_range(GameScriptOrderRow& row);
    int run_entity_turn_to_entity(GameScriptOrderRow& row);
    int run_unit_set_fire_stance(GameScriptOrderRow& row);
public:
    // The +3Ch allowFire / +3Dh allowMove bytes of a squadron's +348h command
    // block (0084D810), keyed by squadron name. This host builds no such block,
    // so the bytes live here; absent means the constructor's defaults
    // (0084D862-0084D8A2).
    struct SquadronPermissions { bool allow_fire{false}; bool allow_move{false}; };
private:
    std::map<std::string, SquadronPermissions> squadron_permissions_;
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

    // Packet cc8_airops_launch_tick: what this process made for a slot+28h.
    struct AirOpsSquadron {
        std::uint32_t entity_id{0};
        std::size_t unit_index{0};
        std::int32_t wing_count{0};
        std::string name;
    };
    std::vector<AirOpsSquadron> squadrons_;
    // The unit count the squadron table was last resolved against.
    std::size_t squadron_resolved_units_{0};
    bool squadron_limit_logged_{false};
    unsigned long long air_ops_ticks_{0};
    unsigned long long air_ops_refills_{0};
    unsigned long long air_ops_released_{0};
    std::size_t air_ops_tracking_{0};
    std::size_t air_ops_refill_logs_{0};

    GameHostLog& log_;
    GameUnitsHost& units_;
    std::vector<bool> dead_published_;   // per unit index, 00929800 has run
    // Packet cc9_mission_end. [game+21E4h]+1Ch: the active dialogs, keyed by id,
    // compared without case (docs/PANEL_SEQUENCE.md). The value is the start time.
    struct DialogKeyLess {
        bool operator()(const std::string& a, const std::string& b) const noexcept;
    };
    std::map<std::string, float, DialogKeyLess> active_dialogs_;
    struct MissionEndRecord {
        bool seen{false};
        float at_seconds{0.0f};
        std::string status;          // Mission.MissionStatus: failed / completed / unset
        std::string fail_text;       // Mission.MissionEndParams.Text
        std::string fail_entity;     // Mission.MissionEndParams.Ent
        std::vector<std::string> objectives;  // level:num=Active/Success
    } mission_end_;
    unsigned long long dialog_starts_{0};
    unsigned long long dialog_kills_{0};
    unsigned long long dialog_queries_{0};
    // 008A4C90's tally. `calls` counts what the scripts asked for; the two
    // `resolved` counters say whether the Lua argument path actually reached a
    // unit and a target, which was the open question the wiring settles.
    unsigned long long pilot_set_target_calls_{0};
    unsigned long long pilot_set_target_unit_resolved_{0};
    unsigned long long pilot_set_target_target_resolved_{0};
    unsigned long long pilot_set_target_issued_{0};
    unsigned long long pilot_set_target_tasks_{0};
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
    // Packet cc8_navigator_path. 008a3600's path argument, kept for the span of
    // one dispatch so the 5Bh arm can build the receiver's descriptor from the
    // same entity 00720fa0 would have re-resolved out of the message.
    bool logged_path_order_{false};
    bool logged_path_points_missing_{false};
    void* path_entity_for_order_{nullptr};

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
