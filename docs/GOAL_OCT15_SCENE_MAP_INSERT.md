# Scene record map insertion: 0046AD70 / 0046B340

The complete ordinary Source bodies are in
`src/native_scene_record_map_insert.cpp`. This packet reconstructs 768 normal
native bytes and the separate eight-byte completed-message cleanup C61CB0.
The report records exact bytes, hashes and typed transfer rows. Live Ghidra
bytes were compared with the preserved installed PE, not substituted exports.
Target: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 base400000.
Names below are descriptive hypotheses, not recovered symbols.

## Native layout and interfaces

The tree has head+4/count+8; actual1Ch nodes have links0/4/8, name length/data
C/10, mapped DWORD14, color18 and sentinel19. The nominal mapped type remains
unclaimed. This uses the existing raw string pool/allocation services and does
not bridge to the older semantic `scene_record_map` representation.

0046AD70 uses ECX tree, four stacked DWORDs output/left/parent/pair, RET10h and
EAX output. 0046B340 uses ECX tree, hidden output/pair, RET8 and EAX output.
The new Source signatures expose these original argument cells and the
driver's scratch cells explicitly. They require valid initialized backing,
stable disjoint metadata and the allocator's separate live-cell contract.
They do not establish original stack placement, physical cross-frame aliases,
native EH/register residue or ABI compatibility. The flag's upper three bytes
remain caller initialized; only its low byte is updated during descent.

## Link, capacity exception and balancing

The prior `STL_xlen_throw_0046ad70` label described only one branch. The unsigned
count threshold is15555554h. That branch builds the full19-byte counted legacy
SBO message `map/set<T> too long`, constructs the actual28h logic-error storage
and overwrites its vtable withD69260 before the original BF6885 throw.
The Source reuses the existing owning `NativeHardwareLayoutTreeLengthError`;
it preserves raw legacy storage, ownership and copy/destruction operations.
Its host throw/catch/RTTI and current allocator services are distinct from the
original D83F98 throw descriptor and FH3/CRT ABI.

Handler C61CB8 points to FuncInfo D893DC, one unwind state. Unwind map D893D4
contains {-1,C61CB0}. That action loads the completed SBO at EBP-50h and tail
jumps4072D0. The Source arms its completed-message scope only after counted
assignment returns. Failure before assignment completion does not add cleanup.
No allocation-frame preparation or tree write precedes the capacity check.

Success captures pair, then head, then parent; prepares allocator argument
cells color/pair/right/parent/left in native push order. The concrete0046A340
provider handles captured allocation, late argument copying and failure cleanup.
After allocation, reread head, increment the current count, and link using the
captured parent. Root writes reread head between root/leftmost/rightmost writes.
Only a nonroot insertion reads the current LOW byte of the left argument.
Leftmost/rightmost repair uses the freshly loaded head after the child write.

The accepted `rebalance_linked_tree_node<TreeInsertAccess<18h,19h>>` matches the
full native red/black repair block, including recolor reloads, the three direct
rotation calls and the mirrored final left rotation's inlined write/read order.
The concrete00467620/00467670 providers are used for direct calls. After current
root is blackened, read the current output cell and publish NODE then OWNER.
No rollback or node-string cleanup is invented after publication starts.

## Unique driver and duplicates

Capture pair once. Read initial root sentinel before writing initial flag low
byte. Each descent uses the accepted00443D00 predicate: zero stored length is
empty, otherwise current CRT `_stricmp`, right data read before left. Write flag
low byte before the next child read, then use the captured comparison result
for traversal. After descent write predecessor NODE first, OWNER second.

The saved last comparison selects the leftmost fast insertion or the complete
004678F0 predecessor decrement. Reload predecessor node after decrement. The
final predecessor comparison captures its node before calling the predicate.
A normal insertion reads the FULL DWORD flag, prepares child call words in
native push order and calls0046AD70. The fast path passes literal1. Publication
reads returned owner, current outer output, then returned node; writes final
NODE, inserted BYTE1, then OWNER. Duplicate publication reads outer output
then predecessor owner, writes captured NODE/BYTE0/OWNER. Padding9..B remains
untouched; no duplicate key retention or allocation occurs.

## Verification boundary

Exact whole native spans, two complete listings (167/111 instructions), helper
reload schedules, the cleanup map/action and concrete provider contracts were
inspected. Typed CALL/JMP rows are checked by `tools/verify_report_calls.py`.
Build and review results are recorded in the packet report when completed.
Existing regression CTests do not execute these bodies against the original.
Original binary ABI, original exception/allocator provider compatibility,
application adoption and game behavior remain unvalidated.
