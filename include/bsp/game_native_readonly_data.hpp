#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <array>

namespace bsp::game {
// Source service for original numeric table/literal reads. The input must be
// the exact supported game executable. Requested spans select 64-KB bands of
// verified .rdata within CE2000..E07B23; committed pages are read-only/non-executable.
// No original code executes or imports resolve. Mutable globals, object owners
// and callable host methods must be supplied by their own source services.
// Retain this object through every raw consumer and its singleton drain.
struct GameNativeDataSpan {
    std::uintptr_t address;
    std::size_t bytes;
};
class GameNativeReadOnlyData final {
public:
    GameNativeReadOnlyData(const std::filesystem::path& original_executable,
        const GameNativeDataSpan* required_spans, std::size_t span_count);
    ~GameNativeReadOnlyData() noexcept;
    GameNativeReadOnlyData(const GameNativeReadOnlyData&) = delete;
    GameNativeReadOnlyData& operator=(const GameNativeReadOnlyData&) = delete;
    GameNativeReadOnlyData(GameNativeReadOnlyData&&) = delete;
    GameNativeReadOnlyData& operator=(GameNativeReadOnlyData&&) = delete;

    // Check section bounds and mapping of a nonempty table/literal span.
    // This returns data at its recorded address; numeric entries are not code.
    const void* data_at(std::uintptr_t address, std::size_t bytes) const;
    static constexpr std::uintptr_t begin_address = 0x00ce2000;
    static constexpr std::size_t byte_count = 0x00125b24;
private:
    std::array<void*,19> reservations_{};
};
} // namespace bsp::game
