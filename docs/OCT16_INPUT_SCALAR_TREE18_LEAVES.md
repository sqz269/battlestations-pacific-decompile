# Actual scalar/int-key tree18 leaves

Source56 reconstructs complete `0069B190` (137 bytes/48 instructions),
`005530D0` (78/30), and `00553120` (82/30): 297 defined Native bytes and
108 instructions. These are direct MSVC Win32 naked implementations on actual
caller-owned storage. They add no carrier, node overlay, provider interface,
context, validation, allocation, payload operation, repair, or rollback.

The authorized worker basis is published ancestor
`424a87e2a82103d137094c71146030766c277e0b`; exact published Main
`dcea22c446e3537a757f6bbfa3c9cf854f8e6772` contains it and has identical
LF bytes for all eight fingerprinted provider/family files. The worker did not
merge foreign leased shards. The Source54 publication receipt is immutable.

| Entry | Existing Ghidra name retained | Native and emitted Source ABI |
| --- | --- | --- |
| `0069B190` | `STL_Tree_Dec_IntKey_N18A_0069b190` | ECX actual iterator; entry EDX unconsumed; no stack input, plain RET; semantic void, incidental EAX; ESI saved/restored |
| `005530D0` | `STL_Tree_Rotate_005530d0` | ECX tree; entry EDX unconsumed; stacked pivot; three RET4 exits; semantic void, EAX retained promoted node; ESI saved/restored |
| `00553120` | `STL_Tree_Rotate_00553120` | Same register/stack shape as left rotation; fixed right rotation schedule |

The Original stored prototypes remain `undefined name(void)`. No Source
prototype or Ghidra annotation is applied by this packet. Public Source APIs are
declared in `include/bsp/native_input_scalar_tree18_leaves.hpp`; they have the
measured fastcall register/stack shapes, with explicit unused EDX parameters.
Static instruction agreement does not establish original-address replacement,
Original fault PCs, runtime binary compatibility, or game behavior.

Actual storage is an eight-byte iterator (owner DWORD0, node DWORD4), tree head
DWORD4, and 18h-byte nodes (left0/parent4/right8, keyCh/mapped10h,
color BYTE14h/nil BYTE15h). The three bodies do not interpret or copy payload.
All field accesses are literal emitted raw instructions, including overlapping
storage effects; there are no C++ node/owner lvalues or host container models.

The predecessor tests current iterator owner, invokes the real admitted SDK
provider if zero, and reads CURRENT node only after that diagnostic returns.
A nil node publishes captured current right before testing its nil byte. A bad
right restores ESI then physically tail transfers to the SDK provider, with no
later owned store even if the provider returns. A nonempty left branch carries
the captured right link through the maximum loop and publishes the maximum
once. The climb tests the captured parent nil byte first; each iteration reads
CURRENT iterator node before parent.left, publishes parent before reading its
next parent, and tests that new parent's nil byte. Its final CURRENT node nil
test precedes final parent publication. The second bad-node tail also restores
ESI and has no post-diagnostic publication. Two seven-byte LEA ESP,[ESP]
instructions are retained as actual instructions, including their positions;
they are not deducted as padding. Both saved Ghidra E9 sites retain their
`CALL_RETURN` overrides; no metadata repair or no-return assumption is made.

Both rotations capture pivot and promoted node before saving ESI. They write
the pivot near link, then REREAD the promoted near link for the literal nil15
test. They read CURRENT pivot.parent after the possible child-parent store,
store promoted.parent, then read CURRENT tree.head and compare its root.
Nonroot paths REREAD pivot.parent after that publication/comparison before
testing the current parent side slot. Root and nonroot paths publish the
appropriate head/parent link, promoted far link, then pivot.parent. ESI is
restored before root branching. No count, extrema, color, payload, or owner
field is written. Exact instructions also retain the path's incidental EAX and
arithmetic flags, subject to the reached SDK's own contract on diagnostic paths.

These are all actual external Native transfers in the owned bodies; rotations
have none. The provider is existing Source and is not newly reconstructed here.

| Actual Native transfer | Owned function | Admitted Source provider / scope |
| --- | --- | --- |
| `0069B198 CALL 00BF6713` | `0069B190` | `invoke_native_invalid_parameter_00bf6713`; current SDK noinfo policy |
| `0069B1B3 JMP 00BF6713` | `0069B190` | Same provider; ESI restored, no later owned action |
| `0069B20F JMP 00BF6713` | `0069B190` | Same provider; ESI restored, no later owned action |

The SDK provider may return, throw, or terminate. Its current SDK noinfo policy
does not close Original BF66EF encoded handler selection, Watson/debugger,
PTD/errno/bootstrap, or Original CRT exception transport. No new callback,
default service, catch, noexcept adapter, or failure translation is introduced.
Caller stack, DF, readable/writable storage lifetime, and transitive provider
progress/alias effects remain inherited. There is no Original FH3/SEH, private
caller-frame, heap ownership, hardware-fault, live ABI, or game validation claim.

Fresh Source-native evidence verifies `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, all 297 PE/live bytes, all 108 current starts, all
19 encoded internal/external edges, whole typed extents, and stored prototypes.
Readonly family comparison uses published pointer-tree18 leaves; it is not a
delegate or promotion of their Native bodies. Old frontend callback-based
factory evidence remains historical and supplies no admitted allocator here.
Unique/hint/map/insertion/factory bodies and Source55 are outside this packet.

Normal `./scripts/build.ps1` exited 0; existing suites passed 3/3 in 8.72s.
No tests or probes were added, and these three Native entries were not executed.
The pre-existing unrelated LNK4006 spawn-request duplicate warning remains.
The actual 1471-byte object has six sections: directives47, CodeView212,
three public CODE sections137/78/82, and checksums48. All 108 instructions,
symbols/auxiliaries, section headers, metadata, and all file bytes were read.
No helpers, code padding, EH/SAFESEH section, BSS, literal, or dispatch table is
emitted. The three zero-addend REL32 bindings are solely to the real SDK Source
symbol at predecessor offsets9h/24h/80h. All 285 other CODE bytes and all
instruction boundaries/internal branch displacements equal Native exactly.
The rotations' complete 160 CODE bytes are identical to their Native bodies.

Before building, 123 currently cited object paths were archived by content
hash, including Source54 bf9, map0584, current unique a62e, and historical
f963/4338. All 459 older JSON receipts, all 123 current paths and immutable
copies remain exact after building; no historical path/hash receipt was
rewritten. Source56's original eight-check Source receipt had a descriptive
line-count typo and remains immutable; its qualified successor269b records the
correct 170 lines and unchanged Source hashes. Source56 freeze cites that
qualified receipt. Ghidra annotation and publication require Root review.
