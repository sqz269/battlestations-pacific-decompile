# Native online achievement pump

Address: 00A3FA70. Packet: `orch5_native_online_achievements`.

`pump_native_online_achievements_00a3fa70` implements the complete normal body
`00A3FA70..00A3FD1F` against the existing `NativeOnlineManagerStorage` allocation.
Its descriptive name is a hypothesis. The original ABI is ECX=manager, a force
byte in a DWORD stack argument, RET 4; the explicit C++ interface is not a
binary replacement. This is independent of the older projected vector in
`xlive_system_pump.cpp`. It introduces no manager, queue, or identity mirror.

| Routine | Coverage | Evidence |
| --- | --- | --- |
| 00A3FA70 | complete normal control flow | 688 saved-image bytes match installed PE; all 207 listing instructions inspected |

The read-only BSP wrappers verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. No Ghidra mutation or save was performed. The
current body has zero listing gaps. At `A3FC5D`, `CMP ESI,ESI` followed by `JZ
A3FC66` makes the `A3FC61` invalid-parameter call unreachable. The decompiler
warning here is justified; no repair is requested. The report retains that
real numeric call row for mechanical validation, although execution skips it.

## Actual storage and producer

The producer was checked before interpreting the fields: `A410A0` passes
ECX+360 to `A41030` at `A410D9`; `A41030` writes one DWORD at vector+8 then
advances that end by four. This is the queue described in `AWARD_GRANT.md`.
The checked vector has begin at manager+364, end at +368, capacity at +36C,
and its container header at +360. The pump never changes begin or capacity.
Pair stores at `A3FB76` and `A3FB86` establish the submitted `{user,id}` layout:
selected user DWORD +11C becomes pair+0, queued ID becomes pair+4.

| Field | Native use |
| --- | --- |
| +364/+368 | current queue pointers, reloaded at checked accesses and removal |
| +384..+394 | five cleared overlapped DWORDs; +398/+39C survive |
| +3A0/+3A4 | retained pair pointer and signed-loop/unsigned-allocation count |
| +3A8 | last submission/extended-error result |
| +11C and +8C+4*index | user index and diagnostic cached state load; the latter can overlap username bytes |

## Submission, completion and library boundaries

An empty current queue returns immediately even with a retained batch. Otherwise
submission requires result !=3E5, count zero, overlap DWORD !=3E5, or a nonzero
low force byte. Force 100h is false, 101h is true. Force may release a still
pending SDK buffer; this native lifetime hazard is preserved.

Submission clears five overlapped words, releases the current buffer, recomputes
the current count, stores it, and allocates. Count uses 32-bit wrapped pointer
subtraction and arithmetic shift by two. Unsigned `MUL 8; SETO; NEG; OR` saturates
overflow to FFFFFFFF, while pair iteration uses signed count comparisons. The
size is passed unchanged to the supplied matching `NativeOnlineStorageMemory`
allocator. `BF55BE` calls malloc/new-handler and throws bad_alloc on exhaustion;
the source adds neither an impossible-size rejection nor rollback.

Every checked index uses unsigned comparison, and every native reload after a
returning allocation, CRT or SDK boundary is retained. The two diagnostic
calls at `A3FBC9`/`A3FD10` target the proven single-RET `4254B0`; their useful
effects are empty. The source retains potentially faulting diagnostic indexed
loads and the second queue check during snapshot construction.

SDK pointers resolve only from an already-loaded module: XUserWriteAchievements
ordinal 5278, XGetOverlappedResult 1083 and XGetOverlappedExtendedError 1082.
Each is Win32 stdcall. The overlap address is the actual owner+384. The pending
branch overwrites +3A8 with extended error without freeing or completing the
batch. The completed branch calls result(overlap,nullptr,TRUE); nonzero returns
without cleanup. Zero removes the first matching entry from the **current**
queue for each current batch pair, ignoring that pair's user field.

For a match, the signed positive trailing-element count is multiplied by four
with 32-bit wrapping. Both destination capacity and copy count passed to
`BF67A7` are that exact byte count; source is destination+4. Its return value
is ignored, and current end is reloaded and decremented even after an error.
The call-site cdecl cleanup is ADD ESP,10h at A3FCB1. Cleanup clears count,
loads/frees the current buffer, clears its pointer after free returns, then
clears result. A release callback can change count after its zero store.

The CRT entries are supplied explicitly to preserve the original policy.
`BF6713` calls `BF66EF` with five zero arguments and cleans 14h bytes. BF66EF
decodes the configured handler at 109DD64 and tail-calls it when present;
such a handler can return. The fallback reaches the CRT invalid-parameter
termination path. It is therefore incorrect to replace each guard with an
unconditional C++ exception. `BF67A7` returns zero for zero count; otherwise it
checks destination/source and capacity, stores errno 22/34, calls that same
handler, and returns the error if it returns. It does not clear the destination.
Valid copies call BF87E0. These library bodies are analyzed dependencies, not
new reconstructions or replaced CRT implementations in this packet.

## Verification and limits

The retained local fixture patches all 16 direct CALL sites in a copy of the
verified original body to controlled in-memory SDK/allocation/CRT boundaries.
It compares complete 3F0h owner bytes, queue and pair buffer bytes, and boundary
traces against the source. Its scenarios include pending state, high force
bits, current duplicate queues, signed/unsigned allocation boundaries, returning
invalid-parameter handlers, ignored move errors, and mutation during allocation
and cleanup. No real DLL is loaded and no account or network is accessed.

Final build, call-check, fixture and artifact-manifest results are recorded in
`reports/native_online_achievements.json`. This does not establish real SDK,
hardware-fault, FH3/SEH, asynchronous lifetime, full-manager pump integration,
fixed-address ABI replacement, or gameplay equivalence. The host must provide
valid live storage, matching allocator/free and CRT policies, and stable module
and entrypoint lifetimes. External asynchronous races are outside the source
contract; native ordering across synchronous calls is retained.
