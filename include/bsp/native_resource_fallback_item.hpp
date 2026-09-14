#pragma once
#include "bsp/native_ref_counted.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
// Borrowed native storage: current profile at00, reference count at04. No
// payload, virtual host interface, allocation, or automatic ownership action.
struct alignas(4) NativeResourceFallbackItemStorage { std::byte bytes[8]; };
static_assert(sizeof(NativeResourceFallbackItemStorage) == 8);

// Complete B86930-B86945: native ECX storage, EAX same, RET. Ordered stores
// CEB130 at00, count1 at04, D631C0 at00. This C++ entry has a new ABI.
void* construct_native_resource_fallback_item_00b86930(void*) noexcept;

// Complete B86990-B869B3: native ECX item, stack flags DWORD (low-byte bit0),
// EAX original pointer, RET4. StampD5C104, call concreteBD30F0, free through
// the existing BF65AC service iff bit0; no count change or scalar null test.
// The returned pointer may already be freed. This C++ entry has a new ABI.
void* scalar_delete_native_resource_fallback_item_00b86990(void*,
    std::uint32_t flags) noexcept;

// Finite D631C0 binding only: original00=BD30E0, original04=B86990. Compose
// with existing invoke_native_ref_counted_delete_00bd30e0, which captures the
// current profile and supplies flags1. Other profiles are binding errors;
// this does not implement type predicates or arbitrary resource-item deletion.
class NativeResourceFallbackDeleteCalls final : public NativeRefCountedDeleteCalls {
public:
    void delete_vslot04(void*, std::uint32_t captured_profile,
        std::uint32_t flags) override;
};
} // namespace bsp
