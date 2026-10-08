#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native parent list header storage requires MSVC Win32.
#endif

namespace bsp {

// Hypothetical name. Whole [004B7EC0,004B7ECD): 13 bytes / 6 instructions.
inline constexpr std::size_t native_parent_list_header_storage_bytes = 0x0c;

// ECX is actual fresh/unowned writable 12-byte storage. Zero DWORDs +0,+4,+8
// in order; EAX returns the full root and ECX becomes zero. EDX and every
// nonvolatile remain unchanged. Both formals occupy registers; plain RET,
// no stack arguments. Defined XOR flags: CF0/PF1/ZF1/SF0/OF0; AF undefined.
// DF is untouched; no calls, allocation, dispatch, nodes, or teardown.
// A populated owning root must not be reset. The empty raw header does not
// establish native parent/World, iterator EH, sentinel, or class lifetime.
void* __fastcall initialize_native_parent_list_header_storage_004b7ec0(
    void* actual_storage_root, std::uint32_t unused_edx) noexcept;

} // namespace bsp
