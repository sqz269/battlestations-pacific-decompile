#pragma once

#include "bsp/native_lua_objects.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

enum class NativeLuaDotPathLiteralRole { dot, zero };
struct NativeLuaDotPathLiteralSpan {
    const void* data;
    std::uintptr_t original_address;
    std::size_t bytes;
    NativeLuaDotPathLiteralRole role;
};

class NativeLuaDotPathFrame;

// Actual stable root14/fresh output14/borrowed path8/status-byte tuple. Context
// construction checks metadata only, not payload/readiness. Actual CE3A70 '.',
// CE43BC '0' and MUTABLE0108FF2C backing are caller obligations. A private Game
// empty byte or semantic string is not a substitute for that mutable address.
class NativeLuaDotPathContext final {
public:
    NativeLuaDotPathContext(NativeLuaObjectStorage& actual_root14,
        void* fresh_output14, const void* borrowed_path8,
        std::uint8_t* actual_status_byte, NativeStringRawPoolContext&,
        NativeLuaDotPathLiteralSpan actual_dot,
        NativeLuaDotPathLiteralSpan actual_zero,
        char* actual_mutable_empty_0108ff2c);
    NativeLuaDotPathContext(const NativeLuaDotPathContext&) = delete;
    NativeLuaDotPathContext& operator=(const NativeLuaDotPathContext&) = delete;
    NativeLuaDotPathContext(NativeLuaDotPathContext&&) = delete;
    NativeLuaDotPathContext& operator=(NativeLuaDotPathContext&&) = delete;
private:
    friend class NativeLuaDotPathFrame;
    friend NativeLuaObjectStorage* resolve_native_lua_dot_path_00b68d70(
        const NativeLuaDotPathContext&, NativeLuaDotPathFrame&);
    NativeLuaObjectStorage& root_;
    void* const output_;
    const void* const path_;
    std::uint8_t* const status_;
    NativeStringRawPoolContext& strings_;
    const NativeLuaDotPathLiteralSpan dot_;
    const NativeLuaDotPathLiteralSpan zero_;
    char* const empty_;
};

// Diagnostics only. Private phase/action authority, not these copied fields,
// controls completion and selected cleanup. Unsigned counters may wrap.
struct NativeLuaDotPathProgress {
    bool attempted{}, failed{}, completed{};
    bool output_copy_entered{}, output_copy_returned{};
    bool initial_rest_assigned{}, status_one_written{}, status_zero_written{};
    std::int32_t selected_state{-1};
    std::uint32_t iterations{}, resize_entered{}, resize_returned{};
    std::uint32_t substring_entered{}, substring_returned{};
    std::uint32_t scan_entered{}, scan_returned{};
    std::uint32_t string_lookup_entered{}, string_lookup_returned{};
    std::uint32_t index_lookup_entered{}, index_lookup_returned{};
    std::uint32_t assignment_entered{}, assignment_returned{};
    std::uint32_t normal_lua_release_entered{}, normal_lua_release_returned{};
    std::uint32_t getter_entered{}, getter_returned{};
    std::uint32_t block_return_entered{}, block_return_returned{};
    std::uint32_t selected_actions_entered{}, selected_actions_returned{};
};

// Caller retains this nonmoving Frame and the actual interpreter/tracking,
// partial string/output and provider graph after failure. Its native bytes are
// uninitialized until the real entry/provider epochs. No destructor cleanup.
class NativeLuaDotPathFrame final {
public:
    explicit NativeLuaDotPathFrame(const NativeLuaDotPathContext&) noexcept;
    ~NativeLuaDotPathFrame() = default;
    NativeLuaDotPathFrame(const NativeLuaDotPathFrame&) = delete;
    NativeLuaDotPathFrame& operator=(const NativeLuaDotPathFrame&) = delete;
    NativeLuaDotPathFrame(NativeLuaDotPathFrame&&) = delete;
    NativeLuaDotPathFrame& operator=(NativeLuaDotPathFrame&&) = delete;
    NativeLuaDotPathProgress progress() const noexcept { return progress_; }
    // Status0 is also a normal result; this receipt does not assert success.
    bool has_normal_result() const noexcept {
        return phase_ == Phase::returned && normal_result_;
    }
    bool requires_process_retention() const noexcept {
        return phase_ == Phase::active || phase_ == Phase::failed;
    }
private:
    friend NativeLuaObjectStorage* resolve_native_lua_dot_path_00b68d70(
        const NativeLuaDotPathContext&, NativeLuaDotPathFrame&);
    enum class Phase { fresh, active, returned, failed };
    const NativeLuaDotPathContext& context_;
    alignas(4) unsigned char rest_[8], segment_[8], prefix_[8], suffix_[8];
    alignas(NativeLuaObjectStorage) unsigned char lookup_bytes_[sizeof(NativeLuaObjectStorage)];
    alignas(NativeLuaObjectStorage) unsigned char index_bytes_[sizeof(NativeLuaObjectStorage)];
    Phase phase_{Phase::fresh};
    bool failed_{}, output_action_armed_{}, normal_result_{};
    std::int32_t state_{-1};
    NativeLuaObjectStorage* output_{};
    NativeLuaObjectStorage* lookup_{};
    NativeLuaObjectStorage* index_{};
    NativeLuaObjectStorage* last_assignment_{};
    void* last_substring_{};
    NativeStringPoolStorage* last_pool_{};
    std::uint32_t rest_length_{}, rest_data_{}, segment_data_{}, split_index_{};
    std::int32_t numeric_index_{};
    NativeLuaDotPathProgress progress_{};
    void fail() noexcept;
    [[noreturn]] void reject(const char*);
    void require_active();
    void set_state(std::int32_t) noexcept;
    void write_status(std::uint8_t) noexcept;
    void resize(void*, std::uint32_t);
    void assign_inline(void*, const void*, std::uint32_t& saved_destination_data);
    void return_captured(std::uint32_t data, std::uint32_t bytes);
    void normal_lua_release(NativeLuaObjectStorage&);
    void unwind_selected() noexcept;
    void complete_and_transfer();
};

// Original ECX=root14, hidden result14/path8/status pointer, EAX=result/RET0C.
// Ordinary Source entry only. Borrowed input is not consumed. No VM/globals,
// pcall/error repair, loading or startup route is provided. Valid raw/typed
// backing, fresh/disjoint output+Frame, real mapped roles/cells and serialized
// no-invalidating-writer/callback custody are caller contracts. Lua longjmp,
// private FH3/SEH/hardware/stack-home behavior is not C++ unwinding equivalence.
NativeLuaObjectStorage* resolve_native_lua_dot_path_00b68d70(
    const NativeLuaDotPathContext&, NativeLuaDotPathFrame&);

} // namespace bsp
