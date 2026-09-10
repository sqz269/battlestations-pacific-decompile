# FileBlock destruction and paired VFS block exit

Addresses: 00bdaf40, 00bdc9b0, 00bdcb30, 00bdebe0.

FileBlock destruction invokes VFS block exit, releases the identifier string,
and destroys the base object. Exit restores the saved gate byte and removes
one list node. It clears the current block name; it does not restore the
parent's name. The FileBlock destructor reloads the current global VFS manager
instead of retaining the manager used at construction.

These conclusions pair with the independently established entry contract in
`FILE_BLOCK_SETUP.md` and constructor/caller evidence in
`VFS_LOAD_PROCESSING_START.md`. They describe the normal, matched, non-reentrant
path. Constructor-failure rollback and exception unwinding remain explicit
boundaries, not an implied RAII guarantee.

## Fresh evidence and ABI

Every live query went through `python tools/bsp.py ghidra ...`, which verifies
project `bsp`, program `/battlestationspacific.exe`, language
`x86:LE:32:default` and image base `00400000` before querying. Configuration
names project `C:/Users/sqz269/bsp.gpr` and binary
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
The disk SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Four complete Ghidra-byte spans matched the installed PE. Ends are exclusive
and include final RET operands. The approved two-callee closure is the actual
manager exit function `00bdc9b0` and its list erase helper `00bdaf40`.

| Start | End | Bytes | Observed ABI |
|---|---|---:|---|
| `00bdebe0` | `00bdebfe` | 30 | ECX FileBlock; stack flags; EAX original object storage on both paths; RET4 |
| `00bdcb30` | `00bdcbac` | 124 | ECX FileBlock; no stack args; RET; no stable result contract |
| `00bdc9b0` | `00bdca75` | 197 | ECX VFS manager; stack name-wrapper pointer; RET4; no stable result contract |
| `00bdaf40` | `00bdaf9e` | 94 | ECX list; stack output-iterator pointer, iterator-owner list, node; EAX output iterator; RET0Ch |

Two data spans also matched: `-FileBlock %s` at `00d68484` and the first two
FileBlock vtable slots at `00d68494`. Those slots are `00bd30e0` and `00bdebe0`.
Hashes, original names/comments and proposals are in
`reports/file_block_lifetime_audit.json`; raw captures and complete offline
disassembly are in ignored `exports/bsp/parallel_file_block_lifetime/`.

## FileBlock object destruction

`00bdcb30` installs vtable `00d68494`, reloads `[0109ceec]` into ECX at
`00bdcb54`, and passes `&FileBlock[+14h]` to `00bdc9b0` at `00bdcb66`.
Only after manager exit returns does it destroy the name's allocation, using
stored length plus1, then call base cleanup `00bd30f0`. It makes no source-stream
retain/release and stores no captured manager pointer in this body.

The paired constructor also reads global `[0109ceec]` for entry. Consequently,
replacing that global between construction and destruction changes the manager
that exit receives. A host wrapper that always exits a captured original
manager would be a deliberate ownership policy, not a recovered native rule.

`00bdebe0` calls this non-deleting destructor, frees FileBlock storage only
when flags bit0 is set, then writes the original object address to EAX before
RET4 on both paths. It does not decrement the object's reference count itself.
The existing generic slot0 helper `00bd30e0` dispatches virtual `+4(1)`; a direct
virtual `+4(1)` call likewise enters this deleting path. Neither path adds a
second decrement inside these audited destructor bodies.

## Exit order and state restoration

All offsets below refer to the VFS manager, a different receiver from the
FileBlock object.

1. If manager byte `+78h` enables observation, call observer `+88h` virtual
   `+0Ch`. A nonempty input name is passed directly; an empty input name selects
   the current-name wrapper at manager `+0Ch` instead. This occurs before any
   exit-state mutation.
2. Clear the current name at manager `+0Ch/+10h` through the existing native
   string resize/empty-copy operation.
3. Decrement DWORD nesting depth `+14h`.
4. If current gate byte `+79h` is nonzero, call the diagnostic helper with
   `-FileBlock %s` and the explicit input name. This call does not use the
   observer's empty-name fallback, and its actual external output remains
   outside this audit.
5. Fetch the last saved node through sentinel `manager[+80h]` then sentinel
   `+4`. After non-end checks, copy its byte `+8` into manager gate `+79h`.
6. Fetch the current last node again and erase it through `00bdaf40`, using
   manager `+7Ch` as both list receiver and iterator owner. The returned iterator
   is stored only in a local temporary.

No previous-name text is read from these nodes or restored by this exit body.
For ordinary nested entry/exit, leaving an inner block yields the previous gate
and outer depth but an empty current name. The outer FileBlock object still
owns its explicit identifier and supplies it at its own destruction.

The following table makes the callback timing precise:

| Observation point | Gate | Depth/name | Saved-node stack |
|---|---|---|---|
| Entry observer virtual `+8` | Already ANDed with new gate | Still previous depth/name | New saved-gate node already linked |
| Exit observer virtual `+0Ch` | Current block gate | Current depth/name still present | Current saved-gate node still linked |
| Successful exit complete | Previous saved gate restored | Depth decremented; current name empty | Last node removed |

The exit observer therefore runs before restoration. Both entry and exit can
expose intermediate state to a reentrant callback. This audit does not promise
balanced nesting if callbacks independently change the block stack or global
manager. There is no explicit observer or input-wrapper retain/release here;
their internal behavior remains a separate boundary.

## List node destruction and raw tail

`00bdaf40` checks that its iterator-owner argument is nonnull and that the node
is not that owner's sentinel. For a node distinct from the receiver's sentinel,
it remembers node `+0` as the next node, assigns `previous->next = next` and
`next->previous = previous`, frees the node, and only then decrements receiver
list size `+8`. It stores `{iterator_owner, next_node}` into the output iterator
and returns that output pointer in EAX with RET0Ch.

The raw continuation after free call `00bdaf7f` is `00bdaf84..00bdaf9d`. It
contains the size decrement and output/return sequence omitted by captured
pseudocode. The Ghidra body already includes these bytes through a shared
epilogue, so a repair should inspect the specific call's flow override rather
than blindly changing the shared free routine or extending this function.
The scalar destructor similarly restores EAX at `00bdebf8` after its optional
free at `00bdebf0`, despite captured `extraout_EAX` pseudocode.

The manager exit invokes `00bf6713` if the saved-node list is empty, at three
validation points, and the erase helper has its own invalid-iterator calls to
the same routine. That runtime helper's handling was not reconstructed. Exit
has already notified the observer, cleared the name and decremented depth
before the first empty-list check. It is not a safe empty-stack no-op. The
meaningful normal-path precondition is a matching live saved node.

## Failure and implementation boundary

The constructor packet established the normal sequence: allocate/copy name,
prepare identifier, then call global manager entry. Its owner did not audit
constructor unwind funclets and found no explicit normal-body catch/rollback.
The FileBlock destructor here writes EH states around manager exit, string
cleanup and base cleanup, but those states alone do not establish unwind
actions or constructor-failure balancing.

In particular, if the entry observer throws, the entry body has already linked
the saved node and changed the gate but has not incremented depth or copied
the new current name. No compensating leave has been proved. If the exit
observer throws, exit's normal name/depth/gate/node mutations have not yet run.
Possible compiler-generated string/base cleanup and propagation must be
verified in the corresponding unwind funclets before claiming a fully balanced
exception-safe scope. A node-construction or list-growth failure also remains
outside the established normal path.

No C++ was added. The current typed manager has no concrete FileBlock tracing
state or observer lifetime implementation. A future scoped implementation needs
explicit manager initialization/destruction, observer virtual `+8/+0Ch`
contracts, reentrancy/global-replacement policy and error/unwind handling. It
must preserve the native empty-name result after exit, instead of automatically
restoring a parent name. Keeping a stable manager alive would be an explicit
host behavior choice.

This packet changes only its documentation/report and ignored evidence. It
makes no Ghidra or shared-ledger edits, runs no build/runtime tests, and claims
neither native ABI compatibility nor game validation.
