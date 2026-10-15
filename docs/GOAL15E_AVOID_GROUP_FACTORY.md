# Complete native-storage avoidance group factory

Addresses: `00417CA0`.

| Routine | Coverage | New Source entry | Original ABI |
|---|---|---|---|
| `00417CA0..00417D58` inclusive, 185 bytes | complete | `find_or_create_native_avoid_group_00417ca0` | ECX actual manager, signed layer DWORD on stack, EAX group pointer, RET 4 |

This new Win32 naked entry operates on the existing actual `Cc13AvoidZoneManagerNative`
and `Cc13AvoidZoneNativeGroup` storage. Its Source fastcall shape adds only an ignored
EDX argument; the original ECX, physical stack argument, result and cleanup remain.
The older `avoid_zone_group_find_or_create_00417ca0(AvoidZoneTable&, int32_t)` remains
an unchanged, distinct semantic index-returning API and ledger record.

## Observed schedule

1. The layer argument is captured **early** into EBX at `00417CA1`, immediately after
   PUSH EBX. Search and allocation do not reread that stack cell. The exact-key loop
   uses signed count decisions, reads actual slot pointers and compares group+10h.
   Each continuation rereads manager+58h. A match reloads the indexed manager slot
   at `00417CED` for the result rather than returning the comparison pointer.
2. On no match, push 14h and allocate. After normal return, initialize the actual
   table word, zero group+4/+8/+Ch, then write -1 to group+10h. The original null
   branch still reaches the requested-key store at `00417CF9`; no graceful null
   result, extra guard, rollback or admission policy is added. The genuine current
   allocator normally returns storage or throws and does not return null.
3. Store the captured requested key. Read current manager+58h once at `00417CFC`
   into EDX for insertion. Find the first existing key strictly greater by a signed
   comparison; this scan uses captured EDX as its bound. Equal keys encountered
   here remain ahead of the new group. Shift actual pointer slots backward, then
   publish the new pointer. The final ADD at `00417D41` or `00417D4F` increments the
   **current** manager count, not a cached count plus one.
4. There is no 20-slot admission check. Reached addresses use literal DWORD/x86
   arithmetic and native memory access order; the Source kernel does not replace
   them with typed array iteration or C++ pointer subtraction. Valid backing for
   every reached slot, group, stack and control cell remains necessary. Allocation
   exceptions propagate with no locally invented cleanup frame or rollback.

The body retains its register preservation, flags, branches and both original
padding LEAs. Provider side effects on manager slots/count are therefore observed
at the same instruction points. No allocator callback or replacement owner exists.

## Concrete services and callers

| Containing function | Transfer site | Target | Evidence/contract |
|---|---|---|---|
| `00417CA0` | `00417CC9` | `00BF681B` | CALL after PUSH 14h; ADD ESP,4 at `00417CCE`; cdecl raw allocator |
| `00424D00` | `00424DBE` | `00417CA0` | key from saved stack input, manager EBP to ECX; EAX becomes next group receiver |
| `00424D00` | `00424DDA` | `00417CA0` | zero key pushed from ESI, same manager ECX |

The checked transfers are:

```text
00417CC9 CALL 00BF681B
00424DBE CALL 00417CA0
00424DDA CALL 00417CA0
```

The first caller has a preceding PUSH EDI for its following operation; the factory
consumes only the subsequently pushed layer DWORD with RET 4. Both live xrefs and
the physical call census find these two transfers. The earlier `00423C50` caller
attribution is unsupported by this direct-reference census and is corrected here;
this is not a claim about all possible indirect calls.

The existing library-role name `operator_new` is retained. Its body uses malloc,
then the actual new handler retry decision, then throws bad_alloc on exhaustion.
Source binds its existing `singleton_lifetime_allocate` service with native_bytes
and host_bytes both 14h, backed by current `std::malloc`, `_callnewh` and
`std::bad_alloc`. Its current CRT heap, handler, exception object and unwind frame
identities are qualified; no original-game CRT identity is asserted.

The sole table-operand substitution installs the actual const
`kNativeAvoidGroupLifetimeSourceVtable`, whose recovered slot zero dispatches the
complete existing Source group scalar-delete/destructor/zone/record/free chain.
It is a real callable table shared with manager clear, not the original numeric
00CE3858 identity, a fake constructor or an assertion about unrecovered table slots,
Native RTTI, installation or binary interchangeability.

## Verification and boundaries

Project/program were checked as `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, base `00400000`. Complete assembly, actual allocator
body, both caller setups, existing schemas and the actual deleting-table provider
were read. Strict MSVC Win32 C++17 `/EHsc /W4 /WX /permissive- /O2 /FAcs /c` passes.

`local/output/compare_avoid_group_factory_bytes.py [COFF-object] [native-byte-listing]`
accepts the primary registered object. It proves a 185-byte emitted body with only
two relocations: the E8 allocator operand at offset 2Ah and table DWORD at 37h.
After masking only these verified operands, every remaining byte matches the live
185-byte native listing. Both padding LEAs and RET 4 are included.

One ignored fixture, `local/output/avoid_group_factory_probe.cpp`, uses the actual
new manager constructor, BF50 sentinel producer, current SDK lock, factory, real
group table and full manager clear/lifetime chain. Before manager clear, a real
16-byte hull node with an allocated hull-data block exercises nonempty standalone
E200: selflinks/count reset, node/data freed, sentinel/proxies/lock retained, with
no sentinel payload read. Keys 5,-7,3 become -7,3,5;
repeated queries return the same actual group pointers without count growth. One
group contains a genuine zone, pointer array and record. Full clear releases these,
retains body capacity and sentinel, queues two bodies, and allows creation again.
Typed actual manager-table calls then exercise flags 0 and 1 with explicit EDX
context; the actual publication cell clears even when it contains another pointer.
Proxy words remain unchanged. Genuine `_heapwalk` observations confirm the tracked
group/record/body/lock/head releases, retained flag-0 receiver and retained queue.
This is bounded current Source heap evidence, not a global no-leak claim.

Run `local/output/run_avoid_group_factory_probe.cmd [archive-path] [archive-only]`.
Default mode overlays the three new modules on the primary existing Release
archive; `archive-only` uses the registered archive. The probe embeds a manifest.
The worker overlay run passes. Primary owns CMake registration, full build,
registered-object/archive reproduction, annotations and refreshed exports.
There is no Native differential, original SEH/hardware-fault, installation,
original-runtime or gameplay validation claim. No new tracked test target exists.

Historical corrections: allocator CALL is `00417CC9` (the old `00417CC7` is PUSH
14h); the argument is captured early at `00417CA1`; insertion's count is captured
at `00417CFC`, while the final ADD uses the current count. Existing older records
remain available and are not silently reclassified as this complete entry.
