#pragma once
#include "bsp/native_gui_button_storage.hpp"
#include "bsp/native_render_context.hpp"

namespace bsp {
struct NativeGuiButtonIdentityContext {
    NativeRenderActualOwnerRegistry& registry;
    NativeGuiButtonLifetimeContext& lifetime;
    const volatile std::uint32_t* button_profile_00d5bed0; // current first two cells minimum
};
// Canonical metadata over the SAME completed actual FCh Button/100h pool slot.
// Borrows its existing +04 atomic; no count initialization, retain or registry
// list insertion. Contexts/profile cells stay live through terminal unbind.
// All zero/explicit scalar retirement passes through this companion. External
// serialization excludes slot reuse until opaque-key unbind has completed.
class NativeGuiButtonReference final : public RenderCommandReference {
public:
    NativeGuiButtonReference(void* actual_button, NativeGuiButtonIdentityContext&);
    ~NativeGuiButtonReference() override;
    NativeGuiButtonReference(const NativeGuiButtonReference&) = delete;
    NativeGuiButtonReference& operator=(const NativeGuiButtonReference&) = delete;
    void* actual_identity() const noexcept { return identity_; }
    bool retired() const noexcept;
    void release_zero_references() noexcept override;
    // Explicit AA3D10, no decrement/count test. Flags0 keeps physical slot but
    // retires payload/metadata. Entered failure also retires metadata; preserve
    // failed raw storage/providers without terminal retry or slot reuse.
    void* delete_scalar_00aa3d10(const volatile std::uint32_t& actual_flags_slot);
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeGuiButtonIdentityContext& context_;
    Phase phase_{Phase::bound};
    void retire() noexcept;
};
} // namespace bsp
