#include "bsp/native_cube_texture_pool_static.hpp"
#include "bsp/native_cube_texture_pool_lifetime.hpp"
#include "bsp/native_cube_texture_pool_trim.hpp"

#include <cstdlib>

namespace bsp {
namespace {
void* actual_canonical_pool;
AllocatorListDomain* actual_canonical_list;
} // namespace

void bind_static_native_cube_texture_pool_0108db70(
    void* actual_pool, AllocatorListDomain& actual_list) {
    bind_native_cube_texture_pool_trim_00d61944(actual_pool, actual_list);
    actual_canonical_pool = actual_pool;
    actual_canonical_list = &actual_list;
}

int initialize_static_native_cube_texture_pool_00cd7b80() {
    initialize_native_cube_texture_pool_00b3f090(
        actual_canonical_pool, *actual_canonical_list);
    return std::atexit(&destroy_static_native_cube_texture_pool_00ce0cb0);
}

void destroy_static_native_cube_texture_pool_00ce0cb0() noexcept {
    destroy_native_cube_texture_pool_00b3e5b0(
        actual_canonical_pool, *actual_canonical_list);
}

} // namespace bsp
