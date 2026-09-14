#include "bsp/native_pak_registry_block_helpers.hpp"

#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string.hpp"

#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Actual native headers contain Win32 pointers.");

const void* at(const void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(
        reinterpret_cast<std::uintptr_t>(storage) + offset);
}
void* at(void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(storage) + offset);
}
std::uint32_t word(const void* storage, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(storage, offset));
}
void put_word(void* storage, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(storage, offset)) = value;
}
void put_byte(void* storage, std::uint32_t offset, unsigned char value) noexcept {
    *static_cast<volatile unsigned char*>(at(storage, offset)) = value;
}
const char* data(const void* header) noexcept {
    return reinterpret_cast<const char*>(word(header, 4));
}
unsigned char signed_positive(std::uint32_t count) noexcept {
    return static_cast<unsigned char>(count != 0 && count < 0x80000000u);
}
} // namespace

void* construct_native_pak_block_name_00bb5670(void* output, const void* input,
    NativeStringStorage& strings) {
    alignas(4) unsigned char suffix[8];
    put_word(suffix, 0, 0); // BB568B; pointer is initialized only by 41E870.
    const char* const input_data = data(input); // BB5693
    std::uint32_t position = 0xffffffffu;
    if (input_data != nullptr) {
        const char* const match = std::strstr(input_data, ".mpak"); // BB56A3
        if (match != nullptr) {
            // BB56AF reloads current source data after strstr, then subtracts.
            position = reinterpret_cast<std::uintptr_t>(match) - word(input, 4);
        }
    }
    if (position == word(input) - 5u) { // BB56B7..BC, including length4/no match.
        // The inlined BB5719..46 copy is the exact 426060 header schedule.
        return copy_construct_native_string_header_00426060(output, input, strings);
    }

    construct_native_string_cstring_0041e870(suffix, ".mpak", strings); // BB56C9
    // DFDcbc state0 -> CC43C0 -> 41DD20 owns only this current suffix header.
    // A failure during its construction has no BB5670-owned cleanup state.
    try {
        concatenate_native_string_headers_004261a0(input, output, suffix, strings);
    } catch (...) {
        destroy_native_string_header_0041dd20(suffix, strings);
        throw;
    }
    // State is -1 before normal return of the temporary; do not retry release.
    destroy_native_string_header_0041dd20(suffix, strings);
    return output;
}

void decrement_native_vfs_block_count_00bd9200(void* manager) noexcept {
    put_word(manager, 0x1c, word(manager, 0x1c) - 1u);
    put_byte(manager, 0x20, signed_positive(word(manager, 0x1c)));
}

void set_native_vfs_block_active_00bd9210(void* manager,
    std::uint32_t byte_word) noexcept {
    put_byte(manager, 0x20, static_cast<unsigned char>(byte_word));
}

void recompute_native_vfs_block_active_00bd9220(void* manager) noexcept {
    put_byte(manager, 0x20, signed_positive(word(manager, 0x1c)));
}

void increment_native_vfs_block_count_00bd9fa0(void* manager) noexcept {
    put_word(manager, 0x1c, word(manager, 0x1c) + 1u);
    put_byte(manager, 0x20, 1);
}
} // namespace bsp
