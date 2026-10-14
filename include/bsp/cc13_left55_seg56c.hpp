// Packet cc13-left55, segment 56 part c: 37 mission-script bindings of the table at 00E0B7B8
// (008BDEC0..008CF7E0) that the earlier seg56 waves left without a reconstruction record.
// local/left55_doc_seg56c.md has the per-function table.
//
// Shared shape (docs/CC13_SEG56.md "Shared shape"): every binding is `int __fastcall(lua_State*
// in ECX)` with a plain RET. The dead prologue (`luaMW_<name> failed:` string built and freed,
// the one-time `luakod` guard), the state wrap (00B66C00), the call frame (00B679B0), the
// argument objects (00B677E0) and their destructors (00B67700) are not repeated; each function
// starts at the first argument read. The return value is 00B66400: a literal where only the
// binding pushes, `LuaFrame::result_count()` where 00B67930 NewTable receives the frame.
//
// Entities are what 00888AA0 resolves (`args.entity_at`); no binding here tests the resolved
// pointer for null unless its comment says so. Field reads and writes go through ImageAccess
// with the image offset; each call the native makes is one Left55Seg56cHost method named by
// the callee address (or vtable slot). Lua values the native handles as LuaObject temporaries
// (iteration keys/values, tables, globals) are opaque `void*` handles owned by the host.
//
// Names are hypotheses from the Lua binding names, not recovered symbols. Status:
// reconstructed, syntax-checked only; not ABI-compatible (new C++ interfaces), not
// fixture-tested, not game-validated.
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "bsp/cc13_left55.hpp"
#include "bsp/engine_seg56_cc13.hpp"
#include "bsp/options_seg56_cc13.hpp"
#include "bsp/scene_deferred_refs.hpp"

namespace bsp::cc13_left55::seg56c {

using bsp::cc13_seg56::LuaArgs;
using bsp::cc13_seg56::LuaResults;
using bsp::cc13_seg56::Vec3;
using bsp::seg56_options_cc13::ImageAccess;
using bsp::seg56_options_cc13::LuaFrame;

inline constexpr std::uint32_t kGameGlobal = 0x00E188A8u;           // the game object
inline constexpr std::uint32_t kInterfaceGlobal = 0x00E198C4u;      // the in-game interface
inline constexpr std::uint32_t kWarningManagerGlobal = 0x00F8A0C4u; // WarningManager
inline constexpr std::uint32_t kListenerSinkGlobal = 0x00F8BBD8u;   // object called at vtable+4 by Loading_Start
inline constexpr std::uint32_t kPleaseWaitScreenGlobal = 0x00E19698u;
inline constexpr std::uint32_t kWeaponNameTable = 0x00E0CFC4u;      // char* table, GetPlayerActWeapon
inline constexpr std::uint32_t kRocketAirGroundSameFlag = 0x00F874FDu; // byte, SetRocketAirGroundTypeDifferent

// The 64-byte matrix 0068A670 fills on the stack (Loading_Start, [ESP+3Ch] at 008CC96C).
struct ListenerMatrix {
    float m[16];
};

// One pure-virtual method per native call site. ABI notes give the native register/stack use.
struct Left55Seg56cHost {
    virtual ~Left55Seg56cHost() = default;

    // --- entity virtuals -------------------------------------------------------------------
    // vtable+5Ch, __thiscall(entity, int class_id), AL: the is-kind-of test. Ids used here:
    // 06h (CheatRemoveWater), 0Fh (plane), 18h (squadron), 1Eh (device), 20h (gun),
    // 2Ah (payload); the meanings are hypotheses from the binding names.
    virtual bool entity_vcall_5c(void* entity, int class_id) = 0;
    // vtable+60h, __thiscall(entity), returns the entity's trigger list (EAX).
    virtual void* entity_vcall_60(void* entity) = 0;
    // vtable+23Ch, __thiscall(entity), result unused (CheatRemoveWater 008C7010).
    virtual void entity_vcall_23c(void* entity) = 0;
    // vtable+F4h, __thiscall(entity, float), result unused (GameOver passes FLD1).
    virtual void entity_vcall_f4(void* entity, float value) = 0;
    // The sub-object at entity+310h: its vtable+38h, __thiscall(entity+310h), AL.
    virtual bool entity_subobject_310_vcall_38(void* entity) = 0;

    // --- Lua objects (opaque handles) ------------------------------------------------------
    // 00B66710 __thiscall(frame, const char*), RET 4: push a C string.
    virtual void lua_push_c_string_00b66710(const char* text) = 0;
    // 004260B0 NativeString_FromInt(u16 id), 00B67910(frame, "thisTable" 00CE7494),
    // 00B678E0 (GetByNativeString), 00B663D0 (PushValue), then the string returned to the pool:
    // pushes thisTable[tostring(id)], the entity tail of the 19 entity-returning bindings
    // (include/bsp/mission_lua_bindings.hpp). One method for that fixed five-call sequence.
    virtual void push_this_table_entity_004260b0_00b663d0(std::uint16_t entity_id) = 0;
    // 00B67930 __thiscall(ECX = frame, LuaObject* out), RET 4: a new table (left on the frame).
    virtual void* lua_new_table_00b67930(LuaFrame& frame) = 0;
    // 00B666C0 __thiscall(table, int index, const LuaObject* value), RET 8: table[index] = value.
    virtual void lua_table_set_object_at_index_00b666c0(void* table, int index, void* value) = 0;
    // 00B66670 __thiscall(table, int index), RET 4: table[index] = {}.
    virtual void lua_table_set_new_table_at_index_00b66670(void* table, int index) = 0;
    // 00B67720 __thiscall(table, LuaObject* out, int index), RET 8: out = table[index].
    virtual void* lua_table_get_by_index_00b67720(void* table, int index) = 0;
    // 0088BA30 __fastcall(ECX = table, EDX = const float src[3]): sets x, y, z.
    virtual void lua_set_vector3_fields_0088ba30(void* table, const Vec3& value) = 0;
    // 00927B40 __thiscall(entity, LuaObject* out), RET 4: the entity's Lua self table.
    virtual void* entity_lua_self_object_00927b40(void* entity) = 0;
    // 00B677E0 __thiscall(frame, LuaObject* out, int index), RET 8: the argument object. Used
    // only where the native passes the argument object itself to an iteration call.
    virtual void* lua_argument_object_00b677e0(int index) = 0;
    // 00B67980 __thiscall(state wrapper, LuaObject* out), RET 4: the globals table.
    virtual void* lua_state_globals_00b67980() = 0;
    // 00B67800 __thiscall(object, LuaObject* out, const char* name), RET 8: out = object[name].
    virtual void* lua_object_get_by_name_00b67800(void* object, const char* name) = 0;
    // 00B65F50 __thiscall(LuaObject*): constructs an unbound object (iteration key/value).
    virtual void* lua_object_construct_00b65f50() = 0;
    // 00B67080 __thiscall(table, LuaObject* key, LuaObject* value), RET 8: first pair.
    virtual void lua_iterate_first_00b67080(void* table, void* key, void* value) = 0;
    // 00B67190 __thiscall(table, LuaObject* key, LuaObject* value), RET 8: next pair.
    virtual void lua_iterate_next_00b67190(void* table, void* key, void* value) = 0;
    // 00B66420 __thiscall(table, const LuaObject* key), RET 4, AL: iteration finished.
    virtual bool lua_object_is_unbound_00b66420(void* table, void* key) = 0;
    // 00B66290 __thiscall(object), __ftol of the number.
    virtual std::int32_t lua_object_get_integer_00b66290(void* object) = 0;
    // 00B662B0 __thiscall(object), borrowed char*; the caller copies it.
    virtual std::string lua_object_get_string_00b662b0(void* object) = 0;
    // 00B662D0 __thiscall(object): the light userdata.
    virtual void* lua_object_to_userdata_00b662d0(void* object) = 0;
    // 0088B840 __fastcall(ECX = object), AL: a table with numeric x/y/z.
    virtual bool lua_object_is_vector3_table_0088b840(void* object) = 0;
    // 00888760 __fastcall(ECX = float out[3], EDX = object), returns out.
    virtual Vec3 lua_object_read_vector3_00888760(void* object) = 0;
    // 00442220 __thiscall(&vector, const LuaObject* element), RET 4: appends a copy of the
    // 14h-byte LuaObject to the binding's local vector (held here as host handles).
    virtual void lua_object_vector_push_back_00442220(std::vector<void*>& vector, void* element) = 0;
    // 00888D20 __fastcall(ECX = argument object): its "Ptr" field (00CFAD08) as userdata.
    virtual void* lua_table_ptr_field_00888d20(int index) = 0;
    // 0088A810 __fastcall(ECX = SceneCommandTarget* out, EDX = argument object).
    virtual SceneCommandTarget read_command_target_0088a810(int index) = 0;

    // --- objectives / HUD ------------------------------------------------------------------
    // 00627960 and 00626C50, both void(void) per their Ghidra prototypes, bodies not read.
    virtual void objectives_refresh_00627960() = 0;
    virtual void objectives_refresh_00626c50() = 0;
    // 005ED5D0 __thiscall(objectives screen, bool expanded), RET 4.
    virtual void objectives_screen_set_expanded_005ed5d0(void* screen, bool expanded) = 0;
    // 00528BE0 __thiscall(counter widget, float value), RET 4.
    virtual void hud_counter_set_value_00528be0(void* counter, float value) = 0;
    // 0060D6E0 __thiscall(screen, bool), RET 4.
    virtual void please_wait_screen_show_with_worker_0060d6e0(void* screen, bool show) = 0;

    // --- units, weapons --------------------------------------------------------------------
    // 007B92E0 __thiscall(plane), AL: any weapon slot answers vtable+20Ch.
    virtual bool weapon_controller_any_slot_answers_007b92e0(void* plane) = 0;
    // 0077C470 __thiscall(unit, uint role_mask, int take), RET 8.
    virtual void unit_send_role_transfer_0077c470(void* unit, std::int32_t role_mask, std::int32_t take) = 0;
    // 00877B90 __thiscall(unit, float health), RET 4.
    virtual void unit_set_health_00877b90(void* unit, float health) = 0;
    // 00926D90 __thiscall(entity, int cause), RET 4.
    virtual void mission_entity_kill_00926d90(void* entity, std::int32_t cause) = 0;
    // 0070D070 __thiscall(group, int index), RET 4: group + 18h + index*34h.
    virtual void* unit_group_member_record_at_0070d070(void* group, std::int32_t index) = 0;
    // 0072CF00 __thiscall(gun, int barrel, float time, int set_full), RET 0Ch.
    virtual void gun_set_barrel_reload_timer_0072cf00(void* gun, std::int32_t barrel, float time,
        std::int32_t set_full) = 0;
    // 00414DB0 __thiscall(entity): refreshes the world pose (+FCh..+104h) when +C8h is clear.
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 00964790 __fastcall(ECX = type id, DL = read_race), returns the class descriptor.
    virtual void* vehicle_class_get_or_create_00964790(std::int32_t type_id, bool read_race) = 0;
    // The descriptor's vtable+18h, __thiscall(descriptor, int), AL.
    virtual bool vehicle_class_vcall_18(void* descriptor, int value) = 0;

    // --- scene -----------------------------------------------------------------------------
    // 00B72300 __thiscall(scene, const NativeString* name, const NativeString* excluded root).
    virtual void* scene_find_node_by_name_excluding_00b72300(void* scene, const std::string& name,
        const std::string& excluded_root) = 0;
    // 00B6DA70 __thiscall(node, float value, char recurse), RET 8.
    virtual void scene_node_set_visibility_factor_00b6da70(void* node, float value, bool recurse) = 0;
    // 00B6D800 __fastcall(ECX = node): LEA EAX,[ECX+54h]; the node's own name (borrowed).
    virtual const void* node_name_header_00b6d800(void* node) = 0;
    // 00B6E750 __thiscall(node, const NativeString* name, byte recursive), RET 8. Two call
    // shapes: with a borrowed name header, and with the binding's own string.
    virtual void* node_find_child_by_header_00b6e750(void* node, const void* name_header, bool recursive) = 0;
    virtual void* node_find_child_by_name_00b6e750(void* node, const std::string& name, bool recursive) = 0;
    // 00880220 __thiscall(object, const NativeString* name), RET 4; body not read.
    virtual void shallow_water_apply_00880220(void* object, const std::string& name) = 0;

    // --- scoring ---------------------------------------------------------------------------
    // 0090ED90 __thiscall(scoring = [game+21A0h], int, const NativeString*, int), RET 0Ch;
    // body not read.
    virtual void scoring_add_party_loss_0090ed90(void* scoring, std::int32_t party,
        const std::string& name, std::int32_t amount) = 0;
    // 0063F1E0 __thiscall(tree, pair* out, const key*), RET 8: std::set insert-unique.
    virtual void scoring_ignored_kill_set_insert_0063f1e0(void* tree, void* entity) = 0;
    // 008CAEE0 __thiscall(set, pair* out, const key*), RET 8: std::set insert-unique.
    virtual void sensitive_unit_set_insert_008caee0(void* set, void* unit) = 0;

    // --- triggers --------------------------------------------------------------------------
    // 0074FBB0 __fastcall(ECX = trigger list, EDX = const NativeString* name), AL.
    virtual bool trigger_list_invoke_by_name_0074fbb0(void* list, const std::string& name) = 0;
    // 00750550 __thiscall(target entity, float range), RET 4: operator new(50h) + 007500E0.
    virtual void* line_of_sight_trigger_create_00750550(void* target, float range) = 0;
    // 007505C0 __thiscall(const SceneCommandTarget*, float range), RET 4.
    virtual void* proximity_trigger_create_007505c0(const SceneCommandTarget& target, float range) = 0;
    // 00750660, no arguments per its Ghidra prototype: operator new(34h).
    virtual void* squad_ordnance_trigger_create_00750660() = 0;
    // 00750790 __thiscall(trigger, const optional<NativeString>*, const NativeString*), RET 8.
    virtual void trigger_set_callback_name_00750790(void* trigger,
        const std::optional<std::string>& optional_name, const std::string& callback) = 0;
    // 0074FAB0 __fastcall(ECX = list, EDX = trigger): trigger->vtable[4](list).
    virtual void trigger_list_add_0074fab0(void* list, void* trigger) = 0;

    // --- game, input, audio ----------------------------------------------------------------
    // 004BEC00: the input manager singleton.
    virtual void* input_manager_singleton_004bec00() = 0;
    // 00A933F0 __thiscall(manager, int context, int level), RET 8.
    virtual void input_set_context_level_00a933f0(void* manager, int context, int level) = 0;
    // 0096BF70 __thiscall(warning manager).
    virtual void warning_manager_clear_waiting_queue_0096bf70(void* manager) = 0;
    // 008053C0 __fastcall(ECX = index): the recon slot, created on first use.
    virtual void* recon_ensure_slot_008053c0(int index) = 0;
    // 004CAC30 __thiscall(game): front-end pump.
    virtual void game_pump_front_end_frame_004cac30(void* game) = 0;
    // 004D2B10 __thiscall(game).
    virtual void game_destroy_world_containers_004d2b10(void* game) = 0;
    // 004CD0F0 __thiscall(game, char hide, byte allow_simulation, char p4), RET 0Ch.
    virtual void game_set_cinematic_mode_004cd0f0(void* game, bool hide, bool allow_simulation, bool p4) = 0;
    // 0068A670 __thiscall(interface, matrix64* out, float velocity[3]* out), RET 8.
    virtual void interface_get_sound_listener_0068a670(void* interface_object, ListenerMatrix& matrix,
        Vec3& velocity) = 0;
    // 004D1100: the effect manager singleton.
    virtual void* effect_manager_singleton_004d1100() = 0;
    // 00867EE0 __thiscall(manager, float delta, void* reference node), RET 8.
    virtual void effect_manager_update_00867ee0(void* manager, float delta, void* reference_node) = 0;
    // [00F8BBD8]'s vtable+4, __thiscall(object, const matrix64*, float3 by value), RET 10h.
    virtual void listener_sink_vcall_04(void* object, const ListenerMatrix& matrix, Vec3 velocity) = 0;

    // --- library ---------------------------------------------------------------------------
    // 004254B0 cdecl(const char*), a trivial body (debug trace); ADD ESP,4 after.
    virtual void debug_trace_004254b0(const char* text) = 0;
    // 00425850 __thiscall(NativeString*, const char*), RET 4, AL: case-insensitive equality.
    virtual bool native_string_equals_insensitive_00425850(const std::string& value, const char* text) = 0;
    // 008C9E10 __fastcall(ECX = first, EDX = last, stack ideal count): VC9 _Sort intro loop
    // over {index, float} pairs (threshold 20h, 0088A720 insertion sort, 0088AC20 partition).
    virtual void sort_scored_entries_008c9e10(ScoredEntry* first, ScoredEntry* last, std::int32_t ideal) = 0;
    // 00BF6713: _invalid_parameter_noinfo of the checked iterators.
    virtual void crt_invalid_parameter_00bf6713() = 0;
};

// The bindings. Parameters a binding does not use are dropped.
int lua_objectives_client_refresh(ImageAccess& image, Left55Seg56cHost& host);           // 008BDEC0
int lua_hack_disable_boat_formations(LuaArgs& args, ImageAccess& image);                  // 008BE010
int lua_hack_force_column_formation_near_islands(LuaArgs& args, ImageAccess& image);      // 008BE180
int lua_get_player_act_weapon(ImageAccess& image, Left55Seg56cHost& host);                // 008BE2F0
int lua_get_plane_equipment(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56cHost& host); // 008BEA50
int lua_get_plane_is_reloading(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56cHost& host); // 008BEBE0
int lua_get_squad_torpedoes(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56cHost& host); // 008BEF80
int lua_set_role_hack(LuaArgs& args, Left55Seg56cHost& host);                             // 008C09A0
int lua_set_mouse_exclusive_repair_category();                                            // 008C0B70
int lua_activate_objectives(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host);   // 008C0E10
int lua_set_rocket_air_ground_type_different(LuaArgs& args, ImageAccess& image);          // 008C1620
int lua_set_hp(LuaArgs& args, Left55Seg56cHost& host);                                    // 008C1790
int lua_wipe_entity_type(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host);      // 008C2AC0
int lua_get_target_info_target(LuaResults& results, ImageAccess& image, Left55Seg56cHost& host); // 008C2C50
int lua_set_visibility_terrain_node(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host); // 008C3170
int lua_get_device(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56cHost& host); // 008C3610
int lua_get_kamikaze_by_dummy(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host); // 008C3880
int lua_get_formation_positions(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56cHost& host); // 008C3AB0
int lua_reload_torpedoes(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host);      // 008C3CE0
int lua_scoring_ignore_players_party_losses(ImageAccess& image);                          // 008C3EA0
int lua_scoring_add_party_loss(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host); // 008C3FE0
int lua_set_counter_timer(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host);     // 008C4200
int lua_set_shallow_water(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host);     // 008C4B50
int lua_remove_trigger(LuaArgs& args, LuaResults& results, Left55Seg56cHost& host);       // 008C5FD0
int lua_cheat_remove_water(LuaArgs& args, Left55Seg56cHost& host);                        // 008C7010
int lua_game_over(ImageAccess& image, Left55Seg56cHost& host);                            // 008C74B0
int lua_class_enumeration(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host);     // 008C8560
int lua_add_line_of_sight_trigger(LuaArgs& args, Left55Seg56cHost& host);                 // 008C9460
int lua_add_proximity_trigger(LuaArgs& args, Left55Seg56cHost& host);                     // 008C9730
int lua_add_squad_ordnance_dropped_trigger(LuaArgs& args, Left55Seg56cHost& host);        // 008C9A10
int lua_hack_sensitive_unit(ImageAccess& image, Left55Seg56cHost& host);                  // 008CBEB0
int lua_scoring_ignore_entity_kill(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host); // 008CC330
int lua_loading_start(ImageAccess& image, Left55Seg56cHost& host);                        // 008CC850
int lua_sort_by_distance(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56cHost& host); // 008CCEB0
int lua_get_payloads(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56cHost& host); // 008CECC0
int lua_get_payload(LuaArgs& args, ImageAccess& image, Left55Seg56cHost& host); // 008CEFC0
int lua_get_guns(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56cHost& host); // 008CF7E0

} // namespace bsp::cc13_left55::seg56c
