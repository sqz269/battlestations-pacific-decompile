# Native mesh subset loading (CM)

Addresses: 00b941d0; compiler supports 00cc2e90, 00cc2e98, 00cc2ea0.

CM implements the complete 750-byte B941D0 subset parent over the recovered native
readers, pools, caches, material owners, hardware layouts and instance generators.
It advances the real mesh-loading path beyond the prior owning wire-value parser.
`reports/native_mesh_subset_loading_cm.json` records the original ABI, all calls,
saved/disk byte checks and validation limits.

## Native order and exceptions

The parent creates a section through actual533FA0 before reading five control
DWORDs. Primitive values0..5 map to1,3,2,5,6,4; unsigned values above5 map to1.
The other four values publish directly at section+C..18. Its completed counted
name is compared as a C string, case-insensitively: `soldiers.mshd` is resized to
12 without preserving bytes, then replaced by `soldier.mshd`. The actual numeric
535320 material factory and canonical admission precede B864C0 assignment. The
temporary material reference is consumed before clearing section streams.

Children are processed in native order: Texture calls complete B93D30 with a
distinct persistent cache frame; LightingSettings calls B937A0; BoundingSphere
discards four scalars straight from x87 ST0; VertexStreamIndex uses the unchecked
wrapping B73260 lookup followed by B85B80. Unknown children skip/detach. Each
normal child release lowers the EH state before invoking BE9ED0, so failure does
not retry that cleanup.

If no stream was appended, the parent uses mesh stream0. It then rebuilds the
actual B865A0 layout, appends/retains the section in the mesh through B73C60,
and only **after mesh publication** calls B85610 with the current mesh+60 value.
The section creator reference is consumed after finalization, followed by the
captured name allocation using the current length and current native pool.

The verified DFC688 FH3 map has just two owned states: the completed local name
and the current completed child. It owns neither section nor material creators,
texture acquisitions, hardware layouts, or mesh publications. The new acquired
frame preserves these effects on failure and is never replayed. Each texture
child and the final generator retain their own continuation state. Cleanup during
unwinding is nonthrowing under the existing supported terminal/pool domain.

`GuiNativeGeometryOwners::create_native_section_00533fa0` complements the existing
GUI convenience creator. It creates the native section and admits its one actual
reference without rollback after construction. Host reference allocation or
transactional bind failure leaves the creator, stable record, and any completed
unbound companion in its caller-owned frame. A failed frame must remain alive
until its native obligations are explicitly resolved. Audit identity fields may
be stale after retirement and do not constitute another retained reference.

## Validation

The tracked MSVC Win32 build and both existing CTests pass. Saved Ghidra and the
installed executable match over750 ordinary bytes and26 support bytes. Every
instruction owner and all41 direct/4 indirect transfers were checked. The missing
CC2EA0 handler was defined and saved under the write lock, without clearing bytes
or changing no-return flags. Prior function names and comments are preserved;
the descriptive names remain reconstruction hypotheses.

One ignored focused fixture exercises the complete subset with a real retained
memory reader, actual string/material/texture pools, canonical owner registry,
actual texture/effect/declaration cache hits, and RTX5090 D3D9 buffers/layouts.
It checks the alias/prefix, all five ordered child kinds, mesh publication,
generic generator attachment and COM declaration readback. It also checks a
transactional section-bind exception and a read exception on the second sphere
scalar after texture/lighting publication. The latter cleans the name/child while
preserving section/material/texture references until explicit fixture retirement.
All15 successfully registered companions retire; the rejected bind retains no
registry entry. Actual caches/tree/allocator list become empty, all slots return,
and the final device/API reference counts are zero.

The effect-cache fixture is explicit: actual B407A0 constructs storage whose
C8..137 pass cells were zeroed **before** the constructor, which leaves them
unwritten. Actual B43700 constructs the descriptor; `generic` is supplied as its
selection-name preimage, and the actual direct-deletion bridge controls teardown.
This does not represent a successful B46950/B43B00 shader load. Missing cold
providers throw if reached. The normal fixture selects an explicit stream index;
the default-stream branch is source/assembly checked only. No original full-parent
execution, native ABI/FH3/SEH equivalence, cold shader loading, render parity or
gameplay validation is claimed.

Frozen source inputs, probe/libraries/build artifacts and byte audits live under
ignored `local/native_mesh_subset_loading_cm/registered/`; tracked reports contain
their hashes. One source-registration line is appended to `cmake/startup.cmake`
without taking a lease on that shared file.

The final coordination audit found a foreign whole-file lease:
`agent/orch5-20260911:orch5_native_platform_cmake`, claimed at01:50:49Z on
2026-09-15, preceded this packet's01:51:29Z append by40seconds. The append relied
on an earlier unleased observation instead of rechecking immediately. This is a
recorded coordination deviation, not an unleased-registration claim. No further
CMake edits were made after discovery; the foreign lease was left untouched.
The frozen build reflects the single appended line in this separate worktree.

## Follow-up packets

Fresh exports identify B944E0 (557 bytes) as the nine-field ordered dispatcher;
its concrete child sources are now present. B94710 (131 bytes) constructs and
publishes the actual mesh/output pair, calls that dispatcher, then polls the live
B72B40 section count with no visible per-element action. Implement both complete
bodies next, including the constructor's raw-slot exception state and metadata-only
mesh admission. The existing GUI mesh creator rolls back on metadata failure and
must not silently define this native parent path.

Cold material/resource loading still needs the actual descriptor/compiler and
state/pass cache children. Publication stays on the agent branch; moving `main`
deltas have not been adopted or validated.
