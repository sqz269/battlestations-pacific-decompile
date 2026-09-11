#include "bsp/native_string.hpp"

#include <cstdlib>
#include <cstring>
#include <new>

namespace bsp {
namespace {

static_assert(sizeof(NativeString) == 8, "Native string is a length dword plus a pointer.");
static_assert(sizeof(std::uintptr_t) == 4);

template<class T> T read_header(const void* header, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const char*>(header) + offset, sizeof(value));
    return value;
}
template<class T> void write_header(void* header, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<char*>(header) + offset, &value, sizeof(value));
}

class CrtStringStorage final : public NativeStringStorage {
public:
    char* allocate(std::uint32_t size) override {
        void* block = std::malloc(size == 0 ? 1u : static_cast<std::size_t>(size));
        if (block == nullptr) throw std::bad_alloc();
        return static_cast<char*>(block);
    }
    void release(char* block, std::uint32_t) noexcept override { std::free(block); }
};

} // namespace

char* PooledStringStorage::allocate(std::uint32_t size) {
    return static_cast<char*>(pool_->allocate_00bd1120(size));
}

void PooledStringStorage::release(char* block, std::uint32_t size) noexcept {
    pool_->release_00bd1510(block, size);
}

NativeStringStorage& crt_string_storage() noexcept {
    static CrtStringStorage storage;
    return storage;
}

void resize_native_string_header_0041dd40(void* actual_header,
    NativeStringStorage& storage, std::uint32_t length, bool preserve) {
    const auto initial_length = read_header<std::uint32_t>(actual_header, 0);
    if (length == initial_length) return; // 0041dd4a does not read the pointer.

    if (length == 0) { // 0041dd52
        auto* const old_data = read_header<char*>(actual_header, 4);
        if (old_data != nullptr) storage.release(old_data, initial_length + 1u);
        write_header<char*>(actual_header, 4, nullptr);
        write_header<std::uint32_t>(actual_header, 0, 0);
        return;
    }

    // 0041dd90 allocates before any old-buffer release. Do not snapshot the
    // source header across this boundary: a callback may have changed it.
    char* block = storage.allocate(length + 1u);
    if (preserve) { // 0041dd95
        const auto current_length = read_header<std::uint32_t>(actual_header, 0);
        const auto copied = length > current_length ? current_length : length;
        // Preserve the existing host policy: omit native memcpy with count 0,
        // which can pass a null source and is not a defined standard C++ call.
        if (copied != 0)
            std::memcpy(block, read_header<char*>(actual_header, 4), copied);
    }
    auto* const old_data = read_header<char*>(actual_header, 4); // 0041ddb9
    if (old_data != nullptr)
        storage.release(old_data, read_header<std::uint32_t>(actual_header, 0) + 1u);
    write_header<char*>(actual_header, 4, block);
    write_header<std::uint32_t>(actual_header, 0, length);
    // Native Win32 address addition, including uint32 wrap. No length guard or
    // cleanup is added if the caller/storage contract cannot support the write.
    *reinterpret_cast<char*>(reinterpret_cast<std::uintptr_t>(block) + length) = '\0';
}

void destroy_native_string_header_0041dd20(void* actual_header,
    NativeStringStorage& storage) noexcept {
    auto* const data = read_header<char*>(actual_header, 4);
    if (data != nullptr)
        storage.release(data, read_header<std::uint32_t>(actual_header, 0) + 1u);
}

void lowercase_native_string_header_004bcc00(void* actual_header) noexcept {
    auto* const data = read_header<unsigned char*>(actual_header, 4);
    if (data == nullptr || read_header<std::uint32_t>(actual_header, 0) == 0)
        return;
    auto cursor = reinterpret_cast<std::uintptr_t>(data);
    std::uint32_t index = 0;
    do {
        auto* const current = reinterpret_cast<volatile unsigned char*>(cursor);
        auto byte = *current;
        if (byte >= 0x41 && byte <= 0x5a)
            byte = static_cast<unsigned char>(byte + 0x20);
        *current = byte;
        ++index;
        ++cursor;
    } while (index < read_header<std::uint32_t>(actual_header, 0));
}

void NativeString::resize_0041dd40(NativeStringStorage& storage, std::uint32_t length, bool preserve) {
    resize_native_string_header_0041dd40(this, storage, length, preserve);
}

NativeString& NativeString::assign_0041e870(NativeStringStorage& storage, const char* text) {
    // 0041e87a: both fields are cleared first, so any buffer already held is
    // abandoned. This mirrors the native, which only ever runs on fresh storage.
    length_ = 0;
    data_ = nullptr;

    const std::uint32_t length = static_cast<std::uint32_t>(std::strlen(text));
    resize_0041dd40(storage, length, true);
    if (data_ != nullptr) {
        // 0041e8b1 copies length + 1 bytes, rewriting the terminator resize
        // already placed. An empty text leaves data_ null and copies nothing.
        std::memcpy(data_, text, length_ + 1u);
    }
    return *this;
}

void NativeString::copy_from_00be0a30_fragment(NativeStringStorage& storage, const NativeString& source) {
    copy_native_string_header_00be0a30_fragment(this, storage, &source);
}
void copy_native_string_header_00be0a30_fragment(void* destination,
    NativeStringStorage& storage, const void* source) {
    if (destination == source) return; //00BE0A6C
    resize_native_string_header_0041dd40(destination, storage,
        read_header<std::uint32_t>(source, 0), true);
    if (read_header<std::uint32_t>(source, 0) != 0) {
        const auto length = read_header<std::uint32_t>(destination, 0);
        if (length) std::memcpy(read_header<void*>(destination, 4),
            read_header<const void*>(source, 4), length);
    }
}

void NativeString::release_to(NativeStringStorage& storage) noexcept {
    if (data_ == nullptr) {
        length_ = 0;
        return;
    }
    storage.release(data_, length_ + 1u);
    data_ = nullptr;
    length_ = 0;
}

char* duplicate_00438e40(const char* text) {
    if (text == nullptr) return nullptr; // 00438e47
    const std::size_t size = std::strlen(text) + 1u;
    void* copy = std::malloc(size);
    if (copy == nullptr) throw std::bad_alloc();
    std::memcpy(copy, text, size); // 00438e6b copies the terminator too.
    return static_cast<char*>(copy);
}

void release_duplicate_00438e40(char* copy) noexcept { std::free(copy); }

}
