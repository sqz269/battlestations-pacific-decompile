# Camera-resource writer discovery

Three original instructions write the requested camera-resource cells:
`00CD83BF -> 0109028C`, `00CD83C4 -> 01090290` and `00CD83D8 -> 01090288`.
Typed Ghidra references and exact original-file instruction windows agree on
these stores. The owning function, entrypoint and complete initializer remain
**unresolved**. This packet stops at a bounded instruction-ownership and function
boundary gate; it supplies no new descriptor initializer or production backing.

Baseline is published `b9e1a76c8`, following the accepted
[Source readiness review](CC12_REMAINING_RESOURCE_DESCRIPTORS_SOURCE_READINESS.md).
Only this document and its JSON report change. Root remains the sole Ghidra
writer. No annotation, listing repair, function definition, C++, CMake or ledger
change was made, and no build, test or runtime probe was run.

## Target and query scope

The configured project is `C:/Users/sqz269/bsp.gpr`, project name `bsp`, program
`/battlestationspacific.exe`, language `x86:LE:32:default`, image base `00400000`.
The configured project file exists. Each typed live query uses the existing
`bsp.py` client, whose `Client.verify()` checks live project name, program path,
language and image base before the requested operation. Five batch verification
records all report live/snapshot counts of 64,730. The client does not separately
compare a live project-directory path with the configured `.gpr` path.

Original-file disassembly used the configured installed executable:
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The original file was only read and fingerprinted.

The initial lease covered `01090288`, `0109028C`, `01090290`, the two report files
and the ignored evidence directory. After references identified the write sites,
the lease was extended to `00CD83BF`, `00CD83C4`, `00CD83D8`, and Root was notified
before any instruction or byte window at those addresses was opened. No guard,
name word, neighboring function body or handler was opened.

## Direct evidence

| Instruction | Selected byte extent | Original-file store | Saved live bytes |
| --- | --- | --- | --- |
| `00CD83BF` | `00CD83BF..00CD83C3`, 5 bytes | `MOV DWORD PTR [0109028C], EAX` | `A3 8C 02 09 01` |
| `00CD83C4` | `00CD83C4..00CD83C9`, 6 bytes | `MOV DWORD PTR [01090290], ECX` | `89 0D 90 02 09 01` |
| `00CD83D8` | `00CD83D8..00CD83DD`, 6 bytes | `MOV DWORD PTR [01090288], ECX` | `89 0D 88 02 09 01` |

`ghidra xrefs` returned five references for `01090288`, two for `0109028C` and
one for `01090290`, each query limited to 80 references. Each result includes the
corresponding direct `WRITE` above. These are all returned records, with no
display spill; they are not a proof that every dynamic or unresolved writer is
known. The other returned references are reads/data references.

Saved `instruction-context` records use context zero: exactly the three requested
instructions, with empty before/after arrays. `disasm-raw` reads each selected
5/6/6-byte original-file window and emits one instruction, below its four-line
limit. Each agrees with the live listing on mnemonic, memory target and source
register. Separate typed `ghidra bytes` results retain the live bytes shown above.
The disk CLI emits decoded instructions, so no separate original/live raw-byte
comparison is claimed.

The reviewed code consists of **three instruction starts and 17 selected bytes**.
Those windows do not establish a whole physical function or body extent. The
14 intervening bytes `00CD83CA..00CD83D7` were not opened, and no entry, return,
branch or predecessor/successor body was traced. Numeric address order does not
prove that all three stores execute in one function or path. The values' origins
in EAX/ECX are also unestablished.

## Ownership and ABI gate

The following bounded metadata results prevent a complete-initializer claim:

- `lookup 00CD83D8` reports no function start there and gives earlier
  `00CD8340 BSP_BoneResourceType_InitializeStatic` as an enclosing **candidate**.
  That nearest indexed candidate is not ownership evidence.
- The indexed range `00CD8380..00CD83F0` returns no function starts. This query
  does not establish every current live or discontiguous function boundary.
- Brief live prototypes at all three write sites do not resolve function
  information. The full query at `00CD83D8` reports `No function found`, and its
  signature query reports no function at that address.
- Typed callers at `00CD83D8` reports `Function not found: null`. Callers are
  **unavailable**, not an established empty caller set.
- `docs-for 00CD83D8` returns zero indexed document mentions.

These are address-query results, not proof that no possible containing function
exists. Actual instruction ownership and boundaries require the next audit.
No pseudocode, complete function assembly, control-flow reconstruction or listing
repair was attempted. Exact original calling convention, arguments, stack cleanup,
return value and register ABI therefore remain unknown.

The actual guard, complete descriptor extent, optional name word, parent
initializer chain, counter usage and store/partial-failure order remain unknown.
The three confirmed stores do not establish an initializer entrypoint, a CRT slot
or a production caller. No adjacent address is promoted to a function entry and
no guard/layout/counter behavior is borrowed from animation, bone or camera-object
initializers.

## Suggested next bounded packet

Use an Astra/Root packet anchored by the three confirmed write instructions to
establish their actual instruction ownership and function boundaries. Inspect
only newly leased addresses, and distinguish saved listing, original-file code
and live Ghidra metadata. If definition or flow repair is necessary, Root performs
the mutation through the existing write lock and retains old metadata.

Only after the entry and complete body are established should the next packet
derive ABI, guard, layout/name, actual parent/counter calls, partial-state behavior
and original caller/CRT placement. The discovered entry may then become a precise
complete-initializer reconstruction packet. No C++ startup packet is ready now.

The existing VFS type owner, same-counter lifetime contract and Source-order
qualification remain unchanged. Camera-resource production backing and consumer
composition stay held. Compact `0109042C` and skin-model `01090344` remain outside
this discovery; their bodies and data were not investigated.

Primary review retained and verified all49 pin occurrences and all12 selected
context Git blobs. Root additionally replayed the three exact selected windows
against the unchanged original PE and worker-retained live hex: all17 bytes
match directly. This adds selected raw-byte equality only. It does not extend
the byte windows or establish a whole body, value origin, ABI, owner, caller,
guard/name/layout/counter/CRT slot. The next ownership/boundary audit remains
separate and must preserve this held complete-initializer boundary.
