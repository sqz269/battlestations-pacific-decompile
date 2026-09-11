#pragma once

#include "bsp/gameplay_effect_definition.hpp"
#include "bsp/gameplay_effect_name_index.hpp"
#include "bsp/gui_lua_runtime.hpp"

namespace bsp {
struct GameplayEffectAcquisitionHost : GameplayEffectNameIndexHost {
    // D0DA58[8] ->00870400. Dispatch the actual definition's current slot+8
    // with the retained Lua definition. Component construction/loading is
    // still a required service; the acquisition sequence executes directly.
    virtual void load_definition_slot_08(GameplayEffectDefinition&,
        GuiLua51Host&, const GuiLuaRef& definition) = 0;
};
struct GameplayEffectAcquisitionContext {
    GameplayEffectManagerContext& manager;
    NativeStringStorage& strings;
    GameplayEffectAcquisitionHost& host;
};

//008700E0: ECX manager, stack out/ID/flag; EAX out, RETCh. ID<=0 stores
// null without releasing an old output. Hits retain; misses read current
// game+1A0C Effects[ID], reject a non-table only when flag's low byte is0,
// load/identify a fresh definition and uniquely insert its weak pointer.
// Temporary cleanup precedes the final cache-value reload into out.
void** acquire_gameplay_effect_by_id_008700e0(GameplayEffectManager&,
    void*& out, std::int32_t id, std::uint32_t flag,
    GameplayEffectAcquisitionContext&);
//00871B50: ECX manager, stack out/name/flag; EAX out, RETCh. Empty name
// stores null. Otherwise use concrete00871750 then concrete008700E0.
void** acquire_gameplay_effect_by_name_00871b50(GameplayEffectManager&,
    void*& out, const NativeString&, std::uint32_t flag,
    GameplayEffectAcquisitionContext&);
//00871BA0: ECX out, EDX name, stack flag; EAX out, RET4. Captures the
// output/name addresses, gets the current manager, then invokes00871B50.
// Uses the process name-index binding installed by its explicit setup API.
void** acquire_gameplay_effect_by_name_00871ba0(void*& out,
    const NativeString&, std::uint32_t flag, GameplayEffectAcquisitionContext&);
} // namespace bsp
