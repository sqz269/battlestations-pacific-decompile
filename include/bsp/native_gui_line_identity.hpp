#pragma once
#include "bsp/native_gui_line_storage.hpp"
#include "bsp/native_render_context.hpp"

namespace bsp {
class GuiWidgetOwner;
struct NativeGuiLineIdentityContext {
    NativeRenderActualOwnerRegistry& registry;
    NativeGuiWidgetLifetimeContext& lifetime;
    const volatile std::uint32_t* line_profile_00d5c9e8; // current first two cells minimum
};
// Separate metadata over one completed actual104h Line/108h pool allocation.
// Borrows its SAME already-live +04 atomic, adds no native retain/store, and
// registers that exact canonical reference. Context/provider domains outlive
// companion, current calls, explicit scalar and failure diagnostics. Once bound,
// every scalar/zero terminal must pass through this companion. External
// serialization excludes slot reuse until matching opaque-key unbind completes.
class NativeGuiLineReference final : public RenderCommandReference {
public:
    NativeGuiLineReference(void* actual_line, NativeGuiLineIdentityContext&);
    ~NativeGuiLineReference() override;
    NativeGuiLineReference(const NativeGuiLineReference&) = delete;
    NativeGuiLineReference& operator=(const NativeGuiLineReference&) = delete;
    void* actual_identity() const noexcept { return identity_; }
    bool retired() const noexcept;
    void release_zero_references() noexcept override;
    // Explicit AC2CA0, no count test/decrement. Flags0 destroys payload without
    // pool return, but retires metadata. Entered exception also retires metadata;
    // preserve actual slot and failed providers, with no native retry/rollback.
    void* delete_scalar_00ac2ca0(const volatile std::uint32_t& actual_flags_slot);
    // Checked admission to existing raw AA6720; call at the SAME Model binding
    // point, before parenting/current74. Never allocates or retains a Model.
    // Caller must already have the sole canonical type4 GuiWidgetOwner/layout;
    // this is not a type factory, base-field projection or page construction.
    void bind_same_model_00aa6720(GuiWidgetOwner&);
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeGuiLineIdentityContext& context_;
    Phase phase_{Phase::bound};
    void retire() noexcept;
};
} // namespace bsp
