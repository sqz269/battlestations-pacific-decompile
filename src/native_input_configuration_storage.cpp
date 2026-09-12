#include "bsp/native_input_configuration_storage.hpp"
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
void erase_range_contents(Word header, Word first, Word last) {
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
    if (!header) invalid_parameter(); // the original equal-owner call
    erase_range_contents(header, captured_begin, captured_end);
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

void clear_input_checked_word_storage(void* header) {
    clear_checked_range(address(header));
}
void* assign_input_checked_word_storage(void* destination, const void* source) {
    assign_identifier_storage(address(destination), address(source));
    return destination;
}
void* erase_input_checked_word_storage(void* header, void* output,
    const void* first_owner, void* first_position,
    const void* last_owner, void* last_position) {
    const auto owner = address(first_owner);
    if (!owner || owner != address(last_owner)) invalid_parameter();
    const auto first = address(first_position);
    const auto last = address(last_position);
    erase_range_contents(address(header), first, last);
    write(address(output), 4, first);
    write(address(output), 0, owner);
    return output;
}
void* NativeInputConfigurationStorage::call_00697bd0(void* destination, const void* source) {
    return assign_input_checked_word_storage(destination, source);
}
void* NativeInputConfigurationStorage::call_006977f0(void* header, void* output,
    const void* first_owner, void* first_position,
    const void* last_owner, void* last_position) {
    return erase_input_checked_word_storage(header, output,
        first_owner, first_position, last_owner, last_position);
}
} // namespace bsp
