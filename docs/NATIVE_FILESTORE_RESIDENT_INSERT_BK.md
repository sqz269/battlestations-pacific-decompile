# Actual FileStore resident insertion

Addresses: `00BE4D20`, `00BE6CF0`, `00BE6590`, `00BE6170`, `00BE660E`.

The new resident-insertion source operates on the existing actual tree and
retained stream records. It supplies the predecessor and link/rebalance
operations used by the primary's BE7340 unique-insertion body. Names are
descriptive hypotheses; explicit string and validation services change the
source interfaces from the original ABI.

| Entry and inclusive span | Original ABI | Coverage |
|---|---|---|
| BE4D20..BE4DA8, 137 bytes | ECX iterator `{owner,node}`; RET or invalid-parameter tail | complete |
| BE6CF0..BE6EDB, 492 bytes | ECX tree; stack output, left-byte DWORD slot, parent, pair; EAX output; RET10h | complete |
| BE6590..BE660D, 126 bytes | five stack DWORDs: left,parent,right,pair,color; EAX node; RET14h; no consumed input ECX | complete with catch below |
| BE6170..BE61F0, 129 bytes | ECX output pair; stack source; EAX output; RET4 | complete, delegates established BE6250 |
| BE660E..BE6622, 21 bytes | compiler catch-all funclet, original parent EBP frame; no normal return | complete C++ free/rethrow schedule, new EH transport |

The last catch body is only a Ghidra function through BE6616. Its returning
free continuation BE6617..BE6622 has `no_ghidra_function`, with both endpoints
inclusive. The raw BF6885 rethrow call at BE661E is retained as a numeric report
row; the integrator must repair this membership before its call check can pass.
The packet does not mutate, rename or save Ghidra.
The final checker result is 21 numeric rows checked, one failure at BE661E;
the imported InterlockedIncrement row is explicitly outside direct-call proof.

## Layout and pair construction

Existing producer BE55E0 and resident-tree ownership establish head/count at
tree+4/+8 and 1Ch nodes. Links are left/parent/right at +0/+4/+8; the 0Ch pair
starts at +Ch and contains string length/data followed by retained stream at
node+14h. Color is +18h (0 red,1 black), nil is +19h, final bytes +1A/+1B remain
untouched. No std::map, replacement sentinel or shadow stream count is added.

BE6170 and the primary's BE6250 have identical complete instruction sequences
after normalization of relative calls/branches and handler addresses. Both
clear the name header before the identity guard, use existing actual resize
and overlap-aware copying, clear pair+8, then reread source+8. A nonnull stream
is published before real InterlockedIncrement(stream+4). Self-copy therefore
clears the stream before rereading it and does not retain the old stream.

The source delegates BE6170 to `copy_native_file_store_resident_pair_00be6250`
from `native_filestore_completion.hpp`. It uses the same `NativeStringStorage`
instance supplied by the surrounding owner. NativeStringStorage release is
already noexcept; original throwing pool-getter and hardware-fault behavior
are not newly established by this delegation.

## Allocation, predecessor and insertion

BE6590 allocates exactly 1Ch through the shared original-new service boundary.
If nonnull, it writes left, parent, right, constructs pair+Ch, then writes the
requested low color byte and nil=0. It does not initialize the padding or
publish a tree entry before the pair is complete.

Its E01550 FuncInfo contains three states and one catch-all range. State1 to
state0 executes CC6CC0..CC6CD0: two placement arguments, CALL 401130 at CC6CC8,
ADD ESP,8. That target is one RET. The catch at BE660E reads the stored raw
allocation, frees it at BE6612, and rethrows with BF6885(0,0) at BE661E. Source
catch/free/rethrow retains that schedule without inventing name destruction,
stream release or rollback of partially modified external string storage.

BE4D20 first validates only that iterator owner is nonnull. A returning handler
continues with the current node. End moves to sentinel.rightmost, publishes
that node and validates its nil byte. A live node with a left child moves to
the maximum in that subtree. Otherwise it climbs parents while the current
iterator node is their left child, publishing each climb. Final invalid-node
validation returns without a later overwrite; otherwise the final parent is
published. No owner-equality or begin guard is invented. The checked fixture
confirms that decrementing begin reaches the sentinel under a valid tree.

BE6CF0 rejects unsigned count >=15555554h before allocation. The complete
length-error path builds the native SBO message from all 19 verified bytes at
CE47BC (`map/set<T> too long`), constructs the actual legacy logic-error
payload, publishes length-error identity D69260, and throws via the existing
owning `NativeHardwareLayoutTreeLengthError` source transport. The temporary
message's cleanup is armed only after assignment; E01634/E0162C routes state0
to CC6D40..CC6D47 and the existing 4072D0 string destructor. Native D83F98 RTTI,
FH3 and SEH identity remain explicit source boundaries.

For admitted counts the routine captures the original head for the new left
and right links, allocates/copies, reloads current head, increments current
count, and links beneath the supplied parent. Empty-root linking updates
root/leftmost/rightmost; other branches update the applicable extreme. It then
performs the original red-black recolor/rotation loop and blackens the current
root. Output node is published before output owner; bytes beyond the two-word
iterator are untouched. No comparator or duplicate detection belongs here.

The established BE4980/BE50E0 actual right/left rotations were read in assembly
and reused. BE6E66..BE6EA2 is an inline copy of the same left-rotation schedule;
its source reuse is recorded as inline correspondence, not a fictitious native
CALL. Signedness, nil checks and current-link reloads follow the full listing.

## Calls and verification

The report lists every owned CALL/tail call and all recorded incoming sites
with their actual enclosing body/range. The only external callers of BE4D20
are two same-ECX wrappers at BE5253 and BE5473 plus BE7340's predecessor step at
BE73EF. BE7340 invokes BE6CF0 at BE73C8 with left=true and BE7416 with its
captured comparison result. BE6CF0 calls BE6590 at BE6D74; BE6590 calls BE6170
at BE65EA. Every semantic callee was inspected before binding it.

Strict MSVC Win32 `/MD /O2 /W4 /WX /fp:strict` source compilation passed with
the primary include directory after this packet's include directory. The
ignored focused fixture linked `/MANIFEST:EMBED` and passed a 12-insertion
scenario covering both rotation directions, red-black invariants, parent links,
extrema, original stream references, complete reverse traversal and a returning
validation callback that repairs the initial iterator. It also verifies a
throwing string allocation leaves tree/count/references unpublished and the
length guard preserves the native 28h exception identity and 19 message bytes.

The fixture uses the unchanged BE6250 and helper source blocks extracted from
an exact frozen primary source file; only fixture includes/namespaces surround
them. Their individual hashes and full-source provenance are recorded. This
avoids requiring unrelated still-pending FileStore completion definitions to
link a bounded insertion fixture. No fake unresolved operation is supplied.

The fixture's string service forwards to existing CRT string storage and can
throw for the explicit allocation case. Nodes and intrusive reference words
use their actual layout. This verifies source behavior and host services, not
original-code differential execution, original allocator/RTTI/FH3 ABI, complete
owner destruction or gameplay. The source proof for raw node freeing is the
verified catch listing and generated source; the fixture does not intercept
CRT free to claim an allocation trace. CMake registration, whole-program build,
root BE7340/callback integration and the remaining catch membership repair are
primary integration work. No permanent tests or shared metadata changed.
