#include "bsp/gameplay_effect_acquisition.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace bsp {
namespace {
struct AcquisitionLuaRef {
    GuiLua51Host& lua;
    GuiLuaRef value;
    ~AcquisitionLuaRef() { lua.release(value); }
};
struct AcquisitionName {
    NativeStringStorage& storage;
    NativeString value;
    ~AcquisitionName() { destroy_native_string_header_0041dd20(&value, storage); }
};
} // namespace

void** acquire_gameplay_effect_by_id_008700e0(GameplayEffectManager& manager,
    void*& out, std::int32_t id, std::uint32_t flag,
    GameplayEffectAcquisitionContext& context) {
    static_assert(sizeof(void*) == 4 && sizeof(LONG) == 4);
    if (id <= 0) {
        out = nullptr;
        return &out;
    }
    auto& entries = *manager.definitions;
    auto found = entries.find(id); //0086B650, checked owner/node iterator.
    if (found != entries.end()) {
        auto* const refs = reinterpret_cast<volatile LONG*>(
            static_cast<unsigned char*>(found->second) + 4);
        InterlockedIncrement(refs);
    } else {
        GuiLua51Host lua(context.host.current_game_lua_1a0c());
        GuiLuaRef effects_ref;
        {
            AcquisitionLuaRef globals{lua, lua.globals()};
            effects_ref = lua.get_by_name(globals.value, "Effects");
        }
        AcquisitionLuaRef effects{lua, effects_ref};
        AcquisitionLuaRef definition{lua, lua.get_by_index(effects.value, id)};
        if (static_cast<std::uint8_t>(flag) == 0
            && lua.type_of(definition.value) != GuiLuaType::Table) {
            out = nullptr; // Store before definition/Effects references die.
            return &out;
        }
        AcquisitionLuaRef name_field{lua, lua.get_by_name(definition.value, "Name")};
        AcquisitionName name{context.strings, {}};
        name.value.assign_0041e870(context.strings, lua.to_string(name_field.value));
        auto* const fresh = allocate_gameplay_effect_definition_00870240_fragment();
        load_gameplay_effect_components_00870400(
            *fresh, lua, definition.value, context.strings, context.components);
        set_gameplay_effect_definition_identity_0086b870(
            *fresh, id, &name.value, context.strings);
        //0086F930: keep an equivalent existing ID, even after reentrant load.
        // The native raw local is neither retained nor released here; a
        // losing fresh allocation is not given invented rollback/cleanup.
        found = entries.lower_bound(id);
        if (found == entries.end() || id < found->first)
            found = entries.emplace_hint(found, id, fresh);
    } // Name string, Name Lua ref, definition Lua ref, then Effects Lua ref.
    out = found->second; //00870345: reread AFTER all miss-path cleanup.
    return &out;
}

void** acquire_gameplay_effect_by_name_00871b50(GameplayEffectManager& manager,
    void*& out, const NativeString& name, std::uint32_t flag,
    GameplayEffectAcquisitionContext& context) {
    if (name.length() == 0) {
        out = nullptr;
        return &out;
    }
    const auto id = lookup_gameplay_effect_id_00871750(name);
    acquire_gameplay_effect_by_id_008700e0(manager, out, id, flag, context);
    return &out;
}

void** acquire_gameplay_effect_by_name_00871ba0(void*& out,
    const NativeString& name, std::uint32_t flag, GameplayEffectAcquisitionContext& context) {
    auto* const manager = get_gameplay_effect_manager_004c1650(context.manager);
    acquire_gameplay_effect_by_name_00871b50(*manager, out, name, flag, context);
    return &out;
}
} // namespace bsp
