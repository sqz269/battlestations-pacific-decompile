# Native container allocation leaves shared by VFS

This packet reconstructs six complete allocation leaves, **243 native bytes**.
The APIs in `bsp/native_vfs_container_allocation.hpp` return raw native storage.
They introduce no manager, list, tree or allocator owner class. Names describe
observed behavior and remain hypotheses rather than recovered symbols.

| Original range, inclusive | Bytes | Source API | Allocation | Producer writes, in order | Coverage |
| --- | ---: | --- | ---: | --- | --- |
| BDA960-BDA979 | 26 | `allocate_native_list_head_00bda960` | 0Ch | DWORD +0/+4 = allocation | complete |
| BDA980-BDA999 | 26 | `allocate_native_list_head_00bda980` | 28h | DWORD +0/+4 = allocation | complete |
| BDABF0-BDAC26 | 55 | `allocate_native_tree_node_00bdabf0` | 24h | DWORD +0/+4/+8 = 0; byte +20=1, +21=0 | complete |
| BDABA0-BDABD6 | 55 | `allocate_native_tree_node_00bdaba0` | 20h | DWORD +0/+4/+8 = 0; byte +1C=1, +1D=0 | complete |
| 4C26B0-4C26E6 | 55 | `allocate_native_tree_node_004c26b0` | 18h | DWORD +0/+4/+8 = 0; byte +14=1, +15=0 | complete |
| 7F82F0-7F8309 | 26 | `allocate_native_list_head_007f82f0` | 0Ch | DWORD +0/+4 = allocation | complete |

The entire bodies were read in live Ghidra and compared with the installed PE;
all six match. Each BSP CLI live query verifies project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base against
`config/target.json`, which points to `C:/Users/sqz269/bsp.gpr`. The installed
executable at `I:/SteamLibrary/steamapps/common/Battlestations Pacific` was read
only. This packet changes no saved Ghidra names, comments, signatures or bodies;
the source name/reconstruction ledger records the proposed interpretations.

## ABI and instruction schedule

All six source APIs are `void* __cdecl()`: no incoming register or stack value
is consumed. An immediate size is pushed; BF681B defines the EAX result; EAX is
then preserved through plain RET. ECX is first defined by `LEA ECX,[EAX+4]`
after the allocation call and is only scratch. The leaves do not use EDX or
callee-saved registers. The saved decompiler signatures omit the pointer return;
the assembly establishes it.

Every DWORD destination is tested independently for zero before its store.
For the tree nodes, the final byte stores are unconditional. Consequently a
forced null allocator result skips +0 but attempts a DWORD write at address 4;
an early null return would change the original schedule. Naked MSVC x86 entries
preserve each instruction, including the independent +4/+8 address tests, while
routing the call to the actual existing source allocation service. Payload,
padding and all other unlisted bytes remain untouched. There is no whole-node
initialization or extra cleanup after a fault.

## Sole direct callee and ownership

| Containing function | Native call site | Native target | Stack argument | Cleanup |
| --- | --- | --- | --- | --- |
| BDA960 | BDA962 | BF681B | 0Ch | BDA967: ADD ESP,4 |
| BDA980 | BDA982 | BF681B | 28h | BDA987: ADD ESP,4 |
| BDABF0 | BDABF2 | BF681B | 24h | BDABF7: ADD ESP,4 |
| BDABA0 | BDABA2 | BF681B | 20h | BDABA7: ADD ESP,4 |
| 4C26B0 | 4C26B2 | BF681B | 18h | 4C26B7: ADD ESP,4 |
| 7F82F0 | 7F82F2 | BF681B | 0Ch | 7F82F7: ADD ESP,4 |

BF681B's live body calls malloc at BF6833 and retries through the new-handler
call at BF6826 if allocation returns null. It throws bad_alloc on exhausted
allocation. The source calls `singleton_lifetime_allocate` with allocation
kind `object` and equal native/host byte counts. Its actual implementation uses
`std::malloc`, `_callnewh` retry and `std::bad_alloc`. Release returned storage
through its malloc-paired `singleton_lifetime_free`. No extra allocator seam,
invented static globals or link stubs were introduced.

The current source CRT owns allocation, handler state and exception identity;
this packet does not replace the original static CRT globals or heap identity.
The normal allocator returns nonnull or throws, so the forced-null probe is
explicit fault-path evidence for the leaves rather than a claim that normal
small allocations return null.

These helpers are shared. In particular BDABA0 also serves BF4D30's physical
directory index and remains reusable by that producer. Tree allocation leaves
initially write the sentinel byte as **zero**. BE1DC0 separately sets it to one
and installs self-links while constructing its heads; these APIs do not perform
those owner-level writes. See the bounded dependency map in
[NATIVE_VFS_MANAGER_PACKET_MAP.md](NATIVE_VFS_MANAGER_PACKET_MAP.md).

## Verification and limits

The machine-readable evidence is
[native_vfs_container_allocation.json](../reports/native_vfs_container_allocation.json).
`scripts/build.ps1` passed with the repository's strict MSVC Win32 flags;
`reconstructed_math` and `native_math_differential` both passed. The seed byte
verification passed, and `verify_report_calls.py` checked all six native call
rows with zero failures. The focused ignored probe passed all six leaves:
their 243 emitted bytes match the original except the six CALL rel32 fields;
real shared-service allocation/free worked; controlled allocations preserved
the exact writes and all poison-filled payload/padding/guard bytes; forced
null results faulted on a DWORD write to address 4 in both original and source.

The probe copies the verified original bytes and the emitted archive entries
into private executable allocations. Only their single allocation CALL is
redirected to a deterministic fixture for poison-byte and null-result checks.
It also exercises each unmodified source API with the actual shared allocator
and free service. Neither the installed executable nor compiled archive code is
patched. The fixture is not linked into the reconstruction.

No original function definition or missing-range repair is needed for these
six complete bodies. This is source/ABI/storage evidence, with no game runtime
validation and no complete VFS manager construction or lifetime readiness claim.
