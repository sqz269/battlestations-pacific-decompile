#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS content removal requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete 00BDD020..00BDD09D: native ECX=actual VFS manager,
// stack=actual8h counted key, RET4, no defined result. Manager+48h is its
// actual string-vector header. Remove every equal-length, CRT-case-insensitive
// match by assigning the last row over it and releasing the last row, then
// examine that same cursor again. Empty strings compare without data access.
// Reload vector begin/count after each iteration; key may alias mutable rows.
// An empty vector never reads key. No validation or local exception cleanup.
// Source carries the concrete existing raw pool explicitly; current CRT locale,
// original register/stack/EH identity and gameplay are not established here.
void remove_native_vfs_content_package_00bdd020(void* actual_manager,
    const void* actual_key, NativeStringRawPoolContext& strings);
} // namespace bsp
