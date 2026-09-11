# Native CRT pow fallback

This packet reconstructs the full BFEB6D..BFED32 **453-byte** fallback instruction stream under an explicit assembly-only source binding. The compiled main body has 457 bytes because two register swaps surround the original special-value call; a separate eleven-byte private thunk supplies the parity helper's required literal pointer. Six original address operands are rebound without changing their instruction lengths. No arithmetic, argument or result snapshot is substituted.

The source is [native_crt_pow_fallback.cpp](../src/native_crt_pow_fallback.cpp), its contract is [native_crt_pow_fallback.hpp](../include/bsp/native_crt_pow_fallback.hpp), and [the audit](../reports/native_crt_pow_fallback_audit.json) pins the complete evidence. The source name is descriptive. **This is not public pow, the SSE2 dispatcher, or renderer gamma.** The original BFEB10 route can still select C19260/C19279 according to actual current state; this packet neither implements that route nor forces either dispatch cell to zero.

## Original entry and state

Enter via CALL with EAX containing the caller's already-loaded y high DWORD and ST0 containing its current y. At entry ESP holds the real return, ESP+4 the current x qword, ESP+0Ch the current y qword, and ESP+14h a readable scratch DWORD. These are original caller-owned slots; the void C++ declaration is only an assembly name and is unsafe for ordinary C++ invocation. The input EAX/ST0 are not reconstructed from those slots. Three additional free x87 slots cover the negative-base duplication and parity temporary. Normal return places the result in ST0, consumes this return, and leaves the outer caller to remove its 20-byte argument/scratch frame.

The original MOV ECX,EAX then PUSH EAX creates T, the packed-CW/scratch DWORD. T+4 is the return, T+8 x, T+10h y, and T+18h scratch. Waiting FSTCW writes only T's low word. Unless it equals 027Fh, the complete C083A5 helper reads the packed DWORD, derives its working CW, writes T+2, and loads that current word. The original low saved CW and unwritten high scratch bits are not initialized or replaced by policy. The normal restore tail and the result-status tail observe the current packed word and POP all four bytes into EDX before RET.

The fallback retains every original access width and read site. In particular, BFECD3 tests a **DWORD at T+17h**, spanning y's sign byte and three scratch bytes; it is not narrowed to a byte test. The loader C083D5 returns one x87 value, EAX and flags, and its immediate conditional branch consumes those flags without an inserted instruction. The FYL2X/exp2 and negative/zero routes retain their original x87 and CL/AX behavior, including current precision and unordered-comparison limits.

## Explicit borrowed bindings

The caller loads four otherwise-unused nonvolatile registers from the 16-byte `NativeCrtPowFallbackContext` binding description. The body never dereferences that aggregate:

| Register | Actual binding | Use |
| --- | --- | --- |
| EBP | Current DWORD 0109DD78 | Both original seven-byte CMP instructions reread this actual cell |
| EBX | D6A684 six-qword region, 48 bytes | Direct complete result-status tail |
| ESI | D7A280 half cell, 8 bytes | Complete special-value provider |
| EDI | Original E165A0 literal-region anchor | Original relative address materialization and literal reads |

The EDI region retains these original relative cells, rather than copying or synthesizing a table:

| Offset from E165A0 | Cell | Extent / little-endian bytes |
| --- | --- | --- |
| -D50h | E15850 parity half | 8: 000000000000E03F |
| -D48h | E15858 name | 4: 706F7700 (`pow` and terminator) |
| -930h | E15C70 negative NaN80 | 10: 00000000000000C0FFFF |
| +0 | E165A0 infinity | 8: 000000000000F07F |
| +8 | E165A8 negative qNaN | 8: 000000000000F8FF |
| +20h | E165C0 negative zero | 8: 0000000000000080 |
| +130h | E166D0 infinity80 | 10: 0000000000000080FF7F |

Gaps are not read. Every reached cell must remain valid and retain its actual immutable original bytes; 0109DD78 itself remains current mutable state. The two LEAs materialize the original name's address without reading or copying its characters. The six parent changes are two CMP `[EBP+disp32(0)]`, two LEA name addresses from EDI, and two ten-byte FLDs from EDI. Each `_emit` block is one fully decoded original instruction with only its address operand changed.

Both original BFED32 calls bind a new private thunk: `LEA EDX,[EDI-D50h]; JMP` to the complete parity provider. It is exactly eleven bytes, changes only intermediate EDX, and adds no return slot, flags or FP effect. Both original caller continuations are traversed until a full EDX overwrite: neither consumes the old EDX. The final normal EDX remains the current packed CW. The helper's actual CL, AX, upper register bits, waiting status reads and precision-dependent classification remain intact.

Immediately before and after only the original C19E24 CALL, `XCHG EBX,ESI` exchanges the special half and status-literal bindings. These two inserted instructions add four bytes, with no flags, stack or FP effect. The full special provider preserves EBX and saves/restores ESI, so the second swap restores both caller roles on normal return. All branch displacements, including external conditional tails, are verified against the explicit instruction map. There is no new ordinary-CALL wrapper around the CW-top tails. No binding restoration is promised after a fault or nonlocal exit.

## Special path and concrete closure

The special path allocates the original 74h bytes: an uninitialized eight-byte output cell followed by the **108-byte x87 save area**. It pushes the output pointer, allocates the original 16 argument bytes, and FSTPs x then y into those slots. The exact waiting FSAVE follows those stores; source spells its original `9B DD 71 08` as FWAIT plus FNSAVE in 32-bit environment mode. The apparent `dword ptr` assembly operand does not reduce the hardware's 108-byte save extent.

After the first register swap, the full special provider receives the original 20-byte argument layout, under the x87 environment reset by FSAVE. No default output is written. After normal return, the second swap precedes the original argument cleanup; the actual saved output pointer is POPped, FRSTOR restores the saved environment, and FLD reads the output's current qword before releasing 74h bytes. The output remains uninitialized on any provider path that does not write it; the caller's original path logic remains the validity contract. No new rollback, FP save policy or EH cleanup is introduced.

The direct closure is C083A5, C083D5, C08390, C0842E, C08479, C08330, C19E24 and BFED32. Special handling further uses full C19DC0, BFA52F, C12F3E and C28548; their original internal eight-byte arguments and readable classification tail remain unchanged. Status uses the existing full shared error body and its explicit unary+17h entry thunk. All thirteen raw original bodies, totaling 1,537 bytes including this fallback, are checked in the same actual archive.

Error handling binds the complete `legacy_crt_87except_00c27489` through the existing raw dispatcher. Before a reached error route, bind the persistent actual `LegacyCrtMathRuntime`, with the actual E16BD0 cell and owning CRT errno accessor. Its reserved-FPIEEE initialization, precision01 exclusion, FP-status scope, volatile-register differences and Win32 runtime/SEH limits remain part of this composition. The C++ provider's original byte identity is not claimed. Nonvolatile preservation through that provider and its underlying concrete services relies on their established Win32 cdecl ABI. No generic arithmetic callback or host pow is added.

## Complete code and artifact proof

The fresh worker starts at main `a6bf5952`. Twenty-four guarded original/PE captures total 1,993 distinct bytes: the owned body, complete bounded raw provider closure, original concrete-provider identity, the public caller's 84-byte entry shape, and actual literal/global spans. The full installed PE hash is checked. Each Ghidra batch verifies project `bsp`, program `/battlestationspacific.exe`, language and image base. Virtual zero-filled data is compared as mapped initial PE state; its initial zero is not a runtime policy.

The strict Release Win32 build uses the actual `bsp_core` target. An ignored CMake hook registers this source and the existing result-status source not yet registered at the worker base; all other providers are already registered. `/W4 /WX /fp:strict`, both existing CTests and all eight fresh seed checks pass. The sole diagnostic exception is C4414, scoped with warning push/disable/pop only around this naked body: MSVC diagnoses external Jcc conversion even with `near ptr`, although all four originals are already six-byte near branches. The proof checks those four complete encodings and symbol relocations. No provider is compiled separately and no new permanent or runtime suite is added.

The final spelling corrections are explicit two-operand FADDP, `near ptr` on those four branches, the narrow C4414 pragma, and two `_emit 84 CD` instructions retaining original TEST CH,CL encodings that MSVC otherwise commutes to TEST CL,CH. Both failed compiler logs and the successful-but-pre-TEST-fix archive are preserved separately; neither is the final proof artifact. A small final source diff records these approved corrections. The final FADDP bytes remain DE C1 and both TEST sites are exactly 84 CD.

The proof freezes the actual library and all nine exact source/provider archive members, recording every COFF section, symbol and relocation. It maps all 123 original fallback instructions to the 125-instruction compiled body, separately verifies the two XCHGs and eleven-byte thunk, and checks every internal branch and external CALL/Jcc/JMP binding. It checks the complete 1,084 raw provider bytes with only their established address/relocation adaptations, plus the existing five-byte unary thunk and whole concrete C++ error-provider section. The complete normal CFG proves restored binding roles and correct packed-CW stack position at every exit. The two parity continuations have a separate old-EDX dead-value proof.

Synthetic, nonoverlapping addresses are used only to check relocation targets; they are not linked or runtime postimages. The ignored `local/pow_fallback/sealed.json` pins original captures, full source contracts, compiler/build records, archive members and the finite proof. Source and object verification do not establish original-caller ABI compatibility, new hardware-exception or linked-runtime coverage, gameplay, full pow or gamma completion. Shared metadata/CMake/Ghidra, installed game files and all prior immutable evidence remain unchanged.


## Primary integration

All 1 complete entries are registered against the same strict main Win32 library `e59c0a7d198e8059d3eb65064cd1857b72e48f91e858906a448a9b38d4414ccf`; two existing CTests and eight original seed spans pass. Full453 original bytes become457 compiled bytes plus11-byte parity thunk. All123-to125 instruction mappings, six parent/current-address changes,13 raw-provider literal changes and all direct/internal/conditional bindings checked; twelve complete raw providers1084 bytes, total1537 raw original bytes. Both context XCHGs and normal CW-stack/EBX-ESI restoration at eight exits verified, oldEDX dead at both parity continuations. Preserve waiting FSAVE108-byte/FRSTOR, uninitialized current output, unaligned DWORD sign read and packed-CW scratch. Nine exact archive objects and all COFF sections; full existing actual owning-CRT error provider with inherited FP/precision/SEH boundaries. Static synthetic relocation proof only; no linked or runtime original/fallback/publicpow/SSE2/gamma/game claim. Reviewed names and evidence comments are saved with prior comments retained; all affected exports are forcibly refreshed. Immutable evidence: `local/pow_fallback_primary/`.
