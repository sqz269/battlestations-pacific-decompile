#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native FileStore factories require MSVC Win32.
#endif

namespace bsp {

// Borrow stable bindings to the application's actual mutable publication cells.
// Owner storage is exactly 0Ch: primary+0, lifetime secondary+4, provider cache+8.
// The manager is the existing raw 01090AA0 domain; no private lifetime is created.
struct NativeFileStoreFactoryContext {
    void* volatile& actual_manager_publication_01090aa0;
    void* volatile& actual_factory_publication_0109db68;
};

// Complete BE5320[30]. ECX raw0Ch, EDX unconsumed, EAX input, RET. Write
// secondary D688AC, primary D688B4, secondary D688B0, then cache+8=0, in order.
void* __fastcall construct_native_filestore_factory_00be5320(
    void* actual_primary, void* unused_edx) noexcept;

// Complete 4FC150[199]. Native no-input getter, EAX primary, RET. Fast return
// is the first publication read. Slow path captures first manager's section+10,
// locks/rechecks, constructs raw0Ch and publishes it, captures secondary+4 BEFORE
// a second manager getter, and registers it through actual BD0C30. Registration
// failure retains publication/allocation and releases only the captured guard.
void* get_native_filestore_factory_004fc150(NativeFileStoreFactoryContext&);

// Complete raw BE5350[34]. ECX primary, RET, no semantic result. Clear the
// publication; write secondary CE3818 and primary CFE9F4. Cache+8 is untouched.
void destroy_native_filestore_factory_00be5350(
    void* actual_primary, NativeFileStoreFactoryContext&) noexcept;

// Complete BE5790[58]. ECX primary, stacked flags, EAX original address, RET4.
// Same lifetime stores as BE5350; flags bit0 frees the entire0Ch allocation.
// The nonnull cache is neither released nor cleared. No unregister call exists.
void* delete_native_filestore_factory_00be5790(void* actual_primary,
    std::uint32_t flags, NativeFileStoreFactoryContext&) noexcept;

// Complete raw BE5340[8]. ECX registered secondary; SUB4/JMP BE5790; RET4.
void* delete_native_filestore_factory_secondary_00be5340(void* actual_secondary,
    std::uint32_t flags, NativeFileStoreFactoryContext&) noexcept;

// Complete raw BE5380[41], transient D688AC profile's scalar deleting slot.
// ECX is the base allocation ITSELF, stacked flags, EAX input, RET4. Clears
// publication, writes only ECX+0=CE3818 and optionally frees that exact ECX.
// This has no -4 adjustment and is not the final factory's secondary dispatch.
void* delete_native_filestore_factory_base_00be5380(void* actual_base,
    std::uint32_t flags, NativeFileStoreFactoryContext&) noexcept;

// Complete source bodies with new explicit service/context interfaces, not
// drop-in original ABI/FH3 replacements. Numeric profiles are identity DWORDs,
// not callable source C++ vtables. Original CRT/exception identity, mutable EH
// spill aliases, hardware-fault cleanup and mixed-owner shutdown dispatch remain
// external boundaries. Provider creation/cache lifetime is a separate packet.
// Descriptive names are hypotheses. No game/runtime startup claim is made.
} // namespace bsp
