#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>

namespace bsp::game {

// Own only the verified initial-image bytes consumed by explicit-pointer VFS
// contexts. Original E144F0/E17BF0 reside in writable .data; this is a stable
// source snapshot, not a live alias of any mutable game-process global.
class GameNativeVfsConstants final {
public:
    static constexpr std::size_t mpkg_xor_key_bytes = 0x219;

    explicit GameNativeVfsConstants(const std::filesystem::path& original_executable);
    GameNativeVfsConstants(const GameNativeVfsConstants&) = delete;
    GameNativeVfsConstants& operator=(const GameNativeVfsConstants&) = delete;
    GameNativeVfsConstants(GameNativeVfsConstants&&) = delete;
    GameNativeVfsConstants& operator=(GameNativeVfsConstants&&) = delete;

    // Retain this owner through all consumers and their raw singleton drain.
    // NativeMpkgArchiveContext accepts the key as const volatile uint8_t*.
    const std::uint8_t* mpkg_xor_key_00e144f0() const noexcept { return mpkg_xor_key_.data(); }
    // The native fallback is an empty C string: exactly one required NUL byte.
    const char* null_pattern_00e17bf0() const noexcept { return null_pattern_.data(); }

private:
    std::array<std::uint8_t,mpkg_xor_key_bytes> mpkg_xor_key_{};
    std::array<char,1> null_pattern_{};
};

} // namespace bsp::game
