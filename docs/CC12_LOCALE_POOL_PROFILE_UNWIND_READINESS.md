# CC12 locale pool profile and unwind readiness

The first DWORD at `00CE4120` points to `004360E0`. Its normal native path trims
fully free 1,024-slot blocks and repairs the identities in a moved last block.
The constructor handler prefix at `00C5EEBE` names `00D85688`; its exact FuncInfo
and three unwind rows identify a declared `2 -> 1 -> 0 -> -1` cleanup chain.
These results close profile-target and unwind-data readiness. Cleanup action
semantics, actual owner construction, slab production and application activation
remain open. No C++ was changed.

This packet follows the [pool lifecycle contract](CC12_LOCALE_NODE_POOL_LIFECYCLE_CONTRACT.md),
[slot provider contract](CC12_LOCALE_NODE_SLOT_PROVIDER_OWNER.md), and accepted
[payload helper](CC12_LOCALE_MAP_RAW_NODE_PAYLOAD_HELPER.md). Its complete machine
record is [the readiness report](../reports/cc12_locale_pool_profile_unwind_readiness.json).

## Bounded capture

Capture HEAD is `f414ed0709d190f7ec09d7ee2390aae4749a6286`, after merging Main.
The Source corpus is byte-identical to lifecycle capture
`6285c5af606dde35623499f8ff81f84c0643ffd7`: 4,215 actual Source/Git blobs, including
4,208 tracked `include/` and `src/` files. Six dependency doc/report pairs retain
12 actual Git blobs. The accepted particle epoch still has 4,215 exact Git blobs;
3,348 of its 3,354 selected files are byte-exact and the remaining six differ only
by line endings. Prepared registry C++ from other work is not used as evidence.

Fourteen read-only client query commands verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Initial/final typed metadata stays at modification
`72`. Fresh retained bytes total 256: profile cell 4, trim hull 160, fixed handler
window 32, descriptor 36, and unwind rows 24. All match the retained original PE
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The installed executable was not reopened. The preceding 1,079 native bytes and
older query epochs remain separately identified.

`004360E0` has a complete stored 95-byte body, within the admitted 512-byte limit.
`00C5EEBE` has no stored function at or containing it, so no body size was inferred.
A supplemental parent admission allowed exactly `00C5EEBE..00C5EEDD` (32 bytes),
decoded only through its first terminal instruction. The remaining 22 bytes were
retained and compared but not decoded, assigned to a function or analyzed.

## Profile virtual zero: `004360E0`

The cell bytes `E0 60 43 00` establish the target. Stored `undefined(void)` omits
the member register input: ECX is copied to ESI, with ESI/EDI saved and restored;
EBX/EBP are untouched. There are no explicit arguments, both exits use plain RET,
and no meaningful EAX result is established.

| Stored owned range | Bytes |
| --- | ---: |
| `004360E0..004360EC` | 13 |
| `004360F0..00436106` | 23 |
| `0043613D..00436157` | 27 |
| `00436160..0043617F` | 32 |

The 160-byte hull decodes into 57 instructions. Assuming `00BF65AC` returns,
54 instructions/149 bytes are reachable through 60 normal control-flow edges.
The unowned `00436107..0043613C` is 18 real continuation instructions/54 bytes.
Three unreachable alignment instructions at `004360ED`, `00436158`, `0043615F`
account for the other 11 bytes. The stored listing contains only 36 instructions.

At `00436102`, Ghidra records `CALL_RETURN` / `CALL_TERMINATOR` and no effective
fallthrough, while the direct free thunk's `no_return` is false. Pseudocode's
apparent exit after free loses the actual compaction path. Original bytes and
all in-hull branch destinations establish that continuation under normal return;
the stored override is retained and no GPR repair was made.

The normal schedule is:

1. Start index zero and compare it unsigned against CURRENT `pool+2C` count.
   Read CURRENT `pool+28` table and its current index entry. A block is empty
   precisely when its WORD at `+7800` equals `0400`; other values are retained.
2. Push and free that block through `00BF65AC`. After normal return, reload both
   table and count. Read `table[count-1]`, write that pointer to `table[index]`,
   then decrement CURRENT `pool+2C`. The table write precedes the count RMW.
3. Release the pushed argument. If index is below the current reduced count,
   reload the moved block and write the index into 1,024 DWORD identities,
   starting at block `+18`, stride `1C`. Node payloads are not moved.
4. Subtract one from index before the common increment to retry the hole.
   Index zero uses the native unsigned wrap sequence. A last-block removal still
   performs the table write before the count decrement.
5. Compare count against zero before setting `pool+34` to `FFFFFFFF`. If count
   was nonzero, capture the table cursor once and scan for the first block whose
   WORD free count is nonzero, rereading CURRENT count for loop bounds. Store that
   first index into the minimum; otherwise leave `FFFFFFFF`.

The body does not enter/leave a critical section, adjust depth, construct an EH
frame, change profile/list links, destroy payloads, clear table capacity, or free
the pool owner. Trim needs external serialization. The 1,024-slot identity loop
is proved; the actual slab allocation extent and producer remain unread.

## Handler prefix and exact unwind data

`00C5EEBE: MOV EAX,00D85688` followed by
`00C5EEC3: JMP 00BF6B43` is the complete admitted 10-byte prefix. Its relative
jump and both live instruction spellings agree with the original bytes. A null
stored function remains null; this is not an inferred ten-byte function body.

The installed MSVC `14.51.36231` headers `ehdata.h`, `ehdata_values.h`, and
`ehdata_forceinclude.h` are retained with hashes. Their x86 absolute pointer
layout, four-byte packing, 29-bit magic field and flag definitions label these
bytes. They do not identify the original game's compiler or prove FH3 execution.

| `00D85688` field | Value |
| --- | --- |
| magicNumber / bbtFlags | `19930522` / `0` |
| maxState | `3` |
| pUnwindMap | `00D85670` |
| nTryBlocks / pTryBlockMap | `0` / `0` |
| nIPMapEntries / pIPtoStateMap | `0` / `0` |
| pESTypeList | `0` |
| EHFlags | `1` (`FI_EHS_FLAG`) |

| Row state | toState | Action |
| ---: | ---: | --- |
| 0 | -1 | `00C5EEA0` |
| 1 | 0 | `00C5EEA8` |
| 2 | 1 | `00C5EEB3` |

No action body, cleanup target, frame expression or section base was read or
inferred. Earlier constructor evidence stores states -1, 0 and then 2; the table
row for state 1 does not introduce a normal state-1 store. Flags and table shape
do not establish fault/SEH coverage, action ordering beyond declared rows,
cleanup semantics, exception identity or double-failure behavior.

## Smallest Source design and next packet

The recommended next packet is `cc12_locale_borrowed_node_recycling_helper`,
limited to dormant normalized helpers for `00736090` and `0073C220`. It has not
started. Its exact proposed native ranges are `00736090..007360F8` and
`0073C220..0073C23F`; `00737CE0`, `00E1768C`, and the actual string publication
cells remain dependencies. No new Native body is necessary for this proposal.

Proposed exclusive files are `include/bsp/native_locale_map_node_recycling.hpp`,
`src/native_locale_map_node_recycling.cpp`,
`docs/CC12_LOCALE_BORROWED_NODE_RECYCLING_HELPER.md`,
`reports/cc12_locale_borrowed_node_recycling_helper.json`, and the corresponding
`local/cc12_locale_borrowed_node_recycling_helper/` directory. The integrator must
serialize any required `cmake/startup.cmake` registration. Existing payload,
string-context and allocator-domain Source is a read-only dependency.

The design requires one supplied actually initialized `E1768C` pool, genuine
allocated slot membership, coherent live block identities, and the existing raw
string context borrowing the same actual `AA8/AA4/AA0` publication cells. A pointer,
field view or zeroed buffer is not construction or canonical-owner evidence.

Slot return retains its own Enter/depth/Leave sequence even under the map's
recursive lock. Read CURRENT depth after Enter, then slot `+18`, CURRENT table
and block. Subtract addresses modulo 32 bits, interpret as signed32, divide by
28 toward zero, and truncate to WORD. Capture the free-count WORD for the index
write at block `+7000+2*count`, then increment the CURRENT WORD at `+7800`.
Compare the captured block index against CURRENT unsigned minimum; decrement
CURRENT depth before Leave. Add no null/bounds guard, rollback or physical free.

The scalar wrapper first calls the existing payload helper on the original node
and actual raw string context. Only after normal payload return does it test
flags bit zero; other bits are ignored. If set, return the original slot to the
same actual pool with no early slot/table reads. Return the original address
bits without a live-object guarantee. Propagating payload failure prevents slot
return; no wrapper rollback is added. Stable Enter/Leave import bindings are a
factoring precondition because the map caches Enter while the provider reloads it.

Acceptance of this design permits a separately scoped helper implementation only.
Any application composition still needs canonical storage/lifetime, actual slab
production, genuine profile binding, unwind cleanup semantics, and payload and
borrower completion before pool teardown. No invented domain, owning storage,
startup registration or placeholder trim is admitted. The finite literal search
of 4,208 current Source files found no `4360E0`, `C5EEBE`, `CE4120` or `E1768C`
match; this is bounded evidence, not proof that no equivalent behavior exists.

## Verification and limits

`local/cc12_locale_pool_profile_unwind_readiness/evidence.zip` contains 4,289
payload files, 80,811,015 uncompressed bytes; ZIP size is 22,034,395 bytes and
SHA-256 is `ea19aa762cd4de2c8dff8d0a2d09b989d7c4b9d1eec9c08f1ebb8aea3e1c4570`.
A guarded read-only ZIP replay independently checks all manifest hashes, actual
Git objects, dependency blobs, prior/current Source equality, original PE slices,
x86 branch targets, normal CFG, retained flow defects and bounded unwind data.
A physical copy produces the same PASS with external reads, process/network calls,
dynamic loading and directory access denied during replay. Replay is evidence
validation, not native execution or a behavioral fixture.

Only this document and its report change. No Source, GPR, ledger, C++ build,
new test, probe, game run or OS change occurs. Profile dispatch is identified and
normal trim behavior is evidenced; Source trim, original ABI replacement, FH3
behavior, canonical lifecycle, raw-map activation and application GO remain false.
