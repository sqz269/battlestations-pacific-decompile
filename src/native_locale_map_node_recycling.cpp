#include "bsp/native_locale_map_node_recycling.hpp"
#include "bsp/native_locale_map_node_lifetime.hpp"

#include <cstring>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native locale node recycling requires MSVC Win32.
#endif

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 0x18);

// Raw operations preserve native-width accesses, wrapping address arithmetic
// and current RMWs without a typed shadow owner or C++ aliasing assumptions.
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

void put_word(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}

void add_word(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        add dword ptr [eax + edx], ecx
    }
}

std::uint16_t half(const void* base, std::uint32_t byte_offset) noexcept {
    std::uint16_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ax, word ptr [eax + edx]
        mov result, ax
    }
    return result;
}

void put_half(void* base, std::uint32_t byte_offset, std::uint16_t value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov cx, value
        mov word ptr [eax + edx], cx
    }
}

void increment_half(void* base, std::uint32_t byte_offset) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        add word ptr [eax + edx], 1
    }
}

std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}

void* pointer(std::uint32_t bits) noexcept {
    return reinterpret_cast<void*>(bits);
}

} // namespace

void return_native_locale_node_slot_00736090(void* pool, void* slot) {
    auto* const section = reinterpret_cast<CRITICAL_SECTION*>(
        pointer(address(pool) + 0x0cu));
    EnterCriticalSection(section);
    add_word(pool, 0x24, 1u);

    const auto block_index = word(slot, 0x18);
    auto* const table = pointer(word(pool, 0x28));
    auto* const block = pointer(word(table, block_index * 4u));
    const auto offset_bits = address(slot) - address(block);
    std::int32_t signed_offset;
    std::memcpy(&signed_offset, &offset_bits, sizeof signed_offset);
    // Native signed IMUL/SAR/SHR sequence truncates division by 28 toward zero.
    const auto slot_index = static_cast<std::uint16_t>(signed_offset / 28);
    const auto free_count = half(block, 0x7800);
    put_half(block, 0x7000u + static_cast<std::uint32_t>(free_count) * 2u,
        slot_index);
    increment_half(block, 0x7800);
    if (block_index < word(pool, 0x34))
        put_word(pool, 0x34, block_index);

    add_word(pool, 0x24, 0xffffffffu);
    LeaveCriticalSection(section);
}

void* scalar_delete_native_locale_map_node_0073c220(void* node,
    std::uint32_t flags, NativeStringRawPoolContext& strings, void* pool) {
    destroy_native_locale_map_node_payload_00737ce0(node, strings);
    // Force the low-byte read of this normalized parameter after payload return.
    if ((*reinterpret_cast<const volatile unsigned char*>(&flags) & 1u) != 0)
        return_native_locale_node_slot_00736090(pool, node);
    return node;
}

} // namespace bsp
