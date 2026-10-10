# Compact resource writer discovery

The original instruction at `00CD8688` writes compact resource cell `0109042C`:
`MOV DWORD PTR [0109042C], ECX`. Typed Ghidra references, its exact live instruction
and original-file disassembly agree on this direct store. The containing function,
entrypoint and complete initializer contract remain **unresolved**. This evidence
does not make a C++ startup packet ready.

This independent packet uses published baseline `e7cb4b9c1` and the accepted
[remaining-descriptor readiness review](CC12_REMAINING_RESOURCE_DESCRIPTORS_SOURCE_READINESS.md).
Its only tracked changes are this document and its report. Root remains the sole
Ghidra writer. No camera/skin ownership investigation, C++/CMake/ledger/config/script
edit, Ghidra mutation, build, test or runtime probe was performed.

## Confirmed instruction and target

The initial lease covered `0109042C`, the two output files and ignored evidence.
After the direct write reference was returned, the lease was extended to
`00CD8688` and Root was notified before the selected instruction was opened.

| Address | Selected extent | Store | Saved live bytes |
| --- | --- | --- | --- |
| `00CD8688` | `00CD8688..00CD868D`, 6 bytes | `MOV DWORD PTR [0109042C], ECX` | `89 0D 2C 04 09 01` |

The cell-reference query returned five records under its limit of 80: reads at
`00B922C0`, `00B922E0`, `00B92EA0`, a data reference at `00B92E94`, and the write
at `00CD8688`. The first read is labeled `BSP_CompactTrackItem_GetTypeDescriptor`
by the current metadata. These other reference sites were not opened. All returned
reference text is retained without a display spill. This establishes one direct
write site; it does not exclude indirect or unresolved writers.

The saved instruction-context response uses context zero, with empty before/after
arrays. Typed live byte reading covers exactly six bytes. Original-file
`disasm-raw` covers the same six-byte window and emits one instruction. Its
mnemonic, destination and source register agree with the live instruction. Live
raw bytes are retained; this worker did not perform a separate original/live
raw-byte equality comparison.

The complete reviewed Native code scope is **one instruction start and six
selected bytes**. No adjacent instruction, guard, descriptor data window, name,
parent/counter call, branch, return, handler or complete function body was opened.
The origin of ECX is unknown. The selected window is not a function extent and
does not establish the incoming register ABI or the ordering of descriptor stores.

## Unresolved ownership and contract

`lookup 00CD8688` reports no function start there and gives the earlier
`00CD8460 BSP_GroupPool_InitializeStatic` as an enclosing candidate. That indexed
candidate is not proof of ownership and its body was not opened.

The live function query reports `No function found for 00cd8688`; the signature
query likewise reports no function at that address. Typed callers returns
`Function not found: null`. Caller information is unavailable, not a confirmed
empty caller set. The exact store-address reference query reports no references
under a limit of 80, while `docs-for 00CD8688` reports zero indexed document mentions.

These bounded address-query results leave instruction ownership and body boundaries
unestablished. They do not prove that no possible containing or discontiguous
function exists. No pseudocode, complete function disassembly, listing repair,
NoReturn change or control-flow recovery was attempted.

Original calling convention, arguments, stack cleanup, return value and incoming
register contract remain unknown. So do the actual guard, complete descriptor
extent/name, parent initializer chain, counter ownership/consumption, store order,
partial-state/repeat-call behavior and original caller or CRT-table placement.
No adjacent address, nearest function, neighboring family or resource name supplies
any of these missing facts.

## Source boundaries and target identity

The current Source header borrows `compact_type_0109042c` as a `const volatile`
reference. `B79BC0` passes that cell's address to `B87CE0` for its compact range.
The existing range header associates the original `B78750/B922C0` pair with
`0109042C`. These are consumer contracts, not an initializer implementation.

Any eventual provider must use the actual retained VFS type storage, the same
`TypeIdCounterLifetime` and common lifetime domain. The preceding readiness review's
Source composition and native-order qualifications remain intact. This packet adds
no copied ID, guessed guard, default descriptor, manufactured success or unrelated
family substitute. Compact remains distinct from camera-resource `01090288`,
skin-model `01090344`, skined-mesh-resource `01090454` and excluded skined-mesh
`01090370`. No other family's Native boundary was investigated here.

The configured project is `C:/Users/sqz269/bsp.gpr` and exists on disk. The current
typed CLI verifies live project name `bsp`, program `/battlestationspacific.exe`,
language `x86:LE:32:default` and image base `00400000` before each live query.
Three batch verification records report live/snapshot counts of 64,730. The client
does not separately compare the live project-directory path with the configured
`.gpr` path; that qualification is retained.

The report pins the current original executable identity, 12 selected CLI receipts,
the exact context JSON, and 12 frozen Source/configuration/tool/context files.
Frozen text matches the baseline Git blobs after CRLF normalization. The accepted
readiness review's Source/build/runtime references are historical context, not new
execution evidence in this packet. The report also records one rejected CLI
spelling before the successful top-level `disasm-raw`; the rejected command did
not open Native bytes.

## Next bounded packet and evidence levels

An **Astra/Root ownership-recovery packet** should start from the exact store at
`00CD8688`, establish the genuine containing function and entrypoint, and obtain
its complete physical body before making initializer or ABI claims. Root must
lease and announce each additional exact body/data/handler boundary before opening
it. Listing, control-flow, overlap or register-ABI recovery belongs to that packet;
Root alone may mutate Ghidra through the write lock with old-value preservation.

After ownership is established, recover the real guard, target extent/name,
support/parent calls, shared counter usage, store order, failure/repeat semantics,
ABI and genuine caller/CRT evidence. Only a reviewed complete provider can justify
a separate Source integration packet. This discovery does not authorize one.

Evidence is limited to direct-write metadata, one selected live instruction and
its original-file disassembly, plus current Source consumer context. Complete
initializer reconstruction, drop-in ABI compatibility, execution, startup,
runtime and gameplay equivalence remain unproved. No builds/tests/probes were
needed for these two documentation/report files.
