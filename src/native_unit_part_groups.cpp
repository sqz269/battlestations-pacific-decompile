#include "bsp/native_unit_part_groups.hpp"
#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/native_singleton_vector_leaves.hpp"
#include "bsp/native_singleton_vector_registration_wrappers.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>
#include <new>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(address(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(address(p) + offset) = value;
}
Word distance(Word end, Word begin, unsigned shift) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(end - begin) >> shift);
}
Word count_rows(void* header) noexcept {
    const Word begin = read(header, 4);
    return begin ? distance(read(header, 8), begin, 4) : 0;
}
void empty_row(void* row) noexcept {
    write(row, 4, 0); write(row, 8, 0); write(row, 12, 0);
}
void clear_rows(Word first, Word last) noexcept {
    for (; first != last; first += 16)
        clear_native_singleton_storage_00bd0220(pointer(first), nullptr);
}
struct CompletedName {
    NativeLegacySboStringStorage& value;
    ~CompletedName() noexcept { native_legacy_sbo_string_destroy_004072d0(value); }
};
// The native inline temporary destructors free only when capacity>=16.
// Reinitialization by the next constructor is separate; do not move it earlier.
void release_temporary(NativeLegacySboStringStorage& value) noexcept {
    if (read(&value, 0x18) >= 16) singleton_lifetime_free(pointer(read(&value, 4)));
}
} // namespace

NativeLegacySboStringStorage& copy_native_part_record_name_00711c30(
    const void* record, NativeLegacySboStringStorage& destination) {
    write(&destination, 0x18, 15);
    write(&destination, 0x14, 0);
    *reinterpret_cast<volatile char*>(&destination.buffer_04.inline_bytes[0]) = 0;
    const auto& source = *reinterpret_cast<const NativeLegacySboStringStorage*>(address(record) + 8);
    return native_legacy_sbo_string_assign_substring_00408120(destination, source, 0, 0xffffffffu);
}
NativeLegacySboStringStorage& construct_native_part_substring_004cdbe0(
    const NativeLegacySboStringStorage& source, NativeLegacySboStringStorage& destination,
    Word offset, Word count) {
    write(&destination, 0x14, 0);
    write(&destination, 0x18, 15);
    *reinterpret_cast<volatile char*>(&destination.buffer_04.inline_bytes[0]) = 0;
    return native_legacy_sbo_string_assign_substring_00408120(destination, source, offset, count);
}

void resize_native_part_group_rows_empty_00713270(void* header, Word requested) {
    const Word begin = read(header, 4);
    const Word size = begin ? distance(read(header, 8), begin, 4) : 0;
    if (size < requested) {
        const Word old_size = begin ? distance(read(header, 8), begin, 4) : 0;
        const Word position = read(header, 8);
        if (begin > position) _invalid_parameter_noinfo();
        const Word added = requested - old_size;
        // 712FF0's private fill copy is a known empty vector from 713380.
        // Its constructor, destructor and every fill copy therefore make no
        // allocation. The original unrelated opaque row word remains untouched.
        Word current_begin = read(header, 4);
        Word capacity = current_begin ? distance(read(header, 12), current_begin, 4) : 0;
        if (!added) return;
        Word current_size = current_begin ? distance(read(header, 8), current_begin, 4) : 0;
        if (0x0fffffffu - current_size < added) native_singleton_length_error_00bd0590();
        current_size = current_begin ? distance(read(header, 8), current_begin, 4) : 0;
        if (capacity < current_size + added) {
            const Word half = capacity >> 1;
            capacity = 0x0fffffffu - half < capacity ? 0 : capacity + half;
            current_size = current_begin ? distance(read(header, 8), current_begin, 4) : 0;
            if (capacity < current_size + added) {
                current_size = current_begin ? distance(read(header, 8), current_begin, 4) : 0;
                capacity = current_size + added;
            }
            if (capacity > 0x0fffffffu) throw std::bad_alloc();
            const Word replacement = address(singleton_lifetime_allocate({
                SingletonAllocationKind::object, capacity * 16u, capacity * 16u}));
            Word destination = replacement;
            // 712A10 transfers each prior row through an initialized empty row.
            for (Word source = read(header, 4); source != position; source += 16, destination += 16) {
                empty_row(pointer(destination));
                for (Word offset = 4; offset <= 12; offset += 4) {
                    const Word old = read(pointer(destination), offset);
                    write(pointer(destination), offset, read(pointer(source), offset));
                    write(pointer(source), offset, old);
                }
            }
            for (Word i = 0; i < added; ++i, destination += 16) empty_row(pointer(destination));
            current_begin = read(header, 4);
            current_size = current_begin ? distance(read(header, 8), current_begin, 4) : 0;
            if (current_begin) {
                clear_rows(current_begin, read(header, 8));
                singleton_lifetime_free(pointer(read(header, 4)));
            }
            write(header, 12, replacement + capacity * 16);
            write(header, 8, replacement + (added + current_size) * 16);
            write(header, 4, replacement);
        } else {
            // Position is the captured end, so the generic tail move and
            // assignment ranges are empty. Only the new rows are constructed.
            const Word end = read(header, 8);
            for (Word i = 0; i < added; ++i) empty_row(pointer(end + i * 16));
            write(header, 8, read(header, 8) + added * 16);
        }
    } else if (begin) {
        const Word end = read(header, 8);
        if (requested < distance(end, begin, 4)) {
            if (begin > end) _invalid_parameter_noinfo();
            const Word current_begin = read(header, 4);
            if (current_begin > read(header, 8)) _invalid_parameter_noinfo();
            const Word first = current_begin + requested * 16;
            if (first > read(header, 8) || first < read(header, 4)) _invalid_parameter_noinfo();
            if (first != end) {
                // 712900's move range starts at old end, hence no assignment.
                clear_rows(first, read(header, 8));
                write(header, 8, first);
            }
        }
    }
}

void build_native_unit_part_groups_00713380(void* part) {
    const Word set = read(part, 0x160);
    Word cursor = read(pointer(set), 0x80);
    void* const records = pointer(set + 0x7c);
    if (cursor > read(pointer(set), 0x84)) _invalid_parameter_noinfo();
    for (;;) {
        const Word current_set = read(part, 0x160);
        const Word end = read(pointer(current_set), 0x84);
        if (read(pointer(current_set), 0x80) > end) _invalid_parameter_noinfo();
        if (address(records) != current_set + 0x7c) _invalid_parameter_noinfo();
        if (cursor == end) return;
        if (cursor >= read(records, 8)) _invalid_parameter_noinfo();
        {
            NativeLegacySboStringStorage name;
            copy_native_part_record_name_00711c30(pointer(read(pointer(cursor))), name);
            const CompletedName completed{name};
            NativeLegacySboStringStorage temporary;
            auto& prefix = construct_native_part_substring_004cdbe0(name, temporary, 0, 6);
            const Word length = read(&prefix, 0x14);
            const Word compared = length < 6 ? length : 6;
            // 4B3FC0 is unsigned byte lexicographic comparison. Only zero/nonzero
            // is observed here; use the actual source CRT operation, not a stub.
            const bool matches = std::memcmp(prefix.data(), "damage", compared) == 0 && length == 6;
            release_temporary(temporary);
            if (matches) {
                auto& suffix = construct_native_part_substring_004cdbe0(name, temporary, 6, 0xffffffffu);
                const Word index = static_cast<Word>(std::atol(suffix.data())) - 1;
                release_temporary(temporary);
                void* const groups = pointer(address(part) + 0x168);
                write(&temporary, 0x18, 15);
                write(&temporary, 0x14, 0);
                temporary.buffer_04.inline_bytes[0] = 0;
                if (!read(groups, 4) || count_rows(groups) <= index)
                    resize_native_part_group_rows_empty_00713270(groups, index + 1);
                if (!read(groups, 4) || index >= count_rows(groups)) _invalid_parameter_noinfo();
                void* const row = pointer(read(groups, 4) + index * 16);
                if (cursor >= read(records, 8)) _invalid_parameter_noinfo();
                // Full 506DD0/505D50/5058E0 and their six leaf dependencies match
                // this existing actual-storage implementation after relocation.
                append_native_singleton_slot_00bd0bc0(row, nullptr, pointer(cursor + 4));
            }
        } // Destroy the complete name before checking the iterator again.
        if (cursor >= read(records, 8)) _invalid_parameter_noinfo();
        cursor += 8;
    }
}
} // namespace bsp
