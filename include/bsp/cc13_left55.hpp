// Packet cc13-left55: functions of segments 55 (00878530..0088B3D0) and 56 (0088B410..008DCD40)
// that the earlier waves left without a reconstruction record. docs/CC13_LEFT55.md has the
// per-function table.
//
// Segment 55's leftovers are the mission-script plumbing under the Lua bindings: the Lua variant
// record (vtable 00D0E6F4) and its map payload, the key type of that map, a few NativeString
// helpers, an instantiation of the VC9 std::sort internals over 8-byte {key, float} pairs, and
// two byte-grid passes of the landscape. They work on image memory directly, so this half uses
// raw layouts (Win32 only; the static_asserts pin the offsets) and routes every call the native
// makes through `Left55Host`, one method per callee, the address in the name.
//
// Names are hypotheses from behaviour, not recovered symbols. Status: reconstructed and
// build-tested only; not fixture-tested, not game-validated; these are new C++ interfaces, not
// ABI-compatible drop-ins (each comment records the native ABI).
#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/engine_seg56_cc13.hpp"

namespace bsp::cc13_left55 {

struct Left55Host;
// 00888FB0 ECX object, RET: clear exactly byte+290h.
void owner_clear_byte_290_00888fb0(void*) noexcept;
// 00889940 ECX message, stack byte, EAX same, RET4. Base kind9Fh,
// vtable D03360. Preserve untouched padding and other base-written bytes.
void* byte_message_9f_construct_00889940(Left55Host& host, void* message, std::uint8_t value);

// ---------------------------------------------------------------------------
// Image layouts
// ---------------------------------------------------------------------------

// The engine's 8-byte string: size, then a pool-allocated, NUL-terminated buffer (null when
// empty). Its storage is returned with 00419CC0 + 00BD1510 over (data, size + 1).
struct NativeStr {
    std::uint32_t size;
    char* data;
};

// Map key of the variant table: kind 0 = number key (+0Ch), kind 1 = string key (+4h).
struct LuaKey {
    std::int32_t kind;    // +00h
    NativeStr text;       // +04h
    std::int32_t number;  // +0Ch
};

// The Lua variant record. vtable 00D0E6F4; tag -1 empty, 0 float, 1 string (heap NativeStr*),
// 2 bool, 3 entity id (u16), 4 light userdata, 5 table (heap map*), 6 nil.
struct LuaVariant {
    std::uint32_t vtable;  // +00h
    std::int32_t tag;      // +04h
    union {
        float number;
        bool boolean;
        std::uint16_t id;
        NativeStr* text;
        void* pointer;
        struct VariantMap* map;
    } payload;             // +08h
    NativeStr name;        // +0Ch, released by the destructor 00884AF0
};

// std::map<LuaKey, LuaVariant> node (VC9 _Tree_nod): links, key, value, colour, isnil.
struct VariantMapNode {
    VariantMapNode* left;    // +00h
    VariantMapNode* parent;  // +04h
    VariantMapNode* right;   // +08h
    LuaKey key;              // +0Ch
    LuaVariant value;        // +1Ch
    std::uint8_t color;      // +30h
    std::uint8_t is_nil;     // +31h
};

// The map object the tag-5 payload points at (0Ch bytes, operator new(0Ch)).
struct VariantMap {
    void* proxy;            // +00h, never touched by these functions
    VariantMapNode* head;   // +04h
    std::uint32_t size;     // +08h
};

// A checked map iterator as the inlined loops hold it: the owning map and the node.
struct VariantMapIterator {
    VariantMap* container;
    VariantMapNode* node;
};

// The element of the sorted array 0088A130..0088AC20 work on: a dword and its float score.
struct ScoredEntry {
    std::uint32_t key;
    float score;
};

// MissionLuaPendingResults (vtable 00D0E6F0): +4 sink, +8 negative stack index, +0Ch armed.
struct PendingResults {
    std::uint32_t vtable;
    void* sink;
    std::int32_t stack_index;
    std::uint8_t armed;
};

// MissionLuaResultSink (vtable 00D0E798): +4 the LuaMachine, +8 the variant vector.
struct ResultSink {
    std::uint32_t vtable;
    void* lua_machine;
    std::uint8_t variants[0x0C];  // +08h std::vector<LuaVariant>
};

// Deferred call record copied by 00887A10: two strings, a variant vector, two dwords.
struct DeferredCall {
    NativeStr self_key;           // +00h
    NativeStr function_name;      // +08h
    std::uint32_t arguments[3];   // +10h vector {first, last, end}
    std::uint32_t stack_first;    // +1Ch
    std::uint32_t stack_last;     // +20h
};

// A pointer array {data, count, capacity}, grown by 00529760.
struct PointerArray {
    void** data;
    std::int32_t count;
    std::int32_t capacity;
};

// Session message 97h built by 00888F30 (base 0075B430, vtable 00D02F24).
struct PositionMessage97 {
    std::uint8_t base[0x18];  // +00h..+17h written by 0075B430, then +4h = 1
    std::uint16_t word_18;    // +18h
    std::uint8_t byte_1a;     // +1Ah
    std::uint8_t pad_1b;
    std::uint8_t byte_1c;     // +1Ch
    std::uint8_t pad_1d[3];
    std::uint8_t byte_20;     // +20h
    std::uint8_t byte_21;     // +21h
    std::uint8_t pad_22[2];
    float value_24;           // +24h
    std::int32_t value_28;    // +28h
    float position[3];        // +2Ch..+34h
};

#if defined(_M_IX86)
static_assert(sizeof(NativeStr) == 8);
static_assert(sizeof(LuaKey) == 0x10);
static_assert(sizeof(LuaVariant) == 0x14);
static_assert(offsetof(VariantMapNode, value) == 0x1C);
static_assert(offsetof(VariantMapNode, is_nil) == 0x31);
static_assert(sizeof(VariantMap) == 0x0C);
static_assert(sizeof(DeferredCall) == 0x24);
static_assert(offsetof(PositionMessage97, position) == 0x2C);
#endif

inline constexpr std::uint32_t kLuaVariantVtable = 0x00D0E6F4u;
inline constexpr std::uint32_t kPositionMessage97Vtable = 0x00D02F24u;
inline constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818u;
inline constexpr std::uint32_t kEmptyCString = 0x00F87904u;  // "" returned for null data

// ---------------------------------------------------------------------------
// Host: one method per native callee
// ---------------------------------------------------------------------------
struct Left55Host {
    virtual ~Left55Host() = default;

    // --- strings ---
    virtual void native_string_resize_0041dd40(NativeStr* s, std::uint32_t size, bool keep) = 0;
    virtual void native_string_assign_0041e870(NativeStr* s, const char* text) = 0;
    // 00419CC0(data, bytes, 1) then 00BD1510 on the pool it returns: the inlined ~NativeStr.
    virtual void pool_release_00419cc0_00bd1510(void* data, std::uint32_t bytes) = 0;
    virtual void native_string_destroy_storage_0041dd20(NativeStr* s) = 0;
    virtual void native_string_append_00425e10(NativeStr* dst, const NativeStr* src) = 0;
    virtual void native_string_append_float_00781740(NativeStr* dst, float value) = 0;
    virtual NativeStr* native_string_copy_construct_00426060(void* storage, const NativeStr* src) = 0;
    virtual void native_string_from_int_004260b0(NativeStr* out, std::int32_t value) = 0;
    virtual void native_string_from_float_0043bdf0(NativeStr* out, float value) = 0;
    // 0043C130 __fastcall(ECX out, EDX prefix, stack const NativeStr*): out = prefix + *text.
    virtual NativeStr* native_string_prefixed_0043c130(NativeStr* out, const char* prefix,
        const NativeStr* text) = 0;
    // 004261A0 __thiscall(ECX left, out, right): out = *left + *right.
    virtual void native_string_concat_004261a0(const NativeStr* left, NativeStr* out,
        const NativeStr* right) = 0;
    // 00781540 __thiscall(ECX key, out): out = copy of key->text.
    virtual void key_text_copy_00781540(const LuaKey* key, NativeStr* out) = 0;

    // --- CRT ---
    virtual void* operator_new_00bf681b(std::uint32_t bytes) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void invalid_parameter_00bf6713() = 0;  // _invalid_parameter_noinfo (checked iterators)
    virtual void rethrow_00bf6885() = 0;            // _CxxThrowException(0, 0): `throw;`

    // --- the variant map ---
    virtual VariantMapNode* map_allocate_head_006edea0(VariantMap* map) = 0;
    // 006EE960 __thiscall(map, out iterator, first iterator, last iterator): erase(first, last).
    virtual void map_erase_range_006ee960(VariantMap* map, VariantMapIterator* out,
        VariantMapIterator first, VariantMapIterator last) = 0;
    virtual void map_copy_00781890(VariantMap* map, const VariantMap* source) = 0;
    virtual VariantMap* map_initialize_header_006eeaf0(void* storage) = 0;
    virtual void map_assign_00781f20(VariantMap* map, const VariantMap* source) = 0;
    virtual void map_iterator_increment_006eda20(VariantMapIterator* it) = 0;
    // 00884D20 __thiscall(map, out iterator, const LuaKey*): find.
    virtual void map_find_00884d20(VariantMap* map, VariantMapIterator* out, const LuaKey* key) = 0;
    virtual void variant_init_tag6_0074fb30(LuaVariant* variant) = 0;

    // --- other containers ---
    virtual void* list_allocate_head_00884830() = 0;
    virtual void* tree_buy_head_node_0088a520() = 0;
    virtual void call_queue_clear_00887d90(void* queue) = 0;
    virtual void deferred_copy_arguments_00887560(void* dst_vector, const void* src_vector) = 0;
    virtual void pointer_array_reserve_00529760(PointerArray* array, std::int32_t capacity) = 0;
    virtual void variant_vector_push_back_006edf00(void* vector, const LuaVariant* value) = 0;

    // --- Lua ---
    virtual void lua_machine_push_string_006b8120(void* machine, const char* text) = 0;
    virtual void lua_machine_get_table_006b8470(void* machine, std::int32_t index) = 0;
    virtual std::int32_t lua_machine_get_top_006b7e70(void* machine) = 0;
    // 00886E00 __thiscall(sink, out variant, stack index, depth 0, options).
    virtual void build_variant_from_stack_00886e00(ResultSink* sink, LuaVariant* out,
        std::int32_t index, std::int32_t depth, std::int32_t options) = 0;
    virtual void lua_table_set_number_number_006b82a0(void* table, float key, float value) = 0;
    virtual void lua_table_set_string_number_006b8260(void* table, const char* key, float value) = 0;
    virtual void lua_table_set_number_string_006b8320(void* table, float key, const char* value) = 0;
    virtual void lua_table_set_string_string_006b8360(void* table, const char* key,
        const char* value) = 0;
    // 00803750 / 008037D0: ECX = an 8-byte scope, stack (instance, key); the scope holds the
    // instance at +4 (include/bsp/recon_values.hpp).
    virtual void table_scope_indexed_00803750(void* scope, void* instance, std::int32_t key) = 0;
    virtual void table_scope_named_008037d0(void* scope, void* instance, const char* key) = 0;
    virtual void lua_settop_00a673e0(void* state, std::int32_t top) = 0;

    // LuaObject (00B6xxxx). `object` is the native 14h-byte LuaObject; `out` a fresh one.
    virtual void lua_object_construct_00b65f50(void* object) = 0;
    virtual void lua_object_get_by_name_00b67800(void* object, void* out, const char* name) = 0;
    virtual void* lua_object_to_userdata_00b662d0(void* object) = 0;
    virtual bool lua_object_is_table_00b661b0(void* object) = 0;
    virtual bool lua_object_is_light_userdata_00b66160(void* object) = 0;
    virtual void lua_object_destruct_00b67700(void* object) = 0;
    virtual void lua_object_iterate_first_00b67080(void* table, void* key, void* value) = 0;
    virtual void lua_object_iterate_next_00b67190(void* table, void* key, void* value) = 0;
    // 00B66420 __thiscall(table, key): true when the iteration key is no longer bound (end).
    virtual bool lua_object_is_unbound_00b66420(void* table, void* key) = 0;

    // --- game ---
    virtual std::int8_t byte_grid_sample_00412070(std::int8_t** columns, std::int32_t width,
        std::int32_t height, std::int32_t x, std::int32_t y, std::int32_t direction,
        std::int8_t fallback) = 0;
    virtual void grid_step_direction8_00412010(std::int32_t* x, std::int32_t* y,
        std::int32_t direction) = 0;
    virtual void session_message_base_0075b430(void* message, std::uint32_t type) = 0;
    virtual std::uint32_t scene_node_type_00bbe290() = 0;
    virtual void scene_node_set_alpha_00bbe490(void* node, float alpha) = 0;
    virtual void game_end_scene_004d7970(void* game, std::int32_t flag) = 0;
    virtual void game_request_state_004d7920(void* game, std::int32_t state) = 0;
    virtual void* profile_manager_00425c20() = 0;
    virtual void profile_clear_checkpoint_00437c70(void* profile_manager) = 0;
    virtual void hud_pause_resume_005fd440(void* pause_screen) = 0;
    virtual void* game_global_00e188a8() = 0;      // [00E188A8]
    virtual void* interface_global_00e198c4() = 0; // [00E198C4]
    virtual void write_global_00f87908(std::uint32_t value) = 0;

    // --- virtual calls on image objects (slot offset in the name) ---
    virtual void handle_sink_vcall_20(void* sink, void* handle, std::uint32_t argument) = 0;
    virtual void transform_vcall_1c(void* object, float out[2], float x, float y) = 0;
    virtual bool node_is_kind_vcall_0c(void* node, std::uint32_t type) = 0;
    virtual bool object_is_kind_vcall_5c(void* object, std::int32_t kind) = 0;
    virtual void refcounted_destroy_vcall_00(void* object) = 0;
    virtual std::int32_t interlocked_decrement(volatile long* value) = 0;
    // 00880420: the terrain source at owner+3D0h and the mask dilation it ends with.
    virtual std::int32_t terrain_rows_vcall_10(void* terrain) = 0;
    virtual std::int32_t terrain_columns_vcall_14(void* terrain) = 0;
    virtual float terrain_height_vcall_20(void* terrain, std::int32_t row, std::int32_t column) = 0;
    // 0087FA90 is __stdcall (ECX is loaded with the owner but unused); reconstructed as
    // landscape_dilate_mask_to_blocks_0087fa90 in include/bsp/band_0085_cc13.hpp.
    virtual void landscape_dilate_mask_0087fa90(std::int8_t** columns, std::int32_t width,
        std::int32_t height, std::int32_t block) = 0;
};

// ---------------------------------------------------------------------------
// Segment 55
// ---------------------------------------------------------------------------

// 0087BB02 / 0087C518: catch(...) funclets of two vector reallocations. Frame values: the new
// block [EBP+14h], its constructed end [EBP+10h], the allocator [EBP-14h]. Destroy the range
// (00878AC0 / 0087B5B0), free the block, rethrow.
void damageable_section_vector_growth_catch_0087bb02(Left55Host& host, void* allocator,
    void* first, void* last, void (*destroy_range_00878ac0)(void*, void*, void*));
void damageable_fake_effect_vector_growth_catch_0087c518(Left55Host& host, void* allocator,
    void* first, void* last, void (*destroy_rows_0087b5b0)(void*, void*, void*));

// 00880080 __stdcall(columns, width, height, bool single_step), RET 10h.
void byte_grid_propagate_falloff_00880080(Left55Host& host, std::int8_t** columns,
    std::int32_t width, std::int32_t height, bool single_step);

// 00880220 __thiscall(ECX owner, argument), RET 4.
void forward_handle_00880220(Left55Host& host, void* owner, std::uint32_t argument);

// 00880250 __fastcall(ECX owner), plain RET, float in ST0.
float transform_unit_length_00880250(Left55Host& host, void* owner);

// 008840B0 __thiscall(ECX left, right), RET 4: -1/0/1.
std::int32_t native_string_compare_008840b0(const NativeStr* left, const NativeStr* right);
// 00884460 __thiscall(ECX left, right), RET 4: left < right.
bool lua_key_less_00884460(const LuaKey* left, const LuaKey* right);
// 008845A0 __thiscall(ECX pending, name), RET 4.
void pending_results_get_field_008845a0(Left55Host& host, PendingResults* pending,
    const char* name);
// 00884660 __thiscall(ECX map, key), RET 4: lower_bound.
VariantMapNode* variant_map_lower_bound_00884660(VariantMap* map, const LuaKey* key);
// 008849A0 __fastcall(ECX), RET: four dwords cleared (+4, +8, +0Ch, then +0).
void clear_four_dwords_008849a0(std::uint32_t* object);
// 008849D0 / 00884A00 / 00884A30 / 00884A90 / 00884AC0: variant constructors, __thiscall RET 4.
LuaVariant* variant_construct_number_008849d0(LuaVariant* v, float value);
LuaVariant* variant_construct_int_00884a00(LuaVariant* v, std::int32_t value);
LuaVariant* variant_construct_bool_00884a30(LuaVariant* v, bool value);
LuaVariant* variant_construct_entity_00884a90(LuaVariant* v, const void* entity);
LuaVariant* variant_construct_entity_record_00884ac0(LuaVariant* v, const void* record);
// 00884AF0 __fastcall(ECX), RET: the destructor.
void variant_destruct_00884af0(Left55Host& host, LuaVariant* v);
// 00884B60 __thiscall(ECX holder, out), RET 4: payload of every node, in order.
std::uint32_t* variant_map_collect_payloads_00884b60(Left55Host& host, void* holder,
    std::uint32_t* out);
// 00884CB0 __fastcall(ECX), RET.
void* list_construct_00884cb0(Left55Host& host, void* list);
// 00884D90 __thiscall(ECX key, const char*), RET 4 / 00884E50 (key, NativeStr by value), RET 8.
LuaKey* lua_key_construct_text_00884d90(Left55Host& host, LuaKey* key, const char* text);
LuaKey* lua_key_construct_moved_00884e50(Left55Host& host, LuaKey* key, NativeStr text);
// 00884EF0 __thiscall(ECX key, source), RET 4: copy constructor.
LuaKey* lua_key_copy_00884ef0(Left55Host& host, LuaKey* key, const LuaKey* source);
// 00884FC0 __thiscall(ECX variant, const NativeStr*), RET 4.
LuaVariant* variant_construct_string_00884fc0(Left55Host& host, LuaVariant* v,
    const NativeStr* text);
// 00885070 ECX map, EDX name, RET 4 (one stack dword never read).
const char* variant_map_find_string_00885070(Left55Host& host, VariantMap* map,
    const char* name);
// 00885400 __thiscall(ECX key, out), RET 4.
NativeStr* lua_key_to_text_00885400(Left55Host& host, const LuaKey* key, NativeStr* out);
// 00885470 __thiscall(ECX variant, out), RET 4.
NativeStr* variant_to_short_text_00885470(Left55Host& host, const LuaVariant* v, NativeStr* out);
// 008855B0 __thiscall(ECX variant, NativeStr* text), RET 4: appends.
void variant_append_text_008855b0(Left55Host& host, const LuaVariant* v, NativeStr* text);
// 00885A70 __thiscall(ECX variant, key, instance), RET 8: stores into the current Lua table.
void variant_store_in_table_00885a70(Left55Host& host, const LuaVariant* v, const LuaKey* key,
    void* instance);
// 00886840 __thiscall(ECX variant, const VariantMap*), RET 4.
LuaVariant* variant_construct_map_00886840(Left55Host& host, LuaVariant* v,
    const VariantMap* source);
// 00886920 __fastcall(ECX): payload release, tag = -1.
void variant_clear_00886920(Left55Host& host, LuaVariant* v);
// Setters, __thiscall RET 4.
void variant_set_number_008869d0(Left55Host& host, LuaVariant* v, float value);
void variant_set_int_008869f0(Left55Host& host, LuaVariant* v, std::int32_t value);
void variant_set_string_00886a10(Left55Host& host, LuaVariant* v, const NativeStr* text);
void variant_set_bool_00886ac0(Left55Host& host, LuaVariant* v, bool value);
void variant_set_entity_00886ae0(Left55Host& host, LuaVariant* v, const void* entity);
void variant_set_id_00886b10(Left55Host& host, LuaVariant* v, std::uint16_t id);
void variant_set_pointer_00886b30(Left55Host& host, LuaVariant* v, void* pointer);
void variant_set_map_00886b50(Left55Host& host, LuaVariant* v, const VariantMap* source);
// 00886C10 __thiscall(ECX, source), RET 4: assignment. 00886DA0: copy constructor.
LuaVariant* variant_assign_00886c10(Left55Host& host, LuaVariant* v, const LuaVariant* source);
LuaVariant* variant_copy_construct_00886da0(Left55Host& host, LuaVariant* v,
    const LuaVariant* source);
// 00887120 __thiscall(ECX sink, count, options), RET 8.
void result_sink_collect_00887120(Left55Host& host, ResultSink* sink, std::int32_t count,
    std::int32_t options);
// 00887370 __thiscall(ECX sink, first, last, options), RET 0Ch.
void result_sink_collect_range_00887370(Left55Host& host, ResultSink* sink, std::int32_t first,
    std::int32_t last, std::int32_t options);
// 00887A10 __thiscall(ECX, source), RET 4.
DeferredCall* deferred_call_copy_00887a10(Left55Host& host, DeferredCall* call,
    const DeferredCall* source);
// 00887E30 / 008882B0 __fastcall(ECX), RET: two identical queue destructors.
void call_queue_destroy_00887e30(Left55Host& host, void* queue);
void call_queue_destroy_008882b0(Left55Host& host, void* queue);
// 00888960 / 00888BE0 __thiscall(ECX LuaObject), RET: the "Ptr" field's userdata.
void* lua_object_ptr_field_00888960(Left55Host& host, void* object);
void* lua_object_ptr_field_00888be0(Left55Host& host, void* object);
// 00888C40 __thiscall(ECX LuaObject), RET.
bool lua_object_is_kind2_entity_00888c40(Left55Host& host, void* object);
// 00888F30 __thiscall(ECX message, const float* position, float value, int value), RET 0Ch.
PositionMessage97* position_message_construct_00888f30(Left55Host& host,
    PositionMessage97* message, const float* position, float value_24, std::int32_t value_28);
// 00888FC0 __thiscall(ECX node, float alpha), RET 4.
void scene_node_set_alpha_recursive_00888fc0(Left55Host& host, void* node, float alpha);
// 008890B0 __fastcall(ECX), RET.
void singleton_destruct_008890b0(Left55Host& host, std::uint32_t* object);
// 00889610 __fastcall(ECX), RET.
void owned_block_release_00889610(Left55Host& host, void** slot);
// 008898C0 __fastcall(ECX first, EDX hole, stack top, value), RET 0Ch: VC9 _Push_heap.
void scored_push_heap_008898c0(ScoredEntry* first, std::int32_t hole, std::int32_t top,
    ScoredEntry value);
// 00889ED0 __stdcall(value), RET 4: a list node {0, 0, value}.
std::uint32_t* list_node_allocate_00889ed0(Left55Host& host, std::uint32_t value);
// 00889F10 __fastcall(ECX), RET.
void* object_if_kind_2b_00889f10(Left55Host& host, void* object);
// 0088A0C0 __fastcall(ECX a, EDX b, stack c), RET 4: VC9 _Med3.
void scored_med3_0088a0c0(ScoredEntry* a, ScoredEntry* b, ScoredEntry* c);
// 0088A130 __fastcall(ECX first, EDX hole, stack bottom, value), RET 0Ch: _Adjust_heap.
void scored_adjust_heap_0088a130(ScoredEntry* first, std::int32_t hole, std::int32_t bottom,
    ScoredEntry value);
// 0088A1E0 __fastcall(ECX first, EDX last, stack dest, 3 unused), RET 10h.
ScoredEntry* scored_uninitialized_copy_0088a1e0(const ScoredEntry* first,
    const ScoredEntry* last, ScoredEntry* dest);
// 0088A2A0 __thiscall(ECX slot, value), RET 4.
void** refcounted_slot_assign_0088a2a0(Left55Host& host, void** slot, void* value);
// 0088A570 __cdecl(first, last, const value*).
void scored_fill_0088a570(ScoredEntry* first, ScoredEntry* last, const ScoredEntry* value);
// 0088A5C0 __fastcall(ECX dest, EDX count, stack value*, 3 unused), RET 10h.
void scored_uninitialized_fill_n_0088a5c0(ScoredEntry* dest, std::uint32_t count,
    const ScoredEntry* value);
// 0088A650 __fastcall(ECX first, EDX mid, stack last), RET 4: _Median.
void scored_median_0088a650(ScoredEntry* first, ScoredEntry* mid, ScoredEntry* last);
// 0088A6D0 __fastcall(ECX first, EDX last, 2 unused stack dwords), RET 8: _Make_heap.
void scored_make_heap_0088a6d0(ScoredEntry* first, ScoredEntry* last);
// 0088A720 __fastcall(ECX first, EDX last): _Insertion_sort.
void scored_insertion_sort_0088a720(ScoredEntry* first, ScoredEntry* last);
// 0088A900 ECX = PointerArray, EBP = the Lua table object (register input): returns 1.
bool lua_table_collect_ptr_fields_0088a900(Left55Host& host, PointerArray* array, void* table);
// 0088AAF0 __fastcall(ECX tree): head node, isnil byte +19h.
void tree_construct_0088aaf0(Left55Host& host, void* tree);
// 0088ABD0 __cdecl(first, last, dest_end): copy_backward.
ScoredEntry* scored_copy_backward_0088abd0(const ScoredEntry* first, const ScoredEntry* last,
    ScoredEntry* dest_end);
// 0088AC20 __fastcall(ECX out pair, EDX first, stack last): _Unguarded_partition.
void scored_unguarded_partition_0088ac20(ScoredEntry** out, ScoredEntry* first,
    ScoredEntry* last);
// 0088B120 __fastcall(ECX map) / 0088B190 __fastcall(ECX variant): a walk that does nothing
// but recurse into nested tables (with the checked-iterator tests).
void variant_map_walk_0088b120(Left55Host& host, VariantMap* map);
void variant_walk_0088b190(Left55Host& host, LuaVariant* v);
// 0088B360 __fastcall(ECX choice), RET.
void end_scene_choice_0088b360(Left55Host& host, std::int32_t choice);

// The parameter block 00880420 reads: four floats and the dilation block size at +8h.
struct ShoreMaskParams {
    float depth;          // +00h, cells with height < -depth are water (0)
    float cell_size;      // +04h
    std::int32_t block;   // +08h, passed to 0087FA90
    float falloff;        // +0Ch
    float ring_width;     // +10h
};

// 00880420 __thiscall(ECX owner, columns, step, const ShoreMaskParams*), RET 0Ch.
void landscape_build_shore_mask_00880420(Left55Host& host, void* owner, std::int8_t** columns,
    std::int32_t step, const ShoreMaskParams* params);

// ---------------------------------------------------------------------------
// Segment 56: helpers and the first ten bindings (0088B410..0088DAA0, 008D3E00)
// ---------------------------------------------------------------------------
// The other bindings of segment 56 are in cc13_left55_seg56a/b/c.hpp. The bindings here follow
// docs/CC13_SEG56.md "Shared shape": the dead `luaMW_<name> failed:` / `luakod` prologue, the
// state wrap (00B66C00) and the call frame (00B679B0) are not reconstructed; each function starts
// at its first argument read and returns what the native returns in EAX.

// The record 0088B410 assigns and 0088B5A0 copies: two dwords, a NativeString, a byte.
struct NamedPairRecord {
    std::uint32_t first;   // +00h
    std::uint32_t second;  // +04h
    NativeStr name;        // +08h
    std::uint8_t flag;     // +10h
};

// 0088B550's record: a NativeString and a dword.
struct NamedValue {
    NativeStr name;        // +00h
    std::uint32_t value;   // +08h
};

// Session messages E2h (vtable 00D03644) and E7h (vtable 00D03658) built by 0088B6D0 / 0088B7B0.
struct NamedSessionMessage {
    std::uint8_t base[0x18];  // +00h..+17h written by 0075B430, then +4h = 1
    NativeStr name;           // +18h
    std::uint32_t value_20;   // +20h (E2h only)
    std::uint32_t value_24;   // +24h (E2h only)
};

// Variant array 0088BBC0 walks: {data, count}.
struct VariantArray {
    LuaVariant* data;
    std::int32_t count;
};

inline constexpr std::uint32_t kNamedMessageE2Vtable = 0x00D03644u;
inline constexpr std::uint32_t kNamedMessageE7Vtable = 0x00D03658u;
inline constexpr std::uint32_t kMultiLobbySettingsVtable = 0x00CEF164u;

struct Left56Host : Left55Host {
    // --- image globals ---
    virtual float now_00f876a4() = 0;                                  // FLD [00F876A4]
    virtual std::int32_t script_nesting_00e1ae84() = 0;                // [00E1AE84]
    virtual void set_script_nesting_00e1ae84(std::int32_t value) = 0;
    virtual void write_global_00f8894c(std::uint32_t value) = 0;       // the lobby singleton slot
    virtual void* packet_statistics_00f871b4() = 0;                    // [00F871B4]

    // --- Lua objects used by the helpers ---
    virtual bool lua_object_is_string_00b660a0(void* object) = 0;
    virtual bool lua_object_is_number_00b66050(void* object) = 0;
    virtual const char* lua_object_get_string_00b662b0(void* object) = 0;
    virtual bool lua_value_is_entity_handle_008889c0(void* object) = 0;
    virtual bool native_string_equals_ci_00425850(const NativeStr* s, const char* text) = 0;
    virtual int stricmp_00bf7fbf(const char* a, const char* b) = 0;

    // --- 008D3E00's five maps ---
    virtual void map_iterator_increment_005672d0(void* iterator) = 0;  // inner maps
    virtual void map_iterator_increment_00569b00(void* iterator) = 0;  // outer maps (+10h, +34h)
    // __thiscall(map, out iterator, first {map, node}, last {map, node}) for each map type.
    virtual void map_erase_range_00576120(void* map, void* out, void* first_map, void* first_node,
        void* last_map, void* last_node) = 0;
    virtual void map_erase_range_00576740(void* map, void* out, void* first_map, void* first_node,
        void* last_map, void* last_node) = 0;
    virtual void map_erase_range_005761f0(void* map, void* out, void* first_map, void* first_node,
        void* last_map, void* last_node) = 0;
    virtual void map_erase_range_005724d0(void* map, void* out, void* first_map, void* first_node,
        void* last_map, void* last_node) = 0;

    // --- binding callees ---
    // LogToFile: 006B7E30 __thiscall(holder, lua_State*), 008875F0 __thiscall(sink, holder),
    // 00887670 __thiscall(sink) (the sink's teardown), 006B7E50 __thiscall(holder).
    virtual void lua_state_holder_construct_006b7e30(void* holder, void* lua_state) = 0;
    virtual void result_sink_construct_008875f0(ResultSink* sink, void* holder) = 0;
    virtual void result_sink_teardown_00887670(ResultSink* sink) = 0;
    virtual void lua_state_holder_destruct_006b7e50(void* holder) = 0;
    // DoFileOutsideMPAK: 00B69D40 __thiscall(ECX = the binding's borrowed state owner, const
    // NativeString* name, 0), RET 8.
    virtual void run_script_with_overrides_00b69d40(const NativeStr* name, std::int32_t flag) = 0;
    // AddWatch / RemoveWatch: __thiscall(ECX = [game+21CCh], NativeString by value).
    virtual void watch_add_00922240(void* watches, NativeStr name) = 0;
    virtual void watch_remove_009222e0(void* watches, NativeStr name) = 0;
    // NetworkLog: 00789560 __thiscall(ECX = [00F871B4]), AL result.
    virtual bool packet_statistics_start_csv_00789560(void* statistics) = 0;
    // GetGuiName: entity vtable+14h, returns a C string; 00B66710 pushes it.
    virtual const char* entity_gui_name_vcall_14(void* entity) = 0;
    virtual void push_c_string_00b66710(const char* text) = 0;
};

// 0088B410 __thiscall(ECX, source), RET 4: assignment.
NamedPairRecord* named_pair_assign_0088b410(Left56Host& host, NamedPairRecord* record,
    const NamedPairRecord* source);
// 0088B4B0 __thiscall(ECX entity, const NativeStr* name, float offset), RET 8.
void entity_start_anim_0088b4b0(Left56Host& host, void* entity, const NativeStr* name,
    float offset);
// 0088B520 __fastcall(ECX tree), RET: returns the tree.
void* tree_construct_0088b520(Left56Host& host, void* tree);
// 0088B550 __thiscall(ECX, source), RET 4: copy constructor.
NamedValue* named_value_copy_0088b550(Left56Host& host, NamedValue* value, const NamedValue* source);
// 0088B5A0 __thiscall(ECX, source), RET 4: copy constructor.
NamedPairRecord* named_pair_copy_0088b5a0(Left56Host& host, NamedPairRecord* record,
    const NamedPairRecord* source);
// 0088B650 __fastcall(ECX first, EDX last), RET: VC9 _Sort_heap over ScoredEntry.
void scored_sort_heap_0088b650(ScoredEntry* first, ScoredEntry* last);
// 0088B6D0 __thiscall(ECX message, const NativeStr*, value_20, value_24), RET 0Ch.
NamedSessionMessage* named_message_e2_construct_0088b6d0(Left56Host& host,
    NamedSessionMessage* message, const NativeStr* name, std::uint32_t value_20,
    std::uint32_t value_24);
// 0088B7B0 __thiscall(ECX message, const NativeStr*), RET 4.
NamedSessionMessage* named_message_e7_construct_0088b7b0(Left56Host& host,
    NamedSessionMessage* message, const NativeStr* name);
// 0088B840 __thiscall(ECX LuaObject), RET: a table of exactly three string-keyed numbers
// named x, y or z (case-insensitive; repeats count).
bool lua_object_is_vector3_table_0088b840(Left56Host& host, void* object);
// 0088BA00 __thiscall(ECX LuaObject), RET.
bool lua_object_is_entity_or_vector3_0088ba00(Left56Host& host, void* object);
// 0088BBC0 __fastcall(ECX array), RET.
void variant_array_walk_0088bbc0(Left56Host& host, VariantArray* array);
// 008D3E00 __fastcall(ECX), RET: the multiplayer lobby settings destructor.
void multi_lobby_settings_destroy_008d3e00(Left56Host& host, void* settings);

// Bindings. LogToFile takes the binding's lua_State* (ECX) because 006B7E30 receives it.
int lua_log_to_file_0088bc00(Left56Host& host, void* lua_state);
int lua_do_file_outside_mpak_0088bd20(cc13_seg56::LuaArgs& args, Left56Host& host);
int lua_log_0088bec0();
int lua_dump_0088bf20();
int lua_assert_0088c3e0(cc13_seg56::LuaArgs& args);
int lua_add_watch_0088d130(cc13_seg56::LuaArgs& args, Left56Host& host);
int lua_log_to_console_0088d420(cc13_seg56::LuaArgs& args);
int lua_remove_watch_0088d560(cc13_seg56::LuaArgs& args, Left56Host& host);
int lua_network_log_0088d6e0(cc13_seg56::LuaArgs& args, cc13_seg56::LuaResults& results,
    Left56Host& host);
int lua_get_gui_name_0088daa0(cc13_seg56::LuaArgs& args, Left56Host& host);

} // namespace bsp::cc13_left55
