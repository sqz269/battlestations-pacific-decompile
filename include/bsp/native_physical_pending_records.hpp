#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical pending records require MSVC Win32.
#endif

namespace bsp {
class NativeStringStorage;

// Complete BF3880 normal storage behavior: native ECX=38h record, RET0.
// Release +28h string, then the current +20h string; retain both headers.
// The existing noexcept storage release cannot project a throwing lazy getter
// and its native FH3 first-string cleanup. See the evidence document.
void destroy_native_physical_pending_names_00bf3880(void* actual_record,
    NativeStringStorage&) noexcept;

// Complete BF3C10: ECX=destination, stack source, EAX=destination, RET4.
// Copy scalar words and construct owned names at +20/+28. Leave +14/+34
// untouched. Zero each name before its identity guard. Failure constructing
// the second name destroys only the current first name; no first-name rollback.
void* copy_construct_native_physical_pending_record_00bf3c10(
    void* actual_destination, const void* actual_source, NativeStringStorage&);

// Actual queue: backing DWORD+0, signed count+4, signed capacity+8, stride38h.
// Complete BF3DA0: clamp capacity>=1; DWORD capacity*38h allocation, copy live
// records, destroy old names, free current backing, publish backing/capacity.
// No replacement/finished-record rollback: native unwind calls RET-only401130.
void reserve_native_physical_pending_records_00bf3da0(void* actual_queue,
    std::int32_t capacity, NativeStringStorage&);

// Complete BF3ED0, including signed comparisons and wrapping arithmetic.
// Grow zeroes all record words except +14/+34; shrink decrements live count
// before each name destruction, then publishes requested count. No bounds guard.
void resize_native_physical_pending_records_00bf3ed0(void* actual_queue,
    std::int32_t count, NativeStringStorage&);

// Complete BF4B80[23], including raw returning tail BF4B92..96: resize0 then
// free current backing. Retain pointer/capacity. Native ECX=queue, RET0.
void destroy_native_physical_pending_records_00bf4b80(void* actual_queue,
    NativeStringStorage&);

// Explicit storage changes these C++ interfaces from the original ABI.
// No handle close, I/O cancellation or staging/OVERLAPPED release is performed
// by these bodies. Names are hypotheses; docs/NATIVE_PHYSICAL_PENDING_RECORDS.md
// records all ABI, EH, allocator and runtime limits.
} // namespace bsp
