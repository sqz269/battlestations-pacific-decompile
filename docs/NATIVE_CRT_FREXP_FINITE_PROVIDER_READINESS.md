# Native CRT finite frexp provider: readiness only

This is an independent static review of exactly `LIBCRT_unmatched_00c12f99`
(`00C12F99..00C13047`, 175 bytes / 62 instructions) and `__set_exp`
(`00C12EB0..00C12ED9`, 42 bytes / 16 instructions). The gap is excluded.
`source_ready=false`, Source authorization is false, Source credit is zero, and
Native credit is zero. Integration review is still required. No C++ interface,
implementation, annotation, listing repair, compiler run, test, emulation, native
body execution, or game validation was performed.

The exact base is `2f47be683a8bba8b133c03e7f6d01a28796cd411`, branch
`agent/cc12_crt_frexp_finite_readiness_sol_oct16`. The machine-readable companion
is `reports/native_crt_frexp_finite_provider_readiness.json`.

## Physical evidence

The original PE was physically copied before analysis and hashes to
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Authoritative captures are under
`local/NativeCrtFrexpFiniteProviderReadinessSol02/evidence/controlled/`.
The final decoder comparison and its before-decode module inventory are under
`evidence/controlled_decoder_final/` in that same family.
Every live batch uses the supported read-only CLI, verifies project `bsp`, marker
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 language and
image base, and runs with `BSP_GHIDRA_AUTOSTART=0`.

The controlled whole-body typed captures contain all 78 exact `InstructionDB`
rows at modification 6928, not merely instruction-context text. Both functions
have one complete body range, are returning, and are not thunks. All overrides
are `NONE`; every default/effective flow, fallthrough and target agrees. Disk
decoding covers precisely 217 bytes without gaps or trailing bytes. Every decoded
start, length and mnemonic agrees with the live listing and typed rows; all disk
bytes agree with live bytes. Body SHA-256 values are:

| Body | SHA-256 |
| --- | --- |
| `00C12F99` | `7bbc5dbffc8a51ce9ede39533baefdefc32e58f0a5c229ec93b04b99fffbf4ff` |
| `00C12EB0` | `a96bd4239072bf5565794cce8917077abafd7d0ed2db860514d23aeac84bec3f` |

`00C13012 CALL 00C12EB0` and `00C13027 CALL 00C12EB0` are the only calls.
The setter's full 16-instruction typed/default-flow body has no call or jump;
its fresh prototype has `call_count=0` and empty `callee_names`. The previously
routed setter self-edge is not a native recursion contract. No repair was made.

## Stack and register contract

For either entry, let the entry ESP be S and the established EBP be F=S-4.
The return address is at S, the eight input bytes at S+4 (`F+8`), and the final
four-byte argument at S+12 (`F+0x10`). The finite provider interprets that final
argument as an output pointer; the setter interprets it as a 32-bit integer
exponent adjustment. Both end with a plain `RET`; their caller removes 12
argument bytes. Neither consumes an incoming x87 argument. On ordinary completed
execution each leaves one additional x87 result in ST(0).

Both preserve EBP by save/restore and leave EBX, ESI and EDI untouched. EAX, ECX
and integer condition flags are volatile. The finite provider assigns EDX on
every completed route and finally returns the output pointer in EAX. The setter
does not read, write, save or restore EDX anywhere. Consequently its preservation
of the finite provider's subnormal exponent is established by the complete
instruction body, rather than by an assumed calling convention.

At the setter's return, if e is the unsigned raw exponent argument and
R=`DWORD[F+0x0e]`, ECX is `R & 0xffff800f` and EAX is
`(((e + 0x3fe) mod 2^32) << 4) mod 2^32 | ECX`. Final OR flags have CF=OF=0,
ZF/SF/PF from that 32-bit EAX and undefined AF. The finite zero route preserves
incoming ECX; its final integer flag writer is `XOR EDX,EDX`. The ordinary
nonzero-exponent route's final flag writer is `SUB EDX,0x3fe`; the subnormal
route's is `ADD ESP,0xc`. These flags are not a recovered public result API.

The finite provider has no local allocation. Each setter invocation places a
zero integer argument at F-4 and the eight input bytes at F-12..F-5. The nested
setter frame is P=F-20; its eight local bytes are P-8..P-1 (F-28..F-21).
Thus the ordinary maximum stack displacement is 32 bytes below the finite
provider's entry ESP. The setter itself allocates eight bytes with two ECX
pushes and overwrites all eight with its completed binary64 store.

Three distinct overlaps must remain distinct. The finite provider's raw
`DWORD[F+0x0e]` at `00C1302C` spans the input's high two bytes and its output
pointer's low two bytes; shifting by four and masking `0x7ff` removes all pointer
bits. The setter's equivalent load at `00C12EBB` spans input high two bytes and
integer argument low two bytes. The accepted outer `_frexp` caller instead has
a control token in its private pointer home. None is an eight-byte floating
load, and none licenses aliasing the three frames.

## Finite provider dataflow

`FLDZ` pushes positive zero, and `FCOM m64` compares that zero with the input.
After `FNSTSW AX`, `TEST AH,0x44; JP` takes the nonzero/unordered route. Equal
signed zeros take `XOR EDX,EDX` and retain the loaded positive zero. Therefore
both input zero signs produce **positive zero and exponent zero** on this route.

The other route zeroes ECX and checks the exponent word, high 20 fraction bits
and low 32 bits. A nonzero exponent goes to `00C1301C`. Its zero is popped with
`FSTP ST0`, the original binary64 input is loaded, then stored as a fresh
eight-byte setter argument. The third setter argument is zero. After the setter
returns, the exponent is extracted from the finite provider's original input
and reduced by 1022. For ordinary finite normal inputs with biased exponent b,
the result has the original sign/fraction and biased exponent 1022, with output
exponent `b-1022` (range -1021..1024).

For a nonzero subnormal, `FCOMP m64` both establishes the sign of the comparison
against zero and pops zero. EAX becomes one for a negative input and zero for a
positive input. EDX starts at -1021. Each loop iteration shifts the high input
DWORD left, carries original low-DWORD bit 31 into high bit zero, shifts the low
DWORD left, and decrements EDX. These are writes to the provider's **own eight
argument bytes**, in that order. The loop terminates when bit 52 is set, clears
that bit with a two-byte RMW, and conditionally restores sign with another
two-byte RMW.

For the nonzero unsigned 52-bit fraction F, let h be its highest set bit and
k=52-h. Then 1<=k<=52, the exponent is `-1021-k` (-1022..-1073), and the rewritten
fraction is `(F << k) & (2^52-1)`. The original sign has been shifted out before
restoration. Clearing the first reached bit 52 leaves an exponent-zero
intermediate, possibly signed zero for a power-of-two subnormal. Its subsequent
load/store and the setter produce the normalized fraction. EDX crosses the
setter unchanged. This is a termination/range derivation from the integer
instructions, without executing a loop model.

At `00C13041`, the provider reloads its four-byte pointer, then performs one
four-byte `MOV [EAX],EDX` at `00C13044`, after the x87 result already exists and
before `POP EBP; RET`. It does not read the pointed-to output or check null.
Aliasing is not excluded: the final write may overwrite argument bytes or other
caller storage after their ordinary uses. A pointer that overwrites saved frame
or return storage can affect the following epilogue. Invalid memory or a prior
trap does not guarantee an exponent write or return.

## Setter dataflow and direct special inputs

The setter reads e, loads its input, reads overlapping DWORD R, and stores the
loaded x87 value into its eight local bytes. Only then does it compute EAX/ECX,
overwrite the local high **two** bytes with AX, and reload the eight-byte local
result. Let W be the actual completed binary64 spill, including any conversion
effects. Its final reload input is exactly
`(W & 0x0000ffffffffffff) | (uint64(EAX & 0xffff) << 48)`.
This formula retains the distinction between original argument bits in R and
converted local bits in W. Integer addition and shift wrap at 32 bits. For
arbitrary e there is no range check, and adjustment bit 11 can also set the sign
bit. A floating-point exponent-only setter API would lose this behavior.

For the pair's e=0 calls, the rewritten exponent is 1022. Under completed,
fault-free, masked-exception IA-32 x87 behavior, finite normal/subnormal values
have `0.5 <= abs(result) < 1`. Direct infinity inputs produce signed 0.5 and
exponent 1025. Direct NaNs take the unordered compare route, are converted by the
finite provider's pre-call FLD/FSTP, and then acquire a finite exponent with
output exponent 1025. Preserve the converted payload/sign and quieting history
in that description; this is not a NaN-return contract. In contrast, a direct
setter invocation samples the original high argument word before its local
spill, so its high-word rewrite can replace that spill's quiet bit. The generic
W/EAX formula is the exact static contract even when masks or result classes
differ. The accepted outer caller dispatches exponent-all-ones inputs elsewhere;
these direct-entry cases do not establish whole `_frexp` special-value behavior.

## FPU state and unresolved environment

Neither body initializes/clears/restores x87 state, reads or writes its control
word, or accesses MXCSR. FNSTSW copies status into AX without clearing it. All
floating memory transfers are eight bytes; the setter's later patch is two
bytes. The ordinary paths require one free x87 push slot, reach only one extra
entry at a time, and preserve lower stack entries while returning net +1.
Faulted stack tags, pending exceptions, and exception handling are not converted
into an invented successful-return precondition or an all-input readiness claim.

The pinned [Intel SDM, Volume 1](https://cdrdv2-public.intel.com/843827/253665-sdm-vol-1-dec-24.pdf),
sections 4.8.3.5, 4.9.1.5, 8.5.1, 8.5.2 and 8.5.5, supplies architecture semantics:
ordered NaN comparisons signal invalid; masked signaling NaNs quiet; denormal
loads/comparisons can signal DE. Tiny exact binary64 spills do not signal masked
underflow, but **unmasked underflow can trap even for an exact tiny result**;
an unmasked memory-store underflow leaves memory/TOP unchanged. Thus masking DE
alone does not establish subnormal completion. With valid stack and completed
finite transfers, every value is exactly representable and no rounding or
precision-control choice changes its bits. These are conditional architectural
inferences, not observed machine execution. No OS-wide or hermetic runtime claim
is made.

The accepted `_frexp` readiness establishes an initial original-input FLD before
integer classification, an eight-byte finite argument spill, a nonnull original
output pointer passed from EDI, a binary64 result spill before the control
restore provider, and a final binary64 reload afterward. Its `0x133f/0xffff`
request and saved token are not recovered control-word semantics. This pair
does not identify precision, rounding, masks, pending-status handling, trap
delivery or restoration in `00C138A7`, nor the caller's other provider behavior.
Do not infer these from a host default or suppress trap paths to mark Source
ready. Existing `legacy_crt_math.cpp/.hpp` are read-only interface references;
they do not provide original ABI or Native proof and receive no new credit.

## Provenance and remaining gate

Fresh Sol session `01a144c9-07cf-75c0-83b0-559317761802` is independently verified
by its actual first session record and canonical agent path. The family contains
the actual completed task tool invocation/result records with byte offsets and
hashes, plus the full original-session prefix length/hash. Coordination records
are labeled separately from execution records. No reasoning/full prefix is
copied, missing outputs are not recreated, and the in-flight sealing invocation
is qualified outside the immutable evidence family.

The stopped predecessor's 121 files are retained byte-identically as external,
unsealed provenance; no predecessor body conclusion or execution label is
adopted. Its parent snapshot manifest is
`3e59bae67647e1d60a0ed6cd3ba8d2876acc82c47e47f76c590908dc3f08d780`.
The accepted caller's separately sealed 156-file family is also physically pinned
and verifies against manifest
`a2dfa47ba429645293f763bea3605446a412703bae7245608fcecd62ceef3b16`.
No new caller/callee body or data window was decoded.

Earlier successful fresh captures remain under `evidence/` as preliminary
records: Python sources/binaries and user-site Capstone/pefile were pinned before
analysis, but existing stdlib bytecode/site startup were incompletely inventoried.
The controlled Ghidra rerun uses literal `python.exe -S`, explicit pinned
user-module path, `PYTHONDONTWRITEBYTECODE=1`, an empty redirected cache, and 1,666
physically preserved runtime/startup files verified before each child invocation.
The initial controlled decoder also missed pefile's `ordlookup` dependency in its
pin list and is retained as preliminary decoder evidence. Before the final
decoder pass, all eight current `ordlookup` source/cache files were physically
pinned, expanding runtime closure to 1,674 files. That final pass verifies all
139 loaded Python module origins/hashes and the loaded Capstone library **before
decoding**. Every authoritative helper/tool source is pinned before use. These
repeat passes correct their stated startup/module scope; they do not relabel
earlier invocations or claim OS isolation.

The evidence manifest is `local/NativeCrtFrexpFiniteProviderReadinessSol02/evidence/manifest.json`;
the separate seal receipt is `local/NativeCrtFrexpFiniteProviderReadinessSol02/seal_receipt.json`.
The pair is available for integration review as a bounded static readiness
contract. Source/API authorization and all Source/Native credit remain zero;
the caller's control, classification, special-value and invalid-pointer providers
remain separate gates. Leases remain held for integrator disposition.

## Integrator readiness admission

Primary review authenticated the author's 62 actual pairs and complete Original
prefix, all sealed evidence and the separately unsealed outer inventory. The
invalid first post-seal PowerShell readback is preserved and explicitly rejected;
the corrected terminating whole-byte readback passed. All 217 bytes/78 typed
instructions and physical x87/overlapping-stack behavior were reviewed. The
separately admitted unmodified integrator checker passed both call rows and
both DOC claims with zero failures, unchecked claims or warnings.

This accepts readiness evidence only. Source-ready/authorization and Source/Native
credit remain false/zero. The nested setter is not a second member of the original
seven outer frexp providers; no seven-to-five subtraction follows from this pair.
Overall frexp Source remains blocked. Main's C++ build is Source12 evidence and
does not provide compiled or runtime proof for these readiness-only bodies.
