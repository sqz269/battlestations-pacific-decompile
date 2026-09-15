# Native resource-instance postprocessor

Addresses: 00b79bc0, 00b8f920, 00b8a180, 00b8a190, 00b8a1a0, 00b8a1b0, 00b78f60, 00b77cd0, 00b76b50

`postprocess_native_resource_instance_00b79bc0` reconstructs the complete
3613-byte physical caller, through its backward camera-assignment tail at
B7A968..B7A9DC. It uses the existing actual range/tree, registry, animator,
skin-binding, matrix-extraction and temporary-array implementations from
AS/AT/AU/AV/AW/AX/AY. The injected context borrows current native tables, type
cells, pooled strings and CRT services. It introduces no replacement graph,
name map, animation factory or successful fallback for unknown virtual calls.

The original entry receives the instance in ECX and returns with plain RET.
The C++ entry adds a borrowed context in EDX. All descriptive names remain
hypotheses. Source reconstruction and bounded differential validation do not
establish original FH3 compatibility, resource graph admission or gameplay.

## Recovered sequence

1. A nonempty compact-track range creates a shared registry. Each current
   instance node gets a 38h optimized animator. Allocation precedes its type
   predicate, the table is captured before reading the current type token,
   and the first matching borrowed compact item is published. Animator and
   registry assignments retain and release in the original order.
2. A nonempty ordinary-track range builds actual 10h node/item-list records,
   registers names even for items without a matching node, and creates 30h
   ordinary animators. Every borrowed track slot is explicitly cleared. Each
   track assignment captures the current node animator before the track getter,
   performs the canonical name lookup and writes the returned index. The
   original missing-name index of -1 is preserved, not converted to a skip.
3. Skin binding and default poses run only inside the nonempty ordinary-track
   phase. Descendant pairs are collected and matched against current mesh
   weight names and base items. Bindings preserve scalar ST0/float32 transport,
   callback ownership and finalization. Other animated nodes copy the current
   local matrix as words, then extract and copy angles through the recovered
   x87 path. Temporary storage retains its native destruction order.
4. Every current node animator dispatches its current slot20. Known ordinary
   and optimized targets use their complete recovered implementations; other
   targets require a complete caller-supplied provider.
5. Camera targets use first matching current node names, with a second vector
   validation before assignment. The old target is released before clearing,
   publishing and retaining the incoming target, including same-pointer
   assignment. Unmatched and empty target names preserve the old target.
   FOV precedes a fresh aspect read; both preserve the original flag mask.

Checked vector access retains current backing reads and returning CRT invalid
parameter boundaries. Table resolution must only resolve the actual table;
virtual calls must implement their full service. The registry validation
callback must be the same BF6713 CRT boundary used by the raw vector helpers.

## Cleanup and original evidence

The five FH3 states at DFAEC4/DFAEE8 have predecessors -1,-1,-1,-1,3.
Their actions CC1E00/0E/1C free only the raw registry/optimized-animator
constructor allocation; CC1E2A destroys the temporary node-track array and
CC1E35 destroys the nested pair array. There is no arbitrary-failure registry
release or ordinary animator construction guard. Normal pair and outer cleanup
drop their respective state first, avoiding an invented second cleanup.

B78F60 and B77CD0 are complete 23-byte physical resize(0)/free/return bodies.
Ghidra currently stores them only through B78F71 and B77CE1; each five-byte
return tail is retained from live bytes matched to the installed PE. This is
a stored-body limitation, not a missing source tail. No global no-return
annotation was changed. B76B50's complete 80-byte resize is instruction-equal
to canonical B40E00 after replacing its reserve call with the previously
verified B76890/B40C80 specialization. Canonical pair storage's rejection of
corrupt extents/overflow remains an explicit source boundary.

The four skin item leaves reproduce the original four bytes each, including
the scalar ST0 return. B8F920's six-byte global load uses an explicit borrowed
current-cell source binding. The unreachable nine-byte alignment gap inside
B79BC0 remains untouched.

## Validation

Strict MSVC Win32 build and both existing CTests pass. An ignored local probe
executes the original complete B79BC0 bytes with checked call/data relocations
and canonical recovered providers, comparing normalized object/storage bytes,
allocation/free order, virtual events, reference counts, pooled-string state
and floating-point status against the C++ source. Five paired cases cover:

- Populated compact and ordinary animation, replacement destruction, skin
  binding, matrix defaults, finalizers and safe same-pointer camera assignment.
- Empty ordinary tracks with nonempty skin data, proving the skin-phase gate.
- A predicate callback replacing a node-vector cell, and unmatched camera names.
- Empty camera target names.
- Empty ranges with a populated node vector.

The type predicate is a shared fixture protocol at the required virtual-provider
boundary, not proof of the game's type hierarchy. Destructors and finalizers
dispatch complete recovered implementations. Native FH3 dispatch is a fail-fast
tripwire and is not exercised. One source-only exception case proves that an
optimized-animator type callback failure frees its raw allocation while retaining
the previously built registry and leaving the node unattached. Invalid graphs
are not admitted: an exploratory fixture that removed a required bone animator
faulted in the original path and was replaced with a valid camera-node mutation.

Four compiled leaf bodies match the original bytes; live token reload is checked.
All 107 address/native call rows validate against current Ghidra. Full traces,
PE spans, unwind data, saved annotations, compiler inputs and linked objects are
retained locally. No workers were dispatched. Full B891A0 graph admission and
a runnable, gameplay-validated game rebuild remain outside this batch.

## Retained evidence

Implementation commit `77d8e274f095331374f564124291d0bd5cb2dee9` is integrated in `aa7e7abf3c59132fe2f42a5cdc512d2fff4540b8`, which was built as a clean checkout. The immutable local manifest retains 733 exact inputs and 133 artifacts, including 80 linked production objects, source/header/compiler/library inputs, ten original spans, paired comparison images, unwind records and saved Ghidra annotations/exports. The provider and game-validation boundaries above remain unchanged.
