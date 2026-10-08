#pragma once

namespace bsp {

// Whole 004837D0..0048381F: 79 bytes / 31 instructions / two free CALLs.
// Hypothetical name; actual raw storage in the canonical current-CRT domain.
// ECX supplies the stable 0Ch {count, head, tail} root; the one stacked DWORD
// supplies an actual coherent live 0Ch {previous, next, borrowed_payload} node
// owned by that root and produced by append_native_unit_list_00484540.
// The caller owns root/payload lifetime; node ownership is consumed by free.
// unused_edx is fastcall padding. RET4; EAX is the pre-free successor or null.
// Nonvolatile GPRs survive; ECX/EDX are volatile across the actual free call.
// Final arithmetic flags come from ADD ESP,4, not the count decrement.
// DF0 is valid CRT entry; no blanket FP/DF preservation is claimed.
// Only valid canonical-free normal behavior is admitted. Original class ABI,
// private CRT/EH, parent/world ownership and gameplay remain unbound.
void* __fastcall erase_native_unit_list_004837d0(
    void* actual_list_ecx, void* unused_edx, void* actual_node) noexcept;

}  // namespace bsp
