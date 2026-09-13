#include "bsp/native_input_configuration_parse.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native input parsing requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
template<class T = Word> volatile T& field(Word p, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile T*>(p + offset);
}
void zero_header(Word h) noexcept { field(h) = 0; field(h, 4) = 0; field(h, 8) = 0; }
float x87_spill(float value) noexcept {
    float result;
    __asm { fld dword ptr [value] }
    __asm { fstp dword ptr [result] }
    return result;
}
void multiply_sign(float& value, const volatile double& factor) noexcept {
    auto* destination = &value;
    const volatile double* source = &factor;
    __asm { mov eax,destination }
    __asm { mov edx,source }
    __asm { fld dword ptr [eax] }
    __asm { fmul qword ptr [edx] }
    __asm { fstp dword ptr [eax] }
}
void release_modifier_header(Word h) {
    resize_native_input_modifiers_00697220(pointer(h), 0);
    singleton_lifetime_free(pointer(field(h)));
}
void copy_descriptor(Word destination, Word source) noexcept {
    // Each native DWORD is copied before the next source read. memcpy also
    // retains the indeterminate padding bytes without interpreting their value.
    for (Word offset = 0; offset != 0x14; offset += 4)
        std::memcpy(pointer(destination + offset), pointer(source + offset), 4);
}
struct LuaScope {
    NativeLuaObjectStorage local;
    NativeLuaObjectStorage& value;
    bool owned = false;
    LuaScope() noexcept : value(local) {}
    explicit LuaScope(NativeLuaObjectStorage& external) noexcept : value(external) {}
    ~LuaScope() noexcept(false) { release(); }
    void release() { if (owned) { owned = false; destroy_native_lua_object_00b67700(value); } }
    void construct() { construct_native_lua_object_00b65f50(&value); owned = true; }
    void named(NativeLuaObjectStorage& table, const char* name) {
        native_lua_get_by_name_00b67800(table, &value, name); owned = true;
    }
    void indexed(NativeLuaObjectStorage& table, std::int32_t index) {
        native_lua_get_by_index_00b67720(table, &value, index); owned = true;
    }
    void reassign(NativeLuaObjectStorage& table, const char* name) {
        LuaScope next; next.named(table, name);
        assign_native_lua_object_00b67690(value, next.value);
        next.release();
    }
};
struct ArrayScope {
    NativeInputArrayHeader value{nullptr, 0, 0};
    const bool modifiers;
    bool owned = true;
    explicit ArrayScope(bool is_modifiers) noexcept : modifiers(is_modifiers) {}
    ~ArrayScope() noexcept(false) { release(); }
    void release() {
        if (!owned) return;
        owned = false;
        if (modifiers) resize_native_input_modifiers_00697220(&value, 0);
        else resize_native_input_context_words_00696e70(&value, 0);
        singleton_lifetime_free(value.base);
    }
};
std::int32_t grow_capacity(Word capacity) noexcept {
    const auto doubled = static_cast<std::int32_t>(capacity + capacity);
    return doubled > 1 ? doubled : 1;
}
void append_context(NativeInputArrayHeader& array, Word value) {
    const auto h = address(&array), capacity = field(h, 8);
    if (field(h, 4) == capacity) reserve_native_input_context_words_00696d80(&array, grow_capacity(capacity));
    const auto slot = field(h) + field(h, 4) * 4u;
    if (slot) field(slot) = value;
    field(h, 4) = field(h, 4) + 1u;
}
void append_modifier(NativeInputArrayHeader& array, const void* descriptor) {
    const auto h = address(&array), capacity = field(h, 8);
    if (field(h, 4) == capacity) reserve_native_input_modifiers_00696de0(&array, grow_capacity(capacity));
    const auto slot = field(h) + field(h, 4) * 0x14u;
    if (slot) copy_descriptor(slot, address(descriptor));
    field(h, 4) = field(h, 4) + 1u;
}
Word word_distance(Word first, Word last) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(last - first) >> 2);
}
bool indexed_contains(Word h, Word action) {
    for (Word index = 0;; ++index) {
        const auto first = field(h, 4);
        if (!first || index >= word_distance(first, field(h, 8))) return false;
        if (!first || index >= word_distance(first, field(h, 8))) _invalid_parameter_noinfo();
        if (field(field(h, 4) + index * 4u) == action) return true;
    }
}
bool checked_contains(Word h, Word action) {
    auto cursor = field(h, 4);
    if (cursor > field(h, 8)) _invalid_parameter_noinfo();
    for (;;) {
        const auto end = field(h, 8);
        if (field(h, 4) > end) _invalid_parameter_noinfo();
        // Native CMP ESI,ESI makes the owner-mismatch call unreachable.
        if (cursor == end) return false;
        if (cursor >= field(h, 8)) _invalid_parameter_noinfo();
        if (field(cursor) == action) return true;
        if (cursor >= field(h, 8)) _invalid_parameter_noinfo();
        cursor += 4u;
    }
}
bool has_input(NativeLuaObjectStorage& inputs, std::int32_t index) {
    LuaScope probe; probe.indexed(inputs, index);
    const auto present = !native_lua_is_nil_00b65fb0(probe.value);
    probe.release();
    return present;
}
void parse_actions(void* configuration, NativeInputConfigurationParsedObjects& objects,
    NativeInputConfigurationParseServices& services) {
    const auto base = address(configuration);
    auto* const owner = get_native_input_action_owner_004bec00(services.loading.cleanup.action_owner);
    LuaScope inputs(objects.inputs); inputs.named(objects.globals, "Inputs");
    LuaScope key(objects.key); key.construct();
    LuaScope value(objects.value); value.construct();
    native_lua_iterate_first_00b67080(inputs.value, key.value, value.value);
    while (!native_lua_is_unbound_00b66420(key.value)) {
        const auto action = static_cast<Word>(native_lua_integer_00b66290(key.value, services.crt_sse2_conversion));
        LuaScope group_key; group_key.construct();
        LuaScope group_value; group_value.construct();
        ArrayScope contexts(false);
        LuaScope table; table.named(value.value, "groups");
        native_lua_iterate_first_00b67080(table.value, group_key.value, group_value.value);
        while (!native_lua_is_unbound_00b66420(group_key.value)) {
            const auto context = static_cast<Word>(native_lua_integer_00b66290(group_value.value, services.crt_sse2_conversion));
            append_context(contexts.value, context);
            native_lua_iterate_next_00b67190(table.value, group_key.value, group_value.value);
        }
        std::uint8_t helper = 0;
        table.reassign(value.value, "helper");
        if (!native_lua_is_nil_00b65fb0(table.value)) helper = native_lua_boolean_00b66250(table.value) ? 1u : 0u;
        configure_native_input_action_00a93c80(owner, action, &contexts.value, helper, services.actions);
        table.reassign(value.value, "inputs");
        for (Word index = 1; has_input(table.value, static_cast<std::int32_t>(index)); ++index) {
            LuaScope input; input.indexed(table.value, static_cast<std::int32_t>(index));
            LuaScope modifier_key; modifier_key.construct();
            LuaScope modifier_value; modifier_value.construct();
            LuaScope component; component.named(input.value, "press");
            std::uint8_t swap = 0;
            if (services.swap_general_00f889c1 && indexed_contains(base + 0x500u, action)) swap = 1;
            else if (services.swap_map_00f889c4 && indexed_contains(base + 0x510u, action)) swap = 1;
            alignas(4) std::byte primary[0x14];
            decode_native_input_descriptor_006974f0(configuration, primary, component.value, swap,
                services.loading.x360comp_00f88a30, services.crt_sse2_conversion);
            ArrayScope required(true), forbidden(true);
            ArrayScope* arrays[] = {&required, &forbidden};
            const char* names[] = {"and", "not"};
            for (unsigned group = 0; group != 2; ++group) {
                component.reassign(input.value, names[group]);
                if (!native_lua_is_nil_00b65fb0(component.value)) {
                    native_lua_iterate_first_00b67080(component.value, modifier_key.value, modifier_value.value);
                    while (!native_lua_is_unbound_00b66420(modifier_key.value)) {
                        alignas(4) std::byte descriptor[0x14];
                        auto* decoded = decode_native_input_descriptor_006974f0(configuration, descriptor, modifier_value.value, swap,
                            services.loading.x360comp_00f88a30, services.crt_sse2_conversion);
                        append_modifier(arrays[group]->value, decoded);
                        native_lua_iterate_next_00b67190(component.value, modifier_key.value, modifier_value.value);
                    }
                }
            }
            const auto one_bits = services.actions.records.bindings.one_00d7a24c;
            float scale; std::memcpy(&scale, &one_bits, 4);
            component.reassign(input.value, "mul");
            if (!native_lua_is_nil_00b65fb0(component.value)) scale = native_lua_number_00b66270(component.value);
            if (field<std::uint8_t>(base, 0x4ca) && checked_contains(base + 0x4e0u, action))
                multiply_sign(scale, services.negative_one_00d7a250);
            if (field<std::uint8_t>(base, 0x4cb) && checked_contains(base + 0x4f0u, action))
                multiply_sign(scale, services.negative_one_00d7a250);
            append_native_input_owner_descriptor_00a93830(owner, action, primary, &required.value, &forbidden.value, x87_spill(scale));
            forbidden.release(); required.release(); component.release();
            modifier_value.release(); modifier_key.release(); input.release();
        }
        table.release(); contexts.release(); group_value.release(); group_key.release();
        native_lua_iterate_next_00b67190(inputs.value, key.value, value.value);
    }
    {
        ArrayScope contexts(false);
        reserve_native_input_context_words_00696d80(&contexts.value, 1);
        append_context(contexts.value, 0x1e);
        configure_native_input_action_00a93c80(owner, 0x128, &contexts.value, 0, services.actions);
        contexts.release();
    }
    value.owned = false; key.owned = false; inputs.owned = false;
}
} // namespace

void* construct_native_input_binding_00a93350(void* fresh, const void* descriptor,
    const void* required, const void* forbidden, float scale) {
    const auto binding = address(fresh), source = address(descriptor);
    field<std::uint8_t>(binding) = 0;
    copy_descriptor(binding + 4u, source);
    zero_header(binding + 0x18u);
    assign_native_input_modifiers_00a92ee0(pointer(binding + 0x18u), required);
    try {
        zero_header(binding + 0x24u);
        assign_native_input_modifiers_00a92ee0(pointer(binding + 0x24u), forbidden);
    } catch (...) { release_modifier_header(binding + 0x18u); throw; }
    std::memcpy(pointer(binding + 0x30u), &scale, 4);
    field<std::uint8_t>(binding, 1) = field(source) == 2 ? 1u : 0u;
    return fresh;
}

void destroy_native_input_binding_00a93070(void* binding) {
    const auto base = address(binding);
    try { release_modifier_header(base + 0x24u); }
    catch (...) { release_modifier_header(base + 0x18u); throw; }
    release_modifier_header(base + 0x18u);
}

void append_native_input_action_descriptor_00a93620(void* action, const void* descriptor,
    const void* required, const void* forbidden, float scale) {
    alignas(4) std::byte temporary[0x34];
    auto* result = construct_native_input_binding_00a93350(temporary, descriptor, required, forbidden, x87_spill(scale));
    try { append_native_input_binding_00a93440(pointer(address(action) + 0x10u), result); }
    catch (...) { destroy_native_input_binding_00a93070(temporary); throw; }
    destroy_native_input_binding_00a93070(temporary);
}

void append_native_input_owner_descriptor_00a93830(void* owner, Word action,
    const void* descriptor, const void* required, const void* forbidden, float scale) {
    const auto spilled = x87_spill(scale);
    auto* record = pointer(field(address(owner), 4) + action * 0x30u);
    append_native_input_action_descriptor_00a93620(record, descriptor, required, forbidden, spilled);
}

void load_native_input_configuration_actions_prefix_00698a10(void* configuration,
    NativeInputConfigurationParsedObjects& objects, NativeInputConfigurationParseServices& services) {
    load_native_input_configuration_modifiers_prefix_00698a10(configuration, &objects.globals,
        services.loading, services.crt_sse2_conversion);
    try { parse_actions(configuration, objects, services); }
    catch (...) { destroy_native_lua_object_00b67700(objects.globals); throw; }
}

void release_native_input_configuration_parsed_objects(NativeInputConfigurationParsedObjects& objects) {
    destroy_native_lua_object_00b67700(objects.value);
    destroy_native_lua_object_00b67700(objects.key);
    destroy_native_lua_object_00b67700(objects.inputs);
    destroy_native_lua_object_00b67700(objects.globals);
}
} // namespace bsp
