# Native SAP processing and complete runtime table, R137

Addresses: 00c36c30, 00c36e20, 00c4be10, 00c4bd60, 00c4c2a0, 00c4c270, 00c4c320, 00c40140, 00c54aa0, 00c4c380, 00c32b30, 00c32b10, 00c32af0, 004043d0

## Result

Eight complete normal bodies (3,911 bytes) supply endpoint allocation, insertion,
movement, pair decisions, incremental insertion, proxy updates, dispatch and the
separate batch path. `NativeDynSapRuntime` supplies all eight callable Win32 SAP
table entries, using the existing concrete proxy constructor, pair services and
lifetime routines. Its stable first-member table borrows the allocator context,
call service and progress record; all must outlive the managers using it.

This supplies the SAP class contract for a caller's runtime context. It does not
install that context into an ordinary application run or prove full-game behavior.

## Recovered bodies

| Entry | Bytes | Original inputs and cleanup | Contract |
| --- | ---: | --- | --- |
| C36C30 | 433 | ECX manager, stack proxy/axis, RET8 | Allocate and link two axis endpoints; publish tagged minimum and plain maximum references. |
| C36E20 | 225 | EAX proxy, ECX axis, stack manager, RET4 | Load bounds into endpoints and insert each by walking backward. |
| C4BE10 | 1,115 | Stack manager/proxy/axis, RET0C | Move minimum and maximum in both directions, with pair creation/removal at crossings. |
| C4BD60 | 166 | CL axis, EDX manager, EAX peer, stack first/add byte, RET8 | Exclude static/static pairs, test other two axes, then create or erase. |
| C4C2A0 | 115 | ESI manager, RET | Insert queued proxies, reloading queue count; clear pending count. |
| C4C270 | 46 | ECX manager, stack proxy, RET4 | Update all three axes only if proxy+38 is inserted. |
| C4C320 | 86 | ECX manager, RET | Unsigned pending count >50 tail-dispatches to batch; otherwise insert then update dynamic proxies. |
| C40140 | 1,725 | ECX manager, RET | Sort and merge queued endpoints, recycle old pairs, rebuild overlaps. |

### Endpoint storage and movement

Each axis uses a 34h pool at manager+4+axis*34h, with 1,000 sixteen-byte slots per
page. C36C30 performs two distinct allocations, preserving each native allocation
and free site. It publishes a new free head before page-vector growth, retains
`2*capacity+2`, current count/base reloads, free-before-publish order, and untouched
payload bytes. The lower endpoint's tag is `proxy|1`; the upper tag is `proxy`.
The proxy receives their addresses at +20/+2C, indexed by axis.

Insertion and movement retain the recovered x87 instruction schedule, float
spills, SSE scalar moves, pointer rewrites and strict/inclusive branch choices.
C4BD60 computes the other axes with `(1<<axis)&3` and repeats that operation for
the remaining axis. C4BE10 also inlines these overlap decisions in several walks.
It invokes the established R136 creation/removal code through explicit context
adapters. No global allocator or thread-local context is introduced.

C4C2A0 allocates three endpoint pairs, inserts axes 0 and 1, updates axis 2 to
create pairs, marks proxy+38, and finally clears manager+240. Its loop reloads the
pending count. C4C320 subsequently walks the dynamic list from +130 to +134 via
proxy+4C; static proxies can be updated explicitly through C4C270.

### Batch processing

C40140 retains its own algorithm. It allocates two temporary arrays of
`pending*16+8` bytes: stack storage at sizes <=400h, otherwise heap storage.
The normal markers are DWORD 0xCCCC and 0xDDDD, respectively. At each axis it
builds two tagged records per pending proxy, performs the original four-pass
radix transform on float bits using a 1000h-byte histogram, and merges the
resulting endpoints into existing lists. It marks queued proxies inserted.

After freeing heap temporaries and clearing pending count, it recycles the entire
active pair chain, resets the sentinels/count, and scans the first axis to rebuild
pairs with bounds checks and static/static exclusion. Existing inserted endpoints
remain in their pools. The original RDTSC and private scratch stores are retained.

The source uses the toolchain's normal `_alloca_probe` and `_alloca_probe_16`
services, and standard memset. The native fixture executes the copied original
43-byte and 22-byte stack helpers; memset is a shared normal library contract.
No new CRT implementation or native exception machinery is claimed.

## Complete table

The eight source slots are create proxy, remove proxy, update proxy, process,
count, first, next and scalar delete. Fastcall thunks provide the original ECX
receiver and stack cleanup expected by Win32 thiscall callers. Create consumes
body/bounds/static arguments and preserves the static argument's low-byte test.
Scalar delete captures the runtime owner before the destructor overwrites the
manager's table pointer with its base profile. No placeholder or copied-original
addresses are present in the production table; native RTTI metadata is absent.

## Evidence and validation

Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. All 6,063
collected live bytes match the original PE: 3,911 new bytes, 1,858 existing
pair/lifetime bytes, 262 CRT reference bytes and 32 table-data bytes. Five false
no-return continuations at C36CBD, C36D7F, C4057F, C40688 and C406A1 were repaired
under the write lock, restoring fifteen bytes. Remaining listing gaps are
non-call alignment padding. Exact direct CALL rows are verified before commit;
the C4C32D tail edge to C40140 is recorded separately.

Strict MSVC Win32 and all three existing CTests pass. An ignored comparison
fixture invokes all eight production table slots and compares against relocated
original processing/pair/lifetime bodies. **34 paired runs match 280,433,704
bytes**, **49,706 allocator events**, and **342 full snapshots**. Only the
manager's table-identity word is normalized; arena storage, endpoint and pair
links, allocation sizes/order/sites, stale and freed bytes are compared exactly.
Snapshots also compare x87 control/status/tag and MXCSR, excluding instruction
pointers. Every allocation is released by each run's end.

Coverage includes 0/1/3/50/51/63/64/501/1001 pending counts, with existing inserted
proxies before the batch cases; the 50/51 dispatch and 63/64 stack/heap boundaries;
three endpoint pages and page-vector growth; and a separate 1,001-proxy sequence
in chunks of at most 50 to exercise incremental endpoint-page growth. It includes
queued update no-ops, movement in both directions on all axes, mixed static and
dynamic proxies, pair recycling, enumeration and inserted proxy/scalar cleanup.
Small incremental and batch cases run under all twelve x87 precision/rounding
combinations, with masked exceptions and default MXCSR.

The fixture initially asserted that every endpoint list remained sorted after
large coordinate jumps. That assertion failed against the original executable
(for example, the 50-proxy case reached adjacent x values 1936 then 1928).
The final fixture retains those inputs, verifies list structure/counts, and
records ordering inversions. Original and source match those inversions and all
raw links; no mathematical sorting guarantee is inferred for arbitrary teleports.

## Limits and follow-up

Both sides share the existing manager/proxy constructors. Slot 0 is invoked
through the production table and compared with that existing source service;
this is not new independent native C54AA0 constructor proof. Body records are
borrowed storage unused by these operations. Normal reverse destruction of three
endpoint pools remains a controlled BF7C6E contract from R133.

Successful allocations, valid axis/storage and serial calls are required.
Callback-driven mutation, private-stack aliases, arbitrary malformed inputs,
allocation failure, native register/FH3/SEH/RTTI ABI and unmasked floating-point
traps are unproved. Normal two-endpoint allocations keep pool parity even; the
second-endpoint page-exhaustion branch is retained but not separately forced.
No new gameplay or ordinary application runtime claim follows from this fixture.

The next integration step is to supply this stable table owner together with the
complete convex table to the application's actual physics context, then recover
remaining class/allocator dependencies and test the admitted runtime path.
See `reports/native_dyn_sap_processing_r137.json` and its flow companion for
byte/call evidence, annotation provenance, build inputs and sealed artifacts.
