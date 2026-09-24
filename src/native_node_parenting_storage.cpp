#include "bsp/native_node_parenting_storage.hpp"
#include "bsp/native_node_raw_transform.hpp"
#include "bsp/native_node_root_propagation.hpp"
#include "bsp/native_resource_graph_builder.hpp"
#include "bsp/system_lighting_owners.hpp"
#include <cstring>
#include <new>
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
SystemAmbientBacklinks& array(Word p) noexcept {
    return *std::launder(reinterpret_cast<SystemAmbientBacklinks*>(p));
}
void require_same_domain(const NativeNodeParentingContext& context) {
    if (&context.node.trees.owners != static_cast<NativeRenderActualOwners*>(&context.node.scenes.owners) ||
        &context.node.trees.decrement_00ce2220 != &context.node.scenes.decrement_00ce2220)
        throw std::invalid_argument("raw parenting requires SAME actual owners and decrement cell");
}
void require_fresh(const NativeNodeParentingAcquired& acquired) {
    if (acquired.started)
        throw std::invalid_argument("raw parenting requires fresh persistent acquisition diagnostics");
}
void invalidate(Word node, Word call_site, NativeNodeParentingAcquired& acquired) {
    const Word flags = read(node, 0x5c);
    if (flags & 2u) {
        write(node, 0x138, read(node, 0x138) & 0xffffffcfu);
        const bool has_child = read(node, 0x34) != 0; // BEFORE captured flags store
        write(node, 0x5c, flags & 0xfffffff5u);
        if (has_child) {
            acquired.active_call_site = call_site;
            invalidate_raw_descendants_00b6da30(ptr(node));
        }
    }
}
void notify(Word node, Word call_site, NativeNodeParentingContext& context,
    NativeNodeParentingAcquired& acquired) {
    const Word profile = read(node);
    const Word target = context.node.world.resolve_profile(profile)[0x40 / 4];
    acquired.active_call_site = call_site;
    context.node.world.invoke_virtual40(target, ptr(node));
}
}

void reserve_native_group_pointer_array_0059e5e0(void* actual_array,
    std::int32_t captured_minimum) {
    // Same95 instruction bytes after normalizing allocator/free rel32 operands;
    // same real CRT domain. Only opaque pointer bits, never host object access.
    reserve_system_ambient_backlinks_00b7b390(array(bits(actual_array)), captured_minimum);
}

void register_native_group_node_00b8f460(void* actual_group,
    const volatile Word& child_argument) {
    const Word child = child_argument;
    const Word group = bits(actual_group);
    if (read(child, 0xa0) == group) return;
    const Word capacity = read(group, 0x180);
    if (read(group, 0x17c) == capacity) {
        const Word doubled = capacity * 2u;
        std::int32_t requested;
        std::memcpy(&requested, &doubled, sizeof requested);
        reserve_native_group_pointer_array_0059e5e0(ptr(group + 0x178u),
            requested > 1 ? requested : 1);
    }
    const Word count = read(group, 0x17c);
    const Word destination = read(group, 0x178) + count * 4u;
    if (destination != 0) write(destination, 0, child);
    write(group, 0x17c, read(group, 0x17c) + 1u);
    write(child, 0xa0, group);
}

void set_native_node_attachment_00b6d7b0(void* actual,
    NativeNodeAttachmentFrame& frame, NativeNodeParentingDispatch& dispatch,
    NativeNodeParentingAcquired& acquired) {
    require_fresh(acquired);
    acquired.started = true;
    const Word node = bits(actual);
    const Word old = read(node, 0xa0);
    if (old != 0) {
        frame.unregister_node_argument = node;
        acquired.active_call_site = 0x00b6d7bf;
        unregister_native_node_attachment_00b8f4c0(ptr(old), frame.unregister_node_argument);
    }
    const Word requested = frame.attachment_argument; // AFTER unregister
    write(node, 0xa0, requested);
    if (requested != 0) {
        frame.registration_node_argument = node;
        acquired.active_call_site = 0x00b6d7d5;
        register_native_group_node_00b8f460(ptr(requested), frame.registration_node_argument);
        Word child = read(node, 0x34);
        while (child != 0) {
            const Word profile = read(child);
            const Word target = dispatch.resolve_profile(profile)[0x1c / 4];
            acquired.active_call_site = 0x00b6d7e9;
            dispatch.invoke_attachment_1c(target, ptr(child), requested);
            child = read(child, 0x3c); // fresh AFTER callback
        }
    }
    acquired.complete = true;
}

void set_native_group_attachment_00b8f4f0(void* actual,
    NativeNodeAttachmentFrame& frame, NativeNodeParentingAcquired& acquired) {
    require_fresh(acquired);
    acquired.started = true;
    const Word node = bits(actual);
    frame.group_key_local = node; // entry PUSH ECX, actual self preimage
    const Word old = read(node, 0xa0);
    if (old != 0) {
        frame.group_key_local = node; // after capture; CMP EAX,EAX always equal
        auto* key = reinterpret_cast<SceneResource* const*>(
            const_cast<const Word*>(&frame.group_key_local));
        acquired.active_call_site = 0x00b8f511;
        // B7BED0 and B7B620 are exact103B instruction-byte aliases.
        (void)erase_system_ambient_backlink_00b7b620(array(old + 0x178u), key);
        write(node, 0xa0, 0);
    }
    const Word requested = frame.attachment_argument;
    write(node, 0xa0, requested);
    if (requested != 0) {
        frame.registration_node_argument = node;
        acquired.active_call_site = 0x00b8f52f;
        register_native_group_node_00b8f460(ptr(requested), frame.registration_node_argument);
    }
    acquired.complete = true;
}

void prepend_native_raw_child_00b6e010(void* actual_parent,
    NativeNodePrependChildFrame& frame, NativeNodeParentingContext& context,
    NativeNodeParentingAcquired& acquired) {
    require_fresh(acquired);
    require_same_domain(context);
    acquired.started = true;
    const Word child = frame.child_argument;
    const Word parent = bits(actual_parent);
    if (read(child, 0x30) == parent) { acquired.complete = true; return; }
    write(child, 0x30, parent);
    const Word root = read(parent, 0xa4);
    acquired.active_call_site = 0x00b6e029;
    propagate_native_node_root_storage_00b6d890(ptr(child), ptr(root), context.node.scenes.children);
    if (read(child, 0x170) == 0) {
        const Word profile = read(child);
        const Word scene = read(parent, 0x170);
        const Word target = context.node.scenes.children.resolve_profile(profile)[0x50 / 4];
        acquired.active_call_site = 0x00b6e047;
        context.node.scenes.children.invoke_child(target, ptr(child), scene, 1);
    }
    write(child, 0x3c, read(parent, 0x34));
    write(child, 0x40, 0);
    const Word previous_first = read(parent, 0x34);
    if (previous_first != 0) write(previous_first, 0x40, child);
    write(parent, 0x38, read(parent, 0x38) + 1u);
    write(parent, 0x34, child);
    invalidate(child, 0x00b6e083, acquired);
    notify(child, 0x00b6e08f, context, acquired);
    acquired.complete = true;
}

void set_native_raw_parent_00b6e680(void* actual,
    NativeNodeParentingFrame& frame, NativeNodeParentingContext& context,
    NativeNodeSetParentAcquired& acquired) {
    require_fresh(acquired);
    require_same_domain(context);
    acquired.started = true;
    const Word node = bits(actual);
    const Word old_parent = read(node, 0x30); // BEFORE incoming capture
    const Word requested = frame.parent_argument;
    if (old_parent == requested) { acquired.complete = true; return; }
    if (old_parent != 0) {
        acquired.active_call_site = 0x00b6e69b;
        unlink_native_raw_child_00b6d940(ptr(old_parent), actual);
        const Word attachment = read(node, 0xa0);
        if (attachment != 0) {
            frame.unregister_node_argument = node;
            acquired.active_call_site = 0x00b6e6ab;
            unregister_native_node_attachment_00b8f4c0(ptr(attachment), frame.unregister_node_argument);
            write(node, 0xa0, 0);
        }
    } else {
        acquired.active_call_site = 0x00b6e6bb;
        propagate_native_node_root_storage_00b6d890(actual, nullptr, context.node.scenes.children);
    }
    if (requested != 0) {
        const Word profile = read(requested); // captured BEFORE real getter
        acquired.active_call_site = 0x00b6e6c7;
        const Word token = native_graph_group_type_00b8e600(context.group_type_0109032c);
        const Word target = context.dispatch.resolve_profile(profile)[0x0c / 4];
        acquired.active_call_site = 0x00b6e6d2;
        const auto is_group = context.dispatch.invoke_type_0c(target, ptr(requested), token);
        const Word attachment = is_group ? requested : read(requested, 0xa0);
        frame.prepend.child_argument = node;
        acquired.active_call_site = 0x00b6e6e3;
        prepend_native_raw_child_00b6e010(ptr(requested), frame.prepend, context, acquired.prepend);
        if (attachment != 0) {
            const Word current_profile = read(node);
            const Word current_target = context.dispatch.resolve_profile(current_profile)[0x1c / 4];
            acquired.active_call_site = 0x00b6e6f4;
            context.dispatch.invoke_attachment_1c(current_target, actual, attachment);
        } else {
            acquired.active_call_site = 0x00b6e717;
            unregister_native_node_tree_00b6d850(actual, frame.recursive_unregister,
                context.node.trees, acquired.recursive_unregister);
        }
    } else {
        const Word root = read(node, 0xa4);
        write(node, 0x30, 0);
        write(node, 0xa4, 0);
        write(node, 0xa0, 0);
        acquired.active_call_site = 0x00b6e710;
        propagate_native_node_root_storage_00b6d890(actual, ptr(root), context.node.scenes.children);
        acquired.active_call_site = 0x00b6e717;
        unregister_native_node_tree_00b6d850(actual, frame.recursive_unregister,
            context.node.trees, acquired.recursive_unregister);
    }
    invalidate(node, 0x00b6e738, acquired);
    notify(node, 0x00b6e744, context, acquired);
    acquired.complete = true;
}
} // namespace bsp
