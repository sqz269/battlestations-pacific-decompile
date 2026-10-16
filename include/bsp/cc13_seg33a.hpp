// Segment 33, lower half (006DECA0..006EC000), reconstruction pass (packet cc13-seg33a).
//
// The gun bots, the battleship and cargo units, the bomb platforms and their session messages, the
// bomb / projectile / bullet entities and the bullet-class family (kamikaze, water mine, torpedo,
// depth charge, ...), and the catapult. docs/CC13_SEG33A.md carries the evidence and the status of
// every routine, including the ones recorded ledger-only because the cc9 gameplay hosts (or an
// earlier cc13/argon file) already implement them.
//
// Model: every routine works on the original object as raw bytes (Win32 layout, offsets as in the
// image) and reaches the engine through Seg33aHost, whose methods name the image routine or the
// virtual slot they stand for. New C++ interfaces, not drop-in binary replacements; nothing here is
// bound into bsp_game. Names are hypotheses, not recovered symbols. Marker: cc13_seg33a.

#ifndef BSP_CC13_SEG33A_HPP
#define BSP_CC13_SEG33A_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg33a {

template <class T>
inline T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
inline void* ptr(void* base, std::size_t off) noexcept { return at<void*>(base, off); }
inline void* add(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) + off;
}
inline void* sub(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) - off;
}

// The archive's tagged by-value argument (8 bytes on the stack): kind 0 = C string key, 2 = float
// (value bits when written, float* when read), 3 = byte (low byte when written), 5/6 = by-address
// aggregates (a vec3 / a two-float record), 4 = object reference.
struct ArchiveArg {
    std::uint32_t kind;
    std::uintptr_t value;
};
inline ArchiveArg key(const char* name) noexcept {
    return ArchiveArg{0u, reinterpret_cast<std::uintptr_t>(name)};
}

// ---------------------------------------------------------------------------
// Image constants.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kZeroVec3 = 0x00F87574;   // three zero floats (.bss, zero-fill)
inline constexpr float kOne_00d7a24c = 1.0f;             // 00D7A24C, float
inline constexpr const char* kBotSectionKey = "_bot";    // 00CF904C
inline constexpr const char* kErrorKey = "Error";        // 00CF9044
inline constexpr const char* kCalcErrTickKey = "calcErrTick";      // 00CF9038
inline constexpr const char* kDelayedFireKey = "delayedFire";      // 00CF902C
inline constexpr const char* kFireDelayTimeKey = "fireDelayTime";  // 00CF901C
inline constexpr const char* kErrorOffsetKey = "ErrorOffset";      // 00CF9010

// The engine's native string: size, then the heap buffer (pool block of size + 1).
struct NativeStr {
    std::uint32_t size;
    char* data;
};

// The entity handle tables the inlined id lookups read: ids below `split` index the low table,
// the rest the high table; entries are 10h bytes with the object at +0Ch.
struct HandleTables {
    std::int32_t low_base;      // [00F89A0C]
    std::int32_t split;         // [00F89A10]
    void* low_entries;          // [00F89A54]
    std::int32_t high_base;     // [00F89A60]
    void* high_entries;         // [00F89AA8]
};

// ---------------------------------------------------------------------------
// Engine boundary. Each method stands for one image routine or one virtual slot, named by
// address; the comment gives the original register ABI.
// ---------------------------------------------------------------------------
struct Seg33aHost {
    virtual ~Seg33aHost() = default;

    // Image data the routines read.
    virtual const float* zero_vec3_00f87574() = 0;   // three zero floats

    // 00927F10, __stdcall(slot), RET 4: the party slot is AI-held.
    virtual bool party_slot_is_ai_held_00927f10(std::int32_t slot) = 0;
    // 00414DB0, __thiscall(entity): refresh the cached world pose.
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 00521EA0, __fastcall(record /*ECX*/): resolve the command-target record to its object.
    virtual void* command_target_resolve_00521ea0(void* record) = 0;
    // 00694A60, __fastcall(subject /*ECX*/, member /*EDX*/): register an observer member.
    virtual void observer_register_pair_00694a60(void* subject, void* member) = 0;
    // 006952A0, __fastcall(subject /*ECX*/, member /*EDX*/): unregister an observer member.
    virtual void observer_unregister_pair_006952a0(void* subject, void* member) = 0;
    // 008FBC80, __thiscall(bot, unit), RET 4: the gun-bot base attach.
    virtual void gun_bot_attach_base_008fbc80(void* bot, void* unit) = 0;
    // 006DEFF0, __thiscall(bot): the artillery bot's aim-error refresh.
    virtual void artillery_bot_refresh_error_006deff0(void* bot) = 0;
    // 008FE660 / 008FE6D0, __thiscall(bot, archive), RET 4: the base property visitors.
    virtual void gun_bot_read_base_properties_008fe660(void* bot, void* archive) = 0;
    virtual void gun_bot_write_base_properties_008fe6d0(void* bot, void* archive) = 0;
    // 0072D5B0, __thiscall(gun, a, b), RET 8: select the gun's fire mode.
    virtual void gun_select_fire_mode_0072d5b0(void* gun, std::int32_t a, std::int32_t b) = 0;

    // 0081ED40, __thiscall(unit, arg), RET 4: the vehicle-unit base constructor.
    virtual void unit_vehicle_base_construct_0081ed40(void* unit, void* arg) = 0;
    // 00BF55BE, __cdecl(size): operator new. 00BF79F0, __cdecl(dst, byte, count): memset.
    virtual void* operator_new_00bf55be(std::size_t size) = 0;
    virtual void memset_00bf79f0(void* dst, int value, std::size_t count) = 0;
    // 009553D0, __thiscall(unit, cls), RET 4: set the vehicle class (+538h, refcounted).
    virtual void unit_set_vehicle_class_009553d0(void* unit, void* cls) = 0;
    // 00928560, __fastcall(unit): push the unit on [unit+30h]+24h.
    virtual void game_entity_register_in_parent_list_00928560(void* unit) = 0;
    // 00484540, __thiscall(list, value), RET 4: push_back. 004837D0, __thiscall(list, node), RET 4.
    virtual void unit_list_push_back_00484540(void* list, void* value) = 0;
    virtual void unit_list_erase_node_004837d0(void* list, void* node) = 0;
    // 006D3620, __fastcall(unit): the early world-list detach level.
    virtual void unit_instance_detach_early_lists_006d3620(void* unit) = 0;

    // 00765000 / 00767E30, __thiscall(message, cursor|stream), RET 4: the net entity-create base
    // message writer / reader.
    virtual void net_entity_create_message_write_00765000(void* message, void* cursor) = 0;
    virtual void net_entity_create_message_read_00767e30(void* message, void* stream) = 0;
    // Bit cursor, ECX = cursor. 00429790 / 004294F0 (ptr, count, zero, signed, scale, width), RET 18h.
    virtual void bit_cursor_write_float_array_00429790(void* cursor, float* values, std::uint32_t count,
                                                       std::int32_t zero, std::int32_t is_signed,
                                                       float scale, std::int32_t width) = 0;
    virtual void bit_cursor_read_float_array_004294f0(void* cursor, float* values, std::uint32_t count,
                                                      std::int32_t zero, std::int32_t is_signed,
                                                      float scale, std::int32_t width) = 0;
    // 004290B0 (value), RET 4; 00428D70 (byte*), RET 4.
    virtual void bit_cursor_write_boolean_004290b0(void* cursor, std::uint32_t value) = 0;
    virtual void bit_cursor_read_boolean_00428d70(void* cursor, std::uint8_t* out) = 0;
    // 00429120 (value, bits), RET 8; 00428E30 (word*, bits), RET 8.
    virtual void bit_cursor_write_word_bits_00429120(void* cursor, std::uint32_t value, std::int32_t bits) = 0;
    virtual void bit_cursor_read_word_bits_00428e30(void* cursor, std::uint16_t* out, std::int32_t bits) = 0;
    // 00695870, __fastcall(owner /*ECX*/): the callback-owner teardown.
    virtual void callback_owner_destroy_00695870(void* owner) = 0;
    // 006E3C00 / 006E42D0, __thiscall(platform, arg), RET 4: the bomb-rack constructors
    // (src/gameplay_seg33_cc13.cpp models both); they return the platform in EAX.
    virtual void* bomb_platform_construct_006e3c00(void* memory, void* arg) = 0;
    virtual void* multi_bomb_platform_construct_006e42d0(void* memory, void* arg) = 0;
    // 0084B8C0, __fastcall(mode /*ECX*/, class /*EDX*/, medium, const float* a, const float* b),
    // RET 0Ch: the impact-effect dispatch.
    virtual void projectile_dispatch_impact_effect_0084b8c0(std::int32_t mode, void* weapon_class,
                                                            std::int32_t medium, const float* a,
                                                            const float* b) = 0;
    // 00B6DA70, __thiscall(node, float value, char recurse), RET 8: scene visibility factor.
    virtual void scene_node_set_visibility_factor_00b6da70(void* node, float value, std::int32_t recurse) = 0;
    // 00B6DB70, __fastcall(transform /*ECX*/): refresh the world matrix.
    virtual void transform_refresh_world_matrix_00b6db70(void* transform) = 0;
    // 00766FC0, __thiscall(message, type, a2, entity, a4), RET 10h: entity-create message base.
    virtual void net_entity_create_message_construct_00766fc0(void* message, std::uint32_t type,
                                                              std::uint32_t a2, void* entity,
                                                              std::uint32_t a4) = 0;
    // 00467050, __thiscall(float* matrix, a, b, c), RET 0Ch: rotation from three angles.
    virtual void matrix_set_rotation_from_euler_00467050(float* matrix, float a, float b, float c) = 0;
    // 0077C160, __thiscall(entity, archive), RET 4: the net entity's replication-state writer.
    virtual void net_entity_write_replication_state_0077c160(void* entity, void* archive) = 0;
    // 004842C0, __thiscall(effect, const float* point), RET 4.
    virtual void point_effect_set_world_point_004842c0(void* effect, const float* point) = 0;
    // 0053D9C0, __thiscall(effect, const float* matrix), RET 4.
    virtual void point_effect_set_world_matrix_0053d9c0(void* effect, const float* matrix) = 0;
    // 0085DC80, __fastcall(float* m /*ECX*/): orthonormalize the basis rows in place.
    virtual void matrix_orthonormalize_basis_rows_0085dc80(float* m) = 0;
    // 004142E0, __thiscall(const float* point /*ECX*/, float* out, const float* m), RET 8 -> out.
    virtual const float* vector3_transform_affine_point_004142e0(const float* point, float* out,
                                                                 const float* m) = 0;
    // 00779FC0, __thiscall(entity, peer, message), RET 8: send a session message to a peer.
    virtual void entity_send_message_to_peer_00779fc0(void* entity, void* peer, void* message) = 0;
    // 007668E0, __fastcall(message /*ECX*/): entity-create message base destructor.
    virtual void net_entity_create_message_destruct_007668e0(void* message) = 0;
    // 008746A0, __thiscall(element, steps), RET 4: run that many fixed steps.
    virtual void sim_object_run_fixed_steps_008746a0(void* element, std::int32_t steps) = 0;
    // [00F876AC] (float): the fixed-step accumulator; [00F876B0] (int): the frame counter.
    virtual float fixed_step_remainder_00f876ac() = 0;
    virtual std::int32_t frame_counter_00f876b0() = 0;
    // 004134F0, __thiscall(float* dst, const float* src), RET 4: 4x4 copy (x87 loads/stores).
    virtual void matrix_copy_4x4_x87_004134f0(void* dst, const void* src) = 0;
    // 00867B10, __fastcall(effect /*ECX*/): stop the point effect's children.
    virtual void point_effect_stop_children_00867b10(void* effect) = 0;
    // [00CE2220]: InterlockedDecrement.
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* value) = 0;
    // 009277F0 (thunk to 00923060), __fastcall(record /*ECX*/): the release tail.
    virtual void release_sub_objects_tail_009277f0(void* record) = 0;
    // The five globals of the entity handle tables (00F89A0C, 00F89A10, 00F89A54, 00F89A60,
    // 00F89AA8), read by the inlined lookups.
    virtual HandleTables entity_handle_tables_00f89a0c() = 0;
    // 0075B500, __thiscall(object, float* out), RET 4: out = identity.
    virtual void transform_set_identity_0075b500(void* object, float* out) = 0;
    // 006E6BB0, __thiscall(shot = record+310h, owner), RET 4: bind the owner.
    virtual void projectile_bind_owner_006e6bb0(void* shot, void* owner) = 0;
    // [00CE221C]: InterlockedIncrement.
    virtual std::int32_t interlocked_increment_00ce221c(std::int32_t* value) = 0;
    // [[00E188A8]+19ECh]: the game's point-effect manager.
    virtual void* effect_manager_00e188a8_19ec() = 0;
    // 008685E0, __fastcall(void** out /*ECX*/, manager /*EDX*/, descriptor (by-value reference,
    // consumed), const float* position, a, b), RET 10h -> out.
    virtual void* const* point_effect_create_from_position_008685e0(void** out, void* manager, void* descriptor,
                                                                    const float* position, std::int32_t a,
                                                                    std::int32_t b) = 0;
    // 00BD2F10, (ECX passed through) (float lo, float hi), RET 8 -> ST0: the shared generator's
    // uniform draw (the process-wide RNG stream).
    virtual float random_uniform_float_range_00bd2f10(std::int32_t ecx, float lo, float hi) = 0;
    // 0077EED0 / 0077E380, __thiscall(entity[, arg]): the unit-owner entity base constructor
    // (RET 4) and destructor (RET).
    virtual void unit_owner_entity_construct_0077eed0(void* entity, void* arg) = 0;
    virtual void unit_owner_entity_destruct_0077e380(void* entity) = 0;
    // 00875890, __thiscall(element, payload, group), RET 8: the tick registration constructor.
    virtual void tick_registration_construct_00875890(void* element, void* payload, std::int32_t group) = 0;
    // 00875490, __fastcall(element /*ECX*/): the tick element destructor.
    virtual void unit_tick_element_destruct_00875490(void* element) = 0;
    // 00928570, __fastcall(entity /*ECX*/): leave [entity+30h]+24h.
    virtual void game_entity_detach_from_parent_list_00928570(void* entity) = 0;
    // 00779BB0, __thiscall(entity, lua, key), RET 8: the base Lua property getter.
    virtual void unit_get_property_recon_level_00779bb0(void* entity, void* lua, void* key) = 0;
    // 00BF7FBF, __cdecl: strcmp.
    virtual int strcmp_00bf7fbf(const char* a, const char* b) = 0;
    // 004260B0, __thiscall(NativeStr* out, int value), RET 4 -> out.
    virtual NativeStr* native_string_from_int_004260b0(NativeStr* out, std::int32_t value) = 0;
    // LuaObject (14h bytes): 00B67910 (lua, out, literal) RET 8 -> out; 00B678E0 (table, out,
    // NativeStr*) RET 8 -> out; 00B663D0 (object) push; 00B66430 (lua) push nil; 00B67700 destroy.
    virtual void* lua_object_get_by_literal_00b67910(void* lua, void* out, const char* name) = 0;
    virtual void* lua_object_get_by_native_string_00b678e0(void* table, void* out, const NativeStr* name) = 0;
    virtual void lua_object_push_value_00b663d0(void* object) = 0;
    virtual void lua_object_push_nil_00b66430(void* lua) = 0;
    virtual void lua_object_destruct_00b67700(void* object) = 0;
    // 00419CC0 -> pool; 00BD1510, __thiscall(pool, block, size, flag), RET 0Ch.
    virtual void* sized_storage_pool_00419cc0() = 0;
    virtual void sized_storage_pool_return_block_00bd1510(void* pool, void* block, std::uint32_t size,
                                                          std::int32_t flag) = 0;
    // [[00E188A8]+19CCh], [[00E188A8]+19F0h], [[00E188A8]+1FE4h], [00E188A8]+1A0Ch, and the empty
    // string 00E19982.
    virtual void* scene_root_00e188a8_19cc() = 0;
    virtual void* game_world_00e188a8_19f0() = 0;
    virtual std::int32_t session_mode_00e188a8_1fe4() = 0;
    virtual void* lua_state_owner_00e188a8_1a0c() = 0;
    virtual const char* empty_string_00e19982() = 0;
    // 0042ED50, __fastcall(node /*ECX*/): invalidate the subtree pose.
    virtual void scene_node_invalidate_subtree_pose_0042ed50(void* node) = 0;
    // 0078CF20, __thiscall(world, float x, float z), RET 8 -> ST0: the water height.
    virtual float game_world_sample_water_height_0078cf20(void* world, float x, float z) = 0;
    // 0077F0E0, __fastcall(entity /*ECX*/): the owner-entity activate and register.
    virtual void unit_owner_entity_activate_and_register_0077f0e0(void* entity) = 0;
    // 00B6E680, __thiscall(node, parent), RET 4; 00B6D890, __thiscall(node, root), RET 4.
    virtual void node_set_parent_00b6e680(void* node, void* parent) = 0;
    virtual void node_propagate_root_registration_00b6d890(void* node, void* root) = 0;
    // 0048D6C0, __thiscall(entity, const char* name), RET 4.
    virtual void scene_entity_set_name_0048d6c0(void* entity, const char* name) = 0;
    // 009238A0, __thiscall(entity, LuaObject* out), RET 4: the entity's saved Lua data.
    virtual void entity_saved_lua_data_009238a0(void* entity, void* out) = 0;
    // 004425C0, __thiscall(reader, LuaObject by value (14h bytes, consumed)), RET 14h.
    virtual void lua_reader_construct_004425c0(void* reader, void* lua_data) = 0;
    // 00BD8E20 (key) RET 8 / 00BD7A20 () / 00BD6830 (key, value) RET 10h / 00441A20 () on the reader.
    virtual void lua_reader_enter_key_00bd8e20(void* reader, ArchiveArg key) = 0;
    virtual void lua_reader_leave_00bd7a20(void* reader) = 0;
    virtual void lua_reader_read_field_00bd6830(void* reader, ArchiveArg key, ArchiveArg value) = 0;
    virtual void lua_reader_destroy_00441a20(void* reader) = 0;
    // 0041DD40, __thiscall(NativeStr*, size, flag), RET 8; 00BF7680, __cdecl memcpy.
    virtual void native_string_resize_0041dd40(NativeStr* text, std::uint32_t size, std::int32_t flag) = 0;
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t count) = 0;
    // 00B67980 (owner, out) RET 4; 00B67800 (object, out, name) RET 8; 00B67720 (object, out,
    // index) RET 8; 00927B40 (entity, out) RET 4; 00B675D0 (object, NativeStr* name, value) RET 8.
    virtual void* lua_state_owner_get_globals_00b67980(void* owner, void* out) = 0;
    virtual void* lua_object_get_by_name_00b67800(void* object, void* out, const char* name) = 0;
    virtual void* lua_object_get_by_index_00b67720(void* object, void* out, std::int32_t index) = 0;
    virtual void* mission_entity_lua_self_object_00927b40(void* entity, void* out) = 0;
    virtual void lua_object_set_object_00b675d0(void* object, const NativeStr* name, void* value) = 0;
    // 0072D7E0, __fastcall(gun /*ECX*/): the gun-base device init.
    virtual void gun_base_init_device_0072d7e0(void* gun) = 0;
    // 009552E0, __thiscall(vehicle_class, count), RET 4 -> the loadout list.
    virtual void* class_loadout_record_009552e0(void* vehicle_class, std::int32_t count) = 0;
    // 00729A80, __thiscall(gun, flag), RET 4 -> AL.
    virtual bool gun_can_fire_00729a80(void* gun, std::int32_t flag) = 0;
    // 0075B430, __thiscall(message, type), RET 4: the session-message base constructor.
    virtual void session_message_construct_base_0075b430(void* message, std::uint32_t type) = 0;
    // 0072D830, __thiscall(gun, message), RET 4: the gun's network-message handler.
    virtual void gun_handle_network_message_0072d830(void* gun, void* message) = 0;
    // 00472520 (TRIV body), __fastcall(object /*ECX*/) -> EAX.
    virtual void* trivial_getter_00472520(void* object) = 0;
    // 0042D2E0, (ECX matrix, EDX float* x, float* y, float* z), RET 8.
    virtual void matrix_extract_normalized_basis_angles_0042d2e0(void* matrix, float* x, float* y, float* z) = 0;
    // 0072ADC0, __thiscall(gun, archive), RET 4: the gun's field writer.
    virtual void gun_debug_dump_fields_0072adc0(void* gun, void* archive) = 0;
    // 004254B0, __cdecl(format, ...): the diagnostic print (an empty body in this build).
    virtual void diagnostic_004254b0(const char* format, const char* argument) = 0;
    // The empty string 00E19984 (.bss).
    virtual const char* empty_string_00e19984() = 0;
    // 00727E30, __fastcall(gun /*ECX*/) -> AL: the gun's fire-if-ready decision.
    virtual bool gun_fire_if_ready_00727e30(void* gun) = 0;
    // 0077C7B0, __thiscall(entity, message, a), RET 8: broadcast a session message.
    virtual void session_owner_broadcast_message_0077c7b0(void* entity, void* message, std::int32_t a) = 0;
    // 005471B0, __thiscall(array, count), RET 4: resize the pointer array.
    virtual void ref_counted_pointer_array_resize_005471b0(void* array, std::int32_t count) = 0;
    // 0072F060 / 00729700, __fastcall(gun /*ECX*/): the gun's saved-state load and descriptor flag.
    virtual void gun_load_saved_state_0072f060(void* gun) = 0;
    virtual void gun_apply_descriptor_node_flag_00729700(void* gun) = 0;
    // 00866B70, __fastcall(handle /*ECX*/): request the effect to stop.
    virtual void effect_handle_request_stop_00866b70(void* handle) = 0;
    // 00867790, __thiscall(effect, float delta, reference), RET 8.
    virtual void effect_update_children_00867790(void* effect, float delta, void* reference) = 0;
    // [[00E188A8]+19FCh].
    virtual void* effect_reference_node_00e188a8_19fc() = 0;
    // 00923060, __fastcall(object /*ECX*/): the release body (also reached through 009277F0).
    virtual void release_sub_objects_body_00923060(void* object) = 0;
    // 00868420, __fastcall(void** out /*ECX*/, manager /*EDX*/, descriptor (by-value reference,
    // consumed), const void* matrix, flag, a), RET 10h -> out.
    virtual void* const* point_effect_create_from_matrix_00868420(void** out, void* manager, void* descriptor,
                                                                  const void* matrix, std::uint32_t flag,
                                                                  std::int32_t a) = 0;
    // 004845D0, __thiscall(void** slot, void* const* source), RET 4: retained-reference assign.
    virtual void retained_reference_assign_004845d0(void* slot, void* const* source) = 0;
    // 00924D90, __thiscall(entity, archive), RET 4: the entity archive writer.
    virtual void entity_write_archive_00924d90(void* entity, void* archive) = 0;
    // 00925CE0 / 00925780 / 00923840, __fastcall(entity /*ECX*/): the scene-node constructor, the
    // entity base destructor and the entity activate base.
    virtual void scene_node_construct_00925ce0(void* entity) = 0;
    virtual void entity_destruct_base_00925780(void* entity) = 0;
    virtual void entity_activate_base_00923840(void* entity) = 0;
    // The global slot 00E199A8.
    virtual void** singleton_slot_00e199a8() = 0;
    // 00926D90, __thiscall(entity, cause), RET 4: kill the mission entity.
    virtual void mission_entity_kill_00926d90(void* entity, std::int32_t cause) = 0;
    // 00BF6989, __cdecl: free.
    virtual void free_00bf6989(void* block) = 0;
    // The twenty C-string pointers at 00E08E58.
    virtual const char* const* weapon_name_table_00e08e58() = 0;
    // The empty string 00E199AC (.bss).
    virtual const char* empty_string_00e199ac() = 0;
    // 0041E870, __thiscall(NativeStr*, const char*), RET 4 -> the string.
    virtual NativeStr* native_string_assign_0041e870(NativeStr* text, const char* source) = 0;
    // 007188A0, __fastcall(const NativeStr* name /*ECX*/) -> EAX: load the resource by name.
    virtual void* resource_load_with_game_factory_007188a0(const NativeStr* name) = 0;
    // 00415350 () -> the singleton lifetime manager; 00BD0C30, __thiscall(manager, object), RET 4.
    virtual void* singleton_lifetime_get_manager_00415350() = 0;
    virtual void singleton_lifetime_register_00bd0c30(void* manager, void* object) = 0;
    // [00CE2218] EnterCriticalSection, [00CE2210] LeaveCriticalSection.
    virtual void enter_critical_section_00ce2218(void* section) = 0;
    virtual void leave_critical_section_00ce2210(void* section) = 0;
    // 00BF681B, __cdecl(size): operator new (the second CRT entry).
    virtual void* operator_new_00bf681b(std::size_t size) = 0;
    // [00E188A8] itself; [[00E188A8]+1A08h]+4 (the Lua machine).
    virtual void* game_00e188a8() = 0;
    virtual void* lua_machine_00e188a8_1a08() = 0;
    // LuaObject: 00B65F50 (object) default ctor; 00B67080 (table, key, value) RET 8 first;
    // 00B67190 (table, key, value) RET 8 next; 00B66420 (ignored, key) RET 4 -> AL end;
    // 00B66000 (object) -> AL exactly boolean true; 00B66290 (object) -> int.
    virtual void lua_object_construct_00b65f50(void* object) = 0;
    virtual void lua_object_iterate_first_00b67080(void* table, void* key, void* value) = 0;
    virtual void lua_object_iterate_next_00b67190(void* table, void* key, void* value) = 0;
    virtual bool lua_object_is_unbound_00b66420(void* ignored, void* key) = 0;
    virtual bool lua_object_is_boolean_true_00b66000(void* object) = 0;
    virtual std::int32_t lua_object_get_integer_00b66290(void* object) = 0;
    // 0043EE60, __cdecl(buffer, format, ...): _vsprintf_s into 2Ah bytes.
    virtual void string_format_buffer_0043ee60(char* buffer, const char* format, std::int32_t value) = 0;
    // 006B8AD0, __thiscall(machine, text, a, b, c), RET 10h: run a Lua string.
    virtual void lua_machine_run_string_006b8ad0(void* machine, const char* text, std::int32_t a, std::int32_t b,
                                                 std::int32_t c) = 0;
    // 00BD30F0, __fastcall(object /*ECX*/): the class-descriptor base destructor.
    virtual void bullet_class_base_destruct_00bd30f0(void* cls) = 0;
    // 006E8320, __fastcall(cls /*ECX*/): the bullet-class base constructor.
    virtual void bullet_class_construct_006e8320(void* cls) = 0;
    // 008255B0, __thiscall(unit, float dt), RET 4: the unit-instance update.
    virtual void unit_instance_update_008255b0(void* unit, float dt) = 0;
    // 006F2C30, (ECX const float* position, EDX owner, kind, void** out), RET 8 -> EAX.
    virtual std::int32_t find_nearest_landing_site_006f2c30(void* position, void* owner, std::int32_t kind,
                                                            void** out) = 0;
    // [00F8A0C4] and 00977DA0, __thiscall(manager, a, unit), RET 8.
    virtual void* warning_manager_00f8a0c4() = 0;
    virtual void warning_manager_report_capture_ship_00977da0(void* manager, std::int32_t a, void* unit) = 0;
    // 007327B0, __thiscall(cls, lua), RET 4: the gun-class Lua reader.
    virtual void gun_class_read_lua_fields_007327b0(void* cls, void* lua) = 0;
    // 00B66330, __thiscall(reference, float fallback), RET 4 -> ST0.
    virtual float lua_reference_get_float_or_default_00b66330(void* reference, float fallback) = 0;
    // 00605070, __fastcall(float* angle /*ECX*/): wrap in place.
    virtual void math_wrap_angle_in_place_00605070(float* angle) = 0;
    // 00438B10 / 00438AA0, (float a, float b), RET 8 -> ST0: wrapped a - b / a + b.
    virtual float math_subtract_wrapped_angle_00438b10(float a, float b) = 0;
    virtual float math_add_wrapped_angle_00438aa0(float a, float b) = 0;
    // 00B662B0, __fastcall(object /*ECX*/) -> the string value; 00B673A0, __thiscall(object,
    // const NativeStr* name, value), RET 8: set a boolean field.
    virtual const char* lua_object_get_string_00b662b0(void* object) = 0;
    virtual void lua_object_set_boolean_00b673a0(void* object, const NativeStr* name, std::int32_t value) = 0;
    // 00425850, __thiscall(const NativeStr*, const char*), RET 4 -> AL: case-insensitive equality.
    virtual bool native_string_equals_insensitive_00425850(const NativeStr* text, const char* other) = 0;
    // 0041DD20, __fastcall(NativeStr* /*ECX*/): release the string storage.
    virtual void native_string_destroy_storage_0041dd20(NativeStr* text) = 0;
    // 00470B80, __fastcall(size /*ECX*/): operator new then memset 0.
    virtual void* memory_alloc_zeroed_00470b80(std::size_t size) = 0;
    // 006EA260, __fastcall(cls /*ECX*/) -> cls: the bomb class constructor (outside this packet).
    virtual void* bomb_class_construct_006ea260(void* cls) = 0;
    // Virtual slots, __thiscall unless noted.
    virtual void archive_open_section_vt04(void* archive, ArchiveArg section) = 0;
    virtual void archive_close_section_vt08(void* archive) = 0;
    virtual void archive_write_field_vt0c(void* archive, ArchiveArg name, ArchiveArg value) = 0;
    virtual void archive_read_field_vt10(void* archive, ArchiveArg name, ArchiveArg value) = 0;
    virtual void gun_bot_clear_target_vt30(void* bot) = 0;
    virtual void* gun_bot_current_target_vt44(void* bot) = 0;
    virtual void gun_release_fire_request_vt1e8(void* gun, std::int32_t value) = 0;
    // Tick element (record+310h) slot 2Ch: non-zero once in water.
    virtual std::int32_t tick_element_in_water_vt2c(void* element) = 0;
    // Bomb record slots 194h / 198h, __thiscall(record, float scaled_step), RET 4.
    virtual void bomb_record_step_air_vt194(void* record, float scaled_step) = 0;
    virtual void bomb_record_step_water_vt198(void* record, float scaled_step) = 0;
    // Scene object slot 34h, __thiscall(object, const float* matrix), RET 4.
    virtual void scene_object_set_matrix_vt34(void* object, const float* matrix) = 0;
    // Tick element slot 4h (float step), RET 4, and slot 0Ch (), RET.
    virtual void tick_element_place_vt04(void* element, float step) = 0;
    virtual void tick_element_activate_vt0c(void* element) = 0;
    // Subject reference slot 4h: the referenced object.
    virtual void* subject_ref_resolve_vt04(void* subject_ref) = 0;
    // Ref-counted object slot 0 (destroy at zero count); deleting destructor slot 4h(flags).
    virtual void ref_counted_destroy_vt00(void* object) = 0;
    virtual void object_deleting_destructor_vt04(void* object, std::int32_t flags) = 0;
    // Bullet class slot 20h, RET 1Ch: create the projectile; answers its +310h sub-object.
    virtual void* bullet_class_create_projectile_vt20(void* cls, std::uint16_t a1, void* a2, const float* position,
                                                     const float* direction_row, std::int32_t a5,
                                                     std::int32_t a6, std::int32_t a7) = 0;
    // Session message slot 20h(entity), RET 4.
    virtual void message_apply_to_entity_vt20(void* message, void* entity) = 0;
    // Projectile class slot 1Ch -> ST0.
    virtual double projectile_class_rate_vt1c(void* cls) = 0;
    // Unit slot 5Ch(1) -> AL.
    virtual bool unit_is_alive_vt5c(void* unit, std::int32_t flag) = 0;
    // Record slot 98h (a, scene, const float* matrix), RET 0Ch; slot 10h () -> EAX.
    virtual void entity_attach_scene_vt98(void* record, std::int32_t a, void* scene, const float* matrix) = 0;
    virtual std::int32_t entity_has_name_vt10(void* record) = 0;
    // Model slot 8h (a, float scale), RET 8 -> the instance.
    virtual void* model_create_instance_vt08(void* model, std::int32_t a, float scale) = 0;
    // Entity slot 18h () -> its scene node.
    virtual void* entity_scene_node_vt18(void* entity) = 0;
    // Scene object slot 38h (const float* local matrix), RET 4.
    virtual void scene_object_set_local_matrix_vt38(void* object, const void* matrix) = 0;
    // Shot slot 34h (const float* a, const float* b), RET 8.
    virtual void shot_set_vectors_vt34(void* shot, const float* a, const float* b) = 0;
    // Rack slots 1BCh (rounds) RET 4, 1C0h (float reload) RET 4, 224h (), 210h (kind, a) RET 8.
    virtual void rack_set_ammo_vt1bc(void* rack, std::int32_t rounds) = 0;
    virtual void rack_set_reload_time_vt1c0(void* rack, float reload_time) = 0;
    virtual void rack_attach_next_round_vt224(void* rack) = 0;
    virtual void* rack_round_of_kind_vt210(void* rack, std::int32_t kind, std::int32_t a) = 0;
    // Bullet descriptor slot 8h (kind) -> AL; owner slot 1E4h () -> AL.
    virtual bool bullet_descriptor_is_kind_vt08(void* descriptor, std::int32_t kind) = 0;
    virtual bool unit_loadout_answers_vt1e4(void* owner) = 0;
    // Rack slots 228h (message) RET 4, 200h (), 1D8h (a, float b, float c) RET 0Ch.
    virtual void rack_apply_fire_message_vt228(void* rack, void* message) = 0;
    virtual void rack_rearm_vt200(void* rack) = 0;
    virtual void rack_fire_vt1d8(void* rack, std::int32_t a, float b, float c) = 0;
    // Rack slot 1F4h (value), RET 4.
    virtual void rack_fire_slot_vt1f4(void* rack, std::int32_t value) = 0;
    // Skill object slot 0 (level), RET 4.
    virtual void skill_set_level_vt00(void* skill, std::int32_t level) = 0;
    // Shot slot 3Ch () (006E6C20 for the base).
    virtual void shot_on_spawn_vt3c(void* shot) = 0;
    // Class slots 14h () and 10h () -> AL.
    virtual void bullet_class_resolve_vt14(void* cls) = 0;
    virtual bool bullet_class_finalise_vt10(void* cls) = 0;
    // Unit slot 174h () -> the current order; order slot 8h () -> AL.
    virtual void* unit_current_order_vt174(void* unit) = 0;
    virtual bool order_is_done_vt08(void* order) = 0;
    // Gun slot 1D0h (flag) -> AL.
    virtual bool gun_can_fire_vt1d0(void* gun, std::int32_t flag) = 0;
    // Bullet class slot 18h (const LuaObject* entry), RET 4: read the class fields.
    virtual void bullet_class_read_lua_vt18(void* cls, void* entry) = 0;
};

// ---------------------------------------------------------------------------
// Gun bots (vtable 00CFE000 and its base).
// ---------------------------------------------------------------------------
bool pilot_bot_party_slot_ai_controlled_006deec0(Seg33aHost& host, void* bot, std::int32_t index);
void gun_bot_set_fire_target_006df170(Seg33aHost& host, void* bot, void* target);
void gun_bot_attach_muzzle_class_006df1f0(Seg33aHost& host, void* bot, void* unit);
void gun_bot_read_properties_006df260(Seg33aHost& host, void* bot, void* archive);
void gun_bot_write_properties_006df390(Seg33aHost& host, void* bot, void* archive);
void artillery_gunner_bot_set_fire_target_006df4c0(Seg33aHost& host, void* bot, void* target);

// ---------------------------------------------------------------------------
// Battleship unit and the ship-family world lists.
// ---------------------------------------------------------------------------
void* battleship_construct_006dfc90(Seg33aHost& host, void* unit, void* arg);
void* ship_unit_sub_object_72c_006dfd10(void* unit);
void* battleship_create_006dfef0(Seg33aHost& host, void* cls, void* arg);
void ship_unit_register_in_world_lists_006dff80(Seg33aHost& host, void* unit);
void unit_instance_detach_from_mid_world_lists_006dffc0(Seg33aHost& host, void* unit);


// ---------------------------------------------------------------------------
// BOMB_CREATE message, bomb platforms, observer member 00CF9348.
// ---------------------------------------------------------------------------
inline constexpr float kFltMax_00d7a248 = 3.4028234663852886e+38f;  // 00D7A248, float
void object_set_vec3_pair_60_006e05f0(void* object, const float* v);
void bomb_create_message_write_006e0690(Seg33aHost& host, void* message, void* cursor);
void bomb_create_message_read_006e06f0(Seg33aHost& host, void* message, void* stream);
void bomb_create_message_vslot20_noop_006e0750(void* message, void* arg);
void observer_member_9348_destruct_006e0860(Seg33aHost& host, void* member);
void* bomb_platform_create_006e08f0(Seg33aHost& host, void* cls, void* arg);
void* multiple_bomb_platform_create_006e0970(Seg33aHost& host, void* cls, void* arg);
bool bomb_projectile_handle_message_006e09f0(Seg33aHost& host, void* projectile, void* message);

void bomb_projectile_set_visible_006e0b40(Seg33aHost& host, void* projectile, std::uint8_t visible);
void bomb_projectile_hide_node_006e0bc0(Seg33aHost& host, void* projectile, void* unused);
void bomb_projectile_trail_last_point_006e0cd0(Seg33aHost& host, void* object, float* out);
void* bomb_create_message_construct_006e0d20(Seg33aHost& host, void* message, std::uint32_t type,
                                            std::uint32_t a2, void* entity);
void observer_member_set_target_006e0db0(Seg33aHost& host, void* member, void* target);
void* observer_member_9348_construct_006e0e00(Seg33aHost& host, void* member, void* target);
void* observer_member_set_target_006e0e60(Seg33aHost& host, void* member, void* target);
float* transform_from_object_006e0ea0(Seg33aHost& host, void* object, float* out);


// ---------------------------------------------------------------------------
// Bomb projectile state and pose.
// ---------------------------------------------------------------------------
inline constexpr const char* kBulletClassKey = "bulletclass";        // 00CF9380
inline constexpr const char* kBombSectionKey = "_bomb";              // 00CF9378
inline constexpr const char* kBulletBaseSectionKey = "_bulletBase";  // 00CF936C
inline constexpr const char* kFlyingKey = "flying";                  // 00CF9364
inline constexpr double kTrailMinSegmentSquared_00d7a268 = 9.999999747378752e-05;  // 00D7A268, double
void bomb_projectile_write_state_006e0f40(Seg33aHost& host, void* projectile, void* archive);
void bomb_projectile_place_step_pose_006e1060(Seg33aHost& host, void* self, float step);

inline constexpr float kLagSeconds_00d0de84 = 0.05f;  // 00D0DE84, float
void bomb_projectile_send_create_message_006e1640(Seg33aHost& host, void* projectile, void* peer);
void bomb_projectile_apply_launch_state_006e16d0(Seg33aHost& host, void* projectile, void* state);
void observer_member_on_target_gone_006e1780(Seg33aHost& host, void* member, void* subject_ref);
void observer_member_938c_on_notify_006e17d0(Seg33aHost& host, void* member, void* subject_ref);
void* observer_member_set_target_006e1820(Seg33aHost& host, void* member, void* target);
void bomb_projectile_release_sub_objects_006e1860(Seg33aHost& host, void* record);
void* bomb_create_message_create_entity_006e1930(Seg33aHost& host, void* message);
void* observer_member_938c_construct_006e1b40(void* member);


// ---------------------------------------------------------------------------
// Projectile shot.
// ---------------------------------------------------------------------------
inline constexpr float kTrailInterval_00ce7628 = 0.03333333507180214f;  // 00CE7628, float
void* attacker_handle_repoint_006e1b90(Seg33aHost& host, void* member, void* target);
void projectile_shot_ignite_motor_006e1cd0(Seg33aHost& host, void* shot);
void* projectile_shot_base_construct_006e22d0(Seg33aHost& host, void* shot);
float random_uniform_between_ac_and_b0_006e2410(Seg33aHost& host, void* object);
void projectile_shot_get_position_006e2470(void* shot, float* out);
float projectile_shot_random_class_range_006e24e0(Seg33aHost& host, void* shot);
void projectile_shot_set_weapon_scale_006e2550(void* shot, float scale);
std::uint8_t projectile_shot_class_flag_74_006e2580(void* shot);
void projectile_shot_destruct_006e25a0(Seg33aHost& host, void* shot);


// ---------------------------------------------------------------------------
// Bomb projectile record.
// ---------------------------------------------------------------------------
inline constexpr float kMinusOne_00d7a260 = -1.0f;              // 00D7A260, float
inline constexpr const char* kOwnerKey = "owner";               // 00CF9620
inline constexpr const char* kThisTableKey = "thisTable";       // 00CE7494
void* bomb_projectile_construct_006e2670(Seg33aHost& host, void* record, void* arg);
void* bomb_projectile_bullet_block_006e27c0(void* record);
void* bomb_projectile_tick_element_006e27d0(void* record);
float weapon_entity_hull_damage_base_006e2820(Seg33aHost& host, void* record);
void bomb_projectile_get_velocity_006e2860(void* record, float* out);
void bomb_projectile_vslot18c_noop_006e2940(void* record, void* arg);
void bomb_register_in_world_lists_006e2990(Seg33aHost& host, void* record);
void bomb_family_detach_from_world_lists_006e29c0(Seg33aHost& host, void* record);
void bomb_projectile_destruct_006e2a20(Seg33aHost& host, void* record);
void projectile_get_property_owner_006e2ac0(Seg33aHost& host, void* record, void* lua, void* key_object);

inline constexpr const char* kClassKey = "Class";      // 00CE44C8
inline constexpr const char* kBulletsKey = "Bullets";  // 00CE5CC8
void* bomb_class_create_projectile_006e2c00(Seg33aHost& host, void* cls, std::uint32_t a1, void* a2,
                                            const float* position, const float* direction, std::int32_t a5,
                                            std::int32_t a6, std::int32_t a7);
void bomb_projectile_activate_006e2e10(Seg33aHost& host, void* record);
void mbomb_platform_bind_owner_loadout_006e3350(Seg33aHost& host, void* rack);
bool mbomb_platform_can_fire_006e3460(Seg33aHost& host, void* rack, std::int32_t flag);
void* bomb_platform_find_child_of_kind_006e3490(Seg33aHost& host, void* platform, std::int32_t kind);
std::int32_t bomb_platform_count_children_of_kind_006e34d0(Seg33aHost& host, void* platform, std::int32_t kind);
bool session_mode_field_is_zero_or_one_006e38e0(void* session);


// ---------------------------------------------------------------------------
// Bomb platform messages and ordnance queries.
// ---------------------------------------------------------------------------
void* gun_fire_message_construct_006e3940(Seg33aHost& host, void* message, std::uint16_t value);
void* bomb_platform_reload_message_construct_006e3a60(Seg33aHost& host, void* message, std::int32_t value);
void slot_copy_vec3_identity_006e3dc0(void* gun, float* out, const float* in);
void stub_ret4_006e3de0(void* gun, void* arg);
bool mbomb_platform_has_round_of_kind_006e3fe0(Seg33aHost& host, void* rack, std::int32_t kind, std::uint8_t loadout);
void* mbomb_platform_loaded_ordnance_006e4060(Seg33aHost& host, void* rack, std::uint8_t loadout);
void* unit_bomb_rack_loaded_device_006e40f0(Seg33aHost& host, void* rack);

inline constexpr const char* kBombPlatformSectionKey = "_bombPlatform";  // 00CF9B84
inline constexpr const char* kAmmoKey = "ammo";                          // 00CF9B7C
inline constexpr const char* kOrgAmmoKey = "orgAmmo";                    // 00CF9B74
inline constexpr const char* kToRepeatTimeKey = "toRepeatTime";          // 00CF9B64
inline constexpr const char* kDropBombsKey = "dropBombs";                // 00CF9B58
inline constexpr const char* kFireMsgReceived = "MBombPlatform - fire msg received";  // 00CF9C04
inline constexpr const char* kFireMsgBombFound =
    "MBombPlatform - fire msg received: bomb founded: %s";  // 00CF9BD0
inline constexpr const char* kFireMsgBombNotFound =
    "MBombPlatform - fire msg received, BUT bomb NOT founded!!!";  // 00CF9B94
bool mbomb_platform_handle_message_006e4290(Seg33aHost& host, void* rack, void* message);
bool mmultiple_bomb_platform_is_busy_006e4360(void* rack);
void* mmultiple_bomb_platform_loaded_ordnance_006e4640(Seg33aHost& host, void* rack, std::uint8_t loadout);
void* bomb_platform_fire_message_construct_006e47e0(Seg33aHost& host, void* message, std::uint32_t unused,
                                                     std::uint16_t id, void* object, const float* a,
                                                     const float* b);
void mbomb_platform_serialize_006e4a60(Seg33aHost& host, void* rack, void* archive);
void mbomb_platform_apply_fire_message_006e4b60(Seg33aHost& host, void* rack, void* message);

inline constexpr const char* kFireMsgSent = "MBombPlatform::Fire: fire msg sent, (%s)";  // 00CF9C28
inline constexpr const char* kUnnamed_00ce391c = "unnamed";                                 // 00CE391C
bool mbomb_platform_fire_if_ready_006e4c10(Seg33aHost& host, void* rack);
void* entity_as_kind_0f_006e4d30(Seg33aHost& host, void* entity);

inline constexpr const char* kSkillLevelKey = "skilllevel";  // 00CF9CD8
inline constexpr const char* kIdKey = "id";                  // 00CF16BC
void* entity_child_local_frame_006e5a80(Seg33aHost& host, void* entity, std::int32_t index);
void bomb_platform_load_saved_state_006e6010(Seg33aHost& host, void* rack);
std::int32_t skill_from_crew_level_006e6210(Seg33aHost& host, std::int32_t crew);
std::int32_t crew_level_from_skill_006e6270(Seg33aHost& host, std::int32_t skill);
void skill_level_read_006e6360(Seg33aHost& host, void* skill, void* reader);
void skill_level_write_006e63a0(Seg33aHost& host, void* skill, void* archive);
void projectile_shot_set_air_mode_006e6410(Seg33aHost& host, void* shot, void* unused);
void projectile_shot_set_water_mode_006e6450(Seg33aHost& host, void* shot, void* unused);
void* observer_member_9348_copy_construct_006e6670(Seg33aHost& host, void* member, const void* other);
void save_writer_write_observer_ref_9348_006e6810(Seg33aHost& host, void* archive, ArchiveArg name,
                                                  void* member_copy);
void projectile_shot_advance_owner_grace_006e6900(Seg33aHost& host, void* shot, float dt);
void bullet_stop_trail_effect_006e6940(Seg33aHost& host, void* object);
void save_reader_read_observer_ref_006e6a60(Seg33aHost& host, void* reader, void* member, ArchiveArg name);
void save_reader_read_unit_handle_006e6b10(Seg33aHost& host, void* member, void* reader);

inline constexpr const char* kBbvKey = "bbv";                    // 00CF9D6C
inline constexpr const char* kBbErrorKey = "bberror";            // 00CF9D64
inline constexpr const char* kCollOwnerKey = "collOwner";        // 00CF9D58
inline constexpr const char* kOwnerTimeKey = "ownerTime";        // 00CF9D4C
inline constexpr const char* kOwnerTypeKey = "ownerType";        // 00CF9D40
inline constexpr const char* kPartyIndexKey = "partyIndex";      // 00CF9D34
inline constexpr const char* kPlayerIndexKey = "playerIndex";    // 00CF9D28
inline constexpr const char* kHitAnythingKey = "hitAnything";    // 00CF9D1C
inline constexpr const char* kUnderWaterKey = "underWater";      // 00CF9D10
inline constexpr const char* kFlyTimeKey = "flyTime";            // 00CF9D08
inline constexpr const char* kPenetrationKey = "penetration";    // 00CF9CFC
inline constexpr const char* kHitDamageMulKey = "hitDamageMul";  // 00CF9CEC
inline constexpr const char* kTracersKey = "tracers";            // 00CF9CE4
void projectile_shot_on_spawn_006e6c20(Seg33aHost& host, void* shot);
void bullet_base_write_state_006e6e80(Seg33aHost& host, void* base, void* archive);


// ---------------------------------------------------------------------------
// Bullet.
// ---------------------------------------------------------------------------
inline constexpr const char* kBulletSectionKey = "_bullet";  // 00CF9D70
inline constexpr float kBulletPhaseMax_00d7a2f0 = 0.10000000149011612f;  // 00D7A2F0, float
void bullet_write_state_006e71b0(Seg33aHost& host, void* bullet, void* archive);
void bullet_base_read_state_006e7850(Seg33aHost& host, void* base, void* reader);
void* projectile_tickable_entity_construct_006e7b00(Seg33aHost& host, void* bullet);
void* bullet_tick_element_006e7c30(void* bullet);
void* bullet_bullet_block_006e7c40(void* bullet);
void bullet_destruct_006e7de0(Seg33aHost& host, void* bullet);
void bullet_activate_006e7e80(Seg33aHost& host, void* bullet);

void singleton_e199a8_reset_on_unwind_006e7fd0(Seg33aHost& host, void* object);
void entity_ref_kill_and_clear_006e8010(Seg33aHost& host, void** ref, std::int32_t cause);
void entity_set_world_position_direct_006e8040(void* entity, const float* position);
void** ref_ptr_assign_006e80d0(Seg33aHost& host, void** slot, void* value);
void pointer_vector_reserve_006e8150(Seg33aHost& host, void* vector, std::int32_t n);
std::int32_t weapon_name_index_006e8250(Seg33aHost& host, NativeStr name);


// ---------------------------------------------------------------------------
// Bullet-class cache.
// ---------------------------------------------------------------------------
void bullet_class_cache_resize_006e86f0(Seg33aHost& host, void* vector, std::int32_t n);
void bullet_class_resolve_linked_class_006e97f0(Seg33aHost& host, void* cls);
void bullet_class_cache_destruct_006e9aa0(Seg33aHost& host, void* vector);
void bullet_class_cache_destruct_006e9ad0(Seg33aHost& host, void* vector);
void bullet_class_cache_singleton_destruct_006e9b10(Seg33aHost& host, void* cache);
void* bullet_class_cache_get_006e9b70(Seg33aHost& host);

inline constexpr const char* kGotKey = "Got";                          // 00CE452C
inline constexpr const char* kBulletsNilFormat = "Bullets[%d]=nil";    // 00CFA430
void bullet_classes_reload_from_lua_006e9c50(Seg33aHost& host);
void bullet_class_destruct_006e9e80(Seg33aHost& host, void* cls);


// ---------------------------------------------------------------------------
// Bullet-class family.
// ---------------------------------------------------------------------------
inline constexpr const char* kArtilleryBulletName = "MArtilleryBullet";  // 00CFA464
inline constexpr const char* kKamikazePlaneName = "MKamikazePlane";      // 00CFA4A0
inline constexpr const char* kFlakBulletName = "MFlakBullet";            // 00CFA560
inline constexpr double kOnePointFive_00ce3d78 = 1.5;                     // 00CE3D78, double
void bullet_class_cache_clear_006ea1a0(Seg33aHost& host);
const char* artillery_bullet_class_get_name_006ea1e0();
const char* kamikaze_plane_class_get_name_006ea220();
const char* flak_bullet_class_get_name_006ea4a0();
void* kamikaze_plane_class_construct_006ea200(Seg33aHost& host, void* cls);
void bomb_class_destruct_body_006ea2c0(Seg33aHost& host, void* cls);
void* rocket_class_construct_006ea330(Seg33aHost& host, void* cls);
void* depth_charge_class_construct_006ea3a0(Seg33aHost& host, void* cls);
void depth_charge_class_destruct_006ea400(Seg33aHost& host, void* cls);
void* flak_bullet_class_construct_006ea470(Seg33aHost& host, void* cls);
void* torpedo_class_construct_006ea4f0(Seg33aHost& host, void* cls);
float torpedo_class_scaled_e4_006ea530(void* cls);
void torpedo_class_destruct_006ea570(Seg33aHost& host, void* cls);
void* water_mine_class_construct_006ea5e0(Seg33aHost& host, void* cls);
void water_mine_class_destruct_006ea640(Seg33aHost& host, void* cls);
void* dummy_target_class_construct_006ea6b0(Seg33aHost& host, void* cls);
void* paratrooper_class_construct_006ea720(Seg33aHost& host, void* cls);
void paratrooper_class_destruct_006ea780(Seg33aHost& host, void* cls);
void* dummy_kamikaze_plane_class_construct_006ea7f0(Seg33aHost& host, void* cls);
void* dummy_submarine_class_construct_006ea870(Seg33aHost& host, void* cls);


// ---------------------------------------------------------------------------
// Cargo and catapult.
// ---------------------------------------------------------------------------
inline constexpr float kTwo_00ce3958 = 2.0f;  // 00CE3958, float
void* const* bullet_class_acquire_and_finalise_006eafe0(Seg33aHost& host, void** holder, std::int32_t id);
void* cargo_construct_006eb160(Seg33aHost& host, void* unit, void* arg);
void* vehicle_class_cargo_create_instance_006eb290(Seg33aHost& host, void* cls, void* arg);
void cargo_update_006eb320(Seg33aHost& host, void* cargo, float dt);
bool plane_is_launching_006eb5c0(void* plane);
void mcatapult_set_ammo_006eb660(void* catapult, std::int32_t n);
void mcatapult_queue_launch_006eb680(Seg33aHost& host, void* catapult, float heading);

inline constexpr float kEight_00ce3918 = 8.0f;                              // 00CE3918, float
inline constexpr float kSnapAngle_00cfaa48 = 0.0001745329354889691f;        // 00CFAA48, float
inline constexpr const char* kVertRotAngleKey = "VertRotAngle";             // 00CFA9FC
inline constexpr const char* kPlaneStartPosZKey = "PlaneStartPosZ";         // 00CFA9EC
inline constexpr const char* kPlaneStartPosYKey = "PlaneStartPosY";         // 00CFA9DC
void catapult_class_read_lua_fields_006eb6b0(Seg33aHost& host, void* cls, void* lua);
float ratio_abs_dc_over_8c_006eb870(void* cls);
void* vehicle_add_launched_message_construct_006eb9c0(Seg33aHost& host, void* message, void* entity);
void* plane_takeoff_message_construct_006eba90(Seg33aHost& host, void* message, std::uint8_t value);
void* catapult_fire_message_construct_006ebb90(Seg33aHost& host, void* message, void* entity, float heading);
void catapult_turn_commit_angles_006ebcc0(Seg33aHost& host, void* turn);
void catapult_turn_step_006ebd90(Seg33aHost& host, void* turn, float dt);
bool mcatapult_fire_if_ready_006ebff0(Seg33aHost& host, void* catapult);

inline constexpr const char* kTypeKey = "Type";                                  // 00CE4780
inline constexpr const char* kBulletTypeName = "Bullet";                         // 00CFA720
inline constexpr const char* kArtilleryTypeName = "Artillery";                   // 00CE5454
inline constexpr const char* kKamikazeTypeName = "Kamikaze";                     // 00CFA714
inline constexpr const char* kBombTypeName = "Bomb";                             // 00CFA70C
inline constexpr const char* kTorpedoTypeName = "Torpedo";                       // 00CE544C
inline constexpr const char* kDepthChargeTypeName = "DepthCharge";               // 00CFA700
inline constexpr const char* kDummyTargetTypeName = "DummyTarget";               // 00CFA6F4
inline constexpr const char* kDummyKamikazePlaneTypeName = "DummyKamikazePlane"; // 00CFA6E0
inline constexpr const char* kDummySubmarineTypeName = "DummySubmarine";         // 00CFA6D0
inline constexpr const char* kParatrooperTypeName = "Paratrooper";               // 00CFA6C4
inline constexpr const char* kFlakTypeName = "Flak";                             // 00CFA6BC
inline constexpr const char* kRocketTypeName = "Rocket";                         // 00CFA6B4
inline constexpr const char* kWaterMineTypeName = "WaterMine";                   // 00CEA74C
void* const* bullet_class_get_or_create_006ea910(Seg33aHost& host, void** holder, std::int32_t id);

}  // namespace bsp::cc13_seg33a

#endif
