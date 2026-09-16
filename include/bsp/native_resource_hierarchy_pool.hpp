#pragma once

namespace bsp {
class NativeMaterialParameterPool;

// Bind the companion for the application's actual 38h hierarchy owner 109022C.
// It must borrow THAT owner and the SAME AllocatorListDomain as other pools;
// the material companion owning F8D3E4 is a different instance. Bind before pool
// startup and leave the binding immutable thereafter. Companion/storage/shared
// list must outlive the real atexit callback, with every payload already dead.
// This adds no pool storage, copied state, initialization guard or exit registry.
void bind_static_native_hierarchy_pool_0109022c(
    NativeMaterialParameterPool& actual_hierarchy_pool) noexcept;

// Complete B87A90-B87A99. Original ignores allocation-size ECX, replaces it with
// 109022C and tail-jumps B185A0; EAX raw 88h slot. No payload initialization.
void* allocate_static_native_hierarchy_slot_00b87a90();

// Complete CD82D0-CD82E5. Original no arguments, RET, EAX atexit result.
// Initialize via B18340 BEFORE real atexit registration. Registration failure
// adds no rollback. GameNativeResourcePoolProcess invokes this explicit startup
// once for the application's canonical pool and shared allocator-list domain.
int initialize_static_native_hierarchy_pool_00cd82d0();

// Complete CE0ED0-CE0ED9. Original selects 109022C and tail-jumps B18470.
// Frees the existing pool's slabs/table without destroying their payloads.
void destroy_static_native_hierarchy_pool_00ce0ed0() noexcept;

// Complete B17AF0-B17B57, consuming the existing 88h return algorithm.
// Original ECX initialized 38h pool, stack slot, RET 4. That entry also serves
// material F8D3E4 callers; this explicit companion parameter preserves ownership.
// Lock BEFORE reading current slot+84; no field destructor or slab reclamation.
void return_native_hierarchy_pool_slot_00b17af0(
    NativeMaterialParameterPool& actual_initialized_pool, void* actual_slot) noexcept;

// New MSVC Win32 source interfaces. Companion references replace native raw
// ECX/global selection; these are not binary ABI entry replacements. Require
// the existing pool's initialized valid-storage domain. No runtime/game proof.
} // namespace bsp
