#include "bsp/native_string_byte_append.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native string byte helpers require MSVC Win32 headers.
#endif

namespace bsp {
namespace {
static_assert(sizeof(NativeString) == 8);
static_assert(sizeof(std::uintptr_t) == 4);

template<class T> T read_header(const void* header, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const char*>(header) + offset, sizeof(value));
    return value;
}
template<class T> void write_header(void* header, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<char*>(header) + offset, &value, sizeof(value));
}

struct TemporaryReturn {
    NativeStringStorage& storage;
    char* const data;
    const std::uint32_t length;
    ~TemporaryReturn() {
        if (data != nullptr) storage.release(data, length + 1u);
    }
};
} // namespace

void* construct_native_string_byte_00531030(void* output,
    std::uint32_t byte_word, NativeStringStorage& storage) {
    write_header<std::uint32_t>(output, 0, 0); // 00531034
    write_header<char*>(output, 4, nullptr); // 0053103A
    char* const block = storage.allocate(2); // 00531041/0053104C
    const auto current_length = read_header<std::uint32_t>(output, 0);
    char* const current_data = read_header<char*>(output, 4);
    // The unsigned <1 branch calls BF7680 with count zero. Omit that call,
    // whose potentially null source is outside the standard C++ copy domain.
    if (current_length >= 1u) block[0] = current_data[0]; // 0053106A..6F
    char* const old_data = read_header<char*>(output, 4); // reload after copy
    if (old_data != nullptr) {
        const auto old_size = read_header<std::uint32_t>(output, 0) + 1u;
        storage.release(old_data, old_size); // 00531081/00531088
    }
    write_header<char*>(output, 4, block); // 00531091
    write_header<std::uint32_t>(output, 0, 1); // 00531094
    block[1] = '\0'; // 0053109A: may alias the output header
    *read_header<unsigned char*>(output, 4) =
        static_cast<unsigned char>(byte_word); // 0053109E/005310A1
    return output;
}

void append_native_string_byte_0054aa70(void* destination,
    std::uint32_t byte_word, NativeStringStorage& storage) {
    alignas(4) unsigned char temporary[8];
    construct_native_string_byte_00531030(temporary, byte_word, storage);
    const auto length = read_header<std::uint32_t>(temporary, 0); // 0054AA9B
    char* const data = read_header<char*>(temporary, 4); // 0054AAA1
    TemporaryReturn cleanup{storage, data, length}; // armed after construction
    if (length == 0) return;
    const auto old_length = read_header<std::uint32_t>(destination, 0);
    resize_native_string_header_0041dd40(destination, storage,
        old_length + length, true);
    char* const current_data = read_header<char*>(destination, 4); // 0054AABF
    auto* const append_at = reinterpret_cast<void*>(
        reinterpret_cast<std::uintptr_t>(current_data) + old_length);
    // BF7694..769A selects backward copying when src<dst<src+count;
    // BF785F..7862 uses STD/REP MOVSD/CLD. Nonwrapping valid copies therefore
    // have memmove behavior; neither cached temporary field is reread here.
    std::memmove(append_at, data, length); // 0054AAC7
}
} // namespace bsp
