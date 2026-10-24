#pragma once
#include "bsp/native_gui_scrollbar_storage.hpp"
#include "bsp/native_render_context.hpp"

namespace bsp {
struct NativeGuiScrollbarIdentityContext {
    NativeRenderActualOwnerRegistry& registry;
    NativeGuiWidgetLifetimeContext& lifetime;
    const volatile std::uint32_t* scrollbar_profile_00d5ccf8; // current first two cells minimum
};
// Canonical metadata over one completed actual FCh Scrollbar/100h pool slot.
// Borrows its SAME already-live +04 atomic and adds no native retain/store.
// Contexts and actual profile cells outlive the companion and all terminal calls.
// Once registered, every zero/explicit scalar terminal uses this companion;
// serialized callers exclude slot reuse until matching opaque-key unbind ends.
class NativeGuiScrollbarReference final : public RenderCommandReference {
public:
    NativeGuiScrollbarReference(void* actual_scrollbar, NativeGuiScrollbarIdentityContext&);
    ~NativeGuiScrollbarReference() override;
    NativeGuiScrollbarReference(const NativeGuiScrollbarReference&) = delete;
    NativeGuiScrollbarReference& operator=(const NativeGuiScrollbarReference&) = delete;
    void* actual_identity() const noexcept { return identity_; }
    bool retired() const noexcept;
    void release_zero_references() noexcept override;
    // Explicit AC8210 does not decrement/test count. Flags0 retires payload and
    // metadata but keeps physical slot. Entered failure retires metadata too;
    // retain failed actual storage/providers for diagnosis, no retry or reuse.
    void* delete_scalar_00ac8210(const volatile std::uint32_t& actual_flags_slot);
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeGuiScrollbarIdentityContext& context_;
    Phase phase_{Phase::bound};
    void retire() noexcept;
};
} // namespace bsp
