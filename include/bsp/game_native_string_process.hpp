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
    // Actual pending-registry publication shares this retained process lifetime.
    // Accessing the cell does not construct or register a registry owner.
    void* volatile& pending_registry_00f878cc() noexcept;
    // Permanent allocation-statistics publication; cell access creates no owner.
    void* volatile& allocation_stats_0109cefc() noexcept;
    NativeStringPoolStorage* volatile& pool_01090aa8() noexcept { return pool_; }
    volatile std::uint32_t& returns_disabled_01090aa4() noexcept { return disabled_; }
    NativeStringRawPoolContext& raw_context() noexcept { return raw_; }
    ActualNativeStringPoolStorage& strings() noexcept { return strings_; }
    // Distinct permanent Source authorities for the two Original initial-NUL
    // roles. Only const pointers escape; Original's writable .data is not
    // claimed universally immutable. Shader E17654 borrowers use this SAME byte.
    const char* property_empty_00e177e4() const noexcept;
    const char* query_empty_00e17654() const noexcept;
private:
    friend GameNativeStringProcess& game_native_string_process();
    GameNativeStringProcess() noexcept = default;
    void* volatile manager_{};
    NativeStringPoolStorage* volatile pool_{};
    volatile std::uint32_t disabled_{};
    NativeStringRawPoolContext raw_{pool_, disabled_, manager_};
    ActualNativeStringPoolStorage strings_{pool_, disabled_, manager_};
    // Append after every established publication/context/storage field.
    const char property_empty_00e177e4_{};
    const char query_empty_00e17654_{};
    // Append after every established authority; do not reuse another raw8 owner.
    void* volatile pending_registry_00f878cc_{};
    void* volatile allocation_stats_0109cefc_{};
};

// Intentionally retained through process termination. No C++ exit destructor
// may invalidate the cells/bindings before another native CRT callback.
GameNativeStringProcess& game_native_string_process();
} // namespace bsp::game
