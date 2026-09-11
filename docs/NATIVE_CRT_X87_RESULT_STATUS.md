# Native CRT x87 result-status tail

This packet reconstructs the complete C08479..C0851C **163-byte** result-status tail. Its new source name is descriptive; the current unmatched CRT entry is not assigned an invented library symbol. The body retains every original instruction, with only two direct CALL relocations and six same-length literal-address operand adaptations. The source is [native_crt_x87_result_status.cpp](../src/native_crt_x87_result_status.cpp), the assembly-only contract is [native_crt_x87_result_status.hpp](../include/bsp/native_crt_x87_result_status.hpp), and [the audit](../reports/native_crt_x87_result_status_audit.json) pins the evidence.

## Entry and stack ownership

This is a **CW-top JMP/tail entry**, not an ordinary CALL target. The caller establishes ST0=current result, ECX=actual name, EDX=operation and EBX=the borrowed literal-region pointer. At entry ESP holds the actual four-byte packed saved-control word, ESP+4 the real return, ESP+8 the current first argument qword, and ESP+10h the second qword when operation1Dh selects binary dispatch. The public void declaration supplies an assembly name; an ordinary C++ call would put a return in the wrong slot and is unsafe. There is no new CALL wrapper or argument adapter.

One additional free x87 slot is required. The original temporary `SUB ESP,8; FST qword [ESP]` rounds to double **without popping ST0**. Its high DWORD is read before the temporary is removed, and the resulting exponent classification controls the original branches. The retained extended x87 value, not that double temporary, continues into scaling or dispatch.

For normal exponents, the current low saved-CW word is compared to 027Fh. That default path skips FLDCW. Otherwise the saved inexact-mask bit and, when required, the current waiting FSTSW status determine whether to dispatch type8 or reload the current saved CW. The later FLDCW is a distinct current memory read; no cached word replaces it.

Exponent-zero and exponent-all-ones paths execute their original FLD/FXCH/FSCALE/FSTP, duplicate/absolute-value comparison, waiting status read and conditional multiplication. They set type4 and type3 respectively before the common operation test. These are literal x87 instruction paths, not host underflow/overflow predicates or clamps. All three waiting status sequences remain the exact `9B DF E0` bytes at C084A4, C084E0 and C08509. Source spells each as FWAIT plus FNSTSW AX so the wait boundary stays explicit. The fresh raw decode has no missing bytes despite formatting gaps in the saved listing.

Operation1Dh calls the completed binary dispatcher; all other operations take the original unary branch. A CALL adds precisely the dispatcher return above the current CW, satisfying that provider's existing raw stack shape. On every normal exit the tail POPs the **current entire packed DWORD into EDX**, then RET consumes the actual return. Outer argument cleanup remains the caller's job. All three normal exits and their stack consumption are checked by the complete CFG proof. Following either dispatch return, only POP EDX/RET executes; no literal is read afterward.

## Literal and provider binding

`NativeCrtX87ResultStatusContext` describes one four-byte borrowed pointer. The assembly caller loads EBX; the body never reads the aggregate or introduces a global. It borrows actual immutable D6A684..D6A6B3 storage, preserving all six original qwords and their relative layout:

| Offset | Original cell | Little-endian bytes | Role |
| --- | --- | --- | --- |
| +00h | D6A684 | FFFFFFFFFFFFEF7F | Maximum finite comparison |
| +08h | D6A68C | 0000000000001000 | Minimum normal comparison |
| +10h | D6A694 | 00000000000098C0 | -1536 scale exponent |
| +18h | D6A69C | 0000000000009840 | +1536 scale exponent |
| +20h | D6A6A4 | 000000000000F07F | Positive infinity multiplier |
| +28h | D6A6AC | 0000000000000000 | Positive zero multiplier |

Each reached read is eight bytes. Each `_emit` block is one independently decoded six-byte FLD, FCOMP or FMUL, changing only absolute addressing to `[EBX+disp32(offset)]`. All branch lengths and instruction positions remain unchanged. No snapshot, synthesized table, entry-time validation or new arithmetic policy is added. The storage must remain valid for every reached read.

The two fixed providers are `dispatch_native_crt_binary_error_00c08330` and `dispatch_native_crt_unary_error_00c08347`. The full shared 83-byte body, its binary74/unary60 reached paths and the explicit five-byte unary JMP thunk retain their established source contract. The thunk targets the shared body+17h and adds no stack/register/flags/FP effects. The binary record's second argument is initialized before result FSTP; the unary record's second argument remains unwritten. Operation/record validity remains part of that provider's caller contract. The dispatchers reload the current record result into ST0 and the current saved CW only after normal provider return.

Before a reached dispatch, bind a persistent actual `LegacyCrtMathRuntime` through the existing API, borrowing the actual E16BD0 cell and the owning CRT errno accessor. The direct complete C27489 source provides the existing concrete Win32/CRT behavior; there is no generic callback or host-pow replacement. Its reserved FPIEEE-byte initialization, reserved precision01 exclusion, FP-status coverage and actual runtime/SEH boundaries remain. Incidental original EAX/ECX equality through this C++ provider is not promised; EDX is subsequently overwritten by the original packed-word POP.

The raw tail, shared dispatcher and unary thunk do not write EBX. The concrete provider's actual compiled prologue saves EBX and establishes its frame with EBX; both normal epilogs restore ESP from EBX and POP the saved EBX before RET. Preservation through its underlying Win32/CRT callees relies on their established ABI. This is a bounded actual-provider contract, not a claim that an arbitrary callback may clobber nonvolatile registers or that C++ provider code matches original CRT bytes.

Current x87 rounding, precision, status, tags and exception masks apply. No EH handler or new unwind cleanup is installed. Faults/nonlocal exits are not promised CW restoration, packed-word consumption or return. The enclosing fallback's FP save/restore policy is outside this entry.

## Verification and limits

The isolated worker begins at main `8be28943`. Five fresh guarded live/PE spans total550 bytes: owned163, literals48, shared-dispatch83, original concrete-provider252 and actual global4. Every Ghidra batch verifies `bsp`, `/battlestationspacific.exe`, language and image base before reading. The complete installed PE hash and every span are pinned.

The strict Release Win32 build includes the new tail and the existing not-yet-registered error-dispatch source through an ignored CMake hook in the actual `bsp_core` target. The concrete legacy CRT provider is already registered. `/W4 /WX /fp:strict`, both existing CTests and all eight fresh seed checks pass. No provider is built separately and no permanent or new runtime suite is added.

The proof freezes the sole actual library and exact `native_crt_x87_result_status.obj`, `native_crt_x87_error_dispatch.obj` and `legacy_crt_math.obj` archive members. All their sections, symbols and relocations are recorded. The entire owned163 and shared83 original bytes match after only their declared relocations/address adaptations. The unary thunk is checked separately, including its +17h addend; the whole concrete C27489 section and normal EBX save/restore instructions are recorded without an original-byte equality claim.

The proof exhaustively accounts for all six original and rebound literal reads, independently decodes each changed instruction, checks every unchanged instruction and branch, verifies the FST nonpop and three waits, and traverses every normal return. The ignored `local/x87_result_status/sealed.json` pins the immutable original, source, build, compiler-command and full object evidence.

This establishes complete bounded source/object reconstruction and repository build checks. It does not establish linked-runtime, new hardware-exception, arbitrary original caller ABI or game validation. BFEB6D, public pow, its actual state-dependent SSE2 route and renderer gamma remain separate; no dispatch state is forced to zero. Shared CMake, ledgers, Ghidra, installed game files and prior sealed artifacts are unchanged.
