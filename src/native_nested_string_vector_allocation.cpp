#include "bsp/native_nested_string_vector_allocation.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <new>

namespace bsp {
void* __fastcall allocate_native_nested_string_vector_00553300(
    std::uint32_t count, void*) {
    // Exact unsigned native DIV/JNC threshold; keep max requestFFFFFFF0
    // admitted even if the actual current CRT cannot fulfill that request.
    if (count > 0x0fffffffu) throw std::bad_alloc();
    const std::uint32_t bytes = count * 16u;
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

[[noreturn]] void __cdecl throw_native_nested_string_vector_length_error_00556c90() {
    // Complete native bodies and raw EH/temporary cleanup have been matched.
    throw_native_string_vector_length_error_00431f70();
}
} // namespace bsp
