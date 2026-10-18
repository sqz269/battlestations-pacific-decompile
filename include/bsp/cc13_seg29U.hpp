// cc13 segment 29 cc9-lane leftovers (packet cc13-seg29U, cc9 approval 2026-10-18): the
// amphibious landing-point bodies 006AC370 / 006ACB40 / 006ACDC0 and the air-ops helpers from
// 006BA620 up. docs/CC13_SEG29U.md carries the evidence and the adoption notes against cc9's
// host models.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as in
// the image) and reaches the engine through Seg29UHost, whose methods name the image routine or
// virtual slot they stand for and give the register inputs. Constants carry their image address
// and width. New C++ interfaces, not drop-in binary replacements; nothing here is bound into
// bsp_game. Names are hypotheses, not recovered symbols.

#ifndef BSP_CC13_SEG29U_HPP
#define BSP_CC13_SEG29U_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg29U {

template <class T>
inline T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
inline void* ptr(void* base, std::size_t off) noexcept { return at<void*>(base, off); }
inline void* add(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) + off;
}

// The engine's 8-byte string {length, data}; data null means empty.
struct NativeStr {
    std::uint32_t length;
    char* data;
};

// ---------------------------------------------------------------------------
// Image constants.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kLandingPointVtable = 0x00CE90E0;      // 006ACB61
inline constexpr std::uint32_t kLandingPointVtable10 = 0x00CE90C4;    // 006ACB68
inline constexpr std::uint32_t kLandingPointVtable24 = 0x00CE90BC;    // 006ACB6F
inline constexpr std::uint32_t kLandingPointVtable170 = 0x00CE90B8;   // 006ACB76
inline constexpr std::uint32_t kCallbackOwnerVtable = 0x00CE74FC;     // 006ACC4F
inline constexpr std::uint32_t kGameObjectGlobal = 0x00E188A8;
inline constexpr std::uint32_t kEmptyCString = 0x00E198ED;            // 006ACEA5
inline constexpr std::int32_t kPathEntityKind = 0x47;                 // IsKindOf argument

struct Seg29UHost {
    virtual ~Seg29UHost() = default;

    // The dword stored at image global `address` ([00E188A8], the game object).
    virtual void* global_pointer(std::uint32_t address) = 0;
    // The address of an image global (00E198ED, the shared empty C string).
    virtual void* global_address(std::uint32_t address) = 0;
    // 00BF6713, the checked-iterator invalid-parameter report (dereferencing end()).
    virtual void invalid_parameter_00bf6713() = 0;
    // 00BF65AC _free / 00BF681B operator new (cdecl).
    virtual void free_00bf65ac(void* block) = 0;
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;

    // --- matrices (006AC370) ------------------------------------------------
    // 00414DB0, __thiscall(ECX = entity): refresh the entity's world pose.
    virtual void entity_pose_refresh_00414db0(void* entity) = 0;
    // 00413920, __thiscall(ECX = lhs 4x4; out, rhs) RET 8 -> EAX = out: lhs * rhs.
    virtual void* matrix_multiply_00413920(void* lhs, void* out, void* rhs) = 0;
    // 004134F0, __thiscall(ECX = dst 4x4; src) RET 4: copy through the x87.
    virtual void matrix_copy_004134f0(void* dst, void* src) = 0;
    // 00B63D50, __fastcall(ECX = out 4x4, EDX = src 4x4): orthogonal scaled affine inverse.
    virtual void matrix_inverse_00b63d50(void* out, void* src) = 0;
    // 004142E0, __thiscall(ECX = point vec3; out vec3, matrix) RET 8 -> EAX = out.
    virtual void* transform_point_004142e0(void* point, void* out, void* matrix) = 0;
    // [object]->vtable[04h](a, b), __thiscall RET 8.
    virtual void object_vslot_04(void* object, std::uint32_t a, void* b) = 0;
    // 007AF150, __thiscall(ECX = path object): derive the path knots.
    virtual void path_derive_knots_007af150(void* path) = 0;
    // 0048CA60, __thiscall(ECX = [game+21D0h]; record, int) RET 8.
    virtual void game_21d0_register_0048ca60(void* owner, void* record, std::int32_t flag) = 0;

    // --- entity teardown (006ACB40) -------------------------------------------
    // 006952A0, __fastcall(ECX = observed, EDX = callback owner): unregister the pair.
    virtual void observer_unregister_pair_006952a0(void* observed, void* owner) = 0;
    // 00695870, __thiscall(ECX = callback owner): BSP_CallbackOwner_Destroy.
    virtual void callback_owner_destroy_00695870(void* owner) = 0;
    // 009287B0, __thiscall(ECX = entity): BSP_GameEntity_Destruct.
    virtual void game_entity_destruct_009287b0(void* entity) = 0;

    // --- scene properties (006ACDC0) -------------------------------------------
    // 009295B0, __thiscall(ECX = entity): the base slot A4h body.
    virtual void entity_default_slot_a4_009295b0(void* entity) = 0;
    // 004260B0, __thiscall(ECX = out; int) RET 4 -> EAX = out: decimal string.
    virtual NativeStr* native_string_from_int_004260b0(NativeStr* out, std::int32_t value) = 0;
    // 0041DD40, __thiscall(ECX = string; length, char preserve) RET 8.
    virtual void native_string_resize_0041dd40(NativeStr* string, std::uint32_t length,
                                               char preserve) = 0;
    // 00BF7680 _memcpy (cdecl).
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t size) = 0;
    // 004261A0, __thiscall(ECX = left; out, right) RET 8: out = left + right.
    virtual void native_string_concat_004261a0(const NativeStr* left, NativeStr* out,
                                               const NativeStr* right) = 0;
    // 00419CC0 (no arguments; the three dwords pushed before it belong to the next call)
    // -> the sized pool, then 00BD1510(ECX = pool; data, size, 1) RET 0Ch: the release every
    // NativeString destructor inlines.
    virtual void* sized_pool_00419cc0() = 0;
    virtual void sized_pool_return_00bd1510(void* pool, void* data, std::uint32_t size,
                                            std::uint32_t count) = 0;
    // 008F2260, __thiscall(ECX = property bag; const char* key) RET 4 -> property or null.
    virtual void* property_bag_find_008f2260(void* bag, const char* key) = 0;
    // 008F0DF0, __thiscall(ECX = property) -> its string value.
    virtual const char* property_string_value_008f0df0(void* property) = 0;
    // 00925A90, __thiscall(ECX = [game+19CCh]; const char* name) RET 4 -> entity or null.
    virtual void* entity_registry_find_00925a90(void* registry, const char* name) = 0;
    // [entity]->vtable[5Ch](kind), __thiscall RET 4 -> AL: IsKindOf.
    virtual bool entity_is_kind_of_5c(void* entity, std::int32_t kind) = 0;
    // 006ACD80 STL_List_PushBack, __thiscall(ECX = list; const T* value) RET 4 (bound STL).
    virtual void list_push_back_006acd80(void* list, void* const* value) = 0;

    // --- air ops (006BA620 and up) ------------------------------------------------
    // [object]->vtable[08h](float dt), __thiscall RET 4 (the traffic object's tick).
    virtual void object_vslot_08_float(void* object, float dt) = 0;
    // [object]->vtable[0Ch]() -> EAX (an entity's kind).
    virtual std::int32_t object_vslot_0c(void* object) = 0;
    // [object]->vtable[10h]() -> ST0 (the block's runway clearance, 006CA5A0 = 35.0 for an
    // airfield block).
    virtual double object_vslot_10_x87(void* object) = 0;
    // [object]->vtable[18h]() -> EAX.
    virtual void* object_vslot_18(void* object) = 0;
    // [object]->vtable[34h](buffer) RET 4 -> EAX = const float[3] (the owner's velocity).
    virtual const float* object_vslot_34(void* object, float* buffer) = 0;
    // [object]->vtable[44h](float out[3]) RET 4 (the traffic runway end point).
    virtual void object_vslot_44(void* object, float* out) = 0;
    // [object]->vtable[114h]() -> EAX (a unit's current command / command list).
    virtual void* object_vslot_114(void* object) = 0;
    // EnterCriticalSection [00CE2218] / LeaveCriticalSection [00CE2210] (stdcall imports).
    virtual void enter_critical_section(void* section) = 0;
    virtual void leave_critical_section(void* section) = 0;
    // 00BF6989 _free (the second CRT thunk, cdecl).
    virtual void free_00bf6989(void* block) = 0;
    // 0075B430 BSP_SessionMessage_ConstructBase, __thiscall(ECX = message; type) RET 4.
    virtual void session_message_construct_base_0075b430(void* message, std::uint32_t type) = 0;
    // 0077C2A0 BSP_Session_RouteMessage, __thiscall(ECX = router; message, a, b) RET 0Ch.
    virtual void session_route_message_0077c2a0(void* router, void* message, std::uint32_t a,
                                                std::uint32_t b) = 0;
    // 0095B9C0 BSP_VehicleClass_GetRegistryId, __thiscall(ECX = class) -> EAX.
    virtual std::uint32_t vehicle_class_registry_id_0095b9c0(void* vehicle_class) = 0;
    // 0071BE40 BSP_WeaponDirector_CurrentCommand, __thiscall(ECX = command list) -> EAX, the
    // command class's image address (compared against 00E08F68, moveto).
    virtual std::uint32_t current_command_0071be40(void* commands) = 0;
    // 0071EB60, __thiscall(ECX = command list; buffer) RET 4 -> EAX = target descriptor.
    virtual void* active_target_descriptor_0071eb60(void* commands, void* buffer) = 0;
    // 006F7DD0, __thiscall(ECX = target descriptor) -> EAX = const float[3] position.
    virtual const float* target_descriptor_position_006f7dd0(void* descriptor) = 0;
    // 0042B2F0, __thiscall(ECX = const float[3]) -> ST0: the vector's length.
    virtual float vector3_length_0042b2f0(const float* vector) = 0;
    // 007EE5C0 BSP_PlaneSquadron_AirborneCount, __thiscall(ECX = squadron) -> EAX.
    virtual std::int32_t squadron_airborne_count_007ee5c0(void* squadron) = 0;
    // 00653400 / 00653390, __thiscall(ECX = member): the observer member destructors.
    virtual void observer_ref_destruct_00653400(void* member) = 0;
    virtual void observer_member_cleanup_00653390(void* member) = 0;
    // 00438AA0 BSP_Math_AddWrappedAngle, stdcall(float a, float b) RET 8 -> ST0.
    virtual float add_wrapped_angle_00438aa0(float a, float b) = 0;
    // 00BF7030 _CIsqrt / 00BF701A _CIatan2(y = ST1, x = ST0): CRT, bound.
    virtual double sqrt_00bf7030(double value) = 0;
    virtual double atan2_00bf701a(double y, double x) = 0;
    // 00467310 BSP_FormatToBuffer32, cdecl(char buffer[20h], const char* format, ...).
    virtual void format_to_buffer32_00467310(char* buffer, const char* format,
                                             std::int32_t value) = 0;
    // 0095BA60 BSP_VehicleClass_MarkPartyRequired, __fastcall(ECX = class, EDX = party).
    virtual void vehicle_class_mark_party_required_0095ba60(void* vehicle_class,
                                                            std::uint32_t party) = 0;
    // 0046BE90 BSP_SceneEntity_RegisterHiddenStockUnit, __thiscall(ECX = [00E18680]; class)
    // RET 4.
    virtual void register_hidden_stock_unit_0046be90(void* registry, void* vehicle_class) = 0;
    // 006BA7B0 (STL, bound): __thiscall(ECX = vector object; index) RET 4 -> &element.
    virtual void* landing_vector_at_006ba7b0(void* vector, std::int32_t index) = 0;
};

inline constexpr std::uint32_t kMoveToCommandClass = 0x00E08F68;      // 006BC602
inline constexpr double kHoldRangeScale = 1.2000000476837158;         // 00CEC160, double
inline constexpr std::size_t kAirOpsSlotStride = 0x58;
inline constexpr std::size_t kLandingRecordStride = 0x14;
inline constexpr std::uint32_t kAirOpsSlotRecordVtable = 0x00CF6564;  // 006BC6AD
inline constexpr std::uint32_t kDoLaunchSlotVtable = 0x00CF85F0;      // MT 89h
inline constexpr std::uint32_t kSlotStateMessageVtable = 0x00CF64C4;  // MT 82h, 006BD306
inline constexpr std::uint32_t kSetSlotVtable = 0x00CF8624;           // MT 81h
inline constexpr std::uint32_t kStockItemVtable = 0x00CF8638;         // MT 85h
inline constexpr std::uint32_t kAppearReadyPlaneVtable = 0x00CF864C;  // MT 86h
inline constexpr double kHalf = 0.5;                                  // 00D7A280, double
inline constexpr double kMotherShipTouchdownOffset = 10.0;            // 00CE3DC0, double
inline constexpr double kAirfieldTouchdownOffset = 35.0;              // 00CF8608, double
inline constexpr float kTouchdownHeight = 0.5f;                       // 00CE3800, float
inline constexpr float kTouchdownZCeiling = -20.0f;                   // 00CF180C, float
inline constexpr double kPolarEpsilon = 1e-10;                        // 00CE3820, double
inline constexpr double kHalfPi = 1.5707963705062866;                 // 00CE3830, (double)(float)
inline constexpr double kTwoPi = 6.2831854820251465;                  // 00CE3828, (double)(float)
// The squadron entity handle registry: ids below [00F89A10] index bank 0 (base [00F89A0C],
// 10h-byte rows at [00F89A54]), the rest bank 1 (base [00F89A60], rows at [00F89AA8]); the
// entity is row +0Ch.
inline constexpr std::uint32_t kHandleSplit = 0x00F89A10;
inline constexpr std::uint32_t kHandleBase0 = 0x00F89A0C;
inline constexpr std::uint32_t kHandleRows0 = 0x00F89A54;
inline constexpr std::uint32_t kHandleBase1 = 0x00F89A60;
inline constexpr std::uint32_t kHandleRows1 = 0x00F89AA8;
// 006BC890's fallback point when the block has no traffic object (zero-filled .bss).
inline constexpr std::uint32_t kRunwayEndFallback = 0x00F87574;
inline constexpr std::uint32_t kHiddenStockRegistry = 0x00E18680;

// 006BA670 BSP_AirOpsBlock_ForwardToTraffic08, __thiscall(ECX = block; float dt) RET 4.
void air_ops_block_tick_006ba670(void* block, float dt, Seg29UHost& host);
// 006BC030 BSP_Entity_VKindIs1Or2, __thiscall(ECX = entity) -> EAX 1 / 0.
std::int32_t entity_kind_is_1_or_2_006bc030(void* entity, Seg29UHost& host);
// 006BC120, __thiscall(ECX = block) -> EAX: 1 when [block+7Ch] is null or its +5Dh is set.
std::int32_t air_ops_block_owner_missing_006bc120(void* block) noexcept;
// 006BC3E0, __thiscall(ECX = message) -> EAX = message: the 89h DOLAUNCHSLOT reader.
void* do_launch_slot_message_construct_006bc3e0(void* message, Seg29UHost& host);
// 006BC5E0, __thiscall(ECX = squadron) -> AL.
bool squadron_target_within_range_006bc5e0(void* squadron, Seg29UHost& host);
// 006BC690, __thiscall(ECX = slot record) RET.
void air_ops_slot_record_destruct_006bc690(void* record, Seg29UHost& host);
// 006BC6F0, __thiscall(ECX = slot; class, count) RET 8.
void air_ops_assign_slot_class_006bc6f0(void* slot, void* vehicle_class, std::int32_t count) noexcept;
// 006BC730, __thiscall(ECX = slot) RET.
void air_ops_slot_refresh_airborne_006bc730(void* slot, Seg29UHost& host);
// 006BC800, __thiscall(ECX = slot) -> AL.
bool air_ops_slot_squadron_moving_out_of_range_006bc800(void* slot, Seg29UHost& host);
// 006BC870, __thiscall(ECX = owner) RET (tail-jumps to 00695870).
void buffered_callback_owner_destruct_006bc870(void* owner, Seg29UHost& host);
// 006BC890, __thiscall(ECX = block; float out[3]) RET 4 -> EAX = out.
float* air_ops_block_runway_end_point_006bc890(void* block, float* out, Seg29UHost& host);
// 006BC8E0, __thiscall(ECX = block; slot) RET 4.
void air_ops_route_do_launch_slot_006bc8e0(void* block, std::uint32_t slot, Seg29UHost& host);
// 006BC960, __thiscall(ECX = holder) RET: T = holder+A4h..+ACh.
void air_ops_holder_refresh_touchdown_006bc960(void* holder, Seg29UHost& host);
// 006BCA40, __thiscall(ECX = holder; float out[3]) RET 4 -> EAX = out.
float* air_ops_holder_touchdown_world_006bca40(void* holder, float* out, Seg29UHost& host);
// 006BCA80, __thiscall(ECX = holder; float out[4], float t) RET 8.
void air_ops_holder_path_point_006bca80(void* holder, float* out, float t, Seg29UHost& host);
// 006BCCE0, __thiscall(ECX = holder; float out[3], const float point[3]) RET 8 -> EAX = out.
float* air_ops_holder_to_local_006bcce0(void* holder, float* out, void* point, Seg29UHost& host);
// 006BCD80 / 006BCE60, __thiscall(ECX = block; plane) RET 4.
void air_ops_block_hold_landing_record_006bcd80(void* block, void* plane, Seg29UHost& host);
void air_ops_block_release_landing_record_006bce60(void* block, void* plane, Seg29UHost& host);
// 006BCF40, __thiscall(ECX = scene property bag) RET.
void air_ops_register_party_plane_stock_006bcf40(void* bag, Seg29UHost& host);
// 006BCFD0, __thiscall(ECX = block; index, out) RET 8 -> AL.
bool air_ops_block_read_queue_record_006bcfd0(void* block, std::int32_t index, void* out,
                                             Seg29UHost& host);
// 006BD080, __thiscall(ECX = block; plane, out) RET 8 -> AL.
bool air_ops_find_landing_assignment_006bd080(void* block, void* plane, void* out,
                                             Seg29UHost& host);
// 006BD230, __thiscall(ECX = block; slot, command) RET 8.
void air_ops_slot_state_from_command_006bd230(void* block, std::uint32_t slot,
                                             std::int32_t command, Seg29UHost& host);
// 006BD620, __thiscall(ECX = record) -> EAX: the live squadron its +34h handle names, or null
// (then +24h = 5 and the handle is cleared).
void* air_ops_slot_check_squadron_handle_006bd620(void* record, Seg29UHost& host);
// 006BD800, __thiscall(ECX = message; block, slot) RET 8 -> EAX = message (MT 81h).
void* set_slot_message_construct_006bd800(void* message, void* block, std::uint32_t slot,
                                          Seg29UHost& host);
// 006BD9E0, __thiscall(ECX = message; class, value) RET 8 -> EAX = message (MT 85h).
void* stock_item_message_construct_006bd9e0(void* message, void* vehicle_class,
                                            std::uint32_t value, Seg29UHost& host);
// 006BDAC0, __thiscall(ECX = message; plane) RET 4 -> EAX = message (MT 86h).
void* appear_ready_plane_message_construct_006bdac0(void* message, void* plane, Seg29UHost& host);
// 006BDBF0, __thiscall(ECX = message) -> EAX: the entity its +20h handle names.
void* message_resolve_unit_006bdbf0(void* message, Seg29UHost& host);

// 006AC370, __thiscall(ECX = landing point; const float* ship_point) RET 4. For each troop path
// record in the list at +1FCh: refresh the path entity's world matrix (+CCh, latch +C8h), take
// its inverse, transform the ship point into path space, hand it to the path object at entity
// +1E4h (vtable +04h with 0), derive knots unless +1FCh is set, then 0048CA60(record, 1) on
// [game+21D0h].
void landing_point_update_paths_006ac370(void* point, void* ship_point, Seg29UHost& host);

// 006ACB40 BSP_LandingPoint_Destruct, __fastcall(ECX = landing point) RET.
void landing_point_destruct_006acb40(void* point, Seg29UHost& host);

// 006ACDC0, LandingPoint vtable slot A4h (00CE9184), __thiscall(ECX = landing point) RET:
// reads the scene properties Paths1..Paths3 into 18h-byte troop path records.
void landing_point_read_paths_006acdc0(void* point, Seg29UHost& host);

}  // namespace bsp::cc13_seg29U

#endif  // BSP_CC13_SEG29U_HPP
