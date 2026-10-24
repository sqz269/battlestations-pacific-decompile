#include "bsp/native_gui_anim_icon_storage.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw AnimIcon storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
void require_storage(const void* raw) {
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % 4)
        throw std::invalid_argument("AnimIcon requires aligned actual144h native storage");
}
volatile Word& word(void* raw, Word offset = 0) noexcept {
    return *reinterpret_cast<volatile Word*>(static_cast<std::byte*>(raw) + offset);
}
Word word(const void* raw, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(static_cast<const std::byte*>(raw) + offset);
}
std::uint8_t byte(const void* raw, Word offset) noexcept {
    return *reinterpret_cast<const volatile std::uint8_t*>(static_cast<const std::byte*>(raw) + offset);
}
void store_byte(void* raw, Word offset, std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(static_cast<std::byte*>(raw) + offset) = value;
}
void copy_fps_x87(void* destination, const void* source) {
    __asm {
        mov ecx, destination
        mov edx, source
        fld dword ptr [edx + 138h]
        fstp dword ptr [ecx + 138h]
    }
}
void return_failed_slot(void* slot) noexcept {
    try { free_gui_widget_pool_slot<GuiAnimIconPoolTraits>(slot); }
    catch (...) { std::terminate(); }
}
void begin_destroy(void* raw, NativeGuiAnimIconStorageContext& context,
    NativeGuiAnimIconDestroyAcquired& acquired) {
    require_storage(raw);
    if (acquired.started || acquired.icon.started)
        throw std::logic_error("AnimIcon destruction requires its fresh retained one-shot frame");
    if (context.icon.singleton_deletion.gui_icon_delayed_loads != &context.icon.delayed)
        throw std::logic_error("AnimIcon requires the SAME real delayed process deletion binding before AB5700");
    acquired.started = true;
}
void destroy_body(void* raw, NativeGuiAnimIconStorageContext& context,
    NativeGuiAnimIconDestroyAcquired& acquired) {
    word(raw) = 0x00d5bce0;
    destroy_native_gui_icon_00ab5e70(raw, context.icon, acquired.icon);
    acquired.icon_returned = true;
}
void record_failure(NativeGuiAnimIconDestroyAcquired& acquired) noexcept {
    acquired.failure = std::current_exception();
    acquired.failed = true;
    // Icon owns all reached native unwind. Never enter it again from this
    // derived failure path, and never return a slot whose destructor failed.
}
} // namespace
NativeGuiAnimIconTailStorage& native_gui_anim_icon_tail(void* raw) {
    require_storage(raw);
    return *reinterpret_cast<NativeGuiAnimIconTailStorage*>(static_cast<std::byte*>(raw) + 0x138);
}
void* construct_native_gui_anim_icon_default_00ad2d50(void* raw,
    const volatile Word& one, const volatile Word& fps) {
    require_storage(raw);
    construct_native_gui_icon_default_00ab5c60(raw, one);
    const Word current_fps = fps; // AD2D58, before flag/profile stores.
    store_byte(raw, 0x140, 1);
    store_byte(raw, 0x141, 1);
    store_byte(raw, 0x142, 1);
    word(raw) = 0x00d5bce0;
    word(raw, 0x138) = current_fps;
    word(raw, 0x13c) = 0;
    word(raw, 0x60) = 12;
    return raw;
}
void* construct_native_gui_anim_icon_copy_00aa1c60(void* raw, const void* source,
    NativeGuiIconCopyContext& context, NativeGuiAnimIconCopyAcquired& acquired) {
    require_storage(raw);
    require_storage(source);
    if (raw == source || acquired.started || acquired.icon.started)
        throw std::invalid_argument("AnimIcon copy requires distinct actual storage and a fresh frame");
    acquired.started = true;
    try {
        construct_native_gui_icon_copy_00ab5d30(raw, source, context, acquired.icon);
        acquired.icon_returned = true;
        word(raw) = 0x00d5bce0;
        copy_fps_x87(raw, source);
        word(raw, 0x13c) = word(source, 0x13c);
        store_byte(raw, 0x140, byte(source, 0x140));
        store_byte(raw, 0x141, byte(source, 0x141));
        store_byte(raw, 0x142, byte(source, 0x142));
        acquired.complete = true;
        return raw;
    } catch (...) {
        acquired.failure = std::current_exception();
        acquired.failed = true;
        throw; // Native AA1C60 has no derived cleanup; Icon owns its own unwind.
    }
}
void* create_native_gui_anim_icon_default_00aa2340(const volatile Word& one, const volatile Word& fps) {
    void* const slot = allocate_gui_widget_pool_slot<GuiAnimIconPoolTraits>();
    if (!slot) return nullptr;
    try { return construct_native_gui_anim_icon_default_00ad2d50(slot, one, fps); }
    catch (...) { return_failed_slot(slot); throw; }
}
void* create_native_gui_anim_icon_copy_00aa2340(const void* source,
    NativeGuiIconCopyContext& context, NativeGuiAnimIconCopyAcquired& acquired) {
    require_storage(source);
    const void* const captured_source = source;
    void* const slot = allocate_gui_widget_pool_slot<GuiAnimIconPoolTraits>();
    if (!slot) return nullptr;
    try { return construct_native_gui_anim_icon_copy_00aa1c60(slot, captured_source, context, acquired); }
    catch (...) { return_failed_slot(slot); throw; }
}
void destroy_native_gui_anim_icon_00ad2dc0(void* raw, NativeGuiAnimIconStorageContext& context,
    NativeGuiAnimIconDestroyAcquired& acquired) {
    begin_destroy(raw, context, acquired);
    try { destroy_body(raw, context, acquired); }
    catch (...) { record_failure(acquired); throw; }
    acquired.complete = true;
}
void* delete_native_gui_anim_icon_00aa1d50(void* raw, const volatile Word& flags,
    NativeGuiAnimIconStorageContext& context, NativeGuiAnimIconDestroyAcquired& acquired) {
    begin_destroy(raw, context, acquired);
    try {
        destroy_body(raw, context, acquired);
        const auto low = *reinterpret_cast<const volatile unsigned char*>(&flags);
        if ((low & 1u) != 0) {
            acquired.pool_return_entered = true;
            free_gui_widget_pool_slot<GuiAnimIconPoolTraits>(raw);
        }
    } catch (...) { record_failure(acquired); throw; }
    acquired.complete = true;
    return raw;
}
void load_native_gui_anim_icon_00ad2ff0(void* raw, NativeGuiAnimIconStorageContext& context,
    NativeGuiAnimIconLoadedCalls& calls) {
    require_storage(raw);
    calls.call_icon_reset_00ab10f0(raw);
    if (byte(raw, 0x142) != 0) return;
    const Word first = word(raw, 0xf4);
    const Word count = first ? static_cast<Word>(static_cast<std::int32_t>(word(raw, 0xf8) - first) >> 6) : 0;
    const Word captured_profile = word(raw); // AD301C, after count/FLD1.
    if (captured_profile != 0x00d5bce0 || !context.anim_icon_profile_00d5bce0)
        throw std::logic_error("AnimIcon loaded hook requires its genuine selected native table");
    const Word target = context.anim_icon_profile_00d5bce0[0x88 / 4];
    if (target != 0x00ab1710)
        throw std::logic_error("AnimIcon loaded hook requires genuine raw current88 selection");
    calls.call_select_current88(raw, target, count - 1u, 0, 0x3f800000);
}
} // namespace bsp
