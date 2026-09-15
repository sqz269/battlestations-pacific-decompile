#pragma once

#include "bsp/native_camera_reference.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>

namespace bsp {
struct NativeDirectionalShadowConstructionContext;
struct NativeDirectionalShadowConstructionExecution;

// Stable caller-owned host storage, separate from the actual 508h shadow owner.
// This block implements no native constructor, destructor, retain or release.
// It must outlive surviving cameras, viewport views and cache child operations.
class NativeDirectionalShadowConstructionBlock final {
public:
    enum class Phase { idle, preparing, prepared, executing, settled };
    static constexpr std::size_t camera_count = 4;
    static constexpr std::size_t viewport_count = 9;

    class Admission final {
    public:
        Admission() noexcept = default;
        Admission(const Admission&) = delete;
        Admission& operator=(const Admission&) = delete;
        Admission(Admission&&) noexcept;
        Admission& operator=(Admission&&) noexcept;
        ~Admission() noexcept;
        // Unused preparation returns to idle. After execution begins this only
        // cancels unused credits and settles, retaining all actual survivors.
        void cancel() noexcept;
        explicit operator bool() const noexcept { return block_ != nullptr; }
    private:
        friend class NativeDirectionalShadowConstructionBlock;
        friend struct NativeDirectionalShadowConstructionExecution;
        friend void* construct_native_directional_shadow_base_00a8e2e0(void*, void*,
            NativeDirectionalShadowConstructionContext&, Admission&&);
        friend void* construct_native_directional_shadow_owner_00a8fa30(void*, void*,
            NativeDirectionalShadowConstructionContext&, Admission&&);
        friend void* allocate_native_directional_shadow_owner_00a8fd30(void*,
            NativeDirectionalShadowConstructionContext&, Admission&&);
        Admission(NativeDirectionalShadowConstructionBlock&, NativeCameraEnvironment&,
            const NativeNodeRawConstants&) noexcept;
        // Validate before moving the caller token or performing any native store.
        // The outer entry moves the token once, then begins execution once.
        // Nested factory/derived/base bodies share it without another admission.
        void validate_prepared() const;
        void begin_execution() noexcept;
        NativeDirectionalShadowConstructionBlock* block_{};
        NativeCameraEnvironment* camera_environment_{};
        const NativeNodeRawConstants* node_constants_{};
        SceneAttachmentRuntime::BindingAdmission scene_admissions_[camera_count];
        GeneratedModelLifetimeRuntime::BindingAdmission lifetime_admissions_[camera_count];
        // V0 standalone +504; V1..4 camera first; V5..8 replacements +10..1C.
        NativeViewportRegistry::Admission viewport_admissions_[viewport_count];
    };

    // User-provided constructor deliberately leaves native raw lanes unwritten.
    NativeDirectionalShadowConstructionBlock() noexcept;
    ~NativeDirectionalShadowConstructionBlock() noexcept;
    NativeDirectionalShadowConstructionBlock(const NativeDirectionalShadowConstructionBlock&) = delete;
    NativeDirectionalShadowConstructionBlock& operator=(const NativeDirectionalShadowConstructionBlock&) = delete;
    NativeDirectionalShadowConstructionBlock(NativeDirectionalShadowConstructionBlock&&) = delete;
    NativeDirectionalShadowConstructionBlock& operator=(NativeDirectionalShadowConstructionBlock&&) = delete;
    // Prepare all 4 scene + 4 lifetime + 9 viewport credits and persistent cache
    // metadata before native allocation. Borrows the already-installed registry.
    [[nodiscard]] Admission admit(NativeCameraEnvironment&,
        const NativeNodeRawConstants&, NativeViewportRegistry&);
    Phase phase() const noexcept { return phase_; }
    // Caller separately proves all native obligations and host borrows ended.
    // Rejects live/reserved views, live owners, unretired references or unfinished
    // cache trees. Performs no native cleanup and never changes a child phase.
    void reset_after_host_quiescence() noexcept;

private:
    friend struct NativeDirectionalShadowConstructionExecution;
    friend void* construct_native_directional_shadow_base_00a8e2e0(void*, void*,
        NativeDirectionalShadowConstructionContext&, Admission&&);
    friend void* construct_native_directional_shadow_owner_00a8fa30(void*, void*,
        NativeDirectionalShadowConstructionContext&, Admission&&);
    friend void* allocate_native_directional_shadow_owner_00a8fd30(void*,
        NativeDirectionalShadowConstructionContext&, Admission&&);

    struct RetirementCookie {
        NativeDirectionalShadowConstructionBlock* block{};
        std::size_t index{};
    };
    struct CameraSlot {
        std::optional<NativeCameraOwner> owner;
        std::optional<NativeCameraReference> reference;
        RetirementCookie retirement;
        // Host facts, never additional native counts or EH states. Completion
        // protects a live camera if a later host reference binding rejects it.
        bool camera_completed{};
        bool retired{};
    };
    enum class BodyPhase { not_started, running, complete, failed };
    struct BodyState {
        BodyPhase phase{BodyPhase::not_started};
        std::int32_t unwind_state{-1};
        std::uint32_t native_site{};
    };
    // Native frame *lane* maps, not executable FH3/private-stack frames. Use
    // memcpy/raw providers for scalar/header access; never initialize whole
    // lanes or recover native identities from ended camera storage.
    // Base origin is EBP-44: flags0, owner4, white8, names10/18/20/28,
    // late size pair30/34, mutable public word48 (all offsets hexadecimal).
    alignas(4) std::byte base_lanes_[0x4c];
    // Derived origin EBP-58: owner0, pair4/8 (later loop raw at4), matrix0C..4B,
    // public word5C (light -> loop count4 -> fifth raw target).
    alignas(4) std::byte derived_lanes_[0x60];
    // Current factory raw allocation, corresponding to EBP-10.
    alignas(4) std::byte factory_raw_lane_[4];
    BodyState base_state_, derived_state_, factory_state_;
    CameraSlot cameras_[camera_count];
    NativeViewportRegistry::Storage viewport_records_[viewport_count];
    std::optional<NativeTextureCacheAcquired> cache_acquisition_;
    NativeViewportRegistry* viewport_registry_{};
    Phase phase_{Phase::idle};

    static void validate_environment(NativeCameraEnvironment&,
        const NativeNodeRawConstants&, NativeViewportRegistry&);
    static void record_camera_retirement(void*, NativeCameraReference&) noexcept;
    static bool cache_tree_quiescent(const NativeTextureCacheAcquired&) noexcept;
    bool idle_storage() const noexcept;
};

// These native bodies belong to the separate constructor module. Source-only
// context/admission interfaces; DY supplies storage and no native-body credit.
void* construct_native_directional_shadow_base_00a8e2e0(void* actual_508h,
    void* actual_light, NativeDirectionalShadowConstructionContext&,
    NativeDirectionalShadowConstructionBlock::Admission&&);
void* construct_native_directional_shadow_owner_00a8fa30(void* actual_508h,
    void* actual_light, NativeDirectionalShadowConstructionContext&,
    NativeDirectionalShadowConstructionBlock::Admission&&);
void* allocate_native_directional_shadow_owner_00a8fd30(void* actual_light,
    NativeDirectionalShadowConstructionContext&,
    NativeDirectionalShadowConstructionBlock::Admission&&);
} // namespace bsp
