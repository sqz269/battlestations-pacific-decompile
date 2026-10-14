// Packet cc13-left55, part b: 39 mission script bindings of segment 56 (0088B410..008D3E00)
// that had no reconstruction. Per-function evidence: local/left55_doc_seg56b.md.
//
// Shape (docs/CC13_SEG56.md "Shared shape"): every binding is `int __fastcall(lua_State* in ECX)`
// with a plain RET. The dead prologue (`luaMW_<name> failed:` built and freed, the one-time
// `luakod` guard), the state wrap 00B66C00, the frame 00B679B0 and the LuaObject destructors
// (00B67700, 00B669A0) are not repeated; each function starts at its first argument read.
// Argument i is 00B677E0(frame, i), 0-based. The return value is 00B66400 (the count of values
// on the frame): a literal where only the binding pushes, LuaFrame::result_count() where a
// callee (a table constructor, a push of a fetched value) receives the frame.
//
// Field reads and writes go through ImageAccess with the image offset; each native call is one
// pure-virtual method of Left55Seg56bHost named by the callee address (or the vtable slot).
// Names are hypotheses from the Lua binding name, not recovered identifiers. Status:
// reconstructed, syntax-checked only; not ABI-compatible, not fixture-tested, not game-validated.
#pragma once

#include <cstdint>
#include <string>

#include "bsp/engine_seg56_cc13.hpp"
#include "bsp/options_seg56_cc13.hpp"

namespace bsp::cc13_left55::seg56b {

using cc13_seg56::LuaArgs;
using cc13_seg56::LuaResults;
using cc13_seg56::Vec3;
using seg56_options_cc13::ImageAccess;
using seg56_options_cc13::LuaFrame;

// ImageAccess plus the one access width the shared interface lacks (MOVZX word at 008A95C1).
struct Left55Seg56bImage : ImageAccess {
    virtual std::uint16_t read_u16(void* base, std::uint32_t offset) = 0;
};

// A native LuaObject (14h-byte wrapper of a Lua value) the host hands out. Its destructor
// (00B67700) is part of the stripped boilerplate.
using LuaRef = void*;

inline constexpr std::uint32_t kGameGlobal = 0x00E188A8u;
inline constexpr std::uint32_t kInGameInterfaceGlobal = 0x00E198C4u;
inline constexpr std::uint32_t kSessionModeOffset = 0x1FE4u;    // game+1FE4h, 1 = host
inline constexpr std::uint32_t kLocalSlotOffset = 0x18ECu;      // game+18ECh, the local player slot
inline constexpr std::uint32_t kScoringManagerOffset = 0x21A0u; // game+21A0h
inline constexpr std::uint32_t kPlayerSlotTable = 0x18CCu;      // game+18CCh, 8 slot pointers
inline constexpr std::uint32_t kZeroVector3 = 0x00F87574u;      // three zero floats

// The net-id entity tables 00F89A0C..00F89AA8 (see gameplay_seg38_cc13.hpp).
inline constexpr std::uint32_t kIdSplit = 0x00F89A10u;
inline constexpr std::uint32_t kIdLowBase = 0x00F89A0Cu;
inline constexpr std::uint32_t kIdLowTable = 0x00F89A54u;
inline constexpr std::uint32_t kIdHighBase = 0x00F89A60u;
inline constexpr std::uint32_t kIdHighTable = 0x00F89AA8u;

// Degrees to radians as GetBorderCross and PutRelTo do it: FMUL double 00CE3D28
// (3.1415927410125732, pi rounded to float, stored as double) then FDIV double 00CE3D20 (180.0).
inline constexpr double kPiAsDouble = 3.1415927410125732;
inline constexpr double kDegreesPerHalfTurn = 180.0;
inline constexpr double kHalfPiAsDouble = 1.5707963705062866; // double 00CE3830
inline constexpr double kBorderProbeLength = 50000.0;        // double 00D139A8
inline constexpr double kBorderProbeLift = 0.0;              // double 00D7A258
inline constexpr double kEtaMinClosingSpeed = 0.001;         // double 00CF3F30

// ---------------------------------------------------------------------------
// Session messages (0075B430(type) on a stack message, the field stores, the route, then the
// base vtable 00CE4974 restored). Offsets from the message base; +4h is 1.
// ---------------------------------------------------------------------------

// SetPartyID 008A8CCD..008A8D08: type 7Fh, word +18h = 0, +4h = 1, byte +1Ah = 0, vtable
// 00D032E8, +1Ch = the party; 0077C2A0(entity, &message, 7, 0).
struct PartyIdMessage {
    static constexpr std::uint8_t kType = 0x7F;
    static constexpr std::uint32_t kVtable = 0x00D032E8u;
    std::uint16_t word_18{0};
    std::int32_t dword_04{1};
    std::uint8_t byte_1a{0};
    std::int32_t party{0}; // +1Ch
};

// SetCommandBuildingOwnerPlayer 008A8EE7..008A8F2A: type D3h, word +18h = 0, +4h = 1, byte
// +1Ah = 0, vtable 00CFADBC, +1Ch = the player, byte +20h = 1; 0077C2A0(entity, &message, 7, 0).
struct CommandBuildingOwnerMessage {
    static constexpr std::uint8_t kType = 0xD3;
    static constexpr std::uint32_t kVtable = 0x00CFADBCu;
    std::uint16_t word_18{0};
    std::int32_t dword_04{1};
    std::uint8_t byte_1a{0};
    std::int32_t player{0}; // +1Ch
    std::uint8_t byte_20{1};
};
inline constexpr int kRouteFlags = 7; // PUSH 7 at 008A8CF7 / 008A8F19

// 004BC120's three outputs (cross point, the two ends of the crossed edge) and its flag.
struct BoxEdgeCrossing {
    Vec3 cross;   // GetBorderCross ESP+54h
    Vec3 edge_a;  // ESP+74h, unused by the binding
    Vec3 edge_b;  // ESP+80h, unused by the binding
    bool crossed{false}; // ESP+13h
};

// The command target 0x178 fills (ESP+44h, at most 18h bytes before the next local):
// byte +0 = an object target, byte +1 = a position target, the position at +8h.
struct CommandTargetBuffer {
    std::uint8_t bytes[0x18]{};
};

// ---------------------------------------------------------------------------
// The host: one method per native callee
// ---------------------------------------------------------------------------
struct Left55Seg56bHost {
    virtual ~Left55Seg56bHost() = default;

    // --- entity virtuals (ECX = entity) ---
    // vtable+5Ch, __thiscall(entity, int class_id) -> bool, RET 4: the class test.
    virtual bool entity_vcall_5c(void* entity, int class_id) = 0;
    // vtable+2Ch, __thiscall(entity, int party, dword entity+58h, int* zero), RET 0Ch.
    virtual void entity_vcall_2c(void* entity, int party, std::int32_t field_58, int* zero) = 0;
    // vtable+34h, __thiscall(entity, Vec3* out) -> out, RET 4. The reconstruction takes the
    // vector at the returned address.
    virtual Vec3 entity_vcall_34(void* entity) = 0;
    // vtable+38h, __thiscall(entity) -> float in ST0.
    virtual float entity_vcall_38(void* entity) = 0;
    // vtable+D8h, __thiscall(entity), no stack arguments.
    virtual void entity_vcall_d8(void* entity) = 0;
    // vtable+114h, __thiscall(entity), no stack arguments; EAX is not used.
    virtual void entity_vcall_114(void* entity) = 0;
    // vtable+118h, __thiscall(entity, const Vec3*), RET 4.
    virtual void entity_vcall_118(void* entity, const void* vector) = 0;
    // vtable+11Ch, __thiscall(entity, float), RET 4.
    virtual void entity_vcall_11c(void* entity, float radians) = 0;
    // vtable+120h, __thiscall(entity, const float matrix[16]), RET 4.
    virtual void entity_vcall_120(void* entity, const float* matrix) = 0;
    // vtable+140h, __thiscall(entity) -> pointer.
    virtual void* entity_vcall_140(void* entity) = 0;
    // vtable+178h, __thiscall(entity, CommandTargetBuffer* out), RET 4.
    virtual void entity_vcall_178(void* entity, CommandTargetBuffer& out) = 0;

    // --- pose and vector kernels ---
    // 00414DB0, __thiscall(entity): refresh the world pose (entity+CCh, the position +FCh).
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 00B63D50, __fastcall(ECX = out matrix entity+110h, EDX = source entity+CCh).
    virtual void matrix_build_orthogonal_scaled_affine_inverse_00b63d50(void* out, void* source) = 0;
    // 0042D0D0, __fastcall(ECX = out, EDX = source, matrix*, bool normalize), RET 8.
    virtual Vec3 vector_transform_direction_0042d0d0(const Vec3& source, void* matrix, bool normalize) = 0;
    // 004142E0, __thiscall(ECX = point, Vec3* out, matrix*), RET 8.
    virtual Vec3 vector_transform_affine_point_004142e0(const Vec3& point, void* matrix) = 0;
    // 0042B2F0, __thiscall(ECX = vector) -> float length (0 at or below 1e-10).
    virtual float vector_length_0042b2f0(const Vec3& vector) = 0;

    // --- units, groups and entities ---
    // 007788B0, __thiscall(unit) -> bool (formation follower); PutRelTo discards the result.
    virtual bool unit_is_formation_follower_007788b0(void* unit) = 0;
    // 00778890, __thiscall(unit) -> bool (the unit leads its group).
    virtual bool unit_is_group_leader_00778890(void* unit) = 0;
    // 0070D060, __thiscall(group, int index) -> member entity, RET 4.
    virtual void* unit_group_member_at_0070d060(void* group, int index) = 0;
    // 00926D90, __thiscall(entity, int cause), RET 4.
    virtual void mission_entity_kill_00926d90(void* entity, int cause) = 0;
    // 00803B50, __fastcall(ECX = recon detection record): clear the forced level.
    virtual void recon_clear_forced_level_00803b50(void* record) = 0;
    // 00BF6713, the checked-iterator failure (_invalid_parameter_noinfo); execution continues.
    virtual void invalid_parameter_00bf6713() = 0;
    // 0043EC50, __thiscall(vector) -> element count.
    virtual std::uint32_t vector_size_0043ec50(void* vector) = 0;
    // 0081AD40, __cdecl-style by-value NativeString argument (copied by 0041E870) -> 0..5.
    virtual int repair_category_from_name_0081ad40(const std::string& name) = 0;
    // 00827A40, __thiscall(unit, int category, int level), RET 8.
    virtual void unit_set_repair_level_00827a40(void* unit, int category, int level) = 0;
    // 0080E130, __thiscall(unit, int category) -> int, RET 4.
    virtual int unit_get_repair_level_0080e130(void* unit, int category) = 0;
    // 00939FE0, __thiscall(repair task = unit+A20h, bool), RET 4: byte +46h.
    virtual void repair_task_set_failure_repair_enabled_00939fe0(void* task, bool enabled) = 0;
    // 006BCD20, __fastcall(ECX = entity, DL = require_active) -> air-operations block.
    virtual void* air_ops_block_006bcd20(void* entity, bool require_active) = 0;
    // 006BF650, __thiscall(block, bool, bool, int), RET 0Ch.
    virtual void air_ops_block_set_orders_006bf650(void* block, bool first, bool second, int third) = 0;
    // 00489C40, __thiscall(game, const Vec3* key) -> border zone record, RET 4.
    virtual void* game_find_border_zone_00489c40(void* game, const Vec3& key) = 0;
    // 0071C4F0, __thiscall(game, const Vec3*) -> bool outside the map, RET 4.
    virtual bool game_point_outside_map_bounds_0071c4f0(void* game, const Vec3& point) = 0;
    // 004BC120, __thiscall(game, a*, b*, cross*, edge_a*, edge_b*, bool*), RET 18h.
    virtual void world_first_box_edge_crossing_004bc120(void* game, const Vec3& a, const Vec3& b,
        BoxEdgeCrossing& out) = 0;
    // 00521EA0, __thiscall(CommandTargetBuffer*) -> the target object or null.
    virtual void* command_target_resolve_object_00521ea0(CommandTargetBuffer& target) = 0;
    // 004260B0, __thiscall(NativeString* out, int) : decimal text of the id.
    virtual std::string native_string_from_int_004260b0(int value) = 0;
    // 004D7970, __thiscall(game, char aborted), RET 4: GGame::EndScene.
    virtual void game_end_scene_004d7970(void* game, bool aborted) = 0;
    // 004C9780 (ECX = stack message, first, second; 0075B430(E4h), vtable 00CE7608) then
    // 0076A9F0(game+1EF0h, &message, 0), then the message destructor (004C0FD0 when byte +18h
    // is 3, 004C1F70, base vtable 00CE4974). One method for the build/send/destroy sequence.
    virtual void session_send_e4_004c9780_0076a9f0(void* session, void* first, int second, int out) = 0;
    // [00F8A2FC] vtable+198h -> bool, vtable+184h; both __thiscall without stack arguments.
    virtual bool object_vcall_198(void* object) = 0;
    virtual void object_vcall_184(void* object) = 0;

    // --- scoring ([game+21A0h], all __thiscall) ---
    virtual void scoring_set_slot_ranking_00905350(void* scoring, int slot, int ranking) = 0;
    virtual void scoring_force_refresh_009150a0(void* scoring, int slot) = 0; // RET 4
    // 0090F140 / 0090F1A0 / 0090F200 / 0090F260: (slot, const NativeString* key, int), RET 0Ch.
    virtual void scoring_add_action_score_0090f140(void* scoring, int slot, const std::string& key, int value) = 0;
    virtual void scoring_add_ship_score_0090f1a0(void* scoring, int slot, const std::string& key, int value) = 0;
    virtual void scoring_add_plane_score_0090f200(void* scoring, int slot, const std::string& key, int value) = 0;
    virtual void scoring_add_command_score_0090f260(void* scoring, int slot, const std::string& key, int value) = 0;
    // 0090EDE0, (slot, const NativeString* token, int value, int flag), RET 10h.
    virtual void scoring_grant_award_0090ede0(void* scoring, int slot, const std::string& token, int value, int flag) = 0;
    // 007FC4C0, __thiscall(profile = game+650h, const NativeString*) -> char, RET 4.
    virtual bool profile_is_unlock_satisfied_007fc4c0(void* profile, const std::string& expression) = 0;

    // --- session routes ---
    // 0077C2A0, __thiscall(target, message*, flags, out), RET 0Ch.
    virtual void route_message_0077c2a0(void* target, const PartyIdMessage& message, int flags, int out) = 0;
    virtual void route_message_0077c2a0(void* target, const CommandBuildingOwnerMessage& message, int flags, int out) = 0;

    // --- Lua tables and values ---
    // 00B67930, __thiscall(frame, LuaObject* out), RET 4: a new table pushed on the frame.
    virtual LuaRef frame_new_table_00b67930(LuaFrame& frame) = 0;
    // 00B67910, __thiscall(frame, LuaObject* out, const char*), RET 8: a global by name.
    virtual LuaRef frame_get_by_literal_00b67910(LuaFrame& frame, const char* name) = 0;
    // 00B678E0, __thiscall(table, LuaObject* out, const NativeString*), RET 8.
    virtual LuaRef lua_get_by_native_string_00b678e0(LuaRef table, const std::string& key) = 0;
    // 00B663D0, __thiscall(object): push the value on the stack (the frame grows).
    virtual void lua_push_value_00b663d0(LuaRef object) = 0;
    // 00B665D0, __thiscall(table, int key, int value), RET 8.
    virtual void table_set_integer_at_index_00b665d0(LuaRef table, int key, int value) = 0;
    // 00B67400, __thiscall(table, const NativeString* key, float), RET 8.
    virtual void table_set_number_field_00b67400(LuaRef table, const std::string& key, float value) = 0;
    // 0088BA30, __fastcall(ECX = table, EDX = const Vec3*): fields x, y, z.
    virtual void table_set_vector3_fields_0088ba30(LuaRef table, const Vec3& value) = 0;
    // 00B66670, __thiscall(table, int index), RET 4: table[index] = {}.
    virtual void table_set_new_table_at_index_00b66670(LuaRef table, int index) = 0;
    // 00B67720, __thiscall(table, LuaObject* out, int index), RET 8.
    virtual LuaRef table_get_by_index_00b67720(LuaRef table, int index) = 0;
    // 00B67460, __thiscall(table, const NativeString* key, int), RET 8.
    virtual void table_set_string_integer_00b67460(LuaRef table, const std::string& key, int value) = 0;
    // 00B673A0, __thiscall(table, const NativeString* key, bool), RET 8.
    virtual void table_set_boolean_00b673a0(LuaRef table, const std::string& key, bool value) = 0;
    // 00B66790, __thiscall(table, const char* key, const char* value), RET 8.
    virtual void table_set_cstring_00b66790(LuaRef table, const char* key, const char* value) = 0;

    // --- movie camera ---
    // 005CD160, __fastcall(ECX = [[00E198C4]+98h]) -> movcam+30h.
    virtual void* movcam_ensure_target_005cd160(void* movcam) = 0;
    // 006B7E30 (LuaStateHolder on the binding's lua_State), 008875F0 (override list),
    // 00887370(first_slot, -1, 4) over stack slots first_slot..top (1-based), operator new(0Ch)
    // zeroed, 006EDF00 per 14h-byte variant record, then 00887670 and 006B7E50. Returns the new
    // vector, or null when operator new failed.
    virtual void* movcam_collect_variants_00887370(LuaArgs& args, int first_slot) = 0;
    // 007A74F0, __thiscall(target, LuaObject value (by value, copied by 00B66FA0), float, bool,
    // float, const NativeString*, variant vector* or null). `value_index` names the argument
    // whose LuaObject is copied.
    virtual void movcam_set_path_walk_point_property_007a74f0(void* target, LuaArgs& args, int value_index,
        float point, bool flag, float number, const std::string& name, void* variants) = 0;
    // 0076D5B0, __thiscall(session = game+1EF0h, const LuaObject*, float, bool, float), RET 10h.
    virtual void session_broadcast_movcam_path_walk_point_property_0076d5b0(void* session, LuaArgs& args,
        int value_index, float point, bool flag, float number) = 0;
};

// ---------------------------------------------------------------------------
// The bindings (address, Lua name)
// ---------------------------------------------------------------------------
int lua_get_eta(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56bHost& host);          // 008A8480
int lua_set_party_id(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56bHost& host);     // 008A8B40
int lua_set_command_building_owner_player(LuaArgs& args, Left55Seg56bHost& host);                        // 008A8D90
int lua_set_ai_control(LuaArgs& args, ImageAccess& image);                                               // 008A91B0
int lua_get_primary_target(LuaArgs& args, LuaFrame& frame, Left55Seg56bImage& image, Left55Seg56bHost& host); // 008A9460
int lua_put_rel_to(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host);                           // 008AA560
int lua_clear_forced_recon_level(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host);             // 008AAB90
int lua_get_role_available(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56bHost& host);  // 008ABAC0
int lua_get_player_index_by_slot(LuaArgs& args, LuaResults& results, ImageAccess& image);                // 008ABC70
int lua_get_valid_player_in_slot(LuaArgs& args, LuaResults& results, ImageAccess& image);                // 008ABDF0
int lua_get_role_owner(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56bHost& host);      // 008ABF90
int lua_save();                                                                                          // 008AC9D0
int lua_add_fire(LuaArgs& args);                                                                         // 008ACD10
int lua_get_fire(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56bHost& host);        // 008ACF00
int lua_set_repair_level(LuaArgs& args, Left55Seg56bHost& host);                                         // 008AD150
int lua_failure_repair_enable(LuaArgs& args, Left55Seg56bHost& host);                                    // 008AD540
int lua_get_repair_level(LuaArgs& args, LuaResults& results, Left55Seg56bHost& host);                    // 008ADC40
int lua_kill_stationaries(ImageAccess& image, Left55Seg56bHost& host);                                   // 008AE180
int lua_kill_bullets(ImageAccess& image, Left55Seg56bHost& host);                                        // 008AE2F0
int lua_get_airbase_orders(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56bHost& host); // 008AE760
int lua_set_airbase_orders(LuaArgs& args, Left55Seg56bHost& host);                                       // 008AE900
int lua_set_border_zone_party(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host);                // 008AEB20
int lua_get_border_zone(LuaFrame& frame, ImageAccess& image, Left55Seg56bHost& host);                    // 008AF260
int lua_get_border_cross(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56bHost& host);    // 008AF700
int lua_end_score(ImageAccess& image, Left55Seg56bHost& host);                                           // 008AFFC0
int lua_training_ground_end_scene(ImageAccess& image, Left55Seg56bHost& host);                           // 008B0470
int lua_movcam_path_walk_path_point_property_potlekokkal(LuaArgs& args, ImageAccess& image,
    Left55Seg56bHost& host);                                                                             // 008B4DA0
int lua_scoring_get_real_play_time(LuaResults& results, ImageAccess& image);                             // 008B8970
int lua_scoring_set_ranking(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host);                  // 008B8CA0
int lua_scoring_force_refresh_scoring_table(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host);  // 008B8E60
int lua_scoring_add_action_score(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host);             // 008B9E90
int lua_scoring_add_ship_score(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host);               // 008BA550
int lua_scoring_add_plane_score(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host);              // 008BAC10
int lua_scoring_add_command_score(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host);            // 008BB2D0
int lua_scoring_is_unlocked(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56bHost& host); // 008BBC40
int lua_set_achievements(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host);                     // 008BC740
int lua_multi_get_players(LuaFrame& frame, ImageAccess& image, Left55Seg56bHost& host);                  // 008BCD80
int lua_get_player_details(LuaFrame& frame, ImageAccess& image, Left55Seg56bHost& host);                 // 008BCF10
int lua_is_unit_untouchable(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56bHost& host); // 008AC420

} // namespace bsp::cc13_left55::seg56b
