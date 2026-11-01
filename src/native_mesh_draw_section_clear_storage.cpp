#include "bsp/native_mesh_draw_section_clear_storage.hpp"

#include <cstdint>
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Signed = std::int32_t;
using CurrentZero = void (__thiscall*)(void*);
static_assert(sizeof(Word) == 4);
static_assert(sizeof(Signed) == 4);
static_assert(sizeof(std::uintptr_t) == 4);
static_assert(sizeof(long) == 4);
static_assert(sizeof(CurrentZero) == 4);
static_assert(sizeof(NativeTextureSurfaceReferenceIncrement) == 4);

// Actual DWORD access/ADD instructions, without C++ object/header overlays.
// The surrounding ordinary Source functions have their own C++ interfaces.
__declspec(naked) Word __fastcall load_word(Word) {
    __asm {
        mov eax, dword ptr [ecx]
        ret
    }
}
__declspec(naked) void __fastcall store_word(Word, Word) {
    __asm {
        mov dword ptr [ecx], edx
        ret
    }
}
__declspec(naked) void __fastcall decrement_current_word(Word) {
    __asm {
        add dword ptr [ecx], -1
        ret
    }
}
Word address_of(const volatile void* storage) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(storage));
}
Signed signed_bits(Word bits) {
    Signed result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
} // namespace

void resize_native_mesh_section_pointers_storage_00b73840(
    void* actual_header,
    Signed desired,
    const NativeApplicationPointerVectorAllocation& allocation) {
    const Word header = address_of(actual_header);
    if (desired > signed_bits(load_word(header + 8u))) {     // B73848..B7384B
        reserve_native_mesh_section_pointers_storage_00b72e10(
            actual_header, desired, allocation);            // B7384E
    }
    Word index = load_word(header + 4u);                    // B73853, ONCE
    while (signed_bits(index) < desired) {                  // B73856 / B73872
        const Word data = load_word(header);               // B73860, CURRENT
        const Word slot = data + index * 4u;                // B73862, low32
        if (slot != 0) store_word(slot, 0);                  // B73865..B73869
        ++index;                                           // B7386F, low32
    }
    while (desired < signed_bits(load_word(header + 4u))) {  // B73876 / B73883
        decrement_current_word(header + 4u);               // B73880, actual ADD
    }
    store_word(header + 4u, static_cast<Word>(desired));     // B73888, original
}

void clear_native_mesh_draw_sections_storage_00b73c10(
    void* actual_mesh,
    const NativeMeshDrawSectionClearStorageContext& context) {
    const Word mesh = address_of(actual_mesh);
    Word index = 0;
    if (signed_bits(load_word(mesh + 0x58u)) > 0) {          // B73C16..B73C19
        const NativeTextureSurfaceReferenceIncrement decrement =
            context.decrement_00ce2220;                    // B73C1C, ONCE
        do {
            const Word data = load_word(mesh + 0x54u);      // B73C23, CURRENT
            const Word captured = load_word(data + index * 4u); // B73C26
            const Word reference = captured + 4u;          // B73C29, low32
            const long result = decrement(reinterpret_cast<volatile long*>(
                static_cast<std::uintptr_t>(reference)));  // B73C2D, SAVED
            if (result == 0) {                             // B73C2F..B73C31
                const Word table = load_word(captured);    // B73C33, CURRENT B
                const Word target = load_word(table);      // B73C35, CURRENT0
                CurrentZero operation;
                std::memcpy(&operation, &target, sizeof(operation));
                operation(reinterpret_cast<void*>(
                    static_cast<std::uintptr_t>(captured))); // B73C39, ECX B
            }
            ++index;                                      // B73C3B, low32
        } while (signed_bits(index) < signed_bits(load_word(mesh + 0x58u)));
                                                           // B73C3E..B73C41
    }
    resize_native_mesh_section_pointers_storage_00b73840(
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(mesh + 0x54u)),
        0, context.allocation);                            // B73C45..B73C4A
}

} // namespace bsp
