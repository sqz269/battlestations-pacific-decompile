#pragma once
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/native_render_context.hpp"

namespace bsp {
struct NativeGuiClipBoxIdentityContext {
    NativeRenderActualOwnerRegistry& registry;
    NativeGuiWidgetLifetimeContext& lifetime;
    // SAME initialized F8C13C process pool used by AA1920/ACE620.
    NativeGuiWidgetPool<GuiClipBoxPoolTraits>& actual_matching_pool;
    const volatile std::uint32_t* clip_box_profile_00d5d058; // at least2 DWORDs
};

// Admit ONLY after complete raw ACE0A0/ACE0F0 construction of the SAME AA1920
// result. The caller proves slot/pool provenance, live subobjects and complete
// graph custody; profile/count checks cannot establish these. Borrow actual+04
// through the existing registry, without another count/store/retain/owner map.
// Failed admission leaves the completed native object caller-owned unchanged.
//
// This metadata, context/pool/registry, canonical nodes and all raw parent,
// child/timed backing/providers remain stable through native callbacks. Every
// terminal/explicit deletion after admission uses this companion. External
// serialization prevents reentry, slot reuse and metadata changes. On entered
// failure retire this identity, retain partial raw graph independently, and
// never retry: AA9730 unwind may already have consumed container backing.
class NativeGuiClipBoxReference final : public RenderCommandReference {
public:
    NativeGuiClipBoxReference(void* actual_clip_box, NativeGuiClipBoxIdentityContext&);
    ~NativeGuiClipBoxReference() override;
    NativeGuiClipBoxReference(const NativeGuiClipBoxReference&) = delete;
    NativeGuiClipBoxReference& operator=(const NativeGuiClipBoxReference&) = delete;
    NativeGuiClipBoxReference(NativeGuiClipBoxReference&&) = delete;
    NativeGuiClipBoxReference& operator=(NativeGuiClipBoxReference&&) = delete;

    // Already-observed zero: CURRENT0=BD30E0 freshly selects CURRENT4=ACE620(1).
    // No decrement or lazy metadata construction. An escaping failure terminates.
    void release_zero_references() noexcept override;
    // Explicit selected scalar body; no count gate/decrement. Flags are borrowed
    // stable/disjoint storage and read only after AA9730, per raw ACE620. Flags0
    // ends payload lifetime without pool return; flags1 also returns the slot.
    // After return the result is only an identity; no raw/count access follows.
    void* delete_scalar_00ace620(const volatile std::uint32_t& actual_flags_slot);
    bool retired() const noexcept;
    void* actual_identity() const noexcept { return identity_; }
    NativeGuiWidgetLifetimeContext& lifetime_context() const noexcept { return context_.lifetime; }
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeGuiClipBoxIdentityContext& context_;
    Phase phase_{Phase::bound};
    void retire() noexcept;
};
} // namespace bsp
