// Packet cc13-left55, segment 55 leftovers. See include/bsp/cc13_left55.hpp for the layouts and
// ABI, docs/CC13_LEFT55.md for the per-function evidence. Comments cite the instruction or
// pseudocode statement each step stands for.
#include "bsp/cc13_left55.hpp"

#include <cmath>
#include <cstring>

namespace bsp::cc13_left55 {

void owner_clear_byte_290_00888fb0(void* owner) noexcept {
    static_cast<std::uint8_t*>(owner)[0x290] = 0;
}
void* byte_message_9f_construct_00889940(Left55Host& host, void* message, std::uint8_t value) {
    auto* p = static_cast<std::uint8_t*>(message);
    host.session_message_base_0075b430(message, 0x9f);
    *reinterpret_cast<std::uint16_t*>(p + 0x18) = 0;
    p[0x1a] = 0;
    p[0x1c] = value;
    *reinterpret_cast<std::uint32_t*>(p + 4) = 1;
    *reinterpret_cast<std::uint32_t*>(p) = 0x00d03360;
    return message;
}
namespace {

// The inlined NativeString copy assignment: resize to the source size, then copy when non-empty.
void copy_into(Left55Host& host, NativeStr* dst, const NativeStr* src)
{
    if (dst != src) {
        host.native_string_resize_0041dd40(dst, src->size, true);
        if (src->size != 0) {
            std::memcpy(dst->data, src->data, dst->size);
        }
    }
}

// The inlined ~NativeString: return the buffer to the sized pool.
void release(Left55Host& host, const NativeStr& s)
{
    if (s.data != nullptr) {
        host.pool_release_00419cc0_00bd1510(s.data, s.size + 1);
    }
}

// The inlined NativeString append: grow by the source size and copy it after the old end.
void append_inline(Left55Host& host, NativeStr* dst, const NativeStr& src)
{
    if (src.size != 0) {
        const std::uint32_t old = dst->size;
        host.native_string_resize_0041dd40(dst, src.size + old, true);
        std::memcpy(dst->data + old, src.data, src.size);
    }
}

// Assign a literal to a temporary, append it inline, release it.
void append_literal(Left55Host& host, NativeStr* dst, const char* text)
{
    NativeStr tmp{};
    host.native_string_assign_0041e870(&tmp, text);
    append_inline(host, dst, tmp);
    release(host, tmp);
}

VariantMap* new_empty_map(Left55Host& host)
{
    auto* map = static_cast<VariantMap*>(host.operator_new_00bf681b(0x0C));
    if (map == nullptr) {
        return nullptr;
    }
    VariantMapNode* head = host.map_allocate_head_006edea0(map);
    map->head = head;
    head->is_nil = 1;
    map->head->parent = map->head;
    map->head->left = map->head;
    map->head->right = map->head;
    map->size = 0;
    return map;
}

// The `*this = source` tail of 00886840 / 00886B50: erase(begin, end), then copy the tree. The
// native does this without testing the fresh map for null, as here.
void assign_map(Left55Host& host, VariantMap* map, const VariantMap* source)
{
    if (map != source) {
        VariantMapIterator out{};
        host.map_erase_range_006ee960(map, &out, {map, map->head->left}, {map, map->head});
        host.map_copy_00781890(map, source);
    }
}

const char* or_empty(const char* text)
{
    return text != nullptr ? text : ""; // the native substitutes 00F87904, an empty string
}

void* lua_state_of_scope(void* const scope[2])
{
    // MOV ECX,[scope+4] (the instance); MOV ECX,[ECX+4] (its lua_State slot).
    return *reinterpret_cast<void* const*>(static_cast<const char*>(scope[1]) + 4);
}

} // namespace

// ---------------------------------------------------------------------------

void damageable_section_vector_growth_catch_0087bb02(Left55Host& host, void* allocator,
    void* first, void* last, void (*destroy_range_00878ac0)(void*, void*, void*))
{
    destroy_range_00878ac0(allocator, first, last); // ECX=[EBP-14h], PUSH [EBP+10h], PUSH ESI
    host.free_00bf65ac(first);                      // 0087BB13
    host.rethrow_00bf6885();                        // 0087BB1F, _CxxThrowException(0, 0)
}

void damageable_fake_effect_vector_growth_catch_0087c518(Left55Host& host, void* allocator,
    void* first, void* last, void (*destroy_rows_0087b5b0)(void*, void*, void*))
{
    destroy_rows_0087b5b0(allocator, first, last);
    host.free_00bf65ac(first);
    host.rethrow_00bf6885();
}

void byte_grid_propagate_falloff_00880080(Left55Host& host, std::int8_t** columns,
    std::int32_t width, std::int32_t height, bool single_step)
{
    // The dirty rectangle starts as the whole grid: min = 0, max = width / height.
    std::int32_t min_x = 0;
    std::int32_t max_x = width;
    std::int32_t min_y = 0;
    std::int32_t max_y = height;
    std::int32_t step = 2;
    bool settled;
    do {
        settled = true;                                   // byte [ESP+13h] = 1
        const std::int32_t x_lo = (min_x - 1) < 0 ? 0 : min_x - 1;
        const std::int32_t x_hi = (max_x + 2) < width ? max_x + 2 : width;
        const std::int32_t y_lo = (min_y - 1) < 0 ? 0 : min_y - 1;
        const std::int32_t y_hi = (max_y + 2) < height ? max_y + 2 : height;
        min_x = x_hi;
        max_x = 0;
        min_y = y_hi;
        max_y = 0;
        step = single_step ? 1 : 3 - step;                // 0088011F / 00880129
        if (x_lo >= x_hi) {
            return;                                       // 00880134 JGE 008801FE
        }
        for (std::int32_t x = x_lo; x < x_hi; ++x) {
            std::int8_t* column = columns[x];
            for (std::int32_t y = y_lo; y < y_hi; ++y) {
                std::int8_t highest = 0;
                for (std::int32_t direction = 0; direction < 8; direction += step) {
                    const std::int8_t sample =
                        host.byte_grid_sample_00412070(columns, width, height, x, y, direction, 0);
                    if (highest <= sample) {              // CMP BL,AL / JG
                        highest = sample;
                    }
                }
                if (column[y] + 1 < highest) {            // MOVSX compares
                    column[y] = static_cast<std::int8_t>(highest - 1);
                    settled = false;
                    if (x < min_x) min_x = x;
                    if (max_x < x) max_x = x;
                    if (y < min_y) min_y = y;
                    if (max_y < y) max_y = y;
                }
            }
        }
    } while (!settled);
}

void forward_handle_00880220(Left55Host& host, void* owner, std::uint32_t argument)
{
    void* handle = *reinterpret_cast<void**>(static_cast<char*>(owner) + 0x42C);
    if (handle != nullptr) {
        char* game = static_cast<char*>(host.game_global_00e188a8());
        char* system = *reinterpret_cast<char**>(game + 0x19F0);
        void* sink = *reinterpret_cast<void**>(system + 0xA8);
        host.handle_sink_vcall_20(sink, handle, argument); // CALL [vtable+20h], ECX = sink
    }
}

float transform_unit_length_00880250(Left55Host& host, void* owner)
{
    void* transform = *reinterpret_cast<void**>(static_cast<char*>(owner) + 0x3D0);
    float origin[2];
    float unit[2];
    host.transform_vcall_1c(transform, origin, 0.0f, 0.0f); // 00880272
    host.transform_vcall_1c(transform, unit, 1.0f, 0.0f);   // 00880292 (FLD1 as x)
    const float dx = origin[0] - unit[0];
    const float dy = origin[1] - unit[1];
    const float squared = dx * dx + dy * dy;                 // FSTP float
    if (static_cast<double>(squared) > 1e-10) {             // double 00CE3820
        return static_cast<float>(std::sqrt(static_cast<double>(squared))); // 00BF7030
    }
    return 0.0f;
}

std::int32_t native_string_compare_008840b0(const NativeStr* left, const NativeStr* right)
{
    if (left->size == 0) {
        return right->size != 0 ? -1 : 0; // NEG / SBB
    }
    if (right->size == 0) {
        return 1;
    }
    const auto* a = reinterpret_cast<const unsigned char*>(left->data);
    const auto* b = reinterpret_cast<const unsigned char*>(right->data);
    for (;;) {
        if (*a != *b) {
            return *a < *b ? -1 : 1;
        }
        if (*a == 0) {
            return 0;
        }
        ++a;
        ++b;
    }
}

bool lua_key_less_00884460(const LuaKey* left, const LuaKey* right)
{
    if (left->kind < right->kind) {
        return true;
    }
    if (left->kind > right->kind) {
        return false;
    }
    if (left->kind == 0) {
        return left->number < right->number;                              // 0088449E
    }
    if (left->kind == 1) {
        return native_string_compare_008840b0(&left->text, &right->text) < 0; // 0088448C
    }
    return false;
}

void pending_results_get_field_008845a0(Left55Host& host, PendingResults* pending,
    const char* name)
{
    if (pending->armed != 0) {
        char* game = static_cast<char*>(host.game_global_00e188a8());
        char* scripting = *reinterpret_cast<char**>(game + 0x1A08);
        void* machine = *reinterpret_cast<void**>(scripting + 4);
        host.lua_machine_push_string_006b8120(machine, name);
        pending->stack_index += -1;                                   // ADD [ESI+8],-1
        host.lua_machine_get_table_006b8470(machine, pending->stack_index);
    }
}

VariantMapNode* variant_map_lower_bound_00884660(VariantMap* map, const LuaKey* key)
{
    VariantMapNode* result = map->head;
    VariantMapNode* node = map->head->parent; // the root
    while (node->is_nil == 0) {
        bool node_less;
        if (node->key.kind < key->kind) {
            node_less = true;
        } else if (node->key.kind > key->kind) {
            node_less = false;
        } else if (node->key.kind == 0) {
            node_less = node->key.number < key->number;
        } else if (node->key.kind == 1) {
            node_less = native_string_compare_008840b0(&node->key.text, &key->text) < 0;
        } else {
            node_less = false;
        }
        if (node_less) {
            node = node->right;
        } else {
            result = node;
            node = node->left;
        }
    }
    return result;
}

void clear_four_dwords_008849a0(std::uint32_t* object)
{
    object[1] = 0;
    object[2] = 0;
    object[3] = 0;
    object[0] = 0;
}

LuaVariant* variant_construct_number_008849d0(LuaVariant* v, float value)
{
    v->vtable = kLuaVariantVtable;
    v->tag = 0;
    v->name = {};
    v->payload.number = value;
    return v;
}

LuaVariant* variant_construct_int_00884a00(LuaVariant* v, std::int32_t value)
{
    v->vtable = kLuaVariantVtable;
    v->tag = 0;
    v->name = {};
    v->payload.number = static_cast<float>(value); // FILD / FSTP float
    return v;
}

LuaVariant* variant_construct_bool_00884a30(LuaVariant* v, bool value)
{
    v->vtable = kLuaVariantVtable;
    v->tag = 2;
    v->name = {};
    v->payload.boolean = value; // byte store only
    return v;
}

LuaVariant* variant_construct_entity_00884a90(LuaVariant* v, const void* entity)
{
    v->vtable = kLuaVariantVtable;
    v->tag = 3;
    v->name = {};
    std::uint16_t id = 0;
    if (entity != nullptr) {
        id = *reinterpret_cast<const std::uint16_t*>(static_cast<const char*>(entity) + 0x174);
    }
    v->payload.id = id;
    return v;
}

LuaVariant* variant_construct_entity_record_00884ac0(LuaVariant* v, const void* record)
{
    v->vtable = kLuaVariantVtable;
    v->tag = 3;
    v->name = {};
    v->payload.id = *reinterpret_cast<const std::uint16_t*>(static_cast<const char*>(record) + 0x18);
    return v;
}

void variant_destruct_00884af0(Left55Host& host, LuaVariant* v)
{
    v->vtable = kLuaVariantVtable;
    variant_clear_00886920(host, v);
    release(host, v->name);
}

std::uint32_t* variant_map_collect_payloads_00884b60(Left55Host& host, void* holder,
    std::uint32_t* out)
{
    auto holder_map = [holder]() {
        return *reinterpret_cast<VariantMap**>(static_cast<char*>(holder) + 8);
    };
    VariantMapIterator it{holder_map(), holder_map()->head->left};
    std::uint32_t* cursor = out;
    for (;;) {
        VariantMapNode* end = holder_map()->head;
        if (it.container == nullptr || it.container != holder_map()) {
            host.invalid_parameter_00bf6713();
        }
        if (it.node == end) {
            break;
        }
        if (it.container == nullptr) {
            host.invalid_parameter_00bf6713();
        }
        if (it.node == it.container->head) {
            host.invalid_parameter_00bf6713();
        }
        *cursor = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(it.node->value.payload.pointer));
        host.map_iterator_increment_006eda20(&it);
        ++cursor;
    }
    return out;
}

void* list_construct_00884cb0(Left55Host& host, void* list)
{
    auto* words = static_cast<void**>(list);
    words[1] = host.list_allocate_head_00884830();
    words[2] = nullptr;
    return list;
}

LuaKey* lua_key_construct_text_00884d90(Left55Host& host, LuaKey* key, const char* text)
{
    key->text = {};
    NativeStr tmp{};
    host.native_string_assign_0041e870(&tmp, text);
    copy_into(host, &key->text, &tmp);
    release(host, tmp);
    key->kind = 1;
    return key;
}

LuaKey* lua_key_construct_moved_00884e50(Left55Host& host, LuaKey* key, NativeStr text)
{
    key->text = {};
    copy_into(host, &key->text, &text);
    key->kind = 1;
    release(host, text); // the by-value argument dies at the end
    return key;
}

LuaKey* lua_key_copy_00884ef0(Left55Host& host, LuaKey* key, const LuaKey* source)
{
    key->text = {};
    key->kind = source->kind;
    if (source->kind == 0) {
        key->number = source->number;
        return key;
    }
    if (source->kind == 1) {
        copy_into(host, &key->text, &source->text);
    }
    return key;
}

LuaVariant* variant_construct_string_00884fc0(Left55Host& host, LuaVariant* v,
    const NativeStr* text)
{
    v->vtable = kLuaVariantVtable;
    v->tag = 1;
    v->name = {};
    auto* copy = static_cast<NativeStr*>(host.operator_new_00bf681b(8));
    if (copy == nullptr) {
        v->payload.text = nullptr;
    } else {
        *copy = {};
        copy_into(host, copy, text);
        v->payload.text = copy;
    }
    return v;
}

const char* variant_map_find_string_00885070(Left55Host& host, VariantMap* map,
    const char* name)
{
    LuaKey key{};
    lua_key_construct_text_00884d90(host, &key, name);
    VariantMapIterator it{};
    host.map_find_00884d20(map, &it, &key);
    release(host, key.text);
    VariantMapNode* end = map->head;
    if (it.container == nullptr || it.container != map) {
        host.invalid_parameter_00bf6713();
    }
    if (it.node == end) {
        return nullptr;
    }
    if (it.container == nullptr) {
        host.invalid_parameter_00bf6713();
    }
    if (it.node == it.container->head) {
        host.invalid_parameter_00bf6713();
    }
    return or_empty(it.node->value.payload.text->data); // [[node+24h]+4]
}

NativeStr* lua_key_to_text_00885400(Left55Host& host, const LuaKey* key, NativeStr* out)
{
    if (key->kind == 1) {
        *out = {};
        copy_into(host, out, &key->text);
        return out;
    }
    host.native_string_from_int_004260b0(out, key->number); // 0088545A
    return out;
}

NativeStr* variant_to_short_text_00885470(Left55Host& host, const LuaVariant* v, NativeStr* out)
{
    switch (v->tag) {
    case 0:
        host.native_string_from_float_0043bdf0(out, v->payload.number);
        return out;
    case 1: {
        NativeStr quote{};
        host.native_string_assign_0041e870(&quote, "\"");                 // 008854F4
        NativeStr head{};
        NativeStr* prefixed = host.native_string_prefixed_0043c130(&head, "\"", v->payload.text);
        host.native_string_concat_004261a0(prefixed, out, &quote);        // 00885524
        release(host, head);
        release(host, quote);
        return out;
    }
    case 5:
        host.native_string_assign_0041e870(out, "{Map}");
        return out;
    default:
        host.native_string_assign_0041e870(out, "");
        return out;
    }
}

void variant_append_text_008855b0(Left55Host& host, const LuaVariant* v, NativeStr* text)
{
    switch (v->tag) {
    case 0:
        host.native_string_append_float_00781740(text, v->payload.number);
        return;
    case 1:
        append_literal(host, text, "\"");
        append_inline(host, text, *v->payload.text);
        append_literal(host, text, "\"");
        return;
    case 5: {
        NativeStr brace{};
        host.native_string_assign_0041e870(&brace, "{");
        host.native_string_append_00425e10(text, &brace);
        release(host, brace);
        VariantMapIterator it{v->payload.map, v->payload.map->head->left};
        for (;;) {
            VariantMapNode* end = v->payload.map->head;
            if (it.container == nullptr || it.container != v->payload.map) {
                host.invalid_parameter_00bf6713();
            }
            if (it.node == end) {
                break;
            }
            if (it.container == nullptr) {
                host.invalid_parameter_00bf6713();
            }
            if (it.node == it.container->head) {
                host.invalid_parameter_00bf6713();
            }
            NativeStr key_text{};
            lua_key_to_text_00885400(host, &it.node->key, &key_text);
            append_inline(host, text, key_text);
            release(host, key_text);
            append_literal(host, text, "=");
            if (it.node == it.container->head) {
                host.invalid_parameter_00bf6713();
            }
            variant_append_text_008855b0(host, &it.node->value, text);
            append_literal(host, text, ";");
            host.map_iterator_increment_006eda20(&it);
        }
        append_literal(host, text, "}");
        return;
    }
    case 6: {
        NativeStr nil{};
        host.native_string_assign_0041e870(&nil, "<NIL>");
        host.native_string_append_00425e10(text, &nil);
        release(host, nil);
        return;
    }
    case 4: {
        NativeStr pointer{};
        host.native_string_assign_0041e870(&pointer, " <C Ptr>");
        host.native_string_append_00425e10(text, &pointer);
        host.native_string_destroy_storage_0041dd20(&pointer);
        return;
    }
    default:
        return; // tags 2 and 3 append nothing
    }
}

void variant_store_in_table_00885a70(Left55Host& host, const LuaVariant* v, const LuaKey* key,
    void* instance)
{
    auto walk_children = [&](const VariantMap* map) {
        VariantMapIterator it{const_cast<VariantMap*>(map), map->head->left};
        for (;;) {
            VariantMapNode* end = v->payload.map->head;
            if (it.container == nullptr || it.container != v->payload.map) {
                host.invalid_parameter_00bf6713();
            }
            if (it.node == end) {
                break;
            }
            if (it.container == nullptr) {
                host.invalid_parameter_00bf6713();
            }
            if (it.node == it.container->head) {
                host.invalid_parameter_00bf6713(); // value dereference
            }
            if (it.node == it.container->head) {
                host.invalid_parameter_00bf6713(); // key dereference
            }
            variant_store_in_table_00885a70(host, &it.node->value, &it.node->key, instance);
            host.map_iterator_increment_006eda20(&it);
        }
    };

    switch (v->tag) {
    case 0:
        if (key->kind == 0) {
            host.lua_table_set_number_number_006b82a0(instance,
                static_cast<float>(key->number), v->payload.number);
            return;
        }
        if (key->kind == 1) {
            NativeStr name{};
            host.key_text_copy_00781540(key, &name);
            host.lua_table_set_string_number_006b8260(instance, or_empty(name.data),
                v->payload.number);
            release(host, name);
        }
        return;
    case 1:
        if (key->kind == 0) {
            host.lua_table_set_number_string_006b8320(instance,
                static_cast<float>(key->number), or_empty(v->payload.text->data));
            return;
        }
        if (key->kind == 1) {
            NativeStr name{};
            host.key_text_copy_00781540(key, &name);
            const char* value = or_empty(v->payload.text->data);
            host.lua_table_set_string_string_006b8360(instance, or_empty(name.data), value);
            release(host, name);
        }
        return;
    case 5: {
        void* scope[2] = {};
        if (key->kind == 0) {
            host.table_scope_indexed_00803750(scope, instance, key->number);
        } else if (key->kind == 1) {
            NativeStr name{};
            host.key_text_copy_00781540(key, &name);
            host.table_scope_named_008037d0(scope, instance, or_empty(name.data));
            release(host, name);
        } else {
            return;
        }
        walk_children(v->payload.map);
        host.lua_settop_00a673e0(lua_state_of_scope(scope), -2); // EDX = -2
        return;
    }
    default:
        return;
    }
}

LuaVariant* variant_construct_map_00886840(Left55Host& host, LuaVariant* v,
    const VariantMap* source)
{
    v->vtable = kLuaVariantVtable;
    v->tag = 5;
    v->name = {};
    VariantMap* map = new_empty_map(host);
    v->payload.map = map;
    assign_map(host, map, source);
    return v;
}

void variant_clear_00886920(Left55Host& host, LuaVariant* v)
{
    if (v->tag == 1) {
        NativeStr* text = v->payload.text;
        if (text != nullptr) {
            release(host, *text);
            host.free_00bf65ac(text);
            v->payload.text = nullptr;
        }
    } else if (v->tag == 5) {
        VariantMap* map = v->payload.map;
        if (map != nullptr) {
            VariantMapIterator out{};
            host.map_erase_range_006ee960(map, &out, {map, map->head->left}, {map, map->head});
            host.free_00bf65ac(map->head);
            map->head = nullptr;
            map->size = 0;
            host.free_00bf65ac(map);
            v->payload.map = nullptr;
        }
    }
    v->tag = -1;
}

void variant_set_number_008869d0(Left55Host& host, LuaVariant* v, float value)
{
    variant_clear_00886920(host, v);
    v->tag = 0;
    v->payload.number = value;
}

void variant_set_int_008869f0(Left55Host& host, LuaVariant* v, std::int32_t value)
{
    variant_clear_00886920(host, v);
    v->tag = 0;
    v->payload.number = static_cast<float>(value);
}

void variant_set_string_00886a10(Left55Host& host, LuaVariant* v, const NativeStr* text)
{
    variant_clear_00886920(host, v);
    v->tag = 1;
    auto* copy = static_cast<NativeStr*>(host.operator_new_00bf681b(8));
    if (copy != nullptr) {
        *copy = {};
        copy_into(host, copy, text);
        v->payload.text = copy;
        return;
    }
    v->payload.text = nullptr;
}

void variant_set_bool_00886ac0(Left55Host& host, LuaVariant* v, bool value)
{
    variant_clear_00886920(host, v);
    v->tag = 2;
    v->payload.boolean = value;
}

void variant_set_entity_00886ae0(Left55Host& host, LuaVariant* v, const void* entity)
{
    variant_clear_00886920(host, v);
    v->tag = 3;
    if (entity != nullptr) {
        v->payload.id =
            *reinterpret_cast<const std::uint16_t*>(static_cast<const char*>(entity) + 0x174);
        return;
    }
    v->payload.id = 0;
}

void variant_set_id_00886b10(Left55Host& host, LuaVariant* v, std::uint16_t id)
{
    variant_clear_00886920(host, v);
    v->tag = 3;
    v->payload.id = id;
}

void variant_set_pointer_00886b30(Left55Host& host, LuaVariant* v, void* pointer)
{
    variant_clear_00886920(host, v);
    v->tag = 4;
    v->payload.pointer = pointer;
}

void variant_set_map_00886b50(Left55Host& host, LuaVariant* v, const VariantMap* source)
{
    variant_clear_00886920(host, v);
    v->tag = 5;
    VariantMap* map = new_empty_map(host);
    v->payload.map = map;
    assign_map(host, map, source);
}

LuaVariant* variant_assign_00886c10(Left55Host& host, LuaVariant* v, const LuaVariant* source)
{
    if (v == source) {
        return v;
    }
    variant_clear_00886920(host, v);
    v->tag = source->tag;
    switch (source->tag) {
    case 0:
    case 4:
        v->payload.pointer = source->payload.pointer; // dword copy
        break;
    case 1: {
        void* storage = host.operator_new_00bf681b(8);
        if (storage == nullptr) {
            v->payload.text = nullptr;
            break;
        }
        v->payload.text = host.native_string_copy_construct_00426060(storage, source->payload.text);
        break;
    }
    case 2:
        v->payload.boolean = source->payload.boolean;
        break;
    case 3:
        v->payload.id = source->payload.id;
        break;
    case 5: {
        void* storage = host.operator_new_00bf681b(0x0C);
        v->payload.map = storage != nullptr ? host.map_initialize_header_006eeaf0(storage) : nullptr;
        host.map_assign_00781f20(v->payload.map, source->payload.map);
        break;
    }
    case 6:
        host.variant_init_tag6_0074fb30(v); // ECX = this, no stack argument
        break;
    default:
        break;
    }
    return v;
}

LuaVariant* variant_copy_construct_00886da0(Left55Host& host, LuaVariant* v,
    const LuaVariant* source)
{
    v->vtable = kLuaVariantVtable;
    v->tag = -1;
    v->name = {};
    return variant_assign_00886c10(host, v, source);
}

void result_sink_collect_00887120(Left55Host& host, ResultSink* sink, std::int32_t count,
    std::int32_t options)
{
    std::int32_t last = host.lua_machine_get_top_006b7e70(sink->lua_machine);
    if (count >= 0) {
        last = count;
    }
    for (std::int32_t index = 1; index <= last; ++index) {
        LuaVariant value{kLuaVariantVtable, -1, {}, {}};
        host.build_variant_from_stack_00886e00(sink, &value, index, 0, options);
        host.variant_vector_push_back_006edf00(sink->variants, &value);
        variant_destruct_00884af0(host, &value);
    }
}

void result_sink_collect_range_00887370(Left55Host& host, ResultSink* sink, std::int32_t first,
    std::int32_t last, std::int32_t options)
{
    LuaVariant unused{kLuaVariantVtable, -1, {}, {}}; // built at entry, destroyed at exit
    if (first < 1) {
        first = 1;
    }
    if (last < 1) {
        last = host.lua_machine_get_top_006b7e70(sink->lua_machine);
    }
    for (; first <= last; ++first) {
        LuaVariant value{kLuaVariantVtable, -1, {}, {}};
        host.build_variant_from_stack_00886e00(sink, &value, first, 0, options);
        host.variant_vector_push_back_006edf00(sink->variants, &value);
        variant_destruct_00884af0(host, &value);
    }
    variant_destruct_00884af0(host, &unused);
}

DeferredCall* deferred_call_copy_00887a10(Left55Host& host, DeferredCall* call,
    const DeferredCall* source)
{
    call->self_key = {};
    copy_into(host, &call->self_key, &source->self_key);
    call->function_name = {};
    copy_into(host, &call->function_name, &source->function_name);
    call->arguments[0] = 0;
    call->arguments[1] = 0;
    call->arguments[2] = 0;
    host.deferred_copy_arguments_00887560(call->arguments, source->arguments);
    call->stack_first = source->stack_first;
    call->stack_last = source->stack_last;
    return call;
}

void call_queue_destroy_00887e30(Left55Host& host, void* queue)
{
    host.call_queue_clear_00887d90(queue);
    void** block = reinterpret_cast<void**>(static_cast<char*>(queue) + 4);
    host.free_00bf65ac(*block);
    *block = nullptr;
}

void call_queue_destroy_008882b0(Left55Host& host, void* queue)
{
    host.call_queue_clear_00887d90(queue);
    void** block = reinterpret_cast<void**>(static_cast<char*>(queue) + 4);
    host.free_00bf65ac(*block);
    *block = nullptr;
}

void* lua_object_ptr_field_00888960(Left55Host& host, void* object)
{
    alignas(4) std::uint8_t field[0x14];
    host.lua_object_get_by_name_00b67800(object, field, "Ptr"); // 00CFAD08
    void* pointer = host.lua_object_to_userdata_00b662d0(field);
    host.lua_object_destruct_00b67700(field);
    return pointer;
}

void* lua_object_ptr_field_00888be0(Left55Host& host, void* object)
{
    alignas(4) std::uint8_t field[0x14];
    host.lua_object_get_by_name_00b67800(object, field, "Ptr");
    void* pointer = host.lua_object_to_userdata_00b662d0(field);
    host.lua_object_destruct_00b67700(field);
    return pointer;
}

bool lua_object_is_kind2_entity_00888c40(Left55Host& host, void* object)
{
    if (!host.lua_object_is_table_00b661b0(object)) {
        return false;
    }
    alignas(4) std::uint8_t field[0x14];
    host.lua_object_get_by_name_00b67800(object, field, "Ptr");
    const bool light = host.lua_object_is_light_userdata_00b66160(field);
    host.lua_object_destruct_00b67700(field);
    if (!light) {
        return false;
    }
    alignas(4) std::uint8_t again[0x14];
    host.lua_object_get_by_name_00b67800(object, again, "Ptr");
    void* pointer = host.lua_object_to_userdata_00b662d0(again);
    host.lua_object_destruct_00b67700(again);
    return pointer != nullptr && host.object_is_kind_vcall_5c(pointer, 2);
}

PositionMessage97* position_message_construct_00888f30(Left55Host& host,
    PositionMessage97* message, const float* position, float value_24, std::int32_t value_28)
{
    host.session_message_base_0075b430(message, 0x97);
    *reinterpret_cast<std::int32_t*>(message->base + 4) = 1;
    message->byte_20 = 1;
    message->byte_21 = 1;
    message->value_28 = value_28;
    *reinterpret_cast<std::uint32_t*>(message->base) = kPositionMessage97Vtable;
    message->value_24 = value_24;
    message->word_18 = 0;
    message->byte_1a = 0;
    message->byte_1c = 0;
    message->position[0] = position[0];
    message->position[1] = position[1];
    message->position[2] = position[2];
    return message;
}

void scene_node_set_alpha_recursive_00888fc0(Left55Host& host, void* node, float alpha)
{
    if (host.node_is_kind_vcall_0c(node, host.scene_node_type_00bbe290())) {
        host.scene_node_set_alpha_00bbe490(node, alpha);
    }
    for (void* child = *reinterpret_cast<void**>(static_cast<char*>(node) + 0x34); child != nullptr;
         child = *reinterpret_cast<void**>(static_cast<char*>(child) + 0x3C)) {
        scene_node_set_alpha_recursive_00888fc0(host, child, alpha);
    }
}

void singleton_destruct_008890b0(Left55Host& host, std::uint32_t* object)
{
    host.write_global_00f87908(0);
    object[0] = kSingletonBaseVtable;
}

void owned_block_release_00889610(Left55Host& host, void** slot)
{
    if (*slot != nullptr) {
        host.free_00bf65ac(*slot);
        *slot = nullptr;
    }
}

void scored_push_heap_008898c0(ScoredEntry* first, std::int32_t hole, std::int32_t top,
    ScoredEntry value)
{
    for (std::int32_t parent = (hole - 1) / 2; top < hole && first[parent].score < value.score;
         parent = (hole - 1) / 2) {
        first[hole] = first[parent];
        hole = parent;
    }
    first[hole] = value;
}

std::uint32_t* list_node_allocate_00889ed0(Left55Host& host, std::uint32_t value)
{
    auto* node = static_cast<std::uint32_t*>(host.operator_new_00bf681b(0x0C));
    if (node != nullptr) {
        node[0] = 0;
        node[1] = 0;
        node[2] = value;
        return node;
    }
    // 00889EEF: the failure path still stores the value through the null node ([0 + 8]).
    *reinterpret_cast<volatile std::uint32_t*>(static_cast<std::uintptr_t>(8)) = value;
    return nullptr;
}

void* object_if_kind_2b_00889f10(Left55Host& host, void* object)
{
    if (object != nullptr && host.object_is_kind_vcall_5c(object, 0x2B)) {
        return object;
    }
    return nullptr;
}

void scored_med3_0088a0c0(ScoredEntry* a, ScoredEntry* b, ScoredEntry* c)
{
    if (b->score < a->score) {
        const ScoredEntry t = *b;
        *b = *a;
        *a = t;
    }
    if (c->score < b->score) {
        const ScoredEntry t = *c;
        *c = *b;
        *b = t;
    }
    if (b->score < a->score) {
        const ScoredEntry t = *b;
        *b = *a;
        *a = t;
    }
}

void scored_adjust_heap_0088a130(ScoredEntry* first, std::int32_t hole, std::int32_t bottom,
    ScoredEntry value)
{
    const std::int32_t top = hole;
    std::int32_t child = 2 * hole + 2;
    for (; child < bottom; child = 2 * child + 2) {
        if (first[child].score < first[child - 1].score) {
            --child;
        }
        first[hole] = first[child];
        hole = child;
    }
    if (child == bottom) {
        first[hole] = first[bottom - 1];
        hole = bottom - 1; // LEA EDX,[EDI-1]
    }
    scored_push_heap_008898c0(first, hole, top, value);
}

ScoredEntry* scored_uninitialized_copy_0088a1e0(const ScoredEntry* first,
    const ScoredEntry* last, ScoredEntry* dest)
{
    for (; first != last; ++first, ++dest) {
        if (dest != nullptr) {
            *dest = *first;
        }
    }
    return dest;
}

void** refcounted_slot_assign_0088a2a0(Left55Host& host, void** slot, void* value)
{
    void* old = *slot;
    if (old != nullptr) {
        auto* count = reinterpret_cast<volatile long*>(static_cast<char*>(old) + 4);
        if (host.interlocked_decrement(count) == 0) { // CALL [00CE2220]
            host.refcounted_destroy_vcall_00(old);
        }
        *slot = nullptr;
    }
    *slot = value;
    return slot;
}

void scored_fill_0088a570(ScoredEntry* first, ScoredEntry* last, const ScoredEntry* value)
{
    for (; first != last; ++first) {
        *first = *value;
    }
}

void scored_uninitialized_fill_n_0088a5c0(ScoredEntry* dest, std::uint32_t count,
    const ScoredEntry* value)
{
    for (; count != 0; --count, ++dest) {
        if (dest != nullptr) {
            *dest = *value;
        }
    }
}

void scored_median_0088a650(ScoredEntry* first, ScoredEntry* mid, ScoredEntry* last)
{
    const std::int32_t count = static_cast<std::int32_t>(last - first);
    if (count > 40) {
        const std::int32_t step = (count + 1) / 8;
        scored_med3_0088a0c0(first, first + step, first + 2 * step);
        scored_med3_0088a0c0(mid - step, mid, mid + step);
        scored_med3_0088a0c0(last - 2 * step, last - step, last);
        scored_med3_0088a0c0(first + step, mid, last - step);
        return;
    }
    scored_med3_0088a0c0(first, mid, last);
}

void scored_make_heap_0088a6d0(ScoredEntry* first, ScoredEntry* last)
{
    const std::int32_t bottom = static_cast<std::int32_t>(last - first);
    for (std::int32_t hole = bottom / 2; hole > 0;) {
        --hole;
        scored_adjust_heap_0088a130(first, hole, bottom, first[hole]);
    }
}

void scored_insertion_sort_0088a720(ScoredEntry* first, ScoredEntry* last)
{
    if (first == last) {
        return;
    }
    for (ScoredEntry* next = first + 1; next != last; ++next) {
        const ScoredEntry value = *next;
        if (value.score < first->score) {
            for (ScoredEntry* p = next; p != first; --p) {
                *p = *(p - 1);
            }
            *first = value;
        } else {
            ScoredEntry* p = next;
            for (; value.score < (p - 1)->score; --p) {
                *p = *(p - 1);
            }
            *p = value;
        }
    }
}

bool lua_table_collect_ptr_fields_0088a900(Left55Host& host, PointerArray* array, void* table)
{
    alignas(4) std::uint8_t key[0x14];
    alignas(4) std::uint8_t value[0x14];
    host.lua_object_construct_00b65f50(key);
    host.lua_object_construct_00b65f50(value);
    host.lua_object_iterate_first_00b67080(table, key, value);
    while (!host.lua_object_is_unbound_00b66420(table, key)) {
        alignas(4) std::uint8_t field[0x14];
        host.lua_object_get_by_name_00b67800(value, field, "Ptr");
        void* pointer = host.lua_object_to_userdata_00b662d0(field);
        host.lua_object_destruct_00b67700(field);
        if (array->count == array->capacity) {
            std::int32_t capacity = array->capacity * 2;
            if (capacity < 2) {
                capacity = 1;
            }
            host.pointer_array_reserve_00529760(array, capacity);
        }
        void** slot = array->data + array->count;
        if (slot != nullptr) {
            *slot = pointer;
        }
        array->count += 1;
        host.lua_object_iterate_next_00b67190(table, key, value);
    }
    host.lua_object_destruct_00b67700(value);
    host.lua_object_destruct_00b67700(key);
    return true;
}

void tree_construct_0088aaf0(Left55Host& host, void* tree)
{
    auto** words = static_cast<void**>(tree);
    auto* head = static_cast<void**>(host.tree_buy_head_node_0088a520());
    words[1] = head;
    reinterpret_cast<std::uint8_t*>(head)[0x19] = 1; // isnil
    head = static_cast<void**>(words[1]);
    head[1] = head;                                  // parent
    head[0] = head;                                  // left
    head[2] = head;                                  // right
    words[2] = nullptr;                              // size
}

ScoredEntry* scored_copy_backward_0088abd0(const ScoredEntry* first, const ScoredEntry* last,
    ScoredEntry* dest_end)
{
    const std::int32_t count = static_cast<std::int32_t>(last - first);
    ScoredEntry* dest = dest_end;
    while (last != first) {
        *--dest = *--last;
    }
    return dest_end - count;
}

void scored_unguarded_partition_0088ac20(ScoredEntry** out, ScoredEntry* first,
    ScoredEntry* last)
{
    // VC9 std::_Unguarded_partition with operator< on .score; the native unrolls the two scans
    // by four (0088AD00.., 0088ADB0..) without changing their order of visits.
    ScoredEntry* mid = first + (last - first) / 2;
    scored_median_0088a650(first, mid, last - 1);
    ScoredEntry* pfirst = mid;
    ScoredEntry* plast = pfirst + 1;
    while (first < pfirst && !((pfirst - 1)->score < pfirst->score) &&
           !(pfirst->score < (pfirst - 1)->score)) {
        --pfirst;
    }
    while (plast < last && !(plast->score < pfirst->score) && !(pfirst->score < plast->score)) {
        ++plast;
    }
    ScoredEntry* gfirst = plast;
    ScoredEntry* glast = pfirst;
    auto swap = [](ScoredEntry* a, ScoredEntry* b) {
        const ScoredEntry t = *a;
        *a = *b;
        *b = t;
    };
    for (;;) {
        for (; gfirst < last; ++gfirst) {
            if (pfirst->score < gfirst->score) {
                continue;
            }
            if (gfirst->score < pfirst->score) {
                break;
            }
            swap(plast++, gfirst);
        }
        for (; first < glast; --glast) {
            if ((glast - 1)->score < pfirst->score) {
                continue;
            }
            if (pfirst->score < (glast - 1)->score) {
                break;
            }
            swap(--pfirst, glast - 1);
        }
        if (glast == first && gfirst == last) {
            out[0] = pfirst;
            out[1] = plast;
            return;
        }
        if (glast == first) {
            if (plast != gfirst) {
                swap(pfirst, plast);
            }
            ++plast;
            swap(pfirst++, gfirst++);
        } else if (gfirst == last) {
            if (--glast != --pfirst) {
                swap(glast, pfirst);
            }
            swap(pfirst, --plast);
        } else {
            swap(--glast, gfirst++);
        }
    }
}

void variant_map_walk_0088b120(Left55Host& host, VariantMap* map)
{
    VariantMapIterator it{map, map->head->left};
    for (;;) {
        VariantMapNode* end = map->head;
        if (it.container == nullptr) {
            host.invalid_parameter_00bf6713();
        }
        if (it.node == end) {
            break;
        }
        if (it.container == nullptr) {
            host.invalid_parameter_00bf6713();
        }
        if (it.node == it.container->head) {
            host.invalid_parameter_00bf6713();
        }
        variant_walk_0088b190(host, &it.node->value); // LEA ECX,[EBP+1Ch]
        host.map_iterator_increment_006eda20(&it);
    }
}

void variant_walk_0088b190(Left55Host& host, LuaVariant* v)
{
    if (v->tag != 0 && v->tag != 1 && v->tag == 5) {
        variant_map_walk_0088b120(host, v->payload.map); // tail JMP
    }
}

void end_scene_choice_0088b360(Left55Host& host, std::int32_t choice)
{
    host.game_end_scene_004d7970(host.game_global_00e188a8(), 0);
    if (choice == 3) {
        host.profile_clear_checkpoint_00437c70(host.profile_manager_00425c20());
    } else if (choice != 1) {
        return;
    }
    host.game_request_state_004d7920(host.game_global_00e188a8(), 0x0B);
    void* pause = *reinterpret_cast<void**>(static_cast<char*>(host.interface_global_00e198c4()) + 0xD8);
    host.hud_pause_resume_005fd440(pause); // tail JMP
}

void landscape_build_shore_mask_00880420(Left55Host& host, void* owner, std::int8_t** columns,
    std::int32_t step, const ShoreMaskParams* params)
{
    void* terrain = *reinterpret_cast<void**>(static_cast<char*>(owner) + 0x3D0);
    const std::int32_t rows = host.terrain_rows_vcall_10(terrain);       // 00880438
    const std::int32_t row_cells = host.terrain_columns_vcall_14(terrain); // 0088044B

    // 00880453..00880480: level = clamp(trunc(falloff / cell_size + 1.5), 1, 100).
    std::int32_t level = static_cast<std::int32_t>(
        static_cast<double>(params->falloff) / params->cell_size + 1.5); // _ftol2_sse
    if (level < 1) {
        level = 1;
    } else if (level > 100) {
        level = 100;
    }

    // 00880484..0088050C: sample the terrain every `step` cells; land = level, water = 0.
    std::int8_t** column = columns;
    for (std::int32_t r = 0; r < rows; r += step, ++column) {
        std::int32_t cell = 0;
        for (std::int32_t c = 0; c < row_cells; c += step, ++cell) {
            const float height = host.terrain_height_vcall_20(terrain, r, c);
            (*column)[cell] = height >= -params->depth ? static_cast<std::int8_t>(level) : 0; // JC
        }
    }
    const std::int32_t width = (rows - 1) / step + 1;
    const std::int32_t height = (row_cells - 1) / step + 1;
    if (level > 1) {
        byte_grid_propagate_falloff_00880080(host, columns, width, height, true); // 00880544
    }

    // 00880549..00880585: ring = trunc(ring_width / (cell_size * 1.7999999523162842) + 1.5);
    // negative, 0 and 1 skip the ring passes, above 100 is 100.
    std::int32_t ring = static_cast<std::int32_t>(static_cast<double>(params->ring_width) /
        (static_cast<double>(params->cell_size) * 1.7999999523162842) + 1.5);
    if (ring > 1) {
        if (ring > 100) {
            ring = 100;
        }
        // 0088058D..008805BE: every positive cell becomes `ring`, then the falloff pass.
        for (std::int32_t x = 0; x < width; ++x) {
            for (std::int32_t y = 0; y < height; ++y) {
                if (columns[x][y] > 0) {
                    columns[x][y] = static_cast<std::int8_t>(ring);
                }
            }
        }
        byte_grid_propagate_falloff_00880080(host, columns, width, height, true); // 008805CD

        // 008805D2..0088064A forward, 0088064C..008806CB backward: an empty cell next (in the
        // four even directions) to an empty or 7Fh cell becomes 7Fh.
        auto mark_open_water = [&](std::int32_t x, std::int32_t y) {
            if (columns[x][y] != 0) {
                return;
            }
            for (std::int32_t direction = 0; direction < 8; direction += 2) {
                const std::int8_t s =
                    host.byte_grid_sample_00412070(columns, width, height, x, y, direction, 0);
                if (s == 0 || s == 0x7F) {
                    columns[x][y] = 0x7F;
                    return;
                }
            }
        };
        for (std::int32_t x = 0; x < width; ++x) {
            for (std::int32_t y = 0; y < height; ++y) {
                mark_open_water(x, y);
            }
        }
        for (std::int32_t x = width - 1; x >= 0; --x) {
            for (std::int32_t y = height - 1; y >= 0; --y) {
                mark_open_water(x, y);
            }
        }
        // 008806CD..00880705: enclosed empty cells become 1, open water (7Fh) becomes 0.
        for (std::int32_t x = 0; x < width; ++x) {
            for (std::int32_t y = 0; y < height; ++y) {
                std::int8_t& cell = columns[x][y];
                if (cell == 0) {
                    cell = 1;
                } else if (cell == 0x7F) {
                    cell = 0;
                }
            }
        }
        // 00880707..008807A1: grow rings in place; a cell at level L whose eight neighbours
        // (outside = 7Fh) are all >= L moves to L + 1.
        for (std::int32_t l = 1; l < ring; ++l) {
            for (std::int32_t x = 0; x < width; ++x) {
                for (std::int32_t y = 0; y < height; ++y) {
                    if (columns[x][y] != l) {
                        continue;
                    }
                    std::int32_t direction = 0;
                    for (; direction < 8; ++direction) {
                        const std::int8_t s = host.byte_grid_sample_00412070(columns, width,
                            height, x, y, direction, 0x7F);
                        if (s < l) {
                            break;
                        }
                    }
                    if (direction == 8) {
                        columns[x][y] = static_cast<std::int8_t>(columns[x][y] + 1);
                    }
                }
            }
        }
        // 008807AB..008807E2: everything below `ring` is cleared.
        for (std::int32_t x = 0; x < width; ++x) {
            for (std::int32_t y = 0; y < height; ++y) {
                if (columns[x][y] < ring) {
                    columns[x][y] = 0;
                }
            }
        }
    }

    // 008807E4..00880A2E: thinning. Repeat over a column window [x_start, x_end) until a pass
    // changes nothing; the window follows the last change (+-2 columns).
    std::int32_t x_start = 0;
    std::int32_t x_end = width;
    bool settled;
    do {
        settled = true;                    // byte [ESP+78h] = 1
        const std::int32_t limit = x_end;  // [ESP+38h]
        if (x_start >= limit) {
            break;
        }
        for (std::int32_t x = x_start; x < limit; ++x) {
            for (std::int32_t y = 0; y < height; ++y) {
                std::int8_t& cell = columns[x][y];
                if (cell <= 0) {
                    continue;
                }
                cell = 1;
                std::int32_t gap_cells[5] = {0, 0, 0, 0, 0}; // [ESP+3Ch]
                std::int32_t gap_run[8] = {-1, -1, -1, -1, -1, -1, -1, -1}; // [ESP+50h]
                std::int32_t solid = 0;                      // [ESP+2Ch]
                std::int32_t runs = 0;                       // EBP
                std::int8_t previous = 0;                    // byte [ESP+13h]
                std::int8_t s = 0;                           // BL
                for (std::int32_t direction = 0; direction < 8; ++direction) {
                    s = host.byte_grid_sample_00412070(columns, width, height, x, y, direction, 0);
                    if (s > 0) {
                        ++solid;
                        const std::int8_t before = direction == 0
                            ? host.byte_grid_sample_00412070(columns, width, height, x, y, 7, 0)
                            : previous;
                        if (before == 0) {
                            ++runs;
                        }
                    } else {
                        gap_cells[runs] += 1;
                        gap_run[direction] = runs;
                    }
                    previous = s;
                }
                if (solid < 2) {
                    cell = 0;                                // 00880914
                } else {
                    if (runs <= 1) {
                        continue;                            // 00880920: no window update
                    }
                    if (s == 0) {
                        // 0088092A: the last gap wraps into the first; relabel it 0.
                        gap_cells[0] += gap_cells[runs];
                        if (gap_run[7] > 0) {
                            std::int32_t i = 7;
                            do {
                                gap_run[i] = 0;
                                --i;
                            } while (gap_run[i] > 0);
                        }
                    }
                    std::int32_t best = 0;
                    std::int32_t best_cells = gap_cells[0];
                    for (std::int32_t i = 1; i < runs; ++i) {
                        if (gap_cells[i] > best_cells) {
                            best_cells = gap_cells[i];
                            best = i;
                        }
                    }
                    // 0088097E..008809BB: fill every gap neighbour outside the largest gap
                    // (directions 0..6 only, as the native loop bound is 7).
                    for (std::int32_t direction = 0; direction < 7; ++direction) {
                        if (gap_run[direction] >= 0 && gap_run[direction] != best) {
                            std::int32_t nx = x;
                            std::int32_t ny = y;
                            host.grid_step_direction8_00412010(&nx, &ny, direction);
                            columns[nx][ny] = 1;
                        }
                    }
                }
                // 008809BD..008809F6: the window follows the change.
                if (settled) {
                    settled = false;
                    x_start = x <= 2 ? 0 : x - 2;
                }
                x_end = width < x + 2 ? width : x + 2;
            }
        }
    } while (!settled);

    host.landscape_dilate_mask_0087fa90(columns, width, height, params->block); // 00880A4F
}

} // namespace bsp::cc13_left55
