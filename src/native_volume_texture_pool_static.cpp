#include "bsp/native_volume_texture_pool_static.hpp"

#include <cstdlib>
#include <stdexcept>

namespace bsp {
namespace {
D3D9SurfacePool* actual_volume_pool;

D3D9SurfacePool& require_volume_pool() {
    if (!actual_volume_pool)
        throw std::logic_error("native volume pool requires its actual storage binding");
    return *actual_volume_pool;
}
} // namespace

void bind_static_native_volume_texture_pool_0108dba8(D3D9SurfacePool& pool) {
    if (actual_volume_pool && actual_volume_pool != &pool)
        throw std::logic_error("native volume pool already has a different storage binding");
    actual_volume_pool = &pool;
}

int initialize_static_native_volume_texture_pool_00cd7ba0() {
    require_volume_pool().initialize_00b3ec60();
    return std::atexit(&destroy_static_native_volume_texture_pool_00ce0cc0);
}

void destroy_static_native_volume_texture_pool_00ce0cc0() {
    require_volume_pool().destroy_00b3e2b0();
}

} // namespace bsp
