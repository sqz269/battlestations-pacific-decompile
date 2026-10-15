#include "bsp/native_scene_dword_vector_allocation.hpp"

#include "bsp/native_singleton_vector_allocation.hpp"

namespace bsp {

void* __fastcall allocate_native_scene_dword_vector_00466ce0(
    std::uint32_t count, std::uint32_t) {
    // Both complete originals have no consumed incoming EDX. The canonical
    // interface reserves that slot; this supplied value is likewise unused.
    return native_singleton_pointer_allocate_00bcfeb0(count, nullptr);
}

[[noreturn]] void throw_native_scene_dword_vector_length_0046a400() {
    // Includes the completed-message guard matching native state0/action
    // 00C61C50, armed only after counted assignment returns. No new raw
    // compiler-action function or private-frame projection is introduced.
    native_singleton_length_error_00bd0590();
}

} // namespace bsp
