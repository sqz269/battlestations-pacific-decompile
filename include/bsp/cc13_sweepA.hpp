#pragma once
// Packet cc13-sweepA: the leftover functions of segments 0..14 (00401010..00527C80)
// that the earlier cc13 waves left without a reconstruction record.
// docs/CC13_SWEEPA.md lists every function of the pool with its outcome, ABI and
// uncertainty, and the ones skipped with the reason.
//
// Objects are reached through Raw (byte offsets into the Win32 layout). Every
// call the image makes goes through a SweepAHost method named for its target;
// CRT memory and string primitives (memcpy, vsprintf_s) are bound directly.
// Descriptive names are hypotheses, not recovered symbols. Nothing here is wired.

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_sweepA {

static_assert(sizeof(void*) == 4, "the sweepA objects are modelled in their Win32 layout");

// Raw view of an image object: at<T>(off) is the field at that byte offset.
struct Raw {
    std::uint8_t* p;
    template <class T>
    T& at(std::size_t off) const noexcept { return *reinterpret_cast<T*>(p + off); }
    void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
    std::uint8_t* field(std::size_t off) const noexcept { return p + off; }
};

// Opaque storage for one image LuaObject (00B65F50 constructs it). The frames
// that hold them space them 14h bytes apart; the true size is not established.
struct LuaObjectStorage {
    alignas(4) std::uint8_t bytes[0x14];
};

struct SweepAHost {
    virtual ~SweepAHost() = default;

    // The pointer stored in the image global dword at `address` (for example
    // [00E188A8], the game object).
    virtual void* global_pointer(std::uint32_t address) = 0;
    // The address of the image global at `address` (for a non-pointer global).
    virtual void* global_address(std::uint32_t address) = 0;

    // --- CRT allocator ---------------------------------------------------
    // 00BF55BE operator new(size), __cdecl.
    virtual void* operator_new_00bf55be(std::uint32_t size) = 0;
    // 00BF6989 _free(block), __cdecl (thunk).
    virtual void free_00bf6989(void* block) = 0;

    // --- strings ----------------------------------------------------------
    // 0041DD40, __thiscall(ECX = {length +0, data +4}, length, char preserve), RET 8.
    virtual void native_string_resize_0041dd40(Raw string, std::uint32_t length, char preserve) = 0;

    // --- camera mover -----------------------------------------------------
    // 004329D0, __thiscall(ECX = mover, float dt), RET 4.
    virtual void camera_mover_apply_shake_004329d0(Raw mover, float dt) = 0;
    // 0042E180, __thiscall(ECX = mover): rebuilds the six planes at +1F8h.
    virtual void camera_mover_build_frustum_planes_0042e180(Raw mover) = 0;

    // --- Lua objects and the save archive ---------------------------------
    // 00A3E540, __thiscall(ECX = [00F8ABE8]), AL = gate.
    virtual bool storage_gate_00a3e540(void* owner) = 0;
    // 00B67980, __thiscall(ECX = state owner, LuaObject* out), RET 4, EAX = out.
    virtual LuaObjectStorage* lua_state_owner_get_globals_00b67980(void* state_owner,
                                                                    LuaObjectStorage* out) = 0;
    // 00B67800, __thiscall(ECX = table, LuaObject* out, const char* name), RET 8.
    virtual void lua_object_get_by_name_00b67800(LuaObjectStorage* table, LuaObjectStorage* out,
                                                 const char* name) = 0;
    // 00B67700, __thiscall(ECX = object): destructor.
    virtual void lua_object_destruct_00b67700(LuaObjectStorage* object) = 0;
    // 00B65F50, __thiscall(ECX = object): nil constructor.
    virtual void lua_object_construct_00b65f50(LuaObjectStorage* object) = 0;
    // 00B65F70, __thiscall(ECX = object), AL = bound.
    virtual bool lua_object_is_bound_00b65f70(LuaObjectStorage* object) = 0;
    // 00B66420, __thiscall(ECX = table, LuaObject* key), RET 4, AL = iteration ended.
    virtual bool lua_object_is_unbound_00b66420(LuaObjectStorage* table, LuaObjectStorage* key) = 0;
    // 00B67080 / 00B67190, __thiscall(ECX = table, key*, value*), RET 8.
    virtual void lua_object_iterate_first_00b67080(LuaObjectStorage* table, LuaObjectStorage* key,
                                                   LuaObjectStorage* value) = 0;
    virtual void lua_object_iterate_next_00b67190(LuaObjectStorage* table, LuaObjectStorage* key,
                                                  LuaObjectStorage* value) = 0;
    // 00BD8C80, __thiscall(ECX = writer, {dword 0, const char* name} by value), RET 8.
    virtual void archive_text_writer_begin_section_00bd8c80(Raw writer, std::uint32_t zero,
                                                            const char* name) = 0;
    // 00BD6CC0, __thiscall(ECX = writer).
    virtual void archive_text_writer_end_section_00bd6cc0(Raw writer) = 0;
    // 00435D30, __thiscall(ECX = owner, writer*, key*, value*), RET 0Ch.
    virtual void lua_slot_write_archive_00435d30(Raw owner, Raw writer, LuaObjectStorage* key,
                                                 LuaObjectStorage* value) = 0;
    // 006ADB50, __thiscall(ECX = a code address used as the continuation, for
    // example 00BD53C0 or 004389F0).
    virtual void storage_operation_run_continuation_006adb50(std::uint32_t operation) = 0;
    // 00B67690, __thiscall(ECX = destination, const LuaObject* source), RET 4.
    virtual void lua_object_assign_00b67690(void* destination, const void* source) = 0;
    // 00BD3E10, __thiscall(ECX = storage manager [0109CECC], name*, kind), RET 8.
    virtual void storage_request_single_write_00bd3e10(void* manager, void* name,
                                                       std::uint32_t kind) = 0;
    // 00425C20, no arguments, EAX = the profile manager singleton.
    virtual void* profile_manager_get_singleton_00425c20() = 0;
    // 00438020, __thiscall(ECX = profile manager).
    virtual void profile_manager_save_checkpoint_00438020(void* manager) = 0;

    // --- math, camera projection and random numbers -----------------------
    // 00B62D10, __thiscall(ECX = source float4, float4* destination, matrix), RET 8,
    // EAX = destination.
    virtual float* vector4_transform_00b62d10(const float* source, float* destination,
                                              const void* matrix) = 0;
    // 0043A660, __fastcall(ECX = const float3* world, EDX = float3* out, int mode,
    // char clamp_depth), RET 8, EAX = the clip code.
    virtual std::int32_t camera_project_world_to_screen_0043a660(const float* world, float* out,
                                                                 std::int32_t mode,
                                                                 char clamp_depth) = 0;
    // 00BD2F10, ECX = stream, (float lo, float hi), RET 8, ST0 = the draw. The
    // process-wide generator: a call here advances the shared stream.
    virtual float random_uniform_00bd2f10(std::uint32_t stream, float lo, float hi) = 0;
    // 00B6FDE0, __thiscall(ECX = camera), EAX = viewport (+10h int width, +14h
    // int height) or null.
    virtual void* camera_get_viewport_00b6fde0(void* camera) = 0;
    // 00B70510, __thiscall(ECX = camera), EAX = the cached inverse view-projection.
    virtual const void* camera_get_inverse_view_projection_00b70510(void* camera) = 0;
    // --- pooled strings and the property map ------------------------------
    // 00419CC0, no arguments, EAX = the sized storage pool (callers push the
    // following thiscall's arguments first).
    virtual void* sized_storage_pool_get_singleton_00419cc0() = 0;
    // 00BD1510, __thiscall(ECX = pool, block, size, unused), RET 0Ch.
    virtual void sized_storage_pool_return_block_00bd1510(void* pool, void* block, std::uint32_t size,
                                                          std::uint32_t unused) = 0;
    // 0043B0A0, __thiscall(ECX = the pool object at image address `pool_object`
    // (00E175B0), block), RET 4: returns a 14h-byte node or string header.
    virtual void node_pool_return_0043b0a0(std::uint32_t pool_object, void* block) = 0;
    // 0043B8B0, __thiscall(ECX = map, const PooledString* key, uint* bucket),
    // RET 8, EAX = node or null; the bucket index is written first.
    virtual void* property_map_find_0043b8b0(Raw map, const void* key, std::uint32_t* bucket) = 0;
    // 0041E870, __thiscall(ECX = 8-byte string, const char* text), RET 4, EAX = string.
    virtual void native_string_construct_0041e870(Raw string, const char* text) = 0;
    // [[object]+0](object, flags): the scalar deleting destructor in vtable slot 0.
    virtual void vcall_scalar_deleting_destructor_slot0(void* object, std::uint32_t flags) = 0;
    // 0043DCA0, __thiscall(ECX = vector, const uint32* value), RET 4: push_back.
    virtual void vector_push_back_0043dca0(Raw vector, const std::uint32_t* value) = 0;

    // --- world queries ----------------------------------------------------
    // 00470470 / 004704B0, __fastcall(ECX = hit record, 54h bytes): reset / destroy.
    virtual void hit_record_reset_00470470(Raw record) = 0;
    virtual void hit_record_destroy_004704b0(Raw record) = 0;
    // 0078CF20, __thiscall(ECX = world [[00E188A8]+19F0h], float x, float z), RET 8, ST0.
    virtual float game_world_sample_water_height_0078cf20(void* world, float x, float z) = 0;
    // 0042E630, no arguments, EAX = the spatial index singleton.
    virtual void* spatial_index_get_singleton_0042e630() = 0;
    // 0098ADD0, __thiscall(ECX = index; from*, to*, exclude, record*, kind filter), RET 14h, AL.
    virtual bool spatial_index_query_segment_0098add0(void* index, const float* from, const float* to,
                                                      void* exclude, Raw record,
                                                      std::uint32_t kind_filter) = 0;
    // [[entity]+5Ch](entity, kind), AL: the entity kind test.
    virtual bool vcall_entity_is_kind_5c(void* entity, std::uint32_t kind) = 0;

    // --- device classes and gun records -----------------------------------
    // 00440180, __thiscall(ECX = slot array {data, count, capacity}, int count), RET 4.
    virtual void device_class_registry_resize_slots_00440180(Raw slots, std::int32_t count) = 0;
    // 00441780, no arguments, EAX = the device-class registry (00E17BF4).
    virtual void* device_class_registry_get_singleton_00441780() = 0;
    // 00B6DC90, adjustor thunk: __thiscall(ECX = animated object, int track), RET 4, ST0.
    virtual float animation_length_00b6dc90(void* object, std::int32_t track) = 0;
    // [[object]+24h](object, int track, float time, int flag): set the play position.
    virtual void vcall_animation_set_time_24(void* object, std::int32_t track, float time,
                                             std::int32_t flag) = 0;
    // 00440380, __thiscall(ECX = std::map), EAX = a fresh head node (18h bytes,
    // isnil byte +11h).
    virtual void* map_buy_head_node_00440380(Raw map) = 0;
    // 00442310 (STL instantiation), __thiscall(ECX = destination std::map, const map*), RET 4.
    virtual void gun_record_map_copy_construct_00442310(Raw destination, Raw source) = 0;
    // 00927F30, __thiscall(ECX = unit, int role), RET 4, AL.
    virtual bool unit_is_local_player_role_00927f30(void* unit, std::int32_t role) = 0;
    // 00432650, no arguments, EAX = the global config.
    virtual void* global_config_get_singleton_00432650() = 0;
    // 0043F880, __thiscall(ECX = vector, int index), RET 4, EAX = &element (float).
    virtual float* config_vector_at_0043f880(Raw vector, std::int32_t index) = 0;
    // 0087D730, __thiscall(ECX = device, float amount), RET 4.
    virtual void unit_scale_damage_for_player_unit_0087d730(Raw device, float amount) = 0;

    // 00443090, __fastcall(ECX = signed class id, DL = retain selector), EAX = class.
    virtual void* device_class_resolve_from_lua_00443090(std::int32_t id, std::uint8_t flag) = 0;
    // [[class]+10h](class, 0) and [[class]+14h](class).
    virtual void vcall_device_class_activate_10(void* cls, std::uint32_t arg) = 0;
    virtual void vcall_device_class_finish_14(void* cls) = 0;

    // --- Lua readers -------------------------------------------------------
    // 0077E830, __thiscall(ECX = entity): the base AttachLuaSelf override.
    virtual void mission_entity_attach_lua_self_0077e830(Raw entity) = 0;
    // 009238A0, __thiscall(ECX = entity, LuaObject* out), RET 4: the saved Lua data.
    virtual void sentity_get_saved_lua_data_009238a0(Raw entity, LuaObjectStorage* out) = 0;
    // 004425C0, __thiscall(ECX = 14h-byte reader, LuaObject by value), RET 14h; the
    // callee owns (and destroys) the by-value argument.
    virtual void lua_reader_construct_004425c0(Raw reader, LuaObjectStorage* by_value) = 0;
    // 00BD8E20, __thiscall(ECX = reader, {dword kind, const void* key} by value), RET 8.
    virtual void lua_reader_enter_key_00bd8e20(Raw reader, std::uint32_t kind, const void* key) = 0;
    // 00BD6830, __thiscall(ECX = reader, {kind, key}, {kind, field*}), RET 10h.
    virtual void lua_reader_read_field_00bd6830(Raw reader, std::uint32_t key_kind, const void* key,
                                                std::uint32_t field_kind, void* field) = 0;
    // 00BD7A20, adjustor thunk __thiscall(ECX = reader): leave the key.
    virtual void lua_reader_leave_key_00bd7a20(Raw reader) = 0;
    // 00441A20, __thiscall(ECX = reader): the stack reader destructor.
    virtual void lua_reader_destroy_00441a20(Raw reader) = 0;

    // 00B66FA0, __thiscall(ECX = destination LuaObject, const LuaObject* source), RET 4.
    virtual void lua_object_copy_construct_00b66fa0(LuaObjectStorage* destination,
                                                    const LuaObjectStorage* source) = 0;
    // [[archive]+10h](archive, {kind, key} by value, {kind, field*} by value): the
    // archive's field serializer.
    virtual void vcall_archive_serialize_field_10(void* archive, std::uint32_t key_kind, const void* key,
                                                  std::uint32_t field_kind, void* field) = 0;
    // STL instantiations bound by address (vendor bodies, not reconstructed):
    // 0044DA70 vector<LuaObject> copy constructor (ECX = destination, source*), RET 4;
    // 00441210 vector<LuaObject> _Tidy (ECX = vector).
    virtual void lua_object_vector_copy_0044da70(Raw destination, Raw source) = 0;
    virtual void lua_object_vector_tidy_00441210(Raw vector) = 0;
    // 00BF6713, the CRT invalid-parameter report of a checked iterator.
    virtual void invalid_parameter_noinfo_00bf6713() = 0;

    // --- panel owner (dialogs) --------------------------------------------
    // 0044BA00, __thiscall(ECX = map, iterator* out, const PooledString* key), RET 8,
    // EAX = out ({container, node}).
    virtual void* panel_map_find_0044ba00(Raw map, void* out, const void* key) = 0;
    // 00451020, __thiscall(ECX = map, iterator* out, container, node), RET 0Ch.
    virtual void panel_map_erase_iterator_00451020(Raw map, void* out, void* container, void* node) = 0;
    // 004512E0, __thiscall(ECX = map, node* root), RET 4: frees a subtree.
    virtual void panel_map_destroy_subtree_004512e0(Raw map, void* root) = 0;
    // 00451920, __thiscall(ECX = map, const PooledString* key, const reader*), RET 8,
    // EAX = the sequence (inserted when missing).
    virtual void* panel_map_lookup_or_insert_00451920(Raw map, const void* key, Raw reader) = 0;
    // 004507D0, __thiscall(ECX = sequence): reads the sequence's Lua properties.
    virtual void panel_sequence_read_properties_004507d0(void* sequence) = 0;
    // 00435C40, __thiscall(ECX = string, const string* other), RET 4, AL.
    virtual bool native_string_equals_insensitive_00435c40(Raw string, Raw other) = 0;
    // 0041E350, __thiscall(ECX = string, const char* text), RET 4.
    virtual void native_string_assign_cstring_0041e350(Raw string, const char* text) = 0;
    // 005B68D0, __thiscall(ECX = [[00E198C4]+A4h]).
    virtual void panel_owner_stop_voice_005b68d0(void* voice_owner) = 0;

    // --- Lua state owners -------------------------------------------------
    // 00B66C00, __thiscall(ECX = owner storage, lua_State*), RET 4: borrowed owner.
    virtual void lua_state_owner_construct_borrowed_00b66c00(Raw owner, void* lua_state) = 0;
    // 00B65EC0, __thiscall(ECX = owner, const char* buffer, size), RET 8.
    virtual void lua_state_owner_do_buffer_00b65ec0(Raw owner, const char* buffer,
                                                    std::uint32_t size) = 0;
    // 00B669A0, __thiscall(ECX = owner): close.
    virtual void lua_state_owner_close_00b669a0(Raw owner) = 0;
    // 00B68100, __thiscall(ECX = table, LuaObject* out, const NativeString* key), RET 8.
    virtual void lua_object_get_by_native_string_00b68100(LuaObjectStorage* table, LuaObjectStorage* out,
                                                          Raw key) = 0;

    // --- segment 5 callees ----------------------------------------------------
    // [[entity]+114h](entity), EAX = its weapon director or null.
    virtual void* vcall_entity_weapon_director_114(void* entity) = 0;
    // 00835860, __thiscall(ECX = director, target, int force), RET 8.
    virtual void weapon_director_set_fire_target_00835860(void* director, void* target,
                                                          std::int32_t force) = 0;
    // 008D8A70, __thiscall(ECX = tokenizer; the image passes whatever ECX holds),
    // EAX = the peeked token text.
    virtual const char* scene_tokenizer_peek_token_008d8a70(void* tokenizer) = 0;
    // 00438E10, __fastcall(ECX = a, EDX = b), EAX = ordering (0 when equal).
    virtual std::int32_t cstring_compare_insensitive_00438e10(const char* a, const char* b) = 0;
    // 00425850, __thiscall(ECX = NativeString, const char* text), RET 4, AL.
    virtual bool native_string_equals_cstring_insensitive_00425850(Raw string, const char* text) = 0;
    // 008528B0, __thiscall(ECX = submarine, int level), RET 4.
    virtual void submarine_set_depth_level_008528b0(void* submarine, std::int32_t level) = 0;

    // --- segment 6 callees ----------------------------------------------------
    // 008E6430, __thiscall(ECX = modifiers, int index, unit), RET 8, ST0: the
    // powerup multiplier lookup (reached by a tail JMP).
    virtual float gameplay_modifiers_lookup_008e6430(Raw modifiers, std::int32_t index, void* unit) = 0;
    // [[object]+108h](object), EAX: the owning unit lookup.
    virtual void* vcall_owner_unit_108(void* object) = 0;

    // --- segment 7 callees ----------------------------------------------------
    // Import 00CE2220 InterlockedDecrement, EAX = the new value.
    virtual long interlocked_decrement_ce2220(volatile long* value) = 0;
    // [[object]+0](object) with no stack argument: the reference-count release.
    virtual void vcall_release_slot0(void* object) = 0;
    // 00BF65AC _free(block), __cdecl (thunk).
    virtual void free_00bf65ac(void* block) = 0;
    // 00485990, __thiscall(ECX = pointer array {data, count, capacity}, int count), RET 4.
    virtual void pod_pointer_array_resize_00485990(Raw array, std::int32_t count) = 0;

    // --- segment 9 callees ----------------------------------------------------
    // 00BF7CD1 `eh vector constructor iterator'(array, size, count, ctor, dtor), __stdcall;
    // 00BF7C6E `eh vector destructor iterator'(array, size, count, dtor), __stdcall.
    // CRT helpers; ctor/dtor are image code addresses.
    virtual void eh_vector_constructor_iterator_00bf7cd1(void* array, std::uint32_t size,
                                                         std::uint32_t count, std::uint32_t ctor,
                                                         std::uint32_t dtor) = 0;
    virtual void eh_vector_destructor_iterator_00bf7c6e(void* array, std::uint32_t size,
                                                        std::uint32_t count, std::uint32_t dtor) = 0;
    // 004C8310, __thiscall(ECX = counted pointer vector, int capacity), RET 4: reserve.
    virtual void effect_handle_vector_reserve_004c8310(Raw vector, std::int32_t capacity) = 0;
    // Import 00CE221C InterlockedIncrement.
    virtual long interlocked_increment_ce221c(volatile long* value) = 0;

    // 004C71C0, __fastcall(ECX = world): lays out the border-zone records.
    virtual void world_layout_border_zone_records_004c71c0(Raw world) = 0;
    // 00798C30, adjustor thunk __thiscall(ECX = view, unit), RET 4, AL.
    virtual bool view_contains_unit_00798c30(void* view, void* unit) = 0;
    // 0078E470, __thiscall(ECX = view), EAX = its unit.
    virtual void* view_unit_0078e470(void* view) = 0;
    // 004BEAD0, __fastcall(ECX = unit), EAX = the unit as a class-flag-0Fh plane or null.
    virtual void* unit_cast_if_class_flag0f_004bead0(void* unit) = 0;
    // 007B8AD0, __thiscall(ECX = plane), AL: squadron flight leader.
    virtual bool unit_is_squadron_flight_leader_007b8ad0(void* plane) = 0;

    // --- segment 11 callees ---------------------------------------------------
    // 00928630, __thiscall(ECX = entity, int registry_select, int id), RET 8.
    virtual void game_entity_construct_00928630(Raw entity, std::int32_t registry_select,
                                                std::int32_t id) = 0;
    // 004E7A10, __thiscall(ECX = std::list), EAX = a fresh head node.
    virtual void* list_buy_head_node_004e7a10(Raw list) = 0;
    // 00BF79F0 memset is bound directly. 006AF3D0, __thiscall(ECX = [[00E188A8]+21D8h],
    // NativeString by value), RET 8, AX = network id; the callee frees the string.
    virtual std::uint16_t entity_network_id_from_name_006af3d0(void* registry, Raw name_by_value) = 0;
    // [[entity]+98h](entity, a, [[00E188A8]+19CCh], b): place in world.
    virtual void vcall_entity_place_98(void* entity, std::uint32_t a, std::uint32_t world,
                                       std::uint32_t b) = 0;

    // 00928560 / 00928570, __fastcall(ECX = entity): the base register / unregister.
    virtual void game_entity_register_00928560(Raw entity) = 0;
    virtual void game_entity_unregister_00928570(Raw entity) = 0;
    // 00484540, __thiscall(ECX = unit list, value), RET 4: push_back.
    virtual void unit_list_push_back_00484540(Raw list, void* value) = 0;
    // 004837D0, __thiscall(ECX = unit list, node), RET 4: erase.
    virtual void unit_list_erase_node_004837d0(Raw list, void* node) = 0;

    // 0077EED0 / 0077E380, __thiscall(ECX = entity[, id]): the unit-owner entity
    // constructor (RET 4) and destructor.
    virtual void unit_owner_entity_construct_0077eed0(Raw entity, std::uint32_t id) = 0;
    virtual void unit_owner_entity_destruct_0077e380(Raw entity) = 0;
    // 00875890, __thiscall(ECX = tick registration, owner, int flag), RET 8.
    virtual void tick_registration_construct_00875890(void* registration, void* owner,
                                                      std::int32_t flag) = 0;
    // 00875490, __thiscall(ECX = tick element or null): destructor.
    virtual void unit_tick_element_destruct_00875490(void* element) = 0;
    // 0049F9B0 / 0049F910, __thiscall(ECX = member at +344h, or null for 0049F910):
    // construct / destruct of the convoy's +344h member.
    virtual void convoy_member344_construct_0049f9b0(void* member) = 0;
    virtual void convoy_member344_destruct_0049f910(void* member) = 0;
    // STL instantiations bound by address: 004E8150 list clear (ECX = list);
    // 004F02E0 tree erase(ret*, first {tree, node}, last {tree, node}), RET 14h;
    // 004E7AE0 tree head node (isnil byte +15h); 004E79F0 list head node.
    virtual void list_clear_004e8150(Raw list) = 0;
    virtual void tree_erase_range_004f02e0(Raw tree, void* ret, void* first_tree, void* first_node,
                                           void* last_tree, void* last_node) = 0;
    virtual void* tree_buy_head_node_004e7ae0(Raw tree) = 0;
    virtual void* list_buy_head_node_004e79f0(Raw list) = 0;

    // 004C1130, no arguments, EAX = the frame job pool.
    virtual void* frame_job_pool_get_singleton_004c1130() = 0;
    // [[pool+4]+10h](pool+4), AL: the pool's deferral predicate.
    virtual bool vcall_job_pool_defers_10(void* pool_interface) = 0;
    // 004F03C0, __fastcall(ECX = property bag, EDX = parent, one stack dword), RET 4.
    virtual void scene_unit_defer_hierarchy_004f03c0(void* bag, std::uint32_t parent,
                                                     std::uint32_t arg) = 0;

    // --- segments 12..14 callees ---------------------------------------------
    // [[unit]+21Ch](unit, int signal).
    virtual void vcall_unit_signal_21c(void* unit, std::int32_t signal) = 0;
    // 006952A0 / 00694A60, __fastcall(ECX = observed object, EDX = observer).
    virtual void observer_unregister_pair_006952a0(void* observed, void* observer) = 0;
    virtual void observer_register_pair_00694a60(void* observed, void* observer) = 0;
    // 0042CF10, __stdcall(float), RET 4, ST0: asin with clamping.
    virtual float geometry_asin_clamped_0042cf10(float value) = 0;
    // [[entity]+16Ch](entity, const char* command, arg), AL: command allowed.
    virtual bool vcall_entity_accepts_command_16c(void* entity, const char* command,
                                                  std::uint32_t arg) = 0;

    // 00414DB0, __fastcall(ECX = entity): refresh the world pose.
    virtual void entity_pose_refresh_world_00414db0(void* entity) = 0;
    // 00524DC0, STL vector erase(ret*, first {vec, ptr}, last {vec, ptr}), RET 14h.
    virtual void vector_erase_range_00524dc0(Raw vector, void* ret, void* first_vec, void* first,
                                             void* last_vec, void* last) = 0;
    // 004BEC00, no arguments (callers push the next call's arguments first), EAX =
    // the input manager; 00A933F0, __thiscall(ECX = manager, context, level), RET 8.
    virtual void* input_manager_get_singleton_004bec00() = 0;
    virtual void input_manager_set_context_level_00a933f0(void* manager, std::int32_t context,
                                                          std::int32_t level) = 0;

    // 005470C0, __thiscall(ECX = weapon group screen [[00E198C4]+50h]): release.
    virtual void hud_weapon_group_screen_release_005470c0(void* screen) = 0;
    // 00549260, __thiscall(ECX = weapon group screen, unit, mode), RET 8.
    virtual void hud_weapon_group_screen_bind_unit_00549260(void* screen, void* unit,
                                                            std::uint32_t mode) = 0;
    // 0077C470, __thiscall(ECX = unit, int message, int arg), RET 8.
    virtual void unit_post_message_0077c470(void* unit, std::int32_t message, std::int32_t arg) = 0;
    // 00694AF0, __fastcall(ECX = observed, EDX = observer), AL: pair registered.
    virtual bool observer_is_pair_registered_00694af0(void* observed, void* observer) = 0;
    // 005198F0, __thiscall(ECX = bomber view): make sure its camera exists.
    virtual void bomber_view_ensure_camera_005198f0(Raw view) = 0;
    // 006DB3F0, __fastcall(ECX = root element, EDX = std::vector<element*>* out).
    virtual void gui_element_collect_descendants_006db3f0(void* root, Raw out_vector) = 0;
    // [[element]+54h](element, float4* out) / [[element]+50h](element, const float4*).
    virtual void vcall_element_get_color_54(void* element, float* color) = 0;
    virtual void vcall_element_set_color_50(void* element, const float* color) = 0;
    // 006DBAC0, __fastcall(ECX = element, DL = flag; a, b, float level), RET 0Ch.
    virtual void gui_element_apply_highlight_pulse_006dbac0(void* element, std::uint8_t flag,
                                                            std::uint32_t a, std::uint32_t b,
                                                            float level) = 0;

    // --- scene creators -------------------------------------------------------
    // 006EAFE0, __fastcall(ECX = counted holder*, EDX = bullet id), EAX = the holder.
    virtual void** bullet_class_acquire_006eafe0(void** holder, std::int32_t id) = 0;
    // [[class]+20h](class, 0, party, const float* b+30h, const float* b+20h, 0, 0, 0), EAX.
    virtual void* vcall_bullet_class_spawn_20(void* cls, std::uint32_t a0, std::uint32_t party,
                                              const std::uint8_t* position, const std::uint8_t* direction,
                                              std::uint32_t a4, std::uint32_t a5, std::uint32_t a6) = 0;
    // 0047B660, __thiscall(ECX = entity, word id), RET 4, EAX = entity: the Path body.
    virtual void* path_construct_0047b660(Raw entity, std::uint16_t id) = 0;
    // 004E6480, __thiscall(ECX = the landscape member at +1E4h): its constructor.
    virtual void landscape_member_construct_004e6480(Raw member) = 0;
    // 008F2260, __thiscall(ECX = property bag, const char* key), RET 4, EAX = property or null.
    virtual void* scene_property_bag_find_008f2260(void* bag, const char* key) = 0;
    // 008F0DF0, __thiscall(ECX = property), EAX = its string value.
    virtual const char* scene_property_get_string_value_008f0df0(void* property) = 0;
    // 00851CB0, __thiscall(ECX = const NativeString* type name), EAX = stationary class.
    virtual void* stationary_class_get_or_create_00851cb0(Raw type_name) = 0;
    // [[class]+4](class), EAX = a new entity of the class.
    virtual void* vcall_class_create_entity_4(void* cls) = 0;
    // 004B3F10, __thiscall(ECX = const NativeString* type name, int flag), RET 4, EAX = wreck class.
    virtual void* wreck_class_get_or_create_activated_004b3f10(Raw type_name, std::int32_t flag) = 0;
    // 0047BB70, __thiscall(ECX = [[00E188A8]+21D0h], class), RET 4, EAX = entity.
    virtual void* wreck_factory_create_0047bb70(void* factory, void* cls) = 0;
    // 0048D6C0, __thiscall(ECX = entity, const char* name), RET 4.
    virtual void scene_entity_set_name_0048d6c0(void* entity, const char* name) = 0;

    // --- segment 10 callees ---------------------------------------------------
    // 004CD0F0, __thiscall(ECX = game, char hide, byte allow_simulation, char p4), RET 0Ch.
    virtual void game_set_cinematic_mode_004cd0f0(Raw game, std::uint8_t hide, std::uint8_t allow,
                                                  std::uint8_t p4) = 0;
    // 00425D10, no arguments, EAX = the menu prompt screen singleton.
    virtual void* menu_command_screen_get_singleton_00425d10() = 0;
    // 00532A20, __thiscall(ECX = menu, int slot), RET 4.
    virtual void menu_prompt_screen_dismiss_slot_00532a20(void* menu, std::int32_t slot) = 0;
    // 006882A0, __thiscall(ECX = [00E198B4]).
    virtual void multi_menu_close_pause_interface_006882a0(void* menu) = 0;
    // 00531B00, __thiscall(ECX = menu; 1, title*, 2, callback, 0, text*, float 0.0f, 0,
    // NativeString by value (callee-owned), 1), RET 2Ch.
    virtual void menu_prompt_screen_raise_00531b00(void* menu, std::int32_t a1, Raw title, std::int32_t a3,
                                                   std::uint32_t callback, std::int32_t a5, Raw text,
                                                   float a7, std::int32_t a8, Raw by_value_string,
                                                   std::int32_t a10) = 0;

    // 0087BF80, __thiscall(ECX = entity): the base SEntityInit slot +A4h.
    virtual void unit_tickable_entity_init_slot_a4_0087bf80(Raw entity) = 0;
    // 00B67720, __thiscall(ECX = table, LuaObject* out, int index), RET 8, EAX = out.
    virtual LuaObjectStorage* lua_object_get_by_index_00b67720(LuaObjectStorage* table, LuaObjectStorage* out,
                                                               std::int32_t index) = 0;
    // 00927B40, __thiscall(ECX = entity, LuaObject* out), RET 4, EAX = out.
    virtual LuaObjectStorage* mission_entity_lua_self_00927b40(Raw entity, LuaObjectStorage* out) = 0;
    // 00B675D0, __thiscall(ECX = table, const NativeString* name, const LuaObject* value), RET 8.
    virtual void lua_object_set_object_00b675d0(LuaObjectStorage* table, Raw name,
                                                const LuaObjectStorage* value) = 0;

    // The 400h-byte static format buffer at 00E177E8.
    virtual char* format_buffer_00e177e8() = 0;
};

// --- segment 0 -------------------------------------------------------------
float abs_float_00412d90(float value);
void pointer_array_reverse_00412e80(Raw array);
std::int32_t int_clamp_00412f60(const std::int32_t* value, const std::int32_t* low,
                                const std::int32_t* high);
int format_string_100_00412f80(char* dst, const char* format, ...);
std::int32_t int_min_00412fa0(const std::int32_t* a, const std::int32_t* b);

// --- segment 2 -------------------------------------------------------------
void camera_set_angles_0042aa00(Raw camera, float rho, float theta, char snap);
void camera_clamp_zoom_0042c450(Raw mover, float low);
void camera_mover_set_shake_0042c4c0(Raw mover, float a0, float a1, float a2, float f0, float f1,
                                     float f2);
float* camera_eye_offset_0042cd00(Raw camera, float* out);
bool camera_mover_frustum_contains_point_0042e3d0(Raw mover, const float* point, SweepAHost& host);
void entity_set_name_004313d0(Raw entity, Raw name, std::uint32_t unused, SweepAHost& host);
Raw four_vector_record_construct_00432380(Raw record);
void camera_mover_step_base_00435410(Raw mover, float dt, SweepAHost& host);
void party_rows_reserve_00435960(Raw rows, std::int32_t capacity, SweepAHost& host);
void party_rows_resize_004359e0(Raw rows, std::int32_t count, SweepAHost& host);
void lua_global_table_write_archive_00435f40(Raw owner, SweepAHost& host);
void profile_manager_request_save_checkpoint_00438a00(Raw manager, const void* source,
                                                     SweepAHost& host);
Raw cstring_key_construct_owned_00438f90(Raw key, const char* text, SweepAHost& host);
float* vector3_transform_project_point_00439820(float* out, const float* point, const void* matrix,
                                               float* inverse_w, SweepAHost& host);
float* vector2_set_random_unit_direction_00439a00(float* out, SweepAHost& host);
char* string_format_static_00439ac0(SweepAHost& host, const char* format, ...);
bool camera_unproject_screen_ray_00439f50(void* camera, float* out_near, float screen_x,
                                          float screen_y, float* out_far, SweepAHost& host);
bool camera_pick_world_point_on_horizontal_plane_0043a290(void* camera, float* out, float screen_x,
                                                         float screen_y, float plane_y,
                                                         float* ray, SweepAHost& host);
bool surface_probe_kind6_above_water_0043b9c0(const float* point, SweepAHost& host);
Raw pooled_string_scalar_delete_0043bfc0(Raw string, std::uint32_t flags, SweepAHost& host);
void pooled_string_ptr_delete_reset_0043cfa0(void** slot, SweepAHost& host);
void property_map_node_destroy_0043d2d0(Raw map, Raw node, SweepAHost& host);
bool property_map_erase_0043d4d0(Raw map, const void* key, SweepAHost& host);
bool property_map_erase_by_name_0043d870(Raw owner, const char* name, SweepAHost& host);
Raw observer_set_construct_0043df40(Raw set, std::uint32_t a, SweepAHost& host);
Raw observer_set_construct_0043df90(Raw set, std::uint32_t a, std::uint32_t b, SweepAHost& host);
Raw observer_set_construct_0043dff0(Raw set, std::uint32_t a, std::uint32_t b, std::uint32_t c,
                                    SweepAHost& host);
Raw observer_set_construct_0043e060(Raw set, std::uint32_t a, std::uint32_t b, std::uint32_t c,
                                    std::uint32_t d, SweepAHost& host);
// --- segment 3 -------------------------------------------------------------
void gun_animation_sync_to_clock_0043ff40(Raw gun, std::uint32_t unused, SweepAHost& host);
float vehicle_class_underwater_armour_default_004407a0(Raw vehicle_class);
void device_class_slots_destroy_00440ca0(Raw slots, SweepAHost& host);
void device_class_slots_destroy_004411f0(Raw slots, SweepAHost& host);
void device_class_registry_clear_00442170(SweepAHost& host);
Raw gun_record_construct_004427a0(Raw record, SweepAHost& host);
Raw gun_record_copy_construct_00442840(Raw record, Raw source, SweepAHost& host);
void gun_records_reserve_00442940(Raw records, std::int32_t capacity, SweepAHost& host);
void gun_records_resize_00442a30(Raw records, std::int32_t count, SweepAHost& host);
void device_apply_scaled_damage_00442ae0(Raw device, float amount, SweepAHost& host);
void gun_records_destroy_00442b70(Raw records, SweepAHost& host);
bool gun_class_base20_is_kind_of_00442c00(std::int32_t id);
bool bomb_platform_class_is_kind_of_00442c70(std::int32_t id);
bool multiple_bomb_platform_class_is_kind_of_00442d00(std::int32_t id);
bool rfs_gun_class_is_kind_of_00442db0(std::int32_t id);
bool rt_gun_class_is_kind_of_00442e50(std::int32_t id);
bool st_gun_class_is_kind_of_00442ee0(std::int32_t id);
bool depth_charge_launcher_class_is_kind_of_00442fa0(std::int32_t id);
bool catapult_class_is_kind_of_00443010(std::int32_t id);
void device_entity_place_in_world_lua_004426c0(Raw entity, SweepAHost& host);
void device_entity_set_lua_class_00440e10(Raw entity, SweepAHost& host);
void* device_class_resolve_and_activate_00443490(std::int32_t id, std::uint8_t flag, SweepAHost& host);
// --- segment 4 -------------------------------------------------------------
void archive_serialize_picture_field_0044a3c0(void* field, void* archive, SweepAHost& host);
Raw lua_reader_copy_construct_0044e670(Raw reader, Raw source, SweepAHost& host);
void panel_owner_kill_dialog_004514a0(Raw owner, Raw name_by_value, SweepAHost& host);
void panel_owner_reset_004518b0(Raw owner, SweepAHost& host);
void panel_owner_start_dialog_from_lua_00451a90(Raw owner, Raw name_by_value,
                                                LuaObjectStorage* table_by_value, SweepAHost& host);
void session_message_start_dialog_apply_00451e50(Raw owner, Raw message, SweepAHost& host);
// --- segment 5 -------------------------------------------------------------
void native_string_at4_destroy_storage_004567b0(Raw record, SweepAHost& host);
void native_string_at4_destroy_storage_00456d10(Raw record, SweepAHost& host);
void scene_record_eight_strings_destroy_0045cfc0(Raw record, SweepAHost& host);
bool scene_token_equals_0045c440(void* tokenizer, const char* text, SweepAHost& host);
void fire_target_holder_apply_004643a0(Raw holder, void* entity, SweepAHost& host);
void entity_command_set_submarine_depth_004654d0(Raw command, void* entity, SweepAHost& host);
// --- segment 6 -------------------------------------------------------------
Raw unit_reference_construct_004684f0(Raw reference, void* unit, SweepAHost& host);
float gameplay_modifiers_powerup_multiplier_00470440(Raw modifiers, std::int32_t index, void* unit,
                                                     SweepAHost& host);
void hit_record_destroy_004704b0(Raw record, SweepAHost& host);
std::uint32_t hit_record_owning_unit_004704e0(Raw record, SweepAHost& host);
float random_in_range_0047b7f0(Raw range, SweepAHost& host);
bool unit_uses_small_turn_roll_limit_0047b880(void* unit, SweepAHost& host);
struct Blob40 {
    alignas(4) std::uint8_t bytes[0x40];
};
bool blob40_greater_by_float8_0047b9f0(Blob40 a, Blob40 b);
// --- segment 7 -------------------------------------------------------------
void traffic_path_set_destruct_0048ee90(Raw set, SweepAHost& host);
// --- segment 9 -------------------------------------------------------------
bool unit_is_viewed_by_player_004bfed0(void* unit, SweepAHost& host);
void world_replace_border_bounds_004ca760(Raw world, const float* lo, const float* hi, SweepAHost& host);
Raw scene_record_player_slot_construct_004c9800(Raw record, SweepAHost& host);
void scene_record_player_slot_destroy_004c9820(Raw record, SweepAHost& host);
void effect_handle_vector_push_back_004caf50(Raw vector, void* const* handle, SweepAHost& host);
void effect_handle_vector_resize_004cb160(Raw vector, std::int32_t count, SweepAHost& host);
Raw end_of_mission_movie_construct_004cc3f0(Raw record, Raw name, SweepAHost& host);
// --- segment 10 ------------------------------------------------------------
void game_show_game_closed_004d7d50(Raw game, std::uint8_t suppress_prompt, SweepAHost& host);

// --- segments 10 and 11: world-list registration ----------------------------
void movie_cam_lookat_unregister_004e79b0(Raw entity, SweepAHost& host);
void landing_point_register_004e9670(Raw entity, SweepAHost& host);
void landing_point_unregister_004e9690(Raw entity, SweepAHost& host);
void* landscape_get_collision_node_004f1390(Raw landscape);
void landscape_register_004f1400(Raw entity, SweepAHost& host);
void landscape_unregister_004f1420(Raw entity, SweepAHost& host);
void spawn_point_register_004f1910(Raw entity, SweepAHost& host);
void spawn_point_unregister_004f1940(Raw entity, SweepAHost& host);
void land_convoy_register_004f25d0(Raw entity, SweepAHost& host);
void land_convoy_unregister_004f2600(Raw entity, SweepAHost& host);
float* color_static_white_004f9d30(SweepAHost& host);

// --- segment 11 ------------------------------------------------------------
Raw landing_point_construct_body_004e9520(Raw entity, std::uint16_t id, SweepAHost& host);
void* landing_point_create_from_scene_004e9d40(const char* name, std::uint32_t a, std::uint32_t b,
                                               std::uint32_t unused3, std::uint32_t unused4,
                                               SweepAHost& host);
Raw spawn_point_construct_body_004f1610(Raw entity, std::uint32_t id, SweepAHost& host);
void spawn_point_destruct_004f19a0(Raw entity, SweepAHost& host);
Raw land_convoy_construct_004f2410(Raw entity, std::uint32_t id, SweepAHost& host);
void land_convoy_destruct_004f2660(Raw entity, SweepAHost& host);
void* landscape_create_from_scene_004f1460(const char* name, std::uint32_t a, std::uint32_t b,
                                           std::uint32_t unused3, std::uint32_t unused4, SweepAHost& host);
Raw landscape_construct_004f11c0(Raw entity, std::uint16_t id, SweepAHost& host);
void* water_mine_create_from_scene_004e9f80(const char* name, std::uint32_t a, const std::uint8_t* b,
                                            void* bag, std::uint32_t unused4, SweepAHost& host);
void* path_create_from_scene_004ea650(const char* name, std::uint32_t a, const void* b,
                                      std::uint32_t unused3, std::uint32_t unused4, SweepAHost& host);
void* land_convoy_create_from_scene_004f2700(const char* name, std::uint32_t a, std::uint32_t b, void* bag,
                                             std::uint32_t unused4, SweepAHost& host);
void* spawn_point_create_from_scene_004f1a60(const char* name, std::uint32_t a, std::uint32_t b,
                                             std::uint32_t unused3, std::uint32_t unused4, SweepAHost& host);
void scene_unit_place_or_defer_004f04c0(void* entity, void* bag, std::uint32_t a, std::uint32_t b,
                                        SweepAHost& host);
bool circle2f_contains_point_004f2fd0(const float* circle, const float* point);
void* stationary_create_from_scene_004f0be0(const char* name, std::uint32_t a, std::uint32_t b,
                                            void* bag, std::uint32_t unused4, SweepAHost& host);
void* wreck_create_from_scene_004f0cc0(const char* name, std::uint32_t a, std::uint32_t b, void* bag,
                                       std::uint32_t unused4, SweepAHost& host);
// --- segments 12..14 -------------------------------------------------------
std::uint32_t hud_weapon_group_state_is_engaged_00518f30(Raw state);
void hud_pilot_screen_pulse_attached_kind25_00519020(Raw screen, SweepAHost& host);
void hud_view_bind_unit_0051e730(Raw view, void* unit, SweepAHost& host);
void hud_bomber_view_bind_unit_00519b00(Raw view, void* unit, SweepAHost& host);
void front_end_screen_reset_descendant_colors_0051c3a0(void* root, std::uint8_t flag, SweepAHost& host);
void direction_to_pitch_yaw_00521370(const float* direction, float* pitch, float* yaw,
                                     SweepAHost& host);
std::uint32_t unit_pick_row_value_00521f10(Raw rows, std::int32_t index);
std::uint8_t unit_pick_read_position_00521fd0(Raw entry, float* out);
const void* unit_pick_attackmove_color_00522680(void* entity, std::uint32_t arg, SweepAHost& host);
Raw unit_distance_record_set_00522860(Raw record, void* unit, SweepAHost& host);
void hud_screen_reset_selection_005251c0(Raw screen, SweepAHost& host);
void control_layout_screen_set_payload_00527c80(Raw screen, std::uint8_t a, std::uint8_t b,
                                                std::uint8_t c, std::uint8_t d);
std::int32_t camera_project_world_to_screen_xy_0043a870(const float* world, float* out_xy,
                                                       std::int32_t mode, char clamp_depth,
                                                       SweepAHost& host);

}  // namespace bsp::cc13_sweepA
