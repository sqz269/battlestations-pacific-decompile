#include "bsp/native_gui_icon_storage.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <atomic>
#include <cstring>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw GUI Icon storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Destroy = NativeGuiIconDestroyAcquired;
void require_storage(const void* raw) {
    if (!raw || reinterpret_cast<std::uintptr_t>(raw) % 4)
        throw std::invalid_argument("raw Icon requires aligned actual native storage");
}
void* at(void* p, Word offset) noexcept { return static_cast<std::byte*>(p) + offset; }
const void* at(const void* p, Word offset) noexcept { return static_cast<const std::byte*>(p) + offset; }
volatile Word& word(void* p, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(p, offset));
}
Word word(const void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}
template<class T> T read(const void* p, Word offset) noexcept {
    return *static_cast<const volatile T*>(at(p, offset));
}
template<class T> void write(void* p, Word offset, T value) noexcept {
    *static_cast<volatile T*>(at(p, offset)) = value;
}
void* pointer(void* p, Word offset = 0) noexcept { return reinterpret_cast<void*>(word(p, offset)); }
void copy_x87(void* destination, const void* source) {
    __asm {
        mov eax, destination
        mov edx, source
        fld dword ptr [edx]
        fstp dword ptr [eax]
    }
}
void copy_ratio_enter_state(void* destination, const void* source, std::int32_t* state) {
    __asm {
        mov ecx, destination
        mov edx, source
        fld dword ptr [edx + 114h]
        mov eax, state
        mov dword ptr [eax], 2
        fstp dword ptr [ecx + 114h]
    }
}
// AB5E0C..5E14 loads the current vtable BETWEEN FLD and FSTP timer128.
Word copy_timer_capture_profile(void* destination, const void* source) {
    Word captured;
    __asm {
        mov ecx, destination
        mov edx, source
        fld dword ptr [edx + 128h]
        mov eax, dword ptr [ecx]
        mov captured, eax
        fstp dword ptr [ecx + 128h]
    }
    return captured;
}
void require_delayed_binding(NativeGuiIconStorageContext& context) {
    if (context.singleton_deletion.gui_icon_delayed_loads != &context.delayed)
        throw std::logic_error("raw Icon requires its SAME delayed service in the real process deletion map before AB5700");
}
void return_failed_slot(void* slot) noexcept {
    try { free_gui_widget_pool_slot<GuiIconPoolTraits>(slot); }
    catch (...) { std::terminate(); }
}
// Actual unwind maps DEEAE8/DEEB24: state2 name -> state1 vector -> state0
// raw base. AB59D0 is the existing AB5850 tail-jump. No delayed cancellation
// occurs on construction unwind, and no direct pool return belongs here.
void unwind_icon(void* raw, int state, NativeGuiIconStorageContext& context) noexcept {
    try {
        if (state >= 2) destroy_native_string_header_0041dd20(at(raw, 0x104), context.strings);
        if (state >= 1) destroy_native_gui_icon_states_00ab5850(at(raw, 0xf0), context);
        if (state >= 0) destroy_native_gui_widget_base_00aa9730(raw, context.lifetime);
    } catch (...) { std::terminate(); }
}
void begin_destroy(void* raw, NativeGuiIconStorageContext& context, Destroy& acquired) {
    require_storage(raw);
    if (acquired.started || acquired.phase != Destroy::Phase::fresh)
        throw std::logic_error("raw Icon destruction requires a fresh retained one-shot frame");
    require_delayed_binding(context); // Admission only; no native field was touched.
    acquired.started = true;
}
void destroy_body(void* raw, NativeGuiIconStorageContext& context, Destroy& acquired) {
    word(raw) = 0x00d5c4c0;
    acquired.native_eh_state = 2;
    acquired.phase = Destroy::Phase::delayed_getter;
    auto* const delayed = context.delayed.get_00ab5700();
    acquired.phase = Destroy::Phase::delayed_cancel;
    // A successful native null allocation faults when AB50E0 reads count08.
    // No replacement singleton or empty cancellation arm is manufactured.
    context.delayed.cancel_owner_00ab50e0(*delayed, raw);
    acquired.native_eh_state = 1; // AB5EB0, BEFORE shader return.
    acquired.phase = Destroy::Phase::shader;
    destroy_native_string_header_0041dd20(at(raw, 0x104), context.strings);
    acquired.native_eh_state = 0; // AB5ED6, BEFORE vector destructor.
    acquired.phase = Destroy::Phase::states;
    destroy_native_gui_icon_states_00ab5850(at(raw, 0xf0), context);
    acquired.native_eh_state = -1; // AB5EE2, BEFORE raw base.
    acquired.phase = Destroy::Phase::base;
    destroy_native_gui_widget_base_00aa9730(raw, context.lifetime);
}
void destroy_failed(void* raw, NativeGuiIconStorageContext& context, Destroy& acquired) noexcept {
    acquired.failure = std::current_exception();
    acquired.cleanup_entered = acquired.native_eh_state >= 0;
    unwind_icon(raw, acquired.native_eh_state, context);
    acquired.failed = true;
    acquired.phase = Destroy::Phase::failed;
}
} // namespace

NativeGuiIconTailStorage& native_gui_icon_tail(void* raw) {
    require_storage(raw);
    return *static_cast<NativeGuiIconTailStorage*>(at(raw, 0xec));
}
void* copy_native_gui_icon_state_00ab3a70(void* destination, const void* source,
    NativeGuiIconStorageContext& context) {
    require_storage(destination);
    require_storage(source);
    // Distinct valid native records are the stock uninitialized-copy domain.
    if (destination == source)
        throw std::invalid_argument("Icon state copy requires distinct actual records");
    word(destination) = word(source);
    word(destination, 4) = 0;
    word(destination, 8) = 0;
    resize_native_string_header_0041dd40(at(destination, 4), context.strings, word(source, 4), true);
    if (word(source, 4) != 0)
        std::memcpy(pointer(destination, 8), reinterpret_cast<const void*>(word(source, 8)), word(destination, 4));
    for (Word offset = 0x0c; offset <= 0x28; offset += 4) word(destination, offset) = word(source, offset);
    for (Word offset = 0x2c; offset <= 0x38; offset += 4) copy_x87(at(destination, offset), at(source, offset));
    const auto pending = read<std::uint8_t>(source, 0x3c);
    void* const texture = pointer(destination);
    write(destination, 0x3c, pending);
    write(destination, 0x3d, read<std::uint8_t>(source, 0x3d));
    if (texture) {
        // AB3B15..3B19 is a raw InterlockedIncrement, with no owner lookup or
        // extra native retain. Registry custody is the caller's precondition.
        auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(at(texture, 4)));
        count.fetch_add(1, std::memory_order_seq_cst);
    }
    return destination;
}
void destroy_native_gui_icon_state_00ab3280(void* raw, NativeGuiIconStorageContext& context) {
    require_storage(raw);
    try {
        if (void* const texture = pointer(raw)) {
            release_native_render_actual_owner(context.actual_owners, texture);
            word(raw) = 0; // AB32C2, after terminal, before reloading name.
        }
    } catch (...) {
        // AB3280 native state0 owns the name even when texture0 throws.
        try { destroy_native_string_header_0041dd20(at(raw, 4), context.strings); }
        catch (...) { std::terminate(); }
        throw;
    }
    destroy_native_string_header_0041dd20(at(raw, 4), context.strings);
}
void destroy_native_gui_icon_states_00ab5850(void* raw, NativeGuiIconStorageContext& context) {
    require_storage(raw);
    auto* first = static_cast<std::byte*>(pointer(raw, 4));
    if (first) {
        auto* const last = static_cast<std::byte*>(pointer(raw, 8));
        for (; first != last; first += 0x40) destroy_native_gui_icon_state_00ab3280(first, context);
        singleton_lifetime_free(pointer(raw, 4)); // CURRENT first, not the captured iterator.
    }
    word(raw, 4) = 0;
    word(raw, 8) = 0;
    word(raw, 12) = 0;
}
void copy_native_gui_icon_states_00ab58f0(void* destination, const void* source,
    NativeGuiIconStorageContext& context) {
    require_storage(destination);
    require_storage(source);
    if (destination == source)
        throw std::invalid_argument("Icon vector copy requires distinct actual headers");
    const Word source_first = word(source, 4);
    const Word source_last = source_first ? word(source, 8) : 0;
    if (source_first && (source_last < source_first || ((source_last - source_first) & 0x3f)))
        throw std::invalid_argument("Icon stock vector requires its valid native record range");
    const Word size = source_first ? static_cast<Word>(static_cast<std::int32_t>(source_last - source_first) >> 6) : 0;
    word(destination, 4) = 0;
    word(destination, 8) = 0;
    word(destination, 12) = 0;
    if (!size) return;
    if (size > 0x03ffffff) throw std::length_error("Icon stock vector exceeds its native record limit");
    const auto bytes = static_cast<std::size_t>(size) * 0x40;
    auto* const allocated = static_cast<std::byte*>(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes}));
    word(destination, 4) = reinterpret_cast<Word>(allocated);
    word(destination, 8) = reinterpret_cast<Word>(allocated);
    word(destination, 12) = reinterpret_cast<Word>(allocated + bytes);
    auto* completed = allocated;
    try {
        // Native AB5962..5999 reloads source extent after allocation. Valid
        // serialized source storage excludes callback mutation/overrun here.
        auto* first = reinterpret_cast<const std::byte*>(word(source, 4));
        auto* const last = reinterpret_cast<const std::byte*>(word(source, 8));
        try {
            for (; first != last; first += 0x40, completed += 0x40)
                copy_native_gui_icon_state_00ab3a70(completed, first, context);
        } catch (...) {
            // AB4BDA..4BF6 destroys only completed records, in forward order.
            // The failed current record is not a constructed owning state.
            try {
                for (auto* cursor = allocated; cursor != completed; cursor += 0x40)
                    destroy_native_gui_icon_state_00ab3280(cursor, context);
            } catch (...) { std::terminate(); }
            throw;
        }
        word(destination, 8) = reinterpret_cast<Word>(completed); // AB599E only after full copy.
    } catch (...) {
        // AB59B6 calls AB5850 with the still-unpublished end==begin, freeing
        // actual CURRENT backing and clearing the header, then rethrows.
        try { destroy_native_gui_icon_states_00ab5850(destination, context); }
        catch (...) { std::terminate(); }
        throw;
    }
}
void* construct_native_gui_icon_default_00ab5c60(void* raw, const volatile Word& actual_one) {
    require_storage(raw);
    construct_native_gui_widget_base_00aa9390(raw, 6, actual_one);
    const Word one = actual_one; // AB5C80, after complete raw base.
    word(raw) = 0x00d5c4c0;
    write<std::uint16_t>(raw, 0xec, 0xffff);
    word(raw, 0xf4) = 0;
    word(raw, 0xf8) = 0;
    word(raw, 0xfc) = 0;
    write<std::uint8_t>(raw, 0x101, 0);
    write<std::uint8_t>(raw, 0x100, 1);
    word(raw, 0x104) = 0;
    word(raw, 0x108) = 0;
    write<std::uint8_t>(raw, 0x10c, 0);
    word(raw, 0x110) = 0;
    word(raw, 0x114) = one;
    word(raw, 0x118) = 0;
    word(raw, 0x11c) = 0;
    word(raw, 0x120) = one;
    word(raw, 0x124) = one;
    write<std::uint16_t>(raw, 0x12c, 0);
    write<std::uint8_t>(raw, 0x134, 0);
    word(raw, 0x128) = 0;
    word(raw, 0x130) = 0;
    return raw;
}
void* construct_native_gui_icon_copy_00ab5d30(void* raw, const void* source,
    NativeGuiIconCopyContext& context, NativeGuiIconCopyAcquired& acquired) {
    require_storage(raw);
    require_storage(source);
    if (raw == source || acquired.started || acquired.base.started)
        throw std::invalid_argument("Icon copy requires distinct actual storage and a fresh retained frame");
    if (!context.storage.icon_profile_00d5c4c0)
        throw std::invalid_argument("Icon copy requires its genuine actual native table");
    acquired.started = true;
    try {
        construct_native_gui_widget_copy_00aa9520(raw, source, context.base, acquired.base);
        acquired.base_returned = true;
        word(raw) = 0x00d5c4c0;
        acquired.native_eh_state = 0;
        write(raw, 0xec, read<std::uint16_t>(source, 0xec));
        acquired.vector_entered = true;
        copy_native_gui_icon_states_00ab58f0(at(raw, 0xf0), at(source, 0xf0), context.storage);
        acquired.states_returned = true;
        write(raw, 0x100, read<std::uint8_t>(source, 0x100));
        write(raw, 0x101, read<std::uint8_t>(source, 0x101));
        word(raw, 0x104) = 0;
        word(raw, 0x108) = 0;
        write(raw, 0x10c, read<std::uint8_t>(source, 0x10c));
        word(raw, 0x110) = word(source, 0x110);
        copy_ratio_enter_state(raw, source, &acquired.native_eh_state); // AB5DD1 between FLD/FSTP114.
        for (Word offset = 0x118; offset <= 0x124; offset += 4) word(raw, offset) = word(source, offset);
        const Word captured_profile = copy_timer_capture_profile(raw, source);
        write(raw, 0x12c, read<std::uint16_t>(source, 0x12c));
        const Word current_word = read<std::uint16_t>(raw, 0xec); // AB5E28, before130/134.
        copy_x87(at(raw, 0x130), at(source, 0x130));
        write(raw, 0x134, read<std::uint8_t>(source, 0x134));
        if (captured_profile != 0x00d5c4c0)
            throw std::logic_error("Icon copy captured an unsupported native current table");
        const Word target = context.storage.icon_profile_00d5c4c0[0x80 / 4];
        if (target != 0x00ab3cb0)
            throw std::logic_error("Icon copy requires its genuine selected native current80 target");
        acquired.current80_entered = true;
        context.current.call_rebuild_current80(raw, target, current_word);
        acquired.complete = true;
        return raw;
    } catch (...) {
        acquired.failure = std::current_exception();
        acquired.cleanup_entered = acquired.native_eh_state >= 0;
        unwind_icon(raw, acquired.native_eh_state, context.storage);
        acquired.failed = true;
        throw;
    }
}
void* create_native_gui_icon_default_00aa1410(const volatile Word& one) {
    void* const slot = allocate_gui_widget_pool_slot<GuiIconPoolTraits>();
    if (!slot) return nullptr;
    try { return construct_native_gui_icon_default_00ab5c60(slot, one); }
    catch (...) { return_failed_slot(slot); throw; }
}
void* create_native_gui_icon_copy_00aa1410(const void* source,
    NativeGuiIconCopyContext& context, NativeGuiIconCopyAcquired& acquired) {
    require_storage(source);
    const void* const captured_source = source;
    void* const slot = allocate_gui_widget_pool_slot<GuiIconPoolTraits>();
    if (!slot) return nullptr;
    try { return construct_native_gui_icon_copy_00ab5d30(slot, captured_source, context, acquired); }
    catch (...) { return_failed_slot(slot); throw; }
}
void NativeGuiIconPreparedCancelCall::destroy_icon_body_00ab5e70(void* raw,
    NativeGuiIconStorageContext& context, NativeGuiIconDestroyAcquired& acquired) {
    void* const captured_receiver = raw;
    word(captured_receiver) = 0x00d5c4c0;
    stage_owner_push(captured_receiver); // AB5E93, SAME HOME lives across getter callbacks.
    acquired.native_eh_state = 2;
    acquired.phase = NativeGuiIconDestroyAcquired::Phase::delayed_getter;
    auto* const delayed = call_getter(); // Real65, no old get/invoke/owned filters.
    acquired.phase = NativeGuiIconDestroyAcquired::Phase::delayed_cancel;
    call_cancel(delayed); // Real57, no dereference/snapshot/restage of HOME.
    acquired.native_eh_state = 1;
    acquired.phase = NativeGuiIconDestroyAcquired::Phase::shader;
    destroy_native_string_header_0041dd20(at(captured_receiver, 0x104), context.strings);
    acquired.native_eh_state = 0;
    acquired.phase = NativeGuiIconDestroyAcquired::Phase::states;
    destroy_native_gui_icon_states_00ab5850(at(captured_receiver, 0xf0), context);
    acquired.native_eh_state = -1;
    acquired.phase = NativeGuiIconDestroyAcquired::Phase::base;
    call_base(captured_receiver, context.lifetime); // Frame's actual selected3arg AA9730 body.
}
void destroy_native_gui_icon_prepared_00ab5e70(void* raw,
    NativeGuiIconStorageContext& context, NativeGuiIconDestroyAcquired& acquired,
    NativeGuiIconPreparedCancelCall& prepared) {
    void* const captured_identity = raw;
    try {
        prepared.enter(captured_identity, context, acquired); // Before late admission/profile effects.
        begin_destroy(captured_identity, context, acquired);
        prepared.destroy_icon_body_00ab5e70(captured_identity, context, acquired);
        acquired.complete = true;
        acquired.phase = NativeGuiIconDestroyAcquired::Phase::complete;
        prepared.normal_return();
    } catch (...) {
        prepared.note_unknown(); // Whole selected Icon scope, not just get/cancel.
        acquired.failure = std::current_exception();
        acquired.failed = true;
        acquired.phase = NativeGuiIconDestroyAcquired::Phase::failed;
        throw; // NEVER old destroy_failed/unwind_icon; retain remaining graph.
    }
}
void* delete_native_gui_icon_prepared_00ab6100(void* raw,
    const volatile Word& actual_flags_home, NativeGuiIconStorageContext& context,
    NativeGuiIconDestroyAcquired& acquired, NativeGuiIconPreparedCancelCall& prepared) {
    void* const captured_identity = raw;
    try {
        prepared.enter(captured_identity, context, acquired);
        begin_destroy(captured_identity, context, acquired);
        prepared.destroy_icon_body_00ab5e70(captured_identity, context, acquired);
        const auto low = *reinterpret_cast<const volatile unsigned char*>(&actual_flags_home);
        if ((low & 1u) != 0) { // ONE lateBYTE6108, no incoming/fullDWORD snapshot.
            acquired.phase = NativeGuiIconDestroyAcquired::Phase::pool_return;
            free_gui_widget_pool_slot<GuiIconPoolTraits>(captured_identity);
        }
        acquired.complete = true;
        acquired.phase = NativeGuiIconDestroyAcquired::Phase::complete;
        prepared.normal_return(); // Source metadata only, no raw receiver read after free.
        return captured_identity;
    } catch (...) {
        prepared.note_unknown();
        acquired.failure = std::current_exception();
        acquired.failed = true;
        acquired.phase = NativeGuiIconDestroyAcquired::Phase::failed;
        throw;
    }
}

void destroy_native_gui_icon_00ab5e70(void* raw, NativeGuiIconStorageContext& context,
    Destroy& acquired) {
    begin_destroy(raw, context, acquired);
    try { destroy_body(raw, context, acquired); }
    catch (...) { destroy_failed(raw, context, acquired); throw; }
    acquired.complete = true;
    acquired.phase = Destroy::Phase::complete;
}
void* delete_native_gui_icon_00ab6100(void* raw, const volatile Word& flags,
    NativeGuiIconStorageContext& context, Destroy& acquired) {
    begin_destroy(raw, context, acquired);
    try {
        destroy_body(raw, context, acquired);
        const auto low = *reinterpret_cast<const volatile unsigned char*>(&flags);
        if ((low & 1u) != 0) {
            acquired.phase = Destroy::Phase::pool_return;
            free_gui_widget_pool_slot<GuiIconPoolTraits>(raw);
        }
    } catch (...) { destroy_failed(raw, context, acquired); throw; }
    acquired.complete = true;
    acquired.phase = Destroy::Phase::complete;
    return raw;
}
} // namespace bsp
