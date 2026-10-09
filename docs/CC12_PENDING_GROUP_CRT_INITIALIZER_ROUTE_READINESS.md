# CC12 pending-group CRT initializer route readiness

The Original PE has a concrete static startup route to the five-group
initializer at `00CD27C0`. Its pointer at `00CE3020` is slot **571** of the
**1004-DWORD** void initializer range `00CE2734..00CE36E4` (end exclusive).
This closes the prior table-membership and static call-route gap. Invocation
remains conditional on the preceding children and table values; no execution,
Source lifecycle or gameplay credit is applied.

The report retains whole live/PE-equal bodies for entry `00BFD2BD` (10 bytes),
CRT startup `00BFD0DD` (480 bytes), `__cinit` at `00BFBC47` (146 bytes), and
`__initterm_e` at `00BFBA92` (32 bytes), plus exactly the four-byte selected
table word. Every live query verifies project `C:/Users/sqz269/bsp.gpr` and
program `/battlestationspacific.exe`. No adjacent table word or external
child body is read, and no Ghidra mutation is made.

## PE entry and pre-WinMain route

The configured Original image's PE entry is `00BFD2BD`. Its two operations
call `00C1815E` and tail-jump to `00BFD0DD`. That CRT startup's normal path
pushes EBX and calls `00BFBC47` at `00BFD222`. The preceding local schedule
sets EBX to one; this depends on ordinary preservation by its unopened
children. The cinit result is tested at `00BFD228`. Zero proceeds to
`00BFD233`; nonzero calls `00BFBA09`, whose return/termination behavior is
not invented. The owned Native WinMain call at `00BFD24F` follows cinit and
other unopened children. No startup child, SEH policy or runtime is executed.

## Integer initialization is an early-return gate

`__cinit` first conditionally calls a current pointer at `00D693D8`, then
calls `00C0680F`. At `00BFBC6F..79`, it pushes end `00CE3708` and start
`00CE36E8`, then calls the complete 32-byte integer initializer walker.
Those bounds describe eight DWORD slots; none of their contents is read by
this audit. `__initterm_e` saves ESI, loads the first stack word as its cursor,
starts EAX at zero, and compares cursor/end as unsigned. Before each next
slot it exits if EAX is nonzero; otherwise it reads the current pointer,
skips null or calls ECX, then advances four bytes. Null slots preserve EAX.
Its result is tested by cinit, and both argument pops preserve TEST flags.
A nonzero result returns from cinit before the void initializer loop.

The metadata's zero named calls does not mean no physical call: the walker
contains `CALL ECX` at `00BFBAA5`. Its actual table values, children and
failure policy remain unopened.

## Actual void-table invocation

On the zero-result arm, cinit saves ESI/EDI and registers an unopened atexit
function. It sets ESI=`00CE2734`, EDI=`00CE36E4` and performs an unsigned
start/end comparison. At `00BFBCA1`, each iteration reads the current DWORD
at ESI; zero skips the call, otherwise `00BFBCA7` calls that current EAX
without pushing an argument. It adds four to ESI and repeats while ESI<EDI.
It does not test initializer EAX results. Ordinary children must preserve
the cursor/bound registers and return stack for this loop to continue.

The independently replayed four bytes at `00CE3020` are `C0 27 CD 00`,
giving `00CD27C0`. The word is within the concrete bounds, aligned at index
571. No adjoining words are inferred from that finding. If all earlier
called entries return normally and the word remains unchanged when read,
this loop reaches and invokes the accepted 153-byte initializer. That body
uses no argument or local frame and ends in plain RET; this particular
zero-pushed-argument delivery fits its observed schedule. Static ROM section
metadata does not establish callback side effects or live execution.

The earlier initializer receipt still owns its exact 20 DWORD stores across
five pairs: head+4=0, head+8=tail, tail+8=0, tail+4=head, in ascending group
order. It performs no profile, count or general zero-fill writes. Its 153
bytes are referenced and neither reread nor counted as Source in this packet.

## Listing and lifecycle limits

The CRT startup's saved live listing has 133 operations. Full linear decoding
of its same 480 physical bytes finds 156, including 23 unlisted diagnostic
operations at `00BFD26A..00BFD297` and `00BFD2A4..00BFD2B1`. Root Astra
reviewed and retained those operations, including their returns, stack resets
and calls, without adding listing/flow/AddressSet entries or treating them
as normal fallthrough. SEH registration data `00E03030`, external handlers,
prologue/epilogue bodies and exception policy remain unopened. This route
claim needs only the explicit normal entry-to-cinit edge and owned table loop.

Actual mutable Source group backing and its production owner/lifecycle remain
open. These groups do not identify the separate pending-global sentinel pair.
The Source build still requires startup and gameplay validation. This packet
changes two evidence files, applies no Source/ABI/startup/gameplay credit,
and runs no build, test or probe.
