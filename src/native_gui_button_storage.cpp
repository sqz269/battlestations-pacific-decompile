#include "bsp/native_gui_button_storage.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_gui_widget_child_links.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include <cstddef>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeGuiButtonTailStorage) == 0x10);
static_assert(offsetof(NativeGuiButtonTailStorage, label_length_f4) == 8);
static_assert(offsetof(NativeGuiButtonTailStorage, label_data_f8) == 0xc);
void require_storage(void* raw) {
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % 4)
        throw std::invalid_argument("native Button requires its aligned actual FCh storage");
}
Word read(const void* raw, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(static_cast<const std::byte*>(raw) + offset);
}
void store(void* raw, std::size_t offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(static_cast<std::byte*>(raw) + offset) = value;
}
void* pointer(const void* raw, std::size_t offset = 0) noexcept {
    return reinterpret_cast<void*>(read(raw, offset));
}
void return_failed_slot(void* slot) noexcept {
    try { free_gui_widget_pool_slot<GuiButtonPoolTraits>(slot); }
    catch (...) { std::terminate(); }
}
void unwind_base(void* raw, NativeGuiWidgetLifetimeContext& base) noexcept {
    try { destroy_native_gui_widget_base_00aa9730(raw, base); }
    catch (...) { std::terminate(); }
}
void append_registry(void* raw, NativeGuiButtonRegistryBinding& binding,
    NativeGuiButtonDefaultAcquired& acquired) {
    auto& list = binding.actual_list_00f8bd38;
    auto* const head = static_cast<NativeEffectDeletionNode*>(pointer(&list, 4));
    auto* const previous = static_cast<NativeEffectDeletionNode*>(pointer(head, 4));
    auto* const captured_head_previous_cell = static_cast<std::byte*>(static_cast<void*>(head)) + 4;
    // AAC210 and AAC5D0 are qualified stock specializations of the existing
    // A9B790/8665F0 and A9D480/8675E0 bodies, not new vendor-library credit.
    acquired.label_cleanup_armed = true; // native EH1 before node allocation
    acquired.registry_entered = true;
    auto* const captured_node = create_native_gui_widget_list_node_00a9b790(head, previous, &raw);
    acquired.registry_node = captured_node;
    grow_native_gui_widget_list_count_00a9d480(list, 1);
    acquired.count_incremented = true;
    acquired.link_started = true;
    store(captured_head_previous_cell, 0, reinterpret_cast<Word>(captured_node));
    void* const current_previous = pointer(captured_node, 4);
    store(current_previous, 0, reinterpret_cast<Word>(captured_node));
}
} // namespace
NativeGuiButtonTailStorage& native_gui_button_tail(void* raw) {
    require_storage(raw);
    return *reinterpret_cast<NativeGuiButtonTailStorage*>(static_cast<std::byte*>(raw) + 0xec);
}
NativeGuiButtonRegistryBinding::NativeGuiButtonRegistryBinding(
    NativeEffectDeletionListStorage& actual, volatile Word& focus)
    : actual_list_00f8bd38(actual), actual_focused_00f8bcf8(focus) {
    if (reinterpret_cast<std::uintptr_t>(&actual) % 4 ||
        reinterpret_cast<std::uintptr_t>(&focus) % 4)
        throw std::invalid_argument("Button requires the sole producer's SAME initialized actual registry/focus");
    const Word head = read(&actual, 4);
    if (!head || head % 4)
        throw std::invalid_argument("Button requires the sole producer's SAME initialized actual registry/focus");
}
void* construct_native_gui_button_default_00aac6f0(void* raw, const volatile Word& one,
    NativeGuiButtonLifetimeContext& context, NativeGuiButtonDefaultAcquired& acquired) {
    require_storage(raw);
    if (acquired.started || acquired.base_returned || acquired.label_cleanup_armed ||
        acquired.registry_entered || acquired.registry_node || acquired.count_incremented ||
        acquired.link_started || acquired.complete || acquired.label_cleanup_entered ||
        acquired.base_cleanup_entered)
        throw std::invalid_argument("Button default construction requires its fresh one-shot acquired frame");
    acquired.started = true;
    construct_native_gui_widget_base_00aa9390(raw, 15, one);
    acquired.base_returned = true;
    store(raw, 0, 0x00d5bed0);
    store(raw, 0xec, 0);
    store(raw, 0xf0, 0);
    store(raw, 0xf4, 0);
    store(raw, 0xf8, 0);
    try { append_registry(raw, context.registry, acquired); }
    catch (...) {
        if (acquired.label_cleanup_armed) {
            acquired.label_cleanup_armed = false;
            acquired.label_cleanup_entered = true;
            try { destroy_native_string_header_0041dd20(static_cast<std::byte*>(raw) + 0xf4, context.strings); }
            catch (...) { std::terminate(); }
        }
        acquired.base_cleanup_entered = true;
        unwind_base(raw, context.base);
        throw;
    }
    acquired.label_cleanup_armed = false;
    acquired.complete = true;
    return raw;
}
void* construct_native_gui_button_copy_00aa3c60(void* raw, const void* source,
    NativeGuiButtonCopyContext& context, NativeGuiButtonCopyAcquired& acquired) {
    require_storage(raw);
    if (!source || acquired.base_returned || acquired.label_copy_entered ||
        acquired.complete || acquired.base_cleanup_entered)
        throw std::invalid_argument("Button copy requires its live source and fresh one-shot acquired frame");
    construct_native_gui_widget_copy_00aa9520(raw, source, context.base, acquired.base);
    acquired.base_returned = true;
    store(raw, 0, 0x00d5bed0);
    store(raw, 0xec, read(source, 0xec));
    store(raw, 0xf0, read(source, 0xf0));
    acquired.label_copy_entered = true;
    try {
        // Exact existing actual8h header schedule: identity capture, zero both
        // fields, resize(current source length,true), late source guard and copy
        // CURRENT destination length/source data. No projected string object.
        copy_construct_native_string_header_00426060(static_cast<std::byte*>(raw) + 0xf4,
            static_cast<const std::byte*>(source) + 0xf4, context.strings);
    } catch (...) {
        acquired.base_cleanup_entered = true;
        unwind_base(raw, context.lifetime); // native state0: no derived string cleanup
        throw;
    }
    acquired.complete = true;
    return raw;
}
void* create_native_gui_button_default_00aa4440(const volatile Word& one,
    NativeGuiButtonLifetimeContext& context, NativeGuiButtonDefaultAcquired& acquired) {
    void* const slot = allocate_gui_widget_pool_slot<GuiButtonPoolTraits>();
    if (!slot) return nullptr;
    try { return construct_native_gui_button_default_00aac6f0(slot, one, context, acquired); }
    catch (...) { return_failed_slot(slot); throw; }
}
void* create_native_gui_button_copy_00aa4440(const void* source,
    NativeGuiButtonCopyContext& context, NativeGuiButtonCopyAcquired& acquired) {
    if (!source) throw std::invalid_argument("Button copy arm requires its captured nonnull native source");
    const void* const captured_source = source;
    void* const slot = allocate_gui_widget_pool_slot<GuiButtonPoolTraits>();
    if (!slot) return nullptr;
    try { return construct_native_gui_button_copy_00aa3c60(slot, captured_source, context, acquired); }
    catch (...) { return_failed_slot(slot); throw; }
}
void destroy_native_gui_button_00aac530(void* raw, NativeGuiButtonLifetimeContext& context) {
    require_storage(raw);
    store(raw, 0, 0x00d5bed0);
    try {
        // AAC310 is the same actual stock remove-all specialization A9BD50.
        // It deletes matching nodes only and never deletes/retains the Button.
        remove_native_gui_widget_list_value_00a9bd50(context.registry.actual_list_00f8bd38, &raw);
        if (context.registry.actual_focused_00f8bcf8 != 0)
            context.registry.actual_focused_00f8bcf8 = 0;
        destroy_native_string_header_0041dd20(static_cast<std::byte*>(raw) + 0xf4, context.strings);
    } catch (...) {
        unwind_base(raw, context.base); // native state0, no derived retry/rollback
        throw;
    }
    destroy_native_gui_widget_base_00aa9730(raw, context.base); // native EH state already -1
}
void* delete_native_gui_button_00aa3d10(void* raw, const volatile Word& flags,
    NativeGuiButtonLifetimeContext& context) {
    destroy_native_gui_button_00aac530(raw, context);
    const auto low = *reinterpret_cast<const volatile unsigned char*>(&flags);
    if ((low & 1u) != 0) free_gui_widget_pool_slot<GuiButtonPoolTraits>(raw);
    return raw;
}
} // namespace bsp
