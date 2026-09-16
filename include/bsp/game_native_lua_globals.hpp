#pragma once
#include "bsp/lua_runtime_globals.hpp"
#include "bsp/native_lua_globals_lifetime.hpp"
#include "bsp/native_lua_fundamentals.hpp"
#include <mutex>

namespace bsp::game {
class GameNativeLuaGlobalsProcess final {
public:
    GameNativeLuaGlobalsProcess(const GameNativeLuaGlobalsProcess&) = delete;
    GameNativeLuaGlobalsProcess& operator=(const GameNativeLuaGlobalsProcess&) = delete;
    // Cache the actual registration result without rollback; no retry after
    // an exception. Repeated startup never resets either native global.
    int initialize_once_00cd7ce0();
    LuaRuntimeGlobals& globals_0108ff20();
    NativeLuaFundamentalsOwner* volatile& fundamentals_0108ff1c() noexcept {
        return fundamentals_;
    }
private:
    friend GameNativeLuaGlobalsProcess& game_native_lua_globals_process();
    GameNativeLuaGlobalsProcess();
    enum class Phase { unattempted, returned, threw };
    NativeLuaFundamentalsOwner* volatile fundamentals_{};
    LuaRuntimeGlobals globals_;
    NativeLuaRegionLifetimeContext lifetime_;
    std::mutex mutex_;
    Phase phase_{Phase::unattempted};
    int registration_{};
};
// Cells and bookkeeping intentionally survive all real CRT callbacks. Native
// CE0D60 owns region cleanup; C++ destruction must neither repeat nor preempt it.
GameNativeLuaGlobalsProcess& game_native_lua_globals_process();
} // namespace bsp::game
