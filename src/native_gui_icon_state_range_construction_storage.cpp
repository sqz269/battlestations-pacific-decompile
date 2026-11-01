#include "bsp/native_gui_icon_state_range_construction_storage.hpp"
#include "bsp/native_gui_icon_storage.hpp"
#include <new>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This normal raw range supplier requires MSVC Win32 DWORD storage contracts.
#endif

namespace bsp {
static_assert(sizeof(void*) == sizeof(std::uint32_t));
static_assert(std::is_trivially_default_constructible_v<NativeGuiIconStateStorage>);
static_assert(std::is_trivially_destructible_v<NativeGuiIconStateStorage>);

std::uint32_t construct_native_gui_icon_state_range_storage_00ab4c50(
    const void* actual_source_begin_ecx, const void* actual_source_end_edx,
    volatile std::uint32_t (&actual_stack_homes)[4],
    NativeGuiIconStateRangeConstructionStorageFrame& frame,
    NativeGuiIconStorageContext& context) {
    volatile std::uint32_t& state = frame.state_04;
    auto* const state_byte = reinterpret_cast<volatile std::uint8_t*>(&frame.state_04);
    state = 0xffffffffu; // Native PUSH -1, without FS/savedESP/private-stack emulation.
    std::uint32_t destination = actual_stack_homes[0];
    static_cast<volatile std::uint32_t&>(frame.end_14) =
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(actual_source_end_edx));
    std::uint32_t source = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual_source_begin_ecx));
    static_cast<volatile std::uint32_t&>(frame.begin_18) = destination;
    state = 0;
    try {
        while (source != static_cast<volatile std::uint32_t&>(frame.end_14)) {
            static_cast<volatile std::uint32_t&>(frame.slot_1c) = destination;
            static_cast<volatile std::uint32_t&>(frame.placement_20) = destination;
            const std::uint32_t placement_nonnull = destination;
            *state_byte = 1;
            if (placement_nonnull != 0) {
                void* const slot = reinterpret_cast<void*>(destination);
                ::new (slot) NativeGuiIconStateStorage;
                copy_native_gui_icon_state_00ab3a70(slot,
                    reinterpret_cast<const void*>(source), context);
            }
            destination += 0x40u;
            *state_byte = 0;
            actual_stack_homes[0] = destination;
            source += 0x40u;
        }
    } catch (...) {
        std::uint32_t cleanup_cursor =
            static_cast<volatile std::uint32_t&>(frame.begin_18);
        const std::uint32_t cleanup_end =
            actual_stack_homes[0];
        while (cleanup_cursor != cleanup_end) {
            destroy_native_gui_icon_state_00ab3280(
                reinterpret_cast<void*>(cleanup_cursor), context);
            cleanup_cursor += 0x40u;
        }
        throw;
    }
    return destination;
}
} // namespace bsp
