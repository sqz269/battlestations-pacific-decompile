# Native shadow draw leaves

Addresses: 00B48D00, 00B6FDC0.

This supplies the complete four-byte current physical-owner getter and eleven-byte camera-plane adjustor used by the deferred AE47E0 path. Source names are hypotheses; current Ghidra names `TRIV_body_00b48d00` and `CG_adjustor_thunk_00b6fdc0` and their comments are preserved in evidence. The worker makes no Ghidra or ledger changes.

| Entry | Original ABI and complete behavior | Source interface |
| --- | --- | --- |
| B48D00..B48D03 | ECX actual logical stream, EAX current DWORD+58, ordinary RET; no stack argument or retain | `native_logical_vertex_physical_00b48d00(const void*)` |
| B6FDC0..B6FDCA | Add2F4 to actual camera ECX, tail-jump full B656F0; its RET4 consumes the public index | `native_camera_plane_00b6fdc0(const void*, void* unused_edx, uint32_t)` |

At both B48D00 caller sites AE492F/AE4A6F, a byte offset has already been pushed for the later B4A9B0 constructor. The getter must leave that word untouched. The +58 interpretation is grounded in B4A9B0's actual physical-owner assignment; no host companion address is returned.

At AE4838, AE47E0 pushes node then index. The adjustor consumes index only, leaving node below the four subsequently copied plane words for AE2FE0. B91AA5 supplies camera ECX and its own descending index. This helper does not clamp, update the frustum or dereference the plane; the existing B656F0 provider preserves wrapped DWORD address arithmetic and the original stack ABI. It has no additional context or allocation.

Accepted evidence is the immutable ET admission and its primary review. EU pins complete original PE/live bytes, current caller ownership, names/comments, and the existing provider's actual source and build registration. Native spans are 4+11=15 bytes; no classifier, recursive parent, constructor, terminal, existing B656F0 or compiler-helper credit is added.

Source candidate validation: source/call review only, with compilation and generated-code validation pending the parent's single ES+EU build. Expected complete emitted bodies are `8B4158C3` with zero relocations and eleven bytes matching the native adjustor after one JMP rel32 operand. C3 is explicit to avoid MSVC RET0 encoding. No standalone build, probe, new tests or runtime/game validation was performed.
