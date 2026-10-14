// Segments 49 and 60, reconstruction pass (packet cc13-seg49).
//
// Segment 49 (007F8400..0080D9B0): recon detection records and their per-slot lists, the
// sensor-class registry, repair-zone areas, rockets and the turn-path planner. Segment 60
// (009222E0..0092DFE0): the entity / party / think framework. docs/CC13_SEG49.md carries the
// evidence and the status of every routine.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as in
// the image) and reaches the engine through Seg49Host, whose methods name the image routine or
// virtual slot they stand for. Routines the cc9 gameplay hosts already implement by behaviour
// (00806840 and the other 008073C0 steps, 00805CF0) are recorded ledger-only and are not
// reconstructed again here. New C++ interfaces, not drop-in binary replacements; nothing here is
// bound into bsp_game.

#ifndef BSP_CC13_SEG49_HPP
#define BSP_CC13_SEG49_HPP

#include <cstddef>
#include <cstdint>

#include "bsp/ingame_gui_cc13.hpp"

namespace bsp::cc13_seg49 {

using bsp::ingame_gui_cc13::Raw;

// ---------------------------------------------------------------------------
// Image addresses the routines store or read.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kGameGlobal = 0x00E188A8;      // *: game, +18CCh players, +18ECh local index, +1FE4h session
inline constexpr std::uint32_t kRendererGlobal = 0x00F8D394;  // *: renderer singleton
inline constexpr std::uint32_t kReconSlotsBegin = 0x00F874BC; // three recon slot pointers ..00F874C8
inline constexpr std::uint32_t kReconSlotsEnd = 0x00F874C8;   // also the scan-class-id singleton pointer
inline constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818;
inline constexpr std::uint32_t kReconTableScopeVtable = 0x00D08E5C;
inline constexpr std::uint32_t kSessionMessage97Vtable = 0x00D02D1C;
inline constexpr const char* kGlobalsUnknown = "globals.unknown";  // 00CE4180

// A unit's per-party detection record: unit + 1E8h + party * 34h (docs/RECON_SLOT_LISTS.md).
inline constexpr std::size_t kUnitDetectionBase = 0x1E8;
inline constexpr std::size_t kDetectionStride = 0x34;
inline constexpr std::size_t kDetectionLevel = 0x04;      // int
inline constexpr std::size_t kDetectionForcedLevel = 0x08; // int
inline constexpr std::size_t kDetectionForced = 0x10;     // byte
inline constexpr std::size_t kDetectionDelegate = 0x2C;   // object, vtable[0](+30h, old, new)
inline constexpr std::size_t kDetectionDelegateArg = 0x30;

// The engine's 8-byte string {length, data}; storage returns to the sized pool as
// (data, length + 1).
struct NativeStringRaw {
    std::uint32_t length;
    char* data;
};

// The archive's tagged argument: kind 0 = C string key, 1 = int (or int* when read), 2 = float
// (or float* when read).
struct ArchiveArg {
    std::uint32_t kind;
    std::uintptr_t value;
};
inline constexpr const char* kArchiveValueKey = "value";              // 00D06BF4
inline constexpr const char* kArchiveForcedLevelKey = "forcedLevel";  // 00D08E7C
inline constexpr float kDetectionLevel1Threshold = 0.25f;  // 00CE3868
inline constexpr float kDetectionLevel2Threshold = 0.5f;   // 00CE3800
inline constexpr float kDetectionDefaultValue = 0.0f;      // 00D7A218

// ---------------------------------------------------------------------------
// Engine services. Each method names the image routine or virtual slot it stands for.
// ---------------------------------------------------------------------------
struct Seg49Host {
    virtual ~Seg49Host() = default;

    // Globals the routines dereference (named by address).
    virtual Raw game_e188a8() = 0;
    virtual void* renderer_f8d394() = 0;
    // 00F874BC + 4 * index, index 0..2 (BSP_Recon_DestroyAllSlots), and 00F874C8.
    virtual void*& recon_slot_global(std::size_t index) = 0;

    // Object virtuals, ECX = object.
    virtual void scalar_deleting_destructor_0(void* object, int flags) = 0;  // vtable +0
    virtual void renderer_virtual_40(void* renderer, Raw message) = 0;       // [00F8D394] vtable +40h
    virtual bool is_kind_of_5c(void* unit, int class_id) = 0;                // unit vtable +5Ch
    virtual const char* unit_name_14(void* unit) = 0;                        // unit vtable +14h
    virtual void gun_set_trigger_held_1e8(void* gun, std::uint8_t held) = 0; // gun vtable +1E8h
    // unit+1E4h is an embedded object; its vtable[1] answers the sensor class index.
    virtual int sensor_class_index_1e4_4(void* sub_object) = 0;
    // The detection record's change delegate: [rec+2Ch] vtable[0](rec+30h, old, new).
    virtual void detection_delegate_0(void* delegate, std::uint32_t arg, int old_level, int new_level) = 0;
    // [gun+3CCh] vtable +38h (local matrix).
    virtual void gun_attachment_virtual_38(void* attachment, float* local_matrix) = 0;

    // Image routines.
    virtual bool gun_handle_network_message_0072d830(void* gun, Raw message) = 0;
    virtual void gun_setup_from_descriptor_0072e6d0(void* gun) = 0;
    virtual void ref_counted_pointer_array_resize_005471b0(void* array, int count) = 0;
    virtual void matrix_copy_4x4_004134f0(float* destination, const float* source) = 0;
    virtual void normalize_vector_with_floor_0042b260(float* vector) = 0;
    virtual void orthonormalize_basis_rows_0085dc80(float* matrix) = 0;
    virtual void scene_node_invalidate_subtree_pose_0042ed50(void* node) = 0;
    virtual void lua_settop_00a673e0(void* lua_state, int index) = 0;
    virtual void session_message_construct_base_0075b430(Raw message, int opcode) = 0;
    virtual void entity_send_message_to_peer_00779fc0(void* entity, void* peer, Raw message) = 0;

    // Sensor checks (00804350 / 00804620).
    virtual bool powerups_flag_e0c978() = 0;           // byte at 00E0C978
    virtual Raw powerup_manager_f88c30() = 0;          // [00F88C30], +118h tested
    virtual float gameplay_modifiers_product_for_unit_008e6430(int key, void* unit) = 0;
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;  // ECX = entity
    virtual float unit_heading_50(void* unit) = 0;     // unit vtable +50h, ST0
    // 0080446F..00804589 use the CRT's _CIatan2 (00BF701A) with ST1 = y, ST0 = x.
    virtual double ci_atan2_00bf701a(double y, double x) = 0;
    // 00438B10, two stack floats, RET 8, ST0 (bsp::wrapped_angle_subtract_00438b10).
    virtual float wrapped_angle_subtract_00438b10(float left, float right) = 0;

    // The archive (save-table) virtuals 00804F10 / 00805020 call, ECX = archive. Keys and values
    // are 8-byte {kind, value} pairs passed by value.
    virtual bool archive_has_14(void* archive, struct ArchiveArg key) = 0;
    virtual void archive_enter_04(void* archive, struct ArchiveArg key) = 0;
    virtual void archive_leave_08(void* archive) = 0;
    virtual void archive_write_0c(void* archive, struct ArchiveArg key, struct ArchiveArg value) = 0;
    virtual void archive_read_10(void* archive, struct ArchiveArg key, struct ArchiveArg out) = 0;

    // Observer pairs, register ABI ECX = first, EDX = second.
    virtual bool observer_is_pair_registered_00694af0(void* first, void* second) = 0;
    virtual void observer_unregister_pair_006952a0(void* first, void* second) = 0;
    virtual void observer_register_pair_00694a60(void* first, void* second) = 0;

    // Engine strings {length, data}.
    virtual void native_string_assign_cstring_0041e350(void* string, const char* text) = 0;  // ECX = string
    virtual void native_string_construct_0041e870(NativeStringRaw& string, const char* text) = 0;
    virtual void native_string_append_00425e10(void* string, const NativeStringRaw& tail) = 0;
    // 00419CC0 (pool singleton) then its 00BD1510(data, size, 1): return a pooled block.
    virtual void pool_return_00bd1510(void* data, std::uint32_t size) = 0;
    virtual const char* empty_string_f874e0() = 0;            // the empty string at 00F874E0
    virtual const char* unit_name_10(void* unit) = 0;         // unit vtable +10h
    virtual const char* unit_display_name_28(void* unit) = 0; // unit vtable +28h

    // 00806F00: the removal notification's vtable +4h (the unit) and 00806480, the scan-class-id
    // singleton getter (constructs on first use).
    virtual void* notification_unit_4(void* notification) = 0;
    virtual Raw recon_scan_class_ids_00806480() = 0;

    // Sensor registry.
    virtual void*& sensor_class_registry_f874e4() = 0;
    virtual void* operator_new_00bf55be(std::size_t size) = 0;
    virtual void ref_ptr_array_resize_00808c20(void* array, int count) = 0;  // ECX = registry+4h
    // 00415350 manager, its tracked section at +10h (EnterCriticalSection, then +18h counter
    // + 1) when present; unlock is the reverse; register is 00BD0C30 on the manager.
    virtual void singleton_lifetime_lock_00415350() = 0;
    virtual void singleton_lifetime_unlock_00415350() = 0;
    virtual void singleton_lifetime_register_00bd0c30(void* object) = 0;

    // Unit factories.
    virtual void memset_00bf79f0(void* block, int value, std::size_t size) = 0;
    virtual void* recon_plane_unit_instance_construct_0074e0f0(void* block, std::uint32_t argument) = 0;
    virtual void unit_set_vehicle_class_009553d0(void* unit, void* vehicle_class) = 0;
    virtual std::uint16_t object_virtual_28_u16(void* object) = 0;  // vtable +28h, AX kept

    // Turn-path planner.
    virtual Raw global_object(std::uint32_t address) = 0;    // an image global block by address
    virtual double ci_atan_00bf8490(double value) = 0;        // _CIatan, ST0
    virtual float wrapped_angle_add_00438aa0(float left, float right) = 0;
    virtual bool gun_bot_angle_within_tolerance_006dee40(float angle, float target, float tolerance) = 0;
    virtual void memcpy_00bf7680(void* destination, const void* source, std::size_t size) = 0;
    virtual void dlist_clear_0080bd40(void* list) = 0;          // ECX = list
    virtual void array44_reserve_0080bc20(void* array, int capacity) = 0;  // ECX = {data, count, capacity}
    virtual float asin_clamped_0042cf10(float value) = 0;       // camera_asin_clamped_0042cf10

    // EntityPath.
    virtual void scene_node_construct_00925ce0(void* entity) = 0;                       // ECX = entity
    virtual void path_component_construct_007b28c0(void* component, void* entity) = 0;  // ECX = entity+170h
    virtual void path_component_destruct_007b1bd0(void* component) = 0;                 // ECX = entity+170h
    virtual void entity_destruct_base_00925780(void* entity) = 0;                       // ECX = entity

    // Repair zones.
    virtual struct RepairZoneVector& repair_zone_areas_f874f0() = 0;
    virtual std::uint32_t read_global_u32(std::uint32_t address) = 0;  // a dword global by address
    virtual void* property_bag_find_008f2260(void* bag, const char* key) = 0;  // ECX = bag
    virtual const char* property_string_value_008f0df0(void* property) = 0;    // ECX = property
    virtual void* entity_registry_find_by_name_00925a90(void* registry, const char* name) = 0;
    virtual void* archive_object_18(void* archive) = 0;                        // archive vtable +18h
    virtual void* entity_path_interface_007ac9d0(void* entity) = 0;            // ECX = entity
    // 007AF800, __thiscall(ECX = path interface, out, index) RET 8 -> EAX = out.
    virtual float* path_transform_point_to_world_007af800(void* path, float* out, int index) = 0;
    virtual double ci_sqrt_00bf7030(double value) = 0;                         // ST0 in, ST0 out
    virtual void callback_owner_destroy_00695870(void* object) = 0;           // JMP, ECX = area
    virtual int party_relative_to_00803510(int ecx_party, int edx_party) = 0;
    // 00419AB0, ECX = start {x, z}, EDX = end, stack query -> ST0 (RET 4).
    virtual float segment_distance_00419ab0(const float* start, const float* end, const float* point) = 0;

    // Rockets.
    virtual void projectile_shot_ignite_motor_006e1cd0(void* object) = 0;   // ECX = object
    virtual void effect_handle_request_stop_00866b70(void* effect) = 0;     // JMP, ECX = [object+28h]
    virtual const float* object_vector_34(void* object, float* out) = 0;    // vtable +34h(out) -> out
    virtual void hit_record_reset_00470470(void* record) = 0;               // ECX = object+1Ch
    virtual const float* zero_vector_f87574() = 0;                          // the three floats at 00F87574
    virtual void bomb_projectile_construct_006e2670(void* rocket, std::uint32_t argument) = 0;
    virtual void bomb_projectile_destruct_006e2a20(void* rocket) = 0;
    virtual void game_entity_register_in_parent_list_00928560(void* entity) = 0;
    virtual void unit_list_push_back_00484540(void* list, void* entity) = 0; // ECX = list
    virtual void bomb_family_detach_from_world_lists_006e29c0(void* entity) = 0;
    virtual void unit_list_erase_node_004837d0(void* list, void* node) = 0;  // ECX = list
    // Rocket vtable +98h(0, [game+19CCh], matrix 4x4), ECX = rocket.
    virtual void rocket_virtual_98(void* rocket, std::uint32_t zero, void* world, const float* matrix) = 0;

    // List nodes and records.
    virtual void* operator_new_00bf681b(std::size_t size) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void free_00bf6989(void* block) = 0;
    // The {data, size, capacity} int array helpers, ECX = array.
    virtual void int_array_reserve_00804030(void* array, int capacity) = 0;
    virtual void int_array_resize_00804090(void* array, int size) = 0;
    virtual void int_array_assign_008058c0(void* array, const void* source) = 0;
    virtual void observer_link_destroy_base_004b7ef0(void* sub_object) = 0;  // ECX = record+14h
};

// The {count, head, tail} list of {prev, next, payload} nodes the recon slots keep.
struct ReconNode {
    ReconNode* prev;
    ReconNode* next;
    void* payload;
};
struct ReconList {
    std::int32_t count;
    ReconNode* head;
    ReconNode* tail;
};

// ---------------------------------------------------------------------------
// Segment 49: 00801200..00803F30
// ---------------------------------------------------------------------------
void renderer_send_flagged_0x40_00801200(Seg49Host& host, std::uint32_t value);
void multiple_bomb_platform_setup_00803330(Seg49Host& host, Raw gun);
bool mrt_gun_handle_network_message_00803480(Seg49Host& host, Raw gun, Raw message);
void cache_sensor_class_index_008034c0(Seg49Host& host, Raw object);
void recon_destroy_all_slots_008034e0(Seg49Host& host);
void recon_table_scope_destruct_00803860(Seg49Host& host, Raw scope);
void zero_triple_008039b0(Raw triple) noexcept;
void recon_scan_class_ids_reset_singleton_base_008039c0(Seg49Host& host, Raw singleton);
int detection_effective_level(Raw record) noexcept;
void recon_detection_clear_forced_level_00803b50(Seg49Host& host, Raw record);
std::uint8_t party_relative_to_local_player_00803be0(Seg49Host& host, int party);
void* recon_filter_undetected_unit_00803d40(Seg49Host& host, int party, void* unit);
const char* recon_display_name_for_party_00803dc0(Seg49Host& host, int party, void* unit);
void recon_slot_send_detection_levels_to_peer_00803e40(Seg49Host& host, Raw slot, int peer_index);
float unit_max_sensor_range_against_00803f30(Seg49Host& host, Raw unit, Raw target);
void recon_list_splice_node_008040e0(ReconList& self, ReconList& source, ReconNode* node) noexcept;
void recon_list_splice_all_00804150(ReconList& self, ReconList& source) noexcept;
void recon_list_clear_008042b0(Seg49Host& host, ReconList& self);
inline constexpr std::uint32_t kReconDetectionRecordVtable = 0x00D08E74;
void recon_detection_record_destruct_00804300(Seg49Host& host, Raw record);
bool unit_sensors_detect_unit_00804350(Seg49Host& host, Raw unit, Raw target, std::uint32_t mask);
bool unit_sensors_detect_point_00804620(Seg49Host& host, Raw unit, const float* point, std::uint32_t mask);
void recon_list_push_back_00804c30(Seg49Host& host, ReconList& self, void* payload);
void recon_list_destroy_payloads_00804d20(Seg49Host& host, ReconList& self);
void recon_list_copy_append_00804e10(Seg49Host& host, ReconList& self, const ReconList& source);
inline constexpr std::uint32_t kReconEntryVtable = 0x00D08E78;
void* recon_entry_construct_00804e70(Seg49Host& host, Raw entry, void* unit, int level);
void recon_detection_set_read_archive_00804f10(Seg49Host& host, Raw set, void* archive);
void recon_detection_set_write_archive_00805020(Seg49Host& host, Raw set, void* archive);
void recon_retire_carry_over_class_00805430(Seg49Host& host, Raw slot, int class_id);
inline constexpr std::uint32_t kObserverEndpointVtable = 0x00CE75CC;
void* recon_detection_construct_00805a70(Seg49Host& host, Raw record);
void recon_detection_set_level_and_detector_00805c60(Seg49Host& host, Raw record, int level, void* detector);
// 00805F60: the scan-class-id singleton (1Ch: vtable, two {data, size, capacity} int arrays).
inline constexpr std::uint32_t kReconScanClassIdsVtable = 0x00D08EAC;
inline constexpr std::int32_t kReconScanPushOrder[22] = {0x35, 0x07, 0x0A, 0x0B, 0x0C, 0x0D, 0x08, 0x09,
                                                         0x0E, 0x13, 0x10, 0x12, 0x11, 0x15, 0x16, 0x17,
                                                         0x19, 0x1B, 0x45, 0x46, 0x2B, 0x34};
void* recon_construct_scan_class_ids_00805f60(Seg49Host& host, Raw singleton);
void recon_scan_class_ids_destruct_00806400(Seg49Host& host, Raw singleton);
void recon_publish_group_level_from_members_008069a0(Seg49Host& host, Raw slot, const ReconList& groups);
void recon_publish_group_level_stored_00806a60(Seg49Host& host, Raw slot, const ReconList& groups);
// The recon unit label {+0h party, +4h unit, +8h dirty byte, +0Ch level, +10h NativeString}.
inline constexpr const char* kLabelRelationWords[4] = {"friendly", "enemy", "neutral", "unknown"};  // 00D08EB8, 00D08E64, 00CE5604, 00CEB808
inline constexpr const char* kLabelSpace = " ";         // 00CE3A90
inline constexpr const char* kLabelObject = " object";  // 00D08EB0
void recon_unit_label_refresh_00806cd0(Seg49Host& host, Raw label);
const char* recon_unit_label_get_cstring_00806ea0(Seg49Host& host, Raw label);
void* recon_unit_label_get_string_00806ec0(Seg49Host& host, Raw label);
void recon_slot_on_unit_removed_00806f00(Seg49Host& host, Raw slot, void* notification);

// Sensor-class registry (00F874E4) and the {data, count, capacity} arrays of its records:
// 1Ch sensor entries and 20h GUI range rings.
inline constexpr std::uint32_t kSensorClassRegistryVtable = 0x00D08FF4;
inline constexpr float kSensorEntryDefaultRange = 3.40282347e+38f;  // 00D7A248, FLT_MAX
struct RawArray {
    std::uint8_t* data;
    std::int32_t count;
    std::int32_t capacity;
};
void sensor_class_registry_reset_singleton_base_00807b40(Seg49Host& host, Raw singleton);
void sensor_entry_list_construct_00807b60(RawArray& list) noexcept;
void sensor_entry_list_reserve_00807bd0(Seg49Host& host, RawArray& list, int capacity);
void sensor_gui_range_list_reserve_00807c60(Seg49Host& host, RawArray& list, int capacity);
void sensor_entry_list_push_back_00807cf0(Seg49Host& host, RawArray& list, const void* entry);
void sensor_gui_range_list_push_back_00807d40(Seg49Host& host, RawArray& list, const void* ring);
void sensor_entry_list_resize_00807d80(Seg49Host& host, RawArray& list, int count);
void sensor_gui_range_list_resize_00807e60(Seg49Host& host, RawArray& list, int count);
void sensor_entry_list_push_back_00807f10(Seg49Host& host, RawArray& list, const void* entry);
void sensor_gui_range_list_push_back_00807f60(Seg49Host& host, RawArray& list, const void* ring);
void sensor_entry_list_destroy_00808080(Seg49Host& host, RawArray& list);
void sensor_gui_range_list_destroy_008080a0(Seg49Host& host, RawArray& list);
void* sensor_table_row_construct_00808200(Raw row) noexcept;
void sensor_class_registry_destruct_00808e40(Seg49Host& host, Raw registry);
void* sensor_class_registry_get_singleton_00808ea0(Seg49Host& host);
void* recon_plane_class_create_unit_instance_008091d0(Seg49Host& host, void* vehicle_class, std::uint32_t argument);
void create_message_store_carrier_type_00809290(Seg49Host& host, Raw message_part, Raw source);

// Repair-zone areas (4Ch): +0h vtable 00D09018, +4h..+0Ch zero, +10h byte, +14h owner, +18h path
// interface, +1Ch end point 0, +28h end point 1, +34h width, +38h half length, +3Ch centre,
// +48h squared bounding radius. Every live area is in the vector 00F874F0 {data, count +4h,
// capacity +8h}.
inline constexpr std::uint32_t kRepairZoneAreaVtable = 0x00D09018;
inline constexpr const char* kRepairZoneAreaKey = "RepairZoneArea";  // 00D09008
inline constexpr double kRepairZoneLengthEpsilon = 1e-10;            // 00CE3820
inline constexpr double kRepairZoneHalf = 0.5;                        // 00D7A280
struct RepairZoneVector {
    void** data;
    std::uint32_t count;
    std::uint32_t capacity;
};
void repair_zone_area_resolve_path_00809380(Seg49Host& host, Raw area, void* registry, Raw bag);
void* repair_zone_area_construct_00809740(Seg49Host& host, Raw area, void* owner);
void repair_zone_area_destruct_00809820(Seg49Host& host, Raw area);
void* repair_zone_area_create_from_properties_00809880(Seg49Host& host, void* owner, void* registry, Raw bag);
void repair_zone_area_update_from_owner_pose_00809940(Seg49Host& host, Raw area, std::uint32_t unused);
bool repair_zone_area_contains_00809a80(Seg49Host& host, Raw area, void* unit, int party, const float* position);
void unit_repair_zone_slot_load_00809bc0(Seg49Host& host, Raw slot, void* owner, void* registry, Raw bag);
void unit_repair_zone_slot_update_00809c10(Seg49Host& host, Raw slot, std::uint32_t argument);
void* repair_zone_area_find_containing_00809c50(Seg49Host& host, void* unit, int party, const float* position);

// Rockets (MRocket, class id 33h, entity 498h; the class's projectile sub-object is +310h).
inline constexpr std::uint32_t kRocketVtable = 0x00D090E8;
inline constexpr std::uint32_t kRocketVtable10 = 0x00D090CC;
inline constexpr std::uint32_t kRocketVtable24 = 0x00D090C4;
inline constexpr std::uint32_t kRocketVtable170 = 0x00D090C0;
inline constexpr std::uint32_t kRocketVtable1E4 = 0x00D090B8;
inline constexpr std::uint32_t kRocketVtable310 = 0x00D09078;
inline constexpr std::uint32_t kRocketVtable3E4 = 0x00D09060;
inline constexpr int kRocketClassId = 0x33;
void rocket_projectile_on_motor_ignition_00809f20(Seg49Host& host, Raw object);
void rocket_projectile_store_vector_190_00809f50(Seg49Host& host, Raw rocket, void* source);
void* hit_record_holder_reset_00809f90(Seg49Host& host, Raw object);
void rocket_clear_homing_target_00809fb0(Seg49Host& host, Raw block);
void rocket_homing_target_construct_0080aad0(Seg49Host& host, Raw block);
void* rocket_projectile_construct_0080ab40(Seg49Host& host, Raw rocket, std::uint32_t argument);
void rocket_projectile_noop_18c_0080ac60() noexcept;
void rocket_projectile_destruct_0080acf0(Seg49Host& host, Raw rocket);
void rocket_projectile_register_in_world_lists_0080ad40(Seg49Host& host, Raw rocket);
void rocket_projectile_unlink_from_world_lists_0080ad80(Seg49Host& host, Raw rocket);
void* rocket_class_create_projectile_0080ade0(Seg49Host& host, void* rocket_class, std::uint32_t argument, int party,
                                              const float* position, const float* direction);

// Turn-path planner (dead code: 0080CFB0 has no caller). Segments are 44h: +0h kind (0, 1 line,
// 2 arc), line start +8h..+10h, end +14h..+1Ch, arc centre +20h..+28h, radius +2Ch, angles
// +30h / +34h, direction byte +38h, running length before +3Ch and after +40h. A path is
// {segments, count, ?, +0Ch length, +10h out-of-slope byte}. The planner's state is the block
// at 00F87500 (+4h start height, +14h..+1Ch goal, +20h heading-constraint byte, +24h heading).
inline constexpr std::uint32_t kTurnPathState = 0x00F87500;
inline constexpr double kTurnPathMinReach = 300.0;          // 00CE3CA8
inline constexpr double kTurnPathMaxReach = 500.0;          // 00CE3840
inline constexpr float kTurnPathHalfPi = 1.57079637f;       // 00CE3C64
inline constexpr float kTurnPathHeadingTolerance = 0.349065870f;  // 00CE398C
inline constexpr float kTurnPathMaxClimb = 0.174532935f;    // 00E0B514
inline constexpr float kTurnPathMinClimb = -0.174532935f;   // 00E0B518
void turn_path_segment_end_point_0080b1f0(Seg49Host& host, Raw segment, float* out);
float turn_path_length_0080b310(Seg49Host& host, Raw path);
bool turn_path_circle_reaches_goal_0080b530(Seg49Host& host, Raw circle, float* out_angle);
void turn_path_apply_climb_0080b690(Seg49Host& host, Raw path);
void aligned_buffer_reserve_0080b7d0(Seg49Host& host, Raw holder, int capacity);
void turn_path_segment_copy_0080b890(Raw destination, Raw source) noexcept;
void turn_path_visit_end_points_0080bb60(Seg49Host& host, Raw path);
void aligned_buffer_push_0080bcb0(Seg49Host& host, Raw holder, const std::uint32_t* value);
void* turn_path_new_candidate_0080bd90(Seg49Host& host, const float* values);
void turn_path_candidate_list_push_0080bea0(Seg49Host& host, ReconList& list, const std::uint32_t* values);
inline constexpr double kTurnPathTwoPi = 6.2831854820251465;  // 00CE3828 (float 2pi as double)
inline constexpr double kTurnPathPi = 3.1415927410125732;     // 00CE3D28; 00CE3D18 is its negation
// 0080BA00's ten stack arguments: two circles {x, z, radius, unused, angle}.
float* turn_path_tangent_points_0080ba00(float* out, const float* args);
inline constexpr double kTurnPathHalfPiDouble = 1.5707963705062866;  // 00CE3830
// A turn circle as 0080C060 receives it: {x, z, radius, direction byte}.
struct TurnCircle {
    float x;
    float z;
    float radius;
    std::uint8_t direction;
};
void turn_path_circle_to_circle_candidates_0080c060(Seg49Host& host, ReconList& candidates, TurnCircle first,
                                                    TurnCircle second);
void turn_path_circle_to_point_candidate_0080c590(Seg49Host& host, ReconList& candidates, TurnCircle circle,
                                                  float point_x, float point_z);
void segment_array_destroy_0080c710(Seg49Host& host, RawArray& segments);
RawArray* segment_array_assign_0080c750(Seg49Host& host, RawArray& self, const RawArray& source);
void segment_array_destroy_0080c7f0(Seg49Host& host, RawArray& segments);
void* turn_path_assign_0080c830(Seg49Host& host, Raw self, Raw source);
void turn_path_owner_release_0080cf60(Seg49Host& host, void*& path);
void turn_path_add_circle_to_point_candidate_0080c8c0(Seg49Host& host, Raw circle, Raw path_holder);
void turn_path_add_circle_to_circle_candidates_0080ca60(Seg49Host& host, Raw path_holder, TurnCircle first,
                                                         TurnCircle second);
// 0080CFB0's thirteen stack arguments, in order.
struct TurnPathRequest {
    float start[3];             // -> 00F87500..08
    std::uint32_t start_flag;   // low byte -> 00F8750C
    float heading;              // -> 00F87510
    float goal[3];              // -> 00F87514..1C
    std::uint32_t goal_flag;    // low byte -> 00F87520 (heading constraint)
    float goal_heading;         // -> 00F87524
    float radius;               // -> 00E0B510
    float max_climb;            // -> 00E0B514
    float min_climb;            // -> 00E0B518
};
void turn_path_plan_0080cfb0(Seg49Host& host, Raw result, const TurnPathRequest& request);

// EntityPath (class id 48h).
inline constexpr std::uint32_t kEntityPathVtable = 0x00D092B8;
inline constexpr std::uint32_t kEntityPathVtable10 = 0x00D092A0;
inline constexpr std::uint32_t kEntityPathVtable24 = 0x00D09298;
inline constexpr std::uint32_t kEntityPathVtable170 = 0x00D093D0;
inline constexpr int kEntityPathClassId = 0x48;
inline constexpr const char* kEntityPathClassName = "EntityPath";  // 00D09404
void* entity_path_construct_0080d690(Seg49Host& host, Raw entity);
void entity_path_destruct_0080d700(Seg49Host& host, Raw entity);
const char* entity_path_class_name_0080d770() noexcept;

// ===========================================================================
// Segment 60 (009222E0..0092DFE0): the entity / party / think framework. src/cc13_seg49_seg60.cpp.
// ===========================================================================

inline constexpr std::uint32_t kSpawnDescriptorVtable = 0x00D03D94;

struct Seg60Host {
    virtual ~Seg60Host() = default;

    // Object virtuals, ECX = object.
    virtual bool is_kind_of_5c(void* entity, int class_id) = 0;              // vtable +5Ch
    virtual void scalar_deleting_destructor_0(void* object, int flags) = 0;  // vtable +0

    // Image routines.
    virtual void string_set_erase_005fe8b0(void* container, const NativeStringRaw* key) = 0;  // ECX = container
    virtual void pool_return_00bd1510(void* data, std::uint32_t size) = 0;   // 00419CC0 then 00BD1510(data, size, 1)
    virtual void* property_bag_clone_008f41f0(void* bag) = 0;               // ECX = bag
    virtual void* operator_new_00bf55be(std::size_t size) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void hit_record_reset_00470470(void* record) = 0;               // ECX = record
    // An image global by address (pointer-sized dword).
    virtual void*& global_pointer(std::uint32_t address) = 0;
    // Entity virtuals +98h(a, b, c) and +A0h().
    virtual void entity_virtual_98(void* entity, std::uint32_t a, std::uint32_t b, std::uint32_t c) = 0;
    virtual void entity_virtual_a0(void* entity) = 0;
    virtual void* critical_section_create_00bd1860() = 0;
    virtual void unit_list_push_back_00484540(void* list, void* entity) = 0;  // ECX = list
    virtual void unit_list_remove_004845a0(void* list, void* entity) = 0;     // ECX = list
    virtual void orthonormalize_basis_rows_0085dc80(float* matrix) = 0;       // ECX = matrix
    virtual void* entity_virtual_20(void* entity) = 0;                        // vtable +20h
    // 007407C0, ECX = decal list, (x, b, c, a) RET 10h.
    virtual void decal_list_add_007407c0(void* list, std::uint32_t x, std::uint32_t b, std::uint32_t c,
                                         std::uint32_t a) = 0;
    virtual void* operator_new_00bf681b(std::size_t size) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual long interlocked_decrement(volatile long* value) = 0;            // [00CE2220]
    virtual void refcounted_virtual_0(void* object) = 0;   // JMP vtable[0], ECX = object, no stack argument
    // Archive (save-table) virtuals, ECX = archive, {kind, value} pairs by value: kind 0 C string,
    // 1 int, 2 float, 3 byte bool, 4 entity reference, 7 matrix pointer.
    virtual void archive_enter_04(void* archive, ArchiveArg key) = 0;
    virtual void archive_leave_08(void* archive) = 0;
    virtual void archive_write_0c(void* archive, ArchiveArg key, ArchiveArg value) = 0;
    virtual std::uintptr_t entity_name_10(void* entity) = 0;                 // vtable +10h, a C string or null
    virtual bool tagged_value_equals_00bd5680(const ArchiveArg& left, const ArchiveArg& right) = 0;  // ECX = left
    // 00924710, ECX = chain {head, tail, count}, (entity) RET 4: unlink through +40h / +44h.
    virtual void entity_unlink_from_sibling_chain_00924710(void* chain, void* entity) = 0;
    virtual int stricmp_00bf7fbf(const char* left, const char* right) = 0;
    virtual void critical_section_destroy_owned_0041cc80(void* owner) = 0;   // ECX = self+4h
    // 00694280's observer lock: its tracked section ([owner+4h]) entered / left when present.
    virtual void observer_lock_enter_00694280() = 0;
    virtual void observer_lock_leave_00694280() = 0;
    virtual void* stl_list_buy_sentinel_00924a50() = 0;
    virtual void hit_record_destroy_004704b0(void* record) = 0;              // ECX = node+8h
    // Engine strings.
    virtual void native_string_construct_0041e870(NativeStringRaw& string, const char* text) = 0;
    virtual void native_string_resize_0041dd40(NativeStringRaw& string, std::uint32_t length, int keep) = 0;
    // 004261A0, ECX = left, (out, right) -> out: out = left + right.
    virtual NativeStringRaw* native_string_concat_004261a0(const NativeStringRaw& left, NativeStringRaw& out,
                                                           const NativeStringRaw& right) = 0;
    // 009269B0, ECX = list, (list, where, a, b, c, d, c) as 00926BD5 pushes them.
    virtual void stl_list_insert_range_009269b0(void* list, void* list_again, void* where, std::uint32_t a,
                                                std::uint32_t b, std::uint32_t c, std::uint32_t d,
                                                std::uint32_t c_again) = 0;
    virtual void entity_registry_destroy_id_table_slots_009516b0(void* table) = 0;  // ECX = table
    virtual void unit_virtual_154(void* unit, int index, std::uint32_t argument) = 0;
    virtual void unit_virtual_148(void* unit, std::uint32_t mask, std::uint32_t value) = 0;
    virtual void* entity_virtual_114(void* entity) = 0;
    virtual std::uint32_t object_virtual_18(void* object) = 0;  // JMP, ECX = object
    // 007796F0, ECX = message, (role bit, owner, 0, 0) -> message.
    virtual void* session_message_roleowner_construct_007796f0(Raw message, std::uint32_t bit, int owner,
                                                               std::uint32_t c, std::uint32_t d) = 0;
    virtual void session_route_message_0077c2a0(void* entity, void* message, std::uint32_t a, std::uint32_t b) = 0;
    // 00696350, ECX = object, EDX = slot, (argument).
    virtual void observer_notify_00696350(void* object, int edx_slot, std::uint32_t argument) = 0;
    // MSVC std::list<Entity*> helpers.
    virtual Raw global_object(std::uint32_t address) = 0;            // an image global object by address
    virtual void* stl_list_buy_node_00924b10(void* list, void* next, void* prev, void* const* value) = 0;
    virtual void stl_list_incsize_009267f0(void* list, std::uint32_t count) = 0;
    virtual void invalid_iterator_00bf6713() = 0;
    virtual void sentity_init_all_00925f20(std::uint8_t cl_flag) = 0;
    virtual void stl_list_destroy_00924a70(void* list) = 0;          // clear, free the head, head = null
    // 009290A0, ECX = entity, (NativeString* name, argument, 0, -1) RET 10h.
    virtual void mission_entity_register_lua_script_009290a0(void* entity, NativeStringRaw* name,
                                                             std::uint32_t argument, int zero, int minus_one) = 0;
    virtual void scene_node_attach_to_parents_009258f0(void* entity, std::uint32_t a, std::uint32_t b,
                                                       std::uint32_t c) = 0;  // ECX = entity, RET 0Ch
    virtual void* spatial_index_get_singleton_0042e630() = 0;
    virtual void spatial_index_detach_node_0098a500(void* index, void* node) = 0;  // ECX = index
    virtual void unit_list_erase_node_004837d0(void* list, void* node) = 0;       // ECX = list
    virtual void physics_remove_body_00c34f70(void* physics, void* body) = 0;     // ECX = [game+18h]
    virtual Raw game_e188a8() = 0;
    virtual void mission_entity_kill_00926d90(void* entity, int flag) = 0;        // ECX = entity
    virtual void node_unlink_and_release_00b6dfa0(void* node) = 0;               // ECX = node
    virtual void trivial_body_00923060(void* entity) = 0;                         // JMP, ECX = entity
    virtual void point_effect_stop_children_00867b10(void* effect) = 0;           // ECX = effect
    virtual void unit_tick_element_destruct_00875490(void* element) = 0;          // ECX = entity+170h
    virtual void entity_destruct_base_00925780(void* entity) = 0;                 // ECX = entity
    virtual float unit_contact_age_seconds_00779ad0(void* unit) = 0;              // ECX = unit, ST0
    // 00C336C0, ECX = 30h-byte physics transform out, (matrix) RET 4.
    virtual void physics_transform_from_matrix_00c336c0(void* out, const void* matrix) = 0;
    virtual void physics_body_set_transform_00c56cf0(void* body, const void* transform) = 0;  // ECX = body
    virtual const void* physics_body_transform_00c32000(void* body) = 0;                       // ECX = body, 30h
    virtual const float* physics_body_linear_velocity_00c31f40(void* body, float* out) = 0;     // ECX = body
    virtual const float* physics_body_angular_velocity_00c31f20(void* body, float* out) = 0;    // ECX = body
    // 00903860, ECX = world, (point, float* height) RET 8 -> AL; seeds height with -1000.
    virtual bool world_ground_height_at_00903860(void* world, const float* point, float* height) = 0;
    // Effects and randomness (00BD2FC0 / 00BD2F10 with ECX = stream 1).
    virtual std::uint32_t random_next_u32_00bd2fc0(int stream) = 0;
    virtual float random_uniform_float_range_00bd2f10(int stream, float low, float high) = 0;
    virtual Raw game_settings_00424c40() = 0;
    virtual void physics_body_bounds_00c31f90(void* body, float* first, float* second) = 0;  // ECX = body
    virtual long interlocked_increment(volatile long* value) = 0;                          // [00CE221C]
    // 00868420, ECX = out handle, EDX = group, (definition handle by value, matrix, 0, 0) RET 10h ->
    // EAX = out handle; the by-value handle is consumed by the callee.
    virtual void** point_effect_create_from_matrix_00868420(void** out, std::uint32_t group, void* definition,
                                                           const float* matrix, std::uint32_t a,
                                                           std::uint32_t b) = 0;
    // 008687C0, ECX = out handle, EDX = parent node, (definition handle by value, matrix, 1, 0, 0).
    virtual void** point_effect_create_with_parent_matrix_008687c0(void** out, void* parent, void* definition,
                                                                  const float* matrix, std::uint32_t a,
                                                                  std::uint32_t b, std::uint32_t c) = 0;
    virtual void transform_refresh_world_matrix_00b6db70(void* node) = 0;     // ECX = node
};

void watch_registry_remove_009222e0(Seg60Host& host, Raw registry, NativeStringRaw name);
// 00922990 / 009229F0 are bsp::faked_family_00922990 / entity_kind_or_faked_009229f0
// (bsp/attack_target_classify.hpp); 00922AD0 composes them.
bool class_id_is_land_family_00922a90(Raw entity) noexcept;
bool entity_is_surface_ship_target_00922ad0(Seg60Host& host, void* entity);
void* spawn_descriptor_copy_00922de0(Seg60Host& host, Raw self, Raw source);
void spawn_descriptor_destruct_00922e50(Seg60Host& host, Raw self);
void resolve_inherited_58_00922ec0(Raw node) noexcept;
void pointer_vector_grow_00923230(Seg60Host& host, Raw vector);
void* vec16_copy_construct_00923420(Seg60Host& host, Raw self, Raw source);
void vec16_reserve_009234d0(Seg60Host& host, Raw self, std::uint32_t capacity);
void singleton_reset_base_0109ce90_00923620(Seg60Host& host, Raw singleton);
void singleton_reset_base_00f899ec_00923680(Seg60Host& host, Raw singleton);
void pointer_vector_push_back_00923760(Seg60Host& host, Raw vector, const std::uint32_t* value);
void* entity_ancestor_00923810(Raw entity, int levels) noexcept;
void entity_place_and_commit_00923870(Seg60Host& host, void* entity, std::uint32_t a, std::uint32_t b, std::uint32_t c);
void* hit_record_reset_self_00923990(Seg60Host& host, void* record);
void* think_record_copy_construct_00923ed0(Seg60Host& host, Raw self, Raw source);
void vec16_reserve_at_least_00923fd0(Seg60Host& host, Raw self, std::uint32_t capacity);
inline constexpr std::uint32_t kDeferredEventLockVtable = 0x00D190C8;
void* deferred_entity_event_lock_construct_00924200(Seg60Host& host, Raw self);
void entity_adjust_world_list_counters_009245a0(Seg60Host& host, Raw entity, int delta_b8, int delta_b4);
void entity_set_local_frame_00924610(Seg60Host& host, Raw entity, const float* translation, const float* row2,
                                     const float* row1);
void entity_add_decal_00924690(Seg60Host& host, Raw entity, std::uint32_t a, std::uint32_t b, std::uint32_t c);
void sentinel_list_clear_00924770(Seg60Host& host, Raw list);
void* sentinel_list_buy_head_00924ac0(Seg60Host& host);
void vec16_assign_00924be0(Seg60Host& host, Raw self, Raw source);
void refcounted_release_00924cf0(Seg60Host& host, Raw object);
void entity_write_archive_00924d90(Seg60Host& host, Raw entity, void* archive);
void entity_set_parent_00924f90(Seg60Host& host, Raw entity, void* parent);
void* entity_registry_find_by_name_00925180(Seg60Host& host, Raw registry, const NativeStringRaw& name);
void think_record_placement_copy_009253a0(Seg60Host& host, void* destination, Raw source);
inline constexpr std::uint32_t kWeakOwnerMutexVtable = 0x00D190B4;
inline constexpr std::uint32_t kPendingInitLockVtable = 0x00D190C0;
void weak_owner_mutex_destruct_00925400(Seg60Host& host, Raw self);
bool observer_link_bound_009255f0(Seg60Host& host, Raw link);
void pending_init_lock_destruct_00925630(Seg60Host& host, Raw self);
void deferred_entity_event_lock_destruct_00925710(Seg60Host& host, Raw self);
void* stl_list_construct_00925af0(Seg60Host& host, Raw list);
void* sentinel_list_construct_00925b10(Seg60Host& host, Raw list);
void hit_record_list_clear_00925bd0(Seg60Host& host, Raw list);
NativeStringRaw* entity_full_path_name_00926420(Seg60Host& host, NativeStringRaw& out, void* entity);
void stl_list_assign_range_00926b80(Seg60Host& host, Raw list, std::uint32_t a, std::uint32_t b, std::uint32_t c,
                                    std::uint32_t d, std::uint32_t unused);
void entity_registry_destroy_id_tables_00927790(Seg60Host& host, Raw registry);
void unit_apply_role_mask_00927810(Seg60Host& host, void* unit, std::uint32_t mask, std::uint32_t argument);
bool unit_has_role_slot_value_00927850(Raw unit, int value) noexcept;
std::uint32_t controlled_target_00927880(Seg60Host& host, void* entity);
void unit_set_role_009278a0(Seg60Host& host, Raw unit, std::uint32_t value);
bool unit_role_word_is_player_or_open_00927c90(Raw unit, int index) noexcept;
std::uint32_t unit_role_mask_for_00927cc0(Raw unit, int value) noexcept;
inline constexpr std::uint32_t kSessionMessageBaseVtable = 0x00CE4974;
void unit_reassign_roles_00927e50(Seg60Host& host, Raw unit, std::uint32_t mask, int from, int to);
inline constexpr std::uint32_t kPendingInitList = 0x00F899CC;  // MSVC list {+4h head, +8h size}
void sentity_init_now_00927610(Seg60Host& host, void* entity, std::uint8_t flag);
void game_entity_write_archive_00928450(Seg60Host& host, Raw entity, void* archive);
inline constexpr std::uint32_t kSceneLockVtable = 0x00D19284;
void scene_lock_destruct_009285c0(Seg60Host& host, Raw self);
void entity_invoke_think_once_009291d0(Seg60Host& host, Raw entity, std::uint32_t argument);
void entity_attach_and_register_00929c80(Seg60Host& host, Raw entity, std::uint32_t a, std::uint32_t b, std::uint32_t c);
void entity_detach_and_kill_00929d70(Seg60Host& host, Raw entity);
void entity_shutdown_once_00929e30(Seg60Host& host, Raw entity);
inline constexpr std::uint32_t kTickableEntityVtable = 0x00D19500;
inline constexpr std::uint32_t kTickableEntityVtables[5][2] = {
    {0x010, 0x00D194E4}, {0x024, 0x00D194DC}, {0x170, 0x00D194C4}, {0x1A4, 0x00D194B8}, {0x1C4, 0x00D194A8}};
inline constexpr std::uint32_t kCollisionBaseVtable1C4 = 0x00CE89E8;
void tickable_game_entity_destruct_0092a040(Seg60Host& host, Raw entity);
inline constexpr float kRecentContactSeconds = 3.0f;  // 00CE3854
bool unit_contact_is_recent_0092ce70(Seg60Host& host, Raw unit);
void motion_proxy_release_body_0092cfd0(Seg60Host& host, Raw proxy);
void motion_proxy_sync_transform_0092d620(Seg60Host& host, Raw proxy, const void* matrix);
void motion_proxy_set_xz_0092d650(Seg60Host& host, Raw proxy, const float* xz);
float* motion_proxy_linear_velocity_0092d6a0(Seg60Host& host, Raw proxy, float* out);
float* motion_proxy_angular_velocity_0092d700(Seg60Host& host, Raw proxy, float* out);
std::uint8_t* copy_48h_records_0092dfe0(const std::uint8_t* first, const std::uint8_t* last, std::uint8_t* destination) noexcept;
void ground_ray_march_0092d000(Seg60Host& host, Raw entity, const float* step, const float* origin, float* out);
void hull_surface_spray_0092a6d0(Seg60Host& host, Raw entity);
inline constexpr float kSprayRandomLow = -1.0f;  // 00D7A260
void hull_side_spray_0092a8c0(Seg60Host& host, Raw entity);
inline constexpr double kFireWidthScale = 0.2;    // 00CE6648
inline constexpr double kFireHeightScale = 0.6;   // 00D09990
inline constexpr double kFireCountDivisor = 10.0; // 00CE3DC0
inline constexpr float kFireCountMax = 2.0f;      // 00CE3958
void tickable_entity_attach_fires_0092a240(Seg60Host& host, Raw entity, void* parent_node);

}  // namespace bsp::cc13_seg49

#endif  // BSP_CC13_SEG49_HPP
