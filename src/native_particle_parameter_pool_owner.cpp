#include "bsp/native_particle_parameter_pool_owner.hpp"

#include <stdexcept>

namespace bsp {
namespace {
NativeWeakHandlePool* static_parameter_pool_00f8d344;

NativeWeakHandlePool& require_static_parameter_pool() {
    if (!static_parameter_pool_00f8d344) {
        throw std::logic_error(
            "native 00F8D344 particle-parameter pool has no actual storage binding");
    }
    return *static_parameter_pool_00f8d344;
}
} // namespace

void bind_static_native_particle_parameter_pool_00f8d344(
    NativeWeakHandlePool& pool) {
    if (static_parameter_pool_00f8d344 && static_parameter_pool_00f8d344 != &pool) {
        throw std::logic_error(
            "native 00F8D344 particle-parameter pool is already bound");
    }
    static_parameter_pool_00f8d344 = &pool;
}

int initialize_static_native_particle_parameter_pool_00cd78b0(
    NativeWeakPoolAtexit register_atexit) {
    require_static_parameter_pool().initialize_00b004b0();
    return register_atexit(&destroy_static_native_particle_parameter_pool_00ce0be0);
}

void destroy_static_native_particle_parameter_pool_00ce0be0() {
    require_static_parameter_pool().destroy_00b002c0();
}

} // namespace bsp
