#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer destruction requires MSVC Win32.
#endif

namespace bsp {
struct NativeLuaBootstrapInputs;
struct NativeShaderStateDefinitionsStorage;
struct NativeStringPoolStorage;
struct NativeRendererBindingResetContext;
struct NativeRendererCacheClearContext;
struct NativeRendererQueryTerminalContext;
struct NativeVertexDeclarationRegistryLifetimeContext;
struct NativeRendererCacheCleanupContext;
struct NativeEffectRegistryDestructionContext;
class NativeNodeDestructionRuntime;

// Immutable original numeric words, used only as selectors for reconstructed
// providers. Each reached table needs its first DWORD. No original EXE address
// is executed. The child contexts supply their own full required table extents.
struct NativeRendererDestructorProfiles {
    const volatile std::uint32_t* control_00d5f1f0;
    const volatile std::uint32_t* state_base_00d621ec;
    const volatile std::uint32_t* state_00d62260;
    const volatile std::uint32_t* system_base_00d626f4;
    const volatile std::uint32_t* system_00d62a3c;
    const volatile std::uint32_t* lua_base_00d5e5a4;
    const volatile std::uint32_t* lua_00d5e5a8;
};

// Borrow one actual application graph. All child contexts, canonical owner
// bindings, pools and publications must refer to these SAME AA0/AA8/AA4 and
// F8D394 cells. Resource-support consumers require raw SoundLifetimeAccess.
// The fixed control-worker process binding remains alive through all joins.
// No context owns or substitutes renderer storage, a manager or a registry.
struct NativeRendererDestructorContext {
    void* volatile& current_renderer_00f8d394;
    void* volatile& singleton_manager_01090aa0;
    NativeStringPoolStorage* volatile& string_pool_01090aa8;
    volatile std::uint32_t& small_returns_disabled_01090aa4;
    NativeShaderStateDefinitionsStorage* volatile& state_definitions_0108fe90;
    void* volatile& system_constants_0108fe94;
    void* volatile& lua_owner_00f8d434;
    const NativeLuaBootstrapInputs& lua_bootstrap;
    NativeRendererBindingResetContext& binding_reset;
    NativeRendererCacheClearContext& binding_cache;
    NativeRendererQueryTerminalContext& query_terminal;
    NativeVertexDeclarationRegistryLifetimeContext& declaration_cleanup;
    NativeRendererCacheCleanupContext& texture_cleanup;
    NativeEffectRegistryDestructionContext& effect_cleanup;
    NativeNodeDestructionRuntime& actual_nodes;
    const NativeRendererDestructorProfiles& profiles;
};

// Full B32920..B339C0 (4257 bytes). Original ECX actual1D94h owner, RET;
// this new fastcall borrows a context in EDX. Complete normal and 29-state
// source C++ unwind schedules; current fields, captured cells and COM receiver
// reloads follow the listing. No extra rollback or release of untouched cells.
// Nonnull +19E0/+19E4/+19E8 must have canonical NativeModelReference bindings
// over actual native storage/raw names. B4C700 return/store gates establish
// model owners at19E4/19E8;19E0's writer remains unproved. Canonical factory
// binding is a separate precondition; projected node substitutes are excluded.
void __fastcall destroy_native_renderer_00b32920(
    void* actual_renderer, NativeRendererDestructorContext*);

// B339F0[30]: call full destructor, then free captured owner iff low flags&1;
// return its original pointer bits, even after free. A throw bypasses free.
void* __fastcall delete_native_renderer_00b339f0(
    void* actual_renderer, NativeRendererDestructorContext*, std::uint32_t flags);
// B32900[8]: incoming ECX is secondary+0C; subtract0C before the same scalar.
void* __fastcall delete_native_renderer_secondary_00b32900(
    void* actual_secondary, NativeRendererDestructorContext*, std::uint32_t flags);

// Valid raw extents/alias lifetimes, supported current profiles and genuine
// provider domains are preconditions. Source cleanup throws while unwinding
// terminate. Existing canonical terminal noexcept/retained-operation limits
// remain; native FH3/private-frame, asynchronous SEH and gameplay are unproved.
} // namespace bsp
