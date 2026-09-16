#pragma once
#include "bsp/native_string_pool_storage.hpp"

namespace bsp::game {

// Original loader-zero publications outlive the application and every CRT
// callback, including a late 00419CC0 that recreates the manager/string pool.
// This owns cells and source bindings only; getters create the native owners.
class GameNativeStringProcess final {
public:
    GameNativeStringProcess(const GameNativeStringProcess&) = delete;
    GameNativeStringProcess& operator=(const GameNativeStringProcess&) = delete;
    void* volatile& manager_01090aa0() noexcept { return manager_; }
    NativeStringPoolStorage* volatile& pool_01090aa8() noexcept { return pool_; }
    volatile std::uint32_t& returns_disabled_01090aa4() noexcept { return disabled_; }
    NativeStringRawPoolContext& raw_context() noexcept { return raw_; }
    ActualNativeStringPoolStorage& strings() noexcept { return strings_; }
private:
    friend GameNativeStringProcess& game_native_string_process();
    GameNativeStringProcess() noexcept = default;
    void* volatile manager_{};
    NativeStringPoolStorage* volatile pool_{};
    volatile std::uint32_t disabled_{};
    NativeStringRawPoolContext raw_{pool_, disabled_, manager_};
    ActualNativeStringPoolStorage strings_{pool_, disabled_, manager_};
};

// Intentionally retained through process termination. No C++ exit destructor
// may invalidate the cells/bindings before another native CRT callback.
GameNativeStringProcess& game_native_string_process();
} // namespace bsp::game
