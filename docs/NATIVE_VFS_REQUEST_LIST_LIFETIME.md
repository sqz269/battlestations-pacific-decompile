# Native VFS request-list lifetime

These four complete logical bodies operate on the actual list and payload
allocations, using the existing actual string-pool getter/return implementation
and the matching CRT free service. Names are descriptive hypotheses. The new
C++ interface is not a drop-in binary replacement.

| Entry | End exclusive | Bytes | Coverage | Native ABI |
| --- | --- | ---: | --- | --- |
| 004D2640 | 004D265D | 29 | complete | ECX actual string-list owner; RET0 |
| 00BE1220 | 00BE12BA | 154 | complete | ECX actual20h payload; RET0 |
| 00BE19E0 | 00BE1A1E | 62 | complete | ECX actual request-list owner; RET0 |
| 00BE1D60 | 00BE1D7D | 29 | complete | ECX actual request-list owner; RET0 |

No routine has stack arguments or an established semantic EAX result. ESI
captures ECX at4D2641/BE1237/BE1D61;BE19E2 captures it in EDI. BE19E0 captures
each next node in EBX atBE1A00 before payload destruction. The complete installed
bytes, including the false-no-return gaps, establish those register lifetimes.

Producer00BE10A0 writes the base length/data pair and the two string-list
sentinels/counts at+0C/+10 and+18/+1C. Its list owner words+8/+14 retain their
contents. Producer00BE1A20 allocates28h, stores next/previous at+0/+4 and calls
00BE13E0 to construct the20h payload at+8. Manager00BE1DC0 allocates the28h
sentinel at+64 and zeros+68, identifying its list owner at+60. The public APIs
accept raw owner pointers rather than introducing another record declaration.

004D2640 calls the actual004D05E0 clear, reloads/frees owner+4, then nulls it.
00BE1220 clears/frees/nulls its+14 list followed by+8, then returns the captured
nonnull base string with its current length+1. The base header stays untouched.
00BE19E0 captures the first node, self-links the current sentinel in two separate
reloads, tests the first node against the current sentinel, then zeros count.
For each node it captures next, destroys payload+8, frees the node and compares
next against the current owner sentinel. 00BE1D60 then reloads/frees/nulls that
sentinel. No owner+0 reset, payload ownership callback, replacement allocation,
or synthetic rollback is added.

FuncInfoE00ED0 and unwind mapE00EC0 give BE1220 state1 the+8 list cleanup via
CC6808→4D2640, followed by state0's base string cleanup CC6800→41DD20. Normal
execution arms1 before destroying+14, lowers to0 before destroying+8 and lowers
to-1 after capturing base data and before returning it. The source guard
records these states. The existing actual pool bridge inherits noexcept
NativeStringStorage::release: failure in its lazily recreating getter terminates.
That existing boundary does not provide native throwing-getter or SEH identity;
no fixture here claims those paths.

| Existing service | Verified contract and source |
| --- | --- |
| 004D05E0 | Actual list clear, retained sentinel, captured next and current sentinel reloads; native_render_resource_record.cpp |
| 0041DD20 | Capture data then current length+1, leave raw header; native_string.cpp |
| 00419CC0 | No native arguments; repeat canonical getter on every string return; native_string_pool_owner.cpp |
| 00BD1510 | ECX pool; block/size/unused on stack; RET0C; native_string_pool_storage.cpp |
| 00BF65AC | Returning cdecl CRT free; caller ADD ESP4; singleton_lifetime.cpp |

The three words pushed before BE129D's getter are pending BD1510 arguments,
not getter inputs. ActualNativeStringPoolStorage is required because the existing
actual004D05E0 overload takes that concrete bridge; its other overload accepts
the older semantic SizedStoragePool. No foreign source was generalized.

The report records every live incoming CALL/JMP and its containing stored
function. The generic004D2640 callers are unwind wrappers passing various local
or embedded list owners; they do not justify a request-specific record type.
DATA xrefs are retained in the ignored evidence, not presented as call sites.
The report separates four BE1220 tail calls outside its stored body. Supported
parent repair intervals are4D2651–4D265D,BE1259–BE12BA,BE1A10–BE1A1B and
BE1D71–BE1D7D, all end exclusive. Never create new functions for these gaps.
Workers made no Ghidra changes; names are recorded in the repository ledger for
the integrating agent to apply with preserved comments and the write lock.

Verification details, byte hashes, the frozen local fixture inputs/results and
the exact build evidence are in reports/native_vfs_request_list_lifetime.json.
Strict MSVC Win32 /W4 /WX /fp:strict compilation and both existing CTests
passed. Eight focused native/source comparisons passed on their first execution:
empty and populated inputs for each routine, asymmetric nested payload lists,
actual pool return order, retained raw fields and sentinel lifetime. The1932
physically frozen inputs were unchanged after execution. No new permanent
tests were added; allocation failure, callback mutation and native exceptions
were not exercised by this fixture.
The installed executable hash is
b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6.
Each live query verified C:/Users/sqz269/bsp.gpr, /battlestationspacific.exe and
the configured8089 bridge. Static byte agreement and the focused comparisons
do not establish gameplay behavior or binary exception compatibility.
