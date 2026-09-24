#include "bsp/native_node_tree_retirement.hpp"
#include "bsp/native_node_destruction.hpp"
#include "bsp/platform_renderer_activation.hpp"
#include "bsp/system_lighting_owners.hpp"
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
Word target(Word node, Word offset, NativeNodeTreeRetirementContext& context) {
    const Word profile = read(node);
    return context.dispatch.resolve_profile(profile)[offset / 4u];
}
}

void unregister_native_node_attachment_00b8f4c0(void* owner,
    const volatile Word& node_argument) {
    const Word node = node_argument; // ESI captures original incoming word
    if (read(node, 0xa0) != bits(owner)) return;
    // B7BED0's entire103B equals B7B620. Opaque pointer bits only; no host
    // SceneResource dereference, count ownership, fake point-light binding or
    // duplicate erase engine. Valid disjoint accessible array domain applies.
    auto& array = *reinterpret_cast<SystemAmbientBacklinks*>(bits(owner) + 0x178u);
    auto* key = reinterpret_cast<SceneResource* const*>(
        const_cast<const Word*>(&node_argument));
    (void)erase_system_ambient_backlink_00b7b620(array, key);
    write(node, 0xa0, 0); // even if first-equal erase found no entry
}

void unlink_native_node_tree_00b6dfa0(void* actual,
    NativeNodeTreeRetirementContext& context, NativeNodeTreeRetirementAcquired& acquired) {
    acquired.started = true;
    const Word node = bits(actual);
    const Word parent = read(node, 0x30);
    if (parent != 0) {
        const Word previous = read(node, 0x40); // before clearing30
        write(node, 0x30, 0);
        if (previous != 0) write(previous, 0x3c, read(node, 0x3c));
        const Word next = read(node, 0x3c);
        if (next != 0) write(next, 0x40, read(node, 0x40));
        const Word first = read(parent, 0x34);
        if (node == first) write(parent, 0x34, read(first, 0x3c));
        write(parent, 0x38, read(parent, 0x38) - 1u);
        write(node, 0x30, 0);
    } else {
        const Word root = read(node, 0xa4);
        if (root != 0) {
            acquired.active_call_site = 0x00b6dff0;
            unlink_native_raw_root_node_00b72220(ptr(root), actual);
            write(node, 0xa4, 0);
        }
    }
    acquired.hierarchy_unlinked = true;
    const Word current_target = target(node, 0x18, context);
    acquired.active_call_site = 0x00b6e007;
    context.dispatch.invoke_virtual18(current_target, actual);
    acquired.complete = true; // metadata only: actual node may already be dead
}

void release_native_node_tree_00b6f310(void* actual,
    NativeNodeTreeRetirementFrame& frame, NativeNodeTreeRetirementContext& context,
    NativeNodeTreeRetirementAcquired& acquired) {
    acquired.started = true;
    const Word node = bits(actual);
    Word index = 0;
    while (index < read(node, 0x168)) {
        const Word begin = read(node, 0x164);
        const Word light = read(begin + index * 4u);
        // Lookup is pure existing metadata over the SAME physical1E0 array.
        auto& binding = context.point_lights.light(ptr(light));
        acquired.active_call_site = 0x00b6f32b;
        remove_native_point_light_backlink_00b7c1a0(binding,
            *reinterpret_cast<NativeNodeStorage*>(actual));
        ++index; // native unsigned index/current unsigned count comparison
    }
    acquired.active_call_site = 0x00b6f342;
    shrink_native_node_point_lights_to_zero_00b6ec70(
        *reinterpret_cast<NativeNodePointLightArray*>(node + 0x164u));
    acquired.point_lights_cleared = true;
    while (read(node, 0x34) != 0) {
        const Word child = read(node, 0x34);
        const Word next = read(child, 0x3c);
        write(node, 0x34, next);
        if (next != 0) write(next, 0x40, 0);
        // Native captures target BEFORE the following child field clears.
        const Word current_target = target(child, 0x18, context);
        write(child, 0x30, 0);
        write(child, 0x3c, 0);
        acquired.active_call_site = 0x00b6f36b;
        context.dispatch.invoke_virtual18(current_target, ptr(child));
        ++acquired.completed_child_calls;
    }
    if (*reinterpret_cast<const volatile unsigned char*>(node + 0x44u) == 0) {
        const Word attachment = read(node, 0xa0); // BEFORE hierarchy stores
        write(node, 0xa4, 0);
        write(node, 0x30, 0);
        write(node, 0x34, 0);
        write(node, 0x40, 0);
        write(node, 0x3c, 0);
        *reinterpret_cast<volatile unsigned char*>(node + 0x44u) = 1;
        acquired.release_flag_stored = true;
        if (attachment != 0) {
            frame.node_argument = node;
            acquired.active_call_site = 0x00b6f398;
            unregister_native_node_attachment_00b8f4c0(ptr(attachment), frame.node_argument);
            write(node, 0xa0, 0);
            acquired.attachment_cleared = true;
        }
        acquired.active_call_site = 0x00b6f3a7;
        const long result = context.decrement_00ce2220(
            reinterpret_cast<volatile long*>(node + 4u));
        acquired.decrement_completed = true;
        if (result == 0) {
            const Word current_target = target(node, 0, context);
            auto& reference = context.owners.resolve_actual(actual);
            if (static_cast<void*>(&reference.reference_count) != ptr(node + 4u))
                throw std::logic_error("tree terminal requires SAME actual node count");
            acquired.active_call_site = 0x00b6f3ba;
            context.dispatch.invoke_virtual0(current_target, actual, reference);
            acquired.terminal_completed = true; // metadata only after terminal
        }
    }
    acquired.complete = true;
}

void unregister_native_node_tree_00b6d850(void* actual,
    NativeNodeTreeRetirementFrame& frame, NativeNodeTreeRetirementContext& context,
    NativeNodeTreeRetirementAcquired& acquired) {
    acquired.started = true;
    const Word node = bits(actual);
    const Word attachment = read(node, 0xa0);
    if (attachment != 0) {
        frame.node_argument = node;
        acquired.active_call_site = 0x00b6d85e;
        unregister_native_node_attachment_00b8f4c0(ptr(attachment), frame.node_argument);
        write(node, 0xa0, 0);
        acquired.attachment_cleared = true;
    }
    Word child = read(node, 0x34);
    while (child != 0) {
        const auto nested = context.frames.recursive_unregister(ptr(child));
        acquired.active_call_site = 0x00b6d876;
        unregister_native_node_tree_00b6d850(ptr(child), nested.frame, context, nested.acquired);
        ++acquired.completed_child_calls;
        child = read(child, 0x3c);
    }
    acquired.complete = true;
}
} // namespace bsp
