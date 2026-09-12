#include "bsp/native_pooled_text.hpp"

#include <cstring>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativePooledTextStorage) == 4);

template<class T> T load(const void* object, std::size_t offset = 0) noexcept {
    T value;
    std::memcpy(&value, static_cast<const char*>(object) + offset, sizeof(value));
    return value;
}
template<class T> void store(void* object, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<char*>(object) + offset, &value, sizeof(value));
}
std::uint32_t size_with_nul(const char* text) noexcept {
    return static_cast<std::uint32_t>(std::strlen(text)) + 1u;
}
void copy_through_nul(char* destination, const char* source) noexcept {
    // Keep native forward-copy behavior and source reload at the caller. A
    // constructor whose destination aliases the source header observes its
    // just-published allocation, rather than the old source pointer.
    char byte;
    do {
        byte = *source++;
        *destination++ = byte;
    } while (byte != '\0');
}
bool token_byte(char value) noexcept {
    const auto byte = static_cast<std::int8_t>(value);
    return byte >= 0x20 && byte < 0x7f;
}
void copy_output_and_destroy(void* output, NativePooledTextStorage& temporary,
    NativeStringStorage& storage) {
    try {
        copy_construct_native_pooled_text_00aee2e0(output, &temporary, storage);
    } catch (...) {
        destroy_native_pooled_text_00aee2a0(&temporary, storage);
        throw;
    }
    destroy_native_pooled_text_00aee2a0(&temporary, storage);
}

} // namespace

void release_native_pooled_text_bytes_00aee1e0(char* text,
    NativeStringStorage& storage) noexcept {
    storage.release(text, size_with_nul(text));
}

void destroy_native_pooled_text_00aee2a0(void* header,
    NativeStringStorage& storage) noexcept {
    auto* const text = load<char*>(header);
    if (text != nullptr) release_native_pooled_text_bytes_00aee1e0(text, storage);
    store<char*>(header, 0, nullptr);
}

void* copy_construct_native_pooled_text_00aee2e0(void* destination,
    const void* source, NativeStringStorage& storage) {
    const auto* const initial = load<const char*>(source);
    if (initial == nullptr) {
        store<char*>(destination, 0, nullptr);
    } else {
        auto* const block = storage.allocate(size_with_nul(initial));
        store(destination, 0, block); // AEE311 precedes source reload AEE313.
        copy_through_nul(block, load<const char*>(source));
    }
    return destination;
}

void* construct_native_pooled_text_00af5660(void* destination,
    const char* text, NativeStringStorage& storage) {
    if (text == nullptr) {
        store<char*>(destination, 0, nullptr);
    } else {
        auto* const block = storage.allocate(size_with_nul(text));
        store(destination, 0, block);
        copy_through_nul(block, text); // Source bytes captured in native EDI.
    }
    return destination;
}

void* assign_native_pooled_text_00af56c0(void* destination,
    const void* source, NativeStringStorage& storage) {
    auto* const old = load<char*>(destination);
    if (old != nullptr) release_native_pooled_text_bytes_00aee1e0(old, storage);
    const auto* const text = load<const char*>(source); // AFTER old return.
    if (text != nullptr) {
        auto* const block = storage.allocate(size_with_nul(text));
        store(destination, 0, block);
        copy_through_nul(block, load<const char*>(source));
    }
    // AF56F6 jumps directly to epilogue: no null write for a null source.
    return destination;
}

std::uint32_t assign_native_pooled_text_prefix_00aee340(void* header,
    const char* text, NativeStringStorage& storage) {
    if (text == nullptr) return 0;
    std::uint32_t count = 0;
    while (static_cast<std::int8_t>(text[count]) > 0x20 &&
        static_cast<std::int8_t>(text[count]) < 0x7f) ++count;
    if (count == 0) return 0;
    auto* const old = load<char*>(header);
    if (old != nullptr) release_native_pooled_text_bytes_00aee1e0(old, storage);
    auto* const block = storage.allocate(count + 1u);
    store(header, 0, block);
    // Native strncpy(count), including its NUL padding if a storage callback
    // changed the bytes after the prefix scan. No second length calculation.
    std::uint32_t copied = 0;
    for (; copied < count && text[copied] != '\0'; ++copied)
        block[copied] = text[copied];
    for (; copied < count; ++copied) block[copied] = '\0';
    load<char*>(header)[count] = '\0'; // Native reloads destination at AEE3A3.
    return count;
}

void* get_native_pooled_text_token_00aee3c0(const void* line,
    void* output, std::int32_t index, NativeStringStorage& storage) {
    NativePooledTextStorage temporary{nullptr};
    const auto* const text = load<const char*>(line);
    if (text == nullptr) {
        return copy_construct_native_pooled_text_00aee2e0(output, &temporary, storage);
    }
    const auto length = static_cast<std::int32_t>(std::strlen(text));
    std::int32_t cursor = 0;
    std::uint32_t token = token_byte(text[0]) ? 0u : 0xffffffffu;
    while (cursor < length && token_byte(text[cursor])) {
        if (token == static_cast<std::uint32_t>(index)) {
            assign_native_pooled_text_prefix_00aee340(&temporary, text + cursor, storage);
            copy_output_and_destroy(output, temporary, storage);
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

bool equal_native_pooled_text_00aedf80(const void* header, const char* text) {
    return _stricmp(load<const char*>(header), text) == 0;
}

void* initialize_native_text_buffer_00af5600(void* buffer) noexcept {
    store<std::uint32_t>(buffer, 0x0c, 0);
    store<char*>(buffer, 0x10, nullptr);
    store<std::uint32_t>(buffer, 4, 0);
    store<std::uint32_t>(buffer, 0, 0);
    store<char*>(buffer, 0x14, nullptr);
    store<std::uint32_t>(buffer, 0x18, 0);
    return buffer;
}

void rewind_native_text_buffer_00af55f0(void* buffer) noexcept {
    store<std::uint32_t>(buffer, 4, 0);
    store<std::uint32_t>(buffer, 0x18, 0);
}

bool read_native_text_buffer_line_00af5740(void* buffer, void* output,
    NativeStringStorage& storage, char* scratch) {
    const auto* const initial_data = load<const char*>(buffer, 0x14);
    if (initial_data == nullptr) return false;
    auto start = load<std::uint32_t>(buffer, 4);
    if (static_cast<std::int32_t>(start) >= load<std::int32_t>(buffer, 8)) return false;
    do {
        const auto cursor = load<std::uint32_t>(buffer, 4);
        if (initial_data[cursor] == '\n') break;
        store(buffer, 4, cursor + 1u);
    } while (load<std::int32_t>(buffer, 4) < load<std::int32_t>(buffer, 8));
    const auto end = load<std::uint32_t>(buffer, 4);
    while (static_cast<std::int32_t>(start) < static_cast<std::int32_t>(end) &&
        static_cast<std::int8_t>(initial_data[start]) <= 0x20) ++start;
    std::uint32_t copied = 0;
    if (start < end) {
        do {
            const auto byte = load<const char*>(buffer, 0x14)[start];
            if (static_cast<std::int8_t>(byte) >= 0x20) scratch[copied++] = byte;
            ++start;
        } while (start < load<std::uint32_t>(buffer, 4));
    }
    scratch[copied] = '\0';
    store(buffer, 4, load<std::uint32_t>(buffer, 4) + 1u);
    NativePooledTextStorage temporary;
    construct_native_pooled_text_00af5660(&temporary, scratch, storage);
    try {
        assign_native_pooled_text_00af56c0(output, &temporary, storage);
    } catch (...) {
        destroy_native_pooled_text_00aee2a0(&temporary, storage);
        throw;
    }
    destroy_native_pooled_text_00aee2a0(&temporary, storage);
    return true;
}

} // namespace bsp
