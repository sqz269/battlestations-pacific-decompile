#pragma once
#include "bsp/native_gui_grid_storage.hpp"

namespace bsp {
struct NativeGuiGridIdentityContext {
    NativeGuiGridContext& grid; // SAME canonical child/widget actual-owner registry
    const volatile std::uint32_t* profile_00d5ce60; // first two current cells minimum
};
// Companion over one SAME completed actual124h Grid/128h slot, borrowing its
// existing+04 atomic. Adds no native count, retain, child link or layout owner.
// Before any scalar/zero terminal, its raw counts/ranges must be in the explicit
// clear-zero supported domain. Current child04 and same-tree lookup are genuine
// caller services; numeric entries are never called as host code.
class NativeGuiGridReference final : public RenderCommandReference {
public:
    NativeGuiGridReference(void* actual_grid, NativeGuiGridIdentityContext&);
    ~NativeGuiGridReference() override;
    NativeGuiGridReference(const NativeGuiGridReference&) = delete;
    NativeGuiGridReference& operator=(const NativeGuiGridReference&) = delete;
    void* actual_identity() const noexcept { return identity_; }
    bool retired() const noexcept;
    bool failed() const noexcept;
    void release_zero_references() noexcept override;
    // Explicit ACBC90: no decrement/count test. Success unbinds opaque key after
    // terminal (flags0 keeps slot). Failure keeps entry/graph/diagnostics bound;
    // retain this SAME companion/context and exit before dependency drain.
    // Never retry, unbind, reuse slot or replace this metadata after failure.
    void* delete_scalar_00acbc90(const volatile std::uint32_t& actual_flags_slot);
    const NativeGuiGridRetirementAcquired& terminal_observation() const noexcept { return terminal_; }
private:
    enum class Phase { bound, destroying, failed, retired };
    void* const identity_;
    NativeGuiGridIdentityContext& context_;
    Phase phase_{Phase::bound};
    NativeGuiGridRetirementAcquired terminal_;
};
} // namespace bsp
