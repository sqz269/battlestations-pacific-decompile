# Raw scalar reader frontier BH

Discovery against frozen candidate e55c2a254e08d8593ce868ddc07af66d09c57c47.
The six scalar bodies are ready for a bounded source packet qualified to the
verified D642C0 memory and D691B0 physical profiles. They are not implemented by
this discovery. Full B7EB90 hierarchy production remains blocked by its separate
raw node construction, string, detach and intrusive-lifetime dependencies.

| Routine | Coverage | Native contract |
| --- | --- | --- |
| BE9A00 | complete | ECX actual wrapper; load node=[ECX], reader=[node+8]; pass &node+20 to BF0280; EAX DWORD; RET |
| BE99D0 | complete | Same loads and budget address; call BF02C0; ST0 float; RET |
| BF0280 | complete | ECX actual reader whose first word is stream; stack budget pointer; current stream slot34; EAX result preserved; RET4 |
| BF02C0 | complete | Same reader/budget inputs; current stream slot44; FSTP32 then FLD32; ST0; RET4 |
| BE42E0 | complete | ECX stream; stack optional actual-count pointer; current slot24(destination,4,original pointer); EAX result; RET4 |
| BE4360 | complete | Same stream/stack inputs and slot24 call; FLD32 result to ST0; RET4 |
| BEF590 | complete inspected existing source | ECX memory owner; destination/requested/optional actual on stack; RET0C |
| BF5030 | complete inspected existing source | ECX physical stream; same three stack arguments; EAX actual; RET0C |

Neither node wrapper accepts a null node, checks attachment, initializes output,
checks remaining bytes, nor provides a Boolean failure result. BF0280 reserves
its actual-count local with PUSH ECX (initial reader-address bits). BF02C0
reserves eight uninitialized bytes; its low DWORD is actual-count and its high
DWORD is the float temporary. Both pass a nonnull pointer to their count local.
After the virtual returns they subtract its reported count from *budget with
ordinary wrapping DWORD SUB, even if count exceeds budget. Neither checks a
read status or retries. A provider which fails to write count leaves the
adapter's previous local bits; the two verified providers publish count when
they return normally.

The concrete scalar bodies copy the incoming actual-count pointer into EDX,
then use the incoming pointer's own stack argument slot as the four-byte output
buffer. They forward the original pointer unchanged to slot24. Therefore
unwritten short-read bytes retain the numeric address bits of that pointer.
EOF leaves all four pointer bits, not zero. Preserve this seed even when
introducing a C++ interface; the chosen stack location is observable on short
reads. Exact native-stack-address parity requires original ABI/stack placement,
not merely a local initialized to some other pointer. Float conversion is x87
FLD32, followed in BF02C0 by FSTP32/FLD32. No FP control word is changed; exception
and signaling-NaN effects cannot be replaced by a promised bitwise memcpy result.

The complete 4Ch prefixes of actual profiles D642C0 and D691B0 were compared
with the installed PE and live Ghidra. Both slot34 words are BE42E0 and both
slot44 words are BE4360. Slot24 is BEF590 for memory and BF5030 for physical.
The memory body unsigned-clamps requested to end-minus-cursor, copies only that
count (special four/two-byte paths, otherwise BF7680), advances cursor, and
publishes optional count. Current source is `src/native_memory_stream.cpp`,
`native_memory_stream_read_00bef590`, with existing qualified
`dispatch_native_memory_stream_read` and `NativeRetainedMemoryOwnerContext`.
Its general copy uses the previously established CRT memmove binding.

The physical body initializes its local count to zero, makes one ReadFile,
and on failure calls BD9E30 with the current manager and field18. If that service
returns, it advances cached64 position and publishes count. Reuse
`read_native_physical_stream_00bf5030` in `src/native_physical_stream_open.cpp`
and `NativePhysicalStreamOpenContext`; this already invokes the actual manager
field90 failure service. It is a new C++ context-bearing interface, not a raw
RET0C callable vtable replacement. Error callback internals and BF7680 were not
reconstructed in BH; they remain existing provider contracts.

`src/stream_scalars.cpp` zero-initializes a typed MemoryStream scalar and has no
nonnull count parameter. `StructuredNode::read_u32/read_float` in
`src/structured_reader.cpp` add leaf/readiness and full-read rejection. Neither
is a substitute. Original profile identity words are not callable host vtables.
An explicit dispatcher must reload current owner/profile/slot words, qualify
the two established profiles, and route their raw providers. Arbitrary profiles
remain out of domain; no generic injected read stub establishes provenance.

Next source packet: own BE9A00, BE99D0, BF0280, BF02C0, BE42E0, BE4360 and new
`include/bsp/native_raw_scalar_reader.hpp`, `src/native_raw_scalar_reader.cpp`,
its evidence doc/report and owned ledger shards. Coordinate CMake integration
through primary. Reuse the existing provider files without taking their
ownership. Supply explicit existing memory/physical contexts; preserve nonnull
actual-count pointer seed, one raw read, modulo debit, native float round trip,
and invalid raw-node behavior. Separate complete native ABI leaves from
context-bearing qualified source dispatch. A minimal short-read differential
case should observe destination tail and debit, not only a full four-byte read.

The report carries every discovered inbound direct call to the four packet
entries, containing body ranges, local argument/return evidence, and the four
resolved virtual edges (six rows across the two raw providers). Initial default
xrefs stopped at25; the retained complete rerun requests1000 and supersedes
that incomplete capture. Live proto establishes each call's containing body.
Byte parity covers complete eight inspected bodies and both profile prefixes,
not every caller body. No Ghidra mutation, C++ change, build, test execution or
gameplay validation was performed. All local evidence, scripts and failure
history are retained; the report inventories the whole local tree twice with
SHA256/SHA512 and sizes, outside that tree to avoid self-hashing.
