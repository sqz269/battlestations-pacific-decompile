#include "bsp/gameplay_effect_name_index.hpp"

#include "bsp/gui_lua_runtime.hpp"
#include "bsp/lua_object.hpp"

#include <cstdlib>
#include <iterator>
#include <utility>

namespace bsp {
namespace {
GameplayEffectNameIndexContext* bound_context;
struct EffectIndexOwnedRef {
    GuiLua51Host& lua;
    GuiLuaRef value;
    ~EffectIndexOwnedRef() { lua.release(value); }
};
struct EffectIndexOwnedName {
    NativeStringStorage& storage;
    NativeString value;
    ~EffectIndexOwnedName() { destroy_native_string_header_0041dd20(&value, storage); }
};
void insert_effect_name(const NativeString& name, std::int32_t id,
    GameplayEffectNameIndexContext& context) {
    // 00505BA0 unique insertion retains an existing value on equivalent keys.
    auto& entries = *context.state.entries;
    auto found = entries.lower_bound(name);
    if (found != entries.end() && !entries.key_comp()(name, found->first)) return;
    EffectIndexOwnedName copy{context.strings, {}};
    // 00502170 initializes a separate node key and deep-copies the pair key.
    copy.value.copy_from_00be0a30_fragment(context.strings, name);
    entries.emplace_hint(found, std::move(copy.value), id);
    ++context.state.count_00f87678;
}
void load_effect_names(GameplayEffectNameIndexContext& context) {
    GuiLua51Host lua(context.host.current_game_lua_1a0c());
    GuiLuaRef effects_ref;
    {
        EffectIndexOwnedRef globals{lua, lua.globals()};
        effects_ref = lua.get_by_name(globals.value, "Effects");
    } // Globals temporary is released before iteration objects are constructed.
    EffectIndexOwnedRef effects{lua, effects_ref};
    LuaTableScan scan(lua, effects.value);
    while (!scan.at_end()) {
        if (lua.type_of(scan.key()) == GuiLuaType::Number) {
            EffectIndexOwnedRef field{lua, lua.get_by_name(scan.value(), "Name")};
            if (lua.type_of(field.value) == GuiLuaType::String) {
                // 008718B3's first conversion is discarded, but its FP effects
                // precede the first string allocation and must still occur.
                (void)lua_object_integer_00b66290(lua, scan.key(), context.crt_sse2_conversion);
                EffectIndexOwnedName first{context.strings, {}};
                first.value.assign_0041e870(context.strings, lua.to_string(field.value));
                const auto id = lua_object_integer_00b66290(
                    lua, scan.key(), context.crt_sse2_conversion);
                EffectIndexOwnedName pair_key{context.strings, {}};
                pair_key.value.assign_0041e870(context.strings, lua.to_string(field.value));
                insert_effect_name(pair_key.value, id, context);
            } // Pair key, first temporary, then the retained Name Lua reference.
        }
        scan.advance();
    } // Value, key, then retained Effects reference.
}
} // namespace

void bind_gameplay_effect_name_index_00f87670(GameplayEffectNameIndexContext& context) noexcept {
    bound_context = &context;
}
std::int32_t lookup_gameplay_effect_id_00871750(const NativeString& name) {
    auto& context = *bound_context;
    auto& state = context.state;
    if ((state.guard_00f8767c & 1u) == 0) {
        state.guard_00f8767c |= 1u; // Before allocation; no rollback on failure.
        state.entries.emplace();
        state.count_00f87678 = 0;
        (void)std::atexit(&destroy_gameplay_effect_name_index_00cdeb00);
    }
    if (state.count_00f87678 == 0) load_effect_names(context);
    auto& entries = *state.entries;
    const auto found = entries.lower_bound(name); // 004BF370
    if (found == entries.end() || entries.key_comp()(name, found->first)) return 0;
    return found->second;
}
void destroy_gameplay_effect_name_index_00cdeb00() {
    auto& context = *bound_context;
    auto& state = context.state;
    auto& entries = *state.entries;
    // 0058B520 visits right subtree, current key, then left. Current count
    // remains published until all nodes have been destroyed by range erase.
    while (!entries.empty()) {
        const auto current = std::prev(entries.end());
        destroy_native_string_header_0041dd20(
            const_cast<NativeString*>(&current->first), context.strings);
        entries.erase(current);
    }
    state.count_00f87678 = 0;
    state.entries.reset(); // Native then frees the current sentinel.
    state.count_00f87678 = 0; //00CDEB36 after head free and head null publication.
}
} // namespace bsp
