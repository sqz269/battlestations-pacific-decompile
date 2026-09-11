# Native hardware-layout tree: next reconstruction boundary

Discovery only, based on `6e2fd5f` and the existing `bsp` Ghidra project,
`/battlestationspacific.exe`. The accompanying
`reports/native_hardware_layout_tree_next.json` contains 32 fresh whole-span
comparisons, 1,915 bytes total; every span matches the installed executable
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The three leased functions account for 904 bytes. Sixteen C++ source/header
hashes pin the available dependencies and the incompatible alias-list layout.
These are saved-analysis and file comparisons, not runtime or game proof.

## Result and proposed ownership

The missing renderer notification is a scan of an actual checked red-black
tree, followed by erasing the first node whose value pointer matches the
hardware layout. It is sufficiently bounded to reconstruct using the real
header, nodes and iterator. It cannot use the existing alias-list erase.

| Proposed packet | Addresses | Readiness and dependencies |
| --- | --- | --- |
| Tree primitives | `00B20860`, `00B20880`, `00B20910`, `00B22BD0`, `00B20DC0` | Ready; concrete raw storage and existing callable invalid-parameter binding. No exception-owner dependency. |
| Checked erase and notification | `00B2EF00`, `00B2F4C0` | Ready with the primitives from current main containing `39d0586`; all branches, complete native returns and owning exception dependencies are concrete. |
| Out-of-range ownership, reserved for the primary integrator | `00441760`, `004412B0` | Committed `39d0586`; both source files inspected and matched to the commit. The integrator reported build/CTest pass. The tree packet owns its host throw transport. |
| Hardware-layout terminal family | `00B60700`, `00B60770`, `00B48960`, `00B60110`, `00B483F0`, `00B48950` | Conditional next packet: requires completed notification/erase, actual CPU declaration dispatch, current renderer dispatch, and the existing actual resource-support domain. |

The first two rows can form one seven-address worker packet with proposed files
`include/bsp/native_hardware_layout_tree.hpp`,
`src/native_hardware_layout_tree.cpp`, `docs/NATIVE_HARDWARE_LAYOUT_TREE.md`,
and `reports/native_hardware_layout_tree_audit.json`. Start it from current main
containing the primary integrator's exception commit `39d0586`. Claim a new
implementation lease before editing.
This discovery changes none of the packet ledger, CMake, C++, tests or Ghidra.

Proposed frozen notification API for the hardware owner:
`void remove_native_hardware_layout_value_00b2f4c0(void* actual_tree_0108d530,
void* target_value, const SingletonLifetimeCallbacks& invalid_parameters);`
It borrows the actual header and executes the scan/erase directly. The hardware
owner retains current renderer/current table `+44` selection. Original renderer
`ECX` is unused by the notification function.

## Actual storage and service boundary

The tree identity is `0108D530`, with head pointer at `+04` (`0108D534`) and
unsigned count at `+08`. The leading DWORD at `+00` is not interpreted by these
functions. A checked iterator is two actual Win32 pointer words,
`{input_owner+00, node+04}`. Its owner is an address identity, not a copied header.

| Node offset | Observed interpretation |
| --- | --- |
| `+00`, `+04`, `+08` | Left, parent, right pointers; the head uses them as minimum, root, maximum. |
| `+0C..+1F` | Opaque key/payload bytes; removal never reads, copies or destroys them. |
| `+20` | Hardware-layout value pointer tested by the notification scan. |
| `+24` | Color byte; observed deletion fixup uses black `1`, red `0`. |
| `+25` | Sentinel flag byte; nonzero means sentinel. |

The minimum accessed extent is `0x26`; this packet does not establish the
allocation size or the meaning of the opaque key. No substitute key type,
semantic map, companion ownership registry, or COM retain/release belongs in
the tree erase. The original node is freed once. Existing
`NativeRenderResourceAliasNode` instead stores next/previous at `+00/+04` and
string metadata at `+08/+0C`; its iterator's two-word shape does not make its
list operations interchangeable with this tree.

`BF65AC` jumps to `BF9DC8`; the existing shared-domain `BF6989` jumps to
`BF65AC`. Thus the existing `singleton_lifetime_free` domain can service this
free. The existing `SingletonLifetimeCallbacks::invalid_parameter` is a
callable, potentially returning service; bind that actual domain. Do not add a
no-op adapter or assume that invalid-parameter handling cannot return.

## Complete owned functions

| Address and exclusive end | Native entry and return |
| --- | --- |
| `B2F4C0..B2F53E`, 126 bytes | Stack value pointer; renderer `ECX` is unused; `RET 4`, no semantic return. |
| `B20DC0..B20E23`, 99 bytes | `ECX` actual checked iterator; normal `RET`, invalid-sentinel tail jump to `BF6713`; incidental `EAX` is not a public result. |
| `B2EF00..B2F1A7`, 679 bytes | `ECX` destination tree; stack output-iterator address, input owner, input node; `RET 0Ch`, `EAX` output address. Input iterator is by value. |

`B2F4C0` captures the initial global head once and starts at its left/minimum.
It carries the iterator owner and node in `ESI/EDI`. Each iteration checks for a
null or foreign owner through `BF6713`, then compares the captured node to the
initially captured head. Before reading `node+20`, it separately checks a null
owner and equality with that owner's *current* head; returning handlers do not
cause immediate register reloads or strengthened revalidation. A mismatch
increments the stack iterator through `B20DC0`, then reloads its owner and node
for the next iteration. A match calls `B2EF00` with the canonical destination
tree and that by-value iterator, then exits. Only the first match is removed.

`B20DC0` first calls the invalid-parameter service if the iterator owner is
null, then loads the current node. A nonzero node sentinel flag performs a
tail jump to the same service; if it returns, it returns directly to the
iterator's caller without advancing. Otherwise, a nonsentinel right child
selects that subtree's leftmost node. If the right child is sentinel, it climbs
parents while the current iterator node is the parent's right child, publishing
each climbed parent into the iterator as it goes, then publishes the final
successor. No equality test against the owner's head is added here.

`B2EF00` initially tests the input node's sentinel byte. Nonzero takes the owning
out-of-range path below. The normal path captures the original node, advances
the by-value iterator, and chooses its replacement/fixup child. It does not
check input-owner equality with the destination tree. A two-child erase
transplants the successor node, patches links/root and swaps color bytes;
neither the opaque key nor the value pointer is copied. Single-child removal
also maintains head/root/minimum/maximum. The black-delete fixup uses symmetric
left/right rotations and current head/root loads.

Two pseudocode hazards are material:

1. The decompiler removes the live two-child transplant at `B2F00F..B2F065`.
   The report includes full raw disassembly. Follow the assembly's branches,
   sentinel guards, color swaps and write order.
2. Ghidra's current function body ends at `B2F170`, immediately after `_free`.
   Actual execution continues through `B2F1A6`: free the **original** node at
   `B2F16C`, load the destination's current count at `B2F171`, decrement only
   when nonzero, read the by-value input owner and advanced node after free,
   write output **owner first** at `B2F18F` then output node at `B2F196`, restore
   the exception frame, and `RET 0Ch` at `B2F1A4`. A zero count does not wrap.
   Existing alias-list erase publishes node first and has different count and
   sentinel behavior; reusing it would change observable behavior.

The extrema helpers return the selected node in `EAX`: maximum
`B20860..B2087C` (28 bytes), minimum `B20880..B2089B` (27 bytes), both `ECX`
node and `RET`. Their decompiler `void` signatures are misleading. Right
rotation `B20910..B20962` (82 bytes) and left rotation `B22BD0..B22C1E`
(78 bytes) take `ECX` tree and stack node, `RET 4`; they update the head's root
or the parent's appropriate child and guard sentinel child-parent writes.

## Owning exception closure

The invalid-iterator branch initializes the observed fields of a 1Ch SBO
temporary (capacity 15, length 0, first byte zero), assigns 27 bytes from
`CE44E0`, `invalid map/set<T> iterator`, through `408720`, then arms state 0.
It constructs a 28h exception through existing `411700`, overwrites its native
profile with `D6926C`, and calls `BF6885` with ThrowInfo `D863A8`.

`CBD7A8` loads FuncInfo `DF6024`, whose sole unwind entry at `DF601C` is
`{-1, CBD7A0}`. The cleanup computes the temporary at `EBP-50h` and tail-jumps
to existing `4072D0`. This cleanup is armed only after counted assignment
completes, and remains active across exception construction and throwing.

ThrowInfo selects cleanup `4412B0`, CatchableTypeArray `D863E4`, and primary
CatchableType `D863F4`: size 28h, copy routine `441760`.

- `441760..441779` is a standalone 25-byte entry missing from Ghidra's current
  function definitions. It takes `ECX` destination and stack source owner,
  calls existing `4118D0`, writes `D6926C`, returns the original destination in
  `EAX`, and `RET 4`. It adds no EH frame; the called copy owns failure cleanup.
- `4412B0..4412E2` is a 50-byte owning destructor. It writes logic-error profile
  `D69248`, frees the member heap buffer when capacity is at least 16, executes
  the missed `ADD ESP,4` at `4412C8`, resets member capacity/length/first byte,
  then tail-jumps to base destruction `BF6454`. It does not free owner storage.

The existing `NativeLegacyExceptionStorage`, logic-error constructor/copy and
SBO helpers provide the actual storage operations. The owning throw pattern in
`native_alias_count_growth.cpp` establishes the relevant host interface shape:
completed-temporary RAII, an owning exception, copy and destructor. The new
out-of-range owner must retain `D6926C` and use its own exact copy dependency.
A bare `std::out_of_range` or a callback that merely reports an error would
skip the observed allocations, copies, cleanup and native profile data.
Native metadata words remain evidence data; they do not create callable host
vtables or establish compatibility with the original exception ABI.

During discovery closeout, the primary integrator supplied frozen APIs
`copy_native_tree_out_of_range_00441760` and
`destroy_native_tree_out_of_range_004412b0` in
`native_tree_out_of_range_exception.hpp/.cpp`. Both files were read directly
from its main checkout, separately hashed in the report, and matched to its
commit `39d0586`; they are outside this worktree's base. They perform the copy/profile write and exact owning
cleanup described above. The integrator reported Win32 build and two existing
CTest checks passing; this discovery did not rerun that build. It also reported
defining `441760`, repairing the `4412C8` free continuation, saving the two names
and refreshing exports. These annotation changes followed the initial capture
above. The seven-address
tree worker should own the host exception class and temporary-guard transport,
and depend on these APIs without editing their files.

## Hardware terminal follow-on

Fresh bytes confirm the prior actual-binding report's chain. At `B48983`, base
layout destruction loads current renderer `F8D394`, then its current table's
`+44` slot and calls it with the layout. The installed slot word at `D5F0EC`
contains `B2F4C0`; this is a verified profile entry, not a claim about a live
game's current renderer. The header and node storage remain actual caller-
supplied storage when reconstructed.

- `B60700..B60765` publishes hardware profile `D62AF4`; releases captured COM
  declaration `+40` through its current COM table `+08`; clears `+40` only after
  successful return; calls actual resource support `B3E730` even though its
  result is unused; disarms state 0 and calls base layout destruction `B48960`.
  FuncInfo `DFA084`/map `DFA07C` routes state 0 through `CC12F0` to `B48960` if
  COM release or singleton lookup throws. Do not duplicate base cleanup if the
  explicit normal base destruction throws.
- `B48960..B489CF` publishes base-layout profile `D61D10`, notifies the current
  renderer, destroys four records at `+08` with stride `0Ch` in reverse through
  `BF7C6E`/`B483F0`, then calls existing base destructor `BD30F0`. Its two-state
  map `DF80BC` routes state 1 through `CBF6D8` to array helper `B48950`, then
  state 0 through `CBF6D0` to `BD30F0`. Before the normal vector destructor call
  it changes to state 0, leaving remaining-element cleanup to the CRT iterator.
- `B483F0..B48419` loads record `+00`; if nonnull, atomically decrements the
  referenced object's `+04` through `CE2220`. Only a zero result calls that
  object's current primary table `+00`. The record is cleared only after that
  call returns. Actual CPU declaration terminals now exist in the pinned
  `native_vertex_declaration_owner.cpp`, but dispatch must use the real profile
  and current storage. No synthetic callback may silently discard this release.
- `B48950..B48960` is the 16-byte EH array wrapper for four `0Ch` records and
  `B483F0`; `ECX` points at the first record, `RET`. It remains a required,
  named-but-incomplete dependency even though the normal destructor inlines
  the equivalent CRT call setup.
- `B60770..B60790` calls the hardware destructor, then only for `flags & 1`
  returns the slot to canonical pool `108FE9C` through `B60110`. `ECX` owner,
  stack flags, `RET 4`, `EAX` original address (including after pool return).
- `B60110..B60175` uses the real critical section at pool `+0C`, increments then
  decrements activity DWORD `+24`, reads slab index from slot `+44`, and reads
  the slab from pool `+28`. Slot stride is `48h`; the signed reciprocal division
  computes its slot index, stored as a WORD in slab `+900 + 2*old_count`. Slab
  `+940` is the WORD free count, incremented with WORD wrap. The unsigned minimum
  slab index is maintained at pool `+34`, then the real lock is released.
  `ECX` pool, stack slot, `RET 4`; no substitute allocator or lock is created.

Before accepting the follow-on family, close both the tree notification and
the current dispatch of the four retained CPU declaration records. The already
reconstructed actual resource support is usable with its published global,
lifetime domain and real lock; a placeholder singleton lookup is insufficient.
No shader/input semantic key, insertion algorithm, constructor, general tree
destructor, full renderer binding, or hardware allocation path is claimed here.

## Verification and annotation handoff

Every live capture passed through `bsp.py ghidra`, which verifies the intended
project/program before querying. Raw bytes supplement incomplete pseudocode;
the owned erase/iterator/notification and two exception terminals have complete
raw disassembly in the JSON. Existing names and available comments were
captured without mutation. At implementation time, re-read annotations under
the write lock, preserve previous values, create the missing `441760` function
and repair the erase extent/no-return issue only with the appropriate lease and
lock, rename proven functions, save the project, and refresh affected exports.
The primary integrator's subsequent `39d0586` already owns the two exception
entries; recheck current state and do not repeat its changes.

No C++ changed, so no build or tests were run for this discovery. A later C++
implementation requires the repository's MSVC Win32 build and proportionate
existing checks. If an added test is justified, the concrete risk is the
assembly-only two-child transplant plus the complete post-free return path;
avoid a broad generic container test suite. No ABI or game-validation claim.
