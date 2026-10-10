#pragma once

#include "bsp/gameplay_effect_definition.hpp"
#include "bsp/gameplay_effect_scalar_components.hpp"
#include "bsp/gameplay_effect_name_index.hpp"
#include "bsp/gui_lua_runtime.hpp"

namespace bsp {
// Concrete definition allocation publishes D0DA58, whose slot8 is00870400.
// Its construction/iteration and seven component reader/cleanup pairs run
// directly. Other current component tables require remaining services.
struct GameplayEffectAcquisitionHost : GameplayEffectNameIndexHost,
    GameplayEffectComponentServices {};
struct GameplayEffectAcquisitionContext {
    GameplayEffectManagerContext& manager;
    NativeStringStorage& strings;
    GameplayEffectAcquisitionHost& host;
    GameplayEffectScalarComponentDispatcher& components;
};

// ID-only raw-manager composition. The existing Lua selector is used directly;
// this needs no name-index cache/setup or by-name acquisition binding.
struct NativeGameplayEffectIdAcquisitionContext {
    NativeGameplayEffectManagerCells& manager;
    GameplayEffectNameIndexHost& current_lua;
    NativeStringStorage& strings;
    GameplayEffectComponentServices& components;
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

// Ordinary Source raw10h overload. Uses the actual tree at manager+4 and the
// existing raw find/unique-insert providers. For positive IDs the manager,
// selected node and actual payload/reference storage must stay valid across
// callbacks. Miss cleanup precedes the current node-value reload into out;
// duplicate insertion adds no retain or losing-fresh cleanup. ID<=0 writes
// null without reading/releasing old out or accessing manager storage.
//
// Lua uses the established GuiLua51Host registry-reference adaptation, not
// Native14h tracked Lua objects. Calling concrete00870400 retains that existing
// Source loader operation; it proves no current native virtual+8/+14 binding.
// The caller supplies genuine current Lua/string/component services and the
// same raw cells/destruction domain. No defaults, slot0/companion adapter,
// prebinding, application activation, Native register/FH3/SEH proof is added.
void** acquire_gameplay_effect_by_id_008700e0(void* actual_raw_manager,
    void*& out, std::int32_t id, std::uint32_t flag,
    NativeGameplayEffectIdAcquisitionContext&);

// Capture actual output/ID, call the actual raw getter unconditionally, then
// load the borrowed full flag DWORD (even for ID<=0). Return captured output,
// ignoring the lower return. The lower tests only flag's low byte. Output and
// flag are distinct live objects; no initialization, early clear or release.
// Stable bindings/private locals stay disjoint across callbacks. The separate
// Lua integer Source bool and Native DWORD conversion modes are not this flag.
void** acquire_gameplay_effect_by_id_00870cd0(void*& out, std::int32_t id,
    const volatile std::uint32_t& actual_flag_word,
    NativeGameplayEffectIdAcquisitionContext&);
} // namespace bsp
