#include "bsp/native_dword_vector_placement.hpp"

#include "bsp/native_dword_vector_copy_construct.hpp"

namespace bsp {

void __fastcall place_native_checked_dword_vector_copy_00697d40(
    void* destination, const void* source) {
    if (destination != nullptr)
        construct_native_checked_dword_vector_copy_00557590(destination, 0u, source);
}

void __stdcall place_native_checked_dword_vector_copy_00697ec0(
    void* destination, const void* source) {
    if (destination != nullptr)
        construct_native_checked_dword_vector_copy_00557590(destination, 0u, source);
}

} // namespace bsp
