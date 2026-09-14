#pragma once
#include "bsp/native_unit_part_construction.hpp"
#include "bsp/native_unit_part_collision.hpp"

namespace bsp {
// Complete 004E6480..004E653B. ECX=existing collision-node storage, RET.
// Copy the current seed bits without x87 conversion; allocate eight raw child
// pointer bytes before initializing the intrusive links. Preserve all other
// bytes, including the primary table. The caller owns the enclosing allocation.
void initialize_native_collision_node_fields_004e6480(void* actual_node,
    const volatile std::uint32_t& minimum_seed,
    const volatile std::uint32_t& maximum_seed);
// Complete 004E6570..004E6587. Install CE89E8, free nonnull children+FC,
// retain the dangling child pointer and every other field. Do not free self.
void destroy_native_collision_node_base_004e6570(void* actual_node) noexcept;

// Complete 007103A0/007103C0 (26 bytes each). ECX unused, RET, EAX=sentinel.
// The canonical allocator owns 30h raw bytes. Only next+0 and previous+4 are
// initialized to self; the 28h value payload is neither constructed nor read.
void* buy_native_unit_part_list_sentinel_007103a0();
void* buy_native_unit_part_shape_sentinel_007103c0();
// Complete 00710870..007108B7 and identical 007108E0..00710927. The constructor
// cleanup entries 00710F90/00710FC0 are tail jumps to these bodies. ECX=list,
// RET. Require a live sentinel at owner+4, even when count+8 is zero. Detach
// nodes before freeing them, capture each next before free, reload the current
// sentinel after free, then free it and null owner+4. Owner+0 stays untouched.
// Values are raw 28h payloads: no pointed object is retained or destroyed.
void destroy_native_unit_part_list_00710870(void* actual_list) noexcept;
void destroy_native_unit_part_shape_list_007108e0(void* actual_list) noexcept;
// Complete 00712B40..00712B7C. Destroy every live 10h row through the existing
// canonical DWORD-vector clear, free the current outer allocation, then zero
// header+4/+8/+C. Preserve header+0 and do not destroy referenced node owners.
void destroy_native_unit_part_group_rows_00712b40(void* actual_header) noexcept;

// Binds recovered base initialization and collision construction to the same
// borrowed globals and validation domain. The inherited storage defaults do
// real allocation/cleanup. Selected-set destruction, render-name binding,
// attachment and final entry production remain required complete providers.
// No copied global values, fallback hierarchy, ownership proxy or native vtable
// invocation is introduced. These are new C++ interfaces, not the native ABI.
class NativeUnitPartStorageBindings : public NativeUnitPartConstructionBindings {
public:
    NativeUnitPartStorageBindings(NativeUnitPartCollisionGlobals globals,
        NativeUnitPartCollisionCallbacks callbacks) noexcept;
    void call_004e6480(void* model) override;
    void call_00712440(void* model) override;
private:
    NativeUnitPartCollisionGlobals globals_;
    NativeUnitPartCollisionCallbacks callbacks_;
};
} // namespace bsp
