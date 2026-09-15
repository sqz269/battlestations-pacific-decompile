# Native Watson and error ownership discovery BL

A bounded direct Watson/cookie packet is ready for qualified source implementation:
BF65BB, C04EF3, BFE120, C185A4 and cookie owner C1815E, with the fixed50h zeroing
path used by Watson. It needs actual recovered storage and real Win32 APIs;
no arbitrary error/termination callback is required. The caller's errno/PTD and
encoded invalid-handler ownership remain separate prerequisites.

Base `eaa1f87cc5594484494824afe5d1fa7f2efb5545`. Discovery only: no C++ or Ghidra
writes, build, tests, runtime experiments or game execution. Twenty-two complete
code/data spans total1972 bytes and match live Ghidra plus the installed PE.
All28 direct call/tail rows passed verification;23 indirect sites are separately
qualified. The report retains the entire local SHA256/SHA512 inventory and the
locally inspected Windows SDK x86 layout evidence.

## Direct Watson body

BF65BB..BF66B6 is252 bytes. Its native interface is cdecl with five diagnostic
arguments, all unused here. Its bytes include a plain return path despite the
current no-return metadata. If the actual services return and the cookie check
returns, the original caller cleans its five arguments.

Let S be entry ESP, pointing to the return word. The adjusted EBP becomes
S-2ACh. Local storage contains an80-byte exception record atS-32Ch, an8-byte
pointer pair atS-2DCh, a716-byte x86 CONTEXT atS-2D4h, and a guard atS-8.
The report lists every written CONTEXT member and its width. Critical details:

- CONTEXT is not zeroed. Unwritten stack fields and upper halves of segment
  DWORDs remain untouched. ContextFlags is10001h (`CONTEXT_CONTROL`).
- The stored EAX is the actual cookie XOR adjusted EBP. Stored EFLAGS come
  after that prolog arithmetic, rather than from the original caller.
- Stored EIP is the caller return word; EBP is the saved caller frame pointer;
  stored ESP is S itself, the address of the return word.
- Only the50h exception record is cleared through BF79F0. Its code becomes
  C000000D and its exception address becomes the captured caller return PC.

The service order is `IsDebuggerPresent`, then `SetUnhandledExceptionFilter(NULL)`,
then `UnhandledExceptionFilter` with the actual local pointer pair. The previous
filter is not restored. Only a zero filter result together with the captured
zero debugger flag invokes C04EF3(2). Finally, the body calls
`TerminateProcess(GetCurrentProcess(), C000000D)`.

If termination returns, BF65BB recovers the cookie, restores ESI, calls BFE120,
restores the adjusted frame and returns. It adds no abort, fastfail, retry or
exception translation. It owns no FH3/SEH cleanup frame. No errno lookup,
allocation, TLS/PTD operation or invalid-handler decoding occurs inside Watson.

## Concrete helpers and shared failure storage

C04EF3 is eight bytes: `AND dword ptr [109EEA8],0; RET`. The observed reason1/2
stack argument is ignored. It performs a real read-modify-write and changes
flags; it is not a synthetic debugger callback or an omitted no-op.

BFE120 is15 bytes. It compares ECX with actual cookieE15590, uses F3C3
`REPZ RET` on equality, and tail-jumps C185A4 on mismatch. The live listing
renders the two-byte return simply as RET.

C185A4..C186A7 is260 bytes. It writes actual global CONTEXT109E5C0[2CCh],
EXCEPTION_RECORD109E568[50h] and debugger flag109E5B8. Native D6E1CC contains
the pointer pair `{109E568,109E5C0}`. These require canonical process storage
with the verified initial zero-fill; the original address words are not usable
host pointers in the rebuilt executable.

The reporter preserves current integer/segment capture and partially updates
that shared storage without clearing it. Its stored EFLAGS follow its own
`SUB ESP,328h`; its stored ESP is entryESP+4, unlike Watson's entryESP. It retains
the otherwise unused `[EBP-320h]` read. It writes codeC0000409, flags1 and the
captured return address, and copies actual cookie/complement words into stack
locals.

It stores `IsDebuggerPresent` into109E5B8, calls C04EF3(1), clears the Windows
exception filter and calls `UnhandledExceptionFilter` on the actual global
pair. It ignores the filter result, reloads109E5B8 and calls the hook again if
that word is zero. It then requests termination with C0000409 and retains
LEAVE/RET if termination returns. No further internal CRT provider occurs.

C1815E..C181F1 is148 bytes and owns the cookie pair. Initial values are
BB40E64E/44BF19B1. If the current cookie differs from the default and has a
nonzero high16, it only refreshes the complement. Otherwise it combines actual
FILETIME, process ID, thread ID, tick count and performance-counter words.
It applies the exact default/high16 adjustments, publishes cookie first and
complement second. The QPC result is ignored and its local output is not
initialized here; no newer-CRT mixing or extra nonzero repair should be added.
The original PE entry invokes this initializer before CRT startup.

BF79F0's full122-byte body includes an SSE tail for zero fills of at least100h.
Watson always requests50h, so that route and even its109EEA4 feature-word read
are unreachable here. Its concrete byte/aligned-REP-STOSD path supplies the
required fixed-size fill; general SSE memset remains outside this packet.

## Separate errno and invalid-parameter ownership

BFFB8B calls C051B7 and returns PTD+8 when nonnull, otherwise canonical fallback
E159E8. C051B7 saves LastError, obtains the current TLS getter through C05070,
and may allocate a214h PTD through C0485A. It decodes the actual setter109DE3C,
publishes the PTD, calls C050F8 initialization and stores the thread fields.
On setter failure it frees, then **clears ESI** before restoring LastError and
returning null. The missing `POP ECX; XOR ESI,ESI` atC0521F..21 was recovered
from complete PE/live bytes; the live listing alone would return a freed pointer.
Actual getter/setter/index, calloc/free and PTD initialization remain external.

BF66EF decodes current global109DD64. A nonnull result tail-jumps the actual
handler with the original return word and all five arguments unchanged. Null
calls C04EF3(2), then tail-jumps Watson with the original arguments. There is
no added thread-local handler priority.

The10-byte BF65B1 publisher simply stores its incoming encoded word109DD64.
Original `__mtinit` calls BFBDFB `__init_pointers`, which obtains encoded-null
through C04FD5/C04F67 and supplies that value to BF65B1. Literal zero is not the
established initialized handler representation. Existing decoder/encoder source
is reusable but borrows actual TLS/PTD/owner state. Existing UCRT adapters
explicitly have a different thread-local/global handler domain.

The implementation packet should preserve native register/frame capture,
partial writes, shared record lifetime, cookie ownership and returned-service
paths. A zero-initialized typed CONTEXT, `RtlCaptureContext`, host UCRT Watson,
generic termination callback or fresh private errno would change the contract.
This evidence does not establish native ABI/SEH identity, source integration,
runtime behavior or the entire heap-selector getter-error chain.
