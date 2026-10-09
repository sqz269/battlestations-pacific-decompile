#include "bsp/native_pending_registry_constructor.hpp"

#include "bsp/native_pending_registry_base_cleanup.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
static_assert(sizeof(void*) == 4);

void* construct_native_pending_registry_00874bc0(
    void* actual_receiver,
    void* volatile& actual_registry_publication_00f878cc) {
    void* const captured_receiver = actual_receiver;
    try {
        *static_cast<volatile std::uint32_t*>(captured_receiver) = 0x00d0dea0u;
        TrackedCriticalSection* const section =
            create_native_tracked_critical_section_00bd1860();
        *reinterpret_cast<TrackedCriticalSection* volatile*>(
            static_cast<std::byte*>(captured_receiver) + 4) = section;
    } catch (...) {
        cleanup_native_pending_registry_base_008748f0(
            captured_receiver, actual_registry_publication_00f878cc);
        throw;
    }
    return captured_receiver;
}

} // namespace bsp
