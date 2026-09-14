# Actual resource root and item dispatch

Addresses: 00B7F430, 00B7E970, 00B87AA0, 00B1FE40, 00B28570.

The source operates on the actual manager28h, parser tree1Ch nodes, resource44h,
structured node24h and four-byte handles. It composes the existing actual stream,
reader, string-pool, hierarchy-pool and pointer-array services. It does not use the
older projected structured model or parser registry.

`B7F430` captures the current global renderer and its slot50 target before reading
the control word. Each child is dispatched in Resource, Hierarchy, BoundingBox
order using its current nonnull string data and CRT case-insensitive comparison.
Unknown children skip and detach. BoundingBox reads six values through BU, then
captures the current manager+24 resource and copies six words into +28..3C. The
normal child release is disarmed before it is called. Only normal completion
reacquires the current renderer and calls its current slot54 target; no balancing
end hook is added during failure cleanup.

`B7E970` uses the actual lower-bound helper at B7DF40 on manager+8 and the child's
counted tag. Its inlined equivalence comparison tests lengths for emptiness and
otherwise calls CRT `_stricmp`; it is not a length-limited byte comparison. The
selected iterator owner/node and end are captured, with the original returning
BF6713 invalid-parameter boundaries retained. The current parser object and its
slot8 target are captured immediately before dispatch. Unknown tags allocate an
eight-byte B86930 fallback and explicitly skip payload. Both paths reacquire
manager+24 and that resource's current slotC target before appending. No AddRef,
null-item filtering or completed-item guard is introduced.

`B87AA0` appends to resource+10/14/18 using the existing B872F0 reserve domain.
Capacity is read before count. Equality grows by wrapping16 with signed minimum16.
After reserve it reloads current count/data, skips only a computed-null element
store, then increments the current count. The item is a borrowed raw pointer at
this boundary; ownership is not reconstructed by inventing a wrapper.

The D63228 default-resource slotC points to B87AA0. D5F0A8 renderer slots50/54 point
to B1FE40/B28570. B1FE40 is RET only. B28570 installs and removes an inactive native
exception frame without any object access or service call. Their source bindings
implement this normal behavior; they do not reproduce that private SEH frame.
Other renderer/resource targets and parser targets are forwarded explicitly.
The game resource's CFD8CC slotC is 71BB40, which is not treated as base append.

Exception evidence: CC1FE3 -> DFB134, unwind map DFB124: state1->0 frees the saved
fallback allocation at synthetic EBP-2C through CC1FD8/BF65AC; state0->-1 releases
the child handle at EBP-34 through CC1FD0/BE9ED0. The fallback allocation guard is
disarmed before skip, so a later failure does not reclaim the constructed item.
CC2098 -> DFB228, map DFB220: state0->-1 releases the child at EBP+4 through
CC2090/BE9ED0. Source cleanup terminates if its child unwind itself throws.

The focused fixture uses the actual memory-stream, reader, root/child, string,
fallback, default-resource, hierarchy and array services. It checks mixed nested
dispatch, case-folded registered lookup, unknown/empty tags, manager resource
changes during parser and box reads, renderer replacement, empty-root bracketing,
parser failure after an actual read and failure after an actual append. Completed
items survive failure at refcount1. Explicit fixture teardown returns their
storage; it is not attributed to the native dispatcher or resource destructor.

The fixture's registered parser is a boundary probe, not the native Mesh parser.
No original dispatcher body, native FH3 exception runtime, corrupt private-stack
aliases, hardware faults or gameplay were exercised. Actual manager registration,
concrete parser implementations, game-resource classification, B80720 load/cache
composition and queue shutdown remain open. Source names are hypotheses and the
C++ interfaces are not original binary ABI replacements.

Byte, call-site, build, source and Ghidra evidence are recorded in
`reports/native_resource_root_dispatch_bw.json` and the BW integration report.
