#pragma once

#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Host setup borrows the actual 12-byte 108D530-equivalent tree header and
// invalid-parameter domain. Bind once before startup and keep both alive
// through real CRT atexit processing. Setup does not initialize raw storage.
void bind_static_native_hardware_layout_tree_0108d530(
    void* actual_tree, const SingletonLifetimeCallbacks& invalid_parameters) noexcept;

// CD7960..CD79A0: allocate the actual 28h sentinel, publish it and self-links,
// clear count, then register the genuine shutdown function with std::atexit.
// Returns the real registration result; failure does not roll back the tree.
int initialize_static_native_hardware_layout_tree_00cd7960();

// CE0C50..CE0C90: erase the captured full range through the complete helper,
// free the current head, then clear head/count. Header word0 is preserved.
// Native normal return also zeros EAX; this new void callback has no result.
void destroy_static_native_hardware_layout_tree_00ce0c50();

// New MSVC Win32 C++ interfaces, not native ABI replacements. Valid actual
// tree storage and shared allocation-domain contracts apply. No private tree,
// alternate callback collector, synthetic dependency, or gameplay claim.
} // namespace bsp
