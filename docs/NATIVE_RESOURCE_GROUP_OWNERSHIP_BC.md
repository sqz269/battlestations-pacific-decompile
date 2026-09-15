# Resource group ownership through the raw name domain

Addresses: `00B8F680`, `00B8F8C0`, `00B86780`, `00B8F4F0`, `00B8F460`.

The resource group factory returns an already constructed native owner. The
existing `NativeGroupOwner` interface could only prepare storage for a later
semantic-name construction, and its destruction path always selected that
semantic domain. A factory group constructed with `NativeStringRawPoolContext`
therefore could not complete this canonical ownership path.

`NativeGroupOwner(NativeGroupStorageView, NativeGroupEnvironment&)` now adopts
the existing prefix and tail. It validates their contiguous layout and group
profile, binds the same node identity into the existing scene and attachment
runtimes, and enters the live phase. It does not construct the native object
again, copy its fields, change its count, publish another type, or allocate a
substitute name pool. The fresh factory owner has no notification owner at `A0h`;
the existing node-binding validation enforces that precondition.

On binding failure, the caller retains its constructed owner. Failure after
scene admission removes only that new binding. No native prefix/tail destructor
runs as compensation for a failed adoption. The preconstruction interface and
its original cleanup remain available to the existing GUI path.

Group destruction now calls the complete `00B6F440` provider with the immutable
name domain borrowed by its `NativeNodeDestructionRuntime`. Raw owners use that
runtime's existing raw context; semantic owners keep the existing path. The
native destruction order remains `00B8F680`: clear the actual attached count,
free its backing, then destroy the node. `00B8F8C0` returns the slot to the actual
group pool only when flags bit 0 is set. Companion unbinding and retirement use
the existing `NativeGroupReference` sequence after native cleanup.

## Focused ownership verification

Strict MSVC Win32 and both existing CTests pass. One source integration fixture
uses the complete `00B86780` factory twice, the actual group and string pools,
current type descriptors initialized through `NativeGroupTypes`, and live
Ghidra/installed-image vtable words. It verifies:

- Both adoptions and reference bindings preserve every byte of each actual
  `18Ch` allocation, including its reference count and trailing pool index.
- Duplicate adoption is refused without disturbing either existing scene
  binding or native owner. The ordinary duplicate scene-binding path is covered;
  allocator exceptions during companion admission are not dynamically injected.
- Canonical `00B6E680` parenting writes the actual parent/child and notification
  owner words between the two factory groups.
- Logical release through the root's `NativeGroupReference` releases the child
  first, then the root, and removes both scene/lifetime identities.
- The large root name is freed once; the small child name returns to the same
  raw string-pool ring. Both actual group slots are available in release order.

The native attachment sequence has a relevant quirk. `00B8F4F0` publishes the
new group into child `A0h` at `00B8F526` before calling `00B8F460` at `00B8F52F`.
The callee compares that same word at `00B8F468` and returns on equality, so this
path does not append the group child to the parent's `178h` reverse array. An
initial fixture assertion expected an append and failed. Inspection of both
complete native bodies confirmed the existing source; the assertion was fixed
without changing parenting behavior or fabricating a reverse entry.

Four complete native reference spans, 294 bytes, match the installed image and
live Ghidra. Eight direct-call rows are checked. Two existing Ghidra names are
preserved with appended ownership evidence and refreshed exports. No new native
body is claimed: this packet composes the existing factory, ownership, parenting,
logical-release and destructor implementations through their actual storage.

The fixture executes reconstructed source; it does not run the original native
destructor or exception runtime. The supported companion projection and raw-node
materialization limits remain. Populated scene attachment, arbitrary item
providers, the complete executable resource graph, and gameplay validation are
still outside this packet. No workers were dispatched.
