#pragma once

#include "bsp/native_string.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

// The payload constructed by 00A9F460, before the map's links at +10h/+14h.
// The key is owned; the two value words are initially zeroed by that body.
struct NativeLocaleStringRecord {
    NativeString key;
    std::uint32_t value_length;
    char16_t* value_data;
};
static_assert(offsetof(NativeLocaleStringRecord, key) == 0);
static_assert(offsetof(NativeLocaleStringRecord, value_length) == 8);
static_assert(offsetof(NativeLocaleStringRecord, value_data) == 12);
static_assert(sizeof(NativeLocaleStringRecord) == 16);

} // namespace bsp
