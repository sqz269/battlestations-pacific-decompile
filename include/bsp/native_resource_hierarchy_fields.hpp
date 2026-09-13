#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Complete B7D640-B7D69E. Native ECX raw0Ch numeric-reference header;
// signed capacity stack, RET4. Header=data/count/capacity DWORDs, values4 bytes.
// Minimum capacity8, DWORD allocation/address arithmetic, current header reloads.
// Preserve count/unused slots; publish replacement data/capacity AFTER old free.
void reserve_native_hierarchy_reference_array_00b7d640(
    void* actual_header, std::int32_t requested_capacity);

// Complete B7D7D0-B7D81F. Native ECX header; signed count stack, RET4.
// Grow by zeroing computed nonnull slots; shrink count without touching values.
// Negative counts/capacities are not clamped. Reserve only above current capacity.
void resize_native_hierarchy_reference_array_00b7d7d0(
    void* actual_header, std::int32_t requested_count);

// Complete B88180-B881F2, plus its name-only FH3 action CC2570. Native ECX
// actual84h hierarchy payload, no stack args, RET. Resize0/free numeric storage
// at+4C, then return current name+4/+8 through the actual raw string-pool context.
// Name cleanup is armed before resize and disarmed before normal name return.
// Negative capacity can allocate32 bytes during cleanup. Headers remain stale;
// no pointee work, pool-slot return, parent traversal or refcount operation.
void destroy_native_hierarchy_fields_00b88180(
    void* actual_record, NativeStringRawPoolContext& actual_strings);

// New MSVC Win32 source interfaces, not original ECX/RET ABI replacements.
// Require valid native readable/writable storage and the application's actual
// string publication/gate/lifetime cells. C++ exception cleanup is represented;
// relocated native FH3, asynchronous faults and gameplay equivalence are unproven.
} // namespace bsp
