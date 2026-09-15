# Native render-service focus refresh

Addresses: `00b0d1e0`, `00b50010`, `00b4ecc0`, `00b72220`; fixed null-root
specialization of `00b6d890` at the `00b4ecdb` call site.

`native_render_service_focus` implements the service refresh called by the
actual window activation path. Four complete bodies total150 native bytes.
The raw null-root specialization covers the entire behavior of B4ECC0's fixed
B6D890(node,0) call, with the full164-byte original function retained as evidence.
Descriptive service/batch/chain names are hypotheses, not recovered class names.

B0D1E0 checks service byte+1C4, captures +20 and, if nonnull, invokes B50010.
That leaf writes receiver byte+24C=1; the inventory's "trivial" tag does not
mean no-op. B0D1E0 then rechecks +1C4, captures +30 and tail-routes to B4ECC0
when nonnull. It retains the original service receiver throughout.

B4ECC0 captures receiver+3C **before** writing byte+250=1, then tests captured
root+0C. Each iteration reloads current receiver+3C and its head, invokes
B6D890(head,0), and reloads receiver+3C/head afterward. It ends by setting
byte+251=1. There is no node release, scene detach or reference-count change.

For requested_root=0, B6D890 returns immediately when node+A4 is already null
and node+30 parent is nonnull. This skips that node's entire descendant chain.
Otherwise, a registered node with no parent is unlinked from its old root;
node+A4 is cleared, then children at +34 are recursively processed with each
child's +3C reloaded after its call. All nonzero-root registration and virtual
scene-attachment branches are unreachable for this fixed argument. The source
does not expose an arbitrary-root interface or claim those branches.

B72220 reads current next+3C and, if nonnull, updates its previous+40. It
then reloads the removed node's previous+40; a nonnull previous gets its next
updated from the current node+3C, otherwise raw root+0C gets that next value.
It preserves the removed node's own link/root/parent fields and references.

## Representation boundary

The existing `RenderNodeRootList` is a host view containing references; it is
not the original root object at whose +0C the native node pointer resides.
`NativeNodeStorage` preserves raw hierarchy pointer words, but its typed +A4
field is currently a pointer to that host root view. Passing such a view to
this raw service path would not preserve the native identity/layout.

This implementation consumes actual raw root/node addresses directly. It does
not cast the host root view into a native root, synthesize another root registry,
or route through the general typed root-propagation API. Full service and node
producers need an explicit compatible identity bridge before application use.
The fixed null-root path has no scene/virtual callbacks and is therefore
recoverable without inventing those missing producer contracts.

## Evidence and validation

Report: `reports/native_render_service_focus.json`. All314 original body bytes
match fresh Ghidra memory and the unchanged installed PE. Seven direct/tail
call rows are mechanically checked; two belong only to the unimplemented
nonzero-root branch and are labeled accordingly. Its two virtual calls are
also retained as unreachable evidence. Another826 bytes preserve the existing
814-byte node constructor and its three actual native constant cells.

Strict Win32 build and both existing CTests pass. No permanent test was added.
A fresh copy of the closed window-focus fixture compiles24 explicit source
units and selects23 at link, using current production libraries. One failed
probe compile (member-name shadowing and NativeString accessor spelling) is
retained; compile/link attempt2 and execution capture1 pass.

The original five bodies execute from a413804-byte sparse RX image containing
314 unchanged instruction bytes at their original relative offsets. All gaps
are CC bytes. Direct call and tail displacements are unchanged. Nonzero-root
callees are deliberately absent because the service supplies zero; no original
instruction, branch or import target is replaced by a stub.

Six178h node allocations run the existing raw B6F5A0 source constructor with
the same actual string-pool domain and mapped native bounds/one constants.
Root, service and batch fields are explicit raw entry preimages. The topology
has two roots and descendants, including a parented node whose root is already
null while its child still has a root. Scene fields hold opaque retained tokens,
not claimed scene implementations. Four cases cover both branches enabled,
the service disabled, +20 absent, and +30 absent. Complete byte comparisons
cover all node allocations and all service/batch/chain/root storage.

The source matches the original root drain, dirty flags, stale links and
child counts. The already-null parented node leaves its descendant untouched.
Reference counts and scene identities remain unchanged. Fixture storage is
explicitly disposed afterward; this is not the game node destructor or a
shutdown proof. The same run retains the earlier actual platform/renderer/
device/Reset/cache/focus-owner/Bink/installed DDS/effect checks.

## Outstanding activation dependencies

This packet does not implement the renderer's B24FB0 file-change refresh.
That body checks actual108D4BB, scans effect registry+1A98 through B22030,
invokes current renderer virtual120, then tail-scans texture registry+1A74
through B21F70. Both scans capture count **before** data, keep a fixed wrapping
2Ch-row end, query BDD340 through current0109CEEC for each row, compare five
unsigned date words, store a newer date before calling current resource08.

Renderer D5F0A8+120 points to B24DD0, which traverses effect owners and invokes
B19000 (texture-name/binding re-resolution). Effect profile D61A00+08 points
to B469A0, a distinct EH-wrapped reload body currently missing its own Ghidra
function despite lying after B46950. It checks108D6F1, logs/cleans, loads current
name through B46950 and has a fallback path. Texture profile D61948+08 points
to B3FA90, whose larger body reopens the stream, unregisters/reloads and recreates
the texture with native dimension/mip policy. These require further recovery;
they must not be treated as a virtual no-op or replaced by a date-only scan.

Full raw sound production, GUI/page producers, actual window initialization
and message-handler integration, active frame/draw, full shutdown and gameplay
remain outstanding. BEC3B0's inner BED3B0 is callee-cleans-five/RET14h; the full
BECEE0 initializer remains cdecl11. New source interfaces are not original ABI,
and invalid storage, concurrent mutation and nonzero-root registration are not
runtime-certified by this fixture.

Immutable closure: `local/checkpoints/9f62a71a/native-render-service-focus/validation.json`, SHA-256 `5fe4ef3cb46caa839fabc373563226900b23ac47b0a889545d5aa40ab0d117b0`. It retains 4931 artifacts, 82 physical Win32 modules and 311 selected production source providers. Captured docs/report precede this closure metadata addition. Ledger upsert sorted existing rows; semantic changes are restricted to the five owned addresses.
