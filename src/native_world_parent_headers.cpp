#include "bsp/native_world_parent_headers.hpp"

#include "bsp/native_parent_header_destructor_callback.hpp"
#include "bsp/native_parent_list_header_storage.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(noexcept(initialize_native_parent_list_header_storage_004b7ec0(nullptr, 0)));
static_assert(noexcept(destroy_native_parent_header_004c2d30(nullptr, 0)));
static_assert(native_world_parent_header_stride == native_parent_list_header_storage_bytes);
static_assert(native_world_parent_array_offset
    + native_world_parent_header_count * native_world_parent_header_stride == 0x4a4);
static_assert(native_world_parent_root_offset + native_world_parent_header_stride
    == native_world_parent_array_offset);

void initialize_native_world_parent_headers(void* actual_world_storage) noexcept {
    auto* const world = static_cast<unsigned char*>(actual_world_storage);
    initialize_native_parent_list_header_storage_004b7ec0(
        world + native_world_parent_root_offset, 0);
    for (std::size_t index = 0; index < native_world_parent_header_count; ++index) {
        initialize_native_parent_list_header_storage_004b7ec0(
            world + native_world_parent_array_offset
                + index * native_world_parent_header_stride, 0);
    }
}

void clear_native_world_parent_headers(void* actual_world_storage) noexcept {
    auto* const world = static_cast<unsigned char*>(actual_world_storage);
    for (std::size_t remaining = native_world_parent_header_count; remaining != 0; --remaining) {
        destroy_native_parent_header_004c2d30(
            world + native_world_parent_array_offset
                + (remaining - 1) * native_world_parent_header_stride, 0);
    }
    destroy_native_parent_header_004c2d30(world + native_world_parent_root_offset, 0);
}

} // namespace bsp
