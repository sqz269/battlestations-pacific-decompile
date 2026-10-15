#include "bsp/game_native_lua_display_string.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_string_float_format.hpp"

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
} // namespace bsp::game
