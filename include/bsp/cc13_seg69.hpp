// Segment 69, reconstruction pass (packet cc13-seg69).
//
// Segment 69 (009FE130..00A16030): unit-class value readers, the AI forced-target and
// hint-weight maps, the candidate-record collectors, the AI command object constructors,
// destructors, describe and debug-draw bodies, the nearest-enemy-group search, the party
// brain constructor and two developer reports. docs/CC13_SEG69.md carries the evidence and
// the status of every routine, including the ones recorded ledger-only because the cc9
// gameplay hosts already implement them (009FF3A0, 009FFD60, 009FFEB0, 00A0FC50, 00A0FC90,
// 00A152E0).
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as
// in the image) and reaches the engine through Seg69Host, whose methods name the image
// routine, global or virtual slot they stand for. New C++ interfaces, not drop-in binary
// replacements; nothing here is bound into bsp_game.

#ifndef BSP_CC13_SEG69_HPP
#define BSP_CC13_SEG69_HPP

#include <cstddef>
#include <cstdint>

#include "bsp/ingame_gui_cc13.hpp"

namespace bsp::cc13_seg69 {

using bsp::ingame_gui_cc13::Raw;

// ---------------------------------------------------------------------------
// Image addresses and constants.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kForcedTargetWeightMap = 0x00F8A734;  // map; +4h head, +8h size
inline constexpr std::uint32_t kHintWeightGlobalMap = 0x00F8A740;    // map; +4h head, +8h size
inline constexpr std::uint32_t kHintWeightPartyMaps = 0x00F8A750;    // 8 maps of 0Ch ..00F8A7B0
inline constexpr std::uint32_t kHintWeightPartyMapsEnd = 0x00F8A7B0;
inline constexpr std::uint32_t kGroupListsByTeam = 0x00F8AA48;       // list per team, 0Ch apart
inline constexpr std::uint32_t kZeroVector = 0x00F87574;             // float[3], BSS zero
inline constexpr std::uint32_t kEmptyName = 0x00F8A728;              // the empty name 00A02010 answers
inline constexpr std::uint32_t kDebugPartyOverride = 0x00E0E344;     // int, saved/restored by 00A0FA70
inline constexpr std::uint32_t kCommandTypeNames = 0x00E0E308;       // const char*[15]

inline constexpr float kNearestMidpointScale = 0.5f;     // double 00D7A280
inline constexpr float kNearestInitialBest = 1.0e10f;     // float 00CE4970 (501502F9h)
inline constexpr float kTargetValueNoLimit = -1.0f;       // float 00D7A260

// Tuning record (00A371A0) offsets read here.
inline constexpr std::size_t kTuningRandomLow = 0x50;     // float, 00BD2F10 low bound
inline constexpr std::size_t kTuningRandomHigh = 0x54;    // float, 00BD2F10 high bound
inline constexpr std::size_t kTuningCloseAttackDist = 0x1F4;  // float

// AI group fields.
inline constexpr std::size_t kGroupId = 0x5634;           // int
inline constexpr std::size_t kGroupTeam = 0x5638;         // int
inline constexpr std::size_t kGroupMembersHead = 0x5640;  // list head node*
inline constexpr std::size_t kGroupMemberCount = 0x5644;  // int

// Entity pose cache (00414DB0 refreshes it).
inline constexpr std::size_t kEntityParent = 0x3C;
inline constexpr std::size_t kEntityLocalMatrix = 0x74;
inline constexpr std::size_t kEntityPoseFresh = 0xC8;     // byte
inline constexpr std::size_t kEntityWorldMatrix = 0xCC;   // float[16]
inline constexpr std::size_t kEntityWorldPosition = 0xFC; // float[3], row 3 of the world matrix
inline constexpr std::size_t kEntityPoseFlag10C = 0x10C;  // byte, cleared on refresh

// Command object vtables (00D22968..00D22C68).
inline constexpr std::uint32_t kCommandBaseVtable = 0x00D22968;
inline constexpr std::uint32_t kMoveToVtable = 0x00D22ABC;
inline constexpr std::uint32_t kCautiousMoveVtable = 0x00D22AEC;
inline constexpr std::uint32_t kRegroupingMoveVtable = 0x00D22B14;
inline constexpr std::uint32_t kCautiousApproachBaseVtable = 0x00D22A08;
inline constexpr std::uint32_t kCautiousMoveApproachVtable = 0x00D22AE4;
inline constexpr std::uint32_t kAttackVtable = 0x00D22B7C;
inline constexpr std::uint32_t kAttackObserverVtable = 0x00D22B64;
inline constexpr std::uint32_t kObserverBaseVtable = 0x00CE3CD4;
inline constexpr std::uint32_t kMoveToAttackVtable = 0x00D22BBC;
inline constexpr std::uint32_t kMoveToAttackObserverVtable = 0x00D22BA4;
inline constexpr std::uint32_t kCautiousAttackVtable = 0x00D22C04;
inline constexpr std::uint32_t kCautiousAttackObserverVtable = 0x00D22BEC;
inline constexpr std::uint32_t kCautiousAttackApproachVtable = 0x00D22BE4;
inline constexpr std::uint32_t kCloseAttackVtable = 0x00D22C44;
inline constexpr std::uint32_t kCloseAttackObserverVtable = 0x00D22C2C;
inline constexpr int kCautiousRouteCounterInitial = 4;

// The engine's 8-byte string {length, data}; storage returns to the sized pool as
// (data, length + 1, 1) through 00419CC0 then 00BD1510.
struct NativeStr {
    std::uint32_t length;
    char* data;
};

// The 20h-byte candidate record 00A00020 and 00A08020 build.
struct CandidateRecord {
    void* subject;          // +00h
    float point_x;          // +04h
    float point_z;          // +08h
    std::uint8_t flag;      // +0Ch
    std::uint8_t pad[3];
    std::uint32_t value10;  // +10h
    float class_weight;     // +14h
    float random_factor;    // +18h, 00BD2F10 stream 1 between tuning +50h and +54h
    std::uint32_t value1c;  // +1Ch
};
static_assert(sizeof(CandidateRecord) == 0x20, "00A00020 builds a 20h-byte record");

// ---------------------------------------------------------------------------
// Engine services. Each method names the image routine, global or virtual slot.
// ---------------------------------------------------------------------------
struct Seg69Host {
    virtual ~Seg69Host() = default;

    // Image globals: the address of the cell (BSS / .data) named by its image address.
    virtual Raw image_global(std::uint32_t address) = 0;

    // Object virtuals, ECX = object.
    virtual bool unit_is_kind_5c(void* unit, int kind) = 0;        // vtable +5Ch
    virtual bool object_is_type_18(void* object, int type) = 0;    // vtable +18h
    virtual void* object_class_1c(void* object) = 0;               // vtable +1Ch
    virtual int command_type_4(void* command) = 0;                 // command vtable +4h
    virtual bool object_virtual_38(void* object) = 0;              // vtable +38h
    virtual bool filter_rejects_0(void* filter, const float* point) = 0;  // vtable +0h(point)
    virtual void planner_debug_draw_28(void* planner) = 0;         // planner vtable +28h
    // One of the eight planner constructors 00A1EFC0, 00A1F0F0, 00A1F220, 00A1F390, 00A1F500,
    // 00A1F630, 00A1F760, 00A1F890, named by address: __thiscall(ECX = memory)(brain), RET 4.
    virtual void* planner_construct(std::uint32_t constructor, void* memory, void* brain) = 0;

    // Engine routines.
    virtual bool can_drop_ordnance_007b9140(void* unit, std::uint32_t arg) = 0;  // ECX unit
    virtual Raw active_mode_tuning_00a371a0() = 0;
    // 00BD2F10 __thiscall(ECX = stream)(low, high), RET 8, ST0: the process-wide RNG.
    virtual float random_uniform_00bd2f10(std::uint32_t stream, float low, float high) = 0;
    virtual float class_weight_009fdf30(void* class_record) = 0;  // ECX = class record, ST0
    virtual void* vehicle_class_by_index_00964790(int index, bool read_race) = 0;  // ECX, DL
    virtual Raw vehicle_class_registry_00437f50() = 0;
    virtual void registry_cache_resize_00437bd0(void* cache, int count) = 0;  // ECX = reg+4
    virtual int class_upgrade_levels_00951f10(void* vehicle_class) = 0;      // ECX
    // 00A08460 __fastcall(ECX = attacker, EDX = level)(target, flag), RET 8, ST0 discarded.
    virtual float target_weight_00a08460(void* attacker, int level, void* target, int flag) = 0;
    virtual int game_mode_004bca50() = 0;                          // ECX = [00E188A8]
    virtual int player_slot_team_009ffd60(int slot) = 0;           // ECX = slot
    virtual void entity_pose_refresh_00414db0(void* entity) = 0;   // ECX = entity
    // 00413920 __thiscall(ECX = local 4x4)(out, parent 4x4), RET 8: returns out.
    virtual float* matrix_multiply_00413920(const float* local, float* out, const float* parent) = 0;
    virtual float group_recon_ratio_00a2c3d0(void* group) = 0;     // ECX = group, ST0
    virtual void* group_by_id_00a2c950(int id) = 0;                // ECX = id
    virtual void group_set_command_00a2bd00(void* group, void* command) = 0;  // ECX, RET 4
    virtual void observer_register_pair_00694a60(void* endpoint, void* owner) = 0;  // ECX, EDX
    virtual void callback_owner_destroy_00695870(void* owner) = 0;  // ECX
    virtual void* director_route_00778860(void* entity) = 0;        // ECX = entity
    virtual bool route_virtual_c(void* route) = 0;                  // route vtable +0Ch
    virtual void* route_virtual_8(void* route) = 0;                 // route vtable +8h
    // 0059CAE0 __thiscall(ECX = points)(out, 0), RET 8.
    virtual void route_points_0059cae0(void* points, void* out, int zero) = 0;
    virtual int route_points_count_0059cac0(void* points) = 0;      // ECX
    virtual void group_target_00a10c20(void* group) = 0;            // ECX, result unused here
    virtual void defend_danger_point_00a113d0(void* command, float* out) = 0;   // ECX, RET 4
    virtual void retreat_point_00a11530(void* command, float* out) = 0;         // ECX, RET 4
    virtual void* support_site_class_list_0084d560(void* site, void* out_list, void* owner,
        int zero) = 0;                                              // ECX = site, RET 0Ch
    virtual void* list_buy_head_node_006599c0(void* list) = 0;      // ECX = list object
    virtual void collect_party_units_0066e590(void* owner, void* list, int filter) = 0;  // ECX, EDX
    virtual void list_clear_nested_006654c0(void* list) = 0;        // ECX = list object
    virtual void record_vector_push_back_00a078f0(void* vector, const CandidateRecord* record) = 0;
    // 00A07D40 __fastcall(ECX = owner, EDX = out vector)(filter, 0).
    virtual void team_entity_records_00a07d40(void* owner, void* out_vector, void* filter,
        int zero) = 0;
    virtual void group_entity_records_00a07e40(void* group, void* vector, int zero) = 0;  // ECX, EDX, RET 4
    virtual float records_min_speed_00a07c10(void* vector) = 0;     // ECX, ST0
    virtual void pointer_vector_push_back_00947fd0(void* vector, void* const* value) = 0;  // ECX
    // Maps: 00A03B90 find (out iterator {container, node}), 00A04CB0 erase(it), 00A079B0 and
    // 00975C40 operator[] (answer the value cell). ECX = the map object.
    virtual void forced_weight_find_00a03b90(void* map, void* out_iterator, const void* key) = 0;
    virtual void forced_weight_erase_00a04cb0(void* map, void* out_iterator, void* container,
        void* node) = 0;
    virtual float* forced_weight_subscript_00a079b0(void* map, const void* key) = 0;
    virtual float* hint_weight_subscript_00975c40(void* map, const void* key) = 0;
    virtual void tree_erase_00a01340(void* map, void* node) = 0;
    virtual void tree_erase_0096bd60(void* map, void* node) = 0;
    virtual void* class_equipment_009552e0(void* vehicle_class, int index) = 0;  // ECX, RET 4
    virtual void* device_class_resolve_00443490(void* device, bool activate) = 0;  // ECX, DL
    virtual float group_target_value_00a0f970(void* group, void* against, int arg1, float limit,
        char* out_text, int arg4, float scale) = 0;               // ECX, EDX, RET 14h, ST0

    // Strings.
    virtual void native_string_resize_0041dd40(NativeStr* s, std::uint32_t size, bool keep) = 0;
    virtual void native_string_assign_0041e870(NativeStr* s, const char* text) = 0;
    // 004261A0 __thiscall(ECX left)(out, right): out = *left + *right; answers out.
    virtual NativeStr* native_string_concat_004261a0(const NativeStr* left, NativeStr* out,
        const NativeStr* right) = 0;
    // 00419CC0(data, bytes, 1) then 00BD1510 on the pool it returns: the inlined ~NativeStr.
    virtual void pool_release_00419cc0_00bd1510(void* data, std::uint32_t bytes) = 0;

    // Lua objects (00B66790 set string field, 00B67580 set new table, 00B67800 get by name,
    // 00B675D0 set object field, 00B67700 destruct, 0088BA30 set vector fields, 00927B40 the
    // entity's self object).
    virtual void lua_set_cstring_00b66790(void* object, const char* key, const char* value) = 0;
    virtual void lua_set_new_table_00b67580(void* object, const NativeStr* key) = 0;
    virtual void* lua_get_by_name_00b67800(void* object, void* out, const char* key) = 0;
    virtual void lua_set_vector3_0088ba30(void* table, const float* vector) = 0;  // ECX, EDX
    virtual void* entity_lua_self_00927b40(void* entity, void* out) = 0;
    virtual void lua_set_object_00b675d0(void* object, const NativeStr* key, void* value) = 0;
    virtual void lua_object_destruct_00b67700(void* object) = 0;

    // CRT.
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void operator_delete_00bf65ac(void* block) = 0;
    virtual void invalid_parameter_00bf6713() = 0;  // checked-iterator failure
    virtual long atol_00bf8417(const char* text) = 0;
    virtual void* fopen_00bf838e(const char* name, const char* mode) = 0;
    virtual void fprintf_00bf80fd(void* file, const char* format, const char* text) = 0;
    virtual void fclose_00bf8081(void* file) = 0;
};

// ---------------------------------------------------------------------------
// Routines.
// ---------------------------------------------------------------------------
float unit_ship_or_squadron_class_value_009fe130(Seg69Host& host, Raw unit);
float class_ship_or_plane_value_009fe170(Seg69Host& host, Raw object);
std::int32_t unit_droppable_ordnance_count_009fe1b0(Seg69Host& host, Raw unit, std::uint32_t arg);
bool unit_is_available_for_ai_009ffe00(Seg69Host& host, Raw unit);
void* unit_class_data_kind5_or_18_009fff20(Seg69Host& host, Raw unit);
Raw ai_candidate_record_construct_00a00020(Seg69Host& host, Raw record, void* subject,
    std::uint32_t value10, float class_weight, std::uint32_t value1c, float random_factor,
    std::uint8_t flag, const float* point);
const char* ai_name_or_empty_00a02010(Seg69Host& host, Raw object);
void ai_forced_target_weight_clear_00a038c0(Seg69Host& host);
void ai_hint_weight_maps_clear_00a03a60(Seg69Host& host);
NativeStr* ai_debug_describe_class_00a04050(Seg69Host& host, NativeStr* out, Raw vehicle_class,
    int equipment_index);
void ai_forced_target_weight_set_00a07eb0(Seg69Host& host, std::uint32_t key0, std::uint32_t key1,
    void* attacker, std::uint32_t key3, float weight);
void ai_hint_weight_set_global_00a07f60(Seg69Host& host, std::uint32_t hint, float weight);
void ai_hint_weight_set_for_party_00a07f80(Seg69Host& host, std::uint32_t hint, int party,
    float weight);
float ai_group_min_speed_00a07fb0(Seg69Host& host, void* group);
void ai_collect_spawn_point_class_records_00a08020(Seg69Host& host, void* owner, void* out_vector,
    void* filter, int kind_filter);
void ai_collect_candidate_records_00a08420(Seg69Host& host, void* owner, bool spawn_points,
    bool team_entities, void* out_vector, void* filter, int kind_filter);
void ai_debug_target_weight_all_classes_00a0c1f0(Seg69Host& host);
void ai_debug_write_groups_target_value_00a0fa70(Seg69Host& host, const char* arguments);
void ai_command_destruct_observer_body(Seg69Host& host, Raw command);  // 00A0FDE0 and three twins
void ai_command_describe_type_00a0ff60(Seg69Host& host, Raw command, void* lua_object);
Raw ai_command_moveto_construct_00a10240(Raw command, void* group, const float* point);
Raw ai_command_cautious_move_construct_00a102d0(Raw command, void* group, const float* point);
Raw ai_command_regrouping_move_construct_00a10370(Raw command, void* group, const float* point);
Raw ai_command_attack_construct_base_00a10710(Seg69Host& host, Raw command, void* group,
    void* target);
Raw ai_command_movetoattack_construct_00a10890(Seg69Host& host, Raw command, void* group,
    void* target);
Raw ai_command_cautious_attack_construct_00a109b0(Seg69Host& host, Raw command, void* group,
    void* target);
Raw ai_command_close_attack_construct_00a10ae0(Seg69Host& host, Raw command, void* group,
    void* target);
void ai_group_refresh_leader_pose(Seg69Host& host, Raw group);  // the shared inline shape
void ai_command_moveto_debug_draw_00a12540(Seg69Host& host, Raw command);
void ai_cautious_approach_visit_route_points_00a12580(Seg69Host& host, Raw approach, Raw group,
    void* unused);
void ai_command_regrouping_move_debug_draw_00a12a50(Seg69Host& host, Raw command);
void ai_command_movetoattack_debug_draw_00a12cd0(Seg69Host& host, Raw command);
void ai_command_cautious_attack_debug_draw_00a12db0(Seg69Host& host, Raw command);
void ai_command_close_attack_debug_draw_00a12ea0(Seg69Host& host, Raw command);
void ai_command_defend_position_debug_draw_00a12f10(Seg69Host& host, Raw command);
void ai_command_patrol_to_debug_draw_00a12f90(Seg69Host& host, Raw command);
void ai_command_retreat_debug_draw_00a12fd0(Seg69Host& host, Raw command);
void* ai_command_find_nearest_enemy_group_00a13080(Seg69Host& host, Raw command, float* out_score,
    const float* other_point);
void ai_command_auto_close_attack_nearest_00a13280(Seg69Host& host, Raw command);
NativeStr* ai_command_describe_string_00a137d0(Seg69Host& host, Raw command, NativeStr* out);
void ai_command_move_describe_lua_00a13960(Seg69Host& host, Raw command, void* lua_object);
void ai_command_attack_describe_lua_00a13a60(Seg69Host& host, Raw command, void* lua_object);
Raw ai_party_brain_construct_00a15a70(Seg69Host& host, Raw brain, int party_slot);
void ai_party_brain_debug_draw_planners_00a15d00(Seg69Host& host, Raw brain);

}  // namespace bsp::cc13_seg69

#endif
