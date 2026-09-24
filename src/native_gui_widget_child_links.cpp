#include "bsp/native_gui_widget_child_links.hpp"
#include "bsp/effect_deletion_queue.hpp"
#include "bsp/native_lua_script_overrides.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw widget child links require MSVC Win32.
#endif

namespace bsp {
namespace {
std::uint32_t word(const void* object, std::size_t offset = 0) {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<const std::byte*>(object) + offset);
}
void store(void* object, std::size_t offset, std::uint32_t value) {
    *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<std::byte*>(object) + offset) = value;
}
void* pointer(const void* object, std::size_t offset = 0) {
    return reinterpret_cast<void*>(word(object, offset));
}
void validate(bool condition) {
    if (!condition) _invalid_parameter_noinfo();
}
CameraTransform& current_node(void* widget, NativeNodeDestructionRuntime& nodes) {
    return nodes.scenes.resolve_key(word(widget, 0x4c)).transform;
}
} // namespace

std::uint32_t native_gui_widget_type_for_key_00aa2490(const NativeString& key,
    const char* empty, const char* underscore) {
    const char* captured = key.data();
    if (!captured) captured = empty;
    const auto cut = reverse_find_native_string_bytes_004bcb80(key, underscore, 0x7fffffff);
    const char* const suffix = captured + (cut + 1u);
    struct Class { const char* name; std::uint32_t id; };
    static constexpr Class classes[] = {
        {"Icon",6}, {"Text",3}, {"Group",2}, {"Progbar",5}, {"Line",4},
        {"Scrollbar",7}, {"Grid",8}, {"Model",9}, {"Movie",10},
        {"Listbox",11}, {"AnimIcon",12}, {"Curve",13}, {"Sound",14},
        {"Button",15}, {"ClipBox",16}, {"Section",17}, {"FrameBox",18}
    };
    for (const auto& entry : classes)
        if (_stricmp(suffix, entry.name) == 0) return entry.id;
    return 0;
}

NativeEffectDeletionNode* create_native_gui_widget_list_node_00a9b790(
    NativeEffectDeletionNode* next, NativeEffectDeletionNode* previous,
    const void* source) {
    return create_effect_deletion_node_008665f0(next, previous, source);
}

void grow_native_gui_widget_list_count_00a9d480(
    NativeEffectDeletionListStorage& list, std::uint32_t increment) {
    grow_effect_deletion_list_count_008675e0(list, increment);
}

void remove_native_gui_widget_list_value_00a9bd50(
    NativeEffectDeletionListStorage& list, const void* source) {
    void* const initial_head = pointer(&list, 4);
    const auto captured_payload = word(source);
    void* cursor = pointer(initial_head);
    while (cursor != initial_head) {
        validate(cursor != pointer(&list, 4));
        if (word(cursor, 8) == captured_payload) {
            validate(cursor != pointer(&list, 4));
            const bool unlink = cursor != pointer(&list, 4);
            void* const next = pointer(cursor);
            if (unlink) {
                store(pointer(cursor, 4), 0, reinterpret_cast<std::uint32_t>(next));
                void* const current_next = pointer(cursor);
                const auto previous = word(cursor, 4);
                store(current_next, 4, previous);
                singleton_lifetime_free(cursor);
                store(&list, 8, word(&list, 8) - 1u);
            }
            cursor = next;
        } else {
            validate(cursor != pointer(&list, 4));
            cursor = pointer(cursor);
        }
    }
}

void detach_native_gui_widget_child_00aa83a0(void* parent, void* child,
    NativeNodeDestructionRuntime& nodes) {
    if (!child) return;
    if (word(child, 0x4c) != 0) {
        set_native_node_parent_null_00b6e680(nodes, current_node(child, nodes));
        propagate_native_node_root_00b6d890(nodes, current_node(child, nodes), nullptr);
        const auto parent_node = word(parent, 0x4c);
        if (parent_node != 0) {
            auto& actual_parent = nodes.scenes.resolve_key(parent_node).transform;
            auto& actual_child = current_node(child, nodes);
            unlink_native_node_child_00b6d940(actual_parent, actual_child);
        }
    }
    remove_native_gui_widget_list_value_00a9bd50(
        *reinterpret_cast<NativeEffectDeletionListStorage*>(
            static_cast<std::byte*>(parent) + 0x64), &child);
    store(child, 0x70, 0);
}

} // namespace bsp
