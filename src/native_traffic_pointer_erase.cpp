#include "bsp/native_traffic_pointer_erase.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"

#include <cstddef>
#include <cstring>

namespace bsp {
namespace {

// The actual selected current SDK four-argument API. This ordinary cdecl
// bridge keeps the owned body's E8 call and physical four-word caller cleanup.
// Its emitted import/forwarder is accounted for separately from Native BF67A7.
int __cdecl move_traffic_pointer_words(
    void* destination, std::size_t capacity,
    const void* source, std::size_t count) {
    return ::memmove_s(destination, capacity, source, count);
}

} // namespace

static_assert(sizeof(void*) == 4);

__declspec(naked) void* __fastcall erase_native_traffic_pointer_range_00488ff0(
    void*, void*, void*, std::uint32_t, void*, std::uint32_t, const void*) {
    __asm {
        push ebp //00488FF0
        mov ebp,[esp+0xc] //00488FF1: capture first-owner
        test ebp,ebp //00488FF5
        push esi //00488FF7
        push edi //00488FF8
        mov edi,ecx //00488FF9: capture actual vector
        jz erase_invalid //00488FFB
        cmp ebp,[esp+0x1c] //00488FFD: actual last-owner argument
        jz erase_positions //00489001
    erase_invalid:
        call invoke_native_invalid_parameter_00bf6713 //00489003
    erase_positions:
        mov esi,[esp+0x18] //00489008: actual first, after diagnostic
        mov edx,[esp+0x20] //0048900C: actual last, after diagnostic
        cmp esi,edx //00489010
        jz erase_output //00489012
        mov eax,[edi+0x8] //00489014: current end
        sub eax,edx //00489017: DWORD wrap
        sar eax,0x2 //00489019: signed suffix count
        test eax,eax //0048901C
        lea ecx,[eax*0x4] //0048901E: DWORD wrap, retain TEST flags
        push ebx //00489025
        lea ebx,[ecx+esi] //00489026: captured new end
        jle erase_publish //00489029: count<=0 still publishes
        push ecx //0048902B: actual count bytes
        push edx //0048902C: actual source
        push ecx //0048902D: actual destination capacity
        push esi //0048902E: actual destination
        call move_traffic_pointer_words //0048902F: genuine current SDK boundary
        add esp,0x10 //00489034: four physical words
    erase_publish:
        mov [edi+0x8],ebx //00489037: publish after copy, including error return
        pop ebx //0048903A
    erase_output:
        mov eax,[esp+0x10] //0048903B: CURRENT actual out argument, after publication
        pop edi //0048903F
        mov [eax+0x4],esi //00489040: position first
        pop esi //00489043
        mov [eax],ebp //00489044: captured owner second
        pop ebp //00489046
        ret 0x14 //00489047
    }
}

} // namespace bsp
