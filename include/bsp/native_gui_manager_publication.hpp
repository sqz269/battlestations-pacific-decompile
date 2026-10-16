#pragma once
#include <cstdint>

namespace bsp {
struct NativePlatformFocusOwnersContext;

// Required borrowed Source binding; construction checks the function pointer
// before any raw entry. No default provider, publication, owner or registration
// is created. The SAME owners context is passed to every scalar dispatch.
class NativeGuiManagerPublicationCalls final {
public:
    using CurrentScalarZero = void (*)(void* provider_state, void* actual_owner,
        std::uint32_t flags, NativePlatformFocusOwnersContext& same_owners);

    explicit NativeGuiManagerPublicationCalls(NativePlatformFocusOwnersContext&,
        void* provider_state, CurrentScalarZero);

    NativePlatformFocusOwnersContext& actual_owners() const noexcept;
    void call_current_scalar_zero(void* actual_owner, std::uint32_t flags) const;

private:
    NativePlatformFocusOwnersContext& owners_;
    void* const provider_state_;
    const CurrentScalarZero current_scalar_zero_;
};

// Whole00736250[164B/44ins]: no consumed Native arguments, void, plain RET.
// The view, owners/cells, provider metadata, actual raw singleton manager and
// captured section must survive all reached calls and exception cleanup. They
// cannot alias retired GUI storage or private guard/Native scratch. Access is
// externally serialized; admitted synchronous callbacks may change publication.
// No typed GuiManager/layout cast, arbitrary numeric callable VA or CRT default.
//
// The scalar provider must read the reached receiver's CURRENT numeric profile,
// resolve its SAME live table, capture CURRENT slot0 and dispatch(flags1).
// It cannot substitute a cached owner/profile or clear publication itself as a
// wrapper policy. A known D5BFCC target may use actual AA6540 with its matching
// NativeGuiMediaFocusLifetimeContext, borrowing these SAME owners/resources.
// Actual callee/base cleanup may clear publication, as the Original does.
// The provider and table/call metadata must remain valid after receiver free.
//
// Early null does nothing. Otherwise capture first manager+10 section; enter,
// increment depth then arm state0; recheck publication; second manager getter
// BEFORE current unregister argument; reload publication for current scalar0(1);
// clear only after return; decrement/Leave captured section. No AddRef/replay.
// Ordinary Source C++ exception cleanup consumes state0 before actual411EE0;
// escaping cleanup failure terminates. A failed entry is not restartable and
// the caller retains its unresolved domain for explicit disposition. Native
// FH3/SEH/private-frame aliases, binary ABI, full populated retirement, Original
// CRT ownership and production activation are not established by this API.
void destroy_native_gui_manager_publication_00736250(
    const NativeGuiManagerPublicationCalls&);
} // namespace bsp
