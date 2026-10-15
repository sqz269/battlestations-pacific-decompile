#include "bsp/native_outer_vector_construct_services.hpp"

#include "bsp/native_nested_string_vector_allocation.hpp"
#include "bsp/native_nested_string_vector_copy.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word), "Native DWORD address width");

Word address(const void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}

void write(Word base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
} // namespace

Word __fastcall initialize_native_outer_vector_storage_00557250(
    void* header, Word unused_edx, Word count) {
    (void)unused_edx;
    const Word target = address(header);
    const Word captured_count = count;
    write(target, 4u, 0u);
    write(target, 8u, 0u);
    write(target, 12u, 0u);
    if (captured_count == 0u)
        return 0u;
    if (captured_count > 0x0fffffffu)
        throw_native_nested_string_vector_length_error_00556c90();
    const Word storage = address(allocate_native_nested_string_vector_00553300(
        captured_count, nullptr));
    write(target, 12u, storage + (captured_count << 4u));
    write(target, 4u, storage);
    write(target, 8u, storage);
    return (storage & 0xffffff00u) | 1u;
}

void* __fastcall copy_native_inner_vectors_uninitialized_006a3350(
    const void* first, const void* last, void* destination,
    Word unused_owner, Word unused_tag3, Word unused_tag4,
    NativeStringRawPoolContext& strings) {
    return copy_native_nested_string_vector_uninitialized_range_00559e00(
        first, last, destination, unused_owner, unused_tag3, unused_tag4, strings);
}

void* __fastcall copy_native_inner_vectors_dispatch_006a4c00(
    Word unused_owner, Word unused_edx, const void* first,
    const void* last, void* destination, NativeStringRawPoolContext& strings) {
    (void)unused_edx;
    return copy_native_inner_vectors_uninitialized_006a3350(
        first, last, destination, unused_owner, address(destination),
        unused_owner & 0xffffff00u, strings);
}
} // namespace bsp
