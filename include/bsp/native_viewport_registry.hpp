#pragma once

#include "bsp/camera_frame_state.hpp"

#include <cstdint>
#include <optional>

namespace bsp {

// Host identities only. One installed domain is shared by all managed viewport
// producers. Its records, views and runtime remain at stable addresses. Mutations
// are serialized; no operation calls an allocator or a user callback after valid
// admission. Native counts and native renderer+1904 remain authoritative.
class NativeViewportRegistry final : public CameraViewportResolver {
public:
    enum class Phase { unused, reserved, live, retired, cancelled };

    // Embed two records in each persistent cockpit construction block. A retired
    // view is inert: its object may still be borrowed, but its native fields must
    // not be read. Never reclaim/rebind it until explicit host quiescence.
    class Storage final {
    public:
        Storage() noexcept = default;
        ~Storage() noexcept;
        Storage(const Storage&) = delete;
        Storage& operator=(const Storage&) = delete;
        Phase phase() const noexcept { return phase_; }
    private:
        friend class NativeViewportRegistry;
        NativeViewportRegistry* registry_{};
        Storage* previous_{};
        Storage* next_{};
        std::uintptr_t actual_key_{};
        Phase phase_{Phase::unused};
        std::optional<CameraViewport> view_;
    };

    class Admission final {
    public:
        Admission() noexcept = default;
        ~Admission() noexcept;
        Admission(const Admission&) = delete;
        Admission& operator=(const Admission&) = delete;
        Admission(Admission&&) noexcept;
        Admission& operator=(Admission&&) noexcept;
        void cancel() noexcept;
        explicit operator bool() const noexcept { return registry_ != nullptr; }
        // Validate before a native allocation. Invalid/moved/consumed admission
        // throws without modifying the token, registry or native storage.
        NativeViewportRegistry& require_registry() const;
    private:
        friend class NativeViewportRegistry;
        Admission(NativeViewportRegistry&, Storage&) noexcept;
        NativeViewportRegistry* registry_{};
        Storage* storage_{};
    };

    NativeViewportRegistry() noexcept = default;
    ~NativeViewportRegistry() override;
    NativeViewportRegistry(const NativeViewportRegistry&) = delete;
    NativeViewportRegistry& operator=(const NativeViewportRegistry&) = delete;
    [[nodiscard]] Admission admit(Storage&);
    void constructed(Admission&, NativeViewportOwner&) noexcept;
    const CameraViewport* resolve_viewport(NativeViewportOwner*) noexcept override;
    void retire_before_destroy(NativeViewportOwner&) noexcept;
    // Caller guarantees all frame calls and semantic viewport()/cache borrows
    // of this host view have ended. This never clears a native renderer field.
    // Live/reserved records cannot be forgotten; empty records are a no-op.
    void forget_quiescent(Storage&) noexcept;

private:
    friend class NativeViewportRegistryBinding;
    void cancel(Admission&) noexcept;
    Storage* records_{}; // caller-owned records, including inert retired records
};

// Install once before managed admission. Further construction runtimes borrow
// this same registry. No competing/nested installation, including the same one.
// Teardown requires every record forgotten and all host consumers quiescent.
class NativeViewportRegistryBinding final {
public:
    explicit NativeViewportRegistryBinding(NativeViewportRegistry&);
    ~NativeViewportRegistryBinding() noexcept;
    NativeViewportRegistryBinding(const NativeViewportRegistryBinding&) = delete;
    NativeViewportRegistryBinding& operator=(const NativeViewportRegistryBinding&) = delete;
private:
    NativeViewportRegistry& registry_;
};

// Common source B1F8F0 entry calls this before either profile store or lifetime
// end. Exact identity only; unregistered diagnostic owners have nothing to retire.
void retire_registered_native_viewport_before_destroy(NativeViewportOwner&) noexcept;

} // namespace bsp
