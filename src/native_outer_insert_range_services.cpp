#include "bsp/native_outer_insert_range_services.hpp"
#include "bsp/native_string_vector_copy_construct.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
Word read(Word base, Word byte_offset) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
void write(Word base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
Word sar_four(Word value) noexcept {
    return (value >> 4) | ((value & 0x80000000u) ? 0xf0000000u : 0u);
}
void swap_source_first(Word source, Word destination, Word byte_offset) noexcept {
    const Word a = read(source, byte_offset);
    const Word b = read(destination, byte_offset);
    write(destination, byte_offset, a);
    write(source, byte_offset, b);
}
void tidy_prefix(Word begin, Word end, NativeStringRawPoolContext& strings) {
    while (begin != end) {
        tidy_native_string_header_vector_00432440(pointer(begin), strings);
        begin += 16u;
    }
}
} // namespace

void* __fastcall transfer_native_inner_vectors_to_empty_006a3200(
    void* first, const void* last, void* destination, const void*, Word, Word,
    NativeStringRawPoolContext& strings) {
    const Word end = bits(last), initial_destination = bits(destination);
    Word source = bits(first), output = initial_destination;
    Word completed = output, cached_empty_begin = 0u;
    Word empty[4]; // Opaque+0 is deliberately not initialized or read.
    empty[1] = 0u;
    empty[2] = 0u;
    empty[3] = 0u;
    const Word empty_address = bits(empty);
    try {
        try {
            while (source != end) {
                if (output != 0u) {
                    construct_native_string_header_vector_copy_00557bc0(
                        pointer(output), 0u, empty, strings);
                    cached_empty_begin = read(empty_address, 4u);
                }
                swap_source_first(source, output, 4u);
                // Native+8 loads destination before source.
                const Word old_destination_end = read(output, 8u);
                const Word old_source_end = read(source, 8u);
                write(output, 8u, old_source_end);
                write(source, 8u, old_destination_end);
                const Word old_source_capacity = read(source, 12u);
                const Word old_destination_capacity = read(output, 12u);
                write(output, 12u, old_source_capacity);
                output += 16u;
                write(source, 12u, old_destination_capacity);
                completed = output;
                source += 16u;
            }
        } catch (...) {
            tidy_prefix(initial_destination, completed, strings);
            throw;
        }
    } catch (...) {
        destroy_native_string_header_vector_004324a0(empty, strings);
        throw;
    }
    // State-1 before normal cleanup; captured begin, CURRENT empty end.
    if (cached_empty_begin != 0u) {
        destroy_native_string_header_range_00432050(pointer(cached_empty_begin),
            pointer(read(empty_address, 8u)), strings);
        singleton_lifetime_free(pointer(cached_empty_begin));
    }
    return pointer(output);
}

void* __fastcall transfer_native_inner_vectors_dispatch_006a4b50(
    const void* owner, Word, void* first, const void* last, void* destination,
    NativeStringRawPoolContext& strings) {
    return transfer_native_inner_vectors_to_empty_006a3200(first, last,
        destination, owner, bits(destination), bits(owner) & 0xffffff00u, strings);
}

void* __cdecl swap_native_inner_vectors_backward_006a3300(void* first,
    const void* last, void* destination_end, Word, Word, Word) {
    const Word begin = bits(first);
    Word source = bits(last), destination = bits(destination_end);
    while (source != begin) {
        const Word source_begin = read(source, 0xfffffff4u);
        const Word destination_begin = read(destination, 0xfffffff4u);
        source -= 16u;
        destination -= 16u;
        write(destination, 4u, source_begin);
        write(source, 4u, destination_begin);
        swap_source_first(source, destination, 8u);
        swap_source_first(source, destination, 12u);
    }
    return pointer(destination);
}

void* __cdecl swap_native_inner_vectors_backward_dispatch_006a4b80(
    void* first, const void* last, void* destination_end) {
    const Word captured_first = bits(first), captured_last = bits(last);
    const Word captured_destination = bits(destination_end);
    swap_native_inner_vectors_backward_006a3300(first, last, destination_end,
        captured_destination, captured_destination, 0u);
    // Native last tag retains incoming ambient ECX high24 bits, but the child
    // never consumes that word. No synthetic Source register input is added.
    return pointer(captured_destination - sar_four(captured_last - captured_first) * 16u);
}

void __fastcall fill_native_inner_vectors_uninitialized_006a43a0(
    void* destination, Word count, const void* value, const void*, Word, Word,
    NativeStringRawPoolContext& strings) {
    const Word begin = bits(destination);
    Word current = begin, completed = begin;
    try {
        while (count != 0u) {
            if (current != 0u)
                construct_native_string_header_vector_copy_00557bc0(
                    pointer(current), 0u, value, strings);
            --count;
            current += 16u;
            completed = current;
        }
    } catch (...) {
        tidy_prefix(begin, completed, strings);
        throw;
    }
}

void* __fastcall fill_native_inner_vectors_dispatch_006a5170(
    const void* owner, Word, void* destination, Word count, const void* value,
    NativeStringRawPoolContext& strings) {
    const Word original_destination = bits(destination), original_count = count;
    fill_native_inner_vectors_uninitialized_006a43a0(destination, count, value,
        owner, bits(value), bits(owner) & 0xffffff00u, strings);
    return pointer(original_destination + original_count * 16u);
}
} // namespace bsp
