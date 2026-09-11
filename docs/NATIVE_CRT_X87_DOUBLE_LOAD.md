# Native CRT x87 double load

The complete original `__fload_withFB` at **00C083D5..00C08418 [67 bytes]** is reconstructed as a naked MSVC Win32 entry. Its entire compiled function body is byte-identical to the freshly checked Ghidra/installed-PE body, including both returns, with **zero relocations and no byte normalization**. This is a new source interface, not a replacement installed at the original binary address. The original CRT library name remains unchanged in Ghidra.

The four-file packet owns only this function. [The audit](../reports/native_crt_x87_double_load_audit.json) records the original, complete actual archive member, object sections/symbols, source snapshots and strict build evidence. The source is [native_crt_x87_double_load.cpp](../src/native_crt_x87_double_load.cpp); the public contract is [native_crt_x87_double_load.hpp](../include/bsp/native_crt_x87_double_load.hpp).

## Original ABI and source boundary

`std::uint32_t __fastcall load_native_crt_double_x87_00c083d5(void* unused_ecx, const void* actual_double)` is **for assembly callers only**. Its integer return declaration does not model its full result or make an ordinary C++ call safe. Original EDX points at eight currently readable bytes; ECX has no input meaning. There are no stack arguments and both exits use RET. EDX and nonvolatile registers survive; the exceptional path clobbers ECX.

On normal return the entry has pushed one value onto the x87 stack. An assembly caller must provide stack capacity, consume ST0, and preserve/use the returned EFLAGS before other instructions destroy them. The integer EAX result is an additional classification result, not the floating-point value. Existing x87 control, status, tag/stack and exception state remain operative. The entry does not save or reset the floating-point environment, provide unwind/exception translation, or guarantee a return after a fault. No safe C++ wrapper or numerical interpretation that discards these outputs is supplied.

## Complete instruction order

The first high-DWORD read at C083D5 is masked by 7FF00000h and compared against 7FF00000h. If that first read selects the finite branch, C083E4 executes FLD qword from the **current** EDX address, then returns. EAX remains the first masked high word, and integer flags remain those from CMP. The loaded bytes can differ from that earlier classification read; no double snapshot or C++ conversion is equivalent to the access schedule.

If the first read selects the exceptional branch, the body performs this exact sequence:

1. Reread the high word; subtract ten bytes from ESP; OR the high word with 7FFF0000h; store it at temporary+6.
2. Reread the high word again, then read the current low word into ECX. SHLD EAX,ECX,11 and SHL ECX,11; store EAX at temporary+4, then ECX at temporary+0. These stores deliberately overlap the earlier temporary+6 DWORD.
3. FLD the ten-byte temporary, restore ESP, TEST EAX,0, and finally reread the current high word into EAX before RET. ECX retains the shifted low word. The final MOV leaves TEST's integer flags intact: ZF=1, PF=1, CF=OF=SF=0; AF is undefined.

The sequence retains the original rereads and overlapping temporary stores, including behavior when actual input storage changes or aliases accessible stack storage. It does not promise atomic reads, sanitize invalid addresses, canonicalize NaNs, or substitute a host cast. x87 instructions and any masked/unmasked exceptions remain the processor's behavior under the caller's current environment. No runtime experiment was needed or performed to claim a semantic model beyond the complete original instruction identity.

## Dependencies and validation

The function has no calls, globals, constant pointers, allocator, owner, error-dispatch or lifetime dependencies. There are no function-body COFF relocations, undefined provider symbols, or compiler-added prologue/epilogue instructions. All 21 instructions, both RETs and all 67 bytes match the original exactly in the actual object extracted from the completed `bsp_core.lib`.

The fresh isolated worktree starts at main `77cef001`. An ignored `local/x87_double_load/extra.cmake` adds only this source to the existing `bsp_core` target; shared CMake files remain untouched. `scripts/build.ps1` completed a Release Win32 build with `/W4 /WX /fp:strict`; both existing CTests passed, and all eight original seed spans matched the installed PE and current Ghidra target. Those existing tests establish repository regression checks, not direct execution of this new assembly-only entry.

The ignored `local/x87_double_load/sealed.json` manifest freezes the complete library, exact object and extracted member, function bytes, full COFF section/symbol/relocation inventory, source/header and build inputs, compiler command record, logs, original capture and seed evidence. Every analysis command verified the configured `bsp` project and `/battlestationspacific.exe` before querying. No Ghidra, shared metadata, installed binary or game state was changed.

This packet makes **no linked-address, runtime, game, or full pow/gamma completion claim**. It closes one exact source prerequisite only. BFEB10's power paths and their remaining math/error providers still require separate reconstruction, as described in [the gamma discovery](NATIVE_RENDERER_GAMMA_DISCOVERY.md).
