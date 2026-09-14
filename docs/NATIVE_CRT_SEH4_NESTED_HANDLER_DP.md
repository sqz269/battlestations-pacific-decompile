# Native local-unwind loop and nested handler (DP)

This packet reconstructs the complete `__local_unwind4` at **00C0DBC4..00C0DC53**
(144 bytes, 48 instructions) and its nested handler at **00C0DC54..00C0DC99**
(70 bytes, 21 instructions). The latter was undefined in the saved function
listing. Fresh installed-PE and live bytes agree, including the terminal RET
and the following unchanged C0DC9A function. Source preserves all 69 native
instructions through naked MSVC x86 entries, with real source-symbol operands
for the five handler/call relocations. The fixed E15590 cookie operand remains
the original literal address in the exact A1 absolute-load instruction. MSVC
miscompiled the bare numeric bracket spelling as a B8 immediate; the complete
byte audit rejected it. The five explicit instruction bytes preserve the real
memory read without a new scratch register, stack touch or segment prefix.
No source fragment, callback, substitute checker,
private cookie, or synthetic exception record supplies a missing edge.

The module is `native_crt_seh4_nested_handler`; both mutually dependent entries
are compiled in its one C++ translation unit. A separate metadata-only MASM
object names the actual C++ handler as an external PROC with `.SAFESEH`. It is
an explicit input to the game and static audit image links, so its inclusion
does not depend on archive extraction of an otherwise unused metadata member.
An explicit `/INCLUDE` of the exact verified handler symbol also pulls its real
C++ archive member. A retained intermediate link proved that the metadata object
alone did not extract that member. Current compiled bytes, symbol spelling,
metadata, table membership and compiler input proofs pass in the final report.
The directory-level language setup and object target are immediate; a first
deferred callback queues the image attachment for a second phase, after startup
target creation irrespective of registry entry ordering.

## Original loop entry and frame

Let S be ESP on entry, H the inherited EBP, P the first stack word `[S+4]`, F
the registration pointer `[S+8]`, and T the target scope level `[S+0C]`.
This is the original three-word cdecl stack interface plus an actual inherited
native EBP. Merely calling the declaration from ordinary C++ does not establish
these compiler-frame or FS conditions. All accessed addresses and wrapping
32-bit arithmetic retain their native meaning; the source adds no validation.

After saving EBX, ESI and EDI, the loop reads P/F/T, then pushes H, P, F, T, T,
the actual handler entry and the previous FS:[0]. The resulting nested
registration is N = S - 28h:

| Offset from N | Current value |
| --- | --- |
| +00 | previous FS:[0] |
| +04 | actual nested handler address |
| +08 | initial T, then canonical `[E15590] XOR N` |
| +0C | T |
| +10 | F |
| +14 | P |
| +18 | H |
| +1C / +20 / +24 | saved EDI / ESI / EBX |
| +28 | original continuation |
| +2C / +30 / +34 | original P / F / T argument words |

The fixed canonical cookie is read and XORed with N before FS:[0] is set to N.
This cookie is distinct from the scope-table decoding input `*P`. Each loop
iteration reloads the original F argument, its encoded table at F+8, the
original P argument and current `*P`; it does not cache a previous decoded
scope table. The current level is F+0C. Level FFFFFFFEh exits immediately.
Otherwise T=FFFFFFFEh requests all scopes; a different T stops when the current
level is **unsigned <= T**. The 12-byte record address is formed by the original
LEA arithmetic from `decoded_table + 10h + 12*level`.

The enclosing level at record+0 is loaded and published to F+0C before the
record+4 filter test. A nonzero filter skips cleanup and reloads the loop. For
a zero filter, the loop pushes NLG code 101h, reads cleanup record+8 into EAX,
calls the real NLG entry, sets ECX=1, **reloads current record+8**, and calls the
real inherited-frame cleanup primitive. This reload remains observable when
the descriptor aliases state changed by NLG. CU is the accepted FFD0C3 body:
CALL EAX and RET, using the actual callback and inherited EBP.

The normal epilog restores FS first, adds 18h to ESP, pops EDI/ESI/EBX and
returns. It deliberately skips the saved H word; it does **not** POP EBP.
Normal return preserves the native effects of actual cleanup calls on EBP.
Final arithmetic flags are those of ADD ESP,18h; subsequent POP/RET do not
change them. DF is unchanged by the loop. The canonical failure provider's
reached scalar fill requires DF=0 if a cookie failure occurs.

## Original handler entry and outcomes

At handler entry ESP=S, the four cdecl argument words are the actual OS
EXCEPTION_RECORD pointer at +4, nested registration R at +8, unused CONTEXT
pointer at +0C, and dispatcher-output pointer at +10. Only the record's
32-bit ExceptionFlags at +4 are tested, with mask 6 (UNWINDING=2 and
EXIT_UNWIND=4). No unwind flag returns ContinueSearch=1, retaining TEST flags.
The CONTEXT argument is never dereferenced.

On an unwind flag, EAX is R, ECX becomes `[R+8] XOR R`, and the real canonical
checker is called with **no additional stack arguments**. On equality the
checker preserves EAX=R and every register, retaining CMP flags through its
F3 C3 return. The old CT context-argument checker cannot bind this edge. A
failure goes to the real canonical reporter. If its real termination service
returns, the original continuation uses the actual returned EAX; the handler
does not invent a no-return prune or force EAX back to R.

The handler saves its incoming EBP, loads H from current `[EAX+18h]`, pushes
T/F/P from +0C/+10/+14, and invokes the actual mutually dependent loop. It then
adds 0Ch, restores incoming EBP, reloads R and the dispatcher-output pointer
from their current original stack slots, writes R to the output and returns
CollidedUnwind=3. ADD ESP,0Ch supplies final arithmetic flags. Normal paths
preserve the native nonvolatile-register schedule. No source exception catch,
fault repair, frame rollback, or abnormal-return policy has been added.

## Caller and provider boundary

The handler is published by the PUSH at C0DBD8, rather than by an ordinary
direct CALL. The original image SafeSEH table at D7CD50 contains its RVA 80DC54
at index 4 / cell D7CD60. Direct loop callers retained in current xrefs are
BF8354 (`__fsopen`), C04D45 (`__wfsopen`), C0DCAA (C0DC9A wrapper), C0DD0B
(`_EH4_LocalUnwind`) and C0DC81 (this handler). The wrappers remain unchanged.
C0DC9A loads inherited EBP through its supplied record, pushes fields +1C,
+18,+28, calls the loop, restores EBP and RET4. C0DD00 loads EBP from its first
stack word, pushes EDX, ECX and its second stack word, calls the loop, restores
EBP and RET8. Parent I/O bodies are not reconstructed by this packet.

Actual source dependencies are DJ `notify_native_crt_nlg_00c16879`, published
CU `call_native_crt_cleanup_00c16898`, and DK
`check_native_crt_canonical_cookie_00bfe120` with its full real failure closure.
The loop and handler have four direct call sites in total and no indirect
instruction of their own; CU's actual CALL EAX remains an external indirect
cleanup contract. The canonical DD owner supplies the process-lifetime cookie,
NLG descriptor and reporter storage through its accepted real bootstrap.
This packet creates no second data owner and does not bypass bootstrap readiness.
The reached checker-to-GS-reporter edge does not call Watson or memset. Their
accepted sources and current unique archive objects remain available, but they
are not misreported as reached dependencies of this handler.

The current SDK EXCEPTION_RECORD field offset, flag values and disposition
values are compile-time checked. Their type names describe the stack words;
they do not create an OS dispatcher, valid native scope table or compiler frame.

## SafeSEH and saved analysis

The initial rebuilt root image has a real SafeSEH table but no implementation
of this new handler. Merely retaining exact handler bytes in a library cannot
establish admission by that image's exception dispatcher. The packet therefore
supplies a metadata-only MASM object explicitly to each supported image link.
Microsoft documents `.SAFESEH` for a local or external PROC when assembled with
`ml /safeseh`; actual current compiler inputs and linked table membership must
still be checked. [MASM .SAFESEH documentation](https://learn.microsoft.com/en-us/cpp/assembler/masm/dot-safeseh?view=msvc-170).

Existing image-base, relocation and mitigation policies are preserved. No
`/SAFESEH:NO`, loader workaround, exception shim or exception execution is used.
The first strict build emitted C4733 for both deliberate FS:[0] writes. Only the
complete loop is surrounded by warning push/disable:4733/pop, with the explicit
metadata requirement stated next to the source. The handler and other code keep
their diagnostics, and /W4 /WX remain enabled. This suppression is contingent on
the actual linked SafeSEH proof, not a claim that exact bytes alone admit a handler.
Table membership proves a registration artifact in that image, not correct
runtime frames, OS dispatch, native scope consumers or gameplay behavior.

The bounded C0DC54 disassembly and function definition used the shared Ghidra
write lock after full PE/live equality and adjacent-function checks. All prior
comments and function information were retained. The provisional descriptive
name is `BSP_CrtSeh4NestedLocalUnwindHandler`; the existing `__local_unwind4`
library name is preserved. Both annotations were saved, read back and exported;
the changed function inventory was snapshotted and indexed. No caller prototype,
no-return property, flow override or adjacent function was changed. The supported
flow-properties query could not run because inline bridge scripts are disabled;
the packet retains that rejection and does not change bridge settings. Exact
flow-property metadata remains unknown, separately from fully decoded bytes.

## Evidence and limits

The machine-readable report pins the original PE, fresh spans, all 69 source
instruction markers, prior CP/CZ/DB evidence, real provider revisions, Ghidra
before/after/export receipts, compiler inputs and outputs, archive membership,
linked image bytes and SafeSEH table membership. The clean full build (03), final
registration relinks (04/05), eight seed comparisons and both existing CTests
pass. Both compiled functions have exactly their native lengths, with all 214
bytes identical outside the five real relocation operands. Nine source/provider
objects have unique current archive membership; the image audit checks 581 rows,
64,487 code bytes and 2,496 resolved relocations with zero unresolved operands.
The owned C++ object has no additional writable section or data definition.

The final game table at 101E4AA8 has 932 entries and contains the actual handler
RVA A6460 / VA 100A6460 at index 0. The never-executed forced image table at
10007160 has ten entries, including RVA 1020 / VA 10001020 at index 0. Both
images have the required HIGHLOW relocation for the compiled handler pointer;
the canonical-cookie address operand has no image-base relocation. The full
MASM diagnostic invocation records `/safeseh /WX`, actual source and object paths;
normal MASM tlogs supply read/write records but no command record. Existing
quoted/case-normalized linker tracking values are checked against the exact
case-sensitive generated symbol, current COFF and linked MAP.

All failed attempts are retained with their corrections. No source or link
acceptance is based on the earlier build success alone.
The rejected B8 object, archive, game, MAP, sources and compiler logs are retained.
Because the initial failed compile left an aggregate write record missing its
output, the final accepted compilation is a clean full build with newly verified
current command/read/write records; the stale record is not silently discarded.

No original handler, cleanup, unwind, cookie-failure, NLG, termination or game
path is executed for this packet. Existing build tests do not exercise these
entries. Source, compiler bytes, current provider binding and image registration
are distinct from native caller/frame adoption and runtime or gameplay validation.
