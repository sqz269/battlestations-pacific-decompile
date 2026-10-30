// cc13 leftover sweep C (packet cc13-sweepC): segments 30, 50, 54, 58, 60-65, 69-71, 73-75, 77,
// 79-83, 86, 87, 89-91.
//
// The functions here are the leftovers of earlier cc13 waves that no implementation in src/
// covers. Routines that an existing host already carries are recorded ledger-only (see
// docs/CC13_SWEEPC.md) and are not reconstructed again here.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as in
// the image) and reaches the engine through SweepCHost, whose methods name the image routine or
// virtual slot they stand for and give the register inputs. Calls between routines that are
// reconstructed here use the C++ directly. Constants carry their image address and width.
// New C++ interfaces, not drop-in binary replacements; nothing here is bound into bsp_game.
// Names are hypotheses, not recovered symbols.

#ifndef BSP_CC13_SWEEPC_HPP
#define BSP_CC13_SWEEPC_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_sweepC {

template <class T>
inline T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
inline void* ptr(void* base, std::size_t off) noexcept { return at<void*>(base, off); }
inline void* add(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) + off;
}

// The archive's tagged by-value argument {kind, value}, two dwords on the stack.
struct ArchiveArg {
    std::uint32_t kind;
    std::uint32_t value;
};

// ---------------------------------------------------------------------------
// Image constants.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kSlotStride = 0x58;                  // air-ops slot record
inline constexpr float kSlotCooldownSeconds = 5.0f;               // 00CE3850, float
inline constexpr double kSiteStampOffset = 99999.0;               // 00CF89D0, double
inline constexpr std::uint32_t kFollowObserverVtable = 0x00CF8900;  // 006CE8CF
inline constexpr std::uint32_t kAirOpsSiteVtable = 0x00CF8948;    // 006CF11A
inline constexpr std::uint32_t kSiteObserverMemberVtable = 0x00CF89B4;  // 006CF130
inline constexpr std::uint32_t kSiteOccupantVectorVtable = 0x00CF899C;  // 006CF14C
inline constexpr std::uint32_t kAirbaseOrdersMessageVtable = 0x00CF8660;  // 006BF6DC
inline constexpr std::uint32_t kAirbaseOrdersMessageKind = 0x88;  // 006BF695
inline constexpr std::uint32_t kFixedStepClock = 0x00F876A4;      // float, the fixed-step clock

// ---------------------------------------------------------------------------
// Engine boundary. Every call the reconstructed bodies make goes through here, at the point the
// image makes it.
// ---------------------------------------------------------------------------
struct SweepCHost {
    virtual ~SweepCHost() = default;

    // [00CE2218] EnterCriticalSection / [00CE2210] LeaveCriticalSection. The engine's tracked
    // lock adds the +18h recursion counter around them; the bodies here do that part.
    virtual void enter_critical_section(void* section) = 0;
    virtual void leave_critical_section(void* section) = 0;
    // 00BF6713, the checked-iterator failure report (debug STL range checks).
    virtual void invalid_iterator_00bf6713() = 0;
    // 00BF55BE operator new / 00BF6989 operator delete.
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void operator_delete_00bf6989(void* block) = 0;
    // [00F876A4], the fixed-step clock (float).
    virtual float fixed_step_clock_00f876a4() = 0;

    // Observer pairs: 00694A60 registers, 006952A0 unregisters, both __fastcall(ECX = subject,
    // EDX = member).
    virtual void observer_register_pair_00694a60(void* subject, void* member) = 0;
    virtual void observer_unregister_pair_006952a0(void* subject, void* member) = 0;
    // 00696350, __fastcall(ECX = object, EDX = slot index, [esp+4] = argument).
    virtual void observer_notify_slot_00696350(void* object, int edx_slot, std::uint32_t arg) = 0;

    // STL list internals (vendor instantiations, not reconstructed): 006BFBD0 buys a node,
    // __thiscall(ECX = list, next, prev, const value*) -> node; 006C89D0 checks and grows the
    // size, __thiscall(ECX = list, count); 006C48F0 is list::remove, __thiscall(ECX = list,
    // const value*).
    virtual void* list_buy_node_006bfbd0(void* list, void* next, void* prev, const void* value) = 0;
    virtual void list_incsize_006c89d0(void* list, std::uint32_t count) = 0;
    virtual void list_remove_006c48f0(void* list, const void* value) = 0;

    // 006BD520, __thiscall(ECX = message storage, block, slot) -> the message: the air-ops slot
    // message for one slot.
    virtual void* landing_slot_message_construct_006bd520(void* message, void* block, int slot) = 0;
    // 0077C7B0, __thiscall(ECX = session owner, message, peer), RET 8: broadcast to every peer.
    virtual void session_owner_broadcast_0077c7b0(void* owner, void* message, std::uint32_t peer) = 0;
    // 0075B430, __thiscall(ECX = message, kind), RET 4: session message base constructor.
    virtual void session_message_construct_base_0075b430(void* message, std::uint32_t kind) = 0;
    // 0077C2A0, __thiscall(ECX = owner, message, channel, peer), RET 0Ch: route a message.
    virtual void session_route_message_0077c2a0(void* owner, void* message, std::uint32_t channel,
                                                std::uint32_t peer) = 0;
    // 006CA770, __thiscall(ECX = block, class, count), RET 8: add stock.
    virtual void air_ops_add_stock_006ca770(void* block, void* vehicle_class, std::int32_t count) = 0;

    // Archive virtual slots: +0Ch writes a tagged value under a key, +10h reads one.
    // __thiscall(ECX = archive, ArchiveArg key, ArchiveArg value), RET 10h.
    virtual void archive_write_vslot_0c(void* archive, ArchiveArg key, ArchiveArg value) = 0;
    virtual void archive_read_vslot_10(void* archive, ArchiveArg key, ArchiveArg value) = 0;
    // 006CDD70, __thiscall(ECX = observer member): BSP_FollowObserverMember_Destroy.
    virtual void follow_observer_member_destroy_006cdd70(void* member) = 0;

    // A dword of image data (a global the routine reads by literal address).
    virtual std::uint32_t image_dword(std::uint32_t address) = 0;
    // 006CCDA0, __thiscall(ECX = block, slot, order, target, player, apply), RET 14h.
    virtual std::int32_t air_ops_slot_command_006ccda0(void* block, std::int32_t slot,
                                                       std::int32_t order, void* target,
                                                       std::int32_t player, std::uint32_t apply) = 0;
    // The 006CD6C0 case targets, each __thiscall(ECX = block): 006C47F0(message) RET 4,
    // 006CA8E0(slot, byte) RET 8, 006C83B0(message) RET 4, 006C7490(slot, 0) RET 8.
    virtual void air_ops_apply_slot_setup_006c47f0(void* block, void* message) = 0;
    virtual void air_ops_launch_slot_006ca8e0(void* block, std::int32_t slot, std::uint8_t flag) = 0;
    virtual void air_ops_apply_slot_record_006c83b0(void* block, void* message) = 0;
    virtual void air_ops_launch_start_006c7490(void* block, std::int32_t slot, std::uint32_t arg) = 0;
    // 006BDBF0, __thiscall(ECX = message) -> the unit its handle names.
    virtual void* message_resolve_unit_006bdbf0(void* message) = 0;
    // [object]->vtable[14h](ECX = object, one stack dword).
    virtual void object_vslot_14(void* object, void* arg) = 0;
    // 007B8A80, __thiscall(ECX = class id) -> the vehicle class record.
    virtual void* vehicle_class_get_007b8a80(std::uint32_t class_id) = 0;
    // [site]->vtable[4Ch](ECX = site) -> an object whose +80h is compared with plane+BF4h.
    virtual void* site_vslot_4c(void* site) = 0;
    // 00758B90, __thiscall(ECX = message storage, flag, plane), RET 8 -> the message: the
    // mother-ship elevator message.
    virtual void* elevator_message_construct_00758b90(void* message, std::uint32_t flag,
                                                      void* plane) = 0;

    // 00BF681B, the second operator new (cdecl, one size argument).
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    // 00BD1860, cdecl, no argument -> a new tracked critical section.
    virtual void* critical_section_create_00bd1860() = 0;
    // Container head constructors used by 006CAC00, each __thiscall(ECX = container) -> the head
    // node pointer the caller stores at container+4: 004870F0 (+10h list), 00659350 (+40h list),
    // 006BFA20 (+B4h list), 006BFD60 (+ECh tree, returns the head node).
    virtual void* list_head_004870f0(void* container) = 0;
    virtual void* list_head_00659350(void* container) = 0;
    virtual void* list_head_006bfa20(void* container) = 0;
    virtual void* tree_head_006bfd60(void* container) = 0;
    // 006C4FA0, __thiscall(ECX = observer list object): its base constructor.
    virtual void observer_list_construct_006c4fa0(void* object) = 0;
    // 0095CC90, __thiscall(ECX = unit, flag, 0), RET 8: the unit game-object base constructor.
    virtual void unit_game_object_construct_0095cc90(void* unit, std::uint32_t flag,
                                                     std::uint32_t zero) = 0;
    // 004E6480, __thiscall(ECX = collision node): field initialisation.
    virtual void collision_node_init_004e6480(void* node) = 0;
    // 009553D0, __thiscall(ECX = unit, descriptor), RET 4: set the vehicle class.
    virtual void unit_set_vehicle_class_009553d0(void* unit, void* descriptor) = 0;
    // 00923BE0, __thiscall(ECX = unit) -> ST0: the unit's health fraction.
    virtual float unit_health_00923be0(void* unit) = 0;
    // 006BA670, __thiscall(ECX = block, float dt), RET 4: the air-ops block tick.
    virtual void air_ops_block_tick_006ba670(void* block, float dt) = 0;
    // 00779C60, __thiscall(ECX = unit, float), RET 4.
    virtual void unit_set_float_00779c60(void* unit, float value) = 0;
    // 00878340, __thiscall(ECX = unit): the land-unit reset base.
    virtual void land_unit_reset_00878340(void* unit) = 0;
    // [object]->vtable[1B8h](ECX = object).
    virtual void object_vslot_1b8(void* object) = 0;
    // 006D2980, __thiscall(ECX = airfield): rebuilds the hangar aim-point list.
    virtual void airfield_rebuild_aim_points_006d2980(void* airfield) = 0;
    // 00928570, __thiscall(ECX = unit); 004837D0, __thiscall(ECX = list, node), RET 4.
    virtual void unit_detach_00928570(void* unit) = 0;
    virtual void list_erase_node_004837d0(void* list, void* node) = 0;

    // --- segment 50 ---
    // 0092D730, __thiscall(ECX = motion controller) -> ST0: body-axis speed.
    // 0071BE40, __thiscall(ECX = weapon director) -> EAX: the current command.
    // Both are reached by tail jumps from 0080E0F0 / 0080E1B0.
    virtual float motion_body_axis_speed_0092d730(void* controller) = 0;
    virtual std::uint32_t director_current_command_0071be40(void* director) = 0;
    // [object]->vtable[5Ch](ECX = object, kind) -> AL: IsKindOf.
    virtual bool object_is_kind_of_vslot_5c(void* object, std::uint32_t kind) = 0;
    // 0085AD00, __thiscall(ECX = node).
    virtual void node_release_0085ad00(void* node) = 0;
    // 00939CB0, __thiscall(ECX = controller, unit), RET 4 -> controller: the motion controller
    // constructor.
    virtual void* motion_controller_construct_00939cb0(void* controller, void* unit) = 0;
    // [object]->vtable[18h](ECX = object, one stack dword).
    virtual void object_vslot_18(void* object, std::uint32_t arg) = 0;
    // 009565A0, __thiscall(ECX = unit, level), RET 4.
    virtual void unit_set_skill_level_009565a0(void* unit, std::uint32_t level) = 0;
    // 00815370, __thiscall(ECX = part, float), RET 4.
    virtual void part_set_controlled_00815370(void* part, float value) = 0;
    // 00810630, __thiscall(ECX = unit+BD0h, float distance, out a, out b, out c), RET 10h.
    virtual void wake_point_at_distance_00810630(void* wake, float distance, void* a, void* b,
                                                 void* c) = 0;
    // 00810E90, __thiscall(ECX = unit) -> EAX.
    virtual std::int32_t unit_torpedo_stock_part_00810e90(void* unit) = 0;
    // 0092BD30, __thiscall(ECX = motion controller); 00818970, __thiscall(ECX = ship);
    // 0095D400, __thiscall(ECX = unit) (the base on-killed slot, reached by a tail jump).
    virtual void motion_controller_on_killed_0092bd30(void* controller) = 0;
    virtual void ship_stop_wreck_effects_00818970(void* ship) = 0;
    virtual void unit_on_killed_base_0095d400(void* unit) = 0;

    // [object]->vtable[2Ch](ECX = object) -> EAX (the weapon director's slot 2Ch).
    virtual void* object_vslot_2c(void* object) = 0;
    // 00414DB0, __thiscall(ECX = entity): refresh the world pose.
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 006F2C30, __fastcall(ECX = position, EDX = party, kind, out*), RET 8 -> EAX: the nearest
    // landing site of the party.
    virtual void* nearest_landing_site_006f2c30(void* position, std::uint32_t party,
                                                std::uint32_t kind, void* out) = 0;
    // 00782790, __thiscall(ECX = record, tick), RET 4; [object]->vtable[18Ch](ECX = object, arg).
    virtual void gun_sync_record_stamp_00782790(void* record, std::uint32_t tick) = 0;
    virtual void object_vslot_18c(void* object, void* arg) = 0;
    // [object]->vtable[50h](ECX = object) -> ST0; [object]->vtable[0](ECX = object) -> EAX.
    virtual float object_vslot_50_float(void* object) = 0;
    virtual std::uint32_t object_vslot_00_noarg(void* object) = 0;
    // 0092D700 / 0092D6A0, __thiscall(ECX = motion controller, scratch*), RET 4 -> a float3*.
    virtual const float* motion_angular_rate_0092d700(void* controller, void* scratch) = 0;
    virtual const float* motion_velocity_0092d6a0(void* controller, void* scratch) = 0;
    // Session message stream halves: 0075B480 writes / 0075B4C0 reads the base header,
    // __thiscall(ECX = message, stream), RET 4.
    virtual void session_message_write_header_0075b480(void* message, void* stream) = 0;
    virtual void session_message_read_header_0075b4c0(void* message, void* stream) = 0;
    // Bit cursor: 004295C0 writes a quantized float, __thiscall(ECX = stream, float value,
    // a, b, float range, bits), RET 14h; 004293F0 reads one, __thiscall(ECX = stream+4,
    // float* out, a, b, float range, bits), RET 14h; 004290B0 writes a byte flag,
    // __thiscall(ECX = stream, byte), RET 4; 00428D70 reads one, __thiscall(ECX = stream+4,
    // byte* out), RET 4.
    virtual void bit_write_float_004295c0(void* stream, float value, std::uint32_t a,
                                          std::uint32_t b, float range, std::uint32_t bits) = 0;
    virtual void bit_read_float_004293f0(void* stream, float* out, std::uint32_t a,
                                         std::uint32_t b, float range, std::uint32_t bits) = 0;
    virtual void bit_write_bool_004290b0(void* stream, std::uint8_t value) = 0;
    virtual void bit_read_bool_00428d70(void* stream, std::uint8_t* out) = 0;

    // 00779BB0, __thiscall(ECX = unit, Lua object, key object), RET 8: the base property reader.
    virtual void unit_get_property_base_00779bb0(void* unit, void* lua, void* key) = 0;
    // 00B664B0, __thiscall(ECX = Lua object, int), RET 4: push an integer.
    virtual void lua_push_integer_00b664b0(void* lua, std::int32_t value) = 0;

    // --- segment 63 ---
    // [object]->vtable[1Ch] / [20h](ECX = object) -> ST0 (kept as double: the x87 value).
    virtual double object_vslot_1c_double(void* object) = 0;
    virtual double object_vslot_20_double(void* object) = 0;
    // A subscription filter: [filter]->vtable[0](ECX = filter, boxed parameter) -> AL.
    virtual bool filter_accepts_vslot_00(void* filter, std::uint32_t parameter) = 0;
    // 009658B0, __thiscall(ECX = vector, index), RET 4 -> &element (checked operator[]; a
    // vendor STL instantiation).
    virtual std::uint32_t* vector_at_009658b0(void* vector, std::uint32_t index) = 0;

    // [00CE2220] InterlockedDecrement(&counter) -> the new count.
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* counter) = 0;
    // [object]->vtable[0](ECX = object): the ref-counted object's destroy slot (no argument).
    virtual void refcounted_destroy_vslot_00(void* object) = 0;
    // 004B7EF0, __thiscall(ECX = by-value member copy): its destructor.
    virtual void member_destroy_004b7ef0(void* member) = 0;

    // --- segment 71 ---
    // 004C1C50, no stack argument -> EAX; 00A335D0, __thiscall(ECX = that), reached by a tail jump.
    virtual void* ai_globals_owner_004c1c50() = 0;
    virtual void ai_globals_load_00a335d0(void* owner) = 0;
    // 00888AA0, __thiscall(ECX = Lua object) -> the native entity; 00B67700, __thiscall(ECX =
    // Lua object): its destructor.
    virtual void* lua_object_to_entity_00888aa0(void* lua_object) = 0;
    virtual void lua_object_destroy_00b67700(void* lua_object) = 0;
    // 00A3EAC0, __thiscall(ECX = [00F8ABE8]) -> EAX; 00A3E510, __thiscall(ECX = [00F8ABE8]) -> AL.
    virtual std::uint32_t xlive_user_index_00a3eac0(void* xlive) = 0;
    virtual bool xlive_signed_in_00a3e510(void* xlive) = 0;
    // 00A4D5E4, __stdcall(user, a, b, out*) -> EAX (the XLive import thunk).
    virtual std::uint32_t xlive_check_privilege_00a4d5e4(std::uint32_t user, std::uint32_t a,
                                                          std::uint32_t b, std::uint32_t* out) = 0;
    // [object]->vtable[24h](ECX = object, lo, hi) -> AL.
    virtual bool object_vslot_24_xuid(void* object, std::uint32_t lo, std::uint32_t hi) = 0;
    // 00771630, __thiscall(ECX = game+1EF0h, lo, hi, dword), RET 0Ch.
    virtual void session_set_participant_policy_00771630(void* session, std::uint32_t lo,
                                                          std::uint32_t hi, std::uint32_t value) = 0;

    // --- segment 73 ---
    // Sound stream calls, each __thiscall(ECX = stream): 00A85C40(byte paused) RET 4;
    // 00A869B0(x, y) RET 8 -> EAX (the next y); 00A86710(const NativeString* name) RET 4;
    // 00A86F40() (reached by a tail jump); 00A77FF0(NativeString* out) RET 4 -> the name.
    virtual void sound_stream_set_paused_00a85c40(void* stream, std::uint8_t paused) = 0;
    virtual std::uint32_t sound_stream_draw_debug_00a869b0(void* stream, std::uint32_t x,
                                                           std::uint32_t y) = 0;
    virtual void sound_stream_select_row_00a86710(void* stream, const void* name) = 0;
    virtual void sound_stream_request_stop_00a86f40(void* stream) = 0;
    virtual const void* sound_stream_name_00a77ff0(void* stream, void* out_string) = 0;
    // 004254B0, cdecl(format, ...): the engine log.
    virtual void log_printf_004254b0(const char* format, const char* text) = 0;
    // 0041DD40, __thiscall(ECX = NativeString, length, keep), RET 8: resize.
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length,
                                               std::uint32_t keep) = 0;
    // The pooled string release every inline NativeString destructor makes: 00419CC0(data,
    // size, 1) answers the pool, then 00BD1510(ECX = pool) returns the block with the same
    // three stack arguments.
    virtual void string_pool_release_00419cc0_00bd1510(void* data, std::uint32_t size) = 0;
    // 00BF7C6E, the CRT `eh vector destructor iterator'(array, element size, count, dtor).
    virtual void eh_vector_destructor_iterator_00bf7c6e(void* array, std::uint32_t size,
                                                         std::uint32_t count,
                                                         std::uint32_t destructor) = 0;
    // 00BF65AC, operator delete (the scalar one); 00A780B0, __thiscall(ECX = manager): its
    // destructor body; 00A78EC0, __thiscall(ECX = records, count), RET 4: resize.
    virtual void operator_delete_00bf65ac(void* block) = 0;
    virtual void dialog_manager_destroy_00a780b0(void* manager) = 0;
    virtual void dialog_channel_records_resize_00a78ec0(void* records, std::uint32_t count) = 0;

    // --- segment 74 ---
    // 00A90E00, __thiscall(ECX = backend): the input backend base destructor (tail jump).
    virtual void input_backend_base_destroy_00a90e00(void* backend) = 0;
    // 00A92370, __thiscall(ECX = action); 00A91A50, __thiscall(ECX = control, float dt, a, b),
    // RET 0Ch.
    virtual void input_action_poll_00a92370(void* action) = 0;
    virtual void input_control_update_00a91a50(void* control, float dt, std::uint32_t a,
                                               std::uint32_t b) = 0;

    // --- segments 75, 77, 79 ---
    // A byte of image data, read and written (static guard bytes).
    virtual std::uint8_t image_byte(std::uint32_t address) = 0;
    virtual void set_image_byte(std::uint32_t address, std::uint8_t value) = 0;
    // 00AA8200, __thiscall(ECX = 00F8BC88): the GUI element type descriptor's own init;
    // 00BEA780, __thiscall(ECX = 0109DB84): the base type descriptor's init; 006FAC20, no
    // argument -> the type id counter object (+4h is the next id).
    virtual void gui_type_base_init_00aa8200(std::uint32_t descriptor) = 0;
    virtual void type_base_init_00bea780(std::uint32_t descriptor) = 0;
    virtual void* type_id_counter_006fac20() = 0;
    // 00AA75F0, __thiscall(ECX = pool 00F8BC94, block), RET 4; 00AA7A80, __thiscall(ECX = pool,
    // size), RET 4 -> block (reached by a tail jump).
    virtual void gui_widget_pool_free_00aa75f0(std::uint32_t pool, void* block) = 0;
    virtual void* gui_widget_pool_allocate_00aa7a80(std::uint32_t pool, std::uint32_t size) = 0;
    // 00B6F960, __thiscall(ECX = node, ...): reached by a tail jump from 00AA6810 (the node name
    // assignment); the stack argument passes through.
    virtual void node_assign_name_00b6f960(void* node, const void* name) = 0;

    // 00AB5C60, __thiscall(ECX = icon): the GUI icon base constructor.
    virtual void gui_icon_construct_00ab5c60(void* icon) = 0;
    // 00B64470, __fastcall(ECX = out matrix, EDX = translation, first, second), RET 8 -> out:
    // compose a matrix from the two stack vectors (GuiModel passes the rotation copy first and the
    // scale second) and the translation.
    virtual void* matrix_compose_00b64470(void* out, const float* translation, const float* first,
                                          const float* second) = 0;
    // [object]->vtable[38h](ECX = object, matrix*); [object]->vtable[88h](ECX = object);
    // [object]->vtable[80h](ECX = object, state).
    virtual void object_vslot_38(void* object, void* matrix) = 0;
    virtual void object_vslot_88(void* object) = 0;
    virtual void object_vslot_80(void* object, std::int32_t state) = 0;
    // 00AA7220, __thiscall(ECX = widget): the widget's position-changed refresh.
    virtual void gui_widget_position_changed_00aa7220(void* widget) = 0;
    // [00CE221C] InterlockedIncrement(&counter).
    virtual std::int32_t interlocked_increment_00ce221c(std::int32_t* counter) = 0;

    // --- segments 90, 91 ---
    // 00B17AF0, __thiscall(ECX = pool 0109022C, slot), RET 4: return an 88-byte pool slot.
    virtual void native_pool_return_00b17af0(std::uint32_t pool, void* slot) = 0;
    // Item array growth / clear, __thiscall(ECX = array, n), RET 4: 00B872F0 and 00B87350 grow
    // to n, 00B873C0 and 00B87410 resize to n (0 clears).
    virtual void resource_item_array_reserve_00b872f0(void* array, std::int32_t n) = 0;
    virtual void hierarchy_item_array_reserve_00b87350(void* array, std::int32_t n) = 0;
    virtual void resource_item_array_resize_00b873c0(void* array, std::int32_t n) = 0;
    virtual void hierarchy_item_array_resize_00b87410(void* array, std::int32_t n) = 0;
    // 00B887D0, __thiscall(ECX = dst string, const src*), RET 4: the resource name copy.
    virtual void resource_name_copy_00b887d0(void* dst, const void* src) = 0;
    // 00B89F20, __thiscall(ECX = new 3Ch block, resource), RET 4 -> instance.
    virtual void* resource_instance_construct_00b89f20(void* block, void* resource) = 0;
    // 00B71930, __fastcall(ECX = size) -> block; 00B71A80, __thiscall(ECX = block, arg), RET 4 ->
    // camera.
    virtual void* camera_pool_allocate_00b71930(std::uint32_t size) = 0;
    virtual void* camera_construct_00b71a80(void* block, std::uint32_t arg) = 0;

    // [object]->vtable[0](ECX = object, one stack dword), reached by a tail jump.
    virtual std::uint32_t object_vslot_00(void* object, std::uint32_t arg) = 0;
};

// ---------------------------------------------------------------------------
// Segment 30: the air-ops block and its landing sites.
// ---------------------------------------------------------------------------

// 006BF150, __thiscall(ECX = block, slot), RET 4. Publishes one slot to the session.
void air_ops_publish_slot_006bf150(SweepCHost& host, void* block, int slot);
// 006BF5C0, __fastcall(ECX = block) -> EAX: -1, 0 or 1.
std::int32_t air_ops_block_status_006bf5c0(SweepCHost& host, void* block);
// 006BF650, __thiscall(ECX = block, bool first, bool second, int third), RET 0Ch.
void air_ops_block_set_orders_006bf650(SweepCHost& host, void* block, std::uint8_t first,
                                       std::uint8_t second, std::int32_t third);
// 006C77E0, __fastcall(ECX = block): drops the +A8h records whose plane is gone or inactive.
void air_ops_block_compact_records_006c77e0(SweepCHost& host, void* block);
// 006CA730, __thiscall(ECX = block, squadron), RET 4: push_back on the list at block+B4h.
void air_ops_block_push_b8_006ca730(SweepCHost& host, void* block, void* squadron);
// 006CC5C0, __thiscall(ECX = block, slot), RET 4: cancel a queued (state 2) slot.
void air_ops_cancel_queued_slot_006cc5c0(SweepCHost& host, void* block, int slot);
// 006CC760, __thiscall(ECX = block, entity), RET 4: queue a plane on the block+C0h container.
void air_ops_push_ready_plane_006cc760(SweepCHost& host, void* block, void* entity);
// 006CD160, __thiscall(ECX = block, message), RET 4: the 82h slot order message.
void air_ops_slot_order_message_006cd160(SweepCHost& host, void* block, void* message);
// 006CD6C0, __thiscall(ECX = block, message), RET 4 -> AL: the block's session message switch.
bool air_ops_handle_session_message_006cd6c0(SweepCHost& host, void* block, void* message);
// 006CEDD0, __fastcall(ECX = site): drops occupants that belong to another deck.
void air_ops_site_release_mismatched_occupants_006cedd0(SweepCHost& host, void* site);
// 006CFFF0, __thiscall(ECX = mother-ship site, flag, plane), RET 8: routes the elevator message.
void mothership_site_route_elevator_message_006cfff0(SweepCHost& host, void* site,
                                                     std::uint32_t flag, void* plane);
// 006CE8A0, __thiscall(ECX = dst, const src*), RET 4 -> dst. Observer member copy constructor.
void* follow_observer_member_copy_construct_006ce8a0(SweepCHost& host, void* dst, void* src);
// 006CE910, __thiscall(ECX = occupant vector, plane), RET 4: register and append.
void air_ops_site_push_occupant_006ce910(SweepCHost& host, void* occupants, void* plane);
// 006CEAD0, __thiscall(ECX = archive, ArchiveArg key, member copy by value (24 bytes)), RET 20h.
// The callee destroys the by-value copy; `member_copy` is that argument area.
void save_writer_write_observer_ref_006cead0(SweepCHost& host, void* archive, ArchiveArg key,
                                             void* member_copy);
// 006CF0A0, __fastcall(ECX = archive, EDX = observer member, ArchiveArg key), RET 8.
void save_reader_read_observer_ref_006cf0a0(SweepCHost& host, void* archive, void* member,
                                            ArchiveArg key);
// 006CF100, __thiscall(ECX = site) -> site. The landing-site base constructor.
void* air_ops_site_construct_006cf100(SweepCHost& host, void* site);
// 006CF3E0, __thiscall(ECX = site) -> the airfield's air-ops block (+44h owner +72Ch) or null.
void* air_ops_site_airfield_block_006cf3e0(void* site);
// 006CAC00, __thiscall(ECX = block, owner), RET 4 -> block. The air-ops block base constructor.
void* air_ops_block_construct_006cac00(SweepCHost& host, void* block, void* owner);
// 006D1C20, __thiscall(ECX = airfield, flag), RET 4 -> airfield. The MAirfield constructor.
void* airfield_construct_006d1c20(SweepCHost& host, void* airfield, std::uint32_t flag);
// 006D3110, __thiscall(ECX = descriptor, flag), RET 4 -> the new airfield (class 45h allocator).
void* airfield_allocate_006d3110(SweepCHost& host, void* descriptor, std::uint32_t flag);
// 006D2560, __thiscall(ECX = airfield) -> ST0: mean hangar health, 0.0 when +5Dh is set.
float airfield_health_fraction_006d2560(SweepCHost& host, void* airfield);
// 006D3200, __thiscall(ECX = airfield, float dt), RET 4: the airfield's update.
void airfield_update_006d3200(SweepCHost& host, void* airfield, float dt);
// 006D31A0, __thiscall(ECX = airfield): restore every hangar, then rebuild the aim points.
void airfield_restore_full_health_006d31a0(SweepCHost& host, void* airfield);
// 006D3620, __thiscall(ECX = unit): leaves the world's two early lists.
void unit_detach_from_early_world_lists_006d3620(SweepCHost& host, void* unit);
// 006D1D30, RET: answers 1 (the airfield's sensor category slot).
std::uint32_t airfield_sensor_category_006d1d30();
// 006D1DD0, __thiscall(ECX = unit) -> unit+310h, or null for a null unit.
void* unit_slot60_sub310_006d1dd0(void* unit);
// 006D1DF0 / 006D1E30, __thiscall(ECX = unit) -> [unit+360h]. Two identical vtable slots.
void* unit_model_pointer_360_006d1df0(void* unit);
void* unit_model_pointer_360_006d1e30(void* unit);
// 006D1EB0, __thiscall(ECX = unit, level): tail jump to [unit+38Ch]->vtable[0] with
// ECX = unit+38Ch; the stack argument passes through (the target, 007B8AE0 for a plane, RETs 4).
std::uint32_t unit_set_skill_level_006d1eb0(SweepCHost& host, void* unit, std::uint32_t level);

// ---------------------------------------------------------------------------
// Segment 50: unit-instance helpers and the ship session messages.
// ---------------------------------------------------------------------------

// 0080DC10, __thiscall(ECX = 6Fh message, unit), RET 4: six dwords +20h..+34h to unit+1134h..
void ship_repair_settings_message_apply_0080dc10(void* message, void* unit);
// 0080DEC0, __thiscall(ECX = unit): unit+1018h = a new 390h-byte motion controller.
void unit_create_motion_controller_0080dec0(SweepCHost& host, void* unit);
// 0080DF80: MOV EAX,6 / RET.
std::uint32_t unit_return_six_0080df80();
// 0080DF90, __thiscall(ECX = unit, level), RET 4.
void unit_skill_set_level_0080df90(SweepCHost& host, void* unit, std::uint32_t level);
// 0080E0F0, __thiscall(ECX = unit) -> ST0: tail jump to 0092D730 on unit+1018h.
float unit_body_axis_speed_0080e0f0(SweepCHost& host, void* unit);
// 0080E130, __thiscall(ECX = unit, category), RET 4 -> unit+1134h[category].
std::uint32_t unit_repair_level_0080e130(void* unit, std::int32_t category);
// 0080E150, __thiscall(ECX = unit) -> [unit+738h], the weapon director.
void* unit_weapon_director_0080e150(void* unit);
// 0080E1B0, __thiscall(ECX = unit): tail jump to 0071BE40 on [unit+738h].
std::uint32_t unit_current_weapon_command_0080e1b0(SweepCHost& host, void* unit);
// 0080E290, __thiscall(ECX = unit): 0085AD00 on every child node answering IsKindOf(22h).
void unit_aim_turning_guns_to_rest_0080e290(SweepCHost& host, void* unit);
// 0080F960, __thiscall(ECX = message, byte), RET 4 -> message: the 7Ah message.
void* unit_message_shipyard_launch_construct_0080f960(SweepCHost& host, void* message,
                                                       std::uint8_t state);
// 0080F9F0, __thiscall(ECX = vehicle class) -> EAX: 1 when either breakup vector is non-empty.
std::uint32_t vehicle_class_has_breakup_pieces_0080f9f0(void* vehicle_class);
// 0080FA50, __thiscall(ECX = message, channel, float amount, byte), RET 0Ch: the 9Eh message.
void* unit_message_fire_or_water_damage_construct_0080fa50(SweepCHost& host, void* message,
                                                           std::int32_t channel, float amount,
                                                           std::uint8_t flag);
// 0080FE10, __thiscall(ECX = message, int), RET 4: the 96h message.
void* unit_message_torpedo_stock_construct_0080fe10(SweepCHost& host, void* message,
                                                    std::int32_t value);
// 0080FEC0, __thiscall(ECX = message, unit), RET 4: the 6Fh message from unit+1134h..
void* unit_message_repair_settings_construct_0080fec0(SweepCHost& host, void* message, void* unit);
// 0080FF30, __thiscall(ECX = message, entity, const float3*), RET 8: the 70h message.
void* unit_message_kamikaze_detonate_construct_0080ff30(SweepCHost& host, void* message,
                                                        void* entity, const float* point);
// 0080FF80, __thiscall(ECX = message, int, const float3*), RET 8: the 90h message.
void* unit_message_hull_impact_construct_0080ff80(SweepCHost& host, void* message,
                                                  std::uint32_t value, const float* point);
// 0080FFD0, __thiscall(ECX = message, a, b, c), RET 0Ch: the 93h message.
void* unit_message_add_hull_torque_construct_0080ffd0(SweepCHost& host, void* message,
                                                      std::uint32_t a, std::uint32_t b,
                                                      std::uint32_t c);
// 00811150, __thiscall(ECX = unit, float, a, b, c), RET 10h: 00810630 on unit+BD0h.
void unit_wake_point_at_distance_00811150(SweepCHost& host, void* unit, float distance, void* a,
                                          void* b, void* c);
// 00812960, __fastcall(ECX = unit) -> EAX: the director's slot-2Ch object when it answers
// IsKindOf(1Ch), else the party's nearest landing site (006F2C30).
void* unit_landing_target_00812960(SweepCHost& host, void* unit);
// 00813830, __thiscall(ECX = unit): routes a 7Ah message with state 0 (0077C2A0(msg, 0, 0)).
void unit_route_shipyard_launch_00813830(SweepCHost& host, void* unit);
// 00813950, __thiscall(ECX = unit, message), RET 4: applies the 8Dh gun sync records.
void unit_apply_gun_sync_00813950(SweepCHost& host, void* unit, void* message);
// 00813CA0, __thiscall(ECX = message, unit, three unused dwords), RET 10h: builds the 8Ch
// MT_SHIP_SYNC message.
void* unit_state_message_build_00813ca0(SweepCHost& host, void* message, void* unit);
// 00813DC0 / 00813EF0, __thiscall(ECX = message, stream), RET 4: serialize / deserialize.
void unit_state_message_serialize_00813dc0(SweepCHost& host, void* message, void* stream);
void unit_state_message_deserialize_00813ef0(SweepCHost& host, void* message, void* stream);
// 008144A0, __thiscall(ECX = unit): unless the session mode is 2, routes the 91h message.
void unit_route_message_91_008144a0(SweepCHost& host, void* unit);
// 00815010, __thiscall(ECX = entity) -> the entity when it answers IsKindOf(6), else null.
void* entity_narrow_to_class6_00815010(SweepCHost& host, void* entity);
// 00815850, __thiscall(ECX = unit) -> EAX: torpedo stock total.
std::int32_t unit_torpedo_stock_total_00815850(SweepCHost& host, void* unit);
// 00815870, __thiscall(ECX = ship, Lua object, key object), RET 8: the ship's GetProperty slot.
void ship_get_property_00815870(SweepCHost& host, void* ship, void* lua, void* key);
// 00817380, __thiscall(ECX = unit, byte flag), RET 4: per-part controlled flag broadcast.
void unit_broadcast_controlled_part_flag_00817380(SweepCHost& host, void* unit, std::uint8_t flag);
// 00818300, __thiscall(ECX = unit) -> EAX: sum of +448h over kind-7 gun children.
std::int32_t unit_torpedo_barrel_count_00818300(SweepCHost& host, void* unit);
// 00819880, __thiscall(ECX = ship): the ship's on-killed slot (vtable +84h).
void ship_on_killed_00819880(SweepCHost& host, void* ship);

// ---------------------------------------------------------------------------
// Segment 63: the mission event channel helpers and subscription matchers.
// ---------------------------------------------------------------------------

// 00967FB0 / 00967FF0, __thiscall(ECX = object) -> EAX: 1 when double(+10h) >
// clock - vtable[1Ch]() (resp. [20h]()). No caller or vtable slot holds either.
std::uint32_t timer_not_elapsed_vslot_1c_00967fb0(SweepCHost& host, void* object);
std::uint32_t timer_not_elapsed_vslot_20_00967ff0(SweepCHost& host, void* object);
// ---------------------------------------------------------------------------
// Segment 64.
// ---------------------------------------------------------------------------
// 0096A8D0, __thiscall(ECX = archive, key (2 dwords), member by value (6 dwords)), RET 20h.
void save_writer_write_member_ref_0096a8d0(SweepCHost& host, void* archive, ArchiveArg key,
                                           void* member_copy);
// The intrusive ref-pointer reset at one offset, __thiscall(ECX = owner): 0096C4A0 (+8h),
// 0096C4D0 (+0Ch), 0096CD00 (+18h), 0096EC90 (+18h).
void refptr_reset_08_0096c4a0(SweepCHost& host, void* owner);
void refptr_reset_0c_0096c4d0(SweepCHost& host, void* owner);
void refptr_reset_18_0096cd00(SweepCHost& host, void* owner);
void refptr_reset_18_0096ec90(SweepCHost& host, void* owner);
// 0096CE00, __thiscall(ECX = object) -> AL: 1 when the list at +28h holds the current player's
// (+170h)->vtable[0]() value.
bool list_holds_current_player_key_0096ce00(SweepCHost& host, void* object);
// 00968060, __thiscall(ECX = box, entity), RET 4 -> box: the boxed entity id (vtable 00D1AF24).
void* hit_channel_entity_key_construct_00968060(void* box, void* entity);
// 009681D0, RET 4: entity usable (+5Ch set; +5Dh, +60h, +5Eh clear). ECX is not read.
std::uint32_t unit_is_active_for_hit_channel_009681d0(void* entity);
// 00968230, __thiscall(ECX = message, byte, byte), RET 8: the E5h session message.
void* session_message_e5_construct_00968230(SweepCHost& host, void* message, std::uint8_t a,
                                            std::uint8_t b);
// 00968260, __thiscall(ECX = message, dword), RET 4: the E6h session message.
void* session_message_e6_construct_00968260(SweepCHost& host, void* message, std::uint32_t value);
// 009683A0, __thiscall(ECX = unit) -> EAX: the current player's unit (or its squadron) or a
// unit sharing its +284h owner. No caller or vtable slot holds it.
std::uint32_t unit_is_current_player_or_ally_009683a0(SweepCHost& host, void* unit);
// The subscription matchers (vtable slot 3, __thiscall(ECX = subscription, params vector),
// RET 4 -> EAX). Two-key: exit zone is hosted elsewhere; leak, fire, repair, target, ammoType,
// gui, generate. Three-key: failure, zone, stock. hit tests eight parameters.
std::uint32_t leak_subscription_matches_009688c0(SweepCHost& host, void* subscription, void* params);
std::uint32_t fire_subscription_matches_00968940(SweepCHost& host, void* subscription, void* params);
std::uint32_t repair_subscription_matches_009689c0(SweepCHost& host, void* subscription, void* params);
std::uint32_t target_subscription_matches_00968bd0(SweepCHost& host, void* subscription, void* params);
std::uint32_t ammo_type_subscription_matches_00968c50(SweepCHost& host, void* subscription, void* params);
std::uint32_t gui_subscription_matches_00968d80(SweepCHost& host, void* subscription, void* params);
std::uint32_t generate_subscription_matches_00968e00(SweepCHost& host, void* subscription, void* params);
std::uint32_t failure_subscription_matches_00968810(SweepCHost& host, void* subscription, void* params);
std::uint32_t zone_subscription_matches_00968a40(SweepCHost& host, void* subscription, void* params);
std::uint32_t stock_subscription_matches_00968cd0(SweepCHost& host, void* subscription, void* params);
std::uint32_t hit_subscription_matches_009685b0(SweepCHost& host, void* subscription, void* params);

// ---------------------------------------------------------------------------
// Segment 71.
// ---------------------------------------------------------------------------
// 00A371C0, no argument: 004C1C50() then a tail jump to 00A335D0 with ECX = its result.
void ai_globals_reload_00a371c0(SweepCHost& host);
// 00A37250, cdecl-shaped with one 14h-byte Lua object by value, RET 14h -> entity+16Ch.
std::uint32_t ai_group_from_lua_entity_00a37250(SweepCHost& host, void* lua_object_copy);
// 00A430E0, __thiscall(ECX = any, a, b), RET 8 -> AL. ECX only seeds the out dword.
bool xlive_query_identifier_policy_00a430e0(SweepCHost& host, std::uint32_t ecx, std::uint32_t a,
                                            std::uint32_t b);
// 00A43180, __thiscall(ECX = XLive client), RET.
void xlive_refresh_participant_policy_00a43180(SweepCHost& host, void* client);

// ---------------------------------------------------------------------------
// Segment 73: the streamed dialog and music manager accessors.
// ---------------------------------------------------------------------------
void dialog_channel_record_set_speaker_layout_00a776b0(SweepCHost& host, void* record,
                                                       std::int32_t layout);
std::uint32_t sound_stream_is_active_00a776d0(void* stream);
std::uint32_t sound_stream_is_over_00a776f0(void* stream);
void streamed_dialog_logical_set_byte_4e_00a77700(void* logical, std::uint8_t value);
std::uint8_t streamed_dialog_logical_get_byte_4e_00a77710(void* logical);
std::uint32_t streamed_dialog_logical_get_category_00a77720(void* logical);
std::uint32_t streamed_dialog_manager_is_dialog_playing_00a77730(void* manager);
void streamed_dialog_manager_set_all_paused_00a77760(SweepCHost& host, void* manager,
                                                     std::uint8_t paused);
void streamed_dialog_manager_draw_debug_00a777a0(SweepCHost& host, void* manager);
void streamed_dialog_manager_set_channel_byte_4e_00a77840(void* manager, std::int32_t channel,
                                                          std::uint8_t value);
void streamed_dialog_manager_set_target_level_00a77860(void* manager, float level);
void* streamed_dialog_refptr_assign_00a77b30(SweepCHost& host, void* slot, void* value);
void dialog_channel_record_release_ref_10_00a77b70(SweepCHost& host, void* record);
void streamed_dialog_physical_pair_array_delete_00a77ec0(SweepCHost& host, void* owner);
void* dialog_channel_record_construct_00a77f80(SweepCHost& host, void* record);
void music_director_select_row_and_store_name_00a782e0(SweepCHost& host, void* director,
                                                      const void* name);
void streamed_dialog_manager_end_dialog_stream_00a78620(SweepCHost& host, void* manager);
void streamed_dialog_logical_delete_00a78f80(SweepCHost& host, void* owner);
void dialog_channel_records_destroy_00a790f0(SweepCHost& host, void* records);

// ---------------------------------------------------------------------------
// Segment 74: input devices.
// ---------------------------------------------------------------------------
// 00A82E20 / 00A82E40, __thiscall(ECX = NativeString): the pooled release of its data.
void native_string_destroy_00a82e20(SweepCHost& host, void* string);
void native_string_destroy_00a82e40(SweepCHost& host, void* string);
// 00A92A20, __thiscall(ECX = action, float dt), RET 4.
void input_action_update_00a92a20(SweepCHost& host, void* action, float dt);
// 00A943D0 / 00A9A0B0, __thiscall(ECX = device) -> device: input device constructors.
void* input_device_construct_00a943d0(void* device);
void* mouse_device_construct_00a9a0b0(void* device);
// 00A97B30, __thiscall(ECX = backend): the native input backend destructor.
void native_input_backend_destroy_00a97b30(SweepCHost& host, void* backend);

// ---------------------------------------------------------------------------
// Segments 75, 77, 79: string-map pool, GUI widgets.
// ---------------------------------------------------------------------------
// 00A9E970, __thiscall(ECX = pool, value), RET 4: the 1024-entry free stack and node fill.
// This narrow entry writes fenced raw bytes: fresh slab backing needs no live
// C++ primitive field objects. Shared at<> and all other entries are unchanged.
void string_map_node_pool_init_block_00a9e970(void* pool, std::uint32_t value);
// 00AA6810, __thiscall(ECX = widget, name): tail jump to 00B6F960 on +4Ch.
void gui_widget_assign_node_name_00aa6810(SweepCHost& host, void* widget, const void* name);
// 00AA7880, __fastcall(ECX = block): 00AA75F0(ECX = 00F8BC94, block).
void gui_widget_pool_free_thunk_00aa7880(SweepCHost& host, void* block);
// 00AA7CF0, (size on the stack): tail jump to 00AA7A80 with ECX = 00F8BC94.
void* gui_widget_pool_allocate_stub_00aa7cf0(SweepCHost& host, std::uint32_t size);
// The type-descriptor initializers: 00AA9350 (cGuiElement, returns the descriptor), 00ACEA20 /
// 00ACEA70 (cGuiClipBox), 00AD0780 / 00AD07D0 (cGuiFrameBox); the *Return variants answer ECX.
void* gui_element_type_id_init_return_00aa9350(SweepCHost& host, void* descriptor);
void gui_clip_box_type_id_init_00acea20(SweepCHost& host, void* descriptor);
void* gui_clip_box_type_id_init_return_00acea70(SweepCHost& host, void* descriptor);
void gui_frame_box_type_id_init_00ad0780(SweepCHost& host, void* descriptor);
void* gui_frame_box_type_id_init_return_00ad07d0(SweepCHost& host, void* descriptor);
// 00ACC4A0, __thiscall(ECX = model): release the ref pointer +F0h, then +ECh = 0.
void gui_model_release_refs_00acc4a0(SweepCHost& host, void* model);
// 00ACBEC0, __thiscall(ECX = model, const float scale[3], const float euler[3]), RET 8.
void gui_model_set_transform_vectors_00acbec0(SweepCHost& host, void* model, const float* scale,
                                              const float* euler);
// 00ACF1D0, __thiscall(ECX = frame box, const float position[2]), RET 4.
void gui_frame_box_set_position_00acf1d0(SweepCHost& host, void* box, const float* position);
// 00AD0680, __thiscall(ECX = frame box, state, texture), RET 8.
void gui_frame_box_set_state_texture_00ad0680(SweepCHost& host, void* box, std::uint32_t state,
                                              void* texture);
// 00AD0D00, __thiscall(ECX = dst record, const src record*), RET 4 -> dst: the 2Ch-byte record
// assignment.
void* gui_record_2c_assign_00ad0d00(SweepCHost& host, void* dst, const void* src);
// 00AD2D50, __thiscall(ECX = icon) -> icon: the AnimIcon constructor.
void* gui_anim_icon_construct_00ad2d50(SweepCHost& host, void* icon);
// Segment 90: factories and resource items.
void* plain_node_factory_destruct_00b87170(void* factory);
void* model_factory_destruct_00b871a0(void* factory);
void* group_factory_destruct_00b871d0(void* factory);
void* plain_node_factory_scalar_delete_00b87180(SweepCHost& host, void* factory, std::uint32_t flags);
void* model_factory_scalar_delete_00b871b0(SweepCHost& host, void* factory, std::uint32_t flags);
void* group_factory_scalar_delete_00b871e0(SweepCHost& host, void* factory, std::uint32_t flags);
void return_native_88_byte_pool_slot_00b876f0(SweepCHost& host, void* slot);
void resource_item_array_push_back_00b87710(SweepCHost& host, void* array, const std::uint32_t* item);
void hierarchy_item_array_push_back_00b87750(SweepCHost& host, void* array, const std::uint32_t* item);
void resource_item_array_clear_00b877f0(SweepCHost& host, void* array);
void hierarchy_item_array_clear_00b87800(SweepCHost& host, void* array);
void* resource_node_copy_construct_00b88b00(SweepCHost& host, void* node, const void* src);
void* resource_node_construct_from_00b88cb0(SweepCHost& host, void* node, const std::uint32_t* key,
                                            const void* name);
void* resource_node_construct_00b88d00(SweepCHost& host, void* node, std::uint32_t a,
                                       std::uint32_t b, std::uint32_t c, const void* keyed_name,
                                       std::uint8_t flag);
void* resource_create_instance_00b89fe0(SweepCHost& host, void* resource);
void* camera_pool_create_camera_00b8a200(SweepCHost& host, std::uint32_t arg);
// Segment 91: 00B999A0, __thiscall(ECX = plane {n, d}, const float3*), RET 4 -> ST0.
float plane_signed_distance_00b999a0(const void* plane, const float* point);
// The pointer-table growth (data +0h, count +4h, capacity +8h), one shape at four addresses.
void pointer_table_grow_00aace40(SweepCHost& host, void* table);
void pointer_table_grow_00acbe50(SweepCHost& host, void* table);
void pointer_table_grow_00acd250(SweepCHost& host, void* table);
void pointer_table_grow_00acefc0(SweepCHost& host, void* table);

}  // namespace bsp::cc13_sweepC

#endif  // BSP_CC13_SWEEPC_HPP
