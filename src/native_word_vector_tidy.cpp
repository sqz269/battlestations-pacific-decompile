#include "bsp/native_word_vector_tidy.hpp"

#include "bsp/native_dword_vector_copy_primitives.hpp"

namespace bsp {

void __fastcall tidy_native_word_vector_range_element_004d29d0(
    void* /*unused_owner*/, std::uint32_t /*unused_edx*/, void* stack_header) {
    tidy_native_dword_vector_004cb130(stack_header);
}

void __fastcall tidy_native_word_vector_storage_004cdce0(void* actual_header) {
    tidy_native_dword_vector_004cb130(actual_header);
}

} // namespace bsp
