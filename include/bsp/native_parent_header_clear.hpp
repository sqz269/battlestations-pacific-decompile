#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native parent header clear requires MSVC Win32.
#endif

namespace bsp {

// Hypothetical name. Whole [004BF8E0,004BF926): 70 bytes / 29 instructions.
// ECX is an actual coherent caller-owned12-byte {count,head,tail} header.
// Nodes are distinct live12-byte {previous,next,borrowed_payload} allocation
// bases produced by accepted native append in the canonical current heap.
// Header and borrowed live payloads must not alias any owned node allocation.
// All nodes are unlinked and canonically freed; header survives as {0,0,0}.
// Payload+8 is never read/destroyed. No original parent/class lifetime is bound.
// Both formals are registers; unused EDX is padding, no stack args/plain RET.
// ESI/nonvolatiles are preserved. Empty EAX/ECX/EDX are unchanged; populated
// volatile outputs are unpromised. Final CMP count0 gives CF0/PF1/AF0/ZF1/
// SF0/OF0. Real CRT requires DF0; no blanket FP/DF promise across free.
void __fastcall clear_native_parent_header_004bf8e0(
    void* actual_header, std::uint32_t unused_edx) noexcept;

} // namespace bsp
