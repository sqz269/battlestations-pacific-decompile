#include "bsp/gameplay_effect_components.hpp"

#include "bsp/lua_object.hpp"
#include "bsp/xlive_updates.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <array>
#include <cstring>
#include <initializer_list>
#include <new>

namespace bsp {
namespace {
template<class T> void store(void* owner, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<unsigned char*>(owner) + offset, &value, sizeof value);
}
void word(void* owner, std::size_t offset, std::uint32_t value) noexcept {
    store(owner, offset, value);
}
void zeros(void* owner, std::initializer_list<std::size_t> offsets) noexcept {
    for (const auto offset : offsets) word(owner, offset, 0);
}
void base_component(void* owner, std::uint32_t vtable) noexcept {
    word(owner, 0, 0x00ceb130);
    word(owner, 4, 1);
    // Begin the actual8h C++ string subobject's lifetime while performing
    // exactly the two native zero stores, with no allocation or release.
    new (static_cast<unsigned char*>(owner) + 8) NativeString;
    store<std::uint8_t>(owner, 0x10, 0);
    zeros(owner, {0x14, 0x18});
    word(owner, 0, vtable);
}
struct ComponentLuaRef {
    GuiLuaHost& lua;
    GuiLuaRef value;
    ~ComponentLuaRef() { lua.release(value); }
};
struct ComponentName {
    NativeStringStorage& storage;
    NativeString value;
    ~ComponentName() { destroy_native_string_header_0041dd20(&value, storage); }
};
bool same_name(const NativeString& name, const char* literal) {
    return name.data() && _stricmp(name.data(), literal) == 0;
}
bool reserved_key(GuiLuaHost& lua, const GuiLuaRef& key, NativeStringStorage& storage) {
    if (lua.type_of(key) != GuiLuaType::String) return false;
    // Three distinct pooled temporaries, short-circuit allocation, reverse
    // destruction BEFORE any Type lookup. Re-fetch the Lua string each time.
    ComponentName first{storage, {}};
    first.value.assign_0041e870(storage, lua.to_string(key));
    if (same_name(first.value, "Name")) return true;
    ComponentName second{storage, {}};
    second.value.assign_0041e870(storage, lua.to_string(key));
    if (same_name(second.value, "Type")) return true;
    ComponentName third{storage, {}};
    third.value.assign_0041e870(storage, lua.to_string(key));
    return same_name(third.value, "Comment");
}
struct ComponentType {
    const char* name;
    std::uint32_t bytes;
    void* (*construct)(void*) noexcept;
    std::uint32_t component_offset;
};
constexpr std::array<ComponentType, 13> component_types{{
    {"Sound", 0x3c, construct_effect_sound_00870733_fragment, 0},
    {"Particle", 0x34, construct_effect_particle_0086bc80, 0},
    {"Tracer", 0xa0, construct_effect_tracer_0086b8b0, 0x80},
    {"WaterTracer", 0xc4, construct_effect_water_tracer_0086bcc0, 0},
    {"Shake", 0x2c, construct_effect_shake_0086bd30, 0},
    {"Flare", 0x34, construct_effect_flare_0086bd70, 0},
    {"ThunderStorm", 0x58, construct_effect_thunderstorm_0086fec0, 0},
    {"Waterdrops", 0x4c, construct_effect_waterdrops_0086bdc0, 0},
    {"ConstRumble", 0x30, construct_effect_const_rumble_0086cf90, 0},
    {"SlopeRumble", 0x30, construct_effect_slope_rumble_0086cfe0, 0},
    {"SquareRumble", 0x40, construct_effect_square_rumble_0086d030, 0},
    {"Light", 0x48, construct_effect_light_0086be00, 0},
    {"Splash", 0x2c, construct_effect_splash_0086be40, 0},
}};
void* make_component(const NativeString& name) {
    for (const auto& type : component_types) {
        if (!same_name(name, type.name)) continue;
        void* const storage = singleton_lifetime_allocate(
            {SingletonAllocationKind::object, type.bytes, type.bytes});
        if (!storage) return nullptr;
        void* const allocation = type.construct(storage);
        return static_cast<unsigned char*>(allocation) + type.component_offset;
    }
    return nullptr;
}
struct ComponentTemporary {
    void* value;
    void* const captured;
    GameplayEffectComponentServices& services;
    ~ComponentTemporary() {
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(
            static_cast<unsigned char*>(captured) + 4)) == 0)
            services.zero_references_slot_00(captured);
    }
};
float load_default_float(const void* address) noexcept {
    //00868C8B..91: x87 load/store of current18 before the default accessor.
    float value;
    __asm {
        mov eax, address
        fld dword ptr [eax]
        fstp dword ptr [value]
    }
    return value;
}
} // namespace

void* construct_effect_sound_00870733_fragment(void* p) noexcept {
    base_component(p, 0x00d0da18); zeros(p, {0x20, 0x24, 0x28}); return p;
}
void* construct_effect_particle_0086bc80(void* p) noexcept {
    base_component(p, 0x00d0d5b4); zeros(p, {0x20, 0x24, 0x28, 0x2c, 0x30}); return p;
}
void* construct_effect_tracer_0086b8b0(void* p) noexcept {
    word(p, 0, 0x00d0d4a4); zeros(p, {0x4c, 0x50, 0x54});
    // Verified read-only image constants, copied as exact binary32 words.
    word(p, 0x10, 0x3f800000); //D7A24C
    word(p, 0x14, 0x3dcccccd); //D7A2F0
    word(p, 0x28, 0x7f7fffff); //D7A248
    zeros(p, {0x18, 0x1c}); store<std::uint8_t>(p, 4, 0);
    zeros(p, {8, 0xc}); word(p, 0x20, 3);
    word(p, 0x2c, 0x41200000); //CE38B8
    zeros(p, {0x40, 0x44}); store<std::uint8_t>(p, 5, 1);
    word(p, 0x80, 0x00ceb130); word(p, 0x84, 1);
    word(p, 0x80, 0x00d0d570);
    new (static_cast<unsigned char*>(p) + 0x88) NativeString;
    store<std::uint8_t>(p, 0x90, 0); word(p, 0x94, 0);
    word(p, 0, 0x00d0d5b0); word(p, 0x80, 0x00d0d590);
    word(p, 0x98, 0x453b8000); //CFA424
    return p;
}
void* construct_effect_water_tracer_0086bcc0(void* p) noexcept {
    base_component(p, 0x00d0d5d4);
    zeros(p, {0x48, 0x4c, 0x50, 0x54, 0x64, 0x68, 0x6c, 0x70, 0x8c}); return p;
}
void* construct_effect_shake_0086bd30(void* p) noexcept {
    base_component(p, 0x00d0d5f4); return p;
}
void* construct_effect_flare_0086bd70(void* p) noexcept {
    base_component(p, 0x00d0d614); zeros(p, {0x28, 0x2c, 0x30}); return p;
}
void* construct_effect_thunderstorm_0086fec0(void* p) noexcept {
    base_component(p, 0x00d0da38); zeros(p, {0x40, 0x44, 0x48, 0x4c, 0x50}); return p;
}
void* construct_effect_waterdrops_0086bdc0(void* p) noexcept {
    base_component(p, 0x00d0d634); return p;
}
void* construct_effect_const_rumble_0086cf90(void* p) noexcept {
    base_component(p, 0x00d0d76c); return p;
}
void* construct_effect_slope_rumble_0086cfe0(void* p) noexcept {
    base_component(p, 0x00d0d78c); return p;
}
void* construct_effect_square_rumble_0086d030(void* p) noexcept {
    base_component(p, 0x00d0d7ac); return p;
}
void* construct_effect_light_0086be00(void* p) noexcept {
    base_component(p, 0x00d0d654); return p;
}
void* construct_effect_splash_0086be40(void* p) noexcept {
    base_component(p, 0x00d0d674); return p;
}

void load_gameplay_effect_components_00870400(GameplayEffectDefinition& definition,
    GuiLua51Host& lua, const GuiLuaRef& table, NativeStringStorage& storage,
    GameplayEffectComponentServices& services) {
    LuaTableScan scan(lua, table);
    while (!scan.at_end()) {
        if (!reserved_key(lua, scan.key(), storage)) {
            ComponentName type{storage, {}};
            {
                ComponentLuaRef field{lua, lua.get_by_name(scan.value(), "Type")};
                type.value.assign_0041e870(storage, lua.to_string(field.value));
            } // Type Lua ref released before Platform lookup; type string lives.
            bool enabled;
            {
                ComponentLuaRef field{lua, lua.get_by_name(scan.value(), "Platform")};
                enabled = lua.type_of(field.value) != GuiLuaType::Boolean
                    || lua.to_boolean(field.value);
            }
            if (enabled) {
                void* const component = make_component(type.value);
                if (component) {
                    services.read_lua_slot_14(component, lua, scan.value());
                    if (lua.type_of(scan.key()) == GuiLuaType::String)
                        assign_native_cstring_0041e350(
                            *std::launder(reinterpret_cast<NativeString*>(
                                static_cast<unsigned char*>(component) + 8)),
                            lua.to_string(scan.key()), storage);
                    ComponentTemporary temporary{component, component, services};
                    append_gameplay_effect_component_0086eb60(
                        definition.native.data() + 8, &temporary.value, services);
                } // Release temporary after append and before type destruction.
            }
        }
        scan.advance();
    }
}

void read_effect_component_base_00868bf0(void* component, GuiLuaHost& lua,
    const GuiLuaRef& table, const bool& crt_sse2_conversion) {
    {
        ComponentLuaRef field{lua, lua.get_by_name(table, "Autostart")};
        store<std::uint8_t>(component, 0x10, lua.to_boolean(field.value) ? 1 : 0);
    }
    {
        ComponentLuaRef field{lua, lua.get_by_name(table, "Delay")};
        const auto delay = lua.type_of(field.value) == GuiLuaType::Number
            ? lua_object_integer_00b66290(lua, field.value, crt_sse2_conversion) : 0;
        store(component, 0x14, delay);
    }
    {
        ComponentLuaRef field{lua, lua.get_by_name(table, "NoFilterDist")};
        const auto fallback = load_default_float(static_cast<unsigned char*>(component) + 0x18);
        const auto distance = lua.type_of(field.value) == GuiLuaType::Number
            ? lua_object_number_00b66270(lua, field.value) : fallback;
        store(component, 0x18, distance);
    }
}
} // namespace bsp
