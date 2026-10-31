#include "bsp/native_gui_icon_delayed_loads_lifetime_storage.hpp"
#include "bsp/native_gui_icon_delayed_loads.hpp"
#include "bsp/native_gui_named_value_array_resize_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw delayed-load lifetime storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeGuiIconDelayedLoadsStorage) == 16);
static_assert(std::is_trivial_v<NativeGuiIconDelayedLoadsDestroyStorageFrame>);

Word bits(void* pointer) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(pointer));
}
void* pointer_from_word(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
Word read_word(const void* base, Word byte_offset) {
    Word value;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
void write_word(void* base, Word byte_offset, Word value) {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
std::uint8_t read_byte(const volatile void* actual_home) {
    std::uint8_t value;
    __asm {
        mov eax, actual_home
        mov al, byte ptr [eax]
        mov value, al
    }
    return value;
}
} // namespace

void destroy_native_gui_icon_delayed_loads_storage_00ab5560(
    void* actual_owner16, NativeGuiIconDelayedLoadsDestroyStorageFrame& frame,
    NativeGuiIconDelayedLoadsDestroyStorageAcquired& acquired,
    NativeStringRawPoolContext& strings,
    NativeGuiIconDelayedLoadsStorage* volatile& publication_00f8bd98,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    write_word(&frame.native_state_home, 0u, 0xffffffffu); // Native PUSH -1.
    const Word owner = bits(actual_owner16);
    write_word(&frame.saved_owner_home, 0u, owner); // PUSH ECX at AB5575.
    acquired.captured_identity_bits_ = owner; // Captured ESI at AB5577.
    write_word(&frame.saved_owner_home, 0u, owner); // Repeated actual spill557A.
    const Word array_header = owner + 4u; // Captured EDI, not publication.
    write_word(&frame.native_state_home, 0u, 0u); // AB5585 before whole58.
    resize_native_gui_named_value_array_storage_00ab4850(pointer_from_word(array_header),
        0u, frame.reserve, strings, actual_feature_word_0109eea4); // ORIGINAL0.
    const Word current_data = read_word(pointer_from_word(array_header), 0u); // 5592.
    acquired.backing_free_entered_ = true; // Passive Source event, actual5595.
    singleton_lifetime_free(pointer_from_word(current_data));
    acquired.backing_free_returned_ = true;
    publication_00f8bd98 = nullptr; // Unconditional SAME cell, even replacement.
    write_word(pointer_from_word(owner), 0u, 0x00ce3818u); // Captured owner55AB.
}

void* scalar_delete_native_gui_icon_delayed_loads_storage_00ab57c0(
    void* captured_owner16, const volatile std::uint32_t& actual_flags_home,
    NativeGuiIconDelayedLoadsDestroyStorageFrame& frame,
    NativeGuiIconDelayedLoadsScalarStorageAcquired& acquired,
    NativeStringRawPoolContext& strings,
    NativeGuiIconDelayedLoadsStorage* volatile& publication_00f8bd98,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    const Word owner = bits(captured_owner16); // Captured ESI at AB57C1.
    acquired.captured_identity_bits_ = owner;
    destroy_native_gui_icon_delayed_loads_storage_00ab5560(pointer_from_word(owner),
        frame, acquired.destruction_, strings, publication_00f8bd98,
        actual_feature_word_0109eea4);
    const std::uint8_t tested = read_byte(&actual_flags_home); // ONE CURRENT57C8.
    acquired.tested_flags_byte_ = tested;
    acquired.flags_test_reached_ = true;
    if ((tested & 1u) != 0u) {
        acquired.owner_free_entered_ = true; // Passive Source event, actual57D0.
        singleton_lifetime_free(pointer_from_word(owner));
        acquired.owner_free_returned_ = true;
    }
    return pointer_from_word(owner); // Identity only, even after physical free.
}
} // namespace bsp
