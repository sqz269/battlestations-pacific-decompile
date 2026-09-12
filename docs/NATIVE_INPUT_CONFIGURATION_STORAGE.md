# Shared input configuration storage

Addresses: `00697BD0`, `006977F0`, `00A917E0`.

`NativeInputConfigurationStorage` supplies the two checked-vector calls required by
`NativeInputConfigurationCleanupContext`. It owns no storage or lifetime. Construct
one and pass it as that context's `storage` reference; the configuration and action
owner remain the caller's actual allocations. No Lua owner, default provider, SDK
service, or parallel container is introduced.

The implementation extracts the existing private DWORD-vector assignment, clear,
and range-erasure operations from `native_input_class_configuration.cpp`.
`A917E0` now forwards to the same helpers. Its requested-count write, captured
ranges, accepted-ID assignment, wildcard append, and allocation domain are unchanged.
These are stateless source storage contracts for recognized library operations;
they are not newly ported STL implementations or original binary ABI replacements.

| Native operation | Original interface | Source coverage |
| --- | --- | --- |
| `697BD0` assignment | ECX destination actual10h header; one source-header stack argument; EAX destination; RET4 | Same-header no-op, empty-source clear, existing-size copy, retained-capacity growth and free-before-allocation paths on valid owning vectors |
| `6977F0` erase | ECX actual10h header; five stack DWORDs: output8h, first owner, first position, last owner, last position; EAX output; RET14h | Returning owner validation, captured range, current-end suffix movement and actual iterator result on valid repaired ranges |
| `A917E0` class configuration | ECX actualF8h backend; class, requested count and optional actual10h ID header on stack; RET0Ch; no semantic return | Existing complete game schedule, now using extracted helpers |

The header is `{opaque, begin, end, capacity}` at offsets `0/4/8/C`. Its producer
and allocation contracts are the same ones already used by raw backend class
configuration and the configuration's checked10h rows. Assignment preserves the
destination's opaque word. Erase does not free storage or change capacity. Its
result writes position at output+4 **before** owner at output+0; if that explicit
output aliases a header, those addressed words are overwritten as in the native
body. The owner pair need only be nonnull and equal to each other; neither is
required to equal the receiver.

The captured end survives returning CRT validation. Begin and current end are
loaded again at their original later points. Assignment frees old storage before
replacement allocation, resets the destination triplet before the allocator can
throw, and reads source endpoints and destination begin after allocation. It uses
the existing `singleton_lifetime_allocate/free` and length-error services.

## Equivalence evidence

Live bytes matched the installed image. Masking only the four displacement bytes
of verified direct E8 CALLs leaves these pairs byte-for-byte identical:

| Pair | Bytes | Purpose |
| --- | ---: | --- |
| `697BD0` / `4F60E0` | 293 | Assignment schedule already implemented privately for class IDs |
| `697890` / `468AF0` | 57 | Captured-range checked clear |
| `557200` / `441B20` | 71 | Triplet reset, size check and allocation publication |

The assignment helper's extracted text is unchanged. The erase helper is split
only to expose the original owner-pair validation and output result independently
of the existing same-header clear caller. `697080` and `6977F0` use forward DWORD
loops; the existing source contracts and counterpart library instances use
`memmove_s`. Their values and endpoints agree for distinct valid owning vectors
and erase ranges `first <= last <= current end`. Independently owning overlapping
vectors, malformed ranges, null placement destinations, arbitrary aliases into
the original call stack, and original CRT/FH3/SEH exception identity are not claimed
equivalent. No new implementation attempts to define these unsupported cases.

`697BD0` ends at RET4 `697CF2..697CF4` (exclusive `697CF5`). Its export omits
`697CB4..697CB6`, but live/image bytes prove `83 C4 04` (ADD ESP4 after free), also
present in counterpart `4F61C4..4F61C6`. `6977F0` ends at RET14h
`697850..697852` (exclusive `697853`); `A917E0` ends at RET0Ch
`A91897..A91899` (exclusive `A9189A`). No missing starts, new native names, or
Ghidra mutations are introduced. Library names are retained.

## Validation

Win32 Release build, both existing CTests and all eight seed comparisons passed.
The unchanged class-configuration fixture passed returning CRT repair, captured
erase suffix, wildcard and explicit IDs, capacity reuse, self-alias clear and
zero remaining class buffers.

The existing nonempty cleanup differential now forwards both library calls to
the concrete adapter: two cleanup passes, two assignments, eight erases, two
listener releases, one publication change and three nested headers cleared. Its
four native spans are retained. One additional native `6977F0` span checks a
returning CRT repair, unequal iterator owners and an output that aliases its
header; source and native snapshots agree. There are five spans, 24 direct-call
relocations and one IAT relocation. These ignored manifested fixtures use the
combined archive and do not exercise SDK, window, polling, force or gameplay.

Receipts and exact artifact hashes are in `reports/native_input_configuration_storage.json`.
Ignored runners are `local/run_native_input_class_configuration_probe.ps1` and
`local/run_native_input_storage_probe_ag.ps1`; both accept `-PrimaryWorktree` for
strict integrated-header/archive validation. The latter verifies captured native
bytes against the installed image before compilation.
