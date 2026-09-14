#include "bsp/native_crt_sbh_free.hpp"
#include "bsp/native_crt_memmove.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native SBH free requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeCrtSbhState) == 28);

__forceinline Word bits(const void* value) {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
__forceinline void* pointer(Word value) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
// MSVC Win32 supplies the original two's-complement conversion and SAR.
// C11D68's clamp is UNSIGNED, unlike the allocation body's signed clamp.
__forceinline Word size_class(Word byte_size) {
    const Word candidate = static_cast<Word>(
        static_cast<std::int32_t>(byte_size) >> 4) - 1u;
    return candidate <= 63u ? candidate : 63u;
}
__forceinline Word class_bit(Word index) {
    return 0x80000000u >> (index & 31u);
}
__forceinline Word group_mask(Word metadata, Word group, Word index) {
    return metadata + group * 4u + (index < 32u ? 0x44u : 0xc4u);
}
__forceinline Word descriptor_mask(Word descriptor, Word index) {
    return descriptor + (index < 32u ? 0u : 4u);
}

// These raw native-width operations preserve record accesses and intervening
// alias-visible reloads; they are source operations, not invented providers.
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
__forceinline void decrement_byte(Word native_address) {
    __asm {
        mov ecx, native_address
        dec byte ptr [ecx]
    }
}
__forceinline bool decrement_word_is_zero(Word native_address) {
    std::uint8_t result;
    __asm {
        mov ecx, native_address
        dec dword ptr [ecx]
        setz al
        mov result, al
    }
    return result != 0;
}
} // namespace

void free_native_sbh_block_00c11d68(
    void* actual_descriptor, void* actual_payload,
    const NativeCrtSbhState& state,
    const volatile std::uint32_t& actual_feature_word_0109eea4) {
    Word descriptor = bits(actual_descriptor);
    const Word metadata = load_word(descriptor + 0x10u);
    const Word payload = bits(actual_payload);
    const Word region = load_word(descriptor + 0x0cu);
    Word block = payload - 4u;
    const Word group = (payload - region) >> 15;
    const Word group_record = metadata + 0x144u + group * 0x204u;
    Word size = load_word(block) - 1u;
    if ((size & 1u) != 0u) {
        return;
    }

    // C11DA6..C11E30: successor header then predecessor footer; successor
    // removal publishes its mask/counter changes before the two link writes.
    const Word successor = block + size;
    const Word successor_size = load_word(successor);
    const Word predecessor_size = load_word(block - 4u);
    if ((successor_size & 1u) == 0u) {
        const Word successor_class = size_class(successor_size);
        const Word next = load_word(successor + 4u);
        const Word previous = load_word(successor + 8u);
        if (next == previous) {
            const Word keep_mask = ~class_bit(successor_class);
            and_word(group_mask(metadata, group, successor_class), keep_mask);
            if (decrement_byte_is_zero(metadata + 4u + successor_class)) {
                and_word(descriptor_mask(descriptor, successor_class), keep_mask);
            }
        }
        const Word current_previous = load_word(successor + 8u);
        const Word current_next = load_word(successor + 4u);
        size += successor_size;
        store_word(current_previous + 4u, current_next);
        const Word reloaded_next = load_word(successor + 4u);
        const Word reloaded_previous = load_word(successor + 8u);
        store_word(reloaded_next + 8u, reloaded_previous);
    }

    Word final_class = size_class(size);
    const bool predecessor_allocated = (predecessor_size & 1u) != 0u;
    Word predecessor_class = 0u;
    if (!predecessor_allocated) {
        block -= predecessor_size;
        predecessor_class = size_class(predecessor_size);
        size += predecessor_size;
        final_class = size_class(size);
        if (predecessor_class != final_class) {
            const Word next = load_word(block + 4u);
            const Word previous = load_word(block + 8u);
            if (next == previous) {
                const Word keep_mask = ~class_bit(predecessor_class);
                and_word(group_mask(metadata, group, predecessor_class), keep_mask);
                if (decrement_byte_is_zero(metadata + 4u + predecessor_class)) {
                    and_word(descriptor_mask(descriptor, predecessor_class), keep_mask);
                }
            }
            const Word current_previous = load_word(block + 8u);
            const Word current_next = load_word(block + 4u);
            store_word(current_previous + 4u, current_next);
            const Word reloaded_next = load_word(block + 4u);
            const Word reloaded_previous = load_word(block + 8u);
            store_word(reloaded_next + 8u, reloaded_previous);
        }
    }

    // If coalescing retained the predecessor's class, retain its ring position.
    if (predecessor_allocated || predecessor_class != final_class) {
        const Word sentinel = group_record + final_class * 8u;
        const Word current_first = load_word(sentinel + 4u);
        store_word(block + 8u, sentinel);
        store_word(block + 4u, current_first);
        store_word(sentinel + 4u, block);
        const Word reloaded_next = load_word(block + 4u);
        store_word(reloaded_next + 8u, block);
        const Word next = load_word(block + 4u);
        const Word previous = load_word(block + 8u);
        if (next == previous) {
            const std::uint8_t old_count = load_byte(metadata + 4u + final_class);
            store_byte(metadata + 4u + final_class,
                static_cast<std::uint8_t>(old_count + 1u));
            if (old_count == 0u) {
                or_word(descriptor_mask(descriptor, final_class), class_bit(final_class));
            }
            or_word(group_mask(metadata, group, final_class), class_bit(final_class));
        }
    }

    store_word(block, size);
    store_word(block + size - 4u, size);
    if (!decrement_word_is_zero(group_record)) {
        return;
    }

    // C11F84..C12069: current canonical cells are deliberately reloaded after
    // each real API call and between the original metadata accesses.
    Word cached_descriptor = state.actual_retained_empty_descriptor_0109e310;
    if (cached_descriptor != 0u) {
        const Word cached_group = state.actual_retained_empty_group_0109ed78;
        auto captured_virtual_free = &::VirtualFree;
        // C11F97 captures the actual import once before the region read.
        // Keep that capture opaque to MSVC so it cannot emit two IAT reads.
        __asm {
            mov eax, captured_virtual_free
            mov captured_virtual_free, eax
        }
        const Word cached_region = load_word(cached_descriptor + 0x0cu);
        (void)captured_virtual_free(pointer(cached_region + (cached_group << 15)),
            0x8000u, 0x4000u);

        const Word current_group = state.actual_retained_empty_group_0109ed78;
        cached_descriptor = state.actual_retained_empty_descriptor_0109e310;
        or_word(cached_descriptor + 8u, class_bit(current_group));

        cached_descriptor = state.actual_retained_empty_descriptor_0109e310;
        Word current_metadata = load_word(cached_descriptor + 0x10u);
        const Word reloaded_group = state.actual_retained_empty_group_0109ed78;
        and_word(current_metadata + 0xc4u + reloaded_group * 4u, 0u);

        cached_descriptor = state.actual_retained_empty_descriptor_0109e310;
        current_metadata = load_word(cached_descriptor + 0x10u);
        decrement_byte(current_metadata + 0x43u);

        cached_descriptor = state.actual_retained_empty_descriptor_0109e310;
        current_metadata = load_word(cached_descriptor + 0x10u);
        if (load_byte(current_metadata + 0x43u) == 0u) {
            and_word(cached_descriptor + 4u, 0xfffffffeu);
            cached_descriptor = state.actual_retained_empty_descriptor_0109e310;
        }
        if (load_word(cached_descriptor + 8u) == 0xffffffffu) {
            const Word release_region = load_word(cached_descriptor + 0x0cu);
            (void)captured_virtual_free(pointer(release_region), 0u, 0x8000u);

            cached_descriptor = state.actual_retained_empty_descriptor_0109e310;
            const Word release_metadata = load_word(cached_descriptor + 0x10u);
            void* const current_heap = state.actual_heap_0109e1bc;
            (void)::HeapFree(current_heap, 0u, pointer(release_metadata));

            const Word current_count = state.actual_descriptor_count_0109ed64;
            cached_descriptor = state.actual_retained_empty_descriptor_0109e310;
            const Word current_base = bits(state.actual_descriptors_0109ed68);
            const Word copy_bytes = current_count * 0x14u - cached_descriptor
                + current_base - 0x14u;
            (void)move_native_crt_bytes_00bf87e0(pointer(cached_descriptor),
                pointer(cached_descriptor + 0x14u), copy_bytes,
                actual_feature_word_0109eea4);

            --state.actual_descriptor_count_0109ed64;
            const Word current_cached_descriptor =
                state.actual_retained_empty_descriptor_0109e310;
            if (descriptor > current_cached_descriptor) {
                descriptor -= 0x14u;
            }
            void* const reloaded_base = state.actual_descriptors_0109ed68;
            state.actual_allocation_rover_0109ed70 = reloaded_base;
        }
    }
    state.actual_retained_empty_descriptor_0109e310 = descriptor;
    state.actual_retained_empty_group_0109ed78 = group;
}
} // namespace bsp
