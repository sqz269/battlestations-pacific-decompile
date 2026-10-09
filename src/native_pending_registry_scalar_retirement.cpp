#include "bsp/native_pending_registry_scalar_retirement.hpp"

#include "bsp/native_pending_registry_base_cleanup.hpp"
#include "bsp/native_tracked_critical_section_release.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>

namespace bsp {
static_assert(sizeof(void*) == 4);

void* retire_native_pending_registry_scalar_00875850(
    void* actual_receiver,
    const volatile std::uint8_t& actual_deleting_flags_byte,
    void* volatile& actual_publication_00f878cc) {
    void* const captured_receiver = actual_receiver;
    *static_cast<volatile std::uint32_t*>(captured_receiver) = 0x00d0dea0u;
    release_native_tracked_critical_section_0041cc80(
        reinterpret_cast<TrackedCriticalSection**>(
            static_cast<std::byte*>(captured_receiver) + 4));
    const bool delete_receiver = (actual_deleting_flags_byte & 1u) != 0;
    cleanup_native_pending_registry_base_008748f0(
        captured_receiver, actual_publication_00f878cc);
    if (delete_receiver) {
        singleton_lifetime_free(captured_receiver);
    }
    return captured_receiver;
}

} // namespace bsp
