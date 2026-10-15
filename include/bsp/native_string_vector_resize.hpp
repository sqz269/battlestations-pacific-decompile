#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual string vector resize requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete0049E050 normal body plus00C63540 owned-parameter unwind scope.
// Native fastcall: ECX actual10h vector, EDX unused; stack unsigned requested
// count, owned fill length, owned fill data; RET0Ch; no semantic result.
// Source keeps those three scalar words, adds the actual raw-pool context and
// places the incoming8h owned header bits in a canonical private NativeString.
// No additional allocation/deep copy is performed for that incoming header.
//
// The two fill words TRANSFER ownership of a separately prepared by-value
// argument, including for shrink/no-change/empty-vector paths. A nonnull data
// block must be accepted by the actual pool/manager/context and length+1 return
// policy. This is not a borrowed fill prototype: do not separately release its
// block after this call. A caller needing to retain a prototype must prepare
// an independent owned copy before invoking this interface. Null data skips
// the cleanup length read/getter/return, even with a nonzero length word.
//
// Capture vector begin before arming owned-parameter cleanup. Compare initial
// SAR32(end-begin,3) size with requested; null begin yields0. Grow recomputes
// size using the SAME captured begin and fresh end, then captures end, reports
// begin>end and continues if the handler returns. Delegate full actual
//0044F410 with count=requested-recomputed size and the captured insertion end.
// Shrink rereads end/size against captured begin; skip when null begin or
// requested>=fresh size. Otherwise report captured begin>captured end; capture
// CURRENT begin and report it>fresh end; compute target from that captured begin;
// report target>fresh end OR target<fresh begin with short-circuit read order.
// Delegate exact erase using captured target/end and a private two-DWORD result.
// No new guard, clamp, owner validation, rollback or generic vector is added.
//
// All later current C++ failures clean the CURRENT owned fill header through
// actual0041DD20 then rethrow. Normal cleanup disables that scope before the
// same destructor, so cleanup failure is not retried. A cleanup exception stops
// subsequent work and escapes in the Source domain; native double-exception,
// FH3/SEH, fault/private-stack/CRT identity and original binary ABI are unproved.
// Extra context/private parameter storage also qualify native frame aliases.
// Earlier NativeStringStorage resize/append projections and fixtures remain
// separate. Accepted current-CRT overlap-copy/zero-byte policy is inherited;
// no new-function native differential or gameplay validation is claimed.
void __fastcall resize_native_string_header_vector_0049e050(void* header,
    std::uint32_t unused_edx, std::uint32_t requested_count,
    std::uint32_t owned_fill_length, void* owned_fill_data,
    NativeStringRawPoolContext& strings);

} // namespace bsp
