#include "bsp/native_gui_grid_storage.hpp"
#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeGuiGridColumnStorage) == 0x14);
static_assert(sizeof(NativeGuiGridTailStorage) == 0x38);
static_assert(offsetof(NativeGuiGridTailStorage, columns_fc) == 0x10);
static_assert(offsetof(NativeGuiGridTailStorage, visual_group_118) == 0x2c);
Word read(const void* p, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(static_cast<const std::byte*>(p) + offset);
}
void store(void* p, std::size_t offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(static_cast<std::byte*>(p) + offset) = value;
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* add(void* p, Word bytes) noexcept { return pointer(bits(p) + bytes); }
const void* add(const void* p, Word bytes) noexcept { return pointer(bits(p) + bytes); }
void require_storage(void* p) {
    if (!p || bits(p) % 4) throw std::invalid_argument("Grid requires aligned actual124h storage");
}
void checked(bool value) {
    if (!value) throw std::invalid_argument("Grid requires its valid current native iterator range");
}
struct Range { Word first, last, end, count; };
Range range(const void* vector, Word stride) {
    const Word first = read(vector, 4), last = read(vector, 8), end = read(vector, 12);
    checked((!first && !last && !end) || (first && last >= first && end >= last &&
        !(first % 4) && !((last - first) % stride) && !((end - first) % stride)));
    // Native size() uses signed ptrdiff_t (IMUL/SAR division by14h), then
    // converts to the unsigned size. Preserve the length-guard input too.
    const Word count = first ? static_cast<Word>(static_cast<std::int32_t>(last - first) /
        static_cast<std::int32_t>(stride)) : 0;
    return {first, last, end, count};
}
template<class Body> void unwind(Body body) noexcept {
    try { body(); } catch (...) { std::terminate(); }
}
void copy_float(void* destination, const void* source) {
    __asm {
        mov eax, destination
        mov edx, source
        fld dword ptr [edx]
        fstp dword ptr [eax]
    }
}
void tidy_columns(void* vector) {
    if (void* first = pointer(read(vector, 4))) {
        destroy_native_gui_grid_columns_00aca730(first, pointer(read(vector, 8)));
        singleton_lifetime_free(pointer(read(vector, 4))); // CURRENT after inner frees
    }
    store(vector, 4, 0); store(vector, 8, 0); store(vector, 12, 0);
}
void copy_pointers(void* destination, const void* source, NativeGuiGridColumnCopyAcquired& a) {
    const Word initial_first = read(source, 4);
    const Word count = initial_first ? static_cast<Word>(static_cast<std::int32_t>(
        read(source, 8) - initial_first) >> 2) : 0;
    store(destination, 4, 0); store(destination, 8, 0); store(destination, 12, 0);
    a.current_inner_allocation = nullptr;
    if (!count) return;
    if (count > 0x3fffffff) throw std::length_error("vector<T> too long");
    void* const allocated = singleton_lifetime_allocate({SingletonAllocationKind::object, count * 4u, count * 4u});
    a.current_inner_allocation = allocated;
    store(destination, 4, bits(allocated)); store(destination, 8, bits(allocated));
    store(destination, 12, bits(allocated) + count * 4u);
    const Word captured_last = read(source, 8);
    checked(read(source, 4) <= captured_last);
    const Word current_first = read(source, 4);
    checked(current_first <= read(source, 8));
    const Word output = read(destination, 4);
    const Word bytes = static_cast<Word>(static_cast<std::int32_t>(captured_last - current_first) >> 2) * 4u;
    if (bytes) std::memmove(pointer(output), pointer(current_first), bytes);
    store(destination, 8, output + bytes);
    // AA4C50 has no own cleanup. Its caller's placement-delete401130 is RET;
    // do not add cleanup of a partially entered current column here.
}
void begin(NativeGuiGridRetirementAcquired& a) {
    if (a.phase != decltype(a.phase)::fresh) throw std::logic_error("Grid retirement cannot replay");
    a.phase = decltype(a.phase)::entered;
}
void require_canonical_child(void* child, NativeGuiGridContext& c) {
    auto& reference = c.actual_owners.resolve_actual(child);
    if (static_cast<void*>(&reference.reference_count) != add(child, 4))
        throw std::logic_error("Grid child must use the SAME canonical actual +04 reference");
}
void destroy_column_children(void* column, NativeGuiGridContext& c, NativeGuiGridRetirementAcquired& a) {
    Word cursor = read(column, 8);
    checked(cursor <= read(column, 12));
    for (;;) {
        const Word current_end = read(column, 12);
        checked(read(column, 8) <= current_end);
        if (cursor == current_end) return;
        checked(cursor < read(column, 12));
        if (void* child = pointer(read(pointer(cursor)))) {
            require_canonical_child(child, c);
            const Word captured_profile = read(child);
            const volatile Word* const table = c.lifetime.bindings.current_table(child, captured_profile);
            if (!table) throw std::logic_error("Grid child requires its genuine current native table");
            const Word target = table[1];
            if (!target) throw std::logic_error("Grid child requires its genuine current04 scalar binding");
            a.active_child = child; a.child_target = target;
            a.child_call_entered = true; a.child_call_returned = false;
            c.lifetime.bindings.call_child_delete_04(child, target, 1);
            a.child_call_returned = true;
        }
        checked(cursor < read(column, 12)); // CURRENT after child callback
        cursor += 4; // native progress only; never erase/clear/repair the cell
    }
}
void clear_zero(void* raw, NativeGuiGridContext& c, NativeGuiGridRetirementAcquired& a) {
    const auto old = static_cast<std::int32_t>(read(raw, 0xfc));
    if (!old) return; // native equality arm touches no derived storage
    store(raw, 0xfc, 0);
    for (std::int32_t i = 0; i < old; ++i) {
        a.column_index = i;
        const auto current = range(add(raw, 0xec), 0x14);
        checked(current.first && static_cast<Word>(i) < current.count);
        destroy_column_children(pointer(read(raw, 0xf0) + static_cast<Word>(i) * 0x14), c, a);
    }
    if (read(raw, 0xfc) != 0) {
        a.unsupported_growth = true;
        throw std::logic_error("Grid count0 callback changed FC: genuine nonzero resize/growth remains required");
    }
    // Concrete ACB870(0)->ACB170 erase-all path. Empty/retained capacity stays;
    // AC AF10 has an empty source range here, then ACA730 frees inner backings.
    void* const vector = add(raw, 0xec);
    const auto current = range(vector, 0x14);
    if (current.first && current.count) {
        void* const new_end = pointer(read(vector, 4));
        checked(bits(new_end) <= read(vector, 8));
        destroy_native_gui_grid_columns_00aca730(new_end, pointer(read(vector, 8)));
        store(vector, 8, bits(new_end));
    }
}
void destroy(void* raw, NativeGuiGridContext& c, NativeGuiGridRetirementAcquired& a) {
    store(raw, 0, 0x00d5ce60);
    try {
        clear_zero(raw, c, a);
        tidy_columns(add(raw, 0xec));
    } catch (...) {
        // Unsupported source branch is a host boundary, not a native throw:
        // retain reached graph rather than fabricating its missing growth/EH.
        if (a.unsupported_growth) throw;
        a.columns_cleanup_entered = true;
        unwind([&] { tidy_columns(add(raw, 0xec)); });
        a.base_cleanup_entered = true;
        unwind([&] { destroy_native_gui_widget_base_00aa9730(raw, c.lifetime); });
        throw;
    }
    a.base_entered = true;
    destroy_native_gui_widget_base_00aa9730(raw, c.lifetime);
}
void return_failed_slot(void* slot) noexcept {
    unwind([&] { free_gui_widget_pool_slot<GuiGridPoolTraits>(slot); });
}
} // namespace
NativeGuiGridTailStorage& native_gui_grid_tail(void* raw) {
    require_storage(raw);
    return *reinterpret_cast<NativeGuiGridTailStorage*>(add(raw, 0xec));
}
void destroy_native_gui_grid_columns_00aca730(void* first, void* end) {
    for (void* current = first; current != end; current = add(current, 0x14)) {
        if (void* backing = pointer(read(current, 8))) singleton_lifetime_free(backing);
        store(current, 8, 0); store(current, 12, 0); store(current, 16, 0);
    }
}
void* copy_native_gui_grid_columns_00acb290(void* destination, const void* source,
    NativeGuiGridColumnCopyAcquired& a) {
    if (!destination || !source || destination == source || a.started)
        throw std::invalid_argument("Grid columns require distinct live actual headers and fresh diagnostics");
    const auto initial = range(source, 0x14); // supported valid source domain, before entry
    a.started = true;
    store(destination, 4, 0); store(destination, 8, 0); store(destination, 12, 0);
    if (!initial.count) { a.complete = true; return destination; }
    if (initial.count > 0x0ccccccc) throw std::length_error("vector<T> too long");
    void* const allocated = singleton_lifetime_allocate({SingletonAllocationKind::object,
        initial.count * 0x14u, initial.count * 0x14u});
    a.outer_allocation = allocated;
    store(destination, 4, bits(allocated)); store(destination, 8, bits(allocated));
    store(destination, 12, bits(allocated) + initial.count * 0x14u);
    void* completed = allocated;
    a.completed_prefix_end = completed;
    try {
        const Word captured_last = read(source, 8);
        checked(read(source, 4) <= captured_last);
        Word cursor = read(source, 4);
        checked(cursor <= read(source, 8));
        void* output = pointer(read(destination, 4));
        for (; cursor != captured_last; cursor += 0x14, output = add(output, 0x14)) {
            a.current_column = output;
            if (output) {
                copy_float(output, pointer(cursor));
                copy_pointers(add(output, 4), add(pointer(cursor), 4), a);
            }
            completed = add(output, 0x14); a.completed_prefix_end = completed;
        }
        store(destination, 8, bits(output));
    } catch (...) {
        a.range_cleanup_entered = true;
        unwind([&] { destroy_native_gui_grid_columns_00aca730(allocated, completed); });
        a.outer_cleanup_entered = true;
        unwind([&] { tidy_columns(destination); });
        throw;
    }
    a.complete = true;
    return destination;
}
void* construct_native_gui_grid_default_00acb0a0(void* raw, const volatile Word& one) {
    require_storage(raw);
    construct_native_gui_widget_base_00aa9390(raw, 8, one);
    store(raw, 0, 0x00d5ce60);
    for (Word offset : {0xf0u, 0xf4u, 0xf8u, 0xfcu, 0x100u, 0x104u, 0x108u}) store(raw, offset, 0);
    *reinterpret_cast<volatile std::uint8_t*>(add(raw, 0x10c)) = 1;
    for (Word offset : {0x110u, 0x114u, 0x118u, 0x11cu, 0x120u}) store(raw, offset, 0);
    return raw;
}
void* construct_native_gui_grid_copy_00acb3f0(void* raw, const void* source,
    NativeGuiGridCopyContext& c, NativeGuiGridCopyAcquired& a) {
    require_storage(raw);
    if (!source || a.base_returned || a.columns_returned || a.complete)
        throw std::invalid_argument("Grid copy requires live source and fresh acquired frame");
    construct_native_gui_widget_copy_00aa9520(raw, source, c.base, a.base);
    a.base_returned = true; a.native_eh_state = 0;
    store(raw, 0, 0x00d5ce60);
    alignas(4) Word name[2];
    try {
        copy_native_gui_grid_columns_00acb290(add(raw, 0xec), add(source, 0xec), a.columns);
        a.columns_returned = true;
        copy_float(add(raw, 0x104), add(source, 0x104));
        copy_float(add(raw, 0x108), add(source, 0x108));
        *reinterpret_cast<volatile std::uint8_t*>(add(raw, 0x10c)) =
            *reinterpret_cast<const volatile std::uint8_t*>(add(source, 0x10c));
        copy_float(add(raw, 0x110), add(source, 0x110));
        a.native_eh_state = 1;
        copy_float(add(raw, 0x114), add(source, 0x114));
        store(name, 0, 0); store(name, 4, 0);
        copy_float(add(raw, 0x11c), add(source, 0x11c));
        copy_float(add(raw, 0x120), add(source, 0x120));
        resize_native_string_header_0041dd40(name, c.grid.strings, 12, true);
        if (void* data = pointer(read(name, 4)))
            std::memcpy(data, "Visual_Group", static_cast<std::size_t>(read(name) + 1u));
        a.native_eh_state = 2;
        void* const found = c.grid.lookup.find_child_00aa7e00(raw, name, 1);
        if (found) require_canonical_child(found, c.grid);
        store(raw, 0x118, bits(found));
        a.native_eh_state = 1;
        destroy_native_string_header_0041dd20(name, c.grid.strings);
    } catch (...) {
        if (a.native_eh_state == 2) {
            a.native_eh_state = 1; a.string_cleanup_entered = true;
            unwind([&] { destroy_native_string_header_0041dd20(name, c.grid.strings); });
        }
        if (a.native_eh_state == 1) {
            a.native_eh_state = 0; a.columns_cleanup_entered = true;
            unwind([&] { tidy_columns(add(raw, 0xec)); });
        }
        a.native_eh_state = -1; a.base_cleanup_entered = true;
        unwind([&] { destroy_native_gui_widget_base_00aa9730(raw, c.grid.lifetime); });
        throw;
    }
    a.native_eh_state = -1; a.complete = true;
    return raw;
}
void* create_native_gui_grid_default_00aa1650(const volatile Word& one) {
    void* const slot = allocate_gui_widget_pool_slot<GuiGridPoolTraits>();
    if (!slot) return nullptr;
    try { return construct_native_gui_grid_default_00acb0a0(slot, one); }
    catch (...) { return_failed_slot(slot); throw; }
}
void* create_native_gui_grid_copy_00aa1650(const void* source,
    NativeGuiGridCopyContext& c, NativeGuiGridCopyAcquired& a) {
    if (!source) throw std::invalid_argument("Grid copy arm requires its captured nonnull source");
    const void* const captured = source;
    void* const slot = allocate_gui_widget_pool_slot<GuiGridPoolTraits>();
    if (!slot) return nullptr;
    try { return construct_native_gui_grid_copy_00acb3f0(slot, captured, c, a); }
    catch (...) { return_failed_slot(slot); throw; }
}
void preflight_native_gui_grid_clear_zero(void* raw) {
    require_storage(raw);
    const auto outer = range(add(raw, 0xec), 0x14);
    const auto count = static_cast<std::int32_t>(read(raw, 0xfc));
    checked(count >= 0 && static_cast<Word>(count) <= outer.count);
    for (Word current = outer.first; current != outer.last; current += 0x14)
        (void)range(add(pointer(current), 4), 4);
}
NativeGuiGridRetirementAcquired::~NativeGuiGridRetirementAcquired() noexcept {
    if (phase == Phase::entered || phase == Phase::failed) std::terminate();
}
void clear_native_gui_grid_zero_00acb9b0(void* raw, NativeGuiGridContext& c,
    NativeGuiGridRetirementAcquired& a) {
    preflight_native_gui_grid_clear_zero(raw); begin(a);
    try { clear_zero(raw, c, a); a.phase = decltype(a.phase)::complete; }
    catch (...) { a.phase = decltype(a.phase)::failed; throw; }
}
void destroy_native_gui_grid_00acbbf0(void* raw, NativeGuiGridContext& c,
    NativeGuiGridRetirementAcquired& a) {
    preflight_native_gui_grid_clear_zero(raw); begin(a);
    try { destroy(raw, c, a); a.phase = decltype(a.phase)::complete; }
    catch (...) { a.phase = decltype(a.phase)::failed; throw; }
}
void* delete_native_gui_grid_00acbc90(void* raw, const volatile Word& flags,
    NativeGuiGridContext& c, NativeGuiGridRetirementAcquired& a) {
    preflight_native_gui_grid_clear_zero(raw); begin(a);
    try {
        destroy(raw, c, a);
        const auto low = *reinterpret_cast<const volatile std::uint8_t*>(&flags);
        if (low & 1u) { free_gui_widget_pool_slot<GuiGridPoolTraits>(raw); a.slot_returned = true; }
        a.phase = decltype(a.phase)::complete; return raw;
    } catch (...) { a.phase = decltype(a.phase)::failed; throw; }
}
} // namespace bsp
