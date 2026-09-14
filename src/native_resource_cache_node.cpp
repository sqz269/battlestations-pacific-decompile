#include "bsp/native_resource_cache_node.hpp"

#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Actual resource-cache storage is Win32.");
const void* at(const void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(storage) + offset);
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
} // namespace

void* construct_native_resource_cache_node_00b7f220(void* node, void* left,
    void* parent, void* right, const void* source, std::uint32_t color_word,
    NativeStringRawPoolContext& strings) {
    void* const key = at(node, 0x0c); // B7F235
    const bool identical = key == source; // B7F238, before all node writes.
    put_word(node, 0, reinterpret_cast<std::uintptr_t>(left));
    put_word(node, 4, reinterpret_cast<std::uintptr_t>(parent));
    put_word(node, 8, reinterpret_cast<std::uintptr_t>(right));
    put_word(key, 0, 0); // B7F242
    put_word(key, 4, 0); // B7F248
    if (!identical) {
        resize_native_string_header_0041dd40(key, strings, word(source), true);
        if (word(source) != 0) { // B7F25E, after resize/getter/pool calls.
            const auto count = word(key); // B7F264
            const auto* input = reinterpret_cast<const void*>(word(source, 4));
            auto* output = reinterpret_cast<void*>(word(key, 4));
            // BF7680 selects backwards copying for overlapping destinations;
            // omit only the existing raw string layer's zero-byte host call.
            if (count != 0) std::memmove(output, input, count);
        }
    }
    const auto resource = word(source, 8); // B7F277; source may alias node.
    put_word(node, 0x14, resource);
    put_byte(node, 0x18, static_cast<unsigned char>(color_word));
    put_byte(node, 0x19, 0);
    return node;
}

void* allocate_native_resource_cache_node_00b7f6a0(void* left, void* parent,
    void* right, const void* source, std::uint32_t color_word,
    NativeStringRawPoolContext& strings) {
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x1c, 0x1c}); // B7F6C3, state-1.
    try {
        if (allocation != nullptr) {
            construct_native_resource_cache_node_00b7f220(allocation,
                left, parent, right, source, color_word, strings);
        }
    } catch (...) {
        // State1 -> CC20D0 -> RET-only401130 adds no cleanup. CatchB7F712
        // frees captured[EBP-14] then BF6885(0,0) rethrows; no key destruction.
        singleton_lifetime_free(allocation);
        throw;
    }
    return allocation; // Captured ESI, not constructor EAX.
}
} // namespace bsp
