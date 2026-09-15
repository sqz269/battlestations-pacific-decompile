# Native damageable crew-tree insertion

## Result and boundary

The signed `int32 -> 44h mapped value` tree used by the source-absent
`0087CA80` Emberkek path now has a complete source reconstruction in
`include/bsp/native_damageable_crew_tree.hpp` and
`src/native_damageable_crew_tree.cpp`. The closure covers lower-bound lookup,
checked hinted insertion, unique insertion, iterator comparison/decrement, node
allocation, both rotations, the full red-black repair, and the owning
length-error branch.

This is a strict MSVC Win32 source interface over caller-owned native-shaped
storage. It does not install the original FH3 metadata and is not a binary
replacement. The source-absent `0087CA80` reader and `004B1400` SoldierClass
resolution remain separate packets.

## Address and ABI evidence

All nine current Ghidra functions have complete listings with no instruction
gaps. Descriptive names below are hypotheses based on body behavior, not
recovered symbols.

| Address | Original body and ABI | Previous name | Reconstructed behavior |
| --- | --- | --- | --- |
| `00876340` | 41 bytes; ECX left iterator, stack right iterator, EAX boolean, RET4 | `STL_inst_00876340` | checked owner comparison, then node equality |
| `00876630` | ECX tree, stack node, RET4 | `FUN_00876630` | right rotation |
| `008767A0` | 137 bytes; ECX iterator, RET | `FUN_008767a0` | checked predecessor/decrement |
| `008773A0` | ECX tree, stack node, RET4 | `FUN_008773a0` | left rotation |
| `00877FE0` | five stack arguments, EAX node, RET14h | `FUN_00877fe0` | allocate and initialize a 58h value node |
| `0087A5F0` | 492 bytes; ECX tree, stack output/left/parent/pair, RET10h; FH3 `00C966F8` | `STL_xlen_throw_0087a5f0` | link node, repair red-black tree, plus length-error branch |
| `0087ADB0` | 185 bytes; ECX tree, stack output/pair, EAX output, RET8 | `FUN_0087adb0` | signed unique insertion returning iterator plus inserted byte |
| `0087B260` | 444 bytes; ECX tree, stack output/two-word hint/pair, EAX output, RET10h | `FUN_0087b260` | checked hinted insertion with unique fallback |
| `0087B750` | 147 bytes; ECX tree, stack int32 pointer, EAX mapped pointer, RET4 | `FUN_0087b750` | signed lower bound and insert-on-miss (`operator[]` behavior) |

The prior `0087A5F0` name was misleading. The string literal and throw occupy
only its count-overflow branch. The ordinary body calls `00877FE0`, links the
node, updates extrema, and performs the complete red-black insertion repair.

## Raw layout and field schedule

The tree header is opaque at `+0`, sentinel/head at `+4`, and unsigned count at
`+8`. Nodes are exactly 58h bytes:

| Node offset | Field |
| --- | --- |
| `+00h/+04h/+08h` | left, parent, right pointers |
| `+0Ch` | signed int32 key |
| `+10h..+53h` | 44h mapped value |
| `+54h` | color (`0` red, `1` black) |
| `+55h` | nil/sentinel byte |

`00877FE0` allocates first, writes left, right, and parent, copies all eighteen
DWORDs of the 48h key/value pair, then writes color and nil. It does not call
the sibling `00877FA0`: that routine zeroes links and writes a black color before
the caller repurposes its node as a sentinel, which is a different observable
store schedule.

`0087A5F0` throws when unsigned count is at least `038E38E2h`. Otherwise it
captures the current head for allocator arguments, allocates, reloads the head,
increments count, links the node and updates root/minimum/maximum. It repairs
red-red ancestry with the native recolor/rotation order and finally makes the
root black. Output publication is node followed by owner.

## Lookup, hint and duplicate behavior

`0087B750` performs a signed lower-bound walk. An exact hit returns node `+10h`.
On a miss it copies seventeen DWORDs from uninitialized stack storage into the
44h mapped part of a local pair, then calls the checked hint path. Those bytes
are deliberately not zeroed. After insertion it independently validates a
nonnull iterator owner and a non-end node through the returning invalid-
parameter boundary, then returns the mapped-value pointer.

`0087B260` preserves the checked-iterator schedule:

1. An empty tree inserts left under the head without validating the hint.
2. A nonempty tree validates hint owner before the minimum case, then validates
   it again on the remaining path. On that second path, it captures the current
   end/head before invoking the callback and compares the hint node with that
   captured pointer after the callback returns.
3. Minimum and end hints take their direct boundary insertions only for strict
   signed ordering. After the captured-end comparison succeeds, maximum lookup
   reloads the current head; a returning callback may have replaced tree `+4`.
4. An interior predecessor/successor gap inserts beside the available nil
   child. Successor traversal uses the existing complete `008772B0` provider;
   `00876340` checks the resulting iterator against end.
5. Every rejected hint falls back to `0087ADB0`, and only its owner/node words
   are copied to the hint output.

`0087ADB0` uses strict signed comparisons. A duplicate publishes its iterator
and inserted byte zero without copying or retaining the supplied mapped value.
A miss calls `0087A5F0` and publishes inserted byte one. The three output
padding bytes remain untouched in both cases.

`00876340`, `008767A0`, and borrowed successor `008772B0` all preserve the fact
that `00BF6713` can return. The source APIs accept
`SingletonLifetimeCallbacks`; after a returning callback, execution continues
from the same native fields rather than converting validation into an exception
or an early return.

## Emberkek consumer

The audited source-absent reader converts the Lua key to int32 at `0087D645`,
resolves the Lua value through `004B1400` at `0087D652`, calls `0087B750`, and
writes the returned SoldierClass pointer at mapped value `+40h` (`0087D665`).
That is node `+50h`. It performs no old-pointer release or extra retain at that
store. The other mapped bytes remain whatever the insertion copied until later
model binding fills them.

## Dependency and evidence boundary

The reconstruction reuses only reviewed providers:

- `advance_native_class_frame_008772b0` for the exact `+55h` checked successor;
- `singleton_lifetime_allocate` for the canonical `00BF681B` allocation/new-
  handler domain;
- native legacy string and logic-error ownership through the existing
  `NativeHardwareLayoutTreeLengthError` transport, which carries the actual
  `00D69260` storage identity but has new host RTTI/catch ABI;
- the shared raw-tree mechanics in
  `include/bsp/detail/native_tree_insert_storage.hpp`, instantiated only after
  comparing this function family’s offsets, count predicate, output order and
  rotation schedule.

No host `std::map` owns or shadows the tree. No unresolved callee was stubbed.
The strict MSVC Win32 repository build passed after compiling the new source,
and both existing CTests passed (`reconstructed_math`, `tool_tests`). A focused
ignored source-only probe invokes the second checked-hint validation with an
invalid owner; its returning callback replaces tree `+4`, then verifies
comparison against the captured old end and insertion beside the maximum loaded
from the replacement head. The probe was compiled as a Win32 executable with an
embedded manifest and passed. The specialization was also checked against all
nine complete current listings, including field offsets, signed comparisons,
output order, count predicate and the two explicit `REP MOVSD` schedules. This
static, source-probe and build evidence does not establish the original FH3
personality, fault behavior, binary ABI compatibility, concurrent mutation
safety, or gameplay/runtime validation.
