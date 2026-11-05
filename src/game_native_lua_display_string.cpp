#include "bsp/game_native_lua_display_string.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_string_float_format.hpp"
#include "bsp/gui_lua_runtime.hpp"
#include <stdexcept>

namespace bsp::game {
namespace {
void construct_number(void* context, NativeString& fresh, float value) {
    auto& strings = *static_cast<NativeStringRawPoolContext*>(context);
    (void)construct_native_string_float_header_0043bdf0(&fresh, strings, value);
}
} // namespace

GameNativeLuaDisplayStringBindings::GameNativeLuaDisplayStringBindings(
    GameNativeStringProcess& process) noexcept
    : calls_{process.strings(), &construct_number, &process.raw_context()} {}

NativeString* GameNativeLuaDisplayStringBindings::to_display_string(
    const NativeLuaObjectStorage& object, void* fresh) const {
    return native_lua_to_display_string_00b68630(object, fresh, calls_);
}

void GameNativeLuaDisplayStringBindings::discard_display_string(
    GuiLua51Host& lua, const GuiLuaRef& value) const {
    alignas(NativeString) std::byte fresh[sizeof(NativeString)];
    NativeString* const result = lua.to_display_string(value, fresh, calls_);
    // A808DA..A808FC releases this completed temporary before EqualsCString.
    // Keep the raw getter/return exception policy, rather than a noexcept
    // semantic release: all three pool publication/gate cells are the same.
    auto& raw = *static_cast<NativeStringRawPoolContext*>(calls_.context);
    destroy_native_string_header_0041dd20(result, raw);
}

void discard_sound_configuration_key_display(GuiLuaHost& lua,
    const GuiLuaRef& value, NativeStringStorage& sound_strings) {
    auto* const actual = dynamic_cast<ActualNativeStringPoolStorage*>(&sound_strings);
    if (!actual) return; // Existing semantic fixtures have no raw pool binding.
    auto& process = game_native_string_process();
    auto* const reader = dynamic_cast<GuiLua51Host*>(&lua);
    if (!reader || !actual->uses_raw_pool_cells(process.raw_context()))
        throw std::logic_error("Sound key display requires the same actual Lua/string domain");
    GameNativeLuaDisplayStringBindings display(process);
    display.discard_display_string(*reader, value);
}
} // namespace bsp::game
