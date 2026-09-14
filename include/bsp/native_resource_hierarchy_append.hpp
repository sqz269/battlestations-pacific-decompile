#pragma once

#include <cstdint>

namespace bsp {
// Full 00B87AE0..00B87B1A. Original: ECX resource, one pointer word on stack,
// RET4. This C++ interface is not a binary ABI replacement.
void append_native_hierarchy_item_00b87ae0(void* resource,
    std::uint32_t hierarchy_item_word);
} // namespace bsp
