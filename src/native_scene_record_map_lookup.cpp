#include "bsp/native_scene_record_map_lookup.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_vfs_date_leaf_providers.hpp"

namespace bsp {
namespace {

static_assert(sizeof(std::uintptr_t) == 4);

std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}

void* at_offset(const void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(address(storage) + offset);
}

std::uint32_t word(const void* storage, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at_offset(storage, offset));
}

void put(void* storage, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at_offset(storage, offset)) = value;
}

void* pointer(const void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(word(storage, offset));
}

bool sentinel(const void* node) noexcept {
    return *static_cast<const volatile unsigned char*>(at_offset(node, 0x19)) != 0;
}

} // namespace

void* __fastcall lower_bound_native_scene_record_map_00468140(
    void* actual_map, std::uint32_t, const void* actual_key_header) {
    auto* candidate = pointer(actual_map, 4);
    auto* node = pointer(candidate, 4);
    while (!sentinel(node)) {
        if (less_native_string_headers_00443d00(at_offset(node, 0x0c), actual_key_header)) {
            node = pointer(node, 8);
        } else {
            candidate = node;
            node = pointer(node, 0);
        }
    }
    return candidate;
}

void* __fastcall find_native_scene_record_map_00468cd0(void* actual_map,
    std::uint32_t, void* actual_iterator_output, const void* actual_key_header) {
    auto* node = lower_bound_native_scene_record_map_00468140(actual_map, 0,
        actual_key_header);
    if (actual_map == nullptr)
        invoke_native_invalid_parameter_00bf6713();

    if (node == pointer(actual_map, 4) ||
        less_native_string_headers_00443d00(actual_key_header, at_offset(node, 0x0c))) {
        node = pointer(actual_map, 4);
    }

    const auto captured_owner = address(actual_map);
    const auto captured_node = address(node);
    put(actual_iterator_output, 0, captured_owner);
    put(actual_iterator_output, 4, captured_node);
    return actual_iterator_output;
}

} // namespace bsp
