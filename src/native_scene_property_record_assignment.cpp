#include "bsp/native_scene_property_record_assignment.hpp"
#include "bsp/native_reference_payload_copy.hpp"
#include "bsp/native_scene_property_assignment_storage.hpp"
#include "bsp/native_string_duplicate.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "The original property records are Win32.");

__forceinline std::uint32_t load_word(const void* root, std::size_t byte_offset) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}

__forceinline void store_word(void* root, std::size_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}

__forceinline void copy_byte(void* destination, const void* source) noexcept {
    __asm {
        mov eax, source
        mov cl, byte ptr [eax + 0ch]
        mov edx, destination
        mov byte ptr [edx + 0ch], cl
    }
}

// Exact private seven-byte/three-instruction direct-m32 operation:
// D9420C D9590C C3. The ordinary caller supplies ECX=destination,EDX=source;
// no Original parent register/return ABI is implied by this private helper.
static __declspec(naked) void __fastcall assign_type1_x87_m32_008f072a(
    void*, const void*) noexcept {
    __asm {
        fld dword ptr [edx + 0ch]
        fstp dword ptr [ecx + 0ch]
        ret
    }
}
} // namespace

__declspec(noinline) void* assign_native_scene_property_record_008f0700(
    void* actual_destination, const void* actual_source) {
    const auto tag = load_word(actual_destination, 4);
    if (tag > 11) return actual_destination;
    switch (tag) {
    case 0:
    case 4: {
        const auto value = load_word(actual_source, 0x0c);
        store_word(actual_destination, 0x0c, value);
        break;
    }
    case 1:
        assign_type1_x87_m32_008f072a(actual_destination, actual_source);
        break;
    case 2: {
        auto* const destination_slot = static_cast<std::byte*>(actual_destination) + 0x0c;
        free_and_clear_native_owned_pointer_004357f0(destination_slot);
        const auto source_pointer = load_word(actual_source, 0x0c);
        const auto* text = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(source_pointer));
        char* const copy = duplicate_native_string_00438e40(text);
        store_word(actual_destination, 0x0c, static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(copy)));
        break;
    }
    case 3:
        copy_byte(actual_destination, actual_source);
        break;
    case 5:
        copy_native_reference_payload_008f0340(
            static_cast<std::byte*>(actual_destination) + 0x18, nullptr,
            static_cast<const std::byte*>(actual_source) + 0x18);
        break;
    case 7: {
        const auto x = load_word(actual_source, 0x0c);
        store_word(actual_destination, 0x0c, x);
        const auto y = load_word(actual_source, 0x10);
        store_word(actual_destination, 0x10, y);
        const auto z = load_word(actual_source, 0x14);
        store_word(actual_destination, 0x14, z);
        break;
    }
    case 8:
    case 9:
    case 10:
    case 11:
        copy_native_scene_property_array_block_008f03b0(
            static_cast<std::byte*>(actual_destination) + 0x20,
            static_cast<const std::byte*>(actual_source) + 0x20);
        break;
    default:
        break;
    }
    return actual_destination;
}

} // namespace bsp
