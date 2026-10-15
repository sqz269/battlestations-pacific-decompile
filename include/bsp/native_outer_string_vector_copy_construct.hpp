#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// Complete0055B150 ordinary body and0055B216 cleanup scope on actual16B
// outer checked-vector headers containing actual16B inner checked headers.
// Native ECX destination, unused EDX, stacked source, RET4/EAX destination.
// Source retains those inputs and adds actual raw-pool context (different ABI).
// Count is captured from source before destination+4/+8/+C are zeroed; +0 is
// untouched. Zero count skips allocation, validation and element access. No
// self guard: nonempty source==destination preserves native clearing/allocation
// and empty-copy effects, including loss of old backing ownership.
// Nonzero count uses actual16B allocator/owning length-throw services, ordered
// begin/end/cap publication, current source validations and actual nested copy.
// Only after source-end capture and first begin comparison is cleanup armed.
// Its C++ catch tidies CURRENT destination, then rethrows; allocation/length
// failures before that point do not enter this catch. Source child exceptions
// and current CRT behavior do not reproduce native FH3/SEH/RTTI or fault identity.
// Requires admitted raw storage, child ownership/pool/CRT contracts and excludes
// aliases to native/Source private frames. No binary/gameplay claim.
void* __fastcall construct_native_outer_string_vector_copy_0055b150(
    void* destination, std::uint32_t unused_edx, const void* source,
    NativeStringRawPoolContext& strings);
} // namespace bsp
