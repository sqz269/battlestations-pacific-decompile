#pragma once

#include "bsp/game_native_readonly_data.hpp"
#include <array>
#include <cstdint>
#include <filesystem>
#include <string>

namespace bsp::game {
class GameNativeMutableCrtData;
class GameNativeCanonicalDataOwner;
enum class GameNativeDataPlan { ReadOnlyV1, CanonicalCrtV2 };
namespace detail {
std::uint32_t native_data_band_mask(const GameNativeDataSpan*, std::size_t);
}
// The only route for adopting fixed native-data bands. This is a process-local,
// move-only ownership capability, created from a one-time inherited handoff.
// Destruction releases only bands verified and transferred to this object.
class GameNativeDataReservation final {
public:
    GameNativeDataReservation() = default;
    ~GameNativeDataReservation() noexcept;
    GameNativeDataReservation(const GameNativeDataReservation&) = delete;
    GameNativeDataReservation& operator=(const GameNativeDataReservation&) = delete;
    GameNativeDataReservation(GameNativeDataReservation&& other) noexcept;
    GameNativeDataReservation& operator=(GameNativeDataReservation&& other) noexcept;

private:
    friend class GameNativeReadOnlyData;
    friend class GameNativeMutableCrtData;
    friend class GameNativeCanonicalDataOwner;
    friend GameNativeDataReservation accept_native_data_handoff(
        const GameNativeDataSpan*, std::size_t, GameNativeDataPlan);
    void transfer_to(std::array<void*,19>& destination, std::uint32_t expected_mask);
    void transfer_ro_subset(std::array<void*,19>& destination, std::uint32_t expected_mask);
    void transfer_mutable_subset(std::array<void*,2>& destination);
    bool finish(bool mapped) noexcept;
    std::array<void*,21> bands_{};
    std::uint32_t mask_ = 0;
    GameNativeDataPlan plan_ = GameNativeDataPlan::ReadOnlyV1;
    void* handoff_view_ = nullptr;
    void* handoff_handle_ = nullptr;
};

// Parses the bootstrap's inherited handle argument, verifies process identity,
// exact requested band set and all 64-KB reservation states, then consumes its
// record exactly once. A malformed or absent record is never adopted.
GameNativeDataReservation accept_native_data_handoff(
    const GameNativeDataSpan* required_spans, std::size_t span_count,
    GameNativeDataPlan plan = GameNativeDataPlan::ReadOnlyV1);

// Owns a newly created suspended child. The child must be our Win32 program
// and must call accept_native_data_handoff before constructing its mapper.
// Destruction before a successful wait_for_mapping terminates this child.
class GameNativeDataBootstrapChild final {
public:
    GameNativeDataBootstrapChild(const std::filesystem::path& child_executable,
        const std::wstring& child_arguments,
        const GameNativeDataSpan* required_spans, std::size_t span_count,
        GameNativeDataPlan plan = GameNativeDataPlan::ReadOnlyV1);
    ~GameNativeDataBootstrapChild() noexcept;
    GameNativeDataBootstrapChild(const GameNativeDataBootstrapChild&) = delete;
    GameNativeDataBootstrapChild& operator=(const GameNativeDataBootstrapChild&) = delete;
    GameNativeDataBootstrapChild(GameNativeDataBootstrapChild&&) = delete;
    GameNativeDataBootstrapChild& operator=(GameNativeDataBootstrapChild&&) = delete;

    // For controlled diagnostics before resume. Do not release allocations
    // made by another owner through this handle.
    void* suspended_process_handle() const noexcept;
    void* process_handle() const noexcept;
    void reserve_and_resume();
    // Returns only after the child mapper reports success. Failure or timeout
    // terminates this child; process teardown reclaims any transferred bands.
    void wait_for_mapping(std::uint32_t timeout_ms);
    std::uint32_t process_id() const noexcept;
private:
    void close() noexcept;
    void fail_child() noexcept;
    void* process_ = nullptr;
    void* thread_ = nullptr;
    void* handoff_view_ = nullptr;
    void* handoff_handle_ = nullptr;
    std::uint32_t process_id_ = 0;
    std::uint32_t mask_ = 0;
    GameNativeDataPlan plan_ = GameNativeDataPlan::ReadOnlyV1;
    bool resumed_ = false;
    bool mapping_confirmed_ = false;
};
} // namespace bsp::game
