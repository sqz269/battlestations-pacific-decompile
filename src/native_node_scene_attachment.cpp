#include "bsp/native_node_scene_attachment.hpp"
#include "bsp/system_lighting_owners.hpp"
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* ptr(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(Word p, Word n = 0) noexcept { return *reinterpret_cast<const volatile Word*>(p + n); }
void write(Word p, Word n, Word v) noexcept { *reinterpret_cast<volatile Word*>(p + n) = v; }
volatile long* count(Word p) noexcept { return reinterpret_cast<volatile long*>(p + 4u); }
SystemAmbientBacklinks& array(Word object) noexcept {
    return *reinterpret_cast<SystemAmbientBacklinks*>(object + 0x178u);
}
void terminal(Word captured, NativeNodeSceneAttachmentContext& context,
    NativeNodeSceneAttachmentAcquired& acquired, Word site) {
    acquired.active_call_site = site;
    auto& reference = context.owners.resolve_actual(ptr(captured));
    if (static_cast<void*>(&reference.reference_count) != ptr(captured + 4u))
        throw std::logic_error("node scene terminal requires captured owner's actual count");
    reference.release_zero_references();
    acquired.terminal_completed = true;
}
void recurse(Word node, Word requested, Word slot,
    NativeNodeSceneAttachmentFrame& frame, NativeNodeSceneAttachmentContext& context,
    NativeNodeSceneAttachmentAcquired& acquired, Word site) {
    // Native loads only the LOW BYTE after every preceding gate/import call.
    if (*reinterpret_cast<const volatile unsigned char*>(&frame.recursion_argument) == 0) return;
    Word child = read(node, 0x34);
    while (child != 0) {
        const Word profile = read(child);
        const volatile Word* const table = context.children.resolve_profile(profile);
        const Word target = table[slot / 4u];
        acquired.active_call_site = site;
        context.children.invoke_child(target, ptr(child), requested, 1);
        child = read(child, 0x3c); // AFTER actual child dispatch/callback
    }
}
bool contains_captured_span(Word begin, Word end, Word requested) noexcept {
    Word cursor = begin;
    while (cursor < end) {
        if (read(cursor) == requested)
            return (static_cast<std::int32_t>(cursor - begin) >> 2) >= 0;
        cursor += 4u;
    }
    return false;
}
}

void set_native_node_scene_storage_00b6ed80(void* actual,
    NativeNodeSceneAttachmentFrame& frame, NativeNodeSceneAttachmentContext& context,
    NativeNodeSceneAttachmentAcquired& acquired) {
    acquired.started = true;
    const Word node = bits(actual);
    const Word initial = read(node, 0x170); // B6ED83 before B6ED8A argument
    const Word requested = frame.scene_argument;
    acquired.captured_scene = ptr(requested);
    if (initial != requested) {
        if (initial != 0) {
            frame.erasure.node_argument = node;
            acquired.active_call_site = 0x00b6ed97;
            unregister_native_scene_node_if_light_00b83ec0(ptr(initial), frame.erasure, context.gates);
            acquired.unregistration_completed = true;
        }
        const Word old = read(node, 0x170);
        if (old != requested) {
            write(node, 0x170, requested);
            acquired.scene_slot_published = true;
            if (requested != 0) {
                acquired.active_call_site = 0x00b6edb5;
                context.increment_00ce221c(count(requested));
                acquired.increment_completed = true;
            }
            if (old != 0) {
                acquired.released_scene = ptr(old);
                acquired.active_call_site = 0x00b6edc3;
                const long result = context.decrement_00ce2220(count(old));
                acquired.decrement_completed = true;
                if (result == 0) terminal(old, context, acquired, 0x00b6edd3);
            }
        }
        const Word current = read(node, 0x170);
        if (current != 0) {
            frame.insertion.node_argument = node;
            acquired.active_call_site = 0x00b6ede1;
            register_native_scene_node_if_light_00b83d50(ptr(current), frame.insertion,
                context.gates, acquired.registration);
            acquired.registration_completed = true;
        }
    }
    recurse(node, requested, 0x50, frame, context, acquired, 0x00b6edfe);
    acquired.complete = true;
}

void remove_native_node_scene_storage_00b6ee10(void* actual,
    NativeNodeSceneAttachmentFrame& frame, NativeNodeSceneAttachmentContext& context,
    NativeNodeSceneAttachmentAcquired& acquired) {
    acquired.started = true;
    const Word expected = frame.scene_argument; // B6EE11 before B6EE18 node170
    const Word node = bits(actual);
    const Word initial = read(node, 0x170);
    acquired.captured_scene = ptr(expected);
    if (initial == expected) {
        if (initial != 0) {
            frame.erasure.node_argument = node;
            acquired.active_call_site = 0x00b6ee27;
            unregister_native_scene_node_if_light_00b83ec0(ptr(initial), frame.erasure, context.gates);
            acquired.unregistration_completed = true;
        }
        const Word current = read(node, 0x170);
        if (current != 0) {
            write(node, 0x170, 0);
            acquired.scene_slot_published = true;
            acquired.released_scene = ptr(current);
            acquired.active_call_site = 0x00b6ee45;
            const long result = context.decrement_00ce2220(count(current));
            acquired.decrement_completed = true;
            if (result == 0) terminal(current, context, acquired, 0x00b6ee55);
        }
    }
    recurse(node, expected, 0x54, frame, context, acquired, 0x00b6ee70);
    acquired.complete = true;
}

void attach_native_light_scene_storage_00b7c020(void* actual,
    NativeNodeSceneAttachmentFrame& frame, NativeNodeSceneAttachmentContext& context,
    NativeNodeSceneAttachmentAcquired& acquired) {
    acquired.started = true;
    const Word light = bits(actual);
    const Word begin = read(light, 0x178);
    const Word size = read(light, 0x17c);
    const Word end = begin + size * 4u;
    const Word requested = frame.scene_argument;
    acquired.captured_scene = ptr(requested);
    if (!contains_captured_span(begin, end, requested)) {
        const Word capacity = read(light, 0x180);
        if (read(light, 0x17c) == capacity) {
            const Word doubled = capacity + capacity;
            const auto signed_doubled = static_cast<std::int32_t>(doubled);
            const std::int32_t minimum = signed_doubled > 1 ? signed_doubled : 1;
            acquired.active_call_site = 0x00b7c06f;
            reserve_system_ambient_backlinks_00b7b390(array(light), minimum);
        }
        const Word current_size = read(light, 0x17c);
        const Word current_begin = read(light, 0x178);
        const Word destination = current_begin + current_size * 4u;
        if (destination != 0) write(destination, 0, requested);
        write(light, 0x17c, read(light, 0x17c) + 1u);
        acquired.light_slot_appended = true;
        frame.insertion.node_argument = light;
        acquired.active_call_site = 0x00b7c089;
        register_native_scene_node_if_light_00b83d50(ptr(requested), frame.insertion,
            context.gates, acquired.registration);
        acquired.registration_completed = true;
        acquired.active_call_site = 0x00b7c092;
        context.increment_00ce221c(count(requested));
        acquired.increment_completed = true;
    }
    recurse(light, requested, 0x50, frame, context, acquired, 0x00b7c0b0);
    acquired.complete = true;
}

void remove_native_light_scene_storage_00b7bd60(void* actual,
    NativeNodeSceneAttachmentFrame& frame, NativeNodeSceneAttachmentContext& context,
    NativeNodeSceneAttachmentAcquired& acquired) {
    acquired.started = true;
    const Word light = bits(actual);
    const Word begin = read(light, 0x178);
    const Word size = read(light, 0x17c);
    const Word end = begin + size * 4u;
    const Word requested = frame.scene_argument;
    acquired.captured_scene = ptr(requested);
    if (contains_captured_span(begin, end, requested)) {
        acquired.active_call_site = 0x00b7bd9e;
        // Existing leaf captures the opaque key before any swap/count store;
        // valid disjoint backing has no callback or write during its search.
        // Pass the SAME actual requested argument cell, not a logical owner.
        auto* const key = reinterpret_cast<SceneResource* const*>(
            const_cast<const Word*>(&frame.scene_argument));
        erase_system_ambient_backlink_00b7b620(array(light), key);
        frame.erasure.node_argument = light;
        acquired.active_call_site = 0x00b7bda6;
        unregister_native_scene_node_if_light_00b83ec0(ptr(requested), frame.erasure, context.gates);
        acquired.unregistration_completed = true;
        acquired.released_scene = ptr(requested);
        acquired.active_call_site = 0x00b7bdaf;
        const long result = context.decrement_00ce2220(count(requested));
        acquired.decrement_completed = true;
        if (result == 0) terminal(requested, context, acquired, 0x00b7bdbf);
    }
    recurse(light, requested, 0x54, frame, context, acquired, 0x00b7bdda);
    acquired.complete = true;
}
} // namespace bsp
