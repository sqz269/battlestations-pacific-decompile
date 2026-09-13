#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>

namespace bsp::game {
// Source service for original numeric table/literal reads. The input must be
// the exact supported game executable; only its verified .rdata bytes are
// mapped, at CE2000..E07B23, with read-only and non-executable protection.
// No original code executes or imports resolve. Mutable globals, object owners
// and callable host methods must be supplied by their own source services.
// Retain this object through every raw consumer and its singleton drain.
class GameNativeReadOnlyData final {
public:
    explicit GameNativeReadOnlyData(const std::filesystem::path& original_executable);
    ~GameNativeReadOnlyData() noexcept;
    GameNativeReadOnlyData(const GameNativeReadOnlyData&) = delete;
    GameNativeReadOnlyData& operator=(const GameNativeReadOnlyData&) = delete;
    GameNativeReadOnlyData(GameNativeReadOnlyData&&) = delete;
    GameNativeReadOnlyData& operator=(GameNativeReadOnlyData&&) = delete;

    // Bounds-check a required table/literal span before composing a consumer.
    // This returns data at its recorded address; numeric entries are not code.
    const void* data_at(std::uintptr_t address, std::size_t bytes) const;
    static constexpr std::uintptr_t begin_address = 0x00ce2000;
    static constexpr std::size_t byte_count = 0x00125b24;
private:
    void* reservation_{};
};
} // namespace bsp::game
