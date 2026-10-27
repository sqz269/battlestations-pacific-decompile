#include "bsp/native_gui_sound_storage.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(GuiSoundPoolTraits::object_bytes == 0x108);
static_assert(GuiSoundPoolTraits::slot_bytes == 0x10c);
static_assert(sizeof(void*) == sizeof(Word));
void* at(void* base, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
const void* at(const void* base, Word offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
volatile Word& word(void* base, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(base, offset));
}
const volatile Word& word(const void* base, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(base, offset));
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
Word bits(const void* value) noexcept { return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value)); }
void begin(NativeGuiSoundStorageAcquired& a, NativeGuiSoundStorageAcquired::Entry entry,
    void* destination, const void* source = nullptr) {
    if (a.phase != NativeGuiSoundStorageAcquired::Phase::fresh)
        throw std::logic_error("native Sound operation cannot replay");
    a.phase = NativeGuiSoundStorageAcquired::Phase::entered;
    a.entry = entry; a.destination = destination; a.source = source;
}
void release(void* identity, NativeGuiSoundInterlocked decrement,
    NativeGuiSoundStorageContext& c, NativeGuiSoundReleaseObservation& r) {
    if (r.entered) throw std::logic_error("native Sound release cannot replay");
    r.entered = true; r.identity = identity;
    if (identity) {
        r.captured_decrement = decrement;
        r.result = decrement(static_cast<volatile long*>(at(identity, 4)));
        r.decremented = true;
        if (r.result == 0) {
            r.zero_entered = true;
            dispatch_native_render_actual_owner_zero(c.owners, identity);
            r.zero_returned = true;
        }
    }
    r.complete = true;
}
void assign(void* destination, const void* source, Word native_base,
    NativeGuiSoundStorageContext& c, NativeGuiSoundStorageAcquired& a,
    NativeGuiSoundAssignmentObservation& o) {
    o.entered = true; o.destination_slot = destination; o.source_slot = source;
    void* const initial = pointer(word(destination)); // D8F9/D9F9.
    if (bits(initial) == word(source)) { o.equal = o.complete = true; return; }
    const auto increment = c.increment_00ce221c; // D907/DA07, captured once.
    o.captured_increment = increment;
    o.held = initial;
    if (initial) {
        a.active_call_site = native_base + 0x42;
        increment(static_cast<volatile long*>(at(initial, 4)));
        o.held_retain_returned = true;
    }
    o.incoming = pointer(word(source)); // AFTER old retain callback.
    o.replaced = pointer(word(destination));
    if (o.replaced != o.incoming) {
        word(destination) = bits(o.incoming); o.published = true;
        if (o.incoming) {
            a.active_call_site = native_base + 0x5e;
            increment(static_cast<volatile long*>(at(o.incoming, 4)));
            o.incoming_retain_returned = true;
        }
        if (o.replaced) {
            a.active_call_site = native_base + 0x68;
            release(o.replaced, c.decrement_00ce2220, c, o.releases[0]);
        }
    }
    if (void* current = pointer(word(destination))) { // D95A/DA5A CURRENT slot.
        a.active_call_site = native_base + 0x8e;
        increment(static_cast<volatile long*>(at(current, 4)));
        o.current_retain_returned = true;
    }
    // Two distinct native releases, each reading CURRENT CE2220. Do not
    // normalize the observed new+2/old-2 transport to ordinary assignment.
    if (o.held) {
        a.active_call_site = native_base + 0xa2;
        release(o.held, c.decrement_00ce2220, c, o.releases[1]);
        a.active_call_site = native_base + 0xc4;
        release(o.held, c.decrement_00ce2220, c, o.releases[2]);
    }
    o.complete = true;
}
void construct(void* destination, const void* source, bool copy,
    NativeGuiSoundStorageContext& c, NativeGuiSoundStorageAcquired& a) {
    a.active_call_site = copy ? 0x00acdbc1u : 0x00acdb35u;
    construct_native_gui_widget_base_00aa9390(destination, 14, c.one_00d7a24c);
    a.base_returned = true;
    word(destination) = 0x00d5cfd0u;
    word(destination, 0xec) = 0;
    word(destination, 0xf0) = 0;
    if (!copy) word(destination, 0xf4) = 0;
    for (const auto offset : {0xf8u, 0xfcu, 0x100u, 0x104u}) word(destination, offset) = 0;
    if (copy) {
        const Word source_f4 = word(source, 0xf4);
        word(destination, 0xf4) = source_f4;
        a.active_call_site = 0x00acdc15;
        assign(at(destination, 0xec), at(source, 0xec), 0x00acd8e0, c, a, a.assignments[0]);
        a.active_call_site = 0x00acdc22;
        assign(at(destination, 0xf0), at(source, 0xf0), 0x00acd9e0, c, a, a.assignments[1]);
    }
    a.derived_returned = true;
}
void destroy(void* sound, NativeGuiSoundStorageContext& c, NativeGuiSoundStorageAcquired& a) {
    word(sound) = 0x00d5cfd0u; // DC60 BEFORE sample/decrement capture.
    void* const sample = pointer(word(sound, 0xec));
    const auto decrement = c.decrement_00ce2220; // DC6C captured once for all four releases.
    a.active_call_site = 0x00acdc84;
    release(sample, decrement, c, a.releases[0]);
    if (sample) word(sound, 0xec) = 0; // DC92 after terminal callback.
    word(sound, 0xec) = 0; // DC98 unconditional second store.
    if (void* instance = pointer(word(sound, 0xf0))) {
        a.stop_receiver = instance;
        a.stop_profile = word(instance);
        const auto* const table = c.stop.current_instance_table(instance, a.stop_profile);
        if (!table) throw std::logic_error("native Sound stop requires its actual current table");
        a.stop_target = table[2];
        a.active_call_site = 0x00acdcb8; a.stop_entered = true;
        c.stop.call_stop_08(instance, a.stop_target, 0);
        a.stop_returned = true;
    }
    void* const tracked = pointer(word(sound, 0xf0)); // DCBA AFTER stop callback.
    a.active_call_site = 0x00acdcc8;
    release(tracked, decrement, c, a.releases[1]);
    if (tracked) word(sound, 0xf0) = 0;
    word(sound, 0xf0) = 0; // DCDC unconditional.
    a.active_call_site = 0x00acdcfe;
    destroy_native_string_header_0041dd20(at(sound, 0x100), c.strings);
    a.active_call_site = 0x00acdd26;
    destroy_native_string_header_0041dd20(at(sound, 0xf8), c.strings);
    // Normal member cleanup re-reads both slots after string-pool callbacks.
    void* const member_instance = pointer(word(sound, 0xf0));
    a.active_call_site = 0x00acdd45;
    release(member_instance, decrement, c, a.releases[2]);
    if (member_instance) word(sound, 0xf0) = 0;
    void* const member_sample = pointer(word(sound, 0xec));
    a.active_call_site = 0x00acdd6b;
    release(member_sample, decrement, c, a.releases[3]);
    if (member_sample) word(sound, 0xec) = 0;
    a.active_call_site = 0x00acdd89;
    destroy_native_gui_widget_base_00aa9730(sound, c.widgets);
    a.base_destroyed = true;
}
} // namespace

NativeGuiSoundStorageAcquired::~NativeGuiSoundStorageAcquired() noexcept {
    if (native_operation_interrupted()) std::terminate();
}
void* construct_native_gui_sound_00acdb30(void* destination,
    NativeGuiSoundStorageContext& c, NativeGuiSoundStorageAcquired& a) {
    begin(a, NativeGuiSoundStorageAcquired::Entry::construct, destination);
    try { construct(destination, nullptr, false, c, a); a.phase = decltype(a.phase)::complete; return destination; }
    catch (...) { a.phase = decltype(a.phase)::failed; throw; }
}
void* copy_native_gui_sound_00acdba0(void* destination, const void* source,
    NativeGuiSoundStorageContext& c, NativeGuiSoundStorageAcquired& a) {
    begin(a, NativeGuiSoundStorageAcquired::Entry::copy, destination, source);
    a.copy_selected = true;
    try { construct(destination, source, true, c, a); a.phase = decltype(a.phase)::complete; return destination; }
    catch (...) { a.phase = decltype(a.phase)::failed; throw; }
}
void destroy_native_gui_sound_00acdc40(void* sound,
    NativeGuiSoundStorageContext& c, NativeGuiSoundStorageAcquired& a) {
    begin(a, NativeGuiSoundStorageAcquired::Entry::destroy, sound);
    try { destroy(sound, c, a); a.phase = decltype(a.phase)::complete; }
    catch (...) { a.phase = decltype(a.phase)::failed; throw; }
}
void* scalar_delete_native_gui_sound_00acdef0(void* sound, const volatile Word& flags,
    NativeGuiSoundStorageContext& c, NativeGuiSoundStorageAcquired& a) {
    begin(a, NativeGuiSoundStorageAcquired::Entry::scalar_delete, sound);
    try {
        destroy(sound, c, a);
        if ((*reinterpret_cast<const volatile unsigned char*>(&flags) & 1u) != 0) {
            a.active_call_site = 0x00acdf05;
            free_gui_widget_pool_slot<GuiSoundPoolTraits>(sound);
            a.slot_returned = true;
        }
        a.phase = decltype(a.phase)::complete; return sound;
    } catch (...) { a.phase = decltype(a.phase)::failed; throw; }
}
void* make_native_gui_sound_00aa1890(const void* source,
    NativeGuiSoundStorageContext& c, NativeGuiSoundStorageAcquired& a) {
    begin(a, NativeGuiSoundStorageAcquired::Entry::factory, nullptr, source);
    try {
        a.active_call_site = 0x00aa18ae;
        a.allocated_slot = allocate_gui_widget_pool_slot<GuiSoundPoolTraits>();
        a.destination = a.allocated_slot;
        a.copy_selected = source != nullptr; // AA18B3 AFTER allocation.
        if (a.allocated_slot) construct(a.allocated_slot, source, a.copy_selected, c, a);
        a.phase = decltype(a.phase)::complete; return a.allocated_slot;
    } catch (...) { a.phase = decltype(a.phase)::failed; throw; }
}
} // namespace bsp
