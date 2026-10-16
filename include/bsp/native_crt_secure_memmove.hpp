#pragma once

#include <cstdint>

namespace bsp {
class NativeCrtStricmpCallees;

// Whole _memmove_s at00BF67A7: 91B/43 instructions, native cdecl(D,Dmax,S,n),
// plain RET and errno_t EAX. This Source cdecl adds two explicit borrowed inputs;
// its caller removes24B rather than the native16B. No native entry/frame alias,
// register/flags, fault/SEH/FH3, heap/runtime owner or gameplay identity is claimed.
//
// Supply the existing real error-callee domain and actual stable canonical
// 0109EEA4 DWORD. Only errno_cell and five-zero invalid_parameter are reached;
// the interface's locale method is unused. No concrete errno/TLS/handler binding,
// default adapter, SDK fallback, feature initializer or CRT owner is supplied.
// Zero count returns0 without buffer/service/feature access. Errors write the
// actual volatile errno cell before the handler; only its normal return produces
// the cached error. Valid movement inherits native_crt_memmove.hpp's DF/extents,
// active-frame exclusion and actual vector-provider preconditions. Reached
// service failures propagate, with no catch, buffer clearing or rollback.
std::int32_t __cdecl move_native_crt_secure_bytes_00bf67a7(
    void* destination, std::uint32_t destination_bytes, const void* source,
    std::uint32_t byte_count,
    const volatile std::uint32_t& actual_feature_word_0109eea4,
    NativeCrtStricmpCallees& actual_error_callees);
} // namespace bsp
