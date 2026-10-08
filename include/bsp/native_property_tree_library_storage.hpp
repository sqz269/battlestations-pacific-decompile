#pragma once

namespace bsp {
// Complete native 008F5670..008F5697 storage constructor. ECX is genuine
// fresh/unowned 10Ch storage; EAX returns that same address; EDI is preserved;
// plain RET; caller requires DF=0. Writes D1650C/D162C8 DATA phase words,
// count zero and all64 heads. No allocation, calls, release or dispatch.
void* __fastcall initialize_native_property_tree_library_storage_008f5670(void*) noexcept;

// The actual108h map at root+4 remains borrowed interior storage. Native
// property-map cleanup, scalar wrappers, historical allocator/class/global
// publication/private EH and game lifetime are outside this constructor.
} // namespace bsp
