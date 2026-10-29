#pragma once
#include "bsp/native_gui_text_field_storage.hpp"
#include "bsp/native_gui_text_prefix_geometry_lifetime.hpp"
#include "bsp/native_gui_text_shader_storage.hpp"
#include <array>
#include <cstddef>
#include <exception>

namespace bsp {
// Caller Source history only. The real retained caller performs41E870/41DD20;
// these markers never issue owned Text1C8 or Effect credit. No native destructor.
struct NativeGuiTextFontInputNameProgress final {
    // Extent classification ONLY; text_header grants no ownership or permission.
    enum class Storage { caller_owned, text_header };
    NativeGuiTextFontInputNameProgress(const void* header, NativeStringRawPoolContext& pool,
        Storage source = Storage::caller_owned) noexcept : header8(header), strings(pool), storage(source) {}
    NativeGuiTextFontInputNameProgress(const NativeGuiTextFontInputNameProgress&) = delete;
    NativeGuiTextFontInputNameProgress& operator=(const NativeGuiTextFontInputNameProgress&) = delete;
    const void* const header8;
    NativeStringRawPoolContext& strings;
    const Storage storage;
    bool construct_entered{}, construct_returned{}, construct_failed{};
    bool return_entered{}, returned{}, return_failed{};
};
struct NativeGuiTextFontLifetimeSelector final {
    NativeGuiTextRawFontNameStorageContext& context;
    const NativeGuiTextRawFontNameStorageFrame& frame;
    const NativeGuiTextRawFontNameStorageAcquired& acquired;
    const NativeGuiTextFontInputNameProgress& input;
};
// Install this exact nonmoving graph before Text allocation/raw122. The optional
// second selector is distinct; its prior points at first.publication_storage().
// All providers/records/C++ Font collection outlive Text and parent drain.
struct NativeGuiTextFontLifetimeStorageContext final {
    const NativeGuiTextPrefixGeometryLifetimeContext& geometry;
    NativeFontResourceOwners& fonts;
    NativeGuiTextShaderStorageContext& shader_context;
    NativeGuiTextShaderStorageFrame& shader;
    const NativeGuiTextReferenceFrame& reference_frame;
    const NativeGuiTextFontLifetimeSelector initial;
    const NativeGuiTextFontLifetimeSelector* const second{};
};
struct NativeGuiTextFontLifetimeRegion final {
    const void* begin{};
    std::size_t bytes{};
};
struct NativeGuiTextFontLifetimeRegions final {
    // Concrete fixed storage, no erased callback/provider or ownership claim.
    std::array<NativeGuiTextFontLifetimeRegion, 48> entries{};
    std::size_t count{};
};

// Metadata admission only, no native ownership, count, header or cleanup body.
// Existing raw122/widget/owner gates remain unchanged until Root composes them.
class NativeGuiTextFontLifetimeStorageFrame final {
public:
    enum class Phase { fresh, prepared, sealed, failed };
    enum class Stage { bound, before_scalar, scalar_preentry,
        raw118_preentry, raw118_body, base };
    explicit NativeGuiTextFontLifetimeStorageFrame(const NativeGuiTextFontLifetimeStorageContext&) noexcept;
    ~NativeGuiTextFontLifetimeStorageFrame() noexcept;
    NativeGuiTextFontLifetimeStorageFrame(const NativeGuiTextFontLifetimeStorageFrame&) = delete;
    NativeGuiTextFontLifetimeStorageFrame& operator=(const NativeGuiTextFontLifetimeStorageFrame&) = delete;
    // Pure metadata checks; never inspect uninitialized input/native headers.
    void prepare_before_text_allocation();
    // Real canonical prefix+geometry must be complete. Runs existing exact-record
    // Source authentication before payload; BEFORE first Font input/selector.
    void seal_after_prefix_geometry(void* actual_text);
    // Every stage authenticates exact factory/reference/scalar and private
    // selector/shader history. No old/default fallback on pending/failed work.
    void require_live(void* actual_text, Stage,
        const NativeGuiTextRetirementAcquired* exact_destruction = nullptr);
    // Stable C++/caller8B storage only, valid after native payload/companion end.
    NativeGuiTextFontLifetimeRegions metadata_regions() const noexcept;
    // Adds current live owned-header/Font extents only while allowed by stage.
    // Header bit6 entry omits native extents and forbids derived payload reads.
    NativeGuiTextFontLifetimeRegions live_regions(void* actual_text, Stage,
        const NativeGuiTextRetirementAcquired* exact_destruction = nullptr);
    // Independent retained metadata only. No raw Text/companion/Font/header/
    // Effect reads, registry lookup or native image getter after disposal.
    bool post_text_quiescent() const noexcept;
    void require_post_text_quiescent();
    bool requires_process_retention() const noexcept;
    Phase phase() const noexcept { return phase_; }
    const NativeGuiTextFontLifetimeStorageContext& prepared_context() const noexcept { return context_; }
    const void* text_identity() const noexcept { return actual_; } // diagnostic after return
    const std::exception_ptr& failure() const noexcept { return failure_; }
private:
    const NativeGuiTextFontLifetimeStorageContext& context_;
    Phase phase_{Phase::fresh};
    bool active_{};
    void* actual_{};
    const NativeGuiTextRawModelReference* reference_{}; // saved identity after disposal
    std::exception_ptr failure_;
    void enter();
    void leave();
    void fail() noexcept;
    void admit(void*, Stage, const NativeGuiTextRetirementAcquired*);
};
} // namespace bsp
