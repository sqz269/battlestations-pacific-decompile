#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS extension-prefix storage requires MSVC Win32.
#endif

namespace bsp {
class ActualNativeStringPoolStorage;
struct SingletonLifetimeCallbacks;

// BE1480: ECX actual A0h VFS manager, stack(extension,directory) actual8h
// pooled headers, RET8. The +54 actual 20h RB tree must already be constructed
// by BE1DC0. Copies/normalizes the pair, suppresses an exact normalized pair,
// then inserts a native 20h node ordered by extension. Source storage and
// returning validation are explicit borrowed dependencies; this is not a
// binary ABI or original FH3/SEH replacement.
void register_native_vfs_extension_prefix_00be1480(void* actual_manager,
    const void* actual_extension, const void* actual_directory,
    ActualNativeStringPoolStorage&, const SingletonLifetimeCallbacks&);
} // namespace bsp
