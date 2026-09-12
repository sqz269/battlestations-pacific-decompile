#include "bsp/native_pooled_text_suffix.hpp"

#include <cstring>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativePooledTextStorage) == 4);

template<class T> T load(const void* object) noexcept {
    T value;
    std::memcpy(&value, object, sizeof(value));
    return value;
}
template<class T> void store(void* object, T value) noexcept {
    std::memcpy(object, &value, sizeof(value));
}
bool search_byte(char value) noexcept {
    const auto byte = static_cast<std::int8_t>(value);
    return byte >= 0x20 && byte < 0x7f;
}

} // namespace

std::uint32_t assign_native_pooled_text_suffix_00af4450(void* header,
    const char* text, NativeStringStorage& storage) {
    if (text == nullptr) return 0;
    std::uint32_t count = 0;
    while (text[count] == '\t' || search_byte(text[count])) ++count;
    if (count == 0) return 0;

    auto* const old = load<char*>(header);
    if (old != nullptr) release_native_pooled_text_bytes_00aee1e0(old, storage);
    auto* const block = storage.allocate(count + 1u);
    store(header, block); // Native AF44A7, before the captured source is copied.
    // BF9280 is the genuine CRT strncpy dependency: keep count and NUL padding.
    // Its provider contract excludes source/destination byte-range overlap.
#pragma warning(suppress: 4996) // Keep the original CRT operation, not strncpy_s.
    std::strncpy(block, text, count);
    load<char*>(header)[count] = '\0'; // AF44AE reloads after copying.
    return count;
}

void* get_native_pooled_text_suffix_00af44c0(const void* line,
    void* output, std::int32_t index, NativeStringStorage& storage) {
    NativePooledTextStorage temporary{nullptr};
    const auto* const text = load<const char*>(line); // Native captures ESI once.
    if (text == nullptr) {
        return copy_construct_native_pooled_text_00aee2e0(output, &temporary, storage);
    }
    const auto length = static_cast<std::int32_t>(std::strlen(text));
    std::int32_t cursor = 0;
    std::uint32_t token = search_byte(text[0]) ? 0u : 0xffffffffu;
    while (cursor < length && search_byte(text[cursor])) {
        if (token == static_cast<std::uint32_t>(index)) {
            try {
                assign_native_pooled_text_suffix_00af4450(&temporary, text + cursor, storage);
                copy_construct_native_pooled_text_00aee2e0(output, &temporary, storage);
            } catch (...) {
                destroy_native_pooled_text_00aee2a0(&temporary, storage);
                throw;
            }
            destroy_native_pooled_text_00aee2a0(&temporary, storage);
            return output;
        }
        if (text[cursor] == ' ') {
            ++token;
            while (cursor < length && text[cursor] == ' ') ++cursor;
        } else {
            ++cursor;
        }
    }
    return copy_construct_native_pooled_text_00aee2e0(output, &temporary, storage);
}

} // namespace bsp
