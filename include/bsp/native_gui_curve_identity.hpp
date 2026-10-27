#pragma once
#include "bsp/native_gui_curve_storage.hpp"

namespace bsp {
struct NativeGuiCurveIdentityContext {
    NativeGuiCurveStorageContext& curve;
    const volatile std::uint32_t* profile_00d5c7f8; // genuine first two cells minimum
};
// Metadata for one completed SAME140h Curve/144h slot. Borrows its existing
// actual+04 atomic and binds the SAME canonical actual-owner registry; no count
// initialization/retain, page token, Model or second raw-tree authority.
class NativeGuiCurveReference final : public RenderCommandReference {
public:
    NativeGuiCurveReference(void* actual_curve, NativeGuiCurveIdentityContext&);
    ~NativeGuiCurveReference() override;
    NativeGuiCurveReference(const NativeGuiCurveReference&) = delete;
    NativeGuiCurveReference& operator=(const NativeGuiCurveReference&) = delete;
    void* actual_identity() const noexcept { return identity_; }
    bool retired() const noexcept;
    bool failed() const noexcept;
    void release_zero_references() noexcept override;
    // ABE010 direct entry, no reference decrement/test. Success unbinds only
    // the opaque raw key after native scalar (flags0 keeps slot). Failure
    // keeps entry/raw graph/context/diagnostics bound and forbids replay,
    // unbind, slot reuse or dependency drain; retain this SAME companion.
    void* delete_scalar_00abe010(const volatile std::uint32_t& actual_flags_slot);
    const NativeGuiCurveStorageAcquired& terminal_observation() const noexcept { return terminal_; }
private:
    enum class Phase { bound, destroying, failed, retired };
    void* const identity_;
    NativeGuiCurveIdentityContext& context_;
    Phase phase_{Phase::bound};
    NativeGuiCurveStorageAcquired terminal_;
};
} // namespace bsp
