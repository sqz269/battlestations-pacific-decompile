#pragma once
// cc13_seg87: the LuaObject helpers, state-owner DoBuffer/DoFile entries and the
// unreferenced script-cache instance methods of segment 87 (00B65BA0..00B6D1B0).
// Evidence, ABI and uncertainty per function: docs/CC13_SEG87.md.
//
// Every routine works on the actual native storage already modelled in
// bsp/native_lua_objects.hpp (14h object, 4C8h owner) and calls the linked
// stock Lua 5.1.1 API at the same points as the binary. Each Lua call reloads
// object->owner->state, as the listings do. Names are hypotheses, not recovered
// symbols. Reconstructed and build-tested only; not fixture- or game-validated.

#include "bsp/locale_text_lookup.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/native_lua_objects.hpp"
#include "bsp/native_lua_bootstrap.hpp"
#include "bsp/native_string.hpp"

#include <cstdint>

struct lua_State;

namespace bsp {

// 00B65C30, __thiscall(ECX = float plane[4]), stack const float matrix[16], RET 4.
// n' = n * R (rows 0..2 of the 4x4 row-major matrix), t' = T * R^T through the
// 3x3 helper 00B65AC0, d' = d - n.t' with the dot rounded to float32 first.
void transform_plane_by_matrix4x3_00b65c30(float plane[4], const float matrix[16]);

// 00B65E20, __thiscall(ECX = owner), stack lua_State*, RET 4. Writes ONLY the
// owns byte (0) and state word; the tracking slots are not initialised, unlike
// 00B66C00. Then registers the application's DoFile (00B69E00) with no upvalue.
NativeLuaStateStorage& construct_native_lua_owner_with_dofile_00b65e20(
    NativeLuaStateStorage& owner, lua_State* state, int (*do_file_00b69e00)(lua_State*));
// 00B69F20, __thiscall(ECX = owner), RET. The same DoFile registration only.
void register_native_lua_dofile_00b69f20(NativeLuaStateStorage& owner,
    int (*do_file_00b69e00)(lua_State*));
// 00B65EC0, __thiscall(ECX = owner), stack (buffer, size), RET 8. Load status
// ignored; lua_call(0, LUA_MULTRET) is unprotected.
void native_lua_do_buffer_00b65ec0(NativeLuaStateStorage& owner, const char* buffer,
    std::uint32_t size);
// 00B65EF0, same ABI, EAX 0 on success and 1 on load or run failure. The error
// value and any results stay on the stack.
std::int32_t native_lua_do_buffer_protected_00b65ef0(NativeLuaStateStorage& owner,
    const char* buffer, std::uint32_t size);

// ECX = object for every LuaObject routine below.
// 00B65F70, RET, AL = kind != 0.
bool native_lua_is_bound_00b65f70(const NativeLuaObjectStorage&) noexcept;
// 00B65F80, RET, EAX: kind 0 -> -1, kind 2 -> lua_type, any other kind -> 5.
std::int32_t native_lua_type_00b65f80(const NativeLuaObjectStorage&);
// 00B660F0, stack const char*, RET 4: kind 2 STRING whose text strcmp-equals.
bool native_lua_equals_cstring_00b660f0(const NativeLuaObjectStorage&, const char* text);
// 00B66160 / 00B66200, RET: the 00B65F80 type equals 2 / 6.
bool native_lua_is_lightuserdata_00b66160(const NativeLuaObjectStorage&);
bool native_lua_is_function_00b66200(const NativeLuaObjectStorage&);
// 00B662E0, RET, EAX: lua_topointer at the object's index; no kind gate.
const void* native_lua_to_pointer_00b662e0(const NativeLuaObjectStorage&);
// Pushes onto the owner's stack; no tracking record. checkstack(1) first.
void native_lua_push_value_00b663d0(const NativeLuaObjectStorage&);                    // RET
void native_lua_push_nil_00b66430(const NativeLuaObjectStorage&);                      // RET
void native_lua_push_boolean_00b66450(const NativeLuaObjectStorage&, std::uint8_t);    // RET 4
void native_lua_push_integer_00b664b0(const NativeLuaObjectStorage&, std::int32_t);    // RET 4
void native_lua_push_cstring_00b66710(const NativeLuaObjectStorage&, const char*);     // RET 4
// object[key] = value with an integer key: checkstack(2), push the key as a
// number, push the value, lua_settable at the object's CURRENT index08.
void native_lua_set_nil_at_00b664e0(NativeLuaObjectStorage&, std::int32_t key);       // RET 4
void native_lua_set_boolean_at_00b66530(NativeLuaObjectStorage&, std::int32_t key,
    std::uint8_t value);                                                               // RET 8
void native_lua_set_integer_at_00b665d0(NativeLuaObjectStorage&, std::int32_t key,
    std::int32_t value);                                                               // RET 8
void native_lua_set_lightuserdata_at_00b66620(NativeLuaObjectStorage&, std::int32_t key,
    void* value);                                                                      // RET 8
void native_lua_set_new_table_at_00b66670(NativeLuaObjectStorage&, std::int32_t key); // RET 4
// Pushes VALUE's index on THIS owner's stack (value.owner is not read).
void native_lua_set_object_at_00b666c0(NativeLuaObjectStorage&, std::int32_t key,
    const NativeLuaObjectStorage& value);                                              // RET 8
void native_lua_set_string_at_00b66740(NativeLuaObjectStorage&, std::int32_t key,
    const char* value);                                                                // RET 8
void native_lua_set_lstring_at_00b672f0(NativeLuaObjectStorage&, std::int32_t key,
    const NativeString& value);                                                        // RET 8
// object[NativeString key] = nil, RET 4; object[C-string key] = NativeString, RET 8.
void native_lua_set_nil_00b67350(NativeLuaObjectStorage&, const NativeString& key);
void native_lua_set_string_00b67630(NativeLuaObjectStorage&, const char* key,
    const NativeString& value);

// 00B67050, stack remove byte, RET 4: a bound object is released through
// 00B66DE0 with the caller's remove flag, then kind = 0.
void native_lua_release_00b67050(NativeLuaObjectStorage&, std::uint8_t remove_stack);
// 00B678E0 / 00B67910, stack (fresh, key), RET 8, EAX fresh: 00B67800 with the
// NativeString data (empty byte 0108FF2C when null) or the literal.
NativeLuaObjectStorage* native_lua_get_by_native_string_00b678e0(NativeLuaObjectStorage&,
    void* fresh, const NativeString& key);
NativeLuaObjectStorage* native_lua_get_by_literal_00b67910(NativeLuaObjectStorage&,
    void* fresh, const char* key);
// 00B67930, stack fresh, RET 4, EAX fresh: lua_createtable(0, 0) then an
// UNTRACKED kind-2 object at the new top. No placement construction first.
NativeLuaObjectStorage* native_lua_new_table_00b67930(NativeLuaObjectStorage&, void* fresh);
// 00B679F0 / 00B67A80 / 00B67B40 / 00B67C40, stack float* out, RET 4, EAX out:
// elements 1..N through 00B67720, lua_tonumber rounded to float32, each
// temporary released (remove flag 1) when bound. 00B67C40 is a byte-for-byte
// twin of 00B67B40 at a separate address.
float* native_lua_get_vector2_00b679f0(NativeLuaObjectStorage&, float* out);
float* native_lua_get_vector3_00b67a80(NativeLuaObjectStorage&, float* out);
float* native_lua_get_vector4_00b67b40(NativeLuaObjectStorage&, float* out);
float* native_lua_get_float4_00b67c40(NativeLuaObjectStorage&, float* out);
// 00B680A0, stack (out, fallback), RET 8, EAX out: 00B67A80 when the object is
// table-like (kind 1/3, or kind 2 TABLE), otherwise the three fallback floats.
float* native_lua_get_vector3_or_00b680a0(NativeLuaObjectStorage&, float* out,
    const float* fallback);

// 00B68630, stack fresh NativeString, RET 4, EAX fresh. Builds the output with
// 0041E870 from "none"/"nil"/"true"|"false"/"lightuserdata"/<string>/"table"/
// "function"/"userdata"/"thread"/"Unknown"; a number goes through 00B66270 and
// the application's 0043BDF0 ("%f" of the float32).
struct NativeLuaDisplayStringCalls {
    NativeStringStorage& strings;
    // 0043BDF0, __thiscall(ECX = fresh NativeString), stack float, RET 4.
    void (*construct_from_float_0043bdf0)(void* context, NativeString& fresh, float value);
    void* context;
};
NativeString* native_lua_to_display_string_00b68630(const NativeLuaObjectStorage&,
    void* fresh, const NativeLuaDisplayStringCalls&);

// 00B687D0, ECX destination table, stack (int key, source), RET 8, AL.
// 00B68AD0, ECX destination table, stack (NativeString* key, source), RET 8, AL.
// A non-table source is stored directly (00B666C0 / 00B675D0) and returns 1.
// A table source creates a fresh table at the key, takes it back as a tracked
// object, then walks the source: NUMBER keys recurse through 00B687D0 with the
// float32 key truncated by _ftol2, STRING keys through 00B68AD0 with a pooled
// NativeString copy; any other key type releases the temporaries and returns 0.
// The result is the AND of the recursive results. `strings` is the pool behind
// 0041E870 and the inline 00419CC0/00BD1510 release.
bool native_lua_deep_copy_to_index_00b687d0(NativeLuaObjectStorage& destination,
    std::int32_t key, NativeLuaObjectStorage& source, NativeStringStorage& strings);
bool native_lua_deep_copy_to_field_00b68ad0(NativeLuaObjectStorage& destination,
    const NativeString& key, NativeLuaObjectStorage& source, NativeStringStorage& strings);

// 00B66960, __thiscall(ECX = cache), RET. The fundamentals cache destructor:
// vtable D62C18, _free the bytes when non-null and clear them, clear the
// global 0108FF1C, then the base vtable CE3818.
struct NativeLuaFundamentalsStorage {
    std::uintptr_t vtable_00;
    char* bytes_04;
    std::uint32_t size_08;
};
static_assert(sizeof(NativeLuaFundamentalsStorage) == 12);
void destroy_native_lua_fundamentals_00b66960(NativeLuaFundamentalsStorage&,
    volatile std::uint32_t& global_0108ff1c, void (*free_00bf6989)(void*));

// The script cache instance (no constructor in the image; these methods are
// unreferenced). Two actual 10h vector headers, an open byte, an inline owner.
struct NativeLuaScriptRecord {
    unsigned char* data_00;          // CRT block
    std::uint32_t compressed_size_04;
    std::uint32_t bound_08;          // inflate buffer size
};
static_assert(sizeof(NativeLuaScriptRecord) == 12);
struct NativeLuaScriptCacheStorage {
    std::uint32_t opaque_00;
    NativeString* names_begin_04;
    NativeString* names_end_08;
    NativeString* names_capacity_0c;
    std::uint32_t opaque_10;
    NativeLuaScriptRecord* records_begin_14;
    NativeLuaScriptRecord* records_end_18;
    NativeLuaScriptRecord* records_capacity_1c;
    std::uint8_t open_20;
    std::uint8_t padding_21[3];
    NativeLuaStateStorage owner_24;
};
static_assert(sizeof(NativeLuaScriptCacheStorage) == 0x24 + 0x4c8);

class NativeLuaScriptCacheCalls {
public:
    virtual ~NativeLuaScriptCacheCalls() = default;
    virtual void* operator_new_00bf55be(std::uint32_t size) = 0;
    virtual void free_00bf6989(void* block) = 0;
    // CRT checked-iterator report; the native call returns to its caller.
    virtual void invalid_parameter_00bf6713() = 0;
    // 004954F0 erase [first, last) of the actual names header, RET 14h.
    virtual void erase_strings_004954f0(void* actual_header, void* output,
        NativeString* first, NativeString* last) = 0;
};
// 00B6BD60, RET: close the owner (00B65E80, identical body to 00B669A0) when
// open, then clear the open byte.
void close_native_lua_script_cache_00b6bd60(NativeLuaScriptCacheStorage&);
// 00B6C0E0, RET: when not open, 00B6A020(mask 65h) on the owner, run every
// global record (vector header 0108FF40) then every instance record, each by
// operator new(bound), zlib uncompress, 00B65EC0 and _free; set open.
struct NativeLuaScriptRecordView {
    NativeLuaScriptRecord* const volatile& begin_0108ff44;
    NativeLuaScriptRecord* const volatile& end_0108ff48;
};
void open_and_run_native_lua_script_cache_00b6c0e0(NativeLuaScriptCacheStorage&,
    const NativeLuaScriptRecordView& globals, NativeStringStorage&,
    const NativeLuaBootstrapInputs&, NativeLuaScriptCacheCalls&);
// 00B6D0A0, RET: only with a non-empty names vector: close when open, erase
// all names, free every record payload, then copy the empty suffix down with
// 00B6C360 and publish the new record end. Capacity is kept.
void clear_native_lua_script_cache_00b6d0a0(NativeLuaScriptCacheStorage&,
    NativeLuaScriptCacheCalls&);

// ===========================================================================
// Segment 89 (00B80FD0..00B86E10): container instantiations, the scene-model
// material walkers and the refcounted render records. Objects are actual
// native storage reached through raw pointers; offsets are from the listings.
// ===========================================================================

class NativeSeg89Calls {
public:
    virtual ~NativeSeg89Calls() = default;
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void delete_00bf65ac(void* block) = 0;
    // __thiscall(ECX = container) head-node allocators, EAX node.
    virtual void* buy_parser_map_head_00b7e000(void* map) = 0;
    virtual void* buy_cache_map_head_00b7e050(void* map) = 0;
    virtual void* buy_registry_list_head_00b82390(void* list) = 0;
    // __thiscall(ECX = rows), stack 0, RET 4.
    virtual void string_table_rows_tidy_00b80e10(void* rows, std::uint32_t argument) = 0;
    // __thiscall(ECX = type), RET.
    virtual void initialize_fallback_resource_type_00b86a00(void* type) = 0;
    // [[object]+0]: the refcount release slot, ECX = object, no stack argument.
    virtual void release_vcall_00(void* object) = 0;
    // [[visitor]+8]: ECX = visitor, stack (resource, argument), callee cleanup.
    virtual void visit_vcall_08(void* visitor, void* resource, std::uint32_t argument) = 0;
    // [[slot]+10h]: ECX = current, stack (6, 0), EAX replacement.
    virtual void* replace_vcall_10(void* current, std::uint32_t kind, std::uint32_t argument) = 0;
};

// 00B80FD0 / 00B81000, __thiscall(ECX = map), RET, EAX map: std::map
// constructors. head = buynode; head+19h (isnil) = 1; head parent/left/right
// (+4/+0/+8) = head; map+8 (size) = 0.
void* construct_native_parser_map_00b80fd0(void* map, NativeSeg89Calls&);
void* construct_native_cache_map_00b81000(void* map, NativeSeg89Calls&);
// 00B81170 / 00B811A0 (identical bodies), __thiscall(ECX = rows), RET:
// 00B80E10(0), then _free(rows+0).
void destroy_native_string_table_rows_00b81170(void* rows, NativeSeg89Calls&);
void destroy_native_string_table_rows_00b811a0(void* rows, NativeSeg89Calls&);
// 00B81A30 / 00B81A70 / 00B86C80 / 00B86CC0, allocator deallocate:
// __thiscall, stack (block, count), RET 8: operator delete(block).
void deallocate_native_block_00b81a30(void* block, std::uint32_t count, NativeSeg89Calls&);
void deallocate_native_block_00b81a70(void* block, std::uint32_t count, NativeSeg89Calls&);
void deallocate_native_block_00b86c80(void* block, std::uint32_t count, NativeSeg89Calls&);
void deallocate_native_block_00b86cc0(void* block, std::uint32_t count, NativeSeg89Calls&);
// 00B81C10, __fastcall(ECX = count), EAX block: 12-byte elements; a count with
// 0xFFFFFFFF / count < 12 throws std::bad_alloc (00BF6340 / 00BF6885).
void* allocate_native_registry_elements_00b81c10(std::uint32_t count, NativeSeg89Calls&);
// 00B82350, __thiscall(ECX = list), RET: std::list clear; the head is relinked
// and size cleared BEFORE the old nodes are deleted.
void clear_native_registry_list_00b82350(void* list, NativeSeg89Calls&);
// 00B82B50, __thiscall(ECX = list), stack allocator (unused), RET 4, EAX list.
void* construct_native_registry_list_00b82b50(void* list, NativeSeg89Calls&);
// 00B82CC0, __thiscall(ECX = vector), RET: delete begin when set, then
// begin/end/capacity (+4/+8/+0Ch) = 0.
void free_native_registry_storage_00b82cc0(void* vector, NativeSeg89Calls&);

// The model walkers 00B84FE0 / 00B850F0 / 00B85200 / 00B85310: __fastcall
// (ECX = model, EDX = key) plus stack arguments. When [[model]+0Ch](00B74310())
// accepts the model, each geometry 00B74640(i < 00B74650) and each element
// 00B732C0(j < 00B72B40) yields material = [element+20h]; then the children
// [model+34h] linked by +3Ch are walked with the same arguments.
class NativeModelWalkCalls {
public:
    virtual ~NativeModelWalkCalls() = default;
    virtual std::uint32_t current_model_type_00b74310() = 0;
    virtual bool is_type_vcall_0c(void* model, std::uint32_t type) = 0;
    virtual std::uint32_t geometry_count_00b74650(void* model) = 0;
    virtual void* geometry_00b74640(void* model, std::uint32_t index) = 0;
    virtual std::uint32_t element_count_00b72b40(void* geometry) = 0;
    virtual void* element_00b732c0(void* geometry, std::uint32_t index) = 0;
    // ECX = effect ([material+7Ch]); EAX the NativeString at effect+B8h.
    virtual const NativeString* effect_name_00b172d0(void* effect) = 0;
    virtual void set_parameter_owner_00b18a40(void* material, void* owner, std::uint32_t flag) = 0;
    virtual float* diffuse_color_00b179f0(void* material, std::uint32_t index) = 0;
    virtual std::int32_t texture_slot_count_00b179c0(void* material) = 0;
    virtual void set_texture_slot_00b189f0(void* material, std::int32_t slot, void* texture) = 0;
};
// 00B84FE0, RET 4: effect names equal (same length, then _stricmp) ->
// 00B18A40(material, owner, 1).
void set_native_model_parameter_owner_by_effect_00b84fe0(void* model, const NativeString& effect,
    void* owner, NativeModelWalkCalls&);
// 00B850F0, RET 4: every texture slot k < 00B179C0 whose current texture
// ([material+10h+4k] when 0 <= k < (short)[material+34h], else null) equals
// `current` -> 00B189F0(material, k, replacement).
void replace_native_model_texture_00b850f0(void* model, void* current, void* replacement,
    NativeModelWalkCalls&);
// 00B85200, RET 4: an effect name containing `effect` (strstr) gets the four
// dwords of `color` copied into 00B179F0(material, 0). Despite the ledger name
// this writes the colour.
void set_native_model_diffuse_by_effect_00b85200(void* model, const NativeString& effect,
    const std::uint32_t color[4], NativeModelWalkCalls&);
// 00B85310, stack (texture, slot), RET 8, AL: the same strstr match ->
// 00B189F0(material, slot, texture); returns whether anything matched here or
// in any child.
bool set_native_model_texture_by_effect_00b85310(void* model, const NativeString& effect,
    void* texture, std::int32_t slot, NativeModelWalkCalls&);

// 00B854D0, __thiscall(ECX = flags), RET, EAX flags: vtable D63190, +4 = 0,
// +5 = 1, +6 = 0.
void* construct_native_debug_render_flags_00b854d0(void* flags) noexcept;
// 00B85510 / 00B85540 (identical bodies), stack flags byte, RET 4, EAX object:
// global 010901CC = 0, base vtable CE3818, operator delete when bit 0 is set.
void* delete_native_debug_render_flags_00b85510(void* object, std::uint8_t flags,
    volatile std::uint32_t& global_010901cc, NativeSeg89Calls&);
void* delete_native_debug_render_flags_00b85540(void* object, std::uint8_t flags,
    volatile std::uint32_t& global_010901cc, NativeSeg89Calls&);

// The Mit {data, count, capacity} pointer vector members 00B85750 / 00B857A0 /
// 00B85A30 / 00B85AC0 (and 00A9AEB0 / 00A9AF00 / 00A9B250 / 00A9B2D0 /
// 00A9EA60 / 00A9EE40 in segment 75) are instruction-for-instruction the
// Widget pool's 00AA6FA0 / 00AA6FF0 / 00AA76C0 / 00AA7740 apart from jump
// targets (docs/CC13_SEG87.md), so they delegate to those existing bodies in
// src/native_material_pools.cpp. ECX = vector for each.
// 00B85750, stack capacity, RET 4: capacity = argument, then reallocate.
void set_native_pointer_vector_capacity_00b85750(NativeGuiPoolSlabTable&, std::uint32_t capacity);
// 00B857A0, RET: capacity = capacity * 2 + 2, then reallocate.
void grow_native_pointer_vector_00b857a0(NativeGuiPoolSlabTable&);
// 00B85A30, stack const pointer&, RET 4: grow when full, then append.
void push_native_pointer_vector_00b85a30(NativeGuiPoolSlabTable&, std::byte* const& value);
// 00B85AC0, stack capacity, RET 4: reallocate only when capacity < argument.
void reserve_native_pointer_vector_00b85ac0(NativeGuiPoolSlabTable&, std::uint32_t capacity);

// 00B85980, stack value, RET 4: 64 dwords at table+60h, stride 64h.
void fill_native_slot_table_00b85980(void* table, std::uint32_t value) noexcept;
// 00B859A0, RET, EAX: word [table+1980h] == 40h.
bool native_slot_table_is_full_00b859a0(const void* table) noexcept;
// 00B85B30, ECX = owner, stack node, RET 4: unlink node from the +38h chain
// that starts at owner+38h (no end test), clear node+38h, then drop one
// reference (InterlockedDecrement node+4; release slot 0 at zero).
void unlink_native_ref_chain_00b85b30(void* owner, void* node, NativeSeg89Calls&);

// The 60h refcounted record built by 00B860E0 and assigned by 00B861D0.
// +20h, +38h, +50h, +5Ch and the +3Ch[count +4Ch] array hold references whose
// count is at +4 (kernel32 InterlockedIncrement/Decrement).
// 00B860E0, ECX record, stack (ref20, d08, d0C, d10, d14, d18, d1C,
// const float color[4], unused), RET 24h, EAX record.
void* construct_native_ref_record_00b860e0(void* record, void* ref_20, std::uint32_t d08,
    std::uint32_t d0c, std::uint32_t d10, std::uint32_t d14, std::uint32_t d18,
    std::uint32_t d1c, const float color[4], std::uint32_t unused, NativeSeg89Calls&);
// 00B861D0, ECX destination, stack source, RET 4, EAX destination.
void* assign_native_ref_record_00b861d0(void* destination, const void* source, NativeSeg89Calls&);

// 00B866B0, RET 4, EAX 0.
std::uint32_t native_return_zero_00b866b0(std::uint32_t ignored) noexcept;
// 00B86800 / 00B86810 / 00B86860 / 00B86870: EAX = the global dword.
std::uint32_t native_fallback_runtime_type_00b86800(const volatile std::uint32_t& global_01090210) noexcept;
std::uint32_t native_fallback_type_descriptor_00b86810(const volatile std::uint32_t& global_01090218) noexcept;
std::uint32_t native_unknown_item_type_token_00b86860(const volatile std::uint32_t& global_0109021c) noexcept;
std::uint32_t native_unknown_item_aux_token_00b86870(const volatile std::uint32_t& global_01090228) noexcept;
// 00B86820, ECX resource, stack (visitor, unused, argument, unused), RET 10h:
// [[visitor]+8](resource, argument).
void dispatch_native_fallback_resource_00b86820(void* resource, void* visitor,
    std::uint32_t unused_1, std::uint32_t argument, std::uint32_t unused_3, NativeSeg89Calls&);
// 00B86880 RET 10h and 00B868A0 RET 4: no operation.
void native_resource_item_noop_00b86880() noexcept;
void native_resource_item_noop_00b868a0() noexcept;
// 00B868D0, stack token, RET 4, AL: token is one of the two dwords at
// 01090210..01090217.
bool native_fallback_resource_is_type_00b868d0(const volatile std::uint32_t (&types_01090210)[2],
    std::uint32_t token) noexcept;
// 00B86AC0, ECX type, RET, EAX type: 00B86A00.
void* initialize_native_fallback_type_00b86ac0(void* type, NativeSeg89Calls&);
// 00B86AE0, stack flags, RET 4, EAX object: vtable D63208, delete on bit 0.
void* delete_native_resource_interface_00b86ae0(void* object, std::uint8_t flags, NativeSeg89Calls&);
// 00B86B90, ECX slot owner, RET: +0Ch = [[+0Ch]+10h](6, 0).
void replace_native_ref_slot_00b86b90(void* owner, NativeSeg89Calls&);
// 00B86D00, ECX tree, stack node, RET 4: the std::_Tree _Rrotate shape
// (left child lifted; nodes +0 left, +4 parent, +8 right, +21h isnil).
void rotate_native_type_tree_00b86d00(void* tree, void* node) noexcept;
// 00B86DF0, ECX node, EAX: follow +8 while the right child's +21h is zero.
void* native_type_tree_maximum_00b86df0(void* node) noexcept;

// ===========================================================================
// Segment 75 (00A9AC70..00AA0F50): the Listbox slab pool, Listbox members
// over the actual 17Ch Listbox object, and small text helpers.
// ===========================================================================

// The Listbox pool at 00F8BC10: one more instance of the shared Mit slab pool
// engine (include/bsp/native_gui_widget_pools_cc13.hpp). Every member is the
// Model/Widget pool member apart from these constants and jump targets.
struct GuiListboxPoolTraits {
    static constexpr std::uint32_t native_global = 0x00F8BC10;
    static constexpr std::uint32_t native_vtable = 0x00D5BBA4;
    static constexpr std::uint32_t native_virtual0 = 0x00A9B0B0;  // Trim
    static constexpr std::size_t slot_bytes = 0x180;
    static constexpr std::size_t object_bytes = slot_bytes - 4;
    static constexpr std::size_t slot_slab_index_offset = slot_bytes - 4;
    static constexpr std::uint32_t slots_per_slab = 16;
    static constexpr std::size_t free_indices_offset = slot_bytes * slots_per_slab;                  // 1800h
    static constexpr std::size_t free_count_offset = slot_bytes * slots_per_slab + 2 * slots_per_slab; // 1820h
    static constexpr std::size_t slab_bytes = free_count_offset + 4;                                  // 1824h
    static constexpr std::uint32_t construct_address = 0x00A9B4A0;
    static constexpr std::uint32_t allocate_address = 0x00A9B580;
    static constexpr std::uint32_t free_address = 0x00A9B150;
    static constexpr std::uint32_t trim_address = 0x00A9B0B0;
    static constexpr std::uint32_t destroy_address = 0x00A9AFD0;
    static constexpr std::uint32_t init_block_address = 0x00A9ACD0;
    static constexpr std::uint32_t free_thunk_address = 0x00A9B330;
    static constexpr std::uint32_t allocate_stub_address = 0x00A9B7E0;
};
static_assert(GuiListboxPoolTraits::free_indices_offset == 0x1800);
static_assert(GuiListboxPoolTraits::free_count_offset == 0x1820);
static_assert(GuiListboxPoolTraits::slab_bytes == 0x1824);
extern template class NativeGuiWidgetPool<GuiListboxPoolTraits>;
using NativeGuiListboxPool = NativeGuiWidgetPool<GuiListboxPoolTraits>;

// Address-named entries over that engine (ABI per traits row above).
void* initialize_native_listbox_pool_slab_00a9acd0(void* raw, std::uint32_t index) noexcept;
void destroy_native_listbox_pool_00a9afd0(NativeGuiListboxPool&);
void trim_native_listbox_pool_00a9b0b0(NativeGuiListboxPool&);
void free_native_listbox_pool_slot_00a9b150(NativeGuiListboxPool&, void* slot) noexcept;
// 00A9B330, __fastcall(ECX = slot): MOV ECX, 00F8BC10; CALL 00A9B150.
void free_native_listbox_slot_thunk_00a9b330(NativeGuiListboxPool& pool_00f8bc10, void* slot) noexcept;
void construct_native_listbox_pool_00a9b4a0(NativeGuiListboxPool&);
void* allocate_native_listbox_pool_slot_00a9b580(NativeGuiListboxPool&);
// The slab-table members (ECX = pool+28h).
void set_native_listbox_pool_table_capacity_00a9aeb0(NativeGuiPoolSlabTable&, std::uint32_t capacity);
void grow_native_listbox_pool_table_00a9af00(NativeGuiPoolSlabTable&);
void push_native_listbox_pool_slab_00a9b250(NativeGuiPoolSlabTable&, std::byte* const& slab);
void reserve_native_listbox_pool_table_00a9b2d0(NativeGuiPoolSlabTable&, std::uint32_t capacity);
// The StringMap node pool's table members (pool 00E1768C, slot 1Ch x 400h).
void grow_native_string_map_pool_table_00a9ea60(NativeGuiPoolSlabTable&);
void push_native_string_map_pool_slab_00a9ee40(NativeGuiPoolSlabTable&, std::byte* const& slab);

// Listbox members over the actual object. Layout used here: row list header
// +FCh {alloc, head +100h, size +104h} with nodes {next +0, prev +4, row +8};
// selected iterator +108h {container, node}; previous +110h; listener +114h;
// highlight selector +118h; flags +11Ch..+11Fh; highlight width float +124h;
// input filter +144h; paging byte +148h; paging list +14Ch; arrows +15Ch/+160h;
// NativeString vector +16Ch {alloc, begin +170h, end +174h, capacity +178h}.
class NativeGuiListboxCalls {
public:
    virtual ~NativeGuiListboxCalls() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void delete_00bf65ac(void* block) = 0;
    // Widget virtual slots (ECX = widget).
    virtual void show_vcall_34(void* widget, std::uint8_t shown) = 0;
    virtual void set_position_vcall_58(void* widget, const float xy[2]) = 0;
    virtual std::int32_t kind_vcall_5c(void* widget) = 0;
    virtual void activate_vcall_60(void* widget, std::uint8_t active) = 0;
    virtual void refresh_vcall_80(void* widget, std::uint8_t force) = 0;
    // Direct calls.
    virtual const float* size_00aa6740(void* widget) = 0;
    virtual const float* local_position_00aa67f0(void* widget) = 0;
    virtual void set_local_y_00aa78f0(void* widget, float y) = 0;
    // ECX = widget, EDX = source, stack float.
    virtual void fit_relative_00ac0820(void* widget, void* source, float value) = 0;
    // ECX = listbox, stack (width, argument), RET 8.
    virtual void set_highlight_width_00a9ceb0(void* listbox, float width, std::uint32_t argument) = 0;
    virtual void select_index_00a9c7c0(void* listbox, std::int32_t index) = 0;
    virtual void clear_list_00a9b3f0(void* list) = 0;
    virtual void* gui_manager_004c12b0() = 0;
    virtual void* highlight_widget_00aa0f50(void* manager, std::int32_t selector) = 0;
    virtual void* buy_list_head_00a9b720(void* list) = 0;
    virtual void destroy_list_storage_00a9b740(void* list) = 0;
    // ECX = first, EDX = last, stack (vector header, owner).
    virtual void destroy_string_range_00432050(void* first, void* last, void* header, void* owner) = 0;
    virtual void destroy_widget_base_00aa9730(void* widget) = 0;
};

// 00A9AC70, stack routine, RET 4: listbox+144h = routine.
void set_native_listbox_input_filter_00a9ac70(void* listbox, std::uint32_t routine) noexcept;
// 00A9BB30, RET: when paging (+148h): clear the byte, clear the +14Ch list,
// hide both arrows ([arrow]+34h(0)).
void close_native_listbox_paging_00a9bb30(void* listbox, NativeGuiListboxCalls&);
// 00A9BB70, stack (row, apply), RET 8, AL: the row itself when its kind
// (+5Ch) is 3, otherwise its first kind-3 child (+64h list); none -> 0.
// With apply: target+1F0h = 0; when width +124h is not 0.0 (NaN counts as
// not equal), set position (width, size.y); then if target+100h is zero,
// +30h = -0.009375f (00CF3F64) and +34h = 0.0f. Returns 1 when found.
bool layout_native_listbox_item_00a9bb70(void* listbox, void* row, std::uint8_t apply,
    NativeGuiListboxCalls&);
// 00A9BF90, RET: listener cleared, every row hidden ([row]+34h(0)) and its
// node unlinked and deleted, selection = end, previous = null, listener
// restored, then +80h(0).
void clear_native_listbox_items_00a9bf90(void* listbox, NativeGuiListboxCalls&);
// 00A9C860, stack data, RET 4: selection = end, then the first row whose
// +D8h equals data becomes selected unless hidden (+77h); then +80h(0).
void select_native_listbox_row_by_data_00a9c860(void* listbox, std::uint32_t data,
    NativeGuiListboxCalls&);
// 00A9C8F0, stack index, RET 4: 00A9C7C0 with the listener cleared.
void select_native_listbox_index_silently_00a9c8f0(void* listbox, std::int32_t index,
    NativeGuiListboxCalls&);
// 00A9CC50, stack count, RET 4: for count > 0, walk back from the head
// count times; each row's local y += size.y * 0.5 (double), then float.
void scroll_native_listbox_00a9cc50(void* listbox, std::int32_t count, NativeGuiListboxCalls&);
// 00A9CDC0, RET: selection = end, +60h(0).
void clear_native_listbox_selection_00a9cdc0(void* listbox, NativeGuiListboxCalls&);
// 00A9CDF0, ECX listbox, EDX other, stack third, RET 4: 00A9CDC0 on each
// other list that is not this one, then this +60h(1).
void select_native_listbox_exclusive_00a9cdf0(void* listbox, void* other, void* third,
    NativeGuiListboxCalls&);
// 00A9CE60, ECX listbox, EDX other, stack (third, fourth), RET 8: fourth
// first, then 00A9CDF0(other, third).
void select_native_listbox_exclusive_00a9ce60(void* listbox, void* other, void* third,
    void* fourth, NativeGuiListboxCalls&);
// 00A9CF30, stack (apply, argument), RET 8: width +124h = 0; with apply, fit
// every row to the manager's highlight widget (00AA0F50 selector +118h),
// raising +124h to the highlight width when larger, then 00A9CEB0(+124h, argument).
void reset_native_listbox_highlight_00a9cf30(void* listbox, std::uint8_t apply,
    std::uint32_t argument, NativeGuiListboxCalls&);
// 00A9D010, ECX list, RET, EAX list: head = 00A9B720, size = 0.
void* construct_native_gui_list_00a9d010(void* list, NativeGuiListboxCalls&);
// 00A9D360, stack (index, step, force), RET 0Ch. Neither centre-vertical
// (+11Dh) nor force: 00A9CC50(size - index). Otherwise step (or, when
// step <= 0.001f, first row size.y * 0.25) moves rows before `index` by
// -step and the rest by +step.
void scroll_native_listbox_to_item_00a9d360(void* listbox, std::int32_t index, float step,
    std::uint8_t force, NativeGuiListboxCalls&);
// 00A9E130, RET: the Listbox destructor body (vtable D5BBF8).
void destroy_native_listbox_00a9e130(void* listbox, NativeGuiListboxCalls&);
// 00A9E210, stack flags, RET 4, EAX listbox: 00A9E130, then the pool free
// 00A9B150 on 00F8BC10 when bit 0 is set.
void* delete_native_listbox_00a9e210(void* listbox, std::uint8_t flags,
    NativeGuiListboxPool& pool_00f8bc10, NativeGuiListboxCalls&);

// The native wide string {length, data} read by 00A9EC30 and 00A9EF60.
struct NativeWideStringHeader {
    std::uint32_t length_00;
    char16_t* data_04;
};
// 00A9EC30, ECX locale manager, stack string*, RET 4: each code unit through
// 00A9EBA0, length and data reloaded per unit.
void uppercase_native_wide_string_00a9ec30(const LocaleTables&, LocaleTextRuntimeHost&,
    NativeWideStringHeader& text);
// 00A9EF60, ECX string, stack (set*, start), RET 8, EAX index or -1:
// _wcscspn from max(start, 0); a start above the length, an empty or null
// string, a null set, or no hit gives -1.
std::int32_t find_first_of_native_wide_string_00a9ef60(const NativeWideStringHeader& text,
    const NativeWideStringHeader& set, std::int32_t start);
// 00A9F460, ECX fresh 10h record, stack const NativeString*, RET 4: header
// zeroed, then (unless self) resize(source length, preserve) and copy that
// many bytes; +8 and +0Ch = 0.
void* copy_construct_native_string_record_00a9f460(void* fresh, const NativeString& source,
    NativeStringStorage&);

class NativeGuiAnimatedValueCalls {
public:
    virtual ~NativeGuiAnimatedValueCalls() = default;
    virtual void stop_vcall_34(void* current, std::uint32_t argument) = 0;
    virtual void set_target_vcall_88(void* current, std::uint32_t target, std::uint32_t mode,
        float rate) = 0;
    // The routine stored at value+4: ECX = current, DL = 0.
    virtual void notify_04(std::uint32_t routine, void* current, std::uint8_t flag) = 0;
};
// 00AA0E20, stack byte, RET 4: [+50h]+34h(0), then +50h = byte ? +54h : +58h.
void restart_native_gui_animated_value_00aa0e20(void* value, std::uint8_t forward,
    NativeGuiAnimatedValueCalls&);
// 00AA0E60, stack target, RET 4: [+50h]+88h(target, 0, 1.0f), then the
// routine at +4 on the (reloaded) +50h with DL = 0.
void set_native_gui_animated_value_target_00aa0e60(void* value, std::uint32_t target,
    NativeGuiAnimatedValueCalls&);

// The 8-byte tagged value the property streams pass by value: type 0 string
// (C-string or NativeString* target), 1 int, 2 float, 3 bool (low byte).
struct NativeTaggedValue {
    std::uint32_t type_00;
    std::uint32_t value_04;
};
static_assert(sizeof(NativeTaggedValue) == 8);

class NativeGuiListboxPropertyCalls {
public:
    virtual ~NativeGuiListboxPropertyCalls() = default;
    // Base widget describer / reader, ECX = widget, stack stream, RET 4.
    virtual void describe_widget_base_00aaaed0(void* widget, void* writer) = 0;
    virtual void read_widget_base_00aaa710(void* widget, void* reader) = 0;
    // ECX = left, stack right, AL.
    virtual bool tagged_value_equals_00bd5680(const NativeTaggedValue& left,
        const NativeTaggedValue& right) = 0;
    // Stream vtable slots (ECX = stream); tagged values are passed by value.
    virtual void begin_table_vcall_04(void* stream, NativeTaggedValue key) = 0;
    virtual void end_table_vcall_08(void* stream) = 0;
    virtual void write_vcall_0c(void* writer, NativeTaggedValue key, NativeTaggedValue value) = 0;
    virtual void read_vcall_0c(void* reader, NativeTaggedValue key, NativeTaggedValue target,
        NativeTaggedValue fallback) = 0;
    virtual void read_vcall_10(void* reader, NativeTaggedValue key, NativeTaggedValue target) = 0;
    virtual bool has_vcall_14(void* reader, NativeTaggedValue key) = 0;
    virtual void invalid_parameter_00bf6713() = 0;
    // ECX = vector header, stack (output, first {header, ptr}, last {header, ptr}).
    virtual void erase_strings_004954f0(void* header, void* output, NativeString* first,
        NativeString* last) = 0;
    // ECX = vector header, stack const NativeString*: checked push_back copy.
    virtual void append_string_00450540(void* header, const NativeString& value) = 0;
    // ECX = widget, stack (const NativeString*, 1), RET 8, EAX child.
    virtual void* find_child_by_name_00aa7e00(void* widget, const NativeString& name,
        std::uint32_t recursive) = 0;
    // ECX = listbox, stack (row, position, after), RET 0Ch.
    virtual void append_row_00a9d750(void* listbox, void* row, void* position,
        std::uint8_t after) = 0;
    virtual void refresh_vcall_80(void* widget, std::uint8_t force) = 0;
};
// 00A9B7F0, Listbox describer, ECX listbox, stack writer, RET 4. After
// 00AAAED0: DontMoveTheItems (+11Ch), AutoControl (+11Eh), CenterVertical
// (+11Dh) against false, LineDistance (+13Ch) and HeightPlus (+128h) against
// 0.0f, each written ({0, name}, value) only when 00BD5680 says it differs;
// then, for a non-empty +16Ch vector, begin "Items", write ({1, i + 1},
// {0, string data or the empty global 00F8BC48}) per entry, end the table.
// The native bool values carry three unwritten stack bytes above the flag;
// they are zero here.
void describe_native_listbox_properties_00a9b7f0(void* listbox, void* writer,
    const char* empty_00f8bc48, NativeGuiListboxPropertyCalls&);
// 00A9E400, Listbox reader, ECX listbox, stack reader, RET 4. After 00AAA710:
// DontMoveTheItems, AutoControl (bool, default false), NewHighlightIndex
// (+118h, int, default 1), CenterVertical; erase the +16Ch strings; LineDistance
// and HeightPlus (float, default 0.0f) only when present; "Items" when present:
// read {1, i} for i = 1.. while present into a pooled NativeString, append it
// (00450540) and release the temporary; then each stored name is resolved with
// 00AA7E00(name, 1) and appended through 00A9D750(row, null, 0); finally +80h(1).
void read_native_listbox_properties_00a9e400(void* listbox, void* reader,
    NativeStringStorage& strings, NativeGuiListboxPropertyCalls&);

} // namespace bsp
