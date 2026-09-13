#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native game-resource factory owners require MSVC Win32.
#endif

namespace bsp {

// Actual eight-byte factory: primary+0 CFD850, registered secondary+4 CFD84C.
// Borrow the application's stable, mutable cells in its existing raw manager
// domain. E19B90 is the factory publication; WinMain's F8D31C alias is separate
// and none of these native owner bodies reads or clears that alias.
struct NativeGameResourceFactoryContext {
    void* volatile& actual_manager_publication_01090aa0;
    void* volatile& actual_factory_publication_00e19b90;
};

// Complete 7175D0[206]. Native no-input getter, EAX primary, RET. Fast return
// uses the first publication read. Slow path captures first manager's section,
// enters/rechecks, allocates8 and writes transient secondary CFD7F8, primary
// CFD850, final secondary CFD84C before publication. Capture nullable secondary
// BEFORE the second manager getter; register through actual BD0C30 even if null.
// Registration failure preserves publication/allocation. The sole native EH
// state releases the captured guard; there is no allocation rollback state.
void* get_native_game_resource_factory_007175d0(NativeGameResourceFactoryContext&);

// Complete 716530[34]. ECX primary, RET, no semantic result. Clear current
// E19B90 unconditionally; write secondary CE3818 then primary CFD7DC.
void destroy_native_game_resource_factory_00716530(
    void* actual_primary, NativeGameResourceFactoryContext&) noexcept;

// Complete 716560[58]. ECX primary, stacked flags, EAX captured primary, RET4.
// Same lifetime stores; bit0 frees the entire8 allocation. No unregister call.
void* delete_native_game_resource_factory_00716560(void* actual_primary,
    std::uint32_t flags, NativeGameResourceFactoryContext&) noexcept;

// Complete 716520[8]. ECX registered secondary; SUB4/JMP716560; inherited RET4.
void* delete_native_game_resource_factory_secondary_00716520(void* actual_secondary,
    std::uint32_t flags, NativeGameResourceFactoryContext&) noexcept;

// Complete 7150B0[41], transient CFD7F8 base profile's scalar deletion entry.
// ECX is the base allocation ITSELF, not an adjusted secondary. Clear E19B90,
// write only this+0=CE3818, optionally free this exact address, return it; RET4.
void* delete_native_game_resource_factory_base_007150b0(void* actual_base,
    std::uint32_t flags, NativeGameResourceFactoryContext&) noexcept;

// Complete source bodies with explicit borrowed services, not original ABI or
// FH3 replacements. Profiles are numeric identity DWORDs, never callable C++
// vtables. Native invalid-pointer/hardware-fault unwind, CRT exception identity,
// Create71B870, resource parsing/cache and process publication are separate.
// Descriptive names are hypotheses; no game/runtime validation claim is made.
} // namespace bsp
