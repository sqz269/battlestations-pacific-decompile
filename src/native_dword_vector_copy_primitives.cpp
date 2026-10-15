#include "bsp/native_dword_vector_copy_primitives.hpp"

#include "bsp/native_dword_uninitialized_copy.hpp"
#include "bsp/native_vector_storage_tidy.hpp"

namespace bsp {

void* __fastcall copy_native_dword_uninitialized_00555000(
    const void* first, const void* last, void* destination,
    void* unused_owner, std::uint32_t unused_metadata,
    std::uint32_t unused_tag) {
    return copy_native_dword_uninitialized_00696d40(
        first, last, destination, unused_owner, unused_metadata, unused_tag);
}

void __fastcall tidy_native_dword_vector_004cb130(void* actual_header) {
    tidy_native_vector_storage_00468840(actual_header);
}

} // namespace bsp
