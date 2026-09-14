#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native effect registry destruction requires MSVC Win32.
#endif

namespace bsp {
struct NativeMaterialEffectCacheContext;
class NativeRenderActualOwners;

// Borrow the SAME actual cache/string/array/loader/canonical-owner domains.
// The existing cache context supplies the D5F074 derived registry profile;
// destruction additionally needs the observed D5F04C base profile through +10.
// Numeric native table words are dispatch evidence, never host callables.
struct NativeEffectRegistryDestructionContext {
    NativeMaterialEffectCacheContext& actual_cache;
    const volatile std::uint32_t* actual_base_profile_00d5f04c;
};

// B32010[31]: incoming ECX unused, nonnull actual effect stack argument, RET4.
// Decrement actual +04 first. Only zero resolves its existing canonical owner,
// whose terminal validates current virtual0 and performs real scalar deletion.
// This function neither adds a companion nor initializes/retains any reference.
void release_native_cached_effect_00b32010(void* actual_effect, NativeRenderActualOwners&);

// B31750[106]: ECX actual registry, RET. Reverse current count/base traversal;
// current child+0C size, subtraction, current registry+10 release, then current
// last record destruction and current count decrement. Finally B30410 resize0.
void clear_native_effect_registry_00b31750(void* actual_registry,
    NativeEffectRegistryDestructionContext&);

// B317C0[23]: ECX raw0Ch header, RET. B30410 resize0 then free CURRENT base
// through the cache's actual array domain. Pointer/capacity remain unchanged.
void destroy_native_effect_record_array_00b317c0(void* actual_header,
    NativeMaterialEffectCacheContext&);

// B320F0[95]: ECX actual registry, RET. Publish D5F04C before state0 arms;
// clear, disarm, then resize0/free. State0 unwind destroys owner+04's array.
// This is object destruction only: registry allocation is not freed.
void destroy_native_effect_registry_00b320f0(void* actual_registry,
    NativeEffectRegistryDestructionContext&);
// B32200[5] is a JMP B320F0, not another destructor body or scalar delete.
void destroy_native_effect_registry_thunk_00b32200(void* actual_registry,
    NativeEffectRegistryDestructionContext&);

// New C++ interfaces; original ECX/stack/FH3 ABI and asynchronous SEH are not
// claimed. Unwind preserves prior writes and current-array cleanup. Cleanup
// failure while unwinding terminates. Existing canonical terminal/pool-release
// noexcept and actual domain requirements remain in force; no game validation.
} // namespace bsp
