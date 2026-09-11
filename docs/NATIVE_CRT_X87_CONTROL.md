# Native CRT x87 control entries

The complete original **00C083A5..00C083BC [23 bytes]** control-word call helper and **00C0842E..00C0843B [13 bytes]** restoration tail are reconstructed as naked MSVC Win32 assembly. Both entire function bodies in the actual built archive member match the freshly checked original bytes exactly: **36 bytes, no relocations, no normalization, no added instructions and no dependencies**.

The source names are descriptive hypotheses. Fresh saved names were `LIBCRT_unmatched_00c083a5` and `LIBCRT_unmatched_00c0842e`; this worker made no Ghidra or metadata edits. [The audit](../reports/native_crt_x87_control_audit.json) records the original and complete object/archive/source evidence. The implementation is [native_crt_x87_control.cpp](../src/native_crt_x87_control.cpp) and the explicit public contract is [native_crt_x87_control.hpp](../include/bsp/native_crt_x87_control.hpp).

## Custom stack contracts

Both declarations are **assembly callers only**. Their `void __cdecl` declarations provide new source names, not safe ordinary C++ invocation, numeric return interfaces, or host FP-environment wrappers. Existing x87 state and FLDCW behavior apply. They do not clear status, save an environment, translate exceptions, or guarantee return after a fault. Neither entry pushes or pops an x87 data-stack value.

`prepare_native_crt_x87_control_00c083a5()` is entered using CALL with a readable/writable packed DWORD at **entry ESP+4**, below the newly pushed return address. The complete six-instruction sequence reads that DWORD into EDX, ANDs 0300h, ORs 007Fh, writes DX at **entry ESP+6**, then FLDCWs the current word at that same address. Thus the original low saved-CW word remains untouched by this function while the upper scratch word receives the working control value. The low saved word is not replaced by the working value. The final EDX is `(initial_packed & 0300h) | 007Fh`; integer flags are those of the OR. Other general registers survive. RET consumes only this helper's return address, leaving the packed slot with its caller.

`restore_native_crt_x87_control_tail_00c0842e()` is a **JMP/tail entry, never a normal CALL entry**. Its entry ESP must point directly at the packed CW DWORD, with the real return address at ESP+4. It compares the current low word to 027Fh; the unequal branch rereads that current word for FLDCW, while the equal branch skips FLDCW exactly as original. The latter is not a guarantee that independently changed live x87 state will be restored. POP EDX then reads and consumes the full current DWORD, and RET consumes the real return beneath it. Integer flags remain those of the earlier CMP; other general registers survive.

Those separate compare/load/pop reads and the original widths are retained. The EDX popped by the tail need not equal a prior snapshot of the slot if actual storage changes between accesses. There is no C++ packed-value copy, pointer callback, exception guard, default-CW substitution or extra stack adjustment. A normal CALL to the tail would put the wrong data at its entry ESP; the header makes that caller obligation explicit.

## Proof and limits

The worktree starts at main `6ccfc91e`. An ignored `local/x87_control/extra.cmake` adds only the new source to the existing `bsp_core` target; no shared CMake registration is included. `scripts/build.ps1` completes a Release Win32 build with `/W4 /WX /fp:strict`; both existing CTests pass, and all eight fresh original seed spans match the PE/current Ghidra target.

The proof freezes the sole actual built library, the complete new object and its exact extracted archive member, both compiled bodies, and every object section/symbol/relocation record. It independently decodes all 11 instructions and requires all bytes, sizes, operands and branch targets to match. Function-body relocation lists and external provider lists are empty. Source/header, compiler command record, build inputs and logs are pinned alongside the original captures in `local/x87_control/sealed.json` under `J:/PROG/battlestations-pacific-decompile-native_crt_x87_control`.

No new test or runtime fixture was needed for the unchanged, dependency-free instruction bodies. Existing tests establish repository regressions, not direct invocation of these assembly-only entries. This is **complete source and COFF-byte evidence**, with no linked-address, runtime, game, full pow or gamma completion claim. The custom entry ABI still requires a correctly constructed assembly caller. Installed-game files, Ghidra and shared metadata were preserved.
