#pragma once

#include "bsp/native_string_vector.hpp"

namespace bsp {

// Full 005EFBA0..005EFC15[118]: ECX actual0Ch vector, one actual8h key
// pointer, EAX signed index/-1, RET4. Capture count then data and wrapping
// unsigned end once. Match equal stored lengths, empty/empty without data
// access, otherwise retained CRT _stricmp. On a match, calculate the index
// against CURRENT vector data. Key may be null when the range is empty.
// New source interface; no STL binding or original x86 ABI replacement.
std::int32_t find_native_string_vector_005efba0(
    const NativeStringVectorStorage& actual_vector, const void* actual_key);

} // namespace bsp
