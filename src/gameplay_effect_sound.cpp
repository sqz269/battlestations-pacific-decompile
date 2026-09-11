#include "bsp/gameplay_effect_sound.hpp"
#include "bsp/gameplay_effect_scalar_components.hpp"
#include "bsp/lua_object.hpp"
#include "bsp/voice_slot_start.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstring>

namespace bsp {
namespace {
template<class T> T load(const void* p, std::size_t n) noexcept {
    T v; std::memcpy(&v, static_cast<const unsigned char*>(p) + n, sizeof v); return v;
}
template<class T> void store(void* p, std::size_t n, T v) noexcept {
    std::memcpy(static_cast<unsigned char*>(p) + n, &v, sizeof v);
}
struct Field { GuiLuaHost& lua; GuiLuaRef value; ~Field() { lua.release(value); } };
struct Name {
    NativeStringStorage& strings; NativeString value;
    ~Name() { destroy_native_string_header_0041dd20(&value, strings); }
};
void assign_sample_name(NativeString& name, const char* text, NativeStringStorage& strings) {
    const auto length = text ? static_cast<std::uint32_t>(std::strlen(text)) : 0;
    name.resize_0041dd40(strings, length, false);
    if (name.data()) std::memcpy(name.data(), text, name.length());
}
void acquire_and_append(void* p, NativeString& name, EffectSoundContext& context, bool clear_temporary) {
    void* sample;
    auto& cache = context.host.current_sample_cache_00f8bbe8();
    auto** const source = acquire_sound_sample_00a83fd0(cache, sample, name, context.cache);
    append_effect_sound_sample_005b9af0(static_cast<unsigned char*>(p) + 0x20, source, context.host);
    // Read the actual temporary AFTER append and any growth callbacks.
    void* const captured = sample;
    if (captured) {
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(static_cast<unsigned char*>(captured) + 4)) == 0)
            context.host.zero_references_slot_00(captured);
        if (clear_temporary) sample = nullptr;
    }
}
std::int32_t increment(std::int32_t value) noexcept {
    const auto bits = static_cast<std::uint32_t>(value) + 1u;
    std::memcpy(&value, &bits, sizeof value); return value;
}
} // namespace
void reserve_effect_sound_samples_005b8ea0(void* header, std::int32_t n, GameplayEffectComponentLifetime& host) {
    reserve_gameplay_effect_components_0086e770(header, n, host);
}
void resize_effect_sound_samples_005b8fa0(void* header, std::int32_t n, GameplayEffectComponentLifetime& host) {
    resize_gameplay_effect_components_0086edd0(header, n, host);
}
void append_effect_sound_sample_005b9af0(void* header, const void* source, GameplayEffectComponentLifetime& host) {
    append_gameplay_effect_component_0086eb60(header, source, host);
}
void read_effect_sound_0086ef60(void* p, GuiLua51Host& lua, const GuiLuaRef& row, EffectSoundContext& context) {
    read_effect_component_base_00868bf0(p, lua, row, context.crt_sse2_conversion);
    Field table{lua, lua.get_by_name(row, "SampleTable")};
    Name sample{context.strings, {}};
    if (lua.type_of(table.value) == GuiLuaType::Table) {
        for (std::int32_t i = 1;; i = increment(i)) {
            bool nil;
            { Field probe{lua, lua.get_by_index(table.value, i)}; nil = lua.type_of(probe.value) == GuiLuaType::Nil; }
            if (nil) break;
            { Field entry{lua, lua.get_by_index(table.value, i)}; assign_sample_name(sample.value, lua.to_string(entry.value), context.strings); }
            acquire_and_append(p, sample.value, context, true);
        }
    } else {
        { Field entry{lua, lua.get_by_name(row, "Sample")}; assign_sample_name(sample.value, lua.to_string(entry.value), context.strings); }
        acquire_and_append(p, sample.value, context, false);
    }
    {
        Field pitch{lua, lua.get_by_name(row, "PitchRnd")};
        store(p, 0x38, lua.type_of(pitch.value) == GuiLuaType::Number ? lua_object_number_00b66270(lua, pitch.value) : 0.0f);
    }
    Name category{context.strings, {}};
    {
        Field field{lua, lua.get_by_name(row, "Category")};
        category.value.assign_0041e870(context.strings, lua.to_string(field.value));
    }
    auto& class_owner = context.host.current_sound_owner_00f8bbd8();
    store(p, 0x30, find_sound_class_00a7acf0(class_owner.levels, category.value, context.host));
    Name type{context.strings, {}};
    {
        Field field{lua, lua.get_by_name(row, "SoundType")};
        type.value.assign_0041e870(context.strings, lua.type_of(field.value) == GuiLuaType::String ? lua.to_string(field.value) : "Normal");
    }
    auto& type_owner = context.host.current_sound_owner_00f8bbd8();
    store(p, 0x34, find_sound_type_00a7b0a0(type_owner.configuration, type.value, context.host));
    { Field field{lua, lua.get_by_name(row, "Persistent")}; store<std::uint8_t>(p, 0x2c, lua.to_boolean(field.value) ? 1 : 0); }
}
void destroy_effect_sound_0086fa90(void* p, EffectSoundContext& context) {
    store<std::uint32_t>(p, 0, 0x00d0da18);
    resize_effect_sound_samples_005b8fa0(static_cast<unsigned char*>(p) + 0x20, 0, context.host);
    singleton_lifetime_free(load<void*>(p, 0x20));
    destroy_effect_component_base_0086b7e0(p, context.strings);
}
void* scalar_delete_effect_sound_0086fb00(void* p, std::uint32_t flags, EffectSoundContext& context) {
    destroy_effect_sound_0086fa90(p, context);
    if (flags & 1u) singleton_lifetime_free(p);
    return p;
}
} // namespace bsp
