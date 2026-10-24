#include "bsp/native_gui_widget_property_children.hpp"
#include "bsp/gui_native_geometry.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw GUI child properties require MSVC Win32.
#endif
namespace bsp {
namespace {
using Word = std::uint32_t;
using Acquired = NativeGuiWidgetPropertyChildrenAcquired;
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* p, Word offset = 0) noexcept {
    return pointer(reinterpret_cast<Word>(p) + offset);
}
Word read(const void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}
void write(void* p, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, offset)) = value;
}
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
Word target(void* raw, Word offset) noexcept { return read(pointer(read(raw)), offset); }
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
void preflight(NativeGuiWidgetPropertyChildrenContext& c, Acquired& a) {
    auto& access = c.models;
    require(a.phase == Acquired::Phase::fresh && a.children.empty() && !a.name_live,
        "GUI children require a fresh retained acquisition frame");
    require(!access.models.actual_names && access.models.nodes.uses_raw_name_pool() &&
        &access.models.nodes.require_raw_name_pool() == &c.strings &&
        &access.models.nodes == &c.parenting.nodes && access.prepare_model &&
        access.retire_failed_model && access.bind_completed_model &&
        &access.models.retained_owners == &access.geometry.actual_owners(),
        "GUI children require SAME raw name, Model, scene and canonical owner domains");
}
void* create_model(void* name, NativeGuiWidgetPropertyChildrenContext& c,
    NativeGuiWidgetPropertyChildAcquired& child, Acquired& a) {
    auto& access = c.models;
    a.active_call_site = 0x00aaad95;
    void* const raw = access.models.pool_01090054.allocate_raw_slot_00b74d00();
    child.raw_model = raw;
    if (!raw) return nullptr; // Original assigns NULL to child+4C unchanged.
    try {
        child.model_owner = &access.prepare_model(access.companion_context, raw, access.models);
        auto& model = *child.model_owner;
        require(&model.storage.node == raw && &model.environment == &access.models &&
            model.phase == NativeModelOwner::Phase::prepared,
            "GUI child Model preparation must preserve SAME unused actual188h slot");
        a.active_call_site = 0x00aaadb1;
        construct_native_model_00b75030(model, name, c.model_constants);
    } catch (...) {
        if (auto* model = child.model_owner) {
            child.model_owner = nullptr;
            access.retire_failed_model(access.companion_context, *model);
        }
        child.raw_model = nullptr;
        access.models.pool_01090054.return_raw_slot_00b74750(raw);
        throw;
    }
    child.constructor_returned = true;
    // Native state2 has ended. A failed metadata registration retains the
    // completed actual creator; it cannot enter constructor slot-return EH.
    child.model_reference = &access.bind_completed_model(access.companion_context, *child.model_owner);
    require(&child.model_reference->model_owner() == child.model_owner &&
        &access.geometry.actual_owners().resolve_actual(raw) ==
            static_cast<RenderCommandReference*>(child.model_reference),
        "GUI child Model must bind its SAME canonical actual+04 reference");
    child.model_bound = true;
    return raw;
}
void release_name(void* name, NativeGuiWidgetPropertyChildrenContext& c, Acquired& a) {
    a.name_live = false; // Native state-1 BEFORE the possibly throwing return.
    a.name_cleanup_entered = true;
    a.active_call_site = 0x00aaae85;
    destroy_native_string_header_0041dd20(name, c.strings);
}
void failed(NativeGuiWidgetPropertyScratch& scratch,
    NativeGuiWidgetPropertyChildrenContext& c, Acquired& a) noexcept {
    a.phase = Acquired::Phase::failed;
    if (a.name_live) {
        try { release_name(at(scratch.bytes, 0x38), c, a); }
        catch (...) { std::terminate(); }
    }
}
void body(void* widget, void* visitor, NativeGuiWidgetPropertyScratch& scratch,
    NativeGuiWidgetPropertyChildrenContext& c, Acquired& a) {
    void* const base = scratch.bytes;
    void* const block = at(base, 0x4c); // Native S+34; scratch begins S-18.
    void* const count = at(base, 0x81c); // Native S+804, after250 pairs.
    void* const name = at(base, 0x38); // Native S+20/24.
    for (Word index = 0; index != 250; ++index) write(block, index * 8, 0xffffffffu);
    const Word enumerate = target(visitor, 0x18); // BEFORE native count store.
    write(count, 0, 0);
    a.active_call_site = 0x00aaad01;
    c.calls.enumerate_keys_18(visitor, enumerate, block);
    write(base, 0x2c, 0); // Native current signed loop index S+14.
    if (static_cast<std::int32_t>(read(count)) <= 0) return;
    write(base, 0x34, bits(block)); // Native current entry S+1C.
    do {
        void* const entry = pointer(read(base, 0x34));
        if (read(entry) == 0) {
            const char* const source = static_cast<const char*>(pointer(read(entry, 4)));
            write(name, 0, 0); write(name, 4, 0);
            Word length = 0;
            while (source[length] != '\0') ++length;
            a.active_call_site = 0x00aaad4a;
            resize_native_string_header_0041dd40(name, c.strings, length, true);
            if (void* data = pointer(read(name, 4)))
                std::memcpy(data, source, static_cast<std::size_t>(read(name) + 1u));
            a.name_live = true; // Native EH state1 only AFTER the copy.
            a.name_cleanup_entered = false;
            a.active_call_site = 0x00aaad78;
            const Word type = native_gui_widget_type_for_key_00aa2490(name, c.types);
            if (type) {
                a.children.emplace_back(); // Persistent metadata only.
                auto& child = a.children.back(); child.type = type;
                a.active_call_site = 0x00aaad89;
                child.widget = c.calls.create_widget_00aa6560(type, nullptr);
                void* const raw_model = create_model(name, c, child, a);
                write(child.widget, 0x4c, bits(raw_model));
                child.model_published = true;
                if (raw_model) write(raw_model, 0x138, read(raw_model, 0x138) & 0xfffffffcu);
                const Word previous_parent = read(child.widget, 0x70);
                write(base, 0x28, bits(child.widget)); // Native S+10 pointer cell.
                if (previous_parent) {
                    a.active_call_site = 0x00aaaddd;
                    detach_native_gui_widget_child_00aa83a0(pointer(previous_parent), child.widget, c.parenting.nodes);
                }
                void* const captured_head = pointer(read(widget, 0x68));
                void* const captured_previous = pointer(read(captured_head, 4));
                write(base, 0x44, bits(captured_head)); // Native S+2C.
                a.active_call_site = 0x00aaadf8;
                auto* const node = create_native_gui_widget_list_node_00a9b790(
                    static_cast<NativeEffectDeletionNode*>(captured_head),
                    static_cast<NativeEffectDeletionNode*>(captured_previous), at(base, 0x28));
                write(base, 0x28, bits(node));
                a.active_call_site = 0x00aaae05;
                grow_native_gui_widget_list_count_00a9d480(
                    *static_cast<NativeEffectDeletionListStorage*>(at(widget, 0x64)), 1);
                void* const current_node = pointer(read(base, 0x28));
                write(pointer(read(base, 0x44)), 4, bits(current_node));
                write(pointer(read(current_node, 4)), 0, bits(current_node));
                write(child.widget, 0x70, bits(widget)); child.linked = true;
                // Native loads parent+4C BEFORE child+4C after list callbacks.
                const Word parent_model = read(widget, 0x4c);
                const Word current_child_model = read(child.widget, 0x4c);
                auto& transform = c.parenting.nodes.scenes.resolve_key(current_child_model).transform;
                auto* parent = parent_model ? &c.parenting.nodes.scenes.resolve_key(parent_model).transform : nullptr;
                a.active_call_site = 0x00aaae24;
                set_native_node_parent_00b6e680(c.parenting, transform, parent);
                a.active_call_site = 0x00aaae31;
                c.calls.widget_created_74(child.widget, target(child.widget, 0x74));
                void* const current_entry = pointer(read(base, 0x34));
                const Word payload = read(current_entry, 4);
                const Word reader_profile = read(visitor);
                const Word tag = read(current_entry);
                const Word enter = read(pointer(reader_profile), 4);
                a.active_call_site = 0x00aaae45;
                c.calls.enter_key_04(visitor, enter, tag, payload);
                a.active_call_site = 0x00aaae50;
                c.calls.read_widget_18(child.widget, target(child.widget, 0x18), visitor);
                a.active_call_site = 0x00aaae59;
                c.calls.leave_key_08(visitor, target(visitor, 8));
                a.active_call_site = 0x00aaae63;
                c.calls.widget_loaded_78(child.widget, target(child.widget, 0x78));
            }
            release_name(name, c, a);
        }
        const Word next_index = read(base, 0x2c) + 1u;
        const Word next_entry = read(base, 0x34) + 8u;
        write(base, 0x2c, next_index); write(base, 0x34, next_entry);
    } while (static_cast<std::int32_t>(read(base, 0x2c)) < static_cast<std::int32_t>(read(count)));
}
} // namespace
NativeGuiWidgetPropertyChildrenAcquired::~NativeGuiWidgetPropertyChildrenAcquired() noexcept {
    if (phase == Phase::entered || phase == Phase::failed) std::terminate();
}
void continue_native_gui_widget_property_children_00aaacd4(void* widget, void* visitor,
    NativeGuiWidgetPropertyScratch& scratch, NativeGuiWidgetPropertyChildrenContext& c, Acquired& a) {
    preflight(c, a); a.phase = Acquired::Phase::entered;
    try { body(widget, visitor, scratch, c, a); a.phase = Acquired::Phase::complete; }
    catch (...) { failed(scratch, c, a); throw; }
}
void read_native_gui_widget_properties_and_children_00aaa710(void* widget, void* visitor,
    NativeGuiWidgetPropertyScratch& scratch, NativeGuiWidgetPropertyInteractionContext& properties,
    NativeGuiWidgetPropertyChildrenContext& children, Acquired& a) {
    preflight(children, a);
    require(&properties.properties.strings == &children.strings,
        "whole GUI properties require the SAME actual string domain in every stage");
    a.phase = Acquired::Phase::entered;
    try {
        read_native_gui_widget_properties_through_transform_00aaa710(widget, visitor, scratch, properties);
        body(widget, visitor, scratch, children, a);
        a.phase = Acquired::Phase::complete;
    } catch (...) { failed(scratch, children, a); throw; }
}
} // namespace bsp
