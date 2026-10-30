#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
class NativeWideStringSubstringFrame;

// Immutable actual raw8 source/output and SAME retained raw pool cells. Pure
// address/value binding; caller proves reached backing/CRT-copy/alias extents,
// private-storage disjointness and serialized context/provider lifetimes.
class NativeWideStringSubstringContext final {
public:
    NativeWideStringSubstringContext(const void* source, void* output,
        std::int32_t position, std::uint32_t count, NativeStringRawPoolContext&) noexcept;
    NativeWideStringSubstringContext(const NativeWideStringSubstringContext&) = delete;
    NativeWideStringSubstringContext& operator=(const NativeWideStringSubstringContext&) = delete;
    NativeWideStringSubstringContext(NativeWideStringSubstringContext&&) = delete;
    NativeWideStringSubstringContext& operator=(NativeWideStringSubstringContext&&) = delete;
private:
    friend class NativeWideStringSubstringFrame;
    friend void* substring_native_wide_string_header_00770e50(
        const NativeWideStringSubstringContext&, NativeWideStringSubstringFrame&);
    const void* const source_;
    void* const output_;
    const std::int32_t position_;
    const std::uint32_t count_;
    NativeStringRawPoolContext& strings_;
};

struct NativeWideStringSubstringProgress {
    bool attempted{}, temp_length_zeroed{}, temp_data_zeroed{}, temp_edge_armed{};
    bool negative_position{}, initial_length_read{}, source_data_read{}, current_length_read{};
    bool empty_path{}, count_clamped{}, source_start_captured{};
    bool resize_entered{}, resize_returned{}, temp_data_captured{};
    bool crt_copy_entered{}, crt_copy_returned{}, output_copy_entered{}, output_copy_returned{};
    bool output_authority{}, native_output_bit_set{}, native_state0_after_copy{};
    bool temp_edge_consumed_normal{}, temp_footer_null_skip{};
    bool temp_getter_entered{}, temp_getter_returned{}, temp_return_entered{}, temp_return_returned{};
    bool temp_selected_consumed{}, temp_destroy_entered{}, temp_destroy_returned{};
    bool output_selected_consumed{}, output_destroy_entered{}, output_destroy_returned{};
    bool empty_normal_discards_temp_edge{}, normal_transfer{}, normal_completed{}, failed{};
};

// Nonmoving caller-retained private one-shot history. Temp8 is uninitialized
// until actual length0/data0 stores. Default destruction and status inspect own
// metadata only; no Native RAII, late header lookup or cleanup retry occurs.
class NativeWideStringSubstringFrame final {
public:
    explicit NativeWideStringSubstringFrame(const NativeWideStringSubstringContext&) noexcept;
    ~NativeWideStringSubstringFrame() noexcept = default;
    NativeWideStringSubstringFrame(const NativeWideStringSubstringFrame&) = delete;
    NativeWideStringSubstringFrame& operator=(const NativeWideStringSubstringFrame&) = delete;
    NativeWideStringSubstringFrame(NativeWideStringSubstringFrame&&) = delete;
    NativeWideStringSubstringFrame& operator=(NativeWideStringSubstringFrame&&) = delete;
    NativeWideStringSubstringProgress progress() const noexcept { return progress_; }
    bool has_normal_result() const noexcept { return phase_ == Phase::returned && normal_result_; }
    bool requires_process_retention() const noexcept { return phase_ == Phase::active || phase_ == Phase::failed; }
private:
    friend void* substring_native_wide_string_header_00770e50(
        const NativeWideStringSubstringContext&, NativeWideStringSubstringFrame&);
    enum class Phase { fresh, active, returned, failed };
    const NativeWideStringSubstringContext& context_;
    alignas(4) unsigned char temp_[8];
    Phase phase_{Phase::fresh};
    bool temp_owed_{}, output_owed_{}, normal_result_{};
    std::uint32_t position_{}, count_{}, source_data_{}, source_length_{}, source_start_{};
    std::uint32_t temp_data_{}, temp_return_bytes_{};
    void* crt_result_{};
    void* output_copy_result_{};
    NativeWideStringSubstringProgress progress_{};
    void fail() noexcept;
    [[noreturn]] void reject(const char*);
    void require_active();
    void construct_output(bool nonempty_path);
    void unwind_selected() noexcept;
    void complete_and_transfer();
};

// Ordinary Source API, not Native ECX=source/stack(output,position,count)/RET0C.
// Negative position adds its DWORD bits into count then becomes0, not from-end.
// Raw CRT copies count WORDs with padding; no memmove/count repair substitutes.
// Empty also calls genuine RAWcopy. Private constructed authority for added
// Source fences is not a claim of Native flag/private-stack/FH3/hardware parity.
// Caller retains every failed/partial/unpublished graph; no startup/runtime grant.
void* substring_native_wide_string_header_00770e50(
    const NativeWideStringSubstringContext&, NativeWideStringSubstringFrame&);
} // namespace bsp
