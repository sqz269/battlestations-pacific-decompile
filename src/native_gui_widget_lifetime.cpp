#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_live_effect_manager.hpp"
#include "bsp/native_logical_vertex_owner.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_renderer_pointer_array_destroy.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI widget lifetime requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
volatile Word& word(void* p, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(p, offset));
}
void* pointer(void* p, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
std::int32_t signed_word(void* p, Word offset) noexcept {
    return static_cast<std::int32_t>(word(p, offset));
}
void validate(bool condition) {
    if (!condition) _invalid_parameter_noinfo();
}
const volatile Word* table(void* owner, NativeGuiWidgetLifetimeContext& c) {
    const Word profile = word(owner);
    const auto* result = profile == 0x00d5c130u ? c.actual_base_table_00d5c130 :
        c.bindings.current_table(owner, profile);
    if (!result) throw std::logic_error("Raw GUI lifetime requires its actual current table");
    return result;
}
GeneratedModelNodeLifetime& node(void* raw, NativeGuiWidgetLifetimeContext& c) {
    auto* result = c.nodes.attachments.find_actual_node(reinterpret_cast<Word>(raw));
    if (!result) throw std::logic_error("Raw GUI node has no canonical actual lifetime binding");
    return *result;
}
void release_node_slot(void* owner, Word offset, NativeGuiWidgetLifetimeContext& c) {
    if (void* captured = pointer(owner, offset)) {
        unlink_and_release_render_model_00b6dfa0(node(captured, c));
        word(owner, offset) = 0;
    }
}
void finish_timed_backing(void* header) {
    if (signed_word(header, 8) < 0)
        reserve_native_gui_timed_pointers_00aa6f30(header, 1);
    if (signed_word(header, 4) < 0) {
        // AA9906..AA991F tests the SIGN of the wrapping byte offset after
        // increment. This inline branch differs from AA77B0's element-index
        // comparison when count*4 wraps; do not substitute generic resize0.
        Word offset = word(header, 4) * 4u;
        do {
            void* const destination = at(pointer(header), offset);
            if (destination) word(destination) = 0;
            offset += 4u;
        } while (static_cast<std::int32_t>(offset) < 0);
    }
    while (signed_word(header, 4) > 0) word(header, 4) = word(header, 4) - 1u;
    void* const captured_data = pointer(header);
    word(header, 4) = 0;
    singleton_lifetime_free(captured_data);
}
void unwind_base(void* owner, int state) noexcept {
    try {
        if (state >= 2) destroy_native_gui_timed_pointers_00aa7f50(at(owner, 0x88));
        if (state >= 1) destroy_native_gui_widget_list_thunk_00a9bce0(at(owner, 0x64));
        if (state >= 0) destroy_native_gui_ref_base_00aa6e10(owner);
    } catch (...) { std::terminate(); }
}
} // namespace

void reserve_native_gui_timed_pointers_00aa6f30(void* header, std::int32_t requested) {
    reserve_native_renderer_pointer_array_00b22d10(header, static_cast<Word>(requested));
}
void resize_native_gui_timed_pointers_00aa77b0(void* header, std::int32_t requested) {
    resize_native_renderer_query_pointers_00b22cc0(header, 0, requested);
}
void destroy_native_gui_timed_pointers_00aa7f50(void* header) {
    destroy_native_renderer_query_pointers_00b27f50(header);
}
void destroy_native_gui_widget_list_00a9b740(void* header) noexcept {
    // Physical layout alias established by A9B720/A4C4A0/4C3200 producers.
    // This borrows the SAME raw list; it is not a logical widget/container cast.
    destroy_native_effect_deletion_list_004c5940(
        *static_cast<NativeEffectDeletionListStorage*>(header));
}
void destroy_native_gui_widget_list_thunk_00a9bce0(void* header) noexcept {
    destroy_native_gui_widget_list_00a9b740(header);
}
bool native_gui_widget_is_kind_of_00a9e070(Word descriptor,
    const volatile Word (&lineage)[2]) noexcept {
    for (const volatile Word* p = lineage; p != lineage + 2; ++p)
        if (*p == descriptor) return true;
    return false;
}
Word native_gui_text_descriptor_00ab6a30(const volatile Word& descriptor) noexcept {
    return descriptor;
}
void release_native_gui_text_glyph_00ab73b0(void* text, NativeGuiWidgetLifetimeContext& c) {
    release_node_slot(text, 0x188, c);
}
void release_native_gui_widget_scene_00aa8320(void* owner, NativeGuiWidgetLifetimeContext& c) {
    void* cursor = pointer(pointer(owner, 0x68));
    for (;;) {
        if (cursor == pointer(owner, 0x68)) break;
        validate(cursor != pointer(owner, 0x68));
        void* child = pointer(cursor, 8);
        const Word target = table(child, c)[0x20 / 4];
        if (target == 0x00aa8320u) release_native_gui_widget_scene_00aa8320(child, c);
        else c.bindings.call_child_scene_20(child, target);
        validate(cursor != pointer(owner, 0x68));
        cursor = pointer(cursor); // AFTER callback; never cached before it.
    }
    // Capture the slot ADDRESS first; load its current entry AFTER getter.
    const volatile Word* const class_cell = table(owner, c) + 0x0c / 4;
    const Word descriptor = native_gui_text_descriptor_00ab6a30(c.actual_text_descriptor_00f8be28);
    const Word target = *class_cell;
    const bool is_text = target == 0x00a9e070u ?
        native_gui_widget_is_kind_of_00a9e070(descriptor, c.actual_base_lineage_00f8bc88) :
        c.bindings.call_class_query_0c(owner, target, descriptor);
    if (is_text) release_native_gui_text_glyph_00ab73b0(owner, c);
    release_node_slot(owner, 0x4c, c);
}
void destroy_native_gui_widget_base_00aa9730(void* owner, NativeGuiWidgetLifetimeContext& c) {
    word(owner) = 0x00d5c130u;
    int unwind_state = 2;
    try {
        release_native_gui_widget_scene_00aa8320(owner, c);
        void* cursor = pointer(pointer(owner, 0x68));
        while (word(owner, 0x6c) != 0) {
            validate(at(owner, 0x64) != nullptr);
            validate(cursor != pointer(owner, 0x68));
            if (void* child = pointer(cursor, 8)) {
                const Word target = table(child, c)[1];
                c.bindings.call_child_delete_04(child, target, 1);
            }
            // Null entries/no self-detach can loop forever natively. Do not
            // manufacture progress by removing a node or changing its count.
            cursor = pointer(pointer(owner, 0x68));
        }
        if (void* parent = pointer(owner, 0x70)) {
            void* captured_owner = owner;
            if (pointer(owner, 0x4c)) {
                set_native_node_parent_null_00b6e680(c.nodes,
                    node(pointer(owner, 0x4c), c).transform());
                propagate_native_node_root_00b6d890(c.nodes,
                    node(pointer(owner, 0x4c), c).transform(), nullptr);
                if (void* parent_node = pointer(parent, 0x4c))
                    unlink_native_node_child_00b6d940(node(parent_node, c).transform(),
                        node(pointer(owner, 0x4c), c).transform());
            }
            c.bindings.remove_all_00a9bd50(at(parent, 0x64), &captured_owner);
            word(owner, 0x70) = 0;
        }
        release_node_slot(owner, 0x4c, c);
        if (signed_word(owner, 0x8c) > 0) {
            Word index = 0;
            Word next_count = 1;
            do {
                if (static_cast<std::int32_t>(index) >= signed_word(owner, 0x8c))
                    resize_native_gui_timed_pointers_00aa77b0(at(owner, 0x88),
                        static_cast<std::int32_t>(next_count));
                void* const captured_slot = at(pointer(owner, 0x88), index * 4u);
                if (void* entry = pointer(captured_slot)) {
                    const Word target = table(entry, c)[0];
                    c.bindings.call_timed_delete_00(entry, target, 1);
                    word(captured_slot) = 0; // ORIGINAL slot, after callback.
                }
                index += 1u;
                next_count += 1u;
            } while (static_cast<std::int32_t>(index) < signed_word(owner, 0x8c));
        }
        unwind_state = 1; // AA9895, before resize0 may allocate.
        finish_timed_backing(at(owner, 0x88));
        destroy_native_gui_widget_list_00a9b740(at(owner, 0x64));
        unwind_state = -1;
        word(owner) = 0x00d5c104u;
        destroy_native_ref_counted_base_00bd30f0(owner);
    } catch (...) {
        unwind_base(owner, unwind_state);
        throw;
    }
}
} // namespace bsp
