#pragma once

#include "bsp/game_native_readonly_data.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>

namespace bsp::game {
class GameNativeDataReservation;
enum class GameNativeDataPlan;

// Layouts of the four recovered cells only. The remainder of each committed
// page is image data, not an initialized host service or callable CRT state.
struct NativeCrtNlgDescriptor {
    std::uint32_t signature, destination, code, frame;
};
struct NativeCrtFailureBlock {
    std::array<std::byte,0x50> record;
    std::uint32_t debugger;
    std::uint32_t padding;
    std::array<std::byte,0x2cc> context;
};
struct NativeCrtExceptionPair {
    std::uint32_t record, context;
};
static_assert(sizeof(NativeCrtNlgDescriptor)==0x10);
static_assert(sizeof(NativeCrtFailureBlock)==0x324);
static_assert(sizeof(NativeCrtExceptionPair)==8);

class GameNativeMutableCrtData final {
public:
    ~GameNativeMutableCrtData() noexcept;
    GameNativeMutableCrtData(const GameNativeMutableCrtData&) = delete;
    GameNativeMutableCrtData& operator=(const GameNativeMutableCrtData&) = delete;

    volatile std::uint32_t& cookie() const noexcept;
    volatile std::uint32_t& complement() const noexcept;
    NativeCrtNlgDescriptor& nlg_descriptor() const noexcept;
    NativeCrtFailureBlock& failure_block() const noexcept;
    volatile std::uint32_t& feature_word() const noexcept;
    volatile std::uint32_t& debugger_hook() const noexcept;
    // Explicit V3 storage borrow only. Taking this reference reads no cell value.
    // V1/V2 have no locale-control role and receive no fallback reference.
    const volatile std::int32_t& locale_changed_0109de1c() const;
private:
    friend class GameNativeCanonicalDataOwner;
    GameNativeMutableCrtData(const std::filesystem::path&,
        GameNativeDataReservation&);
    void initialize(const std::filesystem::path&);
    void verify_pages() const;
    std::array<void*,2> reservations_{};
    const GameNativeDataPlan plan_;
};

// Unique, intentionally non-destructed process owner. initialize() consumes a
// declared canonical capability once and publishes only after checked readiness ACK.
// Borrowed references remain valid through CRT/atexit/thread teardown; the OS
// reclaims the owned RO bands and mutable reservations at process exit;
// V2 has three writable pages, while explicit V3 adds the locale-control page.
// No locale setter is supplied.
class GameNativeCanonicalDataOwner final {
public:
    static const GameNativeCanonicalDataOwner& initialize(
        const std::filesystem::path& original_executable,
        const GameNativeDataSpan* readonly_spans, std::size_t span_count,
        GameNativeDataReservation&& reservation);
    GameNativeCanonicalDataOwner(const GameNativeCanonicalDataOwner&) = delete;
    GameNativeCanonicalDataOwner& operator=(const GameNativeCanonicalDataOwner&) = delete;
    GameNativeReadOnlyData& read_only_data() const noexcept { return *readonly_; }
    GameNativeMutableCrtData& mutable_crt_data() const noexcept { return *mutable_; }
    const NativeCrtExceptionPair& exception_pair() const;
private:
    GameNativeCanonicalDataOwner(const std::filesystem::path&,
        const GameNativeDataSpan*, std::size_t, GameNativeDataReservation&);
    ~GameNativeCanonicalDataOwner() = default;
    struct RollbackDelete {
        void operator()(GameNativeCanonicalDataOwner*) const noexcept;
    };
    void verify_joint() const;
    std::unique_ptr<GameNativeReadOnlyData> readonly_;
    std::unique_ptr<GameNativeMutableCrtData> mutable_;
};
} // namespace bsp::game
