#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native pending registry publication requires MSVC Win32.
#endif

namespace bsp {

// Ordinary 00875280[189] with stable borrowed references to the actual cells.
// The fast path returns its first capture; the slow path rereads publication
// after releasing the first manager's captured raw section. Allocation cleanup
// ends before publication, the second manager lookup and current registration.
// Source C++ failure cleanup uses the actual raw8 guard and captured allocation;
// it does not reproduce native FH3/SEH or mutable native stack-spill aliases.
// Production composition requires the canonical process cell and its matching
// manager deletion binding. Numeric profiles are not callable Source vtables.
void* get_native_pending_registry_00875280(
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0);

} // namespace bsp
