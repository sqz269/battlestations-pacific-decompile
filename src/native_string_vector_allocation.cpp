#include "bsp/native_string_vector_allocation.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>
#include <new>

namespace bsp {
namespace {
// Raw native DWORD publication; no conflicting checked-vector overlay, pointer
// type punning, synthesized object or out-of-array C++ pointer arithmetic.
void write_vector_word(void* actual_vector, std::uint32_t word_offset,
    std::uint32_t value) noexcept {
    __asm {
        mov eax, actual_vector
        mov edx, word_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
} // namespace

[[noreturn]] void __cdecl throw_native_string_vector_length_error_00431f70() {
    // Both complete originals have identical message/constructor/vtable/
    // ThrowInfo order and equivalent state0 -> SBO destroy [EBP-50h] unwind.
    native_singleton_length_error_00bd0590();
}

void* __fastcall allocate_native_string_vector_0042bf40(
    std::uint32_t count, void*) {
    // Native positive-count DIV compares FFFFFFFF/count >=8 unsigned.
    // Keep1FFFFFFF admitted, even if current CRT rejects FFFFFFF8 bytes.
    if (count > 0x1fffffffu) throw std::bad_alloc();
    const std::uint32_t bytes = count * 8u;
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

std::uint32_t __fastcall buy_native_checked_string_vector_00432090(
    void* actual_vector, void*, std::uint32_t count) {
    write_vector_word(actual_vector, 0x04u, 0u); // 0043209C.
    write_vector_word(actual_vector, 0x08u, 0u); // 0043209F.
    write_vector_word(actual_vector, 0x0cu, 0u); // 004320A2; proxy+0 untouched.
    if (count == 0) return 0u; // Native XOR EAX,EAX; XOR AL,AL; RET4.
    if (count > 0x1fffffffu) {
        throw_native_string_vector_length_error_00431f70(); // 004320B6.
    }
    void* const allocation = allocate_native_string_vector_0042bf40(
        count, nullptr); // 004320BB..C3: original explicitly zeroes EDX.
    const auto begin = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(allocation));
    write_vector_word(actual_vector, 0x04u, begin); // 004320C4.
    write_vector_word(actual_vector, 0x08u, begin); // 004320C7.
    const std::uint32_t capacity_end = begin + count * 8u; // Native LEA wrap.
    write_vector_word(actual_vector, 0x0cu, capacity_end); // 004320CD.
    return (capacity_end & 0xffffff00u) | 1u; // 004320D1 MOV AL,1 only.
}
} // namespace bsp
