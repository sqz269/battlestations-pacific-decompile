#pragma once

namespace bsp {

// Whole 00484540..00484597: 87 bytes / 36 instructions / one allocator call.
// Hypothetical name; supplied raw storage in the canonical current-CRT domain.
// actual_list_ecx is the actual stable 0Ch {count, head, tail} allocation.
// Existing 0Ch {previous, next, payload} nodes must be live allocations from
// singleton_lifetime_allocate/free, coherently owned by this same list root.
// The caller owns root/node lifetimes; payload identity remains borrowed.
// Allocation may invoke the real current CRT new handler or throw bad_alloc.
// Count/tail are read after allocation. No parent/class/world owner is created.
// unused_edx is fastcall padding; actual_payload is the one stacked DWORD.
// RET4; EAX=payload, ECX=actual new node, EDX=0; ESI/EDI restored. Arithmetic
// flags come from the final count increment. No blanket FP/DF preservation is
// claimed across CRT calls. Historical private CRT/EH identity is unbound.
void* __fastcall append_native_unit_list_00484540(
    void* actual_list_ecx, void* unused_edx, void* actual_payload);

}  // namespace bsp
