# Qualified raw node detach BI

BE9C40 and BF03E0 now have complete source within the established D642C0 memory
and D691B0 physical stream domain. The two public interfaces in
`native_raw_node_detach.hpp` add an explicit borrowed
`NativeRetainedMemoryOwnerContext`; they are not original ABI thunks. Existing
seek providers and their source files are unchanged. Baseline:3d6f645b.

| Original body | Coverage | Original ABI | New interface result |
| --- | --- | --- | --- |
| BE9C40..BE9C85 | complete within qualified domain | ECX raw wrapper, no stacked arguments, RET, final EAX reader | actual reader pointer |
| BF03E0..BF03F4 | complete within qualified domain | ECX reader, low distance and ignored pointer stacked, RET8 | forwarded raw provider EAX DWORD |
| Existing BEF540..BEF57C | reused complete provider | ECX stream, low/high/origin stacked, RET0C | old selected base in EAX; raw source is declared void |
| Existing BF4F20..BF4F3F | reused complete provider | ECX stream, low/high/origin stacked, RET0C | full SetFilePointerEx BOOL in EAX |

Detach loads node=[wrapper] and parent=[node+0C]. For a nonnull parent it first
subtracts child's declared+1C from parent's remaining+20 using a wrapping DWORD
SUB. It then reads child remaining+20. Only when nonzero does it read reader+8
and call the reader adapter with remaining and an ignored zero-local pointer.
The adapter reloads stream=[reader] and the current profile/slot1C and seeks
(low=remaining, high=0, origin=1). It never reads or writes the extra pointer.

Detach discards the returned value. After normal seek return it writes own
remaining=0, including when the physical result is FALSE. It then reloads
reader=node+8, decrements reader+60, clears node+8 and returns that reader.
The initially-zero remaining branch still performs parent accounting and the
path/attachment updates. No null-node/attached/extent/duplicate guard, rollback,
reference release, allocator, parent clearing or path-cell erasure is added.
The raw memory helpers use explicit loads/stores/SUB so these operations retain
their order and wrap behavior. Faults do not trigger invented recovery.

Current memory profile D642C0 and its slot1C BEF540 are read from the existing
borrowed profile context. The established
`src/native_filestore_open.cpp:native_memory_stream_seek_00bef540` is called
through its source symbol, preserving unchecked low-DWORD cursor addition and
ignored high DWORD. Its native EAX is captured explicitly, rather than replaced
with a Boolean. On this relative path it is the previous absolute cursor.
The helper's known source function pointer is not an injected provider.

For current physical D691B0, the slot1C BF4F20 word is read at its actual numeric
table address, then the existing
`src/native_physical_stream_conversion.cpp:seek_native_physical_stream_00bf4f20`
is called. It returns all32 bits of SetFilePointerEx BOOL and writes the OS
position directly to actual stream+10. No extra physical service context,
success normalization, manual position update or failure translation is added.
Unsupported profile/slot identities are outside the source input domain, as
expressed by the same assumption style as the existing memory dispatcher;
there is no safe fallback. Numeric original table words are never called.

The compiled object is inspected for current profile/slot reads, the memory
source call and EAX capture, no result test after BF03E0's call, and the ordered
parent/remaining/path/attachment stores. This operation deliberately remains
separate from BE9DF0's guarded no-seek destructor accounting branch.

The matching report records strict Win32 `scripts/build.ps1` validation with
`/W4 /WX /fp:strict`, verify-seeds and both existing CTests. One focused local
case calls the new detach source with declared8, remaining3, parent remaining10
and a real raw memory stream. It checks parent remaining2, relative cursor
advance3, child remaining0, path index0, cleared attachment and returned reader,
while preserving declared length, parent, wrapper and an unrelated reader word.
This is source-fixture evidence, not an original-game or physical-failure test.
The ad hoc probe is compiled with C++17 strict flags and `/MANIFEST:EMBED`.

The report binds current compiler command/read/write tlogs to the source,
header and object, and checks that exactly one archive member equals the
compiler object byte-for-byte. Where MSBuild groups write records for a whole
compiler invocation, the complete group is retained and source membership plus
the unique output member are explicitly distinguished from single-source
command/read records. Input hashes are captured before source compilation and
rechecked after it. Every local file is inventoried twice with sizes/SHA256/
SHA512 outside the local tree, including scripts, logs and any failed attempts.

The first strict compilation rejected helper parameter `low`, an inline
assembler keyword. Its source, initial input hashes and full build log are
retained. Renaming it `distance_low` resolved compilation; the corrected input
hashes precede the successful incremental build.

Seven complete spans still match the installed PE and live verified Ghidra:
both reconstructed bodies, both providers, both4Ch profile prefixes and the
SetFilePointerEx IAT word. The call report retains all19 inbound detach calls,
the adapter call and two separately profile-qualified virtual rows, plus the
independently identified OS import. No Ghidra mutation, original ABI replacement,
physical failure runtime or gameplay validation is claimed. Other hierarchy
producer dependencies remain separate.
