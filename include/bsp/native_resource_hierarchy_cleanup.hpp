#pragma once

#include <cstdint>

namespace bsp {
class NativeMaterialParameterPool;
struct NativeStringRawPoolContext;

// Complete B88320-B8833F. Original ECX=84h payload in an 88h pool slot,
// stack flags, RET4,
// EAX=the original record. This source interface borrows the actual string
// context and initialized 109022C hierarchy pool explicitly.
void* cleanup_native_hierarchy_record_00b88320(
    void* actual_record, std::uint32_t flags,
    NativeStringRawPoolContext& actual_strings,
    NativeMaterialParameterPool& actual_hierarchy_pool);
} // namespace bsp
