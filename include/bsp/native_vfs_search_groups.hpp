#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS search-group storage requires MSVC Win32.
#endif

namespace bsp {
class ActualNativeStringPoolStorage;
struct SingletonLifetimeCallbacks;

// Actual A0h manager, actual eight-byte pooled group/value headers. The
// manager's +60 list must have been constructed by BE1DC0. Normal valid
// registration uses its original intrusive nodes and pooled string owners.
// Original ECX manager, stack(group,extension), RET8, no semantic return.
void register_native_vfs_search_extension_00be25e0(void* actual_manager,
    const void* actual_group, const void* actual_extension,
    ActualNativeStringPoolStorage&, const SingletonLifetimeCallbacks&);

// Original ECX manager, stack(group,directory), RET8. Copies and normalizes
// the directory before forwarding selector1. Native empty-directory indexing
// is unsafe; callers must provide a nonempty directory as startup does.
void register_native_vfs_search_directory_00be2600(void* actual_manager,
    const void* actual_group, const void* actual_directory,
    ActualNativeStringPoolStorage&, const SingletonLifetimeCallbacks&);

// Common BE2310, original ECX manager, stack(group,value,selector), RET0C.
// Selector0 chooses node+10 extension values, selector1 node+1C directories.
void register_native_vfs_search_group_value_00be2310(void* actual_manager,
    const void* actual_group, const void* actual_value, std::uint32_t selector,
    ActualNativeStringPoolStorage&, const SingletonLifetimeCallbacks&);
} // namespace bsp
