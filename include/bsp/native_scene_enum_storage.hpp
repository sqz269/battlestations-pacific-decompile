#pragma once
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {
// Storage written by 004D3069 and 008F4DD0; no construction or ownership here.
struct NativeSceneEnumMapStorage {
    std::uint32_t profile;
    std::uint32_t count;
    void* buckets[64];
};
struct NativeSceneEnumRegistryStorage {
    std::uint32_t profile;
    NativeSceneEnumMapStorage map;
};
struct NativeSceneEnumOwnerStorage {
    std::uint32_t profile;
    NativeSceneEnumMapStorage map;
    std::int32_t minimum;
    std::int32_t maximum;
    NativeString type;
    char name[0x80];
};
static_assert(sizeof(void*) == 4);
static_assert(std::is_standard_layout_v<NativeSceneEnumOwnerStorage>);
static_assert(sizeof(NativeSceneEnumMapStorage) == 0x108);
static_assert(offsetof(NativeSceneEnumMapStorage, count) == 4);
static_assert(offsetof(NativeSceneEnumMapStorage, buckets) == 8);
static_assert(sizeof(NativeSceneEnumRegistryStorage) == 0x10c);
static_assert(offsetof(NativeSceneEnumRegistryStorage, map) == 4);
static_assert(sizeof(NativeSceneEnumOwnerStorage) == 0x19c);
static_assert(offsetof(NativeSceneEnumOwnerStorage, map) == 4);
static_assert(offsetof(NativeSceneEnumOwnerStorage, minimum) == 0x10c);
static_assert(offsetof(NativeSceneEnumOwnerStorage, maximum) == 0x110);
static_assert(offsetof(NativeSceneEnumOwnerStorage, type) == 0x114);
static_assert(offsetof(NativeSceneEnumOwnerStorage, name) == 0x11c);
}
