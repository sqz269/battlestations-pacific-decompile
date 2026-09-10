#pragma once

#include "bsp/d3d9_startup.hpp"
#include "bsp/d3d9_surface_pool.hpp"
#include "bsp/native_resource_support.hpp"
#include "bsp/native_string.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace bsp {

// Actual object storage, occupying only [slot, slot+34h) of a 38h pool slot.
// There is no host virtual table, companion reference count, or COM owner.
struct NativeSurfaceOwnerStorage {
    std::uint32_t vtable_00;
    std::atomic<std::int32_t> references_04;
    std::uint32_t scalar_bits_08;
    std::uint32_t scalar_bits_0c;
    NativeString name_10;
    D3DFORMAT format_18;
    UINT width_1c;
    UINT height_20;
    D3DMULTISAMPLE_TYPE multisample_24;
    DWORD flags_28;
    IDirect3DSurface9* surface_2c;
    std::uint8_t kind_30;
    std::array<std::byte, 3> untouched_31;
};
static_assert(sizeof(NativeSurfaceOwnerStorage) == 0x34);
static_assert(offsetof(NativeSurfaceOwnerStorage, references_04) == 4);
static_assert(offsetof(NativeSurfaceOwnerStorage, name_10) == 0x10);
static_assert(offsetof(NativeSurfaceOwnerStorage, surface_2c) == 0x2c);
static_assert(offsetof(NativeSurfaceOwnerStorage, kind_30) == 0x30);

// Borrowed raw pointer array; capacity and the stale final slot survive removal.
struct NativeSurfacePointerArray {
    NativeSurfaceOwnerStorage** data_00;
    std::uint32_t count_04;
    std::uint32_t capacity_08;
};
static_assert(sizeof(NativeSurfacePointerArray) == 12);

// A view of actual renderer storage, not an independently owned renderer/list.
// The renderer identity differs from the address of its array header at +1B0C.
struct NativeSurfaceRendererStorage {
    std::array<std::byte, 0x1b0c> other_renderer_fields;
    NativeSurfacePointerArray surfaces_1b0c;
};
static_assert(offsetof(NativeSurfaceRendererStorage, surfaces_1b0c) == 0x1b0c);

struct NativeSurfaceOwnerContext {
    NativeSurfaceRendererStorage* volatile& actual_renderer_00f8d394;
    D3D9SurfacePool& actual_surface_pool_0108db00;
    SizedStoragePool& actual_string_pool_00419cc0;
    SingletonLifetimeDomain& actual_lifetime_01090aa0;
    NativeResourceSupportStorage* volatile& actual_resource_support_0108fedc;
    std::uint32_t& actual_tracking_counter_0108dafc;
    const volatile std::uint32_t& actual_one_00d7a24c;
};

// 00B3F630: ECX slot, stack COM/flags/kind(low byte), EAX owner, RET 0Ch.
// Caller owns the raw slot and must return it if construction throws.
NativeSurfaceOwnerStorage* construct_native_surface_00b3f630(
    void* actual_pool_slot, IDirect3DSurface9* surface, DWORD flags,
    std::uint8_t kind, NativeSurfaceOwnerContext&);
// 00B3CC80: ECX owner, stack COM, RET 4. Overwrites without releasing old COM;
// ignores GetDesc HRESULT and reloads the owner field after captured AddRef.
void bind_native_surface_00b3cc80(NativeSurfaceOwnerStorage&, IDirect3DSurface9*);
// 00B3F4E0: ECX owner, RET. Unregisters from the actual renderer, consults the
// actual support singleton, releases COM and name, then installs base profile.
void destroy_native_surface_00b3f4e0(NativeSurfaceOwnerStorage&, NativeSurfaceOwnerContext&);
// 00B3F5B0: ECX owner, stack flags, EAX original address, RET 4. Bit zero returns
// the slot to the actual canonical pool only after destruction completes.
NativeSurfaceOwnerStorage* delete_native_surface_00b3f5b0(
    NativeSurfaceOwnerStorage&, std::uint32_t flags, NativeSurfaceOwnerContext&);
// 00B3D510: ECX owner, RET. Captured AddRef/Release pair, then reloaded Release.
void release_native_surface_for_reset_00b3d510(NativeSurfaceOwnerStorage&);
// 00B3D550: ECX owner, stack actual device, RET 4. EAX is the COM result.
// Writes directly to +2C; no preflight, HRESULT branch, rollback or AddRef.
HRESULT recreate_native_surface_00b3d550(NativeSurfaceOwnerStorage&, IDirect3DDevice9&);

// 00B25630: ECX array header, stack pointer-to-target-pointer, EAX bool, RET 4.
bool remove_first_native_surface_00b25630(
    NativeSurfacePointerArray&, NativeSurfaceOwnerStorage* const* target);
// 00B27D60: ECX actual renderer identity, stack target, EAX bool, RET 4.
bool unregister_native_surface_00b27d60(
    NativeSurfaceRendererStorage&, NativeSurfaceOwnerStorage* target);

} // namespace bsp
