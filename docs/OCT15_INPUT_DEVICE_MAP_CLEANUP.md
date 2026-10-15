# Actual InputDevice map cleanup

This packet reconstructs whole `006A7930` clear and the three whole map
destructors `006A7BA0`, `006AA3E0`, `006AA420`. The new RawCtx range service
implements only the admitted full-selection prefix `006A7AA0..006A7B1E`.
Source is [native_input_device_map_cleanup.cpp](../src/native_input_device_map_cleanup.cpp),
with its [public contracts](../include/bsp/native_input_device_map_cleanup.hpp) and
[evidence report](../reports/oct15_input_device_map_cleanup.json).

The actual tree is 12B: opaque DWORD0, head4, count8. Its accepted 9Ch node has
left0/parent4/right8, pooled key at0C/10, record14..97, color98 and nil99.
The dependency is accepted `6a9cdda42b63ba4ec2a44313bac786aac3919613`:
`destroy_native_input_device_subtree_006a7540(tree,0,root,RawCtx)` preserves
right recursion, left capture after recursion, pair destruction and original
node free. The actual native subtree inlines its record/key steps; its already
accepted complete Source composes the proved pair service. No second key guard
or NativeStringStorage bridge is added here.

Clear captures head/root and destroys the subtree. It then reads current H1
and stores H1.parent=H1; reads fresh H2 **before** clearing count and storing
H2.left=H2; reads fresh H3 and stores H3.right=H3. It keeps the sentinel
allocated and leaves opaque0 untouched. Native ECX is tree, EDX is unconsumed,
there are no stacked inputs, and return is plain RET with nonsemantic EAX head
residue. The Source service returns void.

The admitted prefix requires valid exclusively owned actual storage, nonnull
tree/head, owners both equal tree, first-node equal initial head.left and
last-node equal fresh head. Selection storage stays stable through selection.
The output is writable 8B storage, private in the actual consumers. The Source
retains first-owner TEST before initial head/left capture, owner CMP, first-node
CMP, last-owner TEST before fresh head capture, owner CMP and last-node CMP.
The two native diagnostic branches are unreachable under these preconditions;
out-of-fragment returning-handler behavior is not supplied by this API.

After the subtree, the prefix performs the same current-head reset order as
clear, then reloads head/left and captures the result node before writing output
owner then output node. Native ECX is tree, EDX is unconsumed on the admitted
path, the five stacked words are output/first-owner/first-node/last-owner/
last-node, and EAX returns output with RET14h.

Each whole destructor captures head then head.left, supplies owner twice and a
private 8B output, and therefore selects the full prefix on valid stable owned
storage. It reloads and frees **CURRENT head after** the range, clears+4 then+8,
preserves+0 and returns DWORD0. No getter/provider runs before selection. Native
ECX is the actual tree, EDX is unused, there are no stacked inputs and return is
plain RET. All three complete 52B bodies are identical after zeroing only their
two CALLrel32 operands, with the same two targets. The latter two Source APIs
reuse the first whole storage/return schedule.

There is no local EH registration, action, catch or rollback in any of these
five scopes. A normal RawCtx getter failure stops subsequent resets, head free,
clears and output stores. The accepted pair/record guards own their key and
member unwind, terminating on a secondary C++ cleanup exception. These new
normal services propagate exceptions; no blanket noexcept adapter is installed.

Fresh typed metadata describes the ordinary range entry as two body ranges:
127B admitted prefix and 73B partial range, separated by the non-body byte7B1F.
The partial range `006A7B20..006A7B68` remains outside the new RawCtx API.
Existing NativeStringStorage full/partial erasure and its actual AV fixtures
remain valid in their documented domain. Existing 5540C0 iterator Source also
remains. A complete ordinary RawCtx 6A71B0 iterator-erasure binding with its
diagnostics, rotations, throw/count and payload cleanup is not supplied here.

Fresh producer windows in 6AA460 show the device headers at+6C/+8, captured
head/left and owner twice before state arm/range, followed by current-head free
and ordered clears. The 6A7BE0 window has the same subtree/current-head reset
order as clear. These are bounded consumed-path proofs; the whole loader,
InputSettings lifecycle, other maps and their actions are outside this packet.

The owned scopes are 326B/134 instructions. Fresh PE/live comparisons cover
690B/261 current starts, including the excluded partial range, accepted subtree
and three consumer windows. All three normalized whole-body hashes are
`5389ba8e6d7ef09bc989b2a6a69d43c54ca9359b8d1df9d608d04ab7114fc53b`.
The report retains per-span hashes, typed complete body ranges, dependency blob
fingerprints and the ignored detailed byte/start captures. Every live batch
verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

| Native transfer | Scope |
|---|---|
| 006A793C CALL 006A7540 | owned clear |
| 006A7ABA CALL 00BF6713 | owned prefix; excluded diagnostic branch |
| 006A7AD6 CALL 00BF6713 | owned prefix; excluded diagnostic branch |
| 006A7AEA CALL 006A7540 | owned prefix |
| 006A7BB6 CALL 006A7AA0 | owned map |
| 006A7BBF CALL 00BF65AC | owned map |
| 006AA3F6 CALL 006A7AA0 | owned map |
| 006AA3FF CALL 00BF65AC | owned map |
| 006AA436 CALL 006A7AA0 | owned map |
| 006AA43F CALL 00BF65AC | owned map |
| 006A7B2A CALL 00BF6713 | read-only excluded partial |
| 006A7B39 CALL 005540C0 | read-only excluded partial |
| 006A7B47 CALL 006A71B0 | read-only excluded partial |
| 006A7576 CALL 006A7540 | accepted subtree support |
| 006A758F CALL 0055B690 | accepted subtree support |
| 006A75AC CALL 00419CC0 | accepted subtree support |
| 006A75B3 CALL 00BD1510 | accepted subtree support |
| 006A75B9 CALL 00BF65AC | accepted subtree support |
| 006AA4BD CALL 006A7AA0 | read-only consumer window |
| 006AA4C6 CALL 00BF65AC | read-only consumer window |
| 006AA60B CALL 006A7AA0 | read-only consumer window |
| 006AA614 CALL 00BF65AC | read-only consumer window |
| 006A7CEE CALL 006A7540 | read-only consumer window |

MSVC Win32 build and existing three suites passed: reconstructed_math,
native_math_differential and tool_tests, 3/3 in 7.24 seconds. No new tests
were added. Emitted COFF retains the raw selection/reset/output order,
CURRENT-head free and ordered clears/EAX0; both equivalent Source wrappers
forward to the shared complete implementation. Added context changes emitted
Source returns to RET4/RET18h, as qualified in the report.
Extra context and Source private frames, current CRT and CurrentCpp exception
handling do not establish native calling/return/flags ABI, original heap/pool
publication, RTTI/FH3/SEH, frame aliases, faults, stack exhaustion or game
behavior. No Ghidra mutation or push is performed by this worker.
