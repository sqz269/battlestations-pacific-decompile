#pragma once
#include <cstdint>

namespace bsp {
// Whole ACEC40[ACEC40,ACEC7B),59B/24 starts. Native ECX raw vector header,
// stack DWORD index, EAX raw 2Ch record address, RET4. New C++ cdecl value API.
// Caller retains a live aligned header through a returning Source CRT handler.
// First capture current+04; null skips+08. Validate unsigned index against
// signed wrapped(end-begin)/44 bits; invalid input enters the genuine BF6713
// Source gateway once. On return, reload CURRENT+04 and form wrapped index44
// address without recheck. No proxy/capacity/payload read or ownership effect.
// The returned address need not designate valid storage after a returning
// handler; this function never dereferences it. Native stack/fault/flag/CRT ABI,
// Original placement and caller adoption are not established by this Source.
void* native_gui_state_vector_record_00acec40(
    const void* actual_vector, std::uint32_t index);
} // namespace bsp
