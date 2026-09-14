#pragma once
#include <cstdint>
namespace bsp {
struct NativeStringRawPoolContext;

// Actual10h work records: owning name0/4, borrowed context8/resultC.
// Original ECX record; RET. Return only the captured nonnull name allocation
// through current raw pool; leave every record word unchanged.
void destroy_native_loading_work_item_004fdc00(void*, NativeStringRawPoolContext&);

// Original ECX output; stack BY-VALUE name length/data plus context; EAX output,
// RET0C. The explicit0Ch argument-storage pointer represents those three words.
// Capture length/data before clearing output. Copy the key, read current third
// argument for context8, set resultC=0, then return captured parameter data with
// captured length+1. On copy failure only CURRENT by-value name is unwound.
// Normal getter failure occurs after that cleanup is disarmed; no retry.
void* construct_native_loading_work_item_004fe700(void* actual_output,
    void* actual_consumed_arguments_0c, NativeStringRawPoolContext&);

// Original ECX output, stack source10h record, EAX output, RET4. Clear name even
// on identity; copy current name, then context8 and resultC in native order.
// No local cleanup for a failed construction, reference retain or old release.
void* copy_construct_native_loading_work_item_00501720(void* actual_output,
    const void* actual_source, NativeStringRawPoolContext&);

// Original ECX actual0Ch vector, stack source record, RET4. Equality of count
// and capacity triggers reserve(max(signed wrap(2*capacity),1)). Compute current
// destination; nonzero constructs there, then increment CURRENT count. Native
// failure action reads current vector fields before no-op placement delete.
void append_native_loading_work_item_005048f0(void* actual_vector,
    const void* actual_source, NativeStringRawPoolContext&);

// Original ECX loader20h, stack name header/context, RET8. Test only initial
// last job stop BYTE; no empty-queue guard. Deep-copy the by-value argument,
// construct a temporary record, then reload CURRENT array/count/last job for
// append. The reselected job is not checked for stop again. Destroy the
// completed temporary name normally and on append failure; results borrowed.
void submit_native_loading_work_item_00504d20(void* actual_loader,
    const void* actual_name, void* callback_context, NativeStringRawPoolContext&);

// New C++ interfaces over actual storage. Original native stack/EH aliasing,
// CRT/FH3/SEH identity, worker execution and production wiring are not supplied.
} // namespace bsp
