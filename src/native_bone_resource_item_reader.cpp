#include "bsp/native_bone_resource_item_reader.hpp"
#include "bsp/native_string.hpp"

#include <cstring>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Bone resource item reader requires MSVC Win32 x87 assembly.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;

void* at(void* base, Word offset) noexcept {
    return reinterpret_cast<void*>(static_cast<Word>(
        reinterpret_cast<std::uintptr_t>(base)) + offset);
}

Word word(void* base, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(base, offset));
}
} // namespace

void read_native_bone_resource_item_00b8af30(void* actual_item,
    void* actual_handle, NativeResourceStreamReadContext& context) {
    // BEA010 owns construction. The native state remains -1 while it runs.
    Word temporary[2];
    void* const source = read_native_resource_handle_string_00bea010(
        actual_handle, temporary, context);
    void* const destination = at(actual_item, 0x08);
    // Native state0 covers only the completed temporary's assignment window.
    try {
        if (destination != source) {
            resize_native_string_header_0041dd40(destination, context.strings,
                word(source), true);
            if (word(source) != 0) {
                const Word count = word(destination);
                const Word source_data = word(source, 4);
                const Word destination_data = word(destination, 4);
                std::memcpy(reinterpret_cast<void*>(destination_data),
                    reinterpret_cast<const void*>(source_data), count);
            }
        }
    } catch (...) {
        // CC2940: destroy only the completed temporary at native EBP-2Ch.
        try {
            destroy_native_string_header_0041dd20(temporary, context.strings);
        } catch (...) {
            std::terminate();
        }
        throw;
    }
    // State -1 precedes the pool getter/return. A failure here must not retry
    // temporary cleanup or destroy the copied item string.
    destroy_native_string_header_0041dd20(temporary, context.strings);

    // Preserve seven FSTP32 spills, followed by seven FLD32/FSTP32 commits.
    // In particular, an incomplete seventh read writes no item float fields.
    // Integer storage keeps the compiler out of the x87 conversion schedule.
    Word scalar[7];
    __asm {
        mov ebx, actual_item
        mov esi, actual_handle
        mov edi, context
        push edi
        push esi
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp dword ptr scalar[0]
        push edi
        push esi
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp dword ptr scalar[4]
        push edi
        push esi
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp dword ptr scalar[8]
        push edi
        push esi
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp dword ptr scalar[12]
        push edi
        push esi
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp dword ptr scalar[16]
        push edi
        push esi
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp dword ptr scalar[20]
        push edi
        push esi
        call read_native_resource_node_float_00be99d0
        add esp, 8
        fstp dword ptr scalar[24]
        fld dword ptr scalar[0]
        fstp dword ptr [ebx+10h]
        fld dword ptr scalar[4]
        fstp dword ptr [ebx+14h]
        fld dword ptr scalar[8]
        fstp dword ptr [ebx+18h]
        fld dword ptr scalar[12]
        fstp dword ptr [ebx+1ch]
        fld dword ptr scalar[16]
        fstp dword ptr [ebx+20h]
        fld dword ptr scalar[20]
        fstp dword ptr [ebx+24h]
        fld dword ptr scalar[24]
        fstp dword ptr [ebx+28h]
    }
}

void NativeBoneResourceItemReaderCalls::read_item(
    std::uintptr_t target, void* item, void* handle) {
    if (target == 0x00b8af30u) {
        read_native_bone_resource_item_00b8af30(item, handle, context_);
        return;
    }
    remaining_.read_item(target, item, handle);
}
} // namespace bsp
