#include "bsp/native_allocator_failure_object_constructor.hpp"

#include "bsp/native_allocator_base_message_constructor.hpp"

#include <cstdint>

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

void* construct_native_allocator_failure_object_00bf6802(
    void* actual_raw12_receiver,
    const void* actual_pointer_slot_address) {
    void* const captured_receiver = actual_raw12_receiver;
    construct_native_allocator_base_message_00bf638e(
        captured_receiver, 0u, actual_pointer_slot_address, 1u);

    // 00BF6811: retain the captured receiver, not the child's return value.
    *static_cast<volatile std::uint32_t*>(captured_receiver) = 0x00d6923cu;
    return captured_receiver;
}

} // namespace bsp
