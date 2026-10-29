#pragma once
#include "bsp/native_gui_text_default_widget_lifetime.hpp"
#include "bsp/native_string.hpp"
#include <cstdint>
#include <exception>

namespace bsp {
// Caller custody history, not ownership or a TextF4 publication receipt. The
// retained real caller arms these around existing41E870/41DD20. Never infer
// cleanup from entered, and never read header/data after return begins.
struct NativeGuiTextSourceInputProgress final {
    NativeGuiTextSourceInputProgress(const void* header, NativeStringRawPoolContext& pool) noexcept
        : header8(header), strings(pool) {}
    NativeGuiTextSourceInputProgress(const NativeGuiTextSourceInputProgress&) = delete;
    NativeGuiTextSourceInputProgress& operator=(const NativeGuiTextSourceInputProgress&) = delete;
    const void* const header8;
    NativeStringRawPoolContext& strings;
    bool construct_entered{}, construct_returned{}, construct_failed{};
    bool return_entered{}, returned{}, return_failed{};
};

struct NativeGuiTextSourceStorageContext final {
    NativeGuiTextDefaultWidgetLifetimeRecord& text;
    NativeGuiTextDefaultWidgetLifetime& widgets;
    NativeStringRawPoolContext& strings;
};
enum class NativeGuiTextSourceStorageResult { unchanged };
class NativeGuiTextSourceStorageFrame;
struct NativeGuiTextSourceStorageAcquired;
NativeGuiTextSourceStorageResult submit_native_gui_text_source_unchanged_storage_00abaed0(
    NativeGuiTextRawModelReference&, NativeGuiTextSourceStorageContext&,
    NativeGuiTextSourceStorageFrame&, NativeGuiTextSourceStorageAcquired&);

struct NativeGuiTextSourceStorageAcquired final {
    enum class Boundary { none, admission, comparison, unsupported_changed, normal_return };
    struct Diagnostics final {
        Boundary boundary{Boundary::none}, escaped_boundary{Boundary::none};
        bool started{}, comparison_entered{}, comparison_returned{}, equal{}, complete{};
        void* text{}; // address-only after failure/retirement
        const void* input{};
        std::uint32_t native_site{};
        std::exception_ptr failure;
    } diagnostics;
    NativeGuiTextSourceStorageAcquired() = default;
    NativeGuiTextSourceStorageAcquired(const NativeGuiTextSourceStorageAcquired&) = delete;
    NativeGuiTextSourceStorageAcquired& operator=(const NativeGuiTextSourceStorageAcquired&) = delete;
    bool failed() const noexcept { return failed_; }
    bool requires_process_retention() const noexcept { return active_ || failed_; }
    // Historical normal return survives a later rejected tuple/replay. It is
    // NOT current permission: caller must also reject failure/retention.
    bool normal_unchanged_returned() const noexcept { return unchanged_; }
    const NativeGuiTextSourceStorageFrame* bound_frame_identity() const noexcept { return bound_frame_; }
private:
    friend NativeGuiTextSourceStorageResult submit_native_gui_text_source_unchanged_storage_00abaed0(
        NativeGuiTextRawModelReference&, NativeGuiTextSourceStorageContext&,
        NativeGuiTextSourceStorageFrame&, NativeGuiTextSourceStorageAcquired&);
    const NativeGuiTextSourceStorageFrame* bound_frame_{}; // never followed
    bool active_{}, failed_{}, unchanged_{};
};

// Synchronous read-only fragment, installed over stable actual caller metadata.
// Constructor/getters/status/destructor inspect this object's metadata ONLY.
// Fresh or completed destruction performs no native/header/ref/provider action;
// active/failed destruction terminates rather than silently dropping custody.
class NativeGuiTextSourceStorageFrame final {
public:
    enum class Phase { fresh, entered, unchanged, failed };
    NativeGuiTextSourceStorageFrame(const NativeGuiTextSourceStorageContext&,
        const void* input8, const NativeGuiTextSourceInputProgress&,
        const NativeGuiTextReferenceFrame&, NativeGuiTextSourceStorageAcquired&) noexcept;
    ~NativeGuiTextSourceStorageFrame() noexcept;
    NativeGuiTextSourceStorageFrame(const NativeGuiTextSourceStorageFrame&) = delete;
    NativeGuiTextSourceStorageFrame& operator=(const NativeGuiTextSourceStorageFrame&) = delete;
    bool matches(const NativeGuiTextSourceStorageContext&, const void* input8,
        const NativeGuiTextSourceInputProgress&, const NativeGuiTextReferenceFrame&,
        const NativeGuiTextSourceStorageAcquired&, Phase) const noexcept;
    const NativeGuiTextSourceStorageContext* prepared_context() const noexcept { return context_; }
    const void* prepared_input() const noexcept { return input_; }
    const NativeGuiTextSourceInputProgress* prepared_input_progress() const noexcept { return input_progress_; }
    const NativeGuiTextReferenceFrame* prepared_reference_frame() const noexcept { return reference_frame_; }
    const NativeGuiTextSourceStorageAcquired* prepared_acquired() const noexcept { return acquired_; }
    Phase phase() const noexcept { return phase_; }
    bool entered() const noexcept { return entered_; }
    bool comparison_entered() const noexcept { return comparison_entered_; }
    bool comparison_returned() const noexcept { return comparison_returned_; }
    bool normal_unchanged_returned() const noexcept { return unchanged_; }
    bool requires_process_retention() const noexcept { return active_ || phase_ == Phase::entered || phase_ == Phase::failed; }
    const std::exception_ptr& failure() const noexcept { return failure_; }
private:
    friend NativeGuiTextSourceStorageResult submit_native_gui_text_source_unchanged_storage_00abaed0(
        NativeGuiTextRawModelReference&, NativeGuiTextSourceStorageContext&,
        NativeGuiTextSourceStorageFrame&, NativeGuiTextSourceStorageAcquired&);
    const NativeGuiTextSourceStorageContext* const context_;
    const void* const input_;
    const NativeGuiTextSourceInputProgress* const input_progress_;
    const NativeGuiTextReferenceFrame* const reference_frame_;
    NativeGuiTextSourceStorageAcquired* const acquired_;
    Phase phase_{Phase::fresh};
    bool active_{};
    bool entered_{}, comparison_entered_{}, comparison_returned_{}, unchanged_{};
    std::exception_ptr failure_;
};

// Initial domain is constructor-owned current TextF4/F8==0. Perform ONE existing
// 00435C40 and return only for its true ABAF01->AFE5 branch. No localise argument
// is needed: the native branch never reads it. False comparison poisons/throws
// BEFORE AF07; no changed/localised/content/current50 or native ABI is supplied.
//
// Caller supplies genuine live separate C++ records and actual input8B/storage,
// with real construct-returned/not-yet-returning history. Concrete extents are
// checked, but opaque provider allocations and the retained scalar flags cell
// still require caller disjointness/lifetime proof. No pointer or ownership is
// published into raw122/retirement. Complete real input return before scalar;
// retain on Frame OR Acquired OR caller-input failure/pending custody.
} // namespace bsp
