# Allocator exception-base cleanup readiness

The owned physical function is `00BF6454..00BF6469`: **22 bytes and seven
instructions**. The installed PE and exact live byte read agree. Primary
Astra review independently confirmed SHA-256
`5aa135c550cfa5127551047f9b505f7313399dc686df7d16a9711763dc56a8bc`
and authorized this read-only normal-flow model.

Ghidra's visible listing and metadata still report six instructions, omitting
the physical `POP ECX` (`59`) at `00BF6468`. That instruction removes the
pushed `free` argument before the plain `RET` at `00BF6469`. The omission is
retained as evidence. Its cause and any no-return/flow flag are unqueried;
no listing, flow, prototype, comment or other Ghidra mutation is made.

The current private Source helper comments `00BF6454..00BF646A`, an inclusive
23-byte range. Its endpoint is one byte too far. This packet neither reads nor
interprets byte `00BF646A` and does not edit the comment.

## Scope and accepted continuation

Baseline is published `2307e3374e67761712886394517ecd4e18ee8c3b`, which includes
the admitted raw-copy adapter and its actual typed consumer. An initial
metadata/body check found the listing omission, stopped interpretation and
released the lease. Primary Astra independently decoded exactly the allowed
22 bytes, then explicitly selected continuation without repair. The renewed
lease covers this body and metadata-only `_free` at `00BF9DC8`.

Live CLI queries verify project `bsp`, program `/battlestationspacific.exe`,
language `x86:LE:32:default` and image base `00400000`. The configured project
file remains `C:/Users/sqz269/bsp.gpr`. Initial function count was 64729.
No child body, adjacent byte, static owner/slot/string/profile/RTTI, handler or
caller body is selected. Source, CMake and ledger files remain unchanged.

## Complete physical instruction schedule

```text
83 79 08 00 c7 01 70 93 d6 00 74 09 ff 71 04 e8
60 39 00 00 59 c3
```

| Site | Instruction | Access or normal-flow effect |
| --- | --- | --- |
| `00BF6454` | `CMP DWORD [ECX+8],0` | Read and test the full control DWORD first |
| `00BF6458` | `MOV DWORD [ECX],00D69370h` | Publish raw profile data without changing compare flags |
| `00BF645E` | `JE 00BF6469` | Skip message access and child call when the captured comparison was zero |
| `00BF6460` | `PUSH DWORD [ECX+4]` | Load the current message pointer after profile publication |
| `00BF6463` | `CALL 00BF9DC8` | Deliver that single stack argument to `_free` |
| `00BF6468` | `POP ECX` | On normal child return, discard the argument and leave ECX equal to its pushed value |
| `00BF6469` | `RET` | Plain return; no incoming stack-argument cleanup |

The saved metadata name is `LIBCRT_unmatched_00bf6454`, with a saved
`undefined ...(void)` prototype. Decompilation derives a fastcall ECX input;
the physical instructions establish the receiver operand. The packet
preserves both metadata observations without updating a prototype or
promoting the descriptive cleanup role to a recovered class identity.

Let `D` be entry ECX and `T` be entry ESP. The zero-control path leaves ECX
equal to `D` and returns directly. The owned code never writes EAX, so that
path preserves incoming EAX; it does not establish a semantic return value.

On the nonzero path, `PUSH [D+4]` puts the captured message at `T-4`; CALL adds
its return address at `T-8`. Under the child's normal cdecl-return contract,
the child returns to `00BF6468` with ESP=`T-4`. The physical POP restores
ESP=`T` and sets ECX to the pushed message value. RET then returns normally.
This is a call followed by cleanup and return, not a tail jump.

EAX/EDX and flags after the child remain subject to the child. POP and RET do
not replace the returned flags. On the zero path, compare flags survive the
profile MOV, conditional branch and RET. The owned body does not write
EBX/ESI/EDI/EBP, establish a local EH frame, or use x87. No exceptional return,
Native no-return property or provider register behavior is established.

## Direct footprint and field lifetime

The leaf always reads the four bytes at `D+8` before attempting its four-byte
write at `D+0`. It conditionally reads the four bytes at `D+4` afterward. These
accesses require only the appropriate leading 12-byte backing, with `D+0`
writable and the selected input fields readable. There is no receiver null
check, prior-profile read, string/pointee read or larger-owner access.

The comparison uses the whole DWORD against zero, not a byte or an ownership
bit mask. It does not normalize or clear that field. The only direct receiver
write is the profile DWORD. The message and control fields are not locally
rewritten or reset before or after the child call.

No message-pointer null guard is present. Any loaded value, including zero,
is delivered to the child when control is nonzero. Provider behavior for that
value is outside this packet. If the message equals the receiver, the same
pointer is still delivered; no owner-lifetime or allocator-compatibility
guarantee is inferred. The lack of local field writes does not constrain a
child's effects on aliased storage or prove that backing remains alive.

`00D69370` remains raw numerical data. It does not establish a callable Source
vtable, exception type, RTTI, static failure owner or Native runtime identity.
Earlier effects, including profile publication, can remain if a later access
or child fails; no cleanup rollback is selected in this body.

## Native child and current Source boundary

Metadata-only review identifies `00BF9DC8` as `_free`, with saved signature
`void __cdecl _free(void* _Memory)` and body bounds
`00BF9DC8..00BF9E55`. No bytes, listing or pseudocode from that child are read.
Its allocator implementation, null handling, failure policy, nested calls,
flags and exception behavior remain unproved here.

The current Source helper is private `destroy_base(NativeLegacyExceptionStorage&)`
and is declared `noexcept`. It uses a volatile owner reference, captures the
full `owns_base_message_08` value before publishing `exception_vtable`, then
conditionally calls `std::free(actual.base_message_04)` from the current
`<cstdlib>` boundary. There is no additional pointer guard or local field
clear. Its pointer is loaded for the call after profile publication.

This matches the observed source-level access order and conditional argument
delivery. The captured-value condition is not proof of emitted compare/flag
placement, and the Source `noexcept` boundary is not Native EH evidence. This
packet does not inspect emitted cleanup/free bindings or assert that current
Source `std::free` is the selected Native provider.

`NativeLegacyExceptionStorage` is a 0x28-byte type whose relevant fields are
at offsets 0, 4 and 8. Its existing valid typed owners supply leading storage;
the private helper is not a callable raw-12 adapter. A raw 12-byte allocation
must not be cast to that larger owner to reuse it. No new type or ownership
policy is introduced.

## Current admission context and validation

The newer raw-copy primary review is pinned with its current Source/header
and documentation. It records the explicit `#pragma function(strlen)` compiler
fix, normal build with three existing checks, the complete 90-byte Source
adapter corresponding to 88 Original bytes, and the actual 132-byte typed
consumer / 18-byte private wrapper. Its current CRT imports, Core selection
and qualified Source admission do not prove this cleanup's Native ABI or
runtime behavior. Those earlier functions are not counted again.

All 50 Source/document input pins from that newest primary review match this
baseline. The report independently decodes the complete 22-byte owned span,
checks the live-byte and primary-review hashes, identifies the one omitted
listing instruction, verifies the control/profile/message/call/POP/RET order,
and pins the bounded Source/type/verification excerpts.

The two-file staged diff and independent report checks are static evidence.
There is no Source/Ghidra/CMake/ledger mutation, build, test, probe, Native
execution, Source admission credit or ABI/gameplay claim in this packet.
