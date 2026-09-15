#include "bsp/native_crt_sbh_allocation.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native SBH allocation requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeCrtSbhState) == 28);

__forceinline Word bits(const void* pointer) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(pointer));
}
__forceinline void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
__forceinline std::int32_t signed_word(Word value) {
    // The required MSVC Win32 target uses two's-complement conversion and SAR.
    return static_cast<std::int32_t>(value);
}

// Raw native records include unused, initially indeterminate descriptor bits.
// Assembly reads/stores keep their exact widths/order; OR -1 and AND 0 retain
// native read-modify-write accesses without interpreting old bits as C++ values.
__forceinline Word load_word(Word native_address) {
    Word result;
    __asm {
        mov eax, native_address
        mov eax, dword ptr [eax]
        mov result, eax
    }
    return result;
}
__forceinline void store_word(Word native_address, Word value) {
    __asm {
        mov ecx, native_address
        mov eax, value
        mov dword ptr [ecx], eax
    }
}
__forceinline std::uint8_t load_byte(Word native_address) {
    std::uint8_t result;
    __asm {
        mov ecx, native_address
        mov al, byte ptr [ecx]
        mov result, al
    }
    return result;
}
__forceinline void store_byte(Word native_address, std::uint8_t value) {
    __asm {
        mov ecx, native_address
        mov al, value
        mov byte ptr [ecx], al
    }
}
__forceinline void and_word(Word native_address, Word mask) {
    __asm {
        mov ecx, native_address
        mov eax, mask
        and dword ptr [ecx], eax
    }
}
__forceinline void or_word(Word native_address, Word mask) {
    __asm {
        mov ecx, native_address
        mov eax, mask
        or dword ptr [ecx], eax
    }
}
__forceinline bool decrement_byte_is_zero(Word native_address) {
    std::uint8_t result;
    __asm {
        mov ecx, native_address
        dec byte ptr [ecx]
        setz al
        mov result, al
    }
    return result != 0;
}
__forceinline Word class_bit(Word size_class) {
    return 0x80000000u >> (size_class & 31u);
}
__forceinline Word group_mask_address(Word region, Word group, Word size_class) {
    return region + group * 4u + (signed_word(size_class) < 32 ? 0x44u : 0xc4u);
}
__forceinline Word descriptor_mask_address(Word descriptor, Word size_class) {
    return descriptor + (signed_word(size_class) < 32 ? 0u : 4u);
}
__forceinline bool descriptor_matches(Word descriptor, Word low, Word high) {
    const Word current_high = load_word(descriptor + 4u);
    const Word current_low = load_word(descriptor);
    return ((current_high & high) | (current_low & low)) != 0;
}
__forceinline bool group_matches(Word region, Word group, Word low, Word high) {
    const Word current_high = load_word(region + group * 4u + 0xc4u);
    const Word current_low = load_word(region + group * 4u + 0x44u);
    return ((current_high & high) | (current_low & low)) != 0;
}
} // namespace

void* allocate_native_sbh_region_00c1207c(const NativeCrtSbhState& state) {
    const Word capacity = state.actual_descriptor_capacity_0109ed74;
    Word count = state.actual_descriptor_count_0109ed64;
    if (count == capacity) {
        const Word bytes = (capacity + 16u) * 20u;
        void* const current_array = state.actual_descriptors_0109ed68;
        void* const current_heap = state.actual_heap_0109e1bc;
        void* const replacement = ::HeapReAlloc(current_heap, 0, current_array, bytes);
        if (!replacement) return nullptr;
        state.actual_descriptor_capacity_0109ed74 += 16u;
        count = state.actual_descriptor_count_0109ed64;
        state.actual_descriptors_0109ed68 = replacement;
    }
    const Word descriptor = count * 20u + bits(state.actual_descriptors_0109ed68);
    void* const metadata = ::HeapAlloc(state.actual_heap_0109e1bc, 8u, 0x41c4u);
    store_word(descriptor + 0x10u, bits(metadata)); // Includes null publication.
    if (!metadata) return nullptr;

    void* const reservation = ::VirtualAlloc(nullptr, 0x100000u, 0x2000u, 4u);
    store_word(descriptor + 0xcu, bits(reservation)); // Includes null publication.
    if (!reservation) {
        void* const current_metadata = pointer(load_word(descriptor + 0x10u));
        void* const current_heap = state.actual_heap_0109e1bc;
        (void)::HeapFree(current_heap, 0, current_metadata);
        return nullptr; // Native leaves the metadata word and array growth.
    }
    or_word(descriptor + 8u, 0xffffffffu);
    store_word(descriptor, 0u);
    store_word(descriptor + 4u, 0u);
    state.actual_descriptor_count_0109ed64 += 1u;
    const Word current_metadata = load_word(descriptor + 0x10u);
    or_word(current_metadata, 0xffffffffu);
    return pointer(descriptor);
}

std::int32_t allocate_native_sbh_group_00c1212c(void* actual_descriptor) {
    const Word descriptor = bits(actual_descriptor);
    Word available = load_word(descriptor + 8u);
    const Word region = load_word(descriptor + 0x10u);
    Word group = 0;
    while (signed_word(available) >= 0) {
        available += available;
        ++group;
    }
    const Word group_record = region + group * 0x204u + 0x144u;
    Word sentinel = group_record;
    for (Word remaining = 63u; remaining != 0; --remaining) {
        store_word(sentinel + 8u, sentinel);
        store_word(sentinel + 4u, sentinel);
        sentinel += 8u;
    }
    const Word requested = (group << 15u) + load_word(descriptor + 0xcu);
    void* const committed = ::VirtualAlloc(pointer(requested), 0x8000u, 0x1000u, 4u);
    if (!committed) return -1; // Keep the already initialized 63 list pairs.

    const Word last_page = requested + 0x7000u;
    if (requested <= last_page) {
        Word pages = ((last_page - requested) >> 12u) + 1u;
        Word link = requested + 0x10u;
        do {
            or_word(link - 8u, 0xffffffffu);
            or_word(link + 0xfecu, 0xffffffffu);
            store_word(link, link + 0xffcu);
            store_word(link - 4u, 0xff0u);
            store_word(link + 4u, link - 0x1004u);
            store_word(link + 0xfe8u, 0xff0u);
            link += 0x1000u;
        } while (--pages != 0);
    }
    sentinel = group_record + 0x1f8u;
    const Word first = requested + 0xcu;
    store_word(sentinel + 4u, first);
    store_word(first + 8u, sentinel);
    const Word last = last_page + 0xcu;
    store_word(sentinel + 8u, last);
    store_word(last + 4u, sentinel);
    and_word(region + group * 4u + 0x44u, 0u);
    store_word(region + group * 4u + 0xc4u, 1u);
    const std::uint8_t old_count = load_byte(region + 0x43u);
    const std::uint8_t new_count = static_cast<std::uint8_t>(old_count + 1u);
    store_byte(region + 0x43u, new_count);
    if (old_count == 0) or_word(descriptor + 4u, 1u);
    and_word(descriptor + 8u, ~class_bit(group));
    return signed_word(group);
}

void* allocate_native_sbh_block_00c12968(
    std::uint32_t requested_bytes, const NativeCrtSbhState& state) {
    Word end = state.actual_descriptor_count_0109ed64 * 20u;
    end += bits(state.actual_descriptors_0109ed68);
    const Word total = (requested_bytes + 0x17u) & 0xfffffff0u;
    const Word requested_class = static_cast<Word>((signed_word(total) >> 4) - 1);
    Word low_mask;
    Word high_mask;
    if (signed_word(requested_class) < 32) {
        low_mask = 0xffffffffu >> (requested_class & 31u);
        high_mask = 0xffffffffu;
    } else {
        low_mask = 0;
        high_mask = 0xffffffffu >> ((requested_class - 32u) & 31u);
    }
    const Word rover = bits(state.actual_allocation_rover_0109ed70);
    Word descriptor = rover;
    while (descriptor < end && !descriptor_matches(descriptor, low_mask, high_mask)) {
        descriptor += 20u;
    }
    if (descriptor == end) {
        descriptor = bits(state.actual_descriptors_0109ed68);
        while (descriptor < rover && !descriptor_matches(descriptor, low_mask, high_mask)) {
            descriptor += 20u;
        }
        if (descriptor == rover) {
            while (descriptor < end && load_word(descriptor + 8u) == 0) descriptor += 20u;
            if (descriptor == end) {
                descriptor = bits(state.actual_descriptors_0109ed68);
                while (descriptor < rover && load_word(descriptor + 8u) == 0) descriptor += 20u;
                if (descriptor == rover) {
                    descriptor = bits(allocate_native_sbh_region_00c1207c(state));
                    if (descriptor == 0) return nullptr;
                }
            }
            const Word created_group = static_cast<Word>(
                allocate_native_sbh_group_00c1212c(pointer(descriptor)));
            const Word publication_region = load_word(descriptor + 0x10u);
            store_word(publication_region, created_group);
            const Word check_region = load_word(descriptor + 0x10u);
            if (load_word(check_region) == 0xffffffffu) return nullptr;
        }
    }
    state.actual_allocation_rover_0109ed70 = pointer(descriptor);
    const Word region = load_word(descriptor + 0x10u);
    Word group = load_word(region);
    if (group == 0xffffffffu || !group_matches(region, group, low_mask, high_mask)) {
        group = 0;
        while (!group_matches(region, group, low_mask, high_mask)) ++group;
    }
    const Word group_record = region + group * 0x204u + 0x144u;
    Word eligible = load_word(region + group * 4u + 0x44u) & low_mask;
    Word selected_class = 0;
    if (eligible == 0) {
        eligible = load_word(region + group * 4u + 0xc4u) & high_mask;
        selected_class = 32u;
    }
    while (signed_word(eligible) >= 0) {
        eligible += eligible;
        ++selected_class;
    }
    Word block = load_word(group_record + selected_class * 8u + 4u);
    const Word remainder = load_word(block) - total;
    Word remainder_class = static_cast<Word>((signed_word(remainder) >> 4) - 1);
    if (signed_word(remainder_class) > 63) remainder_class = 63u;
    if (remainder_class != selected_class) {
        const Word original_next = load_word(block + 4u);
        const Word original_previous = load_word(block + 8u);
        if (original_next == original_previous) {
            const Word mask = ~class_bit(selected_class);
            and_word(group_mask_address(region, group, selected_class), mask);
            if (decrement_byte_is_zero(region + selected_class + 4u)) {
                and_word(descriptor_mask_address(descriptor, selected_class), mask);
            }
        }
        Word previous = load_word(block + 8u);
        Word next = load_word(block + 4u);
        store_word(previous + 4u, next);
        next = load_word(block + 4u);
        previous = load_word(block + 8u);
        store_word(next + 8u, previous);
        if (remainder != 0) {
            const Word sentinel = group_record + remainder_class * 8u;
            const Word head = load_word(sentinel + 4u);
            store_word(block + 8u, sentinel);
            store_word(block + 4u, head);
            store_word(sentinel + 4u, block);
            const Word current_next = load_word(block + 4u);
            store_word(current_next + 8u, block);
            const Word inserted_next = load_word(block + 4u);
            const Word inserted_previous = load_word(block + 8u);
            if (inserted_next == inserted_previous) {
                const Word counter = region + remainder_class + 4u;
                const std::uint8_t old_count = load_byte(counter);
                const std::uint8_t new_count = static_cast<std::uint8_t>(old_count + 1u);
                store_byte(counter, new_count);
                const Word bit = class_bit(remainder_class);
                if (old_count == 0) or_word(descriptor_mask_address(descriptor, remainder_class), bit);
                or_word(group_mask_address(region, group, remainder_class), bit);
            }
        }
    }
    if (remainder != 0) {
        store_word(block, remainder);
        store_word(block + remainder - 4u, remainder);
    }
    block += remainder;
    store_word(block, total + 1u);
    store_word(block + total - 4u, total + 1u);
    const Word old_allocations = load_word(group_record);
    store_word(group_record, old_allocations + 1u);
    if (old_allocations == 0 &&
        descriptor == state.actual_retained_empty_descriptor_0109e310 &&
        group == state.actual_retained_empty_group_0109ed78) {
        state.actual_retained_empty_descriptor_0109e310 &= 0u;
    }
    store_word(region, group);
    return pointer(block + 4u);
}
} // namespace bsp
