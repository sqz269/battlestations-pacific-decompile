#include "bsp/native_gui_group_factory.hpp"
#include "bsp/native_gui_group_default.hpp"
#include "bsp/native_gui_group_storage.hpp"
#include <exception>

namespace bsp {
void* construct_native_gui_group_copy_00ac6fa0(void* destination,
    const void* source, NativeGuiWidgetCopyContext& context,
    NativeGuiWidgetCopyAcquired& acquired) {
    construct_native_gui_widget_copy_00aa9520(destination, source, context, acquired);
    *static_cast<volatile std::uint32_t*>(destination) = 0x00d5cb80u;
    return destination;
}

void* create_native_gui_group_00aa12f0(const void* source,
    const volatile std::uint32_t& one, NativeGuiWidgetCopyContext& context,
    NativeGuiWidgetCopyAcquired& acquired) {
    // Source is captured before AC76D0; no source payload read precedes it.
    const void* const captured_source = source;
    void* const slot = allocate_native_gui_group_slot_00ac76d0();
    if (!slot) return nullptr;
    try {
        if (captured_source)
            return construct_native_gui_group_copy_00ac6fa0(
                slot, captured_source, context, acquired);
        return construct_native_gui_group_default_00ac6f50(slot, one);
    } catch (...) {
        // Both native unwind states return the captured slot. They do not
        // erase the caller's model-clone diagnostics or destroy the payload.
        try { return_native_gui_group_slot_00ac73d0(slot); }
        catch (...) { std::terminate(); }
        throw;
    }
}

void* delete_native_gui_group_00ac73e0(void* group,
    const volatile std::uint32_t& flags_slot,
    NativeGuiWidgetLifetimeContext& context) {
    *static_cast<volatile std::uint32_t*>(group) = 0x00d5cb80u;
    destroy_native_gui_widget_base_00aa9730(group, context);
    const auto flags = *reinterpret_cast<const volatile unsigned char*>(&flags_slot);
    if ((flags & 1u) != 0)
        game::game_native_gui_group_pool_process().pool_00f8bfa0().return_00ac7260(group);
    return group;
}
} // namespace bsp
