#include "bsp/cc16_dyn_sap_endpoints.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error The recovered float-key transport requires MSVC Win32 x87.
#endif

namespace bsp::cc16_dyn_sap_endpoints {
namespace {
using U32 = std::uint32_t;
using U16 = std::uint16_t;
static_assert(sizeof(void*) == 4, "The recovered addresses require Win32.");

U32 address(const void* value) noexcept { return reinterpret_cast<U32>(value); }
volatile U32& word(U32 location) noexcept {
    return *reinterpret_cast<volatile U32*>(location);
}
volatile U16& half(U32 location) noexcept {
    return *reinterpret_cast<volatile U16*>(location);
}

void adjust_tagged_index(U32 tag, U32 axis, U32 delta) noexcept {
    const U32 location = (tag & 0xfffffffeu) + axis * 2u
        + ((tag & 1u) != 0u ? 0x20u : 0x26u);
    half(location) = static_cast<U16>(static_cast<U32>(half(location)) + delta);
}

// 00C40879..00C408B8 / 00C408BB..00C408F5. Two initial base reads
// precede the count read; each copied entry then reloads count and base.
void erase_entry(U32 header, U32 index) noexcept {
    const U32 first_base = word(header + 4u);
    const U32 second_base = word(header + 4u);
    U32 output = first_base + index * 8u;
    const U32 count = word(header + 8u);
    if (output != second_base + count * 8u - 8u) {
        U32 input = output + 8u;
        for (;;) {
            word(output) = word(input);
            word(output + 4u) = word(input + 4u);
            const U32 next_count = word(header + 8u);
            const U32 next_base = word(header + 4u);
            output += 8u;
            input += 8u;
            if (output == next_base + next_count * 8u - 8u) {
                break;
            }
        }
    }
    word(header + 8u) = word(header + 8u) - 1u;
}

// 00C40B38..00C40B8D. Do not replace the float loads/stores with integer
// copies: actual x87 controls govern signaling NaNs, denormals and faults.
// The lower FLD precedes the first base/index reads. The upper index read
// precedes its FLD, followed by a fresh base read. No FP controls change.
void publish_keys(void* object, void* axis_arrays, U32 axis) noexcept {
    __asm {
        mov esi, object
        mov eax, axis
        mov edx, axis_arrays
        fld dword ptr [esi + eax * 4 + 04h]
        lea ecx, [eax + eax * 2]
        mov edi, dword ptr [edx + ecx * 4 + 04h]
        lea ecx, [edx + ecx * 4 + 04h]
        movzx edx, word ptr [esi + eax * 2 + 20h]
        fstp dword ptr [edi + edx * 8]
        lea edx, [edi + edx * 8]
        mov edi, dword ptr [edx]
        mov ebx, edi
        shr ebx, 1fh
        not ebx
        add ebx, 1
        or ebx, 80000000h
        xor ebx, edi
        mov dword ptr [edx], ebx
        movzx edx, word ptr [esi + eax * 2 + 26h]
        fld dword ptr [esi + eax * 4 + 10h]
        mov edi, dword ptr [ecx]
        lea edx, [edi + edx * 8]
        fstp dword ptr [edx]
        mov edi, dword ptr [edx]
        mov ebx, edi
        shr ebx, 1fh
        not ebx
        add ebx, 1
        or ebx, 80000000h
        xor ebx, edi
        mov dword ptr [edx], ebx
    }
}

// The two stable leftward passes retain their distinct lower/upper index
// fields. Moved tags update their own selected WORD field; the captured entry
// publishes its index, tag, then key, with a fresh base read for each DWORD.
void move_left(U32 object, U32 base_cell, U32 axis, U32 field) noexcept {
    const U32 original = half(object + axis * 2u + field);
    const U32 base = word(base_cell);
    const U32 entry = base + original * 8u;
    const U32 tag = word(entry + 4u);
    const U32 key = word(entry);
    U32 cursor = original;
    if (cursor == 0u) {
        return;
    }
    do {
        const U32 current_base = word(base_cell);
        if (word(current_base + cursor * 8u - 8u) <= key) {
            break;
        }
        const U32 output = current_base + cursor * 8u;
        word(output) = word(output - 8u);
        word(output + 4u) = word(output - 4u);
        const U32 moved_base = word(base_cell);
        const U32 moved_tag = word(moved_base + cursor * 8u + 4u);
        adjust_tagged_index(moved_tag, axis, 1u);
        --cursor;
    } while (cursor != 0u);
    if (cursor != original) {
        half((tag & 0xfffffffeu) + axis * 2u + field) = static_cast<U16>(cursor);
        const U32 tag_base = word(base_cell);
        word(tag_base + cursor * 8u + 4u) = tag;
        const U32 key_base = word(base_cell);
        word(key_base + cursor * 8u) = key;
    }
}
}  // namespace

void remove_endpoints_00c40800(void* object, void* axis_arrays, U32 axis) noexcept {
    const U32 owner = address(object);
    const U32 lower = half(owner + axis * 2u + 0x20u);
    const U32 upper = half(owner + axis * 2u + 0x26u);
    const U32 header = address(axis_arrays) + axis * 12u;
    const U32 base = word(header + 4u);
    U32 cursor = base + lower * 8u + 8u;
    const U32 upper_entry = base + upper * 8u;
    while (cursor != upper_entry) {
        const U32 tag = word(cursor + 4u);
        adjust_tagged_index(tag, axis, 0xffffffffu);
        cursor += 8u;
    }
    const U32 count = word(header + 8u);
    const U32 current_base = word(header + 4u);
    const U32 end = current_base + count * 8u;
    cursor += 8u;
    while (cursor != end) {
        const U32 tag = word(cursor + 4u);
        adjust_tagged_index(tag, axis, 0xfffffffeu);
        cursor += 8u;
    }
    const U32 upper_now = half(owner + axis * 2u + 0x26u);
    erase_entry(header, upper_now);
    const U32 lower_now = half(owner + axis * 2u + 0x20u);
    erase_entry(header, lower_now);
}

void update_keys_move_left_00c40b30(void* object, void* axis_arrays, U32 axis) noexcept {
    publish_keys(object, axis_arrays, axis);
    const U32 owner = address(object);
    const U32 base_cell = address(axis_arrays) + axis * 12u + 4u;
    move_left(owner, base_cell, axis, 0x20u);
    move_left(owner, base_cell, axis, 0x26u);
}

}  // namespace bsp::cc16_dyn_sap_endpoints
