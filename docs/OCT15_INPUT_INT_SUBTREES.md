# InputSettings integer subtree destruction

This packet adds two complete, provider-free ordinary Source entries:
`destroy_native_input_int_only_subtree_004c1890` and
`destroy_native_input_int_scalar_subtree_00554f10` in
`src/native_input_int_subtrees.cpp`. These descriptive names are hypotheses.
The retained library-shaped names are
`STL_IntOnlyTree_DestroySubtree_N14_004C1890` and
`STL_IntScalarTree_DestroySubtree_N18_00554F10`.

The older private generic loop in `native_input_settings_tree_cleanup.cpp`
requires `NativeStringStorage&` even on these payload-free branches. The
00554F10 frontend table/host projection also remains present and unchanged.
Neither provided an ordinary named API without that unrelated provider. This
packet leaves those identities and their historical evidence intact. It does
not establish readiness of whole InputSettings records or partial-range erase.

## Complete bodies and producer-backed representation

| Entry | Complete body | Instructions | Allocation producer | Sentinel constructor | Nil byte |
| --- | --- | ---: | --- | --- | --- |
| 004c1890 | 004c1890..004c18c4, 53 bytes | 23 | 004c2700 requests 14h | 004c4df0 | +11h |
| 00554f10 | 00554f10..00554f44, 53 bytes | 23 | 00554ec0 requests 18h | 00555540 | +15h |

Both allocation producers initialize link words +0/+4/+8 to zero along their
observed branches, set the preceding flag byte to one and the nil byte to zero.
Their constructors publish the returned pointer at tree+4, set its nil byte to
one, self-link all three words and zero tree+8. The destructor consumes only
left+0, right+8 and its exact nil byte. Other bytes remain opaque; there is no
invented payload type, getter, parent-link interpretation or payload cleanup.
The producer/constructor functions are context evidence, not newly owned ports.

For any nonzero nil byte, the entry returns without freeing that node. For a
nonnil node it loads right+8, recursively destroys that subtree, then rereads
CURRENT node left+0 after the recursion returns. It captures that left pointer,
frees CURRENT node, and only then reads the captured left's nil byte to continue
the loop. No links, nil bytes, tree header, count or payload bytes are written.
No null, cycle, sharing or malformed-tree guard is added. The Source uses raw
x86 loads to retain widths/order without C++ node overlays or pointer arithmetic.

## ABI, lifetime and evidence limits

Native ECX is captured and forwarded to recursion, but never dereferenced. EDX
has no consumed input; one node word is stacked and both routines end in RET4.
There is no semantic EAX result. The new void fastcall Source APIs retain that
input shape and pass zero only in recursively unused EDX. This does not claim
identical ambient registers, flags, private stack frame or binary compatibility.

Every reached node/nil sentinel must be readable at the accessed offsets.
Nonnil nodes must form a finite, uniquely owned tree allocated in the accepted
`singleton_lifetime_free` current-CRT domain. The opaque tree word is never
read. Nil sentinels remain allocated. Native BF65AC is the existing CRT-free
boundary; Source uses the real `std::free` service, without allocator callbacks
or a storage provider. Original static-CRT heap identity, allocator hooks,
fault/SEH order, stack exhaustion and game execution are not established.

Complete typed body ranges contain no EH registration, catch scopes or extra
unwind fragments. Source adds no catch or exception transport. All native data
operations here are byte/DWORD integer operations; there is no x87/NaN behavior.

`reports/oct15_input_int_subtrees.json` retains full raw spans and provenance.
The ignored audit `local/oct15_input_int_subtrees_byte_audit.json` records seven
live spans equal to disk without masks, six complete bodies/298 bytes, all 108
current instruction starts equal to Capstone 5.0.7, and nine transfer edges.
Project/program verification used `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` before every live query; no Ghidra writes occurred.

## Exact transfers

Owned bodies: `004c18a7 CALL 004c1890`, `004c18af CALL 00bf65ac`,
`00554f27 CALL 00554f10`, `00554f2f CALL 00bf65ac`.
Producer context: `004c2702 CALL 00bf681b`, `00554ec2 CALL 00bf681b`,
`004c4df3 CALL 004c2700`, `00555543 CALL 00554ec0`.
Accepted free thunk context: `00bf65ac JMP 00bf9dc8`.

The exact Source MSVC Win32 build passed all three existing suites
(`reconstructed_math`, `native_math_differential`, `tool_tests`) in 7.72 seconds.
JSON transfer verification and Markdown claims passed 9/9 with no unchecked
claims or warnings. Independent root Source/native review found no discrepancy;
the report records the frozen Source/header hashes. Compiled COFF inspection
confirms each Source function's byte-width nil load, right recursion, later left
capture, current free, next nil test and RET4. Its 92-byte private frame differs
from the original 53-byte body. No new tests or native/game execution claims are
added; existing differential seeds do not execute these routines.
