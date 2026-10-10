# Skin-model writer discovery

The original instruction at `00CD8524` writes skin-model cell `01090344`:
`MOV DWORD PTR [01090344], ECX`. Typed Ghidra references, its exact live instruction
and original-file disassembly agree on this direct store. Its owning function,
entrypoint and complete initializer contract remain **unresolved**. No C++ startup
packet is ready from this evidence.

This independent packet uses published baseline `da380a564` and the accepted
[remaining-descriptor readiness review](CC12_REMAINING_RESOURCE_DESCRIPTORS_SOURCE_READINESS.md).
It changes only this document and its report. No camera boundary investigation,
compact investigation, C++/CMake/ledger/config/script edit, Ghidra mutation, build,
test or runtime probe was performed. Root remains the sole Ghidra writer.

## Confirmed instruction and target

The initial lease covered `01090344`, the two output files and ignored evidence.
After the direct write reference was returned, the lease was extended to
`00CD8524` and Root was notified before that instruction or its bytes were opened.

| Address | Selected extent | Store | Saved live bytes |
| --- | --- | --- | --- |
| `00CD8524` | `00CD8524..00CD8529`, 6 bytes | `MOV DWORD PTR [01090344], ECX` | `89 0D 44 03 09 01` |

The typed reference query returned five records under a limit of 80: reads at
`00B8F920`, `00B8F940`, `00B91630`, a data reference at `00B91624`, and the write
at `00CD8524`. All returned text is saved without a display spill. This establishes
one direct write site; it does not exclude indirect or unresolved writers.

The saved instruction-context response uses context zero, with empty before/after
arrays. Typed live byte reading covers exactly six bytes. Original-file
`disasm-raw` covers the same six-byte window and emits one instruction, below its
four-line limit. Its mnemonic, destination and register agree with the live
instruction. Live raw bytes are retained; no separate original/live raw-byte
equality comparison is claimed by this worker.

The complete reviewed Native code scope is **one instruction start and six
selected bytes**. No preceding/following instruction, guard, other descriptor
word, name data, parent/counter call, return, branch, handler or whole function
body was opened. The value's origin in ECX is unknown. The window is not a
function extent and does not establish incoming register ABI or store ordering.

## Unresolved function ownership

`lookup 00CD8524` reports no function start there and gives the earlier
`00CD8460 BSP_GroupPool_InitializeStatic` as an enclosing candidate. That nearest
indexed candidate is not proof that the group-pool function owns this store.
The indexed range `00CD8500..00CD8540` returns no function starts.

The live function query reports `No function found for 00cd8524`; the signature
query likewise reports no function at that address. Typed callers returns
`Function not found: null`. Caller information is unavailable, not an established
empty caller set. `docs-for 00CD8524` returns zero indexed document mentions.

These bounded address-query results leave actual instruction ownership and body
boundaries unestablished. They do not prove that no possible containing or
discontiguous function exists. No pseudocode, complete function disassembly,
listing repair, NoReturn change or control-flow recovery was attempted.

Original calling convention, arguments, stack cleanup, return value and incoming
register contract remain unknown. The actual guard, complete descriptor extent,
name word, parent initializer chain, counter ownership/consumption, partial-state
behavior, repeat-call behavior and original caller/CRT placement also remain
unknown. No adjacent address, nearest function or neighboring family's layout is
used to fill those gaps.

## Target identity and retained evidence

The configured project is `C:/Users/sqz269/bsp.gpr` and exists on disk. The existing
typed CLI verifies live project name `bsp`, program `/battlestationspacific.exe`,
language `x86:LE:32:default` and image base `00400000` before each live query.
Three batch verification records report live/snapshot counts of 64,730. The
client does not separately compare the live project-directory path with the
configured `.gpr` path; that qualification is retained.

Original-file decoding used the configured installed executable at
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`:
12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
It was only read and fingerprinted. The report retains the selected typed CLI
responses, exact instruction window and current Source/configuration context.
Historical readiness evidence is kept separate from this discovery.

## Next bounded packet

Use an Astra/Root ownership and function-boundary audit anchored at `00CD8524`.
Lease each newly opened address first. Establish actual owner, entry, full
physical/discontiguous extent and relevant control flow; Root performs any
necessary Ghidra repair through the existing write lock with prior metadata
retained. Only then derive the complete ABI, guard/layout/name, genuine parent
and counter dependencies, partial-state semantics and caller/CRT placement.

Skin-model production backing and initialization remain held. Any eventual
provider must use the actual retained VFS type owner and the same counter/lifetime
domain, with Source order qualified until Native placement is established.
Skin model `01090344` is distinct from skined-mesh resource `01090454` and the
excluded `01090370` family. Camera-resource `01090288` and compact `0109042C`
remain separate. No copied consumer ID, substitute guard or default descriptor
was introduced.

Primary review retained and verified all40 pin occurrences and all12 selected
context Git blobs. Root additionally replayed the exact selected six-byte
window against the unchanged original PE and worker-retained live hex; they
match directly. This adds selected raw-byte equality only. It does not extend
the window or establish a whole body, value origin, ABI, owner, caller, guard,
layout/name/counter/CRT slot. The ownership/boundary audit remains separate.
