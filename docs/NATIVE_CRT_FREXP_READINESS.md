# Native CRT `_frexp` readiness

Packet `cc12_native_crt_frexp_readiness_oct16` is a whole-entry **readiness-only, blocked** review of `00BFEF3B`. `Source_ready=false`, `Source_authorized=false`, and Source credit is **0**. No C++ implementation, compiler/build, tests, probes, emulation, original-body execution, SDK comparison, or Ghidra mutation occurred. This is static body/ABI evidence, not ABI compatibility or game validation.

The isolated branch starts at `67cbe3623909f1d53decad0d929a617842107e07`. Live typed metadata identifies one complete inclusive body, `00BFEF3B-00BFF014`: **218 bytes, 86 instructions, 8 direct call sites, 7 distinct callees**. All 86 exact instruction starts were checked: contiguous coverage, no flow override or fallthrough override, no computed branch, and unchanged default/effective flow and targets at Ghidra modification 6658. The nine direct jump targets stay in this body. The endpoint, rather than the adjacent entry, establishes the extent. The function and direct targets have `no_return=false` in current metadata; this does not establish that providers always return.

Every analysis batch uses the supported read-only BSP commands, with `BSP_GHIDRA_AUTOSTART=0`. Their client verifies project `bsp`, program `/battlestationspacific.exe`, x86 little-endian 32-bit language, and image base `00400000`. Typed metadata additionally identifies `C:/Users/sqz269/bsp.gpr`. The current project descriptor is pinned; it is not a snapshot of all Ghidra database contents.

The whole disk PE was read and preserved before discovery, SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. The separately selected 218-byte disk window matches the live Ghidra bytes, SHA-256 `c97625a587467103232361a0ff29245132e570ea35005fb102001ddf6ea2d3e2`. A full PE hash/copy does not mean provider bodies or gaps were decoded. Only this body's pseudocode/listing and two leased eight-byte data windows were analyzed. Caller `math_frexp` at `00A61AE0` was inspected only as a label/prototype.

## Physical ABI and memory order

The recovered library prototype is `double __cdecl _frexp(double X, int* exponent)`. At entry ESP=S: return address is `[S]`, the eight-byte input is `[S+4..S+11]`, and the four-byte output pointer is `[S+12..S+15]`. After the frame prologue these are `[EBP+8]` and `[EBP+10h]`. Return uses x87 ST(0), with a plain `RET`; the caller removes the 12 argument bytes. EBP, EBX, and EDI are saved/restored; ESI is saved/restored only on the non-null branch. EAX and ECX are used as volatile registers. No direct EDX use is present; provider clobbers, incoming x87 stack requirements, and exceptional unwinding are not proved by this body.

EDI retains the original pointer. The body subsequently overwrites the four-byte incoming pointer slot with EAX returned by `__ctrlfp`; this is a captured control token, whose interpretation as a saved control word remains a provider contract. At `00BFEF81`, the actual read is **four bytes from `[EBP+0Eh]`**, spanning the input's top two bytes and the newly written token's bottom two bytes. Only AX is masked with `7FF0h` and compared with `7FF0h`. Replacing this with a two-byte load would change the physical access contract even though the extra loaded bits do not affect that branch.

The input is first loaded by `FLD m64fp` at `00BFEF7B`, before that integer exponent check and before any exponent-output access. Thus the classifier branch is a raw exponent-field check, but its later floating argument has already crossed the x87 load/store path. All argument and result payload memory accesses below are **eight-byte binary64**; no ten-byte memory payload load/store occurs. x87 register precision and conversions remain environment-dependent.

| Path | Exact local sequence and boundary |
| --- | --- |
| Null output pointer | No input FLD and no local control-provider invocation. The pointer provider returns EAX; five four-byte zero arguments are pushed; a four-byte store writes `16h` through EAX; the second provider is entered. Only if it returns does `FLDZ` produce ST(0), followed by removal of 20 argument bytes and the common return. No exponent output is dereferenced. Provider pointer validity, errno/TLS meaning, callback behavior, and termination are unresolved. |
| All exponent bits not one | Push original output pointer, allocate eight bytes with two four-byte ECX pushes, and FSTP the previously loaded input into that argument. The finite helper receives binary64 then pointer. Its ST(0) result is FSTP-stored to the eight-byte local `[EBP-8]` **before** the second control-provider invocation. The body passes the captured token and `FFFFh`, then FLD-reloads the local **after** that invocation. It removes 20 outstanding argument bytes. The helper owns all output-pointer writes and normalization behavior. |
| All exponent bits one | At `00BFEF90`, `OR dword ptr [EDI],FFFFFFFFh` performs a **four-byte read/modify/write** setting exponent to -1, before the FSTP of the classifier argument and before the classifier provider. It has no LOCK prefix. This is not merely a write-only store. The classifier receives the already loaded/stored binary64 input. Its EAX return is compared as signed 32-bit. |
| Classifier return 1 or 2 | FLD the eight-byte data window at `00E165A8`, then take the common exception-provider path. |
| Classifier return 3 | Push captured token; FLD the original eight-byte input again; allocate eight argument bytes and FSTP the reloaded input; push `17h`; enter the NaN-named provider. Remove 16 bytes on return and return its ST(0) without a local result spill or local control restoration. |
| Classifier return <=0 or >=4 | FLD the original input again and FADD the eight-byte window at `00D7A210`, then take the common exception-provider path. |
| Common exception-provider path | Push token, reserve 16 bytes, FSTP the prospective result into the upper eight bytes, then FLD the original input again and FSTP it into the lower eight bytes. Push `17h`, then `8`; enter the provider, remove 28 bytes, and return its ST(0) directly. No local control restoration follows. |

The input reloads are real: they must not silently become reuse of an earlier host value. This packet imposes no invented non-aliasing rule. A non-null output pointer could overlap input, saved-token, or other accessible storage; the early load, raw overlapping dword read, RMW, provider accesses, and later reloads must keep their observed order. Legal addressability and any original caller restrictions remain external obligations. The helper prototypes are `undefined ... (void)` in Ghidra and do not establish a callable C++ ABI; the table derives outgoing layouts from the selected body's pushes/stores.

## Classification and floating-point environment

The exact local bit predicate is `((original_input_bits >> 48) & 0x7FF0) == 0x7FF0`, evaluated through the four-byte read described above. It is insensitive to the sign bit and does not inspect fraction bits. Therefore normal values, subnormals, +0, and -0 all take the finite-provider path. Infinity and every NaN encoding take the special-provider path. The original input is still subject to the preceding x87 FLD before that decision.

This body does **not** establish normal/subnormal normalization, exponent values for zero, signed-zero preservation, or finite-result bit identity. It also does not establish which classifier return codes correspond to either infinity sign, quiet NaNs, or signaling NaNs. In particular, the `__handle_qnan1` name is not evidence that code 3 identifies the original unmodified NaN encoding: its classifier argument has passed through FLD/FSTP. Signaling-NaN quieting, payload propagation, sign preservation, and raised/status exceptions require actual provider and FP-environment contracts. No finite-only projection is offered as the whole function.

The only two referenced data windows are:

| Window | Current exact bytes and bits | Qualification |
| --- | --- | --- |
| `00E165A8`, length 8 | `00 00 00 00 00 00 F8 FF`; bits `FFF8000000000000` | Binary64 negative quiet-NaN encoding, selected when classifier EAX is 1 or 2. Located in disk `.data`; matching current Ghidra/disk bytes do not establish runtime immutability. No recovered symbol identity or neighboring ownership is asserted. |
| `00D7A210`, length 8 | `00 00 00 00 00 00 F0 3F`; bits `3FF0000000000000` | Binary64 +1 encoding used by FADD in the fallback path. Located in disk `.rdata`. Only this precise window was read; global alias/symbol uncertainty remains. |

Both data windows were leased before targeted reads and independently match the pre-discovery pinned PE. Their hashes are recorded in the JSON report and `static_review.json`.

The first control-provider boundary passes literal `(133Fh, FFFFh)` and records its EAX result. The finite path later passes `(captured_token, FFFFh)`. The body contains no direct control-word/status-word save, restore, clear, initialization, or SSE/MXCSR instruction. Actual mask mapping, precision, rounding mode, pending exception behavior, trap delivery, status preservation, and any SSE environment coupling therefore remain owned by that provider and the active thread. The literal values alone do not prove the resulting environment.

The local FLD, FSTP, FADD, and FLDZ operations can have environment-sensitive effects. The finite result is reduced to an eight-byte memory value before restoration and reloaded afterward; special paths return the helper's ST(0) without that local reduction. The special helpers receive the captured token and own any restoration on those paths; receipt of the token does not prove restoration occurs. The null path never changes controls locally and delegates handler behavior. Neither runtime FP state nor trap behavior was measured.

## Exact provider boundaries

These are eight actual selected-body call sites. Parameter layouts below are observed outgoing stack layouts, not accepted provider source signatures. Offsets are relative to ESP at provider entry, where `[ESP]` is the return address.

| Listing | Current name | Observed boundary |
| --- | --- | --- |
| `00BFEF4B CALL 00BFFB8B` | `LIBCRT_unmatched_00bffb8b` | No explicit stack arguments; EAX used as pointer to a four-byte writable cell. |
| `00BFEF5B CALL 00BF66EF` | `LIBCRT_unmatched_00bf66ef` | Five four-byte zeros at +4 through +20; locally falls through to FLDZ. |
| `00BFEF76 CALL 00C138A7` | `__ctrlfp` | +4=`133Fh`, +8=`FFFFh`; EAX captured. |
| `00BFEF98 CALL 00C12F3E` | `__sptype` | +4=8-byte converted input; signed EAX classification code. |
| `00BFEFBA CALL 00C13658` | `__handle_qnan1` | +4=`17h`, +8=8-byte reloaded/stored input, +16=captured token; ST(0) returned directly. |
| `00BFEFE9 CALL 00C1370A` | `__except1` | +4=`8`, +8=`17h`, +12=8-byte reloaded/stored input, +20=8-byte prospective result, +28=captured token; ST(0) returned directly. |
| `00BFEFF9 CALL 00C12F99` | `LIBCRT_unmatched_00c12f99` | +4=8-byte converted input, +12=original output pointer; ST(0) consumed by FSTP m64. |
| `00BFF005 CALL 00C138A7` | `__ctrlfp` | +4=captured token, +8=`FFFFh`; prior finite-helper arguments are still below these until cleanup. |

Every provider is **named but contract-incomplete for this packet**. Only metadata/prototypes were requested for dependencies; no provider, caller, or SCC body was expanded. The missing whole contract is concrete: finite helper behavior and exact ABI; classifier code mapping after x87 conversion; control/status/rounding/trap ownership; NaN and exception-provider result/restore/error semantics; and null-pointer error-cell/handler behavior. Metadata names and successful report-call verification cannot replace those contracts. Resolving them requires separately authorized, leased packets and review before any Source proposal.

## Evidence and validation boundary

The task-only evidence root is `local/NativeCrtFrexpReadiness01/evidence/`; it contains pre-discovery tool/config/project/PE byte pins, actual subprocess argv/cwd/selected environment and stdout/stderr/result records, live typed responses, exact disk windows, and static coverage/byte comparisons. Each successful CLI analysis invocation pins current physical tool files before its subprocess. Initial interpreter pinning was late: the first discovery/body/pseudocode subprocesses preceded the interpreter pin. The disk-window helper's first pin also followed its first extraction. Both qualifications are retained; neither is described as a pre-use pin.

One outer assembly attempt failed during pin preparation because the installed Python executable is hard-linked. It never launched the assembly subprocess. The authentic outer failure is retained in the raw invocation/result family; no inner completion timestamp, missing stdout, or stderr was reconstructed. Subsequent capture records preserve the original input link count and write ordinary independent copies. All final evidence-family members are read-only regular single-link files, without symlinks/reparse points. `manifest.json` excludes only itself.

Raw tool invocation/result records are extracted only from this worker's Original session, authenticated by byte offsets and hashes against a hash of the entire actual session prefix. The prefix itself, conversation messages, and reasoning are not copied. Completed matching task tool pairs, including the failed preparation, are retained verbatim. The sealing invocation is in flight when it captures its prefix; its later receipt is separate and does not claim a self-contained completed pair within the sealed family.

`tools/verify_report_calls.py` is required for both this Markdown and the JSON report, with the eight exact call sites represented in each. Actual results are retained as `verify.stdout.txt` / `verify.result.json`; zero rows would not establish semantics, and a zero exit from a prototype query alone would not disprove an error response. The positive signatures, typed responses, bytes, and listing coverage are separately inspected. No build, fixture, ABI execution, or game validation credit follows from these static checks. The family hash/count and final branch commit are supplied in the handoff receipt; the two tracked outputs themselves are pinned before sealing.
