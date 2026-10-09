#include "bsp/native_pending_registry_base_cleanup.hpp"

#include "bsp/native_render_service_base.hpp"

namespace bsp {

void cleanup_native_pending_registry_base_008748f0(
    void* actual_receiver,
    void* volatile& actual_registry_publication_00f878cc) {
    actual_registry_publication_00f878cc = nullptr;
    destroy_native_generic_singleton_base_00412430(actual_receiver);
}

} // namespace bsp
