# Watson, cookie check and shared failure proposal CO

The complete BF65BB[252], BFE120[15] and C185A4[260] bodies are ready for a
**qualified borrowed-state source packet**, subject to review of the new capture
and binding ABI below. They are not a ready native cookie-check provider for
unchanged OS/compiler frames. Canonical process storage and startup ownership
remain external. No implementation, build, test or execution is included here.

Base `5c80806cc011d29830cde3a87ff46c7dd5bd5c26`. The original BL discovery
`ed185cb2dfdf1ba045dbf30ebfd47f083eeece80` is preserved: all135 local artifacts
were checked twice with SHA256/SHA512 and copied as provenance. Fresh complete
527 PE/live bytes equal BL, independently decode to122 instructions, and have
six verified direct/tail rows plus ten separately resolved IAT calls. All code
is covered, including both returned-termination tails and BFE128's F3C3 return
(rendered merely RET by the listing). Correct CRT library names are retained.
One supported export of the three owned entries populated shared BSP exports;
no analysis, name, prototype, flow, setting or comment was changed.

The worktree index overstates the callees: encoder/decoder edges at BF65BB and
fclose/free/SEH edges at C185A4 are absent from the complete physical bodies.
Fresh live callee lists agree with the actual instructions. This is a stale
indexed-graph qualification, not grounds to expand the source dependencies.

## Complete original contracts

BF65BB `__invoke_watson` is cdecl with five diagnostic arguments, all unread.
Let S be entry ESP. PUSH EBP; LEA EBP,[ESP-2A8h] gives EBP=S-2ACh; SUB ESP,328h
reserves through S-32Ch. The saved ESI is at S-330h. Local EXCEPTION_RECORD is
S-32Ch[50h], EXCEPTION_POINTERS is S-2DCh[8], CONTEXT is S-2D4h[2CCh], cookie
guard is S-8[4], saved caller EBP is S-4, and the caller return word is at S.
Only the exception record is cleared. The CONTEXT's untouched fields and upper
16 bits of its six segment DWORDs remain uninitialized stack bytes.

BF65D7..BF6611 capture, in order, EAX=cookie XOR adjusted EBP, incoming
ECX/EDX/EBX/ESI/EDI, low16 SS/CS/DS/ES/FS/GS, then EFLAGS after the cookie XOR.
No incoming arithmetic flags are captured. EIP becomes [S], ESP becomes S,
ContextFlags becomes10001h, and EBP becomes [S-4]. BF6647 calls memset(record,
0,50h). The local pair is published record first, context second; record code
becomes C000000Dh and exception address becomes the captured return PC.

BF6665 IsDebuggerPresent is captured in ESI. SetUnhandledExceptionFilter(NULL)
precedes UnhandledExceptionFilter(&local_pair); the previous filter is discarded
and never restored. Only filter result0 AND captured debugger result0 cause
C04EF3(2). BF668F pushes C000000Dh, calls GetCurrentProcess, pushes its result,
then calls TerminateProcess. If services return, BF66A1 loads the saved guard,
XORs adjusted EBP, restores ESI and calls the real BFE120. BF66AF ADD EBP,2A8h;
LEAVE; RET remain reachable if that call returns. No added abort/fastfail,
exception translation, errno access, TLS lookup or cleanup frame is justified.

BFE120 `__security_check_cookie` receives the cookie in ECX, with no stack
arguments. CMP ECX,[E15590] preserves all registers; equality uses F3C3 RET,
inequality tail-jumps C185A4 with the same return word and raw registers. It
does not decode a pointer or repair the cookie. The compare's flags survive
the equal return. The mismatch reporter establishes its own SUB flags instead.

C185A4 `___report_gsfailure` has an ordinary return word and no native stacked
arguments. PUSH EBP; MOV EBP,ESP; SUB ESP,328h yields EBP=S-4. It writes the
shared CONTEXT in the same register/segment order as Watson, but EAX is the
incoming EAX and EFLAGS are after this SUB. Shared EBP=[S-4], EIP=[S], ESP=S+4.
C18619 reads [EBP-320h] even though its value is unused; preserve that read.
It stores ContextFlags10001h, reloads current shared EIP into exception address,
then stores code C0000409h and flags1. Current cookie then complement are copied
to [EBP-328h] and [EBP-324h], respectively. Neither shared object is cleared.

IsDebuggerPresent is written to shared109E5B8. C04EF3(1) runs before clearing
the exception filter. UnhandledExceptionFilter receives the address of the
actual fixed pair D6E1CC; its return is ignored. The current debugger word is
reloaded after that call and, if zero, C04EF3(1) runs again. Termination uses
GetCurrentProcess and C0000409h. C186A6 LEAVE; RET are retained if it returns.
In particular, do not annotate proposed declarations noreturn or prune this
continuation based on current Ghidra metadata.

## Actual storage and providers

| Native storage | Initial image / required identity | Stores in these bodies |
|---|---|---|
| E15590[4], E15594[4] | BB40E64E / 44BF19B1, same CRT cookie domain | Read only here; existing C1815E publishes cookie then complement |
| 109E568[50h] | Canonical shared exception record, image zero-fill | C185A4 writes DWORD +Ch address, +0 code, +4 flags, in that order |
| 109E5B8[4] | Canonical shared debugger word, image zero-fill | API EAX store; later current-word reload |
| 109E5BC[4] | Padding, image zero-fill | Untouched |
| 109E5C0[2CCh] | Canonical shared x86 CONTEXT, image zero-fill | Partial widths below; prior/current untouched bytes survive |
| D6E1CC[8] | Exact initial DWORD pair `{109E568,109E5C0}` | Not written here; API consumes the actual pair storage |
| 109EEA8[4] | Actual hook word | Existing C04EF3 performs AND0 RMW |
| 109EEA4[4] | Actual vector feature word; owner external | No cell read on Watson's 50h memset call |

CONTEXT offset/width writes are: B0/4 EAX, AC/4 ECX, A8/4 EDX, A4/4 EBX,
A0/4 ESI, 9C/4 EDI, C8/2 SS, BC/2 CS, 98/2 DS, 94/2 ES, 90/2 FS, 8C/2 GS,
C0/4 EFLAGS, B4/4 EBP, B8/4 EIP, C4/4 ESP and00/4 ContextFlags. The two bodies
have the different later-field order recorded above and in the full mappings.
The x86 SDK layout and actual KERNEL32 import names/signatures were checked;
TerminateProcess returns BOOL in the SDK. All five APIs use real stdcall imports.
There is no injected termination/filter/debugger service and no owned FS chain.

Existing `native_crt_cookie_initialization` source supplies C1815E and the real
two-argument C04EF3 reason/actual-word helper, but explicitly creates no canonical
cookie state. CL `cd18b804f30eba3d76477213276ccb21836e37de` supplies the complete
four-argument `fill_native_crt_bytes_00bf79f0(destination,fill,count,actual_feature)`.
All134 frozen CL artifacts were checked twice, without modification. Its feature
cell is read only for zero low fill byte and count>=100h; 50h bypasses that read.

C04F67 is already complete in `src/native_crt_libm_callback_registration.cpp`.
Its new second context argument borrows current E15B00/E15AFC and the actual
CE20BC TlsGetValue IAT word. It pushes current E15B00 before capturing that IAT,
reuses the captured import, calls the second returned raw getter unchecked,
reads actual PTD+1F8, and uses real KERNEL32.DLL/EncodePointer module fallback.
A selected encoder writes its result into its actual first caller argument
slot; every path reloads that slot. The decoder analog uses PTD+1FC and actual
DecodePointer with the same observable argument writeback. Their C04EFB gate
has qualified owning CRT/errno/invalid-parameter service dependencies. Those
TLS/PTD services are not called by these three physical Watson bodies and are
not substitutes for their cookie or storage domain. BF66EF/BFFB8B/PTD remain
separate callers/frontiers, as the preserved BL evidence records.

## Proposed complete source packet and qualifications

Propose `src/native_crt_watson_failure.cpp` and
`include/bsp/native_crt_watson_failure.hpp`, implementing all three full bodies
in one TU, with real CL/hook dependencies. A stable `NativeCrtWatsonBindings`
borrows cookie, complement, hook word, feature word, shared exception record,
shared CONTEXT, debugger word and the actual EXCEPTION_POINTERS object. It must
bind the one actual domain; the pair's current fields must refer to that record
and context. Pass the actual pair address to the API, without reconstructing or
replacing it with a local pair. Record/context/pair/bindings and all added stack
words must be valid, stable and disjoint from the reached capture/scratch frame.
No default initializer, private state, host CRT cookie or encoded flag is added.

The proposed naked cdecl Watson adds that context after the five original
arguments: source S+18h, adjusted EBP+2C4h; caller cleanup becomes24 instead of20.
Only EAX is used to load the bound cookie before the original XOR, so the
captured remaining integer registers and XOR flags survive. After their capture,
EDX is dead across the upcoming memset and can load the context and PUSH its
actual feature binding before original PUSH50h/PUSH0/PUSHrecord. Call CL directly
and clean16 instead of12. No dereference of the feature cell is introduced.

For each hook call push the actual hook-word address, then the original reason,
call the existing full helper, POP ECX for the original reason, and LEA ESP,[ESP+4]
to discard the extra binding without changing the helper's final AND flags.
The helper preserves EAX. Before the cookie check, PUSH [EBP+2C4h] directly
preserves all raw registers, CALL the actual checker, then LEA ESP,[ESP+4].

Propose checker `void __cdecl check_native_crt_cookie_00bfe120(const Bindings&)`
as a **register-contract declaration** requiring actual ECX at entry, not an
ordinary C++ call that supplies ECX. Entry S+4 is now the context. PUSH EAX;
MOV EAX,[ESP+8]; load its cookie binding; CMP ECX,[EAX]; POP EAX; conditional
branch; F3C3 RET preserves EAX and comparison flags on equality. The mismatch
must directly JMP the complete reporter with the same return word and context
slot. Do not replace the edge with a callback or a generic security exception.

For reporter `void __cdecl report_native_crt_gsfailure_00c185a4(const Bindings&)`,
keep original PUSH/MOV/SUB before capture. A new PUSH ECX at EBP-32Ch preserves
incoming ECX below the original328h allocation; all subsequent pre-PUSHFD
binding loads/stores use MOV/PUSH/POP only. Load the actual CONTEXT base into
ECX, store incoming EAX first, MOV EAX,[ESP] and store saved incoming ECX second,
then store untouched EDX/EBX/ESI/EDI and six WORD segments in original order.
PUSHFD/POP [base+C0h] therefore captures flags from the original SUB. POP ECX
restores the saved register before later original work. Native EAX is next
overwritten by MOV EAX,[EBP], so using it for the saved ECX does not lose a live
value. The new scratch word does not alias [EBP-320h] or the later cookie locals.
Later binding-address loads may change incidental volatile registers; preserve
all actual field widths, reloads, API order and return EAX from TerminateProcess.

This proposal adds stack accesses, pointer loads and code addresses. The shared
snapshot describes the source capture frame, not the original binary frame.
Uninitialized bytes stay uninitialized; no RtlCaptureContext or typed zeroing is
acceptable. New fault sites, asynchronous observation, incidental volatile
registers, relocation placement and native exception/SEH identity are not proved.
DF=0 and real readable/writable stack/extents are required by the original memset
path. Failures/faults propagate through the real OS/compiler domain, without an
added catch, validation, stack repair or return policy.

## Native consumer blocker and validation gate

CP C0DC54[70] demonstrates a concrete incompatible consumer. With handler entry
ESP=H, its exception record/R/context/dispatcher words occupy H+4/+8/+Ch/+10h.
C0DC66 sets EAX=R; C0DC6A loads ECX=[R+8]; C0DC6D XORs ECX with EAX; C0DC6F calls BFE120
with no added argument. C0DC75 requires returned EAX=R to load [EAX+18h]. The
checker sees return word at H-4, then the handler's return word at H, not a
bindings slot. Our proposed context-bearing checker cannot directly serve this
unchanged native edge even if it preserves EAX. The same warning applies to
unmodified compiler/native frames. A real canonical owner and no-extra-argument
checker/reporter domain must be established separately before claiming that edge
closed. CP evidence is an external read-only contract, not newly claimed code.

After proposal acceptance, implementation validation should map all122 native
instructions and six direct/tail sites to the complete current COFF; qualify
only actual binding/frame/call-cleanup changes. Check F3C3, the real checker-to-
reporter tail, partial writes, capture flags, ignored read, all return tails,
four actual CL/hook source call sites and all ten real API import sites. Prove
every new reference-member offset with current compiler accessors, source/header
reads, exact /Fo object, sole definitions and unique equal archive membership.
Use strict Win32 build, eight seeds and two existing CTests. No execution of
Watson, deliberate cookie failure, termination probe or synthetic API test is
needed or proposed. Native owner/startup, C0DC54 closure and gameplay remain open.
