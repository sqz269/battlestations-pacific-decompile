# Raw gameplay-effect tree and construction

Ten complete source entries (732 original bytes) now supply raw tree operations
and construction for the gameplay-effect manager. The executable's current
typed manager remains a separate interface; these entries are prerequisites
for replacing that projection with the native owner and destruction chain.

| Address | Bytes | Source operation and original ABI |
| --- | ---: | --- |
| `008697B0` | 28 | Maximum: ECX subtree, EAX rightmost node, RET. |
| `008697D0` | 27 | Minimum: ECX subtree, EAX leftmost node, RET. |
| `00869810` | 82 | Rotate right: ECX tree, stack pivot, RET4. |
| `0086A2F0` | 78 | Rotate left: ECX tree, stack pivot, RET4. |
| `00869A20` | 99 | Increment checked iterator: ECX owner/node pair, RET. |
| `0086AA60` | 53 | Erase subtree: ECX tree, stack root, RET4. |
| `0086AC00` | 55 | Allocate blank node: no consumed input, EAX node, RET. |
| `00869C50` | 17 | Clear publication and reset base profile: ECX owner, RET. |
| `00870370` | 104 | Construct manager: ECX raw owner, EAX same owner, RET. |
| `004C1650` | 189 | Get singleton: no consumed input, EAX owner, RET. |

`native_int_pointer_tree18_leaves.cpp` retains the complete seven leaf
instruction schedules. Tree headers contain allocator+0, head+4 and count+8;
18h-byte nodes contain links+0/+4/+8, key/value+0C/+10, color+14 and nil+15.
The constructor and blank allocator leave payload and padding untouched.
Subtree erasure recurses right, captures left, frees the current node and
continues left. It never releases the value payload or modifies the header.

Rotations retain every root/parent branch, nil-child parent update, reread and
store order. Iterator increment allows the null-owner invalid handler to return
before rereading the node. A nil node tails the actual invalid service. The
alignment LEAs, including unreachable maximum padding, retain their encodings.
Fixed adapters call the actual source CRT allocator/free and SDK
`_invalid_parameter_noinfo`; there are no replacement containers or no-op handlers.

`native_gameplay_effect_construction.cpp` reconstructs the three owner entries.
Reset clears the actual volatile F87664 cell before writing CE3818 to the owner.
Construction arms cleanup before writing D0DA64 or allocating a sentinel. It
publishes head+8, marks nil, rereads the head before each self-link and clears
count+0C. Its failure path resets publication/base profile and rethrows without
adding a sentinel or owner free.

The getter uses the completed raw singleton manager, retaining the first
manager's raw section through entered-depth accounting and final release. It
rechecks publication, allocates10h, calls the raw constructor, ends allocation
cleanup, publishes its returned pointer, looks up the manager again and then
rereads the effect publication before registration. Constructor failure frees
the captured allocation after the constructor's own cleanup. A subsequent
lookup/registration failure retains publication/storage and invokes full411EE0.
The fast path returns its initial capture; the slow path rereads after Leave.

## Exception evidence and interfaces

Fourteen fresh live Ghidra/disk spans cover 732 owned bytes and143 EH bytes.
Constructor FuncInfo `DC7E74` uses map `DC7E6C`: state0 to -1 calls C95EC0,
which reloads the owner at EBP-10 and tails869C50. Getter FuncInfo `D8D620`
uses map `D8D610`: state1 to0 calls C64FA8 to free allocation EBP-18;
state0 to -1 calls C64FA0 to destroy guard EBP-14 through411EE0.

The leaf fastcall interfaces retain ECX and stack arguments, adding unused EDX
where needed. Reset/constructor add EDX as a stable reference to the actual
F87664 publication. The getter borrows distinct actual F87664 and01090AA0
cells through a new cdecl interface. No private publication or second domain
is defined. Original no-input callers cannot directly use these new bindings.

Source C++ cleanup covers propagated synchronous C++ exceptions. Original FH3/
SEH frame identity, aliases to private binding/EH spills, hardware-fault cleanup
and provider register/throw identities remain outside these interfaces. Native
profile DWORDs remain identity data, not callable rebuilt C++ vtables.

## Validation and next dependencies

The strict Win32 build, both existing CTests and eight native seed checks passed.
All422 native leaf bytes match the emitted bodies except five named CALL/tail-JMP
operands. The constructor's56 normal-operation bytes match after explicit native
frame removal; the13-byte reset body uses the added publication reference.
Compiled constructor/getter schedules, complete EH tables and SafeSEH entries
were reviewed. The actual archive contains1035 owned CODE bytes and236 EH/
SafeSEH bytes, with four whole provider objects unchanged from the prior build
and all six source-provider symbols resolved in the archive. Thirty unchanged
prebuild inputs, six actual compiler commands and196 read dependencies are pinned
in `reports/native_effect_tree_construction_audit.json`. No new tests were added
and none of the new source entries was executed by this packet.

The next complete source dependencies are iterator erasure `0086E8A0` and range
erasure `0086EE50`. All their raw traversal/rotation/subtree leaf providers now
exist. Iterator erasure still needs its full invalid-iterator exception path,
including owning legacy out_of_range transport, and verification beyond the
shortened stored body ending86EB10. Implement the full range routine, including
its partial-range path. The manager's empty range in a normal destructor does
not justify omitting that path.

Then complete raw manager destructor `0086FE20`, scalar deleting wrapper
`008703E0`, and their source publication bindings. Only after every owner admitted
to the canonical manager has an evidence-backed terminal can raw `00BD0400` and
the executable ownership domain be integrated. Full gameplay validation and
original callable ABI compatibility remain unproved. Descriptive names are
reconstruction hypotheses.
