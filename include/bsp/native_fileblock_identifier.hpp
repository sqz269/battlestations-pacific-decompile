#pragma once

#include "bsp/native_string_vector.hpp"

#include <cstdint>

namespace bsp {

// Full BD20A0..BD216C. Original ECX source8h header, DL delimiter, stack
// destination0Ch vector, RET4. Append deep copies of nonempty spans without
// clearing the destination. Scan stored length (including embedded NUL),
// reload source length after each temporary return. Only a completed substring
// is armed for caller unwind; append/reserve retain their native failure limits.
void split_native_string_nonempty_on_byte_00bd20a0(const void* actual_source,
    std::uint8_t delimiter, NativeStringVectorStorage& destination,
    NativeStringStorage& strings);

// Full BDF950..BDFD7F. Original ECX actual FileBlock owner, no stack arguments,
// RET, no stable result contract. Operates on actual length/data at +14/+18.
// Exact invalid-byte replacement, native atol != 0 abbreviation and fallback
// construction order; neither branch guarantees a 32-byte result. Reuses the
// actual native string/vector storage and armed FH3 cleanup schedule.
// The explicit-storage C++ ABI is new. NativeStringStorage's noexcept returns,
// host CRT/locale, and no arbitrary aliases into compiler locals remain limits;
// no native FH3/SEH identity or game validation is asserted.
void prepare_native_fileblock_identifier_00bdf950(void* actual_fileblock,
    NativeStringStorage& strings);

} // namespace bsp
