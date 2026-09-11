#pragma once

#include "bsp/panel_sequence_types.hpp"

#include <cstdint>
#include <map>
#include <optional>

struct lua_State;
namespace bsp {
using GameplayEffectNameMap = std::map<NativeString, std::int32_t, PanelSequenceNameLess>;
// Canonical host projection of the static map00F87670 and guard00F8767C.
// Native static storage starts zero. Map nodes own their pooled name keys.
// Native allocator word70 is untouched; head74 is represented by engagement.
struct GameplayEffectNameIndexState {
    std::uint32_t allocator_00f87670{};
    std::optional<GameplayEffectNameMap> entries;
    std::uint32_t count_00f87678{};
    std::uint32_t guard_00f8767c{};
};
struct GameplayEffectNameIndexHost {
    virtual ~GameplayEffectNameIndexHost() = default;
    // Resolve CURRENT[00E188A8]+1A0C once per empty-cache load. Borrow the
    // already-open embedded state; this is not mission_lua at game+1A08.
    virtual lua_State& current_game_lua_1a0c() = 0;
};
struct GameplayEffectNameIndexContext {
    GameplayEffectNameIndexState& state;
    GameplayEffectNameIndexHost& host;
    NativeStringStorage& strings;
    const bool& crt_sse2_conversion;
};
// Setup only; binds the one process cache. Context and borrowed dependencies
// must stay alive through real CRT atexit processing. No initialization here.
void bind_gameplay_effect_name_index_00f87670(GameplayEffectNameIndexContext&) noexcept;
// 00871750: ECX unused, stack NativeString* name; EAX signed ID; RET4.
// Sets guard bit1 before initialization and registers the real cleanup once.
// Empty count reloads current Lua Effects; nonempty cache stays unchanged.
std::int32_t lookup_gameplay_effect_id_00871750(const NativeString& name);
// 00CDEB00..00CDEB3F: no inputs; native EAX=0; RET. Releases map keys in
// descending order, frees head, clears head/count; guard and allocator remain.
// Invoked by actual CRT atexit. Not idempotent; do not call before registered
// process shutdown and then leave the same callback pending.
void destroy_gameplay_effect_name_index_00cdeb00();
// New C++ interfaces. Native STL topology, invalid-iterator behavior and SEH
// remain library/ABI boundaries; see docs/GAMEPLAY_EFFECT_NAME_INDEX.md.
} // namespace bsp
