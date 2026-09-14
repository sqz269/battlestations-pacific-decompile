# Unit part-pointer vector storage

Addresses: `0087B460`, `0087AF40`, `008792C0`; supporting routines
`00876A80`, `008776B0`, `00878BB0`, `00878190`, `008793C0`, `008780F0`,
`008786C0`, `0087A7E0`, `00C96770`. Reused specializations: `00492210`,
`00BCFEB0`, `00BD0590`.

The179-byte resize wrapper now has a source provider over actual10h checked
storage: opaque0, begin4, end8, capacity-endC. Slots are raw DWORD pointers.
Health initialization's87B460 operation now defaults to this concrete provider;
compatible overrides remain available for existing ownership domains.

The implementation reuses the established checked-DWORD storage code. Native
87B460 and492210 are byte-identical after normalizing six CALL operands. The
85-byte allocator and105-byte length-error entry likewise match their existing
specializations after address-operand normalization. No second container or
allocation domain is introduced.

The part-pointer insertion helper differs from the DWORD helper: it uses scalar
copy/fill helpers, has an FH3 frame and publishes capacity/end/begin after old
backing release. The existing DWORD provider publishes begin/capacity/end.
An explicit publication policy preserves both. Optimized production code confirms
old stores440/445/452 and the pointer path48C/452/459 in the shared resize body.
The part entry passes policy1; the health default directly calls that entry.

This provider covers resize and its insertion-at-end use over valid, consistent,
owned storage. Added entries get the captured fill value; reallocation uses
max(requested,capacity+capacity/2) with the3FFFFFFF element bound. Shrink keeps
the backing/capacity and existing bytes. Zero/equal-size requests retain native
behavior. Allocation callbacks may throw but cannot structurally mutate the
vector. General middle insertion, malformed range repair, private-frame aliases,
arbitrary overlapping allocations and hardware-fault timing remain outside the
existing storage contract. A wrapper call's broader native domain is not inferred
from these valid-storage results.

Eight fixtures compare original machine-code resize execution with the new
provider and the existing DWORD provider: empty/no-op, zero-filled allocation,
growth with spare capacity,1.5x reallocation, shrink, clear, equal-size, and
allocated-empty growth. Counts, capacities, pointer retention, opaque fields,
live values and preserved old spare bytes match. Native stack deltas are zero.
The source length-overflow case throws the canonical owning length error before
changing storage.24 result images are retained.

The native fixture retains10 complete bodies/1,180 bytes. Allocation and free
edges bind to the existing source CRT; unreached error edges fail the fixture.
This executes the full native resize/insertion/erase bodies on supported cases,
not all branches of the general insertion routine. Native FH3 is untested.
Fifteen live/disk spans and three normalized comparisons are retained. Two
post-free gaps in87AF40 were repaired; its FH3 selectorC96770 was defined from
matching10-byte evidence. Confirmed annotations are saved separately.

Win32 and both existing CTests pass. Exact source, production objects, linked
libraries, compiler-discovered headers, toolchain, cloned/relocated bytes and
outputs are retained under `local/part_vector_ab`. No new repository tests were
added. Actual unit/world bindings, remaining whole providers and gameplay
execution remain unproved. Native and source interfaces differ.

Call-ownership review checks26 direct rows. The free call87B0A5 belongs to
Catch_All@87B0A1, separately from the surrounding insertion function. The
decoded rethrow87B0B1..87B0B5 remains outside its truncated stored body and is
recorded separately. The flow tool refuses to extend a tail ending in a
no-return throw; no function was recreated and no no-return flags were changed.
