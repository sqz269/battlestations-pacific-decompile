#pragma once
#include "bsp/native_render_pointer_arrays.hpp"
#include "bsp/native_render_context.hpp"
#include <array>

namespace bsp {

// Exact 4Ch allocation made by B1DFF0. No implicit initialization/destruction:
// B1D6F0 owns its stores, and the final 10h bytes remain the allocation preimage.
// Arrays and entries borrow raw identities. The binding/model pointers require
// their actual native lifetime operations; this storage has no shared_ptr count.
struct NativeRenderGroupStorage {
    void* binding_00;
    std::int32_t name_length_04;
    char* name_data_08;
    std::int32_t counts_0c[2];
    void* output_entries_14[2];
    void* models_1c[2];
    NativeRenderPointerArrayStorage source_entries_24[2];
    std::array<std::byte, 0x10> untouched_3c;
};
static_assert(sizeof(NativeRenderGroupStorage) == 0x4c);
static_assert(offsetof(NativeRenderGroupStorage, name_length_04) == 0x04);
static_assert(offsetof(NativeRenderGroupStorage, name_data_08) == 0x08);
static_assert(offsetof(NativeRenderGroupStorage, counts_0c) == 0x0c);
static_assert(offsetof(NativeRenderGroupStorage, output_entries_14) == 0x14);
static_assert(offsetof(NativeRenderGroupStorage, models_1c) == 0x1c);
static_assert(offsetof(NativeRenderGroupStorage, source_entries_24) == 0x24);
static_assert(offsetof(NativeRenderGroupStorage, untouched_3c) == 0x3c);

// Original ECX=aligned actual4Ch storage, EAX=same pointer, RET. Establishes
// typed storage without changing its preimage, then runs all native stores.
// Array construction is nonthrowing and allocates nothing. Does not allocate
// the group itself, populate models, or supply a group destructor.
NativeRenderGroupStorage* construct_native_render_group_00b1d6f0(void* actual_storage) noexcept;

// Original ECX=destination pointer cell, EDX=source pointer cell, EAX=destination,
// RET. Capture source/old before publishing new; retain actual incoming+04;
// then release captured old+04/current zero through the canonical owner view.
// Resolver is consulted only at zero. Source cell may alias destination. No
// post-terminal owner access, rollback, or default terminal callback is added.
void** assign_native_instance_binding_00b1ca50(void*& destination,
    const void* source_pointer_cell, NativeRenderActualOwners&);

} // namespace bsp
