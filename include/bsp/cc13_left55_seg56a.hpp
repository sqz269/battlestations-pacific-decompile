// Packet cc13-left55, segment 56 part a: 38 mission script bindings (rows of the binding table at
// 00E0B7B8) that had no reconstruction. local/left55_doc_seg56a.md has the per-function evidence.
//
// Shared shape (docs/CC13_SEG56.md "Shared shape"): every binding is `int __fastcall(lua_State*
// in ECX)` with a plain RET. Its dead prologue (the `luaMW_<name> failed:` string, the one-time
// `luakod` guard), the state wrap (00B66C00), the call frame (00B679B0) and 00B677E0(frame, i) are
// not repeated here; each function starts at the first argument read. The return value is
// 00B66400, the number of values on the frame; every binding below pushes only through its own
// calls, so it is a literal.
//
// Entities are what 00888AA0 resolves (`entity_at`). No binding tests the resolved pointer for
// null unless its comment says so, and none is added. Field reads and writes go through ImageAccess
// with the image offset; each call the native makes is one Left55Seg56aHost method named by the
// callee address (or the vtable slot). Field and method names are hypotheses, not recovered
// identifiers. RET sizes in the host comments follow from the call sites: no call below is
// followed by ADD ESP, so every callee pops its own stack arguments.
//
// Status: reconstructed, syntax-checked only. Not ABI-compatible, not fixture-tested, not
// game-validated.
#pragma once

#include <cstdint>
#include <string>

#include "bsp/engine_seg56_cc13.hpp"
#include "bsp/options_seg56_cc13.hpp"
#include "bsp/scene_deferred_refs.hpp"

namespace bsp::cc13_left55::seg56a {

using cc13_seg56::LuaArgs;
using cc13_seg56::LuaResults;
using cc13_seg56::Vec3;
using seg56_options_cc13::ImageAccess;

inline constexpr std::uint32_t kGameGlobal = 0x00E188A8u;        // the game object
inline constexpr std::uint32_t kGameWorldOffset = 0x19CCu;       // game+19CCh, the world
inline constexpr std::uint32_t kSessionModeOffset = 0x1FE4u;     // game+1FE4h
inline constexpr std::uint32_t kZeroVectorGlobal = 0x00F87574u;  // three floats, 00F87574..7C
inline constexpr std::uint32_t kLocalPlayerSlotGlobal = 0x00F876B8u; // word, MOVZX at 0089E983

// The entity id registry (FindEntityByID 008991C9..008991F5): ids below [00F89A10] index the table
// at [00F89A54] from base [00F89A0C], the rest the table at [00F89AA8] from base [00F89A60]; rows
// are 10h bytes and the entity is the dword at +0Ch.
inline constexpr std::uint32_t kIdSplitGlobal = 0x00F89A10u;
inline constexpr std::uint32_t kIdLowBaseGlobal = 0x00F89A0Cu;
inline constexpr std::uint32_t kIdLowTableGlobal = 0x00F89A54u;
inline constexpr std::uint32_t kIdHighBaseGlobal = 0x00F89A60u;
inline constexpr std::uint32_t kIdHighTableGlobal = 0x00F89AA8u;

// Command objects handed to 0077D600.
inline constexpr std::uint32_t kCommandObjectStop = 0x00E08F88u;        // PUSH at 008A78CF
inline constexpr std::uint32_t kCommandObjectClearOrders = 0x00E08F08u; // PUSH at 008A7A8F

// AddWaterLoad constants, bytes read from the image.
inline constexpr double kWaterLoadHalf = 0.5;          // double 00D7A280 (00 .. E0 3F)
inline constexpr double kWaterLoadDivisor = 10.0;      // double 00CE3DC0 (00 .. 24 40)
inline constexpr double kWaterLoadClampTest = 63.0;    // double 00D099A0 (00 .. 80 4F 40)
inline constexpr float kWaterLoadClampValue = 63.0f;   // float 00D099A8 (00 00 7C 42)
inline constexpr int kWaterLoadRouteFlags = 7;         // PUSH 7 at 00893624

// IsAreaEmpty: a ground height above this counts as land in the area.
inline constexpr float kAreaGroundThreshold = -10.0f;  // float 00CE6848 (00 00 20 C1)

// SquadronSetTravelSpeed stores this float at squadron+388h.
inline constexpr float kTravelSpeedField388 = 0.5f;    // float 00CE3800 (00 00 00 3F)

inline constexpr float kOne = 1.0f;                    // float 00D7A24C (00 00 80 3F)

// ---------------------------------------------------------------------------
// Session messages built inline (0075B430(type) on a stack message, the field stores, then
// 0077C2A0(target, &message, flags, arg); 00CE4974 restored afterwards where noted).
// ---------------------------------------------------------------------------

// SetCommandBuildingLevel 0089B3E9..0089B42E (three arguments) and 0089B435..0089B47C (two):
// type D4h, vtable 00CFADD0, +4h = 1, word +18h = 0, byte +1Ah = 0, +1Ch = integer argument 1,
// +20h = integer argument 2 (0 when absent). Routed with flags 7, 0. Only the two-argument path
// restores the base vtable (0089B47C); the three-argument path jumps past it (0089B433).
struct CommandBuildingLevelMessage {
    static constexpr std::uint8_t kType = 0xD4;
    static constexpr std::uint32_t kVtable = 0x00CFADD0u;
    std::int32_t dword_04{1};
    std::uint16_t word_18{0};
    std::uint8_t byte_1a{0};
    std::int32_t level{0};      // +1Ch
    std::int32_t argument_2{0}; // +20h
};

// PilotStartAttackRun 008A5987..008A59C5: type BCh, vtable 00CF3BB8, +4h = 1 (EBX), word +18h = 0,
// byte +1Ah = 0, byte +1Ch = 0, byte +20h = 1 (BL). Routed with flags 0, 0; base vtable restored
// at 008A59D6.
struct StartAttackRunMessage {
    static constexpr std::uint8_t kType = 0xBC;
    static constexpr std::uint32_t kVtable = 0x00CF3BB8u;
    std::int32_t dword_04{1};
    std::uint16_t word_18{0};
    std::uint8_t byte_1a{0};
    std::uint8_t byte_1c{0};
    std::uint8_t byte_20{1};
};

// ---------------------------------------------------------------------------
// The host: one pure-virtual method per native call site kind
// ---------------------------------------------------------------------------
struct Left55Seg56aHost {
    virtual ~Left55Seg56aHost() = default;

    // --- argument readers not on LuaArgs ---
    // 00888BE0, __fastcall(ECX = argument LuaObject): 00B67800(arg, "Ptr" 00CFAD08) then
    // 00B662D0 on the field; the raw handle behind an entity table, no id lookup.
    virtual void* argument_ptr_field_00888be0(int index) = 0;
    // 00888D20: the same body as 00888BE0 (ledger: "Ptr" via 00B67800 + 00B662D0).
    virtual void* argument_ptr_field_00888d20(int index) = 0;
    // The same two calls inlined: 00B67800(arg, 00CFAD08 "Ptr") then 00B662D0 (0089C078..0089C09B).
    virtual void* argument_ptr_field_00b67800_00b662d0(int index) = 0;
    // 0088A810, __fastcall(ECX = out, EDX = argument): the Lua-to-SceneCommandTarget reader
    // (include/bsp/lua_binding_navigator.hpp).
    virtual SceneCommandTarget read_command_target_0088a810(int index) = 0;

    // --- the entity table push ---
    // 004260B0 formats the zero-extended id, 00B67910 fetches "thisTable" (00CE7494) on the frame,
    // 00B678E0 indexes it by the string, 00B663D0 pushes the value, and the string buffer goes back
    // to the sized pool (00419CC0 + 00BD1510, size+1) when non-null. FindEntityByID 0089920B..
    // 0089929A, GetPlaneSquadron 0089D37E.., GetSquadronLandedBase 0089D59E...
    virtual void push_this_table_entry_004260b0_00b663d0(std::uint16_t id) = 0;

    // --- entity virtuals ---
    // vtable +5Ch, __thiscall(entity, int class_id) -> AL, RET 4: the class/kind test.
    virtual bool entity_vcall_5c(void* entity, std::int32_t class_id) = 0;
    // vtable +114h, __thiscall(unit) -> EAX: the weapon director (DCEnable 0089CBB8, FireStance
    // 008A6758, PlayerCommands 008A6D48).
    virtual void* entity_vcall_114(void* entity) = 0;
    // vtable +1F0h, __thiscall(device), no arguments (ForceFireDepthCharge 0089BB11).
    virtual void entity_vcall_1f0(void* entity) = 0;
    // vtable +34h, __thiscall(entity, Vec3* hidden_out) -> EAX = out, RET 4. On the unit classes
    // include/bsp/approach_target_ref.hpp reads this slot as the velocity; provisional here.
    virtual Vec3 entity_vcall_34(void* entity) = 0;
    // vtable +40h, __thiscall(entity, const Vec3*) (ReplaceDeviceWithEntity 0089C13C..0089C140).
    virtual void entity_vcall_40(void* entity, const Vec3& value) = 0;
    // vtable +38h, __thiscall(entity) -> x87 float (GetRelativeSpeed 008A8410..008A8417).
    virtual float entity_vcall_38(void* entity) = 0;
    // vtable +88h, __thiscall(entity, const float matrix[16]) (EntityTurnToPosition 008A1294..).
    virtual void entity_vcall_88(void* entity, const float* matrix) = 0;
    // (entity+310h)->vtable +0Ch, __thiscall(entity+310h), no arguments (0089C142..0089C151).
    virtual void entity_sub310_vcall_0c(void* sub_object) = 0;

    // --- pose and matrices ---
    // 00414DB0, __thiscall(entity): refresh the world matrix at +CCh.
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 00413920, __thiscall(ECX = left 4x4, out 4x4, right 4x4) -> EAX = out.
    virtual float* matrix_multiply_00413920(const void* left, float* out, const void* right) = 0;
    // 004134F0, __thiscall(ECX = destination 4x4, source 4x4): the x87 4x4 copy.
    virtual void matrix_copy_004134f0(void* destination, const void* source) = 0;
    // 00B63D50, __fastcall(ECX = out 4x4, EDX = source 4x4): orthogonal scaled affine inverse.
    virtual void matrix_affine_inverse_00b63d50(void* out, const void* source) = 0;
    // 0085DC80, __fastcall(ECX = float m[16]), in place.
    virtual void matrix_orthonormalize_basis_rows_0085dc80(float* matrix) = 0;
    // 0042D0D0, __fastcall(ECX = out, EDX = source, matrix, normalize dword) -> EAX = out, RET 8.
    virtual Vec3 vector3_transform_direction_0042d0d0(const Vec3& source, const void* matrix,
        std::int32_t normalize) = 0;
    // 00741E90, __thiscall(entity, const float matrix[16]): the world-pose setter.
    virtual void entity_set_world_pose_matrix_00741e90(void* entity, const void* matrix) = 0;
    // 007B4E90, __fastcall(ECX = const Vec3*) -> x87 heading.
    virtual float vector3_compass_heading_007b4e90(const Vec3& vector) = 0;
    // 00C336C0, __thiscall(ECX = out, const float m[16]), RET 4: copies the 4x4 into a 4x3
    // (include/bsp/ship_hull_shapes.hpp). Dynamics library body, not read here.
    virtual void dyn_copy_matrix43_00c336c0(float out[12], const float* matrix) = 0;
    // 00C56CF0, __thiscall(ECX = body, const float* 4x3). Dynamics library body, not read here.
    virtual void dyn_body_set_transform_00c56cf0(void* body, const float* transform) = 0;
    // 0080E490, __thiscall(unit) -> EAX: the parts object; the binding passes its +2Ch on.
    virtual void* unit_parts_object_0080e490(void* unit) = 0;

    // --- gameplay callees ---
    // 0072C170, __fastcall(ECX = gun entity, EDX = int, target entity, const Vec3*, const Vec3*,
    // float, bool dword), callee-cleaned (no ADD ESP after 008931DE).
    virtual void gun_compute_aim_for_target_0072c170(void* gun, std::int32_t value, void* target,
        const Vec3& point_a, const Vec3& point_b, float number, bool flag) = 0;
    // 0080FF80 (ECX = stack message, count, const float point[3]: type 90h, vtable 00D034A0,
    // +1Ch count, +20h..28h point), then 0077C2A0(entity, &message, flags, 0), then the base
    // vtable 00CE4974 restored (00893613..0089363D).
    virtual void route_hull_impact_message_0080ff80_0077c2a0(void* entity, std::int32_t count,
        const float point[3], int flags, int argument) = 0;
    // 00861C20, __thiscall(grid, column, row, byte value), RET 0Ch: row < 0 writes all 12 rows.
    virtual void flag_grid_set_cell_00861c20(void* grid, std::int32_t column, std::int32_t row,
        bool value) = 0;
    // 0042E630, __cdecl() -> the spatial index singleton.
    virtual void* spatial_index_singleton_0042e630() = 0;
    // 0098B450, __thiscall(index, const float min[3], const float max[3]) -> AL, RET 8. Body not
    // read; a non-zero answer makes IsAreaEmpty push false.
    virtual bool spatial_index_query_box_0098b450(void* index, const float min[3],
        const float max[3]) = 0;
    // 00903860, __thiscall(world, const float point[3], float* out) -> AL, RET 8.
    virtual bool world_ground_height_at_00903860(void* world, const float point[3],
        float* out_height) = 0;
    // 006BCD20, __fastcall(ECX = entity, DL = require_active) -> air-operations block or null.
    virtual void* air_ops_block_006bcd20(void* entity, bool require_active) = 0;
    // 007B8A80: MOV DL,1 / JMP 00964790, so (type id, read_race = true) -> class descriptor.
    virtual void* vehicle_class_007b8a80(std::int32_t type_id) = 0;
    // 00964790, __fastcall(ECX = type id, DL = read_race) -> class descriptor.
    virtual void* vehicle_class_00964790(std::int32_t type_id, bool read_race) = 0;
    // 006CD350, __thiscall(block, slot, class, count, equipment), RET 10h.
    virtual void air_ops_update_slot_006cd350(void* block, std::int32_t slot, void* vehicle_class,
        std::int32_t count, std::int32_t equipment) = 0;
    // 006CCDA0, __thiscall(block, slot, order, arg, owner_player, apply), RET 14h.
    virtual void air_ops_slot_command_006ccda0(void* block, std::int32_t slot, std::int32_t order,
        std::int32_t argument, std::int32_t owner, std::int32_t apply) = 0;
    // 006C4780 (thunk of the slot fill 006C0F00), __thiscall(block, slot, class, count), RET 0Ch.
    virtual void air_ops_fill_slot_006c4780(void* block, std::int32_t slot, void* vehicle_class,
        std::int32_t count) = 0;
    // 006BA4B0, __thiscall(slot record, value), RET 4: body is `[ECX+10h] = value`.
    virtual void air_slot_set_10_006ba4b0(void* slot_record, std::int32_t value) = 0;
    // 006BD800 (ECX = stack message, block, slot: MT_AIRBASE_SETSLOT 81h), then
    // 0077C2A0([block+7Ch], &message, 0, 0), then the base vtable 00CE4974 restored
    // (0089619B..008961C8).
    virtual void route_airbase_setslot_message_006bd800_0077c2a0(void* target, void* block,
        std::int32_t slot, int flags, int argument) = 0;
    // 006C7B10, __thiscall(block, class, count), RET 8: remove stock and trim slots.
    virtual void air_ops_remove_stock_006c7b10(void* block, void* vehicle_class,
        std::int32_t count) = 0;
    // 006CA730, __thiscall(block, squadron), RET 4: append to the list at block+B8h.
    virtual void air_ops_block_push_b8_006ca730(void* block, void* squadron) = 0;
    // 00926D90, __thiscall(entity, int cause), RET 4.
    virtual void mission_entity_kill_00926d90(void* entity, std::int32_t cause) = 0;
    // 00467170, __fastcall(ECX = const char* name) -> command object.
    virtual void* command_registry_find_by_name_00467170(const std::string& name) = 0;
    // 0077D600, __thiscall(entity, command, const SceneCommandTarget*, flags), RET 0Ch.
    virtual void entity_issue_command_0077d600(void* entity, void* command,
        const SceneCommandTarget& target, std::int32_t flags) = 0;
    // 0077C2A0 on an inline message (built as described by the struct), then the base vtable
    // restored where the struct comment says so.
    virtual void route_message_0077c2a0(void* entity, const CommandBuildingLevelMessage& message,
        int flags, int argument) = 0;
    virtual void route_message_0077c2a0(void* entity, const StartAttackRunMessage& message,
        int flags, int argument) = 0;
    // 004CA760, __thiscall(game, const Vec3*, const Vec3*), RET 8. Body not read.
    virtual void game_replace_scene_border_004ca760(void* game, const Vec3& first,
        const Vec3& second) = 0;
    // 0071E150, __thiscall(director, bool), RET 4: the kind-5Ah sub-kind 6 sender.
    virtual void weapon_director_send_subkind6_0071e150(void* director, bool value) = 0;
    // 0071DD30, __thiscall(director, int), RET 4: the kind-5Ah sub-kind 2 sender.
    virtual void weapon_director_send_subkind2_0071dd30(void* director, std::int32_t value) = 0;
    // 007BBBA0, __fastcall(ECX = unit): the ordnance release request.
    virtual void unit_request_ordnance_release_007bbba0(void* unit) = 0;
    // 0070EB00, __thiscall(member, float range) -> AL, RET 4.
    virtual bool observer_target_within_range_0070eb00(void* member, float range) = 0;
    // 007BCC80, __thiscall(unit, float height) -> x87 float, RET 4.
    virtual float weapon_drop_fall_time_007bcc80(void* unit, float height) = 0;
    // 007EE150, __fastcall(ECX = squadron or null) -> x87 float: minimum member +908h.
    virtual float squadron_min_member_908_007ee150(void* squadron) = 0;
    // 00779B60, __fastcall(ECX = unit) -> AL.
    virtual bool unit_is_local_players_role_slot_00779b60(void* unit) = 0;
    // 007788B0, __thiscall(unit) -> AL: [unit+284h] && [[unit+284h]+14h] != unit.
    virtual bool unit_is_formation_follower_007788b0(void* unit) = 0;
    // 007EDEB0 (ECX = stack message, float heading, int 0: MT_ENTITY_RELOCATE 97h heading form),
    // then 0077C2A0(entity, &message, 1, 0), then the base vtable restored (008A11E7..008A120D).
    virtual void route_relocate_heading_message_007edeb0_0077c2a0(void* entity, float heading,
        std::int32_t argument, int flags, int route_argument) = 0;
};

// ---------------------------------------------------------------------------
// The bindings (address, Lua name)
// ---------------------------------------------------------------------------
int lua_hit_target(LuaArgs& args, Left55Seg56aHost& host);                                   // 00892F70
int lua_add_water_load(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host);           // 00893380
int lua_set_weapon_director_target_enable(LuaArgs& args, ImageAccess& image,
    Left55Seg56aHost& host);                                                                 // 008945F0
int lua_is_area_empty(LuaArgs& args, LuaResults& results, ImageAccess& image,
    Left55Seg56aHost& host);                                                                 // 00894C00
int lua_set_air_base_slot(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host);        // 00895ED0
int lua_remove_wrecks(ImageAccess& image, Left55Seg56aHost& host);                           // 008970B0
int lua_remove_air_base_planes(LuaArgs& args, Left55Seg56aHost& host);                       // 008973C0
int lua_find_entity_by_id(LuaArgs& args, LuaResults& results, ImageAccess& image,
    Left55Seg56aHost& host);                                                                 // 008990B0
int lua_order(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host);                    // 0089A660
int lua_set_command_building_level(LuaArgs& args, Left55Seg56aHost& host);                   // 0089B260
int lua_force_fire_depth_charge(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host);  // 0089B9D0
int lua_kill_torpedoes_from_unit(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host); // 0089BB70
int lua_force_equipped_ordnance_type(LuaArgs& args, ImageAccess& image);                     // 0089BD20
int lua_replace_device_with_entity(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host); // 0089BEF0
int lua_replace_scene_border(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host);     // 0089C1B0
int lua_dc_enable(LuaArgs& args, Left55Seg56aHost& host);                                    // 0089CAA0
int lua_get_plane_squadron(LuaArgs& args, LuaResults& results, ImageAccess& image,
    Left55Seg56aHost& host);                                                                 // 0089D250
int lua_get_squadron_landed_base(LuaArgs& args, LuaResults& results, ImageAccess& image,
    Left55Seg56aHost& host);                                                                 // 0089D470
int lua_plane_set_gunfire(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host);        // 0089E050
int lua_squadron_force_release(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host);   // 0089E850
int lua_is_in_position(LuaArgs& args, LuaResults& results, ImageAccess& image,
    Left55Seg56aHost& host);                                                                 // 0089ED20
int lua_plane_set_movie_collision(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host); // 0089F360
int lua_squadron_set_base_unsupported(LuaArgs& args, Left55Seg56aHost& host);                // 0089FAF0
int lua_squadron_set_travel_speed(LuaArgs& args, ImageAccess& image);                        // 008A0630
int lua_entity_turn_to_position(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host);  // 008A0F10
int lua_squadron_set_wanderer_mul(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host); // 008A1B70
int lua_squadron_set_wanderer_enabled(LuaArgs& args, ImageAccess& image,
    Left55Seg56aHost& host);                                                                 // 008A1D60
int lua_plane_bomb_falling_time(LuaArgs& args, LuaResults& results, ImageAccess& image,
    Left55Seg56aHost& host);                                                                 // 008A54C0
int lua_pilot_start_attack_run(LuaArgs& args, Left55Seg56aHost& host);                       // 008A5870
int lua_set_airbase_small_planes(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host); // 008A5A20
int lua_get_squad_fly_time(LuaResults& results, Left55Seg56aHost& host);                     // 008A5D60
int lua_is_player_controlled(LuaResults& results, Left55Seg56aHost& host);                   // 008A5EF0
int lua_set_cheat_turbo(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host);          // 008A62A0
int lua_unit_get_fire_stance(LuaArgs& args, LuaResults& results, ImageAccess& image,
    Left55Seg56aHost& host);                                                                 // 008A6640
int lua_unit_set_player_commands_enabled(LuaArgs& args, Left55Seg56aHost& host);             // 008A6C30
int lua_pilot_stop(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host);               // 008A7780
int lua_pilot_clear_orders(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host);       // 008A7940
int lua_get_relative_speed(LuaArgs& args, LuaResults& results, ImageAccess& image,
    Left55Seg56aHost& host);                                                                 // 008A8280

} // namespace bsp::cc13_left55::seg56a
