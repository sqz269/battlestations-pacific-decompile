#include "bsp/native_device_registry_array.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native device registry arrays require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;

Word word(const void* base, Word byte_offset = 0) noexcept {
    Word value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}

void put(void* base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}

std::int32_t signed_word(const void* base, Word byte_offset) noexcept {
    const Word bits = word(base, byte_offset);
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
Word address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}
} // namespace

void reserve_native_device_registry_array_0043fa60(
    NativeDeviceRegistryArrayStorage& array, std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (signed_word(&array, 8) >= requested) return;

    const Word bytes = static_cast<Word>(requested) * 4u;
    void* const allocation = singleton_lifetime_allocate(
        {SingletonAllocationKind::pointer_slots, bytes, bytes});
    Word destination = address(allocation);
    Word index = 0;
    if (signed_word(&array, 4) > 0) {
        do {
            if (destination != 0) {
                const Word source = word(&array) + index * 4u;
                put(pointer(destination), 0, word(pointer(source)));
            }
            ++index;
            destination += 4u;
        } while (static_cast<std::int32_t>(index) < signed_word(&array, 4));
    }

    singleton_lifetime_free(pointer(word(&array)));
    put(&array, 0, address(allocation));
    put(&array, 8, static_cast<Word>(requested));
}

void resize_native_device_registry_array_00440180(
    NativeDeviceRegistryArrayStorage& array, std::int32_t requested) {
    if (requested > signed_word(&array, 8))
        reserve_native_device_registry_array_0043fa60(array, requested);

    Word index = word(&array, 4);
    if (static_cast<std::int32_t>(index) < requested) {
        do {
            const Word destination = word(&array) + index * 4u;
            if (destination != 0) put(pointer(destination), 0, 0);
            ++index;
        } while (static_cast<std::int32_t>(index) < requested);
    }

    while (requested < signed_word(&array, 4))
        put(&array, 4, word(&array, 4) - 1u);
    put(&array, 4, static_cast<Word>(requested));
}

} // namespace bsp
