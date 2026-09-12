#include "bsp/native_filestore_subtree.hpp"
#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native FileStore subtree operations require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(LONG) == 4);
std::uint32_t word(const void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
bool nil(void* node) noexcept {
    return *reinterpret_cast<const volatile unsigned char*>(
        reinterpret_cast<std::uintptr_t>(node) + 0x19u) != 0;
}
struct PayloadStringCleanup {
    void* payload;
    NativeStringStorage& strings;
    bool armed = true;
    ~PayloadStringCleanup() noexcept {
        if (armed) destroy_native_string_header_0041dd20(payload, strings);
    }
};
void advance_iterator(void* iterator, const SingletonLifetimeCallbacks& invalid) {
    if (!word(iterator)) invalid.invalid_parameter(invalid.context);
    auto* node = pointer(word(iterator, 4));
    if (nil(node)) {
        invalid.invalid_parameter(invalid.context); // Native tail JMP; do not retry.
        return;
    }
    auto* right = pointer(word(node, 8));
    if (!nil(right)) {
        auto* left = pointer(word(right));
        while (!nil(left)) {
            right = left;
            left = pointer(word(right));
        }
        put(iterator, 4, reinterpret_cast<std::uintptr_t>(right));
        return;
    }
    auto* parent = pointer(word(node, 4));
    while (!nil(parent)) {
        if (word(iterator, 4) != word(parent, 8)) break;
        put(iterator, 4, reinterpret_cast<std::uintptr_t>(parent));
        parent = pointer(word(parent, 4));
    }
    put(iterator, 4, reinterpret_cast<std::uintptr_t>(parent));
}
} // namespace

void destroy_native_file_store_payload_00be5d70(void* payload,
    NativeStringStorage& strings, NativeAdoptedSubstreamDispatch& dispatch) {
    auto* const owner = pointer(word(payload, 8));
    PayloadStringCleanup cleanup{payload, strings};
    if (owner) {
        auto* const count = reinterpret_cast<volatile LONG*>(
            reinterpret_cast<std::uintptr_t>(owner) + 4u);
        if (InterlockedDecrement(count) == 0) {
            const auto table = word(owner);
            const auto entry = word(pointer(table));
            // BE5DB1: ECX captured owner, EAX current slot0, EDX captured table,
            // no stack arguments; entry's own native calling contract is kept.
            dispatch.source_zero_reference(entry, owner, table);
        }
        put(payload, 8, 0);
    }
    auto* const data = static_cast<char*>(pointer(word(payload, 4)));
    cleanup.armed = false; // Native disarms before normal string release.
    if (data) strings.release(data, word(payload) + 1u);
}

void erase_native_file_store_pending_subtree_00be66c0(void* tree,
    void* node, NativeStringStorage& strings) {
    while (!nil(node)) {
        erase_native_file_store_pending_subtree_00be66c0(tree,
            pointer(word(node, 8)), strings);
        auto* const data = static_cast<char*>(pointer(word(node, 0x10)));
        auto* const left = pointer(word(node));
        if (data) strings.release(data, word(node, 0x0c) + 1u);
        singleton_lifetime_free(node);
        node = left;
    }
}

void erase_native_file_store_resident_subtree_00be6720(void* tree,
    void* node, NativeStringStorage& strings, NativeAdoptedSubstreamDispatch& dispatch) {
    while (!nil(node)) {
        erase_native_file_store_resident_subtree_00be6720(tree,
            pointer(word(node, 8)), strings, dispatch);
        auto* const left = pointer(word(node));
        destroy_native_file_store_payload_00be5d70(pointer(
            reinterpret_cast<std::uintptr_t>(node) + 0x0cu), strings, dispatch);
        singleton_lifetime_free(node);
        node = left;
    }
}

void advance_native_file_store_resident_iterator_00be4c30(void* iterator,
    const SingletonLifetimeCallbacks& invalid) { advance_iterator(iterator, invalid); }
void advance_native_file_store_pending_iterator_00be4e40(void* iterator,
    const SingletonLifetimeCallbacks& invalid) { advance_iterator(iterator, invalid); }
} // namespace bsp
