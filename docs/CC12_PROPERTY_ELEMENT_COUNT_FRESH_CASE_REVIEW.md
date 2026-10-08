# Property element-count future cases (CC12)

This worksheet proposes exactly four Source-derived cases, one per tag9/tag10/tag11/default path. It adds **Source credit0** and no runtime observations, Source changes, tests or fixture generator. Only the current count-leaf CPP/HPP are consumed; canonical/provider Source and old families/helpers are not accessed.

The current symbol is `bsp::native_scene_property_record_element_count_008ef7f0(const void* actual_record_ecx, uint32_t unused_edx) noexcept`, padded fastcall. The Source header specifies ECX as the actual readable record root, no stack arguments and plain RET. The tag DWORD is at **+0x04**, the conditional unsigned byte-size DWORD at **+0x24**. Root supplies genuine live stable readable fields, with no modeled bag/vector/class or child-ownership inference.

| Proposed case | Inert fill byte | Tag / little-endian bytes at+04 | Size DWORD / little-endian bytes at+24 | Expected full EAX |
|---|---|---|---|---|
| tag9_floor | A7 | 00000009 / 09 00 00 00 | 00000007 / 07 00 00 00 | 00000001 |
| tag10_high_unsigned | 5C | 0000000A / 0A 00 00 00 | FFFFFFFC / FC FF FF FF | 3FFFFFFF |
| tag11_high_unsigned | D2 | 0000000B / 0B 00 00 00 | FFFFFFFF / FF FF FF FF | 15555555 |
| default_unread_size | 69 | FFFFFFFF / FF FF FF FF | A76CD291 / 91 D2 6C A7, inert and **not read** | 00000000 |

The distinct fill bytes are proposed poison for otherwise inert input storage. They are not observed memory, pointers, GPR seeds, owner values or guards. The supplied storage extent/allocation/capture layout is unassigned. Recognized tags require +04 and+24 DWORD readability; the default requires only +04. A future default fixture may hold the poison word at+24, but absence of reads requires Root's complete code/ABI review; unchanged data alone cannot prove absence of reads.

The Source writes no bytes, allocates/frees nothing, invokes no providers, reads no DATA+20 or child bytes, and does not read phase+00, marker+2C, owner+30 or ordinal+34. All actual supplied input bytes must remain unchanged. High-bit sizes are unsigned arithmetic fields; they do not require a corresponding giant child allocation because this getter never accesses a child. This does not admit a complete owning record/class or constructor/clone round trip.

| Case | EDX body effect | Defined arithmetic flag expectation |
|---|---|---|
| tag9_floor | Same incoming EDX | SHR2: maskC5=01; CF1, PF0, ZF0, SF0 |
| tag10_high_unsigned | Same incoming EDX | SHR2: maskC5=04; CF0, PF1, ZF0, SF0 |
| tag11_high_unsigned | AAAAAAAA, unsigned product high DWORD | SHR3: maskC5=04; CF0, PF1, ZF0, SF0 |
| default_unread_size | Same incoming EDX | XOR: mask8C5=44; CF0, PF1, ZF1, SF0, OF0 |

AF and OF are undefined/excluded on the SHR2/3 exits. Default XOR leaves AF undefined/excluded, while OF is defined zero. No8D5 blanket is promised. For tag9, CF is size bit1; PF of result01 is odd. For tag10, FC's bit1 is zero and result lowFF has even parity. For tag11, `FFFFFFFF * AAAAAAAB = AAAAAAAA55555555`; EDX isAAAAAAAA, its bit2 is zero and result low55 has even parity. These are unsigned integer derivations from the current Source contract, not executed values.

At the leaf boundary ECX remains the same actual root; EBX/ESI/EDI/EBP, DF and ES are unchanged. EAX is replaced by the complete count. No stack argument is consumed; the ordinary CALL/RET return-address pair must balance under the future actual caller. Incoming EDX is a register formal, then has the case-specific physical effect above. ECX/EDX are volatile in the C++ calling convention, so observations after an ordinary wrapper require its independent byte/capture review. No frame address, capture size/offset, guard, seed or serialized field is invented, and no blanket FP/MXCSR or wrapper/helper preservation is asserted.

The arithmetic applies to all unsigned DWORD values: tags9/10 floor B/4; tag11 floors B/12 through unsigned MUL-high/SHR3; every other tag returns zero without reading B. Algebraic boundaries such as B0->0, tag11 B11->0/B12->1 and truncation of nonmultiples are explanations only, not extra runtime cases. The helper adds no positivity/divisibility/signed-negative/saturation guard. Constructor byte-span/nonoverflow/copy/lifetime restrictions are separate from this read-only field getter.

Root chooses future Source/Original/ordinary lanes, actual readable storage and live guards, captured registers/flags/ES/DF/ESP, complete fresh machine/helper/caller gates and one process. This worksheet neither consumes a compiled artifact nor claims Native/provider/runtime qualification. Existing independently qualified Source domains are unchanged.

Both actual Source files are frozen with strict Main/worktree/copy before/after hashes. `local/countcases/receipt.json` and `artifact_manifest.json` seal the exact recursive local inventory; only those exact root filenames are excluded. All Source copies, utilities, logs, stops, doc/report snapshots and commit records are included. Main commit context is generation history, not an assertion that mutable metadata remains fixed.
