#pragma once
#include "bsp/native_camera_reference.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>

namespace bsp {
struct NativeCockpitViewportReleaseContext;

// Stable caller-owned host storage, not the actual 24h helper or a native owner.
// Prepare its persistent caller ownership before any native constructor event.
// Surviving cameras/viewports and inert semantic-view borrows outlive the attempt
// and may outlive the helper. No block operation adds a native retain or release.
class NativeCockpitConstructionBlock final {
public:
    enum class Phase { idle, preparing, prepared, executing, settled };

    class Admission final {
    public:
        Admission() noexcept = default;
        Admission(const Admission&) = delete;
        Admission& operator=(const Admission&) = delete;
        Admission(Admission&&) noexcept;
        Admission& operator=(Admission&&) noexcept;
        ~Admission() noexcept;
        // Unused preparation rolls back to idle. Executing settlement cancels
        // only unused credits and preserves companions and all record states.
        void cancel() noexcept;
        explicit operator bool() const noexcept { return block_ != nullptr; }
    private:
        friend class NativeCockpitConstructionBlock;
        friend void* construct_native_cockpit_helper_00b3c800(void*, std::size_t,
            const volatile std::uint32_t&, const volatile std::uint32_t&,
            const NativeCockpitViewportReleaseContext&, Admission&&);
        Admission(NativeCockpitConstructionBlock&, NativeCameraEnvironment&,
            const NativeNodeRawConstants&) noexcept;
        // Constructor friend validates before native helper stores or moving the
        // caller token, then moves it locally and begins execution once.
        void validate_prepared() const;
        void begin_execution() noexcept;
        NativeCockpitConstructionBlock* block_{};
        NativeCameraEnvironment* camera_environment_{};
        const NativeNodeRawConstants* node_constants_{};
        SceneAttachmentRuntime::BindingAdmission scene_admission_;
        GeneratedModelLifetimeRuntime::BindingAdmission lifetime_admission_;
        NativeViewportRegistry::Admission viewport_admissions_[2]; // first, replacement
    };

    NativeCockpitConstructionBlock() noexcept = default;
    ~NativeCockpitConstructionBlock() noexcept;
    NativeCockpitConstructionBlock(const NativeCockpitConstructionBlock&) = delete;
    NativeCockpitConstructionBlock& operator=(const NativeCockpitConstructionBlock&) = delete;
    NativeCockpitConstructionBlock(NativeCockpitConstructionBlock&&) = delete;
    NativeCockpitConstructionBlock& operator=(NativeCockpitConstructionBlock&&) = delete;
    // May grow node vectors; all four reservations precede native events.
    // Borrows the already-installed registry; never installs another domain.
    [[nodiscard]] Admission admit(NativeCameraEnvironment&,
        const NativeNodeRawConstants&, NativeViewportRegistry&);
    Phase phase() const noexcept { return phase_; }
    // Caller proves every semantic frame/cache/view borrow has ended. Requires
    // a settled attempt, absent/retired camera reference, absent/dead owner and
    // no live/reserved viewport record. This performs no native cleanup.
    void reset_after_host_quiescence() noexcept;

private:
    friend void* construct_native_cockpit_helper_00b3c800(void*, std::size_t,
        const volatile std::uint32_t&, const volatile std::uint32_t&,
        const NativeCockpitViewportReleaseContext&, Admission&&);
    static void validate_environment(NativeCameraEnvironment&,
        const NativeNodeRawConstants&, NativeViewportRegistry&);
    static void record_camera_retirement(void*, NativeCameraReference&) noexcept;
    NativeViewportRegistry::Storage viewport_records_[2];
    std::optional<NativeCameraOwner> camera_owner_;
    std::optional<NativeCameraReference> camera_reference_;
    NativeViewportRegistry* viewport_registry_{};
    Phase phase_{Phase::idle};
    bool camera_retired_{};
};

// Declaration only; the separate B3C800 body consumes the block's exact private
// admissions. New C++ interface: original ECX=actual helper, EAX=same, RET0.
// Read actual near/far cells at the native x87 sites, not during host preparation.
void* construct_native_cockpit_helper_00b3c800(void* actual_helper,
    std::size_t helper_bytes, const volatile std::uint32_t& near_00d7a2f0,
    const volatile std::uint32_t& far_00ce38b8,
    const NativeCockpitViewportReleaseContext& viewport_release,
    NativeCockpitConstructionBlock::Admission&&);

} // namespace bsp
