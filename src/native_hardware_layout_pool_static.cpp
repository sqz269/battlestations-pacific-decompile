#include "bsp/native_hardware_layout_pool_static.hpp"
#include "bsp/native_hardware_layout_pool_lifetime.hpp"
#include "bsp/native_hardware_layout_pool_trim.hpp"

#include <cstdlib>

namespace bsp {
namespace {
void* actual_canonical_pool;
AllocatorListDomain* actual_canonical_list;
} // namespace

void bind_static_native_hardware_layout_pool_0108fe9c(
    void* actual_pool, AllocatorListDomain& actual_list) {
    bind_native_hardware_layout_pool_trim_00d62af0(actual_pool, actual_list);
    actual_canonical_pool = actual_pool;
    actual_canonical_list = &actual_list;
}

int initialize_static_native_hardware_layout_pool_00cd7ca0() {
    initialize_native_hardware_layout_pool_00b604d0(
        actual_canonical_pool, *actual_canonical_list);
    return std::atexit(&destroy_static_native_hardware_layout_pool_00ce0d40);
}

void destroy_static_native_hardware_layout_pool_00ce0d40() noexcept {
    destroy_native_hardware_layout_pool_00b60270(
        actual_canonical_pool, *actual_canonical_list);
}

} // namespace bsp
