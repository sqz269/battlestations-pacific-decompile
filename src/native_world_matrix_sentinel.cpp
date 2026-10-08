#include "bsp/native_world_matrix_sentinel.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native World sentinel storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(std::size_t) == 4);
static_assert(sizeof(SingletonAllocationRequest) == 12
    && offsetof(SingletonAllocationRequest, host_bytes) == 8);

// Concrete CDECL binding for the original PUSH 6Ch; CALL allocator site.
// The trivial request remains alive throughout the synchronous allocator call.
__declspec(noinline) void* __cdecl allocate_sentinel_storage(
    std::uint32_t bytes) {
    const SingletonAllocationRequest request{
        SingletonAllocationKind::object, bytes, bytes};
    return singleton_lifetime_allocate(request);
}
} // namespace

// No prologue, local EH object, projection, World store, or memset.
// Expected complete body: 26 bytes/11 instructions; only CALL operand [3,7)
// binds to allocate_sentinel_storage. Emission still requires a future build.
__declspec(naked) void* __cdecl create_native_world_matrix_sentinel_004c3080() {
    __asm {
        push 06Ch
        call allocate_sentinel_storage
        add esp, 4
        test eax, eax
        jz second_link
        mov dword ptr [eax], eax
    second_link:
        lea ecx, [eax + 4]
        test ecx, ecx
        jz finished
        mov dword ptr [ecx], eax
    finished:
        ret
    }
}
} // namespace bsp
