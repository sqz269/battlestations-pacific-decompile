#include "bsp/native_vfs_date_leaf_providers.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS date leaf providers require MSVC Win32.
#endif

namespace bsp {
namespace {

const void* at_offset(const void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(
        reinterpret_cast<std::uintptr_t>(storage) + offset);
}
void* at_offset(void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(storage) + offset);
}
std::uint32_t read_word(const void* storage, std::uint32_t offset) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at_offset(storage, offset));
}
void write_word(void* storage, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at_offset(storage, offset)) = value;
}
void* read_pointer(const void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(read_word(storage, offset));
}
bool is_sentinel(const void* node) noexcept {
    return *static_cast<const volatile unsigned char*>(at_offset(node, 0x19)) != 0;
}
void write_date_descending(void* output, std::uint32_t word) noexcept {
    write_word(output, 0x10, word);
    write_word(output, 0x0c, word);
    write_word(output, 0x08, word);
    write_word(output, 0x04, word);
    write_word(output, 0x00, word);
}

} // namespace

bool less_native_string_headers_00443d00(
    const void* actual_left_header, const void* actual_right_header) {
    if (read_word(actual_left_header, 0) == 0) {
        return read_word(actual_right_header, 0) != 0;
    }
    if (read_word(actual_right_header, 0) == 0) {
        return false;
    }
    const auto* right_data = static_cast<const char*>(read_pointer(actual_right_header, 4));
    const auto* left_data = static_cast<const char*>(read_pointer(actual_left_header, 4));
    return _stricmp(left_data, right_data) < 0;
}

void* lower_bound_native_file_store_name_00be54d0(
    void* actual_tree, const void* actual_name_header) {
    auto* candidate = read_pointer(actual_tree, 4);
    auto* node = read_pointer(candidate, 4);
    while (!is_sentinel(node)) {
        if (less_native_string_headers_00443d00(at_offset(node, 0x0c), actual_name_header)) {
            node = read_pointer(node, 8);
        } else {
            candidate = node;
            node = read_pointer(node, 0);
        }
    }
    return candidate;
}

void* find_native_file_store_name_00be5a50(void* actual_tree,
    void* actual_iterator_output, const void* actual_name_header,
    const SingletonLifetimeCallbacks& invalid_parameters) {
    auto* node = lower_bound_native_file_store_name_00be54d0(actual_tree, actual_name_header);
    if (!actual_tree) {
        invalid_parameters.invalid_parameter(invalid_parameters.context);
    }
    if (node == read_pointer(actual_tree, 4) ||
        less_native_string_headers_00443d00(actual_name_header, at_offset(node, 0x0c))) {
        // BE5A90 rereads the current head after the comparison can call CRT.
        node = read_pointer(actual_tree, 4);
    }
    const auto owner_word = reinterpret_cast<std::uint32_t>(actual_tree);
    const auto node_word = reinterpret_cast<std::uint32_t>(node);
    write_word(actual_iterator_output, 0, owner_word);
    write_word(actual_iterator_output, 4, node_word);
    return actual_iterator_output;
}

void* query_native_file_store_date_00be5c80(void* actual_file_store,
    void* actual_date_output, const void* actual_name_header,
    const SingletonLifetimeCallbacks& invalid_parameters) {
    auto* const captured_head = read_pointer(actual_file_store, 0x18);
    auto* const actual_tree = at_offset(actual_file_store, 0x14);
    write_date_descending(actual_date_output, 0);

    NativeFileStoreNameIterator local_iterator;
    auto* const returned = find_native_file_store_name_00be5a50(
        actual_tree, &local_iterator, actual_name_header, invalid_parameters);
    auto* const returned_owner = read_pointer(returned, 0);
    if (!returned_owner || returned_owner != actual_tree) {
        invalid_parameters.invalid_parameter(invalid_parameters.context);
    }
    if (read_pointer(returned, 4) != captured_head) {
        write_date_descending(actual_date_output, 0xffffffffu);
    }
    return actual_date_output;
}

void* query_native_mpkg_file_date_00bb9d50(
    void* actual_date_output, const void*) noexcept {
    write_date_descending(actual_date_output, 0);
    return actual_date_output;
}

void* query_native_msar_file_date_00bbb640(
    void* actual_date_output, const void*) noexcept {
    write_date_descending(actual_date_output, 0);
    return actual_date_output;
}

} // namespace bsp
