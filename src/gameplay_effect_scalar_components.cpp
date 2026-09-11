#include "bsp/gameplay_effect_scalar_components.hpp"
#include "bsp/lua_object.hpp"
#include "bsp/gameplay_effect_sound.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <array>
#include <cstring>

namespace bsp {
namespace {
template<class T> T load(const void* p, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const unsigned char*>(p) + offset, sizeof value);
    return value;
}
template<class T> void store(void* p, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<unsigned char*>(p) + offset, &value, sizeof value);
}
struct FieldRef {
    GuiLuaHost& lua;
    GuiLuaRef value;
    ~FieldRef() { lua.release(value); }
};
void number(void* p, std::size_t offset, GuiLuaHost& lua, const GuiLuaRef& table,
    const char* key) {
    FieldRef field{lua, lua.get_by_name(table, key)};
    store(p, offset, lua_object_number_00b66270(lua, field.value));
}
void number_or(void* p, std::size_t offset, GuiLuaHost& lua, const GuiLuaRef& table,
    const char* key, float fallback) {
    FieldRef field{lua, lua.get_by_name(table, key)};
    store(p, offset, lua.type_of(field.value) == GuiLuaType::Number
        ? lua_object_number_00b66270(lua, field.value) : fallback);
}
void truth(void* p, std::size_t offset, GuiLuaHost& lua, const GuiLuaRef& table,
    const char* key, bool boolean_only = false) {
    FieldRef field{lua, lua.get_by_name(table, key)};
    store<std::uint8_t>(p, offset, (!boolean_only || lua.type_of(field.value) == GuiLuaType::Boolean)
        && lua.to_boolean(field.value) ? 1 : 0);
}
//00B67D40 tracked-field projection. No table length check: index5 must be
// nil, then read1..4 (numeric strings allowed, missing entries become0).
// Each element is released before reading the next; publish only at the end.
std::array<float, 4> vector4_or(GuiLua51Host& lua, const GuiLuaRef& ref,
    const std::array<float, 4>& fallback) {
    if (lua.type_of(ref) != GuiLuaType::Table) return fallback;
    bool fifth_is_nil;
    {
        FieldRef fifth{lua, lua.get_by_index(ref, 5)};
        fifth_is_nil = lua.type_of(fifth.value) == GuiLuaType::Nil;
    }
    if (!fifth_is_nil) return fallback;
    std::array<float, 4> result;
    for (std::int32_t i = 0; i != 4; ++i) {
        FieldRef element{lua, lua.get_by_index(ref, i + 1)};
        result[static_cast<std::size_t>(i)] = lua_object_number_00b66270(lua, element.value);
    }
    return result;
}
void color(void* p, std::size_t offset, GuiLua51Host& lua, const GuiLuaRef& table,
    const char* key, const std::array<float, 4>& fallback) {
    FieldRef field{lua, lua.get_by_name(table, key)};
    const auto value = vector4_or(lua, field.value, fallback);
    for (std::size_t i = 0; i != value.size(); ++i) store(p, offset + i * 4, value[i]);
}
struct TemporaryName {
    NativeStringStorage& strings;
    NativeString value;
    ~TemporaryName() { destroy_native_string_header_0041dd20(&value, strings); }
};
void* scalar_base(void* p, std::uint32_t flags, EffectScalarComponentContext& context) {
    destroy_effect_component_base_0086b7e0(p, context.strings);
    if ((flags & 1u) != 0) singleton_lifetime_free(p);
    return p;
}
struct ScalarDispatch {
    std::uint32_t vtable;
    void (*read)(void*, GuiLua51Host&, const GuiLuaRef&, EffectScalarComponentContext&);
    void* (*destroy)(void*, std::uint32_t, EffectScalarComponentContext&);
};
constexpr std::array<ScalarDispatch, 7> dispatch{{
    {0x00d0d5f4, read_effect_shake_00868de0, scalar_delete_effect_shake_0086d0a0},
    {0x00d0d76c, read_effect_const_rumble_00868fa0, scalar_delete_effect_const_rumble_0086d0e0},
    {0x00d0d78c, read_effect_slope_rumble_00869080, scalar_delete_effect_slope_rumble_0086d100},
    {0x00d0d7ac, read_effect_square_rumble_00869160, scalar_delete_effect_square_rumble_0086d120},
    {0x00d0d654, read_effect_light_00869fd0, scalar_delete_effect_light_0086d140},
    {0x00d0d674, read_effect_splash_008694c0, scalar_delete_effect_splash_0086d160},
    {0x00d0d634, read_effect_waterdrops_0086b2b0, scalar_delete_effect_waterdrops_0086d0c0},
}};
} // namespace

void read_effect_shake_00868de0(void* p, GuiLua51Host& lua, const GuiLuaRef& row,
    EffectScalarComponentContext& context) {
    read_effect_component_base_00868bf0(p, lua, row, context.crt_sse2_conversion);
    truth(p, 0x20, lua, row, "Persistent");
    number(p, 0x24, lua, row, "Radius");
    number(p, 0x28, lua, row, "Strength");
}
void read_effect_rumble_base_00868ec0(void* p, GuiLua51Host& lua, const GuiLuaRef& row,
    EffectScalarComponentContext& context) {
    read_effect_component_base_00868bf0(p, lua, row, context.crt_sse2_conversion);
    {
        FieldRef field{lua, lua.get_by_name(row, "PowerType")};
        store(p, 0x20, lua_object_integer_00b66290(lua, field.value, context.crt_sse2_conversion));
    }
    number(p, 0x24, lua, row, "TimeTotal");
    number(p, 0x28, lua, row, "Radius");
}
void read_effect_const_rumble_00868fa0(void* p, GuiLua51Host& lua, const GuiLuaRef& row,
    EffectScalarComponentContext& context) {
    read_effect_rumble_base_00868ec0(p, lua, row, context);
    number(p, 0x2c, lua, row, "Magnitude");
}
void read_effect_slope_rumble_00869080(void* p, GuiLua51Host& lua, const GuiLuaRef& row,
    EffectScalarComponentContext& context) {
    read_effect_rumble_base_00868ec0(p, lua, row, context);
    number(p, 0x2c, lua, row, "StartMagnitude");
}
void read_effect_square_rumble_00869160(void* p, GuiLua51Host& lua, const GuiLuaRef& row,
    EffectScalarComponentContext& context) {
    read_effect_rumble_base_00868ec0(p, lua, row, context);
    truth(p, 0x2c, lua, row, "MaxStart");
    number(p, 0x30, lua, row, "MinMagnitude");
    number(p, 0x34, lua, row, "MaxMagnitude");
    number(p, 0x38, lua, row, "MinTime");
    number(p, 0x3c, lua, row, "MaxTime");
}
void read_effect_splash_008694c0(void* p, GuiLua51Host& lua, const GuiLuaRef& row,
    EffectScalarComponentContext& context) {
    read_effect_component_base_00868bf0(p, lua, row, context.crt_sse2_conversion);
    number_or(p, 0x20, lua, row, "Radius", 1.0f);
    number_or(p, 0x24, lua, row, "Amplitude", 0.1f);
    number_or(p, 0x28, lua, row, "Speed", 1.0f);
}
void read_effect_light_00869fd0(void* p, GuiLua51Host& lua, const GuiLuaRef& row,
    EffectScalarComponentContext& context) {
    read_effect_component_base_00868bf0(p, lua, row, context.crt_sse2_conversion);
    number_or(p, 0x20, lua, row, "Duration", 2.0f);
    number_or(p, 0x24, lua, row, "Radius", 10.0f);
    color(p, 0x28, lua, row, "DiffuseColor", {{0.0f, 1.0f, 0.0f, 1.0f}});
    color(p, 0x38, lua, row, "SpecularColor", {{0.0f, 0.0f, 1.0f, 1.0f}});
}
void read_effect_waterdrops_0086b2b0(void* p, GuiLua51Host& lua, const GuiLuaRef& row,
    EffectScalarComponentContext& context) {
    read_effect_component_base_00868bf0(p, lua, row, context.crt_sse2_conversion);
    number_or(p, 0x20, lua, row, "MinLifeTime", 0.5f);
    number_or(p, 0x24, lua, row, "MaxLifeTime", 4.0f);
    number_or(p, 0x28, lua, row, "MinSize", 0.1f);
    number_or(p, 0x2c, lua, row, "MaxSize", 0.25f);
    number_or(p, 0x30, lua, row, "Gravity", 0.95f);
    number_or(p, 0x34, lua, row, "Radius", 10.0f);
    number_or(p, 0x38, lua, row, "Range", 10.0f);
    number_or(p, 0x3c, lua, row, "StickyPercent", 10.0f);
    truth(p, 0x40, lua, row, "ApplyAtBottomOnly", true);
    truth(p, 0x41, lua, row, "StartTopOffScreen", true);
    truth(p, 0x42, lua, row, "ApplyAtTopOnly", true);
    truth(p, 0x43, lua, row, "NoCameraMove", true);
    {
        FieldRef field{lua, lua.get_by_name(row, "WaterdropCount")};
        store<std::int32_t>(p, 0x44, lua.type_of(field.value) == GuiLuaType::Number
            ? lua_object_integer_00b66290(lua, field.value, context.crt_sse2_conversion) : 50);
    }
    TemporaryName texture{context.strings, {}};
    {
        FieldRef field{lua, lua.get_by_name(row, "Texture")};
        //00B685C0: construct, only a String qualifies; no numeric coercion.
        texture.value.assign_0041e870(context.strings, lua.type_of(field.value) == GuiLuaType::String
            ? lua.to_string(field.value) : "white.tga");
    }
    // Field ref is gone; current renderer is resolved only now. Overwrite
    // actual48 without releasing its old value or retaining the returned one.
    store(p, 0x48, context.textures.load_texture_current_slot_64(texture.value, 0));
}

void destroy_effect_component_base_0086b7e0(void* p, NativeStringStorage& strings) noexcept {
    store<std::uint32_t>(p, 0, 0x00d0d570);
    destroy_native_string_header_0041dd20(static_cast<unsigned char*>(p) + 8, strings);
    store<std::uint32_t>(p, 0, 0x00ceb130); //00BD30F0
}
void destroy_effect_waterdrops_0086cd40(void* p, EffectScalarComponentContext& context) {
    store<std::uint32_t>(p, 0, 0x00d0d634);
    void* const texture = load<void*>(p, 0x48);
    if (texture) {
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(
            static_cast<unsigned char*>(texture) + 4)) == 0)
            context.textures.texture_zero_references_slot_00(texture);
        // A replacement published by the callback is overwritten, not released.
        store<void*>(p, 0x48, nullptr);
    }
    destroy_effect_component_base_0086b7e0(p, context.strings);
}
void* scalar_delete_effect_shake_0086d0a0(void* p, std::uint32_t f, EffectScalarComponentContext& c) {
    return scalar_base(p, f, c);
}
void* scalar_delete_effect_waterdrops_0086d0c0(void* p, std::uint32_t f, EffectScalarComponentContext& c) {
    destroy_effect_waterdrops_0086cd40(p, c);
    if ((f & 1u) != 0) singleton_lifetime_free(p);
    return p;
}
void* scalar_delete_effect_const_rumble_0086d0e0(void* p, std::uint32_t f, EffectScalarComponentContext& c) {
    return scalar_base(p, f, c);
}
void* scalar_delete_effect_slope_rumble_0086d100(void* p, std::uint32_t f, EffectScalarComponentContext& c) {
    return scalar_base(p, f, c);
}
void* scalar_delete_effect_square_rumble_0086d120(void* p, std::uint32_t f, EffectScalarComponentContext& c) {
    return scalar_base(p, f, c);
}
void* scalar_delete_effect_light_0086d140(void* p, std::uint32_t f, EffectScalarComponentContext& c) {
    return scalar_base(p, f, c);
}
void* scalar_delete_effect_splash_0086d160(void* p, std::uint32_t f, EffectScalarComponentContext& c) {
    return scalar_base(p, f, c);
}
void GameplayEffectScalarComponentDispatcher::read_lua_slot_14(void* p, GuiLua51Host& lua,
    const GuiLuaRef& row) {
    const auto vtable = load<std::uint32_t>(p, 0);
    if (vtable == 0x00d0da18) { read_effect_sound_0086ef60(p, lua, row, context_.sound); return; }
    for (const auto& entry : dispatch) {
        if (entry.vtable == vtable) { entry.read(p, lua, row, context_); return; }
    }
    remaining_.read_lua_slot_14(p, lua, row);
}
void GameplayEffectScalarComponentDispatcher::zero_references_slot_00(void* p) {
    const auto vtable = load<std::uint32_t>(p, 0);
    if (vtable == 0x00d0da18) { scalar_delete_effect_sound_0086fb00(p, 1, context_.sound); return; }
    for (const auto& entry : dispatch) {
        // All seven slot0 entries are00BD30E0, which calls current slot4(1).
        if (entry.vtable == vtable) { entry.destroy(p, 1, context_); return; }
    }
    remaining_.zero_references_slot_00(p);
}
} // namespace bsp
