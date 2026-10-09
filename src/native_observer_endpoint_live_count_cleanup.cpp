#include "bsp/native_observer_endpoint_live_count_cleanup.hpp"

#include "bsp/native_pending_entity_producers.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

// Retain the complete owned raw schedule. Adapt only the explicit Source
// lifetime arguments and the admitted cdecl kill call; do not cache the count,
// entries, predicates or reference arguments before their selected reads.
__declspec(naked) void __fastcall cleanup_native_observer_endpoint_live_count_007ee620(
    void*, std::uint32_t, NativePendingEntityOwners&, NativePendingEntityProducerAccess&) {
    __asm {
        push ebp                               // 007EE620
        push esi                               // 007EE621
        mov ebp, ecx                           // 007EE622: actual raw receiver
        xor esi, esi                           // 007EE624
        cmp dword ptr [ebp + 3cch], esi         // 007EE626: initial live count
        jle short cleanup_complete             // 007EE62C: signed count <= 0
        push edi                               // 007EE62E: only iteration path
        lea edi, [ebp + 3d0h]                   // 007EE62F: inline entry address
    cleanup_entry:
        mov ecx, dword ptr [edi]                // 007EE635: current selected entry
        cmp byte ptr [ecx + 5ch], 0             // 007EE637
        jnz short cleanup_next                 // 007EE63B
        cmp dword ptr [ecx + 900h], 1           // 007EE63D: conditional second read
        jnz short cleanup_next                 // 007EE644
        push dword ptr [esp + 14h]             // Source: current access reference
        push 1                                 // 007EE646: full DWORD cause one
        push ecx                               // Source: actual selected void*
        push dword ptr [esp + 1ch]             // Source: current owners reference
        call native_pending_entity_kill_00926d90 // 007EE648: admitted Source provider
        add esp, 10h                           // Source: four cdecl arguments
    cleanup_next:
        add esi, 1                             // 007EE64D
        add edi, 4                             // 007EE650
        cmp esi, dword ptr [ebp + 3cch]         // 007EE653: reload after child
        jl short cleanup_entry                 // 007EE659: signed comparison
        pop edi                                // 007EE65B
    cleanup_complete:
        pop esi                                // 007EE65C
        mov byte ptr [ebp + 3ech], 1            // 007EE65D: after ESI, before EBP
        pop ebp                                // 007EE664
        ret 8                                  // 007EE665: Source two references
    }
}

} // namespace bsp
