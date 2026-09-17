#pragma once
#include "bsp/native_material_pools.hpp"
#include "bsp/native_shader_compiler_lookups.hpp"
#include "bsp/native_startup_shader_modes.hpp"
#include <mutex>
#include <array>

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
    volatile std::uint32_t& system_register_limit_00e13078() noexcept {return register_limit_;}
    char* format_scratch_0108d6f8() noexcept {return format_scratch_.data();}
    const char* source_empty_0108d6f2() const noexcept {return &source_empty_;}
    const char* instance_empty_00e17654() const noexcept {return &instance_empty_;}
    // B35110 uses an unbounded formatter. Its normal input must fit this
    // region: 0108D6F8 up to the separate texture counter at 0108DAF8.
    static constexpr std::size_t format_scratch_bytes=0x400;
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
    volatile std::uint32_t register_limit_{77}; // Original image E13078 = 4D.
    std::array<char,format_scratch_bytes> format_scratch_{};
    char source_empty_{}; // Loader-zero 0108D6F2; distinct from E17654.
    char instance_empty_{};
};
GameNativeShaderProcess& game_native_shader_process();
} // namespace bsp::game
