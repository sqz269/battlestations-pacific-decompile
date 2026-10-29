#pragma once
#include "bsp/native_gui_text_main_section_storage.hpp"
#include "bsp/native_gui_text_prefix_geometry_lifetime.hpp"
#include <array>
#include <cstddef>
#include <exception>

namespace bsp {
struct NativeGuiTextShaderStorageFrame;

// Install the whole nonmoving graph before Text allocation/raw122. These are
// the SAME existing379/db55/188/resources, not another Model/geometry/cache.
// Construction stores addresses only; preparation follows whole-Attempt setup.
struct NativeGuiTextMainSectionLifetimeStorageContext final {
    const NativeGuiTextPrefixGeometryLifetimeContext& geometry;
    NativeGuiTextMainSectionStorageContext& construction;
    const NativeGuiTextMainSectionStorageFrame& frame;
    const NativeGuiTextMainSectionStorageAcquired& acquired;
    const NativeGuiTextReferenceFrame& reference_frame;
};
struct NativeGuiTextMainSectionLifetimeRegion final {
    const void* begin{};
    std::size_t bytes{};
};
struct NativeGuiTextMainSectionLifetimeRegions final {
    // Eight stable records and at most four live native extents. The shared
    // reference guard separately walks the material cache's recursive backing.
    std::array<NativeGuiTextMainSectionLifetimeRegion, 16> entries{};
    std::size_t count{};
};

// Metadata admission only: created main section, childless/unparented, one sole
// Section/Material and no streams/layout/content. The original kind is Font-empty.
// The explicit Font+Main kind admits ONLY this structural slice: the coordinator
// separately checks the fixed Font helper while both are alive. No peer traversal.
// This helper neither invokes379/AB8530 nor performs a native owner operation.
class NativeGuiTextMainSectionLifetimeStorageFrame final {
public:
    enum class Phase { fresh, prepared, sealed, failed };
    enum class Stage { bound, before_scalar, scalar_preentry,
        raw118_preentry, raw118_body, base };
    explicit NativeGuiTextMainSectionLifetimeStorageFrame(
        const NativeGuiTextMainSectionLifetimeStorageContext&) noexcept;
    ~NativeGuiTextMainSectionLifetimeStorageFrame() noexcept;
    NativeGuiTextMainSectionLifetimeStorageFrame(const NativeGuiTextMainSectionLifetimeStorageFrame&) = delete;
    NativeGuiTextMainSectionLifetimeStorageFrame& operator=(const NativeGuiTextMainSectionLifetimeStorageFrame&) = delete;
    NativeGuiTextMainSectionLifetimeStorageFrame(NativeGuiTextMainSectionLifetimeStorageFrame&&) = delete;
    NativeGuiTextMainSectionLifetimeStorageFrame& operator=(NativeGuiTextMainSectionLifetimeStorageFrame&&) = delete;

    // Pure metadata; no canonical Text ref exists yet. Names/native fields and
    // provider/type getters are not read. No cache allocation is preseeded.
    void prepare_before_text_allocation();
    // AFTER real379, actual created AB8530 and full late parenting/root return.
    // Requires private83f witness plus the live canonical sole graph; a public
    // COMPLETE/skipped/creator diagnostic cannot manufacture this permission.
    void seal_after_main_section(void* actual_text);
    // Exact factory/reference/scalar/destruction stages. Const admission may
    // poison mutable private metadata; it never changes native ownership.
    void require_live(void* actual_text, Stage,
        const NativeGuiTextRetirementAcquired* exact_destruction = nullptr) const;
    NativeGuiTextMainSectionLifetimeRegions metadata_regions() const noexcept;
    NativeGuiTextMainSectionLifetimeRegions live_regions(void* actual_text, Stage,
        const NativeGuiTextRetirementAcquired* exact_destruction = nullptr) const;

    // Persistent C++ records ONLY after native callbacks/entry erasure. This
    // closes this graph's structural Text debt, not renderer/effect-cache/provider
    // custody or the compound Font slice. It never reads the Font peer, even if
    // that peer's metadata has already ended during owner teardown.
    bool post_text_quiescent() const noexcept;
    void require_post_text_quiescent() const;
    bool requires_process_retention() const noexcept;
    const NativeGuiTextMainSectionLifetimeStorageContext& prepared_context() const noexcept { return context_; }
    Phase phase() const noexcept { return phase_; }
    const void* text_identity() const noexcept { return actual_; }
    const std::exception_ptr& failure() const noexcept { return failure_; }
private:
    const NativeGuiTextMainSectionLifetimeStorageContext& context_;
    mutable Phase phase_{Phase::fresh};
    mutable bool active_{};
    mutable std::exception_ptr failure_;
    void* actual_{};
    NativeGuiTextRawModelReference* reference_{}; // opaque after disposal
    void* primary_{};
    void* internal_{};
    void* mesh_{};
    void* shadow_mesh_{};
    void* section_{};
    void* material_{};
    const NativeGuiTextShaderStorageFrame* shader_{};
    bool empty_font_sealed_{};
    void enter() const;
    void leave() const;
    void fail() const noexcept;
    void admit(void*, Stage, const NativeGuiTextRetirementAcquired*) const;
};
} // namespace bsp
