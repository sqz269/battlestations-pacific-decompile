#pragma once

#include "bsp/native_crt_libm_callback_registration.hpp"
#include "bsp/native_crt_pointer_decode.hpp"

#include <cstdint>

namespace bsp {

// Whole physical C17653..C17717 (197B/66 instructions), including its actual
// filter/handler and returning-Watson continuation. Descriptive Source name;
// native library name: ___crtInitCritSecAndSpinCount.
//
// actual_parameters8: +0 = live pointer DWORD [native EBP+8],
//                     +4 = live spin DWORD    [native EBP+C].
// actual_private12:   +0 = code DWORD         [native EBP-24],
//                     +4 = result DWORD       [native EBP-20],
//                     +8 = platform DWORD     [native EBP-1C].
// Only platform is initially zeroed. Require valid reached raw DWORD accesses;
// result's handler path performs an actual read/AND-zero/write even if its old
// bytes have not been initialized. The two exposed backings may alias reached
// payload at the existing provider contracts; their values are never snapshots.
//
// Added reference/context/binding metadata must stay stable and disjoint from
// the raw cells, reached payload and compiler bookkeeping. The actual cache
// referent and canonical words to which a context refers remain mutable and
// retain their admitted payload aliases.
// Existing codec/TLS/module/owning-errno/invalid-service admissions apply; this
// entry creates no heap, lock, TLS/PTD, cache or registered-handler owner.
//
// MSVC Win32 __try/__except protects only late spin/pointer reads, the selected
// real stdcall initializer and result publication. Its exact C0000017 filter
// publishes code, then the handler rereads that cell before SetLastError(8).
// Decoder/getter/Watson/module/encoder/cache work remains outside that region.
// Compiler SEH/ExceptionPointers/stack state is separate from the three exposed
// native payload cells. This ordinary interface supplies no Original SEH4,
// private-frame/home alias, fault/code address, register/flag or binary ABI
// identity. The genuine Source fallback has its relocated callable address.
std::int32_t initialize_native_crt_critical_section_dispatch_00c17653(
    const void* actual_parameters8,
    void* actual_private12,
    volatile std::uint32_t& actual_encoded_cache_0109e454,
    const NativeCrtPointerDecodeContext& decode,
    const NativeCrtPointerEncodeContext& encode,
    const NativeCrtPointerDecodeSupportContext& support);

} // namespace bsp
