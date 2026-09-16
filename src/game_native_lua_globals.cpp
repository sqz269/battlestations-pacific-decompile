#include "bsp/game_native_lua_globals.hpp"
#include "bsp/game_native_string_process.hpp"
#include <stdexcept>

namespace bsp::game {
GameNativeLuaGlobalsProcess::GameNativeLuaGlobalsProcess()
    : globals_(make_initial_lua_runtime_globals_0108ff20()),
      lifetime_{globals_.region, game_native_string_process().raw_context()} {}
GameNativeLuaGlobalsProcess& game_native_lua_globals_process() {
    static auto* const process = new GameNativeLuaGlobalsProcess;
    return *process;
}
int GameNativeLuaGlobalsProcess::initialize_once_00cd7ce0() {
    std::lock_guard lock(mutex_);
    if (phase_ == Phase::returned) return registration_;
    if (phase_ == Phase::threw)
        throw std::logic_error("Lua region static startup previously threw");
    phase_ = Phase::threw;
    bind_static_native_lua_region_0108ff24(lifetime_);
    registration_ = initialize_static_native_lua_region_00cd7ce0();
    phase_ = Phase::returned;
    return registration_;
}
LuaRuntimeGlobals& GameNativeLuaGlobalsProcess::globals_0108ff20() {
    std::lock_guard lock(mutex_);
    if (phase_ != Phase::returned)
        throw std::logic_error("Lua globals require explicit static startup");
    return globals_;
}
} // namespace bsp::game
