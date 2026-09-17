#pragma once
#include "bsp/native_material_pools.hpp"
#include "bsp/native_shader_compiler_lookups.hpp"
#include "bsp/native_startup_shader_modes.hpp"
#include <mutex>

namespace bsp::game {
class GameNativeReadOnlyData;
// One process domain: original0108FEE4 pool, mode bytes and108D6EC cache cell.
// Native pool startup/shutdown use the SAME E188B4 allocator list and real CRT
// exit callback as other production pools. Storage survives all exit callbacks.
class GameNativeShaderProcess final {
public:
    GameNativeShaderProcess(const GameNativeShaderProcess&)=delete;
    GameNativeShaderProcess& operator=(const GameNativeShaderProcess&)=delete;
    int initialize_state_pool_once_00cd7cc0();
    NativeShaderStateListPool& state_list_pool_0108fee4();
    void configure_startup_modes(const char*,GameNativeReadOnlyData&);
    NativeStartupShaderModes& modes() noexcept {return modes_;}
    const NativeStartupShaderModeOperation& mode_operation() const noexcept {return mode_operation_;}
    NativeShaderBinaryCacheStorage* volatile& cache_0108d6ec() noexcept {return cache_;}
    char* stream_empty_0109db64() noexcept {return &stream_empty_;}
private:
    friend GameNativeShaderProcess& game_native_shader_process();
    GameNativeShaderProcess();
    enum class State { unattempted, returned, threw };
    NativeShaderStateListPoolStorage pool_storage_{};
    NativeShaderStateListPool pool_;
    std::mutex mutex_;
    State state_{State::unattempted};
    int registration_status_{};
    NativeStartupShaderModes modes_;
    NativeStartupShaderModeOperation mode_operation_;
    NativeShaderBinaryCacheStorage* volatile cache_{};
    char stream_empty_{};
};
GameNativeShaderProcess& game_native_shader_process();
} // namespace bsp::game
