#include "bsp/native_dword_assignment_copy_dispatch.hpp"

#include "bsp/native_dword_uninitialized_copy.hpp"

namespace bsp {

void* __fastcall copy_native_dword_assignment_uninitialized_dispatch_006974b0(
    void* owner, std::uint32_t unused_edx, const void* first,
    const void* last, void* destination) {
    return copy_native_dword_uninitialized_dispatch_00697860(
        owner, unused_edx, first, last, destination);
}

} // namespace bsp
