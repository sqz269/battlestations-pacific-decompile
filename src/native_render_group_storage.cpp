#include "bsp/native_render_group_storage.hpp"
#include <cstring>
#include <new>

namespace bsp {

NativeRenderGroupStorage* construct_native_render_group_00b1d6f0(void* raw) noexcept {
    auto* group = ::new (raw) NativeRenderGroupStorage;
    group->binding_00 = nullptr;
    group->name_length_04 = 0;
    group->name_data_08 = nullptr;
    initialize_native_instance_entry_pointers_00b1c4f0(group->source_entries_24[0]);
    initialize_native_instance_entry_pointers_00b1c4f0(group->source_entries_24[1]);
    group->models_1c[1] = nullptr;
    group->models_1c[0] = nullptr;
    group->counts_0c[1] = 0;
    group->counts_0c[0] = 0;
    group->output_entries_14[1] = nullptr;
    group->output_entries_14[0] = nullptr;
    return group;
}

void** assign_native_instance_binding_00b1ca50(void*& destination,
    const void* source_pointer_cell, NativeRenderActualOwners& owners) {
    void* incoming;
    std::memcpy(&incoming, source_pointer_cell, sizeof(incoming));
    void* const previous = destination;
    if (previous != incoming) {
        destination = incoming;
        if (incoming) {
            auto* const count = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
                static_cast<std::byte*>(incoming) + 4));
            count->fetch_add(1, std::memory_order_seq_cst);
        }
        if (previous) release_native_render_actual_owner(owners, previous);
    }
    return &destination;
}

} // namespace bsp
