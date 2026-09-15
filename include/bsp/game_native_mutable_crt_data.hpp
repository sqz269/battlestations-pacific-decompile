#pragma once

#include "bsp/game_native_readonly_data.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>

namespace bsp::game {
class GameNativeDataReservation;

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
private:
    friend class GameNativeCanonicalDataOwner;
    GameNativeMutableCrtData(const std::filesystem::path&,
        GameNativeDataReservation&);
    void initialize(const std::filesystem::path&);
    void verify_pages() const;
    std::array<void*,2> reservations_{};
};

// Unique, intentionally non-destructed process owner. initialize() consumes a
// v2 inherited capability once and publishes only after checked parent ACK.
// Borrowed references remain valid through CRT/atexit/thread teardown; the OS
// reclaims both the RO D6 pair and the three writable pages at process exit.
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
