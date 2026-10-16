#include "bsp/native_gui_layer_lifetime.hpp"
#include "bsp/native_gui_layer_storage.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI layer lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4 && sizeof(long) == 4);
void* at(void* owner, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
volatile Word& word(void* owner, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(owner, offset));
}
void* pointer(void* owner, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(word(owner, offset));
}
unsigned char byte(void* owner, Word offset) noexcept {
    return *static_cast<volatile unsigned char*>(at(owner, offset));
}
void unwind_layer(void* owner, int state, NativeGuiLayerLifetimeContext& c) noexcept {
    try {
        if (state == 1) {
            state = 0;
            destroy_native_string_header_0041dd20(at(owner, 0x100), c.strings);
        }
        if (state == 0)
            destroy_native_gui_widget_base_00aa9730(owner, c.widgets);
    } catch (...) {
        std::terminate();
    }
}
} // namespace

void release_native_gui_scene_storage_00b72250(void* scene, NativeGuiLayerLifetimeContext& c) {
    const auto decrement = c.actual_decrement_00ce2220;
    if (decrement(static_cast<volatile long*>(at(scene, 4))) == 0) {
        const Word profile = word(scene); // Native B72261, AFTER decrement.
        const auto* const table = c.bindings.current_scene_table(scene, profile);
        if (!table)
            throw std::logic_error("Raw scene release requires its actual current table");
        const Word target = table[0];
        c.bindings.call_scene_zero_00(scene, target);
    }
}

void destroy_native_gui_layer_00ac5480(void* owner, NativeGuiLayerLifetimeContext& c) {
    word(owner) = 0x00d5be38u;
    const auto owns_store = byte(owner, 0xf4); // AC54A3, before state1.
    int state = 1;
    try {
        if (owns_store != 0) {
            void* const manager = c.bindings.manager_004c12b0();
            c.bindings.remove_camera_store_00aa4b30(manager, pointer(owner, 0xf0));
            if (void* const scene = pointer(owner, 0xec)) {
                release_native_gui_scene_storage_00b72250(scene, c);
                word(owner, 0xec) = 0; // AFTER terminal/provider callback.
            }
        } else if (void* const scene = pointer(owner, 0xec)) {
            const auto decrement = c.actual_decrement_00ce2220;
            if (decrement(static_cast<volatile long*>(at(scene, 4))) == 0) {
                const Word profile = word(scene); // AFTER decrement.
                const auto* const table = c.bindings.current_scene_table(scene, profile);
                if (!table)
                    throw std::logic_error("Raw layer scene requires its actual current table");
                const Word target = table[0];
                c.bindings.call_scene_zero_00(scene, target);
            }
            word(owner, 0xec) = 0; // No later read of retired scene storage.
        }
        void* const data = pointer(owner, 0x104); // Captured BEFORE state0.
        state = 0;
        if (data) {
            const Word size = word(owner, 0x100) + 1u;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                c.strings.actual_published_01090aa8,
                c.strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, data, size,
                c.strings.actual_small_returns_disabled_01090aa4);
        }
        state = -1; // Consume base cleanup BEFORE entering AA9730.
        destroy_native_gui_widget_base_00aa9730(owner, c.widgets);
    } catch (...) {
        unwind_layer(owner, state, c);
        throw;
    }
}

void* scalar_delete_native_gui_layer_00aa38f0(void* owner,
    const volatile Word& flags, NativeGuiLayerLifetimeContext& c) {
    destroy_native_gui_layer_00ac5480(owner, c);
    const auto low_flags = *reinterpret_cast<const volatile unsigned char*>(&flags);
    if ((low_flags & 1u) != 0)
        return_native_gui_layer_slot_00ac4c40(owner);
    return owner;
}
} // namespace bsp
