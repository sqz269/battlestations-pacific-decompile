# Actual node root propagation

This packet reconstructs complete B6D890 over actual node and outer24h scene
storage. Its body is [B6D890,B6D934),164 bytes,60 instructions; the final
instruction is RET4 at B6D931,3 bytes. Original ECX is the node and the sole
stack word is the requested outer scene. The source interface adds the shared
raw attachment dispatcher and is not a binary replacement.

## Actual identities and read order

The requested pointer is captured at B6D891 before any node read. It remains
the original requested value through all callbacks and direct child recursion.
No caller scratch frame is needed by this body. Its actual node fields are
parent30, firstchild34, next3C, previous40, outer-scene A4 and resource170.
Outer+0C contains an actual node pointer; outer+1C contains actual3Ch resource
identity. No field receives a host root-list, transform or scene companion.

If current A4 equals the request and parent30 is nonzero, return immediately,
including no child traversal. Otherwise an old nonzero root with no current
parent is unlinked through the existing raw B72220, then nodeA4 receives the
captured request. A nonzero request captures current parent30. The parentless
path prepends through genuine raw B721F0 before checking current node170.

When that field is null, the root path captures current nodeA4, then the node
profile and slot50 address, before calling B72110. The existing genuine4B
getter loads current outer+1C. Its nominal pointer type is consumed only as
opaque identity; no host object is accessed. Only after the getter does this
body read the captured slot's current target and invoke it on the actual node
with the resource and recursion0.

The parent path uses the parent captured before its branch. It reads current
node profile, captured parent's current170, and current slot50, in that order.
It invokes that exact target with resource and recursion0. Both paths then
read current firstchild34. Each direct recursive B6D890 uses the original
requested outer scene; the loop rereads current child3C after the return.

## Shared providers and failure

The source reuses `unlink_native_raw_root_node_00b72220`,
`prepend_native_gui_scene_node_00b721f0`, and
`native_scene_lighting_owner_00b72110` unchanged. It shares
`NativeNodeSceneChildDispatch` with the raw node/light attachment packet.
That interface resolves actual numeric profiles through pure metadata lookup
and invokes the genuine captured target on the actual reached node. Here that
node may be self and recursion is zero. No default target or logical dispatcher
is installed. The binding supplies initialized persistent attachment frames
and Acquired records in the same actual owner/import domain.

This function adds no retain, admission, rollback or destructor installation.
An attachment failure leaves completed root/list stores and callee acquisition
diagnostics intact. All callback-modified reached storage must remain valid.
The existing typed B6D890 and raw requested-null-only implementation remain
separate; their source and ledger records are preserved.

## Verification boundary

The complete 164-byte live body matches the installed PE. The report retains
four direct call sites (including recursion), two indirect current50 sites,
the exact final instruction and a complete listing. Ghidra remains read-only.
Strict `scripts/build.ps1` passed MSVC Win32 and both configured CTests
(`reconstructed_math`, `tool_tests`). The focused comparison below passed;
no tracked test was added.

The focused ignored original/source comparison uses copied original B6D890
with genuine shared raw unlink/prepend/getter and B6ED80 attachment providers.
Its actual3Ch resources are constructed by B83C50 and admitted through the
existing canonical same-count references. The fixture uses initialized current
camera descriptor cells, actual174h node prefixes and actual24h root storage.
It does not substitute host scene objects or fabricate resource credits.

One sequence checks actual root unlink/prepend, root-derived and parent-derived
resource identities, callback changes to the incoming request, root1C, current
firstchild and current next, equal-root/nonzero-parent early return, and null
root clearing. Full node-prefix snapshots are normalized only for allocated
identities and relocated profile addresses. Genuine raw removal then balances
the acquired resource credits and real canonical terminals retire both
resources and their ambient owners. Native root/camera construction and node
terminal destruction are outside this fixture.

The exact probe command is retained in the report and
`local/output/cc10_root_propagation_probe.cmd`. It uses `/MD`, `/fp:strict`,
`/link /MANIFEST:EMBED`, and active assertions; `NDEBUG` is rejected at compile
time. The native incoming-word mutation checks the captured request, not
private-stack ABI equivalence. Nested helper/attachment/predicate machine code,
native CRT/FH3/fault behavior and full application/gameplay are not established.

This source is deliberately not installed into legacy node/light/outer-scene
destructors. Raw normal root propagation does not close their host scene-resource
cleanup paths or admit full AC59A0. See the preserved ignored readiness handoff
`local/cc10_raw_root_propagation_readiness.md` for the packet boundary.
