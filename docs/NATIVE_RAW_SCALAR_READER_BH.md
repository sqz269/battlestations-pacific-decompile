# Qualified raw scalar readers BH

Six complete source bodies within the established memory D642C0 and physical
D691B0 profile domain are implemented in `src/native_raw_scalar_reader.cpp`.
All six public interfaces are explicitly context-bearing cdecl source functions;
none is an original ABI thunk. The existing raw memory BEF590 leaf is reused.
The physical BF5030 source retains its existing context-bearing interface.

| Original | Coverage | Original ABI | Source operation |
| --- | --- | --- | --- |
| BE9A00 | complete within qualified domain | ECX wrapper, EAX, RET | actual wrapper/node+8 reader/node+20 budget; DWORD adapter |
| BE99D0 | complete within qualified domain | ECX wrapper, ST0, RET | same raw loads; float adapter |
| BF0280 | complete within qualified domain | ECX reader, stack budget, EAX, RET4 | reload stream+0/current slot34; nonnull count; wrapping debit |
| BF02C0 | complete within qualified domain | ECX reader, stack budget, ST0, RET4 | current slot44; nonnull count; FSTP32/FLD32; wrapping debit |
| BE42E0 | complete within qualified domain | ECX stream, stack optional count, EAX, RET4 | current slot24; one four-byte read into incoming count argument slot |
| BE4360 | complete within qualified domain | ECX stream, stack optional count, ST0, RET4 | same read; FLD32 from incoming count argument slot |

The explicit source stack frames prevent compiler-added float conversions and
preserve the native scalar's unusual argument-slot reuse. The optional count
pointer is copied before its incoming slot becomes the destination buffer.
Unwritten bytes consequently retain that pointer's numeric bits. No independent
zero-fill is added. Both budget adapters pass nonnull pointers to actual-count
locals. The DWORD local initially holds the reader pointer, matching PUSH ECX;
the float count local is uninitialized before the provider writes it. Normal
returns from both qualified providers publish count. Each adapter then performs
one unchecked DWORD SUB, including wrap below zero; neither retries or rejects
short reads. Float retains BE4360 FLD32, then BF02C0 FSTP32, budget-address load,
FLD32, count load, SUB. No FP control word is changed.

The raw wrapper/node loads have no null, leaf, attachment, readiness or ownership
checks. Unknown current profiles or slots throw an explicit source-domain
exception; this is not represented as recovered native read-error behavior.
For valid selected profiles the corresponding existing context pointer must be
nonnull. The unselected context may be null. Arbitrary profiles are outside the
implementation domain, and numeric original vtable words are never called as
host function pointers.

Every native virtual site reloads the current owner profile and slot. Memory
uses the actual profile words already borrowed by
`NativeRetainedMemoryOwnerContext`, qualifying slot24 BEF590 before invoking
`native_memory_stream_read_00bef590`. Physical loads actual D691B0 table memory,
qualifies slot24 BF5030, and invokes `read_native_physical_stream_00bf5030` with
`NativePhysicalStreamOpenContext`. That existing provider performs ReadFile and
the actual current manager failure service; no injected read callback or fake
success service is introduced. Adapter slots34/44 are separately qualified as
BE42E0/BE4360. No existing provider source is changed.

Current PE and live Ghidra bytes match the discovery's eight complete inspected
bodies and both complete4Ch profile prefixes. Every live CLI batch verifies
project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe` on8089.
The installed PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The source report retains the discovery's111 direct inbound call rows and six
virtual rows. The call checker verifies body/call attribution; the six virtual
targets are established separately by the retained profile bytes.

Validation and artifact bindings are recorded in the matching JSON report:
strict Win32 build with `/W4 /WX /fp:strict`, verify-seeds, both existing CTests,
one focused local short-read/debit probe, current compiler command/read/write
tlogs, and unique archive-member equality with the compiler output object.
The one probe uses the real memory read source with a one-byte payload and a
zero budget. It observes the source adapter's actual count-local pointer bits,
checks unchanged high24 output bits and low byte A7, verifies cursor advance1,
and checks budgetFFFFFFFF. This is a source fixture, not an original-native
stack-address differential result. Full exact original caller-stack-address
parity is not claimed. Source/object inspection establishes the x87 schedule;
the probe does not establish every signaling-NaN/FP-exception outcome.

The first strict compilation rejected helper parameter `offset`, an assembler
keyword. The failed source, input hashes and build log remain local; the helper
parameter was renamed `byte_offset` before the successful rebuild. Deferred
CMake registration adds only this source. No new permanent test suite, Ghidra
mutation, original ABI replacement or gameplay validation is claimed.

Full B7EB90 remains blocked on the separate raw node construction, strings,
detach and intrusive lifetime dependencies. This module only closes the
qualified scalar source boundary identified by
`NATIVE_RAW_SCALAR_READER_FRONTIER_BH.md`. The report inventories every local
file with size/SHA256/SHA512 in two identical passes outside the local tree;
failed attempts and scripts are retained.
