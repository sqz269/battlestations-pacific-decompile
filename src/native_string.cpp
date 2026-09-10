#include "bsp/native_string.hpp"

#include <cstdlib>
#include <cstring>
#include <new>

namespace bsp {
namespace {

static_assert(sizeof(NativeString) == 8, "Native string is a length dword plus a pointer.");

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

void NativeString::resize_0041dd40(NativeStringStorage& storage, std::uint32_t length, bool preserve) {
    if (length == length_) return; // 0041dd4a, taken even when data_ is null.

    if (length == 0) { // 0041dd52
        if (data_ != nullptr) storage.release(data_, length_ + 1u);
        data_ = nullptr;
        length_ = 0;
        return;
    }

    // 0041dd90: the new buffer is length + 1 bytes and is taken before the old
    // one is given back, so the copy below can never read freed storage.
    char* block = storage.allocate(length + 1u);
    if (preserve) { // 0041dd95
        const std::uint32_t copied = length > length_ ? length_ : length;
        // The native calls memcpy unconditionally here and passes a null source
        // when the string was empty; the count is zero in that case.
        if (copied != 0) std::memcpy(block, data_, copied);
    }
    if (data_ != nullptr) storage.release(data_, length_ + 1u); // 0041ddb9
    data_ = block;
    length_ = length;
    block[length] = '\0'; // 0041ddda
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
    if (this == &source) return; // 00be0a6c compares the two addresses.
    resize_0041dd40(storage, source.length_, true);
    if (source.length_ != 0) std::memcpy(data_, source.data_, length_);
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
