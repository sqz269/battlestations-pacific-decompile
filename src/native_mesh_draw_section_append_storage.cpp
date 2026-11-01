#include "bsp/native_mesh_draw_section_append_storage.hpp"

#include <cstdint>
#include <cstring>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Signed = std::int32_t;
static_assert(sizeof(Word) == 4);
static_assert(sizeof(Signed) == 4);
static_assert(sizeof(std::uintptr_t) == 4);
static_assert(sizeof(long) == 4);
static_assert(sizeof(NativeTextureSurfaceReferenceIncrement) == 4);

// Raw DWORD instructions form no C++ object/header overlay and leave field
// reads/writes visible across aliases and external calls. The ordinary Source
// interfaces below supply their own ABI; these accessors grant no Native ABI.
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
__declspec(naked) void __fastcall increment_current_word(Word) {
    __asm {
        add dword ptr [ecx], 1
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

void reserve_native_mesh_section_pointers_storage_00b72e10(
    void* actual_header,
    Signed requested,
    const NativeApplicationPointerVectorAllocation& allocation) {
    const Word header = address_of(actual_header);
    if (requested < 4) requested = 4;                       // B72E16..B72E1D
    if (signed_bits(load_word(header + 8u)) >= requested)   // B72E22..B72E25
        return;

    const Word capacity = static_cast<Word>(requested);
    const Word bytes = capacity * 4u;                      // B72E27: low32
    void* const replacement = allocation.allocate_00bf55be(bytes); // B72E30
    const Word captured_replacement = address_of(replacement);    // B72E35
    Word index = 0;
    Word cursor = captured_replacement;
    if (signed_bits(load_word(header + 4u)) > 0) {          // B72E3C..B72E3F
        do {
            if (cursor != 0) {                            // B72E43..B72E45
                const Word data = load_word(header);      // B72E47
                const Word value = load_word(data + index * 4u); // B72E49
                store_word(cursor, value);                // B72E4C
            }
            ++index;                                      // B72E4E: low32
            cursor += 4u;                                 // B72E51: low32
        } while (signed_bits(index) < signed_bits(load_word(header + 4u)));
                                                           // B72E54..B72E57
    }
    const Word old_data = load_word(header);               // B72E59
    allocation.free_00bf6989(
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(old_data))); // B72E5C
    store_word(header, captured_replacement);              // B72E64
    store_word(header + 8u, capacity);                     // B72E66
}

long append_native_mesh_draw_section_storage_00b73c60(
    void* actual_mesh,
    NativeMeshDrawSectionAppendArgumentWordView argument,
    const NativeMeshDrawSectionAppendStorageContext& context) {
    const Word mesh = address_of(actual_mesh);
    const Word header = mesh + 0x54u;
    const Word argument_address = address_of(&argument.word_04);
    const Word capacity = load_word(mesh + 0x5cu);         // B73C60
    if (load_word(mesh + 0x58u) == capacity) {              // B73C63
        Word requested = capacity + capacity;             // B73C6C: low32
        if (signed_bits(requested) <= 4) requested = 4;    // B73C6E..B73C73
        reserve_native_mesh_section_pointers_storage_00b72e10(
            reinterpret_cast<void*>(static_cast<std::uintptr_t>(header)),
            signed_bits(requested), context.allocation);  // B73C7B
    }
    const Word data = load_word(header);                  // B73C80
    const Word count = load_word(header + 4u);             // B73C82
    const Word slot = data + count * 4u;                   // B73C85: low32
    const bool store_slot = slot != 0;                    // B73C88
    const Word section = load_word(argument_address);     // B73C8A, even slot0
    if (store_slot) store_word(slot, section);             // B73C8E..B73C90
    increment_current_word(header + 4u);                  // B73C92, after alias
    const Word reference_address = section + 4u;          // B73C96: low32
    store_word(argument_address, reference_address);      // B73C9A
    const NativeTextureSurfaceReferenceIncrement increment =
        context.increment_00ce221c;                       // B73C9E: CURRENT
    return increment(reinterpret_cast<volatile long*>(
        static_cast<std::uintptr_t>(reference_address)));
}

} // namespace bsp
