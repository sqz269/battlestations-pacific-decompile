#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_gui_widget_raw_model_lifetime.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_live_effect_manager.hpp"
#include "bsp/native_logical_vertex_owner.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_renderer_pointer_array_destroy.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <new>
#include <cstring>

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
class CrtTimedStorage final : public NativeGuiTimedStorageCalls {
public:
    void* allocate_timed_pointer_slots_00bf681b(void*, Word bytes) override {
        return singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
    }
    void free_timed_pointer_slots_00bf65ac(void*, void* backing) override {
        singleton_lifetime_free(backing);
    }
};
void reserve_timed(void* header, Word requested, NativeGuiTimedStorageCalls& calls) {
    if (static_cast<std::int32_t>(requested) < 1) requested = 1;
    if (signed_word(header, 8) >= static_cast<std::int32_t>(requested)) return;
    const Word bytes = requested * 4u;
    void* const replacement = calls.allocate_timed_pointer_slots_00bf681b(header, bytes);
    void* destination = replacement;
    for (Word index = 0; static_cast<std::int32_t>(index) < signed_word(header, 4); ++index) {
        if (destination) word(destination) = word(pointer(header), index * 4u);
        destination = at(destination, 4);
    }
    calls.free_timed_pointer_slots_00bf65ac(header, pointer(header));
    word(header) = reinterpret_cast<Word>(replacement);
    word(header, 8) = requested;
}
void resize_timed(void* header, std::int32_t requested, NativeGuiTimedStorageCalls& calls) {
    if (requested > signed_word(header, 8)) reserve_timed(header, static_cast<Word>(requested), calls);
    Word index = word(header, 4);
    while (static_cast<std::int32_t>(index) < requested) {
        const Word destination = word(header) + index * 4u;
        if (destination) word(reinterpret_cast<void*>(destination)) = 0;
        ++index;
    }
    while (requested < signed_word(header, 4)) word(header, 4) = word(header, 4) - 1u;
    word(header, 4) = static_cast<Word>(requested);
}
void finish_timed_backing(void* header, NativeGuiTimedStorageCalls& calls) {
    if (signed_word(header, 8) < 0)
        reserve_native_gui_timed_pointers_00aa6f30(header, 1, calls);
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
    calls.free_timed_pointer_slots_00bf65ac(header, captured_data);
}
void unwind_base(void* owner, int state, NativeGuiTimedStorageCalls& calls) noexcept {
    try {
        if (state >= 2) destroy_native_gui_timed_pointers_00aa7f50(at(owner, 0x88), calls);
        if (state >= 1) destroy_native_gui_widget_list_thunk_00a9bce0(at(owner, 0x64));
        if (state >= 0) destroy_native_gui_ref_base_00aa6e10(owner);
    } catch (...) { std::terminate(); }
}
} // namespace

void reserve_native_gui_timed_pointers_00aa6f30(void* h, std::int32_t n, NativeGuiTimedStorageCalls& c) {
    reserve_timed(h, static_cast<Word>(n), c);
}
void resize_native_gui_timed_pointers_00aa77b0(void* h, std::int32_t n, NativeGuiTimedStorageCalls& c) {
    resize_timed(h, n, c);
}
void destroy_native_gui_timed_pointers_00aa7f50(void* h, NativeGuiTimedStorageCalls& c) {
    resize_timed(h, 0, c); c.free_timed_pointer_slots_00bf65ac(h, pointer(h));
}
void reserve_native_gui_timed_pointers_00aa6f30(void* h, std::int32_t n) {
    CrtTimedStorage c; reserve_native_gui_timed_pointers_00aa6f30(h, n, c);
}
void resize_native_gui_timed_pointers_00aa77b0(void* h, std::int32_t n) {
    CrtTimedStorage c; resize_native_gui_timed_pointers_00aa77b0(h, n, c);
}
void destroy_native_gui_timed_pointers_00aa7f50(void* h) {
    CrtTimedStorage c; destroy_native_gui_timed_pointers_00aa7f50(h, c);
}
void* construct_native_gui_timed_entry_base_00ad3970(void* block, void* widget) noexcept {
    auto* const entry = ::new (block) NativeGuiTimedEntryStorage;
    entry->profile_00 = 0x00d5d204u;
    entry->remaining_04 = 0.0f;
    entry->actual_widget_08 = widget;
    return block;
}
void* construct_native_gui_timed_entry_alpha_00ac2ed0(void* block, void* widget,
    const volatile float& one) noexcept {
    construct_native_gui_timed_entry_base_00ad3970(block, widget);
    const Word captured_one = *reinterpret_cast<const volatile Word*>(&one);
    word(block) = 0x00d5ca74u;
    word(block, 0xc) = captured_one;
    word(block, 0x10) = captured_one;
    return block;
}
void reset_native_gui_timed_entry_base_00ad3990(void* entry) noexcept { word(entry) = 0x00d5d204u; }
void* scalar_delete_native_gui_timed_entry_alpha_00ac2fe0(void* entry, Word flags,
    void* context, NativeGuiTimedEntryFree free_entry) {
    if (!free_entry) throw std::invalid_argument("raw timed scalar requires SAME-allocation free");
    reset_native_gui_timed_entry_base_00ad3990(entry);
    if ((flags & 1u) != 0) free_entry(context, entry);
    return entry;
}
void* scalar_delete_native_gui_timed_entry_section_00ad3a40(void* entry, Word flags,
    void* context, NativeGuiTimedEntryFree free_entry) {
    if (!free_entry) throw std::invalid_argument("raw timed scalar requires SAME-allocation free");
    const bool destroy_allocation = (flags & 1u) != 0;
    word(entry) = 0x00d5d204u;
    if (destroy_allocation) free_entry(context, entry);
    return entry;
}
void* scalar_delete_native_gui_timed_entry_base_00ad3a60(void* entry, Word flags,
    void* context, NativeGuiTimedEntryFree free_entry) {
    return scalar_delete_native_gui_timed_entry_section_00ad3a40(entry, flags, context, free_entry);
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
namespace {
struct LegacySchedule {
    NativeGuiWidgetLifetimeContext& widget;
    CrtTimedStorage crt;
    NativeGuiWidgetRawNodeLifetimeBindings* raw;
    NativeGuiTimedStorageCalls* timed_storage;
    NativeGuiTimedStorageCalls& timed;
    explicit LegacySchedule(NativeGuiWidgetLifetimeContext& c)
        : widget(c), raw(dynamic_cast<NativeGuiWidgetRawNodeLifetimeBindings*>(&c.bindings)),
          timed_storage(dynamic_cast<NativeGuiTimedStorageCalls*>(&c.bindings)),
          timed(timed_storage ? *timed_storage : crt) {
        if (raw && !timed_storage)
            throw std::logic_error("raw GUI lifetime requires its SAME timed backing custody service");
    }
    void begin_scene(void* owner) { if (raw) raw->begin_raw_widget_scene(owner); }
    void finish_scene() { if (raw) raw->finish_raw_widget_scene(); }
    void boundary(void*) {}
    void child_scene() {}
    void child_delete() {}
    void timed_delete() {}
    void query(Word, Word, bool) {}
    void release_slot(void* owner, Word offset, bool late) {
        if (!raw) { release_node_slot(owner, offset, widget); return; }
        if (void* const captured = pointer(owner, offset)) {
            raw->release_raw_widget_node(captured, late);
            word(owner, offset) = 0;
        }
    }
    void glyph(void* owner) { release_slot(owner, 0x188, false); }
    void primary(void* owner, bool late) { release_slot(owner, 0x4c, late); }
    void parent(void* owner, void* parent) {
        if (raw) { raw->detach_raw_widget_parent(owner, parent); return; }
        auto& c = widget;
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
    void resize(void* header, std::int32_t count) { resize_timed(header, count, timed); }
    void finish_backing(void* header) { finish_timed_backing(header, timed); }
    void unwind(void* owner, int state) {
        if (raw) raw->raw_widget_lifetime_failed();
        else unwind_base(owner, state, timed);
    }
};
struct RawSchedule {
    NativeGuiWidgetRawModelLifetimeContext& raw;
    NativeGuiWidgetLifetimeContext& widget;
    bool nested;
    explicit RawSchedule(NativeGuiWidgetRawModelLifetimeContext& c, bool base)
        : raw(c), widget(c.widget), nested(base) {}
    void begin_scene(void* owner) { raw.slots.begin_scene(raw, owner, nested); }
    void finish_scene() { raw.slots.finish_scene(nested); }
    void boundary(void* owner) { raw.slots.boundary(owner); }
    void child_scene() { raw.slots.reject("raw widget child scene is outside the qualified base domain"); }
    void child_delete() { raw.slots.reject("raw widget child scalar is outside the qualified base domain"); }
    void timed_delete() { raw.slots.reject("raw widget timed scalar is outside the qualified base domain"); }
    void query(Word target, Word descriptor, bool result) { raw.slots.query(target, descriptor, result); }
    void glyph(void* owner) { raw.slots.reject_glyph(raw, owner); }
    void primary(void* owner, bool late) {
        if (void* const captured = pointer(owner, 0x4c)) {
            raw.slots.release_captured(captured, late ? raw.slots.late_primary() : raw.slots.scene_primary());
            word(owner, 0x4c) = 0; // native original slot clear AFTER callback, even replacement
        }
    }
    void parent(void*, void*) { raw.slots.reject("raw widget parent arm has no admitted hierarchy transport"); }
    void resize(void* header, std::int32_t count) { resize_native_gui_timed_pointers_00aa77b0(header, count); }
    void finish_backing(void* header) { CrtTimedStorage calls; finish_timed_backing(header, calls); }
    void unwind(void*, int) { raw.slots.fail(); } // ALL raw escapes retain; no invented native origin
};
template<class Schedule> void release_scene(void* owner, Schedule& schedule) {
    auto& c = schedule.widget;
    schedule.begin_scene(owner);
    void* cursor = pointer(pointer(owner, 0x68));
    for (;;) {
        if (cursor == pointer(owner, 0x68)) break;
        schedule.child_scene();
        validate(cursor != pointer(owner, 0x68));
        void* child = pointer(cursor, 8);
        const Word target = table(child, c)[0x20 / 4];
        if (target == 0x00aa8320u) release_scene(child, schedule);
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
    schedule.query(target, descriptor, is_text);
    if (is_text) schedule.glyph(owner);
    schedule.primary(owner, false);
    schedule.finish_scene();
}
template<class Schedule> void destroy_base(void* owner, Schedule& schedule) {
    auto& c = schedule.widget;
    word(owner) = 0x00d5c130u;
    int unwind_state = 2;
    try {
        release_scene(owner, schedule);
        schedule.boundary(owner);
        void* cursor = pointer(pointer(owner, 0x68));
        while (word(owner, 0x6c) != 0) {
            schedule.child_delete();
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
            schedule.parent(owner, parent);
        }
        schedule.primary(owner, true);
        schedule.boundary(owner);
        if (signed_word(owner, 0x8c) > 0) {
            schedule.timed_delete();
            Word index = 0;
            Word next_count = 1;
            do {
                if (static_cast<std::int32_t>(index) >= signed_word(owner, 0x8c))
                    schedule.resize(at(owner, 0x88), static_cast<std::int32_t>(next_count));
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
        schedule.finish_backing(at(owner, 0x88));
        destroy_native_gui_widget_list_00a9b740(at(owner, 0x64));
        unwind_state = -1;
        word(owner) = 0x00d5c104u;
        destroy_native_ref_counted_base_00bd30f0(owner);
    } catch (...) {
        schedule.unwind(owner, unwind_state);
        throw;
    }
}
} // namespace
void release_native_gui_widget_scene_00aa8320(void* owner, NativeGuiWidgetLifetimeContext& c) {
    LegacySchedule schedule{c}; release_scene(owner, schedule);
}
void destroy_native_gui_widget_base_00aa9730(void* owner, NativeGuiWidgetLifetimeContext& c) {
    LegacySchedule schedule{c}; destroy_base(owner, schedule);
}
void release_native_gui_widget_scene_00aa8320(void* owner, NativeGuiWidgetRawModelLifetimeContext& c) {
    try { RawSchedule schedule(c, false); release_scene(owner, schedule); }
    catch (...) { c.slots.fail(); throw; }
}
void destroy_native_gui_widget_base_00aa9730(void* owner, NativeGuiWidgetRawModelLifetimeContext& c) {
    try {
        c.slots.begin_base(c, owner);
        RawSchedule schedule(c, true); destroy_base(owner, schedule);
        c.slots.finish_base(); // metadata ONLY: base/containers have ended
    } catch (...) { c.slots.fail(); throw; }
}
void release_native_gui_text_glyph_00ab73b0(void* owner, NativeGuiWidgetRawModelLifetimeContext& c) {
    try { c.slots.reject_glyph(c, owner); }
    catch (...) { c.slots.fail(); throw; }
}
} // namespace bsp
