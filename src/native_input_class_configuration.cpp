#include "bsp/native_input_class_configuration.hpp"
#include "bsp/native_input_backend_bindings.hpp"
#include "bsp/native_singleton_vector_allocation.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
using Word = std::uint32_t;
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(Word p, Word offset = 0) noexcept {
    Word value;
    std::memcpy(&value, pointer(p + offset), 4);
    return value;
}
void write(Word p, Word offset, Word value) noexcept { std::memcpy(pointer(p + offset), &value, 4); }
Word distance(Word first, Word last) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(last - first) >> 2);
}
Word count(Word first, Word last) noexcept { return first ? distance(first, last) : 0; }
void invalid_parameter() { _invalid_parameter_noinfo(); }

// Stateless source storage contracts for the recognized vector operations.
// They own no parallel header/container and use the real returning CRT copy.
Word copy_words(Word first, Word last, Word destination) {
    const Word bytes = distance(first, last) * 4u;
    if (bytes) (void)memmove_s(pointer(destination), bytes, pointer(first), bytes);
    return destination + bytes;
}
void erase_captured_range(Word header, Word first, Word last) {
    if (!header) invalid_parameter(); // the two captured iterator owners agree
    if (first == last) return;
    const Word residual = distance(last, read(header, 8));
    const Word bytes = residual * 4u;
    const Word resulting_end = first + bytes;
    if (static_cast<std::int32_t>(residual) > 0)
        (void)memmove_s(pointer(first), bytes, pointer(last), bytes);
    write(header, 8, resulting_end);
}
void clear_checked_range(Word header) {
    const auto captured_end = read(header, 8);
    if (read(header, 4) > captured_end) invalid_parameter();
    const auto captured_begin = read(header, 4);
    if (captured_begin > read(header, 8)) invalid_parameter();
    erase_captured_range(header, captured_begin, captured_end);
}
void append_identifier(Word header, const Word* value) {
    const auto first = read(header, 4);
    if (first && count(first, read(header, 8)) < count(first, read(header, 0xc))) {
        const auto end = read(header, 8);
        write(end, 0, *value);
        write(header, 8, end + 4u);
        return;
    }
    const auto captured_end = read(header, 8);
    if (first > captured_end) invalid_parameter();
    Word iterator[2];
    // Same DWORD storage contract as pointer insertion; IDs are copied as
    // uninterpreted words, never converted to devices or retained/released.
    insert_input_active_pointer_storage(iterator, pointer(header), pointer(captured_end), value);
}
void assign_identifier_storage(Word destination, Word source) {
    if (destination == source) return; // after the caller has erased both ranges
    const auto first = read(source, 4);
    const auto last = read(source, 8);
    const auto size = count(first, last);
    if (!size) {
        clear_checked_range(destination);
        return;
    }
    const auto old_first = read(destination, 4);
    const auto old_size = count(old_first, read(destination, 8));
    if (size <= old_size) {
        (void)copy_words(first, last, old_first);
        const auto current_first = read(source, 4);
        const auto current_size = count(current_first, read(source, 8));
        write(destination, 8, read(destination, 4) + current_size * 4u);
        return;
    }
    if (size > count(old_first, read(destination, 0xc))) {
        if (old_first) singleton_lifetime_free(pointer(old_first));
        const auto current_first = read(source, 4);
        const auto required = count(current_first, read(source, 8));
        // The library assignment discards old storage before this allocation.
        // Its allocation helper clears all three pointers even if it throws.
        write(destination, 4, 0);
        write(destination, 8, 0);
        write(destination, 0xc, 0);
        if (!required) return;
        if (required > 0x3fffffffu) native_singleton_length_error_00bd0590();
        const auto bytes = required * 4u;
        const auto replacement = address(singleton_lifetime_allocate(
            {SingletonAllocationKind::object, bytes, bytes}));
        write(destination, 4, replacement);
        write(destination, 8, replacement);
        write(destination, 0xc, replacement + bytes);
        // Source endpoints and destination are read after allocation.
        const auto end = copy_words(read(source, 4), read(source, 8), read(destination, 4));
        write(destination, 8, end);
        return;
    }
    const auto middle = first + old_size * 4u;
    (void)copy_words(first, middle, old_first);
    const auto end = copy_words(middle, read(source, 8), read(destination, 8));
    write(destination, 8, end);
}
} // namespace

void configure_native_input_class_00a917e0(void* actual_backend, Word device_class,
    Word requested_count, const void* optional_identifiers) {
    const auto group = address(actual_backend) + device_class * 0x24u;
    write(group, 0x68, requested_count); // A917FB, before any validation/copy
    clear_checked_range(group + 0x6cu); // A917FE..A91824, active borrowed pointers
    const auto identifiers = group + 0x7cu;
    clear_checked_range(identifiers); // A91829..A91852
    if (optional_identifiers) { // A91857: native parameter read after both erases
        assign_identifier_storage(identifiers, address(optional_identifiers));
        return;
    }
    if (!requested_count) return;
    const Word wildcard = 0xffffffffu;
    do {
        append_identifier(identifiers, &wildcard); // A91887
    } while (--requested_count != 0);
}
} // namespace bsp
