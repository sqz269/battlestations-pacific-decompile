#pragma once
#include <cstdint>

namespace bsp {
// Borrow the current original globals; no default bounds or replacement owner.
struct NativeUnitPartCollisionGlobals {
    const volatile std::uint32_t& minimum_seed_00d7a248;
    const volatile std::uint32_t& maximum_seed_00d7a244;
    const volatile std::uint32_t* initial_point_00f87574;
    const volatile double& half_00d7a280;
};
struct NativeUnitPartCollisionCallbacks {
    void* context;
    // Complete BF6713 operation. It can return after repairing actual storage.
    void (*invalid_parameter)(void*);
};

// Actual borrowed raw storage. These operations neither create an ownership
// domain nor reinterpret native vtable addresses as process-callable code.
// 723170: ECX source with bounds+38/+44; stack min,max; RET8. Six x87 copies.
void copy_native_part_bounds_00723170(const void* source, void* minimum, void* maximum) noexcept;
// 98AAB0: ECX shape; stack min,max; RET8. Stores six x87 values at+4/+10.
void set_native_part_shape_bounds_0098aab0(void* shape, const void* minimum,
    const void* maximum) noexcept;
// 98A920: ECX spatial node; stack min,max; RET8. Complete local bounds, extent
// and centre sequence, including separate single-precision spills and reloads.
void set_native_spatial_local_bounds_0098a920(void* node, const void* minimum,
    const void* maximum, const volatile double& half) noexcept;
// 711020: stack destination,source; RET8; ECX unused. Null destination inert.
void copy_native_unit_part_shape_00711020(void* destination, const void* source) noexcept;
// 711460: ECX list; stack next,previous,source; RET0C. Allocates actual30h
// storage through the existing BF681B new-handler allocator, then copies value.
// The caller owns the returned node; this operation does not link it.
void* buy_native_unit_part_shape_node_00711460(void* next, void* previous,
    const void* source);

// Complete 712440..7128B2: ECX existing 1ACh part; no stack args; AL boolean.
// The selected set+3C is an actual checked vector of 8-byte records; its first
// DWORD supplies bounds, its second is matched against rows in part+168.
// Non-base rows (index >=1) route matches to list+194. Others enter list+188
// and publish into the existing ten-slot shape array+D0. Both affect bounds.
// Requires live consistent storage and room in that original inline array.
// No truncation, reset, rollback, null-set default, or synthesized hierarchy.
// New C++ ABI; original fault delivery, FH3 and gameplay remain unproved.
bool build_native_unit_part_collision_00712440(void* actual_part,
    NativeUnitPartCollisionGlobals, NativeUnitPartCollisionCallbacks);
} // namespace bsp
