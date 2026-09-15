#include "bsp/native_resource_hierarchy_cleanup.hpp"
#include "bsp/native_resource_hierarchy_fields.hpp"
#include "bsp/native_resource_hierarchy_pool.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native hierarchy cleanup requires MSVC Win32.
#endif

namespace bsp {
void* cleanup_native_hierarchy_record_00b88320(
    void* actual_record, std::uint32_t flags,
    NativeStringRawPoolContext& actual_strings,
    NativeMaterialParameterPool& actual_hierarchy_pool) {
    destroy_native_hierarchy_fields_00b88180(actual_record, actual_strings);
    if ((flags & 1U) != 0U) {
        return_native_hierarchy_pool_slot_00b17af0(actual_hierarchy_pool, actual_record);
    }
    return actual_record;
}
} // namespace bsp
