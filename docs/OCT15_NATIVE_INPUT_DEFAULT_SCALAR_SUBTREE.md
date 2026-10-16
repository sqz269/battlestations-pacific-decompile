# Default input scalar subtree destruction

Addresses: `0069D3F0` only. The retained library identity is
`STL_Tree_EraseSubtree_IntKey_N18B_0069d3f0`; the new Source name is provisional.

| Native entry | Coverage | Original interface | Source |
| --- | --- | --- | --- |
| `0069D3F0` | Complete, 53 bytes / 23 instructions, through `0069D424` inclusive | ECX opaque tree, EDX unused, stacked node, RET4, incidental EAX | `destroy_native_input_default_scalar_subtree_0069d3f0` |

The wrapper composes the accepted raw service in `native_input_int_subtrees.cpp`.
It forwards the same tree and node, with zero in the unconsumed EDX slot. There
is no callback, storage provider, node overlay, payload cleanup, guard or catch.
The old AV integer branches and frontend table projections remain unchanged.

Whole installed PE/live bytes and all current listing starts match for both the
candidate and accepted reference: 53 bytes / 23 instructions each. The complete
bodies are equal after masking only the four-byte free-call rel32 operand at
offsets 32..35. Actual differing offsets are 32, 33 and 34. Both free operands
independently resolve to the real CRT free service shown below. The unmasked
self-call bytes are equal and each origin resolves to its own entry. No
instruction, branch or nil offset is normalized away.

```text
0069D407 CALL 0069D3F0
0069D40F CALL 00BF65AC
00554F27 CALL 00554F10
00554F2F CALL 00BF65AC
```

The initial nil-byte read is at node+15h; any nonzero byte returns immediately.
Otherwise the routine reads current right+8, recurses, then reads CURRENT left+0.
It captures left, frees the original current node, tests captured left+15h after
free, and continues left. Tree is captured/forwarded but never dereferenced.
No mapped value, link, header, count, color or nil byte is written. The sentinel
is not freed, and there is no null-header or null-node skip.

Producer `0069D2D0` allocates 18h bytes and writes left+0/right+8 and zero nil+15h
(`0069D2E6`, `0069D2EC`, `0069D308`). This agrees with the accepted reference's
producer-backed raw offsets; no new C++ node type is declared. The two external
callers pass a root loaded from current head+4, plus the tree in ECX. Their
clear/range algorithms and later header resets are outside this packet.

The 51-byte interpretation was incorrect: `0069D422` is the START of `C2 04 00`,
not the last byte. Complete RET4 ends at `0069D424`; the accepted reference's
RET4 similarly starts at `00554F42` and ends at `00554F44`. Fresh typed bodies,
PE/live bytes and full decode establish 53 bytes in both cases. No listing or
flow repair is necessary or performed.

The wrapper delegates the accepted service. Recursion within that Source service
uses the reference implementation's own entry; the candidate's native self-call
remains a distinct origin, recorded above. This preserves the declared storage
effects; recursive return-address, private-frame, ambient register and stack-
exhaustion identity remain qualified. The added wrapper has no local EH; neither
Native body has an FH3 registration, catch or unwind action. Source exceptions
propagate through the delegated service without a new cleanup policy.

Reached storage must be readable and stable, with finite uniquely owned nodes
and backing compatible with `singleton_lifetime_free`, whose concrete binding
is current `std::free`. Original static-CRT heap identity, fault ordering, SEH,
FH3/private-frame ABI, game execution and whole parent readiness remain unproved.
The Source void interface does not invent a meaningful EAX value. Default clear,
range erase, `006AA460` and all other parents are excluded.

Validation and exact provider/object hashes are recorded in
`reports/oct15_native_input_default_scalar_subtree.json`. The final default Win32
build passed all three existing suites in 14.22 seconds. The entire actual
`bsp_core` object function is 11 bytes / five instructions: a balanced compiler
EBP frame, zeroed dead EDX, then the sole verified REL32 tail delegation with
matching fastcall physical signatures. All seven focused object checks pass.
There are no added object reads, guards, cleanup or EH instructions. No new tests
or Native/game fixture claims are introduced.
