#pragma once

#include "bsp/input_settings.hpp"

namespace bsp {

// Native ECX/EDX point to two checked vectors of int32 action codes; RET.
// 0069e860..0069e8f5 checks lengths, then 0069dd80 compares each dword.
// Empty vectors compare equal. The two references may alias. No floats or
// KeyboardInputBinding fields participate in this comparison.
bool keyboard_input_codes_equal_0069e860(const std::vector<std::int32_t>& first,
    const std::vector<std::int32_t>& second) noexcept;

// Native ECX=settings; stack device-name*, input-name*; AL result; RET 8.
// 006aa090..006aa1bf considers only pairs whose SECOND name matches the input
// (equal native string lengths, then case-insensitive comparison). It returns
// true if any matching pair has equal ordered action-code descriptions.
//
// Device and description lookups insert defaults, as native operator[] does.
// Existing table-form rows with case-insensitively equal names append their
// codes. An absent description is represented by a new empty table-form row;
// it creates no input-order label, runtime binding, or reverse flag. String
// arguments may alias settings storage. Existing model supports ASCII names.
// New host interface, not a binary replacement; docs/KEYBOARD_AXIS_PAIRS.md.
bool use_alternate_axis_slots_006aa090(InputSettings& settings,
    const std::string& device_name, const std::string& input_name);

} // namespace bsp
