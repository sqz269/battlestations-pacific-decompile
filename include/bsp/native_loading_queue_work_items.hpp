#pragma once

#include "bsp/native_string.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native loading queue storage requires MSVC Win32.
#endif

namespace bsp {

// Actual storage, with no host vector, implicit owner, or cached publication:
// pointer/work vector: pointer+0, signed count+4, signed capacity+8 (0Ch).
// work record: owning name length+0/data+4, raw context+8/result+0C (10h).
// loader: pointer vector at+10; job: state+0, stop BYTE+4, work vector+8,
// owning package name+14/+18, raw callback+1C, raw FileBlock+20 (24h).
// References borrow the application's actual string-pool/manager publications.
// No validation, resource release, FileBlock behavior, or thread policy is added.
// Descriptive names are hypotheses. Evidence: NATIVE_LOADING_QUEUE_WORK_ITEMS_BL.md.

// Original ECX actual pointer-vector header, stack signed capacity/count, RET4.
// Reserve clamps to >=1, copies current slots, frees old array, then publishes.
// Resize clears only newly added slots; shrinking does not destroy jobs.
void reserve_native_loading_job_pointers_004fb310(void* actual_vector,
    std::int32_t requested_capacity);
void resize_native_loading_job_pointers_004fb4b0(void* actual_vector,
    std::int32_t requested_count);

// Original ECX actual work-vector header, stack signed index/capacity/count,
// RET4. Names are copied through actual0041DD40 and released through the current
// raw00419CC0/BD1510 domain. Context/results are copied without retain/release.
// Remove requires a valid nonempty vector/index; it adds no native-missing guard.
void remove_native_loading_work_item_00501670(void* actual_vector,
    NativeStringRawPoolContext&, std::int32_t index);
void reserve_native_loading_work_items_005018a0(void* actual_vector,
    NativeStringRawPoolContext&, std::int32_t requested_capacity);
void resize_native_loading_work_items_005019d0(void* actual_vector,
    NativeStringRawPoolContext&, std::int32_t requested_count);

// Original ECX actual24h job, no stack arguments, RET. Frees package name,
// shrinks work records in reverse order and frees their array; does not free
// the outer job or clear its freed-pointer fields. FileBlock/result untouched.
void destroy_native_loading_job_storage_005051a0(void* actual_job,
    NativeStringRawPoolContext&);

// Original ECX actual24h job, stack source8h name header, EAX job, RET4.
// Constructs in place; preserves padding+5..7 and writes callback/FileBlock last.
void* construct_native_loading_job_00505530(void* actual_job,
    NativeStringRawPoolContext&, const void* actual_package_name);

// Original ECX actual20h loader, stack source8h name header, RET4. Allocate24h,
// construct job, grow loader+10 pointer vector only when count==capacity, append.
// Constructor failure frees the outer allocation; later reserve failure does
// not clean the constructed job. This API has no specified return value.
void enqueue_native_loading_job_005055c0(void* actual_loader,
    NativeStringRawPoolContext&, const void* actual_package_name);

// New C++ interfaces, not drop-in thiscall/FH3/SEH replacements. Native DWORD
// arithmetic and signed comparisons are retained. Host CRT allocation/exception
// identity and faults on invalid native storage remain explicit boundaries.

} // namespace bsp
