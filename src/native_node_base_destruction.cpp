#include "bsp/native_node_base_destruction.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_node_destruction.hpp"
#include "bsp/native_node_raw_transform.hpp"
#include "bsp/native_node_root_propagation.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/platform_renderer_activation.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* ptr(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(Word p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(p + offset);
}
void write(Word p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(p + offset) = value;
}
void require_same_domain(const NativeNodeBaseDestructionContext& context) {
    if (&context.trees.owners != static_cast<NativeRenderActualOwners*>(&context.scenes.owners) ||
        &context.trees.decrement_00ce2220 != &context.scenes.decrement_00ce2220)
        throw std::invalid_argument("node base destruction requires SAME actual owners and decrement cell");
}
void return_name(Word node, Word captured_data, NativeStringRawPoolContext& strings) {
    const Word size = read(node, 0x54) + 1u; // BEFORE getter, including wrap
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, ptr(captured_data), size,
        strings.actual_small_returns_disabled_01090aa4);
}
void destroy_current_name(Word node, NativeStringRawPoolContext& strings) {
    const Word data = read(node, 0x58); // genuine41DD20 schedule
    if (data != 0) return_name(node, data, strings);
}
}

void unlink_native_raw_child_00b6d940(void* actual_parent, void* captured_child) noexcept {
    const Word parent = bits(actual_parent), child = bits(captured_child);
    if (read(child, 0x30) != parent) return;
    const Word previous = read(child, 0x40);
    write(child, 0x30, 0);
    if (previous != 0) write(previous, 0x3c, read(child, 0x3c));
    const Word next = read(child, 0x3c);
    if (next != 0) write(next, 0x40, read(child, 0x40));
    const Word first = read(parent, 0x34);
    if (child == first) write(parent, 0x34, read(first, 0x3c));
    write(parent, 0x38, read(parent, 0x38) - 1u);
}

void notify_native_raw_world_00b6dbe0(void* actual, NativeNodeBaseWorldDispatch& calls) {
    const Word node = bits(actual);
    write(node, 0x138, read(node, 0x138) & 0xffffffcfu);
    const Word attachment = read(node, 0xa0);
    if (attachment != 0) {
        const Word profile = read(attachment);
        const Word target = calls.resolve_profile(profile)[0x3c / 4];
        calls.invoke_virtual3c(target, ptr(attachment));
    }
}

void set_native_raw_parent_null_00b6e680(void* actual,
    NativeNodeParentNullFrame& frame, NativeNodeBaseDestructionContext& context,
    NativeNodeParentNullAcquired& acquired) {
    require_same_domain(context); // pure admission check before native work
    acquired.started = true;
    const Word node = bits(actual);
    const Word parent = read(node, 0x30);
    if (parent == 0) { acquired.complete = true; return; }
    acquired.active_call_site = 0x00b6e69b;
    unlink_native_raw_child_00b6d940(ptr(parent), actual);
    const Word attachment = read(node, 0xa0);
    if (attachment != 0) {
        frame.attachment_node_argument = node;
        acquired.active_call_site = 0x00b6e6ab;
        unregister_native_node_attachment_00b8f4c0(ptr(attachment), frame.attachment_node_argument);
        write(node, 0xa0, 0);
    }
    const Word root = read(node, 0xa4); // PUSH before clearing30/A4/A0
    write(node, 0x30, 0);
    write(node, 0xa4, 0);
    write(node, 0xa0, 0);
    acquired.active_call_site = 0x00b6e710;
    propagate_native_node_root_storage_00b6d890(actual, ptr(root), context.scenes.children);
    acquired.active_call_site = 0x00b6e717;
    unregister_native_node_tree_00b6d850(actual, frame.recursive_unregister,
        context.trees, acquired.recursive_unregister);
    const Word flags = read(node, 0x5c);
    if (flags & 2u) {
        write(node, 0x138, read(node, 0x138) & 0xffffffcfu);
        const bool has_child = read(node, 0x34) != 0; // BEFORE flags store
        write(node, 0x5c, flags & 0xfffffff5u);
        if (has_child) {
            acquired.active_call_site = 0x00b6e738;
            invalidate_raw_descendants_00b6da30(actual);
        }
    }
    const Word profile = read(node);
    const Word target = context.world.resolve_profile(profile)[0x40 / 4];
    acquired.active_call_site = 0x00b6e744;
    context.world.invoke_virtual40(target, actual);
    acquired.complete = true;
}

void destroy_native_raw_point_light_array_00b6f3e0(void* actual_array,
    NativePointLightLinksRuntime& allocations) noexcept {
    shrink_native_node_point_lights_to_zero_00b6ec70(
        *static_cast<NativeNodePointLightArray*>(actual_array));
    const Word begin = read(bits(actual_array));
    allocations.free_backing(ptr(begin)); // stale descriptor is untouched
}

void destroy_native_raw_node_base_00b6f440(void* actual,
    NativeNodeBaseDestructionFrame& frame, NativeNodeBaseDestructionContext& context,
    NativeNodeBaseDestructionAcquired& acquired) {
    require_same_domain(context); // no native reads/stores before admission
    acquired.started = true;
    const Word node = bits(actual);
    write(node, 0, 0x00d62c88);
    const Word attachment = read(node, 0xa0);
    int state = 2;
    acquired.native_eh_state = state;
    try {
        if (attachment != 0) {
            frame.attachment_node_argument = node;
            acquired.active_call_site = 0x00b6f47a;
            unregister_native_node_attachment_00b8f4c0(ptr(attachment), frame.attachment_node_argument);
            write(node, 0xa0, 0);
        }
        acquired.active_call_site = 0x00b6f488;
        set_native_raw_parent_null_00b6e680(actual, frame.parent_null, context, acquired.parent_null);
        const Word root = read(node, 0xa4);
        if (root != 0 || read(node, 0x30) == 0) {
            if (root != 0 && read(node, 0x30) == 0) {
                acquired.active_call_site = 0x00b6f4a6;
                unlink_native_raw_root_node_00b72220(ptr(root), actual);
            }
            Word child = read(node, 0x34); // capture BEFORE clearingA4
            write(node, 0xa4, 0);
            while (child != 0) {
                acquired.active_call_site = 0x00b6f4bb;
                propagate_native_node_root_storage_00b6d890(ptr(child), nullptr, context.scenes.children);
                child = read(child, 0x3c);
            }
        }
        const Word retained = read(node, 0x130);
        acquired.captured_retained_owner = ptr(retained);
        if (retained != 0) {
            acquired.active_call_site = 0x00b6f4d5;
            const long result = context.trees.decrement_00ce2220(
                reinterpret_cast<volatile long*>(retained + 4u));
            acquired.retained_decrement_completed = true;
            if (result == 0) {
                auto& reference = context.trees.owners.resolve_actual(ptr(retained));
                if (static_cast<void*>(&reference.reference_count) != ptr(retained + 4u))
                    throw std::logic_error("retained130 terminal requires SAME actual owner count");
                const Word profile = read(retained);
                const Word target = context.trees.dispatch.resolve_profile(profile)[0];
                acquired.active_call_site = 0x00b6f4e5;
                context.trees.dispatch.invoke_virtual0(target, ptr(retained), reference);
                acquired.retained_terminal_completed = true; // no retained payload read
            }
            write(node, 0x130, 0);
        }
        const Word scene = read(node, 0x170); // BEFORE the second130 clear
        frame.scene_remove.recursion_argument = 1;
        frame.scene_remove.scene_argument = scene;
        write(node, 0x130, 0);
        acquired.active_call_site = 0x00b6f4fe;
        remove_native_node_scene_storage_00b6ee10(actual, frame.scene_remove,
            context.scenes, acquired.scene_remove);
        state = 1; acquired.native_eh_state = state; // consume BEFORE resize/free
        acquired.active_call_site = 0x00b6f511;
        shrink_native_node_point_lights_to_zero_00b6ec70(
            *reinterpret_cast<NativeNodePointLightArray*>(node + 0x164u));
        const Word begin = read(node, 0x164);
        acquired.active_call_site = 0x00b6f519;
        context.trees.point_lights.free_backing(ptr(begin));
        acquired.array_cleanup_completed = true;
        const Word data = read(node, 0x58); // BEFORE normal name state consumption
        state = 0; acquired.native_eh_state = state;
        if (data != 0) {
            acquired.active_call_site = 0x00b6f536;
            return_name(node, data, context.strings);
        }
        acquired.name_cleanup_completed = true;
        state = -1; acquired.native_eh_state = state;
        acquired.active_call_site = 0x00b6f552;
        destroy_native_gui_ref_base_00aa6e10(actual); // D5C104 then realBD30F0
        acquired.base_cleanup_completed = true;
        acquired.complete = true;
    } catch (...) {
        acquired.exception_cleanup_started = true;
        try {
            while (state >= 0) {
                const int action = state--;
                acquired.native_eh_state = state; // consume BEFORE cleanup call
                if (action == 2) {
                    acquired.active_call_site = 0x00cc19fc;
                    destroy_native_raw_point_light_array_00b6f3e0(ptr(node + 0x164u), context.trees.point_lights);
                    acquired.array_cleanup_completed = true;
                } else if (action == 1) {
                    acquired.active_call_site = 0x00cc19ee;
                    destroy_current_name(node, context.strings);
                    acquired.name_cleanup_completed = true;
                } else {
                    acquired.active_call_site = 0x00cc19e3;
                    destroy_native_gui_ref_base_00aa6e10(actual);
                    acquired.base_cleanup_completed = true;
                }
            }
        } catch (...) { std::terminate(); } // source C++ projection, not FH3 proof
        throw;
    }
}
} // namespace bsp
