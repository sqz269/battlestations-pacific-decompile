#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native FileStore container allocation requires MSVC Win32.
#endif

namespace bsp {

// Complete original 55-byte leaves: no consumed incoming register/stack
// arguments; EAX allocation; RET. Each allocates 1Ch using the shared source
// CRT allocation service, separately tests p/p+4/p+8 before zeroing those
// DWORDs, then writes bytes +18=1,+19=0. All other bytes remain untouched.
// The caller forms the sentinel; these leaves do not create an owning tree.
// Null is not a safe return: the following +4 write still faults.
void* __cdecl allocate_native_file_store_node_00be55e0();
void* __cdecl allocate_native_file_store_node_00be5690();

} // namespace bsp
