# Native CRT x87 error dispatch

This packet reconstructs both original entries and their **one shared physical body at C08330..C08383 [83 bytes]**. C08330 reaches its 23-byte prefix plus the 51-byte shared tail, totaling 74 original bytes. C08347 reaches its nine-byte prologue plus that same tail, totaling 60 original bytes. The new binary entry contains the complete physical union; the new unary entry is an explicit five-byte JMP thunk to binary-entry+17h. The thunk adds no stack, register, integer-flag or FP effect.

The complete 83-byte compiled body matches the fresh original after only the direct C27489 CALL relocation is rebound to the existing concrete source provider. The unary thunk is proved separately, including its exact target and 17h relocation addend. It is a **new source entry binding**, not an original entry address or general ABI-compatibility claim. `__startOneArgErrorHandling` is the retained original library name at C08347; names in the new source are descriptive hypotheses. This worker does not rename Ghidra functions.

The source and contract are [native_crt_x87_error_dispatch.cpp](../src/native_crt_x87_error_dispatch.cpp) and [native_crt_x87_error_dispatch.hpp](../include/bsp/native_crt_x87_error_dispatch.hpp). [The audit](../reports/native_crt_x87_error_dispatch_audit.json) records the original, actual library/object and complete provider-object evidence.

## Assembly-only entry and stack contract

Both public declarations are **assembly callers only**. Their void C++ declarations do not model the inputs, result or stack slots and do not make ordinary C++ calls safe. Entry registers are EAX=error type, ECX=actual name pointer, EDX=operation and ST0=current result. Entry ESP points to this dispatch return; ESP+4 is the actual packed saved-CW slot; ESP+8 is the enclosing return/opaque slot; ESP+0Ch holds the current first-argument qword; ESP+14h holds the second-argument qword for the binary entry.

The binary path pushes EBP, allocates 20h bytes and stores the error type. It reads/stores each current second-argument DWORD before jumping over the unary prologue. The unary entry reaches that original prologue through the new JMP thunk: it allocates the same 20h-byte record and stores only the type. **Unary argument2 remains unwritten**, not copied or zero-initialized. Callers requiring a valid binary argument, including operation1Dh, must use the binary entry; this packet adds no validation branch.

The actual record is 32 bytes: type+0, name+4, argument1+8, argument2+10h and result+18h. Source static assertions verify every offset against the existing `CameraAxesCrtException` declaration. The shared tail FSTPs result to double **before** storing name and before loading the first argument's current words. This preserves conversion/exception timing and the individual raw reads; there is no argument or result snapshot.

The record and actual saved-CW pointer are passed directly to `legacy_crt_87except_00c27489(operation, &record, &savedCW)` using its established cdecl layout. Only after normal return does the tail FLD the record's current result, compare the current saved-CW word to 027Fh, and conditionally reread it for FLDCW. LEAVE/RET restore the frame and remove only this dispatch return. The packed CW and enclosing return still belong to the outer caller.

The x87 input is replaced by the reloaded result on normal return. EBP and nonvolatile general registers survive. Final integer flags come from the saved-CW comparison. Volatile EAX/ECX/EDX outputs depend on the concrete C++ provider; their incidental original values are not promised. These wrappers install no EH guard, ownership terminal, rollback or cleanup. Unhandled faults/nonlocal exits are not promised a return or CW restoration.

## Concrete error-provider binding

Before entry, the caller must bind a persistent actual `LegacyCrtMathRuntime` through the existing API. That runtime borrows the actual E16BD0 matherr-bypass cell and the owning CRT's per-thread errno accessor. In a rebuilt process the latter may be its actual CRT `_errno`; inside the original process it is the original runtime service. The runtime and both borrowed bindings must remain alive through the call. This packet adds no new runtime, generic callback, math policy or host-pow replacement.

The completed `legacy_crt_87except_00c27489` source explicitly handles binary64 operation1Dh and copies the initialized second operand for 10h/16h/1Dh. It provides its existing x87 status handling, actual continuable Win32 `RaiseException`, continuation-result/control mutation, installed default-matherr behavior and errno writes. It reads but does not directly write the caller's saved-CW pointer. The assembly tail observes the provider's current result only after return and then independently rereads the actual caller slot.

The provider's established restrictions remain part of this composition: reserved FPIEEE bytes are initialized instead of retaining native unspecified stack bytes; reserved x87 precision encoding01 is not contracted; unused binary32 handling is not exposed; prior differential checks cover exception flags rather than every FP instruction/data pointer or condition-code bit. Existing binding checks and native-runtime/TLS service boundaries are inherited. This is not a new complete native FP-state or SEH-ABI equivalence claim. The present packet verifies the operation1Dh source route and exact call binding, **not a new operation1Dh runtime case**.

## Complete code and provenance proof

The fresh worktree starts at main `a0c55a6e`. Three guarded live/PE captures total 339 bytes: the owned83-byte physical union, the existing252-byte original C27489 provider identity, and the actual four-byte E16BD0 initial cell. Every Ghidra query verifies project `bsp` and `/battlestationspacific.exe` before reading.

The strict Release Win32 `bsp_core` build includes this source through an ignored CMake hook. `/W4 /WX /fp:strict`, both existing CTests and all eight fresh seed checks pass. Shared CMake registration, metadata and Ghidra are untouched.

The proof checks every byte and instruction of the full original physical union, allowing only the four-byte REL32 C27489 operand at core+3Ch. It independently traverses both entry CFGs: binary74, unary60, shared51. The new unary object body must be exactly one five-byte JMP with one REL32 relocation to the binary symbol and addend17h; decoding after binding must target the original unary prologue's corresponding offset. The original branch over the unary prologue, both possible final CW paths and the shared RET remain unchanged.

The sole actual built library, both complete archive members (`native_crt_x87_error_dispatch.obj` and `legacy_crt_math.obj`), all their sections/symbols/relocations, the whole compiled C27489 section, original captures, source/header contracts, compiler command record and build inputs/logs are frozen under `local/x87_error_dispatch/`. `sealed.json` pins the immutable bundle. Provider object provenance is explicit; its C++ body is not asserted byte-identical to the original CRT body.

No runtime suite was added: the owned instruction streams and new entry transfer are completely proved, and the direct provider's existing bounded contract is retained. Existing tests establish repository regression checks, not direct execution of these new entries. There is **no linked-address, runtime, game, full pow or gamma completion claim**. Original game files and all previous sealed worker evidence remain unchanged.


## Primary integration

Primary registered both complete source entries and passed the strict Win32 build, both existing CTests and eight fresh seeds. All four packets were checked against the same frozen main library `5f30026b617779bef97bf7e96b9abd5f19b97374811502497f09d17a0593a0c8`. Complete83-byte core has only one direct C27489 CALL relocation; separate unary5-byte JMP has17h addend. Both full actual archive objects and87 COFF sections proved. Persistent actual LegacyCrtMathRuntime/E16BD0/owning errno binding required. Established provider restrictions on reservedFPIEEE initialization, precision01, volatile outputs and FP/status/Win32 exception scope remain. No new operation1D runtime, linked runtime, native SEH/caller ABI, pow/gamma or game claim. Reviewed names and evidence are saved with prior comments retained; correct CRT library names remain. All affected exports were forcibly refreshed. Immutable primary evidence: `local/x87_error_dispatch_primary/`.
