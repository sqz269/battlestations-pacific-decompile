#include "bsp/native_unit_group_lookup.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "The borrowed native group layout is Win32.");

std::int32_t captured_count(const unsigned char* group) noexcept {
    return *reinterpret_cast<const volatile std::int32_t*>(group + 0x4f8);
}

std::uint32_t entity_at(const unsigned char* record) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(record);
}
} // namespace

std::int32_t native_unit_group_member_index_0070d030(
    const void* actual_group, const void* actual_entity) noexcept {
    const auto* const group = static_cast<const unsigned char*>(actual_group);
    const std::int32_t count = captured_count(group);
    const auto identity = reinterpret_cast<std::uintptr_t>(actual_entity);
    const auto* record = group + 0x18;
    for (std::int32_t index = 0; index < count; ++index, record += 0x34) {
        if (entity_at(record) == identity) return index;
    }
    return -1;
}

void* native_unit_group_member_record_0070d080(
    void* actual_group, const void* actual_entity) noexcept {
    auto* const group = static_cast<unsigned char*>(actual_group);
    const std::int32_t count = captured_count(group);
    const auto identity = reinterpret_cast<std::uintptr_t>(actual_entity);
    auto* record = group + 0x18;
    for (std::int32_t index = 0; index < count; ++index, record += 0x34) {
        if (entity_at(record) == identity) return record;
    }
    return nullptr;
}
} // namespace bsp
