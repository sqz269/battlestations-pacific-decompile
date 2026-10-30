#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
class NativeWideStringConcatFrame;

// Bind only actual raw8 identities and SAME retained AA8/AA4/AA0 pool context.
// Constructor is address-only. Caller proves reached backing/copy/alias extents
// and keeps metadata/control/provider lifetimes disjoint and serialized.
class NativeWideStringConcatContext final {
public:
    NativeWideStringConcatContext(void* output, const void* left, const void* right,
        NativeStringRawPoolContext&) noexcept;
    NativeWideStringConcatContext(const NativeWideStringConcatContext&) = delete;
    NativeWideStringConcatContext& operator=(const NativeWideStringConcatContext&) = delete;
    NativeWideStringConcatContext(NativeWideStringConcatContext&&) = delete;
    NativeWideStringConcatContext& operator=(NativeWideStringConcatContext&&) = delete;
private:
    friend class NativeWideStringConcatFrame;
    friend void* concatenate_native_wide_string_header_005cf570(
        const NativeWideStringConcatContext&, NativeWideStringConcatFrame&);
    void* const output_;
    const void* const left_;
    const void* const right_;
    NativeStringRawPoolContext& strings_;
};

struct NativeWideStringConcatProgress {
    bool attempted{}, copy_entered{}, copy_returned{}, constructed_authority{};
    bool right_length_read{}, native_arm_epoch_passed{}, old_output_length_read{};
    bool resize_entered{}, resize_returned{}, right_data_read{}, output_data_read{};
    bool append_copy_entered{}, append_copy_returned{}, zero_copy_omitted{};
    bool normal_transfer{}, selected_edge_consumed{};
    bool selected_destroy_entered{}, selected_destroy_returned{};
    bool normal_completed{}, failed{};
};

// Caller-retained private one-shot authority. Default destruction/status are
// own-metadata only, never native RAII cleanup or late header/provider lookup.
class NativeWideStringConcatFrame final {
public:
    explicit NativeWideStringConcatFrame(const NativeWideStringConcatContext&) noexcept;
    ~NativeWideStringConcatFrame() noexcept = default;
    NativeWideStringConcatFrame(const NativeWideStringConcatFrame&) = delete;
    NativeWideStringConcatFrame& operator=(const NativeWideStringConcatFrame&) = delete;
    NativeWideStringConcatFrame(NativeWideStringConcatFrame&&) = delete;
    NativeWideStringConcatFrame& operator=(NativeWideStringConcatFrame&&) = delete;
    NativeWideStringConcatProgress progress() const noexcept { return progress_; }
    bool has_normal_result() const noexcept { return phase_ == Phase::returned && normal_result_; }
private:
    friend void* concatenate_native_wide_string_header_005cf570(
        const NativeWideStringConcatContext&, NativeWideStringConcatFrame&);
    enum class Phase { fresh, active, returned, failed };
    const NativeWideStringConcatContext& context_;
    Phase phase_{Phase::fresh};
    bool output_owed_{}; // Source authority after actual copy NORMAL return
    bool normal_result_{};
    void* copy_result_{};
    std::uint32_t right_length_{};
    std::uint32_t old_output_length_{};
    NativeWideStringConcatProgress progress_{};
    void fail() noexcept;
    [[noreturn]] void reject(const char*);
    void require_active();
    void unwind_selected() noexcept;
    void complete_and_transfer();
};

// Ordinary Source API: actual RAWcopy then direct concat epochs, NOT005CF090.
// Private output authority precedes added Source healthy fences; Native's
// physical bit1 follows its right-length load. Those are not identical stack/
// FH3/SEH/hardware epochs. Constructor escape is unowned; armed unwind consumes
// before CURRENT436430; normal result transfers without destruction. Caller
// retains failed/partial/unpublished graphs. No loading/startup/runtime grant.
void* concatenate_native_wide_string_header_005cf570(
    const NativeWideStringConcatContext&, NativeWideStringConcatFrame&);
} // namespace bsp
