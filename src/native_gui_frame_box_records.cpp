#include "bsp/native_gui_frame_box_records.hpp"
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI FrameBox record copy requires MSVC Win32.
#endif
static_assert(sizeof(void*) == 4);

namespace bsp {
namespace {
using Word = std::uint32_t;
volatile Word& word(void* raw, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile Word*>(static_cast<std::byte*>(raw) + offset);
}
Word word(const void* raw, std::size_t offset) noexcept {
    return *reinterpret_cast<const volatile Word*>(
        static_cast<const std::byte*>(raw) + offset);
}
} // namespace

void* copy_construct_native_gui_frame_box_record_00ad0c60(
    void* destination, const void* source, NativeStringRawPoolContext& strings) {
    const Word resource = word(source, 0);
    auto* const destination_name = static_cast<std::byte*>(destination) + 4;
    const auto* const source_name = static_cast<const std::byte*>(source) + 4;
    const bool same_header = destination_name == source_name;
    word(destination, 0) = resource;
    word(destination, 4) = 0;
    word(destination, 8) = 0;
    if (!same_header) {
        const Word requested = word(source, 4);
        resize_native_string_header_0041dd40(destination_name, strings, requested, true);
        if (word(source, 4) != 0) {
            const Word count = word(destination, 4);
            const auto* const source_data = reinterpret_cast<const void*>(
                static_cast<std::uintptr_t>(word(source, 8)));
            auto* const destination_data = reinterpret_cast<void*>(
                static_cast<std::uintptr_t>(word(destination, 8)));
            // Existing Source byte-copy boundary: overlap is admitted and a
            // zero count does not invoke a standard copy with null pointers.
            if (count != 0) std::memmove(destination_data, source_data, count);
        }
    }
    __asm {
        mov edi, source
        mov esi, destination
        fld dword ptr [edi+0ch]
        fstp dword ptr [esi+0ch]
        fld dword ptr [edi+10h]
        fstp dword ptr [esi+10h]
        fld dword ptr [edi+14h]
        fstp dword ptr [esi+14h]
        fld dword ptr [edi+18h]
        fstp dword ptr [esi+18h]
    }
    word(destination, 0x1c) = word(source, 0x1c);
    word(destination, 0x20) = word(source, 0x20);
    word(destination, 0x24) = word(source, 0x24);
    word(destination, 0x28) = word(source, 0x28);
    auto* const current_resource = reinterpret_cast<std::byte*>(
        static_cast<std::uintptr_t>(word(destination, 0)));
    if (current_resource != nullptr)
        ::InterlockedIncrement(reinterpret_cast<volatile LONG*>(current_resource + 4));
    return destination;
}
} // namespace bsp
