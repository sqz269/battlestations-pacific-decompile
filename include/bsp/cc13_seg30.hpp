// Segments 30 and 50, reconstruction pass (packet cc13-seg30).
//
// Segment 30 (006BEB70..006D4340): the air-ops block (runway/hangar slots, plane stock, the launch
// queue), its landing sites (airfield site, mothership site, elevator site) and the MAirfield unit.
// Segment 50 (0080DA00..0081AA10): unit-instance helpers, the ship session messages and the
// helmsman estimator. docs/CC13_SEG30.md carries the evidence and the status of every routine.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as in the
// image) and reaches the engine through Seg30Host, whose methods name the image routine or
// virtual slot they stand for. Routines the cc9 gameplay hosts already implement by behaviour
// (src/air_operations.cpp, game_hosts_units.cpp, game_hosts_gunnery.cpp and the cc13 launch_ver /
// script_natives_fix5 files) are recorded ledger-only and are not reconstructed again here.
// New C++ interfaces, not drop-in binary replacements; nothing here is bound into bsp_game.

#ifndef BSP_CC13_SEG30_HPP
#define BSP_CC13_SEG30_HPP

#include <cstddef>
#include <cstdint>
#include "bsp/native_command_target_resolve.hpp"
namespace bsp::airfield_marker_seg31_32_argon { struct Seg31_32Host; }

namespace bsp::cc13_seg30 {

template <class T>
inline T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
inline void* ptr(void* base, std::size_t off) noexcept { return at<void*>(base, off); }
inline void* add(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) + off;
}

// ---------------------------------------------------------------------------
// Image constants.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kGameGlobal = 0x00E188A8;            // *: game, +1FE4h session mode
inline constexpr std::size_t kGameSessionMode = 0x1FE4;
inline constexpr std::size_t kSlotStride = 0x58;                    // air-ops slot record
inline constexpr std::size_t kSlotClass = 0x04;                     // vehicle class pointer
inline constexpr std::size_t kSlotCount = 0x08;                     // planes in the slot
inline constexpr std::size_t kSlotClassField134 = 0x10;             // class+134h copy
inline constexpr std::size_t kSlotState = 0x2C;                     // 1 and 5 = fillable
inline constexpr std::size_t kBlockSlots = 0x4C;                    // slot array
inline constexpr std::size_t kBlockSlotCount = 0x50;
inline constexpr std::size_t kBlockPlaneLimit = 0x58;
inline constexpr std::size_t kBlockOwner = 0x7C;                    // session owner (unit)
inline constexpr const char* kTrafficKey = "traffic";               // 00CE5890
inline constexpr std::uint32_t kCallbackListVtable = 0x00CF87D4;
inline constexpr std::uint32_t kCallbackList8888Vtable = 0x00CF8888;
inline constexpr std::uint32_t kFollowObserverVtable = 0x00CF8900;
inline constexpr std::uint32_t kAirOpsSiteVtable = 0x00CF8948;
inline constexpr std::uint32_t kAirFieldSiteVtable = 0x00CF89F8;
inline constexpr double kDistanceEpsilon = 1e-10;  // 00CE3820, double
inline constexpr std::uint32_t kSupportItemVtable0 = 0x00CF6564;
inline constexpr std::uint32_t kSupportItemVtable14 = 0x00CF6514;
inline constexpr std::uint32_t kSupportItemVtable38 = 0x00CF652C;

// The {count, ...} pair 006BF230 writes: +0h free stock, +4h available.
struct StockPair {
    std::int32_t free;
    std::int32_t available;
};

// The archive's tagged by-value argument: kind 0 = C string key, 1 = index, 2 = float (value
// bits, or float* when read), 4 = object reference (or object** when read).
struct ArchiveArg {
    std::uint32_t kind;
    std::uintptr_t value;
};
inline constexpr const char* kReadyPlaneKey = "readyPlane";            // 00CF89E8
inline constexpr const char* kLastLandingTimeKey = "lastLandingTime";  // 00CF89D8
inline constexpr const char* kPlanesKey = "planes";                    // 00CF8880
inline constexpr std::uint32_t kFollowObserverByValueVtable = 0x00CF89B4;
inline constexpr const char* kLoadDelayKey = "loadDelay";              // 00CF8ACC
inline constexpr const char* kJustLiftedUpIdKey = "justLiftedUpID";    // 00CF8ABC
inline constexpr const char* kElevatorKey = "elevator";                // 00CF8AB0
inline constexpr std::uint32_t kMotherShipSiteVtable = 0x00CF8A58;
inline constexpr float kThresholdA = 10.0f;    // 00CE38B8, float
inline constexpr double kThresholdB = 100.0;   // 00D7A220, double
inline constexpr std::size_t kAirFieldRunwayFailure = 0x748;  // byte
inline constexpr std::size_t kAirFieldHangarFailure = 0x749;  // byte
inline constexpr const char* kSetTargetToken = "settarget";       // 00CF8B0C
inline constexpr const char* kClearTargetToken = "cleartarget";   // 00CF8B00
inline constexpr const char* kClearOrdersToken = "clearorders";   // 00CF8AF4
inline constexpr const char* kSendFightersToken = "sendfighters"; // 00CF8E34
inline constexpr std::uint32_t kAirFieldLockSingletonGlobal = 0x00E19954;
inline constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818;
inline constexpr std::uint32_t kAirFieldLockSingletonVtable = 0x00CF8E48;
inline constexpr std::uint32_t kAirOpsBlockVtable = 0x00CF88A0;
inline constexpr std::uint32_t kSlotMember38DestroyVtable = 0x00CF64AC;
inline constexpr std::uint32_t kSlotMember14DestroyVtable = 0x00CF6494;
inline constexpr float kSlotRelaunchDelay = 5.0f;  // 00CE3850, float
inline constexpr const char* kAirBaseSection = "_airBase";      // 00CF88BC
inline constexpr const char* kAirFieldSection = "_airField";    // 00CF8EA0
inline constexpr const char* kRunwayWidthKey = "runwayWidth";   // 00CF8E94
inline constexpr const char* kRunwayLengthKey = "runwayLength"; // 00CF8E84
inline constexpr const char* kHangarsKey = "hangars";           // 00CF8E7C
inline constexpr const char* kEntityIdKey = "entityID";         // 00CF8E70
inline constexpr const char* kEntryPathIdKey = "entryPathID";   // 00CF8E64
inline constexpr const char* kExitPathIdKey = "exitPathID";     // 00CF8E58
inline constexpr const char* kGameUnitSection = "gameUnit";     // 00CF8E4C
inline constexpr std::uint32_t kCollisionNodeBaseVtable = 0x00CE89E8;
inline constexpr std::uint32_t kAirFieldCollisionNodeVtable = 0x00CF8B18;
inline constexpr float kRunwayMessageField20 = 256.0f;  // 00CF8B3C
inline constexpr std::uint32_t kSetRunwayFailureVtable = 0x00CF8B40;    // type 72h
inline constexpr std::uint32_t kClearRunwayFailureVtable = 0x00CF8B54;  // type 73h
inline constexpr std::uint32_t kSetHangarFailureVtable = 0x00CF8B68;    // type 74h
inline constexpr std::uint32_t kClearHangarFailureVtable = 0x00CF8B7C;  // type 75h
inline constexpr std::uint32_t kShapeBaseVtable = 0x00CE89DC;
inline constexpr std::uint32_t kRunwayShapeVtable = 0x00CF8B94;
inline constexpr std::uint32_t kShapeOriginGlobal = 0x00F87574;  // vec3 (three floats)
inline constexpr double kHalf = 0.5;                              // 00D7A280, double
inline constexpr std::uint32_t kGameTimeGlobal = 0x00F876A4;      // float

// The debug-checked list iterator the image passes by value: {container, node}.
struct CheckedIter {
    void* container;
    void* node;
};

// ---------------------------------------------------------------------------
// Engine services. Each method names the image routine or virtual slot it stands for.
// ---------------------------------------------------------------------------
struct Seg30Host {
    virtual ~Seg30Host() = default;

    // Globals.
    virtual void* game_e188a8() = 0;

    // CRT and Win32.
    virtual void* operator_new_00bf55be(std::size_t bytes) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void invalid_iterator_00bf6713() = 0;          // STL debug-iterator failure
    virtual void enter_critical_section(void* section) = 0; // [00CE2218]
    virtual void leave_critical_section(void* section) = 0; // [00CE2210]

    // Observer / callback-owner framework (segment 25).
    virtual void callback_owner_destroy_00695870(void* owner) = 0;               // ECX = owner
    virtual void observer_unregister_pair_006952a0(void* target, void* member) = 0;  // ECX, EDX
    virtual void observer_register_pair_00694a60(void* target, void* member) = 0;    // ECX, EDX
    // ECX = observer list, EDX = slot index, stack = argument.
    virtual void observer_notify_slot_00696350(void* list, int slot, std::uint32_t arg) = 0;
    virtual void* list_buy_head_node_006bfa20(void* list) = 0;                   // ECX = list
    virtual void list_clear_006c1430(void* list) = 0;                            // ECX = list

    // Save archive / property visitor (vtable): +4h open section (key), +8h close section,
    // +0Ch write (key, value), +10h read (key, value), +14h has (key). ECX = archive.
    virtual void archive_open_04(void* archive, ArchiveArg key) = 0;
    virtual void archive_close_08(void* archive) = 0;
    virtual void archive_write_0c(void* archive, ArchiveArg key, ArchiveArg value) = 0;
    virtual void archive_read_10(void* archive, ArchiveArg key, ArchiveArg value) = 0;
    virtual bool archive_has_14(void* archive, ArchiveArg key) = 0;
    virtual void object_read_lua_0c(void* object, void* archive) = 0;  // the object's vtable +0Ch
    // ECX = archive, EDX = member, stack = key.
    virtual void save_reader_read_observer_ref_006cf0a0(void* archive, void* member, ArchiveArg key) = 0;
    // ECX = archive, stack = an 18h-byte follow member by value (callee destroys it), key.
    virtual void save_writer_write_observer_ref_006cead0(void* archive, void* member_copy, ArchiveArg key) = 0;
    virtual void follow_observer_member_copy_construct_006ce8a0(void* dst, const void* src) = 0;  // ECX = dst
    virtual void air_ops_site_push_occupant_006ce910(void* occupants, void* plane) = 0;           // ECX

    // Generic object virtuals.
    virtual void scalar_deleting_destructor_0(void* object, int flags) = 0;  // vtable +0
    virtual bool is_kind_of_5c(void* object, int class_id) = 0;              // vtable +5Ch

    // Air-ops block routines (ECX = block).
    virtual int class_plane_limit_006bf3e0(void* block, void* vehicle_class) = 0;
    virtual StockPair* stock_available_006bf230(void* block, StockPair* out, void* vehicle_class) = 0;
    virtual int slot_capacity_006bd460(void* block, int slot) = 0;
    virtual int committed_planes_006bd3f0(void* block) = 0;
    virtual void publish_slot_006bf150(void* block, int slot) = 0;
    virtual void fill_slot_006c0f00(void* block, int slot, void* vehicle_class, int count) = 0;
    virtual bool squadron_not_excluded_006c4790(void* block, void* squadron) = 0;
    // ECX = message, (block, slot).
    virtual void airbase_setslot_message_construct_006bd800(void* message, void* block, int slot) = 0;
    virtual void slot_record_copy_construct_0065caf0(void* dst, const void* src) = 0;  // ECX = dst

    // Units / classes / sessions.
    virtual void* vehicle_class_get_or_create_00964790(std::uint32_t class_id, bool create) = 0;  // ECX, DL
    virtual bool unit_uses_small_turn_roll_limit_0047b880(void* unit) = 0;          // ECX = unit
    // ECX = owner, (message, peer).
    virtual void session_owner_broadcast_0077c7b0(void* owner, void* message, std::uint32_t peer) = 0;
    virtual void entity_send_message_to_peer_00779fc0(void* owner, std::uint32_t peer, void* message) = 0;

    // std::list helpers (checked iterators {container, node}; list = {+0h, +4h head, +8h size}).
    virtual void list_remove_006c8750(CheckedIter* out, void* const* value, CheckedIter first,
                                      CheckedIter last) = 0;                      // ECX = out, EDX = value
    virtual void list_erase_range_006c7030(void* list, CheckedIter* out, CheckedIter first,
                                           CheckedIter last) = 0;                 // ECX = list
    virtual void* list_buy_node4_006bfbd0(void* list, void* next, void* prev, void* const* value) = 0;
    virtual void list_increment_size_006c89d0(void* list, std::uint32_t count) = 0;  // throws length_error

    // More object virtuals.
    virtual void* object_virtual_04(void* object) = 0;            // vtable +4h, no arguments
    virtual void object_virtual_0_noargs(void* object) = 0;       // vtable +0, no arguments (release)
    virtual std::int32_t interlocked_decrement(std::int32_t* value) = 0;  // [00CE2220]
    virtual void mothership_step_elevator_frame_007586d0(void* mothership, float dt) = 0;  // ECX

    // Entity world-pose cache (entity+C8h valid flag, +74h local, +CCh world, +3Ch parent).
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;                    // ECX
    virtual void* matrix_multiply_4x4_00413920(void* lhs, void* out, const void* rhs) = 0;  // ECX = lhs
    virtual void matrix_copy_4x4_x87_004134f0(void* dst, const void* src) = 0;            // ECX = dst
    virtual float crt_sqrt_00bf7030(float value) = 0;                                     // _CIsqrt on ST0

    // Air-ops sites.
    virtual void follow_observer_member_destroy_006cdd70(void* member) = 0;         // ECX = member
    virtual void* operator_new_00bf681b(std::size_t bytes) = 0;
    virtual void* air_ops_site_construct_006cf100(void* site) = 0;                   // ECX = site
    // ECX = first, EDX = last, stack = &value; returns the new end (std::remove on dwords).
    virtual std::uint32_t* dword_remove_006ced40(std::uint32_t* first, std::uint32_t* last,
                                                 const std::uint32_t* value) = 0;

    // Mothership site.
    virtual void point_effect_stop_children_00867b10(void* effect) = 0;               // ECX
    virtual void elevator_platform_destruct_006fc2e0(void* platform) = 0;             // ECX
    virtual void* game_tuning_singleton_0042e740() = 0;
    // ECX = holder, (out vec3, world point); returns out.
    virtual float* air_ops_holder_local_point_006bcc90(void* holder, float* out, const float* point) = 0;
    virtual void* mothership_site_construct_006cfaf0(void* site, void* mothership) = 0;  // ECX = site
    // ECX = archive, (out 0Ch-byte section reference, key); returns out.
    virtual void* archive_reader_ref_open_00bd4fe0(void* archive, void* out_ref, ArchiveArg key) = 0;
    virtual void* archive_writer_ref_open_00bd5010(void* archive, void* out_ref, ArchiveArg key) = 0;
    // ECX = platform, stack = the 0Ch-byte section reference by value (callee destroys it).
    virtual void elevator_platform_read_state_006fbf00(void* platform, void* section_ref) = 0;
    virtual void elevator_platform_write_state_006fbfd0(void* platform, void* section_ref) = 0;

    // MAirfield.
    virtual std::uint32_t& global_u32(std::uint32_t address) = 0;  // a .data dword by address
    virtual void mission_entity_attach_lua_self_0077e830(void* entity) = 0;                  // ECX
    // ECX = director, (owner unit sub-object or null, 0); returns the director.
    virtual void* weapon_director_base_construct_008363e0(void* director, void* owner, std::uint32_t arg) = 0;
    virtual void unit_instantiate_class_devices_0095a880(void* unit, std::uint32_t a, std::uint32_t b,
                                                         std::uint32_t c) = 0;              // ECX
    virtual bool message_is_type_0c(void* message, std::uint32_t type) = 0;                  // vtable +0Ch
    virtual bool may_follow_006ba6a0(void* sub_object, const char* token, void* target) = 0;  // ECX
    virtual bool entity_may_follow_target_00779d50(void* entity, const char* token, void* target) = 0;
    virtual int cstring_compare_insensitive_00438e10(const char* lhs, const char* rhs) = 0;  // ECX, EDX
    virtual void collision_node_init_fields_004e6480(void* node) = 0;                        // ECX
    virtual void session_message_construct_base_0075b430(void* message, std::uint32_t type) = 0;  // ECX
    virtual void collision_shape_set_bounds_0098aab0(void* shape, const float* min_corner,
                                                     const float* max_corner) = 0;              // ECX
    virtual void* scene_property_bag_find_008f2260(void* bag, const char* name) = 0;           // ECX
    virtual const char* scene_property_get_string_008f0df0(void* property) = 0;                // ECX
    virtual void* entity_registry_find_by_name_00925a90(void* registry, const char* name) = 0;  // ECX
    virtual void air_field_virtual_204(void* airfield, void* message) = 0;                     // vtable +204h
    virtual void air_field_virtual_208(void* airfield, void* message) = 0;                     // vtable +208h
    virtual bool air_ops_handle_session_message_006cd6c0(void* block, void* message) = 0;      // ECX
    virtual bool unit_handle_message_0095abe0(void* unit, void* message) = 0;                  // ECX
    virtual void* air_field_find_usable_hangar_object_006d2730(void* airfield) = 0;            // ECX
    virtual void object_virtual_c4(void* object, float* out) = 0;                              // vtable +C4h
    virtual const char* empty_string_00e1996c() = 0;
    virtual void* critical_section_create_00bd1860() = 0;
    // ECX = point, (out, matrix).
    virtual void vector3_transform_affine_point_004142e0(const float* point, float* out, const void* matrix) = 0;
    virtual void hit_record_append_part_hit_006d2e30(void* hits, const void* hit) = 0;  // ECX = hits
    virtual void* singleton_lifetime_manager_00415350() = 0;
    virtual void singleton_lifetime_register_00bd0c30(void* manager, void* object) = 0;  // ECX = manager
    virtual void unit_list_erase_node_004837d0(void* list, void* node) = 0;               // ECX = list
    virtual void unit_list_push_back_00484540(void* list, void* unit) = 0;                // ECX = list
    virtual void game_entity_register_in_parent_list_00928560(void* entity) = 0;          // ECX
    virtual void unit_detach_from_early_world_lists_006d3620(void* unit) = 0;             // ECX
    virtual void critical_section_destroy_owned_0041cc80(void* owner) = 0;                // ECX
    virtual void free_00bf65ac(void* block) = 0;  // the _free thunk
    // ECX = tree, (out, first, last).
    virtual void tree_erase_range_006c93f0(void* tree, CheckedIter* out, CheckedIter first, CheckedIter last) = 0;
    virtual void list_tidy_0065b590(void* list) = 0;                    // ECX
    virtual void observer_member_cleanup_00653390(void* member) = 0;    // ECX
    virtual void award_index_list_destroy_00487110(void* list) = 0;     // ECX
    virtual void* spatial_index_singleton_0042e630() = 0;
    virtual void spatial_index_detach_node_0098a500(void* index, void* node) = 0;  // ECX = index
    virtual void object_virtual_04_delete(void* object, int flags) = 0;            // vtable +4h(flags)
    virtual void unit_game_object_destruct_00959940(void* unit) = 0;              // ECX
    virtual void* class_create_entity_28(void* vehicle_class, std::uint32_t id) = 0;  // vtable +28h
    virtual void native_string_resize_0041dd40(void* text, std::uint32_t length, std::uint32_t arg) = 0;  // ECX
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t count) = 0;
    virtual void* object_virtual_114(void* object) = 0;           // vtable +114h, no arguments
    virtual std::int32_t object_virtual_18_int(void* object) = 0;  // vtable +18h, no arguments
    virtual bool air_ops_squadron_target_within_range_006bc5e0(void* squadron) = 0;  // ECX
    virtual void* air_ops_slot_squadron_handle_006bd620(void* message) = 0;           // ECX
    virtual void vehicle_unit_load_archive_common_0095bb10(void* unit, void* archive) = 0;      // ECX
    virtual void air_ops_block_declare_lua_properties_006c19d0(void* block, void* archive) = 0;  // ECX
    virtual void* path_object_get_entity_007aca30(void* path) = 0;                              // ECX
    virtual void weapon_director_visit_properties_008360c0(void* unit, void* archive) = 0;      // ECX
};

// ---------------------------------------------------------------------------
// Segment 30: air-ops block.
// ---------------------------------------------------------------------------
// Oct18b-nA2: new normal-body calls; separate interface leaves existing
// Seg30Host implementations unchanged. No concrete adapter/game wiring added.
struct AirFieldLeafHost : Seg30Host {
    virtual void air_ops_reset_slots_006bd360(void*) = 0;
    virtual void unit_party_race_00951f30(void*,std::uint32_t,std::uint32_t,std::uint32_t*) = 0;
    virtual void child_party_race_2c(void*,std::uint32_t,std::uint32_t,std::uint32_t*) = 0;
    virtual void entity_float_0042ed80(void*,float) = 0;
    virtual void child_float_f4(void*,float) = 0;
    virtual bool native_string_equals_00425850(const void*,const char*) = 0;
    virtual bool unit_set_failure_00953da0(void*,void*,std::uint32_t) = 0;
    virtual bool unit_query_failure_00953e60(void*,void*,std::uint32_t,std::uint8_t*) = 0;
    virtual bool unit_clear_failure_00953ea0(void*,void*,std::uint32_t) = 0;
    virtual void route_message_0077c2a0(void*,void*,std::uint32_t,std::uint32_t) = 0;
    virtual void command_registry_find_0077a020(void*) = 0;
    virtual const void* command_from_ordinal_0077a050(void*) = 0;
    virtual const void* image_address(std::uint32_t) = 0;
    virtual void director_set_target_00835860(void*,std::uint32_t,std::uint32_t) = 0;
};
// Names below are hypotheses. Original ABI and complete-body evidence:
// 006BF1B0 ECX=block,index/RET4; checked list+40h, node payload+8, or zero.
void* air_ops_holder_at_006bf1b0(Seg30Host&,void*,std::uint32_t);
// 006C0020 ECX=archive,key(kind,value),object/RET0C; slot+0Ch receives key
// and by-value {4,object}. 006CE290 ECX=observer,flags/RET4,EAX=identity,
// stamps CF88F8, free only bit0. 006CFE10 ECX=site,RET; [+B8]+1188 or0.
void archive_write_object_006c0020(Seg30Host&,void*,ArchiveArg,void*);
void* observer_scalar_delete_006ce290(Seg30Host&,void*,std::uint32_t);
void* mothership_site_air_ops_006cfe10(void*) noexcept;
// 006D0C60 ECX=self+72Ch ->006BD360, RET. 006D1D50 RET4; 006D1E90 and
// 006D1EA0 RET; 006D1ED0 AL=0 RET4. 006D1FB0 EAX=[ECX+C4h], RET.
void air_field_reset_slots_006d0c60(AirFieldLeafHost&,void*);
void air_field_ignore_006d1d50(std::uint32_t) noexcept;
void unit_empty_hook_006d1e90() noexcept;
bool unit_false_hook_006d1ed0(std::uint32_t) noexcept;
std::uint32_t unit_class_006d1fb0(void*) noexcept;
// 006D2070 ECX=self,3 args/RET0C: *arg3=0Fh, base00951F30, then child
// slot+2Ch over12-byte records at+830/+834; reload end after every call.
void air_field_party_race_006d2070(AirFieldLeafHost&,void*,std::uint32_t,std::uint32_t,std::uint32_t*);
// 006D2210/23D0 ECX=self,NativeString*,value/RET8,AL success. Literal
// RunwayFailure/HangarFailure build72/74 or73/75, route7/flags0; else base.
// 006D2350 adds byte*out/RET0C, reads +748/+749 on match; else base953E60.
bool air_field_set_failure_006d2210(AirFieldLeafHost&,void*,void*,std::uint32_t);
bool air_field_clear_failure_006d23d0(AirFieldLeafHost&,void*,void*,std::uint32_t);
bool air_field_query_failure_006d2350(AirFieldLeafHost&,void*,void*,std::uint32_t,std::uint8_t*);
// 006D2850 ECX=self,message/RET4. Registry call, copy24h..3Bh BEFORE
// ordinal lookups; resolve local target through00521EA0, director=self+8C8h.
void air_field_target_order_006d2850(AirFieldLeafHost&,void*,void*,const NativeCommandTargetHandleTables&);
// 006D3EB0 ECX=self,float/RET4: base0042ED80, then child+F4h; reload end.
void air_field_forward_float_006d3eb0(AirFieldLeafHost&,void*,float);
// 006D60B0 ECX=self,peer/RET4. Stack148h AIRFIELD_CREATE, source+7AC's
// B0/B4 floats or0; send779FC0, destruct6D4420 on normal/EH exit. New C++
// interface uses the existing message family; no native EH/binary ABI proof.
void air_field_send_create_006d60b0(Seg30Host&,
    airfield_marker_seg31_32_argon::Seg31_32Host&,void*,std::uint32_t);
void buffered_callback_owner_destruct_006beec0(Seg30Host& host, void* self);
void air_ops_block_read_traffic_table_006bef30(Seg30Host& host, void* block, void* lua);
int air_ops_total_plane_count_006bf350(Seg30Host& host, void* block);
void air_ops_holder_list_unlink_006bf880(void* list, void* holder);
bool air_ops_can_fill_slot_006c0e70(Seg30Host& host, void* block, int slot, void* vehicle_class, int count);
bool air_ops_can_fill_slot_006c10d0(Seg30Host& host, void* block, int slot, void* vehicle_class, int count);
void air_ops_set_slot_class_and_count_006c1170(Seg30Host& host, void* block, int slot, void* vehicle_class,
                                               std::int32_t count);
// The four identical observer-member setters (target at +14h).
void* observer_member_set_target_006c1310(Seg30Host& host, void* member, void* target);
void* observer_member_set_target_006c1350(Seg30Host& host, void* member, void* target);
void* observer_member_set_target_006c4990(Seg30Host& host, void* member, void* target);
void* observer_member_set_target_006c49d0(Seg30Host& host, void* member, void* target);
void support_manager_item_construct_006c1960(void* self, std::uint32_t arg);
void air_ops_apply_slot_setup_006c47f0(Seg30Host& host, void* block, void* message);
void native_vector58_grow_006c4d80(Seg30Host& host, void* vector);
void* air_ops_callback_list_construct_006c4fa0(Seg30Host& host, void* self);
void air_ops_callback_list_destruct_006c5000(Seg30Host& host, void* self);
bool air_ops_block_accepts_squadron_for_landing_006c5640(Seg30Host& host, void* block, void* squadron);
bool air_ops_block_accepts_plane_for_landing_006c56b0(Seg30Host& host, void* block, void* plane);

void native_vector58_push_back_006c6c60(Seg30Host& host, void* vector, const void* record);
void* air_ops_callback_list8888_construct_006c6e30(Seg30Host& host, void* self);
void air_ops_callback_list_destruct_006c71c0(Seg30Host& host, void* self);
void air_ops_callback_list8888_on_notify_006c88d0(Seg30Host& host, void* self, void* source);
void air_ops_callback_list_add_006ca9c0(Seg30Host& host, void* self, void* target);
void free_owned_block_006cde40(Seg30Host& host, void* self);
void native_dword_vector_grow_006cdea0(Seg30Host& host, void* vector);
void buffered_callback_owner_destruct_006cdf40(Seg30Host& host, void* self);
void mothership_step_forwarder_006ce260(Seg30Host& host, void* self, float dt);
void observer_member_set_target_006ce300(Seg30Host& host, void* member, void* target);  // returns nothing
void native_dword_vector_push_back_006ce330(Seg30Host& host, void* vector, const std::uint32_t* value);
void* ref_ptr_release_006ce400(Seg30Host& host, void* self);
void buffered_callback_owner_destruct_006ce430(Seg30Host& host, void* self);
void* follow_observer_member_construct_006cea10(Seg30Host& host, void* self, void* target);
void* observer_member_set_target_006cea70(Seg30Host& host, void* member, void* target);
void ref_ptr_release_006ceb40(Seg30Host& host, void* self);
void* observer_member_set_target_006cef40(Seg30Host& host, void* member, void* target);

void* air_ops_site_scan_occupants_near_006ce4f0(Seg30Host& host, void* site, const float* point, float radius);
void air_ops_site_destruct_006ceb70(Seg30Host& host, void* site);
void air_ops_site_on_ready_plane_gone_006cee80(Seg30Host& host, void* member, void* source);
void air_ops_site_occupants_on_notify_006ceff0(Seg30Host& host, void* self, void* source);
void* air_field_site_create_006cfa80(Seg30Host& host, void* airfield);
void air_ops_site_read_state_006cf1c0(Seg30Host& host, void* site, void* archive);
void air_ops_site_write_state_006cf2a0(Seg30Host& host, void* site, void* archive);
void mothership_site_destruct_006d0470(Seg30Host& host, void* site);
bool mothership_site_has_free_runway_006d0530(Seg30Host& host, void* site);
void mothership_site_read_state_006d07b0(Seg30Host& host, void* site, void* archive);
void mothership_site_write_state_006d0850(Seg30Host& host, void* site, void* archive);
void* mothership_site_create_006d0a10(Seg30Host& host, void* mothership);

// ---------------------------------------------------------------------------
// Segment 30: MAirfield and its runway messages.
// ---------------------------------------------------------------------------
std::uint32_t class_record_positive_thresholds_006d0c30(void* record);
void air_field_init_lua_and_weapon_director_006d0c80(Seg30Host& host, void* airfield);
void air_field_clear_failure_flags_on_set_006d0d50(Seg30Host& host, void* airfield, void* message);
void air_field_clear_failure_flags_on_clear_006d0d90(Seg30Host& host, void* airfield, void* message);
bool air_field_may_follow_target_006d0df0(Seg30Host& host, void* airfield, const char* token, void* target);
void air_field_lock_singleton_reset_006d0e90(Seg30Host& host, void* singleton);
void free_and_null_006d1070(Seg30Host& host, void* holder);
void native_vector12_reserve_006d1170(Seg30Host& host, void* vector, std::uint32_t capacity);
void native_vector12_grow_006d11e0(Seg30Host& host, void* vector);
void* air_field_collision_node_construct_006d1430(Seg30Host& host, void* node, std::uint32_t owner);
void air_field_collision_node_destruct_006d1460(Seg30Host& host, void* node);
bool air_field_collision_node_is_kind_006d1530(std::uint32_t kind);
// The four AIRBASE_{SET,CLEAR}_{RUNWAY,HANGAR}FAILURE messages, constructed for reading.
void* airbase_set_runway_failure_construct_006d16a0(Seg30Host& host, void* message);
void* airbase_clear_runway_failure_construct_006d1720(Seg30Host& host, void* message);
void* airbase_set_hangar_failure_construct_006d17a0(Seg30Host& host, void* message);
void* airbase_clear_hangar_failure_construct_006d1820(Seg30Host& host, void* message);
void* air_field_runway_shape_construct_006d1990(Seg30Host& host, void* shape, std::uint32_t owner, float width,
                                                float length, std::uint32_t kind);
void native_vector16_grow_006d1a90(Seg30Host& host, void* vector);
void native_vector16_push_back_006d1b60(Seg30Host& host, void* vector, const void* element);
void* unit_get_390_006d1ec0(void* unit);
void* unit_get_class_type_name_006d1f30(void* unit);
void unit_get_default_extent_zero_006d1f50(float* out);
void unit_stamp_attack_times_006d1ff0(Seg30Host& host, void* unit, void* attacker, std::uint32_t unused);
float* scene_position_from_named_entity_006d2190(Seg30Host& host, float* out, void* bag, const char* name,
                                                 const float* fallback);
bool air_field_supports_command_006d25f0(Seg30Host& host, const char* token);
bool air_field_handle_message_006d2910(Seg30Host& host, void* airfield, void* message);
float* air_field_get_world_position_006d2a90(Seg30Host& host, void* airfield, float* out);
void native_vector12_push_back_006d2c10(Seg30Host& host, void* vector, const void* element);
void native_vector12_resize_006d2cb0(Seg30Host& host, void* vector, std::uint32_t count);
std::uint32_t unit_get_6f4_field_006d2fa0(void* unit);
const char* air_field_get_name_006d2fc0(Seg30Host& host, void* airfield);
void* air_field_lock_singleton_construct_006d2ff0(Seg30Host& host, void* self);
void air_field_collision_fill_hit_record_006d3040(Seg30Host& host, void* node, void* record, const float* point);
void air_field_collision_append_part_hit_006d30b0(Seg30Host& host, void* hits, float distance);
void* air_field_lock_singleton_get_006d32f0(Seg30Host& host);
void* entity_as_kind1b_006d3440(Seg30Host& host, void* entity);
void unit_register_in_world_lists_006d35f0(Seg30Host& host, void* unit);
void unit_register_in_world_lists_006d3670(Seg30Host& host, void* unit);
void unit_game_object_detach_from_world_lists_006d36a0(Seg30Host& host, void* unit);
void air_field_detach_from_world_lists_006d3710(Seg30Host& host, void* airfield);
void air_field_lock_singleton_destruct_006d3770(Seg30Host& host, void* singleton);
void air_ops_block_destruct_006ca410(Seg30Host& host, void* block);
void air_field_destruct_006d3b10(Seg30Host& host, void* airfield);
void* airfield_create_message_create_entity_006d4210(Seg30Host& host, void* message);
bool air_ops_slot_state_from_squadron_command_006cd550(Seg30Host& host, void* block, int slot, int mode);
void air_ops_apply_slot_record_message84_006c83b0(Seg30Host& host, void* block, void* message);
void air_field_serialize_006d37e0(Seg30Host& host, void* airfield, void* archive);

// ---------------------------------------------------------------------------
// Segment 50 (src/cc13_seg30_seg50.cpp).
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kControllerSnapshotBaseVtable = 0x00CFDB90;
inline constexpr std::uint32_t kControllerSnapshotVtable = 0x00CFDB9C;
inline constexpr std::uint32_t kControllerSnapshotExVtable = 0x00CFDBA8;
inline constexpr std::uint32_t kShipCreateMessageVtable = 0x00D09410;
inline constexpr std::uint32_t kShipCreateMessage114Vtable = 0x00CFF354;
inline constexpr std::uint32_t kHelmsmanControlMessageVtable = 0x00D02DA8;       // type 8Eh
inline constexpr std::uint32_t kLandingShipsLaunchedMessageVtable = 0x00D03388;  // type 95h
inline constexpr float kSyncSpeedThreshold = 0.5f;       // 00CE3800
inline constexpr float kSyncRange3 = 600.0f;             // 00CE4BC4
inline constexpr float kSyncRange4 = 1400.0f;            // 00CED560
inline constexpr float kSyncRange5 = 4000.0f;            // 00D0946C
inline constexpr float kSyncRange6 = 3.40282347e+38f;    // 00D7A248 (FLT_MAX)
inline constexpr const char* kKotelNeedle = "kotel";     // 00D0949C
inline constexpr const char* kRopeNeedle = "rope";       // 00D09494
inline constexpr double kHullEndMargin = 20.0;           // 00CE3D88, double
inline constexpr float kNegativeZero = -0.0f;            // 00D7A208
inline constexpr double kHelmMinStepWide = static_cast<double>(0.2f);  // 00CE3D10, double (3FC99999A0000000)
inline constexpr float kHelmMinStep = 0.2f;              // 00CE54A0, float
inline constexpr double kHalfPiWide = 1.5707963705062866;  // 00CE3830, double (the float pi/2 widened)
inline constexpr double kTwoPiWide = 6.2831854820251465;   // 00CE3828, double (the float 2pi widened)
inline constexpr float kMinusOne = -1.0f;                // 00D7A260
inline constexpr float kOne = 1.0f;                      // 00D7A24C

struct Seg50Host : Seg30Host {
    virtual void command_target_descriptor_init_00721070(void* descriptor) = 0;  // ECX
    virtual void net_entity_create_message_construct_00766f60(void* message) = 0;  // ECX
    virtual void net_entity_create_message_destruct_007668e0(void* message) = 0;   // ECX
    virtual void object_virtual_10(void* object) = 0;                              // vtable +10h, no arguments
    virtual void node_unlink_and_release_00b6dfa0(void* node) = 0;                 // ECX
    virtual void matrix_build_affine_inverse_00b63d50(void* out, const void* matrix) = 0;  // ECX, EDX
    virtual void replication_build_sync_delta_0077b280(void* entity, void* record, std::uint32_t flag, float t0,
                                                       int k0, float t1, int k1, float t2, int k2, float t3,
                                                       int k3, float t4, int k4) = 0;  // cdecl
    // ECX = message, (a3, a1, ship, 1).
    virtual void net_entity_create_message_construct_00766fc0(void* message, std::uint32_t a3, std::uint32_t a1,
                                                              void* ship, std::uint32_t one) = 0;
    virtual void* controller_snapshot_build_007214c0(void* snapshot, void* director) = 0;  // ECX
    virtual std::int32_t unit_virtual_204_int(void* unit) = 0;                            // vtable +204h
    virtual void create_message_store_carrier_type_00809290(void* part, void* source) = 0;  // ECX
    virtual void net_entity_create_message_write_00765000(void* message, void* writer) = 0;  // ECX
    virtual void net_entity_create_message_read_00767e30(void* message, void* stream) = 0;   // ECX
    virtual void controller_snapshot_write_00721550(void* snapshot, void* writer) = 0;      // ECX
    virtual void controller_snapshot_read_00721610(void* snapshot, void* stream) = 0;       // ECX
    // Bit cursor (ECX = writer or reader).
    virtual void bit_write_bool_004290b0(void* writer, std::uint32_t value) = 0;
    virtual void bit_write_word_00429120(void* writer, std::uint32_t value, std::uint32_t bits) = 0;
    virtual void bit_write_unsigned_00429070(void* writer, std::uint32_t value, std::uint32_t bits) = 0;
    virtual void bit_write_signed_00429090(void* writer, std::uint32_t value, std::uint32_t bits) = 0;
    virtual void bit_write_byte_00428ff0(void* writer, std::uint32_t value, std::uint32_t bits) = 0;
    virtual void bit_read_bool_00428d70(void* reader, std::uint8_t* out) = 0;
    virtual void bit_read_word_00428e30(void* reader, std::uint16_t* out, std::uint32_t bits) = 0;
    virtual void bit_read_unsigned_00428d10(void* reader, std::uint32_t* out, std::uint32_t bits) = 0;
    virtual void bit_read_signed_00428d30(void* reader, std::uint32_t* out, std::uint32_t bits) = 0;
    virtual void bit_read_byte_00428c70(void* reader, std::uint8_t* out, std::uint32_t bits) = 0;
    virtual void session_message_write_extended_header_0075b480(void* message, void* writer) = 0;  // ECX
    virtual void session_message_read_extended_header_0075b4c0(void* message, void* stream) = 0;   // ECX
    virtual void turning_gun_write_snapshot_00859b10(void* gun, void* scratch, void* writer) = 0;  // ECX
    virtual void object_virtual_18_stream(void* object, void* stream) = 0;                       // vtable +18h
    virtual void unit_ai_order_slot_copy_00811d10(void* dst, const void* src) = 0;      // ECX = dst
    virtual void float_array_reserve_008153e0(void* vector, std::int32_t capacity) = 0;  // ECX
    virtual void tick_subnode_base_cleanup_00875b30(void* node) = 0;                    // ECX
    virtual void unit_instance_set_world_matrix_009585b0(void* unit, const void* matrix) = 0;  // ECX
    virtual void motion_proxy_sync_transform_0092d620(void* proxy, const void* matrix) = 0;    // ECX
    virtual void unit_reset_pose_dependent_state_00818ea0(void* unit) = 0;                     // ECX
    virtual void points_array_reserve_capacity_0074d190(void* array, std::int32_t capacity) = 0;  // ECX
    virtual bool device_virtual_1d0(void* device, std::uint32_t arg) = 0;            // vtable +1D0h
    virtual std::uint32_t game_effective_mode_004bca50() = 0;
    virtual double player_points_00946f00(std::int32_t player) = 0;                 // ECX = player, ST0
    virtual float catapult_launch_heading_006ed000(void* catapult, float heading) = 0;  // ECX, ST0
    virtual float math_subtract_wrapped_angle_00438b10(float a, float b) = 0;          // ST0
    virtual void catapult_queue_launch_006eb680(void* catapult, float heading) = 0;     // ECX
    virtual void* node_get_name_header_00b6d800(void* node) = 0;                        // ECX
    virtual const char* strstr_00bf9440(const char* text, const char* needle) = 0;
    virtual void scene_node_set_visibility_factor_00b6da70(void* node, float factor, std::uint32_t arg) = 0;  // ECX
    virtual const float* node_copy_local_position_00b6e0a0(void* node, float* out) = 0;  // ECX
    virtual void node_virtual_2c_set_position(void* node, const float* position) = 0;     // vtable +2Ch
    // ECX = value, EDX = min, stack = max.
    virtual void math_clamp_in_place_00415690(void* value, const void* min, const void* max) = 0;
    virtual double crt_atan2_00bf701a(double y, double x) = 0;      // _CIatan2: ST1 = y, ST0 = x
    virtual double vector3_length_0042b2f0(const float* vector) = 0;  // ECX, ST0
    // ECX = class, (1.0f, speed, 1.0f); ST0.
    virtual double ship_class_yaw_rate_from_rudder_0082ecb0(void* vehicle_class, float a, float speed,
                                                            float b) = 0;
};

void* controller_snapshot_construct_0080ddb0(Seg50Host& host, void* self);
void* controller_snapshot_ex_construct_0080de00(Seg50Host& host, void* self);
void* ship_sync_state_get_or_create_0080e240(Seg50Host& host, void* ship);
void* ship_create_message_construct_for_read_0080e2c0(Seg50Host& host, void* message);
void ship_create_message_destruct_0080e380(Seg50Host& host, void* message);
void buffer_free_0080ee90(Seg50Host& host, void* holder);
void buffer_free_and_null_0080ef70(Seg50Host& host, void* holder);
void object_ptr_release_virtual10_0080ef90(Seg50Host& host, void* holder);
void node_ptr_unlink_and_release_0080f0f0(Seg50Host& host, void* holder);
void vec3_array_grow_0080f2f0(Seg50Host& host, void* vector);

void* ship_helmsman_control_message_construct_0080fd90(Seg50Host& host, void* message, const std::uint32_t* payload);
void* ship_landing_ships_launched_message_construct_0080fdd0(Seg50Host& host, void* message, float value);
float unit_local_x_of_fa0_point_00811820(Seg50Host& host, void* unit);
bool target_is_kind29_with_class_in_set_008129e0(Seg50Host& host, const void* target);
void ship_build_sync_delta_008138a0(Seg50Host& host, void* ship, void* record);
void* ship_create_message_construct_00813a20(Seg50Host& host, void* message, std::uint32_t a1, void* ship,
                                             std::uint32_t a3);
void ship_create_message_write_00813b10(Seg50Host& host, void* message, void* writer);
void ship_create_message_read_00813bd0(Seg50Host& host, void* message, void* stream);
void ship_guns_sync_message_write_00814000(Seg50Host& host, void* message, void* writer);
void ship_guns_sync_message_read_008140d0(Seg50Host& host, void* message, void* stream);
void vec3_array_push_back_00814f70(Seg50Host& host, void* vector, const float* value);
void* ref_ptr_release_00815060(Seg50Host& host, void* holder);
void* ref_ptr_release_00815200(Seg50Host& host, void* holder);
void ref_ptr_release_00817bd0(Seg50Host& host, void* holder);
void ref_ptr_release_0081a590(Seg50Host& host, void* holder);
void ref_ptr_release_0081a8a0(Seg50Host& host, void* holder);
void unit_flip_pending_ai_order_slot_00815ee0(Seg50Host& host, void* unit);
void ship_send_create_message_to_peer_00816990(Seg50Host& host, void* ship, std::uint32_t peer);
void float_array_push_back_00817750(Seg50Host& host, void* vector, const float* value);
void float_array_push_back_0081a4f0(Seg50Host& host, void* vector, const float* value);
void ship_explosion_failure_task_destruct_00818290(Seg50Host& host, void* task);
void unit_set_world_matrix_and_sync_00819840(Seg50Host& host, void* unit, const void* matrix);
void points_array_clear_0081a5c0(Seg50Host& host, void* array);
void ship_launch_from_catapult_008141e0(Seg50Host& host, void* ship, float heading, std::int32_t player);
void scene_node_toggle_kotel_rope_nodes_00815d20(Seg50Host& host, void* self, void* node, std::int32_t hide);
void unit_nearer_hull_end_point_008160f0(Seg50Host& host, void* unit, float* out, const float* target);
void helm_channel_rate_limit_towards_00810900(Seg50Host& host, void* channel, float target, float dt);
void helm_estimator_update_00810a90(Seg50Host& host, void* estimator, float x, float y, float z, float heading,
                                    float time);
void helm_estimator_update_from_vector_00810cf0(Seg50Host& host, void* unit, const float position[3],
                                                const float direction[3], const float unused[3], float time);

}  // namespace bsp::cc13_seg30

#endif  // BSP_CC13_SEG30_HPP
