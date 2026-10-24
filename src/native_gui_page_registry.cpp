#include "bsp/native_gui_page_registry.hpp"

#include "bsp/native_gui_media_focus_lifetime.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/gui_widgets_cc13.hpp"

#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));

Word address(const void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void* at(void* base, Word offset) noexcept { return pointer(address(base) + offset); }
volatile Word& word(void* base, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(base, offset));
}
const volatile Word& word(const void* base, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(pointer(address(base) + offset));
}
void validate(bool valid) {
    if (!valid) _invalid_parameter_noinfo();
}
void shift_after_first(void* vector, Word cursor) {
    const Word following = cursor + 4;
    const Word end = word(vector, 8);
    const std::int32_t count = static_cast<std::int32_t>(end - following) >> 2;
    if (count > 0) {
        const Word bytes = static_cast<Word>(count) * 4;
        (void)memmove_s(pointer(cursor), bytes, pointer(following), bytes);
    }
    word(vector, 8) = word(vector, 8) - 4;
}
void insert_at(void* vector, Word position, void* page) {
    Word result_iterator[2];
    // 004D3B70/004D1B20 and the existing BD08D0/BD0700 adapter have the
    // same raw +4/+8/+C four-byte-slot layout and one-element insertion
    // contract. Borrow its complete checked-insertion/growth implementation.
    insert_one_native_singleton_slots_checked_00bd08d0(vector, nullptr,
        result_iterator, vector, pointer(position), &page);
}
} // namespace

void* find_native_gui_page_00aa3140(void* manager, const void* requested_name,
    NativeGuiPageLookupBindings& bindings) {
    void* const vector = at(manager, 0x14);
    Word cursor = word(vector, 4); // AA3146, native ESI.
    validate(cursor <= word(vector, 8));
    for (;;) {
        const Word end = word(vector, 8); // AA3158, before current begin.
        validate(word(vector, 4) <= end);
        // Native CMP EBX,EBX at3165 is an always-equal container check.
        if (cursor == end) return nullptr;
        validate(cursor < word(vector, 8));
        void* const page = pointer(word(pointer(cursor)));
        const Word profile = word(page); // AA317E, CURRENT receiver/profile.
        const auto* const table = bindings.current_page_table(page, profile);
        if (!table)
            throw std::logic_error("Raw Page lookup requires its actual current table");
        const Word target = table[0x7c / 4];
        const void* const name = target == 0x00ac3fa0u
            ? gui_widgets_cc13::gui_layer_name_00ac3fa0(page)
            : bindings.call_page_name_7c(page, target);
        const Word actual_length = word(name); // AA3185 BEFORE incoming length.
        const Word requested_length = word(requested_name);
        bool equal = false;
        if (actual_length == requested_length) {
            if (actual_length == 0) {
                equal = requested_length == 0;
            } else if (requested_length != 0) {
                const auto* const incoming_data =
                    reinterpret_cast<const char*>(word(requested_name, 4));
                const auto* const actual_data =
                    reinterpret_cast<const char*>(word(name, 4));
                equal = _stricmp(actual_data, incoming_data) == 0;
            }
        }
        if (equal) {
            validate(cursor < word(vector, 8)); // AA31CB, AFTER callback/compare.
            return pointer(word(pointer(cursor))); // AA31D5: reload slot, not page.
        }
        validate(cursor < word(vector, 8)); // AA31BC, before advancing.
        cursor += 4;
    }
}

void remove_native_gui_page_pointer_00aa30c0(void* manager, void* page) {
    void* const vector = at(manager, 0x14);
    Word cursor = word(manager, 0x18);
    validate(cursor <= word(manager, 0x1c));
    for (;;) {
        const Word end = word(vector, 8);
        validate(word(vector, 4) <= end);
        if (cursor == end) return;
        validate(cursor < word(vector, 8));
        if (word(pointer(cursor)) == address(page)) {
            shift_after_first(vector, cursor);
            return;
        }
        validate(cursor < word(vector, 8));
        cursor += 4;
    }
}

void register_native_gui_page_00aa52a0(void* manager, void* page) {
    remove_native_gui_page_pointer_00aa30c0(manager, page);
    void* const vector = at(manager, 0x14);
    Word cursor = word(manager, 0x18);
    validate(cursor <= word(vector, 8));
    for (;;) {
        const Word end = word(vector, 8);
        validate(word(vector, 4) <= end);
        if (cursor == end) break;
        validate(cursor < word(vector, 8));
        const Word existing = word(pointer(cursor));
        const auto incoming_priority = static_cast<std::int32_t>(word(page, 0xfc));
        const auto existing_priority = static_cast<std::int32_t>(word(pointer(existing), 0xfc));
        if (incoming_priority < existing_priority) {
            insert_at(vector, cursor, page);
            return;
        }
        validate(cursor < word(vector, 8));
        cursor += 4;
    }

    const Word begin = word(vector, 4);
    if (begin != 0) {
        const Word count = static_cast<Word>(
            static_cast<std::int32_t>(word(vector, 8) - begin) >> 2);
        const Word capacity = static_cast<Word>(
            static_cast<std::int32_t>(word(vector, 0x0c) - begin) >> 2);
        if (count < capacity) {
            const Word end = word(vector, 8);
            word(pointer(end)) = address(page);
            word(vector, 8) = end + 4;
            return;
        }
    }
    const Word end = word(vector, 8);
    validate(begin <= end);
    insert_at(vector, end, page);
}

void remove_and_destroy_native_gui_page_00aa31f0(void* manager, void* page,
    NativeGuiPageLifetimeHost& virtuals) {
    void* const vector = at(manager, 0x14);
    Word cursor = word(manager, 0x18);
    validate(cursor <= word(manager, 0x1c));
    for (;;) {
        const Word end = word(vector, 8);
        validate(word(vector, 4) <= end);
        if (cursor == end) return;
        validate(cursor < word(vector, 8));
        if (word(pointer(cursor)) == address(page)) {
            shift_after_first(vector, cursor);
            virtuals.call_current_page_virtual_20(page);
            virtuals.call_current_page_virtual_04(page, 1);
            return;
        }
        validate(cursor < word(vector, 8));
        cursor += 4;
    }
}
} // namespace bsp
