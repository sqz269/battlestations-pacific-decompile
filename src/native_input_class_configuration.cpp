#include "bsp/native_input_class_configuration.hpp"
#include "bsp/native_input_backend_bindings.hpp"
#include "bsp/native_input_configuration_storage.hpp"
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

} // namespace

void configure_native_input_class_00a917e0(void* actual_backend, Word device_class,
    Word requested_count, const void* optional_identifiers) {
    const auto group = address(actual_backend) + device_class * 0x24u;
    write(group, 0x68, requested_count); // A917FB, before any validation/copy
    clear_input_checked_word_storage(pointer(group + 0x6cu)); // A917FE..A91824, active borrowed pointers
    const auto identifiers = group + 0x7cu;
    clear_input_checked_word_storage(pointer(identifiers)); // A91829..A91852
    if (optional_identifiers) { // A91857: native parameter read after both erases
        assign_input_checked_word_storage(pointer(identifiers), optional_identifiers);
        return;
    }
    if (!requested_count) return;
    const Word wildcard = 0xffffffffu;
    do {
        append_identifier(identifiers, &wildcard); // A91887
    } while (--requested_count != 0);
}
} // namespace bsp
