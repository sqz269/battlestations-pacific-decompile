#pragma once

namespace bsp {
// Borrow the sole actual12h E18A7C-equivalent header: opaque DWORD+0,
// head+4, count+8. Bind once before initialization, and keep that storage
// alive through real CRT atexit processing. This setup owns no tree/storage.
void bind_static_native_input_action_deadline_map_00e18a7c(void* actual_tree) noexcept;

// CC9E30..CC9E6F: allocate actual18h sentinel through the existing compatible
// raw-tree leaf, publish head, set nil/self-links, clear count, then register
// the real destructor with std::atexit. Return its actual0/-1 result; failed
// registration leaves the initialized allocation owned by the supplied header.
// Native no arguments, EAX registration result, RET. This source API requires
// prior binding; it neither supplies a private header nor a game-clock facade.
int initialize_static_native_input_action_deadline_map_00cc9e30();

// CD9EC0..CD9EFF: capture current head/minimum, erase the actual full range,
// free the reloaded current head, then clear head/count. Opaque DWORD+0 is
// untouched. Native no arguments, EAX0, RET; the new void CRT callback ignores
// that incidental result. Call exactly once after successful initialization.
void destroy_static_native_input_action_deadline_map_00cd9ec0();

// Complete valid static-instance lifetime only. Lookup/insertion4D6900 is
// still an external library-storage dependency, including use at game+5C8.
// No std::map projection, key/value owner, original ABI replacement, arbitrary
// concurrent mutation, native FH3/SEH or application/game validation is added.
} // namespace bsp
