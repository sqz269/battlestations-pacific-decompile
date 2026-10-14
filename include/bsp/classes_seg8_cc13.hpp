#pragma once
// Packet cc13-seg8: segment 8 (00491330..004BA0D0) class data and loaders. The
// ship part, plane part and wreck classes (constructors, destructors, instance
// factories, Lua global teardown, registries and the ship/plane part loaders),
// the soldier entity constructor and global teardown, and small singleton and
// reference helpers. Per-frame unit behaviour in this segment is left to cc9.
// docs/CC13_SEG8.md lists every function with its ABI and uncertainty.
//
// Objects are reached through Raw (byte offsets into the Win32 layout); every
// call the image makes goes through a Seg8Host method named for its target,
// except memset/memcpy, which are called directly. Descriptive names are
// hypotheses, not recovered symbols.

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_sys {
struct LazySingletonHost;
}

namespace bsp::classes_seg8_cc13 {

static_assert(sizeof(void*) == 4, "segment 8 objects are modelled in their Win32 layout");

// Raw view of an image object: at<T>(off) is the field at that byte offset.
struct Raw {
    std::uint8_t* p;
    template <class T>
    T& at(std::size_t off) const noexcept { return *reinterpret_cast<T*>(p + off); }
    void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
};

// An archive value as the writers pass it by value: {tag, value}; tag 0 is a
// string pointer, 1 an int, 2 a float (bit pattern).
struct TaggedValue {
    std::uint32_t tag;
    std::uint32_t value;
};

// A 14h-byte Lua object handle on the native stack (opaque here).
struct LuaObject {
    std::uint8_t storage[0x14];
};

struct Seg8Host {
    virtual ~Seg8Host() = default;
    // Image globals (dword storage) and image data addresses.
    virtual std::uint32_t& global_u32(std::uint32_t address) = 0;
    virtual void* image_address(std::uint32_t address) = 0;
    // Allocation.
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void operator_delete_00bf65ac(void* block) = 0;
    // 006B8AD0 BSP_LuaMachine_RunString, __thiscall(machine, text, 0, 0, 2), RET 10h.
    virtual std::int32_t lua_run_string_006b8ad0(void* machine, const char* text, std::int32_t a,
                                                 std::int32_t b, std::int32_t c) = 0;
    // Class-entity and named-class bases (ECX = object).
    virtual void class_entity_construct_base_00489f30(Raw entity) = 0;
    virtual void class_entity_destruct_base_00487380(Raw entity) = 0;
    virtual void named_class_construct_base_00489e60(Raw cls) = 0;
    virtual void named_class_destruct_base_00489e80(Raw cls) = 0;
    // Reference counting: InterlockedDecrement ([00CE2220]) and vtable slot 0 with no arguments.
    virtual long interlocked_decrement_ce2220(long* addend) = 0;
    virtual void object_slot_00(void* object) = 0;
    // 00867B10, ECX = point effect: stop its children.
    virtual void point_effect_stop_children_00867b10(void* effect) = 0;
    // Scene node: vtable +38h(const matrix*) and 00B6DA70(ECX = node, float factor, int 1).
    virtual void scene_node_slot_38(void* node, const void* matrix) = 0;
    virtual void scene_node_set_visibility_factor_00b6da70(void* node, float factor, std::int32_t flag) = 0;
    // Archive writer: 00924D90 (ECX = entity, archive), then archive vtable slots.
    virtual void entity_save_base_00924d90(Raw entity, void* archive) = 0;
    virtual void archive_begin_slot_04(void* archive, TaggedValue key) = 0;
    virtual void archive_end_slot_08(void* archive) = 0;
    virtual void archive_write_slot_0c(void* archive, TaggedValue key, TaggedValue value) = 0;
    // Sized string pool: 00419CC0 returns the pool, 00BD1510 (ECX = pool) takes the block back.
    virtual void* string_pool_00419cc0() = 0;
    virtual void string_pool_return_00bd1510(void* pool, void* block, std::uint32_t bytes,
                                             std::int32_t count) = 0;
    // 0041DD40, __thiscall(string, length, 1), RET 8: resize a native string.
    virtual void native_string_resize_0041dd40(Raw string, std::uint32_t length, std::int32_t flag) = 0;
    // Class registries (std::map shaped): the head-node buyers, the wreck range erase,
    // the getters and the operator[] lookups (ECX = registry+4, key string), EAX = &value.
    virtual void* tree_buy_head_node_004b1dd0(Raw tree) = 0;
    virtual void* tree_buy_head_node_004ac2f0(Raw tree) = 0;
    virtual void tree_erase_range_004b3170(Raw tree, void* result, void* first, Raw tree_again, void* last) = 0;
    virtual Raw ship_part_registry_004aeb30() = 0;
    virtual Raw plane_part_registry_004a9890() = 0;
    virtual void** ship_part_map_subscript_004ae7f0(Raw map, Raw key) = 0;
    virtual void** plane_part_map_subscript_004a95f0(Raw map, Raw key) = 0;
    // Lua objects: 00B67980 (ECX = game+1A0Ch, out), 00B67800 (ECX = source, out, name),
    // 00B67700 (ECX = object) destruct.
    virtual void lua_get_globals_00b67980(void* owner, LuaObject* out) = 0;
    virtual void lua_get_by_name_00b67800(LuaObject* source, LuaObject* out, const char* name) = 0;
    virtual void lua_object_destruct_00b67700(LuaObject* object) = 0;
    // Class vtable slots +4 (read fields from a Lua table) and +0Ch (finish loading).
    virtual void class_slot_04(void* cls, LuaObject* table) = 0;
    virtual void class_slot_0c(void* cls) = 0;
    // Reference releases.
    virtual void node_unlink_00b6dfa0(void* node) = 0;
    virtual void scene_ref_release_00b72250(void* ref) = 0;
    virtual void mission_entity_kill_00926d90(void* entity, std::uint32_t edx_argument) = 0;
    // 00951560, ECX = table: reset an entity id table.
    virtual void entity_id_table_reset_00951560(Raw table) = 0;
    // 0076FAD0, __thiscall(session, int), RET 4.
    virtual void session_set_mode_0076fad0(Raw session, std::int32_t mode) = 0;
};

// --- Lua global teardown (cdecl, RET) -----------------------------------
void plane_part_class_clear_lua_global_004a7120(Seg8Host& host);
void soldier_class_clear_lua_global_004af930(Seg8Host& host);
void wreck_class_clear_lua_global_004b1890(Seg8Host& host);

// --- entities --------------------------------------------------------------
void plane_part_refresh_visibility_004a7650(Raw part, Seg8Host& host);
void ship_part_refresh_visibility_004ac0b0(Raw part, Seg8Host& host);
Raw plane_part_construct_004a7b50(Raw part, std::uint32_t source, Seg8Host& host);
Raw ship_part_construct_004ac870(Raw part, std::uint32_t source, Seg8Host& host);
Raw soldier_construct_004af7e0(Raw soldier, Seg8Host& host);
Raw wreck_construct_004b1ee0(Raw wreck, Seg8Host& host);
void plane_part_destruct_004a82c0(Raw part, Seg8Host& host);
void ship_part_destruct_004acfa0(Raw part, Seg8Host& host);
void wreck_destruct_004b2590(Raw wreck, Seg8Host& host);
void wreck_save_archive_004b2680(Raw wreck, void* archive, Seg8Host& host);

// --- classes ---------------------------------------------------------------
Raw plane_part_class_construct_004a7bf0(Raw cls, Seg8Host& host);
Raw ship_part_class_construct_004ac910(Raw cls, Seg8Host& host);
Raw wreck_class_construct_004b1f70(Raw cls, Seg8Host& host);
void plane_part_class_destruct_004a7c10(Raw cls, Seg8Host& host);
void ship_part_class_destruct_004ac990(Raw cls, Seg8Host& host);
void wreck_class_destruct_004b1f90(Raw cls, Seg8Host& host);
void* plane_part_class_create_instance_004a7cb0(Raw cls, Seg8Host& host);
void* ship_part_class_create_instance_004aca30(Raw cls, Seg8Host& host);
void* plane_part_class_load_from_globals_004a9960(Raw name, Seg8Host& host);
void* ship_part_class_load_from_globals_004aec00(Raw name, Seg8Host& host);

// --- registries ------------------------------------------------------------
Raw ship_part_class_registry_construct_004ae990(Raw registry, Seg8Host& host);
Raw wreck_class_registry_construct_004b3b50(Raw registry, Seg8Host& host);
void wreck_class_registry_destruct_004b3bc0(Raw registry, Seg8Host& host);
void* ship_part_class_registry_get_004aeb30(void*& global_00e187e8, cc13_sys::LazySingletonHost& host);
void* wreck_class_registry_get_004b3c50(void*& global_00e187f8, cc13_sys::LazySingletonHost& host);

// --- string releases, singleton bases and references -----------------------
void native_string_release_storage_004a83c0(Raw string, Seg8Host& host);
void native_string_release_storage_004ad150(Raw string, Seg8Host& host);
void native_string_release_storage_004b2820(Raw string, Seg8Host& host);
void hint_tracker_destruct_singleton_base_004b4e00(Raw object, Seg8Host& host);
void singleton_00f8a71c_destruct_base_004b4e20(Raw object, Seg8Host& host);
void singleton_00e19a64_destruct_base_004b4eb0(Raw object, Seg8Host& host);
void singleton_00f8998c_destruct_base_004b4ed0(Raw object, Seg8Host& host);
bool win32_platform_consume_close_request_004b4560(Raw platform) noexcept;
void entity_id_table_pair_reset_004b4f40(Raw pair, Seg8Host& host);
void participant_record_set_claimed_004b5670(Raw record, std::uint8_t value) noexcept;
void participant_record_set_gate_byte9_004b5680(Raw record, std::uint8_t value) noexcept;
void session_clear_flag_27d_and_set_local_mode_004b6210(Raw session, Seg8Host& host);
void* world_class_list_head_004b79f0(Raw world) noexcept;
void node_ref_unlink_and_clear_004b81c0(Raw ref, Seg8Host& host);
void node_ref_unlink_and_clear_004b83b0(Raw ref, Seg8Host& host);
void native_scene_ref_release_and_clear_004b83d0(Raw ref, Seg8Host& host);
void mission_entity_ref_kill_and_clear_004b8470(Raw ref, std::uint32_t edx_in, Seg8Host& host);

}  // namespace bsp::classes_seg8_cc13
