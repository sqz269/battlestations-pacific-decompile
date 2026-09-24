#pragma once
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_render_context.hpp"

namespace bsp {
// Complete AC6F70[40B]: ignore incoming ECX, stacked descriptor, result AL,
// RET4. Compare CURRENT F8BFD8/F8BFDC/F8BFE0 in order. Borrow the actual cells;
// do not substitute the Group widget-type enum or snapshot the three words.
// Full upper-EAX/register ABI is not exposed by this source interface.
bool native_gui_group_is_kind_of_00ac6f70(std::uint32_t descriptor,
    const volatile std::uint32_t (&actual_lineage_00f8bfd8)[3]) noexcept;

struct NativeGuiGroupIdentityContext {
    NativeRenderActualOwnerRegistry& registry;
    NativeGuiWidgetLifetimeContext& lifetime;
    // Actual current D5CB80, at least the first two DWORD cells. They are
    // native code tokens, not callable host addresses. No default table.
    const volatile std::uint32_t* group_profile_00d5cb80;
};

// Host metadata admitted SEPARATELY after successful raw Group construction.
// AA12F0 performs no admission call. Storage is the SAME aligned EC-byte
// object/F0-byte pool slot; the already-live prefix+04 atomic is borrowed.
// Bind adds no native store, allocation, retain or release. A duplicate or
// failed host registration leaves the actual owner caller-owned and unchanged.
// Never bind an unfinished constructor, returned slot or another companion.
//
// Caller guarantees lifetime.nodes resolves every reachable actual node to
// the SAME canonical model companion registered in registry; the interface
// carries no invented domain field with which to certify this generically.
// All parent/child/timed callbacks and retained resources use that same domain.
// Required other-derived/timed dispatch remains on lifetime.bindings. Group
// current0C can call the standalone helper above; current20 uses raw AA8320.
//
// Companion/context stay address-stable through dispatch and retirement.
// External synchronization prevents native-slot reuse/admission until metadata
// retirement completes. Once admitted, both terminal and explicit scalar
// deletion must pass through this companion, never a second raw deletion path.
class NativeGuiGroupReference final : public RenderCommandReference {
public:
    NativeGuiGroupReference(void* actual_group, NativeGuiGroupIdentityContext&);
    ~NativeGuiGroupReference() override;
    NativeGuiGroupReference(const NativeGuiGroupReference&) = delete;
    NativeGuiGroupReference& operator=(const NativeGuiGroupReference&) = delete;

    // Count is already zero. Current0 must be BD30E0, which reloads current
    // profile and selects AC73E0 with flags1. No second decrement is performed.
    void release_zero_references() noexcept override;

    // Explicit selected AC73E0 body: no +04 test/decrement. The borrowed actual
    // flags slot is read ONLY by the raw deleter, after AA9730. Flags0 destroys
    // payload and retires metadata while leaving the slot caller-owned; flags1
    // also returns the slot. This returns its numeric identity, not live storage.
    // After entered destruction, an exception also retires metadata and escapes;
    // raw slot/partial native state remain caller-owned, with no retry/rollback.
    // Terminal delivery catches such failures through its existing noexcept
    // boundary. Original FH3/SEH and binary ABI are not claimed.
    void* delete_scalar_00ac73e0(const volatile std::uint32_t& actual_flags_slot);
    bool retired() const noexcept;
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeGuiGroupIdentityContext& context_;
    Phase phase_{Phase::bound};
    void retire() noexcept;
};
} // namespace bsp
