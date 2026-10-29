#pragma once
#include "bsp/native_gui_text_main_section_lifetime_storage.hpp"
#include "bsp/native_gui_text_font_lifetime_storage.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <exception>

namespace bsp {
// Caller custody around actual4C5E60/436430, not header publication/ownership.
// Bind before effects; the real caller arms entered BEFORE each helper and
// returned/failed afterward. Never inspect the header once return has entered.
struct NativeGuiTextEmptyUnchangedInputProgress final {
    NativeGuiTextEmptyUnchangedInputProgress(const void* header,
        ActualNativeStringPoolStorage& actual_strings, NativeStringRawPoolContext& raw) noexcept
        : header8(header), strings(actual_strings), raw_strings(raw) {}
    NativeGuiTextEmptyUnchangedInputProgress(const NativeGuiTextEmptyUnchangedInputProgress&) = delete;
    NativeGuiTextEmptyUnchangedInputProgress& operator=(const NativeGuiTextEmptyUnchangedInputProgress&) = delete;
    const void* const header8;
    ActualNativeStringPoolStorage& strings;
    NativeStringRawPoolContext& raw_strings;
    bool construct_entered{}, construct_returned{}, construct_failed{};
    bool return_entered{}, returned{}, return_failed{};
};

// Whole nonmoving graph is installed before Text allocation/canonical122.
// Existing Font admission is mutable; this reference's identity is fixed and
// must equal the SAME immutable operation/retirement peer. No const_cast.
struct NativeGuiTextEmptyUnchangedStorageContext final {
    const NativeGuiTextPrimaryPrefixLifetimeContext& prefix;
    const NativeGuiTextMainSectionLifetimeStorageFrame& main;
    NativeGuiTextFontLifetimeStorageFrame& font;
    NativeGuiTextMainSectionStorageContext& construction;
    NativeGuiTextMainSectionStorageFrame& ensure_frame;
    NativeGuiTextMainSectionStorageAcquired& ensure_acquired;
    ActualNativeStringPoolStorage& strings;
    NativeStringRawPoolContext& raw_strings;
};

enum class NativeGuiTextEmptyUnchangedStorageResult { unchanged_empty };
enum class NativeGuiTextEmptyUnchangedStorageStage : std::uint8_t {
    none, admission, ensure, copy, font, comparison, capture, disarmed,
    getter, returning, normal_return
};
class NativeGuiTextEmptyUnchangedStorageFrame;
struct NativeGuiTextEmptyUnchangedStorageAcquired;
NativeGuiTextEmptyUnchangedStorageResult submit_native_gui_text_empty_unchanged_storage_00aba8d0(
    NativeGuiTextRawModelReference&, const NativeGuiTextEmptyUnchangedStorageContext&,
    NativeGuiTextEmptyUnchangedStorageFrame&, NativeGuiTextEmptyUnchangedStorageAcquired&);

struct NativeGuiTextEmptyUnchangedStorageAcquired final {
    using Stage = NativeGuiTextEmptyUnchangedStorageStage;
    struct Diagnostics final {
        Stage boundary{Stage::none}, escaped_boundary{Stage::none};
        std::uint32_t native_site{};
        const void* text{};
        const void* input{};
        const void* captured_data{}; // history ONLY after cleanup/failure
        std::uint32_t captured_size{};
        bool started{}, ensure_entered{}, ensure_returned{}, copy_entered{}, copy_returned{};
        bool font_read{}, font_flag_read{}, text_length_read{}, temporary_length_read{};
        bool data_captured{}, disarmed{}, getter_entered{}, getter_returned{};
        bool return_entered{}, return_returned{}, normal_null_cleanup{}, complete{};
        std::exception_ptr failure;
    } diagnostics;
    NativeGuiTextEmptyUnchangedStorageAcquired() = default;
    NativeGuiTextEmptyUnchangedStorageAcquired(const NativeGuiTextEmptyUnchangedStorageAcquired&) = delete;
    NativeGuiTextEmptyUnchangedStorageAcquired& operator=(const NativeGuiTextEmptyUnchangedStorageAcquired&) = delete;
    bool failed() const noexcept { return failed_; }
    bool requires_process_retention() const noexcept { return active_ || failed_; }
    bool normal_empty_returned() const noexcept { return completed_; } // historical, not current permission
    const NativeGuiTextEmptyUnchangedStorageFrame* bound_frame_identity() const noexcept { return bound_frame_; }
private:
    friend class NativeGuiTextEmptyUnchangedStorageFrame;
    friend NativeGuiTextEmptyUnchangedStorageResult submit_native_gui_text_empty_unchanged_storage_00aba8d0(
        NativeGuiTextRawModelReference&, const NativeGuiTextEmptyUnchangedStorageContext&,
        NativeGuiTextEmptyUnchangedStorageFrame&, NativeGuiTextEmptyUnchangedStorageAcquired&);
    const NativeGuiTextEmptyUnchangedStorageFrame* bound_frame_{}; // never followed
    bool active_{}, failed_{}, completed_{};
};

struct NativeGuiTextEmptyUnchangedStorageRegion final {
    const void* begin{};
    std::size_t bytes{};
};
struct NativeGuiTextEmptyUnchangedStorageRegions final {
    // Fixed concrete schema: own metadata plus existing Main16/Font48 live
    // views and native/input/temp extents. Never truncate or infer native caps.
    std::array<NativeGuiTextEmptyUnchangedStorageRegion, 128> entries{};
    std::size_t count{};
};

class NativeGuiTextEmptyUnchangedStorageFrame final {
public:
    enum class Phase { fresh, entered, unchanged, failed };
    enum class Cleanup { none, captured, normal_null, returned };
    using Stage = NativeGuiTextEmptyUnchangedStorageStage;
    NativeGuiTextEmptyUnchangedStorageFrame(const NativeGuiTextEmptyUnchangedStorageContext&,
        const void* actual_input8, const NativeGuiTextEmptyUnchangedInputProgress&,
        const NativeGuiTextReferenceFrame&, NativeGuiTextEmptyUnchangedStorageAcquired&) noexcept;
    ~NativeGuiTextEmptyUnchangedStorageFrame() noexcept;
    NativeGuiTextEmptyUnchangedStorageFrame(const NativeGuiTextEmptyUnchangedStorageFrame&) = delete;
    NativeGuiTextEmptyUnchangedStorageFrame& operator=(const NativeGuiTextEmptyUnchangedStorageFrame&) = delete;
    NativeGuiTextEmptyUnchangedStorageFrame(NativeGuiTextEmptyUnchangedStorageFrame&&) = delete;
    NativeGuiTextEmptyUnchangedStorageFrame& operator=(NativeGuiTextEmptyUnchangedStorageFrame&&) = delete;
    bool matches(const NativeGuiTextEmptyUnchangedStorageContext&, const void* input8,
        const NativeGuiTextEmptyUnchangedInputProgress&, const NativeGuiTextReferenceFrame&,
        const NativeGuiTextEmptyUnchangedStorageAcquired&, Phase) const noexcept;
    Phase phase() const noexcept { return phase_; }
    Stage lexical_stage() const noexcept { return stage_; }
    bool active_lexical_stage() const noexcept { return active_ && phase_ == Phase::entered && !failure_; }
    Cleanup cleanup() const noexcept { return cleanup_; }
    bool normal_empty_returned() const noexcept { return normal_; } // historical after later rejection
    bool completed_without_temp_debt() const noexcept;
    bool requires_process_retention() const noexcept { return active_ || phase_ == Phase::failed || bool(failure_); }
    const NativeGuiTextEmptyUnchangedStorageContext& prepared_context() const noexcept { return *context_; }
    const void* input_header() const noexcept { return input_; }
    const NativeGuiTextEmptyUnchangedInputProgress& input_progress() const noexcept { return *input_progress_; }
    const NativeGuiTextReferenceFrame& reference_frame() const noexcept { return *reference_frame_; }
    const NativeGuiTextEmptyUnchangedStorageAcquired* acquired_identity() const noexcept { return acquired_; }
    const void* temporary_header_extent() const noexcept { return temporary_; }
    const std::exception_ptr& failure() const noexcept { return failure_; }
    // Pure NEW-leaf roles only. Does not call peer admission or follow native
    // headers; valid at fresh pre122/active Ensure and after native lifetimes.
    // Frame contains temporary8; expectedReferenceFrame is an intentional
    // immutable constructor-bound role, never a generic address exception.
    NativeGuiTextEmptyUnchangedStorageRegions metadata_regions() const noexcept;
    // Independent BOUND live admission, NOT a shared recursive borrowing hook.
    // Dynamic header backing only after private issuance and before cleanup;
    // never inspect an unissued/return-entered/ended header to find regions.
    NativeGuiTextEmptyUnchangedStorageRegions live_regions(NativeGuiTextRawModelReference&) const;
    // Metadata ONLY: untouched fresh Ensure/input OR private normal completion
    // plus settled Temp AND actual caller input returned. Const rejection
    // privately poisons even if caught; no native/companion/peer-live reads.
    void require_retirement_quiescent() const;
private:
    friend NativeGuiTextEmptyUnchangedStorageResult submit_native_gui_text_empty_unchanged_storage_00aba8d0(
        NativeGuiTextRawModelReference&, const NativeGuiTextEmptyUnchangedStorageContext&,
        NativeGuiTextEmptyUnchangedStorageFrame&, NativeGuiTextEmptyUnchangedStorageAcquired&);
    const NativeGuiTextEmptyUnchangedStorageContext* const context_;
    const void* const input_;
    const NativeGuiTextEmptyUnchangedInputProgress* const input_progress_;
    const NativeGuiTextReferenceFrame* const reference_frame_;
    NativeGuiTextEmptyUnchangedStorageAcquired* const acquired_;
    alignas(4) std::byte temporary_[8]; // ONLY actual4C8DD0 initializes this header
    mutable Phase phase_{Phase::fresh};
    mutable Stage stage_{Stage::none};
    Cleanup cleanup_{Cleanup::none};
    mutable bool active_{};
    bool entered_{}, ensure_entered_{}, ensure_returned_{}, copy_entered_{}, copy_returned_{};
    bool compared_empty_{}, disarmed_{}, getter_entered_{}, getter_returned_{}, return_entered_{}, return_returned_{};
    bool normal_{};
    const void* captured_data_{}; // never followed after cleanup/failure
    std::uint32_t captured_size_{};
    mutable std::exception_ptr failure_;
    mutable bool acquired_admitted_{};
    void poison(std::exception_ptr) const noexcept;
    void require_fixed_tuple() const;
    void* require_bound(NativeGuiTextRawModelReference&) const;
    void require_record_disjoint(const NativeGuiTextEmptyUnchangedStorageAcquired&) const;
};
} // namespace bsp
