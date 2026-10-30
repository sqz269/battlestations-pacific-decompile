#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeStringPoolStorage;
class NativeLuaLogRawFrame;

// Immutable actual RO18/raw-cell/current-feature roles. Metadata validation
// reads no literal, pool or feature payload and supplies no mapping/provenance.
class NativeLuaLogRawContext final {
public:
    NativeLuaLogRawContext(NativeStringRawPoolContext&,
        const volatile std::uint32_t& actual_feature_0109eea4,
        const void* actual_prefix_00d112d4, std::uint32_t prefix_bytes);
    NativeLuaLogRawContext(const NativeLuaLogRawContext&) = delete;
    NativeLuaLogRawContext& operator=(const NativeLuaLogRawContext&) = delete;
    NativeLuaLogRawContext(NativeLuaLogRawContext&&) = delete;
    NativeLuaLogRawContext& operator=(NativeLuaLogRawContext&&) = delete;
private:
    friend std::int32_t execute_native_lua_log_0088bec0(
        const NativeLuaLogRawContext&, NativeLuaLogRawFrame&);
    NativeStringRawPoolContext& strings_;
    const volatile std::uint32_t& feature_;
    const void* const prefix_;
    const std::uint32_t prefix_bytes_;
};

// Copied diagnostics grant no cleanup, pointer-lifetime or normal authority.
struct NativeLuaLogRawProgress {
    bool attempted{}, failed{}, completed{};
    bool resize_entered{}, resize_returned{}, data_captured{}, null_data{};
    bool length_captured{}, copy_entered{}, copy_returned{};
    bool getter_entered{}, getter_returned{};
    bool block_return_entered{}, block_return_returned{};
    std::uintptr_t captured_data{}, copy_result{}, pool_address{};
    std::uint32_t captured_bytes{};
};

// Retain this nonmoving one-shot attempt and all provider/partial-buffer debt
// after failure. Scratch8 is uninitialized until actual entry stores. No native
// destructor, selected action, return, retry or late payload read is supplied.
class NativeLuaLogRawFrame final {
public:
    explicit NativeLuaLogRawFrame(const NativeLuaLogRawContext&) noexcept;
    ~NativeLuaLogRawFrame() = default;
    NativeLuaLogRawFrame(const NativeLuaLogRawFrame&) = delete;
    NativeLuaLogRawFrame& operator=(const NativeLuaLogRawFrame&) = delete;
    NativeLuaLogRawFrame(NativeLuaLogRawFrame&&) = delete;
    NativeLuaLogRawFrame& operator=(NativeLuaLogRawFrame&&) = delete;
    NativeLuaLogRawProgress progress() const noexcept { return progress_; }
    bool has_normal_result() const noexcept {
        return phase_ == Phase::returned && !failed_ && normal_result_;
    }
    bool requires_process_retention() const noexcept {
        return phase_ == Phase::active || phase_ == Phase::failed;
    }
private:
    friend std::int32_t execute_native_lua_log_0088bec0(
        const NativeLuaLogRawContext&, NativeLuaLogRawFrame&);
    enum class Phase { fresh, active, returned, failed };
    enum class BufferDisposition { unobserved, null_data, live, return_entered, returned };
    const NativeLuaLogRawContext& context_;
    alignas(4) unsigned char scratch_[8];
    Phase phase_{Phase::fresh};
    BufferDisposition disposition_{BufferDisposition::unobserved};
    bool failed_{}, normal_result_{};
    std::uint32_t data_{}, bytes_{};
    void* copy_result_{};
    NativeStringPoolStorage* pool_{};
    NativeLuaLogRawProgress progress_{};
    void fail() noexcept;
    [[noreturn]] void reject(const char*);
    void require_active();
    void complete();
};

// Whole31-instruction normal body, ordinary Source API only. Original does
// not read a Lua state/argument; this is NOT a lua_CFunction or table binding.
// Same real raw resize/copy/getter/return effects precede actual return0.
// Added private healthy/reentry fences do not model physical-stack/FH3/fault
// parity; Native has NO local cleanup. Caller retains actual backing, code,
// cells/current feature and failures. Late status/default dtor are metadata only.
std::int32_t execute_native_lua_log_0088bec0(
    const NativeLuaLogRawContext&, NativeLuaLogRawFrame&);
} // namespace bsp
