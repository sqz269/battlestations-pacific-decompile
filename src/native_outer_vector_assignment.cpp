#include "bsp/native_outer_vector_assignment.hpp"
#include "bsp/native_checked_string_vector_ranges.hpp"
#include "bsp/native_outer_string_vector_cleanup.hpp"
#include "bsp/native_outer_string_vector_cleanup_wrappers.hpp"
#include "bsp/native_outer_vector_construct_services.hpp"
#include "bsp/native_outer_vector_erase_range.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word address(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word word(Word base, Word byte_offset) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
void put(Word base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
Word sar_four(Word value) noexcept {
    return (value >> 4u) | ((value & 0x80000000u) ? 0xf0000000u : 0u);
}
} // namespace

void* __fastcall assign_native_outer_vector_006a6680(void* destination, Word,
    const void* source, NativeStringRawPoolContext& strings) {
    const Word target = address(destination);
    const Word source_header = address(source);
    if (target == source_header) return destination;

    const Word source_begin = word(source_header, 4u);
    Word source_end = 0u;
    Word source_count = 0u;
    if (source_begin != 0u) {
        source_end = word(source_header, 8u);
        source_count = sar_four(source_end - source_begin);
    }
    if (source_count == 0u) {
        clear_native_outer_vector_via_erase_006a6310(destination, strings);
        return destination;
    }

    const Word target_begin = word(target, 4u);
    Word target_size = 0u;
    if (target_begin != 0u)
        target_size = sar_four(word(target, 8u) - target_begin);

    if (source_count <= target_size) {
        const Word copied_end = address(
            copy_native_checked_string_vector_range_006a29c0(
                pointer(source_begin), pointer(source_end),
                pointer(target_begin), strings));
        const Word current_target_end = word(target, 8u);
        destroy_native_outer_string_vector_range_0055a3a0(pointer(copied_end),
            pointer(current_target_end), destination, source_header, strings);

        const Word current_source_begin = word(source_header, 4u);
        Word current_source_count = 0u;
        if (current_source_begin != 0u)
            current_source_count = sar_four(
                word(source_header, 8u) - current_source_begin);
        const Word published_end = (current_source_count << 4u) + word(target, 4u);
        put(target, 8u, published_end);
        return destination;
    }

    Word target_capacity = 0u;
    if (target_begin != 0u)
        target_capacity = sar_four(word(target, 12u) - target_begin);
    if (source_count <= target_capacity) {
        // Native repeats this size read; retain it rather than using target_size.
        Word current_size = 0u;
        if (target_begin != 0u)
            current_size = sar_four(word(target, 8u) - target_begin);
        const Word boundary = source_begin + (current_size << 4u);
        copy_native_checked_string_vector_range_006a29c0(pointer(source_begin),
            pointer(boundary), pointer(target_begin), strings);
        const Word current_target_end = word(target, 8u);
        const Word current_source_end = word(source_header, 8u);
        void* const completed = copy_native_inner_vectors_dispatch_006a4c00(
            target, 0u, pointer(boundary), pointer(current_source_end),
            pointer(current_target_end), strings);
        put(target, 8u, address(completed));
        return destination;
    }

    if (target_begin != 0u) {
        const Word current_target_end = word(target, 8u);
        destroy_native_outer_string_vector_range_dispatch_0055ac60(destination,
            0u, pointer(target_begin), pointer(current_target_end), strings);
        singleton_lifetime_free(pointer(word(target, 4u)));
    }
    const Word current_source_begin = word(source_header, 4u);
    Word current_source_count = 0u;
    if (current_source_begin != 0u)
        current_source_count = sar_four(word(source_header, 8u) - current_source_begin);
    const Word initialized = initialize_native_outer_vector_storage_00557250(
        destination, 0u, current_source_count);
    if ((initialized & 0xffu) == 0u) return destination;

    const Word fresh_target_begin = word(target, 4u);
    const Word fresh_source_end = word(source_header, 8u);
    const Word fresh_source_begin = word(source_header, 4u);
    void* const completed = copy_native_inner_vectors_dispatch_006a4c00(
        target, 0u, pointer(fresh_source_begin), pointer(fresh_source_end),
        pointer(fresh_target_begin), strings);
    put(target, 8u, address(completed));
    return destination;
}

void* __cdecl copy_native_outer_vector_range_006a6c60(const void* first,
    const void* last, void* destination, NativeStringRawPoolContext& strings) {
    Word current = address(first);
    const Word finish = address(last);
    const Word destination_begin = address(destination);
    const Word result = destination_begin + (sar_four(finish - current) << 4u);
    if (current != finish) {
        const Word delta = destination_begin - current;
        do {
            assign_native_outer_vector_006a6680(pointer(current + delta), 0u,
                pointer(current), strings);
            current += 16u;
        } while (current != finish);
    }
    return pointer(result);
}

void __cdecl fill_native_outer_vector_range_006a6ca0(void* first,
    const void* last, const void* value, NativeStringRawPoolContext& strings) {
    Word current = address(first);
    const Word finish = address(last);
    if (current != finish) {
        const void* const captured_value = value;
        do {
            assign_native_outer_vector_006a6680(pointer(current), 0u,
                captured_value, strings);
            current += 16u;
        } while (current != finish);
    }
}
} // namespace bsp
