#include "bsp/native_world_border_helpers.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {

// Genuine current service for the Native 00BF681B external boundary. Keep the
// single stacked size and ordinary caller cleanup in the raw node body.
void* __cdecl allocate_world_border_bytes(std::uint32_t bytes) {
    return singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes});
}

} // namespace

static_assert(sizeof(void*) == 4);

__declspec(naked) void* __stdcall allocate_native_world_border_node_004c2310(
    std::uint32_t, std::uint32_t, const std::uint32_t*) {
    __asm {
        push 0xc //004C2310
        call allocate_world_border_bytes //004C2312: current allocator boundary
        add esp,0x4 //004C2317
        test eax,eax //004C231A
        jz node_previous //004C231C
        mov ecx,[esp+0x4] //004C231E: read actual next argument only now
        mov [eax],ecx //004C2322
    node_previous:
        lea ecx,[eax+0x4] //004C2324
        test ecx,ecx //004C2327
        jz node_value //004C2329
        mov edx,[esp+0x8] //004C232B: read actual previous argument only now
        mov [ecx],edx //004C232F
    node_value:
        lea ecx,[eax+0x8] //004C2331
        test ecx,ecx //004C2334
        jz node_done //004C2336
        mov edx,[esp+0xc] //004C2338: read actual value-cell argument only now
        mov edx,[edx] //004C233C: load the current pointed DWORD
        mov [ecx],edx //004C233E
    node_done:
        ret 0xc //004C2340
    }
}

void __fastcall increment_native_world_border_count_004cf450(
    void* actual_list_header, void*, std::uint32_t addition) {
    auto* const count_address = static_cast<char*>(actual_list_header) + 8;
    std::uint32_t count;
    std::memcpy(&count, count_address, sizeof(count));
    const std::uint32_t available = std::uint32_t{0x3fffffff} - count;
    if (available < addition) {
        // Whole Native error operation, with the established genuine current
        // SDK exception boundary; no invented legacy object or throw provider.
        throw std::length_error("list<T> too long");
    }
    const std::uint32_t result = count + addition;
    std::memcpy(count_address, &result, sizeof(result));
}

__declspec(naked) float __fastcall span_native_world_border_004b6c40(
    const void*, void*, std::uint32_t) {
    __asm {
        mov eax,[esp+0x4] //004B6C40
        test eax,eax //004B6C44
        jz span_x //004B6C46
        cmp eax,0x2 //004B6C48
        jz span_x //004B6C4B
        fld dword ptr [ecx+0x7124] //004B6C4D
        fsub dword ptr [ecx+0x7130] //004B6C53
        fstp dword ptr [esp+0x4] //004B6C59
        fld dword ptr [esp+0x4] //004B6C5D
        ret 0x4 //004B6C61
    span_x:
        fld dword ptr [ecx+0x7128] //004B6C64
        fsub dword ptr [ecx+0x711c] //004B6C6A
        fstp dword ptr [esp+0x4] //004B6C70
        fld dword ptr [esp+0x4] //004B6C74
        ret 0x4 //004B6C78
    }
}

} // namespace bsp
