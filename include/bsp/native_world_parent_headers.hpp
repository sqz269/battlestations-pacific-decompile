#pragma once

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native World parent headers require MSVC Win32.
#endif

namespace bsp {

inline constexpr std::size_t native_world_parent_root_offset = 0x0c;
inline constexpr std::size_t native_world_parent_array_offset = 0x18;
inline constexpr std::size_t native_world_parent_header_count = 97;
inline constexpr std::size_t native_world_parent_header_stride = 0x0c;

// New C++ storage composition for 004CB057..004CB07A. The caller supplies an
// actual writable 0x4BC-byte World allocation with fresh, unowned root/array
// storage. Calls the actual raw initializer on root+0C, then97 headers at+18
// ascending. A populated owning header must not be reset. Writes only the
// contiguous [World+0C,World+4A4) root/array span; allocates no storage.
// Header13's separate complete-helper Source admission remains pending.
void initialize_native_world_parent_headers(void* actual_world_storage) noexcept;

// New C++ composition of constructor cleanup state1 -> state0 -> -1:
// 00C6563B/00BF7C6E clear all97 array headers in reverse through actual
// 004C2D30; 00C65630 then clears the root at+0C through the same callback.
// All98 headers must already be initialized and coherent. Any owned nodes
// must satisfy the accepted raw clear's canonical-heap/nonaliasing contract;
// payloads remain borrowed. The callbacks free nodes, never header/World storage.
// No partial-prefix or diagnostic failure protocol is provided. Both fixed
// callbacks are noexcept; general Native CRT/FH3/SEH and fault recovery are
// outside this interface. This is not the normal World destructor or a Native
// binary-ABI replacement, and it does not touch vptr, sentinel or ready fields.
// The ordinary C++/canonical-free calling context requires DF0.
void clear_native_world_parent_headers(void* actual_world_storage) noexcept;

} // namespace bsp
