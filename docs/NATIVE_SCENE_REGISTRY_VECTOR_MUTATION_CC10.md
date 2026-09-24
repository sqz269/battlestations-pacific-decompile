# Raw scene-registry pair-vector mutation, CC10

Addresses: `00B82BF0`, `00B82FD0`, `00B82C90`, `00B82A90`, `00B824F0`,
`00B83137`, `00B83143`, `00CC2340`.

The source operates on actual vector storage: opaque word at +0, begin +4,
end +8, capacity end +C; each element is two raw DWORDs. It reuses the genuine
raw B821C0/B82570/B82B80 transfer providers and B81B90 allocation provider.
There is no host SceneNodeRegistry/STL vector, owner array, retain or release.
The new public API is source composition, not a drop-in native binary ABI.

| Entry | Exact extent | ABI | Coverage |
|---|---|---|---|
| B82BF0 erase | [B82BF0,B82C5A),106B | ECX vector, five stack words, EAX result pair, RET14h | complete body including returning validation |
| B82FD0 repeated insert | [B82FD0,B83211),577B | ECX vector, four stack words, RET10h | complete normal body; explicit source C++ exception projection |
| B82C90 forward-copy wrapper | [B82C90,B82CB5),37B | ECX preimage, three stack words, EAX advanced destination, RET0Ch | complete exact assembly |
| B82A90 backward pair move | [B82A90,B82ACE),62B | cdecl three words, EAX advanced destination, RET | complete exact assembly |
| B824F0 repeated assignment | [B824F0,B82514),36B | cdecl three words, RET | complete exact assembly |
| B83137 catch | [B83137,B8314C),21B | parent EBP, free current +14 then rethrow | compiler boundary, source catch projection |
| B83143 catch | [B83143,B8314C),9B | parent EBP, rethrow | compiler boundary; shares suffix with B83137 |
| CC2340 handler | [CC2340,CC234A),10B | EAX=DFB620, JMP BF6B43 | native FH3 boundary only |

The 577-byte envelope contains the catch ranges. They are not separate disjoint
code allocations or extra 30 bytes of implementation. B83137 is currently a
Ghidra function only through B8313F. B83143 and CC2340 have no function at this
freeze. Source calls and report rows retain instruction identity separately
from missing-function annotations. The root owns later repair/definitions.

## Caller cells and read points

`NativeSceneRegistryVectorEraseArguments` is five volatile words mapping native
entry ESP+4,+8,+C,+10,+14: result, first owner, first position, last owner, last
position. B82BF1 captures first owner. B82BFD reads last owner only if the first
is nonzero; B82C03 invokes the required genuine returning BF6713 service when
the test fails. B82C08/C0C then read current positions. A finite accessible
range in 8-byte steps is required after a returning callback; the native has
no additional range or owner/vector identity check. B82C30 reads/stores first
component before B82C35 reads/stores second. Only a nonempty compaction loop
reaches B82C43, reloading CURRENT first owner. If last already equals vector
end, the captured original owner survives. B82C47 publishes captured new end;
B82C4B captures CURRENT result pointer, then writes position at +4 before owner
at +0. No allocation, element destruction, shrinking or owner credit occurs.

`NativeSceneRegistryInsertFrame` supplies these initialized address-stable cells:

| Source offset | Native slot | Writes/reads |
|---|---|---|
| 00/04 | EBP-1C/-18 | B82FEE reads source+4 before B82FF5 source+0; B83000/003 snapshot stores after B82FF7 begin capture |
| 08 | EBP-14 | B830A6 zeroes LOW BYTE only after allocation; B830AA reads current full word |
| 0C | EBP+8 | unused iterator owner; untouched |
| 10 | EBP+C | current position at B830A0 after allocation or B8314F in-place; B830D8 zeroes LOW BYTE on growth only, B830DC reads full word |
| 14 | EBP+10 | initial count B83014; current after allocation/copy B830AE/C5/E0, after tail copy B83100; short-tail B8317C; long-tail B831D5 writes source pointer and B831DD rereads it |
| 18 | EBP+14 | initial pair pointer B82FEB, overwritten B830B6 with allocation, B83164/CF with byte count; read B83112 AFTER free, B83196/EB after transfers; catch B83137 reads CURRENT word |

This source layout deliberately omits saved-register, return-address, SEH,
native state and ESP-save gaps. It does not claim a native private-stack ABI.
The source cells may be aliased or changed by genuine bindings subject to the
same accessible ranges; metadata/context storage is disjoint. Transient nested
helper stack arguments are typed snapshots, and arbitrary aliases into those
unexposed native frames are excluded. Native wrapping 32-bit address/count
arithmetic and arithmetic right shifts are retained; no typed float/key map
or pointer subtraction across C++ objects is substituted.

Insert reads both pair words even for zero count, stores the snapshot, then
returns. Initial begin is captured before the snapshot stores. Capacity and
each preallocation size calculation use that captured begin and current end.
Length failure uses required genuine nonreturning B82DD0; there is no success
fallback. Growth capacity is max(required, old+old/2) subject to the native
1FFFFFFF overflow decision. B81B90 uses the actual current allocator slot.

Growth captures CURRENT position and begin after allocation, overwrites only
the local low byte, copies prefix, fills snapshot pairs using CURRENT count,
captures CURRENT end, overwrites only position's low byte, then copies suffix
from the captured position. After transfer it captures current old begin/size
and current count before freeing. After free it rereads CURRENT buffer word,
then stores capacity, end, begin in that order. No allocation rollback is added
outside the native state0 catch, and no earlier allocation is substituted for
the current buffer word. A provider's observable mutations remain observable.

With enough capacity, short suffix (< count) copies suffix to position+bytes,
fills the new tail, adds CURRENT byte-count cell to CURRENT end, reloads end,
then assigns the old suffix range. Long suffix copies the last count pairs to
end, rereads the overwritten count/pointer cell, publishes returned new end,
moves the middle backward, then rereads CURRENT bytes and assigns inserted
pairs. All helpers read/store word0 before reading word1; neither memmove nor
a whole-pair snapshot replaces these overlap semantics. B82C90 retains its
incoming-ECX local preimage/low-byte write and native argument-load schedule.

## Services and exception boundary

Required bindings are the existing actual BF681B allocator, BF65AC free,
nonreturning B82DD0 length failure, plus a current genuine BF6713 invalid-
parameter service for erase. The probe binds the existing source CRT
`_invalid_parameter_noinfo` and installs a real returning CRT handler; production
does not supply a default no-op, assertion-only handler, or forced noreturn.
This is library/error transport, not a missing game algorithm. Allocator/free
domain, argument cells and borrowed binding slots must outlive callbacks.

CC2340 loads descriptor DFB620: magic19930522, maxstate4, unwind map DFB600 with
four {-1,0} entries, two try blocks at DFB644. Try state0..0/catchhigh1 uses
catchall DFB5F0->B83137, freeing CURRENT EBP+14 then rethrowing through BF6885.
Try state2..2/catchhigh3 uses DFB5E0->B83143, rethrow only. Both catches share
the instructions PUSH0,PUSH0,CALL BF6885 at B83143..4B. Native states1/3 are
catch states; no source-owned rollback/owner registration is invented.

The source uses C++ catch/free/rethrow for the growth region. Pure raw transfer
helpers are noexcept for valid memory, and native FH3/SEH fault unwind has NOT
been exercised or reproduced. The source does not claim original CRT RTTI,
FH3 registration records, fault-to-C++ conversion, or private stack identity.

Ghidra still omits B8310F..11 (ADD ESP,4 after normal free B8310A) and
B83140..4B (ADD ESP,4; PUSH0; PUSH0; CALL BF6885 after catch free B8313B).
The PE/live bytes prove both continuations. Root must clear each returning-free
CALL_RETURN override before repair/recreation; recreating alone is insufficient.

## Validation

All 828 unique owned code bytes, 140 EH metadata bytes, and 138 reused transfer
bytes match the original PE and live Ghidra reads. The report records every
direct call/tail target and exact missing-function/last-instruction boundaries.
Worker made no Ghidra writes. Strict MSVC Win32 Release build and all three
existing CTests pass. No tracked test or game path was added.

One ignored original/source composition probe copies eight complete normal
bodies and relocates external calls to the actual transfer/allocation/CRT
providers. Its sequence covers both in-capacity insert paths, a pair source
overlapping moved data, growth allocator mutation of count/position/local
upper bytes, free-time current-buffer replacement with another genuine owned
allocation, and both conditional erase-owner paths after real returning CRT
validation. It compares actual vector/output/argument cells and payload words.
Original native exception transport is not reached. Original position words
whose LOW BYTE was cleared are compared by that byte; their remaining raw
address bits are allocation-location dependent, not falsely claimed equal.
Source and original current-buffer publication, captured capacity/new length,
and source-word overlap semantics are compared. This is finite fixture and
source-contract evidence, not whole-registry attachment or gameplay validation.
