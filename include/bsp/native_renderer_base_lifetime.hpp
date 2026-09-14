#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer base lifetime reconstruction requires MSVC Win32.
#endif

namespace bsp {

// The application's two actual mutable publication cells. No projected
// singleton domain, shadow renderer owner or replacement map.
struct NativeRendererBaseContext final {
    void* volatile& current_renderer_00f8d394;
    void* volatile& singleton_manager_01090aa0;
};

// Complete B33A70: ECX primary object, EAX same pointer, RET. Writes D5F1EC,
// zeroes +4/+8, then stores the actual BD1860 tracked-section allocation at +4.
void* construct_native_renderer_primary_base_00b33a70(void* primary);

// Complete B33D90: ECX primary, plain RET. Write D5F1EC, then release the
// actual tracked section at primary+4 through the existing 0041CC80 body.
void destroy_native_renderer_primary_base_00b33d90(void* primary) noexcept;

// Complete B25F40/B25FE0: ECX is the singleton subobject at primary+0C.
// F8D394 publishes the PRIMARY address; BD0C30 registers the +0C subobject.
// Destruction unregisters CURRENT F8D394+0C (or null), clears F8D394, and
// leaves the captured first-manager section before setting root profile CE3818.
void* construct_native_renderer_singleton_subobject_00b25f40(
    void* subobject, NativeRendererBaseContext&);
void destroy_native_renderer_singleton_subobject_00b25fe0(
    void* subobject, NativeRendererBaseContext&);

// Complete B26090 scalar wrapper. Bit0 passes the captured SUBOBJECT address
// to the matching CRT free domain. A +0C interior subobject must use flags0.
void* delete_native_renderer_singleton_subobject_00b26090(
    void* subobject, std::uint32_t flags, NativeRendererBaseContext&);

// Complete B283F0/B284E0: ECX primary owner, constructor EAX same pointer,
// plain RET. Native EH state0 invokes B33D90 on the primary if the later
// subobject constructor/destructor fails.
void* construct_native_renderer_base_00b283f0(
    void* primary, NativeRendererBaseContext&);
void destroy_native_renderer_base_00b284e0(
    void* primary, NativeRendererBaseContext&);

// Complete B28540 primary renderer-base scalar wrapper and B33E10 standalone
// primary-base scalar wrapper. Only flags bit0 frees captured primary storage.
// Each scalar source interface returns the captured pointer after free.
void* delete_native_renderer_base_00b28540(
    void* primary, std::uint32_t flags, NativeRendererBaseContext&);
void* delete_native_renderer_primary_base_00b33e10(
    void* primary, std::uint32_t flags);

} // namespace bsp
