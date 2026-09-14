#include "bsp/native_resource_hierarchy_pool.hpp"
#include "bsp/native_material_pools.hpp"

#include <cstdlib>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hierarchy pool wrappers require MSVC Win32.
#endif

namespace bsp {
namespace {
NativeMaterialParameterPool* canonical_hierarchy_pool;
} // namespace

void bind_static_native_hierarchy_pool_0109022c(
    NativeMaterialParameterPool& pool) noexcept {
    canonical_hierarchy_pool = &pool;
}

void* allocate_static_native_hierarchy_slot_00b87a90() {
    return canonical_hierarchy_pool->allocate_slot_00b185a0();
}

int initialize_static_native_hierarchy_pool_00cd82d0() {
    canonical_hierarchy_pool->initialize_00b18340();
    return std::atexit(&destroy_static_native_hierarchy_pool_00ce0ed0);
}

void destroy_static_native_hierarchy_pool_00ce0ed0() noexcept {
    canonical_hierarchy_pool->destroy_00b18470();
}

void return_native_hierarchy_pool_slot_00b17af0(
    NativeMaterialParameterPool& pool, void* slot) noexcept {
    pool.return_slot_00b193fa_fragment(slot);
}
} // namespace bsp
