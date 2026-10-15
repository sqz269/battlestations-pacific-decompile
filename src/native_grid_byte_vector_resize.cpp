#include "bsp/native_grid_byte_vector_resize.hpp"
#include "bsp/native_byte_range_primitives.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {

// Consumed allocator<char> semantics from 00418540: zero also calls new(0).
// For every positive DWORD n, UINT32_MAX/n >= 1, so its bad_alloc branch is
// unreachable. Reuse the existing malloc/new-handler loop and matched free.
__declspec(noinline) void* __stdcall allocate_byte_block(std::uint32_t bytes) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}
__declspec(noinline) void __cdecl free_byte_block(void* block) {
    singleton_lifetime_free(block);
}
[[noreturn]] __declspec(noinline) void __cdecl throw_vector_length() {
    // 004200D0 genuinely throws. Current SDK object/type/allocator identity is
    // qualified; do not fabricate a legacy exception record or a returning stub.
    throw std::length_error("vector<T> too long");
}

} // namespace

__declspec(naked) void __fastcall native_grid_byte_vector_resize_004219a0(
    NativeGridByteVector*, void*, std::uint32_t, std::uint8_t) {
    __asm {
        sub esp, 8
        push ebx
        push ebp
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi + 4]
        test eax, eax
        push edi
        jnz have_size
        xor ecx, ecx
        jmp size_ready
    have_size:
        mov ecx, dword ptr [esi + 8]
        sub ecx, eax
    size_ready:
        mov edi, dword ptr [esp + 1ch]
        cmp ecx, edi
        jnc maybe_shrink
        test eax, eax
        jnz have_grow_size
        xor ebx, ebx
        jmp grow_size_ready
    have_grow_size:
        mov ebx, dword ptr [esi + 8]
        sub ebx, eax
    grow_size_ready:
        mov ebp, dword ptr [esi + 8]
        cmp eax, ebp
        jbe grow_validated
        call invoke_native_invalid_parameter_00bf6713 // 004219D5
    grow_validated:
        lea eax, [esp + 20h]
        push eax
        sub edi, ebx
        push edi
        push ebp
        push esi
        mov ecx, esi
        call native_grid_byte_vector_insert_fill_00420dd0 // 004219E6
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 8
        ret 8
    maybe_shrink:
        test eax, eax
        jz resized
        mov ebp, dword ptr [esi + 8]
        mov ecx, ebp
        sub ecx, eax
        cmp edi, ecx
        jnc resized
        cmp eax, ebp
        jbe shrink_first_validated
        call invoke_native_invalid_parameter_00bf6713 // 00421A08
    shrink_first_validated:
        mov ebx, dword ptr [esi + 4]
        cmp ebx, dword ptr [esi + 8]
        jbe shrink_second_validated
        call invoke_native_invalid_parameter_00bf6713 // 00421A15
    shrink_second_validated:
        add edi, ebx
        cmp edi, dword ptr [esi + 8]
        mov dword ptr [esp + 14h], ebx
        ja invalid_shrink_position
        cmp edi, dword ptr [esi + 4]
        jnc shrink_position_validated
    invalid_shrink_position:
        call invoke_native_invalid_parameter_00bf6713 // 00421A2A
    shrink_position_validated:
        push ebp
        push esi
        push edi
        push esi
        lea ecx, [esp + 20h]
        push ecx
        mov ecx, esi
        call native_grid_byte_vector_erase_checked_0041d7d0 // 00421A3A
    resized:
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 8
        ret 8
    }
}

__declspec(naked) void __fastcall native_grid_byte_vector_insert_fill_00420dd0(
    NativeGridByteVector*, void*, NativeGridByteIterator, std::uint32_t, const std::uint8_t*) {
    __asm {
        mov eax, dword ptr [esp + 10h]
        push ebx
        push ebp
        push esi
        mov esi, ecx
        mov cl, byte ptr [eax]
        mov eax, dword ptr [esi + 4]
        test eax, eax
        mov byte ptr [esp + 1ch], cl
        jnz have_capacity
        xor ebp, ebp
        jmp capacity_ready
    have_capacity:
        mov ebp, dword ptr [esi + 0ch]
        sub ebp, eax
    capacity_ready:
        mov ebx, dword ptr [esp + 18h]
        test ebx, ebx
        jz inserted
        test eax, eax
        jnz have_size
        xor ecx, ecx
        jmp size_ready
    have_size:
        mov ecx, dword ptr [esi + 8]
        sub ecx, eax
    size_ready:
        or edx, 0ffffffffh
        sub edx, ecx
        cmp edx, ebx
        jnc length_validated
        call throw_vector_length // 00420E11; genuinely does not return
    length_validated:
        test eax, eax
        jnz have_required_size
        xor ecx, ecx
        jmp required_size_ready
    have_required_size:
        mov ecx, dword ptr [esi + 8]
        sub ecx, eax
    required_size_ready:
        add ecx, ebx
        cmp ebp, ecx
        push edi
        jnc insert_with_capacity
        mov ecx, ebp
        shr ecx, 1
        or edx, 0ffffffffh
        sub edx, ecx
        cmp edx, ebp
        jnc grow_geometric
        xor ebp, ebp
        jmp growth_ready
    grow_geometric:
        add ebp, ecx
    growth_ready:
        test eax, eax
        jnz have_minimum_size
        xor ecx, ecx
        jmp minimum_size_ready
    have_minimum_size:
        mov ecx, dword ptr [esi + 8]
        sub ecx, eax
    minimum_size_ready:
        add ecx, ebx
        cmp ebp, ecx
        jnc capacity_selected
        test eax, eax
        jnz have_exact_size
        xor ecx, ecx
        jmp exact_size_ready
    have_exact_size:
        mov ecx, dword ptr [esi + 8]
        sub ecx, eax
    exact_size_ready:
        lea ebp, [ecx + ebx]
    capacity_selected:
        push ebp
        mov ecx, esi
        call allocate_byte_block // 00420E67 / consumed 00418540
        mov ecx, dword ptr [esi + 4]
        mov edi, eax
        mov eax, dword ptr [esp + 18h]
        sub eax, ecx
        lea edx, [eax + edi]
        mov dword ptr [esp + 1ch], edx
        jz prefix_copied
        push eax
        push ecx
        push eax
        push edi
        call memmove_s // 00420E84
        add esp, 10h
    prefix_copied:
        mov ecx, dword ptr [esp + 1ch]
        lea eax, [esp + 20h]
        push eax
        push ebx
        push ecx
        mov ecx, esi
        call fill_n_native_byte_range_0041f450 // 00420E99
        mov ecx, dword ptr [esi + 8]
        mov edx, dword ptr [esp + 18h]
        sub ecx, edx
        jz suffix_copied
        push ecx
        push edx
        push ecx
        push eax
        call memmove_s // 00420EAD
        add esp, 10h
    suffix_copied:
        mov ecx, dword ptr [esi + 4]
        test ecx, ecx
        jnz have_old_size
        xor eax, eax
        jmp old_size_ready
    have_old_size:
        mov eax, dword ptr [esi + 8]
        sub eax, ecx
    old_size_ready:
        add ebx, eax
        test ecx, ecx
        jz old_storage_freed
        push ecx
        call free_byte_block // 00420ECC / 00BF65AC
        add esp, 4
    old_storage_freed:
        lea edx, [edi + ebp]
        lea eax, [edi + ebx]
        mov dword ptr [esi + 4], edi
        pop edi
        mov dword ptr [esi + 0ch], edx
        mov dword ptr [esi + 8], eax
        pop esi
        pop ebp
        pop ebx
        ret 10h
    insert_with_capacity:
        mov ebp, dword ptr [esi + 8]
        mov edi, dword ptr [esp + 18h]
        mov ecx, ebp
        sub ecx, edi
        cmp ecx, ebx
        mov ecx, esi
        jnc long_tail
        lea edx, [edi + ebx]
        push edx
        push ebp
        push edi
        call copy_native_byte_range_0041e630 // 00420F01
        mov eax, dword ptr [esi + 8]
        mov edx, edi
        lea ecx, [esp + 20h]
        push ecx
        sub edx, eax
        add edx, ebx
        push edx
        push eax
        mov ecx, esi
        call fill_n_native_byte_range_0041f450 // 00420F18
        add dword ptr [esi + 8], ebx
        mov esi, dword ptr [esi + 8]
        lea eax, [esp + 20h]
        push eax
        sub esi, ebx
        push esi
        push edi
        call fill_native_byte_range_0041c630 // 00420F2C
        add esp, 0ch
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret 10h
    long_tail:
        push ebp
        mov eax, ebp
        sub eax, ebx
        push ebp
        push eax
        mov dword ptr [esp + 28h], eax
        call copy_native_byte_range_0041e630 // 00420F46
        mov ecx, dword ptr [esp + 1ch]
        push ebp
        push ecx
        push edi
        mov dword ptr [esi + 8], eax
        call copy_backward_native_byte_range_0041c650 // 00420F55
        lea edx, [esp + 2ch]
        push edx
        lea eax, [edi + ebx]
        push eax
        push edi
        call fill_native_byte_range_0041c630 // 00420F64
        add esp, 18h
        pop edi
    inserted:
        pop esi
        pop ebp
        pop ebx
        ret 10h
    }
}

__declspec(naked) NativeGridByteIterator* __fastcall native_grid_byte_vector_erase_checked_0041d7d0(
    NativeGridByteVector*, void*, NativeGridByteIterator*, NativeGridByteIterator,
    NativeGridByteIterator) {
    __asm {
        push ebp
        mov ebp, dword ptr [esp + 0ch]
        test ebp, ebp
        push esi
        push edi
        mov edi, ecx
        jz invalid_owners
        cmp ebp, dword ptr [esp + 1ch]
        jz owners_validated
    invalid_owners:
        call invoke_native_invalid_parameter_00bf6713 // 0041D7E3
    owners_validated:
        mov esi, dword ptr [esp + 18h]
        mov ecx, dword ptr [esp + 20h]
        cmp esi, ecx
        jz write_result
        mov eax, dword ptr [edi + 8]
        sub eax, ecx
        test eax, eax
        push ebx
        lea ebx, [eax + esi]
        jle tail_copied
        push eax
        push ecx
        push eax
        push esi
        call memmove_s // 0041D805
        add esp, 10h
    tail_copied:
        mov dword ptr [edi + 8], ebx
        pop ebx
    write_result:
        mov eax, dword ptr [esp + 10h]
        pop edi
        mov dword ptr [eax + 4], esi
        pop esi
        mov dword ptr [eax], ebp
        pop ebp
        ret 14h
    }
}

} // namespace bsp
