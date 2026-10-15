#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual string vector insertion requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete00450180..00450208. Native fastcall: ECX actual10h vector,
// EDX unused; stack output/owner/position/value, RET10h, EAX=output. Source
// retains these physical inputs and adds stacked actual raw-pool context.
// No local EH frame, extra catch, owner/lifetime model or host callback.
//
// Capture original begin. Null begin or SAR3(end-begin)==0 gives offset0,
// skipping the preliminary range/owner checks and position subtraction.
// Otherwise validate captured range, then captured owner, reporting/continuing
// through the accepted invalid-parameter service; compute SAR3(position-begin)
// from the ORIGINAL begin. Invoke actual0044F410 with count1 and EDX=value bits.
// After it returns, capture new begin, compare with CURRENT end, report/continue;
// retain that begin for the wrapped result address. Compare result with fresh
// end and, only if result<=end, fresh begin, report/continue. Finally store
// output+4 position BEFORE output+0 owner, and return the original output.
//
// Inputs/storage must satisfy the accepted child/raw-header services. No new
// alignment, iterator, overlap, capacity, null or output-alias validation is
// added. Explicit Source context/by-value inputs and current C++/CRT behavior
// do not reproduce native stack-private aliases, original ABI, fault addresses,
// handler/exception identity or FH3/SEH semantics. Existing qualified append
// fragments remain separate. No new-function native/gameplay proof is claimed.
void* __fastcall insert_native_string_header_vector_one_00450180(void* header,
    std::uint32_t unused_edx, void* output, const void* owner, void* position,
    const void* value, NativeStringRawPoolContext& strings);
} // namespace bsp
