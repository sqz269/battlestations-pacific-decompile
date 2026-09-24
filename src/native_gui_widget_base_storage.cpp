#include "bsp/native_gui_widget_base_storage.hpp"
#include "bsp/native_platform_focus_owners.hpp"
#include "bsp/native_ref_counted.hpp"
#include <initializer_list>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI widget storage reconstruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
volatile Word& word(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile Word*>(
        static_cast<unsigned char*>(storage) + offset);
}
volatile unsigned char& byte(void* storage, std::size_t offset) noexcept {
    return *(static_cast<volatile unsigned char*>(storage) + offset);
}
} // namespace

void* allocate_native_gui_widget_list_sentinel_00a9b720() {
    // All 26 bytes match A4C4A0 except the relative operand of the same
    // operator_new(BF681B) CALL. Reuse its allocator and pointer-test behavior.
    return allocate_native_media_list_sentinel_00a4c4a0();
}

__declspec(naked) void __fastcall destroy_native_gui_ref_base_00aa6e10(void*) noexcept {
    __asm {
        mov dword ptr [ecx], 00d5c104h
        jmp destroy_native_ref_counted_base_00bd30f0
    }
}

NativeGuiTextIdentityPrefix* construct_native_gui_widget_base_00aa9390(
    void* raw, Word type, const volatile Word& actual_one_00d7a24c) {
    auto* const prefix = ::new (raw) NativeGuiTextIdentityPrefix{0x00ceb130, 1};
    const Word first_one = actual_one_00d7a24c; // AA93BF, before D5C130.
    prefix->native_vtable_00 = 0x00d5c130;
    for (const auto offset : {0x0cu, 0x10u, 0x14u, 0x18u, 0x1cu, 0x20u, 0x24u})
        word(raw, offset) = 0;
    word(raw, 0x28) = first_one;
    word(raw, 0x2c) = first_one;
    for (const auto offset : {0x30u, 0x34u, 0x38u, 0x3cu, 0x40u, 0x44u, 0x48u, 0x4cu})
        word(raw, offset) = 0;
    for (const auto offset : {0x50u, 0x54u, 0x58u, 0x5cu})
        word(raw, offset) = first_one;
    word(raw, 0x60) = type;
    void* sentinel;
    try {
        sentinel = allocate_native_gui_widget_list_sentinel_00a9b720();
    } catch (...) {
        // DEDC28 -> state 0 at DEDC20 -> CB73B0 -> AA6E10. This preserves
        // source C++ allocation-failure cleanup, not native FH3/SEH identity.
        destroy_native_gui_ref_base_00aa6e10(raw);
        throw;
    }
    const Word second_one = actual_one_00d7a24c; // AA944E, before +68.
    word(raw, 0x68) = reinterpret_cast<Word>(sentinel);
    word(raw, 0x6c) = 0;
    word(raw, 0x70) = 0;
    for (const auto offset : {0x74u, 0x75u, 0x76u, 0x77u, 0x78u, 0x79u, 0x84u})
        byte(raw, offset) = 0;
    for (const auto offset : {0x7cu, 0x80u, 0x88u, 0x8cu, 0x90u, 0x94u, 0xa4u, 0xa8u, 0xacu})
        word(raw, offset) = 0;
    for (const auto offset : {0xb0u, 0xb4u, 0xb8u, 0xbcu, 0xc0u})
        word(raw, offset) = second_one;
    byte(raw, 0xd4) = 0;
    word(raw, 0xd8) = 0;
    word(raw, 0xdc) = 0;
    word(raw, 0xe0) = 0;
    word(raw, 0xc4) = 0; // Deliberately the last field write, AA94FB.
    return prefix;
}
} // namespace bsp
