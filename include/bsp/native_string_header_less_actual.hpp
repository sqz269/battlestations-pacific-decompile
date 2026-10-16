#pragma once

#include <cstdint>

namespace bsp {

class NativeCrtStricmpCallees;

// 00443D00: two actual 8-byte LENGTH0/DATA4 headers; empty < nonempty.
// This additional Source domain calls the published actual BF7FBF service with
// the caller's actual locale-change cell and existing provided callees. The
// older two-argument CurrentCRT API retains its separate contract.
// Native: two stack words, RET8, AL boolean; upper EAX is incidental.
// Source: four stack words, RET10h. This is not the Original binary ABI.
// Empty branches do not read either data word, the cell, or the services.
// Header storage must admit the actual ordered volatile DWORD reads. Private
// argument/frame aliases, flags, faults, FH3/SEH, CRT ownership and game/runtime
// equivalence are unproved. Exceptions from the actual service propagate.
bool __stdcall less_native_string_headers_with_callees_00443d00(
    const void* actual_left_header, const void* actual_right_header,
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees& actual_callees);

} // namespace bsp
