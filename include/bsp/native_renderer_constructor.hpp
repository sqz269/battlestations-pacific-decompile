#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer construction requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringPoolStorage;
struct NativeShaderStateDefinitionsStorage;
struct NativeLuaBootstrapInputs;
struct NativeVertexDeclarationRegistryLifetimeContext;
struct NativeRendererCacheCleanupContext;
struct NativeEffectRegistryDestructionContext;

// Borrow one application's actual publication cells. Child contexts are made
// from these same cells; no projected manager, second pool or shadow renderer.
// The three cleanup contexts must use those same actual pool/cache/owner
// domains, with genuine current-profile dispatch and canonical companions.
// All cells/providers must outlive successful singleton children, any native
// retained failure state, and all control worker joins.
struct NativeRendererConstructorContext {
    void* volatile& singleton_manager_01090aa0;
    NativeStringPoolStorage* volatile& string_pool_01090aa8;
    volatile std::uint32_t& small_returns_disabled_01090aa4;
    void* volatile& current_renderer_00f8d394;
    void* volatile& lua_owner_00f8d434;
    NativeShaderStateDefinitionsStorage* volatile& state_definitions_0108fe90;
    void* volatile& system_constants_0108fe94;
    volatile std::uint8_t& frame_flag_0108d4b8;
    volatile std::uint8_t& device_lost_0108d4b9;
    const volatile std::uint32_t& live_one_00d7a24c;
    const NativeLuaBootstrapInputs& lua_bootstrap;
    NativeVertexDeclarationRegistryLifetimeContext& declaration_cleanup;
    NativeRendererCacheCleanupContext& texture_cleanup;
    NativeEffectRegistryDestructionContext& effect_cleanup;
    void* actual_resolution_mode_scratch; // writable10h, reused native mode buffer
    void* actual_identifier_scratch; // writable44Ch; Description at+200h
    void* actual_capabilities_scratch; // existing gather's writable770h frame
};

// Complete B32410..B328F7, 1256 bytes. Original ECX fresh1D94h owner, EAX same,
// plain RET. New source interface adds borrowed context in EDX and uses its
// own C++ private frames. Caller supplies aligned actual storage and a live
// CameraPlaneSet at owner+17C0 with intended native preimages restored, as
// required by B29430. Present parameters+1A28..1A5F remain untouched.
//
// Mode/identifier/gather scratch must have valid readable preimages for failed or
// unwritten COM outputs, including reachable Description NULs. Scratch/context
// bindings do not alias writable operands or one another. The actual control
// worker process context must already be bound and retained through all joins.
//
// Executes the native parent state schedule and descending member cleanup on
// source C++ failure. Failed child allocations are freed only after their own
// cleanup; completed singleton children gain no later-parent-failure rollback.
// Native leaks/publications remain. Cleanup failure during unwind terminates.
// Original FH3/SEH frames, CRT exception identity and gameplay are not proved.
void* __fastcall construct_native_renderer_00b32410(
    void* actual_owner, NativeRendererConstructorContext* context);
} // namespace bsp
