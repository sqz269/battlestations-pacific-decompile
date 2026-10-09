# Deferred mission Lua payload retirement: limited 008876B0 evidence

The currently declared `008876B0..008876E9` span passes the 300-byte size gate:
58 live bytes match the installed executable and decode to 18 instructions.
It does **not** establish a complete destructor. The span installs an exception
frame and ends at an ordinary `_free` CALL, with no following stack cleanup,
saved-register restoration, `FS:[0]` restoration, or RET. The decompiler's printed
return is therefore insufficient evidence of a native normal-return path.

As directed by primary review, this packet stops at E9 and preserves the result
as a limited prefix audit. It does not inspect the continuation, the handler at
`00C97543`, child native bodies, callers, tables, or the 984-byte named dispatcher.
No scripts are enabled and no flow repair or exact no-return inference is made.
Source readiness remains blocked first by body extent/continuation, then by the
visible raw vector service and the eventual complete lifetime/EH contract.

The published and worker baseline is
`c1893651fd351ea67c5f2bb3151fd06553112d1f`. The accepted queue-drain audit already
establishes that the caller passes its then-current removal node **plus 8h** in
ECX to this entry. This packet changes exactly this document and its report and
claims zero Original, Source, ABI, runtime, or gameplay credit.

## Complete observed prefix

Let P be the actual incoming payload pointer and S the entry stack pointer.
The observed instructions are:

| Site | Native instruction | Observed effect |
| --- | --- | --- |
| 008876B0 | `PUSH -1` | Initial state word at S-4. |
| 008876B2 | `PUSH 00C97543` | Handler word at S-8; handler uninspected. |
| 008876B7 | `MOV EAX,FS:[0]` | Capture the current exception-chain head. |
| 008876BD | `PUSH EAX` | Previous head at S-12. |
| 008876BE | `MOV FS:[0],ESP` | Publish the stack record at S-12. |
| 008876C5 | `PUSH ECX` | Save P at S-16. |
| 008876C6 | `PUSH ESI` | Save incoming ESI at S-20. |
| 008876C7 | `MOV ESI,ECX` | Retain P in ESI. |
| 008876C9 | `PUSH EDI` | Save incoming EDI at S-24. |
| 008876CA | `MOV [ESP+8],ESI` | Write P again to the S-16 payload slot. |
| 008876CE | `LEA EDI,[ESI+10h]` | Form the raw header address P+10h. |
| 008876D1 | `PUSH 0` | Push one zero DWORD at S-28. |
| 008876D3 | `MOV ECX,EDI` | Deliver P+10h in ECX. |
| 008876D5 | `MOV [ESP+18h],1` | Change the S-4 state word to 1 before the child call. |
| 008876DD | `CALL 006B88E0` | Invoke the raw vector service. |
| 008876E2 | `MOV EDI,[EDI]` | Reload the current first DWORD of the retained header. |
| 008876E4 | `PUSH EDI` | Pass that current pointer to free. |
| 008876E5 | `CALL 00BF6989` | Ordinary cdecl `_free` call; encoded continuation is EA. |

All 58 bytes are retained in the report, with SHA-256
`cadd7e8bc2a59ebd1d5098838a77cb97b92409c7e7a01eac84b73c1cfd696c0b`.
There is no branch or return instruction in this observed span. The last CALL's
encoded return address is `008876EA`, which was not read by this packet. The
initial exception state is -1 and becomes 1 before `006B88E0`; no further state
change or exception-chain restoration appears in the authorized prefix.

## Receiver, child arguments, and extent limits

The incoming ECX is the actual payload P, not a copied `MissionLuaDeferredCall`.
The prefix explicitly retains P in ESI and on the stack. It performs no explicit
read of incoming EDX or an incoming stack argument. EDX remains unchanged through
the first child-call setup; because that child's native body is outside scope,
this observation does not prove that the transitive operation ignores EDX or
other unspecified register residues.

At `006B88E0`, ECX and EDI hold P+10h, ESI holds P, EAX still holds the previously
captured `FS:[0]` value, and a zero DWORD is on the stack. The current analysis name
`BSP_LuaVariantVector_Resize` is an interpretation, not a recovered symbol. This
packet proves argument delivery and the later header reload, not which elements
are destroyed, how the header is mutated, or the child's complete register ABI.
No immediate stack adjustment follows this call in the prefix. Since the final
epilogue is missing, its actual argument cleanup cannot be established merely
from that absence.

The post-call `MOV EDI,[EDI]` requires a compatible child to preserve the retained
header address in EDI. It reads the header's **current** pointer after the complete
child return; capturing that pointer before the call would change the schedule.
The free call receives that reloaded value; it does not explicitly pass P or the
caller's node base. No nonaliasing guarantee is inferred for malformed values.
Its metadata declares `void __cdecl _free(void*)`; a complete calling sequence
must account for the pushed argument, but that cleanup is outside the visible span.

The only direct payload memory access in this prefix is a four-byte read at
P+10h after the child returns. P+10h is also passed to the child, whose required
storage extent remains unaudited. No payload-field store, first-string access,
or second-string access appears before E9. The visible stack writes and `FS:[0]`
write are explicit and separate from payload storage. The total payload/vector
extent, normal return value, full register preservation, and complete destructor
side effects remain open.

The prior queue-drain contract is pinned as historical, accepted evidence:
`0088828B/8E` delivers R+8h in ECX with no pushed argument. After this destructor
returns, that caller pushes R to `00BF65AC`, performs four-byte cleanup, and then
decrements the current host count. Therefore this prefix's free operand
`*[R+18h]` is distinct from the later caller's node-base operand R. The existing
caller's field accesses establish its own payload footprint; they do not prove
the complete storage requirements of this destructor or its vector child.

## Current Source compatibility

The visible child `006B88E0` has live metadata span `006B88E0..006B8983`
(164 bytes, 58 metadata instructions). Only metadata was queried. A case-insensitive
exact-address search in `include/` and `src/` found no Source reference or
definition. The missing raw operation cannot be replaced with `std::vector::clear`
without recovering its element format, destruction, header updates, register
contract, and allocation policy.

The existing `MissionLuaDeferredCall` owns two `std::string` members and a
`std::vector<MissionLuaArgument>`. `MissionLuaArgument` itself owns a string and a
recursive vector and carries typed number/boolean/entity values. These are
semantic value types, not proof of a compatible P+10h raw vector or a native
payload destructor. The existing projected queue's copy/erase behavior also does
not supply this actual-pointer service. The accepted drain audit and its 31 input
pins were replayed; those inputs remain unchanged at the current baseline.

For `00BF6989`, live metadata identifies a five-byte `_free` entry with a cdecl
pointer argument. `NativeGameContainerLifetimeCalls::free_00bf6989` in
`src/native_game_container_lifetime.cpp:41` already calls actual `std::free`;
`singleton_lifetime_free` also calls actual `std::free`. These are concrete host
CRT boundaries. They do not establish that an arbitrary original or reconstructed
vector buffer belongs to their allocation domain. A future composition must use
the allocator/free pair belonging to the actual vector producer, including any
mutation of the buffer during `006B88E0`.

The old name-ledger/Ghidra comment calls the 58 bytes the whole body and suggests
that the two string buffers leak because no string release follows. The current
evidence supports only absence of those operations **within this prefix**. The
missing continuation prevents any whole-destructor no-string-release or leak
conclusion. This packet preserves the existing name and comment without treating
that interpretation as established behavior. No reconstruction record exists for
`008876B0` in the current reconstruction shard, and none is added here.

## Holds and bounded next packet

The primary agent will handle body-extent/flow recovery after this lease releases.
A concrete first continuation packet can verify adjacent function metadata, then
inspect at most 40h bytes starting at `008876EA` through `00887729`, stopping at
any verified conflicting function boundary. Combined with this prefix, that first
window is 122 bytes, still below 300. This is a proposed scope, not evidence that
the epilogue lies there. Compare physical bytes and listing, locate the actual
cleanup/RET and exception-chain restoration or identify the next explicit extent
hold before further reading or any separately authorized repair. Handler/table
analysis remains separately scoped.

After the complete body is established, the visible raw vector service is an
independent 164-byte candidate for its own metadata-gated audit. Unknown continuation
calls must not be presumed absent. Any Source recommendation must preserve the
complete payload lifetime, stack/FS frame, callback/failure ordering, and actual
allocation domains. If the vector child or free fails, the visible prefix supplies
no rollback; what the native handler or missing continuation does is uninspected.
No native exception, unwind, fault, Lua/runtime, owner, or gameplay compatibility
is established by this packet.
