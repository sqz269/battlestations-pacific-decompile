# Native CRT pow fallback support discovery

The fallback is not yet a complete public source provider. Its remaining support is now bounded: two control-word helpers are independently ready; the two error-dispatch entries can directly compose the existing concrete `__87except` provider under that provider's documented runtime contract; the result-status tail depends on those dispatch entries; and the special-value pair depends on the separately sealed classification leaves. **The original BFEB10 SSE2 route remains unimplemented regardless of fallback progress.** No host pow replacement is proposed.

This read-only packet starts at `ddc0bb66`, owns analysis of BFEB10, BFEB6D, C08330, C08347, C08479, C19DC0 and C19E24, and changes only this document and [the report](../reports/native_crt_pow_fallback_support_discovery.json). It preserves Ghidra, source, shared metadata, the installed game and all prior sealed evidence. There was no build, test, runtime, gamma or display operation. The ignored `local/pow_fallback_support/sealed.json` manifest pins 23 fresh Ghidra/PE spans totaling 1,650 distinct bytes plus source contracts. Each live query verified `bsp` and `/battlestationspacific.exe` before reading. Virtual zero-filled PE data is compared as mapped image data, not mistaken for missing file bytes.

## Complete boundaries and current readiness

| Original entry | Complete extent | Current source status / prerequisite |
| --- | --- | --- |
| BFEB10 | BFEB10..BFEB64, 84 bytes | Original dispatcher only; fast path C19260/C19279 remains blocked. |
| BFEB6D | BFEB6D..BFED32, 453 bytes | Full fallback body recovered; public support below is still missing. |
| C08330 | Prefix C08330..C08347, 23 bytes, then C08350..C08383 shared tail, 51 bytes | **74 reached bytes**, not a complete 23-byte function. Binary error-record entry. |
| C08347 | C08347..C08383, 60 bytes | Correct saved library name `__startOneArgErrorHandling`; private camera kernel exists, public complete provider does not. |
| C08479 | C08479..C0851C, 163 bytes | Full result/status tail; calls C08330 or C08347 and reads six concrete constants. |
| C19DC0 | C19DC0..C19E24, 100 bytes | Correct library name `__d_inttype`; directly calls BFA52F and C28548 twice. |
| C19E24 | C19E24..C19F62, 318 bytes | Full special-operand body; directly calls C19DC0. |

C08330 and C08347 physically share C08350..C08383. Their combined physical code is **83 bytes**, not 23+60 bytes of independent implementations or a contiguous 74-byte function. The saved C08330 prefix ends with a jump over C08347's nine-byte prologue. Any source packet must own both entries/shared tail coherently and prove both entry paths. No name inferred from a saved prototype supplies a complete ABI.

Current complete source providers are C083D5 (`__fload_withFB`, packet 4182ea16), C08390/BFED32 (sealed 199f8fdb), and the concrete C27489 `__87except` runtime. The power-leaf source is pinned from its immutable worker because it is absent from this discovery's earlier base. C12F3E/BFA52F/C28548 were independently sealed as **b9743c31** while this discovery ran; their source/API is pinned, with primary review/integration still a separate prerequisite. This packet does not claim or reimplement those three addresses.

## Original fallback entry and stack schedule

BFEB10 takes **ST1=x, ST0=y**, with its original return address at entry ESP. It reads current DWORD 109EEA0; if enabled, it tests `(MXCSR & 1F80h)==1F80h` and `(CW & 007Fh)==007Fh`. These tests do not constrain rounding, x87 precision, DAZ or FTZ. Passing them tail-jumps to C19260, whose complete 25-byte wrapper calls the still-unimplemented C19279 core. The initial mapped zero at 109EEA0 is not a runtime policy: fresh C0AC8F/C0AC9B writer bytes respectively clear it and assign EAX. The previously discovered SSE2/error dependencies remain blocked; their coefficient investigation was not repeated here.

The fallback branch allocates 14h bytes, FXCHs the operands, FSTPs x to an actual stack qword, FSTs y to the next qword while retaining y in ST0, and loads that y high word into EAX before calling BFEB6D. BFEB6D is therefore **not an ordinary `pow(double,double)` entry**: it expects EAX=y high word, ST0=y and this caller-owned stack shape.

Let **T** be ESP just after BFEB6D pushes EAX. T+00 is a packed saved-CW/scratch DWORD; T+04 is BFEB10's internal return address; T+08 is x; T+10 is y; T+18 is the remaining scratch DWORD (offsets hexadecimal). The waiting FSTCW at BFEB70 overwrites only T+00's low word. Unless that word equals 027Fh, C083A5 reads the packed DWORD, retains only precision bits 0300h, ORs 007Fh, writes the new working CW into T+02, and FLDCWs it. The original low saved word remains available for restoration.

The complete C083A5 helper is 23 bytes and already appears only as a private camera kernel. Its call entry expects the packed CW at entry ESP+4 and writes the upper half of that same caller slot at ESP+6; EDX/flags are outputs. The complete C0842E tail is 13 bytes and expects **ESP already pointing at the packed CW, with the real return address beneath it**. It conditionally FLDCWs the low word, POPs the full packed DWORD into EDX, then RETs. It is a JMP/tail entry, not safe for an ordinary CALL that would push another return address. Neither helper has a dependency or needs a new context.

BFEB6D classifies the actual y words, loads the current x through C083D5 and immediately consumes that helper's returned flags. Ordinary positive x uses FYL2X then complete C08390. Negative/zero routes use BFED32's raw CL output and preserve its current-rounding/precision limits. The new BFED32 source borrows E15850 in EDX; a future fallback adapter must deliberately bind that pointer while preserving the native surrounding register/flag schedule, rather than treating the new declaration as an unchanged ABI. Fresh reads of current 109DD78 select normal restoration versus error/status handling. Error dispatch uses actual `"pow\0"` at E15858 and operation 1Dh; E166D0 supplies infinity80 and E15C70 the literal negative NaN80.

All word/byte/dword access widths matter. In particular BFECD3 is `TEST dword ptr [T+17h],80h`, a full four-byte read spanning y's sign byte and three scratch bytes, not a byte test. No operand snapshot, zero-initialization of scratch, or simplified numerical table is justified by the branch mask.

The special-operand route allocates 74h bytes: an eight-byte output cell followed by a 108-byte x87 save area. It pushes the output pointer and two qword arguments, FSTPs x then y, and executes the original **waiting FSAVE** at BFEC7C (bytes begin `9B DD 71 08`). FSAVE also resets the x87 environment, so C19E24 runs under that reset environment when reached from this caller. On normal return the body removes those arguments, FRSTORs the saved environment, FLDs the current output cell and restores ESP. No FH3 guard or exception cleanup surrounds this sequence; a fault/nonlocal exit is not promised an automatic FRSTOR. Neither FSAVE nor this special body saves MXCSR.

## Shared error record and ownership

C08330/C08347 take EAX=error type, ECX=actual name pointer, EDX=operation and the result already in ST0. After their EBP prologue, EBP+08 addresses the caller's saved CW, EBP+10/14 the first argument words, and EBP+18/1C the second argument words. C08330 copies the second argument before entering the shared tail. C08347 leaves that field unwritten. The shared 32-byte stack record is `{int type, char* name, double arg1, double arg2, double result}` at offsets 0,4,8,10h,18h; the result FSTP precedes the name/first-argument stores.

The tail calls fixed **`legacy_crt_87except_00c27489(operation, &record, &savedCW)`** using the original cdecl argument layout. It then FLDs the record's **current** result and rereads the caller's saved CW for the conditional restore. There is no heap allocation, owner callback, generic error policy or lifetime terminal to invent. The stack record and saved slot remain alive through the call; result mutation by the concrete error provider is observed only after normal return. No native unwind handler is installed by these wrappers.

The existing provider explicitly supports binary64 operation **1Dh**, copying the initialized second argument for operations 10h/16h/1Dh. It borrows a persistent `LegacyCrtMathRuntime` containing the actual E16BD0 matherr-bypass cell and owning CRT errno accessor. It performs concrete x87 status generation, continuable Windows `RaiseException`, continuation-result/control mutation, installed return-zero `_matherr` behavior and errno writes. It never directly writes the caller's saved-CW pointer. A direct call to this completed provider is appropriate; the private camera callback slot is unnecessary.

Its evidence boundary remains visible: reserved FPIEEE bytes are initialized rather than native unspecified stack bytes; reserved x87 precision encoding 01 is not contracted; unused binary32 handling is not exposed; prior differential evidence checks x87 exception flags, not every instruction/data pointer or condition-code bit. The current provider requires binding before entry and is not an original TLS/bootstrap reconstruction. Existing coverage includes binary operations, but this discovery did not execute a new operation-1D case. Reusing it does not establish a new full native SEH/FP-state equivalence claim.

## C08479 result/status tail

C08479 enters with the same CW-top stack as C0842E, ST0=current result, ECX=name and EDX=operation. It temporarily spills the result to qword with **FST, not FSTP**, classifies that rounded double's exponent, and retains the x87 result for later scaling/dispatch. Normal exponents test the current saved-CW inexact mask and, when required, a waiting FSTSW's inexact status. The default-CW path skips FLDCW exactly as native. Type 8 dispatch occurs only on the native status/mask branch. Operation 1Dh chooses C08330; other operations choose C08347. Each dispatched return and normal return POPs the packed CW into EDX before RET.

Exponent-zero and exponent-all-ones paths scale the retained x87 value, compare its magnitude, and dispatch types 4 and 3 respectively. They are not host underflow/overflow predicates or scalar clamps. All comparisons, unordered outcomes, temporary rounding, waits and FP side effects must remain instructions. The concrete literal cells are:

| Cell | Original bytes, little endian | Role |
| --- | --- | --- |
| D6A684 | FFFFFFFFFFFFEF7F | Maximum finite double comparison |
| D6A68C | 0000000000001000 | Minimum normal double comparison |
| D6A694 | 00000000000098C0 | -1536 scale exponent |
| D6A69C | 0000000000009840 | +1536 scale exponent |
| D6A6A4 | 000000000000F07F | Positive infinity multiplier |
| D6A6AC | 0000000000000000 | Positive zero multiplier |

A future source context can borrow those six immutable cells in an otherwise unused preserved register, with each original instruction reading only its reached cell. Such a context is an explicit new address-binding contract, not generic arithmetic callbacks or permission to snapshot the result. The two complete error-dispatch providers must exist first.

## C19DC0 and C19E24 support

C19DC0 is a complete cdecl double-slot entry with EAX=0/1/2 and no returned x87 value. It repeatedly reloads the actual incoming double, spills an outgoing double to BFA52F, rejects `AL & 90h`, calls C28548 and compares the returned ST0 with the current input, then multiplies a new input load by actual D7A280=0.5. It spills/reloads that product and calls C28548 again. Both comparisons use FNSTSW/TEST AH,44h/JP; they are not general C++ equality predicates. Parity descriptions inherit the current precision/rounding restriction already identified for BFED32.

The sealed three-leaf packet exposes `native_crt_fpclass_00bfa52f(low,high,readable_tail_word)` and `native_crt_sptype_00c12f3e` with a third public word to cover the original unaligned ten-byte readable span. C19DC0's **internal original call still pushes only eight bytes**; the extra two readable bytes reside in its already allocated stack locals. Inserting a third argument into that native call would change the original schedule. The full raw provider may be called by assembly with this actual readable-stack obligation explicitly recorded. `native_crt_frnd_st0_00c28548(low,high)` returns ST0 and incidental ECX from its scratch; it must likewise be composed by assembly, not a C++ double snapshot.

C19E24 is a cdecl `(double x, double y, double* output)` entry, returning EAX=ESI (initially zero; one on its explicit domain-result path). It rereads raw argument words and the current output pointer at their native sites. It contains x87/integer operations and **no SSE instruction in its entire 318-byte body**. Literal reads are E165A0=positive infinity, E165A8=negative quiet-NaN bytes `000000000000F8FF`, and E165C0=negative zero. Its only direct call is C19DC0. It preserves ESI, balances its own x87 temporaries on normal paths, and writes the output only on selected branches; unsupported paths can return zero with output untouched. A source must not synthesize a default result.

One concrete reason to preserve the literal/control flow is the exact y=-infinity, |x|=1 path: it stores the negative-NaN literal and returns one through C19EC0..C19ECB. The caller turns that result into its native type-1 error route. A modern host power table is not evidence for replacing this branch. No new runtime validation of this special case is claimed.

## Smallest coherent next source packets

| Proposed packet / new file stem | Owned original entries | Readiness and explicit prerequisites |
| --- | --- | --- |
| `native_crt_x87_control2` / `native_crt_x87_control` | C083A5[23], C0842E[13] | **Ready independently.** Exact assembly-only CALL helper and CW-top JMP tail; no context/providers. Best immediate packet. |
| `native_crt_x87_error_dispatch2` / `native_crt_x87_error_dispatch` | C08330 + C08347; shared physical union83 | **Source-ready under existing C27489 contract.** Both raw entry layouts/shared tail together; direct fixed provider, persistent actual LegacyCrtMathRuntime. Prove both entry streams and any declared control-transfer binding. |
| `native_crt_pow_special2` / `native_crt_pow_special` | C19DC0[100], C19E24[318] | Concrete after primary accepts sealed classification3. Borrow actual four literal cells; preserve raw double slots, internal eight-byte call, ST0 results, output nonwrites and current environment. |
| `native_crt_x87_result_status1` / `native_crt_x87_result_status` | C08479[163] | After error-dispatch2; six pinned literal-cell bindings and exact saved-CW tail ABI. |
| `native_crt_pow_fallback1` / `native_crt_pow_fallback` | BFEB6D[453] | After all above and complete loader/power leaves. A concrete fixed context must bind current 109DD78, original name/literals and adapted leaf data; preserve all original stack/register/wait/FSAVE/FRSTOR ordering. Not ready to claim complete today. |

Each proposed source packet would own only its four new cpp/hpp/doc/audit files and explicitly listed addresses, after primary review/lease. These are proposals, not source changes or new leases in this discovery. BFEB10 must remain a separate blocked dispatcher packet until the genuine SSE2 core and its error route are complete; forcing 109EEA0 to zero or calling host pow would silently narrow the original function.


Primary verified all 40 sealed worker files, 58 additional report pins and 23 freshly guarded spans (1,650 bytes), and checked 10 current main source/header inputs. Two control-word helpers are assigned separately; shared error entries, special-operand bodies, result-status tail and full fallback remain subsequent source packets. The genuine SSE2 route still blocks full pow and renderer gamma. No host pow or forced dispatch policy is accepted. Immutable review evidence: `local/pow_fallback_support_discovery_primary/`. No source, original-body execution or gameplay claim is added by this review.
