#pragma once

#include "bsp/native_string_pool_storage.hpp"

namespace bsp {

// Borrow the application's actual pool publication, small-return gate and
// canonical lifetime domain (with NativeStringPoolLifetimeBinding installed).
// These bindings/context remain valid and unchanged throughout the call; the
// pointed-to publication and gate remain current mutable native storage.
struct NativeRendererGatherCapabilitiesContext {
    void* actual_callee_scratch;
    NativeStringPoolStorage* volatile& actual_pool_publication_01090aa8;
    volatile std::uint32_t& actual_small_returns_disabled_01090aa4;
    SingletonLifetimeDomain& actual_lifetime;
};

// Same actual owner/scratch operation with the application's raw14h lifetime
// manager publication. No projected manager or second pool is introduced.
// Raw manager shutdown requires its explicit pool deletion binding; all cells,
// bindings and scratch remain borrowed under the same contracts below.
struct NativeRendererGatherCapabilitiesActualContext {
    void* actual_callee_scratch;
    NativeStringPoolStorage* volatile& actual_pool_publication_01090aa8;
    volatile std::uint32_t& actual_small_returns_disabled_01090aa4;
    void* volatile& actual_manager_publication_01090aa0;
};

// Complete B2C8E0[4094], translated to a NEW MSVC Win32 source interface:
// ECX actual renderer, EDX fixed context; void result. Original ECX/RET has no
// context and uses its own stack. This is not original caller/SEH ABI compatible.
//
// Renderer is actual raw storage (minimum reached extent 1D8Ah), not either
// typed RendererCapabilities projection. Current +1990 must be an actual
// IDirect3D9 object with its live COM table: +14 GetAdapterIdentifier,
// +28 CheckDeviceFormat, +38 GetDeviceCaps. Ignored HRESULT outputs are neither
// zeroed nor synthesized. Every reached query recaptures current +1990/table.
//
// Scratch is caller-supplied, four-byte-aligned readable/writable raw storage,
// at least 770h bytes, representing offsets from native ESP after saved regs:
// +10 loop index; +14/+18 actual string header; +20 four flags; +24 payload;
// +28 saved inner-header address; +2C..1F3 format table; +1F4..323 D3DCAPS9;
// +324..76F D3DADAPTER_IDENTIFIER9 (Description at +524). Caller supplies valid
// initialized bytes wherever a query/body leaves bytes unwritten. Only original
// writes initialize scratch. In particular +25..27 remain untouched and are
// copied into each successful record. A Description NUL must be reachable in
// valid readable storage; the native scan is not limited to its 512-byte field.
// Ordinary raw aliases are subject to the exact current-read/store schedule and
// valid provider ownership/extents; private C++ stack/EH/context aliases are
// outside this interface. Context/bindings must not alias writable raw operands.
//
// Arrays at +1B5C and +1B68 use full actual primitive/nested providers and their
// shared-heap, valid-extent, DWORD-wrap and exception contracts. No clearing or
// rollback of already published renderer fields/array records is added.
// The local string uses full 41DD40 with ActualNativeStringPoolStorage, including
// its current-header reads, nullable-allocation and fresh-copy source limits.
//
// Fixed host memmove adapts the reached BF7680 copy on valid buffers, including
// overlap. Arguments are captured even when zero bytes omit the service call.
// Fixed host strstr adapts BF9440 on valid NUL-terminated strings. Neither claims
// original vector dispatch/0109EEA4, internal strchr, instruction/register,
// SIMD, fault, page-access or asynchronous mutation equivalence.
//
// Cleanup arms after resize and copy. C++ exceptions after that arm destroy the
// current local header through full 41DD20 (pointer then length), then rethrow;
// the existing noexcept release requires a returning pool getter. Normal exit
// tests pointer, disarms, captures length then pointer, calls current 419CC0 and
// BD1510 directly. It may propagate a getter exception without a second cleanup.
// Native FS/FuncInfo/SEH, hardware faults, incidental registers and original
// runtime/game behavior are not established by this source interface.
void __fastcall gather_native_renderer_capabilities_00b2c8e0(
    void* actual_renderer, const NativeRendererGatherCapabilitiesContext* context);

// The SAME full body and literal table, using the complete raw00419CC0 getter
// for each allocation/release and the disarmed normal-return getter. In
// particular normal getter failure may propagate without another local cleanup;
// armed unwind retains NativeStringStorage::release's noexcept limitation.
// Caller scratch/preimages and parent FH3 composition are still explicit.
void __fastcall gather_native_renderer_capabilities_00b2c8e0(
    void* actual_renderer, const NativeRendererGatherCapabilitiesActualContext* context);

} // namespace bsp
