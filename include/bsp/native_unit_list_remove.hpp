#pragma once

namespace bsp {

// Whole [004845A0,004845CE): 46 bytes / 19 instructions / one erase CALL.
// Hypothetical name; caller supplies a stable coherent 0Ch {count,head,tail}
// root and live owned 0Ch {prev,next,payload} nodes actually produced by the
// accepted append Source in the canonical current allocation/free domain.
// Payload is borrowed. Only the first matching node is erased; no payload
// dereference, destruction or separate free occurs. Empty/absent paths erase
// nothing. Every ordinary return is the requested full payload identity,
// including after node free; the erase successor return is discarded.
// ECX=root; unused_edx keeps one payload DWORD at entry ESP+4; both exits RET4.
// ESI is saved/restored. Other nonvolatiles survive the accepted erase/free ABI.
// Empty/absent final TEST zero defines CF0 PF1 ZF1 SF0 OF0 (AF undefined).
// Match inherits erase's final ADD ESP,4 flags and volatile ECX/EDX behavior.
// Valid current-CRT DF0 applies; no blanket FP/DF preservation across free.
// Original class/private CRT/EH, parent/world ownership and gameplay unbound.
void* __fastcall remove_native_unit_list_payload_004845a0(
    void* actual_list_ecx, void* unused_edx, void* borrowed_payload) noexcept;

}  // namespace bsp
