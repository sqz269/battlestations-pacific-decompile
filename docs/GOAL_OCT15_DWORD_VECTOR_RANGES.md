# Scene DWORD vector range providers

Addresses: 00468460, 00469950.

| Native address | Source API | Native span | Coverage |
|---|---|---|---|
| 00468460 | copy_backward_native_scene_dword_range_00468460 | 00468460..0046848E, 47B/19ops | complete |
| 00469950 | fill_native_scene_dword_count_00469950 | 00469950..0046997D, 46B/21ops | complete |

Copy-backward takes cdecl stack arguments first,last,destination_end and uses
plain RET. It reads last then first, subtracts modulo32 and arithmetic-shifts
the raw difference by2. It captures destination_end, scales the shifted bits by4
modulo32 and captures start=end-bytes before any CRT invocation. Only a signed-
positive shifted count reaches the actual SDK memmove_s(start,bytes,first,bytes)
call; all four native arguments are established by PUSH/PUSH/PUSH/PUSH and
ADD ESP,10h. The returned status is ignored. EAX always returns the captured
start, including after a returning error report, zero/negative counts and wrapped
or unaligned input differences. Source expresses SAR2 with unsigned bit operations
and never adds alignment, ordering, null or destination-capacity checks.

Fill-count takes stdcall stack arguments first,unsigned_count,value_address and
uses RET0Ch. TEST clears CF, so JBE at9960 skips only raw count0; high-bit counts
are not rejected as signed-negative. JA after the repeated TEST means nonzero
remaining count. Each iteration rereads the current source DWORD, stores to
current destination, decrements modulo32 and advances the address by4. EAX wraps
originalfirst+(originalcount<<2), independently of loop cursor or source contents.
Count0 does not read the native third stack word or any source payload. The
Source zero path returns first without forwarding value; nonzero delegates the
actual complete accepted 00441180 provider after verifying all46 native bytes
are identical, including branch displacements and RET. No value snapshot,
positive-only domain, copied owner, callback or guard is introduced.

The real canonical Source is fill_native_dword_count_00441180 in
native_dword_fill_ranges.cpp. Its Source/header contract and current word-load/
store helpers were read: current source bytes are reloaded before every store,
including progressive payload aliases. Source reuse is not a native edge from
00469950 to00441180. Both native bodies retain their correct established STL
library names. The family names describe raw DWORD operations and surrounding
users, not a recovered nominal vector/element ownership type.

All three live sites are in 0046AFB0: `0046B07C CALL 00469950` and
`0046B10A CALL 00469950` supply three stack words to count-fill and use its
RET0Ch. `0046B143 CALL 00468460` supplies first=EDI,last=EBX,end=EBP to
copy-backward and ignores its result. Its cdecl cleanup is deferred
to ADD ESP,18h atB15A after a subsequent separate cdecl fill-range. Parent ECX
setup is not a receiver input to either provider. Exact setup, following result
use, containing functions and typed transfer rows are in JSON. The parent's
other providers are independently owned/delivered, not assumed incomplete from
a stale index and not replaced by this packet.

The sole native outgoing edge is `00468483 CALL 00BF67A7`. The complete native
memmove_s body was read, including zero-count, null-pointer, insufficient-size,
errno/report and actual move paths. This packet uses the concrete current SDK
memmove_s API already accepted by the scene DWORD move provider. Its current
errno/handler/exception policy is not the original static CRT's encoded-handler,
private errno, debugger/Watson/FH3/SEH or fault-address identity. No private CRT
child is invented or stubbed. Invalid-pointer reporting may return; captured
Source arguments and result stay independent of that continuation.

All93 owned native bytes match the installed PE; the fill reference's46B is
comparison evidence, not additional coverage. Complete typed bodies/branches,
the SDK-call edge and all three parent call edges are retained without repair.
Guarded live batches verify C:/Users/sqz269/bsp.gpr and program
/battlestationspacific.exe, x86 image base00400000. Annotation records old values,
preserves names/comments, takes the write lock, saves, exports and reads back.
The Source draft awaits normal Win32 compilation and configured existing checks.
No new routine tests are added. Reached backing must remain valid; original
private argument/frame/payload coincidences and asynchronous mutation are not
exposed by these value interfaces. No original binary ABI, dedicated native
differential, application adoption or gameplay proof is implied.
