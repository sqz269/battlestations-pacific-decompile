# Actual resource-cache erasure BP

Addresses: `00B7CB70`, `00B7CB90`, `00B7CE80`, `00B7FA60`, `00B801C0`.

This packet reconstructs the actual cache's extrema walkers, successor iterator,
single-iterator erasure and resource-manager erasure by name. It composes the
integrated actual node, lookup, rotation, raw pooled-string and owning native
exception sources. It does not establish manager construction, cache insertion
ownership, resource destruction or runtime integration.

| Routine | Full inclusive span | Original ABI | Coverage |
|---|---|---|---|
| Maximum node | B7CB70..B7CB8B, 28 bytes | ECX node; EAX node; RET | complete |
| Minimum node | B7CB90..B7CBAA, 27 bytes | ECX node; EAX node; RET | complete |
| Successor | B7CE80..B7CEE2, 99 bytes | ECX actual iterator; RET; no uniform EAX result | complete |
| Erase iterator | B7FA60..B7FD2B, 716 bytes | ECX tree; stack output, input owner, input node; EAX output; RET 0Ch at B7FD29 | complete |
| Erase name | B801C0..B8020C, 77 bytes | ECX manager; stack actual name header; RET 4 at B8020A; no EAX result contract | complete |

The new C++ interfaces add explicit raw string-pool and returning CRT services.
`NativeResourceCacheIteratorStorage` is the actual 8-byte owner/node pair; erase
takes it by value, matching the mutable native stack arguments. These interfaces
are not register/stack/FH3/SEH ABI bridges.

## Storage and current reads

The node producer B7F220 establishes 1Ch-byte nodes with left/parent/right at
0/4/8, pooled name length/data at C/10, raw resource at 14, color at 18 and nil
at 19. Tree +4 is the head and +8 the unsigned count. The manager embeds this
tree at +14h, hence manager head/count are +18/+1C. Color 1 is black; any nonzero
nil byte denotes the sentinel. Volatile field accesses retain native current
loads and writes; they do not make concurrent mutation safe.

Maximum/minimum load the selected current child, test its nil byte, and continue
through the child's selected link. Neither validates the starting pointer or
adds an end-iterator policy. B7CB7D..B7CB7F is a skipped `LEA ECX,[ECX]` alignment
instruction in the complete captured span; it is absent from the reachable
Ghidra listing and requires no control-flow repair.

Successor validates a null owner through BF6713 and then reloads node +4. A nil
node tail-jumps to the same returning CRT boundary at B7CE97; source returns
afterward, preserving a handler's repair. Otherwise it finds the minimum of a
right subtree, or climbs parents while the iterator's current node is the
parent's right child. The climb writes the iterator before each parent reload.
No owner-equals-tree precondition is introduced.

Erase captures the original input node before checking nil or advancing the
local iterator. A nil original throws before owner validation. The subsequent
transplant uses the advanced local node when both children exist; the decompiler
can wrongly eliminate this branch by treating the mutable stack argument as
unchanged. With two children, the successor node is relinked and colors are
swapped. The key and resource payloads are never copied into the original node.
With zero or one child, the head is reloaded separately for minimum and maximum
maintenance. Both rotation directions use the existing actual cache helpers.

The black-node repair preserves sibling reloads after rotations, both nil
sibling paths, near/far child color checks, and the parent reload even when an
upward step reaches the root. Only black original nodes cause final replacement
blackening. The original node identity remains the one eventually freed.

The normal cleanup schedule is:

1. After unlink/rebalance, capture original name data +10. If nonnull, capture
   current length +C plus one with unsigned wrapping, resolve the current pool
   through 419CC0, then return that captured data/size through BD1510 with flag 1.
   The existing raw-header destructor implements this exact sequence.
2. Free the captured original node through BF65AC. Do not clear or destroy its
   mapped resource. No instruction in B7FA60 reads node +14.
3. Reload current tree +8 after free; decrement only if nonzero. Do not saturate
   an earlier captured count or decrement before releasing storage.
4. Capture the advanced local owner and node before writing output owner then
   output node. Return the output pointer.

The pool getter is allowed to fail after the tree is already changed. Native
erase has no normal-path unwind action for the removed node or its key. Source
propagates such a C++ failure without node free, count adjustment, output writes
or invented rollback. The ignored fixture exercises that concrete ownership
boundary and explicitly reclaims its retained test allocations afterward.

Erase-name calls B7E7B0 with manager +14h and the supplied actual name header.
It captures the returned owner, then the current tree head before validation.
A null or mismatching owner invokes BF6713 and may return. The local node is
read afterward and compared with the captured head. If present, erase receives
the captured owner and current local node. Missing names do nothing. Matching
uses the existing empty-marker and case-insensitive C-string rules. There is no
resource-pointer argument, pointer-match condition or mapped-resource release.
The sole current manager caller B8849C passes resource +8 as the key and uses
the current manager returned by 4C1400.

## Native exception evidence

B7FAA8 calls 408720 with the exact 27-byte message at CE44E0,
`invalid map/set<T> iterator`. The 1Ch temporary starts with capacity 15,
length 0 and an empty inline buffer. B7FAB6 arms state 0 only after counted
assignment returns. B7FABA calls 411700 to construct the owning logic-error
base; B7FAC9 replaces its profile with D6926C. B7FAD1 calls BF6885 with throw
information D863A8.

The existing `NativeHardwareLayoutInvalidIterator` is reused solely for that
owning 28h-byte native out-of-range payload. Its source constructor uses 411700,
copy uses 441760 and destruction uses 4412B0. The throw-info descriptor names
4412B0 and catchable array D863E4. Its primary catchable record D863F4 records
size 28h and copy 441760. The source type's hardware-related name does not
select another tree implementation.

FH3 handler CC2118..CC2121 loads FuncInfo DFB340 then tail-jumps to BF6B43.
FuncInfo has magic 19930522, one state, unwind map DFB338, no try blocks and
EH flags 1. State 0 transitions to -1 through CC2110. The defined unwind
CC2110..CC2117 computes EBP-50 and tail-jumps to 4072D0. The source guard is
armed only after assignment succeeds, mirroring this completed temporary's
cleanup if exception construction or throwing unwinds. No cleanup guard is
invented for a partial assignment or for normal tree/key operations.

Native FH3 selection, SEH, original RTTI/catch interoperability and throw-helper
ABI remain outside the new host C++ interface. The fixture establishes owned
message copying and survival beyond the first catch, not native exception ABI.

## Verification and remaining analysis work

Every live batch used `bsp.py ghidra`, verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`, with autostart disabled. The installed PE and live
Ghidra match for all 947 owned bytes, 18 compiler-support bytes and 132 metadata/
message bytes. The JSON report carries each full span, hash, original ABI,
source/dependency hashes and every CALL or external JMP. All 21 supported
transfer rows pass the whole-report verifier; the external JMP rows are
explicitly `tail_jump`.

The erase tail B7FCF2..B7FD2B is currently fully owned by B7FA60, including the
post-free count/output work formerly omitted by a stale CALL_RETURN override.
CC2110 is defined. CC2118 is decoded but has no Ghidra function; its external
JMP remains separately recorded as decoded-only. The primary can define the
exact ten-byte compiler handler, preserve its compiler identity, save/export,
and then promote that one transfer to verified membership. No Ghidra mutation
or naming is performed by this worker.

Strict MSVC Win32 `/std:c++17 /EHsc /O2 /MD /W4 /WX /fp:strict` compilation
passed for the new erase TU, the separately compiled integrated insertion TU
and one ignored actual-storage fixture. The fixture linked with embedded
manifest and passed 62 erasures with independent order, black-height, parent,
node-identity, successor and extrema checks. It also covered borrowed arbitrary
resource pointers and nonmatching iterator owners, returning CRT repairs,
owning invalid-iterator copying, manager missing/equivalent keys, key-before-node
free, post-free count changes to zero/FFFFFFFF, and the post-unlink pool failure.

The fixture reused the already-frozen BN library at
`J:/PROG/battlestations-pacific-decompile-orch4-resource-cache-insert-bo/local/resource_cache_insert_bo/bsp_core_bn_frozen.lib`,
SHA256 `d9947ae6d8e55c2903641262080c16b543df87eb1519df37d932b1824a9030f0`.
Its original primary build source is commit
`9673354d1a985c46c83a8226caa7a54e0290328b`; source/copy/source hashing was captured
when frozen. Current dependency blobs match that commit except insertion,
which is separately compiled from this packet's base
`ba1f9a6c4b9d8a524dd6f74ad0a1e8096bdb94ae`. No primary/main build or library
recopy occurred. The exact fixture/source/object/executable hashes are in the
report; local helpers and executable remain ignored.

This is complete source reconstruction with TU and focused fixture verification.
It is not a baseline build, original-ABI differential run, game execution,
production integration or proof of resource-manager lifetime behavior.
