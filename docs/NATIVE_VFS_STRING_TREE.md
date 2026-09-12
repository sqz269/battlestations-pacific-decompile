# Actual native VFS string-tree lifetime

Addresses: `004CEC60`, `004BE730`, `004CF8A0`, `004D1A50`, `004D74A0`,
`004B9FD0`, `004BE360`, `004BF130`, `004BF180`.

The actual manager string-tree family now has complete raw-storage source in
`src/native_vfs_string_tree.cpp`, declared by `include/bsp/native_vfs_string_tree.hpp`.
It includes partial-range deletion, successor transplantation and both mirrored
rebalance paths. No replacement map or full-range-only implementation is used.
Names are descriptive hypotheses, not recovered C++ symbols.

The report is [native_vfs_string_tree.json](../reports/native_vfs_string_tree.json).
All nine logical code spans (**1,365 bytes**) and four EH spans (**62 bytes**)
match live Ghidra memory and the installed executable. Read-only CLI batches
verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 and image
base `00400000`. Installed executable:
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Routines and original ABI

All ranges below are half-open. Explicit source storage/validation parameters
change the original ABI; these are not drop-in native replacements.

| Entry | End exclusive | Bytes | Original ABI | Coverage |
| --- | --- | ---: | --- | --- |
| `004CEC60` | `004CECB2` | 82 | ECX tree, stack node, RET4 | complete |
| `004BE730` | `004BE793` | 99 | ECX iterator, RET0 or validation tail | complete |
| `004CF8A0` | `004CFB6C` | 716 | ECX tree; output, owner, node; RET0C; EAX output | complete |
| `004D1A50` | `004D1B19` | 201 | ECX tree; output, first owner/node, last owner/node; RET14; EAX output | complete |
| `004D74A0` | `004D74D4` | 52 | ECX tree, RET0; normal EAX zero | complete |
| `004B9FD0` | `004B9FEC` | 28 | ECX node, RET0; EAX maximum | complete |
| `004BE360` | `004BE37B` | 27 | ECX node, RET0; EAX minimum | complete |
| `004BF130` | `004BF17E` | 78 | ECX tree, stack node, RET4; left rotation | complete |
| `004BF180` | `004BF1D2` | 82 | ECX tree, stack node, RET4; right rotation | complete |

`004CEC60` retains its historical `BSP_NativeStringSet_EraseSubtree` ledger
record in `mission_load_hosts.cpp`. That record describes semantic host
forwarding, not an address-matched raw implementation. The new actual function
record is separate; earlier evidence is retained and extended. The RET4 starts
at `004CECAF`; its last byte is `004CECB1`.

The old `STL_xlen_throw_004cf8a0` name is incorrect. Nil input throws an invalid
iterator exception; the normal body performs full erase/rebalance. The proposed
name is `BSP_RawStringTree_EraseIterator_004cf8a0`. Correct `_free` and CRT
exception library names are retained. Worker Ghidra access was read-only;
the parent owns applying annotations, preserving comments and saving exports.

## Raw layout and ordering

Tree storage has an untouched DWORD at +0, current head at +4 and unsigned
count at +8. An 18h-byte node stores left/parent/right at +0/+4/+8, actual
native-string length/data at +C/+10, color at +14 and sentinel at +15.
Color0 is red and color1 black; padding +16/+17 is untouched. An iterator is
the actual eight-byte `(owner,node)` pair, not a copied tree header.

The producer `004C26B0` allocates 18h, zeroes the three links and writes
color1/sentinel0 without initializing payload/padding. `00BE1E9C` selects
manager+6C; `00BE1EA6..00BE1EC5` publishes its head, sets sentinel1, writes
self root/minimum/maximum and zeroes count. This agrees with the existing
`native_vfs_container_allocation` declaration.

`004CEC60` first recurses into the current right subtree, reloads the current
string data, captures the current left link, then releases the string and
frees the node. It iterates the captured left node. It never resets the tree.
The source keeps the native data-before-left-before-length read order, and
uses volatile raw fields for the observed reload/store sequence.

`004BE730` calls the returning validation service for a missing owner, then
reloads node. A sentinel node tails that service; if it returns, increment
returns immediately. A right subtree selects its leftmost node. Otherwise,
parent traversal writes intermediate iterator nodes while ascending from
right children, then publishes the selected parent, including the head.

`004CF8A0` tests the original node sentinel before owner validation. It
advances its separate by-value input iterator; it does not require input
owner to equal the destination tree. Zero/one-child deletion reparents the
replacement only when nonsentinel and repairs extrema through the actual
min/max helpers. Two-child deletion moves the successor node, adjusts the
two possible successor-parent paths and swaps color bytes with the original
node. The original node is always the one released and freed.

Black repair retains both rotations, red-sibling conversion, black-child
propagation and inner/outer nephew cases. Root checks, live head reloads and
parent loads follow the native sequence. It then releases the original
string, frees the original node, loads the **current** unsigned count and
decrements only if nonzero. Finally it captures both returned iterator words
before storing output owner followed by output node. Output may alias raw
tree/node storage. There is no added rollback of tree edits on provider failure.

`004D1A50` captures begin before the first validation callback. Its full-range
test separately validates first and last owner identity against the tree;
full destruction resets root, count, minimum and maximum using current head
reloads, then returns `(tree,current begin)`. The partial loop validates owner
identity at every iteration, advances its first iterator before erasing a
separate copy of the old iterator, then reloads first owner/node. Empty partial
ranges still validate owner identity. `004D74A0` invokes the full current
range, frees the **current** head and clears head/count.

## Providers, calls and exception states

The report carries 29 numeric direct/tail call rows for the nine functions
plus the EH cleanup tail below; all **30 rows** pass live body/target
verification. Each function row has its original stack cleanup and each
call row records the callee body, containing caller and callee contract.

| Provider | Actual contract and cleanup |
| --- | --- |
| `00419CC0` | No consumed arguments, RET0, EAX pool. The preceding pushes belong to BD1510. |
| `00BD1510` | ECX pool; stack block, wrapping length+1, unused1; RET0C. |
| `00BF65AC` | Correct cdecl `_free` thunk; caller ADD ESP,4. Source uses `singleton_lifetime_free`. |
| `00BF6713` | No incoming arguments; internally pushes five zeros, calls BF66EF, ADD ESP,14, RET0. Can return. |
| `00408720` | ECX SBO temporary; stack string,count27; RET8. Complete counted assignment before cleanup is armed. |
| `00411700` | ECX owning exception destination; stack source SBO; RET4/EAX destination. |
| `00BF6885` | Stack object, metadata D863A8; physical RET8, CRT raises the exception. |
| `004072D0` | ECX SBO string, RET0; current storage destruction/reset. |

Production callers pass `ActualNativeStringPoolStorage` through its established
`NativeStringStorage&` base and the application's shared
`SingletonLifetimeCallbacks`. Every nonnull payload release repeats the
actual getter and sized return; this module owns no publication slot or pool.

`004CF8A0` installs handler `00C65CC8..00C65CD1` with FuncInfo `00D8E71C`
(magic19930522, maxState1). Map `00D8E714` has state0 -> -1 through
`00C65CC0`, whose `00C65CC3` tail calls `004072D0` on EBP-50. Nil input
initializes capacity15, length0 and NUL, then assigns27 bytes while state-1.
`004CF8F6` arms state0 before the owning logic-error construction. Only after
success does the native code publish D6926C and invoke the throw runtime.
Every normal erase path remains state-1; there is no tree cleanup funclet.

The source uses the established owning `NativeHardwareLayoutInvalidIterator`
transport, including actual 28h exception data, native copy/destruction
providers and the same temporary arming schedule. Its C++ RTTI, catch ABI,
FH3/SEH frame identity and mutable original spill aliases are boundaries.
The existing `NativeStringStorage::release` is noexcept: failure in lazy pool
recreation terminates, rather than reproducing the original throwing getter.

## Saved Ghidra boundaries to repair

| Current missing interval | Bytes | Meaning |
| --- | ---: | --- |
| `[004CFB32,004CFB6C)` | 58 | Reachable count/output/FS epilogue, RET0C at4CFB69. Existing stored body ends4CFB31. |
| `[004D74C4,004D74D4)` | 16 | Reachable free cleanup, zero head/count, RET0 at4D74D3. Stored body ends4D74C3. |
| `[00C65CC8,00C65CD2)` | 10 | Undefined FH3 handler: MOV EAX,D8E71C; JMP BF6B43. Existing cleanup function is separately C65CC0..C65CC7. |

These raw intervals were verified with bytes and disk disassembly without
assigning them a false stored-function attribution. No worker body extension
or global no-return change was made. Alignment bytes `[004B9FDD,004B9FE0)`
(`8D4900`, bypassed LEA) and `[004D1ACF,004D1AD0)` (`90`, NOP) are padding.

## Verification

`scripts/build.ps1` passes MSVC Win32 Release `/W4 /WX /fp:strict`, with
process-scoped `MSBUILDDISABLENODEREUSE=1`. The first build's preserved log
contains a local definite-initialization warning; initialization was made
explicit and the second build passes. `verify-seeds` reports8/8 matching
native seeds. Both existing CTests pass: `reconstructed_math` and
`native_math_differential`. No permanent test was added.

One ignored fixture in `local/string_tree_evidence` freezes header, source,
object, archive, exact native spans, probe source/object/executable and build
inputs **before** execution. Its final successful attempt compares a
15-node black tree removal sequence plus count mutation/output aliasing
(16 erase checks), partial/full range and wrapper destruction (3 checks),
returning owner repair and sentinel validation, and the source owning exception.
It compares normalized links, extrema, color/padding, current count, returned
iterators and release order/state. The release callback changes the removed
left link after its capture, exercising the subtree schedule.

The initial native mapping at400000 failed before executing native code;
that attempt, exit status and all23 frozen inputs are preserved. The installed
PE has stripped base relocations. Attempt2 copies all nine unchanged original
instruction spans to a private20000000 mapping: relative branches/calls move
together; four external provider entries are instrumented with observable
source storage/free/validation services. Original absolute FH3 handler/data
values remain unchanged and are **not executed**. This proves focused normal
storage behavior, not native EH execution or actual-pool internals.

The final fixture exits0 with
`PASS erase=16 range_destroy=3 iterator_validation=2 source_exception=1 native_code_spans=9 native_bytes=1365`.
All **29 frozen input hashes** are unchanged after execution. It links with
`/MANIFEST:EMBED`; failure logs and before/after manifests are retained locally.
The report includes every frozen hash and artifact path. Selected SHA-256:

- Source: `8e24b5a1ed026a0d9a8d55481b86fa87ace4b90dbf731ac1e8b8f6decad6e256`.
- Archive: `c35ff4d15b3ecb6302311a23b01ec746b5e477d4a5f6e4969a934861fc993c4e`.
- Successful probe: `4585a6025f1b7fd482f02dbdb71f0b2c2e55a665ef82b22961f17bf506891890`.

Status: exported/static-byte-verified, reconstructed, strict-build-tested and
focused-fixture-tested. Native ABI compatibility, native FH3 execution and
game validation are unclaimed. The original game installation is unchanged.
