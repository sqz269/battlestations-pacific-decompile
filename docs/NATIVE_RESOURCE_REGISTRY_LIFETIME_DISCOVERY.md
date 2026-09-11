# Actual resource registry teardown readiness

The five candidate bodies are now bounded and independently checked against
full original assembly. They form a real subtree, iterator, single-node,
range and registry-destruction chain. `B19F90` is a 716-byte erase routine
with an invalid-iterator exception branch; its current `STL_xlen_throw`
name and truncated export do not describe the complete function.

The smallest ready source packet is six tree leaves totaling 396 bytes.
Two further bounded packets can add full single/range erasure and then the
registry destructor/reset. No source is added here, and no full-range-only
replacement is proposed for the general range routine. The independently
implemented `B19E90` lookup does not close any of these ownership routes.

## Evidence and extents

Fresh guarded calls verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Forty-one finite spans, 2,571 bytes, match the
installed original PE SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Base checkout: `ce587728852d16e9f4928068e264f819ad1841f9`. No build, probe,
game, source, Ghidra or shared-metadata mutation was performed.

| Entry | Full range, exclusive end | Bytes | Original ABI |
|---|---|---:|---|
| B1A260 | B1A260–B1A2B2 | 82 | ECX tree, stack subtree node, RET4; no semantic result |
| B1A2F0 | B1A2F0–B1A3B9 | 201 | ECX tree; stack output, first owner/node, last owner/node; EAX output, RET14h |
| B1B5F0 | B1B5F0–B1B65F | 111 | ECX actual registry, RET; no semantic result |
| B19890 | B19890–B198F3 | 99 | ECX mutable two-word iterator, RET or returning CRT tail; no semantic result |
| B19F90 | B19F90–B1A25C | 716 | ECX tree; stack output and by-value owner/node; EAX output, RET0Ch |

The five total 1,209 bytes. Immediate helpers are full `B19640` right
rotation (82), `B19830` left rotation (78), `B196D0` maximum (28), and
`B196F0` minimum (27): another 215 bytes. Rotations take ECX tree and one
stack node, RET4; EAX retains the replacement node but callers consume no
semantic result. Min/max take ECX node, return the selected node in EAX and
RET. They return the current candidate before its nil child, unlike their
misleading void pseudocode.

Required primary analysis repairs, without changing correct `_free` naming
or its library-wide metadata:

- `B1A260` already has the full saved extent, but the `B1A29C` free call is
  missing its returning flow. Bytes `B1A2A1..B1A2AB` are
  `83c404807e19008bfe74c5`: stack cleanup, current captured-left nil test,
  next current-node assignment and loop backedge.
- `B19F90` is saved only through `B1A221`. The returning-free call at
  `B1A21D` continues at `B1A222` through `RET0C` at `B1A259`; full end is
  `B1A25C`, followed by four `CC` bytes and the distinct `B1A260` entry.
  Restore its 58-byte tail and rename/comment it as actual tree erasure.
  Preserve historical name/tag provenance; reassess the overly broad
  `stl_throw_site` tag rather than treating it as proof of throw-only code.
- `B1B5F0` is saved only through `B1B637`. Its free call at `B1B633`
  continues at `B1B638` through the RET at `B1B65E`: 39 missing bytes.
  `B1B65F` is padding before distinct scalar-deleting caller `B1B660`.

The exact tails, saved extents, old B19F90 comment/name/tag view and original
neighbor padding are frozen in the companion audit. Historical shorter
exports remain partial evidence; they are not retroactively full-body proof.

## Actual storage and subtree ownership

The registry is an actual 10h-byte object: profile +0, retained word +4,
tree head +8 and count +C. The tree address is registry+4; head is tree+4
and count is tree+8. Head links +0/+4/+8 hold minimum/root/maximum. Native
1Ch nodes have left/parent/right at +0/+4/+8, pooled key length/data at
+C/+10, factory value at +14 and color/nil bytes at +18/+19. These bodies
do not construct/populate a tree or validate all structural invariants.

`B1A260` captures the supplied tree and node. A nil node exits. For each
non-nil node it recursively destroys the current right subtree, reads the
current key-data pointer, then captures the current left child **before**
any key-pool call. Nonzero key data causes a current length read and wrapped
DWORD `length+1`; the native third return argument is one and unused.
It always calls `419CC0` before `BD1510`, even for a large free or disabled
small-return gate. Then it frees the captured node through `BF65AC`. Only
after that returning free does it test the captured left child's current nil
byte and continue down that child. It never reads, retains, destroys or
frees factory value +14, never resets the disposed key header, and never
updates head or count itself.

Fresh profile words are `D5E594 = {B1B660, B1B3A0}` and
`D5E59C = {B1B710, B1B3A0}`. These are native identities and caller/lifetime
evidence; B1B5F0 itself does not dispatch through or inspect either table.
The scalar-deleting callers remain separate from this body and are not
newly reconstructed by this discovery.

The complete `ActualNativeStringPoolStorage` bridge supplies this exact
getter/return composition through actual `01090AA8` publication,
`01090AA4` gate and the canonical lifetime domain. It uses the existing raw
8AD4A0h owner and embedded ring, not the old `SizedStoragePool` projection.
Data and size must be captured before calling it. Its existing `release`
API is `noexcept`: the returning-getter domain is ready, while a C++ failure
during lazy pool recreation terminates under that established interface.
Do not claim native EH equivalence for that excluded case or introduce a
second allocator, cached pool, early large-free shortcut or cleanup callback.

## Iterator and complete single-node erasure

`B19890` calls returning CRT on null iterator owner, then reads the current
node. A nil node tail-calls `BF6713`; if that handler returns, the function
returns without advancing. Otherwise it selects the minimum in the right
subtree, or climbs parents. On each climb it rereads iterator+4, publishes
the intermediate parent before following its parent, and finally writes the
selected node. It never compares owner identity with a destination tree.

`B19F90` first captures the original input node and tests its nil byte,
before any iterator-owner validation. A nil node builds the real owning
out-of-range payload described below. On the ordinary path it advances its
**by-value input iterator**, retains the originally captured node for
disposal, and chooses replacement/fixup parent. It handles both direct
replacement and a two-child successor transplant. The latter live block
`B1A09C..B1A0F3` rewires the successor and original children/parent/root,
then swaps color bytes; Ghidra's pseudocode incorrectly removes it.

Black-node repair handles both symmetric sibling sides, red-sibling rotation,
nil siblings, two-black-child propagation, inner rotation and final outer
rotation. It rereads the current tree head/root where the native body does,
including after helper calls. The concrete rotation helpers update only
links, use +19 nil checks and reread replacement children before parent
updates. The hardware-layout tree's +24/+25 color/nil storage is different;
its tree providers are not substitutes for these nodes.

After rebalancing, erasure reads the captured original node's current key
data and length, returns that key through the actual pool, and frees the
original node. The recovered tail then reads the **current tree count after
free** and decrements it only when its unsigned DWORD value is nonzero.
It captures the advanced input owner, output pointer and advanced input node
before the first output store, publishes owner then node, and returns output.
No value +14 ownership operation occurs. No input-owner/destination equality
check is added, and output aliases must not make the second store reread
already overwritten input state.

Invalid-iterator construction is finite and already has complete owning
providers. Original order: initialize only SBO capacity=15, length=0 and
first byte NUL; call `408720` with the 27-byte message at `CE44E0`; arm
state 0 only after assignment succeeds; call `411700`; publish `D6926C`;
call `BF6885` with ThrowInfo `D863A8`. FH3 `CBC628` selects FuncInfo
`DF48F0` / map `DF48E8`; state 0 unwinds through `CBC620`, which destroys
the completed temporary at EBP-50 via full `4072D0`. Earlier assignment
failure has no newly armed caller temporary cleanup.

ThrowInfo selects full destructor `4412B0`; its first CatchableType selects
full copy `441760`, size28h, with the actual out_of_range type descriptor at
`E0817C`. The current native SBO, logic-error and out-of-range copy/destructor
providers are complete. A source packet can compose them into an owning host
carrier, as the existing hardware-tree carrier does, while explicitly keeping
host RTTI/catch type/exception transport distinct from the original ABI.
It must not replace this path with a generic `std::out_of_range` or cleanup
callback. The newly pinned full 74-byte `BF6885` invokes imported
`KERNEL32!RaiseException` using template `D693B0`: code `E06D7363`, flag1,
three parameters. Host CRT/FH3/Windows exception dispatch remains an explicit
runtime boundary, not a reconstructed general exception runtime.

## General range and registry destructor

`B1A2F0` captures the first owner and current head minimum before its first
returning validation, then reads the first node. Only when that node equals
the captured minimum does it capture the current head and validate last
owner against the destination; last node is read after the handler. If it
equals the captured head, the whole-tree branch recursively clears the
**current** root. It then repeatedly rereads head while storing root=head,
count=0, minimum=head and maximum=head, in that order. The head used for the
minimum store is loaded before count is cleared. Finally it captures the
current head minimum and output pointer, stores destination owner then
captured minimum and returns output. Empty full ranges take this same path.

The partial path is complete and necessary. Each loop validates the cached
first owner against the current by-value last owner, then compares cached
first node with the last node read after any handler. It advances the actual
local first iterator, calls full B19F90 on the **previously captured**
owner/node, ignores that erase output, then reloads current first node and
owner for the next iteration. B19F90 separately advances its own input copy:
these two increments cannot be collapsed into the returned erase iterator.
On completion output receives the cached first owner then node. No catch or
rollback is introduced for partial progress.

`B1B5F0` captures registry, head and minimum, forms the two iterator values,
then arms state 0 before calling full range erase. After it returns, the
destructor frees the **current** head, then zeros tree head and count,
clears actual `F8D41C`, and publishes `CE3818` on the captured registry.
It performs no initial profile write, factory cleanup, unregister operation,
or free of the registry allocation itself.

FH3 `CBC778` selects `DF4AD8` / map `DF4AD0`: state0 -> `CBC770` -> full
17-byte `B19760`, with captured registry at EBP-18. The reset first clears
F8D41C, then writes CE3818. Thus failure in range cleanup leaves any partial
tree disposal and head/count state as they stand; its unwind only resets
publication/profile. Reusing this prior getter fact is supported by fresh
reset bytes. The registry may remain partially disposed and allocated; no
extra rollback or second node cleanup is justified.

## Proposed source packets

1. **Six actual tree leaves, 396 bytes:** B1A260, B19890, B19640, B19830,
   B196D0 and B196F0. Four new `native_resource_registry_tree_leaves`
   cpp/hpp/doc/audit files. APIs take actual tree/node or an explicit two-word
   iterator, `ActualNativeStringPoolStorage&` for subtree cleanup, and the
   established returning `SingletonLifetimeCallbacks` service for increment.
   No constructors, retained factory policy or runtime allocator callbacks.
2. **Full erase and range, 917 bytes:** B19F90 and B1A2F0 in four new
   `native_resource_registry_tree_erase` files. Requires packet1, the existing
   raw SBO/exception providers, and a reviewed owning host carrier/API with
   the explicit native exception-ABI boundary. APIs return the caller's
   two-word output and take by-value iterator(s), actual tree, actual pool
   and returning CRT service. Preserve all transplant/fixup paths, both
   increments, current counter/head reads and output capture order.
3. **Destructor/reset, 128 bytes:** B1B5F0 and B19760 in four new
   `native_resource_registry_destroy` files. Requires packet2 and a borrowed
   reference to the actual F8D41C publication. No getter, registration,
   constructor, scalar-delete or registry-allocation ownership is implied.

These are proposals, not reconstructed-source claims. The first packet is
immediately bounded. Later packets have concrete dependencies and no need for
an assumed full-range-only stub. Existing typed/general tree implementations
are comparison material only. The current pool/exception provider sources
are unchanged from prior registry-discovery base49b628a2; the adjacent
`native_string.cpp`/header changed for an unrelated BE0A30 adapter. Both
versions and the finite diff are pinned; no old source-pin equivalence is
assumed. Fresh checkout source and all selected direct dependencies are
hashed in the audit.

The singleton getter `B1B730`, wrapper `B1B810`, scalar lifetime adapters and
general registry population remain outside this packet. Their prior
discovery remains referenced evidence, not renewed proof or whole-cache
closure. All source, ABI, fixture and game-validation flags remain false for
this read-only discovery.

The separately completed lookup source is worker commit `1238271b`, still
outside this packet's `ce587728` base. Its sealed audit is pinned as an
external reference, not represented as a provider already present in this
checkout or as a teardown dependency.
